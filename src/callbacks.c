/*
 * NetworkBOPM: The ShadowIRCd Anti-Proxy Solution.
 * callbacks.c: Callbacks from the OPM context.
 *
 * $Id: callbacks.c,v 1.6 2004/08/29 06:51:07 nenolod Exp $
 */

#include "netbopm.h"

extern int sockfd;

void
cb_open_proxy(char *ip, char *type)
{
    sendto_server
	(sockfd, ":%s KLINE * 86400 * %s :Open %s Proxy found on your host.",
	 bopmuid, ip, type);
    snoop("[AUTOSCAN] Found open %s proxy on %s. Banning.", type, ip);
    is_complete = 1;
}

void
cb_end(char *ip)
{
    snoop("[AUTOSCAN] Proxyscan complete for %s.", ip);
    is_complete = 1;
}

