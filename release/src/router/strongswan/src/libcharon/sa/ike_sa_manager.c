/*
 * Copyright (C) 2008-2022 Tobias Brunner
 * Copyright (C) 2005-2011 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 *
 * Copyright (C) secunet Security Networks AG
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
#include <inttypes.h>

#include "ike_sa_manager.h"

#include <daemon.h>
#include <sa/ike_sa_id.h>
#include <bus/bus.h>
#include <threading/thread.h>
#include <threading/condvar.h>
#include <threading/mutex.h>
#include <threading/rwlock.h>
#include <collections/array.h>
#include <collections/linked_list.h>
#include <crypto/hashers/hasher.h>
#include <processing/jobs/delete_ike_sa_job.h>

/* the default size of the hash table (MUST be a power of 2) */
#define DEFAULT_HASHTABLE_SIZE 1

/* the maximum size of the hash table (MUST be a power of 2) */
#define MAX_HASHTABLE_SIZE (1 << 30)

/* the default number of segments (MUST be a power of 2) */
#define DEFAULT_SEGMENT_COUNT 1

typedef struct entry_t entry_t;

/**
 * An entry in the linked list, contains IKE_SA, locking and lookup data.
 */
struct entry_t {

	/**
	 * Number of threads waiting for this ike_sa_t object.
	 */
	int waiting_threads;

	/**
	 * Condvar where threads can wait until ike_sa_t object is free for use again.
	 */
	condvar_t *condvar;

	/**
	 * Thread by which this IKE_SA is currently checked out, if any
	 */
	thread_t *checked_out;

	/**
	 * Whether threads are prevented from getting this IKE_SA.
	 */
	bool driveout_threads;

	/**
	 * Identification of an IKE_SA (SPIs).
	 */
	ike_sa_id_t *ike_sa_id;

	/**
	 * The contained ike_sa_t object.
	 */
	ike_sa_t *ike_sa;

	/**
	 * hash of the IKE_SA_INIT message, used to detect retransmissions
	 */
	chunk_t init_hash;

	/**
	 * remote host address, required for DoS detection and duplicate
	 * checking (host with same my_id and other_id is *not* considered
	 * a duplicate if the address family differs)
	 */
	host_t *other;

	/**
	 * As responder: Is this SA half-open?
	 */
	bool half_open;

	/**
	 * own identity, required for duplicate checking
	 */
	identification_t *my_id;

	/**
	 * remote identity, required for duplicate checking
	 */
	identification_t *other_id;

	/**
	 * message ID or hash of currently processing message, -1 if none
	 */
	uint32_t processing;
};

/**
 * Implementation of entry_t.destroy.
 */
static status_t entry_destroy(entry_t *this)
{
	/* also destroy IKE SA */
	this->ike_sa->destroy(this->ike_sa);
	this->ike_sa_id->destroy(this->ike_sa_id);
	chunk_free(&this->init_hash);
	DESTROY_IF(this->other);
	DESTROY_IF(this->my_id);
	DESTROY_IF(this->other_id);
	this->condvar->destroy(this->condvar);
	free(this);
	return SUCCESS;
}

/**
 * Creates a new entry for the ike_sa_t list.
 */
static entry_t *entry_create()
{
	entry_t *this;

	INIT(this,
		.condvar = condvar_create(CONDVAR_TYPE_DEFAULT),
		.processing = -1,
	);

	return this;
}

/**
 * Function that matches entry_t objects by ike_sa_id_t.
 */
static bool entry_match_by_id(entry_t *entry, void *arg)
{
	ike_sa_id_t *id = arg;

	if (id->equals(id, entry->ike_sa_id))
	{
		return TRUE;
	}
	if ((id->get_responder_spi(id) == 0 ||
		 entry->ike_sa_id->get_responder_spi(entry->ike_sa_id) == 0) &&
		(id->get_ike_version(id) == IKEV1_MAJOR_VERSION ||
		 id->is_initiator(id) == entry->ike_sa_id->is_initiator(entry->ike_sa_id)) &&
		id->get_initiator_spi(id) == entry->ike_sa_id->get_initiator_spi(entry->ike_sa_id))
	{
		/* this is TRUE for IKE_SAs that we initiated but have not yet received a response */
		return TRUE;
	}
	return FALSE;
}

/**
 * Function that matches entry_t objects by ike_sa_t pointers.
 */
static bool entry_match_by_sa(entry_t *entry, void *ike_sa)
{
	return entry->ike_sa == ike_sa;
}

/**
 * Hash function for ike_sa_id_t objects.
 */
static u_int ike_sa_id_hash(ike_sa_id_t *ike_sa_id)
{
	/* IKEv2 does not mandate random SPIs (RFC 5996, 2.6), they just have to be
	 * locally unique, so we use our randomly allocated SPI whether we are
	 * initiator or responder to ensure a good distribution.  The latter is not
	 * possible for IKEv1 as we don't know whether we are original initiator or
	 * not (based on the IKE header).  But as RFC 2408, section 2.5.3 proposes
	 * SPIs (Cookies) to be allocated near random (we allocate them randomly
	 * anyway) it seems safe to always use the initiator SPI. */
	if (ike_sa_id->get_ike_version(ike_sa_id) == IKEV1_MAJOR_VERSION ||
		ike_sa_id->is_initiator(ike_sa_id))
	{
		return ike_sa_id->get_initiator_spi(ike_sa_id);
	}
	return ike_sa_id->get_responder_spi(ike_sa_id);
}

typedef struct half_open_t half_open_t;

/**
 * Struct to manage half-open IKE_SAs per peer.
 */
struct half_open_t {
	/** chunk of remote host address */
	chunk_t other;

	/** the number of half-open IKE_SAs with that host */
	u_int count;

	/** the number of half-open IKE_SAs we responded to with that host */
	u_int count_responder;
};

/**
 * Destroys a half_open_t object.
 */
static void half_open_destroy(half_open_t *this)
{
	chunk_free(&this->other);
	free(this);
}

typedef struct connected_peers_t connected_peers_t;

struct connected_peers_t {
	/** own identity */
	identification_t *my_id;

	/** remote identity */
	identification_t *other_id;

	/** ip address family of peer */
	int family;

	/** list of ike_sa_id_t objects of IKE_SAs between the two identities */
	linked_list_t *sas;
};

static void connected_peers_destroy(connected_peers_t *this)
{
	this->my_id->destroy(this->my_id);
	this->other_id->destroy(this->other_id);
	this->sas->destroy(this->sas);
	free(this);
}

/**
 * Function that matches connected_peers_t objects by the given ids.
 */
static inline bool connected_peers_match(connected_peers_t *connected_peers,
							identification_t *my_id, identification_t *other_id,
							int family)
{
	return my_id->equals(my_id, connected_peers->my_id) &&
		   other_id->equals(other_id, connected_peers->other_id) &&
		   (!family || family == connected_peers->family);
}

typedef struct init_hash_t init_hash_t;

struct init_hash_t {
	/** hash of IKE_SA_INIT or initial phase1 message (data is not cloned) */
	chunk_t hash;

	/** our SPI allocated for the IKE_SA based on this message */
	uint64_t our_spi;
};

typedef struct segment_t segment_t;

/**
 * Struct to manage segments of the hash table.
 */
struct segment_t {
	/** mutex to access a segment exclusively */
	mutex_t *mutex;
};

typedef struct shareable_segment_t shareable_segment_t;

/**
 * Struct to manage segments of the "half-open" and "connected peers" hash tables.
 */
struct shareable_segment_t {
	/** rwlock to access a segment non-/exclusively */
	rwlock_t *lock;

	/** the number of entries in this segment - in case of the "half-open table"
	 * it's the sum of all half_open_t.count in a segment. */
	u_int count;
};

typedef struct table_item_t table_item_t;

/**
 * Instead of using linked_list_t for each bucket we store the data in our own
 * list to save memory.
 */
struct table_item_t {
	/** data of this item */
	void *value;

	/** next item in the overflow list */
	table_item_t *next;
};

typedef struct private_ike_sa_manager_t private_ike_sa_manager_t;

/**
 * Additional private members of ike_sa_manager_t.
 */
struct private_ike_sa_manager_t {
	/**
	 * Public interface of ike_sa_manager_t.
	 */
	ike_sa_manager_t public;

	/**
	 * Hash table with entries for the ike_sa_t objects.
	 */
	table_item_t **ike_sa_table;

	/**
	 * The size of the hash table.
	 */
	u_int table_size;

	/**
	 * Mask to map the hashes to table rows.
	 */
	u_int table_mask;

	/**
	 * Segments of the hash table.
	 */
	segment_t *segments;

	/**
	 * The number of segments.
	 */
	u_int segment_count;

	/**
	 * Mask to map a table row to a segment.
	 */
	u_int segment_mask;

	/**
	 * Enabled while shutting down to mark all new entries so no threads
	 * will check them out.
	 */
	bool new_entries_driveout_threads;

	/**
	 * Hash table with half_open_t objects.
	 */
	table_item_t **half_open_table;

	/**
	  * Segments of the "half-open" hash table.
	 */
	shareable_segment_t *half_open_segments;

	/**
	 * Total number of half-open IKE_SAs.
	 */
	refcount_t half_open_count;

	/**
	 * Total number of half-open IKE_SAs as responder.
	 */
	refcount_t half_open_count_responder;

	/**
	 * Total number of IKE_SAs registered with IKE_SA manager.
	 */
	refcount_t total_sa_count;

	/**
	 * Hash table with connected_peers_t objects.
	 */
	table_item_t **connected_peers_table;

	/**
	 * Segments of the "connected peers" hash table.
	 */
	shareable_segment_t *connected_peers_segments;

	/**
	 * Hash table with init_hash_t objects.
	 */
	table_item_t **init_hashes_table;

