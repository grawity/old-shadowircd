/*
 *  parser.h
 *  kageserv
 *
 *  Created by Administrator on Sun May 23 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef PARSER_H
#define PARSER_H

#include <utility> // pair
#include <vector> // vector
#include <string> // string
#include "server.h"
#include "protocol.h"

typedef enum {
  MSG_INVALID = -1, MSG_CLIENTSERVER, MSG_LOW
} msg_type;

typedef enum {
  SENDER_INVALID = -1, SENDER_SERVER, SENDER_NICK, SENDER_SID, SENDER_UID
} sender_type;

class Parser
{
  public:
    Server serv;
   string raw;
	 char *cmdargs;
	 char *longarg;
	 
	 char *command;
	 char *sender;
	 char *args;
  //private:
	 msg_type    mtype;
	 sender_type stype;
  public:
    Parser(Server i_serv);
	 void parse(char *line);
	 void inspect();
  private:
    void freeall();
}; // class Parser

char upcase(char c);
char downcase(char c);
pair<string, string> splitCommand(string, bool upcase = true);
vector<string> splitstr(string, string, int = 0);

#endif