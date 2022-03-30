// SPDX-License-Identifier: GPL-2.0+
/*
 * CAAM Error Reporting
 *
 * Copyright 2009-2014 Freescale Semiconductor, Inc.
 *
 * Derived from error.c file in linux drivers/crypto/caam
 */

#include <common.h>
#include <malloc.h>
#include "desc.h"
#include "jr.h"

#define CAAM_ERROR_STR_MAX 302

#define JRSTA_SSRC_SHIFT            28
#define JRSTA_CCBERR_CHAID_MASK     0x00f0
#define JRSTA_CCBERR_CHAID_SHIFT    4
#define JRSTA_CCBERR_ERRID_MASK     0x000
#define JRSTA_CCBERR_CHAID_RNG      (0x05 << JRSTA_CCBERR_CHAID_SHIFT)

#define JRSTA_DECOERR_JUMP          0x08000000
#define JRSTA_DECOERR_INDEX_SHIFT   8
#define JRSTA_DECOERR_INDEX_MASK    0xff00
#define JRSTA_DECOERR_ERROR_MASK    0x00ff


static const struct {
	u8 value;
	const char *error_text;
} desc_error_list[] = {
	{ 0x00, "No error." },
	{ 0x01, "SGT Length Error. The descriptor is trying to read" \
		" more data than is contained in the SGT table." },
	{ 0x02, "SGT Null Entry Error." },
	{ 0x03, "Job Ring Control Error. Bad value in Job Ring Control reg." },
	{ 0x04, "Invalid Descriptor Command." },
	{ 0x05, "Reserved." },
	{ 0x06, "Invalid KEY Command" },
	{ 0x07, "Invalid LOAD Command" },
	{ 0x08, "Invalid STORE Command" },
	{ 0x09, "Invalid OPERATION Command" },
	{ 0x0A, "Invalid FIFO LOAD Command" },
	{ 0x0B, "Invalid FIFO STORE Command" },
	{ 0x0C, "Invalid MOVE/MOVE_LEN Command" },
	{ 0x0D, "Invalid JUMP Command" },
	{ 0x0E, "Invalid MATH Command" },
	{ 0x0F, "Invalid SIGNATURE Command" },
	{ 0x10, "Invalid Sequence Command" },
	{ 0x11, "Skip data type invalid. The type must be 0xE or 0xF."},
	{ 0x12, "Shared Descriptor Header Error" },
	{ 0x13, "Header Error. Invalid length or parity, or other problems." },
	{ 0x14, "Burster Error. Burster has gotten to an illegal state" },
	{ 0x15, "Context Register Length Error" },
	{ 0x16, "DMA Error" },
	{ 0x17, "Reserved." },
	{ 0x1A, "Job failed due to JR reset" },
	{ 0x1B, "Job failed due to Fail Mode" },
	{ 0x1C, "DECO Watchdog timer timeout error" },
	{ 0x1D, "DECO tried to copy a key from another DECO but" \
		" the other DECO's Key Registers were locked" },
	{ 0x1E, "DECO attempted to copy data from a DECO" \
		"that had an unmasked Descriptor error" },
	{ 0x1F, "LIODN error" },
	{ 0x20, "DECO has completed a reset initiated via the DRR register" },
	{ 0x21, "Nonce error" },
	{ 0x22, "Meta data is too large (> 511 bytes) for TLS decap" },
	{ 0x23, "Read Input Frame error" },
	{ 0x24, "JDKEK, TDKEK or TDSK not loaded error" },
	{ 0x80, "DNR (do not run) error" },
	{ 0x81, "undefined protocol command" },
	{ 0x82, "invalid setting in PDB" },
	{ 0x83, "Anti-replay LATE error" },
	{ 0x84, "Anti-replay REPLAY error" },
	{ 0x85, "Sequence number overflow" },
	{ 0x86, "Sigver invalid signature" },
	{ 0x87, "DSA Sign Illegal test descriptor" },
	{ 0x88, "Protocol Format Error" },
	{ 0x89, "Protocol Size Error" },
	{ 0xC1, "Blob Command error: Undefined mode" },
	{ 0xC2, "Blob Command error: Secure Memory Blob mode error" },
	{ 0xC4, "Blob Command error: Black Blob key or input size error" },
	{ 0xC5, "Blob Command error: Invalid key destination" },
	{ 0xC8, "Blob Command error: Trusted/Secure mode error" },
	{ 0xF0, "IPsec TTL or hop limit field is 0, or was decremented to 0" },
	{ 0xF1, "3GPP HFN matches or exceeds the Threshold" },
};

static const char * const cha_id_list[] = {
	"",
	"AES",
	"DES",
	"ARC4",
	"MDHA",
	"RNG",
	"SNOW f8",
	"Kasumi f8/9",
	"PKHA",
	"CRCA",
	"SNOW f9",
	"ZUCE",
	"ZUCA",
};

