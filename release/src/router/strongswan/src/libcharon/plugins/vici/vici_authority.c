/*
 * Copyright (C) 2016 Tobias Brunner
 * Copyright (C) 2015 Andreas Steffen
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

#define _GNU_SOURCE

#include "vici_authority.h"
#include "vici_builder.h"

#include <threading/rwlock.h>
#include <collections/linked_list.h>
#include <credentials/certificates/x509.h>
#include <utils/debug.h>

#include <stdio.h>

typedef struct private_vici_authority_t private_vici_authority_t;

/**
 * Private data of an vici_authority_t object.
 */
struct private_vici_authority_t {

	/**
	 * Public vici_authority_t interface.
	 */
	vici_authority_t public;

	/**
	 * Dispatcher
	 */
	vici_dispatcher_t *dispatcher;

	/**
	 * credential backend managed by VICI used for our ca certificates
	 */
	vici_cred_t *cred;

	/**
	 * List of certification authorities
	 */
	linked_list_t *authorities;

	/**
	 * rwlock to lock access to certification authorities
	 */
	rwlock_t *lock;

};

typedef struct authority_t authority_t;

/**
 * loaded certification authorities
 */
struct authority_t {

	/**
	 * Name of the certification authoritiy
	 */
	char *name;

	/**
	 * Reference to CA certificate
	 */
	certificate_t *cert;

	/**
	 * CRL URIs
	 */
	linked_list_t *crl_uris;

	/**
	 * OCSP URIs
	 */
	linked_list_t *ocsp_uris;

	/**
	 * Hashes of certificates issued by this CA
	 */
	linked_list_t *hashes;

	/**
	 * Base URI used for certificates from this CA
	 */
	char *cert_uri_base;
};

/**
 * create a new certification authority
 */
static authority_t *authority_create(char *name)
{
	authority_t *authority;

	INIT(authority,
		.name = strdup(name),
		.crl_uris = linked_list_create(),
		.ocsp_uris = linked_list_create(),
		.hashes = linked_list_create(),
	);

	return authority;
}

/**
 * destroy a certification authority
 */
static void authority_destroy(authority_t *this)
{
	this->crl_uris->destroy_function(this->crl_uris, free);
	this->ocsp_uris->destroy_function(this->ocsp_uris, free);
	this->hashes->destroy_offset(this->hashes, offsetof(identification_t, destroy));
	DESTROY_IF(this->cert);
	free(this->cert_uri_base);
	free(this->name);
	free(this);
}


/**
 * Create a (error) reply message
 */
static vici_message_t* create_reply(char *fmt, ...)
{
	vici_builder_t *builder;
	va_list args;

	builder = vici_builder_create();
	builder->add_kv(builder, "success", fmt ? "no" : "yes");
	if (fmt)
	{
		va_start(args, fmt);
		builder->vadd_kv(builder, "errmsg", fmt, args);
		va_end(args);
	}
	return builder->finalize(builder);
}

/**
 * A rule to parse a key/value or list item
 */
typedef struct {
	/** name of the key/value or list */
	char *name;
	/** function to parse value */
	bool (*parse)(void *out, chunk_t value);
	/** result, passed to parse() */
	void *out;
} parse_rule_t;

/**
 * Parse key/values using a rule-set
 */
static bool parse_rules(parse_rule_t *rules, int count, char *name,
						chunk_t value, vici_message_t **reply)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (streq(name, rules[i].name))
		{
			if (rules[i].parse(rules[i].out, value))
			{
				return TRUE;
			}
			*reply = create_reply("invalid value for: %s, authority discarded",
								  name);
			return FALSE;
		}
	}
	*reply = create_reply("unknown option: %s, authority discarded", name);
	return FALSE;
}

/**
 * Parse callback data, passed to each callback
 */
typedef struct {
	private_vici_authority_t *this;
	vici_message_t *reply;
} request_data_t;

/**
 * Data associated with an authority load
 */
typedef struct {
	request_data_t *request;
	authority_t *authority;
	char *handle;
	uint32_t slot;
	char *module;
	char *file;
} load_data_t;

