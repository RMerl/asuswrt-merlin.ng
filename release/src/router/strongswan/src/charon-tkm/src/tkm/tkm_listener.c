/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include <stdarg.h>
#include <inttypes.h>

#include <daemon.h>
#include <collections/array.h>
#include <collections/hashtable.h>
#include <encoding/payloads/auth_payload.h>
#include <utils/chunk.h>
#include <tkm/types.h>
#include <tkm/constants.h>
#include <tkm/client.h>

#include "tkm.h"
#include "tkm_listener.h"
#include "tkm_keymat.h"
#include "tkm_utils.h"

typedef struct private_tkm_listener_t private_tkm_listener_t;

static hashtable_t *ca_map = NULL;

/**
 * Private data of a tkm_listener_t object.
 */
struct private_tkm_listener_t {

	/**
	 * Public tkm_listener_t interface.
	 */
	tkm_listener_t public;

};

/**
 * Return id of remote identity.
 *
 * TODO: Replace this with the lookup for the remote identity id.
 *
 * Currently the reqid of the first child SA in peer config of IKE SA is
 * returned. Might choose wrong reqid if IKE SA has multiple child configs
 * with different reqids.
 *
 * @param peer_cfg	Remote peer config
 * @return			remote identity id if found, 0 otherwise
 */
static ri_id_type get_remote_identity_id(peer_cfg_t *peer)
{
	ri_id_type remote_id = 0;
	child_cfg_t *child;
	enumerator_t* children;

	children = peer->create_child_cfg_enumerator(peer);

	/* pick the reqid of the first child, no need to enumerate all children. */
	children->enumerate(children, &child);
	remote_id = child->get_reqid(child);
	children->destroy(children);

	return remote_id;
}

/**
 * Build a TKM certificate chain context with given cc id.
 *
 * @param ike_sa	IKE SA containing auth config to build certificate chain from
 * @param cc_id		Certificate chain ID
 * @return			TRUE if certificate chain was built successfully,
 *					FALSE otherwise
 */
