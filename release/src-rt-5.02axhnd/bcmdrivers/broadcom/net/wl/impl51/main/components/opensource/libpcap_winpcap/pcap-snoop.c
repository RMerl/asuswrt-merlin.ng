/*
 * Copyright (c) 1993, 1994, 1995, 1996, 1997
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef lint
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/libpcap/pcap-snoop.c,v 1.55.2.3 2008-04-14 20:41:52 guy Exp $ (LBL)";
#endif // endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // endif

#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <net/raw.h>
#include <net/if.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pcap-int.h"

#ifdef HAVE_OS_PROTO_H
#include "os-proto.h"
#endif // endif

static int
pcap_read_snoop(pcap_t *p, int cnt, pcap_handler callback, u_char *user)
{
	int cc;
	register struct snoopheader *sh;
	register u_int datalen;
	register u_int caplen;
	register u_char *cp;

again:
	/*
	 * Has "pcap_breakloop()" been called?
	 */
	if (p->break_loop) {
		/*
		 * Yes - clear the flag that indicates that it
		 * has, and return -2 to indicate that we were
		 * told to break out of the loop.
		 */
		p->break_loop = 0;
		return (-2);
	}
	cc = read(p->fd, (char *)p->buffer, p->bufsize);
	if (cc < 0) {
		/* Don't choke when we get ptraced */
		switch (errno) {

		case EINTR:
			goto again;

		case EWOULDBLOCK:
			return (0);
		}
		snprintf(p->errbuf, sizeof(p->errbuf),
		    "read: %s", pcap_strerror(errno));
		return (-1);
	}
	sh = (struct snoopheader *)p->buffer;
	datalen = sh->snoop_packetlen;

	if (cc == (p->snapshot + sizeof(struct snoopheader)) &&
	    (datalen < p->snapshot))
		datalen += (64 * 1024);

	caplen = (datalen < p->snapshot) ? datalen : p->snapshot;
	cp = (u_char *)(sh + 1) + p->offset;

	if (p->linktype == DLT_NULL && *((short *)(cp + 2)) == 0) {
		u_int *uip = (u_int *)cp;
		*uip >>= 16;
	}

	if (p->fcode.bf_insns == NULL ||
	    bpf_filter(p->fcode.bf_insns, cp, datalen, caplen)) {
		struct pcap_pkthdr h;
		++p->md.stat.ps_recv;
		h.ts.tv_sec = sh->snoop_timestamp.tv_sec;
		h.ts.tv_usec = sh->snoop_timestamp.tv_usec;
		h.len = datalen;
		h.caplen = caplen;
		(*callback)(user, &h, cp);
		return (1);
	}
	return (0);
}

static int
pcap_inject_snoop(pcap_t *p, const void *buf, size_t size)
{
	int ret;

	ret = write(p->fd, buf, size);
	if (ret == -1) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "send: %s",
		    pcap_strerror(errno));
		return (-1);
	}
	return (ret);
}

static int
pcap_stats_snoop(pcap_t *p, struct pcap_stat *ps)
{
	register struct rawstats *rs;
	struct rawstats rawstats;

	rs = &rawstats;
	memset(rs, 0, sizeof(*rs));
	if (ioctl(p->fd, SIOCRAWSTATS, (char *)rs) < 0) {
		snprintf(p->errbuf, sizeof(p->errbuf),
		    "SIOCRAWSTATS: %s", pcap_strerror(errno));
		return (-1);
	}

	p->md.stat.ps_drop =
	    rs->rs_snoop.ss_ifdrops + rs->rs_snoop.ss_sbdrops +
	    rs->rs_drain.ds_ifdrops + rs->rs_drain.ds_sbdrops;

	/*
	 * "ps_recv" counts only packets that passed the filter.
	 * As filtering is done in userland, this does not include
	 * packets dropped because we ran out of buffer space.
	 */
	*ps = p->md.stat;
	return (0);
}

