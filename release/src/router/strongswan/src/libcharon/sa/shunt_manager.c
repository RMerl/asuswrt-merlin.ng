/*
 * Copyright (C) 2015-2017 Tobias Brunner
 * Copyright (C) 2011-2016 Andreas Steffen
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

#include "shunt_manager.h"

#include <daemon.h>
#include <threading/rwlock.h>
#include <threading/rwlock_condvar.h>
#include <collections/linked_list.h>

#define INSTALL_DISABLED ((u_int)~0)

typedef struct private_shunt_manager_t private_shunt_manager_t;

/**
 * Private data of an shunt_manager_t object.
 */
struct private_shunt_manager_t {

	/**
	 * Public shunt_manager_t interface.
	 */
	shunt_manager_t public;

	/**
	 * Installed shunts, as entry_t
	 */
	linked_list_t *shunts;

	/**
	 * Lock to safely access the list of shunts
	 */
	rwlock_t *lock;

	/**
	 * Number of threads currently installing shunts, or INSTALL_DISABLED
	 */
	u_int installing;

	/**
	 * Condvar to signal shunt installation
	 */
	rwlock_condvar_t *condvar;
};

/**
 * Config entry for a shunt
 */
typedef struct {
	/**
	 * Configured namespace
	 */
	char *ns;

	/**
	 * Child config
	 */
	child_cfg_t *cfg;

} entry_t;

/**
 * Destroy a config entry
 */
static void entry_destroy(entry_t *this)
{
	this->cfg->destroy(this->cfg);
	free(this->ns);
	free(this);
}

/**
 * Install in and out shunt policies in the kernel
 */
static bool install_shunt_policy(child_cfg_t *child)
{
	enumerator_t *e_my_ts, *e_other_ts;
	linked_list_t *my_ts_list, *other_ts_list, *hosts;
	traffic_selector_t *my_ts, *other_ts;
	host_t *host_any, *host_any6;
	policy_type_t policy_type;
	policy_priority_t policy_prio;
	status_t status = SUCCESS;
	uint32_t manual_prio;
	char *interface;
	bool fwd_out;
	ipsec_sa_cfg_t sa = { .mode = MODE_TRANSPORT };

	switch (child->get_mode(child))
	{
		case MODE_PASS:
			policy_type = POLICY_PASS;
			policy_prio = POLICY_PRIORITY_PASS;
			break;
		case MODE_DROP:
			policy_type = POLICY_DROP;
			policy_prio = POLICY_PRIORITY_FALLBACK;
			break;
		default:
			return FALSE;
	}

	host_any = host_create_any(AF_INET);
	host_any6 = host_create_any(AF_INET6);

	hosts = linked_list_create_with_items(host_any, host_any6, NULL);
	my_ts_list =    child->get_traffic_selectors(child, TRUE,  NULL, hosts,
												 FALSE);
	other_ts_list = child->get_traffic_selectors(child, FALSE, NULL, hosts,
												 FALSE);
	hosts->destroy(hosts);

	manual_prio = child->get_manual_prio(child);
	interface = child->get_interface(child);
	fwd_out = child->has_option(child, OPT_FWD_OUT_POLICIES);

	/* enumerate pairs of traffic selectors */
	e_my_ts = my_ts_list->create_enumerator(my_ts_list);
	while (e_my_ts->enumerate(e_my_ts, &my_ts))
	{
		e_other_ts = other_ts_list->create_enumerator(other_ts_list);
		while (e_other_ts->enumerate(e_other_ts, &other_ts))
		{
			if (my_ts->get_type(my_ts) != other_ts->get_type(other_ts))
			{
				continue;
			}
			if (my_ts->get_protocol(my_ts) &&
				other_ts->get_protocol(other_ts) &&
				my_ts->get_protocol(my_ts) != other_ts->get_protocol(other_ts))
			{
				continue;
			}
			/* install out policy */
			kernel_ipsec_policy_id_t id = {
				.dir = POLICY_OUT,
				.src_ts = my_ts,
				.dst_ts = other_ts,
				.mark = child->get_mark(child, FALSE),
				.interface = interface,
			};
			kernel_ipsec_manage_policy_t policy = {
				.type = policy_type,
				.prio = policy_prio,
				.manual_prio = manual_prio,
				.src = host_any,
				.dst = host_any,
				.sa = &sa,
			};
			status |= charon->kernel->add_policy(charon->kernel, &id, &policy);
			if (fwd_out)
			{	/* install "outbound" forward policy */
				id.dir = POLICY_FWD;
				status |= charon->kernel->add_policy(charon->kernel, &id, &policy);
			}
			/* install in policy */
			id = (kernel_ipsec_policy_id_t){
				.dir = POLICY_IN,
				.src_ts = other_ts,
				.dst_ts = my_ts,
				.mark = child->get_mark(child, TRUE),
				.interface = interface,
			};
			status |= charon->kernel->add_policy(charon->kernel, &id, &policy);
			/* install "inbound" forward policy */
			id.dir = POLICY_FWD;
			status |= charon->kernel->add_policy(charon->kernel, &id, &policy);
		}
		e_other_ts->destroy(e_other_ts);
	}
	e_my_ts->destroy(e_my_ts);

	my_ts_list->destroy_offset(my_ts_list,
							   offsetof(traffic_selector_t, destroy));
	other_ts_list->destroy_offset(other_ts_list,
							   offsetof(traffic_selector_t, destroy));
	host_any6->destroy(host_any6);
	host_any->destroy(host_any);

	return status == SUCCESS;
}

