#ifndef	__struct_include__
#define __struct_include__

#include "config.h"
#if !defined(CONFIG_H_LEVEL_12)
#error Incorrect config.h for this revision of ircd.
#endif

#include <stdio.h>
#include <sys/types.h>
#ifdef _FD_SETSIZE
#undef FD_SETSIZE
#define FD_SETSIZE _FD_SETSIZE
#endif
#include <netinet/in.h>
#include <netdb.h>
#if defined( HAVE_STDDEF_H )
#include <stddef.h>
#endif
#ifdef ORATIMING
#include <sys/time.h>
#endif

#ifdef USE_SYSLOG
#include <syslog.h>
#if defined( HAVE_SYS_SYSLOG_H )
#include <sys/syslog.h>
#endif
#endif

#ifdef USE_SSL
#include <openssl/rsa.h>       /* OpenSSL stuff */
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#define REPORT_DO_DNS_		":%s NOTICE AUTH :*** Looking up your hostname..."
#define REPORT_FIN_DNS_		":%s NOTICE AUTH :*** Found your hostname"
#define REPORT_FIN_DNSC_	":%s NOTICE AUTH :*** Found your hostname, cached"
#define REPORT_FAIL_DNS_	":%s NOTICE AUTH :*** Couldn't look up your hostname"
#define REPORT_DO_ID_		":%s NOTICE AUTH :*** Checking Ident"
#define REPORT_FIN_ID_		":%s NOTICE AUTH :*** Got Ident response"
#define REPORT_FAIL_ID_		":%s NOTICE AUTH :*** No Ident response"

extern char REPORT_DO_DNS[256], REPORT_FIN_DNS[256], REPORT_FIN_DNSC[256],
  REPORT_FAIL_DNS[256], REPORT_DO_ID[256], REPORT_FIN_ID[256],
  REPORT_FAIL_ID[256], REPORT_DO_PROXY[256], REPORT_NO_PROXY[256],
  REPORT_GOOD_PROXY[256], REPORT_PROXYEXEMPT[256];

#include "hash.h"

typedef struct ConfItem aConfItem;
typedef struct Client aClient;
typedef struct Channel aChannel;
typedef struct User anUser;
typedef struct Server aServer;
typedef struct SLink Link;
typedef struct ChanLink chanMember;
typedef struct SMode Mode;
typedef struct Watch aWatch;
typedef struct Ban aBan;
typedef struct ListOptions LOpts;
typedef struct Gline aGline;
typedef long ts_val;

typedef struct MotdItem aMotd;

typedef struct OMotdItem aOMotd;
typedef struct RulesItem aRules;

typedef struct t_crline aCRline;


#include "class.h"
#include "dbuf.h"		/* THIS REALLY SHOULDN'T BE HERE!!! --msa */

#define SM_MAXMODES MAXMODEPARAMSUSER

#define	HOSTLEN		63	/* Length of hostname.  Updated to */
				/* comply with RFC1123 */

#ifndef INET6
#define HOSTIPLEN	15	/* Length of dotted quad form of IP */
#define IPLEN		4	/* binary length of an ip number (32 bits) */
#else
#define HOSTIPLEN	40
#define IPLEN		16	/* binary length of an ip6 number (128 bits) */
#endif

/* - Dianora  */

#define MAX_DATE_STRING 32	/* maximum string length for a date string */

#define	USERLEN		10
#define	REALLEN	 	50
#define	CHANNELLEN	32
#define	PASSWDLEN 	63
#define	KEYLEN		23
#define	BUFSIZE		512	/* WARNING: *DONT* CHANGE THIS!!!! */
#define	MAXRECIPIENTS 	20
#define	MAXBANS	 	60
#define MAXEXEMPTS	60

#define MOTDLINELEN	90

#define        MAXSILES        10
#define        MAXSILELENGTH   128

#define MAXDCCALLOW 5
#define DCC_LINK_ME	0x01	/* This is my dcc allow */
#define DCC_LINK_REMOTE 0x02	/* I need to remove these dcc allows from these clients when I die */

#define	USERHOST_REPLYLEN	(NICKLEN+HOSTLEN+USERLEN+5)

/*
 * 'offsetof' is defined in ANSI-C. The following definition * is not
 * absolutely portable (I have been told), but so far * it has worked
 * on all machines I have needed it. The type * should be size_t but...
 * --msa
 */

#ifndef offsetof
#define	offsetof(t,m) (int)((&((t *)0L)->m))
#endif

#define	elementsof(x) (sizeof(x)/sizeof(x[0]))

/* flags for bootup options (command line flags) */

#define	BOOT_CONSOLE 1
#define	BOOT_QUICK	 2
#define	BOOT_DEBUG	 4
#define	BOOT_INETD	 8
#define	BOOT_TTY	   16
#define	BOOT_OPER	   32
#define	BOOT_AUTODIE 64
#define BOOT_STDERR	 128
#define	STAT_LOG	   -6	/* logfile for -x */
#define	STAT_MASTER	-5	/* Local ircd master before identification */
#define	STAT_CONNECTING	-4
#define	STAT_HANDSHAKE	-3
#define	STAT_ME		      -2
#define	STAT_UNKNOWN	-1
/* the line of truth lies here (truth == registeredness) */
#define	STAT_SERVER	0
#define	STAT_CLIENT	    1

/* status macros. */

#define	IsRegisteredUser(x)	((x)->status == STAT_CLIENT)
#define	IsRegistered(x)		((x)->status >= STAT_SERVER)
#define	IsConnecting(x)		((x)->status == STAT_CONNECTING)
#define	IsHandshake(x)		((x)->status == STAT_HANDSHAKE)
#define	IsMe(x)			((x)->status == STAT_ME)
#define	IsUnknown(x)		((x)->status == STAT_UNKNOWN || (x)->status == STAT_MASTER)
#define	IsServer(x)		((x)->status == STAT_SERVER)
#define	IsClient(x)		((x)->status == STAT_CLIENT)
#define	IsLog(x)		((x)->status == STAT_LOG)

