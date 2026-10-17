// SPDX-License-Identifier: GPL-2.0+
/*
 * NC-SI protocol configuration
 *
 * Copyright (C) 2019, IBM Corporation.
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <phy.h>
#include <net.h>
#include <net/ncsi.h>
#include <net/ncsi-pkt.h>
#include <asm/unaligned.h>

#define NCSI_PACKAGE_MAX 8
#define NCSI_CHANNEL_MAX 31

#define NCSI_PACKAGE_SHIFT      5
#define NCSI_PACKAGE_INDEX(c)   (((c) >> NCSI_PACKAGE_SHIFT) & 0x7)
#define NCSI_RESERVED_CHANNEL   0x1f
#define NCSI_CHANNEL_INDEX(c)   ((c) & ((1 << NCSI_PACKAGE_SHIFT) - 1))
#define NCSI_TO_CHANNEL(p, c)   (((p) << NCSI_PACKAGE_SHIFT) | (c))

#define NCSI_PKT_REVISION       0x01

#define NCSI_CAP_GENERIC_MASK	0x7f
#define NCSI_CAP_BC_MASK	0x0f
#define NCSI_CAP_MC_MASK	0x3f
#define NCSI_CAP_AEN_MASK	0x07
#define NCSI_CAP_VLAN_MASK	0x07

static void ncsi_send_ebf(unsigned int np, unsigned int nc);
static void ncsi_send_ae(unsigned int np, unsigned int nc);
static void ncsi_send_gls(unsigned int np, unsigned int nc);
static int ncsi_send_command(unsigned int np, unsigned int nc, unsigned int cmd,
			     uchar *payload, int len, bool wait);

struct ncsi_channel {
	unsigned int	id;
	bool		has_link;

	/* capabilities */
	u32 cap_generic;
	u32 cap_bc;
	u32 cap_mc;
	u32 cap_buffer;
	u32 cap_aen;
	u32 cap_vlan;

	/* version information */
	struct {
		u32 version;            /* Supported BCD encoded NCSI version */
		u32 alpha2;             /* Supported BCD encoded NCSI version */
		u8  fw_name[12];        /* Firmware name string               */
		u32 fw_version;         /* Firmware version                   */
		u16 pci_ids[4];         /* PCI identification                 */
		u32 mf_id;              /* Manufacture ID                     */
	} version;

};

struct ncsi_package {
	unsigned int		id;
	unsigned int		n_channels;
	struct ncsi_channel	*channels;
};

struct ncsi {
	enum {
		NCSI_PROBE_PACKAGE_SP,
		NCSI_PROBE_PACKAGE_DP,
		NCSI_PROBE_CHANNEL_SP,
		NCSI_PROBE_CHANNEL,
		NCSI_CONFIG,
	} state;

	unsigned int	pending_requests;
	unsigned int	requests[256];
	unsigned int	last_request;

	unsigned int	current_package;
	unsigned int	current_channel;

	unsigned int		n_packages;
	struct ncsi_package	*packages;
};

struct ncsi *ncsi_priv;

bool ncsi_active(void)
{
	unsigned int np, nc;

	if (!ncsi_priv)
		return false;

	np = ncsi_priv->current_package;
	nc = ncsi_priv->current_channel;

	if (ncsi_priv->state != NCSI_CONFIG)
		return false;

	return np < NCSI_PACKAGE_MAX && nc < NCSI_CHANNEL_MAX &&
		ncsi_priv->packages[np].channels[nc].has_link;
}