	/**
	 * Segments of the "hashes" hash table.
	 */
	segment_t *init_hashes_segments;

	/**
	 * Configs for which an SA is currently being checked out.
	 */
	array_t *config_checkouts;

	/**
	 * Mutex to protect access to configs.
	 */
	mutex_t *config_mutex;

	/**
	 * Condvar to indicate changes in checkout configs.
	 */
	condvar_t *config_condvar;

	/**
	 * RNG to get random SPIs for our side
	 */
	rng_t *rng;

	/**
	 * Registered callback for IKE SPIs
	 */
	struct {
		spi_cb_t cb;
		void *data;
	} spi_cb;

	/**
	 * Lock to access the RNG instance and the callback
	 */
	rwlock_t *spi_lock;

	/**
	 * Mask applied to local SPIs before mixing in the label
	 */
	uint64_t spi_mask;

	/**
	 * Label applied to local SPIs
	 */
	uint64_t spi_label;

	/**
	 * reuse existing IKE_SAs in checkout_by_config
	 */
	bool reuse_ikesa;

	/**
	 * Configured IKE_SA limit, if any
	 */
	u_int ikesa_limit;
};

/**
 * Acquire a lock to access the segment of the table row with the given index.
 * It also works with the segment index directly.
 */
static inline void lock_single_segment(private_ike_sa_manager_t *this,
									   u_int index)
{
	mutex_t *lock = this->segments[index & this->segment_mask].mutex;
	lock->lock(lock);
}

/**
 * Release the lock required to access the segment of the table row with the given index.
 * It also works with the segment index directly.
 */
static inline void unlock_single_segment(private_ike_sa_manager_t *this,
										 u_int index)
{
	mutex_t *lock = this->segments[index & this->segment_mask].mutex;
	lock->unlock(lock);
}

/**
 * Lock all segments
 */
static void lock_all_segments(private_ike_sa_manager_t *this)
{
	u_int i;

	for (i = 0; i < this->segment_count; i++)
	{
		this->segments[i].mutex->lock(this->segments[i].mutex);
	}
}

/**
 * Unlock all segments
 */
static void unlock_all_segments(private_ike_sa_manager_t *this)
{
	u_int i;

	for (i = 0; i < this->segment_count; i++)
	{
		this->segments[i].mutex->unlock(this->segments[i].mutex);
	}
}

typedef struct private_enumerator_t private_enumerator_t;

/**
 * hash table enumerator implementation
 */
struct private_enumerator_t {

	/**
	 * implements enumerator interface
	 */
	enumerator_t enumerator;

	/**
	 * associated ike_sa_manager_t
	 */
	private_ike_sa_manager_t *manager;

	/**
	 * current segment index
	 */
	u_int segment;

	/**
	 * currently enumerating entry
	 */
	entry_t *entry;

	/**
	 * current table row index
	 */
	u_int row;

	/**
	 * current table item
	 */
	table_item_t *current;

	/**
	 * previous table item
	 */
	table_item_t *prev;
};

METHOD(enumerator_t, enumerate, bool,
	private_enumerator_t *this, va_list args)
{
	entry_t **entry;
	u_int *segment;

	VA_ARGS_VGET(args, entry, segment);

	if (this->entry)
	{
		this->entry->condvar->signal(this->entry->condvar);
		this->entry = NULL;
	}
	while (this->segment < this->manager->segment_count)
	{
		while (this->row < this->manager->table_size)
		{
			this->prev = this->current;
			if (this->current)
			{
				this->current = this->current->next;
			}
			else
			{
				lock_single_segment(this->manager, this->segment);
				this->current = this->manager->ike_sa_table[this->row];
			}
			if (this->current)
			{
				*entry = this->entry = this->current->value;
				*segment = this->segment;
				return TRUE;
			}
			unlock_single_segment(this->manager, this->segment);
			this->row += this->manager->segment_count;
		}
		this->segment++;
		this->row = this->segment;
	}
	return FALSE;
}

METHOD(enumerator_t, enumerator_destroy, void,
	private_enumerator_t *this)
{
	if (this->entry)
	{
		this->entry->condvar->signal(this->entry->condvar);
	}
	if (this->current)
	{
		unlock_single_segment(this->manager, this->segment);
	}
	free(this);
}

/**
 * Creates an enumerator to enumerate the entries in the hash table.
 */
static enumerator_t* create_table_enumerator(private_ike_sa_manager_t *this)
{
	private_enumerator_t *enumerator;

	INIT(enumerator,
		.enumerator = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate,
			.destroy = _enumerator_destroy,
		},
		.manager = this,
	);
	return &enumerator->enumerator;
}

/**
 * Put an entry into the hash table.
 * Note: The caller has to unlock the returned segment.
 */
static u_int put_entry(private_ike_sa_manager_t *this, entry_t *entry)
{
	table_item_t *current, *item;
	u_int row, segment;

	INIT(item,
		.value = entry,
	);

	row = ike_sa_id_hash(entry->ike_sa_id) & this->table_mask;
	segment = row & this->segment_mask;

	lock_single_segment(this, segment);
	/* mark entry during shutdown */
	entry->driveout_threads = this->new_entries_driveout_threads;
	current = this->ike_sa_table[row];
	if (current)
	{	/* insert at the front of current bucket */
		item->next = current;
	}
	this->ike_sa_table[row] = item;
	ref_get(&this->total_sa_count);
	return segment;
}

/**
 * Remove an entry from the hash table.
 * Note: The caller MUST have a lock on the segment of this entry.
 */
static void remove_entry(private_ike_sa_manager_t *this, entry_t *entry)
{
	table_item_t *item, *prev = NULL;
	u_int row;

	row = ike_sa_id_hash(entry->ike_sa_id) & this->table_mask;
	item = this->ike_sa_table[row];
	while (item)
	{
		if (item->value == entry)
		{
			if (prev)
			{
				prev->next = item->next;
			}
			else
			{
				this->ike_sa_table[row] = item->next;
			}
			ignore_result(ref_put(&this->total_sa_count));
			free(item);
			break;
		}
		prev = item;
		item = item->next;
	}
}

/**
 * Remove the entry at the current enumerator position.
 */
static void remove_entry_at(private_enumerator_t *this)
{
	this->entry = NULL;
	if (this->current)
	{
		table_item_t *current = this->current;

		ignore_result(ref_put(&this->manager->total_sa_count));
		this->current = this->prev;

		if (this->prev)
		{
			this->prev->next = current->next;
		}
		else
		{
			this->manager->ike_sa_table[this->row] = current->next;
			unlock_single_segment(this->manager, this->segment);
		}
		free(current);
	}
}

/**
 * Find an entry using the provided match function to compare the entries for
 * equality.
 */
static status_t get_entry_by_match_function(private_ike_sa_manager_t *this,
					ike_sa_id_t *ike_sa_id, entry_t **entry, u_int *segment,
					bool (*match)(entry_t*,void*), void *param)
{
	table_item_t *item;
	u_int row, seg;

	row = ike_sa_id_hash(ike_sa_id) & this->table_mask;
	seg = row & this->segment_mask;

	lock_single_segment(this, seg);
	item = this->ike_sa_table[row];
	while (item)
	{
		if (match(item->value, param))
		{
			*entry = item->value;
			*segment = seg;
			/* the locked segment has to be unlocked by the caller */
			return SUCCESS;
		}
		item = item->next;
	}
	unlock_single_segment(this, seg);
	return NOT_FOUND;
}

/**
 * Find an entry by ike_sa_id_t.
 * Note: On SUCCESS, the caller has to unlock the segment.
 */
static status_t get_entry_by_id(private_ike_sa_manager_t *this,
						ike_sa_id_t *ike_sa_id, entry_t **entry, u_int *segment)
{
	return get_entry_by_match_function(this, ike_sa_id, entry, segment,
									   entry_match_by_id, ike_sa_id);
}

/**
 * Find an entry by IKE_SA pointer.
 * Note: On SUCCESS, the caller has to unlock the segment.
 */
static status_t get_entry_by_sa(private_ike_sa_manager_t *this,
			ike_sa_id_t *ike_sa_id, ike_sa_t *ike_sa, entry_t **entry, u_int *segment)
{
	return get_entry_by_match_function(this, ike_sa_id, entry, segment,
									   entry_match_by_sa, ike_sa);
}

/**
 * Wait until no other thread is using an IKE_SA, return FALSE if entry not
 * acquirable.
 */
static bool wait_for_entry(private_ike_sa_manager_t *this, entry_t *entry,
						   u_int segment)
{
	if (entry->driveout_threads)
	{
		/* we are not allowed to get this */
		return FALSE;
	}
	while (entry->checked_out && !entry->driveout_threads)
	{
		/* so wait until we can get it for us.
		 * we register us as waiting. */
		entry->waiting_threads++;
		entry->condvar->wait(entry->condvar, this->segments[segment].mutex);
		entry->waiting_threads--;
	}
	/* hm, a deletion request forbids us to get this SA, get next one */
	if (entry->driveout_threads)
	{
		/* we must signal here, others may be waiting on it, too */
		entry->condvar->signal(entry->condvar);
		return FALSE;
	}
	return TRUE;
}

/**
 * Put a half-open SA into the hash table.
 */
static void put_half_open(private_ike_sa_manager_t *this, host_t *ip,
						  bool initiator)
{
	table_item_t *item;
	u_int row, segment;
	rwlock_t *lock;
	half_open_t *half_open;
	chunk_t addr;

	addr = ip->get_address(ip);
	row = chunk_hash(addr) & this->table_mask;
	segment = row & this->segment_mask;
	lock = this->half_open_segments[segment].lock;
	lock->write_lock(lock);
	item = this->half_open_table[row];
	while (item)
	{
		half_open = item->value;

		if (chunk_equals(addr, half_open->other))
		{
			break;
		}
		item = item->next;
	}

	if (!item)
	{
		INIT(half_open,
			.other = chunk_clone(addr),
		);
		INIT(item,
			.value = half_open,
			.next = this->half_open_table[row],
		);
		this->half_open_table[row] = item;
	}
	half_open->count++;
	ref_get(&this->half_open_count);
	if (!initiator)
	{
		half_open->count_responder++;
		ref_get(&this->half_open_count_responder);
	}
	this->half_open_segments[segment].count++;
	lock->unlock(lock);
}

