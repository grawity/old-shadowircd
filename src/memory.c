/*
 * NetworkBOPM: The ShadowIRCd Anti-Proxy Solution.
 * memory.c: Memory safe functions.
 *
 * $Id: memory.c,v 1.1 2004/05/24 23:22:41 nenolod Exp $
 */

#include "netbopm.h"

void *scalloc(long elsize, long els) {
  void *buf = calloc(elsize, els);

  if (!buf)
    raise(SIGUSR1);
  return buf;
}

void *smalloc(long size) {
  void *buf = malloc(size);

  if (!buf)
    raise(SIGUSR1);
  return buf;
}

void *srealloc(void *oldptr, long newsize) {
  void *buf = realloc(oldptr, newsize);

  if (!buf)
    raise(SIGUSR1);
  return buf;
}

char *sstrdup (const char *s) {
  char *t = smalloc(strlen(s) + 1);

  strcpy(t, s);
  return t;
}
