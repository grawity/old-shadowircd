/*
 *  parser.cpp
 *  kageserv
 *
 *  Created by Administrator on Sun May 23 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include <ctype.h>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <cctype>
#include "parser.h"
using namespace std;

Parser::Parser(Server i_serv) // : serv(i_serv)
{
  serv = i_serv;
  mtype = MSG_INVALID;
  stype = SENDER_INVALID;
} // Parser::Parser

void Parser::parse(char *line)
{
  freeall();
  raw = line;
  if (*line == ':')
  {
    mtype = MSG_CLIENTSERVER;
	  //cout << "MSG_CLIENTSERVER" << endl;
    line += 1; // take off the :
	 if (strchr(line, ':') != NULL)
	 {
	   longarg = strdup(strchr(line, ':')+1);
	   *(strchr(line, ':')-1) = '\0'; // cut them apart
     //cout << "Slicing longarg off line" << endl;
	 } else {
	   longarg = NULL;
	 }
   cmdargs = strdup(line);
	 sender = strdup(strtok(line, " "));
	 command = strdup(strtok(NULL, " "));
	 args = strtok(NULL, "");
	 if (args)
	   args = strdup(args);
	 else
	   args = strdup("");
	 if (isalpha(*sender))
	  if (strchr(sender, '.') != NULL)
	   stype = SENDER_SERVER;
	  else
		stype = SENDER_NICK;
	 else
	  if (strlen(sender) == 3)
	   stype = SENDER_SID;
	  else
		stype = SENDER_UID;
  } // server or client message
  else
  {
	  //cout << "MSG_LOW" << endl;
    mtype = MSG_LOW;
	 if (strchr(line, ':') != NULL)
	 {
	   longarg = strdup(strchr(line, ':')+1);
	   *(longarg-1) = '\0'; // cut them apart
	 } else {
	   longarg = NULL;
	 }
	 cmdargs = strdup(line);
	 sender = strdup("");
	 command = strdup(strtok(line, " "));
	 args = strtok(NULL, "");
	 if (args)
	   args = strdup(args);
	 else
	   args = strdup("");
  } // low level message
} // Parser::parse

void Parser::freeall()
{
  if (mtype == MSG_CLIENTSERVER || mtype == MSG_LOW)
  {
   //cout << "Freeing type: " << mtype << endl;
	 free(cmdargs);
	 free(longarg);
	 free(command);
	 free(sender);
	 free(args);
  }
} // Parser::freeall

void Parser::inspect()
{
  if (mtype != MSG_CLIENTSERVER)
    return;
  cout << "** From: " << sender << " Type: " << stype << endl;
  cout << "** Command: " << command << " Args: " << args << endl;
  if (longarg)
    cout << "** Long Arg: " << longarg << endl;
} // Parser::inspect

char upcase(char c) { return toupper(c); }
char downcase(char c) { return tolower(c); }

pair<string, string> splitCommand(string line, bool makeupcase)
{
  string command, args;
  if (line.find(" ") != string::npos)
  {
    command.assign(line, 0, line.find(" "));
    args.assign(line, line.find(" ")+1, line.size());
  } else {
    command = line;
    args = "";
  }
  if (makeupcase)
    transform(command.begin(), command.end(), command.begin(), upcase);
  return make_pair(command, args);
}

vector<string> splitstr(string line, string delim, int max)
{
  vector<string> ret;
  string token, remnant;
  remnant = line;
  //cout << "Searching; " << &line << ", " << &remnant << ", " << &delim << endl;
  while (remnant.size() > 0)
  {
    if ((max != 0 && ret.size()+1 >= max) || remnant.find(delim) == string::npos)
    {
      //cout << "Not Found; " << &line << ", " << &remnant << ", " << &delim << endl;
      token = remnant;
      remnant = "";
    }
    else
    {
      //cout << "Found; " << &line << ", " << &remnant << ", " << &delim << endl;
      //cout << "Position " << remnant.find(delim) << " in " << remnant << endl;
      token.assign(remnant, 0, remnant.find(delim));
      remnant.assign(remnant, remnant.find(delim)+1, remnant.size());
    }
    ret.push_back(token);
  }
  return ret;
}