#!/usr/bin/perl

use strict;
use warnings;

use HTTP::Daemon;
use HTTP::Status;
use HTTP::Response;

my $d = new HTTP::Daemon(LocalAddr => '127.0.0.1', LocalPort => 9090);
print "Please contact me at: <URL:", $d->url, ">\n";
while (my $c = $d->accept) {
	while (my $r = $c->get_request) {
		print "Got " . $r->method . " request\n";
		if ($r->method eq 'POST' and $r->url->path eq "/") {
			my $resp = HTTP::Response->new( '200', 'OK' );
	
			$resp->header("Content-Type" => "application/x-www-form-urlencoded");	
			$resp->content("control:Cleartext-Password=password&reply:Reply-Message=testing123");
		
			$c->send_response($resp);
		} else {
			$c->send_error(RC_FORBIDDEN)
		}
 	}
      	$c->close;
	undef($c);
 }
