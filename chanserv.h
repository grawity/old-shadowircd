/*
 *  ChanServ.h
 *  kageserv
 *
 *  Created by Administrator on Sun May 30 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ChanServ_H
#define ChanServ_H

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
} csCmd;

class ChanServ : public Service
{
  public:
    ChanServ(string name, string nick, string user, string host, string gecos);
	 void init(Server *server, string uid);
	 ~ChanServ();
};

void cs_pm_hook(Server*, Service*, Parser*, Simulation*);
void cs_sjoin_hook(Server*, Service*, Parser*, Simulation*);

void cs_register(Server*, Service*, Parser*, Simulation*);
#endif
