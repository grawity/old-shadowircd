/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  dh.h: Diffie-Hellman key exchange header.
 *
 *  Copyright (C) 2003 by the past and present ircd coders, and others.
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
 *  $Id: dh.h,v 1.1 2004/07/29 20:05:56 nenolod Exp $
 */

#ifndef INCLUDED_dh_h
#define INCLUDED_dh_h

#include "setup.h"

#ifdef HAVE_LIBCRYPTO
#include "client.h"

#define KEY_BITS 512

#define RAND_BITS KEY_BITS
#define RAND_BYTES (RAND_BITS / 8)
#define RAND_BYTES_HEX ((RAND_BYTES * 2) + 1)

#define PRIME_BITS 1024
#define PRIME_BYTES (PRIME_BITS / 8)
#define PRIME_BYTES_HEX ((PRIME_BYTES * 2) + 1)

struct dh_session {
   DH *dh;
   char *session_shared;
   int session_shared_length;
};

void dh_init(void);
extern int dh_generate_shared(struct dh_session *, char *);
extern int dh_get_s_shared(char *, int *, struct dh_session *);
extern int dh_hexstr_to_raw(char *, unsigned char *, int *);
extern void dh_end_session(struct dh_session *);
extern struct dh_session *dh_start_session(void);
extern char *dh_get_s_secret(char *, int, struct dh_session *);
extern char *dh_get_s_public(char *, int, struct dh_session *);

extern int safe_SSL_read(struct Client *, void *, int);
extern int safe_SSL_write(struct Client *, const void *, int);
extern int safe_SSL_accept(struct Client *, int);
extern int SSL_smart_shutdown(SSL *);
extern int initssl(void);
extern SSL_CTX *ircdssl_ctx;

#endif

extern void ircd_init_ssl(void);

#endif  /* INCLUDED_dh_h */
