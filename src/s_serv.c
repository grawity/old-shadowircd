#include "struct.h"
#include "common.h"
#include "sys.h"
#include "numeric.h"
#include "msg.h"
#include "channel.h"
#include <resolv.h>
#include <arpa/nameser.h>
#include "dh.h"
#include "zlink.h"
# include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utmp.h>
#include "h.h"
#include "inet.h"
#include <sys/socket.h>
#if defined( HAVE_STRING_H )
# include <string.h>
#else
  /* older unices don't have strchr/strrchr .. help them out */
# include <strings.h>
# undef strchr
# define strchr index
#endif
#include "dich_conf.h"
#include "fdlist.h"
#include "throttle.h"
#include "version.h"
#include "userban.h"

extern fdlist serv_fdlist;
extern int lifesux;
extern int HTMLOCK;
static char buf[BUFSIZE];
extern int rehashed;

#ifdef HIGHEST_CONNECTION
int max_connection_count = 1, max_client_count = 1;
#endif

/* external variables */

/* external functions */

extern void list_glines(aClient *);

#ifdef MAXBUFFERS
extern void reset_sock_opts ();
#endif

extern char *smalldate (time_t);	/* defined in s_misc.c */
extern void outofmemory (void);	/* defined in list.c */
extern void s_die (void);	/* defined in ircd.c as VOIDSIG */
extern void report_conf_links (aClient *, aConfList *, int, char);
extern void show_opers (aClient *, char *);
extern void show_servers (aClient *, char *);
extern void count_memory (aClient *, char *);
extern void rehash_ip_hash ();	/* defined in s_conf.c */
extern char *find_restartpass ();
extern char *find_diepass ();

extern char *nflagstr (long);
extern char *oflagstr (long);


/* Local function prototypes */

static int isnumber (char *);	/* return 0 if not, else return number */
static char *cluster (char *);

int find_eline (aClient *);

int send_motd (aClient *, aClient *, int, char **);
void read_motd (char *);

int send_opermotd (aClient *, aClient *, int, char **);
void read_opermotd (char *);



void read_shortmotd (char *);

char motd_last_changed_date[MAX_DATE_STRING];	/* enough room for date */

char rules_last_changed_date[MAX_DATE_STRING];	/* enough room for date */

char opermotd_last_changed_date[MAX_DATE_STRING];	/* enough room for date */

#ifdef UNKLINE
static int flush_write (aClient *, char *, int, char *, int, char *);
#endif

#ifdef LOCKFILE
/* Shadowfax's lockfile code */
void do_pending_klines (void);

struct pkl
{
  char *comment;		/* Kline Comment */
  char *kline;			/* Actual Kline */
  struct pkl *next;		/* Next Pending Kline */
}
 *pending_klines = NULL;

time_t pending_kline_time = 0;

#endif /* LOCKFILE */

/* Capability Structure */
struct Capability captab[] = {
/* name cap */
  {"NQ", CAP_NOQUIT},
  {"EX", CAP_EXCEPT},
  {"IE", CAP_INVEX},
  {"QU", CAP_QUIET},
  {"KN", CAP_KNOCK},
  {"HUB", CAP_HUB},
  {"GLN", CAP_GLINE},
  {"UID", CAP_UID},
  { 0, 0 },
};

/*
 * m_functions execute protocol messages on this server:
 *
 * client_p:
 * always NON-NULL, pointing to a *LOCAL* client
 * structure (with an open socket connected!). This
 * is the physical socket where the message originated (or
 * which caused the m_function to be executed--some
 * m_functions may call others...).
 *
 * source_p:
 * the source of the message, defined by the
 * prefix part of the message if present. If not or
 * prefix not found, then source_p==client_p.
 *
 *      *Always* true (if 'parse' and others are working correct):
 *
 *      1)      source_p->from == client_p  (note: client_p->from == client_p)
 *
 *      2)      MyConnect(source_p) <=> source_p == client_p (e.g. source_p
 *              cannot be a local connection, unless it's actually client_p!).
 *
 * MyConnect(x) should probably  be defined as (x == x->from) --msa
 *
 * parc:
 * number of variable parameter strings (if zero,
 * parv is allowed to be NULL)
 *
 * parv:
 * a NULL terminated list of parameter pointers,
 * parv[0], sender (prefix string), if not present his points to
 * an empty string.
 *
 * [parc-1]:
 * pointers to additional parameters
 * parv[parc] == NULL, *always*
 *
 * note:   it is guaranteed that parv[0]..parv[parc-1] are all
 *         non-NULL pointers.
 */


/*
 * * m_version
 *      parv[0] = sender prefix
 *      parv[1] = remote server
 */
int
m_version (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  extern char serveropts[];

  if (hunt_server (client_p, source_p, ":%s VERSION :%s", 1, parc, parv) ==
      HUNTED_ISME)
    {
      sendto_one (source_p, rpl_str (RPL_VERSION), me.name, parv[0], version,
		  me.name, RELEASE_STATUS, serveropts, UltimateProtocol,
		  debugmode);

      show_isupport (source_p);
    }
  return 0;
}

void
send_capabilities(struct Client *client_p, int cap_can_send)
{
  struct Capability *cap;
  char  msgbuf[BUFSIZE];
  char  *t;
  int   tl;

  t = msgbuf;

  for (cap = captab; cap->name; ++cap)
    {
      if (cap->cap & cap_can_send)
        {
          tl = ircsprintf(t, "%s ", cap->name);
          t += tl;
        }
    }

  t--;
  *t = '\0';
  sendto_one(client_p, "CAPAB :%s", msgbuf);
}

const char*
show_capabilities(struct Client* target_p)
{
  static char        msgbuf[BUFSIZE];
  struct Capability* cap;
  char *t;
  int  tl;

  t = msgbuf;
  tl = ircsprintf(msgbuf,"TS ");
  t += tl;

  for (cap = captab; cap->cap; ++cap)
    {
      if(cap->cap & target_p->capabilities)
        {
          tl = ircsprintf(t, "%s ", cap->name);
          t += tl;
        }
    }

  t--;
  *t = '\0';

  return msgbuf;
}


/*
 * m_squit
 * there are two types of squits: those going downstream (to the target server)
 * and those going back upstream (from the target server).
 * previously, it wasn't necessary to distinguish between these two types of
 * squits because they neatly echoed back all of the QUIT messages during an squit.
 * This, however, is no longer practical.
 *
 * To clarify here, DOWNSTREAM signifies an SQUIT heading towards the target server
 * UPSTREAM signifies an SQUIT which has successfully completed, heading out everywhere.
 *
 * target_p is the server being squitted.
 * a DOWNSTREAM squit is where the notice did not come from target_p->from.
 * an UPSTREAM squit is where the notice DID come from target_p->from.
 *
 *        parv[0] = sender prefix
 *        parv[1] = server name
 *	  parv[2] = comment
 */

int
m_squit (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aConfItem *aconf;
  char *server;
  aClient *target_p;
  char *comment = (parc > 2) ? parv[2] : source_p->name;

  if (!IsPrivileged (source_p))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }
  if (parc > 1)
    {
      server = parv[1];
      /* To accomodate host masking, a squit for a masked server
       * name is expanded if the incoming mask is the same as the
       * server name for that link to the name of link.
       */
      while ((*server == '*') && IsServer (client_p))
	{
	  aconf = client_p->serv->nline;
	  if (!aconf)
	    break;
	  if (irccmp (server, my_name_for_link (me.name, aconf)) == 0)
	    server = client_p->name;
	  break;		/* WARNING is normal here */
	  /* NOTREACHED */
	}
      /*
       * * The following allows wild cards in SQUIT. Only useful * when
       * the command is issued by an oper.
       */
      for (target_p = client; (target_p = next_client (target_p, server));
	   target_p = target_p->next)
	if (IsServer (target_p) || IsMe (target_p))
	  break;
      if (target_p && IsMe (target_p))
	{
	  /*
	   * This evil oper want to squit the server hes on .. we are not there *hide* - Againaway
	   */
	  sendto_one (source_p, err_str (ERR_NOSUCHSERVER),
		      me.name, parv[0], "SQUIT");
	  return 0;
	}
    }
  else
    {
      if (!IsServer (client_p))
	{
	  /*
	   * We now warn the user about missing params instead of killing him off - ShadowMaster
	   * Itss better if we say we are not there :) - AgY
	   */
	  sendto_one (source_p, err_str (ERR_NOSUCHSERVER),
		      me.name, parv[0], "SQUIT");
	  return 0;
	}
      else
	{
	  /* This is actually protocol error. But, well, closing the
	   * link is very proper answer to that...
	   */
	  server = client_p->sockhost;
	  target_p = client_p;
	}
    }
  if (!target_p)
    {
      sendto_one (source_p, err_str (ERR_NOSUCHSERVER), me.name, parv[0],
		  server);
      return 0;
    }


  if (MyClient (source_p)
      && (((source_p->oflag & OFLAG_GROUTE) && !MyConnect (target_p))
	  || (!(source_p->oflag & OFLAG_LROUTE) && MyConnect (target_p))))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }

  /* If the server is mine, we don't care about upstream or downstream,
     just kill it and do the notice. */

  if (MyConnect (target_p))
    {
      sendto_snomask (SNOMASK_NETINFO, "from %s: Received SQUIT %s from %s (%s)",
		      me.name, target_p->name, get_client_name (source_p,
								HIDEME),
		      comment);
      sendto_serv_butone (NULL, ":%s GNOTICE :Received SQUIT %s from %s (%s)",
			  me.name, server, get_client_name (source_p, HIDEME),
			  comment);

#if defined(USE_SYSLOG) && defined(SYSLOG_SQUIT)
      syslog (LOG_DEBUG, "SQUIT From %s : %s (%s)", parv[0], server, comment);
#endif
      /* I am originating this squit! Not client_p! */
      /* ack, but if client_p is squitting itself.. */
      if (client_p == source_p)
	{
	  exit_client (&me, target_p, source_p, comment);
	  return FLUSH_BUFFER;	/* kludge */
	}
      return exit_client (&me, target_p, source_p, comment);
    }

  /* the server is not connected to me. Determine whether this is an upstream
     or downstream squit */

  if (source_p->from == target_p->from)	/* upstream */
    {
      sendto_snomask (SNOMASK_DEBUG,
			  "Exiting server %s due to upstream squit by %s [%s]",
			  target_p->name, source_p->name, comment);
      return exit_client (client_p, target_p, source_p, comment);
    }

  /* fallthrough: downstream */

  if (!(IsUnconnect (target_p->from)))	/* downstream not unconnect capable */
    {
      sendto_snomask (SNOMASK_DEBUG,
			  "Exiting server %s due to non-unconnect server %s [%s]",
			  target_p->name, target_p->from->name, comment);
      return exit_client (&me, target_p, source_p, comment);
    }

  sendto_snomask (SNOMASK_DEBUG, "Passing along SQUIT for %s by %s [%s]",
		      target_p->name, source_p->name, comment);
  sendto_one (target_p->from, ":%s SQUIT %s :%s", parv[0], target_p->name,
	      comment);

  return 0;
}

/*
 * * ts_servcount *   returns the number of TS servers that are
 * connected to us
 */
int
ts_servcount ()
{
  int i;
  aClient *target_p;
  int r = 0;

  for (i = 0; i <= highest_fd; i++)
    if ((target_p = local[i]) && IsServer (target_p) && DoesTS (target_p))
      r++;
  return r;
}


/*
** m_netctrl
**
** Replacing m_vctrl in a not so pretty cleanup.
*/

int
m_netctrl (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  return 0;
}

/*
** m_vctrl
**
** Used to maintain compactibility on several levels and to make sure all servers on the network
** are running with the same network settings where it is required that they stay the same.
** vctrl is also used to check before any client information is actually passed on so that
** the servers wont have to waste a lot of time and bandwith synching all its data only to discover
** "out oh crap were not compactible with eachother" - ShadowMaster
**
**  parv[0] = sender prefix
**  parv[1] = ultimate protocol
**  parv[2] = nickname lenght
**  parv[3] = Mode x
**  parv[4] = global connec notices
**  parv[5] = Reserved for future extentions
**  parv[6] = Reserved for future extentions
**  parv[7] = Reserved for future extentions
**  parv[8] = Reserved for future extentions
**  parv[9] = Reserved for future extentions
**  parv[10] = Reserved for future extentions
**  parv[11] = Reserved for future extentions
**  parv[12] = Reserved for future extentions
**  parv[13] = Reserved for future extentions
**  parv[14] = Reserved for future extentions
**  parv[15] = Networks Settings File
**/

int
m_vctrl (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  return 0;
/*
  char tempbuff[500];

  if (!IsServer (source_p) || !MyConnect (source_p) || parc < 14)
    return 0;
*/
/*
	if (GotVCtrl(client_p))
	{
		sendto_netinfo("Already got VCTRL from Link %s", client_p->name);
		return 0;
	}
*/


  /*
   **
   ** First off we want to disallow links who have messed up their network settings file so that they cant possibly work
   ** with our settings - ShadowMaster
   **
   */
  /*
     if (atol (parv[1]) < KillProtocol)
     {
     sendto_one (client_p,
     "ERROR :UltimateProtocol %s is not compatible with UltimateProtocol %l. Upgrade to a version with Protocol %l or higher.",
     parv[1], UltimateProtocol, KillProtocol + 1);
     sendto_snomask
     (SNOMASK_NETINFO, "from %s : Link %s is running Protocol \2%s\2. Dropped Link. Only accepting links from Procol 
\2%l\2 or higher.",
     me.name, client_p->name, parv[1], KillProtocol + 1);
     ircsprintf (tempbuff,
     "Link using UltimateProtocol %s. Minimum Protocol required is %l.",
     parv[1], KillProtocol + 1);
     return exit_client (client_p, client_p, client_p, tempbuff);
     return 0;
     }


     if (atol (parv[2]) != NICKLEN)
     {
     sendto_one (client_p,
     "ERROR :Your max nicklenght is \2%s\2. All servers needs to have the same. We have \2%l\2",
     parv[2], NICKLEN);
     sendto_snomask
     (SNOMASK_NETINFO, "Link %s allows nicknames \2%s\2 chars long while we allow \2%l\2 Dropped link Link.",
     client_p->name, parv[2], NICKLEN);
     ircsprintf (tempbuff,
     "Link allowing nicks that are %l chars long. Mismatch, we use %l chars long.",
     parv[1], NICKLEN);
     return exit_client (client_p, client_p, client_p, tempbuff);
     return 0;

     }

     if (atoi (parv[4]) != MODE_x)
     {
     sendto_one (client_p,
     "ERROR :Your net_mode_x is \2%s\2. All servers needs to have the same. We have \2%l\2",
     parv[4], MODE_x);
     sendto_snomask
     (SNOMASK_NETINFO, "Link %s have net_mode_x set to \2%s\2 while we have \2%l\2. Dropped link Link.",
     client_p->name, parv[4], MODE_x);
     ircsprintf (tempbuff,
     "Link having net_mode_x set to %s. Mismatch, we have it set to %l.",
     parv[4], MODE_x);
     return exit_client (client_p, client_p, client_p, tempbuff);
     return 0;

     }


     if (!strcmp (IRCNETWORK, parv[15]) == 0)
     {
     sendto_one (client_p,
     "ERROR :Only accepting links from servers with same networks file. Your using Networks file: \2%s\2. File required: \2%s\2.",
     parv[15], IRCNETWORK);
     sendto_snomask
     (SNOMASK_NETINFO, "Network name mismatch from link %s. (Theirs: \2%s\2 Ours: \2%s\2) Dropped the link.",
     client_p->name, parv[15], IRCNETWORK);
     ircsprintf (tempbuff,
     "Link using Networks File %s. File required is %s.",
     parv[15], IRCNETWORK);
     return exit_client (client_p, client_p, client_p, tempbuff);
     return 0;
     }

   */
  /*
   **
   ** Ok, we allow the link, but we warn if we have something set differenly on potential important settings.
   **
   */
/*
  if (atol (parv[1]) != UltimateProtocol)
    {
      sendto_snomask
	(SNOMASK_NETINFO, "Warning: Link %s is running Protocol %s while we are running %l!",
	 client_p->name, parv[1], UltimateProtocol);
    }


  if (atoi (parv[3]) != GLOBAL_CONNECTS)
    {
      sendto_snomask
	(SNOMASK_NETINFO, "Link %s have set Global Connect Notices to \2%s\2 while we have \2%d\2. Please contact the Network Administration about this.",
	 client_p->name, parv[3], GLOBAL_CONNECTS);
    }
*/
  /*SetVCtrl(client_p); */
/*
  return 0;
*/
}


/*
 * * m_svinfo
 *       parv[0] = sender prefix
 *       parv[1] = TS_CURRENT for the server
 *       parv[2] = TS_MIN for the server
 *       parv[3] = server is standalone or connected to non-TS only
 *       parv[4] = server's idea of UTC time
 */
int
m_svinfo (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  time_t deltat, tmptime, theirtime;

  if (!IsServer (source_p) || !MyConnect (source_p))
    return 0;

  if (parc == 2 && (irccmp (parv[1], "ZIP") == 0))
    {
      SetZipIn (source_p);
      source_p->serv->zip_in = zip_create_input_session ();
      return ZIP_NEXT_BUFFER;
    }

  if (parc < 5 || !DoesTS (source_p))
    return 0;

  if (TS_CURRENT < atoi (parv[2]) || atoi (parv[1]) < TS_MIN)
    {
      /* a server with the wrong TS version connected; since we're
       * TS_ONLY we can't fall back to the non-TS protocol so we drop
       * the link  -orabidoo
       */
      sendto_realops ("Link %s dropped, wrong TS protocol version (%s,%s)",
		      get_client_name (source_p, HIDEME), parv[1], parv[2]);
      return exit_client (source_p, source_p, source_p,
			  "Incompatible TS version");
    }

  tmptime = time (NULL);
  theirtime = atol (parv[4]);
  deltat = abs (theirtime - tmptime);

  if (deltat > TS_MAX_DELTA)
    {
      sendto_snomask
	(SNOMASK_NETINFO, "from %s: Link %s dropped, excessive TS delta (my TS=%d, their TS=%d, delta=%d)",
	 me.name, get_client_name (source_p, HIDEME), tmptime, theirtime,
	 deltat);
      sendto_serv_butone (source_p,
			  ":%s GNOTICE :Link %s dropped, excessive TS delta (delta=%d)",
			  me.name, get_client_name (source_p, HIDEME),
			  deltat);
      return exit_client (source_p, source_p, source_p, "Excessive TS delta");
    }

  if (deltat > TS_WARN_DELTA)
    {
      sendto_realops
	("Link %s notable TS delta (my TS=%d, their TS=%d, delta=%d)",
	 get_client_name (source_p, HIDEME), tmptime, theirtime, deltat);
    }

  return 0;
}

/*
 * * m_server *       parv[0] = sender prefix *       parv[1] = servername *
 * parv[2] = serverinfo/hopcount *      parv[3] = serverinfo
 */
