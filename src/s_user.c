#include "struct.h"
#include "common.h"
#include "sys.h"
#include "numeric.h"
#include "msg.h"
#include "channel.h"
#include "throttle.h"
#include <sys/stat.h>
#include <utmp.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "h.h"
#include "dynconf.h"
#include "supported.h"
#ifdef FLUD
#include "blalloc.h"
#endif /* FLUD  */
#include "userban.h"
#include "inet.h"		/*INET6 */
#if defined( HAVE_STRING_H)
#include <string.h>
#else
#include <strings.h>
#endif

int
do_user (char *, aClient *, aClient *, char *, char *, char *,
	 unsigned long, unsigned int, char *, char *);


extern char motd_last_changed_date[];
extern char opermotd_last_changed_date[];
extern const char *get_listener_name(aClient *);
extern char *glreason;

extern aGline *find_glined_user (aClient *);

extern int send_motd (aClient *, aClient *, int, char **);
extern void send_topic_burst (aClient *);
extern void outofmemory (void);	/*
				 * defined in list.c 
				 */

extern void send_snomask_out (aClient *, aClient *, int);

extern char *strip_colour(char *);

extern char *oflagstr(int);

#ifdef MAXBUFFERS
extern void reset_sock_opts ();
extern int send_lusers (aClient *, aClient *, int, char **);

#endif
extern int lifesux;

#ifdef WINGATE_NOTICE
extern char ProxyMonURL[TOPICLEN + 1];
extern char ProxyMonHost[HOSTLEN + 1];
#endif

static char buf[BUFSIZE], buf2[BUFSIZE];
int user_modes[] = {
  UMODE_i, 'i',			/* Invisible */
  UMODE_o, 'o',			/* Global Oper */
  UMODE_r, 'r',			/* Registered Nick */
  UMODE_w, 'w',			/* See Wallops Notices */
  UMODE_x, 'x',			/* Hiddenhost */
  0, 0
};

int user_mode_table[] = {
  /* 0 - 32 are control chars and space */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0,				/* ! */
  0,				/* " */
  0,				/* # */
  0,				/* $ */
  0,				/* % */
  0,				/* & */
  0,				/* ' */
  0,				/* ( */
  0,				/* ) */
  0,				/* * */
  0,				/* + */
  0,				/* , */
  0,				/* - */
  0,				/* . */
  0,				/* / */
  0,				/* 0 */
  0,				/* 1 */
  0,				/* 2 */
  0,				/* 3 */
  0,				/* 4 */
  0,				/* 5 */
  0,				/* 6 */
  0,				/* 7 */
  0,				/* 8 */
  0,				/* 9 */
  0,				/* : */
  0,				/* ; */
  0,				/* < */
  0,				/* = */
  0,				/* > */
  0,				/* ? */
  0,				/* @ */
  0,				/* A */
  0,				/* B */
  0,				/* C */
  0,				/* D */
  0,				/* E */
  0,    			/* F */
  0,				/* G */
  0,				/* H */
  0,				/* I */
  0,				/* J */
  0,				/* K */
  0,				/* L */
  0,				/* M */
  0,				/* N */
  0,				/* O */
  0,				/* P */
  0,				/* Q */
  0,				/* R */
  0,				/* S */
  0,				/* T */
  0,				/* U */
  0,				/* V */
  0,				/* W */
  0,				/* X */
  0,				/* Y */
  0,				/* Z */
  0,				/* [ */
  0,				/* \ */
  0,				/* ] */
  0,				/* ^ */
  0,				/* _ */
  0,				/* ` */
  0,				/* a */
  0,				/* b */
  0,			/* c */
  0,			/* d */
  0,			/* e */
  0,			/* f */
  0,			/* g */
  0,				/* h */
  UMODE_i,			/* i */
  0,			/* j */
  0,			/* k */
  0,				/* l */
  0,			/* m */
  0,			/* n */
  UMODE_o,			/* o */
  0,				/* p */
  0,				/* q */
  UMODE_r,			/* r */
  0,				/* s */
  0,				/* t */
  0,				/* u */
  0,				/* v */
  UMODE_w,			/* w */
  UMODE_x,			/* x */
  0,     			/* y */
  0,				/* z */
  0,				/* { */
  0,				/* | */
  0,				/* } */
  0,				/* ~ */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 127 - 141 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 142 - 156 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 157 - 171 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 172 - 186 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 187 - 201 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 202 - 216 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 217 - 231 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 232 - 246 */
  0, 0, 0, 0, 0, 0, 0, 0, 0	/* 247 - 255 */
};

/*
 * internally defined functions
 */
int botreject (char *);
unsigned long my_rand (void);	/*
				 * provided by orabidoo 
				 */
/*
 * externally defined functions 
 */
extern int find_fline (aClient *);	/*

					 * defined in s_conf.c
					 */
extern Link *find_channel_link (Link *, aChannel *);	/*

							 * defined in list.c 
							 */
#ifdef FLUD
int flud_num = FLUD_NUM;
int flud_time = FLUD_TIME;
int flud_block = FLUD_BLOCK;
extern BlockHeap *free_fludbots;
extern BlockHeap *free_Links;

void announce_fluder (aClient *, aClient *, aChannel *, int);
struct fludbot *remove_fluder_reference (struct fludbot **, aClient *);
Link *remove_fludee_reference (Link **, void *);
int check_for_fludblock (aClient *, aClient *, aChannel *, int);
int check_for_flud (aClient *, aClient *, aChannel *, int);
void free_fluders (aClient *, aChannel *);
void free_fludees (aClient *);
#endif
int check_for_ctcp (char *, char **);
int allow_dcc (aClient *, aClient *);
static int is_silenced (aClient *, aClient *);

#ifdef ANTI_SPAMBOT
int spam_time = MIN_JOIN_LEAVE_TIME;
int spam_num = MAX_JOIN_LEAVE_COUNT;
#endif

/* defines for check_ctcp results */
#define CTCP_NONE 	0
#define CTCP_YES	1
#define CTCP_DCC	2
#define CTCP_DCCSEND 	3


/*
 * * m_functions execute protocol messages on this server: *
 *
 *      client_p    is always NON-NULL, pointing to a *LOCAL* client *
 * tructure (with an open socket connected!). This *
 * es the physical socket where the message *           originated (or
 * which caused the m_function to be *          executed--some
 * m_functions may call others...). *
 *
 *      source_p    is the source of the message, defined by the *
 * refix part of the message if present. If not *               or
 * prefix not found, then source_p==client_p. *
 *
 *              (!IsServer(client_p)) => (client_p == source_p), because *
 * refixes are taken *only* from servers... *
 * 
 *              (IsServer(client_p)) *                      (source_p == client_p)
 * => the message didn't *                      have the prefix. *
 *
 *                      (source_p != client_p && IsServer(source_p) means *
 * he prefix specified servername. (?) *
 * 
 *                      (source_p != client_p && !IsServer(source_p) means *
 * hat message originated from a remote *                       user
 * (not local). *
 * 
 *              combining *
 * 
 *              (!IsServer(source_p)) means that, source_p can safely *
 * aken as defining the target structure of the *               message
 * in this server. *
 *
 *      *Always* true (if 'parse' and others are working correct): *
 *
 *      1)      source_p->from == client_p  (note: client_p->from == client_p) *
 * 
 *      2)      MyConnect(source_p) <=> source_p == client_p (e.g. source_p *
 * annot* be a local connection, unless it's *          actually
 * client_p!). [MyConnect(x) should probably *              be defined as
 * (x == x->from) --msa ] *
 *
 *      parc    number of variable parameter strings (if zero, *
 * arv is allowed to be NULL) *
 * 
 *      parv    a NULL terminated list of parameter pointers, *
 *
 *                      parv[0], sender (prefix string), if not present *
 * his points to an empty string. *
 * arc-1] *                             pointers to additional
 * parameters *                 parv[parc] == NULL, *always* *
 *
 *              note:   it is guaranteed that parv[0]..parv[parc-1] are
 * all *                        non-NULL pointers.
 */
/*
 * * next_client *    Local function to find the next matching
 * client. The search * can be continued from the specified client
 * entry. Normal *      usage loop is: *
 * 
 *      for (x = client; x = next_client(x,mask); x = x->next) *
 * andleMatchingClient; *
 * 
 */
aClient *
next_client (aClient * next,	/*
				 * First client to check
				 */
	     char *ch)
{				/*
				 * search string (may include wilds) 
				 */
  aClient *tmp = next;

  next = find_client (ch, tmp);
  if (tmp && tmp->prev == next)
    return ((aClient *) NULL);

  if (next != tmp)
    return next;
  for (; next; next = next->next)
    {
      if (!match (ch, next->name))
	break;
    }
  return next;
}

/*
 * this slow version needs to be used for hostmasks *sigh * 
 */

aClient *
next_client_double (aClient * next,	/*
					 * First client to check 
					 */
		    char *ch)
{				/*
				 * search string (may include wilds) 
				 */
  aClient *tmp = next;

  next = find_client (ch, tmp);
  if (tmp && tmp->prev == next)
    return NULL;
  if (next != tmp)
    return next;
  for (; next; next = next->next)
    {
      if (!match (ch, next->name) || !match (next->name, ch))
	break;
    }
  return next;
}

/*
 * * hunt_server *
 *
 *      Do the basic thing in delivering the message (command) *
 * across the relays to the specific server (server) for *
 * actions. *
 * 
 *      Note:   The command is a format string and *MUST* be *
 * f prefixed style (e.g. ":%s COMMAND %s ..."). *              Command
 * can have only max 8 parameters. *
 *
 *      server  parv[server] is the parameter identifying the *
 * arget server. *
 * 
 *      *WARNING* *             parv[server] is replaced with the
 * pointer to the *             real servername from the matched client
 * (I'm lazy *          now --msa). *
 * 
 *      returns: (see #defines)
 */
int
hunt_server (aClient * client_p,
	     aClient * source_p,
	     char *command, int server, int parc, char *parv[])
{
  aClient *target_p;
  int wilds;

  /*
   * * Assume it's me, if no server
   */
  if (parc <= server || BadPtr (parv[server]) ||
      match (me.name, parv[server]) == 0 ||
      match (parv[server], me.name) == 0)
    return (HUNTED_ISME);
  /*
   * * These are to pickup matches that would cause the following *
   * message to go in the wrong direction while doing quick fast *
   * non-matching lookups.
   */
  if ((target_p = find_client (parv[server], NULL)))
    if (target_p->from == source_p->from && !MyConnect (target_p))
      target_p = NULL;
  if (!target_p && (target_p = find_server (parv[server], NULL)))
    if (target_p->from == source_p->from && !MyConnect (target_p))
      target_p = NULL;

  (void) collapse (parv[server]);
  wilds = (strchr (parv[server], '?') || strchr (parv[server], '*'));
  /*
   * Again, if there are no wild cards involved in the server name,
   * use the hash lookup - Dianora
   */

  if (!target_p)
    {
      if (!wilds)
	{
	  target_p = find_name (parv[server], (aClient *) NULL);
	  if (!target_p || !IsRegistered (target_p) || !IsServer (target_p))
	    {
	      sendto_one (source_p, err_str (ERR_NOSUCHSERVER), me.name,
			  parv[0], parv[server]);
	      return (HUNTED_NOSUCH);
	    }
	}
      else
	{
	  for (target_p = client;
	       (target_p = next_client (target_p, parv[server]));
	       target_p = target_p->next)
	    {
	      if (target_p->from == source_p->from && !MyConnect (target_p))
		continue;
	      /*
	       * Fix to prevent looping in case the parameter for some
	       * reason happens to match someone from the from link --jto
	       */
	      if (IsRegistered (target_p) && (target_p != client_p))
		break;
	    }
	}
    }

  if (target_p)
    {
      if (IsMe (target_p) || MyClient (target_p))
	return HUNTED_ISME;
      if (match (target_p->name, parv[server]))
	parv[server] = target_p->name;
      sendto_one (target_p, command, parv[0],
		  parv[1], parv[2], parv[3], parv[4],
		  parv[5], parv[6], parv[7], parv[8]);
      return (HUNTED_PASS);
    }
  sendto_one (source_p, err_str (ERR_NOSUCHSERVER), me.name,
	      parv[0], parv[server]);
  return (HUNTED_NOSUCH);
}

/*
 * * canonize *
 *
 * reduce a string of duplicate list entries to contain only the unique *
 * items.  Unavoidably O(n^2).
 */
char *
canonize (char *buffer)
{
  static char cbuf[BUFSIZ];
  char *s, *t, *cp = cbuf;
  int l = 0;
  char *p = NULL, *p2;

  *cp = '\0';

  for (s = strtoken (&p, buffer, ","); s; s = strtoken (&p, NULL, ","))
    {
      if (l)
	{
	  for (p2 = NULL, t = strtoken (&p2, cbuf, ","); t;
	       t = strtoken (&p2, NULL, ","))
	    if (irccmp (s, t) == 0)
	      break;
	    else if (p2)
	      p2[-1] = ',';
	}
      else
	t = NULL;

      if (!t)
	{
	  if (l)
	    *(cp - 1) = ',';
	  else
	    l = 1;
	  (void) strcpy (cp, s);
	  if (p)
	    cp += (p - s);
	}
      else if (p2)
	p2[-1] = ',';
    }
  return cbuf;
}

char umodestring[128];

void
build_umodestr (void)
{
  int *s;
  char *m;

  m = umodestring;

  for (s = user_modes; *s; s += 2)
    {
      *m++ = (char) (*(s + 1));
    }

  *m = '\0';
}

/*
 * show_isupport
 *
 * inputs	- pointer to client
 * output	-
 * side effects	- display to client what we support (for them)
 */
void
show_isupport (aClient * source_p)
{
  char isupportbuffer[512];

  ircsprintf (isupportbuffer, FEATURES, FEATURESVALUES);
  sendto_one (source_p, rpl_str (RPL_PROTOCTL), me.name, source_p->name,
	      isupportbuffer);

  ircsprintf (isupportbuffer, FEATURES2, FEATURES2VALUES);
  sendto_one (source_p, rpl_str (RPL_PROTOCTL), me.name, source_p->name,
	      isupportbuffer);
  return;
}


/*
 * register_user
 *
 * This function is called when both NICK and USER messages
 * have been accepted for the client, in whatever order.
 * Only after this, is the USER message propagated.
 *
 * NICK's must be propagated at once when received, although
 * it would be better to delay them too until full info is
 * available. Doing it is not so simple though, would have
 * to implement the following:
 *
 * (actually it has been implemented already for a while)
 * -orabidoo
 *
 * 1) user telnets in and gives only "NICK foobar" and waits
 * 2) another user far away logs in normally with the nick
 * "foobar" (quite legal, as this server didn't propagate it).
 * 3) now this server gets nick "foobar" from outside, but has
 * already the same defined locally. Current server would just
 * issue "KILL foobar" to clean out dups. But,this is not
 * fair. It should actually request another nick from local user
 * or kill him/her...
 */

