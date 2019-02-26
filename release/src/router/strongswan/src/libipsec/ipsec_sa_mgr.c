/*
 * Copyright (C) 2012-2017 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
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

#include "ipsec.h"
#include "ipsec_sa_mgr.h"

#include <utils/debug.h>
#include <library.h>
#include <processing/jobs/callback_job.h>
#include <threading/condvar.h>
#include <threading/mutex.h>
#include <collections/hashtable.h>
#include <collections/linked_list.h>

typedef struct private_ipsec_sa_mgr_t private_ipsec_sa_mgr_t;

/**
 * Private additions to ipsec_sa_mgr_t.
 */
struct private_ipsec_sa_mgr_t {

	/**
	 * Public members of ipsec_sa_mgr_t.
	 */
	ipsec_sa_mgr_t public;

	/**
	 * Installed SAs
	 */
	linked_list_t *sas;

	/**
	 * SPIs allocated using get_spi()
	 */
	hashtable_t *allocated_spis;

	/**
	 * Mutex used to synchronize access to the SA manager
	 */
	mutex_t *mutex;

	/**
	 * RNG used to generate SPIs
	 */
	rng_t *rng;
};

/**
 * Struct to keep track of locked IPsec SAs
 */
typedef struct {

	/**
	 * IPsec SA
	 */
	ipsec_sa_t *sa;

	/**
	 * Set if this SA is currently in use by a thread
	 */
	bool locked;

	/**
	 * Condvar used by threads to wait for this entry
	 */
	condvar_t *condvar;

	/**
	 * Number of threads waiting for this entry
	 */
	u_int waiting_threads;

	/**
	 * Set if this entry is awaiting deletion
	 */
	bool awaits_deletion;

}  ipsec_sa_entry_t;

/**
 * Helper struct for expiration events
 */
typedef struct {

	/**
	 * IPsec SA manager
	 */
	private_ipsec_sa_mgr_t *manager;

	/**
	 * Entry that expired
	 */
	ipsec_sa_entry_t *entry;

	/**
	 * SPI of the expired entry
	 */
	uint32_t spi;

	/**
	 * 0 if this is a hard expire, otherwise the offset in s (soft->hard)
	 */
	uint32_t hard_offset;

} ipsec_sa_expired_t;

/*
 * Used for the hash table of allocated SPIs
 */
static bool spi_equals(uint32_t *spi, uint32_t *other_spi)
{
	return *spi == *other_spi;
}

static u_int spi_hash(uint32_t *spi)
{
	return chunk_hash(chunk_from_thing(*spi));
}

/**
 * Create an SA entry
 */
static ipsec_sa_entry_t *create_entry(ipsec_sa_t *sa)
{
	ipsec_sa_entry_t *this;

	INIT(this,
		.condvar = condvar_create(CONDVAR_TYPE_DEFAULT),
		.sa = sa,
	);
	return this;
}

/**
 * Destroy an SA entry
 */
static void destroy_entry(ipsec_sa_entry_t *entry)
{
	entry->condvar->destroy(entry->condvar);
	entry->sa->destroy(entry->sa);
	free(entry);
}

/**
 * Makes sure an entry is safe to remove
 * Must be called with this->mutex held.
 *
 * @return			TRUE if entry can be removed, FALSE if entry is already
*					being removed by another thread
 */
static bool wait_remove_entry(private_ipsec_sa_mgr_t *this,
							  ipsec_sa_entry_t *entry)
{
	if (entry->awaits_deletion)
	{
		/* this will be deleted by another thread already */
		return FALSE;
	}
	entry->awaits_deletion = TRUE;
	while (entry->locked)
	{
		entry->condvar->wait(entry->condvar, this->mutex);
	}
	while (entry->waiting_threads > 0)
	{
		entry->condvar->broadcast(entry->condvar);
		entry->condvar->wait(entry->condvar, this->mutex);
	}
	return TRUE;
}

/**
 * Waits until an is available and then locks it.
 * Must only be called with this->mutex held
 */
static bool wait_for_entry(private_ipsec_sa_mgr_t *this,
						   ipsec_sa_entry_t *entry)
{
	while (entry->locked && !entry->awaits_deletion)
	{
		entry->waiting_threads++;
		entry->condvar->wait(entry->condvar, this->mutex);
		entry->waiting_threads--;
	}
	if (entry->awaits_deletion)
	{
		/* others may still be waiting, */
		entry->condvar->signal(entry->condvar);
		return FALSE;
	}
	entry->locked = TRUE;
	return TRUE;
}

