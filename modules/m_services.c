/*
 * shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 * 
 * $Id: m_services.c,v 1.1.1.1 2004/09/07 04:50:37 nenolod Exp $
 */

/* List of ircd includes from ../include/ */
#include "stdinc.h"
#include "handlers.h"
#include "client.h"
#include "common.h"     /* FALSE bleah */
#include "sprintf_irc.h"
#include "ircd.h"
#include "irc_string.h"
#include "numeric.h"
#include "fdlist.h"
#include "hash.h"
#include "s_bsd.h"
#include "s_conf.h"
#include "s_log.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "channel.h"
#include "channel_mode.h"


#define MAXLINE  512    /* don't change this */

/* Prototypes for functions
 */
static void m_global(struct Client *client_p, struct Client *source_p,
                     int parc, char *parv[]);
static void m_nickserv(struct Client *client_p, struct Client *source_p,
                       int parc, char *parv[]);
static void m_chanserv(struct Client *client_p, struct Client *source_p,
                       int parc, char *parv[]);
static void m_memoserv(struct Client *client_p, struct Client *source_p,
                       int parc, char *parv[]);
static void m_seenserv(struct Client *client_p, struct Client *source_p,
                       int parc, char *parv[]);
static void m_operserv(struct Client *client_p, struct Client *source_p,
                       int parc, char *parv[]);
static void m_statserv(struct Client *client_p, struct Client *source_p,
                       int parc, char *parv[]);
static void m_helpserv(struct Client *client_p, struct Client *source_p,
                       int parc, char *parv[]);
static void m_identify(struct Client *client_p, struct Client *source_p,
                       int parc, char *parv[]);
static void m_botserv(struct Client *client_p, struct Client *source_p,
                       int parc, char *parv[]);
static void m_hostserv(struct Client *client_p, struct Client *source_p,
                       int parc, char *parv[]);


/* Structures
 */
struct Message global_msgtab = {
  "GLOBAL", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_global, m_ignore, m_global}
};
struct Message nickserv_msgtab = {
  "NICKSERV", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_nickserv, m_ignore, m_nickserv}
};
struct Message chanserv_msgtab = {
  "CHANSERV", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_chanserv, m_ignore, m_chanserv}
};
struct Message memoserv_msgtab = {
  "MEMOSERV", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_memoserv, m_ignore, m_memoserv}
};
struct Message seenserv_msgtab = {
  "SEENSERV", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_seenserv, m_ignore, m_seenserv}
};
struct Message operserv_msgtab = {
  "OPERSERV", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_operserv, m_ignore, m_operserv}
};
struct Message statserv_msgtab = {
  "STATSERV", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_statserv, m_ignore, m_statserv}
};
struct Message helpserv_msgtab = {
  "HELPSERV", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_helpserv, m_ignore, m_helpserv}
};
struct Message identify_msgtab = {
  "IDENTIFY", 0, 0, 0, 2, MFLG_SLOW, 0,
  {m_unregistered, m_identify, m_ignore, m_identify}
};
struct Message botserv_msgtab = {
  "BOTSERV", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_botserv, m_ignore, m_botserv}
};
struct Message hostserv_msgtab = {
  "HOSTSERV", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_hostserv, m_ignore, m_hostserv}
};


/* Aliases only for NickServ, ChanServ, MemoServ and BotServ:
 * /ns, /cs, /ms, /bs, /hs, /ss
 */
struct Message ns_msgtab = {
  "NS", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_nickserv, m_ignore, m_nickserv}
};
struct Message cs_msgtab = {
  "CS", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_chanserv, m_ignore, m_chanserv}
};
struct Message ms_msgtab = {
  "MS", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_memoserv, m_ignore, m_memoserv}
};
struct Message bs_msgtab = {
  "BS", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_botserv, m_ignore, m_botserv}
};
struct Message ss_msgtab = {
  "SS", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_statserv, m_ignore, m_statserv}
};
struct Message hs_msgtab = {
  "HS", 0, 0, 1, 0, MFLG_SLOW, 0,
  {m_unregistered, m_hostserv, m_ignore, m_hostserv}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(&global_msgtab);
  mod_add_cmd(&nickserv_msgtab);
  mod_add_cmd(&chanserv_msgtab);
  mod_add_cmd(&memoserv_msgtab);
  mod_add_cmd(&seenserv_msgtab);
  mod_add_cmd(&operserv_msgtab);
  mod_add_cmd(&statserv_msgtab);
  mod_add_cmd(&helpserv_msgtab);
  mod_add_cmd(&identify_msgtab);
  mod_add_cmd(&botserv_msgtab);
  mod_add_cmd(&hostserv_msgtab);

  mod_add_cmd(&ns_msgtab);
  mod_add_cmd(&cs_msgtab);
  mod_add_cmd(&ms_msgtab);
  mod_add_cmd(&bs_msgtab);
  mod_add_cmd(&hs_msgtab);
  mod_add_cmd(&ss_msgtab);

}

