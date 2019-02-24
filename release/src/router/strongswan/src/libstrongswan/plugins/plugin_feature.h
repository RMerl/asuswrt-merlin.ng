/*
 * Copyright (C) 2012-2015 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

/**
 * @defgroup plugin_feature plugin_feature
 * @{ @ingroup plugins
 */

#ifndef PLUGIN_FEATURE_H_
#define PLUGIN_FEATURE_H_

typedef struct plugin_feature_t plugin_feature_t;

#include <library.h>
#include <eap/eap.h>
#include <plugins/plugin.h>
#include <credentials/containers/container.h>

/**
 * Callback function of a plugin to (un-)register a specified feature.
 *
 * @param plugin			plugin instance
 * @param feature			feature to register
 * @param reg				TRUE to register, FALSE to unregister
 * @param cb_data			user data passed with callback function
 * @return					TRUE if registered successfully
 */
typedef bool (*plugin_feature_callback_t)(plugin_t *plugin,
										  plugin_feature_t *feature,
										  bool reg,void *cb_data);

/**
 * Feature a plugin provides or depends on, including registration functions.
 *
 * Each plugin returns a list of plugin features, allowing the plugin loader
 * to resolve dependencies and register the feature. FEATURE_PROVIDE defines
 * features provided by the plugin, hard (DEPENDS) or soft (SDEPEND) dependency
 * specified is related to the previously defined PROVIDE feature.
 * If a plugin feature requires to hook in functionality into the library
 * or a daemon, it can use REGISTER or CALLBACK entries. Each PROVIDE feature
 * uses the REGISTER/CALLBACK entry defined previously. The REGISTER entry
 * defines a common feature registration function directly passed to the
 * associated manager or factory (crypto/credential factory etc.). A callback
 * function is more generic allows the loader to invoke a callback to do
 * the registration. PROVIDE features that do not use a registration or callback
 * function must be listed before any REGISTER/CALLBACK entry, or use the NOOP
 * helper macro.
 *
 * To conveniently create feature lists, use the macros PLUGIN_REGISTER,
 * PLUGIN_CALLBACK, PLUGIN_NOOP, PLUGIN_PROVIDE, PLUGIN_DEPENDS and
 * PLUGIN_SDEPEND. Use indentation to show how the registration functions
 * and dependencies are related to a provided feature, such as:
 *
 * @verbatim
	// two features, one with two dependencies, both use a callback to register
	PLUGIN_CALLBACK(...),
		PLUGIN_PROVIDE(...),
			PLUGIN_DEPENDS(...),
			PLUGIN_SDEPEND(...),
		PLUGIN_PROVIDE(...),
	// common constructor to register for a feature with one dependency
	PLUGIN_REGISTER(...),
		PLUGIN_PROVIDE(...),
			PLUGIN_DEPENDS(...),
	// feature that does not use a registration function
	PLUGIN_NOOP,
		PLUGIN_PROVIDE(...),
	@endverbatim
 */
