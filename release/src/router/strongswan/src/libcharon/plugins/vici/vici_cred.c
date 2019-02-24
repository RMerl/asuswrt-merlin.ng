/*
 * Copyright (C) 2015-2016 Andreas Steffen
 * Copyright (C) 2016-2017 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
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

#include "vici_cred.h"
#include "vici_builder.h"
#include "vici_cert_info.h"

#include <credentials/sets/mem_cred.h>
#include <credentials/certificates/ac.h>
#include <credentials/certificates/crl.h>
#include <credentials/certificates/x509.h>

#include <errno.h>

typedef struct private_vici_cred_t private_vici_cred_t;

/**
 * Directory for saved X.509 CRLs
 */
#define CRL_DIR SWANCTLDIR "/x509crl"

/**
 * Private data of an vici_cred_t object.
 */
struct private_vici_cred_t {

	/**
	 * Public vici_cred_t interface.
	 */
	vici_cred_t public;

	/**
	 * Dispatcher
	 */
	vici_dispatcher_t *dispatcher;

	/**
	 * credentials
	 */
	mem_cred_t *creds;

	/**
	 * separate credential set for token PINs
	 */
	mem_cred_t *pins;

	/**
	 * cache CRLs to disk?
	 */
	bool cachecrl;

};

METHOD(credential_set_t, cache_cert, void,
	private_vici_cred_t *this, certificate_t *cert)
{
	if (cert->get_type(cert) == CERT_X509_CRL && this->cachecrl)
	{
		/* CRLs get written to /etc/swanctl/x509crl/<authkeyId>.crl */
		crl_t *crl = (crl_t*)cert;

		cert->get_ref(cert);
		if (this->creds->add_crl(this->creds, crl))
		{
			char buf[BUF_LEN];
			chunk_t chunk, hex;
			bool is_delta_crl;

			is_delta_crl = crl->is_delta_crl(crl, NULL);
			chunk = crl->get_authKeyIdentifier(crl);
			hex = chunk_to_hex(chunk, NULL, FALSE);
			snprintf(buf, sizeof(buf), "%s/%s%s.crl", CRL_DIR, hex.ptr,
										is_delta_crl ? "_delta" : "");
			free(hex.ptr);

			if (cert->get_encoding(cert, CERT_ASN1_DER, &chunk))
			{
				if (chunk_write(chunk, buf, 022, TRUE))
				{
					DBG1(DBG_CFG, "  written crl file '%s' (%d bytes)",
						 buf, chunk.len);
				}
				else
				{
					DBG1(DBG_CFG, "  writing crl file '%s' failed: %s",
						 buf, strerror(errno));
				}
				free(chunk.ptr);
			}
		}
	}
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

CALLBACK(load_cert, vici_message_t*,
	private_vici_cred_t *this, char *name, u_int id, vici_message_t *message)
{
	certificate_t *cert;
	certificate_type_t type;
	x509_flag_t ext_flag, flag = X509_NONE;
	x509_t *x509;
	chunk_t data;
	bool trusted = TRUE;
	char *str;

	str = message->get_str(message, NULL, "type");
	if (!str)
	{
		return create_reply("certificate type missing");
	}
	if (enum_from_name(certificate_type_names, str, &type))
	{
		if (type == CERT_X509)
		{
			str = message->get_str(message, "NONE", "flag");
			if (!enum_from_name(x509_flag_names, str, &flag))
			{
				return create_reply("invalid certificate flag '%s'", str);
			}
		}
	}
	else if	(!vici_cert_info_from_str(str, &type, &flag))
	{
		return create_reply("invalid certificate type '%s'", str);
	}

	data = message->get_value(message, chunk_empty, "data");
	if (!data.len)
	{
		return create_reply("certificate data missing");
	}

	/* do not set CA flag externally */
	ext_flag = (flag & X509_CA) ? X509_NONE : flag;

	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, type,
							  BUILD_BLOB_PEM, data,
							  BUILD_X509_FLAG, ext_flag,
							  BUILD_END);
	if (!cert)
	{
		return create_reply("parsing %N certificate failed",
							certificate_type_names, type);
	}
	DBG1(DBG_CFG, "loaded certificate '%Y'", cert->get_subject(cert));

	/* check if CA certificate has CA basic constraint set */
	if (flag & X509_CA)
	{
		char err_msg[] = "ca certificate lacks CA basic constraint, rejected";
		x509 = (x509_t*)cert;

		if (!(x509->get_flags(x509) & X509_CA))
		{
			cert->destroy(cert);
			DBG1(DBG_CFG, "  %s", err_msg);
			return create_reply(err_msg);
		}
	}
	if (type == CERT_X509_CRL)
	{
		this->creds->add_crl(this->creds, (crl_t*)cert);
	}
	else
	{
		this->creds->add_cert(this->creds, trusted, cert);
	}
	return create_reply(NULL);
}

