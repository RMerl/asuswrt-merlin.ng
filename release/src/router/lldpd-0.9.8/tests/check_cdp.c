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

char filenameprefix[] = "cdp_send";

#ifdef ENABLE_CDP

START_TEST (test_send_cdpv1)
{
	int n;
	/* Packet we should build:
IEEE 802.3 Ethernet 
    Destination: CDP/VTP/DTP/PAgP/UDLD (01:00:0c:cc:cc:cc)
    Source: 5e:10:8e:e7:84:ad (5e:10:8e:e7:84:ad)
    Length: 106
Logical-Link Control
    DSAP: SNAP (0xaa)
    IG Bit: Individual
    SSAP: SNAP (0xaa)
    CR Bit: Command
    Control field: U, func=UI (0x03)
        000. 00.. = Command: Unnumbered Information (0x00)
        .... ..11 = Frame type: Unnumbered frame (0x03)
    Organization Code: Cisco (0x00000c)
    PID: CDP (0x2000)
Cisco Discovery Protocol
    Version: 1
    TTL: 180 seconds
    Checksum: 0x3af7 [correct]
        [Good: True]
        [Bad : False]
    Device ID: First chassis
        Type: Device ID (0x0001)
        Length: 17
        Device ID: First chassis
    Addresses
        Type: Addresses (0x0002)
        Length: 17
        Number of addresses: 1
        IP address: 172.17.142.37
            Protocol type: NLPID
            Protocol length: 1
            Protocol: IP
            Address length: 4
            IP address: 172.17.142.37
    Port ID: FastEthernet 1/5
        Type: Port ID (0x0003)
        Length: 20
        Sent through Interface: FastEthernet 1/5
    Capabilities
        Type: Capabilities (0x0004)
        Length: 8
        Capabilities: 0x00000011
            .... .... .... .... .... .... .... ...1 = Is  a Router
            .... .... .... .... .... .... .... ..0. = Not a Transparent Bridge
            .... .... .... .... .... .... .... .0.. = Not a Source Route Bridge
            .... .... .... .... .... .... .... 0... = Not a Switch
            .... .... .... .... .... .... ...1 .... = Is  a Host
            .... .... .... .... .... .... ..0. .... = Not IGMP capable
            .... .... .... .... .... .... .0.. .... = Not a Repeater
    Software Version
        Type: Software version (0x0005)
        Length: 23
        Software Version: Chassis description
    Platform: Linux
        Type: Platform (0x0006)
        Length: 9
        Platform: Linux
	*/
	char pkt1[] = {
	  0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc, 0x5e, 0x10,
	  0x8e, 0xe7, 0x84, 0xad, 0x00, 0x6a, 0xaa, 0xaa,
	  0x03, 0x00, 0x00, 0x0c, 0x20, 0x00, 0x01, 0xb4,
	  0x3a, 0xf7, 0x00, 0x01, 0x00, 0x11, 0x46, 0x69,
	  0x72, 0x73, 0x74, 0x20, 0x63, 0x68, 0x61, 0x73,
	  0x73, 0x69, 0x73, 0x00, 0x02, 0x00, 0x11, 0x00,
	  0x00, 0x00, 0x01, 0x01, 0x01, 0xcc, 0x00, 0x04,
	  0xac, 0x11, 0x8e, 0x25, 0x00, 0x03, 0x00, 0x14,
	  0x46, 0x61, 0x73, 0x74, 0x45, 0x74, 0x68, 0x65,
	  0x72, 0x6e, 0x65, 0x74, 0x20, 0x31, 0x2f, 0x35,
	  0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x11,
	  0x00, 0x05, 0x00, 0x17, 0x43, 0x68, 0x61, 0x73,
	  0x73, 0x69, 0x73, 0x20, 0x64, 0x65, 0x73, 0x63,
	  0x72, 0x69, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x00,
	  0x06, 0x00, 0x09, 0x4c, 0x69, 0x6e, 0x75, 0x78 };
	struct packet *pkt;
	in_addr_t addr;
	struct lldpd_mgmt *mgmt;
	struct lldpd cfg = {
		.g_config = {
			.c_ttl = 180,
			.c_platform = "Linux"
		}
	};

	/* Populate port and chassis */
	hardware.h_lport.p_id_subtype = LLDP_PORTID_SUBTYPE_IFNAME;
	hardware.h_lport.p_id = "Not used";
	hardware.h_lport.p_id_len = strlen(hardware.h_lport.p_id);
	hardware.h_lport.p_descr = "FastEthernet 1/5";
	chassis.c_id_subtype = LLDP_CHASSISID_SUBTYPE_LLADDR;
	chassis.c_id = macaddress;
	chassis.c_id_len = ETHER_ADDR_LEN;
	chassis.c_name = "First chassis";
	chassis.c_descr = "Chassis description";
	chassis.c_cap_available = chassis.c_cap_enabled = LLDP_CAP_ROUTER;
	TAILQ_INIT(&chassis.c_mgmt);
	addr = inet_addr("172.17.142.37");
	mgmt = lldpd_alloc_mgmt(LLDPD_AF_IPV4, 
					&addr, sizeof(in_addr_t), 0);
	if (mgmt == NULL)
		ck_abort();
	TAILQ_INSERT_TAIL(&chassis.c_mgmt, mgmt, m_entries);

	/* Build packet */
	n = cdpv1_send(&cfg, &hardware);
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
	fail_unless(TAILQ_NEXT(pkt, next) == NULL, "more than one packet sent");
}
END_TEST