#define	SetMaster(x)		((x)->status = STAT_MASTER)
#define	SetConnecting(x)	((x)->status = STAT_CONNECTING)
#define	SetHandshake(x)		((x)->status = STAT_HANDSHAKE)
#define	SetMe(x)		((x)->status = STAT_ME)
#define	SetUnknown(x)		((x)->status = STAT_UNKNOWN)
#define	SetServer(x)		((x)->status = STAT_SERVER)
#define	SetClient(x)		((x)->status = STAT_CLIENT)
#define	SetLog(x)		((x)->status = STAT_LOG)

#define	FLAGS_PINGSENT     	0x00000001	/* Unreplied ping sent */
#define	FLAGS_DEADSOCKET   	0x00000002	/* Local socket is dead--Exiting soon */
#define	FLAGS_KILLED       	0x00000004	/* Prevents "QUIT" from being sent for this */
#define	FLAGS_BLOCKED      	0x00000008	/* socket is in a blocked condition */
#define FLAGS_REJECT_HOLD  	0x00000010	/* client has been klined */
#define	FLAGS_CLOSING      	0x00000020	/* set when closing to suppress errors */
#define	FLAGS_LISTEN       	0x00000040	/* used to mark clients which we listen() on */
#define	FLAGS_CHKACCESS    	0x00000080	/* ok to check clients access if set */
#define	FLAGS_DOINGDNS	   	0x00000100	/* client is waiting for a DNS response */
#define	FLAGS_AUTH	   	0x00000200	/* client is waiting on rfc931 response */
#define	FLAGS_WRAUTH	   	0x00000400	/* set if we havent writen to ident server */
#define	FLAGS_LOCAL	   	0x00000800	/* set for local clients */
#define	FLAGS_GOTID	   	0x00001000	/* successful ident lookup achieved */
#define	FLAGS_DOID	   	0x00002000	/* I-lines say must use ident return */
#define	FLAGS_NONL	   	0x00004000	/* No \n in buffer */
#define FLAGS_NORMALEX     	0x00008000	/* Client exited normally */
#define FLAGS_SENDQEX      	0x00010000	/* Sendq exceeded */
#define FLAGS_IPHASH       	0x00020000	/* iphashed this client */
#define FLAGS_ULINE 	   	0x00040000	/* client is U-lined */
#define FLAGS_USERBURST	   	0x00080000	/* server in nick/channel netburst */
#define FLAGS_TOPICBURST   	0x00100000	/* server in topic netburst */
#define FLAGS_BURST		(FLAGS_USERBURST | FLAGS_TOPICBURST)
#define FLAGS_SOBSENT      	0x00200000	/* we've sent an SOB, just have to send an EOB */
#define FLAGS_EOBRECV      	0x00400000	/* we're waiting on an EOB */
#define FLAGS_MAP     	   	0x00800000	/* Show this entry in /map */
#define FLAGS_BAD_DNS	   	0x01000000	/* spoofer-guy */
#define FLAGS_SERV_NEGO	   	0x02000000	/* This is a server that has passed connection tests,
						   but is a stat < 0 for handshake purposes */
#define FLAGS_RC4IN        	0x04000000	/* This link is rc4 encrypted. */
#define FLAGS_RC4OUT       	0x08000000	/* This link is rc4 encrypted. */
#define FLAGS_ZIPPED_IN	   	0x10000000	/* This link is gzipped. */
#define FLAGS_ZIPPED_OUT   	0x20000000	/* This link is gzipped. */
#define FLAGS_IPV6         	0x40000000	/* This link use IPV6 */


/* #define      SetUnixSock(x)          ((x)->flags |= FLAGS_UNIX) */

#define	IsListening(x)		((x)->flags & FLAGS_LISTEN)

#define	DoAccess(x)		((x)->flags & FLAGS_CHKACCESS)

#define	IsLocal(x)		((x)->flags & FLAGS_LOCAL)

#define	IsDead(x)		((x)->flags & FLAGS_DEADSOCKET)

#define	SetDNS(x)		((x)->flags |= FLAGS_DOINGDNS)
#define	DoingDNS(x)		((x)->flags & FLAGS_DOINGDNS)
#define	SetAccess(x)		((x)->flags |= FLAGS_CHKACCESS)
#define	DoingAuth(x)		((x)->flags & FLAGS_AUTH)
#define	NoNewLine(x)		((x)->flags & FLAGS_NONL)


#define	ClearDNS(x)		((x)->flags &= ~FLAGS_DOINGDNS)
#define	ClearAuth(x)		((x)->flags &= ~FLAGS_AUTH)
#define	ClearAccess(x)		((x)->flags &= ~FLAGS_CHKACCESS)

#define SetNegoServer(x)	((x)->flags |= FLAGS_SERV_NEGO)
#define IsNegoServer(x)		((x)->flags & FLAGS_SERV_NEGO)
#define ClearNegoServer(x)	((x)->flags &= ~FLAGS_SERV_NEGO)
#define IsRC4OUT(x)		((x)->flags & FLAGS_RC4OUT)
#define SetRC4OUT(x)		((x)->flags |= FLAGS_RC4OUT)
#define IsRC4IN(x)		((x)->flags & FLAGS_RC4IN)
#define SetRC4IN(x)		((x)->flags |= FLAGS_RC4IN)
#define RC4EncLink(x)		(((x)->flags & (FLAGS_RC4IN|FLAGS_RC4OUT)) == (FLAGS_RC4IN|FLAGS_RC4OUT))

#define ZipIn(x)		((x)->flags & FLAGS_ZIPPED_IN)
#define SetZipIn(x)		((x)->flags |= FLAGS_ZIPPED_IN)
#define ZipOut(x)		((x)->flags & FLAGS_ZIPPED_OUT)
#define SetZipOut(x)		((x)->flags |= FLAGS_ZIPPED_OUT)

#define FLAGS2_SSL         	0x00000008	/* This link uses SSL*/
#define FLAGS2_KLINEEXEMPT	0x00000010	/* This client is exempt from klines/akills/glines */
#define FLAGS2_SUPEREXEMPT	0x00000020	/* This client is exempt from klines/akills/glines and class limits */

/* FLAGS2_SSL */
#define IsSSL(x)		((x)->flags2 & FLAGS2_SSL)
#define SetSSL(x)		((x)->flags2 |= FLAGS2_SSL)

