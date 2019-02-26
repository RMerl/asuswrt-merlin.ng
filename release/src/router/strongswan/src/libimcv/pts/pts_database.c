/*
 * Copyright (C) 2011-2012 Sansar Choinyambuu
 * Copyright (C) 2012-2017 Andreas Steffen
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

#define _GNU_SOURCE
#include <stdio.h>
#include <libgen.h>

#include "pts_database.h"

#include <utils/debug.h>
#include <crypto/hashers/hasher.h>


typedef struct private_pts_database_t private_pts_database_t;

/**
 * Private data of a pts_database_t object.
 *
 */
struct private_pts_database_t {

	/**
	 * Public pts_database_t interface.
	 */
	pts_database_t public;

	/**
	 * database instance
	 */
	database_t *db;

};

METHOD(pts_database_t, get_pathname, char*,
	private_pts_database_t *this, bool is_dir, int id)
{
	enumerator_t *e;
	char *path, *name, *sep, *pathname = NULL;

	if (is_dir)
	{
		e = this->db->query(this->db,
				"SELECT path FROM directories WHERE id = ?",
				 DB_INT, id, DB_TEXT);
		if (!e || !e->enumerate(e, &path))
		{
			pathname = NULL;
		}
		else
		{
			pathname = strdup(path);
		}
	}
	else
	{
		e = this->db->query(this->db,
				"SELECT d.path, f.name FROM files AS f "
				"JOIN directories AS d ON d.id = f.dir WHERE f.id = ?",
				 DB_INT, id, DB_TEXT, DB_TEXT);
		if (e && e->enumerate(e, &path, &name))
		{
			if (path[0] == '/')
			{	/* Unix style absolute path */
				sep = "/";
			}
			else
			{	/* Windows absolute path */
				sep = "\\";
			}
			if (asprintf(&pathname, "%s%s%s",
						 path, streq(path, "/") ? "" : sep, name) == -1)
			{
				pathname = NULL;
			}
		}
	}
	DESTROY_IF(e);

	return pathname;
}

METHOD(pts_database_t, create_file_hash_enumerator, enumerator_t*,
	private_pts_database_t *this, int pid, pts_meas_algorithms_t algo,
	bool is_dir, int id)
{
	enumerator_t *e;

	if (is_dir)
	{
		e = this->db->query(this->db,
				"SELECT f.id, f.name, fh.hash FROM file_hashes AS fh "
				"JOIN files AS f ON f.id = fh.file "
				"JOIN directories as d ON d.id = f.dir "
				"JOIN versions as v ON v.id = fh.version "
				"WHERE v.product = ? AND fh.algo = ? AND d.id = ? "
				"ORDER BY f.name",
				DB_INT, pid, DB_INT, algo, DB_INT, id, DB_INT, DB_TEXT, DB_TEXT);
	}
	else
	{
		e = this->db->query(this->db,
				"SELECT f.id, f.name, fh.hash FROM file_hashes AS fh "
				"JOIN files AS f ON f.id = fh.file "
				"JOIN versions AS v ON v.id = fh.version "
				"WHERE v.product = ? AND fh.algo = ? AND fh.file = ?",
				DB_INT, pid, DB_INT, algo, DB_INT, id, DB_INT, DB_TEXT, DB_TEXT);
	}
	return e;
}


METHOD(pts_database_t, get_product_version, bool,
	private_pts_database_t *this, int pid, int *vid)
{
	enumerator_t *e;
	int pkg_id;

	/* does empty package name already exist? */
	e = this->db->query(this->db,
			"SELECT id FROM packages WHERE name = ''", DB_INT);
	if (!e)
	{
		return FALSE;
	}
	if (!e->enumerate(e, &pkg_id))
	{
		/* create generic product version entry */
		if (this->db->execute(this->db, &pkg_id,
				"INSERT INTO packages (name) VALUES ('')") != 1)
		{
			DBG1(DBG_PTS, "could not insert package into database");
			e->destroy(e);
			return FALSE;
		}
	}
	e->destroy(e);

	/* does generic product version already exist? */
	e = this->db->query(this->db,
			"SELECT id FROM versions WHERE product = ? AND package = ?",
			 DB_INT, pid, DB_INT, pkg_id);
	if (!e)
	{
		return FALSE;
	}
	if (!e->enumerate(e, vid))
	{
		/* create generic product version entry */
		if (this->db->execute(this->db, vid,
				"INSERT INTO versions (product, package) VALUES (?, ?)",
				 DB_INT, pid, DB_INT, pkg_id) != 1)
		{
			DBG1(DBG_PTS, "could not insert version into database");
			e->destroy(e);
			return FALSE;
		}
	}
	e->destroy(e);

	return TRUE;
}

