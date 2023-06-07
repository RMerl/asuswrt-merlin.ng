/*
 * tc_util.c		Misc TC utility functions.
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
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "utils.h"
#include "names.h"
#include "tc_util.h"
#include "tc_common.h"

#ifndef LIBDIR
#define LIBDIR "/usr/lib"
#endif

static struct db_names *cls_names;

#define NAMES_DB "/etc/iproute2/tc_cls"

int cls_names_init(char *path)
{
	int ret;

	cls_names = db_names_alloc();
	if (!cls_names)
		return -1;

	ret = db_names_load(cls_names, path ?: NAMES_DB);
	if (ret == -ENOENT && path) {
		fprintf(stderr, "Can't open class names file: %s\n", path);
		return -1;
	}
	if (ret) {
		db_names_free(cls_names);
		cls_names = NULL;
	}

	return 0;
}

void cls_names_uninit(void)
{
	db_names_free(cls_names);
}

const char *get_tc_lib(void)
{
	const char *lib_dir;

	lib_dir = getenv("TC_LIB_DIR");
	if (!lib_dir)
		lib_dir = LIBDIR "/tc/";

	return lib_dir;
}

int get_qdisc_handle(__u32 *h, const char *str)
{
	__u32 maj;
	char *p;

	maj = TC_H_UNSPEC;
	if (strcmp(str, "none") == 0)
		goto ok;
	maj = strtoul(str, &p, 16);
	if (p == str || maj >= (1 << 16))
		return -1;
	maj <<= 16;
	if (*p != ':' && *p != 0)
		return -1;
ok:
	*h = maj;
	return 0;
}

int get_tc_classid(__u32 *h, const char *str)
{
	__u32 maj, min;
	char *p;

	maj = TC_H_ROOT;
	if (strcmp(str, "root") == 0)
		goto ok;
	maj = TC_H_UNSPEC;
	if (strcmp(str, "none") == 0)
		goto ok;
	maj = strtoul(str, &p, 16);
	if (p == str) {
		maj = 0;
		if (*p != ':')
			return -1;
	}
	if (*p == ':') {
		if (maj >= (1<<16))
			return -1;
		maj <<= 16;
		str = p+1;
		min = strtoul(str, &p, 16);
		if (*p != 0)
			return -1;
		if (min >= (1<<16))
			return -1;
		maj |= min;
	} else if (*p != 0)
		return -1;

ok:
	*h = maj;
	return 0;
}

int print_tc_classid(char *buf, int blen, __u32 h)
{
	SPRINT_BUF(handle) = {};
	int hlen = SPRINT_BSIZE - 1;

	if (h == TC_H_ROOT)
		sprintf(handle, "root");
	else if (h == TC_H_UNSPEC)
		snprintf(handle, hlen, "none");
	else if (TC_H_MAJ(h) == 0)
		snprintf(handle, hlen, ":%x", TC_H_MIN(h));
	else if (TC_H_MIN(h) == 0)
		snprintf(handle, hlen, "%x:", TC_H_MAJ(h) >> 16);
	else
		snprintf(handle, hlen, "%x:%x", TC_H_MAJ(h) >> 16, TC_H_MIN(h));

	if (use_names) {
		char clname[IDNAME_MAX] = {};

		if (id_to_name(cls_names, h, clname))
			snprintf(buf, blen, "%s#%s", clname, handle);
		else
			snprintf(buf, blen, "%s", handle);
	} else {
		snprintf(buf, blen, "%s", handle);
	}

	return 0;
}

char *sprint_tc_classid(__u32 h, char *buf)
{
	if (print_tc_classid(buf, SPRINT_BSIZE-1, h))
		strcpy(buf, "???");
	return buf;
}

/* Parse a percent e.g: '30%'
 * return: 0 = ok, -1 = error, 1 = out of range
 */
int parse_percent(double *val, const char *str)
{
	char *p;

	*val = strtod(str, &p) / 100.;
	if (*val > 1.0 || *val < 0.0)
		return 1;
	if (*p && strcmp(p, "%"))
		return -1;

	return 0;
}

