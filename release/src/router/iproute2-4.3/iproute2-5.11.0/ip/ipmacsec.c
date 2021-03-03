/*
 * ipmacsec.c		"ip macsec".
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Sabrina Dubroca <sd@queasysnail.net>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/genetlink.h>
#include <linux/if_ether.h>
#include <linux/if_macsec.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include "ll_map.h"
#include "libgenl.h"

static const char * const validate_str[] = {
	[MACSEC_VALIDATE_DISABLED] = "disabled",
	[MACSEC_VALIDATE_CHECK] = "check",
	[MACSEC_VALIDATE_STRICT] = "strict",
};

static const char * const offload_str[] = {
	[MACSEC_OFFLOAD_OFF] = "off",
	[MACSEC_OFFLOAD_PHY] = "phy",
	[MACSEC_OFFLOAD_MAC] = "mac",
};

struct sci {
	__u64 sci;
	__u16 port;
	char abuf[6];
};

struct sa_desc {
	__u8 an;
	__u32 pn;
	__u8 key_id[MACSEC_KEYID_LEN];
	__u32 key_len;
	__u8 key[MACSEC_MAX_KEY_LEN];
	__u8 active;
};

struct cipher_args {
	__u64 id;
	__u8 icv_len;
};

struct txsc_desc {
	int ifindex;
	__u64 sci;
	__be16 port;
	struct cipher_args cipher;
	__u32 window;
	enum macsec_validation_type validate;
	__u8 encoding_sa;
};

struct rxsc_desc {
	int ifindex;
	__u64 sci;
	__u8 active;
};

#define MACSEC_BUFLEN 1024


/* netlink socket */
static struct rtnl_handle genl_rth;
static int genl_family = -1;

#define MACSEC_GENL_REQ(_req, _bufsiz, _cmd, _flags) \
	GENL_REQUEST(_req, _bufsiz, genl_family, 0, MACSEC_GENL_VERSION, \
		     _cmd, _flags)


static void ipmacsec_usage(void)
{
	fprintf(stderr,
		"Usage: ip macsec add DEV tx sa { 0..3 } [ OPTS ] key ID KEY\n"
		"       ip macsec set DEV tx sa { 0..3 } [ OPTS ]\n"
		"       ip macsec del DEV tx sa { 0..3 }\n"
		"       ip macsec add DEV rx SCI [ on | off ]\n"
		"       ip macsec set DEV rx SCI [ on | off ]\n"
		"       ip macsec del DEV rx SCI\n"
		"       ip macsec add DEV rx SCI sa { 0..3 } [ OPTS ] key ID KEY\n"
		"       ip macsec set DEV rx SCI sa { 0..3 } [ OPTS ]\n"
		"       ip macsec del DEV rx SCI sa { 0..3 }\n"
		"       ip macsec show\n"
		"       ip macsec show DEV\n"
		"       ip macsec offload DEV [ off | phy | mac ]\n"
		"where  OPTS := [ pn <u32> ] [ on | off ]\n"
		"       ID   := 128-bit hex string\n"
		"       KEY  := 128-bit or 256-bit hex string\n"
		"       SCI  := { sci <u64> | port { 1..2^16-1 } address <lladdr> }\n");

	exit(-1);
}

static int get_an(__u8 *val, const char *arg)
{
	int ret = get_u8(val, arg, 0);

	if (ret)
		return ret;

	if (*val > 3)
		return -1;

	return 0;
}

static int get_sci(__u64 *sci, const char *arg)
{
	return get_be64(sci, arg, 16);
}

static int get_port(__be16 *port, const char *arg)
{
	return get_be16(port, arg, 0);
}

#define _STR(a) #a
#define STR(a) _STR(a)

static void get_icvlen(__u8 *icvlen, char *arg)
{
	int ret = get_u8(icvlen, arg, 10);

	if (ret)
		invarg("expected ICV length", arg);

	if (*icvlen < MACSEC_MIN_ICV_LEN || *icvlen > MACSEC_STD_ICV_LEN)
		invarg("ICV length must be in the range {"
		       STR(MACSEC_MIN_ICV_LEN) ".." STR(MACSEC_STD_ICV_LEN)
		       "}", arg);
}

static bool get_sa(int *argcp, char ***argvp, __u8 *an)
{
	int argc = *argcp;
	char **argv = *argvp;
	int ret;

	if (argc <= 0 || strcmp(*argv, "sa") != 0)
		return false;

	NEXT_ARG();
	ret = get_an(an, *argv);
	if (ret)
		invarg("expected an { 0..3 }", *argv);
	argc--; argv++;

	*argvp = argv;
	*argcp = argc;
	return true;
}

static int parse_sa_args(int *argcp, char ***argvp, struct sa_desc *sa)
{
	int argc = *argcp;
	char **argv = *argvp;
	int ret;
	bool active_set = false;

	while (argc > 0) {
		if (strcmp(*argv, "pn") == 0) {
			if (sa->pn != 0)
				duparg2("pn", "pn");
			NEXT_ARG();
			ret = get_u32(&sa->pn, *argv, 0);
			if (ret)
				invarg("expected pn", *argv);
			if (sa->pn == 0)
				invarg("expected pn != 0", *argv);
		} else if (strcmp(*argv, "key") == 0) {
			unsigned int len;

			NEXT_ARG();
			if (!hexstring_a2n(*argv, sa->key_id, MACSEC_KEYID_LEN,
					   &len))
				invarg("expected key id", *argv);
			NEXT_ARG();
			if (!hexstring_a2n(*argv, sa->key, MACSEC_MAX_KEY_LEN,
					   &sa->key_len))
				invarg("expected key", *argv);
		} else if (strcmp(*argv, "on") == 0) {
			if (active_set)
				duparg2("on/off", "on");
			sa->active = true;
			active_set = true;
		} else if (strcmp(*argv, "off") == 0) {
			if (active_set)
				duparg2("on/off", "off");
			sa->active = false;
			active_set = true;
		} else {
			fprintf(stderr, "macsec: unknown command \"%s\"?\n",
				*argv);
			ipmacsec_usage();
		}

		argv++; argc--;
	}

	*argvp = argv;
	*argcp = argc;
	return 0;
}

static __u64 make_sci(char *addr, __be16 port)
{
	__u64 sci;

	memcpy(&sci, addr, ETH_ALEN);
	memcpy(((char *)&sci) + ETH_ALEN, &port, sizeof(port));

	return sci;
}

static bool sci_complete(bool sci, bool port, bool addr, bool port_only)
{
	return sci || (port && (addr || port_only));
}

