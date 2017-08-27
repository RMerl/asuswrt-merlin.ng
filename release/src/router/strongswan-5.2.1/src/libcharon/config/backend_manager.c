/*
 * Copyright (C) 2007-2009 Martin Willi
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

METHOD(backend_manager_t, get_ike_cfg, ike_cfg_t*,
	private_backend_manager_t *this, host_t *me, host_t *other,
	ike_version_t version)
{
	ike_cfg_t *current, *found = NULL;
	char *my_addr, *other_addr;
	enumerator_t *enumerator;
	ike_cfg_match_t match, best = MATCH_ANY;
	ike_data_t *data;

	INIT(data,
		.this = this,
		.me = me,
		.other = other,
	);

	DBG2(DBG_CFG, "looking for an ike config for %H...%H", me, other);

	this->lock->read_lock(this->lock);
	enumerator = enumerator_create_nested(
						this->backends->create_enumerator(this->backends),
						(void*)ike_enum_create, data, (void*)free);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		match = get_ike_match(current, me, other, version);
		DBG3(DBG_CFG, "ike config match: %d (%H %H %N)",
			 match, me, other, ike_version_names, version);
		if (match)
		{
			my_addr = current->get_my_addr(current);
			other_addr = current->get_other_addr(current);
			DBG2(DBG_CFG, "  candidate: %s...%s, prio %d",
				 my_addr, other_addr, match);
			if (match > best)
			{
				DESTROY_IF(found);
				found = current;
				found->get_ref(found);
				best = match;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	if (found)
	{
		my_addr = found->get_my_addr(found);
		other_addr = found->get_other_addr(found);
		DBG2(DBG_CFG, "found matching ike config: %s...%s with prio %d",
			 my_addr, other_addr, best);
	}
	return found;
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
		DBG3(DBG_CFG, "peer config match %s: %d (%N)",
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
	DBG3(DBG_CFG, "peer config match %s: %d (%N -> %#B)",
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

/**
 * convert enumerator value from match_entry to config
 */
static bool peer_enum_filter(linked_list_t *configs,
							 match_entry_t **in, peer_cfg_t **out)
{
	*out = (*in)->cfg;
	return TRUE;
}

/**
 * Clean up temporary config list
 */
static void peer_enum_filter_destroy(linked_list_t *configs)
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
 * Insert entry into match-sorted list, using helper
 */
static void insert_sorted(match_entry_t *entry, linked_list_t *list,
						  linked_list_t *helper)
{
	match_entry_t *current;

	while (list->remove_first(list, (void**)&current) == SUCCESS)
	{
		helper->insert_last(helper, current);
	}
	while (helper->remove_first(helper, (void**)&current) == SUCCESS)
	{
		if (entry && (
			 (entry->match_ike > current->match_ike &&
			  entry->match_peer >= current->match_peer) ||
			 (entry->match_ike >= current->match_ike &&
			  entry->match_peer > current->match_peer)))
		{
			list->insert_last(list, entry);
			entry = NULL;
		}
		list->insert_last(list, current);
	}
	if (entry)
	{
		list->insert_last(list, entry);
	}
}

METHOD(backend_manager_t, create_peer_cfg_enumerator, enumerator_t*,
	private_backend_manager_t *this, host_t *me, host_t *other,
	identification_t *my_id, identification_t *other_id, ike_version_t version)
{
	enumerator_t *enumerator;
	peer_data_t *data;
	peer_cfg_t *cfg;
	linked_list_t *configs, *helper;

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
	/* only once allocated helper list for sorting */
	helper = linked_list_create();
	while (enumerator->enumerate(enumerator, &cfg))
	{
		id_match_t match_peer_me, match_peer_other;
		ike_cfg_match_t match_ike;
		match_entry_t *entry;

		match_peer_me = get_peer_match(my_id, cfg, TRUE);
		match_peer_other = get_peer_match(other_id, cfg, FALSE);
		match_ike = get_ike_match(cfg->get_ike_cfg(cfg), me, other, version);
		DBG3(DBG_CFG, "ike config match: %d (%H %H %N)",
			 match_ike, me, other, ike_version_names, version);

		if (match_peer_me && match_peer_other && match_ike)
		{
			DBG2(DBG_CFG, "  candidate \"%s\", match: %d/%d/%d (me/other/ike)",
				 cfg->get_name(cfg), match_peer_me, match_peer_other, match_ike);

			INIT(entry,
				.match_peer = match_peer_me + match_peer_other,
				.match_ike = match_ike,
				.cfg = cfg->get_ref(cfg),
			);
			insert_sorted(entry, configs, helper);
		}
	}
	enumerator->destroy(enumerator);
	helper->destroy(helper);

	return enumerator_create_filter(configs->create_enumerator(configs),
									(void*)peer_enum_filter, configs,
									(void*)peer_enum_filter_destroy);
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
 * Described in header-file

 */
backend_manager_t *backend_manager_create()
{
	private_backend_manager_t *this;

	INIT(this,
		.public = {
			.get_ike_cfg = _get_ike_cfg,
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