static bool build_cert_chain(const ike_sa_t * const ike_sa, cc_id_type cc_id)
{
	auth_cfg_t *auth;
	certificate_t *cert;
	enumerator_t *rounds;

	DBG1(DBG_IKE, "building certificate chain context %llu for IKE SA %s",
		 cc_id, ike_sa->get_name((ike_sa_t *)ike_sa));

	rounds = ike_sa->create_auth_cfg_enumerator((ike_sa_t *)ike_sa, FALSE);
	while (rounds->enumerate(rounds, &auth))
	{
		cert = auth->get(auth, AUTH_RULE_CA_CERT);
		if (cert)
		{
			auth_rule_t rule;
			enumerator_t *enumerator;
			ca_id_type ca_id;
			public_key_t *pubkey;
			certificate_type ca_cert;
			chunk_t enc_ca_cert, fp;
			array_t *im_certs = NULL;
			uint64_t *raw_id;

			pubkey = cert->get_public_key(cert);
			if (!pubkey)
			{
				DBG1(DBG_IKE, "unable to get CA certificate pubkey");
				rounds->destroy(rounds);
				return FALSE;
			}
			if (!pubkey->get_fingerprint(pubkey, KEYID_PUBKEY_SHA1, &fp))
			{
				DBG1(DBG_IKE, "unable to extract CA certificate fingerprint");
				rounds->destroy(rounds);
				pubkey->destroy(pubkey);
				return FALSE;
			}
			pubkey->destroy(pubkey);

			raw_id = ca_map->get(ca_map, &fp);
			if (!raw_id || *raw_id == 0)
			{
				DBG1(DBG_IKE, "error mapping CA certificate (fp: %#B) to "
					 "ID", &fp);
				rounds->destroy(rounds);
				return FALSE;
			}
			ca_id = *raw_id;

			if (!cert->get_encoding(cert, CERT_ASN1_DER, &enc_ca_cert))
			{
				DBG1(DBG_IKE, "unable to extract encoded CA certificate");
				rounds->destroy(rounds);
				return FALSE;
			}

			chunk_to_sequence(&enc_ca_cert, &ca_cert,
							  sizeof(certificate_type));
			chunk_free(&enc_ca_cert);

			if (ike_cc_check_ca(cc_id, ca_id, ca_cert) != TKM_OK)
			{
				DBG1(DBG_IKE, "CA certificate (fp: %#B, cc_id: %llu) does not"
					 " match trusted CA (ca_id: %llu)", &fp, cc_id, ca_id);
				rounds->destroy(rounds);
				return FALSE;
			}

			/* process intermediate CA certificates in reverse order */
			enumerator = auth->create_enumerator(auth);
			while (enumerator->enumerate(enumerator, &rule, &cert))
			{
				if (rule == AUTH_RULE_IM_CERT)
				{
					array_insert_create(&im_certs, ARRAY_TAIL, cert);
				}
			}
			enumerator->destroy(enumerator);

			while (array_remove(im_certs, ARRAY_TAIL, &cert))
			{
				chunk_t enc_im_cert;
				certificate_type im_cert;

				if (!cert->get_encoding(cert, CERT_ASN1_DER, &enc_im_cert))
				{
					DBG1(DBG_IKE, "unable to extract encoded intermediate CA"
						 " certificate");
					rounds->destroy(rounds);
					array_destroy(im_certs);
					return FALSE;
				}

				chunk_to_sequence(&enc_im_cert, &im_cert,
								  sizeof(certificate_type));
				chunk_free(&enc_im_cert);
				if (ike_cc_add_certificate(cc_id, 1, im_cert) != TKM_OK)
				{
					DBG1(DBG_IKE, "error adding intermediate certificate to"
						 " cert chain (cc_id: %llu)", cc_id);
					rounds->destroy(rounds);
					array_destroy(im_certs);
					return FALSE;
				}
			}
			array_destroy(im_certs);

			/* finally add user certificate and check chain */
			cert = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
			if (cert)
			{
				chunk_t enc_user_cert;
				ri_id_type ri_id;
				certificate_type user_cert;

				/* set user certificate */
				if (!cert->get_encoding(cert, CERT_ASN1_DER, &enc_user_cert))
				{
					DBG1(DBG_IKE, "unable to extract encoded user certificate");
					rounds->destroy(rounds);
					return FALSE;
				}

				chunk_to_sequence(&enc_user_cert, &user_cert, sizeof(certificate_type));
				chunk_free(&enc_user_cert);
				if (ike_cc_add_certificate(cc_id, 1, user_cert) != TKM_OK)
				{
					DBG1(DBG_IKE, "error adding user certificate to cert chain"
						 " (cc_id: %llu)", cc_id);
					rounds->destroy(rounds);
					return FALSE;
				}

				ri_id = get_remote_identity_id(ike_sa->get_peer_cfg((ike_sa_t *)ike_sa));
				if (ike_cc_check_chain(cc_id, ri_id) != TKM_OK)
				{
					DBG1(DBG_IKE, "error checking cert chain (cc_id: %llu)", cc_id);
					rounds->destroy(rounds);
					return FALSE;
				}

				rounds->destroy(rounds);
				return TRUE;
			}
			else
			{
				DBG1(DBG_IKE, "no subject certificate for remote peer");
			}
		}
		else
		{
			DBG1(DBG_IKE, "no CA certificate");
		}
	}

	rounds->destroy(rounds);
	return FALSE;
}

