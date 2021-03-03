/*
 * m_pedit.c		generic packet editor actions module
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:  J Hadi Salim (hadi@cyberus.ca)
 *
 * TODO:
 *	1) Big endian broken in some spots
 *	2) A lot of this stuff was added on the fly; get a big double-double
 *	and clean it up at some point.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <dlfcn.h>
#include "utils.h"
#include "tc_util.h"
#include "m_pedit.h"
#include "rt_names.h"

static struct m_pedit_util *pedit_list;
static int pedit_debug;

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... pedit munge [ex] <MUNGE> [CONTROL]\n"
		"Where: MUNGE := <RAW>|<LAYERED>\n"
		"\t<RAW>:= <OFFSETC>[ATC]<CMD>\n \t\tOFFSETC:= offset <offval> <u8|u16|u32>\n"
		"\t\tATC:= at <atval> offmask <maskval> shift <shiftval>\n"
		"\t\tNOTE: offval is byte offset, must be multiple of 4\n"
		"\t\tNOTE: maskval is a 32 bit hex number\n \t\tNOTE: shiftval is a shift value\n"
		"\t\tCMD:= clear | invert | set <setval>| add <addval> | retain\n"
		"\t<LAYERED>:= ip <ipdata> | ip6 <ip6data>\n"
		" \t\t| udp <udpdata> | tcp <tcpdata> | icmp <icmpdata>\n"
		"\tCONTROL:= reclassify | pipe | drop | continue | pass |\n"
		"\t          goto chain <CHAIN_INDEX>\n"
		"\tNOTE: if 'ex' is set, extended functionality will be supported (kernel >= 4.11)\n"
		"For Example usage look at the examples directory\n");

}

static void usage(void)
{
	explain();
	exit(-1);
}

static int pedit_parse_nopopt(int *argc_p, char ***argv_p,
			      struct m_pedit_sel *sel,
			      struct m_pedit_key *tkey)
{
	int argc = *argc_p;
	char **argv = *argv_p;

	if (argc) {
		fprintf(stderr,
			"Unknown action  hence option \"%s\" is unparsable\n",
			*argv);
		return -1;
	}

	return 0;

}

static struct m_pedit_util *get_pedit_kind(const char *str)
{
	static void *pBODY;
	void *dlh;
	char buf[256];
	struct m_pedit_util *p;

	for (p = pedit_list; p; p = p->next) {
		if (strcmp(p->id, str) == 0)
			return p;
	}

	snprintf(buf, sizeof(buf), "p_%s.so", str);
	dlh = dlopen(buf, RTLD_LAZY);
	if (dlh == NULL) {
		dlh = pBODY;
		if (dlh == NULL) {
			dlh = pBODY = dlopen(NULL, RTLD_LAZY);
			if (dlh == NULL)
				goto noexist;
		}
	}

	snprintf(buf, sizeof(buf), "p_pedit_%s", str);
	p = dlsym(dlh, buf);
	if (p == NULL)
		goto noexist;

reg:
	p->next = pedit_list;
	pedit_list = p;
	return p;

noexist:
	p = calloc(1, sizeof(*p));
	if (p) {
		strlcpy(p->id, str, sizeof(p->id));
		p->parse_peopt = pedit_parse_nopopt;
		goto reg;
	}
	return p;
}

static int pack_key(struct m_pedit_sel *_sel, struct m_pedit_key *tkey)
{
	struct tc_pedit_sel *sel = &_sel->sel;
	struct m_pedit_key_ex *keys_ex = _sel->keys_ex;
	int hwm = sel->nkeys;

	if (hwm >= MAX_OFFS)
		return -1;

	if (tkey->off % 4) {
		fprintf(stderr, "offsets MUST be in 32 bit boundaries\n");
		return -1;
	}

	sel->keys[hwm].val = tkey->val;
	sel->keys[hwm].mask = tkey->mask;
	sel->keys[hwm].off = tkey->off;
	sel->keys[hwm].at = tkey->at;
	sel->keys[hwm].offmask = tkey->offmask;
	sel->keys[hwm].shift = tkey->shift;

	if (_sel->extended) {
		keys_ex[hwm].htype = tkey->htype;
		keys_ex[hwm].cmd = tkey->cmd;
	} else {
		if (tkey->htype != TCA_PEDIT_KEY_EX_HDR_TYPE_NETWORK ||
		    tkey->cmd != TCA_PEDIT_KEY_EX_CMD_SET) {
			fprintf(stderr,
				"Munge parameters not supported. Use 'pedit ex munge ...'.\n");
			return -1;
		}
	}

	sel->nkeys++;
	return 0;
}

static int pack_key32(__u32 retain, struct m_pedit_sel *sel,
		      struct m_pedit_key *tkey)
{
	if (tkey->off > (tkey->off & ~3)) {
		fprintf(stderr,
			"pack_key32: 32 bit offsets must begin in 32bit boundaries\n");
		return -1;
	}

	tkey->val = htonl(tkey->val & retain);
	tkey->mask = htonl(tkey->mask | ~retain);
	return pack_key(sel, tkey);
}

static int pack_key16(__u32 retain, struct m_pedit_sel *sel,
		      struct m_pedit_key *tkey)
{
	int ind, stride;
	__u32 m[4] = { 0x0000FFFF, 0xFF0000FF, 0xFFFF0000 };

	if (tkey->val > 0xFFFF || tkey->mask > 0xFFFF) {
		fprintf(stderr, "pack_key16 bad value\n");
		return -1;
	}

	ind = tkey->off & 3;

	if (ind == 3) {
		fprintf(stderr, "pack_key16 bad index value %d\n", ind);
		return -1;
	}

	stride = 8 * (2 - ind);
	tkey->val = htonl((tkey->val & retain) << stride);
	tkey->mask = htonl(((tkey->mask | ~retain) << stride) | m[ind]);

	tkey->off &= ~3;

	if (pedit_debug)
		printf("pack_key16: Final val %08x mask %08x\n",
		       tkey->val, tkey->mask);
	return pack_key(sel, tkey);
}

static int pack_key8(__u32 retain, struct m_pedit_sel *sel,
		     struct m_pedit_key *tkey)
{
	int ind, stride;
	__u32 m[4] = { 0x00FFFFFF, 0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00 };

	if (tkey->val > 0xFF || tkey->mask > 0xFF) {
		fprintf(stderr, "pack_key8 bad value (val %x mask %x\n",
			tkey->val, tkey->mask);
		return -1;
	}

	ind = tkey->off & 3;

	stride = 8 * (3 - ind);
	tkey->val = htonl((tkey->val & retain) << stride);
	tkey->mask = htonl(((tkey->mask | ~retain) << stride) | m[ind]);

	tkey->off &= ~3;

	if (pedit_debug)
		printf("pack_key8: Final word off %d  val %08x mask %08x\n",
		       tkey->off, tkey->val, tkey->mask);
	return pack_key(sel, tkey);
}

static int pack_mac(struct m_pedit_sel *sel, struct m_pedit_key *tkey,
		    __u8 *mac)
{
	int ret = 0;

	if (!(tkey->off & 0x3)) {
		tkey->mask = 0;
		tkey->val = ntohl(*((__u32 *)mac));
		ret |= pack_key32(~0, sel, tkey);

		tkey->off += 4;
		tkey->mask = 0;
		tkey->val = ntohs(*((__u16 *)&mac[4]));
		ret |= pack_key16(~0, sel, tkey);
	} else if (!(tkey->off & 0x1)) {
		tkey->mask = 0;
		tkey->val = ntohs(*((__u16 *)mac));
		ret |= pack_key16(~0, sel, tkey);

		tkey->off += 4;
		tkey->mask = 0;
		tkey->val = ntohl(*((__u32 *)(mac + 2)));
		ret |= pack_key32(~0, sel, tkey);
	} else {
		fprintf(stderr,
			"pack_mac: mac offsets must begin in 32bit or 16bit boundaries\n");
		return -1;
	}

	return ret;
}

static int pack_ipv6(struct m_pedit_sel *sel, struct m_pedit_key *tkey,
		     __u32 *ipv6)
{
	int ret = 0;
	int i;

	if (tkey->off & 0x3) {
		fprintf(stderr,
			"pack_ipv6: IPv6 offsets must begin in 32bit boundaries\n");
		return -1;
	}

	for (i = 0; i < 4; i++) {
		tkey->mask = 0;
		tkey->val = ntohl(ipv6[i]);

		ret = pack_key32(~0, sel, tkey);
		if (ret)
			return ret;

		tkey->off += 4;
	}

	return 0;
}

static int parse_val(int *argc_p, char ***argv_p, __u32 *val, int type)
{
	int argc = *argc_p;
	char **argv = *argv_p;

	if (argc <= 0)
		return -1;

	if (type == TINT)
		return get_integer((int *)val, *argv, 0);

	if (type == TU32)
		return get_u32(val, *argv, 0);

	if (type == TIPV4) {
		inet_prefix addr;

		if (get_prefix_1(&addr, *argv, AF_INET))
			return -1;

		*val = addr.data[0];
		return 0;
	}

	if (type == TIPV6) {
		inet_prefix addr;

		if (get_prefix_1(&addr, *argv, AF_INET6))
			return -1;

		memcpy(val, addr.data, addr.bytelen);

		return 0;
	}

	if (type == TMAC) {
#define MAC_ALEN 6
		int ret = ll_addr_a2n((char *)val, MAC_ALEN, *argv);

		if (ret == MAC_ALEN)
			return 0;
	}

	return -1;
}

int parse_cmd(int *argc_p, char ***argv_p, __u32 len, int type, __u32 retain,
	      struct m_pedit_sel *sel, struct m_pedit_key *tkey)
{
	__u32 mask[4] = { 0 };
	__u32 val[4] = { 0 };
	__u32 *m = &mask[0];
	__u32 *v = &val[0];
	__u32 o = 0xFF;
	int res = -1;
	int argc = *argc_p;
	char **argv = *argv_p;

	if (argc <= 0)
		return -1;

	if (pedit_debug)
		printf("parse_cmd argc %d %s offset %d length %d\n",
		       argc, *argv, tkey->off, len);

	if (len == 2)
		o = 0xFFFF;
	if (len == 4)
		o = 0xFFFFFFFF;

	if (matches(*argv, "invert") == 0) {
		*v = *m = o;
	} else if (matches(*argv, "set") == 0 ||
		   matches(*argv, "add") == 0) {
		if (matches(*argv, "add") == 0)
			tkey->cmd = TCA_PEDIT_KEY_EX_CMD_ADD;

		if (!sel->extended && tkey->cmd) {
			fprintf(stderr,
				"Non extended mode. only 'set' command is supported\n");
			return -1;
		}

		NEXT_ARG();
		if (parse_val(&argc, &argv, val, type))
			return -1;
	} else if (matches(*argv, "preserve") == 0) {
		retain = 0;
	} else {
		if (matches(*argv, "clear") != 0)
			return -1;
	}

	argc--;
	argv++;

	if (argc && matches(*argv, "retain") == 0) {
		NEXT_ARG();
		if (parse_val(&argc, &argv, &retain, TU32))
			return -1;
		argc--;
		argv++;
	}

	if (len > 4 && retain != ~0) {
		fprintf(stderr,
			"retain is not supported for fields longer the 32 bits\n");
		return -1;
	}

	if (type == TMAC) {
		res = pack_mac(sel, tkey, (__u8 *)val);
		goto done;
	}

	if (type == TIPV6) {
		res = pack_ipv6(sel, tkey, val);
		goto done;
	}

	tkey->val = *v;
	tkey->mask = *m;

	if (type == TIPV4)
		tkey->val = ntohl(tkey->val);

	if (len == 1) {
		res = pack_key8(retain, sel, tkey);
		goto done;
	}
	if (len == 2) {
		res = pack_key16(retain, sel, tkey);
		goto done;
	}
	if (len == 4) {
		res = pack_key32(retain, sel, tkey);
		goto done;
	}

	return -1;
done:
	if (pedit_debug)
		printf("parse_cmd done argc %d %s offset %d length %d\n",
		       argc, *argv, tkey->off, len);
	*argc_p = argc;
	*argv_p = argv;
	return res;

}

static int parse_offset(int *argc_p, char ***argv_p, struct m_pedit_sel *sel,
			struct m_pedit_key *tkey)
{
	int off;
	__u32 len, retain;
	int argc = *argc_p;
	char **argv = *argv_p;
	int res = -1;

	if (argc <= 0)
		return -1;

	if (get_integer(&off, *argv, 0))
		return -1;
	tkey->off = off;

	argc--;
	argv++;

	if (argc <= 0)
		return -1;

	if (matches(*argv, "u32") == 0) {
		len = 4;
		retain = 0xFFFFFFFF;
		goto done;
	}
	if (matches(*argv, "u16") == 0) {
		len = 2;
		retain = 0xffff;
		goto done;
	}
	if (matches(*argv, "u8") == 0) {
		len = 1;
		retain = 0xff;
		goto done;
	}

	return -1;

done:

	NEXT_ARG();

	/* [at <someval> offmask <maskval> shift <shiftval>] */
	if (matches(*argv, "at") == 0) {

		__u32 atv = 0, offmask = 0x0, shift = 0;

		NEXT_ARG();
		if (get_u32(&atv, *argv, 0))
			return -1;
		tkey->at = atv;

		NEXT_ARG();

		if (get_u32(&offmask, *argv, 16))
			return -1;
		tkey->offmask = offmask;

		NEXT_ARG();

		if (get_u32(&shift, *argv, 0))
			return -1;
		tkey->shift = shift;

		NEXT_ARG();
	}

	res = parse_cmd(&argc, &argv, len, TU32, retain, sel, tkey);

	*argc_p = argc;
	*argv_p = argv;
	return res;
}

