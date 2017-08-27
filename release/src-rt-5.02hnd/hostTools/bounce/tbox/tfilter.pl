#!/usr/bin/perl -w
#
# Filename: tfilter.pl
# - Filter logs based on leaf function / branch pruning.
# - Filter logs based on task context
#
#-------------------------------------------------------------------------------

my $version = "BOUNCE TASK FILTER 1.0";
use strict;
use warnings;

use vars qw/%opt $logfile $task $prune $save $infile $outfile
			$collapse $filter/;

$logfile = '';
$task = '';
$prune = 0;
$save = 0;
$collapse = 0;
$filter = 0;

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
     -i input_logfile    : Input log file to filter
     -t task_name        : Task name to filter
     -p prune_func       : Prune branch depth of function call tree
     -f                  : Filter entry and exit
     -c                  : Collapse entry and exit to entry only
     -s                  : Save indented file (over writes original)

EOF
    exit;
}

#-------------------------------------------------------------------------------
# Fetch command line options
sub init_options()
{
    use Getopt::Std;
    my $opt_string = 'i:t:p:fcshv';
    getopts( "$opt_string", \%opt ) or usage();
    usage() if $opt{h};
    if ( $opt{v} ) {
        print "\n\n\t$0 version: $Cb $version $Cn\n";
        usage();
    }
    $logfile = $opt{i} if $opt{i};
	$task	 = $opt{t} if $opt{t};
	$collapse= int($opt{c}) if $opt{c};
	$filter  = int($opt{f}) if $opt{f};
	$prune   = int($opt{p}) if $opt{p};
    $save    = 1 if $opt{s};
}

#-------------------------------------------------------------------------------
# Filter out entirely a task's entry and exit logs
sub do_filter_task {
	while (<INFILE>) {
        if (/^BOUNCE DUMP END.*/) { print OUTFILE "$_"; return; }
		if (/([=><]*\s+)(\w+)(.+)/) {
			next if $2 eq $task;
			print OUTFILE "$_";
		} else {
			print OUTFILE "$_";
		}
	}
}

