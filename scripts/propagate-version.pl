#!perl

# Enable in-place editing
BEGIN
{
    $^I = '.bak';
}

use strict;
use feature 'unicode_strings';

# parse line from include/log4cplus/version.h

open (my $fh, "<", "include/log4cplus/version.h")
    or die $!;

my ($major, $minor, $point, $version);
while (my $line = <$fh>)
{
    if ($line =~ m/\s* # \s* define \s+ LOG4CPLUS_VERSION \s+
          LOG4CPLUS_MAKE_VERSION \s* \(
          \s* (\d+) \s* , \s* (\d+) \s* , \s* (\d+) \s* \)/x)
    {
        ($major, $minor, $point) = ($1, $2, $3);
        $version = "$major.$minor.$point";
        print "version: ", $version, "\n";
        last;
    }
}

# edit configure.ac

{
    local $^I = ".bak";
    local @ARGV = ("configure.ac");
    while (my $line = <>)
    {
        $line =~ s/(.*AC_INIT\(.*\[)(\d+(?:\.\d+(?:\.\d+)?)?)(\].*)/$1$version$3/x;
        $line =~ s/(.*LT_RELEASE=)(.*)/$1$major.$minor/x;
        print $line;
    }

    local @ARGV = ("docs/doxygen.config");
    while (my $line = <>)
    {
        $line =~ s/(\s* PROJECT_NUMBER \s* = \s*)(.*)/$1$version/x;
        $line =~ s/(\s* OUTPUT_DIRECTORY \s* = \s*)(.*)/$1log4cplus-$version\/docs/x;
        print $line;
    }

    local @ARGV = ("docs/webpage_doxygen.config");
    while (my $line = <>)
    {
        $line =~ s/(\s* PROJECT_NUMBER \s* = \s*)(.*)/$1$version/x;
        $line =~ s/(\s* OUTPUT_DIRECTORY \s* = \s*)(.*)/$1webpage_docs-$version/x;
        print $line;
    }

    local @ARGV = ("log4cplus.spec");
    while (my $line = <>)
    {
        $line =~ s/(Version: \s*)(.*)/$1$version/x;
        $line =~ s/(Release: \s*)(.*)/${1}1/x;
        print $line;
    }
}

my @cygport = <cygport/*.cygport>;
die "cannot cope with multiple Cygport files" if (@cygport > 1);
my $cygport = @cygport[0];
my $new_cygport = "cygport/log4cplus-$version-1.cygport";
if ($cygport ne $new_cygport)
{
    system("bzr mv \"$cygport\" \"$new_cygport\"") == 0
        or die "bzr mv on $cygport has failed: $?";
}

