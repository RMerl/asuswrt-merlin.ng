#!/bin/sh
# try to delete the chains dedicated to miniupnpd
# will fail if the jump xxx are still present in other chains.

. "$(dirname "$0")/miniupnpd_functions.sh"

# Prerouting
$NFT delete chain $af $NAT_TABLE $PREROUTING_CHAIN
# Postrouting
$NFT delete chain $af $NAT_TABLE $POSTROUTING_CHAIN
# Filter
$NFT delete chain $af $TABLE $CHAIN
