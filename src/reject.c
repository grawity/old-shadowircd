/*
 *  shadowircd: An advanced IRC daemon (ircd).
 *  reject.c: nenolod's reject engine. (inspired from the ratbox one.)
 *
 *  $Id: reject.c,v 1.2 2004/04/30 21:55:57 nenolod Exp $
 */

#include "stdinc.h"
#include "match.h"
#include "client.h"
#include "s_conf.h"
#include "event.h"
#include "tools.h"
#include "reject.h"

static dlink_list rlist;
static dlink_list delay_exit;
static dlink_list reject_list;
unsigned long reject_count = 0;

struct reject_data_
{
        char *banaddr; /* Either an IP or a CIDR. */
	time_t time;
	unsigned int count;
};

typedef struct reject_data_ reject_data;

reject_data create_reject(dlink_list *rejlist, char *ipaddr)
{
        reject_data *new;
        dlink_node *entry;

        strcpy(new->banaddr, ipaddr);

        dlinkAddTail(new, entry, rejlist);

        return new;
}

reject_data search_reject(dlink_list *rejlist, char *ipaddr)
{
        reject_data *data;
        dlink_node *entry;

        DLINK_FOREACH(entry, rejlist->head) {
		data = (reject_data *)entry->data;
		if (match(data->banaddr, ipaddr))
			return data->banaddr;
        }

        return NULL;
}

void
reject_expires(void *unused)
{
	reject_data *data;
	dlink_node *entry;

	DLINK_FOREACH(entry, rejlist->head) {
		data = (reject_data *)entry->data;
		if (CurrentTime - data->time > 3600) {
			dlinkDelete(entry, &rlist);
			MyFree(data);
		}
	}
}

void
init_reject(void)
{
	eventAdd("reject_expires", reject_expires, NULL, 60);
}

void
add_reject(struct Client *client_p)
{
	reject_data *rdata;

	if (rdata = search_reject(rlist, client_p->ipaddr)) != NULL)
        {
		rdata->time = CurrentTime;
		rdata->count++;
	}
	else
	{
		rdata = create_reject(rlist, client_p->ipaddr));
		rdata->time = CurrentTime;
		rdata->count = 1;
	}
}

int
check_reject(struct Client *client_p))
{
	reject_data *rdata;

	rdata = search_reject(rlist, client_p->ipaddr);
	if (rdata != NULL)
	{
		rdata->time = CurrentTime;
		reject_count++;
		SetReject(client_p);
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
