/*
 * Copyright (C) 2011-2015 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "pkcs11_library.h"

#include <dlfcn.h>

#include <library.h>
#include <asn1/asn1.h>
#include <utils/debug.h>
#include <threading/mutex.h>
#include <collections/linked_list.h>

typedef struct private_pkcs11_library_t private_pkcs11_library_t;


ENUM_BEGIN(ck_rv_names, CKR_OK, CKR_CANT_LOCK,
	"OK",
	"CANCEL",
	"HOST_MEMORY",
	"SLOT_ID_INVALID",
	"(0x04)",
	"GENERAL_ERROR",
	"FUNCTION_FAILED",
	"ARGUMENTS_BAD",
	"NO_EVENT",
	"NEED_TO_CREATE_THREADS",
	"CANT_LOCK");
ENUM_NEXT(ck_rv_names, CKR_ATTRIBUTE_READ_ONLY, CKR_ATTRIBUTE_VALUE_INVALID,
		CKR_CANT_LOCK,
	"ATTRIBUTE_READ_ONLY",
	"ATTRIBUTE_SENSITIVE",
	"ATTRIBUTE_TYPE_INVALID",
	"ATTRIBUTE_VALUE_INVALID");
ENUM_NEXT(ck_rv_names, CKR_DATA_INVALID, CKR_DATA_LEN_RANGE,
		CKR_ATTRIBUTE_VALUE_INVALID,
	"DATA_INVALID"
	"DATA_LEN_RANGE");
ENUM_NEXT(ck_rv_names, CKR_DEVICE_ERROR, CKR_DEVICE_REMOVED,
		CKR_DATA_LEN_RANGE,
	"DEVICE_ERROR",
	"DEVICE_MEMORY",
	"DEVICE_REMOVED");
ENUM_NEXT(ck_rv_names, CKR_ENCRYPTED_DATA_INVALID, CKR_ENCRYPTED_DATA_LEN_RANGE,
		CKR_DEVICE_REMOVED,
	"ENCRYPTED_DATA_INVALID",
	"ENCRYPTED_DATA_LEN_RANGE");
ENUM_NEXT(ck_rv_names, CKR_FUNCTION_CANCELED, CKR_FUNCTION_NOT_SUPPORTED,
		CKR_ENCRYPTED_DATA_LEN_RANGE,
	"FUNCTION_CANCELED",
	"FUNCTION_NOT_PARALLEL",
	"(0x52)",
	"(0x53)",
	"FUNCTION_NOT_SUPPORTED");
ENUM_NEXT(ck_rv_names, CKR_KEY_HANDLE_INVALID, CKR_KEY_UNEXTRACTABLE,
		CKR_FUNCTION_NOT_SUPPORTED,
	"KEY_HANDLE_INVALID",
	"(0x61)",
	"KEY_SIZE_RANGE",
	"KEY_TYPE_INCONSISTENT",
	"KEY_NOT_NEEDED",
	"KEY_CHANGED",
	"KEY_NEEDED",
	"KEY_INDIGESTIBLE",
	"KEY_FUNCTION_NOT_PERMITTED",
	"KEY_NOT_WRAPPABLE",
	"KEY_UNEXTRACTABLE");
ENUM_NEXT(ck_rv_names, CKR_MECHANISM_INVALID, CKR_MECHANISM_PARAM_INVALID,
		CKR_KEY_UNEXTRACTABLE,
	"MECHANISM_INVALID",
	"MECHANISM_PARAM_INVALID");
ENUM_NEXT(ck_rv_names, CKR_OBJECT_HANDLE_INVALID, CKR_OBJECT_HANDLE_INVALID,
		CKR_MECHANISM_PARAM_INVALID,
	"OBJECT_HANDLE_INVALID");
ENUM_NEXT(ck_rv_names, CKR_OPERATION_ACTIVE, CKR_OPERATION_NOT_INITIALIZED,
		CKR_OBJECT_HANDLE_INVALID,
	"OPERATION_ACTIVE",
	"OPERATION_NOT_INITIALIZED");
ENUM_NEXT(ck_rv_names, CKR_PIN_INCORRECT, CKR_PIN_LOCKED,
		CKR_OPERATION_NOT_INITIALIZED,
	"PIN_INCORRECT",
	"PIN_INVALID",
	"PIN_LEN_RANGE",
	"PIN_EXPIRED",
	"PIN_LOCKED");
ENUM_NEXT(ck_rv_names, CKR_SESSION_CLOSED, CKR_SESSION_READ_WRITE_SO_EXISTS,
		CKR_PIN_LOCKED,
	"SESSION_CLOSED",
	"SESSION_COUNT",
	"(0xb2)",
	"SESSION_HANDLE_INVALID",
	"SESSION_PARALLEL_NOT_SUPPORTED",
	"SESSION_READ_ONLY",
	"SESSION_EXISTS",
	"SESSION_READ_ONLY_EXISTS",
	"SESSION_READ_WRITE_SO_EXISTS");
ENUM_NEXT(ck_rv_names, CKR_SIGNATURE_INVALID, CKR_SIGNATURE_LEN_RANGE,
		CKR_SESSION_READ_WRITE_SO_EXISTS,
	"SIGNATURE_INVALID",
	"SIGNATURE_LEN_RANGE");
ENUM_NEXT(ck_rv_names, CKR_TEMPLATE_INCOMPLETE, CKR_TEMPLATE_INCONSISTENT,
		CKR_SIGNATURE_LEN_RANGE,
	"TEMPLATE_INCOMPLETE",
	"TEMPLATE_INCONSISTENT",
);
ENUM_NEXT(ck_rv_names, CKR_TOKEN_NOT_PRESENT, CKR_TOKEN_WRITE_PROTECTED,
		CKR_TEMPLATE_INCONSISTENT,
	"TOKEN_NOT_PRESENT",
	"TOKEN_NOT_RECOGNIZED",
	"TOKEN_WRITE_PROTECTED");
ENUM_NEXT(ck_rv_names, CKR_UNWRAPPING_KEY_HANDLE_INVALID, CKR_UNWRAPPING_KEY_TYPE_INCONSISTENT,
		CKR_TOKEN_WRITE_PROTECTED,
	"UNWRAPPING_KEY_HANDLE_INVALID",
	"UNWRAPPING_KEY_SIZE_RANGE",
	"UNWRAPPING_KEY_TYPE_INCONSISTENT");
ENUM_NEXT(ck_rv_names, CKR_USER_ALREADY_LOGGED_IN, CKR_USER_TOO_MANY_TYPES,
		CKR_UNWRAPPING_KEY_TYPE_INCONSISTENT,
	"USER_ALREADY_LOGGED_IN",
	"USER_NOT_LOGGED_IN",
	"USER_PIN_NOT_INITIALIZED",
	"USER_TYPE_INVALID",
	"USER_ANOTHER_ALREADY_LOGGED_IN",
	"USER_TOO_MANY_TYPES");
ENUM_NEXT(ck_rv_names, CKR_WRAPPED_KEY_INVALID, CKR_WRAPPING_KEY_TYPE_INCONSISTENT,
		CKR_USER_TOO_MANY_TYPES,
	"WRAPPED_KEY_INVALID",
	"(0x111)",
	"WRAPPED_KEY_LEN_RANGE",
	"WRAPPING_KEY_HANDLE_INVALID",
	"WRAPPING_KEY_SIZE_RANGE",
	"WRAPPING_KEY_TYPE_INCONSISTENT");
ENUM_NEXT(ck_rv_names, CKR_RANDOM_SEED_NOT_SUPPORTED, CKR_RANDOM_NO_RNG,
		CKR_WRAPPING_KEY_TYPE_INCONSISTENT,
	"RANDOM_SEED_NOT_SUPPORTED",
	"RANDOM_NO_RNG");
ENUM_NEXT(ck_rv_names, CKR_DOMAIN_PARAMS_INVALID, CKR_DOMAIN_PARAMS_INVALID,
		CKR_RANDOM_NO_RNG,
	"DOMAIN_PARAMS_INVALID");
ENUM_NEXT(ck_rv_names, CKR_BUFFER_TOO_SMALL, CKR_BUFFER_TOO_SMALL,
		CKR_DOMAIN_PARAMS_INVALID,
	"BUFFER_TOO_SMALL");
ENUM_NEXT(ck_rv_names, CKR_SAVED_STATE_INVALID, CKR_SAVED_STATE_INVALID,
		CKR_BUFFER_TOO_SMALL,
	"SAVED_STATE_INVALID");
ENUM_NEXT(ck_rv_names, CKR_INFORMATION_SENSITIVE, CKR_INFORMATION_SENSITIVE,
		CKR_SAVED_STATE_INVALID,
	"INFORMATION_SENSITIVE");
ENUM_NEXT(ck_rv_names, CKR_STATE_UNSAVEABLE, CKR_STATE_UNSAVEABLE,
		CKR_INFORMATION_SENSITIVE,
	"STATE_UNSAVEABLE");
ENUM_NEXT(ck_rv_names, CKR_CRYPTOKI_NOT_INITIALIZED, CKR_CRYPTOKI_ALREADY_INITIALIZED,
		CKR_STATE_UNSAVEABLE,
	"CRYPTOKI_NOT_INITIALIZED",
	"CRYPTOKI_ALREADY_INITIALIZED");
ENUM_NEXT(ck_rv_names, CKR_MUTEX_BAD, CKR_MUTEX_NOT_LOCKED,
		CKR_CRYPTOKI_ALREADY_INITIALIZED,
	"MUTEX_BAD",
	"MUTEX_NOT_LOCKED");
ENUM_NEXT(ck_rv_names, CKR_FUNCTION_REJECTED, CKR_FUNCTION_REJECTED,
		CKR_MUTEX_NOT_LOCKED,
	"FUNCTION_REJECTED");
ENUM_END(ck_rv_names, CKR_FUNCTION_REJECTED);


ENUM_BEGIN(ck_mech_names, CKM_RSA_PKCS_KEY_PAIR_GEN, CKM_DSA_SHA1,
	"RSA_PKCS_KEY_PAIR_GEN",
	"RSA_PKCS",
	"RSA_9796",
	"RSA_X_509",
	"MD2_RSA_PKCS",
	"MD5_RSA_PKCS",
	"SHA1_RSA_PKCS",
	"RIPEMD128_RSA_PKCS",
	"RIPEMD160_RSA_PKCS",
	"RSA_PKCS_OAEP",
	"RSA_X9_31_KEY_PAIR_GEN",
	"RSA_X9_31",
	"SHA1_RSA_X9_31",
	"RSA_PKCS_PSS",
	"SHA1_RSA_PKCS_PSS",
	"(0xf)",
	"DSA_KEY_PAIR_GEN",
	"DSA",
	"DSA_SHA1");
ENUM_NEXT(ck_mech_names, CKM_DH_PKCS_KEY_PAIR_GEN, CKM_DH_PKCS_DERIVE,
		CKM_DSA_SHA1,
	"DH_PKCS_KEY_PAIR_GEN",
	"DH_PKCS_DERIVE");
ENUM_NEXT(ck_mech_names, CKM_X9_42_DH_KEY_PAIR_GEN, CKM_X9_42_MQV_DERIVE,
		CKM_DH_PKCS_DERIVE,
	"X9_42_DH_KEY_PAIR_GEN",
	"X9_42_DH_DERIVE",
	"X9_42_DH_HYBRID_DERIVE",
	"X9_42_MQV_DERIVE");
ENUM_NEXT(ck_mech_names, CKM_SHA256_RSA_PKCS, CKM_SHA512_RSA_PKCS_PSS,
		CKM_X9_42_MQV_DERIVE,
	"SHA256_RSA_PKCS",
	"SHA384_RSA_PKCS",
	"SHA512_RSA_PKCS",
	"SHA256_RSA_PKCS_PSS",
	"SHA384_RSA_PKCS_PSS",
	"SHA512_RSA_PKCS_PSS");
ENUM_NEXT(ck_mech_names, CKM_RC2_KEY_GEN, CKM_RC2_CBC_PAD,
		CKM_SHA512_RSA_PKCS_PSS,
	"RC2_KEY_GEN",
	"RC2_ECB",
	"RC2_CBC",
	"RC2_MAC",
	"RC2_MAC_GENERAL",
	"RC2_CBC_PAD");
ENUM_NEXT(ck_mech_names, CKM_RC4_KEY_GEN, CKM_RC4,
		CKM_RC2_CBC_PAD,
	"RC4_KEY_GEN",
	"RC4");
ENUM_NEXT(ck_mech_names, CKM_DES_KEY_GEN, CKM_DES_CBC_PAD,
		CKM_RC4,
	"DES_KEY_GEN",
	"DES_ECB",
	"DES_CBC",
	"DES_MAC",
	"DES_MAC_GENERAL",
	"DES_CBC_PAD");
ENUM_NEXT(ck_mech_names, CKM_DES2_KEY_GEN, CKM_DES3_CBC_PAD,
		CKM_DES_CBC_PAD,
	"DES2_KEY_GEN",
	"DES3_KEY_GEN",
	"DES3_ECB",
	"DES3_CBC",
	"DES3_MAC",
	"DES3_MAC_GENERAL",
	"DES3_CBC_PAD");
ENUM_NEXT(ck_mech_names, CKM_CDMF_KEY_GEN, CKM_CDMF_CBC_PAD,
		CKM_DES3_CBC_PAD,
	"CDMF_KEY_GEN",
	"CDMF_ECB",
	"CDMF_CBC",
	"CDMF_MAC",
	"CDMF_MAC_GENERAL",
	"CDMF_CBC_PAD");
ENUM_NEXT(ck_mech_names, CKM_MD2, CKM_MD2_HMAC_GENERAL,
		CKM_CDMF_CBC_PAD,
	"MD2",
	"MD2_HMAC",
	"MD2_HMAC_GENERAL");
ENUM_NEXT(ck_mech_names, CKM_MD5, CKM_MD5_HMAC_GENERAL,
		CKM_MD2_HMAC_GENERAL,
	"MD5",
	"MD5_HMAC",
	"MD5_HMAC_GENERAL");
ENUM_NEXT(ck_mech_names, CKM_SHA_1, CKM_SHA_1_HMAC_GENERAL,
		CKM_MD5_HMAC_GENERAL,
	"SHA_1",
	"SHA_1_HMAC",
	"SHA_1_HMAC_GENERAL");
ENUM_NEXT(ck_mech_names, CKM_RIPEMD128, CKM_RIPEMD128_HMAC_GENERAL,
		CKM_SHA_1_HMAC_GENERAL,
	"RIPEMD128",
	"RIPEMD128_HMAC",
	"RIPEMD128_HMAC_GENERAL");
ENUM_NEXT(ck_mech_names, CKM_RIPEMD160, CKM_RIPEMD160_HMAC_GENERAL,
		CKM_RIPEMD128_HMAC_GENERAL,
	"RIPEMD160",
	"RIPEMD160_HMAC",
	"RIPEMD160_HMAC_GENERAL");
ENUM_NEXT(ck_mech_names, CKM_SHA256, CKM_SHA256_HMAC_GENERAL,
		CKM_RIPEMD160_HMAC_GENERAL,
	"SHA256",
	"SHA256_HMAC",
	"SHA256_HMAC_GENERAL");
ENUM_NEXT(ck_mech_names, CKM_SHA384, CKM_SHA384_HMAC_GENERAL,
		CKM_SHA256_HMAC_GENERAL,
	"SHA384",
	"SHA384_HMAC",
	"SHA384_HMAC_GENERAL");
ENUM_NEXT(ck_mech_names, CKM_SHA512, CKM_SHA512_HMAC_GENERAL,
		CKM_SHA384_HMAC_GENERAL	,
	"SHA512",
	"SHA512_HMAC",
	"SHA512_HMAC_GENERAL");
ENUM_NEXT(ck_mech_names, CKM_CAST_KEY_GEN, CKM_CAST_CBC_PAD,
		CKM_SHA512_HMAC_GENERAL,
	"CAST_KEY_GEN",
	"CAST_ECB",
	"CAST_CBC",
	"CAST_MAC",
	"CAST_MAC_GENERAL",
	"CAST_CBC_PAD");
ENUM_NEXT(ck_mech_names, CKM_CAST3_KEY_GEN, CKM_CAST3_CBC_PAD,
		CKM_CAST_CBC_PAD,
	"CAST3_KEY_GEN",
	"CAST3_ECB",
	"CAST3_CBC",
	"CAST3_MAC",
	"CAST3_MAC_GENERAL",
	"CAST3_CBC_PAD");
ENUM_NEXT(ck_mech_names, CKM_CAST128_KEY_GEN, CKM_CAST128_CBC_PAD,
		CKM_CAST3_CBC_PAD,
	"CAST128_KEY_GEN",
	"CAST128_ECB",
	"CAST128_CBC",
	"CAST128_MAC",
	"CAST128_MAC_GENERAL",
	"CAST128_CBC_PAD");
ENUM_NEXT(ck_mech_names, CKM_RC5_KEY_GEN, CKM_RC5_CBC_PAD,
		CKM_CAST128_CBC_PAD,
	"RC5_KEY_GEN",
	"RC5_ECB",
	"RC5_CBC",
	"RC5_MAC",
	"RC5_MAC_GENERAL",
	"RC5_CBC_PAD");
ENUM_NEXT(ck_mech_names, CKM_IDEA_KEY_GEN, CKM_IDEA_CBC_PAD,
		CKM_RC5_CBC_PAD,
	"IDEA_KEY_GEN",
	"IDEA_ECB",
	"IDEA_CBC",
	"IDEA_MAC",
	"IDEA_MAC_GENERAL",
	"IDEA_CBC_PAD");
ENUM_NEXT(ck_mech_names, CKM_GENERIC_SECRET_KEY_GEN, CKM_GENERIC_SECRET_KEY_GEN,
		CKM_IDEA_CBC_PAD,
	"GENERIC_SECRET_KEY_GEN");
ENUM_NEXT(ck_mech_names, CKM_CONCATENATE_BASE_AND_KEY, CKM_EXTRACT_KEY_FROM_KEY,
		CKM_GENERIC_SECRET_KEY_GEN,
	"CONCATENATE_BASE_AND_KEY",
	"(0x361)",
	"CONCATENATE_BASE_AND_DATA",
	"CONCATENATE_DATA_AND_BASE",
	"XOR_BASE_AND_DATA",
	"EXTRACT_KEY_FROM_KEY");
ENUM_NEXT(ck_mech_names, CKM_SSL3_PRE_MASTER_KEY_GEN, CKM_TLS_MASTER_KEY_DERIVE_DH,
		CKM_EXTRACT_KEY_FROM_KEY,
	"SSL3_PRE_MASTER_KEY_GEN",
	"SSL3_MASTER_KEY_DERIVE",
	"SSL3_KEY_AND_MAC_DERIVE",
	"SSL3_MASTER_KEY_DERIVE_DH",
	"TLS_PRE_MASTER_KEY_GEN",
	"TLS_MASTER_KEY_DERIVE",
	"TLS_KEY_AND_MAC_DERIVE",
	"TLS_MASTER_KEY_DERIVE_DH");
ENUM_NEXT(ck_mech_names, CKM_SSL3_MD5_MAC, CKM_SSL3_SHA1_MAC,
		CKM_TLS_MASTER_KEY_DERIVE_DH,
	"SSL3_MD5_MAC",
	"SSL3_SHA1_MAC");
ENUM_NEXT(ck_mech_names, CKM_MD5_KEY_DERIVATION, CKM_SHA1_KEY_DERIVATION,
		CKM_SSL3_SHA1_MAC,
	"MD5_KEY_DERIVATION",
	"MD2_KEY_DERIVATION",
	"SHA1_KEY_DERIVATION");
ENUM_NEXT(ck_mech_names, CKM_PBE_MD2_DES_CBC, CKM_PBE_SHA1_RC2_40_CBC,
		CKM_SHA1_KEY_DERIVATION,
	"PBE_MD2_DES_CBC",
	"PBE_MD5_DES_CBC",
	"PBE_MD5_CAST_CBC",
	"PBE_MD5_CAST3_CBC",
	"PBE_MD5_CAST128_CBC",
	"PBE_SHA1_CAST128_CBC",
	"PBE_SHA1_RC4_128",
	"PBE_SHA1_RC4_40",
	"PBE_SHA1_DES3_EDE_CBC",
	"PBE_SHA1_DES2_EDE_CBC",
	"PBE_SHA1_RC2_128_CBC",
	"PBE_SHA1_RC2_40_CBC");
ENUM_NEXT(ck_mech_names, CKM_PKCS5_PBKD2, CKM_PKCS5_PBKD2,
		CKM_PBE_SHA1_RC2_40_CBC,
	"PKCS5_PBKD2");
ENUM_NEXT(ck_mech_names, CKM_PBA_SHA1_WITH_SHA1_HMAC, CKM_PBA_SHA1_WITH_SHA1_HMAC,
		CKM_PKCS5_PBKD2,
	"PBA_SHA1_WITH_SHA1_HMAC");
ENUM_NEXT(ck_mech_names, CKM_KEY_WRAP_LYNKS, CKM_KEY_WRAP_SET_OAEP,
		CKM_PBA_SHA1_WITH_SHA1_HMAC,
	"KEY_WRAP_LYNKS",
	"KEY_WRAP_SET_OAEP");
ENUM_NEXT(ck_mech_names, CKM_SKIPJACK_KEY_GEN, CKM_SKIPJACK_RELAYX,
		CKM_KEY_WRAP_SET_OAEP,
	"SKIPJACK_KEY_GEN",
	"SKIPJACK_ECB64",
	"SKIPJACK_CBC64",
	"SKIPJACK_OFB64",
	"SKIPJACK_CFB64",
	"SKIPJACK_CFB32",
	"SKIPJACK_CFB16",
	"SKIPJACK_CFB8",
	"SKIPJACK_WRAP",
	"SKIPJACK_PRIVATE_WRAP",
	"SKIPJACK_RELAYX");
ENUM_NEXT(ck_mech_names, CKM_KEA_KEY_PAIR_GEN, CKM_KEA_KEY_DERIVE,
		CKM_SKIPJACK_RELAYX,
	"KEA_KEY_PAIR_GEN",
	"KEA_KEY_DERIVE");
ENUM_NEXT(ck_mech_names, CKM_FORTEZZA_TIMESTAMP, CKM_FORTEZZA_TIMESTAMP,
		CKM_KEA_KEY_DERIVE,
	"FORTEZZA_TIMESTAMP");
ENUM_NEXT(ck_mech_names, CKM_BATON_KEY_GEN, CKM_BATON_WRAP,
		CKM_FORTEZZA_TIMESTAMP,
	"BATON_KEY_GEN",
	"BATON_ECB128",
	"BATON_ECB96",
	"BATON_CBC128",
	"BATON_COUNTER",
	"BATON_SHUFFLE",
	"BATON_WRAP");
ENUM_NEXT(ck_mech_names, CKM_ECDSA_KEY_PAIR_GEN, CKM_ECDSA_SHA1,
		CKM_BATON_WRAP,
	"ECDSA_KEY_PAIR_GEN",
	"ECDSA",
	"ECDSA_SHA1");
ENUM_NEXT(ck_mech_names, CKM_ECDH1_DERIVE, CKM_ECMQV_DERIVE,
		CKM_ECDSA_SHA1,
	"ECDH1_DERIVE",
	"ECDH1_COFACTOR_DERIVE",
	"ECMQV_DERIVE");
ENUM_NEXT(ck_mech_names, CKM_JUNIPER_KEY_GEN, CKM_JUNIPER_WRAP,
		CKM_ECMQV_DERIVE,
	"JUNIPER_KEY_GEN",
	"JUNIPER_ECB128",
	"JUNIPER_CBC128",
	"JUNIPER_COUNTER",
	"JUNIPER_SHUFFLE",
	"JUNIPER_WRAP");
ENUM_NEXT(ck_mech_names, CKM_FASTHASH, CKM_FASTHASH,
		CKM_JUNIPER_WRAP,
	"FASTHASH");
ENUM_NEXT(ck_mech_names, CKM_AES_KEY_GEN, CKM_AES_CBC_PAD,
		CKM_FASTHASH,
	"AES_KEY_GEN",
	"AES_ECB",
	"AES_CBC",
	"AES_MAC",
	"AES_MAC_GENERAL",
	"AES_CBC_PAD");
ENUM_NEXT(ck_mech_names, CKM_DSA_PARAMETER_GEN, CKM_X9_42_DH_PARAMETER_GEN,
		CKM_AES_CBC_PAD,
	"DSA_PARAMETER_GEN",
	"DH_PKCS_PARAMETER_GEN",
	"X9_42_DH_PARAMETER_GEN");
ENUM_END(ck_mech_names, CKM_X9_42_DH_PARAMETER_GEN);


ENUM_BEGIN(ck_attr_names, CKA_CLASS, CKA_LABEL,
	"CLASS",
	"TOKEN",
	"PRIVATE",
	"LABEL");
ENUM_NEXT(ck_attr_names, CKA_APPLICATION, CKA_OBJECT_ID, CKA_LABEL,
	"APPLICATION",
	"VALUE",
	"OBJECT_ID");
ENUM_NEXT(ck_attr_names, CKA_CERTIFICATE_TYPE, CKA_HASH_OF_ISSUER_PUBLIC_KEY,
		CKA_OBJECT_ID,
	"CERTIFICATE_TYPE",
	"ISSUER",
	"SERIAL_NUMBER",
	"AC_ISSUER",
	"OWNER",
	"ATTR_TYPES",
	"TRUSTED",
	"CERTIFICATE_CATEGORY",
	"JAVA_MIDP_SECURITY_DOMAIN",
	"URL",
	"HASH_OF_SUBJECT_PUBLIC_KEY",
	"HASH_OF_ISSUER_PUBLIC_KEY");
ENUM_NEXT(ck_attr_names, CKA_CHECK_VALUE, CKA_CHECK_VALUE,
		CKA_HASH_OF_ISSUER_PUBLIC_KEY,
	"CHECK_VALUE");
ENUM_NEXT(ck_attr_names, CKA_KEY_TYPE, CKA_DERIVE, CKA_CHECK_VALUE,
	"KEY_TYPE",
	"SUBJECT",
	"ID",
	"SENSITIVE",
	"ENCRYPT",
	"DECRYPT",
	"WRAP",
	"UNWRAP",
	"SIGN",
	"SIGN_RECOVER",
	"VERIFY",
	"VERIFY_RECOVER",
	"DERIVE");
ENUM_NEXT(ck_attr_names, CKA_START_DATE, CKA_END_DATE, CKA_DERIVE,
	"START_DATE",
	"END_DATE");
ENUM_NEXT(ck_attr_names, CKA_MODULUS, CKA_COEFFICIENT, CKA_END_DATE,
	"MODULUS",
	"MODULUS_BITS",
	"PUBLIC_EXPONENT",
	"PRIVATE_EXPONENT",
	"PRIME_1",
	"PRIME_2",
	"EXPONENT_1",
	"EXPONENT_2",
	"COEFFICIENT");
ENUM_NEXT(ck_attr_names, CKA_PRIME, CKA_SUB_PRIME_BITS, CKA_COEFFICIENT,
	"PRIME",
	"SUBPRIME",
	"BASE",
	"PRIME_BITS",
	"SUB_PRIME_BITS");
ENUM_NEXT(ck_attr_names, CKA_VALUE_BITS, CKA_KEY_GEN_MECHANISM,
		CKA_SUB_PRIME_BITS,
	"VALUE_BITS",
	"VALUE_LEN",
	"EXTRACTABLE",
	"LOCAL",
	"NEVER_EXTRACTABLE",
	"ALWAYS_SENSITIVE",
	"KEY_GEN_MECHANISM");
ENUM_NEXT(ck_attr_names, CKA_MODIFIABLE, CKA_MODIFIABLE, CKA_KEY_GEN_MECHANISM,
	"MODIFIABLE");
ENUM_NEXT(ck_attr_names, CKA_EC_PARAMS, CKA_EC_POINT, CKA_MODIFIABLE,
	"EC_PARAMS",
	"EC_POINT");
ENUM_NEXT(ck_attr_names, CKA_SECONDARY_AUTH, CKA_ALWAYS_AUTHENTICATE,
		CKA_EC_POINT,
	"SECONDARY_AUTH",
	"AUTH_PIN_FLAGS",
	"ALWAYS_AUTHENTICATE");
ENUM_NEXT(ck_attr_names, CKA_WRAP_WITH_TRUSTED, CKA_WRAP_WITH_TRUSTED,
		CKA_ALWAYS_AUTHENTICATE,
	"WRAP_WITH_TRUSTED");
ENUM_NEXT(ck_attr_names, CKA_HW_FEATURE_TYPE, CKA_HAS_RESET,
		CKA_WRAP_WITH_TRUSTED,
	"HW_FEATURE_TYPE",
	"RESET_ON_INIT",
	"HAS_RESET");
ENUM_NEXT(ck_attr_names, CKA_PIXEL_X, CKA_BITS_PER_PIXEL, CKA_HAS_RESET,
	"PIXEL_X",
	"RESOLUTION",
	"CHAR_ROWS",
	"CHAR_COLUMNS",
	"COLOR",
	"BITS_PER_PIXEL");
ENUM_NEXT(ck_attr_names, CKA_CHAR_SETS, CKA_MIME_TYPES, CKA_BITS_PER_PIXEL,
	"CHAR_SETS",
	"ENCODING_METHODS",
	"MIME_TYPES");
ENUM_NEXT(ck_attr_names, CKA_MECHANISM_TYPE, CKA_SUPPORTED_CMS_ATTRIBUTES,
		CKA_MIME_TYPES,
	"MECHANISM_TYPE",
	"REQUIRED_CMS_ATTRIBUTES",
	"DEFAULT_CMS_ATTRIBUTES",
	"SUPPORTED_CMS_ATTRIBUTES");
ENUM_NEXT(ck_attr_names, CKA_WRAP_TEMPLATE, CKA_UNWRAP_TEMPLATE,
		CKA_SUPPORTED_CMS_ATTRIBUTES,
	"WRAP_TEMPLATE",
	"UNWRAP_TEMPLATE");
ENUM_NEXT(ck_attr_names, CKA_ALLOWED_MECHANISMS, CKA_ALLOWED_MECHANISMS,
		CKA_UNWRAP_TEMPLATE,
	"ALLOWED_MECHANISMS");
ENUM_END(ck_attr_names, CKA_ALLOWED_MECHANISMS);
/* the values in an enum_name_t are stored as int, thus CKA_VENDOR_DEFINED
 * will overflow and is thus not defined here */



