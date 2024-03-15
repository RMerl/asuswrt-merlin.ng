/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include <stdbool.h>

#include "compiler.h"
#include "virtual_memory.h"
#include "program.h"
#include "context.h"

void hashx_compiler_init(hashx_ctx* ctx) {
	/* This can fail, but it's uncommon. We report this up the call chain
	 * later, at the same time as an mprotect or similar failure. */
	ctx->compiler_mem = hashx_vm_alloc(COMP_CODE_SIZE);
}

void hashx_compiler_destroy(hashx_ctx* ctx) {
	hashx_vm_free(ctx->compiler_mem, COMP_CODE_SIZE);
}
