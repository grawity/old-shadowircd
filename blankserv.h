/*
 *  [BlankServ].h
 *  kageserv
 *
 *  Created by Administrator on Sun May 30 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef [BlankServ]_H
#define [BlankServ]_H

#include <iostream>
#include <string>
#include "server.h"
#include "services.h"
#include "parser.h"
#include "protocol.h"
#include "simulation.h"

typedef struct
{
  string hook;
  void (*exec)(Server*, Service*, Parser*, Simulation*);
} [BS]Cmd;

class [BlankServ] : public Service
{
  public:
    [BlankServ](string name, string nick, string user, string host, string gecos);
	 void init(Server *server, string uid);
	 ~[BlankServ]();
};

void [BS]_pm_hook(Server*, Service*, Parser*, Simulation*);

void [BS]_help(Server*, Service*, Parser*, Simulation*);
void [BS]_register(Server*, Service*, Parser*, Simulation*);
void [BS]_userlist(Server*, Service*, Parser*, Simulation*);
void [BS]_beep(Server*, Service*, Parser*, Simulation*);
void [BS]_chanlist(Server*, Service*, Parser*, Simulation*);
void [BS]_raw(Server*, Service*, Parser*, Simulation*);
#endif
