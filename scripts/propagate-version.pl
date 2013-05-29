#!perl

# Enable in-place editing
BEGIN
{
    $^I = '.bak';
}

use strict;

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
}
