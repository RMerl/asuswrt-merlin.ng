/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * TI K3 AM65x NAVSS Ring accelerator Manager (RA) subsystem driver
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com
 */

#ifndef __SOC_TI_K3_NAVSS_RINGACC_API_H_
#define __SOC_TI_K3_NAVSS_RINGACC_API_H_

#include <dm/ofnode.h>

/**
 * enum k3_nav_ring_mode - &struct k3_nav_ring_cfg mode
 *
 * RA ring operational modes
 *
 * @K3_NAV_RINGACC_RING_MODE_RING: Exposed Ring mode for SW direct access
 * @K3_NAV_RINGACC_RING_MODE_MESSAGE: Messaging mode. Messaging mode requires
 *	that all accesses to the queue must go through this IP so that all
 *	accesses to the memory are controlled and ordered. This IP then
 *	controls the entire state of the queue, and SW has no directly control,
 *	such as through doorbells and cannot access the storage memory directly.
 *	This is particularly useful when more than one SW or HW entity can be
 *	the producer and/or consumer at the same time
 * @K3_NAV_RINGACC_RING_MODE_CREDENTIALS: Credentials mode is message mode plus
 *	stores credentials with each message, requiring the element size to be
 *	doubled to fit the credentials. Any exposed memory should be protected
 *	by a firewall from unwanted access
 * @K3_NAV_RINGACC_RING_MODE_QM:  Queue manager mode. This takes the credentials
 *	mode and adds packet length per element, along with additional read only
 *	fields for element count and accumulated queue length. The QM mode only
 *	operates with an 8 byte element size (any other element size is
 *	illegal), and like in credentials mode each operation uses 2 element
 *	slots to store the credentials and length fields
 */
enum k3_nav_ring_mode {
	K3_NAV_RINGACC_RING_MODE_RING = 0,
	K3_NAV_RINGACC_RING_MODE_MESSAGE,
	K3_NAV_RINGACC_RING_MODE_CREDENTIALS,
	K3_NAV_RINGACC_RING_MODE_QM,
	k3_NAV_RINGACC_RING_MODE_INVALID
};

/**
 * enum k3_nav_ring_size - &struct k3_nav_ring_cfg elm_size
 *
 * RA ring element's sizes in bytes.
 */
enum k3_nav_ring_size {
	K3_NAV_RINGACC_RING_ELSIZE_4 = 0,
	K3_NAV_RINGACC_RING_ELSIZE_8,
	K3_NAV_RINGACC_RING_ELSIZE_16,
	K3_NAV_RINGACC_RING_ELSIZE_32,
	K3_NAV_RINGACC_RING_ELSIZE_64,
	K3_NAV_RINGACC_RING_ELSIZE_128,
	K3_NAV_RINGACC_RING_ELSIZE_256,
	K3_NAV_RINGACC_RING_ELSIZE_INVALID
};

struct k3_nav_ringacc;
struct k3_nav_ring;

/**
 * enum k3_nav_ring_cfg - RA ring configuration structure
 *
 * @size: Ring size, number of elements
 * @elm_size: Ring element size
 * @mode: Ring operational mode
 * @flags: Ring configuration flags. Possible values:
 *	 @K3_NAV_RINGACC_RING_SHARED: when set allows to request the same ring
 *	 few times. It's usable when the same ring is used as Free Host PD ring
 *	 for different flows, for example.
 *	 Note: Locking should be done by consumer if required
 */
struct k3_nav_ring_cfg {
	u32 size;
	enum k3_nav_ring_size elm_size;
	enum k3_nav_ring_mode mode;
#define K3_NAV_RINGACC_RING_SHARED BIT(1)
	u32 flags;
};

#define K3_NAV_RINGACC_RING_ID_ANY (-1)
#define K3_NAV_RINGACC_RING_USE_PROXY BIT(1)

/**
 * k3_nav_ringacc_request_ring - request ring from ringacc
 * @ringacc: pointer on ringacc
 * @id: ring id or K3_NAV_RINGACC_RING_ID_ANY for any general purpose ring
 * @flags:
 *	@K3_NAV_RINGACC_RING_USE_PROXY: if set - proxy will be allocated and
 *		used to access ring memory. Sopported only for rings in
 *		Message/Credentials/Queue mode.
 *
 * Returns pointer on the Ring - struct k3_nav_ring
 * or NULL in case of failure.
 */
struct k3_nav_ring *k3_nav_ringacc_request_ring(struct k3_nav_ringacc *ringacc,
						int id, u32 flags);

/**
 * k3_nav_ringacc_get_dev - get pointer on RA device
 * @ringacc: pointer on RA
 *
 * Returns device pointer
 */
struct udevice *k3_nav_ringacc_get_dev(struct k3_nav_ringacc *ringacc);

