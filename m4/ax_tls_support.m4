dnl Check for the __thread/__declspec(thread) construct support.

AC_DEFUN([AX_TLS_SUPPORT],
[
AH_TEMPLATE(HAVE_TLS_SUPPORT,
  [Defined if the compiler understands __thread or __declspec(thread)
   construct.])
AH_TEMPLATE(TLS_SUPPORT_CONSTRUCT, 
  [Defined to the actual TLS support construct.])

AC_CACHE_CHECK([for __declspec(thread)], [ac_cv_declspec_thread],
[
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
      [[
#if defined (__GNUC__)
#  error Please fail.
And extra please fail.
#else
        extern __declspec(thread) int x;
        __declspec(thread) int * ptr = 0;
        int foo () { ptr = &x; return x; }
        __declspec(thread) int x = 1;
#endif
      ]], 
      [[]])],
    [ac_cv_declspec_thread=yes],
    [ac_cv_declspec_thread=no])
])
  
AS_IF([test "x$ac_cv_declspec_thread" = "xyes"],
  [AC_DEFINE(HAVE_TLS_SUPPORT, [1])
   AC_DEFINE(TLS_SUPPORT_CONSTRUCT, [__declspec(thread)])],

[
AC_CACHE_CHECK([for __thread], [ac_cv__thread_keyword],
[
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
      [[extern __thread int x;
        __thread int * ptr = 0;
        int foo () { ptr = &x; return x; }
        __thread int x = 1;
      ]],
      [[]])],
    [ac_cv__thread_keyword=yes],
    [ac_cv__thread_keyword=no])
])

AS_IF([test "x$ac_cv__thread_keyword" = "xyes"],
  [AC_DEFINE(HAVE_TLS_SUPPORT, [1])
   AC_DEFINE(TLS_SUPPORT_CONSTRUCT, [__thread])])
])

])
