/*
 * Copyright Boundary Devices
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#include <config.h>
#include <linux/kernel.h>
#include <asm/arch/ddr.h>
#include <asm/arch/lpddr4_define.h>

/* MNT Reform2 */
#define CFG_DDR_MB 4096
#define CFG_DDR_RANK_BITS 1
#define CFG_DDR_CHANNEL_CNT 2

#ifdef WR_POST_EXT_3200
#define CH2_VAL_INIT4	((LPDDR4_MR3 << 16) | 0x00020008)
#else
#define CH2_VAL_INIT4	((LPDDR4_MR3 << 16) | 8)
#endif

#if CFG_DDR_MB == 1024
	/* Address map is from MSB 28: r14, r13-r0, b2-b0, c9-c0 */
#define CH2_VAL_DDRC_ADDRMAP0_R0	0x0000001F
#define CH2_VAL_DDRC_ADDRMAP6_R0	0x0F070707

#elif CFG_DDR_MB == 2048
	/* Address map is from MSB 28: r15, r14, r13-r0, b2-b0, c9-c0 */
#define CH2_VAL_DDRC_ADDRMAP0_R0	0x0000001F
#define CH2_VAL_DDRC_ADDRMAP6_R0	0x07070707
	/* Address map is from MSB 28: cs, r14, r13-r0, b2-b0, c9-c0 */
#define CH2_VAL_DDRC_ADDRMAP0_R1	0x00000016
#define CH2_VAL_DDRC_ADDRMAP6_R1	0x0F070707

#elif CFG_DDR_MB == 3072
	/* Address map is from MSB 29: r15, r14, cs, r13-r0, b2-b0, c9-c0 */
#define CH2_VAL_DDRC_ADDRMAP0_R1	0x00000015
#define CH2_VAL_DDRC_ADDRMAP6_R1	0x48080707

#elif CFG_DDR_MB == 4096
	/* Address map is from MSB 29: cs, r15, r14, r13-r0, b2-b0, c9-c0 */
#define CH2_VAL_DDRC_ADDRMAP0_R1	0x00000017
#define CH2_VAL_DDRC_ADDRMAP6_R1	0x07070707
#else
#error unsupported memory size
#endif

#define LPDDR4_CS_R0	0x1	/* 0 rank bits, 1 chip select */
#define LPDDR4_CS_R1	0x3	/* 1 rank bit, 2 chip selects */

#if (CFG_DDR_RANK_BITS == 0) || !defined(CH2_VAL_DDRC_ADDRMAP0_R1)
#ifdef CH2_VAL_DDRC_ADDRMAP0_R0
#define CH2_LPDDR4_CS			LPDDR4_CS_R0
#define CH2_VAL_DDRC_ADDRMAP0		CH2_VAL_DDRC_ADDRMAP0_R0
#define CH2_VAL_DDRC_ADDRMAP6		CH2_VAL_DDRC_ADDRMAP6_R0
#else
#error unsupported memory rank/size
#endif
/*
 * rank0 will succeed, even if really rank 1, so we need
 * to probe memory if rank0 succeeds
 */
#if defined(CH2_VAL_DDRC_ADDRMAP0_R0) && defined(CH2_VAL_DDRC_ADDRMAP0_R1)
#define CH2_LPDDR4_CS_NEW		LPDDR4_CS_R1
#define CH2_VAL_DDRC_ADDRMAP0_NEW	CH2_VAL_DDRC_ADDRMAP0_R1
#define CH2_VAL_DDRC_ADDRMAP6_NEW	CH2_VAL_DDRC_ADDRMAP6_R1
#endif

#elif (CFG_DDR_RANK_BITS == 1) || !defined(CH2_VAL_DDRC_ADDRMAP0_R0)
#ifdef CH2_VAL_DDRC_ADDRMAP0_R1
#define CH2_LPDDR4_CS			LPDDR4_CS_R1
#define CH2_VAL_DDRC_ADDRMAP0		CH2_VAL_DDRC_ADDRMAP0_R1
#define CH2_VAL_DDRC_ADDRMAP6		CH2_VAL_DDRC_ADDRMAP6_R1
#else
#error unsupported memory rank/size
#endif

#if defined(CH2_VAL_DDRC_ADDRMAP0_R0) && defined(CH2_VAL_DDRC_ADDRMAP0_R1)
#define CH2_LPDDR4_CS_NEW		LPDDR4_CS_R0
#define CH2_VAL_DDRC_ADDRMAP0_NEW	CH2_VAL_DDRC_ADDRMAP0_R0
#define CH2_VAL_DDRC_ADDRMAP6_NEW	CH2_VAL_DDRC_ADDRMAP6_R0
#endif

#else
#error unsupported rank bits
#endif

#if (CFG_DDR_CHANNEL_CNT == 2)
#if (CFG_DDR_RANK_BITS == 0) && !defined(CH2_VAL_DDRC_ADDRMAP0_R0)
#error unsupported options
#endif
#if (CFG_DDR_RANK_BITS == 1) && !defined(CH2_VAL_DDRC_ADDRMAP0_R1)
#error unsupported options
#endif
#endif
