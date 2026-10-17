/*
 * RFC3927 ZeroConf IPv4 Link-Local addressing
 * (see <http://www.zeroconf.org/>)
 *
 * Copied from BusyBox - networking/zcip.c
 *
 * Copyright (C) 2003 by Arthur van Hoff (avh@strangeberry.com)
 * Copyright (C) 2004 by David Brownell
 * Copyright (C) 2010 by Joe Hershberger
 *
 * Licensed under the GPL v2 or later
 */

#include <common.h>
#include <env.h>
#include <log.h>
#include <net.h>
#include <rand.h>
#include "arp.h"
#include "net_rand.h"

/* We don't need more than 32 bits of the counter */
#define MONOTONIC_MS() ((unsigned)get_timer(0) * (1000 / CONFIG_SYS_HZ))

enum {
/* 169.254.0.0 */
	LINKLOCAL_ADDR = 0xa9fe0000,

	IN_CLASSB_NET = 0xffff0000,
	IN_CLASSB_HOST = 0x0000ffff,

/* protocol timeout parameters, specified in seconds */
	PROBE_WAIT = 1,
	PROBE_MIN = 1,
	PROBE_MAX = 2,
	PROBE_NUM = 3,
	MAX_CONFLICTS = 10,
	RATE_LIMIT_INTERVAL = 60,
	ANNOUNCE_WAIT = 2,
	ANNOUNCE_NUM = 2,
	ANNOUNCE_INTERVAL = 2,
	DEFEND_INTERVAL = 10
};

/* States during the configuration process. */
static enum ll_state_t {
	PROBE = 0,
	RATE_LIMIT_PROBE,
	ANNOUNCE,
	MONITOR,
	DEFEND,
	DISABLED
} state = DISABLED;

static struct in_addr ip;
static int timeout_ms = -1;
static unsigned deadline_ms;
static unsigned conflicts;
static unsigned nprobes;
static unsigned nclaims;
static int ready;
static unsigned int seed;

static void link_local_timeout(void);

/**
 * Pick a random link local IP address on 169.254/16, except that
 * the first and last 256 addresses are reserved.
 */
static struct in_addr pick(void)
{
	unsigned tmp;
	struct in_addr ip;

	do {
		tmp = rand_r(&seed) & IN_CLASSB_HOST;
	} while (tmp > (IN_CLASSB_HOST - 0x0200));
	ip.s_addr = htonl((LINKLOCAL_ADDR + 0x0100) + tmp);
	return ip;
}

/**
 * Return milliseconds of random delay, up to "secs" seconds.
 */
static inline unsigned random_delay_ms(unsigned secs)
{
	return rand_r(&seed) % (secs * 1000);
}

static void configure_wait(void)
{
	if (timeout_ms == -1)
		return;

	/* poll, being ready to adjust current timeout */
	if (!timeout_ms)
		timeout_ms = random_delay_ms(PROBE_WAIT);

	/* set deadline_ms to the point in time when we timeout */
	deadline_ms = MONOTONIC_MS() + timeout_ms;

	debug_cond(DEBUG_DEV_PKT, "...wait %d %s nprobes=%u, nclaims=%u\n",
		   timeout_ms, eth_get_name(), nprobes, nclaims);

	net_set_timeout_handler(timeout_ms, link_local_timeout);
}

void link_local_start(void)
{
	ip = env_get_ip("llipaddr");
	if (ip.s_addr != 0 &&
	    (ntohl(ip.s_addr) & IN_CLASSB_NET) != LINKLOCAL_ADDR) {
		puts("invalid link address");
		net_set_state(NETLOOP_FAIL);
		return;
	}
	net_netmask.s_addr = htonl(IN_CLASSB_NET);

	seed = seed_mac();
	if (ip.s_addr == 0)
		ip = pick();

	state = PROBE;
	timeout_ms = 0;
	conflicts = 0;
	nprobes = 0;
	nclaims = 0;
	ready = 0;

	configure_wait();
}

