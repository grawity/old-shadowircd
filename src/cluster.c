/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  cluster.c: Code for handling kline/dline/xline/resv clusters
 *
 *  Copyright (C) 2003 Hybrid Development Team
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
 *  $Id: cluster.c,v 1.1 2004/04/30 18:13:30 nenolod Exp $
 */

#include "cluster.h"
#include "tools.h"
#include "s_conf.h"
#include "s_serv.h"
#include "memory.h"
#include "irc_string.h"
#include "send.h"
#include "list.h"


void
cluster_kline (struct Client *source_p, int tkline_time, const char *user,
	       const char *host, const char *reason)
{
  sendto_match_servs (source_p, "*", CAP_KLN,
		      "KLINE * %d %s %s :%s",
		      tkline_time, user, host, reason);
}

void
cluster_unkline (struct Client *source_p, const char *user, const char *host)
{
  sendto_match_servs (source_p, "*", CAP_UNKLN,
		      "UNKLINE * %s %s", user, host);
}

void
cluster_xline (struct Client *source_p, const char *gecos,
	       int xtype, const char *reason)
{
  sendto_match_servs (source_p, "*", CAP_CLUSTER,
		      "XLINE * %s %d :%s", gecos, xtype, reason);
}

void
cluster_unxline (struct Client *source_p, const char *gecos)
{
  sendto_match_servs (source_p, "*", CAP_CLUSTER,
		      "UNXLINE * %s", gecos);
}

void
cluster_resv (struct Client *source_p, const char *name, const char *reason)
{
  sendto_match_servs (source_p, "*", CAP_CLUSTER,
		      "RESV * %s :%s", name, reason);
}

void
cluster_unresv (struct Client *source_p, const char *name)
{
  sendto_match_servs (source_p, "*", CAP_CLUSTER,
		      "UNRESV * %s", name);
}

/* remind me to take out locops... it's pointless. --nenolod */
void
cluster_locops (struct Client *source_p, const char *message)
{
  sendto_match_servs (source_p, "*", CAP_CLUSTER,
		      "LOCOPS * :%s", message);
}