/**
 * Private data of an pkcs11_library_t object.
 */
struct private_pkcs11_library_t {

	/**
	 * Public pkcs11_library_t interface.
	 */
	pkcs11_library_t public;

	/**
	 * dlopen() handle
	 */
	void *handle;

	/**
	 * Name as passed to the constructor
	 */
	char *name;

	/**
	 * Supported feature set
	 */
	pkcs11_feature_t features;
};

METHOD(pkcs11_library_t, get_name, char*,
	private_pkcs11_library_t *this)
{
	return this->name;
}

METHOD(pkcs11_library_t, get_features, pkcs11_feature_t,
	private_pkcs11_library_t *this)
{
	return this->features;
}

/**
 * Object enumerator
 */
typedef struct {
	/* implements enumerator_t */
	enumerator_t public;
	/* session */
	CK_SESSION_HANDLE session;
	/* pkcs11 library */
	pkcs11_library_t *lib;
	/* attributes to retrieve */
	CK_ATTRIBUTE_PTR attr;
	/* number of attributes */
	CK_ULONG count;
	/* object handle in case of a single object */
	CK_OBJECT_HANDLE object;
	/* currently allocated attributes, to free */
	linked_list_t *freelist;
} object_enumerator_t;

/**
 * Free contents of attributes in a list
 */
