#!/usr/bin/perl -w
use strict;
use warnings;
use Config;
use File::Basename;

# ToolDir , search path , sh file

my $ToolHelp = shift;
my $FilePath = shift;
my $TempFilePath = shift;
my $DictEnum = shift;
my $DictNoFound = shift;
my $ModelName = shift;
my $HtmlPrepOutput = shift;
my $FH;
open($FH, ">", $TempFilePath) or die "Cannot open $TempFilePath for writing: $!\n";
printf $FH "#!/bin/sh\n";

my $location = $FilePath;
my $n = 0;
my $exclude_n = 0;
my $x64 = $Config{archname} =~ /x86_64/ ? 1 : 0;
readsub($location);
close $FH or die "Cannot close $TempFilePath: $!\n";
print "\nFound $n file(s)!\n";
print "\nExclude $exclude_n file(s)!\n";
exit;


#
# add include list for HTML prep
#
sub includelist_htmlprep {
	my $fn = shift;
# Case exact
	if ($fn eq "Advanced_ADSL_Content.asp") { return 1; }
	if ($fn eq "Advanced_IPTV_Content.asp") { return 1; }
	if ($fn eq "Advanced_FirmwareUpgrade_Content.asp") { return 1; }
	return 0;
}

#
# add exclude list here
#
sub excludelist {
	my $fn = shift;
# Case exact
	if ($fn eq "jquery.js") { return 1; }
	if ($fn eq "svg.js") { return 1; }
	if ($fn eq "tablesorter.js") { return 1; }
	if ($fn eq "jstz.min.js") { return 1; }
	if ($fn eq "ASUS_DDNS_TOS.asp") { return 1; }
	if ($fn eq "ext-all.js") { return 1; }
	if ($fn eq "ext-base.js") { return 1; }
	if ($fn eq "coda-slider.1.1.1.pack.js") { return 1; }
	if ($fn eq "jquery-easing-compatibility.1.2.pack.js") { return 1; }
	if ($fn eq "jquery-easing.1.2.pack.js") { return 1; }
	if ($fn eq "ark-app-list.json") { return 1; }
	return 0;
}

#
# add extension here
#
sub ext_filter {
	my $ext = shift;
	$ext = lc($ext);
	if ($ext eq "asp") { return 1; }
	if ($ext eq "htm") { return 1; }
	if ($ext eq "html") { return 1; }
	if ($ext eq "js") { return 1; }
	if ($ext eq "json") { return 1; }
	if ($ext eq "xml") { return 1; }
	return 0;
}

#
# add NEW preprocessor command here
#
sub addcommand {
	my $fn = shift;
	$ModelName = uc($ModelName);
	 my ($includelist_htmlprep)=&includelist_htmlprep(basename( $fn ));
	 if ($includelist_htmlprep)
	 {
	 # process files that need to do preprocessor
		my $htmlprep = $x64 && -e "$ToolHelp/LnxHtmlPrep.x64" ? "LnxHtmlPrep.x64" : "LnxHtmlPrep";
		printf $FH "%s/%s %s %s %s %s/DefaultModels.txt\n", $ToolHelp, $htmlprep, $fn, $ModelName, $HtmlPrepOutput, $ToolHelp;
	 }
	 printf $FH $ToolHelp."/rmvcomments.pl"." ".$fn."\n";
	 my $rmvtabs = $x64 && -e "$ToolHelp/LnxRmvTabs.x64" ? "LnxRmvTabs.x64" : "LnxRmvTabs";
	 printf $FH "%s/%s %s\n", $ToolHelp, $rmvtabs, $fn;
	 # printf $FH $ToolHelp."/LnxHtmlEnumDict"." ".$fn." ".$DictEnum." ".$DictNoFound."\n";
}

sub readsub {
  my ($file_t) = @_;
  if (-f $file_t) {                  #if its a file
	 my $ext = ($file_t =~ m/([^.]+)$/)[0];
	 #print $ext;
	 my ($ret_ext)=&ext_filter($ext);
	 if ($ret_ext)
	 {
		 my $filename = basename( $file_t );
		 my ($ret_exclude)=&excludelist($filename);
		 if ($ret_exclude)
		 {
		 printf $file_t;
		 printf " in exclude list, skipped\n";
		 $exclude_n++;
		 }
		 else
		 {
		 print $file_t;
		 print "\n";
		 addcommand($file_t);
		 }
		$n++;                           #the total number of files
	 }
  }
  if (-d $file_t) {                  #if its a directory
     opendir(AA,$file_t) || return;
     my @list = readdir(AA);
     closedir (AA);
     my $file_to_act;
     foreach $file_to_act (sort @list) {
       if ($file_to_act =~ /^\.|\.$/) { next; }
       else { readsub("$file_t/$file_to_act"); }
     }
  }
}
