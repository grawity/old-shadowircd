/*
 *  nickserv.cpp
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
#include "md5.h" // for *gasp* md5!
#include "config.h"
#include "protocol.h"
#include "nickserv.h"
#include "services.h"
#include "parser.h"
#include "simulation.h"
#include "server.h"
#include "netsimul.h" // for Net*
#include "datastorage.h"
using namespace std;

static map<string,NSAccount> accounts;
static NSCmd pmcmds[] = { // MUST BE IN ALPHABETICAL ORDER AND ALL CAPS!!!!!
  {"HELP", &ns_help},
  {"IDENTIFY", &ns_ident},
  {"SIDENTIFY", &ns_ident},
  {"REGISTER", &ns_register},
  {"", NULL} // DO NOT REMOVE
};

NickServ::NickServ(string name, string nick, string user,
                        string host, string gecos)
					: Service(name, nick, user, host, gecos)
{
} /// constructor

NSAccount::NSAccount(string name_, string pass_)
{
  name = name_;
  password = pass_;
}

void NickServ::init(Server *server, string uid)
{
  cout << name << " calling parent init" << endl;
  Service::init(server, uid);
  cout << name << " joining #kageserv" << endl;
  Protocol::sjoin(*server, server->local_sid, SERVICECHAN, "Ont", uid.c_str());
  //server->puts(":%s MODE #kageserv -n", uid.c_str());
  //server->puts(":%s PRIVMSG 72AAAAAAN :Connected", uid.c_str());
  (this->addHook)("PRIVMSG", this, &ns_pm_hook);
  (this->addHook)("UID", this, &ns_uid_hook);
  database_connect();
}

NickServ::~NickServ()
{
  Protocol::quit(*serv, uid.c_str(), "Service Shutting Down");
}

void ns_help(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  server->puts(":%s NOTICE %s :NickServ HELP",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :------------",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :NickServ is a nickname registration service",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :NickServ allows users to register nicknames, and",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :keep that nickname for themselves.  A range of ",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :security settings are available to help manage the",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :account.  All NickServ accounts have passwords",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :associated with them.  A user must identify",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :with nickserv before they will be allowed to use",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :any NickServ commands.",
  serv->uid.c_str(), p->sender);
  server->puts(":%s NOTICE %s :No other help available.",
  serv->uid.c_str(), p->sender);
}

void ns_register(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  string command, args;
  pair<string, string> cpair = splitCommand(p->longarg);
  command = cpair.first;
  args = cpair.second;
  if (args.size() <= 0)
    server->puts(":%s NOTICE %s :You must specify a password",
                 serv->uid.c_str(), p->sender);
  else
  {
    map<string,NetUser>::iterator user = sim->users.find(p->sender);
    if (user == sim->users.end())
    {
      server->puts(":%s WALLOPS :Unknown user %s", server->local_sid, p->sender);
    }
    else
    {
      string lowername = user->second.nick;
      //transform(lowername.begin(), lowername.end(), lowername.begin(), downcase);
      //map<string,NSAccount>::iterator acct = accounts.find(lowername);
      //if (acct == accounts.end())
      if (database_query("SELECT name FROM nickserv_users WHERE name = '%s';", 
                         database_escape(lowername).c_str() ) == 0)
      {
        //accounts.insert(
        //  make_pair( lowername, NSAccount( string(user->second.nick) , md5sum(args.c_str()) ) )
        //               );
        database_query("INSERT INTO nickserv_users VALUES (NULL,'%s',MD5('%s'),1,'%s',NOW());",
                        database_escape(lowername).c_str(), database_escape(args).c_str(), 
                        database_escape(user->second.uid).c_str());
        database_query("INSERT INTO user_identification VALUES ('%s',NOW());",
                        database_escape(user->second.uid).c_str());
        server->puts(":%s NOTICE %s :Nickname \002%s\002 has been registered to you.",
                     serv->uid.c_str(), p->sender, user->second.nick.c_str());
        server->puts(":%s NOTICE %s :Your password is \002%s\002 -- remember this for later use.",
                     serv->uid.c_str(), p->sender, args.c_str());
        server->puts(":%s PRIVMSG #kageserv :Created account for %s (%s)",
                     serv->uid.c_str(), user->second.nick.c_str(), md5sum(args.c_str()));
        server->puts(":%s MODE %s :+e", server->local_sid, user->second.uid.c_str());
      } // no account exists yet
      else
      {
        server->puts(":%s NOTICE %s :Nickname %s is already registered!",
                     serv->uid.c_str(), p->sender, user->second.nick.c_str());
      } // account exists
    } // valid user in database
  }
} // REGISTER

void ns_ident(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  string command, args;
  pair<string, string> cpair = splitCommand(p->longarg);
  command = cpair.first;
  args = cpair.second;
  if (args.size() <= 0)
    server->puts(":%s NOTICE %s :You must specify a password",
                 serv->uid.c_str(), p->sender);
  else
  {
    map<string,NetUser>::iterator user = sim->users.find(p->sender);
    if (user == sim->users.end())
    {
      server->puts(":%s WALLOPS :Unknown user %s", server->local_sid, p->sender);
    }
    else
    {
      string lowername = user->second.nick;
      //transform(lowername.begin(), lowername.end(), lowername.begin(), downcase);
      //map<string,NSAccount>::iterator acct = accounts.find(lowername);
      //if (acct == accounts.end())
      if (database_query("SELECT name FROM nickserv_users WHERE name = '%s';",
                         database_escape(lowername).c_str()) == 0)
      {
        server->puts(":%s NOTICE %s :Your nickname is not registered.",
                     serv->uid.c_str(), p->sender);
      } // no account exists yet
      else if (database_query("SELECT name FROM nickserv_users WHERE name = '%s' AND password = MD5('%s');",
                         database_escape(lowername).c_str(), database_escape(args).c_str()) == 0)
      {
        server->puts(":%s NOTICE %s :Password incorrect.",
                     serv->uid.c_str(), p->sender);
      } // wrong password
      else
      {
        if (database_query("UPDATE nickserv_users SET ident_id = '%s', identified = 1 WHERE name = '%s';",
                       database_escape(user->second.uid).c_str(), database_escape(lowername).c_str()) > 0)
        {
          database_query("INSERT INTO user_identification VALUES ('%s',NOW());",
                        database_escape(user->second.uid).c_str());
          server->puts(":%s NOTICE %s :Password accepted -- you are now recognized.",
                       serv->uid.c_str(), p->sender);
          server->puts(":%s MODE %s :+e", server->local_sid, p->sender);
        }
        else
        {
          server->puts(":%s NOTICE %s :Password acceoted -- you are still recognized.",
                       serv->uid.c_str(), p->sender);
          server->puts(":%s MODE %s :+e", server->local_sid, p->sender);
        }
      } // account exists and password is correct
    } // valid user in database
  }
} // IDENT

void ns_sident(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  string command, args;
  pair<string, string> cpair = splitCommand(p->longarg);
  command = cpair.first;
  args = cpair.second;
  if (args.size() <= 0)
    server->puts(":%s NOTICE %s :You must specify a password for SIDENTIFY",
                 serv->uid.c_str(), p->sender);
  else
  {
    map<string,NetUser>::iterator user = sim->users.find(p->sender);
    if (user == sim->users.end())
    {
      server->puts(":%s WALLOPS :Unknown user %s", server->local_sid, p->sender);
    }
    else
    {
      string lowername = user->second.nick;
      //transform(lowername.begin(), lowername.end(), lowername.begin(), downcase);
      if (database_query("SELECT name FROM nickserv_users WHERE name = '%s' AND password = MD5('%s');",
                         database_escape(lowername).c_str(), database_escape(args).c_str()) > 0)
      {
        if (database_query("UPDATE nickserv_users SET ident_id = '%s', identified = 1 WHERE name = '%s';",
                       database_escape(user->second.uid).c_str(), database_escape(lowername).c_str()) >= 0)
        {
          database_query("INSERT INTO user_identification VALUES ('%s',NOW());",
                        database_escape(user->second.uid).c_str());
          server->puts(":%s NOTICE %s :Password acceoted -- you are now recognized.",
                       serv->uid.c_str(), p->sender);
          server->puts(":%s MODE %s :+e", server->local_sid, p->sender);
        }
      } // account exists and password is correct
    } // valid user in database
  } // specified password
} // SIDENT

void ns_pm_hook(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  if (serv->uid != p->args && serv->nick != p->args)
    return; // channel or non-nickserv message
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
  NSCmd *c;
  bool matched = false;
  for (c = pmcmds; c->exec && command > c->hook; c++);
  cout << command << "vs." << c->hook << endl;
  while (command == c->hook)
  {
    cout << c->hook << endl;
    ((c++)->exec)(server, serv, p, sim);
    matched = true;
  }
  if (!matched)
    server->puts(":%s NOTICE %s :Unknown Command", serv->uid.c_str(), p->sender);
} // NickServ PRIVMSG

void ns_uid_hook(Server *server, Service *serv, Parser *p, Simulation *sim)
{
  vector<string> args = splitstr(p->args, " ");
  string lowername = args[0];
  int result;
  // string uid,   string nick, string ident, string host,
  // args[7],      args[0],     args[4],      args[5],
  // string vhost, string ip,   string mode,  string gecos
  // args[8],      args[6],     args[3],      parser->longarg
  //transform(lowername.begin(), lowername.end(), lowername.begin(), downcase);
  result = database_query("SELECT * FROM nickserv_users WHERE name = '%s';",
                           database_escape(lowername).c_str() );
  if (result > 0)
  {
    // \002 002 BOLD
    // \006 006 BLINK
    // \026 022 INVERSE
    // \037 031 UNDERLINE
    server->puts(":%s NOTICE %s :This nickname is registered and protected.  If it is your nickname, type \002/msg %s IDENTIFY \037password\037\002. Otherwise, please choose a different nickname.",
                 serv->uid.c_str(), args[7].c_str(), serv->nick.c_str());
  }
} // NickServ UID

extern "C" Service* load()
{
  return new NickServ("NickServ", "NicServ", "services",
               SERVICEHOST, "Nickname Services");
}

extern "C" void unload(Service* ob)
{
  delete ob;
}

extern "C" char *list()
{
  return "NickServ";
}