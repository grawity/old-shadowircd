/*
 * NetworkBOPM: The ShadowIRCd Anti-Proxy System.
 * main.c: Main code
 *
 * $Id: main.c,v 1.4 2004/08/29 06:48:12 nenolod Exp $
 */

#include "netbopm.h"

extern int      sockfd;

int
main()
{
    daemon(1, 0);
    signal(SIGPIPE, SIG_IGN); /* it appears that libopm likes to break pipe */
    config_file = sstrdup("../data/networkbopm.conf");
    conf_parse();

    conn(sockfd, me.uplink, me.port);

    while (sockfd < 0) {
	printf(".");
    }
    printf("\n");

    /*
     * Log into the server. 
     */
    sendto_server(sockfd, "PASS %s TS 6 %s", me.password, me.sid);
    sendto_server(sockfd, "CAPAB TS6 EOB QS EX UVH IE QU CLUSTER PARA ENCAP HUB");
    sendto_server(sockfd, "SERVER %s 1 :%s", me.svname, me.gecos);

    /*
     * start our burst timer 
     */
    s_time(&burstime);
    me.bursting = 1;

    bopmuid =
	init_psuedoclient("NetworkBOPM", "antiproxy", "NetworkBOPM",
			  "defender.linux-world.org");

    sendto_server(sockfd, ":%s SJOIN %lu %s + :%s",
                    me.sid, time(NULL), me.snoopchan, bopmuid);

    while (1) {
	/*
	 * Read a line and parse it. 
	 * Also, do OPM-related tasks.
	 */
	irc_read();
    }
}

