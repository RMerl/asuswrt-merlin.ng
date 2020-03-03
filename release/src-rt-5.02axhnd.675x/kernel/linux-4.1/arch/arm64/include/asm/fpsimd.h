/*
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ASM_FP_H
#define __ASM_FP_H

#include <asm/ptrace.h>

#ifndef __ASSEMBLY__

/*
 * FP/SIMD storage area has:
 *  - FPSR and FPCR
 *  - 32 128-bit data registers
 *
 * Note that user_fpsimd forms a prefix of this structure, which is
 * relied upon in the ptrace FP/SIMD accessors.
 */
struct fpsimd_state {
	union {
		struct user_fpsimd_state user_fpsimd;
		struct {
			__uint128_t vregs[32];
			u32 fpsr;
			u32 fpcr;
		};
	};
	/* the id of the last cpu to have restored this state */
	unsigned int cpu;
};

/*
 * Struct for stacking the bottom 'n' FP/SIMD registers.
 */
struct fpsimd_partial_state {
	u32		fpsr;
	u32		fpcr;
	u32		num_regs;
	__uint128_t	vregs[32];
};


#if defined(__KERNEL__) && defined(CONFIG_COMPAT)
/* Masks for extracting the FPSR and FPCR from the FPSCR */
#define VFP_FPSCR_STAT_MASK	0xf800009f
#define VFP_FPSCR_CTRL_MASK	0x07f79f00
/*
 * The VFP state has 32x64-bit registers and a single 32-bit
 * control/status register.
 */
#define VFP_STATE_SIZE		((32 * 8) + 4)
#endif

struct task_struct;

extern void fpsimd_save_state(struct fpsimd_state *state);
extern void fpsimd_load_state(struct fpsimd_state *state);

extern void fpsimd_thread_switch(struct task_struct *next);
extern void fpsimd_flush_thread(void);

extern void fpsimd_preserve_current_state(void);
extern void fpsimd_restore_current_state(void);
extern void fpsimd_update_current_state(struct fpsimd_state *state);

extern void fpsimd_flush_task_state(struct task_struct *target);

extern void fpsimd_save_partial_state(struct fpsimd_partial_state *state,
				      u32 num_regs);
extern void fpsimd_load_partial_state(struct fpsimd_partial_state *state);

#endif

#endif
