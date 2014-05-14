
AC_PREREQ([2.49])

AC_INIT([libjsonparser], [1.1.0], [])
VERSION_MAJOR="1.1"

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_CHECK_TOOL(AR, ar, :)

AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug],
		 [Build a debug version of json-parser @<:@default=disabled@:>@])],
	[enable_debug="$enableval"],
	[enable_debug=no]
)

if test "x$enable_debug" = xyes; then
	CFLAGS="$CFLAGS -g -O0"
fi

AC_SUBST(VERSION_MAJOR, $VERSION_MAJOR)

AC_CONFIG_FILES([
                 Makefile
                 json-parser.pc])

AC_OUTPUT

