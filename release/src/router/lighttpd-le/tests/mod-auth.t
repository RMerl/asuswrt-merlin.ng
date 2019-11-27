#!/usr/bin/env perl
BEGIN {
	# add current source dir to the include-path
	# we need this for make distcheck
	(my $srcdir = $0) =~ s,/[^/]+$,/,;
	unshift @INC, $srcdir;
}

use strict;
use IO::Socket;
use Test::More tests => 20;
use LightyTest;

my $tf = LightyTest->new();
my $t;

ok($tf->start_proc == 0, "Starting lighttpd") or die();

$t->{REQUEST}  = ( <<EOF
GET /server-status HTTP/1.0
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 401 } ];
ok($tf->handle_http($t) == 0, 'Missing Auth-token');

$t->{REQUEST}  = ( <<EOF
GET /server-config HTTP/1.0
Authorization: Basic \x80mFuOmphb
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 400 } ];
ok($tf->handle_http($t) == 0, 'Basic-Auth: Invalid base64 Auth-token');

$t->{REQUEST}  = ( <<EOF
GET /server-config HTTP/1.0
Authorization: Basic bm90Oml0Cg==
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 401 } ];
ok($tf->handle_http($t) == 0, 'Basic-Auth: Wrong Auth-token');

$t->{REQUEST}  = ( <<EOF
GET /server-config HTTP/1.0
Authorization: Basic amFuOmphbg==
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 200 } ];
ok($tf->handle_http($t) == 0, 'Basic-Auth: Valid Auth-token - plain');

$t->{REQUEST}  = ( <<EOF
GET /server-config HTTP/1.0
Host: auth-htpasswd.example.org
Authorization: Basic ZGVzOmRlcw==
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 200 } ];
ok($tf->handle_http($t) == 0, 'Basic-Auth: Valid Auth-token - htpasswd (des)');

$t->{REQUEST}  = ( <<EOF
GET /server-config HTTP/1.0
Host: auth-htpasswd.example.org
Authorization: basic ZGVzOmRlcw==
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 200 } ];
ok($tf->handle_http($t) == 0, 'Basic-Auth: Valid Auth-token - htpasswd (des) (lowercase)');

$t->{REQUEST}  = ( <<EOF
GET /server-config HTTP/1.0
Host: auth-htpasswd.example.org
Authorization: Basic c2hhOnNoYQ==
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 200 } ];
ok($tf->handle_http($t) == 0, 'Basic-Auth: Valid Auth-token - htpasswd (sha)');

$t->{REQUEST}  = ( <<EOF
GET /server-config HTTP/1.0
Host: auth-htpasswd.example.org
Authorization: Basic c2hhOnNoYg==
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 401 } ];
ok($tf->handle_http($t) == 0, 'Basic-Auth: Valid Auth-token - htpasswd (sha, wrong password)');

$t->{REQUEST}  = ( <<EOF
GET /server-config HTTP/1.0
Host: auth-htpasswd.example.org
Authorization: Basic YXByLW1kNTphcHItbWQ1
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 200 } ];
ok($tf->handle_http($t) == 0, 'Basic-Auth: Valid Auth-token - htpasswd (apr-md5)');

$t->{REQUEST}  = ( <<EOF
GET /server-config HTTP/1.0
Host: auth-htpasswd.example.org
Authorization: Basic YXByLW1kNTphcHItbWQ2
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 401 } ];
ok($tf->handle_http($t) == 0, 'Basic-Auth: Valid Auth-token - htpasswd (apr-md5, wrong password)');

SKIP: {
	skip "no crypt-md5 under cygwin", 1 if $^O eq 'cygwin';
	skip "no crypt-md5 under darwin", 1 if $^O eq 'darwin';
$t->{REQUEST}  = ( <<EOF
GET /server-config HTTP/1.0
Host: auth-htpasswd.example.org
Authorization: Basic bWQ1Om1kNQ==
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 200 } ];
ok($tf->handle_http($t) == 0, 'Basic-Auth: Valid Auth-token - htpasswd (crypt-md5)');
}

