/*
 * Adapted for Motorola MPC8560 chips
 * Xianghua Xiao <x.xiao@motorola.com>
 *
 * This file is based on "arch/powerpc/8260_io/commproc.c" - here is it's
 * copyright notice:
 *
 * General Purpose functions for the global management of the
 * 8220 Communication Processor Module.
 * Copyright (c) 1999 Dan Malek (dmalek@jlc.net)
 * Copyright (c) 2000 MontaVista Software, Inc (source@mvista.com)
 *	2.3.99 Updates
 * Copyright (c) 2003 Motorola,Inc.
 *
 * In addition to the individual control of the communication
 * channels, there are a few functions that globally affect the
 * communication processor.
 *
 * Buffer descriptors must be allocated from the dual ported memory
 * space.  The allocator for that is here.  When the communication
 * process is reset, we reclaim the memory available.  There is
 * currently no deallocator for this memory.
 */
#include <common.h>
#include <asm/cpm_85xx.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * because we have stack and init data in dual port ram
 * we must reduce the size
 */
#undef	CPM_DATAONLY_SIZE
#define CPM_DATAONLY_SIZE	((uint)(8 * 1024) - CPM_DATAONLY_BASE)

void
m8560_cpm_reset(void)
{
	volatile ccsr_cpm_t *cpm = (ccsr_cpm_t *)CONFIG_SYS_MPC85xx_CPM_ADDR;
	volatile ulong count;

	gd = (gd_t *) (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET);

	/* Reclaim the DP memory for our use.
	*/
	gd->arch.dp_alloc_base = CPM_DATAONLY_BASE;
	gd->arch.dp_alloc_top = gd->arch.dp_alloc_base + CPM_DATAONLY_SIZE;

	/*
	 * Reset CPM
	 */
	cpm->im_cpm_cp.cpcr = CPM_CR_RST;
	count = 0;
	do {			/* Spin until command processed		*/
		__asm__ __volatile__ ("eieio");
	} while ((cpm->im_cpm_cp.cpcr & CPM_CR_FLG) && ++count < 1000000);
}

/* Allocate some memory from the dual ported ram.
 * To help protocols with object alignment restrictions, we do that
 * if they ask.
 */
uint
m8560_cpm_dpalloc(uint size, uint align)
{
	volatile ccsr_cpm_t *cpm = (ccsr_cpm_t *)CONFIG_SYS_MPC85xx_CPM_ADDR;
	uint	retloc;
	uint	align_mask, off;
	uint	savebase;

	align_mask = align - 1;
	savebase = gd->arch.dp_alloc_base;

	off = gd->arch.dp_alloc_base & align_mask;
	if (off != 0)
		gd->arch.dp_alloc_base += (align - off);

	if ((off = size & align_mask) != 0)
		size += align - off;

	if ((gd->arch.dp_alloc_base + size) >= gd->arch.dp_alloc_top) {
		gd->arch.dp_alloc_base = savebase;
		panic("m8560_cpm_dpalloc: ran out of dual port ram!");
	}

	retloc = gd->arch.dp_alloc_base;
	gd->arch.dp_alloc_base += size;

	memset((void *)&(cpm->im_dprambase[retloc]), 0, size);

	return(retloc);
}

/* We also own one page of host buffer space for the allocation of
 * UART "fifos" and the like.
 */
uint
m8560_cpm_hostalloc(uint size, uint align)
{
	/* the host might not even have RAM yet - just use dual port RAM */
	return (m8560_cpm_dpalloc(size, align));
}

/* Set a baud rate generator.  This needs lots of work.  There are
 * eight BRGs, which can be connected to the CPM channels or output
 * as clocks.  The BRGs are in two different block of internal
 * memory mapped space.
 * The baud rate clock is the system clock divided by something.
 * It was set up long ago during the initial boot phase and is
 * is given to us.
 * Baud rate clocks are zero-based in the driver code (as that maps
 * to port numbers).  Documentation uses 1-based numbering.
 */
#define BRG_INT_CLK	gd->arch.brg_clk
#define BRG_UART_CLK	((BRG_INT_CLK + 15) / 16)

/* This function is used by UARTS, or anything else that uses a 16x
 * oversampled clock.
 */
void
m8560_cpm_setbrg(uint brg, uint rate)
{
	volatile ccsr_cpm_t *cpm = (ccsr_cpm_t *)CONFIG_SYS_MPC85xx_CPM_ADDR;
	volatile uint	*bp;

	/* This is good enough to get SMCs running.....
	*/
	if (brg < 4) {
		bp = (uint *)&(cpm->im_cpm_brg1.brgc1);
	}
	else {
		bp = (uint *)&(cpm->im_cpm_brg2.brgc5);
		brg -= 4;
	}
	bp += brg;
	*bp = (((((BRG_UART_CLK+rate-1)/rate)-1)&0xfff)<<1)|CPM_BRG_EN;
}

/* This function is used to set high speed synchronous baud rate
 * clocks.
 */
void
m8560_cpm_fastbrg(uint brg, uint rate, int div16)
{
	volatile ccsr_cpm_t *cpm = (ccsr_cpm_t *)CONFIG_SYS_MPC85xx_CPM_ADDR;
	volatile uint	*bp;

	/* This is good enough to get SMCs running.....
	*/
	if (brg < 4) {
		bp = (uint *)&(cpm->im_cpm_brg1.brgc1);
	}
	else {
		bp = (uint *)&(cpm->im_cpm_brg2.brgc5);
		brg -= 4;
	}
	bp += brg;
	*bp = (((((BRG_INT_CLK+rate-1)/rate)-1)&0xfff)<<1)|CPM_BRG_EN;
	if (div16)
		*bp |= CPM_BRG_DIV16;
}

/* This function is used to set baud rate generators using an external
 * clock source and 16x oversampling.
 */

void
m8560_cpm_extcbrg(uint brg, uint rate, uint extclk, int pinsel)
{
	volatile ccsr_cpm_t *cpm = (ccsr_cpm_t *)CONFIG_SYS_MPC85xx_CPM_ADDR;
	volatile uint	*bp;

	if (brg < 4) {
		bp = (uint *)&(cpm->im_cpm_brg1.brgc1);
	}
	else {
		bp = (uint *)&(cpm->im_cpm_brg2.brgc5);
		brg -= 4;
	}
	bp += brg;
	*bp = ((((((extclk/16)+rate-1)/rate)-1)&0xfff)<<1)|CPM_BRG_EN;
	if (pinsel == 0)
		*bp |= CPM_BRG_EXTC_CLK3_9;
	else
		*bp |= CPM_BRG_EXTC_CLK5_15;
}
