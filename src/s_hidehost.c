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
 *  $Id: s_hidehost.c,v 1.5 2004/07/18 12:24:34 nenolod Exp $
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

#define KEY1 ServerInfo.network_cloakkey1
#define KEY2 ServerInfo.network_cloakkey2
#define KEY3 ServerInfo.network_cloakkey3

unsigned int
convert_md5_to_int (unsigned char *i)
{
  char r[4];

  r[0] = i[0] ^ i[1] ^ i[2] ^ i[3];
  r[1] = i[4] ^ i[5] ^ i[6] ^ i[7];
  r[2] = i[8] ^ i[9] ^ i[10] ^ i[11];
  r[3] = i[12] ^ i[13] ^ i[14] ^ i[15];

  return (((unsigned int) r[0] << 24) +
	  ((unsigned int) r[1] << 16) +
	  ((unsigned int) r[2] << 8) + (unsigned int) r[3]);
}

/* This function is used at config parsing time.
 * If your cloak keys are not random enough, the ircd will complain.
 */
int
check_for_randomness (char *key)
{
  int gotlowcase = 0, gotupcase = 0, gotdigit = 0;
  char *p;

  for (p = key; *p; p++)
    if (islower (*p))
      gotlowcase = 1;
    else if (isupper (*p))
      gotupcase = 1;
    else if (isdigit (*p))
      gotdigit = 1;

  if (gotlowcase && gotupcase && gotdigit)
    return 0;
  return 1;
}

void
hidehost_ipv4 (char *host, char *out)
{
  unsigned int a, b, c, d;
  MD5_CTX hash;
  static char buf[512];
  static unsigned char res[512];
  unsigned int alpha, beta, gamma;

  sscanf (host, "%u.%u.%u.%u", &a, &b, &c, &d);

  ircsprintf (buf, "%s-%s-%s-%s", KEY1, KEY2, host, KEY3);
  MD5_Init (&hash);
  MD5_Update (&hash, buf, strlen (buf));
  MD5_Final (res, &hash);
  alpha = convert_md5_to_int (res);

  ircsprintf (buf, "%d-%s.%s-%d.%d-%s", a, KEY1, KEY2, b, c, KEY3);
  MD5_Init (&hash);
  MD5_Update (&hash, buf, strlen (buf));
  MD5_Final (res, &hash);
  beta = convert_md5_to_int (res);

  ircsprintf (buf, "%s-%d.%s.%d-%s", KEY3, a, KEY2, b, KEY1);
  MD5_Init (&hash);
  MD5_Update (&hash, buf, strlen (buf));
  MD5_Final (res, &hash);
  gamma = convert_md5_to_int (res);

  ircsprintf (out, "%X.%X.%X.IP", alpha, beta, gamma);
}

void
hidehost_ipv6(char *host, char *out)
{
	unsigned int a, b, c, d, e, f, g, h;
	MD5_CTX hash;
	static char buf[512];
	static unsigned char res[512];
	unsigned int alpha, beta, gamma;

        sscanf(host, "%x:%x:%x:%x:%x:%x:%x:%x",
                &a, &b, &c, &d, &e, &f, &g, &h);

        /* ALPHA... */
        ircsprintf(buf, "%s-%s-%s-%s", KEY2, host, KEY3, KEY1);
        MD5_Init(&hash);
        MD5_Update(&hash, buf, strlen(buf));
        MD5_Final(res, &hash);
        alpha = convert_md5_to_int(res);

        /* BETA... */
        ircsprintf(buf, "%s-%x:%x:%x:%x-%s-%x:%x:%x-%s", KEY3, e, d, f, b, KEY2, a, g, c, KEY1);
        MD5_Init(&hash);
        MD5_Update(&hash, buf, strlen(buf));
        MD5_Final(res, &hash);
        beta = convert_md5_to_int(res);

        /* GAMMA... */
        ircsprintf(buf, "%s:%x:%x-%s-%x:%x:%s", KEY1, c, a, KEY3, d, b, KEY2);
        MD5_Init(&hash);
        MD5_Update(&hash, buf, strlen(buf));
        MD5_Final(res, &hash);
        gamma = convert_md5_to_int(res);

        ircsprintf(out, "%X:%X:%X:IP", alpha, beta, gamma);
}

void
hidehost_normalhost (char *host, char *out)
{
  char *p;
  MD5_CTX hash;
  static char buf[512];
  static unsigned char res[512];
  unsigned int alpha;

  ircsprintf(buf, "%s-%s-%s-%s", KEY3, host, KEY1, KEY2);
  MD5_Init (&hash);
  MD5_Update (&hash, buf, strlen (buf));
  MD5_Final (res, &hash);
  alpha = convert_md5_to_int (res);

  for (p = host; *p; p++)
    if (*p == '.')
      if (isalpha (*(p + 1)))
	break;

  if (*p)
    {
      unsigned int len;
      p++;
      ircsprintf (out, "%s-%X.", ServerInfo.network_name, alpha);
      len = strlen (out) + strlen (p);
      if (len <= HOSTLEN)
	strcat (out, p);
      else
	strcat (out, p + (len - HOSTLEN));
    }
  else
    ircsprintf (out, "%s-%X", ServerInfo.network_name, alpha);

}

void
make_virthost (struct Client *client_p)
{
  if (strchr (client_p->host, ':'))
    hidehost_ipv6 (client_p->host, client_p->virthost);

  else if (!strcmp (client_p->host, client_p->ipaddr))
    hidehost_ipv4 (client_p->host, client_p->virthost);

  else
    hidehost_normalhost (client_p->host, client_p->virthost);

  return;
}
