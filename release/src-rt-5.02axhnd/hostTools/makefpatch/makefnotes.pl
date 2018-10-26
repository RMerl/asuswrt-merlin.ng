#!/usr/bin/perl
use strict;
use warnings;

my $FEATUREFILE;
my $featureListFile="";

my $buildKconfig=0;
my $buildHtml=0;
my $buildTxt=0;

my $ignoreFilter="";

my @features=();
my $dbg = 0;


sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}


my $usage = q{
Usage: makefpatch [--help] [--dbg] [-FL featureListFile] [-P patchDir] [-ignore filter] [-T tempDir] dir

options:
  --dbg      turn on verbose debugging
  -kconfig   output kconfig file
  -html      output html file
  -txt       output text file
  -ignore    ignore: specify filters which can be used to ignore certain features.  This
             is a binary expression using field==value, field!=value, field~value, field~!value
             (where the latter two are true if the field contains value, or does not contain
             value). && and || operators are also understood, but brackets are not.
  -FL        featureListFile: specify file with feature list.

Example: makefpatch --dbg -ignore "POC~John Ulvr" -fl featureList.txt -p ./patchDir kernel/linux3.4rt
};

sub printHelp {
    print $usage
}

# Parse passed in arguments:
while (my $arg1 = shift)
{
    if ($arg1 =~ m/^--help$/i ) {
        printHelp;
        exit 0;
    }
    elsif ($arg1 =~ m/^--dbg$/i) {
        $dbg = 1;
    }
    elsif ($arg1 =~ m/^-html$/i) {
        $buildKconfig = 1;
    }
    elsif ($arg1 =~ m/^-kconfig$/i) {
        $buildKconfig = 1;
    }
    elsif ($arg1 =~ m/^-te*xt$/i) {
        $buildTxt = 1;
    }
    elsif ($arg1 =~ m/^-ignore$/i) {
        $ignoreFilter = shift ;
    }
    elsif ($arg1 =~ m/^-fl(.*)$/i ) {
        $featureListFile = $1;
        $featureListFile = shift if (! $featureListFile );
    }
    else
    {
        die "Bad parameter $arg1";
    }
}

sub resolveStringEscapes {
    #there's likely some beter way of doing this, but I couldn't
    #find it on google, so doing it the ugly way.
    my $string = shift;
    my $outstring = "";
    
    while ($string =~ m/^([^\\]*)(\\)(.)(.*)$/) {
        $outstring .= $1;
        if    ($2 == "n")    { $outstring .= "\n"; }
        elsif ($2 == "t")    { $outstring .= "\t"; }
        else                 { $outstring .= $2; }
        $string=$3;
    }
    return $outstring.$string;
}

