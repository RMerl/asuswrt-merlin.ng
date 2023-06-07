// SPDX-License-Identifier: GPL-2.0+

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <linux/dcbnl.h>

#include "dcb.h"
#include "utils.h"

static void dcb_maxrate_help_set(void)
{
	fprintf(stderr,
		"Usage: dcb maxrate set dev STRING\n"
		"           [ tc-maxrate RATE-MAP ]\n"
		"\n"
		" where RATE-MAP := [ RATE-MAP ] RATE-MAPPING\n"
		"       RATE-MAPPING := { all | TC }:RATE\n"
		"       TC := { 0 .. 7 }\n"
		"\n"
	);
}

static void dcb_maxrate_help_show(void)
{
	fprintf(stderr,
		"Usage: dcb [ -i ] maxrate show dev STRING\n"
		"           [ tc-maxrate ]\n"
		"\n"
	);
}

static void dcb_maxrate_help(void)
{
	fprintf(stderr,
		"Usage: dcb maxrate help\n"
		"\n"
	);
	dcb_maxrate_help_show();
	dcb_maxrate_help_set();
}

static int dcb_maxrate_parse_mapping_tc_maxrate(__u32 key, char *value, void *data)
{
	__u64 rate;

	if (get_rate64(&rate, value))
		return -EINVAL;

	return dcb_parse_mapping("TC", key, IEEE_8021QAZ_MAX_TCS - 1,
				 "RATE", rate, -1,
				 dcb_set_u64, data);
}

static void dcb_maxrate_print_tc_maxrate(struct dcb *dcb, const struct ieee_maxrate *maxrate)
{
	size_t size = ARRAY_SIZE(maxrate->tc_maxrate);
	SPRINT_BUF(b);
	size_t i;

	open_json_array(PRINT_JSON, "tc_maxrate");
	print_string(PRINT_FP, NULL, "tc-maxrate ", NULL);

	for (i = 0; i < size; i++) {
		snprintf(b, sizeof(b), "%zd:%%s ", i);
		print_rate(dcb->use_iec, PRINT_ANY, NULL, b, maxrate->tc_maxrate[i]);
	}

	close_json_array(PRINT_JSON, "tc_maxrate");
}

static void dcb_maxrate_print(struct dcb *dcb, const struct ieee_maxrate *maxrate)
{
	dcb_maxrate_print_tc_maxrate(dcb, maxrate);
	print_nl();
}

static int dcb_maxrate_get(struct dcb *dcb, const char *dev, struct ieee_maxrate *maxrate)
{
	return dcb_get_attribute(dcb, dev, DCB_ATTR_IEEE_MAXRATE, maxrate, sizeof(*maxrate));
}

static int dcb_maxrate_set(struct dcb *dcb, const char *dev, const struct ieee_maxrate *maxrate)
{
	return dcb_set_attribute(dcb, dev, DCB_ATTR_IEEE_MAXRATE, maxrate, sizeof(*maxrate));
}

static int dcb_cmd_maxrate_set(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	struct ieee_maxrate maxrate;
	int ret;

	if (!argc) {
		dcb_maxrate_help_set();
		return 0;
	}

	ret = dcb_maxrate_get(dcb, dev, &maxrate);
	if (ret)
		return ret;

	do {
		if (matches(*argv, "help") == 0) {
			dcb_maxrate_help_set();
			return 0;
		} else if (matches(*argv, "tc-maxrate") == 0) {
			NEXT_ARG();
			ret = parse_mapping(&argc, &argv, true,
					    &dcb_maxrate_parse_mapping_tc_maxrate, &maxrate);
			if (ret) {
				fprintf(stderr, "Invalid mapping %s\n", *argv);
				return ret;
			}
			continue;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			dcb_maxrate_help_set();
			return -EINVAL;
		}

		NEXT_ARG_FWD();
	} while (argc > 0);

	return dcb_maxrate_set(dcb, dev, &maxrate);
}

static int dcb_cmd_maxrate_show(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	struct ieee_maxrate maxrate;
	int ret;

	ret = dcb_maxrate_get(dcb, dev, &maxrate);
	if (ret)
		return ret;

	open_json_object(NULL);

	if (!argc) {
		dcb_maxrate_print(dcb, &maxrate);
		goto out;
	}

	do {
		if (matches(*argv, "help") == 0) {
			dcb_maxrate_help_show();
			return 0;
		} else if (matches(*argv, "tc-maxrate") == 0) {
			dcb_maxrate_print_tc_maxrate(dcb, &maxrate);
			print_nl();
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			dcb_maxrate_help_show();
			return -EINVAL;
		}

		NEXT_ARG_FWD();
	} while (argc > 0);

out:
	close_json_object();
	return 0;
}

int dcb_cmd_maxrate(struct dcb *dcb, int argc, char **argv)
{
	if (!argc || matches(*argv, "help") == 0) {
		dcb_maxrate_help();
		return 0;
	} else if (matches(*argv, "show") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv,
					 dcb_cmd_maxrate_show, dcb_maxrate_help_show);
	} else if (matches(*argv, "set") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv,
					 dcb_cmd_maxrate_set, dcb_maxrate_help_set);
	} else {
		fprintf(stderr, "What is \"%s\"?\n", *argv);
		dcb_maxrate_help();
		return -EINVAL;
	}
}
