/*
 * Copyright (C) 2012-2014 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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
	 * Reqid.
	 */
	uint32_t reqid;

	/**
	 * Source address of CHILD SA.
	 */
	host_t *src;

	/**
	 * Destination address of CHILD SA.
	 */
	host_t *dst;

	/**
	 * Local SPI of CHILD SA.
	 */
	uint32_t spi_loc;

	/**
	 * Remote SPI of CHILD SA.
	 */
	uint32_t spi_rem;

	/**
	 * Protocol of CHILD SA (ESP/AH).
	 */
	uint8_t proto;

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

CALLBACK(sad_entry_match, bool,
	sad_entry_t * const entry, va_list args)
{
	const host_t *src, *dst;
	const uint32_t *spi;
	const uint8_t *proto;
	const bool *local;

	VA_ARGS_VGET(args, src, dst, spi, proto, local);

	if (entry->src == NULL || entry->dst == NULL || entry->proto != *proto)
	{
		return FALSE;
	}
	if (*local)
	{
		return entry->src->ip_equals(entry->src, (host_t *)dst) &&
			   entry->dst->ip_equals(entry->dst, (host_t *)src) &&
			   entry->spi_loc == *spi;
	}
	return entry->src->ip_equals(entry->src, (host_t *)src) &&
		   entry->dst->ip_equals(entry->dst, (host_t *)dst) &&
		   entry->spi_rem == *spi;
}

CALLBACK(sad_entry_match_dst, bool,
	sad_entry_t * const entry, va_list args)
{
	const uint32_t *reqid, *spi;
	const uint8_t *proto;

	VA_ARGS_VGET(args, reqid, spi, proto);
	return entry->reqid   == *reqid &&
		   entry->spi_rem == *spi   &&
		   entry->proto   == *proto;
}

CALLBACK(sad_entry_equal, bool,
	sad_entry_t * const left, va_list args)
{
	sad_entry_t *right;

	VA_ARGS_VGET(args, right);

	if (left->src == NULL || left->dst == NULL || right->src == NULL ||
		right->dst == NULL)
	{
		return FALSE;
	}
	return left->esa_id == right->esa_id &&
		   left->reqid == right->reqid &&
		   left->src->ip_equals(left->src, right->src) &&
		   left->dst->ip_equals(left->dst, right->dst) &&
		   left->spi_loc == right->spi_loc &&
		   left->spi_rem == right->spi_rem &&
		   left->proto == right->proto;
}

METHOD(tkm_kernel_sad_t, insert, bool,
	private_tkm_kernel_sad_t * const this, const esa_id_type esa_id,
	const uint32_t reqid, const host_t * const src, const host_t * const dst,
	const uint32_t spi_loc, const uint32_t spi_rem, const uint8_t proto)
{
	sad_entry_t *new_entry;
	bool found;

	INIT(new_entry,
		 .esa_id = esa_id,
		 .reqid = reqid,
		 .src = (host_t *)src,
		 .dst = (host_t *)dst,
		 .spi_loc = spi_loc,
		 .spi_rem = spi_rem,
		 .proto = proto,
	);

	this->mutex->lock(this->mutex);
	found = this->data->find_first(this->data, sad_entry_equal, NULL,
									new_entry);
	if (!found)
	{
		DBG3(DBG_KNL, "inserting SAD entry (esa: %llu, reqid: %u, src: %H, "
			 "dst: %H, spi_loc: %x, spi_rem: %x,proto: %u)", esa_id, reqid, src,
			 dst, ntohl(spi_loc), ntohl(spi_rem), proto);
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
	return !found;
}

METHOD(tkm_kernel_sad_t, get_esa_id, esa_id_type,
	private_tkm_kernel_sad_t * const this, const host_t * const src,
	const host_t * const dst, const uint32_t spi, const uint8_t proto,
	const bool local)
{
	esa_id_type id = 0;
	sad_entry_t *entry = NULL;

	this->mutex->lock(this->mutex);
	const bool res = this->data->find_first(this->data, sad_entry_match,
											(void**)&entry, src, dst, &spi,
											&proto, &local);
	if (res && entry)
	{
		id = entry->esa_id;
		DBG3(DBG_KNL, "returning ESA id %llu of SAD entry (src: %H, dst: %H, "
			 "%sbound spi: %x, proto: %u)", id, src, dst, local ? "in" : "out",
			 ntohl(spi), proto);
	}
	else
	{
		DBG3(DBG_KNL, "no SAD entry found for src %H, dst %H, %sbound spi %x, "
			 "proto %u", src, dst, local ? "in" : "out", ntohl(spi), proto);
	}
	this->mutex->unlock(this->mutex);
	return id;
}

METHOD(tkm_kernel_sad_t, get_dst_host, host_t *,
	private_tkm_kernel_sad_t * const this, const uint32_t reqid,
	const uint32_t spi, const uint8_t proto)
{
	host_t *dst = NULL;
	sad_entry_t *entry = NULL;

	this->mutex->lock(this->mutex);
	const bool res = this->data->find_first(this->data, sad_entry_match_dst,
											(void**)&entry, &reqid, &spi, &proto);
	if (res && entry)
	{
		dst = entry->dst->clone(entry->dst);
		DBG3(DBG_KNL, "returning destination host %H of SAD entry (reqid: %u,"
			 " spi: %x, proto: %u)", dst, reqid, ntohl(spi), proto);
	}
	else
	{
		DBG3(DBG_KNL, "no SAD entry found for reqid %u, spi %x, proto: %u",
			 reqid, ntohl(spi), proto);
	}
	this->mutex->unlock(this->mutex);
	return dst;
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
			.get_dst_host = _get_dst_host,
			.remove = __remove,
			.destroy = _destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.data = linked_list_create(),
	);

	return &this->public;
}
