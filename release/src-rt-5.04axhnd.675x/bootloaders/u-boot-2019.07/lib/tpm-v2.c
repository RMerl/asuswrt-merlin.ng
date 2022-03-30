// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Bootlin
 * Author: Miquel Raynal <miquel.raynal@bootlin.com>
 */

#include <common.h>
#include <dm.h>
#include <tpm-common.h>
#include <tpm-v2.h>
#include "tpm-utils.h"

u32 tpm2_startup(struct udevice *dev, enum tpm2_startup_types mode)
{
	const u8 command_v2[12] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),
		tpm_u32(12),
		tpm_u32(TPM2_CC_STARTUP),
		tpm_u16(mode),
	};
	int ret;

	/*
	 * Note TPM2_Startup command will return RC_SUCCESS the first time,
	 * but will return RC_INITIALIZE otherwise.
	 */
	ret = tpm_sendrecv_command(dev, command_v2, NULL, NULL);
	if (ret && ret != TPM2_RC_INITIALIZE)
		return ret;

	return 0;
}

u32 tpm2_self_test(struct udevice *dev, enum tpm2_yes_no full_test)
{
	const u8 command_v2[12] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),
		tpm_u32(11),
		tpm_u32(TPM2_CC_SELF_TEST),
		full_test,
	};

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_clear(struct udevice *dev, u32 handle, const char *pw,
	       const ssize_t pw_sz)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(27 + pw_sz),		/* Length */
		tpm_u32(TPM2_CC_CLEAR),		/* Command code */

		/* HANDLE */
		tpm_u32(handle),		/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz),			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */
	};
	unsigned int offset = 27;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "s",
			       offset, pw, pw_sz);
	offset += pw_sz;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_pcr_extend(struct udevice *dev, u32 index, const uint8_t *digest)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(33 + TPM2_DIGEST_LEN),	/* Length */
		tpm_u32(TPM2_CC_PCR_EXTEND),	/* Command code */

		/* HANDLE */
		tpm_u32(index),			/* Handle (PCR Index) */

		/* AUTH_SESSION */
		tpm_u32(9),			/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(0),			/* Size of <hmac/password> */
						/* <hmac/password> (if any) */
		tpm_u32(1),			/* Count (number of hashes) */
		tpm_u16(TPM2_ALG_SHA256),	/* Algorithm of the hash */
		/* STRING(digest)		   Digest */
	};
	unsigned int offset = 33;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the digest
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "s",
			       offset, digest, TPM2_DIGEST_LEN);
	offset += TPM2_DIGEST_LEN;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_pcr_read(struct udevice *dev, u32 idx, unsigned int idx_min_sz,
		  void *data, unsigned int *updates)
{
	u8 idx_array_sz = max(idx_min_sz, DIV_ROUND_UP(idx, 8));
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),	/* TAG */
		tpm_u32(17 + idx_array_sz),	/* Length */
		tpm_u32(TPM2_CC_PCR_READ),	/* Command code */

		/* TPML_PCR_SELECTION */
		tpm_u32(1),			/* Number of selections */
		tpm_u16(TPM2_ALG_SHA256),	/* Algorithm of the hash */
		idx_array_sz,			/* Array size for selection */
		/* bitmap(idx)			   Selected PCR bitmap */
	};
	size_t response_len = COMMAND_BUFFER_SIZE;
	u8 response[COMMAND_BUFFER_SIZE];
	unsigned int pcr_sel_idx = idx / 8;
	u8 pcr_sel_bit = BIT(idx % 8);
	unsigned int counter = 0;
	int ret;

	if (pack_byte_string(command_v2, COMMAND_BUFFER_SIZE, "b",
			     17 + pcr_sel_idx, pcr_sel_bit))
		return TPM_LIB_ERROR;

	ret = tpm_sendrecv_command(dev, command_v2, response, &response_len);
	if (ret)
		return ret;

	if (unpack_byte_string(response, response_len, "ds",
			       10, &counter,
			       response_len - TPM2_DIGEST_LEN, data,
			       TPM2_DIGEST_LEN))
		return TPM_LIB_ERROR;

	if (updates)
		*updates = counter;

	return 0;
}

u32 tpm2_get_capability(struct udevice *dev, u32 capability, u32 property,
			void *buf, size_t prop_count)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),		/* TAG */
		tpm_u32(22),				/* Length */
		tpm_u32(TPM2_CC_GET_CAPABILITY),	/* Command code */

		tpm_u32(capability),			/* Capability */
		tpm_u32(property),			/* Property */
		tpm_u32(prop_count),			/* Property count */
	};
	u8 response[COMMAND_BUFFER_SIZE];
	size_t response_len = COMMAND_BUFFER_SIZE;
	unsigned int properties_off;
	int ret;

	ret = tpm_sendrecv_command(dev, command_v2, response, &response_len);
	if (ret)
		return ret;

	/*
	 * In the response buffer, the properties are located after the:
	 * tag (u16), response size (u32), response code (u32),
	 * YES/NO flag (u8), TPM_CAP (u32) and TPMU_CAPABILITIES (u32).
	 */
	properties_off = sizeof(u16) + sizeof(u32) + sizeof(u32) +
			 sizeof(u8) + sizeof(u32) + sizeof(u32);
	memcpy(buf, &response[properties_off], response_len - properties_off);

	return 0;
}

