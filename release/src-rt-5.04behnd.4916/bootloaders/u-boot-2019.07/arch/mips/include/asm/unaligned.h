/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2007 Ralf Baechle (ralf@linux-mips.org)
 */
#ifndef _ASM_MIPS_UNALIGNED_H
#define _ASM_MIPS_UNALIGNED_H

#include <linux/compiler.h>
#if defined(__MIPSEB__)
#define get_unaligned	__get_unaligned_be
#define put_unaligned	__put_unaligned_be
#elif defined(__MIPSEL__)
#define get_unaligned	__get_unaligned_le
#define put_unaligned	__put_unaligned_le
#else
#error "MIPS, but neither __MIPSEB__, nor __MIPSEL__???"
#endif

#include <linux/unaligned/le_byteshift.h>
#include <linux/unaligned/be_byteshift.h>
#include <linux/unaligned/generic.h>

#endif /* _ASM_MIPS_UNALIGNED_H */
