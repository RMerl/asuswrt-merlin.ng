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

char filenameprefix[] = "sonmp_send";

#ifdef ENABLE_SONMP

START_TEST (test_send_sonmp)
{
	int n;
	/* Packet we should build:
IEEE 802.3 Ethernet 
    Destination: Bay-Networks-(Synoptics)-autodiscovery (01:00:81:00:01:00)
    Source: 5e:10:8e:e7:84:ad (5e:10:8e:e7:84:ad)
    Length: 22
Logical-Link Control
    DSAP: SNAP (0xaa)
    IG Bit: Individual
    SSAP: SNAP (0xaa)
    CR Bit: Command
    Control field: U, func=UI (0x03)
        000. 00.. = Command: Unnumbered Information (0x00)
        .... ..11 = Frame type: Unnumbered frame (0x03)
    Organization Code: Nortel Networks SONMP (0x000081)
    PID: SONMP segment hello (0x01a2)
Nortel Networks / SynOptics Network Management Protocol
    NMM IP address: 172.17.142.37 (172.17.142.37)
    Segment Identifier: 0x000004
    Chassis type: Unknown (1)
    Backplane type: ethernet, fast ethernet and gigabit ethernet (12)
    NMM state: New (3)
    Number of links: 1

IEEE 802.3 Ethernet 
    Destination: Bay-Networks-(Synoptics)-autodiscovery (01:00:81:00:01:01)
    Source: 5e:10:8e:e7:84:ad (5e:10:8e:e7:84:ad)
    Length: 22
Logical-Link Control
    DSAP: SNAP (0xaa)
    IG Bit: Individual
    SSAP: SNAP (0xaa)
    CR Bit: Command
    Control field: U, func=UI (0x03)
        000. 00.. = Command: Unnumbered Information (0x00)
        .... ..11 = Frame type: Unnumbered frame (0x03)
    Organization Code: Nortel Networks SONMP (0x000081)
    PID: SONMP flatnet hello (0x01a1)
Nortel Networks / SynOptics Network Management Protocol
    NMM IP address: 172.17.142.37 (172.17.142.37)
    Segment Identifier: 0x000004
    Chassis type: Unknown (1)
    Backplane type: ethernet, fast ethernet and gigabit ethernet (12)
    NMM state: New (3)
    Number of links: 1
	*/
	char pkt1[] = {
		0x01, 0x00, 0x81, 0x00, 0x01, 0x00, 0x5e, 0x10,
		0x8e, 0xe7, 0x84, 0xad, 0x00, 0x16, 0xaa, 0xaa,
		0x03, 0x00, 0x00, 0x81, 0x01, 0xa2, 0xac, 0x11,
		0x8e, 0x25, 0x00, 0x00, 0x04, 0x01, 0x0c, 0x03,
		0x01 };
	char pkt2[] = {
		0x01, 0x00, 0x81, 0x00, 0x01, 0x01, 0x5e, 0x10,
		0x8e, 0xe7, 0x84, 0xad, 0x00, 0x16, 0xaa, 0xaa,
		0x03, 0x00, 0x00, 0x81, 0x01, 0xa1, 0xac, 0x11,
		0x8e, 0x25, 0x00, 0x00, 0x04, 0x01, 0x0c, 0x03,
		0x01 };
	struct packet *pkt;
	in_addr_t addr;	
	struct lldpd_mgmt *mgmt;

	/* Populate port and chassis */
	hardware.h_lport.p_id_subtype = LLDP_PORTID_SUBTYPE_IFNAME;
	hardware.h_lport.p_id = "Not used";
	hardware.h_lport.p_id_len = strlen(hardware.h_lport.p_id);
	chassis.c_id_subtype = LLDP_CHASSISID_SUBTYPE_LLADDR;
	chassis.c_id = macaddress;
	chassis.c_id_len = ETHER_ADDR_LEN;
	TAILQ_INIT(&chassis.c_mgmt);
	addr = inet_addr("172.17.142.37");
	mgmt = lldpd_alloc_mgmt(LLDPD_AF_IPV4, 
				&addr, sizeof(in_addr_t), 0);
	if (mgmt == NULL)
		ck_abort();
	TAILQ_INSERT_TAIL(&chassis.c_mgmt, mgmt, m_entries);

	/* Build packet */
	n = sonmp_send(NULL, &hardware);
	if (n != 0) {
		fail("unable to build packet");
		return;
	}
	if (TAILQ_EMPTY(&pkts)) {
		fail("no packets sent");
		return;
	}
	pkt = TAILQ_FIRST(&pkts);
	ck_assert_int_eq(pkt->size, sizeof(pkt1));
	fail_unless(memcmp(pkt->data, pkt1, sizeof(pkt1)) == 0);
	pkt = TAILQ_NEXT(pkt, next);
	if (!pkt) {
		fail("need one more packet");
		return;
	}
	ck_assert_int_eq(pkt->size, sizeof(pkt2));
	fail_unless(memcmp(pkt->data, pkt2, sizeof(pkt2)) == 0);
	fail_unless(TAILQ_NEXT(pkt, next) == NULL, "more than two packets sent");
}
END_TEST

