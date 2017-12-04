/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2008 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* We also supports FDP which is very similar to CDPv1 */
#include "lldpd.h"
#include "frame.h"

#if defined (ENABLE_CDP) || defined (ENABLE_FDP)

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

static int
cdp_send(struct lldpd *global,
	 struct lldpd_hardware *hardware, int version)
{
	const char *platform = "Unknown";
	struct lldpd_chassis *chassis;
	struct lldpd_mgmt *mgmt;
	struct lldpd_port *port;
	u_int8_t mcastaddr[] = CDP_MULTICAST_ADDR;
	u_int8_t llcorg[] = LLC_ORG_CISCO;
#ifdef ENABLE_FDP
	char *capstr;
#endif
	u_int16_t checksum;
	int length, i;
	u_int32_t cap;
	u_int8_t *packet;
	u_int8_t *pos, *pos_len_eh, *pos_llc, *pos_cdp, *pos_checksum, *tlv, *end;

	log_debug("cdp", "send CDP frame on %s", hardware->h_ifname);

	port = &(hardware->h_lport);
	chassis = port->p_chassis;

#ifdef ENABLE_FDP
	if (version == 0) {
		/* With FDP, change multicast address and LLC PID */
		const u_int8_t fdpmcastaddr[] = FDP_MULTICAST_ADDR;
		const u_int8_t fdpllcorg[] = LLC_ORG_FOUNDRY;
		memcpy(mcastaddr, fdpmcastaddr, sizeof(mcastaddr));
		memcpy(llcorg, fdpllcorg, sizeof(llcorg));
	}
#endif

	length = hardware->h_mtu;
	if ((packet = (u_int8_t*)calloc(1, length)) == NULL)
		return ENOMEM;
	pos = packet;

	/* Ethernet header */
	if (!(
	      POKE_BYTES(mcastaddr, sizeof(mcastaddr)) &&
	      POKE_BYTES(&hardware->h_lladdr, ETHER_ADDR_LEN) &&
	      POKE_SAVE(pos_len_eh) && /* We compute the len later */
	      POKE_UINT16(0)))
		goto toobig;

	/* LLC */
	if (!(
	      POKE_SAVE(pos_llc) &&
	      POKE_UINT8(0xaa) && /* SSAP */
	      POKE_UINT8(0xaa) && /* DSAP */
	      POKE_UINT8(0x03) && /* Control field */
	      POKE_BYTES(llcorg, sizeof(llcorg)) &&
	      POKE_UINT16(LLC_PID_CDP)))
		goto toobig;

	/* CDP header */
	if (!(
	      POKE_SAVE(pos_cdp) &&
	      POKE_UINT8((version == 0)?1:version) &&
	      POKE_UINT8(global?global->g_config.c_ttl:180) &&
	      POKE_SAVE(pos_checksum) && /* Save checksum position */
	      POKE_UINT16(0)))
		goto toobig;

	/* Chassis ID */
	const char *chassis_name = chassis->c_name?chassis->c_name:"";
	if (!(
	      POKE_START_CDP_TLV(CDP_TLV_CHASSIS) &&
	      POKE_BYTES(chassis_name, strlen(chassis_name)) &&
	      POKE_END_CDP_TLV))
		goto toobig;

	/* Adresses */
	/* See:
	 *   http://www.cisco.com/univercd/cc/td/doc/product/lan/trsrb/frames.htm#xtocid12
	 *
	 * It seems that Cisco implies that CDP supports IPv6 using
	 * 802.2 address format with 0xAAAA03 0x000000 0x0800, but
	 * 0x0800 is the Ethernet protocol type for IPv4. Therefore,
	 * we support only IPv4. */
	i = 0;
	TAILQ_FOREACH(mgmt, &chassis->c_mgmt, m_entries)
		if (mgmt->m_family == LLDPD_AF_IPV4) i++;
	if (i > 0) {
		if (!(
		      POKE_START_CDP_TLV(CDP_TLV_ADDRESSES) &&
		      POKE_UINT32(i)))
			goto toobig;
		TAILQ_FOREACH(mgmt, &chassis->c_mgmt, m_entries) {
			switch (mgmt->m_family) {
			case LLDPD_AF_IPV4:
				if (!(
				      POKE_UINT8(1) &&	/* Type: NLPID */
				      POKE_UINT8(1) &&  /* Length: 1 */
				      POKE_UINT8(CDP_ADDRESS_PROTO_IP) && /* IP */
				      POKE_UINT16(sizeof(struct in_addr)) && /* Address length */
				      POKE_BYTES(&mgmt->m_addr, sizeof(struct in_addr))))
					goto toobig;
				break;
			}
		}
		if (!(POKE_END_CDP_TLV))
			goto toobig;
	}