/**
 * Remove a half-open SA from the hash table.
 */
static void remove_half_open(private_ike_sa_manager_t *this, host_t *ip,
							 bool initiator)
{
	table_item_t *item, *prev = NULL;
	u_int row, segment;
	rwlock_t *lock;
	chunk_t addr;

	addr = ip->get_address(ip);
	row = chunk_hash(addr) & this->table_mask;
	segment = row & this->segment_mask;
	lock = this->half_open_segments[segment].lock;
	lock->write_lock(lock);
	item = this->half_open_table[row];
	while (item)
	{
		half_open_t *half_open = item->value;

		if (chunk_equals(addr, half_open->other))
		{
			if (!initiator)
			{
				half_open->count_responder--;
				ignore_result(ref_put(&this->half_open_count_responder));
			}
			ignore_result(ref_put(&this->half_open_count));
			if (--half_open->count == 0)
			{
				if (prev)
				{
					prev->next = item->next;
				}
				else
				{
					this->half_open_table[row] = item->next;
				}
				half_open_destroy(half_open);
				free(item);
			}
			this->half_open_segments[segment].count--;
			break;
		}
		prev = item;
		item = item->next;
	}
	lock->unlock(lock);
}

/**
 * Create an entry and put it into the hash table.
 * Note: The caller has to unlock the segment.
 */
static u_int create_and_put_entry(private_ike_sa_manager_t *this,
								  ike_sa_t *ike_sa, entry_t **entry)
{
	ike_sa_id_t *ike_sa_id = ike_sa->get_id(ike_sa);
	host_t *other = ike_sa->get_other_host(ike_sa);

	*entry = entry_create();
	(*entry)->ike_sa_id = ike_sa_id->clone(ike_sa_id);
	(*entry)->ike_sa = ike_sa;

	if (ike_sa->get_state(ike_sa) == IKE_CONNECTING)
	{
		(*entry)->half_open = TRUE;
		(*entry)->other = other->clone(other);
		put_half_open(this, (*entry)->other, ike_sa_id->is_initiator(ike_sa_id));
	}
	return put_entry(this, *entry);
}

CALLBACK(id_matches, bool,
	ike_sa_id_t *a, va_list args)
{
	ike_sa_id_t *b;

	VA_ARGS_VGET(args, b);
	return a->equals(a, b);
}

/**
 * Put an SA between two peers into the hash table.
 */
static void put_connected_peers(private_ike_sa_manager_t *this, entry_t *entry)
{
	table_item_t *item;
	u_int row, segment;
	rwlock_t *lock;
	connected_peers_t *connected_peers;
	chunk_t my_id, other_id;
	int family;

	my_id = entry->my_id->get_encoding(entry->my_id);
	other_id = entry->other_id->get_encoding(entry->other_id);
	family = entry->other->get_family(entry->other);
	row = chunk_hash_inc(other_id, chunk_hash(my_id)) & this->table_mask;
	segment = row & this->segment_mask;
	lock = this->connected_peers_segments[segment].lock;
	lock->write_lock(lock);
	item = this->connected_peers_table[row];
	while (item)
	{
		connected_peers = item->value;

		if (connected_peers_match(connected_peers, entry->my_id,
								  entry->other_id, family))
		{
			if (connected_peers->sas->find_first(connected_peers->sas,
											id_matches, NULL, entry->ike_sa_id))
			{
				lock->unlock(lock);
				return;
			}
			break;
		}
		item = item->next;
	}

	if (!item)
	{
		INIT(connected_peers,
			.my_id = entry->my_id->clone(entry->my_id),
			.other_id = entry->other_id->clone(entry->other_id),
			.family = family,
			.sas = linked_list_create(),
		);
		INIT(item,
			.value = connected_peers,
			.next = this->connected_peers_table[row],
		);
		this->connected_peers_table[row] = item;
	}
	connected_peers->sas->insert_last(connected_peers->sas,
									  entry->ike_sa_id->clone(entry->ike_sa_id));
	this->connected_peers_segments[segment].count++;
	lock->unlock(lock);
}

/**
 * Remove an SA between two peers from the hash table.
 */
static void remove_connected_peers(private_ike_sa_manager_t *this, entry_t *entry)
{
	table_item_t *item, *prev = NULL;
	u_int row, segment;
	rwlock_t *lock;
	chunk_t my_id, other_id;
	int family;

	my_id = entry->my_id->get_encoding(entry->my_id);
	other_id = entry->other_id->get_encoding(entry->other_id);
	family = entry->other->get_family(entry->other);

	row = chunk_hash_inc(other_id, chunk_hash(my_id)) & this->table_mask;
	segment = row & this->segment_mask;

	lock = this->connected_peers_segments[segment].lock;
	lock->write_lock(lock);
	item = this->connected_peers_table[row];
	while (item)
	{
		connected_peers_t *current = item->value;

		if (connected_peers_match(current, entry->my_id, entry->other_id,
								  family))
		{
			enumerator_t *enumerator;
			ike_sa_id_t *ike_sa_id;

			enumerator = current->sas->create_enumerator(current->sas);
			while (enumerator->enumerate(enumerator, &ike_sa_id))
			{
				if (ike_sa_id->equals(ike_sa_id, entry->ike_sa_id))
				{
					current->sas->remove_at(current->sas, enumerator);
					ike_sa_id->destroy(ike_sa_id);
					this->connected_peers_segments[segment].count--;
					break;
				}
			}
			enumerator->destroy(enumerator);
			if (current->sas->get_count(current->sas) == 0)
			{
				if (prev)
				{
					prev->next = item->next;
				}
				else
				{
					this->connected_peers_table[row] = item->next;
				}
				connected_peers_destroy(current);
				free(item);
			}
			break;
		}
		prev = item;
		item = item->next;
	}
	lock->unlock(lock);
}

/**
 * Get a random SPI for new IKE_SAs
 */
static uint64_t get_spi(private_ike_sa_manager_t *this)
{
	uint64_t spi;

	this->spi_lock->read_lock(this->spi_lock);
	if (this->spi_cb.cb)
	{
		spi = this->spi_cb.cb(this->spi_cb.data);
	}
	else if (!this->rng ||
			 !this->rng->get_bytes(this->rng, sizeof(spi), (uint8_t*)&spi))
	{
		spi = 0;
	}
	this->spi_lock->unlock(this->spi_lock);

	if (spi)
	{
		spi = (spi & ~this->spi_mask) | this->spi_label;
	}
	return spi;
}

/**
 * Calculate the hash of the initial IKE message.  Memory for the hash is
 * allocated on success.
 *
 * @returns TRUE on success
 */
static bool get_init_hash(hasher_t *hasher, message_t *message, chunk_t *hash)
{
	host_t *src;

	if (message->get_first_payload_type(message) == PLV1_FRAGMENT)
	{	/* only hash the source IP, port and SPI for fragmented init messages */
		uint16_t port;
		uint64_t spi;

		src = message->get_source(message);
		if (!hasher->allocate_hash(hasher, src->get_address(src), NULL))
		{
			return FALSE;
		}
		port = src->get_port(src);
		if (!hasher->allocate_hash(hasher, chunk_from_thing(port), NULL))
		{
			return FALSE;
		}
		spi = message->get_initiator_spi(message);
		return hasher->allocate_hash(hasher, chunk_from_thing(spi), hash);
	}
	if (message->get_exchange_type(message) == ID_PROT)
	{	/* include the source for Main Mode as the hash will be the same if
		 * SPIs are reused by two initiators that use the same proposal */
		src = message->get_source(message);

		if (!hasher->allocate_hash(hasher, src->get_address(src), NULL))
		{
			return FALSE;
		}
	}
	return hasher->allocate_hash(hasher, message->get_packet_data(message), hash);
}

/**
 * Check if we already have created an IKE_SA based on the initial IKE message
 * with the given hash.
 * If not the hash is stored, the hash data is not(!) cloned.
 *
 * Also, the local SPI is returned.  In case of a retransmit this is already
 * stored together with the hash, otherwise it is newly allocated and should
 * be used to create the IKE_SA.
 *
 * @returns ALREADY_DONE if the message with the given hash has been seen before
 *			NOT_FOUND if the message hash was not found
 *			FAILED if the SPI allocation failed
 */
static status_t check_and_put_init_hash(private_ike_sa_manager_t *this,
										chunk_t init_hash, uint64_t *our_spi)
{
	table_item_t *item;
	u_int row, segment;
	mutex_t *mutex;
	init_hash_t *init;
	uint64_t spi;

	row = chunk_hash(init_hash) & this->table_mask;
	segment = row & this->segment_mask;
	mutex = this->init_hashes_segments[segment].mutex;
	mutex->lock(mutex);
	item = this->init_hashes_table[row];
	while (item)
	{
		init_hash_t *current = item->value;

		if (chunk_equals(init_hash, current->hash))
		{
			*our_spi = current->our_spi;
			mutex->unlock(mutex);
			return ALREADY_DONE;
		}
		item = item->next;
	}

	spi = get_spi(this);
	if (!spi)
	{
		return FAILED;
	}

	INIT(init,
		.hash = {
			.len = init_hash.len,
			.ptr = init_hash.ptr,
		},
		.our_spi = spi,
	);
	INIT(item,
		.value = init,
		.next = this->init_hashes_table[row],
	);
	this->init_hashes_table[row] = item;
	*our_spi = init->our_spi;
	mutex->unlock(mutex);
	return NOT_FOUND;
}

