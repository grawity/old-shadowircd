/* $Id: util.c,v 1.1 2004/09/08 01:50:26 nenolod Exp $ */

int
sdns_dn_expand(const unsigned char *msg, const unsigned char *eom,
	       const unsigned char *src, char *dst, int dstsiz)
{
	int n = sdns_name_uncompress(msg, eom, src, dst, (size_t) dstsiz);

	if(n > 0 && dst[0] == '.')
		dst[0] = '\0';
	return (n);
}

int
sdns_name_uncompress(const unsigned char *msg, const unsigned char *eom,
		     const unsigned char *src, char *dst, size_t dstsiz)
{
	unsigned char tmp[NS_MAXCDNAME];
	int n;

	if((n = sdns_name_unpack(msg, eom, src, tmp, sizeof tmp)) == -1)
		return (-1);
	if(irc_sdns_name_ntop((char *) tmp, dst, dstsiz) == -1)
		return (-1);
	return (n);
}

int
sdns_name_unpack(const unsigned char *msg, const unsigned char *eom,
		 const unsigned char *src, unsigned char *dst, size_t dstsiz)
{
	const unsigned char *srcp, *dstlim;
	unsigned char *dstp;
	int n, len, checked, l;

	len = -1;
	checked = 0;
	dstp = dst;
	srcp = src;
	dstlim = dst + dstsiz;
	if(srcp < msg || srcp >= eom)
	{
		errno = EMSGSIZE;
		return (-1);
	}
	/* Fetch next label in domain name. */
	while ((n = *srcp++) != 0)
	{
		/* Check for indirection. */
		switch (n & NS_CMPRSFLGS)
		{
		case 0:
		case NS_TYPE_ELT:
			/* Limit checks. */
			if((l = labellen(srcp - 1)) < 0)
			{
				errno = EMSGSIZE;
				return (-1);
			}
			if(dstp + l + 1 >= dstlim || srcp + l >= eom)
			{
				errno = EMSGSIZE;
				return (-1);
			}
			checked += l + 1;
			*dstp++ = n;
			memcpy(dstp, srcp, l);
			dstp += l;
			srcp += l;
			break;

		case NS_CMPRSFLGS:
			if(srcp >= eom)
			{
				errno = EMSGSIZE;
				return (-1);
			}
			if(len < 0)
				len = srcp - src + 1;
			srcp = msg + (((n & 0x3f) << 8) | (*srcp & 0xff));
			if(srcp < msg || srcp >= eom)
			{	/* Out of range. */
				errno = EMSGSIZE;
				return (-1);
			}
			checked += 2;
			/*
			 * Check for loops in the compressed name;
			 * if we've looked at the whole message,
			 * there must be a loop.
			 */
			if(checked >= eom - msg)
			{
				errno = EMSGSIZE;
				return (-1);
			}
			break;

		default:
			errno = EMSGSIZE;
			return (-1);	/* flag error */
		}
	}
	*dstp = '\0';
	if(len < 0)
		len = srcp - src;
	return (len);
}

int
sdns_name_ntop(const char *src, char *dst, size_t dstsiz)
{
	const char *cp;
	char *dn, *eom;
	unsigned char c;
	unsigned int n;
	int l;

	cp = src;
	dn = dst;
	eom = dst + dstsiz;

	while ((n = *cp++) != 0)
	{
		if((n & NS_CMPRSFLGS) == NS_CMPRSFLGS)
		{
			/* Some kind of compression pointer. */
			errno = EMSGSIZE;
			return (-1);
		}
		if(dn != dst)
		{
			if(dn >= eom)
			{
				errno = EMSGSIZE;
				return (-1);
			}
			*dn++ = '.';
		}
		if((l = labellen((unsigned char *) (cp - 1))) < 0)
		{
			errno = EMSGSIZE;	/* XXX */
			return (-1);
		}
		if(dn + l >= eom)
		{
			errno = EMSGSIZE;
			return (-1);
		}
		if((n & NS_CMPRSFLGS) == NS_TYPE_ELT)
		{
			int m;

			if(n != DNS_LABELTYPE_BITSTRING)
			{
				/* XXX: labellen should reject this case */
				errno = EINVAL;
				return (-1);
			}
			if((m = irc_decode_bitstring(&cp, dn, eom)) < 0)
			{
				errno = EMSGSIZE;
				return (-1);
			}
			dn += m;
			continue;
		}
		for ((void) NULL; l > 0; l--)
		{
			c = *cp++;
			if(special(c))
			{
				if(dn + 1 >= eom)
				{
					errno = EMSGSIZE;
					return (-1);
				}
				*dn++ = '\\';
				*dn++ = (char) c;
			}
			else if(!printable(c))
			{
				if(dn + 3 >= eom)
				{
					errno = EMSGSIZE;
					return (-1);
				}
				*dn++ = '\\';
				*dn++ = digits[c / 100];
				*dn++ = digits[(c % 100) / 10];
				*dn++ = digits[c % 10];
			}
			else
			{
				if(dn >= eom)
				{
					errno = EMSGSIZE;
					return (-1);
				}
				*dn++ = (char) c;
			}
		}
	}
	if(dn == dst)
	{
		if(dn >= eom)
		{
			errno = EMSGSIZE;
			return (-1);
		}
		*dn++ = '.';
	}
	if(dn >= eom)
	{
		errno = EMSGSIZE;
		return (-1);
	}
	*dn++ = '\0';
	return (dn - dst);
}
