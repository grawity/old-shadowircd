/*
 *  ChanServ.cpp
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
#include "chanserv.h"
#include "services.h"
#include "parser.h"
#include "simulation.h"
#include "server.h"
#include "netsimul.h" // for Net*
#include "datastorage.h"
using namespace std;

static csCmd cspmcmds[] = { // MUST BE IN ALPHABETICAL ORDER AND ALL CAPS!!!!!
  {"REGISTER", &cs_register},
  {"", NULL} // DO NOT REMOVE
};

ChanServ::ChanServ(string name, string nick, string user,
                        string host, string gecos)
					: Service(name, nick, user, host, gecos)
{
} /// constructor

void ChanServ::init(Server *server, string uid_)
{
  cout << name << " calling parent init" << endl;
  Service::init(server, uid_);
  cout << name << " joining #kageserv" << endl;
  Protocol::sjoin(*server, server->local_sid, SERVICECHAN, "Ont", uid.c_str());
  //server->puts(":%s MODE #kageserv -n", uid.c_str());
  //server->puts(":%s PRIVMSG 72AAAAAAN :Connected", uid.c_str());
  (this->addHook)("PRIVMSG", this, &cs_pm_hook);
  (this->addHook)("SJOIN", this, &cs_sjoin_hook);
  database_connect();
}

ChanServ::~ChanServ()
{
  Protocol::quit(*serv, uid.c_str(), "Service Shutting Down");
}

void cs_help(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  server->puts(":%s NOTICE %s :ChanServ HELP",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :------------",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :No help available.",
  serv->uid.c_str(), p->sender);
}

/* EXAMPLE
void cs_[COMMAND](Server *server, Service *serv, Parser *p, Simulation *sim)
{
  vector<string> args = splitstr(p->longarg, " ", [ARGCOUNT]);
  map<string,NetUser>::iterator user = sim->users.find(string(p->sender));
  if (user == sim->users.end())
  {
    server->puts(":%s WALLOPS :Unknown User %s", serv->uid.c_str(), p->sender);
  }
  else if (user->second.mode.find("o") == string::npos)
  {
    // Oper only command
  }
  else
  {
    // Normal user command 
  }
}
*/

void cs_register(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  // REGISTER #channel password description
  vector<string> args = splitstr(p->longarg, " ", 4);
  map<string,NetUser>::iterator user = sim->users.find(string(p->sender));
  int result;
  
  if (user == sim->users.end())
  {
    server->puts(":%s WALLOPS :Unknown User %s", serv->uid.c_str(), p->sender);
  }
  else
  {
    // Normal user command 
    if (args.size() < 4)
    {
      server->puts(":%s NOTICE %s :Usage: \002/msg %s REGISTER \037#channel\037 \037password\037 \037description\037\002", serv->uid.c_str(), p->sender, serv->nick.c_str());
      return;
    }
    result = database_query("SELECT * FROM user_identification WHERE user_id = '%s';",
               database_escape(user->second.uid).c_str() );
    if (result <= 0)
    {
      server->puts(":%s NOTICE %s :Please identify or register with services before using this command.",
                   serv->uid.c_str(), p->sender, serv->nick.c_str());
      return;
    }
    string channel = args[1];
    string password = args[2];
    string description = args[3];
    result = database_query("SELECT * FROM chanserv_channels WHERE name = '%s';",
               database_escape(channel).c_str() );
    if (result > 0)
    {
      server->puts(":%s NOTICE %s :Channel \002%s\002 is already registered!",
                   serv->uid.c_str(), p->sender, channel.c_str());
      return;
    }
    result = database_query("INSERT INTO chanserv_channels VALUES (NULL,'%s',MD5('%s'),'%s','%s',NOW());",
               database_escape(channel).c_str(), database_escape(password).c_str(),
               database_escape(user->second.nick).c_str(), database_escape(description).c_str() );
    if (result > 0)
    {
      database_query("INSERT INTO chanserv_channels_config (id) VALUES (NULL);");
      server->puts(":%s NOTICE %s :Channel \002%s\002 registered under your nickname: \002%s\002",
                   serv->uid.c_str(), p->sender, channel.c_str(), user->second.nick.c_str());
      server->puts(":%s NOTICE %s :Your channel password is \002%s\002 -- remember it for later use.",
                   serv->uid.c_str(), p->sender, password.c_str());
      Protocol::tmode(*server, serv->uid.c_str(), channel.c_str(), "+r", "");
    } // rows were inserted
  }
}

void cs_pm_hook(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  if (serv->uid != p->args && serv->nick != p->args)
    return; // channel or non-ChanServ message
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
  csCmd *c;
  bool matched = false;
  for (c = cspmcmds; c->exec && command > c->hook; c++);
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

void cs_sjoin_hook(Server *server, Service *serv, Parser *p, Simulation *sim)
{ 
  // [<SID>] [<TS> <CHAN> +<UMODE>] [<USERS>]
  vector<string> args = splitstr(p->args, " ");
  vector<string> users = splitstr(p->longarg, " ");
  string channelname = args[1];
  transform(channelname.begin(), channelname.end(), channelname.begin(), downcase);
  map<string,NetChannel>::iterator chan = sim->channels.find(channelname);
  int result;
  result = database_query("SELECT * FROM chanserv_channels WHERE name = '%s';",
                           database_escape(channelname).c_str() );
  if (chan == sim->channels.end())
  {
    server->puts(":%s WALLOPS :Channel %s is unknown", server->local_sid, channelname.c_str());
    return;
  }
  if (result > 0 && users.size() == chan->second.users.size())
  {
    // \002 002 BOLD
    // \006 006 BLINK
    // \026 022 INVERSE
    // \037 031 UNDERLINE
    server->puts(":%s NOTICE %s :Channel \002%s\002 is registered with %s.",
                 server->local_sid, channelname.c_str(), channelname.c_str(), serv->nick.c_str());
    server->puts(":%s NOTICE %s :Channel modes can be reset by typing \002/msg %s RESET %s MODES\002",
                 server->local_sid, channelname.c_str(), serv->nick.c_str(), channelname.c_str());
  }
} // ChanServ SJOIN

extern "C" Service* load()
{
  return new ChanServ(ChanServ("ChanServ", "ChnServ", "services",
               SERVICEHOST, "Channel Services"));
}

extern "C" void unload(Service* ob)
{
  delete ob;
}

extern "C" char *list()
{
  return "ChanServ";
}