static const char * const err_id_list[] = {
	"No error.",
	"Mode error.",
	"Data size error.",
	"Key size error.",
	"PKHA A memory size error.",
	"PKHA B memory size error.",
	"Data arrived out of sequence error.",
	"PKHA divide-by-zero error.",
	"PKHA modulus even error.",
	"DES key parity error.",
	"ICV check failed.",
	"Hardware error.",
	"Unsupported CCM AAD size.",
	"Class 1 CHA is not reset",
	"Invalid CHA combination was selected",
	"Invalid CHA selected.",
};

static const char * const rng_err_id_list[] = {
	"",
	"",
	"",
	"Instantiate",
	"Not instantiated",
	"Test instantiate",
	"Prediction resistance",
	"Prediction resistance and test request",
	"Uninstantiate",
	"Secure key generation",
};

static void report_ccb_status(const u32 status,
			      const char *error)
{
	u8 cha_id = (status & JRSTA_CCBERR_CHAID_MASK) >>
		    JRSTA_CCBERR_CHAID_SHIFT;
	u8 err_id = status & JRSTA_CCBERR_ERRID_MASK;
	u8 idx = (status & JRSTA_DECOERR_INDEX_MASK) >>
		  JRSTA_DECOERR_INDEX_SHIFT;
	char *idx_str;
	const char *cha_str = "unidentified cha_id value 0x";
	char cha_err_code[3] = { 0 };
	const char *err_str = "unidentified err_id value 0x";
	char err_err_code[3] = { 0 };

	if (status & JRSTA_DECOERR_JUMP)
		idx_str = "jump tgt desc idx";
	else
		idx_str = "desc idx";

	if (cha_id < ARRAY_SIZE(cha_id_list))
		cha_str = cha_id_list[cha_id];
	else
		snprintf(cha_err_code, sizeof(cha_err_code), "%02x", cha_id);

	if ((cha_id << JRSTA_CCBERR_CHAID_SHIFT) == JRSTA_CCBERR_CHAID_RNG &&
	    err_id < ARRAY_SIZE(rng_err_id_list) &&
	    strlen(rng_err_id_list[err_id])) {
		/* RNG-only error */
		err_str = rng_err_id_list[err_id];
	} else if (err_id < ARRAY_SIZE(err_id_list)) {
		err_str = err_id_list[err_id];
	} else {
		snprintf(err_err_code, sizeof(err_err_code), "%02x", err_id);
	}

	debug("%08x: %s: %s %d: %s%s: %s%s\n",
	       status, error, idx_str, idx,
		cha_str, cha_err_code,
		err_str, err_err_code);
}

static void report_jump_status(const u32 status,
			       const char *error)
{
	debug("%08x: %s: %s() not implemented\n",
	       status, error, __func__);
}

static void report_deco_status(const u32 status,
			       const char *error)
{
	u8 err_id = status & JRSTA_DECOERR_ERROR_MASK;
	u8 idx = (status & JRSTA_DECOERR_INDEX_MASK) >>
		  JRSTA_DECOERR_INDEX_SHIFT;
	char *idx_str;
	const char *err_str = "unidentified error value 0x";
	char err_err_code[3] = { 0 };
	int i;

	if (status & JRSTA_DECOERR_JUMP)
		idx_str = "jump tgt desc idx";
	else
		idx_str = "desc idx";

	for (i = 0; i < ARRAY_SIZE(desc_error_list); i++)
		if (desc_error_list[i].value == err_id)
			break;

	if (i != ARRAY_SIZE(desc_error_list) && desc_error_list[i].error_text)
		err_str = desc_error_list[i].error_text;
	else
		snprintf(err_err_code, sizeof(err_err_code), "%02x", err_id);

	debug("%08x: %s: %s %d: %s%s\n",
	       status, error, idx_str, idx, err_str, err_err_code);
}

static void report_jr_status(const u32 status,
			     const char *error)
{
	debug("%08x: %s: %s() not implemented\n",
	       status, error, __func__);
}

static void report_cond_code_status(const u32 status,
				    const char *error)
{
	debug("%08x: %s: %s() not implemented\n",
	       status, error, __func__);
}

void caam_jr_strstatus(u32 status)
{
	static const struct stat_src {
		void (*report_ssed)(const u32 status,
				    const char *error);
		const char *error;
	} status_src[] = {
		{ NULL, "No error" },
		{ NULL, NULL },
		{ report_ccb_status, "CCB" },
		{ report_jump_status, "Jump" },
		{ report_deco_status, "DECO" },
		{ NULL, NULL },
		{ report_jr_status, "Job Ring" },
		{ report_cond_code_status, "Condition Code" },
	};
	u32 ssrc = status >> JRSTA_SSRC_SHIFT;
	const char *error = status_src[ssrc].error;

	/*
	 * If there is no further error handling function, just
	 * print the error code, error string and exit. Otherwise
	 * call the handler function.
	 */
	if (!status_src[ssrc].report_ssed)
		debug("%08x: %s:\n", status, status_src[ssrc].error);
	else
		status_src[ssrc].report_ssed(status, error);
}
