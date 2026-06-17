#!/bin/sh
# vim: set sw=4 ts=4 expandtab:
#
# establish the chains that miniupnpd will update dynamically
#
# 'add' doesn't raise an error if the object already exists. 'create' does.
#

. "$(dirname "$0")/miniupnpd_functions.sh"

# create chain if it doesn't exist
# arguments : existing_chains, table, chain, params
create_chain() {
    if echo $1 | grep -w -q $3; then
        echo "Chain $3 already exists in table $2"
    else
        $NFT create chain $af $2 $3 $4 || exit 1
    fi
}

# create table if it doesn't exist
# arguments : existing_tables, table
create_table() {
    if echo $1 | grep -w -q $2; then
        echo "Table $2 already exists"
    else
        $NFT create table $af $2 \
            '{comment "created by miniupnpd init script";}' || exit 1
    fi
}

# arguments : table chain jump_dest
insert_jump() {
    if ! $NFT list chain $af $1 $2 | \
        sed -n '/^\t\tjump /p' | grep -q "jump $3"
    then
        $NFT insert rule $af $1 $2 "jump $3" || exit 1
    fi
}

existing_tables=$($NFT list tables $af | cut -d' ' -f3)
create_table "$existing_tables" $TABLE

existing_chains=$($NFT list table $af $TABLE | sed -n 's/^\tchain \([^ ]*\).*/\1/p')
create_chain "$existing_chains" $TABLE forward \
    '{type filter hook forward priority 0; policy drop; comment "created by miniupnpd init script";}'
create_chain "$existing_chains" $TABLE $CHAIN

insert_jump $TABLE forward $CHAIN

if [ "$TABLE" != "$NAT_TABLE" ] ; then
    create_table "$existing_tables" $NAT_TABLE
fi

existing_chains=$($NFT list table $af $NAT_TABLE | sed -n 's/^\tchain \([^ ]*\).*/\1/p')
create_chain "$existing_chains" $NAT_TABLE prerouting \
    '{type nat hook prerouting priority -100; policy accept; comment "created by miniupnpd init script";}'
create_chain "$existing_chains" $NAT_TABLE postrouting \
    '{type nat hook postrouting priority 100; policy accept; comment "created by miniupnpd init script";}'
create_chain "$existing_chains" $NAT_TABLE $PREROUTING_CHAIN
create_chain "$existing_chains" $NAT_TABLE $POSTROUTING_CHAIN

insert_jump $NAT_TABLE prerouting $PREROUTING_CHAIN
insert_jump $NAT_TABLE postrouting $POSTROUTING_CHAIN
