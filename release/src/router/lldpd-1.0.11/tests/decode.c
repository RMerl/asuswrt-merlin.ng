/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2015 Vincent Bernat <bernat@luffy.cx>
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

#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "common.h"

#define BUFSIZE 2000

char filenameprefix[] = "decode";

static void
usage(void)
{
	fprintf(stderr, "Usage:   %s PCAP\n", "decode");
	fprintf(stderr, "Version: %s\n", PACKAGE_STRING);

	fprintf(stderr, "\n");

	fprintf(stderr, "Decode content of PCAP files and display a summary\n");
	fprintf(stderr, "on standard output. Only the first packet is decoded.\n");
	exit(1);
}

char*
tohex(char *str, size_t len)
{
	static char *hex = NULL;
	free(hex); hex = NULL;
	if ((hex = malloc(len * 3 + 1)) == NULL) return NULL;
	for (size_t i = 0; i < len; i++)
		snprintf(hex + 3*i, 4, "%02X ", (unsigned char)str[i]);
	return hex;
}

/* We need an assert macro which doesn't abort */
#define assert(x) while (!(x)) { \
		fprintf(stderr, "%s:%d: %s: Assertion  `%s' failed.\n", \
		    __FILE__, __LINE__, __func__, #x); \
		exit(5); \
	}

