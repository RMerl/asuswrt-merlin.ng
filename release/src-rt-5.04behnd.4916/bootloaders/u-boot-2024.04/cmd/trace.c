// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <mapmem.h>
#include <trace.h>
#include <asm/io.h>

static int get_args(int argc, char *const argv[], char **buff,
		    size_t *buff_ptr, size_t *buff_size)
{
	if (argc < 2)
		return -1;
	if (argc < 4) {
		*buff_size = env_get_ulong("profsize", 16, 0);
		*buff = map_sysmem(env_get_ulong("profbase", 16, 0),
				   *buff_size);
		*buff_ptr = env_get_ulong("profoffset", 16, 0);
	} else {
		*buff_size = hextoul(argv[3], NULL);
		*buff = map_sysmem(hextoul(argv[2], NULL),
				   *buff_size);
		*buff_ptr = 0;
	};
	return 0;
}

static int create_func_list(int argc, char *const argv[])
{
	size_t buff_size, avail, buff_ptr, needed, used;
	char *buff;
	int err;

	if (get_args(argc, argv, &buff, &buff_ptr, &buff_size))
		return -1;

	avail = buff_size - buff_ptr;
	err = trace_list_functions(buff + buff_ptr, avail, &needed);
	if (err)
		printf("Error: truncated (%#zx bytes needed)\n", needed);
	used = min(avail, (size_t)needed);
	printf("Function trace dumped to %08lx, size %#zx\n",
	       (ulong)map_to_sysmem(buff + buff_ptr), used);
	env_set_hex("profbase", map_to_sysmem(buff));
	env_set_hex("profsize", buff_size);
	env_set_hex("profoffset", buff_ptr + used);

	return 0;
}

static int create_call_list(int argc, char *const argv[])
{
	size_t buff_size, avail, buff_ptr, needed, used;
	char *buff;
	int err;

	if (get_args(argc, argv, &buff, &buff_ptr, &buff_size))
		return -1;

	avail = buff_size - buff_ptr;
	err = trace_list_calls(buff + buff_ptr, avail, &needed);
	if (err)
		printf("Error: truncated (%#zx bytes needed)\n", needed);
	used = min(avail, (size_t)needed);
	printf("Call list dumped to %08lx, size %#zx\n",
	       (ulong)map_to_sysmem(buff + buff_ptr), used);

	env_set_hex("profbase", map_to_sysmem(buff));
	env_set_hex("profsize", buff_size);
	env_set_hex("profoffset", buff_ptr + used);

	return 0;
}

int do_trace(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const char *cmd = argc < 2 ? NULL : argv[1];

	if (!cmd)
		return cmd_usage(cmdtp);
	switch (*cmd) {
	case 'p':
		trace_set_enabled(0);
		break;
	case 'c':
		if (create_call_list(argc, argv))
			return cmd_usage(cmdtp);
		break;
	case 'r':
		trace_set_enabled(1);
		break;
	case 'f':
		if (create_func_list(argc, argv))
			return cmd_usage(cmdtp);
		break;
	case 's':
		trace_print_stats();
		break;
	default:
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	trace,	4,	1,	do_trace,
	"trace utility commands",
	"stats                        - display tracing statistics\n"
	"trace pause                        - pause tracing\n"
	"trace resume                       - resume tracing\n"
	"trace funclist [<addr> <size>]     - dump function list into buffer\n"
	"trace calls  [<addr> <size>]       "
		"- dump function call trace into buffer"
);
