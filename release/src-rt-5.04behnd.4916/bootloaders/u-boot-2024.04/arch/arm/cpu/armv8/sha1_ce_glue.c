// SPDX-License-Identifier: GPL-2.0-only
/*
 * sha1_ce_glue.c - SHA-1 secure hash using ARMv8 Crypto Extensions
 *
 * Copyright (C) 2022 Linaro Ltd <loic.poulain@linaro.org>
 */

#include <common.h>
#include <u-boot/sha1.h>

extern void sha1_armv8_ce_process(uint32_t state[5], uint8_t const *src,
				  uint32_t blocks);

void sha1_process(sha1_context *ctx, const unsigned char *data,
		  unsigned int blocks)
{
	if (!blocks)
		return;

	sha1_armv8_ce_process(ctx->state, data, blocks);
}
