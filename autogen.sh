#!/bin/sh

libtoolize --force --automake

autoheader
aclocal $ACLOCAL_FLAGS
autoconf
automake   --add-missing

echo 'run "configure; make"'

