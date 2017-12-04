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

#include "lldpd.h"
#include "frame.h"

#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

inline static int
lldpd_af_to_lldp_proto(int af)
{
	switch (af) {
	case LLDPD_AF_IPV4:
		return LLDP_MGMT_ADDR_IP4;
	case LLDPD_AF_IPV6:
		return LLDP_MGMT_ADDR_IP6;
	default:
		return LLDP_MGMT_ADDR_NONE;
	}
}

inline static int
lldpd_af_from_lldp_proto(int proto)
{
	switch (proto) {
	case LLDP_MGMT_ADDR_IP4:
		return LLDPD_AF_IPV4;
	case LLDP_MGMT_ADDR_IP6:
		return LLDPD_AF_IPV6;
	default:
		return LLDPD_AF_UNSPEC;
	}
}

static int _lldp_send(struct lldpd *global,
    struct lldpd_hardware *hardware,
    u_int8_t c_id_subtype,
    char *c_id,
    int c_id_len,
    u_int8_t p_id_subtype,
    char *p_id,
    int p_id_len,
    int shutdown)
{
	struct lldpd_port *port;
	struct lldpd_chassis *chassis;
	struct lldpd_frame *frame;
	int length;
	u_int8_t *packet, *pos, *tlv;
	struct lldpd_mgmt *mgmt;
	int proto;

	u_int8_t mcastaddr_regular[] = LLDP_ADDR_NEAREST_BRIDGE;
	u_int8_t mcastaddr_nontpmr[] = LLDP_ADDR_NEAREST_NONTPMR_BRIDGE;
	u_int8_t mcastaddr_customer[] = LLDP_ADDR_NEAREST_CUSTOMER_BRIDGE;
	u_int8_t *mcastaddr;
#ifdef ENABLE_DOT1
	const u_int8_t dot1[] = LLDP_TLV_ORG_DOT1;
	struct lldpd_vlan *vlan;
	struct lldpd_ppvid *ppvid;
	struct lldpd_pi *pi;
#endif
#ifdef ENABLE_DOT3
	const u_int8_t dot3[] = LLDP_TLV_ORG_DOT3;
#endif
#ifdef ENABLE_LLDPMED
	int i;
	const u_int8_t med[] = LLDP_TLV_ORG_MED;
#endif
#ifdef ENABLE_CUSTOM
	struct lldpd_custom *custom;
#endif
	port = &hardware->h_lport;
	chassis = port->p_chassis;
	length = hardware->h_mtu;
	if ((packet = (u_int8_t*)calloc(1, length)) == NULL)
		return ENOMEM;
	pos = packet;

	/* Ethernet header */
	switch (global->g_config.c_lldp_agent_type) {
	case LLDP_AGENT_TYPE_NEAREST_NONTPMR_BRIDGE: mcastaddr = mcastaddr_nontpmr; break;
	case LLDP_AGENT_TYPE_NEAREST_CUSTOMER_BRIDGE: mcastaddr = mcastaddr_customer; break;
	case LLDP_AGENT_TYPE_NEAREST_BRIDGE:
	default: mcastaddr = mcastaddr_regular; break;
	}
	if (!(
	      /* LLDP multicast address */
	      POKE_BYTES(mcastaddr, ETHER_ADDR_LEN) &&
	      /* Source MAC address */
	      POKE_BYTES(&hardware->h_lladdr, ETHER_ADDR_LEN) &&
	      /* LLDP frame */
	      POKE_UINT16(ETHERTYPE_LLDP)))
		goto toobig;

	/* Chassis ID */
	if (!(
	      POKE_START_LLDP_TLV(LLDP_TLV_CHASSIS_ID) &&
	      POKE_UINT8(c_id_subtype) &&
	      POKE_BYTES(c_id, c_id_len) &&
	      POKE_END_LLDP_TLV))
		goto toobig;

	/* Port ID */
	if (!(
	      POKE_START_LLDP_TLV(LLDP_TLV_PORT_ID) &&
	      POKE_UINT8(p_id_subtype) &&
	      POKE_BYTES(p_id, p_id_len) &&
	      POKE_END_LLDP_TLV))
		goto toobig;

	/* Time to live */
	if (!(
	      POKE_START_LLDP_TLV(LLDP_TLV_TTL) &&
	      POKE_UINT16(shutdown?0:(global?global->g_config.c_ttl:180)) &&
	      POKE_END_LLDP_TLV))
		goto toobig;

	if (shutdown)
		goto end;

	/* System name */
	if (chassis->c_name && *chassis->c_name != '\0') {
		if (!(
			    POKE_START_LLDP_TLV(LLDP_TLV_SYSTEM_NAME) &&
			    POKE_BYTES(chassis->c_name, strlen(chassis->c_name)) &&
			    POKE_END_LLDP_TLV))
			goto toobig;
	}

	/* System description (skip it if empty) */
	if (chassis->c_descr && *chassis->c_descr != '\0') {
		if (!(
			    POKE_START_LLDP_TLV(LLDP_TLV_SYSTEM_DESCR) &&
			    POKE_BYTES(chassis->c_descr, strlen(chassis->c_descr)) &&
			    POKE_END_LLDP_TLV))
			goto toobig;
	}

	/* System capabilities */
	if (global->g_config.c_cap_advertise && chassis->c_cap_available) {
		if (!(
			    POKE_START_LLDP_TLV(LLDP_TLV_SYSTEM_CAP) &&
			    POKE_UINT16(chassis->c_cap_available) &&
			    POKE_UINT16(chassis->c_cap_enabled) &&
			    POKE_END_LLDP_TLV))
			goto toobig;
	}

	/* Management addresses */
	TAILQ_FOREACH(mgmt, &chassis->c_mgmt, m_entries) {
		proto = lldpd_af_to_lldp_proto(mgmt->m_family);
		if (proto == LLDP_MGMT_ADDR_NONE) continue;
		if (!(
			  POKE_START_LLDP_TLV(LLDP_TLV_MGMT_ADDR) &&
			  /* Size of the address, including its type */
			  POKE_UINT8(mgmt->m_addrsize + 1) &&
			  POKE_UINT8(proto) &&
			  POKE_BYTES(&mgmt->m_addr, mgmt->m_addrsize)))
			goto toobig;

		/* Interface port type, OID */
		if (mgmt->m_iface == 0) {
			if (!(
				  /* We don't know the management interface */
				  POKE_UINT8(LLDP_MGMT_IFACE_UNKNOWN) &&
				  POKE_UINT32(0)))
				goto toobig;
		} else {
			if (!(
				  /* We have the index of the management interface */
				  POKE_UINT8(LLDP_MGMT_IFACE_IFINDEX) &&
				  POKE_UINT32(mgmt->m_iface)))
				goto toobig;
		}
		if (!(
			  /* We don't provide an OID for management */
			  POKE_UINT8(0) &&
			  POKE_END_LLDP_TLV))
			goto toobig;
	}

	/* Port description */
	if (port->p_descr && *port->p_descr != '\0') {
		if (!(
			    POKE_START_LLDP_TLV(LLDP_TLV_PORT_DESCR) &&
			    POKE_BYTES(port->p_descr, strlen(port->p_descr)) &&
			    POKE_END_LLDP_TLV))
			goto toobig;
	}

#ifdef ENABLE_DOT1
	/* Port VLAN ID */
	if(port->p_pvid != 0) {
		if (!(
		    POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&
		    POKE_BYTES(dot1, sizeof(dot1)) &&
		    POKE_UINT8(LLDP_TLV_DOT1_PVID) &&
		    POKE_UINT16(port->p_pvid) &&
		    POKE_END_LLDP_TLV)) {
		    goto toobig;
		}
	}
	/* Port and Protocol VLAN IDs */
	TAILQ_FOREACH(ppvid, &port->p_ppvids, p_entries) {
		if (!(
		      POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&
		      POKE_BYTES(dot1, sizeof(dot1)) &&
		      POKE_UINT8(LLDP_TLV_DOT1_PPVID) &&
		      POKE_UINT8(ppvid->p_cap_status) &&
		      POKE_UINT16(ppvid->p_ppvid) &&
		      POKE_END_LLDP_TLV)) {
			goto toobig;
		}
	}
	/* VLANs */
	TAILQ_FOREACH(vlan, &port->p_vlans, v_entries) {
		if (!(
		      POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&
		      POKE_BYTES(dot1, sizeof(dot1)) &&
		      POKE_UINT8(LLDP_TLV_DOT1_VLANNAME) &&
		      POKE_UINT16(vlan->v_vid) &&
		      POKE_UINT8(strlen(vlan->v_name)) &&
		      POKE_BYTES(vlan->v_name, strlen(vlan->v_name)) &&
		      POKE_END_LLDP_TLV))
			goto toobig;
	}
	/* Protocol Identities */
	TAILQ_FOREACH(pi, &port->p_pids, p_entries) {
		if (!(
		      POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&
		      POKE_BYTES(dot1, sizeof(dot1)) &&
		      POKE_UINT8(LLDP_TLV_DOT1_PI) &&
		      POKE_UINT8(pi->p_pi_len) &&
		      POKE_BYTES(pi->p_pi, pi->p_pi_len) &&
		      POKE_END_LLDP_TLV))
			goto toobig;
	}
#endif

#ifdef ENABLE_DOT3
	/* Aggregation status */
	if (!(
	      POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&
	      POKE_BYTES(dot3, sizeof(dot3)) &&
	      POKE_UINT8(LLDP_TLV_DOT3_LA) &&
	      /* Bit 0 = capability ; Bit 1 = status */
	      POKE_UINT8((port->p_aggregid) ? 3:1) &&
	      POKE_UINT32(port->p_aggregid) &&
	      POKE_END_LLDP_TLV))
		goto toobig;

	/* MAC/PHY */
	if (!(
	      POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&
	      POKE_BYTES(dot3, sizeof(dot3)) &&
	      POKE_UINT8(LLDP_TLV_DOT3_MAC) &&
	      POKE_UINT8(port->p_macphy.autoneg_support |
			 (port->p_macphy.autoneg_enabled << 1)) &&
	      POKE_UINT16(port->p_macphy.autoneg_advertised) &&
	      POKE_UINT16(port->p_macphy.mau_type) &&
	      POKE_END_LLDP_TLV))
		goto toobig;

	/* MFS */
	if (port->p_mfs) {
		if (!(
		      POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&
		      POKE_BYTES(dot3, sizeof(dot3)) &&
		      POKE_UINT8(LLDP_TLV_DOT3_MFS) &&
		      POKE_UINT16(port->p_mfs) &&
		      POKE_END_LLDP_TLV))
			goto toobig;
	}
	/* Power */
	if (port->p_power.devicetype) {
		if (!(
		      POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&
		      POKE_BYTES(dot3, sizeof(dot3)) &&
		      POKE_UINT8(LLDP_TLV_DOT3_POWER) &&
		      POKE_UINT8((
				  (((2 - port->p_power.devicetype)    %(1<< 1))<<0) |
				  (( port->p_power.supported          %(1<< 1))<<1) |
				  (( port->p_power.enabled            %(1<< 1))<<2) |
				  (( port->p_power.paircontrol        %(1<< 1))<<3))) &&
		      POKE_UINT8(port->p_power.pairs) &&
		      POKE_UINT8(port->p_power.class)))
			goto toobig;
		/* 802.3at */
		if (port->p_power.powertype != LLDP_DOT3_POWER_8023AT_OFF) {
			if (!(
			      POKE_UINT8((
					  (((port->p_power.powertype ==
					      LLDP_DOT3_POWER_8023AT_TYPE1)?1:0) << 7) |
					   (((port->p_power.devicetype ==
					      LLDP_DOT3_POWER_PSE)?0:1) << 6) |
					   ((port->p_power.source   %(1<< 2))<<4) |
					   ((port->p_power.priority %(1<< 2))<<0))) &&
			      POKE_UINT16(port->p_power.requested) &&
			      POKE_UINT16(port->p_power.allocated)))
				goto toobig;
		}
		if (!(POKE_END_LLDP_TLV))
			goto toobig;
	}
#endif

#ifdef ENABLE_LLDPMED
	if (port->p_med_cap_enabled) {
		/* LLDP-MED cap */
		if (port->p_med_cap_enabled & LLDP_MED_CAP_CAP) {
			if (!(
				    POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&
				    POKE_BYTES(med, sizeof(med)) &&
				    POKE_UINT8(LLDP_TLV_MED_CAP) &&
				    POKE_UINT16(chassis->c_med_cap_available) &&
				    POKE_UINT8(chassis->c_med_type) &&
				    POKE_END_LLDP_TLV))
				goto toobig;
		}

		/* LLDP-MED inventory */
#define LLDP_INVENTORY(value, subtype)					\
		if (value) {						\
		    if (!(						\
			  POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&		\
			  POKE_BYTES(med, sizeof(med)) &&		\
			  POKE_UINT8(subtype) &&			\
			  POKE_BYTES(value,				\
				(strlen(value)>32)?32:strlen(value)) &&	\
			  POKE_END_LLDP_TLV))				\
			    goto toobig;				\
		}

		if (port->p_med_cap_enabled & LLDP_MED_CAP_IV) {
			LLDP_INVENTORY(chassis->c_med_hw,
			    LLDP_TLV_MED_IV_HW);
			LLDP_INVENTORY(chassis->c_med_fw,
			    LLDP_TLV_MED_IV_FW);
			LLDP_INVENTORY(chassis->c_med_sw,
			    LLDP_TLV_MED_IV_SW);
			LLDP_INVENTORY(chassis->c_med_sn,
			    LLDP_TLV_MED_IV_SN);
			LLDP_INVENTORY(chassis->c_med_manuf,
			    LLDP_TLV_MED_IV_MANUF);
			LLDP_INVENTORY(chassis->c_med_model,
			    LLDP_TLV_MED_IV_MODEL);
			LLDP_INVENTORY(chassis->c_med_asset,
			    LLDP_TLV_MED_IV_ASSET);
		}

		/* LLDP-MED location */
		for (i = 0; i < LLDP_MED_LOCFORMAT_LAST; i++) {
			if (port->p_med_location[i].format == i + 1) {
				if (!(
				      POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&
				      POKE_BYTES(med, sizeof(med)) &&
				      POKE_UINT8(LLDP_TLV_MED_LOCATION) &&
				      POKE_UINT8(port->p_med_location[i].format) &&
				      POKE_BYTES(port->p_med_location[i].data,
					  port->p_med_location[i].data_len) &&
				      POKE_END_LLDP_TLV))
					goto toobig;
			}
		}

		/* LLDP-MED network policy */
		for (i = 0; i < LLDP_MED_APPTYPE_LAST; i++) {
			if (port->p_med_policy[i].type == i + 1) {
				if (!(
				      POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&
				      POKE_BYTES(med, sizeof(med)) &&
				      POKE_UINT8(LLDP_TLV_MED_POLICY) &&
				      POKE_UINT32((
					((port->p_med_policy[i].type     %(1<< 8))<<24) |
					((port->p_med_policy[i].unknown  %(1<< 1))<<23) |
					((port->p_med_policy[i].tagged   %(1<< 1))<<22) |
				      /*((0                              %(1<< 1))<<21) |*/
					((port->p_med_policy[i].vid      %(1<<12))<< 9) |
					((port->p_med_policy[i].priority %(1<< 3))<< 6) |
					((port->p_med_policy[i].dscp     %(1<< 6))<< 0) )) &&
				      POKE_END_LLDP_TLV))
					goto toobig;
			}
		}

		/* LLDP-MED POE-MDI */
		if ((port->p_med_power.devicetype == LLDP_MED_POW_TYPE_PSE) ||
		    (port->p_med_power.devicetype == LLDP_MED_POW_TYPE_PD)) {
			int devicetype = 0, source = 0;
			if (!(
			      POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&
			      POKE_BYTES(med, sizeof(med)) &&
			      POKE_UINT8(LLDP_TLV_MED_MDI)))
				goto toobig;
			switch (port->p_med_power.devicetype) {
			case LLDP_MED_POW_TYPE_PSE:
				devicetype = 0;
				switch (port->p_med_power.source) {
				case LLDP_MED_POW_SOURCE_PRIMARY: source = 1; break;
				case LLDP_MED_POW_SOURCE_BACKUP: source = 2; break;
				case LLDP_MED_POW_SOURCE_RESERVED: source = 3; break;
				default: source = 0; break;
				}
				break;
			case LLDP_MED_POW_TYPE_PD:
				devicetype = 1;
				switch (port->p_med_power.source) {
				case LLDP_MED_POW_SOURCE_PSE: source = 1; break;
				case LLDP_MED_POW_SOURCE_LOCAL: source = 2; break;
				case LLDP_MED_POW_SOURCE_BOTH: source = 3; break;
				default: source = 0; break;
				}
				break;
			}
			if (!(
			      POKE_UINT8((
				((devicetype                   %(1<< 2))<<6) |
				((source                       %(1<< 2))<<4) |
				((port->p_med_power.priority   %(1<< 4))<<0) )) &&
			      POKE_UINT16(port->p_med_power.val) &&
			      POKE_END_LLDP_TLV))
				goto toobig;
		}
	}
#endif

#ifdef ENABLE_CUSTOM
	TAILQ_FOREACH(custom, &port->p_custom_list, next) {
		if (!(
		      POKE_START_LLDP_TLV(LLDP_TLV_ORG) &&
		      POKE_BYTES(custom->oui, sizeof(custom->oui)) &&
		      POKE_UINT8(custom->subtype) &&
		      POKE_BYTES(custom->oui_info, custom->oui_info_len) &&
		      POKE_END_LLDP_TLV))
			goto toobig;
	}
#endif

end:
	/* END */
	if (!(
	      POKE_START_LLDP_TLV(LLDP_TLV_END) &&
	      POKE_END_LLDP_TLV))
		goto toobig;

	if (interfaces_send_helper(global, hardware,
		(char *)packet, pos - packet) == -1) {
		log_warn("lldp", "unable to send packet on real device for %s",
		    hardware->h_ifname);
		free(packet);
		return ENETDOWN;
	}

	hardware->h_tx_cnt++;

	/* We assume that LLDP frame is the reference */
	if (!shutdown && (frame = (struct lldpd_frame*)malloc(
			sizeof(int) + pos - packet)) != NULL) {
		frame->size = pos - packet;
		memcpy(&frame->frame, packet, frame->size);
		if ((hardware->h_lport.p_lastframe == NULL) ||
		    (hardware->h_lport.p_lastframe->size != frame->size) ||
		    (memcmp(hardware->h_lport.p_lastframe->frame, frame->frame,
			frame->size) != 0)) {
			free(hardware->h_lport.p_lastframe);
			hardware->h_lport.p_lastframe = frame;
			hardware->h_lport.p_lastchange = time(NULL);
		} else free(frame);
	}

	free(packet);
	return 0;

toobig:
	free(packet);
	return E2BIG;
}

