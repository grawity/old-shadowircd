/* $Id: comp.c,v 1.1 2004/09/08 01:50:26 nenolod Exp $ */

int
sdns_dn_comp(const char *src, unsigned char *dst, int dstsiz,
	     unsigned char **dnptrs, unsigned char **lastdnptr)
{
	return (sdns_name_compress(src, dst, (size_t) dstsiz,
				   (const unsigned char **) dnptrs,
				   (const unsigned char **) lastdnptr));
}
