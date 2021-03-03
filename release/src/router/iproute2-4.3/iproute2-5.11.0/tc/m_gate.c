// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/* Copyright 2020 NXP */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/if_ether.h>
#include "utils.h"
#include "rt_names.h"
#include "tc_util.h"
#include "list.h"
#include <linux/tc_act/tc_gate.h>

struct gate_entry {
	struct list_head list;
	uint8_t	gate_state;
	uint32_t interval;
	int32_t ipv;
	int32_t maxoctets;
};

#define CLOCKID_INVALID (-1)
static const struct clockid_table {
	const char *name;
	clockid_t clockid;
} clockt_map[] = {
	{ "REALTIME", CLOCK_REALTIME },
	{ "TAI", CLOCK_TAI },
	{ "BOOTTIME", CLOCK_BOOTTIME },
	{ "MONOTONIC", CLOCK_MONOTONIC },
	{ NULL }
};

static void explain(void)
{
	fprintf(stderr,
		"Usage: gate [ priority PRIO-SPEC ] [ base-time BASE-TIME ]\n"
		"       [ cycle-time CYCLE-TIME ]\n"
		"       [ cycle-time-ext CYCLE-TIME-EXT ]\n"
		"       [ clockid CLOCKID ] [flags FLAGS]\n"
		"       [ sched-entry GATE0 INTERVAL [ INTERNAL-PRIO-VALUE MAX-OCTETS ] ]\n"
		"       [ sched-entry GATE1 INTERVAL [ INTERNAL-PRIO-VALUE MAX-OCTETS ] ]\n"
		"       ......\n"
		"       [ sched-entry GATEn INTERVAL [ INTERNAL-PRIO-VALUE MAX-OCTETS ] ]\n"
		"       [ CONTROL ]\n"
		"       GATEn := open | close\n"
		"       INTERVAL : nanoseconds period of gate slot\n"
		"       INTERNAL-PRIO-VALUE : internal priority decide which\n"
		"                             rx queue number direct to.\n"
		"                             default to be -1 which means wildcard.\n"
		"       MAX-OCTETS : maximum number of MSDU octets that are\n"
		"                    permitted to pas the gate during the\n"
		"                    specified TimeInterval.\n"
		"                    default to be -1 which means wildcard.\n"
		"       CONTROL := pipe | drop | continue | pass |\n"
		"                  goto chain <CHAIN_INDEX>\n");
}

static void usage(void)
{
	explain();
	exit(-1);
}

static void explain_entry_format(void)
{
	fprintf(stderr, "Usage: sched-entry <open | close> <interval> [ <interval ipv> <octets max bytes> ]\n");
}

static int parse_gate(struct action_util *a, int *argc_p, char ***argv_p,
		      int tca_id, struct nlmsghdr *n);
static int print_gate(struct action_util *au, FILE *f, struct rtattr *arg);

struct action_util gate_action_util = {
	.id = "gate",
	.parse_aopt = parse_gate,
	.print_aopt = print_gate,
};

static int get_clockid(__s32 *val, const char *arg)
{
	const struct clockid_table *c;

	if (strcasestr(arg, "CLOCK_") != NULL)
		arg += sizeof("CLOCK_") - 1;

	for (c = clockt_map; c->name; c++) {
		if (strcasecmp(c->name, arg) == 0) {
			*val = c->clockid;
			return 0;
		}
	}

	return -1;
}

static const char *get_clock_name(clockid_t clockid)
{
	const struct clockid_table *c;

	for (c = clockt_map; c->name; c++) {
		if (clockid == c->clockid)
			return c->name;
	}

	return "invalid";
}

static int get_gate_state(__u8 *val, const char *arg)
{
	if (!strcasecmp("OPEN", arg)) {
		*val = 1;
		return 0;
	}

	if (!strcasecmp("CLOSE", arg)) {
		*val = 0;
		return 0;
	}

	return -1;
}

static struct gate_entry *create_gate_entry(uint8_t gate_state,
					    uint32_t interval,
					    int32_t ipv,
					    int32_t maxoctets)
{
	struct gate_entry *e;

	e = calloc(1, sizeof(*e));
	if (!e)
		return NULL;

	e->gate_state = gate_state;
	e->interval = interval;
	e->ipv = ipv;
	e->maxoctets = maxoctets;

	return e;
}

