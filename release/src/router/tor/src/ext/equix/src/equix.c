/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <equix.h>
#include <hashx.h>
#include "context.h"
#include "solver.h"
#include <hashx_endian.h>

static bool verify_order(const equix_solution* solution) {
	return
		tree_cmp4(&solution->idx[0], &solution->idx[4]) &&
		tree_cmp2(&solution->idx[0], &solution->idx[2]) &&
		tree_cmp2(&solution->idx[4], &solution->idx[6]) &&
		tree_cmp1(&solution->idx[0], &solution->idx[1]) &&
		tree_cmp1(&solution->idx[2], &solution->idx[3]) &&
		tree_cmp1(&solution->idx[4], &solution->idx[5]) &&
		tree_cmp1(&solution->idx[6], &solution->idx[7]);
}

static uint64_t sum_pair(hashx_ctx* hash_func, equix_idx left, equix_idx right) {
	uint8_t hash_left[HASHX_SIZE];
	uint8_t hash_right[HASHX_SIZE];
	hashx_result r_left = hashx_exec(hash_func, left, hash_left);
	hashx_result r_right = hashx_exec(hash_func, right, hash_right);
	if (r_left == HASHX_OK && r_right == HASHX_OK) {
		return load64(hash_left) + load64(hash_right);
	}
	assert(false);
	return ~(uint64_t)0;
}

static equix_result verify_internal(hashx_ctx* hash_func, const equix_solution* solution) {
	uint64_t pair0 = sum_pair(hash_func, solution->idx[0], solution->idx[1]);
	if (pair0 & EQUIX_STAGE1_MASK) {
		return EQUIX_FAIL_PARTIAL_SUM;
	}
	uint64_t pair1 = sum_pair(hash_func, solution->idx[2], solution->idx[3]);
	if (pair1 & EQUIX_STAGE1_MASK) {
		return EQUIX_FAIL_PARTIAL_SUM;
	}
	uint64_t pair4 = pair0 + pair1;
	if (pair4 & EQUIX_STAGE2_MASK) {
		return EQUIX_FAIL_PARTIAL_SUM;
	}
	uint64_t pair2 = sum_pair(hash_func, solution->idx[4], solution->idx[5]);
	if (pair2 & EQUIX_STAGE1_MASK) {
		return EQUIX_FAIL_PARTIAL_SUM;
	}
	uint64_t pair3 = sum_pair(hash_func, solution->idx[6], solution->idx[7]);
	if (pair3 & EQUIX_STAGE1_MASK) {
		return EQUIX_FAIL_PARTIAL_SUM;
	}
	uint64_t pair5 = pair2 + pair3;
	if (pair5 & EQUIX_STAGE2_MASK) {
		return EQUIX_FAIL_PARTIAL_SUM;
	}
	uint64_t pair6 = pair4 + pair5;
	if (pair6 & EQUIX_FULL_MASK) {
		return EQUIX_FAIL_FINAL_SUM;
	}
	return EQUIX_OK;
}

static equix_result equix_hashx_make(
	equix_ctx* ctx,
	const void* challenge,
	size_t challenge_size)
{
	switch (hashx_make(ctx->hash_func, challenge, challenge_size)) {
	case HASHX_OK:
		return EQUIX_OK;
	case HASHX_FAIL_SEED:
		return EQUIX_FAIL_CHALLENGE;
	case HASHX_FAIL_COMPILE:
		return EQUIX_FAIL_COMPILE;
	case HASHX_FAIL_UNDEFINED:
	case HASHX_FAIL_UNPREPARED:
	default:
		return EQUIX_FAIL_INTERNAL;
	}
}

equix_result equix_solve(
	equix_ctx* ctx,
	const void* challenge,
	size_t challenge_size,
	equix_solutions_buffer *output)
{
	if ((ctx->flags & EQUIX_CTX_SOLVE) == 0) {
		return EQUIX_FAIL_NO_SOLVER;
	}

	equix_result result = equix_hashx_make(ctx, challenge, challenge_size);
	if (result != EQUIX_OK) {
		return result;
	}

	output->flags = 0;
	hashx_type func_type;
	if (hashx_query_type(ctx->hash_func, &func_type) == HASHX_OK &&
		func_type == HASHX_TYPE_COMPILED) {
		output->flags |= EQUIX_SOLVER_DID_USE_COMPILER;
	}

	output->count = equix_solver_solve(ctx->hash_func, ctx->heap, output->sols);
	return EQUIX_OK;
}

equix_result equix_verify(
	equix_ctx* ctx,
	const void* challenge,
	size_t challenge_size,
	const equix_solution* solution)
{
	if (!verify_order(solution)) {
		return EQUIX_FAIL_ORDER;
	}

	equix_result result = equix_hashx_make(ctx, challenge, challenge_size);
	if (result != EQUIX_OK) {
		return result;
	}

	return verify_internal(ctx->hash_func, solution);
}
