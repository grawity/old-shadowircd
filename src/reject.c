/*
 *  shadowircd: An advanced IRC daemon (ircd).
 *  reject.c: nenolod's reject engine. (inspired from the ratbox one.)
 *
 *  $Id: reject.c,v 3.4 2004/09/22 18:07:38 nenolod Exp $
 */

#include "stdinc.h"
#include "client.h"
#include "s_conf.h"
#include "event.h"
#include "tools.h"
#include "reject.h"
#include "irc_string.h"
#include "common.h"
#include "memory.h"
#include "ircd.h"
#include "list.h"

dlink_list rlist;
unsigned long reject_count = 0;

struct reject_data_
{
        char banaddr[HOSTIPLEN]; /* The IP to reject. */
	time_t time;
	unsigned int count;
};

typedef struct reject_data_ reject_data;

reject_data *create_reject(char *ipaddr)
{
        reject_data *new = MyMalloc(sizeof(reject_data));
        dlink_node *entry;
        entry = make_dlink_node();

        strcpy(new->banaddr, ipaddr);

        dlinkAddTail(new, entry, &rlist);

        return new;
}

reject_data *search_reject(char *ipaddr)
{
        reject_data *data;
        dlink_node *entry, *next;

        DLINK_FOREACH_SAFE(entry, next, rlist.head) {
		data = entry->data;
		if (match(data->banaddr, ipaddr))
			return data;
        }

        return NULL;
}

void
reject_expires(void *unused)
{
	reject_data *data;
	dlink_node *entry;

	DLINK_FOREACH(entry, rlist.head) {
		data = (reject_data *)entry->data;
		if (data->time + 300 < CurrentTime) {
			dlinkDelete(entry, &rlist);
			MyFree(data);
		}
	}
}

void
init_reject(void)
{
	eventAdd("reject_expires", reject_expires, NULL, 300);
}

void
add_reject(struct Client *client_p)
{
	reject_data *rdata;

	if ((rdata = search_reject(client_p->ipaddr)) != NULL)
        {
		rdata->time = CurrentTime;
		rdata->count++;
	}
	else
	{
		rdata = create_reject(client_p->ipaddr);
		rdata->time = CurrentTime;
		rdata->count = 1;
	}
}

int
check_reject(struct Client *client_p)
{
	reject_data *rdata;

	rdata = search_reject(client_p->ipaddr);
	if (rdata != NULL)
	{
		rdata->time = CurrentTime;
		reject_count++;
		return 1;
	}

	return 0;
}

void flush_reject(void)
{
        dlink_node *ptr, *next;
        reject_data *rdata;

        DLINK_FOREACH_SAFE(ptr, next, rlist.head)
        {
                rdata = ptr->data;
                dlinkDelete(ptr, &rlist);
                MyFree(rdata);
        }
}