static unsigned int cmd_payload(int cmd)
{
	switch (cmd) {
	case NCSI_PKT_CMD_CIS:
		return 0;
	case NCSI_PKT_CMD_SP:
		return 4;
	case NCSI_PKT_CMD_DP:
		return 0;
	case NCSI_PKT_CMD_EC:
		return 0;
	case NCSI_PKT_CMD_DC:
		return 4;
	case NCSI_PKT_CMD_RC:
		return 4;
	case NCSI_PKT_CMD_ECNT:
		return 0;
	case NCSI_PKT_CMD_DCNT:
		return 0;
	case NCSI_PKT_CMD_AE:
		return 8;
	case NCSI_PKT_CMD_SL:
		return 8;
	case NCSI_PKT_CMD_GLS:
		return 0;
	case NCSI_PKT_CMD_SVF:
		return 8;
	case NCSI_PKT_CMD_EV:
		return 4;
	case NCSI_PKT_CMD_DV:
		return 0;
	case NCSI_PKT_CMD_SMA:
		return 8;
	case NCSI_PKT_CMD_EBF:
		return 4;
	case NCSI_PKT_CMD_DBF:
		return 0;
	case NCSI_PKT_CMD_EGMF:
		return 4;
	case NCSI_PKT_CMD_DGMF:
		return 0;
	case NCSI_PKT_CMD_SNFC:
		return 4;
	case NCSI_PKT_CMD_GVI:
		return 0;
	case NCSI_PKT_CMD_GC:
		return 0;
	case NCSI_PKT_CMD_GP:
		return 0;
	case NCSI_PKT_CMD_GCPS:
		return 0;
	case NCSI_PKT_CMD_GNS:
		return 0;
	case NCSI_PKT_CMD_GNPTS:
		return 0;
	case NCSI_PKT_CMD_GPS:
		return 0;
	default:
		printf("NCSI: Unknown command 0x%02x\n", cmd);
		return 0;
	}
}

static u32 ncsi_calculate_checksum(unsigned char *data, int len)
{
	u32 checksum = 0;
	int i;

	for (i = 0; i < len; i += 2)
		checksum += (((u32)data[i] << 8) | data[i + 1]);

	checksum = (~checksum + 1);
	return checksum;
}

static int ncsi_validate_rsp(struct ncsi_rsp_pkt *pkt, int payload)
{
	struct ncsi_rsp_pkt_hdr *hdr = &pkt->rsp;
	u32 checksum, c_offset;
	__be32 pchecksum;

	if (hdr->common.revision != 1) {
		printf("NCSI: 0x%02x response has unsupported revision 0x%x\n",
		       hdr->common.type, hdr->common.revision);
		return -1;
	}

	if (hdr->code != 0) {
		printf("NCSI: 0x%02x response returns error %d\n",
		       hdr->common.type, __be16_to_cpu(hdr->code));
		if (ntohs(hdr->reason) == 0x05)
			printf("(Invalid command length)\n");
		return -1;
	}

	if (ntohs(hdr->common.length) != payload) {
		printf("NCSI: 0x%02x response has incorrect length %d\n",
		       hdr->common.type, hdr->common.length);
		return -1;
	}

	c_offset = sizeof(struct ncsi_rsp_pkt_hdr) + payload - sizeof(checksum);
	pchecksum = get_unaligned_be32((void *)hdr + c_offset);
	if (pchecksum != 0) {
		checksum = ncsi_calculate_checksum((unsigned char *)hdr,
						   c_offset);
		if (pchecksum != checksum) {
			printf("NCSI: 0x%02x response has invalid checksum\n",
			       hdr->common.type);
			return -1;
		}
	}

	return 0;
}

static void ncsi_rsp_ec(struct ncsi_rsp_pkt *pkt)
{
	struct ncsi_rsp_pkt_hdr *rsp = (struct ncsi_rsp_pkt_hdr *)&pkt->rsp;
	unsigned int np, nc;

	np = NCSI_PACKAGE_INDEX(rsp->common.channel);
	nc = NCSI_CHANNEL_INDEX(rsp->common.channel);

	if (ncsi_priv->packages[np].channels[nc].cap_aen != 0)
		ncsi_send_ae(np, nc);
	/* else, done */
}

static void ncsi_rsp_ecnt(struct ncsi_rsp_pkt *pkt)
{
	struct ncsi_rsp_pkt_hdr *rsp = (struct ncsi_rsp_pkt_hdr *)&pkt->rsp;
	unsigned int np, nc;

	np = NCSI_PACKAGE_INDEX(rsp->common.channel);
	nc = NCSI_CHANNEL_INDEX(rsp->common.channel);

	ncsi_send_command(np, nc, NCSI_PKT_CMD_EC, NULL, 0, true);
}

