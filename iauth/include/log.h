/************************************************************************
 *   IRC - Internet Relay Chat, iauth/log.h
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   $Id: log.h,v 1.1.1.1 2003/12/02 20:47:27 nenolod Exp $
 */

#ifndef INCLUDED_log_h
#define INCLUDED_log_h

/*
 * Logging levels
 */
#define L_ERROR    1
#define L_INFO     2

/*
 * Prototypes
 */

/* Quickly fix namespace collision */
#define log _log
void _log(int level, char *format, ...);
void InitLog(char *filename);

#endif /* INCLUDED_log_h */