int
register_user (aClient * client_p, aClient * source_p, char *nick,
	       char *username)
{
  aClient *nsource_p;
  aConfItem *aconf = NULL, *pwaconf = NULL;
  char *parv[3];
  static char ubuf[34];
  static char sbuf[34];
#ifdef INET6
  char *p, *p2;
#else
  char *p;
#endif
  short oldstatus = source_p->status;
  struct userBan *ban;
  anUser *user = source_p->user;
  aMotd *smotd;
  int i, dots;
  int bad_dns;		/* Flags a bad dns name... */
  aGline *gline;
#ifdef ANTI_SPAMBOT
  char spamchar = '\0';

#endif
  char tmpstr2[512];

  user->last = timeofday;
  parv[0] = source_p->name;
  parv[1] = parv[2] = NULL;

  /*
   * Local clients will already have source_p->hostip
   */
  if (!MyConnect (source_p))
    {
      inet_ntop (AFINET, &source_p->ip, source_p->hostip, HOSTIPLEN + 1);
#ifdef INET6
      ip6_expand (source_p->hostip, HOSTIPLEN);
#endif
    }
  p = source_p->hostip;

  if (MyConnect (source_p))
    {
      /*
       * This is where we check for E and F lines and flag users if they match.
       * Should save us having to walk the entire E and F line list later on.
       * We have already walked each list twice if we have throttling enabled
       * but that cannot really be helped.
       */

      if (find_eline (source_p))
	{
	  SetKLineExempt (source_p);
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** You are exempt from K/G lines. congrats.",
		      me.name, source_p->name);
	}
      if (find_fline (source_p))
	{
	  SetSuperExempt (source_p);
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** You are exempt from user limits. congrats.",
		      me.name, source_p->name);
	}

      if ((i = check_client (source_p)))
	{
	  /*
	   * -2 is a socket error, already reported.
	   */
	  if (i != -2)
	    {
	      if (i == -4)
		{
		  ircstp->is_ref++;
		  return exit_client (client_p, source_p, &me,
				      "Too many connections from your hostname");
		}
	      else if (i == -3)
		sendto_snomask (SNOMASK_REJECTS, "%s for %s [%s] ",
				    "I-line is full (Server is full)",
				    get_client_host (source_p), p);
	      else
		sendto_snomask (SNOMASK_REJECTS, "%s from %s [%s]",
				    "Unauthorized client connection",
				    get_client_host (source_p), p);
	      ircstp->is_ref++;
	      return exit_client (client_p, source_p, &me, i == -3 ?
				  "No more connections allowed in your connection class (the server is full)"
				  :
				  "You are not authorized to use this server");
	    }
	  else
	    return exit_client (client_p, source_p, &me, "Socket Error");
	}

#ifdef ANTI_SPAMBOT
      /*
       * This appears to be broken 
       */
      /*
       * Check for single char in user->host -ThemBones 
       */
      if (*(user->host + 1) == '\0')
	spamchar = *user->host;
#endif

      strncpyzt (user->host, source_p->sockhost, HOSTLEN);

      dots = 0;
      p = user->host;
      bad_dns = NO;
      while (*p)
	{
	  if (!isalnum (*p))
	    {
#ifdef RFC1035_ANAL
	      if ((*p != '-') && (*p != '.')
#ifdef INET6
		  && (*p != ':')
#endif
		)
#else
	      if ((*p != '-') && (*p != '.') && (*p != '_') && (*p != '/')
#ifdef INET6
		  && (*p != ':')
#endif
		)
#endif /*
        * RFC1035_ANAL
        */
		bad_dns = YES;
	    }
#ifndef INET6
	  if (*p == '.')
#else
	  if (*p == '.' || *p == ':')
#endif
	    dots++;
	  p++;
	}
      /*
       * Check that the hostname has AT LEAST ONE dot (.) in it. If
       * not, drop the client (spoofed host) -ThemBones
       */
      if (!dots)
	{
	  sendto_realops ("Invalid hostname for %s, dumping user %s",
			  source_p->hostip, source_p->name);
	  return exit_client (client_p, source_p, &me, "Invalid hostname");
	}

      if (bad_dns)
	{
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- You have a bad character in your hostname",
		      me.name, client_p->name);
	  strcpy (user->host, source_p->hostip);
	  strcpy (source_p->sockhost, source_p->hostip);
	}
#ifdef INET6
      /* time to start work =) - Againaway */
      p = user->host;
      if (strstr (p, "::ffff:"))
	{
	  p = strrchr (user->host, ':');
	  strncpyzt (user->host, p, HOSTIPLEN + 1);
	  p = user->host;
	  p2 = strchr (p, ':');
	  p = p2 + 1;
	  *p2 = '\0';
	  strncpyzt (user->host, p, HOSTIPLEN + 1);
	  strncpyzt (source_p->sockhost, p, HOSTIPLEN + 1);
	}
#endif
      pwaconf = source_p->confs->value.aconf;

      if (!BadPtr (pwaconf->passwd)
	  && !StrEq (source_p->passwd, pwaconf->passwd))
	{
	  ircstp->is_ref++;
	  sendto_one (source_p, err_str (ERR_PASSWDMISMATCH), me.name,
		      parv[0]);
	  return exit_client (client_p, source_p, &me, "Bad Password");
	}


      /*
       ** Check for cloners
       */
      if (CLONE_PROTECTION == 1)
	{
	  if (check_clones (client_p, source_p) == -1)
	    {
	      ircsprintf (buf,
			  "Cloning is not permitted on this network. Max %d connections per host",
			  CLONE_LIMIT);
	      return exit_client (client_p, source_p, &me, buf);
	    }
	}

      if ((Count.local >= (MAXCLIENTS - 10)) && !(find_fline (source_p)))
	{
	  sendto_snomask (SNOMASK_REJECTS, "Too many clients, rejecting %s[%s].",
			      nick, source_p->sockhost);
	  ircstp->is_ref++;
	  ircsprintf (buf,
		      "Sorry, server is full - Connect to %s or try later",
		      DEFSERV);
	  return exit_client (client_p, source_p, &me, buf);
	}

#ifdef ANTI_SPAMBOT
      /*
       * It appears, this is catching normal clients 
       */
      /*
       * Reject single char user-given user->host's 
       */
      if (spamchar == 'x')
	{
	  sendto_snomask (SNOMASK_REJECTS,
			      "Rejecting possible Spambot: %s (Single char user-given userhost: %c)",
			      get_client_name (source_p, FALSE), spamchar);
	  ircstp->is_ref++;
	  return exit_client (client_p, source_p, source_p,
			      "Spambot detected, rejected.");
	}
#endif


      if (oldstatus == STAT_MASTER && MyConnect (source_p))
	(void) m_oper (&me, source_p, 1, parv);

      /* hostile username checks begin here */

      {
	char *tmpstr;
	u_char c, cc;
	int lower, upper, special;

	lower = upper = special = cc = 0;

	/*
	 * check for "@" in identd reply -Taner 
	 */
	if ((strchr (user->username, '@') != NULL)
	    || (strchr (username, '@') != NULL))
	  {
	    sendto_snomask (SNOMASK_REJECTS,
				"Illegal \"@\" in username: %s (%s)",
				get_client_name (source_p, FALSE), username);
	    ircstp->is_ref++;

	    (void) ircsprintf (tmpstr2,
			       "Invalid username [%s] - '@' is not allowed!",
			       username);
	    return exit_client (client_p, source_p, source_p, tmpstr2);
	  }
	/*
	 * First check user->username...
	 */
#ifdef IGNORE_FIRST_CHAR
	tmpstr = (user->username[0] == '~' ? &user->username[2] :
		  &user->username[1]);
	/*
	 * Ok, we don't want to TOTALLY ignore the first character. We
	 * should at least check it for control characters, etc -
	 * ThemBones
	 */
	cc = (user->username[0] == '~' ? user->username[1] :
	      user->username[0]);
	if ((!isalnum (cc) && !strchr (" -_.", cc)) || (cc > 127))
	  special++;
#else
	tmpstr = (user->username[0] == '~' ? &user->username[1] :
		  user->username);
#endif /* IGNORE_FIRST_CHAR */

	while (*tmpstr)
	  {
	    c = *(tmpstr++);
	    if (islower (c))
	      {
		lower++;
		continue;
	      }
	    if (isupper (c))
	      {
		upper++;
		continue;
	      }
	    if ((!isalnum (c) && !strchr (" -_.", c)) || (c > 127)
		|| (c < 32))
	      special++;
	  }
#ifdef NO_MIXED_CASE
	if (lower && upper)
	  {
	    sendto_snomask (SNOMASK_REJECTS, "Invalid username: %s (%s@%s)",
				nick, user->username, user->host);
	    ircstp->is_ref++;
	    (void) ircsprintf (tmpstr2, "Invalid username [%s]",
			       user->username);
	    return exit_client (client_p, source_p, &me, tmpstr2);
	  }
#endif /* NO_MIXED_CASE */
	if (special)
	  {
	    sendto_snomask (SNOMASK_REJECTS, "Invalid username: %s (%s@%s)",
				nick, user->username, user->host);
	    ircstp->is_ref++;
	    (void) ircsprintf (tmpstr2, "Invalid username [%s]",
			       user->username);
	    return exit_client (client_p, source_p, &me, tmpstr2);
	  }
	/* Ok, now check the username they provided, if different */
	lower = upper = special = cc = 0;

	if (strcmp (user->username, username))
	  {

#ifdef IGNORE_FIRST_CHAR
	    tmpstr = (username[0] == '~' ? &username[2] : &username[1]);
	    /*
	     * Ok, we don't want to TOTALLY ignore the first character.
	     * We should at least check it for control charcters, etc
	     * -ThemBones
	     */
	    cc = (username[0] == '~' ? username[1] : username[0]);

	    if ((!isalnum (cc) && !strchr (" -_.", cc)) || (cc > 127))
	      special++;
#else
	    tmpstr = (username[0] == '~' ? &username[1] : username);
#endif /* IGNORE_FIRST_CHAR */
	    while (*tmpstr)
	      {
		c = *(tmpstr++);
		if (islower (c))
		  {
		    lower++;
		    continue;
		  }
		if (isupper (c))
		  {
		    upper++;
		    continue;
		  }
		if ((!isalnum (c) && !strchr (" -_.", c)) || (c > 127))
		  special++;
	      }
#ifdef NO_MIXED_CASE
	    if (lower && upper)
	      {
		sendto_snomask (SNOMASK_REJECTS, "Invalid username: %s (%s@%s)",
				    nick, username, user->host);
		ircstp->is_ref++;
		(void) ircsprintf (tmpstr2, "Invalid username [%s]",
				   username);
		return exit_client (client_p, source_p, &me, tmpstr2);
	      }
#endif /* NO_MIXED_CASE */
	    if (special)
	      {
		sendto_snomask (SNOMASK_REJECTS, "Invalid username: %s (%s@%s)",
				    nick, username, user->host);
		ircstp->is_ref++;
		(void) ircsprintf (tmpstr2, "Invalid username [%s]",
				   username);
		return exit_client (client_p, source_p, &me, tmpstr2);
	      }
	  }			/* usernames different  */
      }

      /*
       * reject single character usernames which aren't alphabetic i.e.
       * reject jokers who have '?@somehost' or '.@somehost'
       * 
       * -Dianora
       */

      if ((user->username[1] == '\0') && !isalpha (user->username[0]))
	{
	  sendto_snomask (SNOMASK_REJECTS, "Invalid username: %s (%s@%s)",
			      nick, user->username, user->host);
	  ircstp->is_ref++;
	  (void) ircsprintf (tmpstr2, "Invalid username [%s]",
			     user->username);
	  return exit_client (client_p, source_p, &me, tmpstr2);
	}

      if (is_a_drone (source_p))
	{
#ifdef THROTTLE_ENABLE
	  throttle_force (source_p->hostip);
#endif
	  ircstp->is_ref++;
	  ircstp->is_drone++;
	  return exit_client (client_p, source_p, &me,
			      "You match the pattern of a known trojan, please check your system. Try altering your realname and/or ident to lowercase characters as this can trigger false positives.");
	}

      if (!IsExempt (source_p))
	{
	  /*
	   * Check for gcos bans
	   */
	  if ((aconf = find_conf_name (source_p->info, CONF_GCOS)))
	    {

	      return exit_client (client_p, source_p, source_p,
				  BadPtr (aconf->
					  passwd) ?
				  "Bad GCOS: Reason unspecified" : aconf->
				  passwd);
	    }

	  if (!
	      (ban =
	       check_userbanned (source_p, UBAN_IP | UBAN_CIDR4,
				 UBAN_WILDUSER)))
	    {
	      ban = check_userbanned (source_p, UBAN_HOST, 0);
	    }
      if (ban)
        {
          char *reason, ktype[1024];
          int local;

          local = (ban->flags & UBAN_LOCAL) ? 1 : 0;
          reason = ban->reason ? ban->reason : ktype;

          ircsprintf(ktype, "You are banned from %s: %s (contact %s for more "
                "information)", local ? "this server" : IRCNETWORK_NAME,
                reason, local ? SERVER_KLINE_ADDRESS : NETWORK_KLINE_ADDRESS);
          sendto_one (source_p, "NOTICE * :*** %s", ktype);

          ircstp->is_ref++;
          ircstp->is_ref_1++;
#ifdef THROTTLE_ENABLE
          throttle_force (source_p->hostip);
#endif
          exit_client (source_p, source_p, &me, ktype);
	  return 0;
        }
      }

	gline = (aGline *) find_glined_user(source_p);
        if (gline)
        {
	  char tmpbuf[2048];
	  char tmpbuf2[2048];
          ircsprintf(tmpbuf, "banned from %s: %s (%s) (contact %s for more information)",
                IRCNETWORK_NAME, gline->reason, gline->set_on, NETWORK_KLINE_ADDRESS);
          sendto_one(source_p, ":%s NOTICE %s :*** You are %s", me.name, source_p->name, tmpbuf);
	  ircsprintf(tmpbuf2, "User is %s", tmpbuf);
	  ircstp->is_ref++;
	  ircstp->is_ref_1++;
#ifdef THROTTLE_ENABLE
	  throttle_force (source_p->hostip);
#endif
          exit_client(source_p, source_p, &me, tmpbuf2);
	  return 0;
        }
 

      /*
       * We also send global connect notices if this is enabeled in the networks settings file - ShadowMaster
       */

      sendto_snomask
	(SNOMASK_CONNECTS, "Client connecting at %s: %s (%s@%s) [%s] {%d}%s [%s]",
	 me.name, nick, user->username, user->host,
	 source_p->hostip, get_client_class (source_p),
	 IsSSL (source_p) ? " (SSL)" : "", source_p->info);

      if (GLOBAL_CONNECTS == 1)
	{
	  sendto_serv_butone (client_p,
			      ":%s GCONNECT :Client connecting at %s: %s (%s@%s) [%s] {%d}%s [%s]",
			      me.name, me.name, nick, user->username,
			      user->host, source_p->hostip, get_client_class (source_p),
			      IsSSL (source_p) ? " (SSL)" : "",
			      source_p->info);
	}

      if ((++Count.local) > Count.max_loc)
	{
	  Count.max_loc = Count.local;
	  if (!(Count.max_loc % 10))
	    sendto_ops ("New Max Local Clients: %d", Count.max_loc);
	}
      if ((NOW - Count.day) > 86400)
	{
	  Count.today = 0;
	  Count.day = NOW;
	}
      if ((NOW - Count.week) > 604800)
	{
	  Count.weekly = 0;
	  Count.week = NOW;
	}
      if ((NOW - Count.month) > 2592000)
	{
	  Count.monthly = 0;
	  Count.month = NOW;
	}
      if ((NOW - Count.year) > 31536000)
	{
	  Count.yearly = 0;
	  Count.year = NOW;
	}
      Count.today++;
      Count.weekly++;
      Count.monthly++;
      Count.yearly++;

      if (source_p->flags & FLAGS_BAD_DNS)
	sendto_snomask (SNOMASK_DEBUG,
			    "DNS lookup: %s (%s@%s) is an attempted cache polluter",
			    source_p->name, source_p->user->username,
			    source_p->user->host);

    }
  else
    strncpyzt (user->username, username, USERLEN + 1);
  SetClient (source_p);
  /*
   * Increment our total user count here
   * But do not count ULined clients.
   */

  if (!IsULine (source_p))
    {
      Count.total++;

      if (Count.total > Count.max_tot)
	{
	  Count.max_tot = Count.total;
	}

      if (IsInvisible (source_p))
	{
	  Count.invisi++;
	}
    }

  if (MyConnect (source_p))
    {
      source_p->pingval = get_client_ping (source_p);
      source_p->sendqlen = get_sendq (source_p);
#ifdef MAXBUFFERS
      /*
       * Let's try changing the socket options for the client here...
       * -Taner
       */
      reset_sock_opts (source_p->fd, 0);
      /*
       * End sock_opt hack
       */
#endif
      sendto_one (source_p, rpl_str (RPL_WELCOME), me.name, nick,
		  IRCNETWORK_NAME, nick, source_p->user->username,
		  source_p->user->host);
      /*
       * This is a duplicate of the NOTICE but see below...
       * um, why were we hiding it? they did make it on to the
       * server and all..;) -wd
       */
      sendto_one (source_p, rpl_str (RPL_YOURHOST), me.name, nick,
		  get_listener_name(source_p), version);

#ifdef IRCII_KLUDGE
      sendto_one (source_p,
		  "NOTICE %s :*** Your host is %s, running %s",
		  nick, get_listener_name(source_p), version);
#endif

      sendto_one (source_p, rpl_str (RPL_CREATED), me.name, nick, creation);

      sendto_one (source_p, rpl_str (RPL_MYINFO), me.name, parv[0], me.name,
		  version, umodestring, cmodestring, snomaskstring);

      show_isupport (source_p);

      send_lusers (source_p, source_p, 1, parv);

      if (SHORT_MOTD == 1)
	{
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- Please read the motd if you haven't read it",
		      me.name, nick);

	  sendto_one (source_p, rpl_str (RPL_MOTDSTART),
		      me.name, parv[0], me.name);
	  if ((smotd = shortmotd) == NULL)
	    {
	      sendto_one (source_p,
			  rpl_str (RPL_MOTD),
			  me.name, parv[0], "*** This is the short motd ***");
	    }
	  else
	    {
	      while (smotd)
		{
		  sendto_one (source_p, rpl_str (RPL_MOTD), me.name, parv[0],
			      smotd->line);
		  smotd = smotd->next;
		}
	    }

	  sendto_one (source_p, rpl_str (RPL_ENDOFMOTD), me.name, parv[0]);

	}
      else
	{
	  (void) send_motd (source_p, source_p, 1, parv);
	}

