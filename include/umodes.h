/*
 * shadowircd: an advanced IRC daemon.
 * umodes.h: part of new usermodes system
 *
 * $Id: umodes.h,v 3.5 2004/09/25 03:07:19 nenolod Exp $
 */

/* nice flashy bitfield math, care of asuffield and dancer-ircd.
 * adopted where necessary 05/12/04 by nenolod.
 *
 * some parts rewritten for efficiency too.
 */
#ifndef INCLUDED_UMODES_H
#define INCLUDED_UMODES_H

typedef struct {
  int mode;
  char letter;
  int operonly;
} FLAG_ITEM;

struct bitfield_lookup_t
{
  unsigned int field;
  u_int32_t bit;
};

/* This is initialised in init_umodes() in s_user.c, based on the value of 
 * BITFIELD_SIZE
 */
extern struct bitfield_lookup_t bitfield_lookup[];
extern char umode_list[];

/* the BITFIELD_SIZE define is used to create the bitfield. */
#define BITFIELD_SIZE 64

/* one bit is always needed for invalid modes as a seperator */
#define MAX_UMODE_COUNT (BITFIELD_SIZE)

/* do not change this. ever. not without changing the macros -- they are
 * hardcoded for two 32-bit bitfields.
 */
typedef u_int32_t user_modes[2];

#define SetBit(f,b)   (((f)[bitfield_lookup[b].field]) |=  bitfield_lookup[b].bit)
#define ClearBit(f,b) (((f)[bitfield_lookup[b].field]) &= ~bitfield_lookup[b].bit)
#define TestBit(f,b)  (((f)[bitfield_lookup[b].field]) &  bitfield_lookup[b].bit)

/* borrowed from asuffield's dancer-ircd. --nenolod */
#define CopyUmodes(d,s) (((d)[0] = (s)[0]), ((d)[1] = (s)[1]))
#define AndUmodes(d,a,b) (((d)[0] = (a)[0] & (b)[0]), ((d)[1] = (a)[1] & (b)[1]))
#define OrUmodes(d,a,b) (((d)[0] = (a)[0] | (b)[0]), ((d)[1] = (a)[1] | (b)[1]))
#define AndNotUmodes(d,a,b) (((d)[0] = (a)[0] & ~(b)[0]), ((d)[1] = (a)[1] & ~(b)[1]))
#define NotUmodes(d,s) (((d)[0] = ~s[0]), ((d)[1] = ~s[1]))
#define AnyBits(d) ((d)[0] | (d)[1])
#define SameBits(a,b) (((a)[0] == (b)[0]) && ((a)[1] == (b)[1]))

#define ClearBitfield(b) (((b)[0] = 0), ((b)[1] = 0))

/* These are usually used */
#define HasUmode(c,m) (TestBit((c)->umodes,m))
#define SetUmode(c,m) (SetBit((c)->umodes,m))
#define ClearUmode(c,m) (ClearBit((c)->umodes,m))

/* usermodes */
#define UMODE_SERVNOTICE   0 /* server notices     */                     /* +s */
#define UMODE_CCONN        1 /* Client Connections */                     /* +c */
#define UMODE_REJ          2 /* Bot Rejections */                         /* +j */
#define UMODE_SKILL        3 /* Server Killed */                          /* +k */
#define UMODE_FULL         4 /* Full messages */                          /* +f */
#define UMODE_SPY          5 /* see STATS / LINKS */                      /* +y */
#define UMODE_DEBUG        6 /* 'debugging' info */                       /* +d */
#define UMODE_NCHANGE      7 /* Nick change notice */                     /* +n */
#define UMODE_WALLOP       8 /* send wallops to them */                   /* +w */
#define UMODE_OPERWALL     9  /* Operwalls */                             /* +z */
#define UMODE_INVISIBLE   10 /* makes user invisible */                   /* +i */
#define UMODE_BOTS        11 /* shows bots */                             /* +b */
#define UMODE_EXTERNAL    12 /* show servers introduced and splitting */  /* +X */
#define UMODE_CALLERID    13 /* block unless caller id's */               /* +g */
#define UMODE_UNAUTH      14 /* show unauth connects here */              /* +u */
#define UMODE_LOCOPS      15 /* show locops */                            /* +l */
#define UMODE_OPER        16 /* Operator */                               /* +o */
#define UMODE_ADMIN       17 /* Admin on server */                        /* +a */
#define UMODE_IDENTIFY    18 /* Identified. */                            /* +e */
#define UMODE_HIDEOPER    19 /* Hide operator status. */                  /* +h */
#define UMODE_CLOAK       20 /* Usercloak. */                             /* +v */
#define UMODE_PROTECTED   21 /* user cannot be kicked from channels */    /* +q */
#define UMODE_BLOCKINVITE 22 /* Block Invites. */                         /* +I */
#define UMODE_PMFILTER    23 /* Allow only registered users to PM */      /* +E */
#define UMODE_HELPOP      24 /* Help operator. */                         /* +H */
#define UMODE_SVSOPER     25 /* Services operator. */                     /* +O */
#define UMODE_SVSADMIN    26 /* Services admin. */                        /* +A */
#define UMODE_SVSROOT     27 /* Services root. */                         /* +R */
#define UMODE_SERVICE     28 /* Network service. */                       /* +S */
#define UMODE_SECURE      29 /* client is using SSL */                    /* +Z */
#define UMODE_DEAF        30 /* User is deaf. */                          /* +D */