/**
 * Flushes all entries
 * Must be called with this->mutex held.
 */
static void flush_entries(private_ipsec_sa_mgr_t *this)
{
	ipsec_sa_entry_t *current;
	enumerator_t *enumerator;

	DBG2(DBG_ESP, "flushing SAD");

	enumerator = this->sas->create_enumerator(this->sas);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		if (wait_remove_entry(this, current))
		{
			this->sas->remove_at(this->sas, enumerator);
			destroy_entry(current);
		}
	}
	enumerator->destroy(enumerator);
}

CALLBACK(match_entry_by_sa_ptr, bool,
	ipsec_sa_entry_t *item, va_list args)
{
	ipsec_sa_t *sa;

	VA_ARGS_VGET(args, sa);
	return item->sa == sa;
}

CALLBACK(match_entry_by_spi_inbound, bool,
	ipsec_sa_entry_t *item, va_list args)
{
	uint32_t spi;
	int inbound;

	VA_ARGS_VGET(args, spi, inbound);
	return item->sa->get_spi(item->sa) == spi &&
		   item->sa->is_inbound(item->sa) == inbound;
}

static bool match_entry_by_spi_src_dst(ipsec_sa_entry_t *item, uint32_t spi,
									   host_t *src, host_t *dst)
{
	return item->sa->match_by_spi_src_dst(item->sa, spi, src, dst);
}

CALLBACK(match_entry_by_spi_src_dst_cb, bool,
	ipsec_sa_entry_t *item, va_list args)
{
	host_t *src, *dst;
	uint32_t spi;

	VA_ARGS_VGET(args, spi, src, dst);
	return match_entry_by_spi_src_dst(item, spi, src, dst);
}

CALLBACK(match_entry_by_reqid_inbound, bool,
	ipsec_sa_entry_t *item, va_list args)
{
	uint32_t reqid;
	int inbound;

	VA_ARGS_VGET(args, reqid, inbound);
	return item->sa->match_by_reqid(item->sa, reqid, inbound);
}

CALLBACK(match_entry_by_spi_dst, bool,
	ipsec_sa_entry_t *item, va_list args)
{
	host_t *dst;
	uint32_t spi;

	VA_ARGS_VGET(args, spi, dst);
	return item->sa->match_by_spi_dst(item->sa, spi, dst);
}

/**
 * Remove an entry
 */
static bool remove_entry(private_ipsec_sa_mgr_t *this, ipsec_sa_entry_t *entry)
{
	ipsec_sa_entry_t *current;
	enumerator_t *enumerator;
	bool removed = FALSE;

	enumerator = this->sas->create_enumerator(this->sas);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		if (current == entry)
		{
			if (wait_remove_entry(this, current))
			{
				this->sas->remove_at(this->sas, enumerator);
				removed = TRUE;
			}
			break;
		}
	}
	enumerator->destroy(enumerator);
	return removed;
}

/**
 * Callback for expiration events
 */
static job_requeue_t sa_expired(ipsec_sa_expired_t *expired)
{
	private_ipsec_sa_mgr_t *this = expired->manager;

	this->mutex->lock(this->mutex);
	if (this->sas->find_first(this->sas, NULL, (void**)&expired->entry) &&
		expired->spi == expired->entry->sa->get_spi(expired->entry->sa))
	{	/* only if we find the right SA at this pointer location */
		uint32_t hard_offset;

		hard_offset = expired->hard_offset;
		expired->entry->sa->expire(expired->entry->sa, hard_offset == 0);
		if (hard_offset)
		{	/* soft limit reached, schedule hard expire */
			expired->hard_offset = 0;
			this->mutex->unlock(this->mutex);
			return JOB_RESCHEDULE(hard_offset);
		}
		/* hard limit reached */
		if (remove_entry(this, expired->entry))
		{
			destroy_entry(expired->entry);
		}
	}
	this->mutex->unlock(this->mutex);
	return JOB_REQUEUE_NONE;
}

/**
 * Schedule a job to handle IPsec SA expiration
 */
