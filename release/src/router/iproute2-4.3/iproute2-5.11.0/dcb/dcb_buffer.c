// SPDX-License-Identifier: GPL-2.0+

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <linux/dcbnl.h>

#include "dcb.h"
#include "utils.h"

static void dcb_buffer_help_set(void)
{
	fprintf(stderr,
		"Usage: dcb buffer set dev STRING\n"
		"           [ prio-buffer PRIO-MAP ]\n"
		"           [ buffer-size SIZE-MAP ]\n"
		"\n"
		" where PRIO-MAP := [ PRIO-MAP ] PRIO-MAPPING\n"
		"       PRIO-MAPPING := { all | PRIO }:BUFFER\n"
		"       SIZE-MAP := [ SIZE-MAP ] SIZE-MAPPING\n"
		"       SIZE-MAPPING := { all | BUFFER }:INTEGER\n"
		"       PRIO := { 0 .. 7 }\n"
		"       BUFFER := { 0 .. 7 }\n"
		"\n"
	);
}

static void dcb_buffer_help_show(void)
{
	fprintf(stderr,
		"Usage: dcb buffer show dev STRING\n"
		"           [ prio-buffer ] [ buffer-size ] [ total-size ]\n"
		"\n"
	);
}

static void dcb_buffer_help(void)
{
	fprintf(stderr,
		"Usage: dcb buffer help\n"
		"\n"
	);
	dcb_buffer_help_show();
	dcb_buffer_help_set();
}

static int dcb_buffer_parse_mapping_prio_buffer(__u32 key, char *value, void *data)
{
	struct dcbnl_buffer *buffer = data;
	__u8 buf;

	if (get_u8(&buf, value, 0))
		return -EINVAL;

	return dcb_parse_mapping("PRIO", key, IEEE_8021Q_MAX_PRIORITIES - 1,
				 "BUFFER", buf, DCBX_MAX_BUFFERS - 1,
				 dcb_set_u8, buffer->prio2buffer);
}

static int dcb_buffer_parse_mapping_buffer_size(__u32 key, char *value, void *data)
{
	struct dcbnl_buffer *buffer = data;
	unsigned int size;

	if (get_size(&size, value)) {
		fprintf(stderr, "%d:%s: Illegal value for buffer size\n", key, value);
		return -EINVAL;
	}

	return dcb_parse_mapping("BUFFER", key, DCBX_MAX_BUFFERS - 1,
				 "INTEGER", size, -1,
				 dcb_set_u32, buffer->buffer_size);
}

static void dcb_buffer_print_total_size(const struct dcbnl_buffer *buffer)
{
	print_size(PRINT_ANY, "total_size", "total-size %s ", buffer->total_size);
}

static void dcb_buffer_print_prio_buffer(const struct dcbnl_buffer *buffer)
{
	dcb_print_named_array("prio_buffer", "prio-buffer",
			      buffer->prio2buffer, ARRAY_SIZE(buffer->prio2buffer),
			      dcb_print_array_u8);
}

static void dcb_buffer_print_buffer_size(const struct dcbnl_buffer *buffer)
{
	size_t size = ARRAY_SIZE(buffer->buffer_size);
	SPRINT_BUF(b);
	size_t i;

	open_json_array(PRINT_JSON, "buffer_size");
	print_string(PRINT_FP, NULL, "buffer-size ", NULL);

	for (i = 0; i < size; i++) {
		snprintf(b, sizeof(b), "%zd:%%s ", i);
		print_size(PRINT_ANY, NULL, b, buffer->buffer_size[i]);
	}

	close_json_array(PRINT_JSON, "buffer_size");
}

static void dcb_buffer_print(const struct dcbnl_buffer *buffer)
{
	dcb_buffer_print_prio_buffer(buffer);
	print_nl();

	dcb_buffer_print_buffer_size(buffer);
	print_nl();

	dcb_buffer_print_total_size(buffer);
	print_nl();
}

static int dcb_buffer_get(struct dcb *dcb, const char *dev, struct dcbnl_buffer *buffer)
{
	return dcb_get_attribute(dcb, dev, DCB_ATTR_DCB_BUFFER, buffer, sizeof(*buffer));
}

static int dcb_buffer_set(struct dcb *dcb, const char *dev, const struct dcbnl_buffer *buffer)
{
	return dcb_set_attribute(dcb, dev, DCB_ATTR_DCB_BUFFER, buffer, sizeof(*buffer));
}

static int dcb_cmd_buffer_set(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	struct dcbnl_buffer buffer;
	int ret;

	if (!argc) {
		dcb_buffer_help_set();
		return 0;
	}

	ret = dcb_buffer_get(dcb, dev, &buffer);
	if (ret)
		return ret;

	do {
		if (matches(*argv, "help") == 0) {
			dcb_buffer_help_set();
			return 0;
		} else if (matches(*argv, "prio-buffer") == 0) {
			NEXT_ARG();
			ret = parse_mapping(&argc, &argv, true,
					    &dcb_buffer_parse_mapping_prio_buffer, &buffer);
			if (ret) {
				fprintf(stderr, "Invalid priority mapping %s\n", *argv);
				return ret;
			}
			continue;
		} else if (matches(*argv, "buffer-size") == 0) {
			NEXT_ARG();
			ret = parse_mapping(&argc, &argv, true,
					    &dcb_buffer_parse_mapping_buffer_size, &buffer);
			if (ret) {
				fprintf(stderr, "Invalid buffer size mapping %s\n", *argv);
				return ret;
			}
			continue;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			dcb_buffer_help_set();
			return -EINVAL;
		}

		NEXT_ARG_FWD();
	} while (argc > 0);

	return dcb_buffer_set(dcb, dev, &buffer);
}

static int dcb_cmd_buffer_show(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	struct dcbnl_buffer buffer;
	int ret;

	ret = dcb_buffer_get(dcb, dev, &buffer);
	if (ret)
		return ret;

	open_json_object(NULL);

	if (!argc) {
		dcb_buffer_print(&buffer);
		goto out;
	}

	do {
		if (matches(*argv, "help") == 0) {
			dcb_buffer_help_show();
			return 0;
		} else if (matches(*argv, "prio-buffer") == 0) {
			dcb_buffer_print_prio_buffer(&buffer);
			print_nl();
		} else if (matches(*argv, "buffer-size") == 0) {
			dcb_buffer_print_buffer_size(&buffer);
			print_nl();
		} else if (matches(*argv, "total-size") == 0) {
			dcb_buffer_print_total_size(&buffer);
			print_nl();
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			dcb_buffer_help_show();
			return -EINVAL;
		}

		NEXT_ARG_FWD();
	} while (argc > 0);

out:
	close_json_object();
	return 0;
}

int dcb_cmd_buffer(struct dcb *dcb, int argc, char **argv)
{
	if (!argc || matches(*argv, "help") == 0) {
		dcb_buffer_help();
		return 0;
	} else if (matches(*argv, "show") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv,
					 dcb_cmd_buffer_show, dcb_buffer_help_show);
	} else if (matches(*argv, "set") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv,
					 dcb_cmd_buffer_set, dcb_buffer_help_set);
	} else {
		fprintf(stderr, "What is \"%s\"?\n", *argv);
		dcb_buffer_help();
		return -EINVAL;
	}
}
