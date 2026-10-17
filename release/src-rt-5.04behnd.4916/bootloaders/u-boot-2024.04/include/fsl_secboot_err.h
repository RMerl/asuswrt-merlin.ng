/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#ifndef _FSL_SECBOOT_ERR_H
#define _FSL_SECBOOT_ERR_H

#define ERROR_ESBC_PAMU_INIT					0x100000
#define ERROR_ESBC_SEC_RESET					0x200000
#define ERROR_ESBC_SEC_INIT					0x400000
#define ERROR_ESBC_SEC_DEQ					0x800000
#define ERROR_ESBC_SEC_DEQ_TO					0x1000000
#define ERROR_ESBC_SEC_ENQ					0x2000000
#define ERROR_ESBC_SEC_JOBQ_STATUS				0x4000000
#define ERROR_ESBC_CLIENT_CPUID_NO_MATCH			0x1
#define ERROR_ESBC_CLIENT_HDR_LOC				0x2
#define ERROR_ESBC_CLIENT_HEADER_BARKER				0x4
#define ERROR_ESBC_CLIENT_HEADER_KEY_LEN			0x8
#define ERROR_ESBC_CLIENT_HEADER_SIG_LEN			0x10
#define ERROR_ESBC_CLIENT_HEADER_KEY_REVOKED			0x11
#define ERROR_ESBC_CLIENT_HEADER_INVALID_SRK_NUM_ENTRY		0x12
#define ERROR_ESBC_CLIENT_HEADER_INVALID_KEY_NUM		0x13
#define ERROR_ESBC_CLIENT_HEADER_INV_SRK_ENTRY_KEYLEN		0x14
#define ERROR_ESBC_CLIENT_HEADER_IE_KEY_REVOKED			0x15
#define ERROR_ESBC_CLIENT_HEADER_INVALID_IE_NUM_ENTRY		0x16
#define ERROR_ESBC_CLIENT_HEADER_INVALID_IE_KEY_NUM		0x17
#define ERROR_ESBC_CLIENT_HEADER_INV_IE_ENTRY_KEYLEN		0x18
#define ERROR_IE_TABLE_NOT_FOUND				0x19
#define ERROR_ESBC_CLIENT_HEADER_KEY_LEN_NOT_TWICE_SIG_LEN	0x20
#define ERROR_KEY_TABLE_NOT_FOUND				0x21
#define ERROR_ESBC_CLIENT_HEADER_KEY_MOD_1			0x40
#define ERROR_ESBC_CLIENT_HEADER_KEY_MOD_2			0x80
#define ERROR_ESBC_CLIENT_HEADER_SIG_KEY_MOD			0x100
#define ERROR_ESBC_CLIENT_HEADER_SG_ESBC_EP			0x200
#define ERROR_ESBC_CLIENT_HASH_COMPARE_KEY			0x400
#define ERROR_ESBC_CLIENT_HASH_COMPARE_EM			0x800
#define ERROR_ESBC_CLIENT_SSM_TRUSTSTS				0x1000
#define ERROR_ESBC_CLIENT_BAD_ADDRESS				0x2000
#define ERROR_ESBC_CLIENT_MISC					0x4000
#define ERROR_ESBC_CLIENT_HEADER_SG_ENTIRES_BAD			0x8000
#define ERROR_ESBC_CLIENT_HEADER_SG				0x10000
#define ERROR_ESBC_CLIENT_HEADER_IMG_SIZE			0x20000
#define ERROR_ESBC_WRONG_CMD					0x40000
#define ERROR_ESBC_MISSING_BOOTM				0x80000
#define ERROR_ESBC_CLIENT_MAX					0x0

struct fsl_secboot_errcode {
	int errcode;
	const char *name;
};

