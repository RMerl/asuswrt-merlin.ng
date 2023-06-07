// SPDX-License-Identifier: GPL-2.0+

#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <linux/genetlink.h>
#include <linux/vdpa.h>
#include <linux/virtio_ids.h>
#include <linux/netlink.h>
#include <libmnl/libmnl.h>
#include "mnl_utils.h"

#include "version.h"
#include "json_print.h"
#include "utils.h"

#define VDPA_OPT_MGMTDEV_HANDLE		BIT(0)
#define VDPA_OPT_VDEV_MGMTDEV_HANDLE	BIT(1)
#define VDPA_OPT_VDEV_NAME		BIT(2)
#define VDPA_OPT_VDEV_HANDLE		BIT(3)

struct vdpa_opts {
	uint64_t present; /* flags of present items */
	char *mdev_bus_name;
	char *mdev_name;
	const char *vdev_name;
	unsigned int device_id;
};

struct vdpa {
	struct mnlu_gen_socket nlg;
	struct vdpa_opts opts;
	bool json_output;
	struct indent_mem *indent;
};

static void pr_out_section_start(struct vdpa *vdpa, const char *name)
{
	open_json_object(NULL);
	open_json_object(name);
}

static void pr_out_section_end(struct vdpa *vdpa)
{
	close_json_object();
	close_json_object();
}

static void pr_out_array_start(struct vdpa *vdpa, const char *name)
{
	if (!vdpa->json_output) {
		print_nl();
		inc_indent(vdpa->indent);
		print_indent(vdpa->indent);
	}
	open_json_array(PRINT_ANY, name);
}

static void pr_out_array_end(struct vdpa *vdpa)
{
	close_json_array(PRINT_JSON, NULL);
	if (!vdpa->json_output)
		dec_indent(vdpa->indent);
}

static const enum mnl_attr_data_type vdpa_policy[VDPA_ATTR_MAX + 1] = {
	[VDPA_ATTR_MGMTDEV_BUS_NAME] = MNL_TYPE_NUL_STRING,
	[VDPA_ATTR_MGMTDEV_DEV_NAME] = MNL_TYPE_NUL_STRING,
	[VDPA_ATTR_DEV_NAME] = MNL_TYPE_STRING,
	[VDPA_ATTR_DEV_ID] = MNL_TYPE_U32,
	[VDPA_ATTR_DEV_VENDOR_ID] = MNL_TYPE_U32,
	[VDPA_ATTR_DEV_MAX_VQS] = MNL_TYPE_U32,
	[VDPA_ATTR_DEV_MAX_VQ_SIZE] = MNL_TYPE_U16,
};

static int attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type;

	if (mnl_attr_type_valid(attr, VDPA_ATTR_MAX) < 0)
		return MNL_CB_OK;

	type = mnl_attr_get_type(attr);
	if (mnl_attr_validate(attr, vdpa_policy[type]) < 0)
		return MNL_CB_ERROR;

	tb[type] = attr;
	return MNL_CB_OK;
}

static int vdpa_argv_handle(struct vdpa *vdpa, int argc, char **argv,
			    char **p_mdev_bus_name,
			    char **p_mdev_name)
{
	unsigned int slashcount;
	char *str;

	if (argc <= 0 || *argv == NULL) {
		fprintf(stderr,
			"vdpa identification (\"mgmtdev_bus_name/mgmtdev_name\") expected\n");
		return -EINVAL;
	}
	str = *argv;
	slashcount = get_str_char_count(str, '/');
	if (slashcount > 1) {
		fprintf(stderr,
			"Wrong vdpa mgmtdev identification string format\n");
		fprintf(stderr, "Expected \"mgmtdev_bus_name/mgmtdev_name\"\n");
		fprintf(stderr, "Expected \"mgmtdev_name\"\n");
		return -EINVAL;
	}
	switch (slashcount) {
	case 0:
		*p_mdev_bus_name = NULL;
		*p_mdev_name = str;
		return 0;
	case 1:
		str_split_by_char(str, p_mdev_bus_name, p_mdev_name, '/');
		return 0;
	default:
		return -EINVAL;
	}
}

static int vdpa_argv_str(struct vdpa *vdpa, int argc, char **argv,
			 const char **p_str)
{
	if (argc <= 0 || *argv == NULL) {
		fprintf(stderr, "String parameter expected\n");
		return -EINVAL;
	}
	*p_str = *argv;
	return 0;
}

struct vdpa_args_metadata {
	uint64_t o_flag;
	const char *err_msg;
};