/**
 * k3_nav_ringacc_ring_reset - ring reset
 * @ring: pointer on Ring
 *
 * Resets ring internal state ((hw)occ, (hw)idx).
 * TODO_GS: ? Ring can be reused without reconfiguration
 */
void k3_nav_ringacc_ring_reset(struct k3_nav_ring *ring);
/**
 * k3_nav_ringacc_ring_reset - ring reset for DMA rings
 * @ring: pointer on Ring
 *
 * Resets ring internal state ((hw)occ, (hw)idx). Should be used for rings
 * which are read by K3 UDMA, like TX or Free Host PD rings.
 */
void k3_nav_ringacc_ring_reset_dma(struct k3_nav_ring *ring, u32 occ);

/**
 * k3_nav_ringacc_ring_free - ring free
 * @ring: pointer on Ring
 *
 * Resets ring and free all alocated resources.
 */
int k3_nav_ringacc_ring_free(struct k3_nav_ring *ring);

/**
 * k3_nav_ringacc_get_ring_id - Get the Ring ID
 * @ring: pointer on ring
 *
 * Returns the Ring ID
 */
u32 k3_nav_ringacc_get_ring_id(struct k3_nav_ring *ring);

/**
 * k3_nav_ringacc_ring_cfg - ring configure
 * @ring: pointer on ring
 * @cfg: Ring configuration parameters (see &struct k3_nav_ring_cfg)
 *
 * Configures ring, including ring memory allocation.
 * Returns 0 on success, errno otherwise.
 */
int k3_nav_ringacc_ring_cfg(struct k3_nav_ring *ring,
			    struct k3_nav_ring_cfg *cfg);

/**
 * k3_nav_ringacc_ring_get_size - get ring size
 * @ring: pointer on ring
 *
 * Returns ring size in number of elements.
 */
u32 k3_nav_ringacc_ring_get_size(struct k3_nav_ring *ring);

/**
 * k3_nav_ringacc_ring_get_free - get free elements
 * @ring: pointer on ring
 *
 * Returns number of free elements in the ring.
 */
u32 k3_nav_ringacc_ring_get_free(struct k3_nav_ring *ring);

/**
 * k3_nav_ringacc_ring_get_occ - get ring occupancy
 * @ring: pointer on ring
 *
 * Returns total number of valid entries on the ring
 */
u32 k3_nav_ringacc_ring_get_occ(struct k3_nav_ring *ring);

/**
 * k3_nav_ringacc_ring_is_full - checks if ring is full
 * @ring: pointer on ring
 *
 * Returns true if the ring is full
 */
u32 k3_nav_ringacc_ring_is_full(struct k3_nav_ring *ring);

/**
 * k3_nav_ringacc_ring_push - push element to the ring tail
 * @ring: pointer on ring
 * @elem: pointer on ring element buffer
 *
 * Push one ring element to the ring tail. Size of the ring element is
 * determined by ring configuration &struct k3_nav_ring_cfg elm_size.
 *
 * Returns 0 on success, errno otherwise.
 */
int k3_nav_ringacc_ring_push(struct k3_nav_ring *ring, void *elem);

/**
 * k3_nav_ringacc_ring_pop - pop element from the ring head
 * @ring: pointer on ring
 * @elem: pointer on ring element buffer
 *
 * Push one ring element from the ring head. Size of the ring element is
 * determined by ring configuration &struct k3_nav_ring_cfg elm_size..
 *
 * Returns 0 on success, errno otherwise.
 */
int k3_nav_ringacc_ring_pop(struct k3_nav_ring *ring, void *elem);

/**
 * k3_nav_ringacc_ring_push_head - push element to the ring head
 * @ring: pointer on ring
 * @elem: pointer on ring element buffer
 *
 * Push one ring element to the ring head. Size of the ring element is
 * determined by ring configuration &struct k3_nav_ring_cfg elm_size.
 *
 * Returns 0 on success, errno otherwise.
 * Not Supported by ring modes: K3_NAV_RINGACC_RING_MODE_RING
 */
int k3_nav_ringacc_ring_push_head(struct k3_nav_ring *ring, void *elem);

/**
 * k3_nav_ringacc_ring_pop_tail - pop element from the ring tail
 * @ring: pointer on ring
 * @elem: pointer on ring element buffer
 *
 * Push one ring element from the ring tail. Size of the ring element is
 * determined by ring configuration &struct k3_nav_ring_cfg elm_size.
 *
 * Returns 0 on success, errno otherwise.
 * Not Supported by ring modes: K3_NAV_RINGACC_RING_MODE_RING
 */
int k3_nav_ringacc_ring_pop_tail(struct k3_nav_ring *ring, void *elem);

#endif /* __SOC_TI_K3_NAVSS_RINGACC_API_H_ */
