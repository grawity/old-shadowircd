/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  m_intercept.c: View privmsg traffic not directed towards a channel.
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
 *  $Id: m_intercept.c,v 3.3 2004/09/08 01:18:06 nenolod Exp $
 *
 *  Updated to use the built in privmsg hook for ShadowIRCd 2.4.2! --neno
 */

/* !!! THIS MODULE REQUIRES 2.4-CVS TO WORK !!! - in3crypt */
#include "stdinc.h"
#include "handlers.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "common.h"
#include "s_misc.h"
#include "s_conf.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "channel.h"
#include "channel_mode.h"
#include "irc_string.h"
#include "hash.h"
#include "packet.h"
#include "hook.h"

int show_privmsg(struct hook_privmsg_data *);

/* You must edit the following for the module to work. */

/* target channel: the channel to send reports to! */
#define TARGET_CHANNEL "#pmspy"

/* format: how the data should be displayed */
#define MSGFORMAT ":%s PRIVMSG %s :\002%s\002: %s to %s -> %s"


/* DONT TOUCH ANYTHING BELOW THIS LINE! */
void
_modinit(void)
{
  hook_add_hook("send_privmsg", (hookfn *)show_privmsg);
}

void
_moddeinit(void)
{
  hook_del_hook("send_privmsg", (hookfn *)show_privmsg);
}

const char *_version = "$Revision: 3.3 $";

int
show_privmsg(struct hook_privmsg_data *data)
{
  struct Channel *acptr;
  struct Client *from_p;

  from_p = find_client(data->from);

  acptr = get_or_create_channel(from_p, TARGET_CHANNEL, NULL);

  sendto_realops_flags(UMODE_ALL, L_ALL, "debug: intercepted pm %s %s %s %s",
                 data->type, data->from, data->to, data->text);

  sendto_channel_local(ALL_MEMBERS, acptr, MSGFORMAT, me.name, TARGET_CHANNEL, 
                 data->type, data->from,
                 data->to, data->text);
 
  return 0;
}