#ifdef LITTLE_I_LINES
      if (source_p->confs && source_p->confs->value.aconf &&
	  (source_p->confs->value.aconf->flags & CONF_FLAGS_LITTLE_I_LINE))
	{
	  SetRestricted (source_p);
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- You are in a restricted access mode",
		      me.name, nick);
	  sendto_one (source_p,
		      ":%s NOTICE %s :*** Notice -- You can not be chanopped",
		      me.name, nick);
	}
#endif

    }
  else if (IsServer (client_p))
    {
      aClient *target_p;

      if ((target_p = find_server (user->server, NULL)) &&
	  target_p->from != source_p->from)
	{
	  sendto_snomask (SNOMASK_REJECTS,
			      "Bad User [%s] :%s USER %s@%s %s, != %s[%s]",
			      client_p->name, nick, user->username,
			      user->host, user->server,
			      target_p->name, target_p->from->name);
	  sendto_one (client_p,
		      ":%s KILL %s :%s (%s != %s[%s] USER from wrong direction)",
		      me.name, source_p->name, me.name, user->server,
		      target_p->from->name, target_p->from->sockhost);
	  source_p->flags |= FLAGS_KILLED;
	  return exit_client (source_p, source_p, &me,
			      "USER server wrong direction");

	}
      /*
       * Super GhostDetect: If we can't find the server the user is
       * supposed to be on, then simply blow the user away.     -Taner
       */
      if (!target_p)
	{
	  sendto_one (client_p,
		      ":%s KILL %s :%s GHOST (no server %s on the net)",
		      me.name, source_p->name, me.name, user->server);
	  sendto_realops ("No server %s for user %s[%s@%s] from %s",
			  user->server,
			  source_p->name, user->username,
			  user->host, source_p->from->name);
	  source_p->flags |= FLAGS_KILLED;
	  return exit_client (source_p, source_p, &me, "Ghosted Client");
	}
    }

  send_umode (NULL, source_p, 0, SEND_UMODES, ubuf);
  if (!*ubuf)
    {
      ubuf[0] = '+';
      ubuf[1] = '\0';
    }

  hash_check_watch (source_p, RPL_LOGON);

  sendto_clientcapab_servs_butone (1, client_p,
				   "CLIENT %s %d %ld %s %s %s %s %s %s %lu %lu :%s",
				   nick, source_p->hopcount + 1,
				   source_p->tsinfo, ubuf, "*",
				   user->username, user->host,
				   (IsHidden (source_p) ? source_p->user->
				    virthost : "*"), user->server,
				   source_p->user->servicestamp,
				   htonl (source_p->ip.s_addr),
				   source_p->info);

  if (MyClient (source_p))
    {
      /* if the I:line doesn't have a password and the user does, send it over to NickServ */
      if (BadPtr (pwaconf->passwd) && source_p->passwd[0]
	  && (nsource_p = find_person (NICKSERV_NAME, NULL)) != NULL)
	{
	  sendto_one (nsource_p, ":%s PRIVMSG %s@%s :SIDENTIFY %s",
		      source_p->name, NICKSERV_NAME, SERVICES_SERVER,
		      source_p->passwd);
	}

      if (source_p->passwd[0])
	memset (source_p->passwd, '\0', PASSWDLEN);

      if (ubuf[1])
	send_umode (client_p, source_p, 0, ALL_UMODES, ubuf);

    }

  return 0;
}

unsigned long
my_rand ()
{
  static unsigned long s = 0, t = 0, k = 12345678;
  int i;

  if (s == 0 && t == 0)
    {
      s = (unsigned long) getpid ();
      t = (unsigned long) time (NULL);
    }
  for (i = 0; i < 12; i++)
    {
      s = (((s ^ t) << (t & 31)) | ((s ^ t) >> (31 - (t & 31)))) + k;
      k += s + t;
      t = (((t ^ s) << (s & 31)) | ((t ^ s) >> (31 - (s & 31)))) + k;
      k += s + t;
    }
  return s;
}

int
check_dccsend (aClient * from, aClient * to, char *msg)
{
  return 0;
}

/* check to see if the message has any color chars in it. */
int
msg_has_colors (char *msg)
{
  char *c = msg;

  while (*c)
    {
      if (*c == '\003' || *c == '\033')
	break;
      else
	c++;
    }

  if (*c)
    return 1;

  return 0;
}


/*
  * check target limit: message target rate limiting
  * anti spam control!
  * should only be called for local PERSONS!
  * source_p: client sending message
  * target_p: client receiving message
  *
  * return value:
  * 1: block
  * 0: do nothing
  */

#ifdef MSG_TARGET_LIMIT
int
check_target_limit (aClient * source_p, aClient * target_p)
{
  int ti;
  int max_targets;
  time_t tmin = MSG_TARGET_TIME;	/* minimum time to wait before another message can be sent */

  /* don't limit opers, people talking to themselves, or people talking to services */
  if (IsAnOper (source_p) || source_p == target_p || IsULine (target_p))
    return 0;

  max_targets =
    ((NOW - source_p->firsttime) >
     MSG_TARGET_MINTOMAXTIME) ? MSG_TARGET_MAX : MSG_TARGET_MIN;

  for (ti = 0; ti < max_targets; ti++)
    {
      if (source_p->targets[ti].cli == NULL ||	/* no client */
	  source_p->targets[ti].cli == target_p ||	/* already have this client */
	  source_p->targets[ti].sent < (NOW - MSG_TARGET_TIME)	/* haven't talked to this client in > MSG_TARGET_TIME secs */
	)
	{
	  source_p->targets[ti].cli = target_p;
	  source_p->targets[ti].sent = NOW;
	  break;
	}
      else if ((NOW - source_p->targets[ti].sent) < tmin)
	tmin = NOW - source_p->targets[ti].sent;
    }

  if (ti == max_targets)
    {
      sendto_one (source_p, err_str (ERR_TARGETTOFAST), me.name,
		  source_p->name, target_p->name, MSG_TARGET_TIME - tmin);
      source_p->since += 2;	/* penalize them 2 seconds for this! */
      source_p->num_target_errors++;

      if (source_p->last_target_complain + 60 <= NOW)
	{
	  sendto_snomask (SNOMASK_REJECTS,
			      "Target limited: %s (%s@%s) [%d failed targets]",
			      source_p->name, source_p->user->username,
			      source_p->user->host,
			      source_p->num_target_errors);
	  source_p->num_target_errors = 0;
	  source_p->last_target_complain = NOW;
	}
      return 1;
    }
  return 0;
}
#endif



/*
 * m_message (used in m_private() and m_notice()) the general
 * function to deliver MSG's between users/channels
 *
 * parv[0] = sender prefix
 * parv[1] = receiver list
 * parv[2] = message text
 *
 * massive cleanup * rev argv 6/91
 *
 */

