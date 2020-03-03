#include <linux/perf_event.h>
#include <linux/types.h>

#include <asm/perf_event.h>
#include <asm/msr.h>
#include <asm/insn.h>

#include "perf_event.h"

enum {
	LBR_FORMAT_32		= 0x00,
	LBR_FORMAT_LIP		= 0x01,
	LBR_FORMAT_EIP		= 0x02,
	LBR_FORMAT_EIP_FLAGS	= 0x03,
	LBR_FORMAT_EIP_FLAGS2	= 0x04,
	LBR_FORMAT_MAX_KNOWN    = LBR_FORMAT_EIP_FLAGS2,
};

static enum {
	LBR_EIP_FLAGS		= 1,
	LBR_TSX			= 2,
} lbr_desc[LBR_FORMAT_MAX_KNOWN + 1] = {
	[LBR_FORMAT_EIP_FLAGS]  = LBR_EIP_FLAGS,
	[LBR_FORMAT_EIP_FLAGS2] = LBR_EIP_FLAGS | LBR_TSX,
};

/*
 * Intel LBR_SELECT bits
 * Intel Vol3a, April 2011, Section 16.7 Table 16-10
 *
 * Hardware branch filter (not available on all CPUs)
 */
#define LBR_KERNEL_BIT		0 /* do not capture at ring0 */
#define LBR_USER_BIT		1 /* do not capture at ring > 0 */
#define LBR_JCC_BIT		2 /* do not capture conditional branches */
#define LBR_REL_CALL_BIT	3 /* do not capture relative calls */
#define LBR_IND_CALL_BIT	4 /* do not capture indirect calls */
#define LBR_RETURN_BIT		5 /* do not capture near returns */
#define LBR_IND_JMP_BIT		6 /* do not capture indirect jumps */
#define LBR_REL_JMP_BIT		7 /* do not capture relative jumps */
#define LBR_FAR_BIT		8 /* do not capture far branches */
#define LBR_CALL_STACK_BIT	9 /* enable call stack */

#define LBR_KERNEL	(1 << LBR_KERNEL_BIT)
#define LBR_USER	(1 << LBR_USER_BIT)
#define LBR_JCC		(1 << LBR_JCC_BIT)
#define LBR_REL_CALL	(1 << LBR_REL_CALL_BIT)
#define LBR_IND_CALL	(1 << LBR_IND_CALL_BIT)
#define LBR_RETURN	(1 << LBR_RETURN_BIT)
#define LBR_REL_JMP	(1 << LBR_REL_JMP_BIT)
#define LBR_IND_JMP	(1 << LBR_IND_JMP_BIT)
#define LBR_FAR		(1 << LBR_FAR_BIT)
#define LBR_CALL_STACK	(1 << LBR_CALL_STACK_BIT)

#define LBR_PLM (LBR_KERNEL | LBR_USER)

#define LBR_SEL_MASK	0x1ff	/* valid bits in LBR_SELECT */
#define LBR_NOT_SUPP	-1	/* LBR filter not supported */
#define LBR_IGN		0	/* ignored */

#define LBR_ANY		 \
	(LBR_JCC	|\
	 LBR_REL_CALL	|\
	 LBR_IND_CALL	|\
	 LBR_RETURN	|\
	 LBR_REL_JMP	|\
	 LBR_IND_JMP	|\
	 LBR_FAR)

#define LBR_FROM_FLAG_MISPRED  (1ULL << 63)
#define LBR_FROM_FLAG_IN_TX    (1ULL << 62)
#define LBR_FROM_FLAG_ABORT    (1ULL << 61)

/*
 * x86control flow change classification
 * x86control flow changes include branches, interrupts, traps, faults
 */
enum {
	X86_BR_NONE		= 0,      /* unknown */

	X86_BR_USER		= 1 << 0, /* branch target is user */
	X86_BR_KERNEL		= 1 << 1, /* branch target is kernel */

	X86_BR_CALL		= 1 << 2, /* call */
	X86_BR_RET		= 1 << 3, /* return */
	X86_BR_SYSCALL		= 1 << 4, /* syscall */
	X86_BR_SYSRET		= 1 << 5, /* syscall return */
	X86_BR_INT		= 1 << 6, /* sw interrupt */
	X86_BR_IRET		= 1 << 7, /* return from interrupt */
	X86_BR_JCC		= 1 << 8, /* conditional */
	X86_BR_JMP		= 1 << 9, /* jump */
	X86_BR_IRQ		= 1 << 10,/* hw interrupt or trap or fault */
	X86_BR_IND_CALL		= 1 << 11,/* indirect calls */
	X86_BR_ABORT		= 1 << 12,/* transaction abort */
	X86_BR_IN_TX		= 1 << 13,/* in transaction */
	X86_BR_NO_TX		= 1 << 14,/* not in transaction */
	X86_BR_ZERO_CALL	= 1 << 15,/* zero length call */
	X86_BR_CALL_STACK	= 1 << 16,/* call stack */
};

