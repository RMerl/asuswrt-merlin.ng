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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <string.h>

#include "../log.h"
#include "client.h"

static void
display_cap(struct writer * w, lldpctl_atom_t *chassis, u_int8_t bit, char *symbol)
{
	if (lldpctl_atom_get_int(chassis, lldpctl_k_chassis_cap_available) & bit) {
		tag_start(w, "capability", "Capability");
		tag_attr (w, "type", "", symbol );
		tag_attr (w, "enabled", "",
		    (lldpctl_atom_get_int(chassis, lldpctl_k_chassis_cap_enabled) & bit)?
		    "on":"off");
		tag_end  (w);
	}
}

static void
display_med_capability(struct writer *w, long int available, int cap,
    const char *symbol)
{
	if (available & cap) {
		tag_start(w, "capability", "Capability");
		tag_attr(w, "type", "", symbol);
		tag_attr(w, "available", "", "yes");
		tag_end(w);
	}
}

static void
display_med(struct writer *w, lldpctl_atom_t *port, lldpctl_atom_t *chassis)
{
	lldpctl_atom_t *medpolicies, *medpolicy;
	lldpctl_atom_t *medlocations, *medlocation;
	lldpctl_atom_t *caelements, *caelement;
	lldpctl_atom_t *medpower;
	long int cap = lldpctl_atom_get_int(chassis, lldpctl_k_chassis_med_cap);
	const char *type;

	if (lldpctl_atom_get_int(chassis, lldpctl_k_chassis_med_type) <= 0)
		return;

	tag_start(w, "lldp-med", "LLDP-MED");

	tag_datatag(w, "device-type", "Device Type",
	    lldpctl_atom_get_str(chassis, lldpctl_k_chassis_med_type));

	display_med_capability(w, cap, LLDP_MED_CAP_CAP, "Capabilities");
	display_med_capability(w, cap, LLDP_MED_CAP_POLICY, "Policy");
	display_med_capability(w, cap, LLDP_MED_CAP_LOCATION, "Location");
	display_med_capability(w, cap, LLDP_MED_CAP_MDI_PSE, "MDI/PSE");
	display_med_capability(w, cap, LLDP_MED_CAP_MDI_PD, "MDI/PD");
	display_med_capability(w, cap, LLDP_MED_CAP_IV, "Inventory");

	/* LLDP MED policies */
	medpolicies = lldpctl_atom_get(port, lldpctl_k_port_med_policies);
	lldpctl_atom_foreach(medpolicies, medpolicy) {
		if (lldpctl_atom_get_int(medpolicy,
			lldpctl_k_med_policy_type) <= 0) continue;

		tag_start(w, "policy", "LLDP-MED Network Policy for");
		tag_attr(w, "apptype", "", lldpctl_atom_get_str(medpolicy, lldpctl_k_med_policy_type));
		tag_attr(w, "defined", "Defined",
		    (lldpctl_atom_get_int(medpolicy,
			lldpctl_k_med_policy_unknown) > 0)?"no":"yes");

		if (lldpctl_atom_get_int(medpolicy,
			lldpctl_k_med_policy_tagged) > 0) {
			int vid = lldpctl_atom_get_int(medpolicy,
			    lldpctl_k_med_policy_vid);
			tag_start(w, "vlan", "VLAN");
			if (vid == 0) {
				tag_attr(w, "vid", "", "priority");
			} else if (vid == 4095) {
				tag_attr(w, "vid", "", "reserved");
			} else {
				tag_attr(w, "vid", "",
				    lldpctl_atom_get_str(medpolicy,
					lldpctl_k_med_policy_vid));
			}
			tag_end(w);
		}

		tag_datatag(w, "priority", "Priority",
		    lldpctl_atom_get_str(medpolicy,
			lldpctl_k_med_policy_priority));
		/* Also give a numeric value */
		int pcp = lldpctl_atom_get_int(medpolicy,
		    lldpctl_k_med_policy_priority);
		char spcp[2] = { pcp + '0', '\0' };
		tag_datatag(w, "pcp", "PCP", spcp);
		tag_datatag(w, "dscp", "DSCP Value",
		    lldpctl_atom_get_str(medpolicy,
			lldpctl_k_med_policy_dscp));

		tag_end(w);
	}
	lldpctl_atom_dec_ref(medpolicies);

	/* LLDP MED locations */
	medlocations = lldpctl_atom_get(port, lldpctl_k_port_med_locations);
	lldpctl_atom_foreach(medlocations, medlocation) {
		int format = lldpctl_atom_get_int(medlocation,
		    lldpctl_k_med_location_format);
		if (format <= 0) continue;
		tag_start(w, "location", "LLDP-MED Location Identification");
		tag_attr(w, "type", "Type",
		    lldpctl_atom_get_str(medlocation,
			lldpctl_k_med_location_format));

		switch (format) {
		case LLDP_MED_LOCFORMAT_COORD:
			tag_attr(w, "geoid", "Geoid",
			    lldpctl_atom_get_str(medlocation,
				lldpctl_k_med_location_geoid));
			tag_datatag(w, "lat", "Latitude",
			    lldpctl_atom_get_str(medlocation,
				lldpctl_k_med_location_latitude));
			tag_datatag(w, "lon", "Longitude",
			    lldpctl_atom_get_str(medlocation,
				lldpctl_k_med_location_longitude));
			tag_start(w, "altitude", "Altitude");
			tag_attr(w, "unit", "", lldpctl_atom_get_str(medlocation,
				lldpctl_k_med_location_altitude_unit));
			tag_data(w, lldpctl_atom_get_str(medlocation,
				lldpctl_k_med_location_altitude));
			tag_end(w);
			break;
		case LLDP_MED_LOCFORMAT_CIVIC:
			tag_datatag(w, "country", "Country",
			    lldpctl_atom_get_str(medlocation,
				lldpctl_k_med_location_country));
			caelements = lldpctl_atom_get(medlocation,
			    lldpctl_k_med_location_ca_elements);
			lldpctl_atom_foreach(caelements, caelement) {
				type = lldpctl_atom_get_str(caelement,
				    lldpctl_k_med_civicaddress_type);
				tag_datatag(w, totag(type), type,
				    lldpctl_atom_get_str(caelement,
					lldpctl_k_med_civicaddress_value));
			}
			lldpctl_atom_dec_ref(caelements);
			break;
		case LLDP_MED_LOCFORMAT_ELIN:
			tag_datatag(w, "ecs", "ECS ELIN",
			    lldpctl_atom_get_str(medlocation,
				lldpctl_k_med_location_elin));
			break;
		}

		tag_end(w);
	}
	lldpctl_atom_dec_ref(medlocations);

	/* LLDP MED power */
	medpower = lldpctl_atom_get(port, lldpctl_k_port_med_power);
	if (lldpctl_atom_get_int(medpower, lldpctl_k_med_power_type) > 0) {
 		tag_start(w, "poe", "Extended Power-over-Ethernet");

		tag_datatag(w, "device-type", "Power Type & Source",
		    lldpctl_atom_get_str(medpower, lldpctl_k_med_power_type));
		tag_datatag(w, "source", "Power Source",
		    lldpctl_atom_get_str(medpower, lldpctl_k_med_power_source));
		tag_datatag(w, "priority", "Power priority",
		    lldpctl_atom_get_str(medpower, lldpctl_k_med_power_priority));
		tag_datatag(w, "power", "Power Value",
		    lldpctl_atom_get_str(medpower, lldpctl_k_med_power_val));

		tag_end(w);
	}
	lldpctl_atom_dec_ref(medpower);

	/* LLDP MED inventory */
	do {
		const char *hw = lldpctl_atom_get_str(chassis,
		    lldpctl_k_chassis_med_inventory_hw);
		const char *sw = lldpctl_atom_get_str(chassis,
		    lldpctl_k_chassis_med_inventory_sw);
		const char *fw = lldpctl_atom_get_str(chassis,
		    lldpctl_k_chassis_med_inventory_fw);
		const char *sn = lldpctl_atom_get_str(chassis,
		    lldpctl_k_chassis_med_inventory_sn);
		const char *manuf = lldpctl_atom_get_str(chassis,
		    lldpctl_k_chassis_med_inventory_manuf);
		const char *model = lldpctl_atom_get_str(chassis,
		    lldpctl_k_chassis_med_inventory_model);
		const char *asset = lldpctl_atom_get_str(chassis,
		    lldpctl_k_chassis_med_inventory_asset);
		if (!(hw || sw || fw || sn ||
			manuf || model || asset)) break;

		tag_start(w, "inventory", "Inventory");
		tag_datatag(w, "hardware", "Hardware Revision", hw);
		tag_datatag(w, "software", "Software Revision", sw);
		tag_datatag(w, "firmware", "Firmware Revision", fw);
		tag_datatag(w, "serial", "Serial Number", sn);
		tag_datatag(w, "manufacturer", "Manufacturer", manuf);
		tag_datatag(w, "model", "Model", model);
		tag_datatag(w, "asset", "Asset ID", asset);
		tag_end(w);
	} while(0);

	tag_end(w);
}

