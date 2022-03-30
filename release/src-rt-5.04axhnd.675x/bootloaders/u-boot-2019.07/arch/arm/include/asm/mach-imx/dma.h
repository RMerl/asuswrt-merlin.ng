/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX28 APBH DMA
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * Based on code from LTIB:
 * Copyright 2008-2010 Freescale Semiconductor, Inc. All Rights Reserved.
 */

#ifndef __DMA_H__
#define __DMA_H__

#include <linux/list.h>
#include <linux/compiler.h>

#define DMA_PIO_WORDS		15
#define MXS_DMA_ALIGNMENT	ARCH_DMA_MINALIGN

/*
 * MXS DMA channels
 */
#if defined(CONFIG_MX23)
enum {
	MXS_DMA_CHANNEL_AHB_APBH_LCDIF = 0,
	MXS_DMA_CHANNEL_AHB_APBH_SSP0,
	MXS_DMA_CHANNEL_AHB_APBH_SSP1,
	MXS_DMA_CHANNEL_AHB_APBH_RESERVED0,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI0,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI1,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI2,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI3,
	MXS_MAX_DMA_CHANNELS,
};
#elif defined(CONFIG_MX28)
enum {
	MXS_DMA_CHANNEL_AHB_APBH_SSP0 = 0,
	MXS_DMA_CHANNEL_AHB_APBH_SSP1,
	MXS_DMA_CHANNEL_AHB_APBH_SSP2,
	MXS_DMA_CHANNEL_AHB_APBH_SSP3,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI0,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI1,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI2,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI3,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI4,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI5,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI6,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI7,
	MXS_DMA_CHANNEL_AHB_APBH_HSADC,
	MXS_DMA_CHANNEL_AHB_APBH_LCDIF,
	MXS_DMA_CHANNEL_AHB_APBH_RESERVED0,
	MXS_DMA_CHANNEL_AHB_APBH_RESERVED1,
	MXS_MAX_DMA_CHANNELS,
};
#elif defined(CONFIG_MX6) || defined(CONFIG_MX7)
enum {
	MXS_DMA_CHANNEL_AHB_APBH_GPMI0 = 0,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI1,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI2,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI3,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI4,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI5,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI6,
	MXS_DMA_CHANNEL_AHB_APBH_GPMI7,
	MXS_MAX_DMA_CHANNELS,
};
#endif

/*
 * MXS DMA hardware command.
 *
 * This structure describes the in-memory layout of an entire DMA command,
 * including space for the maximum number of PIO accesses. See the appropriate
 * reference manual for a detailed description of what these fields mean to the
 * DMA hardware.
 */
#define	MXS_DMA_DESC_COMMAND_MASK	0x3
#define	MXS_DMA_DESC_COMMAND_OFFSET	0
#define	MXS_DMA_DESC_COMMAND_NO_DMAXFER	0x0
#define	MXS_DMA_DESC_COMMAND_DMA_WRITE	0x1
#define	MXS_DMA_DESC_COMMAND_DMA_READ	0x2
#define	MXS_DMA_DESC_COMMAND_DMA_SENSE	0x3
#define	MXS_DMA_DESC_CHAIN		(1 << 2)
#define	MXS_DMA_DESC_IRQ		(1 << 3)
#define	MXS_DMA_DESC_NAND_LOCK		(1 << 4)
#define	MXS_DMA_DESC_NAND_WAIT_4_READY	(1 << 5)
#define	MXS_DMA_DESC_DEC_SEM		(1 << 6)
#define	MXS_DMA_DESC_WAIT4END		(1 << 7)
#define	MXS_DMA_DESC_HALT_ON_TERMINATE	(1 << 8)
#define	MXS_DMA_DESC_TERMINATE_FLUSH	(1 << 9)
#define	MXS_DMA_DESC_PIO_WORDS_MASK	(0xf << 12)
#define	MXS_DMA_DESC_PIO_WORDS_OFFSET	12
#define	MXS_DMA_DESC_BYTES_MASK		(0xffff << 16)
#define	MXS_DMA_DESC_BYTES_OFFSET	16

struct mxs_dma_cmd {
	unsigned long		next;
	unsigned long		data;
	union {
		dma_addr_t	address;
		unsigned long	alternate;
	};
	unsigned long		pio_words[DMA_PIO_WORDS];
};

/*
 * MXS DMA command descriptor.
 *
 * This structure incorporates an MXS DMA hardware command structure, along
 * with metadata.
 */
#define	MXS_DMA_DESC_FIRST	(1 << 0)
#define	MXS_DMA_DESC_LAST	(1 << 1)
#define	MXS_DMA_DESC_READY	(1 << 31)

struct mxs_dma_desc {
	struct mxs_dma_cmd	cmd;
	unsigned int		flags;
	dma_addr_t		address;
	void			*buffer;
	struct list_head	node;
} __aligned(MXS_DMA_ALIGNMENT);

/**
 * MXS DMA channel
 *
 * This structure represents a single DMA channel. The MXS platform code
 * maintains an array of these structures to represent every DMA channel in the
 * system (see mxs_dma_channels).
 */
#define	MXS_DMA_FLAGS_IDLE	0
#define	MXS_DMA_FLAGS_BUSY	(1 << 0)
#define	MXS_DMA_FLAGS_FREE	0
#define	MXS_DMA_FLAGS_ALLOCATED	(1 << 16)
#define	MXS_DMA_FLAGS_VALID	(1 << 31)

struct mxs_dma_chan {
	const char *name;
	unsigned long dev;
	struct mxs_dma_device *dma;
	unsigned int flags;
	unsigned int active_num;
	unsigned int pending_num;
	struct list_head active;
	struct list_head done;
};

struct mxs_dma_desc *mxs_dma_desc_alloc(void);
void mxs_dma_desc_free(struct mxs_dma_desc *);
int mxs_dma_desc_append(int channel, struct mxs_dma_desc *pdesc);

int mxs_dma_go(int chan);
void mxs_dma_init(void);
int mxs_dma_init_channel(int chan);
int mxs_dma_release(int chan);

void mxs_dma_circ_start(int chan, struct mxs_dma_desc *pdesc);

#endif	/* __DMA_H__ */