static int get_sci_portaddr(struct sci *sci, int *argcp, char ***argvp,
			    bool port_only, bool optional)
{
	int argc = *argcp;
	char **argv = *argvp;
	int ret;
	bool p = false, a = false, s = false;

	while (argc > 0) {
		if (strcmp(*argv, "sci") == 0) {
			if (p)
				invarg("expected address", *argv);
			if (a)
				invarg("expected port", *argv);
			NEXT_ARG();
			ret = get_sci(&sci->sci, *argv);
			if (ret)
				invarg("expected sci", *argv);
			s = true;
		} else if (strcmp(*argv, "port") == 0) {
			NEXT_ARG();
			ret = get_port(&sci->port, *argv);
			if (ret)
				invarg("expected port", *argv);
			if (sci->port == 0)
				invarg("expected port != 0", *argv);
			p = true;
		} else if (strcmp(*argv, "address") == 0) {
			NEXT_ARG();
			ret = ll_addr_a2n(sci->abuf, sizeof(sci->abuf), *argv);
			if (ret < 0)
				invarg("expected lladdr", *argv);
			a = true;
		} else if (optional) {
			break;
		} else {
			invarg("expected sci, port, or address", *argv);
		}

		argv++; argc--;

		if (sci_complete(s, p, a, port_only))
			break;
	}

	if (!optional && !sci_complete(s, p, a, port_only))
		return -1;

	if (p && a)
		sci->sci = make_sci(sci->abuf, sci->port);

	*argvp = argv;
	*argcp = argc;

	return p || a || s;
}

static bool parse_rxsci(int *argcp, char ***argvp, struct rxsc_desc *rxsc,
			struct sa_desc *rxsa)
{
	struct sci sci = { 0 };

	if (*argcp == 0 ||
	    get_sci_portaddr(&sci, argcp, argvp, false, false) < 0) {
		fprintf(stderr, "expected sci\n");
		ipmacsec_usage();
	}

	rxsc->sci = sci.sci;

	return get_sa(argcp, argvp, &rxsa->an);
}

static int parse_rxsci_args(int *argcp, char ***argvp, struct rxsc_desc *rxsc)
{
	int argc = *argcp;
	char **argv = *argvp;
	bool active_set = false;

	while (argc > 0) {
		if (strcmp(*argv, "on") == 0) {
			if (active_set)
				duparg2("on/off", "on");
			rxsc->active = true;
			active_set = true;
		} else if (strcmp(*argv, "off") == 0) {
			if (active_set)
				duparg2("on/off", "off");
			rxsc->active = false;
			active_set = true;
		} else {
			fprintf(stderr, "macsec: unknown command \"%s\"?\n",
				*argv);
			ipmacsec_usage();
		}

		argv++; argc--;
	}

	*argvp = argv;
	*argcp = argc;
	return 0;
}

enum cmd {
	CMD_ADD,
	CMD_DEL,
	CMD_UPD,
	CMD_OFFLOAD,
	__CMD_MAX
};

static const enum macsec_nl_commands macsec_commands[__CMD_MAX][2][2] = {
	[CMD_ADD] = {
		[0] = {-1, MACSEC_CMD_ADD_RXSC},
		[1] = {MACSEC_CMD_ADD_TXSA, MACSEC_CMD_ADD_RXSA},
	},
	[CMD_UPD] = {
		[0] = {-1, MACSEC_CMD_UPD_RXSC},
		[1] = {MACSEC_CMD_UPD_TXSA, MACSEC_CMD_UPD_RXSA},
	},
	[CMD_DEL] = {
		[0] = {-1, MACSEC_CMD_DEL_RXSC},
		[1] = {MACSEC_CMD_DEL_TXSA, MACSEC_CMD_DEL_RXSA},
	},
	[CMD_OFFLOAD] = {
		[0] = {-1, MACSEC_CMD_UPD_OFFLOAD },
	},
};

static int do_modify_nl(enum cmd c, enum macsec_nl_commands cmd, int ifindex,
			struct rxsc_desc *rxsc, struct sa_desc *sa)
{
	struct rtattr *attr_sa;

	MACSEC_GENL_REQ(req, MACSEC_BUFLEN, cmd, NLM_F_REQUEST);

	addattr32(&req.n, MACSEC_BUFLEN, MACSEC_ATTR_IFINDEX, ifindex);
	if (rxsc) {
		struct rtattr *attr_rxsc;

		attr_rxsc = addattr_nest(&req.n, MACSEC_BUFLEN,
					 MACSEC_ATTR_RXSC_CONFIG);
		addattr64(&req.n, MACSEC_BUFLEN,
			  MACSEC_RXSC_ATTR_SCI, rxsc->sci);
		if (c != CMD_DEL && rxsc->active != 0xff)
			addattr8(&req.n, MACSEC_BUFLEN,
				 MACSEC_RXSC_ATTR_ACTIVE, rxsc->active);

		addattr_nest_end(&req.n, attr_rxsc);
	}

	if (sa->an == 0xff)
		goto talk;

	attr_sa = addattr_nest(&req.n, MACSEC_BUFLEN, MACSEC_ATTR_SA_CONFIG);

	addattr8(&req.n, MACSEC_BUFLEN, MACSEC_SA_ATTR_AN, sa->an);

	if (c != CMD_DEL) {
		if (sa->pn)
			addattr32(&req.n, MACSEC_BUFLEN, MACSEC_SA_ATTR_PN,
				  sa->pn);

		if (sa->key_len) {
			addattr_l(&req.n, MACSEC_BUFLEN, MACSEC_SA_ATTR_KEYID,
				  sa->key_id, MACSEC_KEYID_LEN);
			addattr_l(&req.n, MACSEC_BUFLEN, MACSEC_SA_ATTR_KEY,
				  sa->key, sa->key_len);
		}

		if (sa->active != 0xff) {
			addattr8(&req.n, MACSEC_BUFLEN,
				 MACSEC_SA_ATTR_ACTIVE, sa->active);
		}
	}

	addattr_nest_end(&req.n, attr_sa);

talk:
	if (rtnl_talk(&genl_rth, &req.n, NULL) < 0)
		return -2;

	return 0;
}

static bool check_sa_args(enum cmd c, struct sa_desc *sa)
{
	if (c == CMD_ADD) {
		if (!sa->key_len) {
			fprintf(stderr, "cannot create SA without key\n");
			return -1;
		}

		if (sa->pn == 0) {
			fprintf(stderr, "must specify a packet number != 0\n");
			return -1;
		}
	} else if (c == CMD_UPD) {
		if (sa->key_len) {
			fprintf(stderr, "cannot change key on SA\n");
			return -1;
		}
	}

	return 0;
}

