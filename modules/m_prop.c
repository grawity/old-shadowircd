/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  m_prop.c: Used to modify channel settings.
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
 *  $Id: m_prop.c,v 1.3 2004/04/08 20:45:13 nenolod Exp $
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
#include <string.h>

struct Message prop_msgtab = {
  "PROP", 0, 0, 3, 0, MFLG_SLOW, 0,
  {m_unregistered, m_prop, m_prop, m_prop}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(&prop_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(&prop_msgtab);
}

const char *_version = "$Revision: 1.3 $";
#endif

/*
 * m_prop
 *  parv[0] = sender prefix
 *  parv[1] = channel name
 *  parv[2] = channel property
 *  parv[3] = extra params
 *  ..      = same
 */

static void
m_prop(struct Client *client_p, struct Client *source_p,
       int parc, char *parv[])
{
  struct Channel *chptr = NULL;
  struct Membership *ms;

  if (IsChanPrefix(*parv[1]))
  {
    if ((chptr = hash_find_channel(parv[1])) != NULL)
    {
      sendto_one(source_p, form_str(ERR_NOSUCHCHANNEL),
                 me.name, source_p->name, parv[1]);
      return;
    }
  }

  if ((ms = find_channel_link(source_p, chptr)) == NULL)
  {
    if (MyClient(source_p))
    {
      /*
       * If it's local, stop it, otherwise let it go. 
       */
      sendto_one(source_p,
                 form_str(ERR_NOTONCHANNEL), me.name, source_p->name, parv[1]);
      return;
    }
  }

  if (!has_member_flags(ms, (CHFL_CHANOP | CHFL_CHANOWNER)))
    return;
  if (!strcasecmp(parv[2], "ONJOIN"))
  {
    strcpy(chptr->properties.onjoin, parv[3]);
    if (MyClient(source_p))
      sendto_one(source_p, form_str(RPL_PROPERTYSET),
                 me.name, source_p->name, parv[1], parv[2], parv[3]);
  }

  if (!strcasecmp(parv[2], "DESCRIPTION"))
  {
    strcpy(chptr->properties.desc, parv[3]);
    if (MyClient(source_p))
      sendto_one(source_p, form_str(RPL_PROPERTYSET),
                 me.name, source_p->name, parv[1], parv[2], parv[3]);
  }

  if (!strcasecmp(parv[2], "URL"))
  {
    strcpy(chptr->properties.url, parv[3]);
    if (MyClient(source_p))
      sendto_one(source_p, form_str(RPL_PROPERTYSET),
                 me.name, source_p->name, parv[1], parv[2], parv[3]);
  }

  if (!strcasecmp(parv[2], "TOPIC"))
  {
    if (MyClient(source_p))
      sendto_one(source_p, form_str(RPL_PROPERTYSET),
                 me.name, source_p->name, parv[1], parv[2], parv[3]);
    sendto_channel_topic(chptr, parv[3], me.name, CurrentTime);
  }

  if (!strcasecmp(parv[2], "PASSWORD"))
  {
    if (!chptr->properties.passwd)
      strcpy(chptr->properties.passwd, parv[3]);
    else if (strcmp(parv[3], chptr->properties.passwd))
    {
      sendto_one(source_p,
                 form_str(ERR_PASSWORDINVALID),
                 me.name, source_p->name, parv[1], parv[3]);
      return;
    }
    else if (!strcmp(parv[3], chptr->properties.passwd))
    {
      strcpy(chptr->properties.passwd, parv[4]);
      if (MyClient(source_p))
        sendto_one(source_p, form_str(RPL_PROPERTYSET),
                   me.name, source_p->name, parv[1], parv[2], parv[4]);
    }
  }

  sendto_server(client_p, NULL, chptr, CAP_TS6,
                NOCAPS, NOFLAGS, ":%s PROP %s %s :%s",
                ID(source_p), chptr->chname, parv[2], parv[3]);
  sendto_server(client_p, NULL, chptr,
                NOCAPS, CAP_TS6, NOFLAGS,
                ":%s PROP %s %s :%s", source_p->name,
                chptr->chname, parv[2], parv[3]);
}