static inline int
m_message (aClient * client_p, aClient * source_p, int parc, char *parv[],
	   int notice)
{
  aClient *target_p;
  char *s;
  int i, ret, ischan;
  aChannel *channel_p;
  char *nick, *server, *p, *cmd, *dccmsg;
  char *n;

  cmd = notice ? MSG_NOTICE : MSG_PRIVATE;

  if (parc < 2 || *parv[1] == '\0')
    {
      sendto_one (source_p, err_str (ERR_NORECIPIENT), me.name, parv[0], cmd);
      return -1;
    }

  if (parc < 3 || *parv[2] == '\0')
    {
      sendto_one (source_p, err_str (ERR_NOTEXTTOSEND), me.name, parv[0]);
      return -1;
    }

  if (MyConnect (source_p))
    {
#ifdef ANTI_SPAMBOT
#ifndef ANTI_SPAMBOT_WARN_ONLY
      /*
       * if its a spambot, just ignore it
       */
      if (source_p->join_leave_count >= MAX_JOIN_LEAVE_COUNT)
	{
	  return 0;
	}
#endif
#endif
      parv[1] = canonize (parv[1]);
    }

  for (p = NULL, nick = strtoken (&p, parv[1], ","), i = 0; nick && i < 20;
       nick = strtoken (&p, NULL, ","))
    {
      /*
       * If someone is spamming via "/msg nick1,nick2,nick3,nick4 SPAM"
       * (or even to channels) then subject them to flood control!
       * -Taner
       */
      if (i++ > 10)
	{
#ifdef NO_OPER_FLOOD
	  if (!IsAnOper (source_p) && !IsULine (source_p))
	    {
#endif
	      source_p->since += 4;
#ifdef NO_OPER_FLOOD
	    }
#endif
	}

      /*
       * channel msg?
       */
      ischan = IsChannelName (nick);
      if (ischan && (channel_p = find_channel (nick, NullChn)))
	{
	  if (!notice)
	    switch (check_for_ctcp (parv[2], NULL))
	      {
	      case CTCP_NONE:
		break;

	      case CTCP_DCCSEND:
	      case CTCP_DCC:
		sendto_one (source_p,
			    ":%s NOTICE %s :You may not send a DCC command to a channel (%s)",
			    me.name, parv[0], nick);
		continue;

	      default:
#ifdef FLUD
		if (check_for_flud (source_p, NULL, channel_p, 1))
		  {
		    return 0;
		  }
#endif
		break;
	      }
	  ret = IsULine (source_p) ? 0 : can_send (source_p, channel_p);
	  if (!ret && MyClient (source_p)
	      && (channel_p->mode.mode & MODE_NOCOLOR)
	      && msg_has_colors (parv[2]))
              strip_colour (parv[2]);

	  if (ret)
	    {
	      if (!notice)	/* We now make an effort to tell the client why he cannot send to the channel - ShadowMaster */
		{
		  if (can_send (source_p, channel_p) == MODE_MODERATED)
		    {
		      sendto_one (source_p, err_str (ERR_CANNOTSENDTOCHAN),
				  me.name, parv[0], nick,
				  "Channel is moderated");
		    }
		  else if (can_send (source_p, channel_p) == MODE_NOPRIVMSGS)
		    {
		      sendto_one (source_p, err_str (ERR_CANNOTSENDTOCHAN),
				  me.name, parv[0], nick,
				  "Channel does not accept messages from non members");
		    }
		  else if (can_send (source_p, channel_p) == MODE_BAN)
		    {
		      sendto_one (source_p, err_str (ERR_CANNOTSENDTOCHAN),
				  me.name, parv[0], nick, "You are banned");
		    }
		}
	    }
	  else
	    {
	      sendto_channel_butone (client_p, source_p, channel_p,
				     ":%s %s %s :%s", parv[0], cmd, nick,
				     parv[2]);
	    }
	  continue;

	}

      /*
       * nickname addressed?
       */
      if (!ischan && (target_p = find_person (nick, NULL)))
	{
#ifdef MSG_TARGET_LIMIT
	  /* Only check target limits for my clients */
	  if (MyClient (source_p) && check_target_limit (source_p, target_p))
	    {
	      continue;
	    }
#endif
#ifdef ANTI_DRONE_FLOOD
	  if (MyConnect (target_p) && IsClient (source_p)
	      && !IsAnOper (source_p) && !IsULine (source_p) && DRONE_TIME)
	    {
	      if ((target_p->first_received_message_time + DRONE_TIME) <
		  timeofday)
		{
		  target_p->received_number_of_privmsgs = 1;
		  target_p->first_received_message_time = timeofday;
		  target_p->drone_noticed = 0;
		}
	      else
		{
		  if (target_p->received_number_of_privmsgs > DRONE_COUNT)
		    {
		      if (target_p->drone_noticed == 0)	/* tiny FSM */
			{
#ifdef DRONE_WARNINGS
			  sendto_snomask
			    (SNOMASK_FLOOD, "from %s: Possible Drone Flooder %s [%s@%s] on %s target: %s",
			     me.name, source_p->name,
			     source_p->user->username, source_p->user->host,
			     source_p->user->server, target_p->name);
#endif
			  target_p->drone_noticed = 1;
			}
		      /* heuristic here, if target has been getting a lot
		       * of privmsgs from clients, and sendq is above halfway up
		       * its allowed sendq, then throw away the privmsg, otherwise
		       * let it through. This adds some protection, yet doesn't
		       * DOS the client.
		       * -Dianora
		       */
		      if (DBufLength (&target_p->sendQ) >
			  (get_sendq (target_p) / 2L))
			{
			  if (target_p->drone_noticed == 1)	/* tiny FSM */
			    {
			      sendto_serv_butone (client_p,
						  ":%s NETGLOBAL : ANTI_DRONE_FLOOD SendQ protection activated for %s",
						  me.name, target_p->name);

			      sendto_snomask
				(SNOMASK_FLOOD, "from %s: ANTI_DRONE_FLOOD SendQ protection activated for %s",
				 me.name, target_p->name);

			      sendto_one (target_p,
					  ":%s NOTICE %s :*** Notice -- Server drone flood protection activated for %s",
					  me.name, target_p->name,
					  target_p->name);
			      target_p->drone_noticed = 2;
			    }
			}

		      if (DBufLength (&target_p->sendQ) <=
			  (get_sendq (target_p) / 4L))
			{
			  if (target_p->drone_noticed == 2)
			    {
			      sendto_one (target_p,
					  ":%s NOTICE %s :*** Notice -- Server drone flood protection de-activated for %s",
					  me.name, target_p->name,
					  target_p->name);
			      target_p->drone_noticed = 1;
			    }
			}
		      if (target_p->drone_noticed > 1)
			return 0;
		    }
		  else
		    target_p->received_number_of_privmsgs++;
		}
	    }
#endif
#ifdef FLUD
	  if (!notice && MyFludConnect (target_p))
#else
	  if (!notice && MyConnect (target_p))
#endif
	    {

	      switch (check_for_ctcp (parv[2], &dccmsg))
		{
		case CTCP_NONE:
		  break;

		case CTCP_DCCSEND:
#ifdef FLUD
		  if (check_for_flud (source_p, target_p, NULL, 1))
		    {
		      return 0;
		    }
#endif

		  if (check_dccsend (source_p, target_p, dccmsg))
		    {
		      continue;
		    }
		  break;

		default:
#ifdef FLUD
		  if (check_for_flud (source_p, target_p, NULL, 1))
		    {
		      return 0;
		    }
#endif
		  break;
		}
	    }

	  if (!is_silenced (source_p, target_p))
	    {
	      if (!notice && MyClient (target_p) && target_p->user
		  && target_p->user->away)
		{
		  sendto_one (source_p, rpl_str (RPL_AWAY), me.name, parv[0],
			      target_p->name, target_p->user->away);
		}
	      sendto_prefix_one (target_p, source_p, ":%s %s %s :%s", parv[0],
				 cmd, nick, parv[2]);
	    }
	  continue;
	}

      /*
       ** We probably should clean this up and use a loop later - ShadowMaster
       */
      if ((nick[1] == '#' || nick[2] == '#' || nick[3] == '#'
	   || nick[4] == '#') && nick[0] != '#')
	{
	  n = (char *) strchr (nick, '#');

	  if (n && (channel_p = find_channel (n, NullChn)))
	    {
	      if (can_send (source_p, channel_p) == 0 || IsULine (source_p))
		{
		  /*** FIXME ***/

		  /* This is a major mess and needs to be rewritten to handle things properly */

		  if (strchr (nick, '!') || strchr (nick, '@')
		      || strchr (nick, '%'))
		    {
		      sendto_allchannelops_butone (client_p, source_p,
						   channel_p, ":%s %s %s :%s",
						   parv[0], cmd, nick,
						   parv[2]);
		    }
		  else if (strchr (nick, '+'))
		    {
		      /*if (strchr (nick, '+')) */
		      sendto_channelvoice_butone (client_p, source_p,
						  channel_p, ":%s %s %s :%s",
						  parv[0], cmd, nick,
						  parv[2]);
		    }
		  /*
		     if (strchr(nick, '!')) sendto_channeladmins_butone(client_p, source_p, channel_p, ":%s %s %s :%s", parv[0], cmd, nick, parv[2]);
		     if (strchr(nick, '@')) sendto_channelops_butone(client_p, source_p, channel_p, ":%s %s %s :%s", parv[0], cmd, nick, parv[2]);
		     if (strchr(nick, '%')) sendto_channelhalfops_butone(client_p, source_p, channel_p, ":%s %s %s :%s", parv[0], cmd, nick, parv[2]);
		   */
		}
	      else if (!notice)
		{
		  sendto_one (source_p, err_str (ERR_CANNOTSENDTOCHAN),
			      me.name, parv[0], n, "Must be of type NOTICE");
		}
	    }
	  else
	    {
	      sendto_one (source_p, err_str (ERR_NOSUCHNICK), me.name,
			  parv[0], n);
	    }
	  continue;
	}

      if (IsAnOper (source_p) || IsULine (source_p))
	{
	  /*
	   * the following two cases allow masks in NOTICEs
	   * (for OPERs only)
	   *
	   * Armin, 8Jun90 (gruner@informatik.tu-muenchen.de)
	   */
	  if ((*nick == '$' || *nick == '#'))
	    {
	      if (!(s = (char *) strrchr (nick, '.')))
		{
		  sendto_one (source_p, err_str (ERR_NOTOPLEVEL), me.name,
			      parv[0], nick);
		  continue;
		}
	      while (*++s)
		if (*s == '.' || *s == '*' || *s == '?')
		  {
		    break;
		  }
	      if (*s == '*' || *s == '?')
		{
		  sendto_one (source_p, err_str (ERR_WILDTOPLEVEL), me.name,
			      parv[0], nick);
		  continue;
		}
	      sendto_match_butone (IsServer (client_p) ? client_p : NULL,
				   source_p, nick + 1,
				   (*nick == '#') ? MATCH_HOST : MATCH_SERVER,
				   ":%s %s %s :%s", parv[0], cmd, nick,
				   parv[2]);
	      continue;
	    }
	}

      /*
       * user@server addressed?
       */
      if (!ischan && (server = (char *) strchr (nick, '@'))
	  && (target_p = find_server (server + 1, NULL)))
	{
	  int count = 0;

	  /* Not destined for a user on me :-( */
	  if (!IsMe (target_p))
	    {
	      sendto_one (target_p, ":%s %s %s :%s", parv[0], cmd, nick,
			  parv[2]);
	      continue;
	    }
	  *server = '\0';

	  /*
	   * Look for users which match the destination host
	   * (no host == wildcard) and if one and one only is found
	   * connected to me, deliver message!
	   */
	  target_p = find_person (nick, NULL);
	  if (server)
	    {
	      *server = '@';
	    }
	  if (target_p)
	    {
	      if (count == 1)
		{
		  sendto_prefix_one (target_p, source_p, ":%s %s %s :%s",
				     parv[0], cmd, nick, parv[2]);
		}
	      else if (!notice)
		{
		  sendto_one (source_p, err_str (ERR_TOOMANYTARGETS), me.name,
			      parv[0], nick);
		}
	    }
	  if (target_p)
	    {
	      continue;
	    }
	}
      sendto_one (source_p, err_str (ERR_NOSUCHNICK), me.name, parv[0], nick);
    }
  if ((i > 20) && source_p->user)
    {
      sendto_snomask (SNOMASK_SPY, "User %s (%s@%s) tried to msg %d users",
			  source_p->name, source_p->user->username,
			  source_p->user->host, i);
    }
  return 0;
}

/*
 * * m_private *      parv[0] = sender prefix *       parv[1] =
 * receiver list *      parv[2] = message text
 */

int
m_private (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  return m_message (client_p, source_p, parc, parv, 0);
}


/*
 * * m_notice *       parv[0] = sender prefix *       parv[1] = receiver list *
 * parv[2] = notice text
 */

int
m_notice (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  return m_message (client_p, source_p, parc, parv, 1);
}


/*
** get_mode_str
** by vmlinuz
** returns an ascii string of modes
*/
char *
get_mode_str (aClient * target_p)
{
  int flag;
  int *s;
  char *m;

  m = buf;
  *m++ = '+';
  for (s = user_modes; (flag = *s) && (m - buf < BUFSIZE - 4); s += 2)
    if ((target_p->umode & flag))
      *m++ = (char) (*(s + 1));
  *m = '\0';
  return buf;
}


/*
 * * m_whois *        parv[0] = sender prefix *       parv[1] = nickname
 * masklist
 */
int
m_whois (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  static anUser UnknownUser = {
    NULL,			/* channel */
    NULL,			/* invited */
    NULL,			/* away */
    0,				/* last */
    0,				/* joined */
    "<Unknown>",		/* user */
    "<Unknown>",		/* host */
    "<Unknown>",		/* virthost */
    "<Unknown>",		/* server */
    0,				/* servicestamp */
    NULL			/* silenced */
  };

  Link *lp;
  anUser *user;
  aClient *target_p, *a2client_p;
  aChannel *channel_p;
  char *nick, *tmp, *name;
  char *p = NULL;
  int found, len, mlen;

  static time_t last_used = 0L;

  if (parc < 2)
    {
      sendto_one (source_p, err_str (ERR_NONICKNAMEGIVEN), me.name, parv[0]);
      return 0;
    }

  if (parc > 2)
    {
      if (hunt_server (client_p, source_p, ":%s WHOIS %s :%s", 1, parc, parv)
	  != HUNTED_ISME)
	return 0;
      parv[1] = parv[2];
    }

  for (tmp = parv[1]; (nick = strtoken (&p, tmp, ",")); tmp = NULL)
    {
      int invis, member, showchan;

      found = 0;
      (void) collapse (nick);
      target_p = hash_find_client (nick, (aClient *) NULL);
      if (!target_p || !IsPerson (target_p))
	{
	  sendto_one (source_p, err_str (ERR_NOSUCHNICK), me.name, parv[0],
		      nick);
	  continue;
	}

      user = target_p->user ? target_p->user : &UnknownUser;
      name = (!*target_p->name) ? "?" : target_p->name;
      invis = IsInvisible (target_p);
      member = (user->channel) ? 1 : 0;

      a2client_p = find_server (user->server, NULL);


      if (!IsAnOper (source_p) && IsAnOper (target_p))
	{
	  if ((last_used + MOTD_WAIT) > NOW)
	    return 0;
	  else
	    last_used = NOW;
	}

      sendto_one (source_p, rpl_str (RPL_WHOISUSER), me.name, parv[0], name,
		  user->username,
		  IsHidden (target_p) ? user->virthost : user->host,
		  target_p->info);

      if (IsRegNick (target_p))
	sendto_one (source_p, rpl_str (RPL_WHOISREGNICK), me.name, parv[0],
		    name);

      if ((source_p->oflag & OFLAG_AUSPEX) || source_p == target_p)
	{
	  sendto_one (source_p, rpl_str (RPL_WHOISMODES),
		      me.name, parv[0], name, get_mode_str (target_p));
	}

      /*
       * send the hostname/ip if the operator has the auspex permission.
       *      -nenolod
       */
      if (IsHidden (target_p)
	  && ((source_p->oflag & OFLAG_AUSPEX) || source_p == target_p))
	{
	  sendto_one (source_p, rpl_str (RPL_WHOISHOST), me.name, parv[0],
		      name, target_p->user->host, target_p->hostip);
	}


      mlen = strlen (me.name) + strlen (parv[0]) + 6 + strlen (name);
      for (len = 0, *buf = '\0', lp = user->channel; lp; lp = lp->next)
	{
	  channel_p = lp->value.channel_p;
	  showchan = ShowChannel (source_p, channel_p);
	  if ((showchan || (source_p->oflag & OFLAG_AUSPEX)) && 
		!IsULine (target_p))
	    {
	      if (len + strlen (channel_p->chname) >
		  (size_t) BUFSIZE - 4 - mlen)
		{
		  sendto_one (source_p,
			      ":%s %d %s %s :%s",
			      me.name, RPL_WHOISCHANNELS, parv[0], name, buf);
		  *buf = '\0';
		  len = 0;
		}

	      if (is_chan_op (target_p, channel_p))	/* there a channel operator --Acidic32 */
		*(buf + len++) = '@';
	      if (is_half_op (target_p, channel_p))	/* there a half operator --Acidic32 */
		*(buf + len++) = '%';
	      else if (has_voice (target_p, channel_p))	/* there a voice in the chan --Acidic32 */
		*(buf + len++) = '+';
	      if (len)
		*(buf + len) = '\0';
	      (void) strcpy (buf + len, channel_p->chname);
	      len += strlen (channel_p->chname);
	      (void) strcat (buf + len, " ");
	      len++;
	    }
	}
      if (buf[0] != '\0')
	sendto_one (source_p, rpl_str (RPL_WHOISCHANNELS),
		    me.name, parv[0], name, buf);

      sendto_one (source_p, rpl_str (RPL_WHOISSERVER),
		  me.name, parv[0], name, user->server,
		  a2client_p ? a2client_p->info : "*Not On This Net*");

      if (IsSSLClient (target_p))
	sendto_one (source_p, rpl_str (RPL_USINGSSL), me.name, parv[0], name);

      if (user->away)
	sendto_one (source_p, rpl_str (RPL_AWAY), me.name,
		    parv[0], name, user->away);
      if (IsAnOper (target_p))
	{
	  buf[0] = '\0';
	      sendto_one (source_p, rpl_str (RPL_WHOISOPERATOR),
			  me.name, parv[0], name, "an IRC Operator");
	}

	if (MyConnect(target_p)) {
	  sendto_one (source_p, rpl_str (RPL_WHOISIDLE),
		      me.name, parv[0], name,
		      timeofday - target_p->user->last, target_p->firsttime);
	}
      continue;
      if (!found)
	sendto_one (source_p, err_str (ERR_NOSUCHNICK), me.name, parv[0],
		    nick);
      if (p)
	p[-1] = ',';
    }
  sendto_one (source_p, rpl_str (RPL_ENDOFWHOIS), me.name, parv[0], parv[1]);

  return 0;
}

/*
 * * m_user * parv[0] = sender prefix *       parv[1] = username
 * (login name, account) *      parv[2] = client host name (used only
 * from other servers) *        parv[3] = server host name (used only
 * from other servers) *        parv[4] = users real name info
 */
