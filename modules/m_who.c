/*
 *  ircd-ratbox: an advanced Internet Relay Chat Daemon(ircd).
 *  m_who.c: Shows who is on a channel.
 *
 *  Copyright (C) 1990 Jarkko Oikarinen and University of Oulu, Co Center
 *  Copyright (C) 1996-2002 Hybrid Development Team
 *  Copyright (C) 2002-2004 ircd-ratbox development team
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
 *  $Id: m_who.c,v 1.1 2004/07/29 15:27:34 nenolod Exp $
 */
#include "stdinc.h"
#include "tools.h"
#include "common.h"
#include "handlers.h"
#include "client.h"
#include "channel.h"
#include "channel_mode.h"
#include "hash.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"
#include "send.h"
#include "irc_string.h"
#include "sprintf_irc.h"
#include "s_conf.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "packet.h"
#include "s_log.h"

static void m_who(struct Client *, struct Client *, int, char **);

struct Message who_msgtab = {
	"WHO", 0, 0, 2, 0, MFLG_SLOW, 0,
	{m_unregistered, m_who, m_ignore, m_ignore, m_who}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
	mod_add_cmd(&who_msgtab);
}

void
_moddeinit(void)
{
	mod_del_cmd(&who_msgtab);
}
const char *_version = "$Revision: 1.1 $";
#endif
static void do_who_on_channel(struct Client *source_p, struct Channel *chptr,
			      int server_oper, int member);

static void who_global(struct Client *source_p, char *mask, int server_oper, int);

static void do_who(struct Client *source_p, struct Client *target_p, const char *chname, const char *op_flags);


/*
** m_who
**      parv[0] = sender prefix
**      parv[1] = nickname mask list
**      parv[2] = additional selection flag, only 'o' for now.
*/
static void
m_who(struct Client *client_p, struct Client *source_p, int parc, char *parv[])
{
	static time_t last_used = 0;
	struct Client *target_p;
	struct membership *msptr;
	char *mask = parv[1];
	dlink_node *lp;
	struct Channel *chptr = NULL;
	int server_oper = parc > 2 ? (*parv[2] == 'o') : 0;	/* Show OPERS only */
	int member;
	int whois_cheat = 0;

	/* See if mask is there, collapse it or return if not there */
	collapse(mask);

	if(*mask == '\0')
	{
		sendto_one(source_p, form_str(RPL_ENDOFWHO), me.name, parv[0], "*");
		return;
	}

	/* '/who *' */
	if((*(mask + 1) == (char) 0) && (*mask == '*'))
	{
		if(source_p->user == NULL)
			return;

		if((lp = source_p->user->channel.head) != NULL)
		{
				msptr = lp->data;
				do_who_on_channel(source_p, msptr->chptr, server_oper, YES);
		}

		sendto_one(source_p, form_str(RPL_ENDOFWHO), me.name, parv[0], "*");
		return;
	}

	if(IsOperSpy(source_p) && *mask == '!')
	{
		mask++;
		whois_cheat = 1;

		if(EmptyString(mask))
		{
			sendto_one(source_p, form_str(RPL_ENDOFWHO), 
				   me.name, parv[0], parv[1]);
			return;
		}
			
	}

	/* '/who #some_channel' */
	if(IsChannelName(mask))
	{
		/*
		 * List all users on a given channel
		 */
		chptr = hash_find_channel(mask);
		if(chptr != NULL)
		{
			if(whois_cheat)
				log_operspy(source_p, "WHO", chptr->chname);
			if(IsMember(source_p, chptr) || whois_cheat)
				do_who_on_channel(source_p, chptr, server_oper, YES);
			else if(!SecretChannel(chptr))
				do_who_on_channel(source_p, chptr, server_oper, NO);
		}
		sendto_one(source_p, form_str(RPL_ENDOFWHO), me.name, parv[0], mask);
		return;
	}

	/* '/who nick' */

	if(((target_p = find_client(mask)) != NULL) &&
	   IsPerson(target_p) && (!server_oper || IsOper(target_p)))
	{
		int isinvis = 0;

		isinvis = IsInvisible(target_p);
		DLINK_FOREACH(lp, target_p->user->channel.head)
		{
			msptr = lp->data;
			chptr = msptr->chptr;

			member = IsMember(source_p, chptr);
			if(isinvis && !member)
				continue;

			if(member || (!isinvis && PubChannel(chptr)))
			{
				break;
			}
		}

		if(lp != NULL)
			do_who(source_p, target_p, chptr->chname,
			       find_channel_status(lp->data, 0));
		else
			do_who(source_p, target_p, NULL, "");

		sendto_one(source_p, form_str(RPL_ENDOFWHO), me.name, parv[0], mask);
		return;
	}

	if(!IsFloodDone(source_p))
		flood_endgrace(source_p);

	/* at this point its either /who 0, or a global who, so limit */
	if(!IsOper(source_p))
	{
		if((last_used + ConfigFileEntry.pace_wait) > CurrentTime)
		{
			sendto_one(source_p, form_str(RPL_LOAD2HI),
					me.name, source_p->name, "WHO");
			sendto_one(source_p, form_str(RPL_ENDOFWHO), me.name, parv[0], "*");
			return;
		}
		else
			last_used = CurrentTime;
	}

	/* '/who 0' */
	if((*(mask + 1) == '\0') && (*mask == '0'))
		who_global(source_p, NULL, server_oper, 0);
	else
		who_global(source_p, mask, server_oper, whois_cheat);

	/* Wasn't a nick, wasn't a channel, wasn't a '*' so ... */
	sendto_one(source_p, form_str(RPL_ENDOFWHO), me.name, parv[0], mask);
}

