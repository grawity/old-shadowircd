/* This file is part of shadowdns, a callback-driven asynchronous dns library.
 * conversion.c: Conversion routines.
 *
 * $Id: conversion.c,v 1.1 2004/09/08 22:33:05 nenolod Exp $ 
 */

unsigned int
sdns_get16(const unsigned char *src)
{
        unsigned int dst;
        const unsigned char *t_cp = (const unsigned char *)src;

        dst = ((u_int16_t)t_cp[0] << 8) | ((u_int16_t)t_cp[1]);
        src += NS_INT16SZ;
        return (dst);
}
                                                                                                                                               
unsigned long
sdns_get32(const unsigned char *src)
{
        unsigned long dst;

	const unsigned char *t_cp = (const unsigned char *)src;
	dst = ((u_int32_t)t_cp[0] << 24) | ((u_int32_t)t_cp[1] << 16) |
              ((u_int32_t)t_cp[2] << 8) | ((u_int32_t)t_cp[3]);
	src += NS_INT32SZ;
        return (dst);
}
                                                                                                                                               
void
sdns_put16(unsigned int src, unsigned char *dst)
{
        u_int16_t t_s = (u_int16_t)src;
        unsigned char *t_cp = (unsigned char *)dst;
        *t_cp++ = t_s >> 8;
        *t_cp   = t_s;
        dst += NS_INT16SZ;
}
                                                                                                                                               
void
sdns_put32(unsigned long src, unsigned char *dst)
{
        u_int32_t t_l = (u_int32_t)src;
        unsigned char *t_cp = (unsigned char *)dst;
	*t_cp++ = t_l >> 24;
	*t_cp++ = t_l >> 16;
        *t_cp++ = t_l >> 8;
        *t_cp   = t_l;
        dst += NS_INT32SZ;
}

