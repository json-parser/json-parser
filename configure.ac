
AC_PREREQ([2.49])

AC_INIT([libjsonparser], [1.1.0], [])
VERSION_MAJOR="1.1"

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

AC_ARG_ENABLE([track-source],
	[AS_HELP_STRING([--enable-track-source],
		 [Lets the json-parser user retrieve the source line/column number \
		 of a parsed json_value. @<:@default=disabled@:>@])],
	[enable_value_pos="$enableval"],
	[enable_value_pos=no]
)
if test "x$enable_value_pos" = xyes; then
	CFLAGS="$CFLAGS -DJSON_TRACK_SOURCE"
fi

AC_SUBST(VERSION_MAJOR, $VERSION_MAJOR)

AC_CONFIG_FILES([
                 Makefile
                 json-parser.pc])

AC_OUTPUT
