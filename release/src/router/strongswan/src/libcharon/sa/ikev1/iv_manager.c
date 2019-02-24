/*
 * Copyright (C) 2011-2016 Tobias Brunner
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

#include "iv_manager.h"

#include <library.h>
#include <collections/linked_list.h>

/**
 * Max. number of IVs/QMs to track.
 */
#define MAX_EXCHANGES_DEFAULT 3

typedef struct private_iv_manager_t private_iv_manager_t;
typedef struct iv_data_t iv_data_t;
typedef struct qm_data_t qm_data_t;

/**
 * Data stored for IVs.
 */
struct iv_data_t {
	/**
	 * message ID
	 */
	uint32_t mid;

	/**
	 * current IV
	 */
	chunk_t iv;

	/**
	 * last block of encrypted message
	 */
	chunk_t last_block;
};

/**
 * Private data of a iv_manager_t object.
 */
struct private_iv_manager_t {
	/**
	 * Implement public interface.
	 */
	iv_manager_t public;

	/**
	 * Phase 1 IV.
	 */
	iv_data_t phase1_iv;

	/**
	 * Keep track of IVs for exchanges after phase 1. We store only a limited
	 * number of IVs in an MRU sort of way. Stores iv_data_t objects.
	 */
	linked_list_t *ivs;

	/**
	 * Keep track of Nonces during Quick Mode exchanges. Only a limited number
	 * of QMs are tracked at the same time. Stores qm_data_t objects.
	 */
	linked_list_t *qms;

	/**
	 * Max. number of IVs/Quick Modes to track.
	 */
	int max_exchanges;

	/**
	 * Hasher used for IV generation.
	 */
	hasher_t *hasher;

	/*
	 * Encryption algorithm the block size.
	 */
	size_t block_size;
};

/**
 * Data stored for Quick Mode exchanges.
 */
struct qm_data_t {
	/**
	 * Message ID.
	 */
	uint32_t mid;

	/**
	 * Ni_b (Nonce from first message).
	 */
	chunk_t n_i;

	/**
	 * Nr_b (Nonce from second message).
	 */
	chunk_t n_r;
};

/**
 * Destroy an iv_data_t object.
 */
static void iv_data_destroy(iv_data_t *this)
{
	chunk_free(&this->last_block);
	chunk_free(&this->iv);
	free(this);
}

/**
 * Destroy a qm_data_t object.
 */
static void qm_data_destroy(qm_data_t *this)
{
	chunk_free(&this->n_i);
	chunk_free(&this->n_r);
	free(this);
}

/**
 * Generate an IV.
 */
static bool generate_iv(private_iv_manager_t *this, iv_data_t *iv)
{
	if (iv->mid == 0 || iv->iv.ptr)
	{	/* use last block of previous encrypted message */
		chunk_free(&iv->iv);
		iv->iv = iv->last_block;
		iv->last_block = chunk_empty;
	}
	else
	{
		/* initial phase 2 IV = hash(last_phase1_block | mid) */
		uint32_t net;;
		chunk_t data;

		net = htonl(iv->mid);
		data = chunk_cata("cc", this->phase1_iv.iv, chunk_from_thing(net));
		if (!this->hasher->allocate_hash(this->hasher, data, &iv->iv))
		{
			return FALSE;
		}
		if (iv->iv.len > this->block_size)
		{
			iv->iv.len = this->block_size;
		}
	}
	DBG4(DBG_IKE, "next IV for MID %u %B", iv->mid, &iv->iv);
	return TRUE;
}

/**
 * Try to find an IV for the given message ID, if not found, generate it.
 */
