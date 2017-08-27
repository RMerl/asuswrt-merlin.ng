#!/usr/bin/perl -w
#
# Filename: tindent.pl
# - Translates Pid to name.
# - Indent a bounce trace file containing function entry and exits.
#   If there are # no function call exits then indentation is NOT possible.
#
# Usage: tindent.pl -i trace.log.bnc -s 2
#
# use File::Basename
# ($filename,$dir,$suffix) = fileparse($path);
#
#-------------------------------------------------------------------------------


my $version = "BOUNCE TASK INDENT 1.0";
use strict;
use warnings;

use vars qw/%opt %pid2name $pid %indents $spaces $save $logfile $outfile/;
$spaces = 2;
$logfile = '';
$save = 0;

use English;
use vars qw/$Ce $Cb $Cn/;
if (($OSNAME eq 'linux') || ($OSNAME eq 'uinx')) {
	$Ce = "\e[0;31m"; $Cb = "\e[0;36;44m"; $Cn = "\e[0m";
} else {
	$Ce = ''; $Cb = ''; $Cn = '';
}

#-------------------------------------------------------------------------------
# Display the command line usage
sub usage()
{
	print << "EOF";

    Usage: $0
     -h                  : This (help) message
     -i input_logfile    : Input log file to indent
     -t tab_spaces       : Spaces per tab indent
     -s                  : Save indented file (overrites original)

EOF
	exit;
}

#-------------------------------------------------------------------------------
# Fetch command line options
sub init_options()
{
	use Getopt::Std;
	my $opt_string = 'i:t:shv';
	getopts( "$opt_string", \%opt ) or usage();
	usage() if $opt{h};
	if ($opt{v}) {
		print "\n\n\t$0 version: $Cb $version $Cn\n";
		usage();
	}
	$logfile = $opt{i} if $opt{i};
	$spaces = int($opt{t}) if $opt{t};
	$save = 1 if $opt{s};
}

#-------------------------------------------------------------------------------
# Scan list of pids from /bin/ps output until "BOUNCE PROCESS END"
sub scan_pid2name
{
	while (<LOGFILE>) {
		return if /^BOUNCE PROCESS END/;
		chop $_;
		if (/^\s*(\d+) \w+\s+\d*\s*\w+<*\s+\[*-*(\w+)/) {
			$pid2name{$1} = $2;
			$pid2name{$1} .= "$1" if $2 eq "sh";
			$indents{$1} = 0;
		}
	}
}

#-------------------------------------------------------------------------------
# Translate pid to task name using scanned pid2name, no indentation.
sub do_pid2name
{
	while (<LOGFILE>) {
		if (/^BOUNCE DUMP END.*/) { print OUTFILE "$_"; last; }
		if (/^=>\s+(\d+)(.+)/) {
			$pid2name{$1} = "pid$1" if !defined($pid2name{$1});
			printf OUTFILE "=> %12s $2\n", $pid2name{$1};
		} elsif (/^<=:\s+(\d+)(.+)/) {
			$pid2name{$1} = "pid$1" if !defined($pid2name{$1});
			printf OUTFILE "<= %12s $2\n", $pid2name{$1};
		} else {
			print OUTFILE "$_";
		}
	}
}

#-------------------------------------------------------------------------------
# Translate pid to task name, along with indentation
sub do_pidWindent
{
	foreach $pid (keys(%pid2name)) { $indents{$pid} = 0; }
	while (<LOGFILE>) {
		if (/^BOUNCE DUMP END.*/) { print OUTFILE "$_"; last; }
		if (/^=>\s+(\d+)(.+)/) {
			if (!defined($pid2name{$1})) {
				$pid2name{$1} = "pid$1";
				$indents{$1} = 0;
			}
			print  OUTFILE " " x ($indents{$1} * $spaces) if $indents{$1};
			printf OUTFILE "=> %-12s $2\n", $pid2name{$1}; 
			$indents{$1}++;
		} elsif (/^<=\s+(\d+)(.+)/) {
			if (!defined($pid2name{$1})) {
				$pid2name{$1} = "pid$1";
				$indents{$1} = 0;
			}
			$indents{$1}--;
			$indents{$1} = 0 if $indents{$1} < 0;
			print  OUTFILE " " x ($indents{$1} * $spaces) if $indents{$1} > 0;
			printf OUTFILE "<= %-12s $2\n", $pid2name{$1};
		} else {
			print OUTFILE "$_";
		}
	}
}

#-------------------------------------------------------------------------------
# Main body
#-------------------------------------------------------------------------------

init_options;

# Verify validity of options
if (($logfile eq '') or (! -T $logfile)) {	# validate trace file
	print STDERR "\n\t$Ce Error: invalid trace file $logfile $Cn\n";
	usage;
}
$outfile = $logfile . ".$spaces";

# First pass through the file to fetch the pid to task name mapping.
open LOGFILE, "<$logfile" or
		die "\n\t$Ce Cannot open $logfile : $! $Cn\n";
while (<LOGFILE>) {
	scan_pid2name if /^BOUNCE PROCESS BGN/;
}
close LOGFILE;

# Second pass, convert pid to name, and indent if function exits are logged.
open LOGFILE, "<$logfile" or
	die "\n\t$Ce Cannot open $logfile : $! $Cn\n";
open OUTFILE, ">$outfile" or
	die "\n\t$Ce Cannot open $outfile : $! $Cn\n";
print "\t\ttindent > $outfile\n";
while ( <LOGFILE> ) {
	print OUTFILE "$_";
	if (/^BOUNCE DUMP BGN: FUNC_EXIT<(\d)>/) {
		if (int($1)) {
			do_pidWindent;
		} else {
			do_pid2name;
		}
	}
}
close LOGFILE;
close OUTFILE;

# Replace input file if save option requested.
if ($save) {
	print "\t\trename $outfile > $logfile\n";
	my $cnt = unlink $logfile;
	rename ($outfile, $logfile) if $cnt;
	print "Indented file: $Cb $logfile $Cn\n";
} else {
	print "Generated indented file: $Cb $outfile $Cn\n";
}
