/*******************************************************************
 * shadow-services version 1.0
 * hostserv.c: Handler for the HostServ bot.
 *
 * shadow-services is copyright (c) 2003 shadowircd team.
 * shadow-services is freely distributable under the terms of the
 * GNU General Public License, version 2. See LICENSE in the root
 * directory for more information.
 *
 * $Id: hostserv.c,v 1.1 2003/12/18 23:01:36 nenolod Exp $
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#endif

#include "defs.h"
#include "alloc.h"
#include "channel.h"
#include "chanserv.h"
#include "client.h"
#include "config.h"
#include "hash.h"
#include "helpserv.h"
#include "log.h"
#include "match.h"
#include "memoserv.h"
#include "misc.h"
#include "mystring.h"
#include "nickserv.h"
#include "operserv.h"
#include "settings.h"
#include "sprintf_irc.h"
#include "server.h"
#include "timestr.h"
#include "err.h"
#include "hybdefs.h"
#include "sock.h"

static struct Command vhostcmds[] = {
  { "HELP", h_help, LVL_NONE },
  { "ON", h_on, LVL_IDENT },
  { "SET", h_set, LVL_ADMIN },
  { 0, 0, 0 }
};

/*
 * hs_process()
 *
 * processes input coming to HostServ.
 */

void hs_process (char *nick, char *command) {
  int acnt;
  char **arv;
  struct Command *cptr;
  struct Luser *lptr;
  struct NickInfo *nptr;

  if (!command || !(lptr = FindClient(nick)))
    return;

  if (Network->flags & NET_OFF)
  {
    notice(n_HostServ, lptr->nick,
      "Services are currently \002disabled\002");
    return;
  }

  acnt = SplitBuf(command, &arv);
  if (acnt == 0)
  {
    MyFree(arv);
    return;
  }

  cptr = GetCommand(vhostcmds, arv[0]);

  if (!cptr || (cptr == (struct Command *) -1))
  {
    notice(n_HostServ, lptr->nick,
      "%s command [%s]",
      (cptr == (struct Command *) -1) ? "Ambiguous" : "Unknown",
      arv[0]);
    MyFree(arv);
    return;
  }

  if ((cptr->level == LVL_ADMIN) && !(IsValidAdmin(lptr)))
  {
    notice(n_HostServ, lptr->nick, "Unknown command [%s]",
      arv[0]);
    MyFree(arv);
    return;
  }

  if ((nptr = FindNick(lptr->nick)))
    if (nptr->flags & NS_FORBID)
    {
      notice(n_HostServ, lptr->nick,
        "Cannot execute commands for forbidden nicknames");
      MyFree(arv);
      return;
    }

  if (cptr->level != LVL_NONE)
  {
    if (!nptr)
    {
      notice(n_HostServ, lptr->nick,
        "You must register your nick with \002%s\002 first",
        n_NickServ);
      notice(n_HostServ, lptr->nick,
        "Type: \002/msg %s REGISTER <password>\002 to register your nickname",
        n_NickServ);
      MyFree(arv);
      return;
    }

    if (!(nptr->flags & NS_IDENTIFIED))
    {
      notice(n_HostServ, lptr->nick,
        "Password identification is required for [\002%s\002]",
        cptr->cmd);
      notice(n_HostServ, lptr->nick,
        "Type \002/msg %s IDENTIFY <password>\002 and retry",
        n_NickServ);
      MyFree(arv);
      return;
    }
  }

  (*cptr->func)(lptr, acnt, arv);

  MyFree(arv);
} /* hs_process() */

static void
h_help(struct Luser *lptr, int ac, char **av)

{
  if (ac >= 2)
  {
    char  str[MAXLINE];

    if (ac >= 3)
      ircsprintf(str, "%s %s", av[1], av[2]);
    else
    {
      struct Command *cptr;

      for (cptr = vhostcmds; cptr->cmd; cptr++)
        if (!irccmp(av[1], cptr->cmd))
          break;

      if (cptr->cmd)
        if ((cptr->level == LVL_ADMIN) &&
            !(IsValidAdmin(lptr)))
        {
          notice(n_HostServ, lptr->nick,
            "No help available on \002%s\002",
            av[1]);
          return;
        }

      ircsprintf(str, "%s", av[1]);

      }
    }

    GiveHelp(n_HostServ, lptr->nick, str, NODCC);
  }
  else
    GiveHelp(n_HostServ, lptr->nick, NULL, NODCC);
} /* h_help() */

static void
h_on (struct Luser *lptr, int ac, char **av)
{
  toserv(":%s SVSCLOAK %s :%s\r\n",
	Me.name, lptr->nick, GetLink(lptr->nick)->vhost);
}

static void
h_set (struct Luser *lptr, int ac, char **av)
{
  struct NickInfo *user;

  user = GetLink(av[0]);

  MyStrdup(user->vhost, av[1]);

  notice (n_HostServ, av[0], "A virtualhost [\2%s\2] has been set for your nickname by [\2%s\2].",
          user->vhost, lptr->nick);

  toserv(":%s SVSCLOAK %s :%s\r\n",
	Me.name, av[0], user->vhost);

  user->flags |= NS_USERCLOAK;
}