/**
 * Remove the hash of an initial IKE message from the cache.
 */
static void remove_init_hash(private_ike_sa_manager_t *this, chunk_t init_hash)
{
	table_item_t *item, *prev = NULL;
	u_int row, segment;
	mutex_t *mutex;

	row = chunk_hash(init_hash) & this->table_mask;
	segment = row & this->segment_mask;
	mutex = this->init_hashes_segments[segment].mutex;
	mutex->lock(mutex);
	item = this->init_hashes_table[row];
	while (item)
	{
		init_hash_t *current = item->value;

		if (chunk_equals(init_hash, current->hash))
		{
			if (prev)
			{
				prev->next = item->next;
			}
			else
			{
				this->init_hashes_table[row] = item->next;
			}
			free(current);
			free(item);
			break;
		}
		prev = item;
		item = item->next;
	}
	mutex->unlock(mutex);
}

METHOD(ike_sa_manager_t, checkout, ike_sa_t*,
	private_ike_sa_manager_t *this, ike_sa_id_t *ike_sa_id)
{
	ike_sa_t *ike_sa = NULL;
	entry_t *entry;
	u_int segment;

	DBG2(DBG_MGR, "checkout %N SA with SPIs %.16"PRIx64"_i %.16"PRIx64"_r",
		 ike_version_names, ike_sa_id->get_ike_version(ike_sa_id),
		 be64toh(ike_sa_id->get_initiator_spi(ike_sa_id)),
		 be64toh(ike_sa_id->get_responder_spi(ike_sa_id)));

	if (get_entry_by_id(this, ike_sa_id, &entry, &segment) == SUCCESS)
	{
		if (wait_for_entry(this, entry, segment))
		{
			entry->checked_out = thread_current();
			ike_sa = entry->ike_sa;
			DBG2(DBG_MGR, "IKE_SA %s[%u] successfully checked out",
					ike_sa->get_name(ike_sa), ike_sa->get_unique_id(ike_sa));
		}
		unlock_single_segment(this, segment);
	}
	charon->bus->set_sa(charon->bus, ike_sa);

	if (!ike_sa)
	{
		DBG2(DBG_MGR, "IKE_SA checkout not successful");
	}
	return ike_sa;
}

METHOD(ike_sa_manager_t, create_new, ike_sa_t*,
	private_ike_sa_manager_t* this, ike_version_t version, bool initiator)
{
	ike_sa_id_t *ike_sa_id;
	ike_sa_t *ike_sa;
	uint8_t ike_version;
	uint64_t spi;

	ike_version = version == IKEV1 ? IKEV1_MAJOR_VERSION : IKEV2_MAJOR_VERSION;

	spi = get_spi(this);
	if (!spi)
	{
		DBG1(DBG_MGR, "failed to allocate SPI for new IKE_SA");
		return NULL;
	}

	if (initiator)
	{
		ike_sa_id = ike_sa_id_create(ike_version, spi, 0, TRUE);
	}
	else
	{
		ike_sa_id = ike_sa_id_create(ike_version, 0, spi, FALSE);
	}
	ike_sa = ike_sa_create(ike_sa_id, initiator, version);
	ike_sa_id->destroy(ike_sa_id);

	if (ike_sa)
	{
		DBG2(DBG_MGR, "created IKE_SA %s[%u]", ike_sa->get_name(ike_sa),
			 ike_sa->get_unique_id(ike_sa));
	}
	return ike_sa;
}

METHOD(ike_sa_manager_t, checkout_new, void,
	private_ike_sa_manager_t *this, ike_sa_t *ike_sa)
{
	u_int segment;
	entry_t *entry;

	segment = create_and_put_entry(this, ike_sa, &entry);
	entry->checked_out = thread_current();
	unlock_single_segment(this, segment);
}

/**
 * Get the message ID or message hash to detect early retransmissions
 */
static uint32_t get_message_id_or_hash(message_t *message)
{
	if (message->get_major_version(message) == IKEV1_MAJOR_VERSION)
	{
		/* Use a hash for IKEv1 Phase 1, where we don't have a MID, and Quick
		 * Mode, where all three messages use the same message ID */
		if (message->get_message_id(message) == 0 ||
			message->get_exchange_type(message) == QUICK_MODE)
		{
			return chunk_hash(message->get_packet_data(message));
		}
	}
	return message->get_message_id(message);
}

METHOD(ike_sa_manager_t, checkout_by_message, ike_sa_t*,
	private_ike_sa_manager_t* this, message_t *message)
{
	u_int segment;
	entry_t *entry;
	ike_sa_t *ike_sa = NULL;
	ike_sa_id_t *id;
	ike_version_t ike_version;
	bool is_init = FALSE, untrack_half_open = FALSE;

	id = message->get_ike_sa_id(message);
	/* clone the IKE_SA ID so we can modify the initiator flag */
	id = id->clone(id);
	id->switch_initiator(id);

	DBG2(DBG_MGR, "checkout %N SA by message with SPIs %.16"PRIx64"_i "
		 "%.16"PRIx64"_r", ike_version_names, id->get_ike_version(id),
		 be64toh(id->get_initiator_spi(id)),
		 be64toh(id->get_responder_spi(id)));

	if (id->get_responder_spi(id) == 0 &&
		message->get_message_id(message) == 0)
	{
		if (message->get_major_version(message) == IKEV2_MAJOR_VERSION)
		{
			if (message->get_exchange_type(message) == IKE_SA_INIT &&
				message->get_request(message))
			{
				ike_version = IKEV2;
				is_init = TRUE;
			}
		}
		else
		{
			if (message->get_exchange_type(message) == ID_PROT ||
				message->get_exchange_type(message) == AGGRESSIVE)
			{
				ike_version = IKEV1;
				is_init = TRUE;
				if (id->is_initiator(id))
				{	/* not set in IKEv1, switch back before applying to new SA */
					id->switch_initiator(id);
				}
			}
		}
	}

	if (is_init)
	{
		hasher_t *hasher;
		uint64_t our_spi;
		chunk_t hash;

		untrack_half_open = TRUE;
		hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
		if (!hasher || !get_init_hash(hasher, message, &hash))
		{
			DBG1(DBG_MGR, "ignoring message, failed to hash message");
			DESTROY_IF(hasher);
			id->destroy(id);
			goto out;
		}
		hasher->destroy(hasher);

		/* ensure this is not a retransmit of an already handled init message */
		switch (check_and_put_init_hash(this, hash, &our_spi))
		{
			case NOT_FOUND:
			{	/* we've not seen this packet yet, create a new IKE_SA */
				if (!this->ikesa_limit ||
					this->public.get_count(&this->public) < this->ikesa_limit)
				{
					id->set_responder_spi(id, our_spi);
					ike_sa = ike_sa_create(id, FALSE, ike_version);
					if (ike_sa)
					{
						entry = entry_create();
						entry->ike_sa = ike_sa;
						entry->ike_sa_id = id;
						entry->processing = get_message_id_or_hash(message);
						entry->init_hash = hash;
						entry->half_open = TRUE;
						entry->other = message->get_source(message);
						entry->other = entry->other->clone(entry->other);
						untrack_half_open = FALSE;

						segment = put_entry(this, entry);
						entry->checked_out = thread_current();
						unlock_single_segment(this, segment);

						DBG2(DBG_MGR, "created IKE_SA %s[%u]",
							 ike_sa->get_name(ike_sa),
							 ike_sa->get_unique_id(ike_sa));
						goto out;
					}
					else
					{
						DBG1(DBG_MGR, "creating IKE_SA failed, ignoring message");
					}
				}
				else
				{
					DBG1(DBG_MGR, "ignoring %N, hitting IKE_SA limit (%u)",
						 exchange_type_names, message->get_exchange_type(message),
						 this->ikesa_limit);
				}
				remove_init_hash(this, hash);
				chunk_free(&hash);
				id->destroy(id);
				goto out;
			}
			case FAILED:
			{	/* we failed to allocate an SPI */
				chunk_free(&hash);
				id->destroy(id);
				DBG1(DBG_MGR, "ignoring message, failed to allocate SPI");
				goto out;
			}
			case ALREADY_DONE:
			default:
				break;
		}
		/* it looks like we already handled this init message to some degree */
		id->set_responder_spi(id, our_spi);
		chunk_free(&hash);
		/* untrack the duplicate before waiting for the checkout */
		remove_half_open(this, message->get_source(message), FALSE);
		untrack_half_open = FALSE;
	}

	if (get_entry_by_id(this, id, &entry, &segment) == SUCCESS)
	{
		/* only check out if we are not already processing it. */
		if (entry->processing == get_message_id_or_hash(message))
		{
			DBG1(DBG_MGR, "ignoring request with ID %u, already processing",
				 entry->processing);
		}
		else if (wait_for_entry(this, entry, segment))
		{
			ike_sa_id_t *ike_id;

			ike_id = entry->ike_sa->get_id(entry->ike_sa);
			entry->checked_out = thread_current();
			if (message->get_first_payload_type(message) != PLV1_FRAGMENT &&
				message->get_first_payload_type(message) != PLV2_FRAGMENT)
			{	/* TODO-FRAG: this fails if there are unencrypted payloads */
				entry->processing = get_message_id_or_hash(message);
			}
			if (ike_id->get_responder_spi(ike_id) == 0)
			{
				ike_id->set_responder_spi(ike_id, id->get_responder_spi(id));
			}
			ike_sa = entry->ike_sa;
			DBG2(DBG_MGR, "IKE_SA %s[%u] successfully checked out",
					ike_sa->get_name(ike_sa), ike_sa->get_unique_id(ike_sa));
		}
		unlock_single_segment(this, segment);
	}
	else
	{
		charon->bus->alert(charon->bus, ALERT_INVALID_IKE_SPI, message);
	}
	id->destroy(id);

out:
	if (untrack_half_open)
	{
		remove_half_open(this, message->get_source(message), FALSE);
	}
	charon->bus->set_sa(charon->bus, ike_sa);
	if (!ike_sa)
	{
		DBG2(DBG_MGR, "IKE_SA checkout not successful");
	}
	return ike_sa;
}

