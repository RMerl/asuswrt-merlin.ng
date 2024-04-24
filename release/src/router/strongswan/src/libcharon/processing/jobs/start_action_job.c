/*
 * Copyright (C) 2011 Andreas Steffen
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

#include "start_action_job.h"

#include <daemon.h>


typedef struct private_start_action_job_t private_start_action_job_t;

/**
 * Private data of an start_action_job_t object.
 */
struct private_start_action_job_t {
	/**
	 * Public start_action_job_t interface.
	 */
	start_action_job_t public;
};

METHOD(job_t, destroy, void,
	private_start_action_job_t *this)
{
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_start_action_job_t *this)
{
	enumerator_t *enumerator, *children;
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg;
	action_t action;

	enumerator = charon->backends->create_peer_cfg_enumerator(charon->backends,
											NULL, NULL, NULL, NULL, IKE_ANY);
	while (enumerator->enumerate(enumerator, &peer_cfg))
	{
		children = peer_cfg->create_child_cfg_enumerator(peer_cfg);
		while (children->enumerate(children, &child_cfg))
		{
			action = child_cfg->get_start_action(child_cfg);
			if (action == ACTION_NONE)
			{
				continue;
			}

			DBG1(DBG_JOB, "start action: %N '%s'", action_names, action,
				 child_cfg->get_name(child_cfg));

			if (action & ACTION_TRAP)
			{
				switch (child_cfg->get_mode(child_cfg))
				{
					case MODE_PASS:
					case MODE_DROP:
						charon->shunts->install(charon->shunts,
												peer_cfg->get_name(peer_cfg),
												child_cfg);
						/* no need to check for ACTION_START */
						continue;
					default:
						charon->traps->install(charon->traps, peer_cfg,
											   child_cfg);
						break;
				}
			}

			if (action & ACTION_START)
			{
				charon->controller->initiate(charon->controller,
											 peer_cfg->get_ref(peer_cfg),
											 child_cfg->get_ref(child_cfg),
											 NULL, NULL, 0, 0, FALSE);
			}
		}
		children->destroy(children);
	}
	enumerator->destroy(enumerator);
	return JOB_REQUEUE_NONE;
}

METHOD(job_t, get_priority, job_priority_t,
	private_start_action_job_t *this)
{
	return JOB_PRIO_MEDIUM;
}

/*
 * Described in header
 */
start_action_job_t *start_action_job_create(void)
{
	private_start_action_job_t *this;

	INIT(this,
		.public = {
			.job_interface = {
				.execute = _execute,
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
		},
	);
	return &this->public;
}

