AC_INIT(scis, 0.3.0-git, walter@scummvm.org)
AC_CONFIG_SRCDIR(.)

AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_LEX
if test "x$LEX" != xflex; then
	LEX="$SHELL $missing_dir/missing flex"
	AC_SUBST([LEX_OUTPUT_ROOT], [lex.yy])
	AC_SUBST([LEXLIB], [''])
fi

AC_CHECK_FUNCS([fork pipe dup2 _exit])

AC_ARG_WITH([mcpp],
	[AS_HELP_STRING([--without-mcpp], [disable support for mcpp])],
	[],
	[with_mcpp=yes])

AS_IF([test "x$with_mcpp" != xno],
	AC_CHECK_LIB([mcpp], [mcpp_lib_main]))

AC_ARG_ENABLE([external-cpp], [AC_HELP_STRING([--disable-external-cpp], [don't pipe input files through external C preprocessor])])
AS_IF([test "x$enable_external_cpp" != xno],
	[AC_ARG_VAR([EXTCPP], [external C preprocessor command])
	AC_CHECK_PROG([EXTCPP], [cpp], [cpp])
	AS_IF([test "x$EXTCPP" != x],
		[AC_DEFINE_UNQUOTED([EXTCPP], ["$EXTCPP"],
		  [Define to the external C preprocessor command])])])

AC_OUTPUT(Makefile)
