/*
 * Copyright (C) 2008-2017 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#ifdef HAVE_GLOB_H
#include <glob.h>
#endif

#include "stroke_cred.h"

#include <credentials/certificates/x509.h>
#include <credentials/certificates/crl.h>
#include <credentials/certificates/ac.h>
#include <credentials/containers/pkcs12.h>
#include <credentials/sets/mem_cred.h>
#include <credentials/sets/callback_cred.h>
#include <collections/linked_list.h>
#include <utils/lexparser.h>
#include <threading/rwlock.h>
#include <daemon.h>

/* configuration directories and files */
#define CONFIG_DIR IPSEC_CONFDIR
#define IPSEC_D_DIR CONFIG_DIR "/ipsec.d"
#define PRIVATE_KEY_DIR IPSEC_D_DIR "/private"
#define CERTIFICATE_DIR IPSEC_D_DIR "/certs"
#define CA_CERTIFICATE_DIR IPSEC_D_DIR "/cacerts"
#define AA_CERTIFICATE_DIR IPSEC_D_DIR "/aacerts"
#define ATTR_CERTIFICATE_DIR IPSEC_D_DIR "/acerts"
#define OCSP_CERTIFICATE_DIR IPSEC_D_DIR "/ocspcerts"
#define CRL_DIR IPSEC_D_DIR "/crls"
#define SECRETS_FILE CONFIG_DIR "/ipsec.secrets"

#define MAX_SECRETS_RECURSION 10

typedef struct private_stroke_cred_t private_stroke_cred_t;

/**
 * private data of stroke_cred
 */
struct private_stroke_cred_t {

	/**
	 * public functions
	 */
	stroke_cred_t public;

	/**
	 * secrets file with credential information
	 */
	char *secrets_file;

	/**
	 * credentials: end entity certs, attribute certs, CRLs, etc.
	 */
	mem_cred_t *creds;

	/**
	 * Attribute Authority certificates
	 */
	mem_cred_t *aacerts;

	/**
	 * ignore missing CA basic constraint (i.e. treat all certificates in
	 * ipsec.conf ca sections and ipsec.d/cacerts as CA certificates)
	 */
	bool force_ca_cert;

	/**
	 * cache CRLs to disk?
	 */
	bool cachecrl;

	/**
	 * CA certificate store
	 */
	stroke_ca_t *ca;
};

/** Length of smartcard specifier parts (module, keyid) */
#define SC_PART_LEN 128

/**
 * Kind of smartcard specifier token
 */
typedef enum {
	SC_FORMAT_SLOT_MODULE_KEYID,
	SC_FORMAT_SLOT_KEYID,
	SC_FORMAT_KEYID,
	SC_FORMAT_INVALID,
} smartcard_format_t;

/**
 * Parse a smartcard specifier token
 */
static smartcard_format_t parse_smartcard(char *smartcard, u_int *slot,
										  char *module, char *keyid)
{
	/* The token has one of the following three formats:
	 * - %smartcard<slot>@<module>:<keyid>
	 * - %smartcard<slot>:<keyid>
	 * - %smartcard:<keyid>
	 */
	char buf[2 * SC_PART_LEN], *pos;

	if (sscanf(smartcard, "%%smartcard%u@%255s", slot, buf) == 2)
	{
		pos = strchr(buf, ':');
		if (!pos)
		{
			return SC_FORMAT_INVALID;
		}
		*pos++ = '\0';
		snprintf(module, SC_PART_LEN, "%s", buf);
		snprintf(keyid, SC_PART_LEN, "%s", pos);
		return SC_FORMAT_SLOT_MODULE_KEYID;
	}
	if (sscanf(smartcard, "%%smartcard%u:%127s", slot, keyid) == 2)
	{
		return SC_FORMAT_SLOT_KEYID;
	}
	if (sscanf(smartcard, "%%smartcard:%127s", keyid) == 1)
	{
		return SC_FORMAT_KEYID;
	}
	return SC_FORMAT_INVALID;
}

/**
 * Load a credential from a smartcard
 */
static certificate_t *load_from_smartcard(smartcard_format_t format,
										  u_int slot, char *module, char *keyid,
										  credential_type_t type, int subtype)
{
	chunk_t chunk;
	void *cred;

	chunk = chunk_from_hex(chunk_create(keyid, strlen(keyid)), NULL);
	switch (format)
	{
		case SC_FORMAT_SLOT_MODULE_KEYID:
			cred = lib->creds->create(lib->creds, type, subtype,
							BUILD_PKCS11_SLOT, slot,
							BUILD_PKCS11_MODULE, module,
							BUILD_PKCS11_KEYID, chunk, BUILD_END);
			break;
		case SC_FORMAT_SLOT_KEYID:
			cred = lib->creds->create(lib->creds, type, subtype,
							BUILD_PKCS11_SLOT, slot,
							BUILD_PKCS11_KEYID, chunk, BUILD_END);
			break;
		case SC_FORMAT_KEYID:
			cred = lib->creds->create(lib->creds, type, subtype,
							BUILD_PKCS11_KEYID, chunk, BUILD_END);
			break;
		default:
			cred = NULL;
			break;
	}
	free(chunk.ptr);

	return cred;
}

