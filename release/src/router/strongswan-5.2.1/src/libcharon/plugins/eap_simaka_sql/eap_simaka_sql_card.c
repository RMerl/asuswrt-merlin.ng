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

#include "eap_simaka_sql_card.h"

#include <time.h>

#include <daemon.h>

typedef struct private_eap_simaka_sql_card_t private_eap_simaka_sql_card_t;

/**
 * Private data of an eap_simaka_sql_card_t object.
 */
struct private_eap_simaka_sql_card_t {

	/**
	 * Public eap_simaka_sql_card_t interface.
	 */
	eap_simaka_sql_card_t public;

	/**
	 * Triplet/quintuplet database
	 */
	database_t *db;

	/**
	 * Remove used triplets/quintuplets from database
	 */
	bool remove_used;
};

METHOD(simaka_card_t, get_triplet, bool,
	private_eap_simaka_sql_card_t *this, identification_t *id,
	char rand[SIM_RAND_LEN], char sres[SIM_SRES_LEN], char kc[SIM_KC_LEN])
{
	chunk_t sres_chunk, kc_chunk;
	enumerator_t *query;
	bool found = FALSE;
	char buf[128];

	snprintf(buf, sizeof(buf), "%Y", id);
	query = this->db->query(this->db,
				"select sres, kc from triplets where rand = ? and id = ? "
				"order by use limit 1",
				DB_BLOB, chunk_create(rand, SIM_RAND_LEN), DB_TEXT, buf,
				DB_BLOB, DB_BLOB);
	if (query)
	{
		if (query->enumerate(query, &sres_chunk, &kc_chunk))
		{
			if (sres_chunk.len == SIM_SRES_LEN &&
				kc_chunk.len == SIM_KC_LEN)
			{
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
					"update triplets set use = ? where id = ? and rand = ?",
					DB_UINT, time(NULL), DB_TEXT, buf,
					DB_BLOB, chunk_create(rand, SIM_RAND_LEN));
		}
	}
	return found;
}

METHOD(simaka_card_t, get_quintuplet, status_t,
	private_eap_simaka_sql_card_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char autn[AKA_AUTN_LEN], char ck[AKA_CK_LEN],
	char ik[AKA_IK_LEN], char res[AKA_RES_MAX], int *res_len)
{
	chunk_t ck_chunk, ik_chunk, res_chunk;
	enumerator_t *query;
	status_t found = FAILED;
	char buf[128];

	snprintf(buf, sizeof(buf), "%Y", id);
	query = this->db->query(this->db, "select ck, ik, res from quintuplets "
				"where rand = ? and autn = ? and id = ? order by use limit 1",
				DB_BLOB, chunk_create(rand, AKA_RAND_LEN),
				DB_BLOB, chunk_create(autn, AKA_AUTN_LEN), DB_TEXT, buf,
				DB_BLOB, DB_BLOB, DB_BLOB);
	if (query)
	{
		if (query->enumerate(query, &ck_chunk, &ik_chunk, &res_chunk))
		{
			if (ck_chunk.len == AKA_CK_LEN &&
				ik_chunk.len == AKA_IK_LEN &&
				res_chunk.len <= AKA_RES_MAX)
			{
				memcpy(ck, ck_chunk.ptr, AKA_CK_LEN);
				memcpy(ik, ik_chunk.ptr, AKA_IK_LEN);
				memcpy(res, res_chunk.ptr, res_chunk.len);
				*res_len = res_chunk.len;
				found = SUCCESS;
			}
		}
		query->destroy(query);
	}
	if (found == SUCCESS)
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
					"update quintuplets set use = ? where id = ? and rand = ?",
					DB_UINT, time(NULL), DB_TEXT, buf,
					DB_BLOB, chunk_create(rand, AKA_RAND_LEN));
		}
	}
	return found;
}

METHOD(eap_simaka_sql_card_t, destroy, void,
	private_eap_simaka_sql_card_t *this)
{
	free(this);
}

/**
 * See header
 */
eap_simaka_sql_card_t *eap_simaka_sql_card_create(database_t *db,
												  bool remove_used)
{
	private_eap_simaka_sql_card_t *this;

	INIT(this,
		.public = {
			.card = {
				.get_triplet = _get_triplet,
				.get_quintuplet = _get_quintuplet,
				.resync = (void*)return_false,
				.get_pseudonym = (void*)return_null,
				.set_pseudonym = (void*)nop,
				.get_reauth = (void*)return_null,
				.set_reauth = (void*)nop,
			},
			.destroy = _destroy,
		},
		.db = db,
		.remove_used = remove_used,
	);

	return &this->public;
}
