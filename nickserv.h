/*
 *  nickserv.h
 *  kageserv
 *
 *  Created by Administrator on Sun May 30 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef NICKSERV_H
#define NICKSERV_H

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
} NSCmd;

class NickServ : public Service
{
  public:
    NickServ(string name, string nick, string user, string host, string gecos);
	 void init(Server *server, string uid);
	 ~NickServ();
};

class NSAccount
{
  public:
    string name;
    string password;
    NSAccount(string name_, string pass_);
};

void ns_pm_hook(Server*, Service*, Parser*, Simulation*);
void ns_uid_hook(Server*, Service*, Parser*, Simulation*);

void ns_help(Server*, Service*, Parser*, Simulation*);
void ns_ident(Server*, Service*, Parser*, Simulation*);
void ns_sident(Server*, Service*, Parser*, Simulation*);
void ns_register(Server*, Service*, Parser*, Simulation*);
#endif