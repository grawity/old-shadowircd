#ifndef	__msg_include__
#define __msg_include__

#define MSG_PRIVATE  	"PRIVMSG"	/* PRIV */
#define MSG_WHO      	"WHO"	/* WHO  -> WHOC */
#define MSG_WHOIS    	"WHOIS"	/* WHOI */
#define MSG_WHOWAS   	"WHOWAS"	/* WHOW */
#define MSG_USER     	"USER"	/* USER */
#define MSG_NICK     	"NICK"	/* NICK */
#define MSG_SERVER   	"SERVER"	/* SERV */
#define MSG_LIST     	"LIST"	/* LIST */
#define MSG_TOPIC    	"TOPIC"	/* TOPI */
#define MSG_INVITE   	"INVITE"	/* INVI */
#define MSG_VERSION  	"VERSION"	/* VERS */
#define MSG_QUIT     	"QUIT"	/* QUIT */
#define MSG_SQUIT    	"SQUIT"	/* SQUI */
#define MSG_KILL     	"KILL"	/* KILL */
#define MSG_SNOMASK     "SNOMASK" /* SNOMASK */
#define MSG_LINKS    	"LINKS"	/* LINK */
#define MSG_STATS    	"STATS"	/* STAT */
#define MSG_USERS    	"USERS"	/* USER -> USRS */
#define MSG_HELP     	"HELP"	/* HELP */
#define MSG_ERROR    	"ERROR"	/* ERRO */
#define MSG_AWAY     	"AWAY"	/* AWAY */
#define MSG_CONNECT  	"CONNECT"	/* CONN */
#define MSG_PING     	"PING"	/* PING */
#define MSG_PONG     	"PONG"	/* PONG */
#define MSG_OPER     	"OPER"	/* OPER */
#define MSG_PASS     	"PASS"	/* PASS */
#define MSG_WALLOPS  	"WALLOPS"	/* WALL */
#define MSG_TIME     	"TIME"	/* TIME */
#define MSG_NAMES    	"NAMES"	/* NAME */
#define MSG_ADMIN    	"ADMIN"	/* ADMI */
#define MSG_TRACE    	"TRACE"	/* TRAC */
#define MSG_NOTICE   	"NOTICE"	/* NOTI */
#define MSG_JOIN     	"JOIN"	/* JOIN */
#define MSG_PART     	"PART"	/* PART */
#define MSG_LUSERS   	"LUSERS"	/* LUSE */
#define MSG_MOTD     	"MOTD"	/* MOTD */
#define MSG_MODE     	"MODE"	/* MODE */
#define MSG_KICK     	"KICK"	/* KICK */
#define MSG_REMOVE     	"REMOVE"	/* REMOVE */
#define MSG_USERHOST 	"USERHOST"	/* USER -> USRH */
#define MSG_ISON     	"ISON"	/* ISON */
#define MSG_REHASH   	"REHASH"	/* REHA */
#define MSG_RESTART  	"RESTART"	/* REST */
#define MSG_CLOSE    	"CLOSE"	/* CLOS */
#define MSG_SVINFO   	"SVINFO"	/* SVINFO */
#define MSG_SJOIN    	"SJOIN"	/* SJOIN */
#define MSG_DIE	     	"DIE"	/* DIE */
#define MSG_HASH     	"HASH"	/* HASH */
#define MSG_DNS      	"DNS"	/* DNS  -> DNSS */
#define MSG_OPERWALL 	"OPERWALL"	/* OPERWALL */
#define MSG_GOPER    	"GOPER"	/* GOPER */
#define MSG_GNOTICE  	"GNOTICE"	/* GNOTICE */
#define MSG_KLINE    	"KLINE"	/* KLINE */
#define MSG_GLINE    	"GLINE"	/* GLINE */
#ifdef UNKLINE
#define MSG_UNKLINE  	"UNKLINE"	/* UNKLINE */
#endif
#define MSG_CS		"CS"
#define MSG_BS		"BS"
#define MSG_NS		"NS"
#define MSG_OS		"OS"
#define MSG_MS		"MS"
#define MSG_SS		"SS"
#define MSG_HTM      	"HTM"	/* HTM */
#define MSG_SET      	"SET"	/* SET */
#define MSG_CHANSERV 	"CHANSERV"	/* CHANSERV */
#define MSG_NICKSERV 	"NICKSERV"	/* NICKSERV */
#define MSG_MEMOSERV 	"MEMOSERV"	/* MEMOSERV */
#define MSG_OPERSERV 	"OPERSERV"	/* OPERSERV */
#define MSG_STATSERV 	"STATSERV"	/* STATSERV */
#define MSG_SERVICES 	"SERVICES"	/* SERVICES */
#define MSG_IDENTIFY 	"IDENTIFY"	/* IDENTIFY */
#define MSG_CAPAB    	"CAPAB"	/* CAPAB */
#define MSG_SVSNICK  	"SVSNICK"	/* SVSNICK */
#define MSG_SVSKILL  	"SVSKILL"	/* SVSKILL */
#define MSG_SVSMODE  	"SVSMODE"	/* SVSMODE */
#define MSG_SILENCE  	"SILENCE"	/* SILENCE */
#define MSG_WATCH    	"WATCH"	/* WATCH */
#define MSG_SQLINE   	"SQLINE"	/* SQLINE */
#define MSG_UNSQLINE 	"UNSQLINE"	/* UNSQLINE */
#define MSG_BURST    	"BURST"	/* BURST */
#define MSG_DCCALLOW 	"DCCALLOW"	/* dccallow */
#define MSG_SGLINE   	"SGLINE"	/* sgline */
#define MSG_UNSGLINE 	"UNSGLINE"	/* unsgline */
#define MSG_OPERMOTD	"OPERMOTD"	/* OPERMOTD */
#define MSG_NETINFO	"NETINFO"	/* NETINFO */
#define MSG_NETGLOBAL	"NETGLOBAL"	/* NETGLOBAL */
#define MSG_SETHOST	"SETHOST"	/* SETHOST */
#define MSG_VHOST	"VHOST"	/* VHOST */
#define MSG_GCONNECT	"GCONNECT"	/* GCONNECT */
#define MSG_KNOCK	"KNOCK"	/* KNOCK */
#define MSG_CHANNEL	"CHANNEL"	/* CHANNEL */
#define MSG_BOTSERV	"BOTSERV"	/* BOTSERV */
#define MSG_ROOTSERV	"ROOTSERV"	/* ROOTSERV */
#define MSG_VCTRL	"VCTRL"	/* VCTRL */
#define MSG_MAP		"MAP"	/* MAP */
#define MSG_MAKEPASS	"MAKEPASS"	/* MAKEPASS */
#define MSG_DKEY     	"DKEY"	/* diffie-hellman negotiation */
#define MSG_FJOIN	"FJOIN"	/* Forced Join's */
#define MSG_FMODE	"FMODE"	/* Forced Mode's */
#define MSG_SETIDENT "SETIDENT" /* Ident setting */

