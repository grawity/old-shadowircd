#include "struct.h"
#include "common.h"
#include "sys.h"
#include "numeric.h"
#include "msg.h"
#include "channel.h"
#include "throttle.h"
#include "h.h"

static char buf[BUFSIZE];

void send_snomask (aClient *, aClient *, int, int, char *);
void send_snomask_out (aClient *, aClient *, int);

int snomask_table[] = {
  /* 0 - 32 are control chars and space */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0,                            /* ! */
  0,                            /* " */
  0,                            /* # */
  0,                            /* $ */
  0,                            /* % */
  0,                            /* & */
  0,                            /* ' */
  0,                            /* ( */
  0,                            /* ) */
  0,                            /* * */
  0,                            /* + */
  0,                            /* , */
  0,                            /* - */
  0,                            /* . */
  0,                            /* / */
  0,                            /* 0 */
  0,                            /* 1 */
  0,                            /* 2 */
  0,                            /* 3 */
  0,                            /* 4 */
  0,                            /* 5 */
  0,                            /* 6 */
  0,                            /* 7 */
  0,                            /* 8 */
  0,                            /* 9 */
  0,                            /* : */
  0,                            /* ; */
  0,                            /* < */
  0,                            /* = */
  0,                            /* > */
  0,                            /* ? */
  0,                            /* @ */
  0,                            /* A */
  0,                            /* B */
  0,                      /* C */
  SNOMASK_DCC,                      /* D */
  0,                            /* E */
  0,                      /* F */
  0,                            /* G */
  0,                            /* H */
  0,                            /* I */
  0,                            /* J */
  0,                            /* K */
  0,                            /* L */
  0,                            /* M */
  SNOMASK_NCHANGE,                            /* N */
  0,                      /* O */
  0,                      /* P */
  0,                            /* Q */
  0,                            /* R */
  SNOMASK_SPAM,                      /* S */
  0,                            /* T */
  0,                            /* U */
  0,                            /* V */
  0,                      /* W */
  0,                            /* X */
  0,                            /* Y */
  0,                      /* Z */
  0,                            /* [ */
  0,                            /* \ */
  0,                            /* ] */
  0,                            /* ^ */
  0,                            /* _ */
  0,                            /* ` */
  0,                      /* a */
  0,                            /* b */
  SNOMASK_CONNECTS,                      /* c */
  SNOMASK_DEBUG,                      /* d */
  0,                      /* e */
  SNOMASK_FLOOD,                      /* f */
  SNOMASK_GLOBOPS,                      /* g */
  0,                      /* h */
  0,                      /* i */
  0,                      /* j */
  SNOMASK_KILLS,                      /* k */
  SNOMASK_LINKS,                            /* l */
  0,                      /* m */
  SNOMASK_NETINFO,                      /* n */
  0,                      /* o */
  0,                      /* p */
  0,                            /* q */
  SNOMASK_REJECTS,                      /* r */
  0,                      /* s */
  0,                            /* t */
  0,                            /* u */
  0,                            /* v */
  0,                      /* w */
  0,                      /* x */
  SNOMASK_SPY,                      /* y */
  0,                            /* z */
  0,                            /* { */
  0,                            /* | */
  0,                            /* } */
  0,                            /* ~ */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 127 - 141 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 142 - 156 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 157 - 171 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 172 - 186 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 187 - 201 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 202 - 216 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 217 - 231 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 232 - 246 */
  0, 0, 0, 0, 0, 0, 0, 0, 0     /* 247 - 255 */
};

int snomask_chars[] = {
  SNOMASK_CONNECTS, 'c', /* Local Connects */
  SNOMASK_DEBUG, 'd', /* Debug Notices */
  SNOMASK_DCC, 'D', /* DCC warnings */
  SNOMASK_FLOOD, 'f', /* Flood warnings */
  SNOMASK_GLOBOPS, 'g', /* receive globops */
  SNOMASK_REJECTS, 'r', /* receive rejection notices */
  SNOMASK_KILLS, 'k', /* server kills */
  SNOMASK_SPAM, 'S', /* antispam */
  SNOMASK_NETINFO, 'n', /* network infromation */
  SNOMASK_SPY, 'y', /* stats/tracee requests */
  SNOMASK_NCHANGE, 'N', /* nick changes */
  SNOMASK_LINKS, 'l', /* links requests */
  0, 0
};

