/*
 * Copyright (C) 2018 Tobias Brunner
 * Copyright (C) 2007-2009 Martin Willi
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

#include "backend_manager.h"

#include <sys/types.h>

#include <daemon.h>
#include <collections/linked_list.h>
#include <threading/rwlock.h>


typedef struct private_backend_manager_t private_backend_manager_t;

/**
 * Private data of an backend_manager_t object.
 */
struct private_backend_manager_t {

	/**
	 * Public part of backend_manager_t object.
	 */
	backend_manager_t public;

	/**
	 * list of registered backends
	 */
	linked_list_t *backends;

	/**
	 * rwlock for backends
	 */
	rwlock_t *lock;
};

/**
 * match of an ike_cfg
 */
typedef enum ike_cfg_match_t {
	/* doesn't match at all */
	MATCH_NONE		= 0x00,
	/* match for a %any host. For both hosts, hence skip 0x02 */
	MATCH_ANY		= 0x01,
	/* IKE version matches exactly (config is not for any version) */
	MATCH_VERSION	= 0x04,
	/* local identity matches */
	MATCH_ME		= 0x08,
	/* remote identity matches */
	MATCH_OTHER		= 0x10,
} ike_cfg_match_t;

/**
 * data to pass nested IKE enumerator
 */
typedef struct {
	private_backend_manager_t *this;
	host_t *me;
	host_t *other;
} ike_data_t;

/**
 * inner enumerator constructor for IKE cfgs
 */
static enumerator_t *ike_enum_create(backend_t *backend, ike_data_t *data)
{
	return backend->create_ike_cfg_enumerator(backend, data->me, data->other);
}

/**
 * get a match of a candidate ike_cfg for two hosts
 */
static ike_cfg_match_t get_ike_match(ike_cfg_t *cand, host_t *me, host_t *other,
									 ike_version_t version)
{
	ike_cfg_match_t match = MATCH_NONE;
	int quality;

	if (cand->get_version(cand) != IKE_ANY &&
		version != cand->get_version(cand))
	{
		return MATCH_NONE;
	}

	if (me)
	{
		quality = cand->match_me(cand, me);
		if (!quality)
		{
			return MATCH_NONE;
		}
		match += quality * MATCH_ME;
	}
	else
	{
		match += MATCH_ANY;
	}

	if (other)
	{
		quality = cand->match_other(cand, other);
		if (!quality)
		{
			return MATCH_NONE;
		}
		match += quality * MATCH_OTHER;
	}
	else
	{
		match += MATCH_ANY;
	}

	if (match != MATCH_NONE &&
		cand->get_version(cand) != IKE_ANY)
	{	/* if we have a match, improve it if candidate version specified */
		match += MATCH_VERSION;
	}
	return match;
}

/**
 * list element to help sorting
 */
typedef struct {
	ike_cfg_match_t match;
	ike_cfg_t *cfg;
} ike_match_entry_t;

CALLBACK(ike_enum_filter, bool,
	linked_list_t *configs, enumerator_t *orig, va_list args)
{
	ike_match_entry_t *entry;
	ike_cfg_t **out;

	VA_ARGS_VGET(args, out);

	if (orig->enumerate(orig, &entry))
	{
		*out = entry->cfg;
		return TRUE;
	}
	return FALSE;
}

CALLBACK(ike_match_entry_list_destroy, void,
	linked_list_t *configs)
{
	ike_match_entry_t *entry;

	while (configs->remove_last(configs, (void**)&entry) == SUCCESS)
	{
		entry->cfg->destroy(entry->cfg);
		free(entry);
	}
	configs->destroy(configs);
}

/**
 * Insert entry into match-sorted list
 */
static void insert_sorted_ike(ike_match_entry_t *entry, linked_list_t *list)
{
	enumerator_t *enumerator;
	ike_match_entry_t *current;

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (entry->match > current->match)
		{
			break;
		}
	}
	list->insert_before(list, enumerator, entry);
	enumerator->destroy(enumerator);
}

/**
 * Create a sorted list of all matching IKE configs
 */
