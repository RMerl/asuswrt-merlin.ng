#!/bin/bash

NUM_WL_INTF=`ls -al /sys/class/net/wl* 2>&1 | grep -c "wl. "`
NUM_PROCESSOR=`cat /proc/cpuinfo | grep -c "processor"`

#$(getStrByPos "12 33 44" " " 2) will return 33
getStrByPos() {
        local num=$3;
        IFS="$2"
        set -- $1
        eval result='$'$num;
        echo $result;
}

getPidByName()
{
    local pid=$(getStrByPos "`ps -a | grep -v grep | grep $1`" " " 1);
    if [ -z $pid ]; then
        echo 0;
    else
        echo $pid;
    fi
}

process_name()
{
    local radio=$2;
    case "$1" in
        wl)
            echo "wl${radio}-kthrd"
            ;;
        wfd)
            echo "wfd${radio}-thrd";
            ;;
    esac
}

setProcAffinity() {
    local nvram_name=$1
    local proc_name=$2
    local pid=$3
    local affinity=$4
    echo "set $proc_name $pid to aff:$affinity"
    taskset -p $affinity $pid> /dev/null 2>&1
    if [ "$?" != "0" ]; then
        echo "pin $proc_name failed, rc=$?, pid_wl0=$pid"
    fi
    #nvram kset $nvram_name=$affinity
}

disable_accelerators() {
	# this command disables runner/fc on 49408/508 and
	# archer accel on 47622.
	if type fcctl > /dev/null 2>&1 ; then
		fcctl disable
	fi
}

# If we are on a 47622, and we are running an EAP profile
# (which we are, that's the only way this file gets included)
# then archer traffic management is:
# - enabled if we detect a switch
# - disabled if we don't.
# This is due to head-of-line blocking issue when transmitting
# to Ethernet ports with different speeds or congestion levels.
configure_tm() {
	if ! type archerctl > /dev/null 2>&1 ; then
		return
	fi

	if ethswctl -c switchinfo -v 1 > /dev/null 2>&1; then
		archerctl sysport_tm enable
	else
		archerctl sysport_tm disable
	fi
}

disable_accelerators
configure_tm

radio=0
while [ $radio -lt $NUM_WL_INTF ];
do
    # bind to CPU{x}
    affinity=$((2 << (radio%NUM_PROCESSOR)))
    for ctl_item in wl wfd
    do
        proc_name=$(process_name $ctl_item $radio);
        proc_pid=$(getPidByName $proc_name);
        if [ $proc_pid -gt 0 ]; then
            setProcAffinity "" $proc_name $proc_pid $affinity
        fi
    done

    # bind irq to CPU{x}
    wl=wl$radio
    irq=`cat /proc/interrupts | grep $wl | cut -d" " -f2 | cut -d":" -f1`
    echo $affinity > /proc/irq/$irq/smp_affinity

    radio=$((radio+1))
done

# bind RX task to CPU0
bcmrx_pid=$(getPidByName "bcmsw_rx")
if [ $bcmrx_pid -gt 0 ]; then
    setProcAffinity "" "bcmsw_rx" $bcmrx_pid 1
fi

# bind SKB free task to CPU3
skbfree_pid=$(getPidByName "skb_free_task")
if [ $skbfree_pid -gt 0 ]; then
    setProcAffinity "" ""skb_free_task"" $skbfree_pid 8
fi