sub parseFeatureListFile {
    my $filename = shift;
    my $entriesRef = shift;  #this is a reference to an array of hashes...
    my $idx = 0;
        
    open( FEATURESFILE, "<$filename") or die "Could not open $filename for reading ($!)\n";
    while (my $line = <FEATURESFILE>) {
        chomp($line);
        if ($line =~ m/^\s*$/ ) {
            #blank lines will seperate items
            if ($$entriesRef[$idx]->{name})
            {
                #process entry:
                print "Adding Entry: ".$$entriesRef[$idx]->{name}."\n" if $dbg;
                $idx++;
                #$$entriesRef[$idx]->{help} = "XXXX___DOG"
            }
        }
        elsif ($line =~ m/^\s*#/) {
            #ignore comments completely
        }
        elsif ($line =~ m/FEATURE\s*:\s*(\S+)/)
        {
            my $name = $1;
            die "ERROR: Feature already has name! ($name / ".$$entriesRef[$idx]->{name}."\n" if ($$entriesRef[$idx]->{name});
            print("parsing feature " . $name . "\n") if ($dbg);
            $$entriesRef[$idx]->{name} = $name;
        }
        elsif ($line =~ m/^\s*(\S+):\s*(\S*.*)/)
        {
            my $key = $1;
            my $val = $2;
            if (!$val) {
                #this could be either that the value is blank, or that the 
                #user wants to read the next line...
                $val = "";
            }
            elsif ($val eq "\\")
            {
                $line = <FEATURESFILE>;
                $val = trim($val);
            }

            trim($val);
            if ($val =~ m/^"([^\\"]|\\.)*\\*$/) {
            # handling multiline quotes:
                while (1) {
                    $val .= " ";
                    die "unterminated \" found in file (".$$entriesRef[$idx]->{name}.")\n" if eof FEATURESFILE;
                    $line = <FEATURESFILE>;
                    chomp($line);
                    $line = trim($line);
                    $val .= $line;
                    die "string terminated before end of line\n\t$line\n" if ($line =~ m/[^\\]".+$/);
                    last if ($line =~ m/"$/);
                    die "Unmatched quote in line: \n\t$line\n" if  ($line !~ m/^([^\\"]|\\.)*\\*$/)
                }
                $val =~ m/^"(.*)"$/s;
                $val = $1;
            }
            elsif ($val =~ m/^"(([^\\"]|\\.)*)"$/)
            {
                # handling single line quotes:
                $val = $1;
                #TBD: replace all \" with " and all \\ with \...
            }
            resolveStringEscapes($val);

            $$entriesRef[$idx]->{$key} = $val;
        }
        else {
            die "ERROR: Unparsable line \n\t$line";
        }
    }
}

sub filterFeatures {
    my $entriesRef = shift;    #this is a reference to an array of hashes...
    my $filterStr = shift;

	my @filters = ( );

	#OK, this is cheesy -- it doesn't respect order of operations, or
	#brackets, etc.  But it's fast to implement, and it fits our current
	#needs

	die "ignore expressions do not accept brackets\n" if $filterStr =~ /[\(\)]/;

	while ($filterStr =~ /^(\w+)\s*([=!~]+)\s*([^\|\&\#\~]+)\s*([\|\&\#\~]*)\s*(.*)$/) {
		my $field=$1;
		my $op=$2;
		my $value=trim($3);
		my $join_op=$4;
		$filterStr = $5;

		die "Invalid expression $filterStr\n" if (!$field || !$op);
		
		push(@filters, { field => $field, op => $op, value => $value, join_op => $join_op });
	}

	my $eIdx;
    my $resultSoFar;
	
    for (my $eIdx=0; $eIdx <= $#{$entriesRef}; $eIdx++) {
    	my $entryRef = $entriesRef->[$eIdx];
    	$resultSoFar = 0;
    	my $join_op = "";
		foreach my $filterEntry (@filters) {
			my $result;

			my $entryValue = $entryRef->{${$filterEntry}{field}};
			$entryValue = "" unless defined $entryValue;

			my $filterValue = ${$filterEntry}{value};
			$filterValue = "" unless defined $filterValue;
			
			if (${$filterEntry}{op} eq "==" || ${$filterEntry}{op} eq "=") {
				$result = ($entryValue eq $filterValue);
			} 
			elsif (${$filterEntry}{op} eq "!=") {
				$result = ($entryValue ne $filterValue);
			} 
			elsif (${$filterEntry}{op} eq "~" ||  ${$filterEntry}{op} eq "~=") {
				$result = ($entryValue =~ /($filterValue)/ );
			} 
			elsif (${$filterEntry}{op} eq "!~" || ${$filterEntry}{op} eq "~!") {
				$result = !($entryValue =~ /($filterValue)/ );
			} 
			else {
				die "Invalid ignore expression\n"
			}
			

			if ($join_op eq "||") {
				$resultSoFar = $resultSoFar || $result;
			}
			elsif ($join_op eq "&&") {
				$resultSoFar = $resultSoFar && $result;
			}
			elsif ($join_op) {
				die "unrecognized operator $(join_op)"
			}
			else {
				$resultSoFar = $result;
			}
			$join_op = ${$filterEntry}{join_op}
		}
		splice(@$entriesRef, $eIdx--, 1) if ($resultSoFar);
	}
}

sub printFeaturesRaw
{
	# TBD: this list should come from parameter...
	my %printableFields = ( help => 1, default => 1 );

	foreach my $feature (@features) {
		if ($feature->{name}) {
			print "".$feature->{name}." \n";
			for my $field (keys %$feature) {
				print "  $field:  $feature->{$field}\n" 
					if (exists ($printableFields{$field}));
			}
			print "\n\n";
		}
	}
}

sub printHtml
{
	# good resource for this: http://www.w3schools.com/html/html_tables.asp
	# http://www.textfixer.com/resources/css-tables.php
	

	my $feature;

	my $OUTFILE = *STDOUT;
	
	print $OUTFILE "<!DOCTYPE html>\n";
	print $OUTFILE "<html>\n";
	print $OUTFILE "<head>\n";

	print $OUTFILE "<style type=\"text/css\">\n";
	print $OUTFILE ".bcmKfTable { background-color:#FFFFF0;border-collapse:collapse; }\n";
	print $OUTFILE ".bcmKfTable th { background-color:#8D874B;color:white; }\n";
	print $OUTFILE ".bcmKfTable td, .bcmKfTable th { padding:8px;border:1px solid #8D874B; font-family:verdana}\n";
	print $OUTFILE "</style>\n";
	
	print $OUTFILE "</head>\n";

	print $OUTFILE "<table class=\"bcmKfTable\">\n";
	print $OUTFILE "<tr>\n";
	print $OUTFILE "<th>FEATURE</th><th>DESCRIPTION</th>\n";
	print $OUTFILE "</tr>\n";	
	
	
	foreach $feature (@features)
	{
		next if (!$feature->{name});

		#my $isInternal = ($feature->{distribution} && $feature->{distribution} eq "internal");
		my $isInternal = (!!$feature->{distribution});
		my $isIncomplete = (!!$feature->{status});
	
		print $OUTFILE "<tr>\n";
		
		if ($isInternal) {
			print $OUTFILE "<tr style=\"background-Color:#FFF0F0\">\n" ;
		} elsif ($isIncomplete) {
			print $OUTFILE "<tr style=\"background-Color:#F0F0C0\">\n" ;
		}
		print $OUTFILE "  <td> \n" ;		
		print $OUTFILE "    ".$feature->{name} ;
		print $OUTFILE "<BR><FONT COLOR=\"#AF0000\">(internal)</FONT>" if ($isInternal);
		print $OUTFILE "<BR><FONT COLOR=\"#AF0000\">(status:$feature->{status})</FONT>" if ($isIncomplete);
		print $OUTFILE "<BR><FONT COLOR=\"#AF0000\">(default:$feature->{default})</FONT>" if ($feature->{default});
		print $OUTFILE "\n  </td>\n";
		
		print $OUTFILE "  <td> \n";		
		print $OUTFILE "    ".$feature->{help} if ($feature->{help});
		if ($feature->{internal_notes}) {
			print $OUTFILE "\n    <BR>";
			print $OUTFILE "    <FONT COLOR=\"#0000AF\">";
			print $OUTFILE "    internal note: ".$feature->{internal_notes} ;
			print $OUTFILE "    </FONT>";
		}
		if ($feature->{POC})
		{
			print $OUTFILE "\n    <BR>";
			print $OUTFILE "    <FONT COLOR=\"#606030\">";
			print $OUTFILE "    POC: ".$feature->{POC};
			print $OUTFILE "    </FONT>";
		}
		print $OUTFILE "\n  </td>\n";
		
		print( $OUTFILE "</tr>\n");		
	}

	print $OUTFILE "</html>\n";
}


sub printReleaseDoc
{
	#TBD...  Likely word doc??
}

sub printKconfig
{
	my $feature;

	my $OUTFILE = *STDOUT;
	
	print $OUTFILE "#\n";
	print $OUTFILE "# Automatically Generated Kconfig File.  DO NOT MODIFY\n";
	print $OUTFILE "#\n\n\n";

	foreach $feature (@features)
	{
		next if (!$feature->{name});
		my $featureName;
		$feature->{name} =~ m/CONFIG_(.+)/;
		$featureName = $1;
		die "ERROR: bad featureName: $feature->{name} while parsing $featureListFile\n" if (!$featureName);
		print $OUTFILE "config $featureName\n";
		print $OUTFILE "\tbool\n";
		if ($feature->{default}) {
			print $OUTFILE "\tdefault ".$feature->{default}."\n";
		} else {
			print $OUTFILE "\tdefault y\n";			
		}
		print $OUTFILE "\n";
	}
	
}

die "ERROR: Could not open file $featureListFile\n" if ( !$featureListFile);

print "Loading feature list file $featureListFile\n" if ($dbg);
parseFeatureListFile($featureListFile, \@features);

if ($ignoreFilter) {
	filterFeatures(\@features, $ignoreFilter); 
}


die "no outupt types specified" if ($buildKconfig + $buildHtml + $buildTxt == 0);

if ($buildKconfig) {
	printKconfig;
}
if ($buildHtml) {
	printHtml;
}
if ($buildTxt) {
	printFeaturesRaw;
}

