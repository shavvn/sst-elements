
AC_DEFUN([SST_CONFIG_ELEMENTS], [
	m4_foreach_w(current_element, sst_elements,
		[SST_CONFIG_ELEMENT([current_element])] )
])

AC_DEFUN([SST_CONFIG_ELEMENT], [
	AC_MSG_NOTICE([configuring element $1])

	m4_ifdef([SST_]$1[_CONFIG],
		[SST_$1_CONFIG([build_elem=1], [build_elem=0])], [build_elem=1])

	AC_MSG_CHECKING([if $1 should be built])

	AS_IF([test "$build_elem} -eq 1], [AC_MSG_RESULT([yes])], [AC_MSG_RESULT([no])])

	AC_CONFIG_FILES([src/sst/elements/]$1[/Makefile])
])