CALLBACK(load_key, vici_message_t*,
	private_vici_cred_t *this, char *name, u_int id, vici_message_t *message)
{
	vici_builder_t *builder;
	key_type_t type;
	private_key_t *key;
	chunk_t data, fp;
	char *str;

	str = message->get_str(message, NULL, "type");
	if (!str)
	{
		return create_reply("key type missing");
	}
	if (strcaseeq(str, "any"))
	{
		type = KEY_ANY;
	}
	else if (strcaseeq(str, "rsa"))
	{
		type = KEY_RSA;
	}
	else if (strcaseeq(str, "ecdsa"))
	{
		type = KEY_ECDSA;
	}
	else if (strcaseeq(str, "bliss"))
	{
		type = KEY_BLISS;
	}
	else
	{
		return create_reply("invalid key type: %s", str);
	}
	data = message->get_value(message, chunk_empty, "data");
	if (!data.len)
	{
		return create_reply("key data missing");
	}
	key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, type,
							 BUILD_BLOB_PEM, data, BUILD_END);
	if (!key)
	{
		return create_reply("parsing %N private key failed",
							key_type_names, type);
	}
	if (!key->get_fingerprint(key, KEYID_PUBKEY_SHA1, &fp))
	{
		return create_reply("failed to get key id");
	}

	DBG1(DBG_CFG, "loaded %N private key", key_type_names, type);

	builder = vici_builder_create();
	builder->add_kv(builder, "success", "yes");
	builder->add_kv(builder, "id", "%+B", &fp);
	this->creds->add_key(this->creds, key);

	return builder->finalize(builder);
}

CALLBACK(unload_key, vici_message_t*,
	private_vici_cred_t *this, char *name, u_int id, vici_message_t *message)
{
	chunk_t keyid;
	char buf[BUF_LEN], *hex, *msg = NULL;

	hex = message->get_str(message, NULL, "id");
	if (!hex)
	{
		return create_reply("key id missing");
	}
	keyid = chunk_from_hex(chunk_from_str(hex), NULL);
	snprintf(buf, sizeof(buf), "%+B", &keyid);
	DBG1(DBG_CFG, "unloaded private key with id %s", buf);
	if (this->creds->remove_key(this->creds, keyid))
	{	/* also remove any potential PIN associated with this id */
		this->pins->remove_shared_unique(this->pins, buf);
	}
	else
	{
		msg = "key not found";
	}
	chunk_free(&keyid);
	return create_reply(msg);
}

CALLBACK(get_keys, vici_message_t*,
	private_vici_cred_t *this, char *name, u_int id, vici_message_t *message)
{
	vici_builder_t *builder;
	enumerator_t *enumerator;
	private_key_t *private;
	chunk_t keyid;

	builder = vici_builder_create();
	builder->begin_list(builder, "keys");

	enumerator = this->creds->set.create_private_enumerator(&this->creds->set,
															KEY_ANY, NULL);
	while (enumerator->enumerate(enumerator, &private))
	{
		if (private->get_fingerprint(private, KEYID_PUBKEY_SHA1, &keyid))
		{
			builder->add_li(builder, "%+B", &keyid);
		}
	}
	enumerator->destroy(enumerator);

	builder->end_list(builder);
	return builder->finalize(builder);
}