static void
display_chassis(struct writer* w, lldpctl_atom_t* chassis, int details)
{
	lldpctl_atom_t *mgmts, *mgmt;

	tag_start(w, "chassis", "Chassis");
	tag_start(w, "id", "ChassisID");
	tag_attr (w, "type", "",
	    lldpctl_atom_get_str(chassis,
		lldpctl_k_chassis_id_subtype));
	tag_data(w, lldpctl_atom_get_str(chassis,
		lldpctl_k_chassis_id));
	tag_end(w);
	tag_datatag(w, "name", "SysName",
	    lldpctl_atom_get_str(chassis, lldpctl_k_chassis_name));
	if (details == DISPLAY_BRIEF) {
		tag_end(w);
		return;
	}
	tag_datatag(w, "descr", "SysDescr",
	    lldpctl_atom_get_str(chassis, lldpctl_k_chassis_descr));

	/* Management addresses */
	mgmts = lldpctl_atom_get(chassis, lldpctl_k_chassis_mgmt);
	lldpctl_atom_foreach(mgmts, mgmt) {
		tag_datatag(w, "mgmt-ip", "MgmtIP",
		    lldpctl_atom_get_str(mgmt, lldpctl_k_mgmt_ip));
	}
	lldpctl_atom_dec_ref(mgmts);

	/* Capabilities */
	display_cap(w, chassis, LLDP_CAP_OTHER, "Other");
	display_cap(w, chassis, LLDP_CAP_REPEATER, "Repeater");
	display_cap(w, chassis, LLDP_CAP_BRIDGE, "Bridge");
	display_cap(w, chassis, LLDP_CAP_ROUTER, "Router");
	display_cap(w, chassis, LLDP_CAP_WLAN, "Wlan");
	display_cap(w, chassis, LLDP_CAP_TELEPHONE, "Tel");
	display_cap(w, chassis, LLDP_CAP_DOCSIS, "Docsis");
	display_cap(w, chassis, LLDP_CAP_STATION, "Station");

	tag_end(w);
}

