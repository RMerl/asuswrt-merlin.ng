/*
 * Copyright (C) 2007-2008 Tobias Brunner
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

#include "initiate_mediation_job.h"

#include <sa/ike_sa.h>
#include <daemon.h>


typedef struct private_initiate_mediation_job_t private_initiate_mediation_job_t;

/**
 * Private data of an initiate_mediation_job_t Object
 */
struct private_initiate_mediation_job_t {
	/**
	 * public initiate_mediation_job_t interface
	 */
	initiate_mediation_job_t public;

	/**
	 * ID of the IKE_SA of the mediated connection.
	 */
	ike_sa_id_t *mediated_sa_id;

	/**
	 * ID of the IKE_SA of the mediation connection.
	 */
	ike_sa_id_t *mediation_sa_id;
};

METHOD(job_t, destroy, void,
	private_initiate_mediation_job_t *this)
{
	DESTROY_IF(this->mediation_sa_id);
	DESTROY_IF(this->mediated_sa_id);
	free(this);
}

/**
 * Callback to handle initiation of mediation connection
 */
static bool initiate_callback(private_initiate_mediation_job_t *this,
			debug_t group, level_t level, ike_sa_t *ike_sa,
			char *message)
{
	if (ike_sa && !this->mediation_sa_id)
	{
		this->mediation_sa_id = ike_sa->get_id(ike_sa);
		this->mediation_sa_id = this->mediation_sa_id->clone(this->mediation_sa_id);
	}
	return TRUE;
}

METHOD(job_t, initiate, job_requeue_t,
	private_initiate_mediation_job_t *this)
{
	ike_sa_t *mediated_sa, *mediation_sa;
	peer_cfg_t *mediated_cfg, *mediation_cfg;
	enumerator_t *enumerator;
	auth_cfg_t *auth_cfg;

	mediated_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
												   this->mediated_sa_id);
	if (mediated_sa)
	{
		DBG1(DBG_IKE, "initiating mediation connection");
		mediated_cfg = mediated_sa->get_peer_cfg(mediated_sa);
		mediated_cfg->get_ref(mediated_cfg);

		charon->ike_sa_manager->checkin(charon->ike_sa_manager, mediated_sa);

		mediation_cfg = charon->backends->get_peer_cfg_by_name(charon->backends,
								mediated_cfg->get_mediated_by(mediated_cfg));
		if (!mediation_cfg)
		{
			DBG1(DBG_IKE, "mediation connection '%s' not found, aborting",
				 mediated_cfg->get_mediated_by(mediated_cfg));
			mediated_cfg->destroy(mediated_cfg);
			return JOB_REQUEUE_NONE;
		}
		if (!mediation_cfg->is_mediation(mediation_cfg))
		{
			DBG1(DBG_CFG, "connection '%s' as referred to by '%s' is no "
				 "mediation connection, aborting",
				 mediated_cfg->get_mediated_by(mediated_cfg),
				 mediated_cfg->get_name(mediated_cfg));
			mediated_cfg->destroy(mediated_cfg);
			mediation_cfg->destroy(mediation_cfg);
			return JOB_REQUEUE_NONE;
		}

		enumerator = mediation_cfg->create_auth_cfg_enumerator(mediation_cfg,
															   TRUE);
		if (!enumerator->enumerate(enumerator, &auth_cfg) ||
			auth_cfg->get(auth_cfg, AUTH_RULE_IDENTITY) == NULL)
		{
			mediated_cfg->destroy(mediated_cfg);
			mediation_cfg->destroy(mediation_cfg);
			enumerator->destroy(enumerator);
			return JOB_REQUEUE_NONE;
		}
		enumerator->destroy(enumerator);

		if (charon->connect_manager->check_and_register(charon->connect_manager,
				auth_cfg->get(auth_cfg, AUTH_RULE_IDENTITY),
				mediated_cfg->get_peer_id(mediated_cfg),
				this->mediated_sa_id))
		{
			mediated_cfg->destroy(mediated_cfg);
			mediation_cfg->destroy(mediation_cfg);

			mediated_sa = charon->ike_sa_manager->checkout(
								charon->ike_sa_manager, this->mediated_sa_id);
			if (mediated_sa)
			{
				DBG1(DBG_IKE, "mediation with the same peer is already in "
					 "progress, queued");
				charon->ike_sa_manager->checkin(
								charon->ike_sa_manager, mediated_sa);
			}
			return JOB_REQUEUE_NONE;
		}
		/* we need an additional reference because initiate consumes one */
		mediation_cfg->get_ref(mediation_cfg);

		if (charon->controller->initiate(charon->controller, mediation_cfg, NULL,
				(controller_cb_t)initiate_callback, this, 0, FALSE) != SUCCESS)
		{
			mediation_cfg->destroy(mediation_cfg);
			mediated_cfg->destroy(mediated_cfg);
			mediated_sa = charon->ike_sa_manager->checkout(
								charon->ike_sa_manager, this->mediated_sa_id);
			if (mediated_sa)
			{
				DBG1(DBG_IKE, "initiating mediation connection failed");
				charon->ike_sa_manager->checkin_and_destroy(
									charon->ike_sa_manager, mediated_sa);
			}
			return JOB_REQUEUE_NONE;
		}
		mediation_cfg->destroy(mediation_cfg);

		mediation_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
														this->mediation_sa_id);
		if (mediation_sa)
		{
			if (mediation_sa->initiate_mediation(mediation_sa,
												 mediated_cfg) != SUCCESS)
			{
				mediated_cfg->destroy(mediated_cfg);
				charon->ike_sa_manager->checkin_and_destroy(
								charon->ike_sa_manager, mediation_sa);
				mediated_sa = charon->ike_sa_manager->checkout(
								charon->ike_sa_manager, this->mediated_sa_id);
				if (mediated_sa)
				{
					DBG1(DBG_IKE, "establishing mediation connection failed");
					charon->ike_sa_manager->checkin_and_destroy(
										charon->ike_sa_manager, mediated_sa);
				}
				return JOB_REQUEUE_NONE;
			}
			charon->ike_sa_manager->checkin(charon->ike_sa_manager,
											mediation_sa);
		}
		mediated_cfg->destroy(mediated_cfg);
	}
	else
	{	/* newly created IKE_SA is not checked in yet, try again */
		return JOB_RESCHEDULE_MS(100);
	}
	return JOB_REQUEUE_NONE;
}

