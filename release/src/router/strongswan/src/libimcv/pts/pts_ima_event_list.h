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
 * @defgroup pts_ima_event_list pts_ima_event_list
 * @{ @ingroup pts
 */

#ifndef PTS_IMA_EVENT_LIST_H_
#define PTS_IMA_EVENT_LIST_H_

#include "pts_meas_algo.h"

#include <time.h>

#include <library.h>

typedef struct pts_ima_event_list_t pts_ima_event_list_t;

#define IMA_PCR				10
#define IMA_ALGO_LEN_MIN	5
#define IMA_ALGO_LEN_MAX	8


/**
 * Class retrieving Linux IMA file measurements
 *
 */
struct pts_ima_event_list_t {

	/**
	 * Get the time the file measurements were taken
	 *
	 * @return				Measurement time
	 */
	time_t (*get_time)(pts_ima_event_list_t *this);

	/**
	 * Get the number of non-processed file measurements
	 *
	 * @return				Number of measurements left
	 */
	int (*get_count)(pts_ima_event_list_t *this);

	/**
	 * Get the next file measurement and remove it from the list
	 *
	 * @param measurement	Measurement hash
	 * @param algo			Algorithm used to compute file digests
	 " @param name			Event name (absolute filename or boot_aggregate)
	 * @return				Return code
	 */
	status_t (*get_next)(pts_ima_event_list_t *this, chunk_t *measurement,
						 char **algo, char **name);

	/**
	 * Destroys a pts_ima_event_list_t object.
	 */
	void (*destroy)(pts_ima_event_list_t *this);

};

/**
 * Create a PTS IMA runtime file measurement object
 *
 * @param file				Pathname pointing to the IMA runtime measurements
 * @param pcr_algo			PCR hash measurement algorithm to be used
 * @param pcr_padding		Apply PCR hash padding if hash algorithm is lacking
 */
pts_ima_event_list_t* pts_ima_event_list_create(char *file,
							pts_meas_algorithms_t pcr_algo, bool pcr_padding);

/**
 * Generate an IMA or IMA-NG hash from an event digest and event name
 *
 * @param digest		event digest
 * @param ima_algo		event digest algorithm string ("sha1:", "sha256:", etc.)
 * @param ima_name		event name
 * @param pcr_algo		hash algorithm used by TPM PCR extension
 * @param hash_buf		hash value to be compared with TPM measurement
 * @return				TRUE if computation successful
 */
bool pts_ima_event_hash(chunk_t digest, char *ima_algo, char *ima_name,
						pts_meas_algorithms_t pcr_algo, char *hash_buf);

#endif /** PTS_IMA_EVENT_LIST_H_ @}*/
