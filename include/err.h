/*
 * err.h
 * HybServ2 Services by HybServ2 team
 *
 * $Id: err.h,v 1.1 2003/12/16 19:52:37 nenolod Exp $
 */

#ifndef INCLUDED_err_h
#define INCLUDED_err_h

#define    ERR_NOPRIVS  "No privileges"

/* NickServ error messages */
#define    ERR_NOT_YOUR_NICK  "This nickname is owned by someone else"
#define    ERR_NEED_IDENTIFY  "If this is your nickname, type /msg %s \002IDENTIFY\002 <password>"
#define    ERR_MUST_CHANGE    "If you do not \002IDENTIFY\002 within one minute, you will be disconnected"
#define    ERR_MUST_CHANGE2   "If you do not \002IDENTIFY\002 within one minute, you nickname will be changed"
#define    ERR_NOT_REGGED    "The nickname [\002%s\002] is not registered"
#define    ERR_MORE_INFO      "Type: \002/msg %s HELP %s\002 for more information"
#define    ERR_BAD_PASS      "Password Incorrect"
#define    ERR_NO_HELP        "No help available on \002%s %s\002"
#define   ERR_NOT_LINKED    "The nickname [\002%s\002] is not linked"

/* ChanServ error messages */
#define    ERR_CH_NOT_REGGED  "The channel [\002%s\002] is not registered"
#define    ERR_NEED_ACCESS    "An access level of [\002%d\002] is required for [\002%s\002] on %s"

#endif /* INCLUDED_err_h */
