/*
 *  shadowircd: an advanced Internet Relay Chat daemon(ircd).
 *  m_persist.c: No comment.
 *
 *  Copyright (c) 2004 ShadowIRCd development group
 *
 *  $Id: m_persist.c,v 1.5 2004/03/22 20:18:53 nenolod Exp $
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
 * This is it. Persistant clients.
 * This module works fairly simply:
 *
 * 1. User sets a password for his session.
 * 2. User gets disconnected, or disconnects.
 * 3. User comes back and logs in under another nick.
 * 4. User is joined to the channels he was in and is opped/voiced/etc in
 *    the channels he had status in.
 * 5. Any operator status and usermodes are transfered over.
 * 6. Any vHost is transfered over.
 * 7. The new client is marked as persistant.
 * 8. The old client is exited using client_exit();
 *
 * Rince and repeat. Persistant clients done easily.
 *
 * A majority of this code was written on March 22, 2004 by nenolod.
 */

static void m_persist (struct Client *, struct Client *, int, char **);

struct Message persist_msgtab = {
  "PERSIST", 0, 0, 0, 0, MFLG_SLOW, 0L,
  {m_unregistered, m_persist, m_ignore, m_persist, m_ignore}
};

#ifndef STATIC_MODULES
void
_modinit (void)
{
  mod_add_cmd (&persist_msgtab);
}

void
_moddeinit (void)
{
  mod_del_cmd (&persist_msgtab);
}
#endif

/*
 * m_persist
 * parv[1] = command [PASSWORD|ATTACH|DESTROY]
 * parv[2] = password
 */
static void
m_persist (struct Client *client_p, struct Client *source_p,
           int parc, chat *parv[])
{
  /* do persist stuff here */
}

