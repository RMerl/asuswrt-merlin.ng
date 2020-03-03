#ifndef _ASM_X86_FTRACE_H
#define _ASM_X86_FTRACE_H

#ifdef CONFIG_FUNCTION_TRACER
#ifdef CC_USING_FENTRY
# define MCOUNT_ADDR		((long)(__fentry__))
#else
# define MCOUNT_ADDR		((long)(mcount))
#endif
#define MCOUNT_INSN_SIZE	5 /* sizeof mcount call */

#ifdef CONFIG_DYNAMIC_FTRACE
#define ARCH_SUPPORTS_FTRACE_OPS 1
#endif

#ifndef __ASSEMBLY__
extern void mcount(void);
extern atomic_t modifying_ftrace_code;
extern void __fentry__(void);

static inline unsigned long ftrace_call_adjust(unsigned long addr)
{
	/*
	 * addr is the address of the mcount call instruction.
	 * recordmcount does the necessary offset calculation.
	 */
	return addr;
}

#ifdef CONFIG_DYNAMIC_FTRACE

struct dyn_arch_ftrace {
	/* No extra data needed for x86 */
};

int ftrace_int3_handler(struct pt_regs *regs);

#define FTRACE_GRAPH_TRAMP_ADDR FTRACE_GRAPH_ADDR

#endif /*  CONFIG_DYNAMIC_FTRACE */
#endif /* __ASSEMBLY__ */
#endif /* CONFIG_FUNCTION_TRACER */


#if !defined(__ASSEMBLY__) && !defined(COMPILE_OFFSETS)

#if defined(CONFIG_FTRACE_SYSCALLS) && defined(CONFIG_IA32_EMULATION)
#include <asm/compat.h>

/*
 * Because ia32 syscalls do not map to x86_64 syscall numbers
 * this screws up the trace output when tracing a ia32 task.
 * Instead of reporting bogus syscalls, just do not trace them.
 *
 * If the user realy wants these, then they should use the
 * raw syscall tracepoints with filtering.
 */
#define ARCH_TRACE_IGNORE_COMPAT_SYSCALLS 1
static inline bool arch_trace_is_compat_syscall(struct pt_regs *regs)
{
	if (is_compat_task())
		return true;
	return false;
}
#endif /* CONFIG_FTRACE_SYSCALLS && CONFIG_IA32_EMULATION */
#endif /* !__ASSEMBLY__  && !COMPILE_OFFSETS */

#endif /* _ASM_X86_FTRACE_H */
