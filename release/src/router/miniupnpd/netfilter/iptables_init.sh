#! /bin/sh
# $Id: iptables_init_and_clean.sh,v 1.7 2017/04/21 11:16:09 nanard Exp $
# Improved Miniupnpd iptables init script.
# Checks for state of filter before doing anything..

EXT=1
. $(dirname "$0")/miniupnpd_functions.sh

if [ "$NDIRTY" = "${CHAIN}Chain" ]; then
	echo "Nat table dirty; Cleaning..."
elif [ "$NDIRTY" = "Chain" ]; then
	echo "Dirty NAT chain but no reference..? Fixing..."
	#$IPTABLES -t nat -A PREROUTING -d $EXTIP -i $EXTIF -j $CHAIN
	$IPTABLES -t nat -A PREROUTING -i $EXTIF -j $CHAIN
else
	echo "NAT table clean..initalizing.."
	$IPTABLES -t nat -N $CHAIN
	#$IPTABLES -t nat -A PREROUTING -d $EXTIP -i $EXTIF -j $CHAIN
	$IPTABLES -t nat -A PREROUTING -i $EXTIF -j $CHAIN
fi
if [ "$CLEAN" = "yes" ]; then
	$IPTABLES -t nat -F $CHAIN
fi

if [ "$FDIRTY" = "${CHAIN}Chain" ]; then
	echo "Filter table dirty; Cleaning..."
elif [ "$FDIRTY" = "Chain" ]; then
	echo "Dirty filter chain but no reference..? Fixing..."
	$IPTABLES -t filter -A FORWARD -i $EXTIF ! -o $EXTIF -j $CHAIN
else
	echo "Filter table clean..initalizing.."
	$IPTABLES -t filter -N MINIUPNPD
	$IPTABLES -t filter -A FORWARD -i $EXTIF ! -o $EXTIF -j $CHAIN
fi
if [ "$CLEAN" = "yes" ]; then
	$IPTABLES -t filter -F $CHAIN
fi