static int parse_munge(int *argc_p, char ***argv_p, struct m_pedit_sel *sel)
{
	struct m_pedit_key tkey = {};
	int argc = *argc_p;
	char **argv = *argv_p;
	int res = -1;

	if (argc <= 0)
		return -1;

	if (matches(*argv, "offset") == 0) {
		NEXT_ARG();
		res = parse_offset(&argc, &argv, sel, &tkey);
		goto done;
	} else {
		char k[FILTER_NAMESZ];
		struct m_pedit_util *p = NULL;

		strncpy(k, *argv, sizeof(k) - 1);

		if (argc > 0) {
			p = get_pedit_kind(k);
			if (p == NULL)
				goto bad_val;
			NEXT_ARG();
			res = p->parse_peopt(&argc, &argv, sel, &tkey);
			if (res < 0) {
				fprintf(stderr, "bad pedit parsing\n");
				goto bad_val;
			}
			goto done;
		}
	}

bad_val:
	return -1;

done:

	*argc_p = argc;
	*argv_p = argv;
	return res;
}

static int pedit_keys_ex_getattr(struct rtattr *attr,
				 struct m_pedit_key_ex *keys_ex, int n)
{
	struct rtattr *i;
	int rem = RTA_PAYLOAD(attr);
	struct rtattr *tb[TCA_PEDIT_KEY_EX_MAX + 1];
	struct m_pedit_key_ex *k = keys_ex;