struct plugin_feature_t {
	/** kind of entry */
	enum {
		/* plugin provides this feature */
		FEATURE_PROVIDE,
		/* a feature depends on this feature, hard dependency */
		FEATURE_DEPENDS,
		/* a feature can optionally use this feature, soft dependency */
		FEATURE_SDEPEND,
		/* register the specified function for all following features */
		FEATURE_REGISTER,
		/* use a callback to register all following features */
		FEATURE_CALLBACK,
	} kind;
	/* type of feature */
	enum {
		/** not a feature */
		FEATURE_NONE,
		/** crypter_t */
		FEATURE_CRYPTER,
		/** aead_t */
		FEATURE_AEAD,
		/** signer_t */
		FEATURE_SIGNER,
		/** hasher_t */
		FEATURE_HASHER,
		/** prf_t */
		FEATURE_PRF,
		/** xof_t */
		FEATURE_XOF,
		/** diffie_hellman_t */
		FEATURE_DH,
		/** rng_t */
		FEATURE_RNG,
		/** nonce_gen_t */
		FEATURE_NONCE_GEN,
		/** generic private key support */
		FEATURE_PRIVKEY,
		/** generating new private keys */
		FEATURE_PRIVKEY_GEN,
		/** private_key_t->sign() */
		FEATURE_PRIVKEY_SIGN,
		/** private_key_t->decrypt() */
		FEATURE_PRIVKEY_DECRYPT,
		/** generic public key support */
		FEATURE_PUBKEY,
		/** public_key_t->verify() */
		FEATURE_PUBKEY_VERIFY,
		/** public_key_t->encrypt() */
		FEATURE_PUBKEY_ENCRYPT,
		/** parsing certificates */
		FEATURE_CERT_DECODE,
		/** generating certificates */
		FEATURE_CERT_ENCODE,
		/** parsing containers */
		FEATURE_CONTAINER_DECODE,
		/** generating containers */
		FEATURE_CONTAINER_ENCODE,
		/** EAP server implementation */
		FEATURE_EAP_SERVER,
		/** EAP peer implementation */
		FEATURE_EAP_PEER,
		/** XAuth server implementation */
		FEATURE_XAUTH_SERVER,
		/** XAuth peer implementation */
		FEATURE_XAUTH_PEER,
		/** database_t */
		FEATURE_DATABASE,
		/** fetcher_t */
		FEATURE_FETCHER,
		/** resolver_t */
		FEATURE_RESOLVER,
		/** custom feature, described with a string */
		FEATURE_CUSTOM,
	} type;
	/** More specific data for each type */
	union {
		/** FEATURE_CRYPTER */
		struct {
			encryption_algorithm_t alg;
			size_t key_size;
		} crypter;
		/** FEATURE_AEAD */
		struct {
			encryption_algorithm_t alg;
			size_t key_size;
		} aead;
		/** FEATURE_SIGNER */
		integrity_algorithm_t signer;
		/** FEATURE_PRF */
		pseudo_random_function_t prf;
		/** FEATURE_XOFF */
		ext_out_function_t xof;
		/** FEATURE_HASHER */
		hash_algorithm_t hasher;
		/** FEATURE_DH */
		diffie_hellman_group_t dh_group;
		/** FEATURE_RNG */
		rng_quality_t rng_quality;
		/** FEATURE_PRIVKEY */
		key_type_t privkey;
		/** FEATURE_PRIVKEY_GEN */
		key_type_t privkey_gen;
		/** FEATURE_PRIVKEY_SIGN */
		signature_scheme_t privkey_sign;
		/** FEATURE_PRIVKEY_DECRYPT */
		encryption_scheme_t privkey_decrypt;
		/** FEATURE_PUBKEY */
		key_type_t pubkey;
		/** FEATURE_PUBKEY_VERIFY */
		signature_scheme_t pubkey_verify;
		/** FEATURE_PUBKEY_ENCRYPT */
		encryption_scheme_t pubkey_encrypt;
		/** FEATURE_CERT_DECODE/ENCODE */
		certificate_type_t cert;
		/** FEATURE_CONTAINER_DECODE/ENCODE */
		container_type_t container;
		/** FEATURE_EAP_SERVER/CLIENT */
		eap_vendor_type_t eap;
		/** FEATURE_DATABASE */
		db_driver_t database;
		/** FEATURE_FETCHER */
		char *fetcher;
		/** FEATURE_CUSTOM */
		char *custom;
		/** FEATURE_XAUTH_SERVER/CLIENT */
		char *xauth;

		/** FEATURE_REGISTER */
		struct {
			/** final flag to pass for builder_function_t */
			bool final;
			/** feature specific function to register for this type */
			void *f;
		} reg;

		/** FEATURE_CALLBACK */
		struct {
			/** callback function to invoke for registration */
			plugin_feature_callback_t f;
			/** data to pass to callback */
			void *data;
		} cb;
	} arg;
};

#define FEATURE(kind, type, ...) _PLUGIN_FEATURE_##type(kind, __VA_ARGS__)

/**
 * Define function to register directly for all upcoming features.
 *
 * @param type		feature type to register
 * @param f			type specific function to register
 * @param ...		type specific additional arguments
 */
#define PLUGIN_REGISTER(type, f, ...) _PLUGIN_FEATURE_REGISTER_##type(type, f, ##__VA_ARGS__)

/**
 * Define a callback to invoke for registering all upcoming features.
 *
 * @param cb		type specific callback function to register
 * @param data		data pointer to pass to callback
 */
