#!/usr/bin/env perl
print "Content-Type: text/html\r\n\r\n";
print $ENV{'REMOTE_ADDR'};

if ($ENV{'QUERY_STRING'} eq 'info') {
	print "\nF:",$ENV{'HTTP_X_FORWARDED_FOR'},"\n";

	while (my($key, $value) = each %ENV) {
		printf "%s => %s\n", $key, $value;
	}
}

0;
