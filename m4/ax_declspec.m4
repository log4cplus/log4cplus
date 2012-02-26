dnl Check for the __declspec(dllexport) construct support.

AC_DEFUN([AX_DECLSPEC],
[
AH_TEMPLATE($1_IMPORT,
  [Defined if the compiler understands __declspec(dllexport)
   or construct.])
AH_TEMPLATE($1_EXPORT, 
  [Defined if the compiler understands __declspec(dllimport)
   or __attribute__((visibility("default"))) construct.])

AC_CACHE_CHECK([for __declspec(dllexport)], [ac_cv_declspec],
[
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
      [[
#if defined (__clang__)
// Here the problem is that Clang only warns that it does not support
// __declspec(dllexport) but still compiles the executable.
#  error Please fail.
And extra please fail.
#else
        __declspec(dllexport) int x = 0;
        __declspec(dllexport) int foo ();
        int foo () { return 0; }
        __declspec(dllexport) int bar () { return x; }
#endif
      ]], 
      [[]])],
    [ac_cv_declspec=yes],
    [ac_cv_declspec=no])
])
  
AS_IF([test "x$ac_cv_declspec" = "xyes"],
  [AC_DEFINE($1_IMPORT, [__declspec(dllimport)])
   AC_DEFINE($1_EXPORT, [__declspec(dllexport)])],

[
AC_CACHE_CHECK([for __attribute__((visibility("default")))], [ac_cv__attribute__visibility],
[
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
      [[
#if defined (__GNUC__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ <= 1))
#  error Please fail.
And extra please fail.
#else
        __attribute__((visibility("default"))) int x = 0;
        __attribute__((visibility("default"))) int foo ();
        int foo () { return 0; }
        __attribute__((visibility("default"))) int bar () { return x; }
        __attribute__((visibility("hidden"))) int baz () { return 1; }
#endif
      ]],
      [[]])],
    [ac_cv__attribute__visibility=yes],
    [ac_cv__attribute__visibility=no])
])

AS_IF([test "x$ac_cv__attribute__visibility" = "xyes"],
  [AC_DEFINE($1_IMPORT, [])
   AC_DEFINE($1_EXPORT, [__attribute__ ((visibility("default")))])])
])

AS_IF([test "x$ac_cv__attribute__visibility" = "xno" && test "x$ac_cv_declspec" = "xno"],
  [AC_DEFINE($1_IMPORT, [])
   AC_DEFINE($1_EXPORT, [])])

])
