// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 The Chromium OS Authors.
 * Coypright (c) 2013 Guntermann & Drunck GmbH
 */

#define LOG_CATEGORY UCLASS_TPM

#include <common.h>
#include <dm.h>
#include <asm/unaligned.h>
#include <u-boot/sha1.h>
#include <tpm-common.h>
#include <tpm-v1.h>
#include "tpm-utils.h"

#ifdef CONFIG_TPM_AUTH_SESSIONS

#ifndef CONFIG_SHA1
#error "TPM_AUTH_SESSIONS require SHA1 to be configured, too"
#endif /* !CONFIG_SHA1 */

struct session_data {
	int		valid;
	u32	handle;
	u8		nonce_even[DIGEST_LENGTH];
	u8		nonce_odd[DIGEST_LENGTH];
};

static struct session_data oiap_session = {0, };

#endif /* CONFIG_TPM_AUTH_SESSIONS */

u32 tpm_startup(struct udevice *dev, enum tpm_startup_type mode)
{
	const u8 command[12] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xc, 0x0, 0x0, 0x0, 0x99, 0x0, 0x0,
	};
	const size_t mode_offset = 10;
	u8 buf[COMMAND_BUFFER_SIZE];

	if (pack_byte_string(buf, sizeof(buf), "sw",
			     0, command, sizeof(command),
			     mode_offset, mode))
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, buf, NULL, NULL);
}

u32 tpm_resume(struct udevice *dev)
{
	return tpm_startup(dev, TPM_ST_STATE);
}

u32 tpm_self_test_full(struct udevice *dev)
{
	const u8 command[10] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xa, 0x0, 0x0, 0x0, 0x50,
	};
	return tpm_sendrecv_command(dev, command, NULL, NULL);
}

u32 tpm_continue_self_test(struct udevice *dev)
{
	const u8 command[10] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xa, 0x0, 0x0, 0x0, 0x53,
	};
	return tpm_sendrecv_command(dev, command, NULL, NULL);
}

u32 tpm_clear_and_reenable(struct udevice *dev)
{
	u32 ret;

	log_info("TPM: Clear and re-enable\n");
	ret = tpm_force_clear(dev);
	if (ret != TPM_SUCCESS) {
		log_err("Can't initiate a force clear\n");
		return ret;
	}

	if (tpm_get_version(dev) == TPM_V1) {
		ret = tpm_physical_enable(dev);
		if (ret != TPM_SUCCESS) {
			log_err("TPM: Can't set enabled state\n");
			return ret;
		}

		ret = tpm_physical_set_deactivated(dev, 0);
		if (ret != TPM_SUCCESS) {
			log_err("TPM: Can't set deactivated state\n");
			return ret;
		}
	}

	return TPM_SUCCESS;
}

u32 tpm_nv_define_space(struct udevice *dev, u32 index, u32 perm, u32 size)
{
	const u8 command[101] = {
		0x0, 0xc1,		/* TPM_TAG */
		0x0, 0x0, 0x0, 0x65,	/* parameter size */
		0x0, 0x0, 0x0, 0xcc,	/* TPM_COMMAND_CODE */
		/* TPM_NV_DATA_PUBLIC->... */
		0x0, 0x18,		/* ...->TPM_STRUCTURE_TAG */
		0, 0, 0, 0,		/* ...->TPM_NV_INDEX */
		/* TPM_NV_DATA_PUBLIC->TPM_PCR_INFO_SHORT */
		0x0, 0x3,
		0, 0, 0,
		0x1f,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* TPM_NV_DATA_PUBLIC->TPM_PCR_INFO_SHORT */
		0x0, 0x3,
		0, 0, 0,
		0x1f,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* TPM_NV_ATTRIBUTES->... */
		0x0, 0x17,		/* ...->TPM_STRUCTURE_TAG */
		0, 0, 0, 0,		/* ...->attributes */
		/* End of TPM_NV_ATTRIBUTES */
		0,			/* bReadSTClear */
		0,			/* bWriteSTClear */
		0,			/* bWriteDefine */
		0, 0, 0, 0,		/* size */
	};
	const size_t index_offset = 12;
	const size_t perm_offset = 70;
	const size_t size_offset = 77;
	u8 buf[COMMAND_BUFFER_SIZE];

	if (pack_byte_string(buf, sizeof(buf), "sddd",
			     0, command, sizeof(command),
			     index_offset, index,
			     perm_offset, perm,
			     size_offset, size))
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, buf, NULL, NULL);
}

