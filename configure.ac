#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# $Id: configure.ac,v 1.3 2004/05/25 00:00:53 nenolod Exp $

AC_PREREQ(2.59)
AC_INIT("NetworkBOPM", "1.0-alpha1", "nenolod@nenolod.net")
AC_CONFIG_SRCDIR([src/socket.c])
AC_CONFIG_HEADER([include/sysconf.h])

# Checks for programs.
AC_PROG_CC
AC_PATH_PROG(RM, rm)
AC_PATH_PROG(MV, mv)
AC_PATH_PROG(CP, cp)
AC_PATH_PROG(AR, ar)
AC_C_INLINE
AC_PROG_MAKE_SET
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])
AC_CHECK_HEADERS(sys/poll.h, have_poll_sys_h=yes, have_sys_poll_h=no)

if test "$have_sys_poll_h" = "no"; then
  AC_CHECK_FUNCS(select, have_select=yes, have_select=no)
  if test "$have_select" = "no"; then
    AC_MSG_ERROR([No select() implementation found])
  fi
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([snprintf])
AC_CHECK_FUNCS([gethostbyname gettimeofday memset socket strcasecmp strchr strdup])

AC_CONFIG_FILES([Makefile libopm/Makefile src/Makefile])
AC_OUTPUT
