#! /usr/bin/perl -w
use strict;
open(my $out, '>:raw', 'ivt.bin') or die "Unable to open: $!";
print $out pack("V", 0x412000D1); # Signature
print $out pack("V", 0x80800000); # Load Address (*load_address)
print $out pack("V", 0x0); # Reserved
print $out pack("V", 0x0); # DCD pointer
print $out pack("V", 0x0); # Boot Data
print $out pack("V", 0x80EEA000); # Self Pointer (*ivt)
print $out pack("V", 0x80EEA020); # CSF Pointer (*csf)
print $out pack("V", 0x0); # Reserved
close($out);
