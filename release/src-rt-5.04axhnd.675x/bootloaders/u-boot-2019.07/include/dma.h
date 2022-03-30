/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Álvaro Fernández Rojas <noltari@gmail.com>
 * Copyright (C) 2015 - 2018 Texas Instruments Incorporated <www.ti.com>
 * Written by Mugunthan V N <mugunthanvnm@ti.com>
 *
 */

#ifndef _DMA_H_
#define _DMA_H_

#include <linux/errno.h>
#include <linux/types.h>

/*
 * enum dma_direction - dma transfer direction indicator
 * @DMA_MEM_TO_MEM: Memcpy mode
 * @DMA_MEM_TO_DEV: From Memory to Device
 * @DMA_DEV_TO_MEM: From Device to Memory
 * @DMA_DEV_TO_DEV: From Device to Device
 */
enum dma_direction {
	DMA_MEM_TO_MEM,
	DMA_MEM_TO_DEV,
	DMA_DEV_TO_MEM,
	DMA_DEV_TO_DEV,
};

#define DMA_SUPPORTS_MEM_TO_MEM	BIT(0)
#define DMA_SUPPORTS_MEM_TO_DEV	BIT(1)
#define DMA_SUPPORTS_DEV_TO_MEM	BIT(2)
#define DMA_SUPPORTS_DEV_TO_DEV	BIT(3)

/*
 * struct dma_dev_priv - information about a device used by the uclass
 *
 * @supported: mode of transfers that DMA can support, should be
 *	       one/multiple of DMA_SUPPORTS_*
 */
struct dma_dev_priv {
	u32 supported;
};

#ifdef CONFIG_DMA_CHANNELS
/**
 * A DMA is a feature of computer systems that allows certain hardware
 * subsystems to access main system memory, independent of the CPU.
 * DMA channels are typically generated externally to the HW module
 * consuming them, by an entity this API calls a DMA provider. This API
 * provides a standard means for drivers to enable and disable DMAs, and to
 * copy, send and receive data using DMA.
 *
 * A driver that implements UCLASS_DMA is a DMA provider. A provider will
 * often implement multiple separate DMAs, since the hardware it manages
 * often has this capability. dma_uclass.h describes the interface which
 * DMA providers must implement.
 *
 * DMA consumers/clients are the HW modules driven by the DMA channels. This
 * header file describes the API used by drivers for those HW modules.
 *
 * DMA consumer DMA_MEM_TO_DEV (transmit) usage example (based on networking).
 * Note. dma_send() is sync operation always -  it'll start transfer and will
 * poll for it to complete:
 *	- get/request dma channel
 *	struct dma dma_tx;
 *	ret = dma_get_by_name(common->dev, "tx0", &dma_tx);
 *	if (ret) ...
 *
 *	- enable dma channel
 *	ret = dma_enable(&dma_tx);
 *	if (ret) ...
 *
 *	- dma transmit DMA_MEM_TO_DEV.
 *	struct ti_drv_packet_data packet_data;
 *
 *	packet_data.opt1 = val1;
 *	packet_data.opt2 = val2;
 *	ret = dma_send(&dma_tx, packet, length, &packet_data);
 *	if (ret) ..
 *
 * DMA consumer DMA_DEV_TO_MEM (receive) usage example (based on networking).
 * Note. dma_receive() is sync operation always - it'll start transfer
 * (if required) and will poll for it to complete (or for any previously
 * configured dev2mem transfer to complete):
 *	- get/request dma channel
 *	struct dma dma_rx;
 *	ret = dma_get_by_name(common->dev, "rx0", &dma_rx);
 *	if (ret) ...
 *
 *	- enable dma channel
 *	ret = dma_enable(&dma_rx);
 *	if (ret) ...
 *
 *	- dma receive DMA_DEV_TO_MEM.
 *	struct ti_drv_packet_data packet_data;
 *
 *	len = dma_receive(&dma_rx, (void **)packet, &packet_data);
 *	if (ret < 0) ...
 *
 * DMA consumer DMA_DEV_TO_MEM (receive) zero-copy usage example (based on
 * networking). Networking subsystem allows to configure and use few receive
 * buffers (dev2mem), as Networking RX DMA channels usually implemented
 * as streaming interface
 *	- get/request dma channel
 *	struct dma dma_rx;
 *	ret = dma_get_by_name(common->dev, "rx0", &dma_rx);
 *	if (ret) ...
 *
 *	for (i = 0; i < RX_DESC_NUM; i++) {
 *		ret = dma_prepare_rcv_buf(&dma_rx,
 *					  net_rx_packets[i],
 *					  RX_BUF_SIZE);
 *		if (ret) ...
 *	}
 *
 *	- enable dma channel
 *	ret = dma_enable(&dma_rx);
 *	if (ret) ...
 *
 *	- dma receive DMA_DEV_TO_MEM.
 *	struct ti_drv_packet_data packet_data;
 *
 *	len = dma_receive(&dma_rx, (void **)packet, &packet_data);
 *	if (ret < 0) ..
 *
 *	-- process packet --
 *
 *	- return buffer back to DAM channel
 *	ret = dma_prepare_rcv_buf(&dma_rx,
 *				  net_rx_packets[rx_next],
 *				  RX_BUF_SIZE);
 */

