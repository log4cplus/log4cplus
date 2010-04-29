dnl LOG4CPLUS_CHECK_HEADER([header], [preprocessor symbol to define])
AC_DEFUN([LOG4CPLUS_CHECK_HEADER],
  [AH_TEMPLATE([$2])
   AC_CHECK_HEADER([$1], [AC_DEFINE([$2])])])

dnl LOG4CPLUS_CHECK_FUNCS([function], [preprocessor symbol to define])
AC_DEFUN([LOG4CPLUS_CHECK_FUNCS],
  [AH_TEMPLATE([$2])
   AC_CHECK_FUNCS([$1], [AC_DEFINE([$2])])])
