#include "struct.h"
#include "h.h"
#include "channel.h"
#include "common.h"
#include "sys.h"
#include "fdlist.h"
#include "numeric.h"

aGline *glines = NULL;
aGline *find_glined_user(aClient *);
void add_gline (char *, char *, char *, char *, time_t, time_t, char *);
void check_expire_gline (void);
aGline *expire_gline(aGline *);
int sweep_gline (void);
int find_gline (char *, char *);
int del_gline (char *, char *);

extern char *smalldate(time_t);

char *glreason; /* Where else is better to put this?! */

#define AllocCpy(x, y) x = (char *)MyMalloc(strlen(y) + 1); strcpy(x, y)

/*
 * m_gline()
 *  parv[0] = sender prefix
 *  parv[1] = add/del
 *  parv[2] = user@host
 *  parv[3] = duration (seconds)
 *  parv[4] = reason
 */
int m_gline(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
    char *set_on, *set_by, *username, *hostname, *reason, *p, *tmpmask, *x;
    char *current_date;
    char gmt[256];
    time_t secs, expire, set_at;
    aGline *ag;
    int perm, bad_dns, dots;

    /*
     * At the moment, let servers through, hacked or not.
     */
    if (!(MyClient(sptr) || (sptr->oflag & OFLAG_GLINE)) && !IsServer(sptr))
    {
        sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
        return 0;
    }
    if (parc < 2)
    {
        sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS), me.name, parv[0], "GLINE");
        return 0;
    }

    (void) check_expire_gline();

    tmpmask = parv[2];

    if (parc > 2)
    {
        x = strchr(tmpmask, '@');

        if (!x)
        {
            sendto_one(sptr, ":%s NOTICE %s :*** Notice -- Please use a user@mask", me.name, parv[0]);
            return 0;
        }
    }
    username = strtok(tmpmask, "@");

    if (!username)
    {
        sendto_one(sptr, ":%s NOTICE %s :*** Notice -- Missing username.", me.name, parv[0]);
        return 0;
    }

    hostname = strtok(NULL, "@");

    if (!hostname)
    {
        sendto_one(sptr, ":%s NOTICE %s :*** Notice -- Missing hostname.", me.name, parv[0]);
        return 0;
    }
    if (!strcasecmp(parv[1], "ADD"))
    {
        if (parc < 5)
        {
            sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS), me.name, parv[0], "GLINE");
            return 0;
        }

        set_by = parv[0];
        reason = parv[4];

        if (strlen(reason) > TOPICLEN)
            reason[TOPICLEN+1] = '\0';
        if (strlen(username) > (USERLEN+1))
        {
            sendto_one(sptr, ":%s NOTICE %s :*** Notice -- Usernames are limited to %i characters.", me.name, parv[0], (USERLEN+1));
            return 0;
        }
        if (strlen(hostname) > (HOSTLEN+1))
        {
            sendto_one(sptr, ":%s NOTICE %s :*** Notice -- Hostnames are limited to %i characters.", me.name, parv[0], (HOSTLEN+1));
            return 0;
        }

        if (!match(username, "akjhfkahfasfjd") && !match(hostname, "ldksjfl.kss...kdjfd.jfklsjf"))
        {
            sendto_one(sptr, ":%s NOTICE %s :*** Notice -- Cant G:LINE such a broad username/hostname mask.", me.name, parv[0]);
            return 0;
        }

        dots = 0;
        p = hostname;
        bad_dns = NO;

        while (*p)
        {
            if (!isalnum(*p))
            {
                if ((*p != '-') && (*p != '.') && (*p != '*'))
                bad_dns = YES;
            }
            if (*p == '.')
                dots++;
            p++;
        }

        if (bad_dns == YES)
        {
            sendto_one(sptr, ":%s NOTICE %s :*** Notice -- A hostname may contain a-z, 0-9, '-' & '.' & '*' - please use only these.", me.name, parv[0]);
            return 0;
        }
        if (!dots)
        {
            if (MyConnect(sptr))
                sendto_one(sptr, ":%s NOTICE %s :*** Notice -- You must have at least one '.' (dot) in the hostname.", me.name, parv[0]);
            return 0;
        }

        if (find_gline(username, hostname) == 1)
        {
            if (!IsServer(sptr) && MyClient(sptr))
                sendto_one(sptr, ":%s NOTICE %s :*** Notice -- Username/hostname mask is already on the G:Line list", me.name, parv[0]);
            return 0;
        }

        secs = atol(parv[3]);
        perm = 0;

        if (secs < 0)
        {
            if (!IsServer(sptr) && MyClient(sptr))
                sendto_one(sptr, ":%s NOTICE %s :*** Notice -- Specify a POSITIVE value for a gline expiry time", me.name, parv[0]);
            return 0;
        }
        else if (secs == 0)
        {
            perm = 1;
            expire = secs;
        }
        else
        {
            perm = 0;
            expire = secs + timeofday;
        }
