#include "struct.h"
#include "common.h"
#include "sys.h"
#include "numeric.h"
#include "msg.h"
#include "setup.h"
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pwd.h>
#include <signal.h>
#include <fcntl.h>
#ifdef PROFILING
#include <sys/gmon.h>
#define monstartup __monstartup
#endif
#include "inet.h"
#include "h.h"
#include "version.h"
#include "dh.h"
#include "dynconf.h"

#include "dich_conf.h"
#include "throttle.h"
#include "channel.h"
#include "userban.h"
#include "blalloc.h"

/*
** Allows for backtrace dumps to file on most glibc systems. Disabled by default - ShadowMaster
*/

#undef BACKTRACE_DEBUG

int booted = FALSE;

int IsRestarting;

aConfList EList1 = { 0, NULL };	/* ordered */
aConfList EList2 = { 0, NULL };	/* ordered, reversed */
aConfList EList3 = { 0, NULL };	/* what we can't sort */

aConfList FList1 = { 0, NULL };	/* ordered */
aConfList FList2 = { 0, NULL };	/* ordered, reversed */
aConfList FList3 = { 0, NULL };	/* what we can't sort */

aMotd *motd;
aRules *rules;
aOMotd *omotd;

aMotd *helpfile;		/* misnomer, aMotd could be generalized */

aMotd *shortmotd;		/* short motd */

int find_fline (aClient *);

struct tm *motd_tm;
struct tm *rules_tm;
struct tm *omotd_tm;

/* this stuff by mnystrom@mit.edu */
#include "fdlist.h"

fdlist serv_fdlist;
fdlist oper_fdlist;
fdlist listen_fdlist;

#ifndef NO_PRIORITY
fdlist busycli_fdlist;		/* high-priority clients */
#endif

extern void build_snomaskstr(void);

extern void check_expire_gline();

fdlist default_fdlist;		/* just the number of the entry */

int MAXCLIENTS = MAX_CLIENTS;	/* semi-configurable if
				 *  QUOTE_SET is def  */
struct Counter Count;
int R_do_dns, R_fin_dns, R_fin_dnsc, R_fail_dns, R_do_id, R_fin_id, R_fail_id;

time_t NOW;
time_t last_stat_save;
aClient me;			/* That's me */
aClient *client = &me;		/* Pointer to beginning of Client list */

float curSendK = 0, curRecvK = 0;

#ifdef  LOCKFILE
extern time_t pending_kline_time;
extern struct pkl *pending_klines;
extern void do_pending_klines (void);
#endif

void server_reboot ();
void restart (char *);
static void open_debugfile (), setup_signals ();
static void io_loop ();

/* externally needed functions */

extern void init_fdlist (fdlist *);	/* defined in fdlist.c */
extern void dbuf_init ();	/* defined in dbuf.c */
extern void read_motd (char *);	/* defined in s_serv.c */
extern void read_opermotd (char *);	/* defined in s_serv.c */
extern void read_shortmotd (char *);	/* defined in s_serv.c */
extern void read_help (char *);	/* defined in s_serv.c */

char **myargv;
int portnum = -1;		/* Server port number, listening this */
char *configfile = CONFIGFILE;	/* Server configuration file */
#ifdef KPATH
char *klinefile = KLINEFILE;	/* Server kline file */
# ifdef ZLINES_IN_KPATH
char *zlinefile = KLINEFILE;
# else
char *zlinefile = CONFIGFILE;
# endif
#else
char *klinefile = CONFIGFILE;
char *zlinefile = CONFIGFILE;
#endif

#ifdef VPATH
char *vlinefile = VLINEFILE;
#else
char *vlinefile = CONFIGFILE;
#endif

int debuglevel = -1;		/* Server debug level */
int bootopt = 0;		/* Server boot option flags */
char *debugmode = "";		/* -"-    -"-   -"-  */
void *sbrk0;			/* initial sbrk(0) */
static int dorehash = 0;
static char *dpath = DPATH;
int rehashed = 1;
int zline_in_progress = 0;	/* killing off matching D lines */
int noisy_htm = NOISY_HTM;	/* Is high traffic mode noisy or not? */
time_t nextconnect = 1;		/* time for next try_connections call */
time_t nextping = 1;		/* same as above for check_pings() */
time_t nextdnscheck = 0;	/* next time to poll dns to force timeout */
time_t nextexpire = 1;		/* next expire run on the dns cache */
time_t nextbanexpire = 1;	/* next time to expire the throttles/userbans */

#ifdef PROFILING
extern void _start, etext;

static int profiling_state = 1;
static int profiling_newmsg = 0;
static char profiling_msg[512];

VOIDSIG
s_dumpprof ()
{
  char buf[32];

  sprintf (buf, "gmon.%d", (int) time (NULL));
  setenv ("GMON_OUT_PREFIX", buf, 1);
  _mcleanup ();
  monstartup ((u_long) & _start, (u_long) & etext);
  setenv ("GMON_OUT_PREFIX", "gmon.auto", 1);
  sprintf (profiling_msg, "Reset profile, saved past profile data to %s",
	   buf);
  profiling_newmsg = 1;
}

VOIDSIG
s_toggleprof ()
{
  char buf[32];

  if (profiling_state == 1)
    {
      sprintf (buf, "gmon.%d", (int) time (NULL));
      setenv ("GMON_OUT_PREFIX", buf, 1);
      _mcleanup ();
      sprintf (profiling_msg,
	       "Turned profiling OFF, saved profile data to %s", buf);
      profiling_state = 0;
    }
  else
    {
      __monstartup ((u_long) & _start, (u_long) & etext);
      setenv ("GMON_OUT_PREFIX", "gmon.auto", 1);
      profiling_state = 1;
      sprintf (profiling_msg, "Turned profiling ON");
    }
  profiling_newmsg = 1;
}
#endif

VOIDSIG
s_die ()
{
#ifdef SAVE_MAXCLIENT_STATS
  FILE *fp;
#endif
  dump_connections (me.fd);
#ifdef	USE_SYSLOG
  (void) syslog (LOG_CRIT, "Server killed By SIGTERM");
#endif
#ifdef SAVE_MAXCLIENT_STATS
  fp = fopen (DPATH "/.maxclients", "w");
  if (fp != NULL)
    {
      fprintf (fp, "%d %d %li %li %li %ld %ld %ld %ld", Count.max_loc,
	       Count.max_tot, Count.weekly, Count.monthly,
	       Count.yearly, Count.start, Count.week, Count.month,
	       Count.year);
      fclose (fp);
    }
#endif
  exit (0);
}

