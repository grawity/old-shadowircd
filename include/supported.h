#ifndef INCLUDED_supported_h
#define INCLUDED_supported_h

#include "config.h"
#include "channel.h"
#include "dynconf.h"
#include "version.h"

#define CASEMAP "rfc1459"
#define CHARSET "ascii"
#define STD "i-d"

#define FEATURES "KNOCK" \
                " EXCEPTS" \
                " QUIETS" \
                " CALLERID" \
                " NOQUIT" \
		" SAFELIST" \
		" SSL" \
                " WATCH=%i" \
                " MAXCHANNELS=%i" \
                " MAXBANS=%i" \
		" MAXQUIETS=%i" \
                " NICKLEN=%i" \
                " TOPICLEN=%i" \
                " KICKLEN=%i" \
                " CHANNELLEN=%i" \
		" USERLEN=%i"

#define FEATURESVALUES MAXWATCH,MAXCHANNELSPERUSER,MAXBANS,MAXBANS,NICKLEN, \
        TOPICLEN,TOPICLEN,CHANNELLEN,USERLEN

#define FEATURES2 "EXCEPTS=%s" \
                  " CHANTYPES=%s" \
                  " PREFIX=%s" \
                  " CHANMODES=%s" \
                  " STATUSMSG=%s" \
                  " NETWORK=%s" \
                  " CASEMAPPING=%s" \
                  " CHARSET=%s" \
                  " STD=%s" \
		  " IRCD=%s"

#define FEATURES2VALUES "e", \
                        "#&", \
                        "(ohv)@%+", \
                        "be,k,l,cimnpstKNORS", \
                        "@%+", \
                        IRCNETWORK, \
                        CASEMAP, \
                        CHARSET, \
                        STD, \
			BASEVERSION


#endif /* INCLUDED_supported_h */
