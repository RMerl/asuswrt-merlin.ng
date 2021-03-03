/*
 * q_taprio.c	Time Aware Priority Scheduler
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Vinicius Costa Gomes <vinicius.gomes@intel.com>
 * 		Jesus Sanchez-Palencia <jesus.sanchez-palencia@intel.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "utils.h"
#include "tc_util.h"
#include "list.h"

struct sched_entry {
	struct list_head list;
	uint32_t index;
	uint32_t interval;
	uint32_t gatemask;
	uint8_t cmd;
};

#define CLOCKID_INVALID (-1)
static const struct static_clockid {
	const char *name;
	clockid_t clockid;
} clockids_sysv[] = {
	{ "REALTIME", CLOCK_REALTIME },
	{ "TAI", CLOCK_TAI },
	{ "BOOTTIME", CLOCK_BOOTTIME },
	{ "MONOTONIC", CLOCK_MONOTONIC },
	{ NULL }
};

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... taprio clockid CLOCKID\n"
		"		[num_tc NUMBER] [map P0 P1 ...] "
		"		[queues COUNT@OFFSET COUNT@OFFSET COUNT@OFFSET ...] "
		"		[ [sched-entry index cmd gate-mask interval] ... ] "
		"		[base-time time] [txtime-delay delay]"
		"\n"
		"CLOCKID must be a valid SYS-V id (i.e. CLOCK_TAI)\n");
}

static void explain_clockid(const char *val)
{
	fprintf(stderr, "taprio: illegal value for \"clockid\": \"%s\".\n", val);
	fprintf(stderr, "It must be a valid SYS-V id (i.e. CLOCK_TAI)\n");
}

static int get_clockid(__s32 *val, const char *arg)
{
	const struct static_clockid *c;

	/* Drop the CLOCK_ prefix if that is being used. */
	if (strcasestr(arg, "CLOCK_") != NULL)
		arg += sizeof("CLOCK_") - 1;

	for (c = clockids_sysv; c->name; c++) {
		if (strcasecmp(c->name, arg) == 0) {
			*val = c->clockid;

			return 0;
		}
	}

	return -1;
}

static const char* get_clock_name(clockid_t clockid)
{
	const struct static_clockid *c;

	for (c = clockids_sysv; c->name; c++) {
		if (clockid == c->clockid)
			return c->name;
	}

	return "invalid";
}

static const char *entry_cmd_to_str(__u8 cmd)
{
	switch (cmd) {
	case TC_TAPRIO_CMD_SET_GATES:
		return "S";
	default:
		return "Invalid";
	}
}

static int str_to_entry_cmd(const char *str)
{
	if (strcmp(str, "S") == 0)
		return TC_TAPRIO_CMD_SET_GATES;

	return -1;
}

static int add_sched_list(struct list_head *sched_entries, struct nlmsghdr *n)
{
	struct sched_entry *e;

	list_for_each_entry(e, sched_entries, list) {
		struct rtattr *a;

		a = addattr_nest(n, 1024, TCA_TAPRIO_SCHED_ENTRY);

		addattr_l(n, 1024, TCA_TAPRIO_SCHED_ENTRY_CMD, &e->cmd, sizeof(e->cmd));
		addattr_l(n, 1024, TCA_TAPRIO_SCHED_ENTRY_GATE_MASK, &e->gatemask, sizeof(e->gatemask));
		addattr_l(n, 1024, TCA_TAPRIO_SCHED_ENTRY_INTERVAL, &e->interval, sizeof(e->interval));

		addattr_nest_end(n, a);
	}

	return 0;
}

static void explain_sched_entry(void)
{
	fprintf(stderr, "Usage: ... taprio ... sched-entry <cmd> <gate mask> <interval>\n");
}

static struct sched_entry *create_entry(uint32_t gatemask, uint32_t interval, uint8_t cmd)
{
	struct sched_entry *e;

	e = calloc(1, sizeof(*e));
	if (!e)
		return NULL;

	e->gatemask = gatemask;
	e->interval = interval;
	e->cmd = cmd;

	return e;
}

