/*
 *  services.h
 *  kageserv
 *
 *  Created by Administrator on Sun May 30 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef SERVICES_H
#define SERVICES_H

#include <iostream>
#include <string>
#include "server.h"
#include "parser.h"
#include "protocol.h"
#include "simulation.h"

void GenServices();
void StartServices(Server *server);
void ExecServices();

/*
typedef struct {
  unsigned privmsg: 1;
  unsigned chanmsg: 1;
  unsigned notice:  1;
  unsigned nick:    1;
  unsigned join:    1;
  unsigned mode:    1;
  unsigned kick:    1;
  unsigned topic:   1;
} Filter;
*/

class Service
{
  public:
	 string uid;
   string name;
	 string nick;
	 string user;
	 string host;
	 string gecos;
	 //Filter filter;
	 Server *serv;

   void (*addHook)(string, Service*,
                   void (*exec_)(Server *, Service*, Parser *, Simulation *) );
   

	 Service();
	 Service(string name, string nick, string user, string host, string gecos);
   //void setFilter(Filter filter);
	 virtual void init(Server *server, string uid);
};

class ServHook
{
  public:
    string hook;
    Service *parent;
    void (*exec)(Server*, Service*, Parser*, Simulation*);
    ServHook(string hook_, Service *parent_,
         void (*exec_)(Server*, Service*, Parser*, Simulation*));
};

void GenServices();
void InjectService(Server*, string);
void ExtractService(Server*, string);
void StartServices(Server *);
void ExecServices(Server *serv, Parser *parser, Simulation *sim);
void DelServices();
void AddHook(string hook_, Service *parent_,
             void (*exec_)(Server *, Service*, Parser *, Simulation *) );

#endif