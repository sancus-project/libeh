#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(libeh, 0.0, amery@geeks.cl)
AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign dist-bzip2 -Wall -Werror])

AC_CONFIG_MACRO_DIR([m4])

LT_INIT

# --enable-debug
#
AC_MSG_CHECKING([whether to build with debug information])
AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug], [enable debug data generation (def=almost)])],
	[debugit="$enableval"],
	[debugit=almost])
AC_MSG_RESULT([$debugit])

# CFLAGS
if test x"$debugit" = x"yes"; then
	AC_DEFINE([DEBUG],[],[Debug Mode])
	CFLAGS="-g -O0"
elif test x"$debugit" = x"no"; then
	AC_DEFINE([NDEBUG],[],[No-debug Mode])
	CFLAGS="-O3"
fi
CFLAGS="$CFLAGS -Wall -Wextra -pedantic -Winline"

# Checks for programs.
AC_PROG_CC_C99

# Checks for libraries.
PKG_CHECK_MODULES(libev, [libev])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_SRCDIR([src/eh.h])
AC_CONFIG_FILES([eh.pc])
AC_CONFIG_FILES([Makefile
		 doc/Doxyfile
		 doc/Makefile
		 src/Makefile])
AC_OUTPUT
