/*
 * q_gred.c		GRED.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:    J Hadi Salim(hadi@nortelnetworks.com)
 *             code ruthlessly ripped from
 *	       Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
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
#include <math.h>

#include "utils.h"
#include "tc_util.h"

#include "tc_red.h"


#if 0
#define DPRINTF(format, args...) fprintf(stderr, format, ##args)
#else
#define DPRINTF(format, args...)
#endif

static void explain(void)
{
	fprintf(stderr,
		"Usage: tc qdisc { add | replace | change } ... gred setup vqs NUMBER\n"
		"           default DEFAULT_VQ [ grio ] [ limit BYTES ] [ecn] [harddrop]\n"
		"       tc qdisc change ... gred vq VQ [ prio VALUE ] limit BYTES\n"
		"           min BYTES max BYTES avpkt BYTES [ burst PACKETS ]\n"
		"           [ probability PROBABILITY ] [ bandwidth KBPS ] [ecn] [harddrop]\n");
}

static int init_gred(struct qdisc_util *qu, int argc, char **argv,
		     struct nlmsghdr *n)
{

	struct rtattr *tail;
	struct tc_gred_sopt opt = { 0 };
	__u32 limit = 0;

	opt.def_DP = MAX_DPs;

	while (argc > 0) {
		DPRINTF(stderr, "init_gred: invoked with %s\n", *argv);
		if (strcmp(*argv, "vqs") == 0 ||
		    strcmp(*argv, "DPs") == 0) {
			NEXT_ARG();
			if (get_unsigned(&opt.DPs, *argv, 10)) {
				fprintf(stderr, "Illegal \"vqs\"\n");
				return -1;
			} else if (opt.DPs > MAX_DPs) {
				fprintf(stderr, "GRED: only %u VQs are currently supported\n",
					MAX_DPs);
				return -1;
			}
		} else if (strcmp(*argv, "default") == 0) {
			if (opt.DPs == 0) {
				fprintf(stderr, "\"default\" must be defined after \"vqs\"\n");
				return -1;
			}
			NEXT_ARG();
			if (get_unsigned(&opt.def_DP, *argv, 10)) {
				fprintf(stderr, "Illegal \"default\"\n");
				return -1;
			} else if (opt.def_DP >= opt.DPs) {
				fprintf(stderr, "\"default\" must be less than \"vqs\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "grio") == 0) {
			opt.grio = 1;
		} else if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_size(&limit, *argv)) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "ecn") == 0) {
			opt.flags |= TC_RED_ECN;
		} else if (strcmp(*argv, "harddrop") == 0) {
			opt.flags |= TC_RED_HARDDROP;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

	if (!opt.DPs || opt.def_DP == MAX_DPs) {
		fprintf(stderr, "Illegal gred setup parameters\n");
		return -1;
	}

	DPRINTF("TC_GRED: sending DPs=%u def_DP=%u\n", opt.DPs, opt.def_DP);
	n->nlmsg_flags |= NLM_F_CREATE;
	tail = addattr_nest(n, 1024, TCA_OPTIONS);
	addattr_l(n, 1024, TCA_GRED_DPS, &opt, sizeof(struct tc_gred_sopt));
	if (limit)
		addattr32(n, 1024, TCA_GRED_LIMIT, limit);
	addattr_nest_end(n, tail);
	return 0;
}
/*
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*/
static int gred_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n, const char *dev)
{
	struct rtattr *tail, *entry, *vqs;
	int ok = 0;
	struct tc_gred_qopt opt = { 0 };
	unsigned int burst = 0;
	unsigned int avpkt = 0;
	unsigned int flags = 0;
	double probability = 0.02;
	unsigned int rate = 0;
	int parm;
	__u8 sbuf[256];
	__u32 max_P;

	opt.DP = MAX_DPs;

	while (argc > 0) {
		if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_size(&opt.limit, *argv)) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "setup") == 0) {
			if (ok) {
				fprintf(stderr, "Illegal \"setup\"\n");
				return -1;
			}
			return init_gred(qu, argc-1, argv+1, n);
		} else if (strcmp(*argv, "min") == 0) {
			NEXT_ARG();
			if (get_size(&opt.qth_min, *argv)) {
				fprintf(stderr, "Illegal \"min\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "max") == 0) {
			NEXT_ARG();
			if (get_size(&opt.qth_max, *argv)) {
				fprintf(stderr, "Illegal \"max\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "vq") == 0 ||
			   strcmp(*argv, "DP") == 0) {
			NEXT_ARG();
			if (get_unsigned(&opt.DP, *argv, 10)) {
				fprintf(stderr, "Illegal \"vq\"\n");
				return -1;
			} else if (opt.DP >= MAX_DPs) {
				fprintf(stderr, "GRED: only %u VQs are currently supported\n",
					MAX_DPs);
				return -1;
			} /* need a better error check */
			ok++;
		} else if (strcmp(*argv, "burst") == 0) {
			NEXT_ARG();
			if (get_unsigned(&burst, *argv, 0)) {
				fprintf(stderr, "Illegal \"burst\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "avpkt") == 0) {
			NEXT_ARG();
			if (get_size(&avpkt, *argv)) {
				fprintf(stderr, "Illegal \"avpkt\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "probability") == 0) {
			NEXT_ARG();
			if (sscanf(*argv, "%lg", &probability) != 1) {
				fprintf(stderr, "Illegal \"probability\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "prio") == 0) {
			NEXT_ARG();
			opt.prio = strtol(*argv, (char **)NULL, 10);
			/* some error check here */
			ok++;
		} else if (strcmp(*argv, "bandwidth") == 0) {
			NEXT_ARG();
			if (strchr(*argv, '%')) {
				if (get_percent_rate(&rate, *argv, dev)) {
					fprintf(stderr, "Illegal \"bandwidth\"\n");
					return -1;
				}
			} else if (get_rate(&rate, *argv)) {
				fprintf(stderr, "Illegal \"bandwidth\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "ecn") == 0) {
			flags |= TC_RED_ECN;
		} else if (strcmp(*argv, "harddrop") == 0) {
			flags |= TC_RED_HARDDROP;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

	if (!ok) {
		explain();
		return -1;
	}
	if (opt.DP == MAX_DPs || !opt.limit || !opt.qth_min || !opt.qth_max ||
	    !avpkt) {
		fprintf(stderr, "Required parameter (vq, limit, min, max, avpkt) is missing\n");
		return -1;
	}
	if (!burst) {
		burst = (2 * opt.qth_min + opt.qth_max) / (3 * avpkt);
		fprintf(stderr, "GRED: set burst to %u\n", burst);
	}
	if (!rate) {
		get_rate(&rate, "10Mbit");
		fprintf(stderr, "GRED: set bandwidth to 10Mbit\n");
	}
	if ((parm = tc_red_eval_ewma(opt.qth_min, burst, avpkt)) < 0) {
		fprintf(stderr, "GRED: failed to calculate EWMA constant.\n");
		return -1;
	}
	if (parm >= 10)
		fprintf(stderr, "GRED: WARNING. Burst %u seems to be too large.\n",
		    burst);
	opt.Wlog = parm;
	if ((parm = tc_red_eval_P(opt.qth_min, opt.qth_max, probability)) < 0) {
		fprintf(stderr, "GRED: failed to calculate probability.\n");
		return -1;
	}
	opt.Plog = parm;
	if ((parm = tc_red_eval_idle_damping(opt.Wlog, avpkt, rate, sbuf)) < 0)
	    {
		fprintf(stderr, "GRED: failed to calculate idle damping table.\n");
		return -1;
	}
	opt.Scell_log = parm;

	tail = addattr_nest(n, 1024, TCA_OPTIONS);
	addattr_l(n, 1024, TCA_GRED_PARMS, &opt, sizeof(opt));
	addattr_l(n, 1024, TCA_GRED_STAB, sbuf, 256);
	max_P = probability * pow(2, 32);
	addattr32(n, 1024, TCA_GRED_MAX_P, max_P);

	vqs = addattr_nest(n, 1024, TCA_GRED_VQ_LIST);
	entry = addattr_nest(n, 1024, TCA_GRED_VQ_ENTRY);
	addattr32(n, 1024, TCA_GRED_VQ_DP, opt.DP);
	addattr32(n, 1024, TCA_GRED_VQ_FLAGS, flags);
	addattr_nest_end(n, entry);
	addattr_nest_end(n, vqs);

	addattr_nest_end(n, tail);
	return 0;
}

struct tc_gred_info {
	bool	flags_present;
	__u64	bytes;
	__u32	packets;
	__u32	backlog;
	__u32	prob_drop;
	__u32	prob_mark;
	__u32	forced_drop;
	__u32	forced_mark;
	__u32	pdrop;
	__u32	other;
	__u32	flags;
};

static void
gred_parse_vqs(struct tc_gred_info *info, struct rtattr *vqs)
{
	int rem = RTA_PAYLOAD(vqs);
	unsigned int offset = 0;

	while (rem > offset) {
		struct rtattr *tb_entry[TCA_GRED_VQ_ENTRY_MAX + 1] = {};
		struct rtattr *tb[TCA_GRED_VQ_MAX + 1] = {};
		struct rtattr *entry;
		unsigned int len;
		unsigned int dp;

		entry = RTA_DATA(vqs) + offset;

		parse_rtattr(tb_entry, TCA_GRED_VQ_ENTRY_MAX, entry,
			     rem - offset);
		len = RTA_LENGTH(RTA_PAYLOAD(entry));
		offset += len;

		if (!tb_entry[TCA_GRED_VQ_ENTRY]) {
			fprintf(stderr,
				"ERROR: Failed to parse Virtual Queue entry\n");
			continue;
		}

		parse_rtattr_nested(tb, TCA_GRED_VQ_MAX,
				    tb_entry[TCA_GRED_VQ_ENTRY]);

		if (!tb[TCA_GRED_VQ_DP]) {
			fprintf(stderr,
				"ERROR: Virtual Queue without DP attribute\n");
			continue;
		}

		dp = rta_getattr_u32(tb[TCA_GRED_VQ_DP]);

		if (tb[TCA_GRED_VQ_STAT_BYTES])
			info[dp].bytes =
				rta_getattr_u32(tb[TCA_GRED_VQ_STAT_BYTES]);
		if (tb[TCA_GRED_VQ_STAT_PACKETS])
			info[dp].packets =
				rta_getattr_u32(tb[TCA_GRED_VQ_STAT_PACKETS]);
		if (tb[TCA_GRED_VQ_STAT_BACKLOG])
			info[dp].backlog =
				rta_getattr_u32(tb[TCA_GRED_VQ_STAT_BACKLOG]);
		if (tb[TCA_GRED_VQ_STAT_PROB_DROP])
			info[dp].prob_drop =
				rta_getattr_u32(tb[TCA_GRED_VQ_STAT_PROB_DROP]);
		if (tb[TCA_GRED_VQ_STAT_PROB_MARK])
			info[dp].prob_mark =
				rta_getattr_u32(tb[TCA_GRED_VQ_STAT_PROB_MARK]);
		if (tb[TCA_GRED_VQ_STAT_FORCED_DROP])
			info[dp].forced_drop =
				rta_getattr_u32(tb[TCA_GRED_VQ_STAT_FORCED_DROP]);
		if (tb[TCA_GRED_VQ_STAT_FORCED_MARK])
			info[dp].forced_mark =
				rta_getattr_u32(tb[TCA_GRED_VQ_STAT_FORCED_MARK]);
		if (tb[TCA_GRED_VQ_STAT_PDROP])
			info[dp].pdrop =
				rta_getattr_u32(tb[TCA_GRED_VQ_STAT_PDROP]);
		if (tb[TCA_GRED_VQ_STAT_OTHER])
			info[dp].other =
				rta_getattr_u32(tb[TCA_GRED_VQ_STAT_OTHER]);
		info[dp].flags_present = !!tb[TCA_GRED_VQ_FLAGS];
		if (tb[TCA_GRED_VQ_FLAGS])
			info[dp].flags =
				rta_getattr_u32(tb[TCA_GRED_VQ_FLAGS]);
	}
}

static void
gred_print_stats(struct tc_gred_info *info, struct tc_gred_qopt *qopt)
{
	__u64 bytes = info ? info->bytes : qopt->bytesin;

	if (!is_json_context())
		printf("\n  Queue size: ");

	print_size(PRINT_ANY, "qave", "average %s ", qopt->qave);
	print_size(PRINT_ANY, "backlog", "current %s ", qopt->backlog);

	if (!is_json_context())
		printf("\n  Dropped packets: ");

	if (info) {
		print_uint(PRINT_ANY, "forced_drop", "forced %u ",
			   info->forced_drop);
		print_uint(PRINT_ANY, "prob_drop", "early %u ",
			   info->prob_drop);
		print_uint(PRINT_ANY, "pdrop", "pdrop %u ", info->pdrop);
		print_uint(PRINT_ANY, "other", "other %u ", info->other);

		if (!is_json_context())
			printf("\n  Marked packets: ");
		print_uint(PRINT_ANY, "forced_mark", "forced %u ",
			   info->forced_mark);
		print_uint(PRINT_ANY, "prob_mark", "early %u ",
			   info->prob_mark);
	} else {
		print_uint(PRINT_ANY, "forced_drop", "forced %u ",
			   qopt->forced);
		print_uint(PRINT_ANY, "prob_drop", "early %u ", qopt->early);
		print_uint(PRINT_ANY, "pdrop", "pdrop %u ", qopt->pdrop);
		print_uint(PRINT_ANY, "other", "other %u ", qopt->other);
	}

	if (!is_json_context())
		printf("\n  Total packets: ");

	print_uint(PRINT_ANY, "packets", "%u ", qopt->packets);
	print_size(PRINT_ANY, "bytes", "(%s) ", bytes);
}

static int gred_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct tc_gred_info infos[MAX_DPs] = {};
	struct rtattr *tb[TCA_GRED_MAX + 1];
	struct tc_gred_sopt *sopt;
	struct tc_gred_qopt *qopt;
	bool vq_info = false;
	__u32 *max_p = NULL;
	__u32 *limit = NULL;
	unsigned int i;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_GRED_MAX, opt);

	if (tb[TCA_GRED_PARMS] == NULL)
		return -1;

	if (tb[TCA_GRED_MAX_P] &&
	    RTA_PAYLOAD(tb[TCA_GRED_MAX_P]) >= sizeof(__u32) * MAX_DPs)
		max_p = RTA_DATA(tb[TCA_GRED_MAX_P]);

	if (tb[TCA_GRED_LIMIT] &&
	    RTA_PAYLOAD(tb[TCA_GRED_LIMIT]) == sizeof(__u32))
		limit = RTA_DATA(tb[TCA_GRED_LIMIT]);

	sopt = RTA_DATA(tb[TCA_GRED_DPS]);
	qopt = RTA_DATA(tb[TCA_GRED_PARMS]);
	if (RTA_PAYLOAD(tb[TCA_GRED_DPS]) < sizeof(*sopt) ||
	    RTA_PAYLOAD(tb[TCA_GRED_PARMS]) < sizeof(*qopt)*MAX_DPs) {
		fprintf(f, "\n GRED received message smaller than expected\n");
		return -1;
	}

	if (tb[TCA_GRED_VQ_LIST]) {
		gred_parse_vqs(infos, tb[TCA_GRED_VQ_LIST]);
		vq_info = true;
	}

	print_uint(PRINT_ANY, "dp_cnt", "vqs %u ", sopt->DPs);
	print_uint(PRINT_ANY, "dp_default", "default %u ", sopt->def_DP);

	if (sopt->grio)
		print_bool(PRINT_ANY, "grio", "grio ", true);
	else
		print_bool(PRINT_ANY, "grio", NULL, false);

	if (limit)
		print_size(PRINT_ANY, "limit", "limit %s ", *limit);

	tc_red_print_flags(sopt->flags);

	open_json_array(PRINT_JSON, "vqs");
	for (i = 0; i < MAX_DPs; i++, qopt++) {
		if (qopt->DP >= MAX_DPs)
			continue;

		open_json_object(NULL);

		print_uint(PRINT_ANY, "vq", "\n vq %u ", qopt->DP);
		print_hhu(PRINT_ANY, "prio", "prio %hhu ", qopt->prio);
		print_size(PRINT_ANY, "limit", "limit %s ", qopt->limit);
		print_size(PRINT_ANY, "min", "min %s ", qopt->qth_min);
		print_size(PRINT_ANY, "max", "max %s ", qopt->qth_max);

		if (infos[i].flags_present)
			tc_red_print_flags(infos[i].flags);

		if (show_details) {
			print_uint(PRINT_ANY, "ewma", "ewma %u ", qopt->Wlog);
			if (max_p)
				print_float(PRINT_ANY, "probability",
					    "probability %lg ",
					    max_p[i] / pow(2, 32));
			else
				print_uint(PRINT_ANY, "Plog", "Plog %u ",
					   qopt->Plog);
			print_uint(PRINT_ANY, "Scell_log", "Scell_log %u ",
				   qopt->Scell_log);
		}
		if (show_stats)
			gred_print_stats(vq_info ? &infos[i] : NULL, qopt);
		close_json_object();
	}
	close_json_array(PRINT_JSON, "vqs");
	return 0;
}

struct qdisc_util gred_qdisc_util = {
	.id		= "gred",
	.parse_qopt	= gred_parse_opt,
	.print_qopt	= gred_print_opt,
};
