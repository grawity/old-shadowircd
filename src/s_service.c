/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  s_service.c: Support for IRC 2.9/2.10/2.11 services clients
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
 *  $Id: s_service.c,v 1.1 2004/02/13 18:23:25 nenolod Exp $
 */

#include "client.h"
#include "hash.h"
#include "channel.h"

/*
 * burst_services
 *
 * input        - target_p to send services burst to
 * output       - none
 * side effects - services are sent to the new server
 */
void burst_services(struct Client *target_p)
{
  dlink_node *ptr;
  struct Client *service_p;

  DLINK_FOREACH(ptr, global_client_list.head)
  {
    service_p = ptr->data;
    if (IsService(service_p))
    {
      sendto_one(target_p, "SSERVICE %s %lu %d %s %s %s :%s",
		target_p->name, target_p->tsinfo, target_p->hopcount + 1,
		target_p->username, target_p->host,
		target_p->service->server->name, target_p->info);
    }
  }
}

