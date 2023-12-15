/*
 * common.h - common code header
 *
 * Data and functions shared by ioctl and netlink implementation.
 */

#include "internal.h"
#include "common.h"

#ifndef HAVE_NETIF_MSG
enum {
	NETIF_MSG_DRV		= 0x0001,
	NETIF_MSG_PROBE		= 0x0002,
	NETIF_MSG_LINK		= 0x0004,
	NETIF_MSG_TIMER		= 0x0008,
	NETIF_MSG_IFDOWN	= 0x0010,
	NETIF_MSG_IFUP		= 0x0020,
	NETIF_MSG_RX_ERR	= 0x0040,
	NETIF_MSG_TX_ERR	= 0x0080,
	NETIF_MSG_TX_QUEUED	= 0x0100,
	NETIF_MSG_INTR		= 0x0200,
	NETIF_MSG_TX_DONE	= 0x0400,
	NETIF_MSG_RX_STATUS	= 0x0800,
	NETIF_MSG_PKTDATA	= 0x1000,
	NETIF_MSG_HW		= 0x2000,
	NETIF_MSG_WOL		= 0x4000,
};
#endif

const struct flag_info flags_msglvl[] = {
	{ "drv",	NETIF_MSG_DRV },
	{ "probe",	NETIF_MSG_PROBE },
	{ "link",	NETIF_MSG_LINK },
	{ "timer",	NETIF_MSG_TIMER },
	{ "ifdown",	NETIF_MSG_IFDOWN },
	{ "ifup",	NETIF_MSG_IFUP },
	{ "rx_err",	NETIF_MSG_RX_ERR },
	{ "tx_err",	NETIF_MSG_TX_ERR },
	{ "tx_queued",	NETIF_MSG_TX_QUEUED },
	{ "intr",	NETIF_MSG_INTR },
	{ "tx_done",	NETIF_MSG_TX_DONE },
	{ "rx_status",	NETIF_MSG_RX_STATUS },
	{ "pktdata",	NETIF_MSG_PKTDATA },
	{ "hw",		NETIF_MSG_HW },
	{ "wol",	NETIF_MSG_WOL },
	{}
};
const unsigned int n_flags_msglvl = ARRAY_SIZE(flags_msglvl) - 1;

const struct off_flag_def off_flag_def[] = {
	{ "rx",     "rx-checksumming",		    "rx-checksum",
	  ETHTOOL_GRXCSUM, ETHTOOL_SRXCSUM, ETH_FLAG_RXCSUM,	0 },
	{ "tx",     "tx-checksumming",		    "tx-checksum-*",
	  ETHTOOL_GTXCSUM, ETHTOOL_STXCSUM, ETH_FLAG_TXCSUM,	0 },
	{ "sg",     "scatter-gather",		    "tx-scatter-gather*",
	  ETHTOOL_GSG,	   ETHTOOL_SSG,     ETH_FLAG_SG,	0 },
	{ "tso",    "tcp-segmentation-offload",	    "tx-tcp*-segmentation",
	  ETHTOOL_GTSO,	   ETHTOOL_STSO,    ETH_FLAG_TSO,	0 },
	{ "ufo",    "udp-fragmentation-offload",    "tx-udp-fragmentation",
	  ETHTOOL_GUFO,	   ETHTOOL_SUFO,    ETH_FLAG_UFO,	0 },
	{ "gso",    "generic-segmentation-offload", "tx-generic-segmentation",
	  ETHTOOL_GGSO,	   ETHTOOL_SGSO,    ETH_FLAG_GSO,	0 },
	{ "gro",    "generic-receive-offload",	    "rx-gro",
	  ETHTOOL_GGRO,	   ETHTOOL_SGRO,    ETH_FLAG_GRO,	0 },
	{ "lro",    "large-receive-offload",	    "rx-lro",
	  0,		   0,		    ETH_FLAG_LRO,
	  KERNEL_VERSION(2,6,24) },
	{ "rxvlan", "rx-vlan-offload",		    "rx-vlan-hw-parse",
	  0,		   0,		    ETH_FLAG_RXVLAN,
	  KERNEL_VERSION(2,6,37) },
	{ "txvlan", "tx-vlan-offload",		    "tx-vlan-hw-insert",
	  0,		   0,		    ETH_FLAG_TXVLAN,
	  KERNEL_VERSION(2,6,37) },
	{ "ntuple", "ntuple-filters",		    "rx-ntuple-filter",
	  0,		   0,		    ETH_FLAG_NTUPLE,	0 },
	{ "rxhash", "receive-hashing",		    "rx-hashing",
	  0,		   0,		    ETH_FLAG_RXHASH,	0 },
};