/* vanity flags */
#define UMODE_NETADMIN    31 /* network administrator */                  /* +N */
#define UMODE_TECHADMIN   32 /* technical administrator */                /* +T */

#define UMODE_NOCOLOUR    33 /* block message colours */                  /* +C */
#define UMODE_SENSITIVE   34 /* user has "sensitive ears" */              /* +G */

#define UMODE_ROUTING     35 /* user is on routing team */                /* +L */

#define UMODE_KILLPROT    36 /* user cannot be killed, except by NA */    /* +K */
#define UMODE_WANTSWHOIS  37 /* Whois notifications */                    /* +W */

/* macros for tests... cleaner code */

#define IsInvisible(x)          (HasUmode(x, UMODE_INVISIBLE))
#define IsSetCallerId(x)        (HasUmode(x, UMODE_CALLERID))
#define IsPMFiltered(x)         (HasUmode(x, UMODE_PMFILTER))
#define IsIdentified(x)         (HasUmode(x, UMODE_IDENTIFY))
#define IsBlockInvite(x)        (HasUmode(x, UMODE_BLOCKINVITE))
#define IsDeaf(x)               (HasUmode(x, UMODE_DEAF))
#define IsSensitive(x)          (HasUmode(x, UMODE_SENSITIVE))
#define BlockColour(x)          (HasUmode(x, UMODE_NOCOLOUR))
#define IsSecure(x)             (HasUmode(x, UMODE_SECURE))
#define IsCloaked(x)            (HasUmode(x, UMODE_CLOAK))
#define IsHiding(x)             (HasUmode(x, UMODE_HIDEOPER))

#define IsOper(x)               (HasUmode(x, UMODE_OPER))
#define IsAdmin(x)              (HasUmode(x, UMODE_ADMIN))
#define IsHelpop(x)             (HasUmode(x, UMODE_HELPOP))
#define IsNetadmin(x)           (HasUmode(x, UMODE_NETADMIN))
#define IsTechadmin(x)          (HasUmode(x, UMODE_TECHADMIN))
#define IsServicesRoot(x)       (HasUmode(x, UMODE_SVSROOT))
#define IsServicesAdmin(x)      (HasUmode(x, UMODE_SVSADMIN))
#define IsServicesOper(x)       (HasUmode(x, UMODE_SVSOPER))
#define IsService(x)            (HasUmode(x, UMODE_SERVICE))
#define IsRouting(x)            (HasUmode(x, UMODE_ROUTING))

#define SendRejections(x)       (HasUmode(x, UMODE_REJ))
#define SendNotices(x)          (HasUmode(x, UMODE_SERVNOTICE))
#define SendConnects(x)         (HasUmode(x, UMODE_CCONN))
#define SendServerKills(x)      (HasUmode(x, UMODE_SKILL))
#define SendFullNotice(x)       (HasUmode(x, UMODE_FULL))
#define SendDebugNotice(x)      (HasUmode(x, UMODE_DEBUG))
#define SendNickChange(x)       (HasUmode(x, UMODE_NCHANGE))
#define SendWallops(x)          (HasUmode(x, UMODE_WALLOP))
#define SendOperwall(x)         (HasUmode(x, UMODE_OPERWALL))
#define SendBotNotices(x)       (HasUmode(x, UMODE_BOTS))
#define SendExternalNotices(x)  (HasUmode(x, UMODE_EXTERNAL))
#define SendLocops(x)           (HasUmode(x, UMODE_LOCOPS))
#define SendUnauthorized(x)     (HasUmode(x, UMODE_UNAUTH))

extern FLAG_ITEM user_mode_table[];
extern void  init_umodes(void);
extern char *umodes_as_string(user_modes *);
extern void umodes_from_string(user_modes *, char *);
extern char *umode_difference(user_modes *, user_modes *);
extern user_modes *build_umodes(user_modes *, int, ...);
extern int available_slot;
extern void register_umode(char, int, int, int);
extern void setup_umodesys(void);

#endif

