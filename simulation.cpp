/*
 *  simulation.cpp
 *  kageserv
 *
 *  Created by Administrator on Sun May 23 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include <vector>
#include <string>
#include <utility> // pair
#include <algorithm>
#include "netsimul.h"
#include "simulation.h"
#include "parser.h"
#include "server.h"
#include "protocol.h"
using namespace std;

void uid_hook(Server *serv, Parser *parser, Simulation *sim);
void sid_hook(Server *serv, Parser *parser, Simulation *sim);
void quit_hook(Server *serv, Parser *parser, Simulation *sim);
void squit_hook(Server *serv, Parser *parser, Simulation *sim);
void pass_hook(Server *serv, Parser *parser, Simulation *sim);
void server_hook(Server *serv, Parser *parser, Simulation *sim);
void kill_hook(Server *serv, Parser *parser, Simulation *sim);
void sjoin_hook(Server *serv, Parser *parser, Simulation *sim);
void part_hook(Server *serv, Parser *parser, Simulation *sim);
void mode_hook(Server *serv, Parser *parser, Simulation *sim);

string sjoin_strip(string);

static SimCmd hooks[] = {
  {"PASS",   &pass_hook},
  {"SERVER", &server_hook},
  {"SID",    &sid_hook},
  {"UID",    &uid_hook},
  {"QUIT",   &quit_hook},
  {"SQUIT",  &squit_hook},
  {"MODE",   &mode_hook},
  {"PRIVMSG",&kill_hook},
  {"SJOIN",  &sjoin_hook},
  {"PART",   &part_hook},
  {NULL,     NULL} // DO NOT REMOVE!!!
}; // hooks

Simulation::Simulation(Server *i_serv)
{
  server = i_serv;
  servers.push_back( NetServer("", "", "") );
} // Simulation::Simulation

bool Simulation::execute(Parser *parser)
{
  SimCmd *h;
  for (h = hooks; h->hook; h++)
  {
    if (strcmp(parser->command, h->hook) == 0)
	   (h->exec)(server, parser, this);
  } // for each hook in hooks
} // Simulation::execute

//////////////////////////////////////////////////////////////
//  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //
// //  //  //  //  //  //  //  //  //  //  //  //  //  //  ///
////  //  //  //  //  //  //  //  //  //  //  //  //  //  ////
///  //  //  //  //  //  //  //  //  //  //  //  //  //  // //
//  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //
//////////////////////////////////////////////////////////////

void pass_hook(Server *serv, Parser *parser, Simulation *sim)
{
  cout << "** PASS hook activated" << endl;
  // PASS password TS 6 sid
  cout << parser->args << endl;
  vector<string> args;
  string arg2 = " ";
  string arg1 = parser->args;
  args = splitstr(arg1, arg2);
  cout << "(password) " << args[0] << endl;
  cout << "(TS)       " << args[1] << endl;
  cout << "(6)        " << args[2] << endl;
  sim->servers[0].sid = args[3];
  cout << "(SID)      " << sim->servers[0].sid << endl;
} // sid_hook


void sid_hook(Server *serv, Parser *parser, Simulation *sim)
{
  vector<string> halves;
  vector<string> args;
  NetServer new_s;
  args = splitstr(parser->args, " ", 3);
  cout << "** SID hook activated" << endl;
  cout << "(server)  " << args[0] << endl;
  new_s.name = args[0];
  
  cout << "(hops)    " << args[1] << endl;
  
  cout << "(sid)     " << args[2] << endl;
  new_s.sid = args[2];
  
  cout << "(gecos)   " << parser->longarg << endl;
  new_s.gecos = parser->longarg;
  
  cout << "(link)    " << parser->sender << endl;
  sim->servers.push_back(new_s);
  vector<NetServer>::iterator s;
  for (s = sim->servers.begin(); s != sim->servers.end(); s++)
  {
    if ((*s).sid == parser->sender)
    {
      cout << "** Linking to " << (*s).name << endl;
      s->links.push_back(&sim->servers[sim->servers.size()-1]);
      cout << "** " << s->name << " has " << s->links.size() << " links" << endl;
      break;
    }
  }
} // sid_hook

// [<SID>] [<NICK> <HOPS> <TS> +<UMODE> <USER> <HOST> <IP> <UID> <VHOST>] [<GECOS>]
void uid_hook(Server *serv, Parser *parser, Simulation *sim)
{
  vector<string> args = splitstr(parser->args, " ");
  
  // string uid, string nick, string ident, string host,
  NetUser new_user = NetUser(args[7], args[0], args[4], args[5],
  // string vhost, string ip, string mode, string gecos
     args[8], args[6], args[3], parser->longarg);
  string key = args[7]; // user ID
  transform(key.begin(), key.end(), key.begin(), upcase); // make *sure* its uppercase
  sim->users.insert(pair<string,NetUser>(key, new_user));
} // uid_hook

void quit_hook(Server *serv, Parser *parser, Simulation *sim)
{
  map<string,NetUser>::iterator user = sim->users.find(parser->sender);
  if (user == sim->users.end())
  {
    serv->puts(":%s WALLOPS :Unknown user %s", serv->local_sid, parser->sender);
  }
  else
  {
    sim->users.erase(user);
  }
} // quit_hook

int squit_recurse(Server *serv, string servid, Simulation *sim)
{
  vector<NetServer>::iterator split = sim->servers.begin();
  while (split != sim->servers.end() && split->sid != servid) split++;
  if (split == sim->servers.end())
  {
    serv->puts(":%s WALLOPS :Unknown server ID %s", serv->local_sid, servid.c_str());
    return 0;
  }
  else
  {
    map<string,NetUser>::iterator user = sim->users.begin();
    int ucount = 0, total = 0;
    string temp;
    while (user != sim->users.end())
    {
      cout << "** Checking user " << total++ << "/" << sim->users.size() << "" << endl;
      temp.assign(user->second.uid, 0, 3);
      if (servid == temp)
      {
        cout << "** Deleting user " << user->second.nick << "(" << user->second.uid << ")" << endl;
        sim->users.erase(user);
        ucount++;
        cout << "** " << ucount << " users affected so far" << endl;
      }
      //else
      //{
        user++;
      //}
    }
    vector<NetServer*>::iterator link = split->links.begin();
    while (link != split->links.end())
    {
      cout << "** Linked to server " << (*link)->sid << endl;
      ucount += squit_recurse(serv, (*link)->sid, sim);
      link++;
    }
    sim->servers.erase(split);
    return ucount;
  }
}

void squit_hook(Server *serv, Parser *parser, Simulation *sim)
{
  string servid = splitstr(parser->args, " ", 2)[0];
  vector<NetServer>::iterator split = sim->servers.begin();
  while (split != sim->servers.end() && servid != split->sid)
  {
    cout << "\"" << split->sid << "\" vs. \"" << servid << "\"" << endl;
    split++;
  }
  if (split == sim->servers.end())
  {
    serv->puts(":%s WALLOPS :Unknown server ID %s", serv->local_sid, servid.c_str());
  }
  else
  {
    string sname = split->name;
    serv->puts(":%s WALLOPS :Server \002%s\002 has split, \002%d\002 users affected.", serv->local_sid, 
               sname.c_str(), squit_recurse(serv, servid, sim));
  }
} // squit_hook

void server_hook(Server *serv, Parser *parser, Simulation *sim)
{
  char *temp;
  if (parser->mtype != MSG_LOW)
    return;
  cout << "** SERVER hook activated" << endl;
  cout << "CMDARGS      " << parser->cmdargs << endl;
  temp = strtok(parser->cmdargs, " ");
  cout << "TEMP         " << &temp << endl;
  cout << "(SERVER)     " << temp << endl;
  temp = strtok(NULL, " ");
  cout << "TEMP         " << &temp << endl;
  sim->servers[0].name = strdup(temp);
  cout << "(hostname)   " << sim->servers[0].name << endl;
  sim->servers[0].gecos = strdup(parser->longarg);
  cout << "(gecos)      " << sim->servers[0].gecos << endl;
} // server_hook

void kill_hook(Server *serv, Parser *parser, Simulation *sim)
{
  
} // sid_hook

// [<SID>] [<TS> <CHAN> +<UMODE>] [<USERS>]
void sjoin_hook(Server *serv, Parser *parser, Simulation *sim)
{
  vector<string> users = splitstr(parser->longarg, " ");
  vector<string> args = splitstr(parser->args, " ");
  string name = args[1];
  string mode, rawmode = args[2];
  
  transform(name.begin(), name.end(), name.begin(), downcase);
  mode.assign(rawmode, 1, rawmode.length());
  
  map<string,NetChannel>::iterator chan = sim->channels.find(name);
  if (chan == sim->channels.end())
  {
    NetChannel new_chan(args[1], mode);
    transform(users.begin(), users.end(), users.begin(), sjoin_strip);
    new_chan.users.insert(new_chan.users.end(), users.begin(), users.end());
    sim->channels.insert(pair<string,NetChannel>(name, new_chan));
  }
  else // channel already exists
  {
    transform(users.begin(), users.end(), users.begin(), sjoin_strip);
    chan->second.users.insert(chan->second.users.end(),
                              users.begin(), users.end());
  }
} // sjoin_hook

void part_hook(Server *serv, Parser *parser, Simulation *sim)
{
  map<string,NetChannel>::iterator chan = sim->channels.find(parser->args);
  if (chan == sim->channels.end())
  {
    serv->puts(":%s WALLOPS :Unknown channel %s", serv->local_sid, parser->args);
  }
  else
  {
    vector<string>::iterator user = chan->second.users.begin();
    bool matched = false;
    user = chan->second.users.begin();
    while (user != chan->second.users.end())
    {
      cout << *user << endl;
      if (*user == parser->sender)
      { chan->second.users.erase(user);
        matched = true;
      }
      else
      {  user++;
      }
    }
    if (!matched)
    {
      serv->puts(":%s WALLOPS :Unknown user %s", serv->local_sid, parser->sender);
    }
  }
} // part_hook

void mode_hook(Server *serv, Parser *parser, Simulation *sim)
{
  vector<string> username = splitstr(parser->args, " ", 2);
  string modechange = parser->longarg;
  map<string,NetUser>::iterator user = sim->users.find(username[0]);
  if (username.size() == 2)
    modechange = username[1]+" "+modechange;
  if (user == sim->users.end())
  {
    if (*(parser->args) != '#')
      serv->puts(":%s WALLOPS :Unknown user %s", serv->local_sid, parser->args);
    else
      cout << "** Got deprecated channel MODE command from " << parser->sender << endl;
  }
  else
  {
    cout << "User " << user->second.nick << " had mode " << user->second.mode << endl;
    cout << "Applying mode change " << modechange << endl;
    user->second.apply_mode(modechange);
    cout << "User " << user->second.nick << " has mode " << user->second.mode << endl;
  }
} // mode_hook

string sjoin_strip(string name)
{
  string temp;
  temp.assign(name,
              name.find_first_not_of("!@%^*+"),
              name.size());
  return temp;
}