/*
 * Copyright (C) 2008 Martin Willi
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

#include "load_tester_plugin.h"
#include "load_tester_config.h"
#include "load_tester_creds.h"
#include "load_tester_ipsec.h"
#include "load_tester_listener.h"
#include "load_tester_control.h"
#include "load_tester_diffie_hellman.h"

#include <unistd.h>

#include <daemon.h>
#include <processing/jobs/callback_job.h>
#include <threading/condvar.h>
#include <threading/mutex.h>

typedef struct private_load_tester_plugin_t private_load_tester_plugin_t;

/**
 * private data of load_tester plugin
 */
struct private_load_tester_plugin_t {

	/**
	 * implements plugin interface
	 */
	load_tester_plugin_t public;

	/**
	 * load_tester configuration backend
	 */
	load_tester_config_t *config;

	/**
	 * load_tester credential set implementation
	 */
	load_tester_creds_t *creds;

	/**
	 * Unix control socket to initiate load-tests
	 */
	load_tester_control_t *control;

	/**
	 * event handler, listens on bus
	 */
	load_tester_listener_t *listener;

	/**
	 * number of iterations per thread
	 */
	int iterations;

	/**
	 * number desired initiator threads
	 */
	int initiators;

	/**
	 * currently running initiators
	 */
	int running;

	/**
	 * delay between initiations, in ms
	 */
	int delay;

	/**
	 * Throttle initiation if half-open IKE_SA count reached
	 */
	int init_limit;

	/**
	 * mutex to lock running field
	 */
	mutex_t *mutex;

	/**
	 * condvar to wait for initiators
	 */
	condvar_t *condvar;
};

/**
 * Begin the load test
 */
static job_requeue_t do_load_test(private_load_tester_plugin_t *this)
{
	int i, s = 0, ms = 0;

	this->mutex->lock(this->mutex);
	this->running++;
	this->mutex->unlock(this->mutex);
	if (this->delay)
	{
		s = this->delay / 1000;
		ms = this->delay % 1000;
	}

	for (i = 0; this->iterations == 0 || i < this->iterations; i++)
	{
		peer_cfg_t *peer_cfg;
		child_cfg_t *child_cfg = NULL;
		enumerator_t *enumerator;

		if (this->init_limit)
		{
			while ((charon->ike_sa_manager->get_count(charon->ike_sa_manager) -
						this->listener->get_established(this->listener)) >
					this->init_limit)
			{
				if (s)
				{
					sleep(s);
				}
				if (ms)
				{
					usleep(ms * 1000);
				}
			}
		}

		peer_cfg = charon->backends->get_peer_cfg_by_name(charon->backends,
														  "load-test");
		if (!peer_cfg)
		{
			break;
		}
		enumerator = peer_cfg->create_child_cfg_enumerator(peer_cfg);
		if (!enumerator->enumerate(enumerator, &child_cfg))
		{
			enumerator->destroy(enumerator);
			break;
		}
		enumerator->destroy(enumerator);

		charon->controller->initiate(charon->controller,
					peer_cfg, child_cfg->get_ref(child_cfg),
					NULL, NULL, 0, FALSE);
		if (s)
		{
			sleep(s);
		}
		if (ms)
		{
			usleep(ms * 1000);
		}
	}
	this->mutex->lock(this->mutex);
	this->running--;
	this->condvar->signal(this->condvar);
	this->mutex->unlock(this->mutex);
	return JOB_REQUEUE_NONE;
}

METHOD(plugin_t, get_name, char*,
	private_load_tester_plugin_t *this)
{
	return "load-tester";
}

/**
 * Register load_tester plugin features
 */
static bool register_load_tester(private_load_tester_plugin_t *this,
								 plugin_feature_t *feature, bool reg, void *data)
{
	if (reg)
	{
		u_int i, shutdown_on = 0;

		this->config = load_tester_config_create();
		this->creds = load_tester_creds_create();
		this->control = load_tester_control_create();

		charon->backends->add_backend(charon->backends, &this->config->backend);
		lib->credmgr->add_set(lib->credmgr, &this->creds->credential_set);

		if (lib->settings->get_bool(lib->settings,
				"%s.plugins.load-tester.shutdown_when_complete", 0, lib->ns))
		{
			shutdown_on = this->iterations * this->initiators;
		}
		this->listener = load_tester_listener_create(shutdown_on, this->config);
		charon->bus->add_listener(charon->bus, &this->listener->listener);

		for (i = 0; i < this->initiators; i++)
		{
			lib->processor->queue_job(lib->processor, (job_t*)
				callback_job_create_with_prio((callback_job_cb_t)do_load_test,
										this, NULL, NULL, JOB_PRIO_CRITICAL));
		}
	}
	else
	{
		this->iterations = -1;
		this->mutex->lock(this->mutex);
		while (this->running)
		{
			this->condvar->wait(this->condvar, this->mutex);
		}
		this->mutex->unlock(this->mutex);
		charon->backends->remove_backend(charon->backends, &this->config->backend);
		lib->credmgr->remove_set(lib->credmgr, &this->creds->credential_set);
		charon->bus->remove_listener(charon->bus, &this->listener->listener);
		this->config->destroy(this->config);
		this->creds->destroy(this->creds);
		this->listener->destroy(this->listener);
		this->control->destroy(this->control);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_load_tester_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(DH, load_tester_diffie_hellman_create),
			PLUGIN_PROVIDE(DH, MODP_NULL),
				PLUGIN_DEPENDS(CUSTOM, "load-tester"),
		PLUGIN_CALLBACK((plugin_feature_callback_t)register_load_tester, NULL),
			PLUGIN_PROVIDE(CUSTOM, "load-tester"),
				PLUGIN_DEPENDS(CUSTOM, "kernel-net"),
				PLUGIN_SDEPEND(PRIVKEY, KEY_RSA),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_ANY),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_X509),
		PLUGIN_CALLBACK(kernel_ipsec_register, load_tester_ipsec_create),
			PLUGIN_PROVIDE(CUSTOM, "kernel-ipsec"),
	};
	int count = countof(f);

	*features = f;

	if (!lib->settings->get_bool(lib->settings,
			"%s.plugins.load-tester.fake_kernel", FALSE, lib->ns))
	{
		count -= 2;
	}
	return count;
}

METHOD(plugin_t, destroy, void,
	private_load_tester_plugin_t *this)
{
	this->mutex->destroy(this->mutex);
	this->condvar->destroy(this->condvar);
	free(this);
}

/*
 * see header file
 */
plugin_t *load_tester_plugin_create()
{
	private_load_tester_plugin_t *this;

	if (!lib->settings->get_bool(lib->settings, "%s.plugins.load-tester.enable",
								 FALSE, lib->ns))
	{
		DBG1(DBG_CFG, "disabling load-tester plugin, not configured");
		return NULL;
	}

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.reload = (void*)return_false,
				.destroy = _destroy,
			},
		},
		.delay = lib->settings->get_int(lib->settings,
							"%s.plugins.load-tester.delay", 0, lib->ns),
		.iterations = lib->settings->get_int(lib->settings,
							"%s.plugins.load-tester.iterations", 1, lib->ns),
		.initiators = lib->settings->get_int(lib->settings,
							"%s.plugins.load-tester.initiators", 0, lib->ns),
		.init_limit = lib->settings->get_int(lib->settings,
							"%s.plugins.load-tester.init_limit", 0, lib->ns),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.condvar = condvar_create(CONDVAR_TYPE_DEFAULT),
	);
	return &this->public.plugin;
}
