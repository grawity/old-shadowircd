/* This file is part of shadowdns, a callback-driven asynchronous dns library.
 * encode.c: various encoding functions.
 *
 * $Id: encode.c,v 1.1 2004/09/08 22:50:47 nenolod Exp $
 */

static int
sdns_ns_name_pton(const char *src, unsigned char *dst, size_t dstsiz)
{
        unsigned char *label, *bp, *eom;
        char *cp;
        int c, n, escaped, e = 0;
                                                                                                                                               
        escaped = 0;
        bp = dst;
        eom = dst + dstsiz;
        label = bp++;
                                                                                                                                               
                                                                                                                                               
        while ((c = *src++) != 0)
        {
                if(escaped)
                {
                        if(c == '[')
                        {       /* start a bit string label */
                                if((cp = strchr(src, ']')) == NULL)
                                {
                                        errno = EINVAL; /* ??? */
                                        return (-1);
                                }
                                if((e = sdns_encode_bitsring(&src,
                                                            cp + 2,
                                                            &label, &bp, (const char *) eom)) != 0)
                                {
                                        errno = e;
                                        return (-1);
                                }
                                escaped = 0;
                                label = bp++;
                                if((c = *src++) == 0)
                                        goto done;
                                else if(c != '.')
                                {
                                        errno = EINVAL;
                                        return (-1);
                                }
                                continue;
                        }
                        else if((cp = strchr(digits, c)) != NULL)
                        {
                                n = (cp - digits) * 100;
                                if((c = *src++) == 0 || (cp = strchr(digits, c)) == NULL)
                                {
                                        errno = EMSGSIZE;
                                        return (-1);
                                }
                                n += (cp - digits) * 10;
                                if((c = *src++) == 0 || (cp = strchr(digits, c)) == NULL)
                                {
                                        errno = EMSGSIZE;
                                        return (-1);
                                }
                                n += (cp - digits);
                                if(n > 255)
                                {
                                        errno = EMSGSIZE;
                                        return (-1);
                                }
                                c = n;
                        }
                        escaped = 0;
                }
                else if(c == '\\')
                {
                        escaped = 1;
                        continue;
                }
                else if(c == '.')
                {
                        c = (bp - label - 1);
                        if((c & NS_CMPRSFLGS) != 0)
                        {       /* Label too big. */
                                errno = EMSGSIZE;
                                return (-1);
                        }
                        if(label >= eom)
                        {
                                errno = EMSGSIZE;
                                return (-1);
                        }
                        *label = c;
                        /* Fully qualified ? */
                        if(*src == '\0')
                        {
                                if(c != 0)
                                {
                                        if(bp >= eom)
                                        {
                                                errno = EMSGSIZE;
                                                return (-1);
                                        }
                                        *bp++ = '\0';
                                }
                                if((bp - dst) > NS_MAXCDNAME)
                                {
                                        errno = EMSGSIZE;
                                        return (-1);
                                }
                                return (1);
                        }
                        if(c == 0 || *src == '.')
                        {
                                errno = EMSGSIZE;
                                return (-1);
                        }
                        label = bp++;
                        continue;
                }
                if(bp >= eom)
                {
                        errno = EMSGSIZE;
                        return (-1);
                }
                *bp++ = (unsigned char) c;
        }
        c = (bp - label - 1);
        if((c & NS_CMPRSFLGS) != 0)
        {                       /* Label too big. */
                errno = EMSGSIZE;
                return (-1);
        }
      done:
        if(label >= eom)
        {
                errno = EMSGSIZE;
                return (-1);
        }
        *label = c;
        if(c != 0)
        {
                if(bp >= eom)
                {
                        errno = EMSGSIZE;
                        return (-1);
                }
                *bp++ = 0;
        }
                                                                                                                                               
        if((bp - dst) > NS_MAXCDNAME)
        {                       /* src too big */
                errno = EMSGSIZE;
                return (-1);
        }
                                                                                                                                               
        return (0);
}

