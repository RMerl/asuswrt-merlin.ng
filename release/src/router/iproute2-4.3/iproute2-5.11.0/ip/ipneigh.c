/*
 * ipneigh.c		"ip neigh".
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include "json_print.h"

#define NUD_VALID	(NUD_PERMANENT|NUD_NOARP|NUD_REACHABLE|NUD_PROBE|NUD_STALE|NUD_DELAY)
#define MAX_ROUNDS	10

static struct
{
	int family;
	int index;
	int state;
	int unused_only;
	inet_prefix pfx;
	int flushed;
	char *flushb;
	int flushp;
	int flushe;
	int master;
	int protocol;
	__u8 ndm_flags;
} filter;

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr,
		"Usage: ip neigh { add | del | change | replace }\n"
		"		{ ADDR [ lladdr LLADDR ] [ nud STATE ] | proxy ADDR } [ dev DEV ]\n"
		"		[ router ] [ extern_learn ] [ protocol PROTO ]\n"
		"\n"
		"	ip neigh { show | flush } [ proxy ] [ to PREFIX ] [ dev DEV ] [ nud STATE ]\n"
		"				  [ vrf NAME ]\n"
		"	ip neigh get { ADDR | proxy ADDR } dev DEV\n"
		"\n"
		"STATE := { permanent | noarp | stale | reachable | none |\n"
		"           incomplete | delay | probe | failed }\n");
	exit(-1);
}

static int nud_state_a2n(unsigned int *state, const char *arg)
{
	if (matches(arg, "permanent") == 0)
		*state = NUD_PERMANENT;
	else if (matches(arg, "reachable") == 0)
		*state = NUD_REACHABLE;
	else if (strcmp(arg, "noarp") == 0)
		*state = NUD_NOARP;
	else if (strcmp(arg, "none") == 0)
		*state = NUD_NONE;
	else if (strcmp(arg, "stale") == 0)
		*state = NUD_STALE;
	else if (strcmp(arg, "incomplete") == 0)
		*state = NUD_INCOMPLETE;
	else if (strcmp(arg, "delay") == 0)
		*state = NUD_DELAY;
	else if (strcmp(arg, "probe") == 0)
		*state = NUD_PROBE;
	else if (matches(arg, "failed") == 0)
		*state = NUD_FAILED;
	else {
		if (get_unsigned(state, arg, 0))
			return -1;
		if (*state >= 0x100 || (*state&((*state)-1)))
			return -1;
	}
	return 0;
}

static int flush_update(void)
{
	if (rtnl_send_check(&rth, filter.flushb, filter.flushp) < 0) {
		perror("Failed to send flush request");
		return -1;
	}
	filter.flushp = 0;
	return 0;
}


static int ipneigh_modify(int cmd, int flags, int argc, char **argv)
{
	struct {
		struct nlmsghdr	n;
		struct ndmsg		ndm;
		char			buf[256];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | flags,
		.n.nlmsg_type = cmd,
		.ndm.ndm_family = preferred_family,
		.ndm.ndm_state = NUD_PERMANENT,
	};
	char  *dev = NULL;
	int dst_ok = 0;
	int dev_ok = 0;
	int lladdr_ok = 0;
	char *lla = NULL;
	inet_prefix dst;

	while (argc > 0) {
		if (matches(*argv, "lladdr") == 0) {
			NEXT_ARG();
			if (lladdr_ok)
				duparg("lladdr", *argv);
			lla = *argv;
			lladdr_ok = 1;
		} else if (strcmp(*argv, "nud") == 0) {
			unsigned int state;

			NEXT_ARG();
			if (nud_state_a2n(&state, *argv))
				invarg("nud state is bad", *argv);
			req.ndm.ndm_state = state;
		} else if (matches(*argv, "proxy") == 0) {
			NEXT_ARG();
			if (matches(*argv, "help") == 0)
				usage();
			if (dst_ok)
				duparg("address", *argv);
			get_addr(&dst, *argv, preferred_family);
			dst_ok = 1;
			dev_ok = 1;
			req.ndm.ndm_flags |= NTF_PROXY;
		} else if (strcmp(*argv, "router") == 0) {
			req.ndm.ndm_flags |= NTF_ROUTER;
		} else if (matches(*argv, "extern_learn") == 0) {
			req.ndm.ndm_flags |= NTF_EXT_LEARNED;
		} else if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			dev = *argv;
			dev_ok = 1;
		} else if (matches(*argv, "protocol") == 0) {
			__u32 proto;

			NEXT_ARG();
			if (rtnl_rtprot_a2n(&proto, *argv))
				invarg("\"protocol\" value is invalid\n", *argv);
			if (addattr8(&req.n, sizeof(req), NDA_PROTOCOL, proto))
				return -1;
		} else {
			if (strcmp(*argv, "to") == 0) {
				NEXT_ARG();
			}
			if (matches(*argv, "help") == 0) {
				NEXT_ARG();
			}
			if (dst_ok)
				duparg2("to", *argv);
			get_addr(&dst, *argv, preferred_family);
			dst_ok = 1;
		}
		argc--; argv++;
	}
	if (!dev_ok || !dst_ok || dst.family == AF_UNSPEC) {
		fprintf(stderr, "Device and destination are required arguments.\n");
		exit(-1);
	}
	req.ndm.ndm_family = dst.family;
	if (addattr_l(&req.n, sizeof(req), NDA_DST, &dst.data, dst.bytelen) < 0)
		return -1;

	if (lla && strcmp(lla, "null")) {
		char llabuf[20];
		int l;

		l = ll_addr_a2n(llabuf, sizeof(llabuf), lla);
		if (l < 0)
			return -1;

		if (addattr_l(&req.n, sizeof(req), NDA_LLADDR, llabuf, l) < 0)
			return -1;
	}

	ll_init_map(&rth);

	if (dev) {
		req.ndm.ndm_ifindex = ll_name_to_index(dev);
		if (!req.ndm.ndm_ifindex)
			return nodev(dev);
	}

	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		exit(2);

	return 0;
}

static void print_cacheinfo(const struct nda_cacheinfo *ci)
{
	static int hz;

	if (!hz)
		hz = get_user_hz();

	if (ci->ndm_refcnt)
		print_uint(PRINT_ANY, "refcnt",
				" ref %u", ci->ndm_refcnt);

	print_uint(PRINT_ANY, "used", " used %u", ci->ndm_used / hz);
	print_uint(PRINT_ANY, "confirmed", "/%u", ci->ndm_confirmed / hz);
	print_uint(PRINT_ANY, "updated", "/%u", ci->ndm_updated / hz);
}

static void print_neigh_state(unsigned int nud)
{

	open_json_array(PRINT_JSON,
			is_json_context() ?  "state" : "");

#define PRINT_FLAG(f)						\
	if (nud & NUD_##f) {					\
		nud &= ~NUD_##f;				\
		print_string(PRINT_ANY, NULL, " %s", #f);	\
	}

	PRINT_FLAG(INCOMPLETE);
	PRINT_FLAG(REACHABLE);
	PRINT_FLAG(STALE);
	PRINT_FLAG(DELAY);
	PRINT_FLAG(PROBE);
	PRINT_FLAG(FAILED);
	PRINT_FLAG(NOARP);
	PRINT_FLAG(PERMANENT);
#undef PRINT_FLAG

	close_json_array(PRINT_JSON, NULL);
}

int print_neigh(struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE *)arg;
	struct ndmsg *r = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr *tb[NDA_MAX+1];
	static int logit = 1;
	__u8 protocol = 0;

	if (n->nlmsg_type != RTM_NEWNEIGH && n->nlmsg_type != RTM_DELNEIGH &&
	    n->nlmsg_type != RTM_GETNEIGH) {
		fprintf(stderr, "Not RTM_NEWNEIGH: %08x %08x %08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);

		return 0;
	}
	len -= NLMSG_LENGTH(sizeof(*r));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	if (filter.flushb && n->nlmsg_type != RTM_NEWNEIGH)
		return 0;

	if (filter.family && filter.family != r->ndm_family)
		return 0;
	if (filter.index && filter.index != r->ndm_ifindex)
		return 0;
	if (!(filter.state&r->ndm_state) &&
	    !(r->ndm_flags & NTF_PROXY) &&
	    !(r->ndm_flags & NTF_EXT_LEARNED) &&
	    (r->ndm_state || !(filter.state&0x100)) &&
	    (r->ndm_family != AF_DECnet))
		return 0;

	if (filter.master && !(n->nlmsg_flags & NLM_F_DUMP_FILTERED)) {
		if (logit) {
			logit = 0;
			fprintf(fp,
				"\nWARNING: Kernel does not support filtering by master device\n\n");
		}
	}

	parse_rtattr(tb, NDA_MAX, NDA_RTA(r), n->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

	if (inet_addr_match_rta(&filter.pfx, tb[NDA_DST]))
		return 0;

	if (tb[NDA_PROTOCOL])
		protocol = rta_getattr_u8(tb[NDA_PROTOCOL]);

	if (filter.protocol && filter.protocol != protocol)
		return 0;

	if (filter.unused_only && tb[NDA_CACHEINFO]) {
		struct nda_cacheinfo *ci = RTA_DATA(tb[NDA_CACHEINFO]);

		if (ci->ndm_refcnt)
			return 0;
	}

	if (filter.flushb) {
		struct nlmsghdr *fn;

		if (NLMSG_ALIGN(filter.flushp) + n->nlmsg_len > filter.flushe) {
			if (flush_update())
				return -1;
		}
		fn = (struct nlmsghdr *)(filter.flushb + NLMSG_ALIGN(filter.flushp));
		memcpy(fn, n, n->nlmsg_len);
		fn->nlmsg_type = RTM_DELNEIGH;
		fn->nlmsg_flags = NLM_F_REQUEST;
		fn->nlmsg_seq = ++rth.seq;
		filter.flushp = (((char *)fn) + n->nlmsg_len) - filter.flushb;
		filter.flushed++;
		if (show_stats < 2)
			return 0;
	}

	open_json_object(NULL);
	if (n->nlmsg_type == RTM_DELNEIGH)
		print_bool(PRINT_ANY, "deleted", "Deleted ", true);
	else if (n->nlmsg_type == RTM_GETNEIGH)
		print_null(PRINT_ANY, "miss", "%s ", "miss");

	if (tb[NDA_DST]) {
		const char *dst;
		int family = r->ndm_family;

		if (family == AF_BRIDGE) {
			if (RTA_PAYLOAD(tb[NDA_DST]) == sizeof(struct in6_addr))
				family = AF_INET6;
			else
				family = AF_INET;
		}

		dst = format_host_rta(family, tb[NDA_DST]);
		print_color_string(PRINT_ANY,
				   ifa_family_color(family),
				   "dst", "%s ", dst);
	}

	if (!filter.index && r->ndm_ifindex) {
		if (!is_json_context())
			fprintf(fp, "dev ");

		print_color_string(PRINT_ANY, COLOR_IFNAME,
				   "dev", "%s ",
				   ll_index_to_name(r->ndm_ifindex));
	}

	if (tb[NDA_LLADDR]) {
		const char *lladdr;
		SPRINT_BUF(b1);

		lladdr = ll_addr_n2a(RTA_DATA(tb[NDA_LLADDR]),
				     RTA_PAYLOAD(tb[NDA_LLADDR]),
				     ll_index_to_type(r->ndm_ifindex),
				     b1, sizeof(b1));

		if (!is_json_context())
			fprintf(fp, "lladdr ");

		print_color_string(PRINT_ANY, COLOR_MAC,
				   "lladdr", "%s", lladdr);
	}

	if (r->ndm_flags & NTF_ROUTER)
		print_null(PRINT_ANY, "router", " %s", "router");

	if (r->ndm_flags & NTF_PROXY)
		print_null(PRINT_ANY, "proxy", " %s", "proxy");

	if (r->ndm_flags & NTF_EXT_LEARNED)
		print_null(PRINT_ANY, "extern_learn", " %s ", "extern_learn");

	if (r->ndm_flags & NTF_OFFLOADED)
		print_null(PRINT_ANY, "offload", " %s", "offload");

	if (show_stats) {
		if (tb[NDA_CACHEINFO])
			print_cacheinfo(RTA_DATA(tb[NDA_CACHEINFO]));

		if (tb[NDA_PROBES])
			print_uint(PRINT_ANY, "probes", " probes %u",
				   rta_getattr_u32(tb[NDA_PROBES]));
	}

	if (r->ndm_state)
		print_neigh_state(r->ndm_state);

	if (protocol) {
		SPRINT_BUF(b1);

		print_string(PRINT_ANY, "protocol", " proto %s ",
			     rtnl_rtprot_n2a(protocol, b1, sizeof(b1)));
	}

	print_string(PRINT_FP, NULL, "\n", "");
	close_json_object();
	fflush(stdout);

	return 0;
}

void ipneigh_reset_filter(int ifindex)
{
	memset(&filter, 0, sizeof(filter));
	filter.state = ~0;
	filter.index = ifindex;
}

static int ipneigh_dump_filter(struct nlmsghdr *nlh, int reqlen)
{
	struct ndmsg *ndm = NLMSG_DATA(nlh);
	int err;

	ndm->ndm_flags = filter.ndm_flags;

	if (filter.index) {
		err = addattr32(nlh, reqlen, NDA_IFINDEX, filter.index);
		if (err)
			return err;
	}
	if (filter.master) {
		err = addattr32(nlh, reqlen, NDA_MASTER, filter.master);
		if (err)
			return err;
	}

	return 0;
}

static int do_show_or_flush(int argc, char **argv, int flush)
{
	char *filter_dev = NULL;
	int state_given = 0;

	ipneigh_reset_filter(0);

	if (!filter.family)
		filter.family = preferred_family;

	if (flush) {
		if (argc <= 0) {
			fprintf(stderr, "Flush requires arguments.\n");
			return -1;
		}
		filter.state = ~(NUD_PERMANENT|NUD_NOARP);
	} else
		filter.state = 0xFF & ~NUD_NOARP;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			if (filter_dev)
				duparg("dev", *argv);
			filter_dev = *argv;
		} else if (strcmp(*argv, "master") == 0) {
			int ifindex;

			NEXT_ARG();
			ifindex = ll_name_to_index(*argv);
			if (!ifindex)
				invarg("Device does not exist\n", *argv);
			filter.master = ifindex;
		} else if (strcmp(*argv, "vrf") == 0) {
			int ifindex;

			NEXT_ARG();
			ifindex = ll_name_to_index(*argv);
			if (!ifindex)
				invarg("Not a valid VRF name\n", *argv);
			if (!name_is_vrf(*argv))
				invarg("Not a valid VRF name\n", *argv);
			filter.master = ifindex;
		} else if (strcmp(*argv, "unused") == 0) {
			filter.unused_only = 1;
		} else if (strcmp(*argv, "nud") == 0) {
			unsigned int state;

			NEXT_ARG();
			if (!state_given) {
				state_given = 1;
				filter.state = 0;
			}
			if (nud_state_a2n(&state, *argv)) {
				if (strcmp(*argv, "all") != 0)
					invarg("nud state is bad", *argv);
				state = ~0;
				if (flush)
					state &= ~NUD_NOARP;
			}
			if (state == 0)
				state = 0x100;
			filter.state |= state;
		} else if (strcmp(*argv, "proxy") == 0) {
			filter.ndm_flags = NTF_PROXY;
		} else if (matches(*argv, "protocol") == 0) {
			__u32 prot;

			NEXT_ARG();
			if (rtnl_rtprot_a2n(&prot, *argv)) {
				if (strcmp(*argv, "all"))
					invarg("invalid \"protocol\"\n", *argv);
				prot = 0;
			}
			filter.protocol = prot;
		} else {
			if (strcmp(*argv, "to") == 0) {
				NEXT_ARG();
			}
			if (matches(*argv, "help") == 0)
				usage();
			if (get_prefix(&filter.pfx, *argv, filter.family))
				invarg("to value is invalid\n", *argv);
			if (filter.family == AF_UNSPEC)
				filter.family = filter.pfx.family;
		}
		argc--; argv++;
	}

	ll_init_map(&rth);

	if (filter_dev) {
		filter.index = ll_name_to_index(filter_dev);
		if (!filter.index)
			return nodev(filter_dev);
	}

	if (flush) {
		int round = 0;
		char flushb[4096-512];

		filter.flushb = flushb;
		filter.flushp = 0;
		filter.flushe = sizeof(flushb);

		while (round < MAX_ROUNDS) {
			if (rtnl_neighdump_req(&rth, filter.family,
					       ipneigh_dump_filter) < 0) {
				perror("Cannot send dump request");
				exit(1);
			}
			filter.flushed = 0;
			if (rtnl_dump_filter(&rth, print_neigh, stdout) < 0) {
				fprintf(stderr, "Flush terminated\n");
				exit(1);
			}
			if (filter.flushed == 0) {
				if (show_stats) {
					if (round == 0)
						printf("Nothing to flush.\n");
					else
						printf("*** Flush is complete after %d round%s ***\n", round, round > 1?"s":"");
				}
				fflush(stdout);
				return 0;
			}
			round++;
			if (flush_update() < 0)
				exit(1);
			if (show_stats) {
				printf("\n*** Round %d, deleting %d entries ***\n", round, filter.flushed);
				fflush(stdout);
			}
			filter.state &= ~NUD_FAILED;
		}
		printf("*** Flush not complete bailing out after %d rounds\n",
			MAX_ROUNDS);
		return 1;
	}

	if (rtnl_neighdump_req(&rth, filter.family, ipneigh_dump_filter) < 0) {
		perror("Cannot send dump request");
		exit(1);
	}

	new_json_obj(json);
	if (rtnl_dump_filter(&rth, print_neigh, stdout) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}
	delete_json_obj();

	return 0;
}

static int ipneigh_get(int argc, char **argv)
{
	struct {
		struct nlmsghdr	n;
		struct ndmsg		ndm;
		char			buf[1024];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST,
		.n.nlmsg_type = RTM_GETNEIGH,
		.ndm.ndm_family = preferred_family,
	};
	struct nlmsghdr *answer;
	char  *d = NULL;
	int dst_ok = 0;
	int dev_ok = 0;
	inet_prefix dst;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			d = *argv;
			dev_ok = 1;
		} else if (matches(*argv, "proxy") == 0) {
			NEXT_ARG();
			if (matches(*argv, "help") == 0)
				usage();
			if (dst_ok)
				duparg("address", *argv);
			get_addr(&dst, *argv, preferred_family);
			dst_ok = 1;
			dev_ok = 1;
			req.ndm.ndm_flags |= NTF_PROXY;
		} else {
			if (strcmp(*argv, "to") == 0)
				NEXT_ARG();

			if (matches(*argv, "help") == 0)
				usage();
			if (dst_ok)
				duparg2("to", *argv);
			get_addr(&dst, *argv, preferred_family);
			dst_ok = 1;
		}
		argc--; argv++;
	}

	if (!dev_ok || !dst_ok || dst.family == AF_UNSPEC) {
		fprintf(stderr, "Device and address are required arguments.\n");
		return -1;
	}

	req.ndm.ndm_family = dst.family;
	if (addattr_l(&req.n, sizeof(req), NDA_DST, &dst.data, dst.bytelen) < 0)
		return -1;

	if (d) {
		req.ndm.ndm_ifindex = ll_name_to_index(d);
		if (!req.ndm.ndm_ifindex) {
			fprintf(stderr, "Cannot find device \"%s\"\n", d);
			return -1;
		}
	}

	if (rtnl_talk(&rth, &req.n, &answer) < 0)
		return -2;

	ipneigh_reset_filter(0);
	if (print_neigh(answer, stdout) < 0) {
		fprintf(stderr, "An error :-)\n");
		return -1;
	}

	return 0;
}

int do_ipneigh(int argc, char **argv)
{
	if (argc > 0) {
		if (matches(*argv, "add") == 0)
			return ipneigh_modify(RTM_NEWNEIGH, NLM_F_CREATE|NLM_F_EXCL, argc-1, argv+1);
		if (matches(*argv, "change") == 0 ||
		    strcmp(*argv, "chg") == 0)
			return ipneigh_modify(RTM_NEWNEIGH, NLM_F_REPLACE, argc-1, argv+1);
		if (matches(*argv, "replace") == 0)
			return ipneigh_modify(RTM_NEWNEIGH, NLM_F_CREATE|NLM_F_REPLACE, argc-1, argv+1);
		if (matches(*argv, "delete") == 0)
			return ipneigh_modify(RTM_DELNEIGH, 0, argc-1, argv+1);
		if (matches(*argv, "get") == 0)
			return ipneigh_get(argc-1, argv+1);
		if (matches(*argv, "show") == 0 ||
		    matches(*argv, "lst") == 0 ||
		    matches(*argv, "list") == 0)
			return do_show_or_flush(argc-1, argv+1, 0);
		if (matches(*argv, "flush") == 0)
			return do_show_or_flush(argc-1, argv+1, 1);
		if (matches(*argv, "help") == 0)
			usage();
	} else
		return do_show_or_flush(0, NULL, 0);

	fprintf(stderr, "Command \"%s\" is unknown, try \"ip neigh help\".\n", *argv);
	exit(-1);
}
