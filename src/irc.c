/*
 * NetworkBOPM: The ShadowIRCd Anti-Proxy System.
 * irc.c: IRC parsing functions
 *
 * Major portions of this area of the code borrowed from shrike.
 *
 * $Id: irc.c,v 1.1 2004/05/24 23:22:41 nenolod Exp $
 */

#include "netbopm.h"

/* this splits apart a message with origin and command picked off already */
static int tokenize(char *message, char **parv)
{
  char *pos = NULL;
  char *next;
  uint8_t count = 0;

  if (!message)
    return -1;

  /* first we fid out of there's a : in the message, save that string
   * somewhere so we can set it to the last param in parv
   * also make sure there's a space before it... if not then we're screwed
   */
  pos = message;
  while (1)
  {
    if ((pos = strchr(pos, ':')))
    {
      pos--;
      if (*pos != ' ')
      {
        pos += 2;
        continue;
      }
      *pos = '\0';
      pos++;
      *pos = '\0';
      pos++;
      break;
    }
    else
      break;
  }

  /* now we take the beginning of the message and find all the spaces...
   * set them to \0 and use 'next' to go through the string
   */

  next = message;
  parv[0] = message;
  count = 1;

  while (*next)
  {
    if (count == 19)
    {
      /* we've reached one less than our max limit
       * to handle the parameter we alreayd ripped off
       */
      printf("tokenize(): reached para limit\n");
      return count;
    }
    if (*next == ' ')
    {
      *next = '\0';
      next++;
      /* eat any additional spaces */
      while (*next == ' ')
        next++;
      /* if it's the end of the string, it's simply
       * an extra space before the :parameter. break.
       */
      if (*next == '\0')
        break;
      parv[count] = next;
      count++;
    }
    else
      next++;
  }

  if (pos)
  {
    parv[count] = pos;
    count++;
  }

  return count;
}


void irc_parse(char *line)
{
  char *origin = NULL;
  char *pos;
  char *command = NULL;
  char *message = NULL;
  char *parv[20];
  static char coreLine[BUFSIZE];
  unsigned int parc = 0;
  unsigned int i;
  struct message_ *m;

  /* clear the parv */
  for (i = 0; i < 20; i++)
    parv[i] = NULL;

  if (line != NULL)
  {
    /* sometimes we'll get a blank line with just a \n on it...
     * catch those here... they'll core us later on if we don't
     */
    if (*line == '\n')
      return;
    if (*line == '\000')
      return;

    /* copy the original line so we know what we crashed on */
    memset((char *)&coreLine, '\0', BUFSIZE);
    strlcpy(coreLine, line, BUFSIZE);

    printf(">> %s\n", line);

    /* find the first space */
    if ((pos = strchr(line, ' ')))
    {
      *pos = '\0';
      pos++;
      /* if it starts with a : we have a prefix/origin
       * pull the origin off into `origin', and have pos for the
       * command, message will be the part afterwards
       */
      if (*line == ':')
      {
        origin = line;
        origin++;
        if ((message = strchr(pos, ' ')))
        {
          *message = '\0';
          message++;
          command = pos;
        }
        else
        {
          command = pos;
          message = NULL;
        }
      }
      else
      {
        message = pos;
        command = line;
      }
    }
    /* okay, the nasty part is over, now we need to make a
     * parv out of what's left
     */

    if (message)
    {
      if (*message == ':')
      {
        message++;
        parv[0] = message;
        parc = 1;
      }
      else
        parc = tokenize(message, parv);
    }
    else
      parc = 0;

    /* now we should have origin (or NULL), command, and a parv
     * with it's accompanying parc... let's make ABSOLUTELY sure
     */
    if (!command)
    {
      printf("irc_parse(): command not found: %s\n", coreLine);
      return;
    }

    /* take the command through the hash table */
    if ((m = msg_find(command)))
      if (m->func)
        m->func(origin, parc, parv);
  }
}

static void m_pass(char *origin, uint8_t parc, char *parv[])
{
  if (strcmp(me.password, parv[0]))
  {
    printf("password mismatch from uplink, aborting\n");
  }
}

static void m_ping(char *origin, uint8_t parc, char *parv[])
{
  sendto_server(":%s PONG %s %s", me.svname, me.svname, parv[0]);
}

static void m_eob(char *origin, uint8_t parc, char *parv[])
{
  /* End of burst notification */
    if (me.bursting == 1) {
      e_time(burstime, &burstime);

      printf("e_time worked\n");

      sendto_server(":%s WALLOPS :Finished synching to network in %d %s.",
            me.svname,
            (tv2ms(&burstime) >
             1000) ? (tv2ms(&burstime) / 1000) : tv2ms(&burstime),
            (tv2ms(&burstime) > 1000) ? "s" : "ms");

      me.bursting = 0;  
   }
}

static void m_uid(char *origin, uint8_t parc, char *parv[])
{
  sendto_server(":%s NOTICE %s :Scanning your IP (%s) for open proxies.",
	bopmuid, parv[7], parv[6]);

  return;
}


/* message table */
struct message_ messages[] = {
  { "PASS",    m_pass    },
  { "PING",    m_ping    },
  { "EOB",     m_eob     },
  { "UID",     m_uid     },
  { NULL }
};

struct message_ *msg_find(const char *name)
{
  struct message_ *m;

  for (m = messages; m->name; m++)
    if (!strcasecmp(name, m->name))
      return m;

  return NULL;
}

