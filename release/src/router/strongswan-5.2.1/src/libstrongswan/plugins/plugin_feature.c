/*
 * Copyright (C) 2012-2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
#include <stdio.h>

#include "plugin_feature.h"

#include <utils/debug.h>

ENUM(plugin_feature_names, FEATURE_NONE, FEATURE_CUSTOM,
	"NONE",
	"CRYPTER",
	"AEAD",
	"SIGNER",
	"HASHER",
	"PRF",
	"DH",
	"RNG",
	"NONCE_GEN",
	"PRIVKEY",
	"PRIVKEY_GEN",
	"PRIVKEY_SIGN",
	"PRIVKEY_DECRYPT",
	"PUBKEY",
	"PUBKEY_VERIFY",
	"PUBKEY_ENCRYPT",
	"CERT_DECODE",
	"CERT_ENCODE",
	"CONTAINER_DECODE",
	"CONTAINER_ENCODE",
	"EAP_SERVER",
	"EAP_CLIENT",
	"XAUTH_SERVER",
	"XAUTH_CLIENT",
	"DATABASE",
	"FETCHER",
	"RESOLVER",
	"CUSTOM",
);

/**
 * See header.
 */
u_int32_t plugin_feature_hash(plugin_feature_t *feature)
{
	chunk_t data;

	switch (feature->type)
	{
		case FEATURE_NONE:
		case FEATURE_RNG:
		case FEATURE_NONCE_GEN:
		case FEATURE_DATABASE:
		case FEATURE_FETCHER:
		case FEATURE_RESOLVER:
			/* put these special cases in their (type-specific) buckets */
			data = chunk_empty;
			break;
		case FEATURE_CRYPTER:
			data = chunk_from_thing(feature->arg.crypter);
			break;
		case FEATURE_AEAD:
			data = chunk_from_thing(feature->arg.aead);
			break;
		case FEATURE_SIGNER:
			data = chunk_from_thing(feature->arg.signer);
			break;
		case FEATURE_HASHER:
			data = chunk_from_thing(feature->arg.hasher);
			break;
		case FEATURE_PRF:
			data = chunk_from_thing(feature->arg.prf);
			break;
		case FEATURE_DH:
			data = chunk_from_thing(feature->arg.dh_group);
			break;
		case FEATURE_PRIVKEY:
			data = chunk_from_thing(feature->arg.privkey);
			break;
		case FEATURE_PRIVKEY_GEN:
			data = chunk_from_thing(feature->arg.privkey_gen);
			break;
		case FEATURE_PUBKEY:
			data = chunk_from_thing(feature->arg.pubkey);
			break;
		case FEATURE_PRIVKEY_SIGN:
			data = chunk_from_thing(feature->arg.privkey_sign);
			break;
		case FEATURE_PUBKEY_VERIFY:
			data = chunk_from_thing(feature->arg.pubkey_verify);
			break;
		case FEATURE_PRIVKEY_DECRYPT:
			data = chunk_from_thing(feature->arg.privkey_decrypt);
			break;
		case FEATURE_PUBKEY_ENCRYPT:
			data = chunk_from_thing(feature->arg.pubkey_encrypt);
			break;
		case FEATURE_CERT_DECODE:
		case FEATURE_CERT_ENCODE:
			data = chunk_from_thing(feature->arg.cert);
			break;
		case FEATURE_CONTAINER_DECODE:
		case FEATURE_CONTAINER_ENCODE:
			data = chunk_from_thing(feature->arg.container);
			break;
		case FEATURE_EAP_SERVER:
		case FEATURE_EAP_PEER:
			data = chunk_from_thing(feature->arg.eap);
			break;
		case FEATURE_CUSTOM:
			data = chunk_create(feature->arg.custom,
								strlen(feature->arg.custom));
			break;
		case FEATURE_XAUTH_SERVER:
		case FEATURE_XAUTH_PEER:
			data = chunk_create(feature->arg.xauth,
								strlen(feature->arg.xauth));
			break;
	}
	return chunk_hash_inc(chunk_from_thing(feature->type),
						  chunk_hash(data));
}

