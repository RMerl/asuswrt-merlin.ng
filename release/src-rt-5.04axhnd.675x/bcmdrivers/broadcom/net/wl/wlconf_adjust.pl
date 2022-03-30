#!/usr/bin/perl -w
use File::Copy;
$num_args = $#ARGV + 1;

if ( $num_args != 4 ) {

	print "\n Usage adjust.pl <sourcedir> <conf file> <VIS> <enable/disable>";
	exit;
}

my $orig_file;
my $tmp_new_file;
my $infile;
my $outputfile;
my $line;
my $matching_pattern;
my $matching_pattern_2;
my $replace_line;
my $todo_replace = 0;
#my @filetypes_to_change= ( "HEADER","CONFIG");
my @filetypes_to_change= ( "CONFIG");
my $setting_file;

$setting_dir   = $ARGV[0];
$orig_file =  $ARGV[1];
$setting_filed = uc $ARGV[2];
$setting_value = uc $ARGV[3];


sub open_filehandle_for_output {
	my $filename          = $_[0];
	my $overWriteFilename = ">" . $filename;
	local *FH;
	open( FH, $overWriteFilename ) || die "Could not open $filename";
	return *FH;
}

sub open_filehandle_for_input {
	my $filename = $_[0];
	local *FH;
	open( FH, $filename ) || die "Could not open $filename";
	return *FH;
}

foreach (@filetypes_to_change) {

	$setting_file = $_;
	if ( $_ eq "HEADER" ) {
		$orig_file    = ${setting_dir}.'/shared/bcmconfig.h';
		$tmp_new_file = ${setting_dir}.'/shared/.bcmconfig.h';
	}
	else {
		$tmp_new_file = ${orig_file}.'temp.temp';
	}

	$infile     = open_filehandle_for_input( ${orig_file} );
	$outputfile = open_filehandle_for_output(${tmp_new_file} ) ;

	if($setting_value eq "ENABLE") {
		if ( $setting_file eq "HEADER" ) {
			$matching_pattern = qr/^#undef\s+__CONFIG_${setting_filed}__\s*\n/s;
			$matching_pattern_2 = qr/^#undef\s+__CONFIG_${setting_filed}__\s*\n/s;
			$replace_line     = '#define __CONFIG_' . $setting_filed . '__ 1' . "\n";
		}
		else {
			$matching_pattern = qr/^#\s*CONFIG_${setting_filed} is not .*\n/s;
			$matching_pattern_2 = qr/^CONFIG_${setting_filed}=n\s*\n/s;
			$replace_line = "CONFIG_$setting_filed=y\n";
			
			
		}
	} elsif($setting_value eq "DISABLE") {
		if ( $setting_file eq "HEADER" ) {

			$matching_pattern = qr/^#define\s+__CONFIG_${setting_filed}__\s+1\s*\n/s;
			$matching_pattern_2 = qr/^#define\s+__CONFIG_${setting_filed}__\s+1\s*\n/s;
			$replace_line = '#undef __CONFIG_' . $setting_filed . "__\n";
		}
		else {
			$matching_pattern = qr/^CONFIG_${setting_filed}=y\s*\n/s;
			$matching_pattern_2 = qr/^CONFIG_${setting_filed}=y\s*\n/s;
			$replace_line     = "# CONFIG_${setting_filed} is not set\n";
		}
	} else {
			print "setting values should be [enable/disable]";
			exit;
	}

	while ( $line = <$infile> ) {
		if ( $line =~ $matching_pattern  || $line =~ $matching_pattern_2) {
			print $outputfile $replace_line;
			$todo_replace = 1;
		}
		else {
			print $outputfile $line;
		}
	}

	close $infile;
	close $outputfile;
	if ($todo_replace) {
		unlink $orig_file;
		rename $tmp_new_file, $orig_file;
	} else {
		unlink $tmp_new_file
	}
}