static void free_attrs(object_enumerator_t *this)
{
	CK_ATTRIBUTE_PTR attr;

	while (this->freelist->remove_last(this->freelist, (void**)&attr) == SUCCESS)
	{
		free(attr->pValue);
		attr->pValue = NULL;
		attr->ulValueLen = 0;
	}
}

/**
 * CKA_EC_POINT is encodeed as ASN.1 octet string, we can't handle that and
 * some tokens actually return them even unwrapped.
 *
 * Because ASN1_OCTET_STRING is 0x04 and uncompressed EC_POINTs also begin with
 * 0x04 (compressed ones with 0x02 or 0x03) there will be an attempt to parse
 * unwrapped uncompressed EC_POINTs.  This will fail in most cases as the length
 * will not be correct, however, there is a small chance that the key's first
 * byte denotes the correct length.  Checking the first byte of the key should
 * further reduce the risk of false positives, though.
 *
 * The original memory is freed if the value is unwrapped.
 */
static void unwrap_ec_point(chunk_t *data)
{
	chunk_t wrapped, unwrapped;

	wrapped = unwrapped = *data;
	if (asn1_unwrap(&unwrapped, &unwrapped) == ASN1_OCTET_STRING &&
		unwrapped.len && unwrapped.ptr[0] >= 0x02 && unwrapped.ptr[0] <= 0x04)
	{
		*data = chunk_clone(unwrapped);
		free(wrapped.ptr);
	}
}