int
m_user (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
#define	UFLAGS	(UMODE_i|UMODE_w)
  char *username, *host, *server, *realname;

  if (parc > 2 && (username = (char *) strchr (parv[1], '@')))
    *username = '\0';
  if (parc < 5 || *parv[1] == '\0' || *parv[2] == '\0' ||
      *parv[3] == '\0' || *parv[4] == '\0')
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
		  me.name, parv[0], "USER");
      if (IsServer (client_p))
	sendto_realops ("bad USER param count for %s from %s",
			parv[0], get_client_name (client_p, FALSE));
      else
	return 0;
    }
  /* Copy parameters into better documenting variables */
  username = (parc < 2 || BadPtr (parv[1])) ? "<bad-boy>" : parv[1];
  host = (parc < 3 || BadPtr (parv[2])) ? "<nohost>" : parv[2];
  server = (parc < 4 || BadPtr (parv[3])) ? "<noserver>" : parv[3];
  realname = (parc < 5 || BadPtr (parv[4])) ? "<bad-realname>" : parv[4];

  return do_user (parv[0], client_p, source_p, username, host, server, 0, 0,
		  realname, "*");
}

/*
 * * do_user
 */
int
do_user (char *nick,
	 aClient * client_p,
	 aClient * source_p,
	 char *username,
	 char *host,
	 char *server,
	 unsigned long serviceid,
	 unsigned int ip, char *realname, char *virthost)
{
  anUser *user;

  long oflags;

  user = make_user (source_p);
  oflags = source_p->umode;

  /*
   * changed the goto into if-else...   -Taner
   * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ GOOD FOR YOU Taner!!! - Dianora
   */

  if (!MyConnect (source_p))
    {
      user->server = find_or_add (server);
      strncpyzt (user->host, host, sizeof (user->host));
      if (virthost)
	{
	  strncpyzt (user->virthost, virthost, sizeof (user->virthost));
	}
    }
  else
    {
      if (!IsUnknown (source_p))
	{
	  sendto_one (source_p, err_str (ERR_ALREADYREGISTRED), me.name,
		      nick);
	  return 0;
	}
      if (DEF_MODE_i == 1)
	source_p->umode |= UMODE_i;

      if (MODE_x == 1)
	source_p->umode |= UMODE_x;

#ifdef USE_SSL
      if (IsSSL (source_p))
	SetSSLClient (source_p);
#endif


      source_p->umode |= (UFLAGS & atoi (host));
      strncpyzt (user->host, host, sizeof (user->host));
      user->server = me.name;
    }
  strncpyzt (source_p->info, realname, sizeof (source_p->info));

  source_p->user->servicestamp = serviceid;
  if (!MyConnect (source_p))
    {

#ifndef INET6
      source_p->ip.S_ADDR = ntohl (ip);
#else
      memset (source_p->ip.S_ADDR, 0x0, sizeof (struct IN_ADDR));
#endif
#ifdef THROTTLE_ENABLE
      /* add non-local clients to the throttle checker. obviously, we only
       * do this for REMOTE clients!@$$@! throttle_check() is called
       * elsewhere for the locals! -wd */
      if (ip != 0)
	throttle_check (inet_ntop (AFINET, (char *) &source_p->ip,
				   mydummy, sizeof (mydummy)), -1,
			source_p->tsinfo);
#endif
    }
  if (MyConnect (source_p))
    {
      source_p->oflag = 0;
    }
  if (source_p->name[0])	/*
				 * NICK already received, now I have USER...
				 */
    return register_user (client_p, source_p, source_p->name, username);
  else
    strncpyzt (source_p->user->username, username, USERLEN + 1);
  return 0;
}

/*
 * * m_quit * parv[0] = sender prefix *       parv[1] = comment
 */
int
m_quit (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  char *reason = (parc > 1 && parv[1]) ? parv[1] : client_p->name;
  char comment[TOPICLEN + 1];

  source_p->flags |= FLAGS_NORMALEX;
  if (!IsServer (client_p))
    {
#ifdef ANTI_SPAM_EXIT_MESSAGE
      if (MyConnect (source_p)
	  && (source_p->firsttime + ANTI_SPAM_EXIT_MESSAGE_TIME) > timeofday)
	strcpy (comment, "Client exited");
      else
	{
#endif
	  strcpy (comment, "Quit: ");
	  strncpy (comment + 6, reason, TOPICLEN - 6);
	  comment[TOPICLEN] = 0;
#ifdef ANTI_SPAM_EXIT_MESSAGE
	}
#endif
      return exit_client (client_p, source_p, source_p, comment);
    }
  else
    return exit_client (client_p, source_p, source_p, reason);
}

/*
 * * m_kill * parv[0] = sender prefix *       parv[1] = kill victim *
 * parv[2] = kill path
 *
 * Hidden hostmasks are now used, to stop users who are +sk seeing users hosts
 */
int
m_kill (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aClient *target_p;
  char *user, *path, *killer, *p, *nick;
  char mypath[TOPICLEN + 1];
  char *unknownfmt = "<Unknown>";	/*
					 * AFAIK this shouldnt happen
					 * * but -Raist 
					 */
  int chasing = 0, kcount = 0;

  if (parc < 2 || *parv[1] == '\0')
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
		  me.name, parv[0], "KILL");
      return 0;
    }

  user = parv[1];
  path = parv[2];		/*
				 * Either defined or NULL (parc >= 2!!)
				 */
  if (path == NULL)
    path = ")";

  if (!IsPrivileged (client_p))
    {
      sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
    }
  if (IsAnOper (client_p))
    {
      if (!BadPtr (path))
	if (strlen (path) > (size_t) TOPICLEN)
	  path[TOPICLEN] = '\0';
    }
  if (MyClient (source_p))
    user = canonize (user);
  for (p = NULL, nick = strtoken (&p, user, ","); nick;
       nick = strtoken (&p, NULL, ","))
    {
      chasing = 0;
      if (!(target_p = find_client (nick, NULL)))
	{
	  /*
	   * * If the user has recently changed nick, we automaticly *
	   * rewrite the KILL for this new nickname--this keeps *
	   * servers in synch when nick change and kill collide
	   */
	  if (!(target_p = get_history (nick, (long) KILLCHASETIMELIMIT)))
	    {
	      sendto_one (source_p, err_str (ERR_NOSUCHNICK),
			  me.name, parv[0], nick);
	      return 0;
	    }
	  sendto_one (source_p, ":%s NOTICE %s :KILL changed from %s to %s",
		      me.name, parv[0], nick, target_p->name);
	  chasing = 1;
	}

      if ((!MyConnect (target_p) && MyClient (client_p)
	   && !(client_p->oflag & OFLAG_GKILL)) || (MyConnect (target_p)
					  && MyClient (client_p)
					  && !(client_p->oflag & OFLAG_LKILL)))
	{
	  sendto_one (source_p, err_str (ERR_NOPRIVILEGES), me.name, parv[0]);
	  continue;
	}

      if (IsServer (target_p) || IsMe (target_p) ||
	  (MyClient (source_p) && IsULine (target_p)))
	{
	  sendto_one (source_p, err_str (ERR_CANTKILLSERVER), me.name,
		      parv[0]);
	  continue;
	}

      kcount++;
      if (!IsServer (source_p) && (kcount > MAXKILLS))
	{
	  sendto_one (source_p,
		      ":%s NOTICE %s :Too many targets, kill list was truncated. Maximum is %d.",
		      me.name, source_p->name, MAXKILLS);
	  break;
	}

      if (MyClient (source_p))
	{
	  char myname[HOSTLEN + 1], *s;
	  int slen = 0;

	  strncpy (myname, me.name, HOSTLEN + 1);
	  if ((s = index (myname, '.')))
/*	    *s = 0;*/

	    /* "<myname>!<source_p->user->host>!<source_p->name> (path)" */
	    slen =
	      TOPICLEN - (strlen (source_p->name) +
			  (IsHidden (source_p)
			   ? (strlen (source_p->user->virthost))
			   : (strlen (source_p->user->host))) +
			  strlen (myname) + 8);
	  if (slen < 0)
	    slen = 0;

	  if (strlen (path) > slen)
	    path[slen] = '\0';

	  ircsprintf (mypath, "%s!%s!%s (%s)", myname,
		      (IsHidden (source_p) ? source_p->user->
		       virthost : source_p->user->host), source_p->name,
		      path);
	  mypath[TOPICLEN] = '\0';
	}
      else
	strncpy (mypath, path, TOPICLEN + 1);

	sendto_snomask (SNOMASK_KILLS,
			    "Received KILL message for %s!%s@%s. From %s Path: %s",
			    target_p->name,
			    target_p->user ? target_p->user->
			    username : unknownfmt,
			    target_p->
			    user
			    ? ((IsHidden (target_p) ? target_p->user->
				virthost : target_p->user->
				host)) : unknownfmt, parv[0], mypath);

#if defined(USE_SYSLOG) && defined(SYSLOG_KILL)
      if (IsOper (source_p))
	syslog (LOG_INFO, "KILL From %s!%s@%s For %s Path %s",
		parv[0], target_p->name,
		target_p->user ? target_p->user->username : unknownfmt,
		target_p->user ? target_p->user->host : unknownfmt, mypath);
#endif
      /*
       * * And pass on the message to other servers. Note, that if KILL *
       * was changed, the message has to be sent to all links, also *
       * back. * Suicide kills are NOT passed on --SRB
       */
      if (!MyConnect (target_p) || !MyConnect (source_p)
	  || !IsAnOper (source_p))
	{
	  sendto_serv_butone (client_p, ":%s KILL %s :%s",
			      parv[0], target_p->name, mypath);
	  if (chasing && IsServer (client_p))
	    sendto_one (client_p, ":%s KILL %s :%s",
			me.name, target_p->name, mypath);
	  target_p->flags |= FLAGS_KILLED;
	}
      /*
       * * Tell the victim she/he has been zapped, but *only* if * the
       * victim is on current server--no sense in sending the *
       * notification chasing the above kill, it won't get far * anyway
       * (as this user don't exist there any more either)
       */
      if (MyConnect (target_p))
	sendto_prefix_one (target_p, source_p, ":%s KILL %s :%s",
			   parv[0], target_p->name, mypath);
      /*
       * * Set FLAGS_KILLED. This prevents exit_one_client from sending *
       * the unnecessary QUIT for this. ,This flag should never be *
       * set in any other place...
       */
	{
	  killer = strchr (mypath, '(');
	  if (killer == NULL)
	    killer = "()";
	  (void) ircsprintf (buf2, "Killed (%s %s)", source_p->name, killer);
	}
      if (exit_client (client_p, target_p, source_p, buf2) == FLUSH_BUFFER)
	return FLUSH_BUFFER;
    }
  return 0;
}

/***********************************************************************
	 * m_away() - Added 14 Dec 1988 by jto.
 *            Not currently really working, I don't like this
 *            call at all...
 *
 *            ...trying to make it work. I don't like it either,
 *	      but perhaps it's worth the load it causes to net.
 *	      This requires flooding of the whole net like NICK,
 *	      USER, MODE, etc messages...  --msa
 *
 * 	      Added FLUD-style limiting for those lame scripts out there.
 ***********************************************************************/
/*
 * * m_away * parv[0] = sender prefix *       parv[1] = away message
 */
int
m_away (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  char *away, *awy2 = parv[1];
  /*
   * make sure the user exists
   */
  if (!(source_p->user))
    {
      sendto_snomask (SNOMASK_DEBUG, "Got AWAY from nil user, from %s (%s)\n",
			  client_p->name, source_p->name);
      return 0;
    }

  away = source_p->user->away;

#ifdef NO_AWAY_FLUD
  if (MyClient (source_p))
    {
      if ((source_p->alas + MAX_AWAY_TIME) < NOW)
	{
	  source_p->acount = 0;
	}
      source_p->alas = NOW;
      source_p->acount++;
    }
#endif

  if (parc < 2 || !*awy2)
    {
      /*
       * Marking as not away
       */

      if (away)
	{
	  MyFree (away);
	  source_p->user->away = NULL;
	  /* Don't spam unaway unless they were away - lucas */
	  sendto_serv_butone (client_p, ":%s AWAY", parv[0]);
	}

      if (MyConnect (source_p))
	{
	  sendto_one (source_p, rpl_str (RPL_UNAWAY), me.name, parv[0]);
	}
      return 0;
    }

  /*
   * Marking as away
   */
#ifdef NO_AWAY_FLUD
  /* we dont care if they are just unsetting away, hence this is here */
  /* only care about local non-opers */
  if (MyClient (source_p) && (source_p->acount > MAX_AWAY_COUNT)
      && !IsAnOper (source_p))
    {
      sendto_one (source_p, err_str (ERR_TOOMANYAWAY), me.name, parv[0]);
      return 0;
    }
#endif
  if (strlen (awy2) > (size_t) TOPICLEN)
    {
      awy2[TOPICLEN] = '\0';
    }
  /*
   * some lamers scripts continually do a /away, hence making a lot of
   * unnecessary traffic. *sigh* so... as comstud has done, I've
   * commented out this sendto_serv_butone() call -Dianora
   * readded because of anti-flud stuffs -epi
   */

  sendto_serv_butone (client_p, ":%s AWAY :%s ", parv[0], parv[1]);

  if (away)
    {
      MyFree (away);
    }
  away = (char *) MyMalloc (strlen (awy2) + 1);
  strcpy (away, awy2);

  source_p->user->away = away;

  if (MyConnect (source_p))
      sendto_one (source_p, rpl_str (RPL_NOWAWAY), me.name, parv[0]);
  return 0;
}

/*
 * * m_ping * parv[0] = sender prefix *       parv[1] = origin *
 * parv[2] = destination
 */
int
m_ping (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aClient *target_p;
  char *origin, *destination;

  if (parc < 2 || *parv[1] == '\0')
    {
      sendto_one (source_p, err_str (ERR_NOORIGIN), me.name, parv[0]);
      return 0;
    }
  origin = parv[1];
  destination = parv[2];	/*
				 * Will get NULL or pointer (parc >=
				 * * 2!!) 
				 */

  target_p = find_client (origin, NULL);
  if (!target_p)
    target_p = find_server (origin, NULL);
  if (target_p && target_p != source_p)
    origin = client_p->name;
  if (!BadPtr (destination) && (irccmp (destination, me.name) != 0))
    {
      if ((target_p = find_server (destination, NULL)))
	sendto_one (target_p, ":%s PING %s :%s", parv[0], origin,
		    destination);
      else
	{
	  sendto_one (source_p, err_str (ERR_NOSUCHSERVER),
		      me.name, parv[0], destination);
	  return 0;
	}
    }
  else
    sendto_one (source_p, ":%s PONG %s :%s", me.name,
		(destination) ? destination : me.name, origin);
  return 0;
}

/*
 * * m_pong * parv[0] = sender prefix *       parv[1] = origin *
 * parv[2] = destination
 */
