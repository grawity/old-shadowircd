/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  motd.c: Message of the day functions.
 *
 *  Copyright (C) 2002 by the past and present ircd coders, and others.
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
 *  $Id: motd.c,v 1.4 2004/01/15 20:34:41 nenolod Exp $
 */

#include "stdinc.h"
#include "tools.h"
#include "motd.h"
#include "ircd.h"
#include "fdlist.h"
#include "s_bsd.h"
#include "fileio.h"
#include "s_conf.h"
#include "send.h"
#include "numeric.h"
#include "client.h"
#include "irc_string.h"
#include "sprintf_irc.h"
#include "memory.h"
#include "s_serv.h"

/*
** init_message_file
**
*/
void
init_message_file(MotdType motdType, const char *fileName, MessageFile *motd)
{
  strlcpy(motd->fileName, fileName, sizeof(motd->fileName));
  motd->motdType = motdType;
  motd->contentsOfFile = NULL;
  motd->lastChangedDate[0] = '\0';
}

/*
** send_message_file
**
** This function split off so a server notice could be generated on a
** user requested motd, but not on each connecting client.
*/
int
send_message_file(struct Client *source_p, MessageFile *motdToPrint)
{
  MessageFileLine *linePointer;
  MotdType motdType;
  const char *from, *to;

  if (motdToPrint == NULL)
    return(-1);

  motdType = motdToPrint->motdType;

  if (!MyConnect(source_p) && IsCapable(source_p->from, CAP_TS6) && HasID(source_p))
  {
    from = me.id;
    to = source_p->id;
  }
  else
  {
    from = me.name;
    to = source_p->name;
  }

  switch (motdType)
  {
    case USER_MOTD:
      if (motdToPrint->contentsOfFile == NULL)
      {
        sendto_one(source_p, form_str(ERR_NOMOTD),
                   from, to);
        return(0);
      }

      sendto_one(source_p, form_str(RPL_MOTDSTART),
                 from, to, me.name);

      for (linePointer = motdToPrint->contentsOfFile; linePointer;
           linePointer = linePointer->next)
      {
        sendto_one(source_p, form_str(RPL_MOTD),
                   from, to, linePointer->line);
      }

      sendto_one(source_p, form_str(RPL_ENDOFMOTD), from, to);
      return(0);
      /* NOT REACHED */
      break;

    case SHRT_MOTD:
      if (motdToPrint->contentsOfFile == NULL)
      {
        sendto_one(source_p, form_str(ERR_NOMOTD),
                   from, to);
        return(0);
      }

      sendto_one(source_p, form_str(RPL_MOTDSTART),
                 from, to, me.name);

      for (linePointer = motdToPrint->contentsOfFile; linePointer;
           linePointer = linePointer->next)
      {
        sendto_one(source_p, form_str(RPL_MOTD),
                   from, to, linePointer->line);
      }

      sendto_one(source_p, form_str(RPL_ENDOFMOTD), from, to);
      return(0);
      /* NOT REACHED */
      break;

    case OPER_MOTD:
      if (motdToPrint->contentsOfFile == NULL)
      {
/*      sendto_one(source_p, ":%s NOTICE %s :No OPER MOTD", me.name,
 *                 source_p->name); */
        return(-1);
      }

      sendto_one(source_p,":%s NOTICE %s :Start of OPER MOTD",
                 from, to);
      break;

    default:
      return(0);
      /* NOT REACHED */
  }

  sendto_one(source_p, ":%s NOTICE %s :%s", from, to,
             motdToPrint->lastChangedDate);

  for (linePointer = motdToPrint->contentsOfFile; linePointer;
       linePointer = linePointer->next)
  {
    sendto_one(source_p, ":%s NOTICE %s :%s",
               from, to, linePointer->line);
  }

  sendto_one(source_p, ":%s NOTICE %s :End",
             from, to);
  return(0);
}

/*
 * read_message_file() - original From CoMSTuD, added Aug 29, 1996
 *
 * inputs	- pointer to MessageFileptr
 * output	-
 * side effects	-
 */
int
read_message_file(MessageFile *MessageFileptr)
{
  struct stat sb;
  struct tm *local_tm;

  /* used to clear out old MessageFile entries */
  MessageFileLine *mptr = 0;
  MessageFileLine *next_mptr = 0;

  /* used to add new MessageFile entries */
  MessageFileLine *newMessageLine = 0;
  MessageFileLine *currentMessageLine = 0;

  char buffer[MESSAGELINELEN];
  char *p;
  FBFILE *file;

  for (mptr = MessageFileptr->contentsOfFile; mptr; mptr = next_mptr)
  {
    next_mptr = mptr->next;
    MyFree(mptr);
  }

  MessageFileptr->contentsOfFile = NULL;

  if (stat(MessageFileptr->fileName, &sb) < 0)
    return(-1);

  local_tm = localtime(&sb.st_mtime);

  if (local_tm)
    ircsprintf(MessageFileptr->lastChangedDate,
               "%d/%d/%d %d:%02d",
               local_tm->tm_mday,
               local_tm->tm_mon + 1,
               1900 + local_tm->tm_year,
               local_tm->tm_hour,
               local_tm->tm_min);

  if ((file = fbopen(MessageFileptr->fileName, "r")) == NULL)
    return(-1);

  while (fbgets(buffer, sizeof(buffer), file))
  {
    if ((p = strchr(buffer, '\n')) != NULL)
      *p = '\0';

    newMessageLine = (MessageFileLine *)MyMalloc(sizeof(MessageFileLine));
    strlcpy(newMessageLine->line, buffer, sizeof(newMessageLine->line));
    newMessageLine->next = NULL;

    if (MessageFileptr->contentsOfFile != NULL)
    {
      if (currentMessageLine)
        currentMessageLine->next = newMessageLine;

      currentMessageLine = newMessageLine;
    }
    else
    {
      MessageFileptr->contentsOfFile = newMessageLine;
      currentMessageLine = newMessageLine;
    }
  }

  fbclose(file);
  return(0);
}