static const struct vdpa_args_metadata vdpa_args_required[] = {
	{VDPA_OPT_VDEV_MGMTDEV_HANDLE, "management device handle not set."},
	{VDPA_OPT_VDEV_NAME, "device name is not set."},
	{VDPA_OPT_VDEV_HANDLE, "device name is not set."},
};

static int vdpa_args_finding_required_validate(uint64_t o_required,
					       uint64_t o_found)
{
	uint64_t o_flag;
	int i;

	for (i = 0; i < ARRAY_SIZE(vdpa_args_required); i++) {
		o_flag = vdpa_args_required[i].o_flag;
		if ((o_required & o_flag) && !(o_found & o_flag)) {
			fprintf(stderr, "%s\n", vdpa_args_required[i].err_msg);
			return -EINVAL;
		}
	}
	if (o_required & ~o_found) {
		fprintf(stderr,
			"BUG: unknown argument required but not found\n");
		return -EINVAL;
	}
	return 0;
}

static void vdpa_opts_put(struct nlmsghdr *nlh, struct vdpa *vdpa)
{
	struct vdpa_opts *opts = &vdpa->opts;

	if ((opts->present & VDPA_OPT_MGMTDEV_HANDLE) ||
	    (opts->present & VDPA_OPT_VDEV_MGMTDEV_HANDLE)) {
		if (opts->mdev_bus_name)
			mnl_attr_put_strz(nlh, VDPA_ATTR_MGMTDEV_BUS_NAME,
					  opts->mdev_bus_name);
		mnl_attr_put_strz(nlh, VDPA_ATTR_MGMTDEV_DEV_NAME,
				  opts->mdev_name);
	}
	if ((opts->present & VDPA_OPT_VDEV_NAME) ||
	    (opts->present & VDPA_OPT_VDEV_HANDLE))
		mnl_attr_put_strz(nlh, VDPA_ATTR_DEV_NAME, opts->vdev_name);
}

static int vdpa_argv_parse(struct vdpa *vdpa, int argc, char **argv,
			   uint64_t o_required)
{
	struct vdpa_opts *opts = &vdpa->opts;
	uint64_t o_all = o_required;
	uint64_t o_found = 0;
	int err;

	if (o_required & VDPA_OPT_MGMTDEV_HANDLE) {
		err = vdpa_argv_handle(vdpa, argc, argv, &opts->mdev_bus_name,
				       &opts->mdev_name);
		if (err)
			return err;

		NEXT_ARG_FWD();
		o_found |= VDPA_OPT_MGMTDEV_HANDLE;
	} else if (o_required & VDPA_OPT_VDEV_HANDLE) {
		err = vdpa_argv_str(vdpa, argc, argv, &opts->vdev_name);
		if (err)
			return err;

		NEXT_ARG_FWD();
		o_found |= VDPA_OPT_VDEV_HANDLE;
	}

	while (NEXT_ARG_OK()) {
		if ((matches(*argv, "name") == 0) &&
		    (o_all & VDPA_OPT_VDEV_NAME)) {
			const char *namestr;

			NEXT_ARG_FWD();
			err = vdpa_argv_str(vdpa, argc, argv, &namestr);
			if (err)
				return err;
			opts->vdev_name = namestr;
			NEXT_ARG_FWD();
			o_found |= VDPA_OPT_VDEV_NAME;
		} else if ((matches(*argv, "mgmtdev")  == 0) &&
			   (o_all & VDPA_OPT_VDEV_MGMTDEV_HANDLE)) {
			NEXT_ARG_FWD();
			err = vdpa_argv_handle(vdpa, argc, argv,
					       &opts->mdev_bus_name,
					       &opts->mdev_name);
			if (err)
				return err;

			NEXT_ARG_FWD();
			o_found |= VDPA_OPT_VDEV_MGMTDEV_HANDLE;
		} else {
			fprintf(stderr, "Unknown option \"%s\"\n", *argv);
			return -EINVAL;
		}
	}

	opts->present = o_found;

	return vdpa_args_finding_required_validate(o_required, o_found);
}

static int vdpa_argv_parse_put(struct nlmsghdr *nlh, struct vdpa *vdpa,
			       int argc, char **argv,
			       uint64_t o_required)
{
	int err;

	err = vdpa_argv_parse(vdpa, argc, argv, o_required);
	if (err)
		return err;
	vdpa_opts_put(nlh, vdpa);
	return 0;
}

static void cmd_mgmtdev_help(void)
{
	fprintf(stderr, "Usage: vdpa mgmtdev show [ DEV ]\n");
}