/**
 * Data used to track checkouts by config.
 */
typedef struct {
	/** The peer config for which an IKE_SA is being checked out. */
	peer_cfg_t *cfg;
	/** Number of threads checking out SAs for the same config. */
	int threads;
	/** A thread is currently creating/finding an SA for this config. */
	bool working;
} config_entry_t;

METHOD(ike_sa_manager_t, checkout_by_config, ike_sa_t*,
	private_ike_sa_manager_t *this, peer_cfg_t *peer_cfg)
{
	enumerator_t *enumerator;
	entry_t *entry;
	ike_sa_t *ike_sa = NULL;
	peer_cfg_t *current_peer;
	ike_cfg_t *current_ike;
	config_entry_t *config_entry, *found = NULL;
	u_int segment;
	int i;

	DBG2(DBG_MGR, "checkout IKE_SA by config");

	if (!this->reuse_ikesa && peer_cfg->get_ike_version(peer_cfg) != IKEV1)
	{	/* IKE_SA reuse disabled by config (not possible for IKEv1) */
		ike_sa = create_new(this, peer_cfg->get_ike_version(peer_cfg), TRUE);
		if (ike_sa)
		{
			ike_sa->set_peer_cfg(ike_sa, peer_cfg);
			checkout_new(this, ike_sa);
		}
		charon->bus->set_sa(charon->bus, ike_sa);
		goto out;
	}

	this->config_mutex->lock(this->config_mutex);
	for (i = 0; i < array_count(this->config_checkouts); i++)
	{
		array_get(this->config_checkouts, i, &config_entry);
		if (config_entry->cfg->equals(config_entry->cfg, peer_cfg))
		{
			current_ike = config_entry->cfg->get_ike_cfg(config_entry->cfg);
			if (current_ike->equals(current_ike,
									peer_cfg->get_ike_cfg(peer_cfg)))
			{
				found = config_entry;
				break;
			}
		}
	}
	if (!found)
	{
		INIT(found,
			.cfg = peer_cfg->get_ref(peer_cfg),
		);
		array_insert_create(&this->config_checkouts, ARRAY_TAIL, found);
	}
	found->threads++;
	while (found->working)
	{
		this->config_condvar->wait(this->config_condvar, this->config_mutex);
	}
	found->working = TRUE;
	this->config_mutex->unlock(this->config_mutex);

	enumerator = create_table_enumerator(this);
	while (enumerator->enumerate(enumerator, &entry, &segment))
	{
		if (!wait_for_entry(this, entry, segment))
		{
			continue;
		}
		if (entry->ike_sa->get_state(entry->ike_sa) == IKE_DELETING ||
			entry->ike_sa->get_state(entry->ike_sa) == IKE_REKEYED)
		{	/* skip IKE_SAs which are not usable, wake other waiting threads */
			entry->condvar->signal(entry->condvar);
			continue;
		}

		current_peer = entry->ike_sa->get_peer_cfg(entry->ike_sa);
		if (current_peer && current_peer->equals(current_peer, peer_cfg))
		{
			current_ike = current_peer->get_ike_cfg(current_peer);
			if (current_ike->equals(current_ike, peer_cfg->get_ike_cfg(peer_cfg)))
			{
				entry->checked_out = thread_current();
				ike_sa = entry->ike_sa;
				DBG2(DBG_MGR, "found existing IKE_SA %u with config '%s'",
						ike_sa->get_unique_id(ike_sa),
						current_peer->get_name(current_peer));
				break;
			}
		}
		/* other threads might be waiting for this entry */
		entry->condvar->signal(entry->condvar);
	}
	enumerator->destroy(enumerator);

	if (!ike_sa)
	{
		ike_sa = create_new(this, peer_cfg->get_ike_version(peer_cfg), TRUE);
		if (ike_sa)
		{
			ike_sa->set_peer_cfg(ike_sa, peer_cfg);
			checkout_new(this, ike_sa);
		}
	}
	charon->bus->set_sa(charon->bus, ike_sa);

	this->config_mutex->lock(this->config_mutex);
	found->working = FALSE;
	found->threads--;
	if (!found->threads)
	{
		for (i = 0; i < array_count(this->config_checkouts); i++)
		{
			array_get(this->config_checkouts, i, &config_entry);
			if (config_entry == found)
			{
				array_remove(this->config_checkouts, i, NULL);
				found->cfg->destroy(found->cfg);
				free(found);
				break;
			}
		}
	}
	this->config_condvar->signal(this->config_condvar);
	this->config_mutex->unlock(this->config_mutex);

out:
	if (!ike_sa)
	{
		DBG2(DBG_MGR, "IKE_SA checkout not successful");
	}
	return ike_sa;
}

METHOD(ike_sa_manager_t, checkout_by_id, ike_sa_t*,
	private_ike_sa_manager_t *this, uint32_t id)
{
	enumerator_t *enumerator;
	entry_t *entry;
	ike_sa_t *ike_sa = NULL;
	u_int segment;

	DBG2(DBG_MGR, "checkout IKE_SA by unique ID %u", id);

	enumerator = create_table_enumerator(this);
	while (enumerator->enumerate(enumerator, &entry, &segment))
	{
		if (wait_for_entry(this, entry, segment))
		{
			if (entry->ike_sa->get_unique_id(entry->ike_sa) == id)
			{
				ike_sa = entry->ike_sa;
				entry->checked_out = thread_current();
				break;
			}
			/* other threads might be waiting for this entry */
			entry->condvar->signal(entry->condvar);
		}
	}
	enumerator->destroy(enumerator);

	if (ike_sa)
	{
		DBG2(DBG_MGR, "IKE_SA %s[%u] successfully checked out",
			 ike_sa->get_name(ike_sa), ike_sa->get_unique_id(ike_sa));
	}
	else
	{
		DBG2(DBG_MGR, "IKE_SA checkout not successful");
	}
	charon->bus->set_sa(charon->bus, ike_sa);
	return ike_sa;
}

METHOD(ike_sa_manager_t, checkout_by_name, ike_sa_t*,
	private_ike_sa_manager_t *this, char *name, bool child)
{
	enumerator_t *enumerator, *children;
	entry_t *entry;
	ike_sa_t *ike_sa = NULL;
	child_sa_t *child_sa;
	u_int segment;

	DBG2(DBG_MGR, "checkout IKE_SA by%s name '%s'", child ? " child" : "", name);

	enumerator = create_table_enumerator(this);
	while (enumerator->enumerate(enumerator, &entry, &segment))
	{
		if (wait_for_entry(this, entry, segment))
		{
			/* look for a child with such a policy name ... */
			if (child)
			{
				children = entry->ike_sa->create_child_sa_enumerator(entry->ike_sa);
				while (children->enumerate(children, (void**)&child_sa))
				{
					if (streq(child_sa->get_name(child_sa), name))
					{
						ike_sa = entry->ike_sa;
						break;
					}
				}
				children->destroy(children);
			}
			else /* ... or for a IKE_SA with such a connection name */
			{
				if (streq(entry->ike_sa->get_name(entry->ike_sa), name))
				{
					ike_sa = entry->ike_sa;
				}
			}
			/* got one, return */
			if (ike_sa)
			{
				entry->checked_out = thread_current();
				DBG2(DBG_MGR, "IKE_SA %s[%u] successfully checked out",
						ike_sa->get_name(ike_sa), ike_sa->get_unique_id(ike_sa));
				break;
			}
			/* other threads might be waiting for this entry */
			entry->condvar->signal(entry->condvar);
		}
	}
	enumerator->destroy(enumerator);

	charon->bus->set_sa(charon->bus, ike_sa);

	if (!ike_sa)
	{
		DBG2(DBG_MGR, "IKE_SA checkout not successful");
	}
	return ike_sa;
}

METHOD(ike_sa_manager_t, new_initiator_spi, bool,
	private_ike_sa_manager_t *this, ike_sa_t *ike_sa)
{
	ike_sa_state_t state;
	ike_sa_id_t *ike_sa_id;
	entry_t *entry;
	u_int segment;
	uint64_t new_spi, spi;

	state = ike_sa->get_state(ike_sa);
	if (state != IKE_CONNECTING)
	{
		DBG1(DBG_MGR, "unable to change initiator SPI for IKE_SA in state "
			 "%N", ike_sa_state_names, state);
		return FALSE;
	}

	ike_sa_id = ike_sa->get_id(ike_sa);
	if (!ike_sa_id->is_initiator(ike_sa_id))
	{
		DBG1(DBG_MGR, "unable to change initiator SPI of IKE_SA as responder");
		return FALSE;
	}

	if (ike_sa != charon->bus->get_sa(charon->bus))
	{
		DBG1(DBG_MGR, "unable to change initiator SPI of IKE_SA not checked "
			 "out by current thread");
		return FALSE;
	}

	new_spi = get_spi(this);
	if (!new_spi)
	{
		DBG1(DBG_MGR, "unable to allocate new initiator SPI for IKE_SA");
		return FALSE;
	}

	if (get_entry_by_sa(this, ike_sa_id, ike_sa, &entry, &segment) == SUCCESS)
	{
		if (entry->driveout_threads)
		{	/* it looks like flush() has been called and the SA is being deleted
			 * anyway, no need for a new SPI */
			DBG2(DBG_MGR, "ignored change of initiator SPI during shutdown");
			unlock_single_segment(this, segment);
			return FALSE;
		}
	}
	else
	{
		DBG1(DBG_MGR, "unable to change initiator SPI of IKE_SA, not found");
		return FALSE;
	}

	/* the hashtable row and segment are determined by the local SPI as
	 * initiator, so if we change it the row and segment derived from it might
	 * change as well.  This could be a problem for threads waiting for the
	 * entry (in particular those enumerating entries to check them out by
	 * unique ID or name).  In order to avoid having to drive them out and thus
	 * preventing them from checking out the entry (even though the ID or name
	 * will not change and enumerating it is also fine), we mask the new SPI and
	 * merge it with the old SPI so the entry ends up in the same row/segment.
	 * Since SPIs are 64-bit and the number of rows/segments is usually
	 * relatively low this should not be a problem. */
	spi = ike_sa_id->get_initiator_spi(ike_sa_id);
	new_spi = (spi & (uint64_t)this->table_mask) |
			  (new_spi & ~(uint64_t)this->table_mask);

	DBG2(DBG_MGR, "change initiator SPI of IKE_SA %s[%u] from %.16"PRIx64" to "
		 "%.16"PRIx64, ike_sa->get_name(ike_sa), ike_sa->get_unique_id(ike_sa),
		 be64toh(spi), be64toh(new_spi));

	ike_sa_id->set_initiator_spi(ike_sa_id, new_spi);
	entry->ike_sa_id->replace_values(entry->ike_sa_id, ike_sa_id);

	entry->condvar->signal(entry->condvar);
	unlock_single_segment(this, segment);
	return TRUE;
}