static void
display_custom_tlvs(struct writer* w, lldpctl_atom_t* neighbor)
{
	lldpctl_atom_t *custom_list, *custom;
	int have_custom_tlvs = 0;
	size_t i, len, slen;
	const uint8_t *oui, *oui_info;
	char buf[1600]; /* should be enough for printing */

	custom_list = lldpctl_atom_get(neighbor, lldpctl_k_custom_tlvs);
	lldpctl_atom_foreach(custom_list, custom) {
		/* This tag gets added only once, if there are any custom TLVs */
		if (!have_custom_tlvs) {
			tag_start(w, "unknown-tlvs", "Unknown TLVs");
			have_custom_tlvs++;
		}
		len = 0;
		oui = lldpctl_atom_get_buffer(custom, lldpctl_k_custom_tlv_oui, &len);
		len = 0;
		oui_info = lldpctl_atom_get_buffer(custom, lldpctl_k_custom_tlv_oui_info_string, &len);
		if (!oui)
			continue;
		tag_start(w, "unknown-tlv", "TLV");

		/* Add OUI as attribute */
		snprintf(buf, sizeof(buf), "%02X,%02X,%02X", oui[0], oui[1], oui[2]);
		tag_attr(w, "oui", "OUI", buf);
		snprintf(buf, sizeof(buf), "%d",
		         (int)lldpctl_atom_get_int(custom, lldpctl_k_custom_tlv_oui_subtype));
		tag_attr(w, "subtype", "SubType", buf);
		snprintf(buf, sizeof(buf), "%d", (int)len);
		tag_attr(w, "len", "Len", buf);
		if (len > 0) {
			for (slen=0, i=0; i < len; ++i)
				slen += snprintf(buf + slen, sizeof(buf) > slen ? sizeof(buf) - slen : 0, 
				                 "%02X%s", oui_info[i], ((i < len - 1) ? "," : ""));
			tag_data(w, buf);
		}
		tag_end(w);
	}
	lldpctl_atom_dec_ref(custom_list);

	if (have_custom_tlvs)
		tag_end(w);
}


