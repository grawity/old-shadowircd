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
 *  $Id: channel_filter.c,v 1.5 2004/08/21 08:15:38 nenolod Exp $
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

int filter_add_word (char *);
int filter_del_word (char *);

int filter_add_word (char *word)
{
	dlink_node *ptr;
	dlink_list *list;
	struct Filter *filter;

	collapse(word);

	list = &global_filter_list;

        if (global_filter_list.length > 0)
        {
  		DLINK_FOREACH (ptr, list->head)
		{
			filter = ptr->data;
			if(!strcasecmp(filter->word, word))
			{
				sendto_realops_flags (UMODE_DEBUG, L_ALL, "Matching word, aborting addition.");
				return 0;
			}
		}
	}

	/* Reject <censored> as one of the filter words
         * Warning recieved from Hwy
         */
	if (!strcasecmp(word, "<censored>"))
	{
		sendto_realops_flags (UMODE_DEBUG, L_ALL, "Forbidden filter word, aborting addition.");
		return 0;
	}

	filter = (struct Filter *) BlockHeapAlloc(filter_heap);
	DupString(filter->word, word);
	filter->when = CurrentTime;
	dlinkAdd(filter, &filter->node, list);

	sendto_realops_flags (UMODE_DEBUG, L_ALL, "Added word %s", word);

	return 1;
}

int filter_del_word (char *word)
{
	dlink_node *ptr;
	struct Filter *f;

	if(!word)
	{
		sendto_realops_flags (UMODE_DEBUG, L_ALL, "No word sent with MODE.");
		return 0;
	}

	sendto_realops_flags (UMODE_DEBUG, L_ALL, "Searching for %s", word);

	DLINK_FOREACH (ptr, global_filter_list.head)
	{
		f = ptr->data;

		if(irccmp(word, f->word) == 0)
		{
			MyFree(f->word);
			BlockHeapFree(filter_heap, f);

			dlinkDelete(&f->node, &global_filter_list);
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
		BlockHeapFree(filter_heap, f);
		free_dlink_node(ptr);
	}

	list->head = list->tail = NULL;
	list->length = 0;
}