	/* Port ID */
	const char *port_descr = hardware->h_lport.p_descr?hardware->h_lport.p_descr:"";
	if (!(
	      POKE_START_CDP_TLV(CDP_TLV_PORT) &&
	      POKE_BYTES(port_descr, strlen(port_descr)) &&
	      POKE_END_CDP_TLV))
		goto toobig;

	/* Capabilities */
	if (version != 0) {
		cap = 0;
		if (chassis->c_cap_enabled & LLDP_CAP_ROUTER)
			cap |= CDP_CAP_ROUTER;
		if (chassis->c_cap_enabled & LLDP_CAP_BRIDGE)
			cap |= CDP_CAP_SWITCH;
		cap |= CDP_CAP_HOST;
		if (!(
		      POKE_START_CDP_TLV(CDP_TLV_CAPABILITIES) &&
		      POKE_UINT32(cap) &&
		      POKE_END_CDP_TLV))
			goto toobig;
#ifdef ENABLE_FDP
	} else {
		/* With FDP, it seems that a string is used in place of an int */
		if (chassis->c_cap_enabled & LLDP_CAP_ROUTER)
			capstr = "Router";
		else if (chassis->c_cap_enabled & LLDP_CAP_BRIDGE)
			capstr = "Switch";
		else if (chassis->c_cap_enabled & LLDP_CAP_REPEATER)
			capstr = "Bridge";
		else
			capstr = "Host";
		if (!(
		      POKE_START_CDP_TLV(CDP_TLV_CAPABILITIES) &&
		      POKE_BYTES(capstr, strlen(capstr)) &&
		      POKE_END_CDP_TLV))
			goto toobig;
#endif
	}

	/* Native VLAN */
#ifdef ENABLE_DOT1
	if (version >=2 && hardware->h_lport.p_pvid != 0) {
		if (!(
		      POKE_START_CDP_TLV(CDP_TLV_NATIVEVLAN) &&
		      POKE_UINT16(hardware->h_lport.p_pvid) &&
		      POKE_END_CDP_TLV))
			goto toobig;
	}
#endif

	/* Software version */
	const char * chassis_descr = chassis->c_descr?chassis->c_descr:"";
	if (!(
	      POKE_START_CDP_TLV(CDP_TLV_SOFTWARE) &&
	      POKE_BYTES(chassis_descr, strlen(chassis_descr)) &&
	      POKE_END_CDP_TLV))
		goto toobig;

	/* Platform */
	if (global && global->g_config.c_platform) platform = global->g_config.c_platform;

	if (!(
	      POKE_START_CDP_TLV(CDP_TLV_PLATFORM) &&
	      POKE_BYTES(platform, strlen(platform)) &&
	      POKE_END_CDP_TLV))
		goto toobig;

#ifdef ENABLE_LLDPMED
	/* Power use */
	if ((version >= 2) &&
	    port->p_med_cap_enabled &&
	    (port->p_med_power.source != LLDP_MED_POW_SOURCE_LOCAL) &&
	    (port->p_med_power.val > 0) &&
	    (port->p_med_power.val <= 655)) {
		if (!(
		      POKE_START_CDP_TLV(CDP_TLV_POWER_CONSUMPTION) &&
		      POKE_UINT16(port->p_med_power.val * 100) &&
		      POKE_END_CDP_TLV))
			goto toobig;
	}
#endif
	(void)POKE_SAVE(end);

