/*
 * NetworkBOPM: The ShadowIRCd Anti-Proxy System.
 * socket.c: Socket handling functions.
 *
 * $Id: socket.c,v 1.6 2004/08/29 06:48:12 nenolod Exp $
 */

#include "netbopm.h"

int             sockfd = -1;
char            irc_raw[1025];
int             irc_raw_length = 0;

/*
 * conn           - connects to uplink
 *
 * inputs         - hostname to connect to, port to connect to
 * outputs        - socket
 * side effects   - connection to server is established
 */

int
conn(int sock, char *host, unsigned int port)
{
    struct hostent *he;
    struct sockaddr_in their_addr;

    if ((he = gethostbyname(host)) == NULL) {
	perror("gethostbyname");
	exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("socket");
	exit(1);
    }

    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(port);
    their_addr.sin_addr = *((struct in_addr *) he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);

    connect(sock, (struct sockaddr *) &their_addr, sizeof(their_addr));

    return 0;
}

int
irc_read()
{
    int             len;
    char            ch;

    if (sockfd < 0)
	return -1;

    while ((len = read(sockfd, &ch, 1))) {
	if (len < 0) {
	    if (!sockfd) {
		printf("Lost connection.\n");
		exit(1);
	    }

	    printf("Read error.\n");
	    exit(1);
	}

	if (ch == '\r')
	    continue;

	if (ch == '\n') {
	    /*
	     * Zero out the rest of the string. 
	     */
	    irc_raw[irc_raw_length] = 0;

	    irc_raw_length = 0;

	    irc_parse((char *) &irc_raw);

	    break;
	}

	if (ch != '\r' && ch != '\n' && ch != 0)
	    irc_raw[irc_raw_length++] = ch;

	return 1;
    }

    return 0;
}

int
sendto_server(int sock, char *format, ...)
{
    va_list         ap;
    char            buf[512];
    unsigned int    len,
                    n;

    va_start(ap, format);
    vsnprintf(buf, 512, format, ap);

    if (sockfd == -1) {
	return 1;
    }

    printf("<< %s\n", buf);

    strncat(buf, "\r\n", 512);
    len = strlen(buf);
    /*
     * counting out stuff? 
     */

    /*
     * write it 
     */
    if ((n = write(sock, buf, len)) == -1) {
	if (errno != EAGAIN) {
	    close(sock);
	    sockfd = -1;
	    return 1;
	}
    }

    if (n != len)
	printf
	    ("Major WTF -- incomplete write to socket: length = %d written = %d",
	     len, n);

    va_end(ap);

    return 0;
}

int
snoop(char *format, ...)
{
    va_list         ap;
    char            buf[512];

    va_start(ap, format);
    vsnprintf(buf, 512, format, ap);
    sendto_server(sockfd, ":%s PRIVMSG %s :%s", bopmuid, me.snoopchan, buf);
    va_end(ap);
    return 0;
}


char           *
init_psuedoclient(char *nick, char *user, char *gecos, char *host)
{
    static char     uid[9];
    sprintf(uid, "%s", genUID());
    sendto_server(sockfd, ":%s UID %s 1 %lu +ioS %s %s 0.0.0.0 %s %s :%s",
		  me.sid, nick, time(NULL), user, host, uid, host, gecos);

    return uid;
}


