/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup sql_logger_i sql_logger
 * @{ @ingroup sql
 */

#ifndef SQL_LOGGER_H_
#define SQL_LOGGER_H_

#include <bus/bus.h>
#include <database/database.h>

typedef struct sql_logger_t sql_logger_t;

/**
 * SQL database logger.
 */
struct sql_logger_t {

	/**
	 * Implements logger_t interface
	 */
	logger_t logger;

	/**
	 * Destry the backend.
	 */
	void (*destroy)(sql_logger_t *this);
};

/**
 * Create a sql_logger instance.
 *
 * @param db		underlying database
 * @return			logger instance
 */
sql_logger_t *sql_logger_create(database_t *db);

#endif /** SQL_LOGGER_H_ @}*/