struct udevice;

/**
 * struct dma - A handle to (allowing control of) a single DMA.
 *
 * Clients provide storage for DMA handles. The content of the structure is
 * managed solely by the DMA API and DMA drivers. A DMA struct is
 * initialized by "get"ing the DMA struct. The DMA struct is passed to all
 * other DMA APIs to identify which DMA channel to operate upon.
 *
 * @dev: The device which implements the DMA channel.
 * @id: The DMA channel ID within the provider.
 *
 * Currently, the DMA API assumes that a single integer ID is enough to
 * identify and configure any DMA channel for any DMA provider. If this
 * assumption becomes invalid in the future, the struct could be expanded to
 * either (a) add more fields to allow DMA providers to store additional
 * information, or (b) replace the id field with an opaque pointer, which the
 * provider would dynamically allocated during its .of_xlate op, and process
 * during is .request op. This may require the addition of an extra op to clean
 * up the allocation.
 */
struct dma {
	struct udevice *dev;
	/*
	 * Written by of_xlate. We assume a single id is enough for now. In the
	 * future, we might add more fields here.
	 */
	unsigned long id;
};

# if CONFIG_IS_ENABLED(OF_CONTROL) && CONFIG_IS_ENABLED(DMA)
/**
 * dma_get_by_index - Get/request a DMA by integer index.
 *
 * This looks up and requests a DMA. The index is relative to the client
 * device; each device is assumed to have n DMAs associated with it somehow,
 * and this function finds and requests one of them. The mapping of client
 * device DMA indices to provider DMAs may be via device-tree properties,
 * board-provided mapping tables, or some other mechanism.
 *
 * @dev:	The client device.
 * @index:	The index of the DMA to request, within the client's list of
 *		DMA channels.
 * @dma:	A pointer to a DMA struct to initialize.
 * @return 0 if OK, or a negative error code.
 */
int dma_get_by_index(struct udevice *dev, int index, struct dma *dma);

/**
 * dma_get_by_name - Get/request a DMA by name.
 *
 * This looks up and requests a DMA. The name is relative to the client
 * device; each device is assumed to have n DMAs associated with it somehow,
 * and this function finds and requests one of them. The mapping of client
 * device DMA names to provider DMAs may be via device-tree properties,
 * board-provided mapping tables, or some other mechanism.
 *
 * @dev:	The client device.
 * @name:	The name of the DMA to request, within the client's list of
 *		DMA channels.
 * @dma:	A pointer to a DMA struct to initialize.
 * @return 0 if OK, or a negative error code.
 */
