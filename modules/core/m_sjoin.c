/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  m_sjoin.c: Joins a user to a channel.
 *
 *  Copyright (C) 2002 by the past and present ircd coders, and others.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 *  $Id: m_sjoin.c,v 1.2 2004/09/08 03:44:29 nenolod Exp $
 */

#include "stdinc.h"
#include "tools.h"
#include "handlers.h"
#include "channel.h"
#include "channel_mode.h"
#include "client.h"
#include "hash.h"
#include "irc_string.h"
#include "sprintf_irc.h"
#include "ircd.h"
#include "list.h"
#include "numeric.h"
#include "send.h"
#include "common.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "s_serv.h"
#include "s_conf.h"

extern BlockHeap *ban_heap;
static void ms_sjoin(struct Client *, struct Client *, int, char **);

struct Message sjoin_msgtab = {
  "SJOIN", 0, 0, 0, 0, MFLG_SLOW, 0,
  {m_unregistered, m_ignore, ms_sjoin, m_ignore, m_ignore}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(&sjoin_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(&sjoin_msgtab);
}

const char *_version = "$Revision: 1.2 $";
const char *_desc = "Used in server to server communications";
#endif

static char modebuf[MODEBUFLEN];
static char parabuf[MODEBUFLEN];
static char *para[MAXMODEPARAMS];
static char *mbuf;
static int pargs;

static void set_final_mode(struct Mode *, struct Mode *);
static void remove_our_modes(struct Channel *, struct Client *);
static void remove_a_mode(struct Channel *, struct Client *, int, char);
static void remove_ban_list(struct Channel *, struct Client *, dlink_list *, char, int);

/* ms_sjoin()
 *
 * parv[0] - sender
 * parv[1] - TS
 * parv[2] - channel
 * parv[3] - modes + n arguments (key and/or limit)
 * parv[4+n] - flags+nick list (all in one parameter)
 *
 * process a SJOIN, taking the TS's into account to either ignore the
 * incoming modes or undo the existing ones or merge them, and JOIN
 * all the specified users while sending JOIN/MODEs to local clients
 */
static void
ms_sjoin(struct Client *client_p, struct Client *source_p,
         int parc, char *parv[])
{
  struct Channel *chptr;
  struct Client  *target_p, *lclient_p;
  time_t         newts;
  time_t         oldts;
  time_t         tstosend;
  static         struct Mode mode, *oldmode;
  int            args = 0;
  int            keep_our_modes = 1;
  int            keep_new_modes = 1;
  int            fl;
  int            people = 0;
  int            num_prefix = 0;
  int            isnew;
  int            buflen = 0;
  char           *s, *nhops;
  static         char nick_buf[2*BUFSIZE]; /* buffer for modes and prefix */
  static         char uid_buf[2*BUFSIZE];  /* buffer for modes/prefixes for CAP_TS6 servers */
  static         char sjbuf_nhops[BUFSIZE]; /* buffer with halfops as @ */
  char           *nick_ptr, *uid_ptr;      /* pointers used for making the two mode/prefix buffers */
  char           *p; /* pointer used making sjbuf */
  int            i;
  dlink_node     *m;

  sjbuf_nhops[0] = '\0';

  if (IsClient(source_p) || parc < 5)
    return;

  /* SJOIN's for local channels can't happen. */
  if (*parv[2] != '#')
    return;

  if (!check_channel_name(parv[2]))
    return;

  modebuf[0] = '\0';
  mbuf = modebuf;
  pargs = 0;
  newts = atol(parv[1]);

  mode.mode = 0;
  mode.limit = 0;
  mode.key[0] = '\0';
  s = parv[3];

  while (*s)
  {
    switch (*(s++))
    {
      case 'c':
	mode.mode |= MODE_NOCOLOR;
	break;
      case 'S':
	mode.mode |= MODE_STRIPCOLOR;
	break;
      case 'P':
        mode.mode |= MODE_PEACE;
        break;
      case 'O':
        mode.mode |= MODE_OPERONLY;
        break;
      case 'E':
        mode.mode |= MODE_REGONLY;
        break;
      case 'T':
        mode.mode |= MODE_TOPICLOCK;
        break;
      case 'N':
        mode.mode |= MODE_STICKYNICK;
        break;
      case 'K':
        mode.mode |= MODE_NOKNOCK;
        break;
      case 'z':
	mode.mode |= MODE_NOTHROTTLE;
	break;
      case 't':
        mode.mode |= MODE_TOPICLIMIT;
        break;
      case 'n':
        mode.mode |= MODE_NOPRIVMSGS;
        break;
      case 's':
        mode.mode |= MODE_SECRET;
        break;
      case 'm':
        mode.mode |= MODE_MODERATED;
        break;
      case 'i':
        mode.mode |= MODE_INVITEONLY;
        break;
      case 'p':
        mode.mode |= MODE_PRIVATE;
        break;
      case 'V':
        mode.mode |= MODE_FREEVOICE;
        break;
      case 'r':
        mode.mode |= MODE_REGISTERED;
        break;      
      case 'G':
        mode.mode |= MODE_USEFILTER;
        break;
      case 'k':
        strlcpy(mode.key, parv[4 + args], sizeof(mode.key));
        args++;
        if (parc < 5+args)
          return;
        break;
      case 'l':
        mode.limit = atoi(parv[4 + args]);
        args++;
        if (parc < 5+args)
          return;
        break;
    }
  }

  parabuf[0] = '\0';

  if ((chptr = get_or_create_channel(source_p, parv[2], &isnew)) == NULL)
    return; /* channel name too long? */

  oldts   = chptr->channelts;
  oldmode = &chptr->mode;

  if (ConfigFileEntry.ignore_bogus_ts)
  {
    if (newts < 800000000)
    {
      sendto_realops_flags(UMODE_DEBUG, L_ALL,
			   "*** Bogus TS %lu on %s ignored from %s",
			   (unsigned long)newts, chptr->chname,
			   client_p->name);

      newts = (oldts == 0) ? 0 : 800000000;
    }
  }
  else
  {
    if (!newts && !isnew && oldts)
    {
      sendto_channel_local(ALL_MEMBERS, chptr,
 		  	   ":%s NOTICE %s :*** Notice -- TS for %s changed from %lu to 0",
		  	   me.name, chptr->chname, chptr->chname, (unsigned long)oldts);
      sendto_realops_flags(UMODE_ALL, L_ALL,
		           "Server %s changing TS on %s from %lu to 0",
			   source_p->name, chptr->chname, (unsigned long)oldts);
    }
  }

  if (isnew)
    chptr->channelts = tstosend = newts;
  else if (newts == 0 || oldts == 0)
    chptr->channelts = tstosend = 0;
  else if (newts == oldts)
    tstosend = oldts;
  else if (newts < oldts)
  {
    keep_our_modes = NO;
    chptr->channelts = tstosend = newts;
  }
  else
  {
    keep_new_modes = NO;
    tstosend = oldts;
  }

  if (!keep_new_modes)
    mode = *oldmode;
  else if (keep_our_modes)
  {
    mode.mode |= oldmode->mode;
    if (oldmode->limit > mode.limit)
      mode.limit = oldmode->limit;
    if (strcmp(mode.key, oldmode->key) < 0)
      strcpy(mode.key, oldmode->key);
  }
  set_final_mode(&mode, oldmode);
  chptr->mode = mode;

  /* Lost the TS, other side wins, so remove modes on this side */
  if (!keep_our_modes)
  {
    remove_our_modes(chptr, source_p);

    if (HasID(source_p))
    {
      if (dlink_list_length(&chptr->banlist) > 0)
        remove_ban_list(chptr, client_p, &chptr->banlist,
                        'b', NOCAPS);

      if (dlink_list_length(&chptr->exceptlist) > 0)
        remove_ban_list(chptr, client_p, &chptr->exceptlist,
                        'e', CAP_EX);

      if (dlink_list_length(&chptr->invexlist) > 0)
        remove_ban_list(chptr, client_p, &chptr->invexlist,
                        'I', CAP_IE);
      if (dlink_list_length(&chptr->quietlist) > 0)
        remove_ban_list(chptr, client_p, &chptr->quietlist,
                        'q', CAP_IE);
      if (dlink_list_length(&chptr->restrictlist) > 0)
        remove_ban_list(chptr, client_p, &chptr->restrictlist,
                        'd', CAP_IE);
    }
  }

  if (*modebuf != '\0')
  {
    /* This _SHOULD_ be to ALL_MEMBERS
     * It contains only +imnpstlk, etc */
    sendto_channel_local(ALL_MEMBERS, chptr, ":%s MODE %s %s %s",
	                 source_p->name, chptr->chname, modebuf, parabuf);
  }

  modebuf[0] = parabuf[0] = '\0';
  if (parv[3][0] != '0' && keep_new_modes)
  {
    channel_modes(chptr, source_p, modebuf, parabuf);
  }
  else
  {
    modebuf[0] = '0';
    modebuf[1] = '\0';
  }

  buflen = ircsprintf(nick_buf, ":%s SJOIN %lu %s %s %s:",
		      source_p->name, (unsigned long)tstosend,
		      parv[2], modebuf, parabuf);
  nick_ptr = nick_buf + buflen;

  buflen = ircsprintf(uid_buf, ":%s SJOIN %lu %s %s %s:",
                      ID(source_p), (unsigned long)tstosend,
                      parv[2], modebuf, parabuf);
  uid_ptr = uid_buf + buflen;

  /* check we can fit a nick on the end, as well as \r\n and a prefix "
   * @+".
   */
  if (buflen >= (IRCD_BUFSIZE - 2 - NICKLEN))
  {
    sendto_realops_flags(UMODE_ALL, L_ALL,
			 "Long SJOIN from server: %s(via %s) (ignored)",
			 source_p->name, client_p->name);
    return;
  }

  mbuf = modebuf;
  para[0] = para[1] = para[2] = para[3] = "";
  pargs = 0;

  *mbuf++ = '+';

  nhops = sjbuf_nhops;

  s = parv[args + 4];

  /* remove any leading spaces */
  while (*s == ' ')
    s++;
   
  /* if theres a space, theres going to be more than one nick, change the
   * first space to \0, so s is just the first nick, and point p to the
   * second nick
   */
  if ((p = strchr(s, ' ')) != NULL)
    *p++ = '\0';

  while (s != NULL)
  {
    fl = 0;
    num_prefix = 0;

    for (i = 0; i < 4; i++)
    {
      if (*s == '@')
      {
        fl |= CHFL_CHANOP;
        s++;
      }
#ifndef DISABLE_CHAN_OWNER
      else if (*s == '!')
      {
        fl |= CHFL_CHANOWNER;
        s++;
      }
#endif
      else if (*s == '+')
      {
        fl |= CHFL_VOICE;
        s++;
      }
      else if (*s == '%')
      {
        fl |= CHFL_HALFOP;
        if (keep_new_modes)
        {
          *nhops++ = *s;
          num_prefix++;
        }
        s++;
      }
    }

    target_p = find_person(s);

    /*
     * if the client doesnt exist, or if its fake direction/server, skip.
     * we cannot send ERR_NOSUCHNICK here because if its a UID, we cannot
     * lookup the nick, and its better to never send the numeric than only
     * sometimes.
     */
    if (target_p == NULL ||
        target_p->from != client_p ||
        !IsPerson(target_p))
    {
      goto nextnick;
    }

    if (keep_new_modes)
    {
      if (fl & CHFL_CHANOP)
      {
        *nick_ptr++ = '@';
        *uid_ptr++  = '@';
      }
      if (fl & CHFL_HALFOP)
      {
        *nick_ptr++ = '%';
        *uid_ptr++  = '%';
      }
      if (fl & CHFL_VOICE)
      {
        *nick_ptr++ = '+';
        *uid_ptr++  = '+';
      }
#ifndef DISABLE_CHAN_OWNER
      if (fl & CHFL_CHANOWNER)
      {
        *nick_ptr++ = '!';
        *uid_ptr++  = '!';
      }
#endif
    }

    /* copy the nick to the two buffers */
    nick_ptr += ircsprintf(nick_ptr, "%s ", target_p->name);
    uid_ptr  += ircsprintf(uid_ptr,  "%s ", ID(target_p));

    if (!keep_new_modes)
    {
      if (fl & (CHFL_CHANOP|CHFL_HALFOP|CHFL_CHANOWNER))
        fl = CHFL_DEOPPED;
      else
        fl = 0;
    }
    people++;

    /* LazyLinks - Introduce unknown clients before sending the sjoin */
    if (ServerInfo.hub)
    {
      DLINK_FOREACH(m, serv_list.head)
      {
        lclient_p = m->data;

        /* Hopefully, the server knows about it's own clients. */
        if (client_p == lclient_p)
	  continue;

        /* Ignore non lazylinks */
        if (!IsCapable(lclient_p,CAP_LL))
	  continue;

        /* Ignore servers we won't tell anyway */
        if (!chptr->lazyLinkChannelExists &
            (lclient_p->localClient->serverMask))
          continue;

        /* Ignore servers that already know target_p */
        if (!(target_p->lazyLinkClientExists &
	    lclient_p->localClient->serverMask))
        {
	  /* Tell LazyLink Leaf about client_p,
	   * as the leaf is about to get a SJOIN */
	  sendnick_TS(lclient_p, target_p);
          add_lazylinkclient(lclient_p,target_p);
        }
      }
    }

    if (!IsMember(target_p, chptr))
    {
      add_user_to_channel(chptr, target_p, fl);
      sendto_channel_local(ALL_MEMBERS, chptr, ":%s!%s@%s JOIN :%s",
                           target_p->name, target_p->username,
                           GET_CLIENT_HOST(target_p), parv[2]);
    }

#ifndef DIABLE_CHAN_OWNER
    if (fl & CHFL_CHANOWNER)
    {
      *mbuf++ = 'u';
      para[pargs++] = target_p->name;

      if (pargs >= MAXMODEPARAMS)
      {
        *mbuf = '\0';
        sendto_channel_local(ALL_MEMBERS, chptr, ":%s MODE %s %s %s %s %s %s",
                             source_p->name, chptr->chname, modebuf, para[0],
                             para[1], para[2], para[3]);
        mbuf = modebuf;
        *mbuf++ = '+';

        para[0] = para[1] = para[2] = para[3] = "";
        pargs = 0;
      }
    }
#endif
    if (fl & CHFL_CHANOP)
    {
      *mbuf++ = 'o';
      para[pargs++] = target_p->name;

      if (pargs >= MAXMODEPARAMS)
      {
        *mbuf = '\0';
        sendto_channel_local(ALL_MEMBERS, chptr, ":%s MODE %s %s %s %s %s %s",
                             source_p->name, chptr->chname, modebuf, para[0],
                             para[1], para[2], para[3]);
        mbuf = modebuf;
        *mbuf++ = '+';

        para[0] = para[1] = para[2] = para[3] = "";
        pargs = 0;
      }
    }
    if (fl & CHFL_VOICE)
    {
      *mbuf++ = 'v';
      para[pargs++] = target_p->name;

      if (pargs >= MAXMODEPARAMS)
      {
        *mbuf = '\0';
        sendto_channel_local(ALL_MEMBERS, chptr, ":%s MODE %s %s %s %s %s %s",
                             source_p->name, chptr->chname, modebuf, para[0],
                             para[1], para[2], para[3]);
        mbuf = modebuf;
        *mbuf++ = '+';

        para[0] = para[1] = para[2] = para[3] = "";
        pargs = 0;
      }
    }
    if (fl & CHFL_HALFOP)
    {
      *mbuf++ = 'h';
      para[pargs++] = target_p->name;

      if (pargs >= MAXMODEPARAMS)
      {
        *mbuf = '\0';
        sendto_channel_local(ALL_MEMBERS, chptr, ":%s MODE %s %s %s %s %s %s",
                             source_p->name, chptr->chname, modebuf, para[0],
                             para[1], para[2], para[3]);
        mbuf = modebuf;
        *mbuf++ = '+';

        para[0] = para[1] = para[2] = para[3] = "";
        pargs = 0;
      }
    }

nextnick:
    /* p points to the next nick */
    s = p;

    /* if there was a trailing space and p was pointing to it, then we
     * need to exit.. this has the side effect of breaking double spaces
     * in an sjoin.. but that shouldnt happen anyway
     */
    if (s && (*s == '\0'))
      s = p = NULL;

    /* if p was NULL due to no spaces, s wont exist due to the above, so
     * we cant check it for spaces.. if there are no spaces, then when
     * we next get here, s will be NULL
     */
    if (s != NULL && ((p = strchr(s, ' ')) != NULL))
      *p++ = '\0';
  }

  *mbuf = '\0';
  *(nick_ptr - 1) = '\0';
  *(uid_ptr - 1) = '\0';

  if (pargs != 0)
  {
    sendto_channel_local(ALL_MEMBERS, chptr, ":%s MODE %s %s %s %s %s %s",
                         source_p->name, chptr->chname, modebuf, para[0],
                         para[1], para[2], para[3]);
  }

  if (people == 0)
    return;

  if (parv[4 + args][0] == '\0')
    return;

  /* relay the SJOIN to other servers */
  DLINK_FOREACH(m, serv_list.head)
  {
    target_p = m->data;

    if (target_p == client_p)
      continue;

    if (IsCapable(target_p, CAP_TS6))
      sendto_one(target_p, "%s", uid_buf);
    else
      sendto_one(target_p, "%s", nick_buf);
  }
}

/* set_final_mode()
 *
 * inputs	- pointer to mode to setup
 *		- pointer to old mode
 * output	- NONE
 * side effects	- 
 */
static const struct mode_letter
{
  unsigned int mode;
  unsigned char letter;
} flags[] = {
  { MODE_NOPRIVMSGS, 'n' },
  { MODE_TOPICLIMIT, 't' },
  { MODE_SECRET,     's' },
  { MODE_MODERATED,  'm' },
  { MODE_INVITEONLY, 'i' },
  { MODE_PRIVATE,    'p' },
  { MODE_TOPICLOCK,  'T' },
  { MODE_NOCOLOR,    'c' },
  { MODE_NOTHROTTLE, 'z' },
  { 0, '\0' }
};

static void
set_final_mode(struct Mode *mode, struct Mode *oldmode)
{
  char *pbuf = parabuf;
  int what   = 0;
  int len;
  int i;

  for (i = 0; flags[i].letter; i++)
  {
    if ((flags[i].mode & mode->mode) &&
        !(flags[i].mode & oldmode->mode))
    {
      if (what != 1)
      {
        *mbuf++ = '+';
        what = 1;
      }
      *mbuf++ = flags[i].letter;
    }
  }

  for (i = 0; flags[i].letter; i++)
  {
    if ((flags[i].mode & oldmode->mode) &&
        !(flags[i].mode & mode->mode))
    {
      if (what != -1)
      {
        *mbuf++ = '-';
        what = -1;
      }
      *mbuf++ = flags[i].letter;
    }
  }

  if (oldmode->limit != 0 && mode->limit == 0)
  {
    if (what != -1)
    {
      *mbuf++ = '-';
      what = -1;
    }
    *mbuf++ = 'l';
  }

  if (oldmode->key[0] && !mode->key[0])
  {
    if (what != -1)
    {
      *mbuf++ = '-';
      what = -1;
    }
    *mbuf++ = 'k';
    len = ircsprintf(pbuf, "%s ", oldmode->key);
    pbuf += len;
    pargs++;
  }

  if (mode->limit != 0 && oldmode->limit != mode->limit)
  {
    if (what != 1)
    {
      *mbuf++ = '+';
      what = 1;
    }
    *mbuf++ = 'l';
    len = ircsprintf(pbuf, "%d ", mode->limit);
    pbuf += len;
    pargs++;
  }

  if (mode->key[0] && strcmp(oldmode->key, mode->key))
  {
    if (what != 1)
    {
      *mbuf++ = '+';
      what = 1;
    }
    *mbuf++ = 'k';
    len = ircsprintf(pbuf, "%s ", mode->key);
    pbuf += len;
    pargs++;
  }
  *mbuf = '\0';
}

/* remove_our_modes()
 *
 * inputs	- pointer to channel to remove modes from
 *		- client pointer
 * output	- NONE
 * side effects	- Go through the local members, remove all their
 *		  chanop modes etc., this side lost the TS.
 */
static void
remove_our_modes(struct Channel *chptr, struct Client *source_p)
{
  remove_a_mode(chptr, source_p, CHFL_CHANOP, 'o');
  remove_a_mode(chptr, source_p, CHFL_HALFOP, 'h');
  remove_a_mode(chptr, source_p, CHFL_VOICE, 'v');
#ifndef DISABLE_CHAN_OWNER
  remove_a_mode(chptr, source_p, CHFL_CHANOWNER, 'u');
#endif
}

/* remove_a_mode()
 *
 * inputs	-
 * output	- NONE
 * side effects	- remove ONE mode from a channel
 */
static
void remove_a_mode(struct Channel *chptr, struct Client *source_p,
                   int mask, char flag)
{
  dlink_node *ptr;
  struct Membership *ms;
  char lmodebuf[MODEBUFLEN];
  char *lpara[4];
  int count = 0;

  mbuf = lmodebuf;
  *mbuf++ = '-';

  lpara[0] = lpara[1] = lpara[2] = lpara[3] = "";

  DLINK_FOREACH(ptr, chptr->members.head)
  {
    ms = ptr->data;

    if ((ms->flags & mask) == 0)
      continue;

    ms->flags &= ~mask;

    lpara[count++] = ms->client_p->name;

    *mbuf++ = flag;

    if (count >= 4)
    {
      *mbuf = '\0';
      sendto_channel_local(ALL_MEMBERS, chptr,
		           ":%s MODE %s %s %s %s %s %s",
			   source_p->name,
			   chptr->chname, lmodebuf,
			   lpara[0], lpara[1], lpara[2], lpara[3]);
      mbuf = lmodebuf;
      *mbuf++ = '-';
      count = 0;
      lpara[0] = lpara[1] = lpara[2] = lpara[3] = "";
    }
  }

  if (count != 0)
  {
    *mbuf = '\0';
    sendto_channel_local(ALL_MEMBERS, chptr,
			 ":%s MODE %s %s %s %s %s %s",
			 source_p->name,
			 chptr->chname, lmodebuf,
			 lpara[0], lpara[1], lpara[2], lpara[3]);
  }
}

/* remove_ban_list()
 *
 * inputs	- channel, source, list to remove, char of mode, caps required
 * outputs	- none
 * side effects	- given ban list is removed, modes are sent to local clients and
 *		  non-ts6 servers linked through another uplink other than source_p
 */
static void
remove_ban_list(struct Channel *chptr, struct Client *source_p,
                dlink_list *list, char c, int cap)
{
  static char lmodebuf[MODEBUFLEN];
  static char lparabuf[BUFSIZE];
  struct Ban *banptr;    
  dlink_node *ptr;
  dlink_node *next_ptr;
  char *pbuf;
  int count = 0;      
  int cur_len, mlen, plen;

  pbuf = lparabuf;
  
  cur_len = mlen = ircsprintf(lmodebuf, ":%s MODE %s -",
            source_p->name, chptr->chname);
  mbuf = lmodebuf + mlen;
 
  DLINK_FOREACH_SAFE(ptr, next_ptr, list->head)
  {
    banptr = ptr->data;
 
    plen = strlen(banptr->banstr) + 1;
 
    if(count >= MAXMODEPARAMS || (cur_len + plen) > BUFSIZE - 4)
    {
      /* remove trailing space */
      *(pbuf - 1) = '\0';

      sendto_channel_local(ALL_MEMBERS, chptr, "%s %s",
               lmodebuf, lparabuf);
      sendto_server(source_p, NULL, chptr, cap, CAP_TS6, NOFLAGS,
		    "%s %s", lmodebuf, lparabuf);

      cur_len = mlen;
      mbuf = lmodebuf + mlen;
      pbuf = lparabuf;
      *mbuf = *pbuf = '\0';
      count = 0;
    }     

    *mbuf++ = c;
    cur_len += plen;
    pbuf += ircsprintf(pbuf, "%s ", banptr->banstr);
    count++;

    BlockHeapFree(ban_heap, banptr);
  }

  *(pbuf - 1) = *mbuf = '\0';
  sendto_channel_local(ALL_MEMBERS, chptr, "%s %s", lmodebuf, lparabuf);
  sendto_server(source_p, NULL, chptr, cap, CAP_TS6, NOFLAGS,
		"%s %s", lmodebuf, lparabuf);

  list->head = list->tail = NULL;
  list->length = 0;
}
