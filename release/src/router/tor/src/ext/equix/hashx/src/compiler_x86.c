/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include <string.h>

#include "compiler.h"
#include "program.h"
#include "virtual_memory.h"
#include "unreachable.h"

#if defined(_WIN32) || defined(__CYGWIN__)
#define WINABI
#endif

#define EMIT(p,x) do {           \
		memcpy(p, x, sizeof(x)); \
		p += sizeof(x);          \
	} while (0)
#define EMIT_BYTE(p,x) *((p)++) = x
#define EMIT_U16(p,x) *((uint16_t*)(p)) = x; p += sizeof(uint16_t)
#define EMIT_U32(p,x) *((uint32_t*)(p)) = x; p += sizeof(uint32_t)
#define EMIT_U64(p,x) *((uint64_t*)(p)) = x; p += sizeof(uint64_t)

#define GEN_SIB(scale, index, base) ((scale << 6) | (index << 3) | base)

#ifdef HASHX_COMPILER_X86

/* Largest compiled instruction (BRANCH) */
#define COMP_MAX_INSTR_SIZE 10

static const uint8_t x86_prologue[] = {
#ifndef WINABI
	0x48, 0x89, 0xF9,             /* mov rcx, rdi */
	0x48, 0x83, 0xEC, 0x20,       /* sub rsp, 32 */
	0x4C, 0x89, 0x24, 0x24,       /* mov qword ptr [rsp+0], r12 */
	0x4C, 0x89, 0x6C, 0x24, 0x08, /* mov qword ptr [rsp+8], r13 */
	0x4C, 0x89, 0x74, 0x24, 0x10, /* mov qword ptr [rsp+16], r14 */
	0x4C, 0x89, 0x7C, 0x24, 0x18, /* mov qword ptr [rsp+24], r15 */
#else
	0x4C, 0x89, 0x64, 0x24, 0x08, /* mov qword ptr [rsp+8], r12 */
	0x4C, 0x89, 0x6C, 0x24, 0x10, /* mov qword ptr [rsp+16], r13 */
	0x4C, 0x89, 0x74, 0x24, 0x18, /* mov qword ptr [rsp+24], r14 */
	0x4C, 0x89, 0x7C, 0x24, 0x20, /* mov qword ptr [rsp+32], r15 */
	0x48, 0x83, 0xEC, 0x10,       /* sub rsp, 16 */
	0x48, 0x89, 0x34, 0x24,       /* mov qword ptr [rsp+0], rsi */
	0x48, 0x89, 0x7C, 0x24, 0x08, /* mov qword ptr [rsp+8], rdi */
#endif
	0x31, 0xF6,                   /* xor esi, esi */
	0x8D, 0x7E, 0xFF,             /* lea edi, [rsi-1] */
	0x4C, 0x8B, 0x01,             /* mov r8, qword ptr [rcx+0] */
	0x4C, 0x8B, 0x49, 0x08,       /* mov r9, qword ptr [rcx+8] */
	0x4C, 0x8B, 0x51, 0x10,       /* mov r10, qword ptr [rcx+16] */
	0x4C, 0x8B, 0x59, 0x18,       /* mov r11, qword ptr [rcx+24] */
	0x4C, 0x8B, 0x61, 0x20,       /* mov r12, qword ptr [rcx+32] */
	0x4C, 0x8B, 0x69, 0x28,       /* mov r13, qword ptr [rcx+40] */
	0x4C, 0x8B, 0x71, 0x30,       /* mov r14, qword ptr [rcx+48] */
	0x4C, 0x8B, 0x79, 0x38        /* mov r15, qword ptr [rcx+56] */
};