METHOD(stroke_cred_t, load_peer, certificate_t*,
	private_stroke_cred_t *this, char *filename)
{
	certificate_t *cert = NULL;
	char path[PATH_MAX];

	if (strpfx(filename, "%smartcard"))
	{
		smartcard_format_t format;
		char module[SC_PART_LEN], keyid[SC_PART_LEN];
		u_int slot;

		format = parse_smartcard(filename, &slot, module, keyid);
		if (format != SC_FORMAT_INVALID)
		{
			cert = (certificate_t*)load_from_smartcard(format,
							slot, module, keyid, CRED_CERTIFICATE, CERT_X509);
		}
	}
	else
	{
		if (*filename == '/')
		{
			snprintf(path, sizeof(path), "%s", filename);
		}
		else
		{
			snprintf(path, sizeof(path), "%s/%s", CERTIFICATE_DIR, filename);
		}

		cert = lib->creds->create(lib->creds,
								  CRED_CERTIFICATE, CERT_ANY,
								  BUILD_FROM_FILE, path,
								  BUILD_END);
	}
	if (cert)
	{
		cert = this->creds->add_cert_ref(this->creds, TRUE, cert);
		DBG1(DBG_CFG, "  loaded certificate \"%Y\" from '%s'",
					  cert->get_subject(cert), filename);
		return cert;
	}
	DBG1(DBG_CFG, "  loading certificate from '%s' failed", filename);
	return NULL;
}

METHOD(stroke_cred_t, load_pubkey, certificate_t*,
	private_stroke_cred_t *this, char *filename, identification_t *identity)
{
	certificate_t *cert;
	public_key_t *key;
	char path[PATH_MAX];
	builder_part_t build_part;
	key_type_t type = KEY_ANY;

	if (streq(filename, "%dns"))
	{
		return NULL;
	}
	if (strncaseeq(filename, "dns:", 4))
	{	/* RFC 3110 format */
		build_part = BUILD_BLOB_DNSKEY;
		/* not a complete RR, only RSA supported */
		type = KEY_RSA;
		filename += 4;
	}
	else if (strncaseeq(filename, "ssh:", 4))
	{	/* SSH key */
		build_part = BUILD_BLOB_SSHKEY;
		filename += 4;
	}
	else
	{	/* try PKCS#1 by default */
		build_part = BUILD_BLOB_ASN1_DER;
	}
	if (strncaseeq(filename, "0x", 2) || strncaseeq(filename, "0s", 2))
	{
		chunk_t printable_key, raw_key;

		printable_key = chunk_create(filename + 2, strlen(filename) - 2);
		raw_key = strncaseeq(filename, "0x", 2) ?
								 chunk_from_hex(printable_key, NULL) :
								 chunk_from_base64(printable_key, NULL);
		key = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, type,
								 build_part, raw_key, BUILD_END);
		chunk_free(&raw_key);
		if (key)
		{
			cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
									  CERT_TRUSTED_PUBKEY,
									  BUILD_PUBLIC_KEY, key,
									  BUILD_SUBJECT, identity,
									  BUILD_END);
			type = key->get_type(key);
			key->destroy(key);
			if (cert)
			{
				cert = this->creds->add_cert_ref(this->creds, TRUE, cert);
				DBG1(DBG_CFG, "  loaded %N public key for \"%Y\"",
					 key_type_names, type, identity);
				return cert;
			}
		}
		DBG1(DBG_CFG, "  loading public key for \"%Y\" failed", identity);
	}
	else
	{
		if (*filename == '/')
		{
			snprintf(path, sizeof(path), "%s", filename);
		}
		else
		{
			snprintf(path, sizeof(path), "%s/%s", CERTIFICATE_DIR, filename);
		}

		cert = lib->creds->create(lib->creds,
								  CRED_CERTIFICATE, CERT_TRUSTED_PUBKEY,
								  BUILD_FROM_FILE, path,
								  BUILD_SUBJECT, identity,
								  BUILD_END);
		if (cert)
		{
			cert = this->creds->add_cert_ref(this->creds, TRUE, cert);
			key = cert->get_public_key(cert);
			type = key->get_type(key);
			key->destroy(key);
			DBG1(DBG_CFG, "  loaded %N public key for \"%Y\" from '%s'",
				 key_type_names, type, identity, filename);
			return cert;
		}
		DBG1(DBG_CFG, "  loading public key for \"%Y\" from '%s' failed",
			 identity, filename);
	}
	return NULL;
}

/**
 * Load a CA certificate, optionally force it to be one
 */
static certificate_t *load_ca_cert(char *filename, bool force_ca_cert)
{
	certificate_t *cert = NULL;
	char path[PATH_MAX];

	if (strpfx(filename, "%smartcard"))
	{
		smartcard_format_t format;
		char module[SC_PART_LEN], keyid[SC_PART_LEN];
		u_int slot;

		format = parse_smartcard(filename, &slot, module, keyid);
		if (format != SC_FORMAT_INVALID)
		{
			cert = (certificate_t*)load_from_smartcard(format,
							slot, module, keyid, CRED_CERTIFICATE, CERT_X509);
		}
	}
	else
	{
		if (*filename == '/')
		{
			snprintf(path, sizeof(path), "%s", filename);
		}
		else
		{
			snprintf(path, sizeof(path), "%s/%s", CA_CERTIFICATE_DIR, filename);
		}

		if (force_ca_cert)
		{	/* we treat this certificate as a CA certificate even if it has no
			 * CA basic constraint */
			cert = lib->creds->create(lib->creds,
								  CRED_CERTIFICATE, CERT_X509,
								  BUILD_FROM_FILE, path, BUILD_X509_FLAG, X509_CA,
								  BUILD_END);
		}
		else
		{
			cert = lib->creds->create(lib->creds,
								  CRED_CERTIFICATE, CERT_X509,
								  BUILD_FROM_FILE, path,
								  BUILD_END);
		}
	}
	if (cert)
	{
		x509_t *x509 = (x509_t*)cert;

		if (!(x509->get_flags(x509) & X509_CA))
		{
			DBG1(DBG_CFG, "  ca certificate \"%Y\" lacks ca basic constraint, "
				 "discarded", cert->get_subject(cert));
			cert->destroy(cert);
			return NULL;
		}
		DBG1(DBG_CFG, "  loaded ca certificate \"%Y\" from '%s'",
			 cert->get_subject(cert), filename);
		return cert;
	}
	return NULL;
}