u32 tpm_nv_set_locked(struct udevice *dev)
{
	return tpm_nv_define_space(dev, TPM_NV_INDEX_LOCK, 0, 0);
}

u32 tpm_nv_read_value(struct udevice *dev, u32 index, void *data, u32 count)
{
	const u8 command[22] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0x16, 0x0, 0x0, 0x0, 0xcf,
	};
	const size_t index_offset = 10;
	const size_t length_offset = 18;
	const size_t data_size_offset = 10;
	const size_t data_offset = 14;
	u8 buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	u32 data_size;
	u32 err;

	if (pack_byte_string(buf, sizeof(buf), "sdd",
			     0, command, sizeof(command),
			     index_offset, index,
			     length_offset, count))
		return TPM_LIB_ERROR;
	err = tpm_sendrecv_command(dev, buf, response, &response_length);
	if (err)
		return err;
	if (unpack_byte_string(response, response_length, "d",
			       data_size_offset, &data_size))
		return TPM_LIB_ERROR;
	if (data_size > count)
		return TPM_LIB_ERROR;
	if (unpack_byte_string(response, response_length, "s",
			       data_offset, data, data_size))
		return TPM_LIB_ERROR;

	return 0;
}

u32 tpm_nv_write_value(struct udevice *dev, u32 index, const void *data,
		       u32 length)
{
	const u8 command[256] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xcd,
	};
	const size_t command_size_offset = 2;
	const size_t index_offset = 10;
	const size_t length_offset = 18;
	const size_t data_offset = 22;
	const size_t write_info_size = 12;
	const u32 total_length =
		TPM_REQUEST_HEADER_LENGTH + write_info_size + length;
	u8 buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	u32 err;

	if (pack_byte_string(buf, sizeof(buf), "sddds",
			     0, command, sizeof(command),
			     command_size_offset, total_length,
			     index_offset, index,
			     length_offset, length,
			     data_offset, data, length))
		return TPM_LIB_ERROR;
	err = tpm_sendrecv_command(dev, buf, response, &response_length);
	if (err)
		return err;

	return 0;
}

uint32_t tpm_set_global_lock(struct udevice *dev)
{
	return tpm_nv_write_value(dev, TPM_NV_INDEX_0, NULL, 0);
}

u32 tpm_extend(struct udevice *dev, u32 index, const void *in_digest,
	       void *out_digest)
{
	const u8 command[34] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0x22, 0x0, 0x0, 0x0, 0x14,
	};
	const size_t index_offset = 10;
	const size_t in_digest_offset = 14;
	const size_t out_digest_offset = 10;
	u8 buf[COMMAND_BUFFER_SIZE];
	u8 response[TPM_RESPONSE_HEADER_LENGTH + PCR_DIGEST_LENGTH];
	size_t response_length = sizeof(response);
	u32 err;

	if (pack_byte_string(buf, sizeof(buf), "sds",
			     0, command, sizeof(command),
			     index_offset, index,
			     in_digest_offset, in_digest,
			     PCR_DIGEST_LENGTH))
		return TPM_LIB_ERROR;
	err = tpm_sendrecv_command(dev, buf, response, &response_length);
	if (err)
		return err;

	if (unpack_byte_string(response, response_length, "s",
			       out_digest_offset, out_digest,
			       PCR_DIGEST_LENGTH))
		return TPM_LIB_ERROR;

	return 0;
}