/**
 * See header.
 */
bool plugin_feature_matches(plugin_feature_t *a, plugin_feature_t *b)
{
	if (a->type == b->type)
	{
		switch (a->type)
		{
			case FEATURE_NONE:
				return FALSE;
			case FEATURE_CRYPTER:
				return a->arg.crypter.alg == b->arg.crypter.alg &&
					   a->arg.crypter.key_size == b->arg.crypter.key_size;
			case FEATURE_AEAD:
				return a->arg.aead.alg == b->arg.aead.alg &&
					   a->arg.aead.key_size == b->arg.aead.key_size;
			case FEATURE_SIGNER:
				return a->arg.signer == b->arg.signer;
			case FEATURE_HASHER:
				return a->arg.hasher == b->arg.hasher;
			case FEATURE_PRF:
				return a->arg.prf == b->arg.prf;
			case FEATURE_DH:
				return a->arg.dh_group == b->arg.dh_group;
			case FEATURE_RNG:
				return a->arg.rng_quality <= b->arg.rng_quality;
			case FEATURE_NONCE_GEN:
			case FEATURE_RESOLVER:
				return TRUE;
			case FEATURE_PRIVKEY:
			case FEATURE_PRIVKEY_GEN:
			case FEATURE_PUBKEY:
				return a->arg.privkey == b->arg.privkey;
			case FEATURE_PRIVKEY_SIGN:
			case FEATURE_PUBKEY_VERIFY:
				return a->arg.privkey_sign == b->arg.privkey_sign;
			case FEATURE_PRIVKEY_DECRYPT:
			case FEATURE_PUBKEY_ENCRYPT:
				return a->arg.privkey_decrypt == b->arg.privkey_decrypt;
			case FEATURE_CERT_DECODE:
			case FEATURE_CERT_ENCODE:
				return a->arg.cert == b->arg.cert;
			case FEATURE_CONTAINER_DECODE:
			case FEATURE_CONTAINER_ENCODE:
				return a->arg.container == b->arg.container;
			case FEATURE_EAP_SERVER:
			case FEATURE_EAP_PEER:
				return a->arg.eap == b->arg.eap;
			case FEATURE_DATABASE:
				return a->arg.database == DB_ANY ||
					   a->arg.database == b->arg.database;
			case FEATURE_FETCHER:
				return a->arg.fetcher == NULL ||
					   streq(a->arg.fetcher, b->arg.fetcher);
			case FEATURE_CUSTOM:
				return streq(a->arg.custom, b->arg.custom);
			case FEATURE_XAUTH_SERVER:
			case FEATURE_XAUTH_PEER:
				return streq(a->arg.xauth, b->arg.xauth);
		}
	}
	return FALSE;
}

/**
 * See header.
 */
bool plugin_feature_equals(plugin_feature_t *a, plugin_feature_t *b)
{
	if (a->type == b->type)
	{
		switch (a->type)
		{
			case FEATURE_NONE:
			case FEATURE_CRYPTER:
			case FEATURE_AEAD:
			case FEATURE_SIGNER:
			case FEATURE_HASHER:
			case FEATURE_PRF:
			case FEATURE_DH:
			case FEATURE_NONCE_GEN:
			case FEATURE_RESOLVER:
			case FEATURE_PRIVKEY:
			case FEATURE_PRIVKEY_GEN:
			case FEATURE_PUBKEY:
			case FEATURE_PRIVKEY_SIGN:
			case FEATURE_PUBKEY_VERIFY:
			case FEATURE_PRIVKEY_DECRYPT:
			case FEATURE_PUBKEY_ENCRYPT:
			case FEATURE_CERT_DECODE:
			case FEATURE_CERT_ENCODE:
			case FEATURE_CONTAINER_DECODE:
			case FEATURE_CONTAINER_ENCODE:
			case FEATURE_EAP_SERVER:
			case FEATURE_EAP_PEER:
			case FEATURE_CUSTOM:
			case FEATURE_XAUTH_SERVER:
			case FEATURE_XAUTH_PEER:
				return plugin_feature_matches(a, b);
			case FEATURE_RNG:
				return a->arg.rng_quality == b->arg.rng_quality;
			case FEATURE_DATABASE:
				return a->arg.database == b->arg.database;
			case FEATURE_FETCHER:
				if (a->arg.fetcher && b->arg.fetcher)
				{
					return streq(a->arg.fetcher, b->arg.fetcher);
				}
				return !a->arg.fetcher && !b->arg.fetcher;
		}
	}
	return FALSE;
}

