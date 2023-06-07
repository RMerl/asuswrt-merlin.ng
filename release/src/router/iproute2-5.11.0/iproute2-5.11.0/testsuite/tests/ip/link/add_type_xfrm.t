#!/bin/sh

. lib/generic.sh

ts_log "[Testing Add XFRM Interface, With IF-ID]"

PHYS_DEV="lo"
NEW_DEV="$(rand_dev)"
IF_ID="0xf"

ts_ip "$0" "Add $NEW_DEV xfrm interface"    link add dev $NEW_DEV type xfrm dev $PHYS_DEV if_id $IF_ID

ts_ip "$0" "Show $NEW_DEV xfrm interface"   -d link show dev $NEW_DEV
test_on "$NEW_DEV"
test_on "if_id $IF_ID"

ts_ip "$0" "Del $NEW_DEV xfrm interface"   link del dev $NEW_DEV


ts_log "[Testing Add XFRM Interface, No IF-ID]"

PHYS_DEV="lo"
NEW_DEV="$(rand_dev)"
IF_ID="0xf"

ts_ip "$0" "Add $NEW_DEV xfrm interface"    link add dev $NEW_DEV type xfrm dev $PHYS_DEV

ts_ip "$0" "Show $NEW_DEV xfrm interface"   -d link show dev $NEW_DEV
test_on "$NEW_DEV"
test_on_not "if_id $IF_ID"

ts_ip "$0" "Del $NEW_DEV xfrm interface"   link del dev $NEW_DEV
