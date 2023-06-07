// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/* -
 * m_ct.c     Connection tracking action
 *
 * Authors:   Paul Blakey <paulb@mellanox.com>
 *            Yossi Kuperman <yossiku@mellanox.com>
 *            Marcelo Ricardo Leitner <marcelo.leitner@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include "tc_util.h"
#include <linux/tc_act/tc_ct.h>

static void
usage(void)
{
	fprintf(stderr,
		"Usage: ct clear\n"
		"	ct commit [force] [zone ZONE] [mark MASKED_MARK] [label MASKED_LABEL] [nat NAT_SPEC]\n"
		"	ct [nat] [zone ZONE]\n"
		"Where: ZONE is the conntrack zone table number\n"
		"	NAT_SPEC is {src|dst} addr addr1[-addr2] [port port1[-port2]]\n"
		"\n");
	exit(-1);
}

static int ct_parse_nat_addr_range(const char *str, struct nlmsghdr *n)
{
	inet_prefix addr = { .family = AF_UNSPEC, };
	char *addr1, *addr2 = 0;
	SPRINT_BUF(buffer);
	int attr;
	int ret;

	strncpy(buffer, str, sizeof(buffer) - 1);

	addr1 = buffer;
	addr2 = strchr(addr1, '-');
	if (addr2) {
		*addr2 = '\0';
		addr2++;
	}

	ret = get_addr(&addr, addr1, AF_UNSPEC);
	if (ret)
		return ret;
	attr = addr.family == AF_INET ? TCA_CT_NAT_IPV4_MIN :
					TCA_CT_NAT_IPV6_MIN;
	addattr_l(n, MAX_MSG, attr, addr.data, addr.bytelen);

	if (addr2) {
		ret = get_addr(&addr, addr2, addr.family);
		if (ret)
			return ret;
	}
	attr = addr.family == AF_INET ? TCA_CT_NAT_IPV4_MAX :
					TCA_CT_NAT_IPV6_MAX;
	addattr_l(n, MAX_MSG, attr, addr.data, addr.bytelen);

	return 0;
}

static int ct_parse_nat_port_range(const char *str, struct nlmsghdr *n)
{
	char *port1, *port2 = 0;
	SPRINT_BUF(buffer);
	__be16 port;
	int ret;

	strncpy(buffer, str, sizeof(buffer) - 1);

	port1 = buffer;
	port2 = strchr(port1, '-');
	if (port2) {
		*port2 = '\0';
		port2++;
	}

	ret = get_be16(&port, port1, 10);
	if (ret)
		return -1;
	addattr16(n, MAX_MSG, TCA_CT_NAT_PORT_MIN, port);

	if (port2) {
		ret = get_be16(&port, port2, 10);
		if (ret)
			return -1;
	}
	addattr16(n, MAX_MSG, TCA_CT_NAT_PORT_MAX, port);

	return 0;
}


static int ct_parse_u16(char *str, int value_type, int mask_type,
			struct nlmsghdr *n)
{
	__u16 value, mask;
	char *slash = 0;

	if (mask_type != TCA_CT_UNSPEC) {
		slash = strchr(str, '/');
		if (slash)
			*slash = '\0';
	}

	if (get_u16(&value, str, 0))
		return -1;

	if (slash) {
		if (get_u16(&mask, slash + 1, 0))
			return -1;
	} else {
		mask = UINT16_MAX;
	}

	addattr16(n, MAX_MSG, value_type, value);
	if (mask_type != TCA_CT_UNSPEC)
		addattr16(n, MAX_MSG, mask_type, mask);

	return 0;
}

static int ct_parse_u32(char *str, int value_type, int mask_type,
			struct nlmsghdr *n)
{
	__u32 value, mask;
	char *slash;

	slash = strchr(str, '/');
	if (slash)
		*slash = '\0';

	if (get_u32(&value, str, 0))
		return -1;

	if (slash) {
		if (get_u32(&mask, slash + 1, 0))
			return -1;
	} else {
		mask = UINT32_MAX;
	}

