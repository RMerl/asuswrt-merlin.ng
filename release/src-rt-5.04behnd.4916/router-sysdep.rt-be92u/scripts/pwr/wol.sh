#!/bin/sh

if [ S$1 = S ]; then
	echo "Please specify the interface name."
	echo "Usage: $0 <eth> [<xx:yy:zz:aa:bb:cc>] [sw|int]"
	echo "        <eth>: the interface name"
	echo "        [<xx:yy:zz:aa:bb:cc>]: the mac address, optional"
	echo "        [int|sw]: WOL implemented in internal PHY or Switch"
	exit 2;
fi

ETH=$1
WOL_ST=""
ENET_WOL_ST=/proc/driver/enet/wol_status
ARCHER_WOL_ST=/proc/driver/archer/wol_status
MAC_PATTERN='([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})'

if [ ! -e $ARCHER_WOL_ST ] && [ ! -e $ENET_WOL_ST ]; then
	if [[ ! S$2 =~ $MAC_PATTERN ]]; then
		ethctl $ETH wol-enable;
	else
		MAC=$2;
		ethctl $ETH wol-enable mac $MAC;
	fi
	exit 1;
fi

# set the system to power profile for WOL mode
pwrctl config --all wol
if [ -e $ARCHER_WOL_ST ]; then
    #Configure the interface to detect magic packet
    archer mpd_cfg --int $ETH

    # put network interface to WOL mode
    echo "entering wol mode for interface $ETH"
    archer wol_enter --int $ETH
    WOL_ST=$ARCHER_WOL_ST
else
    if [[ ! S$2 =~ $MAC_PATTERN ]]; then
	ethctl $ETH wol-enable mpd $2;
    else
	MAC=$2;
	ethctl $ETH wol-enable mpd $3 mac $MAC;
    fi
    WOL_ST=$ENET_WOL_ST
fi

# access driver proc entry, wait till none of the systemport interface is in WOL mode
count=$(grep -c "WOL" $WOL_ST)
while [ $count -ne 0 ]
do
sleep 1
count=$(grep -c "WOL" $WOL_ST)
done

# Return system to idle power state
echo "wol test interface $ETH completed"
pwrctl config --all idle
