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


 %obj_matchers=("^} mngr_Dev2WifiRadioObject",\@radio_items);


 sub open_filehandle_for_input
 {
     my $filename = $_[0];
     local *FH;

     open (FH, $filename) || die "Could not open $filename";

     return *FH;
 }

 my $orig_file="./include/wlcsm_dm_generic.h";
 foreach $key (keys %obj_matchers) {
	my $infile=open_filehandle_for_input("$orig_file");
	my $outputfile=open_filehandle_for_output("$orig_file"."_bak");
	my $line;
       my @addlines;
       while($line=<$infile>) {

		if($line =~ /$key/)
		{
			@addlines=@{$obj_matchers{$key}};

			foreach(@addlines) {
				#print "$_\n";
				print $outputfile "$_\n";
			}

		}
		#print $line;
		print $outputfile $line;
	}
	close $outputfile;
	close $infile;
	unlink $orig_file;
	rename "$orig_file"."_bak", $orig_file;
}