CALLBACK(enumerator_filter_wait, bool,
	private_ike_sa_manager_t *this, enumerator_t *orig, va_list args)
{
	entry_t *entry;
	u_int segment;
	ike_sa_t **out;

	VA_ARGS_VGET(args, out);

	while (orig->enumerate(orig, &entry, &segment))
	{
		if (wait_for_entry(this, entry, segment))
		{
			*out = entry->ike_sa;
			charon->bus->set_sa(charon->bus, *out);
			return TRUE;
		}
	}
	return FALSE;
}

CALLBACK(enumerator_filter_skip, bool,
	private_ike_sa_manager_t *this, enumerator_t *orig, va_list args)
{
	entry_t *entry;
	u_int segment;
	ike_sa_t **out;

	VA_ARGS_VGET(args, out);

	while (orig->enumerate(orig, &entry, &segment))
	{
		if (!entry->driveout_threads &&
			!entry->checked_out)
		{
			*out = entry->ike_sa;
			charon->bus->set_sa(charon->bus, *out);
			return TRUE;
		}
	}
	return FALSE;
}

CALLBACK(reset_sa, void,
	void *data)
{
	charon->bus->set_sa(charon->bus, NULL);
}

METHOD(ike_sa_manager_t, create_enumerator, enumerator_t*,
	private_ike_sa_manager_t* this, bool wait)
{
	return enumerator_create_filter(create_table_enumerator(this),
			wait ? (void*)enumerator_filter_wait : (void*)enumerator_filter_skip,
			this, reset_sa);
}

METHOD(ike_sa_manager_t, checkin, void,
	private_ike_sa_manager_t *this, ike_sa_t *ike_sa)
{
	/* to check the SA back in, we look for the pointer of the ike_sa
	 * in all entries.
	 * The lookup is done by initiator SPI, so even if the SPI has changed (e.g.
	 * on reception of a IKE_SA_INIT response) the lookup will work but
	 * updating of the SPI MAY be necessary...
	 */
	entry_t *entry;
	ike_sa_id_t *ike_sa_id;
	host_t *other;
	identification_t *my_id, *other_id;
	u_int segment;

	ike_sa_id = ike_sa->get_id(ike_sa);
	my_id = ike_sa->get_my_id(ike_sa);
	other_id = ike_sa->get_other_eap_id(ike_sa);
	other = ike_sa->get_other_host(ike_sa);

	DBG2(DBG_MGR, "checkin %N SA %s[%u] with SPIs %.16"PRIx64"_i "
		 "%.16"PRIx64"_r", ike_version_names,
		 ike_sa_id->get_ike_version(ike_sa_id), ike_sa->get_name(ike_sa),
		 ike_sa->get_unique_id(ike_sa),
		 be64toh(ike_sa_id->get_initiator_spi(ike_sa_id)),
		 be64toh(ike_sa_id->get_responder_spi(ike_sa_id)));

	/* look for the entry */
	if (get_entry_by_sa(this, ike_sa_id, ike_sa, &entry, &segment) == SUCCESS)
	{
		/* ike_sa_id must be updated */
		entry->ike_sa_id->replace_values(entry->ike_sa_id, ike_sa->get_id(ike_sa));
		/* signal waiting threads */
		entry->checked_out = NULL;
		entry->processing = -1;
		/* check if this SA is half-open */
		if (entry->half_open && ike_sa->get_state(ike_sa) != IKE_CONNECTING)
		{
			/* not half open anymore */
			entry->half_open = FALSE;
			remove_half_open(this, entry->other,
							 entry->ike_sa_id->is_initiator(entry->ike_sa_id));
		}
		else if (entry->half_open && !other->ip_equals(other, entry->other))
		{
			/* the other host's IP has changed, we must update the hash table */
			remove_half_open(this, entry->other,
							 entry->ike_sa_id->is_initiator(entry->ike_sa_id));
			DESTROY_IF(entry->other);
			entry->other = other->clone(other);
			put_half_open(this, entry->other,
						  entry->ike_sa_id->is_initiator(entry->ike_sa_id));
		}
		else if (!entry->half_open &&
				 ike_sa->get_state(ike_sa) == IKE_CONNECTING)
		{
			/* this is a new half-open SA */
			entry->half_open = TRUE;
			entry->other = other->clone(other);
			put_half_open(this, entry->other,
						  entry->ike_sa_id->is_initiator(entry->ike_sa_id));
		}
		entry->condvar->signal(entry->condvar);
	}
	else
	{
		segment = create_and_put_entry(this, ike_sa, &entry);
	}
	DBG2(DBG_MGR, "checkin of IKE_SA successful");

	/* apply identities for duplicate test */
	if ((ike_sa->get_state(ike_sa) == IKE_ESTABLISHED ||
		 ike_sa->get_state(ike_sa) == IKE_PASSIVE) &&
		entry->my_id == NULL && entry->other_id == NULL)
	{
		if (ike_sa->get_version(ike_sa) == IKEV1)
		{
			/* If authenticated and received INITIAL_CONTACT,
			 * delete any existing IKE_SAs with that peer. */
			if (ike_sa->has_condition(ike_sa, COND_INIT_CONTACT_SEEN))
			{
				/* We can't hold the segment locked while checking the
				 * uniqueness as this could lead to deadlocks.  We mark the
				 * entry as checked out while we release the lock so no other
				 * thread can acquire it.  Since it is not yet in the list of
				 * connected peers that will not cause a deadlock as no other
				 * caller of check_uniqueness() will try to check out this SA */
				entry->checked_out = thread_current();
				unlock_single_segment(this, segment);

				this->public.check_uniqueness(&this->public, ike_sa, TRUE);
				ike_sa->set_condition(ike_sa, COND_INIT_CONTACT_SEEN, FALSE);

				/* The entry could have been modified in the mean time, e.g.
				 * because another SA was added/removed next to it or another
				 * thread is waiting, but it should still exist, so there is no
				 * need for a lookup via get_entry_by... */
				lock_single_segment(this, segment);
				entry->checked_out = NULL;
				/* We already signaled waiting threads above, we have to do that
				 * again after checking the SA out and back in again. */
				entry->condvar->signal(entry->condvar);
			}
		}

		entry->my_id = my_id->clone(my_id);
		entry->other_id = other_id->clone(other_id);
		if (!entry->other)
		{
			entry->other = other->clone(other);
		}
		put_connected_peers(this, entry);
	}

	unlock_single_segment(this, segment);

	charon->bus->set_sa(charon->bus, NULL);
}

METHOD(ike_sa_manager_t, checkin_and_destroy, void,
	private_ike_sa_manager_t *this, ike_sa_t *ike_sa)
{
	/* deletion is a bit complex, we must ensure that no thread is waiting for
	 * this SA.
	 * We take this SA from the table, and start signaling while threads
	 * are in the condvar.
	 */
	entry_t *entry;
	ike_sa_id_t *ike_sa_id;
	u_int segment;

	ike_sa_id = ike_sa->get_id(ike_sa);

	DBG2(DBG_MGR, "checkin and destroy IKE_SA %s[%u]", ike_sa->get_name(ike_sa),
			ike_sa->get_unique_id(ike_sa));

	if (get_entry_by_sa(this, ike_sa_id, ike_sa, &entry, &segment) == SUCCESS)
	{
		if (entry->driveout_threads)
		{	/* it looks like flush() has been called and the SA is being deleted
			 * anyway, just check it in */
			DBG2(DBG_MGR, "ignored checkin and destroy of IKE_SA during shutdown");
			entry->checked_out = NULL;
			entry->condvar->broadcast(entry->condvar);
			unlock_single_segment(this, segment);
			return;
		}

		/* mark it, so no threads can get this entry */
		entry->driveout_threads = TRUE;
		/* wait until all workers have done their work */
		while (entry->waiting_threads)
		{
			/* wake up all */
			entry->condvar->broadcast(entry->condvar);
			/* they will wake us again when their work is done */
			entry->condvar->wait(entry->condvar, this->segments[segment].mutex);
		}
		remove_entry(this, entry);
		unlock_single_segment(this, segment);

		if (entry->half_open)
		{
			remove_half_open(this, entry->other,
							 entry->ike_sa_id->is_initiator(entry->ike_sa_id));
		}
		if (entry->my_id && entry->other_id)
		{
			remove_connected_peers(this, entry);
		}
		if (entry->init_hash.ptr)
		{
			remove_init_hash(this, entry->init_hash);
		}

		entry_destroy(entry);

		DBG2(DBG_MGR, "checkin and destroy of IKE_SA successful");
	}
	else
	{
		DBG1(DBG_MGR, "tried to checkin and delete nonexistent IKE_SA");
		ike_sa->destroy(ike_sa);
	}
	charon->bus->set_sa(charon->bus, NULL);
}

