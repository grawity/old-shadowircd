/*
 *  ircd-ratbox: A slightly useful ircd.
 *  class.c: Controls connection classes.
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
 *  $Id: class.c,v 1.1 2004/07/29 15:27:25 nenolod Exp $
 */

#include "stdinc.h"
#include "config.h"

#include "tools.h"
#include "class.h"
#include "client.h"
#include "common.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "send.h"
#include "irc_string.h"
#include "s_debug.h"
#include "memory.h"
#include "patricia.h"

#define BAD_CONF_CLASS          -1
#define BAD_PING                -2
#define BAD_CLIENT_CLASS        -3

struct Class *ClassList;

struct Class *
make_class(void)
{
	struct Class *tmp;

	tmp = (struct Class *) MyMalloc(sizeof(struct Class));

	ConFreq(tmp) = DEFAULT_CONNECTFREQUENCY;
	PingFreq(tmp) = DEFAULT_PINGFREQUENCY;
	MaxUsers(tmp) = 1;
	MaxSendq(tmp) = DEFAULT_SENDQ;

#ifdef IPV6
        tmp->ip_limits = New_Patricia(128);
#else
        tmp->ip_limits = New_Patricia(32);
#endif
                	
	return tmp;
}

void
free_class(struct Class *tmp)
{
	Destroy_Patricia(tmp->ip_limits, NULL);
	MyFree(tmp->class_name);
	MyFree((char *) tmp);
}

/*
 * get_conf_ping
 *
 * inputs	- pointer to struct ConfItem
 * output	- ping frequency
 * side effects - NONE
 */
static int
get_conf_ping(struct ConfItem *aconf)
{
	if((aconf) && ClassPtr(aconf))
		return (ConfPingFreq(aconf));

	Debug((DEBUG_DEBUG, "No Ping For %s", (aconf) ? aconf->name : "*No Conf*"));

	return (BAD_PING);
}

/*
 * get_client_class
 *
 * inputs	- pointer to client struct
 * output	- pointer to name of class
 * side effects - NONE
 */
const char *
get_client_class(struct Client *target_p)
{
	dlink_node *ptr;
	struct ConfItem *aconf;
	const char *retc = "unknown";

	if(target_p && !IsMe(target_p) && (target_p->localClient->confs.head))
		DLINK_FOREACH(ptr, target_p->localClient->confs.head)
	{
		aconf = ptr->data;
		if(aconf->className == NULL)
			retc = "default";
		else
			retc = aconf->className;
	}

	Debug((DEBUG_DEBUG, "Returning Class %s For %s", retc, target_p->name));

	return (retc);
}

/*
 * get_client_ping
 *
 * inputs	- pointer to client struct
 * output	- ping frequency
 * side effects - NONE
 */
int
get_client_ping(struct Client *target_p)
{
	int ping = 0;
	int ping2;
	struct ConfItem *aconf;
	dlink_node *nlink;


	if(target_p->localClient->confs.head != NULL)
	{
		DLINK_FOREACH(nlink, target_p->localClient->confs.head)
		{
			aconf = nlink->data;
			if(aconf->status & CONF_CLIENT_MASK)
			{
				ping2 = get_conf_ping(aconf);
				if((ping2 != BAD_PING) && ((ping > ping2) || !ping))
					ping = ping2;
			}
		}
	}
	else
	{
		ping = DEFAULT_PINGFREQUENCY;
		Debug((DEBUG_DEBUG, "No Attached Confs"));
	}

	if(ping <= 0)
		ping = DEFAULT_PINGFREQUENCY;
	Debug((DEBUG_DEBUG, "Client %s Ping %d", target_p->name, ping));
	return (ping);
}

/*
 * get_con_freq
 *
 * inputs	- pointer to class struct
 * output	- connection frequency
 * side effects - NONE
 */
int
get_con_freq(struct Class *clptr)
{
	if(clptr)
		return (ConFreq(clptr));
	return (DEFAULT_CONNECTFREQUENCY);
}

