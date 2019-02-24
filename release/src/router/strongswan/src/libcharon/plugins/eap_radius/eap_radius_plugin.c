/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2009 Martin Willi
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

/*
 * Copyright (C) 2015 Thom Troy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "eap_radius_plugin.h"

#include "eap_radius.h"
#include "eap_radius_xauth.h"
#include "eap_radius_accounting.h"
#include "eap_radius_dae.h"
#include "eap_radius_forward.h"
#include "eap_radius_provider.h"

#include <radius_client.h>
#include <radius_config.h>

#include <daemon.h>
#include <threading/rwlock.h>
#include <processing/jobs/callback_job.h>
#include <processing/jobs/delete_ike_sa_job.h>

/**
 * Default RADIUS server port for authentication
 */
#define AUTH_PORT 1812

/**
 * Default RADIUS server port for accounting
 */
#define ACCT_PORT 1813

typedef struct private_eap_radius_plugin_t private_eap_radius_plugin_t;

/**
 * Private data of an eap_radius_plugin_t object.
 */
struct private_eap_radius_plugin_t {

	/**
	 * Public radius_plugin_t interface.
	 */
	eap_radius_plugin_t public;

	/**
	 * List of RADIUS server configurations
	 */
	linked_list_t *configs;

	/**
	 * Lock for configs list
	 */
	rwlock_t *lock;

	/**
	 * RADIUS sessions for accounting
	 */
	eap_radius_accounting_t *accounting;

	/**
	 * IKE attribute provider
	 */
	eap_radius_provider_t *provider;

	/**
	 * Dynamic authorization extensions
	 */
	eap_radius_dae_t *dae;

	/**
	 * RADIUS <-> IKE attribute forwarding
	 */
	eap_radius_forward_t *forward;
};

/**
 * Instance of the EAP plugin
 */
static private_eap_radius_plugin_t *instance = NULL;

/**
 * Load RADIUS servers from configuration
 */