#-------------------------------------------------------------------------------
# Filter out the entry and exit logs by pruning a call tree branch
sub do_filter_func {
	my (%call_tree, $call_list, $call_line, $call_task);
	$call_task = '';
	$prune -= 1;

	while (<INFILE>) {
		if (/^BOUNCE DUMP END.*/) { print OUTFILE "$_"; return; }
		if (/=>\s+(\w+)/) {									# process entry log

			if ($call_task ne $1) {							# task switch
				if (defined($call_tree{$call_task})) {		# has call list
					$call_list = $call_tree{$call_task};	# get call list
					foreach $call_line (@{$call_list}) {	# flush each call
						print OUTFILE "$call_line";			# flush entry log
					}
				}
				$call_task = $1;							# setup new task
			}
			if (!defined($call_tree{$1})) {					# first time 
				my (@first_call) = ($_);					# create new list
				$call_tree{$1} = \@first_call;				# save reference 
			} else {
				$call_list = $call_tree{$1};				# get call list
				if ( $prune == $#{$call_list} ) {			# prune depth
					$call_line = shift(@{$call_list});		# make space
					print OUTFILE "$call_line";
				}
				push(@{$call_list}, $_);
			}
		} elsif (/<=\s+(\w+)/) {							# process exit log
			if ($call_task ne $1) {							# task switch
				if (defined($call_tree{$call_task})) {		# has call list
					$call_list = $call_tree{$call_task};	# get call list
					foreach $call_line (@{$call_list}) {	# flush each call
						print OUTFILE "$call_line";			# flush call line
					}
				}
				$call_task = $1;							# setup new task
			}
			if ((defined($call_tree{$1})) and ($#{$call_list} != -1)) {
				pop (@{$call_list});
			} else {
				print OUTFILE "$_";
			}
		} else {
			print OUTFILE "$_";
		}
	}
}

#-------------------------------------------------------------------------------
# Collapse function entry exits to entries only upto prune depth
sub do_collapse_func {
	my (%prune_tree, $call_task);
	$call_task = '';

	while (<INFILE>) {
		if (/^BOUNCE DUMP END.*/) { print OUTFILE "$_"; return; }
		if (/=>\s+(\w+)/) {
			if ($call_task ne $1) {
				$prune_tree{$call_task} = 0;
				$call_task = $1;
			}
			$prune_tree{$1} = 0 if !defined($prune_tree{$1});
			$prune_tree{$1}++ if $prune_tree{$1} < $prune;
			print OUTFILE "$_";
		} elsif (/<=\s+(\w+)/) {
			if ($call_task ne $1) {
				print OUTFILE "$_";
			} else {
				print OUTFILE "$_" if $prune_tree{$1} == 0;
				$prune_tree{$1}-- if $prune_tree{$1} > 0;
			}
		} else {
			print OUTFILE "$_";
		}
	}
}

#-------------------------------------------------------------------------------
# Rename temporary if save
sub do_rename {
	print "\t\trename $outfile > $infile\n";

   	my $cnt = unlink $infile;
	if ($cnt) {
   		rename ($outfile, $infile);
		$outfile = $infile;
	}
}

#-------------------------------------------------------------------------------
# Perform a task filter, prune filter or collapse operation.
sub process_operation {
	my ($opcode, $operation, $suffix) = @_;

   	$infile = $outfile;
    $outfile .= $suffix;

	if ( $opcode eq 'Task' ) {
		print "\tFilter $Cb$task$Cn $infile > $outfile\n";
	} else {
		print "\t$opcode $Cb$prune$Cn $infile > $outfile\n";
	}

    open INFILE, "<$infile" or
		 die "\n\t$Ce $opcode: Cannot open input $infile : $! $Cn\n";
    open OUTFILE,">$outfile" or
		 die "\n\t$Ce $opcode: Cannot open output $outfile : $! $Cn\n";

    while ( <INFILE> ) {
        print OUTFILE "$_";
        &$operation() if /^BOUNCE DUMP BGN:/;
    }
    close INFILE;
    close OUTFILE;

    do_rename if ($save);
}

#-------------------------------------------------------------------------------
# Main body
#-------------------------------------------------------------------------------

init_options;

# Verify logfile.
if (($logfile eq '') or (! -T $logfile)) {  # validate trace file
    print STDERR "\n\t$Ce Error: invalid trace file $logfile $Cn\n";
	usage;
}
$outfile = $logfile;

# Verify at least one operation.
if (($task eq '') and ($filter == 0) and ($collapse == 0)) {
	print STDERR "\n\t$Ce Need to specify task or prune based filtering. $Cn\n";
	usage;
}

# If both filter and collapse specified, select filter and ignore collapse.
if ($filter and $collapse) {
	print STDERR "\n\t$Ce WARNING: doing filter, ignoring collapse$Cn\n";
	$collapse = 0;
}

# Setup default pruning depth if not specified.
if ($prune == 0) {
	$prune = 4 if $collapse;	# Default pruning for collpase
	$prune = 2 if $filter;		# Default pruning for filter
}

# Filter tasks if requested.
process_operation('Task', \&do_filter_task, ".$task") if $task ne '';

# Prune function call tree by filtering or collapsing, as requested.
process_operation('Filter', \&do_filter_func, ".f$prune") if $filter;
process_operation('Collapse', \&do_collapse_func, ".c$prune") if $collapse;

# Cleanup temporaries ...
if ( $task and $prune ) {
	print "\t\tremove $Cb$logfile.$task$Cn\n";
	unlink($logfile . ".$task");
}

if ($save) {
	print "Filtered file: $Cb $logfile $Cn\n";
} else {
	print "Generated filter file: $Cb $outfile $Cn\n";
}
