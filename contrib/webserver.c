/* STATUS module for ShadowIRCd 3.1+
 *
 * You should note that this module *requires* that connection
 * headers are disabled, and will DISABLE them on load.
 *
 * This module adds a webserver to the ircd which displays
 * a status page.
 */

#include "stdinc.h"
#include "tools.h"
#include "common.h"  
#include "handlers.h"
#include "client.h"
#include "hash.h"
#include "channel.h"
#include "channel_mode.h"
#include "hash.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_serv.h"
#include "send.h"
#include "list.h"
#include "irc_string.h"
#include "sprintf_irc.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "hook.h"

static void m_get(struct Client*, struct Client*, int, char**);
static void send_motd_file(struct Client *, MessageFile *);

/* We can only allow this to be used by UNREGISTERED clients.
 * anything else would be insane.
 */
struct Message get_msgtab = {
  "GET", 0, 0, 0, 0, MFLG_SLOW, 0L, /* don't show this in /help */
  {m_get, m_ignore, m_ignore, m_ignore, m_ignore}
};

void
_modinit(void)
{
  mod_add_cmd(&get_msgtab);
  ConfigFileEntry.send_connection_headers = 0;
  ConfigFileEntry.throttle_time = 0;
}

void
_moddeinit(void)
{
  mod_del_cmd(&get_msgtab);
}

const char *_version = "$Revision: 1.1 $";

/* m_get
 *      parv[0] = sender prefix
 *      parv[1] = page to get
 *      parv[2] = protocol version
 */
