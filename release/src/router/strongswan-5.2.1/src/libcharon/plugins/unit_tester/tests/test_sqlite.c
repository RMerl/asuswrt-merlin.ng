/*
 * Copyright (C) 2008 Martin Willi
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

#include <library.h>
#include <daemon.h>
#include <collections/enumerator.h>

#include <unistd.h>


#define DBFILE "/tmp/strongswan-test.db"

/*******************************************************************************
 * sqlite simple test
 ******************************************************************************/
bool test_sqlite()
{
	database_t *db;
	char *txt = "I'm a superduper test";
	chunk_t data = chunk_from_chars(0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08);
	int row;
	chunk_t qdata;
	char *qtxt;
	bool good = FALSE;
	enumerator_t *enumerator;

	db = lib->db->create(lib->db, "sqlite://" DBFILE);
	if (!db)
	{
		return FALSE;
	}
	if (db->execute(db, NULL, "CREATE TABLE test (txt TEXT, data BLOB)") < 0)
	{
		return FALSE;
	}
	if (db->execute(db, &row, "INSERT INTO test (txt, data) VALUES (?,?)",
					DB_TEXT, txt, DB_BLOB, data) < 0)
	{
		return FALSE;
	}
	if (row != 1)
	{
		return FALSE;
	}
	enumerator = db->query(db, "SELECT txt, data FROM test WHERE oid = ?",
						   DB_INT, row,
						   DB_TEXT, DB_BLOB);
	if (!enumerator)
	{
		return FALSE;
	}
	while (enumerator->enumerate(enumerator, &qtxt, &qdata))
	{
		if (good)
		{	/* only one row */
			good = FALSE;
			break;
		}
		if (streq(qtxt, txt) && chunk_equals(data, qdata))
		{
			good = TRUE;
		}
	}
	enumerator->destroy(enumerator);
	if (!good)
	{
		return FALSE;
	}
	if (db->execute(db, NULL, "DELETE FROM test WHERE oid = ?", DB_INT, row) != 1)
	{
		return FALSE;
	}
	if (db->execute(db, NULL, "DROP TABLE test") < 0)
	{
		return FALSE;
	}
	db->destroy(db);
	unlink(DBFILE);
	return TRUE;
}

