/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  m_sqline.c: Global RESV control.
 *
 *  $Id: m_sqline.c,v 1.1 2003/12/03 20:34:03 nenolod Exp $
 */

#include "stdinc.h"
#include "handlers.h"
#include "client.h"
#include "channel.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "s_conf.h"
#include "resv.h"
#include "hash.h"

static void ms_sqline(struct Client *, struct Client *, int, char **);
static void ms_unsqline(struct Client *, struct Client *, int, char **);

struct Message sqline_msgtab = {
  "SQLINE", 0, 0, 3, 0, MFLG_SLOW | MFLG_UNREG, 0,
  {m_ignore, m_not_oper, ms_sqline, m_ignore}
};

struct Message unsqline_msgtab = {
  "UNSQLINE", 0, 0, 2, 0, MFLG_SLOW | MFLG_UNREG, 0,
  {m_ignore, m_not_oper, ms_unsqline, m_ignore}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(&sqline_msgtab);
  mod_add_cmd(&unsqline_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(&sqline_msgtab);
  mod_del_cmd(&unsqline_msgtab);
}

const char *_version = "$Revision: 1.1 $";
#endif

/*
 * ms_sqline()
 *      parv[0] = sender prefix
 *      parv[1] = channel/nick to forbid
 */

static void ms_sqline(struct Client *client_p, struct Client *source_p,
                    int parc, char *parv[])
{
  if(BadPtr(parv[1]))
    return;

  if(IsChannelName(parv[1]))
  {
    struct ResvChannel *resv_p;
    
    resv_p = create_channel_resv(parv[1], parv[2], 0);
  
  }
  else if(clean_resv_nick(parv[1]))
  {

    struct ResvNick *resv_p;

    resv_p = create_nick_resv(parv[1], parv[2], 0);

  }			 

}

/*
 * ms_unsqline()
 *     parv[0] = sender prefix
 *     parv[1] = channel/nick to unforbid
 */

static void ms_unsqline(struct Client *client_p, struct Client *source_p,
                      int parc, char *parv[])
{
  if(IsChannelName(parv[1]))
  {
    struct ResvChannel *resv_p;
    
    if(!ResvChannelList || 
       !(resv_p = (struct ResvChannel *)hash_find_resv(parv[1])))
	return;
  
    else if(resv_p->conf)
      return;	       

    /* otherwise, delete it */
    else
      delete_channel_resv(resv_p);
  }
  else if(clean_resv_nick(parv[1]))
  {
    struct ResvNick *resv_p;

    if(!ResvNickList || !(resv_p = return_nick_resv(parv[1])))
      return;

    else if(resv_p->conf)
      return;

    else
      delete_nick_resv(resv_p);

  }
}
