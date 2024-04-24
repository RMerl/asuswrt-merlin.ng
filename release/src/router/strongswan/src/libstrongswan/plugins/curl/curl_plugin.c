/*
 * Copyright (C) 2023 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

#include "curl_plugin.h"

#include <library.h>
#include <utils/debug.h>
#include "curl_fetcher.h"

#include <curl/curl.h>

typedef struct private_curl_plugin_t private_curl_plugin_t;

/**
 * private data of curl_plugin
 */
struct private_curl_plugin_t {

	/**
	 * public functions
	 */
	curl_plugin_t public;

	/**
	 * Supported features, CURL protocols + 1
	 */
	plugin_feature_t *features;

	/**
	 * Number of supported features
	 */
	int count;
};

/**
 * Append a feature to supported feature list
 */
static void add_feature(private_curl_plugin_t *this, plugin_feature_t f)
{
	this->features = realloc(this->features, ++this->count * sizeof(f));
	this->features[this->count - 1] = f;
}

/**
 * Try to add a feature, and the appropriate SSL dependencies
 */
static void add_feature_with_ssl(private_curl_plugin_t *this, const char *ssl,
								 char *proto, plugin_feature_t f)
{
	/* according to https://curl.se/libcurl/c/threadsafe.html there is only an
	 * issue with thread-safety with older versions of OpenSSL (<= 1.0.2) and
	 * GnuTLS (< 1.6.0), so we just accept all other SSL backends */
	if (strpfx(ssl, "OpenSSL") || strpfx(ssl, "LibreSSL"))
	{
		add_feature(this, f);
		add_feature(this, PLUGIN_DEPENDS(CUSTOM, "openssl-threading"));
	}
	else if (strpfx(ssl, "GnuTLS"))
	{
		add_feature(this, f);
		add_feature(this, PLUGIN_DEPENDS(CUSTOM, "gcrypt-threading"));
	}
	else
	{
		add_feature(this, f);
	}
}

/**
 * Get supported protocols, build plugin feature set
 */
static bool query_protocols(private_curl_plugin_t *this)
{

	struct {
		/* protocol we are interested in, suffixed with "://" */
		char *name;
		/* require SSL library initialization? */
		bool ssl;
	} protos[] = {
		{ "file://",		FALSE,	},
		{ "http://",		FALSE,	},
		{ "https://",		TRUE,	},
		{ "ftp://",			FALSE,	},
	};
	curl_version_info_data *info;
	char *name;
	int i, j;

	add_feature(this, PLUGIN_REGISTER(FETCHER, curl_fetcher_create));

	info = curl_version_info(CURLVERSION_NOW);

	for (i = 0; info->protocols[i]; i++)
	{
		for (j = 0; j < countof(protos); j++)
		{
			name = protos[j].name;
			if (strlen(info->protocols[i]) == strlen(name) - strlen("://"))
			{
				if (strneq(info->protocols[i], name,
						   strlen(name) - strlen("://")))
				{
					if (protos[j].ssl)
					{
						add_feature_with_ssl(this, info->ssl_version, name,
									PLUGIN_PROVIDE(FETCHER, name));
					}
					else
					{
						add_feature(this, PLUGIN_PROVIDE(FETCHER, name));
					}
				}
			}
		}
	}

	return this->count > 1;
}

METHOD(plugin_t, get_name, char*,
	private_curl_plugin_t *this)
{
	return "curl";
}

METHOD(plugin_t, get_features, int,
	private_curl_plugin_t *this, plugin_feature_t *features[])
{
	*features = this->features;
	return this->count;
}

METHOD(plugin_t, destroy, void,
	private_curl_plugin_t *this)
{
	curl_global_cleanup();
	free(this->features);
	free(this);
}

#if LIBCURL_VERSION_NUM >= 0x073800
/**
 * Configure a specific SSL backend if multiple are available
 */
static void set_ssl_backend()
{
	const curl_ssl_backend **avail;
	char *backend, buf[BUF_LEN] = "";
	int i, len = 0, added;

	backend = lib->settings->get_str(lib->settings, "%s.plugins.curl.tls_backend",
									 NULL, lib->ns);
	switch (curl_global_sslset(-1, backend, &avail))
	{
		case CURLSSLSET_UNKNOWN_BACKEND:
			for (i = 0; avail[i]; i++)
			{
				added = snprintf(buf + len, sizeof(buf) - len, " %s",
								 avail[i]->name);
				if (added < sizeof(buf) - len)
				{
					len += added;
				}
			}
			if (backend)
			{
				DBG1(DBG_LIB, "unsupported TLS backend '%s' in libcurl, "
					 "available:%s", backend, buf);
			}
			else
			{
				DBG2(DBG_LIB, "available TLS backends in libcurl:%s", buf);
			}
			break;
		case CURLSSLSET_NO_BACKENDS:
			if (backend)
			{
				DBG1(DBG_LIB, "unable to set TLS backend '%s', libcurl was "
					 "built without TLS support", backend);
			}
			break;
		case CURLSSLSET_TOO_LATE:
			if (backend)
			{
				DBG1(DBG_LIB, "unable to set TLS backend '%s' in libcurl, "
					 "already set", backend);
			}
			break;
		case CURLSSLSET_OK:
			break;
	}
}
#endif

/*
 * see header file
 */
plugin_t *curl_plugin_create()
{
	CURLcode res;
	private_curl_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

#if LIBCURL_VERSION_NUM >= 0x073800
	set_ssl_backend();
#endif

	res = curl_global_init(CURL_GLOBAL_SSL);
	if (res != CURLE_OK)
	{
		/* no SSL support? Try without */
		res = curl_global_init(CURL_GLOBAL_NOTHING);
	}
	if (res != CURLE_OK)
	{
		DBG1(DBG_LIB, "global libcurl initializing failed: %s",
			 curl_easy_strerror(res));
		destroy(this);
		return NULL;
	}

	if (!query_protocols(this))
	{
		DBG1(DBG_LIB, "no usable CURL protocols found, curl disabled");
		destroy(this);
		return NULL;
	}

	return &this->public.plugin;
}