int
m_server (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  int i;
  char info[REALLEN + 1], *inpath, *host;
  aClient *target_p, *bclient_p;
  aConfItem *aconf;
  int hop;
  char nbuf[HOSTLEN * 2 + USERLEN + 5];	/* same size as in s_misc.c */

  info[0] = '\0';
  inpath = get_client_name (client_p, HIDEME);

  if (parc < 2 || *parv[1] == '\0')
    {
      sendto_one (client_p, "ERROR :No servername");
      return 0;
    }

  hop = 0;
  host = parv[1];
  if (parc > 3 && atoi (parv[2]))
    {
      hop = atoi (parv[2]);
      strncpyzt (info, parv[3], REALLEN);
    }
  else if (parc > 2)
    {
      strncpyzt (info, parv[2], REALLEN);
      if ((parc > 3) && ((i = strlen (info)) < (REALLEN - 2)))
	{
	  (void) strcat (info, " ");
	  (void) strncat (info, parv[3], REALLEN - i - 2);
	  info[REALLEN] = '\0';
	}
    }
  /*
   * * July 5, 1997
   * Rewritten to throw away server cruft from users,
   * combined the hostname validity test with cleanup of host name,
   * so a cleaned up hostname can be returned as an error if
   * necessary. - Dianora
   */

  /* yes, the if(strlen) below is really needed!! */
  if (strlen (host) > HOSTLEN)
    host[HOSTLEN] = '\0';

  if (IsPerson (client_p))
    {
      /* A local link that has been identified as a USER tries
       * something fishy... ;-)
       */
      sendto_one (client_p, err_str (ERR_UNKNOWNCOMMAND),
		  me.name, parv[0], "SERVER");

      return 0;
    }
  else
    {
      /* hostile servername check */

      /*
       * Lets check for bogus names and clean them up we don't bother
       * cleaning up ones from users, becasuse we will never see them
       * any more - Dianora
       */


      int bogus_server = 0;
      int found_dot = 0;
      char clean_host[(2 * HOSTLEN) + 1];
      char *s;
      char *d;
      int n;

      s = host;
      d = clean_host;
      n = (2 * HOSTLEN) - 2;

      while (*s && n > 0)
	{
	  if ((unsigned char) *s < (unsigned char) ' ')	/* Is it a control character? */
	    {
	      bogus_server = 1;
	      *d++ = '^';
	      *d++ = (char) ((unsigned char) *s + 0x40);	/* turn it into a printable */
	      n -= 2;
	    }
	  else if ((unsigned char) *s > (unsigned char) '~')
	    {
	      bogus_server = 1;
	      *d++ = '.';
	      n--;
	    }
	  else
	    {
	      if (*s == '.')
		found_dot = 1;
	      *d++ = *s;
	      n--;
	    }
	  s++;
	}
      *d = '\0';

      if ((!found_dot) || bogus_server)
	{
	  sendto_one (source_p, "ERROR :Bogus server name (%s)", clean_host);
	  return exit_client (client_p, client_p, client_p,
			      "Bogus server name");
	}
    }

  /*
   * check to see this host even has an N line before bothering 
   * anyone about it. Its only a quick sanity test to stop the
   * conference room and win95 ircd dorks. Sure, it will be
   * redundantly checked again in m_server_estab() *sigh* yes there
   * will be wasted CPU as the conf list will be scanned twice. But
   * how often will this happen? - Dianora
   *
   * This should (will be) be recoded to check the IP is valid as well,
   * with a pointer to the valid N line conf kept for later, saving an
   * extra lookup.. *sigh* - Dianora
   */
  if (!IsServer (client_p))
    {
      if (find_conf_name (host, CONF_NOCONNECT_SERVER) == NULL)
	{

#ifdef WARN_NO_NLINE
	  sendto_realops ("Link %s dropped, no N: line",
			  get_client_name (client_p, TRUE));
#endif

	  return exit_client (client_p, client_p, client_p, "NO N line");
	}
    }

  if ((target_p = find_name (host, NULL)))
    {
      /*
       * * This link is trying feed me a server that I already have
       * access through another path -- multiple paths not accepted
       * currently, kill this link immediately!!
       *
       * Rather than KILL the link which introduced it, KILL the
       * youngest of the two links. -avalon
       */

      bclient_p =
	(client_p->firsttime >
	 target_p->from->firsttime) ? client_p : target_p->from;
      sendto_one (bclient_p, "ERROR :Server %s already exists", host);
      if (bclient_p == client_p)
	{
	  sendto_snomask
	    (SNOMASK_NETINFO, "from %s: Link %s cancelled, server %s already exists", me.name,
	     get_client_name (bclient_p, HIDEME), host);
	  sendto_serv_butone (bclient_p,
			      ":%s GNOTICE :Link %s cancelled, server %s already exists",
			      me.name, get_client_name (bclient_p, HIDEME),
			      host);
	  return exit_client (bclient_p, bclient_p, &me, "Server Exists");
	}
      /* inform all those who care (set +n) -epi */
      strcpy (nbuf, get_client_name (bclient_p, HIDEME));
      sendto_snomask
	(SNOMASK_NETINFO, "from %s: Link %s cancelled, server %s reintroduced by %s", me.name,
	 nbuf, host, get_client_name (client_p, HIDEME));
      sendto_serv_butone (bclient_p,
			  ":%s GNOTICE :Link %s cancelled, server %s reintroduced by %s",
			  me.name, nbuf, host, get_client_name (client_p,
								HIDEME));
      (void) exit_client (bclient_p, bclient_p, &me, "Server Exists");
    }
  /*
   * The following if statement would be nice to remove since user
   * nicks never have '.' in them and servers must always have '.' in
   * them. There should never be a server/nick name collision, but it
   * is possible a capricious server admin could deliberately do
   * something strange.
   *
   * -Dianora
   */

  if ((target_p = find_client (host, NULL)) && target_p != client_p)
    {
      /*
       * * Server trying to use the same name as a person. Would
       * cause a fair bit of confusion. Enough to make it hellish for
       * a while and servers to send stuff to the wrong place.
       */
      sendto_one (client_p, "ERROR :Nickname %s already exists!", host);
      strcpy (nbuf, get_client_name (client_p, HIDEME));
      sendto_snomask (SNOMASK_NETINFO, "from %s: Link %s cancelled, servername/nick collision",
		      me.name, nbuf);
      sendto_serv_butone (client_p,
			  ":%s GNOTICE :Link %s cancelled, servername/nick collision",
			  me.name, nbuf);
      return exit_client (client_p, client_p, client_p, "Nick as Server");
    }

  if (IsServer (client_p))
    {
      /*
       * * Server is informing about a new server behind this link.
       * Create REMOTE server structure, add it to list and propagate
       * word to my other server links...
       */
      if (parc == 1 || info[0] == '\0')
	{
	  sendto_one (client_p, "ERROR :No server info specified for %s",
		      host);
	  return 0;
	}
      /* L/H line code also removed. It's deprecated. */
      /* Q: line code removed. Q: lines are not a useful feature on a
       * modern net.
       */

      target_p = make_client (client_p, source_p);
      (void) make_server (target_p);
      target_p->hopcount = hop;
      strncpyzt (target_p->name, host, sizeof (target_p->name));
      strncpyzt (target_p->info, info, REALLEN);
      target_p->serv->up = find_or_add (parv[0]);

      SetServer (target_p);

      /* if this server is behind a U-lined server, make it U-lined as well. - lucas */
      /* i'm not sure if we want to do this, but we'll keep it for now. --nenolod */

      if (IsULine (source_p)
	  || (find_uline (client_p->confs, target_p->name)))
	{
	  target_p->flags |= FLAGS_ULINE;
	  sendto_realops ("%s introducing U:lined server %s", client_p->name,
			  target_p->name);
	}

      Count.server++;

      add_client_to_list (target_p);
      (void) add_to_client_hash_table (target_p->name, target_p);
      /*
       * * Old sendto_serv_but_one() call removed because we now need
       * to send different names to different servers (domain name matching)
       */
      for (i = 0; i <= highest_fd; i++)
	{
	  if (!(bclient_p = local[i]) || !IsServer (bclient_p)
	      || bclient_p == client_p || IsMe (bclient_p))
	    continue;
	  if (!(aconf = bclient_p->serv->nline))
	    {
	      sendto_snomask (SNOMASK_NETINFO, "from %s: Lost N-line for %s on %s. Closing",
			      me.name, get_client_name (client_p, HIDEME),
			      host);
	      sendto_serv_butone (client_p,
				  ":%s GNOTICE :Lost N-line for %s on %s. Closing",
				  me.name, get_client_name (client_p, HIDEME),
				  host);
	      return exit_client (client_p, client_p, client_p,
				  "Lost N line");
	    }
	  if (match (my_name_for_link (me.name, aconf), target_p->name) == 0)
	    continue;
	  sendto_one (bclient_p, ":%s SERVER %s %d :%s",
		      parv[0], target_p->name, hop + 1, target_p->info);
	}
      return 0;
    }

  if (!IsUnknown (client_p) && !IsHandshake (client_p))
    return 0;
  /*
   * * A local link that is still in undefined state wants to be a
   * SERVER. Check if this is allowed and change status
   * accordingly...
   */
  /*
   * * Reject a direct nonTS server connection if we're TS_ONLY
   * -orabidoo
   */

  strncpyzt (client_p->name, host, sizeof (client_p->name));
  strncpyzt (client_p->info, info[0] ? info : me.name, REALLEN);
  client_p->hopcount = hop;

  switch (check_server_init (client_p))
    {
    case 0:
      return m_server_estab (client_p);
    case 1:
      sendto_realops ("Access check for %s in progress",
		      get_client_name (client_p, HIDEME));
      return 1;
    default:
      ircstp->is_ref++;
      sendto_realops ("Received unauthorized connection from %s.",
		      get_client_host (client_p));
      return exit_client (client_p, client_p, client_p, "No C/N conf lines");
    }

}

static void
sendnick_TS (aClient * client_p, aClient * target_p)
{
  static char ubuf[34];
  static char sbuf[34];

  if (IsPerson (target_p))
    {
      send_umode (NULL, target_p, 0, SEND_UMODES, ubuf);
      if (!*ubuf)		/* trivial optimization - Dianora */
	{
	  ubuf[0] = '+';
	  ubuf[1] = '\0';
	}

      if (IsClientCapable (client_p))
	{
	  sendto_one (client_p,
		      "CLIENT %s %d %ld %s %s %s %s %s %s %lu %lu :%s",
		      target_p->name, target_p->hopcount + 1,
		      target_p->tsinfo, ubuf, "*", target_p->user->username,
		      target_p->user->host,
		      (IsHidden (target_p) ? target_p->user->virthost : "*"),
		      target_p->user->server, target_p->user->servicestamp,
		      htonl (target_p->ip.S_ADDR), target_p->info);
	}
      else if (IsNICKIP (client_p) && !IsClientCapable (client_p))
	{
	  sendto_one (client_p, "NICK %s %d %ld %s %s %s %s %lu %lu :%s",
		      target_p->name, target_p->hopcount + 1,
		      target_p->tsinfo, ubuf, target_p->user->username,
		      target_p->user->host, target_p->user->server,
		      target_p->user->servicestamp,
		      htonl (target_p->ip.S_ADDR), target_p->info);

	  if (IsHidden (target_p))
	    {
	      sendto_one (client_p, ":%s SETHOST %s %s", me.name,
			  target_p->name, target_p->user->virthost);
	    }
	}
      else
	{
	  sendto_one (client_p, "NICK %s %d %ld %s %s %s %s %lu :%s",
		      target_p->name, target_p->hopcount + 1,
		      target_p->tsinfo, ubuf, target_p->user->username,
		      target_p->user->host, target_p->user->server,
		      target_p->user->servicestamp, target_p->info);

	  if (IsHidden (target_p))
	    {
	      sendto_one (client_p, ":%s SETHOST %s %s", me.name,
			  target_p->name, target_p->user->virthost);
	    }
	}
    }
}



int
do_server_estab (aClient * client_p)
{
  aClient *target_p;
  aConfItem *aconf;
  aChannel *channel_p;
  int i;
  char *inpath = get_client_name (client_p, HIDEME);	/* "refresh" inpath with host  */

  SetServer (client_p);

  Count.server++;
  Count.myserver++;

  if (IsZipCapable (client_p) && DoZipThis (client_p))
    {
      sendto_one (client_p, "SVINFO ZIP");
      SetZipOut (client_p);
      client_p->serv->zip_out = zip_create_output_session ();
    }

#ifdef MAXBUFFERS
  /* let's try to bump up server sock_opts... -Taner */
  reset_sock_opts (client_p->fd, 1);
#endif

  /* adds to fdlist */
  addto_fdlist (client_p->fd, &serv_fdlist);

#ifndef NO_PRIORITY
  /* this causes the server to be marked as "busy" */
  check_fdlists ();
#endif

  client_p->pingval = get_client_ping (client_p);
  client_p->sendqlen = get_sendq (client_p);


  /* Check one more time for good measure... is it there? */
  if ((target_p = find_name (client_p->name, NULL)))
    {
      char nbuf[HOSTLEN * 2 + USERLEN + 5];	/* same size as in s_misc.c */
      aClient *bclient_p;

      /*
       * While negotiating stuff, another copy of this server appeared.
       *
       * Rather than KILL the link which introduced it, KILL the
       * youngest of the two links. -avalon
       */

      bclient_p =
	(client_p->firsttime >
	 target_p->from->firsttime) ? client_p : target_p->from;
      sendto_one (bclient_p, "ERROR :Server %s already exists",
		  client_p->name);
      if (bclient_p == client_p)
	{
	  sendto_snomask (SNOMASK_NETINFO, "from %s: Link %s cancelled, server %s already "
			  "exists (final phase)", me.name,
			  get_client_name (bclient_p, HIDEME),
			  client_p->name);
	  sendto_serv_butone (bclient_p,
			      ":%s GNOTICE :Link %s cancelled, "
			      "server %s already exists (final phase)",
			      me.name, get_client_name (bclient_p, HIDEME),
			      client_p->name);
	  return exit_client (bclient_p, bclient_p, &me,
			      "Server Exists (final phase)");
	}
      /* inform all those who care (set +n) -epi */
      strcpy (nbuf, get_client_name (bclient_p, HIDEME));
      sendto_snomask (SNOMASK_NETINFO, "from %s: Link %s cancelled, server %s reintroduced "
		      "by %s (final phase)", me.name, nbuf, client_p->name,
		      get_client_name (client_p, HIDEME));
      sendto_serv_butone (bclient_p,
			  ":%s GNOTICE :Link %s cancelled, server %s "
			  "reintroduced by %s (final phase)", me.name, nbuf,
			  client_p->name, get_client_name (client_p, HIDEME));
      (void) exit_client (bclient_p, bclient_p, &me,
			  "Server Exists (final phase)");
    }

  /* error, error, error! if a server is U:'d, and it connects to us,
   * we need to figure that out! So, do it here. - lucas
   */

  if ((find_uline (client_p->confs, client_p->name)))
    {
      Count.myulined++;
      client_p->flags |= FLAGS_ULINE;
    }

  sendto_snomask
    (SNOMASK_NETINFO, "from %s: Link with %s established, capabilities(%s)",
	me.name, inpath, show_capabilities(client_p));

  /* Notify everyone of the fact that this has just linked: the entire network should get two
     of these, one explaining the link between me->serv and the other between serv->me */

  sendto_serv_butone (NULL,
		      ":%s GNOTICE :Link with %s established, capabilities(%s)",
		      me.name, inpath, show_capabilities(client_p));

  (void) add_to_client_hash_table (client_p->name, client_p);

  /* add it to scache */

  (void) find_or_add (client_p->name);

  /*
   * * Old sendto_serv_but_one() call removed because we now need to
   * send different names to different servers (domain name
   * matching) Send new server to other servers.
   */
  for (i = 0; i <= highest_fd; i++)
    {
      if (!(target_p = local[i]) || !IsServer (target_p)
	  || target_p == client_p || IsMe (target_p))
	continue;
      if ((aconf = target_p->serv->nline)
	  && !match (my_name_for_link (me.name, aconf), client_p->name))
	continue;
      sendto_one (target_p, ":%s SERVER %s 2 :%s", me.name, client_p->name,
		  client_p->info);
    }

  /*
   * Pass on my client information to the new server
   *
   * First, pass only servers (idea is that if the link gets
   * cancelled beacause the server was already there, there are no
   * NICK's to be cancelled...). Of course, if cancellation occurs,
   * all this info is sent anyway, and I guess the link dies when a
   * read is attempted...? --msa
   *
   * Note: Link cancellation to occur at this point means that at
   * least two servers from my fragment are building up connection
   * this other fragment at the same time, it's a race condition,
   * not the normal way of operation...
   *
   * ALSO NOTE: using the get_client_name for server names-- see
   * previous *WARNING*!!! (Also, original inpath is
   * destroyed...)
   */

  aconf = client_p->serv->nline;
  for (target_p = &me; target_p; target_p = target_p->prev)
    {
      /* target_p->from == target_p for target_p == client_p */
      if (target_p->from == client_p)
	continue;
      if (IsServer (target_p))
	{
	  if (match (my_name_for_link (me.name, aconf), target_p->name) == 0)
	    continue;
	  sendto_one (client_p, ":%s SERVER %s %d :%s",
		      target_p->serv->up, target_p->name,
		      target_p->hopcount + 1, target_p->info);
	}
    }

  /* send out our SQLINES and SGLINES too */
  for (aconf = conf; aconf; aconf = aconf->next)
    {
      if ((aconf->status & CONF_QUARANTINE) && (aconf->status & CONF_SQLINE))
	sendto_one (client_p, ":%s SQLINE %s :%s", me.name,
		    aconf->name, aconf->passwd);
      if ((aconf->status & CONF_GCOS) && (aconf->status & CONF_SGLINE))
	sendto_one (client_p, ":%s SGLINE %d :%s:%s", me.name,
		    strlen (aconf->name), aconf->name, aconf->passwd);
    }

  /* Bursts are about to start.. send a BURST */
  if (IsBurst (client_p))
    sendto_one (client_p, "BURST");

  /* send out our KLINES too, after all, we burst KLINES. */
  
  /*
   * * Send it in the shortened format with the TS, if it's a TS
   * server; walk the list of channels, sending all the nicks that
   * haven't been sent yet for each channel, then send the channel
   * itself -- it's less obvious than sending all nicks first, but
   * on the receiving side memory will be allocated more nicely 
   * saving a few seconds in the handling of a split -orabidoo
   */
  {
    chanMember *cm;
    static char nickissent = 1;

    nickissent = 3 - nickissent;
    /*
     * flag used for each nick to check if we've sent it yet - must
     * be different each time and !=0, so we alternate between 1 and
     * 2 -orabidoo
     */
    for (channel_p = channel; channel_p; channel_p = channel_p->nextch)
      {
	for (cm = channel_p->members; cm; cm = cm->next)
	  {
	    target_p = cm->client_p;
	    if (target_p->nicksent != nickissent)
	      {
		target_p->nicksent = nickissent;
		if (target_p->from != client_p)
		  sendnick_TS (client_p, target_p);
	      }
	  }
	send_channel_modes (client_p, channel_p);
      }
    /* also send out those that are not on any channel */
    for (target_p = &me; target_p; target_p = target_p->prev)
      if (target_p->nicksent != nickissent)
	{
	  target_p->nicksent = nickissent;
	  if (target_p->from != client_p)
	    sendnick_TS (client_p, target_p);
	}
  }

  if (ZipOut (client_p))
    {
      unsigned long inb, outb;
      double rat;

      zip_out_get_stats (client_p->serv->zip_out, &inb, &outb, &rat);

      if (inb)
	{
	  sendto_snomask
	    (SNOMASK_NETINFO, "from %s: Connect burst to %s: %lu bytes normal, %lu compressed (%3.2f%%)",
	     me.name, get_client_name (client_p, HIDEME), inb, outb, rat);
	  sendto_serv_butone (client_p,
			      ":%s GNOTICE :Connect burst to %s: %lu bytes normal, %lu compressed (%3.2f%%)",
			      me.name, get_client_name (client_p, HIDEME),
			      inb, outb, rat);
	}
    }

  /* stuff a PING at the end of this burst so we can figure out when
     the other side has finished processing it. */
  client_p->flags |= FLAGS_BURST | FLAGS_PINGSENT;
  if (IsBurst (client_p))
    client_p->flags |= FLAGS_SOBSENT;
  sendto_one (client_p, "PING :%s", me.name);

  return 0;
}

int
m_server_estab (aClient * client_p)
{
  aConfItem *aconf, *bconf;

  char *inpath, *host, *s, *encr;
/*  int split;*/
  int i;

  inpath = get_client_name (client_p, HIDEME);	/* "refresh" inpath with host  */
/*  split = irccmp (client_p->name, client_p->sockhost);*/
  host = client_p->name;

  if (!(aconf = find_conf (client_p->confs, host, CONF_NOCONNECT_SERVER)))
    {
      ircstp->is_ref++;
      sendto_one (client_p, "ERROR :Access denied. No N line for server %s",
		  inpath);
      sendto_realops ("Access denied. No N line for server %s", inpath);
      return exit_client (client_p, client_p, client_p,
			  "No N line for server");
    }
  if (!(bconf = find_conf (client_p->confs, host, CONF_CONNECT_SERVER)))
    {
      ircstp->is_ref++;
      sendto_one (client_p, "ERROR :Only N (no C) field for server %s",
		  inpath);
      sendto_realops ("Only N (no C) field for server %s", inpath);
      return exit_client (client_p, client_p, client_p,
			  "No C line for server");
    }

  encr = client_p->passwd;
  if (*aconf->passwd && !StrEq (aconf->passwd, encr))
    {
      ircstp->is_ref++;
      sendto_one (client_p, "ERROR :No Access (passwd mismatch) %s", inpath);
      sendto_realops ("Access denied (passwd mismatch) %s", inpath);
      return exit_client (client_p, client_p, client_p, "Bad Password");
    }
  memset (client_p->passwd, '\0', sizeof (client_p->passwd));

  if (HUB == 0)
    {
      for (i = 0; i <= highest_fd; i++)
	if (local[i] && IsServer (local[i]))
	  {
	    ircstp->is_ref++;
	    sendto_one (client_p, "ERROR :I'm a leaf not a hub");
	    return exit_client (client_p, client_p, client_p, "I'm a leaf");
	  }
    }


  /* aconf->port is a CAPAB field, kind-of. kludge. mm, mm. */
  client_p->capabilities |= aconf->port;
  if (IsUnknown (client_p))
    {
      if (bconf->passwd[0])
	sendto_one (client_p, "PASS %s :TS", bconf->passwd);

      /* Pass my info to the new server */

      send_capabilities (client_p, CAPABS);

      sendto_one (client_p, "SERVER %s 1 :%s",
		  my_name_for_link (me.name, aconf),
		  (me.info[0]) ? (me.info) : "IRCers United");
    }
  else
    {
      s = (char *) strchr (aconf->host, '@');
      *s = '\0';		/* should never be NULL -- wanna bet? -Dianora */

      Debug ((DEBUG_INFO, "Check Usernames [%s]vs[%s]", aconf->host,
	      client_p->username));
      if (match (aconf->host, client_p->username))
	{
	  *s = '@';
	  ircstp->is_ref++;
	  sendto_realops ("Username mismatch [%s]v[%s] : %s",
			  aconf->host, client_p->username,
			  get_client_name (client_p, HIDEME));
	  sendto_one (client_p, "ERROR :No Username Match");
	  return exit_client (client_p, client_p, client_p, "Bad User");
	}
      *s = '@';
    }

  sendto_one (client_p, "NETCTRL 0 0 0 0 0 0 0 0 0 0 0 0 0");

  /* send routing notice, this should never happen anymore */
  if (!DoesTS (client_p))
    {
      sendto_snomask (SNOMASK_NETINFO,"from %s: Warning: %s linked, non-TS server",
		      me.name, get_client_name (client_p, TRUE));
      sendto_serv_butone (client_p,
			  ":%s GNOTICE :Warning: %s linked, non-TS server",
			  me.name, get_client_name (client_p, TRUE));
    }

  sendto_one (client_p, "SVINFO %d %d 0 :%ld", TS_CURRENT, TS_MIN,
	      (ts_val) timeofday);

  /* sendto one(client_p, "CAPAB ...."); moved to after PASS but before SERVER
   * now in two places.. up above and in s_bsd.c. - lucas
   * This is to make sure we pass on our capabilities before we establish a server connection */

  det_confs_butmask (client_p,
		     CONF_LEAF | CONF_HUB | CONF_NOCONNECT_SERVER |
		     CONF_ULINE);
  /*
   * * *WARNING*
   *   In the following code in place of plain
   * server's name we send what is returned by
   * get_client_name which may add the "sockhost" after the name.
   * It's *very* *important* that there is a SPACE between
   * the name and sockhost (if present). The receiving server
   * will start the information field from this first blank and
   * thus puts the sockhost into info. ...a bit tricky, but
   * you have been warned, besides code is more neat this way...
   * --msa
   */

  /* doesnt duplicate client_p->serv if allocated this struct already */

  (void) make_server (client_p);
  client_p->serv->up = me.name;
  client_p->serv->nline = aconf;
#ifdef THROTTLE_ENABLE
  throttle_remove (inet_ntop (AFINET, (char *) &client_p->ip,
			      mydummy, sizeof (mydummy)));
#endif
  /* now fill out the servers info so nobody knows dink about it. */
  memset ((char *) &client_p->ip, '\0', sizeof (struct IN_ADDR));
  strcpy (client_p->hostip, "127.0.0.1");
  strcpy (client_p->sockhost, "localhost");

#ifdef HAVE_ENCRYPTION_ON
  if (!CanDoDKEY (client_p) || !WantDKEY (client_p))
    return do_server_estab (client_p);
  else
    {
      SetNegoServer (client_p);	/* VERY IMPORTANT THAT THIS IS HERE */
      sendto_one (client_p, "DKEY START");
    }
#else
  return do_server_estab (client_p);
#endif

  return 0;
}

