/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (c) Copyright 2010-2014 Xilinx, Inc. All rights reserved.
 * (c) Copyright 2016 Topic Embedded Products.
 */

#ifndef _ASM_ARCH_PS7_INIT_GPL_H
#define _ASM_ARCH_PS7_INIT_GPL_H

/* Opcode exit is 0 all the time */
#define OPCODE_EXIT		0U
#define OPCODE_MASKWRITE	0U
#define OPCODE_MASKPOLL		1U
#define OPCODE_MASKDELAY	2U
#define OPCODE_WRITE		3U
#define OPCODE_ADDRESS_MASK	(~3U)

/* Sentinel */
#define EMIT_EXIT()			OPCODE_EXIT
/* Opcode is in lower 2 bits of address, address is always 4-byte aligned */
#define EMIT_MASKWRITE(addr, mask, val)	OPCODE_MASKWRITE | addr, mask, val
#define EMIT_MASKPOLL(addr, mask)	OPCODE_MASKPOLL | addr, mask
#define EMIT_MASKDELAY(addr, mask)	OPCODE_MASKDELAY | addr, mask
#define EMIT_WRITE(addr, val)		OPCODE_WRITE | addr, val

/* Returns codes of ps7_init* */
#define PS7_INIT_SUCCESS		(0)
#define PS7_INIT_CORRUPT		(1)
#define PS7_INIT_TIMEOUT		(2)
#define PS7_POLL_FAILED_DDR_INIT	(3)
#define PS7_POLL_FAILED_DMA		(4)
#define PS7_POLL_FAILED_PLL		(5)

#define PCW_SILICON_VERSION_1	0
#define PCW_SILICON_VERSION_2	1
#define PCW_SILICON_VERSION_3	2

/* Called by spl.c */
int ps7_init(void);
int ps7_post_config(void);

/* Defined in ps7_init_common.c */
int ps7_config(unsigned long *ps7_config_init);

unsigned long ps7GetSiliconVersion(void);

#endif /* _ASM_ARCH_PS7_INIT_GPL_H */