METHOD(listener_t, alert, bool,
	private_tkm_listener_t *this, ike_sa_t *ike_sa,
	alert_t alert, va_list args)
{
	if (alert == ALERT_KEEP_ON_CHILD_SA_FAILURE)
	{
		tkm_keymat_t *keymat;
		isa_id_type isa_id;
		int is_first;

		is_first = va_arg(args, int);
		if (!is_first)
		{
			return TRUE;
		}

		keymat = (tkm_keymat_t*)ike_sa->get_keymat(ike_sa);
		isa_id = keymat->get_isa_id(keymat);

		DBG1(DBG_IKE, "TKM alert listener called for ISA context %llu", isa_id);
		if (ike_isa_skip_create_first(isa_id) != TKM_OK)
		{
			DBG1(DBG_IKE, "Skip of first child SA creation failed for ISA "
				 "context %llu", isa_id);
		}
	}

	return TRUE;
}

METHOD(listener_t, authorize, bool,
	private_tkm_listener_t *this, ike_sa_t *ike_sa,
	bool final, bool *success)
{
	tkm_keymat_t *keymat;
	isa_id_type isa_id;
	cc_id_type cc_id;
	chunk_t *auth, *other_init_msg;
	signature_type signature;
	init_message_type init_msg;

	if (!final)
	{
		return TRUE;
	}

	*success = FALSE;

	keymat = (tkm_keymat_t*)ike_sa->get_keymat(ike_sa);
	isa_id = keymat->get_isa_id(keymat);
	DBG1(DBG_IKE, "TKM authorize listener called for ISA context %llu", isa_id);

	cc_id = tkm->idmgr->acquire_id(tkm->idmgr, TKM_CTX_CC);
	if (!cc_id)
	{
		DBG1(DBG_IKE, "unable to acquire CC context id");
		return TRUE;
	}
	if (!build_cert_chain(ike_sa, cc_id))
	{
		DBG1(DBG_IKE, "unable to build certificate chain");
		goto cc_reset;
	}

	auth = keymat->get_auth_payload(keymat);
	if (!auth->ptr)
	{
		DBG1(DBG_IKE, "no AUTHENTICATION data available");
		goto cc_reset;
	}

	other_init_msg = keymat->get_peer_init_msg(keymat);
	if (!other_init_msg->ptr)
	{
		DBG1(DBG_IKE, "no peer init message available");
		goto cc_reset;
	}

	chunk_to_sequence(auth, &signature, sizeof(signature_type));
	chunk_to_sequence(other_init_msg, &init_msg, sizeof(init_message_type));

	if (ike_isa_auth(isa_id, cc_id, init_msg, signature) != TKM_OK)
	{
		DBG1(DBG_IKE, "TKM based authentication failed"
			 " for ISA context %llu", isa_id);
		goto cc_reset;
	}
	else
	{
		DBG1(DBG_IKE, "TKM based authentication successful"
			 " for ISA context %llu", isa_id);
		*success = TRUE;
	}

cc_reset:
	if (ike_cc_reset(cc_id) != TKM_OK)
	{
		DBG1(DBG_IKE, "unable to reset CC context %llu", cc_id);
	}
	tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_CC, cc_id);
	return TRUE; /* stay registered */
}

METHOD(listener_t, message, bool,
	private_tkm_listener_t *this, ike_sa_t *ike_sa,
	message_t *message, bool incoming, bool plain)
{
	tkm_keymat_t *keymat;
	isa_id_type isa_id;
	auth_payload_t *auth_payload;

	if (!incoming || !plain || message->get_exchange_type(message) != IKE_AUTH)
	{
		return TRUE;
	}

	keymat = (tkm_keymat_t*)ike_sa->get_keymat(ike_sa);
	isa_id = keymat->get_isa_id(keymat);
	DBG1(DBG_IKE, "saving AUTHENTICATION payload for authorize hook"
	     " (ISA context %llu)", isa_id);

	auth_payload = (auth_payload_t*)message->get_payload(message,
														 PLV2_AUTH);
	if (auth_payload)
	{
		chunk_t auth_data;

		auth_data = auth_payload->get_data(auth_payload);
		keymat->set_auth_payload(keymat, &auth_data);
	}
	else
	{
		DBG1(DBG_IKE, "unable to extract AUTHENTICATION payload, authorize will"
			 " fail");
	}

	return TRUE;
}

