/* JUMPSERVER module for ShadowIRCd.
 * This will work on any version of ShadowIRCd, >= 3.0.
 *
 * Drawbacks to this module:
 * - It can only redirect clients which support it. For networks
 *   running primarily X-Chat and mIRC, it will work fine.
 *
 * This module is made available to you, as is with no warranty.
 * I am not responsible for any damages.
 */

#include "stdinc.h"
#include "handlers.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "common.h"
#include "s_misc.h"
#include "s_conf.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "channel.h"
#include "channel_mode.h"
#include "irc_string.h"
#include "hash.h"
#include "packet.h"
#include "hook.h"

static void mo_jumpserver(struct Client *, struct Client *, int, char **);

struct Message jumpserver_msgtab = {
  "JUMPSERVER", 0, 0, 3, 4, MFLG_SLOW, 0,
  {m_unregistered, m_ignore, m_ignore, mo_jumpserver, m_ignore}
};
   
#ifndef STATIC_MODULES
void
_modinit (void)
{ 
  mod_add_cmd (&jumpserver_msgtab);
}

void
_moddeinit (void)
{
  mod_del_cmd (&jumpserver_msgtab);
}

const char *_version = "$Revision: 1.1 $";
#endif

/*
 * parv[1] - new server
 * parv[2] - new port
 * parv[3] - optional reason
 */
static void
mo_jumpserver (struct Client *client_p, struct Client *source_p,
               int parc, char *parv[])
{
  dlink_node *node;
  struct Client *client;
  int redirects = 0;

  DLINK_FOREACH(node, local_client_list.head)
  {
    client = node->data;
    if (MyClient(client) && !IsOper(client))
    {
      sendto_one(client, form_str(RPL_REDIR), me.name,
                 client->name, parv[1], parv[2]);
      exit_client(client, source_p, client_p, parv[3] ? parv[3] : "Redirection");
      redirects++;
    }
  }

  sendto_one(source_p, ":%s NOTICE %s :%d client(s) redirected.",
             me.name, source_p->name, redirects);
}
