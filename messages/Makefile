# $Id: Makefile,v 1.4 2004/08/25 00:22:13 nenolod Exp $
#
RM            = /bin/rm
MV            = /bin/mv
INSTALL       = /bin/install -c
INSTALL_DATA  = ${INSTALL} -m 644
prefix        = /home/nenolod/shadow3.2
messagedir    = /home/nenolod/shadow3.2/messages

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