static int add_gate_list(struct list_head *gate_entries, struct nlmsghdr *n)
{
	struct gate_entry *e;

	list_for_each_entry(e, gate_entries, list) {
		struct rtattr *a;

		a = addattr_nest(n, 1024, TCA_GATE_ONE_ENTRY | NLA_F_NESTED);

		if (e->gate_state)
			addattr(n, MAX_MSG, TCA_GATE_ENTRY_GATE);

		addattr_l(n, MAX_MSG, TCA_GATE_ENTRY_INTERVAL,
			  &e->interval, sizeof(e->interval));
		addattr_l(n, MAX_MSG, TCA_GATE_ENTRY_IPV,
			  &e->ipv, sizeof(e->ipv));
		addattr_l(n, MAX_MSG, TCA_GATE_ENTRY_MAX_OCTETS,
			  &e->maxoctets, sizeof(e->maxoctets));

		addattr_nest_end(n, a);
	}

	return 0;
}

static void free_entries(struct list_head *gate_entries)
{
	struct gate_entry *e, *n;

	list_for_each_entry_safe(e, n, gate_entries, list) {
		list_del(&e->list);
		free(e);
	}
}

static int parse_gate(struct action_util *a, int *argc_p, char ***argv_p,
		      int tca_id, struct nlmsghdr *n)
{
	struct tc_gate parm = {.action = TC_ACT_PIPE};
	struct list_head gate_entries;
	__s32 clockid = CLOCKID_INVALID;
	struct rtattr *tail, *nle;
	char **argv = *argv_p;
	int argc = *argc_p;
	__s64 base_time = 0;
	__s64 cycle_time = 0;
	__s64 cycle_time_ext = 0;
	int entry_num = 0;
	char *invalidarg;
	__u32 flags = 0;
	int prio = -1;

	int err;

	if (matches(*argv, "gate") != 0)
		return -1;

	NEXT_ARG();
	if (argc <= 0)
		return -1;

	INIT_LIST_HEAD(&gate_entries);

	while (argc > 0) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&parm.index, *argv, 10)) {
				invalidarg = "index";
				goto err_arg;
			}
		} else if (matches(*argv, "priority") == 0) {
			NEXT_ARG();
			if (get_s32(&prio, *argv, 0)) {
				invalidarg = "priority";
				goto err_arg;
			}
		} else if (matches(*argv, "base-time") == 0) {
			NEXT_ARG();
			if (get_s64(&base_time, *argv, 10) &&
			    get_time64(&base_time, *argv)) {
				invalidarg = "base-time";
				goto err_arg;
			}
		} else if (matches(*argv, "cycle-time") == 0) {
			NEXT_ARG();
			if (get_s64(&cycle_time, *argv, 10) &&
			    get_time64(&cycle_time, *argv)) {
				invalidarg = "cycle-time";
				goto err_arg;
			}
		} else if (matches(*argv, "cycle-time-ext") == 0) {
			NEXT_ARG();
			if (get_s64(&cycle_time_ext, *argv, 10) &&
			    get_time64(&cycle_time_ext, *argv)) {
				invalidarg = "cycle-time-ext";
				goto err_arg;
			}
		} else if (matches(*argv, "clockid") == 0) {
			NEXT_ARG();
			if (get_clockid(&clockid, *argv)) {
				invalidarg = "clockid";
				goto err_arg;
			}
		} else if (matches(*argv, "flags") == 0) {
			NEXT_ARG();
			if (get_u32(&flags, *argv, 0)) {
				invalidarg = "flags";
				goto err_arg;
			}
		} else if (matches(*argv, "sched-entry") == 0) {
			unsigned int maxoctets_uint = 0;
			int32_t maxoctets = -1;
			struct gate_entry *e;
			uint8_t gate_state = 0;
			__s64 interval_s64 = 0;
			uint32_t interval = 0;
			int32_t ipv = -1;

			if (!NEXT_ARG_OK()) {
				explain_entry_format();
				fprintf(stderr, "\"sched-entry\" is incomplete\n");
				free_entries(&gate_entries);
				return -1;
			}

			NEXT_ARG();

			if (get_gate_state(&gate_state, *argv)) {
				explain_entry_format();
				fprintf(stderr, "\"sched-entry\" is incomplete\n");
				free_entries(&gate_entries);
				return -1;
			}

			if (!NEXT_ARG_OK()) {
				explain_entry_format();
				fprintf(stderr, "\"sched-entry\" is incomplete\n");
				free_entries(&gate_entries);
				return -1;
			}

			NEXT_ARG();

			if (get_u32(&interval, *argv, 0) &&
			    get_time64(&interval_s64, *argv)) {
				explain_entry_format();
				fprintf(stderr, "\"sched-entry\" is incomplete\n");
				free_entries(&gate_entries);
				return -1;
			}

			if (interval_s64 > UINT_MAX) {
				fprintf(stderr, "\"interval\" is too large\n");
				free_entries(&gate_entries);
				return -1;
			} else if (interval_s64) {
				interval = interval_s64;
			}

			if (!NEXT_ARG_OK())
				goto create_entry;

			NEXT_ARG();

			if (get_s32(&ipv, *argv, 0)) {
				PREV_ARG();
				goto create_entry;
			}

			if (!gate_state)
				ipv = -1;

			if (!NEXT_ARG_OK())
				goto create_entry;

			NEXT_ARG();

			if (get_s32(&maxoctets, *argv, 0) &&
			    get_size(&maxoctets_uint, *argv))
				PREV_ARG();

			if (maxoctets_uint > INT_MAX) {
				fprintf(stderr, "\"maxoctets\" is too large\n");
				free_entries(&gate_entries);
				return -1;
			} else if (maxoctets_uint ) {
				maxoctets = maxoctets_uint;
			}

			if (!gate_state)
				maxoctets = -1;

create_entry:
			e = create_gate_entry(gate_state, interval,
					      ipv, maxoctets);
			if (!e) {
				fprintf(stderr, "gate: not enough memory\n");
				free_entries(&gate_entries);
				return -1;
			}

			list_add_tail(&e->list, &gate_entries);
			entry_num++;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			break;
		}

		argc--;
		argv++;
	}

	parse_action_control_dflt(&argc, &argv, &parm.action,
				  false, TC_ACT_PIPE);

	if (!entry_num && !parm.index) {
		fprintf(stderr, "gate: must add at least one entry\n");
		return -1;
	}

	tail = addattr_nest(n, MAX_MSG, tca_id | NLA_F_NESTED);
	addattr_l(n, MAX_MSG, TCA_GATE_PARMS, &parm, sizeof(parm));

	if (prio != -1)
		addattr_l(n, MAX_MSG, TCA_GATE_PRIORITY, &prio, sizeof(prio));

	if (flags)
		addattr_l(n, MAX_MSG, TCA_GATE_FLAGS, &flags, sizeof(flags));

	if (base_time)
		addattr_l(n, MAX_MSG, TCA_GATE_BASE_TIME,
			  &base_time, sizeof(base_time));

	if (cycle_time)
		addattr_l(n, MAX_MSG, TCA_GATE_CYCLE_TIME,
			  &cycle_time, sizeof(cycle_time));

	if (cycle_time_ext)
		addattr_l(n, MAX_MSG, TCA_GATE_CYCLE_TIME_EXT,
			  &cycle_time_ext, sizeof(cycle_time_ext));

	if (clockid != CLOCKID_INVALID)
		addattr_l(n, MAX_MSG, TCA_GATE_CLOCKID,
			  &clockid, sizeof(clockid));

	nle = addattr_nest(n, MAX_MSG, TCA_GATE_ENTRY_LIST | NLA_F_NESTED);
	err = add_gate_list(&gate_entries, n);
	if (err < 0) {
		fprintf(stderr, "Could not add entries to netlink message\n");
		free_entries(&gate_entries);
		return -1;
	}

	addattr_nest_end(n, nle);
	addattr_nest_end(n, tail);
	free_entries(&gate_entries);
	*argc_p = argc;
	*argv_p = argv;

	return 0;
