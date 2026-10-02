#!/usr/bin/perl -w

use FindBin qw($Bin);
use lib "$Bin";
use strict;
use warnings;
use Data::Dumper qw(Dumper);

my $num_args = $#ARGV + 1;

# a hash array to contain the parsed Real-Time Policy Information
my %rt_policy_info;
#my %data;
my $entry_count=0;

# array to list any flag related to rt policy settings
my @rt_policy_info_flagset = qw(RT_SET_CPUMASK RT_SET_SCHED RT_SET_CGROUPS RT_SET_IRQ);

# array to define what attributes we want to print into the metadata file
my @filtered_attrs = qw(cpuMask schedPolicy schedPriority cpuGroupName irqCpuAffinity);

my %map_attrs_to_flag = ("cpuMask","RT_SET_CPUMASK","schedPolicy","RT_SET_SCHED","schedPriority","RT_SET_SCHED","cpuGroupName","RT_SET_CGROUPS","irqCpuAffinity","RT_SET_IRQ");

# hash array to map "SCHED_XXXX" symbol into a character what the "chrt" tool can recognize
my %sch_policy = ("SCHED_NORMAL","o","SCHED_FIFO","f","SCHED_RR","r");

# hash array to map "BCM_RTPRIO_XXXX" symbol to an actual priority value
#my %bcm_priority_level = ("BCM_RTPRIO_HIGH","75","BCM_RTPRIO_VOIPDSP","35","BCM_RTPRIO_DATA","5","BCM_RTPRIO_DATA_CONTROL","10");
my %bcm_priority_level = ("BCM_RTPRIO_HIGH","","BCM_RTPRIO_VOIPDSP","","BCM_RTPRIO_DATA","","BCM_RTPRIO_DATA_CONTROL","");

#print "input_rt_info_file_dir: $input_rt_info_file_dir\n";
#print "output_metadata_file: $output_metadata_file\n";

# trim spaces for both ends
sub trim
{
    my $s = shift;
    $s =~ s/^\s+|\s+$//g;
    return $s;
}

sub open_filehandle_for_output
{
    my $filename = $_[0];
    my $overWriteFilename = ">" . $filename;
    local *FH;

    open (FH, $overWriteFilename) || die "Could not open $filename";

    return *FH;
}

sub open_filehandle_for_input
{
    my $filename = $_[0];
    local *FH;

    open (FH, $filename) || die "Could not open $filename";

    return *FH;
}

#print "@filenames\n";
#print "Num. of files: $numFilenames\n";

