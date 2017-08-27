#!/usr/bin/env perl

#
#   Read in the references, and put into an associative array
#
open FILE, "<refs" || die "Error opening refs: $!\n";
while (<FILE>) {
    chop;
    split;

    $refs{$_[1]} = $_[0];
}
close FILE;

#
#  now loop over the input RFC's.
#
foreach $file (@ARGV) {
    open FILE, "<$file" || die "Error opening $file: $!\n";

    $attribute = "zzzzz";

    # get the current reference
    $ref = $file;
    $ref =~ s/\..*//g;

    open OUTPUT, ">$ref.html" || die "Error creating $ref.html: $!\n";

    #
    #  Print out the HTML header
    #
    print OUTPUT <<EOF;
<!doctype html public "-//w3c//dtd html 4.0 transitional//en">
<HTML>
<head>
   <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
   <meta name="GENERATOR" content="Perl">
   <title>$ref.html</title>
</head>
<body>
<PRE>

EOF

    #  loop over the input file
    while (<FILE>) {
	# html-ize it
	s/&/&amp;/g;
	s/</&lt;/g;
	s/>/&gt;/g;

	if (/\[Page/) {
	    print OUTPUT;
	    next;
	}

	if (/^RFC \d+/) {
	    print OUTPUT;
	    next;
	}

	chop;

	#
	#  Attribute name header.
	#
	if (/^\d+\./ && !/\d$/) {
	    split;

	    if ($refs{$_[1]} ne "") {
		$attribute = $_[1];

		print OUTPUT "<A NAME=\"$attribute\"><H2>$_</H2></a>\n";

	    } else {
		print OUTPUT "<H2>$_</H2>\n";
		$attribute = "zzzz";
	    }
	    next;
	}

	#
	#  Mark these up special.
	#
	if ((/^   Description/) ||
	    (/^   Type/) ||
	    (/^   Length/) ||
	    (/^   Value/)) {
	    print OUTPUT "<B>$_</B>\n";
	    next;
	}

	# Make the current attribute name bold
	s/$attribute/<B>$attribute<\/B>/g;

	split;

	#
	#  Re-write the output with links to where-ever
	#
	foreach $word (@_) {
	    $word =~ s/[^-a-zA-Z]//g;

	    if ($refs{$word} ne "") {
		if ($refs{$word} eq $ref) {
		    s/$word/<A HREF="#$word">$word<\/A>/g;
		} else {
		    s/$word/<A HREF="$refs{$word}.html#$word">$word<\/A>/g;
		}
	    }
	}

	print OUTPUT $_, "\n";
    }

    print OUTPUT "</PRE>\n";
    print OUTPUT "</BODY>\n";
    close OUTPUT;
    close FILE;
}

#
#  And finally, create the index.
#
open OUTPUT, ">attributes.html" || die "Error creating attributes.html: $!\n";

#
#  Print out the HTML header
#
print OUTPUT <<EOF;
<!doctype html public "-//w3c//dtd html 4.0 transitional//en">
<HTML>
<head>
   <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
   <meta name="GENERATOR" content="Perl">
   <title>$ref.html</title>
</head>
<body>

<H2>RADIUS Attribute List</H2>
EOF

$letter = "@";

foreach $key (sort keys %refs) {
    if (substr($key,0,1) ne $letter) {
	print OUTPUT "</UL>\n" if ($letter ne "@");
	$letter = substr($key,0,1);
	print OUTPUT "\n<H3>$letter</H3>\n\n";
        print OUTPUT "<UL>\n";
    }

    print OUTPUT "<A HREF=\"$refs{$key}.html#$key\">$key</A><BR>\n";
}

print OUTPUT "</UL>\n";

print OUTPUT "</BODY>\n";
close OUTPUT;
