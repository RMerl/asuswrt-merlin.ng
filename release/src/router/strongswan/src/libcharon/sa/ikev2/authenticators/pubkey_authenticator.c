/*
 * Copyright (C) 2008-2018 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "pubkey_authenticator.h"

#include <daemon.h>
#include <encoding/payloads/auth_payload.h>
#include <sa/ikev2/keymat_v2.h>
#include <asn1/asn1.h>
#include <asn1/oid.h>
#include <collections/array.h>
#include <credentials/certificates/x509.h>

typedef struct private_pubkey_authenticator_t private_pubkey_authenticator_t;

/**
 * Private data of an pubkey_authenticator_t object.
 */
struct private_pubkey_authenticator_t {

	/**
	 * Public authenticator_t interface.
	 */
	pubkey_authenticator_t public;

	/**
	 * Assigned IKE_SA
	 */
	ike_sa_t *ike_sa;

	/**
	 * nonce to include in AUTH calculation
	 */
	chunk_t nonce;

	/**
	 * IKE_SA_INIT message data to include in AUTH calculation
	 */
	chunk_t ike_sa_init;

	/**
	 * Reserved bytes of ID payload
	 */
	char reserved[3];

	/**
	 * PPK to use
	 */
	chunk_t ppk;

	/**
	 * Add a NO_PPK_AUTH notify
	 */
	bool no_ppk_auth;
};

/**
 * Parse authentication data used for Signature Authentication as per RFC 7427
 */
static bool parse_signature_auth_data(chunk_t *auth_data, key_type_t *key_type,
									  signature_params_t *params)
{
	uint8_t len;

	if (!auth_data->len)
	{
		return FALSE;
	}
	len = auth_data->ptr[0];
	*auth_data = chunk_skip(*auth_data, 1);
	if (!signature_params_parse(*auth_data, 1, params))
	{
		return FALSE;
	}
	*key_type = key_type_from_signature_scheme(params->scheme);
	*auth_data = chunk_skip(*auth_data, len);
	return TRUE;
}

/**
 * Build authentication data used for Signature Authentication as per RFC 7427
 */
static bool build_signature_auth_data(chunk_t *auth_data,
									  signature_params_t *params)
{
	chunk_t data;
	uint8_t len;

	if (!signature_params_build(params, &data))
	{
		chunk_free(auth_data);
		return FALSE;
	}
	len = data.len;
	*auth_data = chunk_cat("cmm", chunk_from_thing(len), data, *auth_data);
	return TRUE;
}

/**
 * Check if the given scheme is supported by the key and, if so, add it to the
 * first array (we add the scheme supported by the key in case the parameters
 * are different)
 */
static void add_scheme_if_supported(array_t *selected, array_t *supported,
									signature_params_t *config)
{
	signature_params_t *sup;
	int i;

	if (!supported)
	{
		array_insert(selected, ARRAY_TAIL, signature_params_clone(config));
		return;
	}

	for (i = 0; i < array_count(supported); i++)
	{
		array_get(supported, i, &sup);
		if (signature_params_comply(sup, config))
		{
			array_insert(selected, ARRAY_TAIL, signature_params_clone(sup));
			return;
		}
	}
}

CALLBACK(destroy_scheme, void,
	signature_params_t *params, int idx, void *user)
{
	signature_params_destroy(params);
}

/**
 * Selects possible signature schemes based on our configuration, the other
 * peer's capabilities and the private key
 */
