#!/usr/bin/perl

open (List, "gmarshal.list");

while (<List>) {
    next unless /^[A-Z]/;
    s/^/"g_cclosure_marshal_/; s/:/__/; s/,/_/g; s/$/",/;
    print;
}
