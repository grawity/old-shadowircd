/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  m_grant.c: Grants operator privileges to a user.
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
 *  $Id: m_grant.c,v 1.1 2004/02/12 18:10:27 nenolod Exp $
 */

#include "stdinc.h"
#include "tools.h"
#include "handlers.h"
#include "client.h"
#include "common.h"
#include "fdlist.h"
#include "irc_string.h"
#include "ircd.h"
#include "numeric.h"
#include "fdlist.h"
#include "s_bsd.h"
#include "s_conf.h"
#include "s_log.h"
#include "s_user.h"
#include "send.h"
#include "list.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "packet.h"

static void mo_grant(struct Client*, struct Client*, int, char**);

struct Message grant_msgtab = {
  "GRANT", 0, 0, 3, 0, MFLG_SLOW, 0,
  {m_unregistered, m_ignore, m_ignore, mo_grant, m_ignore} 
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(&grant_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(&grant_msgtab);
}

const char *_version = "$Revision: 1.1 $";
#endif

/* this is a struct, associating operator permissions with letters. */
struct oper_flag_item
{
  const char *letter;
  const int flag;
};

static struct oper_flag_item oper_flags[] = {
  {"a", OPER_FLAG_AUSPEX},
  {"A", OPER_FLAG_ADMIN},
  {"i", OPER_FLAG_IMMUNE},
  {"v", OPER_FLAG_OVERRIDE},
  {"o", OPER_FLAG_OWNCLOAK},
  {"s", OPER_FLAG_SETCLOAK},
  {"r", OPER_FLAG_REHASH},
  {"d", OPER_FLAG_DIE},
  {"x", OPER_FLAG_X},
  {"k", OPER_FLAG_K},
  {"n", OPER_FLAG_N},
  {"u", OPER_FLAG_UNKLINE},
  {"R", OPER_FLAG_REMOTE},
  {"K", OPER_FLAG_GLOBAL_KILL},
  {"\0", 0}
};

/* these are support functions for grant, based on the ones I wrote for
 * shrike
 * --nenolod
 */

/*
 * string_to_bitmask
 * 
 * input        - character array of flags, client_p to assign
 *                them to
 * output       - none
 * side effects - client_p has permissions added to it
 */
static void string_to_bitmask(char *string, struct Client *target_p)
{
  struct oper_flag_item *flag;
  char *tmp;

  for (tmp = string; *tmp; tmp++)
    for (flag = flagtable; flag->letter; flag++)
      if (*flag->letter == *tmp)
      {
        target_p->localClient->operflags |= flag->bit;
        continue;
      }
}

/*
 * remove_from_bitmask
 *
 * input        - character array of flags, client_p to remove
 *                them from
 * output       - none
 * side effects - client_p has permissions removed from it
 */
static void string_to_bitmask(char *string, struct Client *target_p)
{
  struct oper_flag_item *flag;
  char *tmp;

  for (tmp = string; *tmp; tmp++)
    for (flag = flagtable; flag->letter; flag++)
      if (*flag->letter == *tmp)
      {
        target_p->localClient->operflags &~ flag->bit;
        continue;
      }
}

/* This is the code for the ircd. */

/*
 * mo_grant
 *      parv[0] = sender prefix
 *      parv[1] = subcommand (GIVE|TAKE|CLEAR)
 *      parv[2] = target
 *      parv[3] = flags as string
 */
static void
mo_grant(struct Client *client_p, struct Client *source_p,
       int parc, char *parv[])
{
  
}
