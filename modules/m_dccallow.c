/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  m_dccallow.c: DCCALLOW list manipulation.
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
 *  $Id: m_dccallow.c,v 3.3 2004/09/08 01:18:07 nenolod Exp $
 */

#include "stdinc.h"
#include "tools.h"
#include "common.h"  
#include "handlers.h"
#include "client.h"
#include "client_silence.h"
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

static void m_dccallow(struct Client*, struct Client*, int, char**);


struct Message dccallow_msgtab = {
  "DCCALLOW", 0, 0, 0, 0, MFLG_SLOW, 0L,
  {m_unregistered, m_dccallow, m_dccallow, m_dccallow, m_dccallow}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(&dccallow_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(&dccallow_msgtab);
}

const char *_version = "$Revision: 3.3 $";
#endif

/* m_dccallow
 *      parv[0] = sender prefix
 *      parv[1] = nickname masklist
 */
static void
m_dccallow(struct Client *client_p, struct Client *source_p,
        int parc, char *parv[])
{
  char c, *cp;
  dlink_node *node;
  struct Client *client;

  if (!MyClient(source_p))
    return;

  if (parc < 2) {
    DLINK_FOREACH(node, source_p->dccallow.head) {
      client = node->data;
      if (client)
        sendto_one(source_p, ":%s NOTICE %s :%s (%s@%s)",
                   me.name, source_p->name, client->name, client->username, GET_CLIENT_HOST(client));
    }
    return;
  } else {
    cp = parv[1];
    c = *cp;

    if (c == '+' || c == '-')
      cp++;
    else {
      sendto_one(source_p, ":%s NOTICE %s :Type /DCCALLOW [+|-]<mask> to add or delete entries from"
                    " your dccallow list.", me.name, source_p->name);
      return;
    }

    node = make_dlink_node();
    client = find_client(cp);

    if (c == '+')
      dlinkAddTail(client, node, &source_p->dccallow);
    else
      dlinkFindDelete(&source_p->dccallow, client);
  }
}