	/* Compute len and checksum */
	POKE_RESTORE(pos_len_eh);
	if (!(POKE_UINT16(end - pos_llc))) goto toobig;
	checksum = frame_checksum(pos_cdp, end - pos_cdp, (version != 0) ? 1 : 0);
	POKE_RESTORE(pos_checksum);
	if (!(POKE_UINT16(checksum))) goto toobig;

	if (interfaces_send_helper(global, hardware,
		(char *)packet, end - packet) == -1) {
		log_warn("cdp", "unable to send packet on real device for %s",
			   hardware->h_ifname);
		free(packet);
		return ENETDOWN;
	}

	hardware->h_tx_cnt++;

	free(packet);
	return 0;
 toobig:
	free(packet);
	return -1;
}

#define CHECK_TLV_SIZE(x, name)				   \
	do { if (tlv_len < (x)) {			   \
		log_warnx("cdp", name " CDP/FDP TLV too short received on %s", \
	       hardware->h_ifname);			   \
	   goto malformed;				   \
	} } while (0)
/* cdp_decode also decodes FDP */
int
cdp_decode(struct lldpd *cfg, char *frame, int s,
    struct lldpd_hardware *hardware,
    struct lldpd_chassis **newchassis, struct lldpd_port **newport)
{
	struct lldpd_chassis *chassis;
	struct lldpd_port *port;
	struct lldpd_mgmt *mgmt;
	struct in_addr addr;
#if 0
	u_int16_t cksum;
#endif
	u_int8_t *software = NULL, *platform = NULL;
	int software_len = 0, platform_len = 0, proto, version, nb, caps;
	const unsigned char cdpaddr[] = CDP_MULTICAST_ADDR;
#ifdef ENABLE_FDP
	const unsigned char fdpaddr[] = CDP_MULTICAST_ADDR;
	int fdp = 0;
#endif
	u_int8_t *pos, *tlv, *pos_address, *pos_next_address;
	int length, len_eth, tlv_type, tlv_len, addresses_len, address_len;
#ifdef ENABLE_DOT1
	struct lldpd_vlan *vlan;
#endif

	log_debug("cdp", "decode CDP frame received on %s",
	    hardware->h_ifname);

	if ((chassis = calloc(1, sizeof(struct lldpd_chassis))) == NULL) {
		log_warn("cdp", "failed to allocate remote chassis");
		return -1;
	}
	TAILQ_INIT(&chassis->c_mgmt);
	if ((port = calloc(1, sizeof(struct lldpd_port))) == NULL) {
		log_warn("cdp", "failed to allocate remote port");
		free(chassis);
		return -1;
	}
#ifdef ENABLE_DOT1
	TAILQ_INIT(&port->p_vlans);
#endif

	length = s;
	pos = (u_int8_t*)frame;

	if (length < 2*ETHER_ADDR_LEN + sizeof(u_int16_t) /* Ethernet */ +
	    8 /* LLC */ + 4 /* CDP header */) {
		log_warn("cdp", "too short CDP/FDP frame received on %s", hardware->h_ifname);
		goto malformed;
	}

