/*
 * NetworkBOPM: The ShadowIRCd Anti-Proxy Solution.
 * list.c: Functions for the manipulation of linked lists.
 *
 * $Id: list.c,v 1.2 2004/05/26 15:13:39 nenolod Exp $
 */

#include "netbopm.h"

node_t         *
node_create(void)
{
    node_t         *n;
    n = (node_t *) scalloc(sizeof(node_t), 1);
    n->next = n->prev = n->data = NULL;
    return n;
}

void
node_free(node_t * n)
{
    free(n);
}

void
node_add(void *data, node_t * n, list_t * l)
{
    node_t         *tn;
    n->data = data;
    if (!l->head) {
	l->head = n;
	l->tail = NULL;
	l->count++;
	return;
    }
    tn = l->tail;
    n->prev = tn;
    n->prev->next = n;
    l->tail = n;
    l->count++;
}

void
node_remove(node_t * n, list_t * l)
{
    if (!n->prev)
	l->head = n->next;
    else
	n->prev->next = n->next;

    if (!n->next)
	l->tail = n->prev;
    else
	n->next->prev = n->prev;

    l->count--;
}

node_t         *
node_find(void *data, list_t * l)
{
    node_t         *n;

    LIST_FOREACH(n, l->head) if (n->data == data)
	return n;

    return NULL;
}

server_t       *
server_create(void)
{
    server_t       *server = (server_t *) scalloc(sizeof(server_t), 1);

    server->name = '\0';
    server->sid = '\0';
    server->gecos = '\0';

    return server;
}

server_t       *
server_find(char *sid)
{
    server_t       *server;
    node_t         *n;

    LIST_FOREACH(n, serverlist.head) {
	server = (server_t *) n->data;

	if (!strcasecmp(server->sid, sid))
	    return server;
    }

    return NULL;
}
