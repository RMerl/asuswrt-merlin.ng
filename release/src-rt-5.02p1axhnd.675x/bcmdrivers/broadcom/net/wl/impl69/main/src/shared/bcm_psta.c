/*
 * BCM PSTA protocol processing
 * shared code for PSTA protocol processing in dhd & wl
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: bcm_psta.c 764398 2018-05-25 05:24:44Z $
 */
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcm_psta.h>

#include <ethernet.h>
#include <802.3.h>
#include <bcmudp.h>
#include <bcmdhcp.h>
#include <bcmicmp.h>
#include <bcmarp.h>
#include <vlan.h>

static void csum_fixup_16(uint8 *chksum, uint8 *optr, int olen, uint8 *nptr, int nlen);
static int32 bcm_psta_dhcp_option_find(uint8 *dhcp, uint16 dhcplen, uint16 option_code);
static int32 bcm_psta_dhcp6_option_find(uint8 *dhcp, uint16 dhcplen, uint8 msg_type,
	uint16 option_code);
static int32 bcm_psta_dhcpc_proc(uint8 *udph, uint8 *dhcp, uint16 dhcplen, uint8 *cli_mac,
	uint8 *mod_mac, bool tx, bool is_bcmc);
static int32 bcm_psta_dhcps_proc(uint8 *udph, uint8 *dhcp, uint16 dhcplen, uint8 *cli_mac,
	uint8 *mod_mac, bool tx, bool is_bcmc);
static int32 bcm_psta_dhcpc6_proc(uint8 *udph, uint8 *dhcp, uint16 dhcplen, uint8 *cli_mac,
	uint8 *mod_mac, bool tx, bool is_bcmc);
static int32 bcm_psta_dhcps6_proc(uint8 *udph, uint8 *dhcp, uint16 dhcplen, uint8 *cli_mac,
	uint8 *mod_mac, bool tx, bool is_bcmc);

/*
 * Adjust 16 bit checksum - taken from RFC 3022.
 *
 * The algorithm below is applicable only for even offsets (i.e., optr
 * below must be at an even offset from start of header) and even lengths
 * (i.e., olen and nlen below must be even).
 */
static void
csum_fixup_16(uint8 *chksum, uint8 *optr, int olen, uint8 *nptr, int nlen)
{
	long x, old, new;

	ASSERT(!((uintptr)optr&1) && !(olen&1));
	ASSERT(!((uintptr)nptr&1) && !(nlen&1));

	x = (chksum[0]<< 8)+chksum[1];
	if (!x)
		return;
	x = ~x & 0xffff;
	while (olen)
	{
		old = (optr[0]<< 8)+optr[1]; optr += 2;
		x -= old & 0xffff;
		if (x <= 0) { x--; x &= 0xffff; }
		olen -= 2;
	}
	while (nlen)
	{
		new = (nptr[0]<< 8)+nptr[1]; nptr += 2;
		x += new & 0xffff;
		if (x & 0x10000) { x++; x &= 0xffff; }
		nlen -= 2;
	}
	x = ~x & 0xffff;
	chksum[0] = (uint8)(x >> 8); chksum[1] = (uint8)x;
}

/* Search the specified dhcp option and return the offset of it */
static int32
bcm_psta_dhcp_option_find(uint8 *dhcp, uint16 dhcplen, uint16 option_code)
{
	bool found = FALSE;
	uint16 optlen, offset;

	if (dhcplen <= DHCP_OPT_OFFSET) {
		return BCME_ERROR;
	}

	offset = DHCP_OPT_OFFSET;

	/* First option must be magic cookie */
	if ((dhcp[offset + 0] != 0x63) || (dhcp[offset + 1] != 0x82) ||
	    (dhcp[offset + 2] != 0x53) || (dhcp[offset + 3] != 0x63)) {
		return BCME_ERROR;
	}

	offset += 4;

	while (offset < dhcplen) {
		/* End of options */
		if (*(uint8 *)(dhcp + offset) == 255) {
			break;
		}

		if (*(uint8 *)(dhcp + offset + DHCP_OPT_CODE_OFFSET) == option_code) {
			found = TRUE;
			break;
		}

		optlen = *(uint8 *)(dhcp + offset + DHCP_OPT_LEN_OFFSET) + 2;
		offset += optlen;
	}

	return found ? offset : BCME_ERROR;
}