static void
m_get(struct Client *client_p, struct Client *source_p,
        int parc, char *parv[])
{
  int found_it = 0;
  dlink_node *ptr;
  struct Client *target_p;
  
  char *origurl;
  char *spliceurl;
  DupString(origurl, parv[1]);

  spliceurl = strtok(origurl, "?");
  spliceurl = strtok(NULL, "?");

  /* buy some time for the browser */
  usleep(90000);

  /* ok output the data */
  sendto_one(source_p, "HTTP/1.0 200 OK\nContent-Type: text/html\nConnection: close\n");
  source_p->flags |= FLAGS_IS_NOT_IRC;
  sendto_one(source_p, "<html><head><title>%s on the %s IRC Network</title>",
            me.name, ServerInfo.network_name);
  
  /* Flashy HTML stuff... thanks NhJm */
  sendto_one(source_p, "<style type=\"text/css\"><!--");
  sendto_one(source_p, "a {color: #555555;}");
  sendto_one(source_p, "body {margin-top: 30px; font-family: Bitstream Vera Sans, Tahoma, Verdana, Arial; font-size: 8pt;}");
  sendto_one(source_p, ".navb {position: absolute; top: 0px; left: 0px; height: 26px;}");
  sendto_one(source_p, ".nav {background-color: lightgray; font-weight: bold; border-bottom: 2px solid #555555; color: #555555; text-align: center; width: 14%%; font-family: Bitstream Vera Sans, Tahoma, Verdana, Arial; font-size: 8pt; cursor: default;}");
  sendto_one(source_p, ".navc {background-color: lightgray; text-align: left; font-family: Bitstream Vera Sans, Tahoma, Verdana, Arial; font-size: 8pt; cursor: default;}");
  sendto_one(source_p, "address {border-top: 2px solid #555555;}");
  sendto_one(source_p, "//-->");
  sendto_one(source_p, "</style>");
  sendto_one(source_p, "<script language=\"JavaScript\">");
  sendto_one(source_p, "<!--");
  sendto_one(source_p, "function hover(obj) { obj.style.color = \"black\"; obj.style.borderBottom = \"2px solid black\"; }");
  sendto_one(source_p, "function out(obj) { obj.style.color = \"#555555\"; obj.style.borderBottom = \"2px solid #555555\"; }");
  sendto_one(source_p, "function go(url) { location.href = url; }");
  sendto_one(source_p, "//-->");
  sendto_one(source_p, "</script>");

  /* End Flashy HTML stuff */
  sendto_one(source_p, "</head>");
  sendto_one(source_p, "<body>");

  sendto_one(source_p, "<table width=\"100%%\" align=\"center\" class=\"navb\">\n<tr>");
  sendto_one(source_p, "<td class=\"nav\" onmouseover=\"hover(this);\" onmouseout=\"out(this);\" onclick=\"go('index.x');\">Home</td>");
  sendto_one(source_p, "<td class=\"nav\" onmouseover=\"hover(this);\" onmouseout=\"out(this);\" onclick=\"go('serverinfo.x');\">Server Information</td>");
  sendto_one(source_p, "<td class=\"nav\" onmouseover=\"hover(this);\" onmouseout=\"out(this);\" onclick=\"go('serverlist.x');\">Server List</td>");
  sendto_one(source_p, "<td class=\"nav\" onmouseover=\"hover(this);\" onmouseout=\"out(this);\" onclick=\"go('motd.x');\">Server MOTD</td>");
  sendto_one(source_p, "<td class=\"nav\" onmouseover=\"hover(this);\" onmouseout=\"out(this);\" onclick=\"go('admin.x');\">Admin Information</td>");
  sendto_one(source_p, "<td class=\"nav\" onmouseover=\"hover(this);\" onmouseout=\"out(this);\" onclick=\"go('whois.x');\">Whois</td>");
  sendto_one(source_p, "<td class=\"nav\" onmouseover=\"hover(this);\" onmouseout=\"out(this);\" onclick=\"go('about.x');\">About</td>");
  sendto_one(source_p, "</tr>\n</table>");

  /* / and /index.x -- welcome screen */
  if (!strcasecmp("/", parv[1]) || !strcasecmp("/index.x", parv[1])) {
    sendto_one(source_p, "<h2>Welcome!</h2>");
    sendto_one(source_p, "<p>You have reached the web interface of a ShadowIRCd server. This web interface");
    sendto_one(source_p, "is powered by a shadowircd module, downloadable at <a href=\"http://modules.shadowircd.net\">");
    sendto_one(source_p, "modules.shadowircd.net</a>! For more information about the module, click on the About link.</p>");
    found_it = 1;
  }

  /* /serverinfo.x -- basic server stats */
  if (!strcasecmp("/serverinfo.x", parv[1])) {
    sendto_one(source_p, "<h2>Server Information</h2>\n<p>");
    sendto_one(source_p, "This server is running %s!</p>", ircd_version);
    sendto_one(source_p, "<ul><li>Local clients: %d</li>", Count.local);
    sendto_one(source_p, "<li>Maximum local clients: %d</li>", Count.max_loc);
    sendto_one(source_p, "<li>Global clients: %d</li>", Count.total);
    sendto_one(source_p, "<li>Maximum global clients: %d</li>", Count.max_tot);
    sendto_one(source_p, "<li>Network operators: %d</li>", Count.oper);
    sendto_one(source_p, "</ul>");
    found_it = 1;
  }

  /* /admin.x -- administrative info */
  if (!strcasecmp("/admin.x", parv[1])) {
    sendto_one(source_p, "<h2>Server Administration Info</h2>");
    sendto_one(source_p, "<pre>");
    sendto_one(source_p, "Administrative info about %s", me.name);
    sendto_one(source_p, "%s", AdminInfo.name);
    sendto_one(source_p, "%s", AdminInfo.description);
    sendto_one(source_p, "%s", AdminInfo.email);
    sendto_one(source_p, "</pre>");
    found_it = 1;
  }

  /* /serverlist.x -- Server list */
  if (!strcasecmp("/serverlist.x", parv[1])) {
    sendto_one(source_p, "<h2>Server list</h2>");
    sendto_one(source_p, "<table width=\"100%%\">\n<tr>\n<td class=\"nav\">Name</td><td class=\"nav\">Description</td>\n</tr>");
    DLINK_FOREACH(ptr, global_serv_list.head) {
      sendto_one(source_p, "<tr>");
      target_p = ptr->data;
      sendto_one(source_p, "<td class=\"navc\" onClick=\"go('http://%s:6667/');\">%s</td>",
                 target_p->name, target_p->name);
      sendto_one(source_p, "<td class=\"navc\">%s</td>", target_p->info);
      sendto_one(source_p, "</tr>");
    }      
    sendto_one(source_p, "</table>");
    found_it = 1;
  }

  /* /whois_result.x -- Result from whois query */
  if (!strcasecmp("/whois_result.x", origurl)) {
    char *whois_target;

    /* strtok it this time for = */
    whois_target = strtok(spliceurl, "=");
    whois_target = strtok(NULL, "=");

    sendto_one(source_p, "<h2>Whois Result</h2>");
    sendto_one(source_p, "<p>target = %s</p>", whois_target);    

    target_p = find_client(whois_target);

    if (target_p && IsPerson(target_p)) {
       sendto_one(source_p, "<pre>");
       sendto_one(source_p, "%s is %s@%s (%s)",
                   target_p->name, target_p->username, GET_CLIENT_HOST(target_p),
                   target_p->info);
       sendto_one(source_p, "%s is on server %s", target_p->name,
                   target_p->user->server->name);
       sendto_one(source_p, "</pre>");
    }
    found_it = 1;
  }

  /* /whois.x -- Whois form */
  if (!strcasecmp("/whois.x", origurl)) {
    sendto_one(source_p, "<h2>Whois</h2>");
    sendto_one(source_p, "<p>Use this form to execute WHOIS queries on the IRC Network.</p>");
    sendto_one(source_p, "<form method=\"get\" action=\"whois_result.x\">");
    sendto_one(source_p, "<input type=\"text\" size=\"50\" name=\"target\">");
    sendto_one(source_p, "<input type=\"submit\">");
    sendto_one(source_p, "</form>");
    found_it = 1;
  }

  /* /motd.x -- Server MOTD */
  if (!strcasecmp("/motd.x", parv[1])) {
    sendto_one(source_p, "<h2>Server MOTD</h2>");
    send_motd_file(source_p, &ConfigFileEntry.motd);
    found_it = 1;
  }

  /* /about.x -- module credits */
  if (!strcasecmp("/about.x", parv[1])) {
    sendto_one(source_p, "<h2>Credits</h2>");
    sendto_one(source_p, "<ul>");
    sendto_one(source_p, "<li>Initial concept: Puffball (irc.shocknet.us)</li>");
    sendto_one(source_p, "<li>Additional concept: nenolod, NhJm (irc.shadowircd.net)</li>");
    sendto_one(source_p, "<li>Module coding: nenolod (irc.demontek.com)</li>");
    sendto_one(source_p, "</ul>");
    found_it = 1;
  }

  if (found_it != 1) {
    sendto_one(source_p, "<h2>Object not Found</h2>");
    sendto_one(source_p, "<p>There is no object available matching %s</p>", parv[1]);
  }

  sendto_one(source_p, "<address>%s at %s</address>", ircd_version, me.name);
  sendto_one(source_p, "</body>\n</html>");

  /* disconnect the browser */
  exit_client(source_p, source_p, source_p, "End of Request");
}

static void
send_motd_file(struct Client *source_p, MessageFile *motdToPrint)
{
  MessageFileLine *linePointer;

  sendto_one(source_p, "<pre>");
  sendto_one(source_p, "- %s Message of the Day -", me.name);

  for (linePointer = motdToPrint->contentsOfFile; linePointer;
       linePointer = linePointer->next)
  {
    sendto_one(source_p, "- %s", linePointer->line);
  }

  sendto_one(source_p, "</pre>");

}

