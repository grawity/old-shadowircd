/*
 *  shadowircd: An advanced IRC daemon.
 *  Borrowed from freeworld-ircd, and modified for shadowircd where appropriate.
 *  channel_filter.c: Controls channel filters.
 *
 *  Copyright (C) 2004 FreeWorld-ircd Development Team
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
 *  $Id: channel_filter.c,v 1.1 2004/06/09 21:07:27 nenolod Exp $
 */

#include "stdinc.h"
#include "setup.h"

#include "tools.h"
#include "channel.h"
#include "channel_mode.h"
#include "client.h"
#include "common.h"
#include "hash.h"
#include "irc_string.h"
#include "sprintf_irc.h"
#include "ircd.h"
#include "list.h"
#include "numeric.h"
#include "s_serv.h"             /* captab */
#include "s_user.h"
#include "send.h"
#include "whowas.h"
#include "s_conf.h"             /* ConfigFileEntry, ConfigChannel */
#include "event.h"
#include "memory.h"
#include "balloc.h"
#include "s_log.h"

extern BlockHeap *filter_heap;
static char buf[BUFSIZE];

int filter_add_word (struct Client *, struct Channel *, char *);
int filter_del_word (struct Channel *, char *);

int filter_add_word (struct Client *client_p, struct Channel *chptr, char *word)
{
	dlink_node *ptr;
	dlink_list *list;
	struct Filter *filter;

	if(MyClient(client_p))
	{
		collapse(word);
	}

	list = &chptr->filterlist;

	DLINK_FOREACH (ptr, list->head)
	{
		filter = ptr->data;
		if(match(filter->word, word))
		{
			sendto_realops_flags (UMODE_DEBUG, L_ALL, "Matching word, aborting addition.");
			return 0;
		}
	}

	/* Reject <censored> as one of the filter words
     * Warning recieved from Hwy
     */
	if (match(word, "<censored>"))
	{
		sendto_realops_flags (UMODE_DEBUG, L_ALL, "Forbidden filter word, aborting addition.");
		return 0;
	}

	filter = (struct Filter *) BlockHeapAlloc(filter_heap);
	DupString(filter->word, word);

	if(IsPerson(client_p))
	{
		filter->who = (char *) MyMalloc(strlen(client_p->name) + strlen(client_p->username) + strlen(client_p->host) + 3);
		ircsprintf (filter->who, "%s!%s@%s", client_p->name, client_p->username, client_p->host);
	}
	else
	{
		DupString (filter->who, client_p->name);
	}

	filter->when = CurrentTime;
	dlinkAdd(filter, &filter->node, list);

	sendto_realops_flags (UMODE_DEBUG, L_ALL, "Added word %s", word);

	return 1;
}

int filter_del_word (struct Channel *chptr, char *word)
{
	dlink_node *ptr;
	struct Filter *f;

	if(!word)
	{
		sendto_realops_flags (UMODE_DEBUG, L_ALL, "No word sent with MODE.");
		return 0;
	}

	sendto_realops_flags (UMODE_DEBUG, L_ALL, "Searching for %s", word);

	DLINK_FOREACH (ptr, chptr->filterlist.head)
	{
		f = ptr->data;

		if(irccmp(word, f->word) == 0)
		{
			MyFree(f->word);
			MyFree(f->who);
			BlockHeapFree(filter_heap, f);

			dlinkDelete(&f->node, &chptr->filterlist);
			return 1;
		}
	}

	sendto_realops_flags (UMODE_DEBUG, L_ALL, "Word wasn't found in the list.");
	return 0;
}

void free_filter_list (dlink_list * list)
{
	dlink_node *ptr, *next_ptr;
	struct Filter *f;

	sendto_realops_flags (UMODE_DEBUG, L_ALL, "Freeing filter list.");
	ilog (L_INFO, "Freeing filter list.");

	DLINK_FOREACH_SAFE (ptr, next_ptr, list->head)
	{
		f = ptr->data;
		MyFree(f->word);
		MyFree(f->who);
		BlockHeapFree(filter_heap, f);

		free_dlink_node(ptr);
	}

	list->head = list->tail = NULL;
	list->length = 0;
}

void
send_filter_list(struct Client *client_p, char *chname, dlink_list * top, char flag)
{
    dlink_node *lp;
	struct Filter *filter;
    char mbuf[MODEBUFLEN];
    char pbuf[BUFSIZE];
    int tlen;
    int mlen;
    int cur_len;
    char *mp;
    char *pp;
    int count = 0;

	sendto_realops_flags (UMODE_DEBUG, L_ALL, "Sending filter list.");
	ilog (L_INFO, "Sending filter list.");

    mlen = ircsprintf(buf, ":%s MODE %s +", me.name, chname);
    cur_len = mlen;

    mp = mbuf;
    pp = pbuf;

    DLINK_FOREACH(lp, top->head)
    {
        filter = lp->data;
        tlen = strlen(filter->word) + 3;

        /* uh oh */
        if(tlen > MODEBUFLEN)
            continue;

        if((count >= MAXMODEPARAMS) || ((cur_len + tlen + 2) > (BUFSIZE - 3)))
        {
            sendto_one(client_p, "%s%s %s", buf, mbuf, pbuf);

            mp = mbuf;
            pp = pbuf;
            cur_len = mlen;
            count = 0;
        }

        *mp++ = flag;
        *mp = '\0';
        pp += ircsprintf(pp, "%s ", filter->word);
        cur_len += tlen;
        count++;
    }

    if(count != 0)
        sendto_one(client_p, "%s%s %s", buf, mbuf, pbuf);
}