#define PLUGIN_CALLBACK(cb, data) _PLUGIN_FEATURE_CALLBACK(cb, data)

/**
 * The upcoming features use neither a callback nor a register function.
 */
#define PLUGIN_NOOP _PLUGIN_FEATURE_CALLBACK(NULL, NULL)

/**
 * Define a feature the plugin provides.
 *
 * @param type		feature type to provide
 * @param ...		type specific arguments
 */
#define PLUGIN_PROVIDE(type, ...) _PLUGIN_FEATURE_##type(PROVIDE, __VA_ARGS__)

/**
 * Define a hard dependency for the previously defined feature.
 *
 * @param type		feature type to provide
 * @param ...		type specific arguments
 */
#define PLUGIN_DEPENDS(type, ...) _PLUGIN_FEATURE_##type(DEPENDS, __VA_ARGS__)

/**
 * Define a soft dependency for the previously defined feature.
 *
 * @param type		feature type to provide
 * @param ...		type specific arguments
 */
#define PLUGIN_SDEPEND(type, ...) _PLUGIN_FEATURE_##type(SDEPEND, __VA_ARGS__)

#define __PLUGIN_FEATURE(kind, type, ...)					(plugin_feature_t){ FEATURE_##kind, FEATURE_##type, { __VA_ARGS__ }}
#define _PLUGIN_FEATURE_CRYPTER(kind, alg, size)			__PLUGIN_FEATURE(kind, CRYPTER, .crypter = { alg, size })
#define _PLUGIN_FEATURE_AEAD(kind, alg, size)				__PLUGIN_FEATURE(kind, AEAD, .aead = { alg, size })
#define _PLUGIN_FEATURE_SIGNER(kind, alg)					__PLUGIN_FEATURE(kind, SIGNER, .signer = alg)
#define _PLUGIN_FEATURE_HASHER(kind, alg)					__PLUGIN_FEATURE(kind, HASHER, .hasher = alg)
#define _PLUGIN_FEATURE_PRF(kind, alg)						__PLUGIN_FEATURE(kind, PRF, .prf = alg)
#define _PLUGIN_FEATURE_XOF(kind, alg)						__PLUGIN_FEATURE(kind, XOF, .xof = alg)
#define _PLUGIN_FEATURE_DH(kind, group)						__PLUGIN_FEATURE(kind, DH, .dh_group = group)
#define _PLUGIN_FEATURE_RNG(kind, quality)					__PLUGIN_FEATURE(kind, RNG, .rng_quality = quality)
#define _PLUGIN_FEATURE_NONCE_GEN(kind, ...)				__PLUGIN_FEATURE(kind, NONCE_GEN, .custom = NULL)
#define _PLUGIN_FEATURE_PRIVKEY(kind, type)					__PLUGIN_FEATURE(kind, PRIVKEY, .privkey = type)
#define _PLUGIN_FEATURE_PRIVKEY_GEN(kind, type)				__PLUGIN_FEATURE(kind, PRIVKEY_GEN, .privkey_gen = type)
#define _PLUGIN_FEATURE_PRIVKEY_SIGN(kind, scheme)			__PLUGIN_FEATURE(kind, PRIVKEY_SIGN, .privkey_sign = scheme)
#define _PLUGIN_FEATURE_PRIVKEY_DECRYPT(kind, scheme)		__PLUGIN_FEATURE(kind, PRIVKEY_DECRYPT, .privkey_decrypt = scheme)
#define _PLUGIN_FEATURE_PUBKEY(kind, type)					__PLUGIN_FEATURE(kind, PUBKEY, .pubkey = type)
#define _PLUGIN_FEATURE_PUBKEY_VERIFY(kind, scheme)			__PLUGIN_FEATURE(kind, PUBKEY_VERIFY, .pubkey_verify = scheme)
#define _PLUGIN_FEATURE_PUBKEY_ENCRYPT(kind, scheme)		__PLUGIN_FEATURE(kind, PUBKEY_ENCRYPT, .pubkey_encrypt = scheme)
#define _PLUGIN_FEATURE_CERT_DECODE(kind, type)				__PLUGIN_FEATURE(kind, CERT_DECODE, .cert = type)
#define _PLUGIN_FEATURE_CERT_ENCODE(kind, type)				__PLUGIN_FEATURE(kind, CERT_ENCODE, .cert = type)
#define _PLUGIN_FEATURE_CONTAINER_DECODE(kind, type)		__PLUGIN_FEATURE(kind, CONTAINER_DECODE, .container = type)
#define _PLUGIN_FEATURE_CONTAINER_ENCODE(kind, type)		__PLUGIN_FEATURE(kind, CONTAINER_ENCODE, .container = type)
#define _PLUGIN_FEATURE_EAP_SERVER(kind, type)				_PLUGIN_FEATURE_EAP_SERVER_VENDOR(kind, type, 0)
#define _PLUGIN_FEATURE_EAP_PEER(kind, type)				_PLUGIN_FEATURE_EAP_PEER_VENDOR(kind, type, 0)
#define _PLUGIN_FEATURE_EAP_SERVER_VENDOR(kind, type, vendor)__PLUGIN_FEATURE(kind, EAP_SERVER, .eap = { type, vendor })
#define _PLUGIN_FEATURE_EAP_PEER_VENDOR(kind, type, vendor)	__PLUGIN_FEATURE(kind, EAP_PEER, .eap = { type, vendor })
#define _PLUGIN_FEATURE_DATABASE(kind, type)				__PLUGIN_FEATURE(kind, DATABASE, .database = type)
#define _PLUGIN_FEATURE_FETCHER(kind, type)					__PLUGIN_FEATURE(kind, FETCHER, .fetcher = type)
#define _PLUGIN_FEATURE_RESOLVER(kind, ...)					__PLUGIN_FEATURE(kind, RESOLVER, .custom = NULL)
#define _PLUGIN_FEATURE_CUSTOM(kind, name)					__PLUGIN_FEATURE(kind, CUSTOM, .custom = name)
#define _PLUGIN_FEATURE_XAUTH_SERVER(kind, name)			__PLUGIN_FEATURE(kind, XAUTH_SERVER, .xauth = name)
#define _PLUGIN_FEATURE_XAUTH_PEER(kind, name)				__PLUGIN_FEATURE(kind, XAUTH_PEER, .xauth = name)

