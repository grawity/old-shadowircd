/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  m_whois.c: Shows who a user is.
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
 *  $Id: m_whois.c,v 3.8 2004/09/25 17:12:14 nenolod Exp $
 */

#include "stdinc.h"
#include "tools.h"
#include "common.h"
#include "handlers.h"
#include "client.h"
#include "hash.h"
#include "channel.h"
#include "channel_mode.h"
#include "hash.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_serv.h"
#include "send.h"
#include "list.h"
#include "irc_string.h"
#include "sprintf_irc.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "hook.h"

/* Add the bahamut style WHOISACTUALLY to show the users real ip on
   remote (or local) WHOIS.
   Added 2002-07-15 by Gozem
   Updated 2003-01-17 by MeTaLRoCk
*/

static void do_whois (struct Client *client_p, struct Client *source_p,
		      int parc, char *parv[]);
static int single_whois (struct Client *source_p, struct Client *target_p,
			 int wilds, int glob);
static void whois_person (struct Client *source_p, struct Client *target_p,
			  int glob);

static void m_whois (struct Client *, struct Client *, int, char **);
static void mo_whois (struct Client *, struct Client *, int, char **);


struct Message whois_msgtab = {
  "WHOIS", 0, 0, 0, 0, MFLG_SLOW, 0L,
  {m_unregistered, m_whois, m_ignore, mo_whois, m_ignore}
};

#ifndef STATIC_MODULES
void
_modinit (void)
{
  hook_add_event ("doing_whois");
  mod_add_cmd (&whois_msgtab);
}

void
_moddeinit (void)
{
  hook_del_event ("doing_whois");
  mod_del_cmd (&whois_msgtab);
}

const char *_version = "$Revision: 3.8 $";
#endif

/* m_whois
 *      parv[0] = sender prefix
 *      parv[1] = nickname masklist
 */
static void
m_whois (struct Client *client_p, struct Client *source_p,
	 int parc, char *parv[])
{
  static time_t last_used = 0;

  if (parc < 2 || EmptyString (parv[1]))
    {
      sendto_one (source_p, form_str (ERR_NONICKNAMEGIVEN),
		  me.name, source_p->name);
      return;
    }

  if (parc > 2)
    {
      /* seeing as this is going across servers, we should limit it */
      if ((last_used + ConfigFileEntry.pace_wait_simple) > CurrentTime)
	{
	  if (MyClient (source_p))
	    sendto_one (source_p, form_str (RPL_LOAD2HI),
			me.name, source_p->name);
	  return;
	}
      else
	last_used = CurrentTime;

      if (hunt_server (client_p, source_p, ":%s WHOIS %s :%s", 1, parc, parv)
	  != HUNTED_ISME)
	return;

    }
  do_whois (client_p, source_p, parc, parv);
}

/* mo_whois
 *      parv[0] = sender prefix
 *      parv[1] = nickname masklist
 */
static void
mo_whois (struct Client *client_p, struct Client *source_p,
	  int parc, char *parv[])
{
  if (parc < 2 || EmptyString (parv[1]))
    {
      sendto_one (source_p, form_str (ERR_NONICKNAMEGIVEN),
		  me.name, source_p->name);
      return;
    }

  if (parc > 2)
    {
      if (hunt_server (client_p, source_p, ":%s WHOIS %s :%s", 1, parc, parv)
	  != HUNTED_ISME)
	return;
      parv[1] = parv[2];
    }
  do_whois (client_p, source_p, parc, parv);
}

/* do_whois()
 *
 * inputs	- pointer to 
 * output	- 
 * side effects -
 */
static void
do_whois (struct Client *client_p, struct Client *source_p,
	  int parc, char *parv[])
{
  struct Client *target_p;
  char *nick;
  char *p = NULL;
  int found = NO;
  int wilds;
  int glob = 0;

  /* This lets us make all "whois nick" queries look the same, and all
   * "whois nick nick" queries look the same.  We have to pass it all
   * the way down to whois_person() though -- fl */

  if (parc > 2)
    glob = 1;

  nick = parv[1];
  while (*nick == ',')
    nick++;

  if ((p = strchr (nick, ',')) != NULL)
    *p = '\0';

  if (*nick == '\0')
    return;

  collapse (nick);
  wilds = (strchr (nick, '?') || strchr (nick, '*'));

  if (!wilds)
    {
      if ((target_p = find_client (nick)) != NULL)
	{
	  /* im being asked to reply to a client that isnt mine..
	   * I cant answer authoritively, so better make it non-detailed
	   */
	  if (!MyClient (target_p))
	    glob = 0;

	  if (IsPerson (target_p))
	    {
	      (void) single_whois (source_p, target_p, wilds, glob);
	      found = YES;
	    }
	}
    }
  if (!found)
    sendto_one (source_p, form_str (ERR_NOSUCHNICK),
		me.name, source_p->name, nick);

  sendto_one (source_p, form_str (RPL_ENDOFWHOIS),
	      me.name, source_p->name, parv[1]);
}

