/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  m_persist.c: Persistant client handler.
 *
 *  Copyright (C) 2004 by the past and present ircd coders, and others.
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
 *  $Id: m_persist.c,v 1.1 2004/02/05 23:29:43 nenolod Exp $
 */

#include <string.h>
#include "handlers.h"
#include "packet.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "s_bsd.h"
#include "s_conf.h"
#include "s_serv.h"
#include "s_user.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "fdlist.h"
#include "channel_mode.h"

static void m_persist(struct Client*, struct Client*, int, char**);

struct Message persist_msgtab = {
  "PERSIST", 0, 0, 0, 0, MFLG_SLOW, 0,
  {m_unregistered, m_persist, m_persist, m_persist }
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

char *_version = "$Version$";
#endif

/*
 * m_persist - PERSIST command handler
 *      parv[0] = sender prefix
 *      parv[1] = subcommand
 *
 * Usage in its current state:
 *
 *   /persist pass <password> - set a password to log back in as
 *   /persist reset - reset password to blank
 *   /persist attach <nick> <password> - reattach to a persistent client
 *
 * As long as a password is set, disconnecting from the ircd will
 * simply result in client being put into a persistent state, where
 * no i/o can occur, and client cannot be disconnected (except by
 * being /kill'ed). To actually quit a client, do a "/persist reset"
 * to reset the password, after which /quit'ing or simply pinging
 * off will result in a normal disconnection (blank password means
 * no persistent client). Abnormal termination of the connection
 * (i.e. being packetted) after a password has been set will result
 * in a persistent client. The password still stay the same after
 * reattaching and does not need to be re-entered after attaching.
 */

static void m_persist(struct Client* client_p, struct Client* source_p,
                      int parc, char* parv[])
{
  struct Client* target_p;
  struct Channel *chptr;
  dlink_node *ptr;
  static char modebuf[MODEBUFLEN];
  static char parabuf[MODEBUFLEN];
  static char* namepointer[2];

  if(parc < 2) {
    sendto_one(source_p, ":%s NOTICE %s :PERSIST commands available: PASS, ATTACH, RESET",
               me.name, source_p->name);
    return;
  }

  if(strcasecmp(parv[1],"PASS") == 0) {

    if(parc < 3) {
      sendto_one(source_p, ":%s NOTICE %s :PERSIST PASS usage: /PERSIST PASS <password>",
                 me.name, source_p->name);
      return;
    }

    strncpy(source_p->persistpw, parv[2], PASSWDLEN-1);
    source_p->persistpw[PASSWDLEN-1] = '\0';

    sendto_one(source_p, ":%s NOTICE %s :PERSIST PASS: password set to: %s",
               me.name, source_p->name, source_p->persistpw);

  } else if(strcasecmp(parv[1],"ATTACH") == 0) {

    if(parc < 4) {
      sendto_one(source_p, ":%s NOTICE %s :PERSIST ATTACH usage: /PERSIST ATTACH <nick> <password>",
                 me.name, source_p->name);
      return;
    }

    DLINK_FOREACH(ptr, global_client_list.head)
    {
      target_p = (struct Client *)ptr->data;

      if(strcasecmp(target_p->name, parv[2])) continue;

      if(!target_p->flags & FLAGS_PERSISTANT) {
        sendto_one(source_p, ":%s NOTICE %s :PERSIST ATTACH: Incorrect nick/password", me.name, source_p->name);
        return;
      }

      if(strcmp(target_p->persistpw, parv[3])) {
        sendto_one(source_p, ":%s NOTICE %s :PERSIST ATTACH: Incorrect nick/password", me.name, source_p->name);
        return;
      }



      /*
      ** If we make it here, then persistent reattachment nick/pass was correct,
      ** proceed with reattaching the client.
      */


      /* Client will think he changed nicks to the persistent client's nick */

      sendto_one(source_p, ":%s!%s@%s NICK :%s",
                   source_p->name, source_p->username, source_p->host,
                   target_p->name);


      /* Client is no longer persistent, also swap fd's for reattachment */

      target_p->flags &= ~FLAGS_PERSISTANT;
      target_p->localClient->fd = source_p->localClient->fd;
      target_p->localClient->ctrlfd = source_p->localClient->ctrlfd;

#ifndef HAVE_SOCKETPAIR
      target_p->localClient->fd_r = source_p->localClient->fd_r;
      target_p->localClient->ctrlfd_r = source_p->localClient->ctrlfd_r;
      source_p->localClient->fd_r = -1;
      source_p->localClient->ctrlfd_r = -1;
#endif
      source_p->localClient->fd = -1;
      source_p->localClient->ctrlfd = -1;

      fd_note(target_p->localClient->fd, "Nick: %s", target_p->name);

      /* mark socket to be looked at for read events again */

      comm_setselect(target_p->localClient->fd, FDLIST_IDLECLIENT, COMM_SELECT_READ, read_packet,
        target_p, 0);


      /* send JOIN messages to channels client wasn't in */

      for (ptr = target_p->user->channel.head; ptr; ptr = ptr->next) {
        chptr = ptr->data;
        if(!IsMember(source_p, chptr)) {
          sendto_one(target_p, ":%s!%s@%s JOIN :%s", target_p->name, target_p->username, target_p->host, chptr->chname);
        }
        channel_member_names(target_p, chptr, 1);

        channel_modes(chptr, target_p, modebuf, parabuf);
        sendto_one(target_p, form_str(RPL_CHANNELMODEIS), me.name, parv[0], chptr->chname, modebuf, parabuf);
        sendto_one(target_p, form_str(RPL_CREATIONTIME), me.name, parv[0], chptr->chname, chptr->channelts);
      }


      /* send PART messages to channels persistent client wasn't in */

      for (ptr = source_p->user->channel.head; ptr; ptr = ptr->next) {
        chptr = ptr->data;
        if(!IsMember(target_p, chptr)) {
          sendto_one(target_p, ":%s!%s@%s PART :%s", target_p->name, target_p->username, target_p->host, chptr->chname);
        }
      }


      /* update user mode */

      namepointer[0] = namepointer[1] = target_p->name;
      send_umode_out(target_p, target_p, 0);

      /* Set source_p as dead; it will go away in it's own time. */
      SetDead(source_p);
      return;
    }

    sendto_one(source_p, ":%s NOTICE %s :PERSIST ATTACH: Incorrect nick/password",
               me.name, source_p->name);
    return;

  } else if(strcasecmp(parv[1],"RESET") == 0) {

    source_p->persistpw[0] = '\0';
    source_p->flags &= ~FLAGS_PERSISTANT;

    sendto_one(source_p, ":%s NOTICE %s :PERSIST RESET: Password cleared; client will no longer persist",
                 me.name, source_p->name);

  } else {

    sendto_one(source_p, ":%s NOTICE %s :PERSIST commands available: PASS, ATTACH, RESET",
               me.name, source_p->name);
    return;

  }

  return;
}
