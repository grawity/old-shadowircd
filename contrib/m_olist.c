/*
 *  ircd-ratbox: A slightly useful ircd.
 *  m_olist.c: List channels.  olist is an oper only command
 *             that shows channels regardless of modes.
 *
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
 *  $Id: m_olist.c,v 1.1 2004/07/29 15:28:25 nenolod Exp $
 */

#include "stdinc.h"
#include "sprintf_irc.h"
#include "tools.h"
#include "handlers.h"
#include "channel.h"
#include "channel_mode.h"
#include "client.h"
#include "hash.h"
#include "irc_string.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "newconf.h"

static void mo_olist(struct Client*, struct Client*, int, char**);

struct Message list_msgtab = {
	"OLIST", 0, 0, 0, 0, MFLG_SLOW, 0,
	{m_unregistered, m_ignore, m_ignore, m_ignore, mo_olist}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
	mod_add_cmd(&list_msgtab);
}

void
_moddeinit(void)
{
	mod_del_cmd(&list_msgtab);
}
const char *_version = "$Revision: 1.1 $";
#endif

static void list_all_channels(struct Client *source_p);
static void list_named_channel(struct Client *source_p, char *name);

/*
** mo_olist
**      parv[0] = sender prefix
**      parv[1] = channel
*/
static void
mo_olist(struct Client *client_p, struct Client *source_p, int parc, char *parv[])
{
	if(IsOperSpy(source_p))
	{
		/* If no arg, do all channels *whee*, else just one channel */
		if(parc < 2 || EmptyString(parv[1]))
		{
			log_operspy(source_p, "LIST", NULL);
			list_all_channels(source_p);
		}
		else
		{
			log_operspy(source_p, "LIST", parv[1]);
			list_named_channel(source_p, parv[1]);
		}
	}

	sendto_one(source_p, form_str(RPL_LISTEND), me.name, source_p->name);
}

/*
 * list_all_channels
 * inputs	- pointer to client requesting list
 * output	- 0/1
 * side effects	- list all channels to source_p
 */
static void
list_all_channels(struct Client *source_p)
{
	struct Channel *chptr;
	dlink_node *ptr;

	sendto_one(source_p, form_str(RPL_LISTSTART), me.name, source_p->name);

	DLINK_FOREACH(ptr, global_channel_list.head)
	{
		chptr = ptr->data;

		sendto_one(source_p, form_str(RPL_LIST),
				me.name, source_p->name, chptr->chname,
				dlink_list_length(&chptr->members),
				chptr->topic == NULL ? "" : chptr->topic);
	}
}

/*
 * list_named_channel
 * inputs       - pointer to client requesting list
 * output       - 0/1
 * side effects	- list all channels to source_p
 */
static void
list_named_channel(struct Client *source_p, char *name)
{
	struct Channel *chptr;
	char *p;

	sendto_one(source_p, form_str(RPL_LISTSTART), me.name, source_p->name);

	if((p = strchr(name, ',')))
		*p = '\0';

	if(EmptyString(name))
	{
		sendto_one(source_p, form_str(ERR_NOSUCHCHANNEL), 
				me.name, source_p->name, name);
		return;
	}

	if((chptr = hash_find_channel(name)) == NULL)
		sendto_one(source_p, form_str(ERR_NOSUCHCHANNEL), 
				me.name, source_p->name, name);
	else
		sendto_one(source_p, form_str(RPL_LIST), me.name, source_p->name,
			chptr->chname, dlink_list_length(&chptr->members),
			chptr->topic ? chptr->topic : "");
}
