/*
 * Copyright (C) 2016 Tobias Brunner
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

#include "bypass_lan_listener.h"

#include <collections/hashtable.h>
#include <collections/linked_list.h>
#include <threading/mutex.h>
#include <processing/jobs/callback_job.h>

#include <daemon.h>

typedef struct private_bypass_lan_listener_t private_bypass_lan_listener_t;

/**
 * Private data
 */
struct private_bypass_lan_listener_t {

	/**
	 * Public interface.
	 */
	bypass_lan_listener_t public;

	/**
	 * Currently installed bypass policies, bypass_policy_t*.
	 */
	hashtable_t *policies;

	/**
	 * Mutex to access list of policies.
	 */
	mutex_t *mutex;

	/**
	 * List of interface names to include or exclude (char*), NULL if interfaces
	 * are not filtered.
	 */
	linked_list_t *ifaces_filter;

	/**
	 * TRUE to exclude interfaces listed in ifaces_filter, FALSE to consider
	 * only those listed there.
	 */
	bool ifaces_exclude;
};

/**
 * Data for bypass policies
 */
typedef struct {
	private_bypass_lan_listener_t *listener;
	host_t *net;
	uint8_t mask;
	char *iface;
	child_cfg_t *cfg;
} bypass_policy_t;

/**
 * Destroy a bypass policy
 */
static void bypass_policy_destroy(bypass_policy_t *this)
{
	traffic_selector_t *ts;

	if (this->cfg)
	{
		ts = traffic_selector_create_from_subnet(this->net->clone(this->net),
												 this->mask, 0, 0, 65535);
		DBG1(DBG_IKE, "uninstalling bypass policy for %R", ts);
		charon->shunts->uninstall(charon->shunts, "bypass-lan",
								  this->cfg->get_name(this->cfg));
		this->cfg->destroy(this->cfg);
		ts->destroy(ts);
	}
	this->net->destroy(this->net);
	free(this->iface);
	free(this);
}

/**
 * Hash a bypass policy
 */
static u_int policy_hash(bypass_policy_t *policy)
{
	return chunk_hash_inc(policy->net->get_address(policy->net),
						  chunk_hash(chunk_from_thing(policy->mask)));
}

/**
 * Compare bypass policy
 */
static bool policy_equals(bypass_policy_t *a, bypass_policy_t *b)
{
	return a->mask == b->mask && a->net->equals(a->net, b->net);
}

/**
 * Check if an interface should be considered
 */
static bool consider_interface(private_bypass_lan_listener_t *this, char *iface)
{
	if (!iface || !this->ifaces_filter)
	{
		return TRUE;
	}
	return this->ifaces_filter->find_first(this->ifaces_filter,
					linked_list_match_str, NULL, iface) != this->ifaces_exclude;
}

/**
 * Job updating bypass policies
 */
