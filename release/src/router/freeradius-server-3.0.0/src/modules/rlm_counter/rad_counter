#!/usr/bin/perl
#
#	$Id$
#
use warnings ;
use GDBM_File ;
use Fcntl ;
use Getopt::Long;

my $user = '';
my $divisor = 1;
my $reset = 0;
my $match = '.*';
my $help = 0;

#
#  This should be fixed...
#
$filename = '';

sub show_help {
    print "Usage: $0 --file=<counter filename> [--reset=<seconds>] [--match=<regexp>]\n";
    print "[--user=<username>] [--help] [--hours|--minutes|--seconds]\n\n";
    print "--user=<username>", "\t\t", "Information for specific user\n";
    print "--file=<filename>", "\t\t", "Counter db filename\n";
    print "--match=<regexp>", "\t\t", "Information for matching users\n";
    print "--reset=<number>", "\t\t", "Reset counter to <number>.\n";
    print "\t\t\t\t", "If divisor is set use it, else <number> means seconds\n";
    print "--help", "\t\t\t\t", "Show this help screen\n";
    print "--(hours|minutes|seconds)", "\t", "Specify information divisor\n";
    exit 0;
}

#
#  Print out only one user,
#
#  Or specifiy printing in hours, minutes, or seconds (default)
#
GetOptions ('user=s'  => \$user,
	    'match=s' => \$match,
	    'file=s'  => \$filename,
            'reset=i' => \$reset,
            'help'    => \$help,
	    'hours'   => sub { $divisor = 3600 },
	    'minutes' => sub { $divisor = 60 },
	    'seconds' => sub { $divisor = 1 } );

show_help if ($help || $filename eq '');

#
#  Open the file.
#
if ($reset){
    my $db = tie(%hash, 'GDBM_File', $filename, O_RDWR, 0666) or die "Cannot open $filename: $!\n";
}else{
    my $db = tie(%hash, 'GDBM_File', $filename, O_RDONLY, 0666) or die "Cannot open $filename: $!\n";
}

#
#  If given one name, give the seconds
#
if ($user ne '') {
    if (defined($hash{$user})){
        print $user, "\t\t", int ( unpack('L',$hash{$user}) / $divisor), "\n";
	if ($reset){
            my $uniqueid = (unpack('L A32',$hash{$user}))[1];
            $hash{$user} = pack('L A32',$reset * $divisor,$uniqueid);
            print $user, "\t\t", "Counter reset to ", $reset * $divisor, "\n";
        }
    }else{
        print $user, "\t\t", "Not found\n";
    }

    undef $db;
    untie %hash;
    exit 0;
}

#
#  This may be faster, but unordered.
#while (($key,$val) = each %hash) {
#
foreach $key (sort keys %hash) {
    #
    #  These are special.
    next if ($key eq "DEFAULT1");
    next if ($key eq "DEFAULT2");

    #
    #  Allow user names matching a regex.
    #
    next if ($key !~ /$match/);

    #
    #  Print out the names...
    print $key, "\t\t", int ( unpack('L',$hash{$key}) / $divisor), "\n";
    if ($reset){
        my $uniqueid = (unpack('L A32',$hash{$key}))[1];
        $hash{$key} = pack('L A32',$reset * $divisor,$uniqueid);
        print $key, "\t\t", "Counter reset to ", $reset * $divisor, "\n";
    }
}
undef $db;
untie %hash;
