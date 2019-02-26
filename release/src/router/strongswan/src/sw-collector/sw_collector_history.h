/*
 * Copyright (C) 2017 Andreas Steffen
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
 * @defgroup sw_collector_history_t sw_collector_history
 * @{ @ingroup sw_collector
 */

#ifndef SW_COLLECTOR_HISTORY_H_
#define SW_COLLECTOR_HISTORY_H_

#include "sw_collector_db.h"

#include <library.h>
#include <utils/debug.h>
#include <utils/lexparser.h>

typedef struct sw_collector_history_t sw_collector_history_t;
typedef enum sw_collector_history_op_t sw_collector_history_op_t;

/**
 * Define major history event operations
 */
enum sw_collector_history_op_t {
	SW_OP_INSTALL,
	SW_OP_UPGRADE,
	SW_OP_REMOVE
};

/**
 * Software collector history object
 */
struct sw_collector_history_t {

	/**
	 * Extract timestamp from event in installation history
	 *
	 * @param args			Arguments to be processed
	 * @param buf			timestamp buffer for 21 byte RFC 3339 string
	 * @return				TRUE if extraction succeeded
	 */
	bool (*extract_timestamp)(sw_collector_history_t *this, chunk_t args,
							  char *buf);

	/**
	 * Extract packages from event in installation history 
	 *
	 * @param args			Arguments to be processed
	 * @param eid			Primary key pointing to current event
	 * @param op			Extraction operation 
	 * @return				TRUE if extraction succeeded
	 */
	bool (*extract_packages)(sw_collector_history_t *this, chunk_t args,
							 uint32_t eid, sw_collector_history_op_t op);

	/**
	 * Merge packages from initial installation
	 *
	 * @return				TRUE if merge succeeded
	 */
	bool (*merge_installed_packages)(sw_collector_history_t *this);

	/**
	 * Destroy sw_collector_history_t object
	 */
	void (*destroy)(sw_collector_history_t *this);

};

/**
 * Create an sw_collector_history_t instance
 *
 * @param db				Internal reference to collector database
 * @param source			Software event source number
 */
sw_collector_history_t* sw_collector_history_create(sw_collector_db_t *db,
													uint8_t source);

#endif /** SW_COLLECTOR_HISTORY_H_ @}*/
