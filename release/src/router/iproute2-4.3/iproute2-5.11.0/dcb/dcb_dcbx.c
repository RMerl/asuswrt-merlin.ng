// SPDX-License-Identifier: GPL-2.0+

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <linux/dcbnl.h>

#include "dcb.h"
#include "utils.h"

static void dcb_dcbx_help_set(void)
{
	fprintf(stderr,
		"Usage: dcb dcbx set dev STRING\n"
		"           [ host | lld-managed ]\n"
		"           [ cee | ieee ] [ static ]\n"
		"\n"
	);
}

static void dcb_dcbx_help_show(void)
{
	fprintf(stderr,
		"Usage: dcb dcbx show dev STRING\n"
		"\n"
	);
}

static void dcb_dcbx_help(void)
{
	fprintf(stderr,
		"Usage: dcb dcbx help\n"
		"\n"
	);
	dcb_dcbx_help_show();
	dcb_dcbx_help_set();
}

struct dcb_dcbx_flag {
	__u8 value;
	const char *key_fp;
	const char *key_json;
};

static struct dcb_dcbx_flag dcb_dcbx_flags[] = {
	{DCB_CAP_DCBX_HOST, "host"},
	{DCB_CAP_DCBX_LLD_MANAGED, "lld-managed", "lld_managed"},
	{DCB_CAP_DCBX_VER_CEE, "cee"},
	{DCB_CAP_DCBX_VER_IEEE, "ieee"},
	{DCB_CAP_DCBX_STATIC, "static"},
};

static void dcb_dcbx_print(__u8 dcbx)
{
	int bit;
	int i;

	while ((bit = ffs(dcbx))) {
		bool found = false;

		bit--;
		for (i = 0; i < ARRAY_SIZE(dcb_dcbx_flags); i++) {
			struct dcb_dcbx_flag *flag = &dcb_dcbx_flags[i];

			if (flag->value == 1 << bit) {
				print_bool(PRINT_JSON, flag->key_json ?: flag->key_fp,
					   NULL, true);
				print_string(PRINT_FP, NULL, "%s ", flag->key_fp);
				found = true;
				break;
			}
		}

		if (!found)
			fprintf(stderr, "Unknown DCBX bit %#x.\n", 1 << bit);

		dcbx &= ~(1 << bit);
	}

	print_nl();
}

static int dcb_dcbx_get(struct dcb *dcb, const char *dev, __u8 *dcbx)
{
	__u16 payload_len;
	void *payload;
	int err;

	err = dcb_get_attribute_bare(dcb, DCB_CMD_IEEE_GET, dev, DCB_ATTR_DCBX,
				     &payload, &payload_len);
	if (err != 0)
		return err;

	if (payload_len != 1) {
		fprintf(stderr, "DCB_ATTR_DCBX payload has size %d, expected 1.\n",
			payload_len);
		return -EINVAL;
	}
	*dcbx = *(__u8 *) payload;
	return 0;
}

static int dcb_dcbx_set(struct dcb *dcb, const char *dev, __u8 dcbx)
{
	return dcb_set_attribute_bare(dcb, DCB_CMD_SDCBX, dev, DCB_ATTR_DCBX,
				      &dcbx, 1, DCB_ATTR_DCBX);
}

static int dcb_cmd_dcbx_set(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	__u8 dcbx = 0;
	__u8 i;

	if (!argc) {
		dcb_dcbx_help_set();
		return 0;
	}

	do {
		if (matches(*argv, "help") == 0) {
			dcb_dcbx_help_set();
			return 0;
		}

		for (i = 0; i < ARRAY_SIZE(dcb_dcbx_flags); i++) {
			struct dcb_dcbx_flag *flag = &dcb_dcbx_flags[i];

			if (matches(*argv, flag->key_fp) == 0) {
				dcbx |= flag->value;
				NEXT_ARG_FWD();
				goto next;
			}
		}

		fprintf(stderr, "What is \"%s\"?\n", *argv);
		dcb_dcbx_help_set();
		return -EINVAL;

next:
		;
	} while (argc > 0);

	return dcb_dcbx_set(dcb, dev, dcbx);
}

static int dcb_cmd_dcbx_show(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	__u8 dcbx;
	int ret;

	ret = dcb_dcbx_get(dcb, dev, &dcbx);
	if (ret != 0)
		return ret;

	while (argc > 0) {
		if (matches(*argv, "help") == 0) {
			dcb_dcbx_help_show();
			return 0;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			dcb_dcbx_help_show();
			return -EINVAL;
		}

		NEXT_ARG_FWD();
	}

	open_json_object(NULL);
	dcb_dcbx_print(dcbx);
	close_json_object();
	return 0;
}

int dcb_cmd_dcbx(struct dcb *dcb, int argc, char **argv)
{
	if (!argc || matches(*argv, "help") == 0) {
		dcb_dcbx_help();
		return 0;
	} else if (matches(*argv, "show") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv,
					 dcb_cmd_dcbx_show, dcb_dcbx_help_show);
	} else if (matches(*argv, "set") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv,
					 dcb_cmd_dcbx_set, dcb_dcbx_help_set);
	} else {
		fprintf(stderr, "What is \"%s\"?\n", *argv);
		dcb_dcbx_help();
		return -EINVAL;
	}
}
