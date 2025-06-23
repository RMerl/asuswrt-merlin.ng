/* Copyright (C) 2015-2024 Ben Collins <bcollins@maclara-llc.com>
   This file is part of the JWT C Library

   SPDX-License-Identifier:  MPL-2.0
   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <windows.h>
#include <wincrypt.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <jwt.h>

#include "jwt-private.h"
#include "config.h"

/* Routines to support crypto in LibJWT using Windows crypto APIs. */

#define CERT_THUMBPRINT_STR_LEN		40
#define CERT_THUMBPRINT_HASH_LEN	20
#define MY_ENCODING_TYPE			(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)

#define PEM_PUBLIC_KEY_HEADER		"-----BEGIN PUBLIC KEY-----"

#define CERT_STORE_PREFIX			"cert:\\"

static int
get_cert_location(
	char *uri,
	int uri_len,
	DWORD *store_name,
	wchar_t **store_path,
	CRYPT_HASH_BLOB *thumbprint)
{
	char *path;
	int path_len;
	char *sep;
	size_t store_name_len;
	size_t remainder_len;
	size_t thumbprint_len;
	char* thumbprint_str;
	char* store_path_mbs = NULL;
	size_t wchar_count;
	wchar_t* wchar_buf = NULL;
	errno_t ret;

	/* Certificate store path starts with cert:\ */
	if (strncmp(uri, CERT_STORE_PREFIX, strlen(CERT_STORE_PREFIX)))
		return EINVAL;

	path = uri + strlen(CERT_STORE_PREFIX);
	path_len = uri_len - (int)strlen(CERT_STORE_PREFIX);
	sep = memchr(path, '\\', path_len);
	if (!sep)
	{
		ret = EINVAL;
		goto get_cert_location_done;
	}

	store_name_len = sep - path;

	if (!strncmp(path, "CurrentUser", store_name_len))
		*store_name = CERT_SYSTEM_STORE_CURRENT_USER;
	else if (!strncmp(path, "LocalMachine", store_name_len))
		*store_name = CERT_SYSTEM_STORE_LOCAL_MACHINE;
	else if (!strncmp(path, "CurrentService", store_name_len))
		*store_name = CERT_SYSTEM_STORE_CURRENT_SERVICE;
	else if (!strncmp(path, "Services", store_name_len))
		*store_name = CERT_SYSTEM_STORE_SERVICES;
	else if (!strncmp(path, "Users", store_name_len))
		*store_name = CERT_SYSTEM_STORE_USERS;
	else if (!strncmp(path, "CurrentUserGroupPolicy", store_name_len))
		*store_name = CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY;
	else if (!strncmp(path, "LocalMachineGroupPolicy", store_name_len))
		*store_name = CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY;
	else if (!strncmp(path, "LocalMachineEnterprise", store_name_len))
		*store_name = CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE;
	else
	{
		ret = EINVAL;
		goto get_cert_location_done;
	}

	remainder_len = path_len - (store_name_len + 1);
	store_path_mbs = (char*)jwt_malloc(remainder_len);
	if (!store_path_mbs)
	{
		ret = ENOMEM;
		goto get_cert_location_done;
	}

	if (memcpy_s(store_path_mbs, remainder_len, sep + 1, remainder_len))
	{
		ret = EINVAL;
		goto get_cert_location_done;
	}

	sep = memchr(store_path_mbs, '\\', remainder_len);
	if (!sep)
	{
		ret = EINVAL;
		goto get_cert_location_done;
	}

	*sep = 0;

	thumbprint_str = sep + 1;
	thumbprint_len = remainder_len - (thumbprint_str - store_path_mbs);
	if (thumbprint_len != CERT_THUMBPRINT_STR_LEN)
	{
		ret = EINVAL;
		goto get_cert_location_done;
	}

	if (!CryptStringToBinaryA(
		thumbprint_str,
		CERT_THUMBPRINT_STR_LEN,
		CRYPT_STRING_HEX,
		thumbprint->pbData,
		&thumbprint->cbData,
		NULL,
		NULL))
	{
		ret = EINVAL;
		goto get_cert_location_done;
	}

	/* Convert store_name to wide chars, which cert API requires. */
	if (mbstowcs_s(&wchar_count, NULL, 0, store_path_mbs, 0))
	{
		ret = EINVAL;
		goto get_cert_location_done;
	}

	wchar_buf = (wchar_t*)jwt_malloc(wchar_count * sizeof(wchar_t));
	if (!wchar_buf)
	{
		ret = ENOMEM;
		goto get_cert_location_done;
	}

	if (mbstowcs_s(NULL, wchar_buf, wchar_count, store_path_mbs, _TRUNCATE))
	{
		ret = EINVAL;
		goto get_cert_location_done;
	}

	*store_path = wchar_buf;
	ret = 0;

get_cert_location_done:
	if (store_path_mbs)
		jwt_freemem(store_path_mbs);
	if (ret && wchar_buf)
		jwt_freemem(wchar_buf);

	return ret;
}