static void link_local_timeout(void)
{
	switch (state) {
	case PROBE:
		/* timeouts in the PROBE state mean no conflicting ARP packets
		   have been received, so we can progress through the states */
		if (nprobes < PROBE_NUM) {
			struct in_addr zero_ip = {.s_addr = 0};

			nprobes++;
			debug_cond(DEBUG_LL_STATE, "probe/%u %s@%pI4\n",
				   nprobes, eth_get_name(), &ip);
			arp_raw_request(zero_ip, net_null_ethaddr, ip);
			timeout_ms = PROBE_MIN * 1000;
			timeout_ms += random_delay_ms(PROBE_MAX - PROBE_MIN);
		} else {
			/* Switch to announce state */
			state = ANNOUNCE;
			nclaims = 0;
			debug_cond(DEBUG_LL_STATE, "announce/%u %s@%pI4\n",
				   nclaims, eth_get_name(), &ip);
			arp_raw_request(ip, net_ethaddr, ip);
			timeout_ms = ANNOUNCE_INTERVAL * 1000;
		}
		break;
	case RATE_LIMIT_PROBE:
		/* timeouts in the RATE_LIMIT_PROBE state mean no conflicting
		   ARP packets have been received, so we can move immediately
		   to the announce state */
		state = ANNOUNCE;
		nclaims = 0;
		debug_cond(DEBUG_LL_STATE, "announce/%u %s@%pI4\n",
			   nclaims, eth_get_name(), &ip);
		arp_raw_request(ip, net_ethaddr, ip);
		timeout_ms = ANNOUNCE_INTERVAL * 1000;
		break;
	case ANNOUNCE:
		/* timeouts in the ANNOUNCE state mean no conflicting ARP
		   packets have been received, so we can progress through
		   the states */
		if (nclaims < ANNOUNCE_NUM) {
			nclaims++;
			debug_cond(DEBUG_LL_STATE, "announce/%u %s@%pI4\n",
				   nclaims, eth_get_name(), &ip);
			arp_raw_request(ip, net_ethaddr, ip);
			timeout_ms = ANNOUNCE_INTERVAL * 1000;
		} else {
			/* Switch to monitor state */
			state = MONITOR;
			printf("Successfully assigned %pI4\n", &ip);
			net_copy_ip(&net_ip, &ip);
			ready = 1;
			conflicts = 0;
			timeout_ms = -1;
			/* Never timeout in the monitor state */
			net_set_timeout_handler(0, NULL);

			/* NOTE: all other exit paths should deconfig ... */
			net_set_state(NETLOOP_SUCCESS);
			return;
		}
		break;
	case DEFEND:
		/* We won!  No ARP replies, so just go back to monitor */
		state = MONITOR;
		timeout_ms = -1;
		conflicts = 0;
		break;
	default:
		/* Invalid, should never happen.  Restart the whole protocol */
		state = PROBE;
		ip = pick();
		timeout_ms = 0;
		nprobes = 0;
		nclaims = 0;
		break;
	}
	configure_wait();
}