#define __PLUGIN_FEATURE_REGISTER(type, _f)					(plugin_feature_t){ FEATURE_REGISTER, FEATURE_##type, .arg.reg.f = _f }
#define __PLUGIN_FEATURE_REGISTER_BUILDER(type, _f, _final)	(plugin_feature_t){ FEATURE_REGISTER, FEATURE_##type, .arg.reg = {.f = _f, .final = _final, }}
#define _PLUGIN_FEATURE_REGISTER_CRYPTER(type, f)			__PLUGIN_FEATURE_REGISTER(type, f)
#define _PLUGIN_FEATURE_REGISTER_AEAD(type, f)				__PLUGIN_FEATURE_REGISTER(type, f)
#define _PLUGIN_FEATURE_REGISTER_SIGNER(type, f)			__PLUGIN_FEATURE_REGISTER(type, f)
#define _PLUGIN_FEATURE_REGISTER_HASHER(type, f)			__PLUGIN_FEATURE_REGISTER(type, f)
#define _PLUGIN_FEATURE_REGISTER_PRF(type, f)				__PLUGIN_FEATURE_REGISTER(type, f)
#define _PLUGIN_FEATURE_REGISTER_XOF(type, f)				__PLUGIN_FEATURE_REGISTER(type, f)
#define _PLUGIN_FEATURE_REGISTER_DH(type, f)				__PLUGIN_FEATURE_REGISTER(type, f)
#define _PLUGIN_FEATURE_REGISTER_RNG(type, f)				__PLUGIN_FEATURE_REGISTER(type, f)
#define _PLUGIN_FEATURE_REGISTER_NONCE_GEN(type, f)			__PLUGIN_FEATURE_REGISTER(type, f)
#define _PLUGIN_FEATURE_REGISTER_PRIVKEY(type, f, final)	__PLUGIN_FEATURE_REGISTER_BUILDER(type, f, final)
#define _PLUGIN_FEATURE_REGISTER_PRIVKEY_GEN(type, f, final)__PLUGIN_FEATURE_REGISTER_BUILDER(type, f, final)
#define _PLUGIN_FEATURE_REGISTER_PUBKEY(type, f, final)		__PLUGIN_FEATURE_REGISTER_BUILDER(type, f, final)
#define _PLUGIN_FEATURE_REGISTER_CERT_DECODE(type, f, final)__PLUGIN_FEATURE_REGISTER_BUILDER(type, f, final)
#define _PLUGIN_FEATURE_REGISTER_CERT_ENCODE(type, f, final)__PLUGIN_FEATURE_REGISTER_BUILDER(type, f, final)
#define _PLUGIN_FEATURE_REGISTER_CONTAINER_DECODE(type, f, final)__PLUGIN_FEATURE_REGISTER_BUILDER(type, f, final)
#define _PLUGIN_FEATURE_REGISTER_CONTAINER_ENCODE(type, f, final)__PLUGIN_FEATURE_REGISTER_BUILDER(type, f, final)
#define _PLUGIN_FEATURE_REGISTER_DATABASE(type, f)			__PLUGIN_FEATURE_REGISTER(type, f)
#define _PLUGIN_FEATURE_REGISTER_FETCHER(type, f)			__PLUGIN_FEATURE_REGISTER(type, f)
#define _PLUGIN_FEATURE_REGISTER_RESOLVER(type, f)			__PLUGIN_FEATURE_REGISTER(type, f)