/* 
 * m_burst
 *      parv[0] = sender prefix
 *      parv[1] = SendQ if an EOB
*/
int
m_burst (aClient * client_p, aClient * source_p, int parc, char *parv[])
{

  if (!IsServer (source_p) || source_p != client_p || parc > 2
      || !IsBurst (source_p))
    return 0;
  if (parc == 2)
    {				/* This is an EOB */
      source_p->flags &= ~(FLAGS_EOBRECV);
      if (source_p->flags & (FLAGS_SOBSENT | FLAGS_BURST))
	return 0;

      /* we've already sent our EOB.. we synched first
       * no need to check IsBurst because we shouldn't receive a BURST if they're not BURST capab */

#ifdef HTM_LOCK_ON_NETBURST
      HTMLOCK = NO;
#endif
      sendto_snomask (SNOMASK_NETINFO, "from %s: synch to %s in %d %s at %s sendq", me.name,
		      *parv, (timeofday - source_p->firsttime),
		      (timeofday - source_p->firsttime) == 1 ? "sec" : "secs",
		      parv[1]);
      sendto_serv_butone (NULL,
			  ":%s GNOTICE :synch to %s in %d %s at %s sendq",
			  me.name, source_p->name,
			  (timeofday - source_p->firsttime),
			  (timeofday - source_p->firsttime) ==
			  1 ? "sec" : "secs", parv[1]);

    }
  else
    {				/* SOB.. lock HTM if defined by admin */
#ifdef HTM_LOCK_ON_NETBURST
      HTMLOCK = YES;
#endif
      source_p->flags |= FLAGS_EOBRECV;
    }
  return 0;
}

/*
 * * m_links
 *      parv[0] = sender prefix
 *      parv[1] = servername mask
 * or
 *  	parv[0] = sender prefix
 * 	parv[1] = server to query
 *      parv[2] = servername mask
 */
int
m_links (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  char *mask;
  aClient *target_p;
  char clean_mask[(2 * HOSTLEN) + 1];
  char *s;
  char *d;
  int n;

  if (parc > 2)
    {
      if (hunt_server (client_p, source_p, ":%s LINKS %s :%s", 1, parc, parv)
	  != HUNTED_ISME)
	return 0;
      mask = parv[2];

    }
  else
    mask = parc < 2 ? NULL : parv[1];

  /* reject non-local requests */
  if (!IsAnOper (source_p) && !MyConnect (source_p))
    return 0;

  /*
   * * sigh* Before the kiddies find this new and exciting way of
   * * annoying opers, lets clean up what is sent to all opers
   * * -Dianora
   */

  if (mask)
    {				/* only necessary if there is a mask */
      s = mask;
      d = clean_mask;
      n = (2 * HOSTLEN) - 2;
      while (*s && n > 0)
	{
	  if ((unsigned char) *s < (unsigned char) ' ')	/* Is it a control character? */
	    {
	      *d++ = '^';
	      *d++ = (char) ((unsigned char) *s + 0x40);	/* turn it into a printable */
	      s++;
	      n -= 2;
	    }
	  else if ((unsigned char) *s > (unsigned char) '~')
	    {
	      *d++ = '.';
	      s++;
	      n--;
	    }
	  else
	    {
	      *d++ = *s++;
	      n--;
	    }
	}
      *d = '\0';
    }

  if (MyConnect (source_p))
    sendto_snomask (SNOMASK_LINKS,
			"LINKS %s requested by %s (%s@%s) [%s]",
			mask ? clean_mask : "all",
			source_p->name, source_p->user->username,
			source_p->user->host, source_p->user->server);

  for (target_p = client, (void) collapse (mask); target_p;
       target_p = target_p->next)
    {
      if (!IsServer (target_p) && !IsMe (target_p))
	continue;
      if (!BadPtr (mask) && match (mask, target_p->name))
	continue;

      if (HIDEULINEDSERVS == 1)
	{
	  if (!IsAnOper (source_p) && IsULine (target_p))
	    continue;
	}
      sendto_one (source_p, rpl_str (RPL_LINKS),
		  me.name, parv[0], target_p->name, target_p->serv->up,
		  target_p->hopcount, (target_p->info[0] ? target_p->info :
				       "(Unknown Location)"));
    }

  sendto_one (source_p, rpl_str (RPL_ENDOFLINKS), me.name, parv[0],
	      BadPtr (mask) ? "*" : clean_mask);
  return 0;
}

#ifdef LITTLE_I_LINES
static int report_array[14][3] = {
#else
static int report_array[13][3] = {
#endif
  {CONF_CONNECT_SERVER, RPL_STATSCLINE, 'C'},
  {CONF_NOCONNECT_SERVER, RPL_STATSNLINE, 'N'},
  {CONF_GCOS, RPL_STATSGLINE, 'G'},
  {CONF_CLIENT, RPL_STATSILINE, 'I'},
#ifdef LITTLE_I_LINES
  {CONF_CLIENT, RPL_STATSILINE, 'i'},
#endif
  {CONF_KILL, RPL_STATSKLINE, 'K'},
  {CONF_LEAF, RPL_STATSLLINE, 'L'},
  {CONF_OPERATOR, RPL_STATSOLINE, 'O'},
  {CONF_HUB, RPL_STATSHLINE, 'H'},
  {CONF_LOCOP, RPL_STATSOLINE, 'o'},
  {CONF_QUARANTINE, RPL_STATSQLINE, 'Q'},
  {CONF_ULINE, RPL_STATSULINE, 'U'},
  {CONF_VHOST, RPL_STATSVLINE, 'V'},
  {0, 0}
};

static void
report_configured_links (aClient * source_p, int mask)
{
  static char null[] = "<NULL>";
  aConfItem *tmp;
  int *p, port;
  char c, *host, *pass, *name;

  for (tmp = conf; tmp; tmp = tmp->next)
    {
      if (tmp->status & mask)
	{
	  for (p = &report_array[0][0]; *p; p += 3)
	    {
	      if (*p & tmp->status)
		{
		  break;
		}
	    }
	  if (!*p)
	    {
	      continue;
	    }
#ifdef LITTLE_I_LINES
	  if (tmp->flags & CONF_FLAGS_LITTLE_I_LINE)
	    {
	      p += 3;
	      if (!*p)
		{
		  continue;
		}
	    }
#endif
	  c = (char) *(p + 2);
	  host = BadPtr (tmp->host) ? null : tmp->host;
	  pass = BadPtr (tmp->passwd) ? null : tmp->passwd;
	  name = BadPtr (tmp->name) ? null : tmp->name;
	  port = (int) tmp->port;

	  /* block out IP addresses of U: lined servers in 
	   * a C/N request - lucas/xpsycho
	   * modified on request by taz, hide all
	   * ips to other than routing staff or admins
	   * -epi
	   * yet again, make this just hide _all_ ips. how sad.
	   * - lucas
	   * If we dont even trust our own server admins, which
	   * have access to the IP's in any case... - ShadowMaster
	   */

	  if (tmp->status & CONF_SERVER_MASK && !(source_p->oflag & OFLAG_AUSPEX))
	    {
	      host = "*";
	    }
	  if (tmp->status & CONF_QUARANTINE)
	    {
	      sendto_one (source_p, rpl_str (p[1]), me.name,
			  source_p->name, c, pass, name, port,
			  get_conf_class (tmp));
	    }
	  else if (tmp->status & CONF_GCOS)
	    {
	      sendto_one (source_p, rpl_str (p[1]), me.name,
			  source_p->name, c, pass, name, port,
			  get_conf_class (tmp));
	    }
	  else if (mask & CONF_OPS)
	    {
	      sendto_one (source_p, rpl_str (p[1]), me.name,
			  source_p->name, c, host, name, oflagstr (port),
			  get_conf_class (tmp));
	    }
	  else
	    {
	      if (!IsAnOper (source_p))
		sendto_one (source_p, rpl_str (p[1]), me.name,
			    source_p->name, c, "*", name, port,
			    get_conf_class (tmp));
	      else
		sendto_one (source_p, rpl_str (p[1]), me.name,
			    source_p->name, c, host, name, port,
			    get_conf_class (tmp));
	    }
	}
    }
  return;
}

/*
 *
 * m_stats 
 *      parv[0] = sender prefix 
 *      parv[1] = statistics selector (defaults to Message frequency) 
 *      parv[2] = server name (current server defaulted, if omitted) 
 */
/*
 * 
 * m_stats/stats_conf 
 *    Report N/C-configuration lines from this server. This could 
 *    report other configuration lines too, but converting the
 *    status back to "char" is a bit akward--not worth the code 
 *    it needs...
 * 
 *    Note:   The info is reported in the order the server uses 
 *    it--not reversed as in ircd.conf!
 */

int
m_stats (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  static char Lformat[] = ":%s %d %s %s %u %u %u %u %u :%u %u %s";
  static char Sformat[] =
    ":%s %d %s Name SendQ SendM SendBytes RcveM RcveBytes :Open_since Idle TS";

  struct Message *mptr;
  aClient *target_p;
  char stat = parc > 1 ? parv[1][0] : '\0';
  int i;
  int doall = 0, wilds = 0;
  char *name;
  time_t sincetime;

  static time_t last_used = 0L;

  if (hunt_server (client_p, source_p, ":%s STATS %s :%s", 2, parc, parv) !=
      HUNTED_ISME)
    return 0;

  if (!IsAnOper (source_p) && !IsULine (source_p))
    {
      /* allow remote stats p l ? u */
      if (!
	  ((stat == 'p') || (stat == 'P') || (stat == '?') || (stat == 'u')
	   || (stat == 'l') || (stat == 'L')) && !MyConnect (source_p))
	return 0;

      if ((last_used + MOTD_WAIT) > NOW)
	return 0;
      else
	last_used = NOW;
    }

  if (parc > 2)
    {
      name = parv[2];
      if (irccmp (name, me.name) == 0)
	doall = 2;
      else if (match (name, me.name) == 0)
	doall = 1;
      if (strchr (name, '*') || strchr (name, '?'))
	wilds = 1;
    }
  else
    name = me.name;

  if (stat != (char) 0 && !IsULine (source_p))
    sendto_snomask (SNOMASK_SPY, "STATS %c requested by %s (%s@%s) [%s]",
			stat, source_p->name, source_p->user->username,
			source_p->user->host, source_p->user->server);
  switch (stat)
    {
    case 'g':
    case 'G':
       list_glines(source_p);
       break;
    case 'L':
    case 'l':
      /* changed behavior totally.  This is what we do now:
       * #1: if the user is not opered, never return ips for anything
       * #2: we DON'T deny /stats l for opers.  Ever heard of /sping?
       *     it's easy to see if you're lagging a server, why restrict
       *     something used 99% of the time for good when you're not
       *     doing any harm?
       * #3: NEVER return all users on a server, UGH, just like
       *     /trace, this was fiercely obnoxious.  If you don't
       *     add an argument, you get all SERVER links.
       */
      sendto_one (source_p, Sformat, me.name, RPL_STATSLINKINFO, parv[0]);
      if ((parc > 2) && !(doall || wilds))
	{			/* Single client lookup */
	  if (!(target_p = find_person (name, NULL)))
	    break;
	  /*
	   * sincetime might be greater than timeofday,
	   * store a new value here to avoid sending
	   * negative since-times. -Rak
	   */
	  sincetime = (target_p->since > timeofday) ? 0 :
	    timeofday - target_p->since;
	  sendto_one (source_p, Lformat, me.name,
		      RPL_STATSLINKINFO, parv[0],
		      IsAnOper (source_p) ? get_client_name (target_p,
							     TRUE) :
		      get_client_name (target_p, HIDEME),
		      (int) DBufLength (&target_p->sendQ),
		      (int) target_p->sendM, (int) target_p->sendK,
		      (int) target_p->receiveM, (int) target_p->receiveK,
		      timeofday - target_p->firsttime, sincetime,
		      IsServer (target_p) ? (DoesTS (target_p) ? "TS" :
					     "NoTS") : "-");
	}
      else
	{
	  for (i = 0; i <= highest_fd; i++)
	    {
	      if (!(target_p = local[i]))
		continue;
	      if (!IsServer (target_p))
		continue;	/* nothing but servers */

	      if (HIDEULINEDSERVS == 1)
		{
		  if (IsULine (target_p) && !IsAnOper (source_p))
		    continue;
		}

	      sincetime = (target_p->since > timeofday) ? 0 :
		timeofday - target_p->since;
	      sendto_one (source_p, Lformat, me.name,
			  RPL_STATSLINKINFO, parv[0],
			  get_client_name (target_p, HIDEME),
			  (int) DBufLength (&target_p->sendQ),
			  (int) target_p->sendM, (int) target_p->sendK,
			  (int) target_p->receiveM, (int) target_p->receiveK,
			  timeofday - target_p->firsttime,
			  sincetime,
			  IsServer (target_p) ? (DoesTS (target_p) ?
						 "TS" : "NoTS") : "-");
	    }
	}
      break;
    case 'C':
    case 'c':
      if (!IsAnOper (source_p))
	sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);

      else
	report_configured_links (source_p, CONF_CONNECT_SERVER |
				 CONF_NOCONNECT_SERVER);
      break;
    case 'E':
    case 'e':
#ifdef E_LINES_OPER_ONLY
      if (!IsAnOper (source_p))
	{
	  sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
	  break;
	}
#endif
      report_conf_links (source_p, &EList1, RPL_STATSELINE, 'E');
      report_conf_links (source_p, &EList2, RPL_STATSELINE, 'E');
      report_conf_links (source_p, &EList3, RPL_STATSELINE, 'E');
      break;

    case 'F':
    case 'f':
#ifdef F_LINES_OPER_ONLY
      if (!IsAnOper (source_p))
	{
	  sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
	  break;
	}
#endif
      report_conf_links (source_p, &FList1, RPL_STATSFLINE, 'F');
      report_conf_links (source_p, &FList2, RPL_STATSFLINE, 'F');
      report_conf_links (source_p, &FList3, RPL_STATSFLINE, 'F');
      break;

    case 'X':
    case 'x':
#ifdef G_LINES_OPER_ONLY
      if (!IsAnOper (source_p))
	{
	  sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
	  break;
	}
#endif
      report_configured_links (source_p, CONF_GCOS);
      break;

    case 'H':
    case 'h':
      if ((HIDEULINEDSERVS == 1) && !IsOper (source_p))
	sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      else
	report_configured_links (source_p, CONF_HUB | CONF_LEAF);
      break;

    case 'I':
    case 'i':
      report_configured_links (source_p, CONF_CLIENT);
      break;

    case 'k':
#ifdef K_LINES_OPER_ONLY
      if (!IsAnOper (source_p))
	{
	  sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
	  break;
	}
#endif
      report_userbans_match_flags (source_p, UBAN_TEMPORARY | UBAN_LOCAL, 0);
      break;
    case 'K':
#ifdef K_LINES_OPER_ONLY
      if (!IsAnOper (source_p))
	{
	  sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
	  break;
	}
#endif
      report_userbans_match_flags (source_p, UBAN_LOCAL, UBAN_TEMPORARY);
      break;
    case 'A':
    case 'a':
#ifdef K_LINES_OPER_ONLY
      if (!IsAnOper (source_p))
	{
	  sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
	  break;
	}
#endif
      report_userbans_match_flags (source_p, UBAN_NETWORK, 0);
      break;
    case 'M':
    case 'm':
      /*
       * original behaviour was not to report the command, if
       * the command hadn't been used. I'm going to always
       * report the command instead -Dianora
       */
      for (mptr = msgtab; mptr->cmd; mptr++)
	/*
	 * if (mptr->count) 
	 */
	sendto_one (source_p, rpl_str (RPL_STATSCOMMANDS),
		    me.name, parv[0], mptr->cmd, mptr->count, mptr->bytes);
      break;

    case 'N':
    case 'n':
      sendto_one (source_p, rpl_str (RPL_STATSCOUNT), me.name, parv[0],
		  "User Connects Today: ", Count.today);
      sendto_one (source_p, rpl_str (RPL_STATSCOUNT), me.name, parv[0],
		  "User Connects past week: ", Count.weekly);
      sendto_one (source_p, rpl_str (RPL_STATSCOUNT), me.name, parv[0],
		  "User Connects past month: ", Count.monthly);
      sendto_one (source_p, rpl_str (RPL_STATSCOUNT), me.name, parv[0],
		  "User Connects past year: ", Count.yearly);
      break;
    case 'o':
    case 'O':
      /*
       ** We never ever want to reveal our O Lines to nonopers - ShadowMaster
       */
      if (!IsAnOper (source_p))
	sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      else
	report_configured_links (source_p, CONF_OPS);
      break;

    case 'p':
    case 'P':
      show_opers (source_p, parv[0]);
      break;

    case 'Q':
    case 'q':
#ifdef Q_LINES_OPER_ONLY
      if (!IsAnOper (source_p))
	{
	  sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
	  break;
	}
#endif
      report_configured_links (source_p, CONF_QUARANTINE);
      break;

    case 'R':
    case 'r':
#ifdef DEBUGMODE
      send_usage (source_p, parv[0]);
#endif
      break;

    case 'S':
    case 's':
      if (IsAnOper (source_p))
	list_scache (client_p, source_p, parc, parv);
      else
	sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      break;

    case 'T':
      if (!IsAnOper (source_p))
	{
	  sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
	  break;
	}
#ifdef THROTTLE_ENABLE
      throttle_stats (source_p, parv[0]);
#endif
      break;

    case 't':
      if (!IsAnOper (source_p))
	{
	  sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
	  break;
	}
      tstats (source_p, parv[0]);
      break;

    case 'U':
      if ((HIDEULINEDSERVS == 1) && !IsOper (source_p))
	sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      else
	report_configured_links (source_p, CONF_ULINE);
      break;

    case 'u':
      {
	time_t now;

	now = timeofday - me.since;
	sendto_one (source_p, rpl_str (RPL_STATSUPTIME), me.name, parv[0],
		    now / 86400, (now / 3600) % 24, (now / 60) % 60,
		    now % 60);
#ifdef HIGHEST_CONNECTION
	sendto_one (source_p, rpl_str (RPL_STATSCONN), me.name, parv[0],
		    max_connection_count, max_client_count);
#endif
	break;
      }

    case 'v':
      show_servers (source_p, parv[0]);
      break;
    case 'V':
#ifdef V_LINES_OPER_ONLY
      if (!IsAnOper (source_p))
	{
	  sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
	  break;
	}
#endif
      report_configured_links (source_p, CONF_VHOST);
      break;

    case 'Y':
    case 'y':
      report_classes (source_p);
      break;

    case 'Z':
    case 'z':
      if (IsAnOper (source_p))
	count_memory (source_p, parv[0]);
      else
	sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      break;

    case '?':
      serv_info (source_p, parv[0]);

    default:
      stat = '*';
      break;
    }
  sendto_one (source_p, rpl_str (RPL_ENDOFSTATS), me.name, parv[0], stat);
  return 0;
}

/*
 * * m_users 
 *        parv[0] = sender prefix 
 *        parv[1] = servername
 */
int
m_users (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  if (hunt_server (client_p, source_p, ":%s USERS :%s", 1, parc, parv) ==
      HUNTED_ISME)
    {
      /* No one uses this any more... so lets remap it..   -Taner */
      sendto_one (source_p, rpl_str (RPL_LOCALUSERS), me.name, parv[0],
		  Count.local, Count.max_loc);
      sendto_one (source_p, rpl_str (RPL_GLOBALUSERS), me.name, parv[0],
		  Count.total, Count.max_tot);
    }
  return 0;
}

/*
 * * Note: At least at protocol level ERROR has only one parameter, 
 * although this is called internally from other functions  --msa 
 *
 *      parv[0] = sender prefix
 *      parv[*] = parameters
 */
