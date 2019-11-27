#!/usr/bin/env perl
BEGIN {
    # add current source dir to the include-path
    # we need this for make distcheck
   (my $srcdir = $0) =~ s#/[^/]+$#/#;
   unshift @INC, $srcdir;
}

use strict;
use IO::Socket;
use Test::More tests => 7;
use LightyTest;

my $tf = LightyTest->new();
$tf->{CONFIGFILE} = 'mod-evhost.conf';
my $t;

ok($tf->start_proc == 0, "Starting lighttpd") or die();

# test for correct config
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 200 } ];
$t->{REQUEST}  = ( <<EOF
GET /index.html HTTP/1.0
Host: evhost1.example.org
EOF
 );
ok($tf->handle_http($t) == 0, 'correct pattern using dot notation');

$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 200 } ];
$t->{REQUEST}  = ( <<EOF
GET /index.html HTTP/1.0
Host: evhost2.example.org
EOF
 );
ok($tf->handle_http($t) == 0, 'correct pattern not using dot notation');

# test for broken config
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 404 } ];
$t->{REQUEST}  = ( <<EOF
GET /index.html HTTP/1.0
Host: evhost3.example.org
EOF
 );
ok($tf->handle_http($t) == 0, 'broken pattern 1');

$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 404 } ];
$t->{REQUEST}  = ( <<EOF
GET /index.html HTTP/1.0
Host: evhost4.example.org
EOF
 );
ok($tf->handle_http($t) == 0, 'broken pattern 2');

$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 404 } ];
$t->{REQUEST}  = ( <<EOF
GET /index.html HTTP/1.0
Host: evhost5.example.org
EOF
 );
ok($tf->handle_http($t) == 0, 'broken pattern 3');

ok($tf->stop_proc == 0, "Stopping lighttpd");

