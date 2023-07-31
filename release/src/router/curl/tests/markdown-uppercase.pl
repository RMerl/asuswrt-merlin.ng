#!/usr/bin/env perl
#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at https://curl.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
# SPDX-License-Identifier: curl
#
###########################################################################

my $root=$ARGV[0] || "..";

my @m = `git ls-files -- $root`;

my $errors;

my %accepted=('curl' => 1,
              'libcurl' => 1);

sub checkfile {
    my ($f) = @_;
    chomp $f;
    if($f !~ /\.md\z/) {
        return;
    }
    open(my $fh, "<", "$f");
    my $l = 1;
    my $prevl;
    my $ignore = 0;
    while(<$fh>) {
        my $line = $_;
        chomp $line;
        if($line =~ /^(\`\`\`|\~\~\~)/) {
            # start or stop ignore-mode
            $ignore ^= 1;
        }
        if(!$ignore) {
            if(($prevl =~ /\.\z/) && ($line =~ /^( *)([a-z]+)/)) {
                my ($prefix, $word) = ($1, $2);
                if(!$accepted{$word}) {
                    my $c = length($prefix);
                    print STDERR
                        "$f:$l:$c:error: lowercase $word after period\n";
                    print STDERR "$line\n";
                    print STDERR ' ' x $c;
                    print STDERR "^\n";
                    $errors++;
                }
            }
            elsif($line =~ /^(.*)\. +([a-z]+)/) {
                my ($prefix, $word) = ($1, $2);

                if(($prefix =~ /\.\.\z/) ||
                   ($prefix =~ /[0-9]\z/) ||
                   ($prefix =~ /e.g\z/) ||
                   ($prefix =~ /i.e\z/) ||
                   ($prefix =~ /etc\z/) ||
                   $accepted{$word}) {
                }
                else {
                    my $c = length($prefix) + 2;
                    print STDERR
                        "$f:$l:$c:error: lowercase $word after period\n";
                    print STDERR "$line\n";
                    print STDERR ' ' x $c;
                    print STDERR "^\n";
                    $errors++;
                }
            }
        }
        $prevl = $line;
        $l++;
    }
    close($fh);
}


for my $f (@m) {
    checkfile($f);
}

if($errors) {
    exit 1;
}
print "ok\n";
