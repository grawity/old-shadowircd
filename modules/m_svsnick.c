/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  m_svsnick.c: Server method to change a person's nick.
 *               Intended for services.
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
 *  $Id: m_svsnick.c,v 1.1 2003/12/12 17:58:42 nenolod Exp $
 */

#include "stdinc.h"
#include "handlers.h"
#include "client.h"
#include "hash.h"
#include "fdlist.h"
#include "irc_string.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_stats.h"
#include "s_user.h"
#include "hash.h"
#include "whowas.h"
#include "s_serv.h"
#include "send.h"
#include "list.h"
#include "channel.h"
#include "s_log.h"
#include "resv.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "common.h"
#include "packet.h"

static void ms_svsnick (struct Client *, struct Client *, int, char **);

static int clean_nick_name (char *);


struct Message svsnick_msgtab = {
  "SVSNICK", 0, 0, 3, 0, MFLG_SLOW, 0,
  {m_unregistered, m_ignore, ms_svsnick, m_ignore}
};

#ifndef STATIC_MODULES
void
_modinit (void)
{
  mod_add_cmd (&svsnick_msgtab);
}

void
_moddeinit (void)
{
  mod_del_cmd (&svsnick_msgtab);
}

const char *_version = "$Revision: 1.1 $";
#endif

/*
 * ms_svsnick()
 *
 *     parv[0] = sender prefix
 *     parv[1] = oldnick
 *     parv[2] = newnick
 */
static void
ms_svsnick (struct Client *client_p, struct Client *source_p,
	    int parc, char *parv[])
{
  char oldnick[NICKLEN];
  char newnick[NICKLEN];
  struct Client *oldnickname;
  struct Client *newnickname;

  if (!IsServer (source_p))
    return;

  if (parc < 3 || !parv[1] || !parv[2])
    {
      sendto_one (source_p, form_str (ERR_NONICKNAMEGIVEN), me.name, parv[0]);
      return;
    }

  /* terminate nick to NICKLEN */
  strlcpy (oldnick, parv[1], NICKLEN);
  strlcpy (newnick, parv[2], NICKLEN);

  if (!clean_nick_name (oldnick))
    {
      sendto_one (source_p, form_str (ERR_ERRONEUSNICKNAME),
		  me.name, parv[0], oldnick);
      return;
    }

  /* check the nickname is ok */
  if (!clean_nick_name (newnick))
    {
      sendto_one (source_p, form_str (ERR_ERRONEUSNICKNAME),
		  me.name, parv[0], newnick);
      return;
    }

  if (!(oldnickname = find_client (oldnick)))
    {
      sendto_one (source_p, form_str (ERR_NOSUCHNICK), me.name,
		  source_p->name, oldnick);
      return;
    }

  if ((newnickname = find_client (newnick)))
    {
      sendto_one (source_p, form_str (ERR_NICKNAMEINUSE), me.name,
		  parv[0], newnick);
      return;
    }

  if (MyConnect (oldnickname))
    {
      if (!IsFloodDone (oldnickname))
	flood_endgrace (oldnickname);

      if ((newnickname = find_client (oldnick)))
	{
	  if (newnickname == oldnickname)
	    {
	      /* check the nick isnt exactly the same */
	      if (strcmp (oldnickname->name, newnick))
		{
		  change_local_nick (oldnickname->servptr, oldnickname,
				     newnick);
		  return;
		}
	      else
		{
		  /* client is doing :old NICK old
		   * ignore it..
		   */
		  return;
		}
	    }
	  /* if the client that has the nick isnt registered yet (nick but no
	   * user) then drop the unregged client
	   */
	  if (IsUnknown (newnickname))
	    {
	      /* the old code had an if(MyConnect(target_p)) here.. but I cant see
	       * how that can happen, m_nick() is local only --fl_
	       */

	      exit_client (NULL, newnickname, &me, "Overridden");
	      change_local_nick (oldnickname->servptr, oldnickname, newnick);
	      return;
	    }
	  else
	    {
	      sendto_one (source_p, form_str (ERR_NICKNAMEINUSE), me.name,
			  parv[0], newnick);
	      return;
	    }
	}
      else
	{
	  if (!ServerInfo.hub && uplink && IsCapable (uplink, CAP_LL))
	    {
	      /* The uplink might know someone by this name already. */
	      sendto_one (uplink, ":%s NBURST %s %s %s", me.name, newnick,
			  newnick, oldnickname->name);
	      return;
	    }
	  else
	    {
	      change_local_nick (oldnickname->servptr, oldnickname, newnick);
	      return;
	    }
	}
    }
  else
    {
      sendto_server (client_p, source_p, NULL, NOCAPS, NOCAPS, LL_ICLIENT,
		     ":%s SVSNICK %s %s", me.name, oldnick, newnick);
      return;
    }
}

static int
clean_nick_name (char *nick)
{
  assert (nick);
  if (nick == NULL)
    return 0;

  /* nicks cant start with a digit or - or be 0 length */
  /* This closer duplicates behaviour of hybrid-6 */

  if (*nick == '-' || IsDigit (*nick) || *nick == '\0')
    return 0;

  for (; *nick; nick++)
    {
      if (!IsNickChar (*nick))
	return 0;
    }

  return 1;
}
