/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * caddy.c -- esd VME8349 support for "missing" access modes in TSI148.
 * Copyright (c) 2009 esd gmbh.
 *
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 */

#ifndef __CADDY_H__
#define __CADDY_H__

#define CMD_SIZE	1024
#define ANSWER_SIZE	1024
#define CADDY_MAGIC	"esd vme8349 V1.0"

enum caddy_cmds {
	CADDY_CMD_IO_READ_8,
	CADDY_CMD_IO_READ_16,
	CADDY_CMD_IO_READ_32,
	CADDY_CMD_IO_WRITE_8,
	CADDY_CMD_IO_WRITE_16,
	CADDY_CMD_IO_WRITE_32,
	CADDY_CMD_CONFIG_READ_8,
	CADDY_CMD_CONFIG_READ_16,
	CADDY_CMD_CONFIG_READ_32,
	CADDY_CMD_CONFIG_WRITE_8,
	CADDY_CMD_CONFIG_WRITE_16,
	CADDY_CMD_CONFIG_WRITE_32,
};

struct caddy_cmd {
	uint32_t cmd;
	uint32_t issue;
	uint32_t addr;
	uint32_t par[5];
};

struct caddy_answer {
	uint32_t answer;
	uint32_t issue;
	uint32_t status;
	uint32_t par[5];
};

struct caddy_interface {
	uint8_t  magic[16];
	uint32_t cmd_in;
	uint32_t cmd_out;
	uint32_t heartbeat;
	uint32_t reserved1;
	struct caddy_cmd cmd[CMD_SIZE];
	uint32_t answer_in;
	uint32_t answer_out;
	uint32_t reserved2;
	uint32_t reserved3;
	struct caddy_answer answer[CMD_SIZE];
};

#endif /* of __CADDY_H__ */
