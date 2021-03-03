// SPDX-License-Identifier: GPL-2.0+

#include <errno.h>
#include <stdio.h>
#include <linux/dcbnl.h>

#include "dcb.h"
#include "utils.h"

static void dcb_ets_help_set(void)
{
	fprintf(stderr,
		"Usage: dcb ets set dev STRING\n"
		"           [ willing { on | off } ]\n"
		"           [ { tc-tsa | reco-tc-tsa } TSA-MAP ]\n"
		"           [ { pg-bw | tc-bw | reco-tc-bw } BW-MAP ]\n"
		"           [ { prio-tc | reco-prio-tc } PRIO-MAP ]\n"
		"\n"
		" where TSA-MAP := [ TSA-MAP ] TSA-MAPPING\n"
		"       TSA-MAPPING := { all | TC }:{ strict | cbs | ets | vendor }\n"
		"       BW-MAP := [ BW-MAP ] BW-MAPPING\n"
		"       BW-MAPPING := { all | TC }:INTEGER\n"
		"       PRIO-MAP := [ PRIO-MAP ] PRIO-MAPPING\n"
		"       PRIO-MAPPING := { all | PRIO }:TC\n"
		"       TC := { 0 .. 7 }\n"
		"       PRIO := { 0 .. 7 }\n"
		"\n"
	);
}

static void dcb_ets_help_show(void)
{
	fprintf(stderr,
		"Usage: dcb ets show dev STRING\n"
		"           [ willing ] [ ets-cap ] [ cbs ] [ tc-tsa ]\n"
		"           [ reco-tc-tsa ] [ pg-bw ] [ tc-bw ] [ reco-tc-bw ]\n"
		"           [ prio-tc ] [ reco-prio-tc ]\n"
		"\n"
	);
}

static void dcb_ets_help(void)
{
	fprintf(stderr,
		"Usage: dcb ets help\n"
		"\n"
	);
	dcb_ets_help_show();
	dcb_ets_help_set();
}

static const char *const tsa_names[] = {
	[IEEE_8021QAZ_TSA_STRICT] = "strict",
	[IEEE_8021QAZ_TSA_CB_SHAPER] = "cbs",
	[IEEE_8021QAZ_TSA_ETS] = "ets",
	[IEEE_8021QAZ_TSA_VENDOR] = "vendor",
};

static int dcb_ets_parse_mapping_tc_tsa(__u32 key, char *value, void *data)
{
	__u8 tsa;
	int ret;

	tsa = parse_one_of("TSA", value, tsa_names, ARRAY_SIZE(tsa_names), &ret);
	if (ret)
		return ret;

	return dcb_parse_mapping("TC", key, IEEE_8021QAZ_MAX_TCS - 1,
				 "TSA", tsa, -1U,
				 dcb_set_u8, data);
}

static int dcb_ets_parse_mapping_tc_bw(__u32 key, char *value, void *data)
{
	__u8 bw;

	if (get_u8(&bw, value, 0))
		return -EINVAL;

	return dcb_parse_mapping("TC", key, IEEE_8021QAZ_MAX_TCS - 1,
				 "BW", bw, 100,
				 dcb_set_u8, data);
}

static int dcb_ets_parse_mapping_prio_tc(unsigned int key, char *value, void *data)
{
	__u8 tc;

	if (get_u8(&tc, value, 0))
		return -EINVAL;

	return dcb_parse_mapping("PRIO", key, IEEE_8021QAZ_MAX_TCS - 1,
				 "TC", tc, IEEE_8021QAZ_MAX_TCS - 1,
				 dcb_set_u8, data);
}

static void dcb_print_array_tsa(const __u8 *array, size_t size)
{
	dcb_print_array_kw(array, size, tsa_names, ARRAY_SIZE(tsa_names));
}

static void dcb_ets_print_willing(const struct ieee_ets *ets)
{
	print_on_off(PRINT_ANY, "willing", "willing %s ", ets->willing);
}

