/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2012 Reto Guadagnini
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

#include "ipseckey_plugin.h"

#include <daemon.h>
#include "ipseckey_cred.h"

typedef struct private_ipseckey_plugin_t private_ipseckey_plugin_t;


/**
 * private data of the ipseckey plugin
 */
struct private_ipseckey_plugin_t {

	/**
	 * implements plugin interface
	 */
	ipseckey_plugin_t public;

	/**
	 * credential set
	 */
	ipseckey_cred_t *cred;

	/**
	 * IPSECKEY based authentication enabled
	 */
	bool enabled;
};

METHOD(plugin_t, get_name, char*,
	private_ipseckey_plugin_t *this)
{
	return "ipseckey";
}

METHOD(plugin_t, reload, bool,
	private_ipseckey_plugin_t *this)
{
	bool enabled = lib->settings->get_bool(lib->settings,
								"%s.plugins.ipseckey.enable", FALSE, lib->ns);

	if (enabled != this->enabled)
	{
		if (enabled)
		{
			lib->credmgr->add_set(lib->credmgr, &this->cred->set);
		}
		else
		{
			lib->credmgr->remove_set(lib->credmgr, &this->cred->set);
		}
		this->enabled = enabled;
	}
	DBG1(DBG_CFG, "ipseckey plugin is %sabled", this->enabled ? "en" : "dis");
	return TRUE;
}

/**
 * Create resolver and register credential set
 */
static bool plugin_cb(private_ipseckey_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		resolver_t *res;

		res = lib->resolver->create(lib->resolver);
		if (!res)
		{
			DBG1(DBG_CFG, "failed to create a DNS resolver instance");
			return FALSE;
		}

		this->cred = ipseckey_cred_create(res);
		reload(this);
	}
	else
	{
		if (this->enabled)
		{
			lib->credmgr->remove_set(lib->credmgr, &this->cred->set);
		}
		this->cred->destroy(this->cred);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_ipseckey_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "ipseckey"),
				PLUGIN_DEPENDS(RESOLVER),
				PLUGIN_DEPENDS(PUBKEY, KEY_RSA),
				PLUGIN_DEPENDS(CERT_ENCODE, CERT_TRUSTED_PUBKEY),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_ipseckey_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *ipseckey_plugin_create()
{
	private_ipseckey_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.reload = _reload,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}
