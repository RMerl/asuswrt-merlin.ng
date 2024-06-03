/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 */

#ifndef __EBI__
#define __EBI__

#include <common.h>
#include <asm/io.h>
#include "vct.h"

#define EXT_DEVICE_CHANNEL_3	(0x30000000)
#define EXT_DEVICE_CHANNEL_2	(0x20000000)
#define EXT_DEVICE_CHANNEL_1	(0x10000000)
#define EXT_CPU_ACCESS_ACTIVE	(0x00000001)
#define EXT_DMA_ACCESS_ACTIVE	(1 << 14)
#define EXT_CPU_IORDY_SL	(0x00000001)

#define EBI_CPU_WRITE		(1 << 31)
#define EBI_CPU_ID_SHIFT	(28)
#define EBI_CPU_ADDR_MASK	~(~0UL << EBI_CPU_ID_SHIFT)

/* position of various bit slices in timing register EBI_DEV[01]_TIM1_RD1 */
#define ADDR_LATCH_ENABLE	0
#define ADDR_ACTIVATION		4
#define CHIP_SELECT_START	8
#define OUTPUT_ENABLE_START	12
#define WAIT_TIME		28
#define READ_DURATION		20

/* position of various bit slices in timing register EBI_DEV[01]_TIM1_RD2 */
#define OUTPUT_ENABLE_END	0
#define CHIP_SELECT_END		4
#define ADDR_DEACTIVATION	8
#define RECOVER_TIME		12
#define ACK_TIME		20

/* various bits in configuration register EBI_DEV[01]_CONFIG1 */
#define EBI_EXTERNAL_DATA_8	(1 <<  8)
#define EBI_EXT_ADDR_SHIFT	(1 << 22)
#define EBI_EXTERNAL_DATA_16	EBI_EXT_ADDR_SHIFT
#define EBI_CHIP_SELECT_1	0x2
#define EBI_CHIP_SELECT_2	0x4
#define EBI_BUSY_EN_RD		(1 << 12)
#define DIR_ACCESS_WRITE	(1 << 20)
#define DIR_ACCESS_MASK		(1 << 20)

/* various bits in configuration register EBI_DEV[01]_CONFIG2 */
#define ADDRESS_INCREMENT_ON	0x0
#define ADDRESS_INCREMENT_OFF	0x100
#define QUEUE_LENGTH_1		0x40
#define QUEUE_LENGTH_2		0x80
#define QUEUE_LENGTH_3		0xC0
#define QUEUE_LENGTH_4		0
#define CPU_TRANSFER_SIZE_32	0
#define CPU_TRANSFER_SIZE_16	0x10
#define CPU_TRANSFER_SIZE_8	0x20
#define READ_ENDIANNESS_ABCD	0
#define READ_ENDIANNESS_DCBA	0x4
#define READ_ENDIANNESS_BADC	0x8
#define READ_ENDIANNESS_CDAB	0xC
#define WRITE_ENDIANNESS_ABCD	0
#define WRITE_ENDIANNESS_DCBA	0x1
#define WRITE_ENDIANNESS_BADC	0x2
#define WRITE_ENDIANNESS_CDAB	0x3

/* various bits in configuration register EBI_CTRL_SIG_ACTLV */
#define IORDY_ACTIVELEVEL_HIGH	(1 << 14)
#define ALE_ACTIVELEVEL_HIGH	(1 <<  8)

/* bits in register EBI_SIG_LEVEL */
#define IORDY_LEVEL_MASK	1

static inline void ebi_wait(void)
{
	while (reg_read(EBI_STATUS(EBI_BASE)) & EXT_CPU_ACCESS_ACTIVE)
		;	/* wait */
}

#endif