#define X86_BR_PLM (X86_BR_USER | X86_BR_KERNEL)
#define X86_BR_ANYTX (X86_BR_NO_TX | X86_BR_IN_TX)

#define X86_BR_ANY       \
	(X86_BR_CALL    |\
	 X86_BR_RET     |\
	 X86_BR_SYSCALL |\
	 X86_BR_SYSRET  |\
	 X86_BR_INT     |\
	 X86_BR_IRET    |\
	 X86_BR_JCC     |\
	 X86_BR_JMP	 |\
	 X86_BR_IRQ	 |\
	 X86_BR_ABORT	 |\
	 X86_BR_IND_CALL |\
	 X86_BR_ZERO_CALL)

#define X86_BR_ALL (X86_BR_PLM | X86_BR_ANY)

#define X86_BR_ANY_CALL		 \
	(X86_BR_CALL		|\
	 X86_BR_IND_CALL	|\
	 X86_BR_ZERO_CALL	|\
	 X86_BR_SYSCALL		|\
	 X86_BR_IRQ		|\
	 X86_BR_INT)

static void intel_pmu_lbr_filter(struct cpu_hw_events *cpuc);

/*
 * We only support LBR implementations that have FREEZE_LBRS_ON_PMI
 * otherwise it becomes near impossible to get a reliable stack.
 */

static void __intel_pmu_lbr_enable(bool pmi)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);
	u64 debugctl, lbr_select = 0, orig_debugctl;

	/*
	 * No need to reprogram LBR_SELECT in a PMI, as it
	 * did not change.
	 */
	if (cpuc->lbr_sel && !pmi) {
		lbr_select = cpuc->lbr_sel->config;
		wrmsrl(MSR_LBR_SELECT, lbr_select);
	}

	rdmsrl(MSR_IA32_DEBUGCTLMSR, debugctl);
	orig_debugctl = debugctl;
	debugctl |= DEBUGCTLMSR_LBR;
	/*
	 * LBR callstack does not work well with FREEZE_LBRS_ON_PMI.
	 * If FREEZE_LBRS_ON_PMI is set, PMI near call/return instructions
	 * may cause superfluous increase/decrease of LBR_TOS.
	 */
	if (!(lbr_select & LBR_CALL_STACK))
		debugctl |= DEBUGCTLMSR_FREEZE_LBRS_ON_PMI;
	if (orig_debugctl != debugctl)
		wrmsrl(MSR_IA32_DEBUGCTLMSR, debugctl);
}

static void __intel_pmu_lbr_disable(void)
{
	u64 debugctl;

	rdmsrl(MSR_IA32_DEBUGCTLMSR, debugctl);
	debugctl &= ~(DEBUGCTLMSR_LBR | DEBUGCTLMSR_FREEZE_LBRS_ON_PMI);
	wrmsrl(MSR_IA32_DEBUGCTLMSR, debugctl);
}

static void intel_pmu_lbr_reset_32(void)
{
	int i;

	for (i = 0; i < x86_pmu.lbr_nr; i++)
		wrmsrl(x86_pmu.lbr_from + i, 0);
}

static void intel_pmu_lbr_reset_64(void)
{
	int i;

	for (i = 0; i < x86_pmu.lbr_nr; i++) {
		wrmsrl(x86_pmu.lbr_from + i, 0);
		wrmsrl(x86_pmu.lbr_to   + i, 0);
	}
}

void intel_pmu_lbr_reset(void)
{
	if (!x86_pmu.lbr_nr)
		return;

	if (x86_pmu.intel_cap.lbr_format == LBR_FORMAT_32)
		intel_pmu_lbr_reset_32();
	else
		intel_pmu_lbr_reset_64();
}

/*
 * TOS = most recently recorded branch
 */
static inline u64 intel_pmu_lbr_tos(void)
{
	u64 tos;

	rdmsrl(x86_pmu.lbr_tos, tos);
	return tos;
}

enum {
	LBR_NONE,
	LBR_VALID,
};

