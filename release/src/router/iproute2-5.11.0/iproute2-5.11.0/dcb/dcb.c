// SPDX-License-Identifier: GPL-2.0+

#include <inttypes.h>
#include <stdio.h>
#include <linux/dcbnl.h>
#include <libmnl/libmnl.h>
#include <getopt.h>

#include "dcb.h"
#include "mnl_utils.h"
#include "namespace.h"
#include "utils.h"
#include "version.h"

static int dcb_init(struct dcb *dcb)
{
	dcb->buf = malloc(MNL_SOCKET_BUFFER_SIZE);
	if (dcb->buf == NULL) {
		perror("Netlink buffer allocation");
		return -1;
	}

	dcb->nl = mnlu_socket_open(NETLINK_ROUTE);
	if (dcb->nl == NULL) {
		perror("Open netlink socket");
		goto err_socket_open;
	}

	new_json_obj_plain(dcb->json_output);
	return 0;

err_socket_open:
	free(dcb->buf);
	return -1;
}

static void dcb_fini(struct dcb *dcb)
{
	delete_json_obj_plain();
	mnl_socket_close(dcb->nl);
	free(dcb->buf);
}

static struct dcb *dcb_alloc(void)
{
	struct dcb *dcb;

	dcb = calloc(1, sizeof(*dcb));
	if (!dcb)
		return NULL;
	return dcb;
}

static void dcb_free(struct dcb *dcb)
{
	free(dcb);
}

struct dcb_get_attribute {
	struct dcb *dcb;
	int attr;
	void *payload;
	__u16 payload_len;
};

static int dcb_get_attribute_attr_ieee_cb(const struct nlattr *attr, void *data)
{
	struct dcb_get_attribute *ga = data;

	if (mnl_attr_get_type(attr) != ga->attr)
		return MNL_CB_OK;

	ga->payload = mnl_attr_get_payload(attr);
	ga->payload_len = mnl_attr_get_payload_len(attr);
	return MNL_CB_STOP;
}

static int dcb_get_attribute_attr_cb(const struct nlattr *attr, void *data)
{
	if (mnl_attr_get_type(attr) != DCB_ATTR_IEEE)
		return MNL_CB_OK;

	return mnl_attr_parse_nested(attr, dcb_get_attribute_attr_ieee_cb, data);
}

static int dcb_get_attribute_cb(const struct nlmsghdr *nlh, void *data)
{
	return mnl_attr_parse(nlh, sizeof(struct dcbmsg), dcb_get_attribute_attr_cb, data);
}

static int dcb_get_attribute_bare_cb(const struct nlmsghdr *nlh, void *data)
{
	/* Bare attributes (e.g. DCB_ATTR_DCBX) are not wrapped inside an IEEE
	 * container, so this does not have to go through unpacking in
	 * dcb_get_attribute_attr_cb().
	 */
	return mnl_attr_parse(nlh, sizeof(struct dcbmsg),
			      dcb_get_attribute_attr_ieee_cb, data);
}

struct dcb_set_attribute_response {
	int response_attr;
};

static int dcb_set_attribute_attr_cb(const struct nlattr *attr, void *data)
{
	struct dcb_set_attribute_response *resp = data;
	uint16_t len;
	uint8_t err;

	if (mnl_attr_get_type(attr) != resp->response_attr)
		return MNL_CB_OK;

	len = mnl_attr_get_payload_len(attr);
	if (len != 1) {
		fprintf(stderr, "Response attribute expected to have size 1, not %d\n", len);
		return MNL_CB_ERROR;
	}

	err = mnl_attr_get_u8(attr);
	if (err) {
		fprintf(stderr, "Error when attempting to set attribute: %s\n",
			strerror(err));
		return MNL_CB_ERROR;
	}

	return MNL_CB_STOP;
}

static int dcb_set_attribute_cb(const struct nlmsghdr *nlh, void *data)
{
	return mnl_attr_parse(nlh, sizeof(struct dcbmsg), dcb_set_attribute_attr_cb, data);
}

static int dcb_talk(struct dcb *dcb, struct nlmsghdr *nlh, mnl_cb_t cb, void *data)
{
	int ret;

	ret = mnl_socket_sendto(dcb->nl, nlh, nlh->nlmsg_len);
	if (ret < 0) {
		perror("mnl_socket_sendto");
		return -1;
	}

	return mnlu_socket_recv_run(dcb->nl, nlh->nlmsg_seq, dcb->buf, MNL_SOCKET_BUFFER_SIZE,
				    cb, data);
}