static array_t *select_signature_schemes(keymat_v2_t *keymat,
									auth_cfg_t *auth, private_key_t *private)
{
	enumerator_t *enumerator;
	signature_scheme_t scheme;
	signature_params_t *config;
	auth_rule_t rule;
	key_type_t key_type;
	bool have_config = FALSE;
	array_t *supported = NULL, *selected;

	selected = array_create(0, 0);
	key_type = private->get_type(private);

	if (private->supported_signature_schemes)
	{
		enumerator = private->supported_signature_schemes(private);
		while (enumerator->enumerate(enumerator, &config))
		{
			if (keymat->hash_algorithm_supported(keymat,
								hasher_from_signature_scheme(config->scheme,
															 config->params)))
			{
				array_insert_create(&supported, ARRAY_TAIL,
									signature_params_clone(config));
			}
		}
		enumerator->destroy(enumerator);

		if (!supported)
		{
			return selected;
		}
	}

	enumerator = auth->create_enumerator(auth);
	while (enumerator->enumerate(enumerator, &rule, &config))
	{
		if (rule != AUTH_RULE_IKE_SIGNATURE_SCHEME)
		{
			continue;
		}
		if (key_type == key_type_from_signature_scheme(config->scheme) &&
			keymat->hash_algorithm_supported(keymat,
								hasher_from_signature_scheme(config->scheme,
															 config->params)))
		{
			add_scheme_if_supported(selected, supported, config);
		}
		have_config = TRUE;
	}
	enumerator->destroy(enumerator);

	if (have_config)
	{
		array_destroy_function(supported, destroy_scheme, NULL);
	}
	else
	{
		/* if we have no config, return either whatever schemes the key (and
		 * peer) support or.. */
		if (supported)
		{
			array_destroy(selected);
			return supported;
		}

		/* ...find schemes appropriate for the key and supported by the peer */
		enumerator = signature_schemes_for_key(key_type,
											   private->get_keysize(private));
		while (enumerator->enumerate(enumerator, &config))
		{
			if (config->scheme == SIGN_RSA_EMSA_PSS &&
				!lib->settings->get_bool(lib->settings, "%s.rsa_pss", FALSE,
										 lib->ns))
			{
				continue;
			}
			if (keymat->hash_algorithm_supported(keymat,
								hasher_from_signature_scheme(config->scheme,
															 config->params)))
			{
				array_insert(selected, ARRAY_TAIL,
							 signature_params_clone(config));
			}
		}
		enumerator->destroy(enumerator);

		/* for RSA we tried at least SHA-512, also try other schemes */
		if (key_type == KEY_RSA)
		{
			signature_scheme_t schemes[] = {
				SIGN_RSA_EMSA_PKCS1_SHA2_384,
				SIGN_RSA_EMSA_PKCS1_SHA2_256,
			};
			bool found;
			int i, j;

			for (i = 0; i < countof(schemes); i++)
			{
				scheme = schemes[i];
				found = FALSE;
				for (j = 0; j < array_count(selected); j++)
				{
					array_get(selected, j, &config);
					if (scheme == config->scheme)
					{
						found = TRUE;
						break;
					}
				}
				if (!found && keymat->hash_algorithm_supported(keymat,
										hasher_from_signature_scheme(scheme,
																	 NULL)))
				{
					INIT(config,
						.scheme = scheme,
					)
					array_insert(selected, ARRAY_TAIL, config);
				}
			}
		}
	}
	return selected;
}

/**
 * Adds the given auth data to the message, either in an AUTH payload or
 * a NO_PPK_AUTH notify.
 *
 * The data is freed.
 */
static void add_auth_to_message(message_t *message, auth_method_t method,
								chunk_t data, bool notify)
{
	auth_payload_t *auth_payload;

	if (notify)
	{
		message->add_notify(message, FALSE, NO_PPK_AUTH, data);
	}
	else
	{
		auth_payload = auth_payload_create();
		auth_payload->set_auth_method(auth_payload, method);
		auth_payload->set_data(auth_payload, data);
		message->add_payload(message, (payload_t*)auth_payload);
	}
	chunk_free(&data);
}

/**
 * Create a signature using RFC 7427 signature authentication
 */
