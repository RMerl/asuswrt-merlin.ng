/*
 * Code for replacing ftrace calls with jumps.
 *
 * Copyright (C) 2007-2008 Steven Rostedt <srostedt@redhat.com>
 *
 * Thanks goes out to P.A. Semi, Inc for supplying me with a PPC64 box.
 *
 * Added function graph tracer code, taken from x86 that was written
 * by Frederic Weisbecker, and ported to PPC by Steven Rostedt.
 *
 */

#define pr_fmt(fmt) "ftrace-powerpc: " fmt

#include <linux/spinlock.h>
#include <linux/hardirq.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/ftrace.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/list.h>

#include <asm/cacheflush.h>
#include <asm/code-patching.h>
#include <asm/ftrace.h>
#include <asm/syscall.h>


#ifdef CONFIG_DYNAMIC_FTRACE
static unsigned int
ftrace_call_replace(unsigned long ip, unsigned long addr, int link)
{
	unsigned int op;

	addr = ppc_function_entry((void *)addr);

	/* if (link) set op to 'bl' else 'b' */
	op = create_branch((unsigned int *)ip, addr, link ? 1 : 0);

	return op;
}

static int
ftrace_modify_code(unsigned long ip, unsigned int old, unsigned int new)
{
	unsigned int replaced;

	/*
	 * Note: Due to modules and __init, code can
	 *  disappear and change, we need to protect against faulting
	 *  as well as code changing. We do this by using the
	 *  probe_kernel_* functions.
	 *
	 * No real locking needed, this code is run through
	 * kstop_machine, or before SMP starts.
	 */

	/* read the text we want to modify */
	if (probe_kernel_read(&replaced, (void *)ip, MCOUNT_INSN_SIZE))
		return -EFAULT;

	/* Make sure it is what we expect it to be */
	if (replaced != old)
		return -EINVAL;

	/* replace the text with the new text */
	if (patch_instruction((unsigned int *)ip, new))
		return -EPERM;

	return 0;
}

/*
 * Helper functions that are the same for both PPC64 and PPC32.
 */
static int test_24bit_addr(unsigned long ip, unsigned long addr)
{
	addr = ppc_function_entry((void *)addr);

	/* use the create_branch to verify that this offset can be branched */
	return create_branch((unsigned int *)ip, addr, 0);
}

#ifdef CONFIG_MODULES

static int is_bl_op(unsigned int op)
{
	return (op & 0xfc000003) == 0x48000001;
}

static unsigned long find_bl_target(unsigned long ip, unsigned int op)
{
	static int offset;

	offset = (op & 0x03fffffc);
	/* make it signed */
	if (offset & 0x02000000)
		offset |= 0xfe000000;

	return ip + (long)offset;
}

#ifdef CONFIG_PPC64
static int
__ftrace_make_nop(struct module *mod,
		  struct dyn_ftrace *rec, unsigned long addr)
{
	unsigned int op;
	unsigned long entry, ptr;
	unsigned long ip = rec->ip;
	void *tramp;

	/* read where this goes */
	if (probe_kernel_read(&op, (void *)ip, sizeof(int)))
		return -EFAULT;

	/* Make sure that that this is still a 24bit jump */
	if (!is_bl_op(op)) {
		pr_err("Not expected bl: opcode is %x\n", op);
		return -EINVAL;
	}

	/* lets find where the pointer goes */
	tramp = (void *)find_bl_target(ip, op);

	pr_devel("ip:%lx jumps to %p", ip, tramp);

	if (!is_module_trampoline(tramp)) {
		pr_err("Not a trampoline\n");
		return -EINVAL;
	}

	if (module_trampoline_target(mod, tramp, &ptr)) {
		pr_err("Failed to get trampoline target\n");
		return -EFAULT;
	}

	pr_devel("trampoline target %lx", ptr);

	entry = ppc_global_function_entry((void *)addr);
	/* This should match what was called */
	if (ptr != entry) {
		pr_err("addr %lx does not match expected %lx\n", ptr, entry);
		return -EINVAL;
	}

