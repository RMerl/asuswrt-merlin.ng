/*
 * Copyright (C) 2015-2018 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "eap_radius_accounting.h"
#include "eap_radius_provider.h"
#include "eap_radius_plugin.h"

#include <time.h>

#include <radius_message.h>
#include <radius_client.h>
#include <daemon.h>
#include <collections/array.h>
#include <collections/hashtable.h>
#include <threading/mutex.h>
#include <processing/jobs/callback_job.h>

typedef struct private_eap_radius_accounting_t private_eap_radius_accounting_t;

/**
 * Private data of an eap_radius_accounting_t object.
 */
struct private_eap_radius_accounting_t {

	/**
	 * Public eap_radius_accounting_t interface.
	 */
	eap_radius_accounting_t public;

	/**
	 * Hashtable with sessions, ike_sa_id_t => entry_t
	 */
	hashtable_t *sessions;

	/**
	 * Mutex to lock sessions
	 */
	mutex_t *mutex;

	/**
	 * Session ID prefix
	 */
	uint32_t prefix;

	/**
	 * Format string we use for Called/Calling-Station-Id for a host
	 */
	char *station_id_fmt;

	/**
	 * Disable accounting unless IKE_SA has at least one virtual IP
	 */
	bool acct_req_vip;
};

/**
 * Singleton instance of accounting
 */
static private_eap_radius_accounting_t *singleton = NULL;

/**
 * Acct-Terminate-Cause
 */
typedef enum {
	ACCT_CAUSE_USER_REQUEST = 1,
	ACCT_CAUSE_LOST_CARRIER = 2,
	ACCT_CAUSE_LOST_SERVICE = 3,
	ACCT_CAUSE_IDLE_TIMEOUT = 4,
	ACCT_CAUSE_SESSION_TIMEOUT = 5,
	ACCT_CAUSE_ADMIN_RESET = 6,
	ACCT_CAUSE_ADMIN_REBOOT = 7,
	ACCT_CAUSE_PORT_ERROR = 8,
	ACCT_CAUSE_NAS_ERROR = 9,
	ACCT_CAUSE_NAS_REQUEST = 10,
	ACCT_CAUSE_NAS_REBOOT = 11,
	ACCT_CAUSE_PORT_UNNEEDED = 12,
	ACCT_CAUSE_PORT_PREEMPTED = 13,
	ACCT_CAUSE_PORT_SUSPENDED = 14,
	ACCT_CAUSE_SERVICE_UNAVAILABLE = 15,
	ACCT_CAUSE_CALLBACK = 16,
	ACCT_CAUSE_USER_ERROR = 17,
	ACCT_CAUSE_HOST_REQUEST = 18,
} radius_acct_terminate_cause_t;

/**
 * Usage stats for bytes and packets
 */
typedef struct {
	struct {
		uint64_t sent;
		uint64_t received;
	} bytes, packets;
} usage_t;

/**
 * Add usage stats (modifies a)
 */
static inline void add_usage(usage_t *a, usage_t b)
{
	a->bytes.sent += b.bytes.sent;
	a->bytes.received += b.bytes.received;
	a->packets.sent += b.packets.sent;
	a->packets.received += b.packets.received;
}

/**
 * Subtract usage stats (modifies a)
 */
static inline void sub_usage(usage_t *a, usage_t b)
{
	a->bytes.sent -= b.bytes.sent;
	a->bytes.received -= b.bytes.received;
	a->packets.sent -= b.packets.sent;
	a->packets.received -= b.packets.received;
}

/**
 * Usage stats for a cached/migrated SAs
 */
typedef struct {
	/** unique CHILD_SA identifier */
	uint32_t id;
	/** usage stats for this SA */
	usage_t usage;
} sa_entry_t;

/**
 * Clone an sa_entry_t
 */
static sa_entry_t *clone_sa(sa_entry_t *sa)
{
	sa_entry_t *this;

	INIT(this,
		.id = sa->id,
		.usage = sa->usage,
	);
	return this;
}

/**
 * Hashtable entry with usage stats
 */
