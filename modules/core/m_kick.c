/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  m_kick.c: Kicks a user from a channel.
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
 *  $Id: m_kick.c,v 1.2 2004/09/08 03:44:29 nenolod Exp $
 */

#include "stdinc.h"
#include "tools.h"
#include "handlers.h"
#include "channel.h"
#include "channel_mode.h"
#include "client.h"
#include "irc_string.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "msg.h"
#include "modules.h"
#include "parse.h"
#include "hash.h"
#include "packet.h"
#include "s_serv.h"

static void m_kick(struct Client *, struct Client *, int, char **);

struct Message kick_msgtab = {
  "KICK", 0, 0, 3, 0, MFLG_SLOW, 0,
  {m_unregistered, m_kick, m_kick, m_kick, m_ignore}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(&kick_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(&kick_msgtab);
}

const char *_version = "$Revision: 1.2 $";
const char *_desc = "Implements /kick command -- removes a user from a channel.";
#endif

/* m_kick()
 *  parv[0] = sender prefix
 *  parv[1] = channel
 *  parv[2] = client to kick
 *  parv[3] = kick comment
 */
static void 
m_kick(struct Client *client_p, struct Client *source_p,
       int parc, char *parv[])
{
  struct Client *who;
  struct Channel *chptr;
  int chasing = 0;
  char *comment;
  char *name;
  char *p = NULL;
  char *user;
  const char *from, *to;
  struct Membership *ms = NULL;
  struct Membership *ms_target;

  if (!MyConnect(source_p) && IsCapable(source_p->from, CAP_TS6) && HasID(source_p))
  {
    from = me.id;
    to = source_p->id;
  }
  else
  {
    from = me.name;
    to = source_p->name;
  }

  if (*parv[2] == '\0')
  {
    sendto_one(source_p, form_str(ERR_NEEDMOREPARAMS),
               from, to, "KICK");
    return;
  }

  if (MyClient(source_p) && !IsFloodDone(source_p))
    flood_endgrace(source_p);

  comment = (EmptyString(parv[3])) ? parv[2] : parv[3];
  if (strlen(comment) > (size_t) TOPICLEN)
    comment[TOPICLEN] = '\0';

  name = parv[1];
  while (*name == ',')
    name++;
  if ((p = strchr(name,',')) != NULL)
    *p = '\0';
  if (!*name)
    return;

  if ((chptr = hash_find_channel(name)) == NULL)
  {
    sendto_one(source_p, form_str(ERR_NOSUCHCHANNEL),
               from, to, name);
    return;
  }


  if (!IsServer(source_p) || (!MyConnect(source_p) ||
       (MyConnect(source_p) && !(source_p->localClient->operflags & OPER_FLAG_OVERRIDE))))
  {
    if ((ms = find_channel_link(source_p, chptr)) == NULL)
    {
      if (MyConnect(source_p))
      {
        sendto_one(source_p, form_str(ERR_NOTONCHANNEL),
                   me.name, source_p->name, name);
        return;
      }
    }

#ifndef DISABLE_CHAN_OWNER
    if (!has_member_flags(ms, CHFL_CHANOWNER) && (chptr->mode.mode & MODE_PEACE))
#else
    if (!has_member_flags(ms, CHFL_CHANOP) && (chptr->mode.mode & MODE_PEACE))
#endif
    {
      /* don't kick in +P mode. */
      sendto_one(source_p, form_str(ERR_CHANPEACE), me.name, source_p->name, name);
      return;
    }

    if (!has_member_flags(ms, CHFL_CHANOP|CHFL_HALFOP|CHFL_CHANOWNER))
    {
      /* was a user, not a server, and user isn't seen as a chanop here */
      if (MyConnect(source_p))
      {
        /* user on _my_ server, with no chanops.. so go away */
        sendto_one(source_p, form_str(ERR_CHANOPRIVSNEEDED),
                   me.name, source_p->name, name);
        return;
      }

      if (chptr->channelts == 0)
      {
        /* If its a TS 0 channel, do it the old way */         
        sendto_one(source_p, form_str(ERR_CHANOPRIVSNEEDED),
                   from, to, name);
        return;
      }
    }
  }

  user = parv[2];

  while (*user == ',')
    user++;

  if ((p = strchr(user,',')) != NULL)
    *p = '\0';

  if (!*user)
    return;

  if ((who = find_chasing(source_p, user, &chasing)) == NULL)
    return;

  if (HasUmode(who, UMODE_PROTECTED))
  {
    /* If it isnt an oper overriding the kick, well, then we deny it if the person is umode +q */
    if (MyConnect(source_p) && !(source_p->localClient->operflags & OPER_FLAG_OVERRIDE))
    {
      sendto_one(source_p, ":%s NOTICE %s :*** Can't kick user from channel, he/she is umode +q. (protected oper)",
                 me.name, source_p->name);
      return;
    }

    /* if it is not from us, we let it through --nenolod */
  }
    
  if ((ms_target = find_channel_link(who, chptr)) != NULL)
  {
    /* half ops cannot kick other halfops on private channels */
    if (has_member_flags(ms, CHFL_HALFOP) && !has_member_flags(ms, CHFL_CHANOP))
    {
      if (((chptr->mode.mode & MODE_PRIVATE) && has_member_flags(ms_target, CHFL_CHANOP|CHFL_HALFOP)) ||
            has_member_flags(ms_target, CHFL_CHANOP))
      {
        sendto_one(source_p, form_str(ERR_CHANOPRIVSNEEDED),
                   me.name, source_p->name, name);
        return;
      }
    }

#ifndef DISABLE_CHAN_OWNER
   /* Actually protect the +u's. We were silly and forgot about this one.
    * Thanks to akba on Subnova for discovering this.
    *
    * - however, we do want opers to be able to override this, that way
    *   some of the more abusive networks running shadow can still have 
    *   their fun, as uncool as it probably is. -nenolod
    */
   if (!MyConnect(source_p) || (MyConnect(source_p) && 
         source_p->localClient->operflags & OPER_FLAG_OVERRIDE))
   {
      if (has_member_flags(ms, CHFL_CHANOP) && has_member_flags(ms_target, CHFL_CHANOWNER))
      {
          sendto_one(source_p, form_str(ERR_CHANOPRIVSNEEDED),
                    me.name, source_p->name, name);
          return;
      }

      if (has_member_flags(ms, CHFL_HALFOP) && has_member_flags(ms_target, CHFL_CHANOWNER))
      {
          sendto_one(source_p, form_str(ERR_CHANOPRIVSNEEDED),
                me.name, source_p->name, name);
          return;
      }
   }
#endif

   /* jdc
    * - In the case of a server kicking a user (i.e. CLEARCHAN),
    *   the kick should show up as coming from the server which did
    *   the kick.
    * - Personally, flame and I believe that server kicks shouldn't
    *   be sent anyways.  Just waiting for some oper to abuse it...
    */
    if (IsServer(source_p))
      sendto_channel_local(ALL_MEMBERS, chptr, ":%s KICK %s %s :%s",
                           source_p->name, name, who->name, comment);
    else
      sendto_channel_local(ALL_MEMBERS, chptr, ":%s!%s@%s KICK %s %s :%s",
                           source_p->name, source_p->username,
                           GET_CLIENT_HOST(source_p), name, who->name, comment);

    sendto_server(client_p, NULL, chptr, CAP_TS6, NOCAPS, NOFLAGS,
                  ":%s KICK %s %s :%s",
                  ID(source_p), chptr->chname, ID(who), comment);
    sendto_server(client_p, NULL, chptr, NOCAPS, CAP_TS6, NOFLAGS,
                  ":%s KICK %s %s :%s", source_p->name, chptr->chname,
                  who->name, comment);

    remove_user_from_channel(ms_target);
  }
  else
    sendto_one(source_p, form_str(ERR_USERNOTINCHANNEL),
               from, to, user, name);
}
