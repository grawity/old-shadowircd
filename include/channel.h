#ifndef	__channel_include__
# define __channel_include__
# define find_channel(chname, channel_p) (hash_find_channel(chname, channel_p))
# define CREATE 1		/* whether a channel should be created or just
				 * tested for existance */
# define MODEBUFLEN		200	/* max modebuf we consider from users */
# define REALMODEBUFLEN		512	/* max actual modebuf */
# define NullChn 		((aChannel *) NULL)
# define ChannelExists(n) 	(find_channel(n, NullChn) != NullChn)
#	include "msg.h"
#	define	MAXMODEPARAMS	(MAXPARA-4)
#	define MAXTSMODEPARAMS  (MAXPARA-5)
#define MAXMODEPARAMSUSER 6
#endif
