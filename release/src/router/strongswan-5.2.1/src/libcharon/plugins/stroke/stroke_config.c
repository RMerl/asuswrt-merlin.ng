/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include "stroke_config.h"

#include <hydra.h>
#include <daemon.h>
#include <threading/mutex.h>
#include <utils/lexparser.h>

#include <netdb.h>

typedef struct private_stroke_config_t private_stroke_config_t;

/**
 * private data of stroke_config
 */
struct private_stroke_config_t {

	/**
	 * public functions
	 */
	stroke_config_t public;

	/**
	 * list of peer_cfg_t
	 */
	linked_list_t *list;

	/**
	 * mutex to lock config list
	 */
	mutex_t *mutex;

	/**
	 * ca sections
	 */
	stroke_ca_t *ca;

	/**
	 * credentials
	 */
	stroke_cred_t *cred;

	/**
	 * Virtual IP pool / DNS backend
	 */
	stroke_attribute_t *attributes;
};

METHOD(backend_t, create_peer_cfg_enumerator, enumerator_t*,
	private_stroke_config_t *this, identification_t *me, identification_t *other)
{
	this->mutex->lock(this->mutex);
	return enumerator_create_cleaner(this->list->create_enumerator(this->list),
									 (void*)this->mutex->unlock, this->mutex);
}

/**
 * filter function for ike configs
 */
static bool ike_filter(void *data, peer_cfg_t **in, ike_cfg_t **out)
{
	*out = (*in)->get_ike_cfg(*in);
	return TRUE;
}

METHOD(backend_t, create_ike_cfg_enumerator, enumerator_t*,
	private_stroke_config_t *this, host_t *me, host_t *other)
{
	this->mutex->lock(this->mutex);
	return enumerator_create_filter(this->list->create_enumerator(this->list),
									(void*)ike_filter, this->mutex,
									(void*)this->mutex->unlock);
}

METHOD(backend_t, get_peer_cfg_by_name, peer_cfg_t*,
	private_stroke_config_t *this, char *name)
{
	enumerator_t *e1, *e2;
	peer_cfg_t *current, *found = NULL;
	child_cfg_t *child;

	this->mutex->lock(this->mutex);
	e1 = this->list->create_enumerator(this->list);
	while (e1->enumerate(e1, &current))
	{
		/* compare peer_cfgs name first */
		if (streq(current->get_name(current), name))
		{
			found = current;
			found->get_ref(found);
			break;
		}
		/* compare all child_cfg names otherwise */
		e2 = current->create_child_cfg_enumerator(current);
		while (e2->enumerate(e2, &child))
		{
			if (streq(child->get_name(child), name))
			{
				found = current;
				found->get_ref(found);
				break;
			}
		}
		e2->destroy(e2);
		if (found)
		{
			break;
		}
	}
	e1->destroy(e1);
	this->mutex->unlock(this->mutex);
	return found;
}

/**
 * parse a proposal string, either into ike_cfg or child_cfg
 */
static void add_proposals(private_stroke_config_t *this, char *string,
				ike_cfg_t *ike_cfg, child_cfg_t *child_cfg, protocol_id_t proto)
{
	if (string)
	{
		char *single;
		char *strict;
		proposal_t *proposal;

		strict = string + strlen(string) - 1;
		if (*strict == '!')
		{
			*strict = '\0';
		}
		else
		{
			strict = NULL;
		}
		while ((single = strsep(&string, ",")))
		{
			proposal = proposal_create_from_string(proto, single);
			if (proposal)
			{
				if (ike_cfg)
				{
					ike_cfg->add_proposal(ike_cfg, proposal);
				}
				else
				{
					child_cfg->add_proposal(child_cfg, proposal);
				}
				continue;
			}
			DBG1(DBG_CFG, "skipped invalid proposal string: %s", single);
		}
		if (strict)
		{
			return;
		}
		/* add default porposal to the end if not strict */
	}
	if (ike_cfg)
	{
		ike_cfg->add_proposal(ike_cfg, proposal_create_default(proto));
		ike_cfg->add_proposal(ike_cfg, proposal_create_default_aead(proto));
	}
	else
	{
		child_cfg->add_proposal(child_cfg, proposal_create_default(proto));
		child_cfg->add_proposal(child_cfg, proposal_create_default_aead(proto));
	}
}

/**
 * Build an IKE config from a stroke message
 */