	for (i = RTA_DATA(attr); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		if (!n)
			return -1;

		if (i->rta_type != TCA_PEDIT_KEY_EX)
			return -1;

		parse_rtattr_nested(tb, TCA_PEDIT_KEY_EX_MAX, i);

		k->htype = rta_getattr_u16(tb[TCA_PEDIT_KEY_EX_HTYPE]);
		k->cmd = rta_getattr_u16(tb[TCA_PEDIT_KEY_EX_CMD]);

		k++;
		n--;
	}

	return !!n;
}

static int pedit_keys_ex_addattr(struct m_pedit_sel *sel, struct nlmsghdr *n)
{
	struct m_pedit_key_ex *k = sel->keys_ex;
	struct rtattr *keys_start;
	int i;

	if (!sel->extended)
		return 0;

	keys_start = addattr_nest(n, MAX_MSG, TCA_PEDIT_KEYS_EX | NLA_F_NESTED);

	for (i = 0; i < sel->sel.nkeys; i++) {
		struct rtattr *key_start;

		key_start = addattr_nest(n, MAX_MSG,
					 TCA_PEDIT_KEY_EX | NLA_F_NESTED);

		if (addattr16(n, MAX_MSG, TCA_PEDIT_KEY_EX_HTYPE, k->htype) ||
		    addattr16(n, MAX_MSG, TCA_PEDIT_KEY_EX_CMD, k->cmd)) {
			return -1;
		}

		addattr_nest_end(n, key_start);

		k++;
	}

	addattr_nest_end(n, keys_start);

	return 0;
}