void print_flags(const struct flag_info *info, unsigned int n_info, u32 value)
{
	const char *sep = "";

	while (n_info) {
		if (value & info->value) {
			printf("%s%s", sep, info->name);
			sep = " ";
			value &= ~info->value;
		}
		++info;
		--n_info;
	}

	/* Print any unrecognised flags in hex */
	if (value)
		printf("%s%#x", sep, value);
}

static char *unparse_wolopts(int wolopts)
{
	static char buf[16];
	char *p = buf;

	memset(buf, 0, sizeof(buf));

	if (wolopts) {
		if (wolopts & WAKE_PHY)
			*p++ = 'p';
		if (wolopts & WAKE_UCAST)
			*p++ = 'u';
		if (wolopts & WAKE_MCAST)
			*p++ = 'm';
		if (wolopts & WAKE_BCAST)
			*p++ = 'b';
		if (wolopts & WAKE_ARP)
			*p++ = 'a';
		if (wolopts & WAKE_MAGIC)
			*p++ = 'g';
		if (wolopts & WAKE_MAGICSECURE)
			*p++ = 's';
		if (wolopts & WAKE_FILTER)
			*p++ = 'f';
	} else {
		*p = 'd';
	}

	return buf;
}

int dump_wol(struct ethtool_wolinfo *wol)
{
	fprintf(stdout, "	Supports Wake-on: %s\n",
		unparse_wolopts(wol->supported));
	fprintf(stdout, "	Wake-on: %s\n",
		unparse_wolopts(wol->wolopts));
	if (wol->supported & WAKE_MAGICSECURE) {
		int i;
		int delim = 0;

		fprintf(stdout, "        SecureOn password: ");
		for (i = 0; i < SOPASS_MAX; i++) {
			fprintf(stdout, "%s%02x", delim ? ":" : "",
				wol->sopass[i]);
			delim = 1;
		}
		fprintf(stdout, "\n");
	}

	return 0;
}

void dump_mdix(u8 mdix, u8 mdix_ctrl)
{
	fprintf(stdout, "	MDI-X: ");
	if (mdix_ctrl == ETH_TP_MDI) {
		fprintf(stdout, "off (forced)\n");
	} else if (mdix_ctrl == ETH_TP_MDI_X) {
		fprintf(stdout, "on (forced)\n");
	} else {
		switch (mdix) {
		case ETH_TP_MDI:
			fprintf(stdout, "off");
			break;
		case ETH_TP_MDI_X:
			fprintf(stdout, "on");
			break;
		default:
			fprintf(stdout, "Unknown");
			break;
		}
		if (mdix_ctrl == ETH_TP_MDI_AUTO)
			fprintf(stdout, " (auto)");
		fprintf(stdout, "\n");
	}
}

void print_indir_table(struct cmd_context *ctx, u64 ring_count,
		       u32 indir_size, u32 *indir)
{
	u32 i;

	printf("RX flow hash indirection table for %s with %llu RX ring(s):\n",
	       ctx->devname, ring_count);

	if (!indir_size)
		printf("Operation not supported\n");

	for (i = 0; i < indir_size; i++) {
		if (i % 8 == 0)
			printf("%5u: ", i);
		printf(" %5u", indir[i]);
		if (i % 8 == 7 || i == indir_size - 1)
			fputc('\n', stdout);
	}
}

void print_rss_hkey(u8 *hkey, u32 hkey_size)
{
	u32 i;

	printf("RSS hash key:\n");
	if (!hkey_size || !hkey)
		printf("Operation not supported\n");

	for (i = 0; i < hkey_size; i++) {
		if (i == (hkey_size - 1))
			printf("%02x\n", hkey[i]);
		else
			printf("%02x:", hkey[i]);
	}
}
