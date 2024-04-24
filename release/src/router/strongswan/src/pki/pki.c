/*
 * Copyright (C) 2012-2023 Tobias Brunner
 * Copyright (C) 2009 Martin Willi
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

#define _GNU_SOURCE
#include "command.h"
#include "pki.h"

#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include <utils/debug.h>
#include <credentials/sets/mem_cred.h>
#include <credentials/sets/callback_cred.h>

/**
 * Convert a form string to a encoding type
 */
bool get_form(char *form, cred_encoding_type_t *enc, credential_type_t type)
{
	if (streq(form, "der"))
	{
		switch (type)
		{
			case CRED_CERTIFICATE:
				*enc = CERT_ASN1_DER;
				return TRUE;
			case CRED_PRIVATE_KEY:
				*enc = PRIVKEY_ASN1_DER;
				return TRUE;
			case CRED_PUBLIC_KEY:
				/* der encoded keys usually contain the complete
				 * SubjectPublicKeyInfo */
				*enc = PUBKEY_SPKI_ASN1_DER;
				return TRUE;
			default:
				return FALSE;
		}
	}
	else if (streq(form, "pem"))
	{
		switch (type)
		{
			case CRED_CERTIFICATE:
				*enc = CERT_PEM;
				return TRUE;
			case CRED_PRIVATE_KEY:
				*enc = PRIVKEY_PEM;
				return TRUE;
			case CRED_PUBLIC_KEY:
				*enc = PUBKEY_PEM;
				return TRUE;
			default:
				return FALSE;
		}
	}
	else if (streq(form, "pgp"))
	{
		switch (type)
		{
			case CRED_PRIVATE_KEY:
				*enc = PRIVKEY_PGP;
				return TRUE;
			case CRED_PUBLIC_KEY:
				*enc = PUBKEY_PGP;
				return TRUE;
			default:
				return FALSE;
		}
	}
	else if (streq(form, "dnskey"))
	{
		switch (type)
		{
			case CRED_PUBLIC_KEY:
				*enc = PUBKEY_DNSKEY;
				return TRUE;
			default:
				return FALSE;
		}
	}
	else if (streq(form, "sshkey"))
	{
		switch (type)
		{
			case CRED_PUBLIC_KEY:
				*enc = PUBKEY_SSHKEY;
				return TRUE;
			default:
				return FALSE;
		}
	}
	return FALSE;
}

/**
 * Convert a time string to struct tm using strptime format
 */
static bool convert_time(char *str, char *format, struct tm *tm)
{
#ifdef HAVE_STRPTIME

	char *end;

	if (!format)
	{
		format = "%d.%m.%y %T";
	}

	end = strptime(str, format, tm);
	if (end == NULL || *end != '\0')
	{
		return FALSE;
	}
	return TRUE;

#else /* !HAVE_STRPTIME */

	if (format)
	{
		fprintf(stderr, "custom datetime string format not supported\n");
		return FALSE;
	}

	if (sscanf(str, "%d.%d.%d %d:%d:%d",
			   &tm->tm_mday, &tm->tm_mon, &tm->tm_year,
			   &tm->tm_hour, &tm->tm_min, &tm->tm_sec) != 6)
	{
		return FALSE;
	}
	/* strptime() interprets two-digit years > 68 as 19xx, do the same here.
	 * mktime() expects years based on 1900 */
	if (tm->tm_year <= 68)
	{
		tm->tm_year += 100;
	}
	else if (tm->tm_year >= 1900)
	{	/* looks like four digits? */
		tm->tm_year -= 1900;
	}
	/* month is specified from 0-11 */
	tm->tm_mon--;
	/* automatically detect daylight saving time */
	tm->tm_isdst = -1;
	return TRUE;

#endif /* !HAVE_STRPTIME */
}

/**
 * See header
 */
