// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Reinhard Pfau, Guntermann & Drunck GmbH, reinhard.pfau@gdsys.cc
 */

#include <common.h>
#include <malloc.h>
#include <fs.h>
#include <i2c.h>
#include <mmc.h>
#include <tpm-v1.h>
#include <u-boot/sha1.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <pca9698.h>

#include "hre.h"

/* other constants */
enum {
	ESDHC_BOOT_IMAGE_SIG_OFS	= 0x40,
	ESDHC_BOOT_IMAGE_SIZE_OFS	= 0x48,
	ESDHC_BOOT_IMAGE_ADDR_OFS	= 0x50,
	ESDHC_BOOT_IMAGE_TARGET_OFS	= 0x58,
	ESDHC_BOOT_IMAGE_ENTRY_OFS	= 0x60,
};

enum {
	I2C_SOC_0 = 0,
	I2C_SOC_1 = 1,
};

enum access_mode {
	HREG_NONE	= 0,
	HREG_RD		= 1,
	HREG_WR		= 2,
	HREG_RDWR	= 3,
};

/* register constants */
enum {
	FIX_HREG_DEVICE_ID_HASH	= 0,
	FIX_HREG_UNUSED1	= 1,
	FIX_HREG_UNUSED2	= 2,
	FIX_HREG_VENDOR		= 3,
	COUNT_FIX_HREGS
};

static struct h_reg pcr_hregs[24];
static struct h_reg fix_hregs[COUNT_FIX_HREGS];
static struct h_reg var_hregs[8];

/* hre opcodes */
enum {
	/* opcodes w/o data */
	HRE_NOP		= 0x00,
	HRE_SYNC	= HRE_NOP,
	HRE_CHECK0	= 0x01,
	/* opcodes w/o data, w/ sync dst */
	/* opcodes w/ data */
	HRE_LOAD	= 0x81,
	/* opcodes w/data, w/sync dst */
	HRE_XOR		= 0xC1,
	HRE_AND		= 0xC2,
	HRE_OR		= 0xC3,
	HRE_EXTEND	= 0xC4,
	HRE_LOADKEY	= 0xC5,
};

/* hre errors */
enum {
	HRE_E_OK	= 0,
	HRE_E_TPM_FAILURE,
	HRE_E_INVALID_HREG,
};

static uint64_t device_id;
static uint64_t device_cl;
static uint64_t device_type;

static uint32_t platform_key_handle;

static uint32_t hre_tpm_err;
static int hre_err = HRE_E_OK;

#define IS_PCR_HREG(spec) ((spec) & 0x20)
#define IS_FIX_HREG(spec) (((spec) & 0x38) == 0x08)
#define IS_VAR_HREG(spec) (((spec) & 0x38) == 0x10)
#define HREG_IDX(spec) ((spec) & (IS_PCR_HREG(spec) ? 0x1f : 0x7))

static const uint8_t vendor[] = "Guntermann & Drunck";

/**
 * @brief get the size of a given (TPM) NV area
 * @param tpm		TPM device
 * @param index	NV index of the area to get size for
 * @param size	pointer to the size
 * @return 0 on success, != 0 on error
 */
static int get_tpm_nv_size(struct udevice *tpm, uint32_t index, uint32_t *size)
{
	uint32_t err;
	uint8_t info[72];
	uint8_t *ptr;
	uint16_t v16;

	err = tpm_get_capability(tpm, TPM_CAP_NV_INDEX, index,
				 info, sizeof(info));
	if (err) {
		printf("tpm_get_capability(CAP_NV_INDEX, %08x) failed: %u\n",
		       index, err);
		return 1;
	}

	/* skip tag and nvIndex */
	ptr = info + 6;
	/* skip 2 pcr info fields */
	v16 = get_unaligned_be16(ptr);
	ptr += 2 + v16 + 1 + 20;
	v16 = get_unaligned_be16(ptr);
	ptr += 2 + v16 + 1 + 20;
	/* skip permission and flags */
	ptr += 6 + 3;

	*size = get_unaligned_be32(ptr);
	return 0;
}

