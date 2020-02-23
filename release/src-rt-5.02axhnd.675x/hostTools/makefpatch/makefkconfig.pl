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
                print "Adding Entry: ".$$entriesRef[$idx]->{name}."\n";
                $idx++;
            }
        }
        elsif ($line =~ m/^\s*#/) {
            #ignore comments completely
        }
        elsif ($line =~ m/FEATURE\s*:\s*(\S+)/)
        {
            my $name = $1;
            die "ERROR: Feature already has name! ($name / ".$$entriesRef[$idx]->{name}."\n" if ($$entriesRef[$idx]->{name});
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




