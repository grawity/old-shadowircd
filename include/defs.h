/*
 *  UltimateIRCd - an Internet Relay Chat Daemon, include/defs.h
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
 *  $Id: defs.h,v 1.1 2003/11/04 07:11:55 nenolod Exp $
 *
 */


#if !defined( DEFS_H_INCLUDED )
# define DEFS_H_INCLUDED 1

/*
 * None of the following should need to be changed by hand.  
 */


# if !defined( HAVE_BCOPY )
#	define bcopy(a,b,c) memcpy(b,a,c)
# endif
# if !defined( HAVE_BCMP )
#	define bcmp memcmp
# endif
# if !defined( HAVE_BZERO )
#	define bzero(a,b) memset(a,0,b)
# endif

# if defined( __osf__ )
#	define OSF
#	undef BSD
#	include <sys/param.h>
#	if !defined( BSD )
#	 define BSD
#	endif
# endif

# if !defined(HAVE_DN_SKIPNAME)
#	if defined(HAVE___DN_SKIPNAME)
#	 define dn_skipname __dn_skipname
#	else
#	error Could not find dn_skipname() or __dn_skipname()
# endif
#endif
/*
 * The following OS specific stuff is a compatibility kludge it would
 * be nice to get rid of all of this eventually.
 */
#if defined(OS_SOLARIS2) && !defined( SOL20 )
# define SOL20 1
# define USE_POLL 1		/* Get around stupid select() limitations */
#endif

#if defined( OS_ESIX ) && !defined( ESIX )
# define ESIX 1
#endif

#if defined( OS_NEXT ) && !defined( NEXT )
# define NEXT 1
#endif

#if defined( ultrix ) && !defined( ULTRIX )
# define ULTRIX 1
#endif

#if (defined( sgi ) || defined( __sgi )) && !defined( SGI )
# define SGI 1
#endif

#if defined( aix ) || defined( OS_AIX )
# include <sys/machine.h>
# if BYTE_ORDER == BIG_ENDIAN
#	define BIT_ZERO_ON_LEFT
# elif BYTE_ORDER == LOTTLE_ENDIAN
#	define BIT_ZERO_ON_RIGHT
# endif
# define BSD_INCLUDES
# if !defined( AIX )
#	define AIX 1
# endif
# define USE_POLL 1		/* KLUGE - only define on AIX 4.x!! -cab */
#endif

#if defined( OS_MIPS )
# undef SYSV
# undef BSD
# define BSD 1			/* mips only works in a bsd43 environment */
# if !defined( MIPS )
#	define MIPS 1
# endif
#endif

#if defined( BSD_RELIABLE_SIGNALS ) && (defined( SYSV_UNRELIABLE_SIGNALS ) || defined( POSIX_SIGNALS ))
error You defined too many signal types in setup.h
#elif defined( SYSV_UNRELIABLE_SIGNALS ) && defined( POSIX_SIGNALS )
error You defined too many signal types in setup.h
#endif
#if defined( BSD_RELIABLE_SIGNALS ) || defined( POSIX_SIGNALS )
# define HAVE_RELIABLE_SIGNALS
#endif
#endif				/* DEFS_H_INCLUDED */