METHOD(shunt_manager_t, install, bool,
	private_shunt_manager_t *this, char *ns, child_cfg_t *cfg)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool found = FALSE, success;

	if (!ns)
	{
		DBG1(DBG_CFG, "missing namespace for shunt policy '%s'",
			 cfg->get_name(cfg));
		return FALSE;
	}

	/* check if not already installed */
	this->lock->write_lock(this->lock);
	if (this->installing == INSTALL_DISABLED)
	{	/* flush() has been called */
		this->lock->unlock(this->lock);
		return FALSE;
	}
	enumerator = this->shunts->create_enumerator(this->shunts);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (streq(ns, entry->ns) &&
			streq(cfg->get_name(cfg), entry->cfg->get_name(entry->cfg)))
		{
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (found)
	{
		DBG1(DBG_CFG, "shunt %N policy '%s' already installed",
			 ipsec_mode_names, cfg->get_mode(cfg), cfg->get_name(cfg));
		this->lock->unlock(this->lock);
		return TRUE;
	}
	INIT(entry,
		.ns = strdup(ns),
		.cfg = cfg->get_ref(cfg),
	);
	this->shunts->insert_last(this->shunts, entry);
	this->installing++;
	this->lock->unlock(this->lock);

	success = install_shunt_policy(cfg);

	this->lock->write_lock(this->lock);
	if (!success)
	{
		this->shunts->remove(this->shunts, entry, NULL);
		entry_destroy(entry);
	}
	this->installing--;
	this->condvar->signal(this->condvar);
	this->lock->unlock(this->lock);
	return success;
}

/**
 * Uninstall in and out shunt policies in the kernel
 */
static void uninstall_shunt_policy(child_cfg_t *child)
{
	enumerator_t *e_my_ts, *e_other_ts;
	linked_list_t *my_ts_list, *other_ts_list, *hosts;
	traffic_selector_t *my_ts, *other_ts;
	host_t *host_any, *host_any6;
	policy_type_t policy_type;
	policy_priority_t policy_prio;
	status_t status = SUCCESS;
	uint32_t manual_prio;
	char *interface;
	bool fwd_out;
	ipsec_sa_cfg_t sa = { .mode = MODE_TRANSPORT };

	switch (child->get_mode(child))
	{
		case MODE_PASS:
			policy_type = POLICY_PASS;
			policy_prio = POLICY_PRIORITY_PASS;
			break;
		case MODE_DROP:
			policy_type = POLICY_DROP;
			policy_prio = POLICY_PRIORITY_FALLBACK;
			break;
		default:
			return;
	}

	host_any = host_create_any(AF_INET);
	host_any6 = host_create_any(AF_INET6);

	hosts = linked_list_create_with_items(host_any, host_any6, NULL);
	my_ts_list =    child->get_traffic_selectors(child, TRUE,  NULL, hosts,
												 FALSE);
	other_ts_list = child->get_traffic_selectors(child, FALSE, NULL, hosts,
												 FALSE);
	hosts->destroy(hosts);

	manual_prio = child->get_manual_prio(child);
	interface = child->get_interface(child);
	fwd_out = child->has_option(child, OPT_FWD_OUT_POLICIES);

	/* enumerate pairs of traffic selectors */
	e_my_ts = my_ts_list->create_enumerator(my_ts_list);
	while (e_my_ts->enumerate(e_my_ts, &my_ts))
	{
		e_other_ts = other_ts_list->create_enumerator(other_ts_list);
		while (e_other_ts->enumerate(e_other_ts, &other_ts))
		{
			if (my_ts->get_type(my_ts) != other_ts->get_type(other_ts))
			{
				continue;
			}
			if (my_ts->get_protocol(my_ts) &&
				other_ts->get_protocol(other_ts) &&
				my_ts->get_protocol(my_ts) != other_ts->get_protocol(other_ts))
			{
				continue;
			}
			/* uninstall out policy */
			kernel_ipsec_policy_id_t id = {
				.dir = POLICY_OUT,
				.src_ts = my_ts,
				.dst_ts = other_ts,
				.mark = child->get_mark(child, FALSE),
				.interface = interface,
			};
			kernel_ipsec_manage_policy_t policy = {
				.type = policy_type,
				.prio = policy_prio,
				.manual_prio = manual_prio,
				.src = host_any,
				.dst = host_any,
				.sa = &sa,
			};
			status |= charon->kernel->del_policy(charon->kernel, &id, &policy);
			if (fwd_out)
			{
				/* uninstall "outbound" forward policy */
				id.dir = POLICY_FWD;
				status |= charon->kernel->del_policy(charon->kernel, &id, &policy);
			}
			/* uninstall in policy */
			id = (kernel_ipsec_policy_id_t){
				.dir = POLICY_IN,
				.src_ts = other_ts,
				.dst_ts = my_ts,
				.mark = child->get_mark(child, TRUE),
				.interface = interface,
			};
			status |= charon->kernel->del_policy(charon->kernel, &id, &policy);
			/* uninstall "inbound" forward policy */
			id.dir = POLICY_FWD;
			status |= charon->kernel->del_policy(charon->kernel, &id, &policy);
		}
		e_other_ts->destroy(e_other_ts);
	}
	e_my_ts->destroy(e_my_ts);

	my_ts_list->destroy_offset(my_ts_list,
							   offsetof(traffic_selector_t, destroy));
	other_ts_list->destroy_offset(other_ts_list,
							   offsetof(traffic_selector_t, destroy));
	host_any6->destroy(host_any6);
	host_any->destroy(host_any);

	if (status != SUCCESS)
	{
		DBG1(DBG_CFG, "uninstalling shunt %N 'policy %s' failed",
			 ipsec_mode_names, child->get_mode(child), child->get_name(child));
	}
}

