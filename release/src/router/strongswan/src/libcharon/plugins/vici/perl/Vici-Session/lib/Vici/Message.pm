package Vici::Message;

our $VERSION = '0.9';

use strict;
use Vici::Transport;

use constant {
    SECTION_START => 1,   # Begin a new section having a name
    SECTION_END   => 2,   # End a previously started section
    KEY_VALUE     => 3,   # Define a value for a named key in the section
    LIST_START    => 4,   # Begin a named list for list items
    LIST_ITEM     => 5,   # Define an unnamed item value in the current list
    LIST_END      => 6,   # End a previously started list
};

sub new {
    my $class = shift;
    my $hash = shift;
    my $self = {
        Hash => $hash
    };
    bless($self, $class);
    return $self;
}

sub from_data {
    my $class = shift;
    my $data = shift;
    my %hash = ();

    open my $data_fd, '<', \$data;
    parse($data_fd, \%hash);
    close $data_fd;

    my $self = {
        Hash => \%hash
    };
    bless($self, $class);
    return $self;
}

sub hash {
    my $self = shift;
    return $self->{Hash};
}

sub encode {
    my $self = shift;
    return encode_hash($self->{'Hash'});
}

sub raw {
    my $self = shift;
    return '{' . raw_hash($self->{'Hash'}) . '}';
}

sub result {
    my $self = shift;
    my $result = $self->{'Hash'};
    return ($result->{'success'} eq 'yes', $result->{'errmsg'});
}

# private functions

sub parse {
    my $fd = shift;
    my $hash = shift;
    my $data;

    until ( eof $fd )
    {
        my $type = unpack('C', read_data($fd, 1));

        if ( $type == SECTION_END )
        {
            return;
        }

        my $key = read_len_data($fd, 1);

        if ( $type == KEY_VALUE )
        {
            my $value = read_len_data($fd, 2);
            $hash->{$key} = $value;
        }
        elsif ( $type == SECTION_START )
        {
            my %section = ();
            parse($fd, \%section);
            $hash->{$key} = \%section;
        }
        elsif ( $type == LIST_START )
        {
            my @list = ();
            my $more = 1;

            while ( !eof($fd) and $more )
            {
                my $type = unpack('C', read_data($fd, 1));

                if ( $type == LIST_ITEM )
                {
                    my $value = read_len_data($fd, 2);
                    push(@list, $value);
                }
                elsif ( $type == LIST_END )
                {
                    $more = 0;
                    $hash->{$key} = \@list;
                }
                else
                {
                    die "message parsing error: ", $type, "\n"
                }
            }
        }
        else
        {
            die "message parsing error: ", $type, "\n"
        }
    }
}

sub read_data {
    my $fd = shift;
    my $len = shift;
    my $data;

    my $res = read $fd, $data, $len;
    unless (defined $res and $res == $len)
    {
        die "message parsing error: unable to read ", $len, " bytes\n";
    }
    return $data;
}

sub read_len_data {
    my $fd = shift;
    my $len = shift;

    $len = unpack($len == 1 ? 'C' : 'n', read_data($fd, $len));
    return read_data($fd, $len);
}

sub encode_hash {
    my $hash = shift;
    my $enc = '';

    while ( (my $key, my $value) = each %$hash )
    {
        if ( ref($value) eq 'HASH' )
        {
            $enc .= pack('CC/a*', SECTION_START, $key);
            $enc .= encode_hash($value);
            $enc .= pack('C', SECTION_END);
        }
        elsif ( ref($value) eq 'ARRAY' )
        {
            $enc .= pack('CC/a*', LIST_START, $key);

            foreach my $item (@$value)
            {
                $enc .= pack('Cn/a*', LIST_ITEM, $item);
            }
            $enc .= pack('C', LIST_END);
        }
        else
        {
            $enc .= pack('CC/a*n/a*', KEY_VALUE, $key, $value);
        }
    }
    return $enc;
}

sub raw_hash {
    my $hash = shift;
    my $raw = '';
    my $first = 1;

    while ( (my $key, my $value) = each %$hash )
    {
        if ($first)
        {
            $first = 0;
        }
        else
        {
            $raw .= ' ';
        }
        $raw .= $key;

        if ( ref($value) eq 'HASH' )
        {
            $raw .= '{' . raw_hash($value) . '}';
        }
        elsif ( ref($value) eq 'ARRAY' )
        {
            my $first_item = 1;
            $raw .= '[';

            foreach my $item (@$value)
            {
                if ($first_item)
                {
                    $first_item = 0;
                }
                else
                {
                    $raw .= ' ';
                }
                $raw .= $item;
            }
            $raw .= ']';
        }
        else
        {
            $raw .= '=' . $value;
        }
    }
    return $raw;
}

1;
__END__
=head1 NAME

Vici::Message - Perl extension for building and parsing strongSwan VICI messages

=head1 SYNOPSIS

  use Vici::Message;

=head1 DESCRIPTION

The Vici::Message module is needed by the Vici::Session module to build and
parse messages used in the communication with the open source strongSwan IPsec
daemon (https://www.strongswan.com) via the documented Versatile IKE
Configuration Interface (VICI). VICI allows the configuration, management and
monitoring of multiple IPsec connections.

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