static void
display_autoneg(struct writer * w, int advertised, int bithd, int bitfd, char *desc)
{
	if (!((advertised & bithd) ||
		(advertised & bitfd)))
		return;

	tag_start(w, "advertised", "Adv");
	tag_attr(w, "type", "", desc);
	if (bitfd != bithd) {
		tag_attr(w, "hd", "HD", (advertised & bithd)?"yes":"no");
		tag_attr(w, "fd", "FD", (advertised & bitfd)?"yes":"no");
	}
	tag_end (w);
}

static void
display_port(struct writer *w, lldpctl_atom_t *port, int details)
{
	tag_start(w, "port", "Port");
	tag_start(w, "id", "PortID");
	tag_attr (w, "type", "",
	    lldpctl_atom_get_str(port, lldpctl_k_port_id_subtype));
	tag_data(w, lldpctl_atom_get_str(port, lldpctl_k_port_id));
	tag_end(w);

	tag_datatag(w, "descr", "PortDescr",
	    lldpctl_atom_get_str(port, lldpctl_k_port_descr));
	if (details)
		tag_datatag(w, "ttl", "TTL",
		    lldpctl_atom_get_str(port, lldpctl_k_port_ttl));

	/* Dot3 */
	if (details == DISPLAY_DETAILS) {
		tag_datatag(w, "mfs", "MFS",
		    lldpctl_atom_get_str(port, lldpctl_k_port_dot3_mfs));
		tag_datatag(w, "aggregation", "Port is aggregated. PortAggregID",
		    lldpctl_atom_get_str(port, lldpctl_k_port_dot3_aggregid));

		long int autoneg_support, autoneg_enabled, autoneg_advertised, mautype;
		autoneg_support = lldpctl_atom_get_int(port,
		    lldpctl_k_port_dot3_autoneg_support);
		autoneg_enabled = lldpctl_atom_get_int(port,
		    lldpctl_k_port_dot3_autoneg_enabled);
		autoneg_advertised = lldpctl_atom_get_int(port,
		    lldpctl_k_port_dot3_autoneg_advertised);
		mautype = lldpctl_atom_get_int(port, lldpctl_k_port_dot3_mautype);
		if (autoneg_support > 0 || autoneg_enabled > 0 || mautype > 0) {
			tag_start(w, "auto-negotiation", "PMD autoneg");
			tag_attr (w, "supported", "supported",
			    (autoneg_support > 0)?"yes":"no");
			tag_attr (w, "enabled", "enabled",
			    (autoneg_enabled > 0)?"yes":"no");

			if (autoneg_enabled > 0) {
				if (autoneg_advertised < 0)
					autoneg_advertised = 0;
				display_autoneg(w, autoneg_advertised,
				    LLDP_DOT3_LINK_AUTONEG_10BASE_T,
				    LLDP_DOT3_LINK_AUTONEG_10BASET_FD,
				    "10Base-T");
				display_autoneg(w, autoneg_advertised,
				    LLDP_DOT3_LINK_AUTONEG_100BASE_TX,
				    LLDP_DOT3_LINK_AUTONEG_100BASE_TXFD,
				    "100Base-TX");
				display_autoneg(w, autoneg_advertised,
				    LLDP_DOT3_LINK_AUTONEG_100BASE_T2,
				    LLDP_DOT3_LINK_AUTONEG_100BASE_T2FD,
				    "100Base-T2");
				display_autoneg(w, autoneg_advertised,
				    LLDP_DOT3_LINK_AUTONEG_100BASE_T4,
				    LLDP_DOT3_LINK_AUTONEG_100BASE_T4,
				    "100Base-T4");
				display_autoneg(w, autoneg_advertised,
				    LLDP_DOT3_LINK_AUTONEG_1000BASE_X,
				    LLDP_DOT3_LINK_AUTONEG_1000BASE_XFD,
				    "1000Base-X");
				display_autoneg(w, autoneg_advertised,
				    LLDP_DOT3_LINK_AUTONEG_1000BASE_T,
				    LLDP_DOT3_LINK_AUTONEG_1000BASE_TFD,
				    "1000Base-T");
			}
			tag_datatag(w, "current", "MAU oper type",
			    lldpctl_atom_get_str(port, lldpctl_k_port_dot3_mautype));
			tag_end(w);
		}

		lldpctl_atom_t *dot3_power = lldpctl_atom_get(port,
		    lldpctl_k_port_dot3_power);
		int devicetype = lldpctl_atom_get_int(dot3_power,
		    lldpctl_k_dot3_power_devicetype);
		if (devicetype > 0) {
			tag_start(w, "power", "MDI Power");
			tag_attr(w, "supported", "supported",
			    (lldpctl_atom_get_int(dot3_power,
				lldpctl_k_dot3_power_supported) > 0)?"yes":"no");
			tag_attr(w, "enabled", "enabled",
			    (lldpctl_atom_get_int(dot3_power,
				lldpctl_k_dot3_power_enabled) > 0)?"yes":"no");
			tag_attr(w, "paircontrol", "pair control",
			    (lldpctl_atom_get_int(dot3_power,
				lldpctl_k_dot3_power_paircontrol) > 0)?"yes":"no");
			tag_start(w, "device-type", "Device type");
			tag_data(w, lldpctl_atom_get_str(dot3_power,
				lldpctl_k_dot3_power_devicetype));;
			tag_end(w);
			tag_start(w, "pairs", "Power pairs");
			tag_data(w, lldpctl_atom_get_str(dot3_power,
				lldpctl_k_dot3_power_pairs));
			tag_end(w);
			tag_start(w, "class", "Class");
			tag_data(w, lldpctl_atom_get_str(dot3_power,
				lldpctl_k_dot3_power_class));
			tag_end(w);

			/* 802.3at */
			if (lldpctl_atom_get_int(dot3_power,
				lldpctl_k_dot3_power_type) > LLDP_DOT3_POWER_8023AT_OFF) {
				tag_start(w, "power-type", "Power type");
				tag_data(w, lldpctl_atom_get_str(dot3_power,
					lldpctl_k_dot3_power_type));
				tag_end(w);

				tag_start(w, "source", "Power Source");
				tag_data(w, lldpctl_atom_get_str(dot3_power,
					lldpctl_k_dot3_power_source));
				tag_end(w);

				tag_start(w, "priority", "Power Priority");
				tag_data(w, lldpctl_atom_get_str(dot3_power,
					lldpctl_k_dot3_power_priority));
				tag_end(w);

				tag_start(w, "requested", "PD requested power Value");
				tag_data(w, lldpctl_atom_get_str(dot3_power,
					lldpctl_k_dot3_power_requested));
				tag_end(w);

				tag_start(w, "allocated", "PSE allocated power Value");
				tag_data(w, lldpctl_atom_get_str(dot3_power,
					lldpctl_k_dot3_power_allocated));
				tag_end(w);
			}

			tag_end(w);
		}
		lldpctl_atom_dec_ref(dot3_power);
	}

	tag_end(w);
}

