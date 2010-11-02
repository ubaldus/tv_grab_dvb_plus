AC_PREREQ([2.63])
AC_INIT([tv_grab_dvb_plus], [0.2], [tv_grab_dvb_plus@teilifis.com])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_PROG_CXX
AC_PROG_RANLIB

AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/ioctl.h unistd.h])

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
		 libsi/Makefile
                 src/Makefile])
AC_CHECK_LIB(pthread, pthread_kill)
# jpeg cap

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([alarm dup2 memmove memset mkdir strcasecmp strerror strrchr])

AC_OUTPUT
