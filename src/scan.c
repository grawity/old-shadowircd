/*
 * $Id: scan.c,v 1.1 2004/08/29 07:08:49 nenolod Exp $
 */

#include "netbopm.h"

int scanfd;

static void do_scan_http(char *ip, int port)
{
   char *line;

   conn(scanfd, ip, port);
   if (scanfd > -1)
     sendto_server(scanfd, "CONNECT %s:6667 HTTP/1.1\n", me.targetip);
   line = my_readline(scanfd);
   if (!fnmatch("*connection established*", line, 0))
     cb_open_proxy(ip, "HTTP");
   close(scanfd);
}

void do_scan(char *ip)
{
   do_scan_http(ip, 80);
   do_scan_http(ip, 8080);
   do_scan_http(ip, 3128);
}