static void __intel_pmu_lbr_restore(struct x86_perf_task_context *task_ctx)
{
	int i;
	unsigned lbr_idx, mask;
	u64 tos;

	if (task_ctx->lbr_callstack_users == 0 ||
	    task_ctx->lbr_stack_state == LBR_NONE) {
		intel_pmu_lbr_reset();
		return;
	}

	mask = x86_pmu.lbr_nr - 1;
	tos = intel_pmu_lbr_tos();
	for (i = 0; i < x86_pmu.lbr_nr; i++) {
		lbr_idx = (tos - i) & mask;
		wrmsrl(x86_pmu.lbr_from + lbr_idx, task_ctx->lbr_from[i]);
		wrmsrl(x86_pmu.lbr_to + lbr_idx, task_ctx->lbr_to[i]);
	}
	task_ctx->lbr_stack_state = LBR_NONE;
}

static void __intel_pmu_lbr_save(struct x86_perf_task_context *task_ctx)
{
	int i;
	unsigned lbr_idx, mask;
	u64 tos;

	if (task_ctx->lbr_callstack_users == 0) {
		task_ctx->lbr_stack_state = LBR_NONE;
		return;
	}

	mask = x86_pmu.lbr_nr - 1;
	tos = intel_pmu_lbr_tos();
	for (i = 0; i < x86_pmu.lbr_nr; i++) {
		lbr_idx = (tos - i) & mask;
		rdmsrl(x86_pmu.lbr_from + lbr_idx, task_ctx->lbr_from[i]);
		rdmsrl(x86_pmu.lbr_to + lbr_idx, task_ctx->lbr_to[i]);
	}
	task_ctx->lbr_stack_state = LBR_VALID;
}

void intel_pmu_lbr_sched_task(struct perf_event_context *ctx, bool sched_in)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);
	struct x86_perf_task_context *task_ctx;

	if (!x86_pmu.lbr_nr)
		return;

	/*
	 * If LBR callstack feature is enabled and the stack was saved when
	 * the task was scheduled out, restore the stack. Otherwise flush
	 * the LBR stack.
	 */
	task_ctx = ctx ? ctx->task_ctx_data : NULL;
	if (task_ctx) {
		if (sched_in) {
			__intel_pmu_lbr_restore(task_ctx);
			cpuc->lbr_context = ctx;
		} else {
			__intel_pmu_lbr_save(task_ctx);
		}
		return;
	}

	/*
	 * When sampling the branck stack in system-wide, it may be
	 * necessary to flush the stack on context switch. This happens
	 * when the branch stack does not tag its entries with the pid
	 * of the current task. Otherwise it becomes impossible to
	 * associate a branch entry with a task. This ambiguity is more
	 * likely to appear when the branch stack supports priv level
	 * filtering and the user sets it to monitor only at the user
	 * level (which could be a useful measurement in system-wide
	 * mode). In that case, the risk is high of having a branch
	 * stack with branch from multiple tasks.
 	 */
	if (sched_in) {
		intel_pmu_lbr_reset();
		cpuc->lbr_context = ctx;
	}
}

static inline bool branch_user_callstack(unsigned br_sel)
{
	return (br_sel & X86_BR_USER) && (br_sel & X86_BR_CALL_STACK);
}

void intel_pmu_lbr_enable(struct perf_event *event)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);
	struct x86_perf_task_context *task_ctx;

	if (!x86_pmu.lbr_nr)
		return;

	/*
	 * Reset the LBR stack if we changed task context to
	 * avoid data leaks.
	 */
	if (event->ctx->task && cpuc->lbr_context != event->ctx) {
		intel_pmu_lbr_reset();
		cpuc->lbr_context = event->ctx;
	}
	cpuc->br_sel = event->hw.branch_reg.reg;

	if (branch_user_callstack(cpuc->br_sel) && event->ctx &&
					event->ctx->task_ctx_data) {
		task_ctx = event->ctx->task_ctx_data;
		task_ctx->lbr_callstack_users++;
	}

	cpuc->lbr_users++;
	perf_sched_cb_inc(event->ctx->pmu);
}

void intel_pmu_lbr_disable(struct perf_event *event)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);
	struct x86_perf_task_context *task_ctx;

	if (!x86_pmu.lbr_nr)
		return;

	if (branch_user_callstack(cpuc->br_sel) && event->ctx &&
					event->ctx->task_ctx_data) {
		task_ctx = event->ctx->task_ctx_data;
		task_ctx->lbr_callstack_users--;
	}

	cpuc->lbr_users--;
	WARN_ON_ONCE(cpuc->lbr_users < 0);
	perf_sched_cb_dec(event->ctx->pmu);

	if (cpuc->enabled && !cpuc->lbr_users) {
		__intel_pmu_lbr_disable();
		/* avoid stale pointer */
		cpuc->lbr_context = NULL;
	}
}