/* who_common_channel
 * inputs	- pointer to client requesting who
 * 		- pointer to channel member chain.
 *		- char * mask to match
 *		- int if oper on a server or not
 *		- pointer to int maxmatches
 * output	- NONE
 * side effects - lists matching clients on specified channel,
 * 		  marks matched clients.
 *
 */
static void
who_common_channel(struct Client *source_p, struct Channel *chptr,
		   char *mask, int server_oper, int *maxmatches)
{
	struct membership *msptr;
	struct Client *target_p;
	dlink_node *ptr;

	DLINK_FOREACH(ptr, chptr->members.head)
	{
		msptr = ptr->data;
		target_p = msptr->client_p;

		if(!IsInvisible(target_p) || IsMarked(target_p))
			continue;

		if(server_oper && !IsOper(target_p))
			continue;

		SetMark(target_p);

		if((mask == NULL) ||
		   match(mask, target_p->name) || match(mask, target_p->username) ||
		   match(mask, target_p->host) ||
		   (match(mask, target_p->user->server) &&
		    (IsOper(source_p) || !ConfigServerHide.hide_servers)) ||
		   match(mask, target_p->info))
		{

			do_who(source_p, target_p, NULL, "");

			if(*maxmatches > 0)
			{
				--(*maxmatches);
				if(*maxmatches == 0)
					return;
			}

		}
	}
}

/*
 * who_global
 *
 * inputs	- pointer to client requesting who
 *		- char * mask to match
 *		- int if oper on a server or not
 * output	- NONE
 * side effects - do a global scan of all clients looking for match
 *		  this is slightly expensive on EFnet ...
 */
static void
who_global(struct Client *source_p, char *mask, int server_oper, int whois_cheat)
{
	struct Client *target_p;
	struct membership *msptr;
	dlink_node *lp, *ptr;
	int maxmatches = 500;

	if(!whois_cheat)
	{
		/* first, list all matching INvisible clients on common channels */
		DLINK_FOREACH(lp, source_p->user->channel.head)
		{
			msptr = lp->data;
			who_common_channel(source_p, msptr->chptr, mask, server_oper, &maxmatches);
		}
	}
	else
		log_operspy(source_p, "WHO", mask);

	/* second, list all matching visible clients */
	DLINK_FOREACH(ptr, global_client_list.head)
	{
		target_p = (struct Client *) ptr->data;
		if(!IsPerson(target_p))
			continue;

		if(IsInvisible(target_p) && !whois_cheat)
		{
			ClearMark(target_p);
			continue;
		}

		if(server_oper && !IsOper(target_p))
			continue;

		if(!mask ||
		   match(mask, target_p->name) || match(mask, target_p->username) ||
		   match(mask, target_p->host) || match(mask, target_p->user->server) ||
		   match(mask, target_p->info))
		{

			do_who(source_p, target_p, NULL, "");
			if(maxmatches > 0)
			{
				--maxmatches;
				if(maxmatches == 0)
					return;
			}

		}

	}
}


/*
 * do_who_on_channel
 *
 * inputs	- pointer to client requesting who
 *		- pointer to channel to do who on
 *		- The "real name" of this channel
 *		- int if source_p is a server oper or not
 *		- int if client is member or not
 * output	- NONE
 * side effects - do a who on given channel
 */
static void
do_who_on_channel(struct Client *source_p, struct Channel *chptr, 
		  int server_oper, int member)
{
	struct Client *target_p;
	struct membership *msptr;
	dlink_node *ptr;

	DLINK_FOREACH(ptr, chptr->members.head)
	{
		msptr = ptr->data;
		target_p = msptr->client_p;

		if(server_oper && !IsOper(target_p))
			continue;

		if(member || !IsInvisible(target_p))
			do_who(source_p, target_p, chptr->chname,
			       find_channel_status(msptr, 0));
	}
}

/*
 * do_who
 *
 * inputs	- pointer to client requesting who
 *		- pointer to client to do who on
 *		- The reported name
 *		- channel flags
 * output	- NONE
 * side effects - do a who on given person
 */

static void
do_who(struct Client *source_p, struct Client *target_p, const char *chname, const char *op_flags)
{
	char status[5];

	ircsprintf(status, "%c%s%s",
		   target_p->user->away ? 'G' : 'H', IsOper(target_p) ? "*" : "", op_flags);

	sendto_one(source_p, form_str(RPL_WHOREPLY), me.name, source_p->name,
		   (chname) ? (chname) : "*",
		   target_p->username,
		   target_p->host, target_p->user->server, target_p->name,
		   status, target_p->hopcount, target_p->info);
}