/* Send a shutdown LLDPDU. */
int
lldp_send_shutdown(struct lldpd *global,
    struct lldpd_hardware *hardware)
{
	if (hardware->h_lchassis_previous_id == NULL ||
	    hardware->h_lport_previous_id == NULL)
		return 0;
	return _lldp_send(global, hardware,
	    hardware->h_lchassis_previous_id_subtype,
	    hardware->h_lchassis_previous_id,
	    hardware->h_lchassis_previous_id_len,
	    hardware->h_lport_previous_id_subtype,
	    hardware->h_lport_previous_id,
	    hardware->h_lport_previous_id_len,
	    1);
}

int
lldp_send(struct lldpd *global,
	  struct lldpd_hardware *hardware)
{
	struct lldpd_port *port = &hardware->h_lport;
	struct lldpd_chassis *chassis = port->p_chassis;
	int ret;

	/* Check if we have a change. */
	if (hardware->h_lchassis_previous_id != NULL &&
	    hardware->h_lport_previous_id != NULL &&
	    (hardware->h_lchassis_previous_id_subtype != chassis->c_id_subtype ||
		hardware->h_lchassis_previous_id_len != chassis->c_id_len ||
		hardware->h_lport_previous_id_subtype != port->p_id_subtype ||
		hardware->h_lport_previous_id_len != port->p_id_len ||
		memcmp(hardware->h_lchassis_previous_id,
		    chassis->c_id, chassis->c_id_len) ||
		memcmp(hardware->h_lport_previous_id,
		    port->p_id, port->p_id_len))) {
		log_info("lldp", "MSAP has changed for port %s, sending a shutdown LLDPDU",
		    hardware->h_ifname);
		if ((ret = lldp_send_shutdown(global, hardware)) != 0)
			return ret;
	}

