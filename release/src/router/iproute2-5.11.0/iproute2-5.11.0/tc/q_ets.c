// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause

/*
 * Enhanced Transmission Selection - 802.1Qaz-based Qdisc
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr, "Usage: ... ets [bands NUMBER] [strict NUMBER] [quanta Q1 Q2...] [priomap P1 P2...]\n");
}

static void cexplain(void)
{
	fprintf(stderr, "Usage: ... ets [quantum Q1]\n");
}

static unsigned int parse_quantum(const char *arg)
{
	unsigned int quantum;

	if (get_unsigned(&quantum, arg, 10)) {
		fprintf(stderr, "Illegal \"quanta\" element\n");
		return 0;
	}
	if (!quantum)
		fprintf(stderr, "\"quanta\" must be > 0\n");
	return quantum;
}

static int parse_nbands(const char *arg, __u8 *pnbands, const char *what)
{
	unsigned int tmp;

	if (get_unsigned(&tmp, arg, 10)) {
		fprintf(stderr, "Illegal \"%s\"\n", what);
		return -1;
	}
	if (tmp > TCQ_ETS_MAX_BANDS) {
		fprintf(stderr, "The number of \"%s\" must be <= %d\n",
			what, TCQ_ETS_MAX_BANDS);
		return -1;
	}

	*pnbands = tmp;
	return 0;
}

static int ets_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			 struct nlmsghdr *n, const char *dev)
{
	__u8 nbands = 0;
	__u8 nstrict = 0;
	bool quanta_mode = false;
	unsigned int nquanta = 0;
	__u32 quanta[TCQ_ETS_MAX_BANDS];
	bool priomap_mode = false;
	unsigned int nprio = 0;
	__u8 priomap[TC_PRIO_MAX + 1];
	unsigned int tmp;
	struct rtattr *tail, *nest;

	while (argc > 0) {
		if (strcmp(*argv, "bands") == 0) {
			if (nbands) {
				fprintf(stderr, "Duplicate \"bands\"\n");
				return -1;
			}
			NEXT_ARG();
			if (parse_nbands(*argv, &nbands, "bands"))
				return -1;
			priomap_mode = quanta_mode = false;
		} else if (strcmp(*argv, "strict") == 0) {
			if (nstrict) {
				fprintf(stderr, "Duplicate \"strict\"\n");
				return -1;
			}
			NEXT_ARG();
			if (parse_nbands(*argv, &nstrict, "strict"))
				return -1;
			priomap_mode = quanta_mode = false;
		} else if (strcmp(*argv, "quanta") == 0) {
			if (nquanta) {
				fprintf(stderr, "Duplicate \"quanta\"\n");
				return -1;
			}
			NEXT_ARG();
			priomap_mode = false;
			quanta_mode = true;
			goto parse_quantum;
		} else if (strcmp(*argv, "priomap") == 0) {
			if (nprio) {
				fprintf(stderr, "Duplicate \"priomap\"\n");
				return -1;
			}
			NEXT_ARG();
			priomap_mode = true;
			quanta_mode = false;
			goto parse_priomap;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else if (quanta_mode) {
			unsigned int quantum;

parse_quantum:
			quantum = parse_quantum(*argv);
			if (!quantum)
				return -1;
			quanta[nquanta++] = quantum;
		} else if (priomap_mode) {
			unsigned int band;

parse_priomap:
			if (get_unsigned(&band, *argv, 10)) {
				fprintf(stderr, "Illegal \"priomap\" element\n");
				return -1;
			}
			if (nprio > TC_PRIO_MAX) {
				fprintf(stderr, "\"priomap\" index cannot be higher than %u\n", TC_PRIO_MAX);
				return -1;
			}
			priomap[nprio++] = band;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

	if (!nbands)
		nbands = nquanta + nstrict;
	if (!nbands) {
		fprintf(stderr, "One of \"bands\", \"quanta\" or \"strict\" needs to be specified\n");
		explain();
		return -1;
	}
	if (nbands < 1) {
		fprintf(stderr, "The number of \"bands\" must be >= 1\n");
		explain();
		return -1;
	}
	if (nstrict + nquanta > nbands) {
		fprintf(stderr, "Not enough total bands to cover all the strict bands and quanta\n");
		explain();
		return -1;
	}
	for (tmp = 0; tmp < nprio; tmp++) {
		if (priomap[tmp] >= nbands) {
			fprintf(stderr, "\"priomap\" element is out of bounds\n");
			return -1;
		}
	}

	tail = addattr_nest(n, 1024, TCA_OPTIONS | NLA_F_NESTED);
	addattr_l(n, 1024, TCA_ETS_NBANDS, &nbands, sizeof(nbands));
	if (nstrict)
		addattr_l(n, 1024, TCA_ETS_NSTRICT, &nstrict, sizeof(nstrict));
	if (nquanta) {
		nest = addattr_nest(n, 1024, TCA_ETS_QUANTA | NLA_F_NESTED);
		for (tmp = 0; tmp < nquanta; tmp++)
			addattr_l(n, 1024, TCA_ETS_QUANTA_BAND,
				  &quanta[tmp], sizeof(quanta[0]));
		addattr_nest_end(n, nest);
	}
	if (nprio) {
		nest = addattr_nest(n, 1024, TCA_ETS_PRIOMAP | NLA_F_NESTED);
		for (tmp = 0; tmp < nprio; tmp++)
			addattr_l(n, 1024, TCA_ETS_PRIOMAP_BAND,
				  &priomap[tmp], sizeof(priomap[0]));
		addattr_nest_end(n, nest);
	}
	addattr_nest_end(n, tail);

	return 0;
}

static int ets_parse_copt(struct qdisc_util *qu, int argc, char **argv,
			  struct nlmsghdr *n, const char *dev)
{
	unsigned int quantum = 0;
	struct rtattr *tail;

	while (argc > 0) {
		if (strcmp(*argv, "quantum") == 0) {
			if (quantum) {
				fprintf(stderr, "Duplicate \"quantum\"\n");
				return -1;
			}
			NEXT_ARG();
			quantum = parse_quantum(*argv);
			if (!quantum)
				return -1;
		} else if (strcmp(*argv, "help") == 0) {
			cexplain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			cexplain();
			return -1;
		}
		argc--; argv++;
	}

	tail = addattr_nest(n, 1024, TCA_OPTIONS | NLA_F_NESTED);
	if (quantum)
		addattr_l(n, 1024, TCA_ETS_QUANTA_BAND, &quantum,
			  sizeof(quantum));
	addattr_nest_end(n, tail);

	return 0;
}

static int ets_print_opt_quanta(struct rtattr *opt)
{
	int len = RTA_PAYLOAD(opt);
	unsigned int offset;

	open_json_array(PRINT_ANY, "quanta");
	for (offset = 0; offset < len; ) {
		struct rtattr *tb[TCA_ETS_MAX + 1] = {NULL};
		struct rtattr *attr;
		__u32 quantum;

		attr = RTA_DATA(opt) + offset;
		parse_rtattr(tb, TCA_ETS_MAX, attr, len - offset);
		offset += RTA_LENGTH(RTA_PAYLOAD(attr));

		if (!tb[TCA_ETS_QUANTA_BAND]) {
			fprintf(stderr, "No ETS band quantum\n");
			return -1;
		}

		quantum = rta_getattr_u32(tb[TCA_ETS_QUANTA_BAND]);
		print_uint(PRINT_ANY, NULL, " %u", quantum);

	}
	close_json_array(PRINT_ANY, " ");

	return 0;
}

static int ets_print_opt_priomap(struct rtattr *opt)
{
	int len = RTA_PAYLOAD(opt);
	unsigned int offset;

	open_json_array(PRINT_ANY, "priomap");
	for (offset = 0; offset < len; ) {
		struct rtattr *tb[TCA_ETS_MAX + 1] = {NULL};
		struct rtattr *attr;
		__u8 band;

		attr = RTA_DATA(opt) + offset;
		parse_rtattr(tb, TCA_ETS_MAX, attr, len - offset);
		offset += RTA_LENGTH(RTA_PAYLOAD(attr)) + 3 /* padding */;

		if (!tb[TCA_ETS_PRIOMAP_BAND]) {
			fprintf(stderr, "No ETS priomap band\n");
			return -1;
		}

		band = rta_getattr_u8(tb[TCA_ETS_PRIOMAP_BAND]);
		print_uint(PRINT_ANY, NULL, " %u", band);

	}
	close_json_array(PRINT_ANY, " ");

	return 0;
}

