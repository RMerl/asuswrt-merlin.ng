/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2013 Vincent Bernat <bernat@luffy.cx>
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
#include <unistd.h>
#include <errno.h>
#include <net/bpf.h>

struct bpf_buffer {
	size_t len;		/* Total length of the buffer */
	struct bpf_hdr data[0];
};

int
ifbpf_phys_init(struct lldpd *cfg,
    struct lldpd_hardware *hardware)
{
	struct bpf_buffer *buffer = NULL;
	int fd = -1;

	log_debug("interfaces", "initialize ethernet device %s",
	    hardware->h_ifname);
	if ((fd = priv_iface_init(hardware->h_ifindex, hardware->h_ifname)) == -1)
		return -1;

	/* Allocate receive buffer */
	hardware->h_data = buffer =
	    malloc(ETHER_MAX_LEN + BPF_WORDALIGN(sizeof(struct bpf_hdr)) + sizeof(struct bpf_buffer));
	if (buffer == NULL) {
		log_warn("interfaces",
		    "unable to allocate buffer space for BPF on %s",
		    hardware->h_ifname);
		goto end;
	}
	buffer->len = ETHER_MAX_LEN + BPF_WORDALIGN(sizeof(struct bpf_hdr));

	/* Setup multicast */
	interfaces_setup_multicast(cfg, hardware->h_ifname, 0);

	hardware->h_sendfd = fd; /* Send */

	levent_hardware_add_fd(hardware, fd); /* Receive */
	log_debug("interfaces", "interface %s initialized (fd=%d)", hardware->h_ifname,
	    fd);
	return 0;

end:
	if (fd >= 0) close(fd);
	free(buffer);
	hardware->h_data = NULL;
	return -1;
}

/* Ethernet send/receive through BPF */
static int
ifbpf_eth_send(struct lldpd *cfg, struct lldpd_hardware *hardware,
    char *buffer, size_t size)
{
	log_debug("interfaces", "send PDU to ethernet device %s (fd=%d)",
	    hardware->h_ifname, hardware->h_sendfd);
	return write(hardware->h_sendfd,
	    buffer, size);
}

static int
ifbpf_eth_recv(struct lldpd *cfg,
    struct lldpd_hardware *hardware,
    int fd, char *buffer, size_t size)
{
	struct bpf_buffer *bpfbuf = hardware->h_data;
	struct bpf_hdr *bh;
	log_debug("interfaces", "receive PDU from ethernet device %s",
	    hardware->h_ifname);

	/* We assume we have only receive one packet (unbuffered mode). Dunno if
	 * this is correct. */
	if (read(fd, bpfbuf->data, bpfbuf->len) == -1) {
		if (errno == ENETDOWN) {
			log_debug("interfaces", "error while receiving frame on %s (network down)",
			    hardware->h_ifname);
		} else {
			log_warn("interfaces", "error while receiving frame on %s",
			    hardware->h_ifname);
			hardware->h_rx_discarded_cnt++;
		}
		return -1;
	}
	bh = (struct bpf_hdr*)bpfbuf->data;
	if (bh->bh_caplen < size)
		size = bh->bh_caplen;
	memcpy(buffer, (char *)bpfbuf->data + bh->bh_hdrlen, size);

	return size;
}

static int
ifbpf_eth_close(struct lldpd *cfg, struct lldpd_hardware *hardware)
{
	log_debug("interfaces", "close ethernet device %s",
	    hardware->h_ifname);
	interfaces_setup_multicast(cfg, hardware->h_ifname, 1);
	return 0;
}

struct lldpd_ops bpf_ops = {
	.send = ifbpf_eth_send,
	.recv = ifbpf_eth_recv,
	.cleanup = ifbpf_eth_close,
};