static VOIDSIG
s_rehash ()
{
#ifdef	POSIX_SIGNALS
  struct sigaction act;
#endif
  dorehash = 1;
#ifdef	POSIX_SIGNALS
  act.sa_handler = s_rehash;
  act.sa_flags = 0;
  (void) sigemptyset (&act.sa_mask);
  (void) sigaddset (&act.sa_mask, SIGHUP);
  (void) sigaction (SIGHUP, &act, NULL);
#else
  (void) signal (SIGHUP, s_rehash);	/* sysV -argv */
#endif
}

void
restart (char *mesg)
{
  static int was_here = NO;	/* redundant due to restarting flag below */
  if (was_here)
    abort ();
  was_here = YES;

#ifdef	USE_SYSLOG

  (void) syslog (LOG_WARNING,
		 "Restarting Server because: %s, sbrk(0)-etext: %d", mesg,
		 (void *) sbrk ((size_t) 0) - (void *) sbrk0);
#endif
  server_reboot ();
}

VOIDSIG
s_restart ()
{
  static int restarting = 0;

#ifdef	USE_SYSLOG
  (void) syslog (LOG_WARNING, "Server Restarting on SIGINT");
#endif
  if (restarting == 0)
    {
      /* Send (or attempt to) a dying scream to oper if present */
      restarting = 1;
      server_reboot ();
    }
}


void
server_reboot ()
{
  int i;
  sendto_ops ("Aieeeee!!!  Restarting server... sbrk(0)-etext: %d",
	      (void *) sbrk ((size_t) 0) - (void *) sbrk0);

  Debug ((DEBUG_NOTICE, "Restarting server..."));
  /*
   * fd 0 must be 'preserved' if either the -d or -i options have
   * been passed to us before restarting.
   */
#ifdef USE_SYSLOG
  (void) closelog ();
#endif
  for (i = 3; i < MAXCONNECTIONS; i++)
    (void) close (i);

  if (!(bootopt & (BOOT_TTY | BOOT_DEBUG)))
    (void) close (2);

  (void) close (1);

  if ((bootopt & BOOT_CONSOLE) || isatty (0))
    (void) close (0);

  if (!(bootopt & (BOOT_INETD | BOOT_OPER)))
    (void) execv (MYNAME, myargv);

#ifdef USE_SYSLOG
  /* Have to reopen since it has been closed above */
  openlog (myargv[0], LOG_PID | LOG_NDELAY, LOG_FACILITY);
  syslog (LOG_CRIT, "execv(%s,%s) failed: %m\n", MYNAME, myargv[0]);
  closelog ();
#endif

  Debug ((DEBUG_FATAL, "Couldn't restart server: %s", strerror (errno)));
  IsRestarting = 1;
}

/*
 * try_connections 
 *
 *      Scan through configuration and try new connections. 
 *   Returns  the calendar time when the next call to this
 *      function should be made latest. (No harm done if this 
 *      is called earlier or later...)
 */
static time_t
try_connections (time_t currenttime)
{
  aConfItem *aconf, **pconf, *con_conf = (aConfItem *) NULL;
  aClient *client_p;
  aClass *cltmp;
  int connecting, confrq, con_class = 0;
  time_t next = 0;

  connecting = FALSE;

  Debug ((DEBUG_NOTICE, "Connection check at   : %s", myctime (currenttime)));

  for (aconf = conf; aconf; aconf = aconf->next)
    {
      /* Also when already connecting! (update holdtimes) --SRB */
      if (!(aconf->status & CONF_CONNECT_SERVER) || aconf->port <= 0)
	continue;
      cltmp = Class (aconf);

      /*
       * * Skip this entry if the use of it is still on hold until 
       * future. Otherwise handle this entry (and set it on hold 
       * until next time). Will reset only hold times, if already 
       * made one successfull connection... [this algorithm is a bit
       * fuzzy... -- msa >;) ]
       */

      if ((aconf->hold > currenttime))
	{
	  if ((next > aconf->hold) || (next == 0))
	    next = aconf->hold;
	  continue;
	}

      confrq = get_con_freq (cltmp);
      aconf->hold = currenttime + confrq;

      /* Found a CONNECT config with port specified, scan clients 
       * and see if this server is already connected?
       */

      client_p = find_name (aconf->name, (aClient *) NULL);

      if (!client_p && (Links (cltmp) < MaxLinks (cltmp)) &&
	  (!connecting || (Class (cltmp) > con_class)))
	{
	  con_class = Class (cltmp);

	  con_conf = aconf;
	  /* We connect only one at time... */
	  connecting = TRUE;
	}

      if ((next > aconf->hold) || (next == 0))
	next = aconf->hold;
    }

  if (connecting)
    {
      if (con_conf->next)	/* are we already last? */
	{
	  for (pconf = &conf; (aconf = *pconf); pconf = &(aconf->next))
	    /*
	     * put the current one at the end and make sure we try all
	     * connections
	     */
	    if (aconf == con_conf)
	      *pconf = aconf->next;
	  (*pconf = con_conf)->next = 0;
	}
      if (connect_server (con_conf, (aClient *) NULL,
			  (struct hostent *) NULL) == 0)
	sendto_snomask (SNOMASK_NETINFO, "Connection to %s activated.", me.name,
			con_conf->name);
    }
  Debug ((DEBUG_NOTICE, "Next connection check : %s", myctime (next)));
  return (next);
}

/* dianora's code in the new checkpings is slightly wasteful.
 * however, upon inspection (thanks seddy), when we close a connection,
 * the ordering of local[i] is NOT reordered; simply local[highest_fd] becomes
 * local[i], so we can just i--;  - lucas
 */

