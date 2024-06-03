/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Common values for the ChaCha20 algorithm
 */

#ifndef _CRYPTO_CHACHA20_H
#define _CRYPTO_CHACHA20_H

#include <crypto/skcipher.h>
#include <linux/types.h>
#include <linux/crypto.h>

#define CHACHA20_IV_SIZE	16
#define CHACHA20_KEY_SIZE	32
#define CHACHA20_BLOCK_SIZE	64

struct chacha20_ctx {
	u32 key[8];
};

void chacha20_block(u32 *state, u8 *stream);
void crypto_chacha20_init(u32 *state, struct chacha20_ctx *ctx, u8 *iv);
int crypto_chacha20_setkey(struct crypto_skcipher *tfm, const u8 *key,
			   unsigned int keysize);
int crypto_chacha20_crypt(struct skcipher_request *req);

enum chacha_constants { /* expand 32-byte k */
	CHACHA_CONSTANT_EXPA = 0x61707865U,
	CHACHA_CONSTANT_ND_3 = 0x3320646eU,
	CHACHA_CONSTANT_2_BY = 0x79622d32U,
	CHACHA_CONSTANT_TE_K = 0x6b206574U
};

static inline void chacha_init_consts(u32 *state)
{
	state[0]  = CHACHA_CONSTANT_EXPA;
	state[1]  = CHACHA_CONSTANT_ND_3;
	state[2]  = CHACHA_CONSTANT_2_BY;
	state[3]  = CHACHA_CONSTANT_TE_K;
}

#endif
