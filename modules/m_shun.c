/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  m_shun.c: Makes a designated client captive
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
 *  $Id: m_shun.c,v 3.3 2004/09/08 01:18:07 nenolod Exp $
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

static void mo_shun(struct Client*, struct Client*, int, char**);


struct Message shun_msgtab = {
  "SHUN", 0, 0, 0, 0, MFLG_SLOW, 0L,
  {m_unregistered, m_ignore, m_ignore, mo_shun, m_ignore}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(&shun_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(&shun_msgtab);
}

const char *_version = "$Revision: 3.3 $";
#endif

/* mo_shun
 *      parv[0] = sender prefix
 *      parv[1] = nickname masklist
 */
static void
mo_shun(struct Client *client_p, struct Client *source_p,
        int parc, char *parv[])
{
  struct Client *target_p;

  if (parc < 2 || EmptyString(parv[1]))
  {
    sendto_one(source_p, form_str(ERR_NONICKNAMEGIVEN),
               me.name, source_p->name);
    return;
  }

  if ((target_p = find_client(parv[1])) != NULL)
  {
    if (MyClient(target_p))
    {
      target_p->handler = DUMMY_HANDLER;
      sendto_one(source_p, ":%s NOTICE %s :%s is now shunned",
		 me.name, source_p->name, parv[1]);
    }
  }
  else
    sendto_one(source_p, form_str(ERR_NOSUCHNICK),
               me.name, source_p->name, parv[1]);
}
