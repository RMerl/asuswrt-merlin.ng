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

#include "coupling_validator.h"

#include <errno.h>
#include <time.h>

#include <daemon.h>
#include <threading/mutex.h>

/* buffer size for hex-encoded hash */
#define MAX_HASH_SIZE (HASH_SIZE_SHA512 * 2 + 1)

typedef struct private_coupling_validator_t private_coupling_validator_t;

/**
 * Private data of an coupling_validator_t object.
 */
struct private_coupling_validator_t {

	/**
	 * Public coupling_validator_t interface.
	 */
	coupling_validator_t public;

	/**
	 * Mutex
	 */
	mutex_t *mutex;

	/**
	 * File with device couplings
	 */
	FILE *f;

	/**
	 * Hasher to create hashes
	 */
	hasher_t *hasher;

	/**
	 * maximum number of couplings
	 */
	int max_couplings;
};

/**
 * Get hash of a certificate
 */
static bool get_cert_hash(private_coupling_validator_t *this,
						  certificate_t *cert, char *hex)
{
	char buf[MAX_HASH_SIZE];
	chunk_t encoding;

	if (!cert->get_encoding(cert, CERT_ASN1_DER, &encoding))
	{
		return FALSE;
	}
	if (!this->hasher->get_hash(this->hasher, encoding, buf))
	{
		free(encoding.ptr);
		return FALSE;
	}
	free(encoding.ptr);
	chunk_to_hex(chunk_create(buf, this->hasher->get_hash_size(this->hasher)),
				 hex, FALSE);
	return TRUE;
}

/**
 * Check if we have an entry for a given hash
 */
static bool has_entry(private_coupling_validator_t *this, char *hash)
{
	char line[256];
	int hash_len;

	hash_len = strlen(hash);
	rewind(this->f);

	while (fgets(line, sizeof(line), this->f))
	{
		if (strlen(line) >= hash_len &&
			strncaseeq(line, hash, hash_len))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Get the number of coupling entries we currently have
 */
static int get_number_of_entries(private_coupling_validator_t *this)
{
	char line[256];
	int count = 0;

	rewind(this->f);

	while (fgets(line, sizeof(line), this->f))
	{
		/* valid entries start with hex encoded hash */
		if (strchr("1234567890abcdefABCDEF", line[0]))
		{
			count++;
		}
	}
	return count;
}

/**
 * Add a new entry to the file
 */
static bool add_entry(private_coupling_validator_t *this, char *hash,
					  identification_t *id)
{
	return fseek(this->f, 0, SEEK_END) == 0 &&
		   fprintf(this->f, "%s %u '%Y'\n", hash, time(NULL), id) > 0;
}

METHOD(cert_validator_t, validate, bool,
	private_coupling_validator_t *this,
	certificate_t *subject, certificate_t *issuer,
	bool online, u_int pathlen, bool anchor, auth_cfg_t *auth)
{
	bool valid = FALSE;
	char hash[MAX_HASH_SIZE];

	if (pathlen != 0)
	{
		return TRUE;
	}
	if (get_cert_hash(this, subject, hash))
	{
		this->mutex->lock(this->mutex);
		if (has_entry(this, hash))
		{
			DBG1(DBG_CFG, "coupled certificate '%Y' found, accepted",
				 subject->get_subject(subject));
			valid = TRUE;
		}
		else if (get_number_of_entries(this) < this->max_couplings)
		{
			if (add_entry(this, hash, subject->get_subject(subject)))
			{
				DBG1(DBG_CFG, "coupled new certificate '%Y'",
					 subject->get_subject(subject));
				valid = TRUE;
			}
			else
			{
				DBG1(DBG_CFG, "coupling new certificate '%Y' failed",
					 subject->get_subject(subject));
				lib->credmgr->call_hook(lib->credmgr,
										CRED_HOOK_POLICY_VIOLATION, subject);
			}
		}
		else
		{
			DBG1(DBG_CFG, "coupling new certificate '%Y' failed, limit of %d "
				 "couplings reached", subject->get_subject(subject),
				 this->max_couplings);
			lib->credmgr->call_hook(lib->credmgr, CRED_HOOK_POLICY_VIOLATION,
									subject);
		}
		this->mutex->unlock(this->mutex);
	}
	return valid;
}

METHOD(coupling_validator_t, destroy, void,
	private_coupling_validator_t *this)
{
	if (this->f)
	{
		fclose(this->f);
	}
	DESTROY_IF(this->hasher);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
coupling_validator_t *coupling_validator_create()
{
	private_coupling_validator_t *this;
	hash_algorithm_t alg;
	char *path, *hash;

	INIT(this,
		.public = {
			.validator = {
				.validate = _validate,
			},
			.destroy = _destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.max_couplings = lib->settings->get_int(lib->settings,
												"%s.plugins.coupling.max", 1,
												lib->ns),
	);

	hash = lib->settings->get_str(lib->settings,
								  "%s.plugins.coupling.hash", "sha1", lib->ns);
	if (!enum_from_name(hash_algorithm_short_names, hash, &alg))
	{
		DBG1(DBG_CFG, "unknown coupling hash algorithm: %s", hash);
		destroy(this);
		return NULL;
	}
	this->hasher = lib->crypto->create_hasher(lib->crypto, alg);
	if (!this->hasher)
	{
		DBG1(DBG_CFG, "unsupported coupling hash algorithm: %s", hash);
		destroy(this);
		return NULL;
	}

	path = lib->settings->get_str(lib->settings,
								  "%s.plugins.coupling.file", NULL, lib->ns);
	if (!path)
	{
		DBG1(DBG_CFG, "coupling file path unspecified");
		destroy(this);
		return NULL;
	}
	this->f = fopen(path, "a+");
	if (!this->f)
	{
		DBG1(DBG_CFG, "opening coupling file '%s' failed: %s",
			 path, strerror(errno));
		destroy(this);
		return NULL;
	}
	setlinebuf(this->f);
	return &this->public;
}
