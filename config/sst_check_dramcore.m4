
AC_DEFUN([SST_CHECK_DRAMCORE], [
  AC_ARG_WITH([dramcore],
    [AS_HELP_STRING([--with-dramcore@<:@=DIR@:>@],
      [Use DUMMYSim package installed in optionally specified DIR])])

  sst_check_dramcore_happy="yes"
  AS_IF([test "$with_dramcore" = "no"], [sst_check_dramcore_happy="no"])

  CPPFLAGS_saved="$CPPFLAGS"
  LDFLAGS_saved="$LDFLAGS"
  LIBS_saved="$LIBS"

  AS_IF([test ! -z "$with_dramcore" -a "$with_dramcore" != "yes"],
    [DRAMCORE_CPPFLAGS="-I$with_dramcore/src -DDRAMCORE -DHAVE_DRAMCORE"
     CPPFLAGS="$DRAMCORE_CPPFLAGS $CPPFLAGS"
     DRAMCORE_LDFLAGS="-L$with_dramcore"
     DRAMCORE_LIBDIR="$with_dramcore"
     LDFLAGS="$DRAMCORE_LDFLAGS $LDFLAGS"],
    [DRAMCORE_CPPFLAGS=
     DRAMCORE_LDFLAGS=
     DRAMCORE_LIBDIR=])

  AC_LANG_PUSH(C++)
  AC_CHECK_HEADERS([memory_system.h hmc.h], [], [sst_check_dramcore_happy="no"])
  AC_CHECK_LIB([dramcore], [libdramcore_is_present],
    [DRAMCORE_LIB="-ldramcore"], [sst_check_dramcore_happy="no"])
  AC_LANG_POP(C++)

  CPPFLAGS="$CPPFLAGS_saved"
  LDFLAGS="$LDFLAGS_saved"
  LIBS="$LIBS_saved"

  AC_SUBST([DRAMCORE_CPPFLAGS])
  AC_SUBST([DRAMCORE_LDFLAGS])
  AC_SUBST([DRAMCORE_LIB])
  AC_SUBST([DRAMCORE_LIBDIR])
  AM_CONDITIONAL([HAVE_DRAMCORE], [test "$sst_check_dramcore_happy" = "yes"])
  AS_IF([test "$sst_check_dramcore_happy" = "yes"],
        [AC_DEFINE([HAVE_DRAMCORE], [1], [Set to 1 if DUMMYSim was found])])
  AC_DEFINE_UNQUOTED([DRAMCORE_LIBDIR], ["$DRAMCORE_LIBDIR"], [Path to dummy library])

  AS_IF([test "$sst_check_dramcore_happy" = "yes"], [$1], [$2])
])