	log_debug("lldp", "send LLDP PDU to %s",
	    hardware->h_ifname);

	if ((ret = _lldp_send(global, hardware,
		    chassis->c_id_subtype,
		    chassis->c_id,
		    chassis->c_id_len,
		    port->p_id_subtype,
		    port->p_id,
		    port->p_id_len,
		    0)) != 0)
		return ret;

	/* Record current chassis and port ID */
	free(hardware->h_lchassis_previous_id);
	hardware->h_lchassis_previous_id_subtype = chassis->c_id_subtype;
	hardware->h_lchassis_previous_id_len = chassis->c_id_len;
	if ((hardware->h_lchassis_previous_id = malloc(chassis->c_id_len)) != NULL)
		memcpy(hardware->h_lchassis_previous_id, chassis->c_id,
		    chassis->c_id_len);
	free(hardware->h_lport_previous_id);
	hardware->h_lport_previous_id_subtype = port->p_id_subtype;
	hardware->h_lport_previous_id_len = port->p_id_len;
	if ((hardware->h_lport_previous_id = malloc(port->p_id_len)) != NULL)
		memcpy(hardware->h_lport_previous_id, port->p_id,
		    port->p_id_len);

	return 0;
}

#define CHECK_TLV_SIZE(x, name)				   \
	do { if (tlv_size < (x)) {			   \
			log_warnx("lldp", name " TLV too short received on %s",	\
	       hardware->h_ifname);			   \
	   goto malformed;				   \
	} } while (0)