METHOD(pts_database_t, add_file_measurement, bool,
	private_pts_database_t *this, int vid, pts_meas_algorithms_t algo,
	chunk_t measurement, char *filename, bool is_dir, int id)
{
	enumerator_t *e;
	char *name;
	uint8_t hash_buf[HASH_SIZE_SHA512];
	uint8_t hex_meas_buf[2*HASH_SIZE_SHA512+1], *hex_hash_buf;
	chunk_t hash, hex_hash, hex_meas;
	int hash_id, fid;
	bool success = TRUE;

	if (is_dir)
	{
		/* does filename entry already exist? */
		e = this->db->query(this->db,
				"SELECT id FROM files WHERE name = ? AND dir = ?",
				 DB_TEXT, filename, DB_INT, id, DB_INT);
		if (!e)
		{
			return FALSE;
		}
		if (!e->enumerate(e, &fid))
		{
			/* create filename entry */
			if (this->db->execute(this->db, &fid,
					"INSERT INTO files (name, dir) VALUES (?, ?)",
					 DB_TEXT, filename, DB_INT, id) != 1)
			{
				DBG1(DBG_PTS, "could not insert filename into database");
				success = FALSE;
			}
		}
		e->destroy(e);
	}
	else
	{
		fid = id;

		/* verify filename */
		e = this->db->query(this->db,
				 "SELECT name FROM files WHERE id = ?", DB_INT, fid, DB_TEXT);
		if (!e)
		{
			return FALSE;
		}
		if (!e->enumerate(e, &name) || !streq(name, filename))
		{
			DBG1(DBG_PTS, "filename of reference measurement does not match");
			success = FALSE;
		}
		e->destroy(e);
	}

	if (!success)
	{
		return FALSE;
	}

	/* does hash measurement value already exist? */
	e = this->db->query(this->db,
			"SELECT id, hash FROM file_hashes "
			"WHERE algo = ? AND file = ? AND version = ?",
			 DB_INT, algo, DB_INT, fid, DB_INT, vid, DB_INT, DB_TEXT);
	if (!e)
	{
		return FALSE;
	}
	if (e->enumerate(e, &hash_id, &hex_hash_buf))
	{
		hex_hash = chunk_from_str(hex_hash_buf);
		hash = chunk_from_hex(hex_hash, hash_buf);

		if (!chunk_equals(measurement, hash))
		{
			/* update hash measurement value */
			if (this->db->execute(this->db, &hash_id,
					"UPDATE file_hashes SET hash = ? WHERE id = ?",
					 DB_BLOB, measurement, DB_INT, hash_id) != 1)
			{
				success = FALSE;
			}
		}
	}
	else
	{
		hex_meas = chunk_to_hex(measurement, hex_meas_buf, FALSE);
		hex_meas_buf[hex_meas.len] = '\0';

		/* insert hash measurement value */
		if (this->db->execute(this->db, &hash_id,
				"INSERT INTO file_hashes (file, version, algo, hash) "
				"VALUES (?, ?, ?, ?)", DB_INT, fid, DB_INT, vid,
				 DB_INT, algo, DB_TEXT, hex_meas_buf) != 1)
		{
			success = FALSE;
		}
	}
	e->destroy(e);

	return success;
}

METHOD(pts_database_t, create_file_meas_enumerator, enumerator_t*,
	private_pts_database_t *this, int pid, pts_meas_algorithms_t algo,
	char *filename)
{
	enumerator_t *e;
	char *dir, *file;

	if (strlen(filename) < 1)
	{
		return NULL;
	}

	/* separate filename into directory and basename components */
	dir = path_dirname(filename);
	file = path_basename(filename);

	if (*dir == '.')
	{	/* relative pathname */
		e = this->db->query(this->db,
				"SELECT fh.hash FROM file_hashes AS fh "
				"JOIN files AS f ON f.id = fh.file "
				"JOIN versions AS v ON v.id = fh.version "
				"WHERE v.product = ? AND f.name = ? AND fh.algo = ? "
				"ORDER BY v.time DESC",
				DB_INT, pid, DB_TEXT, file, DB_INT, algo, DB_TEXT);
	}
	else
	{	/* absolute pathname */
		int did;

		/* find directory entry first */
		e = this->db->query(this->db,
				"SELECT id FROM directories WHERE path = ?",
				DB_TEXT, dir, DB_INT);

		if (!e || !e->enumerate(e, &did))
		{
			goto err;
		}
		e->destroy(e);

		e = this->db->query(this->db,
				"SELECT fh.hash FROM file_hashes AS fh "
				"JOIN files AS f ON f.id = fh.file "
				"JOIN versions AS v ON v.id = fh.version "
				"WHERE v.product = ? AND f.dir = ? AND f.name = ? AND fh.algo = ? "
				"ORDER BY v.time DESC",
				DB_INT, pid, DB_INT, did, DB_TEXT, file, DB_INT, algo, DB_TEXT);
	}

err:
	free(file);
	free(dir);

	return e;
}