int
m_error (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  char *para;

  para = (parc > 1 && *parv[1] != '\0') ? parv[1] : "<>";

  Debug ((DEBUG_ERROR, "Received ERROR message from %s: %s",
	  source_p->name, para));
  /*
   * * Ignore error messages generated by normal user clients 
   * (because ill-behaving user clients would flood opers screen
   * otherwise). Pass ERROR's from other sources to the local
   * operator...
   */
  if (IsPerson (client_p) || IsUnknown (client_p))
    return 0;
  if (client_p == source_p)
    sendto_realops ("ERROR :from %s -- %s",
		    get_client_name (client_p, HIDEME), para);
  else
    sendto_realops ("ERROR :from %s via %s -- %s", source_p->name,
		    get_client_name (client_p, HIDEME), para);
  return 0;
}

/*
 * * m_help 
 * Moved to s_help.c --Acidic32
 */

/*
 * parv[0] = sender parv[1] = host/server mask. 
 * parv[2] = server to query
 * 
 * 199970918 JRL hacked to ignore parv[1] completely and require parc > 3
 * to cause a force
 */
int
m_lusers (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  int send_lusers (aClient *, aClient *, int, char **);

  if (parc > 2)
    {
      if (hunt_server (client_p, source_p, ":%s LUSERS %s :%s", 2, parc, parv)
	  != HUNTED_ISME)
	return 0;
    }
  return send_lusers (client_p, source_p, parc, parv);
}

/*
 * send_lusers
 *     parv[0] = sender
 *     parv[1] = host/server mask.
 *     parv[2] = server to query
 */
int
send_lusers (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
#define LUSERS_CACHE_TIME 180
  static long last_time = 0;
  static int s_count = 0, c_count = 0, u_count = 0, i_count = 0;
  static int o_count = 0, m_client = 0, m_server = 0, m_ulined = 0;
  int forced;
  aClient *target_p;

  forced = (IsAnOper (source_p) && (parc > 3));

  Count.unknown = 0;
  m_server = Count.myserver;
  m_ulined = Count.myulined;
  m_client = Count.local;
  i_count = Count.invisi;
  u_count = Count.unknown;
  c_count = Count.total - Count.invisi;
  s_count = Count.server;
  o_count = Count.oper;
  if (forced || (timeofday > last_time + LUSERS_CACHE_TIME))
    {
      last_time = timeofday;
      /* only recount if more than a second has passed since last request
       * use LUSERS_CACHE_TIME instead... 
       */
      s_count = 0;
      c_count = 0;
      u_count = 0;
      i_count = 0;
      o_count = 0;
      m_client = 0;
      m_server = 0;
      m_ulined = 0;

      for (target_p = client; target_p; target_p = target_p->next)
	{
	  switch (target_p->status)
	    {
	    case STAT_SERVER:
	      if (MyConnect (target_p))
		{
		  m_server++;
		  if (IsULine (target_p))
		    m_ulined++;
		}
	    case STAT_ME:
	      s_count++;
	      break;
	    case STAT_CLIENT:
	      if (IsAnOper (target_p))
		o_count++;
#ifdef	SHOW_INVISIBLE_LUSERS
	      if (MyConnect (target_p))
		m_client++;
	      if (!IsInvisible (target_p))
		c_count++;
	      else
		i_count++;
#else
	      if (MyConnect (target_p))
		{
		  if (IsInvisible (target_p))
		    {
		      if (IsAnOper (source_p))
			m_client++;
		    }
		  else
		    m_client++;
		}
	      if (!IsInvisible (target_p))
		c_count++;
	      else
		i_count++;
#endif
	      break;
	    default:
	      u_count++;
	      break;
	    }
	}
      /*
       * We only want to reassign the global counts if the recount time
       * has expired, and NOT when it was forced, since someone may
       * supply a mask which will only count part of the userbase
       * -Taner
       */
      if (!forced)
	{
	  if (m_server != Count.myserver)
	    {
	      sendto_snomask (SNOMASK_DEBUG,
				  "Local server count off by %d",
				  Count.myserver - m_server);
	      Count.myserver = m_server;
	    }
	  if (m_ulined != Count.myulined)
	    {
	      sendto_snomask (SNOMASK_DEBUG,
				  "Local ulinedserver count off by %d",
				  Count.myulined - m_ulined);
	      Count.myulined = m_ulined;
	    }
	  if (s_count != Count.server)
	    {
	      sendto_snomask (SNOMASK_DEBUG,
				  "Server count off by %d",
				  Count.server - s_count);
	      Count.server = s_count;
	    }
	  if (i_count != Count.invisi)
	    {
	      sendto_snomask (SNOMASK_DEBUG,
				  "Invisible client count off by %d",
				  Count.invisi - i_count);
	      Count.invisi = i_count;
	    }
	  if ((c_count + i_count) != Count.total)
	    {
	      sendto_snomask (SNOMASK_DEBUG, "Total client count off by %d",
				  Count.total - (c_count + i_count));
	      Count.total = c_count + i_count;
	    }
	  if (m_client != Count.local)
	    {
	      sendto_snomask (SNOMASK_DEBUG,
				  "Local client count off by %d",
				  Count.local - m_client);
	      Count.local = m_client;
	    }
	  if (o_count != Count.oper)
	    {
	      sendto_snomask (SNOMASK_DEBUG,
				  "Oper count off by %d",
				  Count.oper - o_count);
	      Count.oper = o_count;
	    }
	  Count.unknown = u_count;
	}			/* Complain & reset loop */
    }				/* Recount loop */

#ifdef SAVE_MAXCLIENT_STATS
  if ((timeofday - last_stat_save) > SAVE_MAXCLIENT_STATS_TIME)
    {				/* save stats */
      FILE *fp;

      last_stat_save = timeofday;

      fp = fopen (DPATH "/.maxclients", "w");
      if (fp != NULL)
	{
	  fprintf (fp, "%d %d %li %li %li %ld %ld %ld %ld", Count.max_loc,
		   Count.max_tot, Count.weekly, Count.monthly,
		   Count.yearly, Count.start, Count.week, Count.month,
		   Count.year);
	  fclose (fp);
	  sendto_snomask (SNOMASK_DEBUG, "Saved maxclient statistics");
	}
    }
#endif


#ifndef	SHOW_INVISIBLE_LUSERS
  if (IsAnOper (source_p) && i_count)
#endif
    sendto_one (source_p, rpl_str (RPL_LUSERCLIENT), me.name, parv[0],
		c_count, i_count, s_count);
#ifndef	SHOW_INVISIBLE_LUSERS
  else
    sendto_one (source_p,
		":%s %d %s :There are %d users on %d servers", me.name,
		RPL_LUSERCLIENT, parv[0], c_count, s_count);
#endif
  if (o_count)
    sendto_one (source_p, rpl_str (RPL_LUSEROP), me.name, parv[0], o_count);
  if (u_count > 0)
    sendto_one (source_p, rpl_str (RPL_LUSERUNKNOWN), me.name, parv[0],
		u_count);

  /* This should be ok */
  if (Count.chan > 0)
    sendto_one (source_p, rpl_str (RPL_LUSERCHANNELS),
		me.name, parv[0], Count.chan);
  sendto_one (source_p, rpl_str (RPL_LUSERME),
#ifdef HIDEULINEDSERVS
	      me.name, parv[0], m_client,
	      IsOper (source_p) ? m_server : m_server - m_ulined);
#else
	      me.name, parv[0], m_client, m_server);
#endif

#ifdef CLIENT_COUNT
  sendto_one (source_p, rpl_str (RPL_LOCALUSERS), me.name, parv[0],
	      Count.local, Count.max_loc);
  sendto_one (source_p, rpl_str (RPL_GLOBALUSERS), me.name, parv[0],
	      Count.total, Count.max_tot);
#else
# ifdef HIGHEST_CONNECTION
  sendto_one (source_p, rpl_str (RPL_STATSCONN), me.name, parv[0],
	      max_connection_count, max_client_count);
  if (m_client > max_client_count)
    max_client_count = m_client;
  if ((m_client + m_server) > max_connection_count)
    {
      max_connection_count = m_client + m_server;
      if (max_connection_count % 10 == 0)
	sendto_ops ("New highest connections: %d (%d clients)",
		    max_connection_count, max_client_count);
    }
# endif	/* HIGHEST_CONNECTION */
#endif /* CLIENT_COUNT */
  return 0;
}

/***********************************************************************
 * m_connect() - Added by Jto 11 Feb 1989
 ***********************************************************************/
/*
 * * m_connect 
 *      parv[0] = sender prefix 
 *      parv[1] = servername
 * 	parv[2] = port number 
 * 	parv[3] = remote server
 */
int
m_connect (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  int port, tmpport, retval;
  aConfItem *aconf;
  aClient *target_p;

  if (!IsPrivileged (source_p))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return -1;
    }

  if ((MyClient (source_p) && !(source_p->oflag & OFLAG_GROUTE) && parc > 3) ||
      (MyClient (source_p) && !(source_p->oflag & OFLAG_LROUTE) && parc <= 3))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }

  if (hunt_server (client_p, source_p, ":%s CONNECT %s %s :%s",
		   3, parc, parv) != HUNTED_ISME)
    return 0;

  if (parc < 2 || *parv[1] == '\0')
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
		  me.name, parv[0], "CONNECT");
      return -1;
    }

  if ((target_p = find_server (parv[1], NULL)))
    {
      sendto_one (source_p, ":%s NOTICE %s :Connect: Server %s %s %s.",
		  me.name, parv[0], parv[1], "already exists from",
		  target_p->from->name);
      return 0;
    }

  for (aconf = conf; aconf; aconf = aconf->next)
    if (aconf->status == CONF_CONNECT_SERVER
	&& match (parv[1], aconf->name) == 0)
      break;

  /* Checked first servernames, then try hostnames. */
  if (!aconf)
    for (aconf = conf; aconf; aconf = aconf->next)
      if (aconf->status == CONF_CONNECT_SERVER &&
	  (match (parv[1], aconf->host) == 0 ||
	   match (parv[1], strchr (aconf->host, '@') + 1) == 0))
	break;

  if (!aconf)
    {
      sendto_one (source_p,
		  "NOTICE %s :Connect: No C line found for %s.",
		  parv[0], parv[1]);
      return 0;
    }
  /*
   * * Get port number from user, if given. If not specified, use
   * the default form configuration structure. If missing from
   * there, then use the precompiled default.
   */
  tmpport = port = aconf->port;
  if (parc > 2 && !BadPtr (parv[2]))
    {
      if ((port = atoi (parv[2])) <= 0)
	{
	  sendto_one (source_p,
		      "NOTICE %s :Connect: Illegal port number", parv[0]);
	  return 0;
	}
    }
  else if (port <= 0 && (port = portnum) <= 0)
    {
      sendto_one (source_p, ":%s NOTICE %s :Connect: missing port number",
		  me.name, parv[0]);
      return 0;
    }
  /*
   * * Notify all operators about remote connect requests
   * Let's notify about local connects, too. - lucas
   * sendto_ops_butone -> sendto_serv_butone(), like in df. -mjs
   */
  sendto_snomask (SNOMASK_NETINFO, "from %s: %s CONNECT %s %s from %s",
		  me.name, IsAnOper (client_p) ? "Local" : "Remote",
		  parv[1], parv[2] ? parv[2] : "",
		  get_client_name (source_p, HIDEME));
  sendto_serv_butone (NULL, ":%s GNOTICE :%s CONNECT %s %s from %s",
		      me.name, IsAnOper (client_p) ? "Local" : "Remote",
		      parv[1], parv[2] ? parv[2] : "",
		      get_client_name (source_p, HIDEME));

#if defined(USE_SYSLOG) && defined(SYSLOG_CONNECT)
  syslog (LOG_DEBUG, "CONNECT From %s : %s %s", parv[0], parv[1],
	  parv[2] ? parv[2] : "");
#endif

  aconf->port = port;
  switch (retval = connect_server (aconf, source_p, NULL))
    {
    case 0:
      sendto_one (source_p,
		  ":%s NOTICE %s :*** Connecting to %s.",
		  me.name, parv[0], aconf->name);
      break;
    case -1:
      sendto_one (source_p, ":%s NOTICE %s :*** Couldn't connect to %s.",
		  me.name, parv[0], aconf->name);
      break;
    case -2:
      sendto_one (source_p, ":%s NOTICE %s :*** Host %s is unknown.",
		  me.name, parv[0], aconf->name);
      break;
    default:
      sendto_one (source_p,
		  ":%s NOTICE %s :*** Connection to %s failed: %s",
		  me.name, parv[0], aconf->name, strerror (retval));
    }
  aconf->port = tmpport;
  return 0;
}

/*
 * * m_wallops (write to *all* opers currently online)
 *      parv[0] = sender prefix 
 *      parv[1] = message text
 */
int
m_wallops (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  char *message = parc > 1 ? parv[1] : NULL;

  if (BadPtr (message))
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
		  me.name, parv[0], "WALLOPS");
      return 0;
    }

  if (!IsServer (source_p) && MyConnect (source_p)
      && !(source_p->oflag & OFLAG_WALLOPS))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return (0);
    }

  sendto_wallops_butone (IsServer (client_p) ? client_p : NULL, source_p,
			 ":%s WALLOPS :%s", parv[0], message);
  return 0;
}

/*
 * m_goper  (Russell) sort of like wallop, but only to ALL +o clients
 * on every server. 
 *      parv[0] = sender prefix 
 *      parv[1] = message text
 * Taken from df465, ported to hybrid. -mjs
 */
int
m_goper (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  char *message = parc > 1 ? parv[1] : NULL;

  if (check_registered (source_p))
    return 0;

  if (BadPtr (message))
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
		  me.name, parv[0], "GOPER");
      return 0;
    }
  if (!IsServer (source_p) || !IsULine (source_p))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }

  sendto_serv_butone (IsServer (client_p) ? client_p : NULL, ":%s GOPER :%s",
		      parv[0], message);
  sendto_realops ("from %s: %s", parv[0], message);
  return 0;
}

/*
 * m_gnotice  (Russell) sort of like wallop, but only to +g clients on *
 * this server. 
 *      parv[0] = sender prefix 
 *      parv[1] = message text 
 * ported from df465 to hybrid -mjs
 *
 * This function itself doesnt need any changes for the move to +n routing
 * notices, to sendto takes care of it.  Now only sends to +n clients -epi
 */
int
m_gnotice (aClient * client_p, aClient * source_p, int parc, char *parv[])
{

  char *message = parc > 1 ? parv[1] : NULL;

  if (check_registered (source_p))
    return 0;

  if (BadPtr (message))
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
		  me.name, parv[0], "GNOTICE");
      return 0;
    }
  if (!IsServer (source_p) && MyConnect (source_p))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }

  sendto_serv_butone (IsServer (client_p) ? client_p : NULL,
		      ":%s GNOTICE :%s", parv[0], message);
  sendto_snomask (SNOMASK_NETINFO, "%s", parv[0], message);
  return 0;
}

/*
** m_gconnect based on original Bahamut/DreamForge m_chatops
*/
int
m_gconnect (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  char *message = parc > 1 ? parv[1] : NULL;

  if (check_registered (source_p))
    return 0;
  if (BadPtr (message))
    {
      if (MyClient (source_p))
	sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
		    me.name, parv[0], "GCONNECT");
      return 0;
    }

  if (MyClient (source_p) && !IsServer (source_p) && !IsULine (source_p))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }

  if (strlen (message) > TOPICLEN)
    message[TOPICLEN] = '\0';
  sendto_serv_butone (IsServer (client_p) ? client_p : NULL,
		      ":%s GCONNECT :%s", parv[0], message);
  sendto_snomask (SNOMASK_CONNECTS, "%s",  message);
  return 0;
}

/*
 * * m_time 
 * 	 parv[0] = sender prefix
 *       parv[1] = servername
 */
int
m_time (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  if (hunt_server (client_p, source_p, ":%s TIME :%s", 1, parc, parv) ==
      HUNTED_ISME)
    sendto_one (source_p, rpl_str (RPL_TIME), me.name, parv[0], me.name,
		date ((long) 0));
  return 0;
}

/*
 * * m_admin 
 *        parv[0] = sender prefix
 *        parv[1] = servername
 */
int
m_admin (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aConfItem *aconf;

  if (hunt_server (client_p, source_p, ":%s ADMIN :%s", 1, parc, parv) !=
      HUNTED_ISME)
    return 0;

  if (IsPerson (source_p))
    sendto_snomask (SNOMASK_SPY, "ADMIN requested by %s (%s@%s) [%s]",
			source_p->name, source_p->user->username,
			source_p->user->host, source_p->user->server);

  if ((aconf = find_admin ()))
    {
      sendto_one (source_p, rpl_str (RPL_ADMINME), me.name, parv[0], me.name);
      sendto_one (source_p, rpl_str (RPL_ADMINLOC1),
		  me.name, parv[0], aconf->host ? aconf->host : "");
      sendto_one (source_p, rpl_str (RPL_ADMINLOC2),
		  me.name, parv[0], aconf->passwd ? aconf->passwd : "");
      sendto_one (source_p, rpl_str (RPL_ADMINEMAIL),
		  me.name, parv[0], aconf->name ? aconf->name : "");
    }
  else
    sendto_one (source_p, err_str (ERR_NOADMININFO), me.name, parv[0],
		me.name);
  return 0;
}

/* Shadowfax's server side, anti flood code */
#ifdef FLUD
extern int flud_num;
extern int flud_time;
extern int flud_block;
#endif

#ifdef ANTI_SPAMBOT
extern int spam_num;
extern int spam_time;
#endif

#ifdef NO_CHANOPS_WHEN_SPLIT
extern int server_split_recovery_time;
#endif

/*
 * m_set - set options while running
 */