static int do_modify_txsa(enum cmd c, int argc, char **argv, int ifindex)
{
	struct sa_desc txsa = {0};
	enum macsec_nl_commands cmd;

	txsa.an = 0xff;
	txsa.active = 0xff;

	if (argc == 0 || !get_sa(&argc, &argv, &txsa.an))
		ipmacsec_usage();

	if (c == CMD_DEL)
		goto modify;

	if (parse_sa_args(&argc, &argv, &txsa))
		return -1;

	if (check_sa_args(c, &txsa))
		return -1;

modify:
	cmd = macsec_commands[c][1][0];
	return do_modify_nl(c, cmd, ifindex, NULL, &txsa);
}

static int do_modify_rxsci(enum cmd c, int argc, char **argv, int ifindex)
{
	struct rxsc_desc rxsc = {0};
	struct sa_desc rxsa = {0};
	bool sa_set;
	enum macsec_nl_commands cmd;

	rxsc.ifindex = ifindex;
	rxsc.active = 0xff;
	rxsa.an = 0xff;
	rxsa.active = 0xff;

	sa_set = parse_rxsci(&argc, &argv, &rxsc, &rxsa);

	if (c == CMD_DEL)
		goto modify;

	if (sa_set && (parse_sa_args(&argc, &argv, &rxsa) ||
		       check_sa_args(c, &rxsa)))
		return -1;
	if (!sa_set && parse_rxsci_args(&argc, &argv, &rxsc))
		return -1;

modify:
	cmd = macsec_commands[c][sa_set][1];
	return do_modify_nl(c, cmd, rxsc.ifindex, &rxsc, &rxsa);
}

static int do_modify(enum cmd c, int argc, char **argv)
{
	int ifindex;

	if (argc == 0)
		ipmacsec_usage();

	ifindex = ll_name_to_index(*argv);
	if (!ifindex) {
		fprintf(stderr, "Device \"%s\" does not exist.\n", *argv);
		return -1;
	}
	argc--; argv++;

	if (argc == 0)
		ipmacsec_usage();

	if (strcmp(*argv, "tx") == 0)
		return do_modify_txsa(c, argc-1, argv+1, ifindex);
	if (strcmp(*argv, "rx") == 0)
		return do_modify_rxsci(c, argc-1, argv+1, ifindex);

	ipmacsec_usage();
	return -1;
}

static int do_offload(enum cmd c, int argc, char **argv)
{
	enum macsec_offload offload;
	struct rtattr *attr;
	int ifindex, ret;

	if (argc == 0)
		ipmacsec_usage();

	ifindex = ll_name_to_index(*argv);
	if (!ifindex) {
		fprintf(stderr, "Device \"%s\" does not exist.\n", *argv);
		return -1;
	}
	argc--; argv++;

	if (argc == 0)
		ipmacsec_usage();

	offload = parse_one_of("offload", *argv, offload_str, ARRAY_SIZE(offload_str), &ret);
	if (ret)
		ipmacsec_usage();

	MACSEC_GENL_REQ(req, MACSEC_BUFLEN, macsec_commands[c][0][1], NLM_F_REQUEST);

	addattr32(&req.n, MACSEC_BUFLEN, MACSEC_ATTR_IFINDEX, ifindex);

	attr = addattr_nest(&req.n, MACSEC_BUFLEN, MACSEC_ATTR_OFFLOAD);
	addattr8(&req.n, MACSEC_BUFLEN, MACSEC_OFFLOAD_ATTR_TYPE, offload);
	addattr_nest_end(&req.n, attr);

	if (rtnl_talk(&genl_rth, &req.n, NULL) < 0)
		return -2;

	return 0;
}

/* dump/show */
static struct {
	int ifindex;
	__u64 sci;
} filter;

static int validate_dump(struct rtattr **attrs)
{
	return attrs[MACSEC_ATTR_IFINDEX] && attrs[MACSEC_ATTR_SECY] &&
	       attrs[MACSEC_ATTR_TXSA_LIST] && attrs[MACSEC_ATTR_RXSC_LIST] &&
	       attrs[MACSEC_ATTR_TXSC_STATS] && attrs[MACSEC_ATTR_SECY_STATS];

}

static int validate_secy_dump(struct rtattr **attrs)
{
	return attrs[MACSEC_SECY_ATTR_SCI] &&
	       attrs[MACSEC_SECY_ATTR_ENCODING_SA] &&
	       attrs[MACSEC_SECY_ATTR_CIPHER_SUITE] &&
	       attrs[MACSEC_SECY_ATTR_ICV_LEN] &&
	       attrs[MACSEC_SECY_ATTR_PROTECT] &&
	       attrs[MACSEC_SECY_ATTR_REPLAY] &&
	       attrs[MACSEC_SECY_ATTR_OPER] &&
	       attrs[MACSEC_SECY_ATTR_VALIDATE] &&
	       attrs[MACSEC_SECY_ATTR_ENCRYPT] &&
	       attrs[MACSEC_SECY_ATTR_INC_SCI] &&
	       attrs[MACSEC_SECY_ATTR_ES] &&
	       attrs[MACSEC_SECY_ATTR_SCB];
}

static void print_flag(struct rtattr *attrs[], const char *desc,
		       int field)
{
	__u8 flag;

	if (!attrs[field])
		return;

	flag = rta_getattr_u8(attrs[field]);
	if (is_json_context())
		print_bool(PRINT_JSON, desc, NULL, flag);
	else {
		print_string(PRINT_FP, NULL, "%s ", desc);
		print_string(PRINT_FP, NULL, "%s ",
			     flag ? "on" : "off");
	}
}

static void print_key(struct rtattr *key)
{
	SPRINT_BUF(keyid);

	print_string(PRINT_ANY, "key", " key %s\n",
		     hexstring_n2a(RTA_DATA(key), RTA_PAYLOAD(key),
				   keyid, sizeof(keyid)));
}

#define CIPHER_NAME_GCM_AES_128 "GCM-AES-128"
#define CIPHER_NAME_GCM_AES_256 "GCM-AES-256"
#define DEFAULT_CIPHER_NAME CIPHER_NAME_GCM_AES_128

