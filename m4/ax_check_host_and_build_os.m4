dnl | HOST
dnl | the type of system on which the package runs. By default it is the same as
dnl | the build machine. Specifying a host-type that differs from build-type,
dnl | when build-type was also explicitly specified, enables cross-compilation
dnl | mode.
dnl |
dnl | BUILD
dnl | the type of system on which the package is being configured and compiled.
dnl | It defaults to the result of running config.guess. Specifying a build-type
dnl | that differs from host-type enables cross-compilation mode.
dnl |
dnl | ref. https://www.gnu.org/software/autoconf/manual/autoconf-2.69/html_node/Specifying-Target-Triplets.html#Specifying-Target-Triplets

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

os_win32=no
os_linux=no
os_freebsd=no
os_gnu=no
os_iphone=no

case "$host" in
  *-mingw*|*-*-cygwin*)
      os_win32=yes
      TARGET_OS=windows
      ;;
  *-*-*netbsd*)
      os_netbsd=yes
      TARGET_OS=unix
      ;;
  *-*-*freebsd*)
      os_freebsd=yes
      TARGET_OS=unix
      ;;
  *-*-*openbsd*)
      os_openbsd=yes
      TARGET_OS=unix
      ;;
  *-*-linux*)
      os_linux=yes
      os_gnu=yes
      TARGET_OS=unix
      ;;
  *-*-solaris*)
      os_solaris=yes
      TARGET_OS=unix
      ;;
  arm*-*-darwin*)
      os_darwin=yes
      os_iphone=yes
      TARGET_OS=unix
      ;;
  *-*-darwin*)
      os_darwin=yes
      TARGET_OS=unix
      ;;
  gnu*|k*bsd*-gnu*)
      os_gnu=yes
      TARGET_OS=unix
      ;;
  *)
      AC_MSG_WARN([*** Please add $host to configure.ac checks!])
      ;;
esac


dnl | you can use enable_crosscompile to check that we are cross compiling
enable_crosscompile=no
if test "x$host" != "x$build"; then
  enable_crosscompile=yes
fi

dnl | Now, based on above values you can make decisions, e.g.
dnl | define constants in config.h
dnl |
dnl | if test "$TARGET_OS" = "unix"; then
dnl |   AC_SUBST(POSIX_OS, 1)
dnl |   AC_DEFINE_UNQUOTED([POSIX_OS], [1], [Whether OS is POSIX compliant])
dnl | fi
dnl |
dnl | if test "$os_iphone" = "yes"; then
dnl |   AC_SUBST(IPHONE_OS, 1)
dnl |   AC_DEFINE_UNQUOTED([IPHONE_OS], [1], [Whether OS is iOS])
dnl | fi