/*
 * shadowircd: an advanced IRC daemon.
 * umodes.c: New usermode system, derived from dancer-ircd's usermode code.
 *
 * $Id: umodes.c,v 3.7 2004/09/25 06:00:28 nenolod Exp $
 */
#include "stdinc.h"
#include "s_user.h"
#include "umodes.h"

FLAG_ITEM user_mode_table[256] = {
  { 0, 0, 0 }
};

int user_modes_from_c_to_bitmask[256];
char umode_list[256];
int available_slot = 0;

struct  bitfield_lookup_t bitfield_lookup[BITFIELD_SIZE];

void init_umodes(void)
{
  unsigned int i, field, bit;
  char *p = umode_list;
  available_slot = 0;
  /* Fill out the reverse umode map */
  for (i = 0; user_mode_table[i].letter; i++)
    {
      user_modes_from_c_to_bitmask[(unsigned char)user_mode_table[i].letter] = user_mode_table[i].mode;
      *p++ = user_mode_table[i].letter;
      available_slot++;
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
    if (user_modes_from_c_to_bitmask[(unsigned char) *flags])
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

/*
 * register_umode: adds a umode to the ircd :)
 *
 * input        - letter, slot, operonly (true or false)
 * output       - none
 * side effects - umode is added to the umode table
 */
void
register_umode(char letter, int slot, int operonly, int rebuild)
{
  user_mode_table[available_slot].letter = letter;
  user_mode_table[available_slot].mode = slot;
  user_mode_table[available_slot].operonly = operonly;

  available_slot++;

  if (rebuild == 1)
    init_umodes();
}

/* 
 * MAKE SURE THE LAST ENTRY IN HERE USES 1 AS THE FOURTH OPTION! 
 * This will ensure that the umode table is properly generated!
 *       --nenolod
 */
void
setup_umodesys(void)
{
  register_umode('s', UMODE_SERVNOTICE, 0, 0);
  register_umode('c', UMODE_CCONN, 1, 0);
  register_umode('r', UMODE_REJ, 1, 0);
  register_umode('k', UMODE_SKILL, 1, 0);
  register_umode('f', UMODE_FULL, 1, 0);
  register_umode('y', UMODE_SPY, 1, 0);
  register_umode('d', UMODE_DEBUG, 1, 0);
  register_umode('n', UMODE_NCHANGE, 1, 0);
  register_umode('w', UMODE_WALLOP, 0, 0);
  register_umode('z', UMODE_OPERWALL, 1, 0);
  register_umode('i', UMODE_INVISIBLE, 1, 0);
  register_umode('b', UMODE_BOTS, 1, 0);
  register_umode('x', UMODE_EXTERNAL, 1, 0);
  register_umode('g', UMODE_CALLERID, 0, 0);
  register_umode('u', UMODE_UNAUTH, 1, 0);
  register_umode('l', UMODE_LOCOPS, 1, 0);
  register_umode('o', UMODE_OPER, 1, 0);
  register_umode('a', UMODE_ADMIN, 1, 0);
  register_umode('e', UMODE_IDENTIFY, 1, 0);
  register_umode('h', UMODE_HIDEOPER, 1, 0);
  register_umode('v', UMODE_CLOAK, 0, 0);
  register_umode('q', UMODE_PROTECTED, 1, 0);
  register_umode('I', UMODE_BLOCKINVITE, 0, 0);
  register_umode('E', UMODE_PMFILTER, 0, 0);
  register_umode('H', UMODE_HELPOP, 1, 0);
  register_umode('O', UMODE_SVSOPER, 1, 0);
  register_umode('A', UMODE_SVSADMIN, 1, 0);
  register_umode('R', UMODE_SVSROOT, 1, 0);
  register_umode('S', UMODE_SERVICE, 1, 0);
  register_umode('Z', UMODE_SECURE, 1, 0);
  register_umode('D', UMODE_DEAF, 1, 0);
  register_umode('N', UMODE_NETADMIN, 1, 0);
  register_umode('T', UMODE_TECHADMIN, 1, 0);
  register_umode('C', UMODE_NOCOLOUR, 0, 0);
  register_umode('G', UMODE_SENSITIVE, 0, 0);
  register_umode('L', UMODE_ROUTING, 1, 0);
  register_umode('K', UMODE_KILLPROT, 1, 0);
  register_umode('W', UMODE_WANTSWHOIS, 1, 1);
}
