/*
 * Copyright (C) 2009 Martin Willi
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

#include <time.h>
#include <errno.h>

#include "pki.h"

#include <collections/linked_list.h>
#include <credentials/certificates/certificate.h>
#include <credentials/certificates/x509.h>
#include <asn1/asn1.h>

/**
 * Free cert policy with OID
 */
static void destroy_cert_policy(x509_cert_policy_t *policy)
{
	free(policy->oid.ptr);
	free(policy);
}

/**
 * Free policy mapping
 */
static void destroy_policy_mapping(x509_policy_mapping_t *mapping)
{
	free(mapping->issuer.ptr);
	free(mapping->subject.ptr);
	free(mapping);
}

/**
 * Create a self signed certificate.
 */
static int self()
{
	cred_encoding_type_t form = CERT_ASN1_DER;
	key_type_t type = KEY_RSA;
	hash_algorithm_t digest = HASH_SHA1;
	certificate_t *cert = NULL;
	private_key_t *private = NULL;
	public_key_t *public = NULL;
	char *file = NULL, *dn = NULL, *hex = NULL, *error = NULL, *keyid = NULL;
	identification_t *id = NULL;
	linked_list_t *san, *ocsp, *permitted, *excluded, *policies, *mappings;
	int pathlen = X509_NO_CONSTRAINT, inhibit_any = X509_NO_CONSTRAINT;
	int inhibit_mapping = X509_NO_CONSTRAINT, require_explicit = X509_NO_CONSTRAINT;
	chunk_t serial = chunk_empty;
	chunk_t encoding = chunk_empty;
	time_t not_before, not_after, lifetime = 1095 * 24 * 60 * 60;
	char *datenb = NULL, *datena = NULL, *dateform = NULL;
	x509_flag_t flags = 0;
	x509_cert_policy_t *policy = NULL;
	char *arg;

	san = linked_list_create();
	ocsp = linked_list_create();
	permitted = linked_list_create();
	excluded = linked_list_create();
	policies = linked_list_create();
	mappings = linked_list_create();

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				goto usage;
			case 't':
				if (streq(arg, "rsa"))
				{
					type = KEY_RSA;
				}
				else if (streq(arg, "ecdsa"))
				{
					type = KEY_ECDSA;
				}
				else
				{
					error = "invalid input type";
					goto usage;
				}
				continue;
			case 'g':
				if (!enum_from_name(hash_algorithm_short_names, arg, &digest))
				{
					error = "invalid --digest type";
					goto usage;
				}
				continue;
			case 'i':
				file = arg;
				continue;
			case 'x':
				keyid = arg;
				continue;
			case 'd':
				dn = arg;
				continue;
			case 'a':
				san->insert_last(san, identification_create_from_string(arg));
				continue;
			case 'l':
				lifetime = atoi(arg) * 24 * 60 * 60;
				if (!lifetime)
				{
					error = "invalid --lifetime value";
					goto usage;
				}
				continue;
			case 'D':
				dateform = arg;
				continue;
			case 'F':
				datenb = arg;
				continue;
			case 'T':
				datena = arg;
				continue;
			case 's':
				hex = arg;
				continue;
			case 'b':
				flags |= X509_CA;
				continue;
			case 'p':
				pathlen = atoi(arg);
				continue;
			case 'n':
				permitted->insert_last(permitted,
									   identification_create_from_string(arg));
				continue;
			case 'N':
				excluded->insert_last(excluded,
									  identification_create_from_string(arg));
				continue;
			case 'P':
			{
				chunk_t oid;

				oid = asn1_oid_from_string(arg);
				if (!oid.len)
				{
					error = "--cert-policy OID invalid";
					goto usage;
				}
				INIT(policy,
					.oid = oid,
				);
				policies->insert_last(policies, policy);
				continue;
			}
			case 'C':
				if (!policy)
				{
					error = "--cps-uri must follow a --cert-policy";
					goto usage;
				}
				policy->cps_uri = arg;
				continue;
			case 'U':
				if (!policy)
				{
					error = "--user-notice must follow a --cert-policy";
					goto usage;
				}
				policy->unotice_text = arg;
				continue;
			case 'M':
			{
				char *pos = strchr(arg, ':');
				x509_policy_mapping_t *mapping;
				chunk_t subject_oid, issuer_oid;

				if (pos)
				{
					*pos++ = '\0';
					issuer_oid = asn1_oid_from_string(arg);
					subject_oid = asn1_oid_from_string(pos);
				}
				if (!pos || !issuer_oid.len || !subject_oid.len)
				{
					error = "--policy-map OIDs invalid";
					goto usage;
				}
				INIT(mapping,
					.issuer = issuer_oid,
					.subject = subject_oid,
				);
				mappings->insert_last(mappings, mapping);
				continue;
			}
			case 'E':
				require_explicit = atoi(arg);
				continue;
			case 'H':
				inhibit_mapping = atoi(arg);
				continue;
			case 'A':
				inhibit_any = atoi(arg);
				continue;
			case 'e':
				if (streq(arg, "serverAuth"))
				{
					flags |= X509_SERVER_AUTH;
				}
				else if (streq(arg, "clientAuth"))
				{
					flags |= X509_CLIENT_AUTH;
				}
				else if (streq(arg, "ikeIntermediate"))
				{
					flags |= X509_IKE_INTERMEDIATE;
				}
				else if (streq(arg, "crlSign"))
				{
					flags |= X509_CRL_SIGN;
				}
				else if (streq(arg, "ocspSigning"))
				{
					flags |= X509_OCSP_SIGNER;
				}
				else if (streq(arg, "msSmartcardLogon"))
				{
					flags |= X509_MS_SMARTCARD_LOGON;
				}
				continue;
			case 'f':
				if (!get_form(arg, &form, CRED_CERTIFICATE))
				{
					error = "invalid output format";
					goto usage;
				}
				continue;
			case 'o':
				ocsp->insert_last(ocsp, arg);
				continue;
			case EOF:
				break;
			default:
				error = "invalid --self option";
				goto usage;
		}
		break;
	}

	if (!dn)
	{
		error = "--dn is required";
		goto usage;
	}
	if (!calculate_lifetime(dateform, datenb, datena, lifetime,
							&not_before, &not_after))
	{
		error = "invalid --not-before/after datetime";
		goto usage;
	}
	id = identification_create_from_string(dn);
	if (id->get_type(id) != ID_DER_ASN1_DN)
	{
		error = "supplied --dn is not a distinguished name";
		goto end;
	}
	if (file)
	{
		private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, type,
									 BUILD_FROM_FILE, file, BUILD_END);
	}
	else if (keyid)
	{
		chunk_t chunk;

		chunk = chunk_from_hex(chunk_create(keyid, strlen(keyid)), NULL);
		private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_ANY,
									 BUILD_PKCS11_KEYID, chunk, BUILD_END);
		free(chunk.ptr);
	}
	else
	{
		chunk_t chunk;

		set_file_mode(stdin, CERT_ASN1_DER);
		if (!chunk_from_fd(0, &chunk))
		{
			fprintf(stderr, "%s: ", strerror(errno));
			error = "reading private key failed";
			goto end;
		}
		private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, type,
									 BUILD_BLOB, chunk, BUILD_END);
		free(chunk.ptr);
	}
	if (!private)
	{
		error = "loading private key failed";
		goto end;
	}
	public = private->get_public_key(private);
	if (!public)
	{
		error = "extracting public key failed";
		goto end;
	}
	if (hex)
	{
		serial = chunk_from_hex(chunk_create(hex, strlen(hex)), NULL);
	}
	else
	{
		rng_t *rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);

		if (!rng)
		{
			error = "no random number generator found";
			goto end;
		}
		if (!rng_allocate_bytes_not_zero(rng, 8, &serial, FALSE))
		{
			error = "failed to generate serial number";
			rng->destroy(rng);
			goto end;
		}
		serial.ptr[0] &= 0x7F;
		rng->destroy(rng);
	}
	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
						BUILD_SIGNING_KEY, private, BUILD_PUBLIC_KEY, public,
						BUILD_SUBJECT, id, BUILD_NOT_BEFORE_TIME, not_before,
						BUILD_NOT_AFTER_TIME, not_after, BUILD_SERIAL, serial,
						BUILD_DIGEST_ALG, digest, BUILD_X509_FLAG, flags,
						BUILD_PATHLEN, pathlen, BUILD_SUBJECT_ALTNAMES, san,
						BUILD_OCSP_ACCESS_LOCATIONS, ocsp,
						BUILD_PERMITTED_NAME_CONSTRAINTS, permitted,
						BUILD_EXCLUDED_NAME_CONSTRAINTS, excluded,
						BUILD_CERTIFICATE_POLICIES, policies,
						BUILD_POLICY_MAPPINGS, mappings,
						BUILD_POLICY_REQUIRE_EXPLICIT, require_explicit,
						BUILD_POLICY_INHIBIT_MAPPING, inhibit_mapping,
						BUILD_POLICY_INHIBIT_ANY, inhibit_any,
						BUILD_END);
	if (!cert)
	{
		error = "generating certificate failed";
		goto end;
	}
	if (!cert->get_encoding(cert, form, &encoding))
	{
		error = "encoding certificate failed";
		goto end;
	}
	set_file_mode(stdout, form);
	if (fwrite(encoding.ptr, encoding.len, 1, stdout) != 1)
	{
		error = "writing certificate key failed";
		goto end;
	}