bool calculate_lifetime(char *format, char *nbstr, char *nastr, time_t span,
						time_t *nb, time_t *na)
{
	struct tm tm;
	time_t now;

	now = time(NULL);

	localtime_r(&now, &tm);
	if (nbstr)
	{
		if (!convert_time(nbstr, format, &tm))
		{
			return FALSE;
		}
	}
	*nb = mktime(&tm);
	if (*nb == -1)
	{
		return FALSE;
	}

	localtime_r(&now, &tm);
	if (nastr)
	{
		if (!convert_time(nastr, format, &tm))
		{
			return FALSE;
		}
	}
	*na = mktime(&tm);
	if (*na == -1)
	{
		return FALSE;
	}

	if (!nbstr && nastr)
	{
		*nb = *na - span;
	}
	else if (!nastr)
	{
		*na = *nb + span;
	}
	return TRUE;
}

/**
 * Set output file mode appropriate for credential encoding form on Windows
 */
void set_file_mode(FILE *stream, cred_encoding_type_t enc)
{
#ifdef WIN32
	int fd;

	switch (enc)
	{
		case CERT_PEM:
		case PRIVKEY_PEM:
		case PUBKEY_PEM:
			/* keep default text mode */
			return;
		default:
			/* switch to binary mode */
			break;
	}
	fd = fileno(stream);
	if (fd != -1)
	{
		_setmode(fd, _O_BINARY);
	}
#endif
}

/*
 * Described in header
 */