#define OPEN_PRIVATE_STORE_ERROR(__err) { ret = __err; goto open_private_key_from_store_done; }

static int open_private_key_from_store(
	unsigned char* key,
	int key_len,
	HCERTSTORE* hCertStore_out,
	PCCERT_CONTEXT* pSignerCert_out,
	NCRYPT_PROV_HANDLE* hStorageProv_out,
	NCRYPT_KEY_HANDLE* hKey_out)
{
	int ret = EINVAL;
	DWORD store_name;
	wchar_t* store_path = NULL;
	CRYPT_HASH_BLOB thumbprint;
	BYTE thumbprint_buf[CERT_THUMBPRINT_HASH_LEN];
	DWORD keyProvInfoSize;
	HCERTSTORE hCertStore = NULL;
	PCCERT_CONTEXT pSignerCert = NULL;
	CRYPT_KEY_PROV_INFO *pKeyProvInfo = NULL;
	DWORD dwKeyFlags;
	NCRYPT_PROV_HANDLE hStorageProv = (NCRYPT_PROV_HANDLE)NULL;
	NCRYPT_KEY_HANDLE hKey = (NCRYPT_KEY_HANDLE)NULL;

	*hCertStore_out = NULL;
	*pSignerCert_out = NULL;
	*hStorageProv_out = (NCRYPT_PROV_HANDLE)NULL;
	*hKey_out = (NCRYPT_KEY_HANDLE)NULL;

	/* Break down cert path. */
	thumbprint.pbData = thumbprint_buf;
	thumbprint.cbData = CERT_THUMBPRINT_HASH_LEN;
	ret = get_cert_location(key, key_len, &store_name, &store_path, &thumbprint);
	if (ret)
		goto open_private_key_from_store_done;

	/* Open the certificate store. */
	if (!(hCertStore = CertOpenStore(
		CERT_STORE_PROV_SYSTEM,
		0,
		(HCRYPTPROV_LEGACY)NULL,
		store_name | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG,
		store_path)))
		OPEN_PRIVATE_STORE_ERROR(ENOENT);

	/*
	 * Get a pointer to the signer's certificate.
	 * This certificate must have access to the signer's private key.
	 */
	if (!(pSignerCert = CertFindCertificateInStore(
		hCertStore,
		MY_ENCODING_TYPE,
		0,
		CERT_FIND_HASH,
		&thumbprint,
		NULL)))
		OPEN_PRIVATE_STORE_ERROR(ENOENT);

	/* Get private key provider info. */
	if (!(CertGetCertificateContextProperty(
		pSignerCert,
		CERT_KEY_PROV_INFO_PROP_ID,
		NULL,
		&keyProvInfoSize)))
		OPEN_PRIVATE_STORE_ERROR(ENOENT);

	if (!(pKeyProvInfo = (CRYPT_KEY_PROV_INFO *)jwt_malloc(keyProvInfoSize)))
		OPEN_PRIVATE_STORE_ERROR(ENOMEM);

	if (!CertGetCertificateContextProperty(
		pSignerCert,
		CERT_KEY_PROV_INFO_PROP_ID,
		pKeyProvInfo,
		&keyProvInfoSize))
		OPEN_PRIVATE_STORE_ERROR(ENOENT);

	/* Open key using NCrypt. */
	if (NCryptOpenStorageProvider(
		&hStorageProv,
		pKeyProvInfo->pwszProvName,
		0) != ERROR_SUCCESS)
		OPEN_PRIVATE_STORE_ERROR(ENOENT);

	dwKeyFlags = NCRYPT_SILENT_FLAG |
		((store_name == CERT_SYSTEM_STORE_LOCAL_MACHINE) ? NCRYPT_MACHINE_KEY_FLAG : 0);
	if (NCryptOpenKey(
		hStorageProv,
		&hKey,
		pKeyProvInfo->pwszContainerName,
		0,
		dwKeyFlags) != ERROR_SUCCESS)
		OPEN_PRIVATE_STORE_ERROR(ENOENT);

	ret = 0;

open_private_key_from_store_done:
	if (store_path)
		jwt_freemem(store_path);

	if (pKeyProvInfo)
		jwt_freemem(pKeyProvInfo);

	if (ret)
	{
		if (pSignerCert)
			CertFreeCertificateContext(pSignerCert);

		if (hCertStore)
			CertCloseStore(hCertStore, CERT_CLOSE_STORE_CHECK_FLAG);

		if (hKey)
			NCryptFreeObject(hKey);

		if (hStorageProv)
			NCryptFreeObject(hStorageProv);
	}
	else
	{
		*pSignerCert_out = pSignerCert;
		*hCertStore_out = hCertStore;
		*hKey_out = hKey;
		*hStorageProv_out = hStorageProv;
	}

	return ret;
}