static void dcb_ets_print_ets_cap(const struct ieee_ets *ets)
{
	print_uint(PRINT_ANY, "ets_cap", "ets-cap %d ", ets->ets_cap);
}

static void dcb_ets_print_cbs(const struct ieee_ets *ets)
{
	print_on_off(PRINT_ANY, "cbs", "cbs %s ", ets->cbs);
}

static void dcb_ets_print_tc_bw(const struct ieee_ets *ets)
{
	dcb_print_named_array("tc_bw", "tc-bw",
			      ets->tc_tx_bw, ARRAY_SIZE(ets->tc_tx_bw),
			      dcb_print_array_u8);
}

static void dcb_ets_print_pg_bw(const struct ieee_ets *ets)
{
	dcb_print_named_array("pg_bw", "pg-bw",
			      ets->tc_rx_bw, ARRAY_SIZE(ets->tc_rx_bw),
			      dcb_print_array_u8);
}

static void dcb_ets_print_tc_tsa(const struct ieee_ets *ets)
{
	dcb_print_named_array("tc_tsa", "tc-tsa",
			      ets->tc_tsa, ARRAY_SIZE(ets->tc_tsa),
			      dcb_print_array_tsa);
}

static void dcb_ets_print_prio_tc(const struct ieee_ets *ets)
{
	dcb_print_named_array("prio_tc", "prio-tc",
			      ets->prio_tc, ARRAY_SIZE(ets->prio_tc),
			      dcb_print_array_u8);
}

static void dcb_ets_print_reco_tc_bw(const struct ieee_ets *ets)
{
	dcb_print_named_array("reco_tc_bw", "reco-tc-bw",
			      ets->tc_reco_bw, ARRAY_SIZE(ets->tc_reco_bw),
			      dcb_print_array_u8);
}

static void dcb_ets_print_reco_tc_tsa(const struct ieee_ets *ets)
{
	dcb_print_named_array("reco_tc_tsa", "reco-tc-tsa",
			      ets->tc_reco_tsa, ARRAY_SIZE(ets->tc_reco_tsa),
			      dcb_print_array_tsa);
}

static void dcb_ets_print_reco_prio_tc(const struct ieee_ets *ets)
{
	dcb_print_named_array("reco_prio_tc", "reco-prio-tc",
			      ets->reco_prio_tc, ARRAY_SIZE(ets->reco_prio_tc),
			      dcb_print_array_u8);
}

static void dcb_ets_print(const struct ieee_ets *ets)
{
	dcb_ets_print_willing(ets);
	dcb_ets_print_ets_cap(ets);
	dcb_ets_print_cbs(ets);
	print_nl();

	dcb_ets_print_tc_bw(ets);
	print_nl();

	dcb_ets_print_pg_bw(ets);
	print_nl();

	dcb_ets_print_tc_tsa(ets);
	print_nl();

	dcb_ets_print_prio_tc(ets);
	print_nl();

	dcb_ets_print_reco_tc_bw(ets);
	print_nl();

	dcb_ets_print_reco_tc_tsa(ets);
	print_nl();

	dcb_ets_print_reco_prio_tc(ets);
	print_nl();
}

static int dcb_ets_get(struct dcb *dcb, const char *dev, struct ieee_ets *ets)
{
	return dcb_get_attribute(dcb, dev, DCB_ATTR_IEEE_ETS, ets, sizeof(*ets));
}