static ike_cfg_t *build_ike_cfg(private_stroke_config_t *this, stroke_msg_t *msg)
{
	enumerator_t *enumerator;
	stroke_end_t tmp_end;
	ike_cfg_t *ike_cfg;
	host_t *host;
	u_int16_t ikeport;
	char me[256], other[256], *token;
	bool swapped = FALSE;;

	enumerator = enumerator_create_token(msg->add_conn.other.address, ",", " ");
	while (enumerator->enumerate(enumerator, &token))
	{
		if (!strchr(token, '/'))
		{
			host = host_create_from_dns(token, 0, 0);
			if (host)
			{
				if (hydra->kernel_interface->get_interface(
										hydra->kernel_interface, host, NULL))
				{
					DBG2(DBG_CFG, "left is other host, swapping ends");
					tmp_end = msg->add_conn.me;
					msg->add_conn.me = msg->add_conn.other;
					msg->add_conn.other = tmp_end;
					swapped = TRUE;
				}
				host->destroy(host);
			}
		}
	}
	enumerator->destroy(enumerator);

	if (!swapped)
	{
		enumerator = enumerator_create_token(msg->add_conn.me.address, ",", " ");
		while (enumerator->enumerate(enumerator, &token))
		{
			if (!strchr(token, '/'))
			{
				host = host_create_from_dns(token, 0, 0);
				if (host)
				{
					if (!hydra->kernel_interface->get_interface(
										hydra->kernel_interface, host, NULL))
					{
						DBG1(DBG_CFG, "left nor right host is our side, "
							 "assuming left=local");
					}
					host->destroy(host);
				}
			}
		}
		enumerator->destroy(enumerator);
	}

	if (msg->add_conn.me.allow_any)
	{
		snprintf(me, sizeof(me), "%s,0.0.0.0/0,::/0",
				 msg->add_conn.me.address);
	}
	if (msg->add_conn.other.allow_any)
	{
		snprintf(other, sizeof(other), "%s,0.0.0.0/0,::/0",
				 msg->add_conn.other.address);
	}
	ikeport = msg->add_conn.me.ikeport;
	ikeport = (ikeport == IKEV2_UDP_PORT) ?
			   charon->socket->get_port(charon->socket, FALSE) : ikeport;
	ike_cfg = ike_cfg_create(msg->add_conn.version,
							 msg->add_conn.other.sendcert != CERT_NEVER_SEND,
							 msg->add_conn.force_encap,
							 msg->add_conn.me.allow_any ?
								me : msg->add_conn.me.address,
							 ikeport,
							 msg->add_conn.other.allow_any ?
								other : msg->add_conn.other.address,
							 msg->add_conn.other.ikeport,
							 msg->add_conn.fragmentation,
							 msg->add_conn.ikedscp);

	add_proposals(this, msg->add_conn.algorithms.ike, ike_cfg, NULL, PROTO_IKE);
	return ike_cfg;
}

/**
 * Add CRL constraint to config
 */
static void build_crl_policy(auth_cfg_t *cfg, bool local, int policy)
{
	/* CRL/OCSP policy, for remote config only */
	if (!local)
	{
		switch (policy)
		{
			case CRL_STRICT_YES:
				/* if yes, we require a GOOD validation */
				cfg->add(cfg, AUTH_RULE_CRL_VALIDATION, VALIDATION_GOOD);
				break;
			case CRL_STRICT_IFURI:
				/* for ifuri, a SKIPPED validation is sufficient */
				cfg->add(cfg, AUTH_RULE_CRL_VALIDATION, VALIDATION_SKIPPED);
				break;
			default:
				break;
		}
	}
}

/**
 * Parse public key / signature strength constraints
 */