static int
pcap_activate_snoop(pcap_t *p)
{
	int fd;
	struct sockaddr_raw sr;
	struct snoopfilter sf;
	u_int v;
	int ll_hdrlen;
	int snooplen;
	struct ifreq ifr;

	fd = socket(PF_RAW, SOCK_RAW, RAWPROTO_SNOOP);
	if (fd < 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "snoop socket: %s",
		    pcap_strerror(errno));
		goto bad;
	}
	p->fd = fd;
	memset(&sr, 0, sizeof(sr));
	sr.sr_family = AF_RAW;
	(void)strncpy(sr.sr_ifname, p->opt.source, sizeof(sr.sr_ifname));
	if (bind(fd, (struct sockaddr *)&sr, sizeof(sr))) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "snoop bind: %s",
		    pcap_strerror(errno));
		goto bad;
	}
	memset(&sf, 0, sizeof(sf));
	if (ioctl(fd, SIOCADDSNOOP, &sf) < 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "SIOCADDSNOOP: %s",
		    pcap_strerror(errno));
		goto bad;
	}
	if (handle->opt.buffer_size != 0)
		v = handle->opt.buffer_size;
	else
		v = 64 * 1024;	/* default to 64K buffer size */
	(void)setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&v, sizeof(v));
	if (strncmp("et", p->opt.source, 2) == 0 ||	/* Challenge 10 Mbit */
	    strncmp("ec", p->opt.source, 2) == 0 ||	/* Indigo/Indy 10 Mbit,
							   O2 10/100 */
	    strncmp("ef", p->opt.source, 2) == 0 ||	/* O200/2000 10/100 Mbit */
	    strncmp("eg", p->opt.source, 2) == 0 ||	/* Octane/O2xxx/O3xxx Gigabit */
	    strncmp("gfe", p->opt.source, 3) == 0 ||	/* GIO 100 Mbit */
	    strncmp("fxp", p->opt.source, 3) == 0 ||	/* Challenge VME Enet */
	    strncmp("ep", p->opt.source, 2) == 0 ||	/* Challenge 8x10 Mbit EPLEX */
	    strncmp("vfe", p->opt.source, 3) == 0 ||	/* Challenge VME 100Mbit */
	    strncmp("fa", p->opt.source, 2) == 0 ||
	    strncmp("qaa", p->opt.source, 3) == 0 ||
	    strncmp("cip", p->opt.source, 3) == 0 ||
	    strncmp("el", p->opt.source, 2) == 0) {
		p->linktype = DLT_EN10MB;
		p->offset = RAW_HDRPAD(sizeof(struct ether_header));
		ll_hdrlen = sizeof(struct ether_header);
		p->dlt_list = (u_int *) malloc(sizeof(u_int) * 2);
		/*
		 * If that fails, just leave the list empty.
		 */
		if (p->dlt_list != NULL) {
			p->dlt_list[0] = DLT_EN10MB;
			p->dlt_list[1] = DLT_DOCSIS;
			p->dlt_count = 2;
		}
	} else if (strncmp("ipg", p->opt.source, 3) == 0 ||
		   strncmp("rns", p->opt.source, 3) == 0 ||	/* O2/200/2000 FDDI */
		   strncmp("xpi", p->opt.source, 3) == 0) {
		p->linktype = DLT_FDDI;
		p->offset = 3;
		ll_hdrlen = 13;
	} else if (strncmp("ppp", p->opt.source, 3) == 0) {
		p->linktype = DLT_RAW;
		ll_hdrlen = 0;	/* DLT_RAW meaning "no PPP header, just the IP packet"? */
	} else if (strncmp("qfa", p->opt.source, 3) == 0) {
		p->linktype = DLT_IP_OVER_FC;
		ll_hdrlen = 24;
	} else if (strncmp("pl", p->opt.source, 2) == 0) {
		p->linktype = DLT_RAW;
		ll_hdrlen = 0;	/* Cray UNICOS/mp pseudo link */
	} else if (strncmp("lo", p->opt.source, 2) == 0) {
		p->linktype = DLT_NULL;
		ll_hdrlen = 4;
	} else {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE,
		    "snoop: unknown physical layer type");
		goto bad;
	}

	if (p->opt.rfmon) {
		/*
		 * No monitor mode on Irix (no Wi-Fi devices on
		 * hardware supported by Irix).
		 */
		return (PCAP_ERROR_RFMON_NOTSUP);
	}

#ifdef SIOCGIFMTU
	(void)strncpy(ifr.ifr_name, p->opt.source, sizeof(ifr.ifr_name));
	if (ioctl(fd, SIOCGIFMTU, (char *)&ifr) < 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "SIOCGIFMTU: %s",
		    pcap_strerror(errno));
		goto bad;
	}
#ifndef ifr_mtu
#define ifr_mtu	ifr_metric
#endif // endif
	if (p->snapshot > ifr.ifr_mtu + ll_hdrlen)
		p->snapshot = ifr.ifr_mtu + ll_hdrlen;
#endif // endif

	/*
	 * The argument to SIOCSNOOPLEN is the number of link-layer
	 * payload bytes to capture - it doesn't count link-layer
	 * header bytes.
	 */
	snooplen = p->snapshot - ll_hdrlen;
	if (snooplen < 0)
		snooplen = 0;
	if (ioctl(fd, SIOCSNOOPLEN, &snooplen) < 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "SIOCSNOOPLEN: %s",
		    pcap_strerror(errno));
		goto bad;
	}
	v = 1;
	if (ioctl(fd, SIOCSNOOPING, &v) < 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "SIOCSNOOPING: %s",
		    pcap_strerror(errno));
		goto bad;
	}

	p->bufsize = 4096;
	p->buffer = (u_char *)malloc(p->bufsize);
	if (p->buffer == NULL) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "malloc: %s",
		    pcap_strerror(errno));
		goto bad;
	}

	/*
	 * "p->fd" is a socket, so "select()" should work on it.
	 */
	p->selectable_fd = p->fd;

	p->read_op = pcap_read_snoop;
	p->inject_op = pcap_inject_snoop;
	p->setfilter_op = install_bpf_program;	/* no kernel filtering */
	p->setdirection_op = NULL;	/* Not implemented. */
	p->set_datalink_op = NULL;	/* can't change data link type */
	p->getnonblock_op = pcap_getnonblock_fd;
	p->setnonblock_op = pcap_setnonblock_fd;
	p->stats_op = pcap_stats_snoop;

	return (0);
 bad:
	return (PCAP_ERROR);
}

pcap_t *
pcap_create(const char *device, char *ebuf)
{
	pcap_t *p;

	p = pcap_create_common(device, ebuf);
	if (p == NULL)
		return (NULL);

	p->activate_op = pcap_activate_snoop;
	return (p);
}

int
pcap_platform_finddevs(pcap_if_t **alldevsp, char *errbuf)
{
	return (0);
}
