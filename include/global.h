/*
 * global.h
 * HybServ2 Services by HybServ2 team
 *
 * $Id: global.h,v 1.1 2003/12/16 19:52:38 nenolod Exp $
 */

#ifndef INCLUDED_global_h
#define INCLUDED_global_h

#ifndef INCLUDED_config_h
#include "config.h"        /* GLOBALSERVICES */
#define INCLUDED_config_h
#endif

#ifdef GLOBALSERVICES

/*
 * Prototypes
 */

void gs_process(char *nick, char *command);

#endif /* GLOBALSERVICES */

#endif /* INCLUDED_global_h */
