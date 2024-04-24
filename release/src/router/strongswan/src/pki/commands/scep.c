/*
 * Copyright (C) 2005 Jan Hutter, Martin Willi
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2022 Andreas Steffen, strongSec GmbH
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

#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "pki.h"
#include "pki_cert.h"
#include "scep/scep.h"

#include <credentials/certificates/certificate.h>
#include <credentials/certificates/x509.h>
#include <credentials/sets/mem_cred.h>
#include <asn1/asn1.h>

/* default polling time interval in SCEP manual mode */
#define DEFAULT_POLL_INTERVAL    60       /* seconds */

/**
 * Enroll an X.509 certificate with a SCEP server (RFC 8894)
 */
static int scep()
{
	char *arg, *url = NULL, *file = NULL, *dn = NULL, *error = NULL;
	char *ca_enc_file = NULL, *ca_sig_file = NULL;
	char *client_cert_file = NULL, *client_key_file = NULL;
	cred_encoding_type_t form = CERT_ASN1_DER;
	chunk_t scep_response = chunk_empty;
	chunk_t challenge_password = chunk_empty;
	chunk_t cert_type = chunk_empty;
	chunk_t serialNumber = chunk_empty;
	chunk_t transID = chunk_empty;
	chunk_t pkcs10_encoding = chunk_empty;
	chunk_t pkcs7_req = chunk_empty;
	chunk_t certPoll = chunk_empty;
	chunk_t issuerAndSubject = chunk_empty;
	chunk_t data = chunk_empty;
	hash_algorithm_t digest_alg = HASH_SHA256;
	encryption_algorithm_t cipher = ENCR_AES_CBC;
	uint16_t key_size = 128;
	signature_params_t *scheme = NULL;
	private_key_t *private = NULL, *priv_signer = NULL;
	public_key_t *public = NULL;
	certificate_t *pkcs10 = NULL, *x509_signer = NULL, *cert = NULL;
	certificate_t *x509_ca_sig = NULL, *x509_ca_enc = NULL;
	identification_t *subject = NULL, *issuer = NULL;
	container_t *container = NULL;
	mem_cred_t *creds = NULL, *client_creds = NULL;
	scep_msg_t scep_msg_type;
	scep_attributes_t attrs = empty_scep_attributes;
	uint32_t caps_flags;
	u_int poll_interval = DEFAULT_POLL_INTERVAL;
	u_int max_poll_time = 0, poll_start = 0;
	u_int http_code = 0;
	time_t notBefore, notAfter;
	linked_list_t *san;
	int status = 1;
	bool ok, http_post = FALSE;

	bool pss = lib->settings->get_bool(lib->settings,
								"%s.rsa_pss", FALSE, lib->ns);

	bool renewal_via_pkcs_req = lib->settings->get_bool(lib->settings,
								"%s.scep.renewal_via_pkcs_req", FALSE, lib->ns);


	/* initialize certificate validity */
	notBefore = time(NULL);
	notAfter  = notBefore + 365 * 24 * 60 * 60;

	/* initialize list of subjectAltNames */
	san = linked_list_create();

	/* initialize CA certificate storage */
	creds = mem_cred_create();
	lib->credmgr->add_set(lib->credmgr, &creds->set);

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':       /* --help */
				goto usage;
			case 'u':       /* --url */
				url = arg;
				continue;
			case 'i':       /* --in */
				file = arg;
				continue;
			case 'd':       /* --dn */
				dn = arg;
				continue;
			case 'a':       /* --san */
				san->insert_last(san, identification_create_from_string(arg));
				continue;
			case 'P':       /* --profile */
				cert_type = chunk_create(arg, strlen(arg));
				continue;
			case 'p':       /* --password */
				challenge_password = chunk_create(arg, strlen(arg));
				continue;
			case 'e':       /* --cacert-enc */
				ca_enc_file = arg;
				continue;
			case 's':       /* --cacert-sig */
				ca_sig_file = arg;
				continue;
			case 'C':       /* --cacert */
				cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
									BUILD_FROM_FILE, arg, BUILD_END);
				if (!cert)
				{
					DBG1(DBG_APP, "could not load cacert file '%s'", arg);
					goto err;
				}
				creds->add_cert(creds, TRUE, cert);
				continue;
			case 'c':       /* --cert */
				client_cert_file = arg;
				continue;
			case 'k':       /* --key */
				client_key_file = arg;
				continue;
			case 'E':       /* --cipher */
				if (strcaseeq(arg, "des3"))
				{
					cipher = ENCR_3DES;
					key_size = 0;
				}
				else if (strcaseeq(arg, "aes"))
				{
					cipher = ENCR_AES_CBC;
					key_size = 128;
				}
				else
				{
					error = "invalid --cipher type";
					goto usage;
				}
				continue;
			case 'g':       /* --digest */
				if (!enum_from_name(hash_algorithm_short_names, arg, &digest_alg))
				{
					error = "invalid --digest type";
					goto usage;
				}
				continue;
			case 'R':       /* --rsa-padding */
				if (!parse_rsa_padding(arg, &pss))
				{
					error = "invalid RSA padding";
					goto usage;
				}
				continue;
			case 't':       /* --pollinterval */
				poll_interval = atoi(optarg);
				if (poll_interval <= 0)
				{
					error = "invalid interval specified";
					goto usage;
				}
				continue;
			case 'm':       /* --maxpolltime */
				max_poll_time = atoi(optarg);
				continue;
			case 'f':       /* --form */
				if (!get_form(arg, &form, CRED_CERTIFICATE))
				{
					error = "invalid certificate output format";
					goto usage;
				}
				continue;
			case EOF:
				break;
			default:
				error =  "invalid --scep option";
				goto usage;
		}
		break;
	}

	if (!url)
	{
		error = "--url is required";
		goto usage;
	}

	if (!ca_enc_file)
	{
		error = "--cacert-enc is required";
		goto usage;
	}

	if (!ca_sig_file)
	{
		error = "--cacert-sig is required";
		goto usage;
	}

	if (client_cert_file && !client_key_file)
	{
		error = "--key is required if --cert is set";
		goto usage;
	}

	if (!dn && !client_cert_file)
	{
		error = "--dn is required if --cert is not set";
		goto usage;
	}
	else if (dn)
	{
		subject = identification_create_from_string(dn);
		if (subject->get_type(subject) != ID_DER_ASN1_DN)
		{
			DBG1(DBG_APP, "supplied --dn is not a distinguished name");
			goto err;
		}
	}

	/* load RSA private key from file or stdin */
	if (file)
	{
		private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
									 BUILD_FROM_FILE, file, BUILD_END);
	}
	else
	{
		chunk_t chunk;

		set_file_mode(stdin, CERT_ASN1_DER);
		if (!chunk_from_fd(0, &chunk))
		{
			DBG1(DBG_APP, "reading private key failed: %s", strerror(errno));
			goto err;
		}
		private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
									 BUILD_BLOB, chunk, BUILD_END);
		free(chunk.ptr);
	}
	if (!private)
	{
		DBG1(DBG_APP, "parsing private key failed");
		goto err;
	}
	public = private->get_public_key(private);

	/* Request capabilities from SCEP server */
	if (!scep_http_request(url, SCEP_GET_CA_CAPS, FALSE, chunk_empty,
						   &scep_response, &http_code))
	{
		DBG1(DBG_APP, "did not receive a valid scep response: HTTP %u", http_code);
		goto err;
	}
	caps_flags = scep_parse_caps(scep_response);
	chunk_free(&scep_response);

	/* check support of selected digest algorithm */
	switch (digest_alg)
	{
		case HASH_SHA256:
			ok = (caps_flags & SCEP_CAPS_SHA256) ||
				 (caps_flags & SCEP_CAPS_SCEPSTANDARD);
			break;
		case HASH_SHA384:
			ok = (caps_flags & SCEP_CAPS_SHA384);
			break;
		case HASH_SHA512:
			ok = (caps_flags & SCEP_CAPS_SHA512);
			break;
		case HASH_SHA224:
			ok = (caps_flags & SCEP_CAPS_SHA224);
			break;
		case HASH_SHA1:
			ok = (caps_flags & SCEP_CAPS_SHA1);
			break;
		default:
			ok = FALSE;
	}
	if (!ok)
	{
		DBG1(DBG_APP, "%N digest algorithm not supported by CA",
					  hash_algorithm_short_names, digest_alg);
		goto err;
	}

	/* check support of selected encryption algorithm */
	switch (cipher)
	{
		case ENCR_AES_CBC:
			ok = (caps_flags & SCEP_CAPS_AES) ||
				 (caps_flags & SCEP_CAPS_SCEPSTANDARD);
			break;
		case ENCR_3DES:
			ok = (caps_flags & SCEP_CAPS_DES3);
			break;
		default:
			ok = FALSE;
	}
	if (!ok)
	{
		DBG1(DBG_APP, "%N encryption algorithm not supported by CA",
					  encryption_algorithm_names, cipher);
		goto err;
	}
	DBG2(DBG_APP, "%N digest and %N encryption algorithm supported by CA",
				  hash_algorithm_short_names, digest_alg,
				  encryption_algorithm_names, cipher);

	/* check support of HTTP POST operation */
	if ((caps_flags & SCEP_CAPS_POSTPKIOPERATION) ||
	    (caps_flags & SCEP_CAPS_SCEPSTANDARD))
	{
		http_post = TRUE;
	}
	DBG2(DBG_APP, "HTTP POST %ssupported", http_post ? "" : "not ");

	if (!scep_generate_transaction_id(public, &transID, &serialNumber))
	{
		DBG1(DBG_APP, "generating transaction ID failed");
		goto err;
	}
	DBG1(DBG_APP, "transaction ID: %.*s", (int)transID.len, transID.ptr);

	scheme = get_signature_scheme(private, digest_alg, pss);
	if (!scheme)
	{
		DBG1(DBG_APP, "no signature scheme found");
		goto err;
	}

	if (client_cert_file)
	{
		/* check support of Renewal Operation */
		if (!(caps_flags & SCEP_CAPS_RENEWAL))
		{
			DBG1(DBG_APP, "Renewal operation not supported by SCEP server");
			goto err;
		}
		DBG2(DBG_APP, "SCEP Renewal operation supported");

		/* set message type for SCEP renewal request */
		scep_msg_type = renewal_via_pkcs_req ? SCEP_PKCSReq_MSG :
											   SCEP_RenewalReq_MSG;

		/* load old client certificate */
		x509_signer  = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
									BUILD_FROM_FILE, client_cert_file, BUILD_END);
		if (!x509_signer)
		{
			DBG1(DBG_APP, "loading client cert file '%s' failed",
						   client_cert_file);
			goto err;
		}

		/* load old RSA private key */
		priv_signer = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
									 BUILD_FROM_FILE, client_key_file, BUILD_END);
		if (!priv_signer)
		{
			DBG1(DBG_APP, "loading client private key file '%s' failed",
						   client_key_file);
			x509_signer->destroy(x509_signer);
			goto err;
		}

		if (!subject)
		{
			subject = x509_signer->get_subject(x509_signer);
			subject = subject->clone(subject);
		}
	}
	else
	{
		/* create self-signed X.509 certificate */
		x509_signer = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
									BUILD_SIGNING_KEY, private,
									BUILD_PUBLIC_KEY, public,
									BUILD_SUBJECT, subject,
									BUILD_NOT_BEFORE_TIME, notBefore,
									BUILD_NOT_AFTER_TIME, notAfter,
									BUILD_SERIAL, serialNumber,
									BUILD_SUBJECT_ALTNAMES, san,
									BUILD_SIGNATURE_SCHEME, scheme,
									BUILD_END);
		if (!x509_signer)
		{
			DBG1(DBG_APP, "generating self-signed certificate failed");
			goto err;
		}

		/* the signing key is identical to the client key */
		priv_signer = private->get_ref(private);

		/* set message type for SCEP request */
		scep_msg_type = SCEP_PKCSReq_MSG;
	}

	/* create a separate set for the self-signed or old client credentials */
	client_creds = mem_cred_create();
	lib->credmgr->add_set(lib->credmgr, &client_creds->set);

	client_creds->add_cert(client_creds, FALSE, x509_signer);
	client_creds->add_key(client_creds, priv_signer);

	/* generate PKCS#10 certificate request */
	pkcs10 = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_PKCS10_REQUEST,
							BUILD_SIGNING_KEY, private,
							BUILD_SUBJECT, subject,
							BUILD_SUBJECT_ALTNAMES, san,
							BUILD_CHALLENGE_PWD, challenge_password,
							BUILD_CERT_TYPE_EXT, cert_type,
							BUILD_SIGNATURE_SCHEME, scheme,
							BUILD_END);
	if (!pkcs10)
	{
		DBG1(DBG_APP, "generating certificate request failed");
		goto err;
	}

	/* generate PKCS#10 encoding */
	if (!pkcs10->get_encoding(pkcs10, CERT_ASN1_DER, &pkcs10_encoding))
	{
		DBG1(DBG_APP, "encoding certificate request failed");
		pkcs10->destroy(pkcs10);
		goto err;
	}
	pkcs10->destroy(pkcs10);

	/* load CA or RA certificate used for encryption */
	x509_ca_enc = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
									BUILD_FROM_FILE, ca_enc_file, BUILD_END);
	if (!x509_ca_enc)
	{
		DBG1(DBG_APP, "could not load encryption cacert file '%s'", ca_enc_file);
		goto end;
	}

	/* load CA certificate used for signature verification */
	x509_ca_sig = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
									BUILD_FROM_FILE, ca_sig_file, BUILD_END);
	if (!x509_ca_sig)
	{
		DBG1(DBG_APP, "could not load signature cacert file '%s'", ca_sig_file);
		goto end;
	}
	x509_ca_sig = creds->add_cert_ref(creds, TRUE, x509_ca_sig);

	/* build pkcs7 request */
	pkcs7_req = scep_build_request(pkcs10_encoding, transID, scep_msg_type,
								   x509_ca_enc, cipher, key_size, x509_signer,
								   digest_alg, scheme, priv_signer);
	if (!pkcs7_req.ptr)
	{
		DBG1(DBG_APP, "failed to build SCEP request");
		goto end;
	}

	if (!scep_http_request(url, SCEP_PKI_OPERATION, http_post, pkcs7_req,
						   &scep_response, &http_code))
	{
		DBG1(DBG_APP, "did not receive a valid SCEP response: HTTP %u", http_code);
		goto end;
	}

	if (!scep_parse_response(scep_response, transID, &container, &attrs))
	{
			goto end;
	}

	/* in case of manual mode, we are going into a polling loop */
	if (attrs.pkiStatus == SCEP_PENDING)
	{
		issuer = x509_ca_sig->get_subject(x509_ca_sig);
		issuerAndSubject = asn1_wrap(ASN1_SEQUENCE, "cc",
							issuer->get_encoding(issuer),
							subject->get_encoding(subject));
		if (max_poll_time > 0)
		{
			DBG1(DBG_APP, "  SCEP request pending, polling every %d seconds"
						  " up to %d seconds", poll_interval, max_poll_time);
		}
		else
		{
			DBG1(DBG_APP, "  SCEP request pending, polling indefinitely"
						  " every %d seconds", poll_interval);
		}
		poll_start = time_monotonic(NULL);
	}

	while (attrs.pkiStatus == SCEP_PENDING)
	{
		if (max_poll_time > 0 &&
		   (time_monotonic(NULL) - poll_start) >= max_poll_time)
		{
			DBG1(DBG_APP, "maximum poll time reached: %d seconds", max_poll_time);
			goto end;
		}
		DBG1(DBG_APP, "  going to sleep for %d seconds", poll_interval);
		sleep(poll_interval);
		chunk_free(&certPoll);
		chunk_free(&scep_response);
		chunk_free(&attrs.transID);
		chunk_free(&attrs.recipientNonce);
		container->destroy(container);
		container = NULL;

		DBG1(DBG_APP, "transaction ID: %.*s", (int)transID.len, transID.ptr);

		certPoll = scep_build_request(issuerAndSubject, transID, SCEP_CertPoll_MSG,
									  x509_ca_enc, cipher, key_size, x509_signer,
									  digest_alg, scheme, priv_signer);
		if (!certPoll.ptr)
		{
			DBG1(DBG_APP, "failed to build SCEP certPoll request");
			goto end;
		}
		if (!scep_http_request(url, SCEP_PKI_OPERATION, http_post, certPoll,
							   &scep_response, &http_code))
		{
			DBG1(DBG_APP, "did not receive a valid SCEP response: HTTP %u",
						   http_code);
			goto end;
		}
		if (!scep_parse_response(scep_response, transID, &container, &attrs))
		{
			goto end;
		}
	}

	if (attrs.pkiStatus != SCEP_SUCCESS)
	{
		DBG1(DBG_APP, "reply status is not 'SUCCESS'");
		goto end;
	}

	if (!container->get_data(container, &data))
	{
		DBG1(DBG_APP, "extracting enveloped-data failed");
		goto end;
	}
	container->destroy(container);

	/* decrypt enveloped-data container */
	container = lib->creds->create(lib->creds,
								   CRED_CONTAINER, CONTAINER_PKCS7,
								   BUILD_BLOB_ASN1_DER, data,
								   BUILD_END);
	chunk_free(&data);

	if (!container)
	{
		DBG1(DBG_APP, "could not decrypt envelopedData");
		goto end;
	}

	if (!container->get_data(container, &data))
	{
		DBG1(DBG_APP, "extracting encrypted-data failed");
		goto end;
	}
	container->destroy(container);
	container = NULL;
	status = 0;