void link_local_receive_arp(struct arp_hdr *arp, int len)
{
	int source_ip_conflict;
	int target_ip_conflict;
	struct in_addr null_ip = {.s_addr = 0};

	if (state == DISABLED)
		return;

	/* We need to adjust the timeout in case we didn't receive a
	   conflicting packet. */
	if (timeout_ms > 0) {
		unsigned diff = deadline_ms - MONOTONIC_MS();
		if ((int)(diff) < 0) {
			/* Current time is greater than the expected timeout
			   time. This should never happen */
			debug_cond(DEBUG_LL_STATE,
				   "missed an expected timeout\n");
			timeout_ms = 0;
		} else {
			debug_cond(DEBUG_INT_STATE, "adjusting timeout\n");
			timeout_ms = diff | 1; /* never 0 */
		}
	}
#if 0
 /* XXX Don't bother with ethernet link just yet */
	if ((fds[0].revents & POLLIN) == 0) {
		if (fds[0].revents & POLLERR) {
			/*
			 * FIXME: links routinely go down;
			 */
			bb_error_msg("iface %s is down", eth_get_name());
			if (ready)
				run(argv, "deconfig", &ip);
			return EXIT_FAILURE;
		}
		continue;
	}
#endif

	debug_cond(DEBUG_INT_STATE, "%s recv arp type=%d, op=%d,\n",
		   eth_get_name(), ntohs(arp->ar_pro),
		   ntohs(arp->ar_op));
	debug_cond(DEBUG_INT_STATE, "\tsource=%pM %pI4\n",
		   &arp->ar_sha,
		   &arp->ar_spa);
	debug_cond(DEBUG_INT_STATE, "\ttarget=%pM %pI4\n",
		   &arp->ar_tha,
		   &arp->ar_tpa);

	if (arp->ar_op != htons(ARPOP_REQUEST) &&
	    arp->ar_op != htons(ARPOP_REPLY)) {
		configure_wait();
		return;
	}

	source_ip_conflict = 0;
	target_ip_conflict = 0;

	if (memcmp(&arp->ar_spa, &ip, ARP_PLEN) == 0 &&
	    memcmp(&arp->ar_sha, net_ethaddr, ARP_HLEN) != 0)
		source_ip_conflict = 1;

	/*
	 * According to RFC 3927, section 2.2.1:
	 * Check if packet is an ARP probe by checking for a null source IP
	 * then check that target IP is equal to ours and source hw addr
	 * is not equal to ours. This condition should cause a conflict only
	 * during probe.
	 */
	if (arp->ar_op == htons(ARPOP_REQUEST) &&
	    memcmp(&arp->ar_spa, &null_ip, ARP_PLEN) == 0 &&
	    memcmp(&arp->ar_tpa, &ip, ARP_PLEN) == 0 &&
	    memcmp(&arp->ar_sha, net_ethaddr, ARP_HLEN) != 0) {
		target_ip_conflict = 1;
	}

	debug_cond(DEBUG_NET_PKT,
		   "state = %d, source ip conflict = %d, target ip conflict = "
		   "%d\n", state, source_ip_conflict, target_ip_conflict);
	switch (state) {
	case PROBE:
	case ANNOUNCE:
		/* When probing or announcing, check for source IP conflicts
		   and other hosts doing ARP probes (target IP conflicts). */
		if (source_ip_conflict || target_ip_conflict) {
			conflicts++;
			state = PROBE;
			if (conflicts >= MAX_CONFLICTS) {
				debug("%s ratelimit\n", eth_get_name());
				timeout_ms = RATE_LIMIT_INTERVAL * 1000;
				state = RATE_LIMIT_PROBE;
			}

			/* restart the whole protocol */
			ip = pick();
			timeout_ms = 0;
			nprobes = 0;
			nclaims = 0;
		}
		break;
	case MONITOR:
		/* If a conflict, we try to defend with a single ARP probe */
		if (source_ip_conflict) {
			debug("monitor conflict -- defending\n");
			state = DEFEND;
			timeout_ms = DEFEND_INTERVAL * 1000;
			arp_raw_request(ip, net_ethaddr, ip);
		}
		break;
	case DEFEND:
		/* Well, we tried.  Start over (on conflict) */
		if (source_ip_conflict) {
			state = PROBE;
			debug("defend conflict -- starting over\n");
			ready = 0;
			net_ip.s_addr = 0;

			/* restart the whole protocol */
			ip = pick();
			timeout_ms = 0;
			nprobes = 0;
			nclaims = 0;
		}
		break;
	default:
		/* Invalid, should never happen.  Restart the whole protocol */
		debug("invalid state -- starting over\n");
		state = PROBE;
		ip = pick();
		timeout_ms = 0;
		nprobes = 0;
		nclaims = 0;
		break;
	}
	configure_wait();
}