/**
 * Used by stroke_ca.c
 */
certificate_t *stroke_load_ca_cert(char *filename)
{
	bool force_ca_cert;

	force_ca_cert = lib->settings->get_bool(lib->settings,
						"%s.plugins.stroke.ignore_missing_ca_basic_constraint",
						FALSE, lib->ns);
	return load_ca_cert(filename, force_ca_cert);
}

/**
 * Load a CA certificate from disk
 */
static void load_x509_ca(private_stroke_cred_t *this, char *file,
						 mem_cred_t *creds)
{
	certificate_t *cert;

	cert = load_ca_cert(file, this->force_ca_cert);
	if (cert)
	{
		cert = this->ca->get_cert_ref(this->ca, cert);
		creds->add_cert(creds, TRUE, cert);
	}
	else
	{
		DBG1(DBG_CFG, "  loading ca certificate from '%s' failed", file);
	}
}

/**
 * Load AA certificate with flags from disk
 */
static void load_x509_aa(private_stroke_cred_t *this,char *file,
						 mem_cred_t *creds)
{
	certificate_t *cert;

	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
							  BUILD_FROM_FILE, file,
							  BUILD_X509_FLAG, X509_AA, BUILD_END);
	if (cert)
	{
		DBG1(DBG_CFG, "  loaded AA certificate \"%Y\" from '%s'",
			 cert->get_subject(cert), file);
		creds->add_cert(creds, TRUE, cert);
	}
	else
	{
		DBG1(DBG_CFG, "  loading AA certificate from '%s' failed", file);
	}
}

/**
 * Load a certificate with flags from disk
 */
static void load_x509(private_stroke_cred_t *this, char *file, x509_flag_t flag,
					  mem_cred_t *creds)
{
	certificate_t *cert;

	/* for all other flags, we add them to the certificate. */
	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
							  BUILD_FROM_FILE, file,
							  BUILD_X509_FLAG, flag, BUILD_END);
	if (cert)
	{
		DBG1(DBG_CFG, "  loaded certificate \"%Y\" from '%s'",
			 cert->get_subject(cert), file);
		creds->add_cert(creds, TRUE, cert);
	}
	else
	{
		DBG1(DBG_CFG, "  loading certificate from '%s' failed", file);
	}
}

/**
 * Load a CRL from a file
 */
static void load_x509_crl(private_stroke_cred_t *this, char *file,
						  mem_cred_t *creds)
{
	certificate_t *cert;

	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509_CRL,
							  BUILD_FROM_FILE, file, BUILD_END);
	if (cert)
	{
		DBG1(DBG_CFG, "  loaded crl from '%s'",  file);
		creds->add_crl(creds, (crl_t*)cert);
	}
	else
	{
		DBG1(DBG_CFG, "  loading crl from '%s' failed", file);
	}
}

/**
 * Load an attribute certificate from a file
 */
static void load_x509_ac(private_stroke_cred_t *this, char *file,
						 mem_cred_t *creds)
{
	certificate_t *cert;

	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509_AC,
							  BUILD_FROM_FILE, file, BUILD_END);
	if (cert)
	{
		DBG1(DBG_CFG, "  loaded attribute certificate from '%s'", file);
		creds->add_cert(creds, FALSE, cert);
	}
	else
	{
		DBG1(DBG_CFG, "  loading attribute certificate from '%s' failed", file);
	}
}

/**
 * load trusted certificates from a directory
 */