int
m_set (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  char *command;

  if (!MyClient (source_p) || !IsOper (source_p))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }

  if (parc > 1)
    {
      command = parv[1];
      if (ircncmp (command, "MAX", 3) == 0)
	{
	  if (parc > 2)
	    {
	      int new_value = atoi (parv[2]);

	      if (new_value > MASTER_MAX)
		{
		  sendto_one (source_p,
			      ":%s NOTICE %s :You cannot set MAXCLIENTS to > MASTER_MAX (%d)",
			      me.name, parv[0], MASTER_MAX);
		  return 0;
		}
	      if (new_value < 32)
		{
		  sendto_one (source_p,
			      ":%s NOTICE %s :You cannot set MAXCLIENTS to < 32 (%d:%d)",
			      me.name, parv[0], MAXCLIENTS, highest_fd);
		  return 0;
		}
	      MAXCLIENTS = new_value;
	      sendto_one (source_p,
			  ":%s NOTICE %s :NEW MAXCLIENTS = %d (Current = %d)",
			  me.name, parv[0], MAXCLIENTS, Count.local);
	      sendto_realops
		("%s!%s@%s set new MAXCLIENTS to %d (%d current)", parv[0],
		 source_p->user->username, source_p->sockhost, MAXCLIENTS,
		 Count.local);
	      return 0;
	    }
	  sendto_one (source_p, ":%s NOTICE %s :Current Maxclients = %d (%d)",
		      me.name, parv[0], MAXCLIENTS, Count.local);
	  return 0;
	}
#ifdef FLUD
      else if (ircncmp (command, "FLUDNUM", 7) == 0)
	{
	  if (parc > 2)
	    {
	      int newval = atoi (parv[2]);
	      if (newval <= 0)
		{
		  sendto_one (source_p, ":%s NOTICE %s :flud NUM must be > 0",
			      me.name, parv[0]);
		  return 0;
		}
	      flud_num = newval;
	      sendto_ops ("%s has changed flud NUM to %i", parv[0], flud_num);
	      sendto_one (source_p,
			  ":%s NOTICE %s :flud NUM is now set to %i", me.name,
			  parv[0], flud_num);
	      return 0;
	    }
	  else
	    {
	      sendto_one (source_p, ":%s NOTICE %s :flud NUM is currently %i",
			  me.name, parv[0], flud_num);
	      return 0;
	    }
	}
      else if (ircncmp (command, "FLUDTIME", 8) == 0)
	{
	  if (parc > 2)
	    {
	      int newval = atoi (parv[2]);
	      if (newval <= 0)
		{
		  sendto_one (source_p,
			      ":%s NOTICE %s :flud TIME must be > 0", me.name,
			      parv[0]);
		  return 0;
		}
	      flud_time = newval;
	      sendto_ops ("%s has changed flud TIME to %i", parv[0],
			  flud_time);
	      sendto_one (source_p,
			  ":%s NOTICE %s :flud TIME is now set to %i",
			  me.name, parv[0], flud_time);
	      return 0;
	    }
	  else
	    {
	      sendto_one (source_p,
			  ":%s NOTICE %s :flud TIME is currently %i", me.name,
			  parv[0], flud_time);
	      return 0;
	    }
	}
      else if (ircncmp (command, "FLUDBLOCK", 9) == 0)
	{
	  if (parc > 2)
	    {
	      int newval = atoi (parv[2]);
	      if (newval < 0)
		{
		  sendto_one (source_p,
			      ":%s NOTICE %s :flud BLOCK must be >= 0",
			      me.name, parv[0]);
		  return 0;
		}
	      flud_block = newval;
	      if (flud_block == 0)
		{
		  sendto_ops ("%s has disabled flud detection/protection",
			      parv[0]);
		  sendto_one (source_p,
			      ":%s NOTICE %s :flud detection disabled",
			      me.name, parv[0]);
		}
	      else
		{
		  sendto_ops ("%s has changed flud BLOCK to %i",
			      parv[0], flud_block);
		  sendto_one (source_p,
			      ":%s NOTICE %s :flud BLOCK is now set to %i",
			      me.name, parv[0], flud_block);
		}
	      return 0;
	    }
	  else
	    {
	      sendto_one (source_p,
			  ":%s NOTICE %s :flud BLOCK is currently %i",
			  me.name, parv[0], flud_block);
	      return 0;
	    }
	}
#endif
#ifdef NO_CHANOPS_WHEN_SPLIT
      else if (ircncmp (command, "SPLITDELAY", 10) == 0)
	{
	  if (parc > 2)
	    {
	      int newval = atoi (parv[2]);
	      if (newval < 0)
		{
		  sendto_one (source_p,
			      ":%s NOTICE %s :split delay must be >= 0",
			      me.name, parv[0]);
		  return 0;
		}
	      if (newval > MAX_SERVER_SPLIT_RECOVERY_TIME)
		newval = MAX_SERVER_SPLIT_RECOVERY_TIME;
	      sendto_ops
		("%s has changed spam SERVER SPLIT RECOVERY TIME  to %i",
		 parv[0], newval);
	      sendto_one (source_p,
			  ":%s NOTICE %s :SERVER SPLIT RECOVERY TIME is now set to %i",
			  me.name, parv[0], newval);
	      server_split_recovery_time = (newval * 60);
	      return 0;

	    }
	  else
	    {
	      sendto_one (source_p,
			  ":%s NOTICE %s :SERVER SPLIT RECOVERY TIME is currenly %i",
			  me.name, parv[0], server_split_recovery_time / 60);
	    }
	}
#endif
#ifdef ANTI_SPAMBOT
      /* int spam_time = MIN_JOIN_LEAVE_TIME; 
       * int spam_num = MAX_JOIN_LEAVE_COUNT;
       */
      else if (ircncmp (command, "SPAMNUM", 7) == 0)
	{
	  if (parc > 2)
	    {
	      int newval = atoi (parv[2]);
	      if (newval <= 0)
		{
		  sendto_one (source_p, ":%s NOTICE %s :spam NUM must be > 0",
			      me.name, parv[0]);
		  return 0;
		}
	      if (newval < MIN_SPAM_NUM)
		spam_num = MIN_SPAM_NUM;
	      else
		spam_num = newval;
	      sendto_ops ("%s has changed spam NUM to %i", parv[0], spam_num);
	      sendto_one (source_p,
			  ":%s NOTICE %s :spam NUM is now set to %i", me.name,
			  parv[0], spam_num);
	      return 0;
	    }
	  else
	    {
	      sendto_one (source_p, ":%s NOTICE %s :spam NUM is currently %i",
			  me.name, parv[0], spam_num);
	      return 0;
	    }
	}
      else if (ircncmp (command, "SPAMTIME", 8) == 0)
	{
	  if (parc > 2)
	    {
	      int newval = atoi (parv[2]);
	      if (newval <= 0)
		{
		  sendto_one (source_p,
			      ":%s NOTICE %s :spam TIME must be > 0", me.name,
			      parv[0]);
		  return 0;
		}
	      if (newval < MIN_SPAM_TIME)
		spam_time = MIN_SPAM_TIME;
	      else
		spam_time = newval;
	      sendto_ops ("%s has changed spam TIME to %i", parv[0],
			  spam_time);
	      sendto_one (source_p,
			  ":%s NOTICE %s :SPAM TIME is now set to %i",
			  me.name, parv[0], spam_time);
	      return 0;
	    }
	  else
	    {
	      sendto_one (source_p,
			  ":%s NOTICE %s :spam TIME is currently %i", me.name,
			  parv[0], spam_time);
	      return 0;
	    }
	}

#endif
#ifdef THROTTLE_ENABLE
      else if (ircncmp (command, "THROTTLE", 8) == 0)
	{
	  char *changed = NULL;
	  char *to = NULL;
	  /* several values available:
	   * ENABLE [on|off] to enable the code
	   * COUNT [n] to set a max count, must be > 1
	   * TIME [n] to set a max time before expiry, must be > 5
	   * RECORDTIME [n] to set a time for the throttle records to expire
	   * HASH [n] to set the size of the hash table, must be bigger than
	   *     the default */

	  /* only handle individual settings if parc > 3 (they're actually
	   * changing stuff) */
	  if (parc > 3)
	    {
	      if (irccmp (parv[2], "ENABLE") == 0)
		{
		  changed = "ENABLE";
		  if (tolower (*parv[3]) == 'y'
		      || (irccmp (parv[3], "on") == 0))
		    {
		      throttle_enable = 1;
		      to = "ON";
		    }
		  else if (tolower (*parv[3]) == 'n' ||
			   (irccmp (parv[3], "off") == 0))
		    {
		      throttle_enable = 0;
		      to = "OFF";
		    }
		}
	      else if (irccmp (parv[2], "COUNT") == 0)
		{
		  int cnt;
		  changed = "COUNT";
		  cnt = atoi (parv[3]);
		  if (cnt > 1)
		    {
		      throttle_tcount = cnt;
		      to = parv[3];
		    }
		}
	      else if (irccmp (parv[2], "TIME") == 0)
		{
		  int cnt;
		  changed = "TIME";
		  cnt = atoi (parv[3]);
		  if (cnt >= 5)
		    {
		      throttle_ttime = cnt;
		      to = parv[3];
		    }
		}
	      else if (irccmp (parv[2], "RECORDTIME") == 0)
		{
		  int cnt;
		  changed = "RECORDTIME";
		  cnt = atoi (parv[3]);
		  if (cnt >= 30)
		    {
		      throttle_rtime = cnt;
		      to = parv[3];
		    }
		}
	      else if (irccmp (parv[2], "HASH") == 0)
		{
		  int cnt;
		  changed = "HASH";
		  cnt = atoi (parv[3]);
		  if (cnt >= THROTTLE_HASHSIZE)
		    {
		      throttle_resize (cnt);
		      to = parv[3];
		    }
		}
	      if (to != NULL)
		{
		  sendto_ops ("%s has changed throttle %s to %s", parv[0],
			      changed, to);
		  sendto_one (source_p,
			      ":%s NOTICE %s :set throttle %s to %s", me.name,
			      parv[0], changed, to);
		}
	    }
	  else
	    {
	      /* report various things, we cannot easily get the hash size, so
	       * leave that alone. */
	      sendto_one (source_p, ":%s NOTICE %s :THROTTLE %s", me.name,
			  parv[0], throttle_enable ? "enabled" : "disabled");
	      sendto_one (source_p, ":%s NOTICE %s :THROTTLE COUNT=%d",
			  me.name, parv[0], throttle_tcount);
	      sendto_one (source_p, ":%s NOTICE %s :THROTTLE TIME=%d sec",
			  me.name, parv[0], throttle_ttime);
	      sendto_one (source_p, ":%s NOTICE %s :THROTTLE RECORDTIME=%d",
			  me.name, parv[0], throttle_rtime);
	    }
	}
#endif
      else
	{
	  sendto_one (source_p, ":%s NOTICE %s :Unknown option: %s", me.name,
		      parv[0], command);
	  return 0;
	}
    }
  else
    {
      sendto_one (source_p, ":%s NOTICE %s :Options: MAX", me.name, parv[0]);
#ifdef FLUD
      sendto_one (source_p,
		  ":%s NOTICE %s :Options: FLUDNUM, FLUDTIME, FLUDBLOCK",
		  me.name, parv[0]);
#endif

#ifdef ANTI_SPAMBOT
      sendto_one (source_p, ":%s NOTICE %s :Options: SPAMNUM, SPAMTIME",
		  me.name, parv[0]);
#endif

#ifdef NO_CHANOPS_WHEN_SPLIT
      sendto_one (source_p, ":%s NOTICE %s :Options: SPLITDELAY",
		  me.name, parv[0]);
#endif
      sendto_one (source_p, ":%s NOTICE %s :Options: THROTTLE "
		  "<ENABLE|COUNT|TIME|BANTIME|HASH> [setting]", me.name,
		  parv[0]);
    }
  return 0;
}

/*
 * m_htm - high traffic mode info
 */
int
m_htm (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
#define LOADCFREQ 5
  char *command;

  extern int lifesux, LRV, LCF, noisy_htm;	/* in ircd.c */
  extern int currlife;

  if (!MyClient (source_p) || !IsOper (source_p))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }
  sendto_one (source_p,
	      ":%s NOTICE %s :HTM is %s(%d), %s. Max rate = %dk/s. Current = %dk/s",
	      me.name, parv[0], lifesux ? "ON" : "OFF", lifesux,
	      noisy_htm ? "NOISY" : "QUIET", LRV, currlife);
  if (parc > 1)
    {
      command = parv[1];
      if (irccmp (command, "TO") == 0)
	{
	  if (parc > 2)
	    {
	      int new_value = atoi (parv[2]);

	      if (new_value < 10)
		sendto_one (source_p,
			    ":%s NOTICE %s :\002Cannot set LRV < 10!\002",
			    me.name, parv[0]);
	      else
		LRV = new_value;

	      sendto_one (source_p,
			  ":%s NOTICE %s :NEW Max rate = %dk/s. Current = %dk/s",
			  me.name, parv[0], LRV, currlife);
	      sendto_realops
		("%s!%s@%s set new HTM rate to %dk/s (%dk/s current)",
		 parv[0], source_p->user->username, source_p->sockhost, LRV,
		 currlife);
	    }
	  else
	    sendto_one (source_p,
			":%s NOTICE %s :LRV command needs an integer parameter",
			me.name, parv[0]);
	}
      else
	{
	  if (irccmp (command, "ON") == 0)
	    {
	      lifesux = 1;
	      sendto_one (source_p, ":%s NOTICE %s :HTM is now ON.", me.name,
			  parv[0]);
	      sendto_ops ("Entering high-traffic mode: Forced by %s!%s@%s",
			  parv[0], source_p->user->username,
			  IsHidden (source_p) ? source_p->user->
			  virthost : source_p->sockhost);
	      LCF = 30;		/* 30s */
	    }
	  else if (irccmp (command, "OFF") == 0)
	    {
#ifdef HTM_LOCK_ON_NETBURST
	      if (HTMLOCK == YES)
		{
		  if (!IsServerAdmin (source_p))
		    {
		      sendto_one (source_p,
				  ":%s NOTICE %s :Cannot change HTM - Currently LOCKED");
		      return 0;
		    }
		  else
		    {
		      sendto_one (source_p,
				  ":%s NOTICE %s :Removing HTM LOCK");
		      sendto_ops
			("Resuming standard operation through HTM LOCK: Forced by %s!%s@%s",
			 parv[0], source_p->user->username,
			 IsHidden (source_p) ? source_p->user->
			 virthost : source_p->sockhost);
		      lifesux = 0;
		      LCF = LOADCFREQ;
		      return 0;
		    }
		}
#endif

	      lifesux = 0;
	      LCF = LOADCFREQ;
	      sendto_one (source_p, ":%s NOTICE %s :HTM is now OFF.", me.name,
			  parv[0]);
	      sendto_ops ("Resuming standard operation: Forced by %s!%s@%s",
			  parv[0], source_p->user->username,
			  IsHidden (source_p) ? source_p->user->
			  virthost : source_p->sockhost);
	    }
	  else if (irccmp (command, "QUIET") == 0)
	    {
	      sendto_ops ("HTM is now QUIET");
	      noisy_htm = NO;
	    }
	  else if (irccmp (command, "NOISY") == 0)
	    {
	      sendto_ops ("HTM is now NOISY");
	      noisy_htm = YES;
	    }
	  else
	    sendto_one (source_p,
			":%s NOTICE %s :Commands are:HTM [ON] [OFF] [TO int] [QUIET] [NOISY]",
			me.name, parv[0]);
	}
    }
  return 0;
}

/*
 * cluster() input            
 * - pointer to a hostname output 
 * pointer to a static of the hostname masked for use in a kline. side 
 * effects - NONE
 * 
 * reworked a tad -Dianora
 */

static char *
cluster (char *hostname)
{
  static char result[HOSTLEN + 1];	/* result to return */
  char temphost[HOSTLEN + 1];	/* workplace */
  char *ipp;			/* used to find if host is ip # only */
  char *host_mask;		/* used to find host mask portion to '*' */
  char *zap_point = (char *) NULL;	/* used to zap last nnn portion of an ip # */
  char *tld;			/* Top Level Domain */
  int is_ip_number;		/* flag if its an IP # */
  int number_of_dots;		/* count # of dots for ip# and domain klines */

  if (!hostname)
    return (char *) NULL;	/* EEK! */

  /*
   * If a '@' is found in the hostname, this is bogus and must have
   * been introduced by server that doesn't check for bogus domains
   * (dns spoof) very well. *sigh* just return it... I could also
   * legitimately return (char *)NULL as above.
   *
   * -Dianora
   */

  if (strchr (hostname, '@'))
    {
      strncpyzt (result, hostname, HOSTLEN);
      return (result);
    }

  strncpyzt (temphost, hostname, HOSTLEN);

  is_ip_number = YES;		/* assume its an IP# */
  ipp = temphost;
  number_of_dots = 0;

  while (*ipp)
    {
      if (*ipp == '.')
	{
	  number_of_dots++;
	  if (number_of_dots == 3)
	    zap_point = ipp;
	  ipp++;
	}
      else if (!isdigit (*ipp))
	{
	  is_ip_number = NO;
	  break;
	}
      ipp++;
    }

  if (is_ip_number && (number_of_dots == 3))
    {
      zap_point++;
      *zap_point++ = '*';	/* turn 111.222.333.444 into ... */
      *zap_point = '\0';	/* 111.222.333.* */
      strncpy (result, temphost, HOSTLEN);
      return (result);
    }
  else
    {
      tld = strrchr (temphost, '.');
      if (tld)
	{
	  number_of_dots = 2;
	  if (tld[3])		/* its at least a 3 letter tld */
	    number_of_dots = 1;
	  if (tld != temphost)	/* in these days of dns spoofers ... */
	    host_mask = tld - 1;	/* Look for host portion to '*' */
	  else
	    host_mask = tld;	/* degenerate case hostname is '.com' ect. */

	  while (host_mask != temphost)
	    {
	      if (*host_mask == '.')
		number_of_dots--;
	      if (number_of_dots == 0)
		{
		  result[0] = '*';
		  strncpy (result + 1, host_mask, HOSTLEN - 1);
		  return (result);
		}
	      host_mask--;
	    }
	  result[0] = '*';	/* foo.com => *foo.com */
	  strncpy (result + 1, temphost, HOSTLEN);
	}
      else
	{			/*  no tld found oops. just return it as is */
	  strncpy (result, temphost, HOSTLEN);
	  return (result);
	}
    }

  return (result);
}

int
m_kline (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  struct userBan *ban, *oban;
#if defined (LOCKFILE)
  struct pkl *k;
#else
  int out;
#endif
  char buffer[1024];
  char *filename;		/* filename to use for kline */
  char *user, *host;
  char *reason;
  char *current_date;
  aClient *target_p;
  char tempuser[USERLEN + 2];
  char temphost[HOSTLEN + 1];
  int temporary_kline_time = 0;	/* -Dianora */
  time_t temporary_kline_time_seconds = 0;
  int time_specified = 0;
  char *argv;
  int i;
  char fbuf[512];

  if (!MyClient (source_p) || !IsAnOper (source_p) || !(source_p->oflag & OFLAG_KLINE) || !IsServer(source_p))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }

  if (parc < 2)
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
		  me.name, parv[0], "KLINE");
      return 0;
    }

  argv = parv[1];

  if ((temporary_kline_time = isnumber (argv)) >= 0)
    {
      if (parc < 3)
	{
	  sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
		      me.name, parv[0], "KLINE");
	  return 0;
	}
      if (temporary_kline_time > (24 * 60 * 7))
	temporary_kline_time = (24 * 60 * 7);	/*  Max it at 1 week */

      temporary_kline_time_seconds =
	(time_t) temporary_kline_time *(time_t) 60;

      /* turn it into minutes */
      argv = parv[2];
      parc--;
      time_specified = 1;
    }
  else
    {
      temporary_kline_time = 0;	/* -1 minute klines are bad... :) - lucas */
    }

  if (strchr (argv, ' '))
    {
      sendto_one (source_p,
		  ":%s NOTICE %s :Poorly formatted hostname "
		  "(contains spaces). Be sure you are using the form: "
		  "/quote KLINE [time] <user@host/nick> :<reason>",
		  me.name, parv[0]);
      return 0;
    }


  if ((host = strchr (argv, '@')) || *argv == '*')
    {
      /* Explicit user@host mask given */

      if (host)			/* Found user@host */
	{
	  user = argv;		/* here is user part */
	  *(host++) = '\0';	/* and now here is host */
	}
      else
	{
	  user = "*";		/* no @ found, assume its *@somehost */
	  host = argv;
	}

      if (!*host)		/* duh. no host found, assume its '*' host */
	host = "*";
      strncpyzt (tempuser, user, USERLEN + 2);	/* allow for '*' in front */
      strncpyzt (temphost, host, HOSTLEN);
      user = tempuser;
      host = temphost;
    }
  else
    {
      /* Try to find user@host mask from nick */

      if (!(target_p = find_chasing (source_p, argv, NULL)))
	return 0;

      if (!target_p->user)
	return 0;

      if (IsServer (target_p))
	{
	  sendto_one (source_p,
		      ":%s NOTICE %s :Can't KLINE a server, use @'s "
		      "where appropriate", me.name, parv[0]);
	  return 0;
	}
      /*
       * turn the "user" bit into "*user", blow away '~' if found in
       * original user name (non-idented)
       */

      tempuser[0] = '*';
      if (*target_p->user->username == '~')
	strcpy (tempuser + 1, (char *) target_p->user->username + 1);
      else
	strcpy (tempuser + 1, target_p->user->username);
      user = tempuser;
      host = cluster (target_p->user->host);
    }

  if (time_specified)
    argv = parv[3];
  else
    argv = parv[2];

#ifdef DEFAULT_KLINE_TIME
  if (time_specified == 0)
    {
      temporary_kline_time = DEFAULT_KLINE_TIME;
      temporary_kline_time_seconds =
	(time_t) temporary_kline_time *(time_t) 60;
    }
#endif

  if (parc > 2)
    {
      if (*argv)
	reason = argv;
      else
	reason = "No reason";
    }
  else
    reason = "No reason";

  if (!match (user, "akjhfkahfasfjd") &&
      !match (host, "ldksjfl.kss...kdjfd.jfklsjf"))
    {
      sendto_one (source_p, ":%s NOTICE %s :Can't K-Line *@*", me.name,
		  parv[0]);
      return 0;
    }

  /* we can put whatever we want in temp K: lines */
  if (temporary_kline_time == 0 && strchr (reason, ':'))
    {
      sendto_one (source_p,
		  ":%s NOTICE %s :Invalid character ':' in comment",
		  me.name, parv[0]);
      return 0;
    }

  if (temporary_kline_time == 0 && strchr (reason, '#'))
    {
      sendto_one (source_p,
		  ":%s NOTICE %s :Invalid character '#' in comment",
		  me.name, parv[0]);
      return 0;
    }

  ban = make_hostbased_ban (user, host);
  if (!ban)
    {
      sendto_one (source_p, ":%s NOTICE %s :Malformed ban %s@%s", me.name,
		  parv[0], user, host);
      return 0;
    }

  if ((oban = find_userban_exact (ban, 0)))
    {
      char *ktype =
	(oban->flags & UBAN_LOCAL) ? LOCAL_BANNED_NAME : NETWORK_BANNED_NAME;

      sendto_one (source_p, ":%s NOTICE %s :[%s@%s] already %s for %s",
		  me.name, parv[0], user, host, ktype,
		  oban->reason ? oban->reason : "<No Reason>");

      userban_free (ban);
      return 0;
    }

  current_date = smalldate ((time_t) 0);
  ircsprintf (buffer, "%s (%s)", reason, current_date);

  ban->flags |= UBAN_LOCAL;
  ban->reason = (char *) MyMalloc (strlen (buffer) + 1);
  strcpy (ban->reason, buffer);

  if (temporary_kline_time)
    {
      ban->flags |= UBAN_TEMPORARY;
      ban->timeset = timeofday;
      ban->duration = temporary_kline_time_seconds;
    }

  if (user_match_ban (source_p, ban))
    {
      sendto_one (source_p,
		  ":%s NOTICE %s :You attempted to add a ban [%s@%s] which would affect yourself. Aborted.",
		  me.name, parv[0], user, host);
      userban_free (ban);
      return 0;
    }

  add_hostbased_userban (ban);

  /* Check local users against it */
  for (i = highest_fd; i >= 0; i--)
    {
      if (!(target_p = local[i]) || IsMe (target_p) || IsLog (target_p))
	continue;

      if (IsPerson (target_p) && user_match_ban (target_p, ban)
	  && !IsExempt (target_p))
	{
	  sendto_realops (LOCAL_BAN_NAME " active for %s",
			  get_client_name (target_p, FALSE));
	  ircsprintf (fbuf, LOCAL_BANNED_NAME ": %s", reason);
	  exit_client (target_p, target_p, &me, fbuf);
	}
    }

  host = get_userban_host (ban, fbuf, 512);

  if (temporary_kline_time)
    {
      sendto_realops ("%s added temporary %d min. " LOCAL_BAN_NAME
		      " for [%s@%s] [%s]", parv[0], temporary_kline_time,
		      user, host, reason);
      return 0;
    }

  filename = klinefile;

#ifdef KPATH
  sendto_one (source_p,
	      ":%s NOTICE %s :Added K-Line [%s@%s] to server klinefile",
	      me.name, parv[0], user, host);
#else
  sendto_one (source_p, ":%s NOTICE %s :Added K-Line [%s@%s] to server "
	      "configfile", me.name, parv[0], user, host);
