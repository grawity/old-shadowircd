/*
 *  OperServ.h
 *  kageserv
 *
 *  Created by Administrator on Sun May 30 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef OPERSERV_H
#define OPERSERV_H

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
} OSCmd;

class OperServ : public Service
{
  public:
    OperServ(string name, string nick, string user, string host, string gecos);
	 void init(Server *server, string uid);
	 ~OperServ();
};

void os_pm_hook(Server*, Service*, Parser*, Simulation*);

void os_help(Server*, Service*, Parser*, Simulation*);
void os_register(Server*, Service*, Parser*, Simulation*);
void os_userlist(Server*, Service*, Parser*, Simulation*);
void os_beep(Server*, Service*, Parser*, Simulation*);
void os_chanlist(Server*, Service*, Parser*, Simulation*);
void os_raw(Server*, Service*, Parser*, Simulation*);
void os_umode(Server*, Service*, Parser*, Simulation*);
void os_tmode(Server*, Service*, Parser*, Simulation*);
void os_flood(Server*, Service*, Parser*, Simulation*);
void os_global(Server*, Service*, Parser*, Simulation*);
#endif
