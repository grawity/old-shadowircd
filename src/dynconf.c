/*
 *  UltimateIRCd - an Internet Relay Chat Daemon, src/dynconf.c
 *
 *  Copyright (C) 2002 by the past and present ircd coders, and others.
 *  Refer to the documentation within doc/authors/ for full credits and copyrights.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 *  $Id: dynconf.c,v 1.1 2003/11/04 07:11:36 nenolod Exp $
 *
 */

#define DYNCONF_C
#include "struct.h"
#include "common.h"
#include "sys.h"
#include "numeric.h"
#include "msg.h"
#include "channel.h"
#include "config.h"
#include "dynconf.h"
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <utmp.h>
#include <fcntl.h>
#include "h.h"
#if defined( HAVE_STRING_H )
# include <string.h>
#else
# include <strings.h>
# undef strchr
# define strchr index
#endif

#define DoDebug fprintf(stderr, "[%s] %s | %li\n", babuf, __FILE__, __LINE__);
#define AllocCpy(x,y) if ((x) && type == 1) MyFree((x)); x = (char *) MyMalloc(strlen(y) + 1); strcpy(x,y)
#define XtndCpy(x,y) x = (char *) MyMalloc(strlen(y) + 2); *x = '\0'; strcat(x, "*"); strcpy(x,y)


int report_dynconf (aClient *);
int report_network (aClient *);
int report_options (aClient *);
int report_hostmasks (aClient *);
int report_services (aClient *);
int report_misc (aClient *);

/* internals */
aConfiguration iConf;
int icx = 0;
char buf[1024];


/* strips \r and \n's from the line */
void
iCstrip (char *line)
{
  char *c;

  if ((c = strchr (line, '\n')))
    *c = '\0';
  if ((c = strchr (line, '\r')))
    *c = '\0';
}

/* this loads dynamic DCONF */
int
load_conf (char *filename, int type)
{
  FILE *zConf;
  char *version = NULL;
  char *i;

  zConf = fopen (filename, "r");

  if (!zConf)
    {
      if (type == 1)
	{
	  sendto_realops ("DynConf Fatal Error: Could not load %s !",
			  filename);
	  return -1;
	}
      else
	{
	  fprintf (stderr, "§              ~¤§¤~ ERROR ~¤§¤~\n");
	  fprintf (stderr, "§ DynConf Fatal Error: Couldn't load %s !!!\n",
		   filename);
	  fprintf (stderr,
		   "§~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~§\n");

	  exit (-1);
	}
    }
  i = fgets (buf, 1023, zConf);
  if (!i)
    {
      if (type == 1)
	{
	  sendto_realops ("DynConf Fatal Error: Error reading from %s !",
			  filename);
	  return -1;
	}
      else
	{
	  fprintf (stderr, "§              ~¤§¤~ ERROR ~¤§¤~\n");
	  fprintf (stderr, "§ DynConf Fatal Error: Couldn't read %s !!!\n",
		   filename);
	  fprintf (stderr,
		   "§~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~§\n");
	  exit (-1);
	}
    }
  iCstrip (buf);
  version = strtok (buf, "^");
  version = strtok (NULL, "");
  /* is this an ircd.ini file? */
  if (!match ("1.*", version))
    {
      /* wrong version */
      if (strcmp (version, DYNCONF_CONF_VERSION))
	{
	  if (type == 1)
	    {
	      sendto_realops
		("DynConf Warning: %s is version %s. Expected version: %s - Please upgrade.",
		 filename, version, DYNCONF_CONF_VERSION);
	    }
	  else
	    {
	      fprintf (stderr,
		       "§ DynConf Warning: %s is version %s. Expected version: %s - Please upgrade.\n",
		       filename, version, DYNCONF_CONF_VERSION);
	    }
	}
      load_conf2 (zConf, filename, type);
      return -1;
    }
  else if (!match ("2.*", version))
    {
      /* network file */
      /* wrong version */
      if (strcmp (version, DYNCONF_NETWORK_VERSION))
	{
	  if (type == 1)
	    {
	      sendto_realops
		("DynConf Warning: %s is version %s. Expected version: %s - Please upgrade.",
		 filename, version, DYNCONF_NETWORK_VERSION);
	    }
	  else
	    {
	      fprintf (stderr,
		       "§ DynConf Warning: %s is version %s. Expected version: %s - Please upgrade.\n",
		       filename, version, DYNCONF_NETWORK_VERSION);
	    }
	}
      load_conf3 (zConf, filename, type);
      return -1;
    }
  else
    {
      if (type == 1)
	{
	  sendto_realops
	    ("DynConf Fatal Error: Malformed version header in %s.", INCLUDE);
	  return -1;
	}
      else
	{
	  fprintf (stderr, "§              ~¤§¤~ ERROR ~¤§¤~\n");
	  fprintf (stderr,
		   "§ DynConf Fatal Error: Malformed or missing version header in %s.\n",
		   INCLUDE);
	  fprintf (stderr,
		   "§~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~~¤§¤~§\n");
	  exit (-1);
	}
      return -1;
    }
  return -1;
}

