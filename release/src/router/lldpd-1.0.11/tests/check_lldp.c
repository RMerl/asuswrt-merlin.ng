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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <check.h>
#include "common.h"

char filenameprefix[] = "lldp_send";

static struct lldpd test_lldpd = {
	.g_config = {
		.c_cap_advertise     = 1,  /* Chassis capabilities advertisement */
		.c_mgmt_advertise    = 1,  /* Management addresses advertisement */
	}
};

#define ck_assert_str_eq_n(X, Y, N) \
	ck_assert_msg(!strncmp(X, Y, N), "Assertion '"#X"=="#Y"' failed: "#X"==\"%s\", "#Y"==\"%s\"", X, Y)

static void
check_received_port(
	struct lldpd_port *sport,
	struct lldpd_port *rport)
{
	ck_assert_int_eq(rport->p_id_subtype, sport->p_id_subtype);
	ck_assert_int_eq(rport->p_id_len, sport->p_id_len);
	ck_assert_str_eq_n(rport->p_id, sport->p_id, sport->p_id_len);
	ck_assert_str_eq(rport->p_descr, sport->p_descr);
#ifdef ENABLE_DOT3
	ck_assert_int_eq(rport->p_mfs, sport->p_mfs);
#endif
}

static void
check_received_chassis(
	struct lldpd_chassis *schassis,
	struct lldpd_chassis *rchassis)
{
	ck_assert_int_eq(rchassis->c_id_subtype, schassis->c_id_subtype);
	ck_assert_int_eq(rchassis->c_id_len, schassis->c_id_len);
	ck_assert_str_eq_n(rchassis->c_id, schassis->c_id, schassis->c_id_len);
	ck_assert_str_eq(rchassis->c_name, schassis->c_name);
	ck_assert_str_eq(rchassis->c_descr, schassis->c_descr);
	ck_assert_int_eq(rchassis->c_cap_available, schassis->c_cap_available);
	ck_assert_int_eq(rchassis->c_cap_enabled, schassis->c_cap_enabled);
}

#ifdef ENABLE_LLDPMED
static void
check_received_port_med(
	struct lldpd_port *sport,
	struct lldpd_port *rport)
{
	ck_assert_int_eq(rport->p_med_cap_enabled, sport->p_med_cap_enabled);
	ck_assert_int_eq(rport->p_med_cap_enabled, sport->p_med_cap_enabled);
	ck_assert_int_eq(
		rport->p_med_location[LLDP_MED_LOCFORMAT_CIVIC-1].format,
		sport->p_med_location[LLDP_MED_LOCFORMAT_CIVIC-1].format);
	ck_assert_int_eq(
		rport->p_med_location[LLDP_MED_LOCFORMAT_CIVIC-1].data_len,
		sport->p_med_location[LLDP_MED_LOCFORMAT_CIVIC-1].data_len);
	ck_assert_str_eq_n(
		rport->p_med_location[LLDP_MED_LOCFORMAT_CIVIC-1].data,
		sport->p_med_location[LLDP_MED_LOCFORMAT_CIVIC-1].data,
		sport->p_med_location[LLDP_MED_LOCFORMAT_CIVIC-1].data_len);
	ck_assert_int_eq(
		rport->p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].type,
		sport->p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].type);
	ck_assert_int_eq(
		rport->p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].tagged,
		sport->p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].tagged);
	ck_assert_int_eq(
		rport->p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].vid,
		sport->p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].vid);
	ck_assert_int_eq(
		rport->p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].priority,
		sport->p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].priority);
	ck_assert_int_eq(
		rport->p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].dscp,
		sport->p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].dscp);
	ck_assert_int_eq(
		rport->p_med_power.devicetype, sport->p_med_power.devicetype);
	ck_assert_int_eq(rport->p_med_power.source, sport->p_med_power.source);
	ck_assert_int_eq(rport->p_med_power.priority,
		sport->p_med_power.priority);
	ck_assert_int_eq(rport->p_med_power.val, sport->p_med_power.val);
}

static void
check_received_chassis_med(
	struct lldpd_chassis *schassis,
	struct lldpd_chassis *rchassis)
{
	ck_assert_int_eq(rchassis->c_med_cap_available,
		schassis->c_med_cap_available);
	ck_assert_int_eq(rchassis->c_med_type, schassis->c_med_type);
	ck_assert_str_eq(rchassis->c_med_hw, schassis->c_med_hw);
	ck_assert_str_eq(rchassis->c_med_fw, schassis->c_med_fw);
	ck_assert_str_eq(rchassis->c_med_sw, schassis->c_med_sw);
	ck_assert_str_eq(rchassis->c_med_sn, schassis->c_med_sn);
}
#endif

#ifdef ENABLE_DOT3
static void
check_received_port_dot3(
	struct lldpd_port *sport,
	struct lldpd_port *rport)
{
	ck_assert_int_eq(rport->p_aggregid, sport->p_aggregid);
	ck_assert_int_eq(rport->p_macphy.autoneg_support,
		sport->p_macphy.autoneg_support);
	ck_assert_int_eq(rport->p_macphy.autoneg_enabled,
		sport->p_macphy.autoneg_enabled);
	ck_assert_int_eq(rport->p_macphy.autoneg_advertised,
		sport->p_macphy.autoneg_advertised);
	ck_assert_int_eq(rport->p_macphy.mau_type, sport->p_macphy.mau_type);
}
#endif