static void schedule_expiration(private_ipsec_sa_mgr_t *this,
								ipsec_sa_entry_t *entry)
{
	lifetime_cfg_t *lifetime = entry->sa->get_lifetime(entry->sa);
	ipsec_sa_expired_t *expired;
	callback_job_t *job;
	uint32_t timeout;

	if (!lifetime->time.life)
	{	/* no expiration at all */
		return;
	}

	INIT(expired,
		.manager = this,
		.entry = entry,
		.spi = entry->sa->get_spi(entry->sa),
	);

	/* schedule a rekey first, a hard timeout will be scheduled then, if any */
	expired->hard_offset = lifetime->time.life - lifetime->time.rekey;
	timeout = lifetime->time.rekey;

	if (lifetime->time.life <= lifetime->time.rekey ||
		lifetime->time.rekey == 0)
	{	/* no rekey, schedule hard timeout */
		expired->hard_offset = 0;
		timeout = lifetime->time.life;
	}

	job = callback_job_create((callback_job_cb_t)sa_expired, expired,
							  (callback_job_cleanup_t)free, NULL);
	lib->scheduler->schedule_job(lib->scheduler, (job_t*)job, timeout);
}

/**
 * Remove all allocated SPIs
 */
static void flush_allocated_spis(private_ipsec_sa_mgr_t *this)
{
	enumerator_t *enumerator;
	uint32_t *current;

	DBG2(DBG_ESP, "flushing allocated SPIs");
	enumerator = this->allocated_spis->create_enumerator(this->allocated_spis);
	while (enumerator->enumerate(enumerator, NULL, (void**)&current))
	{
		this->allocated_spis->remove_at(this->allocated_spis, enumerator);
		DBG2(DBG_ESP, "  removed allocated SPI %.8x", ntohl(*current));
		free(current);
	}
	enumerator->destroy(enumerator);
}

/**
 * Pre-allocate an SPI for an inbound SA
 */
static bool allocate_spi(private_ipsec_sa_mgr_t *this, uint32_t spi)
{
	uint32_t *spi_alloc;

	if (this->allocated_spis->get(this->allocated_spis, &spi) ||
		this->sas->find_first(this->sas, match_entry_by_spi_inbound,
							  NULL, spi, TRUE))
	{
		return FALSE;
	}
	spi_alloc = malloc_thing(uint32_t);
	*spi_alloc = spi;
	this->allocated_spis->put(this->allocated_spis, spi_alloc, spi_alloc);
	return TRUE;
}

METHOD(ipsec_sa_mgr_t, get_spi, status_t,
	private_ipsec_sa_mgr_t *this, host_t *src, host_t *dst, uint8_t protocol,
	uint32_t *spi)
{
	uint32_t spi_min, spi_max, spi_new;

	spi_min = lib->settings->get_int(lib->settings, "%s.spi_min",
									 0x00000100, lib->ns);
	spi_max = lib->settings->get_int(lib->settings, "%s.spi_max",
									 0xffffffff, lib->ns);
	if (spi_min > spi_max)
	{
		spi_new = spi_min;
		spi_min = spi_max;
		spi_max = spi_new;
	}
	/* make sure the SPI is valid (not in range 0-255) */
	spi_min = max(spi_min, 0x00000100);
	spi_max = max(spi_max, 0x00000100);

	this->mutex->lock(this->mutex);
	if (!this->rng)
	{
		this->rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
		if (!this->rng)
		{
			this->mutex->unlock(this->mutex);
			DBG1(DBG_ESP, "failed to create RNG for SPI generation");
			return FAILED;
		}
	}

	do
	{
		if (!this->rng->get_bytes(this->rng, sizeof(spi_new),
								 (uint8_t*)&spi_new))
		{
			this->mutex->unlock(this->mutex);
			DBG1(DBG_ESP, "failed to allocate SPI");
			return FAILED;
		}
		spi_new = spi_min + spi_new % (spi_max - spi_min + 1);
		spi_new = htonl(spi_new);
	}
	while (!allocate_spi(this, spi_new));
	this->mutex->unlock(this->mutex);

	*spi = spi_new;

	DBG2(DBG_ESP, "allocated SPI %.8x", ntohl(*spi));
	return SUCCESS;
}

