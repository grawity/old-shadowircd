/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  m_svskill.c: Server method to kill silently.
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
 *  $Id: m_svskill.c,v 1.2 2004/07/29 18:07:56 nenolod Exp $
 */

#include "stdinc.h"
#include "handlers.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "sprintf_irc.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "hash.h"
#include "modules.h"
#include "s_conf.h"

static void m_svskill (struct Client *, struct Client *, int, char **);

struct Message svskill_msgtab = {
  "SVSKILL", 0, 0, 0, 0, MFLG_SLOW | MFLG_UNREG, 0,
  {m_unregistered, m_ignore, m_svskill, m_ignore, m_ignore}
};

#ifndef STATIC_MODULES
void
_modinit (void)
{
  mod_add_cmd (&svskill_msgtab);
}

void
_moddeinit (void)
{
  mod_del_cmd (&svskill_msgtab);
}

const char *_version = "$Revision: 1.2 $";
#endif

/*
** m_svskill
**      parv[0] = sender prefix
**      parv[1] = person to disconnect
**      parv[2] = reason
*/
static void
m_svskill (struct Client *client_p, struct Client *source_p,
	   int parc, char *parv[])
{
  struct Client *target_p;
  dlink_node *server_node;
  char reason[TOPICLEN + 1];

  if (!IsServer (source_p))
    return;

  if ((target_p = (struct Client *) find_client (parv[1])) == NULL)
    return;

  ircsprintf (reason, "%s", parv[2]);

  SetKilled(target_p);

  exit_client (client_p, target_p, target_p, reason);

  /* Propigate kill attempts. */
  sendto_server (source_p, NULL, NOCAPS, NOCAPS,
                 ":%s SVSKILL %s :%s", parv[0], parv[1], parv[2]);         
}
