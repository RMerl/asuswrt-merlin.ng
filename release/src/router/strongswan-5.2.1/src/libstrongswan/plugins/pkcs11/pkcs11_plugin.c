/*
 * Copyright (C) 2011 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "pkcs11_plugin.h"

#include <library.h>
#include <utils/debug.h>
#include <collections/linked_list.h>
#include <threading/mutex.h>
#include <threading/rwlock.h>

#include "pkcs11_manager.h"
#include "pkcs11_creds.h"
#include "pkcs11_private_key.h"
#include "pkcs11_public_key.h"
#include "pkcs11_hasher.h"
#include "pkcs11_rng.h"
#include "pkcs11_dh.h"

typedef struct private_pkcs11_plugin_t private_pkcs11_plugin_t;

/**
 * private data of pkcs11_plugin
 */
struct private_pkcs11_plugin_t {

	/**
	 * public functions
	 */
	pkcs11_plugin_t public;

	/**
	 * PKCS#11 library/slot manager
	 */
	pkcs11_manager_t *manager;

	/**
	 * List of credential sets, pkcs11_creds_t
	 */
	linked_list_t *creds;

	/**
	 * mutex to lock list
	 */
	mutex_t *mutex;

	/**
	 * TRUE if events from tokens are to be handled
	 */
	bool handle_events;

	/**
	 * Lock for the above flag
	 */
	rwlock_t *handle_events_lock;
};

/**
 * Token event callback function
 */
static void token_event_cb(private_pkcs11_plugin_t *this, pkcs11_library_t *p11,
						   CK_SLOT_ID slot, bool add)
{
	enumerator_t *enumerator;
	pkcs11_creds_t *creds, *found = NULL;

	this->handle_events_lock->read_lock(this->handle_events_lock);
	if (add && this->handle_events)
	{
		if (lib->settings->get_bool(lib->settings,
								"%s.plugins.pkcs11.modules.%s.load_certs",
								TRUE, lib->ns, p11->get_name(p11)))
		{
			creds = pkcs11_creds_create(p11, slot);
			if (creds)
			{
				this->mutex->lock(this->mutex);
				this->creds->insert_last(this->creds, creds);
				this->mutex->unlock(this->mutex);
				lib->credmgr->add_set(lib->credmgr, &creds->set);
			}
		}
	}
	else if (this->handle_events)
	{
		this->mutex->lock(this->mutex);
		enumerator = this->creds->create_enumerator(this->creds);
		while (enumerator->enumerate(enumerator, &creds))
		{
			if (creds->get_library(creds) == p11 &&
				creds->get_slot(creds) == slot)
			{
				found = creds;
				this->creds->remove_at(this->creds, enumerator);
				break;
			}
		}
		enumerator->destroy(enumerator);
		this->mutex->unlock(this->mutex);

		if (found)
		{
			lib->credmgr->remove_set(lib->credmgr, &found->set);
			found->destroy(found);
			/* flush the cache after a token is gone */
			lib->credmgr->flush_cache(lib->credmgr, CERT_X509);
		}
	}
	this->handle_events_lock->unlock(this->handle_events_lock);
}

METHOD(plugin_t, get_name, char*,
	private_pkcs11_plugin_t *this)
{
	return "pkcs11";
}

/**
 * Load/unload certificates from tokens.
 */
static bool handle_certs(private_pkcs11_plugin_t *this,
						 plugin_feature_t *feature, bool reg, void *data)
{
	this->handle_events_lock->write_lock(this->handle_events_lock);
	this->handle_events = reg;
	this->handle_events_lock->unlock(this->handle_events_lock);

	if (reg)
	{
		enumerator_t *enumerator;
		pkcs11_library_t *p11;
		CK_SLOT_ID slot;

		enumerator = this->manager->create_token_enumerator(this->manager);
		while (enumerator->enumerate(enumerator, &p11, &slot))
		{
			token_event_cb(this, p11, slot, TRUE);
		}
		enumerator->destroy(enumerator);

		lib->creds->add_builder(lib->creds, CRED_CERTIFICATE,
								CERT_X509, FALSE, (void*)pkcs11_creds_load);
	}
	else
	{
		pkcs11_creds_t *creds;

		while (this->creds->remove_last(this->creds, (void**)&creds) == SUCCESS)
		{
			lib->credmgr->remove_set(lib->credmgr, &creds->set);
			creds->destroy(creds);
		}

		lib->creds->remove_builder(lib->creds, (void*)pkcs11_creds_load);
	}
	return TRUE;
}

METHOD(plugin_t, reload, bool,
	private_pkcs11_plugin_t *this)
{
	if (lib->settings->get_bool(lib->settings, "%s.plugins.pkcs11.reload_certs",
								FALSE, lib->ns))
	{
		DBG1(DBG_CFG, "reloading certificates from PKCS#11 tokens");
		handle_certs(this, NULL, FALSE, NULL);
		handle_certs(this, NULL, TRUE, NULL);
		return TRUE;
	}
	return FALSE;
}