#define OPEN_PUBLIC_STORE_ERROR(__err) { ret = __err; goto open_public_key_from_store_done; }

static int open_public_key_from_store(
	unsigned char* key,
	int key_len,
	HCERTSTORE* hCertStore_out,
	PCCERT_CONTEXT* pSignerCert_out,
	BCRYPT_KEY_HANDLE* hKey_out)
{
	int ret = EINVAL;
	DWORD store_name;
	wchar_t* store_path = NULL;
	CRYPT_HASH_BLOB thumbprint;
	BYTE thumbprint_buf[CERT_THUMBPRINT_HASH_LEN];
	HCERTSTORE hCertStore = NULL;
	PCCERT_CONTEXT pSignerCert = NULL;
	BCRYPT_KEY_HANDLE hKey = (BCRYPT_KEY_HANDLE)NULL;

	*hCertStore_out = NULL;
	*pSignerCert_out = NULL;
	*hKey_out = (BCRYPT_KEY_HANDLE)NULL;

	/* Break down cert path. */
	thumbprint.pbData = thumbprint_buf;
	thumbprint.cbData = CERT_THUMBPRINT_HASH_LEN;
	ret = get_cert_location(key, key_len, &store_name, &store_path, &thumbprint);
	if (ret)
		goto open_public_key_from_store_done;

	/* Open the certificate store. */
	if (!(hCertStore = CertOpenStore(
		CERT_STORE_PROV_SYSTEM,
		0,
		(HCRYPTPROV_LEGACY)NULL,
		store_name | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG,
		store_path)))
		OPEN_PUBLIC_STORE_ERROR(ENOENT);

	/*
	 * Get a pointer to the signer's certificate.
	 * This certificate must have access to the signer's private key.
	 */
	if (!(pSignerCert = CertFindCertificateInStore(
		hCertStore,
		MY_ENCODING_TYPE,
		0,
		CERT_FIND_HASH,
		&thumbprint,
		NULL)))
		OPEN_PUBLIC_STORE_ERROR(ENOENT);

	if (!CryptImportPublicKeyInfoEx2(X509_ASN_ENCODING, &pSignerCert->pCertInfo->SubjectPublicKeyInfo, 0, NULL, &hKey))
		OPEN_PUBLIC_STORE_ERROR(EINVAL);

	ret = 0;

open_public_key_from_store_done:
	if (store_path)
		jwt_freemem(store_path);

	if (ret)
	{
		if (pSignerCert)
			CertFreeCertificateContext(pSignerCert);

		if (hCertStore)
			CertCloseStore(hCertStore, CERT_CLOSE_STORE_CHECK_FLAG);

		if (hKey)
			BCryptDestroyKey(hKey);
	}
	else
	{
		*pSignerCert_out = pSignerCert;
		*hCertStore_out = hCertStore;
		*hKey_out = hKey;
	}

	return ret;
}