u32 tpm2_dam_reset(struct udevice *dev, const char *pw, const ssize_t pw_sz)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(27 + pw_sz),		/* Length */
		tpm_u32(TPM2_CC_DAM_RESET),	/* Command code */

		/* HANDLE */
		tpm_u32(TPM2_RH_LOCKOUT),	/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz),			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */
	};
	unsigned int offset = 27;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "s",
			       offset, pw, pw_sz);
	offset += pw_sz;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_dam_parameters(struct udevice *dev, const char *pw,
			const ssize_t pw_sz, unsigned int max_tries,
			unsigned int recovery_time,
			unsigned int lockout_recovery)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(27 + pw_sz + 12),	/* Length */
		tpm_u32(TPM2_CC_DAM_PARAMETERS), /* Command code */

		/* HANDLE */
		tpm_u32(TPM2_RH_LOCKOUT),	/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz),			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */

		/* LOCKOUT PARAMETERS */
		/* tpm_u32(max_tries)		   Max tries (0, always lock) */
		/* tpm_u32(recovery_time)	   Recovery time (0, no lock) */
		/* tpm_u32(lockout_recovery)	   Lockout recovery */
	};
	unsigned int offset = 27;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 *     - max tries
	 *     - recovery time
	 *     - lockout recovery
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "sddd",
			       offset, pw, pw_sz,
			       offset + pw_sz, max_tries,
			       offset + pw_sz + 4, recovery_time,
			       offset + pw_sz + 8, lockout_recovery);
	offset += pw_sz + 12;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

int tpm2_change_auth(struct udevice *dev, u32 handle, const char *newpw,
		     const ssize_t newpw_sz, const char *oldpw,
		     const ssize_t oldpw_sz)
{
	unsigned int offset = 27;
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(offset + oldpw_sz + 2 + newpw_sz), /* Length */
		tpm_u32(TPM2_CC_HIERCHANGEAUTH), /* Command code */

		/* HANDLE */
		tpm_u32(handle),		/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + oldpw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(oldpw_sz)		/* Size of <hmac/password> */
		/* STRING(oldpw)		   <hmac/password> (if any) */

		/* TPM2B_AUTH (TPM2B_DIGEST) */
		/* tpm_u16(newpw_sz)		   Digest size, new pw length */
		/* STRING(newpw)		   Digest buffer, new pw */
	};
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the old password (if any)
	 *     - size of the new password
	 *     - new password
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "sws",
			       offset, oldpw, oldpw_sz,
			       offset + oldpw_sz, newpw_sz,
			       offset + oldpw_sz + 2, newpw, newpw_sz);
	offset += oldpw_sz + 2 + newpw_sz;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_pcr_setauthpolicy(struct udevice *dev, const char *pw,
			   const ssize_t pw_sz, u32 index, const char *key)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(35 + pw_sz + TPM2_DIGEST_LEN), /* Length */
		tpm_u32(TPM2_CC_PCR_SETAUTHPOL), /* Command code */

		/* HANDLE */
		tpm_u32(TPM2_RH_PLATFORM),	/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz)			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */

		/* TPM2B_AUTH (TPM2B_DIGEST) */
		/* tpm_u16(TPM2_DIGEST_LEN)	   Digest size length */
		/* STRING(key)			   Digest buffer (PCR key) */

		/* TPMI_ALG_HASH */
		/* tpm_u16(TPM2_ALG_SHA256)   Algorithm of the hash */

		/* TPMI_DH_PCR */
		/* tpm_u32(index),		   PCR Index */
	};
	unsigned int offset = 27;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 *     - the PCR key length
	 *     - the PCR key
	 *     - the hash algorithm
	 *     - the PCR index
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "swswd",
			       offset, pw, pw_sz,
			       offset + pw_sz, TPM2_DIGEST_LEN,
			       offset + pw_sz + 2, key, TPM2_DIGEST_LEN,
			       offset + pw_sz + 2 + TPM2_DIGEST_LEN,
			       TPM2_ALG_SHA256,
			       offset + pw_sz + 4 + TPM2_DIGEST_LEN, index);
	offset += pw_sz + 2 + TPM2_DIGEST_LEN + 2 + 4;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_pcr_setauthvalue(struct udevice *dev, const char *pw,
			  const ssize_t pw_sz, u32 index, const char *key,
			  const ssize_t key_sz)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(33 + pw_sz + TPM2_DIGEST_LEN), /* Length */
		tpm_u32(TPM2_CC_PCR_SETAUTHVAL), /* Command code */

		/* HANDLE */
		tpm_u32(index),			/* Handle (PCR Index) */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz),			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */

		/* TPM2B_DIGEST */
		/* tpm_u16(key_sz)		   Key length */
		/* STRING(key)			   Key */
	};
	unsigned int offset = 27;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 *     - the number of digests, 1 in our case
	 *     - the algorithm, sha256 in our case
	 *     - the digest (64 bytes)
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "sws",
			       offset, pw, pw_sz,
			       offset + pw_sz, key_sz,
			       offset + pw_sz + 2, key, key_sz);
	offset += pw_sz + 2 + key_sz;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}