static struct nlmsghdr *dcb_prepare(struct dcb *dcb, const char *dev,
				    uint32_t nlmsg_type, uint8_t dcb_cmd)
{
	struct dcbmsg dcbm = {
		.cmd = dcb_cmd,
	};
	struct nlmsghdr *nlh;

	nlh = mnlu_msg_prepare(dcb->buf, nlmsg_type, NLM_F_REQUEST, &dcbm, sizeof(dcbm));
	mnl_attr_put_strz(nlh, DCB_ATTR_IFNAME, dev);
	return nlh;
}

static int __dcb_get_attribute(struct dcb *dcb, int command,
			       const char *dev, int attr,
			       void **payload_p, __u16 *payload_len_p,
			       int (*get_attribute_cb)(const struct nlmsghdr *nlh,
						       void *data))
{
	struct dcb_get_attribute ga;
	struct nlmsghdr *nlh;
	int ret;

	nlh = dcb_prepare(dcb, dev, RTM_GETDCB, command);

	ga = (struct dcb_get_attribute) {
		.dcb = dcb,
		.attr = attr,
		.payload = NULL,
	};
	ret = dcb_talk(dcb, nlh, get_attribute_cb, &ga);
	if (ret) {
		perror("Attribute read");
		return ret;
	}
	if (ga.payload == NULL) {
		perror("Attribute not found");
		return -ENOENT;
	}

	*payload_p = ga.payload;
	*payload_len_p = ga.payload_len;
	return 0;
}

int dcb_get_attribute_va(struct dcb *dcb, const char *dev, int attr,
			 void **payload_p, __u16 *payload_len_p)
{
	return __dcb_get_attribute(dcb, DCB_CMD_IEEE_GET, dev, attr,
				   payload_p, payload_len_p,
				   dcb_get_attribute_cb);
}

int dcb_get_attribute_bare(struct dcb *dcb, int cmd, const char *dev, int attr,
			   void **payload_p, __u16 *payload_len_p)
{
	return __dcb_get_attribute(dcb, cmd, dev, attr,
				   payload_p, payload_len_p,
				   dcb_get_attribute_bare_cb);
}

int dcb_get_attribute(struct dcb *dcb, const char *dev, int attr, void *data, size_t data_len)
{
	__u16 payload_len;
	void *payload;
	int ret;

	ret = dcb_get_attribute_va(dcb, dev, attr, &payload, &payload_len);
	if (ret)
		return ret;

	if (payload_len != data_len) {
		fprintf(stderr, "Wrong len %d, expected %zd\n", payload_len, data_len);
		return -EINVAL;
	}

	memcpy(data, payload, data_len);
	return 0;
}

static int __dcb_set_attribute(struct dcb *dcb, int command, const char *dev,
			       int (*cb)(struct dcb *, struct nlmsghdr *, void *),
			       void *data, int response_attr)
{
	struct dcb_set_attribute_response resp = {
		.response_attr = response_attr,
	};
	struct nlmsghdr *nlh;
	int ret;

	nlh = dcb_prepare(dcb, dev, RTM_SETDCB, command);

	ret = cb(dcb, nlh, data);
	if (ret)
		return ret;

	ret = dcb_talk(dcb, nlh, dcb_set_attribute_cb, &resp);
	if (ret) {
		perror("Attribute write");
		return ret;
	}
	return 0;
}

struct dcb_set_attribute_ieee_cb {
	int (*cb)(struct dcb *dcb, struct nlmsghdr *nlh, void *data);
	void *data;
};

static int dcb_set_attribute_ieee_cb(struct dcb *dcb, struct nlmsghdr *nlh, void *data)
{
	struct dcb_set_attribute_ieee_cb *ieee_data = data;
	struct nlattr *nest;
	int ret;

	nest = mnl_attr_nest_start(nlh, DCB_ATTR_IEEE);
	ret = ieee_data->cb(dcb, nlh, ieee_data->data);
	if (ret)
		return ret;
	mnl_attr_nest_end(nlh, nest);

	return 0;
}

int dcb_set_attribute_va(struct dcb *dcb, int command, const char *dev,
			 int (*cb)(struct dcb *dcb, struct nlmsghdr *nlh, void *data),
			 void *data)
{
	struct dcb_set_attribute_ieee_cb ieee_data = {
		.cb = cb,
		.data = data,
	};

	return __dcb_set_attribute(dcb, command, dev,
				   &dcb_set_attribute_ieee_cb, &ieee_data,
				   DCB_ATTR_IEEE);
}

struct dcb_set_attribute {
	int attr;
	const void *data;
	size_t data_len;
};

static int dcb_set_attribute_put(struct dcb *dcb, struct nlmsghdr *nlh, void *data)
{
	struct dcb_set_attribute *dsa = data;

	mnl_attr_put(nlh, dsa->attr, dsa->data_len, dsa->data);
	return 0;
}

