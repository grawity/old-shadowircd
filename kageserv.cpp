/*
 *  kageserv.cpp
 *  kageserv
 *
 *  Created by Administrator on Sat May 22 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include "kageserv.h"
#include "protocol.h"
#include "parser.h"
#include "server.h"
#include "simulation.h"
#include "services.h"
#include "config.h"
using namespace std;

int main(int argc, char **argv)
{
  char *line;
  char *ksid;
  bool restart = false;
  bool connect = true;
  Server network = Server("kageserv.pwnix.net",
								  "Kageserv Services");
  Parser parser = Parser(network);
  Simulation simul = Simulation(&network);
  
  cout << "Done constructing" << endl;
  do // while restart
  {
    GenServices();
    restart = false;
	  connect = network.connect(CONNECT_HOST, CONNECT_PORT, CONNECT_PASS);
   //connect = network.connect("litterbox.ath.cx", 6667, "raymondsucks");
   //connect = network.connect("127.0.0.1", 7776, "raymondsucks");
   cout << "Listening to network" << endl;
	 while (connect && (line = network.gets()) != NULL)
    {
      if (strncmp(line, "ERROR", 5) == 0)
      {
	     cout << "Server Disconnected" << endl;
		  restart = true;
		  sleep(30);
		  break;
      } // ERROR from server
		/*
		else if (strncmp(line, ":2AI EOB", 8) == 0)
      {
	       Protocol::signon(network,
	               network.local_sid, "KageServ", 1, "oeaS",
                  "Services", network.local_host, network.local_host,
						"127.0.0.1", ksid,
                  "KageServ Service");
			 network.puts(":%s EOB", network.local_sid);
			 Protocol::sjoin(network, network.local_sid, SERVICECHAN,
			                 "", ksid);
			 network.puts(":%s PRIVMSG Eko :testing...", ksid);
      } // EOB from server
		*/
		else if (strncmp(line, "PING :", 6) == 0)
      {
	     network.puts("PONG %s\n", (line+6));
      } // PING from server
      cout << ">> " << line << endl;
		parser.parse(line);
		//if (strcmp(parser.command, "EOB") == 0)
    if (strcmp(parser.command, "PASS") == 0)
		{
		    cout << "Starting services" << endl;
	       StartServices(&network);
		}
		//parser.inspect();
		simul.execute(&parser);
    ExecServices(&network, &parser, &simul);
    } // while
	 DelServices();
    network.disconnect();
  } while (restart);
} // int main