int
main(int argc, char **argv)
{
	if (argc != 2 ||
	    !strcmp(argv[1], "-h") ||
	    !strcmp(argv[1], "--help"))
		usage();

	int fd = open(argv[1], O_RDONLY);
	assert(fd != -1);

	char buf[BUFSIZE];
	ssize_t len = read(fd, buf, BUFSIZE);
	assert(len != -1);

	struct pcap_hdr hdr;
	assert(len >= sizeof(hdr));
	memcpy(&hdr, buf, sizeof(hdr));
	assert(hdr.magic_number == 0xa1b2c3d4); /* Assume the same byte order as us */
	assert(hdr.version_major == 2);
	assert(hdr.version_minor == 4);
	assert(hdr.thiszone == 0);
	/* Don't care about other flags */

	struct pcaprec_hdr rechdr;
	assert(len >= sizeof(hdr) + sizeof(rechdr));
	memcpy(&rechdr, buf + sizeof(hdr), sizeof(rechdr));
	assert(len >= sizeof(hdr) + sizeof(rechdr) + rechdr.incl_len);

	/* For decoding, we only need a very basic hardware */
	struct lldpd_hardware hardware;
	memset(&hardware, 0, sizeof(struct lldpd_hardware));
	hardware.h_mtu = 1500;
	strlcpy(hardware.h_ifname, "test", sizeof(hardware.h_ifname));

	char *frame = buf + sizeof(hdr) + sizeof(rechdr);
	struct lldpd_chassis *nchassis = NULL;
	struct lldpd_port *nport = NULL;
	int decoded = 0;
	if (lldp_decode(NULL, frame, rechdr.incl_len, &hardware, &nchassis, &nport) == -1) {
		fprintf(stderr, "Not decoded as a LLDP frame\n");
	} else {
		fprintf(stderr, "Decoded as a LLDP frame\n");
		decoded = 1;
	}
#if defined ENABLE_CDP || defined ENABLE_FDP
	if (cdp_decode(NULL, frame, rechdr.incl_len, &hardware, &nchassis, &nport) == -1) {
		fprintf(stderr, "Not decoded as a CDP frame\n");
	} else {
		fprintf(stderr, "Decoded as a CDP frame\n");
		decoded = 1;
	}
#endif
#ifdef ENABLE_SONMP
	if (sonmp_decode(NULL, frame, rechdr.incl_len, &hardware, &nchassis, &nport) == -1) {
		fprintf(stderr, "Not decoded as a SONMP frame\n");
	} else {
		fprintf(stderr, "Decoded as a SONMP frame\n");
		decoded = 1;
	}
#endif
#ifdef ENABLE_EDP
	if (edp_decode(NULL, frame, rechdr.incl_len, &hardware, &nchassis, &nport) == -1) {
		fprintf(stderr, "Not decoded as a EDP frame\n");
	} else {
		fprintf(stderr, "Decoded as a EDP frame\n");
		decoded = 1;
	}
#endif
	if (!decoded) exit(1);

	printf("Chassis:\n");
	printf(" Index: %" PRIu16 "\n", nchassis->c_index);
	printf(" Protocol: %" PRIu8 "\n", nchassis->c_protocol);
	printf(" ID subtype: %" PRIu8 "\n", nchassis->c_id_subtype);
	printf(" ID: %s\n", tohex(nchassis->c_id, nchassis->c_id_len));
	printf(" Name: %s\n", nchassis->c_name?nchassis->c_name:"(null)");
	printf(" Description: %s\n", nchassis->c_descr?nchassis->c_descr:"(null)");
	printf(" Cap available: %" PRIu16 "\n", nchassis->c_cap_available);
	printf(" Cap enabled: %" PRIu16 "\n", nchassis->c_cap_enabled);
	struct lldpd_mgmt *mgmt;
	TAILQ_FOREACH(mgmt, &nchassis->c_mgmt, m_entries) {
		char ipaddress[INET6_ADDRSTRLEN + 1];
		int af; size_t alen;
		switch (mgmt->m_family) {
		case LLDPD_AF_IPV4:
			alen = INET_ADDRSTRLEN + 1;
			af  = AF_INET;
			break;
		case LLDPD_AF_IPV6:
			alen = INET6_ADDRSTRLEN + 1;
			af = AF_INET6;
			break;
		default:
			len = 0;
		}
		if (len == 0) continue;
		if (inet_ntop(af, &mgmt->m_addr, ipaddress, alen) == NULL)
			break;
		printf(" mgmt: %s\n", ipaddress);
	}
#ifdef ENABLE_LLDPMED
	printf(" MED cap: %" PRIu16 "\n", nchassis->c_med_cap_available);
	printf(" MED type: %" PRIu8 "\n", nchassis->c_med_type);
	printf(" MED HW: %s\n", nchassis->c_med_hw?nchassis->c_med_hw:"(null)");
	printf(" MED FW: %s\n", nchassis->c_med_fw?nchassis->c_med_fw:"(null)");
	printf(" MED SW: %s\n", nchassis->c_med_sw?nchassis->c_med_sw:"(null)");
	printf(" MED SN: %s\n", nchassis->c_med_sn?nchassis->c_med_sn:"(null)");
	printf(" MED manufacturer: %s\n", nchassis->c_med_manuf?nchassis->c_med_manuf:"(null)");
	printf(" MED model: %s\n", nchassis->c_med_model?nchassis->c_med_model:"(null)");
	printf(" MED asset: %s\n", nchassis->c_med_asset?nchassis->c_med_asset:"(null)");
#endif

	printf("Port:\n");
	printf(" ID subtype: %" PRIu8 "\n", nport->p_id_subtype);
	printf(" ID: %s\n", tohex(nport->p_id, nport->p_id_len));
	printf(" Description: %s\n", nport->p_descr?nport->p_descr:"(null)");
	printf(" MFS: %" PRIu16 "\n", nport->p_mfs);
	printf(" TTL: %" PRIu16 "\n", nport->p_ttl);
#ifdef ENABLE_DOT3
	printf(" Dot3 aggrID: %" PRIu32 "\n", nport->p_aggregid);
	printf(" Dot3 MAC/phy autoneg supported: %" PRIu8 "\n", nport->p_macphy.autoneg_support);
	printf(" Dot3 MAC/phy autoneg enabled: %" PRIu8 "\n", nport->p_macphy.autoneg_enabled);
	printf(" Dot3 MAC/phy autoneg advertised: %" PRIu16 "\n", nport->p_macphy.autoneg_advertised);
	printf(" Dot3 MAC/phy MAU type: %" PRIu16 "\n", nport->p_macphy.mau_type);
	printf(" Dot3 power device type: %" PRIu8 "\n", nport->p_power.devicetype);
	printf(" Dot3 power supported: %" PRIu8 "\n", nport->p_power.supported);
	printf(" Dot3 power enabled: %" PRIu8 "\n", nport->p_power.enabled);
	printf(" Dot3 power pair control: %" PRIu8 "\n", nport->p_power.paircontrol);
	printf(" Dot3 power pairs: %" PRIu8 "\n", nport->p_power.pairs);
	printf(" Dot3 power class: %" PRIu8 "\n", nport->p_power.class);
	printf(" Dot3 power type: %" PRIu8 "\n", nport->p_power.powertype);
	printf(" Dot3 power source: %" PRIu8 "\n", nport->p_power.source);
	printf(" Dot3 power priority: %" PRIu8 "\n", nport->p_power.priority);
	printf(" Dot3 power requested: %" PRIu16 "\n", nport->p_power.requested);
	printf(" Dot3 power allocated: %" PRIu16 "\n", nport->p_power.allocated);
#endif
#ifdef ENABLE_LLDPMED
	printf(" MED cap: %" PRIu16 "\n", nport->p_med_cap_enabled);
	for (int i = 0; i < LLDP_MED_APPTYPE_LAST; i++) {
		if (nport->p_med_policy[i].type == 0) continue;
		printf(" MED policy type: %" PRIu8 "\n", nport->p_med_policy[i].type);
		printf(" MED policy unknown: %" PRIu8 "\n", nport->p_med_policy[i].unknown);
		printf(" MED policy tagged: %" PRIu8 "\n", nport->p_med_policy[i].tagged);
		printf(" MED policy vid: %" PRIu16 "\n", nport->p_med_policy[i].vid);
		printf(" MED policy priority: %" PRIu8 "\n", nport->p_med_policy[i].priority);
		printf(" MED policy dscp: %" PRIu8 "\n", nport->p_med_policy[i].dscp);
	}
	for (int i = 0; i < LLDP_MED_LOCFORMAT_LAST; i++) {
		if (nport->p_med_location[i].format == 0) continue;
		printf(" MED location format: %" PRIu8 "\n", nport->p_med_location[i].format);
		printf(" MED location: %s\n", tohex(nport->p_med_location[i].data,
			nport->p_med_location[i].data_len));
	}
	printf(" MED power device type: %" PRIu8 "\n", nport->p_med_power.devicetype);
	printf(" MED power source: %" PRIu8 "\n", nport->p_med_power.source);
	printf(" MED power priority: %" PRIu8 "\n", nport->p_med_power.priority);
	printf(" MED power value: %" PRIu16 "\n", nport->p_med_power.val);
#endif
#ifdef ENABLE_DOT1
	printf(" Dot1 PVID: %" PRIu16 "\n", nport->p_pvid);
	struct lldpd_vlan *vlan;
	TAILQ_FOREACH(vlan, &nport->p_vlans, v_entries) {
		printf(" Dot1 VLAN: %s (%" PRIu16 ")\n", vlan->v_name, vlan->v_vid);
	}
	struct lldpd_ppvid *ppvid;
	TAILQ_FOREACH(ppvid, &nport->p_ppvids, p_entries) {
		printf(" Dot1 PPVID: %" PRIu16 " (status: %" PRIu8 ")\n",
		    ppvid->p_ppvid, ppvid->p_cap_status);
	}
	struct lldpd_pi *pid;
	TAILQ_FOREACH(pid, &nport->p_pids, p_entries) {
		printf(" Dot1 PI: %s\n", tohex(pid->p_pi, pid->p_pi_len));
	}
#endif
#ifdef ENABLE_CUSTOM
	struct lldpd_custom *custom;
	TAILQ_FOREACH(custom, &nport->p_custom_list, next) {
		printf(" Custom OUI: %s\n",
		    tohex((char*)custom->oui, sizeof(custom->oui)));
		printf(" Custom subtype: %" PRIu8 "\n", custom->subtype);
		printf(" Custom info: %s\n",
		    tohex((char*)custom->oui_info, custom->oui_info_len));
	}
#endif
	exit(0);
}