void intel_pmu_lbr_enable_all(bool pmi)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);

	if (cpuc->lbr_users)
		__intel_pmu_lbr_enable(pmi);
}

void intel_pmu_lbr_disable_all(void)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);

	if (cpuc->lbr_users)
		__intel_pmu_lbr_disable();
}

static void intel_pmu_lbr_read_32(struct cpu_hw_events *cpuc)
{
	unsigned long mask = x86_pmu.lbr_nr - 1;
	u64 tos = intel_pmu_lbr_tos();
	int i;

	for (i = 0; i < x86_pmu.lbr_nr; i++) {
		unsigned long lbr_idx = (tos - i) & mask;
		union {
			struct {
				u32 from;
				u32 to;
			};
			u64     lbr;
		} msr_lastbranch;

		rdmsrl(x86_pmu.lbr_from + lbr_idx, msr_lastbranch.lbr);

		cpuc->lbr_entries[i].from	= msr_lastbranch.from;
		cpuc->lbr_entries[i].to		= msr_lastbranch.to;
		cpuc->lbr_entries[i].mispred	= 0;
		cpuc->lbr_entries[i].predicted	= 0;
		cpuc->lbr_entries[i].reserved	= 0;
	}
	cpuc->lbr_stack.nr = i;
}

/*
 * Due to lack of segmentation in Linux the effective address (offset)
 * is the same as the linear address, allowing us to merge the LIP and EIP
 * LBR formats.
 */
static void intel_pmu_lbr_read_64(struct cpu_hw_events *cpuc)
{
	unsigned long mask = x86_pmu.lbr_nr - 1;
	int lbr_format = x86_pmu.intel_cap.lbr_format;
	u64 tos = intel_pmu_lbr_tos();
	int i;
	int out = 0;

	for (i = 0; i < x86_pmu.lbr_nr; i++) {
		unsigned long lbr_idx = (tos - i) & mask;
		u64 from, to, mis = 0, pred = 0, in_tx = 0, abort = 0;
		int skip = 0;
		int lbr_flags = lbr_desc[lbr_format];

		rdmsrl(x86_pmu.lbr_from + lbr_idx, from);
		rdmsrl(x86_pmu.lbr_to   + lbr_idx, to);

		if (lbr_flags & LBR_EIP_FLAGS) {
			mis = !!(from & LBR_FROM_FLAG_MISPRED);
			pred = !mis;
			skip = 1;
		}
		if (lbr_flags & LBR_TSX) {
			in_tx = !!(from & LBR_FROM_FLAG_IN_TX);
			abort = !!(from & LBR_FROM_FLAG_ABORT);
			skip = 3;
		}
		from = (u64)((((s64)from) << skip) >> skip);

		/*
		 * Some CPUs report duplicated abort records,
		 * with the second entry not having an abort bit set.
		 * Skip them here. This loop runs backwards,
		 * so we need to undo the previous record.
		 * If the abort just happened outside the window
		 * the extra entry cannot be removed.
		 */
		if (abort && x86_pmu.lbr_double_abort && out > 0)
			out--;

		cpuc->lbr_entries[out].from	 = from;
		cpuc->lbr_entries[out].to	 = to;
		cpuc->lbr_entries[out].mispred	 = mis;
		cpuc->lbr_entries[out].predicted = pred;
		cpuc->lbr_entries[out].in_tx	 = in_tx;
		cpuc->lbr_entries[out].abort	 = abort;
		cpuc->lbr_entries[out].reserved	 = 0;
		out++;
	}
	cpuc->lbr_stack.nr = out;
}

void intel_pmu_lbr_read(void)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);

	if (!cpuc->lbr_users)
		return;

	if (x86_pmu.intel_cap.lbr_format == LBR_FORMAT_32)
		intel_pmu_lbr_read_32(cpuc);
	else
		intel_pmu_lbr_read_64(cpuc);

	intel_pmu_lbr_filter(cpuc);
}

/*
 * SW filter is used:
 * - in case there is no HW filter
 * - in case the HW filter has errata or limitations
 */