/**
 * See header.
 */
char* plugin_feature_get_string(plugin_feature_t *feature)
{
	char *str = NULL;

	if (feature->kind == FEATURE_REGISTER)
	{
		return strdup("(register function)");
	}
	switch (feature->type)
	{
		case FEATURE_NONE:
			return strdup("NONE");
		case FEATURE_CRYPTER:
			if (asprintf(&str, "%N:%N-%d", plugin_feature_names, feature->type,
					encryption_algorithm_names, feature->arg.crypter.alg,
					feature->arg.crypter.key_size) > 0)
			{
				return str;
			}
			break;
		case FEATURE_AEAD:
			if (asprintf(&str, "%N:%N-%d", plugin_feature_names, feature->type,
					encryption_algorithm_names, feature->arg.aead.alg,
					feature->arg.aead.key_size) > 0)
			{
				return str;
			}
			break;
		case FEATURE_SIGNER:
			if (asprintf(&str, "%N:%N", plugin_feature_names, feature->type,
					integrity_algorithm_names, feature->arg.signer) > 0)
			{
				return str;
			}
			break;
		case FEATURE_HASHER:
			if (asprintf(&str, "%N:%N", plugin_feature_names, feature->type,
					hash_algorithm_names, feature->arg.hasher) > 0)
			{
				return str;
			}
			break;
		case FEATURE_PRF:
			if (asprintf(&str, "%N:%N", plugin_feature_names, feature->type,
					pseudo_random_function_names, feature->arg.prf) > 0)
			{
				return str;
			}
			break;
		case FEATURE_DH:
			if (asprintf(&str, "%N:%N", plugin_feature_names, feature->type,
					diffie_hellman_group_names, feature->arg.dh_group) > 0)
			{
				return str;
			}
			break;
		case FEATURE_RNG:
			if (asprintf(&str, "%N:%N", plugin_feature_names, feature->type,
					rng_quality_names, feature->arg.rng_quality) > 0)
			{
				return str;
			}
			break;
		case FEATURE_NONCE_GEN:
		case FEATURE_RESOLVER:
			if (asprintf(&str, "%N", plugin_feature_names, feature->type) > 0)
			{
				return str;
			}
			break;
		case FEATURE_PRIVKEY:
		case FEATURE_PRIVKEY_GEN:
		case FEATURE_PUBKEY:
			if (asprintf(&str, "%N:%N", plugin_feature_names, feature->type,
					key_type_names, feature->arg.privkey) > 0)
			{
				return str;
			}
			break;
		case FEATURE_PRIVKEY_SIGN:
		case FEATURE_PUBKEY_VERIFY:
			if (asprintf(&str, "%N:%N", plugin_feature_names, feature->type,
					signature_scheme_names, feature->arg.privkey_sign) > 0)
			{
				return str;
			}
			break;
		case FEATURE_PRIVKEY_DECRYPT:
		case FEATURE_PUBKEY_ENCRYPT:
			if (asprintf(&str, "%N:%N", plugin_feature_names, feature->type,
					encryption_scheme_names, feature->arg.privkey_decrypt) > 0)
			{
				return str;
			}
			break;
		case FEATURE_CERT_DECODE:
		case FEATURE_CERT_ENCODE:
			if (asprintf(&str, "%N:%N", plugin_feature_names, feature->type,
					certificate_type_names, feature->arg.cert) > 0)
			{
				return str;
			}
			break;
		case FEATURE_CONTAINER_DECODE:
		case FEATURE_CONTAINER_ENCODE:
			if (asprintf(&str, "%N:%N", plugin_feature_names, feature->type,
					container_type_names, feature->arg.container) > 0)
			{
				return str;
			}
			break;
		case FEATURE_EAP_SERVER:
		case FEATURE_EAP_PEER:
			if (asprintf(&str, "%N:%N", plugin_feature_names, feature->type,
					eap_type_short_names, feature->arg.eap) > 0)
			{
				return str;
			}
			break;
		case FEATURE_DATABASE:
			if (asprintf(&str, "%N:%N", plugin_feature_names, feature->type,
					db_driver_names, feature->arg.database) > 0)
			{
				return str;
			}
			break;
		case FEATURE_FETCHER:
			if (asprintf(&str, "%N:%s", plugin_feature_names, feature->type,
					feature->arg.fetcher) > 0)
			{
				return str;
			}
			break;
		case FEATURE_CUSTOM:
			if (asprintf(&str, "%N:%s", plugin_feature_names, feature->type,
					feature->arg.custom) > 0)
			{
				return str;
			}
			break;
		case FEATURE_XAUTH_SERVER:
		case FEATURE_XAUTH_PEER:
			if (asprintf(&str, "%N:%s", plugin_feature_names, feature->type,
					feature->arg.xauth) > 0)
			{
				return str;
			}
			break;
	}
	if (!str)
	{
		str = strdup("(invalid)");
	}
	return str;
}

