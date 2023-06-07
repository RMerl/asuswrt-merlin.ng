/*
 * m_tunnel_key.c	ip tunnel manipulation module
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Amir Vadai <amir@vadai.me>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/if_ether.h>
#include "utils.h"
#include "rt_names.h"
#include "tc_util.h"
#include <linux/tc_act/tc_tunnel_key.h>

static void explain(void)
{
	fprintf(stderr,
		"Usage: tunnel_key unset\n"
		"       tunnel_key set <TUNNEL_KEY>\n"
		"Where TUNNEL_KEY is a combination of:\n"
		"id <TUNNELID>\n"
		"src_ip <IP> (mandatory)\n"
		"dst_ip <IP> (mandatory)\n"
		"dst_port <UDP_PORT>\n"
		"geneve_opts | vxlan_opts | erspan_opts <OPTIONS>\n"
		"csum | nocsum (default is \"csum\")\n");
}

static void usage(void)
{
	explain();
	exit(-1);
}

static int tunnel_key_parse_ip_addr(const char *str, int addr4_type,
				    int addr6_type, struct nlmsghdr *n)
{
	inet_prefix addr;
	int ret;

	ret = get_addr(&addr, str, AF_UNSPEC);
	if (ret)
		return ret;

	addattr_l(n, MAX_MSG, addr.family == AF_INET ? addr4_type : addr6_type,
		  addr.data, addr.bytelen);

	return 0;
}

static int tunnel_key_parse_key_id(const char *str, int type,
				   struct nlmsghdr *n)
{
	__be32 key_id;
	int ret;

	ret = get_be32(&key_id, str, 10);
	if (!ret)
		addattr32(n, MAX_MSG, type, key_id);

	return ret;
}

static int tunnel_key_parse_dst_port(char *str, int type, struct nlmsghdr *n)
{
	int ret;
	__be16 dst_port;

	ret = get_be16(&dst_port, str, 10);
	if (ret)
		return -1;

	addattr16(n, MAX_MSG, type, dst_port);

	return 0;
}

static int tunnel_key_parse_be16(char *str, int base, int type,
				 struct nlmsghdr *n)
{
	int ret;
	__be16 value;

	ret = get_be16(&value, str, base);
	if (ret)
		return ret;

	addattr16(n, MAX_MSG, type, value);

	return 0;
}

static int tunnel_key_parse_be32(char *str, int base, int type,
				 struct nlmsghdr *n)
{
	__be32 value;
	int ret;

	ret = get_be32(&value, str, base);
	if (ret)
		return ret;

	addattr32(n, MAX_MSG, type, value);

	return 0;
}

static int tunnel_key_parse_u8(char *str, int base, int type,
			       struct nlmsghdr *n)
{
	int ret;
	__u8 value;

	ret = get_u8(&value, str, base);
	if (ret)
		return ret;

	addattr8(n, MAX_MSG, type, value);

	return 0;
}

static int tunnel_key_parse_u32(char *str, int base, int type,
				struct nlmsghdr *n)
{
	__u32 value;
	int ret;

	ret = get_u32(&value, str, base);
	if (ret)
		return ret;

	addattr32(n, MAX_MSG, type, value);

	return 0;
}

static int tunnel_key_parse_geneve_opt(char *str, struct nlmsghdr *n)
{
	char *token, *saveptr = NULL;
	struct rtattr *nest;
	int i, ret;

	nest = addattr_nest(n, MAX_MSG, TCA_TUNNEL_KEY_ENC_OPTS_GENEVE);

	token = strtok_r(str, ":", &saveptr);
	i = 1;
	while (token) {
		switch (i) {
		case TCA_TUNNEL_KEY_ENC_OPT_GENEVE_CLASS:
		{
			ret = tunnel_key_parse_be16(token, 16, i, n);
			if (ret)
				return ret;
			break;
		}
		case TCA_TUNNEL_KEY_ENC_OPT_GENEVE_TYPE:
		{
			ret = tunnel_key_parse_u8(token, 16, i, n);
			if (ret)
				return ret;
			break;
		}
		case TCA_TUNNEL_KEY_ENC_OPT_GENEVE_DATA:
		{
			size_t token_len = strlen(token);
			uint8_t *opts;

			opts = malloc(token_len / 2);
			if (!opts)
				return -1;
			if (hex2mem(token, opts, token_len / 2) < 0) {
				free(opts);
				return -1;
			}
			addattr_l(n, MAX_MSG, i, opts, token_len / 2);
			free(opts);

			break;
		}
		default:
			return -1;
		}

		token = strtok_r(NULL, ":", &saveptr);
		i++;
	}

	addattr_nest_end(n, nest);

	return 0;
}

static int tunnel_key_parse_geneve_opts(char *str, struct nlmsghdr *n)
{
	char *token, *saveptr = NULL;
	struct rtattr *nest;
	int ret;

	nest = addattr_nest(n, MAX_MSG, TCA_TUNNEL_KEY_ENC_OPTS);

	token = strtok_r(str, ",", &saveptr);
	while (token) {
		ret = tunnel_key_parse_geneve_opt(token, n);
		if (ret)
			return ret;

		token = strtok_r(NULL, ",", &saveptr);
	}

	addattr_nest_end(n, nest);

	return 0;
}

static int tunnel_key_parse_vxlan_opt(char *str, struct nlmsghdr *n)
{
	struct rtattr *encap, *nest;
	int ret;

	encap = addattr_nest(n, MAX_MSG,
			     TCA_TUNNEL_KEY_ENC_OPTS | NLA_F_NESTED);
	nest = addattr_nest(n, MAX_MSG,
			    TCA_TUNNEL_KEY_ENC_OPTS_VXLAN | NLA_F_NESTED);

	ret = tunnel_key_parse_u32(str, 0,
				   TCA_TUNNEL_KEY_ENC_OPT_VXLAN_GBP, n);
	if (ret)
		return ret;

	addattr_nest_end(n, nest);
	addattr_nest_end(n, encap);

	return 0;
}

static int tunnel_key_parse_erspan_opt(char *str, struct nlmsghdr *n)
{
	char *token, *saveptr = NULL;
	struct rtattr *encap, *nest;
	int i, ret;

	encap = addattr_nest(n, MAX_MSG,
			     TCA_TUNNEL_KEY_ENC_OPTS | NLA_F_NESTED);
	nest = addattr_nest(n, MAX_MSG,
			    TCA_TUNNEL_KEY_ENC_OPTS_ERSPAN | NLA_F_NESTED);

	token = strtok_r(str, ":", &saveptr);
	i = 1;
	while (token) {
		switch (i) {
		case TCA_TUNNEL_KEY_ENC_OPT_ERSPAN_VER:
		{
			ret = tunnel_key_parse_u8(token, 0, i, n);
			if (ret)
				return ret;
			break;
		}
		case TCA_TUNNEL_KEY_ENC_OPT_ERSPAN_INDEX:
		{
			ret = tunnel_key_parse_be32(token, 0, i, n);
			if (ret)
				return ret;
			break;
		}
		case TCA_TUNNEL_KEY_ENC_OPT_ERSPAN_DIR:
		{
			ret = tunnel_key_parse_u8(token, 0, i, n);
			if (ret)
				return ret;
			break;
		}
		case TCA_TUNNEL_KEY_ENC_OPT_ERSPAN_HWID:
		{
			ret = tunnel_key_parse_u8(token, 0, i, n);
			if (ret)
				return ret;
			break;
		}
		default:
			return -1;
		}

		token = strtok_r(NULL, ":", &saveptr);
		i++;
	}

	addattr_nest_end(n, nest);
	addattr_nest_end(n, encap);

	return 0;
}

static int tunnel_key_parse_tos_ttl(char *str, int type, struct nlmsghdr *n)
{
	int ret;
	__u8 val;

	ret = get_u8(&val, str, 10);
	if (ret)
		ret = get_u8(&val, str, 16);
	if (ret)
		return -1;

	addattr8(n, MAX_MSG, type, val);

	return 0;
}

static int parse_tunnel_key(struct action_util *a, int *argc_p, char ***argv_p,
			    int tca_id, struct nlmsghdr *n)
{
	struct tc_tunnel_key parm = {};
	char **argv = *argv_p;
	int argc = *argc_p;
	struct rtattr *tail;
	int action = 0;
	int ret;
	int has_src_ip = 0;
	int has_dst_ip = 0;
	int csum = 1;

	if (matches(*argv, "tunnel_key") != 0)
		return -1;

	tail = addattr_nest(n, MAX_MSG, tca_id);

	NEXT_ARG();

	while (argc > 0) {
		if (matches(*argv, "unset") == 0) {
			if (action) {
				fprintf(stderr, "unexpected \"%s\" - action already specified\n",
					*argv);
				explain();
				return -1;
			}
			action = TCA_TUNNEL_KEY_ACT_RELEASE;
		} else if (matches(*argv, "set") == 0) {
			if (action) {
				fprintf(stderr, "unexpected \"%s\" - action already specified\n",
					*argv);
				explain();
				return -1;
			}
			action = TCA_TUNNEL_KEY_ACT_SET;
		} else if (matches(*argv, "src_ip") == 0) {
			NEXT_ARG();
			ret = tunnel_key_parse_ip_addr(*argv,
						       TCA_TUNNEL_KEY_ENC_IPV4_SRC,
						       TCA_TUNNEL_KEY_ENC_IPV6_SRC,
						       n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"src_ip\"\n");
				return -1;
			}
			has_src_ip = 1;
		} else if (matches(*argv, "dst_ip") == 0) {
			NEXT_ARG();
			ret = tunnel_key_parse_ip_addr(*argv,
						       TCA_TUNNEL_KEY_ENC_IPV4_DST,
						       TCA_TUNNEL_KEY_ENC_IPV6_DST,
						       n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"dst_ip\"\n");
				return -1;
			}
			has_dst_ip = 1;
		} else if (matches(*argv, "id") == 0) {
			NEXT_ARG();
			ret = tunnel_key_parse_key_id(*argv, TCA_TUNNEL_KEY_ENC_KEY_ID, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"id\"\n");
				return -1;
			}
		} else if (matches(*argv, "dst_port") == 0) {
			NEXT_ARG();
			ret = tunnel_key_parse_dst_port(*argv,
							TCA_TUNNEL_KEY_ENC_DST_PORT, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"dst port\"\n");
				return -1;
			}
		} else if (matches(*argv, "geneve_opts") == 0) {
			NEXT_ARG();

			if (tunnel_key_parse_geneve_opts(*argv, n)) {
				fprintf(stderr, "Illegal \"geneve_opts\"\n");
				return -1;
			}
		} else if (matches(*argv, "vxlan_opts") == 0) {
			NEXT_ARG();

			if (tunnel_key_parse_vxlan_opt(*argv, n)) {
				fprintf(stderr, "Illegal \"vxlan_opts\"\n");
				return -1;
			}
		} else if (matches(*argv, "erspan_opts") == 0) {
			NEXT_ARG();

			if (tunnel_key_parse_erspan_opt(*argv, n)) {
				fprintf(stderr, "Illegal \"erspan_opts\"\n");
				return -1;
			}
		} else if (matches(*argv, "tos") == 0) {
			NEXT_ARG();
			ret = tunnel_key_parse_tos_ttl(*argv,
							TCA_TUNNEL_KEY_ENC_TOS, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"tos\"\n");
				return -1;
			}
		} else if (matches(*argv, "ttl") == 0) {
			NEXT_ARG();
			ret = tunnel_key_parse_tos_ttl(*argv,
							TCA_TUNNEL_KEY_ENC_TTL, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"ttl\"\n");
				return -1;
			}
		} else if (matches(*argv, "csum") == 0) {
			csum = 1;
		} else if (matches(*argv, "nocsum") == 0) {
			csum = 0;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			break;
		}
		NEXT_ARG_FWD();
	}

	addattr8(n, MAX_MSG, TCA_TUNNEL_KEY_NO_CSUM, !csum);

	parse_action_control_dflt(&argc, &argv, &parm.action,
				  false, TC_ACT_PIPE);

	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&parm.index, *argv, 10)) {
				fprintf(stderr, "tunnel_key: Illegal \"index\"\n");
				return -1;
			}

			NEXT_ARG_FWD();
		}
	}

	if (action == TCA_TUNNEL_KEY_ACT_SET &&
	    (!has_src_ip || !has_dst_ip)) {
		fprintf(stderr, "set needs tunnel_key parameters\n");
		explain();
		return -1;
	}

	parm.t_action = action;
	addattr_l(n, MAX_MSG, TCA_TUNNEL_KEY_PARMS, &parm, sizeof(parm));
	addattr_nest_end(n, tail);

	*argc_p = argc;
	*argv_p = argv;

	return 0;
}

static void tunnel_key_print_ip_addr(FILE *f, const char *name,
				     struct rtattr *attr)
{
	int family;
	size_t len;

	if (!attr)
		return;

	len = RTA_PAYLOAD(attr);

	if (len == 4)
		family = AF_INET;
	else if (len == 16)
		family = AF_INET6;
	else
		return;

	print_nl();
	if (matches(name, "src_ip") == 0)
		print_string(PRINT_ANY, "src_ip", "\tsrc_ip %s",
			     rt_addr_n2a_rta(family, attr));
	else if (matches(name, "dst_ip") == 0)
		print_string(PRINT_ANY, "dst_ip", "\tdst_ip %s",
			     rt_addr_n2a_rta(family, attr));
}

static void tunnel_key_print_key_id(FILE *f, const char *name,
				    struct rtattr *attr)
{
	if (!attr)
		return;
	print_nl();
	print_uint(PRINT_ANY, "key_id", "\tkey_id %u", rta_getattr_be32(attr));
}

static void tunnel_key_print_dst_port(FILE *f, char *name,
				      struct rtattr *attr)
{
	if (!attr)
		return;
	print_nl();
	print_uint(PRINT_ANY, "dst_port", "\tdst_port %u",
		   rta_getattr_be16(attr));
}

static void tunnel_key_print_flag(FILE *f, const char *name_on,
				  const char *name_off,
				  struct rtattr *attr)
{
	if (!attr)
		return;
	print_nl();
	print_string(PRINT_ANY, "flag", "\t%s",
		     rta_getattr_u8(attr) ? name_on : name_off);
}

static void tunnel_key_print_geneve_options(struct rtattr *attr)
{
	struct rtattr *tb[TCA_TUNNEL_KEY_ENC_OPT_GENEVE_MAX + 1];
	struct rtattr *i = RTA_DATA(attr);
	int ii, data_len = 0, offset = 0;
	int rem = RTA_PAYLOAD(attr);
	char *name = "geneve_opts";
	char strbuf[rem * 2 + 1];
	char data[rem * 2 + 1];
	uint8_t data_r[rem];
	uint16_t clss;
	uint8_t type;

	open_json_array(PRINT_JSON, name);
	print_nl();
	print_string(PRINT_FP, name, "\t%s ", name);

	while (rem) {
		parse_rtattr(tb, TCA_TUNNEL_KEY_ENC_OPT_GENEVE_MAX, i, rem);
		clss = rta_getattr_be16(tb[TCA_TUNNEL_KEY_ENC_OPT_GENEVE_CLASS]);
		type = rta_getattr_u8(tb[TCA_TUNNEL_KEY_ENC_OPT_GENEVE_TYPE]);
		data_len = RTA_PAYLOAD(tb[TCA_TUNNEL_KEY_ENC_OPT_GENEVE_DATA]);
		hexstring_n2a(RTA_DATA(tb[TCA_TUNNEL_KEY_ENC_OPT_GENEVE_DATA]),
			      data_len, data, sizeof(data));
		hex2mem(data, data_r, data_len);
		offset += data_len + 20;
		rem -= data_len + 20;
		i = RTA_DATA(attr) + offset;

		open_json_object(NULL);
		print_uint(PRINT_JSON, "class", NULL, clss);
		print_uint(PRINT_JSON, "type", NULL, type);
		open_json_array(PRINT_JSON, "data");
		for (ii = 0; ii < data_len; ii++)
			print_uint(PRINT_JSON, NULL, NULL, data_r[ii]);
		close_json_array(PRINT_JSON, "data");
		close_json_object();

		sprintf(strbuf, "%04x:%02x:%s", clss, type, data);
		if (rem)
			print_string(PRINT_FP, NULL, "%s,", strbuf);
		else
			print_string(PRINT_FP, NULL, "%s", strbuf);
	}

	close_json_array(PRINT_JSON, name);
}

static void tunnel_key_print_vxlan_options(struct rtattr *attr)
{
	struct rtattr *tb[TCA_TUNNEL_KEY_ENC_OPT_VXLAN_MAX + 1];
	struct rtattr *i = RTA_DATA(attr);
	int rem = RTA_PAYLOAD(attr);
	char *name = "vxlan_opts";
	__u32 gbp;

	parse_rtattr(tb, TCA_TUNNEL_KEY_ENC_OPT_VXLAN_MAX, i, rem);
	gbp = rta_getattr_u32(tb[TCA_TUNNEL_KEY_ENC_OPT_VXLAN_GBP]);

	print_nl();
	print_string(PRINT_FP, name, "\t%s ", name);
	open_json_array(PRINT_JSON, name);
	open_json_object(NULL);
	print_uint(PRINT_ANY, "gbp", "%u", gbp);
	close_json_object();
	close_json_array(PRINT_JSON, name);
}

static void tunnel_key_print_erspan_options(struct rtattr *attr)
{
	struct rtattr *tb[TCA_TUNNEL_KEY_ENC_OPT_ERSPAN_MAX + 1];
	struct rtattr *i = RTA_DATA(attr);
	int rem = RTA_PAYLOAD(attr);
	char *name = "erspan_opts";
	__u8 ver, hwid, dir;
	__u32 idx;

	parse_rtattr(tb, TCA_TUNNEL_KEY_ENC_OPT_ERSPAN_MAX, i, rem);
	ver = rta_getattr_u8(tb[TCA_TUNNEL_KEY_ENC_OPT_ERSPAN_VER]);
	if (ver == 1) {
		idx = rta_getattr_be32(tb[TCA_TUNNEL_KEY_ENC_OPT_ERSPAN_INDEX]);
		dir = 0;
		hwid = 0;
	} else {
		idx = 0;
		dir = rta_getattr_u8(tb[TCA_TUNNEL_KEY_ENC_OPT_ERSPAN_DIR]);
		hwid = rta_getattr_u8(tb[TCA_TUNNEL_KEY_ENC_OPT_ERSPAN_HWID]);
	}

	print_nl();
	print_string(PRINT_FP, name, "\t%s ", name);
	open_json_array(PRINT_JSON, name);
	open_json_object(NULL);
	print_uint(PRINT_ANY, "ver", "%u", ver);
	print_uint(PRINT_ANY, "index", ":%u", idx);
	print_uint(PRINT_ANY, "dir", ":%u", dir);
	print_uint(PRINT_ANY, "hwid", ":%u", hwid);
	close_json_object();
	close_json_array(PRINT_JSON, name);
}

static void tunnel_key_print_key_opt(struct rtattr *attr)
{
	struct rtattr *tb[TCA_TUNNEL_KEY_ENC_OPTS_MAX + 1];

	if (!attr)
		return;

	parse_rtattr_nested(tb, TCA_TUNNEL_KEY_ENC_OPTS_MAX, attr);
	if (tb[TCA_TUNNEL_KEY_ENC_OPTS_GENEVE])
		tunnel_key_print_geneve_options(
			tb[TCA_TUNNEL_KEY_ENC_OPTS_GENEVE]);
	else if (tb[TCA_TUNNEL_KEY_ENC_OPTS_VXLAN])
		tunnel_key_print_vxlan_options(
			tb[TCA_TUNNEL_KEY_ENC_OPTS_VXLAN]);
	else if (tb[TCA_TUNNEL_KEY_ENC_OPTS_ERSPAN])
		tunnel_key_print_erspan_options(
			tb[TCA_TUNNEL_KEY_ENC_OPTS_ERSPAN]);
}

static void tunnel_key_print_tos_ttl(FILE *f, char *name,
				     struct rtattr *attr)
{
	if (!attr)
		return;

	if (matches(name, "tos") == 0 && rta_getattr_u8(attr) != 0) {
		print_nl();
		print_uint(PRINT_ANY, "tos", "\ttos 0x%x",
			   rta_getattr_u8(attr));
	} else if (matches(name, "ttl") == 0 && rta_getattr_u8(attr) != 0) {
		print_nl();
		print_uint(PRINT_ANY, "ttl", "\tttl %u",
			   rta_getattr_u8(attr));
	}
}

static int print_tunnel_key(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct rtattr *tb[TCA_TUNNEL_KEY_MAX + 1];
	struct tc_tunnel_key *parm;

	print_string(PRINT_ANY, "kind", "%s ", "tunnel_key");
	if (!arg)
		return 0;

	parse_rtattr_nested(tb, TCA_TUNNEL_KEY_MAX, arg);

	if (!tb[TCA_TUNNEL_KEY_PARMS]) {
		fprintf(stderr, "Missing tunnel_key parameters\n");
		return -1;
	}
	parm = RTA_DATA(tb[TCA_TUNNEL_KEY_PARMS]);

	switch (parm->t_action) {
	case TCA_TUNNEL_KEY_ACT_RELEASE:
		print_string(PRINT_ANY, "mode", " %s", "unset");
		break;
	case TCA_TUNNEL_KEY_ACT_SET:
		print_string(PRINT_ANY, "mode", " %s", "set");
		tunnel_key_print_ip_addr(f, "src_ip",
					 tb[TCA_TUNNEL_KEY_ENC_IPV4_SRC]);
		tunnel_key_print_ip_addr(f, "dst_ip",
					 tb[TCA_TUNNEL_KEY_ENC_IPV4_DST]);
		tunnel_key_print_ip_addr(f, "src_ip",
					 tb[TCA_TUNNEL_KEY_ENC_IPV6_SRC]);
		tunnel_key_print_ip_addr(f, "dst_ip",
					 tb[TCA_TUNNEL_KEY_ENC_IPV6_DST]);
		tunnel_key_print_key_id(f, "key_id",
					tb[TCA_TUNNEL_KEY_ENC_KEY_ID]);
		tunnel_key_print_dst_port(f, "dst_port",
					  tb[TCA_TUNNEL_KEY_ENC_DST_PORT]);
		tunnel_key_print_key_opt(tb[TCA_TUNNEL_KEY_ENC_OPTS]);
		tunnel_key_print_flag(f, "nocsum", "csum",
				      tb[TCA_TUNNEL_KEY_NO_CSUM]);
		tunnel_key_print_tos_ttl(f, "tos",
					  tb[TCA_TUNNEL_KEY_ENC_TOS]);
		tunnel_key_print_tos_ttl(f, "ttl",
					  tb[TCA_TUNNEL_KEY_ENC_TTL]);
		break;
	}
	print_action_control(f, " ", parm->action, "");

	print_nl();
	print_uint(PRINT_ANY, "index", "\t index %u", parm->index);
	print_int(PRINT_ANY, "ref", " ref %d", parm->refcnt);
	print_int(PRINT_ANY, "bind", " bind %d", parm->bindcnt);

	if (show_stats) {
		if (tb[TCA_TUNNEL_KEY_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_TUNNEL_KEY_TM]);

			print_tm(f, tm);
		}
	}

	print_nl();

	return 0;
}

struct action_util tunnel_key_action_util = {
	.id = "tunnel_key",
	.parse_aopt = parse_tunnel_key,
	.print_aopt = print_tunnel_key,
};