#endif /* KPATH */

  sendto_realops ("%s added K-Line for [%s@%s] [%s]",
		  parv[0], user, host, reason);

#if defined(LOCKFILE)
  if ((k = (struct pkl *) malloc (sizeof (struct pkl))) == NULL)
    {
      sendto_one (source_p, ":%s NOTICE %s :Problem allocating memory",
		  me.name, parv[0]);
      return (0);
    }

  (void) ircsprintf (buffer, "#%s!%s@%s K'd: %s@%s:%s\n",
		     source_p->name, source_p->user->username,
		     source_p->user->host, user, host, reason);

  if ((k->comment = strdup (buffer)) == NULL)
    {
      free (k);
      sendto_one (source_p, ":%s NOTICE %s :Problem allocating memory",
		  me.name, parv[0]);
      return (0);
    }

  (void) ircsprintf (buffer, "K:%s:%s (%s):%s\n",
		     host, reason, current_date, user);

  if ((k->kline = strdup (buffer)) == NULL)
    {
      free (k->comment);
      free (k);
      sendto_one (source_p, ":%s NOTICE %s :Problem allocating memory",
		  me.name, parv[0]);
      return (0);
    }
  k->next = pending_klines;
  pending_klines = k;

  do_pending_klines ();
  return (0);

#else /*  LOCKFILE - MDP */

  if ((out = open (filename, O_RDWR | O_APPEND | O_CREAT)) == -1)
    {
      sendto_one (source_p, ":%s NOTICE %s :Problem opening %s ",
		  me.name, parv[0], filename);
      return 0;
    }

  (void) ircsprintf (buffer, "#%s!%s@%s K'd: %s@%s:%s\n",
		     source_p->name, source_p->user->username,
		     source_p->user->host, user, host, reason);

  if (write (out, buffer, strlen (buffer)) <= 0)
    {
      sendto_one (source_p, ":%s NOTICE %s :Problem writing to %s",
		  me.name, parv[0], filename);
      (void) close (out);
      return 0;
    }

  (void) ircsprintf (buffer, "K:%s:%s (%s):%s\n", host, reason,
		     current_date, user);

  if (write (out, buffer, strlen (buffer)) <= 0)
    {
      sendto_one (source_p, ":%s NOTICE %s :Problem writing to %s",
		  me.name, parv[0], filename);
      (void) close (out);
      return 0;
    }

  (void) close (out);

#ifdef USE_SYSLOG
  syslog (LOG_NOTICE, "%s added K-Line for [%s@%s] [%s]", parv[0],
	  user, host, reason);
#endif

  return 0;
#endif /* LOCKFILE */
}

/*
 * isnumber()
 * 
 * inputs               
 * - pointer to ascii string in output             
 * - 0 if not an integer number, else the number side effects  
 * - none return -1 if not an integer. 
 * (if someone types in maxint, oh well..) - lucas
 */

static int
isnumber (char *p)
{
  int result = 0;

  while (*p)
    {
      if (isdigit (*p))
	{
	  result *= 10;
	  result += ((*p) & 0xF);
	  p++;
	}
      else
	return (-1);
    }
  /*
   * in the degenerate case where oper does a /quote kline 0 user@host
   * :reason i.e. they specifically use 0, I am going to return 1
   * instead as a return value of non-zero is used to flag it as a
   * temporary kline
   */

  /*
   * er, no. we return 0 because 0 means that it's a permanent kline. -lucas
   * oh, and we only do this if DEFAULT_KLINE_TIME is specified.
   */

#ifndef DEFAULT_KLINE_TIME
  if (result == 0)
    result = 1;
#endif

  return (result);
}

#ifdef UNKLINE
/*
 * * m_unkline
 * Added Aug 31, 1997
 * common (Keith Fralick) fralick@gate.net
 *
 *      parv[0] = sender
 *      parv[1] = address to remove
 *
 * re-worked and cleanedup for use in hybrid-5 -Dianora
 *
 */
int
m_unkline (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  struct userBan *ban;
  int in, out;
  int pairme = NO;
  char buf[256], buff[256];
  char temppath[256];

  char *filename;		/* filename to use for unkline */
  char *user, *host;
  char *p;
  int nread;
  int error_on_write = NO;
  struct stat oldfilestat;
  mode_t oldumask;

  ircsprintf (temppath, "%s.tmp", klinefile);

  if (!IsAnOper (source_p) || !(source_p->oflag & OFLAG_UNKLINE))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }

  if (parc < 2)
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
		  me.name, parv[0], "UNKLINE");
      return 0;
    }

  if ((host = strchr (parv[1], '@')) || *parv[1] == '*')
    {
      /* Explicit user@host mask given */

      if (host)			/* Found user@host */
	{
	  user = parv[1];	/* here is user part */
	  *(host++) = '\0';	/* and now here is host */
	}
      else
	{
	  user = "*";		/* no @ found, assume its *@somehost */
	  host = parv[1];
	}
    }
  else
    {
      sendto_one (source_p, ":%s NOTICE %s :Invalid parameters",
		  me.name, parv[0]);
      return 0;
    }

  if ((user[0] == '*') && (user[1] == '\0') && (host[0] == '*') &&
      (host[1] == '\0'))
    {
      sendto_one (source_p, ":%s NOTICE %s :Cannot UNK-Line everyone",
		  me.name, parv[0]);
      return 0;
    }

  ban = make_hostbased_ban (user, host);
  if (ban)
    {
      struct userBan *oban;

      ban->flags |= (UBAN_LOCAL | UBAN_TEMPORARY);
      if ((oban = find_userban_exact (ban, UBAN_LOCAL | UBAN_TEMPORARY)))
	{
	  char tmp[512];

	  host = get_userban_host (oban, tmp, 512);

	  remove_userban (oban);
	  userban_free (oban);
	  userban_free (ban);

	  sendto_one (source_p,
		      ":%s NOTICE %s :K-Line for [%s@%s] is removed", me.name,
		      parv[0], user, host);
	  sendto_ops ("%s has removed the K-Line for: [%s@%s] (%d matches)",
		      parv[0], user, host, 1);

	  return 0;
	}

      userban_free (ban);
    }

# if defined(LOCKFILE)
  if (lock_kline_file () < 0)
    {
      sendto_one (source_p, ":%s NOTICE %s :%s is locked try again in a "
		  "few minutes", me.name, parv[0], klinefile);
      return -1;
    }
# endif

  filename = klinefile;

  if ((in = open (filename, O_RDONLY)) == -1)
    {
      sendto_one (source_p, ":%s NOTICE %s :Cannot open %s",
		  me.name, parv[0], filename);
# if defined(LOCKFILE)
      (void) unlink (LOCKFILE);
# endif
      return 0;
    }
  if (fstat (in, &oldfilestat) < 0)	/*  Save the old file mode */
    oldfilestat.st_mode = 0644;

  oldumask = umask (0);		/* ircd is normally too paranoid */

  if ((out = open (temppath, O_WRONLY | O_CREAT, oldfilestat.st_mode)) == -1)
    {
      sendto_one (source_p, ":%s NOTICE %s :Cannot open %s",
		  me.name, parv[0], temppath);
      (void) close (in);
# if defined (LOCKFILE)
      (void) unlink (LOCKFILE);
# endif
      umask (oldumask);		/* Restore the old umask */
      return 0;
    }
  umask (oldumask);		/* Restore the old umask */

  /*
   * #Dianora!db@ts2-11.ottawa.net K'd: foo@bar:No reason K:bar:No
   * reason (1997/08/30 14.56):foo
   */

  remove_userbans_match_flags (UBAN_LOCAL, UBAN_TEMPORARY);

  while ((nread = dgets (in, buf, sizeof (buf))) > 0)
    {
      buf[nread] = '\0';

      if ((buf[1] == ':') && ((buf[0] == 'k') || (buf[0] == 'K')))
	{
	  /* its a K: line */

	  char *found_host;
	  char *found_user;
	  char *found_comment;

	  strcpy (buff, buf);

	  p = strchr (buff, '\n');
	  if (p)
	    *p = '\0';
	  p = strchr (buff, '\r');
	  if (p)
	    *p = '\0';

	  found_host = buff + 2;	/* point past the K: */

	  p = strchr (found_host, ':');
	  if (p == (char *) NULL)
	    {
	      sendto_one (source_p, ":%s NOTICE %s :K-Line file corrupted",
			  me.name, parv[0]);
	      sendto_one (source_p, ":%s NOTICE %s :Couldn't find host",
			  me.name, parv[0]);
	      if (!error_on_write)
		error_on_write = flush_write (source_p, parv[0],
					      out, buf, strlen (buf),
					      temppath);
	      continue;		/* This K line is corrupted ignore */
	    }
	  *p = '\0';
	  p++;

	  found_comment = p;
	  p = strchr (found_comment, ':');
	  if (p == (char *) NULL)
	    {
	      sendto_one (source_p, ":%s NOTICE %s :K-Line file corrupted",
			  me.name, parv[0]);
	      sendto_one (source_p, ":%s NOTICE %s :Couldn't find comment",
			  me.name, parv[0]);
	      if (!error_on_write)
		error_on_write = flush_write (source_p, parv[0],
					      out, buf, strlen (buf),
					      temppath);
	      continue;		/* This K line is corrupted ignore */
	    }
	  *p = '\0';
	  p++;

	  found_user = p;
	  /*
	   * Ok, if its not an exact match on either the user or the
	   * host then, write the K: line out, and I add it back to the
	   * K line tree
	   */
	  if ((irccmp (host, found_host) != 0)
	      || (irccmp (user, found_user) != 0))
	    {
	      struct userBan *tban;
	      char *ub_u, *ub_r;

	      if (!error_on_write)
		error_on_write = flush_write (source_p, parv[0],
					      out, buf, strlen (buf),
					      temppath);

	      if (BadPtr (found_host))
		continue;

	      ub_u = BadPtr (found_user) ? "*" : found_user;
	      ub_r = BadPtr (found_comment) ? "<No Reason>" : found_comment;

	      if (!(tban = make_hostbased_ban (ub_u, found_host)))
		continue;

	      tban->flags |= UBAN_LOCAL;
	      tban->reason = (char *) MyMalloc (strlen (ub_r) + 1);
	      strcpy (tban->reason, ub_r);
	      tban->timeset = NOW;

	      add_hostbased_userban (tban);
	    }
	  else
	    pairme++;
	}
      else if (buf[0] == '#')
	{
	  char *userathost;
	  char *found_user;
	  char *found_host;

	  strcpy (buff, buf);
	  /*
	   * #Dianora!db@ts2-11.ottawa.net K'd: foo@bar:No reason
	   * K:bar:No reason (1997/08/30 14.56):foo
	   *
	   * If its a comment coment line, i.e. #ignore this line Then just
	   * ignore the line
	   */
	  p = strchr (buff, ':');
	  if (p == (char *) NULL)
	    {
	      if (!error_on_write)
		error_on_write = flush_write (source_p, parv[0],
					      out, buf, strlen (buf),
					      temppath);
	      continue;
	    }
	  *p = '\0';
	  p++;

	  userathost = p;
	  p = strchr (userathost, ':');

	  if (p == (char *) NULL)
	    {
	      if (!error_on_write)
		error_on_write = flush_write (source_p, parv[0],
					      out, buf, strlen (buf),
					      temppath);
	      continue;
	    }
	  *p = '\0';

	  while (*userathost == ' ')
	    userathost++;

	  found_user = userathost;
	  p = strchr (found_user, '@');
	  if (p == (char *) NULL)
	    {
	      if (!error_on_write)
		error_on_write = flush_write (source_p, parv[0],
					      out, buf, strlen (buf),
					      temppath);
	      continue;
	    }
	  *p = '\0';
	  found_host = p;
	  found_host++;

	  if ((irccmp (found_host, host) != 0) ||
	      (irccmp (found_user, user) != 0))
	    {
	      if (!error_on_write)
		error_on_write = flush_write (source_p, parv[0],
					      out, buf, strlen (buf),
					      temppath);
	    }
	}
      else
	{			/* its the ircd.conf file, and not a Kline or comment */
	  if (!error_on_write)
	    error_on_write = flush_write (source_p, parv[0],
					  out, buf, strlen (buf), temppath);
	}
    }

  (void) close (in);

  /* The result of the rename should be checked too... oh well */
  /*
   * If there was an error on a write above, then its been reported
   * and I am not going to trash the original kline /conf file
   * -Dianora
   */
  if ((!error_on_write) && (close (out) >= 0))
    (void) rename (temppath, filename);
  else
    {
      sendto_one (source_p,
		  ":%s NOTICE %s :Couldn't write temp kline file, aborted",
		  me.name, parv[0]);
# if defined (LOCKFILE)
      (void) unlink (LOCKFILE);
# endif
      return -1;
    }

# if defined (LOCKFILE)
  (void) unlink (LOCKFILE);
# endif

  if (pairme == NO)
    {
      sendto_one (source_p, ":%s NOTICE %s :No K-Line for %s@%s",
		  me.name, parv[0], user, host);
      return 0;
    }

  sendto_one (source_p, ":%s NOTICE %s :K-Line for [%s@%s] is removed",
	      me.name, parv[0], user, host);
  sendto_ops ("%s has removed the K-Line for: [%s@%s] (%d matches)",
	      parv[0], user, host, pairme);
  return 0;
}

/*
 * flush_write()
 *
 * inputs               
 * - pointer to client structure of oper requesting unkline
 * - out is the file descriptor
 * - buf is the buffer to write
 * - ntowrite is the expected number of character to be written 
 * - temppath is the temporary file name to be written output
 *   YES for error on write
 *   NO for success side effects
 * - if successful, the buf is written to output file if a write failure
 *   happesn, and the file pointed to by temppath, if its non NULL, is
 *   removed.
 *
 * The idea here is, to be as robust as possible when writing to the kline
 * file.
 *
 * Yes, I could have dug the opernick out of source_p. I didn't feel like it.
 * so sue me.
 * 
 * -Dianora
 */

static int
flush_write (aClient * source_p, char *opernick,
	     int out, char *buf, int ntowrite, char *temppath)
{
  int nwritten;
  int error_on_write = NO;

  nwritten = write (out, buf, ntowrite);
  if (nwritten != ntowrite)
    {
      sendto_one (source_p, ":%s NOTICE %s :Unable to write to %s",
		  me.name, opernick, temppath);
      error_on_write = YES;
      (void) close (out);
      if (temppath != (char *) NULL)
	(void) unlink (temppath);
    }
  return (error_on_write);
}
#endif /* UNKLINE */


/*
 * * m_rehash *
 *
 */
int
m_rehash (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  if (!(source_p->oflag & OFLAG_REHASH))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }

  if (parc > 1)
    {
      if (irccmp (parv[1], "DNS") == 0)
	{
	  sendto_one (source_p, rpl_str (RPL_REHASHING), me.name, parv[0],
		      "DNS");
	  flush_cache ();	/* flush the dns cache */
	  res_init ();		/* re-read /etc/resolv.conf */
	  sendto_ops ("%s is rehashing DNS while whistling innocently",
		      parv[0]);
	  return 0;
	}
      else if (irccmp (parv[1], "TKLINES") == 0)
	{
	  sendto_one (source_p, rpl_str (RPL_REHASHING), me.name, parv[0],
		      "temp klines");
	  remove_userbans_match_flags (UBAN_LOCAL | UBAN_TEMPORARY, 0);
	  sendto_ops ("%s is clearing temp klines while whistling innocently",
		      parv[0]);
	  return 0;
	}
      else if (irccmp (parv[1], "GC") == 0)
	{
	  sendto_one (source_p, rpl_str (RPL_REHASHING), me.name, parv[0],
		      "garbage collecting");
	  block_garbage_collect ();
	  sendto_ops ("%s is garbage collecting while whistling innocently",
		      parv[0]);
	  return 0;
	}
      else if (irccmp (parv[1], "MOTD") == 0)
	{
	  sendto_one (source_p, rpl_str (RPL_REHASHING), me.name, parv[0],
		      "motd");
	  sendto_ops ("%s is forcing re-reading of MOTD file", parv[0]);
	  read_motd (MOTD);

	  if (SHORT_MOTD == 1)
	    read_shortmotd (SHORTMOTD);

	  return (0);
	}
      else if (irccmp (parv[1], "OPERMOTD") == 0)
	{
	  sendto_one (source_p, rpl_str (RPL_REHASHING), me.name, parv[0],
		      "opermotd");
	  sendto_ops ("%s is forcing re-reading of OPERMOTD file", parv[0]);
	  read_opermotd (OMOTD);
	  return (0);
	}

      else if (irccmp (parv[1], "IP") == 0)
	{
	  sendto_one (source_p, rpl_str (RPL_REHASHING), me.name, parv[0],
		      "ip hash");
	  rehash_ip_hash ();
	  sendto_ops ("%s is rehashing iphash while whistling innocently",
		      parv[0]);
	}
      else if (irccmp (parv[1], "DYNCONF") == 0)
	{
	  sendto_one (source_p, rpl_str (RPL_REHASHING), me.name, parv[0],
		      "dynconf");
	  load_conf (DCONF, 1);
	  sendto_ops ("%s is rehashing dynamic config files", parv[0]);
	  return 0;
	}
      else if (irccmp (parv[1], "CHANNELS") == 0)
	{
	  sendto_one (source_p, rpl_str (RPL_REHASHING), me.name, parv[0],
		      "%s", IRCD_RESTRICT);
	  cr_rehash ();
	  sendto_ops ("%s is rehashing channel restrict config file",
		      parv[0]);
	  return 0;
	}
#ifdef THROTTLE_ENABLE
      else if (irccmp (parv[1], "THROTTLES") == 0)
	{
	  sendto_one (source_p, rpl_str (RPL_REHASHING), me.name, parv[0],
		      "throttles");
	  throttle_rehash ();
	  sendto_ops ("%s is rehashing throttles", parv[0]);
	  return 0;
	}
#endif
      else
	{
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- Supported parameters:",
		      me.name, parv[0]);
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- \2DNS\2 -- Flushes the DNS cache and rereads /etc/resolve.conf",
		      me.name, parv[0]);
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- \2TKLINES\2 -- Clears temporary kLines",
		      me.name, parv[0]);
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- \2GC\2 -- Garbage Collecting",
		      me.name, parv[0]);
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- \2MOTD\2 -- Re-read the MOTD file and SHORT MOTD file",
		      me.name, parv[0]);
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- \2OPERMOTD\2 -- Re-read the OPER MOTD file",
		      me.name, parv[0]);
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- \2IP\2 -- Rehash the iphash",
		      me.name, parv[0]);
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- \2AKILLS\2 -- Rehash akills",
		      me.name, parv[0]);
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- \2DYNCONF\2 -- Rehash the dynamic config files",
		      me.name, parv[0]);
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- \2CHANNELS\2 -- Rehash the channel restrict config file",
		      me.name, parv[0]);
#ifdef THROTTLE_ENABLE
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- \2THROTTLES\2 -- Rehash the throttles",
		      me.name, parv[0]);
#endif
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- End of REHASH help",
		      me.name, parv[0]);
	  return 0;
	}
    }
  else
    {
      sendto_one (source_p, rpl_str (RPL_REHASHING), me.name, parv[0],
		  configfile);
      sendto_ops
	("%s is rehashing Server config file while whistling innocently",
	 parv[0]);
# ifdef USE_SYSLOG
      syslog (LOG_INFO, "REHASH From %s\n",
	      get_client_name (source_p, FALSE));
# endif
      return rehash (client_p, source_p,
		     (parc > 1) ? ((*parv[1] == 'q') ? 2 : 0) : 0);
    }
  return 0;			/* shouldn't ever get here */
}

/*
 * * m_restart *
 * 
 */
int
m_restart (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  char *pass = NULL;

  if (!(source_p->oflag & OFLAG_RESTART))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }
  /*
   * m_restart is now password protected as in df465 only change --
   * this one doesn't allow a reason to be specified. future changes:
   * crypt()ing of password, reason to be re-added -mjs
   */
  if ((pass = (char *) find_restartpass ()))
    {
      if (parc < 2)
	{
	  sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS), me.name,
		      parv[0], "RESTART");
	  return 0;
	}
      if (strcmp (pass, parv[1]))
	{
	  sendto_one (source_p, err_str (ERR_PASSWDMISMATCH), me.name,
		      parv[0]);
	  return 0;
	}
    }

#ifdef USE_SYSLOG
  syslog (LOG_WARNING, "Server RESTART by %s\n",
	  get_client_name (source_p, FALSE));
#endif
  sprintf (buf, "Server RESTART by %s", get_client_name (source_p, TRUE));
  restart (buf);
  return 0;			/* NOT REACHED */
}

/*
 * * m_trace
 *        parv[0] = sender prefix
 *        parv[1] = servername
 */
