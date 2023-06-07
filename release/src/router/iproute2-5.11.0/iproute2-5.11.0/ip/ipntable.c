/*
 * Copyright (C)2006 USAGI/WIDE Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 */
/*
 * based on ipneigh.c
 */
/*
 * Authors:
 *	Masahide NAKAMURA @USAGI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <time.h>

#include "utils.h"
#include "ip_common.h"
#include "json_print.h"

static struct
{
	int family;
	int index;
#define NONE_DEV	(-1)
	const char *name;
} filter;

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr,
		"Usage: ip ntable change name NAME [ dev DEV ]\n"
		"	 [ thresh1 VAL ] [ thresh2 VAL ] [ thresh3 VAL ] [ gc_int MSEC ]\n"
		"	 [ PARMS ]\n"
		"Usage: ip ntable show [ dev DEV ] [ name NAME ]\n"

		"PARMS := [ base_reachable MSEC ] [ retrans MSEC ] [ gc_stale MSEC ]\n"
		"	 [ delay_probe MSEC ] [ queue LEN ]\n"
		"	 [ app_probes VAL ] [ ucast_probes VAL ] [ mcast_probes VAL ]\n"
		"	 [ anycast_delay MSEC ] [ proxy_delay MSEC ] [ proxy_queue LEN ]\n"
		"	 [ locktime MSEC ]\n"
		);

	exit(-1);
}

static int ipntable_modify(int cmd, int flags, int argc, char **argv)
{
	struct {
		struct nlmsghdr	n;
		struct ndtmsg		ndtm;
		char			buf[1024];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndtmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | flags,
		.n.nlmsg_type = cmd,
		.ndtm.ndtm_family = preferred_family,
	};
	char *namep = NULL;
	char *threshsp = NULL;
	char *gc_intp = NULL;
	char parms_buf[1024] = {};
	struct rtattr *parms_rta = (struct rtattr *)parms_buf;
	int parms_change = 0;

	parms_rta->rta_type = NDTA_PARMS;
	parms_rta->rta_len = RTA_LENGTH(0);

	while (argc > 0) {
		if (strcmp(*argv, "name") == 0) {
			int len;

			NEXT_ARG();
			if (namep)
				duparg("NAME", *argv);

			namep = *argv;
			len = strlen(namep) + 1;
			addattr_l(&req.n, sizeof(req), NDTA_NAME, namep, len);
		} else if (strcmp(*argv, "thresh1") == 0) {
			__u32 thresh1;

			NEXT_ARG();
			threshsp = *argv;

			if (get_u32(&thresh1, *argv, 0))
				invarg("\"thresh1\" value is invalid", *argv);

			addattr32(&req.n, sizeof(req), NDTA_THRESH1, thresh1);
		} else if (strcmp(*argv, "thresh2") == 0) {
			__u32 thresh2;

			NEXT_ARG();
			threshsp = *argv;

			if (get_u32(&thresh2, *argv, 0))
				invarg("\"thresh2\" value is invalid", *argv);

			addattr32(&req.n, sizeof(req), NDTA_THRESH2, thresh2);
		} else if (strcmp(*argv, "thresh3") == 0) {
			__u32 thresh3;

			NEXT_ARG();
			threshsp = *argv;

			if (get_u32(&thresh3, *argv, 0))
				invarg("\"thresh3\" value is invalid", *argv);

			addattr32(&req.n, sizeof(req), NDTA_THRESH3, thresh3);
		} else if (strcmp(*argv, "gc_int") == 0) {
			__u64 gc_int;

			NEXT_ARG();
			gc_intp = *argv;

			if (get_u64(&gc_int, *argv, 0))
				invarg("\"gc_int\" value is invalid", *argv);

			addattr_l(&req.n, sizeof(req), NDTA_GC_INTERVAL,
				  &gc_int, sizeof(gc_int));
		} else if (strcmp(*argv, "dev") == 0) {
			__u32 ifindex;

			NEXT_ARG();
			ifindex = ll_name_to_index(*argv);
			if (!ifindex)
				return nodev(*argv);

			rta_addattr32(parms_rta, sizeof(parms_buf),
				      NDTPA_IFINDEX, ifindex);
		} else if (strcmp(*argv, "base_reachable") == 0) {
			__u64 breachable;

			NEXT_ARG();

			if (get_u64(&breachable, *argv, 0))
				invarg("\"base_reachable\" value is invalid", *argv);

			rta_addattr_l(parms_rta, sizeof(parms_buf),
				      NDTPA_BASE_REACHABLE_TIME,
				      &breachable, sizeof(breachable));
			parms_change = 1;
		} else if (strcmp(*argv, "retrans") == 0) {
			__u64 retrans;

			NEXT_ARG();

			if (get_u64(&retrans, *argv, 0))
				invarg("\"retrans\" value is invalid", *argv);

			rta_addattr_l(parms_rta, sizeof(parms_buf),
				      NDTPA_RETRANS_TIME,
				      &retrans, sizeof(retrans));
			parms_change = 1;
		} else if (strcmp(*argv, "gc_stale") == 0) {
			__u64 gc_stale;

			NEXT_ARG();

			if (get_u64(&gc_stale, *argv, 0))
				invarg("\"gc_stale\" value is invalid", *argv);

			rta_addattr_l(parms_rta, sizeof(parms_buf),
				      NDTPA_GC_STALETIME,
				      &gc_stale, sizeof(gc_stale));
			parms_change = 1;
		} else if (strcmp(*argv, "delay_probe") == 0) {
			__u64 delay_probe;

			NEXT_ARG();

			if (get_u64(&delay_probe, *argv, 0))
				invarg("\"delay_probe\" value is invalid", *argv);

			rta_addattr_l(parms_rta, sizeof(parms_buf),
				      NDTPA_DELAY_PROBE_TIME,
				      &delay_probe, sizeof(delay_probe));
			parms_change = 1;
		} else if (strcmp(*argv, "queue") == 0) {
			__u32 queue;

			NEXT_ARG();

			if (get_u32(&queue, *argv, 0))
				invarg("\"queue\" value is invalid", *argv);

			rta_addattr32(parms_rta, sizeof(parms_buf),
				      NDTPA_QUEUE_LEN, queue);
			parms_change = 1;
		} else if (strcmp(*argv, "app_probes") == 0) {
			__u32 aprobe;

			NEXT_ARG();

			if (get_u32(&aprobe, *argv, 0))
				invarg("\"app_probes\" value is invalid", *argv);

			rta_addattr32(parms_rta, sizeof(parms_buf),
				      NDTPA_APP_PROBES, aprobe);
			parms_change = 1;
		} else if (strcmp(*argv, "ucast_probes") == 0) {
			__u32 uprobe;

			NEXT_ARG();

			if (get_u32(&uprobe, *argv, 0))
				invarg("\"ucast_probes\" value is invalid", *argv);

			rta_addattr32(parms_rta, sizeof(parms_buf),
				      NDTPA_UCAST_PROBES, uprobe);
			parms_change = 1;
		} else if (strcmp(*argv, "mcast_probes") == 0) {
			__u32 mprobe;

			NEXT_ARG();

			if (get_u32(&mprobe, *argv, 0))
				invarg("\"mcast_probes\" value is invalid", *argv);

			rta_addattr32(parms_rta, sizeof(parms_buf),
				      NDTPA_MCAST_PROBES, mprobe);
			parms_change = 1;
		} else if (strcmp(*argv, "anycast_delay") == 0) {
			__u64 anycast_delay;

			NEXT_ARG();

			if (get_u64(&anycast_delay, *argv, 0))
				invarg("\"anycast_delay\" value is invalid", *argv);

			rta_addattr_l(parms_rta, sizeof(parms_buf),
				      NDTPA_ANYCAST_DELAY,
				      &anycast_delay, sizeof(anycast_delay));
			parms_change = 1;
		} else if (strcmp(*argv, "proxy_delay") == 0) {
			__u64 proxy_delay;

			NEXT_ARG();

			if (get_u64(&proxy_delay, *argv, 0))
				invarg("\"proxy_delay\" value is invalid", *argv);

			rta_addattr_l(parms_rta, sizeof(parms_buf),
				      NDTPA_PROXY_DELAY,
				      &proxy_delay, sizeof(proxy_delay));
			parms_change = 1;
		} else if (strcmp(*argv, "proxy_queue") == 0) {
			__u32 pqueue;

			NEXT_ARG();

			if (get_u32(&pqueue, *argv, 0))
				invarg("\"proxy_queue\" value is invalid", *argv);

			rta_addattr32(parms_rta, sizeof(parms_buf),
				      NDTPA_PROXY_QLEN, pqueue);
			parms_change = 1;
		} else if (strcmp(*argv, "locktime") == 0) {
			__u64 locktime;

			NEXT_ARG();

			if (get_u64(&locktime, *argv, 0))
				invarg("\"locktime\" value is invalid", *argv);

			rta_addattr_l(parms_rta, sizeof(parms_buf),
				      NDTPA_LOCKTIME,
				      &locktime, sizeof(locktime));
			parms_change = 1;
		} else {
			invarg("unknown", *argv);
		}

		argc--; argv++;
	}

	if (!namep)
		missarg("NAME");
	if (!threshsp && !gc_intp && !parms_change) {
		fprintf(stderr, "Not enough information: changeable attributes required.\n");
		exit(-1);
	}

	if (parms_rta->rta_len > RTA_LENGTH(0)) {
		addattr_l(&req.n, sizeof(req), NDTA_PARMS, RTA_DATA(parms_rta),
			  RTA_PAYLOAD(parms_rta));
	}

	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		exit(2);

	return 0;
}

static const char *ntable_strtime_delta(__u32 msec)
{
	static char str[32];
	struct timeval now = {};
	time_t t;
	struct tm *tp;

	if (msec == 0)
		goto error;

	if (gettimeofday(&now, NULL) < 0) {
		perror("gettimeofday");
		goto error;
	}

	t = now.tv_sec - (msec / 1000);
	tp = localtime(&t);
	if (!tp)
		goto error;

	strftime(str, sizeof(str), "%Y-%m-%d %T", tp);

	return str;
 error:
	strcpy(str, "(error)");
	return str;
}

static void print_ndtconfig(const struct ndt_config *ndtc)
{

	print_uint(PRINT_ANY, "key_length",
		   "    config key_len %u ", ndtc->ndtc_key_len);
	print_uint(PRINT_ANY, "entry_size",
		   "entry_size %u ", ndtc->ndtc_entry_size);
	print_uint(PRINT_ANY, "entries", "entries %u ", ndtc->ndtc_entries);

	print_nl();

	print_string(PRINT_ANY, "last_flush",
		     "        last_flush %s ",
		     ntable_strtime_delta(ndtc->ndtc_last_flush));
	print_string(PRINT_ANY, "last_rand",
		     "last_rand %s ",
		     ntable_strtime_delta(ndtc->ndtc_last_rand));

	print_nl();

	print_uint(PRINT_ANY, "hash_rnd",
		   "        hash_rnd %u ", ndtc->ndtc_hash_rnd);
	print_0xhex(PRINT_ANY, "hash_mask",
		    "hash_mask %08llx ", ndtc->ndtc_hash_mask);

	print_uint(PRINT_ANY, "hash_chain_gc",
		   "hash_chain_gc %u ", ndtc->ndtc_hash_chain_gc);
	print_uint(PRINT_ANY, "proxy_qlen",
		   "proxy_qlen %u ", ndtc->ndtc_proxy_qlen);

	print_nl();
}

static void print_ndtparams(struct rtattr *tpb[])
{

	if (tpb[NDTPA_IFINDEX]) {
		__u32 ifindex = rta_getattr_u32(tpb[NDTPA_IFINDEX]);

		print_string(PRINT_FP, NULL, "    dev ", NULL);
		print_color_string(PRINT_ANY, COLOR_IFNAME,
				   "dev", "%s ", ll_index_to_name(ifindex));
		print_nl();
	}

	print_string(PRINT_FP, NULL, "    ", NULL);
	if (tpb[NDTPA_REFCNT]) {
		__u32 refcnt = rta_getattr_u32(tpb[NDTPA_REFCNT]);

		print_uint(PRINT_ANY, "refcnt", "refcnt %u ", refcnt);
	}

	if (tpb[NDTPA_REACHABLE_TIME]) {
		__u64 reachable = rta_getattr_u64(tpb[NDTPA_REACHABLE_TIME]);

		print_u64(PRINT_ANY, "reachable",
			   "reachable %llu ", reachable);
	}

	if (tpb[NDTPA_BASE_REACHABLE_TIME]) {
		__u64 breachable
			= rta_getattr_u64(tpb[NDTPA_BASE_REACHABLE_TIME]);

		print_u64(PRINT_ANY, "base_reachable",
			   "base_reachable %llu ", breachable);
	}

	if (tpb[NDTPA_RETRANS_TIME]) {
		__u64 retrans = rta_getattr_u64(tpb[NDTPA_RETRANS_TIME]);

		print_u64(PRINT_ANY, "retrans", "retrans %llu ", retrans);
	}

	print_string(PRINT_FP, NULL, "%s    ", _SL_);

	if (tpb[NDTPA_GC_STALETIME]) {
		__u64 gc_stale = rta_getattr_u64(tpb[NDTPA_GC_STALETIME]);

		print_u64(PRINT_ANY, "gc_stale", "gc_stale %llu ", gc_stale);
	}

	if (tpb[NDTPA_DELAY_PROBE_TIME]) {
		__u64 delay_probe
			= rta_getattr_u64(tpb[NDTPA_DELAY_PROBE_TIME]);

		print_u64(PRINT_ANY, "delay_probe",
			   "delay_probe %llu ", delay_probe);
	}

	if (tpb[NDTPA_QUEUE_LEN]) {
		__u32 queue = rta_getattr_u32(tpb[NDTPA_QUEUE_LEN]);

		print_uint(PRINT_ANY, "queue", "queue %u ", queue);
	}

	print_string(PRINT_FP, NULL, "%s    ", _SL_);

	if (tpb[NDTPA_APP_PROBES]) {
		__u32 aprobe = rta_getattr_u32(tpb[NDTPA_APP_PROBES]);

		print_uint(PRINT_ANY, "app_probes", "app_probes %u ", aprobe);
	}

	if (tpb[NDTPA_UCAST_PROBES]) {
		__u32 uprobe = rta_getattr_u32(tpb[NDTPA_UCAST_PROBES]);

		print_uint(PRINT_ANY, "ucast_probes",
			   "ucast_probes %u ", uprobe);
	}

	if (tpb[NDTPA_MCAST_PROBES]) {
		__u32 mprobe = rta_getattr_u32(tpb[NDTPA_MCAST_PROBES]);

		print_uint(PRINT_ANY, "mcast_probes",
			   "mcast_probes %u ", mprobe);
	}

	print_string(PRINT_FP, NULL, "%s    ", _SL_);

	if (tpb[NDTPA_ANYCAST_DELAY]) {
		__u64 anycast_delay = rta_getattr_u64(tpb[NDTPA_ANYCAST_DELAY]);

		print_u64(PRINT_ANY, "anycast_delay",
			   "anycast_delay %llu ", anycast_delay);
	}

	if (tpb[NDTPA_PROXY_DELAY]) {
		__u64 proxy_delay = rta_getattr_u64(tpb[NDTPA_PROXY_DELAY]);

		print_u64(PRINT_ANY, "proxy_delay",
			   "proxy_delay %llu ", proxy_delay);
	}

	if (tpb[NDTPA_PROXY_QLEN]) {
		__u32 pqueue = rta_getattr_u32(tpb[NDTPA_PROXY_QLEN]);

		print_uint(PRINT_ANY, "proxy_queue", "proxy_queue %u ", pqueue);
	}

	if (tpb[NDTPA_LOCKTIME]) {
		__u64 locktime = rta_getattr_u64(tpb[NDTPA_LOCKTIME]);

		print_u64(PRINT_ANY, "locktime", "locktime %llu ", locktime);
	}

	print_nl();
}

static void print_ndtstats(const struct ndt_stats *ndts)
{

	print_string(PRINT_FP, NULL, "    stats ", NULL);

	print_u64(PRINT_ANY, "allocs", "allocs %llu ", ndts->ndts_allocs);
	print_u64(PRINT_ANY, "destroys", "destroys %llu ",
		   ndts->ndts_destroys);
	print_u64(PRINT_ANY, "hash_grows", "hash_grows %llu ",
		   ndts->ndts_hash_grows);

	print_string(PRINT_FP, NULL, "%s    ", _SL_);

	print_u64(PRINT_ANY, "res_failed", "res_failed %llu ",
		   ndts->ndts_res_failed);
	print_u64(PRINT_ANY, "lookups", "lookups %llu ", ndts->ndts_lookups);
	print_u64(PRINT_ANY, "hits", "hits %llu ", ndts->ndts_hits);

	print_string(PRINT_FP, NULL, "%s    ", _SL_);

	print_u64(PRINT_ANY, "rcv_probes_mcast", "rcv_probes_mcast %llu ",
		   ndts->ndts_rcv_probes_mcast);
	print_u64(PRINT_ANY, "rcv_probes_ucast", "rcv_probes_ucast %llu ",
		   ndts->ndts_rcv_probes_ucast);

	print_string(PRINT_FP, NULL, "%s    ", _SL_);

	print_u64(PRINT_ANY, "periodic_gc_runs", "periodic_gc_runs %llu ",
		   ndts->ndts_periodic_gc_runs);
	print_u64(PRINT_ANY, "forced_gc_runs", "forced_gc_runs %llu ",
		   ndts->ndts_forced_gc_runs);

	print_string(PRINT_FP, NULL, "%s    ", _SL_);

	print_u64(PRINT_ANY, "table_fulls", "table_fulls %llu ",
		  ndts->ndts_table_fulls);

	print_nl();
}

static int print_ntable(struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE *)arg;
	struct ndtmsg *ndtm = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr *tb[NDTA_MAX+1];
	struct rtattr *tpb[NDTPA_MAX+1];
	int ret;

	if (n->nlmsg_type != RTM_NEWNEIGHTBL) {
		fprintf(stderr, "Not NEIGHTBL: %08x %08x %08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
		return 0;
	}
	len -= NLMSG_LENGTH(sizeof(*ndtm));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	if (preferred_family && preferred_family != ndtm->ndtm_family)
		return 0;

	parse_rtattr(tb, NDTA_MAX, NDTA_RTA(ndtm),
		     n->nlmsg_len - NLMSG_LENGTH(sizeof(*ndtm)));

	if (tb[NDTA_NAME]) {
		const char *name = rta_getattr_str(tb[NDTA_NAME]);

		if (filter.name && strcmp(filter.name, name))
			return 0;
	}

	if (tb[NDTA_PARMS]) {
		parse_rtattr(tpb, NDTPA_MAX, RTA_DATA(tb[NDTA_PARMS]),
			     RTA_PAYLOAD(tb[NDTA_PARMS]));

		if (tpb[NDTPA_IFINDEX]) {
			__u32 ifindex = rta_getattr_u32(tpb[NDTPA_IFINDEX]);

			if (filter.index && filter.index != ifindex)
				return 0;
		} else {
			if (filter.index && filter.index != NONE_DEV)
				return 0;
		}
	}

	open_json_object(NULL);
	print_string(PRINT_ANY, "family",
		     "%s ", family_name(ndtm->ndtm_family));

	if (tb[NDTA_NAME]) {
		const char *name = rta_getattr_str(tb[NDTA_NAME]);

		print_string(PRINT_ANY, "name", "%s ", name);
	}

	print_nl();

	ret = (tb[NDTA_THRESH1] || tb[NDTA_THRESH2] || tb[NDTA_THRESH3] ||
	       tb[NDTA_GC_INTERVAL]);
	if (ret)
		print_string(PRINT_FP, NULL, "    ", NULL);

	if (tb[NDTA_THRESH1]) {
		__u32 thresh1 = rta_getattr_u32(tb[NDTA_THRESH1]);

		print_uint(PRINT_ANY, "thresh1", "thresh1 %u ", thresh1);
	}

	if (tb[NDTA_THRESH2]) {
		__u32 thresh2 = rta_getattr_u32(tb[NDTA_THRESH2]);

		print_uint(PRINT_ANY, "thresh2", "thresh2 %u ", thresh2);
	}

	if (tb[NDTA_THRESH3]) {
		__u32 thresh3 = rta_getattr_u32(tb[NDTA_THRESH3]);

		print_uint(PRINT_ANY, "thresh3", "thresh3 %u ", thresh3);
	}

	if (tb[NDTA_GC_INTERVAL]) {
		__u64 gc_int = rta_getattr_u64(tb[NDTA_GC_INTERVAL]);

		print_u64(PRINT_ANY, "gc_interval", "gc_int %llu ", gc_int);
	}

	if (ret)
		print_nl();

	if (tb[NDTA_CONFIG] && show_stats)
		print_ndtconfig(RTA_DATA(tb[NDTA_CONFIG]));

	if (tb[NDTA_PARMS])
		print_ndtparams(tpb);

	if (tb[NDTA_STATS] && show_stats)
		print_ndtstats(RTA_DATA(tb[NDTA_STATS]));

	print_string(PRINT_FP, NULL, "\n", "");
	close_json_object();
	fflush(fp);

	return 0;
}

static void ipntable_reset_filter(void)
{
	memset(&filter, 0, sizeof(filter));
}

static int ipntable_show(int argc, char **argv)
{
	ipntable_reset_filter();

	filter.family = preferred_family;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();

			if (strcmp("none", *argv) == 0)
				filter.index = NONE_DEV;
			else if ((filter.index = ll_name_to_index(*argv)) == 0)
				invarg("\"DEV\" is invalid", *argv);
		} else if (strcmp(*argv, "name") == 0) {
			NEXT_ARG();

			filter.name = *argv;
		} else
			invarg("unknown", *argv);

		argc--; argv++;
	}

	if (rtnl_neightbldump_req(&rth, preferred_family) < 0) {
		perror("Cannot send dump request");
		exit(1);
	}

	new_json_obj(json);
	if (rtnl_dump_filter(&rth, print_ntable, stdout) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}
	delete_json_obj();

	return 0;
}

int do_ipntable(int argc, char **argv)
{
	ll_init_map(&rth);

	if (argc > 0) {
		if (matches(*argv, "change") == 0 ||
		    matches(*argv, "chg") == 0)
			return ipntable_modify(RTM_SETNEIGHTBL,
					       NLM_F_REPLACE,
					       argc-1, argv+1);
		if (matches(*argv, "show") == 0 ||
		    matches(*argv, "lst") == 0 ||
		    matches(*argv, "list") == 0)
			return ipntable_show(argc-1, argv+1);
		if (matches(*argv, "help") == 0)
			usage();
	} else
		return ipntable_show(0, NULL);

	fprintf(stderr, "Command \"%s\" is unknown, try \"ip ntable help\".\n", *argv);
	exit(-1);
}
