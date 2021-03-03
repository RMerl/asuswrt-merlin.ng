/*
 * iprule.c		"ip rule".
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <linux/if.h>
#include <linux/fib_rules.h>
#include <errno.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include "json_print.h"

enum list_action {
	IPRULE_LIST,
	IPRULE_FLUSH,
	IPRULE_SAVE,
};

extern struct rtnl_handle rth;

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr,
		"Usage: ip rule { add | del } SELECTOR ACTION\n"
		"       ip rule { flush | save | restore }\n"
		"       ip rule [ list [ SELECTOR ]]\n"
		"SELECTOR := [ not ] [ from PREFIX ] [ to PREFIX ] [ tos TOS ] [ fwmark FWMARK[/MASK] ]\n"
		"            [ iif STRING ] [ oif STRING ] [ pref NUMBER ] [ l3mdev ]\n"
		"            [ uidrange NUMBER-NUMBER ]\n"
		"            [ ipproto PROTOCOL ]\n"
		"            [ sport [ NUMBER | NUMBER-NUMBER ]\n"
		"            [ dport [ NUMBER | NUMBER-NUMBER ] ]\n"
		"ACTION := [ table TABLE_ID ]\n"
		"          [ protocol PROTO ]\n"
		"          [ nat ADDRESS ]\n"
		"          [ realms [SRCREALM/]DSTREALM ]\n"
		"          [ goto NUMBER ]\n"
		"          SUPPRESSOR\n"
		"SUPPRESSOR := [ suppress_prefixlength NUMBER ]\n"
		"              [ suppress_ifgroup DEVGROUP ]\n"
		"TABLE_ID := [ local | main | default | NUMBER ]\n");
	exit(-1);
}

static struct
{
	int not;
	int l3mdev;
	int iifmask, oifmask, uidrange;
	unsigned int tb;
	unsigned int tos, tosmask;
	unsigned int pref, prefmask;
	unsigned int fwmark, fwmask;
	uint64_t tun_id;
	char iif[IFNAMSIZ];
	char oif[IFNAMSIZ];
	struct fib_rule_uid_range range;
	inet_prefix src;
	inet_prefix dst;
	int protocol;
	int protocolmask;
	struct fib_rule_port_range sport;
	struct fib_rule_port_range dport;
	__u8 ipproto;
} filter;

static inline int frh_get_table(struct fib_rule_hdr *frh, struct rtattr **tb)
{
	__u32 table = frh->table;
	if (tb[RTA_TABLE])
		table = rta_getattr_u32(tb[RTA_TABLE]);
	return table;
}

static bool filter_nlmsg(struct nlmsghdr *n, struct rtattr **tb, int host_len)
{
	struct fib_rule_hdr *frh = NLMSG_DATA(n);
	__u32 table;

	if (preferred_family != AF_UNSPEC && frh->family != preferred_family)
		return false;

	if (filter.prefmask &&
	    filter.pref ^ (tb[FRA_PRIORITY] ? rta_getattr_u32(tb[FRA_PRIORITY]) : 0))
		return false;
	if (filter.not && !(frh->flags & FIB_RULE_INVERT))
		return false;

	if (filter.src.family) {
		inet_prefix *f_src = &filter.src;

		if (f_src->family != frh->family ||
		    f_src->bitlen > frh->src_len)
			return false;

		if (inet_addr_match_rta(f_src, tb[FRA_SRC]))
			return false;
	}

	if (filter.dst.family) {
		inet_prefix *f_dst = &filter.dst;

		if (f_dst->family != frh->family ||
		    f_dst->bitlen > frh->dst_len)
			return false;

		if (inet_addr_match_rta(f_dst, tb[FRA_DST]))
			return false;
	}

	if (filter.tosmask && filter.tos ^ frh->tos)
		return false;

	if (filter.fwmark) {
		__u32 mark = 0;

		if (tb[FRA_FWMARK])
			mark = rta_getattr_u32(tb[FRA_FWMARK]);
		if (filter.fwmark ^ mark)
			return false;
	}
	if (filter.fwmask) {
		__u32 mask = 0;

		if (tb[FRA_FWMASK])
			mask = rta_getattr_u32(tb[FRA_FWMASK]);
		if (filter.fwmask ^ mask)
			return false;
	}

	if (filter.iifmask) {
		if (tb[FRA_IFNAME]) {
			if (strcmp(filter.iif, rta_getattr_str(tb[FRA_IFNAME])) != 0)
				return false;
		} else {
			return false;
		}
	}

	if (filter.oifmask) {
		if (tb[FRA_OIFNAME]) {
			if (strcmp(filter.oif, rta_getattr_str(tb[FRA_OIFNAME])) != 0)
				return false;
		} else {
			return false;
		}
	}

	if (filter.l3mdev && !(tb[FRA_L3MDEV] && rta_getattr_u8(tb[FRA_L3MDEV])))
		return false;

	if (filter.uidrange) {
		struct fib_rule_uid_range *r = RTA_DATA(tb[FRA_UID_RANGE]);

		if (!tb[FRA_UID_RANGE] ||
		    r->start != filter.range.start ||
		    r->end != filter.range.end)
			return false;
	}

	if (filter.ipproto) {
		__u8 ipproto = 0;

		if (tb[FRA_IP_PROTO])
			ipproto = rta_getattr_u8(tb[FRA_IP_PROTO]);
		if (filter.ipproto != ipproto)
			return false;
	}

	if (filter.sport.start) {
		const struct fib_rule_port_range *r;

		if (!tb[FRA_SPORT_RANGE])
			return false;

		r = RTA_DATA(tb[FRA_SPORT_RANGE]);
		if (r->start != filter.sport.start ||
		    r->end != filter.sport.end)
			return false;
	}

	if (filter.dport.start) {
		const struct fib_rule_port_range *r;

		if (!tb[FRA_DPORT_RANGE])
			return false;

		r = RTA_DATA(tb[FRA_DPORT_RANGE]);
		if (r->start != filter.dport.start ||
		    r->end != filter.dport.end)
			return false;
	}

	if (filter.tun_id) {
		__u64 tun_id = 0;

		if (tb[FRA_TUN_ID]) {
			tun_id = ntohll(rta_getattr_u64(tb[FRA_TUN_ID]));
			if (filter.tun_id != tun_id)
				return false;
		} else {
			return false;
		}
	}

	table = frh_get_table(frh, tb);
	if (filter.tb > 0 && filter.tb ^ table)
		return false;

	return true;
}

int print_rule(struct nlmsghdr *n, void *arg)
{
	FILE *fp = arg;
	struct fib_rule_hdr *frh = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	int host_len = -1;
	__u32 table, prio = 0;
	struct rtattr *tb[FRA_MAX+1];
	SPRINT_BUF(b1);

	if (n->nlmsg_type != RTM_NEWRULE && n->nlmsg_type != RTM_DELRULE)
		return 0;

	len -= NLMSG_LENGTH(sizeof(*frh));
	if (len < 0)
		return -1;

	parse_rtattr(tb, FRA_MAX, RTM_RTA(frh), len);

	host_len = af_bit_len(frh->family);

	if (!filter_nlmsg(n, tb, host_len))
		return 0;

	open_json_object(NULL);
	if (n->nlmsg_type == RTM_DELRULE)
		print_bool(PRINT_ANY, "deleted", "Deleted ", true);

	if (tb[FRA_PRIORITY])
		prio = rta_getattr_u32(tb[FRA_PRIORITY]);

	print_uint(PRINT_ANY, "priority", "%u:\t", prio);

	if (frh->flags & FIB_RULE_INVERT)
		print_null(PRINT_ANY, "not", "not ", NULL);

	if (tb[FRA_SRC]) {
		const char *src = rt_addr_n2a_rta(frh->family, tb[FRA_SRC]);

		print_string(PRINT_FP, NULL, "from ", NULL);
		print_color_string(PRINT_ANY, ifa_family_color(frh->family),
				   "src", "%s", src);
		if (frh->src_len != host_len)
			print_uint(PRINT_ANY, "srclen", "/%u", frh->src_len);
	} else if (frh->src_len) {
		print_string(PRINT_ANY, "src", "from %s", "0");
		print_uint(PRINT_ANY, "srclen", "/%u", frh->src_len);
	} else {
		print_string(PRINT_ANY, "src", "from %s", "all");
	}

	if (tb[FRA_DST]) {
		const char *dst = rt_addr_n2a_rta(frh->family, tb[FRA_DST]);

		print_string(PRINT_FP, NULL, " to ", NULL);
		print_color_string(PRINT_ANY, ifa_family_color(frh->family),
				   "dst", "%s", dst);
		if (frh->dst_len != host_len)
			print_uint(PRINT_ANY, "dstlen", "/%u", frh->dst_len);
	} else if (frh->dst_len) {
		print_string(PRINT_ANY, "dst", " to %s", "0");
		print_uint(PRINT_ANY, "dstlen", "/%u", frh->dst_len);
	}

	if (frh->tos) {
		print_string(PRINT_ANY, "tos",
			     " tos %s",
			     rtnl_dsfield_n2a(frh->tos, b1, sizeof(b1)));
	}

	if (tb[FRA_FWMARK] || tb[FRA_FWMASK]) {
		__u32 mark = 0, mask = 0;

		if (tb[FRA_FWMARK])
			mark = rta_getattr_u32(tb[FRA_FWMARK]);

		if (tb[FRA_FWMASK] &&
		    (mask = rta_getattr_u32(tb[FRA_FWMASK])) != 0xFFFFFFFF) {
			print_0xhex(PRINT_ANY, "fwmark", " fwmark %#llx", mark);
			print_0xhex(PRINT_ANY, "fwmask", "/%#llx", mask);
		} else {
			print_0xhex(PRINT_ANY, "fwmark", " fwmark %#llx", mark);
		}
	}

	if (tb[FRA_IFNAME]) {
		if (!is_json_context())
			fprintf(fp, " iif ");
		print_color_string(PRINT_ANY, COLOR_IFNAME,
				   "iif", "%s",
				   rta_getattr_str(tb[FRA_IFNAME]));

		if (frh->flags & FIB_RULE_IIF_DETACHED)
			print_null(PRINT_ANY, "iif_detached", " [detached]",
				   NULL);
	}

	if (tb[FRA_OIFNAME]) {
		if (!is_json_context())
			fprintf(fp, " oif ");

		print_color_string(PRINT_ANY, COLOR_IFNAME, "oif", "%s",
				   rta_getattr_str(tb[FRA_OIFNAME]));

		if (frh->flags & FIB_RULE_OIF_DETACHED)
			print_null(PRINT_ANY, "oif_detached", " [detached]",
				   NULL);
	}

	if (tb[FRA_L3MDEV]) {
		__u8 mdev = rta_getattr_u8(tb[FRA_L3MDEV]);

		if (mdev)
			print_null(PRINT_ANY, "l3mdev",
				   " lookup [l3mdev-table]", NULL);
	}

	if (tb[FRA_UID_RANGE]) {
		struct fib_rule_uid_range *r = RTA_DATA(tb[FRA_UID_RANGE]);

		print_uint(PRINT_ANY, "uid_start", " uidrange %u", r->start);
		print_uint(PRINT_ANY, "uid_end", "-%u", r->end);
	}

	if (tb[FRA_IP_PROTO]) {
		SPRINT_BUF(pbuf);
		print_string(PRINT_ANY, "ipproto", " ipproto %s",
			     inet_proto_n2a(rta_getattr_u8(tb[FRA_IP_PROTO]),
					    pbuf, sizeof(pbuf)));
	}

	if (tb[FRA_SPORT_RANGE]) {
		struct fib_rule_port_range *r = RTA_DATA(tb[FRA_SPORT_RANGE]);

		if (r->start == r->end) {
			print_uint(PRINT_ANY, "sport", " sport %u", r->start);
		} else {
			print_uint(PRINT_ANY, "sport_start", " sport %u",
				   r->start);
			print_uint(PRINT_ANY, "sport_end", "-%u", r->end);
		}
	}

	if (tb[FRA_DPORT_RANGE]) {
		struct fib_rule_port_range *r = RTA_DATA(tb[FRA_DPORT_RANGE]);

		if (r->start == r->end) {
			print_uint(PRINT_ANY, "dport", " dport %u", r->start);
		} else {
			print_uint(PRINT_ANY, "dport_start", " dport %u",
				   r->start);
			print_uint(PRINT_ANY, "dport_end", "-%u", r->end);
		}
	}

	if (tb[FRA_TUN_ID]) {
		__u64 tun_id = ntohll(rta_getattr_u64(tb[FRA_TUN_ID]));

		print_u64(PRINT_ANY, "tun_id", " tun_id %llu", tun_id);
	}

	table = frh_get_table(frh, tb);
	if (table) {
		print_string(PRINT_ANY, "table",
			     " lookup %s",
			     rtnl_rttable_n2a(table, b1, sizeof(b1)));

		if (tb[FRA_SUPPRESS_PREFIXLEN]) {
			int pl = rta_getattr_u32(tb[FRA_SUPPRESS_PREFIXLEN]);

			if (pl != -1)
				print_int(PRINT_ANY, "suppress_prefixlen",
					  " suppress_prefixlength %d", pl);
		}

		if (tb[FRA_SUPPRESS_IFGROUP]) {
			int group = rta_getattr_u32(tb[FRA_SUPPRESS_IFGROUP]);

			if (group != -1) {
				const char *grname
					= rtnl_group_n2a(group, b1, sizeof(b1));

				print_string(PRINT_ANY, "suppress_ifgroup",
					     " suppress_ifgroup %s", grname);
			}
		}
	}

	if (tb[FRA_FLOW]) {
		__u32 to = rta_getattr_u32(tb[FRA_FLOW]);
		__u32 from = to>>16;

		to &= 0xFFFF;
		if (from)
			print_string(PRINT_ANY,
				     "flow_from", " realms %s/",
				     rtnl_rtrealm_n2a(from, b1, sizeof(b1)));
		else
			print_string(PRINT_FP, NULL, " realms ", NULL);

		print_string(PRINT_ANY, "flow_to", "%s",
			     rtnl_rtrealm_n2a(to, b1, sizeof(b1)));
	}

	if (frh->action == RTN_NAT) {
		if (tb[RTA_GATEWAY]) {
			const char *gateway;

			gateway = format_host_rta(frh->family, tb[RTA_GATEWAY]);

			print_string(PRINT_ANY, "nat_gateway",
				     " map-to %s", gateway);
		} else {
			print_null(PRINT_ANY, "masquerade", " masquerade", NULL);
		}
	} else if (frh->action == FR_ACT_GOTO) {
		if (tb[FRA_GOTO])
			print_uint(PRINT_ANY, "goto", " goto %u",
				   rta_getattr_u32(tb[FRA_GOTO]));
		else
			print_string(PRINT_ANY, "goto", " goto %s", "none");

		if (frh->flags & FIB_RULE_UNRESOLVED)
			print_null(PRINT_ANY, "unresolved",
				   " [unresolved]", NULL);
	} else if (frh->action == FR_ACT_NOP) {
		print_null(PRINT_ANY, "nop", " nop", NULL);
	} else if (frh->action != FR_ACT_TO_TBL) {
		print_string(PRINT_ANY, "action", " %s",
			     rtnl_rtntype_n2a(frh->action, b1, sizeof(b1)));
	}

	if (tb[FRA_PROTOCOL]) {
		__u8 protocol = rta_getattr_u8(tb[FRA_PROTOCOL]);

		if ((protocol && protocol != RTPROT_KERNEL) || show_details > 0) {
			print_string(PRINT_ANY, "protocol", " proto %s",
				     rtnl_rtprot_n2a(protocol, b1, sizeof(b1)));
		}
	}
	print_string(PRINT_FP, NULL, "\n", "");
	close_json_object();
	fflush(fp);
	return 0;
}

static __u32 rule_dump_magic = 0x71706986;

static int save_rule_prep(void)
{
	int ret;

	if (isatty(STDOUT_FILENO)) {
		fprintf(stderr, "Not sending a binary stream to stdout\n");
		return -1;
	}

	ret = write(STDOUT_FILENO, &rule_dump_magic, sizeof(rule_dump_magic));
	if (ret != sizeof(rule_dump_magic)) {
		fprintf(stderr, "Can't write magic to dump file\n");
		return -1;
	}

	return 0;
}

static int save_rule(struct nlmsghdr *n, void *arg)
{
	int ret;

	ret = write(STDOUT_FILENO, n, n->nlmsg_len);
	if ((ret > 0) && (ret != n->nlmsg_len)) {
		fprintf(stderr, "Short write while saving nlmsg\n");
		ret = -EIO;
	}

	return ret == n->nlmsg_len ? 0 : ret;
}

static int flush_rule(struct nlmsghdr *n, void *arg)
{
	struct rtnl_handle rth2;
	struct fib_rule_hdr *frh = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr *tb[FRA_MAX+1];
	int host_len = -1;

	len -= NLMSG_LENGTH(sizeof(*frh));
	if (len < 0)
		return -1;

	parse_rtattr(tb, FRA_MAX, RTM_RTA(frh), len);

	host_len = af_bit_len(frh->family);
	if (!filter_nlmsg(n, tb, host_len))
		return 0;

	if (tb[FRA_PROTOCOL]) {
		__u8 protocol = rta_getattr_u8(tb[FRA_PROTOCOL]);

		if ((filter.protocol ^ protocol) & filter.protocolmask)
			return 0;
	}

	if (tb[FRA_PRIORITY]) {
		n->nlmsg_type = RTM_DELRULE;
		n->nlmsg_flags = NLM_F_REQUEST;

		if (rtnl_open(&rth2, 0) < 0)
			return -1;

		if (rtnl_talk(&rth2, n, NULL) < 0)
			return -2;

		rtnl_close(&rth2);
	}

	return 0;
}

static int iprule_list_flush_or_save(int argc, char **argv, int action)
{
	rtnl_filter_t filter_fn;
	int af = preferred_family;

	if (af == AF_UNSPEC)
		af = AF_INET;

	if (action == IPRULE_SAVE && argc > 0) {
		fprintf(stderr, "\"ip rule save\" does not take any arguments.\n");
		return -1;
	}

	switch (action) {
	case IPRULE_SAVE:
		if (save_rule_prep())
			return -1;
		filter_fn = save_rule;
		break;
	case IPRULE_FLUSH:
		filter_fn = flush_rule;
		break;
	default:
		filter_fn = print_rule;
	}

	memset(&filter, 0, sizeof(filter));

	while (argc > 0) {
		if (matches(*argv, "preference") == 0 ||
		    matches(*argv, "order") == 0 ||
		    matches(*argv, "priority") == 0) {
			__u32 pref;

			NEXT_ARG();
			if (get_u32(&pref, *argv, 0))
				invarg("preference value is invalid\n", *argv);
			filter.pref = pref;
			filter.prefmask = 1;
		} else if (strcmp(*argv, "not") == 0) {
			filter.not = 1;
		} else if (strcmp(*argv, "tos") == 0) {
			__u32 tos;

			NEXT_ARG();
			if (rtnl_dsfield_a2n(&tos, *argv))
				invarg("TOS value is invalid\n", *argv);
			filter.tos = tos;
			filter.tosmask = 1;
		} else if (strcmp(*argv, "fwmark") == 0) {
			char *slash;
			__u32 fwmark, fwmask;

			NEXT_ARG();
			slash = strchr(*argv, '/');
			if (slash != NULL)
				*slash = '\0';
			if (get_u32(&fwmark, *argv, 0))
				invarg("fwmark value is invalid\n", *argv);
			filter.fwmark = fwmark;
			if (slash) {
				if (get_u32(&fwmask, slash+1, 0))
					invarg("fwmask value is invalid\n",
					       slash+1);
				filter.fwmask = fwmask;
			}
		} else if (strcmp(*argv, "dev") == 0 ||
			   strcmp(*argv, "iif") == 0) {
			NEXT_ARG();
			if (get_ifname(filter.iif, *argv))
				invarg("\"iif\"/\"dev\" not a valid ifname", *argv);
			filter.iifmask = 1;
		} else if (strcmp(*argv, "oif") == 0) {
			NEXT_ARG();
			if (get_ifname(filter.oif, *argv))
				invarg("\"oif\" not a valid ifname", *argv);
			filter.oifmask = 1;
		} else if (strcmp(*argv, "l3mdev") == 0) {
			filter.l3mdev = 1;
		} else if (strcmp(*argv, "uidrange") == 0) {
			NEXT_ARG();
			filter.uidrange = 1;
			if (sscanf(*argv, "%u-%u",
				   &filter.range.start,
				   &filter.range.end) != 2)
				invarg("invalid UID range\n", *argv);

		} else if (matches(*argv, "tun_id") == 0) {
			__u64 tun_id;

			NEXT_ARG();
			if (get_u64(&tun_id, *argv, 0))
				invarg("\"tun_id\" value is invalid\n", *argv);
			filter.tun_id = tun_id;
		} else if (matches(*argv, "lookup") == 0 ||
			   matches(*argv, "table") == 0) {
			__u32 tid;

			NEXT_ARG();
			if (rtnl_rttable_a2n(&tid, *argv))
				invarg("table id value is invalid\n", *argv);
			filter.tb = tid;
		} else if (matches(*argv, "from") == 0 ||
			   matches(*argv, "src") == 0) {
			NEXT_ARG();
			if (get_prefix(&filter.src, *argv, af))
				invarg("from value is invalid\n", *argv);
		} else if (matches(*argv, "protocol") == 0) {
			__u32 prot;
			NEXT_ARG();
			filter.protocolmask = -1;
			if (rtnl_rtprot_a2n(&prot, *argv)) {
				if (strcmp(*argv, "all") != 0)
					invarg("invalid \"protocol\"\n", *argv);
				prot = 0;
				filter.protocolmask = 0;
			}
			filter.protocol = prot;
		} else if (strcmp(*argv, "ipproto") == 0) {
			int ipproto;

			NEXT_ARG();
			ipproto = inet_proto_a2n(*argv);
			if (ipproto < 0)
				invarg("Invalid \"ipproto\" value\n", *argv);
			filter.ipproto = ipproto;
		} else if (strcmp(*argv, "sport") == 0) {
			struct fib_rule_port_range r;
			int ret;

			NEXT_ARG();
			ret = sscanf(*argv, "%hu-%hu", &r.start, &r.end);
			if (ret == 1)
				r.end = r.start;
			else if (ret != 2)
				invarg("invalid port range\n", *argv);
			filter.sport = r;
		} else if (strcmp(*argv, "dport") == 0) {
			struct fib_rule_port_range r;
			int ret;

			NEXT_ARG();
			ret = sscanf(*argv, "%hu-%hu", &r.start, &r.end);
			if (ret == 1)
				r.end = r.start;
			else if (ret != 2)
				invarg("invalid dport range\n", *argv);
			filter.dport = r;
		} else{
			if (matches(*argv, "dst") == 0 ||
			    matches(*argv, "to") == 0) {
				NEXT_ARG();
			}
			if (get_prefix(&filter.dst, *argv, af))
				invarg("to value is invalid\n", *argv);
		}
		argc--; argv++;
	}

	if (rtnl_ruledump_req(&rth, af) < 0) {
		perror("Cannot send dump request");
		return 1;
	}

	new_json_obj(json);
	if (rtnl_dump_filter(&rth, filter_fn, stdout) < 0) {
		fprintf(stderr, "Dump terminated\n");
		return 1;
	}
	delete_json_obj();

	return 0;
}

static int rule_dump_check_magic(void)
{
	int ret;
	__u32 magic = 0;

	if (isatty(STDIN_FILENO)) {
		fprintf(stderr, "Can't restore rule dump from a terminal\n");
		return -1;
	}

	ret = fread(&magic, sizeof(magic), 1, stdin);
	if (magic != rule_dump_magic) {
		fprintf(stderr, "Magic mismatch (%d elems, %x magic)\n",
			ret, magic);
		return -1;
	}

	return 0;
}

static int restore_handler(struct rtnl_ctrl_data *ctrl,
			   struct nlmsghdr *n, void *arg)
{
	int ret;

	n->nlmsg_flags |= NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;

	ll_init_map(&rth);

	ret = rtnl_talk(&rth, n, NULL);
	if ((ret < 0) && (errno == EEXIST))
		ret = 0;

	return ret;
}


static int iprule_restore(void)
{
	if (rule_dump_check_magic())
		exit(-1);

	exit(rtnl_from_file(stdin, &restore_handler, NULL));
}

static int iprule_modify(int cmd, int argc, char **argv)
{
	int l3mdev_rule = 0;
	int table_ok = 0;
	__u32 tid = 0;
	struct {
		struct nlmsghdr	n;
		struct fib_rule_hdr	frh;
		char			buf[1024];
	} req = {
		.n.nlmsg_type = cmd,
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct fib_rule_hdr)),
		.n.nlmsg_flags = NLM_F_REQUEST,
		.frh.family = preferred_family,
		.frh.action = FR_ACT_UNSPEC,
	};

	if (cmd == RTM_NEWRULE) {
		if (argc == 0) {
			fprintf(stderr,
				"\"ip rule add\" requires arguments.\n");
			return -1;
		}
		req.n.nlmsg_flags |= NLM_F_CREATE|NLM_F_EXCL;
		req.frh.action = FR_ACT_TO_TBL;
	}

	if (cmd == RTM_DELRULE && argc == 0) {
		fprintf(stderr, "\"ip rule del\" requires arguments.\n");
		return -1;
	}

	while (argc > 0) {
		if (strcmp(*argv, "not") == 0) {
			req.frh.flags |= FIB_RULE_INVERT;
		} else if (strcmp(*argv, "from") == 0) {
			inet_prefix dst;

			NEXT_ARG();
			get_prefix(&dst, *argv, req.frh.family);
			req.frh.src_len = dst.bitlen;
			addattr_l(&req.n, sizeof(req), FRA_SRC,
				  &dst.data, dst.bytelen);
		} else if (strcmp(*argv, "to") == 0) {
			inet_prefix dst;

			NEXT_ARG();
			get_prefix(&dst, *argv, req.frh.family);
			req.frh.dst_len = dst.bitlen;
			addattr_l(&req.n, sizeof(req), FRA_DST,
				  &dst.data, dst.bytelen);
		} else if (matches(*argv, "preference") == 0 ||
			   matches(*argv, "order") == 0 ||
			   matches(*argv, "priority") == 0) {
			__u32 pref;

			NEXT_ARG();
			if (get_u32(&pref, *argv, 0))
				invarg("preference value is invalid\n", *argv);
			addattr32(&req.n, sizeof(req), FRA_PRIORITY, pref);
		} else if (strcmp(*argv, "tos") == 0 ||
			   matches(*argv, "dsfield") == 0) {
			__u32 tos;

			NEXT_ARG();
			if (rtnl_dsfield_a2n(&tos, *argv))
				invarg("TOS value is invalid\n", *argv);
			req.frh.tos = tos;
		} else if (strcmp(*argv, "fwmark") == 0) {
			char *slash;
			__u32 fwmark, fwmask;

			NEXT_ARG();

			slash = strchr(*argv, '/');
			if (slash != NULL)
				*slash = '\0';
			if (get_u32(&fwmark, *argv, 0))
				invarg("fwmark value is invalid\n", *argv);
			addattr32(&req.n, sizeof(req), FRA_FWMARK, fwmark);
			if (slash) {
				if (get_u32(&fwmask, slash+1, 0))
					invarg("fwmask value is invalid\n",
					       slash+1);
				addattr32(&req.n, sizeof(req),
					  FRA_FWMASK, fwmask);
			}
		} else if (matches(*argv, "realms") == 0) {
			__u32 realm;

			NEXT_ARG();
			if (get_rt_realms_or_raw(&realm, *argv))
				invarg("invalid realms\n", *argv);
			addattr32(&req.n, sizeof(req), FRA_FLOW, realm);
		} else if (matches(*argv, "protocol") == 0) {
			__u32 proto;

			NEXT_ARG();
			if (rtnl_rtprot_a2n(&proto, *argv))
				invarg("\"protocol\" value is invalid\n", *argv);
			addattr8(&req.n, sizeof(req), FRA_PROTOCOL, proto);
		} else if (matches(*argv, "tun_id") == 0) {
			__u64 tun_id;

			NEXT_ARG();
			if (get_be64(&tun_id, *argv, 0))
				invarg("\"tun_id\" value is invalid\n", *argv);
			addattr64(&req.n, sizeof(req), FRA_TUN_ID, tun_id);
		} else if (matches(*argv, "table") == 0 ||
			   strcmp(*argv, "lookup") == 0) {
			NEXT_ARG();
			if (rtnl_rttable_a2n(&tid, *argv))
				invarg("invalid table ID\n", *argv);
			if (tid < 256)
				req.frh.table = tid;
			else {
				req.frh.table = RT_TABLE_UNSPEC;
				addattr32(&req.n, sizeof(req), FRA_TABLE, tid);
			}
			table_ok = 1;
		} else if (matches(*argv, "suppress_prefixlength") == 0 ||
			   strcmp(*argv, "sup_pl") == 0) {
			int pl;

			NEXT_ARG();
			if (get_s32(&pl, *argv, 0) || pl < 0)
				invarg("suppress_prefixlength value is invalid\n",
				       *argv);
			addattr32(&req.n, sizeof(req),
				  FRA_SUPPRESS_PREFIXLEN, pl);
		} else if (matches(*argv, "suppress_ifgroup") == 0 ||
			   strcmp(*argv, "sup_group") == 0) {
			NEXT_ARG();
			int group;

			if (rtnl_group_a2n(&group, *argv))
				invarg("Invalid \"suppress_ifgroup\" value\n",
				       *argv);
			addattr32(&req.n, sizeof(req),
				  FRA_SUPPRESS_IFGROUP, group);
		} else if (strcmp(*argv, "dev") == 0 ||
			   strcmp(*argv, "iif") == 0) {
			NEXT_ARG();
			if (check_ifname(*argv))
				invarg("\"iif\"/\"dev\" not a valid ifname", *argv);
			addattr_l(&req.n, sizeof(req), FRA_IFNAME,
				  *argv, strlen(*argv)+1);
		} else if (strcmp(*argv, "oif") == 0) {
			NEXT_ARG();
			if (check_ifname(*argv))
				invarg("\"oif\" not a valid ifname", *argv);
			addattr_l(&req.n, sizeof(req), FRA_OIFNAME,
				  *argv, strlen(*argv)+1);
		} else if (strcmp(*argv, "l3mdev") == 0) {
			addattr8(&req.n, sizeof(req), FRA_L3MDEV, 1);
			table_ok = 1;
			l3mdev_rule = 1;
		} else if (strcmp(*argv, "uidrange") == 0) {
			struct fib_rule_uid_range r;

			NEXT_ARG();
			if (sscanf(*argv, "%u-%u", &r.start, &r.end) != 2)
				invarg("invalid UID range\n", *argv);
			addattr_l(&req.n, sizeof(req), FRA_UID_RANGE, &r,
				  sizeof(r));
		} else if (strcmp(*argv, "nat") == 0 ||
			   matches(*argv, "map-to") == 0) {
			NEXT_ARG();
			fprintf(stderr, "Warning: route NAT is deprecated\n");
			addattr32(&req.n, sizeof(req), RTA_GATEWAY,
				  get_addr32(*argv));
			req.frh.action = RTN_NAT;
		} else if (strcmp(*argv, "ipproto") == 0) {
			int ipproto;

			NEXT_ARG();
			ipproto = inet_proto_a2n(*argv);
			if (ipproto < 0)
				invarg("Invalid \"ipproto\" value\n",
				       *argv);
			addattr8(&req.n, sizeof(req), FRA_IP_PROTO, ipproto);
		} else if (strcmp(*argv, "sport") == 0) {
			struct fib_rule_port_range r;
			int ret = 0;

			NEXT_ARG();
			ret = sscanf(*argv, "%hu-%hu", &r.start, &r.end);
			if (ret == 1)
				r.end = r.start;
			else if (ret != 2)
				invarg("invalid port range\n", *argv);
			addattr_l(&req.n, sizeof(req), FRA_SPORT_RANGE, &r,
				  sizeof(r));
		} else if (strcmp(*argv, "dport") == 0) {
			struct fib_rule_port_range r;
			int ret = 0;

			NEXT_ARG();
			ret = sscanf(*argv, "%hu-%hu", &r.start, &r.end);
			if (ret == 1)
				r.end = r.start;
			else if (ret != 2)
				invarg("invalid dport range\n", *argv);
			addattr_l(&req.n, sizeof(req), FRA_DPORT_RANGE, &r,
				  sizeof(r));
		} else {
			int type;

			if (strcmp(*argv, "type") == 0)
				NEXT_ARG();

			if (matches(*argv, "help") == 0)
				usage();
			else if (matches(*argv, "goto") == 0) {
				__u32 target;

				type = FR_ACT_GOTO;
				NEXT_ARG();
				if (get_u32(&target, *argv, 0))
					invarg("invalid target\n", *argv);
				addattr32(&req.n, sizeof(req),
					  FRA_GOTO, target);
			} else if (matches(*argv, "nop") == 0)
				type = FR_ACT_NOP;
			else if (rtnl_rtntype_a2n(&type, *argv))
				invarg("Failed to parse rule type", *argv);
			req.frh.action = type;
			table_ok = 1;
		}
		argc--;
		argv++;
	}

	if (l3mdev_rule && tid != 0) {
		fprintf(stderr,
			"table can not be specified for l3mdev rules\n");
		return -EINVAL;
	}

	if (req.frh.family == AF_UNSPEC)
		req.frh.family = AF_INET;

	if (!table_ok && cmd == RTM_NEWRULE)
		req.frh.table = RT_TABLE_MAIN;

	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		return -2;

	return 0;
}

int do_iprule(int argc, char **argv)
{
	if (argc < 1) {
		return iprule_list_flush_or_save(0, NULL, IPRULE_LIST);
	} else if (matches(argv[0], "list") == 0 ||
		   matches(argv[0], "lst") == 0 ||
		   matches(argv[0], "show") == 0) {
		return iprule_list_flush_or_save(argc-1, argv+1, IPRULE_LIST);
	} else if (matches(argv[0], "save") == 0) {
		return iprule_list_flush_or_save(argc-1, argv+1, IPRULE_SAVE);
	} else if (matches(argv[0], "restore") == 0) {
		return iprule_restore();
	} else if (matches(argv[0], "add") == 0) {
		return iprule_modify(RTM_NEWRULE, argc-1, argv+1);
	} else if (matches(argv[0], "delete") == 0) {
		return iprule_modify(RTM_DELRULE, argc-1, argv+1);
	} else if (matches(argv[0], "flush") == 0) {
		return iprule_list_flush_or_save(argc-1, argv+1, IPRULE_FLUSH);
	} else if (matches(argv[0], "help") == 0)
		usage();

	fprintf(stderr,
		"Command \"%s\" is unknown, try \"ip rule help\".\n", *argv);
	exit(-1);
}

int do_multirule(int argc, char **argv)
{
	switch (preferred_family) {
	case AF_UNSPEC:
	case AF_INET:
		preferred_family = RTNL_FAMILY_IPMR;
		break;
	case AF_INET6:
		preferred_family = RTNL_FAMILY_IP6MR;
		break;
	case RTNL_FAMILY_IPMR:
	case RTNL_FAMILY_IP6MR:
		break;
	default:
		fprintf(stderr,
			"Multicast rules are only supported for IPv4/IPv6, was: %i\n",
			preferred_family);
		exit(-1);
	}

	return do_iprule(argc, argv);
}
