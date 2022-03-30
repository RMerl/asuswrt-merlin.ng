# -*-tcl-*-
#
# Testbed configuration file for utf25 testbed
#
# ==============
# Load Packages
#===============
#package require UTF::Aeroflex
#package require UTF::AeroflexDirect
package require UTF
package require UTF::utils
package require UTF::Linux
package require UTF::STB
package require UTFD
#package require UTF::Sniffer
#package require UTF::TclReadLines

set UTF::Use11h 1;
set UTF::UseCSA 1;
set UTF::WebTree 1
set UTF::DataRatePlot 1
# set UTF::projswbuild /projects/bca/swbuild
set UTF::projswbuild /projects/{hnd,bca}/swbuild
#set nvramDM "devicemode=1 "
#set nvramCTDMA "devpath1=pcie/1/1/ 1:ctdma=1 devpath2=pcie/2/3/ 2:ctdma=1 3:ctdma=1"
#set nvramMUauto "wl0_mu_features=0x8000 wl1_mu_features=0x8000 wl2_mu_features=0x8000"
#set nvramMU "wl0_mu_features=1 wl1_mu_features=1 wl2_mu_features=1"
set ::wan_peer_sta_list {}
set conf1x1 "wl rxchain 1; wl txchain 1"

#If you want EmbeddedNightly.test to do log parsing, you need to add the statement below to your config file. This is in addition to the above statement.

set ::UTF::PostTestHook {
    package require UTF::utils
    UTF::do_post_test_analysis [info script] ""}

#To use LSF compute farm to offload your endpoint, add:

set ::aux_lsf_queue sj-hnd

# Optional items for controlchart.test to run each iteration
#set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results

#set ::UTF::SummaryDir "/projects/hnd_svt_ap9/$::env(LOGNAME)/utf11"
set ::UTF::SummaryDir "/projects/hnd_swa_access_ext4/work/$::env(LOGNAME)/UTF/utf25"

# Define power controllers on cart
#UTF::Power::Synaccess utf25npc1 -lan_ip 172.16.1.44 -relay 10.19.87.227 -rev 1
#UTF::Power::Synaccess utf25npc2 -lan_ip 172.16.1.45 -relay 10.19.87.227 -rev 1
#UTF::Power::Synaccess utf25npc3 -lan_ip 172.16.1.46 -relay 10.19.87.227

# Attenuator - Aeroflex
#UTF::Aeroflex af -lan_ip 172.16.1.30 \
#    -group {G1 {5 6 7 8} ALL{5,6,7,8}}
#ALL {1 2 3 4 5 6 7 8}

# For 3390/4366/4366 AP run
#G1 configure -default 25

#---------------------------------------
# Perfcache migrate between objects old -> new

set UTF::PerfCacheMigrate {
	3390_2g 3390/4366_2g
	3390_5g 3390/4366_5g
}

#---------------------------------------
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    #ALL attn default;
#    G1 attn default;

#ASKSD
#    foreach S {4366mcc0tst3} {
#        catch {$S wl down}
#        $S deinit
#    }

    foreach dev "10.19.87.227 utf25-sta1" {
        catch {
        $dev sysctl -w net.core.rmem_max="16000001"
        $dev sysctl -w net.core.wmem_max="16000000"
        $dev sysctl -w net.ipv4.tcp_rmem="4096 87380 16000000"
        $dev sysctl -w net.ipv4.tcp_wmem="4096 87380 16000000"
        $dev sysctl -w net.core.netdev_max_backlog="3000"
        $dev echo bic > /proc/sys/net/ipv4/tcp_congestion_control
        }
    }

    # delete myself
    unset ::UTF::SetupTestBed

    return
}

# -----------------------------------------
# UTF Endpoints f19 - Traffic generators (no wireless cards)
UTF::Linux 10.19.87.227 -lan_ip 10.19.87.227 \
    -sta {lan enp0s26u1u2}

lan configure -ipaddr 10.0.0.157

#UTF::Linux 10.19.94.250 -lan_ip 10.19.94.250 \
#    -sta {lan3 enp0s20u2}

#UTF::Linux 10.19.94.158 -lan_ip 10.19.94.158 \
#    -sta {lan2 enp0s26u1u2}

#lan2 configure -ipaddr 10.0.0.166

#        -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl bw_cap 2g -1; wl up;}

UTF::Linux utf25-sta1 -lan_ip 10.19.87.234 \
     -sta {4360sta1 enp2s0}

lan configure -ipaddr 10.0.0.86
#7445/4360 configure -ipaddr 192.168.1.99 -lanpeer {7445/43602 7445/4360}

# -----------------------------------------------------------------------
# Perfhook variables