START_TEST (test_send_cdpv2)
{
	int n;
	/* Packet we should build:
IEEE 802.3 Ethernet 
    Destination: CDP/VTP/DTP/PAgP/UDLD (01:00:0c:cc:cc:cc)
    Source: 5e:10:8e:e7:84:ad (5e:10:8e:e7:84:ad)
 the factory default)
    Length: 111
Logical-Link Control
    DSAP: SNAP (0xaa)
    IG Bit: Individual
    SSAP: SNAP (0xaa)
    CR Bit: Command
    Control field: U, func=UI (0x03)
        000. 00.. = Command: Unnumbered Information (0x00)
        .... ..11 = Frame type: Unnumbered frame (0x03)
    Organization Code: Cisco (0x00000c)
    PID: CDP (0x2000)
Cisco Discovery Protocol
    Version: 2
    TTL: 180 seconds
    Checksum: 0x5926 [correct]
        [Good: True]
        [Bad : False]
    Device ID: Second chassis
        Type: Device ID (0x0001)
        Length: 18
        Device ID: Second chassis
    Addresses
        Type: Addresses (0x0002)
        Length: 26
        Number of addresses: 2
        IP address: 172.17.142.36
            Protocol type: NLPID
            Protocol length: 1
            Protocol: IP
            Address length: 4
            IP address: 172.17.142.36
        IP address: 172.17.142.38
            Protocol type: NLPID
            Protocol length: 1
            Protocol: IP
            Address length: 4
            IP address: 172.17.142.38
    Port ID: Gigabit Ethernet 5/8
        Type: Port ID (0x0003)
        Length: 24
        Sent through Interface: Gigabit Ethernet 5/8
    Capabilities
        Type: Capabilities (0x0004)
        Length: 8
        Capabilities: 0x00000019
            .... .... .... .... .... .... .... ...1 = Is  a Router
            .... .... .... .... .... .... .... ..0. = Not a Transparent Bridge
            .... .... .... .... .... .... .... .0.. = Not a Source Route Bridge
            .... .... .... .... .... .... .... 1... = Is  a Switch
            .... .... .... .... .... .... ...1 .... = Is  a Host
            .... .... .... .... .... .... ..0. .... = Not IGMP capable
            .... .... .... .... .... .... .0.. .... = Not a Repeater
    Software Version
        Type: Software version (0x0005)
        Length: 23
        Software Version: Chassis description
    Platform: Linux
        Type: Platform (0x0006)
        Length: 9
        Platform: Linux
	*/
	char pkt1[] = {
	  0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc, 0x5e, 0x10,
	  0x8e, 0xe7, 0x84, 0xad, 0x00, 0x78, 0xaa, 0xaa,
	  0x03, 0x00, 0x00, 0x0c, 0x20, 0x00, 0x02, 0xb4,
	  0xc8, 0x67, 0x00, 0x01, 0x00, 0x12, 0x53, 0x65,
	  0x63, 0x6f, 0x6e, 0x64, 0x20, 0x63, 0x68, 0x61,
	  0x73, 0x73, 0x69, 0x73, 0x00, 0x02, 0x00, 0x1a,
	  0x00, 0x00, 0x00, 0x02, 0x01, 0x01, 0xcc, 0x00,
	  0x04, 0xac, 0x11, 0x8e, 0x24, 0x01, 0x01, 0xcc,
	  0x00, 0x04, 0xac, 0x11, 0x8e, 0x26, 0x00, 0x03,
	  0x00, 0x18, 0x47, 0x69, 0x67, 0x61, 0x62, 0x69,
	  0x74, 0x20, 0x45, 0x74, 0x68, 0x65, 0x72, 0x6e,
	  0x65, 0x74, 0x20, 0x35, 0x2f, 0x38, 0x00, 0x04,
	  0x00, 0x08, 0x00, 0x00, 0x00, 0x19, 0x00, 0x05,
	  0x00, 0x17, 0x43, 0x68, 0x61, 0x73, 0x73, 0x69,
	  0x73, 0x20, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69,
	  0x70, 0x74, 0x69, 0x6f, 0x6e, 0x00, 0x06, 0x00,
	  0x09, 0x4c, 0x69, 0x6e, 0x75, 0x78 };
	struct packet *pkt;
	in_addr_t addr1;
	in_addr_t addr2;
	struct lldpd_mgmt *mgmt1;
	struct lldpd_mgmt *mgmt2;
	struct lldpd cfg = {
		.g_config = {
			.c_ttl = 180,
			.c_platform = "Linux"
		}
	};

	/* Populate port and chassis */
	hardware.h_lport.p_id_subtype = LLDP_PORTID_SUBTYPE_LLADDR;
	hardware.h_lport.p_id = macaddress;
	hardware.h_lport.p_id_len = ETHER_ADDR_LEN;
	hardware.h_lport.p_descr = "Gigabit Ethernet 5/8";
	chassis.c_id_subtype = LLDP_CHASSISID_SUBTYPE_LLADDR;
	chassis.c_id = macaddress;
	chassis.c_id_len = ETHER_ADDR_LEN;
	chassis.c_name = "Second chassis";
	chassis.c_descr = "Chassis description";
	chassis.c_cap_available = chassis.c_cap_enabled =
	  LLDP_CAP_ROUTER | LLDP_CAP_BRIDGE;
	TAILQ_INIT(&chassis.c_mgmt);
	addr1 = inet_addr("172.17.142.36");
	addr2 = inet_addr("172.17.142.38");
	mgmt1 = lldpd_alloc_mgmt(LLDPD_AF_IPV4, 
				 &addr1, sizeof(in_addr_t), 0);
	mgmt2 = lldpd_alloc_mgmt(LLDPD_AF_IPV4, 
				 &addr2, sizeof(in_addr_t), 0);
	if (mgmt1 == NULL || mgmt2 == NULL)
		ck_abort();
	TAILQ_INSERT_TAIL(&chassis.c_mgmt, mgmt1, m_entries);
	TAILQ_INSERT_TAIL(&chassis.c_mgmt, mgmt2, m_entries);

	/* Build packet */
	n = cdpv2_send(&cfg, &hardware);
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
	fail_unless(TAILQ_NEXT(pkt, next) == NULL, "more than one packet sent");
}
END_TEST

