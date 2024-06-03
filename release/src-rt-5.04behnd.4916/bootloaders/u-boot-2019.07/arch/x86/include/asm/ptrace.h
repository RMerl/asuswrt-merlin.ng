#ifndef _I386_PTRACE_H
#define _I386_PTRACE_H

#include <asm/types.h>

#define EBX 0
#define ECX 1
#define EDX 2
#define ESI 3
#define EDI 4
#define EBP 5
#define EAX 6
#define DS 7
#define ES 8
#define FS 9
#define GS 10
#define ORIG_EAX 11
#define EIP 12
#define CS  13
#define EFL 14
#define UESP 15
#define SS   16
#define FRAME_SIZE 17

/* this struct defines the way the registers are stored on the
   stack during a system call. */

struct pt_regs {
	long ebx;
	long ecx;
	long edx;
	long esi;
	long edi;
	long ebp;
	long eax;
	int  xds;
	int  xes;
	int  xfs;
	int  xgs;
	long orig_eax;
	long eip;
	int  xcs;
	long eflags;
	long esp;
	int  xss;
}  __attribute__ ((packed));

struct irq_regs {
	/* Pushed by irq_common_entry */
	long ebx;
	long ecx;
	long edx;
	long esi;
	long edi;
	long ebp;
	long esp;
	long eax;
	long xds;
	long xes;
	long xfs;
	long xgs;
	long xss;
	/* Pushed by vector handler (irq_<num>) */
	long irq_id;
	/* Pushed by cpu in response to interrupt */
	union {
		struct {
			long eip;
			long xcs;
			long eflags;
		} ctx1;
		struct {
			long err;
			long eip;
			long xcs;
			long eflags;
		} ctx2;
	} context;
}  __attribute__ ((packed));

/* Arbitrarily choose the same ptrace numbers as used by the Sparc code. */
#define PTRACE_GETREGS            12
#define PTRACE_SETREGS            13
#define PTRACE_GETFPREGS          14
#define PTRACE_SETFPREGS          15
#define PTRACE_GETFPXREGS         18
#define PTRACE_SETFPXREGS         19

#define PTRACE_SETOPTIONS         21

/* options set using PTRACE_SETOPTIONS */
#define PTRACE_O_TRACESYSGOOD     0x00000001

#ifdef __KERNEL__
#define user_mode(regs) ((VM_MASK & (regs)->eflags) || (3 & (regs)->xcs))
#define instruction_pointer(regs) ((regs)->eip)
extern void show_regs(struct pt_regs *);
#endif

#endif
