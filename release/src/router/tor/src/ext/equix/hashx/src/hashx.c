/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <hashx.h>
#include "blake2.h"
#include "hashx_endian.h"
#include "program.h"
#include "context.h"
#include "compiler.h"

#if HASHX_SIZE > 32
#error HASHX_SIZE cannot be more than 32
#endif

#ifndef HASHX_BLOCK_MODE
#define HASHX_INPUT_ARGS input
#else
#define HASHX_INPUT_ARGS input, size
#endif

static bool initialize_program(hashx_ctx* ctx, siphash_state keys[2]) {
	if (!hashx_program_generate(&keys[0], &ctx->program)) {
		return false;
	}
#ifndef HASHX_BLOCK_MODE
	memcpy(&ctx->keys, &keys[1], 32);
#else
	memcpy(&ctx->params.salt, &keys[1], 32);
#endif
	return true;
}

hashx_result hashx_make(hashx_ctx* ctx, const void* seed, size_t size) {
	assert(ctx != NULL);
	assert(seed != NULL || size == 0);

	uint8_t keys_bytes[2 * sizeof(siphash_state)];
	blake2b_state hash_state;
	hashx_blake2b_init_param(&hash_state, &hashx_blake2_params);
	hashx_blake2b_update(&hash_state, seed, size);
	hashx_blake2b_final(&hash_state, keys_bytes, sizeof(keys_bytes));

	siphash_state keys[2];
	keys[0].v0 = load64(keys_bytes + 0 * sizeof(uint64_t));
	keys[0].v1 = load64(keys_bytes + 1 * sizeof(uint64_t));
	keys[0].v2 = load64(keys_bytes + 2 * sizeof(uint64_t));
	keys[0].v3 = load64(keys_bytes + 3 * sizeof(uint64_t));
	keys[1].v0 = load64(keys_bytes + 4 * sizeof(uint64_t));
	keys[1].v1 = load64(keys_bytes + 5 * sizeof(uint64_t));
	keys[1].v2 = load64(keys_bytes + 6 * sizeof(uint64_t));
	keys[1].v3 = load64(keys_bytes + 7 * sizeof(uint64_t));

	ctx->func_type = (hashx_type)0;
	if (!initialize_program(ctx, keys)) {
		return HASHX_FAIL_SEED;
	}

	switch (ctx->ctx_type) {
	case HASHX_TYPE_INTERPRETED:
		ctx->func_type = HASHX_TYPE_INTERPRETED;
		return HASHX_OK;
	case HASHX_TYPE_COMPILED:
	case HASHX_TRY_COMPILE:
		if (ctx->compiler_mem != NULL &&
			hashx_compile(&ctx->program, ctx->compiler_mem)) {
			ctx->func_type = HASHX_TYPE_COMPILED;
			return HASHX_OK;
		}
		if (ctx->ctx_type == HASHX_TRY_COMPILE) {
			ctx->func_type = HASHX_TYPE_INTERPRETED;
			return HASHX_OK;
		} else {
			return HASHX_FAIL_COMPILE;
		}
	default:
		return HASHX_FAIL_UNDEFINED;
	}
}

hashx_result hashx_query_type(hashx_ctx* ctx, hashx_type *type_out) {
	assert(ctx != NULL);
	assert(type_out != NULL);

	if (ctx->func_type == (hashx_type)0) {
		return HASHX_FAIL_UNPREPARED;
	}
	*type_out = ctx->func_type;
	return HASHX_OK;
}

hashx_result hashx_exec(const hashx_ctx* ctx, HASHX_INPUT, void* output) {
	assert(ctx != NULL);
	assert(output != NULL);

	uint64_t r[8];
#ifndef HASHX_BLOCK_MODE
	hashx_siphash24_ctr_state512(&ctx->keys, input, r);
#else
	hashx_blake2b_4r(&ctx->params, input, size, r);
#endif

	if (ctx->func_type == HASHX_TYPE_COMPILED) {
		typedef void program_func(uint64_t r[8]);
		assert(ctx->compiler_mem != NULL);
		((program_func*)ctx->compiler_mem)(r);
	} else if (ctx->func_type == HASHX_TYPE_INTERPRETED) {
		hashx_program_execute(&ctx->program, r);
	} else {
		return HASHX_FAIL_UNPREPARED;
	}

	/* Hash finalization to remove bias toward 0 caused by multiplications */
#ifndef HASHX_BLOCK_MODE
	r[0] += ctx->keys.v0;
	r[1] += ctx->keys.v1;
	r[6] += ctx->keys.v2;
	r[7] += ctx->keys.v3;
#else
	const uint8_t* p = (const uint8_t*)&ctx->params;
	r[0] ^= load64(&p[8 * 0]);
	r[1] ^= load64(&p[8 * 1]);
	r[2] ^= load64(&p[8 * 2]);
	r[3] ^= load64(&p[8 * 3]);
	r[4] ^= load64(&p[8 * 4]);
	r[5] ^= load64(&p[8 * 5]);
	r[6] ^= load64(&p[8 * 6]);
	r[7] ^= load64(&p[8 * 7]);
#endif
	/* 1 SipRound per 4 registers is enough to pass SMHasher. */
	SIPROUND(r[0], r[1], r[2], r[3]);
	SIPROUND(r[4], r[5], r[6], r[7]);

	/* output */
#if HASHX_SIZE > 0
	/* optimized output for hash sizes that are multiples of 8 */
#if HASHX_SIZE % 8 == 0
	uint8_t* temp_out = (uint8_t*)output;
#if HASHX_SIZE >= 8
	store64(temp_out + 0, r[0] ^ r[4]);
#endif
#if HASHX_SIZE >= 16
	store64(temp_out + 8, r[1] ^ r[5]);
#endif
#if HASHX_SIZE >= 24
	store64(temp_out + 16, r[2] ^ r[6]);
#endif
#if HASHX_SIZE >= 32
	store64(temp_out + 24, r[3] ^ r[7]);
#endif
#else /* any output size */
	uint8_t temp_out[32];
#if HASHX_SIZE > 0
	store64(temp_out + 0, r[0] ^ r[4]);
#endif
#if HASHX_SIZE > 8
	store64(temp_out + 8, r[1] ^ r[5]);
#endif
#if HASHX_SIZE > 16
	store64(temp_out + 16, r[2] ^ r[6]);
#endif
#if HASHX_SIZE > 24
	store64(temp_out + 24, r[3] ^ r[7]);
#endif
	memcpy(output, temp_out, HASHX_SIZE);
#endif
#endif
	return HASHX_OK;
}