static void ncsi_rsp_ebf(struct ncsi_rsp_pkt *pkt)
{
	struct ncsi_rsp_pkt_hdr *rsp = (struct ncsi_rsp_pkt_hdr *)&pkt->rsp;
	unsigned int np, nc;

	np = NCSI_PACKAGE_INDEX(rsp->common.channel);
	nc = NCSI_CHANNEL_INDEX(rsp->common.channel);

	ncsi_send_command(np, nc, NCSI_PKT_CMD_ECNT, NULL, 0, true);
}

static void ncsi_rsp_sma(struct ncsi_rsp_pkt *pkt)
{
	struct ncsi_rsp_pkt_hdr *rsp = (struct ncsi_rsp_pkt_hdr *)&pkt->rsp;
	unsigned int np, nc;

	np = NCSI_PACKAGE_INDEX(rsp->common.channel);
	nc = NCSI_CHANNEL_INDEX(rsp->common.channel);

	ncsi_send_ebf(np, nc);
}

static void ncsi_rsp_gc(struct ncsi_rsp_pkt *pkt)
{
	struct ncsi_rsp_gc_pkt *gc = (struct ncsi_rsp_gc_pkt *)pkt;
	struct ncsi_rsp_pkt_hdr *rsp = (struct ncsi_rsp_pkt_hdr *)&gc->rsp;
	struct ncsi_channel *c;
	unsigned int np, nc;

	np = NCSI_PACKAGE_INDEX(rsp->common.channel);
	nc = NCSI_CHANNEL_INDEX(rsp->common.channel);

	if (np >= ncsi_priv->n_packages ||
	    nc >= ncsi_priv->packages[np].n_channels) {
		printf("NCSI: Invalid package / channel (0x%02x, 0x%02x)\n",
		       np, nc);
		return;
	}

	c = &ncsi_priv->packages[np].channels[nc];
	c->cap_generic = ntohl(gc->cap) & NCSI_CAP_GENERIC_MASK;
	c->cap_bc = ntohl(gc->bc_cap) & NCSI_CAP_BC_MASK;
	c->cap_mc = ntohl(gc->mc_cap) & NCSI_CAP_MC_MASK;
	c->cap_aen = ntohl(gc->aen_cap) & NCSI_CAP_AEN_MASK;
	c->cap_vlan = ntohl(gc->vlan_mode) & NCSI_CAP_VLAN_MASK;

	/* End of probe for this channel */
}

static void ncsi_rsp_gvi(struct ncsi_rsp_pkt *pkt)
{
	struct ncsi_rsp_gvi_pkt *gvi = (struct ncsi_rsp_gvi_pkt *)pkt;
	struct ncsi_rsp_pkt_hdr *rsp = (struct ncsi_rsp_pkt_hdr *)&gvi->rsp;
	struct ncsi_channel *c;
	unsigned int np, nc, i;

	np = NCSI_PACKAGE_INDEX(rsp->common.channel);
	nc = NCSI_CHANNEL_INDEX(rsp->common.channel);

	if (np >= ncsi_priv->n_packages ||
	    nc >= ncsi_priv->packages[np].n_channels) {
		printf("NCSI: Invalid package / channel (0x%02x, 0x%02x)\n",
		       np, nc);
		return;
	}

	c = &ncsi_priv->packages[np].channels[nc];
	c->version.version = get_unaligned_be32(&gvi->ncsi_version);
	c->version.alpha2 = gvi->alpha2;
	memcpy(c->version.fw_name, gvi->fw_name, sizeof(c->version.fw_name));
	c->version.fw_version = get_unaligned_be32(&gvi->fw_version);
	for (i = 0; i < ARRAY_SIZE(c->version.pci_ids); i++)
		c->version.pci_ids[i] = get_unaligned_be16(gvi->pci_ids + i);
	c->version.mf_id = get_unaligned_be32(&gvi->mf_id);

	if (ncsi_priv->state == NCSI_PROBE_CHANNEL)
		ncsi_send_command(np, nc, NCSI_PKT_CMD_GC, NULL, 0, true);
}