START_TEST (test_send_rcv_basic)
{
	int n;
	struct packet *pkt;
	struct lldpd_chassis *nchassis = NULL;
	struct lldpd_port *nport = NULL;

	/* Populate port and chassis */
	hardware.h_lport.p_id_subtype = LLDP_PORTID_SUBTYPE_IFNAME;
	hardware.h_lport.p_id = "FastEthernet 1/5";
	hardware.h_lport.p_id_len = strlen(hardware.h_lport.p_id);
	hardware.h_lport.p_descr = "Fake port description";
	hardware.h_lport.p_mfs = 1516;
	chassis.c_id_subtype = LLDP_CHASSISID_SUBTYPE_LLADDR;
	chassis.c_id = macaddress;
	chassis.c_id_len = ETHER_ADDR_LEN;
	chassis.c_name = "First chassis";
	chassis.c_descr = "Chassis description";
	chassis.c_cap_available = chassis.c_cap_enabled = LLDP_CAP_ROUTER;

	/* Build packet */
	n = lldp_send(&test_lldpd, &hardware);
	if (n != 0) {
		fail("unable to build packet");
		return;
	}
	if (TAILQ_EMPTY(&pkts)) {
		fail("no packets sent");
		return;
	}
	pkt = TAILQ_FIRST(&pkts);
	fail_unless(TAILQ_NEXT(pkt, next) == NULL, "more than one packet sent");

	/* decode the retrieved packet calling lldp_decode() */
	fail_unless(lldp_decode(NULL, pkt->data, pkt->size, &hardware,
		&nchassis, &nport) != -1);
	if (!nchassis || !nport) {
		fail("unable to decode packet");
		return;
	}
	/* verify port values */
	check_received_port(&hardware.h_lport, nport);
	/* verify chassis values */
	check_received_chassis(&chassis, nchassis);
}
END_TEST

#define ETHERTYPE_OFFSET 2 * ETHER_ADDR_LEN
#define VLAN_TAG_SIZE 2
START_TEST (test_send_rcv_vlan_tx)
{
	int n;
	struct packet *pkt;
	struct lldpd_chassis *nchassis = NULL;
	struct lldpd_port *nport = NULL;
	int vlan_id = 100;
	int vlan_prio = 5;
	int vlan_dei = 1;
	unsigned int vlan_tag = 0;
	unsigned int tmp;

	/* Populate port and chassis */
	hardware.h_lport.p_id_subtype = LLDP_PORTID_SUBTYPE_IFNAME;
	hardware.h_lport.p_id = "FastEthernet 1/5";
	hardware.h_lport.p_id_len = strlen(hardware.h_lport.p_id);
	hardware.h_lport.p_descr = "Fake port description";
	hardware.h_lport.p_mfs = 1516;

	/* Assembly VLAN tag: Priority(3bits) | DEI(1bit) | VID(12bits) */
	vlan_tag = ((vlan_prio & 0x7) << 13) |
		   ((vlan_dei & 0x1) << 12) |
		   (vlan_id & 0xfff);
	hardware.h_lport.p_vlan_tx_tag = vlan_tag;
	hardware.h_lport.p_vlan_tx_enabled = 1;
	chassis.c_id_subtype = LLDP_CHASSISID_SUBTYPE_LLADDR;
	chassis.c_id = macaddress;
	chassis.c_id_len = ETHER_ADDR_LEN;
	chassis.c_name = "First chassis";
	chassis.c_descr = "Chassis description";
	chassis.c_cap_available = chassis.c_cap_enabled = LLDP_CAP_ROUTER;

	/* Build packet */
	n = lldp_send(&test_lldpd, &hardware);
	if (n != 0) {
		fail("unable to build packet");
		return;
	}
	if (TAILQ_EMPTY(&pkts)) {
		fail("no packets sent");
		return;
	}
	pkt = TAILQ_FIRST(&pkts);
	fail_unless(TAILQ_NEXT(pkt, next) == NULL, "more than one packet sent");

	/* Check ETHER_TYPE, should be VLAN */
	memcpy(&tmp, (unsigned char*) pkt->data + ETHERTYPE_OFFSET, ETHER_TYPE_LEN);
	ck_assert_uint_eq(ntohl(tmp)>>16, ETHERTYPE_VLAN);

	/* Check VLAN tag */
	memcpy(&tmp, (unsigned char*) pkt->data + ETHERTYPE_OFFSET + ETHER_TYPE_LEN, VLAN_TAG_SIZE);
	ck_assert_uint_eq(ntohl(tmp)>>16, vlan_tag);

	/* Remove VLAN ethertype and VLAN tag */
	memmove((unsigned char*) pkt->data + ETHERTYPE_OFFSET,
		/* move all after VLAN tag */
		(unsigned char*) pkt->data + ETHERTYPE_OFFSET + ETHER_TYPE_LEN + VLAN_TAG_SIZE,
		/* size without src and dst MAC, VLAN tag */
		pkt->size - (ETHERTYPE_OFFSET + ETHER_TYPE_LEN + VLAN_TAG_SIZE));

	/* Decode the packet without VLAN tag, calling lldp_decode() */
	fail_unless(lldp_decode(NULL, pkt->data, pkt->size, &hardware,
		&nchassis, &nport) != -1);
	if (!nchassis || !nport) {
		fail("unable to decode packet");
		return;
	}

	/* Verify port values (VLAN information is not checked here) */
	check_received_port(&hardware.h_lport, nport);
	/* Verify chassis values */
	check_received_chassis(&chassis, nchassis);
}
END_TEST

#ifdef ENABLE_DOT1
/* This test case tests send and receive of all DOT1 TLVs(2005 and 2009): 
   Port Valn ID, VLAN, Port Protocol VLAN ID, Protocol Identity,
   VID Usage Digest, Management VID, and 802.1ax Link Aggregation TLVs */