METHOD(ipsec_sa_mgr_t, add_sa, status_t,
	private_ipsec_sa_mgr_t *this, host_t *src, host_t *dst, uint32_t spi,
	uint8_t protocol, uint32_t reqid,	mark_t mark, uint32_t tfc,
	lifetime_cfg_t *lifetime, uint16_t enc_alg, chunk_t enc_key,
	uint16_t int_alg, chunk_t int_key, ipsec_mode_t mode, uint16_t ipcomp,
	uint16_t cpi, bool initiator, bool encap, bool esn, bool inbound,
	bool update)
{
	ipsec_sa_entry_t *entry;
	ipsec_sa_t *sa_new;

	DBG2(DBG_ESP, "adding SAD entry with SPI %.8x and reqid {%u}",
		 ntohl(spi), reqid);
	DBG2(DBG_ESP, "  using encryption algorithm %N with key size %d",
		 encryption_algorithm_names, enc_alg, enc_key.len * 8);
	DBG2(DBG_ESP, "  using integrity algorithm %N with key size %d",
		 integrity_algorithm_names, int_alg, int_key.len * 8);

	sa_new = ipsec_sa_create(spi, src, dst, protocol, reqid, mark, tfc,
							 lifetime, enc_alg, enc_key, int_alg, int_key, mode,
							 ipcomp, cpi, encap, esn, inbound);
	if (!sa_new)
	{
		DBG1(DBG_ESP, "failed to create SAD entry");
		return FAILED;
	}

	this->mutex->lock(this->mutex);

	if (update)
	{	/* remove any pre-allocated SPIs */
		uint32_t *spi_alloc;

		spi_alloc = this->allocated_spis->remove(this->allocated_spis, &spi);
		free(spi_alloc);
	}

	if (this->sas->find_first(this->sas, match_entry_by_spi_src_dst_cb, NULL,
							  spi, src, dst))
	{
		this->mutex->unlock(this->mutex);
		DBG1(DBG_ESP, "failed to install SAD entry: already installed");
		sa_new->destroy(sa_new);
		return FAILED;
	}

	entry = create_entry(sa_new);
	schedule_expiration(this, entry);
	this->sas->insert_first(this->sas, entry);

	this->mutex->unlock(this->mutex);
	return SUCCESS;
}

METHOD(ipsec_sa_mgr_t, update_sa, status_t,
	private_ipsec_sa_mgr_t *this, uint32_t spi, uint8_t protocol,
	uint16_t cpi, host_t *src, host_t *dst, host_t *new_src, host_t *new_dst,
	bool encap, bool new_encap, mark_t mark)
{
	ipsec_sa_entry_t *entry = NULL;

	DBG2(DBG_ESP, "updating SAD entry with SPI %.8x from %#H..%#H to %#H..%#H",
		 ntohl(spi), src, dst, new_src, new_dst);

	if (!new_encap)
	{
		DBG1(DBG_ESP, "failed to update SAD entry: can't deactivate UDP "
			 "encapsulation");
		return NOT_SUPPORTED;
	}

	this->mutex->lock(this->mutex);
	if (this->sas->find_first(this->sas, match_entry_by_spi_src_dst_cb,
							 (void**)&entry, spi, src, dst) &&
		wait_for_entry(this, entry))
	{
		entry->sa->set_source(entry->sa, new_src);
		entry->sa->set_destination(entry->sa, new_dst);
		/* checkin the entry */
		entry->locked = FALSE;
		entry->condvar->signal(entry->condvar);
	}
	this->mutex->unlock(this->mutex);

	if (!entry)
	{
		DBG1(DBG_ESP, "failed to update SAD entry: not found");
		return FAILED;
	}
	return SUCCESS;
}

METHOD(ipsec_sa_mgr_t, query_sa, status_t,
	private_ipsec_sa_mgr_t *this, host_t *src, host_t *dst,
	uint32_t spi, uint8_t protocol, mark_t mark,
	uint64_t *bytes, uint64_t *packets, time_t *time)
{
	ipsec_sa_entry_t *entry = NULL;

	this->mutex->lock(this->mutex);
	if (this->sas->find_first(this->sas, match_entry_by_spi_src_dst_cb,
							 (void**)&entry, spi, src, dst) &&
		wait_for_entry(this, entry))
	{
		entry->sa->get_usestats(entry->sa, bytes, packets, time);
		/* checkin the entry */
		entry->locked = FALSE;
		entry->condvar->signal(entry->condvar);
	}
	this->mutex->unlock(this->mutex);

	return entry ? SUCCESS : NOT_FOUND;
}

