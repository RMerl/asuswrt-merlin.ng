#!/usr/bin/env perl

print "Content-Type: text/html\r\n\r\n";

print $ENV{"PATH_INFO"};

0;
