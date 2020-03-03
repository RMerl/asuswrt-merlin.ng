#ifndef ARCH_PERF_REGS_H
#define ARCH_PERF_REGS_H

#include <stdlib.h>
#include <linux/types.h>
#include <asm/perf_regs.h>

void perf_regs_load(u64 *regs);

#ifndef HAVE_ARCH_X86_64_SUPPORT
#define PERF_REGS_MASK ((1ULL << PERF_REG_X86_32_MAX) - 1)
#define PERF_REGS_MAX PERF_REG_X86_32_MAX
#define PERF_SAMPLE_REGS_ABI PERF_SAMPLE_REGS_ABI_32
#else
#define REG_NOSUPPORT ((1ULL << PERF_REG_X86_DS) | \
		       (1ULL << PERF_REG_X86_ES) | \
		       (1ULL << PERF_REG_X86_FS) | \
		       (1ULL << PERF_REG_X86_GS))
#define PERF_REGS_MASK (((1ULL << PERF_REG_X86_64_MAX) - 1) & ~REG_NOSUPPORT)
#define PERF_REGS_MAX PERF_REG_X86_64_MAX
#define PERF_SAMPLE_REGS_ABI PERF_SAMPLE_REGS_ABI_64
#endif
#define PERF_REG_IP PERF_REG_X86_IP
#define PERF_REG_SP PERF_REG_X86_SP

static inline const char *perf_reg_name(int id)
{
	switch (id) {
	case PERF_REG_X86_AX:
		return "AX";
	case PERF_REG_X86_BX:
		return "BX";
	case PERF_REG_X86_CX:
		return "CX";
	case PERF_REG_X86_DX:
		return "DX";
	case PERF_REG_X86_SI:
		return "SI";
	case PERF_REG_X86_DI:
		return "DI";
	case PERF_REG_X86_BP:
		return "BP";
	case PERF_REG_X86_SP:
		return "SP";
	case PERF_REG_X86_IP:
		return "IP";
	case PERF_REG_X86_FLAGS:
		return "FLAGS";
	case PERF_REG_X86_CS:
		return "CS";
	case PERF_REG_X86_SS:
		return "SS";
	case PERF_REG_X86_DS:
		return "DS";
	case PERF_REG_X86_ES:
		return "ES";
	case PERF_REG_X86_FS:
		return "FS";
	case PERF_REG_X86_GS:
		return "GS";
#ifdef HAVE_ARCH_X86_64_SUPPORT
	case PERF_REG_X86_R8:
		return "R8";
	case PERF_REG_X86_R9:
		return "R9";
	case PERF_REG_X86_R10:
		return "R10";
	case PERF_REG_X86_R11:
		return "R11";
	case PERF_REG_X86_R12:
		return "R12";
	case PERF_REG_X86_R13:
		return "R13";
	case PERF_REG_X86_R14:
		return "R14";
	case PERF_REG_X86_R15:
		return "R15";
#endif /* HAVE_ARCH_X86_64_SUPPORT */
	default:
		return NULL;
	}

	return NULL;
}

#endif /* ARCH_PERF_REGS_H */
