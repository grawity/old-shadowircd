/*
 *  ircd-ratbox: an advanced Internet Relay Chat Daemon(ircd).
 *  m_motd.c: Shows the current message of the day.
 *
 *  Copyright (C) 1990 Jarkko Oikarinen and University of Oulu, Co Center
 *  Copyright (C) 1996-2002 Hybrid Development Team
 *  Copyright (C) 2002-2004 ircd-ratbox development team
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
 *  $Id: m_motd.c,v 1.1 2004/07/29 15:27:40 nenolod Exp $
 */

#include "stdinc.h"
#include "client.h"
#include "tools.h"
#include "motd.h"
#include "ircd.h"
#include "send.h"
#include "numeric.h"
#include "handlers.h"
#include "hook.h"
#include "msg.h"
#include "s_serv.h"		/* hunt_server */
#include "parse.h"
#include "modules.h"
#include "s_conf.h"


static void m_motd(struct Client *, struct Client *, int, char **);
static void mo_motd(struct Client *, struct Client *, int, char **);

struct Message motd_msgtab = {
	"MOTD", 0, 0, 0, 1, MFLG_SLOW, 0,
	{m_unregistered, m_motd, mo_motd, m_ignore, mo_motd}
};
#ifndef STATIC_MODULES
void
_modinit(void)
{
	mod_add_cmd(&motd_msgtab);
}

void
_moddeinit(void)
{
	mod_del_cmd(&motd_msgtab);
}

const char *_version = "$Revision: 1.1 $";
#endif

/*
** m_motd
**      parv[0] = sender prefix
**      parv[1] = servername
*/
static void
m_motd(struct Client *client_p, struct Client *source_p, int parc, char *parv[])
{
	static time_t last_used = 0;

	if((last_used + ConfigFileEntry.pace_wait) > CurrentTime)
	{
		/* safe enough to give this on a local connect only */
		sendto_one(source_p, form_str(RPL_LOAD2HI),
			   me.name, source_p->name, "MOTD");
		sendto_one(source_p, form_str(RPL_ENDOFMOTD),
			me.name, source_p->name);
		return;
	}
	else
		last_used = CurrentTime;

	/* This is safe enough to use during non hidden server mode */
	if(!ConfigServerHide.disable_remote && !ConfigServerHide.hide_servers)
	{
		if(hunt_server(client_p, source_p, ":%s MOTD :%s", 1, parc, parv) != HUNTED_ISME)
			return;
	}

	hook_call_event(h_doing_motd_id, source_p);

	SendMessageFile(source_p, &ConfigFileEntry.motd);
}

/*
** mo_motd
**      parv[0] = sender prefix
**      parv[1] = servername
*/
static void
mo_motd(struct Client *client_p, struct Client *source_p, int parc, char *parv[])
{
	if(!IsClient(source_p))
		return;

	if(hunt_server(client_p, source_p, ":%s MOTD :%s", 1, parc, parv) != HUNTED_ISME)
		return;

	hook_call_event(h_doing_motd_id, source_p);

	SendMessageFile(source_p, &ConfigFileEntry.motd);
}
