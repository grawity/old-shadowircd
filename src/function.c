/*
 * NetworkBOPM: The ShadowIRCd Anti-Proxy System.
 * function.c: Various functions.
 *
 * $Id: function.c,v 1.1 2004/05/24 23:22:41 nenolod Exp $
 */

#include "netbopm.h"

/* translates microseconds into miliseconds */
long tv2ms(struct timeval *tv)
{
  return (tv->tv_sec * 1000) + (long) (tv->tv_usec / 1000);
}

void e_time(struct timeval sttime, struct timeval *ttime)
{
  struct timeval now;

  gettimeofday(&now, NULL);
  timersub(&now, &sttime, ttime);
}

void s_time(struct timeval *sttime)
{
  gettimeofday(sttime, NULL);
}

