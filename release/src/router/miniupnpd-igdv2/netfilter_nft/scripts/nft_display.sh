#!/bin/sh
# display rules added by miniupnpd in its dedicated chains

. "$(dirname "$0")/miniupnpd_functions.sh"

# Prerouting
$NFT list chain $af $NAT_TABLE $PREROUTING_CHAIN
# Postrouting
$NFT list chain $af $NAT_TABLE $POSTROUTING_CHAIN
# Filter
$NFT list chain $af $TABLE $CHAIN