static const char *cs_id_to_name(__u64 cid)
{
	switch (cid) {
	case MACSEC_DEFAULT_CIPHER_ID:
		return DEFAULT_CIPHER_NAME;
	case MACSEC_CIPHER_ID_GCM_AES_128:
	     /* MACSEC_DEFAULT_CIPHER_ALT: */
		return CIPHER_NAME_GCM_AES_128;
	case MACSEC_CIPHER_ID_GCM_AES_256:
		return CIPHER_NAME_GCM_AES_256;
	default:
		return "(unknown)";
	}
}

static const char *validate_to_str(__u8 validate)
{
	if (validate >= ARRAY_SIZE(validate_str))
		return "(unknown)";

	return validate_str[validate];
}

static const char *offload_to_str(__u8 offload)
{
	if (offload >= ARRAY_SIZE(offload_str))
		return "(unknown)";

	return offload_str[offload];
}

static void print_attrs(struct rtattr *attrs[])
{
	print_flag(attrs, "protect", MACSEC_SECY_ATTR_PROTECT);

	if (attrs[MACSEC_SECY_ATTR_VALIDATE]) {
		__u8 val = rta_getattr_u8(attrs[MACSEC_SECY_ATTR_VALIDATE]);

		print_string(PRINT_ANY, "validate",
			     "validate %s ", validate_to_str(val));
	}

	print_flag(attrs, "sc", MACSEC_RXSC_ATTR_ACTIVE);
	print_flag(attrs, "sa", MACSEC_SA_ATTR_ACTIVE);
	print_flag(attrs, "encrypt", MACSEC_SECY_ATTR_ENCRYPT);
	print_flag(attrs, "send_sci", MACSEC_SECY_ATTR_INC_SCI);
	print_flag(attrs, "end_station", MACSEC_SECY_ATTR_ES);
	print_flag(attrs, "scb", MACSEC_SECY_ATTR_SCB);
	print_flag(attrs, "replay", MACSEC_SECY_ATTR_REPLAY);

	if (attrs[MACSEC_SECY_ATTR_WINDOW]) {
		__u32 win = rta_getattr_u32(attrs[MACSEC_SECY_ATTR_WINDOW]);

		print_uint(PRINT_ANY, "window", "window %u ", win);
	}

	if (attrs[MACSEC_SECY_ATTR_CIPHER_SUITE]) {
		__u64 cid = rta_getattr_u64(attrs[MACSEC_SECY_ATTR_CIPHER_SUITE]);

		print_nl();
		print_string(PRINT_ANY, "cipher_suite",
			     "    cipher suite: %s,", cs_id_to_name(cid));
	}

	if (attrs[MACSEC_SECY_ATTR_ICV_LEN]) {
		__u8 icv_len = rta_getattr_u8(attrs[MACSEC_SECY_ATTR_ICV_LEN]);

		print_uint(PRINT_ANY, "icv_length",
		     " using ICV length %u\n", icv_len);
	}
}

static __u64 getattr_u64(const struct rtattr *stat)
{
	size_t len = RTA_PAYLOAD(stat);

	switch (len) {
	case sizeof(__u64):
		return rta_getattr_u64(stat);
	case sizeof(__u32):
		return rta_getattr_u32(stat);
	case sizeof(__u16):
		return rta_getattr_u16(stat);
	case sizeof(__u8):
		return rta_getattr_u8(stat);
	default:
		fprintf(stderr, "invalid attribute length %zu\n",
			len);
		exit(-1);
	}
}

static void print_fp_stats(const char *prefix,
			   const char *names[], unsigned int num,
			   struct rtattr *stats[])
{
	unsigned int i;
	int pad;

	printf("%sstats:", prefix);

	for (i = 1; i < num; i++) {
		if (!names[i])
			continue;
		printf(" %s", names[i]);
	}

	printf("\n%s      ", prefix);

	for (i = 1; i < num; i++) {
		if (!names[i])
			continue;

		pad = strlen(names[i]) + 1;
		if (stats[i])
			printf("%*llu", pad, getattr_u64(stats[i]));
		else
			printf("%*c", pad, '-');
	}
	printf("\n");
}

static void print_json_stats(const char *names[], unsigned int num,
			     struct rtattr *stats[])
{
	unsigned int i;

	for (i = 1; i < num; i++) {
		if (!names[i] || !stats[i])
			continue;

		print_u64(PRINT_JSON, names[i],
			   NULL, getattr_u64(stats[i]));
	}
}

static void print_stats(const char *prefix,
			const char *names[], unsigned int num,
			struct rtattr *stats[])
{

	if (is_json_context())
		print_json_stats(names, num, stats);
	else
		print_fp_stats(prefix, names, num, stats);
}

static const char *txsc_stats_names[NUM_MACSEC_TXSC_STATS_ATTR] = {
	[MACSEC_TXSC_STATS_ATTR_OUT_PKTS_PROTECTED] = "OutPktsProtected",
	[MACSEC_TXSC_STATS_ATTR_OUT_PKTS_ENCRYPTED] = "OutPktsEncrypted",
	[MACSEC_TXSC_STATS_ATTR_OUT_OCTETS_PROTECTED] = "OutOctetsProtected",
	[MACSEC_TXSC_STATS_ATTR_OUT_OCTETS_ENCRYPTED] = "OutOctetsEncrypted",
};

static void print_txsc_stats(const char *prefix, struct rtattr *attr)
{
	struct rtattr *stats[MACSEC_TXSC_STATS_ATTR_MAX + 1];

	if (!attr || show_stats == 0)
		return;

	parse_rtattr_nested(stats, MACSEC_TXSC_STATS_ATTR_MAX, attr);

	print_stats(prefix, txsc_stats_names, NUM_MACSEC_TXSC_STATS_ATTR,
		    stats);
}

static const char *secy_stats_names[NUM_MACSEC_SECY_STATS_ATTR] = {
	[MACSEC_SECY_STATS_ATTR_OUT_PKTS_UNTAGGED] = "OutPktsUntagged",
	[MACSEC_SECY_STATS_ATTR_IN_PKTS_UNTAGGED] = "InPktsUntagged",
	[MACSEC_SECY_STATS_ATTR_OUT_PKTS_TOO_LONG] = "OutPktsTooLong",
	[MACSEC_SECY_STATS_ATTR_IN_PKTS_NO_TAG] = "InPktsNoTag",
	[MACSEC_SECY_STATS_ATTR_IN_PKTS_BAD_TAG] = "InPktsBadTag",
	[MACSEC_SECY_STATS_ATTR_IN_PKTS_UNKNOWN_SCI] = "InPktsUnknownSCI",
	[MACSEC_SECY_STATS_ATTR_IN_PKTS_NO_SCI] = "InPktsNoSCI",
	[MACSEC_SECY_STATS_ATTR_IN_PKTS_OVERRUN] = "InPktsOverrun",
};