static void ncsi_rsp_gls(struct ncsi_rsp_pkt *pkt)
{
	struct ncsi_rsp_gls_pkt *gls = (struct ncsi_rsp_gls_pkt *)pkt;
	struct ncsi_rsp_pkt_hdr *rsp = (struct ncsi_rsp_pkt_hdr *)&gls->rsp;
	unsigned int np, nc;

	np = NCSI_PACKAGE_INDEX(rsp->common.channel);
	nc = NCSI_CHANNEL_INDEX(rsp->common.channel);

	if (np >= ncsi_priv->n_packages ||
	    nc >= ncsi_priv->packages[np].n_channels) {
		printf("NCSI: Invalid package / channel (0x%02x, 0x%02x)\n",
		       np, nc);
		return;
	}

	ncsi_priv->packages[np].channels[nc].has_link =
					!!(get_unaligned_be32(&gls->status));

	if (ncsi_priv->state == NCSI_PROBE_CHANNEL)
		ncsi_send_command(np, nc, NCSI_PKT_CMD_GVI, NULL, 0, true);
}

static void ncsi_rsp_cis(struct ncsi_rsp_pkt *pkt)
{
	struct ncsi_rsp_pkt_hdr *rsp = (struct ncsi_rsp_pkt_hdr *)pkt;
	struct ncsi_package *package;
	unsigned int np, nc;

	np = NCSI_PACKAGE_INDEX(rsp->common.channel);
	nc = NCSI_CHANNEL_INDEX(rsp->common.channel);

	if (np >= ncsi_priv->n_packages) {
		printf("NCSI: Mystery package 0x%02x from CIS\n", np);
		return;
	}

	package = &ncsi_priv->packages[np];

	if (nc < package->n_channels) {
		/*
		 * This is fine in general but in the current design we
		 * don't send CIS commands to known channels.
		 */
		debug("NCSI: Duplicate channel 0x%02x\n", nc);
		return;
	}

	package->channels = realloc(package->channels,
				    sizeof(struct ncsi_channel) *
				    (package->n_channels + 1));
	if (!package->channels) {
		printf("NCSI: Could not allocate memory for new channel\n");
		return;
	}

	debug("NCSI: New channel 0x%02x\n", nc);

	package->channels[nc].id = nc;
	package->channels[nc].has_link = false;
	package->n_channels++;

	ncsi_send_gls(np, nc);
}

static void ncsi_rsp_dp(struct ncsi_rsp_pkt *pkt)
{
	struct ncsi_rsp_pkt_hdr *rsp = (struct ncsi_rsp_pkt_hdr *)pkt;
	unsigned int np;

	/* No action needed */

	np = NCSI_PACKAGE_INDEX(rsp->common.channel);
	if (np >= ncsi_priv->n_packages)
		debug("NCSI: DP response from unknown package %d\n", np);
}

static void ncsi_rsp_sp(struct ncsi_rsp_pkt *pkt)
{
	struct ncsi_rsp_pkt_hdr *rsp = (struct ncsi_rsp_pkt_hdr *)pkt;
	unsigned int np;

	np = NCSI_PACKAGE_INDEX(rsp->common.channel);

	if (np < ncsi_priv->n_packages) {
		/* Already know about this package */
		debug("NCSI: package 0x%02x selected\n", np);
		return;
	}

	debug("NCSI: adding new package %d\n", np);

	ncsi_priv->packages = realloc(ncsi_priv->packages,
				      sizeof(struct ncsi_package) *
				      (ncsi_priv->n_packages + 1));
	if (!ncsi_priv->packages) {
		printf("NCSI: could not allocate memory for new package\n");
		return;
	}

	ncsi_priv->packages[np].id = np;
	ncsi_priv->packages[np].n_channels = 0;
	ncsi_priv->packages[np].channels = NULL;
	ncsi_priv->n_packages++;
}

