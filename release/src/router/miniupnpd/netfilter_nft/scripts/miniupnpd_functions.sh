#! /bin/sh

NFT=$(which nft) || {
	echo "Can't find nft" >&2
	exit 1
}

TABLE="filter"
NAT_TABLE="filter"
CHAIN="miniupnpd"
PREROUTING_CHAIN="prerouting_miniupnpd"
POSTROUTING_CHAIN="postrouting_miniupnpd"

while getopts ":t:n:c:p:r:h" opt; do
	case $opt in
		t)
			TABLE=$OPTARG
			;;
		n)
			NAT_TABLE=$OPTARG
			;;
		c)
			CHAIN=$OPTARG
			;;
		p)
			PREROUTING_CHAIN=$OPTARG
			;;
		r)
			POSTROUTING_CHAIN=$OPTARG
			;;
		h)
			echo "Usage: $0 [options]"
			echo
			echo "Options:"
			echo "  -t table              upnp_table_name in miniupnpd.conf"
			echo "  -n nat_table          upnp_nat_table_name in miniupnpd.conf"
			echo "  -c chain              upnp_forward_chain in miniupnpd.conf"
			echo "  -p nat_chain          upnp_nat_chain in miniupnpd.conf"
			echo "  -r postrouting_chain  upnp_nat_postrouting_chain in miniupnpd.conf"
			exit 0
			;;
		\?)
			echo "Invalid option: -$OPTARG" >&2
			echo "To show usage: $0 -h" >&2
			exit 1
			;;
		:)
			echo "Option -$OPTARG requires an argument." >&2
			exit 1
			;;
	esac
done
