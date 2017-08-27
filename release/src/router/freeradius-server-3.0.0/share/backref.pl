#!/usr/bin/env perl
#
#  Cross-reference RFC attributes.
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
	    next;
	}
	$blank = 0;

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

	    next;
	}

	#
	#  Get attribute.
	#
	if (/^ATTRIBUTE\s+([\w-]+)\s+(\w+)\s+(\w+)(.*)/) {
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

	    if ($begin_vendor == 0) {
		#
		#  FIXME: Catch and print conflicting attributes.
		#
		$file{$value} = $filename;
		$file{$value} =~ s/dictionary\.//;
		$name{$value} = $name . $tabs;
	    }

	    #
	    #  See if it's old format, with the vendor at the end of
	    #  the line.  If so, make it the new format.
	    #
	    if ($stuff =~ /$vendor/) {
		if ($begin_vendor == 0) {
		    $begin_vendor = 1;
		}
		$stuff =~ s/$vendor//;
		$stuff =~ s/\s+$//;
	    }

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
    }

    close FILE;

}

#
#  Print out the attributes.
#
foreach $attr (sort {$a <=> $b} keys %file) {
    print $name{$attr}, $attr, "\t", $file{$attr}, "\n";
}