#define OPEN_PUBLIC_PEM_ERROR(__err) { ret = __err; goto open_public_key_from_pem_done; }

static int open_public_key_from_pem(
	unsigned char* key,
	int key_len,
	BCRYPT_KEY_HANDLE* hKey_out)
{
	int ret = EINVAL;
	BYTE* pbPublicKeyDer = NULL;
	DWORD cbPublicKeyDer;
	PCERT_PUBLIC_KEY_INFO pbPublicKeyInfo = NULL;
	DWORD cbPublicKeyInfo;
	BCRYPT_KEY_HANDLE hKey = (BCRYPT_KEY_HANDLE)NULL;

	/* Convert public key from PEM to DER. */
	if (!CryptStringToBinaryA(
		key,
		key_len,
		CRYPT_STRING_BASE64HEADER,
		NULL,
		&cbPublicKeyDer,
		NULL,
		NULL))
		OPEN_PUBLIC_PEM_ERROR(EINVAL);

	if (!(pbPublicKeyDer = (BYTE *)jwt_malloc(cbPublicKeyDer)))
		OPEN_PUBLIC_PEM_ERROR(ENOMEM);

	if (!CryptStringToBinaryA(
		key,
		key_len,
		CRYPT_STRING_BASE64HEADER,
		pbPublicKeyDer,
		&cbPublicKeyDer,
		NULL,
		NULL))
		OPEN_PUBLIC_PEM_ERROR(EINVAL);

	/* Create public key info. */
	if (!CryptDecodeObjectEx(
		X509_ASN_ENCODING,
		X509_PUBLIC_KEY_INFO,
		pbPublicKeyDer,
		cbPublicKeyDer,
		0,
		NULL,
		NULL,
		&cbPublicKeyInfo))
		OPEN_PUBLIC_PEM_ERROR(EINVAL);

	if (!(pbPublicKeyInfo = (PCERT_PUBLIC_KEY_INFO)jwt_malloc(cbPublicKeyInfo)))
		OPEN_PUBLIC_PEM_ERROR(ENOMEM);

	if (!CryptDecodeObjectEx(
		X509_ASN_ENCODING,
		X509_PUBLIC_KEY_INFO,
		pbPublicKeyDer,
		cbPublicKeyDer,
		0,
		NULL,
		pbPublicKeyInfo,
		&cbPublicKeyInfo))
		OPEN_PUBLIC_PEM_ERROR(EINVAL);

	/* Import public key into CNG. */
	if (!CryptImportPublicKeyInfoEx2(
		X509_ASN_ENCODING,
		pbPublicKeyInfo,
		0,
		NULL,
		&hKey))
		OPEN_PUBLIC_PEM_ERROR(EINVAL);

	*hKey_out = hKey;
	ret = 0;

open_public_key_from_pem_done:
	if (pbPublicKeyDer)
		jwt_freemem(pbPublicKeyDer);

	if (pbPublicKeyInfo)
		jwt_freemem(pbPublicKeyInfo);

	if (ret)
	{
		if (hKey)
			BCryptDestroyKey(hKey);
	}
	else
	{
		*hKey_out = hKey;
	}

	return ret;
}

static int is_public_key_pem(unsigned char* key, int key_len)
{
	if (key_len < strlen(PEM_PUBLIC_KEY_HEADER))
		return 0;

	return !memcmp(key, PEM_PUBLIC_KEY_HEADER, strlen(PEM_PUBLIC_KEY_HEADER));
}

#define SIGN_HMAC_ERROR(__err) { ret = __err; goto jwt_sign_sha_hmac_done; }