	addattr32(n, MAX_MSG, value_type, value);
	addattr32(n, MAX_MSG, mask_type, mask);

	return 0;
}

static int ct_parse_mark(char *str, struct nlmsghdr *n)
{
	return ct_parse_u32(str, TCA_CT_MARK, TCA_CT_MARK_MASK, n);
}

static int ct_parse_labels(char *str, struct nlmsghdr *n)
{
#define LABELS_SIZE	16
	uint8_t labels[LABELS_SIZE], lmask[LABELS_SIZE];
	char *slash, *mask = NULL;
	size_t slen, slen_mask = 0;

	slash = index(str, '/');
	if (slash) {
		*slash = 0;
		mask = slash+1;
		slen_mask = strlen(mask);
	}

	slen = strlen(str);
	if (slen > LABELS_SIZE*2 || slen_mask > LABELS_SIZE*2) {
		char errmsg[128];

		snprintf(errmsg, sizeof(errmsg),
				"%zd Max allowed size %d",
				slen, LABELS_SIZE*2);
		invarg(errmsg, str);
	}

	if (hex2mem(str, labels, slen/2) < 0)
		invarg("ct: labels must be a hex string\n", str);
	addattr_l(n, MAX_MSG, TCA_CT_LABELS, labels, slen/2);

	if (mask) {
		if (hex2mem(mask, lmask, slen_mask/2) < 0)
			invarg("ct: labels mask must be a hex string\n", mask);
	} else {
		memset(lmask, 0xff, sizeof(lmask));
		slen_mask = sizeof(lmask)*2;
	}
	addattr_l(n, MAX_MSG, TCA_CT_LABELS_MASK, lmask, slen_mask/2);

	return 0;
}

static int
parse_ct(struct action_util *a, int *argc_p, char ***argv_p, int tca_id,
		struct nlmsghdr *n)
{
	struct tc_ct sel = {};
	char **argv = *argv_p;
	struct rtattr *tail;
	int argc = *argc_p;
	int ct_action = 0;
	int ret;

	tail = addattr_nest(n, MAX_MSG, tca_id);

	if (argc && matches(*argv, "ct") == 0)
		NEXT_ARG_FWD();