static int intel_pmu_setup_sw_lbr_filter(struct perf_event *event)
{
	u64 br_type = event->attr.branch_sample_type;
	int mask = 0;

	if (br_type & PERF_SAMPLE_BRANCH_USER)
		mask |= X86_BR_USER;

	if (br_type & PERF_SAMPLE_BRANCH_KERNEL)
		mask |= X86_BR_KERNEL;

	/* we ignore BRANCH_HV here */

	if (br_type & PERF_SAMPLE_BRANCH_ANY)
		mask |= X86_BR_ANY;

	if (br_type & PERF_SAMPLE_BRANCH_ANY_CALL)
		mask |= X86_BR_ANY_CALL;

	if (br_type & PERF_SAMPLE_BRANCH_ANY_RETURN)
		mask |= X86_BR_RET | X86_BR_IRET | X86_BR_SYSRET;

	if (br_type & PERF_SAMPLE_BRANCH_IND_CALL)
		mask |= X86_BR_IND_CALL;

	if (br_type & PERF_SAMPLE_BRANCH_ABORT_TX)
		mask |= X86_BR_ABORT;

	if (br_type & PERF_SAMPLE_BRANCH_IN_TX)
		mask |= X86_BR_IN_TX;

	if (br_type & PERF_SAMPLE_BRANCH_NO_TX)
		mask |= X86_BR_NO_TX;

	if (br_type & PERF_SAMPLE_BRANCH_COND)
		mask |= X86_BR_JCC;

	if (br_type & PERF_SAMPLE_BRANCH_CALL_STACK) {
		if (!x86_pmu_has_lbr_callstack())
			return -EOPNOTSUPP;
		if (mask & ~(X86_BR_USER | X86_BR_KERNEL))
			return -EINVAL;
		mask |= X86_BR_CALL | X86_BR_IND_CALL | X86_BR_RET |
			X86_BR_CALL_STACK;
	}

	/*
	 * stash actual user request into reg, it may
	 * be used by fixup code for some CPU
	 */
	event->hw.branch_reg.reg = mask;
	return 0;
}

/*
 * setup the HW LBR filter
 * Used only when available, may not be enough to disambiguate
 * all branches, may need the help of the SW filter
 */
static int intel_pmu_setup_hw_lbr_filter(struct perf_event *event)
{
	struct hw_perf_event_extra *reg;
	u64 br_type = event->attr.branch_sample_type;
	u64 mask = 0, v;
	int i;

	for (i = 0; i < PERF_SAMPLE_BRANCH_MAX_SHIFT; i++) {
		if (!(br_type & (1ULL << i)))
			continue;

		v = x86_pmu.lbr_sel_map[i];
		if (v == LBR_NOT_SUPP)
			return -EOPNOTSUPP;

		if (v != LBR_IGN)
			mask |= v;
	}
	reg = &event->hw.branch_reg;
	reg->idx = EXTRA_REG_LBR;

	/*
	 * The first 9 bits (LBR_SEL_MASK) in LBR_SELECT operate
	 * in suppress mode. So LBR_SELECT should be set to
	 * (~mask & LBR_SEL_MASK) | (mask & ~LBR_SEL_MASK)
	 */
	reg->config = mask ^ x86_pmu.lbr_sel_mask;

	return 0;
}

int intel_pmu_setup_lbr_filter(struct perf_event *event)
{
	int ret = 0;

	/*
	 * no LBR on this PMU
	 */
	if (!x86_pmu.lbr_nr)
		return -EOPNOTSUPP;

	/*
	 * setup SW LBR filter
	 */
	ret = intel_pmu_setup_sw_lbr_filter(event);
	if (ret)
		return ret;

	/*
	 * setup HW LBR filter, if any
	 */
	if (x86_pmu.lbr_sel_map)
		ret = intel_pmu_setup_hw_lbr_filter(event);

	return ret;
}

/*
 * return the type of control flow change at address "from"
 * intruction is not necessarily a branch (in case of interrupt).
 *
 * The branch type returned also includes the priv level of the
 * target of the control flow change (X86_BR_USER, X86_BR_KERNEL).
 *
 * If a branch type is unknown OR the instruction cannot be
 * decoded (e.g., text page not present), then X86_BR_NONE is
 * returned.
 */
