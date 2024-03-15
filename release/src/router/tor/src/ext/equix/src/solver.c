/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include "solver.h"
#include "context.h"
#include "solver_heap.h"
#include <hashx_endian.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#ifdef _MSC_VER
#pragma warning (disable : 4146) /* unary minus applied to unsigned type */
#endif

#define CLEAR(x) memset(&x, 0, sizeof(x))
#define MAKE_ITEM(bucket, left, right) ((left) << 17 | (right) << 8 | (bucket))
#define ITEM_BUCKET(item) (item) % NUM_COARSE_BUCKETS
#define ITEM_LEFT_IDX(item) (item) >> 17
#define ITEM_RIGHT_IDX(item) ((item) >> 8) & 511
#define INVERT_BUCKET(idx) -(idx) % NUM_COARSE_BUCKETS
#define INVERT_SCRATCH(idx) -(idx) % NUM_FINE_BUCKETS
#define STAGE1_IDX(buck, pos) heap->stage1_indices.buckets[buck].items[pos]
#define STAGE2_IDX(buck, pos) heap->stage2_indices.buckets[buck].items[pos]
#define STAGE3_IDX(buck, pos) heap->stage3_indices.buckets[buck].items[pos]
#define STAGE1_DATA(buck, pos) heap->stage1_data.buckets[buck].items[pos]
#define STAGE2_DATA(buck, pos) heap->stage2_data.buckets[buck].items[pos]
#define STAGE3_DATA(buck, pos) heap->stage3_data.buckets[buck].items[pos]
#define STAGE1_SIZE(buck) heap->stage1_indices.counts[buck]
#define STAGE2_SIZE(buck) heap->stage2_indices.counts[buck]
#define STAGE3_SIZE(buck) heap->stage3_indices.counts[buck]
#define SCRATCH(buck, pos) heap->scratch_ht.buckets[buck].items[pos]
#define SCRATCH_SIZE(buck) heap->scratch_ht.counts[buck]
#define SWAP_IDX(a, b)      \
    do {                    \
        equix_idx temp = a; \
        a = b;              \
        b = temp;           \
    } while(0)
#define CARRY (bucket_idx != 0)
#define BUCK_START 0
#define BUCK_END (NUM_COARSE_BUCKETS / 2 + 1)

typedef uint32_t u32;
typedef stage1_idx_item s1_idx;
typedef stage2_idx_item s2_idx;
typedef stage3_idx_item s3_idx;

static FORCE_INLINE bool hash_value(hashx_ctx* hash_func, equix_idx index, uint64_t *value_out) {
	char hash[HASHX_SIZE];
	hashx_result result = hashx_exec(hash_func, index, hash);
	if (result == HASHX_OK) {
		*value_out = load64(hash);
		return true;
	} else {
		assert(false);
		return false;
	}
}

static void build_solution_stage1(equix_idx* output, solver_heap* heap, s2_idx root) {
	u32 bucket = ITEM_BUCKET(root);
	u32 bucket_inv = INVERT_BUCKET(bucket);
	u32 left_parent_idx = ITEM_LEFT_IDX(root);
	u32 right_parent_idx = ITEM_RIGHT_IDX(root);
	s1_idx left_parent = STAGE1_IDX(bucket, left_parent_idx);
	s1_idx right_parent = STAGE1_IDX(bucket_inv, right_parent_idx);
	output[0] = left_parent;
	output[1] = right_parent;
	if (!tree_cmp1(&output[0], &output[1])) {
		SWAP_IDX(output[0], output[1]);
	}
}

static void build_solution_stage2(equix_idx* output, solver_heap* heap, s3_idx root) {
	u32 bucket = ITEM_BUCKET(root);
	u32 bucket_inv = INVERT_BUCKET(bucket);
	u32 left_parent_idx = ITEM_LEFT_IDX(root);
	u32 right_parent_idx = ITEM_RIGHT_IDX(root);
	s2_idx left_parent = STAGE2_IDX(bucket, left_parent_idx);
	s2_idx right_parent = STAGE2_IDX(bucket_inv, right_parent_idx);
	build_solution_stage1(&output[0], heap, left_parent);
	build_solution_stage1(&output[2], heap, right_parent);
	if (!tree_cmp2(&output[0], &output[2])) {
		SWAP_IDX(output[0], output[2]);
		SWAP_IDX(output[1], output[3]);
	}
}

static void build_solution(equix_solution* solution, solver_heap* heap, s3_idx left, s3_idx right) {
	build_solution_stage2(&solution->idx[0], heap, left);
	build_solution_stage2(&solution->idx[4], heap, right);
	if (!tree_cmp4(&solution->idx[0], &solution->idx[4])) {
		SWAP_IDX(solution->idx[0], solution->idx[4]);
		SWAP_IDX(solution->idx[1], solution->idx[5]);
		SWAP_IDX(solution->idx[2], solution->idx[6]);
		SWAP_IDX(solution->idx[3], solution->idx[7]);
	}
}

