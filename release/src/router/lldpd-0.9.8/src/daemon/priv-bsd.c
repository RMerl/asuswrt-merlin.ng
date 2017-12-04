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

#include <unistd.h>
#include <net/bpf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int
asroot_iface_init_os(int ifindex, char *name, int *fd)
{
	int enable, required, rc;
	struct bpf_insn filter[] = { LLDPD_FILTER_F };
	struct ifreq ifr = { .ifr_name = {} };
	struct bpf_program fprog = {
		.bf_insns = filter,
		.bf_len = sizeof(filter)/sizeof(struct bpf_insn)
	};

#ifndef HOST_OS_SOLARIS
	int n = 0;
	char dev[20];
	do {
		snprintf(dev, sizeof(dev), "/dev/bpf%d", n++);
		*fd = open(dev, O_RDWR);
	} while (*fd < 0 && errno == EBUSY);
#else
	*fd = open("/dev/bpf", O_RDWR);
#endif
	if (*fd < 0) {
		rc = errno;
		log_warn("privsep", "unable to find a free BPF");
		return rc;
	}

	/* Set buffer size */
	required = ETHER_MAX_LEN + BPF_WORDALIGN(sizeof(struct bpf_hdr));
	if (ioctl(*fd, BIOCSBLEN, (caddr_t)&required) < 0) {
		rc = errno;
		log_warn("privsep",
		    "unable to set receive buffer size for BPF on %s",
		    name);
		return rc;
	}

	/* Bind the interface to BPF device */
	strlcpy(ifr.ifr_name, name, IFNAMSIZ);
	if (ioctl(*fd, BIOCSETIF, (caddr_t)&ifr) < 0) {
		rc = errno;
		log_warn("privsep", "failed to bind interface %s to BPF",
		    name);
		return rc;
	}

	/* Disable buffering */
	enable = 1;
	if (ioctl(*fd, BIOCIMMEDIATE, (caddr_t)&enable) < 0) {
		rc = errno;
		log_warn("privsep", "unable to disable buffering for %s",
		    name);
		return rc;
	}

	/* Let us write the MAC address (raw packet mode) */
	enable = 1;
	if (ioctl(*fd, BIOCSHDRCMPLT, (caddr_t)&enable) < 0) {
		rc = errno;
		log_warn("privsep",
		    "unable to set the `header complete` flag for %s",
		    name);
		return rc;
	}

	/* Don't see sent packets */
#ifdef HOST_OS_OPENBSD
	enable = BPF_DIRECTION_OUT;
	if (ioctl(*fd, BIOCSDIRFILT, (caddr_t)&enable) < 0)
#else
	enable = 0;
	if (ioctl(*fd, BIOCSSEESENT, (caddr_t)&enable) < 0)
#endif
	{
		rc = errno;
		log_warn("privsep",
		    "unable to set packet direction for BPF filter on %s",
		    name);
		return rc;
	}

	/* Install read filter */
	if (ioctl(*fd, BIOCSETF, (caddr_t)&fprog) < 0) {
		rc = errno;
		log_warn("privsep", "unable to setup BPF filter for %s",
		    name);
		return rc;
	}
#ifdef BIOCSETWF
	/* Install write filter (optional) */
	if (ioctl(*fd, BIOCSETWF, (caddr_t)&fprog) < 0) {
		rc = errno;
		log_info("privsep", "unable to setup write BPF filter for %s",
		    name);
		return rc;
	}
#endif

#ifdef BIOCLOCK
	/* Lock interface, but first make it non blocking since we cannot do
	 * this later */
	levent_make_socket_nonblocking(*fd);
	if (ioctl(*fd, BIOCLOCK, (caddr_t)&enable) < 0) {
		rc = errno;
		log_info("privsep", "unable to lock BPF interface %s",
		    name);
		return rc;
	}
#endif
	return 0;
}

int
asroot_iface_description_os(const char *name, const char *description)
{
#ifdef IFDESCRSIZE
#if defined HOST_OS_FREEBSD || defined HOST_OS_OPENBSD
	char descr[IFDESCRSIZE];
	int rc, sock = -1;
#if defined HOST_OS_FREEBSD
	struct ifreq ifr = {
		.ifr_buffer = { .buffer = descr,
				.length = IFDESCRSIZE }
	};
#else
	struct ifreq ifr = {
		.ifr_data = (caddr_t)descr
	};
#endif
	strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == 1) {
		rc = errno;
		log_warnx("privsep", "unable to open inet socket");
		return rc;
	}
	if (strlen(description) == 0) {
		/* No neighbor, try to append "was" to the current description */
		if (ioctl(sock, SIOCGIFDESCR, (caddr_t)&ifr) < 0) {
			rc = errno;
			log_warnx("privsep", "unable to get description of %s",
			    name);
			close(sock);
			return rc;
		}
		if (strncmp(descr, "lldpd: ", 7) == 0) {
			if (strncmp(descr + 7, "was ", 4) == 0) {
				/* Already has an old neighbor */
				close(sock);
				return 0;
			} else {
				/* Append was */
				memmove(descr + 11, descr + 7,
				    sizeof(descr) - 11);
				memcpy(descr, "lldpd: was ", 11);
			}
		} else {
			/* No description, no neighbor */
			strlcpy(descr, "lldpd: no neighbor", sizeof(descr));
		}
	} else
		snprintf(descr, sizeof(descr), "lldpd: connected to %s", description);
#if defined HOST_OS_FREEBSD
	ift.ifr_buffer.length = strlen(descr);
#endif
	if (ioctl(sock, SIOCSIFDESCR, (caddr_t)&ifr) < 0) {
		rc = errno;
		log_warnx("privsep", "unable to set description of %s",
		    name);
		close(sock);
		return rc;
	}
	close(sock);
	return 0;
#endif
#endif /* IFDESCRSIZE */
	static int once = 0;
	if (!once) {
		log_warnx("privsep", "cannot set interface description for this OS");
		once = 1;
	}
	return 0;
}

int
asroot_iface_promisc_os(const char *name)
{
	/* The promiscuous mode can be set when setting BPF
	   (BIOCPROMISC). Unfortunately, the interface is locked down and we
	   cannot change that without reopening a new socket. Let's do nothing
	   for now. */
	return 0;
}