int dcb_set_attribute(struct dcb *dcb, const char *dev, int attr, const void *data, size_t data_len)
{
	struct dcb_set_attribute dsa = {
		.attr = attr,
		.data = data,
		.data_len = data_len,
	};

	return dcb_set_attribute_va(dcb, DCB_CMD_IEEE_SET, dev,
				    &dcb_set_attribute_put, &dsa);
}

int dcb_set_attribute_bare(struct dcb *dcb, int command, const char *dev,
			   int attr, const void *data, size_t data_len,
			   int response_attr)
{
	struct dcb_set_attribute dsa = {
		.attr = attr,
		.data = data,
		.data_len = data_len,
	};

	return __dcb_set_attribute(dcb, command, dev,
				   &dcb_set_attribute_put, &dsa, response_attr);
}

void dcb_print_array_u8(const __u8 *array, size_t size)
{
	SPRINT_BUF(b);
	size_t i;

	for (i = 0; i < size; i++) {
		snprintf(b, sizeof(b), "%zd:%%d ", i);
		print_uint(PRINT_ANY, NULL, b, array[i]);
	}
}

void dcb_print_array_u64(const __u64 *array, size_t size)
{
	SPRINT_BUF(b);
	size_t i;

	for (i = 0; i < size; i++) {
		snprintf(b, sizeof(b), "%zd:%%" PRIu64 " ", i);
		print_u64(PRINT_ANY, NULL, b, array[i]);
	}
}

void dcb_print_array_on_off(const __u8 *array, size_t size)
{
	SPRINT_BUF(b);
	size_t i;

	for (i = 0; i < size; i++) {
		snprintf(b, sizeof(b), "%zd:%%s ", i);
		print_on_off(PRINT_ANY, NULL, b, array[i]);
	}
}

void dcb_print_array_kw(const __u8 *array, size_t array_size,
			const char *const kw[], size_t kw_size)
{
	SPRINT_BUF(b);
	size_t i;

	for (i = 0; i < array_size; i++) {
		__u8 emt = array[i];

		snprintf(b, sizeof(b), "%zd:%%s ", i);
		if (emt < kw_size && kw[emt])
			print_string(PRINT_ANY, NULL, b, kw[emt]);
		else
			print_string(PRINT_ANY, NULL, b, "???");
	}
}

void dcb_print_named_array(const char *json_name, const char *fp_name,
			   const __u8 *array, size_t size,
			   void (*print_array)(const __u8 *, size_t))
{
	open_json_array(PRINT_JSON, json_name);
	print_string(PRINT_FP, NULL, "%s ", fp_name);
	print_array(array, size);
	close_json_array(PRINT_JSON, json_name);
}

int dcb_parse_mapping(const char *what_key, __u32 key, __u32 max_key,
		      const char *what_value, __u64 value, __u64 max_value,
		      void (*set_array)(__u32 index, __u64 value, void *data),
		      void *set_array_data)
{
	bool is_all = key == (__u32) -1;

	if (!is_all && key > max_key) {
		fprintf(stderr, "In %s:%s mapping, %s is expected to be 0..%d\n",
			what_key, what_value, what_key, max_key);
		return -EINVAL;
	}

	if (value > max_value) {
		fprintf(stderr, "In %s:%s mapping, %s is expected to be 0..%llu\n",
			what_key, what_value, what_value, max_value);
		return -EINVAL;
	}

	if (is_all) {
		for (key = 0; key <= max_key; key++)
			set_array(key, value, set_array_data);
	} else {
		set_array(key, value, set_array_data);
	}

	return 0;
}

void dcb_set_u8(__u32 key, __u64 value, void *data)
{
	__u8 *array = data;

	array[key] = value;
}

void dcb_set_u32(__u32 key, __u64 value, void *data)
{
	__u32 *array = data;

	array[key] = value;
}

void dcb_set_u64(__u32 key, __u64 value, void *data)
{
	__u64 *array = data;

	array[key] = value;
}

int dcb_cmd_parse_dev(struct dcb *dcb, int argc, char **argv,
		      int (*and_then)(struct dcb *dcb, const char *dev,
				      int argc, char **argv),
		      void (*help)(void))
{
	const char *dev;

	if (!argc || matches(*argv, "help") == 0) {
		help();
		return 0;
	} else if (matches(*argv, "dev") == 0) {
		NEXT_ARG();
		dev = *argv;
		if (check_ifname(dev)) {
			invarg("not a valid ifname", *argv);
			return -EINVAL;
		}
		NEXT_ARG_FWD();
		return and_then(dcb, dev, argc, argv);
	} else {
		fprintf(stderr, "Expected `dev DEV', not `%s'", *argv);
		help();
		return -EINVAL;
	}
}

