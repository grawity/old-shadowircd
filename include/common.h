/*
 *  UltimateIRCd - an Internet Relay Chat Daemon, include/common.h
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
 *  $Id: common.h,v 1.1 2003/11/04 07:11:55 nenolod Exp $
 *
 */


#ifndef	__common_include__
# define __common_include__
# define IRCD_MIN(a, b)  ((a) < (b) ? (a) : (b))
# if defined( HAVE_PARAM_H )
#	include <sys/param.h>
# endif
# ifndef NULL
#	define NULL 0
# endif
# ifdef TRUE
#	undef TRUE
# endif
# ifdef FALSE
#	undef FALSE
# endif
# define FALSE (0)
# define TRUE  (!FALSE)
# define HIDEME 2
/* Blah. I use these a lot. -Dianora */
# ifdef YES
#	undef YES
# endif
# define YES 1
# ifdef NO
#	undef NO
# endif
# define NO  0
# ifdef FOREVER
#	undef FOREVER
# endif
# define FOREVER for(;;)
/* -Dianora */
# if !defined(STDC_HEADERS)
char *malloc (), *calloc ();
void free ();
# else
#	ifdef MALLOCH
#	 include MALLOCH
#	endif
# endif
extern void flush_fdlist_connections ();

/*
 * irccmp - case insensitive comparison of s1 and s2
 */
extern int irccmp(const char *s1, const char *s2);

/*
 * ircncmp - counted case insensitive comparison of s1 and s2
 */
extern int ircncmp(const char *s1, const char *s2, int n);

extern int irccmp_lex(const char *s1, const char *s2);

# if !defined( HAVE_STRTOK )
extern char *strtok (char *, char *);
# endif
# if !defined( HAVE_STRTOKEN )
extern char *strtoken (char **, char *, char *);
# endif
# if !defined( HAVE_INET_ADDR )
extern unsigned long inet_addr (char *);
# endif
# if !defined(HAVE_INET_NTOA) || !defined(HAVE_INET_NETOF)
#	include <netinet/in.h>
# endif
# if !defined( HAVE_INET_NTOA )
extern char *inet_ntoa (struct in_addr);
# endif
# if !defined( HAVE_INET_NETOF )
extern int inet_netof (struct in_addr);
# endif
extern char *myctime (time_t);
extern char *strtoken (char **, char *, char *);
# if !defined(HAVE_MINMAX)
#	ifndef MAX
#	 define MAX(a, b)	((a) > (b) ? (a) : (b))
#	endif
#	ifndef MIN
#	 define MIN(a, b)	((a) < (b) ? (a) : (b))
#	endif
# endif	/* !HAVE_MINMAX */
# define DupString(x,y) do{x=MyMalloc(strlen(y)+1);(void)strcpy(x,y);}while(0)
extern unsigned char tolowertab[];
# undef tolower
# define tolower(c) (tolowertab[(u_char)(c)])
extern unsigned char touppertab[];
# undef toupper
# define toupper(c) (touppertab[(u_char)(c)])
# undef isalpha
# undef isdigit
# undef isxdigit
# undef isalnum
# undef isprint
# undef isascii
# undef isgraph
# undef ispunct
# undef islower
# undef isupper
# undef isspace
# undef iscntrl
extern unsigned char char_atribs[];
# define PRINT 1
# define CNTRL 2
# define ALPHA 4
# define PUNCT 8
# define DIGIT 16
# define SPACE 32
# define	iscntrl(c) (char_atribs[(u_char)(c)]&CNTRL)
# define isalpha(c) (char_atribs[(u_char)(c)]&ALPHA)
# define isspace(c) (char_atribs[(u_char)(c)]&SPACE)
# define islower(c) ((char_atribs[(u_char)(c)]&ALPHA) && ((u_char)(c) > 0x5f))
# define isupper(c) ((char_atribs[(u_char)(c)]&ALPHA) && ((u_char)(c) < 0x60))
# define isdigit(c) (char_atribs[(u_char)(c)]&DIGIT)
# define	isxdigit(c) (isdigit(c) || 'a' <= (c) && (c) <= 'f' || \
                      'A' <= (c) && (c) <= 'F')
# define isalnum(c) (char_atribs[(u_char)(c)]&(DIGIT|ALPHA))
# define isprint(c) (char_atribs[(u_char)(c)]&PRINT)
# define isascii(c) ((u_char)(c) >= 0 && (u_char)(c) <= 0x7f)
# define isgraph(c) ((char_atribs[(u_char)(c)]&PRINT) && ((u_char)(c) != 0x32))
# define ispunct(c) (!(char_atribs[(u_char)(c)]&(CNTRL|ALPHA|DIGIT)))
extern struct SLink *find_user_link ();
#endif /* common_include */