/* Search the specified dhcp6 option and return the offset of it */
static int32
bcm_psta_dhcp6_option_find(uint8 *dhcp, uint16 dhcplen, uint8 msg_type, uint16 option_code)
{
	bool found = FALSE;
	uint16 len, optlen, offset;

	/* Get the pointer to options */
	if (msg_type == DHCP6_TYPE_RELAYFWD)
		offset = DHCP6_RELAY_OPT_OFFSET;
	else
		offset = DHCP6_MSG_OPT_OFFSET;

	len = offset;
	while (len < dhcplen) {
		if (*(uint16 *)(dhcp + offset) == HTON16(option_code)) {
			found = TRUE;
			break;
		}
		optlen = NTOH16(*(uint16 *)(dhcp + offset + DHCP6_OPT_LEN_OFFSET)) + 4;
		offset += optlen;
		len += optlen;
	}

	return found ? offset : BCME_ERROR;
}

/* Process DHCP server frame (server to client) */
static int32
bcm_psta_dhcps_proc(uint8 *udph, uint8 *dhcp, uint16 dhcplen, uint8 *cli_mac,
	uint8 *mod_mac, bool tx, bool is_bcmc)
{
	uint8 chaddr[ETHER_ADDR_LEN];
	int32 opt_offset;

	/* Only interested in replies when receiving from server in rx dir */
	if (tx)
		return BCME_OK;

	if (*(dhcp + DHCP_TYPE_OFFSET) != DHCP_TYPE_REPLY)
		return BCME_OK;

	bcopy(dhcp + DHCP_CHADDR_OFFSET, chaddr, ETHER_ADDR_LEN);

	if (!is_bcmc)
		ASSERT(!tx && BCM_PSTA_IS_ALIAS(mod_mac, chaddr));

	/* Check if we need to replace with original MAC addr */
	if (BCM_PSTA_IS_ALIAS(mod_mac, chaddr)) {
		BCM_PSTA_CLR_ALIAS(cli_mac, mod_mac, chaddr);

		csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
		              dhcp + DHCP_CHADDR_OFFSET, ETHER_ADDR_LEN,
		              chaddr, ETHER_ADDR_LEN);
		BCM_PSTA_CLR_ALIAS(cli_mac, mod_mac, dhcp + DHCP_CHADDR_OFFSET);
	}

	/* If client identifier option is not present then
	 * no processing required.
	 */
	opt_offset = bcm_psta_dhcp_option_find(dhcp, dhcplen, DHCP_OPT_CODE_CLIENTID);

	if (opt_offset == BCME_ERROR)
		return BCME_OK;

	/* First octet indicates type of data. If type is 1, it indicates
	 * that next 6 octets contain mac address.
	 */
	if (*(dhcp + opt_offset + DHCP_OPT_DATA_OFFSET) == 0x01) {
		uint32 offset, mac_offset = opt_offset + DHCP_OPT_DATA_OFFSET + 1;
		uint8 cli_addr[ETHER_ADDR_LEN];

		bcopy(dhcp + mac_offset, chaddr, ETHER_ADDR_LEN);
		/* Return if it is already modified */
		if (!BCM_PSTA_IS_ALIAS(mod_mac, chaddr))
			return BCME_OK;

		BCM_PSTA_CLR_ALIAS(cli_mac, mod_mac, chaddr);

		/* For DHCP replies, replace chaddr with host's MAC */
		if (mac_offset & 1) {
			cli_addr[0] = dhcp[mac_offset - 1];
			bcopy(chaddr, cli_addr + 1, ETHER_ADDR_LEN - 1);
			offset = mac_offset - 1;
		} else {
			bcopy(chaddr, cli_addr, ETHER_ADDR_LEN);
			offset = mac_offset;
		}

		csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
		              dhcp + offset, ETHER_ADDR_LEN,
		              cli_addr, ETHER_ADDR_LEN);
		BCM_PSTA_CLR_ALIAS(cli_mac, mod_mac, dhcp + mac_offset);
	}

	return BCME_OK;
}