static void solve_stage0(hashx_ctx* hash_func, solver_heap* heap) {
	CLEAR(heap->stage1_indices.counts);
	for (u32 i = 0; i < INDEX_SPACE; ++i) {
		uint64_t value;
		if (!hash_value(hash_func, i, &value))
			break;
		u32 bucket_idx = value % NUM_COARSE_BUCKETS;
		u32 item_idx = STAGE1_SIZE(bucket_idx);
		if (item_idx >= COARSE_BUCKET_ITEMS)
			continue;
		STAGE1_SIZE(bucket_idx) = item_idx + 1;
		STAGE1_IDX(bucket_idx, item_idx) = i;
		STAGE1_DATA(bucket_idx, item_idx) = value / NUM_COARSE_BUCKETS; /* 52 bits */
	}
}

#define MAKE_PAIRS1                                                           \
    stage1_data_item value = STAGE1_DATA(bucket_idx, item_idx) + CARRY;       \
    u32 fine_buck_idx = value % NUM_FINE_BUCKETS;                             \
    u32 fine_cpl_bucket = INVERT_SCRATCH(fine_buck_idx);                      \
    u32 fine_cpl_size = SCRATCH_SIZE(fine_cpl_bucket);                        \
    for (u32 fine_idx = 0; fine_idx < fine_cpl_size; ++fine_idx) {            \
        u32 cpl_index = SCRATCH(fine_cpl_bucket, fine_idx);                   \
        stage1_data_item cpl_value = STAGE1_DATA(cpl_bucket, cpl_index);      \
        stage1_data_item sum = value + cpl_value;                             \
        assert((sum % NUM_FINE_BUCKETS) == 0);                                \
        sum /= NUM_FINE_BUCKETS; /* 45 bits */                                \
        u32 s2_buck_id = sum % NUM_COARSE_BUCKETS;                            \
        u32 s2_item_id = STAGE2_SIZE(s2_buck_id);                             \
        if (s2_item_id >= COARSE_BUCKET_ITEMS)                                \
            continue;                                                         \
        STAGE2_SIZE(s2_buck_id) = s2_item_id + 1;                             \
        STAGE2_IDX(s2_buck_id, s2_item_id) =                                  \
            MAKE_ITEM(bucket_idx, item_idx, cpl_index);                       \
        STAGE2_DATA(s2_buck_id, s2_item_id) =                                 \
            sum / NUM_COARSE_BUCKETS; /* 37 bits */                           \
    }                                                                         \

static void solve_stage1(solver_heap* heap) {
	CLEAR(heap->stage2_indices.counts);
	for (u32 bucket_idx = BUCK_START; bucket_idx < BUCK_END; ++bucket_idx) {
		u32 cpl_bucket = INVERT_BUCKET(bucket_idx);
		CLEAR(heap->scratch_ht.counts);
		u32 cpl_buck_size = STAGE1_SIZE(cpl_bucket);
		for (u32 item_idx = 0; item_idx < cpl_buck_size; ++item_idx) {
			{
				stage1_data_item value = STAGE1_DATA(cpl_bucket, item_idx);
				u32 fine_buck_idx = value % NUM_FINE_BUCKETS;
				u32 fine_item_idx = SCRATCH_SIZE(fine_buck_idx);
				if (fine_item_idx >= FINE_BUCKET_ITEMS)
					continue;
				SCRATCH_SIZE(fine_buck_idx) = fine_item_idx + 1;
				SCRATCH(fine_buck_idx, fine_item_idx) = item_idx;
			}
			if (cpl_bucket == bucket_idx) {
				MAKE_PAIRS1
			}
		}
		if (cpl_bucket != bucket_idx) {
			u32 buck_size = STAGE1_SIZE(bucket_idx);
			for (u32 item_idx = 0; item_idx < buck_size; ++item_idx) {
				MAKE_PAIRS1
			}
		}
	}
}

#define MAKE_PAIRS2                                                           \
    stage2_data_item value = STAGE2_DATA(bucket_idx, item_idx) + CARRY;       \
    u32 fine_buck_idx = value % NUM_FINE_BUCKETS;                             \
    u32 fine_cpl_bucket = INVERT_SCRATCH(fine_buck_idx);                      \
    u32 fine_cpl_size = SCRATCH_SIZE(fine_cpl_bucket);                        \
    for (u32 fine_idx = 0; fine_idx < fine_cpl_size; ++fine_idx) {            \
        u32 cpl_index = SCRATCH(fine_cpl_bucket, fine_idx);                   \
        stage2_data_item cpl_value = STAGE2_DATA(cpl_bucket, cpl_index);      \
        stage2_data_item sum = value + cpl_value;                             \
        assert((sum % NUM_FINE_BUCKETS) == 0);                                \
        sum /= NUM_FINE_BUCKETS; /* 30 bits */                                \
        u32 s3_buck_id = sum % NUM_COARSE_BUCKETS;                            \
        u32 s3_item_id = STAGE3_SIZE(s3_buck_id);                             \
        if (s3_item_id >= COARSE_BUCKET_ITEMS)                                \
            continue;                                                         \
        STAGE3_SIZE(s3_buck_id) = s3_item_id + 1;                             \
        STAGE3_IDX(s3_buck_id, s3_item_id) =                                  \
            MAKE_ITEM(bucket_idx, item_idx, cpl_index);                       \
        STAGE3_DATA(s3_buck_id, s3_item_id) =                                 \
            (stage3_data_item)(sum / NUM_COARSE_BUCKETS); /* 22 bits */       \
    }                                                                         \