	if (PEEK_CMP(cdpaddr, sizeof(cdpaddr)) != 0) {
#ifdef ENABLE_FDP
		PEEK_RESTORE((u_int8_t*)frame);
		if (PEEK_CMP(fdpaddr, sizeof(fdpaddr)) != 0)
			fdp = 1;
		else {
#endif
			log_info("cdp", "frame not targeted at CDP/FDP multicast address received on %s",
			    hardware->h_ifname);
			goto malformed;
#ifdef ENABLE_FDP
		}
#endif
	}
	PEEK_DISCARD(ETHER_ADDR_LEN);	/* Don't care of source address */
	len_eth = PEEK_UINT16;
	if (len_eth > length) {
		log_warnx("cdp", "incorrect 802.3 frame size reported on %s",
		    hardware->h_ifname);
		goto malformed;
	}
	PEEK_DISCARD(6);	/* Skip beginning of LLC */
	proto = PEEK_UINT16;
	if (proto != LLC_PID_CDP) {
		if ((proto != LLC_PID_DRIP) &&
		    (proto != LLC_PID_PAGP) &&
		    (proto != LLC_PID_PVSTP) &&
		    (proto != LLC_PID_UDLD) &&
		    (proto != LLC_PID_VTP) &&
		    (proto != LLC_PID_DTP) &&
		    (proto != LLC_PID_STP))
			log_debug("cdp", "incorrect LLC protocol ID received on %s",
			    hardware->h_ifname);
		goto malformed;
	}

#if 0
	/* Check checksum */
	cksum = frame_checksum(pos, len_eth - 8,
#ifdef ENABLE_FDP
	    !fdp		/* fdp = 0 -> cisco checksum */
#else
	    1			/* cisco checksum */
#endif
		);
	if (cksum != 0) {
		log_info("cdp", "incorrect CDP/FDP checksum for frame received on %s (%d)",
			  hardware->h_ifname, cksum);
		goto malformed;
	}
#endif

	/* Check version */
	version = PEEK_UINT8;
	if ((version != 1) && (version != 2)) {
		log_warnx("cdp", "incorrect CDP/FDP version (%d) for frame received on %s",
		    version, hardware->h_ifname);
		goto malformed;
	}
	port->p_ttl = PEEK_UINT8; /* TTL */
	PEEK_DISCARD_UINT16;	     /* Checksum, already checked */