int
load_conf2 (FILE * conf, char *filename, int type)
{
  char *databuf = buf;
  char *stat = databuf;
  char *p, *setto, *var;

  if (!conf)
    return -1;

  *databuf = '\0';

  /* loop to read data */
  while (stat != NULL)
    {
      stat = fgets (buf, 1020, conf);
      if ((*buf == '#') || (*buf == '/'))
	continue;

      iCstrip (buf);
      if (*buf == '\0')
	continue;
      p = strtok (buf, " ");

      if (irccmp (p, "Include") == 0)
	{
	  strtok (NULL, " ");
	  setto = strtok (NULL, "");
	  /* We need this for STATS S -codemastr 
	     Isn't there a better way to show all include files? --Stskeeps */
	  AllocCpy (INCLUDE, setto);
	  load_conf (setto, type);
	}
      else if (irccmp (p, "Set") == 0)
	{
	  var = strtok (NULL, " ");
	  if (var == NULL)
	    continue;
	  if (*var == '\0')
	    continue;

	  strtok (NULL, " ");
	  setto = strtok (NULL, "");

	  /*
	   * HUB
	   */
	  if (strcmp (var, "hub") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "0";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set hub in ircd.ini is invalid (NULL value). Resetting to default.\n");
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set hub in ircd.ini is invalid (NULL value). Resetting to default.");
		}
	      if (strchr (setto, ' '))
		{
		  setto = "0";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set hub in ircd.ini is invalid (Contains spaces). Resetting to default.\n");
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set hub in ircd.ini is invalid (Contains spaces). Resetting to default.");
		}
	      HUB = atoi (setto);
	    }
	  /*
	   * CLIENT_FLOOD
	   */
	  else if (strcmp (var, "client_flood") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "2560";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set client_flood in ircd.ini is invalid (NULL value). Resetting to default.\n");
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set client_flood in ircd.ini is invalid (NULL value). Resetting to default.");
		}
	      if (strchr (setto, ' '))
		{
		  setto = "2560";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set client_flood in ircd.ini is invalid (Contains spaces). Resetting to default.\n");
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set client_flood in ircd.ini is invalid (Contains spaces). Resetting to default.");
		}
	      CLIENT_FLOOD = atol (setto);
	    }
	  /*
	   * MAX_CHANNELS_PER_USER
	   */
	  else if (strcmp (var, "max_channels_per_user") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "10";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set max_channels_per_user in ircd.ini is invalid (NULL value). Resetting to default.\n");
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set max_channels_per_user in ircd.ini is invalid (NULL value). Resetting to default.");
		}
	      if (strchr (setto, ' '))
		{
		  setto = "10";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set max_channels_per_user in ircd.ini is invalid (Contains spaces). Resetting to default.\n");
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set max_channels_per_user in ircd.ini is invalid (Contains spaces). Resetting to default.");
		}
	      MAXCHANNELSPERUSER = atol (setto);
	    }
	  /*
	   * SERVER_KLINE_ADDRESS
	   */
	  else if (strcmp (var, "server_kline_address") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "Admin@PorlyConfiguredServer.com";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set server_kline_address in ircd.ini is invalid (NULL value). Resetting to default.\n");
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set server_kline_address in ircd.ini is invalid (NULL value). Resetting to default.");
		}
	      if (strchr (setto, ' '))
		{
		  setto = "Admin@PorlyConfiguredServer.com";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set server_kline_address in ircd.ini is invalid (Contains spaces). Resetting to default.\n");
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set server_kline_address in ircd.ini is invalid (Contains spaces). Resetting to default.");
		}
	      if (!strchr (setto, '@'))
		{
		  setto = "Admin@PorlyConfiguredServer.com";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set server_kline_address in ircd.ini is invalid (Is not a valid Email-Address). Resetting to default.\n");
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set server_kline_address in ircd.ini is invalid (Is not a valid Email-Adress). Resetting to default.");
		}
	      AllocCpy (SERVER_KLINE_ADDRESS, setto);
	    }
	  /*
	   * DEF_MODE_i
	   */
	  else if (strcmp (var, "def_mode_i") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set def_mode_i in ircd.ini is invalid (NULL value). Resetting to default.\n");
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set def_mode_i in ircd.ini is invalid (NULL value). Resetting to default.");
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set def_mode_i in ircd.ini is invalid (Contains spaces). Resetting to default.\n");
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set def_mode_i in ircd.ini is invalid (Contains spaces). Resetting to default.");
		}
	      DEF_MODE_i = atoi (setto);
	    }
	  /*
	   * SHORT_MOTD
	   */
	  else if (strcmp (var, "short_motd") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "0";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set short_motd in ircd.ini is invalid (NULL value). Resetting to default.\n");
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set short_motd in ircd.ini is invalid (NULL value). Resetting to default.");
		}
	      if (strchr (setto, ' '))
		{
		  setto = "0";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set short_motd in ircd.ini is invalid (Contains spaces). Resetting to default.\n");
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set short_motd in ircd.ini is invalid (Contains spaces). Resetting to default.");
		}
	      SHORT_MOTD = atoi (setto);
	    }

	  else
	    {
	      if (type == 0)
		fprintf (stderr,
			 "§ Dynconf Warning: Invalid Set %s %s setting in %s \n",
			 var, setto, filename);
	      else
		sendto_realops
		  ("Dynconf Warning: Invalid Set %s %s setting in %s ", var,
		   setto, filename);
	    }

	}
      else
	{

	  if (type == 0)
	    {
	      fprintf (stderr,
		       "§ Dynconf Warning: %s in %s is not a known setting to me!\n",
		       p, filename);
	    }
	  else
	    {
	      sendto_realops
		("Dynconf Warning: %s in %s is not a known setting to me!",
		 p, filename);
	    }
	}
    }
  if (type == 0)
    {
      fprintf (stderr, "§ Dynconf Loaded %s\n", filename);
    }
  else
    {
      sendto_realops ("Loaded %s", filename);
    }
  return 0;
}

