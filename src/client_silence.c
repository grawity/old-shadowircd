/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  client_silence.c: Controls silence lists.
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
 *  $Id: client_silence.c,v 1.4 2004/06/07 17:12:56 nenolod Exp $
 */

#include "stdinc.h"
#include "tools.h"
#include "client.h"
#include "channel_mode.h"
#include "common.h"
#include "event.h"
#include "fdlist.h"
#include "hash.h"
#include "irc_string.h"
#include "sprintf_irc.h"
#include "ircd.h"
#include "list.h"
#include "numeric.h"
#include "packet.h"
#include "s_auth.h"
#include "s_bsd.h"
#include "s_conf.h"
#include "s_log.h"
#include "s_misc.h"
#include "s_serv.h"
#include "send.h"
#include "whowas.h"
#include "s_user.h"
#include "dbuf.h"
#include "memory.h"
#include "hostmask.h"
#include "balloc.h"
#include "listener.h"
#include "irc_res.h"
#include "userhost.h"
                      
/* is_silenced()
 *
 * inputs       - pointer to target client
 *              - pointer to source client
 * output       - returns an int 0 if not banned,
 *                an int 1 if otherwise
 */
int
is_silenced(struct Client *target, struct Client *source)
{
  char src_host[NICKLEN + USERLEN + HOSTLEN + 6];
  char src_vhost[NICKLEN + USERLEN + HOSTLEN + 6];
  char src_iphost[NICKLEN + USERLEN + HOSTLEN + 6];

  if (!IsPerson(who))
    return(0);

  ircsprintf(src_host,"%s!%s@%s", source->name, source->username, source->host);
  ircsprintf(src_vhost,"%s!%s@%s", source->name, source->username, source->virthost);
  ircsprintf(src_iphost,"%s!%s@%s", source->name, source->username, source->ipaddr);

  return(check_silenced(target, src_host, src_vhost, src_iphost));
}

/* check_silenced()
 *
 * inputs 	- pointer to target client
 *		- three banstrings (hostname, vhost, ip)
 * output	- returns int 1 if on silence list, 0 if not
 */
static int
check_silenced(struct Client *target, const char *s, const char *s2, const char *s3)
{
  dlink_node *silence;
  struct Ban *actualSilence;

  DLINK_FOREACH(silence, target->silence_list.head)
  {
    actualSilence = silence->data;

    if (match(actualSilence->banstr,  s) ||
        match(actualSilence->banstr, s2) ||
        match(actualSilence->banstr, s3) ||
        match_cidr(actualSilence->banstr, s3))
      break;
    else
      actualBan = NULL;
  }

  return((actualSilence ? 1 : 0));
}

int
add_silence(struct Client *client_p, char *silenceid)
{
  dlink_node *silence;
  struct Ban *actualSilence;

  if (MyClient(client_p))
  {

    if (dlink_list_length(client_p->silence_list) >=
        ConfigFileEntry.max_silence)
    {
      sendto_one(client_p, form_str(ERR_SILENCELISTFULL),
                 me.name, client_p->name, chptr->chname, banid);
      return(0);
    }

    collapse(silenceid);
  }

  DLINK_FOREACH(ban, client_p->silence_list.head)
  {
    actualSilence = silence->data;

    if (match(actualSilence->banstr, silenceid))
      return(0);
  }

  actualSilence = (struct Ban *)MyMalloc(sizeof(struct Ban));
  memset(actualSilence, 0, sizeof(struct Ban));
  DupString(actualSilence->banstr, silenceid);

  DupString(actualSilence->who, client_p->name);

  actualSilence->when = CurrentTime;

  dlinkAdd(actualSilence, &actualSilence->node, client_p->silence_list);

  return(1);
}

int
del_silence(struct Client *client_p, char *banid)
{
  dlink_node *ban;
  struct Ban *banptr;
                                                                                                                                              
  if (banid == NULL)
    return(0);
                                                                                                                                              
  DLINK_FOREACH(ban, client_p->silence_list.head)
  {
    banptr = ban->data;
                                                                                                                                              
    if (irccmp(banid, banptr->banstr) == 0)
    {
      MyFree(banptr->banstr);
      MyFree(banptr->who);
      dlinkDelete(&banptr->node, client_p->silence_list);
      BlockHeapFree(ban_heap, banptr);
      return(1);
    }
  }
                                                                                                                                              
  return(0);
}