static void print_secy_stats(const char *prefix, struct rtattr *attr)
{
	struct rtattr *stats[MACSEC_SECY_STATS_ATTR_MAX + 1];

	if (!attr || show_stats == 0)
		return;

	parse_rtattr_nested(stats, MACSEC_SECY_STATS_ATTR_MAX, attr);

	print_stats(prefix, secy_stats_names,
		    NUM_MACSEC_SECY_STATS_ATTR, stats);
}

static const char *rxsa_stats_names[NUM_MACSEC_SA_STATS_ATTR] = {
	[MACSEC_SA_STATS_ATTR_IN_PKTS_OK] = "InPktsOK",
	[MACSEC_SA_STATS_ATTR_IN_PKTS_INVALID] = "InPktsInvalid",
	[MACSEC_SA_STATS_ATTR_IN_PKTS_NOT_VALID] = "InPktsNotValid",
	[MACSEC_SA_STATS_ATTR_IN_PKTS_NOT_USING_SA] = "InPktsNotUsingSA",
	[MACSEC_SA_STATS_ATTR_IN_PKTS_UNUSED_SA] = "InPktsUnusedSA",
};

static void print_rxsa_stats(const char *prefix, struct rtattr *attr)
{
	struct rtattr *stats[MACSEC_SA_STATS_ATTR_MAX + 1];

	if (!attr || show_stats == 0)
		return;

	parse_rtattr_nested(stats, MACSEC_SA_STATS_ATTR_MAX, attr);

	print_stats(prefix, rxsa_stats_names, NUM_MACSEC_SA_STATS_ATTR, stats);
}

static const char *txsa_stats_names[NUM_MACSEC_SA_STATS_ATTR] = {
	[MACSEC_SA_STATS_ATTR_OUT_PKTS_PROTECTED] = "OutPktsProtected",
	[MACSEC_SA_STATS_ATTR_OUT_PKTS_ENCRYPTED] = "OutPktsEncrypted",
};

static void print_txsa_stats(const char *prefix, struct rtattr *attr)
{
	struct rtattr *stats[MACSEC_SA_STATS_ATTR_MAX + 1];

	if (!attr || show_stats == 0)
		return;

	parse_rtattr_nested(stats, MACSEC_SA_STATS_ATTR_MAX, attr);

	print_stats(prefix, txsa_stats_names, NUM_MACSEC_SA_STATS_ATTR, stats);
}

static void print_tx_sc(const char *prefix, __u64 sci, __u8 encoding_sa,
			struct rtattr *txsc_stats, struct rtattr *secy_stats,
			struct rtattr *sa)
{
	struct rtattr *sa_attr[MACSEC_SA_ATTR_MAX + 1];
	struct rtattr *a;
	int rem;

	print_string(PRINT_FP, NULL, "%s", prefix);
	print_0xhex(PRINT_ANY, "sci",
		    "TXSC: %016llx", ntohll(sci));
	print_uint(PRINT_ANY, "encoding_sa",
		   " on SA %d\n", encoding_sa);

	print_secy_stats(prefix, secy_stats);
	print_txsc_stats(prefix, txsc_stats);

	open_json_array(PRINT_JSON, "sa_list");
	rem = RTA_PAYLOAD(sa);
	for (a = RTA_DATA(sa); RTA_OK(a, rem); a = RTA_NEXT(a, rem)) {
		bool state;

		open_json_object(NULL);
		parse_rtattr_nested(sa_attr, MACSEC_SA_ATTR_MAX, a);
		state = rta_getattr_u8(sa_attr[MACSEC_SA_ATTR_ACTIVE]);

		print_string(PRINT_FP, NULL, "%s", prefix);
		print_string(PRINT_FP, NULL, "%s", prefix);
		print_uint(PRINT_ANY, "an", "%d:",
			   rta_getattr_u8(sa_attr[MACSEC_SA_ATTR_AN]));
		print_uint(PRINT_ANY, "pn", " PN %u,",
			   rta_getattr_u32(sa_attr[MACSEC_SA_ATTR_PN]));

		print_bool(PRINT_JSON, "active", NULL, state);
		print_string(PRINT_FP, NULL,
			     " state %s,", state ? "on" : "off");
		print_key(sa_attr[MACSEC_SA_ATTR_KEYID]);

		print_txsa_stats(prefix, sa_attr[MACSEC_SA_ATTR_STATS]);
		close_json_object();
	}
	close_json_array(PRINT_JSON, NULL);
}

static const char *rxsc_stats_names[NUM_MACSEC_RXSC_STATS_ATTR] = {
	[MACSEC_RXSC_STATS_ATTR_IN_OCTETS_VALIDATED] = "InOctetsValidated",
	[MACSEC_RXSC_STATS_ATTR_IN_OCTETS_DECRYPTED] = "InOctetsDecrypted",
	[MACSEC_RXSC_STATS_ATTR_IN_PKTS_UNCHECKED] = "InPktsUnchecked",
	[MACSEC_RXSC_STATS_ATTR_IN_PKTS_DELAYED] = "InPktsDelayed",
	[MACSEC_RXSC_STATS_ATTR_IN_PKTS_OK] = "InPktsOK",
	[MACSEC_RXSC_STATS_ATTR_IN_PKTS_INVALID] = "InPktsInvalid",
	[MACSEC_RXSC_STATS_ATTR_IN_PKTS_LATE] = "InPktsLate",
	[MACSEC_RXSC_STATS_ATTR_IN_PKTS_NOT_VALID] = "InPktsNotValid",
	[MACSEC_RXSC_STATS_ATTR_IN_PKTS_NOT_USING_SA] = "InPktsNotUsingSA",
	[MACSEC_RXSC_STATS_ATTR_IN_PKTS_UNUSED_SA] = "InPktsUnusedSA",
};

static void print_rxsc_stats(const char *prefix, struct rtattr *attr)
{
	struct rtattr *stats[MACSEC_RXSC_STATS_ATTR_MAX + 1];

	if (!attr || show_stats == 0)
		return;

	parse_rtattr_nested(stats, MACSEC_RXSC_STATS_ATTR_MAX, attr);

	print_stats(prefix, rxsc_stats_names,
		    NUM_MACSEC_RXSC_STATS_ATTR, stats);
}

static void print_rx_sc(const char *prefix, __be64 sci, __u8 active,
			struct rtattr *rxsc_stats, struct rtattr *sa)
{
	struct rtattr *sa_attr[MACSEC_SA_ATTR_MAX + 1];
	struct rtattr *a;
	int rem;