static iv_data_t *lookup_iv(private_iv_manager_t *this, uint32_t mid)
{
	enumerator_t *enumerator;
	iv_data_t *iv, *found = NULL;

	if (mid == 0)
	{
		return &this->phase1_iv;
	}

	enumerator = this->ivs->create_enumerator(this->ivs);
	while (enumerator->enumerate(enumerator, &iv))
	{
		if (iv->mid == mid)
		{	/* IV gets moved to the front of the list */
			this->ivs->remove_at(this->ivs, enumerator);
			found = iv;
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!found)
	{
		INIT(found,
			.mid = mid,
		);
		if (!generate_iv(this, found))
		{
			iv_data_destroy(found);
			return NULL;
		}
	}
	this->ivs->insert_first(this->ivs, found);
	/* remove least recently used IV if maximum reached */
	if (this->ivs->get_count(this->ivs) > this->max_exchanges &&
		this->ivs->remove_last(this->ivs, (void**)&iv) == SUCCESS)
	{
		iv_data_destroy(iv);
	}
	return found;
}

METHOD(iv_manager_t, init_iv_chain, bool,
	private_iv_manager_t *this, chunk_t data, hasher_t *hasher,
	size_t block_size)
{
	this->hasher = hasher;
	this->block_size = block_size;

	if (!this->hasher->allocate_hash(this->hasher, data, &this->phase1_iv.iv))
	{
		return FALSE;
	}
	if (this->phase1_iv.iv.len > this->block_size)
	{
		this->phase1_iv.iv.len = this->block_size;
	}
	DBG4(DBG_IKE, "initial IV %B", &this->phase1_iv.iv);
	return TRUE;
}

METHOD(iv_manager_t, get_iv, bool,
	private_iv_manager_t *this, uint32_t mid, chunk_t *out)
{
	iv_data_t *iv;

	iv = lookup_iv(this, mid);
	if (iv)
	{
		*out = iv->iv;
		return TRUE;
	}
	return FALSE;
}

METHOD(iv_manager_t, update_iv, bool,
	private_iv_manager_t *this, uint32_t mid, chunk_t last_block)
{
	iv_data_t *iv = lookup_iv(this, mid);
	if (iv)
	{	/* update last block */
		chunk_free(&iv->last_block);
		iv->last_block = chunk_clone(last_block);
		return TRUE;
	}
	return FALSE;
}

METHOD(iv_manager_t, confirm_iv, bool,
	private_iv_manager_t *this, uint32_t mid)
{
	iv_data_t *iv = lookup_iv(this, mid);
	if (iv)
	{
		return generate_iv(this, iv);
	}
	return FALSE;
}

METHOD(iv_manager_t, lookup_quick_mode, void,
	private_iv_manager_t *this, uint32_t mid, chunk_t **n_i, chunk_t **n_r)
{
	enumerator_t *enumerator;
	qm_data_t *qm, *found = NULL;

	enumerator = this->qms->create_enumerator(this->qms);
	while (enumerator->enumerate(enumerator, &qm))
	{
		if (qm->mid == mid)
		{	/* state gets moved to the front of the list */
			this->qms->remove_at(this->qms, enumerator);
			found = qm;
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!found)
	{
		INIT(found,
			.mid = mid,
		);
	}

	*n_i = &found->n_i;
	*n_r = &found->n_r;

	this->qms->insert_first(this->qms, found);
	/* remove least recently used state if maximum reached */
	if (this->qms->get_count(this->qms) > this->max_exchanges &&
		this->qms->remove_last(this->qms, (void**)&qm) == SUCCESS)
	{
		qm_data_destroy(qm);
	}
}

METHOD(iv_manager_t, remove_quick_mode, void,
	private_iv_manager_t *this, uint32_t mid)
{
	enumerator_t *enumerator;
	qm_data_t *qm;

	enumerator = this->qms->create_enumerator(this->qms);
	while (enumerator->enumerate(enumerator, &qm))
	{
		if (qm->mid == mid)
		{
			this->qms->remove_at(this->qms, enumerator);
			qm_data_destroy(qm);
			break;
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(iv_manager_t, destroy, void,
	private_iv_manager_t *this)
{
	chunk_free(&this->phase1_iv.iv);
	chunk_free(&this->phase1_iv.last_block);
	this->ivs->destroy_function(this->ivs, (void*)iv_data_destroy);
	this->qms->destroy_function(this->qms, (void*)qm_data_destroy);
	free(this);
}

iv_manager_t *iv_manager_create(int max_exchanges)
{
	private_iv_manager_t *this;

	INIT(this,
		.public = {
			.init_iv_chain = _init_iv_chain,
			.get_iv = _get_iv,
			.update_iv = _update_iv,
			.confirm_iv = _confirm_iv,
			.lookup_quick_mode = _lookup_quick_mode,
			.remove_quick_mode = _remove_quick_mode,
			.destroy = _destroy,
		},
		.ivs = linked_list_create(),
		.qms = linked_list_create(),
		.max_exchanges = max_exchanges,
	);

	if (!this->max_exchanges)
	{
		this->max_exchanges = lib->settings->get_int(lib->settings,
					"%s.max_ikev1_exchanges", MAX_EXCHANGES_DEFAULT, lib->ns);
	}
	return &this->public;
}