static void pr_out_handle_start(struct vdpa *vdpa, struct nlattr **tb)
{
	const char *mdev_bus_name = NULL;
	const char *mdev_name;
	SPRINT_BUF(buf);

	mdev_name = mnl_attr_get_str(tb[VDPA_ATTR_MGMTDEV_DEV_NAME]);
	if (tb[VDPA_ATTR_MGMTDEV_BUS_NAME]) {
		mdev_bus_name = mnl_attr_get_str(tb[VDPA_ATTR_MGMTDEV_BUS_NAME]);
		sprintf(buf, "%s/%s", mdev_bus_name, mdev_name);
	} else {
		sprintf(buf, "%s", mdev_name);
	}

	if (vdpa->json_output)
		open_json_object(buf);
	else
		printf("%s: ", buf);
}

static void pr_out_handle_end(struct vdpa *vdpa)
{
	if (vdpa->json_output)
		close_json_object();
	else
		print_nl();
}

static void __pr_out_vdev_handle_start(struct vdpa *vdpa, const char *vdev_name)
{
	SPRINT_BUF(buf);

	sprintf(buf, "%s", vdev_name);
	if (vdpa->json_output)
		open_json_object(buf);
	else
		printf("%s: ", buf);
}

static void pr_out_vdev_handle_start(struct vdpa *vdpa, struct nlattr **tb)
{
	const char *vdev_name;

	vdev_name = mnl_attr_get_str(tb[VDPA_ATTR_DEV_NAME]);
	__pr_out_vdev_handle_start(vdpa, vdev_name);
}

static void pr_out_vdev_handle_end(struct vdpa *vdpa)
{
	if (vdpa->json_output)
		close_json_object();
	else
		print_nl();
}

static struct str_num_map class_map[] = {
	{ .str = "net", .num = VIRTIO_ID_NET },
	{ .str = "block", .num = VIRTIO_ID_BLOCK },
	{ .str = NULL, },
};

static const char *parse_class(int num)
{
	const char *class;

	class = str_map_lookup_uint(class_map, num);
	return class ? class : "< unknown class >";
}

static void pr_out_mgmtdev_show(struct vdpa *vdpa, const struct nlmsghdr *nlh,
				struct nlattr **tb)
{
	const char *class;
	unsigned int i;

	pr_out_handle_start(vdpa, tb);

	if (tb[VDPA_ATTR_MGMTDEV_SUPPORTED_CLASSES]) {
		uint64_t classes = mnl_attr_get_u64(tb[VDPA_ATTR_MGMTDEV_SUPPORTED_CLASSES]);

		pr_out_array_start(vdpa, "supported_classes");

		for (i = 1; i < 64; i++) {
			if ((classes & (1ULL << i)) == 0)
				continue;

			class = parse_class(i);
			print_string(PRINT_ANY, NULL, " %s", class);
		}
		pr_out_array_end(vdpa);
	}

	pr_out_handle_end(vdpa);
}

static int cmd_mgmtdev_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[VDPA_ATTR_MAX + 1] = {};
	struct vdpa *vdpa = data;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);

	if (!tb[VDPA_ATTR_MGMTDEV_DEV_NAME])
		return MNL_CB_ERROR;

	pr_out_mgmtdev_show(vdpa, nlh, tb);

	return MNL_CB_OK;
}