typedef struct {
	/** IKE_SA identifier this entry is stored under */
	ike_sa_id_t *id;
	/** RADIUS accounting session ID */
	char sid[24];
	/** cached Class attributes */
	array_t *class_attrs;
	/** number of sent/received octets/packets for expired SAs */
	usage_t usage;
	/** list of cached SAs, sa_entry_t (sorted by their unique ID) */
	array_t *cached;
	/** list of migrated SAs, sa_entry_t (sorted by their unique ID) */
	array_t *migrated;
	/** session creation time */
	time_t created;
	/** terminate cause */
	radius_acct_terminate_cause_t cause;
	/* interim interval and timestamp of last update */
	struct {
		uint32_t interval;
		time_t last;
	} interim;
	/** did we send Accounting-Start */
	bool start_sent;
} entry_t;

/**
 * Destroy an entry_t
 */
static void destroy_entry(entry_t *this)
{
	array_destroy_function(this->cached, (void*)free, NULL);
	array_destroy_function(this->migrated, (void*)free, NULL);
	array_destroy_function(this->class_attrs, (void*)chunk_free, NULL);
	this->id->destroy(this->id);
	free(this);
}

/**
 * Accounting message status types
 */
typedef enum {
	ACCT_STATUS_START = 1,
	ACCT_STATUS_STOP = 2,
	ACCT_STATUS_INTERIM_UPDATE = 3,
	ACCT_STATUS_ACCOUNTING_ON = 7,
	ACCT_STATUS_ACCOUNTING_OFF = 8,
} radius_acct_status_t;

/**
 * Hashtable hash function
 */
static u_int hash(ike_sa_id_t *key)
{
	return key->get_responder_spi(key);
}

/**
 * Hashtable equals function
 */
static bool equals(ike_sa_id_t *a, ike_sa_id_t *b)
{
	return a->equals(a, b);
}

/**
 * Sort cached SAs
 */
static int sa_sort(const void *a, const void *b, void *user)
{
	const sa_entry_t *ra = a, *rb = b;
	return ra->id - rb->id;
}

/**
 * Find a cached SA
 */
static int sa_find(const void *a, const void *b)
{
	return sa_sort(a, b, NULL);
}

/**
 * Update or create usage counters of a cached SA
 */
static void update_sa(entry_t *entry, uint32_t id, usage_t usage)
{
	sa_entry_t *sa, lookup;

	lookup.id = id;
	if (array_bsearch(entry->cached, &lookup, sa_find, &sa) == -1)
	{
		INIT(sa,
			.id = id,
		);
		array_insert_create(&entry->cached, ARRAY_TAIL, sa);
		array_sort(entry->cached, sa_sort, NULL);
	}
	sa->usage = usage;
}

/**
 * Update usage counter when a CHILD_SA rekeys/goes down
 */
static void update_usage(private_eap_radius_accounting_t *this,
						 ike_sa_t *ike_sa, child_sa_t *child_sa)
{
	usage_t usage;
	entry_t *entry;

	child_sa->get_usestats(child_sa, TRUE, NULL, &usage.bytes.received,
						   &usage.packets.received);
	child_sa->get_usestats(child_sa, FALSE, NULL, &usage.bytes.sent,
						   &usage.packets.sent);

	this->mutex->lock(this->mutex);
	entry = this->sessions->get(this->sessions, ike_sa->get_id(ike_sa));
	if (entry)
	{
		update_sa(entry, child_sa->get_unique_id(child_sa), usage);
	}
	this->mutex->unlock(this->mutex);
}

/**
 * Collect usage stats for all CHILD_SAs of the given IKE_SA, optionally returns
 * the total number of bytes and packets
 */
static array_t *collect_stats(ike_sa_t *ike_sa, usage_t *total)
{
	enumerator_t *enumerator;
	child_sa_t *child_sa;
	array_t *stats;
	sa_entry_t *sa;
	usage_t usage;

	if (total)
	{
		*total = (usage_t){};
	}

	stats = array_create(0, 0);
	enumerator = ike_sa->create_child_sa_enumerator(ike_sa);
	while (enumerator->enumerate(enumerator, &child_sa))
	{
		INIT(sa,
			.id = child_sa->get_unique_id(child_sa),
		);
		array_insert(stats, ARRAY_TAIL, sa);
		array_sort(stats, sa_sort, NULL);

		child_sa->get_usestats(child_sa, TRUE, NULL, &usage.bytes.received,
							   &usage.packets.received);
		child_sa->get_usestats(child_sa, FALSE, NULL, &usage.bytes.sent,
							   &usage.packets.sent);
		sa->usage = usage;
		if (total)
		{
			add_usage(total, usage);
		}
	}
	enumerator->destroy(enumerator);
	return stats;
}

/**
 * Cleanup cached SAs
 */