end:
	/* remove the old client certificate before extracting the new one */
	lib->credmgr->remove_set(lib->credmgr, &client_creds->set);
	client_creds->destroy(client_creds);

	if (status == 0)
	{
		status = pki_cert_extract_cert(data, form) ? 0 : 1;
		chunk_clear(&data);
	}

err:
	/* cleanup */
	lib->credmgr->remove_set(lib->credmgr, &creds->set);
	creds->destroy(creds);
	san->destroy_offset(san, offsetof(identification_t, destroy));
	signature_params_destroy(scheme);
	DESTROY_IF(subject);
	DESTROY_IF(private);
	DESTROY_IF(public);
	DESTROY_IF(x509_ca_enc);
	DESTROY_IF(x509_ca_sig);
	DESTROY_IF(container);
	chunk_free(&scep_response);
	chunk_free(&serialNumber);
	chunk_free(&transID);
	chunk_free(&pkcs10_encoding);
	chunk_free(&pkcs7_req);
	chunk_free(&certPoll);
	chunk_free(&issuerAndSubject);
	chunk_free(&attrs.transID);
	chunk_free(&attrs.recipientNonce);

	return status;

usage:
	lib->credmgr->remove_set(lib->credmgr, &creds->set);
	creds->destroy(creds);
	san->destroy_offset(san, offsetof(identification_t, destroy));

	return command_usage(error);
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		scep, 'S', "scep",
		"Enroll an X.509 certificate with a SCEP server",
		{"--url url [--in file] [--dn distinguished-name] [--san subjectAltName]+",
		 "[--profile profile] [--password password]",
		 " --cacert-enc file --cacert-sig file [--cacert file]+",
		 " --cert file --key file] [--cipher aes|des3]",
		 "[--digest sha256|sha384|sha512|sha224|sha1] [--rsa-padding pkcs1|pss]",
		 "[--interval time] [--maxpolltime time] [--outform der|pem]"},
		{
			{"help",        'h', 0, "show usage information"},
			{"url",         'u', 1, "URL of the SCEP server"},
			{"in",          'i', 1, "RSA private key input file, default: stdin"},
			{"dn",          'd', 1, "subject distinguished name (optional if --cert is given)"},
			{"san",         'a', 1, "subjectAltName to include in cert request"},
			{"profile",     'P', 1, "certificate profile name to include in cert request"},
			{"password",    'p', 1, "challengePassword to include in cert request"},
			{"cacert-enc",  'e', 1, "CA certificate for encryption"},
			{"cacert-sig",  's', 1, "CA certificate for signature verification"},
			{"cacert",      'C', 1, "Additional CA certificates"},
			{"cert",        'c', 1, "Old certificate about to be renewed"},
			{"key",         'k', 1, "Old RSA private key about to be replaced"},
			{"cipher",      'E', 1, "encryption cipher, default: aes"},
			{"digest",      'g', 1, "digest for signature creation, default: sha256"},
			{"rsa-padding", 'R', 1, "padding for RSA signatures, default: pkcs1"},
			{"interval",    't', 1, "poll interval, default: 60s"},
			{"maxpolltime", 'm', 1, "maximum poll time, default: 0 (no limit)"},
			{"outform",     'f', 1, "encoding of stored certificates, default: der"},
		}
	});
}