static void ncsi_update_state(struct ncsi_rsp_pkt_hdr *nh)
{
	bool timeout = !nh;
	int np, nc;

	switch (ncsi_priv->state) {
	case NCSI_PROBE_PACKAGE_SP:
		if (!timeout &&
		    ncsi_priv->current_package + 1 < NCSI_PACKAGE_MAX) {
			ncsi_priv->current_package++;
		} else {
			ncsi_priv->state = NCSI_PROBE_PACKAGE_DP;
			ncsi_priv->current_package = 0;
		}
		return ncsi_probe_packages();
	case NCSI_PROBE_PACKAGE_DP:
		if (ncsi_priv->current_package + 1 < ncsi_priv->n_packages &&
		    !timeout) {
			ncsi_priv->current_package++;
		} else {
			if (!ncsi_priv->n_packages) {
				printf("NCSI: no packages found\n");
				net_set_state(NETLOOP_FAIL);
				return;
			}
			printf("NCSI: probing channels\n");
			ncsi_priv->state = NCSI_PROBE_CHANNEL_SP;
			ncsi_priv->current_package = 0;
			ncsi_priv->current_channel = 0;
		}
		return ncsi_probe_packages();
	case NCSI_PROBE_CHANNEL_SP:
		if (!timeout && nh->common.type == NCSI_PKT_RSP_SP) {
			ncsi_priv->state = NCSI_PROBE_CHANNEL;
			return ncsi_probe_packages();
		}
		printf("NCSI: failed to select package 0x%0x2 or timeout\n",
		       ncsi_priv->current_package);
		net_set_state(NETLOOP_FAIL);
		break;
	case NCSI_PROBE_CHANNEL:
		// TODO only does package 0 for now
		if (ncsi_priv->pending_requests == 0) {
			np = ncsi_priv->current_package;
			nc = ncsi_priv->current_channel;

			/* Configure first channel that has link */
			if (ncsi_priv->packages[np].channels[nc].has_link) {
				ncsi_priv->state = NCSI_CONFIG;
			} else if (ncsi_priv->current_channel + 1 <
				   NCSI_CHANNEL_MAX) {
				ncsi_priv->current_channel++;
			} else {
				// XXX As above only package 0
				printf("NCSI: no channel found with link\n");
				net_set_state(NETLOOP_FAIL);
				return;
			}
			return ncsi_probe_packages();
		}
		break;
	case NCSI_CONFIG:
		if (ncsi_priv->pending_requests == 0) {
			printf("NCSI: configuration done!\n");
			net_set_state(NETLOOP_SUCCESS);
		} else if (timeout) {
			printf("NCSI: timeout during configure\n");
			net_set_state(NETLOOP_FAIL);
		}
		break;
	default:
		printf("NCSI: something went very wrong, nevermind\n");
		net_set_state(NETLOOP_FAIL);
		break;
	}
}

static void ncsi_timeout_handler(void)
{
	if (ncsi_priv->pending_requests)
		ncsi_priv->pending_requests--;

	ncsi_update_state(NULL);
}

