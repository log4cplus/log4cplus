#! env perl
use strict;
use autodie qw(:all);
use File::Basename;
use File::Spec::Functions;
use File::Spec::Functions qw(rel2abs abs2rel);
use File::Glob qw(:globally);
use Data::Dumper;
use Data::UUID;

my @FILES =
    ( 'README.md'
      , 'docs/unicode.txt'
      , 'docs/release.txt'
      , 'AUTHORS'
      , 'LICENSE' );

my @PANDOC_1ST_STEP_SWITCHES =
    ( '--smart'
      , '--self-contained'
      , '--normalize'
      , '--atx-headers'
      , '-f', 'markdown'
      , '-t', 'markdown' );

my @PANDOC_2ND_STEP_SWITCHES =
    ( '--smart'
      , '--self-contained'
      , '--toc'
      , '--number-sections'
      # Qt4 / Win32 / MSVC has example that breaks listings on some versions of
      # TeXLive
      , '--listings'
      , '-f', 'markdown'
      , '-t', 'latex',
      , '--latex-engine=lualatex',
      , '--include-in-header=docs/latex-header.tex'
      , '--include-before-body=docs/latex-body.tex'
      , '-V', 'lang=english',
      , '-V', 'geometry:a4paper');

# pre-compute  various source information strings

my %sources = ();

foreach my $f (@FILES)
{
    my @parts = fileparse($f);
    my $rec = $sources{$f} = {};
    $rec->{file} = $parts[0];
    $rec->{abs_path} = rel2abs($f);
    $rec->{dir} = (fileparse($rec->{abs_path}))[1];
    $rec->{tmp_base} = $rec->{abs_path} . '.build-pdf';
    $rec->{md_tmp} = $rec->{tmp_base} . '.ref_rename_step';
    $rec->{cleanup_glob} = $rec->{abs_path} . '.*';
}

print Dumper(\%sources), "\n";

# unlink temporary files

foreach my $f (@FILES)
{
    my $rec = $sources{$f};
    my @junk = glob($rec->{cleanup_glob});
    print Dumper(\@junk), "\n";
    foreach my $file (@junk)
    {
        unlink $file;
    }
}

# pre-process Markdown to eliminate duplicate references

my $ug = new Data::UUID;

foreach my $f (@FILES)
{
    my $rec = $sources{$f};
    my $uuid = $ug->create();
    my $uuid_str = $ug->to_hexstring($uuid);
    my @args = (
        'pandoc', @PANDOC_1ST_STEP_SWITCHES,
        '--id-prefix=build-pdf:' . $uuid_str,
        '-o', $rec->{md_tmp},
        $rec->{abs_path});
    print "args: ", Dumper(@args), "\n";
    system(@args);
}

# produce LaTeX file containing contents of all documents

# collect all file parts
my @parts = ();
for my $f (@FILES)
{
    my $rec = $sources{$f};
    push @parts, abs2rel($rec->{md_tmp});
}

print Dumper(\@parts), "\n";

my @args = (
    'pandoc', @PANDOC_2ND_STEP_SWITCHES,
    '-o', 'README.md.tex',
    @parts);

print Dumper(\@args), "\n";

system(@args);


# produce PDF using latexmk utility to run the LuaLaTeX

@args = (
    'latexmk', '-gg', '-lualatex', 'README.md.tex');

print Dumper(\@args), "\n";

system(@args);
