/*
 *  server.h
 *  kageserv
 *
 *  Created by Administrator on Sat May 22 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef SERVER_H
#define SERVER_H

#include <cstdlib> 
#include <errno.h> 
#include <cstring> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h>

class Server
{
  // Public Attributes
  public:
    // Hostname of this (local) server
	 char *local_host;
	 // Description of this (local) server
	 char *local_gecos;
	 // Server ID of this (local) server
	 char *local_sid;
	 // Server transmission protocol number
	 int   local_ts;
	 
	 // Remote host we are connected to
	 char *remote_host;
	 // Remote port we are connected to
	 int   port;
	 // Are we connected?
	 bool  connected;

  // Private Attributes
  private:
    // Socket File Descriptor for Server connection
	 int   sockfd;
	 
  // Public Functions
  public:

    // Server needs to know:
	 //  1) Services Hostname
	 //  2) Services Description
	 //  3) Server ID (will use Server.genSID() if you don't give
	 Server(char *i_hostname, char *i_description);
	 Server(char *i_hostname, char *i_description, char *i_sid);
	 
	 // Empty [Con|De]structor
	 Server();
	 ~Server();
	 
	 // Connect to the remote server
	 bool connect(char *r_hostname, int r_port, char *password);

    // get a line from the server
    char *gets();
	 // put a line to the server (does not append \n)
	 bool  puts(char *fmt, ...);

    // disconnect from the server
    bool disconnect();
  // Private Functions
  private:
    
  // Utility Methods
  public:
    static char *genSID();
	 bool link(char *password);
};

#endif