static void
display_vlans(struct writer *w, lldpctl_atom_t *port)
{
	lldpctl_atom_t *vlans, *vlan;
	int foundpvid = 0;
	int pvid, vid;

	pvid = lldpctl_atom_get_int(port,
	    lldpctl_k_port_vlan_pvid);

	vlans = lldpctl_atom_get(port, lldpctl_k_port_vlans);
	lldpctl_atom_foreach(vlans, vlan) {
		vid = lldpctl_atom_get_int(vlan,
		    lldpctl_k_vlan_id);
		if (pvid == vid)
			foundpvid = 1;

		tag_start(w, "vlan", "VLAN");
		tag_attr(w, "vlan-id", "",
		    lldpctl_atom_get_str(vlan, lldpctl_k_vlan_id));
		if (pvid == vid)
			tag_attr(w, "pvid", "pvid", "yes");
		tag_data(w, lldpctl_atom_get_str(vlan,
			lldpctl_k_vlan_name));
		tag_end(w);
	}
	lldpctl_atom_dec_ref(vlans);

	if (!foundpvid && pvid > 0) {
		tag_start(w, "vlan", "VLAN");
		tag_attr(w, "vlan-id", "",
		    lldpctl_atom_get_str(port,
			lldpctl_k_port_vlan_pvid));
		tag_attr(w, "pvid", "pvid", "yes");
		tag_end(w);
	}
}

static void
display_ppvids(struct writer *w, lldpctl_atom_t *port)
{
	lldpctl_atom_t *ppvids, *ppvid;
	ppvids = lldpctl_atom_get(port, lldpctl_k_port_ppvids);
	lldpctl_atom_foreach(ppvids, ppvid) {
		int status = lldpctl_atom_get_int(ppvid,
		    lldpctl_k_ppvid_status);
		tag_start(w, "ppvid", "PPVID");
		if (lldpctl_atom_get_int(ppvid,
			lldpctl_k_ppvid_id) > 0)
			tag_attr(w, "value", "",
			    lldpctl_atom_get_str(ppvid,
				lldpctl_k_ppvid_id));
		tag_attr(w, "supported", "supported",
			 (status & LLDP_PPVID_CAP_SUPPORTED)?"yes":"no");
		tag_attr(w, "enabled", "enabled",
			 (status & LLDP_PPVID_CAP_ENABLED)?"yes":"no");
		tag_end(w);
	}
	lldpctl_atom_dec_ref(ppvids);
}

