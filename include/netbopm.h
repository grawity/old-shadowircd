/*
 * NetworkBOPM: The ShadowIRCd Anti-Proxy System.
 * netbopm.h: Unified headerfile.
 *
 * <licensing info here>
 *
 * $Id: netbopm.h,v 1.1.1.1 2004/05/24 23:22:47 nenolod Exp $
 */

#ifndef INCLUDED_NETBOPM_H
#define INCLUDED_NETBOPM_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <ctype.h>
#include <regex.h>
#include <fcntl.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "sysconf.h"

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <grp.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/socket.h>

/* me structure */

struct me_ {
   char *password; /* Password */
   char *sid;      /* Server ID */ 
   char *uplink;   /* Uplink */
   char *gecos;    /* GECOS */
   char *svname;   /* Server Name */
   int port;       /* Connection Port */
   int bursting;   /* Are we bursting? */
};

typedef struct me_ me_t;

me_t me;

/* struct for irc message hash table */
struct message_
{
  const char *name;
  void (*func) (char *origin, uint8_t parc, char *parv[]);
};

typedef struct message_ message_t;

#define BUFSIZE 512

void irc_parse(char *);
int sendto_server(char *, ...);

struct message_ *msg_find(const char *);
long tv2ms(struct timeval *tv);
void e_time(struct timeval, struct timeval *);
void s_time(struct timeval *);
struct timeval burstime;

typedef struct node_ node_t;
typedef struct list_ list_t;

/* dlink stuff */
struct node_ {
  node_t *next, *prev;
  void *data;
};

struct list_ {
  node_t *head, *tail;
  int count;
};

/* macros for dlink stuff */
#define LIST_FOREACH(n, head) for (n = (head); n; n = n->next)
#define LIST_FOREACH_NEXT(n, head) for (n = (head); n->next; n = n->next)
#define LIST_LENGTH(list) (list)->count
#define LIST_FOREACH_SAFE(n, tn, head) for (n = (head), tn = n ? n->next : NULL; n != NULL; n = tn, tn = n ? n->next : NULL)

node_t *node_create(void);
void    node_free(node_t *);
void    node_add(void *, node_t *, list_t *);
void    node_remove(node_t *, list_t *);
node_t *node_find(void *, list_t *);

/* memory stuff */
void   *scalloc(long, long);
void   *smalloc(long);
void   *srealloc(void *, long);
char   *sstrdup(const char *);

/* Ok, now for the fun stuff. */
typedef struct user_ user_t;
typedef struct server_ server_t;

struct server_ {
  char *name;
  char *sid;
  char *gecos;

  server_t *uplink;
};

struct user_ {
  char *nick;
  char *user;
  char *host;
  char *uid;
  char *vhost;
  char *ip;

  server_t *server;

  long flags;
};

list_t userlist;
list_t serverlist;

server_t *server_create(void);
server_t *server_find(char *);

/* NetworkBOPM noticing client UID */
char *bopmuid;

char *genUID(void);

int conn (char *, unsigned int);
int irc_read (void);
char *init_psuedoclient(char *, char *, char *, char *);

/* Configuration stuff */
typedef struct _configfile CONFIGFILE;
typedef struct _configentry CONFIGENTRY;

struct _configfile {
  char *cf_filename;
  CONFIGENTRY *cf_entries;
  CONFIGFILE *cf_next;
};

struct _configentry {
  CONFIGFILE *ce_fileptr;

  int ce_varlinenum;
  char *ce_varname;
  char *ce_vardata;
  
  int ce_vardatanum;
  int ce_fileposstart;
  int ce_fileposend;

  int ce_sectlinenum;

  CONFIGENTRY *ce_entries;
  CONFIGENTRY *ce_prevlevel;
  CONFIGENTRY *ce_next;
};

void conf_parse(void);
CONFIGFILE *config_load(char *);
char *config_file;
void config_free (CONFIGFILE *);
void conf_init(void);
int conf_rehash(void);
int conf_check(void);
CONFIGENTRY *config_find(CONFIGENTRY *, char *);
#endif