static int cmd_mgmtdev_show(struct vdpa *vdpa, int argc, char **argv)
{
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	struct nlmsghdr *nlh;
	int err;

	if (argc == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlu_gen_socket_cmd_prepare(&vdpa->nlg, VDPA_CMD_MGMTDEV_GET,
					  flags);
	if (argc > 0) {
		err = vdpa_argv_parse_put(nlh, vdpa, argc, argv,
					  VDPA_OPT_MGMTDEV_HANDLE);
		if (err)
			return err;
	}

	pr_out_section_start(vdpa, "mgmtdev");
	err = mnlu_gen_socket_sndrcv(&vdpa->nlg, nlh, cmd_mgmtdev_show_cb, vdpa);
	pr_out_section_end(vdpa);
	return err;
}

static int cmd_mgmtdev(struct vdpa *vdpa, int argc, char **argv)
{
	if (!argc || matches(*argv, "help") == 0) {
		cmd_mgmtdev_help();
		return 0;
	} else if (matches(*argv, "show") == 0 ||
		   matches(*argv, "list") == 0) {
		return cmd_mgmtdev_show(vdpa, argc - 1, argv + 1);
	}
	fprintf(stderr, "Command \"%s\" not found\n", *argv);
	return -ENOENT;
}

static void cmd_dev_help(void)
{
	fprintf(stderr, "Usage: vdpa dev show [ DEV ]\n");
	fprintf(stderr, "       vdpa dev add name NAME mgmtdev MANAGEMENTDEV\n");
	fprintf(stderr, "       vdpa dev del DEV\n");
}

static const char *device_type_name(uint32_t type)
{
	switch (type) {
	case 0x1: return "network";
	case 0x2: return "block";
	default: return "<unknown type>";
	}
}

static void pr_out_dev(struct vdpa *vdpa, struct nlattr **tb)
{
	const char *mdev_name = mnl_attr_get_str(tb[VDPA_ATTR_MGMTDEV_DEV_NAME]);
	uint32_t device_id = mnl_attr_get_u32(tb[VDPA_ATTR_DEV_ID]);
	const char *mdev_bus_name = NULL;
	char mgmtdev_buf[128];

	if (tb[VDPA_ATTR_MGMTDEV_BUS_NAME])
		mdev_bus_name = mnl_attr_get_str(tb[VDPA_ATTR_MGMTDEV_BUS_NAME]);

	if (mdev_bus_name)
		sprintf(mgmtdev_buf, "%s/%s", mdev_bus_name, mdev_name);
	else
		sprintf(mgmtdev_buf, "%s", mdev_name);
	pr_out_vdev_handle_start(vdpa, tb);
	print_string(PRINT_ANY, "type", "type %s", device_type_name(device_id));
	print_string(PRINT_ANY, "mgmtdev", " mgmtdev %s", mgmtdev_buf);

	if (tb[VDPA_ATTR_DEV_VENDOR_ID])
		print_uint(PRINT_ANY, "vendor_id", " vendor_id %u",
			   mnl_attr_get_u32(tb[VDPA_ATTR_DEV_VENDOR_ID]));
	if (tb[VDPA_ATTR_DEV_MAX_VQS])
		print_uint(PRINT_ANY, "max_vqs", " max_vqs %u",
			   mnl_attr_get_u32(tb[VDPA_ATTR_DEV_MAX_VQS]));
	if (tb[VDPA_ATTR_DEV_MAX_VQ_SIZE])
		print_uint(PRINT_ANY, "max_vq_size", " max_vq_size %u",
			   mnl_attr_get_u16(tb[VDPA_ATTR_DEV_MAX_VQ_SIZE]));
	pr_out_vdev_handle_end(vdpa);
}

static int cmd_dev_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[VDPA_ATTR_MAX + 1] = {};
	struct vdpa *vdpa = data;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[VDPA_ATTR_MGMTDEV_DEV_NAME] ||
	    !tb[VDPA_ATTR_DEV_NAME] || !tb[VDPA_ATTR_DEV_ID])
		return MNL_CB_ERROR;
	pr_out_dev(vdpa, tb);
	return MNL_CB_OK;
}

static int cmd_dev_show(struct vdpa *vdpa, int argc, char **argv)
{
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	struct nlmsghdr *nlh;
	int err;

	if (argc <= 0)
		flags |= NLM_F_DUMP;

	nlh = mnlu_gen_socket_cmd_prepare(&vdpa->nlg, VDPA_CMD_DEV_GET, flags);
	if (argc > 0) {
		err = vdpa_argv_parse_put(nlh, vdpa, argc, argv,
					  VDPA_OPT_VDEV_HANDLE);
		if (err)
			return err;
	}

	pr_out_section_start(vdpa, "dev");
	err = mnlu_gen_socket_sndrcv(&vdpa->nlg, nlh, cmd_dev_show_cb, vdpa);
	pr_out_section_end(vdpa);
	return err;
}

static int cmd_dev_add(struct vdpa *vdpa, int argc, char **argv)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlu_gen_socket_cmd_prepare(&vdpa->nlg, VDPA_CMD_DEV_NEW,
					  NLM_F_REQUEST | NLM_F_ACK);
	err = vdpa_argv_parse_put(nlh, vdpa, argc, argv,
				  VDPA_OPT_VDEV_MGMTDEV_HANDLE | VDPA_OPT_VDEV_NAME);
	if (err)
		return err;

	return mnlu_gen_socket_sndrcv(&vdpa->nlg, nlh, NULL, NULL);
}

static int cmd_dev_del(struct vdpa *vdpa,  int argc, char **argv)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlu_gen_socket_cmd_prepare(&vdpa->nlg, VDPA_CMD_DEV_DEL,
					  NLM_F_REQUEST | NLM_F_ACK);
	err = vdpa_argv_parse_put(nlh, vdpa, argc, argv, VDPA_OPT_VDEV_HANDLE);
	if (err)
		return err;

	return mnlu_gen_socket_sndrcv(&vdpa->nlg, nlh, NULL, NULL);
}