START_TEST (test_send_rcv_dot1_tlvs)
{
	int n;
	struct lldpd_vlan *rvlan, vlan1, vlan2, vlan3;
	struct lldpd_ppvid ppvid, *rppvid;
	struct lldpd_pi pi1, pi2, *rpi;
	struct lldpd_chassis *nchassis = NULL;
	struct lldpd_port *nport = NULL;
	struct packet *pkt;

	/* Populate port and chassis */
	hardware.h_lport.p_id_subtype = LLDP_PORTID_SUBTYPE_LLADDR;
	hardware.h_lport.p_id = macaddress;
	hardware.h_lport.p_id_len = ETHER_ADDR_LEN;
	hardware.h_lport.p_descr = "Fake port description";
	hardware.h_lport.p_mfs = 1516;
	hardware.h_lport.p_pvid = 1500;
	chassis.c_id_subtype = LLDP_CHASSISID_SUBTYPE_LOCAL;
	chassis.c_id = "Chassis name";
	chassis.c_id_len = strlen(chassis.c_id);
	chassis.c_name = "Second chassis";
	chassis.c_descr = "Chassis description";
	chassis.c_cap_available = LLDP_CAP_ROUTER | LLDP_CAP_BRIDGE;
	chassis.c_cap_enabled = LLDP_CAP_ROUTER;
	vlan1.v_name = "Voice"; vlan1.v_vid = 157;
	vlan2.v_name = "Data"; vlan2.v_vid = 1247;
	vlan3.v_name = "Control"; vlan3.v_vid = 741;
	TAILQ_INSERT_TAIL(&hardware.h_lport.p_vlans, &vlan1, v_entries);
	TAILQ_INSERT_TAIL(&hardware.h_lport.p_vlans, &vlan2, v_entries);
	TAILQ_INSERT_TAIL(&hardware.h_lport.p_vlans, &vlan3, v_entries);
	ppvid.p_cap_status = 3;
	ppvid.p_ppvid = 1500;
	TAILQ_INSERT_TAIL(&hardware.h_lport.p_ppvids, &ppvid, p_entries);
	pi1.p_pi = "IEEE Link Aggregration Control Protocol 802.3ad";
	pi1.p_pi_len = strlen(pi1.p_pi);
	pi2.p_pi = "IEEE Link Layer Discovery Protocol 802.1ab-2005";
	pi2.p_pi_len = strlen(pi2.p_pi);
	TAILQ_INSERT_TAIL(&hardware.h_lport.p_pids, &pi1, p_entries);
	TAILQ_INSERT_TAIL(&hardware.h_lport.p_pids, &pi2, p_entries);

	/* Build packet */
	n = lldp_send(&test_lldpd, &hardware);
	if (n != 0) {
		fail("unable to build packet");
		return;
	}
	if (TAILQ_EMPTY(&pkts)) {
		fail("no packets sent");
		return;
	}
	pkt = TAILQ_FIRST(&pkts);
	fail_unless(TAILQ_NEXT(pkt, next) == NULL, "more than one packet sent");

	/* decode the retrieved packet calling lldp_decode() */
	fail_unless(lldp_decode(NULL, pkt->data, pkt->size, &hardware,
		&nchassis, &nport) != -1);
	if (!nchassis || !nport) {
		fail("unable to decode packet");
		return;
	}

	/* verify port values */
	check_received_port(&hardware.h_lport, nport);
	/* verify chassis values */
	check_received_chassis(&chassis, nchassis);

	if (TAILQ_EMPTY(&nport->p_vlans)) {
		fail("no VLAN");
		return;
	}

	rvlan = TAILQ_FIRST(&nport->p_vlans);
	ck_assert_int_eq(rvlan->v_vid, vlan1.v_vid);
	ck_assert_str_eq(rvlan->v_name, vlan1.v_name);

	rvlan = TAILQ_NEXT(rvlan, v_entries);
	if (!rvlan) {
		fail("no more VLAN");
		return;
	}
	ck_assert_int_eq(rvlan->v_vid, vlan2.v_vid);
	ck_assert_str_eq(rvlan->v_name, vlan2.v_name);

	rvlan = TAILQ_NEXT(rvlan, v_entries);
	if (!rvlan) {
		fail("no more VLAN");
		return;
	}
	ck_assert_int_eq(rvlan->v_vid, vlan3.v_vid);
	ck_assert_str_eq(rvlan->v_name, vlan3.v_name);

	rvlan = TAILQ_NEXT(rvlan, v_entries);
	fail_unless(rvlan == NULL);

	ck_assert_int_eq(nport->p_pvid, hardware.h_lport.p_pvid);

	if (TAILQ_EMPTY(&nport->p_ppvids)) {
		fail("no Port Protocal VLAN ID");
		return;
	}
	rppvid = TAILQ_FIRST(&nport->p_ppvids);
	ck_assert_int_eq(rppvid->p_cap_status, ppvid.p_cap_status);
	ck_assert_int_eq(rppvid->p_ppvid, ppvid.p_ppvid);
	
	if (TAILQ_EMPTY(&nport->p_pids)) {
		fail("no Protocal Identity TLV");
		return;
	}
	rpi = TAILQ_FIRST(&nport->p_pids);
	ck_assert_int_eq(rpi->p_pi_len, pi1.p_pi_len);
	ck_assert_str_eq_n(rpi->p_pi, pi1.p_pi, pi1.p_pi_len);

	rpi = TAILQ_NEXT(rpi, p_entries);
	if (!rpi) {
		fail("no more Protocol Identity TLVs");
		return;
	}
	ck_assert_int_eq(rpi->p_pi_len, pi2.p_pi_len);
	ck_assert_str_eq_n(rpi->p_pi, pi2.p_pi, pi2.p_pi_len);

	rpi = TAILQ_NEXT(rpi, p_entries);
	fail_unless(rpi == NULL);
}
END_TEST
#endif

