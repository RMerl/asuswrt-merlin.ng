#! /usr/bin/env perl

use strict;
use warnings;
use Data::Dumper;
use Getopt::Long;

my $usage = q[

refresh_options.pl options_file > new_options_file

Any comments and options from the options_template.txt will be copied
to the output in order EXCEPT that all options specified ONLY in the 
template file will be commented out with #- marks.

Any defintions in the options file will replace the corresponding line
from the options_template file.

Any commented out defintions using only a single # followed by whitespace
in the options file will replace the corresponding line from the 
options_template file, single # and all.

Any comments in the options file beginning with ## will be copied to the
new options file either at the top, in a named [section], or stick
with the options definition that follows them.

];


my $overwrite;

GetOptions("overwrite",\$overwrite) or die($usage);

my $target = pop @ARGV;
if (!$target) {
    print $usage;
    exit 1; 
}
my @sources = ( @ARGV );

my %sections;
my %vars;

foreach my $src (@sources, $target) {
    my $section = "top";
    my $in_header = 1;
    my @nextvar = ();
    open(SRC,"<", $src);
    while (my $l = <SRC>) {
        chomp $l;
        if ($l =~ /^##/) {
            if ($in_header) {
                push @{$sections{$section}}, $l;
            } else {
                push @nextvar, $l; 
            }
        } elsif ($l =~ /^#-\s*\[(\w+)\]/) {
            $section = $1;
            $in_header = 1;
        } elsif ($l =~ /^#-/) {
            # ignore anything else with #- that doesnt have a section tag
        } elsif ($l =~ /^#?\s*(\w+)\s*:?=/) {
            $vars{$1} = { line => $l, precomment => [ @nextvar ] };
            @nextvar = ();
        } elsif ($l =~ /^\s*$/) {
            $in_header = 0;
        }
    } 

}

# print Dumper(\%sections,\%vars);

open(T,"<","options_template.txt");
if ($overwrite) {
    my $perm = (stat $target)[2] & 07777;
    chmod($perm | 0600, $target);
    open(OF, ">", $target);
} else {
    open(OF, ">&", STDOUT);
}

my $state = "header";
my $section;
while (my $t = <T>) {
    chomp $t;
    if ($t =~ /^#-\s*\[(\w+)\]/) {
            $section = $1;
            $state = "header";
            print OF "$t\n";
    } elsif (($state eq "header") && ($t =~ /^#-\s*\S/)) {
            print OF "$t\n";
    } elsif (($state eq "header") && ($t =~ /^#-/)) {
            print OF "$t\n";
            if ($sections{$section}) { 
                print OF join("\n",@{$sections{$section}}) . "\n";
            }
            $state = "body";
    } elsif ($state eq "header") {
            print OF "$t\n";
    } elsif (($state eq "body") && ($t =~ /^#?\s*(\w+)\s*:?=/)) {
        if ($vars{$1}) {
            print OF join("\n",(@{$vars{$1}->{precomment}},$vars{$1}->{line})) . "\n";
        } else {
            if ($t =~ /^#/) {
                print OF "$t\n";
            } else {
                print OF "#- $t\n";
            }
        }
    } else {
        print OF "$t\n";
    }
}