int
m_pong (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aClient *target_p;
  char *origin, *destination;

  if (parc < 2 || *parv[1] == '\0')
    {
      sendto_one (source_p, err_str (ERR_NOORIGIN), me.name, parv[0]);
      return 0;
    }

  origin = parv[1];
  destination = parv[2];
  client_p->flags &= ~FLAGS_PINGSENT;
  source_p->flags &= ~FLAGS_PINGSENT;

  /* if it's my client and it's a server.. */
  if (source_p == client_p && IsServer (client_p))
    {
      if (source_p->flags & FLAGS_USERBURST)
	{
	  source_p->flags &= ~FLAGS_USERBURST;
	  sendto_snomask
	    (SNOMASK_NETINFO, "from %s: %s has processed user/channel burst, sending topic burst.",
	     me.name, source_p->name);
	  send_topic_burst (source_p);
	  source_p->flags |= FLAGS_PINGSENT | FLAGS_SOBSENT;
	  sendto_one (source_p, "PING :%s", me.name);
	}
      else if (source_p->flags & FLAGS_TOPICBURST)
	{
	  source_p->flags &= ~FLAGS_TOPICBURST;
	  sendto_snomask
	    (SNOMASK_NETINFO, "from %s: %s has processed topic burst (synched to network data).",
	     me.name, source_p->name);
	  if (HUB == 1)
	    {
	      sendto_serv_butone (source_p,
				  ":%s GNOTICE :%s has synched to network data.",
				  me.name, source_p->name);
	    }
	  /* Kludge: Get the "sync" message on small networks immediately */
	  sendto_one (source_p, "PING :%s", me.name);
	}
    }

  if (!BadPtr (destination) && (irccmp (destination, me.name) != 0)
      && IsRegistered (source_p))
    {
      if ((target_p = find_client (destination, NULL)) ||
	  (target_p = find_server (destination, NULL)))
	sendto_one (target_p, ":%s PONG %s %s", parv[0], origin, destination);
      else
	{
	  sendto_one (source_p, err_str (ERR_NOSUCHSERVER),
		      me.name, parv[0], destination);
	  return 0;
	}
    }

#ifdef	DEBUGMODE
  else
    Debug ((DEBUG_NOTICE, "PONG: %s %s", origin,
	    destination ? destination : "*"));
#endif
  return 0;
}

/*
 * * m_oper * parv[0] = sender prefix *       parv[1] = oper name *
 * parv[2] = oper password
 */
int
m_oper (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aConfItem *aconf;
  char *name, *password, *encr, *oper_ip;

#ifdef CRYPT_OPER_PASSWORD
  extern char *crypt ();

#endif /*
        * CRYPT_OPER_PASSWORD
        */

  name = parc > 1 ? parv[1] : (char *) NULL;
  password = parc > 2 ? parv[2] : (char *) NULL;

  if (!IsServer (client_p) && (BadPtr (name) || BadPtr (password)))
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
		  me.name, parv[0], "OPER");
      return 0;
    }

  /* if message arrived from server, trust it, and set to oper */

  if ((IsServer (client_p) || IsMe (client_p)) && !IsOper (source_p))
    {

      source_p->umode |= UMODE_o;
      sendto_serv_butone (client_p, ":%s MODE %s :+o", parv[0], parv[0]);

      Count.oper++;
      if (IsMe (client_p))
	sendto_one (source_p, rpl_str (RPL_YOUREOPER), me.name, parv[0]);
      return 0;
    }
  else if (IsAnOper (source_p))
    {
      if (MyConnect (source_p))
	sendto_one (source_p, rpl_str (RPL_YOUREOPER), me.name, parv[0]);
      sendto_one (source_p,
		  ":%s NOTICE %s :*** Notice -- opermotd was last changed at %s",
		  me.name, parv[0], opermotd_last_changed_date);
      return 0;
    }
  if (!(aconf = find_conf_exact (name, source_p->username, source_p->sockhost,
				 CONF_OPS)) &&
      !(aconf = find_conf_exact (name, source_p->username,
				 client_p->hostip, CONF_OPS)))
    {
      sendto_one (source_p, err_str (ERR_NOOPERHOST), me.name, parv[0]);
      sendto_serv_butone
	(client_p,
	 ":%s NETGLOBAL : Failed OPER attempt for [%s] by %s (%s@%s) (no O:line)",
	 me.name, name, parv[0], source_p->user->username,
	 source_p->user->host);
      sendto_snomask
	(SNOMASK_NETINFO, "from %s: Failed OPER attempt for [%s] by %s (%s@%s) (no O:line)",
	 me.name, name, parv[0], source_p->user->username,
	 source_p->user->host);
      return 0;
    }
  oper_ip = source_p->hostip;
#ifdef CRYPT_OPER_PASSWORD
  /* use first two chars of the password they send in as salt */
  /* passwd may be NULL pointer. Head it off at the pass... */
  if (password && *aconf->passwd)
    encr = crypt (password, aconf->passwd);
  else
    encr = "";
#else
  encr = password;
#endif /* CRYPT_OPER_PASSWORD */

  if ((aconf->status & CONF_OPS) &&
      StrEq (encr, aconf->passwd) && !attach_conf (source_p, aconf))
    {
      int old = (source_p->umode & ALL_UMODES);
      char *s;

      s = strchr (aconf->host, '@');
      if (s == (char *) NULL)
	{
	  sendto_one (source_p, err_str (ERR_NOOPERHOST), me.name, parv[0]);
	  sendto_realops ("corrupt aconf->host = [%s]", aconf->host);
	  return 0;
	}
      *s++ = '\0';

      /*
       ** Here we check what flags the user have in his Oline then we give him usermodes and send out notices based on those flags.
       ** We also send out notices to all the opers on the entire network of users opering up. - ShadowMaster
       */

	  if (!IsHidden (source_p))
	    SetHidden (source_p);
	  make_opervirthost (source_p->user->virthost, OPERHOST);
	  sendto_serv_butone (client_p, ":%s SETHOST %s %s", me.name,
			      source_p->name, source_p->user->virthost);

	  sendto_serv_butone (client_p,
			      ":%s NETGLOBAL :Hmm, %s (%s@%s) just activated an O:Line on [%s] for [%s].",
			      me.name, parv[0], source_p->user->username,
			      source_p->user->host, me.name, name);
	  sendto_snomask
	    (SNOMASK_NETINFO, "from %s: Hmm, %s (%s@%s) just activated an O:Line on [%s] for [%s].", me.name,
	     parv[0], source_p->user->username, source_p->user->host, name);

      source_p->umode |= (UMODE_o | UMODE_w);

      source_p->snomask |= (SNOMASK_CONNECTS | SNOMASK_NETINFO | SNOMASK_KILLS | SNOMASK_GLOBOPS | SNOMASK_FLOOD | SNOMASK_SPY | SNOMASK_LINKS);
      send_snomask_out (client_p, source_p, 0);

      source_p->oflag = aconf->port;

      sendto_one (source_p, ":%s NOTICE %s :*** Your permissions are %s",
                me.name, parv[0], oflagstr(source_p->oflag));

      send_umode_out (client_p, source_p, old);

      Count.oper++;
      *--s = '@';
      addto_fdlist (source_p->fd, &oper_fdlist);
#ifdef THROTTLE_ENABLE
      throttle_remove (oper_ip);
#endif
      sendto_one (source_p, rpl_str (RPL_YOUREOPER), me.name, parv[0]);
      /*** FIXME ***/
      /* Stop notice from being sent if no OperMOTD has been loaded */
      sendto_one (source_p,
		  ":%s NOTICE %s :*** Notice -- opermotd was last changed at %s",
		  me.name, parv[0], opermotd_last_changed_date);
      source_p->pingval = get_client_ping (source_p);
      source_p->sendqlen = get_sendq (source_p);

#if !defined(CRYPT_OPER_PASSWORD) && (defined(FNAME_OPERLOG) || (defined(USE_SYSLOG) && defined(SYSLOG_OPER)))
      encr = "";
#endif
#if defined(USE_SYSLOG) && defined(SYSLOG_OPER)
      syslog (LOG_INFO, "OPER (%s) (%s) by (%s!%s@%s)",
	      name, encr, parv[0], source_p->user->username,
	      source_p->sockhost);
#endif
#if defined(FNAME_OPERLOG)
      {
	int logfile;

	/*
	 * This conditional makes the logfile active only after it's
	 * been created - thus logging can be turned off by removing
	 * the file.
	 *
	 * stop NFS hangs...most systems should be able to open a file in
	 * 3 seconds. -avalon (curtesy of wumpus)
	 */
	(void) alarm (3);
	if (IsPerson (source_p) &&
	    (logfile = open (FNAME_OPERLOG, O_WRONLY | O_APPEND)) != -1)
	  {
	    (void) alarm (0);
	    (void) ircsprintf (buf, "%s OPER (%s) (%s) by (%s!%s@%s)\n",
			       myctime (timeofday), name, encr,
			       parv[0], source_p->user->username,
			       source_p->sockhost);
	    (void) alarm (3);
	    (void) write (logfile, buf, strlen (buf));
	    (void) alarm (0);
	    (void) close (logfile);
	  }
	(void) alarm (0);
	/*
	 * Modification by pjg 
	 */
      }
#endif
    }
  else
    {
      (void) detach_conf (source_p, aconf);
      sendto_one (source_p, err_str (ERR_PASSWDMISMATCH), me.name, parv[0]);
#ifdef FAILED_OPER_NOTICE
      sendto_serv_butone
	(client_p,
	 ":%s NETGLOBAL : Failed OPER attempt for [%s] by %s (%s@%s) (password mismatch)",
	 me.name, name, parv[0], source_p->user->username,
	 source_p->user->host);
      sendto_snomask
	(SNOMASK_NETINFO, "from %s: Failed OPER attempt for [%s] by %s (%s@%s) (password mismatch)",
	 me.name, name, parv[0], source_p->user->username,
	 source_p->user->host);
#endif
    }
  return 0;
}

/***************************************************************************
 * m_pass() - Added Sat, 4 March 1989
 ***************************************************************************/
/*
 * * m_pass * parv[0] = sender prefix *       parv[1] = password *
 * parv[2] = optional extra version information
 */
int
m_pass (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  char *password = parc > 1 ? parv[1] : NULL;

  if (BadPtr (password))
    {
      sendto_one (client_p, err_str (ERR_NEEDMOREPARAMS),
		  me.name, parv[0], "PASS");
      return 0;
    }
  if (!MyConnect (source_p)
      || (!IsUnknown (client_p) && !IsHandshake (client_p)))
    {
      sendto_one (client_p, err_str (ERR_ALREADYREGISTRED), me.name, parv[0]);
      return 0;
    }
  strncpyzt (client_p->passwd, password, sizeof (client_p->passwd));
  if (parc > 2)
    {
      int l = strlen (parv[2]);

      if (l < 2)
	return 0;
      /*
       * if (strcmp(parv[2]+l-2, "TS") == 0) 
       */
      if (parv[2][0] == 'T' && parv[2][1] == 'S')
	client_p->tsinfo = (ts_val) TS_DOESTS;
    }
  return 0;
}

/*
 * m_userhost
 *
 * Added by Darren Reed 13/8/91 to aid clients and reduce the need for
 * complicated requests like WHOIS.
 *
 * Returns user/host information only (no spurious AWAY labels or channels).
 *
 * Rewritten by Thomas J. Stensås (ShadowMaster) 15/10/02 to clean it up a bit aswell as
 * add hiddenhost support.
 */
int
m_userhost (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aClient *target_p;
  char *s, *p = NULL;
  int i, j = 0, len = 0;

  if (parc < 2)
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS), me.name, parv[0],
		  "USERHOST");
      return 0;
    }

  *buf2 = '\0';

  for (i = 5, s = strtoken (&p, parv[1], " "); i && s;
       s = strtoken (&p, (char *) NULL, " "), i--)
    {
      if ((target_p = find_person (s, NULL)))
	{
	  if (j == 0)
	    {
	      /* Always show realhost to self as some clients depend on this */
	      if (MyClient (source_p) && (source_p == target_p))
		{
		  ircsprintf (buf, "%s%s=%c%s@%s",
			      target_p->name,
			      IsOper (target_p) ? "*" : "",
			      (target_p->user->away) ? '-' : '+',
			      target_p->user->username, target_p->user->host);

		}
	      else
		{
		  ircsprintf (buf, "%s%s=%c%s@%s",
			      target_p->name,
			      IsAnOper (target_p) ? "*" : "",
			      (target_p->user->away) ? '-' : '+',
			      target_p->user->username,
			      (IsHidden (target_p) ? target_p->user->
			       virthost : target_p->user->host));
		}
	      j++;
	    }
	  else
	    {
	      if (j == 1)
		{
		  len = strlen (buf);
		}
	      strcat (buf, " ");
	      /* Always show realhost to self as some clients depend on this */
	      if (MyClient (source_p) && (source_p == target_p))
		{
		  ircsprintf (buf2, "%s%s=%c%s@%s",
			      target_p->name,
			      IsOper (target_p) ? "*" : "",
			      (target_p->user->away) ? '-' : '+',
			      target_p->user->username, target_p->user->host);

		}
	      else
		{
		  ircsprintf (buf2, "%s%s=%c%s@%s",
			      target_p->name,
			      IsAnOper (target_p) ? "*" : "",
			      (target_p->user->away) ? '-' : '+',
			      target_p->user->username,
			      (IsHidden (target_p) ? target_p->user->
			       virthost : target_p->user->host));
		}
	      strncat (buf, buf2, sizeof (buf) - len);
	      len += strlen (buf2);
	      j++;
	    }
	}
      else
	{
	  sendto_one (source_p, err_str (ERR_NOSUCHNICK), me.name, parv[0],
		      s);
	}
    }
  if (j)
    {
      sendto_one (source_p, rpl_str (RPL_USERHOST), me.name, parv[0], buf);
    }

  /* We have a userhost for more than 1 client, and one or more needs to return realhost */
  if (j)
    {
      for (i = 5, s = strtoken (&p, parv[1], " "); i && s;
	   s = strtoken (&p, (char *) NULL, " "), i--)
	{
	  if ((target_p = find_person (s, NULL)))
	    {
	      if (IsHidden (target_p) && IsAnOper (source_p)
		  && (source_p != target_p))
		{
		  sendto_one (source_p, rpl_str (RPL_WHOISHOST), me.name,
			      parv[0], target_p->name, target_p->user->host,
			      target_p->hostip);
		}
	    }
	}
    }
  return 0;
}

/*
 * m_ison added by Darren Reed 13/8/91 to act as an efficent user
 * indicator with respect to cpu/bandwidth used. Implemented for NOTIFY
 * feature in clients. Designed to reduce number of whois requests. Can
 * process nicknames in batches as long as the maximum buffer length.
 * 
 * format: ISON :nicklist
 */
/*
 * Take care of potential nasty buffer overflow problem -Dianora
 * 
 */

int
m_ison (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  aClient *target_p;
  char *s, **pav = parv;
  char *p = (char *) NULL;
  int len;
  int len2;
  if (parc < 2)
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
		  me.name, parv[0], "ISON");
      return 0;
    }

  (void) ircsprintf (buf, rpl_str (RPL_ISON), me.name, *parv);
  len = strlen (buf);
  if (!IsOper (client_p))
    client_p->priority += 20;	/*
				 * this keeps it from moving to 'busy'
				 * * list 
				 */
  for (s = strtoken (&p, *++pav, " "); s;
       s = strtoken (&p, (char *) NULL, " "))
    if ((target_p = find_person (s, NULL)))
      {
	len2 = strlen (target_p->name);
	if ((len + len2 + 5) < sizeof (buf))
	  {			/*
				 * make sure can never
				 * * overflow  
	     *//*
	     * allow for extra ' ','\0' etc. 
	     */
	    (void) strcat (buf, target_p->name);
	    len += len2;
	    (void) strcat (buf, " ");
	    len++;
	  }
	else
	  break;
      }
  sendto_one (source_p, "%s", buf);
  return 0;
}

