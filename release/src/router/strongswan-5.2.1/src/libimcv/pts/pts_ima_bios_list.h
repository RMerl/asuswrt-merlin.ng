/*
 * Copyright (C) 2014 Andreas Steffen
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
 * @defgroup pts_ima_bios_list pts_ima_bios_list
 * @{ @ingroup pts
 */

#ifndef PTS_IMA_BIOS_LIST_H_
#define PTS_IMA_BIOS_LIST_H_

#include <time.h>

#include <library.h>

typedef struct pts_ima_bios_list_t pts_ima_bios_list_t;

/**
 * Class retrieving Linux IMA BIOS measurements
 *
 */
struct pts_ima_bios_list_t {

	/**
	 * Get the time the BIOS measurements were taken
	 *
	 * @return				Measurement time
	 */
	time_t (*get_time)(pts_ima_bios_list_t *this);

	/**
	 * Get the number of non-processed BIOS measurements
	 *
	 * @return				Number of measurements left
	 */
	int (*get_count)(pts_ima_bios_list_t *this);

	/**
	 * Get the next BIOS measurement and remove it from the list
	 *
	 * @param pcr			PCR where the measurement was extended into
	 * @param measurement	Measurement hash
	 * @return				Return code
	 */
	status_t (*get_next)(pts_ima_bios_list_t *this, uint32_t *pcr,
													chunk_t *measurement);

	/**
	 * Destroys a pts_ima_bios_list_t object.
	 */
	void (*destroy)(pts_ima_bios_list_t *this);

};

/**
 * Create a PTS IMA BIOS measurement object
 *
 * @param file				Pathname pointing to the BIOS measurements
 */
pts_ima_bios_list_t* pts_ima_bios_list_create(char *file);

#endif /** PTS_IMA_BIOS_LIST_H_ @}*/
