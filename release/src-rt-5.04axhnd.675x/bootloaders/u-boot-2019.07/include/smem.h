/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * The shared memory system is an allocate-only heap structure that
 * consists of one of more memory areas that can be accessed by the processors
 * in the SoC.
 *
 * Allocation can be done globally for all processors or to an individual processor.
 * This is controlled by the @host parameter.
 *
 * Allocation and management of heap can be implemented in various ways,
 * The @item parameter should be used as an index/hash to the memory region.
 *
 * Copyright (c) 2018 Ramon Fried <ramon.fried@gmail.com>
 */

#ifndef _smemh_
#define _smemh_

/* struct smem_ops: Operations for the SMEM uclass */
struct smem_ops {
	/**
	 * alloc() - allocate space for a smem item
	 *
	 * @host:	remote processor id, or -1 for all processors.
	 * @item:	smem item handle
	 * @size:	number of bytes to be allocated
	 * @return 0 if OK, -ve on error
	 */
	int (*alloc)(unsigned int host,
		unsigned int item, size_t size);

	/**
	 * get() - Resolve ptr of size of a smem item
	 *
	 * @host:	the remote processor, of -1 for all processors.
	 * @item:	smem item handle
	 * @size:	pointer to be filled out with the size of the item
	 * @return	pointer on success, NULL on error
	 */
	void *(*get)(unsigned int host,
		unsigned int item, size_t *size);

	/**
	 * get_free_space() - Get free space in smem in bytes
	 *
	 * @host:   the remote processor identifying a partition, or -1
	 *			for all processors.
	 * @return	free space, -ve on error
	 */
	int (*get_free_space)(unsigned int host);
};

#define smem_get_ops(dev)	((struct smem_ops *)(dev)->driver->ops)

/**
 * smem_alloc() - allocate space for a smem item
 * @host:	remote processor id, or -1
 * @item:	smem item handle
 * @size:	number of bytes to be allocated
 * @return 0 if OK, -ve on error
 *
 * Allocate space for a given smem item of size @size, given that the item is
 * not yet allocated.
 */
int smem_alloc(struct udevice *dev, unsigned int host, unsigned int item, size_t size);

/**
 * smem_get() - resolve ptr of size of a smem item
 * @host:	the remote processor, or -1 for all processors.
 * @item:	smem item handle
 * @size:	pointer to be filled out with size of the item
 * @return	pointer on success, NULL on error
 *
 * Looks up smem item and returns pointer to it. Size of smem
 * item is returned in @size.
 */
void *smem_get(struct udevice *dev, unsigned int host, unsigned int item, size_t *size);

/**
 * smem_get_free_space() - retrieve amount of free space in a partition
 * @host:	the remote processor identifying a partition, or -1
 *			for all processors.
 * @return	size in bytes, -ve on error
 *
 * To be used by smem clients as a quick way to determine if any new
 * allocations has been made.
 */
int smem_get_free_space(struct udevice *dev, unsigned int host);

#endif /* _smem_h_ */