static status_t sign_signature_auth(private_pubkey_authenticator_t *this,
									auth_cfg_t *auth, private_key_t *private,
									identification_t *id, message_t *message)
{
	enumerator_t *enumerator;
	keymat_v2_t *keymat;
	signature_params_t *params = NULL;
	array_t *schemes;
	chunk_t octets = chunk_empty, auth_data;
	status_t status = FAILED;

	keymat = (keymat_v2_t*)this->ike_sa->get_keymat(this->ike_sa);
	schemes = select_signature_schemes(keymat, auth, private);
	if (!array_count(schemes))
	{
		DBG1(DBG_IKE, "no common hash algorithm found to create signature "
			 "with %N key", key_type_names, private->get_type(private));
		array_destroy(schemes);
		return FAILED;
	}

	if (keymat->get_auth_octets(keymat, FALSE, this->ike_sa_init, this->nonce,
							this->ppk, id, this->reserved, &octets, schemes))
	{
		enumerator = array_create_enumerator(schemes);
		while (enumerator->enumerate(enumerator, &params))
		{
			if (!private->sign(private, params->scheme, params->params, octets,
							   &auth_data) ||
				!build_signature_auth_data(&auth_data, params))
			{
				DBG2(DBG_IKE, "unable to create %N signature for %N key",
					 signature_scheme_names, params->scheme, key_type_names,
					 private->get_type(private));
				continue;
			}
			add_auth_to_message(message, AUTH_DS, auth_data, FALSE);
			status = SUCCESS;

			if (this->no_ppk_auth)
			{
				chunk_free(&octets);

				if (keymat->get_auth_octets(keymat, FALSE, this->ike_sa_init,
											this->nonce, chunk_empty, id,
											this->reserved, &octets, schemes) &&
					private->sign(private, params->scheme, params->params,
								  octets, &auth_data) &&
					build_signature_auth_data(&auth_data, params))
				{
					add_auth_to_message(message, AUTH_DS, auth_data, TRUE);
				}
				else
				{
					DBG2(DBG_IKE, "unable to create %N signature for %N key "
						 "without PPK", signature_scheme_names, params->scheme,
						 key_type_names, private->get_type(private));
					status = FAILED;
				}
			}
			break;
		}
		enumerator->destroy(enumerator);
	}
	if (params)
	{
		if (params->scheme == SIGN_RSA_EMSA_PSS)
		{
			rsa_pss_params_t *pss = params->params;
			DBG1(DBG_IKE, "authentication of '%Y' (myself) with %N_%N_SALT_%zd "
				 "%s", id, signature_scheme_names, params->scheme,
				 hash_algorithm_short_names_upper, pss->hash, pss->salt_len,
				 status == SUCCESS ? "successful" : "failed");
		}
		else
		{
			DBG1(DBG_IKE, "authentication of '%Y' (myself) with %N %s", id,
				 signature_scheme_names, params->scheme,
				 status == SUCCESS ? "successful" : "failed");
		}
	}
	else
	{
		DBG1(DBG_IKE, "authentication of '%Y' (myself) failed", id);
	}
	array_destroy_function(schemes, destroy_scheme, NULL);
	chunk_free(&octets);
	return status;
}

/**
 * Get the auth octets and the signature scheme (in case it is changed by the
 * keymat).
 */
static bool get_auth_octets_scheme(private_pubkey_authenticator_t *this,
								bool verify, identification_t *id, chunk_t ppk,
								chunk_t *octets, signature_params_t **scheme)
{
	keymat_v2_t *keymat;
	array_t *schemes;
	bool success = FALSE;

	schemes = array_create(0, 0);
	array_insert(schemes, ARRAY_TAIL, *scheme);

	keymat = (keymat_v2_t*)this->ike_sa->get_keymat(this->ike_sa);
	if (keymat->get_auth_octets(keymat, verify, this->ike_sa_init, this->nonce,
								ppk, id, this->reserved, octets,
								schemes) &&
		array_remove(schemes, 0, scheme))
	{
		success = TRUE;
	}
	else
	{
		*scheme = NULL;
	}
	array_destroy_function(schemes, destroy_scheme, NULL);
	return success;
}

/**
 * Create a classic IKEv2 signature
 */
static status_t sign_classic(private_pubkey_authenticator_t *this,
							 auth_cfg_t *auth, private_key_t *private,
							 identification_t *id, message_t *message)
{
	signature_scheme_t scheme;
	signature_params_t *params;
	auth_method_t auth_method = AUTH_NONE;
	chunk_t octets = chunk_empty, auth_data;
	status_t status = FAILED;

	switch (private->get_type(private))
	{
		case KEY_RSA:
			scheme = SIGN_RSA_EMSA_PKCS1_SHA1;
			auth_method = AUTH_RSA;
			break;
		case KEY_ECDSA:
			/* deduct the signature scheme from the keysize */
			switch (private->get_keysize(private))
			{
				case 256:
					scheme = SIGN_ECDSA_256;
					auth_method = AUTH_ECDSA_256;
					break;
				case 384:
					scheme = SIGN_ECDSA_384;
					auth_method = AUTH_ECDSA_384;
					break;
				case 521:
					scheme = SIGN_ECDSA_521;
					auth_method = AUTH_ECDSA_521;
					break;
				default:
					DBG1(DBG_IKE, "%d bit ECDSA private key size not supported",
						 private->get_keysize(private));
					return FAILED;
			}
			break;
		default:
			DBG1(DBG_IKE, "private key of type %N not supported",
				 key_type_names, private->get_type(private));
			return FAILED;
	}

	INIT(params,
		.scheme = scheme,
	);
	if (get_auth_octets_scheme(this, FALSE, id, this->ppk, &octets, &params) &&
		private->sign(private, params->scheme, NULL, octets, &auth_data))
	{
		add_auth_to_message(message, auth_method, auth_data, FALSE);
		status = SUCCESS;

		if (this->no_ppk_auth)
		{
			chunk_free(&octets);
			if (get_auth_octets_scheme(this, FALSE, id, chunk_empty, &octets,
									   &params) &&
				private->sign(private, params->scheme, NULL, octets,
							  &auth_data))
			{
				add_auth_to_message(message, auth_method, auth_data, TRUE);
			}
			else
			{
				status = FAILED;
			}
		}
	}
	if (params)
	{
		signature_params_destroy(params);
	}
	DBG1(DBG_IKE, "authentication of '%Y' (myself) with %N %s", id,
		 auth_method_names, auth_method,
		 status == SUCCESS ? "successful" : "failed");
	chunk_free(&octets);
	return status;
}