/**
 * Get attributes for a given object during enumeration
 */
static bool get_attributes(object_enumerator_t *this, CK_OBJECT_HANDLE object)
{
	chunk_t data;
	CK_RV rv;
	int i;

	free_attrs(this);

	/* get length of objects first */
	rv = this->lib->f->C_GetAttributeValue(this->session, object,
										   this->attr, this->count);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GetAttributeValue(NULL) error: %N", ck_rv_names, rv);
		return FALSE;
	}
	/* allocate required chunks */
	for (i = 0; i < this->count; i++)
	{
		if (this->attr[i].pValue == NULL &&
			this->attr[i].ulValueLen != 0 && this->attr[i].ulValueLen != -1)
		{
			this->attr[i].pValue = malloc(this->attr[i].ulValueLen);
			this->freelist->insert_last(this->freelist, &this->attr[i]);
		}
	}
	/* get the data */
	rv = this->lib->f->C_GetAttributeValue(this->session, object,
										   this->attr, this->count);
	if (rv != CKR_OK)
	{
		free_attrs(this);
		DBG1(DBG_CFG, "C_GetAttributeValue() error: %N", ck_rv_names, rv);
		return FALSE;
	}
	for (i = 0; i < this->count; i++)
	{
		if (this->attr[i].type == CKA_EC_POINT)
		{
			data = chunk_create(this->attr[i].pValue, this->attr[i].ulValueLen);
			unwrap_ec_point(&data);
			this->attr[i].pValue = data.ptr;
			this->attr[i].ulValueLen = data.len;
		}
	}
	return TRUE;
}

