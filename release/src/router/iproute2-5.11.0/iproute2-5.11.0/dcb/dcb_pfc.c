// SPDX-License-Identifier: GPL-2.0+

#include <errno.h>
#include <stdio.h>
#include <linux/dcbnl.h>

#include "dcb.h"
#include "utils.h"

static void dcb_pfc_help_set(void)
{
	fprintf(stderr,
		"Usage: dcb pfc set dev STRING\n"
		"           [ prio-pfc PFC-MAP ]\n"
		"           [ macsec-bypass { on | off } ]\n"
		"           [ delay INTEGER ]\n"
		"\n"
		" where PFC-MAP := [ PFC-MAP ] PFC-MAPPING\n"
		"       PFC-MAPPING := { all | TC }:PFC\n"
		"       TC := { 0 .. 7 }\n"
		"       PFC := { on | off }\n"
		"\n"
	);
}

static void dcb_pfc_help_show(void)
{
	fprintf(stderr,
		"Usage: dcb [ -s ] pfc show dev STRING\n"
		"           [ pfc-cap ] [ prio-pfc ] [ macsec-bypass ]\n"
		"           [ delay ] [ requests ] [ indications ]\n"
		"\n"
	);
}

static void dcb_pfc_help(void)
{
	fprintf(stderr,
		"Usage: dcb pfc help\n"
		"\n"
	);
	dcb_pfc_help_show();
	dcb_pfc_help_set();
}

static void dcb_pfc_to_array(__u8 array[IEEE_8021QAZ_MAX_TCS], __u8 pfc_en)
{
	int i;

	for (i = 0; i < IEEE_8021QAZ_MAX_TCS; i++)
		array[i] = !!(pfc_en & (1 << i));
}

static void dcb_pfc_from_array(__u8 array[IEEE_8021QAZ_MAX_TCS], __u8 *pfc_en_p)
{
	__u8 pfc_en = 0;
	int i;

	for (i = 0; i < IEEE_8021QAZ_MAX_TCS; i++) {
		if (array[i])
			pfc_en |= 1 << i;
	}

	*pfc_en_p = pfc_en;
}

static int dcb_pfc_parse_mapping_prio_pfc(__u32 key, char *value, void *data)
{
	struct ieee_pfc *pfc = data;
	__u8 pfc_en[IEEE_8021QAZ_MAX_TCS];
	bool enabled;
	int ret;

	dcb_pfc_to_array(pfc_en, pfc->pfc_en);

	enabled = parse_on_off("PFC", value, &ret);
	if (ret)
		return ret;

	ret = dcb_parse_mapping("PRIO", key, IEEE_8021QAZ_MAX_TCS - 1,
				"PFC", enabled, -1,
				dcb_set_u8, pfc_en);
	if (ret)
		return ret;

	dcb_pfc_from_array(pfc_en, &pfc->pfc_en);
	return 0;
}

static void dcb_pfc_print_pfc_cap(const struct ieee_pfc *pfc)
{
	print_uint(PRINT_ANY, "pfc_cap", "pfc-cap %d ", pfc->pfc_cap);
}

static void dcb_pfc_print_macsec_bypass(const struct ieee_pfc *pfc)
{
	print_on_off(PRINT_ANY, "macsec_bypass", "macsec-bypass %s ", pfc->mbc);
}

static void dcb_pfc_print_delay(const struct ieee_pfc *pfc)
{
	print_uint(PRINT_ANY, "delay", "delay %d ", pfc->delay);
}

static void dcb_pfc_print_prio_pfc(const struct ieee_pfc *pfc)
{
	__u8 pfc_en[IEEE_8021QAZ_MAX_TCS];

	dcb_pfc_to_array(pfc_en, pfc->pfc_en);
	dcb_print_named_array("prio_pfc", "prio-pfc",
			      pfc_en, ARRAY_SIZE(pfc_en), &dcb_print_array_on_off);
}

static void dcb_pfc_print_requests(const struct ieee_pfc *pfc)
{
	open_json_array(PRINT_JSON, "requests");
	print_string(PRINT_FP, NULL, "requests ", NULL);
	dcb_print_array_u64(pfc->requests, ARRAY_SIZE(pfc->requests));
	close_json_array(PRINT_JSON, "requests");
}

static void dcb_pfc_print_indications(const struct ieee_pfc *pfc)
{
	open_json_array(PRINT_JSON, "indications");
	print_string(PRINT_FP, NULL, "indications ", NULL);
	dcb_print_array_u64(pfc->indications, ARRAY_SIZE(pfc->indications));
	close_json_array(PRINT_JSON, "indications");
}

