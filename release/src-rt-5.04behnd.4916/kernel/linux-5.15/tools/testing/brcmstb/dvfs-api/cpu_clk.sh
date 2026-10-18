#!/bin/sh

# Cycle through available CPU clock frequencies using the userspace
# governor.

F_FREQS=/sys/devices/system/cpu/cpufreq/policy0/scaling_available_frequencies
F_GOV=/sys/devices/system/cpu/cpufreq/policy0/scaling_governor
F_CUR_FREQ=/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_cur_freq
F_SET_FREQ=/sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed

for i in $F_FREQS $F_GOV $F_CUR_FREQ $F_SET_FREQ ; do
    if [ ! -f $i ] ; then
	echo "Cannot find file '$i'"
	exit 1;
    fi
done


old_gov=$(cat $F_GOV)
echo userspace > $F_GOV

freqs=$(cat $F_FREQS)
echo "Available freq: $freqs"
echo "Current freq: $(cat $F_CUR_FREQ)"
echo
sleep 1

cur=$(cat $F_CUR_FREQ)
for i in $freqs $freqs ; do
    echo -n "Changing current freq $cur to $i "
    echo $i > $F_SET_FREQ
    sleep 1
    cur=$(cat $F_CUR_FREQ)
    if [ $cur = $i ] ; then
	echo "=> PASS";
    else
	echo "=> FAIL";
	echo $old_gov > $F_GOV
	exit 1
    fi
done

echo $old_gov > $F_GOV
exit 0
