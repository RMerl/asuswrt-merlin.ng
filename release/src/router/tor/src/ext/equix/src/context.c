/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include <stdlib.h>
#include <equix.h>
#include <virtual_memory.h>
#include "context.h"
#include "solver_heap.h"

equix_ctx* equix_alloc(equix_ctx_flags flags) {
	equix_ctx* ctx = malloc(sizeof(equix_ctx));
	if (ctx == NULL) {
		goto failure;
	}
	ctx->flags = (equix_ctx_flags)0;

	if (flags & EQUIX_CTX_MUST_COMPILE) {
		ctx->hash_func = hashx_alloc(HASHX_TYPE_COMPILED);
	} else if (flags & EQUIX_CTX_TRY_COMPILE) {
		ctx->hash_func = hashx_alloc(HASHX_TRY_COMPILE);
	} else {
		ctx->hash_func = hashx_alloc(HASHX_TYPE_INTERPRETED);
	}
	if (ctx->hash_func == NULL) {
		goto failure;
	}

	if (flags & EQUIX_CTX_SOLVE) {
		if (flags & EQUIX_CTX_HUGEPAGES) {
#ifdef EQUIX_SUPPORT_HUGEPAGES
			ctx->heap = hashx_vm_alloc_huge(sizeof(solver_heap));
#else
			ctx->heap = NULL;
#endif
		}
		else {
			ctx->heap = malloc(sizeof(solver_heap));
		}
		if (ctx->heap == NULL) {
			goto failure;
		}
	} else {
		ctx->heap = NULL;
	}

	ctx->flags = flags;
	return ctx;
failure:
	equix_free(ctx);
	return NULL;
}

void equix_free(equix_ctx* ctx) {
	if (ctx != NULL) {
		if (ctx->flags & EQUIX_CTX_SOLVE) {
			if (ctx->flags & EQUIX_CTX_HUGEPAGES) {
				hashx_vm_free(ctx->heap, sizeof(solver_heap));
			}
			else {
				free(ctx->heap);
			}
		}
		hashx_free(ctx->hash_func);
		free(ctx);
	}
}
