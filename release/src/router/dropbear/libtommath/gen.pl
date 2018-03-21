#!/usr/bin/perl -w
#
# Generates a "single file" you can use to quickly
# add the whole source without any makefile troubles
#
use strict;
use warnings;

open(my $out, '>', 'mpi.c') or die "Couldn't open mpi.c for writing: $!";
foreach my $filename (glob 'bn*.c') {
   open(my $src, '<', $filename) or die "Couldn't open $filename for reading: $!";
   print {$out} "/* Start: $filename */\n";
   print {$out} $_ while <$src>;
   print {$out} "\n/* End: $filename */\n\n";
   close $src or die "Error closing $filename after reading: $!";
}
print {$out} "\n/* EOF */\n";
close $out or die "Error closing mpi.c after writing: $!";

system('perl -pli -e "s/\s*$//" mpi.c');