int
lldp_decode(struct lldpd *cfg, char *frame, int s,
    struct lldpd_hardware *hardware,
    struct lldpd_chassis **newchassis, struct lldpd_port **newport)
{
	struct lldpd_chassis *chassis;
	struct lldpd_port *port;
	char lldpaddr[ETHER_ADDR_LEN];
	const char dot1[] = LLDP_TLV_ORG_DOT1;
	const char dot3[] = LLDP_TLV_ORG_DOT3;
	const char med[] = LLDP_TLV_ORG_MED;
	const char dcbx[] = LLDP_TLV_ORG_DCBX;
	unsigned char orgid[3];
	int length, gotend = 0, ttl_received = 0;
	int tlv_size, tlv_type, tlv_subtype;
	u_int8_t *pos, *tlv;
	char *b;
#ifdef ENABLE_DOT1
	struct lldpd_vlan *vlan = NULL;
	int vlan_len;
	struct lldpd_ppvid *ppvid;
	struct lldpd_pi *pi = NULL;
#endif
	struct lldpd_mgmt *mgmt;
	int af;
	u_int8_t addr_str_length, addr_str_buffer[32];
	u_int8_t addr_family, addr_length, *addr_ptr, iface_subtype;
	u_int32_t iface_number, iface;
#ifdef ENABLE_CUSTOM
	struct lldpd_custom *custom = NULL;
#endif