set internal_perf_hooks_nVn {
    -pre_perf_hook {{%S wl assoc} {%S wl reset_cnts} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl counters} {%S wl dump ampdu} \
    {%S wl sta_info [4366mcc0tst3 macaddr]} {%S wl rssi [4366mcc0tst3 macaddr]}} \
    -wlinitcmds {wl msglevel +error +assoc; cat /proc/version; brctl show;}
}

set external_perf_hooks_nVn {
    -pre_perf_hook {{%S wl assoclist} {%S wl reset_cnts}} \
    -post_perf_hook {{%S wl nrate} {%S wl dump rssi} {%S wl counters}} \
    -wlinitcmds {wl msglevel +error +assoc; cat /proc/version; brctl show;}
}

set internal_perf_hooks {
    -pre_perf_hook {{%S wl assoc} {%S wl reset_cnts} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl counters} {%S wl dump ampdu} \
    {%S wl assoclist} {%S wl sta_info [4366mcc0tst3 macaddr]} {%S wl rssi [4366mcc0tst3 macaddr]}} \
    -wlinitcmds {wl msglevel + error; cat /proc/version; brctl show;}
}

set external_perf_hooks {
    -pre_perf_hook {{%S wl assoclist} {%S wl reset_cnts}} \
    -post_perf_hook {{%S wl nrate} {%S wl dump rssi} {%S wl counters}} \
    {%S wl sta_info [4366mcc0tst3 macaddr]}
}

set rdkm_tag_3390_4366 {
    -model "3390" \
    -cmtag "CMWIFI" \
    -cmtag2 "93390smwvg" \
    -brand "cmwifi_4366_3390b0" \
    -type "rdkm" \
    -cmrelease "18.3" \
    -cmradio "3390_4366_4366" \
    -buildserver "/projects/bcawlan_builds/Irvine"
}

# ---------------------------------------------
# CableModem Objects in order
# 3390

package require UTF::CableModem
package require UTF::DSL

########################################################################################################################################
#----------------------------------------------   O B J E C T     D E F I N I T I O N -------------------------------------------------#
#    -mpstat 1 \
########################################################################################################################################
# ----------------------------- 3390  for lattice bridge firmware AP=> 3390 with 4366(dualband) 4366(2g) --------------------#

#-lan_ip 10.0.0.1 \
#3390 BCM9436660DCMX2_v0
UTF::CableModem 3390 -sta {3390_4366 wl0 3390_4366.%7 wl0.% 3390_4366 wl1 3390_4366.%7 wl1.%} \
    -lan_ip 10.0.0.1 \
    -relay "10.19.87.227" -lanpeer {lan} \
    -tag "4366" \
    -date "2019.04.01" \
    -console "10.19.87.227:40000" \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1} \
    -nomaxmem 1 -nocustom 1 \
    -nvram {
        wl0_chanspec=6
        wl0_ssid=ssid_test_2g
        wl1_chanspec=36
        wl1_ssid=ssid_test_5g
    } \
    -wlinitcmds {wl msglevel + error; cat /proc/version; brctl show;}

3390_4366 clone 3390_4366_5g -sta {3390_4366_5g wl1 3390_4366_5g.%7 wl1.%} -perfchans {36/80 36l 36} {*}$internal_perf_hooks_nVn
3390_4366 clone 3390_4366_2g -sta {3390_4366_2g wl0 3390_4366_2g.%7 wl0.%} -perfchans {6l 6} {*}$internal_perf_hooks_nVn

3390_4366_5g configure -dualband {3390/4366_2g -c1 36/80 -c2 6l -lan1 lan -lan2 lan2}
#3390_4366_5g configure -dualband {3390/4366_2g -c1 36/80 -c2 6l -lan1 lan}

#3390_4366_5g clone rdkm_3390/66-5g_tob -bridge1 "brlan0" -bridge2 "brlan1" {*}$rdkm_nightly_3390_4366
#3390_4366_2g clone rdkm_3390/66-2g_tob -bridge1 "brlan0" -bridge2 "brlan1" {*}$rdkm_nightly_3390_4366
#3390_4366_5g clone rdkm_3390/66-5g_tag -bridge1 "brlan0"  {*}$rdkm_tag_3390_4366
#3390_4366_2g clone rdkm_3390/66-2g_tag -bridge1 "brlan0"  {*}$rdkm_tag_3390_4366

#set UTF::RouterNightlyCustom {

#    if {$STA3 ne "" && [regexp {(.*x)/0} $Router - base]} {

#	package require UTF::Test::TripleBand

#    TripleBand ${base}/0 ${base}/1 ${base}/2 $STA1 $STA2 $STA3 \
#        -c1 44/80 -c2 3l -c3 157/80 -lan1 lan -lan2 lan2

#    }
#}

# ChannelSweep with up/dn traffic
set UTF::ChannelPerf 1
set UTF::KPPSSweep 1

UTF::Q utf25 10.19.87.227