METHOD(authenticator_t, build, status_t,
	private_pubkey_authenticator_t *this, message_t *message)
{
	private_key_t *private;
	identification_t *id;
	auth_cfg_t *auth;
	status_t status;

	id = this->ike_sa->get_my_id(this->ike_sa);
	auth = this->ike_sa->get_auth_cfg(this->ike_sa, TRUE);
	private = lib->credmgr->get_private(lib->credmgr, KEY_ANY, id, auth);
	if (!private)
	{
		DBG1(DBG_IKE, "no private key found for '%Y'", id);
		return NOT_FOUND;
	}

	if (this->ike_sa->supports_extension(this->ike_sa, EXT_SIGNATURE_AUTH))
	{
		status = sign_signature_auth(this, auth, private, id, message);
	}
	else
	{
		status = sign_classic(this, auth, private, id, message);
	}
	private->destroy(private);
	return status;
}

/**
 * Check if the end-entity certificate, if any, is compliant with RFC 4945
 */
static bool is_compliant_cert(auth_cfg_t *auth)
{
	certificate_t *cert;
	x509_t *x509;

	cert = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
	if (!cert || cert->get_type(cert) != CERT_X509)
	{
		return TRUE;
	}
	x509 = (x509_t*)cert;
	if (x509->get_flags(x509) & X509_IKE_COMPLIANT)
	{
		return TRUE;
	}
	DBG1(DBG_IKE, "rejecting certificate without digitalSignature or "
		 "nonRepudiation keyUsage flags");
	return FALSE;
}