static int branch_type(unsigned long from, unsigned long to, int abort)
{
	struct insn insn;
	void *addr;
	int bytes_read, bytes_left;
	int ret = X86_BR_NONE;
	int ext, to_plm, from_plm;
	u8 buf[MAX_INSN_SIZE];
	int is64 = 0;

	to_plm = kernel_ip(to) ? X86_BR_KERNEL : X86_BR_USER;
	from_plm = kernel_ip(from) ? X86_BR_KERNEL : X86_BR_USER;

	/*
	 * maybe zero if lbr did not fill up after a reset by the time
	 * we get a PMU interrupt
	 */
	if (from == 0 || to == 0)
		return X86_BR_NONE;

	if (abort)
		return X86_BR_ABORT | to_plm;

	if (from_plm == X86_BR_USER) {
		/*
		 * can happen if measuring at the user level only
		 * and we interrupt in a kernel thread, e.g., idle.
		 */
		if (!current->mm)
			return X86_BR_NONE;

		/* may fail if text not present */
		bytes_left = copy_from_user_nmi(buf, (void __user *)from,
						MAX_INSN_SIZE);
		bytes_read = MAX_INSN_SIZE - bytes_left;
		if (!bytes_read)
			return X86_BR_NONE;

		addr = buf;
	} else {
		/*
		 * The LBR logs any address in the IP, even if the IP just
		 * faulted. This means userspace can control the from address.
		 * Ensure we don't blindy read any address by validating it is
		 * a known text address.
		 */
		if (kernel_text_address(from)) {
			addr = (void *)from;
			/*
			 * Assume we can get the maximum possible size
			 * when grabbing kernel data.  This is not
			 * _strictly_ true since we could possibly be
			 * executing up next to a memory hole, but
			 * it is very unlikely to be a problem.
			 */
			bytes_read = MAX_INSN_SIZE;
		} else {
			return X86_BR_NONE;
		}
	}

	/*
	 * decoder needs to know the ABI especially
	 * on 64-bit systems running 32-bit apps
	 */
#ifdef CONFIG_X86_64
	is64 = kernel_ip((unsigned long)addr) || !test_thread_flag(TIF_IA32);
#endif
	insn_init(&insn, addr, bytes_read, is64);
	insn_get_opcode(&insn);
	if (!insn.opcode.got)
		return X86_BR_ABORT;

	switch (insn.opcode.bytes[0]) {
	case 0xf:
		switch (insn.opcode.bytes[1]) {
		case 0x05: /* syscall */
		case 0x34: /* sysenter */
			ret = X86_BR_SYSCALL;
			break;
		case 0x07: /* sysret */
		case 0x35: /* sysexit */
			ret = X86_BR_SYSRET;
			break;
		case 0x80 ... 0x8f: /* conditional */
			ret = X86_BR_JCC;
			break;
		default:
			ret = X86_BR_NONE;
		}
		break;
	case 0x70 ... 0x7f: /* conditional */
		ret = X86_BR_JCC;
		break;
	case 0xc2: /* near ret */
	case 0xc3: /* near ret */
	case 0xca: /* far ret */
	case 0xcb: /* far ret */
		ret = X86_BR_RET;
		break;
	case 0xcf: /* iret */
		ret = X86_BR_IRET;
		break;
	case 0xcc ... 0xce: /* int */
		ret = X86_BR_INT;
		break;
	case 0xe8: /* call near rel */
		insn_get_immediate(&insn);
		if (insn.immediate1.value == 0) {
			/* zero length call */
			ret = X86_BR_ZERO_CALL;
			break;
		}
	case 0x9a: /* call far absolute */
		ret = X86_BR_CALL;
		break;
	case 0xe0 ... 0xe3: /* loop jmp */
		ret = X86_BR_JCC;
		break;
	case 0xe9 ... 0xeb: /* jmp */
		ret = X86_BR_JMP;
		break;
	case 0xff: /* call near absolute, call far absolute ind */
		insn_get_modrm(&insn);
		ext = (insn.modrm.bytes[0] >> 3) & 0x7;
		switch (ext) {
		case 2: /* near ind call */
		case 3: /* far ind call */
			ret = X86_BR_IND_CALL;
			break;
		case 4:
		case 5:
			ret = X86_BR_JMP;
			break;
		}
		break;
	default:
		ret = X86_BR_NONE;
	}
	/*
	 * interrupts, traps, faults (and thus ring transition) may
	 * occur on any instructions. Thus, to classify them correctly,
	 * we need to first look at the from and to priv levels. If they
	 * are different and to is in the kernel, then it indicates
	 * a ring transition. If the from instruction is not a ring
	 * transition instr (syscall, systenter, int), then it means
	 * it was a irq, trap or fault.
	 *
	 * we have no way of detecting kernel to kernel faults.
	 */
	if (from_plm == X86_BR_USER && to_plm == X86_BR_KERNEL
	    && ret != X86_BR_SYSCALL && ret != X86_BR_INT)
		ret = X86_BR_IRQ;

	/*
	 * branch priv level determined by target as
	 * is done by HW when LBR_SELECT is implemented
	 */
	if (ret != X86_BR_NONE)
		ret |= to_plm;

	return ret;
}

