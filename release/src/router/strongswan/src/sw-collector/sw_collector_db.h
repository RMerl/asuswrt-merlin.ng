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
 * @defgroup sw_collector_db_t sw_collector_db
 * @{ @ingroup sw_collector
 */

#ifndef SW_COLLECTOR_DB_H_
#define SW_COLLECTOR_DB_H_

#include <library.h>

typedef struct sw_collector_db_t sw_collector_db_t;
typedef enum sw_collector_db_query_t sw_collector_db_query_t;

/**
 * Type of software identifier queries
 */
enum sw_collector_db_query_t {
	SW_QUERY_ALL,
	SW_QUERY_INSTALLED,
	SW_QUERY_REMOVED
};

/**
 * Software collector database object
 */
struct sw_collector_db_t {

	/**
	 * bAdd event to database
	 *
	 * @param timestamp		Timestamp in 20 octet RFC 3339 format
	 * @return				Primary key pointing to event ID or 0 if failed
	 */
	uint32_t (*add_event)(sw_collector_db_t *this, char *timestamp);

	/**
	 * Get last event, zero EID if none exists
	 *
	 * @param eid			Primary key pointing to last event
	 * @param epoch			Epoch
	 * @param last_time		Timestamp in 20 octet RFC 3339 format of last event
	 * @return
	 */
	bool (*get_last_event)(sw_collector_db_t *this, uint32_t *eid,
							   uint32_t *epoch, char **last_time);

	/**
	 * Add software identifier event to database
	 *
	 * @param eid			Foreign key pointing to an event ID
	 * @param sw_id			Foreign key pointing to a software identifier
	 * @param action		1 for CREATION, 2 for deletion
	 * @return				TRUE if successful
	 */
	bool (*add_sw_event)(sw_collector_db_t *this, uint32_t eid, uint32_t sw_id,
						 uint8_t action);

	/**
	 * Set software_identifier, checking if the identifier already exists
	 *
	 * @param name			Software identifier
	 * @param package		Software package
	 * @param version		Version of software package
	 * @param source		Source ID of the software collector
	 * @param installed		Installation status to be set, TRUE if installed
	 * @return				Primary key pointing to SW ID or 0 if failed
	 */
	uint32_t (*set_sw_id)(sw_collector_db_t *this, char *name, char *package,
						  char *version, uint8_t source, bool installed);

	/**
	 * Get software_identifier record
	 *
	 * @param name			Software identifier
	 * @param package		Software package
	 * @param version		Version of software package
	 * @param source		Source ID of the software collector
	 * @param installed		Installation status
	 * @return				Primary key pointing to SW ID or 0 if failed
	 */
	uint32_t (*get_sw_id)(sw_collector_db_t *this, char *name, char **package,
						  char **version, uint8_t *source, bool *installed);

	/**
	 * Get number of installed or removed software identifiers
	 *
	 * @param type			Query type (ALL, INSTALLED, REMOVED)
	 * @return				Count
	 */
	uint32_t (*get_sw_id_count)(sw_collector_db_t *this,
								sw_collector_db_query_t type);

	/**
	 * Update the software identifier version
	 *
	 * @param sw_id			Primary key of software identifier
	 * @param name			Software identifier
	 * @param version		Package version
	 * @param installed		Installation status
	 * @return				TRUE if update successful
	 */
	bool (*update_sw_id)(sw_collector_db_t *this, uint32_t sw_id, char *name,
						 char *version, bool installed);

	/**
	 * Update the package name
	 *
	 * @param package_filter	Package name[s] to be changed
	 * @param package			New package name
	 * @return					TRUE if update successful
	 */
	int (*update_package)(sw_collector_db_t *this, char *package_filter,
												   char *package);

	/**
	 * Enumerate over all collected [installed] software identities
	 *
	 * @param type			Query type (ALL, INSTALLED, REMOVED)
	 * @param package		If not NULL enumerate over all package versions
	 * @return				Enumerator
	 */
	enumerator_t* (*create_sw_enumerator)(sw_collector_db_t *this,
										  sw_collector_db_query_t type,
										  char *package);

	/**
	 * Destroy sw_collector_db_t object
	 */
	void (*destroy)(sw_collector_db_t *this);

};

/**
 * Create an sw_collector_db_t instance
 *
 * @param uri				database URI
 */
sw_collector_db_t* sw_collector_db_create(char *uri);

#endif /** SW_COLLECTOR_DB_H_ @}*/