#ifdef ENABLE_LLDPMED
START_TEST (test_send_rcv_med)
{
	int n;
	struct packet *pkt;
	struct lldpd_chassis *nchassis = NULL;
	struct lldpd_port *nport = NULL;

	/* Populate port and chassis */
	hardware.h_lport.p_id_subtype = LLDP_PORTID_SUBTYPE_LLADDR;
	hardware.h_lport.p_id = macaddress;
	hardware.h_lport.p_id_len = ETHER_ADDR_LEN;
	hardware.h_lport.p_descr = "Fake port description";
	hardware.h_lport.p_mfs = 1516;
	chassis.c_id_subtype = LLDP_CHASSISID_SUBTYPE_LOCAL;
	chassis.c_id = "Chassis name";
	chassis.c_id_len = strlen(chassis.c_id);
	chassis.c_name = "Third chassis";
	chassis.c_descr = "Chassis description";
	chassis.c_cap_available = LLDP_CAP_ROUTER | LLDP_CAP_BRIDGE;
	chassis.c_cap_enabled = LLDP_CAP_ROUTER;
	chassis.c_med_cap_available = LLDP_MED_CAP_CAP | LLDP_MED_CAP_POLICY |
		LLDP_MED_CAP_LOCATION | LLDP_MED_CAP_MDI_PSE |
		LLDP_MED_CAP_IV;
	chassis.c_med_type = LLDP_MED_CLASS_III;
	chassis.c_med_hw = "hardware rev 5";
	chassis.c_med_fw = "47b5";
	chassis.c_med_sw = "2.6.22b5";
	chassis.c_med_sn = "SN 47842";
	hardware.h_lport.p_med_cap_enabled = chassis.c_med_cap_available;
	hardware.h_lport.p_med_location[LLDP_MED_LOCFORMAT_CIVIC-1].format =
		LLDP_MED_LOCFORMAT_CIVIC;
	hardware.h_lport.p_med_location[LLDP_MED_LOCFORMAT_CIVIC-1].data = "Your favorite city";
	hardware.h_lport.p_med_location[LLDP_MED_LOCFORMAT_CIVIC-1].data_len = 
		sizeof("Your favorite city");
	hardware.h_lport.p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].type =
		LLDP_MED_APPTYPE_SOFTPHONEVOICE;
	hardware.h_lport.p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].tagged =
		1;
	hardware.h_lport.p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].vid =
		51;
	hardware.h_lport.p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].priority =
		6;
	hardware.h_lport.p_med_policy[LLDP_MED_APPTYPE_SOFTPHONEVOICE-1].dscp =
		46;
	hardware.h_lport.p_med_power.devicetype = LLDP_MED_POW_TYPE_PSE;
	hardware.h_lport.p_med_power.source = LLDP_MED_POW_SOURCE_PRIMARY;
	hardware.h_lport.p_med_power.priority = LLDP_MED_POW_PRIO_HIGH;
	hardware.h_lport.p_med_power.val = 65;

	/* Build packet */
	n = lldp_send(&test_lldpd, &hardware);
	if (n != 0) {
		fail("unable to build packet");
		return;
	}
	if (TAILQ_EMPTY(&pkts)) {
		fail("no packets sent");
		return;
	}
	pkt = TAILQ_FIRST(&pkts);
	fail_unless(TAILQ_NEXT(pkt, next) == NULL, "more than one packet sent");

	/* decode the retrieved packet calling lldp_decode() */
	fail_unless(lldp_decode(NULL, pkt->data, pkt->size, &hardware,
		&nchassis, &nport) != -1);
	if (!nchassis || !nport) {
		fail("unable to decode packet");
		return;
	}
	/* verify port values */
	check_received_port(&hardware.h_lport, nport);
	/* verify chassis values */
	check_received_chassis(&chassis, nchassis);

	/* veridfy med content */
	check_received_port_med(&hardware.h_lport, nport);
	check_received_chassis_med(&chassis, nchassis);
}
END_TEST
#endif