METHOD(pts_database_t, check_comp_measurement, status_t,
	private_pts_database_t *this, chunk_t measurement, int cid, int aik_id,
	int seq_no, int pcr, pts_meas_algorithms_t algo)
{
	enumerator_t *e;
	chunk_t hash;
	status_t status = NOT_FOUND;

	e = this->db->query(this->db,
					"SELECT hash FROM component_hashes "
					"WHERE component = ?  AND key = ? "
					"AND seq_no = ? AND pcr = ? AND algo = ? ",
					DB_INT, cid, DB_INT, aik_id, DB_INT, seq_no,
					DB_INT, pcr, DB_INT, algo, DB_BLOB);
	if (!e)
	{
		DBG1(DBG_PTS, "no database query enumerator returned");
		return FAILED;
	}

	while (e->enumerate(e, &hash))
	{
		if (chunk_equals(hash, measurement))
		{
			status = SUCCESS;
			break;
		}
		else
		{
			DBG1(DBG_PTS, "PCR %2d no matching component measurement #%d "
						  "found in database", pcr, seq_no);
			DBG1(DBG_PTS, "  expected: %#B", &hash);
			DBG1(DBG_PTS, "  received: %#B", &measurement);
			status = VERIFY_ERROR;
			break;
		}
	}
	e->destroy(e);

	if (status == NOT_FOUND)
	{
		DBG1(DBG_PTS, "PCR %2d no measurement #%d "
					  "found in database", pcr, seq_no);
	}

	return status;
}

METHOD(pts_database_t, insert_comp_measurement, status_t,
	private_pts_database_t *this, chunk_t measurement, int cid, int aik_id,
	int seq_no, int pcr, pts_meas_algorithms_t algo)
{
	int id;

	if (this->db->execute(this->db, &id,
					"INSERT INTO component_hashes "
					"(component, key, seq_no, pcr, algo, hash) "
					"VALUES (?, ?, ?, ?, ?, ?)",
					DB_INT, cid, DB_INT, aik_id, DB_INT, seq_no, DB_INT, pcr,
					DB_INT, algo, DB_BLOB, measurement) == 1)
	{
		return SUCCESS;
	}

	DBG1(DBG_PTS, "could not insert component measurement into database");
	return FAILED;
}

METHOD(pts_database_t, delete_comp_measurements, int,
	private_pts_database_t *this, int cid, int aik_id)
{
	return this->db->execute(this->db, NULL,
					"DELETE FROM component_hashes "
					"WHERE component = ? AND key = ?",
					DB_INT, cid, DB_INT, aik_id);
}

METHOD(pts_database_t, get_comp_measurement_count, status_t,
	private_pts_database_t *this, pts_comp_func_name_t *comp_name,
	int aik_id, pts_meas_algorithms_t algo, int *cid, int *count)
{
	enumerator_t *e;
	status_t status = SUCCESS;

	/* Initialize count */
	*count = 0;

	/* Get the primary key of the Component Functional Name */
	e = this->db->query(this->db,
				"SELECT id FROM components "
			"	WHERE vendor_id = ?  AND name = ? AND qualifier = ?",
				DB_INT, comp_name->get_vendor_id(comp_name),
				DB_INT, comp_name->get_name(comp_name),
				DB_INT, comp_name->get_qualifier(comp_name),
				DB_INT);
	if (!e)
	{
		DBG1(DBG_PTS, "no database query enumerator returned");
		return FAILED;
	}
	if (!e->enumerate(e, cid))
	{
		DBG1(DBG_PTS, "component functional name not found in database");
		e->destroy(e);
		return FAILED;
	}
	e->destroy(e);

	/* Get the number of stored measurements for a given AIK and component */
	e = this->db->query(this->db,
				"SELECT COUNT(*) FROM component_hashes AS ch "
				"WHERE component = ?  AND key = ? AND algo = ?",
				DB_INT, *cid, DB_INT, aik_id, DB_INT, algo, DB_INT);
	if (!e)
	{
		DBG1(DBG_PTS, "no database query enumerator returned");
		return FAILED;
	}
	if (!e->enumerate(e, count))
	{
		DBG1(DBG_PTS, "no component measurement count returned from database");
		status = FAILED;
	}
	e->destroy(e);

	return status;
}

METHOD(pts_database_t, destroy, void,
	private_pts_database_t *this)
{
	free(this);
}

/**
 * See header
 */
pts_database_t *pts_database_create(imv_database_t *imv_db)
{
	private_pts_database_t *this;

	if (!imv_db)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.get_pathname = _get_pathname,
			.create_file_hash_enumerator = _create_file_hash_enumerator,
			.get_product_version = _get_product_version,
			.add_file_measurement = _add_file_measurement,
			.create_file_meas_enumerator = _create_file_meas_enumerator,
			.check_comp_measurement = _check_comp_measurement,
			.insert_comp_measurement = _insert_comp_measurement,
			.delete_comp_measurements = _delete_comp_measurements,
			.get_comp_measurement_count = _get_comp_measurement_count,
			.destroy = _destroy,
		},
		.db = imv_db->get_database(imv_db),
	);

	return &this->public;
}