sub parse_input_file
{
    my $fileRef = shift;
    my $section_start=0;
    my $section_end=0;

    while(my $line = <$fileRef>)
    {
        # skip for "comment" line or empty line
        next if $line =~ /^\s*(#.*)?$/;

        # section begin
        if ($line =~ /^BEGIN$/)
        {
            if ($section_start ne 0)
            {
                print "parsing error: missing \"END\" declaration before\n";
                return -1;
            }
            $section_start=1;
        }
        elsif ($line =~ /^END$/) #section end
        {
            if ($section_start ne 1)
            {
                print "parsing error: missing \"BEGIN\" declaration before\n";
                return -1;
            }
            $section_start=0;

            # finished parsing an entity of rt policy info
            $entry_count++;
        }
        elsif ($line =~ /^([^=]+?)\s*=\s*(.*?)\s*$/) # entering the body of the BEGIN/END block. Parse it!
        {
            # the incomming data line matchs this format ==> attribute = value
            my ($attr, $value) = ($1, $2);

            # special case handling for the $value withing double quote
            # for example: "-I 1"
            if ($value =~ /"(.*?)"/)
            {
                # save parsed result into hash array.
                $rt_policy_info{$entry_count}{trim($attr)} = $1;
                next;
            }

            # general case to parse the $value string.
            # if the value string contains whitespace(s) inside,
            # we just grab the first token which is separated from the value string by setting whitespaces as separater.
            my @_value = split( /\s+/, trim($value));

            # save parsed result into hash array.
            $rt_policy_info{$entry_count}{trim($attr)} = $_value[0];
        }
    }
    return 0;
    #print Dumper \%data;
}

sub get_converted_value_by_key
{
    my $ref = shift;
    my $key = shift;
    my $value = $ref->{$key};
#print Dumper $ref;

    return if (!defined($value));

    # convert the value from a "SYMBOL" into an actual value 
    # or the other value we wan to translate
    if ($key eq "schedPriority")
    {
        if (defined($bcm_priority_level{$value}))
        {
            return $bcm_priority_level{$value};
        }
    }
    elsif ($key eq "schedPolicy")
    {
        if (defined($sch_policy{$value}))
        {
            return $sch_policy{$value};
        }
    }

    # otherwise, return raw data
    return $value;
}

sub is_contained_rt_flag
{
    my $flags = shift;
    my $i;
    my $size = scalar @rt_policy_info_flagset;

    for($i=0; $i<$size; $i++)
    {
        if (index($flags,$rt_policy_info_flagset[$i]) != -1)
        {
            return 1;
        }
    }
    return 0;
}

sub extract_rt_info
{
    my $fileRef = shift;
    my $i;
    my $j;
    my $hash_size = keys %rt_policy_info; # num. of entities
    my $value;

    # go through the hash array
    # each "$rt_policy_info{$i}" element stored the related rt information for an entity
    for($i=0; $i<$hash_size; $i++)
    {
        # skip if "name" or "flags" attr is not defined
        next if (!defined($rt_policy_info{$i}{name}) || !defined($rt_policy_info{$i}{flags}));

        # skip if "flags" attr is not including any flag related to rt policy settings
        next if (!is_contained_rt_flag($rt_policy_info{$i}{flags}));

        printf $fileRef "$i,$rt_policy_info{$i}{name}";

        # only retrieve and print out the info with the attributes which defined in "filtered_attrs"
        for($j=0; $j<=$#filtered_attrs; $j++)
        {
            if (index($rt_policy_info{$i}{flags}, $map_attrs_to_flag{$filtered_attrs[$j]}) eq -1)
            {
                printf $fileRef ",";
                next;
            }

            if (defined($rt_policy_info{$i}{$filtered_attrs[$j]}))
            {
                $value = get_converted_value_by_key(\%{$rt_policy_info{$i}},$filtered_attrs[$j]);

                if (defined($value))
                {
                    print $fileRef ",$value";
                }
                else
                {
                    printf $fileRef ",";
                }
            }
            else
            {
                printf $fileRef ",";
            }
        }
        print $fileRef "\n";
    }
}

sub parse_symbol_table
{
    my $dir = shift;
    my $inputFileRef;

    $inputFileRef = open_filehandle_for_input("$dir/symbol_table.txt");

    while(my $line = <$inputFileRef>)
    {
        # skip for "comment" line or empty line
        next if $line =~ /^\s*(#.*)?$/;

        my ($symbol, $value) = split( /\s+/, trim($line) );
        # override the default value in $bcm_priority_level
        if (defined($bcm_priority_level{$symbol}))
        {
            $bcm_priority_level{$symbol} = $value;
            next;
        }
    }

    close($inputFileRef);
}

sub parse_rt_policy_info_files
{
    my $dir = shift;
    #my @filenames = glob "$dir/eid_*.txt";
    my @filenames = glob "$dir/rt_settings_*.txt";
    my $numFilenames = @filenames;
    my $i;
    my $inputFileRef;

    for ($i=0; $i < $numFilenames; $i++)
    {
        $inputFileRef = open_filehandle_for_input($filenames[$i]);

        if (parse_input_file($inputFileRef) eq -1)
        {
            close($inputFileRef);
            die "[$0] Failed to parse file:$filenames[$i]\n";
        }

        close($inputFileRef);
        #print Dumper \%data;
    }
}

sub save_rt_policy_info_metadata
{
    my $outputFile = shift;
    my $outputFileRef;

    $outputFileRef = open_filehandle_for_output($outputFile);

    extract_rt_info($outputFileRef);
    close($outputFileRef);
}

###########################################################################
#
# Begin of main
#
###########################################################################

if ($num_args != 2) 
{
    print "\nUsage: gen_rt_setting_metadata.pl <input_rt_policy_info_file_dir> <output_rt_policy_info_metadata>\n";
    exit;
}
else
{
    parse_symbol_table($ARGV[0]);
    parse_rt_policy_info_files($ARGV[0]);
    save_rt_policy_info_metadata($ARGV[1]);
}