#ifdef ENABLE_DOT3
START_TEST (test_send_rcv_dot3)
{
	int n;
	struct lldpd_chassis *nchassis = NULL;
	struct lldpd_port *nport = NULL;
	struct packet *pkt;

	/* Populate port and chassis */
	hardware.h_lport.p_id_subtype = LLDP_PORTID_SUBTYPE_IFNAME;
	hardware.h_lport.p_id = "FastEthernet 1/5";
	hardware.h_lport.p_id_len = strlen(hardware.h_lport.p_id);
	hardware.h_lport.p_descr = "Fake port description";
	hardware.h_lport.p_mfs = 1516;
	hardware.h_lport.p_aggregid = 5;
	hardware.h_lport.p_macphy.autoneg_support = 1;
	hardware.h_lport.p_macphy.autoneg_enabled = 1;
	hardware.h_lport.p_macphy.autoneg_advertised = LLDP_DOT3_LINK_AUTONEG_10BASE_T |
		LLDP_DOT3_LINK_AUTONEG_10BASET_FD | LLDP_DOT3_LINK_AUTONEG_100BASE_TX |
		LLDP_DOT3_LINK_AUTONEG_100BASE_TXFD;
	hardware.h_lport.p_macphy.mau_type = LLDP_DOT3_MAU_100BASETXFD;
	chassis.c_id_subtype = LLDP_CHASSISID_SUBTYPE_LLADDR;
	chassis.c_id = macaddress;
	chassis.c_id_len = ETHER_ADDR_LEN;
	chassis.c_name = "Fourth chassis";
	chassis.c_descr = "Long chassis description";
	chassis.c_cap_available = chassis.c_cap_enabled = LLDP_CAP_ROUTER | LLDP_CAP_WLAN;

	/* Build packet */
	n = lldp_send(&test_lldpd, &hardware);
	if (n != 0) {
		fail("unable to build packet");
		return;
	}
	if (TAILQ_EMPTY(&pkts)) {
		fail("no packets sent");
		return;
	}
	pkt = TAILQ_FIRST(&pkts);
	fail_unless(TAILQ_NEXT(pkt, next) == NULL, "more than one packet sent");

	/* decode the retrieved packet calling lldp_decode() */
	fail_unless(lldp_decode(NULL, pkt->data, pkt->size, &hardware,
		&nchassis, &nport) != -1);
	if (!nchassis || !nport) {
		fail("unable to decode packet");
		return;
	}
	/* verify port values */
	check_received_port(&hardware.h_lport, nport);
	/* verify chassis values */
	check_received_chassis(&chassis, nchassis);
	/* verify dot3 values */
	check_received_port_dot3(&hardware.h_lport, nport);
}
END_TEST
#endif

START_TEST (test_recv_min)
{
	char pkt1[] = {
		0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e, 0x00, 0x17,
		0xd1, 0xa8, 0x35, 0xbe, 0x88, 0xcc, 0x02, 0x07,
		0x04, 0x00, 0x17, 0xd1, 0xa8, 0x35, 0xbf, 0x04,
		0x07, 0x03, 0x00, 0x17, 0xd1, 0xa8, 0x36, 0x02,
		0x06, 0x02, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00 };
	/* This is:
Ethernet II, Src: Nortel_a8:35:be (00:17:d1:a8:35:be), Dst: LLDP_Multicast (01:80:c2:00:00:0e)
    Destination: LLDP_Multicast (01:80:c2:00:00:0e)
    Source: Nortel_a8:35:be (00:17:d1:a8:35:be)
    Type: 802.1 Link Layer Discovery Protocol (LLDP) (0x88cc)
Link Layer Discovery Protocol
    Chassis Subtype = MAC address
        0000 001. .... .... = TLV Type: Chassis Id (1)
        .... ...0 0000 0111 = TLV Length: 7
        Chassis Id Subtype: MAC address (4)
        Chassis Id: Nortel_a8:35:bf (00:17:d1:a8:35:bf)
    Port Subtype = MAC address
        0000 010. .... .... = TLV Type: Port Id (2)
        .... ...0 0000 0111 = TLV Length: 7
        Port Id Subtype: MAC address (3)
        Port Id: Nortel_a8:36:02 (00:17:d1:a8:36:02)
    Time To Live = 120 sec
        0000 011. .... .... = TLV Type: Time to Live (3)
        .... ...0 0000 0010 = TLV Length: 2
        Seconds: 120
    End of LLDPDU
        0000 000. .... .... = TLV Type: End of LLDPDU (0)
        .... ...0 0000 0000 = TLV Length: 0
	*/
	struct lldpd_chassis *nchassis = NULL;
	struct lldpd_port *nport = NULL;
	char mac1[] = { 0x0, 0x17, 0xd1, 0xa8, 0x35, 0xbf };
	char mac2[] = { 0x0, 0x17, 0xd1, 0xa8, 0x36, 0x02 };

	fail_unless(lldp_decode(NULL, pkt1, sizeof(pkt1), &hardware,
		&nchassis, &nport) != -1);
	if (!nchassis || !nport) {
		fail("unable to decode packet");
		return;
	}
	ck_assert_int_eq(nchassis->c_id_subtype,
	    LLDP_CHASSISID_SUBTYPE_LLADDR);
	ck_assert_int_eq(nchassis->c_id_len, ETHER_ADDR_LEN);
	fail_unless(memcmp(mac1, nchassis->c_id, ETHER_ADDR_LEN) == 0);
	ck_assert_int_eq(nport->p_id_subtype,
	    LLDP_PORTID_SUBTYPE_LLADDR);
	ck_assert_int_eq(nport->p_id_len, ETHER_ADDR_LEN);
	fail_unless(memcmp(mac2, nport->p_id, ETHER_ADDR_LEN) == 0);
	ck_assert_ptr_eq(nchassis->c_name, NULL);
	ck_assert_ptr_eq(nchassis->c_descr, NULL);
	ck_assert_ptr_eq(nport->p_descr, NULL);
	ck_assert_int_eq(nport->p_ttl, 120);
}
END_TEST