static void
display_pids(struct writer *w, lldpctl_atom_t *port)
{
	lldpctl_atom_t *pids, *pid;
	pids = lldpctl_atom_get(port, lldpctl_k_port_pis);
	lldpctl_atom_foreach(pids, pid) {
		const char *pi = lldpctl_atom_get_str(pid, lldpctl_k_pi_id);
		if (pi && strlen(pi) > 0)
			tag_datatag(w, "pi", "PI", pi);
	}
	lldpctl_atom_dec_ref(pids);
}

static const char*
display_age(time_t lastchange)
{
	static char sage[30];
	int age = (int)(time(NULL) - lastchange);
	if (snprintf(sage, sizeof(sage),
		"%d day%s, %02d:%02d:%02d",
		age / (60*60*24),
		(age / (60*60*24) > 1)?"s":"",
		(age / (60*60)) % 24,
		(age / 60) % 60,
		age % 60) >= sizeof(sage))
		return "too much";
	else
		return sage;
}

void
display_local_chassis(lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, int details)
{
	tag_start(w, "local-chassis", "Local chassis");

	lldpctl_atom_t *chassis = lldpctl_get_local_chassis(conn);
	display_chassis(w, chassis, details);
	if (details == DISPLAY_DETAILS) {
		display_med(w, NULL, chassis);
	}
	lldpctl_atom_dec_ref(chassis);

	tag_end(w);
}

void
display_interface(lldpctl_conn_t *conn, struct writer *w, int hidden,
    lldpctl_atom_t *iface, lldpctl_atom_t *neighbor, int details, int protocol)
{
	if (!hidden &&
	    lldpctl_atom_get_int(neighbor, lldpctl_k_port_hidden))
		return;

	/* user might have specified protocol to filter on display */
	if ((protocol != LLDPD_MODE_MAX) &&
	    (protocol != lldpctl_atom_get_int(neighbor, lldpctl_k_port_protocol)))
	    return;

	lldpctl_atom_t *chassis = lldpctl_atom_get(neighbor, lldpctl_k_port_chassis);

	tag_start(w, "interface", "Interface");
	tag_attr(w, "name", "",
	    lldpctl_atom_get_str(iface, lldpctl_k_interface_name));
	tag_attr(w, "via" , "via",
	    lldpctl_atom_get_str(neighbor, lldpctl_k_port_protocol));
	if (details > DISPLAY_BRIEF) {
		tag_attr(w, "rid" , "RID",
		    lldpctl_atom_get_str(chassis, lldpctl_k_chassis_index));
		tag_attr(w, "age" , "Time",
		    display_age(lldpctl_atom_get_int(neighbor, lldpctl_k_port_age)));
	}

	display_chassis(w, chassis, details);
	display_port(w, neighbor, details);
	if (details == DISPLAY_DETAILS) {
		display_vlans(w, neighbor);
		display_ppvids(w, neighbor);
		display_pids(w, neighbor);
		display_med(w, neighbor, chassis);
	}

	lldpctl_atom_dec_ref(chassis);

	display_custom_tlvs(w, neighbor);

	tag_end(w);
}

/**
 * Display information about interfaces.
 *
 * @param conn       Connection to lldpd.
 * @param w          Writer.
 * @param hidden     Whatever to show hidden ports.
 * @param env        Environment from which we may find the list of ports.
 * @param details    Level of details we need (DISPLAY_*).
 */
void
display_interfaces(lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env,
    int hidden, int details)
{
	lldpctl_atom_t *iface;
	int protocol = LLDPD_MODE_MAX;
	const char *proto_str;

	/* user might have specified protocol to filter display results */
	proto_str = cmdenv_get(env, "protocol");

	if (proto_str) {
		log_debug("display", "filter protocol: %s ", proto_str);

		protocol = 0;
		for (lldpctl_map_t *protocol_map =
			 lldpctl_key_get_map(lldpctl_k_port_protocol);
		     protocol_map->string;
		     protocol_map++) {
			if (!strcasecmp(proto_str, protocol_map->string)) {
				protocol = protocol_map->value;
				break;
			}
		}
	}

	tag_start(w, "lldp", "LLDP neighbors");
	while ((iface = cmd_iterate_on_interfaces(conn, env))) {
		lldpctl_atom_t *port;
		lldpctl_atom_t *neighbors;
		lldpctl_atom_t *neighbor;
		port      = lldpctl_get_port(iface);
		neighbors = lldpctl_atom_get(port, lldpctl_k_port_neighbors);
		lldpctl_atom_foreach(neighbors, neighbor) {
			display_interface(conn, w, hidden, iface, neighbor, details, protocol);
		}
		lldpctl_atom_dec_ref(neighbors);
		lldpctl_atom_dec_ref(port);
	}
	tag_end(w);
}

