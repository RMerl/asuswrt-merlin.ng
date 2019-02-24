/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "actions.h"
#include "conftest.h"

#include <daemon.h>
#include <processing/jobs/callback_job.h>
#include <processing/jobs/rekey_ike_sa_job.h>
#include <processing/jobs/rekey_child_sa_job.h>
#include <processing/jobs/send_dpd_job.h>

typedef struct private_actions_t private_actions_t;

/**
 * Private data of an actions_t object.
 */
struct private_actions_t {

	/**
	 * Public actions_t interface.
	 */
	actions_t public;
};

/**
 * Initiate a CHILD_SA
 */
static job_requeue_t initiate(char *config)
{
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg = NULL, *current;
	enumerator_t *enumerator;

	peer_cfg = charon->backends->get_peer_cfg_by_name(charon->backends, config);
	if (!peer_cfg)
	{
		DBG1(DBG_CFG, "initiating '%s' failed, config not found", config);
		return JOB_REQUEUE_NONE;
	}
	enumerator = peer_cfg->create_child_cfg_enumerator(peer_cfg);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (streq(current->get_name(current), config))
		{
			child_cfg = current;
			child_cfg->get_ref(child_cfg);
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (child_cfg)
	{
		DBG1(DBG_CFG, "initiating IKE_SA for CHILD_SA config '%s'", config);
		charon->controller->initiate(charon->controller, peer_cfg, child_cfg,
									 NULL, NULL, 0, FALSE);
	}
	else
	{
		DBG1(DBG_CFG, "initiating '%s' failed, CHILD_SA config not found",
			 config);
	}

	return JOB_REQUEUE_NONE;
}

/**
 * Rekey an IKE_SA
 */
static job_requeue_t rekey_ike(char *config)
{
	enumerator_t *enumerator;
	job_t *job = NULL;
	ike_sa_t *ike_sa;

	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		if (strcaseeq(config, ike_sa->get_name(ike_sa)))
		{
			job = (job_t*)rekey_ike_sa_job_create(ike_sa->get_id(ike_sa), FALSE);
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (job)
	{
		DBG1(DBG_CFG, "starting rekey of IKE_SA '%s'", config);
		lib->processor->queue_job(lib->processor, job);
	}
	else
	{
		DBG1(DBG_CFG, "rekeying '%s' failed, IKE_SA not found", config);
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Rekey an CHILD_SA
 */
static job_requeue_t rekey_child(char *config)
{
	enumerator_t *enumerator, *children;
	ike_sa_t *ike_sa;
	child_sa_t *child_sa;
	uint32_t spi, proto;
	host_t *dst = NULL;

	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		children = ike_sa->create_child_sa_enumerator(ike_sa);
		while (children->enumerate(children, &child_sa))
		{
			if (streq(config, child_sa->get_name(child_sa)))
			{
				dst = ike_sa->get_my_host(ike_sa);
				dst = dst->clone(dst);
				proto = child_sa->get_protocol(child_sa);
				spi = child_sa->get_spi(child_sa, TRUE);
				break;
			}
		}
		children->destroy(children);
	}
	enumerator->destroy(enumerator);
	if (dst)
	{
		DBG1(DBG_CFG, "starting rekey of CHILD_SA '%s'", config);
		lib->processor->queue_job(lib->processor,
						(job_t*)rekey_child_sa_job_create(proto, spi, dst));
		dst->destroy(dst);
	}
	else
	{
		DBG1(DBG_CFG, "rekeying '%s' failed, CHILD_SA not found", config);
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Do a liveness check
 */
static job_requeue_t liveness(char *config)
{
	enumerator_t *enumerator;
	job_t *job = NULL;
	ike_sa_t *ike_sa;

	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		if (strcaseeq(config, ike_sa->get_name(ike_sa)))
		{
			job = (job_t*)send_dpd_job_create(ike_sa->get_id(ike_sa));
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (job)
	{
		DBG1(DBG_CFG, "starting liveness check of IKE_SA '%s'", config);
		lib->processor->queue_job(lib->processor, job);
	}
	else
	{
		DBG1(DBG_CFG, "liveness check for '%s' failed, IKE_SA not found", config);
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Close an IKE_SA with all CHILD_SAs
 */
static job_requeue_t close_ike(char *config)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	int id = 0;

	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		if (strcaseeq(config, ike_sa->get_name(ike_sa)))
		{
			id = ike_sa->get_unique_id(ike_sa);
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (id)
	{
		DBG1(DBG_CFG, "closing IKE_SA '%s'", config);
		charon->controller->terminate_ike(charon->controller, id, FALSE, NULL,
										  NULL, 0);
	}
	else
	{
		DBG1(DBG_CFG, "unable to close IKE_SA '%s', not found", config);
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Close a CHILD_SAs
 */
static job_requeue_t close_child(char *config)
{
	enumerator_t *enumerator, *children;
	ike_sa_t *ike_sa;
	child_sa_t *child_sa;
	int id = 0;

	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{

		children = ike_sa->create_child_sa_enumerator(ike_sa);
		while (children->enumerate(children, (void**)&child_sa))
		{
			if (streq(config, child_sa->get_name(child_sa)))
			{
				id = child_sa->get_unique_id(child_sa);
				break;
			}
		}
		children->destroy(children);
	}
	enumerator->destroy(enumerator);
	if (id)
	{
		DBG1(DBG_CFG, "closing CHILD_SA '%s'", config);
		charon->controller->terminate_child(charon->controller, id,
											NULL, NULL, 0);
	}
	else
	{
		DBG1(DBG_CFG, "unable to close CHILD_SA '%s', not found", config);
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Load a single action
 */
static void load_action(settings_t *settings, char *action)
{
	static struct {
		char *name;
		callback_job_cb_t cb;
	} actions[] = {
		{"initiate",		(void*)initiate},
		{"rekey_ike",		(void*)rekey_ike},
		{"rekey_child",		(void*)rekey_child},
		{"liveness",		(void*)liveness},
		{"close_ike",		(void*)close_ike},
		{"close_child",		(void*)close_child},
	};
	bool found = FALSE;
	int i;

	for (i = 0; i < countof(actions); i++)
	{
		if (strncaseeq(actions[i].name, action, strlen(actions[i].name)))
		{
			int delay;
			char *config;

			found = TRUE;
			delay = settings->get_int(settings, "actions.%s.delay", 0, action);
			config = settings->get_str(settings, "actions.%s.config",
									   NULL, action);
			if (!config)
			{
				DBG1(DBG_CFG, "no config defined for action '%s'", action);
				break;
			}
			lib->scheduler->schedule_job(lib->scheduler,
				(job_t*)callback_job_create(actions[i].cb, config, NULL, NULL),
				delay);
		}
	}
	if (!found)
	{
		DBG1(DBG_CFG, "unknown action '%s', skipped", action);
	}
}

/**
 * Load configured actions
 */
static void load_actions(settings_t *settings)
{
	enumerator_t *enumerator;
	char *action;

	enumerator = settings->create_section_enumerator(settings, "actions");
	while (enumerator->enumerate(enumerator, &action))
	{
		load_action(settings, action);
	}
	enumerator->destroy(enumerator);
}

METHOD(actions_t, destroy, void,
	private_actions_t *this)
{
	free(this);
}

/**
 * See header
 */
actions_t *actions_create()
{
	private_actions_t *this;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
	);

	load_actions(conftest->test);

	return &this->public;
}