METHOD(ipsec_sa_mgr_t, del_sa, status_t,
	private_ipsec_sa_mgr_t *this, host_t *src, host_t *dst, uint32_t spi,
	uint8_t protocol, uint16_t cpi, mark_t mark)
{
	ipsec_sa_entry_t *current, *found = NULL;
	enumerator_t *enumerator;

	this->mutex->lock(this->mutex);
	enumerator = this->sas->create_enumerator(this->sas);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		if (match_entry_by_spi_src_dst(current, spi, src, dst))
		{
			if (wait_remove_entry(this, current))
			{
				this->sas->remove_at(this->sas, enumerator);
				found = current;
			}
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);

	if (found)
	{
		DBG2(DBG_ESP, "deleted %sbound SAD entry with SPI %.8x",
			 found->sa->is_inbound(found->sa) ? "in" : "out", ntohl(spi));
		destroy_entry(found);
		return SUCCESS;
	}
	return FAILED;
}

METHOD(ipsec_sa_mgr_t, checkout_by_reqid, ipsec_sa_t*,
	private_ipsec_sa_mgr_t *this, uint32_t reqid, bool inbound)
{
	ipsec_sa_entry_t *entry;
	ipsec_sa_t *sa = NULL;

	this->mutex->lock(this->mutex);
	if (this->sas->find_first(this->sas, match_entry_by_reqid_inbound,
							 (void**)&entry, reqid, inbound) &&
		wait_for_entry(this, entry))
	{
		sa = entry->sa;
	}
	this->mutex->unlock(this->mutex);
	return sa;
}

METHOD(ipsec_sa_mgr_t, checkout_by_spi, ipsec_sa_t*,
	private_ipsec_sa_mgr_t *this, uint32_t spi, host_t *dst)
{
	ipsec_sa_entry_t *entry;
	ipsec_sa_t *sa = NULL;

	this->mutex->lock(this->mutex);
	if (this->sas->find_first(this->sas, match_entry_by_spi_dst,
							 (void**)&entry, spi, dst) &&
		wait_for_entry(this, entry))
	{
		sa = entry->sa;
	}
	this->mutex->unlock(this->mutex);
	return sa;
}

METHOD(ipsec_sa_mgr_t, checkin, void,
	private_ipsec_sa_mgr_t *this, ipsec_sa_t *sa)
{
	ipsec_sa_entry_t *entry;

	this->mutex->lock(this->mutex);
	if (this->sas->find_first(this->sas, match_entry_by_sa_ptr,
							 (void**)&entry, sa))
	{
		if (entry->locked)
		{
			entry->locked = FALSE;
			entry->condvar->signal(entry->condvar);
		}
	}
	this->mutex->unlock(this->mutex);
}

METHOD(ipsec_sa_mgr_t, flush_sas, status_t,
	private_ipsec_sa_mgr_t *this)
{
	this->mutex->lock(this->mutex);
	flush_entries(this);
	this->mutex->unlock(this->mutex);
	return SUCCESS;
}

METHOD(ipsec_sa_mgr_t, destroy, void,
	private_ipsec_sa_mgr_t *this)
{
	this->mutex->lock(this->mutex);
	flush_entries(this);
	flush_allocated_spis(this);
	this->mutex->unlock(this->mutex);

	this->allocated_spis->destroy(this->allocated_spis);
	this->sas->destroy(this->sas);

	this->mutex->destroy(this->mutex);
	DESTROY_IF(this->rng);
	free(this);
}

/**
 * Described in header.
 */
ipsec_sa_mgr_t *ipsec_sa_mgr_create()
{
	private_ipsec_sa_mgr_t *this;

	INIT(this,
		.public = {
			.get_spi = _get_spi,
			.add_sa = _add_sa,
			.update_sa = _update_sa,
			.query_sa = _query_sa,
			.del_sa = _del_sa,
			.checkout_by_spi = _checkout_by_spi,
			.checkout_by_reqid = _checkout_by_reqid,
			.checkin = _checkin,
			.flush_sas = _flush_sas,
			.destroy = _destroy,
		},
		.sas = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.allocated_spis = hashtable_create((hashtable_hash_t)spi_hash,
										   (hashtable_equals_t)spi_equals, 16),
	);

	return &this->public;
}