/*
 * m_umode() added 15/10/91 By Darren Reed. parv[0] - sender parv[1] -
 * username to change mode for parv[2] - modes to change
 */
int
m_umode (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  int flag;
  int *s;
  char **p, *m;
  aClient *target_p;
  int what, setflags;
  int badflag = NO;		/*
				 * Only send one bad flag notice
				 * * -Dianora
				 */
  what = MODE_ADD;
  if (parc < 2)
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
		  me.name, parv[0], "MODE");
      return 0;
    }

  if (!(target_p = find_person (parv[1], NULL)))
    {
      if (MyConnect (source_p))
	sendto_one (source_p, err_str (ERR_NOSUCHCHANNEL),
		    me.name, parv[0], parv[1]);
      return 0;
    }

  if ((IsServer (source_p) || (source_p != target_p)
       || (target_p->from != source_p->from)))
    {
      if (!IsServer (client_p))
	sendto_one (source_p, err_str (ERR_USERSDONTMATCH), me.name, parv[0]);
      return 0;
    }


  if (parc < 3)
    {
      m = buf;
      *m++ = '+';
      for (s = user_modes; (flag = *s) && (m - buf < BUFSIZE - 4); s += 2)
	{
	  if (source_p->umode & (flag & ALL_UMODES))
	    *m++ = (char) (*(s + 1));
	}
      *m = '\0';
      sendto_one (source_p, rpl_str (RPL_UMODEIS), me.name, parv[0], buf);
      return 0;
    }

  /*
   * find flags already set for user
   */
  setflags = 0;
  for (s = user_modes; (flag = *s); s += 2)
    if (source_p->umode & flag)
      setflags |= flag;

  /*
   * parse mode change string(s)
   */
  for (p = &parv[2]; p && *p; p++)
    for (m = *p; *m; m++)
      switch (*m)
	{
	case '+':
	  what = MODE_ADD;
	  break;
	case '-':
	  what = MODE_DEL;
	  break;
	  /*
	   * we may not get these, but they shouldnt be in
	   * default
	   */
	case ' ':
	case '\r':
	case '\n':
	case '\t':
	  break;
	case 'o':
	  if (what == MODE_ADD)
	    {
	      if (IsServer (client_p) && !IsOper (source_p))
		{
		  /*
		   * Dont ever count ULined clients as they are services
		   * of some sort.
		   */
		  if (!IsULine (source_p))
		    {
		      ++Count.oper;
		    }
		  SetOper (source_p);
		}
	    }
	  else
	    {
	      if (!IsOper (source_p))
		break;

	      ClearOper (source_p);

	      if (!IsULine (source_p))
		{
		  Count.oper--;
		}

	      if (MyConnect (source_p))
		{
		  det_confs_butmask (source_p, CONF_CLIENT & ~CONF_OPS);
		  source_p->sendqlen = get_sendq (source_p);
		  source_p->oflag = 0;
		  delfrom_fdlist (source_p->fd, &oper_fdlist);
		}

	      if (MODE_x == 0)
		{
		  source_p->umode &= ~UMODE_x;
		}
	    }
	  break;
	case 'r':
	  break;		/* users can't set themselves +r! */
	case 'x':
	  if (MyClient (source_p) && !IsServer (client_p))
	    {
	      /*
	       * This network doesnt use hiddenhosts, dont let users set themselves +x
	       */
	      if (MODE_x == 0 && what == MODE_ADD)
		{
		  break;
		}
	      /*
	       * Else the network uses hiddenhosts
	       * If the network wants to lock hiddenhosts on for all users
	       * dont let them set -x
	       */
	      else if (MODE_x == 1 && MODE_x_LOCK == 1 && what == MODE_DEL)
		{
		  break;
		}
	    }

	  if (what == MODE_ADD)
	    {
	      SetHidden (source_p);
	    }
	  else
	    {
	      ClearHidden (source_p);
	    }
	  break;
	default:
	  if ((flag = user_mode_table[(unsigned char) *m]))
	    {
	      if (what == MODE_ADD)
		{
		  source_p->umode |= flag;
		}
	      else
		{
		  source_p->umode &= ~flag;
		}
	    }
	  else
	    {
	      if (MyConnect (source_p))
		{
		  badflag = YES;
		}
	    }
	  break;
	}

  if (badflag)
    sendto_one (source_p, err_str (ERR_UMODEUNKNOWNFLAG), me.name, parv[0]);

  send_umode_out (client_p, source_p, setflags);
  return 0;
}

/*
 * send the MODE string for user (user) to connection client_p -avalon
 */
void
send_umode (aClient * client_p,
	    aClient * source_p, int old, int sendmask, char *umode_buf)
{
  int *s, flag;
  char *m;
  int what = MODE_NULL;
  /*
   * build a string in umode_buf to represent the change in the user's
   * mode between the new (source_p->flag) and 'old'.
   */
  m = umode_buf;
  *m = '\0';
  for (s = user_modes; (flag = *s); s += 2)
    {
      if (MyClient (source_p) && !(flag & sendmask))
	continue;
      if ((flag & old) && !(source_p->umode & flag))
	{
	  if (what == MODE_DEL)
	    *m++ = *(s + 1);
	  else
	    {
	      what = MODE_DEL;
	      *m++ = '-';
	      *m++ = *(s + 1);
	    }
	}
      else if (!(flag & old) && (source_p->umode & flag))
	{
	  if (what == MODE_ADD)
	    *m++ = *(s + 1);
	  else
	    {
	      what = MODE_ADD;
	      *m++ = '+';
	      *m++ = *(s + 1);
	    }
	}
    }
  *m = '\0';
  if (*umode_buf && client_p)
    sendto_one (client_p, ":%s MODE %s :%s", source_p->name, source_p->name,
		umode_buf);
}

/*
 * added Sat Jul 25 07:30:42 EST 1992
 */
/*
 * extra argument evenTS added to send to TS servers or not -orabidoo
 * 
 * extra argument evenTS no longer needed with TS only th+hybrid server
 * -Dianora
 */
void
send_umode_out (aClient * client_p, aClient * source_p, int old)
{
  int i, j;
  aClient *target_p;
  fdlist fdl = serv_fdlist;
  send_umode (NULL, source_p, old, SEND_UMODES, buf);
  /*
   * Cycling through serv_fdlist here should be MUCH faster than
   * looping through every client looking for servers. -ThemBones
   */
  for (i = fdl.entry[j = 1]; j <= fdl.last_entry; i = fdl.entry[++j])
    {
      if ((target_p = local[i]) && (target_p != client_p)
	  && (target_p != source_p) && (*buf))
	{
	  sendto_one (target_p, ":%s MODE %s :%s", source_p->name,
		      source_p->name, buf);
	}
    }
  if (client_p && MyClient (client_p))
    {
      send_umode (client_p, source_p, old, ALL_UMODES, buf);
    }
}

/*
 * extra argument evenTS added to send to TS servers or not -orabidoo
 *
 * extra argument evenTS no longer needed with TS only th+hybrid server
 * -Dianora
 */

/*
 * This function checks to see if a CTCP message (other than ACTION) is
 * contained in the passed string.  This might seem easier than I am
 * doing it, but a CTCP message can be changed together, even after a
 * normal message.
 * 
 * If the message is found, and it's a DCC message, pass it back in
 * *dcclient_p.
 *
 * Unfortunately, this makes for a bit of extra processing in the
 * server.
 */

int
check_for_ctcp (char *str, char **dcclient_p)
{
  char *p = str;
  while ((p = strchr (p, 1)) != NULL)
    {
      if (ircncmp (++p, "DCC", 3) == 0)
	{
	  if (dcclient_p)
	    *dcclient_p = p;
	  if (ircncmp (p + 3, " SEND", 5) == 0)
	    return CTCP_DCCSEND;
	  else
	    return CTCP_DCC;
	}
      if (ircncmp (++p, "ACTION", 6) != 0)
	return CTCP_YES;
      if ((p = strchr (p, 1)) == NULL)
	return CTCP_NONE;
      if (!(*(++p)))
	break;;
    }
  return CTCP_NONE;
}

/*
 * Shadowfax's FLUD code 
 */

#ifdef FLUD

void
announce_fluder (aClient * fluder,	/*
					 * fluder, client being fluded 
					 */
		 aClient * client_p, aChannel * channel_p,	/*
								 * channel being fluded 
								 */
		 int type)
{				/*
				 * for future use 
				 */
  char *fludee;
  if (client_p)
    fludee = client_p->name;
  else
    fludee = channel_p->chname;
  sendto_snomask (SNOMASK_DEBUG,
		      "Flooder %s [%s@%s] on %s target: %s",
		      fluder->name, fluder->user->username,
		      fluder->user->host, fluder->user->server, fludee);
}

/*
 * This is really just a "convenience" function.  I can only keep three
 * or * four levels of pointer dereferencing straight in my head.  This
 * remove * an entry in a fluders list.  Use this when working on a
 * fludees list :)
 */
struct fludbot *
remove_fluder_reference (struct fludbot **fluders, aClient * fluder)
{
  struct fludbot *current, *prev, *next;
  prev = NULL;
  current = *fluders;
  while (current)
    {
      next = current->next;
      if (current->fluder == fluder)
	{
	  if (prev)
	    prev->next = next;
	  else
	    *fluders = next;
	  BlockHeapFree (free_fludbots, current);
	}
      else
	prev = current;
      current = next;
    }

  return (*fluders);
}

/*
 * Another function to unravel my mind. 
 */
Link *
remove_fludee_reference (Link ** fludees, void *fludee)
{
  Link *current, *prev, *next;
  prev = NULL;
  current = *fludees;
  while (current)
    {
      next = current->next;
      if (current->value.client_p == (aClient *) fludee)
	{
	  if (prev)
	    prev->next = next;
	  else
	    *fludees = next;
	  BlockHeapFree (free_Links, current);
	}
      else
	prev = current;
      current = next;
    }

  return (*fludees);
}

int
check_for_fludblock (aClient * fluder,	/*
					 * fluder being fluded 
					 */
		     aClient * client_p,	/*
						 * client being fluded 
						 */
		     aChannel * channel_p,	/*
						 * channel being fluded 
						 */
		     int type)
{				/*
				 * for future use 
				 */
  time_t now;
  int blocking;
  /*
   * If it's disabled, we don't need to process all of this 
   */
  if (flud_block == 0)
    return 0;
  /*
   * It's either got to be a client or a channel being fluded 
   */
  if ((client_p == NULL) && (channel_p == NULL))
    return 0;
  if (client_p && !MyFludConnect (client_p))
    {
      sendto_ops ("check_for_fludblock() called for non-local client");
      return 0;
    }

  /*
   * Are we blocking fluds at this moment? 
   */
  time (&now);
  if (client_p)
    blocking = (client_p->fludblock > (now - flud_block));
  else
    blocking = (channel_p->fludblock > (now - flud_block));
  return (blocking);
}

int
check_for_flud (aClient * fluder,	/*
					 * fluder, client being fluded 
					 */
		aClient * client_p, aChannel * channel_p,	/*
								 * channel being fluded 
								 */
		int type)
{				/*
				 * for future use 
				 */
  time_t now;
  struct fludbot *current, *prev, *next;
  int blocking, count, found;
  Link *newfludee;
  /*
   * If it's disabled, we don't need to process all of this 
   */
  if (flud_block == 0)
    return 0;
  /*
   * It's either got to be a client or a channel being fluded 
   */
  if ((client_p == NULL) && (channel_p == NULL))
    return 0;
  if (client_p && !MyFludConnect (client_p))
    {
      sendto_ops ("check_for_flud() called for non-local client");
      return 0;
    }

  /*
   * Are we blocking fluds at this moment? 
   */
  time (&now);
  if (client_p)
    blocking = (client_p->fludblock > (now - flud_block));
  else
    blocking = (channel_p->fludblock > (now - flud_block));
  /*
   * Collect the Garbage 
   */
  if (!blocking)
    {
      if (client_p)
	current = client_p->fluders;
      else
	current = channel_p->fluders;
      prev = NULL;
      while (current)
	{
	  next = current->next;
	  if (current->last_msg < (now - flud_time))
	    {
	      if (client_p)
		remove_fludee_reference (&current->fluder->fludees,
					 (void *) client_p);
	      else
		remove_fludee_reference (&current->fluder->fludees,
					 (void *) channel_p);
	      if (prev)
		prev->next = current->next;
	      else if (client_p)
		client_p->fluders = current->next;
	      else
		channel_p->fluders = current->next;
	      BlockHeapFree (free_fludbots, current);
	    }
	  else
	    prev = current;
	  current = next;
	}
    }
  /*
   * Find or create the structure for the fluder, and update the
   * counter * and last_msg members.  Also make a running total count
   */
  if (client_p)
    current = client_p->fluders;
  else
    current = channel_p->fluders;
  count = found = 0;
  while (current)
    {
      if (current->fluder == fluder)
	{
	  current->last_msg = now;
	  current->count++;
	  found = 1;
	}
      if (current->first_msg < (now - flud_time))
	count++;
      else
	count += current->count;
      current = current->next;
    }
  if (!found)
    {
      if ((current = BlockHeapALLOC (free_fludbots, struct fludbot)) != NULL)
	{
	  current->fluder = fluder;
	  current->count = 1;
	  current->first_msg = now;
	  current->last_msg = now;
	  if (client_p)
	    {
	      current->next = client_p->fluders;
	      client_p->fluders = current;
	    }
	  else
	    {
	      current->next = channel_p->fluders;
	      channel_p->fluders = current;
	    }

	  count++;
	  if ((newfludee = BlockHeapALLOC (free_Links, Link)) != NULL)
	    {
	      if (client_p)
		{
		  newfludee->flags = 0;
		  newfludee->value.client_p = client_p;
		}
	      else
		{
		  newfludee->flags = 1;
		  newfludee->value.channel_p = channel_p;
		}
	      newfludee->next = fluder->fludees;
	      fluder->fludees = newfludee;
	    }
	  else
	    outofmemory ();
	  /*
	   * If we are already blocking now, we should go ahead * and
	   * announce the new arrival
	   */
	  if (blocking)
	    announce_fluder (fluder, client_p, channel_p, type);
	}
      else
	outofmemory ();
    }
  /*
   * Okay, if we are not blocking, we need to decide if it's time to *
   * begin doing so.  We already have a count of messages received in *
   * the last flud_time seconds
   */
  if (!blocking && (count > flud_num))
    {
      blocking = 1;
      ircstp->is_flud++;
      /*
       * if we are going to say anything to the fludee, now is the *
       * time to mention it to them.
       */
      if (client_p)
	sendto_one (client_p,
		    ":%s NOTICE %s :*** Notice -- Server flood protection activated for %s",
		    me.name, client_p->name, client_p->name);
      else
	sendto_channel_butserv (channel_p, &me,
				":%s NOTICE %s :*** Notice -- Server flood protection activated for %s",
				me.name, channel_p->chname,
				channel_p->chname);
      /*
       * Here we should go back through the existing list of * fluders
       * and announce that they were part of the game as * well.
       */
      if (client_p)
	current = client_p->fluders;
      else
	current = channel_p->fluders;
      while (current)
	{
	  announce_fluder (current->fluder, client_p, channel_p, type);
	  current = current->next;
	}
    }
  /*
   * update blocking timestamp, since we received a/another CTCP
   * message
   */
  if (blocking)
    {
      if (client_p)
	client_p->fludblock = now;
      else
	channel_p->fludblock = now;
    }

  return (blocking);
}