static void cleanup_sas(private_eap_radius_accounting_t *this, ike_sa_t *ike_sa,
						entry_t *entry)
{
	enumerator_t *enumerator;
	child_sa_t *child_sa;
	sa_entry_t *sa, *found;
	array_t *sas;

	sas = array_create(0, 0);
	enumerator = ike_sa->create_child_sa_enumerator(ike_sa);
	while (enumerator->enumerate(enumerator, &child_sa))
	{
		INIT(sa,
			.id = child_sa->get_unique_id(child_sa),
		);
		array_insert(sas, ARRAY_TAIL, sa);
		array_sort(sas, sa_sort, NULL);
	}
	enumerator->destroy(enumerator);

	enumerator = array_create_enumerator(entry->cached);
	while (enumerator->enumerate(enumerator, &sa))
	{
		if (array_bsearch(sas, sa, sa_find, &found) == -1)
		{
			/* SA is gone, add its latest stats to the total for this IKE_SA
			 * and remove the cache entry */
			add_usage(&entry->usage, sa->usage);
			array_remove_at(entry->cached, enumerator);
			free(sa);
		}
	}
	enumerator->destroy(enumerator);
	enumerator = array_create_enumerator(entry->migrated);
	while (enumerator->enumerate(enumerator, &sa))
	{
		if (array_bsearch(sas, sa, sa_find, &found) == -1)
		{
			/* SA is gone, subtract stats from the total for this IKE_SA */
			sub_usage(&entry->usage, sa->usage);
			array_remove_at(entry->migrated, enumerator);
			free(sa);
		}
	}
	enumerator->destroy(enumerator);
	array_destroy_function(sas, (void*)free, NULL);
}

/**
 * Send a RADIUS message, wait for response
 */
static bool send_message(private_eap_radius_accounting_t *this,
						 radius_message_t *request)
{
	radius_message_t *response;
	radius_client_t *client;
	bool ack = FALSE;

	client = eap_radius_create_client();
	if (client)
	{
		response = client->request(client, request);
		if (response)
		{
			ack = response->get_code(response) == RMC_ACCOUNTING_RESPONSE;
			response->destroy(response);
		}
		client->destroy(client);
	}
	return ack;
}

/**
 * Add common IKE_SA parameters to RADIUS account message
 */