static void load_configs(private_eap_radius_plugin_t *this)
{
	enumerator_t *enumerator;
	radius_config_t *config;
	char *nas_identifier, *secret, *address, *section;
	int auth_port, acct_port, sockets, preference;
	u_int retransmit_tries;
	double retransmit_timeout, retransmit_base;

	address = lib->settings->get_str(lib->settings,
								"%s.plugins.eap-radius.server", NULL, lib->ns);
	if (address)
	{	/* legacy configuration */
		secret = lib->settings->get_str(lib->settings,
								"%s.plugins.eap-radius.secret", NULL, lib->ns);
		if (!secret)
		{
			DBG1(DBG_CFG, "no RADIUS secret defined");
			return;
		}
		nas_identifier = lib->settings->get_str(lib->settings,
						"%s.plugins.eap-radius.nas_identifier", "strongSwan",
						lib->ns);
		auth_port = lib->settings->get_int(lib->settings,
						"%s.plugins.eap-radius.port", AUTH_PORT, lib->ns);
		sockets = lib->settings->get_int(lib->settings,
						"%s.plugins.eap-radius.sockets", 1, lib->ns);

		retransmit_tries = lib->settings->get_int(lib->settings,
						"%s.plugins.eap-radius.retransmit_tries", 4, lib->ns);
		retransmit_timeout = lib->settings->get_double(lib->settings,
						"%s.plugins.eap-radius.retransmit_timeout", 2, lib->ns);
		retransmit_base = lib->settings->get_double(lib->settings,
						"%s.plugins.eap-radius.retransmit_base", 1.4, lib->ns);

		config = radius_config_create(address, address, auth_port, ACCT_PORT,
									  nas_identifier, secret, sockets, 0,
									  retransmit_tries, retransmit_timeout,
									  retransmit_base);
		if (!config)
		{
			DBG1(DBG_CFG, "no RADUIS server defined");
			return;
		}
		this->configs->insert_last(this->configs, config);
		return;
	}

	enumerator = lib->settings->create_section_enumerator(lib->settings,
									"%s.plugins.eap-radius.servers", lib->ns);
	while (enumerator->enumerate(enumerator, &section))
	{
		address = lib->settings->get_str(lib->settings,
							"%s.plugins.eap-radius.servers.%s.address", NULL,
							lib->ns, section);
		if (!address)
		{
			DBG1(DBG_CFG, "RADIUS server '%s' misses address, skipped", section);
			continue;
		}
		secret = lib->settings->get_str(lib->settings,
							"%s.plugins.eap-radius.servers.%s.secret", NULL,
							lib->ns, section);
		if (!secret)
		{
			DBG1(DBG_CFG, "RADIUS server '%s' misses secret, skipped", section);
			continue;
		}
		nas_identifier = lib->settings->get_str(lib->settings,
				"%s.plugins.eap-radius.servers.%s.nas_identifier",
					lib->settings->get_str(lib->settings,
						"%s.plugins.eap-radius.nas_identifier", "strongSwan",
						lib->ns),
				lib->ns, section);
		auth_port = lib->settings->get_int(lib->settings,
			"%s.plugins.eap-radius.servers.%s.auth_port",
				lib->settings->get_int(lib->settings,
					"%s.plugins.eap-radius.servers.%s.port",
						lib->settings->get_int(lib->settings,
							"%s.plugins.eap-radius.port", AUTH_PORT, lib->ns),
					lib->ns, section),
			lib->ns, section);
		acct_port = lib->settings->get_int(lib->settings,
				"%s.plugins.eap-radius.servers.%s.acct_port", ACCT_PORT,
				lib->ns, section);
		sockets = lib->settings->get_int(lib->settings,
				"%s.plugins.eap-radius.servers.%s.sockets",
					lib->settings->get_int(lib->settings,
						"%s.plugins.eap-radius.sockets", 1, lib->ns),
				lib->ns, section);

		retransmit_tries = lib->settings->get_int(lib->settings,
				"%s.plugins.eap-radius.servers.%s.retransmit_tries",
					lib->settings->get_int(lib->settings,
						"%s.plugins.eap-radius.retransmit_tries", 4, lib->ns),
				lib->ns, section);

		retransmit_timeout = lib->settings->get_double(lib->settings,
				"%s.plugins.eap-radius.servers.%s.retransmit_timeout",
					lib->settings->get_double(lib->settings,
						"%s.plugins.eap-radius.retransmit_timeout", 2, lib->ns),
				lib->ns, section);

		retransmit_base = lib->settings->get_double(lib->settings,
				"%s.plugins.eap-radius.servers.%s.retransmit_base",
					lib->settings->get_double(lib->settings,
						"%s.plugins.eap-radius.retransmit_base", 1.4, lib->ns),
				lib->ns, section);

		preference = lib->settings->get_int(lib->settings,
				"%s.plugins.eap-radius.servers.%s.preference", 0,
				lib->ns, section);

		config = radius_config_create(section, address, auth_port, acct_port,
								nas_identifier, secret, sockets, preference,
								retransmit_tries, retransmit_timeout,
								retransmit_base);
		if (!config)
		{
			DBG1(DBG_CFG, "loading RADIUS server '%s' failed, skipped", section);
			continue;
		}
		this->configs->insert_last(this->configs, config);
	}
	enumerator->destroy(enumerator);

	DBG1(DBG_CFG, "loaded %d RADIUS server configuration%s",
		 this->configs->get_count(this->configs),
		 this->configs->get_count(this->configs) == 1 ? "" : "s");
}

METHOD(plugin_t, get_name, char*,
	private_eap_radius_plugin_t *this)
{
	return "eap-radius";
}

/**
 * Register listener
 */
