/************************************************************************
 *   IRC - Internet Relay Chat, doc/example_module.c
 *   Copyright (C) 2001 Hybrid Development Team
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   $Id: m_svshost.c,v 1.1 2004/07/29 15:27:50 nenolod Exp $
 */

/* List of ircd includes from ../include/ */
#include "stdinc.h"
#include "handlers.h"
#include "client.h"
#include "common.h"     /* FALSE bleah */
#include "ircd.h"
#include "irc_string.h"
#include "numeric.h"
#include "fdlist.h"
#include "s_bsd.h"
#include "s_conf.h"
#include "s_log.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "hash.h"

static void ms_svshost	(struct Client *client_p, struct Client *source_p, int parc, char *parv[]);
static void me_svshost	(struct Client *client_p, struct Client *source_p, int parc, char *parv[]);

struct Message svshost_msgtab = {
	"SVSHOST", 0, 0, 4, 0, MFLG_SLOW, 0,
	{m_ignore, m_ignore, ms_svshost, ms_svshost, m_ignore}
};

#ifndef STATIC_MODULES
void _modinit (void)
{
	mod_add_cmd (&svshost_msgtab);
}

void _moddeinit (void)
{
	mod_del_cmd (&svshost_msgtab);
}

const char *_version = "$Revision 1.7 $ 10/06/04";
#endif

/*
 * mo_svshost
 *	parv[0] = sender prefix
 *      parv[1] = target server
 *	parv[2] = target nickname
 *	parv[3] = new hostname
 */

static void ms_svshost	(struct Client *client_p, struct Client *source_p, int parc, char *parv[])
{
	struct Client *target_p;
	int setflags;
	const char *nick;
	const char *hostname;
	char buffer[BUFSIZE];

	nick = parv[2];
	hostname = parv[3];

	propagate_generic (client_p, "SVSHOST", parv[1], CAP_IPMASK, "%s :%s", nick, hostname);

	/* Assume nick will be a valid client */
	target_p = find_client (nick);

	/* Assume all is good */
	strlcpy (target_p->host, hostname, HOSTLEN);

	sendto_match_servs (&me, "*", CAP_ENCAP, NOCAPS, "ENCAP * SNOTICE :Changed %s's hostname to %s",
				target_p->name, hostname);
	sendto_realops_flags (UMODE_SERVNOTICE, L_ALL, "Changed %s's hostname to %s", target_p->name, hostname);
}

/*
 * me_svshost
 *      parv[0] = sender prefix
 *	parv[1] = target server
 *      parv[2] = target nickname
 *      parv[3] = new hostname
 */

static void me_svshost	(struct Client *client_p, struct Client *source_p, int parc, char *parv[])
{
	struct Client *target_p;
	int setflags;
	const char *nick;
	const char *hostname;

	nick = parv[2];
	hostname = parv[3];

	propagate_generic (client_p, "SVSHOST", parv[1], CAP_IPMASK, "%s :%s", nick, hostname);

        /* Assume nick will be a valid client */
        target_p = find_client (nick);

        /* Assume all is good */
        strlcpy (target_p->host, hostname, HOSTLEN);

        sendto_match_servs (&me, "*", CAP_ENCAP, NOCAPS, "ENCAP * SNOTICE :Changed %s's hostname to %s",
                                target_p->name, hostname);
	sendto_realops_flags (UMODE_SERVNOTICE, L_ALL, "Changed %s's hostname to %s", target_p->name, hostname);
}
