#ifndef SEND_H
#define SEND_H

extern int send_queued (aClient *);

#include <stdarg.h>
#include "fdlist.h"

extern void init_send ();

extern void send_operwall (aClient *, char *, char *);
extern void sendto_netglobal (char *pattern, ...);

extern void sendto_all_butone (aClient * one, aClient * from, char *pattern,
			       ...);
extern void sendto_channel_butone (aClient * one, aClient * from,
				   aChannel * channel_p, char *pattern, ...);
extern void sendto_channel_butserv (aChannel * channel_p, aClient * from,
				    char *pattern, ...);
extern void sendto_allchannelops_butone (aClient * one, aClient * from,
					 aChannel * channel_p, char *pattern,
					 ...);
extern void sendto_channelops_butone (aClient * one, aClient * from,
				      aChannel * channel_p, char *pattern, ...);
extern void sendto_channelhalfops_butone (aClient * one, aClient * from,
					  aChannel * channel_p, char *pattern,
					  ...);
extern void sendto_channelvoice_butone (aClient * one, aClient * from,
					aChannel * channel_p, char *pattern, ...);
extern void sendto_channelvoiceops_butone (aClient * one, aClient * from,
					   aChannel * channel_p, char *patern,
					   ...);
extern void sendto_common_channels (aClient * user, char *pattern, ...);
extern void send_quit_to_common_channels (aClient * from, char *reason);
extern void send_part_to_common_channels (aClient * from);
extern void sendto_fdlist (fdlist * listp, char *pattern, ...);
extern void sendto_locops (char *pattern, ...);
extern void sendto_match_butone (aClient * one, aClient * from, char *mask,
				 int what, char *pattern, ...);
extern void sendto_match_servs (aChannel * channel_p, aClient * from,
				char *format, ...);
extern void sendto_one (aClient * to, char *pattern, ...);
extern void sendto_ops (char *pattern, ...);
extern void sendto_ops_butone (aClient * one, aClient * from, char *pattern,
			       ...);
extern void sendto_prefix_one (aClient * to, aClient * from, char *pattern,
			       ...);

extern void sendto_realops_lev (int lev, char *pattern, ...);
extern void sendto_realops (char *pattern, ...);
extern void sendto_serv_butone (aClient * one, char *pattern, ...);
extern void sendto_wallops_butone (aClient * one, aClient * from,
				   char *pattern, ...);
extern void sendto_gnotice (char *pattern, ...);

extern void ts_warn (char *pattern, ...);

extern void sendto_servs (aChannel * channel_p, aClient * from,
				 char *pattern, ...);

extern void sendto_noquit_servs_butone (int noquit, aClient * one,
					char *pattern, ...);
extern void sendto_nickip_servs_butone (int nonickip, aClient * one,
					char *pattern, ...);
extern void sendto_clientcapab_servs_butone (int clientcapab, aClient * one,
					char *pattern, ...);

extern void sendto_snomask (int snomask, char *pattern, ...);

extern void vsendto_fdlist (fdlist * listp, char *pattern, va_list vl);
extern void vsendto_one (aClient * to, char *pattern, va_list vl);
extern void vsendto_prefix_one (aClient * to, aClient * from, char *pattern,
				va_list vl);
extern void vsendto_realops (char *pattern, va_list vl);

extern void flush_connections ();
extern void dump_connections ();
#endif