void
display_stat(struct writer *w, const char *tag, const char *descr,
	long unsigned int cnt)
{
	char buf[20] = {};

	tag_start(w, tag, descr);
	snprintf(buf, sizeof(buf), "%lu", cnt);
	tag_attr(w, tag, "", buf);
	tag_end(w);
}

void
display_interface_stats(lldpctl_conn_t *conn, struct writer *w,
		lldpctl_atom_t *port)
{
	tag_start(w, "interface", "Interface");
	tag_attr(w, "name", "",
	    lldpctl_atom_get_str(port, lldpctl_k_port_name));

	display_stat(w, "tx", "Transmitted",
			lldpctl_atom_get_int(port, lldpctl_k_tx_cnt));
	display_stat(w, "rx", "Received",
			lldpctl_atom_get_int(port, lldpctl_k_rx_cnt));

	display_stat(w, "rx_discarded_cnt", "Discarded",
			lldpctl_atom_get_int(port,
			lldpctl_k_rx_discarded_cnt));

	display_stat(w, "rx_unrecognized_cnt", "Unrecognized",
			lldpctl_atom_get_int(port,
			lldpctl_k_rx_unrecognized_cnt));

	display_stat(w, "ageout_cnt", "Ageout",
			lldpctl_atom_get_int(port,
			lldpctl_k_ageout_cnt));

	display_stat(w, "insert_cnt", "Inserted",
			lldpctl_atom_get_int(port,
			lldpctl_k_insert_cnt));

	display_stat(w, "delete_cnt", "Deleted",
			lldpctl_atom_get_int(port,
			lldpctl_k_delete_cnt));

	tag_end(w);
}

/**
 * Display interface stats
 *
 * @param conn       Connection to lldpd.
 * @param w          Writer.
 * @param env        Environment from which we may find the list of ports.
 */
void
display_interfaces_stats(lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env)
{
	lldpctl_atom_t *iface;
	int summary = 0;
	u_int64_t h_tx_cnt = 0;
	u_int64_t h_rx_cnt = 0;
	u_int64_t h_rx_discarded_cnt = 0;
	u_int64_t h_rx_unrecognized_cnt = 0;
	u_int64_t h_ageout_cnt = 0;
	u_int64_t h_insert_cnt = 0;
	u_int64_t h_delete_cnt = 0;

	if (cmdenv_get(env, "summary"))
		summary = 1;

	tag_start(w, "lldp", (summary ? "LLDP Global statistics" :
		"LLDP statistics"));
	while ((iface = cmd_iterate_on_interfaces(conn, env))) {
		lldpctl_atom_t *port;
		port      = lldpctl_get_port(iface);
		if (!summary)
			display_interface_stats(conn, w, port);
		else {
			h_tx_cnt += lldpctl_atom_get_int(port,
					lldpctl_k_tx_cnt);
			h_rx_cnt += lldpctl_atom_get_int(port,
					lldpctl_k_rx_cnt);
			h_rx_discarded_cnt += lldpctl_atom_get_int(port,
					lldpctl_k_rx_discarded_cnt);
			h_rx_unrecognized_cnt += lldpctl_atom_get_int(port,
					lldpctl_k_rx_unrecognized_cnt);
			h_ageout_cnt += lldpctl_atom_get_int(port,
						lldpctl_k_ageout_cnt);
			h_insert_cnt += lldpctl_atom_get_int(port,
						lldpctl_k_insert_cnt);
			h_delete_cnt += lldpctl_atom_get_int(port,
						lldpctl_k_delete_cnt);
		}
		lldpctl_atom_dec_ref(port);
	}

	if (summary) {
		tag_start(w, "summary", "Summary of stats");
		display_stat(w, "tx", "Transmitted", h_tx_cnt);
		display_stat(w, "rx", "Received", h_rx_cnt);
		display_stat(w, "rx_discarded_cnt", "Discarded",
			h_rx_discarded_cnt);

		display_stat(w, "rx_unrecognized_cnt", "Unrecognized",
			h_rx_unrecognized_cnt);

		display_stat(w, "ageout_cnt", "Ageout", h_ageout_cnt);

		display_stat(w, "insert_cnt", "Inserted", h_insert_cnt);

		display_stat(w, "delete_cnt", "Deleted", h_delete_cnt);
		tag_end(w);
	}
	tag_end(w);
}

