/*
 * timestr.h
 * HybServ2 Services by HybServ2 team
 *
 * $Id: timestr.h,v 1.1 2003/12/16 19:52:37 nenolod Exp $
 */

#ifndef INCLUDED_timestr_h
#define INCLUDED_timestr_h

#ifndef INCLUDED_sys_time_h
#include <sys/time.h>        /* struct timeval */
#define INCLUDED_sys_time_h
#endif

/*
 * Prototypes
 */

char *timeago(time_t timestamp, int flag);
long timestr(char *format);
struct timeval *GetTime(struct timeval *timer);
long GetGMTOffset(time_t unixtime);

#endif /* INCLUDED_timestr_h */