METHOD(job_t, reinitiate, job_requeue_t,
	private_initiate_mediation_job_t *this)
{
	ike_sa_t *mediated_sa, *mediation_sa;
	peer_cfg_t *mediated_cfg;

	mediated_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
												   this->mediated_sa_id);
	if (mediated_sa)
	{
		mediated_cfg = mediated_sa->get_peer_cfg(mediated_sa);
		mediated_cfg->get_ref(mediated_cfg);
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, mediated_sa);

		mediation_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
														this->mediation_sa_id);
		if (mediation_sa)
		{
			if (mediation_sa->initiate_mediation(mediation_sa,
												 mediated_cfg) != SUCCESS)
			{
				DBG1(DBG_JOB, "initiating mediated connection '%s' failed",
					 mediated_cfg->get_name(mediated_cfg));
				mediated_cfg->destroy(mediated_cfg);
				charon->ike_sa_manager->checkin_and_destroy(
										charon->ike_sa_manager,
										mediation_sa);
				mediated_sa = charon->ike_sa_manager->checkout(
										charon->ike_sa_manager,
										this->mediated_sa_id);
				if (mediated_sa)
				{
					DBG1(DBG_IKE, "establishing mediation connection failed");
					charon->ike_sa_manager->checkin_and_destroy(
										charon->ike_sa_manager,
										mediated_sa);
				}
				return JOB_REQUEUE_NONE;
			}
			charon->ike_sa_manager->checkin(charon->ike_sa_manager,
											mediation_sa);
		}

		mediated_cfg->destroy(mediated_cfg);
	}
	return JOB_REQUEUE_NONE;
}

METHOD(job_t, get_priority, job_priority_t,
	private_initiate_mediation_job_t *this)
{
	return JOB_PRIO_MEDIUM;
}

/**
 * Creates an empty job
 */
static private_initiate_mediation_job_t *initiate_mediation_job_create_empty()
{
	private_initiate_mediation_job_t *this;
	INIT(this,
		.public = {
			.job_interface = {
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
		},
	);
	return this;
}

/*
 * Described in header
 */
initiate_mediation_job_t *initiate_mediation_job_create(ike_sa_id_t *ike_sa_id)
{
	private_initiate_mediation_job_t *this = initiate_mediation_job_create_empty();

	this->public.job_interface.execute = _initiate;
	this->mediated_sa_id = ike_sa_id->clone(ike_sa_id);

	return &this->public;
}

/*
 * Described in header
 */
initiate_mediation_job_t *reinitiate_mediation_job_create(ike_sa_id_t *mediation_sa_id,
		ike_sa_id_t *mediated_sa_id)
{
	private_initiate_mediation_job_t *this = initiate_mediation_job_create_empty();

	this->public.job_interface.execute = _reinitiate;
	this->mediation_sa_id = mediation_sa_id->clone(mediation_sa_id);
	this->mediated_sa_id = mediated_sa_id->clone(mediated_sa_id);

	return &this->public;
}