static time_t
check_pings (time_t currenttime)
{
  aClient *client_p;
  int ping = 0, i;
  time_t oldest = 0;		/* timeout removed, see EXPLANATION below */
  char fbuf[512], *errtxt = "No response from %s, closing link";
  char ping_time_out_buffer[64];


  for (i = 0; i <= highest_fd; i++)
    {
      if (!(client_p = local[i]) || IsMe (client_p) || IsLog (client_p))
	continue;

      /* Note: No need to notify opers here. It's
       * already done when "FLAGS_DEADSOCKET" is set.
       */

      if (client_p->flags & FLAGS_DEADSOCKET)
	{
	  (void) exit_client (client_p, client_p, &me,
			      (client_p->
			       flags & FLAGS_SENDQEX) ? "SendQ exceeded" :
			      "Dead socket");
	  i--;
	  continue;
	}

      if (IsRegistered (client_p))
	ping = client_p->pingval;
      else
	ping = CONNECTTIMEOUT;

      /*
       * Ok, so goto's are ugly and can be avoided here but this code
       * is already indented enough so I think its justified. -avalon
       *
       * justified by what? laziness? <g>
       * If the client pingtime is fine (ie, not larger than the client ping)
       * skip over all the checks below. - lucas
       */

      if (ping < (currenttime - client_p->lasttime))
	{
	  /*
	   * If the server hasnt talked to us in 2*ping seconds and it has
	   * a ping time, then close its connection. If the client is a
	   * user and a KILL line was found to be active, close this
	   * connection too.
	   */
	  if (((client_p->flags & FLAGS_PINGSENT)
	       && ((currenttime - client_p->lasttime) >= (2 * ping)))
	      ||
	      ((!IsRegistered (client_p)
		&& (currenttime - client_p->since) >= ping)))
	    {
	      if (!IsRegistered (client_p)
		  && (DoingDNS (client_p) || DoingAuth (client_p)
		  ))
		{
		  if (client_p->authfd >= 0)
		    {
		      (void) close (client_p->authfd);
		      client_p->authfd = -1;
		      client_p->count = 0;
		      *client_p->buffer = '\0';
		    }
#ifdef SHOW_HEADERS
#ifdef USE_SSL
		  if (!IsSSL (client_p))
		    {
#endif
		      if (DoingDNS (client_p))
			sendto_one (client_p, REPORT_FAIL_DNS);
		      if (DoingAuth (client_p))
			sendto_one (client_p, REPORT_FAIL_ID);
#ifdef USE_SSL
		    }
#endif

#endif
		  Debug ((DEBUG_NOTICE, "DNS/AUTH timeout %s",
			  get_client_name (client_p, TRUE)));
		  del_queries ((char *) client_p);
		  ClearAuth (client_p);
		  ClearDNS (client_p);
		  SetAccess (client_p);
		  client_p->since = currenttime;
		  continue;
		}

	      if (IsServer (client_p) || IsConnecting (client_p)
		  || IsHandshake (client_p))
		{
		  ircsprintf (fbuf, "from %s: %s", me.name, errtxt);
		  sendto_snomask (SNOMASK_NETINFO, fbuf, get_client_name (client_p, HIDEME));
		  ircsprintf (fbuf, ":%s GNOTICE :%s", me.name, errtxt);
		  sendto_serv_butone (client_p, fbuf,
				      get_client_name (client_p, HIDEME));
		}

	      (void) ircsprintf (ping_time_out_buffer,
				 "Ping timeout: %d seconds",
				 currenttime - client_p->lasttime);

	      (void) exit_client (client_p, client_p, &me,
				  ping_time_out_buffer);
	      i--;		/* subtract out this fd so we check it again.. */
	      continue;
	    }			/* don't send pings during a burst, as we send them already. */

	  else if (!(client_p->flags & (FLAGS_PINGSENT | FLAGS_BURST)))
	    {
	      /*
	       * if we havent PINGed the connection and we havent heard from
	       * it in a while, PING it to make sure it is still alive.
	       */
	      client_p->flags |= FLAGS_PINGSENT;
	      /*
	       * not nice but does the job 
	       */
	      client_p->lasttime = currenttime - ping;
	      sendto_one (client_p, "PING :%s", me.name);
	    }
	}

      /* see EXPLANATION below

       * timeout = client_p->lasttime + ping;
       * while (timeout <= currenttime)
       *  timeout += ping;
       * if (timeout < oldest || !oldest)
       *   oldest = timeout;
       */

      /*
       * Check UNKNOWN connections - if they have been in this state
       * for > 100s, close them.
       */
      if (IsUnknown (client_p))
	if (client_p->
	    firsttime ? ((timeofday - client_p->firsttime) > 100) : 0)
	  (void) exit_client (client_p, client_p, &me,
			      "Connection Timed Out");
    }

  rehashed = 0;
  zline_in_progress = 0;

  /* EXPLANATION
   * on a server with a large volume of clients, at any given point
   * there may be a client which needs to be pinged the next second,
   * or even right away (a second may have passed while running
   * check_pings). Preserving CPU time is more important than
   * pinging clients out at exact times, IMO. Therefore, I am going to make
   * check_pings always return currenttime + 9. This means that it may take
   * a user up to 9 seconds more than pingfreq to timeout. Oh well.
   * Plus, the number is 9 to 'stagger' our check_pings calls out over
   * time, to avoid doing it and the other tasks ircd does at the same time
   * all the time (which are usually done on intervals of 5 seconds or so). 
   * - lucas
   *
   *  if (!oldest || oldest < currenttime)
   *     oldest = currenttime + PINGFREQUENCY;
   */

  oldest = currenttime + 9;

  Debug ((DEBUG_NOTICE, "Next check_ping() call at: %s, %d %d %d",
	  myctime (oldest), ping, oldest, currenttime));

  return oldest;
}

/*
 * * bad_command
 *    This is called when the commandline is not acceptable.
 *    Give error message and exit without starting anything.
 */
static int
bad_command ()
{

  (void)
    printf
    ("Usage: ircd %s[-h servername] [-p portnumber] [-x loglevel] [-s] [-t]\n",
#ifdef CMDLINE_CONFIG
     "[-f config] "
#else
     ""
#endif
    );
  (void) printf ("Server not started\n\n");
  return (-1);
}

#ifndef TRUE
#define TRUE 1
#endif

/*
 * code added by mika nystrom (mnystrom@mit.edu)
 * this flag is used to signal globally that the server is heavily
 * loaded, something which can be taken into account when processing
 * e.g. user commands and scheduling ping checks
 * Changed by Taner Halicioglu (taner@CERF.NET)
 */

#define LOADCFREQ 5		/* every 5s */
#define LOADRECV 40		/* 40k/s    */

int lifesux = 1;
int LRV = LOADRECV;
time_t LCF = LOADCFREQ;
int currlife = 0;
int HTMLOCK = NO;