static const char *
N(const char *str) {
	if (str == NULL || strlen(str) == 0) return "(none)";
	return str;
}

void
display_configuration(lldpctl_conn_t *conn, struct writer *w)
{
	lldpctl_atom_t *configuration;

	configuration = lldpctl_get_configuration(conn);
	if (!configuration) {
		log_warnx("lldpctl", "not able to get configuration. %s",
		    lldpctl_last_strerror(conn));
		return;
	}

	tag_start(w, "configuration", "Global configuration");
	tag_start(w, "config", "Configuration");

	tag_datatag(w, "tx-delay", "Transmit delay",
	    lldpctl_atom_get_str(configuration, lldpctl_k_config_tx_interval));
	tag_datatag(w, "tx-hold", "Transmit hold",
	    lldpctl_atom_get_str(configuration, lldpctl_k_config_tx_hold));
	tag_datatag(w, "rx-only", "Receive mode",
	    lldpctl_atom_get_int(configuration, lldpctl_k_config_receiveonly)?
	    "yes":"no");
	tag_datatag(w, "mgmt-pattern", "Pattern for management addresses",
	    N(lldpctl_atom_get_str(configuration, lldpctl_k_config_mgmt_pattern)));
	tag_datatag(w, "iface-pattern", "Interface pattern",
	    N(lldpctl_atom_get_str(configuration, lldpctl_k_config_iface_pattern)));
	tag_datatag(w, "cid-pattern", "Interface pattern for chassis ID",
	    N(lldpctl_atom_get_str(configuration, lldpctl_k_config_cid_pattern)));
	tag_datatag(w, "description", "Override description with",
	    N(lldpctl_atom_get_str(configuration, lldpctl_k_config_description)));
	tag_datatag(w, "platform", "Override platform with",
	    N(lldpctl_atom_get_str(configuration, lldpctl_k_config_platform)));
	tag_datatag(w, "hostname", "Override system name with",
	    N(lldpctl_atom_get_str(configuration, lldpctl_k_config_hostname)));
	tag_datatag(w, "advertise-version", "Advertise version",
	    lldpctl_atom_get_int(configuration, lldpctl_k_config_advertise_version)?
	    "yes":"no");
	tag_datatag(w, "ifdescr-update", "Update interface descriptions",
	    lldpctl_atom_get_int(configuration, lldpctl_k_config_ifdescr_update)?
	    "yes":"no");
	tag_datatag(w, "iface-promisc", "Promiscuous mode on managed interfaces",
	    lldpctl_atom_get_int(configuration, lldpctl_k_config_iface_promisc)?
	    "yes":"no");
	tag_datatag(w, "lldpmed-no-inventory", "Disable LLDP-MED inventory",
	    (lldpctl_atom_get_int(configuration, lldpctl_k_config_lldpmed_noinventory) == 0)?
	    "no":"yes");
	tag_datatag(w, "lldpmed-faststart", "LLDP-MED fast start mechanism",
	    (lldpctl_atom_get_int(configuration, lldpctl_k_config_fast_start_enabled) == 0)?
	    "no":"yes");
	tag_datatag(w, "lldpmed-faststart-interval", "LLDP-MED fast start interval",
	    N(lldpctl_atom_get_str(configuration, lldpctl_k_config_fast_start_interval)));
	tag_datatag(w, "bond-slave-src-mac-type",
		"Source MAC for LLDP frames on bond slaves",
		lldpctl_atom_get_str(configuration,
			lldpctl_k_config_bond_slave_src_mac_type));
	tag_datatag(w, "lldp-portid-type",
		"Port ID TLV subtype for LLDP frames",
		lldpctl_atom_get_str(configuration,
			lldpctl_k_config_lldp_portid_type));
	tag_datatag(w, "lldp-agent-type",
		"Agent type",
		lldpctl_atom_get_str(configuration,
			lldpctl_k_config_lldp_agent_type));

	tag_end(w);
	tag_end(w);

	lldpctl_atom_dec_ref(configuration);
}
