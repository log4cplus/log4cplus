dnl Check for the __sync_add_and_fetch() and __sync_sub_and_fetch()
dnl compiler built-ins.

AC_DEFUN([AX__SYNC],
[

dnl
dnl Check for __sync_add_and_fetch().
dnl

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

dnl
dnl Check for __sync_sub_and_fetch().
dnl

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

dnl
dnl Check for C++11 atomics.
dnl

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

dnl
dnl Check for __atomic_add_fetch().
dnl

AH_TEMPLATE([HAVE___ATOMIC_ADD_FETCH],
  [Defined if the compiler provides __atomic_add_fetch().])

AC_CACHE_CHECK([for __atomic_add_fetch], [ac_cv___atomic_add_fetch],
[
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
      [[
#include <stdlib.h>
      ]], 
      [[
volatile int x = 1;
volatile int y = __atomic_add_fetch (&x, 1, __ATOMIC_RELAXED);
if (x != 2 || y != 2)
  abort ();
      ]])],
    [ac_cv___atomic_add_fetch=yes],
    [ac_cv___atomic_add_fetch=no])
])

AS_IF([test "x$ac_cv___atomic_add_fetch" = "xyes"],
  [AC_DEFINE([HAVE___ATOMIC_ADD_FETCH], [1])])

dnl
dnl Check for __atomic_sub_fetch().
dnl

AH_TEMPLATE([HAVE___ATOMIC_SUB_FETCH], 
  [Defined if the compiler provides __atomic_sub_fetch().])

AC_CACHE_CHECK([for __atomic_sub_fetch], [ac_cv___atomic_sub_fetch],
[
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
      [[
#include <stdlib.h>
      ]], 
      [[
volatile int x = 2;
volatile int y = __atomic_sub_fetch (&x, 1, __ATOMIC_RELEASE);
__atomic_thread_fence (__ATOMIC_ACQUIRE);
if (x != 1 || y != 1)
  abort ();
      ]])],
    [ac_cv___atomic_sub_fetch=yes],
    [ac_cv___atomic_sub_fetch=no])
])

AS_IF([test "x$ac_cv___atomic_sub_fetch" = "xyes"],
  [AC_DEFINE([HAVE___ATOMIC_SUB_FETCH], [1])])

dnl
dnl Check for Solaris 10 atomic.h provided functions.
dnl

AH_TEMPLATE([HAVE_ATOMIC_INC_UINT],
  [Defined if the compiler provides atomic_inc_uint().])

AC_CACHE_CHECK([for atomic_inc_uint], [ac_cv_atomic_inc_uint],
[
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
      [[
#include <stdlib.h>
#include <atomic.h>
      ]], 
      [[
volatile unsigned int x = 1;
atomic_inc_uint (&x);
if (x != 2)
  abort ();
      ]])],
    [ac_cv_atomic_inc_uint=yes],
    [ac_cv_atomic_inc_uint=no])
])

AS_IF([test "x$ac_cv_atomic_inc_uint" = "xyes"],
  [AC_DEFINE([HAVE_ATOMIC_INC_UINT], [1])])

dnl
dnl Check for atomic_dec_uint_nv().
dnl

AH_TEMPLATE([HAVE_ATOMIC_DEC_UINT_NV], 
  [Defined if the compiler provides atomic_dec_uint_nv().])

AC_CACHE_CHECK([for atomic_dec_uint_nv], [ac_cv_atomic_dec_uint_nv],
[
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
      [[
#include <stdlib.h>
#include <atomic.h>
      ]], 
      [[
volatile unsigned int x = 2;
volatile unsigned int y = atomic_dec_uint_nv (&x);
membar_exit();
if (x != 1 || y != 1)
  abort ();
      ]])],
    [ac_cv_atomic_dec_uint_nv=yes],
    [ac_cv_atomic_dec_uint_nv=no])
])

AS_IF([test "x$ac_cv_atomic_dec_uint_nv" = "xyes"],
  [AC_DEFINE([HAVE_ATOMIC_DEC_UINT_NV], [1])])

])
