// SPDX-License-Identifier: GPL-2.0-only
/*
 * sha256_ce_glue.c - SHA-256 secure hash using ARMv8 Crypto Extensions
 *
 * Copyright (C) 2022 Linaro Ltd <loic.poulain@linaro.org>
 */

#include <common.h>
#include <u-boot/sha256.h>

extern void sha256_armv8_ce_process(uint32_t state[8], uint8_t const *src,
				    uint32_t blocks);

void sha256_process(sha256_context *ctx, const unsigned char *data,
		    unsigned int blocks)
{
	if (!blocks)
		return;

	sha256_armv8_ce_process(ctx->state, data, blocks);
}