METHOD(enumerator_t, object_enumerate, bool,
	object_enumerator_t *this, va_list args)
{
	CK_OBJECT_HANDLE object, *out;
	CK_ULONG found;
	CK_RV rv;

	VA_ARGS_VGET(args, out);

	if (!this->object)
	{
		rv = this->lib->f->C_FindObjects(this->session, &object, 1, &found);
		if (rv != CKR_OK)
		{
			DBG1(DBG_CFG, "C_FindObjects() failed: %N", ck_rv_names, rv);
			return FALSE;
		}
	}
	else
	{
		object = this->object;
		found = 1;
	}
	if (found)
	{
		if (this->attr)
		{
			if (!get_attributes(this, object))
			{
				return FALSE;
			}
		}
		if (out)
		{
			*out = object;
		}
		return TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, object_destroy, void,
	object_enumerator_t *this)
{
	if (!this->object)
	{
		this->lib->f->C_FindObjectsFinal(this->session);
	}
	free_attrs(this);
	this->freelist->destroy(this->freelist);
	free(this);
}

METHOD(pkcs11_library_t, create_object_enumerator, enumerator_t*,
	private_pkcs11_library_t *this, CK_SESSION_HANDLE session,
	CK_ATTRIBUTE_PTR tmpl, CK_ULONG tcount,
	CK_ATTRIBUTE_PTR attr, CK_ULONG acount)
{
	object_enumerator_t *enumerator;
	CK_RV rv;

	rv = this->public.f->C_FindObjectsInit(session, tmpl, tcount);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_FindObjectsInit() failed: %N", ck_rv_names, rv);
		return enumerator_create_empty();
	}

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _object_enumerate,
			.destroy = _object_destroy,
		},
		.session = session,
		.lib = &this->public,
		.attr = attr,
		.count = acount,
		.freelist = linked_list_create(),
	);
	return &enumerator->public;
}

