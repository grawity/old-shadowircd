/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  spy_notices.c: Provides basic interception for +y users.
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
 *  $Id: spy_notices.c,v 1.1 2004/09/07 04:50:39 nenolod Exp $
 */
#include "stdinc.h"
#include "tools.h"
#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"

int show_admin(struct hook_spy_data *);
int show_info(struct hook_spy_data *);
int show_links(struct hook_links_data *);
int show_motd(struct hook_spy_data *);
int show_stats(struct hook_stats_data *);
int show_stats_p(struct hook_spy_data *);
int show_trace(struct hook_spy_data *);
int show_ltrace(struct hook_spy_data *);
int show_notice(struct hook_mfunc_data *);


void
_modinit(void)
{
  hook_add_hook("doing_admin", (hookfn *)show_admin);
  hook_add_hook("doing_info", (hookfn *)show_info);
  hook_add_hook("doing_links", (hookfn *)show_links);
  hook_add_hook("doing_motd", (hookfn *)show_motd);
  hook_add_hook("doing_stats", (hookfn *)show_stats);
  hook_add_hook("doing_stats_p", (hookfn *)show_stats_p);
  hook_add_hook("doing_trace", (hookfn *)show_trace);
  hook_add_hook("doing_ltrace", (hookfn *)show_ltrace);
}

void
_moddeinit(void)
{
  hook_del_hook("doing_admin", (hookfn *)show_admin);
  hook_del_hook("doing_info", (hookfn *)show_info);
  hook_del_hook("doing_links", (hookfn *)show_links);
  hook_del_hook("doing_motd", (hookfn *)show_motd);
  hook_del_hook("doing_stats", (hookfn *)show_stats);
  hook_del_hook("doing_stats_p", (hookfn *)show_stats_p);
  hook_del_hook("doing_trace", (hookfn *)show_trace);
  hook_del_hook("doing_ltrace", (hookfn *)show_ltrace);
}

const char *_version = "$Revision: 1.1 $";

int
show_admin(struct hook_spy_data *data)
{
  sendto_realops_flags(UMODE_SPY, L_ALL,
     "admin requested by %s (%s@%s) [%s]",
     data->source_p->name, data->source_p->username,
     GET_CLIENT_HOST(data->source_p), data->source_p->user->server->name);

  return 0;
}

int
show_info(struct hook_spy_data *data)
{
  sendto_realops_flags(UMODE_SPY, L_ALL,
     "info requested by %s (%s@%s) [%s]",
     data->source_p->name, data->source_p->username,
     GET_CLIENT_HOST(data->source_p), data->source_p->user->server->name);

  return 0;
}

int
show_links(struct hook_links_data *data)
{
  sendto_realops_flags(UMODE_SPY, L_ALL,
     "LINKS '%s' requested by %s (%s@%s) [%s]",
     data->mask, data->source_p->name, data->source_p->username,
     GET_CLIENT_HOST(data->source_p), data->source_p->user->server->name);

  return 0;
}

int
show_motd(struct hook_spy_data *data)
{
  sendto_realops_flags(UMODE_SPY, L_ALL,
     "MOTD requested by %s (%s@%s) [%s]",
     data->source_p->name, data->source_p->username,
     GET_CLIENT_HOST(data->source_p), data->source_p->user->server->name);

  return 0;
}

int
show_stats(struct hook_stats_data *data)
{
  if((data->statchar == 'L') || (data->statchar == 'l'))
    {
      if(data->name != NULL)
        sendto_realops_flags(UMODE_SPY, L_ALL,
                             "STATS %c requested by %s (%s@%s) [%s] on %s",
                             data->statchar,
                             data->source_p->name,
                             data->source_p->username,
                             GET_CLIENT_HOST(data->source_p),
                             data->source_p->user->server->name,
                             data->name);
      else
        sendto_realops_flags(UMODE_SPY, L_ALL,
                             "STATS %c requested by %s (%s@%s) [%s]",
                             data->statchar,
                             data->source_p->name,
                             data->source_p->username,
                             GET_CLIENT_HOST(data->source_p),
                             data->source_p->user->server->name);
    }
  else
    {
      sendto_realops_flags(UMODE_SPY, L_ALL,
                           "STATS %c requested by %s (%s@%s) [%s]",
                           data->statchar, data->source_p->name, data->source_p->username,
                           GET_CLIENT_HOST(data->source_p), data->source_p->user->server->name);
    }

  return 0;
}

int
show_stats_p(struct hook_spy_data *data)
{
  sendto_realops_flags(UMODE_SPY, L_ALL,
           "STATS p requested by %s (%s@%s) [%s]",
           data->source_p->name, data->source_p->username,
           GET_CLIENT_HOST(data->source_p), data->source_p->user->server->name);

  return 0;
}

int
show_trace(struct hook_spy_data *data)
{
  sendto_realops_flags(UMODE_SPY, L_ALL,
           "trace requested by %s (%s@%s) [%s]",
           data->source_p->name, data->source_p->username,
           GET_CLIENT_HOST(data->source_p), data->source_p->user->server->name);

  return 0;
}

int
show_ltrace(struct hook_spy_data *data)
{
  sendto_realops_flags(UMODE_SPY, L_ALL,
      "ltrace requested by %s (%s@%s) [%s]",
      data->source_p->name, data->source_p->username,
      GET_CLIENT_HOST(data->source_p), data->source_p->user->server->name);

  return 0;
}
