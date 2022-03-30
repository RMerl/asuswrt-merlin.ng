// SPDX-License-Identifier: GPL-2.0+
/*
 * (c) Copyright 2010-2017 Xilinx, Inc. All rights reserved.
 * (c) Copyright 2016 Topic Embedded Products.
 */

#include <asm/io.h>
#include <asm/spl.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/ps7_init_gpl.h>

__weak int ps7_init(void)
{
	/*
	 * This function is overridden by the one in
	 * board/xilinx/zynq/(platform)/ps7_init_gpl.c, if it exists.
	 */
	return 0;
}

__weak int ps7_post_config(void)
{
	/*
	 * This function is overridden by the one in
	 * board/xilinx/zynq/(platform)/ps7_init_gpl.c, if it exists.
	 */
	return 0;
}

/* For delay calculation using global registers*/
#define SCU_GLOBAL_TIMER_COUNT_L32	0xF8F00200
#define SCU_GLOBAL_TIMER_COUNT_U32	0xF8F00204
#define SCU_GLOBAL_TIMER_CONTROL	0xF8F00208
#define SCU_GLOBAL_TIMER_AUTO_INC	0xF8F00218
#define APU_FREQ  666666666

#define PS7_MASK_POLL_TIME 100000000

/* IO accessors. No memory barriers desired. */
static inline void iowrite(unsigned long val, unsigned long addr)
{
	__raw_writel(val, addr);
}

static inline unsigned long ioread(unsigned long addr)
{
	return __raw_readl(addr);
}

/* start timer */
static void perf_start_clock(void)
{
	iowrite((1 << 0) | /* Timer Enable */
		(1 << 3) | /* Auto-increment */
		(0 << 8), /* Pre-scale */
		SCU_GLOBAL_TIMER_CONTROL);
}

/* Compute mask for given delay in miliseconds*/
static unsigned long get_number_of_cycles_for_delay(unsigned long delay)
{
	return (APU_FREQ / (2 * 1000)) * delay;
}

/* stop timer */
static void perf_disable_clock(void)
{
	iowrite(0, SCU_GLOBAL_TIMER_CONTROL);
}

/* stop timer and reset timer count regs */
static void perf_reset_clock(void)
{
	perf_disable_clock();
	iowrite(0, SCU_GLOBAL_TIMER_COUNT_L32);
	iowrite(0, SCU_GLOBAL_TIMER_COUNT_U32);
}

static void perf_reset_and_start_timer(void)
{
	perf_reset_clock();
	perf_start_clock();
}

int __weak ps7_config(unsigned long *ps7_config_init)
{
	unsigned long *ptr = ps7_config_init;
	unsigned long opcode;
	unsigned long addr;
	unsigned long val;
	unsigned long mask;
	unsigned int numargs;
	int i;
	unsigned long delay;

	for (;;) {
		opcode = ptr[0];
		if (opcode == OPCODE_EXIT)
			return PS7_INIT_SUCCESS;
		addr = (opcode & OPCODE_ADDRESS_MASK);

		switch (opcode & ~OPCODE_ADDRESS_MASK) {
		case OPCODE_MASKWRITE:
			numargs = 3;
			mask = ptr[1];
			val = ptr[2];
			iowrite((ioread(addr) & ~mask) | (val & mask), addr);
			break;

		case OPCODE_WRITE:
			numargs = 2;
			val = ptr[1];
			iowrite(val, addr);
			break;

		case OPCODE_MASKPOLL:
			numargs = 2;
			mask = ptr[1];
			i = 0;
			while (!(ioread(addr) & mask)) {
				if (i == PS7_MASK_POLL_TIME)
					return PS7_INIT_TIMEOUT;
				i++;
			}
			break;

		case OPCODE_MASKDELAY:
			numargs = 2;
			mask = ptr[1];
			delay = get_number_of_cycles_for_delay(mask);
			perf_reset_and_start_timer();
			while (ioread(addr) < delay)
				;
			break;

		default:
			return PS7_INIT_CORRUPT;
		}

		ptr += numargs;
	}
}

unsigned long __weak __maybe_unused ps7GetSiliconVersion(void)
{
	return zynq_get_silicon_version();
}