	while (length) {
		if (length < 4) {
			log_warnx("cdp", "CDP/FDP TLV header is too large for "
			    "frame received on %s",
			    hardware->h_ifname);
			goto malformed;
		}
		tlv_type = PEEK_UINT16;
		tlv_len = PEEK_UINT16 - 4;
		(void)PEEK_SAVE(tlv);
		if ((tlv_len < 0) || (length < tlv_len)) {
			log_warnx("cdp", "incorrect size in CDP/FDP TLV header for frame "
			    "received on %s",
			    hardware->h_ifname);
			goto malformed;
		}
		switch (tlv_type) {
		case CDP_TLV_CHASSIS:
			if ((chassis->c_name = (char *)calloc(1, tlv_len + 1)) == NULL) {
				log_warn("cdp", "unable to allocate memory for chassis name");
				goto malformed;
			}
			PEEK_BYTES(chassis->c_name, tlv_len);
			chassis->c_id_subtype = LLDP_CHASSISID_SUBTYPE_LOCAL;
			if ((chassis->c_id =  (char *)malloc(tlv_len)) == NULL) {
				log_warn("cdp", "unable to allocate memory for chassis ID");
				goto malformed;
			}
			memcpy(chassis->c_id, chassis->c_name, tlv_len);
			chassis->c_id_len = tlv_len;
			break;
		case CDP_TLV_ADDRESSES:
			CHECK_TLV_SIZE(4, "Address");
			addresses_len = tlv_len - 4;
			for (nb = PEEK_UINT32; nb > 0; nb--) {
				(void)PEEK_SAVE(pos_address);
				/* We first try to get the real length of the packet */
				if (addresses_len < 2) {
					log_warn("cdp", "too short address subframe "
						  "received on %s",
						  hardware->h_ifname);
					goto malformed;
				}
				PEEK_DISCARD_UINT8; addresses_len--;
				address_len = PEEK_UINT8; addresses_len--;
				if (addresses_len < address_len + 2) {
					log_warn("cdp", "too short address subframe "
						  "received on %s",
						  hardware->h_ifname);
					goto malformed;
				}
				PEEK_DISCARD(address_len);
				addresses_len -= address_len;
				address_len = PEEK_UINT16; addresses_len -= 2;
				if (addresses_len < address_len) {
					log_warn("cdp", "too short address subframe "
						  "received on %s",
						  hardware->h_ifname);
					goto malformed;
				}
				PEEK_DISCARD(address_len);
				(void)PEEK_SAVE(pos_next_address);
				/* Next, we go back and try to extract
				   IPv4 address */
				PEEK_RESTORE(pos_address);
				if ((PEEK_UINT8 == 1) && (PEEK_UINT8 == 1) &&
				    (PEEK_UINT8 == CDP_ADDRESS_PROTO_IP) &&
				    (PEEK_UINT16 == sizeof(struct in_addr))) {
						PEEK_BYTES(&addr, sizeof(struct in_addr));
						mgmt = lldpd_alloc_mgmt(LLDPD_AF_IPV4, &addr, 
									sizeof(struct in_addr), 0);
						if (mgmt == NULL) {
							if (errno == ENOMEM)
								log_warn("cdp",
								    "unable to allocate memory for management address");
							else
								log_warn("cdp",
								    "too large management address received on %s",
								    hardware->h_ifname);
							goto malformed;
						}
						TAILQ_INSERT_TAIL(&chassis->c_mgmt, mgmt, m_entries);
				}
				/* Go to the end of the address */
				PEEK_RESTORE(pos_next_address);
			}
			break;
		case CDP_TLV_PORT:
			if (tlv_len == 0) {
				log_warn("cdp", "too short port description received");
				goto malformed;
			}
			if ((port->p_descr = (char *)calloc(1, tlv_len + 1)) == NULL) {
				log_warn("cdp", "unable to allocate memory for port description");
				goto malformed;
			}
			PEEK_BYTES(port->p_descr, tlv_len);
			port->p_id_subtype = LLDP_PORTID_SUBTYPE_IFNAME;
			if ((port->p_id =  (char *)calloc(1, tlv_len)) == NULL) {
				log_warn("cdp", "unable to allocate memory for port ID");
				goto malformed;
			}
			memcpy(port->p_id, port->p_descr, tlv_len);
			port->p_id_len = tlv_len;
			break;
		case CDP_TLV_CAPABILITIES:
#ifdef ENABLE_FDP
			if (fdp) {
				/* Capabilities are string with FDP */
				if (!strncmp("Router", (char*)pos, tlv_len))
					chassis->c_cap_enabled = LLDP_CAP_ROUTER;
				else if (!strncmp("Switch", (char*)pos, tlv_len))
					chassis->c_cap_enabled = LLDP_CAP_BRIDGE;
				else if (!strncmp("Bridge", (char*)pos, tlv_len))
					chassis->c_cap_enabled = LLDP_CAP_REPEATER;
				else
					chassis->c_cap_enabled = LLDP_CAP_STATION;
				chassis->c_cap_available = chassis->c_cap_enabled;
				break;
			}
#endif
			CHECK_TLV_SIZE(4, "Capabilities");
			caps = PEEK_UINT32;
			if (caps & CDP_CAP_ROUTER)
				chassis->c_cap_enabled |= LLDP_CAP_ROUTER;
			if (caps & 0x0e)
				chassis->c_cap_enabled |= LLDP_CAP_BRIDGE;
			if (chassis->c_cap_enabled == 0)
				chassis->c_cap_enabled = LLDP_CAP_STATION;
			chassis->c_cap_available = chassis->c_cap_enabled;
			break;
		case CDP_TLV_SOFTWARE:
			software_len = tlv_len;
			(void)PEEK_SAVE(software);
			break;
		case CDP_TLV_PLATFORM:
			platform_len = tlv_len;
			(void)PEEK_SAVE(platform);
			break;
#ifdef ENABLE_DOT1
		case CDP_TLV_NATIVEVLAN:
			CHECK_TLV_SIZE(2, "Native VLAN");
			if ((vlan = (struct lldpd_vlan *)calloc(1,
				sizeof(struct lldpd_vlan))) == NULL) {
				log_warn("cdp", "unable to alloc vlan "
					  "structure for "
					  "tlv received on %s",
					  hardware->h_ifname);
				goto malformed;
			}
			vlan->v_vid = port->p_pvid = PEEK_UINT16;
			if (asprintf(&vlan->v_name, "VLAN #%d", vlan->v_vid) == -1) {
				log_warn("cdp", "unable to alloc VLAN name for "
					  "TLV received on %s",
					  hardware->h_ifname);
				free(vlan);
				goto malformed;
			}
			TAILQ_INSERT_TAIL(&port->p_vlans,
					  vlan, v_entries);
			break;
#endif
		default:
			log_debug("cdp", "unknown CDP/FDP TLV type (%d) received on %s",
			    ntohs(tlv_type), hardware->h_ifname);
			hardware->h_rx_unrecognized_cnt++;
		}
		PEEK_DISCARD(tlv + tlv_len - pos);
	}
	if (!software && platform) {
		if ((chassis->c_descr = (char *)calloc(1,
			    platform_len + 1)) == NULL) {
			log_warn("cdp", "unable to allocate memory for chassis description");
			goto malformed;
		}
		memcpy(chassis->c_descr, platform, platform_len);
	} else if (software && !platform) {
		if ((chassis->c_descr = (char *)calloc(1,
			    software_len + 1)) == NULL) {
			log_warn("cdp", "unable to allocate memory for chassis description");
			goto malformed;
		}
		memcpy(chassis->c_descr, software, software_len);
	} else if (software && platform) {
#define CONCAT_PLATFORM " running on\n"
		if ((chassis->c_descr = (char *)calloc(1,
			    software_len + platform_len +
			    strlen(CONCAT_PLATFORM) + 1)) == NULL) {
			log_warn("cdp", "unable to allocate memory for chassis description");
			goto malformed;
		}
		memcpy(chassis->c_descr, platform, platform_len);
		memcpy(chassis->c_descr + platform_len,
		    CONCAT_PLATFORM, strlen(CONCAT_PLATFORM));
		memcpy(chassis->c_descr + platform_len + strlen(CONCAT_PLATFORM),
		    software, software_len);
	}
	if ((chassis->c_id == NULL) ||
	    (port->p_id == NULL) ||
	    (chassis->c_name == NULL) ||
	    (chassis->c_descr == NULL) ||
	    (port->p_descr == NULL) ||
	    (port->p_ttl == 0) ||
	    (chassis->c_cap_enabled == 0)) {
		log_warnx("cdp", "some mandatory CDP/FDP tlv are missing for frame received on %s",
		    hardware->h_ifname);
		goto malformed;
	}
	*newchassis = chassis;
	*newport = port;
	return 1;

malformed:
	lldpd_chassis_cleanup(chassis, 1);
	lldpd_port_cleanup(port, 1);
	free(port);
	return -1;
}

