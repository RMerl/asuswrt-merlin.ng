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

#ifdef ENABLE_SONMP

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

static struct sonmp_chassis sonmp_chassis_types[] = {
	{1, "unknown (via SONMP)"},
	{2, "Nortel 3000"},
	{3, "Nortel 3030"},
	{4, "Nortel 2310"},
	{5, "Nortel 2810"},
	{6, "Nortel 2912"},
	{7, "Nortel 2914"},
	{8, "Nortel 271x"},
	{9, "Nortel 2813"},
	{10, "Nortel 2814"},
	{11, "Nortel 2915"},
	{12, "Nortel 5000"},
	{13, "Nortel 2813SA"},
	{14, "Nortel 2814SA"},
	{15, "Nortel 810M"},
	{16, "Nortel EtherCell"},
	{17, "Nortel 5005"},
	{18, "Alcatel Ethernet workgroup conc."},
	{20, "Nortel 2715SA"},
	{21, "Nortel 2486"},
	{22, "Nortel 28000 series"},
	{23, "Nortel 23000 series"},
	{24, "Nortel 5DN00x series"},
	{25, "BayStack Ethernet"},
	{26, "Nortel 23100 series"},
	{27, "Nortel 100Base-T Hub"},
	{28, "Nortel 3000 Fast Ethernet"},
	{29, "Nortel Orion switch"},
	{30, "unknown"},
	{31, "Nortel DDS "},
	{32, "Nortel Centillion"},
	{33, "Nortel Centillion"},
	{34, "Nortel Centillion"},
	{35, "BayStack 301"},
	{36, "BayStack TokenRing Hub"},
	{37, "Nortel FVC Multimedia Switch"},
	{38, "Nortel Switch Node"},
	{39, "BayStack 302 Switch"},
	{40, "BayStack 350 Switch"},
	{41, "BayStack 150 Ethernet Hub"},
	{42, "Nortel Centillion 50N switch"},
	{43, "Nortel Centillion 50T switch"},
	{44, "BayStack 303 and 304 Switches"},
	{45, "BayStack 200 Ethernet Hub"},
	{46, "BayStack 250 10/100 Ethernet Hub"},
	{48, "BayStack 450 10/100/1000 Switches"},
	{49, "BayStack 410 10/100 Switches"},
	{50, "Nortel Ethernet Routing 1200 L3 Switch"},
	{51, "Nortel Ethernet Routing 1250 L3 Switch"},
	{52, "Nortel Ethernet Routing 1100 L3 Switch"},
	{53, "Nortel Ethernet Routing 1150 L3 Switch"},
	{54, "Nortel Ethernet Routing 1050 L3 Switch"},
	{55, "Nortel Ethernet Routing 1051 L3 Switch"},
	{56, "Nortel Ethernet Routing 8610 L3 Switch"},
	{57, "Nortel Ethernet Routing 8606 L3 Switch"},
	{58, "Nortel Ethernet Routing Switch 8010"},
	{59, "Nortel Ethernet Routing Switch 8006"},
	{60, "BayStack 670 wireless access point"},
	{61, "Nortel Ethernet Routing Switch 740 "},
	{62, "Nortel Ethernet Routing Switch 750 "},
	{63, "Nortel Ethernet Routing Switch 790"},
	{64, "Nortel Business Policy Switch 2000 10/100 Switches"},
	{65, "Nortel Ethernet Routing 8110 L2 Switch"},
	{66, "Nortel Ethernet Routing 8106 L2 Switch"},
	{67, "BayStack 3580 Gig Switch"},
	{68, "BayStack 10 Power Supply Unit"},
	{69, "BayStack 420 10/100 Switch"},
	{70, "OPTera Metro 1200 Ethernet Service Module"},
	{71, "Nortel Ethernet Routing Switch 8010co"},
	{72, "Nortel Ethernet Routing 8610co L3 switch"},
	{73, "Nortel Ethernet Routing 8110co L2 switch"},
	{74, "Nortel Ethernet Routing 8003"},
	{75, "Nortel Ethernet Routing 8603 L3 switch"},
	{76, "Nortel Ethernet Routing 8103 L2 switch"},
	{77, "BayStack 380 10/100/1000 Switch"},
	{78, "Nortel Ethernet Switch 470-48T"},
	{79, "OPTera Metro 1450 Ethernet Service Module"},
	{80, "OPTera Metro 1400 Ethernet Service Module"},
	{81, "Alteon Switch Family"},
	{82, "Ethernet Switch 460-24T-PWR"},
	{83, "OPTera Metro 8010 OPM L2 Switch"},
	{84, "OPTera Metro 8010co OPM L2 Switch"},
	{85, "OPTera Metro 8006 OPM L2 Switch"},
	{86, "OPTera Metro 8003 OPM L2 Switch"},
	{87, "Alteon 180e"},
	{88, "Alteon AD3"},
	{89, "Alteon 184"},
	{90, "Alteon AD4"},
	{91, "Nortel Ethernet Routing 1424 L3 switch"},
	{92, "Nortel Ethernet Routing 1648 L3 switch"},
	{93, "Nortel Ethernet Routing 1612 L3 switch"},
	{94, "Nortel Ethernet Routing 1624 L3 switch "},
	{95, "BayStack 380-24F Fiber 1000 Switch"},
	{96, "Nortel Ethernet Routing Switch 5510-24T"},
	{97, "Nortel Ethernet Routing Switch 5510-48T"},
	{98, "Nortel Ethernet Switch 470-24T"},
	{99, "Nortel Networks Wireless LAN Access Point 2220"},
	{100, "Ethernet Routing RBS 2402 L3 switch"},
	{101, "Alteon Application Switch 2424  "},
	{102, "Alteon Application Switch 2224 "},
	{103, "Alteon Application Switch 2208 "},
	{104, "Alteon Application Switch 2216"},
	{105, "Alteon Application Switch 3408"},
	{106, "Alteon Application Switch 3416"},
	{107, "Nortel Networks Wireless LAN SecuritySwitch 2250"},
	{108, "Ethernet Switch 425-48T"},
	{109, "Ethernet Switch 425-24T"},
	{110, "Nortel Networks Wireless LAN Access Point 2221"},
	{111, "Nortel Metro Ethernet Service Unit 24-T SPF switch"},
	{112, "Nortel Metro Ethernet Service Unit 24-T LX DC switch"},
	{113, "Nortel Ethernet Routing Switch 8300 10-slot chassis"},
	{114, "Nortel Ethernet Routing Switch 8300 6-slot chassis"},
	{115, "Nortel Ethernet Routing Switch 5520-24T-PWR"},
	{116, "Nortel Ethernet Routing Switch 5520-48T-PWR"},
	{117, "Nortel Networks VPN Gateway 3050"},
	{118, "Alteon SSL 310 10/100"},
	{119, "Alteon SSL 310 10/100 Fiber"},
	{120, "Alteon SSL 310 10/100 FIPS"},
	{121, "Alteon SSL 410 10/100/1000"},
	{122, "Alteon SSL 410 10/100/1000 Fiber"},
	{123, "Alteon Application Switch 2424-SSL"},
	{124, "Nortel Ethernet Switch 325-24T"},
	{125, "Nortel Ethernet Switch 325-24G"},
	{126, "Nortel Networks Wireless LAN Access Point 2225"},
	{127, "Nortel Networks Wireless LAN SecuritySwitch 2270"},
	{128, "Nortel 24-port Ethernet Switch 470-24T-PWR"},
	{129, "Nortel 48-port Ethernet Switch 470-48T-PWR"},
	{130, "Nortel Ethernet Routing Switch 5530-24TFD"},
	{131, "Nortel Ethernet Switch 3510-24T"},
	{132, "Nortel Metro Ethernet Service Unit 12G AC L3 switch"},
	{133, "Nortel Metro Ethernet Service Unit 12G DC L3 switch"},
	{134, "Nortel Secure Access Switch"},
	{135, "Networks VPN Gateway 3070"},
	{136, "OPTera Metro 3500"},
	{137, "SMB BES 1010 24T"},
	{138, "SMB BES 1010 48T"},
	{139, "SMB BES 1020 24T PWR"},
	{140, "SMB BES 1020 48T PWR"},
	{141, "SMB BES 2010 24T"},
	{142, "SMB BES 2010 48T"},
	{143, "SMB BES 2020 24T PWR"},
	{144, "SMB BES 2020 48T PWR"},
	{145, "SMB BES 110 24T"},
	{146, "SMB BES 110 48T"},
	{147, "SMB BES 120 24T PWR"},
	{148, "SMB BES 120 48T PWR"},
	{149, "SMB BES 210 24T"},
	{150, "SMB BES 210 48T"},
	{151, "SMB BES 220 24T PWR"},
	{152, "SMB BES 220 48T PWR"},
	{153, "OME 6500"},
	{0, "unknown (via SONMP)"},
};