	while (argc > 0) {
		if (matches(*argv, "zone") == 0) {
			NEXT_ARG();

			if (ct_parse_u16(*argv,
					 TCA_CT_ZONE, TCA_CT_UNSPEC, n)) {
				fprintf(stderr, "ct: Illegal \"zone\"\n");
				return -1;
			}
		} else if (matches(*argv, "nat") == 0) {
			ct_action |= TCA_CT_ACT_NAT;

			NEXT_ARG();
			if (matches(*argv, "src") == 0)
				ct_action |= TCA_CT_ACT_NAT_SRC;
			else if (matches(*argv, "dst") == 0)
				ct_action |= TCA_CT_ACT_NAT_DST;
			else
				continue;

			NEXT_ARG();
			if (matches(*argv, "addr") != 0)
				usage();

			NEXT_ARG();
			ret = ct_parse_nat_addr_range(*argv, n);
			if (ret) {
				fprintf(stderr, "ct: Illegal nat address range\n");
				return -1;
			}

			NEXT_ARG_FWD();
			if (matches(*argv, "port") != 0)
				continue;

			NEXT_ARG();
			ret = ct_parse_nat_port_range(*argv, n);
			if (ret) {
				fprintf(stderr, "ct: Illegal nat port range\n");
				return -1;
			}
		} else if (matches(*argv, "clear") == 0) {
			ct_action |= TCA_CT_ACT_CLEAR;
		} else if (matches(*argv, "commit") == 0) {
			ct_action |= TCA_CT_ACT_COMMIT;
		} else if (matches(*argv, "force") == 0) {
			ct_action |= TCA_CT_ACT_FORCE;
		} else if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&sel.index, *argv, 10)) {
				fprintf(stderr, "ct: Illegal \"index\"\n");
				return -1;
			}
		} else if (matches(*argv, "mark") == 0) {
			NEXT_ARG();

			ret = ct_parse_mark(*argv, n);
			if (ret) {
				fprintf(stderr, "ct: Illegal \"mark\"\n");
				return -1;
			}
		} else if (matches(*argv, "label") == 0) {
			NEXT_ARG();

			ret = ct_parse_labels(*argv, n);
			if (ret) {
				fprintf(stderr, "ct: Illegal \"label\"\n");
				return -1;
			}
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			break;
		}
		NEXT_ARG_FWD();
	}

	if (ct_action & TCA_CT_ACT_CLEAR &&
	    ct_action & ~TCA_CT_ACT_CLEAR) {
		fprintf(stderr, "ct: clear can only be used alone\n");
		return -1;
	}

	if (ct_action & TCA_CT_ACT_NAT_SRC &&
	    ct_action & TCA_CT_ACT_NAT_DST) {
		fprintf(stderr, "ct: src and dst nat can't be used together\n");
		return -1;
	}

	if ((ct_action & TCA_CT_ACT_COMMIT) &&
	    (ct_action & TCA_CT_ACT_NAT) &&
	    !(ct_action & (TCA_CT_ACT_NAT_SRC | TCA_CT_ACT_NAT_DST))) {
		fprintf(stderr, "ct: commit and nat must set src or dst\n");
		return -1;
	}

	if (!(ct_action & TCA_CT_ACT_COMMIT) &&
	    (ct_action & (TCA_CT_ACT_NAT_SRC | TCA_CT_ACT_NAT_DST))) {
		fprintf(stderr, "ct: src or dst is only valid if commit is set\n");
		return -1;
	}

	parse_action_control_dflt(&argc, &argv, &sel.action, false,
				  TC_ACT_PIPE);

	addattr16(n, MAX_MSG, TCA_CT_ACTION, ct_action);
	addattr_l(n, MAX_MSG, TCA_CT_PARMS, &sel, sizeof(sel));
	addattr_nest_end(n, tail);

	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

static int ct_sprint_port(char *buf, const char *prefix, struct rtattr *attr)
{
	if (!attr)
		return 0;

	return sprintf(buf, "%s%d", prefix, rta_getattr_be16(attr));
}

static int ct_sprint_ip_addr(char *buf, const char *prefix,
			     struct rtattr *attr)
{
	int family;
	size_t len;

	if (!attr)
		return 0;

	len = RTA_PAYLOAD(attr);

	if (len == 4)
		family = AF_INET;
	else if (len == 16)
		family = AF_INET6;
	else
		return 0;

	return sprintf(buf, "%s%s", prefix, rt_addr_n2a_rta(family, attr));
}

static void ct_print_nat(int ct_action, struct rtattr **tb)
{
	size_t done = 0;
	char out[256] = "";
	bool nat = false;

	if (!(ct_action & TCA_CT_ACT_NAT))
		return;

	if (ct_action & TCA_CT_ACT_NAT_SRC) {
		nat = true;
		done += sprintf(out + done, "src");
	} else if (ct_action & TCA_CT_ACT_NAT_DST) {
		nat = true;
		done += sprintf(out + done, "dst");
	}

	if (nat) {
		done += ct_sprint_ip_addr(out + done, " addr ",
					  tb[TCA_CT_NAT_IPV4_MIN]);
		done += ct_sprint_ip_addr(out + done, " addr ",
					  tb[TCA_CT_NAT_IPV6_MIN]);
		if (tb[TCA_CT_NAT_IPV4_MAX] &&
		    memcmp(RTA_DATA(tb[TCA_CT_NAT_IPV4_MIN]),
			   RTA_DATA(tb[TCA_CT_NAT_IPV4_MAX]), 4))
			done += ct_sprint_ip_addr(out + done, "-",
						  tb[TCA_CT_NAT_IPV4_MAX]);
		else if (tb[TCA_CT_NAT_IPV6_MAX] &&
			    memcmp(RTA_DATA(tb[TCA_CT_NAT_IPV6_MIN]),
				   RTA_DATA(tb[TCA_CT_NAT_IPV6_MAX]), 16))
			done += ct_sprint_ip_addr(out + done, "-",
						  tb[TCA_CT_NAT_IPV6_MAX]);
		done += ct_sprint_port(out + done, " port ",
				       tb[TCA_CT_NAT_PORT_MIN]);
		if (tb[TCA_CT_NAT_PORT_MAX] &&
		    memcmp(RTA_DATA(tb[TCA_CT_NAT_PORT_MIN]),
			   RTA_DATA(tb[TCA_CT_NAT_PORT_MAX]), 2))
			done += ct_sprint_port(out + done, "-",
					       tb[TCA_CT_NAT_PORT_MAX]);
	}

	if (done)
		print_string(PRINT_ANY, "nat", " nat %s", out);
	else
		print_string(PRINT_ANY, "nat", " nat", "");
}

