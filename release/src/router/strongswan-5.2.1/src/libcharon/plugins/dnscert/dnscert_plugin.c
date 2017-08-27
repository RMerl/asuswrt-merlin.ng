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
/*
 * Copyright (C) 2013 Ruslan Marchenko
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

#include "dnscert_plugin.h"

#include <daemon.h>
#include "dnscert_cred.h"

typedef struct private_dnscert_plugin_t private_dnscert_plugin_t;


/**
 * private data of the dnscert plugin
 */
struct private_dnscert_plugin_t {

	/**
	 * implements plugin interface
	 */
	dnscert_plugin_t public;

	/**
	 * credential set
	 */
	dnscert_cred_t *cred;

	/**
	 * DNSCERT based authentication enabled
	 */
	bool enabled;
};

METHOD(plugin_t, get_name, char*,
	private_dnscert_plugin_t *this)
{
	return "dnscert";
}

METHOD(plugin_t, reload, bool,
	private_dnscert_plugin_t *this)
{
	bool enabled = lib->settings->get_bool(lib->settings,
								"%s.plugins.dnscert.enable", FALSE, lib->ns);

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
	DBG1(DBG_CFG, "dnscert plugin is %sabled", this->enabled ? "en" : "dis");
	return TRUE;
}

/**
 * Create resolver and register credential set
 */
static bool plugin_cb(private_dnscert_plugin_t *this,
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

		this->cred = dnscert_cred_create(res);
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
	private_dnscert_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "dnscert"),
				PLUGIN_DEPENDS(RESOLVER),
				PLUGIN_DEPENDS(CERT_DECODE, CERT_ANY),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_X509),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_GPG),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_dnscert_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *dnscert_plugin_create()
{
	private_dnscert_plugin_t *this;

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
