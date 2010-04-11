dnl Two functions for checking for availability of C99 style
dnl __VA_ARGS__ and GNU style args... variadic macro support.

AC_DEFUN([AX_C99_VARIADIC_MACROS],
[
AH_TEMPLATE([HAS_C99_VARIADIC_MACROS],
  [Defined if the compiler supports C99 style variadic macros with
    __VA_ARGS__.])

AC_CACHE_CHECK([for C99 variadic macros], [ac_cv_c99_variadic_macros],
[
  AC_LANG_ASSERT([C++])

  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
      [[
#include <cstdio>
#include <cstdlib>
      ]],
      [[
        using namespace std;
#define MACRO(buf, ...) (sprintf (buf, "%d", __VA_ARGS__))
        char a[10];
        MACRO(a, 1);
        if (a[0] != '1')
          abort ();
        if (a[1] != 0)
          abort ();
      ]]
    )],
    [ac_cv_c99_variadic_macros=yes],
    [ac_cv_c99_variadic_macros=no])
])
])


AC_DEFUN([AX_GNU_VARIADIC_MACROS],
[
AH_TEMPLATE([HAS_GNU_VARIADIC_MACROS],
  [Defined if the compiler supports GNU style variadic macros.])

AC_CACHE_CHECK([for GNU style variadic macros], [ac_cv_gnu_variadic_macros],
[
  AC_LANG_ASSERT([C++])

  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
      [[
#include <cstdio>
#include <cstdlib>
      ]],
      [[
        using namespace std;
#define MACRO(buf, args...) (sprintf (buf, "%d", args))
        char a[10];
        MACRO(a, 1);
        if (a[0] != '1')
          abort ();
        if (a[1] != 0)
          abort ();
      ]]
    )],
    [ac_cv_gnu_variadic_macros=yes],
    [ac_cv_gnu_variadic_macros=no])
])
])