u32 tpm_pcr_read(struct udevice *dev, u32 index, void *data, size_t count)
{
	const u8 command[14] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x15,
	};
	const size_t index_offset = 10;
	const size_t out_digest_offset = 10;
	u8 buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	u32 err;

	if (count < PCR_DIGEST_LENGTH)
		return TPM_LIB_ERROR;

	if (pack_byte_string(buf, sizeof(buf), "sd",
			     0, command, sizeof(command),
			     index_offset, index))
		return TPM_LIB_ERROR;
	err = tpm_sendrecv_command(dev, buf, response, &response_length);
	if (err)
		return err;
	if (unpack_byte_string(response, response_length, "s",
			       out_digest_offset, data, PCR_DIGEST_LENGTH))
		return TPM_LIB_ERROR;

	return 0;
}

u32 tpm_tsc_physical_presence(struct udevice *dev, u16 presence)
{
	const u8 command[12] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xc, 0x40, 0x0, 0x0, 0xa, 0x0, 0x0,
	};
	const size_t presence_offset = 10;
	u8 buf[COMMAND_BUFFER_SIZE];

	if (pack_byte_string(buf, sizeof(buf), "sw",
			     0, command, sizeof(command),
			     presence_offset, presence))
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, buf, NULL, NULL);
}

u32 tpm_finalise_physical_presence(struct udevice *dev)
{
	const u8 command[12] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xc, 0x40, 0x0, 0x0, 0xa, 0x2, 0xa0,
	};

	return tpm_sendrecv_command(dev, command, NULL, NULL);
}

u32 tpm_read_pubek(struct udevice *dev, void *data, size_t count)
{
	const u8 command[30] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0x1e, 0x0, 0x0, 0x0, 0x7c,
	};
	const size_t response_size_offset = 2;
	const size_t data_offset = 10;
	const size_t header_and_checksum_size = TPM_RESPONSE_HEADER_LENGTH + 20;
	u8 response[COMMAND_BUFFER_SIZE + TPM_PUBEK_SIZE];
	size_t response_length = sizeof(response);
	u32 data_size;
	u32 err;

	err = tpm_sendrecv_command(dev, command, response, &response_length);
	if (err)
		return err;
	if (unpack_byte_string(response, response_length, "d",
			       response_size_offset, &data_size))
		return TPM_LIB_ERROR;
	if (data_size < header_and_checksum_size)
		return TPM_LIB_ERROR;
	data_size -= header_and_checksum_size;
	if (data_size > count)
		return TPM_LIB_ERROR;
	if (unpack_byte_string(response, response_length, "s",
			       data_offset, data, data_size))
		return TPM_LIB_ERROR;

	return 0;
}

u32 tpm_force_clear(struct udevice *dev)
{
	const u8 command[10] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xa, 0x0, 0x0, 0x0, 0x5d,
	};

	return tpm_sendrecv_command(dev, command, NULL, NULL);
}

u32 tpm_physical_enable(struct udevice *dev)
{
	const u8 command[10] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xa, 0x0, 0x0, 0x0, 0x6f,
	};

	return tpm_sendrecv_command(dev, command, NULL, NULL);
}

u32 tpm_physical_disable(struct udevice *dev)
{
	const u8 command[10] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xa, 0x0, 0x0, 0x0, 0x70,
	};

	return tpm_sendrecv_command(dev, command, NULL, NULL);
}

u32 tpm_physical_set_deactivated(struct udevice *dev, u8 state)
{
	const u8 command[11] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xb, 0x0, 0x0, 0x0, 0x72,
	};
	const size_t state_offset = 10;
	u8 buf[COMMAND_BUFFER_SIZE];

	if (pack_byte_string(buf, sizeof(buf), "sb",
			     0, command, sizeof(command),
			     state_offset, state))
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, buf, NULL, NULL);
}

