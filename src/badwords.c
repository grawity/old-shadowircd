/*
 *  shadowircd: An advanced Internet Relay Chat Daemon (ircd).
 *  src/badwords.c: Functions related to filtering.
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
 *  check_text borrowed from freeworld ircd, written by Hwy/Jeremy.
 *
 *  $Id: badwords.c,v 1.1 2004/04/07 19:11:44 nenolod Exp $
 */

#include "stdinc.h"
#include "handlers.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "common.h"
#include "s_conf.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

#include "channel.h"
#include "channel_mode.h"
#include "irc_string.h"
#include "hash.h"
#include "class.h"
#include "msg.h"
#include "packet.h"

char *check_text (char *origstr, char *search)
{
  char *source_p;
  char *target_p;
  static char target[BUFSIZE + 1];
  int slen, dlen;

  /* Check for NULL */
  if (!search)
  {
    sendto_realops_flags (UMODE_DEBUG, L_ALL, "Search string empty.");
    return origstr;
  }

  target[0] = '\0';
  source_p = origstr;
  target_p = target;
  slen = strlen(search);

  /* XXX Check for running over your BUFSIZE */
  while (*source_p != '\0')
  {
    if (strncasecmp(source_p, search, slen) == 0)
    {
      /* Found the target string */
      *target_p = '\0';
      strlcat(target_p, "<censored>", BUFSIZE + 1);
      target_p += 10; /* strlen("<censored>") */
      source_p += slen;
    }
    else
    {
      *target_p = *source_p;
      target_p++;
      source_p++;
    }
  }

  *target_p = '\0';
  return target;
}

