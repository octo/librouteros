AC_INIT(librouteros, 1.1.2)
AC_CONFIG_SRCDIR(src/main.c)
AC_CONFIG_HEADERS(src/config.h)
AM_INIT_AUTOMAKE(dist-bzip2)
AC_LANG(C)

AC_PREFIX_DEFAULT("/opt/librouteros")

# API version
LIBROUTEROS_MAJOR=`echo $PACKAGE_VERSION | cut -d'.' -f1`
LIBROUTEROS_MINOR=`echo $PACKAGE_VERSION | cut -d'.' -f2`
LIBROUTEROS_PATCH=`echo $PACKAGE_VERSION | cut -d'.' -f3`
AC_SUBST(LIBROUTEROS_MAJOR)
AC_SUBST(LIBROUTEROS_MINOR)
AC_SUBST(LIBROUTEROS_PATCH)

# ABI version
LIBROUTEROS_CURRENT=2
LIBROUTEROS_REVISION=2
LIBROUTEROS_AGE=2
AC_SUBST(LIBROUTEROS_CURRENT)
AC_SUBST(LIBROUTEROS_REVISION)
AC_SUBST(LIBROUTEROS_AGE)

#
# Check for programs/utilities
#
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AM_CONDITIONAL(COMPILER_IS_GCC, test "x$GCC" = "xyes")

#
# configure libtool
#
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

#
# Checks for header files.
#
AC_HEADER_STDC
AC_CHECK_HEADERS(sys/types.h)
AC_CHECK_HEADERS(sys/socket.h)
AC_CHECK_HEADERS(netdb.h)

#
# Check for libgcrypt
#
AC_CHECK_HEADERS(gcrypt.h, [], [AC_MSG_ERROR("gcrypt.h is required for librouteros to compile")])

socket_needs_socket="no"
AC_CHECK_FUNCS(socket, [],
	AC_CHECK_LIB(socket, socket,
		[socket_needs_socket="yes"],
		AC_MSG_ERROR(cannot find socket)))
AM_CONDITIONAL(BUILD_WITH_LIBSOCKET, test "x$socket_needs_socket" = "xyes")

AC_ARG_ENABLE(debug, [AS_HELP_STRING([--enable-debug], [Enable extensive debugging output.])],
[
	if test "x$enable_debug" = "xyes"
	then
		AC_DEFINE(WITH_DEBUG, 1, [Define to 1 if you want to get debugging output.])
	fi
], [])
AM_CONDITIONAL(BUILD_WITH_DEBUG, test "x$enable_debug" = "xyes")

AC_OUTPUT(Makefile doc/Makefile src/Makefile src/routeros_version.h)