METHOD(pkcs11_library_t, create_object_attr_enumerator, enumerator_t*,
	private_pkcs11_library_t *this, CK_SESSION_HANDLE session,
	CK_OBJECT_HANDLE object, CK_ATTRIBUTE_PTR attr, CK_ULONG count)
{
	object_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _object_enumerate,
			.destroy = _object_destroy,
		},
		.session = session,
		.lib = &this->public,
		.attr = attr,
		.count = count,
		.object = object,
		.freelist = linked_list_create(),
	);
	return &enumerator->public;
}

/**
 * Enumerator over mechanisms
 */
typedef struct {
	/* implements enumerator_t */
	enumerator_t public;
	/* PKCS#11 library */
	pkcs11_library_t *lib;
	/* slot of token */
	CK_SLOT_ID slot;
	/* mechanism type list */
	CK_MECHANISM_TYPE_PTR mechs;
	/* number of mechanism types */
	CK_ULONG count;
	/* current mechanism */
	CK_ULONG current;
} mechanism_enumerator_t;

METHOD(enumerator_t, enumerate_mech, bool,
	mechanism_enumerator_t *this, va_list args)
{
	CK_MECHANISM_INFO *info;
	CK_MECHANISM_TYPE *type;
	CK_RV rv;

	VA_ARGS_VGET(args, type, info);

	if (this->current >= this->count)
	{
		return FALSE;
	}
	if (info)
	{
		rv = this->lib->f->C_GetMechanismInfo(this->slot,
											  this->mechs[this->current], info);
		if (rv != CKR_OK)
		{
			DBG1(DBG_CFG, "C_GetMechanismInfo() failed: %N", ck_rv_names, rv);
			return FALSE;
		}
	}
	*type = this->mechs[this->current++];
	return TRUE;
}

