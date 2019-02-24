/*
 * Copyright (C) 2018 Andreas Steffen
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

#ifdef TSS_TSS2_V2

#include "tpm_tss_tss2_names.h"

#include <tss2/tss2_sys.h>

/**
 * TPM 2.0 algorithm ID names
 */
ENUM_BEGIN(tpm_alg_id_names, TPM2_ALG_ERROR, TPM2_ALG_RSA,
	"ERROR",
	"RSA"
);
ENUM_NEXT(tpm_alg_id_names, TPM2_ALG_SHA1, TPM2_ALG_KEYEDHASH, TPM2_ALG_RSA,
	"SHA1",
	"HMAC",
	"AES",
	"MGF1",
	"KEYEDHASH"
);
ENUM_NEXT(tpm_alg_id_names, TPM2_ALG_XOR, TPM2_ALG_SHA512, TPM2_ALG_KEYEDHASH,
	"XOR",
	"SHA256",
	"SHA384",
	"SHA512"
);
ENUM_NEXT(tpm_alg_id_names, TPM2_ALG_NULL, TPM2_ALG_NULL, TPM2_ALG_SHA512,
	"NULL"
);
ENUM_NEXT(tpm_alg_id_names, TPM2_ALG_SM3_256, TPM2_ALG_ECMQV, TPM2_ALG_NULL,
	"SM3_256",
	"SM4",
	"RSASSA",
	"RSAES",
	"RSAPSS",
	"OAEP",
	"ECDSA",
	"ECDH",
	"SM2",
	"ECSCHNORR",
	"ECMQV"
);
ENUM_NEXT(tpm_alg_id_names, TPM2_ALG_KDF1_SP800_56A, TPM2_ALG_ECC, TPM2_ALG_ECMQV,
	"KDF1_SP800_56A",
	"KDF2",
	"KDF1_SP800_108",
	"ECC"
);
ENUM_NEXT(tpm_alg_id_names, TPM2_ALG_SYMCIPHER, TPM2_ALG_CAMELLIA, TPM2_ALG_ECC,
	"SYMCIPHER",
	"CAMELLIA"
);
ENUM_NEXT(tpm_alg_id_names, TPM2_ALG_CTR, TPM2_ALG_ECB, TPM2_ALG_CAMELLIA,
	"CTR",
	"OFB",
	"CBC",
	"CFB",
	"ECB"
);
ENUM_END(tpm_alg_id_names, TPM2_ALG_ECB);

/**
 * TPM 2.0 ECC curve names
 */
ENUM_BEGIN(tpm_ecc_curve_names, TPM2_ECC_NONE, TPM2_ECC_NIST_P521,
	"NONE",
	"NIST_P192",
	"NIST_P224",
	"NIST_P256",
	"NIST_P384",
	"NIST_P521"
);
ENUM_NEXT(tpm_ecc_curve_names, TPM2_ECC_BN_P256, TPM2_ECC_BN_P638, TPM2_ECC_NIST_P521,
	"BN_P256",
	"BN_P638"
);
ENUM_NEXT(tpm_ecc_curve_names, TPM2_ECC_SM2_P256, TPM2_ECC_SM2_P256, TPM2_ECC_BN_P638,
	"SM2_P256"
);
ENUM_END(tpm_ecc_curve_names, TPM2_ECC_SM2_P256);

#endif /* TSS_TSS2_V2 */