/* Load .network options */
int
load_conf3 (FILE * conf, char *filename, int type)
{
  char *databuf = buf;
  char *stat = databuf;
  char *p, *setto, *var;
  if (!conf)
    return -1;

  *databuf = '\0';

  /* loop to read data */
  while (stat != NULL)
    {
      stat = fgets (buf, 1020, conf);
      if ((*buf == '#') || (*buf == '/'))
	continue;

      iCstrip (buf);
      if (*buf == '\0')
	continue;

      p = strtok (buf, " ");
      if (irccmp (p, "Set") == 0)
	{
	  var = strtok (NULL, " ");
	  if (var == NULL)
	    continue;
	  if (*var == '\0')
	    continue;

	  strtok (NULL, " ");
	  setto = strtok (NULL, "");
	  /*
	   * IRCNETWORK
	   */
	  if (strcmp (var, "ircnetwork") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "SomeIRC";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set ircnetwork in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set ircnetwork in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "SomeIRC";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set ircnetwork in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set ircnetwork in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (IRCNETWORK, setto);
	    }
	  /*
	   * IRCNETWORK_NAME
	   */
	  else if (strcmp (var, "ircnetwork_name") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "The Some IRC Network";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set ircnetwork_name in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set ircnetwork_name in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (IRCNETWORK_NAME, setto);
	    }
	  /*
	   * DEFSERV
	   */
	  else if (strcmp (var, "defserv") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "irc.SomeIRC.net";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set defserv in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set defserv in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "irc.SomeIRC.net";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set defserv in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set defserv in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (DEFSERV, setto);
	    }
	  /*
	   * WEBSITE
	   */
	  else if (strcmp (var, "website") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "http://www.SomeIRC.net/";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set website in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set website in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "http://www.SomeIRC.net/";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set website in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set website in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (WEBSITE, setto);
	    }
	  /*
	   * RULES_PAGE
	   */
	  else if (strcmp (var, "rules_page") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "http://www.SomeIRC.net/rules/";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set rules_page in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set rules_page in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "http://www.SomeIRC.net/rules/";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set rules_page in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set rules_page in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (RULES_PAGE, setto);
	    }
	  /*
	   * NETWORK_KLINE_ADDRESS
	   */
	  else if (strcmp (var, "network_kline_address") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "KLine@SomeIRC.net";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set network_kline_address in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set network_kline_address in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "KLine@SomeIRC.net";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set network_kline_address in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set network_kline_address in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      if (!strchr (setto, '@'))
		{
		  setto = "KLine@SomeIRC.net";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set network_kline_address in %s is invalid (Is not a valid Email-Address). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set network_kline_address in %s is invalid (Is not a valid Email-Adress). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (NETWORK_KLINE_ADDRESS, setto);
	    }
	  /*
	   * MODE_x
	   */
	  else if (strcmp (var, "mode_x") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set mode_x in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set mode_x in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set mode_x in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set mode_x in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      MODE_x = atoi (setto);
	    }
	  /*
	   * MODE_x_LOCK
	   */
	  else if (strcmp (var, "mode_x_lock") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set mode_x_lock in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set mode_x_lock in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set mode_x_lock in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set mode_x_lock in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      MODE_x_LOCK = atoi (setto);
	    }
	  /*
	   * CLONE_PROTECTION
	   */
	  else if (strcmp (var, "clone_protection") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set clone_protection in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set clone_protection in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set clone_protection in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set clone_protection in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      CLONE_PROTECTION = atoi (setto);
	    }
	  /*
	   * CLONE_LIMIT
	   */
	  else if (strcmp (var, "clone_limit") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "5";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set clone_limit in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set clone_limit in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "5";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set clone_limit in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set clone_limit in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      CLONE_LIMIT = atol (setto);
	    }
	  /*
	   * OPER_CLONE_LIMIT_BYPASS
	   */
	  else if (strcmp (var, "oper_clone_limit_bypass") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set oper_clone_limit_bypass in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set oper_clone_limit_bypass in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set oper_clone_limit_bypass in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set oper_clone_limit_bypass in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      OPER_CLONE_LIMIT_BYPASS = atoi (setto);
	    }
	  /*
	   * HIDEULINEDSERVS
	   */
	  else if (strcmp (var, "hideulinedservs") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set hideulinedservs in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set hideulinedservs in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set hideulinedservs in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set hideulinedservs in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      HIDEULINEDSERVS = atoi (setto);
	    }
	  /*
	   * GLOBAL_CONNECTS
	   */
	  else if (strcmp (var, "global_connects") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set global_connects in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set global_connects in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set global_connects in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set global_connects in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      GLOBAL_CONNECTS = atoi (setto);
	    }
	  /*
	   * OPER_AUTOPROTECT
	   */
	  else if (strcmp (var, "oper_autoprotect") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set oper_autoprotect in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set oper_autoprotect in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set oper_autoprotect in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set oper_autoprotect in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      OPER_AUTOPROTECT = atoi (setto);
	    }
	  /*
	   * OPER_MODE_OVERRIDE
	   */
	  else if (strcmp (var, "oper_mode_override") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "3";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set oper_mode_override in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set oper_mode_override in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "3";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set oper_mode_override in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set oper_mode_override in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      OPER_MODE_OVERRIDE = atol (setto);
	    }
	  /*
	   * OPER_MODE_OVERRIDE_NOTICE
	   */
	  else if (strcmp (var, "oper_mode_override_notice") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set oper_mode_override_notice in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set oper_mode_override_notice in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set oper_mode_override_notice in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set oper_mode_override_notice in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      OPER_MODE_OVERRIDE_NOTICE = atoi (setto);
	    }
	  /*
	   * USERS_AUTO_JOIN
	   */
	  else if (strcmp (var, "users_auto_join") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "0";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set users_auto_join in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set users_auto_join in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "0";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set users_auto_join in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set users_auto_join in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (USERS_AUTO_JOIN, setto);
	    }
	  /*
	   * OPERS_AUTO_JOIN
	   */
	  else if (strcmp (var, "opers_auto_join") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "0";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set opers_auto_join in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set opers_auto_join in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "0";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set opers_auto_join in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set opers_auto_join in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (OPERS_AUTO_JOIN, setto);
	    }
	  /*
	   * SERVICES_SERVER
	   */
	  else if (strcmp (var, "services_server") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "services.SomeIRC.net";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set services_server in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set services_server in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "services.SomeIRC.net";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set services_server in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set services_server in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (SERVICES_SERVER, setto);
	    }
	  /*
	   * STATSERV_SERVER
	   */
	  else if (strcmp (var, "statserv_server") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "statistics.SomeIRC.net";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set statserv_server in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set statserv_server in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "statistics.SomeIRC.net";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set statserv_server in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set statserv_server in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (STATSERV_SERVER, setto);
	    }
	  /*
	   * ENABLE_NICKSERV_ALIAS
	   */
	  else if (strcmp (var, "enable_nickserv_alias") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_nickserv_alias in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_nickserv_alias in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_nickserv_alias in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_nickserv_alias in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      ENABLE_NICKSERV_ALIAS = atoi (setto);
	    }
	  /*
	   * NICKSERV_NAME
	   */
	  else if (strcmp (var, "nickserv_name") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "nickserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set nickserv_name in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set nickserv_name in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "nickserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set nickserv_name in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set nickserv_name in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (NICKSERV_NAME, setto);
	    }
	  /*
	   * ENABLE_CHANSERV_ALIAS
	   */
	  else if (strcmp (var, "enable_chanserv_alias") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_chanserv_alias in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_chanserv_alias in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_chanserv_alias in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_chanserv_alias in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      ENABLE_CHANSERV_ALIAS = atoi (setto);
	    }
	  /*
	   * CHANSERV_NAME
	   */
	  else if (strcmp (var, "chanserv_name") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "chanserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set chanserv_name in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set chanserv_name in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "chanserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set chanserv_name in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set chanserv_name in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (CHANSERV_NAME, setto);
	    }
	  /*
	   * ENABLE_MEMOSERV_ALIAS
	   */
	  else if (strcmp (var, "enable_memoserv_alias") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_memoserv_alias in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_memoserv_alias in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_memoserv_alias in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_memoserv_alias in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      ENABLE_MEMOSERV_ALIAS = atoi (setto);
	    }
	  /*
	   * MEMOSERV_NAME
	   */
	  else if (strcmp (var, "memoserv_name") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "memoserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set memoserv_name in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set memoserv_name in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "memoserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set memoserv_name in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set memoserv_name in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (MEMOSERV_NAME, setto);
	    }
	  /*
	   * ENABLE_BOTSERV_ALIAS
	   */
	  else if (strcmp (var, "enable_botserv_alias") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_botserv_alias in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_botserv_alias in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_botserv_alias in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_botserv_alias in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      ENABLE_BOTSERV_ALIAS = atoi (setto);
	    }
	  /*
	   * BOTSERV_NAME
	   */
	  else if (strcmp (var, "botserv_name") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "botserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set botserv_name in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set botserv_name in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "botserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set botserv_name in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set botserv_name in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (BOTSERV_NAME, setto);
	    }
	  /*
	   * ENABLE_ROOTSERV_ALIAS
	   */
	  else if (strcmp (var, "enable_rootserv_alias") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_rootserv_alias in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_rootserv_alias in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_rootserv_alias in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_rootserv_alias in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      ENABLE_ROOTSERV_ALIAS = atoi (setto);
	    }
	  /*
	   * ROOTSERV_NAME
	   */
	  else if (strcmp (var, "rootserv_name") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "rootserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set rootserv_name in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set rootserv_name in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "rootserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set rootserv_name in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set rootserv_name in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (ROOTSERV_NAME, setto);
	    }
	  /*
	   * ENABLE_OPERSERV_ALIAS
	   */
	  else if (strcmp (var, "enable_operserv_alias") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_operserv_alias in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_operserv_alias in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_operserv_alias in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_operserv_alias in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      ENABLE_OPERSERV_ALIAS = atoi (setto);
	    }
	  /*
	   * OPERSERV_NAME
	   */
	  else if (strcmp (var, "operserv_name") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "operserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set operserv_name in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set operserv_name in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "operserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set operserv_name in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set operserv_name in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (OPERSERV_NAME, setto);
	    }
	  /*
	   * ENABLE_STATSERV_ALIAS
	   */
	  else if (strcmp (var, "enable_statserv_alias") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_statserv_alias in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_statserv_alias in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "1";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set enable_statserv_alias in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set enable_statserv_alias in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      ENABLE_STATSERV_ALIAS = atoi (setto);
	    }
	  /*
	   * STATSERV_NAME
	   */
	  else if (strcmp (var, "statserv_name") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "statserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set statserv_name in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set statserv_name in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "statserv";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set statserv_name in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set statserv_name in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (STATSERV_NAME, setto);
	    }
	  else if (strcmp (var, "operhost") == 0)
	    {
	       if (setto == NULL)
		 {
		   setto = "staff.someirc.net";
	         }
	      AllocCpy (OPERHOST, setto);
	    }
	  /*
	   * HELPCHAN
	   */
	  else if (strcmp (var, "helpchan") == 0)
	    {
	      if (setto == NULL)
		{
		  setto = "#help";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set helpchan in %s is invalid (NULL value). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set helpchan in %s is invalid (NULL value). Resetting to default.",
		       INCLUDE);
		}
	      if (strchr (setto, ' '))
		{
		  setto = "#help";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set helpchan in %s is invalid (Contains spaces). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set helpchan in %s is invalid (Contains spaces). Resetting to default.",
		       INCLUDE);
		}
	      if (!strchr (setto, '#'))
		{
		  setto = "#help";

		  if (type == 0)
		    fprintf (stderr,
			     "§ Dynconf ERROR: Set helpchan in %s is invalid (Is no a valid channel). Resetting to default.\n",
			     INCLUDE);
		  else
		    sendto_realops
		      ("Dynconf ERROR: Set helpchan in %s is invalid (Is no a valid channel). Resetting to default.",
		       INCLUDE);
		}
	      AllocCpy (HELPCHAN, setto);
	    }
	  else
	    {
	      if (type == 0)
		fprintf (stderr,
			 "§ Dynconf Warning: Invalid Set %s %s setting in %s \n",
			 var, setto, INCLUDE);
	      else
		sendto_realops
		  ("Dynconf Warning: Invalid Set %s %s setting in %s ", var,
		   setto, INCLUDE);
	    }

	}
      else
	{

	  if (type == 0)
	    {
	      fprintf (stderr,
		       "§ Dynconf Warning: %s in %s is not a known setting to me!\n",
		       p, INCLUDE);
	    }
	  else
	    {
	      sendto_realops
		("Dynconf Warning: %s in %s is not a known setting to me!",
		 p, INCLUDE);
	    }
	}

    }
  if (type == 0)
    {
      fprintf (stderr, "§ Dynconf Loaded %s\n", INCLUDE);
    }
  else
    {
      sendto_realops ("Loaded %s", INCLUDE);
    }
  return 0;
}


void
init_dynconf (void)
{
  bzero (&iConf, sizeof (iConf));
}