u32 tpm_get_capability(struct udevice *dev, u32 cap_area, u32 sub_cap,
		       void *cap, size_t count)
{
	const u8 command[22] = {
		0x0, 0xc1,		/* TPM_TAG */
		0x0, 0x0, 0x0, 0x16,	/* parameter size */
		0x0, 0x0, 0x0, 0x65,	/* TPM_COMMAND_CODE */
		0x0, 0x0, 0x0, 0x0,	/* TPM_CAPABILITY_AREA */
		0x0, 0x0, 0x0, 0x4,	/* subcap size */
		0x0, 0x0, 0x0, 0x0,	/* subcap value */
	};
	const size_t cap_area_offset = 10;
	const size_t sub_cap_offset = 18;
	const size_t cap_offset = 14;
	const size_t cap_size_offset = 10;
	u8 buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	u32 cap_size;
	u32 err;

	if (pack_byte_string(buf, sizeof(buf), "sdd",
			     0, command, sizeof(command),
			     cap_area_offset, cap_area,
			     sub_cap_offset, sub_cap))
		return TPM_LIB_ERROR;
	err = tpm_sendrecv_command(dev, buf, response, &response_length);
	if (err)
		return err;
	if (unpack_byte_string(response, response_length, "d",
			       cap_size_offset, &cap_size))
		return TPM_LIB_ERROR;
	if (cap_size > response_length || cap_size > count)
		return TPM_LIB_ERROR;
	if (unpack_byte_string(response, response_length, "s",
			       cap_offset, cap, cap_size))
		return TPM_LIB_ERROR;

	return 0;
}

u32 tpm_get_permanent_flags(struct udevice *dev,
			    struct tpm_permanent_flags *pflags)
{
	const u8 command[22] = {
		0x0, 0xc1,		/* TPM_TAG */
		0x0, 0x0, 0x0, 0x16,	/* parameter size */
		0x0, 0x0, 0x0, 0x65,	/* TPM_COMMAND_CODE */
		0x0, 0x0, 0x0, 0x4,	/* TPM_CAP_FLAG_PERM */
		0x0, 0x0, 0x0, 0x4,	/* subcap size */
		0x0, 0x0, 0x1, 0x8,	/* subcap value */
	};
	const size_t data_size_offset = TPM_HEADER_SIZE;
	const size_t data_offset = TPM_HEADER_SIZE + sizeof(u32);
	u8 response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	u32 err;
	u32 data_size;

	err = tpm_sendrecv_command(dev, command, response, &response_length);
	if (err)
		return err;
	if (unpack_byte_string(response, response_length, "d",
			       data_size_offset, &data_size)) {
		log_err("Cannot unpack data size\n");
		return TPM_LIB_ERROR;
	}
	if (data_size < sizeof(*pflags)) {
		log_err("Data size too small\n");
		return TPM_LIB_ERROR;
	}
	if (unpack_byte_string(response, response_length, "s",
			       data_offset, pflags, sizeof(*pflags))) {
		log_err("Cannot unpack pflags\n");
		return TPM_LIB_ERROR;
	}

	return 0;
}

u32 tpm_get_permissions(struct udevice *dev, u32 index, u32 *perm)
{
	const u8 command[22] = {
		0x0, 0xc1,		/* TPM_TAG */
		0x0, 0x0, 0x0, 0x16,	/* parameter size */
		0x0, 0x0, 0x0, 0x65,	/* TPM_COMMAND_CODE */
		0x0, 0x0, 0x0, 0x11,
		0x0, 0x0, 0x0, 0x4,
	};
	const size_t index_offset = 18;
	const size_t perm_offset = 60;
	u8 buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	u32 err;

	if (pack_byte_string(buf, sizeof(buf), "d", 0, command, sizeof(command),
			     index_offset, index))
		return TPM_LIB_ERROR;
	err = tpm_sendrecv_command(dev, buf, response, &response_length);
	if (err)
		return err;
	if (unpack_byte_string(response, response_length, "d",
			       perm_offset, perm))
		return TPM_LIB_ERROR;

	return 0;
}

#ifdef CONFIG_TPM_FLUSH_RESOURCES
u32 tpm_flush_specific(struct udevice *dev, u32 key_handle, u32 resource_type)
{
	const u8 command[18] = {
		0x00, 0xc1,             /* TPM_TAG */
		0x00, 0x00, 0x00, 0x12, /* parameter size */
		0x00, 0x00, 0x00, 0xba, /* TPM_COMMAND_CODE */
		0x00, 0x00, 0x00, 0x00, /* key handle */
		0x00, 0x00, 0x00, 0x00, /* resource type */
	};
	const size_t key_handle_offset = 10;
	const size_t resource_type_offset = 14;
	u8 buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	u32 err;

	if (pack_byte_string(buf, sizeof(buf), "sdd",
			     0, command, sizeof(command),
			     key_handle_offset, key_handle,
			     resource_type_offset, resource_type))
		return TPM_LIB_ERROR;

	err = tpm_sendrecv_command(dev, buf, response, &response_length);
	if (err)
		return err;
	return 0;
}
#endif /* CONFIG_TPM_FLUSH_RESOURCES */

