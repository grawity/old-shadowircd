CC		= gcc
RM		= /bin/rm
AR		= /usr/bin/ar
MV		= /bin/mv
CP		= /bin/cp
INSTALL		= /usr/bin/install -c
PREFIX		= /usr/local
BIN		= libopm.a
MKDEP		= @MKDEP@

# Default CFLAGS
#CFLAGS		= -g -O2 -Wall

# Developer CFLAGS
CFLAGS		= -g -O2 -Wunused -Wall -ggdb -pedantic -Wshadow -Wmissing-declarations -Wno-long-long

CFLAGS		+= -I../include/

VERSION		= 1.0

LIBS		= 
LDFLAGS		= 
CPPFLAGS	= 

default: all

BASE_SRCS = compat.c config.c inet.c libopm.c list.c malloc.c proxy.c snprintf.c
SRCS = ${BASE_SRCS}

OBJS = ${SRCS:.c=.o}

all: libopm.a

build: all

libopm.a: $(OBJS)
	$(AR) csrv $@ $(OBJS)

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} -c $<

clean:
	${RM} -f *.o *.a

distclean: clean
	${RM} -f Makefile