static void dcb_pfc_print(const struct dcb *dcb, const struct ieee_pfc *pfc)
{
	dcb_pfc_print_pfc_cap(pfc);
	dcb_pfc_print_macsec_bypass(pfc);
	dcb_pfc_print_delay(pfc);
	print_nl();

	dcb_pfc_print_prio_pfc(pfc);
	print_nl();

	if (dcb->stats) {
		dcb_pfc_print_requests(pfc);
		print_nl();

		dcb_pfc_print_indications(pfc);
		print_nl();
	}
}

static int dcb_pfc_get(struct dcb *dcb, const char *dev, struct ieee_pfc *pfc)
{
	return dcb_get_attribute(dcb, dev, DCB_ATTR_IEEE_PFC, pfc, sizeof(*pfc));
}

static int dcb_pfc_set(struct dcb *dcb, const char *dev, const struct ieee_pfc *pfc)
{
	return dcb_set_attribute(dcb, dev, DCB_ATTR_IEEE_PFC, pfc, sizeof(*pfc));
}

static int dcb_cmd_pfc_set(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	struct ieee_pfc pfc;
	int ret;

	if (!argc) {
		dcb_pfc_help_set();
		return 0;
	}

	ret = dcb_pfc_get(dcb, dev, &pfc);
	if (ret)
		return ret;

	do {
		if (matches(*argv, "help") == 0) {
			dcb_pfc_help_set();
			return 0;
		} else if (matches(*argv, "prio-pfc") == 0) {
			NEXT_ARG();
			ret = parse_mapping(&argc, &argv, true,
					    &dcb_pfc_parse_mapping_prio_pfc, &pfc);
			if (ret) {
				fprintf(stderr, "Invalid pfc mapping %s\n", *argv);
				return ret;
			}
			continue;
		} else if (matches(*argv, "macsec-bypass") == 0) {
			NEXT_ARG();
			pfc.mbc = parse_on_off("macsec-bypass", *argv, &ret);
			if (ret)
				return ret;
		} else if (matches(*argv, "delay") == 0) {
			NEXT_ARG();
			/* Do not support the size notations for delay.
			 * Delay is specified in "bit times", not bits, so
			 * it is not applicable. At the same time it would
			 * be confusing that 10Kbit does not mean 10240,
			 * but 1280.
			 */
			if (get_u16(&pfc.delay, *argv, 0)) {
				fprintf(stderr, "Invalid delay `%s', expected an integer 0..65535\n",
					*argv);
				return -EINVAL;
			}
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			dcb_pfc_help_set();
			return -EINVAL;
		}

		NEXT_ARG_FWD();
	} while (argc > 0);

	return dcb_pfc_set(dcb, dev, &pfc);
}

static int dcb_cmd_pfc_show(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	struct ieee_pfc pfc;
	int ret;

	ret = dcb_pfc_get(dcb, dev, &pfc);
	if (ret)
		return ret;

	open_json_object(NULL);

	if (!argc) {
		dcb_pfc_print(dcb, &pfc);
		goto out;
	}

	do {
		if (matches(*argv, "help") == 0) {
			dcb_pfc_help_show();
			return 0;
		} else if (matches(*argv, "prio-pfc") == 0) {
			dcb_pfc_print_prio_pfc(&pfc);
			print_nl();
		} else if (matches(*argv, "pfc-cap") == 0) {
			dcb_pfc_print_pfc_cap(&pfc);
			print_nl();
		} else if (matches(*argv, "macsec-bypass") == 0) {
			dcb_pfc_print_macsec_bypass(&pfc);
			print_nl();
		} else if (matches(*argv, "delay") == 0) {
			dcb_pfc_print_delay(&pfc);
			print_nl();
		} else if (matches(*argv, "requests") == 0) {
			dcb_pfc_print_requests(&pfc);
			print_nl();
		} else if (matches(*argv, "indications") == 0) {
			dcb_pfc_print_indications(&pfc);
			print_nl();
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			dcb_pfc_help_show();
			return -EINVAL;
		}

		NEXT_ARG_FWD();
	} while (argc > 0);

out:
	close_json_object();
	return 0;
}

int dcb_cmd_pfc(struct dcb *dcb, int argc, char **argv)
{
	if (!argc || matches(*argv, "help") == 0) {
		dcb_pfc_help();
		return 0;
	} else if (matches(*argv, "show") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv,
					 dcb_cmd_pfc_show, dcb_pfc_help_show);
	} else if (matches(*argv, "set") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv,
					 dcb_cmd_pfc_set, dcb_pfc_help_set);
	} else {
		fprintf(stderr, "What is \"%s\"?\n", *argv);
		dcb_pfc_help();
		return -EINVAL;
	}
}
