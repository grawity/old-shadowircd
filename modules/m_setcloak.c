/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  m_setcloak.c: Cloaks a user.
 *
 *  Copyright (C) 2004 by the past and present ircd coders, and others.
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
 *  $Id: m_setcloak.c,v 1.3 2004/05/12 21:22:13 nenolod Exp $
 */

#include "stdinc.h"
#include "tools.h"
#include "common.h"
#include "handlers.h"
#include "client.h"
#include "channel.h"
#include "channel_mode.h"
#include "hash.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"
#include "send.h"
#include "list.h"
#include "irc_string.h"
#include "s_conf.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "whowas.h"		/* off_history */
#include "s_user.h"

void m_setcloak (struct Client *client_p, struct Client *source_p, int parc,
		 char *parv[]);

struct Message setcloak_msgtab = {
  "SETCLOAK", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_setcloak, m_setcloak, m_setcloak}
};

void
_modinit (void)
{
  mod_add_cmd (&setcloak_msgtab);
}

void
_moddeinit (void)
{
  mod_del_cmd (&setcloak_msgtab);
}

const char *_version = "$Revision: 1.3 $";


/* m_svscloak
 * parv[1] - Nick to cloak
 * parv[2] - Hostname to cloak to
 */

void
m_setcloak (struct Client *client_p, struct Client *source_p, int parc,
	    char *parv[])
{
  struct Client *target_p;
  char *hostname, *target;
  oper_flags old;

  if (parc < 3 || EmptyString (parv[2]))
    {
      sendto_one (source_p, form_str (ERR_NEEDMOREPARAMS), me.name, parv[0], "SETCLOAK");
      return;
    }
  target = parv[1];
  hostname = parv[2];

  if ((target_p = find_person (target)))
    {
      if (MyConnect (source_p))
	{
	  if (source_p == target_p)
	    {
	      if (!IsOperSetOwnCloak(source_p))
		{
		  return;
		}
	    }
	  else
	    {
	      if (!IsOperSetAnyCloak(source_p))
		{
		  return;
		}
	    }
	}
      sendto_server (client_p, NULL, NULL, NOCAPS, NOCAPS, NOFLAGS,
		     ":%s SVSCLOAK %s :%s", parv[0], parv[1], parv[2]);
      strncpy (target_p->virthost, hostname, HOSTLEN);
      old = target_p->umodes;
      SetUmode(target_p, UMODE_CLOAK);
      send_umode_out (target_p, target_p, old);
      target_p->flags |= FLAGS_USERCLOAK;	/* set the usercloak flag so that in the
						 * future, the server will burst the new vhost
						 * in case of netsplit.
						 *          --nenolod
						 */
      off_history (target_p);
    }
  else
    {
      sendto_one (source_p, form_str (ERR_NOSUCHNICK), me.name,
		  source_p->name, target);
      return;
    }
  return;
}