START_TEST (test_recv_sonmp)
{
	char pkt1[] = {
		0x01, 0x00, 0x81, 0x00, 0x01, 0x00, 0x00, 0x1b,
		0x25, 0x08, 0x50, 0x47, 0x00, 0x13, 0xaa, 0xaa,
		0x03, 0x00, 0x00, 0x81, 0x01, 0xa2, 0xac, 0x10,
		0x65, 0xa8, 0x00, 0x02, 0x08, 0x38, 0x0c, 0x02,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00 };
	/* This is:
IEEE 802.3 Ethernet 
    Destination: Bay-Networks-(Synoptics)-autodiscovery (01:00:81:00:01:00)
    Source: Nortel_08:50:47 (00:1b:25:08:50:47)
    Length: 19
    Trailer: 000000000000000000000000000000000000000000000000...
Logical-Link Control
    DSAP: SNAP (0xaa)
    IG Bit: Individual
    SSAP: SNAP (0xaa)
    CR Bit: Command
    Control field: U, func=UI (0x03)
        000. 00.. = Command: Unnumbered Information (0x00)
        .... ..11 = Frame type: Unnumbered frame (0x03)
    Organization Code: Nortel Networks SONMP (0x000081)
    PID: SONMP segment hello (0x01a2)
Nortel Networks / SynOptics Network Management Protocol
    NMM IP address: 172.16.101.168 (172.16.101.168)
    Segment Identifier: 0x000208
    Chassis type: Accelar 8610 L3 switch (56)
    Backplane type: ethernet, fast ethernet and gigabit ethernet (12)
    NMM state: Heartbeat (2)
    Number of links: 1
	*/
	struct lldpd_chassis *nchassis = NULL;
	struct lldpd_port *nport = NULL;
	char cid[5];
	in_addr_t ip;

	fail_unless(sonmp_decode(NULL, pkt1, sizeof(pkt1), &hardware,
		&nchassis, &nport) != -1);
	if (!nchassis || !nport) {
		fail("unable to decode packet");
		return;
	}
	ck_assert_int_eq(nchassis->c_id_subtype,
	    LLDP_CHASSISID_SUBTYPE_ADDR);
	ck_assert_int_eq(nchassis->c_id_len, 5);
	cid[0] = 1;
	ip = inet_addr("172.16.101.168");
	memcpy(cid + 1, &ip, sizeof(in_addr_t));
	fail_unless(memcmp(nchassis->c_id, cid, 5) == 0);
	ck_assert_str_eq(nchassis->c_name, "172.16.101.168");
	ck_assert_str_eq(nchassis->c_descr, "Nortel Ethernet Routing 8610 L3 Switch");
	ck_assert_int_eq(TAILQ_FIRST(&nchassis->c_mgmt)->m_addr.inet.s_addr,
	    (u_int32_t)inet_addr("172.16.101.168"));
	ck_assert_int_eq(TAILQ_FIRST(&nchassis->c_mgmt)->m_iface, 0);
	ck_assert_str_eq(nport->p_descr, "port 2/8");
	ck_assert_int_eq(nport->p_id_subtype,
	    LLDP_PORTID_SUBTYPE_LOCAL);
	ck_assert_int_eq(nport->p_id_len, strlen("00-02-08"));
	fail_unless(memcmp(nport->p_id,
		"00-02-08", strlen("00-02-08")) == 0);
	ck_assert_int_eq(nchassis->c_cap_enabled, 0);
}
END_TEST

#endif

Suite *
sonmp_suite(void)
{
	Suite *s = suite_create("SONMP");

#ifdef ENABLE_SONMP
	TCase *tc_send = tcase_create("Send SONMP packets");
	TCase *tc_receive = tcase_create("Receive SONMP packets");

	tcase_add_checked_fixture(tc_send, pcap_setup, pcap_teardown);
	tcase_add_test(tc_send, test_send_sonmp);
	suite_add_tcase(s, tc_send);

	tcase_add_test(tc_receive, test_recv_sonmp);
	suite_add_tcase(s, tc_receive);
#endif

	return s;
}

int
main()
{
	int number_failed;
	Suite *s = sonmp_suite ();
	SRunner *sr = srunner_create (s);
	srunner_set_fork_status (sr, CK_NOFORK); /* Can't fork because
						    we need to write
						    files */
	srunner_run_all (sr, CK_ENV);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