START_TEST (test_recv_lldpd)
{
	/* This is a frame generated by lldpd */
	char pkt1[] = {
		0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e, 0x00, 0x16,
		0x17, 0x2f, 0xa1, 0xb6, 0x88, 0xcc, 0x02, 0x07,
		0x04, 0x00, 0x16, 0x17, 0x2f, 0xa1, 0xb6, 0x04,
		0x07, 0x03, 0x00, 0x16, 0x17, 0x2f, 0xa1, 0xb6,
		0x06, 0x02, 0x00, 0x78, 0x0a, 0x1a, 0x6e, 0x61,
		0x72, 0x75, 0x74, 0x6f, 0x2e, 0x58, 0x58, 0x58,
		0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
		0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
		0x0c, 0x3f, 0x4c, 0x69, 0x6e, 0x75, 0x78, 0x20,
		0x32, 0x2e, 0x36, 0x2e, 0x32, 0x39, 0x2d, 0x32,
		0x2d, 0x61, 0x6d, 0x64, 0x36, 0x34, 0x20, 0x23,
		0x31, 0x20, 0x53, 0x4d, 0x50, 0x20, 0x53, 0x75,
		0x6e, 0x20, 0x4d, 0x61, 0x79, 0x20, 0x31, 0x37,
		0x20, 0x31, 0x37, 0x3a, 0x31, 0x35, 0x3a, 0x34,
		0x37, 0x20, 0x55, 0x54, 0x43, 0x20, 0x32, 0x30,
		0x30, 0x39, 0x20, 0x78, 0x38, 0x36, 0x5f, 0x36,
		0x34, 0x0e, 0x04, 0x00, 0x1c, 0x00, 0x14, 0x10,
		0x0c, 0x05, 0x01, 0x0a, 0xee, 0x50, 0x4b, 0x02,
		0x00, 0x00, 0x00, 0x03, 0x00, 0x08, 0x04, 0x65,
		0x74, 0x68, 0x30, 0xfe, 0x09, 0x00, 0x12, 0x0f,
		0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x09,
		0x00, 0x12, 0x0f, 0x01, 0x03, 0x6c, 0x03, 0x00,
		0x10, 0xfe, 0x06, 0x00, 0x12, 0x0f, 0x04, 0x05,
		0xdc, 0xfe, 0x07, 0x00, 0x12, 0xbb, 0x01, 0x00,
		0x00, 0x00, 0xfe, 0x0f, 0x00, 0x12, 0xbb, 0x05,
		0x4e, 0x44, 0x39, 0x39, 0x31, 0x37, 0x38, 0x39,
		0x37, 0x30, 0x32, 0xfe, 0x0b, 0x00, 0x12, 0xbb,
		0x06, 0x30, 0x38, 0x30, 0x30, 0x31, 0x32, 0x20,
		0xfe, 0x12, 0x00, 0x12, 0xbb, 0x07, 0x32, 0x2e,
		0x36, 0x2e, 0x32, 0x39, 0x2d, 0x32, 0x2d, 0x61,
		0x6d, 0x64, 0x36, 0x34, 0xfe, 0x10, 0x00, 0x12,
		0xbb, 0x08, 0x31, 0x30, 0x35, 0x38, 0x32, 0x30,
		0x38, 0x35, 0x30, 0x30, 0x30, 0x39, 0xfe, 0x15,
		0x00, 0x12, 0xbb, 0x09, 0x4e, 0x45, 0x43, 0x20,
		0x43, 0x6f, 0x6d, 0x70, 0x75, 0x74, 0x65, 0x72,
		0x73, 0x20, 0x53, 0x41, 0x53, 0xfe, 0x13, 0x00,
		0x12, 0xbb, 0x0a, 0x50, 0x4f, 0x57, 0x45, 0x52,
		0x4d, 0x41, 0x54, 0x45, 0x20, 0x56, 0x4c, 0x33,
		0x35, 0x30, 0xfe, 0x0d, 0x00, 0x12, 0xbb, 0x0b,
		0x31, 0x30, 0x30, 0x32, 0x30, 0x37, 0x31, 0x32,
		0x30, 0x00, 0x00 };
	/* This is:
Ethernet II, Src: Msi_2f:a1:b6 (00:16:17:2f:a1:b6), Dst: LLDP_Multicast (01:80:c2:00:00:0e)
    Destination: LLDP_Multicast (01:80:c2:00:00:0e)
    Source: Msi_2f:a1:b6 (00:16:17:2f:a1:b6)
    Type: 802.1 Link Layer Discovery Protocol (LLDP) (0x88cc)
Link Layer Discovery Protocol
    Chassis Subtype = MAC address
        0000 001. .... .... = TLV Type: Chassis Id (1)
        .... ...0 0000 0111 = TLV Length: 7
        Chassis Id Subtype: MAC address (4)
        Chassis Id: Msi_2f:a1:b6 (00:16:17:2f:a1:b6)
    Port Subtype = MAC address
        0000 010. .... .... = TLV Type: Port Id (2)
        .... ...0 0000 0111 = TLV Length: 7
        Port Id Subtype: MAC address (3)
        Port Id: Msi_2f:a1:b6 (00:16:17:2f:a1:b6)
    Time To Live = 120 sec
        0000 011. .... .... = TLV Type: Time to Live (3)
        .... ...0 0000 0010 = TLV Length: 2
        Seconds: 120
    System Name = naruto.XXXXXXXXXXXXXXXXXXX
        0000 101. .... .... = TLV Type: System Name (5)
        .... ...0 0001 1010 = TLV Length: 26
        System Name = naruto.bureau.b1.p.fti.net
    System Description = Linux 2.6.29-2-amd64 #1 SMP Sun May 17 17:15:47 UTC 2009 x86_64
        0000 110. .... .... = TLV Type: System Description (6)
        .... ...0 0011 1111 = TLV Length: 63
        System Description = Linux 2.6.29-2-amd64 #1 SMP Sun May 17 17:15:47 UTC 2009 x86_64
    Capabilities
        0000 111. .... .... = TLV Type: System Capabilities (7)
        .... ...0 0000 0100 = TLV Length: 4
        Capabilities: 0x001c
            .... .... .... .1.. = Bridge
            .... .... .... 1... = WLAN access point
            .... .... ...1 .... = Router
        Enabled Capabilities: 0x0014
            .... .... .... .1.. = Bridge
            .... .... ...1 .... = Router
    Management Address
        0001 000. .... .... = TLV Type: Management Address (8)
        .... ...0 0000 1100 = TLV Length: 12
        Address String Length: 5
        Address Subtype: IPv4 (1)
        Management Address: 10.238.80.75
        Interface Subtype: ifIndex (2)
        Interface Number: 3
        OID String Length: 0
    Port Description = eth0
        0000 100. .... .... = TLV Type: Port Description (4)
        .... ...0 0000 0100 = TLV Length: 4
        Port Description: eth0
    IEEE 802.3 - Link Aggregation
        1111 111. .... .... = TLV Type: Organization Specific (127)
        .... ...0 0000 1001 = TLV Length: 9
        Organization Unique Code: IEEE 802.3 (0x00120f)
        IEEE 802.3 Subtype: Link Aggregation (0x03)
        Aggregation Status: 0x01
            .... ...1 = Aggregation Capability: Yes
            .... ..0. = Aggregation Status: Not Enabled
        Aggregated Port Id: 0
    IEEE 802.3 - MAC/PHY Configuration/Status
        1111 111. .... .... = TLV Type: Organization Specific (127)
        .... ...0 0000 1001 = TLV Length: 9
        Organization Unique Code: IEEE 802.3 (0x00120f)
        IEEE 802.3 Subtype: MAC/PHY Configuration/Status (0x01)
        Auto-Negotiation Support/Status: 0x03
            .... ...1 = Auto-Negotiation: Supported
            .... ..1. = Auto-Negotiation: Enabled
        PMD Auto-Negotiation Advertised Capability: 0x6C03
            .... .... .... ...1 = 1000BASE-T (full duplex mode)
            .... .... .... ..1. = 1000BASE-T (half duplex mode)
            .... .1.. .... .... = 100BASE-TX (full duplex mode)
            .... 1... .... .... = 100BASE-TX (half duplex mode)
            ..1. .... .... .... = 10BASE-T (full duplex mode)
            .1.. .... .... .... = 10BASE-T (half duplex mode)
        Operational MAU Type: 100BaseTXFD - 2 pair category 5 UTP, full duplex mode (0x0010)
    IEEE 802.3 - Maximum Frame Size
        1111 111. .... .... = TLV Type: Organization Specific (127)
        .... ...0 0000 0110 = TLV Length: 6
        Organization Unique Code: IEEE 802.3 (0x00120f)
        IEEE 802.3 Subtype: Maximum Frame Size (0x04)
        Maximum Frame Size: 1500
    TIA - Media Capabilities
        1111 111. .... .... = TLV Type: Organization Specific (127)
        .... ...0 0000 0111 = TLV Length: 7
        Organization Unique Code: TIA (0x0012bb)
        Media Subtype: Media Capabilities (0x01)
        Capabilities: 0x0000
        Class Type: Type Not Defined
    TIA - Inventory - Hardware Revision
        1111 111. .... .... = TLV Type: Organization Specific (127)
        .... ...0 0000 1111 = TLV Length: 15
        Organization Unique Code: TIA (0x0012bb)
        Media Subtype: Inventory - Hardware Revision (0x05)
        Hardware Revision: ND991789702
    TIA - Inventory - Firmware Revision
        1111 111. .... .... = TLV Type: Organization Specific (127)
        .... ...0 0000 1011 = TLV Length: 10
        Organization Unique Code: TIA (0x0012bb)
        Media Subtype: Inventory - Firmware Revision (0x06)
        Firmware Revision: 080012
    TIA - Inventory - Software Revision
        1111 111. .... .... = TLV Type: Organization Specific (127)
        .... ...0 0001 0010 = TLV Length: 18
        Organization Unique Code: TIA (0x0012bb)
        Media Subtype: Inventory - Software Revision (0x07)
        Software Revision: 2.6.29-2-amd64
    TIA - Inventory - Serial Number
        1111 111. .... .... = TLV Type: Organization Specific (127)
        .... ...0 0001 0000 = TLV Length: 16
        Organization Unique Code: TIA (0x0012bb)
        Media Subtype: Inventory - Serial Number (0x08)
        Serial Number: 105820850009
    TIA - Inventory - Manufacturer Name
        1111 111. .... .... = TLV Type: Organization Specific (127)
        .... ...0 0001 0101 = TLV Length: 21
        Organization Unique Code: TIA (0x0012bb)
        Media Subtype: Inventory - Manufacturer Name (0x09)
        Manufacturer Name: NEC Computers SAS
    TIA - Inventory - Model Name
        1111 111. .... .... = TLV Type: Organization Specific (127)
        .... ...0 0001 0011 = TLV Length: 19
        Organization Unique Code: TIA (0x0012bb)
        Media Subtype: Inventory - Model Name (0x0a)
        Model Name: POWERMATE VL350
    TIA - Inventory - Asset ID
        1111 111. .... .... = TLV Type: Organization Specific (127)
        .... ...0 0000 1101 = TLV Length: 13
        Organization Unique Code: TIA (0x0012bb)
        Media Subtype: Inventory - Asset ID (0x0b)
        Asset ID: 100207120
    End of LLDPDU
        0000 000. .... .... = TLV Type: End of LLDPDU (0)
        .... ...0 0000 0000 = TLV Length: 0
	*/
	struct lldpd_chassis *nchassis = NULL;
	struct lldpd_port *nport = NULL;
	char mac1[] = { 0x00, 0x16, 0x17, 0x2f, 0xa1, 0xb6 };

	fail_unless(lldp_decode(NULL, pkt1, sizeof(pkt1), &hardware,
		&nchassis, &nport) != -1);
	if (!nchassis || !nport) {
		fail("unable to decode packet");
		return;
	}
	ck_assert_int_eq(nchassis->c_id_subtype,
	    LLDP_CHASSISID_SUBTYPE_LLADDR);
	ck_assert_int_eq(nchassis->c_id_len, ETHER_ADDR_LEN);
	fail_unless(memcmp(mac1, nchassis->c_id, ETHER_ADDR_LEN) == 0);
	ck_assert_int_eq(nport->p_id_subtype,
	    LLDP_PORTID_SUBTYPE_LLADDR);
	ck_assert_int_eq(nport->p_id_len, ETHER_ADDR_LEN);
	fail_unless(memcmp(mac1, nport->p_id, ETHER_ADDR_LEN) == 0);
	ck_assert_int_eq(nport->p_ttl, 120);
	ck_assert_str_eq(nchassis->c_name, "naruto.XXXXXXXXXXXXXXXXXXX");
	ck_assert_str_eq(nchassis->c_descr,
	    "Linux 2.6.29-2-amd64 #1 SMP Sun May 17 17:15:47 UTC 2009 x86_64");
	ck_assert_str_eq(nport->p_descr, "eth0");
	ck_assert_int_eq(nchassis->c_cap_available,
	    LLDP_CAP_WLAN | LLDP_CAP_ROUTER | LLDP_CAP_BRIDGE);
	ck_assert_int_eq(nchassis->c_cap_enabled,
	    LLDP_CAP_ROUTER | LLDP_CAP_BRIDGE);
	ck_assert_int_eq(nchassis->c_mgmt.tqh_first->m_addr.inet.s_addr,
	    (u_int32_t)inet_addr("10.238.80.75"));
	ck_assert_int_eq(nchassis->c_mgmt.tqh_first->m_iface, 3);
#ifdef ENABLE_DOT3
	ck_assert_int_eq(nport->p_aggregid, 0);
	ck_assert_int_eq(nport->p_macphy.autoneg_enabled, 1);
	ck_assert_int_eq(nport->p_macphy.autoneg_support, 1);
	ck_assert_int_eq(nport->p_macphy.autoneg_advertised,
	    LLDP_DOT3_LINK_AUTONEG_1000BASE_TFD |
	    LLDP_DOT3_LINK_AUTONEG_1000BASE_T |
	    LLDP_DOT3_LINK_AUTONEG_100BASE_TXFD |
	    LLDP_DOT3_LINK_AUTONEG_100BASE_TX |
	    LLDP_DOT3_LINK_AUTONEG_10BASET_FD |
	    LLDP_DOT3_LINK_AUTONEG_10BASE_T);
	ck_assert_int_eq(nport->p_macphy.mau_type,
	    LLDP_DOT3_MAU_100BASETXFD);
	ck_assert_int_eq(nport->p_mfs, 1500);
#endif
#ifdef ENABLE_LLDPMED
	ck_assert_int_eq(nchassis->c_med_type, 0);
	ck_assert_str_eq(nchassis->c_med_hw, "ND991789702");
	ck_assert_str_eq(nchassis->c_med_fw, "080012 "); /* Extra space */
	ck_assert_str_eq(nchassis->c_med_sw, "2.6.29-2-amd64");
	ck_assert_str_eq(nchassis->c_med_sn, "105820850009");
	ck_assert_str_eq(nchassis->c_med_manuf, "NEC Computers SAS");
	ck_assert_str_eq(nchassis->c_med_model, "POWERMATE VL350");
	ck_assert_str_eq(nchassis->c_med_asset, "100207120");
#endif
}
END_TEST

