/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * Reinhard Pfau, Guntermann & Drunck GmbH, reinhard.pfau@gdsys.cc
 */

#ifndef __HRE_H
#define __HRE_H

struct key_program {
	uint32_t magic;
	uint32_t code_crc;
	uint32_t code_size;
	uint8_t code[];
};

struct h_reg {
	bool valid;
	uint8_t digest[20];
};

/* CCDM specific contants */
enum {
	/* NV indices */
	NV_COMMON_DATA_INDEX	= 0x40000001,
	/* magics for key blob chains */
	MAGIC_KEY_PROGRAM	= 0x68726500,
	MAGIC_HMAC		= 0x68616300,
	MAGIC_END_OF_CHAIN	= 0x00000000,
	/* sizes */
	NV_COMMON_DATA_MIN_SIZE	= 3 * sizeof(uint64_t) + 2 * sizeof(uint16_t),
};

int hre_verify_program(struct key_program *prg);
int hre_run_program(struct udevice *tpm, const uint8_t *code, size_t code_size);

#endif /* __HRE_H */