/**
 * Cleanup function for create_id_enumerator
 */
static void id_enumerator_cleanup(linked_list_t *ids)
{
	ids->destroy_offset(ids, offsetof(ike_sa_id_t, destroy));
}

METHOD(ike_sa_manager_t, create_id_enumerator, enumerator_t*,
	private_ike_sa_manager_t *this, identification_t *me,
	identification_t *other, int family)
{
	table_item_t *item;
	u_int row, segment;
	rwlock_t *lock;
	linked_list_t *ids = NULL;

	row = chunk_hash_inc(other->get_encoding(other),
						 chunk_hash(me->get_encoding(me))) & this->table_mask;
	segment = row & this->segment_mask;

	lock = this->connected_peers_segments[segment].lock;
	lock->read_lock(lock);
	item = this->connected_peers_table[row];
	while (item)
	{
		connected_peers_t *current = item->value;

		if (connected_peers_match(current, me, other, family))
		{
			ids = current->sas->clone_offset(current->sas,
											 offsetof(ike_sa_id_t, clone));
			break;
		}
		item = item->next;
	}
	lock->unlock(lock);

	if (!ids)
	{
		return enumerator_create_empty();
	}
	return enumerator_create_cleaner(ids->create_enumerator(ids),
									 (void*)id_enumerator_cleanup, ids);
}

/**
 * Move all CHILD_SAs and virtual IPs from old to new
 */
static void adopt_children_and_vips(ike_sa_t *old, ike_sa_t *new)
{
	enumerator_t *enumerator;
	child_sa_t *child_sa;
	host_t *vip;
	int chcount = 0, vipcount = 0;

	charon->bus->children_migrate(charon->bus, new->get_id(new),
								  new->get_unique_id(new));
	enumerator = old->create_child_sa_enumerator(old);
	while (enumerator->enumerate(enumerator, &child_sa))
	{
		old->remove_child_sa(old, enumerator);
		new->add_child_sa(new, child_sa);
		chcount++;
	}
	enumerator->destroy(enumerator);

	new->adopt_child_tasks(new, old);

	enumerator = old->create_virtual_ip_enumerator(old, FALSE);
	while (enumerator->enumerate(enumerator, &vip))
	{
		new->add_virtual_ip(new, FALSE, vip);
		vipcount++;
	}
	enumerator->destroy(enumerator);
	/* this does not release the addresses, which is good, but it does trigger
	 * an assign_vips(FALSE) event... */
	old->clear_virtual_ips(old, FALSE);
	/* ...trigger the analogous event on the new SA */
	charon->bus->set_sa(charon->bus, new);
	charon->bus->assign_vips(charon->bus, new, TRUE);
	charon->bus->children_migrate(charon->bus, NULL, 0);
	charon->bus->set_sa(charon->bus, old);

	if (chcount || vipcount)
	{
		DBG1(DBG_IKE, "detected reauth of existing IKE_SA, adopting %d "
			 "children and %d virtual IPs", chcount, vipcount);
	}
}

/**
 * Delete an existing IKE_SA due to a unique replace policy
 */
static status_t enforce_replace(private_ike_sa_manager_t *this,
								ike_sa_t *duplicate, ike_sa_t *new,
								identification_t *other, host_t *host)
{
	charon->bus->alert(charon->bus, ALERT_UNIQUE_REPLACE);

	if (host->equals(host, duplicate->get_other_host(duplicate)))
	{
		/* looks like a reauthentication attempt */
		if (!new->has_condition(new, COND_INIT_CONTACT_SEEN) &&
			new->get_version(new) == IKEV1)
		{
			/* IKEv1 implicitly takes over children, IKEv2 recreates them
			 * explicitly. */
			adopt_children_and_vips(duplicate, new);
		}
		/* For IKEv1 we have to delay the delete for the old IKE_SA. Some
		 * peers need to complete the new SA first, otherwise the quick modes
		 * might get lost. For IKEv2 we do the same, as we want overlapping
		 * CHILD_SAs to keep connectivity up. */
		lib->scheduler->schedule_job(lib->scheduler, (job_t*)
			delete_ike_sa_job_create(duplicate->get_id(duplicate), TRUE), 10);
		DBG1(DBG_IKE, "schedule delete of duplicate IKE_SA for peer '%Y' due "
			 "to uniqueness policy and suspected reauthentication", other);
		return SUCCESS;
	}
	DBG1(DBG_IKE, "deleting duplicate IKE_SA for peer '%Y' due to "
		 "uniqueness policy", other);
	return duplicate->delete(duplicate, FALSE);
}