static int parse_pedit(struct action_util *a, int *argc_p, char ***argv_p,
		       int tca_id, struct nlmsghdr *n)
{
	struct m_pedit_sel sel = {};

	int argc = *argc_p;
	char **argv = *argv_p;
	int ok = 0, iok = 0;
	struct rtattr *tail;

	while (argc > 0) {
		if (pedit_debug > 1)
			fprintf(stderr, "while pedit (%d:%s)\n", argc, *argv);
		if (matches(*argv, "pedit") == 0) {
			NEXT_ARG();
			ok++;

			if (matches(*argv, "ex") == 0) {
				if (ok > 1) {
					fprintf(stderr,
						"'ex' must be before first 'munge'\n");
					explain();
					return -1;
				}
				sel.extended = true;
				NEXT_ARG();
			}

			continue;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else if (matches(*argv, "munge") == 0) {
			if (!ok) {
				fprintf(stderr, "Bad pedit construct (%s)\n",
					*argv);
				explain();
				return -1;
			}
			NEXT_ARG();

			if (parse_munge(&argc, &argv, &sel)) {
				fprintf(stderr, "Bad pedit construct (%s)\n",
					*argv);
				explain();
				return -1;
			}
			ok++;
		} else {
			break;
		}

	}

	if (!ok) {
		explain();
		return -1;
	}

	parse_action_control_dflt(&argc, &argv, &sel.sel.action, false, TC_ACT_OK);

	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&sel.sel.index, *argv, 10)) {
				fprintf(stderr, "Pedit: Illegal \"index\"\n");
				return -1;
			}
			argc--;
			argv++;
			iok++;
		}
	}

	tail = addattr_nest(n, MAX_MSG, tca_id);
	if (!sel.extended) {
		addattr_l(n, MAX_MSG, TCA_PEDIT_PARMS, &sel,
			  sizeof(sel.sel) +
			  sel.sel.nkeys * sizeof(struct tc_pedit_key));
	} else {
		addattr_l(n, MAX_MSG, TCA_PEDIT_PARMS_EX, &sel,
			  sizeof(sel.sel) +
			  sel.sel.nkeys * sizeof(struct tc_pedit_key));

		pedit_keys_ex_addattr(&sel, n);
	}

	addattr_nest_end(n, tail);

	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