int jwt_sign_sha_hmac(jwt_t *jwt, char **out, unsigned int *len,
		      const char *str)
{
	int ret = EINVAL;
	LPCWSTR alg;
	BCRYPT_ALG_HANDLE hAlg = (BCRYPT_ALG_HANDLE)NULL;
	BCRYPT_HASH_HANDLE hHash = (BCRYPT_HASH_HANDLE)NULL;
	BYTE* pbHashObject = NULL;
	DWORD cbHashObject;
	BYTE* pbHashValue = NULL;
	DWORD cbHashValue;
	DWORD cbDummy;

	/* Lookup algorithm type. */
	switch (jwt->alg) {
	/* HMAC */
	case JWT_ALG_HS256:
		alg = BCRYPT_SHA256_ALGORITHM;
		break;
	case JWT_ALG_HS384:
		alg = BCRYPT_SHA384_ALGORITHM;
		break;
	case JWT_ALG_HS512:
		alg = BCRYPT_SHA512_ALGORITHM;
		break;
	default:
		/* For now, do not support ECC on Windows */
		SIGN_HMAC_ERROR(EINVAL);
	}

	/* Hash the JWT using the selected algorithm, and with MAC. */
	if (BCryptOpenAlgorithmProvider(
		&hAlg,
		alg,
		NULL,
		BCRYPT_ALG_HANDLE_HMAC_FLAG) != ERROR_SUCCESS)
		SIGN_HMAC_ERROR(EINVAL);

	if (BCryptGetProperty(
		hAlg,
		BCRYPT_OBJECT_LENGTH,
		(PBYTE)&cbHashObject,
		sizeof(DWORD),
		&cbDummy,
		0) != ERROR_SUCCESS)
		SIGN_HMAC_ERROR(EINVAL);

	if (!(pbHashObject = (BYTE*)jwt_malloc(cbHashObject)))
		SIGN_HMAC_ERROR(ENOMEM);

	if (BCryptCreateHash(
		hAlg,
		&hHash,
		pbHashObject,
		cbHashObject,
		jwt->key,
		jwt->key_len,
		0) != ERROR_SUCCESS)
		SIGN_HMAC_ERROR(EINVAL);

	if (BCryptGetProperty(
		hAlg,
		BCRYPT_HASH_LENGTH,
		(PBYTE)&cbHashValue,
		sizeof(DWORD),
		&cbDummy,
		0) != ERROR_SUCCESS)
		SIGN_HMAC_ERROR(EINVAL);

	if (!(pbHashValue = (BYTE*)jwt_malloc(cbHashValue)))
		SIGN_HMAC_ERROR(ENOMEM);

	if (BCryptHashData(
		hHash,
		(PUCHAR)str,
		(ULONG)strlen(str),
		0) != ERROR_SUCCESS)
		SIGN_HMAC_ERROR(EINVAL);

	if (BCryptFinishHash(
		hHash,
		pbHashValue,
		cbHashValue,
		0) != ERROR_SUCCESS)
		SIGN_HMAC_ERROR(EINVAL);

	/* Done! */
	*out = pbHashValue;
	*len = cbHashValue;
	ret = 0;

jwt_sign_sha_hmac_done:
	if (hHash)
		BCryptDestroyHash(hHash);

	if (hAlg)
		BCryptCloseAlgorithmProvider(hAlg, 0);

	if (pbHashObject)
		jwt_freemem(pbHashObject);

	/* Only free result string if function failed. */
	if (ret && pbHashValue)
		jwt_freemem(pbHashValue);

	return ret;
}

#define VERIFY_HMAC_ERROR(__err) { ret = __err; goto jwt_verify_hmac_done; }

int jwt_verify_sha_hmac(jwt_t *jwt, const char *head, const char *sig)
{
	int ret;
	char* pbHash = NULL;
	unsigned int cbHash;
	char* pbB64 = NULL;
	DWORD cbB64;

	/* Compute the HMAC on the "head" string. */
	ret = jwt_sign_sha_hmac(jwt, &pbHash, &cbHash, head);
	if (ret)
		goto jwt_verify_hmac_done;

	/* Encode as Base64. */
	if (!CryptBinaryToStringA(
		pbHash,
		cbHash,
		CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
		NULL,
		&cbB64))
		VERIFY_HMAC_ERROR(EINVAL);

	/* Null terminator is already included in base64Size. */
	pbB64 = (char*)jwt_malloc(cbB64);
	if (!pbB64)
		VERIFY_HMAC_ERROR(ENOMEM);

	/* Get the actual value. */
	if (!CryptBinaryToStringA(
		pbHash,
		cbHash,
		CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
		pbB64,
		&cbB64))
		VERIFY_HMAC_ERROR(EINVAL);

	/* URI encode. */
	jwt_base64uri_encode(pbB64);

	/* And now... */
	ret = jwt_strcmp(pbB64, sig) ? EINVAL : 0;

jwt_verify_hmac_done:
	if (pbHash)
		jwt_freemem(pbHash);

	if (pbB64)
		jwt_freemem(pbB64);

	return ret;
}