static const struct fsl_secboot_errcode fsl_secboot_errcodes[] = {
	{ ERROR_ESBC_PAMU_INIT,
		"Error in initializing PAMU"},
	{ ERROR_ESBC_SEC_RESET,
		"Error in resetting Job ring of SEC"},
	{ ERROR_ESBC_SEC_INIT,
		"Error in initializing SEC"},
	{ ERROR_ESBC_SEC_ENQ,
		"Error in enqueue operation by SEC"},
	{ ERROR_ESBC_SEC_DEQ_TO,
		"Dequeue operation by SEC is timed out"},
	{ ERROR_ESBC_SEC_DEQ,
		"Error in dequeue operation by SEC"},
	{ ERROR_ESBC_SEC_JOBQ_STATUS,
		"Error in status of the job submitted to SEC"},
	{ ERROR_ESBC_CLIENT_CPUID_NO_MATCH,
		"Current core is not boot core i.e core0" },
	{ ERROR_ESBC_CLIENT_HDR_LOC,
		"Header address not in allowed memory range" },
	{ ERROR_ESBC_CLIENT_HEADER_BARKER,
		"Wrong barker code in header" },
	{ ERROR_ESBC_CLIENT_HEADER_KEY_LEN,
		"Wrong public key length in header" },
	{ ERROR_ESBC_CLIENT_HEADER_SIG_LEN,
		"Wrong signature length in header" },
	{ ERROR_ESBC_CLIENT_HEADER_KEY_LEN_NOT_TWICE_SIG_LEN,
		"Public key length not twice of signature length" },
	{ ERROR_ESBC_CLIENT_HEADER_KEY_MOD_1,
		"Public key Modulus most significant bit not set" },
	{ ERROR_ESBC_CLIENT_HEADER_KEY_MOD_2,
		"Public key Modulus in header not odd" },
	{ ERROR_ESBC_CLIENT_HEADER_SIG_KEY_MOD,
		"Signature not less than modulus" },
	{ ERROR_ESBC_CLIENT_HEADER_SG_ESBC_EP,
		"Entry point not in allowed space or one of the SG entries" },
	{ ERROR_ESBC_CLIENT_HASH_COMPARE_KEY,
		"Public key hash comparison failed" },
	{ ERROR_ESBC_CLIENT_HASH_COMPARE_EM,
		"RSA verification failed" },
	{ ERROR_ESBC_CLIENT_SSM_TRUSTSTS,
		"SNVS not in TRUSTED state" },
	{ ERROR_ESBC_CLIENT_BAD_ADDRESS,
		"Bad address error" },
	{ ERROR_ESBC_CLIENT_MISC,
		"Miscallaneous error" },
	{ ERROR_ESBC_CLIENT_HEADER_SG,
		"No SG support"  },
	{ ERROR_ESBC_CLIENT_HEADER_IMG_SIZE,
		"Invalid Image size"  },
	{ ERROR_ESBC_WRONG_CMD,
		"Unknown cmd/Wrong arguments. Core in infinite loop"},
	{ ERROR_ESBC_MISSING_BOOTM,
		"Bootm command missing from bootscript" },
	{ ERROR_ESBC_CLIENT_HEADER_KEY_REVOKED,
		"Selected key is revoked" },
	{ ERROR_ESBC_CLIENT_HEADER_INVALID_SRK_NUM_ENTRY,
		"Wrong key entry" },
	{ ERROR_ESBC_CLIENT_HEADER_INVALID_KEY_NUM,
		"Wrong key is selected" },
	{ ERROR_ESBC_CLIENT_HEADER_INV_SRK_ENTRY_KEYLEN,
		"Wrong srk public key len in header" },
	{ ERROR_ESBC_CLIENT_HEADER_IE_KEY_REVOKED,
		"Selected IE key is revoked" },
	{ ERROR_ESBC_CLIENT_HEADER_INVALID_IE_NUM_ENTRY,
		"Wrong key entry in IE Table" },
	{ ERROR_ESBC_CLIENT_HEADER_INVALID_IE_KEY_NUM,
		"Wrong IE key is selected" },
	{ ERROR_ESBC_CLIENT_HEADER_INV_IE_ENTRY_KEYLEN,
		"Wrong IE public key len in header" },
	{ ERROR_IE_TABLE_NOT_FOUND,
		"Information about IE Table missing" },
	{ ERROR_KEY_TABLE_NOT_FOUND,
		"No Key/ Key Table Found in header"},
	{ ERROR_ESBC_CLIENT_MAX, "NULL" }
};

void fsl_secboot_handle_error(int error);
#endif
