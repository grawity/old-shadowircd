/*
 *  protocol.h
 *  kageserv
 *
 *  Created by Administrator on Sun May 23 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "server.h"
using namespace std;

class Protocol
{
  public:
    static bool signon(Server serv,
	                    const char *sid, const char *nick, int hops,
							  const char *umode, const char *user, const char *host,
							  const char *vhost, const char *ip,
							  const char *uid, const char *gecos);
    static bool sjoin(Server serv,
                  const char *sid, const char *chan, const char *mode, const char *user);
    static bool quit(Server serv, const char *uid, const char *reason);
    static bool part(Server serv, const char *uid, const char *ch, const char *resn);
    static bool tmode(Server, const char *, const char *, const char *, const char *);
    static bool umode(Server, const char *, const char *, const char *);
    static const char *genSID();
    static const char *genUID(const char *sid);
    static string get_next_uid(string sid);
    static string add_one_to_uid(int i);
}; // class Protocol

#endif
