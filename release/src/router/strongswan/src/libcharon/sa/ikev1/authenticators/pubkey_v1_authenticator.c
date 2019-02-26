/*
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

#include "pubkey_v1_authenticator.h"

#include <daemon.h>
#include <sa/ikev1/keymat_v1.h>
#include <encoding/payloads/hash_payload.h>
#include <credentials/certificates/x509.h>

typedef struct private_pubkey_v1_authenticator_t private_pubkey_v1_authenticator_t;

/**
 * Private data of an pubkey_v1_authenticator_t object.
 */
struct private_pubkey_v1_authenticator_t {

	/**
	 * Public authenticator_t interface.
	 */
	pubkey_v1_authenticator_t public;

	/**
	 * Assigned IKE_SA
	 */
	ike_sa_t *ike_sa;

	/**
	 * TRUE if we are initiator
	 */
	bool initiator;

	/**
	 * DH key exchange
	 */
	diffie_hellman_t *dh;

	/**
	 * Others DH public value
	 */
	chunk_t dh_value;

	/**
	 * Encoded SA payload, without fixed header
	 */
	chunk_t sa_payload;

	/**
	 * Encoded ID payload, without fixed header
	 */
	chunk_t id_payload;

	/**
	 * Key type to use
	 */
	key_type_t type;
};

METHOD(authenticator_t, build, status_t,
	private_pubkey_v1_authenticator_t *this, message_t *message)
{
	hash_payload_t *sig_payload;
	chunk_t hash, sig, dh;
	keymat_v1_t *keymat;
	status_t status;
	private_key_t *private;
	identification_t *id;
	auth_cfg_t *auth;
	signature_scheme_t scheme = SIGN_RSA_EMSA_PKCS1_NULL;

	if (this->type == KEY_ECDSA)
	{
		scheme = SIGN_ECDSA_WITH_NULL;
	}

	id = this->ike_sa->get_my_id(this->ike_sa);
	auth = this->ike_sa->get_auth_cfg(this->ike_sa, TRUE);
	private = lib->credmgr->get_private(lib->credmgr, this->type, id, auth);
	if (!private)
	{
		DBG1(DBG_IKE, "no %N private key found for '%Y'",
			 key_type_names, this->type, id);
		return NOT_FOUND;
	}

	if (!this->dh->get_my_public_value(this->dh, &dh))
	{
		private->destroy(private);
		return FAILED;
	}
	keymat = (keymat_v1_t*)this->ike_sa->get_keymat(this->ike_sa);
	if (!keymat->get_hash(keymat, this->initiator, dh, this->dh_value,
					this->ike_sa->get_id(this->ike_sa), this->sa_payload,
					this->id_payload, &hash, &scheme))
	{
		private->destroy(private);
		free(dh.ptr);
		return FAILED;
	}
	free(dh.ptr);

	if (private->sign(private, scheme, NULL, hash, &sig))
	{
		sig_payload = hash_payload_create(PLV1_SIGNATURE);
		sig_payload->set_hash(sig_payload, sig);
		free(sig.ptr);
		message->add_payload(message, &sig_payload->payload_interface);
		status = SUCCESS;
		DBG1(DBG_IKE, "authentication of '%Y' (myself) successful", id);
	}
	else
	{
		DBG1(DBG_IKE, "authentication of '%Y' (myself) failed", id);
		status = FAILED;
	}
	private->destroy(private);
	free(hash.ptr);

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
	private_pubkey_v1_authenticator_t *this, message_t *message)
{
	chunk_t hash, sig, dh;
	keymat_v1_t *keymat;
	public_key_t *public;
	hash_payload_t *sig_payload;
	auth_cfg_t *auth, *current_auth;
	enumerator_t *enumerator;
	status_t status = NOT_FOUND;
	identification_t *id;
	signature_scheme_t scheme = SIGN_RSA_EMSA_PKCS1_NULL;

	if (this->type == KEY_ECDSA)
	{
		scheme = SIGN_ECDSA_WITH_NULL;
	}

	sig_payload = (hash_payload_t*)message->get_payload(message, PLV1_SIGNATURE);
	if (!sig_payload)
	{
		DBG1(DBG_IKE, "SIG payload missing in message");
		return FAILED;
	}

	id = this->ike_sa->get_other_id(this->ike_sa);
	if (!this->dh->get_my_public_value(this->dh, &dh))
	{
		return FAILED;
	}
	keymat = (keymat_v1_t*)this->ike_sa->get_keymat(this->ike_sa);
	if (!keymat->get_hash(keymat, !this->initiator, this->dh_value, dh,
					this->ike_sa->get_id(this->ike_sa), this->sa_payload,
					this->id_payload, &hash, &scheme))
	{
		free(dh.ptr);
		return FAILED;
	}
	free(dh.ptr);

	sig = sig_payload->get_hash(sig_payload);
	auth = this->ike_sa->get_auth_cfg(this->ike_sa, FALSE);
	enumerator = lib->credmgr->create_public_enumerator(lib->credmgr, this->type,
														id, auth, TRUE);
	while (enumerator->enumerate(enumerator, &public, &current_auth))
	{
		if (public->verify(public, scheme, NULL, hash, sig) &&
			is_compliant_cert(current_auth))
		{
			DBG1(DBG_IKE, "authentication of '%Y' with %N successful",
				 id, signature_scheme_names, scheme);
			status = SUCCESS;
			auth->merge(auth, current_auth, FALSE);
			auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
			break;
		}
		else
		{
			DBG1(DBG_IKE, "signature validation failed, looking for another key");
			status = FAILED;
		}
	}
	enumerator->destroy(enumerator);
	free(hash.ptr);
	if (status != SUCCESS)
	{
		DBG1(DBG_IKE, "no trusted %N public key found for '%Y'",
			 key_type_names, this->type, id);
	}
	return status;
}

METHOD(authenticator_t, destroy, void,
	private_pubkey_v1_authenticator_t *this)
{
	chunk_free(&this->id_payload);
	free(this);
}

/*
 * Described in header.
 */
pubkey_v1_authenticator_t *pubkey_v1_authenticator_create(ike_sa_t *ike_sa,
										bool initiator, diffie_hellman_t *dh,
										chunk_t dh_value, chunk_t sa_payload,
										chunk_t id_payload, key_type_t type)
{
	private_pubkey_v1_authenticator_t *this;

	INIT(this,
		.public = {
			.authenticator = {
				.build = _build,
				.process = _process,
				.is_mutual = (void*)return_false,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.initiator = initiator,
		.dh = dh,
		.dh_value = dh_value,
		.sa_payload = sa_payload,
		.id_payload = id_payload,
		.type = type,
	);

	return &this->public;
}
