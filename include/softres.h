/*
 * ShadowIRCd - An Internet Relay Chat Daemon
 *
 * See LICENSE for licensing information. I'm a lazy bastard.
 *
 * softres.h: Used to fix problems with the libresolv header on
 *            crackheaded systems.
 */

#include "setup.h"

#if (!defined (HAS_DN_SKIPNAME) || defined (HAS___DN_SKIPNAME))
#define dn_skipname __dn_skipname
#define dn_comp     __dn_comp
#endif

extern struct state __res;