/* Process DHCP client frame (client to server) */
static int32
bcm_psta_dhcpc_proc(uint8 *udph, uint8 *dhcp, uint16 dhcplen, uint8 *cli_mac,
	uint8 *mod_mac, bool tx, bool is_bcmc)
{
	uint8 chaddr[ETHER_ADDR_LEN];
	int32 opt_offset;
	/* Only interested in requests when sending to server in tx dir */
	if (!tx) {
		return BCME_OK;
	} else {
		if (*(dhcp + DHCP_TYPE_OFFSET) != DHCP_TYPE_REQUEST)
			return BCME_OK;
	}

	bcopy(dhcp + DHCP_CHADDR_OFFSET, chaddr, ETHER_ADDR_LEN);

	ASSERT(tx && !BCM_PSTA_IS_ALIAS(mod_mac, chaddr));

	if (BCM_EA_CMP(cli_mac, chaddr)) {
		BCM_PSTA_SET_ALIAS(cli_mac, mod_mac, chaddr);
		/* For DHCP requests, replace chaddr with host's MAC */
		csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
			dhcp + DHCP_CHADDR_OFFSET, ETHER_ADDR_LEN,
			chaddr, ETHER_ADDR_LEN);
		BCM_PSTA_SET_ALIAS(cli_mac, mod_mac, dhcp + DHCP_CHADDR_OFFSET);
	}

	/* If client identifier option is not present then no processing required */
	opt_offset = bcm_psta_dhcp_option_find(dhcp, dhcplen, DHCP_OPT_CODE_CLIENTID);

	if (opt_offset == BCME_ERROR)
		return BCME_OK;

	/* First octet indicates type of data. If type is 1, it indicates
	 * that next 6 octets contain mac address.
	 */
	if (*(dhcp + opt_offset + DHCP_OPT_DATA_OFFSET) == 0x01) {
		uint32 offset, mac_offset = opt_offset + DHCP_OPT_DATA_OFFSET + 1;
		uint8 cli_addr[ETHER_ADDR_LEN];

		bcopy(dhcp + mac_offset, chaddr, ETHER_ADDR_LEN);
		/* Only modify if chaddr is one of the clients */
		if (!BCM_PSTA_IS_ALIAS(cli_mac, chaddr))
			return BCME_OK;

		BCM_PSTA_SET_ALIAS(cli_mac, mod_mac, chaddr);

		/* For DHCP requests, replace chaddr with host's MAC */
		if (mac_offset & 1) {
			cli_addr[0] = dhcp[mac_offset - 1];
			bcopy(chaddr, cli_addr + 1, ETHER_ADDR_LEN - 1);
			offset = mac_offset - 1;
		} else {
			bcopy(chaddr, cli_addr, ETHER_ADDR_LEN);
			offset = mac_offset;
		}

		csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
		              dhcp + offset, ETHER_ADDR_LEN,
		              cli_addr, ETHER_ADDR_LEN);
		BCM_PSTA_SET_ALIAS(cli_mac, mod_mac, dhcp + mac_offset);
	}

	return BCME_OK;
}

/* Process DHCP client frame (client to server) */
static int32
bcm_psta_dhcpc6_proc(uint8 *udph, uint8 *dhcp, uint16 dhcplen, uint8 *cli_mac,
	uint8 *mod_mac, bool tx, bool is_bcmc)
{
	int32 opt_offset, duid_offset;
	uint8 chaddr[ETHER_ADDR_LEN];
	uint8 msg_type;

	/* Only interested in requests when sending to server in tx dir */
	if (!tx)
		return BCME_OK;

	msg_type = *(dhcp + DHCP6_TYPE_OFFSET);

	ASSERT((msg_type == DHCP6_TYPE_SOLICIT) ||
	       (msg_type == DHCP6_TYPE_REQUEST) ||
	       (msg_type == DHCP6_TYPE_RENEW) ||
	       (msg_type == DHCP6_TYPE_REBIND) ||
	       (msg_type == DHCP6_TYPE_DECLINE) ||
	       (msg_type == DHCP6_TYPE_CONFIRM) ||
	       (msg_type == DHCP6_TYPE_INFOREQ) ||
	       (msg_type == DHCP6_TYPE_RELAYFWD));
	/* If client identifier option is not present then
	 * no processing required.
	 */
	opt_offset = bcm_psta_dhcp6_option_find(dhcp, dhcplen, msg_type,
		DHCP6_OPT_CODE_CLIENTID);

	if (opt_offset == BCME_ERROR)
		return BCME_OK;

	duid_offset = (opt_offset + DHCP6_OPT_DATA_OFFSET);
	/* Look for DUID-LLT or DUID-LL */
	if (*(uint16 *)(dhcp + duid_offset) == HTON16(1))
		duid_offset += 8;
	else if (*(uint16 *)(dhcp + duid_offset) == HTON16(3))
		duid_offset += 4;
	else
		return BCME_OK;

	bcopy(dhcp + duid_offset, chaddr, ETHER_ADDR_LEN);

	ASSERT(tx && !BCM_PSTA_IS_ALIAS(mod_mac, chaddr));
	/* Return if it is already modified */
	if (!BCM_EA_CMP(cli_mac, chaddr))
		return BCME_OK;

	BCM_PSTA_SET_ALIAS(cli_mac, mod_mac, chaddr);
	/* For DHCP requests, replace chaddr with host's MAC */
	csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
		dhcp + duid_offset, ETHER_ADDR_LEN,
		chaddr, ETHER_ADDR_LEN);
	BCM_PSTA_SET_ALIAS(cli_mac, mod_mac, dhcp + duid_offset);

	return BCME_OK;
}