METHOD(tkm_listener_t, destroy, void,
	private_tkm_listener_t *this)
{
	free(this);
}

/**
 * See header
 */
tkm_listener_t *tkm_listener_create()
{
	private_tkm_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.authorize = _authorize,
				.message = _message,
				.alert = _alert,
			},
			.destroy = _destroy,
		},
	);

	return &this->public;
}

static u_int hash(const chunk_t *key)
{
	return chunk_hash(*key);
}

static bool equals(const chunk_t *key, const chunk_t *other_key)
{
	return chunk_equals(*key, *other_key);
}

static u_int id_hash(const uint64_t *key)
{
	return chunk_hash(chunk_create((u_char*)key, sizeof(uint64_t)));
}

static bool id_equals(const uint64_t *key, const uint64_t *other_key)
{
	return *key == *other_key;
}

/*
 * Described in header.
 */
int register_ca_mapping()
{
	char *section, *tkm_ca_id_str, *key_fp_str;
	chunk_t *key_fp;
	uint64_t *tkm_ca_id;
	hashtable_t *id_map;
	enumerator_t *enumerator;
	bool err = FALSE;

	ca_map = hashtable_create((hashtable_hash_t)hash,
							  (hashtable_equals_t)equals, 8);
	id_map = hashtable_create((hashtable_hash_t)id_hash,
							  (hashtable_equals_t)id_equals, 8);

	enumerator = lib->settings->create_section_enumerator(lib->settings,
														  "%s.ca_mapping",
														  lib->ns);
	while (enumerator->enumerate(enumerator, &section))
	{
		tkm_ca_id_str = lib->settings->get_str(lib->settings,
											   "%s.ca_mapping.%s.id", NULL,
											   lib->ns, section);
		tkm_ca_id = malloc_thing(uint64_t);
		*tkm_ca_id = settings_value_as_uint64(tkm_ca_id_str, 0);

		key_fp_str = lib->settings->get_str(lib->settings,
											"%s.ca_mapping.%s.fingerprint", NULL,
											lib->ns, section);
		if (key_fp_str)
		{
			key_fp = malloc_thing(chunk_t);
			*key_fp = chunk_from_hex(chunk_from_str(key_fp_str), NULL);
		}

		if (!*tkm_ca_id || !key_fp_str || !key_fp->len ||
			id_map->get(id_map, tkm_ca_id) != NULL)
		{
			DBG1(DBG_CFG, "error adding CA ID mapping '%s': ID %s, FP '%s'",
				 section, tkm_ca_id_str, key_fp_str);
			free(tkm_ca_id);
			if (key_fp_str)
			{
				chunk_free(key_fp);
				free(key_fp);
			}
			err = TRUE;
		}
		else
		{
			DBG2(DBG_CFG, "adding CA ID mapping '%s': ID %" PRIu64 ", FP '%#B'",
				 section, *tkm_ca_id, key_fp);
			ca_map->put(ca_map, key_fp, tkm_ca_id);
			/* track CA IDs for uniqueness, set value to not-NULL */
			id_map->put(id_map, tkm_ca_id, id_map);
		}
	}
	enumerator->destroy(enumerator);
	id_map->destroy(id_map);

	return err ? 0 : ca_map->get_count(ca_map);
}

/*
 * Described in header.
 */
void destroy_ca_mapping()
{
	enumerator_t *enumerator;
	chunk_t *key;
	uint64_t *value;

	if (ca_map)
	{
		enumerator = ca_map->create_enumerator(ca_map);
		while (enumerator->enumerate(enumerator, &key, &value))
		{
			chunk_free(key);
			free(key);
			free(value);
		}
		enumerator->destroy(enumerator);
		ca_map->destroy(ca_map);
	}
	ca_map = NULL;
}