int
sonmp_send(struct lldpd *global,
    struct lldpd_hardware *hardware)
{
	const u_int8_t mcastaddr[] = SONMP_MULTICAST_ADDR;
	const u_int8_t llcorg[] = LLC_ORG_NORTEL;
	struct lldpd_chassis *chassis;
	struct lldpd_mgmt *mgmt;
	u_int8_t *packet, *pos, *pos_pid, *end;
	int length;
	struct in_addr address;

	log_debug("sonmp", "send SONMP PDU to %s",
	    hardware->h_ifname);

	chassis = hardware->h_lport.p_chassis;
	length = hardware->h_mtu;
	if ((packet = (u_int8_t*)calloc(1, length)) == NULL)
		return ENOMEM;
	pos = packet;

	/* Ethernet header */
	if (!(
	      /* SONMP multicast address as target */
	      POKE_BYTES(mcastaddr, sizeof(mcastaddr)) &&
	      /* Source MAC addresss */
	      POKE_BYTES(&hardware->h_lladdr, ETHER_ADDR_LEN) &&
	      /* SONMP frame is of fixed size */
	      POKE_UINT16(SONMP_SIZE)))
		goto toobig;

	/* LLC header */
	if (!(
	      /* DSAP and SSAP */
	      POKE_UINT8(0xaa) && POKE_UINT8(0xaa) &&
	      /* Control field */
	      POKE_UINT8(0x03) &&
	      /* ORG */
	      POKE_BYTES(llcorg, sizeof(llcorg)) &&
	      POKE_SAVE(pos_pid) && /* We will modify PID later to
				       create a new frame */
	      POKE_UINT16(LLC_PID_SONMP_HELLO)))
		goto toobig;