int
m_trace (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  int i;
  aClient *target_p = NULL;
  aClass *cltmp;
  char *tname;
  int doall, link_s[MAXCONNECTIONS + 1], link_u[MAXCONNECTIONS + 1];
  int cnt = 0, wilds = 0, dow = 0;

  tname = (parc > 1) ? parv[1] : me.name;

  if (HIDEULINEDSERVS == 1)
    {
      if ((target_p = next_client_double (client, tname)))
	{
	  if (!(IsAnOper (source_p)) && IsULine (target_p))
	    {
	      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name,
			  parv[0]);
	      return 0;
	    }
	  target_p = NULL;	/* shrug, we borrowed it, reset it just in case */
	}
    }

  if (parc > 2)
    if (hunt_server (client_p, source_p, ":%s TRACE %s :%s", 2, parc, parv))
      return 0;

  switch (hunt_server (client_p, source_p, ":%s TRACE :%s", 1, parc, parv))
    {
    case HUNTED_PASS:		/*  note: gets here only if parv[1] exists */
      {
	aClient *ac2ptr = next_client_double (client, tname);
	if (ac2ptr)
	  sendto_one (source_p, rpl_str (RPL_TRACELINK), me.name, parv[0],
		      version, debugmode, tname, ac2ptr->from->name);
	else
	  sendto_one (source_p, rpl_str (RPL_TRACELINK), me.name, parv[0],
		      version, debugmode, tname, "ac2ptr_is_NULL!!");
	return 0;
      }
    case HUNTED_ISME:
      break;
    default:
      return 0;
    }
  if (!IsAnOper (source_p))
    {
      if (parv[1] && !strchr (parv[1], '.') && (strchr (parv[1], '*')
						|| strchr (parv[1], '?')))
	/* bzzzt, no wildcard nicks for nonopers */
	{
	  sendto_one (source_p, rpl_str (RPL_ENDOFTRACE), me.name,
		      parv[0], parv[1]);
	  return 0;
	}
    }
  sendto_snomask (SNOMASK_SPY, "TRACE requested by %s (%s@%s) [%s]",
		      source_p->name, source_p->user->username,
		      source_p->user->host, source_p->user->server);

  doall = (parv[1] && (parc > 1)) ? !match (tname, me.name) : TRUE;
  wilds = !parv[1] || strchr (tname, '*') || strchr (tname, '?');
  dow = wilds || doall;
  if (!IsAnOper (source_p) || !dow)	/* non-oper traces must be full nicks */
    /* lets also do this for opers tracing nicks */
    {
      char *name;
      int class;
      target_p = hash_find_client (tname, (aClient *) NULL);
      if (!target_p || !IsPerson (target_p))
	{
	  /* this should only be reached if the matching
	     target is this server */
	  sendto_one (source_p, rpl_str (RPL_ENDOFTRACE), me.name,
		      parv[0], tname);
	  return 0;

	}
      name = get_client_name (target_p, FALSE);
      class = get_client_class (target_p);
      /*
       ** By all means we do NOT want to send every user the real IP at this point.
       ** We mite want to write a new switch into get_client_name where we give the hiddenhost.
       ** For now we just give users the 0.0.0.0 IP reply - ShadowMaster
       */
      if (IsAnOper (target_p))
	{
	  sendto_one (source_p, rpl_str (RPL_TRACEOPERATOR),
		      me.name, parv[0], class, get_client_name (target_p,
								HIDEME),
		      timeofday - target_p->lasttime);
	}
      else
	{
	  sendto_one (source_p, rpl_str (RPL_TRACEUSER),
		      me.name, parv[0], class, get_client_name (target_p,
								HIDEME),
		      timeofday - target_p->lasttime);
	}
      sendto_one (source_p, rpl_str (RPL_ENDOFTRACE), me.name, parv[0],
		  tname);
      return 0;
    }
  if (dow && lifesux && !IsOper (source_p))
    {
      sendto_one (source_p, rpl_str (RPL_LOAD2HI), me.name, parv[0]);
      return 0;
    }

  memset ((char *) link_s, '\0', sizeof (link_s));
  memset ((char *) link_u, '\0', sizeof (link_u));
  /*
   * Count up all the servers and clients in a downlink.
   */
  if (doall)
    {
      for (target_p = client; target_p; target_p = target_p->next)
	{
	  if (IsPerson (target_p)
	      && (!IsInvisible (target_p) || IsAnOper (source_p)))
	    {
	      if (target_p->from->fd == me.fd)
		{
		  link_u[MAXCONNECTIONS]++;
		}
	      else
		{
		  link_u[target_p->from->fd]++;
		}
	    }
	  else if (IsServer (target_p))
	    {
	      if ((HIDEULINEDSERVS == 1)
		  && (IsOper (source_p) || !IsULine (target_p)))
		{
		  if (target_p->from->fd == me.fd)
		    {
		      link_s[MAXCONNECTIONS]++;
		    }
		  else
		    {
		      link_s[target_p->from->fd]++;
		    }
		}
	    }
	}
    }


  /*
   * report all direct connections
   */

  for (i = 0; i <= highest_fd; i++)
    {
      char *name;
      int class;

      if (!(target_p = local[i]))	/* Local Connection? */
	continue;
      if (HIDEULINEDSERVS == 1)
	{
	  if (!IsOper (source_p) && IsULine (target_p))
	    continue;
	}
      if (IsInvisible (target_p) && dow &&
	  !(MyConnect (source_p) && IsAnOper (source_p)) &&
	  !IsAnOper (target_p) && (target_p != source_p))
	continue;
      if (!doall && wilds && match (tname, target_p->name))
	continue;
      if (!dow && (irccmp (tname, target_p->name) != 0))
	continue;
      if (IsAnOper (source_p))
	name = get_client_name (target_p, FALSE);
      else
	name = get_client_name (target_p, HIDEME);
      class = get_client_class (target_p);

      switch (target_p->status)
	{
	case STAT_CONNECTING:
	  sendto_one (source_p, rpl_str (RPL_TRACECONNECTING), me.name,
		      parv[0], class, name);
	  cnt++;
	  break;
	case STAT_HANDSHAKE:
	  sendto_one (source_p, rpl_str (RPL_TRACEHANDSHAKE), me.name,
		      parv[0], class, name);
	  cnt++;
	  break;
	case STAT_ME:
	  break;
	case STAT_UNKNOWN:
	  /* added time -Taner */
	  sendto_one (source_p, rpl_str (RPL_TRACEUNKNOWN),
		      me.name, parv[0], class, name,
		      target_p->firsttime ? timeofday -
		      target_p->firsttime : -1);
	  cnt++;
	  break;
	case STAT_CLIENT:
	  /*
	   * Only opers see users if there is a wildcard but
	   * anyone can see all the opers.
	   */
	  if (((IsAnOper (source_p) &&
		(MyClient (source_p))) || !(dow && IsInvisible (target_p)))
	      || !dow || IsAnOper (target_p))
	    {
	      if (IsAnOper (target_p))
		sendto_one (source_p,
			    rpl_str (RPL_TRACEOPERATOR),
			    me.name, parv[0], class, name,
			    timeofday - target_p->lasttime);
	    }
	  break;
	case STAT_SERVER:
	  sendto_one (source_p, rpl_str (RPL_TRACESERVER),
		      me.name, parv[0], class, link_s[i],
		      link_u[i], name,
		      *(target_p->serv->bynick) ? target_p->serv->
		      bynick : "*",
		      *(target_p->serv->byuser) ? target_p->serv->
		      byuser : "*",
		      *(target_p->serv->byhost) ? target_p->serv->byhost : me.
		      name);
	  cnt++;
	  break;
	case STAT_LOG:
	  sendto_one (source_p, rpl_str (RPL_TRACELOG), me.name,
		      parv[0], LOGFILE, target_p->port);
	  cnt++;
	  break;
	default:		/*
				 * ...we actually shouldn't come
				 * * here... --msa 
				 */
	  sendto_one (source_p, rpl_str (RPL_TRACENEWTYPE), me.name,
		      parv[0], name);
	  cnt++;
	  break;
	}
    }
  /*
   * Add these lines to summarize the above which can get rather long
   * and messy when done remotely - Avalon
   */
  if (!(source_p->oflag & OFLAG_WALLOPS) || !cnt)
    {
      if (cnt)
	{
	  sendto_one (source_p, rpl_str (RPL_ENDOFTRACE), me.name,
		      parv[0], tname);
	  return 0;
	}
      /* let the user have some idea that its at the end of the trace */
      sendto_one (source_p, rpl_str (RPL_TRACESERVER),
		  me.name, parv[0], 0, link_s[MAXCONNECTIONS],
		  link_u[MAXCONNECTIONS], me.name, "*", "*", me.name,
		  timeofday - target_p->lasttime);
      sendto_one (source_p, rpl_str (RPL_ENDOFTRACE), me.name, parv[0],
		  tname);
      return 0;
    }

  if ((HIDEULINEDSERVS == 1) && IsOper (source_p))
    for (cltmp = FirstClass (); doall && cltmp; cltmp = NextClass (cltmp))
      if (Links (cltmp) > 0)
	sendto_one (source_p, rpl_str (RPL_TRACECLASS), me.name,
		    parv[0], Class (cltmp), Links (cltmp));

  sendto_one (source_p, rpl_str (RPL_ENDOFTRACE), me.name, parv[0], tname);
  return 0;
}

/*
 * * m_motd
 * 	 parv[0] = sender prefix
 *       parv[1] = servername
 */
int
m_motd (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  static time_t last_used = 0L;
  if (hunt_server (client_p, source_p, ":%s MOTD :%s", 1, parc, parv) !=
      HUNTED_ISME)
    return 0;
  if (!IsAnOper (source_p))
    {
      if ((last_used + MOTD_WAIT) > NOW)
	return 0;
      else
	last_used = NOW;

    }
  sendto_snomask (SNOMASK_SPY, "MOTD requested by %s (%s@%s) [%s]",
		      source_p->name, source_p->user->username,
		      source_p->user->host, source_p->user->server);
  send_motd (client_p, source_p, parc, parv);
  return 0;
}

/*
 ** send_motd
 **  parv[0] = sender prefix
 **  parv[1] = servername
 **
 ** This function split off so a server notice could be generated on a
 ** user requested motd, but not on each connecting client.
 ** -Dianora
 */
int
send_motd (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aMotd *temp;
  struct tm *tm;

  tm = motd_tm;
  if (motd == (aMotd *) NULL)
    {
      sendto_one (source_p, err_str (ERR_NOMOTD), me.name, parv[0]);
      return 0;
    }
  sendto_one (source_p, rpl_str (RPL_MOTDSTART), me.name, parv[0], me.name);

  if (tm)
    sendto_one (source_p,
		":%s %d %s :- %d/%d/%d %d:%02d", me.name, RPL_MOTD,
		parv[0], tm->tm_mday, tm->tm_mon + 1,
		1900 + tm->tm_year, tm->tm_hour, tm->tm_min);

  temp = motd;
  while (temp)
    {
      sendto_one (source_p, rpl_str (RPL_MOTD), me.name, parv[0], temp->line);
      temp = temp->next;
    }
  sendto_one (source_p, rpl_str (RPL_ENDOFMOTD), me.name, parv[0]);
  return 0;
}

/*
 * read_motd() - From CoMSTuD, added Aug 29, 1996
 */
void
read_motd (char *filename)
{
  aMotd *temp, *last;
  struct stat sb;
  char buffer[MOTDLINELEN], *tmp;
  int fd;

  /* Clear out the old MOTD */

  while (motd)
    {
      temp = motd->next;
      MyFree (motd);
      motd = temp;
    }
  fd = open (filename, O_RDONLY);
  if (fd == -1)
    return;
  fstat (fd, &sb);
  motd_tm = localtime (&sb.st_mtime);
  last = (aMotd *) NULL;

  while (dgets (fd, buffer, MOTDLINELEN - 1) > 0)
    {
      if ((tmp = (char *) strchr (buffer, '\n')))
	*tmp = '\0';
      if ((tmp = (char *) strchr (buffer, '\r')))
	*tmp = '\0';
      temp = (aMotd *) MyMalloc (sizeof (aMotd));

      strncpyzt (temp->line, buffer, MOTDLINELEN);
      temp->next = (aMotd *) NULL;
      if (!motd)
	motd = temp;
      else
	last->next = temp;
      last = temp;
    }
  close (fd);

  if (motd_tm)
    (void) sprintf (motd_last_changed_date,
		    "%d/%d/%d %d:%02d",
		    motd_tm->tm_mday,
		    motd_tm->tm_mon + 1,
		    1900 + motd_tm->tm_year,
		    motd_tm->tm_hour, motd_tm->tm_min);
}


void
read_shortmotd (char *filename)
{
  aMotd *temp, *last;
  char buffer[MOTDLINELEN], *tmp;
  int fd;

  /* Clear out the old MOTD */

  while (shortmotd)
    {
      temp = shortmotd->next;
      MyFree (shortmotd);
      shortmotd = temp;
    }
  if (SHORT_MOTD == 0)
    return;
  fd = open (filename, O_RDONLY);
  if (fd == -1)
    return;

  last = (aMotd *) NULL;

  while (dgets (fd, buffer, MOTDLINELEN - 1) > 0)
    {
      if ((tmp = (char *) strchr (buffer, '\n')))
	*tmp = '\0';
      if ((tmp = (char *) strchr (buffer, '\r')))
	*tmp = '\0';
      temp = (aMotd *) MyMalloc (sizeof (aMotd));

      strncpyzt (temp->line, buffer, MOTDLINELEN);
      temp->next = (aMotd *) NULL;
      if (!shortmotd)
	shortmotd = temp;
      else
	last->next = temp;
      last = temp;
    }
  close (fd);
}

/*
 * * m_opermotd - Based on the bahamut m_motd - ShadowMaster
 * 	 parv[0] = sender prefix 
 *       parv[1] = servername
 */
int
m_opermotd (aClient * client_p, aClient * source_p, int parc, char *parv[])
{

  if (!IsAnOper (source_p))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }
  if (hunt_server (client_p, source_p, ":%s OPERMOTD :%s", 1, parc, parv) !=
      HUNTED_ISME)
    return 0;

  sendto_snomask (SNOMASK_SPY, "OPERMOTD requested by %s (%s@%s) [%s]",
		      source_p->name, source_p->user->username,
		      source_p->user->host, source_p->user->server);
  send_opermotd (client_p, source_p, parc, parv);
  return 0;
}

/*
 ** send_opermotd - Based on the bahamut send_motd - ShadowMaster
 **  parv[0] = sender prefix
 **  parv[1] = servername
*/
int
send_opermotd (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aOMotd *temp;
  struct tm *tm;

  tm = omotd_tm;
  if (omotd == (aOMotd *) NULL)
    {
      sendto_one (source_p, err_str (ERR_NOOPERMOTD), me.name, parv[0]);
      return 0;
    }
  sendto_one (source_p, rpl_str (RPL_OPERMOTDSTART), me.name, parv[0],
	      me.name);

  if (tm)
    sendto_one (source_p,
		":%s %d %s :- %d/%d/%d %d:%02d", me.name, RPL_OPERMOTD,
		parv[0], tm->tm_mday, tm->tm_mon + 1,
		1900 + tm->tm_year, tm->tm_hour, tm->tm_min);

  temp = omotd;
  while (temp)
    {
      sendto_one (source_p, rpl_str (RPL_OPERMOTD), me.name, parv[0],
		  temp->line);
      temp = temp->next;
    }
  sendto_one (source_p, rpl_str (RPL_ENDOFOPERMOTD), me.name, parv[0]);
  return 0;
}

/*
 * read_opermotd() - Based on CoMSTuD's read_motd - ShadowMaster
 */
void
read_opermotd (char *filename)
{
  aOMotd *temp, *last;
  struct stat sb;
  char buffer[MOTDLINELEN], *tmp;
  int fd;

  /* Clear out the old OPERMOTD */

  while (omotd)
    {
      temp = omotd->next;
      MyFree (omotd);
      omotd = temp;
    }
  fd = open (filename, O_RDONLY);
  if (fd == -1)
    return;
  fstat (fd, &sb);
  omotd_tm = localtime (&sb.st_mtime);
  last = (aOMotd *) NULL;

  while (dgets (fd, buffer, MOTDLINELEN - 1) > 0)
    {
      if ((tmp = (char *) strchr (buffer, '\n')))
	*tmp = '\0';
      if ((tmp = (char *) strchr (buffer, '\r')))
	*tmp = '\0';
      temp = (aOMotd *) MyMalloc (sizeof (aOMotd));

      strncpyzt (temp->line, buffer, MOTDLINELEN);
      temp->next = (aOMotd *) NULL;
      if (!omotd)
	omotd = temp;
      else
	last->next = temp;
      last = temp;
    }
  close (fd);

  if (omotd_tm)
    (void) sprintf (opermotd_last_changed_date,
		    "%d/%d/%d %d:%02d",
		    omotd_tm->tm_mday,
		    omotd_tm->tm_mon + 1,
		    1900 + omotd_tm->tm_year,
		    omotd_tm->tm_hour, omotd_tm->tm_min);
}

/*
 * read_help() - modified from from CoMSTuD's read_motd added Aug 29,
 * 1996 modifed  Aug 31 1997 - Dianora
 * 
 * Use the same idea for the oper helpfile
 */
void
read_help (char *filename)
{
  aMotd *temp, *last;
  char buffer[MOTDLINELEN], *tmp;
  int fd;

  /* Clear out the old HELPFILE */

  while (helpfile)
    {
      temp = helpfile->next;
      MyFree (helpfile);
      helpfile = temp;
    }

  fd = open (filename, O_RDONLY);
  if (fd == -1)
    return;

  last = (aMotd *) NULL;

  while (dgets (fd, buffer, MOTDLINELEN - 1) > 0)
    {
      if ((tmp = (char *) strchr (buffer, '\n')))
	*tmp = '\0';
      if ((tmp = (char *) strchr (buffer, '\r')))
	*tmp = '\0';
      temp = (aMotd *) MyMalloc (sizeof (aMotd));

      strncpyzt (temp->line, buffer, MOTDLINELEN);
      temp->next = (aMotd *) NULL;
      if (!helpfile)
	helpfile = temp;
      else
	last->next = temp;
      last = temp;
    }
  close (fd);
}

/*
 * * m_close - added by Darren Reed Jul 13 1992.
 */
int
m_close (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aClient *target_p;
  int i;
  int closed = 0;

  if (!MyOper (source_p))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }

  for (i = highest_fd; i; i--)
    {
      if (!(target_p = local[i]))
	continue;
      if (!IsUnknown (target_p) && !IsConnecting (target_p)
	  && !IsHandshake (target_p))
	continue;
      sendto_one (source_p, rpl_str (RPL_CLOSING), me.name, parv[0],
		  get_client_name (target_p, TRUE), target_p->status);
      (void) exit_client (target_p, target_p, target_p, "Oper Closing");
      closed++;
    }
  sendto_one (source_p, rpl_str (RPL_CLOSEEND), me.name, parv[0], closed);
  return 0;
}

int
m_die (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aClient *target_p;
  int i;
  char *pass = NULL;

  if (!(source_p->oflag & OFLAG_DIE))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }
  /* X line -mjs */

  if ((pass = (char *) find_diepass ()))
    {
      if (parc < 2)
	{
	  sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS), me.name,
		      parv[0], "DIE");
	  return 0;
	}
      if (strcmp (pass, parv[1]))
	{
	  sendto_one (source_p, err_str (ERR_PASSWDMISMATCH), me.name,
		      parv[0]);
	  return 0;
	}
    }

  for (i = 0; i <= highest_fd; i++)
    {
      if (!(target_p = local[i]))
	continue;
      if (IsClient (target_p))
	sendto_one (target_p,
		    ":%s NOTICE %s :Server Terminating. %s",
		    me.name, target_p->name, get_client_name (source_p,
							      FALSE));
      else if (IsServer (target_p))
	sendto_one (target_p, ":%s ERROR :Terminated by %s",
		    me.name, get_client_name (source_p, TRUE));
    }
  (void) s_die ();
  return 0;
}

int m_capab(aClient *client_p, aClient *source_p,
                    int parc, char *parv[])
{
  struct Capability *cap;
  int i;
  char* p;
  char* s;

  for (i=1; i<parc; i++)
  {
    for (s = strtoken(&p, parv[i], " "); s; s = strtoken(&p, NULL, " "))
    {
        for (cap = captab; cap->name; cap++)
        {
          if (!irccmp(cap->name, s))
          {
            client_p->capabilities |= cap->cap;
            break;
          }
        }
    } /* for */
  } /* for */
  return 0;
}

/*
 * Shadowfax's LOCKFILE code
 */
#ifdef LOCKFILE

int
lock_kline_file ()
{
  int fd;

  /* Create Lockfile */

  if ((fd = open (LOCKFILE, O_WRONLY | O_CREAT | O_EXCL, 0666)) < 0)
    {
      sendto_realops ("%s is locked, klines pending", klinefile);
      pending_kline_time = time (NULL);
      return (-1);
    }
  (void) close (fd);
  return (1);
}

void
do_pending_klines ()
{
  int fd;
  char s[20];
  struct pkl *k, *ok;

  if (!pending_klines)
    return;

  /* Create Lockfile */
  if ((fd = open (LOCKFILE, O_WRONLY | O_CREAT | O_EXCL, 0666)) < 0)
    {
      sendto_realops ("%s is locked, klines pending", klinefile);
      pending_kline_time = time (NULL);
      return;
    }
  (void) ircsprintf (s, "%d\n", getpid ());
  (void) write (fd, s, strlen (s));
  close (fd);

  /* Open klinefile */
  if ((fd = open (klinefile, O_WRONLY | O_APPEND)) == -1)
    {
      sendto_realops
	("Pending klines cannot be written, cannot open %s", klinefile);
      unlink (LOCKFILE);
      return;
    }

  /* Add the Pending Klines */

  k = pending_klines;
  while (k)
    {
      write (fd, k->comment, strlen (k->comment));
      write (fd, k->kline, strlen (k->kline));
      free (k->comment);
      free (k->kline);
      ok = k;
      k = k->next;
      free (ok);
    }
  pending_klines = NULL;
  pending_kline_time = 0;

  close (fd);

  /* Delete the Lockfile */
  unlink (LOCKFILE);
}
#endif

/* m_svskill - Just about the same as outta df
 *  - Raistlin
 * parv[0] = servername
 * parv[1] = client
 * parv[2] = nick stamp
 * parv[3] = kill message
 */

