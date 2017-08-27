#!/usr/bin/env perl
#
#  Sort the attributes in a dictionary, and put them into a canonical
#  form.  This will DESTROY any comments!
#
#  Usage: cat dictionary | ./attrsort.pl > new
#
#  This is a bit of a hack.  The main purpose is to be able to quickly
#  "diff" two dictionaries which have significant differences...
#
#  $Id$
#

while (<>) {
    #
    #  Get attribute.
    #
    if (/^ATTRIBUTE\s+([\w-]+)\s+(\w+)\s+(\w+)(.*)/) {
	$name=$1;
	$value = $2;
	$type = $3;
	$stuff = $4;

	$value =~ tr/[A-F]/[a-f]/; # normal form for hex
	$value =~ tr/X/x/;

	if ($value =~ /^0x/) {
	    $index = hex $value;
	} else {
	    $index = $value;
	}

	$attributes{$index} = "$name $value $type$stuff";
	$name2val{$name} = $index;
	next;
    }

    #
    #  Values.
    #
    if (/^VALUE\s+([\w-]+)\s+([\w-\/,.]+)\s+(\w+)(.*)/) {
	$attr = $1;
	$name = $2;
	$value = $3;
	$stuff = $d;

	$value =~ tr/[A-F]/[a-f]/; # normal form for hex
	$value =~ tr/X/x/;

	if ($value =~ /^0x/) {
	    $index = hex $value;
	} else {
	    $index = $value;
	}

	if (!defined $name2val{$attr}) {
	    print "# FIXME: FORWARD REF?\nVALUE $attr $name $value$stuff\n";
	    next;
	}

	$values{$name2val{$attr}}{$index} = "$attr $name $value$stuff";
	next;
    }
}

#
#  Print out the attributes sorted by number.
#
foreach $attr_val (sort {$a <=> $b} keys %attributes) {
    print "ATTRIBUTE ", $attributes{$attr_val}, "\n";
}

foreach $value (sort {$a <=> $b} keys %values) {
    print $value, "\t", $attributes{$value}, "\n";
}

#
#  And again, this time printing out values.
#
foreach $attr_val (sort {$a <=> $b} keys %attributes) {

    next if (!defined %{$values{$attr_val}});

    foreach $value (sort {$a <=> $b} keys %{$values{$attr_val}}) {
	print "VALUE ", $values{$attr_val}{$value}, "\n";
    }
}
