#!/bin/sh

showUsage() {
    echo "Usage: $0 <suspend|resume>  [-t <wan|lan|all>] [-l level]"
    echo "   level:  1: minimimal powerdown"
    echo "           9: kill switch and stop (requires reset to recover)"
    echo
}

killSwitch() {
    if [ $1 == "up" ]; then
        echo "RESETTING BOARD"
        reboot;
    else
        echo "WARNING - POWERING DOWN SWITCH - RESET REQUIRED TO POWER SWITCH BACK UP"
        mod='wl dhd wfd bcm63xx_pcie pwrmngtd bcm_enet rdpa_cmd rdpa_mw rdpa '
        pwrctl stop
        killall -q acsd mcpd swmdk
        for m in $mod; do echo \"Removing module $m\"; rmmod -w $m; done
        echo 0 > /sys/power/bpcm/rdp/sr_control
        for f in /sys/power/bpcm/rdp/zone?/power; do echo 0 > $f; done
        for f in /sys/power/bpcm/switch/zone[02]/power; do echo 0 > $f; done
    fi
}

powerLan() {
    eths="$(brctl show br0 | grep -o "eth[0-9]\+")";
    for intf in ${eths}; do
        echo "powering $1 ${intf}"
        ethctl ${intf} phy-power $1
    done
}

powerWan() {
    if [ $1 == "up" ]; then
        # Note: this makes an assumption that the default power setting for serdes is 1
        ethctl phy serdespower eth0 1;
    else
        ethctl phy serdespower eth0 3;
    fi
}

doError() {
    [ $# -gt 0 ] && msg=$1;
    echo "Error: $msg"
    echo
    showUsage
    exit 1
}

[ $# -gt 0 ] || doError "Missing Parameters"


[ $# -gt 0 ] || doError "Expecting suspend or resume"
case $1 in 
    suspend)           suspendOrResume=$1; upOrDown="down" shift;;
    resume)            suspendOrResume=$1; upOrDown="up" shift;;
    *)                 doError "Unknown subcomand $1"
esac

#check for optional parameters:
level=1;
type="all"

#getopts not supported on modem shell
while [ $# -gt 0 ]; do
    case $1 in
        -l)  level=$2; shift;;
        -t)  type=$2; shift;;
        -d)  debug=y;;
        *)  doError "Unknown flag $FLAG";;
    esac
    shift
done

case $level in
    1|9)    ;;
    *)      doError "unknown level $level"
esac

case $type in
    wan)    doLan=""; doWan=y;;
    lan)    doWan=""; doLan=y;;
    all)    doWan=y;doLan=y;;
    *)      doError "Unknown type \"$type\"";;
esac

#do power:

case $level in
    1)  [ "$doLan" = "y" ] && powerLan $upOrDown $debug
        [ "$doWan" = "y" ] && powerWan $upOrDown $debug;;            
    9)  killSwitch $upOrDown $debug;;
    *)  doError "level $level not supported";;
esac