METHOD(ike_sa_manager_t, check_uniqueness, bool,
	private_ike_sa_manager_t *this, ike_sa_t *ike_sa, bool force_replace)
{
	bool cancel = FALSE;
	peer_cfg_t *peer_cfg;
	unique_policy_t policy;
	enumerator_t *enumerator;
	ike_sa_id_t *id = NULL;
	identification_t *me, *other;
	host_t *other_host;

	peer_cfg = ike_sa->get_peer_cfg(ike_sa);
	policy = peer_cfg->get_unique_policy(peer_cfg);
	if (policy == UNIQUE_NEVER || (policy == UNIQUE_NO && !force_replace))
	{
		return FALSE;
	}
	me = ike_sa->get_my_id(ike_sa);
	other = ike_sa->get_other_eap_id(ike_sa);
	other_host = ike_sa->get_other_host(ike_sa);

	enumerator = create_id_enumerator(this, me, other,
									  other_host->get_family(other_host));
	while (enumerator->enumerate(enumerator, &id))
	{
		status_t status = SUCCESS;
		ike_sa_t *duplicate;

		duplicate = checkout(this, id);
		if (!duplicate)
		{
			continue;
		}
		if (force_replace)
		{
			DBG1(DBG_IKE, "destroying duplicate IKE_SA for peer '%Y', "
				 "received INITIAL_CONTACT", other);
			charon->bus->ike_updown(charon->bus, duplicate, FALSE);
			checkin_and_destroy(this, duplicate);
			continue;
		}
		peer_cfg = duplicate->get_peer_cfg(duplicate);
		if (peer_cfg && peer_cfg->equals(peer_cfg, ike_sa->get_peer_cfg(ike_sa)))
		{
			switch (duplicate->get_state(duplicate))
			{
				case IKE_ESTABLISHED:
				case IKE_REKEYING:
					switch (policy)
					{
						case UNIQUE_REPLACE:
							status = enforce_replace(this, duplicate, ike_sa,
													 other, other_host);
							break;
						case UNIQUE_KEEP:
							/* potential reauthentication? */
							if (!other_host->equals(other_host,
										duplicate->get_other_host(duplicate)))
							{
								cancel = TRUE;
								/* we keep the first IKE_SA and delete all
								 * other duplicates that might exist */
								policy = UNIQUE_REPLACE;
							}
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
		}
		if (status == DESTROY_ME)
		{
			checkin_and_destroy(this, duplicate);
		}
		else
		{
			checkin(this, duplicate);
		}
	}
	enumerator->destroy(enumerator);
	/* reset thread's current IKE_SA after checkin */
	charon->bus->set_sa(charon->bus, ike_sa);
	return cancel;
}

METHOD(ike_sa_manager_t, has_contact, bool,
	private_ike_sa_manager_t *this, identification_t *me,
	identification_t *other, int family)
{
	table_item_t *item;
	u_int row, segment;
	rwlock_t *lock;
	bool found = FALSE;

	row = chunk_hash_inc(other->get_encoding(other),
						 chunk_hash(me->get_encoding(me))) & this->table_mask;
	segment = row & this->segment_mask;
	lock = this->connected_peers_segments[segment].lock;
	lock->read_lock(lock);
	item = this->connected_peers_table[row];
	while (item)
	{
		if (connected_peers_match(item->value, me, other, family))
		{
			found = TRUE;
			break;
		}
		item = item->next;
	}
	lock->unlock(lock);

	return found;
}

METHOD(ike_sa_manager_t, get_count, u_int,
	private_ike_sa_manager_t *this)
{
	return (u_int)ref_cur(&this->total_sa_count);
}

METHOD(ike_sa_manager_t, get_half_open_count, u_int,
	private_ike_sa_manager_t *this, host_t *ip, bool responder_only)
{
	table_item_t *item;
	u_int row, segment;
	rwlock_t *lock;
	chunk_t addr;
	u_int count = 0;

	if (ip)
	{
		addr = ip->get_address(ip);
		row = chunk_hash(addr) & this->table_mask;
		segment = row & this->segment_mask;
		lock = this->half_open_segments[segment].lock;
		lock->read_lock(lock);
		item = this->half_open_table[row];
		while (item)
		{
			half_open_t *half_open = item->value;

			if (chunk_equals(addr, half_open->other))
			{
				count = responder_only ? half_open->count_responder
									   : half_open->count;
				break;
			}
			item = item->next;
		}
		lock->unlock(lock);
	}
	else
	{
		count = responder_only ? (u_int)ref_cur(&this->half_open_count_responder)
							   : (u_int)ref_cur(&this->half_open_count);
	}
	return count;
}

METHOD(ike_sa_manager_t, track_init, void,
	private_ike_sa_manager_t *this, host_t *ip)
{
	put_half_open(this, ip, FALSE);
}

METHOD(ike_sa_manager_t, set_spi_cb, void,
	private_ike_sa_manager_t *this, spi_cb_t callback, void *data)
{
	this->spi_lock->write_lock(this->spi_lock);
	this->spi_cb.cb = callback;
	this->spi_cb.data = data;
	this->spi_lock->unlock(this->spi_lock);
}

/**
 * Destroy a single entry
 */
static void remove_and_destroy_entry(private_ike_sa_manager_t *this,
									 enumerator_t *enumerator, entry_t *entry)
{
	if (entry->half_open)
	{
		remove_half_open(this, entry->other,
						 entry->ike_sa_id->is_initiator(entry->ike_sa_id));
	}
	if (entry->my_id && entry->other_id)
	{
		remove_connected_peers(this, entry);
	}
	if (entry->init_hash.ptr)
	{
		remove_init_hash(this, entry->init_hash);
	}
	remove_entry_at((private_enumerator_t*)enumerator);
	entry_destroy(entry);
}

/**
 * Destroy all entries
 */
static void destroy_all_entries(private_ike_sa_manager_t *this)
{
	enumerator_t *enumerator;
	entry_t *entry;
	u_int segment;

	enumerator = create_table_enumerator(this);
	while (enumerator->enumerate(enumerator, &entry, &segment))
	{
		charon->bus->set_sa(charon->bus, entry->ike_sa);
		remove_and_destroy_entry(this, enumerator, entry);
	}
	enumerator->destroy(enumerator);
	charon->bus->set_sa(charon->bus, NULL);
}

METHOD(ike_sa_manager_t, flush, void,
	private_ike_sa_manager_t *this)
{
	enumerator_t *enumerator;
	entry_t *entry;
	u_int segment;

	/* prevent threads from creating new SAs */
	this->spi_lock->write_lock(this->spi_lock);
	DESTROY_IF(this->rng);
	this->rng = NULL;
	this->spi_cb.cb = NULL;
	this->spi_cb.data = NULL;
	this->spi_lock->unlock(this->spi_lock);

	lock_all_segments(this);
	DBG2(DBG_MGR, "going to destroy IKE_SA manager and all managed IKE_SAs");
	enumerator = create_table_enumerator(this);
	while (enumerator->enumerate(enumerator, &entry, &segment))
	{
		/* mark all entries so threads can't check these IKE_SAs out */
		entry->driveout_threads = TRUE;
	}
	enumerator->destroy(enumerator);
	DBG2(DBG_MGR, "wait for threads to leave IKE_SAs and delete and destroy "
		 "them");
	/* we are intermittently unlocking the segments below, which allows threads
	 * to create new entries. we want these to get marked directly so other
	 * threads can't get them again. we re-enumerate the table until all
	 * entries are gone */
	this->new_entries_driveout_threads = TRUE;
	while (ref_cur(&this->total_sa_count))
	{
		enumerator = create_table_enumerator(this);
		while (enumerator->enumerate(enumerator, &entry, &segment))
		{
			while (entry->waiting_threads || entry->checked_out)
			{
				/* wake up all */
				entry->condvar->broadcast(entry->condvar);
				/* go sleeping until they are gone */
				entry->condvar->wait(entry->condvar,
									 this->segments[segment].mutex);
			}
			charon->bus->set_sa(charon->bus, entry->ike_sa);
			entry->ike_sa->delete(entry->ike_sa, TRUE);
			remove_and_destroy_entry(this, enumerator, entry);
			charon->bus->set_sa(charon->bus, NULL);
		}
		enumerator->destroy(enumerator);
	}
	unlock_all_segments(this);
}

METHOD(ike_sa_manager_t, destroy, void,
	private_ike_sa_manager_t *this)
{
	u_int i;

	/* in case new SAs were checked in after flush() was called */
	lock_all_segments(this);
	destroy_all_entries(this);
	unlock_all_segments(this);

	free(this->ike_sa_table);
	free(this->half_open_table);
	free(this->connected_peers_table);
	free(this->init_hashes_table);
	for (i = 0; i < this->segment_count; i++)
	{
		this->segments[i].mutex->destroy(this->segments[i].mutex);
		this->half_open_segments[i].lock->destroy(this->half_open_segments[i].lock);
		this->connected_peers_segments[i].lock->destroy(this->connected_peers_segments[i].lock);
		this->init_hashes_segments[i].mutex->destroy(this->init_hashes_segments[i].mutex);
	}
	free(this->segments);
	free(this->half_open_segments);
	free(this->connected_peers_segments);
	free(this->init_hashes_segments);

	array_destroy(this->config_checkouts);
	this->config_mutex->destroy(this->config_mutex);
	this->config_condvar->destroy(this->config_condvar);

	this->spi_lock->destroy(this->spi_lock);
	free(this);
}

/**
 * This function returns the next-highest power of two for the given number.
 * The algorithm works by setting all bits on the right-hand side of the most
 * significant 1 to 1 and then increments the whole number so it rolls over
 * to the nearest power of two. Note: returns 0 for n == 0
 */
static u_int get_nearest_powerof2(u_int n)
{
	u_int i;

	--n;
	for (i = 1; i < sizeof(u_int) * 8; i <<= 1)
	{
		n |= n >> i;
	}
	return ++n;
}

/*
 * Described in header.
 */
ike_sa_manager_t *ike_sa_manager_create()
{
	private_ike_sa_manager_t *this;
	char *spi_val;
	u_int i;

	INIT(this,
		.public = {
			.create_new = _create_new,
			.checkout_new = _checkout_new,
			.checkout = _checkout,
			.checkout_by_message = _checkout_by_message,
			.checkout_by_config = _checkout_by_config,
			.checkout_by_id = _checkout_by_id,
			.checkout_by_name = _checkout_by_name,
			.new_initiator_spi = _new_initiator_spi,
			.check_uniqueness = _check_uniqueness,
			.has_contact = _has_contact,
			.create_enumerator = _create_enumerator,
			.create_id_enumerator = _create_id_enumerator,
			.checkin = _checkin,
			.checkin_and_destroy = _checkin_and_destroy,
			.get_count = _get_count,
			.get_half_open_count = _get_half_open_count,
			.track_init = _track_init,
			.flush = _flush,
			.set_spi_cb = _set_spi_cb,
			.destroy = _destroy,
		},
	);

	this->rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (this->rng == NULL)
	{
		DBG1(DBG_MGR, "manager initialization failed, no RNG supported");
		free(this);
		return NULL;
	}
	this->spi_lock = rwlock_create(RWLOCK_TYPE_DEFAULT);
	spi_val = lib->settings->get_str(lib->settings, "%s.spi_mask", NULL,
									 lib->ns);
	this->spi_mask = settings_value_as_uint64(spi_val, 0);
	spi_val = lib->settings->get_str(lib->settings, "%s.spi_label", NULL,
									 lib->ns);
	this->spi_label = settings_value_as_uint64(spi_val, 0);
	if (this->spi_mask || this->spi_label)
	{
		DBG1(DBG_IKE, "using SPI label 0x%.16"PRIx64" and mask 0x%.16"PRIx64,
			 this->spi_label, this->spi_mask);
		/* the allocated SPI is assumed to be in network order */
		this->spi_mask = htobe64(this->spi_mask);
		this->spi_label = htobe64(this->spi_label);
	}

	this->ikesa_limit = lib->settings->get_int(lib->settings,
											   "%s.ikesa_limit", 0, lib->ns);

	this->table_size = get_nearest_powerof2(lib->settings->get_int(
									lib->settings, "%s.ikesa_table_size",
									DEFAULT_HASHTABLE_SIZE, lib->ns));
	this->table_size = max(1, min(this->table_size, MAX_HASHTABLE_SIZE));
	this->table_mask = this->table_size - 1;

	this->segment_count = get_nearest_powerof2(lib->settings->get_int(
									lib->settings, "%s.ikesa_table_segments",
									DEFAULT_SEGMENT_COUNT, lib->ns));
	this->segment_count = max(1, min(this->segment_count, this->table_size));
	this->segment_mask = this->segment_count - 1;

	this->ike_sa_table = calloc(this->table_size, sizeof(table_item_t*));
	this->segments = (segment_t*)calloc(this->segment_count, sizeof(segment_t));
	for (i = 0; i < this->segment_count; i++)
	{
		this->segments[i].mutex = mutex_create(MUTEX_TYPE_RECURSIVE);
	}

	/* we use the same table parameters for the table to track half-open SAs */
	this->half_open_table = calloc(this->table_size, sizeof(table_item_t*));
	this->half_open_segments = calloc(this->segment_count, sizeof(shareable_segment_t));
	for (i = 0; i < this->segment_count; i++)
	{
		this->half_open_segments[i].lock = rwlock_create(RWLOCK_TYPE_DEFAULT);
	}

	/* also for the hash table used for duplicate tests */
	this->connected_peers_table = calloc(this->table_size, sizeof(table_item_t*));
	this->connected_peers_segments = calloc(this->segment_count, sizeof(shareable_segment_t));
	for (i = 0; i < this->segment_count; i++)
	{
		this->connected_peers_segments[i].lock = rwlock_create(RWLOCK_TYPE_DEFAULT);
	}

	/* and again for the table of hashes of seen initial IKE messages */
	this->init_hashes_table = calloc(this->table_size, sizeof(table_item_t*));
	this->init_hashes_segments = calloc(this->segment_count, sizeof(segment_t));
	for (i = 0; i < this->segment_count; i++)
	{
		this->init_hashes_segments[i].mutex = mutex_create(MUTEX_TYPE_RECURSIVE);
	}

	this->config_mutex = mutex_create(MUTEX_TYPE_DEFAULT);
	this->config_condvar = condvar_create(CONDVAR_TYPE_DEFAULT);

	this->reuse_ikesa = lib->settings->get_bool(lib->settings,
											"%s.reuse_ikesa", TRUE, lib->ns);
	return &this->public;
}