/* Process DHCP server frame (server to client) */
static int32
bcm_psta_dhcps6_proc(uint8 *udph, uint8 *dhcp, uint16 dhcplen, uint8 *cli_mac,
	uint8 *mod_mac, bool tx, bool is_bcmc)
{
	int32 opt_offset, duid_offset;
	uint8 chaddr[ETHER_ADDR_LEN];
	uint8 msg_type;

	/* Only interested in replies received from server in rx dir */
	if (tx)
		return BCME_OK;

	msg_type = *(dhcp + DHCP6_TYPE_OFFSET);

	ASSERT((msg_type == DHCP6_TYPE_ADVERTISE) ||
	       (msg_type == DHCP6_TYPE_REPLY) ||
	       (msg_type == DHCP6_TYPE_RECONFIGURE) ||
	       (msg_type == DHCP6_TYPE_RELAYREPLY));

	/* If server identifier option is not present then
	 * no processing required.
	 */
	opt_offset = bcm_psta_dhcp6_option_find(dhcp, dhcplen, msg_type,
	                                        DHCP6_OPT_CODE_CLIENTID);
	if (opt_offset == BCME_ERROR)
		return BCME_OK;

	duid_offset = (opt_offset + DHCP6_OPT_DATA_OFFSET);

	/* Look for DUID-LLT or DUID-LL */
	if (*(uint16 *)(dhcp + duid_offset) == HTON16(1))
		duid_offset += 8;
	else if (*(uint16 *)(dhcp + duid_offset) == HTON16(3))
		duid_offset += 4;
	else
		return BCME_OK;

	bcopy(dhcp + duid_offset, chaddr, ETHER_ADDR_LEN);

	if (!is_bcmc)
		ASSERT(!tx && BCM_PSTA_IS_ALIAS(mod_mac, chaddr));
	/* Return if it is already modified */
	if (!BCM_EA_CMP(mod_mac, chaddr))
		return BCME_OK;

	BCM_PSTA_CLR_ALIAS(cli_mac, mod_mac, chaddr);

	csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
		dhcp + duid_offset, ETHER_ADDR_LEN,
		chaddr, ETHER_ADDR_LEN);

	BCM_PSTA_CLR_ALIAS(cli_mac, mod_mac, dhcp + duid_offset);

	return BCME_OK;
}