static int ncsi_send_command(unsigned int np, unsigned int nc, unsigned int cmd,
			     uchar *payload, int len, bool wait)
{
	struct ncsi_pkt_hdr *hdr;
	__be32 *pchecksum;
	int eth_hdr_size;
	u32 checksum;
	uchar *pkt, *start;
	int final_len;

	pkt = calloc(1, PKTSIZE_ALIGN + PKTALIGN);
	if (!pkt)
		return -ENOMEM;
	start = pkt;

	eth_hdr_size = net_set_ether(pkt, net_bcast_ethaddr, PROT_NCSI);
	pkt += eth_hdr_size;

	/* Set NCSI command header fields */
	hdr = (struct ncsi_pkt_hdr *)pkt;
	hdr->mc_id = 0;
	hdr->revision = NCSI_PKT_REVISION;
	hdr->id = ++ncsi_priv->last_request;
	ncsi_priv->requests[ncsi_priv->last_request] = 1;
	hdr->type = cmd;
	hdr->channel = NCSI_TO_CHANNEL(np, nc);
	hdr->length = htons(len);

	if (payload && len)
		memcpy(pkt + sizeof(struct ncsi_pkt_hdr), payload, len);

	/* Calculate checksum */
	checksum = ncsi_calculate_checksum((unsigned char *)hdr,
					   sizeof(*hdr) + len);
	pchecksum = (__be32 *)((void *)(hdr + 1) + len);
	put_unaligned_be32(htonl(checksum), pchecksum);

	if (wait) {
		net_set_timeout_handler(1000UL, ncsi_timeout_handler);
		ncsi_priv->pending_requests++;
	}

	if (len < 26)
		len = 26;
	/* frame header, packet header, payload, checksum */
	final_len = eth_hdr_size + sizeof(struct ncsi_cmd_pkt_hdr) + len + 4;

	net_send_packet(start, final_len);
	free(start);
	return 0;
}

static void ncsi_handle_aen(struct ip_udp_hdr *ip, unsigned int len)
{
	struct ncsi_aen_pkt_hdr *hdr = (struct ncsi_aen_pkt_hdr *)ip;
	int payload, i;
	__be32 pchecksum;
	u32 checksum;

	switch (hdr->type) {
	case NCSI_PKT_AEN_LSC:
		printf("NCSI: link state changed\n");
		payload = 12;
		break;
	case NCSI_PKT_AEN_CR:
		printf("NCSI: re-configuration required\n");
		payload = 4;
		break;
	case NCSI_PKT_AEN_HNCDSC:
		/* Host notifcation - N/A but weird */
		debug("NCSI: HNCDSC AEN received\n");
		return;
	default:
		printf("%s: Invalid type 0x%02x\n", __func__, hdr->type);
		return;
	}

	/* Validate packet */
	if (hdr->common.revision != 1) {
		printf("NCSI: 0x%02x response has unsupported revision 0x%x\n",
		       hdr->common.type, hdr->common.revision);
		return;
	}

	if (ntohs(hdr->common.length) != payload) {
		printf("NCSI: 0x%02x response has incorrect length %d\n",
		       hdr->common.type, hdr->common.length);
		return;
	}

	pchecksum = get_unaligned_be32((void *)(hdr + 1) + payload - 4);
	if (pchecksum != 0) {
		checksum = ncsi_calculate_checksum((unsigned char *)hdr,
						   sizeof(*hdr) + payload - 4);
		if (pchecksum != checksum) {
			printf("NCSI: 0x%02x response has invalid checksum\n",
			       hdr->common.type);
			return;
		}
	}

	/* Link or configuration lost - just redo the discovery process */
	ncsi_priv->state = NCSI_PROBE_PACKAGE_SP;
	for (i = 0; i < ncsi_priv->n_packages; i++)
		free(ncsi_priv->packages[i].channels);
	free(ncsi_priv->packages);
	ncsi_priv->n_packages = 0;

	ncsi_priv->current_package = NCSI_PACKAGE_MAX;
	ncsi_priv->current_channel = NCSI_CHANNEL_MAX;

	ncsi_probe_packages();
}

