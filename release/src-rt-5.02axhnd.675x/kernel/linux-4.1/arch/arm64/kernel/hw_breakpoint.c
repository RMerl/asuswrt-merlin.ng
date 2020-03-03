/*
 * HW_breakpoint: a unified kernel/user-space hardware breakpoint facility,
 * using the CPU's debug registers.
 *
 * Copyright (C) 2012 ARM Limited
 * Author: Will Deacon <will.deacon@arm.com>
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

#define pr_fmt(fmt) "hw-breakpoint: " fmt

#include <linux/compat.h>
#include <linux/cpu_pm.h>
#include <linux/errno.h>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <linux/ptrace.h>
#include <linux/smp.h>

#include <asm/current.h>
#include <asm/debug-monitors.h>
#include <asm/hw_breakpoint.h>
#include <asm/kdebug.h>
#include <asm/traps.h>
#include <asm/cputype.h>
#include <asm/system_misc.h>
#include <asm/uaccess.h>

/* Breakpoint currently in use for each BRP. */
static DEFINE_PER_CPU(struct perf_event *, bp_on_reg[ARM_MAX_BRP]);

/* Watchpoint currently in use for each WRP. */
static DEFINE_PER_CPU(struct perf_event *, wp_on_reg[ARM_MAX_WRP]);

/* Currently stepping a per-CPU kernel breakpoint. */
static DEFINE_PER_CPU(int, stepping_kernel_bp);

/* Number of BRP/WRP registers on this CPU. */
static int core_num_brps;
static int core_num_wrps;

/* Determine number of BRP registers available. */
static int get_num_brps(void)
{
	return ((read_cpuid(ID_AA64DFR0_EL1) >> 12) & 0xf) + 1;
}

/* Determine number of WRP registers available. */
static int get_num_wrps(void)
{
	return ((read_cpuid(ID_AA64DFR0_EL1) >> 20) & 0xf) + 1;
}

int hw_breakpoint_slots(int type)
{
	/*
	 * We can be called early, so don't rely on
	 * our static variables being initialised.
	 */
	switch (type) {
	case TYPE_INST:
		return get_num_brps();
	case TYPE_DATA:
		return get_num_wrps();
	default:
		pr_warning("unknown slot type: %d\n", type);
		return 0;
	}
}

#define READ_WB_REG_CASE(OFF, N, REG, VAL)	\
	case (OFF + N):				\
		AARCH64_DBG_READ(N, REG, VAL);	\
		break

#define WRITE_WB_REG_CASE(OFF, N, REG, VAL)	\
	case (OFF + N):				\
		AARCH64_DBG_WRITE(N, REG, VAL);	\
		break

#define GEN_READ_WB_REG_CASES(OFF, REG, VAL)	\
	READ_WB_REG_CASE(OFF,  0, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  1, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  2, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  3, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  4, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  5, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  6, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  7, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  8, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  9, REG, VAL);	\
	READ_WB_REG_CASE(OFF, 10, REG, VAL);	\
	READ_WB_REG_CASE(OFF, 11, REG, VAL);	\
	READ_WB_REG_CASE(OFF, 12, REG, VAL);	\
	READ_WB_REG_CASE(OFF, 13, REG, VAL);	\
	READ_WB_REG_CASE(OFF, 14, REG, VAL);	\
	READ_WB_REG_CASE(OFF, 15, REG, VAL)

#define GEN_WRITE_WB_REG_CASES(OFF, REG, VAL)	\
	WRITE_WB_REG_CASE(OFF,  0, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF,  1, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF,  2, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF,  3, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF,  4, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF,  5, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF,  6, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF,  7, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF,  8, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF,  9, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF, 10, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF, 11, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF, 12, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF, 13, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF, 14, REG, VAL);	\
	WRITE_WB_REG_CASE(OFF, 15, REG, VAL)