static int ets_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_ETS_MAX + 1];
	__u8 nbands;
	__u8 nstrict;
	int err;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_ETS_MAX, opt);

	if (!tb[TCA_ETS_NBANDS] || !tb[TCA_ETS_PRIOMAP]) {
		fprintf(stderr, "Incomplete ETS options\n");
		return -1;
	}

	nbands = rta_getattr_u8(tb[TCA_ETS_NBANDS]);
	print_uint(PRINT_ANY, "bands", "bands %u ", nbands);

	if (tb[TCA_ETS_NSTRICT]) {
		nstrict = rta_getattr_u8(tb[TCA_ETS_NSTRICT]);
		print_uint(PRINT_ANY, "strict", "strict %u ", nstrict);
	}

	if (tb[TCA_ETS_QUANTA]) {
		err = ets_print_opt_quanta(tb[TCA_ETS_QUANTA]);
		if (err)
			return err;
	}

	return ets_print_opt_priomap(tb[TCA_ETS_PRIOMAP]);
}

static int ets_print_copt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_ETS_MAX + 1];
	__u32 quantum;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_ETS_MAX, opt);

	if (tb[TCA_ETS_QUANTA_BAND]) {
		quantum = rta_getattr_u32(tb[TCA_ETS_QUANTA_BAND]);
		print_uint(PRINT_ANY, "quantum", "quantum %u ", quantum);
	}

	return 0;
}

struct qdisc_util ets_qdisc_util = {
	.id		= "ets",
	.parse_qopt	= ets_parse_opt,
	.parse_copt	= ets_parse_copt,
	.print_qopt	= ets_print_opt,
	.print_copt	= ets_print_copt,
};