static int cmd_dev(struct vdpa *vdpa, int argc, char **argv)
{
	if (!argc)
		return cmd_dev_show(vdpa, argc - 1, argv + 1);

	if (matches(*argv, "help") == 0) {
		cmd_dev_help();
		return 0;
	} else if (matches(*argv, "show") == 0 ||
		   matches(*argv, "list") == 0) {
		return cmd_dev_show(vdpa, argc - 1, argv + 1);
	} else if (matches(*argv, "add") == 0) {
		return cmd_dev_add(vdpa, argc - 1, argv + 1);
	} else if (matches(*argv, "del") == 0) {
		return cmd_dev_del(vdpa, argc - 1, argv + 1);
	}
	fprintf(stderr, "Command \"%s\" not found\n", *argv);
	return -ENOENT;
}

static void help(void)
{
	fprintf(stderr,
		"Usage: vdpa [ OPTIONS ] OBJECT { COMMAND | help }\n"
		"where  OBJECT := { mgmtdev | dev }\n"
		"       OPTIONS := { -V[ersion] | -n[o-nice-names] | -j[son] | -p[retty] | -v[erbose] }\n");
}

static int vdpa_cmd(struct vdpa *vdpa, int argc, char **argv)
{
	if (!argc || matches(*argv, "help") == 0) {
		help();
		return 0;
	} else if (matches(*argv, "mgmtdev") == 0) {
		return cmd_mgmtdev(vdpa, argc - 1, argv + 1);
	} else if (matches(*argv, "dev") == 0) {
		return cmd_dev(vdpa, argc - 1, argv + 1);
	}
	fprintf(stderr, "Object \"%s\" not found\n", *argv);
	return -ENOENT;
}

static int vdpa_init(struct vdpa *vdpa)
{
	int err;

	err = mnlu_gen_socket_open(&vdpa->nlg, VDPA_GENL_NAME,
				   VDPA_GENL_VERSION);
	if (err) {
		fprintf(stderr, "Failed to connect to vdpa Netlink\n");
		return -errno;
	}
	new_json_obj_plain(vdpa->json_output);
	return 0;
}

static void vdpa_fini(struct vdpa *vdpa)
{
	delete_json_obj_plain();
	mnlu_gen_socket_close(&vdpa->nlg);
}

static struct vdpa *vdpa_alloc(void)
{
	struct vdpa *vdpa = calloc(1, sizeof(struct vdpa));

	if (!vdpa)
		return NULL;

	vdpa->indent = alloc_indent_mem();
	if (!vdpa->indent)
		goto indent_err;

	return vdpa;

indent_err:
	free(vdpa);
	return NULL;
}

static void vdpa_free(struct vdpa *vdpa)
{
	free_indent_mem(vdpa->indent);
	free(vdpa);
}

int main(int argc, char **argv)
{
	static const struct option long_options[] = {
		{ "Version",		no_argument,	NULL, 'V' },
		{ "json",		no_argument,	NULL, 'j' },
		{ "pretty",		no_argument,	NULL, 'p' },
		{ "help",		no_argument,	NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};
	struct vdpa *vdpa;
	int opt;
	int err;
	int ret;

	vdpa = vdpa_alloc();
	if (!vdpa) {
		fprintf(stderr, "Failed to allocate memory for vdpa\n");
		return EXIT_FAILURE;
	}

	while ((opt = getopt_long(argc, argv, "Vjpsh", long_options, NULL)) >= 0) {
		switch (opt) {
		case 'V':
			printf("vdpa utility, iproute2-%s\n", version);
			ret = EXIT_SUCCESS;
			goto vdpa_free;
		case 'j':
			vdpa->json_output = true;
			break;
		case 'p':
			pretty = true;
			break;
		case 'h':
			help();
			ret = EXIT_SUCCESS;
			goto vdpa_free;
		default:
			fprintf(stderr, "Unknown option.\n");
			help();
			ret = EXIT_FAILURE;
			goto vdpa_free;
		}
	}

	argc -= optind;
	argv += optind;

	err = vdpa_init(vdpa);
	if (err) {
		ret = EXIT_FAILURE;
		goto vdpa_free;
	}

	err = vdpa_cmd(vdpa, argc, argv);
	if (err) {
		ret = EXIT_FAILURE;
		goto vdpa_fini;
	}

	ret = EXIT_SUCCESS;

vdpa_fini:
	vdpa_fini(vdpa);
vdpa_free:
	vdpa_free(vdpa);
	return ret;
}