static u64 read_wb_reg(int reg, int n)
{
	u64 val = 0;

	switch (reg + n) {
	GEN_READ_WB_REG_CASES(AARCH64_DBG_REG_BVR, AARCH64_DBG_REG_NAME_BVR, val);
	GEN_READ_WB_REG_CASES(AARCH64_DBG_REG_BCR, AARCH64_DBG_REG_NAME_BCR, val);
	GEN_READ_WB_REG_CASES(AARCH64_DBG_REG_WVR, AARCH64_DBG_REG_NAME_WVR, val);
	GEN_READ_WB_REG_CASES(AARCH64_DBG_REG_WCR, AARCH64_DBG_REG_NAME_WCR, val);
	default:
		pr_warning("attempt to read from unknown breakpoint register %d\n", n);
	}

	return val;
}

static void write_wb_reg(int reg, int n, u64 val)
{
	switch (reg + n) {
	GEN_WRITE_WB_REG_CASES(AARCH64_DBG_REG_BVR, AARCH64_DBG_REG_NAME_BVR, val);
	GEN_WRITE_WB_REG_CASES(AARCH64_DBG_REG_BCR, AARCH64_DBG_REG_NAME_BCR, val);
	GEN_WRITE_WB_REG_CASES(AARCH64_DBG_REG_WVR, AARCH64_DBG_REG_NAME_WVR, val);
	GEN_WRITE_WB_REG_CASES(AARCH64_DBG_REG_WCR, AARCH64_DBG_REG_NAME_WCR, val);
	default:
		pr_warning("attempt to write to unknown breakpoint register %d\n", n);
	}
	isb();
}

/*
 * Convert a breakpoint privilege level to the corresponding exception
 * level.
 */
static enum debug_el debug_exception_level(int privilege)
{
	switch (privilege) {
	case AARCH64_BREAKPOINT_EL0:
		return DBG_ACTIVE_EL0;
	case AARCH64_BREAKPOINT_EL1:
		return DBG_ACTIVE_EL1;
	default:
		pr_warning("invalid breakpoint privilege level %d\n", privilege);
		return -EINVAL;
	}
}

enum hw_breakpoint_ops {
	HW_BREAKPOINT_INSTALL,
	HW_BREAKPOINT_UNINSTALL,
	HW_BREAKPOINT_RESTORE
};

/**
 * hw_breakpoint_slot_setup - Find and setup a perf slot according to
 *			      operations
 *
 * @slots: pointer to array of slots
 * @max_slots: max number of slots
 * @bp: perf_event to setup
 * @ops: operation to be carried out on the slot
 *
 * Return:
 *	slot index on success
 *	-ENOSPC if no slot is available/matches
 *	-EINVAL on wrong operations parameter
 */
static int hw_breakpoint_slot_setup(struct perf_event **slots, int max_slots,
				    struct perf_event *bp,
				    enum hw_breakpoint_ops ops)
{
	int i;
	struct perf_event **slot;

	for (i = 0; i < max_slots; ++i) {
		slot = &slots[i];
		switch (ops) {
		case HW_BREAKPOINT_INSTALL:
			if (!*slot) {
				*slot = bp;
				return i;
			}
			break;
		case HW_BREAKPOINT_UNINSTALL:
			if (*slot == bp) {
				*slot = NULL;
				return i;
			}
			break;
		case HW_BREAKPOINT_RESTORE:
			if (*slot == bp)
				return i;
			break;
		default:
			pr_warn_once("Unhandled hw breakpoint ops %d\n", ops);
			return -EINVAL;
		}
	}
	return -ENOSPC;
}

static int hw_breakpoint_control(struct perf_event *bp,
				 enum hw_breakpoint_ops ops)
{
	struct arch_hw_breakpoint *info = counter_arch_bp(bp);
	struct perf_event **slots;
	struct debug_info *debug_info = &current->thread.debug;
	int i, max_slots, ctrl_reg, val_reg, reg_enable;
	enum debug_el dbg_el = debug_exception_level(info->ctrl.privilege);
	u32 ctrl;