static void ct_print_labels(struct rtattr *attr,
			    struct rtattr *mask_attr)
{
	const unsigned char *str;
	bool print_mask = false;
	char out[256], *p;
	int data_len, i;

	if (!attr)
		return;

	data_len = RTA_PAYLOAD(attr);
	hexstring_n2a(RTA_DATA(attr), data_len, out, sizeof(out));
	p = out + data_len*2;

	data_len = RTA_PAYLOAD(attr);
	str = RTA_DATA(mask_attr);
	if (data_len != 16)
		print_mask = true;
	for (i = 0; !print_mask && i < data_len; i++) {
		if (str[i] != 0xff)
			print_mask = true;
	}
	if (print_mask) {
		*p++ = '/';
		hexstring_n2a(RTA_DATA(mask_attr), data_len, p,
			      sizeof(out)-(p-out));
		p += data_len*2;
	}
	*p = '\0';

	print_string(PRINT_ANY, "label", " label %s", out);
}

static int print_ct(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct rtattr *tb[TCA_CT_MAX + 1];
	const char *commit;
	struct tc_ct *p;
	int ct_action = 0;

	print_string(PRINT_ANY, "kind", "%s", "ct");
	if (arg == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_CT_MAX, arg);
	if (tb[TCA_CT_PARMS] == NULL) {
		print_string(PRINT_FP, NULL, "%s", "[NULL ct parameters]");
		return -1;
	}

	p = RTA_DATA(tb[TCA_CT_PARMS]);

	if (tb[TCA_CT_ACTION])
		ct_action = rta_getattr_u16(tb[TCA_CT_ACTION]);
	if (ct_action & TCA_CT_ACT_COMMIT) {
		commit = ct_action & TCA_CT_ACT_FORCE ?
			 "commit force" : "commit";
		print_string(PRINT_ANY, "action", " %s", commit);
	} else if (ct_action & TCA_CT_ACT_CLEAR) {
		print_string(PRINT_ANY, "action", " %s", "clear");
	}

	print_masked_u32("mark", tb[TCA_CT_MARK], tb[TCA_CT_MARK_MASK], false);
	print_masked_u16("zone", tb[TCA_CT_ZONE], NULL, false);
	ct_print_labels(tb[TCA_CT_LABELS], tb[TCA_CT_LABELS_MASK]);
	ct_print_nat(ct_action, tb);

	print_action_control(f, " ", p->action, "");

	print_nl();
	print_uint(PRINT_ANY, "index", "\t index %u", p->index);
	print_int(PRINT_ANY, "ref", " ref %d", p->refcnt);
	print_int(PRINT_ANY, "bind", " bind %d", p->bindcnt);

	if (show_stats) {
		if (tb[TCA_CT_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_CT_TM]);

			print_tm(f, tm);
		}
	}
	print_nl();

	return 0;
}

struct action_util ct_action_util = {
	.id = "ct",
	.parse_aopt = parse_ct,
	.print_aopt = print_ct,
};
