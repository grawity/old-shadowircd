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
 *  $Id: m_grant.c,v 1.8 2004/02/12 22:27:12 nenolod Exp $
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
#include "hash.h"

static void string_to_bitmask (char *, struct Client *);
static void remove_from_bitmask (char *, struct Client *);
static void mo_grant (struct Client *, struct Client *, int, char **);

struct Message grant_msgtab = {
  "GRANT", 0, 0, 3, 0, MFLG_SLOW, 0,
  {m_unregistered, m_ignore, m_ignore, mo_grant, m_ignore}
};

#ifndef STATIC_MODULES
void
_modinit (void)
{
  mod_add_cmd (&grant_msgtab);
}

void
_moddeinit (void)
{
  mod_del_cmd (&grant_msgtab);
}

const char *_version = "$Revision: 1.8 $";
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
  {NULL, 0}
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
static void
string_to_bitmask (char *string, struct Client *target_p)
{
  struct oper_flag_item *flag;
  char *tmp;

  for (tmp = string; *tmp; tmp++)
    for (flag = oper_flags; flag->letter; flag++)
      if (*flag->letter == *tmp)
	{
	  target_p->localClient->operflags |= flag->flag;
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
static void
remove_from_bitmask (char *string, struct Client *target_p)
{
  struct oper_flag_item *flag;
  char *tmp;

  for (tmp = string; *tmp; tmp++)
    for (flag = oper_flags; flag->letter; flag++)
      if (*flag->letter == *tmp)
	{
	  target_p->localClient->operflags &= ~flag->flag;
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
mo_grant (struct Client *client_p, struct Client *source_p,
	  int parc, char *parv[])
{
  struct Client *target_p;
  unsigned int old;
  dlink_node *dm;

  if (!strcasecmp (parv[1], "GIVE"))
    {
      if (!parv[2])
	{
	  sendto_one (source_p, ":%s NOTICE %s :Not enough parameters",
		      me.name, source_p->name);
	  return;
	}

      if (!(target_p = find_client (parv[2])))
	{
	  sendto_one (source_p, form_str (ERR_NOSUCHNICK), me.name,
		      source_p->name, parv[2]);
	  return;
	}

      if (!parv[3])
	{
	  sendto_one (source_p, ":%s NOTICE %s :Not enough parameters",
		      me.name, source_p->name);
	  return;
	}

      /* Since we're granting privs, lets make sure the user has umode +o. */
      if (!(target_p->umodes & UMODE_OPER))
	{
	  old = target_p->umodes;
	  SetOper (target_p);
	  target_p->umodes |= UMODE_HELPOP;

	  Count.oper++;

	  assert (dlinkFind (&oper_list, target_p) == NULL);
	  dlinkAdd (target_p, make_dlink_node (), &oper_list);

	  if (ConfigFileEntry.oper_umodes)
	    target_p->umodes |= ConfigFileEntry.oper_umodes & ALL_UMODES;
	  else
	    target_p->umodes |= (UMODE_SERVNOTICE | UMODE_OPERWALL |
				 UMODE_WALLOP | UMODE_LOCOPS) & ALL_UMODES;

	  sendto_realops_flags (UMODE_ALL, L_ALL,
				"%s (%s@%s) is now an operator",
				target_p->name, target_p->username,
				target_p->host);
	  sendto_one (target_p, form_str (RPL_YOUREOPER), me.name,
		      target_p->name);

	  /* Since we're becoming an oper, lets set the spoof. */
	  strncpy (target_p->virthost, ServerInfo.network_operhost, HOSTLEN);

	  sendto_server (NULL, target_p, NULL, NOCAPS, NOCAPS, NOFLAGS,
			 ":%s SVSCLOAK %s :%s", me.name, target_p->name,
			 target_p->virthost);
	}

      /* Ok, lets grant the privs now. */

      string_to_bitmask (parv[3], target_p);

      if (target_p->localClient->operflags & OPER_FLAG_ADMIN)
	target_p->umodes |= UMODE_ADMIN;

      send_umode_out (target_p, target_p, old);

      sendto_one (target_p,
		  ":%s NOTICE %s :*** Notice -- %s used GRANT to GIVE you operator permissions: %s",
		  me.name, target_p->name, source_p->name, parv[3]);

      return;
    }

  if (!strcasecmp (parv[1], "TAKE"))
    {
      if (!parv[2])
	{
	  sendto_one (source_p, ":%s NOTICE %s :Not enough parameters",
		      me.name, source_p->name);
	  return;
	}

      if (!(target_p = find_client (parv[2])))
	{
	  sendto_one (source_p, form_str (ERR_NOSUCHNICK), me.name,
		      source_p->name, parv[2]);
	  return;
	}

      if (!parv[3])
	{
	  sendto_one (source_p, ":%s NOTICE %s :Not enough parameters",
		      me.name, source_p->name);
	  return;
	}

      /* Ok, lets grant the privs now. */

      remove_from_bitmask (parv[3], target_p);

      if (target_p->localClient->operflags == 0)
	{
	  old = target_p->umodes;
	  ClearOper (target_p);
	  target_p->umodes &= ~UMODE_HELPOP;

	  Count.oper--;

	  if ((dm = dlinkFindDelete (&oper_list, target_p)) != NULL)
	    free_dlink_node (dm);

	  if (ConfigFileEntry.oper_umodes)
	    target_p->umodes &= ~(ConfigFileEntry.oper_umodes & ALL_UMODES);
	  else
	    target_p->umodes &= ~((UMODE_SERVNOTICE | UMODE_OPERWALL |
				   UMODE_WALLOP | UMODE_LOCOPS) & ALL_UMODES);

	  /* Restore their spoof. */
	  make_virthost (target_p->host, target_p->virthost);

	  sendto_server (NULL, target_p, NULL, NOCAPS, NOCAPS, NOFLAGS,
			 ":%s SVSCLOAK %s :%s", me.name, target_p->name,
			 target_p->virthost);
	}

      send_umode_out (target_p, target_p, old);

      sendto_one (target_p,
		  ":%s NOTICE %s :*** Notice -- %s used GRANT to TAKE operator permissions: %s",
		  me.name, target_p->name, source_p->name, parv[3]);

      return;
    }

  if (!strcasecmp (parv[1], "CLEAR"))
    {
      if (!parv[2])
	{
	  sendto_one (source_p, ":%s NOTICE %s :Not enough parameters",
		      me.name, source_p->name);
	  return;
	}

      if (!(target_p = find_client (parv[2])))
	{
	  sendto_one (source_p, form_str (ERR_NOSUCHNICK), me.name,
		      source_p->name, parv[2]);
	  return;
	}

      ClearOperFlags (target_p);

      Count.oper--;

      old = target_p->umodes;
      ClearOper (target_p);
      target_p->umodes &= ~UMODE_HELPOP;

      if ((dm = dlinkFindDelete (&oper_list, target_p)) != NULL)
	free_dlink_node (dm);

      if (ConfigFileEntry.oper_umodes)
	target_p->umodes &= ~(ConfigFileEntry.oper_umodes & ALL_UMODES);
      else
	target_p->umodes &= ~((UMODE_SERVNOTICE | UMODE_OPERWALL |
			       UMODE_WALLOP | UMODE_LOCOPS) & ALL_UMODES);

      /* Restore their spoof. */
      make_virthost (target_p->host, target_p->virthost);

      sendto_server (NULL, target_p, NULL, NOCAPS, NOCAPS, NOFLAGS,
		     ":%s SVSCLOAK %s :%s", me.name, target_p->name,
		     target_p->virthost);

      send_umode_out (target_p, target_p, old);

      sendto_one (target_p,
		  ":%s NOTICE %s :*** Notice -- %s used GRANT to TAKE operator permissions: %s",
		  me.name, target_p->name, source_p->name, parv[3]);

      return;
    }

  sendto_one (source_p,
	      ":%s NOTICE %s :*** Syntax: /GRANT [GIVE|TAKE|CLEAR] <nick> <flags>",
	      me.name, source_p->name);

}
