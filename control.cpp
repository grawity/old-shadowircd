/*
 *  Control.cpp
 *  kageserv
 *
 *  Created by Administrator on Sun May 30 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include <string>
#include <algorithm> // for transform
#include <cctype> // for toupper
#include <utility> // for pair
#include <map> // for map =P
#include "config.h"
#include "protocol.h"
#include "control.h"
#include "services.h"
#include "parser.h"
#include "simulation.h"
#include "server.h"
#include "netsimul.h" // for Net*
using namespace std;

static CONCmd CONpmcmds[] = { // MUST BE IN ALPHABETICAL ORDER AND ALL CAPS!!!!!
  {"LOAD", &CON_load},
  {"RELOAD", &CON_reload},
  {"UNLOAD", &CON_unload},
  {"", NULL} // DO NOT REMOVE
};

static CONCmd CONwcmds[] = { // MUST BE IN ALPHABETICAL ORDER AND ALL CAPS!!!!!
  {"CONFIRM", &CON_null},
  {"DELINK", &CON_delink},
  {"DEOPER", &CON_deoper},
  {"", NULL} // DO NOT REMOVE
};

Control::Control(string name, string nick, string user,
                        string host, string gecos)
					: Service(name, nick, user, host, gecos)
{
} /// constructor

void Control::init(Server *server, string uid_)
{
  cout << name << " calling parent init" << endl;
  Service::init(server, uid_);
  cout << name << " joining #kageserv" << endl;
  Protocol::sjoin(*server, server->local_sid, SERVICECHAN, "Ont", uid.c_str());
  //server->puts(":%s MODE #kageserv -n", uid.c_str());
  //server->puts(":%s PRIVMSG 72AAAAAAN :Connected", uid.c_str());
  (this->addHook)("PRIVMSG", this, &CON_pm_hook);
  (this->addHook)("WALLOPS", this, &CON_wallops);
}

Control::~Control()
{
  Protocol::quit(*serv, uid.c_str(), "Service Shutting Down");
}

void CON_load(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  vector<string> args = splitstr(p->longarg, " ");
  map<string,NetUser>::iterator user = sim->users.find(string(p->sender));
  if (user == sim->users.end())
  {
    server->puts(":%s WALLOPS :Unknown User %s", serv->uid.c_str(), p->sender);
  }
  else if (user->second.mode.find("o") == string::npos)
  {
    server->puts(":%s NOTICE %s :Command Unavailable", serv->uid.c_str(), p->sender);
  }
  else
  {
    if (args.size() > 1)
    {
      InjectService(server, "./"+args[1]);
    }
  }
}

void CON_reload(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  vector<string> args = splitstr(p->longarg, " ");
  map<string,NetUser>::iterator user = sim->users.find(string(p->sender));
  if (user == sim->users.end())
  {
    server->puts(":%s WALLOPS :Unknown User %s", serv->uid.c_str(), p->sender);
  }
  else if (user->second.mode.find("o") == string::npos)
  {
    server->puts(":%s NOTICE %s :Command Unavailable", serv->uid.c_str(), p->sender);
  }
  else
  {
    if (args.size() > 1)
    {
      ExtractService(server, "./"+args[1]);
      InjectService(server, "./"+args[1]);
    }
  }
}

void CON_unload(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  vector<string> args = splitstr(p->longarg, " ");
  map<string,NetUser>::iterator user = sim->users.find(string(p->sender));
  if (user == sim->users.end())
  {
    server->puts(":%s WALLOPS :Unknown User %s", serv->uid.c_str(), p->sender);
  }
  else if (user->second.mode.find("o") == string::npos)
  {
    server->puts(":%s NOTICE %s :Command Unavailable", serv->uid.c_str(), p->sender);
  }
  else
  {
    if (args.size() > 1)
    {
      ExtractService(server, "./"+args[1]);
    }
  }
}

/////////////////////
//  //  //  //  // //
// //  //  //  //  //
/////////////////////

void CON_null(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  server->puts(":%s WALLOPS :Command Confirmed.", serv->uid.c_str());
}

void CON_delink(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  vector<string> args = splitstr(p->longarg, " ", 2);
  map<string,NetUser>::iterator user = sim->users.find(string(p->sender));
  if (user == sim->users.end())
  {
    server->puts(":%s WALLOPS :Unknown User %s", serv->uid.c_str(), p->sender);
  }
  else if (user->second.mode.find("o") == string::npos)
  {
    server->puts(":%s NOTICE %s :Command Unavailable", serv->uid.c_str(), p->sender);
    server->puts(":%s WALLOPS :User %s attempting to delink services without oper", serv->uid.c_str(), p->sender);
  }
  else
  {
    if (args.size() == 1)
      server->puts(":%s WALLOPS :Services shutting down (Requested by %s)",
        serv->uid.c_str(), user->second.nick.c_str());
    else
      server->puts(":%s WALLOPS :Services shutting down (%s)", serv->uid.c_str(),
        args[1].c_str());
    cout << "** Services shutting down" << endl;
    cout << "** Requested by: " << user->second.nick << endl;
    if (args.size() == 2)
    cout << "** Reason: " << args[1] << endl;
    server->disconnect();
    exit(0);
  }
}

void CON_deoper(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  vector<string> args = splitstr(p->longarg, " ", 2);
  map<string,NetUser>::iterator user;
  for (user = sim->users.begin(); user != sim->users.end(); user++)
  {
    if (user->second.nick == args[1])
    {
      server->puts(":%s WALLOPS :Deopering %s", serv->uid.c_str(), args[1].c_str());
      args[1] = user->second.uid;
      server->puts(":%s MODE %s :-o", server->local_sid, args[1].c_str());
      return;
    } // if the nick matches
  } // search all users
  server->puts(":%s WALLOPS :No users match \"%s\"", serv->uid.c_str(), args[1].c_str());
}

/////////////////////
//  //  //  //  // //
// //  //  //  //  //
/////////////////////

void CON_pm_hook(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  if (serv->uid != p->args && serv->nick != p->args)
    return; // channel or non-Control message
  string line = p->longarg;
  string command, args;
  if (line.find(" ") != string::npos)
  {
    command.assign(line, 0, line.find(" "));
    args.assign(line, line.find(" ")+1, line.size());
  }
  else
  {
    command = line;
    args = "";
  }
  transform(command.begin(), command.end(), command.begin(), upcase);
  CONCmd *c;
  bool matched = false;
  for (c = CONpmcmds; c->exec && command > c->hook; c++);
  cout << command << "vs." << c->hook << endl;
  while (command == c->hook)
  {
    cout << c->hook << endl;
    ((c++)->exec)(server, serv, p, sim);
    matched = true;
  }
  if (!matched)
    server->puts(":%s NOTICE %s :Unknown Command", serv->uid.c_str(), p->sender);
}

void CON_wallops(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  //if (serv->uid != p->args && serv->nick != p->args)
  //  return; // channel or non-Control message
  string line = p->longarg;
  string command, args;
  cout << line << endl;
  if (line.find(" ") != string::npos)
  {
    command.assign(line, 0, line.find(" "));
    args.assign(line, line.find(" ")+1, line.size());
  }
  else
  {
    command = line;
    args = "";
  }
  transform(command.begin(), command.end(), command.begin(), upcase);
  CONCmd *c;
  bool matched = false;
  for (c = CONwcmds; c->exec && command > c->hook; c++);
  cout << command << "vs." << c->hook << endl;
  while (command == c->hook)
  {
    cout << c->hook << endl;
    //server->puts(":%s WALLOPS :Confirmed.", server->local_sid);
    ((c++)->exec)(server, serv, p, sim);
    matched = true;
  }
}