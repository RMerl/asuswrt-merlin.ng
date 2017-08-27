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

#include "cmd_creds.h"

#include <unistd.h>

#include <utils/debug.h>
#include <credentials/sets/mem_cred.h>
#include <credentials/containers/pkcs12.h>
#include <credentials/sets/callback_cred.h>

typedef struct private_cmd_creds_t private_cmd_creds_t;

/**
 * Private data of an cmd_creds_t object.
 */
struct private_cmd_creds_t {

	/**
	 * Public cmd_creds_t interface.
	 */
	cmd_creds_t public;

	/**
	 * Reused in-memory credential set
	 */
	mem_cred_t *creds;

	/**
	 * Callback credential set to get secrets
	 */
	callback_cred_t *cb;

	/**
	 * Kind of secret we recently prompted
	 */
	shared_key_type_t prompted;

	/**
	 * Path to ssh-agent socket
	 */
	char *agent;

	/**
	 * Local identity
	 */
	char *identity;
};

/**
 * Callback function to prompt for secret
 */
static shared_key_t* callback_shared(private_cmd_creds_t *this,
								shared_key_type_t type,
								identification_t *me, identification_t *other,
								id_match_t *match_me, id_match_t *match_other)
{
	shared_key_t *shared;
	char *label, *pwd = NULL;

	if (type == this->prompted)
	{
		return NULL;
	}
	switch (type)
	{
		case SHARED_EAP:
			label = "EAP password: ";
			break;
		case SHARED_IKE:
			label = "Preshared Key: ";
			break;
		case SHARED_PRIVATE_KEY_PASS:
			label = "Password: ";
			break;
		case SHARED_PIN:
			label = "PIN: ";
			break;
		default:
			return NULL;
	}
#ifdef HAVE_GETPASS
	pwd = getpass(label);
#endif
	if (!pwd || strlen(pwd) == 0)
	{
		return NULL;
	}
	this->prompted = type;
	if (match_me)
	{
		*match_me = ID_MATCH_PERFECT;
	}
	if (match_other)
	{
		*match_other = ID_MATCH_PERFECT;
	}
	shared = shared_key_create(type, chunk_clone(chunk_from_str(pwd)));
	/* cache password in case it is required more than once */
	this->creds->add_shared(this->creds, shared, NULL);
	return shared->get_ref(shared);
}

/**
 * Load a trusted certificate from path
 */
static void load_cert(private_cmd_creds_t *this, char *path)
{
	certificate_t *cert;

	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
							  BUILD_FROM_FILE, path, BUILD_END);
	if (!cert)
	{
		DBG1(DBG_CFG, "loading certificate from '%s' failed", path);
		exit(1);
	}
	this->creds->add_cert(this->creds, TRUE, cert);
}

/**
 * Load a private key of given kind from path
 */
static void load_key(private_cmd_creds_t *this, key_type_t type, char *path)
{
	private_key_t *privkey;

	privkey = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, type,
								 BUILD_FROM_FILE, path, BUILD_END);
	if (!privkey)
	{
		DBG1(DBG_CFG, "loading %N private key from '%s' failed",
			 key_type_names, type, path);
		exit(1);
	}
	this->creds->add_key(this->creds, privkey);
}

/**
 * Load a private and public key via ssh-agent
 */
static void load_agent(private_cmd_creds_t *this)
{
	private_key_t *privkey;
	public_key_t *pubkey;
	identification_t *id;
	certificate_t *cert;

	privkey = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_ANY,
								 BUILD_AGENT_SOCKET, this->agent, BUILD_END);
	if (!privkey)
	{
		DBG1(DBG_CFG, "failed to load private key from ssh-agent");
		exit(1);
	}
	pubkey = privkey->get_public_key(privkey);
	if (!pubkey)
	{
		DBG1(DBG_CFG, "failed to load public key from ssh-agent");
		privkey->destroy(privkey);
		exit(1);
	}
	id = identification_create_from_string(this->identity);
	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
							  CERT_TRUSTED_PUBKEY, BUILD_PUBLIC_KEY, pubkey,
							  BUILD_SUBJECT, id, BUILD_END);
	pubkey->destroy(pubkey);
	id->destroy(id);
	if (!cert)
	{
		DBG1(DBG_CFG, "failed to create certificate for ssh-agent public key");
		privkey->destroy(privkey);
		exit(1);
	}
	this->creds->add_cert(this->creds, TRUE, cert);
	this->creds->add_key(this->creds, privkey);
}

/**
 * Load a PKCS#12 file from path
 */
static void load_pkcs12(private_cmd_creds_t *this, char *path)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	private_key_t *key;
	container_t *container;
	pkcs12_t *pkcs12;

	container = lib->creds->create(lib->creds, CRED_CONTAINER, CONTAINER_PKCS12,
								   BUILD_FROM_FILE, path, BUILD_END);
	if (!container)
	{
		DBG1(DBG_CFG, "loading PKCS#12 file '%s' failed", path);
		exit(1);
	}
	pkcs12 = (pkcs12_t*)container;
	enumerator = pkcs12->create_cert_enumerator(pkcs12);
	while (enumerator->enumerate(enumerator, &cert))
	{
		this->creds->add_cert(this->creds, TRUE, cert->get_ref(cert));
	}
	enumerator->destroy(enumerator);
	enumerator = pkcs12->create_key_enumerator(pkcs12);
	while (enumerator->enumerate(enumerator, &key))
	{
		this->creds->add_key(this->creds, key->get_ref(key));
	}
	enumerator->destroy(enumerator);
	container->destroy(container);
}

METHOD(cmd_creds_t, handle, bool,
	private_cmd_creds_t *this, cmd_option_type_t opt, char *arg)
{
	switch (opt)
	{
		case CMD_OPT_CERT:
			load_cert(this, arg);
			break;
		case CMD_OPT_RSA:
			load_key(this, KEY_RSA, arg);
			break;
		case CMD_OPT_PKCS12:
			load_pkcs12(this, arg);
			break;
		case CMD_OPT_IDENTITY:
			this->identity = arg;
			break;
		case CMD_OPT_AGENT:
			this->agent = arg ?: getenv("SSH_AUTH_SOCK");
			if (!this->agent)
			{
				DBG1(DBG_CFG, "no ssh-agent socket defined");
				exit(1);
			}
			break;
		default:
			return FALSE;
	}
	if (this->agent && this->identity)
	{
		load_agent(this);
		/* only do this once */
		this->agent = NULL;
	}
	return TRUE;
}

METHOD(cmd_creds_t, destroy, void,
	private_cmd_creds_t *this)
{
	lib->credmgr->remove_set(lib->credmgr, &this->creds->set);
	lib->credmgr->remove_set(lib->credmgr, &this->cb->set);
	this->creds->destroy(this->creds);
	this->cb->destroy(this->cb);
	free(this);
}

/**
 * See header
 */
cmd_creds_t *cmd_creds_create()
{
	private_cmd_creds_t *this;

	INIT(this,
		.public = {
			.handle = _handle,
			.destroy = _destroy,
		},
		.creds = mem_cred_create(),
		.prompted = SHARED_ANY,
	);
	this->cb = callback_cred_create_shared((void*)callback_shared, this);

	lib->credmgr->add_set(lib->credmgr, &this->creds->set);
	lib->credmgr->add_set(lib->credmgr, &this->cb->set);

	return &this->public;
}
