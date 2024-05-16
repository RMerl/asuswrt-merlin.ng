#!/bin/bash

. /bin/wlinc.sh

disable_accelerators() {
	# this command disables runner/fc on 49408/508 and
	# archer accel on 47622.
	if type fcctl > /dev/null 2>&1 ; then
		fcctl disable
		fcctl flush
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
	if [ -z "$ARCHER" ]; then
		return
	fi

	if ethswctl -c switchinfo -v 1 > /dev/null 2>&1; then
		archerctl sysport_tm enable
	else
		archerctl sysport_tm disable
	fi
}

# Try to set affinity for the IRQ. If it doesn't work, say, because
# the interrupt is not tied to the GIC, do it for its interrupt
# controller node.
set_irq_affinity() {
	# avoid disaster if we reached the end
	if [[ "$1" == *"GIC"* ]]; then
		return
	fi

	irq=`grep "$1$" /proc/interrupts | cut -d" " -f2 | cut -d":" -f1`

	if [ -z "$irq" ]; then
		return
	fi

	# if the command fails, try its parent.
	if ! echo "$2" > /proc/irq/$irq/smp_affinity 2> /dev/null; then
		controller=`grep "$1$" /proc/interrupts | sed -r 's/\s+/;/g' | cut -d";" -f7`
		set_irq_affinity "$controller" "$2"
	fi
}

disable_accelerators
configure_tm

radio=0
while [ $radio -lt $NUM_WL_INTF ];
do
    # bind to CPU{x}
    affinity=$((2 << (radio%NUM_PROCESSOR)))
    for ctl_item in ${PROC_LIST_PER_RADIO}
    do
        proc_name=$(get_process_name $ctl_item $radio);
        proc_pid=$(getPidByName $proc_name);
        if [ $proc_pid -gt 0 ]; then
            setProcAffinity "" $proc_name $proc_pid $affinity
        fi
    done

    # bind irq to CPU{x}
    wl=wl$radio
    set_irq_affinity "$wl" "$affinity"
    wlm2m=wlan_${radio}_m2m
    set_irq_affinity "$wlm2m" "$affinity"

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