METHOD(plugin_t, get_features, int,
	private_pkcs11_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f_hash[] = {
		PLUGIN_REGISTER(HASHER, pkcs11_hasher_create),
			PLUGIN_PROVIDE(HASHER, HASH_MD2),
			PLUGIN_PROVIDE(HASHER, HASH_MD5),
			PLUGIN_PROVIDE(HASHER, HASH_SHA1),
			PLUGIN_PROVIDE(HASHER, HASH_SHA256),
			PLUGIN_PROVIDE(HASHER, HASH_SHA384),
			PLUGIN_PROVIDE(HASHER, HASH_SHA512),
	};
	static plugin_feature_t f_dh[] = {
		PLUGIN_REGISTER(DH, pkcs11_dh_create),
			PLUGIN_PROVIDE(DH, MODP_2048_BIT),
			PLUGIN_PROVIDE(DH, MODP_2048_224),
			PLUGIN_PROVIDE(DH, MODP_2048_256),
			PLUGIN_PROVIDE(DH, MODP_1536_BIT),
			PLUGIN_PROVIDE(DH, MODP_3072_BIT),
			PLUGIN_PROVIDE(DH, MODP_4096_BIT),
			PLUGIN_PROVIDE(DH, MODP_6144_BIT),
			PLUGIN_PROVIDE(DH, MODP_8192_BIT),
			PLUGIN_PROVIDE(DH, MODP_1024_BIT),
			PLUGIN_PROVIDE(DH, MODP_1024_160),
			PLUGIN_PROVIDE(DH, MODP_768_BIT),
			PLUGIN_PROVIDE(DH, MODP_CUSTOM),
	};
	static plugin_feature_t f_ecdh[] = {
		PLUGIN_REGISTER(DH, pkcs11_dh_create),
			PLUGIN_PROVIDE(DH, ECP_192_BIT),
			PLUGIN_PROVIDE(DH, ECP_224_BIT),
			PLUGIN_PROVIDE(DH, ECP_256_BIT),
			PLUGIN_PROVIDE(DH, ECP_384_BIT),
			PLUGIN_PROVIDE(DH, ECP_521_BIT),
	};
	static plugin_feature_t f_rng[] = {
		PLUGIN_REGISTER(RNG, pkcs11_rng_create),
			PLUGIN_PROVIDE(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(RNG, RNG_TRUE),
	};
	static plugin_feature_t f_privkey[] = {
		PLUGIN_REGISTER(PRIVKEY, pkcs11_private_key_connect, FALSE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ANY),
	};
	static plugin_feature_t f_pubkey[] = {
		PLUGIN_REGISTER(PUBKEY, pkcs11_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_RSA),
			PLUGIN_PROVIDE(PUBKEY, KEY_ECDSA),
	};
	static plugin_feature_t f_manager[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)handle_certs, NULL),
			PLUGIN_PROVIDE(CUSTOM, "pkcs11-certs"),
				PLUGIN_DEPENDS(CERT_DECODE, CERT_X509),
	};
	static plugin_feature_t f[countof(f_hash) + countof(f_dh) + countof(f_rng) +
							  countof(f_ecdh) + countof(f_privkey) +
							  countof(f_pubkey) + countof(f_manager)] = {};
	static int count = 0;

	if (!count)
	{	/* initialize only once */
		bool use_ecc = lib->settings->get_bool(lib->settings,
								"%s.plugins.pkcs11.use_ecc", FALSE, lib->ns);
		plugin_features_add(f, f_manager, countof(f_manager), &count);
		/* private key handling for EC keys is not disabled by use_ecc */
		plugin_features_add(f, f_privkey, countof(f_privkey), &count);
		if (lib->settings->get_bool(lib->settings,
								"%s.plugins.pkcs11.use_pubkey", FALSE, lib->ns))
		{
			plugin_features_add(f, f_pubkey, countof(f_pubkey) - (use_ecc ? 0 : 1),
								&count);
		}
		if (lib->settings->get_bool(lib->settings,
								"%s.plugins.pkcs11.use_hasher", FALSE, lib->ns))
		{
			plugin_features_add(f, f_hash, countof(f_hash), &count);
		}
		if (lib->settings->get_bool(lib->settings,
								"%s.plugins.pkcs11.use_rng", FALSE, lib->ns))
		{
			plugin_features_add(f, f_rng, countof(f_rng), &count);
		}
		if (lib->settings->get_bool(lib->settings,
								"%s.plugins.pkcs11.use_dh", FALSE, lib->ns))
		{
			plugin_features_add(f, f_dh, countof(f_dh), &count);
			if (use_ecc)
			{
				plugin_features_add(f, f_ecdh, countof(f_ecdh), &count);
			}
		}
	}
	*features = f;
	return count;
}

METHOD(plugin_t, destroy, void,
	private_pkcs11_plugin_t *this)
{
	lib->set(lib, "pkcs11-manager", NULL);
	this->manager->destroy(this->manager);
	this->creds->destroy(this->creds);
	this->mutex->destroy(this->mutex);
	this->handle_events_lock->destroy(this->handle_events_lock);
	free(this);
}

/*
 * see header file
 */
plugin_t *pkcs11_plugin_create()
{
	private_pkcs11_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.reload = _reload,
				.destroy = _destroy,
			},
		},
		.creds = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.handle_events_lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	this->manager = pkcs11_manager_create((void*)token_event_cb, this);
	lib->set(lib, "pkcs11-manager", this->manager);

	return &this->public.plugin;
}
