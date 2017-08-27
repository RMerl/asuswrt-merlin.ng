/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include "systime_fix_plugin.h"
#include "systime_fix_validator.h"

#include <daemon.h>
#include <processing/jobs/callback_job.h>
#include <processing/jobs/delete_ike_sa_job.h>
#include <processing/jobs/rekey_ike_sa_job.h>

#include <time.h>

/**
 * Defining _XOPEN_SOURCE is difficult with libstrongswan includes,
 * declare function explicitly.
 */
char *strptime(const char *s, const char *format, struct tm *tm);

typedef struct private_systime_fix_plugin_t private_systime_fix_plugin_t;

/**
 * Private data of systime_fix plugin
 */
struct private_systime_fix_plugin_t {

	/**
	 * Implements plugin interface
	 */
	systime_fix_plugin_t public;

	/**
	 * Certificate lifetime validator
	 */
	systime_fix_validator_t *validator;

	/**
	 * Interval we check for a now-valid system time, in seconds. 0 if disabled
	 */
	u_int interval;

	/**
	 * Timestamp where we start considering system time valid
	 */
	time_t threshold;

	/**
	 * Do we trigger reauth or delete when finding expired certificates?
	 */
	bool reauth;
};

METHOD(plugin_t, get_name, char*,
	private_systime_fix_plugin_t *this)
{
	return "systime-fix";
}

/**
 * Check if all certificates associated to an IKE_SA have valid lifetimes
 */
static bool has_invalid_certs(ike_sa_t *ike_sa)
{
	enumerator_t *cfgs, *items;
	certificate_t *cert;
	auth_rule_t type;
	auth_cfg_t *auth;
	time_t not_before, not_after;
	bool valid = TRUE;

	cfgs = ike_sa->create_auth_cfg_enumerator(ike_sa, FALSE);
	while (valid && cfgs->enumerate(cfgs, &auth))
	{
		items = auth->create_enumerator(auth);
		while (valid && items->enumerate(items, &type, &cert))
		{
			switch (type)
			{
				case AUTH_RULE_SUBJECT_CERT:
				case AUTH_RULE_IM_CERT:
				case AUTH_RULE_CA_CERT:
					if (!cert->get_validity(cert, NULL, &not_before, &not_after))
					{
						DBG1(DBG_CFG, "certificate '%Y' invalid "
							"(valid from %T to %T)", cert->get_subject(cert),
							 &not_before, FALSE, &not_after, FALSE);
						valid = FALSE;
					}
					break;
				default:
					break;
			}
		}
		items->destroy(items);
	}
	cfgs->destroy(cfgs);

	if (valid)
	{
		DBG1(DBG_CFG, "all certificates have valid lifetimes");
	}
	return !valid;
}

/**
 * Check system time, reevaluate certificates
 */
static job_requeue_t check_systime(private_systime_fix_plugin_t *this)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	char *action;
	job_t *job;

	if (time(NULL) < this->threshold)
	{
		DBG2(DBG_CFG, "systime not valid, rechecking in %ds", this->interval);
		lib->scheduler->schedule_job(lib->scheduler, (job_t*)
					callback_job_create((callback_job_cb_t)check_systime, this,
										NULL, NULL), this->interval);
		return JOB_REQUEUE_NONE;
	}

	DBG1(DBG_CFG, "system time got valid, rechecking certificates");

	enumerator = charon->ike_sa_manager->create_enumerator(
												charon->ike_sa_manager, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		if (has_invalid_certs(ike_sa))
		{
			if (this->reauth)
			{
				action = "reauthenticating";
				job = &rekey_ike_sa_job_create(ike_sa->get_id(ike_sa),
											   TRUE)->job_interface;
			}
			else
			{
				action = "deleting";
				job = &delete_ike_sa_job_create(ike_sa->get_id(ike_sa),
												TRUE)->job_interface;
			}
			DBG1(DBG_CFG, "%s[%d] has certificates not valid, %s IKE_SA",
				 ike_sa->get_name(ike_sa), ike_sa->get_unique_id(ike_sa),
				 action);
			lib->processor->queue_job(lib->processor, job);
		}
	}
	enumerator->destroy(enumerator);

	return JOB_REQUEUE_NONE;
}

/**
 * Load cert lifetime validator configuration
 */
static bool load_validator(private_systime_fix_plugin_t *this)
{
	struct tm tm = {
		.tm_mday = 1,
	};
	char *str, *fmt;

	fmt = lib->settings->get_str(lib->settings,
			"%s.plugins.%s.threshold_format", "%Y", lib->ns, get_name(this));
	str = lib->settings->get_str(lib->settings,
			"%s.plugins.%s.threshold", NULL, lib->ns, get_name(this));
	if (!str)
	{
		DBG1(DBG_CFG, "no threshold configured for %s, disabled",
			 get_name(this));
		return FALSE;
	}
	if (strptime(str, fmt, &tm) == NULL)
	{
		DBG1(DBG_CFG, "threshold for %s invalid, disabled", get_name(this));
		return FALSE;
	}
	this->threshold = mktime(&tm);
	if (this->threshold == -1)
	{
		DBG1(DBG_CFG, "converting threshold for %s failed, disabled",
			 get_name(this));
		return FALSE;
	}
	if (time(NULL) >= this->threshold)
	{
		DBG1(DBG_CFG, "system time looks good, disabling %s", get_name(this));
		return FALSE;
	}

	DBG1(DBG_CFG, "enabling %s, threshold: %s", get_name(this), asctime(&tm));
	this->validator = systime_fix_validator_create(this->threshold);
	return TRUE;
}

/**
 * Load validator
 */
static bool plugin_cb(private_systime_fix_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		if (!load_validator(this))
		{
			return FALSE;
		}
		lib->credmgr->add_validator(lib->credmgr, &this->validator->validator);
		if (this->interval != 0)
		{
			DBG1(DBG_CFG, "starting systime check, interval: %ds",
				 this->interval);
			lib->scheduler->schedule_job(lib->scheduler, (job_t*)
					callback_job_create((callback_job_cb_t)check_systime,
										this, NULL, NULL), this->interval);
		}
	}
	else
	{
		lib->credmgr->remove_validator(lib->credmgr,
									   &this->validator->validator);
		this->validator->destroy(this->validator);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_systime_fix_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "systime-fix"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_systime_fix_plugin_t *this)
{
	free(this);
}

/**
 * Plugin constructor
 */
plugin_t *systime_fix_plugin_create()
{
	private_systime_fix_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.interval = lib->settings->get_int(lib->settings,
						"%s.plugins.%s.interval", 0, lib->ns, get_name(this)),
		.reauth = lib->settings->get_bool(lib->settings,
						"%s.plugins.%s.reauth", FALSE, lib->ns, get_name(this)),
	);

	return &this->public.plugin;
}