/* single_whois()
 *
 * Inputs	- source_p client to report to
 *		- target_p client to report on
 *		- wilds whether wildchar char or not
 * Output	- if found return 1
 * Side Effects	- do a single whois on given client
 * 		  writing results to source_p
 */
static int
single_whois (struct Client *source_p, struct Client *target_p,
	      int wilds, int glob)
{
  dlink_node *ptr;
  struct Channel *chptr;
  const char *name;
  int invis;
  int member;
  int showperson;

  assert (target_p->user != NULL);

  if (target_p->name[0] == '\0')
    name = "?";
  else
    name = target_p->name;

  invis = IsInvisible (target_p);
  member = (target_p->user->channel.head != NULL) ? 1 : 0;
  showperson = (wilds && !invis && !member) || !wilds;

  DLINK_FOREACH (ptr, target_p->user->channel.head)
  {
    chptr = ((struct Membership *) ptr->data)->chptr;
    member = IsMember (source_p, chptr);
    if (invis && !member)
      continue;
    if (member || (!invis && PubChannel (chptr)))
      {
	showperson = 1;
	break;
      }
    if (!invis && HiddenChannel (chptr) && !SecretChannel (chptr))
      {
	showperson = 1;
	break;
      }
  }

  if (showperson)
    whois_person (source_p, target_p, glob);

  return (1);
}

/* whois_person()
 *
 * Inputs	- source_p client to report to
 *		- target_p client to report on
 * Output	- NONE
 * Side Effects	- 
 */
