#ifndef _ASM_ALPHA_SYSCALL_H
#define _ASM_ALPHA_SYSCALL_H

#include <uapi/linux/audit.h>

static inline int syscall_get_arch(void)
{
	return AUDIT_ARCH_ALPHA;
}

#endif	/* _ASM_ALPHA_SYSCALL_H */
