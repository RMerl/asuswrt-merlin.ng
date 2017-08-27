#!/usr/bin/env perl
foreach $file (@ARGV) {
    open FILE, "<$file" || die "Error opening $file: $!\n";

    $ref = $file;
    $ref =~ s/\..*//g;

    while (<FILE>) {
	next if (!/^(\d+\.)+\s+([a-zA-Z]+-)+[a-zA-Z]/);

	chop;
	split;

	next if ($_[1] =~ /,/);

	next if (defined($file{$_[1]}));

	print $ref, "\t", $_[1], "\n";

	$file{$_[1]} = $ref;
    }

    close FILE;
}