static int dcb_ets_validate_bw(const __u8 bw[], const __u8 tsa[], const char *what)
{
	bool has_ets = false;
	unsigned int total = 0;
	unsigned int tc;

	for (tc = 0; tc < IEEE_8021QAZ_MAX_TCS; tc++) {
		if (tsa[tc] == IEEE_8021QAZ_TSA_ETS) {
			has_ets = true;
			break;
		}
	}

	/* TC bandwidth is only intended for ETS, but 802.1Q-2018 only requires
	 * that the sum be 100, and individual entries 0..100. It explicitly
	 * notes that non-ETS TCs can have non-0 TC bandwidth during
	 * reconfiguration.
	 */
	for (tc = 0; tc < IEEE_8021QAZ_MAX_TCS; tc++) {
		if (bw[tc] > 100) {
			fprintf(stderr, "%d%% for TC %d of %s is not a valid bandwidth percentage, expected 0..100%%\n",
				bw[tc], tc, what);
			return -EINVAL;
		}
		total += bw[tc];
	}

	/* This is what 802.1Q-2018 requires. */
	if (total == 100)
		return 0;

	/* But this requirement does not make sense for all-strict
	 * configurations. Anything else than 0 does not make sense: either BW
	 * has not been reconfigured for the all-strict allocation yet, at which
	 * point we expect sum of 100. Or it has already been reconfigured, at
	 * which point accept 0.
	 */
	if (!has_ets && total == 0)
		return 0;

	fprintf(stderr, "Bandwidth percentages in %s sum to %d%%, expected %d%%\n",
		what, total, has_ets ? 100 : 0);
	return -EINVAL;
}

static int dcb_ets_set(struct dcb *dcb, const char *dev, const struct ieee_ets *ets)
{
	/* Do not validate pg-bw, which is not standard and has unclear
	 * meaning.
	 */
	if (dcb_ets_validate_bw(ets->tc_tx_bw, ets->tc_tsa, "tc-bw") ||
	    dcb_ets_validate_bw(ets->tc_reco_bw, ets->tc_reco_tsa, "reco-tc-bw"))
		return -EINVAL;

	return dcb_set_attribute(dcb, dev, DCB_ATTR_IEEE_ETS, ets, sizeof(*ets));
}

static int dcb_cmd_ets_set(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	struct ieee_ets ets;
	int ret;

	if (!argc) {
		dcb_ets_help_set();
		return 1;
	}

	ret = dcb_ets_get(dcb, dev, &ets);
	if (ret)
		return ret;

	do {
		if (matches(*argv, "help") == 0) {
			dcb_ets_help_set();
			return 0;
		} else if (matches(*argv, "willing") == 0) {
			NEXT_ARG();
			ets.willing = parse_on_off("willing", *argv, &ret);
			if (ret)
				return ret;
		} else if (matches(*argv, "tc-tsa") == 0) {
			NEXT_ARG();
			ret = parse_mapping(&argc, &argv, true, &dcb_ets_parse_mapping_tc_tsa,
					    ets.tc_tsa);
			if (ret) {
				fprintf(stderr, "Invalid tc-tsa mapping %s\n", *argv);
				return ret;
			}
			continue;
		} else if (matches(*argv, "reco-tc-tsa") == 0) {
			NEXT_ARG();
			ret = parse_mapping(&argc, &argv, true, &dcb_ets_parse_mapping_tc_tsa,
					    ets.tc_reco_tsa);
			if (ret) {
				fprintf(stderr, "Invalid reco-tc-tsa mapping %s\n", *argv);
				return ret;
			}
			continue;
		} else if (matches(*argv, "tc-bw") == 0) {
			NEXT_ARG();
			ret = parse_mapping(&argc, &argv, true, &dcb_ets_parse_mapping_tc_bw,
					    ets.tc_tx_bw);
			if (ret) {
				fprintf(stderr, "Invalid tc-bw mapping %s\n", *argv);
				return ret;
			}
			continue;
		} else if (matches(*argv, "pg-bw") == 0) {
			NEXT_ARG();
			ret = parse_mapping(&argc, &argv, true, &dcb_ets_parse_mapping_tc_bw,
					    ets.tc_rx_bw);
			if (ret) {
				fprintf(stderr, "Invalid pg-bw mapping %s\n", *argv);
				return ret;
			}
			continue;
		} else if (matches(*argv, "reco-tc-bw") == 0) {
			NEXT_ARG();
			ret = parse_mapping(&argc, &argv, true, &dcb_ets_parse_mapping_tc_bw,
					    ets.tc_reco_bw);
			if (ret) {
				fprintf(stderr, "Invalid reco-tc-bw mapping %s\n", *argv);
				return ret;
			}
			continue;
		} else if (matches(*argv, "prio-tc") == 0) {
			NEXT_ARG();
			ret = parse_mapping(&argc, &argv, true, &dcb_ets_parse_mapping_prio_tc,
					    ets.prio_tc);
			if (ret) {
				fprintf(stderr, "Invalid prio-tc mapping %s\n", *argv);
				return ret;
			}
			continue;
		} else if (matches(*argv, "reco-prio-tc") == 0) {
			NEXT_ARG();
			ret = parse_mapping(&argc, &argv, true, &dcb_ets_parse_mapping_prio_tc,
					    ets.reco_prio_tc);
			if (ret) {
				fprintf(stderr, "Invalid reco-prio-tc mapping %s\n", *argv);
				return ret;
			}
			continue;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			dcb_ets_help_set();
			return -EINVAL;
		}

		NEXT_ARG_FWD();
	} while (argc > 0);

	return dcb_ets_set(dcb, dev, &ets);
}