static void add_ike_sa_parameters(private_eap_radius_accounting_t *this,
								  radius_message_t *message, ike_sa_t *ike_sa)
{
	enumerator_t *enumerator;
	host_t *vip, *host;
	char buf[MAX_RADIUS_ATTRIBUTE_SIZE + 1];
	chunk_t data;
	uint32_t value;

	/* virtual NAS-Port-Type */
	value = htonl(5);
	message->add(message, RAT_NAS_PORT_TYPE, chunk_from_thing(value));
	/* framed ServiceType */
	value = htonl(2);
	message->add(message, RAT_SERVICE_TYPE, chunk_from_thing(value));

	value = htonl(ike_sa->get_unique_id(ike_sa));
	message->add(message, RAT_NAS_PORT, chunk_from_thing(value));
	message->add(message, RAT_NAS_PORT_ID,
				 chunk_from_str(ike_sa->get_name(ike_sa)));

	host = ike_sa->get_my_host(ike_sa);
	data = host->get_address(host);
	switch (host->get_family(host))
	{
		case AF_INET:
			message->add(message, RAT_NAS_IP_ADDRESS, data);
			break;
		case AF_INET6:
			message->add(message, RAT_NAS_IPV6_ADDRESS, data);
		default:
			break;
	}
	snprintf(buf, sizeof(buf), this->station_id_fmt, host);
	message->add(message, RAT_CALLED_STATION_ID, chunk_from_str(buf));
	host = ike_sa->get_other_host(ike_sa);
	snprintf(buf, sizeof(buf), this->station_id_fmt, host);
	message->add(message, RAT_CALLING_STATION_ID, chunk_from_str(buf));

	snprintf(buf, sizeof(buf), "%Y", ike_sa->get_other_eap_id(ike_sa));
	message->add(message, RAT_USER_NAME, chunk_from_str(buf));

	enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, FALSE);
	while (enumerator->enumerate(enumerator, &vip))
	{
		switch (vip->get_family(vip))
		{
			case AF_INET:
				message->add(message, RAT_FRAMED_IP_ADDRESS,
							 vip->get_address(vip));
				break;
			case AF_INET6:
				message->add(message, RAT_FRAMED_IPV6_ADDRESS,
							 vip->get_address(vip));
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Add any unclaimed IP addresses to the message
 */
static void add_unclaimed_ips(radius_message_t *message, ike_sa_t *ike_sa)
{
	eap_radius_provider_t *provider;
	enumerator_t *enumerator;
	host_t *vip;

	provider = eap_radius_provider_get();
	enumerator = provider->clear_unclaimed(provider,
										   ike_sa->get_unique_id(ike_sa));
	while (enumerator->enumerate(enumerator, &vip))
	{
		switch (vip->get_family(vip))
		{
			case AF_INET:
				message->add(message, RAT_FRAMED_IP_ADDRESS,
							 vip->get_address(vip));
				break;
			case AF_INET6:
				message->add(message, RAT_FRAMED_IPV6_ADDRESS,
							 vip->get_address(vip));
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Add the Class attributes received in the Access-Accept message to the
 * RADIUS accounting message
 */
static void add_class_attributes(radius_message_t *message, entry_t *entry)
{
	enumerator_t *enumerator;
	chunk_t *cls;

	enumerator = array_create_enumerator(entry->class_attrs);
	while (enumerator->enumerate(enumerator, &cls))
	{
		message->add(message, RAT_CLASS, *cls);
	}
	enumerator->destroy(enumerator);
}

/**
 * Get an existing or create a new entry from the locked session table
 */
static entry_t* get_or_create_entry(private_eap_radius_accounting_t *this,
									ike_sa_id_t *id, uint32_t unique)
{
	entry_t *entry;
	time_t now;

	entry = this->sessions->get(this->sessions, id);
	if (!entry)
	{
		now = time_monotonic(NULL);

		INIT(entry,
			.id = id->clone(id),
			.created = now,
			.interim = {
				.last = now,
			},
			/* default terminate cause, if none other caught */
			.cause = ACCT_CAUSE_USER_REQUEST,
		);
		snprintf(entry->sid, sizeof(entry->sid), "%u-%u", this->prefix, unique);
		this->sessions->put(this->sessions, entry->id, entry);
	}
	return entry;
}

/* forward declaration */
static void schedule_interim(private_eap_radius_accounting_t *this,
							 entry_t *entry);

/**
 * Data passed to send_interim() using callback job
 */
typedef struct {
	/** reference to radius accounting */
	private_eap_radius_accounting_t *this;
	/** IKE_SA identifier to send interim update to */
	ike_sa_id_t *id;
} interim_data_t;

/**
 * Clean up interim data
 */
void destroy_interim_data(interim_data_t *this)
{
	this->id->destroy(this->id);
	free(this);
}

/**
 * Send an interim update for entry of given IKE_SA identifier
 */
static job_requeue_t send_interim(interim_data_t *data)
{
	private_eap_radius_accounting_t *this = data->this;
	usage_t usage;
	radius_message_t *message = NULL;
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	entry_t *entry;
	uint32_t value;
	array_t *stats;
	sa_entry_t *sa, *found;

	ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager, data->id);
	if (!ike_sa)
	{
		return JOB_REQUEUE_NONE;
	}
	stats = collect_stats(ike_sa, &usage);
	charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);

	/* avoid any races by returning IKE_SA before acquiring lock */

	this->mutex->lock(this->mutex);
	entry = this->sessions->get(this->sessions, data->id);
	if (entry)
	{
		entry->interim.last = time_monotonic(NULL);

		enumerator = array_create_enumerator(entry->cached);
		while (enumerator->enumerate(enumerator, &sa))
		{
			if (array_bsearch(stats, sa, sa_find, &found) != -1)
			{
				/* SA is still around, update stats (e.g. for IKEv1 where
				 * SA might get used even after rekeying) */
				sa->usage = found->usage;
			}
			else
			{
				/* SA is gone, add its last stats to the total for this IKE_SA
				 * and remove the cache entry */
				add_usage(&entry->usage, sa->usage);
				array_remove_at(entry->cached, enumerator);
				free(sa);
			}
		}
		enumerator->destroy(enumerator);

		enumerator = array_create_enumerator(entry->migrated);
		while (enumerator->enumerate(enumerator, &sa))
		{
			if (array_bsearch(stats, sa, sa_find, &found) != -1)
			{
				/* SA is still around, but we have to compensate */
				sub_usage(&usage, sa->usage);
			}
			else
			{
				/* SA is gone, subtract stats from the total for this IKE_SA */
				sub_usage(&entry->usage, sa->usage);
				array_remove_at(entry->migrated, enumerator);
				free(sa);
			}
		}
		enumerator->destroy(enumerator);

		add_usage(&usage, entry->usage);

		message = radius_message_create(RMC_ACCOUNTING_REQUEST);
		value = htonl(ACCT_STATUS_INTERIM_UPDATE);
		message->add(message, RAT_ACCT_STATUS_TYPE, chunk_from_thing(value));
		message->add(message, RAT_ACCT_SESSION_ID,
					 chunk_create(entry->sid, strlen(entry->sid)));
		add_class_attributes(message, entry);
		add_ike_sa_parameters(this, message, ike_sa);

		value = htonl(usage.bytes.sent);
		message->add(message, RAT_ACCT_OUTPUT_OCTETS, chunk_from_thing(value));
		value = htonl(usage.bytes.sent >> 32);
		if (value)
		{
			message->add(message, RAT_ACCT_OUTPUT_GIGAWORDS,
						 chunk_from_thing(value));
		}
		value = htonl(usage.packets.sent);
		message->add(message, RAT_ACCT_OUTPUT_PACKETS, chunk_from_thing(value));

		value = htonl(usage.bytes.received);
		message->add(message, RAT_ACCT_INPUT_OCTETS, chunk_from_thing(value));
		value = htonl(usage.bytes.received >> 32);
		if (value)
		{
			message->add(message, RAT_ACCT_INPUT_GIGAWORDS,
						 chunk_from_thing(value));
		}
		value = htonl(usage.packets.received);
		message->add(message, RAT_ACCT_INPUT_PACKETS, chunk_from_thing(value));

		value = htonl(entry->interim.last - entry->created);
		message->add(message, RAT_ACCT_SESSION_TIME, chunk_from_thing(value));

		schedule_interim(this, entry);
	}
	this->mutex->unlock(this->mutex);
	array_destroy_function(stats, (void*)free, NULL);

	if (message)
	{
		if (!send_message(this, message))
		{
			if (lib->settings->get_bool(lib->settings,
							"%s.plugins.eap-radius.accounting_close_on_timeout",
							TRUE, lib->ns))
			{
				eap_radius_handle_timeout(data->id);
			}
		}
		message->destroy(message);
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Schedule interim update for given entry
 */
static void schedule_interim(private_eap_radius_accounting_t *this,
							 entry_t *entry)
{
	if (entry->interim.interval)
	{
		interim_data_t *data;
		timeval_t tv = {
			.tv_sec = entry->interim.last + entry->interim.interval,
		};

		INIT(data,
			.this = this,
			.id = entry->id->clone(entry->id),
		);
		lib->scheduler->schedule_job_tv(lib->scheduler,
			(job_t*)callback_job_create_with_prio(
				(callback_job_cb_t)send_interim,
				data, (void*)destroy_interim_data,
				(callback_job_cancel_t)return_false, JOB_PRIO_CRITICAL), tv);
	}
}

/**
 * Check if an IKE_SA has assigned a virtual IP (to peer)
 */
static bool has_vip(ike_sa_t *ike_sa)
{
	enumerator_t *enumerator;
	host_t *host;
	bool found;

	enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, FALSE);
	found = enumerator->enumerate(enumerator, &host);
	enumerator->destroy(enumerator);

	return found;
}

/**
 * Send an accounting start message
 */
static void send_start(private_eap_radius_accounting_t *this, ike_sa_t *ike_sa)
{
	radius_message_t *message;
	entry_t *entry;
	uint32_t value;

	if (this->acct_req_vip && !has_vip(ike_sa))
	{
		return;
	}

	this->mutex->lock(this->mutex);

	entry = get_or_create_entry(this, ike_sa->get_id(ike_sa),
								ike_sa->get_unique_id(ike_sa));
	if (entry->start_sent)
	{
		this->mutex->unlock(this->mutex);
		return;
	}
	entry->start_sent = TRUE;

	message = radius_message_create(RMC_ACCOUNTING_REQUEST);
	value = htonl(ACCT_STATUS_START);
	message->add(message, RAT_ACCT_STATUS_TYPE, chunk_from_thing(value));
	message->add(message, RAT_ACCT_SESSION_ID,
				 chunk_create(entry->sid, strlen(entry->sid)));
	add_class_attributes(message, entry);

	if (!entry->interim.interval)
	{
		entry->interim.interval = lib->settings->get_time(lib->settings,
					"%s.plugins.eap-radius.accounting_interval", 0, lib->ns);
		if (entry->interim.interval)
		{
			DBG1(DBG_CFG, "scheduling RADIUS Interim-Updates every %us",
				 entry->interim.interval);
		}
	}
	schedule_interim(this, entry);
	this->mutex->unlock(this->mutex);

	add_ike_sa_parameters(this, message, ike_sa);
	if (!send_message(this, message))
	{
		eap_radius_handle_timeout(ike_sa->get_id(ike_sa));
	}
	message->destroy(message);
}

/**
 * Send an account stop message
 */
static void send_stop(private_eap_radius_accounting_t *this, ike_sa_t *ike_sa)
{
	radius_message_t *message;
	enumerator_t *enumerator;
	entry_t *entry;
	sa_entry_t *sa;
	uint32_t value;

	this->mutex->lock(this->mutex);
	entry = this->sessions->remove(this->sessions, ike_sa->get_id(ike_sa));
	this->mutex->unlock(this->mutex);
	if (entry)
	{
		if (!entry->start_sent)
		{	/* we tried to authenticate this peer, but never sent a start */
			destroy_entry(entry);
			return;
		}
		enumerator = array_create_enumerator(entry->cached);
		while (enumerator->enumerate(enumerator, &sa))
		{
			add_usage(&entry->usage, sa->usage);
		}
		enumerator->destroy(enumerator);

		enumerator = array_create_enumerator(entry->migrated);
		while (enumerator->enumerate(enumerator, &sa))
		{
			sub_usage(&entry->usage, sa->usage);
		}
		enumerator->destroy(enumerator);

		message = radius_message_create(RMC_ACCOUNTING_REQUEST);
		value = htonl(ACCT_STATUS_STOP);
		message->add(message, RAT_ACCT_STATUS_TYPE, chunk_from_thing(value));
		message->add(message, RAT_ACCT_SESSION_ID,
					 chunk_create(entry->sid, strlen(entry->sid)));
		add_class_attributes(message, entry);
		add_ike_sa_parameters(this, message, ike_sa);
		add_unclaimed_ips(message, ike_sa);

		value = htonl(entry->usage.bytes.sent);
		message->add(message, RAT_ACCT_OUTPUT_OCTETS, chunk_from_thing(value));
		value = htonl(entry->usage.bytes.sent >> 32);
		if (value)
		{
			message->add(message, RAT_ACCT_OUTPUT_GIGAWORDS,
						 chunk_from_thing(value));
		}
		value = htonl(entry->usage.packets.sent);
		message->add(message, RAT_ACCT_OUTPUT_PACKETS, chunk_from_thing(value));

		value = htonl(entry->usage.bytes.received);
		message->add(message, RAT_ACCT_INPUT_OCTETS, chunk_from_thing(value));
		value = htonl(entry->usage.bytes.received >> 32);
		if (value)
		{
			message->add(message, RAT_ACCT_INPUT_GIGAWORDS,
						 chunk_from_thing(value));
		}
		value = htonl(entry->usage.packets.received);
		message->add(message, RAT_ACCT_INPUT_PACKETS, chunk_from_thing(value));

		value = htonl(time_monotonic(NULL) - entry->created);
		message->add(message, RAT_ACCT_SESSION_TIME, chunk_from_thing(value));

		value = htonl(entry->cause);
		message->add(message, RAT_ACCT_TERMINATE_CAUSE, chunk_from_thing(value));

		if (!send_message(this, message))
		{
			eap_radius_handle_timeout(NULL);
		}
		message->destroy(message);
		destroy_entry(entry);
	}
}

METHOD(listener_t, alert, bool,
	private_eap_radius_accounting_t *this, ike_sa_t *ike_sa, alert_t alert,
	va_list args)
{
	radius_acct_terminate_cause_t cause;
	entry_t *entry;

	switch (alert)
	{
		case ALERT_IKE_SA_EXPIRED:
			cause = ACCT_CAUSE_SESSION_TIMEOUT;
			break;
		case ALERT_RETRANSMIT_SEND_TIMEOUT:
			cause = ACCT_CAUSE_LOST_SERVICE;
			break;
		default:
			return TRUE;
	}
	this->mutex->lock(this->mutex);
	entry = this->sessions->get(this->sessions, ike_sa->get_id(ike_sa));
	if (entry)
	{
		entry->cause = cause;
	}
	this->mutex->unlock(this->mutex);
	return TRUE;
}

METHOD(listener_t, ike_updown, bool,
	private_eap_radius_accounting_t *this, ike_sa_t *ike_sa, bool up)
{
	if (!up)
	{
		enumerator_t *enumerator;
		child_sa_t *child_sa;

		/* update usage for all children just before sending stop */
		enumerator = ike_sa->create_child_sa_enumerator(ike_sa);
		while (enumerator->enumerate(enumerator, &child_sa))
		{
			update_usage(this, ike_sa, child_sa);
		}
		enumerator->destroy(enumerator);

		send_stop(this, ike_sa);
	}
	return TRUE;
}

METHOD(listener_t, message_hook, bool,
	private_eap_radius_accounting_t *this, ike_sa_t *ike_sa,
	message_t *message, bool incoming, bool plain)
{
	/* start accounting here, virtual IP now is set */
	if (plain && ike_sa->get_state(ike_sa) == IKE_ESTABLISHED &&
		!incoming && !message->get_request(message))
	{
		if (ike_sa->get_version(ike_sa) == IKEV2 &&
			message->get_exchange_type(message) == IKE_AUTH)
		{
			send_start(this, ike_sa);
		}
	}
	return TRUE;
}

METHOD(listener_t, assign_vips, bool,
	private_eap_radius_accounting_t *this, ike_sa_t *ike_sa, bool assign)
{
	/* start accounting as soon as the virtual IP is set */
	if (assign && ike_sa->get_version(ike_sa) == IKEV1)
	{
		send_start(this, ike_sa);
	}
	return TRUE;
}

METHOD(listener_t, ike_rekey, bool,
	private_eap_radius_accounting_t *this, ike_sa_t *old, ike_sa_t *new)
{
	entry_t *entry;

	this->mutex->lock(this->mutex);
	entry = this->sessions->remove(this->sessions, old->get_id(old));
	if (entry)
	{
		/* update IKE_SA identifier */
		entry->id->destroy(entry->id);
		entry->id = new->get_id(new);
		entry->id = entry->id->clone(entry->id);
		/* fire new interim update job, old gets invalid */
		schedule_interim(this, entry);

		cleanup_sas(this, new, entry);

		entry = this->sessions->put(this->sessions, entry->id, entry);
		if (entry)
		{
			destroy_entry(entry);
		}
	}
	this->mutex->unlock(this->mutex);

	return TRUE;
}

METHOD(listener_t, child_rekey, bool,
	private_eap_radius_accounting_t *this, ike_sa_t *ike_sa,
	child_sa_t *old, child_sa_t *new)
{
	entry_t *entry;

	update_usage(this, ike_sa, old);
	this->mutex->lock(this->mutex);
	entry = this->sessions->get(this->sessions, ike_sa->get_id(ike_sa));
	if (entry)
	{
		cleanup_sas(this, ike_sa, entry);
	}
	this->mutex->unlock(this->mutex);
	return TRUE;
}

METHOD(listener_t, children_migrate, bool,
	private_eap_radius_accounting_t *this, ike_sa_t *ike_sa, ike_sa_id_t *new,
	uint32_t unique)
{
	enumerator_t *enumerator;
	sa_entry_t *sa, *sa_new, *cached;
	entry_t *entry_old, *entry_new;
	array_t *stats;

	if (!new)
	{
		return TRUE;
	}
	stats = collect_stats(ike_sa, NULL);
	this->mutex->lock(this->mutex);
	entry_old = this->sessions->get(this->sessions, ike_sa->get_id(ike_sa));
	if (entry_old)
	{
		entry_new = get_or_create_entry(this, new, unique);
		enumerator = array_create_enumerator(stats);
		while (enumerator->enumerate(enumerator, &sa))
		{
			/* if the SA was already rekeyed/cached we cache it too on the new
			 * SA to track it properly until it's finally gone */
			if (array_bsearch(entry_old->cached, sa, sa_find, &cached) != -1)
			{
				sa_new = clone_sa(sa);
				array_insert_create(&entry_new->cached, ARRAY_TAIL, sa_new);
				array_sort(entry_new->cached, sa_sort, NULL);
			}
			/* if the SA was used, we store it to compensate on the new SA */
			if (sa->usage.bytes.sent || sa->usage.bytes.received ||
				sa->usage.packets.sent || sa->usage.packets.received)
			{
				sa_new = clone_sa(sa);
				array_insert_create(&entry_new->migrated, ARRAY_TAIL, sa_new);
				array_sort(entry_new->migrated, sa_sort, NULL);
				/* store/update latest stats on old SA to report in Stop */
				update_sa(entry_old, sa->id, sa->usage);
			}
		}
		enumerator->destroy(enumerator);
	}
	this->mutex->unlock(this->mutex);
	array_destroy_function(stats, (void*)free, NULL);
	return TRUE;
}

METHOD(listener_t, child_updown, bool,
	private_eap_radius_accounting_t *this, ike_sa_t *ike_sa,
	child_sa_t *child_sa, bool up)
{
	if (!up && ike_sa->get_state(ike_sa) == IKE_ESTABLISHED)
	{
		update_usage(this, ike_sa, child_sa);
	}
	return TRUE;
}

METHOD(eap_radius_accounting_t, destroy, void,
	private_eap_radius_accounting_t *this)
{
	charon->bus->remove_listener(charon->bus, &this->public.listener);
	singleton = NULL;
	this->mutex->destroy(this->mutex);
	this->sessions->destroy(this->sessions);
	free(this);
}

/**
 * See header
 */
eap_radius_accounting_t *eap_radius_accounting_create()
{
	private_eap_radius_accounting_t *this;

	INIT(this,
		.public = {
			.listener = {
				.alert = _alert,
				.ike_updown = _ike_updown,
				.ike_rekey = _ike_rekey,
				.message = _message_hook,
				.assign_vips = _assign_vips,
				.child_updown = _child_updown,
				.child_rekey = _child_rekey,
				.children_migrate = _children_migrate,
			},
			.destroy = _destroy,
		},
		/* use system time as Session ID prefix */
		.prefix = (uint32_t)time(NULL),
		.sessions = hashtable_create((hashtable_hash_t)hash,
									 (hashtable_equals_t)equals, 32),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);
	if (lib->settings->get_bool(lib->settings,
				"%s.plugins.eap-radius.station_id_with_port", TRUE, lib->ns))
	{
		this->station_id_fmt = "%#H";
	}
	else
	{
		this->station_id_fmt = "%H";
	}
	if (lib->settings->get_bool(lib->settings,
							"%s.plugins.eap-radius.accounting", FALSE, lib->ns))
	{
		singleton = this;
		charon->bus->add_listener(charon->bus, &this->public.listener);
	}
	this->acct_req_vip = lib->settings->get_bool(lib->settings,
							"%s.plugins.eap-radius.accounting_requires_vip",
							FALSE, lib->ns);

	return &this->public;
}

/*
 * Described in header
 */
char *eap_radius_accounting_session_id(ike_sa_t *ike_sa)
{
	entry_t *entry;
	char *sid = NULL;

	if (singleton)
	{
		singleton->mutex->lock(singleton->mutex);
		entry = get_or_create_entry(singleton, ike_sa->get_id(ike_sa),
									ike_sa->get_unique_id(ike_sa));
		sid = strdup(entry->sid);
		singleton->mutex->unlock(singleton->mutex);
	}
	return sid;
}

/*
 * Described in header
 */
void eap_radius_accounting_start_interim(ike_sa_t *ike_sa, uint32_t interval)
{
	if (singleton)
	{
		entry_t *entry;

		DBG1(DBG_CFG, "scheduling RADIUS Interim-Updates every %us", interval);
		singleton->mutex->lock(singleton->mutex);
		entry = get_or_create_entry(singleton, ike_sa->get_id(ike_sa),
									ike_sa->get_unique_id(ike_sa));
		entry->interim.interval = interval;
		singleton->mutex->unlock(singleton->mutex);
	}
}

/*
 * Described in header
 */
void eap_radius_accounting_add_class(ike_sa_t *ike_sa, chunk_t cls)
{
	if (singleton)
	{
		entry_t *entry;
		chunk_t clone;

		DBG2(DBG_CFG, "cache RADIUS Class attribute %B", &cls);
		singleton->mutex->lock(singleton->mutex);
		entry = get_or_create_entry(singleton, ike_sa->get_id(ike_sa),
									ike_sa->get_unique_id(ike_sa));
		clone = chunk_clone(cls);
		array_insert_create_value(&entry->class_attrs, sizeof(chunk_t),
								  ARRAY_TAIL, &clone);
		singleton->mutex->unlock(singleton->mutex);
	}
}