end:
	DESTROY_IF(id);
	DESTROY_IF(cert);
	DESTROY_IF(public);
	DESTROY_IF(private);
	san->destroy_offset(san, offsetof(identification_t, destroy));
	permitted->destroy_offset(permitted, offsetof(identification_t, destroy));
	excluded->destroy_offset(excluded, offsetof(identification_t, destroy));
	policies->destroy_function(policies, (void*)destroy_cert_policy);
	mappings->destroy_function(mappings, (void*)destroy_policy_mapping);
	ocsp->destroy(ocsp);
	free(encoding.ptr);
	free(serial.ptr);

	if (error)
	{
		fprintf(stderr, "%s\n", error);
		return 1;
	}
	return 0;

usage:
	san->destroy_offset(san, offsetof(identification_t, destroy));
	permitted->destroy_offset(permitted, offsetof(identification_t, destroy));
	excluded->destroy_offset(excluded, offsetof(identification_t, destroy));
	policies->destroy_function(policies, (void*)destroy_cert_policy);
	mappings->destroy_function(mappings, (void*)destroy_policy_mapping);
	ocsp->destroy(ocsp);
	return command_usage(error);
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		self, 's', "self",
		"create a self signed certificate",
		{" [--in file|--keyid hex] [--type rsa|ecdsa]",
		 " --dn distinguished-name [--san subjectAltName]+",
		 "[--lifetime days] [--serial hex] [--ca] [--ocsp uri]+",
		 "[--flag serverAuth|clientAuth|crlSign|ocspSigning|msSmartcardLogon]+",
		 "[--nc-permitted name] [--nc-excluded name]",
		 "[--policy-map issuer-oid:subject-oid]",
		 "[--policy-explicit len] [--policy-inhibit len] [--policy-any len]",
		 "[--cert-policy oid [--cps-uri uri] [--user-notice text]]+",
		 "[--digest md5|sha1|sha224|sha256|sha384|sha512] [--outform der|pem]"},
		{
			{"help",			'h', 0, "show usage information"},
			{"in",				'i', 1, "private key input file, default: stdin"},
			{"keyid",			'x', 1, "keyid on smartcard of private key"},
			{"type",			't', 1, "type of input key, default: rsa"},
			{"dn",				'd', 1, "subject and issuer distinguished name"},
			{"san",				'a', 1, "subjectAltName to include in certificate"},
			{"lifetime",		'l', 1, "days the certificate is valid, default: 1095"},
			{"not-before",		'F', 1, "date/time the validity of the cert starts"},
			{"not-after",		'T', 1, "date/time the validity of the cert ends"},
			{"dateform",		'D', 1, "strptime(3) input format, default: %d.%m.%y %T"},
			{"serial",			's', 1, "serial number in hex, default: random"},
			{"ca",				'b', 0, "include CA basicConstraint, default: no"},
			{"pathlen",			'p', 1, "set path length constraint"},
			{"nc-permitted",	'n', 1, "add permitted NameConstraint"},
			{"nc-excluded",		'N', 1, "add excluded NameConstraint"},
			{"cert-policy",		'P', 1, "certificatePolicy OID to include"},
			{"cps-uri",			'C', 1, "Certification Practice statement URI for certificatePolicy"},
			{"user-notice",		'U', 1, "user notice for certificatePolicy"},
			{"policy-mapping",	'M', 1, "policyMapping from issuer to subject OID"},
			{"policy-explicit",	'E', 1, "requireExplicitPolicy constraint"},
			{"policy-inhibit",	'H', 1, "inhibitPolicyMapping constraint"},
			{"policy-any",		'A', 1, "inhibitAnyPolicy constraint"},
			{"flag",			'e', 1, "include extendedKeyUsage flag"},
			{"ocsp",			'o', 1, "OCSP AuthorityInfoAccess URI to include"},
			{"digest",			'g', 1, "digest for signature creation, default: sha1"},
			{"outform",			'f', 1, "encoding of generated cert, default: der"},
		}
	});
}
