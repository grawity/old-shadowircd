/*
 *  shadowircd: an advanced Internet Relay Chat Daemon(ircd).
 *  client_silence.h: Controls silence lists.
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
 *  $Id: client_silence.h,v 1.1 2004/09/07 04:50:40 nenolod Exp $
 */

#ifndef CLIENT_SILENCE_H
#define CLIENT_SILENCE_H
int is_silenced(struct Client *, struct Client *);
int add_silence(struct Client *client_p, char *silenceid);
int del_silence(struct Client *client_p, char *banid);
#endif