/**
 * @brief search for a key by usage auth and pub key hash.
 * @param tpm		TPM device
 * @param auth	usage auth of the key to search for
 * @param pubkey_digest	(SHA1) hash of the pub key structure of the key
 * @param[out] handle	the handle of the key iff found
 * @return 0 if key was found in TPM; != 0 if not.
 */
static int find_key(struct udevice *tpm, const uint8_t auth[20],
		    const uint8_t pubkey_digest[20], uint32_t *handle)
{
	uint16_t key_count;
	uint32_t key_handles[10];
	uint8_t buf[288];
	uint8_t *ptr;
	uint32_t err;
	uint8_t digest[20];
	size_t buf_len;
	unsigned int i;

	/* fetch list of already loaded keys in the TPM */
	err = tpm_get_capability(tpm, TPM_CAP_HANDLE, TPM_RT_KEY, buf,
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
		err = tpm_get_pub_key_oiap(tpm, key_handles[i], auth, buf,
					   &buf_len);
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

/**
 * @brief read CCDM common data from TPM NV
 * @param tpm		TPM device
 * @return 0 if CCDM common data was found and read, !=0 if something failed.
 */
static int read_common_data(struct udevice *tpm)
{
	uint32_t size = 0;
	uint32_t err;
	uint8_t buf[256];
	sha1_context ctx;

	if (get_tpm_nv_size(tpm, NV_COMMON_DATA_INDEX, &size) ||
	    size < NV_COMMON_DATA_MIN_SIZE)
		return 1;
	err = tpm_nv_read_value(tpm, NV_COMMON_DATA_INDEX,
				buf, min(sizeof(buf), size));
	if (err) {
		printf("tpm_nv_read_value() failed: %u\n", err);
		return 1;
	}

	device_id = get_unaligned_be64(buf);
	device_cl = get_unaligned_be64(buf + 8);
	device_type = get_unaligned_be64(buf + 16);

	sha1_starts(&ctx);
	sha1_update(&ctx, buf, 24);
	sha1_finish(&ctx, fix_hregs[FIX_HREG_DEVICE_ID_HASH].digest);
	fix_hregs[FIX_HREG_DEVICE_ID_HASH].valid = true;

	platform_key_handle = get_unaligned_be32(buf + 24);

	return 0;
}

/**
 * @brief get pointer to  hash register by specification
 * @param spec	specification of a hash register
 * @return pointer to hash register or NULL if @a spec does not qualify a
 * valid hash register; NULL else.
 */
static struct h_reg *get_hreg(uint8_t spec)
{
	uint8_t idx;

	idx = HREG_IDX(spec);
	if (IS_FIX_HREG(spec)) {
		if (idx < ARRAY_SIZE(fix_hregs))
			return fix_hregs + idx;
		hre_err = HRE_E_INVALID_HREG;
	} else if (IS_PCR_HREG(spec)) {
		if (idx < ARRAY_SIZE(pcr_hregs))
			return pcr_hregs + idx;
		hre_err = HRE_E_INVALID_HREG;
	} else if (IS_VAR_HREG(spec)) {
		if (idx < ARRAY_SIZE(var_hregs))
			return var_hregs + idx;
		hre_err = HRE_E_INVALID_HREG;
	}
	return NULL;
}

/**
 * @brief get pointer of a hash register by specification and usage.
 * @param tpm		TPM device
 * @param spec	specification of a hash register
 * @param mode	access mode (read or write or read/write)
 * @return pointer to hash register if found and valid; NULL else.
 *
 * This func uses @a get_reg() to determine the hash register for a given spec.
 * If a register is found it is validated according to the desired access mode.
 * The value of automatic registers (PCR register and fixed registers) is
 * loaded or computed on read access.
 */
static struct h_reg *access_hreg(struct udevice *tpm, uint8_t spec,
				 enum access_mode mode)
{
	struct h_reg *result;

	result = get_hreg(spec);
	if (!result)
		return NULL;

	if (mode & HREG_WR) {
		if (IS_FIX_HREG(spec)) {
			hre_err = HRE_E_INVALID_HREG;
			return NULL;
		}
	}
	if (mode & HREG_RD) {
		if (!result->valid) {
			if (IS_PCR_HREG(spec)) {
				hre_tpm_err = tpm_pcr_read(tpm, HREG_IDX(spec),
					result->digest, 20);
				result->valid = (hre_tpm_err == TPM_SUCCESS);
			} else if (IS_FIX_HREG(spec)) {
				switch (HREG_IDX(spec)) {
				case FIX_HREG_DEVICE_ID_HASH:
					read_common_data(tpm);
					break;
				case FIX_HREG_VENDOR:
					memcpy(result->digest, vendor, 20);
					result->valid = true;
					break;
				}
			} else {
				result->valid = true;
			}
		}
		if (!result->valid) {
			hre_err = HRE_E_INVALID_HREG;
			return NULL;
		}
	}

	return result;
}

static void *compute_and(void *_dst, const void *_src, size_t n)
{
	uint8_t *dst = _dst;
	const uint8_t *src = _src;
	size_t i;

	for (i = n; i-- > 0; )
		*dst++ &= *src++;

	return _dst;
}

static void *compute_or(void *_dst, const void *_src, size_t n)
{
	uint8_t *dst = _dst;
	const uint8_t *src = _src;
	size_t i;

	for (i = n; i-- > 0; )
		*dst++ |= *src++;

	return _dst;
}

static void *compute_xor(void *_dst, const void *_src, size_t n)
{
	uint8_t *dst = _dst;
	const uint8_t *src = _src;
	size_t i;

	for (i = n; i-- > 0; )
		*dst++ ^= *src++;

	return _dst;
}

static void *compute_extend(void *_dst, const void *_src, size_t n)
{
	uint8_t digest[20];
	sha1_context ctx;

	sha1_starts(&ctx);
	sha1_update(&ctx, _dst, n);
	sha1_update(&ctx, _src, n);
	sha1_finish(&ctx, digest);
	memcpy(_dst, digest, min(n, sizeof(digest)));

	return _dst;
}

static int hre_op_loadkey(struct udevice *tpm, struct h_reg *src_reg,
			  struct h_reg *dst_reg, const void *key,
			  size_t key_size)
{
	uint32_t parent_handle;
	uint32_t key_handle;

	if (!src_reg || !dst_reg || !src_reg->valid || !dst_reg->valid)
		return -1;
	if (find_key(tpm, src_reg->digest, dst_reg->digest, &parent_handle))
		return -1;
	hre_tpm_err = tpm_load_key2_oiap(tpm, parent_handle, key, key_size,
					 src_reg->digest, &key_handle);
	if (hre_tpm_err) {
		hre_err = HRE_E_TPM_FAILURE;
		return -1;
	}

	return 0;
}

/**
 * @brief executes the next opcode on the hash register engine.
 * @param tpm		TPM device
 * @param[in,out] ip	pointer to the opcode (instruction pointer)
 * @param[in,out] code_size	(remaining) size of the code
 * @return new instruction pointer on success, NULL on error.
 */
static const uint8_t *hre_execute_op(struct udevice *tpm, const uint8_t **ip,
				     size_t *code_size)
{
	bool dst_modified = false;
	uint32_t ins;
	uint8_t opcode;
	uint8_t src_spec;
	uint8_t dst_spec;
	uint16_t data_size;
	struct h_reg *src_reg, *dst_reg;
	uint8_t buf[20];
	const uint8_t *src_buf, *data;
	uint8_t *ptr;
	int i;
	void * (*bin_func)(void *, const void *, size_t);

	if (*code_size < 4)
		return NULL;

	ins = get_unaligned_be32(*ip);
	opcode = **ip;
	data = *ip + 4;
	src_spec = (ins >> 18) & 0x3f;
	dst_spec = (ins >> 12) & 0x3f;
	data_size = (ins & 0x7ff);

	debug("HRE: ins=%08x (op=%02x, s=%02x, d=%02x, L=%d)\n", ins,
	      opcode, src_spec, dst_spec, data_size);

	if ((opcode & 0x80) && (data_size + 4) > *code_size)
		return NULL;

	src_reg = access_hreg(tpm, src_spec, HREG_RD);
	if (hre_err || hre_tpm_err)
		return NULL;
	dst_reg = access_hreg(tpm, dst_spec,
			      (opcode & 0x40) ? HREG_RDWR : HREG_WR);
	if (hre_err || hre_tpm_err)
		return NULL;

	switch (opcode) {
	case HRE_NOP:
		goto end;
	case HRE_CHECK0:
		if (src_reg) {
			for (i = 0; i < 20; ++i) {
				if (src_reg->digest[i])
					return NULL;
			}
		}
		break;
	case HRE_LOAD:
		bin_func = memcpy;
		goto do_bin_func;
	case HRE_XOR:
		bin_func = compute_xor;
		goto do_bin_func;
	case HRE_AND:
		bin_func = compute_and;
		goto do_bin_func;
	case HRE_OR:
		bin_func = compute_or;
		goto do_bin_func;
	case HRE_EXTEND:
		bin_func = compute_extend;
do_bin_func:
		if (!dst_reg)
			return NULL;
		if (src_reg) {
			src_buf = src_reg->digest;
		} else {
			if (!data_size) {
				memset(buf, 0, 20);
				src_buf = buf;
			} else if (data_size == 1) {
				memset(buf, *data, 20);
				src_buf = buf;
			} else if (data_size >= 20) {
				src_buf = data;
			} else {
				src_buf = buf;
				for (ptr = (uint8_t *)src_buf, i = 20; i > 0;
					i -= data_size, ptr += data_size)
					memcpy(ptr, data,
					       min_t(size_t, i, data_size));
			}
		}
		bin_func(dst_reg->digest, src_buf, 20);
		dst_reg->valid = true;
		dst_modified = true;
		break;
	case HRE_LOADKEY:
		if (hre_op_loadkey(tpm, src_reg, dst_reg, data, data_size))
			return NULL;
		break;
	default:
		return NULL;
	}

	if (dst_reg && dst_modified && IS_PCR_HREG(dst_spec)) {
		hre_tpm_err = tpm_extend(tpm, HREG_IDX(dst_spec),
					 dst_reg->digest, dst_reg->digest);
		if (hre_tpm_err) {
			hre_err = HRE_E_TPM_FAILURE;
			return NULL;
		}
	}
end:
	*ip += 4;
	*code_size -= 4;
	if (opcode & 0x80) {
		*ip += data_size;
		*code_size -= data_size;
	}

	return *ip;
}

/**
 * @brief runs a program on the hash register engine.
 * @param tpm		TPM device
 * @param code		pointer to the (HRE) code.
 * @param code_size	size of the code (in bytes).
 * @return 0 on success, != 0 on failure.
 */
int hre_run_program(struct udevice *tpm, const uint8_t *code, size_t code_size)
{
	size_t code_left;
	const uint8_t *ip = code;

	code_left = code_size;
	hre_tpm_err = 0;
	hre_err = HRE_E_OK;
	while (code_left > 0)
		if (!hre_execute_op(tpm, &ip, &code_left))
			return -1;

	return hre_err;
}

int hre_verify_program(struct key_program *prg)
{
	uint32_t crc;

	crc = crc32(0, prg->code, prg->code_size);

	if (crc != prg->code_crc) {
		printf("HRC crc mismatch: %08x != %08x\n",
		       crc, prg->code_crc);
		return 1;
	}
	return 0;
}
