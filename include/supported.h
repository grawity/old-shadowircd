/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  supported.h: Header for 005 numeric etc...
 *
 *  Copyright (C) 2002 by the past and present ircd coders, and others.
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
 *  $Id: supported.h,v 1.6 2004/02/12 23:02:22 nenolod Exp $
 */

#ifndef INCLUDED_supported_h
#define INCLUDED_supported_h

#include "channel.h"
#include "ircd_defs.h"
#include "s_serv.h"

#define CASEMAP "rfc1459"

#ifdef HAVE_LIBCRYPTO
#define FEATURES "SSL WALLCHOPS"   \
                 "%s%s%s"          \
                 " MODES=%i"       \
                 " MAXCHANNELS=%i" \
                 " MAXBANS=%i"     \
                 " MAXTARGETS=%i"  \
                 " NICKLEN=%i"     \
                 " TOPICLEN=%i"    \
                 " KICKLEN=%i"
#else
#define FEATURES "WALLCHOPS"       \
                 "%s%s%s"          \
                 " MODES=%i"       \
                 " MAXCHANNELS=%i" \
                 " MAXBANS=%i"     \
                 " MAXTARGETS=%i"  \
                 " NICKLEN=%i"     \
                 " TOPICLEN=%i"    \
                 " KICKLEN=%i"
#endif

#define FEATURESVALUES ConfigChannel.use_knock ? " KNOCK" : "", \
        ConfigChannel.use_except ? " EXCEPTS" : "", \
        ConfigChannel.use_invex ? " INVEX" : "", \
        MAXMODEPARAMS,ConfigChannel.max_chans_per_user, \
        ConfigChannel.max_bans, \
        ConfigFileEntry.max_targets,NICKLEN-1,TOPICLEN,TOPICLEN

#define FEATURES2 "CHANTYPES=%s"      \
                  " PREFIX=%s"        \
		  " CHANMODES=%s%s%s" \
		  " NETWORK=%s"       \
		  " CASEMAPPING=%s"   \
		  " GRANT ETRACE CALLERID%s"

#define FEATURES2VALUES ConfigChannel.disable_local_channels ? "#" : "#&", \
                        "(uohv)!@%+", \
                        ConfigChannel.use_except ? "e" : "", \
                        ConfigChannel.use_invex ? "I" : "", \
                        "b,k,l,imnpstTFcz", \
                        ServerInfo.network_name, CASEMAP, \
			(uplink && IsCapable(uplink, CAP_LL)) ? "" : " SAFELIST"

/*
 * mIRC's readme does not concern me. --nenolod.
 */

#endif /* INCLUDED_supported_h */