static linked_list_t *get_matching_ike_cfgs(private_backend_manager_t *this,
											host_t *me, host_t *other,
											ike_version_t version)
{
	ike_cfg_t *current;
	char *my_addr, *other_addr;
	enumerator_t *enumerator;
	ike_data_t *data;
	linked_list_t *configs;
	ike_cfg_match_t match;
	ike_match_entry_t *entry;

	INIT(data,
		.this = this,
		.me = me,
		.other = other,
	);

	configs = linked_list_create();

	this->lock->read_lock(this->lock);
	enumerator = enumerator_create_nested(
						this->backends->create_enumerator(this->backends),
						(void*)ike_enum_create, data, (void*)free);

	while (enumerator->enumerate(enumerator, &current))
	{
		my_addr = current->get_my_addr(current);
		other_addr = current->get_other_addr(current);
		match = get_ike_match(current, me, other, version);
		DBG3(DBG_CFG, "ike config match: %d (%s...%s %N)", match, my_addr,
			 other_addr, ike_version_names, current->get_version(current));

		if (match)
		{
			DBG2(DBG_CFG, "  candidate: %s...%s, prio %d",
				 my_addr, other_addr, match);

			INIT(entry,
				.match = match,
				.cfg = current->get_ref(current),
			);
			insert_sorted_ike(entry, configs);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return configs;
}

METHOD(backend_manager_t, get_ike_cfg, ike_cfg_t*,
	private_backend_manager_t *this, host_t *me, host_t *other,
	ike_version_t version)
{
	linked_list_t *configs;
	ike_match_entry_t *entry;
	ike_cfg_t *found = NULL;
	char *my_addr, *other_addr;

	DBG2(DBG_CFG, "looking for an %N config for %H...%H", ike_version_names,
		 version, me, other);

	configs = get_matching_ike_cfgs(this, me, other, version);
	if (configs->get_first(configs, (void**)&entry) == SUCCESS)
	{
		found = entry->cfg->get_ref(entry->cfg);

		my_addr = found->get_my_addr(found);
		other_addr = found->get_other_addr(found);
		DBG2(DBG_CFG, "found matching ike config: %s...%s with prio %d",
			 my_addr, other_addr, entry->match);
	}
	ike_match_entry_list_destroy(configs);

	return found;
}

METHOD(backend_manager_t, create_ike_cfg_enumerator, enumerator_t*,
	private_backend_manager_t *this, host_t *me, host_t *other,
	ike_version_t version)
{
	linked_list_t *configs;

	DBG2(DBG_CFG, "looking for %N configs for %H...%H", ike_version_names,
		 version, me, other);

	configs = get_matching_ike_cfgs(this, me, other, version);

	return enumerator_create_filter(configs->create_enumerator(configs),
									ike_enum_filter, configs,
									ike_match_entry_list_destroy);
}

/**
 * Get the best ID match in one of the configs auth_cfg
 */
static id_match_t get_peer_match(identification_t *id,
								 peer_cfg_t *cfg, bool local)
{
	enumerator_t *enumerator;
	auth_cfg_t *auth;
	identification_t *candidate;
	id_match_t match = ID_MATCH_NONE;
	char *where = local ? "local" : "remote";
	chunk_t data;

	if (!id)
	{
		DBG3(DBG_CFG, "  %s id match: %d (%N)",
			 where, ID_MATCH_ANY, id_type_names, ID_ANY);
		return ID_MATCH_ANY;
	}

	/* compare first auth config only */
	enumerator = cfg->create_auth_cfg_enumerator(cfg, local);
	if (enumerator->enumerate(enumerator, &auth))
	{
		candidate = auth->get(auth, AUTH_RULE_IDENTITY);
		if (candidate)
		{
			match = id->matches(id, candidate);
			/* match vice-versa, as the proposed IDr might be ANY */
			if (!match)
			{
				match = candidate->matches(candidate, id);
			}
		}
		else
		{
			match = ID_MATCH_ANY;
		}
	}
	enumerator->destroy(enumerator);

	data = id->get_encoding(id);
	DBG3(DBG_CFG, "  %s id match: %d (%N: %#B)",
		 where, match, id_type_names, id->get_type(id), &data);
	return match;
}

/**
 * data to pass nested peer enumerator
 */
typedef struct {
	rwlock_t *lock;
	identification_t *me;
	identification_t *other;
} peer_data_t;

/**
 * list element to help sorting
 */
typedef struct {
	id_match_t match_peer;
	ike_cfg_match_t match_ike;
	peer_cfg_t *cfg;
} match_entry_t;

/**
 * inner enumerator constructor for peer cfgs
 */
static enumerator_t *peer_enum_create(backend_t *backend, peer_data_t *data)
{
	return backend->create_peer_cfg_enumerator(backend, data->me, data->other);
}

/**
 * unlock/cleanup peer enumerator
 */
static void peer_enum_destroy(peer_data_t *data)
{
	data->lock->unlock(data->lock);
	free(data);
}

CALLBACK(peer_enum_filter, bool,
	linked_list_t *configs, enumerator_t *orig, va_list args)
{
	match_entry_t *entry;
	peer_cfg_t **out;

	VA_ARGS_VGET(args, out);

	if (orig->enumerate(orig, &entry))
	{
		*out = entry->cfg;
		return TRUE;
	}
	return FALSE;
}

CALLBACK(peer_enum_filter_destroy, void,
	linked_list_t *configs)
{
	match_entry_t *entry;

	while (configs->remove_last(configs, (void**)&entry) == SUCCESS)
	{
		entry->cfg->destroy(entry->cfg);
		free(entry);
	}
	configs->destroy(configs);
}

/**
 * Insert entry into match-sorted list
 */
static void insert_sorted(match_entry_t *entry, linked_list_t *list)
{
	enumerator_t *enumerator;
	match_entry_t *current;

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &current))
	{
		if ((entry->match_ike > current->match_ike &&
			 entry->match_peer >= current->match_peer) ||
			(entry->match_ike >= current->match_ike &&
			  entry->match_peer > current->match_peer))
		{
			break;
		}
	}
	list->insert_before(list, enumerator, entry);
	enumerator->destroy(enumerator);
}

