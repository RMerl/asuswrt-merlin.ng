/*
 * Atomic futex routines
 *
 * Based on the PowerPC implementataion
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (C) 2013 TangoTec Ltd.
 *
 * Baruch Siach <baruch@tkos.co.il>
 */

#ifndef _ASM_XTENSA_FUTEX_H
#define _ASM_XTENSA_FUTEX_H

#ifdef __KERNEL__

#include <linux/futex.h>
#include <linux/uaccess.h>
#include <linux/errno.h>

#define __futex_atomic_op(insn, ret, oldval, uaddr, oparg) \
	__asm__ __volatile(				\
	"1:	l32i	%0, %2, 0\n"			\
		insn "\n"				\
	"	wsr	%0, scompare1\n"		\
	"2:	s32c1i	%1, %2, 0\n"			\
	"	bne	%1, %0, 1b\n"			\
	"	movi	%1, 0\n"			\
	"3:\n"						\
	"	.section .fixup,\"ax\"\n"		\
	"	.align 4\n"				\
	"4:	.long	3b\n"				\
	"5:	l32r	%0, 4b\n"			\
	"	movi	%1, %3\n"			\
	"	jx	%0\n"				\
	"	.previous\n"				\
	"	.section __ex_table,\"a\"\n"		\
	"	.long 1b,5b,2b,5b\n"			\
	"	.previous\n"				\
	: "=&r" (oldval), "=&r" (ret)			\
	: "r" (uaddr), "I" (-EFAULT), "r" (oparg)	\
	: "memory")

static inline int futex_atomic_op_inuser(int encoded_op, u32 __user *uaddr)
{
	int op = (encoded_op >> 28) & 7;
	int cmp = (encoded_op >> 24) & 15;
	int oparg = (encoded_op << 8) >> 20;
	int cmparg = (encoded_op << 20) >> 20;
	int oldval = 0, ret;
	if (encoded_op & (FUTEX_OP_OPARG_SHIFT << 28))
		oparg = 1 << oparg;

	if (!access_ok(VERIFY_WRITE, uaddr, sizeof(u32)))
		return -EFAULT;

#if !XCHAL_HAVE_S32C1I
	return -ENOSYS;
#endif

	pagefault_disable();

	switch (op) {
	case FUTEX_OP_SET:
		__futex_atomic_op("mov %1, %4", ret, oldval, uaddr, oparg);
		break;
	case FUTEX_OP_ADD:
		__futex_atomic_op("add %1, %0, %4", ret, oldval, uaddr,
				oparg);
		break;
	case FUTEX_OP_OR:
		__futex_atomic_op("or %1, %0, %4", ret, oldval, uaddr,
				oparg);
		break;
	case FUTEX_OP_ANDN:
		__futex_atomic_op("and %1, %0, %4", ret, oldval, uaddr,
				~oparg);
		break;
	case FUTEX_OP_XOR:
		__futex_atomic_op("xor %1, %0, %4", ret, oldval, uaddr,
				oparg);
		break;
	default:
		ret = -ENOSYS;
	}

	pagefault_enable();

	if (ret)
		return ret;

	switch (cmp) {
	case FUTEX_OP_CMP_EQ: return (oldval == cmparg);
	case FUTEX_OP_CMP_NE: return (oldval != cmparg);
	case FUTEX_OP_CMP_LT: return (oldval < cmparg);
	case FUTEX_OP_CMP_GE: return (oldval >= cmparg);
	case FUTEX_OP_CMP_LE: return (oldval <= cmparg);
	case FUTEX_OP_CMP_GT: return (oldval > cmparg);
	}

	return -ENOSYS;
}

static inline int
futex_atomic_cmpxchg_inatomic(u32 *uval, u32 __user *uaddr,
			      u32 oldval, u32 newval)
{
	int ret = 0;

	if (!access_ok(VERIFY_WRITE, uaddr, sizeof(u32)))
		return -EFAULT;

#if !XCHAL_HAVE_S32C1I
	return -ENOSYS;
#endif

	__asm__ __volatile__ (
	"	# futex_atomic_cmpxchg_inatomic\n"
	"	wsr	%5, scompare1\n"
	"1:	s32c1i	%1, %4, 0\n"
	"	s32i	%1, %6, 0\n"
	"2:\n"
	"	.section .fixup,\"ax\"\n"
	"	.align 4\n"
	"3:	.long	2b\n"
	"4:	l32r	%1, 3b\n"
	"	movi	%0, %7\n"
	"	jx	%1\n"
	"	.previous\n"
	"	.section __ex_table,\"a\"\n"
	"	.long 1b,4b\n"
	"	.previous\n"
	: "+r" (ret), "+r" (newval), "+m" (*uaddr), "+m" (*uval)
	: "r" (uaddr), "r" (oldval), "r" (uval), "I" (-EFAULT)
	: "memory");

	return ret;
}

#endif /* __KERNEL__ */
#endif /* _ASM_XTENSA_FUTEX_H */