METHOD(enumerator_t, destroy_mech, void,
	mechanism_enumerator_t *this)
{
	free(this->mechs);
	free(this);
}

METHOD(pkcs11_library_t, create_mechanism_enumerator, enumerator_t*,
	private_pkcs11_library_t *this, CK_SLOT_ID slot)
{
	mechanism_enumerator_t *enumerator;
	CK_RV rv;

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_mech,
			.destroy = _destroy_mech,
		},
		.lib = &this->public,
		.slot = slot,
	);

	rv = enumerator->lib->f->C_GetMechanismList(slot, NULL, &enumerator->count);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GetMechanismList() failed: %N", ck_rv_names, rv);
		free(enumerator);
		return enumerator_create_empty();
	}
	enumerator->mechs = malloc(sizeof(CK_MECHANISM_TYPE) * enumerator->count);
	rv = enumerator->lib->f->C_GetMechanismList(slot, enumerator->mechs,
												&enumerator->count);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GetMechanismList() failed: %N", ck_rv_names, rv);
		destroy_mech(enumerator);
		return enumerator_create_empty();
	}
	return &enumerator->public;
}

METHOD(pkcs11_library_t, get_ck_attribute, bool,
	private_pkcs11_library_t *this, CK_SESSION_HANDLE session,
	CK_OBJECT_HANDLE obj, CK_ATTRIBUTE_TYPE type, chunk_t *data)
{
	CK_ATTRIBUTE attr = { type, NULL, 0 };
	CK_RV rv;
	rv = this->public.f->C_GetAttributeValue(session, obj, &attr, 1);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GetAttributeValue(%N) error: %N", ck_attr_names, type,
			 ck_rv_names, rv);
		return FALSE;
	}
	*data = chunk_alloc(attr.ulValueLen);
	attr.pValue = data->ptr;
	rv = this->public.f->C_GetAttributeValue(session, obj, &attr, 1);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GetAttributeValue(%N) error: %N", ck_attr_names, type,
			 ck_rv_names, rv);
		chunk_free(data);
		return FALSE;
	}
	if (attr.type == CKA_EC_POINT)
	{
		unwrap_ec_point(data);
	}
	return TRUE;
}

