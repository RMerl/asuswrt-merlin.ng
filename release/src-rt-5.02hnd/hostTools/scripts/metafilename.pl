#!/usr/bin/perl
use strict;
use warnings;

# Global dictionary
my %dictionary = (
  C =>          'BRCM_CHIP',
  Fap =>        'CONFIG_BCM_FAP',
  FapM =>       'CONFIG_BCM_FAP_MODULE',
  FapI =>       'CONFIG_BCM_FAP_IMPL',
  PktDma => 	'CONFIG_BCM_PKTDMA',
  PktDmaI =>    'CONFIG_BCM_PKTDMA_IMPL',
  L1CShft => 	'CONFIG_MIPS_L1_CACHE_SHIFT',
  TxBds =>      'CONFIG_BCM_NR_TX_BDS',
  RxBds =>      'CONFIG_BCM_NR_RX_BDS',
  Qos =>        'CONFIG_BCM_INGQOS',
  QosM =>       'CONFIG_BCM_INGQOS_MODULE',
  Bpm =>        'CONFIG_BCM_BPM',
  BpmM =>       'CONFIG_BCM_BPM_MODULE',
  Tm =>         'CONFIG_BCM_TM',
  TmM =>        'CONFIG_BCM_TM_MODULE',
  Ipv6 =>       'CONFIG_BCM_FAP_IPV6',
  ESwtch => 	'CONFIG_BCM_EXT_SWITCH',
  RxSplit => 	'CONFIG_BCM_PKTDMA_RX_SPLITTING',
  TxSplit => 	'CONFIG_BCM_PKTDMA_TX_SPLITTING',
  RxDmaChans => 'CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS',
  TxDmaChans => 'CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS',
  Xtm =>        'CONFIG_BCM_XTMCFG',
  XtmM =>       'CONFIG_BCM_XTMCFG_MODULE',
  XtmI =>       'CONFIG_BCM_XTMRT_IMPL',
  FL2 =>        'CONFIG_BCM_FAP_LAYER2',
  FGpon =>      'CONFIG_BCM_FAP_GPON', 
  GinpRtx =>    'CONFIG_BCM_DSL_GINP_RTX', 
  FGso =>       'CONFIG_BCM_FAP_GSO',
  FGsoLpBk =>   'CONFIG_BCM_FAP_GSO_LOOPBACK',
  Gmac =>       'CONFIG_BCM_GMAC',
  T =>          'TEST_VAR',
  Jumbo =>      'CONFIG_BCM_JUMBO_FRAME',
  IntExtSw =>   'CONFIG_BCM_PORTS_ON_INT_EXT_SW',
);


# Extract meta data out of a filename (text string)

use Getopt::Long;

my $file_elem_separator = '.';
my $file_elem_equal = '_';
my $meta_elem_separator = ' ';
my $meta_elem_equal = '=';
my $file2meta = 0;
my $meta2file = 0;
my $defaults = 0;
my $dictFile = 0;
my $checkname = 0;

my $usage = q{
Usage: metafilename <--file2meta/--meta2file/--checkname> <default_settings> <settings>

options:
  -f       Take a filename as input and extract the metadata
  -m       Take metadata as input and generates a filename
  -d       Reads a default configuration and converts it to make variables
  -c       Performs sanity check on filename and default name
  -dictionary filename.txt   Reads a dictionary from a file (entries in the form X => Y)

Example: metafilename.pl -f "FL2_y.Ipv6_n" "myfile.C_63268.Ipv6_y"
         metafilename.pl -m "FL2_y.Ipv6_n" "BRCM_CHIP=63268 CONFIG_BCM_FAP_IPV6=y"
         metafilename.pl -c "FL2_y.Ipv6_n" "Ipv6_y"
         metafilename.pl -d "FL2_y.Ipv6_n"
};

GetOptions(
    'file2meta',       \$file2meta,
    'meta2file',       \$meta2file,
    'checkname',       \$checkname,
    'defaults',        \$defaults,
    'extdictionary=s', \$dictFile
  )
  or die $usage;

# Arguments
die $usage unless ((@ARGV == 2 && $file2meta+$meta2file+$checkname == 1 && $defaults == 0) || (@ARGV == 1 && $file2meta+$meta2file+$checkname == 0 && $defaults == 1));

my $input_string = pop @ARGV;
my $default_filename = pop @ARGV unless @ARGV == 0;

if ($dictFile) {
	# Local dictionary
   open (MYFILE, "<$dictFile");
   while (<MYFILE>) {
      if ($_ =~ /\s*(\w+)\s*=>?\s*'?(\w+)\'?/) {
        $dictionary{$1} = $2;
      }  else {
         print STDERR "$0: $dictFile couldn't parse $_\n";
      }
   }
   close (MYFILE);
}

# Reverse dictionary
my %rdictionary = ();
for my $key (keys %dictionary) {
   $rdictionary{$dictionary{$key}} = $key;
}

# Process the default metadata
my %default_metadata = ();

# Determine if make arguments are passed in
if ($file2meta) {
   f2m($default_filename, $input_string);
} elsif ($meta2file) {
   m2f($default_filename, $input_string);
} elsif ($checkname) {
   checkName($default_filename, $input_string);
} else {
   d2m($input_string);
}

