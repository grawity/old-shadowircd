/*
 *  protocol.cpp
 *  kageserv
 *
 *  Created by Administrator on Sun May 23 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <string>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include "protocol.h"
using namespace std;

// :<SID> UID <NICK> <HOPS> <TS> +<UMODE> <USER> <HOST> <IP> <UID> <VHOST> :<GECOS>
bool Protocol::signon(Server serv,
                  const char *sid, const char *nick, int hops, const char *umode,
                  const char *user, const char *host, const char *vhost,
						const char *ip, const char *uid, const char *gecos)
{
  return serv.puts(":%s UID %s %d %d +%s %s %s %s %s %s :%s",
            sid, nick, hops, time(NULL), umode, user, host, ip, uid, vhost, gecos);
} // signon

// :<SID> SJOIN <TS> <CHAN> +<UMODE> :<USER>
bool Protocol::sjoin(Server serv,
                  const char *sid, const char *chan, const char *mode,
						const char *user)
{
  return serv.puts(":%s SJOIN %d %s +%s :%s",
            sid, time(NULL), chan, mode, user);
} // sjoin

bool Protocol::quit(Server serv, const char *uid, const char *reason)
{
  return serv.puts(":%s QUIT :%s", uid, reason);
} // quit

bool Protocol::part(Server serv, const char *uid, const char *chan, const char *reason)
{
  return serv.puts(":%s PART %s :%s", uid, chan, reason);
} // part

bool Protocol::tmode(Server serv, const char *uid, const char *chan, const char *modes, const char *args)
{
  return serv.puts(":%s TMODE %d %s %s %s", uid, 0/*time(NULL)*/, chan, modes, args);
}

bool Protocol::umode(Server serv, const char *id, const char *victim, const char *mode)
{
  return serv.puts(":%s MODE %s :%s", id, victim, mode);
}

const char *Protocol::genSID()
{
  static char buf[4];
  srandom(time(NULL));
  for (int i = 0; i < sizeof(buf)/sizeof(char); i++)
  {
    // NULL
    if (i == (sizeof(buf)/sizeof(char))-1)
	   buf[i] = 0x00;
	 
    // Number
    else if (i == 0)
	   buf[i] = '1'+(random()%10);
		
	 // Capital Letter
	 else
	   buf[i] = 'A'+(random()%26);
  } // generate each letter of the SID
  return buf;
}

const char *Protocol::genUID(const char *sid)
{
  static char buf[10];
  srandom(time(NULL));
  for (int i = 0; i < sizeof(buf)/sizeof(char); i++)
  {
    // NULL
    if (i == (sizeof(buf)/sizeof(char))-1)
	   buf[i] = 0x00;
	 
    // SID before ID
    else if (i >= 0 && i < 3)
	   buf[i] = *(sid+i);
		
	 // Capital Letter
	 else //if (i == 3 || random()%2 == 0)
	   buf[i] = 'A'+(random()%26);
		
	 // Number
	 //else
	 //  buf[i] = '0'+(random()%10);
  } // generate each letter of the SID
  return buf;
}

string Protocol::get_next_uid(string sid)
{
  return sid+add_one_to_uid(6-1);
}
string Protocol::add_one_to_uid(int i)
{
  static string new_uid;
  if (new_uid.empty())
    new_uid = "AAAAAA";
  else if (i >= 0)		/* Not reached server SID portion yet? */
  {
    if (new_uid[i] == 'Z')
      new_uid[i] = '0';
    else if (new_uid[i] == '9')
    {
      new_uid[i] = 'A';
      add_one_to_uid(i-1);
    }
    else new_uid[i] = new_uid[i] + 1;
  }
  else
  {
    new_uid = "AAAAAA";
  }
  return new_uid;
}