	/*
	 * Our original call site looks like:
	 *
	 * bl <tramp>
	 * ld r2,XX(r1)
	 *
	 * Milton Miller pointed out that we can not simply nop the branch.
	 * If a task was preempted when calling a trace function, the nops
	 * will remove the way to restore the TOC in r2 and the r2 TOC will
	 * get corrupted.
	 *
	 * Use a b +8 to jump over the load.
	 */
	op = 0x48000008;	/* b +8 */

	if (patch_instruction((unsigned int *)ip, op))
		return -EPERM;

	return 0;
}

#else /* !PPC64 */
static int
__ftrace_make_nop(struct module *mod,
		  struct dyn_ftrace *rec, unsigned long addr)
{
	unsigned int op;
	unsigned int jmp[4];
	unsigned long ip = rec->ip;
	unsigned long tramp;

	if (probe_kernel_read(&op, (void *)ip, MCOUNT_INSN_SIZE))
		return -EFAULT;

	/* Make sure that that this is still a 24bit jump */
	if (!is_bl_op(op)) {
		pr_err("Not expected bl: opcode is %x\n", op);
		return -EINVAL;
	}

	/* lets find where the pointer goes */
	tramp = find_bl_target(ip, op);

	/*
	 * On PPC32 the trampoline looks like:
	 *  0x3d, 0x80, 0x00, 0x00  lis r12,sym@ha
	 *  0x39, 0x8c, 0x00, 0x00  addi r12,r12,sym@l
	 *  0x7d, 0x89, 0x03, 0xa6  mtctr r12
	 *  0x4e, 0x80, 0x04, 0x20  bctr
	 */

	pr_devel("ip:%lx jumps to %lx", ip, tramp);

	/* Find where the trampoline jumps to */
	if (probe_kernel_read(jmp, (void *)tramp, sizeof(jmp))) {
		pr_err("Failed to read %lx\n", tramp);
		return -EFAULT;
	}

	pr_devel(" %08x %08x ", jmp[0], jmp[1]);

	/* verify that this is what we expect it to be */
	if (((jmp[0] & 0xffff0000) != 0x3d800000) ||
	    ((jmp[1] & 0xffff0000) != 0x398c0000) ||
	    (jmp[2] != 0x7d8903a6) ||
	    (jmp[3] != 0x4e800420)) {
		pr_err("Not a trampoline\n");
		return -EINVAL;
	}

	tramp = (jmp[1] & 0xffff) |
		((jmp[0] & 0xffff) << 16);
	if (tramp & 0x8000)
		tramp -= 0x10000;

	pr_devel(" %lx ", tramp);

	if (tramp != addr) {
		pr_err("Trampoline location %08lx does not match addr\n",
		       tramp);
		return -EINVAL;
	}

	op = PPC_INST_NOP;

	if (patch_instruction((unsigned int *)ip, op))
		return -EPERM;

	return 0;
}
#endif /* PPC64 */
#endif /* CONFIG_MODULES */

int ftrace_make_nop(struct module *mod,
		    struct dyn_ftrace *rec, unsigned long addr)
{
	unsigned long ip = rec->ip;
	unsigned int old, new;

	/*
	 * If the calling address is more that 24 bits away,
	 * then we had to use a trampoline to make the call.
	 * Otherwise just update the call site.
	 */
	if (test_24bit_addr(ip, addr)) {
		/* within range */
		old = ftrace_call_replace(ip, addr, 1);
		new = PPC_INST_NOP;
		return ftrace_modify_code(ip, old, new);
	}

#ifdef CONFIG_MODULES
	/*
	 * Out of range jumps are called from modules.
	 * We should either already have a pointer to the module
	 * or it has been passed in.
	 */
	if (!rec->arch.mod) {
		if (!mod) {
			pr_err("No module loaded addr=%lx\n", addr);
			return -EFAULT;
		}
		rec->arch.mod = mod;
	} else if (mod) {
		if (mod != rec->arch.mod) {
			pr_err("Record mod %p not equal to passed in mod %p\n",
			       rec->arch.mod, mod);
			return -EINVAL;
		}
		/* nothing to do if mod == rec->arch.mod */
	} else
		mod = rec->arch.mod;

	return __ftrace_make_nop(mod, rec, addr);
#else
	/* We should not get here without modules */
	return -EINVAL;
#endif /* CONFIG_MODULES */
}