static void dcb_help(void)
{
	fprintf(stderr,
		"Usage: dcb [ OPTIONS ] OBJECT { COMMAND | help }\n"
		"       dcb [ -f | --force ] { -b | --batch } filename [ -n | --netns ] netnsname\n"
		"where  OBJECT := { app | buffer | dcbx | ets | maxrate | pfc }\n"
		"       OPTIONS := [ -V | --Version | -i | --iec | -j | --json\n"
		"                  | -N | --Numeric | -p | --pretty\n"
		"                  | -s | --statistics | -v | --verbose]\n");
}

static int dcb_cmd(struct dcb *dcb, int argc, char **argv)
{
	if (!argc || matches(*argv, "help") == 0) {
		dcb_help();
		return 0;
	} else if (matches(*argv, "app") == 0) {
		return dcb_cmd_app(dcb, argc - 1, argv + 1);
	} else if (matches(*argv, "buffer") == 0) {
		return dcb_cmd_buffer(dcb, argc - 1, argv + 1);
	} else if (matches(*argv, "dcbx") == 0) {
		return dcb_cmd_dcbx(dcb, argc - 1, argv + 1);
	} else if (matches(*argv, "ets") == 0) {
		return dcb_cmd_ets(dcb, argc - 1, argv + 1);
	} else if (matches(*argv, "maxrate") == 0) {
		return dcb_cmd_maxrate(dcb, argc - 1, argv + 1);
	} else if (matches(*argv, "pfc") == 0) {
		return dcb_cmd_pfc(dcb, argc - 1, argv + 1);
	}

	fprintf(stderr, "Object \"%s\" is unknown\n", *argv);
	return -ENOENT;
}

static int dcb_batch_cmd(int argc, char *argv[], void *data)
{
	struct dcb *dcb = data;

	return dcb_cmd(dcb, argc, argv);
}

static int dcb_batch(struct dcb *dcb, const char *name, bool force)
{
	return do_batch(name, force, dcb_batch_cmd, dcb);
}

int main(int argc, char **argv)
{
	static const struct option long_options[] = {
		{ "Version",		no_argument,		NULL, 'V' },
		{ "force",		no_argument,		NULL, 'f' },
		{ "batch",		required_argument,	NULL, 'b' },
		{ "iec",		no_argument,		NULL, 'i' },
		{ "json",		no_argument,		NULL, 'j' },
		{ "Numeric",		no_argument,		NULL, 'N' },
		{ "pretty",		no_argument,		NULL, 'p' },
		{ "statistics",		no_argument,		NULL, 's' },
		{ "netns",		required_argument,	NULL, 'n' },
		{ "help",		no_argument,		NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};
	const char *batch_file = NULL;
	bool force = false;
	struct dcb *dcb;
	int opt;
	int err;
	int ret;

	dcb = dcb_alloc();
	if (!dcb) {
		fprintf(stderr, "Failed to allocate memory for dcb\n");
		return EXIT_FAILURE;
	}

	while ((opt = getopt_long(argc, argv, "b:fhijn:psvNV",
				  long_options, NULL)) >= 0) {

		switch (opt) {
		case 'V':
			printf("dcb utility, iproute2-%s\n", version);
			ret = EXIT_SUCCESS;
			goto dcb_free;
		case 'f':
			force = true;
			break;
		case 'b':
			batch_file = optarg;
			break;
		case 'j':
			dcb->json_output = true;
			break;
		case 'N':
			dcb->numeric = true;
			break;
		case 'p':
			pretty = true;
			break;
		case 's':
			dcb->stats = true;
			break;
		case 'n':
			if (netns_switch(optarg)) {
				ret = EXIT_FAILURE;
				goto dcb_free;
			}
			break;
		case 'i':
			dcb->use_iec = true;
			break;
		case 'h':
			dcb_help();
			return 0;
		default:
			fprintf(stderr, "Unknown option.\n");
			dcb_help();
			ret = EXIT_FAILURE;
			goto dcb_free;
		}
	}

	argc -= optind;
	argv += optind;

	err = dcb_init(dcb);
	if (err) {
		ret = EXIT_FAILURE;
		goto dcb_free;
	}

	if (batch_file)
		err = dcb_batch(dcb, batch_file, force);
	else
		err = dcb_cmd(dcb, argc, argv);

	if (err) {
		ret = EXIT_FAILURE;
		goto dcb_fini;
	}

	ret = EXIT_SUCCESS;

dcb_fini:
	dcb_fini(dcb);
dcb_free:
	dcb_free(dcb);

	return ret;
}
