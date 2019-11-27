#!/usr/bin/env perl

if ($ENV{"QUERY_STRING"} eq "internal-redir") {
    print "Location: /cgi-pathinfo.pl/foo\r\n\r\n";
    exit 0;
}

print "Content-Type: text/html\r\n\r\n";

print $ENV{"SCRIPT_NAME"};

0;
