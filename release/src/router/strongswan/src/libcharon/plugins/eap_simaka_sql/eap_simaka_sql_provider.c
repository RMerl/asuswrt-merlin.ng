/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "eap_simaka_sql_provider.h"

#include <time.h>

#include <daemon.h>

typedef struct private_eap_simaka_sql_provider_t private_eap_simaka_sql_provider_t;

/**
 * Private data of an eap_simaka_sql_provider_t object.
 */
struct private_eap_simaka_sql_provider_t {

	/**
	 * Public eap_simaka_sql_provider_t interface.
	 */
	eap_simaka_sql_provider_t public;

	/**
	 * Triplet/quintuplet database
	 */
	database_t *db;

	/**
	 * Remove used triplets/quintuplets from database
	 */
	bool remove_used;
};

METHOD(simaka_provider_t, get_triplet, bool,
	private_eap_simaka_sql_provider_t *this, identification_t *id,
	char rand[SIM_RAND_LEN], char sres[SIM_SRES_LEN], char kc[SIM_KC_LEN])
{
	chunk_t rand_chunk, sres_chunk, kc_chunk;
	enumerator_t *query;
	bool found = FALSE;
	char buf[128];

	snprintf(buf, sizeof(buf), "%Y", id);
	query = this->db->query(this->db,
				"select rand, sres, kc from triplets where id = ? order by used",
				DB_TEXT, buf, DB_BLOB, DB_BLOB, DB_BLOB);
	if (query)
	{
		if (query->enumerate(query, &rand_chunk, &sres_chunk, &kc_chunk))
		{
			if (rand_chunk.len == SIM_RAND_LEN &&
				sres_chunk.len == SIM_SRES_LEN &&
				kc_chunk.len == SIM_KC_LEN)
			{
				memcpy(rand, rand_chunk.ptr, SIM_RAND_LEN);
				memcpy(sres, sres_chunk.ptr, SIM_SRES_LEN);
				memcpy(kc, kc_chunk.ptr, SIM_KC_LEN);
				found = TRUE;
			}
		}
		query->destroy(query);
	}
	if (found)
	{
		if (this->remove_used)
		{
			this->db->execute(this->db, NULL,
					"delete from triplets where id = ? and rand = ?",
					DB_TEXT, buf, DB_BLOB, chunk_create(rand, SIM_RAND_LEN));
		}
		else
		{
			this->db->execute(this->db, NULL,
					"update triplets set used = ? where id = ? and rand = ?",
					DB_UINT, time(NULL), DB_TEXT, buf,
					DB_BLOB, chunk_create(rand, SIM_RAND_LEN));
		}
	}
	return found;
}

METHOD(simaka_provider_t, get_quintuplet, bool,
	private_eap_simaka_sql_provider_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char xres[AKA_RES_MAX], int *xres_len,
	char ck[AKA_CK_LEN], char ik[AKA_IK_LEN], char autn[AKA_AUTN_LEN])
{
	chunk_t rand_chunk, xres_chunk, ck_chunk, ik_chunk, autn_chunk;
	enumerator_t *query;
	bool found = FALSE;
	char buf[128];

	snprintf(buf, sizeof(buf), "%Y", id);
	query = this->db->query(this->db, "select rand, res, ck, ik, autn "
				"from quintuplets where id = ? order by used", DB_TEXT, buf,
				DB_BLOB, DB_BLOB, DB_BLOB, DB_BLOB, DB_BLOB);
	if (query)
	{
		if (query->enumerate(query, &rand_chunk, &xres_chunk,
							 &ck_chunk, &ik_chunk, &autn_chunk))
		{
			if (rand_chunk.len == AKA_RAND_LEN &&
				xres_chunk.len <= AKA_RES_MAX &&
				ck_chunk.len == AKA_CK_LEN &&
				ik_chunk.len == AKA_IK_LEN &&
				autn_chunk.len == AKA_AUTN_LEN)
			{
				memcpy(rand, rand_chunk.ptr, AKA_RAND_LEN);
				memcpy(xres, xres_chunk.ptr, xres_chunk.len);
				*xres_len = xres_chunk.len;
				memcpy(ck, ck_chunk.ptr, AKA_CK_LEN);
				memcpy(ik, ik_chunk.ptr, AKA_IK_LEN);
				memcpy(autn, autn_chunk.ptr, AKA_AUTN_LEN);
				found = TRUE;
			}
		}
		query->destroy(query);
	}
	if (found)
	{
		if (this->remove_used)
		{
			this->db->execute(this->db, NULL,
					"delete from quintuplets where id = ? and rand = ?",
					DB_TEXT, buf, DB_BLOB, chunk_create(rand, SIM_RAND_LEN));
		}
		else
		{
			this->db->execute(this->db, NULL,
					"update quintuplets set used = ? where id = ? and rand = ?",
					DB_UINT, time(NULL), DB_TEXT, buf,
					DB_BLOB, chunk_create(rand, AKA_RAND_LEN));
		}
	}
	return found;
}

METHOD(eap_simaka_sql_provider_t, destroy, void,
	private_eap_simaka_sql_provider_t *this)
{
	free(this);
}

/**
 * See header
 */
eap_simaka_sql_provider_t *eap_simaka_sql_provider_create(database_t *db,
														  bool remove_used)
{
	private_eap_simaka_sql_provider_t *this;

	INIT(this,
		.public = {
			.provider = {
				.get_triplet = _get_triplet,
				.get_quintuplet = _get_quintuplet,
				.resync = (void*)return_false,
				.is_pseudonym = (void*)return_null,
				.gen_pseudonym = (void*)return_null,
				.is_reauth = (void*)return_null,
				.gen_reauth = (void*)return_null,
			},
			.destroy = _destroy,
		},
		.db = db,
		.remove_used = remove_used,
	);

	return &this->public;
}