#ifdef __OpenBSD__
        strncpy(gmt, asctime(gmtime((clock_t *)&expire)), sizeof(gmt));
#else
        strncpy(gmt, asctime(gmtime((time_t *)&expire)), sizeof(gmt));
#endif
        gmt[strlen(gmt) - 1] = '\0';
        set_at = timeofday;

        if (!IsServer(sptr))
        {
            if (perm == 1)
            {
                sendto_snomask (SNOMASK_NETINFO, "%s added G:Line for %s@%s which will not expire (%s)", set_by, username, 
				hostname, reason);
            }
            else
            {
                sendto_snomask (SNOMASK_NETINFO, "%s added G:Line for %s@%s which will expire on %s GMT (%s)",
				set_by, username, hostname, gmt, reason);
            }
        }
        sendto_serv_butone(cptr, ":%s GLINE ADD %s@%s %l :%s", parv[0], username, hostname, secs, reason);
        current_date = (char *) smalldate((time_t) 0);
        set_on = current_date;
        (void) add_gline(set_on, set_by, username, hostname, expire, set_at, reason);
        (void) sweep_gline();
        return 1;
    }
    else if (!strcasecmp(parv[1], "DEL"))
    {
        if (parc < 3)
        {
            sendto_one(sptr, ":%s NOTICE %s :*** Notice -- Syntax: /gline del <user@host>", me.name, parv[0]);
            sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS), me.name, parv[0], "GLINE");
            return 0;
        }
        if (find_gline(username, hostname) == 0)
        {
            if (!IsServer(sptr) && MyClient(sptr))
                sendto_one(sptr, ":%s NOTICE %s :*** Notice -- Username/hostname mask not found on the G:Line list.", me.name, parv[0]); 
            return 0;
        }

        for (ag = glines; ag; ag = ag->next)
        {
        	del_gline(username, hostname);
	        sendto_snomask(SNOMASK_NETINFO, "%s deleted G:Line for %s@%s", parv[0], username, hostname);
	        sendto_serv_butone(cptr, ":%s GLINE DEL %s@%s", parv[0], username, hostname);
	        return 0;
        }
    }
    else
    {
        sendto_one(sptr, ":%s NOTICE %s :*** Notice -- Syntax: /gline <add/del> <user@host> [duration in seconds] [:reason]", me.name, parv[0]);
        return 0;
    }
    return 0;
}

/*
 * find_gline()
 */
int find_gline(username, hostname)
    char *username, *hostname;
{
    aGline *ag;

    (void) check_expire_gline();

    for (ag = glines; ag; ag = ag->next)
    {
        if (!match(ag->username, username) && !match(ag->hostname, hostname))
            return 1;
    }
    return 0;
}

int sweep_gline()
{
    aClient *acptr;
    int i;
    char tmpbuf[1024];
    aGline *gline;

    (void) check_expire_gline();

    for (i = 0; i <= highest_fd; i++)
    {
        if (!(acptr = local[i]) || !IsClient(acptr))
            continue;
	gline = (aGline *) find_glined_user(acptr);
        if (gline)
	{
          ircsprintf(tmpbuf, "You are banned from %s: %s (%s) (contact %s for more information)", 
		IRCNETWORK_NAME, gline->reason, gline->set_on, NETWORK_KLINE_ADDRESS);
	  sendto_one(acptr, ":%s NOTICE %s :*** %s", me.name, acptr->name, tmpbuf);
	  exit_client(acptr, acptr, &me, tmpbuf);
	}
    }
    return 1;
}