static const uint8_t x86_epilogue[] = {
	0x4C, 0x89, 0x01,             /* mov qword ptr [rcx+0], r8 */
	0x4C, 0x89, 0x49, 0x08,       /* mov qword ptr [rcx+8], r9 */
	0x4C, 0x89, 0x51, 0x10,       /* mov qword ptr [rcx+16], r10 */
	0x4C, 0x89, 0x59, 0x18,       /* mov qword ptr [rcx+24], r11 */
	0x4C, 0x89, 0x61, 0x20,       /* mov qword ptr [rcx+32], r12 */
	0x4C, 0x89, 0x69, 0x28,       /* mov qword ptr [rcx+40], r13 */
	0x4C, 0x89, 0x71, 0x30,       /* mov qword ptr [rcx+48], r14 */
	0x4C, 0x89, 0x79, 0x38,       /* mov qword ptr [rcx+56], r15 */
#ifndef WINABI
	0x4C, 0x8B, 0x24, 0x24,       /* mov r12, qword ptr [rsp+0] */
	0x4C, 0x8B, 0x6C, 0x24, 0x08, /* mov r13, qword ptr [rsp+8] */
	0x4C, 0x8B, 0x74, 0x24, 0x10, /* mov r14, qword ptr [rsp+16] */
	0x4C, 0x8B, 0x7C, 0x24, 0x18, /* mov r15, qword ptr [rsp+24] */
	0x48, 0x83, 0xC4, 0x20,       /* add rsp, 32 */
#else
	0x48, 0x8B, 0x34, 0x24,       /* mov rsi, qword ptr [rsp+0] */
	0x48, 0x8B, 0x7C, 0x24, 0x08, /* mov rdi, qword ptr [rsp+8] */
	0x48, 0x83, 0xC4, 0x10,       /* add rsp, 16 */
	0x4C, 0x8B, 0x64, 0x24, 0x08, /* mov r12, qword ptr [rsp+8] */
	0x4C, 0x8B, 0x6C, 0x24, 0x10, /* mov r13, qword ptr [rsp+16] */
	0x4C, 0x8B, 0x74, 0x24, 0x18, /* mov r14, qword ptr [rsp+24] */
	0x4C, 0x8B, 0x7C, 0x24, 0x20, /* mov r15, qword ptr [rsp+32] */
#endif
	0xC3                          /* ret */
};

bool hashx_compile_x86(const hashx_program* program, uint8_t* code) {
	if (!hashx_vm_rw(code, COMP_CODE_SIZE))
		return false;
	uint8_t* pos = code;
	uint8_t* target = NULL;
	EMIT(pos, x86_prologue);
	for (size_t i = 0; i < program->code_size; ++i) {
		if (pos + COMP_MAX_INSTR_SIZE > code + COMP_CODE_SIZE)
			return false;
		const instruction* instr = &program->code[i];
		switch (instr->opcode)
		{
		case INSTR_UMULH_R:
			EMIT_U64(pos, 0x8b4ce0f749c08b49 |
				(((uint64_t)instr->src) << 40) |
				(((uint64_t)instr->dst) << 16));
			EMIT_BYTE(pos, 0xc2 + 8 * instr->dst);
			break;
		case INSTR_SMULH_R:
			EMIT_U64(pos, 0x8b4ce8f749c08b49 |
				(((uint64_t)instr->src) << 40) |
				(((uint64_t)instr->dst) << 16));
			EMIT_BYTE(pos, 0xc2 + 8 * instr->dst);
			break;
		case INSTR_MUL_R:
			EMIT_U32(pos, 0xc0af0f4d | (instr->dst << 27) | (instr->src << 24));
			break;
		case INSTR_SUB_R:
			EMIT_U16(pos, 0x2b4d);
			EMIT_BYTE(pos, 0xc0 | (instr->dst << 3) | instr->src);
			break;
		case INSTR_XOR_R:
			EMIT_U16(pos, 0x334d);
			EMIT_BYTE(pos, 0xc0 | (instr->dst << 3) | instr->src);
			break;
		case INSTR_ADD_RS:
			EMIT_U32(pos, 0x00048d4f |
				(instr->dst << 19) |
				GEN_SIB(instr->imm32, instr->src, instr->dst) << 24);
			break;
		case INSTR_ROR_C:
			EMIT_U32(pos, 0x00c8c149 | (instr->dst << 16) | (instr->imm32 << 24));
			break;
		case INSTR_ADD_C:
			EMIT_U16(pos, 0x8149);
			EMIT_BYTE(pos, 0xc0 | instr->dst);
			EMIT_U32(pos, instr->imm32);
			break;
		case INSTR_XOR_C:
			EMIT_U16(pos, 0x8149);
			EMIT_BYTE(pos, 0xf0 | instr->dst);
			EMIT_U32(pos, instr->imm32);
			break;
		case INSTR_TARGET:
			target = pos; /* +2 */
			EMIT_U32(pos, 0x440fff85);
			EMIT_BYTE(pos, 0xf7);
			break;
		case INSTR_BRANCH:
			EMIT_U64(pos, ((uint64_t)instr->imm32) << 32 | 0xc2f7f209);
			EMIT_U16(pos, ((target - pos) << 8) | 0x74);
			break;
		default:
			UNREACHABLE;
		}
	}
	if (pos + sizeof x86_epilogue > code + COMP_CODE_SIZE)
		return false;
	EMIT(pos, x86_epilogue);
	return hashx_vm_rx(code, COMP_CODE_SIZE);
}

#endif
