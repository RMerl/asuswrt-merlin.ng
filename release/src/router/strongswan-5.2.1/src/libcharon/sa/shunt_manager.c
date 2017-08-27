/*
 * Copyright (C) 2011 Andreas Steffen
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

#include <hydra.h>
#include <daemon.h>
#include <threading/rwlock.h>
#include <collections/linked_list.h>


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
	 * Installed shunts, as child_cfg_t
	 */
	linked_list_t *shunts;
};

/**
 * Install in and out shunt policies in the kernel
 */
static bool install_shunt_policy(child_cfg_t *child)
{
	enumerator_t *e_my_ts, *e_other_ts;
	linked_list_t *my_ts_list, *other_ts_list;
	traffic_selector_t *my_ts, *other_ts;
	host_t *host_any;
	policy_type_t policy_type;
	policy_priority_t policy_prio;
	status_t status = SUCCESS;
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

	my_ts_list =    child->get_traffic_selectors(child, TRUE,  NULL, NULL);
	other_ts_list = child->get_traffic_selectors(child, FALSE, NULL, NULL);
	host_any = host_create_any(AF_INET);

	/* enumerate pairs of traffic selectors */
	e_my_ts = my_ts_list->create_enumerator(my_ts_list);
	while (e_my_ts->enumerate(e_my_ts, &my_ts))
	{
		e_other_ts = other_ts_list->create_enumerator(other_ts_list);
		while (e_other_ts->enumerate(e_other_ts, &other_ts))
		{
			/* install out policy */
			status |= hydra->kernel_interface->add_policy(
								hydra->kernel_interface, host_any, host_any,
								my_ts, other_ts, POLICY_OUT, policy_type,
								&sa, child->get_mark(child, FALSE),
								policy_prio);

			/* install in policy */
			status |= hydra->kernel_interface->add_policy(
								hydra->kernel_interface, host_any, host_any,
								other_ts, my_ts, POLICY_IN, policy_type,
								&sa, child->get_mark(child, TRUE),
								policy_prio);

			/* install forward policy */
			status |= hydra->kernel_interface->add_policy(
								hydra->kernel_interface, host_any, host_any,
								other_ts, my_ts, POLICY_FWD, policy_type,
								&sa, child->get_mark(child, TRUE),
								policy_prio);
		}
		e_other_ts->destroy(e_other_ts);
	}
	e_my_ts->destroy(e_my_ts);

	my_ts_list->destroy_offset(my_ts_list,
							   offsetof(traffic_selector_t, destroy));
	other_ts_list->destroy_offset(other_ts_list,
							   offsetof(traffic_selector_t, destroy));
	host_any->destroy(host_any);

	return status == SUCCESS;
}

METHOD(shunt_manager_t, install, bool,
	private_shunt_manager_t *this, child_cfg_t *child)
{
	enumerator_t *enumerator;
	child_cfg_t *child_cfg;
	bool found = FALSE;

	/* check if not already installed */
	enumerator = this->shunts->create_enumerator(this->shunts);
	while (enumerator->enumerate(enumerator, &child_cfg))
	{
		if (streq(child_cfg->get_name(child_cfg), child->get_name(child)))
		{
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (found)
	{
		DBG1(DBG_CFG, "shunt %N policy '%s' already installed",
			 ipsec_mode_names, child->get_mode(child), child->get_name(child));
		return TRUE;
	}
	this->shunts->insert_last(this->shunts, child->get_ref(child));

	return install_shunt_policy(child);
}

/**
 * Uninstall in and out shunt policies in the kernel
 */
static void uninstall_shunt_policy(child_cfg_t *child)
{
	enumerator_t *e_my_ts, *e_other_ts;
	linked_list_t *my_ts_list, *other_ts_list;
	traffic_selector_t *my_ts, *other_ts;
	policy_priority_t policy_prio;
	status_t status = SUCCESS;

	switch (child->get_mode(child))
	{
		case MODE_PASS:
			policy_prio = POLICY_PRIORITY_PASS;
			break;
		case MODE_DROP:
			policy_prio = POLICY_PRIORITY_FALLBACK;
			break;
		default:
			return;
	}

	my_ts_list =    child->get_traffic_selectors(child, TRUE,  NULL, NULL);
	other_ts_list = child->get_traffic_selectors(child, FALSE, NULL, NULL);

	/* enumerate pairs of traffic selectors */
	e_my_ts = my_ts_list->create_enumerator(my_ts_list);
	while (e_my_ts->enumerate(e_my_ts, &my_ts))
	{
		e_other_ts = other_ts_list->create_enumerator(other_ts_list);
		while (e_other_ts->enumerate(e_other_ts, &other_ts))
		{
			/* uninstall out policy */
			status |= hydra->kernel_interface->del_policy(
							hydra->kernel_interface, my_ts, other_ts,
							POLICY_OUT, 0, child->get_mark(child, FALSE),
							policy_prio);

			/* uninstall in policy */
			status |= hydra->kernel_interface->del_policy(
							hydra->kernel_interface, other_ts, my_ts,
							POLICY_IN, 0, child->get_mark(child, TRUE),
							policy_prio);

			/* uninstall forward policy */
			status |= hydra->kernel_interface->del_policy(
							hydra->kernel_interface, other_ts, my_ts,
							POLICY_FWD, 0, child->get_mark(child, TRUE),
							policy_prio);
		}
		e_other_ts->destroy(e_other_ts);
	}
	e_my_ts->destroy(e_my_ts);

	my_ts_list->destroy_offset(my_ts_list,
							   offsetof(traffic_selector_t, destroy));
	other_ts_list->destroy_offset(other_ts_list,
							   offsetof(traffic_selector_t, destroy));

	if (status != SUCCESS)
	{
		DBG1(DBG_CFG, "uninstalling shunt %N 'policy %s' failed",
			 ipsec_mode_names, child->get_mode(child), child->get_name(child));
	}
}

METHOD(shunt_manager_t, uninstall, bool,
	private_shunt_manager_t *this, char *name)
{
	enumerator_t *enumerator;
	child_cfg_t *child, *found = NULL;

	enumerator = this->shunts->create_enumerator(this->shunts);
	while (enumerator->enumerate(enumerator, &child))
	{
		if (streq(name, child->get_name(child)))
		{
			this->shunts->remove_at(this->shunts, enumerator);
			found = child;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (!found)
	{
		return FALSE;
	}
	uninstall_shunt_policy(child);
	child->destroy(child);
	return TRUE;
}

METHOD(shunt_manager_t, create_enumerator, enumerator_t*,
	private_shunt_manager_t *this)
{
	return this->shunts->create_enumerator(this->shunts);
}

METHOD(shunt_manager_t, destroy, void,
	private_shunt_manager_t *this)
{
	child_cfg_t *child;

	while (this->shunts->remove_last(this->shunts, (void**)&child) == SUCCESS)
	{
		uninstall_shunt_policy(child);
		child->destroy(child);
	}
	this->shunts->destroy(this->shunts);
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
			.destroy = _destroy,
		},
		.shunts = linked_list_create(),
	);

	return &this->public;
}
