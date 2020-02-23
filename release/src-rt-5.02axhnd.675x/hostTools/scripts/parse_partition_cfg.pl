#!/usr/bin/perl -w
use strict;
use warnings; 
use Data::Dumper;

# This hash associates the partition config entry names with a regexp
# which matches the valid values for each entry
my %partition_cfg_format = (
	PARTITION_NAME 			=> "\\S",
	PARTITION_SIZE_CFG_TYPE	=> "\\bsize\\b|\\bfill_dev\\b|\\bfill_free\\b",
	PARTITION_SIZE 			=> "\\d+",
	PARTITION_FS_MNG 		=> "\\bext4\\b|\\bvfat\\b",
	PARTITION_FS_RAW 		=> "\\bubifs\\b",
	PARTITION_MNG_DEV 		=> "\\S",
	PARTITION_RAW_DEV 		=> "\\S",
	PARTITION_CREATE_POLICY	=> "\\bmandatory\\b|\\boptional\\b",
	PARTITION_MNT_OPT 		=> "",
	PARTITION_MNT_POINT 	=> "",
);

# 0 - Parse arguments and open files
my $num_args = $#ARGV + 1;
if ($num_args != 3) {
	print "\nUsage: parse_partition_cfg.pl <partition_config_file1,partition_config_file2,..> <part_input_script> <part_output_script>\n";
	exit;
}

my @cfg_file_names = split(',',$ARGV[0]);
my $input_script_file_name=$ARGV[1];
my $output_script_file_name=$ARGV[2];
open my $fh_ip_scr, '<', $input_script_file_name  or die "Unable to open file: '$input_script_file_name' $!\n";

# 1 - Parse partition configuration files into an array of partition hashes
my %partitioncfg;
my @partitions;
my $partition_started=0;
my $i;
my $fh_cfg;
my $cfg_file_name; 

foreach my $cfg_file_name ( @cfg_file_names ) {
	open $fh_cfg,    '<', $cfg_file_name       or die "Unable to open file: '$cfg_file_name' $!\n";
	print "Parsing partition config file: $cfg_file_name\n";

	while (<$fh_cfg>) {
		# Look for partition start markers, note that blank line signals end of partition as well
		if (( $_ =~ /^\[PARTITION\]/ ) || ( $_ =~ /^\s*$/ )){
			if ( $partition_started ) {
				# Partition just ended, push hash into array
				push @partitions, { %partitioncfg };
				%partitioncfg = ();

				# If blank line is detected then wait until partition tag re-appears
				if ( $_ =~ /^\s*$/ ) {
					$partition_started=0;
				}
			} elsif ( $_ =~ /^\[PARTITION\]/ ) {
				$partition_started=1;
			}
		}

		# Look for partition config entries ( match '=' )
		if ( $partition_started && $_ =~ /=/ ) {
			# Split name and value along '='
			my ($key, $value) = split /=/, $_;
			# Remove newline 
			chomp($value);
			# Remove spaces
			$value =~ s/^\s+|\s+$//g;
			# Push into hash
			$partitioncfg{$key} = $value;
			
			# Remove any duplicate partition hashes
			if ( $key eq "PARTITION_NAME" ) {
				for ( $i=0; $i<@partitions; $i++ ) {
					if ( $partitions[$i]->{$key} eq $value ) {
						# Remove hash from array
						print "Removing Duplicate partition $value\n";
						splice(@partitions, $i, 1);
						last;
					}
				}
			}
		}
	}
	# Add the last processed partition ( if no blank line at the end )
	if ( %partitioncfg && $partition_started) {
		push @partitions, { %partitioncfg };
		$partition_started=0;
	}
}
#print Dumper @partitions;

# 2 - Verify all partitions that we have parsed
my $href;
my $entry;
my $regexp;
my $entryval;
my $err=0;
my @partition_cfg_entries = keys %partition_cfg_format;
my $output_partitions = "partitions\=\"\n";

# Process each partitioncfg hash from the partitions array
for $href ( @partitions ) {
	$err = 0;
	# Verify each partition entry
	foreach $entry ( @partition_cfg_entries ) {
		if ( exists($href->{$entry} )) {
			# Try and match regexp for entry, this takes care of basic type checking
			$regexp = $partition_cfg_format{$entry};
			$entryval = $href->{$entry};
			if ( !($entryval =~ /$regexp/) ) {
				print "Invalid value for Config entry ", $entry, ": ", $entryval, " !\n";
				$err = 1;
			} else {
				# Check that size is greater than 0
				if ( $entry eq "PARTITION_SIZE" && $entryval <= 0 ) {
					print "Invalid value for partition size! Size must be > 0!\n";
					$err = 1;
				}
				$output_partitions .= $entry."=".$entryval."|";
			}
		} else {
			print "Config entry for ", $entry, " is missing!\n";
			$err = 1;
		}
	}
	
	# Check if user has configured a mandatory partition with a 100% fill size AND multiple partitions
	# are defined. Such a configuration  will prevent any optional partitions from ever being created
	if ( $href->{PARTITION_CREATE_POLICY} eq "mandatory" &&  (@partitions > 1) ) {
		if ( $href->{PARTITION_SIZE_CFG_TYPE} =~ /fill*/ && $href->{PARTITION_SIZE} == 100 ) {
			print "Multiple partitions detected AND Mandatory partition has 100% fill size! Optional partitions will never be created!\n";
			$err = 1;
		}
	}

	if ( $err ) {
		print "Invalid Configuration for User Partition: ", $href->{PARTITION_NAME}, "\n";
		$output_partitions = "";
		last;
	} else {
		$output_partitions .= "\n\n";
	}
}

# 3 - Combine partition script and parsed partitions into final output script
if ( $output_partitions ) {
	open my $fh_op_scr, '>', $output_script_file_name or die "Unable to open file: '$output_script_file_name' $!\n";
	$output_partitions .= "\"\n";
	#print "\n", $output_partitions;
	while (<$fh_ip_scr>) {
		if ( $_ =~ /__BRCM_USER_PARTITION_CFG__=/ ) {
			print $fh_op_scr $output_partitions
		} else {
			print $fh_op_scr $_
		}
	}
	close $fh_op_scr;
}

close $fh_cfg;
close $fh_ip_scr;

exit $err;