static int taprio_parse_opt(struct qdisc_util *qu, int argc,
			    char **argv, struct nlmsghdr *n, const char *dev)
{
	__s32 clockid = CLOCKID_INVALID;
	struct tc_mqprio_qopt opt = { };
	__s64 cycle_time_extension = 0;
	struct list_head sched_entries;
	struct rtattr *tail, *l;
	__u32 taprio_flags = 0;
	__u32 txtime_delay = 0;
	__s64 cycle_time = 0;
	__s64 base_time = 0;
	int err, idx;

	INIT_LIST_HEAD(&sched_entries);

	while (argc > 0) {
		idx = 0;
		if (strcmp(*argv, "num_tc") == 0) {
			NEXT_ARG();
			if (get_u8(&opt.num_tc, *argv, 10)) {
				fprintf(stderr, "Illegal \"num_tc\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "map") == 0) {
			while (idx < TC_QOPT_MAX_QUEUE && NEXT_ARG_OK()) {
				NEXT_ARG();
				if (get_u8(&opt.prio_tc_map[idx], *argv, 10)) {
					PREV_ARG();
					break;
				}
				idx++;
			}
			for ( ; idx < TC_QOPT_MAX_QUEUE; idx++)
				opt.prio_tc_map[idx] = 0;
		} else if (strcmp(*argv, "queues") == 0) {
			char *tmp, *tok;

			while (idx < TC_QOPT_MAX_QUEUE && NEXT_ARG_OK()) {
				NEXT_ARG();

				tmp = strdup(*argv);
				if (!tmp)
					break;

				tok = strtok(tmp, "@");
				if (get_u16(&opt.count[idx], tok, 10)) {
					free(tmp);
					PREV_ARG();
					break;
				}
				tok = strtok(NULL, "@");
				if (get_u16(&opt.offset[idx], tok, 10)) {
					free(tmp);
					PREV_ARG();
					break;
				}
				free(tmp);
				idx++;
			}
		} else if (strcmp(*argv, "sched-entry") == 0) {
			uint32_t mask, interval;
			struct sched_entry *e;
			uint8_t cmd;

			NEXT_ARG();
			err = str_to_entry_cmd(*argv);
			if (err < 0) {
				explain_sched_entry();
				return  -1;
			}
			cmd = err;

			NEXT_ARG();
			if (get_u32(&mask, *argv, 16)) {
				explain_sched_entry();
				return -1;
			}

			NEXT_ARG();
			if (get_u32(&interval, *argv, 0)) {
				explain_sched_entry();
				return -1;
			}

			e = create_entry(mask, interval, cmd);
			if (!e) {
				fprintf(stderr, "taprio: not enough memory for new schedule entry\n");
				return -1;
			}

			list_add_tail(&e->list, &sched_entries);

		} else if (strcmp(*argv, "base-time") == 0) {
			NEXT_ARG();
			if (get_s64(&base_time, *argv, 10)) {
				PREV_ARG();
				break;
			}
		} else if (strcmp(*argv, "cycle-time") == 0) {
			NEXT_ARG();
			if (cycle_time) {
				fprintf(stderr, "taprio: duplicate \"cycle-time\" specification\n");
				return -1;
			}

			if (get_s64(&cycle_time, *argv, 10)) {
				PREV_ARG();
				break;
			}

		} else if (strcmp(*argv, "cycle-time-extension") == 0) {
			NEXT_ARG();
			if (cycle_time_extension) {
				fprintf(stderr, "taprio: duplicate \"cycle-time-extension\" specification\n");
				return -1;
			}

			if (get_s64(&cycle_time_extension, *argv, 10)) {
				PREV_ARG();
				break;
			}
		} else if (strcmp(*argv, "clockid") == 0) {
			NEXT_ARG();
			if (clockid != CLOCKID_INVALID) {
				fprintf(stderr, "taprio: duplicate \"clockid\" specification\n");
				return -1;
			}
			if (get_clockid(&clockid, *argv)) {
				explain_clockid(*argv);
				return -1;
			}
		} else if (strcmp(*argv, "flags") == 0) {
			NEXT_ARG();
			if (taprio_flags) {
				fprintf(stderr, "taprio: duplicate \"flags\" specification\n");
				return -1;
			}
			if (get_u32(&taprio_flags, *argv, 0)) {
				PREV_ARG();
				return -1;
			}

		} else if (strcmp(*argv, "txtime-delay") == 0) {
			NEXT_ARG();
			if (txtime_delay != 0) {
				fprintf(stderr, "taprio: duplicate \"txtime-delay\" specification\n");
				return -1;
			}
			if (get_u32(&txtime_delay, *argv, 0)) {
				PREV_ARG();
				return -1;
			}

		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "Unknown argument\n");
			return -1;
		}
		argc--; argv++;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);

	if (clockid != CLOCKID_INVALID)
		addattr_l(n, 1024, TCA_TAPRIO_ATTR_SCHED_CLOCKID, &clockid, sizeof(clockid));

	if (taprio_flags)
		addattr_l(n, 1024, TCA_TAPRIO_ATTR_FLAGS, &taprio_flags, sizeof(taprio_flags));

	if (opt.num_tc > 0)
		addattr_l(n, 1024, TCA_TAPRIO_ATTR_PRIOMAP, &opt, sizeof(opt));

	if (txtime_delay)
		addattr_l(n, 1024, TCA_TAPRIO_ATTR_TXTIME_DELAY, &txtime_delay, sizeof(txtime_delay));

	if (base_time)
		addattr_l(n, 1024, TCA_TAPRIO_ATTR_SCHED_BASE_TIME, &base_time, sizeof(base_time));

	if (cycle_time)
		addattr_l(n, 1024, TCA_TAPRIO_ATTR_SCHED_CYCLE_TIME,
			  &cycle_time, sizeof(cycle_time));

	if (cycle_time_extension)
		addattr_l(n, 1024, TCA_TAPRIO_ATTR_SCHED_CYCLE_TIME_EXTENSION,
			  &cycle_time_extension, sizeof(cycle_time_extension));

	l = addattr_nest(n, 1024, TCA_TAPRIO_ATTR_SCHED_ENTRY_LIST | NLA_F_NESTED);

	err = add_sched_list(&sched_entries, n);
	if (err < 0) {
		fprintf(stderr, "Could not add schedule to netlink message\n");
		return -1;
	}

	addattr_nest_end(n, l);

	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int print_sched_list(FILE *f, struct rtattr *list)
{
	struct rtattr *item;
	int rem;

	if (list == NULL)
		return 0;

	rem = RTA_PAYLOAD(list);

	open_json_array(PRINT_JSON, "schedule");

	print_nl();

	for (item = RTA_DATA(list); RTA_OK(item, rem); item = RTA_NEXT(item, rem)) {
		struct rtattr *tb[TCA_TAPRIO_SCHED_ENTRY_MAX + 1];
		__u32 index = 0, gatemask = 0, interval = 0;
		__u8 command = 0;

		parse_rtattr_nested(tb, TCA_TAPRIO_SCHED_ENTRY_MAX, item);

		if (tb[TCA_TAPRIO_SCHED_ENTRY_INDEX])
			index = rta_getattr_u32(tb[TCA_TAPRIO_SCHED_ENTRY_INDEX]);

		if (tb[TCA_TAPRIO_SCHED_ENTRY_CMD])
			command = rta_getattr_u8(tb[TCA_TAPRIO_SCHED_ENTRY_CMD]);

		if (tb[TCA_TAPRIO_SCHED_ENTRY_GATE_MASK])
			gatemask = rta_getattr_u32(tb[TCA_TAPRIO_SCHED_ENTRY_GATE_MASK]);

		if (tb[TCA_TAPRIO_SCHED_ENTRY_INTERVAL])
			interval = rta_getattr_u32(tb[TCA_TAPRIO_SCHED_ENTRY_INTERVAL]);

		open_json_object(NULL);
		print_uint(PRINT_ANY, "index", "\tindex %u", index);
		print_string(PRINT_ANY, "cmd", " cmd %s", entry_cmd_to_str(command));
		print_0xhex(PRINT_ANY, "gatemask", " gatemask %#llx", gatemask);
		print_uint(PRINT_ANY, "interval", " interval %u", interval);
		close_json_object();

		print_nl();
	}

	close_json_array(PRINT_ANY, "");

	return 0;
}

static int print_schedule(FILE *f, struct rtattr **tb)
{
	int64_t base_time = 0, cycle_time = 0, cycle_time_extension = 0;

	if (tb[TCA_TAPRIO_ATTR_SCHED_BASE_TIME])
		base_time = rta_getattr_s64(tb[TCA_TAPRIO_ATTR_SCHED_BASE_TIME]);

	if (tb[TCA_TAPRIO_ATTR_SCHED_CYCLE_TIME])
		cycle_time = rta_getattr_s64(tb[TCA_TAPRIO_ATTR_SCHED_CYCLE_TIME]);

	if (tb[TCA_TAPRIO_ATTR_SCHED_CYCLE_TIME_EXTENSION])
		cycle_time_extension = rta_getattr_s64(
			tb[TCA_TAPRIO_ATTR_SCHED_CYCLE_TIME_EXTENSION]);

	print_lluint(PRINT_ANY, "base_time", "\tbase-time %lld", base_time);

	print_lluint(PRINT_ANY, "cycle_time", " cycle-time %lld", cycle_time);

	print_lluint(PRINT_ANY, "cycle_time_extension",
		     " cycle-time-extension %lld", cycle_time_extension);

	print_sched_list(f, tb[TCA_TAPRIO_ATTR_SCHED_ENTRY_LIST]);

	return 0;
}

static int taprio_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_TAPRIO_ATTR_MAX + 1];
	struct tc_mqprio_qopt *qopt = 0;
	__s32 clockid = CLOCKID_INVALID;
	int i;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_TAPRIO_ATTR_MAX, opt);

	if (tb[TCA_TAPRIO_ATTR_PRIOMAP] == NULL)
		return -1;

	qopt = RTA_DATA(tb[TCA_TAPRIO_ATTR_PRIOMAP]);

	print_uint(PRINT_ANY, "tc", "tc %u ", qopt->num_tc);

	open_json_array(PRINT_ANY, "map");
	for (i = 0; i <= TC_PRIO_MAX; i++)
		print_uint(PRINT_ANY, NULL, " %u", qopt->prio_tc_map[i]);
	close_json_array(PRINT_ANY, "");

	print_nl();

	open_json_array(PRINT_ANY, "queues");
	for (i = 0; i < qopt->num_tc; i++) {
		open_json_object(NULL);
		print_uint(PRINT_ANY, "offset", " offset %u", qopt->offset[i]);
		print_uint(PRINT_ANY, "count", " count %u", qopt->count[i]);
		close_json_object();
	}
	close_json_array(PRINT_ANY, "");

	print_nl();

	if (tb[TCA_TAPRIO_ATTR_SCHED_CLOCKID])
		clockid = rta_getattr_s32(tb[TCA_TAPRIO_ATTR_SCHED_CLOCKID]);

	print_string(PRINT_ANY, "clockid", "clockid %s", get_clock_name(clockid));

	if (tb[TCA_TAPRIO_ATTR_FLAGS]) {
		__u32 flags;

		flags = rta_getattr_u32(tb[TCA_TAPRIO_ATTR_FLAGS]);
		print_0xhex(PRINT_ANY, "flags", " flags %#x", flags);
	}

	if (tb[TCA_TAPRIO_ATTR_TXTIME_DELAY]) {
		__u32 txtime_delay;

		txtime_delay = rta_getattr_s32(tb[TCA_TAPRIO_ATTR_TXTIME_DELAY]);
		print_uint(PRINT_ANY, "txtime_delay", " txtime delay %d", txtime_delay);
	}

	print_schedule(f, tb);

	if (tb[TCA_TAPRIO_ATTR_ADMIN_SCHED]) {
		struct rtattr *t[TCA_TAPRIO_ATTR_MAX + 1];

		parse_rtattr_nested(t, TCA_TAPRIO_ATTR_MAX,
				    tb[TCA_TAPRIO_ATTR_ADMIN_SCHED]);

		open_json_object(NULL);

		print_schedule(f, t);

		close_json_object();
	}

	return 0;
}

struct qdisc_util taprio_qdisc_util = {
	.id		= "taprio",
	.parse_qopt	= taprio_parse_opt,
	.print_qopt	= taprio_print_opt,
};