/**
 * Clean up data associated with an authority load
 */
static void free_load_data(load_data_t *data)
{
	if (data->authority)
	{
		authority_destroy(data->authority);
	}
	free(data->handle);
	free(data->module);
	free(data->file);
	free(data);
}

/**
 * Parse a string
 */
CALLBACK(parse_string, bool,
	char **str, chunk_t v)
{
	if (!chunk_printable(v, NULL, ' '))
	{
		return FALSE;
	}
	*str = strndup(v.ptr, v.len);

	return TRUE;
}

/**
 * Parse a uint32_t
 */
CALLBACK(parse_uint32, bool,
	uint32_t *out, chunk_t v)
{
	char buf[16], *end;
	u_long l;

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	l = strtoul(buf, &end, 0);
	if (*end == 0)
	{
		*out = l;
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse list of URIs
 */
CALLBACK(parse_uris, bool,
	linked_list_t *out, chunk_t v)
{
	char *uri;

	if (!chunk_printable(v, NULL, ' '))
	{
		return FALSE;
	}
	uri = strndup(v.ptr, v.len);
	out->insert_last(out, uri);

	return TRUE;
}

/**
 * Parse a CA certificate
 */
CALLBACK(parse_cacert, bool,
	certificate_t **cacert, chunk_t v)
{
	certificate_t *cert;
	x509_t *x509;

	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
										  BUILD_BLOB_PEM, v, BUILD_END);
	if (!cert)
	{
		return create_reply("parsing %N certificate failed",
							certificate_type_names, CERT_X509);
	}
	x509 = (x509_t*)cert;

	if ((x509->get_flags(x509) & X509_CA) != X509_CA)
	{
		cert->destroy(cert);
		return create_reply("certificate without CA flag, rejected");
	}
	*cacert = cert;

	return TRUE;
}

CALLBACK(authority_kv, bool,
	load_data_t *data, vici_message_t *message, char *name, chunk_t value)
{
	parse_rule_t rules[] = {
		{ "cacert",			parse_cacert, &data->authority->cert			},
		{ "file",			parse_string, &data->file						},
		{ "handle",			parse_string, &data->handle						},
		{ "slot",			parse_uint32, &data->slot						},
		{ "module",			parse_string, &data->module						},
		{ "cert_uri_base",	parse_string, &data->authority->cert_uri_base	},
	};

	return parse_rules(rules, countof(rules), name, value,
					   &data->request->reply);
}

CALLBACK(authority_li, bool,
	load_data_t *data, vici_message_t *message, char *name, chunk_t value)
{
	parse_rule_t rules[] = {
		{ "crl_uris",	parse_uris, data->authority->crl_uris  },
		{ "ocsp_uris",	parse_uris, data->authority->ocsp_uris },
	};

	return parse_rules(rules, countof(rules), name, value,
					   &data->request->reply);
}

static void log_authority_data(authority_t *authority)
{
	enumerator_t *enumerator;
	identification_t *subject;
	bool first = TRUE;
	char *uri;

	subject = authority->cert->get_subject(authority->cert);
	DBG2(DBG_CFG, "  cacert = %Y", subject);

	enumerator = authority->crl_uris->create_enumerator(authority->crl_uris);
	while (enumerator->enumerate(enumerator, &uri))
	{
		if (first)
		{
			DBG2(DBG_CFG, "  crl_uris = %s", uri);
			first = FALSE;
		}
		else
		{
			DBG2(DBG_CFG, "             %s", uri);
		}
	}
	enumerator->destroy(enumerator);

	first = TRUE;
	enumerator = authority->ocsp_uris->create_enumerator(authority->ocsp_uris);
	while (enumerator->enumerate(enumerator, &uri))
	{
		if (first)
		{
			DBG2(DBG_CFG, "  ocsp_uris = %s", uri);
			first = FALSE;
		}
		else
		{
			DBG2(DBG_CFG, "              %s", uri);
		}
	}
	enumerator->destroy(enumerator);

	if (authority->cert_uri_base)
	{
		DBG2(DBG_CFG, "  cert_uri_base = %s", authority->cert_uri_base);
	}
}

CALLBACK(authority_sn, bool,
	request_data_t *request, vici_message_t *message,
	vici_parse_context_t *ctx, char *name)
{
	enumerator_t *enumerator;
	linked_list_t *authorities;
	authority_t *authority;
	vici_cred_t *cred;
	load_data_t *data;
	chunk_t handle;

	INIT(data,
		.request = request,
		.authority = authority_create(name),
		.slot = -1,
	);

	DBG2(DBG_CFG, " authority %s:", name);

	if (!message->parse(message, ctx, NULL, authority_kv, authority_li, data))
	{
		free_load_data(data);
		return FALSE;
	}
	if (!data->authority->cert)
	{
		if (data->file)
		{
			data->authority->cert = lib->creds->create(lib->creds,
										CRED_CERTIFICATE, CERT_X509,
										BUILD_FROM_FILE, data->file, BUILD_END);
		}
		else if (data->handle)
		{
			handle = chunk_from_hex(chunk_from_str(data->handle), NULL);
			if (data->slot != -1)
			{
				data->authority->cert = lib->creds->create(lib->creds,
								CRED_CERTIFICATE, CERT_X509,
								BUILD_PKCS11_KEYID, handle,
								BUILD_PKCS11_SLOT, data->slot,
								data->module ? BUILD_PKCS11_MODULE : BUILD_END,
								data->module, BUILD_END);
			}
			else
			{
				data->authority->cert = lib->creds->create(lib->creds,
								CRED_CERTIFICATE, CERT_X509,
								BUILD_PKCS11_KEYID, handle,
								data->module ? BUILD_PKCS11_MODULE : BUILD_END,
								data->module, BUILD_END);
			}
			chunk_free(&handle);
		}
	}
	if (!data->authority->cert)
	{
		request->reply = create_reply("CA certificate missing: %s", name);
		free_load_data(data);
		return FALSE;
	}
	log_authority_data(data->authority);

	request->this->lock->write_lock(request->this->lock);

	authorities = request->this->authorities;
	enumerator = authorities->create_enumerator(authorities);
	while (enumerator->enumerate(enumerator, &authority))
	{
		if (streq(authority->name, name))
		{
			/* remove the old authority definition */
			authorities->remove_at(authorities, enumerator);
			authority_destroy(authority);
			break;
		}
	}
	enumerator->destroy(enumerator);
	authorities->insert_last(authorities, data->authority);

	cred = request->this->cred;
	data->authority->cert = cred->add_cert(cred, data->authority->cert);
	data->authority = NULL;

	request->this->lock->unlock(request->this->lock);
	free_load_data(data);

	return TRUE;
}

CALLBACK(load_authority, vici_message_t*,
	private_vici_authority_t *this, char *name, u_int id, vici_message_t *message)
{
	request_data_t request = {
		.this = this,
	};

	if (!message->parse(message, NULL, authority_sn, NULL, NULL, &request))
	{
		if (request.reply)
		{
			return request.reply;
		}
		return create_reply("parsing request failed");
	}
	return create_reply(NULL);
}

CALLBACK(unload_authority, vici_message_t*,
	private_vici_authority_t *this, char *name, u_int id, vici_message_t *message)
{
	enumerator_t *enumerator;
	authority_t *authority;
	char *authority_name;
	bool found = FALSE;

	authority_name = message->get_str(message, NULL, "name");
	if (!authority_name)
	{
		return create_reply("unload: missing authority name");
	}

	this->lock->write_lock(this->lock);
	enumerator = this->authorities->create_enumerator(this->authorities);
	while (enumerator->enumerate(enumerator, &authority))
	{
		if (streq(authority->name, authority_name))
		{
			this->authorities->remove_at(this->authorities, enumerator);
			authority_destroy(authority);
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	if (!found)
	{
		return create_reply("unload: authority '%s' not found", authority_name);
	}
	return create_reply(NULL);
}

CALLBACK(get_authorities, vici_message_t*,
	private_vici_authority_t *this, char *name, u_int id,
	vici_message_t *message)
{
	vici_builder_t *builder;
	enumerator_t *enumerator;
	authority_t *authority;

	builder = vici_builder_create();
	builder->begin_list(builder, "authorities");

	this->lock->read_lock(this->lock);
	enumerator = this->authorities->create_enumerator(this->authorities);
	while (enumerator->enumerate(enumerator, &authority))
	{
		builder->add_li(builder, "%s", authority->name);
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	builder->end_list(builder);

	return builder->finalize(builder);
}

CALLBACK(list_authorities, vici_message_t*,
	private_vici_authority_t *this, char *name, u_int id, vici_message_t *request)
{
	enumerator_t *enumerator, *e;
	authority_t *authority;
	vici_builder_t *b;
	char *str, *uri;

	str = request->get_str(request, NULL, "name");

	this->lock->read_lock(this->lock);
	enumerator = this->authorities->create_enumerator(this->authorities);
	while (enumerator->enumerate(enumerator, &authority))
	{
		if (str && !streq(str, authority->name))
		{
			continue;
		}
		b = vici_builder_create();

		/* open authority section */
		b->begin_section(b, authority->name);

		/* subject DN of cacert */
		b->add_kv(b, "cacert", "%Y",
					  authority->cert->get_subject(authority->cert));

		/* list of crl_uris */
		b->begin_list(b, "crl_uris");
		e = authority->crl_uris->create_enumerator(authority->crl_uris);
		while (e->enumerate(e, &uri))
		{
			b->add_li(b, "%s", uri);
		}
		e->destroy(e);
		b->end_list(b);

		/* list of ocsp_uris */
		b->begin_list(b, "ocsp_uris");
		e = authority->ocsp_uris->create_enumerator(authority->ocsp_uris);
		while (e->enumerate(e, &uri))
		{
			b->add_li(b, "%s", uri);
		}
		e->destroy(e);
		b->end_list(b);

		/* cert_uri_base */
		if (authority->cert_uri_base)
		{
			b->add_kv(b, "cert_uri_base", "%s", authority->cert_uri_base);
		}

		/* close authority and raise event */
		b->end_section(b);
		this->dispatcher->raise_event(this->dispatcher, "list-authority", id,
									  b->finalize(b));
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	b = vici_builder_create();
	return b->finalize(b);
}

static void manage_command(private_vici_authority_t *this,
						   char *name, vici_command_cb_t cb, bool reg)
{
	this->dispatcher->manage_command(this->dispatcher, name,
									 reg ? cb : NULL, this);
}

/**
 * (Un-)register dispatcher functions
 */
static void manage_commands(private_vici_authority_t *this, bool reg)
{
	this->dispatcher->manage_event(this->dispatcher, "list-authority", reg);

	manage_command(this, "load-authority", load_authority, reg);
	manage_command(this, "unload-authority", unload_authority, reg);
	manage_command(this, "get-authorities", get_authorities, reg);
	manage_command(this, "list-authorities", list_authorities, reg);
}

/**
 * data to pass to create_inner_cdp
 */
typedef struct {
	private_vici_authority_t *this;
	certificate_type_t type;
	identification_t *id;
} cdp_data_t;

/**
 * destroy cdp enumerator data and unlock list
 */
static void cdp_data_destroy(cdp_data_t *data)
{
	data->this->lock->unlock(data->this->lock);
	free(data);
}

/**
 * inner enumerator constructor for CDP URIs
 */
static enumerator_t *create_inner_cdp(authority_t *authority, cdp_data_t *data)
{
	public_key_t *public;
	enumerator_t *enumerator = NULL;
	linked_list_t *list;

	if (data->type == CERT_X509_OCSP_RESPONSE)
	{
		list = authority->ocsp_uris;
	}
	else
	{
		list = authority->crl_uris;
	}

	public = authority->cert->get_public_key(authority->cert);
	if (public)
	{
		if (!data->id)
		{
			enumerator = list->create_enumerator(list);
		}
		else
		{
			if (public->has_fingerprint(public, data->id->get_encoding(data->id)))
			{
				enumerator = list->create_enumerator(list);
			}
		}
		public->destroy(public);
	}
	return enumerator;
}

/**
 * inner enumerator constructor for "Hash and URL"
 */
static enumerator_t *create_inner_cdp_hashandurl(authority_t *authority,
												 cdp_data_t *data)
{
	enumerator_t *enumerator = NULL, *hash_enum;
	identification_t *current;

	if (!data->id || !authority->cert_uri_base)
	{
		return NULL;
	}

	hash_enum = authority->hashes->create_enumerator(authority->hashes);
	while (hash_enum->enumerate(hash_enum, &current))
	{
		if (current->matches(current, data->id))
		{
			char *url, *hash;

			url = malloc(strlen(authority->cert_uri_base) + 40 + 1);
			strcpy(url, authority->cert_uri_base);
			hash = chunk_to_hex(current->get_encoding(current), NULL, FALSE).ptr;
			strncat(url, hash, 40);
			free(hash);

			enumerator = enumerator_create_single(url, free);
			break;
		}
	}
	hash_enum->destroy(hash_enum);
	return enumerator;
}

METHOD(credential_set_t, create_cdp_enumerator, enumerator_t*,
	private_vici_authority_t *this, certificate_type_t type,
	identification_t *id)
{
	cdp_data_t *data;

	switch (type)
	{	/* we serve CRLs, OCSP responders and URLs for "Hash and URL" */
		case CERT_X509:
		case CERT_X509_CRL:
		case CERT_X509_OCSP_RESPONSE:
		case CERT_ANY:
			break;
		default:
			return NULL;
	}
	data = malloc_thing(cdp_data_t);
	data->this = this;
	data->type = type;
	data->id = id;

	this->lock->read_lock(this->lock);

	return enumerator_create_nested(
			this->authorities->create_enumerator(this->authorities),
			(type == CERT_X509) ? (void*)create_inner_cdp_hashandurl :
			(void*)create_inner_cdp, data, (void*)cdp_data_destroy);
}

METHOD(vici_authority_t, check_for_hash_and_url, void,
	private_vici_authority_t *this, certificate_t* cert)
{
	authority_t *authority;
	enumerator_t *enumerator;
	hasher_t *hasher;

	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (hasher == NULL)
	{
		DBG1(DBG_CFG, "unable to use hash-and-url: sha1 not supported");
		return;
	}

	this->lock->write_lock(this->lock);
	enumerator = this->authorities->create_enumerator(this->authorities);
	while (enumerator->enumerate(enumerator, &authority))
	{
		if (authority->cert_uri_base &&
			cert->issued_by(cert, authority->cert, NULL))
		{
			chunk_t hash, encoded;

			if (cert->get_encoding(cert, CERT_ASN1_DER, &encoded))
			{
				if (hasher->allocate_hash(hasher, encoded, &hash))
				{
					authority->hashes->insert_last(authority->hashes,
						identification_create_from_encoding(ID_KEY_ID, hash));
					chunk_free(&hash);
				}
				chunk_free(&encoded);
			}
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	hasher->destroy(hasher);
}

METHOD(vici_authority_t, destroy, void,
	private_vici_authority_t *this)
{
	manage_commands(this, FALSE);

	this->authorities->destroy_function(this->authorities,
									   (void*)authority_destroy);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
vici_authority_t *vici_authority_create(vici_dispatcher_t *dispatcher,
										vici_cred_t *cred)
{
	private_vici_authority_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_private_enumerator = (void*)return_null,
				.create_cert_enumerator = (void*)return_null,
				.create_shared_enumerator = (void*)return_null,
				.create_cdp_enumerator = _create_cdp_enumerator,
				.cache_cert = (void*)nop,
			},
			.check_for_hash_and_url = _check_for_hash_and_url,
			.destroy = _destroy,
		},
		.dispatcher = dispatcher,
		.cred = cred,
		.authorities = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	manage_commands(this, TRUE);

	return &this->public;
}