void
free_fluders (aClient * client_p, aChannel * channel_p)
{
  struct fludbot *fluders, *next;
  if ((client_p == NULL) && (channel_p == NULL))
    {
      sendto_ops ("free_fluders(NULL, NULL)");
      return;
    }

  if (client_p && !MyFludConnect (client_p))
    return;
  if (client_p)
    fluders = client_p->fluders;
  else
    fluders = channel_p->fluders;
  while (fluders)
    {
      next = fluders->next;
      if (client_p)
	remove_fludee_reference (&fluders->fluder->fludees,
				 (void *) client_p);
      else
	remove_fludee_reference (&fluders->fluder->fludees,
				 (void *) channel_p);
      BlockHeapFree (free_fludbots, fluders);
      fluders = next;
    }
}

void
free_fludees (aClient * badguy)
{
  Link *fludees, *next;
  if (badguy == NULL)
    {
      sendto_ops ("free_fludees(NULL)");
      return;
    }
  fludees = badguy->fludees;
  while (fludees)
    {
      next = fludees->next;
      if (fludees->flags)
	remove_fluder_reference (&fludees->value.channel_p->fluders, badguy);
      else
	{
	  if (!MyFludConnect (fludees->value.client_p))
	    sendto_ops ("free_fludees() encountered non-local client");
	  else
	    remove_fluder_reference (&fludees->value.client_p->fluders,
				     badguy);
	}

      BlockHeapFree (free_Links, fludees);
      fludees = next;
    }
}
#endif /*
        * FLUD 
        */


/* is_silenced - Returns 1 if a source_p is silenced by target_p */
static int
is_silenced (aClient * source_p, aClient * target_p)
{
  Link *lp;
  anUser *user;
  char sender[HOSTLEN + NICKLEN + USERLEN + 5];
  if (!(target_p->user) || !(lp = target_p->user->silence)
      || !(user = source_p->user))
    {
      return 0;
    }
  ircsprintf (sender, "%s!%s@%s", source_p->name, user->username, user->host);
  for (; lp; lp = lp->next)
    {
      if (!match (lp->value.cp, sender))
	{
	  if (!MyConnect (source_p))
	    {
	      sendto_one (source_p->from, ":%s SILENCE %s :%s",
			  target_p->name, source_p->name, lp->value.cp);
	      lp->flags = 1;
	    }
	  return 1;
	}
    }
  /*
   * If the source has a hiddenhost.. we need to check against the hiddenhost aswell.
   */
  if (IsHidden (source_p))
    {
      ircsprintf (sender, "%s!%s@%s", source_p->name, user->username,
		  user->virthost);
      for (; lp; lp = lp->next)
	{
	  if (!match (lp->value.cp, sender))
	    {
	      if (!MyConnect (source_p))
		{
		  sendto_one (source_p->from, ":%s SILENCE %s :%s",
			      target_p->name, source_p->name, lp->value.cp);
		  lp->flags = 1;
		}
	      return 1;
	    }
	}
    }

  return 0;
}

int
del_silence (aClient * source_p, char *mask)
{
  Link **lp, *tmp;
  for (lp = &(source_p->user->silence); *lp; lp = &((*lp)->next))
    if (irccmp (mask, (*lp)->value.cp) == 0)
      {
	tmp = *lp;
	*lp = tmp->next;
	MyFree (tmp->value.cp);
	free_link (tmp);
	return 0;
      }
  return 1;
}

static int
add_silence (aClient * source_p, char *mask)
{
  Link *lp;
  int cnt = 0, len = 0;
  for (lp = source_p->user->silence; lp; lp = lp->next)
    {
      len += strlen (lp->value.cp);
      if (MyClient (source_p))
	{
	  if ((len > MAXSILELENGTH) || (++cnt >= MAXSILES))
	    {
	      sendto_one (source_p, err_str (ERR_SILELISTFULL), me.name,
			  source_p->name, mask);
	      return -1;
	    }
	  else
	    {
	      if (!match (lp->value.cp, mask))
		return -1;
	    }
	}
      else if (irccmp (lp->value.cp, mask) == 0)
	return -1;
    }
  lp = make_link ();
  lp->next = source_p->user->silence;
  lp->value.cp = (char *) MyMalloc (strlen (mask) + 1);
  (void) strcpy (lp->value.cp, mask);
  source_p->user->silence = lp;
  return 0;
}

/* m_silence
 * parv[0] = sender prefix
 * From local client:
 * parv[1] = mask (NULL sends the list)
 * From remote client:
 * parv[1] = nick that must be silenced
 * parv[2] = mask
 */
int
m_silence (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  Link *lp;
  aClient *target_p = NULL;
  char c, *cp;
  if (check_registered_user (source_p))
    return 0;
  if (MyClient (source_p))
    {
      target_p = source_p;
      if (parc < 2 || *parv[1] == '\0'
	  || (target_p = find_person (parv[1], NULL)))
	{
	  if (!(target_p->user))
	    return 0;
	  for (lp = target_p->user->silence; lp; lp = lp->next)
	    sendto_one (source_p, rpl_str (RPL_SILELIST), me.name,
			source_p->name, target_p->name, lp->value.cp);
	  sendto_one (source_p, rpl_str (RPL_ENDOFSILELIST), me.name,
		      target_p->name);
	  return 0;
	}
      cp = parv[1];
      c = *cp;
      if (c == '-' || c == '+')
	cp++;
      else
	if (!(strchr (cp, '@') || strchr (cp, '.') ||
	      strchr (cp, '!') || strchr (cp, '*')))
	{
	  sendto_one (source_p, err_str (ERR_NOSUCHNICK), me.name, parv[0],
		      parv[1]);
	  return 0;
	}
      else
	c = '+';
      cp = pretty_mask (cp);
      if ((c == '-' && !del_silence (source_p, cp)) ||
	  (c != '-' && !add_silence (source_p, cp)))
	{
	  sendto_prefix_one (source_p, source_p, ":%s SILENCE %c%s", parv[0],
			     c, cp);
	  if (c == '-')
	    sendto_serv_butone (NULL, ":%s SILENCE * -%s", source_p->name,
				cp);
	}
    }
  else if (parc < 3 || *parv[2] == '\0')
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS), me.name, parv[0],
		  "SILENCE");
      return -1;
    }
  else if ((c = *parv[2]) == '-' || (target_p = find_person (parv[1], NULL)))
    {
      if (c == '-')
	{
	  if (!del_silence (source_p, parv[2] + 1))
	    sendto_serv_butone (client_p, ":%s SILENCE %s :%s",
				parv[0], parv[1], parv[2]);
	}
      else
	{
	  (void) add_silence (source_p, parv[2]);
	  if (!MyClient (target_p))
	    sendto_one (target_p, ":%s SILENCE %s :%s",
			parv[0], parv[1], parv[2]);
	}
    }
  else
    {
      sendto_one (source_p, err_str (ERR_NOSUCHNICK), me.name, parv[0],
		  parv[1]);
      return 0;
    }
  return 0;
}

int
add_dccallow (aClient * source_p, aClient * optr)
{
  Link *lp;
  int cnt = 0;
  for (lp = source_p->user->dccallow; lp; lp = lp->next)
    {
      if (lp->flags != DCC_LINK_ME)
	continue;
      if (++cnt >= MAXDCCALLOW)
	{
	  sendto_one (source_p, err_str (ERR_TOOMANYDCC), me.name,
		      source_p->name, optr->name, MAXDCCALLOW);
	  return 0;
	}
      else if (lp->value.client_p == optr)
	{
	  /* silently return */
	  return 0;
	}
    }

  lp = make_link ();
  lp->value.client_p = optr;
  lp->flags = DCC_LINK_ME;
  lp->next = source_p->user->dccallow;
  source_p->user->dccallow = lp;
  lp = make_link ();
  lp->value.client_p = source_p;
  lp->flags = DCC_LINK_REMOTE;
  lp->next = optr->user->dccallow;
  optr->user->dccallow = lp;
  sendto_one (source_p, rpl_str (RPL_DCCSTATUS), me.name, source_p->name,
	      optr->name, "added to");
  return 0;
}

int
del_dccallow (aClient * source_p, aClient * optr)
{
  Link **lpp, *lp;
  int found = 0;
  for (lpp = &(source_p->user->dccallow); *lpp; lpp = &((*lpp)->next))
    {
      if ((*lpp)->flags != DCC_LINK_ME)
	continue;
      if ((*lpp)->value.client_p == optr)
	{
	  lp = *lpp;
	  *lpp = lp->next;
	  free_link (lp);
	  found++;
	  break;
	}
    }

  if (!found)
    {
      sendto_one (source_p, ":%s %d %s :%s is not in your DCC allow list",
		  me.name, RPL_DCCINFO, source_p->name, optr->name);
      return 0;
    }

  for (found = 0, lpp = &(optr->user->dccallow); *lpp; lpp = &((*lpp)->next))
    {
      if ((*lpp)->flags != DCC_LINK_REMOTE)
	continue;
      if ((*lpp)->value.client_p == source_p)
	{
	  lp = *lpp;
	  *lpp = lp->next;
	  free_link (lp);
	  found++;
	  break;
	}
    }

  if (!found)
    sendto_snomask (SNOMASK_DEBUG,
			"%s was in dccallowme list of %s but not in dccallowrem list!",
			optr->name, source_p->name);
  sendto_one (source_p, rpl_str (RPL_DCCSTATUS), me.name, source_p->name,
	      optr->name, "removed from");
  return 0;
}

int
allow_dcc (aClient * to, aClient * from)
{
  Link *lp;
  for (lp = to->user->dccallow; lp; lp = lp->next)
    {
      if (lp->flags == DCC_LINK_ME && lp->value.client_p == from)
	return 1;
    }
  return 0;
}

int
m_dccallow (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  Link *lp;
  char *p, *s;
  char *cn;
  aClient *target_p, *lastclient_p = NULL;
  int didlist = 0, didhelp = 0, didanything = 0;
  char **ptr;
  static char *dcc_help[] = {
    "/DCCALLOW [<+|->nick[,<+|->nick, ...]] [list] [help]",
    "You may allow DCCs of filetypes which are otherwise blocked by the IRC server",
    "by specifying a DCC allow for the user you want to recieve files from.",
    "For instance, to allow the user bob to send you file.exe, you would type:",
    "/dccallow +bob",
    "and bob would then be able to send you files. bob will have to resend the file",
    "if the server gave him an error message before you added him to your allow list.",
    "/dccallow -bob",
    "Will do the exact opposite, removing him from your dcc allow list.",
    "/dccallow list",
    "Will list the users currently on your dcc allow list.", NULL
  };
  if (!MyClient (source_p))
    return 0;			/* don't accept dccallows from servers or clients that aren't mine.. */
  if (parc < 2)
    {
      sendto_one (source_p,
		  ":%s NOTICE %s :No command specified for DCCALLOW. Type /dccallow help for more information.",
		  me.name, source_p->name);
      return 0;
    }

  for (p = NULL, s = strtoken (&p, parv[1], ", "); s;
       s = strtoken (&p, NULL, ", "))
    {
      if (*s == '+')
	{
	  didanything++;
	  cn = s + 1;
	  if (*cn == '\0')
	    continue;
	  target_p = find_person (cn, NULL);
	  if (target_p == source_p)
	    continue;
	  if (!target_p)
	    {
	      sendto_one (source_p, err_str (ERR_NOSUCHNICK), me.name,
			  source_p->name, cn);
	      continue;
	    }

	  if (lastclient_p == target_p)
	    sendto_snomask (SNOMASK_FLOOD,
				"User %s (%s@%s) may be flooding dccallow: add %s",
				source_p->name, source_p->user->username,
				source_p->user->host, target_p->name);
	  lastclient_p = target_p;
	  add_dccallow (source_p, target_p);
	}
      else if (*s == '-')
	{
	  didanything++;
	  cn = s + 1;
	  if (*cn == '\0')
	    continue;
	  target_p = find_person (cn, NULL);
	  if (target_p == source_p)
	    continue;
	  if (!target_p)
	    {
	      sendto_one (source_p, err_str (ERR_NOSUCHNICK), me.name,
			  source_p->name, cn);
	      continue;
	    }

	  if (lastclient_p == target_p)
	    sendto_snomask (SNOMASK_SPY,
				"User %s (%s@%s) may be flooding dccallow: del %s",
				source_p->name, source_p->user->username,
				source_p->user->host, target_p->name);
	  lastclient_p = target_p;
	  del_dccallow (source_p, target_p);
	}
      else
	{
	  if (!didlist && ircncmp (s, "list", 4) == 0)
	    {
	      didanything++;
	      didlist++;
	      sendto_one (source_p,
			  ":%s %d %s :The following users are on your dcc allow list:",
			  me.name, RPL_DCCINFO, source_p->name);
	      for (lp = source_p->user->dccallow; lp; lp = lp->next)
		{
		  if (lp->flags == DCC_LINK_REMOTE)
		    continue;
		  sendto_one (source_p, ":%s %d %s :%s (%s@%s)", me.name,
			      RPL_DCCLIST, source_p->name,
			      lp->value.client_p->name,
			      lp->value.client_p->user->username,
			      IsHidden (client_p) ? lp->value.client_p->user->
			      virthost : lp->value.client_p->user->host);
		}
	      sendto_one (source_p, rpl_str (RPL_ENDOFDCCLIST), me.name,
			  source_p->name, s);
	    }
	  else if (!didhelp && ircncmp (s, "help", 4) == 0)
	    {
	      didanything++;
	      didhelp++;
	      for (ptr = dcc_help; *ptr; ptr++)
		sendto_one (source_p, ":%s %d %s :%s", me.name, RPL_DCCINFO,
			    source_p->name, *ptr);
	      sendto_one (source_p, rpl_str (RPL_ENDOFDCCLIST), me.name,
			  source_p->name, s);
	    }
	}
    }

  if (!didanything)
    {
      sendto_one (source_p,
		  ":%s NOTICE %s :Invalid syntax for DCCALLOW. Type /dccallow help for more information.",
		  me.name, source_p->name);
      return 0;
    }

  return 0;
}