METHOD(authenticator_t, process, status_t,
	private_pubkey_authenticator_t *this, message_t *message)
{
	public_key_t *public;
	auth_method_t auth_method;
	auth_payload_t *auth_payload;
	notify_payload_t *notify;
	chunk_t auth_data, octets;
	identification_t *id;
	auth_cfg_t *auth, *current_auth;
	enumerator_t *enumerator;
	key_type_t key_type = KEY_ECDSA;
	signature_params_t *params;
	status_t status = NOT_FOUND;
	const char *reason = "unsupported";
	bool online;

	auth_payload = (auth_payload_t*)message->get_payload(message, PLV2_AUTH);
	if (!auth_payload)
	{
		return FAILED;
	}
	auth_method = auth_payload->get_auth_method(auth_payload);
	auth_data = auth_payload->get_data(auth_payload);

	if (this->ike_sa->supports_extension(this->ike_sa, EXT_PPK) &&
		!this->ppk.ptr)
	{	/* look for a NO_PPK_AUTH notify if we have no PPK */
		notify = message->get_notify(message, NO_PPK_AUTH);
		if (notify)
		{
			DBG1(DBG_IKE, "no PPK available, using NO_PPK_AUTH notify");
			auth_data = notify->get_notification_data(notify);
		}
	}

	INIT(params);
	switch (auth_method)
	{
		case AUTH_RSA:
			key_type = KEY_RSA;
			params->scheme = SIGN_RSA_EMSA_PKCS1_SHA1;
			break;
		case AUTH_ECDSA_256:
			params->scheme = SIGN_ECDSA_256;
			break;
		case AUTH_ECDSA_384:
			params->scheme = SIGN_ECDSA_384;
			break;
		case AUTH_ECDSA_521:
			params->scheme = SIGN_ECDSA_521;
			break;
		case AUTH_DS:
			if (parse_signature_auth_data(&auth_data, &key_type, params))
			{
				break;
			}
			reason = "payload invalid";
			/* fall-through */
		default:
			DBG1(DBG_IKE, "%N authentication %s", auth_method_names,
				 auth_method, reason);
			signature_params_destroy(params);
			return INVALID_ARG;
	}
	id = this->ike_sa->get_other_id(this->ike_sa);
	if (!get_auth_octets_scheme(this, TRUE, id, this->ppk, &octets, &params))
	{
		return FAILED;
	}
	auth = this->ike_sa->get_auth_cfg(this->ike_sa, FALSE);
	online = !this->ike_sa->has_condition(this->ike_sa,
										  COND_ONLINE_VALIDATION_SUSPENDED);
	enumerator = lib->credmgr->create_public_enumerator(lib->credmgr,
													key_type, id, auth, online);
	while (enumerator->enumerate(enumerator, &public, &current_auth))
	{
		if (public->verify(public, params->scheme, params->params, octets,
						   auth_data) &&
			is_compliant_cert(current_auth))
		{
			if (auth_method != AUTH_DS)
			{
				DBG1(DBG_IKE, "authentication of '%Y' with %N successful", id,
					 auth_method_names, auth_method);
			}
			else if (params->scheme == SIGN_RSA_EMSA_PSS)
			{
				rsa_pss_params_t *pss = params->params;
				DBG1(DBG_IKE, "authentication of '%Y' with %N_%N_SALT_%zd "
					 "successful", id, signature_scheme_names, params->scheme,
					 hash_algorithm_short_names_upper, pss->hash, pss->salt_len);
			}
			else
			{
				DBG1(DBG_IKE, "authentication of '%Y' with %N successful", id,
					 signature_scheme_names, params->scheme);
			}
			status = SUCCESS;
			auth->merge(auth, current_auth, FALSE);
			auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
			auth->add(auth, AUTH_RULE_IKE_SIGNATURE_SCHEME,
					  signature_params_clone(params));
			if (!online)
			{
				auth->add(auth, AUTH_RULE_CERT_VALIDATION_SUSPENDED, TRUE);
			}
			break;
		}
		else
		{
			status = FAILED;
			DBG1(DBG_IKE, "signature validation failed, looking for another key");
		}
	}
	enumerator->destroy(enumerator);
	chunk_free(&octets);
	signature_params_destroy(params);
	if (status == NOT_FOUND)
	{
		DBG1(DBG_IKE, "no trusted %N public key found for '%Y'",
			 key_type_names, key_type, id);
	}
	return status;
}

METHOD(authenticator_t, use_ppk, void,
	private_pubkey_authenticator_t *this, chunk_t ppk, bool no_ppk_auth)
{
	this->ppk = ppk;
	this->no_ppk_auth = no_ppk_auth;
}

METHOD(authenticator_t, destroy, void,
	private_pubkey_authenticator_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
pubkey_authenticator_t *pubkey_authenticator_create_builder(ike_sa_t *ike_sa,
									chunk_t received_nonce, chunk_t sent_init,
									char reserved[3])
{
	private_pubkey_authenticator_t *this;

	INIT(this,
		.public = {
			.authenticator = {
				.build = _build,
				.process = (void*)return_failed,
				.use_ppk = _use_ppk,
				.is_mutual = (void*)return_false,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.ike_sa_init = sent_init,
		.nonce = received_nonce,
	);
	memcpy(this->reserved, reserved, sizeof(this->reserved));

	return &this->public;
}

/*
 * Described in header.
 */
pubkey_authenticator_t *pubkey_authenticator_create_verifier(ike_sa_t *ike_sa,
									chunk_t sent_nonce, chunk_t received_init,
									char reserved[3])
{
	private_pubkey_authenticator_t *this;

	INIT(this,
		.public = {
			.authenticator = {
				.build = (void*)return_failed,
				.process = _process,
				.use_ppk = _use_ppk,
				.is_mutual = (void*)return_false,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.ike_sa_init = received_init,
		.nonce = sent_nonce,
	);
	memcpy(this->reserved, reserved, sizeof(this->reserved));

	return &this->public;
}
