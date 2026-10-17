// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 */

#include <common.h>
#include <command.h>
#include <iotrace.h>

static void do_print_stats(void)
{
	ulong start, size, needed_size, offset, count;

	printf("iotrace is %sabled\n", iotrace_get_enabled() ? "en" : "dis");
	iotrace_get_buffer(&start, &size, &needed_size, &offset, &count);
	printf("Start:  %08lx\n", start);
	printf("Actual Size:   %08lx\n", size);
	printf("Needed Size:   %08lx\n", needed_size);
	iotrace_get_region(&start, &size);
	printf("Region: %08lx\n", start);
	printf("Size:   %08lx\n", size);
	printf("Offset: %08lx\n", offset);
	printf("Output: %08lx\n", start + offset);
	printf("Count:  %08lx\n", count);
	printf("CRC32:  %08lx\n", (ulong)iotrace_get_checksum());
}

static void do_print_trace(void)
{
	ulong start, size, needed_size, offset, count;

	struct iotrace_record *cur_record;

	iotrace_get_buffer(&start, &size, &needed_size, &offset, &count);

	if (!start || !size || !count)
		return;

	printf("Timestamp  Value          Address\n");

	cur_record = (struct iotrace_record *)start;
	for (int i = 0; i < count; i++) {
		if (cur_record->flags & IOT_WRITE)
			printf("%08llu: 0x%08lx --> 0x%08llx\n",
			       cur_record->timestamp,
					cur_record->value,
					(unsigned long long)cur_record->addr);
		else
			printf("%08llu: 0x%08lx <-- 0x%08llx\n",
			       cur_record->timestamp,
					cur_record->value,
					(unsigned long long)cur_record->addr);

		cur_record++;
	}
}

static int do_set_buffer(int argc, char *const argv[])
{
	ulong addr = 0, size = 0;

	if (argc == 2) {
		addr = hextoul(*argv++, NULL);
		size = hextoul(*argv++, NULL);
	} else if (argc != 0) {
		return CMD_RET_USAGE;
	}

	iotrace_set_buffer(addr, size);

	return 0;
}

static int do_set_region(int argc, char *const argv[])
{
	ulong addr = 0, size = 0;

	if (argc == 2) {
		addr = hextoul(*argv++, NULL);
		size = hextoul(*argv++, NULL);
	} else if (argc != 0) {
		return CMD_RET_USAGE;
	}

	iotrace_set_region(addr, size);

	return 0;
}

int do_iotrace(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const char *cmd = argc < 2 ? NULL : argv[1];

	if (!cmd)
		return cmd_usage(cmdtp);
	switch (*cmd) {
	case 'b':
		return do_set_buffer(argc - 2, argv + 2);
	case 'l':
		return do_set_region(argc - 2, argv + 2);
	case 'p':
		iotrace_set_enabled(0);
		break;
	case 'r':
		iotrace_set_enabled(1);
		break;
	case 's':
		do_print_stats();
		break;
	case 'd':
		do_print_trace();
		break;
	default:
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	iotrace,	4,	1,	do_iotrace,
	"iotrace utility commands",
	"stats                        - display iotrace stats\n"
	"iotrace buffer <address> <size>      - set iotrace buffer\n"
	"iotrace limit <address> <size>       - set iotrace region limit\n"
	"iotrace pause                        - pause tracing\n"
	"iotrace resume                       - resume tracing\n"
	"iotrace dump                         - dump iotrace buffer"
);
