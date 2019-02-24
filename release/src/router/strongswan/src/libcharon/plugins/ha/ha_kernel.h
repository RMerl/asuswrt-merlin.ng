/*
 * Copyright (C) 2009 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup ha_kernel ha_kernel
 * @{ @ingroup ha
 */

#ifndef HA_KERNEL_H_
#define HA_KERNEL_H_

typedef struct ha_kernel_t ha_kernel_t;

#include "ha_segments.h"

/**
 * HA segment kernel configuration interface.
 */
struct ha_kernel_t {

	/**
	 * Get the segment a host is in.
	 *
	 * @param host		host to get segment for
	 * @return			segment number
	 */
	u_int (*get_segment)(ha_kernel_t *this, host_t *host);

	/**
	 * Get the segment a host/SPI is in, as used for CHILD_SA segmentation.
	 *
	 * @param host		host to get segment for
	 * @param spi		SPI to include in hash
	 * @return			segment number
	 */
	u_int (*get_segment_spi)(ha_kernel_t *this, host_t *host, uint32_t spi);

	/**
	 * Get the segment an arbitrary integer is in.
	 *
	 * @param n			integer to segmentate
	 */
	u_int (*get_segment_int)(ha_kernel_t *this, int n);

	/**
	 * Activate a segment at kernel level for all cluster addresses.
	 *
	 * @param segment	segment to activate
	 */
	void (*activate)(ha_kernel_t *this, u_int segment);

	/**
	 * Deactivate a segment at kernel level for all cluster addresses.
	 *
	 * @param segment	segment to deactivate
	 */
	void (*deactivate)(ha_kernel_t *this, u_int segment);

	/**
	 * Destroy a ha_kernel_t.
	 */
	void (*destroy)(ha_kernel_t *this);
};

/**
 * Create a ha_kernel instance.
 *
 * @param count			total number of segments to use
 */
ha_kernel_t *ha_kernel_create(u_int count);

#endif /** HA_KERNEL_ @}*/
