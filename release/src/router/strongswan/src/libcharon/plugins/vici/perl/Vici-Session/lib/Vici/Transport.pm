package Vici::Transport;

our $VERSION = '0.9';

use strict;

sub new {
    my $class = shift;
    my $self = {
        Socket => shift,
    };
    bless($self, $class);
    return $self;
}

sub send {
    my ($self, $data) = @_;
    my $packet = pack('N/a*', $data);
    $self->{'Socket'}->send($packet);
}

sub receive {
    my $self = shift;
    my $packet_header;

    $packet_header = $self->_recv_all(4);
    my $packet_len = unpack('N', $packet_header);
    return $self->_recv_all($packet_len);
}

sub _recv_all {
    my ($self, $len) = @_;
    my $data;

    while ($len)
    {
        my $buf;
        unless (defined $self->{'Socket'}->recv($buf, $len))
        {
            die "error reading from socket\n";
        }
        $len -= length($buf);
        $data .= $buf;
    }
    return $data;
}

1;
__END__
=head1 NAME

Vici::Transport - Perl extension for communicating via a strongSwan VICI socket

=head1 SYNOPSIS

  use Vici::Transport;

=head1 DESCRIPTION

The Vici::Transport module is needed by the Vici::Packet module to send
and receive packets over the UNIX socket used in the communication with the
open source strongSwan IPsec daemon (https://www.strongswan.com) via the
documented Versatile IKE Configuration Interface (VICI). VICI allows the
onfiguration, management and monitoring of multiple IPsec connections.

=head2 EXPORT

None by default.

=head1 SEE ALSO

strongSwan Wiki:  https://wiki.strongswan.org/projects/strongswan/wiki/Vici

strongSwan Mailing list:  users@lists.strongswan.org

=head1 AUTHOR

Andreas Steffen, E<lt>andreas.steffen@strongswan.orgE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2015 by Andreas Steffen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

=cut

