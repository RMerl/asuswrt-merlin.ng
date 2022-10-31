#!/bin/sh

if [ S$1 = S ]; then
	echo "Please specify the interface name."
	echo "Usage: $0 <eth> [<xx:yy:zz:aa:bb:cc>]"
	echo "        <eth>: the interface name"
	echo "        [<xx:yy:zz:aa:bb:cc>]: the mac address, optional"
	exit 2;
fi

ETH=$1
ARCHER_WOL_ST=/proc/driver/archer/wol_status

if ! [ -e $ARCHER_WOL_ST ]; then
	if [ S$2 = S ]; then
		ethctl $ETH wol-sleep;
	else
		MAC=$2;
		ethctl $ETH wol-sleep mac $MAC;
	fi
	exit 1;
fi

# set the system to power profile for WOL mode
pwrctl config --all wol

# put network interface to WOL mode
echo "entering wol mode for interface $ETH"
archer wol_enter --int $ETH

# access driver proc entry, wait till none of the systemport interface is in WOL mode
count=$(grep -c "WOL" $ARCHER_WOL_ST)
while [ $count -ne 0 ]
do
sleep 1
count=$(grep -c "WOL" $ARCHER_WOL_ST)
done

# Return system to idle power state
echo "wol test interface $ETH completed"
pwrctl config --all idle