/* add_class()
 *
 * input	- class to add
 * output	-
 * side effects - class is added to ClassList if new, else old class
 *                is updated with new values.
 */
void
add_class(struct Class *classptr)
{
	struct Class *tmpptr;

	tmpptr = find_class(classptr->class_name);

	if(tmpptr == ClassList)
	{
		classptr->next = tmpptr->next;
		tmpptr->next = classptr;
		CurrUsers(classptr) = 0;
	}
	else
	{
		MaxUsers(tmpptr) = MaxUsers(classptr);
		MaxLocal(tmpptr) = MaxLocal(classptr);
		MaxGlobal(tmpptr) = MaxGlobal(classptr);
		MaxIdent(tmpptr) = MaxIdent(classptr);
		PingFreq(tmpptr) = PingFreq(classptr);
		MaxSendq(tmpptr) = MaxSendq(classptr);
		MaxSendqEob(tmpptr) = MaxSendqEob(classptr);
		ConFreq(tmpptr) = ConFreq(classptr);
		CidrBitlen(tmpptr) = CidrBitlen(classptr);
		CidrAmount(tmpptr) = CidrAmount(classptr);

		free_class(classptr);
	}
}


/*
 * find_class
 *
 * inputs	- string name of class
 * output	- corresponding class pointer
 * side effects	- NONE
 */
struct Class *
find_class(const char *classname)
{
	struct Class *cltmp;

	if(classname == NULL)
	{
		return (ClassList);	/* return class 0 */
	}

	for (cltmp = ClassList; cltmp; cltmp = cltmp->next)
		if(!strcmp(ClassName(cltmp), classname))
			return cltmp;
	return ClassList;
}

/*
 * check_class
 *
 * inputs	- NONE
 * output	- NONE
 * side effects	- 
 */
void
check_class()
{
	struct Class *cltmp, *cltmp2;

	Debug((DEBUG_DEBUG, "Class check:"));

	for (cltmp2 = cltmp = ClassList; cltmp; cltmp = cltmp2->next)
	{
		if(MaxUsers(cltmp) < 0)
		{
			cltmp2->next = cltmp->next;
			if(CurrUsers(cltmp) <= 0)
				free_class(cltmp);
		}
		else
			cltmp2 = cltmp;
	}
}

/*
 * initclass
 *
 * inputs	- NONE
 * output	- NONE
 * side effects	- 
 */
void
initclass()
{
	ClassList = make_class();
	DupString(ClassName(ClassList), "default");
}

/*
 * report_classes
 *
 * inputs	- pointer to client to report to
 * output	- NONE
 * side effects	- class report is done to this client
 */
void
report_classes(struct Client *source_p)
{
	struct Class *cltmp;

	for (cltmp = ClassList; cltmp; cltmp = cltmp->next)
		sendto_one(source_p, form_str(RPL_STATSYLINE), me.name, source_p->name,
			   'Y', ClassName(cltmp), PingFreq(cltmp), ConFreq(cltmp),
			   MaxUsers(cltmp), MaxSendq(cltmp), MaxSendqEob(cltmp),
			   MaxLocal(cltmp), MaxIdent(cltmp), MaxGlobal(cltmp), MaxIdent(cltmp),
			   CurrUsers(cltmp));
}

/*
 * get_sendq
 *
 * inputs	- pointer to client
 * output	- sendq for this client as found from its class
 * side effects	- NONE
 */
long
get_sendq(struct Client *client_p)
{
	dlink_node *ptr;
	struct Class *cl;
	struct ConfItem *aconf;

	if(client_p && !IsMe(client_p) && (client_p->localClient->confs.head))
	{
		DLINK_FOREACH(ptr, client_p->localClient->confs.head)
		{
			aconf = ptr->data;
			if(aconf == NULL)
				continue;

			if(!(cl = aconf->c_class))
				continue;

			if(ClassName(cl))
			{
				if(HasSentEob(client_p) && MaxSendqEob(cl))
					return MaxSendqEob(cl);
				else
					return MaxSendq(cl);
			}
		}
	}

	return DEFAULT_SENDQ;
}