CALLBACK(load_token, vici_message_t*,
	private_vici_cred_t *this, char *name, u_int id, vici_message_t *message)
{
	vici_builder_t *builder;
	private_key_t *key;
	shared_key_t *shared = NULL;
	identification_t *owner;
	mem_cred_t *set = NULL;
	chunk_t handle, fp;
	char buf[BUF_LEN], *hex, *module, *pin, *unique = NULL;
	int slot;

	hex = message->get_str(message, NULL, "handle");
	if (!hex)
	{
		return create_reply("keyid missing");
	}
	handle = chunk_from_hex(chunk_from_str(hex), NULL);
	slot = message->get_int(message, -1, "slot");
	module = message->get_str(message, NULL, "module");
	pin = message->get_str(message, NULL, "pin");

	if (pin)
	{	/* provide the pin in a temporary credential set to access the key */
		shared = shared_key_create(SHARED_PIN, chunk_clone(chunk_from_str(pin)));
		owner = identification_create_from_encoding(ID_KEY_ID, handle);
		set = mem_cred_create();
		set->add_shared(set, shared->get_ref(shared), owner, NULL);
		lib->credmgr->add_local_set(lib->credmgr, &set->set, FALSE);
	}
	if (slot >= 0)
	{
		key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_ANY,
						BUILD_PKCS11_KEYID, handle,
						BUILD_PKCS11_SLOT, slot,
						module ? BUILD_PKCS11_MODULE : BUILD_END, module,
						BUILD_END);
	}
	else
	{
		key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_ANY,
						BUILD_PKCS11_KEYID, handle,
						module ? BUILD_PKCS11_MODULE : BUILD_END, module,
						BUILD_END);
	}
	if (set)
	{
		lib->credmgr->remove_local_set(lib->credmgr, &set->set);
		set->destroy(set);
	}
	if (!key)
	{
		chunk_free(&handle);
		DESTROY_IF(shared);
		return create_reply("loading private key from token failed");
	}
	builder = vici_builder_create();
	builder->add_kv(builder, "success", "yes");
	if (key->get_fingerprint(key, KEYID_PUBKEY_SHA1, &fp))
	{
		snprintf(buf, sizeof(buf), "%+B", &fp);
		builder->add_kv(builder, "id", "%s", buf);
		unique = buf;
	}
	if (shared && unique)
	{	/* use the handle as owner, but the key identifier as unique ID */
		owner = identification_create_from_encoding(ID_KEY_ID, handle);
		this->pins->add_shared_unique(this->pins, unique, shared,
									linked_list_create_with_items(owner, NULL));
	}
	else
	{
		DESTROY_IF(shared);
	}
	DBG1(DBG_CFG, "loaded %N private key from token", key_type_names,
		 key->get_type(key));
	this->creds->add_key(this->creds, key);
	chunk_free(&handle);
	return builder->finalize(builder);
}

CALLBACK(shared_owners, bool,
	linked_list_t *owners, vici_message_t *message, char *name, chunk_t value)
{
	if (streq(name, "owners"))
	{
		char buf[256];

		if (!vici_stringify(value, buf, sizeof(buf)))
		{
			return FALSE;
		}
		owners->insert_last(owners, identification_create_from_string(buf));
	}
	return TRUE;
}

CALLBACK(load_shared, vici_message_t*,
	private_vici_cred_t *this, char *name, u_int id, vici_message_t *message)
{
	shared_key_type_t type;
	linked_list_t *owners;
	chunk_t data;
	char *unique, *str, buf[512] = "";
	enumerator_t *enumerator;
	identification_t *owner;
	int len;

	unique = message->get_str(message, NULL, "id");
	str = message->get_str(message, NULL, "type");
	if (!str)
	{
		return create_reply("shared key type missing");
	}
	if (strcaseeq(str, "ike"))
	{
		type = SHARED_IKE;
	}
	else if (strcaseeq(str, "eap") || strcaseeq(str, "xauth"))
	{
		type = SHARED_EAP;
	}
	else if (strcaseeq(str, "ntlm"))
	{
		type = SHARED_NT_HASH;
	}
	else if (strcaseeq(str, "ppk"))
	{
		type = SHARED_PPK;
	}
	else
	{
		return create_reply("invalid shared key type: %s", str);
	}
	data = message->get_value(message, chunk_empty, "data");
	if (!data.len)
	{
		return create_reply("shared key data missing");
	}

	owners = linked_list_create();
	if (!message->parse(message, NULL, NULL, NULL, shared_owners, owners))
	{
		owners->destroy_offset(owners, offsetof(identification_t, destroy));
		return create_reply("parsing shared key owners failed");
	}
	if (owners->get_count(owners) == 0)
	{
		owners->insert_last(owners, identification_create_from_string("%any"));
	}

	enumerator = owners->create_enumerator(owners);
	while (enumerator->enumerate(enumerator, &owner))
	{
		len = strlen(buf);
		if (len < sizeof(buf))
		{
			snprintf(buf + len, sizeof(buf) - len, "%s'%Y'",
					 len ? ", " : "", owner);
		}
	}
	enumerator->destroy(enumerator);

	if (unique)
	{
		DBG1(DBG_CFG, "loaded %N shared key with id '%s' for: %s",
			 shared_key_type_names, type, unique, buf);
	}
	else
	{
		DBG1(DBG_CFG, "loaded %N shared key for: %s",
			 shared_key_type_names, type, buf);
	}

	this->creds->add_shared_unique(this->creds, unique,
						shared_key_create(type, chunk_clone(data)), owners);

	return create_reply(NULL);
}

