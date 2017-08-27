#!/usr/bin/perl -w

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#
# Filename: symfind_mips
#
# Description:
#
#   Resolve instruction addresses in a log file to symbolName+Offset.
#   The logfile is scanned for a instruction addesses using format A[0xSSSSSSSS]
#   where S=[a..z|A..Z|0..9|_], i.e. a alpha numeric character or underscore.
#
#   The view pathname from which the target image was built is specified using:
#         command line argument: -c <CCVIEWPATH>
#   or,
#	SF_CCVIEWPATH="/home/self/ClearCase/self_dev_view/CommEngine"
#	export SF_CCVIEWPATH
#
#   The project name, e.g. 96368GW
#         command line argument: -p <PROJECTNAME>
#   or,
#	SF_PROJECTNAME=96368GW
#	export SF_PROJECTNAME
#
#   The kernel version override, defaults to 2.6.21.5
#         command line argument: -k <KERNELVERSION>
#
#   Kernel Symbols are extracted from the System.map file located in:
#         command line argument: -s <SYSTEMMAPFILE>
#         or,
#         default: $SF_CCVIEWPATH/kernel/linux/System.map
#
#
#   Module Symbols are extracted using:
#
#   A. the modules relocation info text file that was created using
#      "cat /proc/modules" on target.
#      This modules file specifies which libraries are loaded and their
#      relocation on target.
#         command line argument: -m <MODULEINFOFILE>
#
#   B. the relocatable libraries built for the target.
#      The directory containing the libraries is specified using:
#         command line argument: -l <LIBRARYDIRPATH>
#         or,
#         $SF_CCVIEWPATH/target/$SF_PROJECTNAME/modules/lib/modules/$SF_KERNELVERSION/extra
#         $SF_CCVIEWPATH/target/$SF_PROJECTNAME/modules/lib/modules/$SF_KERNELVERSION/kernel
#
#   or,
#
#   C. Using a previously generated target "System and Modules" symbol map.
#        command line argument: -t <SYMBOLMAPFILE>
#
#      The -g command line option can be used to generate the SymbolMap
#      for reuse in subsequent parsing of logfiles.
#        command line argument: -g <GENSYMBOLFILE>
#
#
#   The logfile containing instruction addrsses to be resolved
#   is specified using:
#        command line argument: -i <InputFileName>
#
#   The output resolved logfile name is specified using
#        command line argument: -o <OutputFileName> 
#
#****************************************************************************
#
#  Copyright (c) 2000-2008 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or
#  its licensors, and may only be used, duplicated, modified or distributed
#  pursuant to the terms and conditions of a separate, written license
#  agreement executed between you and Broadcom (an "Authorized License").
#  Except as set forth in an Authorized License, Broadcom grants no license
#  (express or implied), right to use, or waiver of any kind with respect to
#  the Software, and Broadcom expressly reserves all rights in and to the
#  Software and all intellectual property rights therein.  IF YOU HAVE NO
#  AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
#  AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
#  SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization,
#  constitutes the valuable trade secrets of Broadcom, and you shall use all
#  reasonable efforts to protect the confidentiality thereof, and to use this
#  information only in connection with your use of Broadcom integrated circuit
#  products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

use strict;
use warnings;

use vars qw/ %opt $symFind_version $sfdbg %ignoresymbols
             %symbolList @symbolList $symCount $symResolve $symRegex
             %moduleList %moduleInitList $systemmapfile $moduleinfofile
             $libraryextradir $librarykerneldir
             $symbolmapfile $gensymbolfile
             $inputlogfile $outputlogfile
             $multipleSymDelimit
             /;

use English;
use vars qw/$Ce $Cb $Cn/;
if (($OSNAME eq 'linux') || ($OSNAME eq 'uinx')) {
	$Ce = "\e[0;31m"; $Cb = "\e[0;36;44m"; $Cn = "\e[0m";
} else {
	$Ce = ''; $Cb = ''; $Cn = '';
}