char REPORT_DO_DNS[256], REPORT_FIN_DNS[256], REPORT_FIN_DNSC[256],
  REPORT_FAIL_DNS[256], REPORT_DO_ID[256], REPORT_FIN_ID[256],
  REPORT_FAIL_ID[256];

FILE *dumpfp = NULL;

int
main (int argc, char *argv[])
{
  uid_t uid, euid;
  int portarg = 0, fd;
#ifdef SAVE_MAXCLIENT_STATS
  FILE *mcsfp;
#endif
#ifdef USE_SSL
  extern int ssl_capable;
#endif

  /*
   * Initialize the Blockheap allocator
   */
  initBlockHeap ();

  build_version ();
  build_umodestr ();
  mode_sort (umodestring);
  build_cmodestr ();
  mode_sort (cmodestring);
  build_snomaskstr ();
  mode_sort (snomaskstring);

  Count.server = 1;		/* us */
  Count.oper = 0;
  Count.chan = 0;
  Count.local = 0;
  Count.total = 0;
  Count.invisi = 0;
  Count.unknown = 0;
  Count.max_loc = 0;
  Count.max_tot = 0;
  Count.today = 0;
  Count.weekly = 0;
  Count.monthly = 0;
  Count.yearly = 0;
  Count.start = NOW;
  Count.day = NOW;
  Count.week = NOW;
  Count.month = NOW;
  Count.year = NOW;

#ifdef SAVE_MAXCLIENT_STATS
  mcsfp = fopen (DPATH "/.maxclients", "r");
  if (mcsfp != NULL)
    {
      fscanf (mcsfp, "%d %d %li %li %li %ld %ld %ld %ld", &Count.max_loc,
	      &Count.max_tot, &Count.weekly, &Count.monthly, &Count.yearly,
	      &Count.start, &Count.week, &Count.month, &Count.year);
      fclose (mcsfp);
    }
#endif



  /*
   * this code by mika@cs.caltech.edu
   * it is intended to keep the ircd from being swapped out. BSD
   * swapping criteria do not match the requirements of ircd
   */

#ifdef INITIAL_DBUFS
  dbuf_init ();			/* set up some dbuf stuff to control paging */
#endif
  sbrk0 = sbrk ((size_t) 0);
  uid = getuid ();
  euid = geteuid ();
#ifdef PROFILING
  setenv ("GMON_OUT_PREFIX", "gmon.out", 1);
  (void) signal (SIGUSR1, s_dumpprof);
  (void) signal (SIGUSR2, s_toggleprof);
#endif
  myargv = argv;
  (void) umask (077);		/* better safe than sorry --SRB  */
  memset ((char *) &me, '\0', sizeof (me));

  setup_signals ();
  /*
   * * All command line parameters have the syntax "-fstring"  or "-f
   * string" (e.g. the space is optional). String may  be empty. Flag
   * characters cannot be concatenated (like "-fxyz"), it would
   * conflict with the form "-fstring".
   */
  while (--argc > 0 && (*++argv)[0] == '-')
    {
      char *p = argv[0] + 1;
      int flag = *p++;

      if (flag == '\0' || *p == '\0')
	{
	  if (argc > 1 && argv[1][0] != '-')
	    {
	      p = *++argv;
	      argc -= 1;
	    }
	  else
	    p = "";
	}

      switch (flag)
	{
	case 'a':
	  bootopt |= BOOT_AUTODIE;
	  break;
	case 'c':
	  bootopt |= BOOT_CONSOLE;
	  break;
	case 'q':
	  bootopt |= BOOT_QUICK;
	  break;
	case 'd':
	  (void) setuid ((uid_t) uid);
	  dpath = p;
	  break;
	case 'o':		/* Per user local daemon... */
	  (void) setuid ((uid_t) uid);
	  bootopt |= BOOT_OPER;
	  break;
#ifdef CMDLINE_CONFIG
	case 'f':
	  (void) setuid ((uid_t) uid);
	  configfile = p;
	  break;

# ifdef KPATH
	case 'k':
	  (void) setuid ((uid_t) uid);
	  klinefile = p;
	  break;
# endif

#endif
	case 'h':
	  strncpyzt (me.name, p, sizeof (me.name));
	  break;
	case 'i':
	  bootopt |= BOOT_INETD | BOOT_AUTODIE;
	  break;
	case 'p':
	  if ((portarg = atoi (p)) > 0)
	    portnum = portarg;
	  break;
	case 's':
	  bootopt |= BOOT_STDERR;
	  break;
	case 't':
	  (void) setuid ((uid_t) uid);
	  bootopt |= BOOT_TTY;
	  break;
	case 'v':
	  (void) printf ("ircd %s\n", version);
	  exit (0);
	case 'x':
#ifdef	DEBUGMODE
	  (void) setuid ((uid_t) uid);
	  debuglevel = atoi (p);
	  debugmode = *p ? p : "0";
	  bootopt |= BOOT_DEBUG;
	  break;
#else
	  break;
#endif
	default:
	  bad_command ();
	  break;
	}
    }

  if (argc > 0)
    return bad_command ();	/* This should exit out  */


#ifdef HAVE_ENCRYPTION_ON
  if (dh_init () == -1)
    return 0;
#endif

  if (chdir (dpath)) {
	fprintf(stderr, "Can't chdir() to data directory - %s. Exiting.", dpath);
	exit (-1);
  }

  (void) printf ("ircd %s\n", version);

  motd = (aMotd *) NULL;
  rules = (aRules *) NULL;
  omotd = (aOMotd *) NULL;
  helpfile = (aMotd *) NULL;
  motd_tm = NULL;
  rules_tm = NULL;
  omotd_tm = NULL;
  shortmotd = NULL;
  read_motd (MOTD);
  read_opermotd (OMOTD);
  read_help (HELPFILE);
  read_shortmotd (SHORTMOTD);

  clear_client_hash_table ();
  clear_channel_hash_table ();
  clear_scache_hash_table ();	/* server cache name table */
  clear_ip_hash_table ();	/* client host ip hash table */
  /* init the throttle system -wd */
  throttle_init ();
  init_userban ();
  initlists ();
  initclass ();
  initwhowas ();
  initstats ();
  booted = FALSE;

  init_dynconf ();
  load_conf (DCONF, 0);
  booted = TRUE;
  /* We have dynamic hubs... prevent hub's from starting in HTM */
  if (HUB == 1)
    {
      lifesux = 0;
    }
  /* Now we can buil the protoctl */
  cr_loadconf (IRCD_RESTRICT, 0);
  init_tree_parse (msgtab);
  init_send ();
  NOW = time (NULL);
  open_debugfile ();
  NOW = time (NULL);

  init_fdlist (&serv_fdlist);
  init_fdlist (&oper_fdlist);
  init_fdlist (&listen_fdlist);

#ifndef NO_PRIORITY
  init_fdlist (&busycli_fdlist);
#endif

  init_fdlist (&default_fdlist);
  {
    int i;

    for (i = MAXCONNECTIONS + 1; i > 0; i--)
      {
	default_fdlist.entry[i] = i - 1;
      }
  }

  if ((timeofday = time (NULL)) == -1)
    {
#ifdef USE_SYSLOG
      syslog (LOG_WARNING, "Clock Failure (%d), TS can be corrupted", errno);
#endif

      fprintf (stderr,
	       "§ Warning: Clock Failure (%d), TS can be corrupted!!\n",
	       errno);
      sendto_realops ("Clock Failure (%d), TS can be corrupted", errno);
    }



//      if (portnum < 0)
//        portnum = 4444;
  /*
   * We always have a portnumber in the M Line, so we take the default port from there always - ShadowMaster
   */
  me.port = portnum;

#ifdef USE_SSL
  ssl_capable = initssl ();
#endif

  /*
   * init_sys() will also launch the IRCd into the background so we wont be able to send anything to stderr and have
   * it seen by the user unless he is running it in console debugmode. So we want to do some checks here to be able to
   * tell the client about it - ShadowMaster
   */

  if ((fd = openconf (configfile)) == -1)
    {
      Debug ((DEBUG_FATAL, "Failed in reading configuration file %s",
	      configfile));
      exit (-1);
    }
#ifdef SEPARATE_QUOTE_KLINES_BY_DATE
  {
    struct tm *tmptr;
    char timebuffer[20], filename[200];

    tmptr = localtime (&NOW);
    (void) strftime (timebuffer, 20, "%y%m%d", tmptr);
    ircsprintf (filename, "%s.%s", klinefile, timebuffer);
    if ((fd = openconf (filename)) == -1)
      {
	Debug ((DEBUG_ERROR, "Failed reading kline file %s", filename));
      }
  }
#else
# ifdef KPATH
  if ((fd = openconf (klinefile)) == -1)
    {
      Debug ((DEBUG_ERROR, "Failed reading kline file %s", klinefile));
    }
# endif
#endif

# ifdef VPATH
  if ((fd = openconf (vlinefile)) == -1)
    {
      Debug ((DEBUG_ERROR, "Failed reading vline file %s", vlinefile));
    }
# endif

  (void) init_sys ();
  me.flags = FLAGS_LISTEN;
  if (bootopt & BOOT_INETD)
    {
      me.fd = 0;
      local[0] = &me;
      me.flags = FLAGS_LISTEN;
    }
  else
    me.fd = -1;

#ifdef USE_SYSLOG
# define SYSLOG_ME     "ircd"
  openlog (SYSLOG_ME, LOG_PID | LOG_NDELAY, LOG_FACILITY);
#endif


  if ((fd = openconf (configfile)) != -1)
    (void) initconf (bootopt, fd, NULL);

# ifdef KPATH
  if ((fd = openconf (klinefile)) != -1)
    (void) initconf (0, fd, NULL);
#endif

#ifdef VPATH
  if ((fd = openconf (vlinefile)) != -1)
    (void) initconf (0, fd, NULL);
#endif

  if (!(bootopt & BOOT_INETD))
    {
      static char star[] = "*";
      aConfItem *aconf;
#ifndef	INET6
      u_long vaddr;
#else
      char vaddr[sizeof (struct IN_ADDR)];
#endif

      if ((aconf = find_me ()) && portarg <= 0 && aconf->port > 0)
	portnum = aconf->port;

      Debug ((DEBUG_ERROR, "Port = %d", portnum));

      if ((aconf->passwd[0] != '\0') && (aconf->passwd[0] != '*'))
#ifndef INET6
	inet_pton (AFINET, aconf->passwd, &vaddr);
#else
	inet_pton (AFINET, aconf->passwd, vaddr);
#endif
      else
#ifndef INET6
	vaddr = 0;
#else
	memset (vaddr, 0x0, sizeof (vaddr));
#endif
      if (inetport (&me, star, portnum, vaddr))
	{
	  if (bootopt & BOOT_STDERR)
	    {
	      fprintf (stderr, "§ Couldn't bind to primary port %d\n",
		       portnum);
	      fprintf (stderr,
		       "§ Please make sure no other application is using port %d on this IP\n",
		       portnum);
	      fprintf (stderr,
		       "§ and that the IRCd is not already running.\n");
	      fprintf (stderr,
		       "§~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~§\n");
	    }
#ifdef USE_SYSLOG
	  (void) syslog (LOG_CRIT, "Couldn't bind to primary port %d\n",
			 portnum);
#endif
	  exit (1);
	}
    }
  else if (inetport (&me, "*", 0, 0))
    {
      if (bootopt & BOOT_STDERR)
	{
	  fprintf (stderr, "§ Couldn't bind to port passed from inetd\n");
	  fprintf (stderr,
		   "§~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~§\n");
	}
#ifdef USE_SYSLOG
      syslog (LOG_CRIT, "Couldn't bind to port passed from inetd\n");
#endif
      exit (1);
    }

  set_non_blocking (me.fd, &me);
  (void) get_my_name (&me, me.sockhost, sizeof (me.sockhost) - 1);
  if (me.name[0] == '\0')
    strncpyzt (me.name, me.sockhost, sizeof (me.name));
  me.hopcount = 0;
  me.authfd = -1;
  me.confs = NULL;
  me.next = NULL;
  me.user = NULL;
  me.from = &me;
  SetMe (&me);
  make_server (&me);
  me.serv->up = me.name;
  me.lasttime = me.since = me.firsttime = NOW;
  (void) add_to_client_hash_table (me.name, &me);

  /* We don't want to calculate these every time they are used :) */


  sprintf (REPORT_DO_DNS, REPORT_DO_DNS_, me.name);
  sprintf (REPORT_FIN_DNS, REPORT_FIN_DNS_, me.name);
  sprintf (REPORT_FIN_DNSC, REPORT_FIN_DNSC_, me.name);
  sprintf (REPORT_FAIL_DNS, REPORT_FAIL_DNS_, me.name);
  sprintf (REPORT_DO_ID, REPORT_DO_ID_, me.name);
  sprintf (REPORT_FIN_ID, REPORT_FIN_ID_, me.name);
  sprintf (REPORT_FAIL_ID, REPORT_FAIL_ID_, me.name);

  R_do_dns = strlen (REPORT_DO_DNS);
  R_fin_dns = strlen (REPORT_FIN_DNS);
  R_fin_dnsc = strlen (REPORT_FIN_DNSC);
  R_fail_dns = strlen (REPORT_FAIL_DNS);
  R_do_id = strlen (REPORT_DO_ID);
  R_fin_id = strlen (REPORT_FIN_ID);
  R_fail_id = strlen (REPORT_FAIL_ID);

  check_class ();
  if (bootopt & BOOT_OPER)
    {
      aClient *tmp = add_connection (&me, 0);

      if (!tmp)
	exit (1);
      SetMaster (tmp);
    }
  else
    write_pidfile ();

  Debug ((DEBUG_NOTICE, "Server ready..."));
#ifdef USE_SYSLOG
  syslog (LOG_NOTICE, "Server Ready");
#endif
  NOW = time (NULL);

#ifndef NO_PRIORITY
  check_fdlists ();
#endif

  if ((timeofday = time (NULL)) == -1)
    {
#ifdef USE_SYSLOG
      syslog (LOG_WARNING, "Clock Failure (%d), TS can be corrupted", errno);
#endif
      sendto_ops ("Clock Failure (%d), TS can be corrupted", errno);
    }

#ifdef DUMP_DEBUG
  dumpfp = fopen ("dump.log", "w");
#endif

  io_loop ();
  return 0;
}