/* FLAGS2_KLINEEXEMPT */
#define IsKLineExempt(x)	((x)->flags2 & FLAGS2_KLINEEXEMPT)
#define SetKLineExempt(x)	((x)->flags2 |= FLAGS2_KLINEEXEMPT)
#define ClearKLineExempt(x)	((x)->flags2 &= ~FLAGS2_KLINEEXEMPT)

/* FLAGS2_SUPEREXEMPT */
#define IsSuperExempt(x)	((x)->flags2 & FLAGS2_SUPEREXEMPT)
#define SetSuperExempt(x)	((x)->flags2 |= FLAGS2_SUPEREXEMPT)
#define ClearSuperExempt(x)	((x)->flags2 &= ~FLAGS2_SUPEREXEMPT)

#define IsExempt(x)		(IsKLineExempt(x) || IsSuperExempt(x))

/* Capabilities of the ircd -- OLD VERSION */

#define CAPAB_TS3     0x0000001	/* Supports the TS3 Protocal */
#define CAPAB_NOQUIT  0x0000002	/* Supports NOQUIT */
#define CAPAB_NSJOIN  0x0000004	/* server supports new smart sjoin */
#define CAPAB_BURST   0x0000008	/* server supports BURST command */
#define CAPAB_UNCONN  0x0000010	/* server supports UNCONNECT */
#define CAPAB_SSJ3    0x0000020	/* Server supports our new SSJOIN protocol */
#define CAPAB_DKEY    0x0000040	/* server supports dh-key exchange using "DKEY" */
#define CAPAB_ZIP     0x0000080	/* server supports gz'd links */
#define CAPAB_DOZIP   0x0000100	/* output to this link shall be gzipped */
#define CAPAB_DODKEY  0x0000200	/* do I do dkey with this link? */
#define CAPAB_NICKIP  0x0000400	/* IP in the NICK line? */
#define CAPAB_TS5     0x0000800	/* Supports the TS5 Protocol */
#define CAPAB_CLIENT  0x0001000 /* Supports CLIENT */
#define CAPAB_TSMODE  0x0002000 /* MODE's parv[2] is channel_p->channelts for channel mode */
#define CAPAB_IPV6   0x0004000 /* Server is able to handle ipv6 address masks */
#define CAPAB_SSJ4    0x0008000 /* Server supports smart join protocol 4 */
#define CAPAB_SSJ5    0x0010000 /* Server supports smart join protocol 5 */

/* Capability Structure */
struct Capability {
  char *name;  /* name of capability */
  unsigned int cap; /* bitmask */
};

#define CAP_NOQUIT	0x00000001
#define CAP_EXCEPT	0x00000002
#define CAP_INVEX	0x00000004
#define CAP_QUIET	0x00000008
#define CAP_KNOCK	0x00000010
#define CAP_HUB		0x00000020
#define CAP_GLINE	0x00000040
#define CAP_UID		0x00000080
#define CAP_CLI		0x00000100

#define SetTS3(x)   	((x)->capabilities |= CAPAB_TS3)
#define IsTS3(x)       	((x)->capabilities & CAPAB_TS3)

#define SetNoQuit(x) 	((x)->capabilities |= CAPAB_NOQUIT)
#define IsNoQuit(x) 	((x)->capabilities & CAPAB_NOQUIT)

#define SetSSJoin(x)	((x)->capabilities |= CAPAB_NSJOIN)
#define IsSSJoin(x)	((x)->capabilities & CAPAB_NSJOIN)

#define SetBurst(x)	((x)->capabilities |= CAPAB_BURST)
#define IsBurst(x)	((x)->capabilities & CAPAB_BURST)

#define SetUnconnect(x)	((x)->capabilities |= CAPAB_UNCONN)
#define IsUnconnect(x)	((x)->capabilities & CAPAB_UNCONN)

#define SetSSJoin3(x)	((x)->capabilities |= CAPAB_SSJ3)
#define IsSSJoin3(x)	((x)->capabilities & CAPAB_SSJ3)
#define ClearSSJoin3(x)	((x)->capabilities &= ~CAPAB_SSJ3)

#define SetDKEY(x)	((x)->capabilities |= CAPAB_DKEY)
#define CanDoDKEY(x)    ((x)->capabilities & CAPAB_DKEY)
#define WantDKEY(x)	((x)->capabilities & CAPAB_DODKEY)	/* N: line, flag E */

#define SetZipCapable(x) ((x)->capabilities |= CAPAB_ZIP)
#define IsZipCapable(x)	((x)->capabilities & CAPAB_ZIP)
#define DoZipThis(x) 	((x)->capabilities & CAPAB_DOZIP)	/* this is set in N: line, flag Z */

#define SetNICKIP(x)    ((x)->capabilities |= CAPAB_NICKIP)
#define IsNICKIP(x)     ((x)->capabilities & CAPAB_NICKIP)

#define SetTS5(x)	((x)->capabilities |= CAPAB_TS5)
#define IsTS5(x)		((x)->capabilities & CAPAB_TS5)

#define SetClientCapable(x)	((x)->capabilities |= CAPAB_CLIENT)
#define IsClientCapable(x)	((x)->capabilities & CAPAB_CLIENT)

#define SetTSMODE(x)    ((x)->capabilities |= CAPAB_TSMODE)
#define IsTSMODE(x)     ((x)->capabilities & CAPAB_TSMODE)

#define SetIpv6(x)    ((x)->capabilities |= CAPAB_IPV6)
#define IsIpv6(x)     ((x)->capabilities & CAPAB_IPV6)


#define SetSSJoin4(x)	((x)->capabilities |= CAPAB_SSJ4)
#define IsSSJoin4(x)	((x)->capabilities & CAPAB_SSJ4)
#define ClearSSJoin4(x)	((x)->capabilities &= ~CAPAB_SSJ4)

#define SetSSJoin5(x)	((x)->capabilities |= CAPAB_SSJ5)
#define IsSSJoin5(x)	((x)->capabilities & CAPAB_SSJ5)


/* flag macros. */
#define IsULine(x) ((x)->flags & FLAGS_ULINE)


