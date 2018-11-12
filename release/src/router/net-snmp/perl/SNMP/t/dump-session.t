#!./perl

use strict;
use Test;

BEGIN {
    unless(grep /blib/, @INC) {
        chdir 't' if -d 't';
        @INC = '../lib' if -d '../lib';
    }
    eval "use Cwd qw(abs_path)";
    $ENV{'SNMPCONFPATH'} = 'nopath';
    $ENV{'MIBDIRS'} = '+' . abs_path("../../mibs");
    plan tests => 1;
}

use SNMP;
use Data::Dumper;
use vars qw($agent_port $comm $agent_host);
require "t/startagent.pl";

# See also https://sourceforge.net/p/net-snmp/bugs/2488/

my $s = new SNMP::Session(DestHost=>$agent_host, Version=>1, Community=>$comm,
                          RemotePort=>$agent_port);

print Dumper($s->get('anything'));

ok(1);
