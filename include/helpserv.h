/*
 * helpserv.h
 * HybServ2 Services by HybServ2 team
 *
 * $Id: helpserv.h,v 1.1 2003/12/16 19:52:38 nenolod Exp $
 */

#ifndef INCLUDED_helpserv_h
#define INCLUDED_helpserv_h

#ifndef INCLUDED_config_h
#include "config.h"        /* HELPSERVICES */
#define INCLUDED_config_h
#endif

#ifdef HELPSERVICES

void hs_process(char *nick, char *command);

#endif /* HELPSERVICES */

void GiveHelp(char *serv, char *nick, char *command, int sockfd);

#endif /* INCLUDED_helpserv_h */
