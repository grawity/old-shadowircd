# $Id: Makefile,v 1.7 2004/08/28 21:27:08 nenolod Exp $
#
RM            = /bin/rm
MV            = /bin/mv
INSTALL       = /usr/bin/install -c
INSTALL_DATA  = ${INSTALL} -m 644
prefix        = /home/nenolod/shadow
messagedir    = /home/nenolod/shadow/messages

CLOBBER=

LANGFILES     = fantasy.lang

all:

install:
	mkdir -p $(messagedir)
	-@echo    "Installing language files to $(messagedir)...";          \

	@for i in $(LANGFILES); do                                          \
		if test -f $(messagedir)/$$i -a -z "$(CLOBBER)"; then         \
			echo $(MV) $(messagedir)/$$i $(messagedir)/$$i.old; \
			$(MV) $(messagedir)/$$i $(messagedir)/$$i.old;      \
		fi;                                                         \
		echo $(INSTALL_DATA) $$i $(messagedir);                     \
		$(INSTALL_DATA) $$i $(messagedir);                          \
	done

distclean:
	${RM} -f Makefile

.PHONY: install distclean