/*
 * implement actual branch filter based on user demand.
 * Hardware may not exactly satisfy that request, thus
 * we need to inspect opcodes. Mismatched branches are
 * discarded. Therefore, the number of branches returned
 * in PERF_SAMPLE_BRANCH_STACK sample may vary.
 */
static void
intel_pmu_lbr_filter(struct cpu_hw_events *cpuc)
{
	u64 from, to;
	int br_sel = cpuc->br_sel;
	int i, j, type;
	bool compress = false;

	/* if sampling all branches, then nothing to filter */
	if ((br_sel & X86_BR_ALL) == X86_BR_ALL)
		return;

	for (i = 0; i < cpuc->lbr_stack.nr; i++) {

		from = cpuc->lbr_entries[i].from;
		to = cpuc->lbr_entries[i].to;

		type = branch_type(from, to, cpuc->lbr_entries[i].abort);
		if (type != X86_BR_NONE && (br_sel & X86_BR_ANYTX)) {
			if (cpuc->lbr_entries[i].in_tx)
				type |= X86_BR_IN_TX;
			else
				type |= X86_BR_NO_TX;
		}

		/* if type does not correspond, then discard */
		if (type == X86_BR_NONE || (br_sel & type) != type) {
			cpuc->lbr_entries[i].from = 0;
			compress = true;
		}
	}

	if (!compress)
		return;

	/* remove all entries with from=0 */
	for (i = 0; i < cpuc->lbr_stack.nr; ) {
		if (!cpuc->lbr_entries[i].from) {
			j = i;
			while (++j < cpuc->lbr_stack.nr)
				cpuc->lbr_entries[j-1] = cpuc->lbr_entries[j];
			cpuc->lbr_stack.nr--;
			if (!cpuc->lbr_entries[i].from)
				continue;
		}
		i++;
	}
}

/*
 * Map interface branch filters onto LBR filters
 */
static const int nhm_lbr_sel_map[PERF_SAMPLE_BRANCH_MAX_SHIFT] = {
	[PERF_SAMPLE_BRANCH_ANY_SHIFT]		= LBR_ANY,
	[PERF_SAMPLE_BRANCH_USER_SHIFT]		= LBR_USER,
	[PERF_SAMPLE_BRANCH_KERNEL_SHIFT]	= LBR_KERNEL,
	[PERF_SAMPLE_BRANCH_HV_SHIFT]		= LBR_IGN,
	[PERF_SAMPLE_BRANCH_ANY_RETURN_SHIFT]	= LBR_RETURN | LBR_REL_JMP
						| LBR_IND_JMP | LBR_FAR,
	/*
	 * NHM/WSM erratum: must include REL_JMP+IND_JMP to get CALL branches
	 */
	[PERF_SAMPLE_BRANCH_ANY_CALL_SHIFT] =
	 LBR_REL_CALL | LBR_IND_CALL | LBR_REL_JMP | LBR_IND_JMP | LBR_FAR,
	/*
	 * NHM/WSM erratum: must include IND_JMP to capture IND_CALL
	 */
	[PERF_SAMPLE_BRANCH_IND_CALL_SHIFT] = LBR_IND_CALL | LBR_IND_JMP,
	[PERF_SAMPLE_BRANCH_COND_SHIFT]     = LBR_JCC,
};

static const int snb_lbr_sel_map[PERF_SAMPLE_BRANCH_MAX_SHIFT] = {
	[PERF_SAMPLE_BRANCH_ANY_SHIFT]		= LBR_ANY,
	[PERF_SAMPLE_BRANCH_USER_SHIFT]		= LBR_USER,
	[PERF_SAMPLE_BRANCH_KERNEL_SHIFT]	= LBR_KERNEL,
	[PERF_SAMPLE_BRANCH_HV_SHIFT]		= LBR_IGN,
	[PERF_SAMPLE_BRANCH_ANY_RETURN_SHIFT]	= LBR_RETURN | LBR_FAR,
	[PERF_SAMPLE_BRANCH_ANY_CALL_SHIFT]	= LBR_REL_CALL | LBR_IND_CALL
						| LBR_FAR,
	[PERF_SAMPLE_BRANCH_IND_CALL_SHIFT]	= LBR_IND_CALL,
	[PERF_SAMPLE_BRANCH_COND_SHIFT]		= LBR_JCC,
};

