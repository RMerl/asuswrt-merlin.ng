/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2 if the
 *   License as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file rlm_rest.c
 * @brief Integrate FreeRADIUS with RESTfull APIs
 *
 * @copyright 2012-2013  Arran Cudbard-Bell <arran.cudbardb@freeradius.org>
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/token.h>

#include "rest.h"

/*
 *	TLS Configuration
 */
static CONF_PARSER tls_config[] = {
	{ "ca_file", PW_TYPE_FILE_INPUT,
	  offsetof(rlm_rest_section_t,tls_ca_file), NULL, NULL},
	{ "ca_path", PW_TYPE_FILE_INPUT,
	  offsetof(rlm_rest_section_t,tls_ca_path), NULL, NULL},
	{ "certificate_file", PW_TYPE_FILE_INPUT,
	  offsetof(rlm_rest_section_t,tls_certificate_file), NULL, NULL},
	{ "private_key_file", PW_TYPE_FILE_INPUT,
	  offsetof(rlm_rest_section_t,tls_private_key_file), NULL, NULL },
	{ "private_key_password", PW_TYPE_STRING_PTR,
	  offsetof(rlm_rest_section_t, tls_private_key_password), NULL, NULL },
	{ "random_file", PW_TYPE_STRING_PTR, /* OK if it changes on HUP */
	  offsetof(rlm_rest_section_t,tls_random_file), NULL, NULL },
	{ "check_cert", PW_TYPE_BOOLEAN,
	  offsetof(rlm_rest_section_t, tls_check_cert), NULL, "yes" },
	{ "check_cert_cn", PW_TYPE_BOOLEAN,
	  offsetof(rlm_rest_section_t, tls_check_cert_cn), NULL, "yes" },

	{ NULL, -1, 0, NULL, NULL }
};

/*
 *	A mapping of configuration file names to internal variables.
 *
 *	Note that the string is dynamically allocated, so it MUST
 *	be freed.  When the configuration file parse re-reads the string,
 *	it free's the old one, and strdup's the new one, placing the pointer
 *	to the strdup'd string into 'config.string'.  This gets around
 *	buffer over-flows.
 */
static const CONF_PARSER section_config[] = {
	{ "uri", PW_TYPE_STRING_PTR,
	 offsetof(rlm_rest_section_t, uri), 	   NULL, ""  },
	{ "method", PW_TYPE_STRING_PTR,
	 offsetof(rlm_rest_section_t, method_str), NULL, "GET" },
	{ "body", PW_TYPE_STRING_PTR,
	 offsetof(rlm_rest_section_t, body_str),   NULL, "post" },

	/* User authentication */
	{ "auth", PW_TYPE_STRING_PTR,
	 offsetof(rlm_rest_section_t, auth_str),   NULL, "none" },
	{ "username", PW_TYPE_STRING_PTR,
	 offsetof(rlm_rest_section_t, username),   NULL, NULL },
	{ "password", PW_TYPE_STRING_PTR,
	 offsetof(rlm_rest_section_t, password),   NULL, NULL },
	{ "require_auth", PW_TYPE_BOOLEAN,
	 offsetof(rlm_rest_section_t, require_auth), NULL, "no"},

	/* Transfer configuration */
	{ "timeout", PW_TYPE_INTEGER,
	 offsetof(rlm_rest_section_t, timeout),    NULL, "0" },
	{ "chunk", PW_TYPE_INTEGER,
	 offsetof(rlm_rest_section_t, chunk), 	   NULL, "0" },

	/* TLS Parameters */
	{ "tls", PW_TYPE_SUBSECTION, 0, NULL, (void const *) tls_config },

	{ NULL, -1, 0, NULL, NULL }
};

static const CONF_PARSER module_config[] = {
	{ "connect_uri", PW_TYPE_STRING_PTR,
	 offsetof(rlm_rest_t, connect_uri), NULL, "http://localhost/" },

	{ NULL, -1, 0, NULL, NULL }
};

