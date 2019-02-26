package Vici::Session;

our $VERSION = '0.9';

use strict;
use Vici::Packet;
use Vici::Message;

sub new {
    my $class = shift;
    my $socket = shift;
    my $self = {
        Packet => Vici::Packet->new($socket),
    };
    bless($self, $class);
    return $self;
}

sub version {
    return request('version', @_);
}

sub stats {
    return request('stats', @_);
}

sub reload_settings {
   return request_res('reload-settings', @_);
}

sub initiate {
    return request_vars_res('initiate', @_);
}

sub terminate {
    return request_vars_res('terminate', @_);
}

sub redirect {
    return request_vars_res('redirect', @_);
}

sub install {
    return request_vars_res('install', @_);
}

sub uninstall {
    return request_vars_res('uninstall', @_);
}

sub list_sas {
    return request_list('list-sas', 'list-sa', @_);
}

sub list_policies {
    return request_list('list-policies', 'list-policy', @_);
}

sub list_conns {
    return request_list('list-conns', 'list-conn', @_);
}

sub get_conns {
    return request('get-conns', @_);
}

sub list_certs {
    return request_list('list-certs', 'list-cert', @_);
}

sub list_authorities {
    return request_list('list-authorities', 'list-authority', @_);
}

sub get_authorities {
    return request('get-authorities', @_);
}

sub load_conn {
    return request_vars_res('load-conn', @_);
}

sub unload_conn {
    return request_vars_res('unload-conn', @_);
}

sub load_cert {
    return request_vars_res('load-cert', @_);
}

sub load_key {
    return request_vars_res('load-key', @_);
}

sub load_shared {
    return request_vars_res('load-shared', @_);
}

sub flush_certs {
    return request_vars_res('flush-certs', @_);
}

sub clear_creds {
   return request_res('clear-creds', @_);
}

sub load_authority {
    return request_vars_res('load-authority', @_);
}

sub unload_authority {
    return request_vars_res('unload-authority', @_);
}

sub load_pool {
    return request_vars_res('load-pool', @_);
}

sub unload_pool {
    return request_vars_res('unload-pool', @_);
}

sub get_pools {
    return request('get-pools', @_);
}

sub get_algorithms {
    return request('get-algorithms', @_);
}

# Private functions

sub request {
    my ($command, $self) = @_;
    return $self->{'Packet'}->request($command);
}

sub request_res {
    my ($command, $self) = @_;
    my $msg = $self->{'Packet'}->request($command);
    return $msg->result();
}

sub request_vars_res {
    my ($command, $self, $vars) = @_;
    my $msg = $self->{'Packet'}->request($command, $vars);
    return $msg->result();
}

sub request_list {
    my ($command, $event, $self, $vars) = @_;
    return $self->{'Packet'}->streamed_request($command, $event, $vars);
}

1;
__END__
=head1 NAME

Vici::Session - Perl binding for the strongSwan VICI configuration interface

=head1 SYNOPSIS

  use Vici::Session;

=head1 DESCRIPTION

The Vici::Session module allows a Perl script to communicate with the open
source strongSwan IPsec daemon (https://www.strongswan.com) via the documented
Versatile IKE Configuration Interface (VICI). VICI allows the configuration,
management and monitoring of multiple IPsec connections.

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