CALLBACK(unload_shared, vici_message_t*,
	private_vici_cred_t *this, char *name, u_int id, vici_message_t *message)
{
	char *unique;

	unique = message->get_str(message, NULL, "id");
	if (!unique)
	{
		return create_reply("unique identifier missing");
	}
	DBG1(DBG_CFG, "unloaded shared key with id '%s'", unique);
	this->creds->remove_shared_unique(this->creds, unique);
	return create_reply(NULL);
}

CALLBACK(get_shared, vici_message_t*,
	private_vici_cred_t *this, char *name, u_int id, vici_message_t *message)
{
	vici_builder_t *builder;
	enumerator_t *enumerator;
	char *unique;

	builder = vici_builder_create();
	builder->begin_list(builder, "keys");

	enumerator = this->creds->create_unique_shared_enumerator(this->creds);
	while (enumerator->enumerate(enumerator, &unique))
	{
		builder->add_li(builder, "%s", unique);
	}
	enumerator->destroy(enumerator);

	builder->end_list(builder);
	return builder->finalize(builder);
}

CALLBACK(clear_creds, vici_message_t*,
	private_vici_cred_t *this, char *name, u_int id, vici_message_t *message)
{
	this->creds->clear(this->creds);
	lib->credmgr->flush_cache(lib->credmgr, CERT_ANY);

	return create_reply(NULL);
}

CALLBACK(flush_certs, vici_message_t*,
	private_vici_cred_t *this, char *name, u_int id, vici_message_t *message)
{
	certificate_type_t type = CERT_ANY;
	x509_flag_t flag = X509_NONE;
	char *str;

	str = message->get_str(message, NULL, "type");
	if (str && !enum_from_name(certificate_type_names, str, &type) &&
			   !vici_cert_info_from_str(str, &type, &flag))
	{
		return create_reply("invalid certificate type '%s'", str);
	}
	lib->credmgr->flush_cache(lib->credmgr, type);

	return create_reply(NULL);
}

static void manage_command(private_vici_cred_t *this,
						   char *name, vici_command_cb_t cb, bool reg)
{
	this->dispatcher->manage_command(this->dispatcher, name,
									 reg ? cb : NULL, this);
}

/**
 * (Un-)register dispatcher functions
 */
static void manage_commands(private_vici_cred_t *this, bool reg)
{
	manage_command(this, "clear-creds", clear_creds, reg);
	manage_command(this, "flush-certs", flush_certs, reg);
	manage_command(this, "load-cert", load_cert, reg);
	manage_command(this, "load-key", load_key, reg);
	manage_command(this, "unload-key", unload_key, reg);
	manage_command(this, "get-keys", get_keys, reg);
	manage_command(this, "load-token", load_token, reg);
	manage_command(this, "load-shared", load_shared, reg);
	manage_command(this, "unload-shared", unload_shared, reg);
	manage_command(this, "get-shared", get_shared, reg);
}

METHOD(vici_cred_t, add_cert, certificate_t*,
	private_vici_cred_t *this, certificate_t *cert)
{
	return this->creds->add_cert_ref(this->creds, TRUE, cert);
}

METHOD(vici_cred_t, destroy, void,
	private_vici_cred_t *this)
{
	manage_commands(this, FALSE);

	lib->credmgr->remove_set(lib->credmgr, &this->creds->set);
	this->creds->destroy(this->creds);
	lib->credmgr->remove_set(lib->credmgr, &this->pins->set);
	this->pins->destroy(this->pins);
	free(this);
}

/**
 * See header
 */
vici_cred_t *vici_cred_create(vici_dispatcher_t *dispatcher)
{
	private_vici_cred_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_private_enumerator = (void*)return_null,
				.create_cert_enumerator = (void*)return_null,
				.create_shared_enumerator = (void*)return_null,
				.create_cdp_enumerator = (void*)return_null,
				.cache_cert = (void*)_cache_cert,
			},
			.add_cert = _add_cert,
			.destroy = _destroy,
		},
		.dispatcher = dispatcher,
		.creds = mem_cred_create(),
		.pins = mem_cred_create(),
	);

	if (lib->settings->get_bool(lib->settings, "%s.cache_crls", FALSE, lib->ns))
	{
		this->cachecrl = TRUE;
		DBG1(DBG_CFG, "crl caching to %s enabled", CRL_DIR);
	}
	lib->credmgr->add_set(lib->credmgr, &this->creds->set);
	lib->credmgr->add_set(lib->credmgr, &this->pins->set);

	manage_commands(this, TRUE);

	return &this->public;
}
