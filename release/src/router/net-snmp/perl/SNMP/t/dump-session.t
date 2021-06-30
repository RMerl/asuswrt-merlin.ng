#!./perl

use strict;
use warnings;
use Test;

BEGIN {
    eval "use Cwd qw(abs_path)";
    plan tests => 1;
}

use SNMP;
use Data::Dumper;
require "t/startagent.pl";
use vars qw($agent_host $agent_port $comm);

# See also https://sourceforge.net/p/net-snmp/bugs/2488/

my $s = new SNMP::Session(DestHost=>$agent_host, Version=>1, Community=>$comm,
                          RemotePort=>$agent_port);

print Dumper($s->get('anything'));

ok(1);

snmptest_cleanup();
