/*
 * NetworkBOPM: The ShadowIRCd Anti-Proxy Solution.
 * callbacks.c: Callbacks from the OPM context.
 *
 * $Id: callbacks.c,v 1.5 2004/08/29 06:48:12 nenolod Exp $
 */

#include "netbopm.h"

void
cb_open_proxy(char *ip)
{
    sendto_server
	(":%s KLINE * 86400 * %s :Open Proxy found on your host. Please visit www.blitzed.org/proxy?ip=%s for more information.",
	 bopmuid, ip, ip);
    snoop("[AUTOSCAN] Found open proxy for %s. Banning.", r->ip);
    is_complete = 1;
}

void
cb_end(char *ip)
{
    snoop("[AUTOSCAN] Proxyscan complete for %s.", r->ip);
    is_complete = 1;
}

