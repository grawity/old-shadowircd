/*
 *  netsimul.h
 *  kageserv
 *
 *  Created by Administrator on Sun May 23 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef NETSIMUL_H
#define NETSIMUL_H

#include <vector>
#include <string>
using namespace std;

class NetMode
{
  public:
  bool plus;
  char mode;
  string object;
  
  NetMode();
  NetMode(bool, char, string);
  bool operator<(const NetMode &ob);
  bool operator==(const NetMode &ob);
};

class NetUser
{
  public:
  string uid;
  string nick, ident, host, vhost, ip;
  string mode, gecos;
  
  NetUser();
  NetUser(string uid, string nick, string ident, string host,
          string vhost, string ip, string mode, string gecos);
  string apply_mode(string mode);
  bool operator<(const NetUser &ob);
  bool operator==(const NetUser &ob);
}; // class NetUser

class NetChannel
{
  public:
  string name, topic, modes;
  vector<string> users;
  vector<NetMode> usermodes;
  
  NetChannel();
  NetChannel(string name, string modes);
  void join(NetUser);
  bool operator<(const NetChannel &ob);
  bool operator==(const NetChannel &ob);
};

class NetServer
{
  public:
  string name, sid, gecos;
  vector<NetServer*> links;
  NetServer();
  NetServer(string sid, string name, string gecos);
  bool operator<(const NetServer &ob);
  bool operator==(const NetServer &ob);
};

#endif