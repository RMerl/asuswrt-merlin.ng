#!/usr/bin/env perl
################################################################################
# (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
#     http://www.softwarestudio.org
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of either:
#
#   The LGPL as published by the Free Software Foundation, version
#   2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.txt
#
# Or:
#
#   The Mozilla Public License Version 1.0. You may obtain a copy of
#   the License at http://www.mozilla.org/MPL/
################################################################################

use Getopt::Std;
getopts('i:');

# the argument should be the path to the restriction datafile, usually
# design-data/restrictions.csv
open(F, "$ARGV[0]") || die "Can't open restriction data file $ARGV[0]:$!";

# Write the file inline by copying everything before a demarcation
# line, and putting the generated data after the demarcation

if ($opt_i) {

  open(IN, $opt_i) || die "Can't open input file $opt_i";

  while (<IN>) {

    if (/<insert_code_here>/) {
      insert_code();
    }

    if (/Do not edit/) {
      last;
    }

    print;

  }

  close IN;
}

sub insert_code
{
  # First build the property restriction table
  print "static const icalrestriction_property_record icalrestriction_property_records[] = {\n";

  while (<F>) {

    chop;

    s/\#.*$//;

    my ($method, $targetcomp, $prop, $subcomp, $restr, $sub) = split(/,/, $_);

    next if !$method;

    if (!$sub) {
      $sub = "NULL";
    } else {
      $sub = "icalrestriction_" . $sub;
    }

    if ($prop ne "NONE") {
      print(
"    \{ICAL_METHOD_${method},ICAL_${targetcomp}_COMPONENT,ICAL_${prop}_PROPERTY,ICAL_RESTRICTION_${restr},$sub},\n"
      );
    }

  }

  # Print the terminating line
  print "    {ICAL_METHOD_NONE,ICAL_NO_COMPONENT,ICAL_NO_PROPERTY,ICAL_RESTRICTION_NONE,NULL}\n";

  print "};\n";

}
