/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  list.c: Various assorted functions for various structures.
 *
 *  Copyright (C) 2002 by the past and present ircd coders, and others.
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
 *  $Id: list.c,v 3.3 2004/09/08 01:18:08 nenolod Exp $
 */

#include "stdinc.h"
#include "tools.h"
#include "client.h"
#include "list.h"
#include "memory.h"
#include "balloc.h"

static int32_t dlinks_count = 0;
static int32_t slinks_count = 0;

static BlockHeap *dnode_heap;
static BlockHeap *snode_heap;


/* make_server()
 *
 * inputs       - pointer to client struct
 * output       - pointer to struct Server
 * side effects - add's an Server information block to a client
 *                if it was not previously allocated.
 */
struct Server *
make_server(struct Client *client_p)
{
  struct Server *serv = client_p->serv;

  if (!serv)
  {
    serv = MyMalloc(sizeof(struct Server));
    client_p->serv = serv;
  }

  return(client_p->serv);
}

/* init_dlink_nodes()
 *
 * inputs       - NONE
 * output       - NONE
 * side effects - initializes the dnode BlockHeap
 */
void
init_dlink_nodes(void)
{
  dnode_heap = BlockHeapCreate(sizeof(dlink_node), DNODE_HEAP_SIZE);
}

/* make_dlink_node()
 *
 * inputs       - NONE
 * output       - pointer to new dlink_node
 * side effects	- NONE
 */
dlink_node *
make_dlink_node(void)
{
  dlink_node *lp = BlockHeapAlloc(dnode_heap);

  ++dlinks_count;

  lp->next = NULL;
  lp->prev = NULL;
  return(lp);
}

/* free_dlink_node()
 *
 * inputs       - pointer to dlink_node
 * output       - NONE
 * side effects	- free given dlink_node 
 */
void
free_dlink_node(dlink_node *ptr)
{
  BlockHeapFree(dnode_heap, ptr);
  --dlinks_count;
  assert(dlinks_count >= 0);
}

/* init_slink_nodes()
 *
 * inputs       - NONE
 * output       - NONE
 * side effects - initializes the snode BlockHeap
 */
void
init_slink_nodes(void)
{
  snode_heap = BlockHeapCreate(sizeof(slink_node), SNODE_HEAP_SIZE);
}

/* make_slink_node()
 *
 * inputs       - NONE
 * output       - pointer to new slink_node
 * side effects - NONE
 */
slink_node *
make_slink_node(void)
{
  slink_node *lp = BlockHeapAlloc(snode_heap);

  ++slinks_count;

  lp->next = NULL;
  return(lp);
}

/* free_slink_node()
 *
 * inputs       - pointer to slink_node
 * output       - NONE
 * side effects - free given slink_node
 */
void
free_slink_node(slink_node *ptr)
{
  BlockHeapFree(snode_heap, ptr);
  --slinks_count;
  assert(slinks_count >= 0);
}

/* count_links_memory()
 *
 * inputs       - pointer to dlinks memory actually used
 *              - pointer to dlinks memory allocated total in block allocator
 * output       - NONE
 * side effects - NONE
 */
void
count_links_memory(int *count, unsigned long *links_memory_used)
{
  *count = dlinks_count;
  *links_memory_used = dlinks_count * sizeof(dlink_node);
}
