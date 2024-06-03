#!/usr/bin/perl -w

use lib "$ARGV[4]";
use BRCM::GenConfig;
use File::Copy;
use File::Basename;
use File::Spec::Functions;

$num_args = $#ARGV + 1;
if ( $num_args != 6 ) {
	print "\n Usage cperouer_init_config.pl <builddir> <wldir> <routerdir> <conf file> <perlib path> <profile path>\n";
	exit;
}

#These are items CPEROUTER does not want to have from HNDROUTER source
my @cpe_disable_items = ( "EZC","UDHCPD","RC","IPTABLES", "IGD","SAMBA","NVRAMD","TREND_IQOS","DNSMASQ");
#profile related changes, change configurtion by profile configurtion.
my %config_to_change = (
    "BUILD_WLVISUALIZATION", "VISUALIZATION",
    "BUILD_WBD", "WBD",
    "BUILD_BRCM_AIRIQ", "AIRIQ",
    "BUILD_RDKWIFI", "!BCMESCAND",
    "RDK_BUILD", "!RPCAPD"
);

my $p = new BRCM::GenConfig($ARGV[5]);
my $build_dir = $ARGV[0];
my $wl_dir = $ARGV[1];
my $router_dir = $ARGV[2];
my $config_file =  $ARGV[3];
my $dot_config = catfile($router_dir,'.config');
my $wladjust_pl = catfile($wl_dir,'wlconf_adjust.pl');
my $utility_sh= catfile($build_dir,'patches','utility.sh');
my $brcm_lib_dir = catfile($build_dir,'hostTools','PerlLib');
my $chip = $p->get('BRCM_CHIP');

foreach my $n (@cpe_disable_items) {
	system($wladjust_pl,$router_dir,$config_file,$n,'disable');
}

#when build CPEROUTER and HOSTAPD enabled, enable RC to generate libnv2hapdcfg.so
if (( $p->get("BUILD_BRCM_CPEROUTER")) &&  ( $p->get("BUILD_BRCM_HOSTAPD"))) {
	system($wladjust_pl,$router_dir,$config_file,'RC','enable');
} else {
	system($wladjust_pl,$router_dir,$config_file,'RC','disable');
}

if ( $chip =~ /^(4908|47622|63178|4912|6813|6756)$/ ) {
	if ( $p->get("BUILD_BRCM_CPEROUTER") && ( $p->get("BUILD_BSTREAM_IQOS"))) {
		system($wladjust_pl,$router_dir,$config_file,'TREND_IQOS','enable');
	} else {
		system($wladjust_pl,$router_dir,$config_file,'TREND_IQOS','disable');
	}
}

my @p_conf_items = keys %config_to_change;
for my $item (@p_conf_items) {
    print "$item";
    my $revert_conf=0;
    my $config_item=$config_to_change{$item};
    my $config_value='enable';
    if ( substr($config_to_change{$item}, 0, 1) eq '!' ) {
        $revert_conf=1;
        $config_item=substr($config_to_change{$item},1);
    }
    if ( my $i = $p->get($item) ) {
        if ( $revert_conf == 1 ) {
            $config_value='disable';
        }
    } else {
        if ( $revert_conf == 0 ) {
            $config_value='disable';
        }
    }
    print "$config_value  $item => $config_item\n";
    system($wladjust_pl,$router_dir,$config_file,$config_item,$config_value);
}

copy($config_file,$dot_config);
system($utility_sh,$build_dir,$router_dir);
