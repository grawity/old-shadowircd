/*
 *  services.cpp
 *  kageserv
 *
 *  Created by Administrator on Sun May 30 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include <string>
#include <vector>
#include <dlfcn.h>
#include "config.h"
#include "services.h"
#include "parser.h"
#include "protocol.h"
#include "control.h"

static int lastserv = 0;
static char *modules[] = {"./nickserv.so", "./chanserv.so", "./operserv.so"};
Service *ControlServ;
static vector<Service*> servs;
static vector<void*> libs;
static vector<ServHook> hooks;
//static Filter default_filter = {1, 1, 1, 1, 1, 1, 1, 1};
static Service blankserv = Service();

Service::Service()
{
  name = "BlankServ";
  nick = "BlankServ";
  user = "service";
  host = "unconfigured.services.net";
  gecos = "Unconfigured Service";
  //filter = default_filter;
} // default constructor.

Service::Service(string name_, string nick_, string user_, string host_, string gecos_)
{
  name = name_;
  nick = nick_;
  user = user_;
  host = host_;
  gecos = gecos_;
  //filter = default_filter;
}

//void Service::setFilter(Filter filter_)
//{
//  filter = filter_;
//}

void Service::init(Server *server, string uid_)
{
  serv = server;
  uid = uid_;
  cout << name << ".uid = " << uid << endl;
  cout << name << " sending signon" << endl;
  Protocol::signon(*server,
                  server->local_sid, nick.c_str(), 0, "aoeSW",
						user.c_str(), server->local_host, host.c_str(), "127.0.0.1",
						uid.c_str(), gecos.c_str());
}

void GenServices()
{
  Service* (*load)();
  void*    lib;
  ControlServ = new Control(CONTROLSERVICE, CONTROLSERVICE, "control",
               SERVICEHOST, "Services Control");
  for (int i = 0; i < sizeof(modules)/sizeof(char*); i++)
  {
    lib = dlopen(modules[i], RTLD_NOW);
    if (lib != NULL)
    {
      load = (Service*(*)()) dlsym(lib, "load");
      if (load != NULL)
      {  servs.push_back(load());
      } else {
        cerr << "Could not load symbols from " << modules[i] << endl;
        cerr << "Error was: " << dlerror() << endl;
        return;
      }
      libs.push_back(lib);
    } else {
      cerr << "Could not load module " << modules[i] << endl;
      cerr << "Error was: " << dlerror() << endl;
      return;
    }
    
  }
}

void InjectService(Server *server, string filename)
{
  Service* (*load)();
  void*    lib;
    lib = dlopen(filename.c_str(), RTLD_NOW);
    if (lib != NULL)
    {
      load = (Service*(*)()) dlsym(lib, "load");
      if (load != NULL)
      {  servs.push_back(load());
      } else {
        cerr << "Could not load symbols from " << filename << endl;
        cerr << "Error was: " << dlerror() << endl;
        return;
      }
      libs.push_back(lib);
      servs[servs.size()-1]->addHook = &AddHook;
      servs[servs.size()-1]->init(server, Protocol::get_next_uid(server->local_sid));
    } else {
      cerr << "Could not load module " << filename << endl;
      cerr << "Error was: " << dlerror() << endl;
      return;
    }
}



void ExtractService(Server *server, string filename)
{
  void (*unload)(Service*);
  char *(*list)();
  void*    lib;
    lib = dlopen(filename.c_str(), RTLD_NOW);
    if (lib != NULL)
    {
      list = (char*(*)()) dlsym(lib, "list");
      unload = (void(*)(Service*)) dlsym(lib, "unload");
      if (unload == NULL || list == NULL)
      {
        cerr << "Could not load symbols from " << filename << endl;
        cerr << "Error was: " << dlerror() << endl;
        return;
      }
    } else {
      cerr << "Could not load module " << filename << endl;
      cerr << "Error was: " << dlerror() << endl;
      return;
    }
    // if we make it here, we have a list() and an unload()
    char *servicename = (list)();
    vector<Service*>::iterator s = servs.begin();
    vector<void*>::iterator l = libs.begin();
    vector<ServHook>::iterator h = hooks.begin();
    while (h != hooks.end())
    {
      if ((*h).parent->name == servicename)
      {
        cout << "Erasing hook owned by " << (*h).parent->name << endl;
        h = hooks.erase(h); // returns an iterator to the elem after deleted elem
      }
      else
      {
        cout << "Saving hook owned by " << (*h).parent->name << endl;
        h++;
      }
    }
    while (s != servs.end())
    {
      if ((*s)->name == servicename)
      {
        Protocol::quit(*server, (*s)->uid.c_str(), "Service Unloaded");
        (unload)(*s);
        servs.erase(s);
        libs.erase(l);
        break;
      }
      s++;
      l++;
    }
    dlclose(lib); // once for the initial open
}

void StartServices(Server *server)
{
  ControlServ->addHook = &AddHook;
  ControlServ->init(server, Protocol::get_next_uid(server->local_sid));
  for (int i = 0; i < servs.size() && servs[i]; i++)
  {
    cout << "Starting service #" << i << " (" << servs[i]->name << ")" << endl;
    servs[i]->addHook = &AddHook;
	  servs[i]->init(server, Protocol::get_next_uid(server->local_sid));
  }
}

void ExecServices(Server *serv, Parser *parser, Simulation *sim)
{
  vector<string> args = splitstr(parser->args, " ");
    
    for (int h = 0; h < hooks.size(); h++)
    {
	    //cout << "Searching: " << parser->command << " vs. " << hooks[h].hook << endl;
      if (hooks[h].hook == parser->command)
	      (hooks[h].exec)(serv, hooks[h].parent, parser, sim);
    } // for each hook in hooks

}

void DelServices()
{
  void (*unload)(Service*);
  delete ControlServ;
  for (int i = 0; i < servs.size(); i++)
  {
    cout << "Deleting service #" << i << " (" << servs[i]->name << ")" << endl;
    unload = (void(*)(Service*)) dlsym(libs[i], "unload");
    if (unload != NULL)
    {
      unload(servs[i]);
    } else {
      cerr << "Could not load symbols from " << (modules[i]||"Dynamic Module") << endl;
      cerr << "Error was: " << dlerror() << endl;
    }
    dlclose(libs[i]);
  }
  servs.clear();
  libs.clear();
  hooks.clear();
}

ServHook::ServHook(string hook_, Service *par_,
           void (*exec_)(Server*, Service*, Parser*, Simulation*))
{
  hook = hook_;
  exec = exec_;
  parent = par_;
}

void AddHook(string hook_, Service *parent_,
             void (*exec_)(Server *, Service *, Parser *, Simulation *) )
{
  hooks.push_back(ServHook(hook_, parent_, exec_));
}
