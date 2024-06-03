package Digest::Crc32;

# Cyclic Redundency Check interface for buffers and files

use strict;
use Carp;
use vars qw($VERSION $poly);

$VERSION = 0.01;

$poly = 0xEDB88320;

sub version { sprintf("%f", $VERSION); }

sub new {
    my $self = {};
    my $proto = shift;
    my $class = ref($proto) || $proto;
    bless($self,$class);
    return $self;
}

sub _crc32 {
	my $self = shift;
	my $comp = shift;
	for (my $cnt = 0; $cnt < 8; $cnt++) { $comp = $comp & 1 ? $poly ^ ($comp >> 1) : $comp >> 1; }
	return $comp;
}

sub strcrc32 {
	my $self = shift;
	my $crc = 0xFFFFFFFF;
	my ($tcmp) = @_;
	foreach (split(//,$tcmp)) { $crc = (($crc>>8) & 0x00FFFFFF) ^ $self->_crc32(($crc ^ ord($_)) & 0xFF); }
	return $crc^0xFFFFFFFF;
}

sub filecrc32 {
	my $self = shift;
	my $file = shift;
	my $crc = 0xFFFFFFFF;
	open(FILE, $file) or croak "Failed to open the file";
	while (<FILE>) {
	    foreach (split(//,$_)) { $crc = (($crc>>8) & 0x00FFFFFF) ^ $self->_crc32(($crc ^ ord($_)) & 0xFF); }
	}
	close(FILE);
	return $crc^0xFFFFFFFF;
}


=head1 NAME

Digest::CRC32 - Cyclic Redundency Check digests implementation

=head1 VERSION

0.01

=head1 SYNOPSIS

	use Digest::CRC32;

	my $crc = new Digest::CRC32();

	# Digest for a string
	printf $crc->strcrc32("Hello world");

	#Digest for a file
	print $crc->filecrc32($myfile);

=head1 DESCRIPTION

This module provides a perl implementation to generate 32 bits CRC digests for buffers and files.

=head1 COPYRIGHT

Copyright 2004 by Faycal Chraibi. All rights reserved.

This library is a free software. You can redistribute it and/or modify it under the same terms as Perl itself.

=head1 AUTHOR

Faycal Chraibi <fays@cpan.org>

=head1 SEE ALSO

L<Digest::CRC>