/*
 * find_glined_user()
 */
aGline *find_glined_user(aClient *cptr)
{
    aGline *ag;

    (void) check_expire_gline();

    if (IsServer(cptr) || IsMe(cptr) || IsULine(cptr))
        return 0;

    for (ag = glines; ag; ag = ag->next)
    {
        if ((cptr->user->username) && (((cptr->sockhost) && (!match(ag->hostname, cptr->sockhost) && !match(ag->username, cptr->user->username))) ||
            ((cptr->hostip) && (!match(ag->hostname, cptr->hostip) && !match(ag->username, cptr->user->username)))))
        {
            return ag;
        }
    }
    return NULL;
}

void add_gline(set_on, set_by, username, hostname, expire, set_at, reason)
    char *username, *hostname;
    char *set_on, *set_by, *reason;
    time_t expire, set_at;
{
    aGline *ag;

    ag = (aGline *)MyMalloc(sizeof(aGline));

    AllocCpy(ag->set_on, set_on);
    AllocCpy(ag->set_by, set_by);
    AllocCpy(ag->username, username);
    AllocCpy(ag->hostname, hostname);
    AllocCpy(ag->reason, reason);

    ag->expire = expire;
    ag->set_at = set_at;
    ag->next = glines;
    ag->prev = NULL;

    if (glines)
    {
        glines->prev = ag;
    }
    glines = ag;
}
/*
 * del_gline()
 */
int del_gline(username, hostname)
    char *username, *hostname;
{
    aGline *ag;

    for (ag = glines; ag; ag = ag->next)
    {
        if (!match(ag->username, username) && !match(ag->hostname, hostname))
        {
            MyFree((char *)ag->set_on);
            MyFree((char *)ag->set_by);
            MyFree((char *)ag->username);
            MyFree((char *)ag->hostname);
            MyFree((char *)ag->reason);

            if (ag->prev)
                ag->prev->next = ag->next;
            else
                glines = ag->next;
            if (ag->next)
                ag->next->prev = ag->prev;

            MyFree((aGline *) ag);
            return 1;
        }
    }
    return 0;
}

/*
 * check_expire_gline()
 */
void check_expire_gline(void)
{
    aGline *ag;
    time_t nowtime = timeofday;

    for (ag = glines; ag; ag = ag->next)
    {
        if ((ag->expire <= nowtime) && (ag->expire != 0))
            expire_gline(ag);
    }
}

/*
 * expire_gline()
 */
aGline *expire_gline(ag)
    aGline *ag;
{
    sendto_snomask (SNOMASK_NETINFO, "G:Line for %s@%s (set by %s %l seconds ago) has now expired",
        ag->username, ag->hostname, ag->set_by, (timeofday - ag->set_at), ag->reason);

    del_gline(ag->username, ag->hostname);
    return 0;
}

/*
 * send_glines()
 */
void send_glines(aClient *cptr)
{
    aGline *ag;
    time_t expire;

    (void) check_expire_gline();

    for (ag = glines; ag; ag = ag->next)
    {
        expire = ag->expire;

        if (expire < 0)
            expire = -expire;
        else if (expire > 0)
            expire = expire - timeofday;
        sendto_one(cptr, ":%s GLINE ADD %s@%s %lu :%s", me.name, ag->username, ag->hostname,
            (ag->expire == 0) ? ag->expire : ag->expire - timeofday, ag->reason);
    }
}

/*
 * list_glines()
 */
void list_glines(aClient *sptr)
{
    aGline *ag;

    (void) check_expire_gline();

    if (!glines)
    {
        sendto_one(sptr, ":%s %d %s :There are currently no G:Lines in place on %s", me.name, RPL_SETTINGS, sptr->name, me.name);
        return;
    }

    for (ag = glines; ag; ag = ag->next)
    {
        sendto_one(sptr, ":%s %i %s :G %s@%s %s %s (%s)", me.name, RPL_TEXT, sptr->name,
            ag->username, ag->hostname, ag->reason, ag->set_on, ag->set_by);
    }
}