/**
 * See header.
 */
bool plugin_feature_load(plugin_t *plugin, plugin_feature_t *feature,
						 plugin_feature_t *reg)
{
	char *name;

	if (!reg)
	{	/* noting to do for this feature */
		return TRUE;
	}
	if (reg->kind == FEATURE_CALLBACK)
	{
		if (!reg->arg.cb.f ||
			 reg->arg.cb.f(plugin, feature, TRUE, reg->arg.cb.data))
		{
			return TRUE;
		}
		return FALSE;
	}
	name = plugin->get_name(plugin);
	switch (feature->type)
	{
		case FEATURE_CRYPTER:
			lib->crypto->add_crypter(lib->crypto, feature->arg.crypter.alg,
								name, reg->arg.reg.f);
			break;
		case FEATURE_AEAD:
			lib->crypto->add_aead(lib->crypto, feature->arg.aead.alg,
								name, reg->arg.reg.f);
			break;
		case FEATURE_SIGNER:
			lib->crypto->add_signer(lib->crypto, feature->arg.signer,
								name, reg->arg.reg.f);
			break;
		case FEATURE_HASHER:
			lib->crypto->add_hasher(lib->crypto, feature->arg.hasher,
								name, reg->arg.reg.f);
			break;
		case FEATURE_PRF:
			lib->crypto->add_prf(lib->crypto, feature->arg.prf,
								name, reg->arg.reg.f);
			break;
		case FEATURE_DH:
			lib->crypto->add_dh(lib->crypto, feature->arg.dh_group,
								name, reg->arg.reg.f);
			break;
		case FEATURE_RNG:
			lib->crypto->add_rng(lib->crypto, feature->arg.rng_quality,
								name, reg->arg.reg.f);
			break;
		case FEATURE_NONCE_GEN:
			lib->crypto->add_nonce_gen(lib->crypto,
								name, reg->arg.reg.f);
			break;
		case FEATURE_PRIVKEY:
		case FEATURE_PRIVKEY_GEN:
			lib->creds->add_builder(lib->creds, CRED_PRIVATE_KEY,
								feature->arg.privkey, reg->arg.reg.final,
								reg->arg.reg.f);
			break;
		case FEATURE_PUBKEY:
			lib->creds->add_builder(lib->creds, CRED_PUBLIC_KEY,
								feature->arg.pubkey, reg->arg.reg.final,
								reg->arg.reg.f);
			break;
		case FEATURE_CERT_DECODE:
		case FEATURE_CERT_ENCODE:
			lib->creds->add_builder(lib->creds, CRED_CERTIFICATE,
								feature->arg.cert, reg->arg.reg.final,
								reg->arg.reg.f);
			break;
		case FEATURE_CONTAINER_DECODE:
		case FEATURE_CONTAINER_ENCODE:
			lib->creds->add_builder(lib->creds, CRED_CONTAINER,
								feature->arg.container, reg->arg.reg.final,
								reg->arg.reg.f);
			break;
		case FEATURE_DATABASE:
			lib->db->add_database(lib->db, reg->arg.reg.f);
			break;
		case FEATURE_FETCHER:
			lib->fetcher->add_fetcher(lib->fetcher, reg->arg.reg.f,
									  feature->arg.fetcher);
			break;
		case FEATURE_RESOLVER:
			lib->resolver->add_resolver(lib->resolver, reg->arg.reg.f);
			break;
		default:
			break;
	}
	return TRUE;
}

