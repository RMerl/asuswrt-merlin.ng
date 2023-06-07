// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * rdma.c	RDMA tool
 * Authors:     Leon Romanovsky <leonro@mellanox.com>
 */

#include "rdma.h"
#include "version.h"
#include "color.h"

static void help(char *name)
{
	pr_out("Usage: %s [ OPTIONS ] OBJECT { COMMAND | help }\n"
	       "       %s [ -f[orce] ] -b[atch] filename\n"
	       "where  OBJECT := { dev | link | resource | system | statistic | help }\n"
	       "       OPTIONS := { -V[ersion] | -d[etails] | -j[son] | -p[retty] -r[aw]}\n", name, name);
}

static int cmd_help(struct rd *rd)
{
	help(rd->filename);
	return 0;
}

static int rd_cmd(struct rd *rd, int argc, char **argv)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		cmd_help },
		{ "help",	cmd_help },
		{ "dev",	cmd_dev },
		{ "link",	cmd_link },
		{ "resource",	cmd_res },
		{ "system",	cmd_sys },
		{ "statistic",	cmd_stat },
		{ 0 }
	};

	rd->argc = argc;
	rd->argv = argv;

	return rd_exec_cmd(rd, cmds, "object");
}

static int rd_batch_cmd(int argc, char *argv[], void *data)
{
	struct rd *rd = data;

	return rd_cmd(rd, argc, argv);
}

static int rd_batch(struct rd *rd, const char *name, bool force)
{
	return do_batch(name, force, rd_batch_cmd, rd);
}

static int rd_init(struct rd *rd, char *filename)
{
	uint32_t seq;
	int ret;

	rd->filename = filename;
	INIT_LIST_HEAD(&rd->dev_map_list);
	INIT_LIST_HEAD(&rd->filter_list);

	rd->buff = malloc(MNL_SOCKET_BUFFER_SIZE);
	if (!rd->buff)
		return -ENOMEM;

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_GET,
		       &seq, (NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP));
	ret = rd_send_msg(rd);
	if (ret)
		return ret;

	return rd_recv_msg(rd, rd_dev_init_cb, rd, seq);
}

static void rd_cleanup(struct rd *rd)
{
	rd_free(rd);
}

int main(int argc, char **argv)
{
	static const struct option long_options[] = {
		{ "version",		no_argument,		NULL, 'V' },
		{ "help",		no_argument,		NULL, 'h' },
		{ "json",		no_argument,		NULL, 'j' },
		{ "pretty",		no_argument,		NULL, 'p' },
		{ "details",		no_argument,		NULL, 'd' },
		{ "raw",		no_argument,		NULL, 'r' },
		{ "force",		no_argument,		NULL, 'f' },
		{ "batch",		required_argument,	NULL, 'b' },
		{ NULL, 0, NULL, 0 }
	};
	bool show_driver_details = false;
	const char *batch_file = NULL;
	bool show_details = false;
	bool json_output = false;
	bool show_raw = false;
	bool force = false;
	struct rd rd = {};
	char *filename;
	int opt;
	int err;
	filename = basename(argv[0]);

	while ((opt = getopt_long(argc, argv, ":Vhdrpjfb:",
				  long_options, NULL)) >= 0) {
		switch (opt) {
		case 'V':
			printf("%s utility, iproute2-%s\n",
			       filename, version);
			return EXIT_SUCCESS;
		case 'p':
			pretty = 1;
			break;
		case 'd':
			if (show_details)
				show_driver_details = true;
			else
				show_details = true;
			break;
		case 'r':
			show_raw = true;
			break;
		case 'j':
			json_output = 1;
			break;
		case 'f':
			force = true;
			break;
		case 'b':
			batch_file = optarg;
			break;
		case 'h':
			help(filename);
			return EXIT_SUCCESS;
		case ':':
			pr_err("-%c option requires an argument\n", optopt);
			return EXIT_FAILURE;
		default:
			pr_err("Unknown option.\n");
			help(filename);
			return EXIT_FAILURE;
		}
	}

	argc -= optind;
	argv += optind;

	rd.show_details = show_details;
	rd.show_driver_details = show_driver_details;
	rd.json_output = json_output;
	rd.pretty_output = pretty;
	rd.show_raw = show_raw;

	err = rd_init(&rd, filename);
	if (err)
		goto out;

	if (batch_file)
		err = rd_batch(&rd, batch_file, force);
	else
		err = rd_cmd(&rd, argc, argv);
out:
	/* Always cleanup */
	rd_cleanup(&rd);
	return err ? EXIT_FAILURE : EXIT_SUCCESS;
}