	print_string(PRINT_FP, NULL, "%s", prefix);
	print_0xhex(PRINT_ANY, "sci",
		    "RXSC: %016llx,", ntohll(sci));
	print_bool(PRINT_JSON, "active", NULL, active);
	print_string(PRINT_FP, NULL,
		     " state %s\n", active ? "on" : "off");
	print_rxsc_stats(prefix, rxsc_stats);

	open_json_array(PRINT_JSON, "sa_list");
	rem = RTA_PAYLOAD(sa);
	for (a = RTA_DATA(sa); RTA_OK(a, rem); a = RTA_NEXT(a, rem)) {
		bool state;

		open_json_object(NULL);
		parse_rtattr_nested(sa_attr, MACSEC_SA_ATTR_MAX, a);
		state = rta_getattr_u8(sa_attr[MACSEC_SA_ATTR_ACTIVE]);

		print_string(PRINT_FP, NULL, "%s", prefix);
		print_string(PRINT_FP, NULL, "%s", prefix);
		print_uint(PRINT_ANY, "an", "%u:",
			   rta_getattr_u8(sa_attr[MACSEC_SA_ATTR_AN]));
		print_uint(PRINT_ANY, "pn", " PN %u,",
			   rta_getattr_u32(sa_attr[MACSEC_SA_ATTR_PN]));

		print_bool(PRINT_JSON, "active", NULL, state);
		print_string(PRINT_FP, NULL, " state %s,",
			     state ? "on" : "off");

		print_key(sa_attr[MACSEC_SA_ATTR_KEYID]);

		print_rxsa_stats(prefix, sa_attr[MACSEC_SA_ATTR_STATS]);
		close_json_object();
	}
	close_json_array(PRINT_JSON, NULL);
}

static void print_rxsc_list(struct rtattr *sc)
{
	int rem = RTA_PAYLOAD(sc);
	struct rtattr *c;

	open_json_array(PRINT_JSON, "rx_sc");
	for (c = RTA_DATA(sc); RTA_OK(c, rem); c = RTA_NEXT(c, rem)) {
		struct rtattr *sc_attr[MACSEC_RXSC_ATTR_MAX + 1];

		open_json_object(NULL);

		parse_rtattr_nested(sc_attr, MACSEC_RXSC_ATTR_MAX, c);
		print_rx_sc("    ",
			    rta_getattr_u64(sc_attr[MACSEC_RXSC_ATTR_SCI]),
			    rta_getattr_u32(sc_attr[MACSEC_RXSC_ATTR_ACTIVE]),
			    sc_attr[MACSEC_RXSC_ATTR_STATS],
			    sc_attr[MACSEC_RXSC_ATTR_SA_LIST]);
		close_json_object();
	}
	close_json_array(PRINT_JSON, NULL);
}

static int process(struct nlmsghdr *n, void *arg)
{
	struct genlmsghdr *ghdr;
	struct rtattr *attrs[MACSEC_ATTR_MAX + 1];
	struct rtattr *attrs_secy[MACSEC_SECY_ATTR_MAX + 1];
	int len = n->nlmsg_len;
	int ifindex;
	__u64 sci;
	__u8 encoding_sa;

	if (n->nlmsg_type != genl_family)
		return -1;

	len -= NLMSG_LENGTH(GENL_HDRLEN);
	if (len < 0)
		return -1;

	ghdr = NLMSG_DATA(n);
	if (ghdr->cmd != MACSEC_CMD_GET_TXSC)
		return 0;

	parse_rtattr(attrs, MACSEC_ATTR_MAX, (void *) ghdr + GENL_HDRLEN, len);
	if (!validate_dump(attrs)) {
		fprintf(stderr, "incomplete dump message\n");
		return -1;
	}

	ifindex = rta_getattr_u32(attrs[MACSEC_ATTR_IFINDEX]);
	parse_rtattr_nested(attrs_secy, MACSEC_SECY_ATTR_MAX,
			    attrs[MACSEC_ATTR_SECY]);

	if (!validate_secy_dump(attrs_secy)) {
		fprintf(stderr, "incomplete dump message\n");
		return -1;
	}

	sci = rta_getattr_u64(attrs_secy[MACSEC_SECY_ATTR_SCI]);
	encoding_sa = rta_getattr_u8(attrs_secy[MACSEC_SECY_ATTR_ENCODING_SA]);

	if (filter.ifindex && ifindex != filter.ifindex)
		return 0;

	if (filter.sci && sci != filter.sci)
		return 0;

	open_json_object(NULL);
	print_uint(PRINT_ANY, "ifindex", "%u: ", ifindex);
	print_color_string(PRINT_ANY, COLOR_IFNAME, "ifname",
			   "%s: ", ll_index_to_name(ifindex));

	print_attrs(attrs_secy);

	print_tx_sc("    ", sci, encoding_sa,
		    attrs[MACSEC_ATTR_TXSC_STATS],
		    attrs[MACSEC_ATTR_SECY_STATS],
		    attrs[MACSEC_ATTR_TXSA_LIST]);

	if (attrs[MACSEC_ATTR_RXSC_LIST])
		print_rxsc_list(attrs[MACSEC_ATTR_RXSC_LIST]);

	if (attrs[MACSEC_ATTR_OFFLOAD]) {
		struct rtattr *attrs_offload[MACSEC_OFFLOAD_ATTR_MAX + 1];
		__u8 offload;

		parse_rtattr_nested(attrs_offload, MACSEC_OFFLOAD_ATTR_MAX,
				    attrs[MACSEC_ATTR_OFFLOAD]);

		offload = rta_getattr_u8(attrs_offload[MACSEC_OFFLOAD_ATTR_TYPE]);
		print_string(PRINT_ANY, "offload",
			     "    offload: %s ", offload_to_str(offload));
		print_nl();
	}

	close_json_object();

	return 0;
}

static int do_dump(int ifindex)
{
	MACSEC_GENL_REQ(req, MACSEC_BUFLEN, MACSEC_CMD_GET_TXSC,
			NLM_F_REQUEST | NLM_F_DUMP);

	memset(&filter, 0, sizeof(filter));
	filter.ifindex = ifindex;

	req.n.nlmsg_seq = genl_rth.dump = ++genl_rth.seq;
	if (rtnl_send(&genl_rth, &req, req.n.nlmsg_len) < 0) {
		perror("Failed to send dump request");
		exit(1);
	}

	new_json_obj(json);
	if (rtnl_dump_filter(&genl_rth, process, stdout) < 0) {
		delete_json_obj();
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}
	delete_json_obj();

	return 0;
}

