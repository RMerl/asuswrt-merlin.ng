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

#include <string.h>

#include "sql_logger.h"

#include <daemon.h>
#include <threading/thread_value.h>

typedef struct private_sql_logger_t private_sql_logger_t;

/**
 * Private data of an sql_logger_t object
 */
struct private_sql_logger_t {

	/**
	 * Public part
	 */
	sql_logger_t public;

	/**
	 * database connection
	 */
	database_t *db;

	/**
	 * logging level
	 */
	int level;

	/**
	 * avoid recursive calls by the same thread
	 */
	thread_value_t *recursive;
};

METHOD(logger_t, log_, void,
	private_sql_logger_t *this, debug_t group, level_t level, int thread,
	ike_sa_t* ike_sa, const char *message)
{
	if (this->recursive->get(this->recursive))
	{
		return;
	}
	this->recursive->set(this->recursive, this->recursive);

	if (ike_sa)
	{
		chunk_t local_spi, remote_spi;
		host_t *local_host, *remote_host;
		identification_t *local_id, *remote_id;
		uint64_t ispi, rspi;
		ike_sa_id_t *id;

		id = ike_sa->get_id(ike_sa);
		ispi = id->get_initiator_spi(id);
		rspi = id->get_responder_spi(id);
		if (id->is_initiator(id))
		{
			local_spi.ptr = (char*)&ispi;
			remote_spi.ptr = (char*)&rspi;
		}
		else
		{
			local_spi.ptr = (char*)&rspi;
			remote_spi.ptr = (char*)&ispi;
		}
		local_spi.len = remote_spi.len = sizeof(ispi);
		local_id = ike_sa->get_my_id(ike_sa);
		remote_id = ike_sa->get_other_id(ike_sa);
		local_host = ike_sa->get_my_host(ike_sa);
		remote_host = ike_sa->get_other_host(ike_sa);

		this->db->execute(this->db, NULL, "REPLACE INTO ike_sas ("
						  "local_spi, remote_spi, id, initiator, "
						  "local_id_type, local_id_data, "
						  "remote_id_type, remote_id_data, "
						  "host_family, local_host_data, remote_host_data) "
						  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
						  DB_BLOB, local_spi, DB_BLOB, remote_spi,
						  DB_INT, ike_sa->get_unique_id(ike_sa),
						  DB_INT, id->is_initiator(id),
						  DB_INT, local_id->get_type(local_id),
						  DB_BLOB, local_id->get_encoding(local_id),
						  DB_INT, remote_id->get_type(remote_id),
						  DB_BLOB, remote_id->get_encoding(remote_id),
						  DB_INT, local_host->get_family(local_host),
						  DB_BLOB, local_host->get_address(local_host),
						  DB_BLOB, remote_host->get_address(remote_host));
		this->db->execute(this->db, NULL, "INSERT INTO logs ("
						  "local_spi, `signal`, level, msg) "
						  "VALUES (?, ?, ?, ?)",
						  DB_BLOB, local_spi, DB_INT, group, DB_INT, level,
						  DB_TEXT, message);
	}

	this->recursive->set(this->recursive, NULL);
}

METHOD(logger_t, get_level, level_t,
	private_sql_logger_t *this, debug_t group)
{
	return this->level;
}

METHOD(sql_logger_t, destroy, void,
	private_sql_logger_t *this)
{
	this->recursive->destroy(this->recursive);
	free(this);
}

/**
 * Described in header.
 */
sql_logger_t *sql_logger_create(database_t *db)
{
	private_sql_logger_t *this;

	INIT(this,
		.public = {
			.logger = {
				.log = _log_,
				.get_level = _get_level,
			},
			.destroy = _destroy,
		},
		.db = db,
		.recursive = thread_value_create(NULL),
		.level = lib->settings->get_int(lib->settings,
										"%s.plugins.sql.loglevel", -1, lib->ns),
	);

	return &this->public;
}

