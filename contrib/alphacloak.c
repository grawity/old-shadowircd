/*
 *  alphacloak.c: Sexier cloaking module.
 *
 *  This is just to prove that I can do this. Consider it mainly a demonstration.
 *          -nenolod
 */

#include "stdinc.h"
#include "tools.h"
#include "s_user.h"
#include "s_conf.h"
#include "client.h"
#include "common.h"
#include "sprintf_irc.h"
#include "hook.h"

static void cloak_host(char *ip)
{
  short i = 0;
      
  while (i < 15) {
    if (ip[i] != (char )0)
      ip[i] =
      (char )ip[i] + (char )52 + ((char )ip[i] / ((char )ip[i - 1] ? (char )ip[i - 1] : ((char)ip[i + 1])  
          ? (char)ip[i + 1] : (char) 48));
    i++;
  }  
}     

void
makevirthost (struct Client *client_p)
{
  char *p;
  char ipmask[16];

  for (p = client_p->host; *p; p++)
    if (*p == '.')
      if (isalpha (*(p + 1)))
        break;

  strcpy(ipmask, client_p->ipaddr);
  cloak_host(ipmask);
  ircsprintf(client_p->virthost, "%s-%s.", ServerInfo.network_name, ipmask);
  strcat(client_p->virthost, isalpha(*(p+1)) ? p : "IP");

  return;
}

void
_modinit(void)
{
  hook_add_hook("make_virthost", (hookfn *)makevirthost);
}

void
_moddeinit(void)
{
  hook_del_hook("make_virthost", (hookfn *)makevirthost);
}

char *_version = "$Revision: 1.1 $";
