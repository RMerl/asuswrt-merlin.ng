#!/usr/bin/env perl

use strict;
use warnings;
use bytes;
use Getopt::Long;
use File::Find;
use File::Temp qw[ tempfile ];
use File::Basename;
use Data::Dumper;

my $output = "blob.bin";
my @files;

GetOptions( "output=s", \$output );

open( FO, ">", $output );

# generate list of paths
find(
    sub {
        if ( -f $_ ) {
            push @files, $File::Find::name;
        }
    },
    @ARGV
);

my $ptr = 0;
my $rec;
foreach my $file (@files) {
    my (
        $dev,  $ino,   $mode,  $nlink, $uid,     $gid, $rdev,
        $size, $atime, $mtime, $ctime, $blksize, $blocks
    ) = stat($file);
    my $content;
    open( F, "<", $file );
    {
        local $/;
        $content = <F>;
    }
    close(F);
    my $fname = basename($file);
    my $flen =
      ( length($fname) + 8 ) & ( ~7 );    # length + NULL + padding to 8-bytes
    my $totlen = ( 16 + $flen + $size + 4 + 7 ) & ( ~7 );
    my $row = pack( '(LLL)>', $totlen - 4, $flen + 8, $size );
    my ( $tfh, $tmpfile );
    ( $tfh, $tmpfile ) = tempfile();
    print $tfh $row;
    close($tfh);
    my $crc = `hostTools/gencrc32 $tmpfile`;
    $crc =~ s/\s*(\S+)\s*/$1/;
    $row .= pack( 'L>', hex($crc) );
    unlink $tmpfile;
    $row .= substr( $fname . "\0" x 8, 0, $flen );    # padded filename
    $row .= substr( $content,          0, $size );    # data
    ( $tfh, $tmpfile ) = tempfile();
    print $tfh substr( $row, 16 );
    close($tfh);
    $crc = `hostTools/gencrc32 $tmpfile`;
    $crc =~ s/\s*(\S+)\s*/$1/;
    $row .= pack( 'L>', hex($crc) );
    unlink $tmpfile;
    $row .= "\0" x 8;
    substr( $row, $totlen ) = '';
    print FO $row;
}
print FO pack( 'LLLL', 0, 0, 0, 0 );

print "------\n";
print Dumper(@files);

