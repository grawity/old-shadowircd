/*
 *  [BlankServ].cpp
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
#include "[BlankServ].h"
#include "services.h"
#include "parser.h"
#include "simulation.h"
#include "server.h"
#include "netsimul.h" // for Net*
using namespace std;

static [BS]Cmd [BS]pmcmds[] = { // MUST BE IN ALPHABETICAL ORDER AND ALL CAPS!!!!!
  {"BEEP", &[BS]_beep},
  {"CHANLIST", &[BS]_chanlist},
  {"HELP", &[BS]_help},
  {"RAW", &[BS]_raw},
  {"USERLIST", &[BS]_userlist},
  {"", NULL} // DO NOT REMOVE
};

[BlankServ]::[BlankServ](string name, string nick, string user,
                        string host, string gecos)
					: Service(name, nick, user, host, gecos)
{
} /// constructor

void [BlankServ]::init(Server *server, string uid_)
{
  cout << name << " calling parent init" << endl;
  Service::init(server, uid_);
  cout << name << " joining #kageserv" << endl;
  Protocol::sjoin(*server, server->local_sid, "#kageserv", "Ont", uid.c_str());
  //server->puts(":%s MODE #kageserv -n", uid.c_str());
  //server->puts(":%s PRIVMSG 72AAAAAAN :Connected", uid.c_str());
  (this->addHook)("PRIVMSG", this, &[BS]_pm_hook);
}

[BlankServ]::~[BlankServ]()
{
  Protocol::quit(*serv, uid.c_str(), "Service Shutting Down");
}

void [BS]_help(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  server->puts(":%s NOTICE %s :[BlankServ] HELP",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :------------",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :No help available.",
  serv->uid.c_str(), p->sender);
}

void [BS]_userlist(Server *server, Service *serv, Parser *p, Simulation *sim)
{
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
    user = sim->users.begin();
    string message;
    while(user != sim->users.end())
    {
      message = "User " + user->second.nick + "!" + user->second.ident;
      message += "@" + user->second.host + "(" + user->second.gecos + ")";
      server->puts(":%s NOTICE %s :%s", serv->uid.c_str(), p->sender, message.c_str());
      message = "  ID: " + user->second.uid + " Cloak: " + user->second.vhost;
      message += " Mode: " + user->second.mode;
      server->puts(":%s NOTICE %s :%s", serv->uid.c_str(), p->sender, message.c_str());
      user++;
    }
  }
}

void [BS]_beep(Server *server, Service *serv, Parser *p, Simulation *sim)
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
    cout << "BEEP " << args[1] << endl;
    map<string,NetChannel>::iterator chan = sim->channels.begin();
    string message;
    chan = sim->channels.find(args[1]);
    Protocol::sjoin(*server, server->local_sid, args[1].c_str(), "", serv->uid.c_str());
    if (chan != sim->channels.end())
    {
      cout << "Sending beeps to " << args[1] << endl;
      map<string,NetUser>::iterator user;
      vector<string>::iterator users = chan->second.users.begin();
      string temp;
      while (users != chan->second.users.end())
      {
        temp.assign((*users),
                    (*users).find_first_not_of("!@%^*+"),
                    (*users).size());
        user = sim->users.find(temp);
        if (user != sim->users.end())
        {
          message = "Beep beep, " + user->second.nick + "! >=)";
          server->puts(":%s PRIVMSG %s :%s", serv->uid.c_str(),
                                            args[1].c_str(),
                                            message.c_str());
        }
        users++;
      }
      Protocol::part(*server, serv->uid.c_str(), args[1].c_str(), "Beep Beep!");
    }
  }
}

void [BS]_chanlist(Server *server, Service *serv, Parser *p, Simulation *sim)
{
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
    map<string,NetChannel>::iterator chan = sim->channels.begin();
    string message;
    while(chan != sim->channels.end())
    {
      message = "Channel " + chan->second.name;
      server->puts(":%s NOTICE %s :%s", serv->uid.c_str(), p->sender, message.c_str());
      message = "  Mode: +" + chan->second.modes;
      server->puts(":%s NOTICE %s :%s", serv->uid.c_str(), p->sender, message.c_str());
      map<string,NetUser>::iterator user;
      vector<string>::iterator users = chan->second.users.begin();
      string temp;
      while (users != chan->second.users.end())
      {
        temp.assign((*users),
                    (*users).find_first_not_of("!@%^*+"),
                    (*users).size());
        user = sim->users.find(temp);
        if (user != sim->users.end())
        {
          message = "  " + user->second.nick;
          server->puts(":%s NOTICE %s :%s", serv->uid.c_str(),
                                            p->sender,
                                            message.c_str());
        }
        users++;
      }
      chan++;
    }
  }
}

void [BS]_raw(Server *server, Service *serv, Parser *p, Simulation *sim)
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
    server->puts(":%s WALLOPS :Unauthorized RAW from %s (%s)", serv->uid.c_str(),
      user->second.nick.c_str(), user->second.host.c_str());
  }
  else
  {
    cout << "args.size() = " << args.size() << endl;
    server->puts(":%s %s", serv->uid.c_str(), args[1].c_str()); 
  }
}

void [BS]_pm_hook(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  if (serv->uid != p->args && serv->nick != p->args)
    return; // channel or non-[BlankServ] message
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
  [BS]Cmd *c;
  bool matched = false;
  for (c = [BS]pmcmds; c->exec && command > c->hook; c++);
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

extern "C" Service* load()
{
  return new [BlankServ]([BlankServ]("[BlankServ]", "[BlankServ]", "services",
               SERVICEHOST, "???? Services"));
}

extern "C" void unload(Service* ob)
{
  delete ob;
}

extern "C" char *list()
{
  return "[BlankServ]";
}