	log_debug("lldp", "receive LLDP PDU on %s",
	    hardware->h_ifname);

	if ((chassis = calloc(1, sizeof(struct lldpd_chassis))) == NULL) {
		log_warn("lldp", "failed to allocate remote chassis");
		return -1;
	}
	TAILQ_INIT(&chassis->c_mgmt);
	if ((port = calloc(1, sizeof(struct lldpd_port))) == NULL) {
		log_warn("lldp", "failed to allocate remote port");
		free(chassis);
		return -1;
	}
#ifdef ENABLE_DOT1
	TAILQ_INIT(&port->p_vlans);
	TAILQ_INIT(&port->p_ppvids);
	TAILQ_INIT(&port->p_pids);
#endif
#ifdef ENABLE_CUSTOM
	TAILQ_INIT(&port->p_custom_list);
#endif

	length = s;
	pos = (u_int8_t*)frame;

	if (length < 2*ETHER_ADDR_LEN + sizeof(u_int16_t)) {
		log_warnx("lldp", "too short frame received on %s", hardware->h_ifname);
		goto malformed;
	}
	PEEK_BYTES(lldpaddr, ETHER_ADDR_LEN);
	if (memcmp(lldpaddr, (const char [])LLDP_ADDR_NEAREST_BRIDGE, ETHER_ADDR_LEN) &&
	    memcmp(lldpaddr, (const char [])LLDP_ADDR_NEAREST_NONTPMR_BRIDGE, ETHER_ADDR_LEN) &&
	    memcmp(lldpaddr, (const char [])LLDP_ADDR_NEAREST_CUSTOMER_BRIDGE, ETHER_ADDR_LEN)) {
		log_info("lldp", "frame not targeted at LLDP multicast address received on %s",
		    hardware->h_ifname);
		goto malformed;
	}
	PEEK_DISCARD(ETHER_ADDR_LEN);	/* Skip source address */
	if (PEEK_UINT16 != ETHERTYPE_LLDP) {
		log_info("lldp", "non LLDP frame received on %s",
		    hardware->h_ifname);
		goto malformed;
	}