bool parse_rsa_padding(char *padding, bool *pss)
{
	if (streq(padding, "pss"))
	{
		*pss = TRUE;
	}
	else if (streq(padding, "pkcs1"))
	{
		*pss = FALSE;
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

/**
 * Determine a default hash algorithm for the given key
 */
static hash_algorithm_t get_default_digest(private_key_t *private)
{
	enumerator_t *enumerator;
	signature_params_t *params;
	hash_algorithm_t alg = HASH_UNKNOWN;

	enumerator = signature_schemes_for_key(private->get_type(private),
										   private->get_keysize(private));
	if (enumerator->enumerate(enumerator, &params))
	{
		alg = hasher_from_signature_scheme(params->scheme, params->params);
	}
	enumerator->destroy(enumerator);

	/* default to SHA-256 */
	return alg == HASH_UNKNOWN ? HASH_SHA256 : alg;
}

/*
 * Described in header
 */
signature_params_t *get_signature_scheme(private_key_t *private,
										 hash_algorithm_t digest, bool pss)
{
	signature_params_t *scheme, *selected = NULL;
	enumerator_t *enumerator;

	if (private->supported_signature_schemes)
	{
		enumerator = private->supported_signature_schemes(private);
		while (enumerator->enumerate(enumerator, &scheme))
		{
			if (private->get_type(private) == KEY_RSA &&
				pss != (scheme->scheme == SIGN_RSA_EMSA_PSS))
			{
				continue;
			}
			if (digest == HASH_UNKNOWN ||
				digest == hasher_from_signature_scheme(scheme->scheme,
													   scheme->params))
			{
				selected = signature_params_clone(scheme);
				break;
			}
		}
		enumerator->destroy(enumerator);
		return selected;
	}

	if (digest == HASH_UNKNOWN)
	{
		digest = get_default_digest(private);
	}
	if (private->get_type(private) == KEY_RSA && pss)
	{
		rsa_pss_params_t pss_params = {
			.hash = digest,
			.mgf1_hash = digest,
			.salt_len = RSA_PSS_SALT_LEN_DEFAULT,
		};
		signature_params_t pss_scheme = {
			.scheme = SIGN_RSA_EMSA_PSS,
			.params = &pss_params,
		};
		rsa_pss_params_set_salt_len(&pss_params, 0);
		scheme = signature_params_clone(&pss_scheme);
	}
	else
	{
		INIT(scheme,
			.scheme = signature_scheme_from_oid(
								hasher_signature_algorithm_to_oid(digest,
												private->get_type(private))),
		);
	}
	return scheme;
}

/*
 * Described in header
 */
traffic_selector_t* parse_ts(char *str)
{
	ts_type_t type = TS_IPV4_ADDR_RANGE;
	char *to, from[64];

	if (strchr(str, ':'))
	{
		type = TS_IPV6_ADDR_RANGE;
	}
	to = strchr(str, '-');
	if (to)
	{
		snprintf(from, sizeof(from), "%.*s", (int)(to - str), str);
		to++;
		return traffic_selector_create_from_string(0, type, from, 0, to, 65535);
	}
	return traffic_selector_create_from_cidr(str, 0, 0, 65535);
}

/*
 * Described in header
 */
bool allocate_serial(size_t len, chunk_t *serial)
{
	rng_t *rng;

	if (!len)
	{
		len = 1;
	}
	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng)
	{
		fprintf(stderr, "no random number generator found\n");
		return FALSE;
	}
	if (!rng_allocate_bytes_not_zero(rng, len, serial, FALSE))
	{
		rng->destroy(rng);
		return FALSE;
	}
	/* ensure the serial is positive but doesn't start with 0 */
	while (!(serial->ptr[0] &= 0x7F))
	{
		if (!rng->get_bytes(rng, 1, serial->ptr))
		{
			rng->destroy(rng);
			return FALSE;
		}
	}
	rng->destroy(rng);
	return TRUE;
}

/**
 * Callback credential set pki uses
 */
static callback_cred_t *cb_set;

/**
 * Credential set to cache entered secrets
 */
static mem_cred_t *cb_creds;

static shared_key_type_t prompted;

/**
 * Callback function to receive credentials
 */
static shared_key_t* cb(void *data, shared_key_type_t type,
						identification_t *me, identification_t *other,
						id_match_t *match_me, id_match_t *match_other)
{
	char buf[64], *label, *secret = NULL;
	shared_key_t *shared;

	if (prompted == type)
	{
		return NULL;
	}
	switch (type)
	{
		case SHARED_PIN:
			label = "Smartcard PIN";
			break;
		case SHARED_PRIVATE_KEY_PASS:
			label = "Private key passphrase";
			break;
		default:
			return NULL;
	}
	snprintf(buf, sizeof(buf), "%s: ", label);
#ifdef HAVE_GETPASS
	secret = getpass(buf);
#endif
	if (secret && strlen(secret))
	{
		prompted = type;
		if (match_me)
		{
			*match_me = ID_MATCH_PERFECT;
		}
		if (match_other)
		{
			*match_other = ID_MATCH_NONE;
		}
		shared = shared_key_create(type, chunk_clone(chunk_from_str(secret)));
		memwipe(secret, strlen(secret));
		/* cache password in case it is required more than once */
		cb_creds->add_shared(cb_creds, shared, NULL);
		return shared->get_ref(shared);
	}
	return NULL;
}

/**
 * Register PIN/Passphrase callback function
 */
static void add_callback()
{
	cb_set = callback_cred_create_shared(cb, NULL);
	lib->credmgr->add_set(lib->credmgr, &cb_set->set);
	cb_creds = mem_cred_create();
	lib->credmgr->add_set(lib->credmgr, &cb_creds->set);
}

/**
 * Unregister PIN/Passphrase callback function
 */
static void remove_callback()
{
	lib->credmgr->remove_set(lib->credmgr, &cb_creds->set);
	cb_creds->destroy(cb_creds);
	lib->credmgr->remove_set(lib->credmgr, &cb_set->set);
	cb_set->destroy(cb_set);
}

/**
 * Library initialization and operation parsing
 */
int main(int argc, char *argv[])
{
	char *plugins;

	atexit(library_deinit);
	if (!library_init(NULL, "pki"))
	{
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}
	if (lib->integrity &&
		!lib->integrity->check_file(lib->integrity, "pki", argv[0]))
	{
		fprintf(stderr, "integrity check of pki failed\n");
		exit(SS_RC_DAEMON_INTEGRITY);
	}
	plugins = getenv("PKI_PLUGINS");
	if (!plugins)
	{
		plugins = lib->settings->get_str(lib->settings, "pki.load", PLUGINS);
	}
	if (!lib->plugins->load(lib->plugins, plugins))
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}

	add_callback();
	atexit(remove_callback);
	return command_dispatch(argc, argv);
}
