/*
 * NetworkBOPM: The ShadowIRCd Anti-Proxy Solution.
 * utility.c: Utility functions.
 *
 * $Id: utility.c,v 1.2 2004/05/26 15:13:39 nenolod Exp $
 */

#include "netbopm.h"

char           *
genUID(void)
{
    static char     uid[9];
    int             i = 0;

    sprintf(uid, "%s", me.sid);

    while (i != 6) {
	sprintf(uid, "%s%c", uid, (char) ('A' + random() % 26));
	i++;
    }

    return uid;
}