/*  Get the ether type from the ethernet header  */
int32
bcm_psta_ether_type(osl_t *osh, uint8 *eh, uint16 *et, uint16 *ip_off, bool *is_1x)
{
	uint16 ether_type, pull = ETHER_HDR_LEN;

	ether_type = NTOH16(*(uint16 *)(eh + ETHER_TYPE_OFFSET));

	/* LLC/SNAP frame */
	if ((ether_type <= ETHER_MAX_DATA) &&
	    (ether_type >= ETHER_HDR_LEN + SNAP_HDR_LEN + ETHER_TYPE_LEN)) {
		uint8 llc_snap_hdr[SNAP_HDR_LEN] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
		uint8 llc_stp_hdr[5] = {0x42, 0x42, 0x03, 0x00, 0x00};

		/* Unknown llc-snap header */
		if (bcmp(llc_snap_hdr, eh + ETHER_HDR_LEN, SNAP_HDR_LEN) &&
			bcmp(llc_stp_hdr, eh + ETHER_HDR_LEN, 5)) {
			return BCME_ERROR;
		}
		pull += (SNAP_HDR_LEN + ETHER_TYPE_LEN);
		ether_type = NTOH16(*(uint16 *)(eh + ETHER_HDR_LEN + SNAP_HDR_LEN));
	}

	/* Get the ether type from VLAN frame */
	if (ether_type == ETHER_TYPE_8021Q) {
		ether_type = NTOH16(*(uint16 *)(eh + ETHER_HDR_LEN));
		pull += VLAN_TAG_LEN;
	}

	if (ip_off != NULL)
		*ip_off = pull;

	/* 802.1x frames need not go thru' protocol processing here */
	if (is_1x != NULL) {
		*is_1x = ((ether_type == ETHER_TYPE_802_1X) ||
#ifdef BCMWAPI_WAI
		          (ether_type == ETHER_TYPE_WAI) ||
#endif /* BCMWAPI_WAI */
		          (ether_type == ETHER_TYPE_802_1X_PREAUTH));
	}

	if (et != NULL)
		*et = ether_type;

	return BCME_OK;
}

/* Process ARP request and replies in tx and rx directions */
int32
bcm_psta_arp_proc(uint8 *ah, uint8 *cli_mac, uint8 *mod_mac, bool tx)
{
	uint8 chaddr[ETHER_ADDR_LEN];

	/* Modify the source mac address in ARP header */
	if (tx) {
		/* ARP requests */
		bcopy(ah + ARP_SRC_ETH_OFFSET, chaddr, ETHER_ADDR_LEN);

		ASSERT(!BCM_PSTA_IS_ALIAS(mod_mac, chaddr));

		BCM_PSTA_SET_ALIAS(cli_mac, mod_mac, chaddr);

		/* Return if it is already modified */
		if (!BCM_EA_CMP(mod_mac, chaddr))
			return BCME_OK;

		BCM_PSTA_SET_ALIAS(cli_mac, mod_mac, ah + ARP_SRC_ETH_OFFSET);

	} else {
		/* Restore the host/client's mac address in ARP reply */
		if (*(uint16 *)(ah + ARP_OPC_OFFSET) == HTON16(ARP_OPC_REPLY)) {
			bcopy(ah + ARP_TGT_ETH_OFFSET, chaddr, ETHER_ADDR_LEN);

			ASSERT(BCM_PSTA_IS_ALIAS(mod_mac, chaddr));

			/* Return if it is already modified */
			if (!BCM_EA_CMP(mod_mac, chaddr))
				return BCME_OK;

			BCM_PSTA_CLR_ALIAS(cli_mac, mod_mac,
				ah + ARP_TGT_ETH_OFFSET);
		}
	}
	return BCME_OK;
}

/* Process UDP request/replies */
int32
bcm_psta_udp_proc(void *psta, void *cfg, void **p, uint8 *ih, uint8 *uh,
                  uint8 **shost, bool tx, bool is_bcmc, bcm_psta_dhcp_proc_cb psta_dhcp_proc_cb)
{
	uint16 port, dhcplen;
	bool is_dhcp;

	port = NTOH16(*(uint16 *)(uh + UDP_DEST_PORT_OFFSET));

	is_dhcp = (((IP_VER(ih) == IP_VER_4) &&
	            ((port == DHCP_PORT_SERVER) || (port == DHCP_PORT_CLIENT))) ||
	           ((IP_VER(ih) == IP_VER_6) &&
	            ((port == DHCP6_PORT_SERVER) || (port == DHCP6_PORT_CLIENT))));

	if (is_dhcp) {
		dhcplen = NTOH16(*(uint16 *)(uh + UDP_LEN_OFFSET)) - UDP_HDR_LEN;
		return psta_dhcp_proc_cb(psta, cfg, p, uh,
			uh + UDP_HDR_LEN, dhcplen,
			port, shost, tx, is_bcmc);
	}

	return BCME_OK;
}

