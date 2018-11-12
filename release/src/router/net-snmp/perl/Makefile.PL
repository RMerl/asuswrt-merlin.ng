use strict;
use warnings;
use ExtUtils::MakeMaker;
use Config;
require 5;
use MakefileSubs;

WriteMakefile(TopLevelInitMakeParams());

sub TopLevelInitMakeParams {
    my $nsconfig="net-snmp-config"; # in path by default
    my %Params = (
		  'NAME' => 'Bundle::NetSNMP',
		  'DIR' => [qw(default_store ASN OID agent SNMP TrapReceiver)],
		  );

    # bogus, but these options need to be passed to the lower levels
    my $opts = NetSNMPGetOpts("./");

    return(%Params);
}
