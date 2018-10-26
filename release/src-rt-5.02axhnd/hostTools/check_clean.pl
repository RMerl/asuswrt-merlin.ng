#!/usr/bin/env perl
use Getopt::Long;

my $profile;

GetOptions('profile=s',\$profile);
open(P, "<$profile");
$profile = <P>;
close(P);


my $whitelist_file = shift;
open( W, "<$whitelist_file" );
my @whitelist = <W>;

my @errors;
while ( my $f = <> ) {
    chomp $f;
    my $result = "ERR";
    foreach my $w (@whitelist) {
        if ( $w =~ /^(OK|WARN)\s+(\S+)\s*$/ ) {
            my $action = $1;
            my $re     = $2;
            if ( $f =~ /$re/ ) {
                # print "$f matches $re ...  $action\n";
                $result = $action;
                last;
            }
        }
    }
    if ( $result eq "ERR" ) {
        push(@errors,"Error - $f not cleaned");
        $errors++;
    }
    elsif ( $result eq "WARN" )
    {
        print "Warning - $f not cleaned\n";
    }

}
print join("\n",@errors);
print "\n\nclean check completed for $profile\n";
exit($errors > 0 ? 1 : 0);


