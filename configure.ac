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

AC_ARG_ENABLE([run-time-cpp], [AC_HELP_STRING([--disable-run-time-cpp], [don't pipe input files through C preprocessor])])
AS_IF([test "x$enable_run_time_cpp" != xno], [
	AC_ARG_WITH([mcpp],
		[AS_HELP_STRING([--without-mcpp], [disable support for mcpp])],
		[],
		[with_mcpp=yes]
	)
	AS_IF([test "x$with_mcpp" != xno], [AC_CHECK_LIB([mcpp], [mcpp_lib_main])])
	AC_ARG_VAR([RTCPP], [run-time C preprocessor command])
	AC_CHECK_PROG([RTCPP], [cpp], [cpp])
	AS_IF([test "x$RTCPP" != xno], [
		AC_DEFINE_UNQUOTED([RTCPP], ["$RTCPP"],
			[Define to the run-time C preprocessor command]
		)
	])
])

AC_OUTPUT(Makefile)