int dma_get_by_name(struct udevice *dev, const char *name, struct dma *dma);
# else
static inline int dma_get_by_index(struct udevice *dev, int index,
				   struct dma *dma)
{
	return -ENOSYS;
}

static inline int dma_get_by_name(struct udevice *dev, const char *name,
				  struct dma *dma)
{
	return -ENOSYS;
}
# endif

/**
 * dma_request - Request a DMA by provider-specific ID.
 *
 * This requests a DMA using a provider-specific ID. Generally, this function
 * should not be used, since dma_get_by_index/name() provide an interface that
 * better separates clients from intimate knowledge of DMA providers.
 * However, this function may be useful in core SoC-specific code.
 *
 * @dev: The DMA provider device.
 * @dma: A pointer to a DMA struct to initialize. The caller must
 *	 have already initialized any field in this struct which the
 *	 DMA provider uses to identify the DMA channel.
 * @return 0 if OK, or a negative error code.
 */
int dma_request(struct udevice *dev, struct dma *dma);

/**
 * dma_free - Free a previously requested DMA.
 *
 * @dma: A DMA struct that was previously successfully requested by
 *	 dma_request/get_by_*().
 * @return 0 if OK, or a negative error code.
 */
int dma_free(struct dma *dma);

/**
 * dma_enable() - Enable (turn on) a DMA channel.
 *
 * @dma: A DMA struct that was previously successfully requested by
 *	 dma_request/get_by_*().
 * @return zero on success, or -ve error code.
 */
int dma_enable(struct dma *dma);

/**
 * dma_disable() - Disable (turn off) a DMA channel.
 *
 * @dma: A DMA struct that was previously successfully requested by
 *	 dma_request/get_by_*().
 * @return zero on success, or -ve error code.
 */
int dma_disable(struct dma *dma);

/**
 * dma_prepare_rcv_buf() - Prepare/add receive DMA buffer.
 *
 * It allows to implement zero-copy async DMA_DEV_TO_MEM (receive) transactions
 * if supported by DMA providers.
 *
 * @dma: A DMA struct that was previously successfully requested by
 *	 dma_request/get_by_*().
 * @dst: The receive buffer pointer.
 * @size: The receive buffer size
 * @return zero on success, or -ve error code.
 */
int dma_prepare_rcv_buf(struct dma *dma, void *dst, size_t size);

/**
 * dma_receive() - Receive a DMA transfer.
 *
 * @dma: A DMA struct that was previously successfully requested by
 *	 dma_request/get_by_*().
 * @dst: The destination pointer.
 * @metadata: DMA driver's channel specific data
 * @return length of received data on success, or zero - no data,
 * or -ve error code.
 */
int dma_receive(struct dma *dma, void **dst, void *metadata);

/**
 * dma_send() - Send a DMA transfer.
 *
 * @dma: A DMA struct that was previously successfully requested by
 *	 dma_request/get_by_*().
 * @src: The source pointer.
 * @len: Length of the data to be sent (number of bytes).
 * @metadata: DMA driver's channel specific data
 * @return zero on success, or -ve error code.
 */
int dma_send(struct dma *dma, void *src, size_t len, void *metadata);
#endif /* CONFIG_DMA_CHANNELS */

/*
 * dma_get_device - get a DMA device which supports transfer
 * type of transfer_type
 *
 * @transfer_type - transfer type should be one/multiple of
 *		    DMA_SUPPORTS_*
 * @devp - udevice pointer to return the found device
 * @return - will return on success and devp will hold the
 *	     pointer to the device
 */
int dma_get_device(u32 transfer_type, struct udevice **devp);

/*
 * dma_memcpy - try to use DMA to do a mem copy which will be
 *		much faster than CPU mem copy
 *
 * @dst - destination pointer
 * @src - souce pointer
 * @len - data length to be copied
 * @return - on successful transfer returns no of bytes
	     transferred and on failure return error code.
 */
int dma_memcpy(void *dst, void *src, size_t len);

#endif	/* _DMA_H_ */
