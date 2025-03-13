/*
    i2ctransfer.c - A user-space program to send concatenated i2c messages
    Copyright (C) 2015-17 Wolfram Sang <wsa@sang-engineering.com>
    Copyright (C) 2015-17 Renesas Electronics Corporation

    Based on i2cget.c:
    Copyright (C) 2005-2012  Jean Delvare <jdelvare@suse.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "i2cbusses.h"
#include "util.h"
#include "../version.h"

enum parse_state {
	PARSE_GET_DESC,
	PARSE_GET_DATA,
};

#define PRINT_STDERR	(1 << 0)
#define PRINT_READ_BUF	(1 << 1)
#define PRINT_WRITE_BUF	(1 << 2)
#define PRINT_HEADER	(1 << 3)

static void help(void)
{
	fprintf(stderr,
		"Usage: i2ctransfer [-f] [-y] [-v] [-V] I2CBUS DESC [DATA] [DESC [DATA]]...\n"
		"  I2CBUS is an integer or an I2C bus name\n"
		"  DESC describes the transfer in the form: {r|w}LENGTH[@address]\n"
		"    1) read/write-flag 2) LENGTH (range 0-65535) 3) I2C address (use last one if omitted)\n"
		"  DATA are LENGTH bytes for a write message. They can be shortened by a suffix:\n"
		"    = (keep value constant until LENGTH)\n"
		"    + (increase value by 1 until LENGTH)\n"
		"    - (decrease value by 1 until LENGTH)\n"
		"    p (use pseudo random generator until LENGTH with value as seed)\n\n"
		"Example (bus 0, read 8 byte at offset 0x64 from EEPROM at 0x50):\n"
		"  # i2ctransfer 0 w1@0x50 0x64 r8\n"
		"Example (same EEPROM, at offset 0x42 write 0xff 0xfe ... 0xf0):\n"
		"  # i2ctransfer 0 w17@0x50 0x42 0xff-\n");
}

static int check_funcs(int file)
{
	unsigned long funcs;

	/* check adapter functionality */
	if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
		fprintf(stderr, "Error: Could not get the adapter "
			"functionality matrix: %s\n", strerror(errno));
		return -1;
	}

	if (!(funcs & I2C_FUNC_I2C)) {
		fprintf(stderr, MISSING_FUNC_FMT, "I2C transfers");
		return -1;
	}

	return 0;
}

static void print_msgs(struct i2c_msg *msgs, __u32 nmsgs, unsigned flags)
{
	FILE *output = flags & PRINT_STDERR ? stderr : stdout;
	unsigned i;
	__u16 j;

	for (i = 0; i < nmsgs; i++) {
		int read = msgs[i].flags & I2C_M_RD;
		int print_buf = (read && (flags & PRINT_READ_BUF)) ||
				(!read && (flags & PRINT_WRITE_BUF));

		if (flags & PRINT_HEADER)
			fprintf(output, "msg %u: addr 0x%02x, %s, len %u",
				i, msgs[i].addr, read ? "read" : "write", msgs[i].len);

		if (msgs[i].len && print_buf) {
			if (flags & PRINT_HEADER)
				fprintf(output, ", buf ");
			for (j = 0; j < msgs[i].len - 1; j++)
				fprintf(output, "0x%02x ", msgs[i].buf[j]);
			/* Print final byte with newline */
			fprintf(output, "0x%02x\n", msgs[i].buf[j]);
		} else if (flags & PRINT_HEADER) {
			fprintf(output, "\n");
		}
	}
}

static int confirm(const char *filename, struct i2c_msg *msgs, __u32 nmsgs)
{
	fprintf(stderr, "WARNING! This program can confuse your I2C bus, cause data loss and worse!\n");
	fprintf(stderr, "I will send the following messages to device file %s:\n", filename);
	print_msgs(msgs, nmsgs, PRINT_STDERR | PRINT_HEADER | PRINT_WRITE_BUF);

	fprintf(stderr, "Continue? [y/N] ");
	fflush(stderr);
	if (!user_ack(0)) {
		fprintf(stderr, "Aborting on user request.\n");
		return 0;
	}

	return 1;
}