#define _PLUGIN_FEATURE_CALLBACK(_cb, _data) (plugin_feature_t){ FEATURE_CALLBACK, FEATURE_NONE, .arg.cb = { .f = _cb, .data = _data } }

/**
 * Names for plugin_feature_t types.
 */
extern enum_name_t *plugin_feature_names;

/**
 * Add a set of plugin features to the given array, which must have enough space
 * to store the added features.
 *
 * @param features		the array of plugin features to extend
 * @param to_add		the features to add
 * @param count			number of features to add
 * @param pos			current position in the features array, gets advanced
 */
static inline void plugin_features_add(plugin_feature_t *features,
									   plugin_feature_t *to_add,
									   int count, int *pos)
{
	int i;

	for (i = 0; i < count; i++)
	{
		features[(*pos)++] = to_add[i];
	}
}

/**
 * Calculates a hash value for the given feature.
 *
 * Since this is intended to be used with the plugin_features_matches function
 * the hash is not really unique for all types of features (e.g. RNGs are all
 * mapped to the same value because they are loosely matched by said function).
 *
 * @param feature	feature to hash
 * @return			hash value of the feature
 */
uint32_t plugin_feature_hash(plugin_feature_t *feature);

/**
 * Check if feature a matches to feature b.
 *
 * This is no check for equality.  For instance, for FEATURE_RNG a matches b if
 * a's strength is at least the strength of b.  Or for FEATURE_SQL if a is
 * DB_ANY it will match b if it is of the same type.
 *
 * @param a			feature to check
 * @param b			feature to match against
 * @return			TRUE if a matches b
 */
bool plugin_feature_matches(plugin_feature_t *a, plugin_feature_t *b);

/**
 * Check if feature a equals feature b.
 *
 * @param a			feature
 * @param b			feature to compare
 * @return			TRUE if a equals b
 */
bool plugin_feature_equals(plugin_feature_t *a, plugin_feature_t *b);

/**
 * Get a string describing feature.
 *
 * @param feature	feature to describe
 * @return			allocated string describing feature
 */
char* plugin_feature_get_string(plugin_feature_t *feature);

/**
 * Load a plugin feature using a REGISTER/CALLBACK feature entry.
 *
 * @param plugin	plugin providing feature
 * @param feature	feature to load
 * @param reg		REGISTER/CALLBACK feature entry to use for registration
 */
bool plugin_feature_load(plugin_t *plugin, plugin_feature_t *feature,
						 plugin_feature_t *reg);

/**
 * Unload a plugin feature using a REGISTER/CALLBACK feature entry.
 *
 * @param plugin	plugin providing feature
 * @param feature	feature to unload
 * @param reg		REGISTER/CALLBACK feature entry to use for deregistration
 */
bool plugin_feature_unload(plugin_t *plugin, plugin_feature_t *feature,
						   plugin_feature_t *reg);

#endif /** PLUGIN_FEATURE_H_ @}*/