/* User Modes */
#define UMODE_o     	0x00000001	/* umode +o - Oper */
#define UMODE_i		0x00000002	/* umode +i - Invisible */
#define UMODE_r		0x00000004	/* umode +r - Registered */
#define UMODE_w		0x00000008	/* umode +w - Wallops */
#define UMODE_x		0x00000010	/* umode +x - Hiddenhost */
#define UMODE_I		0x00000020	/* umode +I - Block Invites */

#define SNOMASK_CONNECTS    0x00000001
#define SNOMASK_DEBUG       0x00000002
#define SNOMASK_DCC         0x00000004
#define SNOMASK_FLOOD       0x00000008
#define SNOMASK_GLOBOPS     0x00000010
#define SNOMASK_REJECTS     0x00000020
#define SNOMASK_KILLS       0x00000040
#define SNOMASK_SPAM        0x00000080
#define SNOMASK_NETINFO     0x00000100
#define SNOMASK_SPY         0x00000200
#define SNOMASK_NCHANGE     0x00000400
#define SNOMASK_LINKS       0x00000800

#define SEND_UMODES	ALL_UMODES
#define ALL_UMODES	(UMODE_o|UMODE_i|UMODE_r|UMODE_w|UMODE_x)

#define	FLAGS_ID (FLAGS_DOID|FLAGS_GOTID)

#define	IsOper(x)		((x)->umode & UMODE_o)
#define	SetOper(x)		((x)->umode |= UMODE_o)
#define	ClearOper(x)		((x)->umode &= ~UMODE_o)

#define	IsInvisible(x)		((x)->umode & UMODE_i)
#define	SetInvisible(x)		((x)->umode |= UMODE_i)
#define	ClearInvisible(x)	((x)->umode &= ~UMODE_i)

#define IsARegNick(x)   	((x)->umode & (UMODE_r))

#define IsRegNick(x)    	((x)->umode & UMODE_r)
#define SetRegNick(x)   	((x)->umode |= UMODE_r)

#define IsHidden(x)             ((x)->umode & UMODE_x)
#define SetHidden(x)            ((x)->umode |= UMODE_x)
#define ClearHidden(x)		((x)->umode &= ~UMODE_x)

/* sModes */
#define SMODE_s		0x00000001	/* smode +s - Client is connected using SSL */

#define SEND_SMODES SMODE_s

/* SMODE_s */
#define IsSSLClient(x)		((x)->smode & SMODE_s)
#define SetSSLClient(x)		((x)->smode |= SMODE_s)

#define	IsPerson(x)		((x)->user && IsClient(x))

#define	IsPrivileged(x)		(IsAnOper(x) || IsServer(x))

/* flags2 macros. */

#define IsRestricted(x)		((x)->flags & FLAGS_RESTRICTED)
#define SetRestricted(x)	((x)->flags |= FLAGS_RESTRICTED)
/* Oper flags */

#define IsAnOper(x)		((x)->umode & UMODE_o)

/* permissions */
#define OFLAG_AUSPEX		0x00000001
#define OFLAG_OPERWALK		0x00000002
#define OFLAG_KLINE		0x00000004
#define OFLAG_UNKLINE		0x00000008
#define OFLAG_LROUTE		0x00000010
#define OFLAG_GROUTE		0x00000020
#define OFLAG_DIE		0x00000040
#define OFLAG_GNOTICE		0x00000080
#define OFLAG_GLINE		0x00000100
#define OFLAG_LKILL		0x00000200
#define OFLAG_GKILL		0x00000400
#define OFLAG_LNOTICE		0x00000800
#define OFLAG_USREDIT		0x00001000
#define OFLAG_IMMUNE		0x00002000
#define OFLAG_CHEDIT		0x00004000
#define OFLAG_SELFEDIT		0x00008000
#define OFLAG_REHASH		0x00010000
#define OFLAG_RESTART		0x00020000
#define OFLAG_WALLOPS		0x00040000

/* defined debugging levels */
#define	DEBUG_FATAL  0
#define	DEBUG_ERROR  1		/* report_error() and other errors that are found */
#define	DEBUG_NOTICE 3
#define	DEBUG_DNS    4		/* used by all DNS related routines - a *lot* */
#define	DEBUG_INFO   5		/* general usful info */
#define	DEBUG_NUM    6		/* numerics */
#define	DEBUG_SEND   7		/* everything that is sent out */
#define	DEBUG_DEBUG  8		/* anything to do with debugging, ie unimportant :) */
#define	DEBUG_MALLOC 9		/* malloc/free calls */
#define	DEBUG_LIST  10		/* debug list use */
/* defines for curses in client */
#define	DUMMY_TERM	0
#define	CURSES_TERM	1
#define	TERMCAP_TERM	2

/*
 *  IPv4/IPv6 ?
 */

char mydummy[64];
char mydummy2[64];

#ifdef INET6

# define WHOSTENTP(x) ((x)[0]|(x)[1]|(x)[2]|(x)[3]|(x)[4]|(x)[5]|(x)[6]|(x)[7]|(x)[8]|(x)[9]|(x)[10]|(x)[11]|(x)[12]|(x)[13]|(x)[14]|(x)[15])

# define	AFINET		AF_INET6
# define	SOCKADDR_IN	sockaddr_in6
# define	SOCKADDR	sockaddr
# define	SIN_FAMILY	sin6_family
# define	SIN_PORT	sin6_port
# define	SIN_ADDR	sin6_addr
# define	S_ADDR		s6_addr
# define	IN_ADDR		in6_addr
# define	INADDRANY_STR	"0::0"

# if defined(linux) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(bsdi)
#  ifndef s6_laddr
#   define s6_laddr        s6_addr32
#  endif
# endif

#else
# define	AFINET		AF_INET
# define	SOCKADDR_IN	sockaddr_in
# define	SOCKADDR	sockaddr
# define	SIN_FAMILY	sin_family
# define	SIN_PORT	sin_port
# define	SIN_ADDR	sin_addr
# define	S_ADDR		s_addr
# define	IN_ADDR		in_addr
# define	INADDRANY_STR	"0.0.0.0"

# define WHOSTENTP(x) (x)
#endif

