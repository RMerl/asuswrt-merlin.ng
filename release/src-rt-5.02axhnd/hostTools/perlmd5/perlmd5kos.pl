#!/usr/bin/perl -w
use Digest::MD5;

if (@ARGV < 1) {
    print STDERR "Usage: perlmd5kos dir [dir ...]\n";
    exit 1;
}

open FIND,"find @ARGV -name \*.ko |";
while (<FIND>) {
        chomp;
        open(my $fh,'<',$_) or warn "open $_: $!\n" and next;
        my $mtime = (stat($fh))[9];
        my ($ss,$mm,$hh,$DD,$MM,$YY,undef,undef,undef) = gmtime($mtime);
        printf "%s %s %4d-%02d-%02d %02d:%02d:%02d\n",
                $_,Digest::MD5->new->addfile($fh)->hexdigest,
                1900+$YY,1+$MM,$DD,$hh,$mm,$ss;
}

