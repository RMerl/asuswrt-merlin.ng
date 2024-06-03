/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Álvaro Fernández Rojas <noltari@gmail.com>
 * Copyright (C) 2015 - 2018 Texas Instruments Incorporated <www.ti.com>
 * Written by Mugunthan V N <mugunthanvnm@ti.com>
 *
 */

#ifndef _DMA_UCLASS_H
#define _DMA_UCLASS_H

/* See dma.h for background documentation. */

#include <dma.h>

struct ofnode_phandle_args;

/*
 * struct dma_ops - Driver model DMA operations
 *
 * The uclass interface is implemented by all DMA devices which use
 * driver model.
 */
struct dma_ops {
#ifdef CONFIG_DMA_CHANNELS
	/**
	 * of_xlate - Translate a client's device-tree (OF) DMA specifier.
	 *
	 * The DMA core calls this function as the first step in implementing
	 * a client's dma_get_by_*() call.
	 *
	 * If this function pointer is set to NULL, the DMA core will use a
	 * default implementation, which assumes #dma-cells = <1>, and that
	 * the DT cell contains a simple integer DMA Channel.
	 *
	 * At present, the DMA API solely supports device-tree. If this
	 * changes, other xxx_xlate() functions may be added to support those
	 * other mechanisms.
	 *
	 * @dma: The dma struct to hold the translation result.
	 * @args:	The dma specifier values from device tree.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*of_xlate)(struct dma *dma,
			struct ofnode_phandle_args *args);
	/**
	 * request - Request a translated DMA.
	 *
	 * The DMA core calls this function as the second step in
	 * implementing a client's dma_get_by_*() call, following a successful
	 * xxx_xlate() call, or as the only step in implementing a client's
	 * dma_request() call.
	 *
	 * @dma: The DMA struct to request; this has been filled in by
	 *   a previoux xxx_xlate() function call, or by the caller of
	 *   dma_request().
	 * @return 0 if OK, or a negative error code.
	 */
	int (*request)(struct dma *dma);
	/**
	 * free - Free a previously requested dma.
	 *
	 * This is the implementation of the client dma_free() API.
	 *
	 * @dma: The DMA to free.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*free)(struct dma *dma);
	/**
	 * enable() - Enable a DMA Channel.
	 *
	 * @dma: The DMA Channel to manipulate.
	 * @return zero on success, or -ve error code.
	 */
	int (*enable)(struct dma *dma);
	/**
	 * disable() - Disable a DMA Channel.
	 *
	 * @dma: The DMA Channel to manipulate.
	 * @return zero on success, or -ve error code.
	 */
	int (*disable)(struct dma *dma);
	/**
	 * prepare_rcv_buf() - Prepare/Add receive DMA buffer.
	 *
	 * @dma: The DMA Channel to manipulate.
	 * @dst: The receive buffer pointer.
	 * @size: The receive buffer size
	 * @return zero on success, or -ve error code.
	 */
	int (*prepare_rcv_buf)(struct dma *dma, void *dst, size_t size);
	/**
	 * receive() - Receive a DMA transfer.
	 *
	 * @dma: The DMA Channel to manipulate.
	 * @dst: The destination pointer.
	 * @metadata: DMA driver's specific data
	 * @return zero on success, or -ve error code.
	 */
	int (*receive)(struct dma *dma, void **dst, void *metadata);
	/**
	 * send() - Send a DMA transfer.
	 *
	 * @dma: The DMA Channel to manipulate.
	 * @src: The source pointer.
	 * @len: Length of the data to be sent (number of bytes).
	 * @metadata: DMA driver's specific data
	 * @return zero on success, or -ve error code.
	 */
	int (*send)(struct dma *dma, void *src, size_t len, void *metadata);
#endif /* CONFIG_DMA_CHANNELS */
	/**
	 * transfer() - Issue a DMA transfer. The implementation must
	 *   wait until the transfer is done.
	 *
	 * @dev: The DMA device
	 * @direction: direction of data transfer (should be one from
	 *   enum dma_direction)
	 * @dst: The destination pointer.
	 * @src: The source pointer.
	 * @len: Length of the data to be copied (number of bytes).
	 * @return zero on success, or -ve error code.
	 */
	int (*transfer)(struct udevice *dev, int direction, void *dst,
			void *src, size_t len);
};

#endif /* _DMA_UCLASS_H */