	if (info->ctrl.type == ARM_BREAKPOINT_EXECUTE) {
		/* Breakpoint */
		ctrl_reg = AARCH64_DBG_REG_BCR;
		val_reg = AARCH64_DBG_REG_BVR;
		slots = this_cpu_ptr(bp_on_reg);
		max_slots = core_num_brps;
		reg_enable = !debug_info->bps_disabled;
	} else {
		/* Watchpoint */
		ctrl_reg = AARCH64_DBG_REG_WCR;
		val_reg = AARCH64_DBG_REG_WVR;
		slots = this_cpu_ptr(wp_on_reg);
		max_slots = core_num_wrps;
		reg_enable = !debug_info->wps_disabled;
	}

	i = hw_breakpoint_slot_setup(slots, max_slots, bp, ops);

	if (WARN_ONCE(i < 0, "Can't find any breakpoint slot"))
		return i;

	switch (ops) {
	case HW_BREAKPOINT_INSTALL:
		/*
		 * Ensure debug monitors are enabled at the correct exception
		 * level.
		 */
		enable_debug_monitors(dbg_el);
		/* Fall through */
	case HW_BREAKPOINT_RESTORE:
		/* Setup the address register. */
		write_wb_reg(val_reg, i, info->address);

		/* Setup the control register. */
		ctrl = encode_ctrl_reg(info->ctrl);
		write_wb_reg(ctrl_reg, i,
			     reg_enable ? ctrl | 0x1 : ctrl & ~0x1);
		break;
	case HW_BREAKPOINT_UNINSTALL:
		/* Reset the control register. */
		write_wb_reg(ctrl_reg, i, 0);

		/*
		 * Release the debug monitors for the correct exception
		 * level.
		 */
		disable_debug_monitors(dbg_el);
		break;
	}

	return 0;
}

/*
 * Install a perf counter breakpoint.
 */
int arch_install_hw_breakpoint(struct perf_event *bp)
{
	return hw_breakpoint_control(bp, HW_BREAKPOINT_INSTALL);
}

void arch_uninstall_hw_breakpoint(struct perf_event *bp)
{
	hw_breakpoint_control(bp, HW_BREAKPOINT_UNINSTALL);
}

static int get_hbp_len(u8 hbp_len)
{
	unsigned int len_in_bytes = 0;

	switch (hbp_len) {
	case ARM_BREAKPOINT_LEN_1:
		len_in_bytes = 1;
		break;
	case ARM_BREAKPOINT_LEN_2:
		len_in_bytes = 2;
		break;
	case ARM_BREAKPOINT_LEN_4:
		len_in_bytes = 4;
		break;
	case ARM_BREAKPOINT_LEN_8:
		len_in_bytes = 8;
		break;
	}

	return len_in_bytes;
}

/*
 * Check whether bp virtual address is in kernel space.
 */
int arch_check_bp_in_kernelspace(struct perf_event *bp)
{
	unsigned int len;
	unsigned long va;
	struct arch_hw_breakpoint *info = counter_arch_bp(bp);

	va = info->address;
	len = get_hbp_len(info->ctrl.len);

	return (va >= TASK_SIZE) && ((va + len - 1) >= TASK_SIZE);
}

/*
 * Extract generic type and length encodings from an arch_hw_breakpoint_ctrl.
 * Hopefully this will disappear when ptrace can bypass the conversion
 * to generic breakpoint descriptions.
 */