static int rlm_rest_perform(rlm_rest_t *instance, rlm_rest_section_t *section,
			    void *handle, REQUEST *request,
			    char const *username, char const *password)
{
	size_t uri_len;
	char *uri = NULL;

	int ret;

	RDEBUG("Expanding URI components");

	/*
	 *	Build xlat'd URI, this allows REST servers to be specified by
	 *	request attributes.
	 */
	uri_len = rest_uri_build(&uri, instance, section, request);
	if (uri_len <= 0) return -1;

	RDEBUG("Sending HTTP %s to \"%s\"", fr_int2str(http_method_table, section->method, NULL), uri);

	/*
	 *	Configure various CURL options, and initialise the read/write
	 *	context data.
	 */
	ret = rest_request_config(instance, section, request, handle, section->method, section->body,
				  uri, username, password);
	talloc_free(uri);
	if (ret < 0) return -1;

	/*
	 *	Send the CURL request, pre-parse headers, aggregate incoming
	 *	HTTP body data into a single contiguous buffer.
	 */
	ret = rest_request_perform(instance, section, request, handle);
	if (ret < 0) return -1;

	return 0;
}

static void rlm_rest_cleanup(rlm_rest_t *instance, rlm_rest_section_t *section,
			     void *handle)
{
	rest_request_cleanup(instance, section, handle);
};

static int parse_sub_section(CONF_SECTION *parent,
	 		     rlm_rest_section_t *config,
	 		     rlm_components_t comp)
{
	CONF_SECTION *cs;

	char const *name = section_type_value[comp].section;

	cs = cf_section_sub_find(parent, name);
	if (!cs) {
		/* TODO: Should really setup section with default values */
		return 0;
	}

	if (cf_section_parse(cs, config, section_config) < 0) {
		return -1;
	}

	/*
	 *	Add section name (Maybe add to headers later?).
	 */
	config->name = name;

	/*
	 *	Sanity check
	 */
	 if ((config->username && !config->password) || (!config->username && config->password)) {
	 	cf_log_err_cs(cs, "'username' and 'password' must both be set or both be absent");

	 	return -1;
	 }

	/*
	 *	Convert HTTP method auth and body type strings into their
	 *	integer equivalents.
	 */
	config->auth = fr_str2int(http_auth_table, config->auth_str, HTTP_AUTH_UNKNOWN);
	if (config->auth == HTTP_AUTH_UNKNOWN) {
		cf_log_err_cs(cs, "Unknown HTTP auth type '%s'", config->auth_str);
		return -1;
	} else if ((config->auth != HTTP_AUTH_NONE) && !http_curl_auth[config->auth]) {
		cf_log_err_cs(cs, "Unsupported HTTP auth type \"%s\", check libcurl version, OpenSSL build "
			      "configuration, then recompile this module", config->auth_str);

		return -1;
	}

	config->method = fr_str2int(http_method_table, config->method_str,
				    HTTP_METHOD_CUSTOM);

	config->body = fr_str2int(http_body_type_table, config->body_str,
				  HTTP_BODY_UNKNOWN);

	if (config->body == HTTP_BODY_UNKNOWN) {
		cf_log_err_cs(cs, "Unknown HTTP body type '%s'",
			      config->body_str);
		return -1;
	}

	if (http_body_type_supported[config->body] == HTTP_BODY_UNSUPPORTED) {
		cf_log_err_cs(cs, "Unsupported HTTP body type \"%s\""
		       ", please submit patches",
		       config->body_str);
		return -1;
	}

	return 1;
}

/*
 *	Do any per-module initialization that is separate to each
 *	configured instance of the module.  e.g. set up connections
 *	to external databases, read configuration files, set up
 *	dictionary entries, etc.
 *
 *	If configuration information is given in the config section
 *	that must be referenced in later calls, store a handle to it
 *	in *instance otherwise put a null pointer there.
 */
