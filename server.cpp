/*
 *  server.cpp
 *  kageserv
 *
 *  Created by Administrator on Sat May 22 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include <ctime>
#include <cstdlib> 
#include <cstdio>
#include <cstdarg> 
#include <cstring>
#include <unistd.h>
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h>
#include "config.h"
#include "server.h"
using namespace std;

Server::Server(char *i_hostname, char *i_description)
{
  local_host = strdup(i_hostname);
  local_gecos = strdup(i_description);
  local_sid = genSID();
  local_ts = 6;
} // Server::Server

Server::Server(char *i_hostname, char *i_description, char *i_sid)
{
  local_host = strdup(i_hostname);
  local_gecos = strdup(i_description);
  local_sid = strdup(i_sid);
  local_ts = 6;
} // Server::Server

Server::Server()
{
} // Server::Server
Server::~Server()
{
} // Server::~Server

bool Server::connect(char *r_hostname, int r_port, char *password)
{
        struct hostent *he;
        struct sockaddr_in their_addr; // connector's address information 
		  
        cerr << "gethostbyname()" << endl;
        if ((he=gethostbyname(r_hostname)) == NULL) {  // get the host info 
            cerr << "Error resolving hostname " << r_hostname << ":" << endl;
            perror("gethostbyname");
            return false; //exit(1);
        }
		  
        cerr << "socket()" << endl;
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            return false; //exit(1);
        }
		  
        their_addr.sin_family = AF_INET;    // host byte order 
        their_addr.sin_port = htons(r_port);  // short, network byte order 
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

        cerr << "connect()" << endl;
        if (::connect(sockfd, (struct sockaddr *)&their_addr,
                                              sizeof(struct sockaddr)) == -1) {
            perror("connect");
            return false;
        }
        
        cerr << "link()" << endl;
		  if (link(password) == false)
		  {
		    cout << "Failed to send link information" << endl;
			 return false;
		  }
		  
        return true;
} // bool Server::connect

char *Server::gets()
{
  int numbytes;
  static char buf[MAXDATASIZE];
  static char ch[1] = "";
  memset(buf, '\0', MAXDATASIZE);
  for (int i = 0; i<MAXDATASIZE; i++){
    ch[0] = '\0';
    if (recv(sockfd,ch,sizeof(ch),0) <= 0) return buf;
    if (ch[0] == '\r') continue;
    if (ch[0] == '\n') break;
    buf[i]=ch[0];
    buf[i+1]='\0';
  }
  return buf;
} // Server::recvLine

/* Good Version, make it better
int recvLine(char reply[MAXDATASIZE]){
   int length = 0;
   int i;
   char ch[1] = "";
   for (i = 0; i<MAXDATASIZE; i++){
     ch[0] = '\0';
     if (recv(sockfd,ch,sizeof(ch),0) == 0) return -1;
     if (ch[0] == '\r') continue;
     if (ch[0] == '\n') break;
     reply[i]=ch[0];
     length++;
     //cout << (int)ch[0] << endl;
     reply[i+1]='\0';
   }
  return length;
}
*/

bool Server::puts( char *fmt, ... )
{
  static char message[MAXDATASIZE];
  char *format;
  memset(message, '\0', MAXDATASIZE);
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(message, MAXDATASIZE, fmt, ap);
  va_end(ap);
  message[strlen(message)] = '\n';
  if (send(sockfd, message, strlen(message), 0) == -1) {
    perror("send");
    return false;
  }
  cout << "<< " << message << flush; // \n is already part of message
  return true;
} // bool Server::puts

bool Server::disconnect()
{
  close(sockfd);
  return true;
} // Server::disconnect

// PASS [password] TS [time] :[SID]
// CAPAB TS TBURST KNOCK UNKLN KLN ENCAP UVH RE QU CLUSTER PARA TS6 EOB QS CHW IE EX
// SERVER [hostname] [hops] :[gecos]
bool Server::link(char *password)
{
  bool ret = true;
  cerr << "Sending PASS" << endl;
  ret |= puts("PASS %s TS %d :%s", password, local_ts, local_sid);
  cerr << "Sending CAPAB" << endl;
  ret |= puts("CAPAB TS TBURST KNOCK UNKLN KLN ENCAP UVH RE QU CLUSTER PARA TS6 EOB QS CHW IE EX");
  cerr << "Sending SERVER" << endl;
  ret |= puts("SERVER %s 1 :%s", local_host, local_gecos);
  return ret;
}

char *Server::genSID()
{
  static char buf[4];
  srandom(time(NULL));
  for (int i = 0; i < sizeof(buf)/sizeof(char); i++)
  {
    // NULL
    if (i == (sizeof(buf)/sizeof(char))-1)
	   buf[i] = 0x00;
	 
    // Number
    else if (i == 0)
	   buf[i] = '0'+(random()%10);
		
	 // Capital Letter
	 else
	   buf[i] = 'A'+(random()%26);
  } // generate each letter of the SID
  return buf;
}