/*
 * shadowircd: an advanced Internet Relay Chat daemon(ircd). m_persist.c: No
 * comment.
 * 
 * Copyright (c) 2004 ShadowIRCd development group
 * 
 * $Id: m_persist.c,v 1.6 2004/03/22 21:20:34 nenolod Exp $
 */

#include "stdinc.h"
#include "tools.h"
#include "common.h"
#include "handlers.h"
#include "client.h"
#include "hash.h"
#include "channel.h"
#include "channel_mode.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_serv.h"
#include "send.h"
#include "list.h"
#include "irc_string.h"
#include "sprintf_irc.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "hook.h"

/*
 * This is it. Persistant clients. This module works fairly simply:
 * 
 * 1. User sets a password for his session. 2. User gets disconnected, or
 * disconnects. 3. User comes back and logs in under another nick. 4. User is
 * joined to the channels he was in and is opped/voiced/etc in the channels
 * he had status in. 5. Any operator status and usermodes are transfered
 * over. 6. Any vHost is transfered over. 7. The new client is marked as
 * persistant. 8. The old client is exited using client_exit();
 * 
 * Rince and repeat. Persistant clients done easily.
 * 
 * A majority of this code was written on March 22, 2004 by nenolod.
 */

static void     m_persist(struct Client *, struct Client *, int, char **);

struct Message  persist_msgtab = {
	"PERSIST", 0, 0, 0, 0, MFLG_SLOW, 0L,
	{m_unregistered, m_persist, m_ignore, m_persist, m_ignore}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
	mod_add_cmd(&persist_msgtab);
}

void
_moddeinit(void)
{
	mod_del_cmd(&persist_msgtab);
}
#endif

/*
 * m_persist parv[1] = command [PASSWORD|ATTACH|DESTROY] parv[2] = password
 */
static void
m_persist(struct Client * client_p, struct Client * source_p,
	  int parc, char *parv[])
{
	struct Client  *target_p;
	struct Channel *chptr;
	struct Membership *ms;
	unsigned int    old = 0;
	dlink_node     *dm;

	if (!strcasecmp(parv[1], "PASSWORD")) {
		if (!parv[2]) {
			sendto_one(source_p, ":%s NOTICE %s :*** Not enough parameters",
				   me.name, source_p->name);
			return;
		}
		strcpy(parv[2], source_p->persistpw);
		return;
	}
	if (!strcasecmp(parv[1], "ATTACH")) {
		if (!(target_p = find_client(parv[2]))) {
			sendto_one(source_p, ":%s NOTICE %s :*** There are no persistant clients using this nickname.",
				   me.name, source_p->name);
			return;
		}
		if (!strcmp(parv[3], target_p->persistpw)) {
			sendto_one(source_p, ":%s NOTICE %s :*** Attaching to %s.",
				   me.name, source_p->name, target_p->name);

			/* Join the user to his channels. */
			DLINK_FOREACH(dm, target_p->user->channel.head) {
				ms = (struct Membership *) ptr->data;
				chptr = ms->chptr;

				add_user_to_channel(chptr, source_p, ms->flags);

				sendto_server(client_p, source_p, chptr, CAP_TS6, NOCAPS, LL_ICLIENT,
					      ":%s SJOIN %lu %s +nt :%s",
				     me.id, (unsigned long)chptr->channelts,
					      chptr->chname, source_p->id);
				sendto_server(client_p, source_p, chptr, NOCAPS, CAP_TS6, LL_ICLIENT,
					      ":%s SJOIN %lu %s +nt :%s",
				   me.name, (unsigned long)chptr->channelts,
					      chptr->chname, source_p->name);

				sendto_channel_local(ALL_MEMBERS, chptr, ":%s!%s@%s JOIN :%s",
					 source_p->name, source_p->username,
				  GET_CLIENT_HOST(source_p), chptr->chname);

				if (ms->flags & CHFL_CHANOWNER) {
					sendto_channel_local(ALL_MEMBERS, chptr,
						  ":%s!%s@%s MODE %s +u %s",
					 source_p->name, source_p->username,
							     GET_CLIENT_HOST(source_p), chptr->chname,
							     source_p->name);
				}
				if (ms->flags & CHFL_CHANOP) {
					sendto_channel_local(ALL_MEMBERS, chptr,
						  ":%s!%s@%s MODE %s +o %s",
					 source_p->name, source_p->username,
							     GET_CLIENT_HOST(source_p), chptr->chname,
							     source_p->name);
				}
				if (ms->flags & CHFL_HALFOP) {
					sendto_channel_local(ALL_MEMBERS, chptr,
						  ":%s!%s@%s MODE %s +h %s",
					 source_p->name, source_p->username,
							     GET_CLIENT_HOST(source_p), chptr->chname,
							     source_p->name);
				}
				if (ms->flags & CHFL_VOICE) {
					sendto_channel_local(ALL_MEMBERS, chptr,
						  ":%s!%s@%s MODE %s +v %s",
					 source_p->name, source_p->username,
							     GET_CLIENT_HOST(source_p), chptr->chname,
							     source_p->name);
				}
			}
		}
	}
}