static const char * const pedit_htype_str[] = {
	[TCA_PEDIT_KEY_EX_HDR_TYPE_NETWORK] = "",
	[TCA_PEDIT_KEY_EX_HDR_TYPE_ETH] = "eth",
	[TCA_PEDIT_KEY_EX_HDR_TYPE_IP4] = "ipv4",
	[TCA_PEDIT_KEY_EX_HDR_TYPE_IP6] = "ipv6",
	[TCA_PEDIT_KEY_EX_HDR_TYPE_TCP] = "tcp",
	[TCA_PEDIT_KEY_EX_HDR_TYPE_UDP] = "udp",
};

static int print_pedit_location(FILE *f,
				enum pedit_header_type htype, __u32 off)
{
	char *buf = NULL;
	int rc;

	if (htype != TCA_PEDIT_KEY_EX_HDR_TYPE_NETWORK) {
		if (htype < ARRAY_SIZE(pedit_htype_str))
			rc = asprintf(&buf, "%s", pedit_htype_str[htype]);
		else
			rc = asprintf(&buf, "unknown(%d)", htype);
		if (rc < 0)
			return rc;
		print_string(PRINT_ANY, "htype", "%s", buf);
		print_int(PRINT_ANY, "offset", "%+d", off);
	} else {
		print_string(PRINT_JSON, "htype", NULL, "network");
		print_int(PRINT_ANY, "offset", "%d", off);
	}