void ncsi_receive(struct ethernet_hdr *et, struct ip_udp_hdr *ip,
		  unsigned int len)
{
	struct ncsi_rsp_pkt *pkt = (struct ncsi_rsp_pkt *)ip;
	struct ncsi_rsp_pkt_hdr *nh = (struct ncsi_rsp_pkt_hdr *)&pkt->rsp;
	void (*handler)(struct ncsi_rsp_pkt *pkt) = NULL;
	unsigned short payload;

	if (ncsi_priv->pending_requests)
		ncsi_priv->pending_requests--;

	if (len < sizeof(struct ncsi_rsp_pkt_hdr)) {
		printf("NCSI: undersized packet: %u bytes\n", len);
		goto out;
	}

	if (nh->common.type == NCSI_PKT_AEN)
		return ncsi_handle_aen(ip, len);

	switch (nh->common.type) {
	case NCSI_PKT_RSP_SP:
		payload = 4;
		handler = ncsi_rsp_sp;
		break;
	case NCSI_PKT_RSP_DP:
		payload = 4;
		handler = ncsi_rsp_dp;
		break;
	case NCSI_PKT_RSP_CIS:
		payload = 4;
		handler = ncsi_rsp_cis;
		break;
	case NCSI_PKT_RSP_GLS:
		payload = 16;
		handler = ncsi_rsp_gls;
		break;
	case NCSI_PKT_RSP_GVI:
		payload = 40;
		handler = ncsi_rsp_gvi;
		break;
	case NCSI_PKT_RSP_GC:
		payload = 32;
		handler = ncsi_rsp_gc;
		break;
	case NCSI_PKT_RSP_SMA:
		payload = 4;
		handler = ncsi_rsp_sma;
		break;
	case NCSI_PKT_RSP_EBF:
		payload = 4;
		handler = ncsi_rsp_ebf;
		break;
	case NCSI_PKT_RSP_ECNT:
		payload = 4;
		handler = ncsi_rsp_ecnt;
		break;
	case NCSI_PKT_RSP_EC:
		payload = 4;
		handler = ncsi_rsp_ec;
		break;
	case NCSI_PKT_RSP_AE:
		payload = 4;
		handler = NULL;
		break;
	default:
		printf("NCSI: unsupported packet type 0x%02x\n",
		       nh->common.type);
		goto out;
	}

	if (ncsi_validate_rsp(pkt, payload) != 0) {
		printf("NCSI: discarding invalid packet of type 0x%02x\n",
		       nh->common.type);
		goto out;
	}

	if (handler)
		handler(pkt);
out:
	ncsi_update_state(nh);
}

static void ncsi_send_sp(unsigned int np)
{
	uchar payload[4] = {0};

	ncsi_send_command(np, NCSI_RESERVED_CHANNEL, NCSI_PKT_CMD_SP,
			  (unsigned char *)&payload,
			  cmd_payload(NCSI_PKT_CMD_SP), true);
}

static void ncsi_send_dp(unsigned int np)
{
	ncsi_send_command(np, NCSI_RESERVED_CHANNEL, NCSI_PKT_CMD_DP, NULL, 0,
			  true);
}

static void ncsi_send_gls(unsigned int np, unsigned int nc)
{
	ncsi_send_command(np, nc, NCSI_PKT_CMD_GLS, NULL, 0, true);
}

static void ncsi_send_cis(unsigned int np, unsigned int nc)
{
	ncsi_send_command(np, nc, NCSI_PKT_CMD_CIS, NULL, 0, true);
}

static void ncsi_send_ae(unsigned int np, unsigned int nc)
{
	struct ncsi_cmd_ae_pkt cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.mode = htonl(ncsi_priv->packages[np].channels[nc].cap_aen);

	ncsi_send_command(np, nc, NCSI_PKT_CMD_AE,
			  ((unsigned char *)&cmd)
			  + sizeof(struct ncsi_cmd_pkt_hdr),
			  cmd_payload(NCSI_PKT_CMD_AE), true);
}

static void ncsi_send_ebf(unsigned int np, unsigned int nc)
{
	struct ncsi_cmd_ebf_pkt cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.mode = htonl(ncsi_priv->packages[np].channels[nc].cap_bc);

	ncsi_send_command(np, nc, NCSI_PKT_CMD_EBF,
			  ((unsigned char *)&cmd)
			  + sizeof(struct ncsi_cmd_pkt_hdr),
			  cmd_payload(NCSI_PKT_CMD_EBF), true);
}

