#!/bin/sh

# set the system to power profile for WOL mode
pwrctl config --all wol

# put network interface to WOL mode
echo "entering wol mode for interface $1"
archer wol_enter --int $1

# access driver proc entry, wait till none of the systemport interface is in WOL mode
count=$(grep -c "WOL" /proc/driver/archer/wol_status)
while [ $count -ne 0 ]
do
sleep 1
count=$(grep -c "WOL" /proc/driver/archer/wol_status)
done

# Return system to idle power state
echo "wol test interface $1 completed"
pwrctl config --all idle
