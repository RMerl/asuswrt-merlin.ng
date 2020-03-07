#!/usr/bin/perl -w

 sub open_filehandle_for_output
 {
     my $filename = $_[0];
     my $overWriteFilename = ">" . $filename;
     local *FH;
 
     open (FH, $overWriteFilename) || die "Could not open $filename";
 
     return *FH;
 }
 
 
@radio_items= ( "    WL_STATION_LIST_ENTRY     	*stationList;  /**< runtime STA list */",
				"    WL_FLT_MAC_ENTRY           *m_tblScanWdsMac; /**<scan AP list */");

@assocsta_items= (  "    void 	*prev;  /**< prev station pointer */",
					"    void 	*next;  /**< next station pointer */");

%obj_matchers=( "^} mngr_Dev2WifiRadioObject",\@radio_items,
				"^} mngr_Dev2WifiAssociatedDeviceObject",\@assocsta_items);

 sub open_filehandle_for_input
 {
     my $filename = $_[0];
     local *FH;
 
     open (FH, $filename) || die "Could not open $filename";
 
     return *FH;
 }

 my $orig_file="./include/wlcsm_dm_generic_orig.h";
 my $out_file="./include/wlcsm_dm_generic.h";

sub check_file_changed {
	if( -f $out_file) {
		my $infile=open_filehandle_for_input("$out_file");
		my @check_items=();
		my $found_item=0;

		foreach $key (keys %obj_matchers) {
			@addlines=@{$obj_matchers{$key}};
			push @check_items,$addlines[0];
		}

		while($line=<$infile>) {
			foreach(@check_items) {
				if($line eq $_."\n") {
					$found_item = $found_item+1;
				}
			}
		}

		close $infile;
		if($found_item==@check_items)  {
			print "file already changed\n";
			return 1;
		} else {

		print "go ahead to modify the dm_generic file\n";
		return 0;
		}
	} 
	return 0;
}

if(check_file_changed()) {
	exit 0;
} 

		my $infile=open_filehandle_for_input("$orig_file");
		my $timestamp=localtime;
		my $outputfile_name;
		do {
			$timestamp=localtime;
			$timestamp =~ s/\s/_/g;
			$outputfile_name="$orig_file".$timestamp;
		} while( -f $outputfile_name );

my $line;
my @addlines;

my $outputfile=open_filehandle_for_output($outputfile_name);

while($line=<$infile>) {
	foreach $key (keys %obj_matchers) {
		if($line =~ /$key/)
		{
			@addlines=@{$obj_matchers{$key}};

			foreach(@addlines) {
				#print "$_\n";
				print $outputfile "$_\n";
			}

		}
	}
	print $outputfile $line;
}
close $outputfile;
close $infile;
rename $outputfile_name, $out_file;