static void ncsi_send_sma(unsigned int np, unsigned int nc)
{
	struct ncsi_cmd_sma_pkt cmd;
	unsigned char *addr, i;

	addr = eth_get_ethaddr();
	if (!addr) {
		printf("NCSI: no MAC address configured\n");
		return;
	}

	memset(&cmd, 0, sizeof(cmd));
	for (i = 0; i < ARP_HLEN; i++)
		cmd.mac[i] = addr[i];
	cmd.index = 1;
	cmd.at_e = 1;

	ncsi_send_command(np, nc, NCSI_PKT_CMD_SMA,
			  ((unsigned char *)&cmd)
			  + sizeof(struct ncsi_cmd_pkt_hdr),
			  cmd_payload(NCSI_PKT_CMD_SMA), true);
}

void ncsi_probe_packages(void)
{
	struct ncsi_package *package;
	unsigned int np, nc;

	switch (ncsi_priv->state) {
	case NCSI_PROBE_PACKAGE_SP:
		if (ncsi_priv->current_package == NCSI_PACKAGE_MAX)
			ncsi_priv->current_package = 0;
		ncsi_send_sp(ncsi_priv->current_package);
		break;
	case NCSI_PROBE_PACKAGE_DP:
		ncsi_send_dp(ncsi_priv->current_package);
		break;
	case NCSI_PROBE_CHANNEL_SP:
		if (ncsi_priv->n_packages > 0)
			ncsi_send_sp(ncsi_priv->current_package);
		else
			printf("NCSI: no packages discovered, configuration not possible\n");
		break;
	case NCSI_PROBE_CHANNEL:
		/* Kicks off chain of channel discovery */
		ncsi_send_cis(ncsi_priv->current_package,
			      ncsi_priv->current_channel);
		break;
	case NCSI_CONFIG:
		for (np = 0; np < ncsi_priv->n_packages; np++) {
			package = &ncsi_priv->packages[np];
			for (nc = 0; nc < package->n_channels; nc++)
				if (package->channels[nc].has_link)
					break;
			if (nc < package->n_channels)
				break;
		}
		if (np == ncsi_priv->n_packages) {
			printf("NCSI: no link available\n");
			return;
		}

		printf("NCSI: configuring channel %d\n", nc);
		ncsi_priv->current_package = np;
		ncsi_priv->current_channel = nc;
		/* Kicks off rest of configure chain */
		ncsi_send_sma(np, nc);
		break;
	default:
		printf("NCSI: unknown state 0x%x\n", ncsi_priv->state);
	}
}

int ncsi_probe(struct phy_device *phydev)
{
	if (!phydev->priv) {
		phydev->priv = malloc(sizeof(struct ncsi));
		if (!phydev->priv)
			return -ENOMEM;
		memset(phydev->priv, 0, sizeof(struct ncsi));
	}

	ncsi_priv = phydev->priv;

	return 0;
}

int ncsi_startup(struct phy_device *phydev)
{
	/* Set phydev parameters */
	phydev->speed = SPEED_100;
	phydev->duplex = DUPLEX_FULL;
	/* Normal phy reset is N/A */
	phydev->flags |= PHY_FLAG_BROKEN_RESET;

	/* Set initial probe state */
	ncsi_priv->state = NCSI_PROBE_PACKAGE_SP;

	/* No active package/channel yet */
	ncsi_priv->current_package = NCSI_PACKAGE_MAX;
	ncsi_priv->current_channel = NCSI_CHANNEL_MAX;

	/* Pretend link works so the MAC driver sets final bits up */
	phydev->link = true;

	/* Set ncsi_priv so we can use it when called from net_loop() */
	ncsi_priv = phydev->priv;

	return 0;
}

int ncsi_shutdown(struct phy_device *phydev)
{
	printf("NCSI: Disabling package %d\n", ncsi_priv->current_package);
	ncsi_send_dp(ncsi_priv->current_package);
	return 0;
}

U_BOOT_PHY_DRIVER(ncsi) = {
	.uid		= PHY_NCSI_ID,
	.mask		= 0xffffffff,
	.name		= "NC-SI",
	.features	= PHY_100BT_FEATURES | PHY_DEFAULT_FEATURES |
				SUPPORTED_100baseT_Full | SUPPORTED_MII,
	.probe		= ncsi_probe,
	.startup	= ncsi_startup,
	.shutdown	= ncsi_shutdown,
};
