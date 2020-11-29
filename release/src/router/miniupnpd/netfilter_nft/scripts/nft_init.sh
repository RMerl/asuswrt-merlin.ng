#!/bin/sh
#
# establish the chains that miniupnpd will update dynamically
#
# 'add' doesn't raise an error if the object already exists. 'create' does.
#

#opts="--echo"

echo "create nat table"
nft ${opts} add table nat

echo "create chain in nat table"
nft ${opts} add chain nat MINIUPNPD

echo "create pcp peer chain in nat table"
nft ${opts} add chain nat MINIUPNPD-POSTROUTING

echo "create filter table"
nft ${opts} add table inet filter

echo "create chain in filter table"
nft ${opts} add chain inet filter MINIUPNPD