int main(int argc, char *argv[])
{
	char filename[20];
	int i2cbus, address = -1, file, arg_idx = 1, nmsgs = 0, nmsgs_sent, i;
	int force = 0, yes = 0, version = 0, verbose = 0;
	struct i2c_msg msgs[I2C_RDRW_IOCTL_MAX_MSGS];
	enum parse_state state = PARSE_GET_DESC;
	unsigned buf_idx = 0;

	for (i = 0; i < I2C_RDRW_IOCTL_MAX_MSGS; i++)
		msgs[i].buf = NULL;

	/* handle (optional) arg_idx first */
	while (arg_idx < argc && argv[arg_idx][0] == '-') {
		switch (argv[arg_idx][1]) {
		case 'V': version = 1; break;
		case 'v': verbose = 1; break;
		case 'f': force = 1; break;
		case 'y': yes = 1; break;
		default:
			fprintf(stderr, "Error: Unsupported option \"%s\"!\n",
				argv[arg_idx]);
			help();
			exit(1);
		}
		arg_idx++;
	}

	if (version) {
		fprintf(stderr, "i2ctransfer version %s\n", VERSION);
		exit(0);
	}

	if (arg_idx == argc) {
		help();
		exit(1);
	}

	i2cbus = lookup_i2c_bus(argv[arg_idx++]);
	if (i2cbus < 0)
		exit(1);

	file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
	if (file < 0 || check_funcs(file))
		exit(1);

	while (arg_idx < argc) {
		char *arg_ptr = argv[arg_idx];
		unsigned long len, raw_data;
		__u16 flags;
		__u8 data, *buf;
		char *end;

		if (nmsgs > I2C_RDRW_IOCTL_MAX_MSGS) {
			fprintf(stderr, "Error: Too many messages (max: %d)\n",
				I2C_RDRW_IOCTL_MAX_MSGS);
			goto err_out;
		}

		switch (state) {
		case PARSE_GET_DESC:
			flags = 0;

			switch (*arg_ptr++) {
			case 'r': flags |= I2C_M_RD; break;
			case 'w': break;
			default:
				fprintf(stderr, "Error: Invalid direction\n");
				goto err_out_with_arg;
			}

			len = strtoul(arg_ptr, &end, 0);
			if (len > 0xffff || arg_ptr == end) {
				fprintf(stderr, "Error: Length invalid\n");
				goto err_out_with_arg;
			}

			arg_ptr = end;
			if (*arg_ptr) {
				if (*arg_ptr++ != '@') {
					fprintf(stderr, "Error: Unknown separator after length\n");
					goto err_out_with_arg;
				}

				/* We skip 10-bit support for now. If we want it,
				 * it should be marked with a 't' flag before
				 * the address here.
				 */

				if (!force) {
					address = parse_i2c_address(arg_ptr);
					if (address < 0)
						goto err_out_with_arg;

					/* Ensure address is not busy */
					if (set_slave_addr(file, address, 0))
						goto err_out_with_arg;
				} else {
					/* 'force' allows whole address range */
					address = strtol(arg_ptr, &end, 0);
					if (arg_ptr == end || *end || address > 0x7f) {
						fprintf(stderr, "Error: Invalid chip address\n");
						goto err_out_with_arg;
					}
				}
			} else {
				/* Reuse last address if possible */
				if (address < 0) {
					fprintf(stderr, "Error: No address given\n");
					goto err_out_with_arg;
				}
			}

			msgs[nmsgs].addr = address;
			msgs[nmsgs].flags = flags;
			msgs[nmsgs].len = len;

			if (len) {
				buf = malloc(len);
				if (!buf) {
					fprintf(stderr, "Error: No memory for buffer\n");
					goto err_out_with_arg;
				}
				memset(buf, 0, len);
				msgs[nmsgs].buf = buf;
			}

			if (flags & I2C_M_RD || len == 0) {
				nmsgs++;
			} else {
				buf_idx = 0;
				state = PARSE_GET_DATA;
			}

			break;

		case PARSE_GET_DATA:
			raw_data = strtoul(arg_ptr, &end, 0);
			if (raw_data > 0xff || arg_ptr == end) {
				fprintf(stderr, "Error: Invalid data byte\n");
				goto err_out_with_arg;
			}
			data = raw_data;
			len = msgs[nmsgs].len;

			while (buf_idx < len) {
				msgs[nmsgs].buf[buf_idx++] = data;

				if (!*end)
					break;

				switch (*end) {
				/* Pseudo randomness (8 bit AXR with a=13 and b=27) */
				case 'p':
					data = (data ^ 27) + 13;
					data = (data << 1) | (data >> 7);
					break;
				case '+': data++; break;
				case '-': data--; break;
				case '=': break;
				default:
					fprintf(stderr, "Error: Invalid data byte suffix\n");
					goto err_out_with_arg;
				}
			}

			if (buf_idx == len) {
				nmsgs++;
				state = PARSE_GET_DESC;
			}

			break;

		default:
			/* Should never happen */
			fprintf(stderr, "Internal Error: Unknown state in state machine!\n");
			goto err_out;
		}

		arg_idx++;
	}

	if (state != PARSE_GET_DESC || nmsgs == 0) {
		fprintf(stderr, "Error: Incomplete message\n");
		goto err_out;
	}

	if (yes || confirm(filename, msgs, nmsgs)) {
		struct i2c_rdwr_ioctl_data rdwr;

		rdwr.msgs = msgs;
		rdwr.nmsgs = nmsgs;
		nmsgs_sent = ioctl(file, I2C_RDWR, &rdwr);
		if (nmsgs_sent < 0) {
			fprintf(stderr, "Error: Sending messages failed: %s\n", strerror(errno));
			goto err_out;
		} else if (nmsgs_sent < nmsgs) {
			fprintf(stderr, "Warning: only %d/%d messages were sent\n", nmsgs_sent, nmsgs);
		}

		print_msgs(msgs, nmsgs_sent, PRINT_READ_BUF | (verbose ? PRINT_HEADER | PRINT_WRITE_BUF : 0));
	}

	close(file);

	for (i = 0; i < nmsgs; i++)
		free(msgs[i].buf);

	exit(0);

 err_out_with_arg:
	fprintf(stderr, "Error: faulty argument is '%s'\n", argv[arg_idx]);
 err_out:
	close(file);

	for (i = 0; i <= nmsgs; i++)
		free(msgs[i].buf);

	exit(1);
}