static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	rlm_rest_t *inst = instance;
	char const *xlat_name;

	xlat_name = cf_section_name2(conf);
	if (!xlat_name) {
		xlat_name = cf_section_name1(conf);
	}

	inst->xlat_name = xlat_name;

	/*
	 *	Parse sub-section configs.
	 */
	if (
		(parse_sub_section(conf, &inst->authorize,
				   RLM_COMPONENT_AUTZ) < 0) ||
		(parse_sub_section(conf, &inst->authenticate,
				   RLM_COMPONENT_AUTH) < 0) ||
		(parse_sub_section(conf, &inst->accounting,
				   RLM_COMPONENT_ACCT) < 0) ||
		(parse_sub_section(conf, &inst->checksimul,
				   RLM_COMPONENT_SESS) < 0) ||
		(parse_sub_section(conf, &inst->postauth,
				   RLM_COMPONENT_POST_AUTH) < 0))
	{
		return -1;
	}

	/*
	 *	Initialise REST libraries.
	 */
	if (rest_init(inst) < 0) {
		return -1;
	}

	inst->conn_pool = fr_connection_pool_init(conf, inst, mod_conn_create, mod_conn_alive, mod_conn_delete, NULL);
	if (!inst->conn_pool) {
		return -1;
	}

	return 0;
}

/*
 *	Find the named user in this modules database.  Create the set
 *	of attribute-value pairs to check and reply with for this user
 *	from the database. The authentication code only needs to check
 *	the password, the rest is done here.
 */
static rlm_rcode_t mod_authorize(void *instance, REQUEST *request)
{
	rlm_rest_t *inst = instance;
	rlm_rest_section_t *section = &inst->authorize;

	void *handle;
	int hcode;
	int rcode = RLM_MODULE_OK;
	int ret;

	handle = fr_connection_get(inst->conn_pool);
	if (!handle) return RLM_MODULE_FAIL;

	ret = rlm_rest_perform(instance, section, handle, request, NULL, NULL);
	if (ret < 0) {
		rcode = RLM_MODULE_FAIL;
		goto end;
	}

	hcode = rest_get_handle_code(handle);
	switch (hcode) {
		case 404:
		case 410:
			rcode = RLM_MODULE_NOTFOUND;
			break;
		case 403:
			rcode = RLM_MODULE_USERLOCK;
			break;
		case 401:
			/*
			 *	Attempt to parse content if there was any.
			 */
			ret = rest_request_decode(inst, section, request, handle);
			if (ret < 0) {
				rcode = RLM_MODULE_FAIL;
				break;
			}

			rcode = RLM_MODULE_REJECT;
			break;
		case 204:
			rcode = RLM_MODULE_OK;
			break;
		default:
			/*
			 *	Attempt to parse content if there was any.
			 */
			if ((hcode >= 200) && (hcode < 300)) {
				ret = rest_request_decode(inst, section, request, handle);
				if (ret < 0) 	   rcode = RLM_MODULE_FAIL;
				else if (ret == 0) rcode = RLM_MODULE_OK;
				else		   rcode = RLM_MODULE_UPDATED;
				break;
			} else if (hcode < 500) {
				rcode = RLM_MODULE_INVALID;
			} else {
				rcode = RLM_MODULE_FAIL;
			}
	}

	end:

	rlm_rest_cleanup(instance, section, handle);

	fr_connection_release(inst->conn_pool, handle);

	return rcode;
}

/*
 *	Authenticate the user with the given password.
 */
