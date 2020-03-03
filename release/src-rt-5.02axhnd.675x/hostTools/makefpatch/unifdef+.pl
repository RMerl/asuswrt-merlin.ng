#!/usr/bin/perl
use strict;
use warnings;
use FindBin qw($Bin);
use lib "$Bin/../PerlLib";
use BRCM::UnifdefPlus;
use Carp;


my $versionString = "0.5";

my $usage = q{
Usage: 
  unifdef+ [options] file
  unifdef+ [options] -F file
  unifdef+ [options] [-I file] [-O file]
  unifdef+ --version

Options:
  -D name[=val]     declare name as defined.  All compiler directives related to name will be 
                    simplified as if name was defined as val.  If val is not specified, a value 
                    of 'y' will be assumed.  This option may be repeated multiple times.                    
  -U name           declare name as undefined.  All compiler directives related to name will be
                    simplified as if name was undefined.  This option may be repeated multiple
                    times.
  -I filename       Take input from filename
  -O filename       Send output to filename
  -F filename       Modify filename
  --simplifiedonly    Only output lines within a simplified #if/#elif/#else/#endif clause
  --dbg             run with debugging enabled
  --version         outputs the current version of unifdef+


Description:
  The unifdef+ utility is designed to simplify c and cpp files based on preprocessor conditionals.
It has the ability to expand and simplify preprocessor conditionals ( #if, #ifdef, #elif, #else,
#endif statements) based on the values passed in on the command line.  If a preprocessor conditional 
contains any of the macros specified on the command line, unifdef+ will substitute the known values 
and then proceed to simplify the expression.  If the condition resolves to either true or false, all 
unused code is removed from the output.  Otherwise, the condition is replaced with the simplified 
version in the output.

Exit Status:
  The exit status will be 0 if the input has not been modified, 1 if there are modifications, or
  2 if there is an error processing the input.

Example: 
  unifdef+ -U CC_FEATURE1 -D CC_VER=12 -D CC_FEATURE2=y somefile.c

  
};

sub printHelp {
    print $usage
}



$SIG{'__DIE__'} =
  sub { $! = 2; confess( "XXXX" . @_ ); };    # make sure that die() returns 2

sub die2 {
    my $v = shift;
    print STDERR "DIED: $v\n";
    exit(2);
}

my $wasModified = 0; # for now, using global because it's faster to implement...

my %defines   = ();
my %undefines = ();
my $dbg       = 0;
my $simplifiedonly = 0;

my $INFILE         = *STDIN;
my $OUTFILE        = *STDOUT;
my $inFileName     = "";
my $outFileName    = "";
my $tmpOutFileName = "";

# the following is a whitespace character sequence used to replace a \
# line ending.

my $val;

# Parse passed in arguments:
while ( my $arg = shift ) {
    if ( $arg =~ /^-D(.*)/i ) {
        my $define = $1;
        $define = shift if ( $define !~ /\S/ );
        if ( $define =~ m/(\w+)\=(\w+)/ ) {

            # handle -DMACRO=val
            $defines{$1} = $2;
        }
        elsif ( $define =~ /(\w+)/ ) {

            # handle -DMACRO like -DMACRO=y
            $defines{$1} = 'y';
        }
        else {
            die2("ERROR: could not parse parameters $arg");
        }
    }
    elsif ( $arg =~ /^-U(.*)/i ) {
        my $macro = $1;
        $macro = shift if ( $macro !~ /^\S/ );
        die2("ERROR: could not parse -U parameters") if ( $macro eq "" );
        $undefines{$macro} = "X";
    }
    elsif ( $arg =~ /^-F(.*)/i ) {
        my $val = $1;
        $val = shift if ( $val !~ /\S/ );
        die2("ERROR: could not parse parameter -f") unless $val;
        $inFileName     = $val;
        $outFileName    = $val;
        $tmpOutFileName = "." . $val . ".unifdef+.tmp";
    }
    elsif ( $arg =~ /^-I(.*)/i ) {
        my $val = $1;
        $val = shift if ( $val !~ /\S/ );
        die2("ERROR: could not parse parameter -i") unless $val;
        $inFileName = $val;
    }
    elsif ( $arg =~ /^-O(.*)/i ) {
        my $val = $1;
        $val = shift if ( $val !~ /^\S/ );
        die2("ERROR: could not parse parameter -o") unless $val;
        $outFileName = $val;
    }
    elsif ( $arg =~ /^(DEBUG|--dbg)/ ) {
        print $OUTFILE "Debugging on\n";
        $dbg = 1;
    }
    elsif ( $arg =~ /^--simplifiedonly/ ) {
        $simplifiedonly = 1;
    }
    elsif ( $arg =~ /^--version/ ) {
        print $OUTFILE "unifdef+ $versionString \n";
        print $OUTFILE "Copyright (C) 2013 Broadcom corporation\n";
        print $OUTFILE "There is NO warranty on this; not even for MERCHANTABILITY or FITNESS ";
        print $OUTFILE "FOR A PARTICULAR PURPOSE.\n";
        exit 0;
    }
    elsif ( $arg =~ /^--help/ ) {
        print $OUTFILE $usage;
        exit 0;
    }
    else {
        $inFileName = $arg;
    }
}

$tmpOutFileName = $outFileName . ".unifdef+.tmp"
  if ( $inFileName && ( $inFileName eq $outFileName ) );
die2 "could not find file $inFileName\n" if ( $inFileName && !-e $inFileName );

if ($inFileName) {
    open $INFILE, "<", $inFileName or die2 $!;
}

if ($tmpOutFileName) {
    open $OUTFILE, ">", $tmpOutFileName or die2 $!;
}
elsif ($outFileName) {
    open $OUTFILE, ">", $outFileName or die2 $!;
}

sub dbgArgs {
    my $macro;
    my $val;

    print $OUTFILE "DEFINES:\n";
    while ( ( $macro, $val ) = each(%defines) ) {
        print $OUTFILE "  $macro=$val\n";
    }

    print $OUTFILE "UNDEFINES:\n";
    while ( ( $macro, $val ) = each(%undefines) ) {
        print $OUTFILE "  $macro not set\n";
    }
    print $OUTFILE "\n";
    print $OUTFILE "infile=$inFileName;\n";
    print $OUTFILE "outfile=$outFileName;\n";
    print $OUTFILE "tmpOutFile=$tmpOutFileName;\n";
    print $OUTFILE "\n\n\n";
}

dbgArgs if $dbg;

# Main line of program here:
my $unif = new BRCM::UnifdefPlus(
    defines   => \%defines,
    undefines => \%undefines,
    dbg       => $dbg,
    simplifiedonly => $simplifiedonly
);
$wasModified = $unif->parse(
    INFILE  => $INFILE,
    OUTFILE => $OUTFILE,
);
close $OUTFILE if ($outFileName);
close $INFILE  if ($inFileName);

#wasModified may be 2, indicating error was detected
#`cp -f $tmpOutFileName $outFileName;` if ($tmpOutFileName);
if ($wasModified == 1) {
    `cp -f $tmpOutFileName $outFileName; rm -f $tmpOutFileName`
      if ($tmpOutFileName);
    exit $wasModified;
}
else {
    `rm -f $tmpOutFileName` if ($tmpOutFileName);
    exit $wasModified;
}