METHOD(backend_manager_t, create_peer_cfg_enumerator, enumerator_t*,
	private_backend_manager_t *this, host_t *me, host_t *other,
	identification_t *my_id, identification_t *other_id, ike_version_t version)
{
	enumerator_t *enumerator;
	peer_data_t *data;
	peer_cfg_t *cfg;
	linked_list_t *configs;

	INIT(data,
		.lock = this->lock,
		.me = my_id,
		.other = other_id,
	);

	/* create a sorted list with all matches */
	this->lock->read_lock(this->lock);
	enumerator = enumerator_create_nested(
					this->backends->create_enumerator(this->backends),
					(void*)peer_enum_create, data, (void*)peer_enum_destroy);

	if (!me && !other && !my_id && !other_id)
	{	/* shortcut if we are doing a "listall" */
		return enumerator;
	}

	configs = linked_list_create();
	while (enumerator->enumerate(enumerator, &cfg))
	{
		ike_cfg_t *ike_cfg = cfg->get_ike_cfg(cfg);
		ike_cfg_match_t match_ike;
		id_match_t match_peer_me, match_peer_other;
		match_entry_t *entry;
		char *my_addr, *other_addr;

		match_ike = get_ike_match(ike_cfg, me, other, version);
		my_addr = ike_cfg->get_my_addr(ike_cfg);
		other_addr = ike_cfg->get_other_addr(ike_cfg);
		DBG3(DBG_CFG, "peer config \"%s\", ike match: %d (%s...%s %N)",
			 cfg->get_name(cfg), match_ike, my_addr, other_addr,
			 ike_version_names, ike_cfg->get_version(ike_cfg));

		if (!match_ike)
		{
			continue;
		}

		match_peer_me = get_peer_match(my_id, cfg, TRUE);
		if (!match_peer_me)
		{
			continue;
		}
		match_peer_other = get_peer_match(other_id, cfg, FALSE);

		if (match_peer_other)
		{
			DBG2(DBG_CFG, "  candidate \"%s\", match: %d/%d/%d (me/other/ike)",
				 cfg->get_name(cfg), match_peer_me, match_peer_other, match_ike);
			INIT(entry,
				.match_peer = match_peer_me + match_peer_other,
				.match_ike = match_ike,
				.cfg = cfg->get_ref(cfg),
			);
			insert_sorted(entry, configs);
		}
	}
	enumerator->destroy(enumerator);

	return enumerator_create_filter(configs->create_enumerator(configs),
									peer_enum_filter, configs,
									peer_enum_filter_destroy);
}

METHOD(backend_manager_t, get_peer_cfg_by_name, peer_cfg_t*,
	private_backend_manager_t *this, char *name)
{
	backend_t *backend;
	peer_cfg_t *config = NULL;
	enumerator_t *enumerator;

	this->lock->read_lock(this->lock);
	enumerator = this->backends->create_enumerator(this->backends);
	while (config == NULL && enumerator->enumerate(enumerator, (void**)&backend))
	{
		config = backend->get_peer_cfg_by_name(backend, name);
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return config;
}

METHOD(backend_manager_t, remove_backend, void,
	private_backend_manager_t *this, backend_t *backend)
{
	this->lock->write_lock(this->lock);
	this->backends->remove(this->backends, backend, NULL);
	this->lock->unlock(this->lock);
}

METHOD(backend_manager_t, add_backend, void,
	private_backend_manager_t *this, backend_t *backend)
{
	this->lock->write_lock(this->lock);
	this->backends->insert_last(this->backends, backend);
	this->lock->unlock(this->lock);
}

METHOD(backend_manager_t, destroy, void,
	private_backend_manager_t *this)
{
	this->backends->destroy(this->backends);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * Described in header
 */
backend_manager_t *backend_manager_create()
{
	private_backend_manager_t *this;

	INIT(this,
		.public = {
			.get_ike_cfg = _get_ike_cfg,
			.create_ike_cfg_enumerator = _create_ike_cfg_enumerator,
			.get_peer_cfg_by_name = _get_peer_cfg_by_name,
			.create_peer_cfg_enumerator = _create_peer_cfg_enumerator,
			.add_backend = _add_backend,
			.remove_backend = _remove_backend,
			.destroy = _destroy,
		},
		.backends = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