#define MSG_CLIENT	"CLIENT"	 /* CLIENT */
#define MSG_NETCTRL	"NETCTRL"	/* NETCTRL */
#define MSG_SMODE	"SMODE"		/* SMODE */
#define MSG_RESYNCH	"RESYNCH"      /* RESYNCH */

#define MAXPARA      15

extern int m_kline (aClient *, aClient *, int, char **);
extern int m_gline (aClient *, aClient *, int, char **);
extern int m_setident (aClient *, aClient *, int, char **);
extern int m_unkline (aClient *, aClient *, int, char **);
extern int m_private (aClient *, aClient *, int, char **);
extern int m_topic (aClient *, aClient *, int, char **);
extern int m_join (aClient *, aClient *, int, char **);
extern int m_part (aClient *, aClient *, int, char **);
extern int m_mode (aClient *, aClient *, int, char **);
extern int m_ping (aClient *, aClient *, int, char **);
extern int m_pong (aClient *, aClient *, int, char **);
extern int m_wallops (aClient *, aClient *, int, char **);
extern int m_kick (aClient *, aClient *, int, char **);
extern int m_remove (aClient *, aClient *, int, char **);
extern int m_nick (aClient *, aClient *, int, char **);
extern int m_error (aClient *, aClient *, int, char **);
extern int m_notice (aClient *, aClient *, int, char **);
extern int m_invite (aClient *, aClient *, int, char **);
extern int m_quit (aClient *, aClient *, int, char **);
extern int m_kill (aClient *, aClient *, int, char **);
extern int m_motd (aClient *, aClient *, int, char **);
extern int m_who (aClient *, aClient *, int, char **);
extern int m_whois (aClient *, aClient *, int, char **);
extern int m_user (aClient *, aClient *, int, char **);
extern int m_list (aClient *, aClient *, int, char **);
extern int m_server (aClient *, aClient *, int, char **);
extern int m_links (aClient *, aClient *, int, char **);
extern int m_summon (aClient *, aClient *, int, char **);
extern int m_snomask (aClient *, aClient *, int, char **);
extern int m_stats (aClient *, aClient *, int, char **);
extern int m_users (aClient *, aClient *, int, char **);
extern int m_chanserv (aClient *, aClient *, int, char **);
extern int m_nickserv (aClient *, aClient *, int, char **);
extern int m_operserv (aClient *, aClient *, int, char **);
extern int m_statserv (aClient *, aClient *, int, char **);
extern int m_memoserv (aClient *, aClient *, int, char **);
extern int m_services (aClient *, aClient *, int, char **);
extern int m_identify (aClient *, aClient *, int, char **);
extern int m_svsnick (aClient *, aClient *, int, char **);
extern int m_svskill (aClient *, aClient *, int, char **);
extern int m_svsmode (aClient *, aClient *, int, char **);
extern int m_version (aClient *, aClient *, int, char **);
extern int m_help (aClient *, aClient *, int, char **);
extern int m_squit (aClient *, aClient *, int, char **);
extern int m_away (aClient *, aClient *, int, char **);
extern int m_connect (aClient *, aClient *, int, char **);
extern int m_oper (aClient *, aClient *, int, char **);
extern int m_pass (aClient *, aClient *, int, char **);
extern int m_trace (aClient *, aClient *, int, char **);
extern int m_time (aClient *, aClient *, int, char **);
extern int m_names (aClient *, aClient *, int, char **);
extern int m_admin (aClient *, aClient *, int, char **);
extern int m_lusers (aClient *, aClient *, int, char **);
extern int m_umode (aClient *, aClient *, int, char **);
extern int m_close (aClient *, aClient *, int, char **);
extern int m_motd (aClient *, aClient *, int, char **);
extern int m_whowas (aClient *, aClient *, int, char **);
extern int m_userhost (aClient *, aClient *, int, char **);
extern int m_ison (aClient *, aClient *, int, char **);
extern int m_svinfo (aClient *, aClient *, int, char **);
extern int m_sjoin (aClient *, aClient *, int, char **);
extern int m_goper (aClient *, aClient *, int, char **);
extern int m_gnotice (aClient *, aClient *, int, char **);
extern int m_rehash (aClient *, aClient *, int, char **);
extern int m_restart (aClient *, aClient *, int, char **);
extern int m_die (aClient *, aClient *, int, char **);
extern int m_hash (aClient *, aClient *, int, char **);
extern int m_dns (aClient *, aClient *, int, char **);
extern int m_htm (aClient *, aClient *, int, char **);
extern int m_set (aClient *, aClient *, int, char **);
extern int m_capab (aClient *, aClient *, int, char **);
extern int m_silence (aClient *, aClient *, int, char **);
extern int m_watch (aClient *, aClient *, int, char **);
extern int m_sqline (aClient *, aClient *, int, char **);
extern int m_unsqline (aClient *, aClient *, int, char **);
extern int m_burst (aClient *, aClient *, int, char **);
extern int m_dccallow (aClient *, aClient *, int, char **);
extern int m_sgline (aClient *, aClient *, int, char **);
extern int m_unsgline (aClient *, aClient *, int, char **);
extern int m_opermotd (aClient *, aClient *, int, char **);
extern int m_sethost (aClient *, aClient *, int, char **);
extern int m_vhost (aClient *, aClient *, int, char **);
extern int m_gconnect (aClient *, aClient *, int, char **);
extern int m_knock (aClient *, aClient *, int, char **);
extern int m_botserv (aClient *, aClient *, int, char **);
extern int m_rootserv (aClient *, aClient *, int, char **);
extern int m_vctrl (aClient *, aClient *, int, char **);
extern int m_map (aClient *, aClient *, int, char **);
extern int m_makepass (aClient *, aClient *, int, char **);
extern int m_dkey (aClient *, aClient *, int, char **);
extern int m_fjoin (aClient *, aClient *, int, char **);
extern int m_fmode (aClient *, aClient *, int, char **);
extern int m_client (aClient *, aClient *, int, char **);
extern int m_netctrl (aClient *, aClient *, int, char **);
extern int m_smode (aClient *, aClient *, int, char **);
extern int  m_resynch(aClient *, aClient *, int, char **);

