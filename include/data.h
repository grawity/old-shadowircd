/*
 * data.h
 * HybServ2 Services by HybServ2 team
 *
 * $Id: data.h,v 1.1 2003/12/16 19:52:38 nenolod Exp $
 */

#ifndef INCLUDED_data_h
#define INCLUDED_data_h

#ifndef INCLUDED_stdio_h
#include <stdio.h>        /* FILE * */
#define INCLUDED_stdio_h
#endif

#ifndef INCLUDED_sys_types_h
#include <sys/types.h>        /* time_t */
#define INCLUDED_sys_types_h
#endif

#ifndef INCLUDED_operserv_h
#include "operserv.h"
#define INCLUDED_operserv_h
#endif

/*
 * Prototypes
 */

void BackupDatabases(time_t unixtime);
int ReloadData();
FILE *CreateDatabase(char *name, char *info);

int WriteDatabases();
int WriteOpers();
int WriteIgnores();
int WriteStats();
int WriteNicks();
int WriteChans();
int WriteMemos();

#ifdef SEENSERVICES
int WriteSeen();
#endif

void LoadData();
int ns_loaddata();
int cs_loaddata();
int ms_loaddata();
int ss_loaddata();
int os_loaddata();
int ignore_loaddata();

#ifdef SEENSERVICES
int es_loaddata();
#endif

#endif /* INCLUDED_data_h */