	free(buf);
	return 0;
}

static int print_pedit(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct tc_pedit_sel *sel;
	struct rtattr *tb[TCA_PEDIT_MAX + 1];
	struct m_pedit_key_ex *keys_ex = NULL;
	int err;

	print_string(PRINT_ANY, "kind", " %s ", "pedit");
	if (arg == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_PEDIT_MAX, arg);

	if (!tb[TCA_PEDIT_PARMS] && !tb[TCA_PEDIT_PARMS_EX]) {
		fprintf(stderr, "Missing pedit parameters\n");
		return -1;
	}

	if (tb[TCA_PEDIT_PARMS]) {
		sel = RTA_DATA(tb[TCA_PEDIT_PARMS]);
	} else {
		int err;

		sel = RTA_DATA(tb[TCA_PEDIT_PARMS_EX]);

		if (!tb[TCA_PEDIT_KEYS_EX]) {
			fprintf(f, "Netlink error\n");
			return -1;
		}

		keys_ex = calloc(sel->nkeys, sizeof(*keys_ex));
		if (!keys_ex) {
			fprintf(f, "Out of memory\n");
			return -1;
		}

		err = pedit_keys_ex_getattr(tb[TCA_PEDIT_KEYS_EX], keys_ex,
					    sel->nkeys);
		if (err) {
			fprintf(f, "Netlink error\n");

			free(keys_ex);
			return -1;
		}
	}

	print_action_control(f, "action ", sel->action, " ");
	print_uint(PRINT_ANY, "nkeys", "keys %d\n", sel->nkeys);
	print_uint(PRINT_ANY, "index", " \t index %u", sel->index);
	print_int(PRINT_ANY, "ref", " ref %d", sel->refcnt);
	print_int(PRINT_ANY, "bind", " bind %d", sel->bindcnt);

	if (show_stats) {
		if (tb[TCA_PEDIT_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_PEDIT_TM]);

			print_tm(f, tm);
		}
	}
	open_json_array(PRINT_JSON, "keys");
	if (sel->nkeys) {
		int i;
		struct tc_pedit_key *key = sel->keys;
		struct m_pedit_key_ex *key_ex = keys_ex;

		for (i = 0; i < sel->nkeys; i++, key++) {
			enum pedit_header_type htype =
				TCA_PEDIT_KEY_EX_HDR_TYPE_NETWORK;
			enum pedit_cmd cmd = TCA_PEDIT_KEY_EX_CMD_SET;

			if (keys_ex) {
				htype = key_ex->htype;
				cmd = key_ex->cmd;

				key_ex++;
			}

			open_json_object(NULL);
			print_uint(PRINT_FP, NULL, "\n\t key #%d  at ", i);

			err = print_pedit_location(f, htype, key->off);
			if (err) {
				free(keys_ex);
				return err;
			}

			/* In FP, report the "set" command as "val" to keep
			 * backward compatibility. Report the true name in JSON.
			 */
			print_string(PRINT_FP, NULL, ": %s",
				     cmd ? "add" : "val");
			print_string(PRINT_JSON, "cmd", NULL,
				     cmd ? "add" : "set");
			print_hex(PRINT_ANY, "val", " %08x",
				  (unsigned int)ntohl(key->val));
			print_hex(PRINT_ANY, "mask", " mask %08x",
				  (unsigned int)ntohl(key->mask));
			close_json_object();
		}
	} else {
		fprintf(f, "\npedit %x keys %d is not LEGIT", sel->index,
			sel->nkeys);
	}
	close_json_array(PRINT_JSON, " ");

	print_nl();

	free(keys_ex);
	return 0;
}

struct action_util pedit_action_util = {
	.id = "pedit",
	.parse_aopt = parse_pedit,
	.print_aopt = print_pedit,
};
