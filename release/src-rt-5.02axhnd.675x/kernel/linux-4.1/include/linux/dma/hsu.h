/*
 * Driver for the High Speed UART DMA
 *
 * Copyright (C) 2015 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _DMA_HSU_H
#define _DMA_HSU_H

#include <linux/device.h>
#include <linux/interrupt.h>

#include <linux/platform_data/dma-hsu.h>

struct hsu_dma;

/**
 * struct hsu_dma_chip - representation of HSU DMA hardware
 * @dev:		 struct device of the DMA controller
 * @irq:		 irq line
 * @regs:		 memory mapped I/O space
 * @length:		 I/O space length
 * @offset:		 offset of the I/O space where registers are located
 * @hsu:		 struct hsu_dma that is filed by ->probe()
 * @pdata:		 platform data for the DMA controller if provided
 */
struct hsu_dma_chip {
	struct device			*dev;
	int				irq;
	void __iomem			*regs;
	unsigned int			length;
	unsigned int			offset;
	struct hsu_dma			*hsu;
	struct hsu_dma_platform_data	*pdata;
};

/* Export to the internal users */
irqreturn_t hsu_dma_irq(struct hsu_dma_chip *chip, unsigned short nr);

/* Export to the platform drivers */
int hsu_dma_probe(struct hsu_dma_chip *chip);
int hsu_dma_remove(struct hsu_dma_chip *chip);

#endif /* _DMA_HSU_H */
