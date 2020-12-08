# ===========================================================================
#   https://www.gnu.org/software/autoconf-archive/ax_cflags_sun_option.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CFLAGS_SUN_OPTION (optionflag [,[shellvar][,[A][,[NA]]])
#
# DESCRIPTION
#
#   AX_CFLAGS_SUN_OPTION(+xtreme) would show a message as like "checking
#   CFLAGS for sun/cc +xtreme ... yes" and adds the optionflag to CFLAGS if
#   it is understood. You can override the shellvar-default of CFLAGS of
#   course. The order of arguments stems from the explicit macros like
#   AX_CFLAGS_WARN_ALL.
#
#   The cousin AX_CXXFLAGS_SUN_OPTION would check for an option to add to
#   CXXFLAGS - and it uses the autoconf setup for C++ instead of C (since it
#   is possible to use different compilers for C and C++).
#
#   The macro is a lot simpler than any special AX_CFLAGS_* macro (or
#   ax_cxx_rtti.m4 macro) but allows to check for arbitrary options.
#   However, if you use this macro in a few places, it would be great if you
#   would make up a new function-macro and submit it to the ac-archive.
#
#    - $1 option-to-check-for : required ("-option" as non-value)
#    - $2 shell-variable-to-add-to : CFLAGS (or CXXFLAGS in the other case)
#    - $3 action-if-found : add value to shellvariable
#    - $4 action-if-not-found : nothing
#
#   note: in earlier versions, $1-$2 were swapped. We try to detect the
#   situation and accept a $2=~/-/ as being the old option-to-check-for.
#
#   see also: AX_CFLAGS_GCC_OPTION for the widely used original variant.
#
# LICENSE
#
#   Copyright (c) 2008 Guido U. Draheim <guidod@gmx.de>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.  This file is offered as-is, without any
#   warranty.

#serial 16

AC_DEFUN([AX_FLAGS_SUN_OPTION_PRIVATE], [dnl
AX_CHECK_COMPILE_FLAG([$1], [flag_ok="yes"], [flag_ok="no"], [+xstrconst -Xc])
AS_CASE([".$flag_ok"],
  [.ok|.ok,*], [$3],
  [.|.no|.no,*], [$4],
  [m4_default($3,[AX_APPEND_FLAG([$1],[$2])])])
])

AC_DEFUN([AX_CFLAGS_SUN_OPTION],[
  AC_LANG_PUSH([C])
  AX_FLAGS_SUN_OPTION_PRIVATE(ifelse(m4_bregexp([$2],[-]),-1,[[$1],[$2]],[[$2],[$1]]),[$3],[$4])
  AC_LANG_POP
])

AC_DEFUN([AX_CXXFLAGS_SUN_OPTION],[
  AC_LANG_PUSH([C++])
  AX_FLAGS_SUN_OPTION_PRIVATE(ifelse(m4_bregexp([$2],[-]),-1,[[$1],[$2]],[[$2],[$1]]),[$3],[$4])
  AC_LANG_POP
])
