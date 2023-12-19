#!/usr/bin/perl
# by david bird - to format stuff in this directory

sub trim {
    my $line = shift;
    $line =~ s/^\s+//g;
    $line =~ s/\s+$//g;
    $line;
}

sub man2wiki {
    $r = 0;
    $in_tp=0;
    $in_tp_s=0;
    $in_ex=0;
    $nl=0;
    $in_rs=0;

    $out='';

    while (<STDIN>) {
	next if (/^\.\\\"/);
	chop;

	$line = $_;

	$line = trim($line);

	if ($line =~ /^\.TH (.*)$/) {
	    $nl=0;
	    @a = split / /, $line;
	    $out .= "= [[CoovaChilli]] - ".trim($a[1])."(".trim($a[2]).") =\n";
	} elsif ($line =~ /^\.SH (.*)$/) {
	    $nl=0;
	    $l = $1;
	    $l =~ s/"//g; #"
	    $out .= "\n== ".trim($l)." ==\n\n";
	    $in_ex = ($l eq 'EXAMPLES');
	    $in_tp_s=0;
	} elsif ($line =~ /^\.B (.*)$/) {
	    $nl=0;
	    $l = $1;
	    $l =~ s/"//g; #"
	    if ($in_tp) {
		$out .= "; '''".trim($l)."''' ";
	    } else {
		$out .= "'''".trim($l)."''' ";
	    }
	} elsif ($line =~ /^\.TP/) {
	    $nl=0;
	    $in_tp=1;
	    $in_tp_s=1;
	} elsif ($line =~ /^\.BI (.*)$/) {
	    $nl=0;
	    $l = $1;
	    $l =~ s/"//g; #"
	    @a = split / /, $l, 2;
	    if ($in_tp) {
		$out .= "; '''".trim($a[0])."''' ";
		$out .= "''".trim($a[1])."'' " if ($a[1]);
	    } else {
		$out .= "'''".trim($a[0])."''' ";
		$out .= "''".trim($a[1])."'' ";
	    }
	} elsif ($line =~ /^\.I (.*)$/) {
	    $nl=0;
	    $out .= " " if $in_ex;
	    $out .= "''".trim($1)."'' ";
	    $out .= "BACKSLASH" if $in_ex && $line =~ /\\$/;
	    $out .= "\n " if $in_ex;
	    $out .= "\n" if $in_tp_s && $nl;
	} elsif ($line =~ /^\.LP/) {
	} elsif ($line =~ /^\.RS/) {
	    $out .= "\n<blockquote>";
#	    $out .= "\n: ";
#	    $out .= "\n<div style='margin-left:60px;'>\n";
	    $in_rs = 1;
	} elsif ($line =~ /^\.RE/) {
#	    $out .= "\n</dd></dl>\n";
	    if ($in_rs) {
#		$out .= "\n</div>\n";
		$out .= "</blockquote>\n\n";
		$in_rs = 0;
	    }
	} elsif ($line =~ /^\.BR (.*)$/) {
	    $nl=0;
	    $link = trim($1);
	    if ($link =~ /chilli/) {
		$l = $link;
		$l =~ s/\(\d+\)//g;
		$out .= "[[CoovaChilli $l|$link]] ";
	    } else {
		$link =~ s/\s//g;
		$out .= "'''$link'''";
	    }
	} else {
	    if ($line =~ /^$/) {
		if ($nl < 1) {
		    $out .= "\n\n";
		}
		$nl++;
		$in_tp=0;
	    } else {
		$nl=0;
		if ($in_tp) {
		    $out .= ": ";
		    $in_tp=0;
		}
		$out .= "$line ";
		$out .= "BACKSLASH\n " if ($in_ex && $line =~ /\\$/);
	    }
	}
    }

    $out =~ s/\n\n\n/\n\n/gs;
    $out =~ s/\n\n\n/\n\n/gs;
    $out =~ s/<[^\@>]+\@[^>]+>//sg;
    $out =~ s/ , /, /gs;
    $out =~ s/ \. /. /gs;
    $out =~ s/\\//gs;
    $out =~ s/BACKSLASH/\\/gs;

#    print "<div style='float:right;'>__TOC__</div>\n";
    print trim($out);
    print "\n";
}

sub attributes2wiki {
    $r = 0;
    while (<STDIN>) {
	chop;
	if (/^[a-zA-Z]/) {
	    @cols = split(/\s*:\s*/,$_,9);
	    print "|-";
	    print "style='background-color: #ddd;' " if ($r % 2);
	    print "\n";
	    $i = 0;
	    foreach (@cols) {
		$l = 0;
		print "| ";
		print "style='font-weight:bold;border-right: solid 1px grey;' " if ($i < 1 && ++$l);
		print "style='border-right: solid 1px grey;' " if ($i > 0 && $i < 8 && ++$l);
		print "align='center' " if ($i > 1 && $i < 8 && ++$l);
		print "align='right' " if ($i == 1 && ++$l);
		print "nowrap='nowrap' " if ($i < 2 && ++$l);
		print "| " if $l;
		print $_;
		print "\n";
		$i++;
	    }
	    $r++;
	}
    }
}

sub attributes2man {
    $r = 0;

    $attributes='';
    while (<STDIN>) {
	chop;
	if (/^[a-zA-Z]/) {
	    @cols = split(/\s*:\s*/,$_,9);
	    $attributes .= "\n";
	    $attributes .= ".TP\n";
	    $attributes .= ".B $cols[0] ($cols[1])\n";
	    $attributes .= $cols[8];
	    $attributes .= "\n";
	    $r++;
	}
    }

    $file = `cat chilli-radius.5.in`;
    $file =~ s/\@ATTRIBUTES\@/$attributes/;

    print $file
}

man2wiki() if ($ARGV[0] eq 'man2wiki');
attributes2wiki() if ($ARGV[0] eq 'attributes2wiki');
attributes2man() if ($ARGV[0] eq 'chilli-radius');
