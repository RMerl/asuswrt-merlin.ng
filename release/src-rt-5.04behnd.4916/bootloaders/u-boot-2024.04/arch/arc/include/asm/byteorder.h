/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

#ifndef __ASM_ARC_BYTEORDER_H
#define __ASM_ARC_BYTEORDER_H

#include <asm/types.h>

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
	#define __BYTEORDER_HAS_U64__
	#define __SWAB_64_THRU_32__
#endif

#ifdef __LITTLE_ENDIAN__
	#include <linux/byteorder/little_endian.h>
#else
	#include <linux/byteorder/big_endian.h>
#endif	/* CONFIG_SYS_BIG_ENDIAN */

#endif	/* ASM_ARC_BYTEORDER_H */
