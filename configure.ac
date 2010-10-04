#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(libeh, 0.0, amery@geeks.cl)
AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign dist-bzip2 -Wall -Werror])

AC_CONFIG_MACRO_DIR([m4])

LT_INIT

# Checks for programs.
AC_PROG_CC_C99

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
CWARNFLAGS="-Wall -Wextra -pedantic"
AC_SUBST(CWARNFLAGS)

# Checks for library functions.
AC_CHECK_LIB(ev, ev_loop)

AC_CONFIG_SRCDIR([src/eh.h])
AC_CONFIG_FILES([eh.pc])
AC_CONFIG_FILES([Makefile
		 doc/Makefile
		 src/Makefile])
AC_OUTPUT
