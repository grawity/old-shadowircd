/*
 *  Control.h
 *  kageserv
 *
 *  Created by Administrator on Sun May 30 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Control_H
#define Control_H

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
} CONCmd;

class Control : public Service
{
  public:
    Control(string name, string nick, string user, string host, string gecos);
	 void init(Server *server, string uid);
	 ~Control();
};

void CON_pm_hook(Server*, Service*, Parser*, Simulation*);
void CON_wallops(Server*, Service*, Parser*, Simulation*);

void CON_load(Server*, Service*, Parser*, Simulation*);
void CON_reload(Server*, Service*, Parser*, Simulation*);
void CON_unload(Server*, Service*, Parser*, Simulation*);

void CON_null(Server*, Service*, Parser*, Simulation*);
void CON_delink(Server*, Service*, Parser*, Simulation*);
void CON_deoper(Server*, Service*, Parser*, Simulation*);

#endif
