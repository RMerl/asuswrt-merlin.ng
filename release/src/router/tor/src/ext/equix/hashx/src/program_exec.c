/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include "program.h"
#include "force_inline.h"
#include "unreachable.h"
#include "siphash.h"
#include "hashx_endian.h"

#if defined(__SIZEOF_INT128__)
typedef unsigned __int128 uint128_t;
typedef __int128 int128_t;
static FORCE_INLINE uint64_t umulh(uint64_t a, uint64_t b) {
	return ((uint128_t)a * b) >> 64;
	}
static FORCE_INLINE int64_t smulh(int64_t a, int64_t b) {
	return ((int128_t)a * b) >> 64;
}
#define HAVE_UMULH
#define HAVE_SMULH
#endif

#if defined(_MSC_VER)
#pragma warning (disable : 4146) /* unary minus applied to unsigned type */
#define HAS_VALUE(X) X ## 0
#define EVAL_DEFINE(X) HAS_VALUE(X)
#include <intrin.h>
#include <stdlib.h>

static FORCE_INLINE uint64_t rotr64(uint64_t x, unsigned int c) {
	return _rotr64(x, c);
}

#define HAVE_ROTR

#if EVAL_DEFINE(__MACHINEARM64_X64(1))
static FORCE_INLINE uint64_t umulh(uint64_t a, uint64_t b) {
	return __umulh(a, b);
}
#define HAVE_UMULH
#endif

#if EVAL_DEFINE(__MACHINEX64(1))
static FORCE_INLINE int64_t smulh(int64_t a, int64_t b) {
	int64_t hi;
	_mul128(a, b, &hi);
	return hi;
}
#define HAVE_SMULH
#endif

#endif

#ifndef HAVE_ROTR
static FORCE_INLINE uint64_t rotr64(uint64_t a, unsigned int b) {
	return (a >> b) | (a << (64 - b));
}
#define HAVE_ROTR
#endif

#ifndef HAVE_UMULH
#define LO(x) ((x)&0xffffffff)
#define HI(x) ((x)>>32)
static uint64_t umulh(uint64_t a, uint64_t b) {
	uint64_t ah = HI(a), al = LO(a);
	uint64_t bh = HI(b), bl = LO(b);
	uint64_t x00 = al * bl;
	uint64_t x01 = al * bh;
	uint64_t x10 = ah * bl;
	uint64_t x11 = ah * bh;
	uint64_t m1 = LO(x10) + LO(x01) + HI(x00);
	uint64_t m2 = HI(x10) + HI(x01) + LO(x11) + HI(m1);
	uint64_t m3 = HI(x11) + HI(m2);

	return (m3 << 32) + LO(m2);
}
#undef LO
#undef HI
#define HAVE_UMULH
#endif

#ifndef HAVE_SMULH
static int64_t smulh(int64_t a, int64_t b) {
	int64_t hi = umulh(a, b);
	if (a < 0LL) hi -= b;
	if (b < 0LL) hi -= a;
	return hi;
}
#define HAVE_SMULH
#endif

static FORCE_INLINE uint64_t sign_extend_2s_compl(uint32_t x) {
	return (-1 == ~0) ?
		(uint64_t)(int64_t)(int32_t)(x) :
		(x > INT32_MAX ? (x | 0xffffffff00000000ULL) : (uint64_t)x);
}

void hashx_program_execute(const hashx_program* program, uint64_t r[8]) {
	size_t target = 0;
	bool branch_enable = true;
	uint32_t result = 0;
#ifdef HASHX_PROGRAM_STATS
	int branch_idx = 0;
#endif
	for (size_t i = 0; i < program->code_size; ++i) {
		const instruction* instr = &program->code[i];
		switch (instr->opcode)
		{
		case INSTR_UMULH_R:
			result = (uint32_t) (r[instr->dst] = umulh(r[instr->dst],
			                                           r[instr->src]));
			break;
		case INSTR_SMULH_R:
			result = (uint32_t) (r[instr->dst] = smulh(r[instr->dst],
			                                           r[instr->src]));
			break;
		case INSTR_MUL_R:
			r[instr->dst] *= r[instr->src];
			break;
		case INSTR_SUB_R:
			r[instr->dst] -= r[instr->src];
			break;
		case INSTR_XOR_R:
			r[instr->dst] ^= r[instr->src];
			break;
		case INSTR_ADD_RS:
			r[instr->dst] += r[instr->src] << instr->imm32;
			break;
		case INSTR_ROR_C:
			r[instr->dst] = rotr64(r[instr->dst], instr->imm32);
			break;
		case INSTR_ADD_C:
			r[instr->dst] += sign_extend_2s_compl(instr->imm32);
			break;
		case INSTR_XOR_C:
			r[instr->dst] ^= sign_extend_2s_compl(instr->imm32);
			break;
		case INSTR_TARGET:
			target = i;
			break;
		case INSTR_BRANCH:
			if (branch_enable && (result & instr->imm32) == 0) {
				i = target;
				branch_enable = false;
#ifdef HASHX_PROGRAM_STATS
				((hashx_program*)program)->branch_count++;
				((hashx_program*)program)->branches[branch_idx]++;
#endif
			}
#ifdef HASHX_PROGRAM_STATS
			branch_idx++;
#endif
			break;
		default:
			UNREACHABLE;
		}
	}
}