Suite *
lldp_suite(void)
{
	Suite *s = suite_create("LLDP");
	TCase *tc_send = tcase_create("Send LLDP packets");
	TCase *tc_receive = tcase_create("Receive LLDP packets");

	/* Send tests are first run without knowing the result. The
	   result is then checked with:
	     tshark -V -T text -r tests/lldp_send_0000.pcap

	   If the result is correct, then, we get the packet as C
	   bytes using wireshark export to C arrays (tshark seems not
	   be able to do this).
	*/

	tcase_add_checked_fixture(tc_send, pcap_setup, pcap_teardown);
	tcase_add_test(tc_send, test_send_rcv_basic);
	tcase_add_test(tc_send, test_send_rcv_vlan_tx);
#ifdef ENABLE_DOT1
	tcase_add_test(tc_send, test_send_rcv_dot1_tlvs);
#endif
#ifdef ENABLE_LLDPMED
	tcase_add_test(tc_send, test_send_rcv_med);
#endif
#ifdef ENABLE_DOT3
	tcase_add_test(tc_send, test_send_rcv_dot3);
#endif
	suite_add_tcase(s, tc_send);

	tcase_add_test(tc_receive, test_recv_min);
	tcase_add_test(tc_receive, test_recv_lldpd);
	suite_add_tcase(s, tc_receive);

	return s;
}

int
main()
{
	int number_failed;
	Suite *s = lldp_suite ();
	SRunner *sr = srunner_create (s);
	srunner_set_fork_status (sr, CK_NOFORK); /* Can't fork because
						    we need to write
						    files */
	srunner_run_all (sr, CK_ENV);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