#ifdef CONFIG_TPM_AUTH_SESSIONS

/**
 * Fill an authentication block in a request.
 * This func can create the first as well as the second auth block (for
 * double authorized commands).
 *
 * @param request	pointer to the request (w/ uninitialised auth data)
 * @param request_len0	length of the request without auth data
 * @param handles_len	length of the handles area in request
 * @param auth_session	pointer to the (valid) auth session to be used
 * @param request_auth	pointer to the auth block of the request to be filled
 * @param auth		authentication data (HMAC key)
 */
static u32 create_request_auth(const void *request, size_t request_len0,
			       size_t handles_len,
			       struct session_data *auth_session,
			       void *request_auth, const void *auth)
{
	u8 hmac_data[DIGEST_LENGTH * 3 + 1];
	sha1_context hash_ctx;
	const size_t command_code_offset = 6;
	const size_t auth_nonce_odd_offset = 4;
	const size_t auth_continue_offset = 24;
	const size_t auth_auth_offset = 25;

	if (!auth_session || !auth_session->valid)
		return TPM_LIB_ERROR;

	sha1_starts(&hash_ctx);
	sha1_update(&hash_ctx, request + command_code_offset, 4);
	if (request_len0 > TPM_REQUEST_HEADER_LENGTH + handles_len)
		sha1_update(&hash_ctx,
			    request + TPM_REQUEST_HEADER_LENGTH + handles_len,
			    request_len0 - TPM_REQUEST_HEADER_LENGTH
			    - handles_len);
	sha1_finish(&hash_ctx, hmac_data);

	sha1_starts(&hash_ctx);
	sha1_update(&hash_ctx, auth_session->nonce_odd, DIGEST_LENGTH);
	sha1_update(&hash_ctx, hmac_data, sizeof(hmac_data));
	sha1_finish(&hash_ctx, auth_session->nonce_odd);

	if (pack_byte_string(request_auth, TPM_REQUEST_AUTH_LENGTH, "dsb",
			     0, auth_session->handle,
			     auth_nonce_odd_offset, auth_session->nonce_odd,
			     DIGEST_LENGTH,
			     auth_continue_offset, 1))
		return TPM_LIB_ERROR;
	if (pack_byte_string(hmac_data, sizeof(hmac_data), "ss",
			     DIGEST_LENGTH,
			     auth_session->nonce_even,
			     DIGEST_LENGTH,
			     2 * DIGEST_LENGTH,
			     request_auth + auth_nonce_odd_offset,
			     DIGEST_LENGTH + 1))
		return TPM_LIB_ERROR;
	sha1_hmac(auth, DIGEST_LENGTH, hmac_data, sizeof(hmac_data),
		  request_auth + auth_auth_offset);

	return TPM_SUCCESS;
}

/**
 * Verify an authentication block in a response.
 * Since this func updates the nonce_even in the session data it has to be
 * called when receiving a succesfull AUTH response.
 * This func can verify the first as well as the second auth block (for
 * double authorized commands).
 *
 * @param command_code	command code of the request
 * @param response	pointer to the request (w/ uninitialised auth data)
 * @param handles_len	length of the handles area in response
 * @param auth_session	pointer to the (valid) auth session to be used
 * @param response_auth	pointer to the auth block of the response to be verified
 * @param auth		authentication data (HMAC key)
 */
