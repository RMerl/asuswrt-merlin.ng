// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2004,2007,2008 Freescale Semiconductor, Inc.
 * (C) Copyright 2002, 2003 Motorola Inc.
 * Xianghua Xiao (X.Xiao@motorola.com)
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/fsl_dma.h>

/* Controller can only transfer 2^26 - 1 bytes at a time */
#define FSL_DMA_MAX_SIZE	(0x3ffffff)

#if defined(CONFIG_MPC83xx)
#define FSL_DMA_MR_DEFAULT (FSL_DMA_MR_CTM_DIRECT | FSL_DMA_MR_DMSEN)
#else
#define FSL_DMA_MR_DEFAULT (FSL_DMA_MR_BWC_DIS | FSL_DMA_MR_CTM_DIRECT)
#endif


#if defined(CONFIG_MPC83xx)
dma83xx_t *dma_base = (void *)(CONFIG_SYS_MPC83xx_DMA_ADDR);
#elif defined(CONFIG_MPC85xx)
ccsr_dma_t *dma_base = (void *)(CONFIG_SYS_MPC85xx_DMA_ADDR);
#elif defined(CONFIG_MPC86xx)
ccsr_dma_t *dma_base = (void *)(CONFIG_SYS_MPC86xx_DMA_ADDR);
#else
#error "Freescale DMA engine not supported on your processor"
#endif

static void dma_sync(void)
{
#if defined(CONFIG_MPC85xx)
	asm("sync; isync; msync");
#elif defined(CONFIG_MPC86xx)
	asm("sync; isync");
#endif
}

static void out_dma32(volatile unsigned *addr, int val)
{
#if defined(CONFIG_MPC83xx)
	out_le32(addr, val);
#else
	out_be32(addr, val);
#endif
}

static uint in_dma32(volatile unsigned *addr)
{
#if defined(CONFIG_MPC83xx)
	return in_le32(addr);
#else
	return in_be32(addr);
#endif
}

static uint dma_check(void) {
	volatile fsl_dma_t *dma = &dma_base->dma[0];
	uint status;

	/* While the channel is busy, spin */
	do {
		status = in_dma32(&dma->sr);
	} while (status & FSL_DMA_SR_CB);

	/* clear MR[CS] channel start bit */
	out_dma32(&dma->mr, in_dma32(&dma->mr) & ~FSL_DMA_MR_CS);
	dma_sync();

	if (status != 0)
		printf ("DMA Error: status = %x\n", status);

	return status;
}

#if !defined(CONFIG_MPC83xx)
void dma_init(void) {
	volatile fsl_dma_t *dma = &dma_base->dma[0];

	out_dma32(&dma->satr, FSL_DMA_SATR_SREAD_SNOOP);
	out_dma32(&dma->datr, FSL_DMA_DATR_DWRITE_SNOOP);
	out_dma32(&dma->sr, 0xffffffff); /* clear any errors */
	dma_sync();
}
#endif

int dmacpy(phys_addr_t dest, phys_addr_t src, phys_size_t count) {
	volatile fsl_dma_t *dma = &dma_base->dma[0];
	uint xfer_size;

	while (count) {
		xfer_size = min(FSL_DMA_MAX_SIZE, count);

		out_dma32(&dma->dar, (u32) (dest & 0xFFFFFFFF));
		out_dma32(&dma->sar, (u32) (src & 0xFFFFFFFF));
#if !defined(CONFIG_MPC83xx)
		out_dma32(&dma->satr,
			in_dma32(&dma->satr) | (u32)((u64)src >> 32));
		out_dma32(&dma->datr,
			in_dma32(&dma->datr) | (u32)((u64)dest >> 32));
#endif
		out_dma32(&dma->bcr, xfer_size);
		dma_sync();

		/* Prepare mode register */
		out_dma32(&dma->mr, FSL_DMA_MR_DEFAULT);
		dma_sync();

		/* Start the transfer */
		out_dma32(&dma->mr, FSL_DMA_MR_DEFAULT | FSL_DMA_MR_CS);

		count -= xfer_size;
		src += xfer_size;
		dest += xfer_size;

		dma_sync();

		if (dma_check())
			return -1;
	}

	return 0;
}

/*
 * 85xx/86xx use dma to initialize SDRAM when !CONFIG_ECC_INIT_VIA_DDRCONTROLLER
 * while 83xx uses dma to initialize SDRAM when CONFIG_DDR_ECC_INIT_VIA_DMA
 */
#if ((!defined CONFIG_MPC83xx && defined(CONFIG_DDR_ECC) &&	\
	!defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)) ||		\
	(defined(CONFIG_MPC83xx) && defined(CONFIG_DDR_ECC_INIT_VIA_DMA)))
void dma_meminit(uint val, uint size)
{
	uint *p = 0;
	uint i = 0;

	for (*p = 0; p < (uint *)(8 * 1024); p++) {
		if (((uint)p & 0x1f) == 0)
			ppcDcbz((ulong)p);

		*p = (uint)CONFIG_MEM_INIT_VALUE;

		if (((uint)p & 0x1c) == 0x1c)
			ppcDcbf((ulong)p);
	}

	dmacpy(0x002000, 0, 0x002000); /* 8K */
	dmacpy(0x004000, 0, 0x004000); /* 16K */
	dmacpy(0x008000, 0, 0x008000); /* 32K */
	dmacpy(0x010000, 0, 0x010000); /* 64K */
	dmacpy(0x020000, 0, 0x020000); /* 128K */
	dmacpy(0x040000, 0, 0x040000); /* 256K */
	dmacpy(0x080000, 0, 0x080000); /* 512K */
	dmacpy(0x100000, 0, 0x100000); /* 1M */
	dmacpy(0x200000, 0, 0x200000); /* 2M */
	dmacpy(0x400000, 0, 0x400000); /* 4M */

	for (i = 1; i < size / 0x800000; i++)
		dmacpy((0x800000 * i), 0, 0x800000);
}
#endif