static int parse_percent_rate(char *rate, size_t len,
			      const char *str, const char *dev)
{
	long dev_mbit;
	int ret;
	double perc, rate_bit;
	char *str_perc = NULL;

	if (!dev[0]) {
		fprintf(stderr, "No device specified; specify device to rate limit by percentage\n");
		return -1;
	}

	if (read_prop(dev, "speed", &dev_mbit))
		return -1;

	ret = sscanf(str, "%m[0-9.%]", &str_perc);
	if (ret != 1)
		goto malf;

	ret = parse_percent(&perc, str_perc);
	if (ret == 1) {
		fprintf(stderr, "Invalid rate specified; should be between [0,100]%% but is %s\n", str);
		goto err;
	} else if (ret == -1) {
		goto malf;
	}

	free(str_perc);

	rate_bit = perc * dev_mbit * 1000 * 1000;

	ret = snprintf(rate, len, "%lf", rate_bit);
	if (ret <= 0 || ret >= len) {
		fprintf(stderr, "Unable to parse calculated rate\n");
		return -1;
	}

	return 0;

malf:
	fprintf(stderr, "Specified rate value could not be read or is malformed\n");
err:
	free(str_perc);
	return -1;
}

int get_percent_rate(unsigned int *rate, const char *str, const char *dev)
{
	char r_str[20];

	if (parse_percent_rate(r_str, sizeof(r_str), str, dev))
		return -1;

	return get_rate(rate, r_str);
}

int get_percent_rate64(__u64 *rate, const char *str, const char *dev)
{
	char r_str[20];

	if (parse_percent_rate(r_str, sizeof(r_str), str, dev))
		return -1;

	return get_rate64(rate, r_str);
}

void tc_print_rate(enum output_type t, const char *key, const char *fmt,
		   unsigned long long rate)
{
	print_rate(use_iec, t, key, fmt, rate);
}

char *sprint_ticks(__u32 ticks, char *buf)
{
	return sprint_time(tc_core_tick2time(ticks), buf);
}

int get_size_and_cell(unsigned int *size, int *cell_log, char *str)
{
	char *slash = strchr(str, '/');

	if (slash)
		*slash = 0;

	if (get_size(size, str))
		return -1;

	if (slash) {
		int cell;
		int i;

		if (get_integer(&cell, slash+1, 0))
			return -1;
		*slash = '/';

		for (i = 0; i < 32; i++) {
			if ((1<<i) == cell) {
				*cell_log = i;
				return 0;
			}
		}
		return -1;
	}
	return 0;
}

void print_devname(enum output_type type, int ifindex)
{
	const char *ifname = ll_index_to_name(ifindex);

	if (!is_json_context())
		printf("dev ");

	print_color_string(type, COLOR_IFNAME,
			   "dev", "%s ", ifname);
}

static const char *action_n2a(int action)
{
	static char buf[64];

	if (TC_ACT_EXT_CMP(action, TC_ACT_GOTO_CHAIN))
		return "goto";
	if (TC_ACT_EXT_CMP(action, TC_ACT_JUMP))
		return "jump";
	switch (action) {
	case TC_ACT_UNSPEC:
		return "continue";
	case TC_ACT_OK:
		return "pass";
	case TC_ACT_SHOT:
		return "drop";
	case TC_ACT_RECLASSIFY:
		return "reclassify";
	case TC_ACT_PIPE:
		return "pipe";
	case TC_ACT_STOLEN:
		return "stolen";
	case TC_ACT_TRAP:
		return "trap";
	default:
		snprintf(buf, 64, "%d", action);
		return buf;
	}
}

/* Convert action branch name into numeric format.
 *
 * Parameters:
 * @arg - string to parse
 * @result - pointer to output variable
 * @allow_num - whether @arg may be in numeric format already
 *
 * In error case, returns -1 and does not touch @result. Otherwise returns 0.
 */
int action_a2n(char *arg, int *result, bool allow_num)
{
	int n;
	char dummy;
	struct {
		const char *a;
		int n;
	} a2n[] = {
		{"continue", TC_ACT_UNSPEC},
		{"drop", TC_ACT_SHOT},
		{"shot", TC_ACT_SHOT},
		{"pass", TC_ACT_OK},
		{"ok", TC_ACT_OK},
		{"reclassify", TC_ACT_RECLASSIFY},
		{"pipe", TC_ACT_PIPE},
		{"goto", TC_ACT_GOTO_CHAIN},
		{"jump", TC_ACT_JUMP},
		{"trap", TC_ACT_TRAP},
		{ NULL },
	}, *iter;

	for (iter = a2n; iter->a; iter++) {
		if (matches(arg, iter->a) != 0)
			continue;
		n = iter->n;
		goto out_ok;
	}
	if (!allow_num || sscanf(arg, "%d%c", &n, &dummy) != 1)
		return -1;

out_ok:
	if (result)
		*result = n;
	return 0;
}

