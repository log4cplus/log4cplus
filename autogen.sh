#!/bin/sh

libtoolize --force --automake

autoheader
aclocal $ACLOCAL_FLAGS
autoconf
automake --include-deps --add-missing

echo 'run "configure; make"'