static void parse_pubkey_constraints(char *auth, auth_cfg_t *cfg)
{
	enumerator_t *enumerator;
	bool rsa = FALSE, ecdsa = FALSE, rsa_len = FALSE, ecdsa_len = FALSE;
	int strength;
	char *token;

	enumerator = enumerator_create_token(auth, "-", "");
	while (enumerator->enumerate(enumerator, &token))
	{
		bool found = FALSE;
		int i;
		struct {
			char *name;
			signature_scheme_t scheme;
			key_type_t key;
		} schemes[] = {
			{ "md5",		SIGN_RSA_EMSA_PKCS1_MD5,		KEY_RSA,	},
			{ "sha1",		SIGN_RSA_EMSA_PKCS1_SHA1,		KEY_RSA,	},
			{ "sha224",		SIGN_RSA_EMSA_PKCS1_SHA224,		KEY_RSA,	},
			{ "sha256",		SIGN_RSA_EMSA_PKCS1_SHA256,		KEY_RSA,	},
			{ "sha384",		SIGN_RSA_EMSA_PKCS1_SHA384,		KEY_RSA,	},
			{ "sha512",		SIGN_RSA_EMSA_PKCS1_SHA512,		KEY_RSA,	},
			{ "sha1",		SIGN_ECDSA_WITH_SHA1_DER,		KEY_ECDSA,	},
			{ "sha256",		SIGN_ECDSA_WITH_SHA256_DER,		KEY_ECDSA,	},
			{ "sha384",		SIGN_ECDSA_WITH_SHA384_DER,		KEY_ECDSA,	},
			{ "sha512",		SIGN_ECDSA_WITH_SHA512_DER,		KEY_ECDSA,	},
			{ "sha256",		SIGN_ECDSA_256,					KEY_ECDSA,	},
			{ "sha384",		SIGN_ECDSA_384,					KEY_ECDSA,	},
			{ "sha512",		SIGN_ECDSA_521,					KEY_ECDSA,	},
		};

		if (rsa_len || ecdsa_len)
		{	/* expecting a key strength token */
			strength = atoi(token);
			if (strength)
			{
				if (rsa_len)
				{
					cfg->add(cfg, AUTH_RULE_RSA_STRENGTH, (uintptr_t)strength);
				}
				else if (ecdsa_len)
				{
					cfg->add(cfg, AUTH_RULE_ECDSA_STRENGTH, (uintptr_t)strength);
				}
			}
			rsa_len = ecdsa_len = FALSE;
			if (strength)
			{
				continue;
			}
		}
		if (streq(token, "rsa"))
		{
			rsa = rsa_len = TRUE;
			continue;
		}
		if (streq(token, "ecdsa"))
		{
			ecdsa = ecdsa_len = TRUE;
			continue;
		}
		if (streq(token, "pubkey"))
		{
			continue;
		}

		for (i = 0; i < countof(schemes); i++)
		{
			if (streq(schemes[i].name, token))
			{
				/* for each matching string, allow the scheme, if:
				 * - it is an RSA scheme, and we enforced RSA
				 * - it is an ECDSA scheme, and we enforced ECDSA
				 * - it is not a key type specific scheme
				 */
				if ((rsa && schemes[i].key == KEY_RSA) ||
					(ecdsa && schemes[i].key == KEY_ECDSA) ||
					(!rsa && !ecdsa))
				{
					cfg->add(cfg, AUTH_RULE_SIGNATURE_SCHEME,
							 (uintptr_t)schemes[i].scheme);
				}
				found = TRUE;
			}
		}
		if (!found)
		{
			DBG1(DBG_CFG, "ignoring invalid auth token: '%s'", token);
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * build authentication config
 */
static auth_cfg_t *build_auth_cfg(private_stroke_config_t *this,
								  stroke_msg_t *msg, bool local, bool primary)
{
	identification_t *identity;
	certificate_t *certificate;
	char *auth, *id, *pubkey, *cert, *ca, *groups;
	stroke_end_t *end, *other_end;
	auth_cfg_t *cfg;
	bool loose = FALSE;

	/* select strings */
	if (local)
	{
		end = &msg->add_conn.me;
		other_end = &msg->add_conn.other;
	}
	else
	{
		end = &msg->add_conn.other;
		other_end = &msg->add_conn.me;
	}
	if (primary)
	{
		auth = end->auth;
		id = end->id;
		if (!id)
		{	/* leftid/rightid fallback to address */
			id = end->address;
		}
		cert = end->cert;
		ca = end->ca;
		if (ca && streq(ca, "%same"))
		{
			ca = other_end->ca;
		}
	}
	else
	{
		auth = end->auth2;
		id = end->id2;
		if (local && !id)
		{	/* leftid2 falls back to leftid */
			id = end->id;
		}
		cert = end->cert2;
		ca = end->ca2;
		if (ca && streq(ca, "%same"))
		{
			ca = other_end->ca2;
		}
	}
	if (id && *id == '%' && !streq(id, "%any") && !streq(id, "%any6"))
	{	/* has only an effect on rightid/2 */
		loose = !local;
		id++;
	}

	if (!auth)
	{
		if (primary)
		{
			auth = "pubkey";
		}
		else
		{	/* no second authentication round, fine. But load certificates
			 * for other purposes (EAP-TLS) */
			if (cert)
			{
				certificate = this->cred->load_peer(this->cred, cert);
				if (certificate)
				{
					certificate->destroy(certificate);
				}
			}
			return NULL;
		}
	}

	cfg = auth_cfg_create();

	/* add identity and peer certificate */
	identity = identification_create_from_string(id);
	if (cert)
	{
		enumerator_t *enumerator;
		bool has_subject = FALSE;
		certificate_t *first = NULL;

		enumerator = enumerator_create_token(cert, ",", " ");
		while (enumerator->enumerate(enumerator, &cert))
		{
			certificate = this->cred->load_peer(this->cred, cert);
			if (certificate)
			{
				if (local)
				{
					this->ca->check_for_hash_and_url(this->ca, certificate);
				}
				cfg->add(cfg, AUTH_RULE_SUBJECT_CERT, certificate);
				if (!first)
				{
					first = certificate;
				}
				if (identity->get_type(identity) != ID_ANY &&
					certificate->has_subject(certificate, identity))
				{
					has_subject = TRUE;
				}
			}
		}
		enumerator->destroy(enumerator);

		if (first && !has_subject)
		{
			DBG1(DBG_CFG, "  id '%Y' not confirmed by certificate, "
				 "defaulting to '%Y'", identity, first->get_subject(first));
			identity->destroy(identity);
			identity = first->get_subject(first);
			identity = identity->clone(identity);
		}
	}
	/* add raw RSA public key */
	pubkey = end->rsakey;
	if (pubkey && !streq(pubkey, "") && !streq(pubkey, "%cert"))
	{
		certificate = this->cred->load_pubkey(this->cred, pubkey, identity);
		if (certificate)
		{
			cfg->add(cfg, AUTH_RULE_SUBJECT_CERT, certificate);
		}
	}
	if (identity->get_type(identity) != ID_ANY)
	{
		cfg->add(cfg, AUTH_RULE_IDENTITY, identity);
		if (loose)
		{
			cfg->add(cfg, AUTH_RULE_IDENTITY_LOOSE, TRUE);
		}
	}
	else
	{
		identity->destroy(identity);
	}

	/* CA constraint */
	if (ca)
	{
		identity = identification_create_from_string(ca);
		certificate = lib->credmgr->get_cert(lib->credmgr, CERT_X509,
											 KEY_ANY, identity, TRUE);
		identity->destroy(identity);
		if (certificate)
		{
			cfg->add(cfg, AUTH_RULE_CA_CERT, certificate);
		}
		else
		{
			DBG1(DBG_CFG, "CA certificate \"%s\" not found, discarding CA "
				 "constraint", ca);
		}
	}

	/* groups */
	groups = primary ? end->groups : end->groups2;
	if (groups)
	{
		enumerator_t *enumerator;
		char *group;

		enumerator = enumerator_create_token(groups, ",", " ");
		while (enumerator->enumerate(enumerator, &group))
		{
			cfg->add(cfg, AUTH_RULE_GROUP,
					 identification_create_from_string(group));
		}
		enumerator->destroy(enumerator);
	}

	/* certificatePolicies */
	if (end->cert_policy)
	{
		enumerator_t *enumerator;
		char *policy;

		enumerator = enumerator_create_token(end->cert_policy, ",", " ");
		while (enumerator->enumerate(enumerator, &policy))
		{
			cfg->add(cfg, AUTH_RULE_CERT_POLICY, strdup(policy));
		}
		enumerator->destroy(enumerator);
	}

	/* authentication metod (class, actually) */
	if (strpfx(auth, "pubkey") ||
		strpfx(auth, "rsa") ||
		strpfx(auth, "ecdsa"))
	{
		cfg->add(cfg, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
		build_crl_policy(cfg, local, msg->add_conn.crl_policy);

		parse_pubkey_constraints(auth, cfg);
	}
	else if (streq(auth, "psk") || streq(auth, "secret"))
	{
		cfg->add(cfg, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PSK);
	}
	else if (strpfx(auth, "xauth"))
	{
		char *pos;

		pos = strchr(auth, '-');
		if (pos)
		{
			cfg->add(cfg, AUTH_RULE_XAUTH_BACKEND, strdup(++pos));
		}
		cfg->add(cfg, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_XAUTH);
		if (msg->add_conn.xauth_identity)
		{
			cfg->add(cfg, AUTH_RULE_XAUTH_IDENTITY,
				identification_create_from_string(msg->add_conn.xauth_identity));
		}
	}
	else if (strpfx(auth, "eap"))
	{
		eap_vendor_type_t *type;

		cfg->add(cfg, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_EAP);

		type = eap_vendor_type_from_string(auth);
		if (type)
		{
			cfg->add(cfg, AUTH_RULE_EAP_TYPE, type->type);
			if (type->vendor)
			{
				cfg->add(cfg, AUTH_RULE_EAP_VENDOR, type->vendor);
			}
			free(type);
		}

		if (msg->add_conn.eap_identity)
		{
			if (streq(msg->add_conn.eap_identity, "%identity"))
			{
				identity = identification_create_from_encoding(ID_ANY,
															   chunk_empty);
			}
			else
			{
				identity = identification_create_from_string(
													msg->add_conn.eap_identity);
			}
			cfg->add(cfg, AUTH_RULE_EAP_IDENTITY, identity);
		}
		if (msg->add_conn.aaa_identity)
		{
			cfg->add(cfg, AUTH_RULE_AAA_IDENTITY,
				identification_create_from_string(msg->add_conn.aaa_identity));
		}
	}
	else
	{
		if (!streq(auth, "any"))
		{
			DBG1(DBG_CFG, "authentication method %s unknown, fallback to any",
				 auth);
		}
		build_crl_policy(cfg, local, msg->add_conn.crl_policy);
	}
	return cfg;
}

/**
 * build a peer_cfg from a stroke msg
 */
static peer_cfg_t *build_peer_cfg(private_stroke_config_t *this,
								  stroke_msg_t *msg, ike_cfg_t *ike_cfg)
{
	identification_t *peer_id = NULL;
	peer_cfg_t *mediated_by = NULL;
	unique_policy_t unique;
	u_int32_t rekey = 0, reauth = 0, over, jitter;
	peer_cfg_t *peer_cfg;
	auth_cfg_t *auth_cfg;

#ifdef ME
	if (msg->add_conn.ikeme.mediation && msg->add_conn.ikeme.mediated_by)
	{
		DBG1(DBG_CFG, "a mediation connection cannot be a mediated connection "
			 "at the same time, aborting");
		return NULL;
	}

	if (msg->add_conn.ikeme.mediation)
	{
		/* force unique connections for mediation connections */
		msg->add_conn.unique = 1;
	}

	if (msg->add_conn.ikeme.mediated_by)
	{
		mediated_by = charon->backends->get_peer_cfg_by_name(charon->backends,
											msg->add_conn.ikeme.mediated_by);
		if (!mediated_by)
		{
			DBG1(DBG_CFG, "mediation connection '%s' not found, aborting",
				 msg->add_conn.ikeme.mediated_by);
			return NULL;
		}
		if (!mediated_by->is_mediation(mediated_by))
		{
			DBG1(DBG_CFG, "connection '%s' as referred to by '%s' is "
				 "no mediation connection, aborting",
				 msg->add_conn.ikeme.mediated_by, msg->add_conn.name);
			mediated_by->destroy(mediated_by);
			return NULL;
		}
		if (msg->add_conn.ikeme.peerid)
		{
			peer_id = identification_create_from_string(msg->add_conn.ikeme.peerid);
		}
		else if (msg->add_conn.other.id)
		{
			peer_id = identification_create_from_string(msg->add_conn.other.id);
		}
	}
#endif /* ME */

	jitter = msg->add_conn.rekey.margin * msg->add_conn.rekey.fuzz / 100;
	over = msg->add_conn.rekey.margin;
	if (msg->add_conn.rekey.reauth)
	{
		reauth = msg->add_conn.rekey.ike_lifetime - over;
	}
	else
	{
		rekey = msg->add_conn.rekey.ike_lifetime - over;
	}
	switch (msg->add_conn.unique)
	{
		case 1: /* yes */
		case 2: /* replace */
			unique = UNIQUE_REPLACE;
			break;
		case 3: /* keep */
			unique = UNIQUE_KEEP;
			break;
		case 4: /* never */
			unique = UNIQUE_NEVER;
			break;
		default: /* no */
			unique = UNIQUE_NO;
			break;
	}
	if (msg->add_conn.dpd.action == 0)
	{	/* dpdaction=none disables DPD */
		msg->add_conn.dpd.delay = 0;
	}

	/* other.sourceip is managed in stroke_attributes. If it is set, we define
	 * the pool name as the connection name, which the attribute provider
	 * uses to serve pool addresses. */
	peer_cfg = peer_cfg_create(msg->add_conn.name, ike_cfg,
		msg->add_conn.me.sendcert, unique,
		msg->add_conn.rekey.tries, rekey, reauth, jitter, over,
		msg->add_conn.mobike, msg->add_conn.aggressive,
		msg->add_conn.pushmode == 0,
		msg->add_conn.dpd.delay, msg->add_conn.dpd.timeout,
		msg->add_conn.ikeme.mediation, mediated_by, peer_id);

	if (msg->add_conn.other.sourceip)
	{
		enumerator_t *enumerator;
		char *token;

		enumerator = enumerator_create_token(msg->add_conn.other.sourceip,
											 ",", " ");
		while (enumerator->enumerate(enumerator, &token))
		{
			if (streq(token, "%modeconfig") || streq(token, "%modecfg") ||
				streq(token, "%config") || streq(token, "%cfg") ||
				streq(token, "%config4") || streq(token, "%config6"))
			{
				/* empty pool, uses connection name */
				this->attributes->add_pool(this->attributes,
								mem_pool_create(msg->add_conn.name, NULL, 0));
				peer_cfg->add_pool(peer_cfg, msg->add_conn.name);
			}
			else if (*token == '%')
			{
				/* external named pool */
				peer_cfg->add_pool(peer_cfg, token + 1);
			}
			else
			{
				/* in-memory pool, named using CIDR notation */
				host_t *base;
				int bits;

				base = host_create_from_subnet(token, &bits);
				if (base)
				{
					this->attributes->add_pool(this->attributes,
										mem_pool_create(token, base, bits));
					peer_cfg->add_pool(peer_cfg, token);
					base->destroy(base);
				}
				else
				{
					DBG1(DBG_CFG, "IP pool %s invalid, ignored", token);
				}
			}
		}
		enumerator->destroy(enumerator);
	}

	if (msg->add_conn.me.sourceip && msg->add_conn.other.sourceip)
	{
		DBG1(DBG_CFG, "'%s' has both left- and rightsourceip, but IKE can "
			 "negotiate one virtual IP only, ignoring local virtual IP",
			 msg->add_conn.name);
	}
	else if (msg->add_conn.me.sourceip)
	{
		enumerator_t *enumerator;
		char *token;

		enumerator = enumerator_create_token(msg->add_conn.me.sourceip, ",", " ");
		while (enumerator->enumerate(enumerator, &token))
		{
			host_t *vip = NULL;

			if (streq(token, "%modeconfig") || streq(token, "%modecfg") ||
				streq(token, "%config") || streq(token, "%cfg"))
			{	/* try to deduce an address family */
				if (msg->add_conn.me.subnets)
				{	/* use the same family as in local subnet, if any */
					if (strchr(msg->add_conn.me.subnets, '.'))
					{
						vip = host_create_any(AF_INET);
					}
					else
					{
						vip = host_create_any(AF_INET6);
					}
				}
				else if (msg->add_conn.other.subnets)
				{	/* use the same family as in remote subnet, if any */
					if (strchr(msg->add_conn.other.subnets, '.'))
					{
						vip = host_create_any(AF_INET);
					}
					else
					{
						vip = host_create_any(AF_INET6);
					}
				}
				else
				{
					char *addr, *next, *hit;

					/* guess virtual IP family based on local address. If
					 * multiple addresses are specified, we look at the first
					 * only, as with leftallowany a ::/0 is always appended. */
					addr = ike_cfg->get_my_addr(ike_cfg);
					next = strchr(addr, ',');
					hit = strchr(addr, ':');
					if (hit && (!next || hit < next))
					{
						vip = host_create_any(AF_INET6);
					}
					else
					{
						vip = host_create_any(AF_INET);
					}
				}
			}
			else if (streq(token, "%config4"))
			{
				vip = host_create_any(AF_INET);
			}
			else if (streq(token, "%config6"))
			{
				vip = host_create_any(AF_INET6);
			}
			else
			{
				vip = host_create_from_string(token, 0);
				if (!vip)
				{
					DBG1(DBG_CFG, "ignored invalid subnet token: %s", token);
				}
			}

			if (vip)
			{
				peer_cfg->add_virtual_ip(peer_cfg, vip);
			}
		}
		enumerator->destroy(enumerator);
	}

	/* build leftauth= */
	auth_cfg = build_auth_cfg(this, msg, TRUE, TRUE);
	if (auth_cfg)
	{
		peer_cfg->add_auth_cfg(peer_cfg, auth_cfg, TRUE);
	}
	else
	{	/* we require at least one config on our side */
		peer_cfg->destroy(peer_cfg);
		return NULL;
	}
	/* build leftauth2= */
	auth_cfg = build_auth_cfg(this, msg, TRUE, FALSE);
	if (auth_cfg)
	{
		peer_cfg->add_auth_cfg(peer_cfg, auth_cfg, TRUE);
	}
	/* build rightauth= */
	auth_cfg = build_auth_cfg(this, msg, FALSE, TRUE);
	if (auth_cfg)
	{
		peer_cfg->add_auth_cfg(peer_cfg, auth_cfg, FALSE);
	}
	/* build rightauth2= */
	auth_cfg = build_auth_cfg(this, msg, FALSE, FALSE);
	if (auth_cfg)
	{
		peer_cfg->add_auth_cfg(peer_cfg, auth_cfg, FALSE);
	}
	return peer_cfg;
}

/**
 * Parse a protoport specifier
 */
static bool parse_protoport(char *token, u_int16_t *from_port,
							u_int16_t *to_port, u_int8_t *protocol)
{
	char *sep, *port = "", *endptr;
	struct protoent *proto;
	struct servent *svc;
	long int p;

	sep = strrchr(token, ']');
	if (!sep)
	{
		return FALSE;
	}
	*sep = '\0';

	sep = strchr(token, '/');
	if (sep)
	{	/* protocol/port */
		*sep = '\0';
		port = sep + 1;
	}

	if (streq(token, "%any"))
	{
		*protocol = 0;
	}
	else
	{
		proto = getprotobyname(token);
		if (proto)
		{
			*protocol = proto->p_proto;
		}
		else
		{
			p = strtol(token, &endptr, 0);
			if ((*token && *endptr) || p < 0 || p > 0xff)
			{
				return FALSE;
			}
			*protocol = (u_int8_t)p;
		}
	}
	if (streq(port, "%any"))
	{
		*from_port = 0;
		*to_port = 0xffff;
	}
	else if (streq(port, "%opaque"))
	{
		*from_port = 0xffff;
		*to_port = 0;
	}
	else if (*port)
	{
		svc = getservbyname(port, NULL);
		if (svc)
		{
			*from_port = *to_port = ntohs(svc->s_port);
		}
		else
		{
			p = strtol(port, &endptr, 0);
			if (p < 0 || p > 0xffff)
			{
				return FALSE;
			}
			*from_port = p;
			if (*endptr == '-')
			{
				port = endptr + 1;
				p = strtol(port, &endptr, 0);
				if (p < 0 || p > 0xffff)
				{
					return FALSE;
				}
			}
			*to_port = p;
			if (*endptr)
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

/**
 * build a traffic selector from a stroke_end
 */
static void add_ts(private_stroke_config_t *this,
				   stroke_end_t *end, child_cfg_t *child_cfg, bool local)
{
	traffic_selector_t *ts;

	if (end->tohost)
	{
		ts = traffic_selector_create_dynamic(end->protocol,
											 end->from_port, end->to_port);
		child_cfg->add_traffic_selector(child_cfg, local, ts);
	}
	else
	{
		if (!end->subnets)
		{
			host_t *net;

			net = host_create_from_string(end->address, 0);
			if (net)
			{
				ts = traffic_selector_create_from_subnet(net, 0, end->protocol,
												end->from_port, end->to_port);
				child_cfg->add_traffic_selector(child_cfg, local, ts);
			}
		}
		else
		{
			enumerator_t *enumerator;
			char *subnet, *pos;
			u_int16_t from_port, to_port;
			u_int8_t proto;

			enumerator = enumerator_create_token(end->subnets, ",", " ");
			while (enumerator->enumerate(enumerator, &subnet))
			{
				from_port = end->from_port;
				to_port = end->to_port;
				proto = end->protocol;

				pos = strchr(subnet, '[');
				if (pos)
				{
					*(pos++) = '\0';
					if (!parse_protoport(pos, &from_port, &to_port, &proto))
					{
						DBG1(DBG_CFG, "invalid proto/port: %s, skipped subnet",
							 pos);
						continue;
					}
				}
				if (streq(subnet, "%dynamic"))
				{
					ts = traffic_selector_create_dynamic(proto,
														 from_port, to_port);
				}
				else
				{
					ts = traffic_selector_create_from_cidr(subnet, proto,
														   from_port, to_port);
				}
				if (ts)
				{
					child_cfg->add_traffic_selector(child_cfg, local, ts);
				}
				else
				{
					DBG1(DBG_CFG, "invalid subnet: %s, skipped", subnet);
				}
			}
			enumerator->destroy(enumerator);
		}
	}
}

/**
 * map starter magic values to our action type
 */
static action_t map_action(int starter_action)
{
	switch (starter_action)
	{
		case 2: /* =hold */
			return ACTION_ROUTE;
		case 3: /* =restart */
			return ACTION_RESTART;
		default:
			return ACTION_NONE;
	}
}

/**
 * build a child config from the stroke message
 */
static child_cfg_t *build_child_cfg(private_stroke_config_t *this,
									stroke_msg_t *msg)
{
	child_cfg_t *child_cfg;
	lifetime_cfg_t lifetime = {
		.time = {
			.life = msg->add_conn.rekey.ipsec_lifetime,
			.rekey = msg->add_conn.rekey.ipsec_lifetime - msg->add_conn.rekey.margin,
			.jitter = msg->add_conn.rekey.margin * msg->add_conn.rekey.fuzz / 100
		},
		.bytes = {
			.life = msg->add_conn.rekey.life_bytes,
			.rekey = msg->add_conn.rekey.life_bytes - msg->add_conn.rekey.margin_bytes,
			.jitter = msg->add_conn.rekey.margin_bytes * msg->add_conn.rekey.fuzz / 100
		},
		.packets = {
			.life = msg->add_conn.rekey.life_packets,
			.rekey = msg->add_conn.rekey.life_packets - msg->add_conn.rekey.margin_packets,
			.jitter = msg->add_conn.rekey.margin_packets * msg->add_conn.rekey.fuzz / 100
		}
	};
	mark_t mark_in = {
		.value = msg->add_conn.mark_in.value,
		.mask = msg->add_conn.mark_in.mask
	};
	mark_t mark_out = {
		.value = msg->add_conn.mark_out.value,
		.mask = msg->add_conn.mark_out.mask
	};

	child_cfg = child_cfg_create(
				msg->add_conn.name, &lifetime, msg->add_conn.me.updown,
				msg->add_conn.me.hostaccess, msg->add_conn.mode, ACTION_NONE,
				map_action(msg->add_conn.dpd.action),
				map_action(msg->add_conn.close_action), msg->add_conn.ipcomp,
				msg->add_conn.inactivity, msg->add_conn.reqid,
				&mark_in, &mark_out, msg->add_conn.tfc);
	if (msg->add_conn.replay_window != -1)
	{
		child_cfg->set_replay_window(child_cfg, msg->add_conn.replay_window);
	}
	child_cfg->set_mipv6_options(child_cfg, msg->add_conn.proxy_mode,
											msg->add_conn.install_policy);
	add_ts(this, &msg->add_conn.me, child_cfg, TRUE);
	add_ts(this, &msg->add_conn.other, child_cfg, FALSE);

	if (msg->add_conn.algorithms.ah)
	{
		add_proposals(this, msg->add_conn.algorithms.ah,
					  NULL, child_cfg, PROTO_AH);
	}
	else
	{
		add_proposals(this, msg->add_conn.algorithms.esp,
					  NULL, child_cfg, PROTO_ESP);
	}
	return child_cfg;
}

METHOD(stroke_config_t, add, void,
	private_stroke_config_t *this, stroke_msg_t *msg)
{
	ike_cfg_t *ike_cfg, *existing_ike;
	peer_cfg_t *peer_cfg, *existing;
	child_cfg_t *child_cfg;
	enumerator_t *enumerator;
	bool use_existing = FALSE;

	ike_cfg = build_ike_cfg(this, msg);
	if (!ike_cfg)
	{
		return;
	}
	peer_cfg = build_peer_cfg(this, msg, ike_cfg);
	if (!peer_cfg)
	{
		ike_cfg->destroy(ike_cfg);
		return;
	}

	enumerator = create_peer_cfg_enumerator(this, NULL, NULL);
	while (enumerator->enumerate(enumerator, &existing))
	{
		existing_ike = existing->get_ike_cfg(existing);
		if (existing->equals(existing, peer_cfg) &&
			existing_ike->equals(existing_ike, peer_cfg->get_ike_cfg(peer_cfg)))
		{
			use_existing = TRUE;
			peer_cfg->destroy(peer_cfg);
			peer_cfg = existing;
			peer_cfg->get_ref(peer_cfg);
			DBG1(DBG_CFG, "added child to existing configuration '%s'",
				 peer_cfg->get_name(peer_cfg));
			break;
		}
	}
	enumerator->destroy(enumerator);

	child_cfg = build_child_cfg(this, msg);
	if (!child_cfg)
	{
		peer_cfg->destroy(peer_cfg);
		return;
	}
	peer_cfg->add_child_cfg(peer_cfg, child_cfg);

	if (use_existing)
	{
		peer_cfg->destroy(peer_cfg);
	}
	else
	{
		/* add config to backend */
		DBG1(DBG_CFG, "added configuration '%s'", msg->add_conn.name);
		this->mutex->lock(this->mutex);
		this->list->insert_last(this->list, peer_cfg);
		this->mutex->unlock(this->mutex);
	}
}

METHOD(stroke_config_t, del, void,
	private_stroke_config_t *this, stroke_msg_t *msg)
{
	enumerator_t *enumerator, *children;
	peer_cfg_t *peer;
	child_cfg_t *child;
	bool deleted = FALSE;

	this->mutex->lock(this->mutex);
	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &peer))
	{
		bool keep = FALSE;

		/* remove any child with such a name */
		children = peer->create_child_cfg_enumerator(peer);
		while (children->enumerate(children, &child))
		{
			if (streq(child->get_name(child), msg->del_conn.name))
			{
				peer->remove_child_cfg(peer, children);
				child->destroy(child);
				deleted = TRUE;
			}
			else
			{
				keep = TRUE;
			}
		}
		children->destroy(children);

		/* if peer config has no children anymore, remove it */
		if (!keep)
		{
			this->list->remove_at(this->list, enumerator);
			peer->destroy(peer);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);

	if (deleted)
	{
		DBG1(DBG_CFG, "deleted connection '%s'", msg->del_conn.name);
	}
	else
	{
		DBG1(DBG_CFG, "connection '%s' not found", msg->del_conn.name);
	}
}

METHOD(stroke_config_t, set_user_credentials, void,
	private_stroke_config_t *this, stroke_msg_t *msg, FILE *prompt)
{
	enumerator_t *enumerator, *children, *remote_auth;
	peer_cfg_t *peer, *found = NULL;
	auth_cfg_t *auth_cfg, *remote_cfg;
	auth_class_t auth_class;
	child_cfg_t *child;
	identification_t *id, *identity, *gw = NULL;
	shared_key_type_t type = SHARED_ANY;
	chunk_t password = chunk_empty;

	this->mutex->lock(this->mutex);
	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, (void**)&peer))
	{	/* find the peer (or child) config with the given name */
		if (streq(peer->get_name(peer), msg->user_creds.name))
		{
			found = peer;
		}
		else
		{
			children = peer->create_child_cfg_enumerator(peer);
			while (children->enumerate(children, &child))
			{
				if (streq(child->get_name(child), msg->user_creds.name))
				{
					found = peer;
					break;
				}
			}
			children->destroy(children);
		}

		if (found)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (!found)
	{
		DBG1(DBG_CFG, "  no config named '%s'", msg->user_creds.name);
		fprintf(prompt, "no config named '%s'\n", msg->user_creds.name);
		this->mutex->unlock(this->mutex);
		return;
	}

	id = identification_create_from_string(msg->user_creds.username);
	if (strlen(msg->user_creds.username) == 0 ||
		!id || id->get_type(id) == ID_ANY)
	{
		DBG1(DBG_CFG, "  invalid username '%s'", msg->user_creds.username);
		fprintf(prompt, "invalid username '%s'\n", msg->user_creds.username);
		this->mutex->unlock(this->mutex);
		DESTROY_IF(id);
		return;
	}

	/* replace/set the username in the first EAP/XAuth auth_cfg, also look for
	 * a suitable remote ID.
	 * note that adding the identity here is not fully thread-safe as the
	 * peer_cfg and in turn the auth_cfg could be in use. for the default use
	 * case (setting user credentials before upping the connection) this will
	 * not be a problem, though. */
	enumerator = found->create_auth_cfg_enumerator(found, TRUE);
	remote_auth = found->create_auth_cfg_enumerator(found, FALSE);
	while (enumerator->enumerate(enumerator, (void**)&auth_cfg))
	{
		if (remote_auth->enumerate(remote_auth, (void**)&remote_cfg))
		{	/* fall back on rightid, in case aaa_identity is not specified */
			identity = remote_cfg->get(remote_cfg, AUTH_RULE_IDENTITY);
			if (identity && identity->get_type(identity) != ID_ANY)
			{
				gw = identity;
			}
		}

		auth_class = (uintptr_t)auth_cfg->get(auth_cfg, AUTH_RULE_AUTH_CLASS);
		if (auth_class == AUTH_CLASS_EAP || auth_class == AUTH_CLASS_XAUTH)
		{
			if (auth_class == AUTH_CLASS_EAP)
			{
				auth_cfg->add(auth_cfg, AUTH_RULE_EAP_IDENTITY, id->clone(id));
				/* if aaa_identity is specified use that as remote ID */
				identity = auth_cfg->get(auth_cfg, AUTH_RULE_AAA_IDENTITY);
				if (identity && identity->get_type(identity) != ID_ANY)
				{
					gw = identity;
				}
				DBG1(DBG_CFG, "  configured EAP-Identity %Y", id);
			}
			else
			{
				auth_cfg->add(auth_cfg, AUTH_RULE_XAUTH_IDENTITY,
							  id->clone(id));
				DBG1(DBG_CFG, "  configured XAuth username %Y", id);
			}
			type = SHARED_EAP;
			break;
		}
	}
	enumerator->destroy(enumerator);
	remote_auth->destroy(remote_auth);
	/* clone the gw ID before unlocking the mutex */
	if (gw)
	{
		gw = gw->clone(gw);
	}
	this->mutex->unlock(this->mutex);

	if (type == SHARED_ANY)
	{
		DBG1(DBG_CFG, "  config '%s' unsuitable for user credentials",
			 msg->user_creds.name);
		fprintf(prompt, "config '%s' unsuitable for user credentials\n",
				msg->user_creds.name);
		id->destroy(id);
		DESTROY_IF(gw);
		return;
	}

	if (msg->user_creds.password)
	{
		char *pass;

		pass = msg->user_creds.password;
		password = chunk_clone(chunk_create(pass, strlen(pass)));
		memwipe(pass, strlen(pass));
	}
	else
	{	/* prompt the user for the password */
		char buf[256];

		fprintf(prompt, "Password:\n");
		if (fgets(buf, sizeof(buf), prompt))
		{
			password = chunk_clone(chunk_create(buf, strlen(buf)));
			if (password.len > 0)
			{	/* trim trailing \n */
				password.len--;
			}
			memwipe(buf, sizeof(buf));
		}
	}

	if (password.len)
	{
		shared_key_t *shared;
		linked_list_t *owners;

		shared = shared_key_create(type, password);

		owners = linked_list_create();
		owners->insert_last(owners, id->clone(id));
		if (gw && gw->get_type(gw) != ID_ANY)
		{
			owners->insert_last(owners, gw->clone(gw));
			DBG1(DBG_CFG, "  added %N secret for %Y %Y", shared_key_type_names,
				 type, id, gw);
		}
		else
		{
			DBG1(DBG_CFG, "  added %N secret for %Y", shared_key_type_names,
				 type, id);
		}
		this->cred->add_shared(this->cred, shared, owners);
		DBG4(DBG_CFG, "  secret: %#B", &password);
	}
	else
	{	/* in case a user answers the password prompt by just pressing enter */
		chunk_clear(&password);
	}
	id->destroy(id);
	DESTROY_IF(gw);
}

METHOD(stroke_config_t, destroy, void,
	private_stroke_config_t *this)
{
	this->list->destroy_offset(this->list, offsetof(peer_cfg_t, destroy));
	this->mutex->destroy(this->mutex);
	free(this);
}

/*
 * see header file
 */
stroke_config_t *stroke_config_create(stroke_ca_t *ca, stroke_cred_t *cred,
									  stroke_attribute_t *attributes)
{
	private_stroke_config_t *this;

	INIT(this,
		.public = {
			.backend = {
				.create_peer_cfg_enumerator = _create_peer_cfg_enumerator,
				.create_ike_cfg_enumerator = _create_ike_cfg_enumerator,
				.get_peer_cfg_by_name = _get_peer_cfg_by_name,
			},
			.add = _add,
			.del = _del,
			.set_user_credentials = _set_user_credentials,
			.destroy = _destroy,
		},
		.list = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_RECURSIVE),
		.ca = ca,
		.cred = cred,
		.attributes = attributes,
	);

	return &this->public;
}