struct Counter
{
  int server;			/* servers */
  int myserver;			/* my servers */
  int myulined;			/* my ulined servers */
  int oper;			/* Opers */
  int chan;			/* Channels */
  int local;			/* Local Clients */
  int total;			/* total clients */
  int invisi;			/* invisible clients */
  int unknown;			/* unknown connections */
  int max_loc;			/* MAX local clients */
  int max_tot;			/* MAX global clients */
  ts_val start;			/* when we started collecting info */
  u_long today;			/* Client Connections today */
  ts_val day;			/* when today started */
  u_long weekly;		/* connections this week */
  ts_val week;			/* when this week started */
  u_long monthly;		/* connections this month */
  ts_val month;			/* when this month started */
  u_long yearly;		/* this is gonna be big */
  ts_val year;			/* when this year started (HEH!) */
};

struct MotdItem
{
  char line[MOTDLINELEN];
  struct MotdItem *next;
};
struct OMotdItem
{
  char line[MOTDLINELEN];
  struct OMotdItem *next;
};
struct RulesItem
{
  char line[MOTDLINELEN];
  struct RulesItem *next;
};



/*
 * lets speed this up... also removed away information. *tough* Dianora
 */
typedef struct Whowas
{
  int hashv;
  char name[NICKLEN + 1];
  char username[USERLEN + 1];
  char hostname[HOSTLEN + 1];
  char virthostname[HOSTLEN + 1];
  char hostipname[HOSTLEN + 1];
  int washidden;
  char *servername;
  char realname[REALLEN + 1];
  time_t logoff;
  struct Client *online;	/* Pointer to new nickname for chasing or NULL */

  struct Whowas *next;		/* for hash table... */

  struct Whowas *prev;		/* for hash table... */
  struct Whowas *cnext;		/* for client struct linked list */
  struct Whowas *cprev;		/* for client struct linked list */
}
aWhowas;

struct ConfItem
{
  unsigned int status;		/* If CONF_ILLEGAL, delete when no clients */
  unsigned int flags;		/* i-lines and akills use this */
  int clients;			/* Number of *LOCAL* clients using this */
  struct IN_ADDR ipnum; 	/* ip number of host field */
  char *host;
  char *localhost;
  char *passwd;
  char *name;
  int port;
  time_t hold;			/* Hold action until this time (calendar time) */
  aClass *class;		/* Class of connection */
  struct ConfItem *next;
};

#define	CONF_ILLEGAL	0x80000000
#define	CONF_MATCH		0x40000000

/* #define      CONF_QUARANTINED_SERVER 0x0001 */
#define	CONF_CLIENT		0x00000002
#define	CONF_CONNECT_SERVER	0x00000004
#define	CONF_NOCONNECT_SERVER	0x00000008
#define	CONF_LOCOP		0x00000010
#define	CONF_OPERATOR		0x00000020
#define	CONF_ME			0x00000040
#define	CONF_KILL		0x00000080
#define	CONF_ADMIN		0x00000100
#define	CONF_CLASS		0x00000200
#define	CONF_SERVICE		0x00000400
#define	CONF_LEAF		0x00000800
#define	CONF_LISTEN_PORT	0x00001000
#define	CONF_HUB		0x00002000
#define CONF_ELINE		0x00004000
#define CONF_FLINE		0x00008000

#define CONF_QUARANTINED_NICK 	0x00040000
#define CONF_ULINE 		0x00080000
#define CONF_DRPASS		0x00100000	/* die/restart pass, from df465 */

#define CONF_SQLINE     	0x00400000
#define CONF_GCOS               0x00800000
#define CONF_SGLINE             0x01000000

#define CONF_VHOST		0x04000000
#define CONF_QUARANTINED_CHAN	0x08000000
#define CONF_NETWORKADMIN	0x10000000
#define CONF_QUARANTINE         (CONF_QUARANTINED_NICK|CONF_QUARANTINED_CHAN)
#define	CONF_OPS		(CONF_OPERATOR | CONF_LOCOP)
#define	CONF_SERVER_MASK	(CONF_CONNECT_SERVER | CONF_NOCONNECT_SERVER)
#define	CONF_CLIENT_MASK	(CONF_CLIENT | CONF_SERVICE | CONF_OPS | CONF_SERVER_MASK)
#define	IsIllegal(x)	((x)->status & CONF_ILLEGAL)
#ifdef LITTLE_I_LINES
#define CONF_FLAGS_LITTLE_I_LINE	0x0001
#endif

/* Client structures */
struct User
{
  Link *channel;		/* chain of channel pointer blocks */
  Link *invited;		/* chain of invite pointer blocks */
  char *away;			/* pointer to away message */
  time_t last;
  int joined;			/* number of channels joined */
  char username[USERLEN + 1];
  char host[HOSTLEN + 1];
  char virthost[HOSTLEN + 1];
  char *server;			/* pointer to scached server name */
#ifdef OS_SOLARIS
  uint_t servicestamp;		/* solaris is gay -epi */
#else
  u_int32_t servicestamp;	/* Services id - Raistlin */
#endif
  /*
   * In a perfect world the 'server' name should not be needed, a
   * pointer to the client describing the server is enough. 
   * Unfortunately, in reality, server may not yet be in links while
   * USER is introduced... --msa
   */
  Link *silence;		/* chain of silenced users */
  LOpts *lopt;			/* Saved /list options */
  Link *dccallow;		/* chain of dcc send allowed users */
};

struct Server
{
  char *up;			/* Pointer to scache name */
  char bynick[NICKLEN + 1];
  char byuser[USERLEN + 1];
  char byhost[HOSTLEN + 1];
  aConfItem *nline;		/* N-line pointer for this server */
  int dkey_flags;		/* dkey flags */
#ifdef HAVE_ENCRYPTION_ON
  void *sessioninfo_in;		/* pointer to opaque sessioninfo structure */
  void *sessioninfo_out;	/* pointer to opaque sessioninfo structure */
  void *rc4_in;			/* etc */
  void *rc4_out;		/* etc */
#endif
  void *zip_out;
  void *zip_in;
};