err_arg:
	invarg(invalidarg, *argv);
	free_entries(&gate_entries);

	return -1;
}

static int print_gate_list(struct rtattr *list)
{
	struct rtattr *item;
	int rem;

	rem = RTA_PAYLOAD(list);

	print_string(PRINT_FP, NULL, "%s", _SL_);
	print_string(PRINT_FP, NULL, "\tschedule:%s", _SL_);
	open_json_array(PRINT_JSON, "schedule");

	for (item = RTA_DATA(list);
	     RTA_OK(item, rem);
	     item = RTA_NEXT(item, rem)) {
		struct rtattr *tb[TCA_GATE_ENTRY_MAX + 1];
		__u32 index = 0, interval = 0;
		__u8 gate_state = 0;
		__s32 ipv = -1, maxoctets = -1;
		SPRINT_BUF(buf);

		parse_rtattr_nested(tb, TCA_GATE_ENTRY_MAX, item);

		if (tb[TCA_GATE_ENTRY_INDEX])
			index = rta_getattr_u32(tb[TCA_GATE_ENTRY_INDEX]);

		if (tb[TCA_GATE_ENTRY_GATE])
			gate_state = 1;

		if (tb[TCA_GATE_ENTRY_INTERVAL])
			interval = rta_getattr_u32(tb[TCA_GATE_ENTRY_INTERVAL]);

		if (tb[TCA_GATE_ENTRY_IPV])
			ipv = rta_getattr_s32(tb[TCA_GATE_ENTRY_IPV]);

		if (tb[TCA_GATE_ENTRY_MAX_OCTETS])
			maxoctets = rta_getattr_s32(tb[TCA_GATE_ENTRY_MAX_OCTETS]);

		open_json_object(NULL);
		print_uint(PRINT_ANY, "number", "\t number %4u", index);
		print_string(PRINT_ANY, "gate_state", "\tgate-state %s ",
			     gate_state ? "open" : "close");

		print_uint(PRINT_JSON, "interval", NULL, interval);

		memset(buf, 0, sizeof(buf));
		print_string(PRINT_FP, NULL, "\tinterval %s",
			     sprint_time64(interval, buf));

		if (ipv != -1) {
			print_uint(PRINT_ANY, "ipv", "\t ipv %-10u", ipv);
		} else {
			print_int(PRINT_JSON, "ipv", NULL, ipv);
			print_string(PRINT_FP, NULL, "\t ipv %s", "wildcard");
		}

		if (maxoctets != -1) {
			print_size(PRINT_ANY, "max_octets", "\t max-octets %s",
				   maxoctets);
		} else {
			print_string(PRINT_FP, NULL,
				     "\t max-octets %s", "wildcard");
			print_int(PRINT_JSON, "max_octets", NULL, maxoctets);
		}

		close_json_object();
		print_string(PRINT_FP, NULL, "%s", _SL_);
	}

	close_json_array(PRINT_ANY, "");

	return 0;
}

