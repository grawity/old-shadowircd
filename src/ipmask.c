/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  FreeWorld-ircd:
 *  ipmask.c: IP Masking routines.
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
 *  $Id: ipmask.c,v 1.1 2004/07/29 15:27:30 nenolod Exp $
 */

#include "stdinc.h"
#include "tools.h"
#include "client.h"
#include "s_user.h"
#include "channel.h"
#include "channel_mode.h"
#include "class.h"
#include "common.h"
#include "fdlist.h"
#include "hash.h"
#include "irc_string.h"
#include "sprintf_irc.h"
#include "ircd.h"
#include "listener.h"
#include "motd.h"
#include "ircd_handler.h"
#include "msg.h"
#include "numeric.h"
#include "s_bsd.h"
#include "s_conf.h"
#include "s_log.h"
#include "s_serv.h"
#include "s_stats.h"
#include "scache.h"
#include "send.h"
#include "supported.h"
#include "whowas.h"
#include "memory.h"
#include "packet.h"

extern void make_virthost(char *, char *, char *);

char *ret_maskip(struct Client *client_p)
{
        static char rmask[HOSTLEN];

        rmask[0] = '\0';

	make_virthost (client_p->localClient->sockhost, client_p->host, rmask);

	if (rmask[0] != '\0')
		return rmask;
	else
		return client_p->rhost;
}