static void load_certdir(private_stroke_cred_t *this, char *path,
						 certificate_type_t type, x509_flag_t flag,
						 mem_cred_t *creds)
{
	enumerator_t *enumerator;
	struct stat st;
	char *file;

	enumerator = enumerator_create_directory(path);
	if (enumerator)
	{
		while (enumerator->enumerate(enumerator, NULL, &file, &st))
		{
			if (!S_ISREG(st.st_mode))
			{
				/* skip special file */
				continue;
			}
			switch (type)
			{
				case CERT_X509:
					if (flag & X509_CA)
					{
						load_x509_ca(this, file, creds);
					}
					else if (flag & X509_AA)
					{
						load_x509_aa(this, file, creds);
					}
					else
					{
						load_x509(this, file, flag, creds);
					}
					break;
				case CERT_X509_CRL:
					load_x509_crl(this, file, creds);
					break;
				case CERT_X509_AC:
					load_x509_ac(this, file, creds);
					break;
				default:
					break;
			}
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		DBG1(DBG_CFG, "  reading directory failed");
	}
}

METHOD(credential_set_t, cache_cert, void,
	private_stroke_cred_t *this, certificate_t *cert)
{
	if (cert->get_type(cert) == CERT_X509_CRL && this->cachecrl)
	{
		/* CRLs get written to /etc/ipsec.d/crls/<authkeyId>.crl */
		crl_t *crl = (crl_t*)cert;

		cert->get_ref(cert);
		if (this->creds->add_crl(this->creds, crl))
		{
			char buf[BUF_LEN];
			chunk_t chunk, hex;
			bool is_delta_crl;

			is_delta_crl = crl->is_delta_crl(crl, NULL);

			chunk = crl->get_authKeyIdentifier(crl);
			hex = chunk_to_hex(chunk, NULL, FALSE);
			snprintf(buf, sizeof(buf), "%s/%s%s.crl", CRL_DIR, hex.ptr,
										is_delta_crl ? "_delta" : "");
			free(hex.ptr);

			if (cert->get_encoding(cert, CERT_ASN1_DER, &chunk))
			{
				if (chunk_write(chunk, buf, 022, TRUE))
				{
					DBG1(DBG_CFG, "  written crl file '%s' (%d bytes)",
						 buf, chunk.len);
				}
				else
				{
					DBG1(DBG_CFG, "  writing crl file '%s' failed: %s",
						 buf, strerror(errno));
				}
				free(chunk.ptr);
			}
		}
	}
}

METHOD(stroke_cred_t, cachecrl, void,
	private_stroke_cred_t *this, bool enabled)
{
	DBG1(DBG_CFG, "crl caching to %s %s",
		 CRL_DIR, enabled ? "enabled" : "disabled");
	this->cachecrl = enabled;
}


/**
 * Convert a string of characters into a binary secret
 * A string between single or double quotes is treated as ASCII characters
 * A string prepended by 0x is treated as HEX and prepended by 0s as Base64
 */
static err_t extract_secret(chunk_t *secret, chunk_t *line)
{
	chunk_t raw_secret;
	char delimiter = ' ';
	bool quotes = FALSE;

	if (!eat_whitespace(line))
	{
		return "missing secret";
	}

	if (*line->ptr == '\'' || *line->ptr == '"')
	{
		quotes = TRUE;
		delimiter = *line->ptr;
		line->ptr++;  line->len--;
	}

	if (!extract_token(&raw_secret, delimiter, line))
	{
		if (delimiter == ' ')
		{
			raw_secret = *line;
		}
		else
		{
			return "missing second delimiter";
		}
	}

	if (quotes)
	{
		/* treat as an ASCII string */
		*secret = chunk_clone(raw_secret);
		return NULL;
	}
	/* treat 0x as hex, 0s as base64 */
	if (raw_secret.len > 2)
	{
		if (strncasecmp("0x", raw_secret.ptr, 2) == 0)
		{
			*secret = chunk_from_hex(chunk_skip(raw_secret, 2), NULL);
			return NULL;
		}
		if (strncasecmp("0s", raw_secret.ptr, 2) == 0)
		{
			*secret = chunk_from_base64(chunk_skip(raw_secret, 2), NULL);
			return NULL;
		}
	}
	*secret = chunk_clone(raw_secret);
	return NULL;
}

/**
 * Data for passphrase callback
 */
typedef struct {
	/** cached passphrases */
	mem_cred_t *cache;
	/** socket we use for prompting */
	FILE *prompt;
	/** type of secret to unlock */
	int type;
	/** private key file */
	char *path;
	/** number of tries */
	int try;
} passphrase_cb_data_t;

/**
 * Callback function to receive passphrases
 */
static shared_key_t* passphrase_cb(passphrase_cb_data_t *data,
								   shared_key_type_t type, identification_t *me,
								   identification_t *other, id_match_t *match_me,
								   id_match_t *match_other)
{
	static const int max_tries = 3;
	shared_key_t *shared;
	chunk_t secret;
	char buf[256];

	if (type != SHARED_ANY && type != SHARED_PRIVATE_KEY_PASS)
	{
		return NULL;
	}

	data->try++;
	if (data->try > max_tries + 1)
	{	/* another builder might call this after we gave up, fail silently */
		return NULL;
	}
	if (data->try > max_tries)
	{
		fprintf(data->prompt, "Passphrase invalid, giving up.\n");
		return NULL;
	}
	if (data->try > 1)
	{
		fprintf(data->prompt, "Passphrase invalid!\n");
	}
	fprintf(data->prompt, "%s '%s' is encrypted.\n",
			data->type == CRED_PRIVATE_KEY ? "Private key" : "PKCS#12 file",
			data->path);
	fprintf(data->prompt, "Passphrase:\n");
	if (fgets(buf, sizeof(buf), data->prompt))
	{
		secret = chunk_create(buf, strlen(buf));
		if (secret.len > 1)
		{	/* trim appended \n */
			secret.len--;
			if (match_me)
			{
				*match_me = ID_MATCH_PERFECT;
			}
			if (match_other)
			{
				*match_other = ID_MATCH_NONE;
			}
			shared = shared_key_create(SHARED_PRIVATE_KEY_PASS,
									   chunk_clone(secret));
			data->cache->add_shared(data->cache, shared->get_ref(shared), NULL);
			return shared;
		}
	}
	return NULL;
}

/**
 * Data for PIN callback
 */
typedef struct {
	/** socket we use for prompting */
	FILE *prompt;
	/** card label */
	char *card;
	/** card keyid */
	chunk_t keyid;
	/** number of tries */
	int try;
	/** provided PIN */
	shared_key_t *shared;
} pin_cb_data_t;

/**
 * Callback function to receive PINs
 */
static shared_key_t* pin_cb(pin_cb_data_t *data, shared_key_type_t type,
							identification_t *me, identification_t *other,
							id_match_t *match_me, id_match_t *match_other)
{
	chunk_t secret;
	char buf[256];

	if (type != SHARED_ANY && type != SHARED_PIN)
	{
		return NULL;
	}

	if (!me || !chunk_equals(me->get_encoding(me), data->keyid))
	{
		return NULL;
	}

	data->try++;
	if (data->try > 1)
	{
		fprintf(data->prompt, "PIN invalid, aborting.\n");
		return NULL;
	}
	fprintf(data->prompt, "Login to '%s' required\n", data->card);
	fprintf(data->prompt, "PIN:\n");
	if (fgets(buf, sizeof(buf), data->prompt))
	{
		secret = chunk_create(buf, strlen(buf));
		if (secret.len > 1)
		{	/* trim appended \n */
			secret.len--;
			if (match_me)
			{
				*match_me = ID_MATCH_PERFECT;
			}
			if (match_other)
			{
				*match_other = ID_MATCH_NONE;
			}
			DESTROY_IF(data->shared);
			data->shared = shared_key_create(SHARED_PIN, chunk_clone(secret));
			return data->shared->get_ref(data->shared);
		}
	}
	return NULL;
}

/**
 * Load a smartcard with a PIN
 */
static bool load_pin(mem_cred_t *secrets, chunk_t line, int line_nr,
					 FILE *prompt)
{
	chunk_t sc = chunk_empty, secret = chunk_empty;
	char smartcard[BUF_LEN], keyid[SC_PART_LEN], module[SC_PART_LEN];
	private_key_t *key = NULL;
	u_int slot;
	chunk_t chunk;
	shared_key_t *shared = NULL;
	identification_t *id;
	mem_cred_t *mem = NULL;
	callback_cred_t *cb = NULL;
	pin_cb_data_t pin_data;
	smartcard_format_t format;

	err_t ugh = extract_value(&sc, &line);

	if (ugh != NULL)
	{
		DBG1(DBG_CFG, "line %d: %s", line_nr, ugh);
		return FALSE;
	}
	if (sc.len == 0)
	{
		DBG1(DBG_CFG, "line %d: expected %%smartcard specifier", line_nr);
		return FALSE;
	}
	snprintf(smartcard, sizeof(smartcard), "%.*s", (int)sc.len, sc.ptr);
	smartcard[sizeof(smartcard) - 1] = '\0';

	format = parse_smartcard(smartcard, &slot, module, keyid);
	if (format == SC_FORMAT_INVALID)
	{
		DBG1(DBG_CFG, "line %d: the given %%smartcard specifier is not"
				" supported or invalid", line_nr);
		return FALSE;
	}

	if (!eat_whitespace(&line))
	{
		DBG1(DBG_CFG, "line %d: expected PIN", line_nr);
		return FALSE;
	}
	ugh = extract_secret(&secret, &line);
	if (ugh != NULL)
	{
		DBG1(DBG_CFG, "line %d: malformed PIN: %s", line_nr, ugh);
		return FALSE;
	}

	chunk = chunk_from_hex(chunk_create(keyid, strlen(keyid)), NULL);
	if (secret.len == 7 && strpfx(secret.ptr, "%prompt"))
	{
		free(secret.ptr);
		if (!prompt)
		{	/* no IO channel to prompt, skip */
			chunk_clear(&chunk);
			return TRUE;
		}
		/* use callback credential set to prompt for the pin */
		pin_data = (pin_cb_data_t){
			.prompt = prompt,
			.card = smartcard,
			.keyid = chunk,
		};
		cb = callback_cred_create_shared((void*)pin_cb, &pin_data);
		lib->credmgr->add_local_set(lib->credmgr, &cb->set, FALSE);
	}
	else
	{
		/* provide our pin in a temporary credential set */
		shared = shared_key_create(SHARED_PIN, secret);
		id = identification_create_from_encoding(ID_KEY_ID, chunk);
		mem = mem_cred_create();
		mem->add_shared(mem, shared->get_ref(shared), id, NULL);
		lib->credmgr->add_local_set(lib->credmgr, &mem->set, FALSE);
	}

	/* unlock: smartcard needs the pin and potentially calls public set */
	key = (private_key_t*)load_from_smartcard(format, slot, module, keyid,
											  CRED_PRIVATE_KEY, KEY_ANY);

	if (key)
	{
		DBG1(DBG_CFG, "  loaded private key from %.*s", (int)sc.len, sc.ptr);
		secrets->add_key(secrets, key);
	}
	if (mem)
	{
		if (!key)
		{
			shared->destroy(shared);
			shared = NULL;
		}
		lib->credmgr->remove_local_set(lib->credmgr, &mem->set);
		mem->destroy(mem);
	}
	if (cb)
	{
		if (key)
		{
			shared = pin_data.shared;
		}
		else
		{
			DESTROY_IF(pin_data.shared);
		}
		lib->credmgr->remove_local_set(lib->credmgr, &cb->set);
		cb->destroy(cb);
	}
	if (shared)
	{
		id = identification_create_from_encoding(ID_KEY_ID, chunk);
		secrets->add_shared(secrets, shared, id, NULL);
	}
	chunk_clear(&chunk);
	return TRUE;
}

/**
 * Load a private key or PKCS#12 container from a file
 */
static bool load_from_file(chunk_t line, int line_nr, FILE *prompt,
						   char *path, int type, int subtype,
						   void **result)
{
	chunk_t filename;
	chunk_t secret = chunk_empty;

	err_t ugh = extract_value(&filename, &line);

	if (ugh != NULL)
	{
		DBG1(DBG_CFG, "line %d: %s", line_nr, ugh);
		return FALSE;
	}
	if (filename.len == 0)
	{
		DBG1(DBG_CFG, "line %d: empty filename", line_nr);
		return FALSE;
	}
	if (*filename.ptr == '/')
	{
		/* absolute path name */
		snprintf(path, PATH_MAX, "%.*s", (int)filename.len, filename.ptr);
	}
	else
	{
		/* relative path name */
		snprintf(path, PATH_MAX, "%s/%.*s", PRIVATE_KEY_DIR,
				 (int)filename.len, filename.ptr);
	}

	/* check for optional passphrase */
	if (eat_whitespace(&line))
	{
		ugh = extract_secret(&secret, &line);
		if (ugh != NULL)
		{
			DBG1(DBG_CFG, "line %d: malformed passphrase: %s", line_nr, ugh);
			return FALSE;
		}
	}
	if (secret.len == 7 && strpfx(secret.ptr, "%prompt"))
	{
		callback_cred_t *cb;
		passphrase_cb_data_t pp_data = {
			.prompt = prompt,
			.type = type,
			.path = path,
			.try = 0,
		};

		free(secret.ptr);
		if (!prompt)
		{
			*result = NULL;
			return TRUE;
		}
		/* add cache first so if valid passphrases are needed multiple times
		 * the callback is not called anymore */
		pp_data.cache = mem_cred_create();
		lib->credmgr->add_local_set(lib->credmgr, &pp_data.cache->set, FALSE);
		/* use callback credential set to prompt for the passphrase */
		cb = callback_cred_create_shared((void*)passphrase_cb, &pp_data);
		lib->credmgr->add_local_set(lib->credmgr, &cb->set, FALSE);

		*result = lib->creds->create(lib->creds, type, subtype,
									 BUILD_FROM_FILE, path, BUILD_END);

		lib->credmgr->remove_local_set(lib->credmgr, &cb->set);
		cb->destroy(cb);
		lib->credmgr->remove_local_set(lib->credmgr, &pp_data.cache->set);
		pp_data.cache->destroy(pp_data.cache);
	}
	else
	{
		mem_cred_t *mem = NULL;
		shared_key_t *shared;

		/* provide our pin in a temporary credential set */
		shared = shared_key_create(SHARED_PRIVATE_KEY_PASS, secret);
		mem = mem_cred_create();
		mem->add_shared(mem, shared, NULL);
		if (eat_whitespace(&line))
		{	/* if there is a second passphrase add that too, could be needed for
			 * PKCS#12 files using different passwords for MAC and encryption */
			ugh = extract_secret(&secret, &line);
			if (ugh != NULL)
			{
				DBG1(DBG_CFG, "line %d: malformed passphrase: %s", line_nr, ugh);
				mem->destroy(mem);
				return FALSE;
			}
			shared = shared_key_create(SHARED_PRIVATE_KEY_PASS, secret);
			mem->add_shared(mem, shared, NULL);
		}
		lib->credmgr->add_local_set(lib->credmgr, &mem->set, FALSE);

		*result = lib->creds->create(lib->creds, type, subtype,
									 BUILD_FROM_FILE, path, BUILD_END);

		lib->credmgr->remove_local_set(lib->credmgr, &mem->set);
		mem->destroy(mem);
	}
	return TRUE;
}

/**
 * Load a private key
 */
static bool load_private(mem_cred_t *secrets, chunk_t line, int line_nr,
						 FILE *prompt, key_type_t key_type)
{
	char path[PATH_MAX];
	private_key_t *key;

	if (!load_from_file(line, line_nr, prompt, path, CRED_PRIVATE_KEY,
						key_type, (void**)&key))
	{
		return FALSE;
	}
	if (key)
	{
		DBG1(DBG_CFG, "  loaded %N private key from '%s'",
			 key_type_names, key->get_type(key), path);
		secrets->add_key(secrets, key);
	}
	else
	{
		DBG1(DBG_CFG, "  loading private key from '%s' failed", path);
	}
	return TRUE;
}

/**
 * Load a PKCS#12 container
 */
static bool load_pkcs12(private_stroke_cred_t *this, mem_cred_t *secrets,
						chunk_t line, int line_nr, FILE *prompt)
{
	enumerator_t *enumerator;
	char path[PATH_MAX];
	certificate_t *cert;
	private_key_t *key;
	pkcs12_t *pkcs12;

	if (!load_from_file(line, line_nr, prompt, path, CRED_CONTAINER,
						CONTAINER_PKCS12, (void**)&pkcs12))
	{
		return FALSE;
	}
	if (!pkcs12)
	{
		DBG1(DBG_CFG, "  loading credentials from '%s' failed", path);
		return TRUE;
	}
	enumerator = pkcs12->create_cert_enumerator(pkcs12);
	while (enumerator->enumerate(enumerator, &cert))
	{
		x509_t *x509 = (x509_t*)cert;

		if (x509->get_flags(x509) & X509_CA)
		{
			DBG1(DBG_CFG, "  loaded ca certificate \"%Y\" from '%s'",
				 cert->get_subject(cert), path);
		}
		else
		{
			DBG1(DBG_CFG, "  loaded certificate \"%Y\" from '%s'",
				 cert->get_subject(cert), path);
		}
		this->creds->add_cert(this->creds, TRUE, cert->get_ref(cert));
	}
	enumerator->destroy(enumerator);
	enumerator = pkcs12->create_key_enumerator(pkcs12);
	while (enumerator->enumerate(enumerator, &key))
	{
		DBG1(DBG_CFG, "  loaded %N private key from '%s'",
			 key_type_names, key->get_type(key), path);
		secrets->add_key(secrets, key->get_ref(key));
	}
	enumerator->destroy(enumerator);
	pkcs12->container.destroy(&pkcs12->container);
	return TRUE;
}

/**
 * Load a shared key
 */
static bool load_shared(mem_cred_t *secrets, chunk_t line, int line_nr,
						shared_key_type_t type, chunk_t ids)
{
	shared_key_t *shared_key;
	linked_list_t *owners;
	chunk_t secret = chunk_empty;

	err_t ugh = extract_secret(&secret, &line);
	if (ugh != NULL)
	{
		DBG1(DBG_CFG, "line %d: malformed secret: %s", line_nr, ugh);
		return FALSE;
	}
	shared_key = shared_key_create(type, secret);
	DBG1(DBG_CFG, "  loaded %N secret for %s", shared_key_type_names, type,
		 ids.len > 0 ? (char*)ids.ptr : "%any");
	DBG4(DBG_CFG, "  secret: %#B", &secret);

	owners = linked_list_create();
	while (ids.len > 0)
	{
		chunk_t id;

		ugh = extract_value(&id, &ids);
		if (ugh != NULL)
		{
			DBG1(DBG_CFG, "line %d: %s", line_nr, ugh);
			shared_key->destroy(shared_key);
			owners->destroy_offset(owners, offsetof(identification_t, destroy));
			return FALSE;
		}
		if (id.len == 0)
		{
			continue;
		}

		/* NULL terminate the ID string */
		*(id.ptr + id.len) = '\0';
		owners->insert_last(owners, identification_create_from_string(id.ptr));
	}
	if (!owners->get_count(owners))
	{
		owners->insert_last(owners,
					identification_create_from_encoding(ID_ANY, chunk_empty));
	}
	secrets->add_shared_list(secrets, shared_key, owners);
	return TRUE;
}

/**
 * reload ipsec.secrets
 */
static void load_secrets(private_stroke_cred_t *this, mem_cred_t *secrets,
						 char *file, int level, FILE *prompt)
{
	int line_nr = 0;
	chunk_t *src, line;

	DBG1(DBG_CFG, "loading secrets from '%s'", file);
	src = chunk_map(file, FALSE);
	if (!src)
	{
		DBG1(DBG_CFG, "opening secrets file '%s' failed: %s", file,
			 strerror(errno));
		return;
	}

	if (!secrets)
	{
		secrets = mem_cred_create();
	}

	while (fetchline(src, &line))
	{
		chunk_t ids, token;
		key_type_t key_type;
		shared_key_type_t type;

		line_nr++;

		if (!eat_whitespace(&line))
		{
			continue;
		}
		if (line.len > strlen("include ") && strpfx(line.ptr, "include "))
		{
			char **expanded, *dir, pattern[PATH_MAX];
			u_char *pos;

			if (level > MAX_SECRETS_RECURSION)
			{
				DBG1(DBG_CFG, "maximum level of %d includes reached, ignored",
					 MAX_SECRETS_RECURSION);
				continue;
			}
			/* terminate filename by space */
			line = chunk_skip(line, strlen("include "));
			pos = memchr(line.ptr, ' ', line.len);
			if (pos)
			{
				line.len = pos - line.ptr;
			}
			if (line.len && line.ptr[0] == '/')
			{
				if (line.len + 1 > sizeof(pattern))
				{
					DBG1(DBG_CFG, "include pattern too long, ignored");
					continue;
				}
				snprintf(pattern, sizeof(pattern), "%.*s",
						 (int)line.len, line.ptr);
			}
			else
			{	/* use directory of current file if relative */
				dir = path_dirname(file);

				if (line.len + 1 + strlen(dir) + 1 > sizeof(pattern))
				{
					DBG1(DBG_CFG, "include pattern too long, ignored");
					free(dir);
					continue;
				}
				snprintf(pattern, sizeof(pattern), "%s/%.*s",
						 dir, (int)line.len, line.ptr);
				free(dir);
			}
#ifdef HAVE_GLOB_H
			{
				glob_t buf;
				if (glob(pattern, GLOB_ERR, NULL, &buf) != 0)
				{
					DBG1(DBG_CFG, "expanding file expression '%s' failed",
						 pattern);
				}
				else
				{
					for (expanded = buf.gl_pathv; *expanded != NULL; expanded++)
					{
						load_secrets(this, secrets, *expanded, level + 1,
									 prompt);
					}
				}
				globfree(&buf);
			}
#else /* HAVE_GLOB_H */
			/* if glob(3) is not available, try to load pattern directly */
			load_secrets(this, secrets, pattern, level + 1, prompt);
#endif /* HAVE_GLOB_H */
			continue;
		}

		if (line.len > 2 && strpfx(line.ptr, ": "))
		{
			/* no ids, skip the ':' */
			ids = chunk_empty;
			line.ptr++;
			line.len--;
		}
		else if (extract_token_str(&ids, " : ", &line))
		{
			/* NULL terminate the extracted id string */
			*(ids.ptr + ids.len) = '\0';
		}
		else
		{
			DBG1(DBG_CFG, "line %d: missing ' : ' separator", line_nr);
			break;
		}

		if (!eat_whitespace(&line) || !extract_token(&token, ' ', &line))
		{
			DBG1(DBG_CFG, "line %d: missing token", line_nr);
			break;
		}
		if (match("RSA", &token) || match("ECDSA", &token) ||
			match("PKCS8", &token))
		{
			if (match("RSA", &token))
			{
				key_type = KEY_RSA;
			}
			else if (match("ECDSA", &token))
			{
				key_type = KEY_ECDSA;
			}
			else
			{
				key_type = KEY_ANY;
			}
			if (!load_private(secrets, line, line_nr, prompt, key_type))
			{
				break;
			}
		}
		else if (match("P12", &token))
		{
			if (!load_pkcs12(this, secrets, line, line_nr, prompt))
			{
				break;
			}
		}
		else if (match("PIN", &token))
		{
			if (!load_pin(secrets, line, line_nr, prompt))
			{
				break;
			}
		}
		else if ((match("PSK", &token) && (type = SHARED_IKE)) ||
				 (match("EAP", &token) && (type = SHARED_EAP)) ||
				 (match("NTLM", &token) && (type = SHARED_NT_HASH)) ||
				 (match("XAUTH", &token) && (type = SHARED_EAP)))
		{
			if (!load_shared(secrets, line, line_nr, type, ids))
			{
				break;
			}
		}
		else
		{
			DBG1(DBG_CFG, "line %d: token must be either RSA, ECDSA, PKCS8, "
						  "P12, PIN, PSK, EAP, XAUTH or NTLM", line_nr);
			break;
		}
	}
	chunk_unmap_clear(src);

	if (level == 0)
	{	/* replace secrets in active credential set */
		this->creds->replace_secrets(this->creds, secrets, FALSE);
		secrets->destroy(secrets);
	}
}

/**
 * load all certificates from ipsec.d
 */
static void load_certs(private_stroke_cred_t *this)
{
	mem_cred_t *creds;

	DBG1(DBG_CFG, "loading ca certificates from '%s'",
		 CA_CERTIFICATE_DIR);
	creds = mem_cred_create();
	load_certdir(this, CA_CERTIFICATE_DIR, CERT_X509, X509_CA, creds);
	this->ca->replace_certs(this->ca, creds);
	creds->destroy(creds);

	DBG1(DBG_CFG, "loading aa certificates from '%s'",
		 AA_CERTIFICATE_DIR);
	load_certdir(this, AA_CERTIFICATE_DIR, CERT_X509, X509_AA, this->aacerts);

	DBG1(DBG_CFG, "loading ocsp signer certificates from '%s'",
		 OCSP_CERTIFICATE_DIR);
	load_certdir(this, OCSP_CERTIFICATE_DIR, CERT_X509, X509_OCSP_SIGNER,
				 this->creds);

	DBG1(DBG_CFG, "loading attribute certificates from '%s'",
		 ATTR_CERTIFICATE_DIR);
	load_certdir(this, ATTR_CERTIFICATE_DIR, CERT_X509_AC, 0, this->creds);

	DBG1(DBG_CFG, "loading crls from '%s'",
		 CRL_DIR);
	load_certdir(this, CRL_DIR, CERT_X509_CRL, 0, this->creds);
}

METHOD(stroke_cred_t, reread, void,
	private_stroke_cred_t *this, stroke_msg_t *msg, FILE *prompt)
{
	mem_cred_t *creds;

	if (msg->reread.flags & REREAD_SECRETS)
	{
		DBG1(DBG_CFG, "rereading secrets");
		load_secrets(this, NULL, this->secrets_file, 0, prompt);
	}
	if (msg->reread.flags & REREAD_CACERTS)
	{
		/* first reload certificates in ca sections, so we can refer to them */
		this->ca->reload_certs(this->ca);

		DBG1(DBG_CFG, "rereading ca certificates from '%s'",
			 CA_CERTIFICATE_DIR);
		creds = mem_cred_create();
		load_certdir(this, CA_CERTIFICATE_DIR, CERT_X509, X509_CA, creds);
		this->ca->replace_certs(this->ca, creds);
		creds->destroy(creds);
	}
	if (msg->reread.flags & REREAD_AACERTS)
	{
		DBG1(DBG_CFG, "rereading aa certificates from '%s'",
			 AA_CERTIFICATE_DIR);
		creds = mem_cred_create();
		load_certdir(this, AA_CERTIFICATE_DIR, CERT_X509, X509_AA, creds);
		this->aacerts->replace_certs(this->aacerts, creds, FALSE);
		creds->destroy(creds);
		lib->credmgr->flush_cache(lib->credmgr, CERT_X509);
	}
	if (msg->reread.flags & REREAD_OCSPCERTS)
	{
		DBG1(DBG_CFG, "rereading ocsp signer certificates from '%s'",
			 OCSP_CERTIFICATE_DIR);
		load_certdir(this, OCSP_CERTIFICATE_DIR, CERT_X509,
			 X509_OCSP_SIGNER, this->creds);
	}
	if (msg->reread.flags & REREAD_ACERTS)
	{
		DBG1(DBG_CFG, "rereading attribute certificates from '%s'",
			 ATTR_CERTIFICATE_DIR);
		load_certdir(this, ATTR_CERTIFICATE_DIR, CERT_X509_AC, 0, this->creds);
	}
	if (msg->reread.flags & REREAD_CRLS)
	{
		DBG1(DBG_CFG, "rereading crls from '%s'",
			 CRL_DIR);
		load_certdir(this, CRL_DIR, CERT_X509_CRL, 0, this->creds);
	}
}

METHOD(stroke_cred_t, add_shared, void,
	private_stroke_cred_t *this, shared_key_t *shared, linked_list_t *owners)
{
	this->creds->add_shared_list(this->creds, shared, owners);
}

METHOD(stroke_cred_t, destroy, void,
	private_stroke_cred_t *this)
{
	lib->credmgr->remove_set(lib->credmgr, &this->aacerts->set);
	lib->credmgr->remove_set(lib->credmgr, &this->creds->set);
	this->aacerts->destroy(this->aacerts);
	this->creds->destroy(this->creds);
	free(this);
}

/*
 * see header file
 */
stroke_cred_t *stroke_cred_create(stroke_ca_t *ca)
{
	private_stroke_cred_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_private_enumerator = (void*)return_null,
				.create_cert_enumerator = (void*)return_null,
				.create_shared_enumerator = (void*)return_null,
				.create_cdp_enumerator = (void*)return_null,
				.cache_cert = (void*)_cache_cert,
			},
			.reread = _reread,
			.load_peer = _load_peer,
			.load_pubkey = _load_pubkey,
			.add_shared = _add_shared,
			.cachecrl = _cachecrl,
			.destroy = _destroy,
		},
		.secrets_file = lib->settings->get_str(lib->settings,
								"%s.plugins.stroke.secrets_file", SECRETS_FILE,
								lib->ns),
		.creds = mem_cred_create(),
		.aacerts = mem_cred_create(),
		.ca = ca,
	);

	if (lib->settings->get_bool(lib->settings, "%s.cache_crls", FALSE, lib->ns))
	{
		cachecrl(this, TRUE);
	}
	lib->credmgr->add_set(lib->credmgr, &this->creds->set);
	lib->credmgr->add_set(lib->credmgr, &this->aacerts->set);

	this->force_ca_cert = lib->settings->get_bool(lib->settings,
						"%s.plugins.stroke.ignore_missing_ca_basic_constraint",
						FALSE, lib->ns);

	load_certs(this);
	load_secrets(this, NULL, this->secrets_file, 0, NULL);

	return &this->public;
}