struct Client
{
  struct Client *next, *prev, *hnext;
  anUser *user;			/* ...defined, if this is a User */
  aServer *serv;		/* ...defined, if this is a server */
  aWhowas *whowas;		/* Pointers to whowas structs */
  time_t lasttime;		/* ...should be only LOCAL clients? --msa */
  time_t firsttime;		/* time client was created */
  time_t since;			/* last time we parsed something */
  ts_val tsinfo;		/* TS on the nick, SVINFO on servers */
  long flags;			/* client flags */
  long flags2;			/* More client flags. Ran out of space - ShadowMaster */
  long umode;			/* uMode flags */
  long smode;			/* sMode flags */
  long snomask;			/* snoMask flags */
  aClient *from;		/* == self, if Local Client, *NEVER* NULL! */
  aClient *uplink;		/* this client's uplink to the network */
  int fd;			/* >= 0, for local clients */
  int hopcount;			/* number of servers to this 0 = local */
  short status;			/* Client type */
  char nicksent;
  char name[HOSTLEN + 1];	/* Unique name of the client, nick or host */
  char info[REALLEN + 1];	/* Free form additional client information */
#ifdef FLUD
  Link *fludees;
#endif
  struct IN_ADDR ip;		/* keep real ip# too */
  char hostip[HOSTIPLEN + 1];	/* Keep real ip as string too - Dianora */

  Link *watch;			/* user's watch list */
  int watches;			/* how many watches this user has set */

#ifdef USE_SSL
    SSL *ssl;
    X509 *client_cert;
#endif /*SSL*/

  /*
   * The following fields are allocated only for local clients 
   * (directly connected to this server with a socket.  The first
   * of them MUST be the "count"--it is the field to which the
   * allocation is tied to! Never refer to  these fields, if (from != self).
   */

  int count;			/* Amount of data in buffer */
#ifdef FLUD
  time_t fludblock;
  struct fludbot *fluders;
#endif
#ifdef ANTI_SPAMBOT
  time_t last_join_time;	/* when this client last joined a channel */
  time_t last_leave_time;	/* when this client last left a channel */
  int join_leave_count;		/*
				 * count of JOIN/LEAVE in less 
				 * than MIN_JOIN_LEAVE_TIME seconds */
  int oper_warn_count_down;	/*
				 * warn opers of this possible spambot 
				 * every time this gets to 0 */
#endif
#ifdef ANTI_DRONE_FLOOD
  time_t            first_received_message_time;
  int               received_number_of_privmsgs;
  int               drone_noticed;
#endif
  char buffer[BUFSIZE];		/* Incoming message buffer */
  short lastsq;			/* # of 2k blocks when sendqueued called last */
  struct DBuf sendQ;		/* Outgoing message queue--if socket full */
  struct DBuf recvQ;		/* Hold for data incoming yet to be parsed */
  long sendM;			/* Statistics: protocol messages send */
  long sendK;			/* Statistics: total k-bytes send */
  long receiveM;		/* Statistics: protocol messages received */
  long receiveK;		/* Statistics: total k-bytes received */
  u_short sendB;		/* counters to count upto 1-k lots of bytes */
  u_short receiveB;		/* sent and received. */
  long lastrecvM;		/* to check for activity --Mika */
  int priority;
  aClient *acpt;		/* listening client which we accepted from */
  Link *confs;			/* Configuration record associated */
  int authfd;			/* fd for rfc931 authentication */
  char username[USERLEN + 1];	/* username here now for auth stuff */
  unsigned short port;		/* and the remote port# too :-) */
  struct hostent *hostp;
#ifdef ANTI_NICK_FLOOD
  time_t last_nick_change;
  int number_of_nick_changes;
#endif
#ifdef NO_AWAY_FLUD
  time_t alas;			/* last time of away set */
  int acount;			/* count of away settings */
#endif

  char sockhost[HOSTLEN + 1];	/*
				 * This is the host name from
				 * the socket and after which the
				 * connection was accepted. */
  char passwd[PASSWDLEN + 1];
  /* try moving this down here to prevent weird problems... ? */
  int oflag;			/* Operator Flags */
  int sockerr;			/* what was the last error returned for this socket? */
  int capabilities;		/* what this server/client supports */
  int pingval;			/* cache client class ping value here */
  int sendqlen;			/* cache client max sendq here */

#ifdef MSG_TARGET_LIMIT
     struct {
        struct Client *cli;
        time_t sent;
     } targets[MSG_TARGET_MAX];              /* structure for target rate limiting */
     time_t last_target_complain;
     unsigned int num_target_errors;
#endif

};


#define	CLIENT_LOCAL_SIZE sizeof(aClient)
#define	CLIENT_REMOTE_SIZE offsetof(aClient,count)
/* statistics structures */
struct stats
{
  unsigned int is_cl;		/* number of client connections */
  unsigned int is_sv;		/* number of server connections */
  unsigned int is_ni;		/* connection but no idea who it was */
  unsigned short is_cbs;	/* bytes sent to clients */
  unsigned short is_cbr;	/* bytes received to clients */
  unsigned short is_sbs;	/* bytes sent to servers */
  unsigned short is_sbr;	/* bytes received to servers */
  unsigned long is_cks;		/* k-bytes sent to clients */
  unsigned long is_ckr;		/* k-bytes received to clients */
  unsigned long is_sks;		/* k-bytes sent to servers */
  unsigned long is_skr;		/* k-bytes received to servers */
  time_t is_cti;		/* time spent connected by clients */
  time_t is_sti;		/* time spent connected by servers */
  unsigned int is_ac;		/* connections accepted */
  unsigned int is_ref;		/* accepts refused */
  unsigned int is_throt;	/* accepts throttled */
  unsigned int is_drone;	/* refused drones */
  unsigned int is_unco;		/* unknown commands */
  unsigned int is_wrdi;		/* command going in wrong direction */
  unsigned int is_unpf;		/* unknown prefix */
  unsigned int is_empt;		/* empty message */
  unsigned int is_num;		/* numeric message */
  unsigned int is_kill;		/* number of kills generated on collisions */
  unsigned int is_fake;		/* MODE 'fakes' */
  unsigned int is_asuc;		/* successful auth requests */
  unsigned int is_abad;		/* bad auth requests */
  unsigned int is_psuc;		/* successful proxy requests */
  unsigned int is_pbad;		/* bad proxy requests */
  unsigned int is_udp;		/* packets recv'd on udp port */
  unsigned int is_loc;		/* local connections made */
  unsigned int is_ref_1;	/* refused at kline stage 1 */
  unsigned int is_ref_2;	/* refused at kline stage 2 */
#ifdef FLUD
  unsigned int is_flud;		/* users/channels flood protected */
#endif				/* FLUD */
  char pad[16];			/* Make this an even 1024 bytes */
};