static bool plugin_cb(private_eap_radius_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		this->accounting = eap_radius_accounting_create();
		this->forward = eap_radius_forward_create();
		this->provider = eap_radius_provider_create();

		load_configs(this);

		if (lib->settings->get_bool(lib->settings,
						"%s.plugins.eap-radius.dae.enable", FALSE, lib->ns))
		{
			this->dae = eap_radius_dae_create(this->accounting);
		}
		if (this->forward)
		{
			charon->bus->add_listener(charon->bus, &this->forward->listener);
		}
		charon->attributes->add_provider(charon->attributes,
										 &this->provider->provider);
	}
	else
	{
		charon->attributes->remove_provider(charon->attributes,
											&this->provider->provider);
		if (this->forward)
		{
			charon->bus->remove_listener(charon->bus, &this->forward->listener);
			this->forward->destroy(this->forward);
		}
		DESTROY_IF(this->dae);
		this->provider->destroy(this->provider);
		this->accounting->destroy(this->accounting);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_eap_radius_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK(eap_method_register, eap_radius_create),
			PLUGIN_PROVIDE(EAP_SERVER, EAP_RADIUS),
				PLUGIN_DEPENDS(CUSTOM, "eap-radius"),
		PLUGIN_CALLBACK(xauth_method_register, eap_radius_xauth_create_server),
			PLUGIN_PROVIDE(XAUTH_SERVER, "radius"),
				PLUGIN_DEPENDS(CUSTOM, "eap-radius"),
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "eap-radius"),
				PLUGIN_DEPENDS(HASHER, HASH_MD5),
				PLUGIN_DEPENDS(SIGNER, AUTH_HMAC_MD5_128),
				PLUGIN_DEPENDS(RNG, RNG_WEAK),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, reload, bool,
	private_eap_radius_plugin_t *this)
{
	this->lock->write_lock(this->lock);
	this->configs->destroy_offset(this->configs,
								  offsetof(radius_config_t, destroy));
	this->configs = linked_list_create();
	load_configs(this);
	this->lock->unlock(this->lock);
	return TRUE;
}

METHOD(plugin_t, destroy, void,
	private_eap_radius_plugin_t *this)
{
	this->configs->destroy_offset(this->configs,
								  offsetof(radius_config_t, destroy));
	this->lock->destroy(this->lock);
	free(this);
	instance = NULL;
}

/*
 * see header file
 */
plugin_t *eap_radius_plugin_create()
{
	private_eap_radius_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.reload = _reload,
				.destroy = _destroy,
			},
		},
		.configs = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);
	instance = this;

	return &this->public.plugin;
}

/**
 * See header
 */
radius_client_t *eap_radius_create_client()
{
	if (instance)
	{
		enumerator_t *enumerator;
		radius_config_t *config, *selected = NULL;
		int current, best = -1;

		instance->lock->read_lock(instance->lock);
		enumerator = instance->configs->create_enumerator(instance->configs);
		while (enumerator->enumerate(enumerator, &config))
		{
			current = config->get_preference(config);
			if (current > best ||
				/* for two with equal preference, 50-50 chance */
				(current == best && random() % 2 == 0))
			{
				DBG2(DBG_CFG, "RADIUS server '%s' is candidate: %d",
					 config->get_name(config), current);
				best = current;
				DESTROY_IF(selected);
				selected = config->get_ref(config);
			}
			else
			{
				DBG2(DBG_CFG, "RADIUS server '%s' skipped: %d",
					 config->get_name(config), current);
			}
		}
		enumerator->destroy(enumerator);
		instance->lock->unlock(instance->lock);

		if (selected)
		{
			return radius_client_create(selected);
		}
	}
	return NULL;
}

/**
 * Job to delete all active IKE_SAs
 */
static job_requeue_t delete_all_async(void *data)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;

	enumerator = charon->ike_sa_manager->create_enumerator(
												charon->ike_sa_manager, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		lib->processor->queue_job(lib->processor,
				(job_t*)delete_ike_sa_job_create(ike_sa->get_id(ike_sa), TRUE));
	}
	enumerator->destroy(enumerator);

	return JOB_REQUEUE_NONE;
}

/**
 * See header.
 */
void eap_radius_handle_timeout(ike_sa_id_t *id)
{
	charon->bus->alert(charon->bus, ALERT_RADIUS_NOT_RESPONDING);

	if (lib->settings->get_bool(lib->settings,
								"%s.plugins.eap-radius.close_all_on_timeout",
								FALSE, lib->ns))
	{
		DBG1(DBG_CFG, "deleting all IKE_SAs after RADIUS timeout");
		lib->processor->queue_job(lib->processor,
				(job_t*)callback_job_create_with_prio(
						(callback_job_cb_t)delete_all_async, NULL, NULL,
						(callback_job_cancel_t)return_false, JOB_PRIO_CRITICAL));
	}
	else if (id)
	{
		DBG1(DBG_CFG, "deleting IKE_SA after RADIUS timeout");
		lib->processor->queue_job(lib->processor,
				(job_t*)delete_ike_sa_job_create(id, TRUE));
	}
}
