/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  s_hidehost.c: Usercloaking.
 *
 *  Copyright (C) 2004 by the past and present ircd coders, and others.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 *  $Id: s_hidehost.c,v 1.3 2004/07/18 04:07:58 nenolod Exp $
 */

#include "stdinc.h"
#include "tools.h"
#include "s_user.h"
#include "s_conf.h"
#include "channel.h"
#include "channel_mode.h"
#include "client.h"
#include "common.h"
#include "sprintf_irc.h"
#include "md5.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <utmp.h>

unsigned int
convert_md5_to_int(unsigned char *i)
{
	char r[4];

        r[0] = i[0] ^ i[1] ^ i[2] ^ i[3];
        r[1] = i[4] ^ i[5] ^ i[6] ^ i[7];
        r[2] = i[8] ^ i[9] ^ i[10] ^ i[11];
        r[3] = i[12] ^ i[13] ^ i[14] ^ i[15];

        return ( ((unsigned int)r[0] << 24) +
                 ((unsigned int)r[1] << 16) +
                 ((unsigned int)r[2] << 8) +
                 (unsigned int)r[3]);
}

void
hidehost_ipv4 (char *host, char *out)
{
	unsigned int a, b, c, d;
	MD5_CTX hash;
	static char buf[512];
        static unsigned char res[512];
	unsigned int alpha, beta, gamma;

        sscanf(host, "%u.%u.%u.%u", &a, &b, &c, &d);

        ircsprintf(buf, "%s", host);
        MD5_Init(&hash);
        MD5_Update(&hash, buf, strlen(buf));
        MD5_Final(res, &hash);
        alpha = convert_md5_to_int(res);

        ircsprintf(buf, "%d.%d.%d", a, b, c);
        MD5_Init(&hash);
        MD5_Update(&hash, buf, strlen(buf));
        MD5_Final(res, &hash);
        beta = convert_md5_to_int(res);

        ircsprintf(buf, "%d.%d", a, b);
        MD5_Init(&hash);
        MD5_Update(&hash, buf, strlen(buf));
        MD5_Final(res, &hash);
        gamma = convert_md5_to_int(res);

        ircsprintf(out, "%X.%X.%X.IP", alpha, beta, gamma);
}

void
hidehost_normalhost(char *host, char *out)
{
	char *p;
	MD5_CTX hash;
	static char buf[512];
	static unsigned char res[512];
	unsigned int alpha;

	strcpy(buf, host);
        MD5_Init(&hash);
        MD5_Update(&hash, buf, strlen(buf));
        MD5_Final(res, &hash);
        alpha = convert_md5_to_int(res);

        for (p = host; *p; p++)
                if (*p == '.')
                        if (isalpha(*(p + 1)))
                                break;

        if (*p)
        {
                unsigned int len;
                p++;
                ircsprintf(out, "%s-%X.", ServerInfo.network_name, alpha);
                len = strlen(out) + strlen(p);
                if (len <= HOSTLEN)
                        strcat(out, p);
                else
                        strcat(out, p + (len - HOSTLEN));
        } else
                ircsprintf(out, "%s-%X", ServerInfo.network_name, alpha);

}

void
make_virthost (struct Client *client_p)
{
	char *p;

	if (!strcmp(client_p->host, client_p->ipaddr))
                hidehost_ipv4(client_p->host, client_p->virthost);
	else
		hidehost_normalhost(client_p->host, client_p->virthost);

	return;
}