#define SIGN_PEM_ERROR(__err) { ret = __err; goto jwt_sign_sha_pem_done; }

int jwt_sign_sha_pem(jwt_t *jwt, char **out, unsigned int *len,
		     const char *str)
{
	int ret = EINVAL;
	LPCWSTR alg;
	int isECDSA = 0;
	HCERTSTORE hCertStore = NULL;   
	PCCERT_CONTEXT pSignerCert = NULL; 
	NCRYPT_PROV_HANDLE hStorageProv = (NCRYPT_PROV_HANDLE)NULL;
	NCRYPT_KEY_HANDLE hStoreKey = (NCRYPT_KEY_HANDLE)NULL;
	BCRYPT_ALG_HANDLE hAlg = (BCRYPT_ALG_HANDLE)NULL;
	BCRYPT_HASH_HANDLE hHash = (BCRYPT_HASH_HANDLE)NULL;
	BYTE* pbHashObject = NULL;
	DWORD cbHashObject;
	BYTE* pbHashValue = NULL;
	DWORD cbHashValue;
	DWORD cbDummy;
	BCRYPT_PKCS1_PADDING_INFO paddingInfo;
	BYTE* pbSignature = NULL;
	DWORD cbSignature;
	DWORD ncryptSignFlags;

	/* Lookup algorithm type. */
	switch (jwt->alg) {
	/* RSA */
	case JWT_ALG_RS256:
		alg = BCRYPT_SHA256_ALGORITHM;
		isECDSA = 0;
		break;
	case JWT_ALG_RS384:
		alg = BCRYPT_SHA384_ALGORITHM;
		isECDSA = 0;
		break;
	case JWT_ALG_RS512:
		alg = BCRYPT_SHA512_ALGORITHM;
		isECDSA = 0;
		break;

	/* ECC */
	case JWT_ALG_ES256:
		alg = BCRYPT_SHA256_ALGORITHM;
		isECDSA = 1;
		break;
	case JWT_ALG_ES384:
		alg = BCRYPT_SHA384_ALGORITHM;
		isECDSA = 1;
		break;
	case JWT_ALG_ES512:
		alg = BCRYPT_SHA512_ALGORITHM;
		isECDSA = 1;
		break;

	default:
		SIGN_PEM_ERROR(EINVAL);
	}

	/*
	 * Open private key in certificate store.
	 *
	 * Note that in Windows crypto APIs, there are no obvious ways to
	 * import a generic private key from a PEM encoded string, nor is
	 * it desirable on Windows platforms to have private keys outside
	 * of the Windows certificate store. Therefore, only support keys
	 * in the certificate store.
	 */
	ret = open_private_key_from_store(
		jwt->key,
		jwt->key_len,
		&hCertStore,
		&pSignerCert,
		&hStorageProv,
		&hStoreKey);
	if (ret)
		goto jwt_sign_sha_pem_done;

	/* Hash the JWT using the selected algorithm. */
	if (BCryptOpenAlgorithmProvider(
		&hAlg,
		alg,
		NULL,
		0) != ERROR_SUCCESS)
		SIGN_PEM_ERROR(EINVAL);

	if (BCryptGetProperty(
		hAlg,
		BCRYPT_OBJECT_LENGTH,
		(PBYTE)&cbHashObject,
		sizeof(DWORD),
		&cbDummy,
		0) != ERROR_SUCCESS)
		SIGN_PEM_ERROR(EINVAL);

	if (!(pbHashObject = (BYTE*)jwt_malloc(cbHashObject)))
		SIGN_PEM_ERROR(ENOMEM);

	if (BCryptCreateHash(
		hAlg,
		&hHash,
		pbHashObject,
		cbHashObject,
		NULL,
		0,
		0) != ERROR_SUCCESS)
		SIGN_PEM_ERROR(EINVAL);

	if (BCryptGetProperty(
		hAlg,
		BCRYPT_HASH_LENGTH,
		(PBYTE)&cbHashValue,
		sizeof(DWORD),
		&cbDummy,
		0) != ERROR_SUCCESS)
		SIGN_PEM_ERROR(EINVAL);

	if (!(pbHashValue = (BYTE*)jwt_malloc(cbHashValue)))
		SIGN_PEM_ERROR(ENOMEM);

	if (BCryptHashData(
		hHash,
		(PUCHAR)str,
		(ULONG)strlen(str),
		0) != ERROR_SUCCESS)
		SIGN_PEM_ERROR(EINVAL);

	if (BCryptFinishHash(
		hHash,
		pbHashValue,
		cbHashValue,
		0) != ERROR_SUCCESS)
		SIGN_PEM_ERROR(EINVAL);

	/* Sign the hash using the private key. */
	ncryptSignFlags = NCRYPT_SILENT_FLAG | (isECDSA ? 0 : NCRYPT_PAD_PKCS1_FLAG);
	paddingInfo.pszAlgId = alg;
	if (NCryptSignHash(
		hStoreKey,
		(isECDSA ? NULL : &paddingInfo),
		pbHashValue,
		cbHashValue,
		NULL,
		0,
		&cbSignature,
		ncryptSignFlags) != ERROR_SUCCESS)
		SIGN_PEM_ERROR(EINVAL);

	if (!(pbSignature = (BYTE*)jwt_malloc(cbSignature)))
		SIGN_PEM_ERROR(ENOMEM);

	paddingInfo.pszAlgId = alg;
	if (NCryptSignHash(
		hStoreKey,
		(isECDSA ? NULL : &paddingInfo),
		pbHashValue,
		cbHashValue,
		pbSignature,
		cbSignature,
		&cbDummy,
		ncryptSignFlags) != ERROR_SUCCESS)
		SIGN_PEM_ERROR(EINVAL);

	/* Done! */
	*out = pbSignature;
	*len = cbSignature;
	ret = 0;

jwt_sign_sha_pem_done:
	if (hHash)
		BCryptDestroyHash(hHash);

	if (hAlg)
		BCryptCloseAlgorithmProvider(hAlg, 0);

	if (hStoreKey)
		NCryptFreeObject(hStoreKey);

	if (hStorageProv)
		NCryptFreeObject(hStorageProv);

	if (pSignerCert)
		CertFreeCertificateContext(pSignerCert);

	if (hCertStore)
		CertCloseStore(hCertStore, CERT_CLOSE_STORE_CHECK_FLAG);

	if (pbHashObject)
		jwt_freemem(pbHashObject);

	if (pbHashValue)
		jwt_freemem(pbHashValue);

	/* Only free result string if function failed. */
	if (ret && pbSignature)
		jwt_freemem(pbSignature);
	
	return ret;
}