int
m_snomask (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
  int flag;
  int *s;
  char **p, *m;
  aClient *target_p;
  int what, setflags;
  int badflag = NO;             

  if (!IsAnOper (source_p))
    {
      sendto_one (source_p, ":%s NOTICE %s :*** %s -- this command is restricted to IRCops.",
	me.name, parv[0], "SNOMASK");
      return 0;
    }

  what = MODE_ADD;
  if (parc < 2)
    {
      sendto_one (source_p, err_str (ERR_NEEDMOREPARAMS),
                  me.name, parv[0], "SNOMASK");
      return 0;
    }

  if (!(target_p = find_person (parv[1], NULL)))
    {
      if (MyConnect (source_p))
        sendto_one (source_p, err_str (ERR_NOSUCHCHANNEL),
                    me.name, parv[0], parv[1]);
      return 0;
    }

  if ((IsServer (source_p) || (source_p != target_p)
       || (target_p->from != source_p->from)))
    {
      if (!IsServer (client_p))
        sendto_one (source_p, err_str (ERR_USERSDONTMATCH), me.name, parv[0]);
      return 0;
    }

  if (parc < 3)
    {
      m = buf;
      for (s = snomask_chars; (flag = *s) && (m - buf < BUFSIZE - 4); s += 2)
        {
          if (source_p->snomask & flag)
            *m++ = (char) (*(s + 1));
        }
      *m = '\0';
      sendto_one (source_p, rpl_str (RPL_SNOMASKIS), me.name, parv[0], buf);
      return 0;
    }

  /*
   * find flags already set for user
   */
  setflags = 0;
  for (s = snomask_chars; (flag = *s); s += 2)
    if (source_p->snomask & flag)
      setflags |= flag;

  /*
   * parse mode change string(s)
   */
  for (p = &parv[2]; p && *p; p++)
    for (m = *p; *m; m++)
      switch (*m)
        {
        case '+':
          what = MODE_ADD;
          break;
        case '-':
          what = MODE_DEL;
          break;
        default:
          if ((flag = snomask_table[(unsigned char) *m]))
            {
              if (what == MODE_ADD)
                {
                  source_p->snomask |= flag;
                }
              else
                {
                  source_p->snomask &= ~flag;
                }
            }
          else
            {
              if (MyConnect (source_p))
                {
                  badflag = YES;
                }
            }
          break;
       }
   (void) send_snomask_out (client_p, source_p, setflags);
   return 0;
}

char snomaskstring[128];

void
build_snomaskstr (void)
{
  int *s;
  char *m;

  m = snomaskstring;

  for (s = snomask_chars; *s; s += 2)
    {
      *m++ = (char) (*(s + 1));
    }

  *m = '\0';
}

void
send_snomask_out (aClient * client_p, aClient * source_p, int old)
{
  if (client_p && MyClient (client_p))
    {
      (void) send_snomask (client_p, source_p, old, ALL_UMODES, buf);
    }
}

void
send_snomask (aClient * client_p,
            aClient * source_p, int old, int sendmask, char *umode_buf)
{
  int *s, flag;
  char *m;
  /*
   * build a string in umode_buf to represent the change in the user's
   * mode between the new (source_p->flag) and 'old'.
   */
  m = umode_buf;
      m = buf;
      for (s = snomask_chars; (flag = *s) && (m - buf < BUFSIZE - 4); s += 2)
        {
          if (client_p->snomask & flag)
            *m++ = (char) (*(s + 1));
        }
      *m = '\0';
  if (*umode_buf && client_p)
    sendto_one (client_p, rpl_str(RPL_SNOMASKIS), me.name, source_p->name,
                umode_buf);
}

