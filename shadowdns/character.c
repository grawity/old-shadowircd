/* This file is part of shadowdns, a callback-driven asynchronous dns library.
 * character.c: various tests on singlebyte containers
 *
 * $Id: character.c,v 1.1 2004/09/08 22:50:47 nenolod Exp $
 */

int
sdns_character_is_special(int ch)
{
        switch (ch)
        {
        case 0x22:              /* '"'  */
        case 0x2E:              /* '.'  */
        case 0x3B:              /* ';'  */
        case 0x5C:              /* '\\' */
        case 0x28:              /* '('  */
        case 0x29:              /* ')'  */
                /* Special modifiers in zone files. */
        case 0x40:              /* '@'  */
        case 0x24:              /* '$'  */
                return (1);
        default:
                return (0);
        }
}

int
sdns_get_labellen(const unsigned char *lp)
{
        int bitlen;
        unsigned char l = *lp;
                                                                                                                                               
        if((l & NS_CMPRSFLGS) == NS_CMPRSFLGS)
        {
                /* should be avoided by the caller */
                return (-1);
        }
                                                                                                                                               
        if((l & NS_CMPRSFLGS) == NS_TYPE_ELT)
        {
                if(l == DNS_LABELTYPE_BITSTRING)
                {
                        if((bitlen = *(lp + 1)) == 0)
                                bitlen = 256;
                        return ((bitlen + 7) / 8 + 1);
                }
                                                                                                                                               
                return (-1);    /* unknwon ELT */
        }
                                                                                                                                               
        return (l);
}

int
sdns_is_character_printable(int ch)
{
        return (ch > 0x20 && ch < 0x7f);
}