exit;

# Utility function to generate a hash from filename
sub f2h {
   my ($filename, $myhref) = @_;

   # Get the raw elements
   my @rawdata = split /\Q$file_elem_separator\E/, $filename;

   # Process each raw element
   for my $r (@rawdata) {
      if ($r =~ /\Q$file_elem_equal\E/) {
         my ($key, $val) = split /\Q$file_elem_equal\E/, $r;
         $myhref->{$key} = $val;
      }
   }
}

#
# Generate Make variables from Filename
#
sub f2m {
   my ($default_filename, $filename) = @_;

   # Remove the file extension (everything after last .)
   #$filename =~ m/^(.*)\..*/;
   #$filename = $1;

   checkName($default_filename, $filename);

   my %metadata = ();
   # Set the defaults into the hash
   f2h($default_filename, \%metadata);

   # Overwrite the hash with the data from the filename
   f2h($filename, \%metadata);

   # Generate the makefile variables
   my $makevars = "";
   for my $key (sort keys %metadata) {
      $makevars = $makevars.$dictionary{$key}.$meta_elem_equal.$metadata{$key}.$meta_elem_separator;
   }
   chop($makevars);
   print "$makevars\n";
}

#
# Generate Filename from the Make variables
#
sub m2f {
   my ($default_filename, $makeargs) = @_;

   my %default_metadata = ();
   # Set the defaults into the hash
   f2h($default_filename, \%default_metadata);

   # Get the raw elements
   my @rawdata = split /\Q$meta_elem_separator\E/, $makeargs;

   # Prepare to store results in a hash
   my %metadata = ();

   # Process each raw element
   for my $r (@rawdata) {
      if ($r =~ /\Q$meta_elem_equal\E/) {
         my ($key, $val) = split /\Q$meta_elem_equal\E/, $r;
         $metadata{$key} = $val;
      }
   }

   my %filenameElems = ();
   
   # Filename Elements
   for my $key (sort keys %metadata) {
      if ($metadata{$key} ne $default_metadata{$rdictionary{$key}}) {
        $filenameElems{$rdictionary{$key}} = $metadata{$key};
      }
   }

   # Filename
   my $filename = "";
   for my $key2 (sort keys %filenameElems) {
      $filename = $filename.$key2.$file_elem_equal.$filenameElems{$key2}.$file_elem_separator
   }
   
   chop($filename);
   print "$filename\n";
}


#
# Generate make variables from default string
#
sub d2m {
   my ($default_filename) = @_;

   my %default_metadata = ();
   # Set the defaults into the hash
   f2h($default_filename, \%default_metadata);

   # Generate the makefile variables
   my $makevars = "";
   for my $key (sort keys %default_metadata) {
      if (!exists $dictionary{$key} || $dictionary{$key} eq "" ) {
      	 die("Error: unknown filename part: $key\n");
      }
      else {      
	      $makevars = $makevars.$dictionary{$key}.$meta_elem_equal."\$(".$dictionary{$key}.")".$meta_elem_separator;   	
      }
	      
      #$makevars = $makevars.$dictionary{$key}.$meta_elem_equal.$ENV{$dictionary{$key}}.$meta_elem_separator;
   }   
   chop($makevars);
   print "$makevars\n";
}

#
# Check if a release name is valid.  Dies on failure
#
sub checkName {
   my ($default_filename, $filename) = @_;
   my %default_hash = ();
   my %file_hash = ();
   my $rawelem = "";   
   my $lastkey="";
   
   for my $rawelem (split (/\Q$file_elem_separator\E/, $default_filename)) {
      my $key;
      my $val;
      if ($rawelem =~ /\Q$file_elem_equal\E/) {
         ($key, $val) = split /\Q$file_elem_equal\E/, $rawelem;
         if (! exists($dictionary{$key}) ) {      
            die "ERROR -- element ($key) in default, but not dictionary\n";
         }
         $default_hash{$key} = $val;
      }
      else {
      	 die "ERROR -- element ($rawelem) without equal operator ($meta_elem_equal)\n";
      }

      if ($key le $lastkey) {
         die "ERROR -- out of order key ($lastkey / $key) in default filename\n";
      }
      else {
         $lastkey = $key;
         
      }
   }

   $lastkey="";
   for my $rawelem (split (/\Q$file_elem_separator\E/, $filename)) {
      my $key;
      my $val;
      
      if ($rawelem =~ /\Q$file_elem_equal\E/) {
         ($key, $val) = split /\Q$file_elem_equal\E/, $rawelem;
         $file_hash{$key} = $val;

         if ($key le $lastkey) {
            die "ERROR -- out of order element ($lastkey $key) in filename\n";
         }
         elsif (! exists($default_hash{$key}) ) {      
            die "ERROR -- element ($key) in filename ($filename) but not in default\n";
         }
         elsif ( $default_hash{$key} eq $file_hash{$key} ) {
            die "ERROR -- element ($key) in filename ($filename) has same value as in default\n";
         }
         else {
            $lastkey = $key;
         }

      }   
   }
}