#ifdef ENABLE_CDP
int
cdpv1_send(struct lldpd *global,
    struct lldpd_hardware *hardware)
{
	return cdp_send(global, hardware, 1);
}

int
cdpv2_send(struct lldpd *global,
    struct lldpd_hardware *hardware)
{
	return cdp_send(global, hardware, 2);
}
#endif

#ifdef ENABLE_FDP
int
fdp_send(struct lldpd *global,
    struct lldpd_hardware *hardware)
{
	return cdp_send(global, hardware, 0);
}
#endif

#ifdef ENABLE_CDP
static int
cdp_guess(char *pos, int length, int version)
{
	const u_int8_t mcastaddr[] = CDP_MULTICAST_ADDR;
	if (length < 2*ETHER_ADDR_LEN + sizeof(u_int16_t) /* Ethernet */ +
	    8 /* LLC */ + 4 /* CDP header */)
		return 0;
	if (PEEK_CMP(mcastaddr, ETHER_ADDR_LEN) != 0)
		return 0;
	PEEK_DISCARD(ETHER_ADDR_LEN); PEEK_DISCARD_UINT16; /* Ethernet */
	PEEK_DISCARD(8);			     /* LLC */
	return (PEEK_UINT8 == version);
}

int
cdpv1_guess(char *frame, int len)
{
	return cdp_guess(frame, len, 1);
}

int
cdpv2_guess(char *frame, int len)
{
	return cdp_guess(frame, len, 2);
}
#endif

#endif /* defined (ENABLE_CDP) || defined (ENABLE_FDP) */
