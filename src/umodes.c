/*
 * shadowircd: an advanced IRC daemon.
 * umodes.c: New usermode system, derived from dancer-ircd's usermode code.
 *
 * $Id: umodes.c,v 1.6 2004/05/13 19:23:58 nenolod Exp $
 */
#include "stdinc.h"
#include "s_user.h"
#include "umodes.h"

FLAG_ITEM user_mode_table[256] = {
  { UMODE_SERVNOTICE,	's', 1 },
  { UMODE_CCONN,	'c', 1 },
  { UMODE_REJ,		'r', 1 },
  { UMODE_SKILL,	'k', 1 },
  { UMODE_FULL,		'f', 1 },
  { UMODE_SPY,		'y', 1 },
  { UMODE_DEBUG, 	'd', 1 },
  { UMODE_NCHANGE, 	'n', 1 },
  { UMODE_WALLOP,	'w', 1 },
  { UMODE_OPERWALL,	'z', 1 },
  { UMODE_INVISIBLE,	'i', 0 },
  { UMODE_BOTS,         'b', 1 },
  { UMODE_EXTERNAL,	'x', 1 },
  { UMODE_CALLERID, 	'g', 0 }, 
  { UMODE_UNAUTH, 	'u', 1 },
  { UMODE_LOCOPS,	'l', 1 },
  { UMODE_OPER,		'o', 1 },
  { UMODE_ADMIN,        'a', 1 },
  { UMODE_IDENTIFY, 	'e', 0 },
  { UMODE_HIDEOPER,     'h', 1 },
  { UMODE_CLOAK,	'v', 0 },
  { UMODE_BLOCKINVITE,	'I', 0 },
  { UMODE_PMFILTER,	'E', 0 },
  { UMODE_HELPOP,       'H', 1 },
  { UMODE_SVSOPER,	'O', 1 },
  { UMODE_SVSADMIN,	'A', 1 },
  { UMODE_SVSROOT,	'R', 1 },
  { UMODE_SERVICE,      'S', 1 },
  { UMODE_SECURE,	'Z', 1 },
  { UMODE_DEAF,		'D', 0 },
  { UMODE_NETADMIN,	'N', 1 },
  { UMODE_TECHADMIN,	'T', 1 },
  { UMODE_NOCOLOUR, 	'C', 0 },
  { UMODE_SENSITIVE,    'G', 0 },
  { UMODE_ROUTING,      'L', 1 },
  { UMODE_WANTSWHOIS, 	'W', 1 },
  { 0, 0, 0 }
};

int user_modes_from_c_to_bitmask[256];
char umode_list[256];

struct  bitfield_lookup_t bitfield_lookup[BITFIELD_SIZE];

void init_umodes(void)
{
  unsigned int i, field, bit;
  char *p = umode_list;
  /* Fill out the reverse umode map */
  for (i = 0; user_mode_table[i].letter; i++)
    {
      user_modes_from_c_to_bitmask[(unsigned char)user_mode_table[i].letter] = user_mode_table[i].mode;
      *p++ = user_mode_table[i].letter;
    }
  *p = '\0';
  /* Fill out the bitfield map */
  bit = 1;
  field = 0;
  for (i = 0; i < BITFIELD_SIZE; i++)
    {
      bitfield_lookup[i].bit = bit;
      bitfield_lookup[i].field = field;
      if (bit == 0x80000000)
        {
          field++;
          bit = 1;
        }
      else
        bit = bit << 1;
    }
}

char *
umodes_as_string(user_modes *flags)
{
  static char flags_out[MAX_UMODE_COUNT];
  char *flags_ptr;
  int i;

  flags_ptr = flags_out;
  *flags_ptr = '\0';

  for (i = 0; user_mode_table[i].letter; i++ )
    {
      if (TestBit(*flags, user_mode_table[i].mode))
        *flags_ptr++ = user_mode_table[i].letter;
    }
  *flags_ptr = '\0';
  return(flags_out);
}

void
umodes_from_string(user_modes *u, char *flags)
{
  for(; *flags; flags++)
    SetBit((*u), user_modes_from_c_to_bitmask[(unsigned char)*flags]);
}

user_modes *
build_umodes(user_modes *u, int modes, ...)
{
  va_list args;
  int i;

  va_start(args, modes);
  for (i = 0; i < modes; i++)
    SetBit(*u, user_mode_table[va_arg(args, unsigned int)].mode);
  va_end(args);
  return u;
}

char *
umode_difference(user_modes *old_modes, user_modes *new_modes)
{
  static char buf[MAX_UMODE_COUNT+3];
  int i,flag;
  char *p = buf;
  char t = '=';

  /* Do this twice because we don't want +a-b+c-d and so on, we
   * want -bd+ac
   */
  for (i = 0; user_mode_table[i].letter; i++ )
    {
      flag = user_mode_table[i].mode;

      if (TestBit(*old_modes, flag) && !TestBit(*new_modes, flag))
        {
          if (t != '-')
            t = *p++ = '-';
          *p++ = user_mode_table[i].letter;
        }
    }

  for (i = 0; user_mode_table[i].letter; i++ )
    {
      flag = user_mode_table[i].mode;

      if (!TestBit(*old_modes, flag) && TestBit(*new_modes, flag))
        {
          if (t != '+')
            t = *p++ = '+';
          *p++ = user_mode_table[i].letter;
        }
    }
  *p = '\0';
  return buf;
}

