#!/usr/bin/env perl -w

use Digest::MD5;

$num_args = $#ARGV + 1;
if ($num_args != 1) {
    print "\nUsage: perlmd5 filename\n";
    exit;
}

$filename=$ARGV[0];
open(my $fh, "<", $filename)
    or die "cannot open $filename $!";

$ctx = Digest::MD5->new;
$ctx->addfile($fh);

print lc $ctx->hexdigest . "\n";