#ifdef CONFIG_MODULES
#ifdef CONFIG_PPC64
static int
__ftrace_make_call(struct dyn_ftrace *rec, unsigned long addr)
{
	unsigned int op[2];
	void *ip = (void *)rec->ip;

	/* read where this goes */
	if (probe_kernel_read(op, ip, sizeof(op)))
		return -EFAULT;

	/*
	 * We expect to see:
	 *
	 * b +8
	 * ld r2,XX(r1)
	 *
	 * The load offset is different depending on the ABI. For simplicity
	 * just mask it out when doing the compare.
	 */
	if ((op[0] != 0x48000008) || ((op[1] & 0xffff0000) != 0xe8410000)) {
		pr_err("Unexpected call sequence: %x %x\n", op[0], op[1]);
		return -EINVAL;
	}

	/* If we never set up a trampoline to ftrace_caller, then bail */
	if (!rec->arch.mod->arch.tramp) {
		pr_err("No ftrace trampoline\n");
		return -EINVAL;
	}

	/* Ensure branch is within 24 bits */
	if (!create_branch(ip, rec->arch.mod->arch.tramp, BRANCH_SET_LINK)) {
		pr_err("Branch out of range\n");
		return -EINVAL;
	}

	if (patch_branch(ip, rec->arch.mod->arch.tramp, BRANCH_SET_LINK)) {
		pr_err("REL24 out of range!\n");
		return -EINVAL;
	}

	return 0;
}
#else
static int
__ftrace_make_call(struct dyn_ftrace *rec, unsigned long addr)
{
	unsigned int op;
	unsigned long ip = rec->ip;

	/* read where this goes */
	if (probe_kernel_read(&op, (void *)ip, MCOUNT_INSN_SIZE))
		return -EFAULT;

	/* It should be pointing to a nop */
	if (op != PPC_INST_NOP) {
		pr_err("Expected NOP but have %x\n", op);
		return -EINVAL;
	}

	/* If we never set up a trampoline to ftrace_caller, then bail */
	if (!rec->arch.mod->arch.tramp) {
		pr_err("No ftrace trampoline\n");
		return -EINVAL;
	}

	/* create the branch to the trampoline */
	op = create_branch((unsigned int *)ip,
			   rec->arch.mod->arch.tramp, BRANCH_SET_LINK);
	if (!op) {
		pr_err("REL24 out of range!\n");
		return -EINVAL;
	}

	pr_devel("write to %lx\n", rec->ip);

	if (patch_instruction((unsigned int *)ip, op))
		return -EPERM;

	return 0;
}
#endif /* CONFIG_PPC64 */
#endif /* CONFIG_MODULES */

int ftrace_make_call(struct dyn_ftrace *rec, unsigned long addr)
{
	unsigned long ip = rec->ip;
	unsigned int old, new;

	/*
	 * If the calling address is more that 24 bits away,
	 * then we had to use a trampoline to make the call.
	 * Otherwise just update the call site.
	 */
	if (test_24bit_addr(ip, addr)) {
		/* within range */
		old = PPC_INST_NOP;
		new = ftrace_call_replace(ip, addr, 1);
		return ftrace_modify_code(ip, old, new);
	}

#ifdef CONFIG_MODULES
	/*
	 * Out of range jumps are called from modules.
	 * Being that we are converting from nop, it had better
	 * already have a module defined.
	 */
	if (!rec->arch.mod) {
		pr_err("No module loaded\n");
		return -EINVAL;
	}

	return __ftrace_make_call(rec, addr);
#else
	/* We should not get here without modules */
	return -EINVAL;
#endif /* CONFIG_MODULES */
}

int ftrace_update_ftrace_func(ftrace_func_t func)
{
	unsigned long ip = (unsigned long)(&ftrace_call);
	unsigned int old, new;
	int ret;

	old = *(unsigned int *)&ftrace_call;
	new = ftrace_call_replace(ip, (unsigned long)func, 1);
	ret = ftrace_modify_code(ip, old, new);

	return ret;
}

