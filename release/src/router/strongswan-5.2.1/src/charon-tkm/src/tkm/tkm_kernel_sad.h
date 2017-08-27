/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup tkm-kernel-sad kernel sad
 * @{ @ingroup tkm
 */

#ifndef TKM_KERNEL_SAD_H_
#define TKM_KERNEL_SAD_H_

#include <networking/host.h>
#include <tkm/types.h>

typedef struct tkm_kernel_sad_t tkm_kernel_sad_t;

/**
 * The TKM kernel SAD (security association database) stores information about
 * CHILD SAs.
 */
struct tkm_kernel_sad_t {

	/**
	 * Insert new SAD entry with specified parameters.
	 *
	 * @param esa_id		ESP SA context identifier
	 * @param src			source address of CHILD SA
	 * @param dst			destination address of CHILD SA
	 * @param spi			SPI of CHILD SA
	 * @param proto			protocol of CHILD SA (ESP/AH)
	 * @return				TRUE if entry was inserted, FALSE otherwise
	 */
	bool (*insert)(tkm_kernel_sad_t * const this, const esa_id_type esa_id,
				   const host_t * const src, const host_t * const dst,
				   const u_int32_t spi, const u_int8_t proto);

	/**
	 * Get ESA id for entry with given parameters.
	 *
	 * @param src			source address of CHILD SA
	 * @param dst			destination address of CHILD SA
	 * @param spi			SPI of CHILD SA
	 * @param proto			protocol of CHILD SA (ESP/AH)
	 * @return				ESA id of entry if found, 0 otherwise
	 */
	esa_id_type (*get_esa_id)(tkm_kernel_sad_t * const this,
				 const host_t * const src, const host_t * const dst,
				 const u_int32_t spi, const u_int8_t proto);

	/**
	 * Remove entry with given ESA id from SAD.
	 *
	 * @param esa_id		ESA identifier of entry to remove
	 * @return				TRUE if entry was removed, FALSE otherwise
	 */
	bool (*remove)(tkm_kernel_sad_t * const this, const esa_id_type esa_id);

	/**
	 * Destroy a tkm_kernel_sad instance.
	 */
	void (*destroy)(tkm_kernel_sad_t *this);

};

/**
 * Create a TKM kernel SAD instance.
 */
tkm_kernel_sad_t *tkm_kernel_sad_create();

#endif /** TKM_KERNEL_SAD_H_ @}*/
