CC = g++
MC = g++
CFLAGS = -ggdb -I/home/eko/mysql/include
CLIBS = -ldl -lmysqlclient
MFLAGS = -shared ${CFLAGS} #-fPIC
TARG1 = kageserv
MOD1 = nickserv.so operserv.so chanserv.so
OFILES2 = control.o services.o md5.o datastorage.o
OFILES1 = kageserv.o server.o protocol.o parser.o netsimul.o simulation.o ${OFILES2}

all : cls ${TARG1} ${MOD1}

cls :
	clear

clean :
	rm -rf *.o

modclean :
	rm -rf *.so

again : clean modclean all

kageserv : ${OFILES1}
	${CC} ${CFLAGS} -o kageserv ${OFILES1} ${CLIBS}

kageserv.o : kageserv.cpp kageserv.h server.h parser.h
	${CC} ${CFLAGS} -c kageserv.cpp

server.o : server.cpp server.h
	${CC} ${CFLAGS} -c server.cpp

protocol.o : protocol.cpp protocol.h server.h
	${CC} ${CFLAGS} -c protocol.cpp

parser.o : parser.cpp parser.h protocol.h server.h
	${CC} ${CFLAGS} -c parser.cpp

netsimul.o : netsimul.cpp netsimul.h
	${CC} ${CFLAGS} -c netsimul.cpp

simulation.o : simulation.cpp simulation.h netsimul.h parser.h
	${CC} ${CFLAGS} -c simulation.cpp

services.o : parser.h server.h protocol.h services.h control.h services.cpp
	${CC} ${CFLAGS} -c services.cpp

control.o : parser.h server.h protocol.h control.h control.cpp
	${CC} ${CFLAGS} -c control.cpp

md5.o : md5.h md5.cpp
	${CC} -c md5.cpp

datastorage.o : datastorage.cpp datastorage.h
	${CC} ${CFLAGS} -c datastorage.cpp

# Modules
nickserv.so : ${OFILES1} services.h parser.h server.h protocol.h nickserv.h nickserv.cpp
	${MC} ${MFLAGS} -o nickserv.so ${OFILES1} nickserv.cpp

operserv.so : ${OFILES1} services.h parser.h server.h protocol.h operserv.h operserv.cpp
	${MC} ${MFLAGS} -o operserv.so ${OFILES1} operserv.cpp

chanserv.so : ${OFILES1} services.h parser.h server.h protocol.h chanserv.h chanserv.cpp
	${MC} ${MFLAGS} -o chanserv.so ${OFILES1} chanserv.cpp
