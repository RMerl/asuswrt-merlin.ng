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
	 * @param algo			Algorithm used to hash files
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
 * @param file				Pathname pointing to the IMA runtme measurements
 */
pts_ima_event_list_t* pts_ima_event_list_create(char *file);

#endif /** PTS_IMA_EVENT_LIST_H_ @}*/
