AC_INIT(scis, 0.3.0-git, walter@scummvm.org)
AC_CONFIG_SRCDIR(.)

AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AM_PROG_LEX

AC_ARG_ENABLE([external-cpp], [AC_HELP_STRING([--disable-external-cpp], [don't pipe input files through external C preprocessor])])
if test "x$enable_external_cpp" != "xno"; then
	AC_ARG_VAR([EXTCPP], [external C preprocessor command])
	AC_CHECK_PROG([EXTCPP], [cpp], [cpp])
	if test "x$EXTCPP" != x; then
		AC_DEFINE_UNQUOTED([EXTCPP], ["$EXTCPP"],
		  [Define to the external C preprocessor command])
	fi
fi
AC_CHECK_FUNCS([fork pipe dup2 _exit])

AC_OUTPUT(Makefile)
