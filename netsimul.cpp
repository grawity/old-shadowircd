/*
 *  netsimul.cpp
 *  kageserv
 *
 *  Created by Administrator on Sun May 23 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */
#include <iostream> //DEBUG

#include <vector>
#include <string>
#include <algorithm>
#include "netsimul.h"
using namespace std;

NetMode::NetMode() {}
NetMode::NetMode(bool p, char m, string o)
{
  plus = p;
  mode = m;
  object = o;
} // NetMode::NetMode

bool NetMode::operator<(const NetMode &ob) { return this->mode<ob.mode; }
bool NetMode::operator==(const NetMode &ob) { return this->mode==ob.mode; }

NetUser::NetUser() {}
NetUser::NetUser(string u, string n, string i, string h,
          string v, string p, string m, string g)
{
  uid = u; nick = n; ident = i; host = h; vhost = v; ip = p; mode = m; gecos = g;
} // NetUser::NetUser

string NetUser::apply_mode(string m)
{
  bool plus = true;
  string::iterator ch = m.begin();
  while (ch != m.end())
  {
    if (*ch == '+')
      plus = true;
    else if (*ch == '-')
      plus = false;
    else if (plus)
    {
      // we're giving this letter
      mode += *ch;
    }
    else
    {
      // we're removing this letter
      size_t d = mode.find(*ch);
      if (d < mode.size())
        mode.erase(d);
    }
    ch++;
  }
  return mode;
}

bool NetUser::operator<(const NetUser &ob) {
  return this->uid < ob.uid; }
bool NetUser::operator==(const NetUser &ob) {
  return this->uid == ob.uid; }

NetChannel::NetChannel() {}
NetChannel::NetChannel(string n, string m)
{
  name = n; modes = m;
  cout << n << " (" <<modes<< ")" << endl;
} // NetUser::NetUser

bool NetChannel::operator<(const NetChannel &ob) {
  return (this->name) < (ob.name); }
bool NetChannel::operator==(const NetChannel &ob) {
  return (this->name) == (ob.name); }
void NetChannel::join(NetUser nu)
{
  users.push_back(nu.uid);
  sort(users.begin(), users.end());
};

NetServer::NetServer() {}
NetServer::NetServer(string s, string n, string g)
{
  sid = s; name = n; gecos = g;
}
bool NetServer::operator<(const NetServer &ob) {
  return (this->sid) < (ob.sid); }
bool NetServer::operator==(const NetServer &ob) {
  return (this->sid) == (ob.sid); }