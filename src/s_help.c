#include "struct.h"
#include "common.h"
#include "sys.h"
#include "h.h"
#include "dynconf.h"
#include "numeric.h"
#include "msg.h"

int
m_help (aClient * client_p, aClient * source_p, int parc, char *parv[])
{
 int i;
 for (i = 0; msgtab[i].cmd; i++)
   sendto_one(source_p, ":%s NOTICE %s :%s",
               me.name, parv[0], msgtab[i].cmd);
 return 0;
}
