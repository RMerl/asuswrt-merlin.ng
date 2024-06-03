/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2005 - 2013 Tensilica Inc.
 * Copyright (C) 2014 - 2016 Cadence Design Systems Inc.
 */

#ifndef _XTENSA_ASMMACRO_H
#define _XTENSA_ASMMACRO_H

#include <asm/arch/core.h>

/*
 * Function entry and return macros for supported ABIs.
 */

#if defined(__XTENSA_WINDOWED_ABI__)
#define abi_entry	entry	sp, 16
#define abi_ret		retw
#elif defined(__XTENSA_CALL0_ABI__)
#define abi_entry
#define abi_ret		ret
#else
#error Unsupported Xtensa ABI
#endif

/*
 * Some little helpers for loops. Use zero-overhead-loops
 * where applicable and if supported by the processor.
 *
 * __loopi ar, at, size, inc
 *         ar	register initialized with the start address
 *	   at	scratch register used by macro
 *	   size	size immediate value
 *	   inc	increment
 *
 * __loops ar, as, at, inc_log2[, mask_log2][, cond][, ncond]
 *	   ar	register initialized with the start address
 *	   as	register initialized with the size
 *	   at	scratch register use by macro
 *	   inc_log2	increment [in log2]
 *	   mask_log2	mask [in log2]
 *	   cond		true condition (used in loop'cond')
 *	   ncond	false condition (used in b'ncond')
 *
 * __loop  as
 *	   restart loop. 'as' register must not have been modified!
 *
 * __endla ar, as, incr
 *	   ar	start address (modified)
 *	   as	scratch register used by __loops/__loopi macros or
 *		end address used by __loopt macro
 *	   inc	increment
 */

#if XCHAL_HAVE_LOOPS

.macro	__loopi ar, at, size, incr
	movi	\at, ((\size + \incr - 1) / (\incr))
	loop	\at, 99f
.endm


.macro	__loops	ar, as, at, incr_log2, mask_log2, cond, ncond
	.ifgt \incr_log2 - 1
		addi	\at, \as, (1 << \incr_log2) - 1
		.ifnc \mask_log2,
			extui	\at, \at, \incr_log2, \mask_log2
		.else
			srli	\at, \at, \incr_log2
		.endif
	.endif
	loop\cond	\at, 99f
.endm


.macro	__loopt	ar, as, at, incr_log2
	sub	\at, \as, \ar
	.ifgt	\incr_log2 - 1
		addi	\at, \at, (1 << \incr_log2) - 1
		srli	\at, \at, \incr_log2
	.endif
	loop	\at, 99f
.endm


.macro	__loop	as
	loop	\as, 99f
.endm


.macro	__endl	ar, as
99:
.endm


#else

.macro	__loopi ar, at, size, incr
	movi	\at, ((\size + \incr - 1) / (\incr))
	addi	\at, \ar, \size
98:
.endm


.macro	__loops	ar, as, at, incr_log2, mask_log2, cond, ncond
	.ifnc \mask_log2,
		extui	\at, \as, \incr_log2, \mask_log2
	.else
		.ifnc \ncond,
			srli	\at, \as, \incr_log2
		.endif
	.endif
	.ifnc \ncond,
		b\ncond	\at, 99f

	.endif
	.ifnc \mask_log2,
		slli	\at, \at, \incr_log2
		add	\at, \ar, \at
	.else
		add	\at, \ar, \as
	.endif
98:
.endm

.macro	__loopt	ar, as, at, incr_log2
98:
.endm


.macro	__loop	as
98:
.endm


.macro	__endl	ar, as
	bltu	\ar, \as, 98b
99:
.endm


#endif


.macro	__endla	ar, as, incr
	addi	\ar, \ar, \incr
	__endl	\ar \as
.endm


#endif /* _XTENSA_ASMMACRO_H */