static int __parse_action_control(int *argc_p, char ***argv_p, int *result_p,
				  bool allow_num, bool ignore_a2n_miss)
{
	int argc = *argc_p;
	char **argv = *argv_p;
	int result;

	if (!argc)
		return -1;
	if (action_a2n(*argv, &result, allow_num) == -1) {
		if (!ignore_a2n_miss)
			fprintf(stderr, "Bad action type %s\n", *argv);
		return -1;
	}
	if (result == TC_ACT_GOTO_CHAIN) {
		__u32 chain_index;

		NEXT_ARG();
		if (matches(*argv, "chain") != 0) {
			fprintf(stderr, "\"chain index\" expected\n");
			return -1;
		}
		NEXT_ARG();
		if (get_u32(&chain_index, *argv, 10) ||
		    chain_index > TC_ACT_EXT_VAL_MASK) {
			fprintf(stderr, "Illegal \"chain index\"\n");
			return -1;
		}
		result |= chain_index;
	}
	if (result == TC_ACT_JUMP) {
		__u32 jump_cnt = 0;

		NEXT_ARG();
		if (get_u32(&jump_cnt, *argv, 10) ||
		    jump_cnt > TC_ACT_EXT_VAL_MASK) {
			fprintf(stderr, "Invalid \"jump count\" (%s)\n", *argv);
			return -1;
		}
		result |= jump_cnt;
	}
	NEXT_ARG_FWD();
	*argc_p = argc;
	*argv_p = argv;
	*result_p = result;
	return 0;
}

/* Parse action control including possible options.
 *
 * Parameters:
 * @argc_p - pointer to argc to parse
 * @argv_p - pointer to argv to parse
 * @result_p - pointer to output variable
 * @allow_num - whether action may be in numeric format already
 *
 * In error case, returns -1 and does not touch @result_1p. Otherwise returns 0.
 */
int parse_action_control(int *argc_p, char ***argv_p,
			 int *result_p, bool allow_num)
{
	return __parse_action_control(argc_p, argv_p, result_p,
				      allow_num, false);
}

/* Parse action control including possible options.
 *
 * Parameters:
 * @argc_p - pointer to argc to parse
 * @argv_p - pointer to argv to parse
 * @result_p - pointer to output variable
 * @allow_num - whether action may be in numeric format already
 * @default_result - set as a result in case of parsing error
 *
 * In case there is an error during parsing, the default result is used.
 */
void parse_action_control_dflt(int *argc_p, char ***argv_p,
			       int *result_p, bool allow_num,
			       int default_result)
{
	if (__parse_action_control(argc_p, argv_p, result_p, allow_num, true))
		*result_p = default_result;
}

