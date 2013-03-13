#!/bin/sh

FILES=`find . \( \( -not -path './objdir*' \) -and \( -name '*.h' -o -name '*.hxx' -o -name '*.cxx' \) \)`
perl -wn -i.bak -e '
use strict;

my $YEAR = 2013;

my $sep = qr/[\s,;]/;
if (/copyright/i)
{
  print STDERR;
}
if (/(copyright $sep+ (?:\(c\) $sep+)? (?:\d{4})) (\s* - \s*) (\d{4})/ixgp)
{
  #print STDERR "($1) ($2) ($3)\n";
  #print STDERR "${^PREMATCH}$1$2$YEAR${^POSTMATCH}";
  print "${^PREMATCH}$1$2$YEAR${^POSTMATCH}";
}
elsif (/(copyright $sep+ (?:\(c\) $sep+)? (\d{4}))/ixgp)
{
  #print STDERR "($1)\n";
  #print STDERR "${^PREMATCH}$1-$YEAR${^POSTMATCH}";
  print "${^PREMATCH}$1-$YEAR${^POSTMATCH}";
}
else
{
  print;
}
' $FILES
