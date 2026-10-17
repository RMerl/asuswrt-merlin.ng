/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#ifndef __ASM_NIOS2_BYTEORDER_H_
#define __ASM_NIOS2_BYTEORDER_H_

#include <asm/types.h>

#if !defined(__STRICT_ANSI__) || defined(__KERNEL__)
#  define __BYTEORDER_HAS_U64__
#  define __SWAB_64_THRU_32__
#endif

#include <linux/byteorder/little_endian.h>

#endif /* __ASM_NIOS2_BYTEORDER_H_ */
