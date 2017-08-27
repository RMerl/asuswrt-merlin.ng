/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "command.h"

#include <errno.h>

static int stats(vici_conn_t *conn)
{
	vici_req_t *req;
	vici_res_t *res;
	char *arg;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	int ret;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 'P':
				format |= COMMAND_FORMAT_PRETTY;
				/* fall through to raw */
			case 'r':
				format |= COMMAND_FORMAT_RAW;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --stats option");
		}
		break;
	}

	req = vici_begin("stats");
	res = vici_submit(req, conn);
	if (!res)
	{
		ret = errno;
		fprintf(stderr, "stats request failed: %s\n", strerror(errno));
		return ret;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "stats reply", format & COMMAND_FORMAT_PRETTY, stdout);
	}
	else
	{
		printf("uptime: %s, since %s\n",
			vici_find_str(res, "", "uptime.running"),
			vici_find_str(res, "", "uptime.since"));

		printf("worker threads: %s total, %s idle, working: %s/%s/%s/%s\n",
			vici_find_str(res, "", "workers.total"),
			vici_find_str(res, "", "workers.idle"),
			vici_find_str(res, "", "workers.active.critical"),
			vici_find_str(res, "", "workers.active.high"),
			vici_find_str(res, "", "workers.active.medium"),
			vici_find_str(res, "", "workers.active.low"));

		printf("job queues: %s/%s/%s/%s\n",
			vici_find_str(res, "", "queues.critical"),
			vici_find_str(res, "", "queues.high"),
			vici_find_str(res, "", "queues.medium"),
			vici_find_str(res, "", "queues.low"));

		printf("jobs scheduled: %s\n",
			vici_find_str(res, "", "scheduled"));

		printf("IKE_SAs: %s total, %s half-open\n",
			vici_find_str(res, "", "ikesas.total"),
			vici_find_str(res, "", "ikesas.half-open"));

		if (vici_find_str(res, NULL, "mem.total"))
		{
			printf("memory usage: %s bytes, %s allocations\n",
				vici_find_str(res, "", "mem.total"),
				vici_find_str(res, "", "mem.allocs"));
		}
		if (vici_find_str(res, NULL, "mallinfo.sbrk"))
		{
			printf("mallinfo: sbrk %s, mmap %s, used %s, free %s\n",
				vici_find_str(res, "", "mallinfo.sbrk"),
				vici_find_str(res, "", "mallinfo.mmap"),
				vici_find_str(res, "", "mallinfo.used"),
				vici_find_str(res, "", "mallinfo.free"));
		}
	}
	vici_free_res(res);
	return 0;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		stats, 'S', "stats", "show daemon stats information",
		{"[--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}
