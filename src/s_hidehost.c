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
 *  $Id: s_hidehost.c,v 1.2 2004/07/15 18:21:56 nenolod Exp $
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

char *calcmask(char *myip, char *yourip)
{
    int cnt;
    unsigned char arr[16];
    char res[HOSTLEN], out[HOSTLEN];

    md5_buffer(myip, HOSTLEN, arr);

    sprintf(res, "%01x", arr[0]);

    for (cnt = 1; cnt <= 15; ++cnt)
    {
        sprintf(out, "%01x", arr[cnt]);
        strcat(res, out);
    }

    strcpy(yourip, res);
    return yourip;
}

void
make_virthost (char *curr, char *new) {
	calcmask(curr, new);
	return;
}
