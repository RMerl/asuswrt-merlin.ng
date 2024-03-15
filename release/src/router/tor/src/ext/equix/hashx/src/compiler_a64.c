/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include <string.h>
#include <assert.h>

#include "compiler.h"
#include "program.h"
#include "virtual_memory.h"
#include "unreachable.h"

#define EMIT(p,x) do {           \
        memcpy(p, x, sizeof(x)); \
        p += sizeof(x);          \
    } while (0)
#define EMIT_U32(p,x) *((uint32_t*)(p)) = x; p += sizeof(uint32_t)
#define EMIT_IMM32(p,x)                                              \
    EMIT_U32(p, 0x9280000c           |                               \
        ((x <= INT32_MAX) << 30)     |                               \
        (((x <= INT32_MAX) ? (x & 0xFFFF) : (~x & 0xFFFF)) << 5));   \
    EMIT_U32(p, 0xf2a0000c           |                               \
        (((x >> 16) & 0xFFFF) << 5));

#ifdef HASHX_COMPILER_A64

/* Largest compiled instruction (BRANCH) */
#define COMP_MAX_INSTR_SIZE 24

static const uint8_t a64_prologue[] = {
	0x07, 0x1c, 0x40, 0xf9, /* ldr x7, [x0, #56] */
	0x06, 0x18, 0x40, 0xf9, /* ldr x6, [x0, #48] */
	0x05, 0x14, 0x40, 0xf9, /* ldr x5, [x0, #40] */
	0x04, 0x10, 0x40, 0xf9, /* ldr x4, [x0, #32] */
	0x03, 0x0c, 0x40, 0xf9, /* ldr x3, [x0, #24] */
	0x02, 0x08, 0x40, 0xf9, /* ldr x2, [x0, #16] */
	0x01, 0x04, 0x40, 0xf9, /* ldr x1, [x0, #8]  */
	0xe8, 0x03, 0x00, 0xaa, /* mov x8, x0        */
	0x00, 0x00, 0x40, 0xf9, /* ldr x0, [x0]      */
	0xe9, 0x03, 0x1f, 0x2a, /* mov w9, wzr       */
};

static const uint8_t a64_epilogue[] = {
	0x00, 0x01, 0x00, 0xf9, /* str x0, [x8]      */
	0x01, 0x05, 0x00, 0xf9, /* str x1, [x8, #8]  */
	0x02, 0x09, 0x00, 0xf9, /* str x2, [x8, #16] */
	0x03, 0x0d, 0x00, 0xf9, /* str x3, [x8, #24] */
	0x04, 0x11, 0x00, 0xf9, /* str x4, [x8, #32] */
	0x05, 0x15, 0x00, 0xf9, /* str x5, [x8, #40] */
	0x06, 0x19, 0x00, 0xf9, /* str x6, [x8, #48] */
	0x07, 0x1d, 0x00, 0xf9, /* str x7, [x8, #56] */
	0xc0, 0x03, 0x5f, 0xd6, /* ret               */
};

bool hashx_compile_a64(const hashx_program* program, uint8_t* code) {
	if (!hashx_vm_rw(code, COMP_CODE_SIZE))
		return false;
	uint8_t* pos = code;
	uint8_t* target = NULL;
	int creg = -1;
	EMIT(pos, a64_prologue);
	for (size_t i = 0; i < program->code_size; ++i) {
		if (pos + COMP_MAX_INSTR_SIZE > code + COMP_CODE_SIZE)
			return false;
		const instruction* instr = &program->code[i];
		switch (instr->opcode)
		{
		case INSTR_UMULH_R:
			EMIT_U32(pos, 0x9bc07c00 |
				(instr->src << 16)   |
				(instr->dst << 5)    |
				(instr->dst));
			if (target != NULL) {
				creg = instr->dst;
			}
			break;
		case INSTR_SMULH_R:
			EMIT_U32(pos, 0x9b407c00 |
				(instr->src << 16)   |
				(instr->dst << 5)    |
				(instr->dst));
			if (target != NULL) {
				creg = instr->dst;
			}
			break;
		case INSTR_MUL_R:
			assert(creg != instr->dst);
			EMIT_U32(pos, 0x9b007c00 |
				(instr->src << 16)   |
				(instr->dst << 5)    |
				(instr->dst));
			break;
		case INSTR_SUB_R:
			assert(creg != instr->dst);
			EMIT_U32(pos, 0xcb000000 |
				(instr->src << 16)   |
				(instr->dst << 5)    |
				(instr->dst));
			break;
		case INSTR_XOR_R:
			assert(creg != instr->dst);
			EMIT_U32(pos, 0xca000000 |
				(instr->src << 16)   |
				(instr->dst << 5)    |
				(instr->dst));
			break;
		case INSTR_ADD_RS:
			assert(creg != instr->dst);
			EMIT_U32(pos, 0x8b000000 |
				(instr->src << 16)   |
				(instr->imm32 << 10) |
				(instr->dst << 5)    |
				(instr->dst));
			break;
		case INSTR_ROR_C:
			assert(creg != instr->dst);
			EMIT_U32(pos, 0x93c00000 |
				(instr->dst << 16)   |
				(instr->imm32 << 10) |
				(instr->dst << 5)    |
				(instr->dst));
			break;
		case INSTR_ADD_C:
			assert(creg != instr->dst);
			EMIT_IMM32(pos, instr->imm32);
			EMIT_U32(pos, 0x8b0c0000 |
				(instr->dst << 5) |
				(instr->dst));
			break;
		case INSTR_XOR_C:
			assert(creg != instr->dst);
			EMIT_IMM32(pos, instr->imm32);
			EMIT_U32(pos, 0xca0c0000 |
				(instr->dst << 5) |
				(instr->dst));
			break;
		case INSTR_TARGET:
			target = pos;
			break;
		case INSTR_BRANCH:
			EMIT_IMM32(pos, instr->imm32);
			EMIT_U32(pos, 0x2a00012b | (creg << 16));
			EMIT_U32(pos, 0x6a0c017f);
			EMIT_U32(pos, 0x5a891129);
			EMIT_U32(pos, 0x54000000 |
				((((uint32_t)(target - pos)) >> 2) & 0x7FFFF) << 5);
			target = NULL;
			creg = -1;
			break;
		default:
			UNREACHABLE;
		}
	}
	if (pos + sizeof a64_epilogue > code + COMP_CODE_SIZE)
		return false;
	EMIT(pos, a64_epilogue);
	if (!hashx_vm_rx(code, COMP_CODE_SIZE))
		return false;
#ifdef __GNUC__
	__builtin___clear_cache((void*)code, (void*)pos);
#endif
	return true;
}

#endif