static const int hsw_lbr_sel_map[PERF_SAMPLE_BRANCH_MAX_SHIFT] = {
	[PERF_SAMPLE_BRANCH_ANY_SHIFT]		= LBR_ANY,
	[PERF_SAMPLE_BRANCH_USER_SHIFT]		= LBR_USER,
	[PERF_SAMPLE_BRANCH_KERNEL_SHIFT]	= LBR_KERNEL,
	[PERF_SAMPLE_BRANCH_HV_SHIFT]		= LBR_IGN,
	[PERF_SAMPLE_BRANCH_ANY_RETURN_SHIFT]	= LBR_RETURN | LBR_FAR,
	[PERF_SAMPLE_BRANCH_ANY_CALL_SHIFT]	= LBR_REL_CALL | LBR_IND_CALL
						| LBR_FAR,
	[PERF_SAMPLE_BRANCH_IND_CALL_SHIFT]	= LBR_IND_CALL,
	[PERF_SAMPLE_BRANCH_COND_SHIFT]		= LBR_JCC,
	[PERF_SAMPLE_BRANCH_CALL_STACK_SHIFT]	= LBR_REL_CALL | LBR_IND_CALL
						| LBR_RETURN | LBR_CALL_STACK,
};

/* core */
void __init intel_pmu_lbr_init_core(void)
{
	x86_pmu.lbr_nr     = 4;
	x86_pmu.lbr_tos    = MSR_LBR_TOS;
	x86_pmu.lbr_from   = MSR_LBR_CORE_FROM;
	x86_pmu.lbr_to     = MSR_LBR_CORE_TO;

	/*
	 * SW branch filter usage:
	 * - compensate for lack of HW filter
	 */
	pr_cont("4-deep LBR, ");
}

/* nehalem/westmere */
void __init intel_pmu_lbr_init_nhm(void)
{
	x86_pmu.lbr_nr     = 16;
	x86_pmu.lbr_tos    = MSR_LBR_TOS;
	x86_pmu.lbr_from   = MSR_LBR_NHM_FROM;
	x86_pmu.lbr_to     = MSR_LBR_NHM_TO;

	x86_pmu.lbr_sel_mask = LBR_SEL_MASK;
	x86_pmu.lbr_sel_map  = nhm_lbr_sel_map;

	/*
	 * SW branch filter usage:
	 * - workaround LBR_SEL errata (see above)
	 * - support syscall, sysret capture.
	 *   That requires LBR_FAR but that means far
	 *   jmp need to be filtered out
	 */
	pr_cont("16-deep LBR, ");
}

/* sandy bridge */
void __init intel_pmu_lbr_init_snb(void)
{
	x86_pmu.lbr_nr	 = 16;
	x86_pmu.lbr_tos	 = MSR_LBR_TOS;
	x86_pmu.lbr_from = MSR_LBR_NHM_FROM;
	x86_pmu.lbr_to   = MSR_LBR_NHM_TO;

	x86_pmu.lbr_sel_mask = LBR_SEL_MASK;
	x86_pmu.lbr_sel_map  = snb_lbr_sel_map;

	/*
	 * SW branch filter usage:
	 * - support syscall, sysret capture.
	 *   That requires LBR_FAR but that means far
	 *   jmp need to be filtered out
	 */
	pr_cont("16-deep LBR, ");
}

/* haswell */
void intel_pmu_lbr_init_hsw(void)
{
	x86_pmu.lbr_nr	 = 16;
	x86_pmu.lbr_tos	 = MSR_LBR_TOS;
	x86_pmu.lbr_from = MSR_LBR_NHM_FROM;
	x86_pmu.lbr_to   = MSR_LBR_NHM_TO;

	x86_pmu.lbr_sel_mask = LBR_SEL_MASK;
	x86_pmu.lbr_sel_map  = hsw_lbr_sel_map;

	pr_cont("16-deep LBR, ");
}

/* atom */
void __init intel_pmu_lbr_init_atom(void)
{
	/*
	 * only models starting at stepping 10 seems
	 * to have an operational LBR which can freeze
	 * on PMU interrupt
	 */
	if (boot_cpu_data.x86_model == 28
	    && boot_cpu_data.x86_mask < 10) {
		pr_cont("LBR disabled due to erratum");
		return;
	}

	x86_pmu.lbr_nr	   = 8;
	x86_pmu.lbr_tos    = MSR_LBR_TOS;
	x86_pmu.lbr_from   = MSR_LBR_CORE_FROM;
	x86_pmu.lbr_to     = MSR_LBR_CORE_TO;

	/*
	 * SW branch filter usage:
	 * - compensate for lack of HW filter
	 */
	pr_cont("8-deep LBR, ");
}