START_TEST (test_recv_cdpv1)
{
	char pkt1[] = {
		0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc, 0x00, 0xe0,
		0x1e, 0xd5, 0xd5, 0x15, 0x01, 0x1e, 0xaa, 0xaa,
		0x03, 0x00, 0x00, 0x0c, 0x20, 0x00, 0x01, 0xb4,
		0xdf, 0xf0, 0x00, 0x01, 0x00, 0x06, 0x52, 0x31,
		0x00, 0x02, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01,
		0x01, 0x01, 0xcc, 0x00, 0x04, 0xc0, 0xa8, 0x0a,
		0x01, 0x00, 0x03, 0x00, 0x0d, 0x45, 0x74, 0x68,
		0x65, 0x72, 0x6e, 0x65, 0x74, 0x30, 0x00, 0x04,
		0x00, 0x08, 0x00, 0x00, 0x00, 0x11, 0x00, 0x05,
		0x00, 0xd8, 0x43, 0x69, 0x73, 0x63, 0x6f, 0x20,
		0x49, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x65, 0x74,
		0x77, 0x6f, 0x72, 0x6b, 0x20, 0x4f, 0x70, 0x65,
		0x72, 0x61, 0x74, 0x69, 0x6e, 0x67, 0x20, 0x53,
		0x79, 0x73, 0x74, 0x65, 0x6d, 0x20, 0x53, 0x6f,
		0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x20, 0x0a,
		0x49, 0x4f, 0x53, 0x20, 0x28, 0x74, 0x6d, 0x29,
		0x20, 0x31, 0x36, 0x30, 0x30, 0x20, 0x53, 0x6f,
		0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x20, 0x28,
		0x43, 0x31, 0x36, 0x30, 0x30, 0x2d, 0x4e, 0x59,
		0x2d, 0x4c, 0x29, 0x2c, 0x20, 0x56, 0x65, 0x72,
		0x73, 0x69, 0x6f, 0x6e, 0x20, 0x31, 0x31, 0x2e,
		0x32, 0x28, 0x31, 0x32, 0x29, 0x50, 0x2c, 0x20,
		0x52, 0x45, 0x4c, 0x45, 0x41, 0x53, 0x45, 0x20,
		0x53, 0x4f, 0x46, 0x54, 0x57, 0x41, 0x52, 0x45,
		0x20, 0x28, 0x66, 0x63, 0x31, 0x29, 0x0a, 0x43,
		0x6f, 0x70, 0x79, 0x72, 0x69, 0x67, 0x68, 0x74,
		0x20, 0x28, 0x63, 0x29, 0x20, 0x31, 0x39, 0x38,
		0x36, 0x2d, 0x31, 0x39, 0x39, 0x38, 0x20, 0x62,
		0x79, 0x20, 0x63, 0x69, 0x73, 0x63, 0x6f, 0x20,
		0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x73, 0x2c,
		0x20, 0x49, 0x6e, 0x63, 0x2e, 0x0a, 0x43, 0x6f,
		0x6d, 0x70, 0x69, 0x6c, 0x65, 0x64, 0x20, 0x54,
		0x75, 0x65, 0x20, 0x30, 0x33, 0x2d, 0x4d, 0x61,
		0x72, 0x2d, 0x39, 0x38, 0x20, 0x30, 0x36, 0x3a,
		0x33, 0x33, 0x20, 0x62, 0x79, 0x20, 0x64, 0x73,
		0x63, 0x68, 0x77, 0x61, 0x72, 0x74, 0x00, 0x06,
		0x00, 0x0e, 0x63, 0x69, 0x73, 0x63, 0x6f, 0x20,
		0x31, 0x36, 0x30, 0x31 };
	/* This is:
IEEE 802.3 Ethernet 
    Destination: CDP/VTP/DTP/PAgP/UDLD (01:00:0c:cc:cc:cc)
    Source: Cisco_d5:d5:15 (00:e0:1e:d5:d5:15)
    Length: 286
Logical-Link Control
    DSAP: SNAP (0xaa)
    IG Bit: Individual
    SSAP: SNAP (0xaa)
    CR Bit: Command
    Control field: U, func=UI (0x03)
        000. 00.. = Command: Unnumbered Information (0x00)
        .... ..11 = Frame type: Unnumbered frame (0x03)
    Organization Code: Cisco (0x00000c)
    PID: CDP (0x2000)
Cisco Discovery Protocol
    Version: 1
    TTL: 180 seconds
    Checksum: 0xdff0 [correct]
        [Good: True]
        [Bad : False]
    Device ID: R1
        Type: Device ID (0x0001)
        Length: 6
        Device ID: R1
    Addresses
        Type: Addresses (0x0002)
        Length: 17
        Number of addresses: 1
        IP address: 192.168.10.1
            Protocol type: NLPID
            Protocol length: 1
            Protocol: IP
            Address length: 4
            IP address: 192.168.10.1
    Port ID: Ethernet0
        Type: Port ID (0x0003)
        Length: 13
        Sent through Interface: Ethernet0
    Capabilities
        Type: Capabilities (0x0004)
        Length: 8
        Capabilities: 0x00000011
            .... .... .... .... .... .... .... ...1 = Is  a Router
            .... .... .... .... .... .... .... ..0. = Not a Transparent Bridge
            .... .... .... .... .... .... .... .0.. = Not a Source Route Bridge
            .... .... .... .... .... .... .... 0... = Not a Switch
            .... .... .... .... .... .... ...1 .... = Is  a Host
            .... .... .... .... .... .... ..0. .... = Not IGMP capable
            .... .... .... .... .... .... .0.. .... = Not a Repeater
    Software Version
        Type: Software version (0x0005)
        Length: 216
        Software Version: Cisco Internetwork Operating System Software 
                          IOS (tm) 1600 Software (C1600-NY-L), Version 11.2(12)P, RELEASE SOFTWARE (fc1)
                          Copyright (c) 1986-1998 by cisco Systems, Inc.
                          Compiled Tue 03-Mar-98 06:33 by dschwart
    Platform: cisco 1601
        Type: Platform (0x0006)
        Length: 14
        Platform: cisco 1601
	*/
	struct lldpd_chassis *nchassis = NULL;
	struct lldpd_port *nport = NULL;

	fail_unless(cdpv1_guess(pkt1, sizeof(pkt1)));
	fail_unless(cdp_decode(NULL, pkt1, sizeof(pkt1), &hardware,
		&nchassis, &nport) != -1);
	if (!nchassis || !nport) {
		fail("unable to decode packet");
		return;
	}
	ck_assert_int_eq(nchassis->c_id_subtype,
	    LLDP_CHASSISID_SUBTYPE_LOCAL);
	ck_assert_int_eq(nchassis->c_id_len, 2);
	fail_unless(memcmp(nchassis->c_id, "R1", 2) == 0);
	ck_assert_str_eq(nchassis->c_name, "R1");
	ck_assert_int_eq(TAILQ_FIRST(&nchassis->c_mgmt)->m_addr.inet.s_addr,
	    (u_int32_t)inet_addr("192.168.10.1"));
	ck_assert_int_eq(TAILQ_FIRST(&nchassis->c_mgmt)->m_iface, 0);
	ck_assert_int_eq(nport->p_id_subtype,
	    LLDP_PORTID_SUBTYPE_IFNAME);
	ck_assert_int_eq(nport->p_id_len, strlen("Ethernet0"));
	fail_unless(memcmp(nport->p_id,
		"Ethernet0", strlen("Ethernet0")) == 0);
	ck_assert_str_eq(nport->p_descr, "Ethernet0");
	ck_assert_int_eq(nchassis->c_cap_enabled, LLDP_CAP_ROUTER);
	ck_assert_str_eq(nchassis->c_descr,
	    "cisco 1601 running on\n"
	    "Cisco Internetwork Operating System Software \n"
	    "IOS (tm) 1600 Software (C1600-NY-L), Version 11.2(12)P, RELEASE SOFTWARE (fc1)\n"
	    "Copyright (c) 1986-1998 by cisco Systems, Inc.\n"
	    "Compiled Tue 03-Mar-98 06:33 by dschwart");
}
END_TEST

