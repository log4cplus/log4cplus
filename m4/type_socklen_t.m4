dnl Check for the existance of type socklen_t.

AC_DEFUN([TYPE_SOCKLEN_T],
[
AH_TEMPLATE([socklen_t], [Define to int if undefined.])
AC_CACHE_CHECK([for socklen_t], ac_cv_type_socklen_t,
[
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
   #include <sys/socket.h>]], [[socklen_t len = 42; return 0;]])],[ac_cv_type_socklen_t=yes],[ac_cv_type_socklen_t=no])
])
  if test $ac_cv_type_socklen_t != yes; then
    AC_DEFINE(socklen_t, int)
  fi
])
