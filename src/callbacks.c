/*
 * NetworkBOPM: The ShadowIRCd Anti-Proxy Solution.
 * callbacks.c: Callbacks from the OPM context.
 *
 * $Id: callbacks.c,v 1.3 2004/08/29 06:02:11 nenolod Exp $
 */

#include "netbopm.h"

void
cb_open_proxy(OPM_T * s, OPM_REMOTE_T * r, int notused, void *data)
{
    sendto_server
	(":%s KLINE * 86400 * %s :Open Proxy found on your host. Please visit www.blitzed.org/proxy?ip=%s for more information.",
	 bopmuid, r->ip, r->ip);
    snoop("[AUTOSCAN] Found open proxy for %s. Banning.", r-ip);
    opm_end(s, r);
    opm_free(s);
    is_complete = 1;
}

void
cb_end(OPM_T * s, OPM_REMOTE_T * r, int e, void *data)
{
    opm_remote_free(r);
    snoop("[AUTOSCAN] Proxyscan complete for %s.", r->ip);
    is_complete = 1;
}

void
cb_err(OPM_T * s, OPM_REMOTE_T * r, int e, void *data)
{
	snoop("There was an error. Errcode was: %d", e);
}
