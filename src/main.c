/*
 * NetworkBOPM: The ShadowIRCd Anti-Proxy System.
 * main.c: Main code
 *
 * $Id: main.c,v 1.3 2004/08/29 05:53:06 nenolod Exp $
 */

#include "netbopm.h"

#define ARRAY_SIZEOF(x) (sizeof(x) / sizeof((x)[0]))

extern int      sockfd;

int
main()
{
    daemon(1, 0);
    signal(SIGPIPE, SIG_IGN); /* it appears that libopm likes to break pipe */
    config_file = sstrdup("../data/networkbopm.conf");
    conf_parse();

    conn(me.uplink, me.port);

    while (sockfd < 0) {
	printf(".");
    }
    printf("\n");

    /*
     * Log into the server. 
     */
    sendto_server("PASS %s TS 6 %s", me.password, me.sid);
    sendto_server("CAPAB TS6 EOB QS EX UVH IE QU CLUSTER PARA ENCAP HUB");
    sendto_server("SERVER %s 1 :%s", me.svname, me.gecos);

    /*
     * start our burst timer 
     */
    s_time(&burstime);
    me.bursting = 1;

    bopmuid =
	init_psuedoclient("NetworkBOPM", "antiproxy", "NetworkBOPM",
			  "defender.linux-world.org");

    sendto_server(":%s SJOIN %lu %s + :%s",
                    me.sid, time(NULL), me.snoopchan, bopmuid);

    while (1) {
	/*
	 * Read a line and parse it. 
	 * Also, do OPM-related tasks.
	 */
	irc_read();
    }
}

OPM_T          *
opm_initialize(void)
{
    OPM_T          *scanner;
    int             fdlimit = 256;
    int             scan_port = 7000;
    int             max_read = 4096;
    int             scan_timeout = 10;
    unsigned int    i,
                    s;

    unsigned short  http_ports[] = {
	8000, 8080, 10000, 3128, 80
    };

    unsigned short  wingate_ports[] = {
	23
    };

    unsigned short  router_ports[] = {
	23
    };

    unsigned short  socks4_ports[] = {
	1080
    };

    unsigned short  socks5_ports[] = {
	1080
    };

    unsigned short  httppost_ports[] = {
	80, 8090, 3128, 8000, 8080
    };

    scanner = opm_create();

    /*
     * Configure the OPM scanner code -- callbacks are in 
     * callbacks.c
     */
    opm_callback(scanner, OPM_CALLBACK_OPENPROXY, &cb_open_proxy, NULL);
    opm_callback(scanner, OPM_CALLBACK_END, &cb_end, NULL);
    opm_callback(scanner, OPM_CALLBACK_ERROR, &cb_err, NULL);

    /*
     * Configure scanner settings. We use 32 FD's to be minimal.
     */
    opm_config(scanner, OPM_CONFIG_FD_LIMIT, &fdlimit);
    opm_config(scanner, OPM_CONFIG_SCAN_IP, &me.targetip);
    opm_config(scanner, OPM_CONFIG_SCAN_PORT, &scan_port);
    opm_config(scanner, OPM_CONFIG_TARGET_STRING,
	       "*** Looking up your hostname...");
    opm_config(scanner, OPM_CONFIG_TIMEOUT, &scan_timeout);
    opm_config(scanner, OPM_CONFIG_MAX_READ, &max_read);

    /*
     * Add the protocols we wish to scan to the list of protocols
     * that we scan.
     */
    for (s = ARRAY_SIZEOF(http_ports), i = 0; i < s; i++) {
	opm_addtype(scanner, OPM_TYPE_HTTP, http_ports[i]);
    }

    for (s = ARRAY_SIZEOF(wingate_ports), i = 0; i < s; i++) {
	opm_addtype(scanner, OPM_TYPE_WINGATE, wingate_ports[i]);
    }

    for (s = ARRAY_SIZEOF(router_ports), i = 0; i < s; i++) {
	opm_addtype(scanner, OPM_TYPE_ROUTER, router_ports[i]);
    }

    for (s = ARRAY_SIZEOF(socks4_ports), i = 0; i < s; i++) {
	opm_addtype(scanner, OPM_TYPE_SOCKS4, socks4_ports[i]);
    }

    for (s = ARRAY_SIZEOF(socks5_ports), i = 0; i < s; i++) {
	opm_addtype(scanner, OPM_TYPE_SOCKS5, socks5_ports[i]);
    }

    for (s = ARRAY_SIZEOF(httppost_ports), i = 0; i < s; i++) {
	opm_addtype(scanner, OPM_TYPE_HTTPPOST, httppost_ports[i]);
    }

    return scanner;
}
