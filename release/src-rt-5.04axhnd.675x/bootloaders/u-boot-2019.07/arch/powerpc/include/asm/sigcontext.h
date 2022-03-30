#ifndef _ASM_PPC_SIGCONTEXT_H
#define _ASM_PPC_SIGCONTEXT_H

#include <asm/ptrace.h>


struct sigcontext_struct {
	unsigned long	_unused[4];
	int		signal;
	unsigned long	handler;
	unsigned long	oldmask;
	struct pt_regs	*regs;
};

#endif
