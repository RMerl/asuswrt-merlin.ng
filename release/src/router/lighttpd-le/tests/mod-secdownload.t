#!/usr/bin/env perl
BEGIN {
	# add current source dir to the include-path
	# we need this for make distcheck
	(my $srcdir = $0) =~ s,/[^/]+$,/,;
	unshift @INC, $srcdir;
}

use strict;
use IO::Socket;
use Test::More tests => 15;
use LightyTest;
use Digest::MD5 qw(md5_hex);
use Digest::SHA qw(hmac_sha1 hmac_sha256);
use MIME::Base64 qw(encode_base64url);

my $tf = LightyTest->new();
my $t;

ok($tf->start_proc == 0, "Starting lighttpd") or die();

my $secret = "verysecret";
my ($f, $thex, $m);

$t->{REQUEST}  = ( <<EOF
GET /index.html HTTP/1.0
Host: www.example.org
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 200 } ];

ok($tf->handle_http($t) == 0, 'skipping secdownload - direct access');

## MD5
$f = "/index.html";
$thex = sprintf("%08x", time);
$m = md5_hex($secret.$f.$thex);

$t->{REQUEST}  = ( <<EOF
GET /sec/$m/$thex$f HTTP/1.0
Host: vvv.example.org
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 200 } ];

ok($tf->handle_http($t) == 0, 'secdownload (md5)');

$thex = sprintf("%08x", time - 1800);
$m = md5_hex($secret.$f.$thex);

$t->{REQUEST}  = ( <<EOF
GET /sec/$m/$thex$f HTTP/1.0
Host: vvv.example.org
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 410 } ];

ok($tf->handle_http($t) == 0, 'secdownload - gone (timeout) (md5)');

$t->{REQUEST}  = ( <<EOF
GET /sec$f HTTP/1.0
Host: vvv.example.org
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 404 } ];

ok($tf->handle_http($t) == 0, 'secdownload - direct access (md5)');

$f = "/noexists";
$thex = sprintf("%08x", time);
$m = md5_hex($secret.$f.$thex);

$t->{REQUEST}  = ( <<EOF
GET /sec/$m/$thex$f HTTP/1.0
Host: vvv.example.org
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 404 } ];

ok($tf->handle_http($t) == 0, 'secdownload - timeout (md5)');

## HMAC-SHA1
$f = "/index.html";
$thex = sprintf("%08x", time);
$m = encode_base64url(hmac_sha1("/$thex$f", $secret));

$t->{REQUEST}  = ( <<EOF
GET /sec/$m/$thex$f HTTP/1.0
Host: vvv-sha1.example.org
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 200 } ];

ok($tf->handle_http($t) == 0, 'secdownload (hmac-sha1)');

$thex = sprintf("%08x", time - 1800);
$m = encode_base64url(hmac_sha1("/$thex$f", $secret));

$t->{REQUEST}  = ( <<EOF
GET /sec/$m/$thex$f HTTP/1.0
Host: vvv-sha1.example.org
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 410 } ];

ok($tf->handle_http($t) == 0, 'secdownload - gone (timeout) (hmac-sha1)');

$t->{REQUEST}  = ( <<EOF
GET /sec$f HTTP/1.0
Host: vvv-sha1.example.org
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 404 } ];

ok($tf->handle_http($t) == 0, 'secdownload - direct access (hmac-sha1)');


$f = "/noexists";
$thex = sprintf("%08x", time);
$m = encode_base64url(hmac_sha1("/$thex$f", $secret));

$t->{REQUEST}  = ( <<EOF
GET /sec/$m/$thex$f HTTP/1.0
Host: vvv-sha1.example.org
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 404 } ];

ok($tf->handle_http($t) == 0, 'secdownload - timeout (hmac-sha1)');

## HMAC-SHA256
$f = "/index.html";
$thex = sprintf("%08x", time);
$m = encode_base64url(hmac_sha256("/$thex$f", $secret));

$t->{REQUEST}  = ( <<EOF
GET /sec/$m/$thex$f HTTP/1.0
Host: vvv-sha256.example.org
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 200 } ];

ok($tf->handle_http($t) == 0, 'secdownload (hmac-sha256)');

$thex = sprintf("%08x", time - 1800);
$m = encode_base64url(hmac_sha256("/$thex$f", $secret));

$t->{REQUEST}  = ( <<EOF
GET /sec/$m/$thex$f HTTP/1.0
Host: vvv-sha256.example.org
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 410 } ];

ok($tf->handle_http($t) == 0, 'secdownload - gone (timeout) (hmac-sha256)');

$t->{REQUEST}  = ( <<EOF
GET /sec$f HTTP/1.0
Host: vvv-sha256.example.org
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 404 } ];

ok($tf->handle_http($t) == 0, 'secdownload - direct access (hmac-sha256)');


$f = "/noexists";
$thex = sprintf("%08x", time);
$m = encode_base64url(hmac_sha256("/$thex$f", $secret));

$t->{REQUEST}  = ( <<EOF
GET /sec/$m/$thex$f HTTP/1.0
Host: vvv-sha256.example.org
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 404 } ];

ok($tf->handle_http($t) == 0, 'secdownload - timeout (hmac-sha256)');

## THE END

ok($tf->stop_proc == 0, "Stopping lighttpd");