# Default Address searching regular expression: A[0xwwwwwwww],
# where w is an alphanumeric character
# $symRegex = 'A\[0x(\w+)\]';

$symRegex = '\[<(\w+)>\]';

$multipleSymDelimit = "|";	# Multiple symbol delimiter
my $nm_mips = "/opt/toolchains/uclibc-crosstools-gcc-4.2.3-3/usr/bin/mips-linux-nm";
$symFind_version = "BCM SYMFIND 1.1";

sub usage()
{
   print << "EOF";

   usage: $0 
     -h                  : this (help) message
     -v                  : version
     -d                  : verbose debug printing

     -c CCVIEWPATH       : Clearcase view pathname
                           [e.g. /home/self/ClearCase/self_dev_view/CommEngine

     -p PROJECTNAME      : Project name [e.g. 96368GW]
     -k KERNELVERSION    : Kernel version, default: 2.6.21.5

     -s SYSTEMMAPFILE    : Pathname of System.map file
     -m MODULEINFOFILE   : Pathname of output file from 'cat /proc/modules'
     -l LIBRARYDIRPATH   : Pathname to directory containing modules|libraries

     -t SYMBOLMAPFILE    : Pathname of Symbol [System and Modules] file
     -g GENSYMBOLFILE    : Generate a global Symbol map file

     -e SYMADDRREGEX     : Regular Expression for Symbol Address: \[<(\\w+)>\] or A\[0x(\\w+)\]
     -i INPUTLOGFILE     : Input Log file containing instruction addresses
     -o OUTPUTLOGFILE    : Output Log file containing resolved symbols

    example: $0 -c /home/self/ClearCase/self_dev_view/CommEngine -p 96368GW -m ./modules -g 96368GW_symbols
    example: $0 -t 96368GW_symbols -i kmalloc.log -o kmalloc.trace
    example: $0 -t 96368GW_symbols -e "([8c]0[01].....)" -i kmalloc.log -o kmalloc.trace

EOF
        exit;
}

sub init()
{
   use Getopt::Std;
   my $opt_string = 'hvdc:p:k:s:m:l:t:g:e:i:o:';
   getopts( "$opt_string", \%opt ) or usage();
   usage() if $opt{h};

   if ( $opt{v} ) {

      print "\n\n\t$0 version: $Cb$symFind_version$Cn\n";
      usage();
   }

   $sfdbg = 1 if $opt{d};

   my $ccviewpath = '';
   if ( $ENV{SF_CCVIEWPATH} ) {
      $ccviewpath = $ENV{SF_CCVIEWPATH};
      print "\tENV SF_CCVIEWPATH = $Cb$ccviewpath$Cn\n" if $sfdbg;
   }
   if ( $opt{c} ) { # command line option overrides environment variable 
      $ccviewpath = $opt{c};
      print "\tCCVIEWPATH = $Cb$ccviewpath$Cn\n" if $sfdbg;
   }

   my $projectname = '';
   if ( $ENV{SF_PROJECTNAME} ) {
      $projectname = $ENV{SF_PROJECTNAME};
      print STDERR "\tENV SF_PROJECTNAME = $Cb$projectname$Cn\n" if $sfdbg;
   }
   if ( $opt{p} ) { # command line option overrides environment variable
      $projectname = $opt{p};
      print "\tPROJECTNAME = $Cb$projectname$Cn\n" if $sfdbg;
   }

   my $kernelversion = "2.6.21.5";
   if ( $ENV{SF_KERNELVERSION} ) {
      $kernelversion  = $ENV{SF_KERNELVERSION};
      print "\tENV SF_KERNELVERSION = $Cb$kernelversion$Cn\n" if $sfdbg;
   }
   if ( $opt{k} ) { # command line option overrides environment variable
      $kernelversion  = $opt{k};
      print "\tKERNELVERSION = $Cb$kernelversion$Cn\n" if $sfdbg;
   }

        # System Map Override:
        # - Try environment variable SF_SYSTEMMAPFILE
        # - Use System.map specified in clearcase view
        # - Use command line option -s <SystemmapFilePathName> 
   if ( $ENV{SF_SYSTEMMAPFILE} ) {
      $systemmapfile  = $ENV{SF_SYSTEMMAPFILE};
      print "\tENV SF_SYSTEMMAPFILE = $Cb$systemmapfile$Cn\n" if $sfdbg;
   }
   if ( $ccviewpath ) { # -c option overrides environment [ hmmm ]
      $systemmapfile  = $ccviewpath . "/kernel/linux/System.map";
      print "\tCC View System.map = $Cb$systemmapfile$Cn\n" if $sfdbg;
   }
   if ( $opt{s} ) { # command line option overrides environment variable
      $systemmapfile  = $opt{s};
      print "\tSYSTEMMAPFILE = $Cb$systemmapfile$Cn\n" if $sfdbg;
   }

        # Module Info text file created using "cat /proc/modules" on target
   if ( $ENV{SF_MODULEINFOFILE} ) {
      $moduleinfofile = $ENV{SF_MODULEINFOFILE};
      print "\tENV SF_MODULEINFOFILE = $Cb$moduleinfofile$Cn\n" if $sfdbg;
   }
   if ( $opt{m} ) { # command line option overrides environment variable
      $moduleinfofile = $opt{m};
      print "\tMODULEINFOFILE = $Cb$moduleinfofile$Cn\n" if $sfdbg;
   }

        # Directory containing extra and kernel/net/atm *.ko modules 
        # - use command line option -l <LibraryDirpath>
        # - construct modules directory path from View info
   my $librarydirpath;
   if ( $ENV{SF_LIBRARYDIRPATH} ) {
      $librarydirpath = $ENV{SF_LIBRARYDIRPATH};
      print "\tENV SF_LIBRARYDIRPATH = $Cb$librarydirpath$Cn\n" if $sfdbg;
   }
   if ( $opt{l} ) { # command line option overrides environment variable
      $librarydirpath = $opt{l};
      print "\tLIBRARYDIRPATH = $Cb$librarydirpath$Cn\n" if $sfdbg;
   }
   if ( $librarydirpath ) {
      if ( -d $librarydirpath ) {
         $libraryextradir  = $librarydirpath;
         $librarykerneldir = $librarydirpath;
      }
      else {
         print STDERR "\n\t$Ce WARNING: Invalid modules directory $librarydirpath $Cn\n";
      }
   }
   else {
      my $cclibrarydirpath = $ccviewpath . "/targets/"
                           . $projectname . "/modules/lib/modules/"
                           . $kernelversion;
      print "cclibrarydirpath = $Cb$cclibrarydirpath$Cn\n" if $sfdbg;

      if ( -d $cclibrarydirpath ) {
         $libraryextradir  = $cclibrarydirpath . "/extra/";
         $librarykerneldir = $cclibrarydirpath . "/kernel/";
      }
   }

   if ( $sfdbg ) {
      print "\tExtra  Modules Path: $Cb$libraryextradir$Cn\n" if $libraryextradir;
      print "\tKernel Modules Path: $Cb$librarykerneldir$Cn\n" if $librarykerneldir;
   }

   if ( $ENV{SF_SYMBOLMAPFILE} ) {
      $symbolmapfile  = $ENV{SF_SYMBOLMAPFILE};
      print "\tENV SF_SYMBOLMAPFILE = $Cb$symbolmapfile$Cn\n" if $sfdbg;
   }
   if ( $opt{t} ) {
      $symbolmapfile = $opt{t};
      print "\tSYMBOLMAPFILE = $Cb$symbolmapfile$Cn\n" if $sfdbg;
   }

   if ( $ENV{SF_GENSYMBOLFILE} ) {
      $gensymbolfile = $ENV{SF_GENSYMBOLFILE};
      print "\tENV SF_GENSYMBOLFILE = $Cb$gensymbolfile$Cn\n" if $sfdbg;
   }
   if ( $opt{g} ) {
      $gensymbolfile = $opt{g};
      print "\tGENSYMBOLFILE = $Cb$gensymbolfile$Cn\n" if $sfdbg;
   }

   if ( $opt{e} ) {
      $symRegex = $opt{e};
      print "\tENV SF_REGEX = $Cb$$symRegex$Cn\n" if $sfdbg;
   }

   if ( $ENV{SF_INPUTLOGFILE} ) {
      $inputlogfile = $ENV{SF_INPUTLOGFILE};
      print "\tENV SF_INPUTLOGFILE = $Cb$inputlogfile$Cn\n" if $sfdbg;
   }
   if ( $opt{i} ) {
      $inputlogfile = $opt{i};
      print "\tINPUTLOGFILE = $Cb$inputlogfile$Cn\n" if $sfdbg;
   }

   if ( $ENV{SF_OUTPUTLOGFILE} ) {
      $outputlogfile  = $ENV{SF_OUTPUTLOGFILE};
      print "\tENV SF_OUTPUTLOGFILE = $Cb$outputlogfile$Cn\n" if $sfdbg;
   }
   if ( $opt{o} ) {
      $outputlogfile  = $opt{o};
      print "\tOUTPUTLOGFILE = $Cb$outputlogfile$Cn\n" if $sfdbg;
   }

   if ( ! $symbolmapfile  && ! $systemmapfile )
   {
      print STDERR "\n\t$Ce ERROR: No System.map nor Symbol file specified $Cn\n";
      usage();
   }

   $symCount = 0;
}

# Scan the NM generated file for text [tT] symbols
# Example Format: 80000000 T symbolname
sub scanNmFile() {

   my ($nmFile, $offset) = @_;

   print "\nScanning $Cb$nmFile$Cn\n";
   print "\t ... please wait, applying offset $Cb$offset$Cn ... ";

   open NMFILE, "<$nmFile" or die "\n\t$Ce Cannot open $nmFile : $! $Cn\n";

   if ( $offset eq "00000000" ) {
      while ( <NMFILE> ) {
         if ( /(\w+) +[tT] +(.+)$/ ) { # REGEX

            if ( ! $ignoresymbols{$2} )
            {
               if ( ! $symbolList{$1} )   # Insert only unique elements
               {
                  $symbolList{$1} = $2;
                  @symbolList = ( @symbolList, $1 );

                  $symCount++;
               }
               else   # duplicate symbol name for instruction address
               {
                  $symbolList{$1} = $symbolList{$1} . $multipleSymDelimit . $2;
               }
            }
         }
      }
   }
   else { # module needs relocation
      my ($hexAddr, $addr);

      while ( <NMFILE> ) {
         if ( /(\w+) +[tT] +(.+)$/ ) { # REGEX

            $hexAddr = hex($1) + hex($offset); # relocate to offset
            $addr = sprintf "%x", $hexAddr;

            if ( ! $ignoresymbols{$2} )
            {
               if ( ! $symbolList{$addr} )   # Insert only unique elements
               {
                  $symbolList{$addr} = $2;
                  @symbolList = ( @symbolList, $addr );

                  $symCount++;
               }
               else   # duplicate symbol name for instruction address
               {
                  $symbolList{$addr} = $symbolList{$addr} . $multipleSymDelimit . $2;
               }
            }
         }
      }
   } 

   close NMFILE;

   print "total $Cb$symCount$Cn symbols\n";
}

# Scan the Symbol file generated by sym_find previously
sub scanSymbolFile() {

   print "\nScanning SYM $Cb$symbolmapfile$Cn\n\t ... please wait ... ";

   open SYMMAP, "<$symbolmapfile" or
		die "\n\t$Ce Cannot open $symbolmapfile : $! $Cn\n";

   while ( <SYMMAP> ) {

      if ( /(\w+) +(.+)$/ ) { # REGEX

         if ( ! $symbolList{$1} )   # Insert only unique elements
         {
            $symbolList{$1} = $2;
            @symbolList = ( @symbolList, $1 );

            $symCount++;
         }
      }
   }

   close SYMMAP;

   print "total $Cb$symCount$Cn symbols\n";
 
}

# Write all scanned (resolved) symbols to a symbol file for future use
sub genSymbolFile() {

   my $addr;
   print "\nGenerating SYM $Cb$gensymbolfile$Cn\n\t ... please wait ... "; 

   open SYMFILE, ">$gensymbolfile" or
		die "\n\t$Ce Cannot open $gensymbolfile : $1 $Cn\n";

   foreach $addr ( @symbolList ) {
      print SYMFILE  "$addr $symbolList{$addr}\n";
   }

   close SYMFILE;

   print "total $Cb$symCount$Cn symbols\n";
}

# Get the list of modules and their relocated offset from "cat /proc/modules" file
sub getModuleList() {

   print "\nCollecting KO $Cb$moduleinfofile$Cn\n\t... please wait ... ";

   open MODINFO, "<$moduleinfofile" or die "Cannot open $moduleinfofile : $!\n";

   my $modCount = 0;
   while ( <MODINFO> ) {
      # adsldd 139808 0 - Live 0xc0060000
      # atmapi 56128 3 br2684,blaa_dd,adsldd, Live 0xc0021000

      if ( /(\w+) .* Live 0x(\w+) 0x(\w+)/ ) {
         $moduleList{$1} = $2;
         $moduleInitList{$1} = $3;
         $modCount++;
      }
      elsif ( /(\w+) .* Live 0x(\w+)/ ) { 
         $moduleList{$1} = $2;
         $modCount++;
      }
   }

   close MODINFO;

   print "total $Cb$modCount$Cn KO modules\n";
}

# Locate the module.ko object file in the extra or kernel library directory
sub findModule() {

   my $moduleFileName = '';
   my $module = $_[0] . ".ko";

   if ( ! $libraryextradir && ! $librarykerneldir ) {
      print STDERR "\n\t$Ce WARNING: No library path specified, cannot locate $module $Cn\n";
   }

   if ( $libraryextradir ) {
      $moduleFileName = $libraryextradir . "/" . $module;
      return $moduleFileName if ( -r $moduleFileName );
   }
   if ( $librarykerneldir ) {
      $moduleFileName = $librarykerneldir . "/net/atm/" . $module;
      return $moduleFileName if ( -r $moduleFileName );
      $moduleFileName = $librarykerneldir . "/net/xfrm/" . $module;
      return $moduleFileName if ( -r $moduleFileName );
      $moduleFileName = $librarykerneldir . "/net/netfilter/" . $module;
      return $moduleFileName if ( -r $moduleFileName );
      $moduleFileName = $librarykerneldir . "/net/ipv6/netfilter/" . $module;
      return $moduleFileName if ( -r $moduleFileName );
      $moduleFileName = $librarykerneldir . "/net/ipv4/netfilter/" . $module;
      return $moduleFileName if ( -r $moduleFileName );
      $moduleFileName = $librarykerneldir . "/net/ipv4/netfilter/broadcom/" . $module;
      return $moduleFileName if ( -r $moduleFileName );
   }

   return;
}

sub findSymbol() {
   my ($findAddr, $symbolName, $addr, $prevAddr, $delta, $deltaHex);
   $findAddr = $_[0];
   $symbolName = "@[0x" . $findAddr . "]";
   $prevAddr = 80010000;
   # $prevName = "_text";

   foreach $addr ( @symbolList ) {
      if ( $findAddr lt $addr ) {
         $delta = hex($findAddr) - hex($prevAddr);
         $deltaHex = sprintf "+%x", $delta;
         $symbolName = "@[" . $symbolList{$prevAddr} . $deltaHex . "]";
         last;
      }
      $prevAddr = $addr;
   }

   return $symbolName;
}

#
# Start of main program
#

# Initialize globals, overide by Environment variables or command line options.
&init();

# Scan the "System.map" file generated via "mips-linux-uclibc-nm"
&scanNmFile( $systemmapfile, "00000000" ) if ( $systemmapfile );

$ignoresymbols{"init"} = "init";
$ignoresymbols{"fini"} = "fini";
$ignoresymbols{"initial_table"} = "initial_table";
$ignoresymbols{"nat_initial_table"} = "nat_initial_table";
$ignoresymbols{"init_module"} = "init_module";
$ignoresymbols{"cleanup_module"} ="cleanup_module";

# Get the list of modules specified in "cat proc/modules"
&getModuleList() if ( $moduleinfofile );

# For each loaded module,
# 1. Locate the module in the clearcase view or the directory containing all modules
# 2. run "mips-linux-uclibc-nm" on the module.ko
# 3. scan in the symbols from the output of nm
my $moduleName;
foreach $moduleName ( keys %moduleList ) {
   my $module_ko = &findModule( $moduleName );

   if ( $module_ko ) {
      print "\nProcessing KO $Cb$module_ko$Cn\n";

      system("$nm_mips $module_ko > /tmp/$moduleName");
      &scanNmFile( "/tmp/$moduleName", $moduleList{$moduleName} );
      system("rm /tmp/$moduleName");
   }
}

print "\n";

foreach $moduleName ( keys %moduleInitList ) {
	# This module may have an Init Text Segment
         print "Adding symbol $moduleInitList{$moduleName} = $moduleName.INIT_TEXT_CODE\n";
         $symbolList{ $moduleInitList{$moduleName} } = $moduleName . ".INIT_TEXT_CODE";
         @symbolList = ( @symbolList, $moduleInitList{$moduleName} );
         $symCount++;
}

&scanSymbolFile() if ( $symbolmapfile);

$symbolList{"00400000"} = "userapp";
@symbolList = ( @symbolList, "00400000" );
$symCount++;

# Sort the symbol list array, for searching instruction address with offset
@symbolList = sort(@symbolList);

&genSymbolFile() if ( $gensymbolfile );

exit() if ( ! $inputlogfile ); # no log file to parse

# Open input and output log files
open IN_LOG_FILE, "<$inputlogfile" or die "Cannot open $inputlogfile for read :$!";

if ( $outputlogfile ) {
   open ( OUT_LOG_FILE, ">$outputlogfile") or die "Cannot open $outputlogfile for write :$!";
}
else {
   open ( OUT_LOG_FILE, ">-" ) or die "Cannot open stdout for write : $!";
}

$symResolve = 0;

print "\nProcessing LOG $Cb$inputlogfile$Cn\n\t ... please wait ... ";

while ( <IN_LOG_FILE> ) {

   my $inputline = $_;
   my $symresolve = 0;

   # Format: ".*A[0x800c0a3c].*"
   #            ++++VVVVVVVV+

   # One or more addresses per input line to be resolved
   # while ( $inputline =~ /^(.*)A\[0x(\w+)\](.*)$/ ) { # REGEX
   while ( $inputline =~ /^(.*)$symRegex(.*)$/ ) { # REGEX

      $symresolve = 1;

      my $preamble = $1;
      my $iaddress = $2; 
      my $postamble = $3;

      my $symbolname = &findSymbol( $iaddress );

      if ( $symbolname ) {
         $inputline = $preamble . $symbolname . $postamble;
         $symResolve++;
      }
      else {
         $inputline = $preamble . "@[0x" . $iaddress . "]" . $postamble;
      }
   }

   if ( $symresolve == 1 ) {
      print OUT_LOG_FILE "$inputline\n";
   }
   else {
      print OUT_LOG_FILE "$inputline";
   }
}

close IN_LOG_FILE;
close OUT_LOG_FILE;

print "Resolved $Cb$symResolve$Cn addresses\n";