/* mode structure for channels */

struct SMode
{
  unsigned int mode;
  int limit;
  char key[KEYLEN + 1];
};

/* Message table structure */

struct Message
{
  char *cmd;
  int (*func) ();
  unsigned int count;		/* number of times command used */
  int parameters;
  char flags;

  /* bit 0 set means that this command is allowed to be used only on
   * the average of once per 2 seconds -SRB */

  /* I could have defined other bit maps to above instead of the next
     * two flags that I added. so sue me. -Dianora */

  char allow_unregistered_use;	/* flag if this command can be used 
				   * if unregistered */

  char reset_idle;		/* flag if this command causes idle time to be 
				   * reset */
  unsigned long bytes;
};

typedef struct msg_tree
{
  char *final;
  struct Message *msg;
  struct msg_tree *pointers[26];
}
MESSAGE_TREE;

/*
 * Move BAN_INFO information out of the SLink struct its _only_ used
 * for bans, no use wasting the memory for it in any other type of
 * link. Keep in mind, doing this that it makes it slower as more
 * Malloc's/Free's have to be done, on the plus side bans are a smaller
 * percentage of SLink usage. Over all, the th+hybrid coding team came
 * to the conclusion it was worth the effort.
 * 
 * - Dianora
 */

struct Ban
{
  char *banstr;
  char *who;
  time_t when;
  u_char type;
  aBan *next;
};

/* channel member link structure, used for chanmember chains */
struct ChanLink
{
  struct ChanLink *next;
  aClient *client_p;
  int flags;
  int bans;			/* for bquiet: number of bans against this user */
};

/* general link structure used for chains */

struct SLink
{
  struct SLink *next;
  union
  {
    aClient *client_p;
    aChannel *channel_p;
    aConfItem *aconf;
    aBan *banptr;
    aBan *exemptlist;
    aBan *quietlist;
    aBan *invexlist;
    aWatch *wptr;
    char *cp;
  }
  value;
  int flags;
};

/* channel structure */

struct Channel
{
  struct Channel *nextch, *prevch, *hnextch;
  int hashv;			/* raw hash value */
  Mode mode;
  char topic[TOPICLEN + 1];
  char topic_info[NICKLEN + 1 + USERLEN + 1 + HOSTLEN + 1];
  time_t topic_time;
  int users;
  chanMember *members;
  Link *invites;
  aBan *banlist;
  aBan *quietlist;
  aBan *exemptlist;
  aBan *invexlist;
  ts_val channelts;
#ifdef FLUD
  time_t fludblock;
  struct fludbot *fluders;
#endif
  char chname[CHANNELLEN + 1];
};
#ifdef TS5
#define	TS_CURRENT	5	/* current TS protocol version */
#else
#define	TS_CURRENT	3	/* current TS protocol version */
#endif
#define	TS_MIN		3	/* minimum supported TS protocol version */
#define	TS_DOESTS	0x20000000
#define	DoesTS(x)	((x)->tsinfo == TS_DOESTS)
/* Channel Related macros follow */

/* Channel related flags */

#define	CHFL_CHANOP     	0x0001	/* Channel operator */
#define	CHFL_VOICE      	0x0002	/* the power to speak */
#define	CHFL_DEOPPED 		0x0004	/* deopped by us, modes need to be bounced */
#define CHFL_HALFOP		0x0008	/* Channel Half Operator */
#define CHFL_DEHALFOPPED	0x0010	/* De Halfopped by us, mode needs to be bounced */
#define	CHFL_BAN		0x0020	/* ban channel flag */
#define CHFL_EXEMPT		0x0040	/* exempt ban channel flag */
#define CHFL_QUIET		0x0080  /* quiet ban channel flag */
#define CHFL_INVEX		0x0100	/* invite exception channel flag */
/* ban mask types */

#define MTYP_FULL      0x01	/* mask is nick!user@host */
#define MTYP_USERHOST  0x02	/* mask is user@host */
#define MTYP_HOST      0x04	/* mask is host only */

/* Channel Visibility macros */

#define	MODE_CHANOP	    	CHFL_CHANOP
#define	MODE_VOICE	    	CHFL_VOICE
#define	MODE_DEOPPED  		CHFL_DEOPPED
#define MODE_HALFOP		CHFL_HALFOP
#define MODE_DEHALFOPPED	CHFL_DEHALFOPPED
#define MODE_INVEX		0x00000020
#define MODE_QUIET		0x00000040
#define	MODE_PRIVATE  		0x00000080
#define	MODE_SECRET   		0x00000100
#define	MODE_MODERATED  	0x00000200
#define	MODE_TOPICLIMIT 	0x00000400
#define	MODE_INVITEONLY 	0x00000800
#define	MODE_NOPRIVMSGS 	0x00001000
#define	MODE_KEY	      	0x00002000
#define	MODE_BAN	      	0x00004000
#define	MODE_LIMIT	    	0x00008000
#define MODE_REGISTERED		0x00010000
#define MODE_REGONLY		0x00020000
#define MODE_NOCOLOR		0x00040000
#define MODE_OPERONLY   	0x00080000
#define MODE_EXEMPT		0x00100000
#define MODE_NOINVITE		0x00200000
#define MODE_NOKNOCK		0x00400000
#define MODE_SECURE		0x00800000

/* the hell is this? - lucas */
/* #define	MODE_FLAGS	    0x3fff*/

/* mode flags which take another parameter (With PARAmeterS) */

#define	MODE_WPARAS	(MODE_CHANOP|MODE_VOICE|MODE_BAN|MODE_EXEMPT|MODE_KEY|MODE_LIMIT|MODE_HALFOP|MODE_CHANADMIN|MODE_QUIET)

/*
 * Undefined here, these are used in conjunction with the above modes
 * in the source. #define       MODE_DEL       0x40000000 #define
 * MODE_ADD       0x80000000
 */

