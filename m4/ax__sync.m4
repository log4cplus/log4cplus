dnl Check for the __sync_add_and_fetch() and __sync_sub_and_fetch()
dnl compiler built-ins.

AC_DEFUN([AX__SYNC],
[
AH_TEMPLATE([HAVE___SYNC_ADD_AND_FETCH],
  [Defined if the compiler provides __sync_add_and_fetch().])

AC_CACHE_CHECK([for __sync_add_and_fetch], [ac_cv___sync_add_and_fetch],
[
  AC_RUN_IFELSE(
    [AC_LANG_PROGRAM(
      [[
#include <stdlib.h>
      ]], 
      [[
volatile int x = 1;
volatile int y = __sync_add_and_fetch (&x, 1);
if (x != 2 || y != 2)
  abort ();
      ]])],
    [ac_cv___sync_add_and_fetch=yes],
    [ac_cv___sync_add_and_fetch=no])
])

AS_IF([test "x$ac_cv___sync_add_and_fetch" = "xyes"],
  [AC_DEFINE([HAVE___SYNC_ADD_AND_FETCH], [1])])

AH_TEMPLATE([HAVE___SYNC_SUB_AND_FETCH], 
  [Defined if the compiler provides __sync_sub_and_fetch().])

AC_CACHE_CHECK([for __sync_sub_and_fetch], [ac_cv___sync_sub_and_fetch],
[
  AC_RUN_IFELSE(
    [AC_LANG_PROGRAM(
      [[
#include <stdlib.h>
      ]], 
      [[
volatile int x = 2;
volatile int y = __sync_sub_and_fetch (&x, 1);
if (x != 1 || y != 1)
  abort ();
      ]])],
    [ac_cv___sync_sub_and_fetch=yes],
    [ac_cv___sync_sub_and_fetch=no])
])

AS_IF([test "x$ac_cv___sync_sub_and_fetch" = "xyes"],
  [AC_DEFINE([HAVE___SYNC_SUB_AND_FETCH], [1])])


])
