#!/bin/sh

export AUTOMAKE_SUFFIX=-1.12.6
export AUTOCONF_SUFFIX=-2.69
export LIBTOOL_SUFFIX=-2.4.2

export ACLOCAL="aclocal${AUTOMAKE_SUFFIX}"
export AUTOMAKE="automake${AUTOMAKE_SUFFIX}"

export AUTOCONF="autoconf${AUTOCONF_SUFFIX}"
export AUTOHEADER="autoheader${AUTOCONF_SUFFIX}"
export AUTOM4TE="autom4te${AUTOCONF_SUFFIX}"
export AUTORECONF="autoreconf${AUTOCONF_SUFFIX}"
export AUTOSCAN="autoscan${AUTOCONF_SUFFIX}"
export AUTOUPDATE="autoupdate${AUTOCONF_SUFFIX}"
export IFNAMES="ifnames${AUTOCONF_SUFFIX}"

export LIBTOOLIZE="libtoolize${LIBTOOL_SUFFIX}"
export LIBTOOL="libtool${LIBTOOL_SUFFIX}"

$AUTORECONF -f
