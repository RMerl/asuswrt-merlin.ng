#!/bin/sh
#
# Undo the things nft_init.sh did
#
# Do not disturb other existing structures in nftables, e.g. those created by firewalld
#

nft --check list table nat > /dev/null 2>&1
if [ $? -eq "0" ]; then
{
	# nat table exists, so first remove the chains we added
	nft --check list chain nat MINIUPNPD > /dev/null 2>&1
	if [ $? -eq "0" ]; then
		echo "Remove chain from nat table"
		nft delete chain nat MINIUPNPD
	fi

	nft --check list chain nat MINIUPNPD-POSTROUTING > /dev/null 2>&1
	if [ $? -eq "0" ]; then
		echo "Remove pcp peer chain from nat table"
		nft delete chain nat MINIUPNPD-POSTROUTING
	fi

	# then remove the table itself
	echo "Remove nat table"
	nft delete table nat
}
fi

nft --check list table inet filter > /dev/null 2>&1
if [ $? -eq "0" ]; then
{
	# filter table exists, so first remove the chain we added
	nft --check list chain inet filter MINIUPNPD > /dev/null 2>&1
	if [ $? -eq "0" ]; then
		echo "Remove chain from filter table"
		nft delete chain inet filter MINIUPNPD
	fi

	# then remove the table itself
	echo "Remove filter table"
	nft delete table inet filter
}
fi
