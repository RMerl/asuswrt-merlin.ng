#!/usr/bin/env perl
#
#  Format the dictionaries according to a standard scheme.
#
#  Usage: cat dictionary | ./format.pl > new
#
#  We don't over-write the dictionaries in place, so that the process
#  can be double-checked by hand.
#
#  This is a bit of a hack.
#
#  FIXME: get lengths from variables, rather than hard-coding.
#
######################################################################
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#
#    Copyright (C) 2010 Alan DeKok <aland@freeradius.org>
#
######################################################################
#
#  $Id$
#

$begin_vendor = 0;
$blank = 0;

while (@ARGV) {
    $filename = shift;

    open FILE, "<$filename" or die "Failed to open $filename: $!\n";

    @output = ();

    while (<FILE>) {
	#
	#  Clear out trailing whitespace
	#
	s/[ \t]+$//;

	#
	#  And CR's
	#
	s/\r//g;

	#
	#  Suppress multiple blank lines
	#
	if (/^\s+$/) {
	    next if ($blank == 1);
	    $blank = 1;
	    push @output, "\n";
	    next;
	}
	$blank = 0;

	s/\s*$/\n/;

	#
	#  Remember the vendor
	#
	if (/^VENDOR\s+([\w-]+)\s+(\w+)(.*)/) {
	    $name=$1;
	    $len = length $name;
	    if ($len < 32) {
		$lenx = 32 - $len;
		$lenx += 7;		# round up
		$lenx /= 8;
		$lenx = int $lenx;
		$tabs = "\t" x $lenx;
	    } else {
		$tabs = " ";
	    }
	    push @output, "VENDOR\t\t$name$tabs$2$3\n";
	    $vendor = $name;
	    next;
	}

	#
	#  Remember if we did begin-vendor.
	#
	if (/^BEGIN-VENDOR\s+([\w-]+)/) {
	    $begin_vendor = 1;
	    if (!defined $vendor) {
		$vendor = $1;
	    } elsif ($vendor ne $1) {
		# do something smart
	    }

	    push @output, "BEGIN-VENDOR\t$vendor\n";
	    next;
	}

	#
	#  Get attribute.
	#
	if (/^ATTRIBUTE\s+([\w-]+)\s+([\w.]+)\s+(\w+)(.*)/) {
	    $name=$1;
	    $len = length $name;
	    if ($len < 40) {
		$lenx = 40 - $len;
		$lenx += 7;		# round up
		$lenx /= 8;
		$lenx = int $lenx;
		$tabs = "\t" x $lenx;
		if ($tabs eq "") {
		    $tabs = " ";
		}
	    } else {
		$tabs = " ";
	    }

	    $value = $2;
	    $type = $3;
	    $stuff = $4;

	    #
	    #  See if it's old format, with the vendor at the end of
	    #  the line.  If so, make it the new format.
	    #
	    if ($stuff =~ /$vendor/) {
		if ($begin_vendor == 0) {
		    push @output, "BEGIN-VENDOR\t$vendor\n\n";
		    $begin_vendor = 1;
		}
		$stuff =~ s/$vendor//;
		$stuff =~ s/\s+$//;
	    }

	    push @output, "ATTRIBUTE\t$name$tabs$value\t$type$stuff\n";
	    next;
	}

	#
	#  Values.
	#
	if (/^VALUE\s+([\w-]+)\s+([\w-\/,.]+)\s+(\w+)(.*)/) {
	    $attr=$1;
	    $len = length $attr;
	    if ($len < 32) {
		$lenx = 32 - $len;
		$lenx += 7;		# round up
		$lenx /= 8;
		$lenx = int $lenx;
		$tabsa = "\t" x $lenx;
		if ($tabsa eq "") {
		    $tabsa = " ";
		    $len += 1;
		} else {
		    $len -= $len % 8;
		    $len += 8 * length $tabsa;
		}
	    } else {
		$tabsa = " ";
		$len += 1;
	    }

	    #
	    #  For the code below, we assume that the attribute lengths
	    #
	    if ($len < 32) {
		$lena = 0;
	    } else {
		$lena = $len - 32;
	    }

	    $name = $2;
	    $len = length $name;
	    if ($len < 24) {
		$lenx = 24 - $lena - $len;
		$lenx += 7;		# round up
		$lenx /= 8;
		$lenx = int $lenx;
		$tabsn = "\t" x $lenx;
		if ($tabsn eq "") {
		    $tabsn = " ";
		}
	    } else {
		$tabsn = " ";
	    }

	    push @output, "VALUE\t$attr$tabsa$name$tabsn$3$4\n";
	    next;
	}

	#
	#  Remember if we did this.
	#
	if (/^END-VENDOR/) {
	    $begin_vendor = 0;
	}

	#
	#  Everything else gets dumped out as-is.
	#
	push @output, $_;
    }

#
#  If we changed the format, print the end vendor, too.
#
    if ($begin_vendor) {
	push @output, "\nEND-VENDOR\t$vendor\n";
    }

    close FILE;

    open FILE, ">$filename" or die "Failed to open $filename: $!\n";

    print FILE @output;

    close FILE;
}
