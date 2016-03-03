
AC_DEFUN([SST_CORE_CHECK_INSTALL], [
	AC_ARG_WITH[sst-core],
	  [AS_HELP_STRING([--with-sst-core=@<:@=DIR@:>@],
	    [Use SST Discrete Event Core installed in DIR])])

  sst_core_check_happy="yes"
  sst_found_config="no"
  SST_CONFIG=""

  AS_IF([test "x$with_sst_core" = "xyes"], [AC_PATH_PROG([SST_CONFIG], [sst-config], [], [$PATH])],
	[AC_PATH_PROG([SST_CONFIG], [sst-config], [], [$PATH$PATH_SEPARATOR$with_sst_core/bin])]

  AC_MSG_CHECKING([for sst-config tool])
  AS_IF([test -x "$SST_CONFIG"],
	[AC_MSG_RESULT([found ($SST_CONFIG)])],
	[AC_MSG_ERROR([Unable to find sst-config in the PATH], [1])]

  SST_CPPFLAGS=`$SST_CONFIG --CPPFLAGS`
  SST_CXXFLAGS=`$SST_CONFIG --CXXFLAGS`
  SST_LDFLAGS=`$SST_CONFIG --LDFLAGS`
  SST_LIBS=`$SST_CONFIG --LIBS`
 
  BOOST_CPPFLAGS=`$SST_CONFIG --BOOST_CPPFLAGS`
  BOOST_LDFLAGS=`$SST_CONFIG --BOOST_LDFLAGS`
  BOOST_LIBS=`$SST_CONFIG --BOOST_LIBS`

  PYTHON_CPPFLAGS=`$SST_CONFIG --PYTHON_CPPFLAGS`
  PYTHON_LDFLAGS=`$SST_CONFIG --PYTHON_LDFLAGS`

  CPPFLAGS="$CPPFLAGS $SST_CPPFLAGS $BOOST_CPPFLAGS"
  CXXFLAGS="$CXXFLAGS $SST_CXXFLAGS"
  LDFLAGS="$LDFLAGS $SST_LDFLAGS $BOOST_LDFLAGS"
  LIBS="$SST_LIBS $LIBS"

])