$t->{REQUEST}  = ( <<EOF
GET /server-config HTTP/1.0
Authorization: Basic bWQ1Om1kNA==
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 401 } ];
ok($tf->handle_http($t) == 0, 'Basic-Auth: Valid Auth-token');

## this should not crash
$t->{REQUEST}  = ( <<EOF
GET /server-status HTTP/1.0
User-Agent: Wget/1.9.1
Authorization: Digest username="jan", realm="jan", nonce="9a5428ccc05b086a08d918e73b01fc6f",
                uri="/server-status", response="ea5f7d9a30b8b762f9610ccb87dea74f"
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 401 } ];
ok($tf->handle_http($t) == 0, 'Digest-Auth: missing qop, no crash');

# (Note: test case is invalid; mismatch between request line and uri="..."
#  is not what is intended to be tested here, but that is what is invalid)
# https://redmine.lighttpd.net/issues/477
## this should not crash
$t->{REQUEST}  = ( <<EOF
GET /server-status HTTP/1.0
User-Agent: Wget/1.9.1
Authorization: Digest username="jan", realm="jan",
	nonce="b1d12348b4620437c43dd61c50ae4639",
	uri="/MJ-BONG.xm.mpc", qop=auth, noncecount=00000001",
	cnonce="036FCA5B86F7E7C4965C7F9B8FE714B7",
	response="29B32C2953C763C6D033C8A49983B87E"
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 400 } ];
ok($tf->handle_http($t) == 0, 'Digest-Auth: missing nc (noncecount instead), no crash');

$t->{REQUEST}  = ( <<EOF
GET /server-config HTTP/1.0
Authorization: Basic =
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 400 } ];
ok($tf->handle_http($t) == 0, 'Basic-Auth: Invalid Base64');

$t->{REQUEST}  = ( <<EOF
GET /server-status HTTP/1.0
Authorization: Digest username="jan", realm="download archiv",
	nonce="b3b26457000000003a9b34a3cd56d26e48a52a498ac9765d4b",
	uri="/server-status", qop=auth, nc=00000001,
	algorithm="md5-sess", response="049b000fb00ab51dddea6f093a96aa2e"
EOF
 );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 400 } ];
ok($tf->handle_http($t) == 0, 'Digest-Auth: md5-sess + missing cnonce');

 $t->{REQUEST}  = ( <<EOF
GET /server-status HTTP/1.0
Authorization: Digest username="jan", realm="download archiv",
	nonce="b3b26457000000003a9b34a3cd56d26e48a52a498ac9765d4b",
	uri="/server-status", qop=auth, nc=00000001, cnonce="65ee1b37",
	algorithm="md5", response="049b000fb00ab51dddea6f093a96aa2e"
EOF
  );
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 401, 'WWW-Authenticate' => '/, stale=true$/' } ];
ok($tf->handle_http($t) == 0, 'Digest-Auth: stale nonce');

$t->{REQUEST}  = ( <<EOF
GET /server-status HTTP/1.0
Authorization: Digest username="jan", realm="download archiv",
	nonce="b3b26457000000003a9b34a3cd56d26e48a52a498ac9765d4b",
	uri="/server-status", qop=auth, nc=00000001, cnonce="65ee1b37",
	algorithm="md5", response="049b000fb00ab51dddea6f093a96aa2e"     
EOF
 ); # note: trailing whitespace at end of request line above is intentional
$t->{RESPONSE} = [ { 'HTTP-Protocol' => 'HTTP/1.0', 'HTTP-Status' => 401, 'WWW-Authenticate' => '/, stale=true$/' } ];
ok($tf->handle_http($t) == 0, 'Digest-Auth: trailing WS, stale nonce');



ok($tf->stop_proc == 0, "Stopping lighttpd");