	address.s_addr = htonl(INADDR_ANY);
	TAILQ_FOREACH(mgmt, &chassis->c_mgmt, m_entries) {
		if (mgmt->m_family == LLDPD_AF_IPV4) {
			address.s_addr = mgmt->m_addr.inet.s_addr;
		}
		break;
	}

	/* SONMP */
	if (!(
		  /* Our IP address */
		  POKE_BYTES(&address, sizeof(struct in_addr)) &&
		  /* Segment on three bytes, we don't have slots, so we
		 skip the first two bytes */
		  POKE_UINT16(0) &&
		  POKE_UINT8(hardware->h_ifindex) &&
		  POKE_UINT8(1) &&  /* Chassis: Other */
		  POKE_UINT8(12) &&	/* Back: Ethernet, Fast Ethernet and Gigabit */
		  POKE_UINT8(SONMP_TOPOLOGY_NEW) && /* Should work. We have no state */
		  POKE_UINT8(1) &&	/* Links: Dunno what it is */
		  POKE_SAVE(end)))
		goto toobig;
				
	if (interfaces_send_helper(global, hardware,
		(char *)packet, end - packet) == -1) {
		log_warn("sonmp", "unable to send packet on real device for %s",
			   hardware->h_ifname);
		free(packet);
		return ENETDOWN;
	}

	POKE_RESTORE(pos_pid);	/* Modify LLC PID */
	(void)POKE_UINT16(LLC_PID_SONMP_FLATNET);
	POKE_RESTORE(packet);	/* Go to the beginning */
	PEEK_DISCARD(ETHER_ADDR_LEN - 1); /* Modify the last byte of the MAC address */
	(void)POKE_UINT8(1);

	if (interfaces_send_helper(global, hardware,
		(char *)packet, end - packet) == -1) {
		log_warn("sonmp", "unable to send second SONMP packet on real device for %s",
			   hardware->h_ifname);
		free(packet);
		return ENETDOWN;
	}

	free(packet);
	hardware->h_tx_cnt++;
	return 0;
 toobig:
	free(packet);
	return -1;
}

int
sonmp_decode(struct lldpd *cfg, char *frame, int s,
    struct lldpd_hardware *hardware,
    struct lldpd_chassis **newchassis, struct lldpd_port **newport)
{
	const u_int8_t mcastaddr[] = SONMP_MULTICAST_ADDR;
	struct lldpd_chassis *chassis;
	struct lldpd_port *port;
	struct lldpd_mgmt *mgmt;
	int length, i;
	u_int8_t *pos;
	u_int8_t seg[3], rchassis;
	struct in_addr address;

	log_debug("sonmp", "decode SONMP PDU from %s",
	    hardware->h_ifname);

