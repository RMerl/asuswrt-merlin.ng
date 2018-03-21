#!/usr/bin/perl

# we want to filter every between START_INS and END_INS out and then insert crap from another file (this is fun)

use strict;
use warnings;

open(my $src, '<', shift);
open(my $ins, '<', shift);
open(my $tmp, '>', 'tmp.delme');

my $l = 0;
while (<$src>) {
   if ($_ =~ /START_INS/) {
      print {$tmp} $_;
      $l = 1;
      while (<$ins>) {
         print {$tmp} $_;
      }
      close $ins;
   } elsif ($_ =~ /END_INS/) {
      print {$tmp} $_;
      $l = 0;
   } elsif ($l == 0) {
      print {$tmp} $_;
   }
}

close $tmp;
close $src;

# ref:         $Format:%D$
# git commit:  $Format:%H$
# commit time: $Format:%ai$