static u32 verify_response_auth(u32 command_code, const void *response,
				size_t response_len0, size_t handles_len,
				struct session_data *auth_session,
				const void *response_auth, const void *auth)
{
	u8 hmac_data[DIGEST_LENGTH * 3 + 1];
	u8 computed_auth[DIGEST_LENGTH];
	sha1_context hash_ctx;
	const size_t return_code_offset = 6;
	const size_t auth_continue_offset = 20;
	const size_t auth_auth_offset = 21;
	u8 auth_continue;

	if (!auth_session || !auth_session->valid)
		return TPM_AUTHFAIL;
	if (pack_byte_string(hmac_data, sizeof(hmac_data), "d",
			     0, command_code))
		return TPM_LIB_ERROR;
	if (response_len0 < TPM_RESPONSE_HEADER_LENGTH)
		return TPM_LIB_ERROR;

	sha1_starts(&hash_ctx);
	sha1_update(&hash_ctx, response + return_code_offset, 4);
	sha1_update(&hash_ctx, hmac_data, 4);
	if (response_len0 > TPM_RESPONSE_HEADER_LENGTH + handles_len)
		sha1_update(&hash_ctx,
			    response + TPM_RESPONSE_HEADER_LENGTH + handles_len,
			    response_len0 - TPM_RESPONSE_HEADER_LENGTH
			    - handles_len);
	sha1_finish(&hash_ctx, hmac_data);

	memcpy(auth_session->nonce_even, response_auth, DIGEST_LENGTH);
	auth_continue = ((u8 *)response_auth)[auth_continue_offset];
	if (pack_byte_string(hmac_data, sizeof(hmac_data), "ssb",
			     DIGEST_LENGTH,
			     response_auth,
			     DIGEST_LENGTH,
			     2 * DIGEST_LENGTH,
			     auth_session->nonce_odd,
			     DIGEST_LENGTH,
			     3 * DIGEST_LENGTH,
			     auth_continue))
		return TPM_LIB_ERROR;

	sha1_hmac(auth, DIGEST_LENGTH, hmac_data, sizeof(hmac_data),
		  computed_auth);

	if (memcmp(computed_auth, response_auth + auth_auth_offset,
		   DIGEST_LENGTH))
		return TPM_AUTHFAIL;

	return TPM_SUCCESS;
}

u32 tpm_terminate_auth_session(struct udevice *dev, u32 auth_handle)
{
	const u8 command[18] = {
		0x00, 0xc1,		/* TPM_TAG */
		0x00, 0x00, 0x00, 0x00,	/* parameter size */
		0x00, 0x00, 0x00, 0xba,	/* TPM_COMMAND_CODE */
		0x00, 0x00, 0x00, 0x00,	/* TPM_HANDLE */
		0x00, 0x00, 0x00, 0x02,	/* TPM_RESOURCE_TYPE */
	};
	const size_t req_handle_offset = TPM_REQUEST_HEADER_LENGTH;
	u8 request[COMMAND_BUFFER_SIZE];

	if (pack_byte_string(request, sizeof(request), "sd",
			     0, command, sizeof(command),
			     req_handle_offset, auth_handle))
		return TPM_LIB_ERROR;
	if (oiap_session.valid && oiap_session.handle == auth_handle)
		oiap_session.valid = 0;

	return tpm_sendrecv_command(dev, request, NULL, NULL);
}

u32 tpm_end_oiap(struct udevice *dev)
{
	u32 err = TPM_SUCCESS;

	if (oiap_session.valid)
		err = tpm_terminate_auth_session(dev, oiap_session.handle);
	return err;
}

u32 tpm_oiap(struct udevice *dev, u32 *auth_handle)
{
	const u8 command[10] = {
		0x00, 0xc1,		/* TPM_TAG */
		0x00, 0x00, 0x00, 0x0a,	/* parameter size */
		0x00, 0x00, 0x00, 0x0a,	/* TPM_COMMAND_CODE */
	};
	const size_t res_auth_handle_offset = TPM_RESPONSE_HEADER_LENGTH;
	const size_t res_nonce_even_offset = TPM_RESPONSE_HEADER_LENGTH + 4;
	u8 response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	u32 err;

	if (oiap_session.valid)
		tpm_terminate_auth_session(dev, oiap_session.handle);

	err = tpm_sendrecv_command(dev, command, response, &response_length);
	if (err)
		return err;
	if (unpack_byte_string(response, response_length, "ds",
			       res_auth_handle_offset, &oiap_session.handle,
			       res_nonce_even_offset, &oiap_session.nonce_even,
			       (u32)DIGEST_LENGTH))
		return TPM_LIB_ERROR;
	oiap_session.valid = 1;
	if (auth_handle)
		*auth_handle = oiap_session.handle;
	return 0;
}