void
_moddeinit(void)
{
  mod_del_cmd(&global_msgtab);
  mod_del_cmd(&nickserv_msgtab);
  mod_del_cmd(&chanserv_msgtab);
  mod_del_cmd(&memoserv_msgtab);
  mod_del_cmd(&seenserv_msgtab);
  mod_del_cmd(&operserv_msgtab);
  mod_del_cmd(&statserv_msgtab);
  mod_del_cmd(&helpserv_msgtab);
  mod_del_cmd(&identify_msgtab);
  mod_del_cmd(&botserv_msgtab);
  mod_del_cmd(&hostserv_msgtab);

  mod_del_cmd(&ns_msgtab);
  mod_del_cmd(&cs_msgtab);
  mod_del_cmd(&ms_msgtab);
  mod_del_cmd(&bs_msgtab);
  mod_del_cmd(&hs_msgtab);
  mod_del_cmd(&ss_msgtab);
}

char *_version = "$Revision: 1.1.1.1 $";
#endif

/*
 * GetString()
 *
 * Reverse the array parv back into a normal string assuming there are
 * 'parc' indices in the array. Space is allocated for the new string.
 * Thanks to kre.
 */
char *GetString(int parc, char *parv[])
{
  char *final;
  char temp[MAXLINE];
  int ii = 0;

  final = MyMalloc(sizeof(char));
  *final = '\0';

  while (ii < parc)
  {
    ircsprintf(temp, "%s%s", parv[ii], ((ii + 1) >= parc) ? "" : " ");
    final = MyRealloc(final,  strlen(final) + strlen(temp) +
        sizeof(char));
    strcat(final, temp);
    ++ii;
  }

  return (final);
} /* GetString() */


/*
 * m_identify()
 *
 * Short identify command.
 */
static void m_identify(struct Client *client_p, struct Client *source_p,
                       int parc, char *parv[])
{
  struct Client *acptr;

  if (parc == 3)
  {
    acptr = find_person("ChanServ");
    if (!acptr || IsServer(acptr))
    {
      sendto_one(source_p,
          ":%s NOTICE %s :*** Notice -- Services are currently down. "
          "Please wait a few moments, and then try again.",
          me.name, parv[0]);
      return;
    }
    else
      sendto_one(acptr, ":%s PRIVMSG ChanServ :IDENTIFY %s %s",
	         parv[0], parv[1], parv[2]);
  }
  else
  if (parc == 2)
  {
    acptr = find_person("NickServ");
    if (!acptr || IsServer(acptr))
    {
      sendto_one(source_p,
          ":%s NOTICE %s :*** Notice -- Services are currently down. "
          "Please wait a few moments, and then try again.",
		      me.name, parv[0]);
      return;
    }
    else
      sendto_one(acptr, ":%s PRIVMSG NickServ :IDENTIFY %s",
                 parv[0], parv[1]);
  }
  else
  /* WTF? -kre */
  /* if (parc == 0 || parv[1] == '\0') */
  {
    sendto_one(source_p,
        ":%s NOTICE %s :Syntax:  IDENTIFY <password>            - for nickname",
               me.name, parv[0]);
    sendto_one(source_p,
        ":%s NOTICE %s :Syntax:  IDENTIFY <channel> <password>  - for channel",
               me.name, parv[0]);
  }

} /* m_identify() */


/*
 * deliver_services_msg()
 *
 *   parv[0]    = sender prefix
 *   servmsg    = message for services (using GetString() function).
 *                I think this is better. -ddosen
 */
static void deliver_services_msg(char *service, char *command, struct
    Client *client_p, struct Client *source_p, int parc, char *parv[])
{
  struct Client *acptr;
  char *servmsg;

  /* first of all, make sure the nick is online.
   */
  acptr = find_client(service);

  if (!acptr || IsServer(acptr))
  {
    sendto_one(source_p,
        ":%s NOTICE %s :*** Notice -- Services are currently down. "
        "Please wait a few moments, and then try again.",
        me.name, parv[0]);
  }
  else 
  {
    if (parc == 1 || parv[1] == '\0')
    {
      sendto_one(source_p, form_str(ERR_NEEDMOREPARAMS), me.name, parv[0],
          command);
      return;
    }

    servmsg = GetString(parc - 1, parv + 1);

    /* deliver the message.
     * now, it's more secure (using user@host)
     */
    sendto_one(acptr, ":%s PRIVMSG %s :%s", parv[0], acptr->name,
        servmsg);
    }
} /* deliver_services_msg() */


/* SERV_FUNC
 * Macro for generating service's function.
 */
#define SERV_FUNC(a,b,c) static void a (struct Client *client_p, struct \
    Client *source_p, int parc, char *parv[]) \
{ deliver_services_msg (b, c, client_p, source_p, parc, parv); }

/* let's generate :-)
 *
 * Fields are:  function_name, services nickname, command name
 */
SERV_FUNC(m_global, "Global", "GLOBAL")
SERV_FUNC(m_nickserv, "NickServ", "NICKSERV")
SERV_FUNC(m_chanserv, "ChanServ", "CHANSERV")
SERV_FUNC(m_memoserv, "MemoServ", "MEMOSERV")
SERV_FUNC(m_seenserv, "SeenServ", "SEENSERV")
SERV_FUNC(m_operserv, "OperServ", "OPERSERV")
SERV_FUNC(m_statserv, "StatServ", "STATSERV")
SERV_FUNC(m_helpserv, "HelpServ", "HELPSERV")
SERV_FUNC(m_botserv, "BotServ", "BOTSERV")
SERV_FUNC(m_hostserv, "HostServ", "HOSTSERV")