START_TEST (test_recv_cdpv2)
{
	char pkt1[] = {
		0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc, 0xca, 0x00,
		0x68, 0x46, 0x00, 0x00, 0x01, 0x30, 0xaa, 0xaa,
		0x03, 0x00, 0x00, 0x0c, 0x20, 0x00, 0x02, 0xb4,
		0x54, 0x27, 0x00, 0x01, 0x00, 0x0f, 0x72, 0x74,
		0x62, 0x67, 0x36, 0x74, 0x65, 0x73, 0x74, 0x30,
		0x31, 0x00, 0x05, 0x00, 0xd3, 0x43, 0x69, 0x73,
		0x63, 0x6f, 0x20, 0x49, 0x6e, 0x74, 0x65, 0x72,
		0x6e, 0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b, 0x20,
		0x4f, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6e,
		0x67, 0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d,
		0x20, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72,
		0x65, 0x20, 0x0a, 0x49, 0x4f, 0x53, 0x20, 0x28,
		0x74, 0x6d, 0x29, 0x20, 0x37, 0x32, 0x30, 0x30,
		0x20, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72,
		0x65, 0x20, 0x28, 0x43, 0x37, 0x32, 0x30, 0x30,
		0x2d, 0x50, 0x2d, 0x4d, 0x29, 0x2c, 0x20, 0x56,
		0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x20, 0x31,
		0x32, 0x2e, 0x32, 0x28, 0x34, 0x36, 0x29, 0x2c,
		0x20, 0x52, 0x45, 0x4c, 0x45, 0x41, 0x53, 0x45,
		0x20, 0x53, 0x4f, 0x46, 0x54, 0x57, 0x41, 0x52,
		0x45, 0x20, 0x28, 0x66, 0x63, 0x31, 0x29, 0x0a,
		0x43, 0x6f, 0x70, 0x79, 0x72, 0x69, 0x67, 0x68,
		0x74, 0x20, 0x28, 0x63, 0x29, 0x20, 0x31, 0x39,
		0x38, 0x36, 0x2d, 0x32, 0x30, 0x30, 0x37, 0x20,
		0x62, 0x79, 0x20, 0x63, 0x69, 0x73, 0x63, 0x6f,
		0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x73,
		0x2c, 0x20, 0x49, 0x6e, 0x63, 0x2e, 0x0a, 0x43,
		0x6f, 0x6d, 0x70, 0x69, 0x6c, 0x65, 0x64, 0x20,
		0x54, 0x68, 0x75, 0x20, 0x32, 0x36, 0x2d, 0x41,
		0x70, 0x72, 0x2d, 0x30, 0x37, 0x20, 0x32, 0x31,
		0x3a, 0x35, 0x36, 0x20, 0x62, 0x79, 0x20, 0x70,
		0x77, 0x61, 0x64, 0x65, 0x00, 0x06, 0x00, 0x11,
		0x63, 0x69, 0x73, 0x63, 0x6f, 0x20, 0x37, 0x32,
		0x30, 0x36, 0x56, 0x58, 0x52, 0x00, 0x02, 0x00,
		0x11, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0xcc,
		0x00, 0x04, 0xac, 0x42, 0x37, 0x03, 0x00, 0x03,
		0x00, 0x13, 0x46, 0x61, 0x73, 0x74, 0x45, 0x74,
		0x68, 0x65, 0x72, 0x6e, 0x65, 0x74, 0x30, 0x2f,
		0x30, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x0b, 0x00, 0x05, 0x00 };
	/* This is:
IEEE 802.3 Ethernet 
    Destination: CDP/VTP/DTP/PAgP/UDLD (01:00:0c:cc:cc:cc)
    Source: ca:00:68:46:00:00 (ca:00:68:46:00:00)
    Length: 304
Logical-Link Control
    DSAP: SNAP (0xaa)
    IG Bit: Individual
    SSAP: SNAP (0xaa)
    CR Bit: Command
    Control field: U, func=UI (0x03)
        000. 00.. = Command: Unnumbered Information (0x00)
        .... ..11 = Frame type: Unnumbered frame (0x03)
    Organization Code: Cisco (0x00000c)
    PID: CDP (0x2000)
Cisco Discovery Protocol
    Version: 2
    TTL: 180 seconds
    Checksum: 0x5427 [correct]
        [Good: True]
        [Bad : False]
    Device ID: rtbg6test01
        Type: Device ID (0x0001)
        Length: 15
        Device ID: rtbg6test01
    Software Version
        Type: Software version (0x0005)
        Length: 211
        Software Version: Cisco Internetwork Operating System Software 
                          IOS (tm) 7200 Software (C7200-P-M), Version 12.2(46), RELEASE SOFTWARE (fc1)
                          Copyright (c) 1986-2007 by cisco Systems, Inc.
                          Compiled Thu 26-Apr-07 21:56 by pwade
    Platform: cisco 7206VXR
        Type: Platform (0x0006)
        Length: 17
        Platform: cisco 7206VXR
    Addresses
        Type: Addresses (0x0002)
        Length: 17
        Number of addresses: 1
        IP address: 172.66.55.3
            Protocol type: NLPID
            Protocol length: 1
            Protocol: IP
            Address length: 4
            IP address: 172.66.55.3
    Port ID: FastEthernet0/0
        Type: Port ID (0x0003)
        Length: 19
        Sent through Interface: FastEthernet0/0
    Capabilities
        Type: Capabilities (0x0004)
        Length: 8
        Capabilities: 0x00000000
            .... .... .... .... .... .... .... ...0 = Not a Router
            .... .... .... .... .... .... .... ..0. = Not a Transparent Bridge
            .... .... .... .... .... .... .... .0.. = Not a Source Route Bridge
            .... .... .... .... .... .... .... 0... = Not a Switch
            .... .... .... .... .... .... ...0 .... = Not a Host
            .... .... .... .... .... .... ..0. .... = Not IGMP capable
            .... .... .... .... .... .... .0.. .... = Not a Repeater
    Duplex: Half
        Type: Duplex (0x000b)
        Length: 5
        Duplex: Half
	*/
	struct lldpd_chassis *nchassis = NULL;
	struct lldpd_port *nport = NULL;

	fail_unless(cdpv2_guess(pkt1, sizeof(pkt1)));
	fail_unless(cdp_decode(NULL, pkt1, sizeof(pkt1), &hardware,
		&nchassis, &nport) != -1);
	if (!nchassis || !nport) {
		fail("unable to decode packet");
		return;
	}
	ck_assert_int_eq(nchassis->c_id_subtype,
	    LLDP_CHASSISID_SUBTYPE_LOCAL);
	ck_assert_int_eq(nchassis->c_id_len, strlen("rtbg6test01"));
	fail_unless(memcmp(nchassis->c_id,
		"rtbg6test01", strlen("rtbg6test01")) == 0);
	ck_assert_str_eq(nchassis->c_name, "rtbg6test01");
	ck_assert_int_eq(TAILQ_FIRST(&nchassis->c_mgmt)->m_addr.inet.s_addr,
	    (u_int32_t)inet_addr("172.66.55.3"));
	ck_assert_int_eq(TAILQ_FIRST(&nchassis->c_mgmt)->m_iface, 0);
	ck_assert_int_eq(nport->p_id_subtype,
	    LLDP_PORTID_SUBTYPE_IFNAME);
	ck_assert_int_eq(nport->p_id_len, strlen("FastEthernet0/0"));
	fail_unless(memcmp(nport->p_id,
		"FastEthernet0/0", strlen("FastEthernet0/0")) == 0);
	ck_assert_str_eq(nport->p_descr, "FastEthernet0/0");
	ck_assert_int_eq(nchassis->c_cap_enabled, LLDP_CAP_STATION);
	ck_assert_str_eq(nchassis->c_descr,
	    "cisco 7206VXR running on\n"
	    "Cisco Internetwork Operating System Software \n"
	    "IOS (tm) 7200 Software (C7200-P-M), Version 12.2(46), RELEASE SOFTWARE (fc1)\n"
	    "Copyright (c) 1986-2007 by cisco Systems, Inc.\n"
	    "Compiled Thu 26-Apr-07 21:56 by pwade");
}
END_TEST

#endif

Suite *
cdp_suite(void)
{
	Suite *s = suite_create("CDP");

#ifdef ENABLE_CDP
	TCase *tc_send = tcase_create("Send CDP packets");
	TCase *tc_receive = tcase_create("Receive CDP packets");

	tcase_add_checked_fixture(tc_send, pcap_setup, pcap_teardown);
	tcase_add_test(tc_send, test_send_cdpv1);
	tcase_add_test(tc_send, test_send_cdpv2);
	suite_add_tcase(s, tc_send);

	tcase_add_test(tc_receive, test_recv_cdpv1);
	tcase_add_test(tc_receive, test_recv_cdpv2);
	suite_add_tcase(s, tc_receive);
#endif

	return s;
}

int
main()
{
	int number_failed;
	Suite *s = cdp_suite ();
	SRunner *sr = srunner_create (s);
	srunner_set_fork_status (sr, CK_NOFORK); /* Can't fork because
						    we need to write
						    files */
	srunner_run_all (sr, CK_ENV);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