static int parse_action_control_slash_spaces(int *argc_p, char ***argv_p,
					     int *result1_p, int *result2_p,
					     bool allow_num)
{
	int argc = *argc_p;
	char **argv = *argv_p;
	int result1 = -1, result2;
	int *result_p = &result1;
	int ok = 0;
	int ret;

	while (argc > 0) {
		switch (ok) {
		case 1:
			if (strcmp(*argv, "/") != 0)
				goto out;
			result_p = &result2;
			NEXT_ARG();
			/* fall-through */
		case 0: /* fall-through */
		case 2:
			ret = parse_action_control(&argc, &argv,
						   result_p, allow_num);
			if (ret)
				return ret;
			ok++;
			break;
		default:
			goto out;
		}
	}
out:
	*result1_p = result1;
	if (ok == 2)
		*result2_p = result2;
	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

/* Parse action control with slash including possible options.
 *
 * Parameters:
 * @argc_p - pointer to argc to parse
 * @argv_p - pointer to argv to parse
 * @result1_p - pointer to the first (before slash) output variable
 * @result2_p - pointer to the second (after slash) output variable
 * @allow_num - whether action may be in numeric format already
 *
 * In error case, returns -1 and does not touch @result*. Otherwise returns 0.
 */
int parse_action_control_slash(int *argc_p, char ***argv_p,
			       int *result1_p, int *result2_p, bool allow_num)
{
	int result1, result2, argc = *argc_p;
	char **argv = *argv_p;
	char *p = strchr(*argv, '/');

	if (!p)
		return parse_action_control_slash_spaces(argc_p, argv_p,
							 result1_p, result2_p,
							 allow_num);
	*p = 0;
	if (action_a2n(*argv, &result1, allow_num)) {
		*p = '/';
		return -1;
	}

	*p = '/';
	if (action_a2n(p + 1, &result2, allow_num))
		return -1;

	*result1_p = result1;
	*result2_p = result2;
	NEXT_ARG_FWD();
	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

void print_action_control(FILE *f, const char *prefix,
			  int action, const char *suffix)
{
	print_string(PRINT_FP, NULL, "%s", prefix);
	open_json_object("control_action");
	print_string(PRINT_ANY, "type", "%s", action_n2a(action));
	if (TC_ACT_EXT_CMP(action, TC_ACT_GOTO_CHAIN))
		print_uint(PRINT_ANY, "chain", " chain %u",
			   action & TC_ACT_EXT_VAL_MASK);
	if (TC_ACT_EXT_CMP(action, TC_ACT_JUMP))
		print_uint(PRINT_ANY, "jump", " %u",
			   action & TC_ACT_EXT_VAL_MASK);
	close_json_object();
	print_string(PRINT_FP, NULL, "%s", suffix);
}

int get_linklayer(unsigned int *val, const char *arg)
{
	int res;

	if (matches(arg, "ethernet") == 0)
		res = LINKLAYER_ETHERNET;
	else if (matches(arg, "atm") == 0)
		res = LINKLAYER_ATM;
	else if (matches(arg, "adsl") == 0)
		res = LINKLAYER_ATM;
	else
		return -1; /* Indicate error */

	*val = res;
	return 0;
}

static void print_linklayer(char *buf, int len, unsigned int linklayer)
{
	switch (linklayer) {
	case LINKLAYER_UNSPEC:
		snprintf(buf, len, "%s", "unspec");
		return;
	case LINKLAYER_ETHERNET:
		snprintf(buf, len, "%s", "ethernet");
		return;
	case LINKLAYER_ATM:
		snprintf(buf, len, "%s", "atm");
		return;
	default:
		snprintf(buf, len, "%s", "unknown");
		return;
	}
}

char *sprint_linklayer(unsigned int linklayer, char *buf)
{
	print_linklayer(buf, SPRINT_BSIZE-1, linklayer);
	return buf;
}

void print_tm(FILE *f, const struct tcf_t *tm)
{
	int hz = get_user_hz();

	if (tm->install != 0)
		print_uint(PRINT_ANY, "installed", " installed %u sec",
			   tm->install / hz);

	if (tm->lastuse != 0)
		print_uint(PRINT_ANY, "last_used", " used %u sec",
			   tm->lastuse / hz);

	if (tm->firstuse != 0)
		print_uint(PRINT_ANY, "first_used", " firstused %u sec",
			   tm->firstuse / hz);

	if (tm->expires != 0)
		print_uint(PRINT_ANY, "expires", " expires %u sec",
			   tm->expires / hz);
}

static void print_tcstats_basic_hw(struct rtattr **tbs, char *prefix)
{
	struct gnet_stats_basic bs_hw;

	if (!tbs[TCA_STATS_BASIC_HW])
		return;

	memcpy(&bs_hw, RTA_DATA(tbs[TCA_STATS_BASIC_HW]),
	       MIN(RTA_PAYLOAD(tbs[TCA_STATS_BASIC_HW]), sizeof(bs_hw)));

	if (bs_hw.bytes == 0 && bs_hw.packets == 0)
		return;

	if (tbs[TCA_STATS_BASIC]) {
		struct gnet_stats_basic bs;

		memcpy(&bs, RTA_DATA(tbs[TCA_STATS_BASIC]),
		       MIN(RTA_PAYLOAD(tbs[TCA_STATS_BASIC]),
			   sizeof(bs)));

		if (bs.bytes >= bs_hw.bytes && bs.packets >= bs_hw.packets) {
			print_nl();
			print_string(PRINT_FP, NULL, "%s", prefix);
			print_lluint(PRINT_ANY, "sw_bytes",
				     "Sent software %llu bytes",
				     bs.bytes - bs_hw.bytes);
			print_uint(PRINT_ANY, "sw_packets", " %u pkt",
				   bs.packets - bs_hw.packets);
		}
	}

	print_nl();
	print_string(PRINT_FP, NULL, "%s", prefix);
	print_lluint(PRINT_ANY, "hw_bytes", "Sent hardware %llu bytes",
		     bs_hw.bytes);
	print_uint(PRINT_ANY, "hw_packets", " %u pkt", bs_hw.packets);
}

void print_tcstats2_attr(FILE *fp, struct rtattr *rta, char *prefix, struct rtattr **xstats)
{
	struct rtattr *tbs[TCA_STATS_MAX + 1];

	parse_rtattr_nested(tbs, TCA_STATS_MAX, rta);

	if (tbs[TCA_STATS_BASIC]) {
		struct gnet_stats_basic bs = {0};
		__u64 packets64 = 0;

		if (tbs[TCA_STATS_PKT64])
			packets64 = rta_getattr_u64(tbs[TCA_STATS_PKT64]);

		memcpy(&bs, RTA_DATA(tbs[TCA_STATS_BASIC]),
		       MIN(RTA_PAYLOAD(tbs[TCA_STATS_BASIC]), sizeof(bs)));
		print_string(PRINT_FP, NULL, "%s", prefix);
		print_lluint(PRINT_ANY, "bytes", "Sent %llu bytes", bs.bytes);
		if (packets64)
			print_lluint(PRINT_ANY, "packets",
				     " %llu pkt", packets64);
		else
			print_uint(PRINT_ANY, "packets",
				   " %u pkt", bs.packets);
	}

	if (tbs[TCA_STATS_QUEUE]) {
		struct gnet_stats_queue q = {0};

		memcpy(&q, RTA_DATA(tbs[TCA_STATS_QUEUE]),
		       MIN(RTA_PAYLOAD(tbs[TCA_STATS_QUEUE]), sizeof(q)));
		print_uint(PRINT_ANY, "drops", " (dropped %u", q.drops);
		print_uint(PRINT_ANY, "overlimits", ", overlimits %u",
			   q.overlimits);
		print_uint(PRINT_ANY, "requeues", " requeues %u) ", q.requeues);
	}

	if (tbs[TCA_STATS_BASIC_HW])
		print_tcstats_basic_hw(tbs, prefix);

	if (tbs[TCA_STATS_RATE_EST64]) {
		struct gnet_stats_rate_est64 re = {0};

		memcpy(&re, RTA_DATA(tbs[TCA_STATS_RATE_EST64]),
		       MIN(RTA_PAYLOAD(tbs[TCA_STATS_RATE_EST64]),
			   sizeof(re)));
		print_string(PRINT_FP, NULL, "\n%s", prefix);
		print_lluint(PRINT_JSON, "rate", NULL, re.bps);
		tc_print_rate(PRINT_FP, NULL, "rate %s", re.bps);
		print_lluint(PRINT_ANY, "pps", " %llupps", re.pps);
	} else if (tbs[TCA_STATS_RATE_EST]) {
		struct gnet_stats_rate_est re = {0};

		memcpy(&re, RTA_DATA(tbs[TCA_STATS_RATE_EST]),
		       MIN(RTA_PAYLOAD(tbs[TCA_STATS_RATE_EST]), sizeof(re)));
		print_string(PRINT_FP, NULL, "\n%s", prefix);
		print_uint(PRINT_JSON, "rate", NULL, re.bps);
		tc_print_rate(PRINT_FP, NULL, "rate %s", re.bps);
		print_uint(PRINT_ANY, "pps", " %upps", re.pps);
	}

	if (tbs[TCA_STATS_QUEUE]) {
		struct gnet_stats_queue q = {0};

		memcpy(&q, RTA_DATA(tbs[TCA_STATS_QUEUE]),
		       MIN(RTA_PAYLOAD(tbs[TCA_STATS_QUEUE]), sizeof(q)));
		if (!tbs[TCA_STATS_RATE_EST])
			print_nl();
		print_string(PRINT_FP, NULL, "%s", prefix);
		print_size(PRINT_ANY, "backlog", "backlog %s", q.backlog);
		print_uint(PRINT_ANY, "qlen", " %up", q.qlen);
		print_uint(PRINT_FP, NULL, " requeues %u", q.requeues);
	}

	if (xstats)
		*xstats = tbs[TCA_STATS_APP] ? : NULL;
}

void print_tcstats_attr(FILE *fp, struct rtattr *tb[], char *prefix,
			struct rtattr **xstats)
{
	if (tb[TCA_STATS2]) {
		print_tcstats2_attr(fp, tb[TCA_STATS2], prefix, xstats);
		if (xstats && !*xstats)
			goto compat_xstats;
		return;
	}
	/* backward compatibility */
	if (tb[TCA_STATS]) {
		struct tc_stats st = {};

		/* handle case where kernel returns more/less than we know about */
		memcpy(&st, RTA_DATA(tb[TCA_STATS]),
		       MIN(RTA_PAYLOAD(tb[TCA_STATS]), sizeof(st)));

		fprintf(fp,
			"%sSent %llu bytes %u pkts (dropped %u, overlimits %u) ",
			prefix, (unsigned long long)st.bytes,
			st.packets, st.drops, st.overlimits);

		if (st.bps || st.pps || st.qlen || st.backlog) {
			fprintf(fp, "\n%s", prefix);
			if (st.bps || st.pps) {
				fprintf(fp, "rate ");
				if (st.bps)
					tc_print_rate(PRINT_FP, NULL, "%s ",
						      st.bps);
				if (st.pps)
					fprintf(fp, "%upps ", st.pps);
			}
			if (st.qlen || st.backlog) {
				fprintf(fp, "backlog ");
				if (st.backlog)
					print_size(PRINT_FP, NULL, "%s ",
						   st.backlog);
				if (st.qlen)
					fprintf(fp, "%up ", st.qlen);
			}
		}
	}

compat_xstats:
	if (tb[TCA_XSTATS] && xstats)
		*xstats = tb[TCA_XSTATS];
}

static void print_masked_type(__u32 type_max,
			      __u32 (*rta_getattr_type)(const struct rtattr *),
			      const char *name, struct rtattr *attr,
			      struct rtattr *mask_attr, bool newline)
{
	SPRINT_BUF(namefrm);
	__u32 value, mask;
	SPRINT_BUF(out);
	size_t done;

	if (!attr)
		return;

	value = rta_getattr_type(attr);
	mask = mask_attr ? rta_getattr_type(mask_attr) : type_max;

	if (is_json_context()) {
		sprintf(namefrm, "\n  %s %%u", name);
		print_hu(PRINT_ANY, name, namefrm,
			 rta_getattr_type(attr));
		if (mask != type_max) {
			char mask_name[SPRINT_BSIZE-6];

			sprintf(mask_name, "%s_mask", name);
			if (newline)
				print_string(PRINT_FP, NULL, "%s ", _SL_);
			sprintf(namefrm, " %s %%u", mask_name);
			print_hu(PRINT_ANY, mask_name, namefrm, mask);
		}
	} else {
		done = sprintf(out, "%u", value);
		if (mask != type_max)
			sprintf(out + done, "/0x%x", mask);
		if (newline)
			print_string(PRINT_FP, NULL, "%s ", _SL_);
		sprintf(namefrm, " %s %%s", name);
		print_string(PRINT_ANY, name, namefrm, out);
	}
}

void print_masked_u32(const char *name, struct rtattr *attr,
		      struct rtattr *mask_attr, bool newline)
{
	print_masked_type(UINT32_MAX, rta_getattr_u32, name, attr, mask_attr,
			  newline);
}

static __u32 __rta_getattr_u16_u32(const struct rtattr *attr)
{
	return rta_getattr_u16(attr);
}

void print_masked_u16(const char *name, struct rtattr *attr,
		      struct rtattr *mask_attr, bool newline)
{
	print_masked_type(UINT16_MAX, __rta_getattr_u16_u32, name, attr,
			  mask_attr, newline);
}

static __u32 __rta_getattr_u8_u32(const struct rtattr *attr)
{
	return rta_getattr_u8(attr);
}

void print_masked_u8(const char *name, struct rtattr *attr,
		     struct rtattr *mask_attr, bool newline)
{
	print_masked_type(UINT8_MAX,  __rta_getattr_u8_u32, name, attr,
			  mask_attr, newline);
}

static __u32 __rta_getattr_be16_u32(const struct rtattr *attr)
{
	return rta_getattr_be16(attr);
}

void print_masked_be16(const char *name, struct rtattr *attr,
		       struct rtattr *mask_attr, bool newline)
{
	print_masked_type(UINT16_MAX, __rta_getattr_be16_u32, name, attr,
			  mask_attr, newline);
}