#define VERIFY_PEM_ERROR(__err) { ret = __err; goto jwt_verify_sha_pem_done; }

int jwt_verify_sha_pem(jwt_t *jwt, const char *head, const char *sig_b64)
{
	int ret = EINVAL;
	LPCWSTR alg;
	int isECDSA = 0;
	BYTE* pbSignature = NULL;
	DWORD cbSignature;
	HCERTSTORE hCertStore = NULL;
	PCCERT_CONTEXT pSignerCert = NULL;
	BCRYPT_ALG_HANDLE hHashAlg = (BCRYPT_ALG_HANDLE)NULL;
	BCRYPT_KEY_HANDLE hKey = (BCRYPT_KEY_HANDLE)NULL;
	BCRYPT_HASH_HANDLE hHash = (BCRYPT_HASH_HANDLE)NULL;
	BYTE* pbHashObject = NULL;
	DWORD cbHashObject;
	BYTE* pbHashValue = NULL;
	DWORD cbHashValue;
	DWORD cbDummy;
	BCRYPT_PKCS1_PADDING_INFO paddingInfo;

	/* Lookup algorithm type. */
	switch (jwt->alg) {
	/* RSA */
	case JWT_ALG_RS256:
		alg = BCRYPT_SHA256_ALGORITHM;
		isECDSA = 0;
		break;
	case JWT_ALG_RS384:
		alg = BCRYPT_SHA384_ALGORITHM;
		isECDSA = 0;
		break;
	case JWT_ALG_RS512:
		alg = BCRYPT_SHA512_ALGORITHM;
		isECDSA = 0;
		break;

	/* ECC */
	case JWT_ALG_ES256:
		alg = BCRYPT_SHA256_ALGORITHM;
		isECDSA = 1;
		break;
	case JWT_ALG_ES384:
		alg = BCRYPT_SHA384_ALGORITHM;
		isECDSA = 1;
		break;
	case JWT_ALG_ES512:
		alg = BCRYPT_SHA512_ALGORITHM;
		isECDSA = 1;
		break;

	default:
		VERIFY_PEM_ERROR(EINVAL);
	}

	/* Decode signature. */
	if (!(pbSignature = jwt_b64_decode(sig_b64, &cbSignature)))
		VERIFY_PEM_ERROR(EINVAL);

	/* Open handle to public key. */
	if (is_public_key_pem(jwt->key, jwt->key_len))
	{
		ret = open_public_key_from_pem(
			jwt->key,
			jwt->key_len,
			&hKey);
	}
	else
	{
		ret = open_public_key_from_store(
			jwt->key,
			jwt->key_len,
			&hCertStore,
			&pSignerCert,
			&hKey);
	}
	if (ret)
		goto jwt_verify_sha_pem_done;

	/* Hash data. */
	if (BCryptOpenAlgorithmProvider(
		&hHashAlg,
		alg,
		NULL,
		0) != ERROR_SUCCESS)
		VERIFY_PEM_ERROR(EINVAL);

	if (BCryptGetProperty(
		hHashAlg,
		BCRYPT_OBJECT_LENGTH,
		(PBYTE)&cbHashObject,
		sizeof(DWORD),
		&cbDummy,
		0) != ERROR_SUCCESS)
		VERIFY_PEM_ERROR(EINVAL);

	if (!(pbHashObject = (BYTE*)jwt_malloc(cbHashObject)))
		VERIFY_PEM_ERROR(ENOMEM);

	if (BCryptCreateHash(
		hHashAlg,
		&hHash,
		pbHashObject,
		cbHashObject,
		NULL,
		0,
		0) != ERROR_SUCCESS)
		VERIFY_PEM_ERROR(EINVAL);

	if (BCryptGetProperty(
		hHashAlg,
		BCRYPT_HASH_LENGTH,
		(PBYTE)&cbHashValue,
		sizeof(DWORD),
		&cbDummy,
		0) != ERROR_SUCCESS)
		VERIFY_PEM_ERROR(EINVAL);

	if (!(pbHashValue = (BYTE*)jwt_malloc(cbHashValue)))
		VERIFY_PEM_ERROR(ENOMEM);

	if (BCryptHashData(
		hHash,
		(PUCHAR)head,
		(ULONG)strlen(head),
		0) != ERROR_SUCCESS)
		VERIFY_PEM_ERROR(EINVAL);

	if (BCryptFinishHash(
		hHash,
		pbHashValue,
		cbHashValue,
		0) != ERROR_SUCCESS)
		VERIFY_PEM_ERROR(EINVAL);

	/* Verify hash against signature. */
	paddingInfo.pszAlgId = alg;
	if (BCryptVerifySignature(
		hKey,
		(isECDSA ? NULL : &paddingInfo),
		pbHashValue,
		cbHashValue,
		pbSignature,
		cbSignature,
		(isECDSA ? 0 : BCRYPT_PAD_PKCS1)) != ERROR_SUCCESS)
		VERIFY_PEM_ERROR(EINVAL);

	/* Signature valid! */
	ret = 0;

jwt_verify_sha_pem_done:
	if (pbSignature)
		jwt_freemem(pbSignature);

	if (pbHashObject)
		jwt_freemem(pbHashObject);

	if (pbHashValue)
		jwt_freemem(pbHashValue);

	if (hHash)
		BCryptDestroyHash(hHash);

	if (hKey)
		BCryptDestroyKey(hKey);

	if (hHashAlg)
		BCryptCloseAlgorithmProvider(hHashAlg, 0);

	if (pSignerCert)
		CertFreeCertificateContext(pSignerCert);

	if (hCertStore)
		CertCloseStore(hCertStore, CERT_CLOSE_STORE_CHECK_FLAG);

	return ret;
}