static int __ftrace_replace_code(struct dyn_ftrace *rec, int enable)
{
	unsigned long ftrace_addr = (unsigned long)FTRACE_ADDR;
	int ret;

	ret = ftrace_update_record(rec, enable);

	switch (ret) {
	case FTRACE_UPDATE_IGNORE:
		return 0;
	case FTRACE_UPDATE_MAKE_CALL:
		return ftrace_make_call(rec, ftrace_addr);
	case FTRACE_UPDATE_MAKE_NOP:
		return ftrace_make_nop(NULL, rec, ftrace_addr);
	}

	return 0;
}

void ftrace_replace_code(int enable)
{
	struct ftrace_rec_iter *iter;
	struct dyn_ftrace *rec;
	int ret;

	for (iter = ftrace_rec_iter_start(); iter;
	     iter = ftrace_rec_iter_next(iter)) {
		rec = ftrace_rec_iter_record(iter);
		ret = __ftrace_replace_code(rec, enable);
		if (ret) {
			ftrace_bug(ret, rec);
			return;
		}
	}
}

void arch_ftrace_update_code(int command)
{
	if (command & FTRACE_UPDATE_CALLS)
		ftrace_replace_code(1);
	else if (command & FTRACE_DISABLE_CALLS)
		ftrace_replace_code(0);

	if (command & FTRACE_UPDATE_TRACE_FUNC)
		ftrace_update_ftrace_func(ftrace_trace_function);

	if (command & FTRACE_START_FUNC_RET)
		ftrace_enable_ftrace_graph_caller();
	else if (command & FTRACE_STOP_FUNC_RET)
		ftrace_disable_ftrace_graph_caller();
}

int __init ftrace_dyn_arch_init(void)
{
	return 0;
}
#endif /* CONFIG_DYNAMIC_FTRACE */

#ifdef CONFIG_FUNCTION_GRAPH_TRACER

#ifdef CONFIG_DYNAMIC_FTRACE
extern void ftrace_graph_call(void);
extern void ftrace_graph_stub(void);

int ftrace_enable_ftrace_graph_caller(void)
{
	unsigned long ip = (unsigned long)(&ftrace_graph_call);
	unsigned long addr = (unsigned long)(&ftrace_graph_caller);
	unsigned long stub = (unsigned long)(&ftrace_graph_stub);
	unsigned int old, new;

	old = ftrace_call_replace(ip, stub, 0);
	new = ftrace_call_replace(ip, addr, 0);

	return ftrace_modify_code(ip, old, new);
}

int ftrace_disable_ftrace_graph_caller(void)
{
	unsigned long ip = (unsigned long)(&ftrace_graph_call);
	unsigned long addr = (unsigned long)(&ftrace_graph_caller);
	unsigned long stub = (unsigned long)(&ftrace_graph_stub);
	unsigned int old, new;

	old = ftrace_call_replace(ip, addr, 0);
	new = ftrace_call_replace(ip, stub, 0);

	return ftrace_modify_code(ip, old, new);
}
#endif /* CONFIG_DYNAMIC_FTRACE */

/*
 * Hook the return address and push it in the stack of return addrs
 * in current thread info. Return the address we want to divert to.
 */
unsigned long prepare_ftrace_return(unsigned long parent, unsigned long ip)
{
	struct ftrace_graph_ent trace;
	unsigned long return_hooker;

	if (unlikely(ftrace_graph_is_dead()))
		goto out;

	if (unlikely(atomic_read(&current->tracing_graph_pause)))
		goto out;

	return_hooker = ppc_function_entry(return_to_handler);

	trace.func = ip;
	trace.depth = current->curr_ret_stack + 1;

	/* Only trace if the calling function expects to */
	if (!ftrace_graph_entry(&trace))
		goto out;

	if (ftrace_push_return_trace(parent, ip, &trace.depth, 0) == -EBUSY)
		goto out;

	parent = return_hooker;
out:
	return parent;
}
#endif /* CONFIG_FUNCTION_GRAPH_TRACER */

#if defined(CONFIG_FTRACE_SYSCALLS) && defined(CONFIG_PPC64)
unsigned long __init arch_syscall_addr(int nr)
{
	return sys_call_table[nr*2];
}
#endif /* CONFIG_FTRACE_SYSCALLS && CONFIG_PPC64 */