#define	HoldChannel(x)		(!(x))

/*name invisible */

#define	SecretChannel(x)	((x) && ((x)->mode.mode & MODE_SECRET))

/* channel not shown but names are */

#define	HiddenChannel(x)	((x) && ((x)->mode.mode & MODE_PRIVATE))

/* channel visible */

#define	ShowChannel(v,c)	(PubChannel(c) || IsMember((v),(c)))
#define	PubChannel(x)		((!x) || ((x)->mode.mode &(MODE_PRIVATE | MODE_SECRET)) == 0)

/* #define IsMember(user,chan) 
 * (find_user_link((chan)->members,user) ? 1 : 0) */

#define IsMember(blah,chan) ((blah && blah->user && find_channel_link((blah->user)->channel, chan)) ? 1 : 0)

#define	IsChannelName(name) ((name) && (*(name) == '#' || *(name) == '&'))

/* Misc macros */

#define	BadPtr(x) (!(x) || (*(x) == '\0'))

#define	isvalid(c) (((c) >= 'A' && (c) < '~') || isdigit(c) || (c) == '-')

#define	MyConnect(x)			((x)->fd >= 0)
#define	MyClient(x)			(MyConnect(x) && IsClient(x))
#define	MyOper(x)			(MyConnect(x) && IsOper(x))

/* String manipulation macros */

/* strncopynt --> strncpyzt to avoid confusion, sematics changed N must
 * be now the number of bytes in the array --msa */

#define	strncpyzt(x, y, N) do{(void)strncpy(x,y,N);x[N-1]='\0';}while(0)
#define	StrEq(x,y)	(!strcmp((x),(y)))

/* used in SetMode() in channel.c and m_umode() in s_msg.c */

#define	MODE_NULL      0
#define	MODE_ADD       0x40000000
#define	MODE_DEL       0x20000000

/* return values for hunt_server() */

#define	HUNTED_NOSUCH	(-1)	/* if the hunted server is not found */
#define	HUNTED_ISME	0	/* if this server should execute the command */
#define	HUNTED_PASS	1	/* if message passed onwards successfully */

/* used when sending to #mask or $mask */

#define	MATCH_SERVER  1
#define	MATCH_HOST    2

/* used for async dns values */

#define	ASYNC_NONE	(-1)
#define	ASYNC_CLIENT	0
#define	ASYNC_CONNECT	1
#define	ASYNC_CONF	2
#define	ASYNC_SERVER	3

/* misc variable externs */

extern char version[128], protoctl[512], smodestring[128], umodestring[128], cmodestring[128],
  *creditstext[], *copyrighttext[], snomaskstring[128];
extern char *generation, *creation;

/* misc defines */

#define ZIP_NEXT_BUFFER -4
#define RC4_NEXT_BUFFER -3
#define	FLUSH_BUFFER	-2
#define	UTMP		"/etc/utmp"
#define	COMMA		","

#ifdef ORATIMING
/*
 * Timing stuff (for performance measurements): compile with
 * -DORATIMING and put a TMRESET where you want the counter of time
 * spent set to 0, a TMPRINT where you want the accumulated results,
 * and TMYES/TMNO pairs around the parts you want timed -orabidoo
 */

extern struct timeval tsdnow, tsdthen;
extern unsigned long tsdms;

#define TMRESET tsdms=0;
#define TMYES gettimeofday(&tsdthen, NULL);
#define TMNO gettimeofday(&tsdnow, NULL); if (tsdnow.tv_sec!=tsdthen.tv_sec) tsdms+=1000000*(tsdnow.tv_sec-tsdthen.tv_sec); tsdms+=tsdnow.tv_usec; tsdms-=tsdthen.tv_usec;
#define TMPRINT sendto_ops("Time spent: %ld ms", tsdms);
#else
#define TMRESET
#define TMYES
#define TMNO
#define TMPRINT
#endif

/* allow 5 minutes after server rejoins the network before allowing
 * chanops new channel */

#ifdef NO_CHANOPS_WHEN_SPLIT
#define MAX_SERVER_SPLIT_RECOVERY_TIME 5
#endif

#ifdef FLUD
struct fludbot
{
  struct Client *fluder;
  int count;
  time_t first_msg, last_msg;
  struct fludbot *next;
};

#endif /* FLUD */

struct Watch
{
  aWatch *hnext;
  time_t lasttime;
  Link *watch;
  char nick[1];
};

struct ListOptions
{
  LOpts *next;
  Link *yeslist, *nolist;
  int starthash;
  short int showall;
  unsigned short usermin;
  int usermax;
  time_t currenttime;
  time_t chantimemin;
  time_t chantimemax;
  time_t topictimemin;
  time_t topictimemax;
};

typedef struct SearchOptions
{
  int umodes;
  char *nick;
  char *user;
  char *host;
  char *gcos;
  char *ip;
  char *vhost;
  aChannel *channel;
  aClient *server;
  char umode_plus:1;
  char nick_plus:1;
  char user_plus:1;
  char host_plus:1;
  char gcos_plus:1;
  char ip_plus:1;
  char vhost_plus:1;
  char chan_plus:1;
  char serv_plus:1;
  char away_plus:1;
  char check_away:1;
  char check_umode:1;
  char show_chan:1;
  char search_chan:1;
  char isoper:1;
  char spare:4;			/* spare space for more stuff(?) */
}
SOpts;


struct t_crline
{
  char *channel;
  int type;
  aCRline *next, *prev;
};

#define IsSendable(x)      (DBufLength(&x->sendQ) < 16384)
#define DoList(x)    (((x)->user) && ((x)->user->lopt))

/* internal defines for client_p->sockerr */
#define IRCERR_BUFALLOC		-11
#define IRCERR_ZIP		-12
#define IRCERR_SSL		-13

struct Gline
{
    char *set_on;
    char *set_by;       /* who set gline */
    char *username;     /* username for user@host */
    char *hostname;     /* hostname for user@host */
    time_t expire;      /* when gline expires (seconds) */
    time_t set_at;      /* when gline was set */
    char *reason;       /* reason for gline */
    aGline *next;
    aGline *prev;
};


#endif /* __struct_include__ */