static int do_show(int argc, char **argv)
{
	int ifindex;

	if (argc == 0)
		return do_dump(0);

	ifindex = ll_name_to_index(*argv);
	if (ifindex == 0) {
		fprintf(stderr, "Device \"%s\" does not exist.\n", *argv);
		return -1;
	}

	argc--, argv++;
	if (argc == 0)
		return do_dump(ifindex);

	ipmacsec_usage();
	return -1;
}

int do_ipmacsec(int argc, char **argv)
{
	if (argc < 1)
		ipmacsec_usage();

	if (matches(*argv, "help") == 0)
		ipmacsec_usage();

	if (genl_init_handle(&genl_rth, MACSEC_GENL_NAME, &genl_family))
		exit(1);

	if (matches(*argv, "show") == 0)
		return do_show(argc-1, argv+1);

	if (matches(*argv, "add") == 0)
		return do_modify(CMD_ADD, argc-1, argv+1);
	if (matches(*argv, "set") == 0)
		return do_modify(CMD_UPD, argc-1, argv+1);
	if (matches(*argv, "delete") == 0)
		return do_modify(CMD_DEL, argc-1, argv+1);
	if (matches(*argv, "offload") == 0)
		return do_offload(CMD_OFFLOAD, argc-1, argv+1);

	fprintf(stderr, "Command \"%s\" is unknown, try \"ip macsec help\".\n",
		*argv);
	exit(-1);
}

/* device creation */
static void macsec_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	if (!tb)
		return;

	if (tb[IFLA_MACSEC_SCI]) {
		if (is_json_context()) {
			SPRINT_BUF(b1);

			snprintf(b1, sizeof(b1), "%016llx",
				 ntohll(rta_getattr_u64(tb[IFLA_MACSEC_SCI])));
			print_string(PRINT_JSON, "sci", NULL, b1);
		} else {
			fprintf(f, "sci %016llx ",
				ntohll(rta_getattr_u64(tb[IFLA_MACSEC_SCI])));
		}
	}

	print_flag(tb, "protect", IFLA_MACSEC_PROTECT);

	if (tb[IFLA_MACSEC_CIPHER_SUITE]) {
		__u64 csid
			= rta_getattr_u64(tb[IFLA_MACSEC_CIPHER_SUITE]);

		print_string(PRINT_ANY,
			     "cipher_suite",
			     "cipher %s ",
			     cs_id_to_name(csid));
	}

	if (tb[IFLA_MACSEC_ICV_LEN]) {
		if (is_json_context()) {
			char b2[4];

			snprintf(b2, sizeof(b2), "%hhu",
				 rta_getattr_u8(tb[IFLA_MACSEC_ICV_LEN]));
			print_uint(PRINT_JSON, "icv_len", NULL, atoi(b2));
		} else {
			fprintf(f, "icvlen %hhu ",
				rta_getattr_u8(tb[IFLA_MACSEC_ICV_LEN]));
		}
	}

	if (tb[IFLA_MACSEC_ENCODING_SA]) {
		if (is_json_context()) {
			char b2[4];

			snprintf(b2, sizeof(b2), "%hhu",
				 rta_getattr_u8(tb[IFLA_MACSEC_ENCODING_SA]));
			print_uint(PRINT_JSON, "encoding_sa", NULL, atoi(b2));
		} else {
			fprintf(f, "encodingsa %hhu ",
				rta_getattr_u8(tb[IFLA_MACSEC_ENCODING_SA]));
		}
	}

	if (tb[IFLA_MACSEC_VALIDATION]) {
		__u8 val = rta_getattr_u8(tb[IFLA_MACSEC_VALIDATION]);

		print_string(PRINT_ANY,
			     "validation",
			     "validate %s ",
			     validate_to_str(val));
	}

	if (tb[IFLA_MACSEC_OFFLOAD]) {
		__u8 val = rta_getattr_u8(tb[IFLA_MACSEC_OFFLOAD]);

		print_string(PRINT_ANY,
			     "offload",
			     "offload %s ",
			     offload_to_str(val));
	}

	const char *inc_sci, *es, *replay;

	if (is_json_context()) {
		inc_sci = "inc_sci";
		replay = "replay_protect";
		es = "es";
	} else {
		inc_sci = "send_sci";
		es = "end_station";
		replay = "replay";
	}

	print_flag(tb, "encrypt", IFLA_MACSEC_ENCRYPT);
	print_flag(tb, inc_sci, IFLA_MACSEC_INC_SCI);
	print_flag(tb, es, IFLA_MACSEC_ES);
	print_flag(tb, "scb", IFLA_MACSEC_SCB);
	print_flag(tb, replay, IFLA_MACSEC_REPLAY_PROTECT);

	if (tb[IFLA_MACSEC_WINDOW])
		print_int(PRINT_ANY,
			  "window",
			  "window %d ",
			  rta_getattr_u32(tb[IFLA_MACSEC_WINDOW]));
}

static bool check_txsc_flags(bool es, bool scb, bool sci)
{
	if (sci && (es || scb))
		return false;
	if (es && scb)
		return false;
	return true;
}

static void usage(FILE *f)
{
	fprintf(f,
		"Usage: ... macsec [ [ address <lladdr> ] port { 1..2^16-1 } | sci <u64> ]\n"
		"                  [ cipher { default | gcm-aes-128 | gcm-aes-256 } ]\n"
		"                  [ icvlen { 8..16 } ]\n"
		"                  [ encrypt { on | off } ]\n"
		"                  [ send_sci { on | off } ]\n"
		"                  [ end_station { on | off } ]\n"
		"                  [ scb { on | off } ]\n"
		"                  [ protect { on | off } ]\n"
		"                  [ replay { on | off} window { 0..2^32-1 } ]\n"
		"                  [ validate { strict | check | disabled } ]\n"
		"                  [ encodingsa { 0..3 } ]\n"
		"                  [ offload { mac | phy | off } ]\n"
		);
}

static int macsec_parse_opt(struct link_util *lu, int argc, char **argv,
			    struct nlmsghdr *n)
{
	int ret;
	__u8 encoding_sa = 0xff;
	__u32 window = -1;
	enum macsec_offload offload;
	struct cipher_args cipher = {0};
	enum macsec_validation_type validate;
	bool es = false, scb = false, send_sci = false;
	int replay_protect = -1;
	struct sci sci = { 0 };

	ret = get_sci_portaddr(&sci, &argc, &argv, true, true);
	if (ret < 0) {
		fprintf(stderr, "expected sci\n");
		return -1;
	}