u32 tpm_load_key2_oiap(struct udevice *dev, u32 parent_handle, const void *key,
		       size_t key_length, const void *parent_key_usage_auth,
		       u32 *key_handle)
{
	const u8 command[14] = {
		0x00, 0xc2,		/* TPM_TAG */
		0x00, 0x00, 0x00, 0x00,	/* parameter size */
		0x00, 0x00, 0x00, 0x41,	/* TPM_COMMAND_CODE */
		0x00, 0x00, 0x00, 0x00,	/* parent handle */
	};
	const size_t req_size_offset = 2;
	const size_t req_parent_handle_offset = TPM_REQUEST_HEADER_LENGTH;
	const size_t req_key_offset = TPM_REQUEST_HEADER_LENGTH + 4;
	const size_t res_handle_offset = TPM_RESPONSE_HEADER_LENGTH;
	u8 request[sizeof(command) + TPM_KEY12_MAX_LENGTH +
		   TPM_REQUEST_AUTH_LENGTH];
	u8 response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	u32 err;

	if (!oiap_session.valid) {
		err = tpm_oiap(dev, NULL);
		if (err)
			return err;
	}
	if (pack_byte_string(request, sizeof(request), "sdds",
			     0, command, sizeof(command),
			     req_size_offset,
			     sizeof(command) + key_length
			     + TPM_REQUEST_AUTH_LENGTH,
			     req_parent_handle_offset, parent_handle,
			     req_key_offset, key, key_length
		))
		return TPM_LIB_ERROR;

	err = create_request_auth(request, sizeof(command) + key_length, 4,
				  &oiap_session,
				  request + sizeof(command) + key_length,
				  parent_key_usage_auth);
	if (err)
		return err;
	err = tpm_sendrecv_command(dev, request, response, &response_length);
	if (err) {
		if (err == TPM_AUTHFAIL)
			oiap_session.valid = 0;
		return err;
	}

	err = verify_response_auth(0x00000041, response,
				   response_length - TPM_RESPONSE_AUTH_LENGTH,
				   4, &oiap_session,
				   response + response_length -
				   TPM_RESPONSE_AUTH_LENGTH,
				   parent_key_usage_auth);
	if (err)
		return err;

	if (key_handle) {
		if (unpack_byte_string(response, response_length, "d",
				       res_handle_offset, key_handle))
			return TPM_LIB_ERROR;
	}

	return 0;
}

u32 tpm_get_pub_key_oiap(struct udevice *dev, u32 key_handle,
			 const void *usage_auth, void *pubkey,
			 size_t *pubkey_len)
{
	const u8 command[14] = {
		0x00, 0xc2,		/* TPM_TAG */
		0x00, 0x00, 0x00, 0x00,	/* parameter size */
		0x00, 0x00, 0x00, 0x21,	/* TPM_COMMAND_CODE */
		0x00, 0x00, 0x00, 0x00,	/* key handle */
	};
	const size_t req_size_offset = 2;
	const size_t req_key_handle_offset = TPM_REQUEST_HEADER_LENGTH;
	const size_t res_pubkey_offset = TPM_RESPONSE_HEADER_LENGTH;
	u8 request[sizeof(command) + TPM_REQUEST_AUTH_LENGTH];
	u8 response[TPM_RESPONSE_HEADER_LENGTH + TPM_PUBKEY_MAX_LENGTH +
		    TPM_RESPONSE_AUTH_LENGTH];
	size_t response_length = sizeof(response);
	u32 err;

	if (!oiap_session.valid) {
		err = tpm_oiap(dev, NULL);
		if (err)
			return err;
	}
	if (pack_byte_string(request, sizeof(request), "sdd",
			     0, command, sizeof(command),
			     req_size_offset,
			     (u32)(sizeof(command)
			     + TPM_REQUEST_AUTH_LENGTH),
			     req_key_handle_offset, key_handle
		))
		return TPM_LIB_ERROR;
	err = create_request_auth(request, sizeof(command), 4, &oiap_session,
				  request + sizeof(command), usage_auth);
	if (err)
		return err;
	err = tpm_sendrecv_command(dev, request, response, &response_length);
	if (err) {
		if (err == TPM_AUTHFAIL)
			oiap_session.valid = 0;
		return err;
	}
	err = verify_response_auth(0x00000021, response,
				   response_length - TPM_RESPONSE_AUTH_LENGTH,
				   0, &oiap_session,
				   response + response_length -
				   TPM_RESPONSE_AUTH_LENGTH,
				   usage_auth);
	if (err)
		return err;

	if (pubkey) {
		if ((response_length - TPM_RESPONSE_HEADER_LENGTH
		     - TPM_RESPONSE_AUTH_LENGTH) > *pubkey_len)
			return TPM_LIB_ERROR;
		*pubkey_len = response_length - TPM_RESPONSE_HEADER_LENGTH
			- TPM_RESPONSE_AUTH_LENGTH;
		memcpy(pubkey, response + res_pubkey_offset,
		       response_length - TPM_RESPONSE_HEADER_LENGTH
		       - TPM_RESPONSE_AUTH_LENGTH);
	}

	return 0;
}