static int print_gate(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct tc_gate *parm;
	struct rtattr *tb[TCA_GATE_MAX + 1];
	__s32 clockid = CLOCKID_INVALID;
	__s64 base_time = 0;
	__s64 cycle_time = 0;
	__s64 cycle_time_ext = 0;
	SPRINT_BUF(buf);
	int prio = -1;

	if (arg == NULL)
		return -1;

	parse_rtattr_nested(tb, TCA_GATE_MAX, arg);

	if (!tb[TCA_GATE_PARMS]) {
		fprintf(stderr, "Missing gate parameters\n");
		return -1;
	}

	print_string(PRINT_FP, NULL, "%s", "\n");

	parm = RTA_DATA(tb[TCA_GATE_PARMS]);

	if (tb[TCA_GATE_PRIORITY])
		prio = rta_getattr_s32(tb[TCA_GATE_PRIORITY]);

	if (prio != -1) {
		print_int(PRINT_ANY, "priority", "\tpriority %-8d", prio);
	} else {
		print_string(PRINT_FP, NULL, "\tpriority %s", "wildcard");
		print_int(PRINT_JSON, "priority", NULL, prio);
	}

	if (tb[TCA_GATE_CLOCKID])
		clockid = rta_getattr_s32(tb[TCA_GATE_CLOCKID]);
	print_string(PRINT_ANY, "clockid", "\tclockid %s",
		     get_clock_name(clockid));

	if (tb[TCA_GATE_FLAGS]) {
		__u32 flags;

		flags = rta_getattr_u32(tb[TCA_GATE_FLAGS]);
		print_0xhex(PRINT_ANY, "flags", "\tflags %#x", flags);
	}

	print_string(PRINT_FP, NULL, "%s", "\n");

	if (tb[TCA_GATE_BASE_TIME])
		base_time = rta_getattr_s64(tb[TCA_GATE_BASE_TIME]);

	memset(buf, 0, sizeof(buf));
	print_string(PRINT_FP, NULL, "\tbase-time %s",
		     sprint_time64(base_time, buf));
	print_lluint(PRINT_JSON, "base_time", NULL, base_time);

	if (tb[TCA_GATE_CYCLE_TIME])
		cycle_time = rta_getattr_s64(tb[TCA_GATE_CYCLE_TIME]);

	memset(buf, 0, sizeof(buf));
	print_string(PRINT_FP, NULL,
		     "\tcycle-time %s", sprint_time64(cycle_time, buf));
	print_lluint(PRINT_JSON, "cycle_time", NULL, cycle_time);

	if (tb[TCA_GATE_CYCLE_TIME_EXT])
		cycle_time_ext = rta_getattr_s64(tb[TCA_GATE_CYCLE_TIME_EXT]);

	memset(buf, 0, sizeof(buf));
	print_string(PRINT_FP, NULL, "\tcycle-time-ext %s",
		     sprint_time64(cycle_time_ext, buf));
	print_lluint(PRINT_JSON, "cycle_time_ext", NULL, cycle_time_ext);

	if (tb[TCA_GATE_ENTRY_LIST])
		print_gate_list(tb[TCA_GATE_ENTRY_LIST]);

	print_action_control(f, "\t", parm->action, "");

	print_uint(PRINT_ANY, "index", "\n\t index %u", parm->index);
	print_int(PRINT_ANY, "ref", " ref %d", parm->refcnt);
	print_int(PRINT_ANY, "bind", " bind %d", parm->bindcnt);

	if (show_stats) {
		if (tb[TCA_GATE_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_GATE_TM]);

			print_tm(f, tm);
		}
	}

	print_string(PRINT_FP, NULL, "%s", "\n");

	return 0;
}
