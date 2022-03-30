/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 */

#ifndef _AISIMAGE_H_
#define _AISIMAGE_H_

/* all values are for little endian systems */
#define AIS_MAGIC_WORD	0x41504954
#define AIS_FCN_MAX	8

enum {
	AIS_CMD_LOAD	= 0x58535901,
	AIS_CMD_VALCRC	= 0x58535902,
	AIS_CMD_ENCRC	= 0x58535903,
	AIS_CMD_DISCRC	= 0x58535904,
	AIS_CMD_JMP	= 0x58535905,
	AIS_CMD_JMPCLOSE = 0x58535906,
	AIS_CMD_BOOTTBL	= 0x58535907,
	AIS_CMD_FILL	= 0x5853590A,
	AIS_CMD_FNLOAD	= 0x5853590D,
	AIS_CMD_SEQREAD	= 0x58535963,
};

struct ais_cmd_load {
	uint32_t cmd;
	uint32_t addr;
	uint32_t size;
	uint32_t data[1];
};

struct ais_cmd_func {
	uint32_t cmd;
	uint32_t func_args;
	uint32_t parms[AIS_FCN_MAX];
};

struct ais_cmd_jmpclose {
	uint32_t cmd;
	uint32_t addr;
};

#define CMD_DATA_STR	"DATA"

enum ais_file_cmd {
	CMD_INVALID,
	CMD_FILL,
	CMD_CRCON,
	CMD_CRCOFF,
	CMD_CRCCHECK,
	CMD_JMPCLOSE,
	CMD_JMP,
	CMD_SEQREAD,
	CMD_DATA,
	CMD_PLL0,
	CMD_PLL1,
	CMD_CLK,
	CMD_DDR2,
	CMD_EMIFA,
	CMD_EMIFA_ASYNC,
	CMD_PLL,
	CMD_PSC,
	CMD_PINMUX,
	CMD_BOOTTABLE
};

enum aisimage_fld_types {
	CFG_INVALID = -1,
	CFG_COMMAND,
	CFG_VALUE,
};

struct ais_header {
	uint32_t magic;
	char data[1];
};

#endif /* _AISIMAGE_H_ */
