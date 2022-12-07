/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2009 Andreas Steffen
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

#include "addrblock_validator.h"

#include <utils/debug.h>
#include <credentials/certificates/x509.h>
#include <selectors/traffic_selector.h>

typedef struct private_addrblock_validator_t private_addrblock_validator_t;

/**
 * Private data of an addrblock_validator_t object.
 */
struct private_addrblock_validator_t {

	/**
	 * Public addrblock_validator_t interface.
	 */
	addrblock_validator_t public;

	/**
	 * Whether to reject subject certificates not having a addrBlock extension
	 */
	bool strict;

	/**
	 * How deep to validate issuer parent addrBlock validity, -1 for full
	 */
	int depth;
};

/**
 * Do the addrblock check for two x509 plugins
 */
static bool check_addrblock(private_addrblock_validator_t *this,
							x509_t *subject, x509_t *issuer, u_int pathlen)
{
	bool subject_const, issuer_const, contained = TRUE;
	enumerator_t *subject_enumerator, *issuer_enumerator;
	traffic_selector_t *subject_ts, *issuer_ts;

	subject_const = subject->get_flags(subject) & X509_IP_ADDR_BLOCKS;
	issuer_const = issuer->get_flags(issuer) & X509_IP_ADDR_BLOCKS;

	if (!subject_const && !issuer_const)
	{
		return TRUE;
	}
	if (!subject_const)
	{
		DBG1(DBG_CFG, "subject certificate lacks ipAddrBlocks extension");
		return !this->strict;
	}
	if (this->depth >= 0 && this->depth <= pathlen)
	{
		/* at pathlen 0: skip for depth configuration == 0,
		 * at pathlen 1: skip for depth configurations 0..1,
		 * ... */
		DBG1(DBG_CFG, "  skipping issuer ipAddrBlocks validation "
					  "at pathlen %u", pathlen);
		return TRUE;
	}
	if (!issuer_const)
	{
		DBG1(DBG_CFG, "issuer certificate lacks ipAddrBlocks extension");
		return FALSE;
	}
	subject_enumerator = subject->create_ipAddrBlock_enumerator(subject);
	while (subject_enumerator->enumerate(subject_enumerator, &subject_ts))
	{
		contained = FALSE;

		issuer_enumerator = issuer->create_ipAddrBlock_enumerator(issuer);
		while (issuer_enumerator->enumerate(issuer_enumerator, &issuer_ts))
		{
			if (subject_ts->is_contained_in(subject_ts, issuer_ts))
			{
				DBG2(DBG_CFG, "  subject address block %R is contained in "
							  "issuer address block %R", subject_ts, issuer_ts);
				contained = TRUE;
				break;
			}
		}
		issuer_enumerator->destroy(issuer_enumerator);
		if (!contained)
		{
			DBG1(DBG_CFG, "subject address block %R is not contained in any "
						  "issuer address block", subject_ts);
			break;
		}
	}
	subject_enumerator->destroy(subject_enumerator);
	return contained;
}

METHOD(cert_validator_t, validate, bool,
	private_addrblock_validator_t *this, certificate_t *subject,
	certificate_t *issuer, u_int pathlen, bool anchor, auth_cfg_t *auth)
{
	if (subject->get_type(subject) == CERT_X509 &&
		issuer->get_type(issuer) == CERT_X509)
	{
		if (!check_addrblock(this, (x509_t*)subject, (x509_t*)issuer, pathlen))
		{
			lib->credmgr->call_hook(lib->credmgr, CRED_HOOK_POLICY_VIOLATION,
									subject);
			return FALSE;
		}
	}
	return TRUE;
}

METHOD(addrblock_validator_t, destroy, void,
	private_addrblock_validator_t *this)
{
	free(this);
}

/**
 * See header
 */
addrblock_validator_t *addrblock_validator_create()
{
	private_addrblock_validator_t *this;

	INIT(this,
		.public = {
			.validator = {
				.validate = _validate,
			},
			.destroy = _destroy,
		},
		.strict = lib->settings->get_bool(lib->settings,
						"%s.plugins.addrblock.strict", TRUE, lib->ns),
		.depth = lib->settings->get_int(lib->settings,
						"%s.plugins.addrblock.depth", -1, lib->ns),
	);

	return &this->public;
}