int arch_bp_generic_fields(struct arch_hw_breakpoint_ctrl ctrl,
			   int *gen_len, int *gen_type)
{
	/* Type */
	switch (ctrl.type) {
	case ARM_BREAKPOINT_EXECUTE:
		*gen_type = HW_BREAKPOINT_X;
		break;
	case ARM_BREAKPOINT_LOAD:
		*gen_type = HW_BREAKPOINT_R;
		break;
	case ARM_BREAKPOINT_STORE:
		*gen_type = HW_BREAKPOINT_W;
		break;
	case ARM_BREAKPOINT_LOAD | ARM_BREAKPOINT_STORE:
		*gen_type = HW_BREAKPOINT_RW;
		break;
	default:
		return -EINVAL;
	}

	/* Len */
	switch (ctrl.len) {
	case ARM_BREAKPOINT_LEN_1:
		*gen_len = HW_BREAKPOINT_LEN_1;
		break;
	case ARM_BREAKPOINT_LEN_2:
		*gen_len = HW_BREAKPOINT_LEN_2;
		break;
	case ARM_BREAKPOINT_LEN_4:
		*gen_len = HW_BREAKPOINT_LEN_4;
		break;
	case ARM_BREAKPOINT_LEN_8:
		*gen_len = HW_BREAKPOINT_LEN_8;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/*
 * Construct an arch_hw_breakpoint from a perf_event.
 */
static int arch_build_bp_info(struct perf_event *bp)
{
	struct arch_hw_breakpoint *info = counter_arch_bp(bp);

	/* Type */
	switch (bp->attr.bp_type) {
	case HW_BREAKPOINT_X:
		info->ctrl.type = ARM_BREAKPOINT_EXECUTE;
		break;
	case HW_BREAKPOINT_R:
		info->ctrl.type = ARM_BREAKPOINT_LOAD;
		break;
	case HW_BREAKPOINT_W:
		info->ctrl.type = ARM_BREAKPOINT_STORE;
		break;
	case HW_BREAKPOINT_RW:
		info->ctrl.type = ARM_BREAKPOINT_LOAD | ARM_BREAKPOINT_STORE;
		break;
	default:
		return -EINVAL;
	}

	/* Len */
	switch (bp->attr.bp_len) {
	case HW_BREAKPOINT_LEN_1:
		info->ctrl.len = ARM_BREAKPOINT_LEN_1;
		break;
	case HW_BREAKPOINT_LEN_2:
		info->ctrl.len = ARM_BREAKPOINT_LEN_2;
		break;
	case HW_BREAKPOINT_LEN_4:
		info->ctrl.len = ARM_BREAKPOINT_LEN_4;
		break;
	case HW_BREAKPOINT_LEN_8:
		info->ctrl.len = ARM_BREAKPOINT_LEN_8;
		break;
	default:
		return -EINVAL;
	}

	/*
	 * On AArch64, we only permit breakpoints of length 4, whereas
	 * AArch32 also requires breakpoints of length 2 for Thumb.
	 * Watchpoints can be of length 1, 2, 4 or 8 bytes.
	 */
	if (info->ctrl.type == ARM_BREAKPOINT_EXECUTE) {
		if (is_compat_task()) {
			if (info->ctrl.len != ARM_BREAKPOINT_LEN_2 &&
			    info->ctrl.len != ARM_BREAKPOINT_LEN_4)
				return -EINVAL;
		} else if (info->ctrl.len != ARM_BREAKPOINT_LEN_4) {
			/*
			 * FIXME: Some tools (I'm looking at you perf) assume
			 *	  that breakpoints should be sizeof(long). This
			 *	  is nonsense. For now, we fix up the parameter
			 *	  but we should probably return -EINVAL instead.
			 */
			info->ctrl.len = ARM_BREAKPOINT_LEN_4;
		}
	}

	/* Address */
	info->address = bp->attr.bp_addr;

	/*
	 * Privilege
	 * Note that we disallow combined EL0/EL1 breakpoints because
	 * that would complicate the stepping code.
	 */
	if (arch_check_bp_in_kernelspace(bp))
		info->ctrl.privilege = AARCH64_BREAKPOINT_EL1;
	else
		info->ctrl.privilege = AARCH64_BREAKPOINT_EL0;

	/* Enabled? */
	info->ctrl.enabled = !bp->attr.disabled;

	return 0;
}

/*
 * Validate the arch-specific HW Breakpoint register settings.
 */
int arch_validate_hwbkpt_settings(struct perf_event *bp)
{
	struct arch_hw_breakpoint *info = counter_arch_bp(bp);
	int ret;
	u64 alignment_mask, offset;

	/* Build the arch_hw_breakpoint. */
	ret = arch_build_bp_info(bp);
	if (ret)
		return ret;

	/*
	 * Check address alignment.
	 * We don't do any clever alignment correction for watchpoints
	 * because using 64-bit unaligned addresses is deprecated for
	 * AArch64.
	 *
	 * AArch32 tasks expect some simple alignment fixups, so emulate
	 * that here.
	 */
	if (is_compat_task()) {
		if (info->ctrl.len == ARM_BREAKPOINT_LEN_8)
			alignment_mask = 0x7;
		else
			alignment_mask = 0x3;
		offset = info->address & alignment_mask;
		switch (offset) {
		case 0:
			/* Aligned */
			break;
		case 1:
			/* Allow single byte watchpoint. */
			if (info->ctrl.len == ARM_BREAKPOINT_LEN_1)
				break;
		case 2:
			/* Allow halfword watchpoints and breakpoints. */
			if (info->ctrl.len == ARM_BREAKPOINT_LEN_2)
				break;
		default:
			return -EINVAL;
		}

		info->address &= ~alignment_mask;
		info->ctrl.len <<= offset;
	} else {
		if (info->ctrl.type == ARM_BREAKPOINT_EXECUTE)
			alignment_mask = 0x3;
		else
			alignment_mask = 0x7;
		if (info->address & alignment_mask)
			return -EINVAL;
	}

	/*
	 * Disallow per-task kernel breakpoints since these would
	 * complicate the stepping code.
	 */
	if (info->ctrl.privilege == AARCH64_BREAKPOINT_EL1 && bp->hw.target)
		return -EINVAL;

	return 0;
}

/*
 * Enable/disable all of the breakpoints active at the specified
 * exception level at the register level.
 * This is used when single-stepping after a breakpoint exception.
 */
static void toggle_bp_registers(int reg, enum debug_el el, int enable)
{
	int i, max_slots, privilege;
	u32 ctrl;
	struct perf_event **slots;

	switch (reg) {
	case AARCH64_DBG_REG_BCR:
		slots = this_cpu_ptr(bp_on_reg);
		max_slots = core_num_brps;
		break;
	case AARCH64_DBG_REG_WCR:
		slots = this_cpu_ptr(wp_on_reg);
		max_slots = core_num_wrps;
		break;
	default:
		return;
	}

	for (i = 0; i < max_slots; ++i) {
		if (!slots[i])
			continue;

		privilege = counter_arch_bp(slots[i])->ctrl.privilege;
		if (debug_exception_level(privilege) != el)
			continue;

		ctrl = read_wb_reg(reg, i);
		if (enable)
			ctrl |= 0x1;
		else
			ctrl &= ~0x1;
		write_wb_reg(reg, i, ctrl);
	}
}

/*
 * Debug exception handlers.
 */
static int breakpoint_handler(unsigned long unused, unsigned int esr,
			      struct pt_regs *regs)
{
	int i, step = 0, *kernel_step;
	u32 ctrl_reg;
	u64 addr, val;
	struct perf_event *bp, **slots;
	struct debug_info *debug_info;
	struct arch_hw_breakpoint_ctrl ctrl;

	slots = this_cpu_ptr(bp_on_reg);
	addr = instruction_pointer(regs);
	debug_info = &current->thread.debug;

	for (i = 0; i < core_num_brps; ++i) {
		rcu_read_lock();

		bp = slots[i];

		if (bp == NULL)
			goto unlock;

		/* Check if the breakpoint value matches. */
		val = read_wb_reg(AARCH64_DBG_REG_BVR, i);
		if (val != (addr & ~0x3))
			goto unlock;

		/* Possible match, check the byte address select to confirm. */
		ctrl_reg = read_wb_reg(AARCH64_DBG_REG_BCR, i);
		decode_ctrl_reg(ctrl_reg, &ctrl);
		if (!((1 << (addr & 0x3)) & ctrl.len))
			goto unlock;

		counter_arch_bp(bp)->trigger = addr;
		perf_bp_event(bp, regs);

		/* Do we need to handle the stepping? */
		if (!bp->overflow_handler)
			step = 1;
unlock:
		rcu_read_unlock();
	}

	if (!step)
		return 0;

	if (user_mode(regs)) {
		debug_info->bps_disabled = 1;
		toggle_bp_registers(AARCH64_DBG_REG_BCR, DBG_ACTIVE_EL0, 0);

		/* If we're already stepping a watchpoint, just return. */
		if (debug_info->wps_disabled)
			return 0;

		if (test_thread_flag(TIF_SINGLESTEP))
			debug_info->suspended_step = 1;
		else
			user_enable_single_step(current);
	} else {
		toggle_bp_registers(AARCH64_DBG_REG_BCR, DBG_ACTIVE_EL1, 0);
		kernel_step = this_cpu_ptr(&stepping_kernel_bp);

		if (*kernel_step != ARM_KERNEL_STEP_NONE)
			return 0;

		if (kernel_active_single_step()) {
			*kernel_step = ARM_KERNEL_STEP_SUSPEND;
		} else {
			*kernel_step = ARM_KERNEL_STEP_ACTIVE;
			kernel_enable_single_step(regs);
		}
	}

	return 0;
}

static int watchpoint_handler(unsigned long addr, unsigned int esr,
			      struct pt_regs *regs)
{
	int i, step = 0, *kernel_step, access;
	u32 ctrl_reg;
	u64 val, alignment_mask;
	struct perf_event *wp, **slots;
	struct debug_info *debug_info;
	struct arch_hw_breakpoint *info;
	struct arch_hw_breakpoint_ctrl ctrl;

	slots = this_cpu_ptr(wp_on_reg);
	debug_info = &current->thread.debug;

	for (i = 0; i < core_num_wrps; ++i) {
		rcu_read_lock();

		wp = slots[i];

		if (wp == NULL)
			goto unlock;

		info = counter_arch_bp(wp);
		/* AArch32 watchpoints are either 4 or 8 bytes aligned. */
		if (is_compat_task()) {
			if (info->ctrl.len == ARM_BREAKPOINT_LEN_8)
				alignment_mask = 0x7;
			else
				alignment_mask = 0x3;
		} else {
			alignment_mask = 0x7;
		}

		/* Check if the watchpoint value matches. */
		val = read_wb_reg(AARCH64_DBG_REG_WVR, i);
		if (val != (untagged_addr(addr) & ~alignment_mask))
			goto unlock;

		/* Possible match, check the byte address select to confirm. */
		ctrl_reg = read_wb_reg(AARCH64_DBG_REG_WCR, i);
		decode_ctrl_reg(ctrl_reg, &ctrl);
		if (!((1 << (addr & alignment_mask)) & ctrl.len))
			goto unlock;

		/*
		 * Check that the access type matches.
		 * 0 => load, otherwise => store
		 */
		access = (esr & AARCH64_ESR_ACCESS_MASK) ? HW_BREAKPOINT_W :
			 HW_BREAKPOINT_R;
		if (!(access & hw_breakpoint_type(wp)))
			goto unlock;

		info->trigger = addr;
		perf_bp_event(wp, regs);

		/* Do we need to handle the stepping? */
		if (!wp->overflow_handler)
			step = 1;

unlock:
		rcu_read_unlock();
	}

	if (!step)
		return 0;

	/*
	 * We always disable EL0 watchpoints because the kernel can
	 * cause these to fire via an unprivileged access.
	 */
	toggle_bp_registers(AARCH64_DBG_REG_WCR, DBG_ACTIVE_EL0, 0);

	if (user_mode(regs)) {
		debug_info->wps_disabled = 1;

		/* If we're already stepping a breakpoint, just return. */
		if (debug_info->bps_disabled)
			return 0;

		if (test_thread_flag(TIF_SINGLESTEP))
			debug_info->suspended_step = 1;
		else
			user_enable_single_step(current);
	} else {
		toggle_bp_registers(AARCH64_DBG_REG_WCR, DBG_ACTIVE_EL1, 0);
		kernel_step = this_cpu_ptr(&stepping_kernel_bp);

		if (*kernel_step != ARM_KERNEL_STEP_NONE)
			return 0;

		if (kernel_active_single_step()) {
			*kernel_step = ARM_KERNEL_STEP_SUSPEND;
		} else {
			*kernel_step = ARM_KERNEL_STEP_ACTIVE;
			kernel_enable_single_step(regs);
		}
	}

	return 0;
}

/*
 * Handle single-step exception.
 */
int reinstall_suspended_bps(struct pt_regs *regs)
{
	struct debug_info *debug_info = &current->thread.debug;
	int handled_exception = 0, *kernel_step;

	kernel_step = this_cpu_ptr(&stepping_kernel_bp);

	/*
	 * Called from single-step exception handler.
	 * Return 0 if execution can resume, 1 if a SIGTRAP should be
	 * reported.
	 */
	if (user_mode(regs)) {
		if (debug_info->bps_disabled) {
			debug_info->bps_disabled = 0;
			toggle_bp_registers(AARCH64_DBG_REG_BCR, DBG_ACTIVE_EL0, 1);
			handled_exception = 1;
		}

		if (debug_info->wps_disabled) {
			debug_info->wps_disabled = 0;
			toggle_bp_registers(AARCH64_DBG_REG_WCR, DBG_ACTIVE_EL0, 1);
			handled_exception = 1;
		}

		if (handled_exception) {
			if (debug_info->suspended_step) {
				debug_info->suspended_step = 0;
				/* Allow exception handling to fall-through. */
				handled_exception = 0;
			} else {
				user_disable_single_step(current);
			}
		}
	} else if (*kernel_step != ARM_KERNEL_STEP_NONE) {
		toggle_bp_registers(AARCH64_DBG_REG_BCR, DBG_ACTIVE_EL1, 1);
		toggle_bp_registers(AARCH64_DBG_REG_WCR, DBG_ACTIVE_EL1, 1);

		if (!debug_info->wps_disabled)
			toggle_bp_registers(AARCH64_DBG_REG_WCR, DBG_ACTIVE_EL0, 1);

		if (*kernel_step != ARM_KERNEL_STEP_SUSPEND) {
			kernel_disable_single_step();
			handled_exception = 1;
		} else {
			handled_exception = 0;
		}

		*kernel_step = ARM_KERNEL_STEP_NONE;
	}

	return !handled_exception;
}

/*
 * Context-switcher for restoring suspended breakpoints.
 */
void hw_breakpoint_thread_switch(struct task_struct *next)
{
	/*
	 *           current        next
	 * disabled: 0              0     => The usual case, NOTIFY_DONE
	 *           0              1     => Disable the registers
	 *           1              0     => Enable the registers
	 *           1              1     => NOTIFY_DONE. per-task bps will
	 *                                   get taken care of by perf.
	 */

	struct debug_info *current_debug_info, *next_debug_info;

	current_debug_info = &current->thread.debug;
	next_debug_info = &next->thread.debug;

	/* Update breakpoints. */
	if (current_debug_info->bps_disabled != next_debug_info->bps_disabled)
		toggle_bp_registers(AARCH64_DBG_REG_BCR,
				    DBG_ACTIVE_EL0,
				    !next_debug_info->bps_disabled);

	/* Update watchpoints. */
	if (current_debug_info->wps_disabled != next_debug_info->wps_disabled)
		toggle_bp_registers(AARCH64_DBG_REG_WCR,
				    DBG_ACTIVE_EL0,
				    !next_debug_info->wps_disabled);
}

/*
 * CPU initialisation.
 */
static void hw_breakpoint_reset(void *unused)
{
	int i;
	struct perf_event **slots;
	/*
	 * When a CPU goes through cold-boot, it does not have any installed
	 * slot, so it is safe to share the same function for restoring and
	 * resetting breakpoints; when a CPU is hotplugged in, it goes
	 * through the slots, which are all empty, hence it just resets control
	 * and value for debug registers.
	 * When this function is triggered on warm-boot through a CPU PM
	 * notifier some slots might be initialized; if so they are
	 * reprogrammed according to the debug slots content.
	 */
	for (slots = this_cpu_ptr(bp_on_reg), i = 0; i < core_num_brps; ++i) {
		if (slots[i]) {
			hw_breakpoint_control(slots[i], HW_BREAKPOINT_RESTORE);
		} else {
			write_wb_reg(AARCH64_DBG_REG_BCR, i, 0UL);
			write_wb_reg(AARCH64_DBG_REG_BVR, i, 0UL);
		}
	}

	for (slots = this_cpu_ptr(wp_on_reg), i = 0; i < core_num_wrps; ++i) {
		if (slots[i]) {
			hw_breakpoint_control(slots[i], HW_BREAKPOINT_RESTORE);
		} else {
			write_wb_reg(AARCH64_DBG_REG_WCR, i, 0UL);
			write_wb_reg(AARCH64_DBG_REG_WVR, i, 0UL);
		}
	}
}

static int hw_breakpoint_reset_notify(struct notifier_block *self,
						unsigned long action,
						void *hcpu)
{
	int cpu = (long)hcpu;
	if (action == CPU_ONLINE)
		smp_call_function_single(cpu, hw_breakpoint_reset, NULL, 1);
	return NOTIFY_OK;
}

static struct notifier_block hw_breakpoint_reset_nb = {
	.notifier_call = hw_breakpoint_reset_notify,
};

#ifdef CONFIG_CPU_PM
extern void cpu_suspend_set_dbg_restorer(void (*hw_bp_restore)(void *));
#else
static inline void cpu_suspend_set_dbg_restorer(void (*hw_bp_restore)(void *))
{
}
#endif

/*
 * One-time initialisation.
 */
static int __init arch_hw_breakpoint_init(void)
{
	core_num_brps = get_num_brps();
	core_num_wrps = get_num_wrps();

	pr_info("found %d breakpoint and %d watchpoint registers.\n",
		core_num_brps, core_num_wrps);

	cpu_notifier_register_begin();

	/*
	 * Reset the breakpoint resources. We assume that a halting
	 * debugger will leave the world in a nice state for us.
	 */
	smp_call_function(hw_breakpoint_reset, NULL, 1);
	hw_breakpoint_reset(NULL);

	/* Register debug fault handlers. */
	hook_debug_fault_code(DBG_ESR_EVT_HWBP, breakpoint_handler, SIGTRAP,
			      TRAP_HWBKPT, "hw-breakpoint handler");
	hook_debug_fault_code(DBG_ESR_EVT_HWWP, watchpoint_handler, SIGTRAP,
			      TRAP_HWBKPT, "hw-watchpoint handler");

	/* Register hotplug notifier. */
	__register_cpu_notifier(&hw_breakpoint_reset_nb);

	cpu_notifier_register_done();

	/* Register cpu_suspend hw breakpoint restore hook */
	cpu_suspend_set_dbg_restorer(hw_breakpoint_reset);

	return 0;
}
arch_initcall(arch_hw_breakpoint_init);

void hw_breakpoint_pmu_read(struct perf_event *bp)
{
}

/*
 * Dummy function to register with die_notifier.
 */
int hw_breakpoint_exceptions_notify(struct notifier_block *unused,
				    unsigned long val, void *data)
{
	return NOTIFY_DONE;
}
