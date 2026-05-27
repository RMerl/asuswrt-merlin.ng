#!/bin/sh
# vim: set ts=4 sw=4:
#
# Undo the things nft_init.sh did
#
# Do not disturb other existing structures in nftables, e.g. those created by firewalld
#

. "$(dirname "$0")/miniupnpd_functions.sh"

# remove table if created by us
remove_table() {
	if $NFT list table $af $1 | sed -n '2p' | \
		grep -q 'comment "created by miniupnpd init script"'
	then
		$NFT delete table $af $1 || exit 1
	else
		echo "$1 was not created by miniupnpd init script"
	fi
}

existing_tables=$($NFT list tables $af | cut -d' ' -f3)

if echo $existing_tables | grep -w -q $TABLE ; then
	remove_table $TABLE
fi
if [ "$TABLE" != "$NAT_TABLE" ] ; then
	if echo $existing_tables | grep -w -q $NAT_TABLE ; then
		remote_table $NAT_TABLE
	fi
fi
