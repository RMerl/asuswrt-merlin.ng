#!/usr/bin/perl
#
# Copyright (C) 2008 Sky Network Services. All Rights Reserved.
# 
# This program is free software; you can redistribute it and/or modify it
# under the same terms as Perl itself.
#
use strict;
use warnings;

use threads;
use threads::shared;

use Net::Radius::Packet;
use Net::Radius::Dictionary;
use NetSNMP::agent qw/:all/;
use NetSNMP::ASN qw/:all/;
use IO::Socket::INET;
use Digest::HMAC_MD5;
use Log::Log4perl qw/:easy/;
#use Data::Dumper;
#$Data::Dumper::Indent = 1;
#$Data::Dumper::Sortkeys = 1;
#$| = 1;

# config. should be really loaded from some file
my $cfg = {
    snmp => {
        agent => {
            Name => 'freeradius-snmp',
            AgentX => 1,
        },
        oid_root => '1.3.6.1.2.1.67',
        oid_sub => {
            1 => [qw/auth proxyauth/],
            2 => [qw/acct proxyacct/],
        },
    },

    radius => {
        host => 'localhost',
        port => 18120,
        secret => 'adminsecret',
#        dictionary => '../radiusd/share/dictionary',
        dictionary => 'dictionary.hacked',
        refresh_rate => 20,
    },


    log => {
        level => $DEBUG,
        layout => '%d{ISO8601} <%p> (%L) %m%n',
        file => 'STDERR'
    },

    clients => undef,
};
Log::Log4perl->easy_init($cfg->{log});

INFO 'starting';
my %snmp_data :shared;
my @snmp_data_k :shared;


INFO 'initializing snmp';
my $agent = new NetSNMP::agent(%{$cfg->{snmp}->{agent}});

#lets create the thread as early as possible (but it has to be AFTER initializing snmp)
INFO 'launching radius client thread';
threads->create(\&radius_updater);

#we export only subtrees, not the whole tree
$agent->register(
    $cfg->{snmp}->{agent}->{Name},
    $cfg->{snmp}->{oid_root}.'.'.$_, \&snmp_handler) or die
  foreach keys %{$cfg->{snmp}->{oid_sub}};

INFO 'entering client main loop';
$agent->main_loop;

WARN 'something caused me to exit';
exit 0;


# initialize common radius client stuff
sub radius_stats_init {
    our ( $d, $s, $rid );

    $d = new Net::Radius::Dictionary;
    $d->readfile($cfg->{radius}->{dictionary});
    srand ($$ ^ time);
    $rid = int rand 255;

    $s = new IO::Socket::INET(
        PeerHost => $cfg->{radius}->{host},
        PeerPort => $cfg->{radius}->{port},
        Proto => 'udp',
        Timeout => 5) or die;

}

# build server status packet, send it, fetch and parse the result
sub radius_stats_get {
    my ( $type, %args ) = @_;

    our ( $d, $s, $rid );

    my $p_req = new Net::Radius::Packet $d;
    $p_req->set_code('Status-Server');
    $p_req->set_vsattr('FreeRADIUS', 'FreeRADIUS-Statistics-Type', $type);
    $p_req->set_vsattr('FreeRADIUS', $_, $args{$_}) foreach keys %args;

    #update id
    $p_req->set_identifier($rid++);
    $p_req->set_authenticator(pack 'C*', map { int rand 255 } 0..15);

    #recalc authenticator
    $p_req->set_attr('Message-Authenticator', "\0"x16, 1);
    $p_req->set_attr('Message-Authenticator', Digest::HMAC_MD5::hmac_md5($p_req->pack, $cfg->{radius}->{secret}), 1);

    #send brand new and shiny request
    $s->send($p_req->pack) or die;

    my $p_data;
    if ( defined $s->recv($p_data, 2048) ) {
        my $p_res = new Net::Radius::Packet $d, $p_data;

        my %response =  map {
            $_ => $p_res->vsattr($d->vendor_num('FreeRADIUS'), $_)->[0]
        } $p_res->vsattributes($d->vendor_num('FreeRADIUS'));
        return \%response;

    }else {
        warn "no answer, $!\n";
        return undef;
    }

}

