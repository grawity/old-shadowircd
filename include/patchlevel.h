/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  patchlevel.h: A header defining the patchlevel.
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
 *  $Id: patchlevel.h,v 3.7 2004/09/25 17:30:21 nenolod Exp $
 */

#ifndef PATCHLEVEL
#define RELEASE 1
#define BETA 2
#define ALPHA 3
#define CURRENT 4

#define BRANCHSTATUS RELEASE

#define BASENAME "ShadowIRCd"
#define MAJOR 3
#define MINOR 3

#define PATCH1 "-rc2"
#define PATCH2 ""
#define PATCH3 ""
#define PATCH4 ""
#define PATCH5 ""

#define PATCHES PATCH1 PATCH2 PATCH3 PATCH4 PATCH5

void build_version(void);

#endif
