/*
 *  UltimateIRCd - an Internet Relay Chat Daemon, src/s_err.c
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
 *  $Id:
 *
 */

/*
 * Copyright: Rossi 'vejeta' Marcello <vjt@users.sourceforge.net>
 */

/* va_copy hooks for IRCd */

#if defined(__powerpc__)
# if defined(__NetBSD__)
#  define VA_COPY va_copy
# elif defined(__FreeBSD__) || defined(__linux__)
#  define VA_COPY __va_copy
# endif
#else
# define VA_COPY(x, y) x = y
#endif