#wrappers for specific types of stats
sub radius_stats_get_global { return radius_stats_get(0x1f); }
sub radius_stats_get_client { return radius_stats_get(0x3f, 'FreeRADIUS-Stats-Client-Number' => $_[0]); }


#main loop of thread fetching status from freeradius server
sub radius_updater {
    radius_stats_init();

    while (1) {
        INFO 'fetching new data';
        my $main_stat = radius_stats_get_global();

        if ( defined $main_stat ) {
            my @clients_stat = ();

            if ( $cfg->{clients} ) {
                my $client_id = 0;

                while (1) {
                    my $client_stat = radius_stats_get_client($client_id);
                    last unless exists $client_stat->{'FreeRADIUS-Stats-Client-IP-Address'};
                    push @clients_stat, $client_stat;
                    $client_id += 1;
                }
            }

            #todo ng: update on demand, and update only parts of snmp visible stats

            INFO 'got data, updating stats';
            radius_snmp_stats($main_stat, \@clients_stat);

        }else {
            WARN 'problem with fetching data';

        }

        INFO 'stats updated, sleeping';
        sleep $cfg->{radius}->{refresh_rate};
    }

}

#helper to get string from NetSNMP::OID
sub oid_s { return join '.', $_[0]->to_array; }

#handler for snmp requests from master agent(clients)
sub snmp_handler {
    DEBUG 'got new request';
    my ($handler, $registration_info, $request_info, $requests) = @_;

    lock %snmp_data;
    lock @snmp_data_k;

    for ( my $request = $requests; $request; $request = $request->next() ) {
        INFO 'request type '.$request_info->getMode.' for oid: '.oid_s($request->getOID);

        if ( $request_info->getMode == MODE_GET ) {
            my $oid_s = oid_s($request->getOID);
            if ( exists $snmp_data{$oid_s} ) {
                $request->setValue($snmp_data{$oid_s}->[0], ''.$snmp_data{$oid_s}->[1]);
            }

        }elsif ( $request_info->getMode == MODE_GETNEXT ) {
            foreach my $oid ( @snmp_data_k ) {
                #the keys is sorted in ascending order, so we are looking for
                #first value bigger than one in request
                if ( $request->getOID < NetSNMP::OID->new($oid) ) {
                    $request->setValue($snmp_data{$oid}->[0], ''.$snmp_data{$oid}->[1]);
                    $request->setOID($oid);
                    last;
                }
            }

        }else {
            #no write support
            $request->setError($request_info, SNMP_ERR_READONLY);

        }
    }
    DEBUG 'finished processing the request';
}

