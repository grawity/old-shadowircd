/*
 *  simulation.h
 *  kageserv
 *
 *  Created by Administrator on Sun May 23 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <map>
#include <string>
#include "netsimul.h"
#include "parser.h"
#include "server.h"
using namespace std;

class Simulation;

typedef struct {
  char *hook;
  void (*exec)(Server *serv, Parser *parser, Simulation *sim);
} SimCmd;

class Simulation
{
  public:
    Server *server;
    map<string,NetUser> users;
    map<string,NetChannel> channels;
    vector<NetServer> servers;
  
    Simulation(Server *server);
    bool execute(Parser *parser);
}; // class Simulation

#endif