static rlm_rcode_t mod_authenticate(void *instance, UNUSED REQUEST *request)
{
	rlm_rest_t *inst = instance;
	rlm_rest_section_t *section = &inst->authenticate;

	void *handle;
	int hcode;
	int rcode = RLM_MODULE_OK;
	int ret;

	VALUE_PAIR const *username;
	VALUE_PAIR const *password;

	username = pairfind(request->packet->vps, PW_USER_NAME, 0, TAG_ANY);
	if (!username) {
		REDEBUG("Can't perform authentication, 'User-Name' attribute not found in the request");

		return RLM_MODULE_INVALID;
	}

	password = pairfind(request->config_items, PW_CLEARTEXT_PASSWORD, 0, TAG_ANY);
	if (!password) {
		REDEBUG("Can't perform authentication, 'Cleartext-Password' attribute not found in the control list");

		return RLM_MODULE_INVALID;
	}

	handle = fr_connection_get(inst->conn_pool);
	if (!handle) return RLM_MODULE_FAIL;

	ret = rlm_rest_perform(instance, section, handle, request, username->vp_strvalue, password->vp_strvalue);
	if (ret < 0) {
		rcode = RLM_MODULE_FAIL;
		goto end;
	}

	hcode = rest_get_handle_code(handle);
	switch (hcode) {
		case 404:
		case 410:
			rcode = RLM_MODULE_NOTFOUND;
			break;
		case 403:
			rcode = RLM_MODULE_USERLOCK;
			break;
		case 401:
			/*
			 *	Attempt to parse content if there was any.
			 */
			ret = rest_request_decode(inst, section, request, handle);
			if (ret < 0) {
				rcode = RLM_MODULE_FAIL;
				break;
			}

			rcode = RLM_MODULE_REJECT;
			break;
		case 204:
			rcode = RLM_MODULE_OK;
			break;
		default:
			/*
			 *	Attempt to parse content if there was any.
			 */
			if ((hcode >= 200) && (hcode < 300)) {
				ret = rest_request_decode(inst, section, request, handle);
				if (ret < 0) 	   rcode = RLM_MODULE_FAIL;
				else if (ret == 0) rcode = RLM_MODULE_OK;
				else		   rcode = RLM_MODULE_UPDATED;
				break;
			} else if (hcode < 500) {
				rcode = RLM_MODULE_INVALID;
			} else {
				rcode = RLM_MODULE_FAIL;
			}
	}

	end:

	rlm_rest_cleanup(instance, section, handle);

	fr_connection_release(inst->conn_pool, handle);

	return rcode;
}

/*
 *	Write accounting information to this modules database.
 */
static rlm_rcode_t mod_accounting(void *instance, UNUSED REQUEST *request)
{
	rlm_rest_t *inst = instance;
	rlm_rest_section_t *section = &inst->accounting;

	void *handle;
	int hcode;
	int rcode = RLM_MODULE_OK;
	int ret;

	handle = fr_connection_get(inst->conn_pool);
	if (!handle) return RLM_MODULE_FAIL;

	ret = rlm_rest_perform(inst, section, handle, request, NULL, NULL);
	if (ret < 0) {
		rcode = RLM_MODULE_FAIL;
		goto end;
	}

	hcode = rest_get_handle_code(handle);
	if (hcode >= 500) {
		rcode = RLM_MODULE_FAIL;
	} else if (hcode == 204) {
		rcode = RLM_MODULE_OK;
	} else if ((hcode >= 200) && (hcode < 300)) {
		ret = rest_request_decode(inst, section, request, handle);
		if (ret < 0) 	   rcode = RLM_MODULE_FAIL;
		else if (ret == 0) rcode = RLM_MODULE_OK;
		else		   rcode = RLM_MODULE_UPDATED;
	} else {
		rcode = RLM_MODULE_INVALID;
	}

	end:

	rlm_rest_cleanup(inst, section, handle);

	fr_connection_release(inst->conn_pool, handle);

	return rcode;
}

/*
 *	Only free memory we allocated.  The strings allocated via
 *	cf_section_parse() do not need to be freed.
 */
static int mod_detach(void *instance)
{
	rlm_rest_t *inst = instance;

	fr_connection_pool_delete(inst->conn_pool);

	/* Free any memory used by libcurl */
	rest_cleanup();

	return 0;
}

/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 *
 *	If the module needs to temporarily modify it's instantiation
 *	data, the type should be changed to RLM_TYPE_THREAD_UNSAFE.
 *	The server will then take care of ensuring that the module
 *	is single-threaded.
 */
module_t rlm_rest = {
	RLM_MODULE_INIT,
	"rlm_rest",
	RLM_TYPE_THREAD_SAFE,		/* type */
	sizeof(rlm_rest_t),
	module_config,
	mod_instantiate,		/* instantiation */
	mod_detach,			/* detach */
	{
		mod_authenticate,	/* authentication */
		mod_authorize,		/* authorization */
		NULL,			/* preaccounting */
		mod_accounting,		/* accounting */
		NULL,			/* checksimul */
		NULL,			/* pre-proxy */
		NULL,			/* post-proxy */
		NULL			/* post-auth */
	},
};