	if (ret > 0) {
		if (sci.sci)
			addattr_l(n, MACSEC_BUFLEN, IFLA_MACSEC_SCI,
				  &sci.sci, sizeof(sci.sci));
		else
			addattr_l(n, MACSEC_BUFLEN, IFLA_MACSEC_PORT,
				  &sci.port, sizeof(sci.port));
	}

	while (argc > 0) {
		if (strcmp(*argv, "cipher") == 0) {
			NEXT_ARG();
			if (cipher.id)
				duparg("cipher", *argv);
			if (strcmp(*argv, "default") == 0)
				cipher.id = MACSEC_DEFAULT_CIPHER_ID;
			else if (strcmp(*argv, "gcm-aes-128") == 0 ||
			         strcmp(*argv, "GCM-AES-128") == 0)
				cipher.id = MACSEC_CIPHER_ID_GCM_AES_128;
			else if (strcmp(*argv, "gcm-aes-256") == 0 ||
			         strcmp(*argv, "GCM-AES-256") == 0)
				cipher.id = MACSEC_CIPHER_ID_GCM_AES_256;
			else
				invarg("expected: default, gcm-aes-128 or"
				       " gcm-aes-256", *argv);
		} else if (strcmp(*argv, "icvlen") == 0) {
			NEXT_ARG();
			if (cipher.icv_len)
				duparg("icvlen", *argv);
			get_icvlen(&cipher.icv_len, *argv);
		} else if (strcmp(*argv, "encrypt") == 0) {
			NEXT_ARG();
			int i;

			i = parse_on_off("encrypt", *argv, &ret);
			if (ret != 0)
				return ret;
			addattr8(n, MACSEC_BUFLEN, IFLA_MACSEC_ENCRYPT, i);
		} else if (strcmp(*argv, "send_sci") == 0) {
			NEXT_ARG();
			int i;

			i = parse_on_off("send_sci", *argv, &ret);
			if (ret != 0)
				return ret;
			send_sci = i;
			addattr8(n, MACSEC_BUFLEN,
				 IFLA_MACSEC_INC_SCI, send_sci);
		} else if (strcmp(*argv, "end_station") == 0) {
			NEXT_ARG();
			int i;

			i = parse_on_off("end_station", *argv, &ret);
			if (ret != 0)
				return ret;
			es = i;
			addattr8(n, MACSEC_BUFLEN, IFLA_MACSEC_ES, es);
		} else if (strcmp(*argv, "scb") == 0) {
			NEXT_ARG();
			int i;

			i = parse_on_off("scb", *argv, &ret);
			if (ret != 0)
				return ret;
			scb = i;
			addattr8(n, MACSEC_BUFLEN, IFLA_MACSEC_SCB, scb);
		} else if (strcmp(*argv, "protect") == 0) {
			NEXT_ARG();
			int i;

			i = parse_on_off("protect", *argv, &ret);
			if (ret != 0)
				return ret;
			addattr8(n, MACSEC_BUFLEN, IFLA_MACSEC_PROTECT, i);
		} else if (strcmp(*argv, "replay") == 0) {
			NEXT_ARG();
			int i;

			i = parse_on_off("replay", *argv, &ret);
			if (ret != 0)
				return ret;
			replay_protect = !!i;
		} else if (strcmp(*argv, "window") == 0) {
			NEXT_ARG();
			ret = get_u32(&window, *argv, 0);
			if (ret)
				invarg("expected replay window size", *argv);
		} else if (strcmp(*argv, "validate") == 0) {
			NEXT_ARG();
			validate = parse_one_of("validate", *argv, validate_str,
						ARRAY_SIZE(validate_str), &ret);
			if (ret != 0)
				return ret;
			addattr8(n, MACSEC_BUFLEN,
				 IFLA_MACSEC_VALIDATION, validate);
		} else if (strcmp(*argv, "encodingsa") == 0) {
			if (encoding_sa != 0xff)
				duparg2("encodingsa", "encodingsa");
			NEXT_ARG();
			ret = get_an(&encoding_sa, *argv);
			if (ret)
				invarg("expected an { 0..3 }", *argv);
		} else if (strcmp(*argv, "offload") == 0) {
			NEXT_ARG();
			offload = parse_one_of("offload", *argv, offload_str,
					       ARRAY_SIZE(offload_str), &ret);
			if (ret != 0)
				return ret;
			addattr8(n, MACSEC_BUFLEN,
				 IFLA_MACSEC_OFFLOAD, offload);
		} else {
			fprintf(stderr, "macsec: unknown command \"%s\"?\n",
				*argv);
			usage(stderr);
			return -1;
		}

		argv++; argc--;
	}

	if (!check_txsc_flags(es, scb, send_sci)) {
		fprintf(stderr,
			"invalid combination of send_sci/end_station/scb\n");
		return -1;
	}

	if (window != -1 && replay_protect == -1) {
		fprintf(stderr,
			"replay window set, but replay protection not enabled. did you mean 'replay on window %u'?\n",
			window);
		return -1;
	} else if (window == -1 && replay_protect == 1) {
		fprintf(stderr,
			"replay protection enabled, but no window set. did you mean 'replay on window VALUE'?\n");
		return -1;
	}

	if (cipher.id)
		addattr_l(n, MACSEC_BUFLEN, IFLA_MACSEC_CIPHER_SUITE,
			  &cipher.id, sizeof(cipher.id));
	if (cipher.icv_len)
		addattr_l(n, MACSEC_BUFLEN, IFLA_MACSEC_ICV_LEN,
			  &cipher.icv_len, sizeof(cipher.icv_len));

	if (replay_protect != -1) {
		addattr32(n, MACSEC_BUFLEN, IFLA_MACSEC_WINDOW, window);
		addattr8(n, MACSEC_BUFLEN, IFLA_MACSEC_REPLAY_PROTECT,
			 replay_protect);
	}

	if (encoding_sa != 0xff) {
		addattr_l(n, MACSEC_BUFLEN, IFLA_MACSEC_ENCODING_SA,
			  &encoding_sa, sizeof(encoding_sa));
	}

	return 0;
}

static void macsec_print_help(struct link_util *lu, int argc, char **argv,
			      FILE *f)
{
	usage(f);
}

struct link_util macsec_link_util = {
	.id = "macsec",
	.maxattr = IFLA_MACSEC_MAX,
	.parse_opt = macsec_parse_opt,
	.print_help = macsec_print_help,
	.print_opt = macsec_print_opt,
};