METHOD(shunt_manager_t, uninstall, bool,
	private_shunt_manager_t *this, char *ns, char *name)
{
	enumerator_t *enumerator;
	entry_t *entry, *found = NULL;

	this->lock->write_lock(this->lock);
	enumerator = this->shunts->create_enumerator(this->shunts);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if ((!ns || streq(ns, entry->ns)) &&
			streq(name, entry->cfg->get_name(entry->cfg)))
		{
			this->shunts->remove_at(this->shunts, enumerator);
			found = entry;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	if (!found)
	{
		return FALSE;
	}
	uninstall_shunt_policy(found->cfg);
	entry_destroy(found);
	return TRUE;
}

CALLBACK(filter_entries, bool,
	void *unused, enumerator_t *orig, va_list args)
{
	entry_t *entry;
	child_cfg_t **cfg;
	char **ns;

	VA_ARGS_VGET(args, ns, cfg);

	if (orig->enumerate(orig, &entry))
	{
		if (ns)
		{
			*ns = entry->ns;
		}
		*cfg = entry->cfg;
		return TRUE;
	}
	return FALSE;
}

METHOD(shunt_manager_t, create_enumerator, enumerator_t*,
	private_shunt_manager_t *this)
{
	this->lock->read_lock(this->lock);
	return enumerator_create_filter(
							this->shunts->create_enumerator(this->shunts),
							filter_entries, this->lock,
							(void*)this->lock->unlock);
}

METHOD(shunt_manager_t, flush, void,
	private_shunt_manager_t *this)
{
	entry_t *entry;

	this->lock->write_lock(this->lock);
	while (this->installing)
	{
		this->condvar->wait(this->condvar, this->lock);
	}
	while (this->shunts->remove_last(this->shunts, (void**)&entry) == SUCCESS)
	{
		uninstall_shunt_policy(entry->cfg);
		entry_destroy(entry);
	}
	this->installing = INSTALL_DISABLED;
	this->lock->unlock(this->lock);
}

METHOD(shunt_manager_t, destroy, void,
	private_shunt_manager_t *this)
{
	this->shunts->destroy_offset(this->shunts, offsetof(child_cfg_t, destroy));
	this->lock->destroy(this->lock);
	this->condvar->destroy(this->condvar);
	free(this);
}

/**
 * See header
 */
shunt_manager_t *shunt_manager_create()
{
	private_shunt_manager_t *this;

	INIT(this,
		.public = {
			.install = _install,
			.uninstall = _uninstall,
			.create_enumerator = _create_enumerator,
			.flush = _flush,
			.destroy = _destroy,
		},
		.shunts = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.condvar = rwlock_condvar_create(),
	);

	return &this->public;
}
