# $Id: Makefile,v 1.1 2004/07/29 15:26:53 nenolod Exp $

all: respond

respond: respond.c
	gcc -g -O2 -Wall -o respond respond.c -lcrypto -ggdb

clean:
	rm -f respond *.core *~
