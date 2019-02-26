/*
 * Copyright (C) 2017 Andreas Steffen
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

#include "tpm_cert.h"

#include <tpm_tss.h>

#include <utils/debug.h>


/**
 * See header.
 */
certificate_t *tpm_cert_load(certificate_type_t type, va_list args)
{
	tpm_tss_t *tpm;
	chunk_t keyid = chunk_empty, pin = chunk_empty, data = chunk_empty;
	certificate_t *cert;
	char handle_str[4];
	size_t len;
	uint32_t hierarchy = 0x40000001;  /* TPM_RH_OWNER */
	uint32_t handle;
	bool success;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_PKCS11_KEYID:
				keyid = va_arg(args, chunk_t);
				continue;
			case BUILD_PKCS11_SLOT:
				hierarchy = va_arg(args, int);
				continue;
			case BUILD_PKCS11_MODULE:
				va_arg(args, char*);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	/* convert keyid into 32 bit TPM key object handle */
	if (!keyid.len)
	{
		return NULL;
	}
	len = min(keyid.len, 4);
	memset(handle_str, 0x00, 4);
	memcpy(handle_str + 4 - len, keyid.ptr + keyid.len - len, len);
	handle = untoh32(handle_str);

	/* try to find a TPM 2.0 */
	tpm = tpm_tss_probe(TPM_VERSION_2_0);
	if (!tpm)
	{
		DBG1(DBG_LIB, "no TPM 2.0 found");
		return NULL;
	}
	success = tpm->get_data(tpm, hierarchy, handle, pin, &data);
	tpm->destroy(tpm);

	if (!success)
	{
		DBG1(DBG_LIB, "loading certificate from TPM NV index 0x%08x failed",
					   handle);
		return NULL;
	}
	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
							  BUILD_BLOB_ASN1_DER, data, BUILD_END);
	free(data.ptr);

	if (!cert)
	{
		DBG1(DBG_LIB, "parsing certificate from TPM NV index 0x%08x failed",
					   handle);
		return NULL;
	}
	DBG1(DBG_LIB, "loaded certificate from TPM NV index 0x%08x", handle);

	return cert;
}