	if ((chassis = calloc(1, sizeof(struct lldpd_chassis))) == NULL) {
		log_warn("sonmp", "failed to allocate remote chassis");
		return -1;
	}
	TAILQ_INIT(&chassis->c_mgmt);
	if ((port = calloc(1, sizeof(struct lldpd_port))) == NULL) {
		log_warn("sonmp", "failed to allocate remote port");
		free(chassis);
		return -1;
	}
#ifdef ENABLE_DOT1
	TAILQ_INIT(&port->p_vlans);
#endif

	length = s;
	pos = (u_int8_t*)frame;
	if (length < SONMP_SIZE) {
		log_warnx("sonmp", "too short SONMP frame received on %s", hardware->h_ifname);
		goto malformed;
	}
	if (PEEK_CMP(mcastaddr, sizeof(mcastaddr)) != 0)
		/* There is two multicast address. We just handle only one of
		 * them. */
		goto malformed;
	/* We skip to LLC PID */
	PEEK_DISCARD(ETHER_ADDR_LEN); PEEK_DISCARD_UINT16;
	PEEK_DISCARD(6);
	if (PEEK_UINT16 != LLC_PID_SONMP_HELLO) {
		log_debug("sonmp", "incorrect LLC protocol ID received for SONMP on %s",
		    hardware->h_ifname);
		goto malformed;
	}

	chassis->c_id_subtype = LLDP_CHASSISID_SUBTYPE_ADDR;
	if ((chassis->c_id = calloc(1, sizeof(struct in_addr) + 1)) == NULL) {
		log_warn("sonmp", "unable to allocate memory for chassis id on %s",
			hardware->h_ifname);
		goto malformed;
	}
	chassis->c_id_len = sizeof(struct in_addr) + 1;
	chassis->c_id[0] = 1;
	PEEK_BYTES(&address, sizeof(struct in_addr));
	memcpy(chassis->c_id + 1, &address, sizeof(struct in_addr));
	if (asprintf(&chassis->c_name, "%s", inet_ntoa(address)) == -1) {
		log_warnx("sonmp", "unable to write chassis name for %s",
		    hardware->h_ifname);
		goto malformed;
	}
	PEEK_BYTES(seg, sizeof(seg));
	rchassis = PEEK_UINT8;
	for (i=0; sonmp_chassis_types[i].type != 0; i++) {
		if (sonmp_chassis_types[i].type == rchassis)
			break;
	}
	if (asprintf(&chassis->c_descr, "%s",
		sonmp_chassis_types[i].description) == -1) {
		log_warnx("sonmp", "unable to write chassis description for %s",
		    hardware->h_ifname);
		goto malformed;
	}
	mgmt = lldpd_alloc_mgmt(LLDPD_AF_IPV4, &address, sizeof(struct in_addr), 0);
	if (mgmt == NULL) {
		if (errno == ENOMEM)
			log_warn("sonmp", "unable to allocate memory for management address");
		else
			log_warn("sonmp", "too large management address received on %s",
			    hardware->h_ifname);
		goto malformed;
	}
	TAILQ_INSERT_TAIL(&chassis->c_mgmt, mgmt, m_entries);
	port->p_ttl = cfg?(cfg->g_config.c_tx_interval * cfg->g_config.c_tx_hold):
	    LLDPD_TTL;

	port->p_id_subtype = LLDP_PORTID_SUBTYPE_LOCAL;
	if (asprintf(&port->p_id, "%02x-%02x-%02x",
		seg[0], seg[1], seg[2]) == -1) {
		log_warn("sonmp", "unable to allocate memory for port id on %s",
		    hardware->h_ifname);
		goto malformed;
	}
	port->p_id_len = strlen(port->p_id);

	/* Port description depend on the number of segments */
	if ((seg[0] == 0) && (seg[1] == 0)) {
		if (asprintf(&port->p_descr, "port %d",
			seg[2]) == -1) {
			log_warnx("sonmp", "unable to write port description for %s",
			    hardware->h_ifname);
			goto malformed;
		}
	} else if (seg[0] == 0) {
		if (asprintf(&port->p_descr, "port %d/%d",
			seg[1], seg[2]) == -1) {
			log_warnx("sonmp", "unable to write port description for %s",
			    hardware->h_ifname);
			goto malformed;
		}
	} else {
		if (asprintf(&port->p_descr, "port %x:%x:%x",
			seg[0], seg[1], seg[2]) == -1) {
			log_warnx("sonmp", "unable to write port description for %s",
			    hardware->h_ifname);
			goto malformed;
		}
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

#endif /* ENABLE_SONMP */
