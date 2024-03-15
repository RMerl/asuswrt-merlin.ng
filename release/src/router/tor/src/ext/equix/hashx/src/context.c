/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include <stdlib.h>
#include <string.h>

#include <hashx.h>
#include "context.h"
#include "compiler.h"
#include "program.h"

#define STRINGIZE_INNER(x) #x
#define STRINGIZE(x) STRINGIZE_INNER(x)

/* Salt used when generating hash functions. Useful for domain separation. */
#ifndef HASHX_SALT
#define HASHX_SALT HashX v1
#endif

/* Blake2b params used to generate program keys */
const blake2b_param hashx_blake2_params = {
	.digest_length = 64,
	.key_length = 0,
	.fanout = 1,
	.depth = 1,
	.leaf_length = 0,
	.node_offset = 0,
	.node_depth = 0,
	.inner_length = 0,
	.reserved = { 0 },
	.salt = STRINGIZE(HASHX_SALT),
	.personal = { 0 }
};

hashx_ctx* hashx_alloc(hashx_type type) {
	hashx_ctx* ctx = malloc(sizeof(hashx_ctx));
	if (ctx == NULL)
		return NULL;

	memset(ctx, 0, sizeof *ctx);
	ctx->ctx_type = type;
	if (type == HASHX_TYPE_COMPILED || type == HASHX_TRY_COMPILE) {
		hashx_compiler_init(ctx);
	}

#ifdef HASHX_BLOCK_MODE
	memcpy(&ctx->params, &hashx_blake2_params, 32);
#endif
	return ctx;
}

void hashx_free(hashx_ctx* ctx) {
	if (ctx != NULL) {
		hashx_compiler_destroy(ctx);
		free(ctx);
	}
}

#ifdef HASHX_RNG_CALLBACK
void hashx_rng_callback(hashx_ctx* ctx,
                        void (*callback)(uint64_t*, void*),
                        void* callback_user_data)
{
	ctx->program.rng_callback = callback;
	ctx->program.rng_callback_user_data = callback_user_data;
}
#endif
