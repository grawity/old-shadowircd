#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT("NetworkBOPM", "1.0-alpha1", "nenolod@nenolod.net")
AC_CONFIG_SRCDIR([src/socket.c])
AC_CONFIG_HEADER([include/sysconf.h])

# Checks for programs.
AC_PROG_CC
AC_PATH_PROG(RM, rm)
AC_PATH_PROG(MV, mv)
AC_PATH_PROG(CP, cp)
AC_C_INLINE
AC_PROG_MAKE_SET
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gethostbyname gettimeofday memset socket strcasecmp strchr strdup])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
