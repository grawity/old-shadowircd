/* $Id: setup.c,v 1.1 2004/09/08 01:50:26 nenolod Exp $ */

int
sdns_init(void)
{
	irc_nscount = 0;
#ifdef __mingw32__
	if(VerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		return (get_res_from_reg_9x());
	else if(VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		return (get_res_from_reg_nt());
#else
	return (parse_resvconf());
#endif
}

int
parse_resvconf(void)
{
	char *p;
	char *opt;
	char *arg;
	char input[MAXLINE];
	FBFILE *file;

#ifndef __mingw32__
	if((file = fbopen("/etc/resolv.conf", "r")) == NULL)
#else
	if((file = fbopen("/cygdrive/c/ShadowIRCd/etc/nameservers.conf", "r")) == NULL)
#endif
		return (-1);

	while (fbgets(input, MAXLINE, file) != NULL)
	{
		/* blow away any newline */
		if((p = strpbrk(input, "\r\n")) != NULL)
			*p = '\0';

		/* Ignore comment lines immediately */
		if(*input == '#')
			continue;

		p = input;
		/* skip until something thats not a space is seen */
		while (IsSpace(*p))
			p++;
		/* if at this point, have a '\0' then continue */
		if(*p == '\0')
			continue;

		/* skip until a space is found */
		opt = input;
		while (!IsSpace(*p))
			if(*p++ == '\0')
				continue;	/* no arguments?.. ignore this line */
		/* blow away the space character */
		*p++ = '\0';

		/* skip these spaces that are before the argument */
		while (IsSpace(*p))
			p++;
		/* Now arg should be right where p is pointing */
		arg = p;
		if((p = strpbrk(arg, " \t")) != NULL)
			*p = '\0';	/* take the first word */

		if(strcasecmp(opt, "domain") == 0)
			strlcpy(irc_domain, arg, HOSTLEN);
		else if(strcasecmp(opt, "nameserver") == 0)
			add_nameserver(arg);
	}

	fbclose(file);
	return (0);
}

void
add_nameserver(char *arg)
{
	struct addrinfo hints, *res;
	/* Done max number of nameservers? */
	if((irc_nscount + 1) >= IRCD_MAXNS)
		return;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;

	if(irc_getaddrinfo(arg, "domain", &hints, &res))
		return;

	if(res == NULL)
		return;

	memcpy(&irc_nsaddr_list[irc_nscount].ss, res->ai_addr, res->ai_addrlen);
	irc_nsaddr_list[irc_nscount].ss_len = res->ai_addrlen;
	irc_nscount++;
	irc_freeaddrinfo(res);
}
