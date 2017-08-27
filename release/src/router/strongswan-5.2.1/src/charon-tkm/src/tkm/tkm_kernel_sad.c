/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include <collections/linked_list.h>
#include <threading/mutex.h>
#include <utils/debug.h>

#include "tkm_kernel_sad.h"

typedef struct private_tkm_kernel_sad_t private_tkm_kernel_sad_t;

/**
 * Private data of tkm_kernel_sad.
 */
struct private_tkm_kernel_sad_t {

	/**
	 * Public functions.
	 */
	tkm_kernel_sad_t public;

	/**
	 * Linked list of SAD entries.
	 */
	linked_list_t *data;

	/**
	 * Lock used to protect SA data.
	 */
	mutex_t *mutex;

};

typedef struct sad_entry_t sad_entry_t;

/**
 * Data structure holding all information of an SAD entry.
 */
struct sad_entry_t {

	/**
	 * ESA identifier.
	 */
	esa_id_type esa_id;

	/**
	 * Source address of CHILD SA.
	 */
	host_t *src;

	/**
	 * Destination address of CHILD SA.
	 */
	host_t *dst;

	/**
	 * SPI of CHILD SA.
	 */
	u_int32_t spi;

	/**
	 * Protocol of CHILD SA (ESP/AH).
	 */
	u_int8_t proto;

};

/**
 * Destroy an sad_entry_t object.
 */
static void sad_entry_destroy(sad_entry_t *entry)
{
	if (entry)
	{
		DESTROY_IF(entry->src);
		DESTROY_IF(entry->dst);
		free(entry);
	}
}

/**
 * Find a list entry with given src, dst, spi and proto values.
 */
static bool sad_entry_match(sad_entry_t * const entry, const host_t * const src,
							const host_t * const dst, const u_int32_t * const spi,
							const u_int8_t * const proto)
{
	if (entry->src == NULL || entry->dst == NULL)
	{
		return FALSE;
	}

	return src->ip_equals(entry->src, (host_t *)src) &&
		   dst->ip_equals(entry->dst, (host_t *)dst) &&
		   entry->spi == *spi && entry->proto == *proto;
}

/**
 * Compare two SAD entries for equality.
 */
static bool sad_entry_equal(sad_entry_t * const left, sad_entry_t * const right)
{
	if (left->src == NULL || left->dst == NULL || right->src == NULL ||
		right->dst == NULL)
	{
		return FALSE;
	}
	return left->esa_id == right->esa_id &&
		   left->src->ip_equals(left->src, right->src) &&
		   left->dst->ip_equals(left->dst, right->dst) &&
		   left->spi == right->spi && left->proto == right->proto;
}

METHOD(tkm_kernel_sad_t, insert, bool,
	private_tkm_kernel_sad_t * const this, const esa_id_type esa_id,
	const host_t * const src, const host_t * const dst, const u_int32_t spi,
	const u_int8_t proto)
{
	status_t result;
	sad_entry_t *new_entry;

	INIT(new_entry,
		 .esa_id = esa_id,
		 .src = (host_t *)src,
		 .dst = (host_t *)dst,
		 .spi = spi,
		 .proto = proto,
	);

	this->mutex->lock(this->mutex);
	result = this->data->find_first(this->data,
									(linked_list_match_t)sad_entry_equal, NULL,
									new_entry);
	if (result == NOT_FOUND)
	{
		DBG3(DBG_KNL, "inserting SAD entry (esa: %llu, src: %H, dst: %H, "
			 "spi: %x, proto: %u)", esa_id, src, dst, ntohl(spi), proto);
		new_entry->src = src->clone((host_t *)src);
		new_entry->dst = dst->clone((host_t *)dst);
		this->data->insert_last(this->data, new_entry);
	}
	else
	{
		DBG1(DBG_KNL, "SAD entry with esa id %llu already exists!", esa_id);
		free(new_entry);
	}
	this->mutex->unlock(this->mutex);
	return result == NOT_FOUND;
}

METHOD(tkm_kernel_sad_t, get_esa_id, esa_id_type,
	private_tkm_kernel_sad_t * const this, const host_t * const src,
	const host_t * const dst, const u_int32_t spi, const u_int8_t proto)
{
	esa_id_type id = 0;
	sad_entry_t *entry = NULL;

	this->mutex->lock(this->mutex);
	const status_t res = this->data->find_first(this->data,
												(linked_list_match_t)sad_entry_match,
												(void**)&entry, src, dst, &spi,
												&proto);
	if (res == SUCCESS && entry)
	{
		id = entry->esa_id;
		DBG3(DBG_KNL, "getting ESA id of SAD entry (esa: %llu, src: %H, "
			 "dst: %H, spi: %x, proto: %u)", id, src, dst, ntohl(spi),
			 proto);
	}
	else
	{
		DBG3(DBG_KNL, "no SAD entry found");
	}
	this->mutex->unlock(this->mutex);
	return id;
}

METHOD(tkm_kernel_sad_t, _remove, bool,
	private_tkm_kernel_sad_t * const this, const esa_id_type esa_id)
{
	sad_entry_t *current;
	bool removed = FALSE;
	enumerator_t *enumerator;

	this->mutex->lock(this->mutex);
	enumerator = this->data->create_enumerator(this->data);
	while (enumerator->enumerate(enumerator, (void **)&current))
	{
		if (current->esa_id == esa_id)
		{
			this->data->remove_at(this->data, enumerator);
			sad_entry_destroy(current);
			removed = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (removed)
	{
		DBG3(DBG_KNL, "removed SAD entry (esa: %llu)", esa_id);
	}
	else
	{
		DBG1(DBG_KNL, "no SAD entry with ESA id %llu found!", esa_id);
	}
	this->mutex->unlock(this->mutex);

	return removed;
}


METHOD(tkm_kernel_sad_t, destroy, void,
	private_tkm_kernel_sad_t *this)
{
	this->mutex->destroy(this->mutex);
	this->data->destroy_function(this->data, (void*)sad_entry_destroy);
	free(this);
}

/*
 * see header file
 */
tkm_kernel_sad_t *tkm_kernel_sad_create()
{
	private_tkm_kernel_sad_t *this;

	INIT(this,
		.public = {
			.insert = _insert,
			.get_esa_id = _get_esa_id,
			.remove = __remove,
			.destroy = _destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.data = linked_list_create(),
	);

	return &this->public;
}