	while (length && (!gotend)) {
		if (length < 2) {
			log_warnx("lldp", "tlv header too short received on %s",
			    hardware->h_ifname);
			goto malformed;
		}
		tlv_size = PEEK_UINT16;
		tlv_type = tlv_size >> 9;
		tlv_size = tlv_size & 0x1ff;
		(void)PEEK_SAVE(tlv);
		if (length < tlv_size) {
			log_warnx("lldp", "frame too short for tlv received on %s",
			    hardware->h_ifname);
			goto malformed;
		}
		switch (tlv_type) {
		case LLDP_TLV_END:
			if (tlv_size != 0) {
				log_warnx("lldp", "lldp end received with size not null on %s",
				    hardware->h_ifname);
				goto malformed;
			}
			if (length)
				log_debug("lldp", "extra data after lldp end on %s",
				    hardware->h_ifname);
			gotend = 1;
			break;
		case LLDP_TLV_CHASSIS_ID:
		case LLDP_TLV_PORT_ID:
			CHECK_TLV_SIZE(2, "Port Id");
			tlv_subtype = PEEK_UINT8;
			if ((tlv_subtype == 0) || (tlv_subtype > 7)) {
				log_warnx("lldp", "unknown subtype for tlv id received on %s",
				    hardware->h_ifname);
				goto malformed;
			}
			if ((b = (char *)calloc(1, tlv_size - 1)) == NULL) {
				log_warn("lldp", "unable to allocate memory for id tlv "
				    "received on %s",
				    hardware->h_ifname);
				goto malformed;
			}
			PEEK_BYTES(b, tlv_size - 1);
			if (tlv_type == LLDP_TLV_PORT_ID) {
				port->p_id_subtype = tlv_subtype;
				port->p_id = b;
				port->p_id_len = tlv_size - 1;
			} else {
				chassis->c_id_subtype = tlv_subtype;
				chassis->c_id = b;
				chassis->c_id_len = tlv_size - 1;
			}
			break;
		case LLDP_TLV_TTL:
			CHECK_TLV_SIZE(2, "TTL");
			port->p_ttl = PEEK_UINT16;
			ttl_received = 1;
			break;
		case LLDP_TLV_PORT_DESCR:
		case LLDP_TLV_SYSTEM_NAME:
		case LLDP_TLV_SYSTEM_DESCR:
			if (tlv_size < 1) {
				log_debug("lldp", "empty tlv received on %s",
				    hardware->h_ifname);
				break;
			}
			if ((b = (char *)calloc(1, tlv_size + 1)) == NULL) {
				log_warn("lldp", "unable to allocate memory for string tlv "
				    "received on %s",
				    hardware->h_ifname);
				goto malformed;
			}
			PEEK_BYTES(b, tlv_size);
			if (tlv_type == LLDP_TLV_PORT_DESCR)
				port->p_descr = b;
			else if (tlv_type == LLDP_TLV_SYSTEM_NAME)
				chassis->c_name = b;
			else chassis->c_descr = b;
			break;
		case LLDP_TLV_SYSTEM_CAP:
			CHECK_TLV_SIZE(4, "System capabilities");
			chassis->c_cap_available = PEEK_UINT16;
			chassis->c_cap_enabled = PEEK_UINT16;
			break;
		case LLDP_TLV_MGMT_ADDR:
			CHECK_TLV_SIZE(1, "Management address");
			addr_str_length = PEEK_UINT8;
			if (addr_str_length > sizeof(addr_str_buffer)) {
				log_warnx("lldp", "too large management address on %s",
				    hardware->h_ifname);
				goto malformed;
			}
			CHECK_TLV_SIZE(1 + addr_str_length, "Management address");
			PEEK_BYTES(addr_str_buffer, addr_str_length);
			addr_length = addr_str_length - 1;
			addr_family = addr_str_buffer[0];
			addr_ptr = &addr_str_buffer[1];
			CHECK_TLV_SIZE(1 + addr_str_length + 5, "Management address");
			iface_subtype = PEEK_UINT8;
			iface_number = PEEK_UINT32;

			af = lldpd_af_from_lldp_proto(addr_family);
			if (af == LLDPD_AF_UNSPEC)
				break;
			if (iface_subtype == LLDP_MGMT_IFACE_IFINDEX)
				iface = iface_number;
			else
				iface = 0;
			mgmt = lldpd_alloc_mgmt(af, addr_ptr, addr_length, iface);
			if (mgmt == NULL) {
				if (errno == ENOMEM)
					log_warn("lldp", "unable to allocate memory "
					    "for management address");
				else
					log_warn("lldp", "too large management address "
					    "received on %s", hardware->h_ifname);
				goto malformed;
			}
			TAILQ_INSERT_TAIL(&chassis->c_mgmt, mgmt, m_entries);
			break;
		case LLDP_TLV_ORG:
			CHECK_TLV_SIZE(1 + (int)sizeof(orgid), "Organisational");
			PEEK_BYTES(orgid, sizeof(orgid));
			tlv_subtype = PEEK_UINT8;
			if (memcmp(dot1, orgid, sizeof(orgid)) == 0) {
#ifndef ENABLE_DOT1
				hardware->h_rx_unrecognized_cnt++;
#else
				/* Dot1 */
				switch (tlv_subtype) {
				case LLDP_TLV_DOT1_VLANNAME:
					CHECK_TLV_SIZE(7, "VLAN");
					if ((vlan = (struct lldpd_vlan *)calloc(1,
						    sizeof(struct lldpd_vlan))) == NULL) {
						log_warn("lldp", "unable to alloc vlan "
						    "structure for "
						    "tlv received on %s",
						    hardware->h_ifname);
						goto malformed;
					}
					vlan->v_vid = PEEK_UINT16;
					vlan_len = PEEK_UINT8;
					CHECK_TLV_SIZE(7 + vlan_len, "VLAN");
					if ((vlan->v_name =
						(char *)calloc(1, vlan_len + 1)) == NULL) {
						log_warn("lldp", "unable to alloc vlan name for "
						    "tlv received on %s",
						    hardware->h_ifname);
						goto malformed;
					}
					PEEK_BYTES(vlan->v_name, vlan_len);
					TAILQ_INSERT_TAIL(&port->p_vlans,
					    vlan, v_entries);
					vlan = NULL;
					break;
				case LLDP_TLV_DOT1_PVID:
					CHECK_TLV_SIZE(6, "PVID");
					port->p_pvid = PEEK_UINT16;
					break;
				case LLDP_TLV_DOT1_PPVID:
					CHECK_TLV_SIZE(7, "PPVID");
					/* validation needed */
					/* PPVID has to be unique if more than
					   one PPVID TLVs are received  - 
					   discard if duplicate */
					/* if support bit is not set and 
					   enabled bit is set - PPVID TLV is
					   considered error  and discarded */
					/* if PPVID > 4096 - bad and discard */
					if ((ppvid = (struct lldpd_ppvid *)calloc(1,
						    sizeof(struct lldpd_ppvid))) == NULL) {
						log_warn("lldp", "unable to alloc ppvid "
						    "structure for "
						    "tlv received on %s",
						    hardware->h_ifname);
						goto malformed;
					}
					ppvid->p_cap_status = PEEK_UINT8;
					ppvid->p_ppvid = PEEK_UINT16;	
					TAILQ_INSERT_TAIL(&port->p_ppvids,
					    ppvid, p_entries);
					break;
				case LLDP_TLV_DOT1_PI:
					/* validation needed */
					/* PI has to be unique if more than 
					   one PI TLVs are received  - discard
					   if duplicate ?? */
					CHECK_TLV_SIZE(5, "PI");
					if ((pi = (struct lldpd_pi *)calloc(1,
						    sizeof(struct lldpd_pi))) == NULL) {
						log_warn("lldp", "unable to alloc PI "
						    "structure for "
						    "tlv received on %s",
						    hardware->h_ifname);
						goto malformed;
					}
					pi->p_pi_len = PEEK_UINT8;
					CHECK_TLV_SIZE(5 + pi->p_pi_len, "PI");
					if ((pi->p_pi =
						(char *)calloc(1, pi->p_pi_len)) == NULL) {
						log_warn("lldp", "unable to alloc pid name for "
						    "tlv received on %s",
						    hardware->h_ifname);
						goto malformed;
					}
					PEEK_BYTES(pi->p_pi, pi->p_pi_len);
					TAILQ_INSERT_TAIL(&port->p_pids,
					    pi, p_entries);
					pi = NULL;
					break;
				default:
					/* Unknown Dot1 TLV, ignore it */
					hardware->h_rx_unrecognized_cnt++;
				}
#endif
			} else if (memcmp(dot3, orgid, sizeof(orgid)) == 0) {
#ifndef ENABLE_DOT3
				hardware->h_rx_unrecognized_cnt++;
#else
				/* Dot3 */
				switch (tlv_subtype) {
				case LLDP_TLV_DOT3_MAC:
					CHECK_TLV_SIZE(9, "MAC/PHY");
					port->p_macphy.autoneg_support = PEEK_UINT8;
					port->p_macphy.autoneg_enabled =
					    (port->p_macphy.autoneg_support & 0x2) >> 1;
					port->p_macphy.autoneg_support =
					    port->p_macphy.autoneg_support & 0x1;
					port->p_macphy.autoneg_advertised =
					    PEEK_UINT16;
					port->p_macphy.mau_type = PEEK_UINT16;
					break;
				case LLDP_TLV_DOT3_LA:
					CHECK_TLV_SIZE(9, "Link aggregation");
					PEEK_DISCARD_UINT8;
					port->p_aggregid = PEEK_UINT32;
					break;
				case LLDP_TLV_DOT3_MFS:
					CHECK_TLV_SIZE(6, "MFS");
					port->p_mfs = PEEK_UINT16;
					break;
				case LLDP_TLV_DOT3_POWER:
					CHECK_TLV_SIZE(7, "Power");
					port->p_power.devicetype = PEEK_UINT8;
					port->p_power.supported =
						(port->p_power.devicetype & 0x2) >> 1;
					port->p_power.enabled =
						(port->p_power.devicetype & 0x4) >> 2;
					port->p_power.paircontrol =
						(port->p_power.devicetype & 0x8) >> 3;
					port->p_power.devicetype =
						(port->p_power.devicetype & 0x1)?
						LLDP_DOT3_POWER_PSE:LLDP_DOT3_POWER_PD;
					port->p_power.pairs = PEEK_UINT8;
					port->p_power.class = PEEK_UINT8;
					/* 802.3at? */
					if (tlv_size >= 12) {
						port->p_power.powertype = PEEK_UINT8;
						port->p_power.source =
						    (port->p_power.powertype & (1<<5 | 1<<4)) >> 4;
						port->p_power.priority =
						    (port->p_power.powertype & (1<<1 | 1<<0));
						port->p_power.powertype =
						    (port->p_power.powertype & (1<<7))?
						    LLDP_DOT3_POWER_8023AT_TYPE1:
						    LLDP_DOT3_POWER_8023AT_TYPE2;
						port->p_power.requested = PEEK_UINT16;
						port->p_power.allocated = PEEK_UINT16;
					} else
						port->p_power.powertype =
						    LLDP_DOT3_POWER_8023AT_OFF;
					break;
				default:
					/* Unknown Dot3 TLV, ignore it */
					hardware->h_rx_unrecognized_cnt++;
				}
#endif
			} else if (memcmp(med, orgid, sizeof(orgid)) == 0) {
				/* LLDP-MED */
#ifndef ENABLE_LLDPMED
				hardware->h_rx_unrecognized_cnt++;
#else
				u_int32_t policy;
				unsigned loctype;
				unsigned power;

				switch (tlv_subtype) {
				case LLDP_TLV_MED_CAP:
					CHECK_TLV_SIZE(7, "LLDP-MED capabilities");
					chassis->c_med_cap_available = PEEK_UINT16;
					chassis->c_med_type = PEEK_UINT8;
					port->p_med_cap_enabled |=
					    LLDP_MED_CAP_CAP;
					break;
				case LLDP_TLV_MED_POLICY:
					CHECK_TLV_SIZE(8, "LLDP-MED policy");
					policy = PEEK_UINT32;
					if (((policy >> 24) < 1) ||
					    ((policy >> 24) > LLDP_MED_APPTYPE_LAST)) {
						log_info("lldp", "unknown policy field %d "
						    "received on %s",
						    policy,
						    hardware->h_ifname);
						break;
					}
					port->p_med_policy[(policy >> 24) - 1].type =
					    (policy >> 24);
					port->p_med_policy[(policy >> 24) - 1].unknown =
					    ((policy & 0x800000) != 0);
					port->p_med_policy[(policy >> 24) - 1].tagged =
					    ((policy & 0x400000) != 0);
					port->p_med_policy[(policy >> 24) - 1].vid =
					    (policy & 0x001FFE00) >> 9;
					port->p_med_policy[(policy >> 24) - 1].priority =
					    (policy & 0x1C0) >> 6;
					port->p_med_policy[(policy >> 24) - 1].dscp =
					    policy & 0x3F;
					port->p_med_cap_enabled |=
					    LLDP_MED_CAP_POLICY;
					break;
				case LLDP_TLV_MED_LOCATION:
					CHECK_TLV_SIZE(5, "LLDP-MED Location");
					loctype = PEEK_UINT8;
					if ((loctype < 1) ||
					    (loctype > LLDP_MED_LOCFORMAT_LAST)) {
						log_info("lldp", "unknown location type "
						    "received on %s",
						    hardware->h_ifname);
						break;
					}
					if ((port->p_med_location[loctype - 1].data =
						(char*)malloc(tlv_size - 5)) == NULL) {
						log_warn("lldp", "unable to allocate memory "
						    "for LLDP-MED location for "
						    "frame received on %s",
						    hardware->h_ifname);
						goto malformed;
					}
					PEEK_BYTES(port->p_med_location[loctype - 1].data,
					    tlv_size - 5);
					port->p_med_location[loctype - 1].data_len =
					    tlv_size - 5;
					port->p_med_location[loctype - 1].format = loctype;
					port->p_med_cap_enabled |=
					    LLDP_MED_CAP_LOCATION;
					break;
				case LLDP_TLV_MED_MDI:
					CHECK_TLV_SIZE(7, "LLDP-MED PoE-MDI");
					power = PEEK_UINT8;
					switch (power & 0xC0) {
					case 0x0:
						port->p_med_power.devicetype = LLDP_MED_POW_TYPE_PSE;
						port->p_med_cap_enabled |=
						    LLDP_MED_CAP_MDI_PSE;
						switch (power & 0x30) {
						case 0x0:
							port->p_med_power.source =
							    LLDP_MED_POW_SOURCE_UNKNOWN;
							break;
						case 0x10:
							port->p_med_power.source =
							    LLDP_MED_POW_SOURCE_PRIMARY;
							break;
						case 0x20:
							port->p_med_power.source =
							    LLDP_MED_POW_SOURCE_BACKUP;
							break;
						default:
							port->p_med_power.source =
							    LLDP_MED_POW_SOURCE_RESERVED;
						}
						break;
					case 0x40:
						port->p_med_power.devicetype = LLDP_MED_POW_TYPE_PD;
						port->p_med_cap_enabled |=
						    LLDP_MED_CAP_MDI_PD;
						switch (power & 0x30) {
						case 0x0:
							port->p_med_power.source =
							    LLDP_MED_POW_SOURCE_UNKNOWN;
							break;
						case 0x10:
							port->p_med_power.source =
							    LLDP_MED_POW_SOURCE_PSE;
							break;
						case 0x20:
							port->p_med_power.source =
							    LLDP_MED_POW_SOURCE_LOCAL;
							break;
						default:
							port->p_med_power.source =
							    LLDP_MED_POW_SOURCE_BOTH;
						}
						break;
					default:
						port->p_med_power.devicetype =
						    LLDP_MED_POW_TYPE_RESERVED;
					}
					if ((power & 0x0F) > LLDP_MED_POW_PRIO_LOW)
						port->p_med_power.priority =
						    LLDP_MED_POW_PRIO_UNKNOWN;
					else
						port->p_med_power.priority =
						    power & 0x0F;
					port->p_med_power.val = PEEK_UINT16;
					break;
				case LLDP_TLV_MED_IV_HW:
				case LLDP_TLV_MED_IV_SW:
				case LLDP_TLV_MED_IV_FW:
				case LLDP_TLV_MED_IV_SN:
				case LLDP_TLV_MED_IV_MANUF:
				case LLDP_TLV_MED_IV_MODEL:
				case LLDP_TLV_MED_IV_ASSET:
					if (tlv_size <= 4)
						b = NULL;
					else {
						if ((b = (char*)malloc(tlv_size - 3)) ==
						    NULL) {
							log_warn("lldp", "unable to allocate "
							    "memory for LLDP-MED "
							    "inventory for frame "
							    "received on %s",
							    hardware->h_ifname);
							goto malformed;
						}
						PEEK_BYTES(b, tlv_size - 4);
						b[tlv_size - 4] = '\0';
					}
					switch (tlv_subtype) {
					case LLDP_TLV_MED_IV_HW:
						chassis->c_med_hw = b;
						break;
					case LLDP_TLV_MED_IV_FW:
						chassis->c_med_fw = b;
						break;
					case LLDP_TLV_MED_IV_SW:
						chassis->c_med_sw = b;
						break;
					case LLDP_TLV_MED_IV_SN:
						chassis->c_med_sn = b;
						break;
					case LLDP_TLV_MED_IV_MANUF:
						chassis->c_med_manuf = b;
						break;
					case LLDP_TLV_MED_IV_MODEL:
						chassis->c_med_model = b;
						break;
					case LLDP_TLV_MED_IV_ASSET:
						chassis->c_med_asset = b;
						break;
					}
					port->p_med_cap_enabled |=
					    LLDP_MED_CAP_IV;
					break;
				default:
					/* Unknown LLDP MED, ignore it */
					hardware->h_rx_unrecognized_cnt++;
				}
#endif /* ENABLE_LLDPMED */
			} else if (memcmp(dcbx, orgid, sizeof(orgid)) == 0) {
				log_debug("lldp", "unsupported DCBX tlv received on %s - ignore",
				    hardware->h_ifname);
				hardware->h_rx_unrecognized_cnt++;
			} else {
				log_debug("lldp", "unknown org tlv [%02x:%02x:%02x] received on %s",
				    orgid[0], orgid[1], orgid[2],
				    hardware->h_ifname);
				hardware->h_rx_unrecognized_cnt++;
#ifdef ENABLE_CUSTOM
				custom = (struct lldpd_custom*)calloc(1, sizeof(struct lldpd_custom));
				if (!custom) {
					log_warn("lldp",
					    "unable to allocate memory for custom TLV");
					goto malformed;
				}
				custom->oui_info_len = tlv_size > 4 ? tlv_size - 4 : 0;
				memcpy(custom->oui, orgid, sizeof(custom->oui));
				custom->subtype = tlv_subtype;
				if (custom->oui_info_len > 0) {
					custom->oui_info = malloc(custom->oui_info_len);
					if (!custom->oui_info) {
						log_warn("lldp",
						    "unable to allocate memory for custom TLV data");
						goto malformed;
					}
					PEEK_BYTES(custom->oui_info, custom->oui_info_len);
				}
				TAILQ_INSERT_TAIL(&port->p_custom_list, custom, next);
				custom = NULL;
#endif
			}
			break;
		default:
			log_warnx("lldp", "unknown tlv (%d) received on %s",
			    tlv_type, hardware->h_ifname);
			goto malformed;
		}
		if (pos > tlv + tlv_size) {
			log_warnx("lldp", "BUG: already past TLV!");
			goto malformed;
		}
		PEEK_DISCARD(tlv + tlv_size - pos);
	}

	/* Some random check */
	if ((chassis->c_id == NULL) ||
	    (port->p_id == NULL) ||
	    (!ttl_received) ||
	    (gotend == 0)) {
		log_warnx("lldp", "some mandatory tlv are missing for frame received on %s",
		    hardware->h_ifname);
		goto malformed;
	}
	*newchassis = chassis;
	*newport = port;
	return 1;
malformed:
#ifdef ENABLE_CUSTOM
	free(custom);
#endif
#ifdef ENABLE_DOT1
	free(vlan);
	free(pi);
#endif
	lldpd_chassis_cleanup(chassis, 1);
	lldpd_port_cleanup(port, 1);
	free(port);
	return -1;
}
