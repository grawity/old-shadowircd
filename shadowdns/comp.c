/* $Id: comp.c,v 1.2 2004/09/08 22:33:05 nenolod Exp $ */

int
sdns_dn_comp(const char *src, unsigned char *dst, int dstsiz,
	     unsigned char **dnptrs, unsigned char **lastdnptr)
{
	return (sdns_name_compress(src, dst, (size_t) dstsiz,
				   (const unsigned char **) dnptrs,
				   (const unsigned char **) lastdnptr));
}

int
sdns_dn_skipname(const unsigned char *ptr, const unsigned char *eom)
{
        const unsigned char *saveptr = ptr;
                                                                                                                                               
        if(sdns_ns_name_skip(&ptr, eom) == -1)
                return (-1);
        return (ptr - saveptr);
}

int
sdns_ns_name_skip(const unsigned char **ptrptr, const unsigned char *eom)
{
        const unsigned char *cp;
        unsigned int n;
        int l;
                                                                                                                                               
        cp = *ptrptr;
                                                                                                                                               
        while (cp < eom && (n = *cp++) != 0)
        {
                /* Check for indirection. */
                switch (n & NS_CMPRSFLGS)
                {
                case 0: /* normal case, n == len */
                        cp += n;
                        continue;
                case NS_TYPE_ELT:       /* EDNS0 extended label */
                        if((l = labellen(cp - 1)) < 0)
                        {
                                errno = EMSGSIZE;       /* XXX */
                                return (-1);
                        }
                                                                                                                                               
                        cp += l;
                        continue;
                case NS_CMPRSFLGS:      /* indirection */
                        cp++;
                        break;
                default:        /* illegal type */
                        errno = EMSGSIZE;
                        return (-1);
                }
                                                                                                                                               
                break;
        }
                                                                                                                                               
        if(cp > eom)
        {
                errno = EMSGSIZE;
                return (-1);
        }
                                                                                                                                               
        *ptrptr = cp;
        return (0);
}