METHOD(pkcs11_library_t, destroy, void,
	private_pkcs11_library_t *this)
{
	this->public.f->C_Finalize(NULL);
	dlclose(this->handle);
	free(this->name);
	free(this);
}

/**
 * See header
 */
void pkcs11_library_trim(char *str, int len)
{
	int i;

	str[len - 1] = '\0';
	for (i = len - 2; i > 0; i--)
	{
		if (str[i] == ' ')
		{
			str[i] = '\0';
			continue;
		}
		break;
	}
}

/**
 * Mutex creation callback
 */
static CK_RV CreateMutex(CK_VOID_PTR_PTR data)
{
	*data = mutex_create(MUTEX_TYPE_RECURSIVE);
	return CKR_OK;
}

/**
 * Mutex destruction callback
 */
static CK_RV DestroyMutex(CK_VOID_PTR data)
{
	mutex_t *mutex = (mutex_t*)data;

	mutex->destroy(mutex);
	return CKR_OK;
}

/**
 * Mutex lock callback
 */
static CK_RV LockMutex(CK_VOID_PTR data)
{
	mutex_t *mutex = (mutex_t*)data;

	mutex->lock(mutex);
	return CKR_OK;
}

/**
 * Mutex unlock callback
 */
static CK_RV UnlockMutex(CK_VOID_PTR data)
{
	mutex_t *mutex = (mutex_t*)data;

	mutex->unlock(mutex);
	return CKR_OK;
}

/**
 * Check if the library has at least a given cryptoki version
 */
static bool has_version(CK_INFO *info, int major, int minor)
{
	return info->cryptokiVersion.major > major ||
			(info->cryptokiVersion.major == major &&
			 info->cryptokiVersion.minor >= minor);
}

/**
 * Check for optional PKCS#11 library functionality
 */
static void check_features(private_pkcs11_library_t *this, CK_INFO *info)
{
	if (has_version(info, 2, 20))
	{
		this->features |= PKCS11_TRUSTED_CERTS;
		this->features |= PKCS11_ALWAYS_AUTH_KEYS;
	}
}

/**
 * Initialize a PKCS#11 library
 */
static bool initialize(private_pkcs11_library_t *this, char *name, char *file,
					   bool os_locking)
{
	CK_C_GetFunctionList pC_GetFunctionList;
	CK_INFO info;
	CK_RV rv;
	static CK_C_INITIALIZE_ARGS args = {
		.CreateMutex = CreateMutex,
		.DestroyMutex = DestroyMutex,
		.LockMutex = LockMutex,
		.UnlockMutex = UnlockMutex,
	};
	static CK_C_INITIALIZE_ARGS args_os = {
		.flags = CKF_OS_LOCKING_OK,
	};

	pC_GetFunctionList = dlsym(this->handle, "C_GetFunctionList");
	if (!pC_GetFunctionList)
	{
		DBG1(DBG_CFG, "C_GetFunctionList not found for '%s': %s", name, dlerror());
		return FALSE;
	}
	rv = pC_GetFunctionList(&this->public.f);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GetFunctionList() error for '%s': %N",
			 name, ck_rv_names, rv);
		return FALSE;
	}
	if (os_locking)
	{
		rv = CKR_CANT_LOCK;
	}
	else
	{
		rv = this->public.f->C_Initialize(&args);
	}
	if (rv == CKR_CANT_LOCK)
	{	/* fallback to OS locking */
		os_locking = TRUE;
		rv = this->public.f->C_Initialize(&args_os);
	}
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_Initialize() error for '%s': %N",
			 name, ck_rv_names, rv);
		return FALSE;
	}
	rv = this->public.f->C_GetInfo(&info);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GetInfo() error for '%s': %N",
			 name, ck_rv_names, rv);
		this->public.f->C_Finalize(NULL);
		return FALSE;
	}

	pkcs11_library_trim(info.manufacturerID,
			strnlen(info.manufacturerID, sizeof(info.manufacturerID)));
	pkcs11_library_trim(info.libraryDescription,
			strnlen(info.libraryDescription, sizeof(info.libraryDescription)));

	DBG1(DBG_CFG, "loaded PKCS#11 v%d.%d library '%s' (%s)",
		 info.cryptokiVersion.major, info.cryptokiVersion.minor, name, file);
	DBG1(DBG_CFG, "  %s: %s v%d.%d",
		 info.manufacturerID, info.libraryDescription,
		 info.libraryVersion.major, info.libraryVersion.minor);
	if (os_locking)
	{
		DBG1(DBG_CFG, "  uses OS locking functions");
	}

	check_features(this, &info);
	return TRUE;
}

/**
 * See header
 */
pkcs11_library_t *pkcs11_library_create(char *name, char *file, bool os_locking)
{
	private_pkcs11_library_t *this;

	INIT(this,
		.public = {
			.get_name = _get_name,
			.get_features = _get_features,
			.create_object_enumerator = _create_object_enumerator,
			.create_object_attr_enumerator = _create_object_attr_enumerator,
			.create_mechanism_enumerator = _create_mechanism_enumerator,
			.get_ck_attribute = _get_ck_attribute,
			.destroy = _destroy,
		},
		.name = strdup(name),
		.handle = dlopen(file, RTLD_LAZY),
	);

	if (!this->handle)
	{
		DBG1(DBG_CFG, "opening PKCS#11 library failed: %s", dlerror());
		free(this);
		return NULL;
	}

	if (!initialize(this, name, file, os_locking))
	{
		dlclose(this->handle);
		free(this);
		return NULL;
	}

	return &this->public;
}