/* DHCP header processing call for Server/Client */
int32
bcm_psta_dhcp_proc(uint16 port, uint8 *uh, uint8 *dhcph, uint16 dhcplen,
	uint8 *cli_mac,	uint8 *mod_mac, bool tx, bool is_bcmc)
{

/* DHCP tx/rx body processing */
	switch (port) {

	case DHCP_PORT_SERVER:
		return bcm_psta_dhcpc_proc(uh, dhcph, dhcplen, cli_mac, mod_mac, tx, is_bcmc);

	case DHCP6_PORT_SERVER:
		return bcm_psta_dhcpc6_proc(uh, dhcph, dhcplen, cli_mac, mod_mac, tx, is_bcmc);

	case DHCP_PORT_CLIENT:
		return bcm_psta_dhcps_proc(uh, dhcph, dhcplen, cli_mac, mod_mac, tx, is_bcmc);

	case DHCP6_PORT_CLIENT:
		return bcm_psta_dhcps6_proc(uh, dhcph, dhcplen, cli_mac, mod_mac, tx, is_bcmc);

	default:
		break;
	}

	return BCME_OK;
}

/* ICMPv6 header processing */
int32
bcm_psta_icmp6_proc(uint8 *ih, uint8 *icmph, uint16 icmplen, uint8 *cli_mac,
	uint8 *mod_mac, bool tx)
{
	struct icmp6_opt *opt;
	uint8 chaddr[ETHER_ADDR_LEN];
	uint8 *src_haddr = NULL;

	/*  ICMPv6 information message processing  */
	if (icmph[0] == ICMP6_RTR_SOLICITATION) {
		opt = (struct icmp6_opt*)&icmph[ICMP6_RTRSOL_OPT_OFFSET];
		if (opt->type == ICMP6_OPT_TYPE_SRC_LINK_LAYER)
			src_haddr = opt->data;
	} else if (icmph[0] == ICMP6_RTR_ADVERTISEMENT) {
		opt = (struct icmp6_opt *)&icmph[ICMP6_RTRADV_OPT_OFFSET];
		if (opt->type == ICMP6_OPT_TYPE_PREFIX_INFO)
			opt = (struct icmp6_opt *)&icmph[ICMP6_RTRADV_OPT_OFFSET +
			(opt->length * 8)];
		if (opt->type == ICMP6_OPT_TYPE_SRC_LINK_LAYER)
			src_haddr = opt->data;
	} else if (icmph[0] == ICMP6_NEIGH_SOLICITATION) {
		uint8 unspec[IPV6_ADDR_LEN] = { 0 };
		/* Option is not present when the src ip address is
		 * unspecified address.
		 */
		if (bcmp(unspec, ih + IPV6_SRC_IP_OFFSET, IPV6_ADDR_LEN) == 0)
			return BCME_OK;
		opt = (struct icmp6_opt *) &icmph[ICMP6_NEIGHSOL_OPT_OFFSET];
		if (opt->type == ICMP6_OPT_TYPE_SRC_LINK_LAYER)
			src_haddr = opt->data;
	} else if (icmph[0] == ICMP6_NEIGH_ADVERTISEMENT) {
		/* When responding to unicast neighbor soliciation this
		 * option may or may not be present.
		 */
		if ((icmplen - ICMP6_NEIGHADV_OPT_OFFSET) == 0)
			return BCME_OK;
		opt = (struct icmp6_opt *)&icmph[ICMP6_NEIGHADV_OPT_OFFSET];
		if (opt->type == ICMP6_OPT_TYPE_TGT_LINK_LAYER)
			src_haddr = opt->data;
	} else if (icmph[0] == ICMP6_REDIRECT) {
		opt = (struct icmp6_opt *)&icmph[ICMP6_REDIRECT_OPT_OFFSET];
		if (opt->type == ICMP6_OPT_TYPE_TGT_LINK_LAYER)
			src_haddr = opt->data;
	}

	if (tx) {
		if (src_haddr != NULL)
			bcopy(src_haddr, chaddr, ETHER_ADDR_LEN);
		else
			return BCME_OK;

		/* Modify the src link layer address only if it matches
		 * client's mac address.
		 */
		ASSERT(!BCM_PSTA_IS_ALIAS(mod_mac, chaddr));
		BCM_PSTA_SET_ALIAS(cli_mac, mod_mac, chaddr);
		/* Return if it is already modified */
		if (!BCM_PSTA_IS_ALIAS(mod_mac, chaddr))
			return BCME_OK;

		csum_fixup_16(icmph + ICMP_CHKSUM_OFFSET,
			src_haddr, ETHER_ADDR_LEN, chaddr, ETHER_ADDR_LEN);

		BCM_PSTA_SET_ALIAS(cli_mac, mod_mac, src_haddr);
	}

	return BCME_OK;
}