static time_t next_gc = 0;
void
io_loop ()
{
  char to_send[200];
  time_t lasttime = 0;
  long lastrecvK = 0;
  int lrv = 0;
  time_t lastbwcalc = 0;
  long lastbwSK = 0, lastbwRK = 0;
  time_t lasttimeofday;
  int delay = 0;

  while (1)
    {
      lasttimeofday = timeofday;

      if ((timeofday = time (NULL)) == -1)
	{
#ifdef USE_SYSLOG
	  syslog (LOG_WARNING, "Clock Failure (%d), TS can be corrupted",
		  errno);
#endif
	  sendto_ops ("Clock Failure (%d), TS can be corrupted", errno);
	}

      if (timeofday < lasttimeofday)
	{
	  ircsprintf (to_send,
		      "System clock is running backwards - (%d < %d)",
		      timeofday, lasttimeofday);
	  report_error (to_send, &me);
	}

      NOW = timeofday;


      if (!next_gc)
	{
	  next_gc = timeofday + 600;
	}

      /*
       * Calculate a moving average of our total traffic.
       * Traffic is a 4 second average, 'sampled' every 2 seconds.
       */

      if ((timeofday - lastbwcalc) >= 2)
	{
	  long ilength = timeofday - lastbwcalc;

	  curSendK += (float) (me.sendK - lastbwSK) / (float) ilength;
	  curRecvK += (float) (me.receiveK - lastbwRK) / (float) ilength;
	  curSendK /= 2;
	  curRecvK /= 2;

	  lastbwSK = me.sendK;
	  lastbwRK = me.receiveK;
	  lastbwcalc = timeofday;
	}

      /*
       * This chunk of code determines whether or not "life sucks", that
       * is to say if the traffic level is so high that standard server
       * commands should be restricted
       *
       * Changed by Taner so that it tells you what's going on as well as
       * allows forced on (long LCF), etc...
       */

      if ((timeofday - lasttime) >= LCF)
	{
	  lrv = LRV * LCF;
	  lasttime = timeofday;
	  currlife = (me.receiveK - lastrecvK) / LCF;
	  if ((me.receiveK - lrv) > lastrecvK || HTMLOCK == YES)
	    {
	      if (!lifesux)
		{
		  /*
		   * In the original +th code Taner had
		   *
		   * LCF << 1;  / * add hysteresis * /
		   *
		   * which does nothing... so, the hybrid team changed it to
		   *
		   * LCF <<= 1;  / * add hysteresis * /
		   *
		   * suddenly, there were reports of clients mysteriously just
		   * dropping off... Neither rodder or I can see why it makes
		   * a difference, but lets try it this way...
		   *
		   * The original dog3 code, does not have an LCF variable
		   *
		   * -Dianora
		   *
		   */
		  lifesux = 1;

		  if (noisy_htm)
		    sendto_ops
		      ("Entering high-traffic mode - (%dk/s > %dk/s)",
		       currlife, LRV);
		}
	      else
		{
		  lifesux++;	/* Ok, life really sucks! */
		  LCF += 2;	/* Wait even longer */
		  if (noisy_htm)
		    sendto_ops
		      ("Still high-traffic mode %d%s (%d delay): %dk/s",
		       lifesux, (lifesux > 9) ? " (TURBO)" : "", (int) LCF,
		       currlife);

		  /* Reset htm here, because its been on a little too long.
		   * Bad Things(tm) tend to happen with HTM on too long -epi */

		  if (lifesux > 15)
		    {
		      if (noisy_htm)
			sendto_ops
			  ("Resetting HTM and raising limit to: %dk/s\n",
			   LRV + 5);
		      LCF = LOADCFREQ;
		      lifesux = 0;
		      LRV += 5;
		    }
		}
	    }
	  else
	    {
	      LCF = LOADCFREQ;
	      if (lifesux)
		{
		  lifesux = 0;
		  if (noisy_htm)
		    sendto_ops ("Resuming standard operation . . . .");
		}
	    }
	  lastrecvK = me.receiveK;
	}
      /*
       * * We only want to connect if a connection is due, not every
       * time through.  Note, if there are no active C lines, this call
       * to Tryconnections is made once only; it will return 0. - avalon
       */

      if (nextconnect && timeofday >= nextconnect)
	nextconnect = try_connections (timeofday);

      /* DNS checks. One to timeout queries, one for cache expiries. */

      if (timeofday >= nextdnscheck)
	nextdnscheck = timeout_query_list (timeofday);
      if (timeofday >= nextexpire)
	nextexpire = expire_cache (timeofday);

      if (timeofday >= nextbanexpire)
	{
	  /*
	   * magic number: 31 seconds
	   * space out these heavy tasks at semi-random intervals, so as not to coincide
	   * with anything else ircd does regularly
	   */
	  nextbanexpire = NOW + 31;
	  expire_userbans ();
	  check_expire_gline();
#ifdef THROTTLE_ENABLE
	  throttle_timer (NOW);
#endif
	}

      /*
       * * take the smaller of the two 'timed' event times as the time
       * of next event (stops us being late :) - avalon WARNING -
       * nextconnect can return 0!
       */

      if (nextconnect)
	delay = MIN (nextping, nextconnect);
      else
	delay = nextping;
      delay = MIN (nextdnscheck, delay);
      delay = MIN (nextexpire, delay);
      delay -= timeofday;

      /*
       * * Adjust delay to something reasonable [ad hoc values] (one
       * might think something more clever here... --msa) 
       * We don't really need to check that often and as long 
       * as we don't delay too long, everything should be ok. 
       * waiting too long can cause things to timeout... 
       * i.e. PINGS -> a disconnection :( 
       * - avalon
       */
      if (delay < 1)
	delay = 1;
      else
	delay = MIN (delay, TIMESEC);
      /*
       * We want to read servers on every io_loop, as well as "busy"
       * clients (which again, includes servers. If "lifesux", then we
       * read servers AGAIN, and then flush any data to servers. -Taner
       */

#ifndef NO_PRIORITY
      read_message (0, &serv_fdlist);
      read_message (1, &busycli_fdlist);
      if (lifesux)
	{
	  (void) read_message (1, &serv_fdlist);
	  if (lifesux > 9)	/* life really sucks */
	    {
	      (void) read_message (1, &busycli_fdlist);
	      (void) read_message (1, &serv_fdlist);
	    }
	  flush_fdlist_connections (&serv_fdlist);
	}

      if ((timeofday = time (NULL)) == -1)
	{
#ifdef USE_SYSLOG
	  syslog (LOG_WARNING, "Clock Failure (%d), TS can be corrupted",
		  errno);
#endif
	  sendto_ops ("Clock Failure (%d), TS can be corrupted", errno);
	}
      /*
       * CLIENT_SERVER = TRUE: If we're in normal mode, or if "lifesux"
       * and a few seconds have passed, then read everything.
       * CLIENT_SERVER = FALSE: If it's been more than lifesux*2 seconds
       * (that is, at most 1 second, or at least 2s when lifesux is != 0)
       * check everything. -Taner
       */
      {
	static time_t lasttime = 0;

# ifdef CLIENT_SERVER
	if (!lifesux || (lasttime + lifesux) < timeofday)
	  {
# else
	if ((lasttime + (lifesux + 1)) < timeofday)
	  {
# endif
	    (void) read_message (delay ? delay : 1, NULL);	/* check everything! */
	    lasttime = timeofday;
	  }
      }
#else
      (void) read_message (delay, NULL);	/* check everything! */
#endif
      /*
       * * ...perhaps should not do these loops every time, but only if
       * there is some chance of something happening (but, note that
       * conf->hold times may be changed elsewhere--so precomputed next
       * event time might be too far away... (similarly with ping
       * times) --msa
       */

      if ((timeofday >= nextping))
	nextping = check_pings (timeofday);

#ifdef PROFILING
      if (profiling_newmsg)
	{
	  sendto_realops ("PROFILING: %s", profiling_msg);
	  profiling_newmsg = 0;
	}
#endif

      if (dorehash && !lifesux)
	{
	  (void) rehash (&me, &me, 1);
	  dorehash = 0;
	}
      /*

       * Flush output buffers on all connections now if they 
       * have data in them (or at least try to flush)  -avalon
       *
       * flush_connections(me.fd);
       *
       * avalon, what kind of crack have you been smoking? why
       * on earth would we flush_connections blindly when
       * we already check to see if we can write (and do)
       * in read_message? There is no point, as this causes
       * lots and lots of unnecessary sendto's which 
       * 99% of the time will fail because if we couldn't
       * empty them in read_message we can't empty them here.
       * one effect: during htm, output to normal lusers
       * will lag.
       */

      /* Now we've made this call a bit smarter. */
      /* Only flush non-blocked sockets. */

      flush_connections (me.fd);

#ifndef NO_PRIORITY
      check_fdlists ();
#endif

      if (timeofday >= next_gc)
	{
	  block_garbage_collect ();
	  next_gc = timeofday + 600;
	}


#ifdef	LOCKFILE
      /*
       * * If we have pending klines and CHECK_PENDING_KLINES minutes
       * have passed, try writing them out.  -ThemBones
       */

      if ((pending_klines) && ((timeofday - pending_kline_time)
			       >= (CHECK_PENDING_KLINES * 60)))
	do_pending_klines ();
#endif
   }
}

/*
 * open_debugfile
 * 
 * If the -t option is not given on the command line when the server is
 * started, all debugging output is sent to the file set by LPATH in
 * config.h Here we just open that file and make sure it is opened to
 * fd 2 so that any fprintf's to stderr also goto the logfile.  If the
 * debuglevel is not set from the command line by -x, use /dev/null as
 * the dummy logfile as long as DEBUGMODE has been defined, else dont
 * waste the fd.
 */
static void
open_debugfile ()
{
#ifdef	DEBUGMODE
  int fd;
  aClient *client_p;

  if (debuglevel >= 0)
    {
      client_p = make_client (NULL, NULL);
      client_p->fd = 2;
      SetLog (client_p);
      client_p->port = debuglevel;
      client_p->flags = 0;
      client_p->acpt = client_p;
      local[2] = client_p;
      (void) strcpy (client_p->sockhost, me.sockhost);

      (void) printf ("isatty = %d ttyname = %#x\n",
		     isatty (2), (u_int) ttyname (2));
      if (!(bootopt & BOOT_TTY))	/* leave debugging output on fd */
	{
	  (void) truncate (LOGFILE, 0);
	  if ((fd = open (LOGFILE, O_WRONLY | O_CREAT, 0600)) < 0)
	    if ((fd = open ("/dev/null", O_WRONLY)) < 0)
	      exit (-1);
	  if (fd != 2)
	    {
	      (void) dup2 (fd, 2);
	      (void) close (fd);
	    }
	  strncpyzt (client_p->name, LOGFILE, sizeof (client_p->name));
	}
      else if (isatty (2) && ttyname (2))
	strncpyzt (client_p->name, ttyname (2), sizeof (client_p->name));
      else
	(void) strcpy (client_p->name, "FD2-Pipe");
      Debug ((DEBUG_FATAL, "Debug: File <%s> Level: %d at %s",
	      client_p->name, client_p->port, myctime (time (NULL))));
    }
  else
    local[2] = NULL;
#endif
  return;
}

#ifdef BACKTRACE_DEBUG
#include <execinfo.h>

/*
 * lastdyingscream - signal handler function
 * Will output backtrace info.
 *
 * Written by solaris - mbusigin@amber.org.uk
 * code must be compiled with -rdynamic and some sort of -g flag
 */
void
lastdyingscream (int s)
{
  int fd;
  void *array[128];
  int r;

  fd = open ("lastdyingscream.log", O_CREAT | O_WRONLY);

  memset (array, '\0', sizeof (array));
  r = backtrace (array, sizeof (array) - 1);

  backtrace_symbols_fd (array, r, fd);
  shutdown (fd, 2);
  close (fd);
}
#endif

static void
setup_signals ()
{
#ifdef	POSIX_SIGNALS
  struct sigaction act;

  act.sa_handler = SIG_IGN;
  act.sa_flags = 0;
  (void) sigemptyset (&act.sa_mask);
  (void) sigaddset (&act.sa_mask, SIGPIPE);
  (void) sigaddset (&act.sa_mask, SIGALRM);
# ifdef	SIGWINCH
  (void) sigaddset (&act.sa_mask, SIGWINCH);
  (void) sigaction (SIGWINCH, &act, NULL);
# endif
  (void) sigaction (SIGPIPE, &act, NULL);
  act.sa_handler = dummy;
  (void) sigaction (SIGALRM, &act, NULL);
  act.sa_handler = s_rehash;
  (void) sigemptyset (&act.sa_mask);
  (void) sigaddset (&act.sa_mask, SIGHUP);
  (void) sigaction (SIGHUP, &act, NULL);
  act.sa_handler = s_restart;
  (void) sigaddset (&act.sa_mask, SIGINT);
  (void) sigaction (SIGINT, &act, NULL);
  act.sa_handler = s_die;
  (void) sigaddset (&act.sa_mask, SIGTERM);
  (void) sigaction (SIGTERM, &act, NULL);


#else
# ifndef	HAVE_RELIABLE_SIGNALS
  (void) signal (SIGPIPE, dummy);
#  ifdef	SIGWINCH
  (void) signal (SIGWINCH, dummy);
#  endif
# else
#  ifdef	SIGWINCH
  (void) signal (SIGWINCH, SIG_IGN);
#  endif
  (void) signal (SIGPIPE, SIG_IGN);
# endif
  (void) signal (SIGALRM, dummy);
  (void) signal (SIGHUP, s_rehash);
  (void) signal (SIGTERM, s_die);
  (void) signal (SIGINT, s_restart);
#ifdef BACKTRACE_DEBUG
  /* a function that emits a last dying scream backtrace */
  (void) signal (SIGSEGV, lastdyingscream);
#endif
#endif

#ifdef RESTARTING_SYSTEMCALLS
  /*
   * * At least on Apollo sr10.1 it seems continuing system calls 
   * after signal is the default. The following 'siginterrupt' 
   * should change that default to interrupting calls.
   */
  (void) siginterrupt (SIGALRM, 1);
#endif
}

#ifndef NO_PRIORITY
/*
 * This is a pretty expensive routine -- it loops through all the fd's,
 * and finds the active clients (and servers and opers) and places them
 * on the "busy client" list
 */
void
check_fdlists ()
{
#ifdef CLIENT_SERVER
#define BUSY_CLIENT(x)	(((x)->priority < 55) || (!lifesux && ((x)->priority < 75)))
#else
#define BUSY_CLIENT(x)	(((x)->priority < 40) || (!lifesux && ((x)->priority < 60)))
#endif
#define FDLISTCHKFREQ  2

  aClient *client_p;
  int i, j;

  j = 0;
  for (i = highest_fd; i >= 0; i--)
    {
      if (!(client_p = local[i]))
	continue;
      if (IsServer (client_p) || IsListening (client_p) || IsOper (client_p))
	{
	  busycli_fdlist.entry[++j] = i;
	  continue;
	}
      if (client_p->receiveM == client_p->lastrecvM)
	{
	  client_p->priority += 2;	/* lower a bit */
	  if (client_p->priority > 90)
	    client_p->priority = 90;
	  else if (BUSY_CLIENT (client_p))
	    busycli_fdlist.entry[++j] = i;
	  continue;
	}
      else
	{
	  client_p->lastrecvM = client_p->receiveM;
	  client_p->priority -= 30;	/* active client */
	  if (client_p->priority < 0)
	    {
	      client_p->priority = 0;
	      busycli_fdlist.entry[++j] = i;
	    }
	  else if (BUSY_CLIENT (client_p))
	    busycli_fdlist.entry[++j] = i;
	}
    }
  busycli_fdlist.last_entry = j;	/* rest of the fdlist is garbage */
/*   return (now + FDLISTCHKFREQ + (lifesux + 1)); */
}
#endif

void
build_version (void)
{
  char *s = PATCHES;
  ircsprintf (version, "%s/%s.%s-(%s)%s%s", BASE_VERSION,
	      MAJOR_RELEASE, MINOR_RELEASE, RELEASE_NAME,
	      (*s != 0 ? "+" : ""), (*s != 0 ? PATCHES : ""));
}

void
log (const char *fmt, ...)
{
  return;
}


void
mode_sort (char *str)
{
  char temp;
  int x, y;

  for (x = 0; x < (strlen (str) - 1); x++)
    {
      for (y = x + 1; y < strlen (str); y++)
	{
	  if (isupper (str[y]) && islower (str[x]))
	    continue;
	  if ((islower (str[y]) && isupper (str[x])) || (str[y] < str[x]))
	    {
	      temp = str[x];
	      str[x] = str[y];
	      str[y] = temp;
	    }
	}
    }
}