int
m_svskill (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aClient *target_p;
  char *comment;
  char reason[TOPICLEN + 1];
  ts_val ts = 0;

  if (parc > 3)
    {
      comment = parv[3] ? parv[3] : parv[0];
      ts = atol (parv[2]);
    }
  else
    comment = (parc > 2 && parv[2]) ? parv[2] : parv[0];

  if (!IsULine (source_p))
    return -1;
  if ((target_p = find_client (parv[1], NULL))
      && (!ts || ts == target_p->tsinfo))
    {
      if (MyClient (target_p))
	{
	  strcpy (reason, "SVSKilled: ");
	  strncpy (reason + 11, comment, TOPICLEN - 11);
	  reason[TOPICLEN] = '\0';
	  exit_client (target_p, target_p, source_p, reason);
	  return (target_p == client_p) ? FLUSH_BUFFER : 0;
	}

      if (target_p->from == client_p)
	{
	  sendto_snomask (SNOMASK_DEBUG,
			      "Received wrong-direction SVSKILL for %s (behind %s) from %s",
			      target_p->name, client_p->name,
			      get_client_name (source_p, HIDEME));
	  return 0;
	}
      else if (ts == 0)
	sendto_one (target_p->from, ":%s SVSKILL %s :%s", parv[0], parv[1],
		    comment);
      else
	sendto_one (target_p->from, ":%s SVSKILL %s %ld :%s", parv[0],
		    parv[1], ts, comment);
    }
  return 0;
}

/*
 * RPL_NOWON   - Online at the moment (Succesfully added to WATCH-list)
 * RPL_NOWOFF  - Offline at the moement (Succesfully added to WATCH-list)
 * RPL_WATCHOFF   - Succesfully removed from WATCH-list.
 * ERR_TOOMANYWATCH - Take a guess :>  Too many WATCH entries.
 */
static void
show_watch (aClient * client_p, char *name, int rpl1, int rpl2)
{
  aClient *target_p;

  if ((target_p = find_person (name, NULL)))
    sendto_one (client_p, rpl_str (rpl1), me.name, client_p->name,
		target_p->name, target_p->user->username,
		IsHidden (target_p) ? target_p->user->virthost : target_p->
		user->host, target_p->lasttime);
  else
    sendto_one (client_p, rpl_str (rpl2), me.name, client_p->name, name, "*",
		"*", 0);
}

/*
 * m_watch
 */
int
m_watch (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aClient *target_p;
  char *s, *p, *user;
  char def[2] = "l";

  if (parc < 2)
    {
      /* Default to 'l' - list who's currently online */
      parc = 2;
      parv[1] = def;
    }

  for (p = NULL, s = strtoken (&p, parv[1], ", "); s;
       s = strtoken (&p, NULL, ", "))
    {
      if ((user = (char *) strchr (s, '!')))
	*user++ = '\0';		/* Not used */

      /*
       * Prefix of "+", they want to add a name to their WATCH
       * list. 
       */
      if (*s == '+')
	{
	  if (*(s + 1))
	    {
	      if (source_p->watches >= MAXWATCH)
		{
		  sendto_one (source_p, err_str (ERR_TOOMANYWATCH),
			      me.name, client_p->name, s + 1);
		  continue;
		}
	      add_to_watch_hash_table (s + 1, source_p);
	    }
	  show_watch (source_p, s + 1, RPL_NOWON, RPL_NOWOFF);
	  continue;
	}

      /*
       * Prefix of "-", coward wants to remove somebody from their
       * WATCH list.  So do it. :-)
       */
      if (*s == '-')
	{
	  del_from_watch_hash_table (s + 1, source_p);
	  show_watch (source_p, s + 1, RPL_WATCHOFF, RPL_WATCHOFF);
	  continue;
	}

      /*
       * Fancy "C" or "c", they want to nuke their WATCH list and start
       * over, so be it.
       */
      if (*s == 'C' || *s == 'c')
	{
	  hash_del_watch_list (source_p);
	  continue;
	}

      /*
       * Now comes the fun stuff, "S" or "s" returns a status report of
       * their WATCH list.  I imagine this could be CPU intensive if its
       * done alot, perhaps an auto-lag on this?
       */
      if (*s == 'S' || *s == 's')
	{
	  Link *lp;
	  aWatch *anptr;
	  int count = 0;

	  /*
	   * Send a list of how many users they have on their WATCH list
	   * and how many WATCH lists they are on.
	   */
	  anptr = hash_get_watch (source_p->name);
	  if (anptr)
	    for (lp = anptr->watch, count = 1; (lp = lp->next); count++);
	  sendto_one (source_p, rpl_str (RPL_WATCHSTAT), me.name, parv[0],
		      source_p->watches, count);

	  /*
	   * Send a list of everybody in their WATCH list. Be careful
	   * not to buffer overflow.
	   */
	  if ((lp = source_p->watch) == NULL)
	    {
	      sendto_one (source_p, rpl_str (RPL_ENDOFWATCHLIST), me.name,
			  parv[0], *s);
	      continue;
	    }
	  *buf = '\0';
	  strcpy (buf, lp->value.wptr->nick);
	  count = strlen (parv[0]) + strlen (me.name) + 10 + strlen (buf);
	  while ((lp = lp->next))
	    {
	      if (count + strlen (lp->value.wptr->nick) + 1 > BUFSIZE - 2)
		{
		  sendto_one (source_p, rpl_str (RPL_WATCHLIST), me.name,
			      parv[0], buf);
		  *buf = '\0';
		  count = strlen (parv[0]) + strlen (me.name) + 10;
		}
	      strcat (buf, " ");
	      strcat (buf, lp->value.wptr->nick);
	      count += (strlen (lp->value.wptr->nick) + 1);
	    }
	  sendto_one (source_p, rpl_str (RPL_WATCHLIST), me.name, parv[0],
		      buf);
	  sendto_one (source_p, rpl_str (RPL_ENDOFWATCHLIST), me.name,
		      parv[0], *s);
	  continue;
	}

      /*
       * Well that was fun, NOT.  Now they want a list of everybody in
       * their WATCH list AND if they are online or offline? Sheesh,
       * greedy arn't we?
       */
      if (*s == 'L' || *s == 'l')
	{
	  Link *lp = source_p->watch;

	  while (lp)
	    {
	      if ((target_p = find_person (lp->value.wptr->nick, NULL)))
		sendto_one (source_p, rpl_str (RPL_NOWON), me.name, parv[0],
			    target_p->name, target_p->user->username,
			    IsHidden (target_p) ? target_p->user->
			    virthost : target_p->user->host,
			    target_p->tsinfo);
	      /*
	       * But actually, only show them offline if its a capital
	       * 'L' (full list wanted).
	       */
	      else if (isupper (*s))
		sendto_one (source_p, rpl_str (RPL_NOWOFF), me.name,
			    parv[0], lp->value.wptr->nick, "*", "*",
			    lp->value.wptr->lasttime);
	      lp = lp->next;
	    }

	  sendto_one (source_p, rpl_str (RPL_ENDOFWATCHLIST), me.name,
		      parv[0], *s);
	  continue;
	}
      /* Hmm.. unknown prefix character.. Ignore it. :-) */
    }

  return 0;
}

int
m_sqline (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aConfItem *aconf;

  if (!(IsServer (source_p) || IsULine (source_p)))
    return 0;

  if (parc < 2)
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS), me.name, parv[0],
		  "SQLINE");
      return 0;
    }

  /* get rid of redundancies */
  parv[1] = collapse (parv[1]);

  /* if we have any Q:lines (SQ or Q) that match
   * this Q:line, just return (no need to waste cpu */

  if (!
      (aconf =
       find_conf_name (parv[1],
		       *parv[1] ==
		       '#' ? CONF_QUARANTINED_CHAN : CONF_QUARANTINED_NICK)))
    {
      /* okay, it doesn't suck, build a new conf for it */
      aconf = make_conf ();

      /* Q:line and SQline, woo */
      if (*parv[1] == '#')
	aconf->status = (CONF_QUARANTINED_CHAN | CONF_SQLINE);
      else
	aconf->status = (CONF_QUARANTINED_NICK | CONF_SQLINE);
      DupString (aconf->name, parv[1]);
      DupString (aconf->passwd, (parv[2] != NULL ? parv[2] : "Reserved"));
      aconf->next = conf;
      conf = aconf;
    }
  sendto_serv_butone (client_p, ":%s SQLINE %s :%s", source_p->name, parv[1],
		      aconf->passwd);
  return 0;
}

int
m_unsqline (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  int matchit = 0;
  char *mask;
  aConfItem *aconf, *ac2, *ac3;

  if (!(IsServer (source_p) || IsULine (source_p)))
    return 0;

  if (parc < 2)
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS), me.name, parv[0],
		  "UNSQLINE");
      return 0;
    }

  if (parc == 3)
    {
      matchit = atoi (parv[1]);
      mask = parv[2];
    }
  else
    mask = parv[1];

  /* find the sqline and remove it */
  /* this was way too complicated and ugly. Fixed. -lucas */
  /* Changed this to use match.  Seems to make more sense? */
  /* Therefore unsqline * will clear the sqline list. */

  ac2 = ac3 = aconf = conf;
  while (aconf)
    {
      if ((aconf->status & (CONF_QUARANTINE))
	  && (aconf->status & (CONF_SQLINE)) &&
	  aconf->name && (matchit
			  ? !match (mask, aconf->name)
			  : (irccmp (mask, aconf->name) == 0)))
	{
	  if (conf == aconf)
	    ac2 = ac3 = conf = aconf->next;
	  else
	    ac2 = ac3->next = aconf->next;
	  MyFree (aconf->passwd);
	  MyFree (aconf->name);
	  MyFree (aconf);
	  aconf = ac2;
	}
      else
	{
	  ac3 = aconf;
	  aconf = aconf->next;
	}
    }

  if (parc == 3)
    sendto_serv_butone (client_p, ":%s UNSQLINE %d :%s", source_p->name,
			matchit, mask);
  else
    sendto_serv_butone (client_p, ":%s UNSQLINE :%s", source_p->name, mask);
  return 0;
}

int
m_sgline (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aClient *target_p;
  aConfItem *aconf;
  int len, i;
  char *mask, *reason, fbuf[512];

  if (!(IsServer (source_p) || IsULine (source_p)))
    return 0;

  if (parc < 3)
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS), me.name, parv[0],
		  "SGLINE");
      return 0;
    }

  len = atoi (parv[1]);
  mask = parv[2];
  if ((strlen (mask) > len) && (mask[len]) == ':')
    {
      mask[len] = '\0';
      reason = mask + len + 1;
    }
  else
    {				/* Bogus */
      return 0;
    }

  /* if we have any G:lines (SG or G) that match
   * this G:line, just return (no need to waste cpu */

  if (!(aconf = find_conf_name (mask, CONF_GCOS)))
    {
      /* okay, it doesn't suck, build a new conf for it */
      aconf = make_conf ();

      aconf->status = (CONF_GCOS | CONF_SGLINE);	/* G:line and SGline, woo */
      DupString (aconf->name, mask);
      DupString (aconf->passwd, reason ? reason : "Reason Unspecified");
      aconf->next = conf;
      conf = aconf;

      /* Check local users against it */
      for (i = 0; i <= highest_fd; i++)
	{
	  if (!(target_p = local[i]) || IsMe (target_p) || IsLog (target_p))
	    continue;
	  if (IsPerson (target_p))
	    {
	      if ((target_p->hostip) && (!match (mask, target_p->info)))
		{
		  if (find_eline (target_p))
		    {
		      sendto_realops ("E-Line active for %s",
				      get_client_name (target_p, FALSE));
		    }
		  else
		    {
		      sendto_ops ("SGline active for %s",
				  get_client_name (target_p, HIDEME));
		      ircsprintf (fbuf, "SGlined: %s", reason);
		      (void) exit_client (target_p, target_p, &me, fbuf);
		      i--;
		    }
		}
	    }
	}
    }
  sendto_serv_butone (client_p, ":%s SGLINE %d :%s:%s", source_p->name, len,
		      aconf->name, aconf->passwd);
  return 0;
}

int
m_unsgline (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  int matchit = 0;
  char *mask;
  aConfItem *aconf, *ac2, *ac3;

  if (!(IsServer (source_p) || IsULine (source_p)))
    return 0;

  if (parc < 2)
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS), me.name, parv[0],
		  "UNSGLINE");
      return 0;
    }


  if (parc == 3)
    {
      matchit = atoi (parv[1]);
      mask = parv[2];
    }
  else
    mask = parv[1];

  /* find the sgline and remove it */
  /* this was way too complicated and ugly. Fixed. -lucas */
  /* Changed this to use match.  Seems to make more sense? */
  /* Therefore unsgline * will clear the sgline list. */
  ac2 = ac3 = aconf = conf;
  while (aconf)
    {
      if ((aconf->status & (CONF_GCOS))
	  && (aconf->status & (CONF_SGLINE)) &&
	  aconf->name && (matchit
			  ? !match (mask, aconf->name)
			  : (irccmp (mask, aconf->name) == 0)))
	{
	  if (conf == aconf)
	    ac2 = ac3 = conf = aconf->next;
	  else
	    ac2 = ac3->next = aconf->next;
	  MyFree (aconf->passwd);
	  MyFree (aconf->name);
	  MyFree (aconf);
	  aconf = ac2;
	}
      else
	{
	  ac3 = aconf;
	  aconf = aconf->next;
	}
    }

  if (parc == 3)
    sendto_serv_butone (client_p, ":%s UNSGLINE %d :%s", source_p->name,
			matchit, mask);
  else
    sendto_serv_butone (client_p, ":%s UNSGLINE :%s", source_p->name, mask);
  return 0;
}

/*
 * dump_map (used by m_map) from StarChat IRCd, minor additions by ShadowMaster
 */
void
dump_map (aClient * client_p, aClient * server, char *mask,
	  int prompt_length, int length)
{
  static char prompt[64];
  char *p = &prompt[prompt_length];
  int cnt = 0, local = 0;
  aClient *target_p;

  *p = '\0';

  if (prompt_length > 60)
    sendto_one (client_p, rpl_str (RPL_MAPMORE), me.name, client_p->name,
		prompt, server->name);
  else
    {
      for (target_p = client; target_p; target_p = target_p->next)
	{
	  if (IsPerson (target_p) && !IsULine (target_p))
	    {			/* We dont count ULined clients towards the network total - ShadowMaster */
	      ++cnt;
	      if (irccmp (target_p->user->server, server->name) == 0)
		++local;
	    }
	}

      sendto_one (client_p, rpl_str (RPL_MAP), me.name, client_p->name,
		  prompt, length, server->name, local, (local * 100) / cnt);
      cnt = 0;
    }

  if (prompt_length > 0)
    {
      p[-1] = ' ';
      if (p[-2] == '`')
	p[-2] = ' ';
    }
  if (prompt_length > 60)
    return;

  strcpy (p, "|-");


  for (target_p = client; target_p; target_p = target_p->next)
    {
      if (!IsServer (target_p)
	  || (irccmp (target_p->serv->up, server->name) != 0))
	continue;

      if (HIDEULINEDSERVS == 1)
	{
	  if (!IsAnOper (client_p) && IsULine (target_p))
	    continue;
	}

      if (match (mask, target_p->name))
	target_p->flags &= ~FLAGS_MAP;
      else
	{
	  target_p->flags |= FLAGS_MAP;
	  cnt++;
	}
    }

  for (target_p = client; target_p; target_p = target_p->next)
    {
      if (!(target_p->flags & FLAGS_MAP) ||	/* != */
	  !IsServer (target_p)
	  || (irccmp (target_p->serv->up, server->name) != 0))
	continue;

      if (HIDEULINEDSERVS == 1)
	{
	  if (!IsAnOper (client_p) && IsULine (target_p))
	    continue;
	}

      if (--cnt == 0)
	*p = '`';
      dump_map (client_p, target_p, mask, prompt_length + 2, length - 2);
    }

  if (prompt_length > 0)
    p[-1] = '-';
}

/*
* m_map from StarChat IRCd.
*
*      parv[0] = sender prefix
*      parv[1] = server mask
*/
int
m_map (aClient * client_p, aClient * source_p, int parc, char *parv[])
{

  aClient *target_p;
  int longest = strlen (me.name);

  if (check_registered (source_p))
    return 0;

  sendto_snomask (SNOMASK_SPY, "MAP requested by %s (%s@%s) [%s]",
		      source_p->name, source_p->user->username,
		      source_p->user->host, source_p->user->server);

  if (parc < 2)
    parv[1] = "*";

  for (target_p = client; target_p; target_p = target_p->next)
    if (IsServer (target_p)
	&& (strlen (target_p->name) + target_p->hopcount * 2) > longest)
      longest = strlen (target_p->name) + target_p->hopcount * 2;

  if (longest > 60)
    longest = 60;
  longest += 2;
  dump_map (source_p, &me, parv[1], 0, longest);


  sendto_one (source_p, rpl_str (RPL_MAPEND), me.name, parv[0]);

  return 0;
}


#define DKEY_GOTIN  0x01
#define DKEY_GOTOUT 0x02

#define DKEY_DONE(x) (((x) & (DKEY_GOTIN|DKEY_GOTOUT)) == (DKEY_GOTIN|DKEY_GOTOUT))

int
m_dkey (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  if (!(IsNegoServer (source_p) && parc > 1))
    {
      if (IsPerson (source_p))
	return 0;
      return exit_client (source_p, source_p, source_p,
			  "Not negotiating now");
    }
#ifdef HAVE_ENCRYPTION_ON
  if (irccmp (parv[1], "START") == 0)
    {
      char keybuf[1024];

      if (parc != 2)
	return exit_client (source_p, source_p, source_p,
			    "DKEY START failure");

      if (source_p->serv->sessioninfo_in != NULL
	  && source_p->serv->sessioninfo_out != NULL)
	return exit_client (source_p, source_p, source_p,
			    "DKEY START duplicate?!");

      source_p->serv->sessioninfo_in = dh_start_session ();
      source_p->serv->sessioninfo_out = dh_start_session ();

      sendto_snomask
	(SNOMASK_NETINFO, "from %s: Initiating diffie-hellman key exchange with %s", me.name,
	 source_p->name);

      dh_get_s_public (keybuf, 1024, source_p->serv->sessioninfo_in);
      sendto_one (source_p, "DKEY PUB I %s", keybuf);

      dh_get_s_public (keybuf, 1024, source_p->serv->sessioninfo_out);
      sendto_one (source_p, "DKEY PUB O %s", keybuf);
      return 0;
    }

  if (irccmp (parv[1], "PUB") == 0)
    {
      char keybuf[1024];
      int keylen;

      if (parc != 4 || !source_p->serv->sessioninfo_in
	  || !source_p->serv->sessioninfo_out)
	return exit_client (source_p, source_p, source_p, "DKEY PUB failure");

      if (irccmp (parv[2], "O") == 0)	/* their out is my in! */
	{
	  if (!dh_generate_shared (source_p->serv->sessioninfo_in, parv[3]))
	    return exit_client (source_p, source_p, source_p,
				"DKEY PUB O invalid");
	  source_p->serv->dkey_flags |= DKEY_GOTOUT;
	}
      else if (irccmp (parv[2], "I") == 0)	/* their out is my in! */
	{
	  if (!dh_generate_shared (source_p->serv->sessioninfo_out, parv[3]))
	    return exit_client (source_p, source_p, source_p,
				"DKEY PUB I invalid");
	  source_p->serv->dkey_flags |= DKEY_GOTIN;
	}
      else
	return exit_client (source_p, source_p, source_p,
			    "DKEY PUB bad option");

      if (DKEY_DONE (source_p->serv->dkey_flags))
	{
	  sendto_one (source_p, "DKEY DONE");
	  SetRC4OUT (source_p);

	  keylen = 1024;

	  if (!dh_get_s_shared
	      (keybuf, &keylen, source_p->serv->sessioninfo_in))
	    return exit_client (source_p, source_p, source_p,
				"Could not setup encrypted session");

	  source_p->serv->rc4_in = rc4_initstate (keybuf, keylen);

	  keylen = 1024;

	  if (!dh_get_s_shared
	      (keybuf, &keylen, source_p->serv->sessioninfo_out))
	    return exit_client (source_p, source_p, source_p,
				"Could not setup encrypted session");

	  source_p->serv->rc4_out = rc4_initstate (keybuf, keylen);

	  dh_end_session (source_p->serv->sessioninfo_in);
	  dh_end_session (source_p->serv->sessioninfo_out);

	  source_p->serv->sessioninfo_in = source_p->serv->sessioninfo_out =
	    NULL;
	  return 0;
	}

      return 0;
    }

  if (irccmp (parv[1], "DONE") == 0)
    {
      if (!
	  ((source_p->serv->sessioninfo_in == NULL
	    && source_p->serv->sessioninfo_out == NULL)
	   && (source_p->serv->rc4_in != NULL
	       && source_p->serv->rc4_out != NULL)))
	return exit_client (source_p, source_p, source_p,
			    "DKEY DONE when not done!");
      SetRC4IN (source_p);
      sendto_snomask
	(SNOMASK_NETINFO, "from %s: Diffie-Hellman exchange with %s complete, connection encrypted.",
	 me.name, source_p->name);
      sendto_one (source_p, "DKEY EXIT");
      return RC4_NEXT_BUFFER;
    }

  if (irccmp (parv[1], "EXIT") == 0)
    {
      if (!(IsRC4IN (source_p) && IsRC4OUT (source_p)))
	return exit_client (source_p, source_p, source_p,
			    "DKEY EXIT when not in proper stage");
      ClearNegoServer (source_p);
      return do_server_estab (source_p);
    }
#endif

  return 0;
}