static void solve_stage2(solver_heap* heap) {
	CLEAR(heap->stage3_indices.counts);
	for (u32 bucket_idx = BUCK_START; bucket_idx < BUCK_END; ++bucket_idx) {
		u32 cpl_bucket = INVERT_BUCKET(bucket_idx);
		CLEAR(heap->scratch_ht.counts);
		u32 cpl_buck_size = STAGE2_SIZE(cpl_bucket);
		for (u32 item_idx = 0; item_idx < cpl_buck_size; ++item_idx) {
			{
				stage2_data_item value = STAGE2_DATA(cpl_bucket, item_idx);
				u32 fine_buck_idx = value % NUM_FINE_BUCKETS;
				u32 fine_item_idx = SCRATCH_SIZE(fine_buck_idx);
				if (fine_item_idx >= FINE_BUCKET_ITEMS)
					continue;
				SCRATCH_SIZE(fine_buck_idx) = fine_item_idx + 1;
				SCRATCH(fine_buck_idx, fine_item_idx) = item_idx;
			}
			if (cpl_bucket == bucket_idx) {
				MAKE_PAIRS2
			}
		}
		if (cpl_bucket != bucket_idx) {
			u32 buck_size = STAGE2_SIZE(bucket_idx);
			for (u32 item_idx = 0; item_idx < buck_size; ++item_idx) {
				MAKE_PAIRS2
			}
		}
	}
}

#define MAKE_PAIRS3                                                           \
    stage3_data_item value = STAGE3_DATA(bucket_idx, item_idx) + CARRY;       \
    u32 fine_buck_idx = value % NUM_FINE_BUCKETS;                             \
    u32 fine_cpl_bucket = INVERT_SCRATCH(fine_buck_idx);                      \
    u32 fine_cpl_size = SCRATCH_SIZE(fine_cpl_bucket);                        \
    for (u32 fine_idx = 0; fine_idx < fine_cpl_size; ++fine_idx) {            \
        u32 cpl_index = SCRATCH(fine_cpl_bucket, fine_idx);                   \
        stage3_data_item cpl_value = STAGE3_DATA(cpl_bucket, cpl_index);      \
        stage3_data_item sum = value + cpl_value;                             \
        assert((sum % NUM_FINE_BUCKETS) == 0);                                \
        sum /= NUM_FINE_BUCKETS; /* 15 bits */                                \
        if ((sum & EQUIX_STAGE1_MASK) == 0) {                                 \
            /* we have a solution */                                          \
            s3_idx item_left = STAGE3_IDX(bucket_idx, item_idx);              \
            s3_idx item_right = STAGE3_IDX(cpl_bucket, cpl_index);            \
            build_solution(&output[sols_found], heap, item_left, item_right); \
            if (++(sols_found) >= EQUIX_MAX_SOLS) {                           \
                return sols_found;                                            \
            }                                                                 \
        }                                                                     \
    }                                                                         \

static int solve_stage3(solver_heap* heap, equix_solution output[EQUIX_MAX_SOLS]) {
	int sols_found = 0;

	for (u32 bucket_idx = BUCK_START; bucket_idx < BUCK_END; ++bucket_idx) {
		u32 cpl_bucket = -bucket_idx & (NUM_COARSE_BUCKETS - 1);
		CLEAR(heap->scratch_ht.counts);
		u32 cpl_buck_size = STAGE3_SIZE(cpl_bucket);
		for (u32 item_idx = 0; item_idx < cpl_buck_size; ++item_idx) {
			{
				stage3_data_item value = STAGE3_DATA(cpl_bucket, item_idx);
				u32 fine_buck_idx = value % NUM_FINE_BUCKETS;
				u32 fine_item_idx = SCRATCH_SIZE(fine_buck_idx);
				if (fine_item_idx >= FINE_BUCKET_ITEMS)
					continue;
				SCRATCH_SIZE(fine_buck_idx) = fine_item_idx + 1;
				SCRATCH(fine_buck_idx, fine_item_idx) = item_idx;
			}
			if (cpl_bucket == bucket_idx) {
				MAKE_PAIRS3
			}
		}
		if (cpl_bucket != bucket_idx) {
			u32 buck_size = STAGE3_SIZE(bucket_idx);
			for (u32 item_idx = 0; item_idx < buck_size; ++item_idx) {
				MAKE_PAIRS3
			}
		}
	}

	return sols_found;
}

int equix_solver_solve(
	hashx_ctx* hash_func,
	solver_heap* heap,
	equix_solution output[EQUIX_MAX_SOLS])
{
	solve_stage0(hash_func, heap);
	solve_stage1(heap);
	solve_stage2(heap);
	return solve_stage3(heap, output);
}