static int dcb_cmd_ets_show(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	struct ieee_ets ets;
	int ret;

	ret = dcb_ets_get(dcb, dev, &ets);
	if (ret)
		return ret;

	open_json_object(NULL);

	if (!argc) {
		dcb_ets_print(&ets);
		goto out;
	}

	do {
		if (matches(*argv, "help") == 0) {
			dcb_ets_help_show();
			return 0;
		} else if (matches(*argv, "willing") == 0) {
			dcb_ets_print_willing(&ets);
			print_nl();
		} else if (matches(*argv, "ets-cap") == 0) {
			dcb_ets_print_ets_cap(&ets);
			print_nl();
		} else if (matches(*argv, "cbs") == 0) {
			dcb_ets_print_cbs(&ets);
			print_nl();
		} else if (matches(*argv, "tc-tsa") == 0) {
			dcb_ets_print_tc_tsa(&ets);
			print_nl();
		} else if (matches(*argv, "reco-tc-tsa") == 0) {
			dcb_ets_print_reco_tc_tsa(&ets);
			print_nl();
		} else if (matches(*argv, "tc-bw") == 0) {
			dcb_ets_print_tc_bw(&ets);
			print_nl();
		} else if (matches(*argv, "pg-bw") == 0) {
			dcb_ets_print_pg_bw(&ets);
			print_nl();
		} else if (matches(*argv, "reco-tc-bw") == 0) {
			dcb_ets_print_reco_tc_bw(&ets);
			print_nl();
		} else if (matches(*argv, "prio-tc") == 0) {
			dcb_ets_print_prio_tc(&ets);
			print_nl();
		} else if (matches(*argv, "reco-prio-tc") == 0) {
			dcb_ets_print_reco_prio_tc(&ets);
			print_nl();
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			dcb_ets_help_show();
			return -EINVAL;
		}

		NEXT_ARG_FWD();
	} while (argc > 0);

out:
	close_json_object();
	return 0;
}

int dcb_cmd_ets(struct dcb *dcb, int argc, char **argv)
{
	if (!argc || matches(*argv, "help") == 0) {
		dcb_ets_help();
		return 0;
	} else if (matches(*argv, "show") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv, dcb_cmd_ets_show, dcb_ets_help_show);
	} else if (matches(*argv, "set") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv, dcb_cmd_ets_set, dcb_ets_help_set);
	} else {
		fprintf(stderr, "What is \"%s\"?\n", *argv);
		dcb_ets_help();
		return -EINVAL;
	}
}
