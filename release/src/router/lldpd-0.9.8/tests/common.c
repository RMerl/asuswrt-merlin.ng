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
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <check.h>
#include "common.h"

int dump = -1;
char *filename = NULL;
struct pkts_t pkts;
char macaddress[ETHER_ADDR_LEN] = { 0x5e, 0x10, 0x8e, 0xe7, 0x84, 0xad };
struct lldpd_hardware hardware;
struct lldpd_chassis chassis;

int
pcap_send(struct lldpd *cfg, struct lldpd_hardware *hardware,
    char *buffer, size_t size)
{
	struct pcaprec_hdr hdr;
	struct packet *pkt;
	int n;

	/* Write pcap record header */
	hdr.ts_sec = time(NULL);
	hdr.ts_usec = 0;
	hdr.incl_len = hdr.orig_len = size;
	n = write(dump, &hdr, sizeof(hdr));
	if (n == 1) {
		fail("unable to write pcap record header to %s", filename);
		return -1;
	}

	/* Write data */
	n = write(dump, buffer, size);
	if (n == -1) {
		fail("unable to write pcap data to %s", filename);
		return -1;
	}

	/* Append to list of packets */
	pkt = (struct packet *)malloc(size + sizeof(TAILQ_HEAD(,packet)) + sizeof(int));
	if (!pkt) {
		fail("unable to allocate packet");
		return -1;
	}
	memcpy(pkt->data, buffer, size);
	pkt->size = size;
	TAILQ_INSERT_TAIL(&pkts, pkt, next);
	return 0;
}

struct lldpd_ops pcap_ops = {
	.send = pcap_send,
	.recv = NULL,		/* Won't be used */
	.cleanup = NULL,	/* Won't be used */
};


void
pcap_setup()
{
	static int serial = 0;
	struct pcap_hdr hdr;
	int n;
	/* Prepare packet buffer */
	TAILQ_INIT(&pkts);
	/* Open a new dump file */
	n = asprintf(&filename, "%s_%04d.pcap", filenameprefix, serial++);
	if (n == -1) {
		fail("unable to compute filename");
		return;
	}
	dump = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (dump == -1) {
		fail("unable to open %s", filename);
		return;
	}
	/* Write a PCAP header */
	hdr.magic_number = 0xa1b2c3d4;
	hdr.version_major = 2;
	hdr.version_minor = 4;
	hdr.thiszone = 0;
	hdr.sigfigs = 0;
	hdr.snaplen = 65535;
	hdr.network = 1;
	n = write(dump, &hdr, sizeof(hdr));
	if (n == -1) {
		fail("unable to write pcap header to %s", filename);
		return;
	}
	/* Prepare hardware */
	memset(&hardware, 0, sizeof(struct lldpd_hardware));
	TAILQ_INIT(&hardware.h_rports);
#ifdef ENABLE_DOT1
	TAILQ_INIT(&hardware.h_lport.p_vlans);
	TAILQ_INIT(&hardware.h_lport.p_ppvids);
	TAILQ_INIT(&hardware.h_lport.p_pids);
#endif
	hardware.h_mtu = 1500;
	hardware.h_ifindex = 4;
	strlcpy(hardware.h_ifname, "test", sizeof(hardware.h_ifname));
	memcpy(hardware.h_lladdr, macaddress, ETHER_ADDR_LEN);
	hardware.h_ops = &pcap_ops;
	/* Prepare chassis */
	memset(&chassis, 0, sizeof(struct lldpd_chassis));
	hardware.h_lport.p_chassis = &chassis;
}

void
pcap_teardown()
{
	struct packet *npkt, *pkt;
	for (pkt = TAILQ_FIRST(&pkts);
	    pkt != NULL;
	    pkt = npkt) {
		npkt = TAILQ_NEXT(pkt, next);
		TAILQ_REMOVE(&pkts, pkt, next);
		free(pkt);
	}
	if (dump != -1) {
		close(dump);
		dump = -1;
	}
	if (filename) {
		free(filename);
		filename = NULL;
	}
}

/* Disable leak detection sanitizer */
int __lsan_is_turned_off() {
	return 1;
}