#ifdef CONFIG_TPM_LOAD_KEY_BY_SHA1
u32 tpm_find_key_sha1(struct udevice *dev, const u8 auth[20],
		      const u8 pubkey_digest[20], u32 *handle)
{
	u16 key_count;
	u32 key_handles[10];
	u8 buf[288];
	u8 *ptr;
	u32 err;
	u8 digest[20];
	size_t buf_len;
	unsigned int i;

	/* fetch list of already loaded keys in the TPM */
	err = tpm_get_capability(dev, TPM_CAP_HANDLE, TPM_RT_KEY, buf,
				 sizeof(buf));
	if (err)
		return -1;
	key_count = get_unaligned_be16(buf);
	ptr = buf + 2;
	for (i = 0; i < key_count; ++i, ptr += 4)
		key_handles[i] = get_unaligned_be32(ptr);

	/* now search a(/ the) key which we can access with the given auth */
	for (i = 0; i < key_count; ++i) {
		buf_len = sizeof(buf);
		err = tpm_get_pub_key_oiap(key_handles[i], auth, buf, &buf_len);
		if (err && err != TPM_AUTHFAIL)
			return -1;
		if (err)
			continue;
		sha1_csum(buf, buf_len, digest);
		if (!memcmp(digest, pubkey_digest, 20)) {
			*handle = key_handles[i];
			return 0;
		}
	}
	return 1;
}
#endif /* CONFIG_TPM_LOAD_KEY_BY_SHA1 */

#endif /* CONFIG_TPM_AUTH_SESSIONS */

u32 tpm_get_random(struct udevice *dev, void *data, u32 count)
{
	const u8 command[14] = {
		0x0, 0xc1,		/* TPM_TAG */
		0x0, 0x0, 0x0, 0xe,	/* parameter size */
		0x0, 0x0, 0x0, 0x46,	/* TPM_COMMAND_CODE */
	};
	const size_t length_offset = 10;
	const size_t data_size_offset = 10;
	const size_t data_offset = 14;
	u8 buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	u32 data_size;
	u8 *out = data;

	while (count > 0) {
		u32 this_bytes = min((size_t)count,
				     sizeof(response) - data_offset);
		u32 err;

		if (pack_byte_string(buf, sizeof(buf), "sd",
				     0, command, sizeof(command),
				     length_offset, this_bytes))
			return TPM_LIB_ERROR;
		err = tpm_sendrecv_command(dev, buf, response,
					   &response_length);
		if (err)
			return err;
		if (unpack_byte_string(response, response_length, "d",
				       data_size_offset, &data_size))
			return TPM_LIB_ERROR;
		if (data_size > count)
			return TPM_LIB_ERROR;
		if (unpack_byte_string(response, response_length, "s",
				       data_offset, out, data_size))
			return TPM_LIB_ERROR;

		count -= data_size;
		out += data_size;
	}

	return 0;
}