#ifdef MSGTAB
struct Message msgtab[] = {
  {MSG_PRIVATE, m_private, 0, MAXPARA, 1, 0, 1, 0L},
  {MSG_NICK, m_nick, 0, MAXPARA, 1, 1, 0, 0L},
  {MSG_NOTICE, m_notice, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_JOIN, m_join, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_MODE, m_mode, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_SNOMASK, m_snomask, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_QUIT, m_quit, 0, MAXPARA, 1, 1, 0, 0L},
  {MSG_PART, m_part, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_TOPIC, m_topic, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_INVITE, m_invite, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_KICK, m_kick, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_REMOVE, m_remove, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_WALLOPS, m_wallops, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_PONG, m_pong, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_PING, m_ping, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_ERROR, m_error, 0, MAXPARA, 1, 1, 0, 0L},
  {MSG_KILL, m_kill, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_USER, m_user, 0, MAXPARA, 1, 1, 0, 0L},
  {MSG_AWAY, m_away, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_ISON, m_ison, 0, 1, 1, 0, 0, 0L},
  {MSG_SERVER, m_server, 0, MAXPARA, 1, 1, 0, 0L},
  {MSG_SQUIT, m_squit, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_WHOIS, m_whois, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_WHO, m_who, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_WHOWAS, m_whowas, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_LIST, m_list, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_NAMES, m_names, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_USERHOST, m_userhost, 0, 1, 1, 0, 0, 0L},
  {MSG_TRACE, m_trace, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_PASS, m_pass, 0, MAXPARA, 1, 1, 0, 0L},
  {MSG_LUSERS, m_lusers, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_TIME, m_time, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_OPER, m_oper, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_CONNECT, m_connect, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_VERSION, m_version, 0, MAXPARA, 1, 1, 0, 0L},
  {MSG_STATS, m_stats, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_LINKS, m_links, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_ADMIN, m_admin, 0, MAXPARA, 1, 1, 0, 0L},
  {MSG_USERS, m_users, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_HELP, m_help, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_MOTD, m_motd, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_SVINFO, m_svinfo, 0, MAXPARA, 1, 1, 0, 0L},
  {MSG_SJOIN, m_sjoin, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_GOPER, m_goper, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_GNOTICE, m_gnotice, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_CLOSE, m_close, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_KLINE, m_kline, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_GLINE, m_gline, 0, MAXPARA, 1, 0, 0, 0L},
#ifdef UNKLINE
  {MSG_UNKLINE, m_unkline, 0, MAXPARA, 1, 0, 0, 0L},
#endif
  {MSG_HASH, m_hash, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_DNS, m_dns, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_REHASH, m_rehash, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_RESTART, m_restart, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_DIE, m_die, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_HTM, m_htm, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_SET, m_set, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_CHANSERV, m_chanserv, 0, 1, 1, 0, 0, 0L},
  {MSG_CS, m_chanserv, 0, 1, 1, 0, 0, 0L},
  {MSG_NICKSERV, m_nickserv, 0, 1, 1, 0, 0, 0L},
  {MSG_NS, m_nickserv, 0, 1, 1, 0, 0, 0L},
  {MSG_OPERSERV, m_operserv, 0, 1, 1, 0, 0, 0L},
  {MSG_OS, m_operserv, 0, 1, 1, 0, 0, 0L},
  {MSG_STATSERV, m_statserv, 0, 1, 1, 0, 0, 0L},
  {MSG_SS, m_statserv, 0, 1, 1, 0, 0, 0L},
  {MSG_MEMOSERV, m_memoserv, 0, 1, 1, 0, 0, 0L},
  {MSG_MS, m_memoserv, 0, 1, 1, 0, 0, 0L},
  {MSG_SERVICES, m_services, 0, 1, 1, 0, 0, 0L},
  {MSG_IDENTIFY, m_identify, 0, 1, 1, 0, 0, 0L},
  {MSG_SVSNICK, m_svsnick, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_SVSKILL, m_svskill, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_SVSMODE, m_svsmode, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_SILENCE, m_silence, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_WATCH, m_watch, 0, 1, 1, 0, 0, 0L},
  {MSG_DCCALLOW, m_dccallow, 0, 1, 1, 0, 0, 0L},
  {MSG_SQLINE, m_sqline, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_UNSQLINE, m_unsqline, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_CAPAB, m_capab, 0, MAXPARA, 1, 1, 0, 0L},
  {MSG_BURST, m_burst, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_SGLINE, m_sgline, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_UNSGLINE, m_unsgline, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_OPERMOTD, m_opermotd, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_SETHOST, m_sethost, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_SETIDENT, m_setident, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_VHOST, m_vhost, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_GCONNECT, m_gconnect, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_KNOCK, m_knock, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_CHANNEL, m_join, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_BOTSERV, m_botserv, 0, 1, 1, 0, 0, 0L},
  {MSG_BS, m_botserv, 0, 1, 1, 0, 0, 0L},
  {MSG_ROOTSERV, m_rootserv, 0, 1, 1, 0, 0, 0L},
  {MSG_VCTRL, m_vctrl, 0, MAXPARA, 1, 1, 0, 0L},
  {MSG_MAP, m_map, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_MAKEPASS, m_makepass, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_DKEY, m_dkey, 0, MAXPARA, 1, 1, 0, 0L},
  {MSG_FJOIN, m_fjoin, 0, MAXPARA, 1, 0, 0, 0L},
  {MSG_FMODE, m_fmode, 0, MAXPARA, 1, 0, 0, 0L},
 {MSG_CLIENT, m_client, 0, MAXPARA, 1, 1, 0, 0L},
 {MSG_NETCTRL, m_netctrl, 0, MAXPARA, 1, 1, 0, 0L},
 {MSG_SMODE, m_smode, 0, MAXPARA, 1, 0, 0, 0L},
 {MSG_RESYNCH, m_resynch, 0, MAXPARA, 1, 0, 0, 0L},
  {(char *) 0, (int (*)()) 0, 0, 0, 0, 0, 0, 0L}
};

MESSAGE_TREE *msg_tree_root;
#else
extern struct Message msgtab[];
extern MESSAGE_TREE *msg_tree_root;
#endif
#endif /* __msg_include__  */