/**
 * See header.
 */
bool plugin_feature_unload(plugin_t *plugin, plugin_feature_t *feature,
						   plugin_feature_t *reg)
{
	if (!reg)
	{	/* noting to do for this feature */
		return TRUE;
	}
	if (reg->kind == FEATURE_CALLBACK)
	{
		if (!reg->arg.cb.f ||
			 reg->arg.cb.f(plugin, feature, FALSE, reg->arg.cb.data))
		{
			return TRUE;
		}
		return FALSE;
	}
	switch (feature->type)
	{
		case FEATURE_CRYPTER:
			lib->crypto->remove_crypter(lib->crypto, reg->arg.reg.f);
			break;
		case FEATURE_AEAD:
			lib->crypto->remove_aead(lib->crypto, reg->arg.reg.f);
			break;
		case FEATURE_SIGNER:
			lib->crypto->remove_signer(lib->crypto, reg->arg.reg.f);
			break;
		case FEATURE_HASHER:
			lib->crypto->remove_hasher(lib->crypto, reg->arg.reg.f);
			break;
		case FEATURE_PRF:
			lib->crypto->remove_prf(lib->crypto, reg->arg.reg.f);
			break;
		case FEATURE_DH:
			lib->crypto->remove_dh(lib->crypto, reg->arg.reg.f);
			break;
		case FEATURE_RNG:
			lib->crypto->remove_rng(lib->crypto, reg->arg.reg.f);
			break;
		case FEATURE_NONCE_GEN:
			lib->crypto->remove_nonce_gen(lib->crypto, reg->arg.reg.f);
			break;
		case FEATURE_PRIVKEY:
		case FEATURE_PRIVKEY_GEN:
			lib->creds->remove_builder(lib->creds, reg->arg.reg.f);
			break;
		case FEATURE_PUBKEY:
			lib->creds->remove_builder(lib->creds, reg->arg.reg.f);
			break;
		case FEATURE_CERT_DECODE:
		case FEATURE_CERT_ENCODE:
			lib->creds->remove_builder(lib->creds, reg->arg.reg.f);
			break;
		case FEATURE_CONTAINER_DECODE:
		case FEATURE_CONTAINER_ENCODE:
			lib->creds->remove_builder(lib->creds, reg->arg.reg.f);
			break;
		case FEATURE_DATABASE:
			lib->db->remove_database(lib->db, reg->arg.reg.f);
			break;
		case FEATURE_FETCHER:
			lib->fetcher->remove_fetcher(lib->fetcher, reg->arg.reg.f);
			break;
		case FEATURE_RESOLVER:
			lib->resolver->remove_resolver(lib->resolver, reg->arg.reg.f);
			break;
		default:
			break;
	}
	return TRUE;
}