static void
whois_person (struct Client *source_p, struct Client *target_p, int glob)
{
  char buf[BUFSIZE];
  dlink_node *lp;
  struct Client *server_p;
  struct Channel *chptr;
  struct Membership *ms;
  int cur_len = 0;
  int mlen;
  char *t;
  int tlen;
  int reply_to_send = NO;
  struct hook_mfunc_data hd;

  assert (target_p->user != NULL);

  server_p = target_p->user->server;

  if (HasUmode(target_p, UMODE_WANTSWHOIS))
    sendto_one (target_p, ":%s NOTICE %s :*** Notice -- %s (%s@%s) is doing a /whois on you.",
                me.name, target_p->name, source_p->name, source_p->username, 
                GET_CLIENT_HOST(source_p));

  sendto_one (source_p, form_str (RPL_WHOISUSER), me.name,
	      source_p->name, target_p->name,
	      target_p->username, GET_CLIENT_HOST (target_p), target_p->info);

  ircsprintf (buf, form_str (RPL_WHOISCHANNELS),
	      me.name, source_p->name, target_p->name, "");

  mlen = strlen (buf);
  cur_len = mlen;
  t = buf + mlen;

  DLINK_FOREACH (lp, target_p->user->channel.head)
  {
    ms = lp->data;
    chptr = ms->chptr;

    if (ShowChannel (source_p, chptr))
      {
	if ((cur_len + strlen (chptr->chname) + 2) > (BUFSIZE - 4))
	  {
	    sendto_one (source_p, "%s", buf);
	    cur_len = mlen;
	    t = buf + mlen;
	  }

	ircsprintf (t, "%s%s ", get_member_status (ms, YES, 0), chptr->chname);

	tlen = strlen (t);
	t += tlen;
	cur_len += tlen;
	reply_to_send = YES;
      }
  }

  if (reply_to_send)
    sendto_one (source_p, "%s", buf);

  if (!IsOperAuspex(source_p))
    sendto_one (source_p, form_str (RPL_WHOISSERVER), me.name, source_p->name,
		target_p->name, ServerInfo.whois_hiding, 
                ServerInfo.whois_description);
  else
    sendto_one (source_p, form_str (RPL_WHOISSERVER), me.name, source_p->name,
		target_p->name, server_p->name, server_p->info);

  if (target_p->user->away)
    sendto_one (source_p, form_str (RPL_AWAY), me.name,
		source_p->name, target_p->name, target_p->user->away);

  if (IsIdentified(target_p))
    sendto_one (source_p, form_str (RPL_HASIDENTIFIED),
		me.name, source_p->name, target_p->name);

  if (IsOper (target_p))
    {
      if (!(HasUmode(target_p, UMODE_HIDEOPER)))
	{
          if (HasUmode(target_p, UMODE_NETADMIN))
            {
              sendto_one (source_p, form_str (RPL_WHOISOPERATOR),
                          me.name, source_p->name, target_p->name,
                          "an IRC Operator - Network Administrator");
            }
          else if (HasUmode(target_p, UMODE_TECHADMIN))
            {
              sendto_one (source_p, form_str (RPL_WHOISOPERATOR),
                         me.name, source_p->name, target_p->name,
                          "an IRC Operator - Technical Administrator");
            }
          else if (HasUmode(target_p, UMODE_ROUTING))
            {
              sendto_one (source_p, form_str (RPL_WHOISOPERATOR),
                         me.name, source_p->name, target_p->name,
                          "an IRC Operator - Routing Team Member");
            }
	  else if (HasUmode(target_p, UMODE_SERVICE))
	    {
	      sendto_one (source_p, form_str (RPL_WHOISOPERATOR),
			  me.name, source_p->name, target_p->name,
			  "a Network Service");
	    }
	  else if (HasUmode(target_p, UMODE_SVSROOT))
	    {
	      sendto_one (source_p, form_str (RPL_WHOISOPERATOR),
			  me.name, source_p->name, target_p->name,
			  "an IRC Operator - Services Root Administrator");
	    }
	  else if (HasUmode(target_p, UMODE_SVSADMIN))
	    {
	      sendto_one (source_p, form_str (RPL_WHOISOPERATOR),
			  me.name, source_p->name, target_p->name,
			  "an IRC Operator - Services Administrator");
	    }
	  else if (HasUmode(target_p, UMODE_ADMIN))
	    {
	      sendto_one (source_p, form_str (RPL_WHOISOPERATOR),
			  me.name, source_p->name, target_p->name,
			  "an IRC Operator - Server Administrator");
	    }
	  else if (HasUmode(target_p, UMODE_SVSOPER))
	    {
	      sendto_one (source_p, form_str (RPL_WHOISOPERATOR),
			  me.name, source_p->name, target_p->name,
			  "an IRC Operator - Services Operator");
	    }
	  else if (HasUmode(target_p, UMODE_OPER))
	    {
	      sendto_one (source_p, form_str (RPL_WHOISOPERATOR),
			  me.name, source_p->name, target_p->name,
			  "an IRC Operator");
	    }
	}
    }

  if (HasUmode(target_p, UMODE_HELPOP))
    sendto_one (source_p, form_str (RPL_WHOISHELPOP),
		me.name, source_p->name, target_p->name);

  if (IsOperAuspex (source_p))
    sendto_one (source_p, form_str (RPL_WHOISUMODES),
                me.name, source_p->name, target_p->name, 
                umodes_as_string(&target_p->umodes));

  if (HasUmode(target_p, UMODE_SENSITIVE))
    sendto_one (source_p, form_str (RPL_WHOISSENSITIVE),
                me.name, source_p->name, target_p->name);

  if (HasUmode(target_p, UMODE_SECURE))
    sendto_one (source_p, form_str (RPL_WHOISSECURE),
		me.name, source_p->name, target_p->name,
		"is using a secure connection (ssl)");

  if (target_p->swhois)
    sendto_one (source_p, form_str (RPL_WHOISSECURE),
                me.name, source_p->name, target_p->name,
                target_p->swhois);

  if (IsOperAuspex (source_p))
    sendto_one (source_p, form_str (RPL_WHOISACTUALLY),
                me.name, source_p->name, target_p->name,
                target_p->host, target_p->ipaddr);

  if (MyConnect (target_p))	/* Can't do any of this if not local! db */
    {
      if ((glob) || (MyClient (source_p)))
	{
	  sendto_one (source_p, form_str (RPL_WHOISIDLE),
		      me.name, source_p->name, target_p->name,
		      CurrentTime - target_p->user->last,
		      target_p->firsttime);
	}
    }

  hd.client_p = target_p;
  hd.source_p = source_p;
  /* although we should fill in parc and parv, we don't ..
   * be careful of this when writing whois hooks
   */
  hook_call_event ("doing_whois", &hd);
}