static job_requeue_t update_bypass(private_bypass_lan_listener_t *this)
{
	enumerator_t *enumerator;
	hashtable_t *seen;
	bypass_policy_t *found, *lookup;
	traffic_selector_t *ts;
	host_t *net;
	uint8_t mask;
	char *iface;

	seen = hashtable_create((hashtable_hash_t)policy_hash,
							(hashtable_equals_t)policy_equals, 4);

	this->mutex->lock(this->mutex);

	enumerator = charon->kernel->create_local_subnet_enumerator(charon->kernel);
	while (enumerator->enumerate(enumerator, &net, &mask, &iface))
	{
		if (!consider_interface(this, iface))
		{
			continue;
		}

		INIT(lookup,
			.net = net->clone(net),
			.mask = mask,
			.iface = strdupnull(iface),
		);
		found = seen->put(seen, lookup, lookup);
		if (found)
		{	/* in case the same subnet is on multiple interfaces */
			bypass_policy_destroy(found);
		}

		found = this->policies->get(this->policies, lookup);
		if (!found)
		{
			child_cfg_create_t child = {
				.mode = MODE_PASS,
			};
			child_cfg_t *cfg;
			char name[128];

			ts = traffic_selector_create_from_subnet(net->clone(net), mask,
													 0, 0, 65535);
			snprintf(name, sizeof(name), "Bypass LAN %R", ts);

			cfg = child_cfg_create(name, &child);
			cfg->add_traffic_selector(cfg, FALSE, ts->clone(ts));
			cfg->add_traffic_selector(cfg, TRUE, ts);
			charon->shunts->install(charon->shunts, "bypass-lan", cfg);
			DBG1(DBG_IKE, "installed bypass policy for %R", ts);

			INIT(found,
				.net = net->clone(net),
				.mask = mask,
				.iface = strdupnull(iface),
				.cfg = cfg,
			);
			this->policies->put(this->policies, found, found);
		}
	}
	enumerator->destroy(enumerator);

	enumerator = this->policies->create_enumerator(this->policies);
	while (enumerator->enumerate(enumerator, NULL, &lookup))
	{
		found = seen->get(seen, lookup);
		if (!found)
		{
			this->policies->remove_at(this->policies, enumerator);
			bypass_policy_destroy(lookup);
		}
		else if (!streq(lookup->iface, found->iface))
		{	/* if the subnet is on multiple interfaces, we only get the last
			 * one (hopefully, they are enumerated in a consistent order) */
			ts = traffic_selector_create_from_subnet(
												lookup->net->clone(lookup->net),
												lookup->mask, 0, 0, 65535);
			DBG1(DBG_IKE, "interface change for bypass policy for %R (from %s "
				 "to %s)", ts, lookup->iface, found->iface);
			ts->destroy(ts);
			free(lookup->iface);
			lookup->iface = strdupnull(found->iface);
			/* there is currently no API to update shunts, so we remove and
			 * reinstall it to update the route */
			charon->shunts->uninstall(charon->shunts, "bypass-lan",
									  lookup->cfg->get_name(lookup->cfg));
			charon->shunts->install(charon->shunts, "bypass-lan", lookup->cfg);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);

	seen->destroy_function(seen, (void*)bypass_policy_destroy);
	return JOB_REQUEUE_NONE;
}

METHOD(kernel_listener_t, roam, bool,
	private_bypass_lan_listener_t *this, bool address)
{
	lib->processor->queue_job(lib->processor,
			(job_t*)callback_job_create((callback_job_cb_t)update_bypass, this,
									NULL, (callback_job_cancel_t)return_false));
	return TRUE;
}

METHOD(bypass_lan_listener_t, reload_interfaces, void,
	private_bypass_lan_listener_t *this)
{
	char *ifaces;

	this->mutex->lock(this->mutex);
	DESTROY_FUNCTION_IF(this->ifaces_filter, (void*)free);
	this->ifaces_filter = NULL;
	this->ifaces_exclude = FALSE;

	ifaces = lib->settings->get_str(lib->settings,
					"%s.plugins.bypass-lan.interfaces_use", NULL, lib->ns);
	if (!ifaces)
	{
		this->ifaces_exclude = TRUE;
		ifaces = lib->settings->get_str(lib->settings,
					"%s.plugins.bypass-lan.interfaces_ignore", NULL, lib->ns);
	}
	if (ifaces)
	{
		enumerator_t *enumerator;
		char *iface;

		enumerator = enumerator_create_token(ifaces, ",", " ");
		while (enumerator->enumerate(enumerator, &iface))
		{
			if (!this->ifaces_filter)
			{
				this->ifaces_filter = linked_list_create();
			}
			this->ifaces_filter->insert_last(this->ifaces_filter,
											 strdup(iface));
		}
		enumerator->destroy(enumerator);
	}
	this->mutex->unlock(this->mutex);
	lib->processor->queue_job(lib->processor,
			(job_t*)callback_job_create((callback_job_cb_t)update_bypass, this,
									NULL, (callback_job_cancel_t)return_false));
}

METHOD(bypass_lan_listener_t, destroy, void,
	private_bypass_lan_listener_t *this)
{
	enumerator_t *enumerator;
	bypass_policy_t *policy;

	enumerator = this->policies->create_enumerator(this->policies);
	while (enumerator->enumerate(enumerator, NULL, &policy))
	{
		bypass_policy_destroy(policy);
	}
	enumerator->destroy(enumerator);
	DESTROY_FUNCTION_IF(this->ifaces_filter, (void*)free);
	this->policies->destroy(this->policies);
	this->mutex->destroy(this->mutex);
	free(this);
}

/*
 * See header
 */
bypass_lan_listener_t *bypass_lan_listener_create()
{
	private_bypass_lan_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.roam = _roam,
			},
			.reload_interfaces = _reload_interfaces,
			.destroy = _destroy,
		},
		.policies = hashtable_create((hashtable_hash_t)policy_hash,
									 (hashtable_equals_t)policy_equals, 4),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	reload_interfaces(this);
	return &this->public;
}
