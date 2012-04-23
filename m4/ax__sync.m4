dnl Check for the __sync_add_and_fetch() and __sync_sub_and_fetch()
dnl compiler built-ins.

AC_DEFUN([AX__SYNC],
[
AH_TEMPLATE([HAVE___SYNC_ADD_AND_FETCH],
  [Defined if the compiler provides __sync_add_and_fetch().])

AC_CACHE_CHECK([for __sync_add_and_fetch], [ac_cv___sync_add_and_fetch],
[
  AC_LINK_IFELSE(
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
  AC_LINK_IFELSE(
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

AH_TEMPLATE([HAVE_CXX11_ATOMICS], [Defined if the compiler provides
  C++11 <atomic> header and increment, decrement operations.])

AC_CACHE_CHECK([for C++11 atomics], [ac_cv_cxx11_atomics],
[
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
      [[
#include <atomic>
      ]],
      [[
#define TEST_ATOMIC(type) do { \
std::atomic<type> x (0); \
std::atomic_fetch_add_explicit (&x, static_cast<type>(1), std::memory_order_acquire); \
std::atomic_fetch_sub_explicit (&x, static_cast<type>(1), std::memory_order_release); \
} while (0)

TEST_ATOMIC(int);
TEST_ATOMIC(unsigned int);
TEST_ATOMIC(long);
TEST_ATOMIC(unsigned long);
std::atomic_thread_fence (std::memory_order_acquire);
      ]])],
    [ac_cv_cxx11_atomics=yes],
    [ac_cv_cxx11_atomics=no])
])

AS_IF([test "x$ac_cv_cxx11_atomics" = "xyes"],
  [AC_DEFINE([HAVE_CXX11_ATOMICS], [1])])

])