#init part of subtree for handling radius auth statistics
sub radius_snmp_stats_init_auth {
    my ( $snmp_data_n, $oid, $clients ) = @_;

    @{$snmp_data_n->{$oid.'.1.1.1.1'} = &share([])} = (ASN_OCTET_STR, ''); #radiusAuthServIdent
    @{$snmp_data_n->{$oid.'.1.1.1.2'} = &share([])} = (ASN_TIMETICKS, 0); #radiusAuthServUpTime
    @{$snmp_data_n->{$oid.'.1.1.1.3'} = &share([])} = (ASN_TIMETICKS, 0); #radiusAuthServResetTime
    @{$snmp_data_n->{$oid.'.1.1.1.4'} = &share([])} = (ASN_INTEGER, 0); #radiusAuthServConfigReset
    @{$snmp_data_n->{$oid.'.1.1.1.5'} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServTotalAccessRequests
    @{$snmp_data_n->{$oid.'.1.1.1.6'} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServTotalInvalidRequests
    @{$snmp_data_n->{$oid.'.1.1.1.7'} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServTotalDupAccessRequests
    @{$snmp_data_n->{$oid.'.1.1.1.8'} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServTotalAccessAccepts
    @{$snmp_data_n->{$oid.'.1.1.1.9'} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServTotalAccessRejects
    @{$snmp_data_n->{$oid.'.1.1.1.10'} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServTotalAccessChallenges
    @{$snmp_data_n->{$oid.'.1.1.1.11'} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServTotalMalformedAccessRequests
    @{$snmp_data_n->{$oid.'.1.1.1.12'} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServTotalBadAuthenticators
    @{$snmp_data_n->{$oid.'.1.1.1.13'} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServTotalPacketsDropped
    @{$snmp_data_n->{$oid.'.1.1.1.14'} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServTotalUnknownTypes

    #radiusAuthClientTable
    for (1 .. scalar @$clients) {
        @{$snmp_data_n->{$oid.'.1.1.1.15.1.1.'.$_} = &share([])} = (ASN_INTEGER, $_); #radiusAuthClientIndex
        @{$snmp_data_n->{$oid.'.1.1.1.15.1.2.'.$_} = &share([])} = (ASN_IPADDRESS, pack 'C4', split /\./, $clients->[$_-1]->{'FreeRADIUS-Stats-Client-IP-Address'}); #radiusAuthClientAddress
        @{$snmp_data_n->{$oid.'.1.1.1.15.1.3.'.$_} = &share([])} = (ASN_OCTET_STR, $clients->[$_-1]->{'FreeRADIUS-Stats-Client-Number'}); #radiusAuthClientID
#        @{$snmp_data_n->{$oid.'.1.1.1.15.1.4.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServAccessRequests
#        @{$snmp_data_n->{$oid.'.1.1.1.15.1.5.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServDupAccessRequests
#        @{$snmp_data_n->{$oid.'.1.1.1.15.1.6.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServAccessAccepts
#        @{$snmp_data_n->{$oid.'.1.1.1.15.1.7.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServAccessRejects
#        @{$snmp_data_n->{$oid.'.1.1.1.15.1.8.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServAccessChallenges
#        @{$snmp_data_n->{$oid.'.1.1.1.15.1.9.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServMalformedAccessRequests
#        @{$snmp_data_n->{$oid.'.1.1.1.15.1.10.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServBadAuthenticators
#        @{$snmp_data_n->{$oid.'.1.1.1.15.1.11.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServPacketsDropped
#        @{$snmp_data_n->{$oid.'.1.1.1.15.1.12.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAuthServUnknownTypes
    }
}

#init part of subtree for handling radius acct statistics
sub radius_snmp_stats_init_acct {
    my ( $snmp_data_n, $oid, $clients ) = @_;

    @{$snmp_data_n->{$oid.'.1.1.1.1'} = &share([])} = (ASN_OCTET_STR, ''); #radiusAccServIdent
    @{$snmp_data_n->{$oid.'.1.1.1.2'} = &share([])} = (ASN_TIMETICKS, 0); #radiusAccServUpTime
    @{$snmp_data_n->{$oid.'.1.1.1.3'} = &share([])} = (ASN_TIMETICKS, 0); #radiusAccServResetTime
    @{$snmp_data_n->{$oid.'.1.1.1.4'} = &share([])} = (ASN_INTEGER, 0); #radiusAccServConfigReset
    @{$snmp_data_n->{$oid.'.1.1.1.5'} = &share([])} = (ASN_COUNTER, 0); #radiusAccServTotalRequests
    @{$snmp_data_n->{$oid.'.1.1.1.6'} = &share([])} = (ASN_COUNTER, 0); #radiusAccServTotalInvalidRequests
    @{$snmp_data_n->{$oid.'.1.1.1.7'} = &share([])} = (ASN_COUNTER, 0); #radiusAccServTotalDupRequests
    @{$snmp_data_n->{$oid.'.1.1.1.8'} = &share([])} = (ASN_COUNTER, 0); #radiusAccServTotalResponses
    @{$snmp_data_n->{$oid.'.1.1.1.9'} = &share([])} = (ASN_COUNTER, 0); #radiusAccServTotalMalformedRequests
    @{$snmp_data_n->{$oid.'.1.1.1.10'} = &share([])} = (ASN_COUNTER, 0); #radiusAccServTotalBadAuthenticators
    @{$snmp_data_n->{$oid.'.1.1.1.11'} = &share([])} = (ASN_COUNTER, 0); #radiusAccServTotalPacketsDropped
    @{$snmp_data_n->{$oid.'.1.1.1.12'} = &share([])} = (ASN_COUNTER, 0); #radiusAccServTotalNoRecords
    @{$snmp_data_n->{$oid.'.1.1.1.13'} = &share([])} = (ASN_COUNTER, 0); #radiusAccServTotalUnknownTypes

    #radiusAccClientTable
    for (1 .. scalar @$clients) {
        @{$snmp_data_n->{$oid.'.1.1.1.14.1.1.'.$_} = &share([])} = (ASN_INTEGER, $_); #radiusAccClientIndex
        @{$snmp_data_n->{$oid.'.1.1.1.14.1.2.'.$_} = &share([])} = (ASN_IPADDRESS, pack 'C4', split /\./, $clients->[$_-1]->{'FreeRADIUS-Stats-Client-IP-Address'}); #radiusAccClientAddress
        @{$snmp_data_n->{$oid.'.1.1.1.14.1.3.'.$_} = &share([])} = (ASN_OCTET_STR, $clients->[$_-1]->{'FreeRADIUS-Stats-Client-Number'}); #radiusAccClientID
#        @{$snmp_data_n->{$oid.'.1.1.1.14.1.4.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAccServPacketsDropped
#        @{$snmp_data_n->{$oid.'.1.1.1.14.1.5.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAccServRequests
#        @{$snmp_data_n->{$oid.'.1.1.1.14.1.6.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAccServDupRequests
#        @{$snmp_data_n->{$oid.'.1.1.1.14.1.7.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAccServResponses
#        @{$snmp_data_n->{$oid.'.1.1.1.14.1.8.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAccServBadAuthenticators
#        @{$snmp_data_n->{$oid.'.1.1.1.14.1.9.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAccServMalformedRequests
#        @{$snmp_data_n->{$oid.'.1.1.1.14.1.10.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAccServNoRecords
#        @{$snmp_data_n->{$oid.'.1.1.1.14.1.11.'.$_} = &share([])} = (ASN_COUNTER, 0); #radiusAccServUnknownTypes
    }
}

#fill part of subtree with data from radius auth statistics
sub radius_snmp_stats_fill_auth {
    my ( $snmp_data_n, $oid, $prefix, $main, $clients ) = @_;
    #hmm .. proxy?

    my $time = time;

    $snmp_data_n->{$oid.'.1.1.1.1'}->[1] = 'snmp(over)radius';
    $snmp_data_n->{$oid.'.1.1.1.2'}->[1] = ($time - $main->{'FreeRADIUS-Stats-Start-Time'})*100;
    $snmp_data_n->{$oid.'.1.1.1.3'}->[1] = ($time - $main->{'FreeRADIUS-Stats-HUP-Time'})*100;
    $snmp_data_n->{$oid.'.1.1.1.4'}->[1] = 0;
    $snmp_data_n->{$oid.'.1.1.1.5'}->[1] += $main->{$prefix.'Access-Requests'};
    $snmp_data_n->{$oid.'.1.1.1.6'}->[1] += $main->{$prefix.'Auth-Invalid-Requests'};
    $snmp_data_n->{$oid.'.1.1.1.7'}->[1] += $main->{$prefix.'Auth-Duplicate-Requests'};
    $snmp_data_n->{$oid.'.1.1.1.8'}->[1] += $main->{$prefix.'Access-Accepts'};
    $snmp_data_n->{$oid.'.1.1.1.9'}->[1] += $main->{$prefix.'Access-Rejects'};
    $snmp_data_n->{$oid.'.1.1.1.10'}->[1] += $main->{$prefix.'Access-Challenges'};
    $snmp_data_n->{$oid.'.1.1.1.11'}->[1] += $main->{$prefix.'Auth-Malformed-Requests'};
    $snmp_data_n->{$oid.'.1.1.1.12'}->[1] += 0;
    $snmp_data_n->{$oid.'.1.1.1.13'}->[1] += $main->{$prefix.'Auth-Dropped-Requests'};
    $snmp_data_n->{$oid.'.1.1.1.14'}->[1] += $main->{$prefix.'Auth-Unknown-Types'};

    for (1 .. scalar @$clients) {
#         $snmp_data_n->{$oid.'.1.1.1.15.1.4.'.$_}->[1] += $clients->[$_-1]->{$prefix.'Access-Requests'};
#         $snmp_data_n->{$oid.'.1.1.1.15.1.5.'.$_}->[1] += $clients->[$_-1]->{$prefix.'Auth-Duplicate-Requests'};
#         $snmp_data_n->{$oid.'.1.1.1.15.1.6.'.$_}->[1] += $clients->[$_-1]->{$prefix.'Access-Accepts'};
#         $snmp_data_n->{$oid.'.1.1.1.15.1.7.'.$_}->[1] += $clients->[$_-1]->{$prefix.'Access-Rejects'};
#         $snmp_data_n->{$oid.'.1.1.1.15.1.8.'.$_}->[1] += $clients->[$_-1]->{$prefix.'Access-Challenges'};
#         $snmp_data_n->{$oid.'.1.1.1.15.1.9.'.$_}->[1] += $clients->[$_-1]->{$prefix.'Auth-Malformed-Requests'};
#         $snmp_data_n->{$oid.'.1.1.1.15.1.10.'.$_}->[1] += 0;
#         $snmp_data_n->{$oid.'.1.1.1.15.1.11.'.$_}->[1] += $clients->[$_-1]->{$prefix.'Auth-Dropped-Requests'};
#         $snmp_data_n->{$oid.'.1.1.1.15.1.12.'.$_}->[1] += $clients->[$_-1]->{$prefix.'Auth-Unknown-Types'};
    }
}

#fill part of subtree with data from radius acct statistics
sub radius_snmp_stats_fill_acct {
    my ( $snmp_data_n, $oid, $prefix, $main, $clients ) = @_;
    #hmm .. proxy?

    my $time = time;

    $snmp_data_n->{$oid.'.1.1.1.1'}->[1] = 'snmp(over)radius';
    $snmp_data_n->{$oid.'.1.1.1.2'}->[1] = ($time - $main->{'FreeRADIUS-Stats-Start-Time'})*100;
    $snmp_data_n->{$oid.'.1.1.1.3'}->[1] = ($time - $main->{'FreeRADIUS-Stats-HUP-Time'})*100;
    $snmp_data_n->{$oid.'.1.1.1.4'}->[1] = 0;
    $snmp_data_n->{$oid.'.1.1.1.5'}->[1] += $main->{$prefix.'Accounting-Requests'};
    $snmp_data_n->{$oid.'.1.1.1.6'}->[1] += $main->{$prefix.'Acct-Invalid-Requests'};
    $snmp_data_n->{$oid.'.1.1.1.7'}->[1] += $main->{$prefix.'Acct-Duplicate-Requests'};
    $snmp_data_n->{$oid.'.1.1.1.8'}->[1] += $main->{$prefix.'Accounting-Responses'};
    $snmp_data_n->{$oid.'.1.1.1.9'}->[1] += $main->{$prefix.'Acct-Malformed-Requests'};
    $snmp_data_n->{$oid.'.1.1.1.10'}->[1] += 0;
    $snmp_data_n->{$oid.'.1.1.1.11'}->[1] += $main->{$prefix.'Acct-Dropped-Requests'};
    $snmp_data_n->{$oid.'.1.1.1.12'}->[1] += 0;
    $snmp_data_n->{$oid.'.1.1.1.13'}->[1] += $main->{$prefix.'Acct-Unknown-Types'};

    for (1 .. scalar @$clients) {
#         $snmp_data_n->{$oid.'.1.1.1.14.1.4.'.$_}->[1] += $clients->[$_-1]->{$prefix.''};# 'radiusAccServPacketsDropped';
#         $snmp_data_n->{$oid.'.1.1.1.14.1.5.'.$_}->[1] += $clients->[$_-1]->{$prefix.''};# 'radiusAccServRequests';
#         $snmp_data_n->{$oid.'.1.1.1.14.1.6.'.$_}->[1] += $clients->[$_-1]->{$prefix.''};# 'radiusAccServDupRequests';
#         $snmp_data_n->{$oid.'.1.1.1.14.1.7.'.$_}->[1] += $clients->[$_-1]->{$prefix.''};# 'radiusAccServResponses';
#         $snmp_data_n->{$oid.'.1.1.1.14.1.8.'.$_}->[1] += $clients->[$_-1]->{$prefix.''};# 'radiusAccServBadAuthenticators';
#         $snmp_data_n->{$oid.'.1.1.1.14.1.9.'.$_}->[1] += $clients->[$_-1]->{$prefix.''};# 'radiusAccServMalformedRequests';
#         $snmp_data_n->{$oid.'.1.1.1.14.1.10.'.$_}->[1] += $clients->[$_-1]->{$prefix.''};# 'radiusAccServNoRecords';
#         $snmp_data_n->{$oid.'.1.1.1.14.1.11.'.$_}->[1] += $clients->[$_-1]->{$prefix.''};# 'radiusAccServUnknownTypes';
    }
}

#update statistics
sub radius_snmp_stats {
    my ( $main, $clients ) = @_;

#print Dumper($main, $clients);

    my %snmp_data_n;

    # we have to go through all oid's
    foreach my $oid_s ( keys %{$cfg->{snmp}->{oid_sub}} ) {

        #we're rebuilding the tree for data
        #we could do it only once, but it will change when we will start handling more dynamic
        #tree (clients)
        my %types = map { $_ => 1 } map { /(?:proxy)?(\w+)/; $1 } @{$cfg->{snmp}->{oid_sub}->{$oid_s}};
        WARN 'two conflicting types for oid '.$oid_s  if scalar keys %types > 1;

        if ( (keys %types)[0] eq 'auth' ) {
            radius_snmp_stats_init_auth(\%snmp_data_n, $cfg->{snmp}->{oid_root}.'.'.$oid_s, $clients);

        }elsif ( (keys %types)[0] eq 'acct' ) {
            radius_snmp_stats_init_acct(\%snmp_data_n, $cfg->{snmp}->{oid_root}.'.'.$oid_s, $clients);

        }else {
            WARN 'unknown subtree type '.(keys %types)[0];

        }

        #now lets refill the statistics
        foreach my $type ( @{$cfg->{snmp}->{oid_sub}->{$oid_s}} ) {     
            if ( $type eq 'auth' ) {
                radius_snmp_stats_fill_auth(
                    \%snmp_data_n, $cfg->{snmp}->{oid_root}.'.'.$oid_s,
                    'FreeRADIUS-Total-', $main, $clients);

            }elsif ( $type eq 'proxyauth' ) {
                radius_snmp_stats_fill_auth(
                    \%snmp_data_n, $cfg->{snmp}->{oid_root}.'.'.$oid_s,
                    'FreeRADIUS-Total-Proxy-', $main, $clients);

            }elsif ( $type eq 'acct' ) {
                radius_snmp_stats_fill_acct(
                    \%snmp_data_n, $cfg->{snmp}->{oid_root}.'.'.$oid_s,
                    'FreeRADIUS-Total-', $main, $clients);

            }elsif ( $type eq 'proxyacct' ) {
                radius_snmp_stats_fill_acct(
                    \%snmp_data_n, $cfg->{snmp}->{oid_root}.'.'.$oid_s,
                    'FreeRADIUS-Total-Proxy-', $main, $clients);

            }else {
                WARN 'unknown subtree type '.$type;
            }
            
        }
    }

    #we rebuild the tree, so lets now lock the shared variables and push new data there
    lock %snmp_data;
    lock @snmp_data_k;

    %snmp_data = %snmp_data_n;
    @snmp_data_k = map { oid_s($_) } sort { $a <=> $b } map { NetSNMP::OID->new($_) } keys %snmp_data_n;
}

=head1 NAME

freeradius snmp agentx subagent

=head1 VERSION

=head1 SYNOPSIS

make sure snmpd is agentx master (snmpd.conf):
master agentx

run the script (no demonizing support yet):

./freeradius-snmp.pl

then you can walk the tree (default oid):

snmpbulkwalk -On -v2c -cpublic localhost .1.3.6.1.2.1.67

=head1 DESCRIPTION

=head1 DEPENDENCIES

Net-Radius (either 1.56 + net-radius-freeradius-dictionary.diff to use freeradius dictionaries
  or vanilla upstream one + dictionary.hacked)
NetSNMP perl modules (available with net-snmp distribution)
Digest::HMAC
Log::Log4perl

=head1 AUTHOR

Stanislaw Sawa <stanislaw.sawa(at)sns.bskyb.com>

=head1 COPYRIGHT

Copyright (C) 2008 Sky Network Services. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.
