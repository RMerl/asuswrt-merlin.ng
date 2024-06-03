// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Reinhard Pfau, Guntermann & Drunck GmbH, reinhard.pfau@gdsys.cc
 */

/* TODO: some more #ifdef's to avoid unneeded code for stage 1 / stage 2 */

#ifdef CCDM_ID_DEBUG
#define DEBUG
#endif

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <fs.h>
#include <i2c.h>
#include <mmc.h>
#include <tpm-v1.h>
#include <u-boot/sha1.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <pca9698.h>

#undef CCDM_FIRST_STAGE
#undef CCDM_SECOND_STAGE
#undef CCDM_AUTO_FIRST_STAGE

#ifdef CONFIG_DEVELOP
#define CCDM_DEVELOP
#endif

#ifdef CONFIG_TRAILBLAZER
#define CCDM_FIRST_STAGE
#undef CCDM_SECOND_STAGE
#else
#undef CCDM_FIRST_STAGE
#define CCDM_SECOND_STAGE
#endif

#if defined(CCDM_DEVELOP) && defined(CCDM_SECOND_STAGE) && \
	!defined(CCCM_FIRST_STAGE)
#define CCDM_AUTO_FIRST_STAGE
#endif

/* CCDM specific contants */
enum {
	/* NV indices */
	NV_COMMON_DATA_INDEX	= 0x40000001,
	/* magics for key blob chains */
	MAGIC_KEY_PROGRAM	= 0x68726500,
	MAGIC_HMAC		= 0x68616300,
	MAGIC_END_OF_CHAIN	= 0x00000000,
	/* sizes */
	NV_COMMON_DATA_MIN_SIZE	= 3 * sizeof(uint64_t) + 2 * sizeof(uint16_t),
};

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

struct key_program {
	uint32_t magic;
	uint32_t code_crc;
	uint32_t code_size;
	uint8_t code[];
};

struct h_reg {
	bool valid;
	uint8_t digest[20];
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
	FIX_HREG_SELF_HASH	= 1,
	FIX_HREG_STAGE2_HASH	= 2,
	FIX_HREG_VENDOR		= 3,
	COUNT_FIX_HREGS
};


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

static void(*bl2_entry)(void);

static struct h_reg pcr_hregs[24];
static struct h_reg fix_hregs[COUNT_FIX_HREGS];
static struct h_reg var_hregs[8];
static uint32_t hre_tpm_err;
static int hre_err = HRE_E_OK;

#define IS_PCR_HREG(spec) ((spec) & 0x20)
#define IS_FIX_HREG(spec) (((spec) & 0x38) == 0x08)
#define IS_VAR_HREG(spec) (((spec) & 0x38) == 0x10)
#define HREG_IDX(spec) ((spec) & (IS_PCR_HREG(spec) ? 0x1f : 0x7))

static int get_tpm(struct udevice **devp)
{
	int rc;

	rc = uclass_first_device_err(UCLASS_TPM, devp);
	if (rc) {
		printf("Could not find TPM (ret=%d)\n", rc);
		return CMD_RET_FAILURE;
	}

	return 0;
}

static const uint8_t vendor[] = "Guntermann & Drunck";

/**
 * @brief read a bunch of data from MMC into memory.
 *
 * @param mmc	pointer to the mmc structure to use.
 * @param src	offset where the data starts on MMC/SD device (in bytes).
 * @param dst	pointer to the location where the read data should be stored.
 * @param size	number of bytes to read from the MMC/SD device.
 * @return number of bytes read or -1 on error.
 */
static int ccdm_mmc_read(struct mmc *mmc, u64 src, u8 *dst, int size)
{
	int result = 0;
	u32 blk_len, ofs;
	ulong block_no, n, cnt;
	u8 *tmp_buf = NULL;

	if (size <= 0)
		goto end;

	blk_len = mmc->read_bl_len;
	tmp_buf = malloc(blk_len);
	if (!tmp_buf)
		goto failure;
	block_no = src / blk_len;
	ofs = src % blk_len;

	if (ofs) {
		n = mmc->block_dev.block_read(&mmc->block_dev, block_no++, 1,
			tmp_buf);
		if (!n)
			goto failure;
		result = min(size, (int)(blk_len - ofs));
		memcpy(dst, tmp_buf + ofs, result);
		dst += result;
		size -= result;
	}
	cnt = size / blk_len;
	if (cnt) {
		n = mmc->block_dev.block_read(&mmc->block_dev, block_no, cnt,
			dst);
		if (n != cnt)
			goto failure;
		size -= cnt * blk_len;
		result += cnt * blk_len;
		dst += cnt * blk_len;
		block_no += cnt;
	}
	if (size) {
		n = mmc->block_dev.block_read(&mmc->block_dev, block_no++, 1,
			tmp_buf);
		if (!n)
			goto failure;
		memcpy(dst, tmp_buf, size);
		result += size;
	}
	goto end;
failure:
	result = -1;
end:
	if (tmp_buf)
		free(tmp_buf);
	return result;
}

/**
 * @brief returns a location where the 2nd stage bootloader can be(/ is) placed.
 *
 * @return pointer to the location for/of the 2nd stage bootloader
 */
static u8 *get_2nd_stage_bl_location(ulong target_addr)
{
	ulong addr;
#ifdef CCDM_SECOND_STAGE
	addr = env_get_ulong("loadaddr", 16, CONFIG_LOADADDR);
#else
	addr = target_addr;
#endif
	return (u8 *)(addr);
}


#ifdef CCDM_SECOND_STAGE
/**
 * @brief returns a location where the image can be(/ is) placed.
 *
 * @return pointer to the location for/of the image
 */
static u8 *get_image_location(void)
{
	ulong addr;
	/* TODO use other area? */
	addr = env_get_ulong("loadaddr", 16, CONFIG_LOADADDR);
	return (u8 *)(addr);
}
#endif

/**
 * @brief get the size of a given (TPM) NV area
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
 * @return 0 if CCDM common data was found and read, !=0 if something failed.
 */
static int read_common_data(struct udevice *tpm)
{
	uint32_t size;
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
 * @brief compute hash of bootloader itself.
 * @param[out] dst	hash register where the hash should be stored
 * @return 0 on success, != 0 on failure.
 *
 * @note MUST be called at a time where the boot loader is accessible at the
 * configured location (; so take care when code is reallocated).
 */
static int compute_self_hash(struct h_reg *dst)
{
	sha1_csum((const uint8_t *)CONFIG_SYS_MONITOR_BASE,
		  CONFIG_SYS_MONITOR_LEN, dst->digest);
	dst->valid = true;
	return 0;
}

int ccdm_compute_self_hash(void)
{
	if (!fix_hregs[FIX_HREG_SELF_HASH].valid)
		compute_self_hash(&fix_hregs[FIX_HREG_SELF_HASH]);
	return 0;
}

/**
 * @brief compute the hash of the 2nd stage boot loader (on SD card)
 * @param[out] dst	hash register to store the computed hash
 * @return 0 on success, != 0 on failure
 *
 * Determines the size and location of the 2nd stage boot loader on SD card,
 * loads the 2nd stage boot loader and computes the (SHA1) hash value.
 * Within the 1st stage boot loader, the 2nd stage boot loader is loaded at
 * the desired memory location and the variable @a bl2_entry is set.
 *
 * @note This sets the variable @a bl2_entry to the entry point when the
 * 2nd stage boot loader is loaded at its configured memory location.
 */
static int compute_second_stage_hash(struct h_reg *dst)
{
	int result = 0;
	u32 code_len, code_offset, target_addr, exec_entry;
	struct mmc *mmc;
	u8 *load_addr = NULL;
	u8 buf[128];

	mmc = find_mmc_device(0);
	if (!mmc)
		goto failure;
	mmc_init(mmc);

	if (ccdm_mmc_read(mmc, 0, buf, sizeof(buf)) < 0)
		goto failure;

	code_offset = *(u32 *)(buf + ESDHC_BOOT_IMAGE_ADDR_OFS);
	code_len = *(u32 *)(buf + ESDHC_BOOT_IMAGE_SIZE_OFS);
	target_addr = *(u32 *)(buf + ESDHC_BOOT_IMAGE_TARGET_OFS);
	exec_entry =  *(u32 *)(buf + ESDHC_BOOT_IMAGE_ENTRY_OFS);

	load_addr = get_2nd_stage_bl_location(target_addr);
	if (load_addr == (u8 *)target_addr)
		bl2_entry = (void(*)(void))exec_entry;

	if (ccdm_mmc_read(mmc, code_offset, load_addr, code_len) < 0)
		goto failure;

	sha1_csum(load_addr, code_len, dst->digest);
	dst->valid = true;

	goto end;
failure:
	result = 1;
	bl2_entry = NULL;
end:
	return result;
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
				case FIX_HREG_SELF_HASH:
					ccdm_compute_self_hash();
					break;
				case FIX_HREG_STAGE2_HASH:
					compute_second_stage_hash(result);
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
	/* TODO remember key handle somehow? */

	return 0;
}

/**
 * @brief executes the next opcode on the hash register engine.
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
 * @param code		pointer to the (HRE) code.
 * @param code_size	size of the code (in bytes).
 * @return 0 on success, != 0 on failure.
 */
static int hre_run_program(struct udevice *tpm, const uint8_t *code,
			   size_t code_size)
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

static int check_hmac(struct key_program *hmac,
	const uint8_t *data, size_t data_size)
{
	uint8_t key[20], computed_hmac[20];
	uint32_t type;

	type = get_unaligned_be32(hmac->code);
	if (type != 0)
		return 1;
	memset(key, 0, sizeof(key));
	compute_extend(key, pcr_hregs[1].digest, 20);
	compute_extend(key, pcr_hregs[2].digest, 20);
	compute_extend(key, pcr_hregs[3].digest, 20);
	compute_extend(key, pcr_hregs[4].digest, 20);

	sha1_hmac(key, sizeof(key), data, data_size, computed_hmac);

	return memcmp(computed_hmac, hmac->code + 4, 20);
}

static int verify_program(struct key_program *prg)
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

#if defined(CCDM_FIRST_STAGE) || (defined CCDM_AUTO_FIRST_STAGE)
static struct key_program *load_sd_key_program(void)
{
	u32 code_len, code_offset;
	struct mmc *mmc;
	u8 buf[128];
	struct key_program *result = NULL, *hmac = NULL;
	struct key_program header;

	mmc = find_mmc_device(0);
	if (!mmc)
		return NULL;
	mmc_init(mmc);

	if (ccdm_mmc_read(mmc, 0, buf, sizeof(buf)) <= 0)
		goto failure;

	code_offset = *(u32 *)(buf + ESDHC_BOOT_IMAGE_ADDR_OFS);
	code_len = *(u32 *)(buf + ESDHC_BOOT_IMAGE_SIZE_OFS);

	code_offset += code_len;
	/* TODO: the following needs to be the size of the 2nd stage env */
	code_offset += CONFIG_ENV_SIZE;

	if (ccdm_mmc_read(mmc, code_offset, buf, 4*3) < 0)
		goto failure;

	header.magic = get_unaligned_be32(buf);
	header.code_crc = get_unaligned_be32(buf + 4);
	header.code_size = get_unaligned_be32(buf + 8);

	if (header.magic != MAGIC_KEY_PROGRAM)
		goto failure;

	result = malloc(sizeof(struct key_program) + header.code_size);
	if (!result)
		goto failure;
	*result = header;

	printf("load key program chunk from SD card (%u bytes) ",
	       header.code_size);
	code_offset += 12;
	if (ccdm_mmc_read(mmc, code_offset, result->code, header.code_size)
		< 0)
		goto failure;
	code_offset += header.code_size;
	puts("\n");

	if (verify_program(result))
		goto failure;

	if (ccdm_mmc_read(mmc, code_offset, buf, 4*3) < 0)
		goto failure;

	header.magic = get_unaligned_be32(buf);
	header.code_crc = get_unaligned_be32(buf + 4);
	header.code_size = get_unaligned_be32(buf + 8);

	if (header.magic == MAGIC_HMAC) {
		puts("check integrity\n");
		hmac = malloc(sizeof(struct key_program) + header.code_size);
		if (!hmac)
			goto failure;
		*hmac = header;
		code_offset += 12;
		if (ccdm_mmc_read(mmc, code_offset, hmac->code,
				  hmac->code_size) < 0)
			goto failure;
		if (verify_program(hmac))
			goto failure;
		if (check_hmac(hmac, result->code, result->code_size)) {
			puts("key program integrity could not be verified\n");
			goto failure;
		}
		puts("key program verified\n");
	}

	goto end;
failure:
	if (result)
		free(result);
	result = NULL;
end:
	if (hmac)
		free(hmac);

	return result;
}
#endif

#ifdef CCDM_SECOND_STAGE
/**
 * @brief load a key program from file system.
 * @param ifname	interface of the file system
 * @param dev_part_str	device part of the file system
 * @param fs_type	tyep of the file system
 * @param path		path of the file to load.
 * @return the loaded structure or NULL on failure.
 */
static struct key_program *load_key_chunk(const char *ifname,
	const char *dev_part_str, int fs_type,
	const char *path)
{
	struct key_program *result = NULL;
	struct key_program header;
	uint32_t crc;
	uint8_t buf[12];
	loff_t i;

	if (fs_set_blk_dev(ifname, dev_part_str, fs_type))
		goto failure;
	if (fs_read(path, (ulong)buf, 0, 12, &i) < 0)
		goto failure;
	if (i < 12)
		goto failure;
	header.magic = get_unaligned_be32(buf);
	header.code_crc = get_unaligned_be32(buf + 4);
	header.code_size = get_unaligned_be32(buf + 8);

	if (header.magic != MAGIC_HMAC && header.magic != MAGIC_KEY_PROGRAM)
		goto failure;

	result = malloc(sizeof(struct key_program) + header.code_size);
	if (!result)
		goto failure;
	if (fs_set_blk_dev(ifname, dev_part_str, fs_type))
		goto failure;
	if (fs_read(path, (ulong)result, 0,
		    sizeof(struct key_program) + header.code_size, &i) < 0)
		goto failure;
	if (i <= 0)
		goto failure;
	*result = header;

	crc = crc32(0, result->code, result->code_size);

	if (crc != result->code_crc) {
		printf("%s: HRC crc mismatch: %08x != %08x\n",
		       path, crc, result->code_crc);
		goto failure;
	}
	goto end;
failure:
	if (result) {
		free(result);
		result = NULL;
	}
end:
	return result;
}
#endif

#if defined(CCDM_FIRST_STAGE) || (defined CCDM_AUTO_FIRST_STAGE)
static const uint8_t prg_stage1_prepare[] = {
	0x00, 0x20, 0x00, 0x00, /* opcode: SYNC f0 */
	0x00, 0x24, 0x00, 0x00, /* opcode: SYNC f1 */
	0x01, 0x80, 0x00, 0x00, /* opcode: CHECK0 PCR0 */
	0x81, 0x22, 0x00, 0x00, /* opcode: LOAD PCR0, f0 */
	0x01, 0x84, 0x00, 0x00, /* opcode: CHECK0 PCR1 */
	0x81, 0x26, 0x10, 0x00, /* opcode: LOAD PCR1, f1 */
	0x01, 0x88, 0x00, 0x00, /* opcode: CHECK0 PCR2 */
	0x81, 0x2a, 0x20, 0x00, /* opcode: LOAD PCR2, f2 */
	0x01, 0x8c, 0x00, 0x00, /* opcode: CHECK0 PCR3 */
	0x81, 0x2e, 0x30, 0x00, /* opcode: LOAD PCR3, f3 */
};

static int first_stage_actions(struct udevice *tpm)
{
	int result = 0;
	struct key_program *sd_prg = NULL;

	puts("CCDM S1: start actions\n");
#ifndef CCDM_SECOND_STAGE
	if (tpm_continue_self_test(tpm))
		goto failure;
#else
	tpm_continue_self_test(tpm);
#endif
	mdelay(37);

	if (hre_run_program(tpm, prg_stage1_prepare,
			    sizeof(prg_stage1_prepare)))
		goto failure;

	sd_prg = load_sd_key_program();
	if (sd_prg) {
		if (hre_run_program(tpm, sd_prg->code, sd_prg->code_size))
			goto failure;
		puts("SD code run successfully\n");
	} else {
		puts("no key program found on SD\n");
		goto failure;
	}
	goto end;
failure:
	result = 1;
end:
	if (sd_prg)
		free(sd_prg);
	printf("CCDM S1: actions done (%d)\n", result);
	return result;
}
#endif

#ifdef CCDM_FIRST_STAGE
static int first_stage_init(void)
{
	struct udevice *tpm;
	int ret;

	puts("CCDM S1\n");
	ret = get_tpm(&tpm);
	if (ret || tpm_init(tpm) || tpm_startup(tpm, TPM_ST_CLEAR))
		return 1;
	ret = first_stage_actions(tpm);
#ifndef CCDM_SECOND_STAGE
	if (!ret) {
		if (bl2_entry)
			(*bl2_entry)();
		ret = 1;
	}
#endif
	return ret;
}
#endif

#ifdef CCDM_SECOND_STAGE
static const uint8_t prg_stage2_prepare[] = {
	0x00, 0x80, 0x00, 0x00, /* opcode: SYNC PCR0 */
	0x00, 0x84, 0x00, 0x00, /* opcode: SYNC PCR1 */
	0x00, 0x88, 0x00, 0x00, /* opcode: SYNC PCR2 */
	0x00, 0x8c, 0x00, 0x00, /* opcode: SYNC PCR3 */
	0x00, 0x90, 0x00, 0x00, /* opcode: SYNC PCR4 */
};

static const uint8_t prg_stage2_success[] = {
	0x81, 0x02, 0x40, 0x14, /* opcode: LOAD PCR4, #<20B data> */
	0x48, 0xfd, 0x95, 0x17, 0xe7, 0x54, 0x6b, 0x68, /* data */
	0x92, 0x31, 0x18, 0x05, 0xf8, 0x58, 0x58, 0x3c, /* data */
	0xe4, 0xd2, 0x81, 0xe0, /* data */
};

static const uint8_t prg_stage_fail[] = {
	0x81, 0x01, 0x00, 0x14, /* opcode: LOAD v0, #<20B data> */
	0xc0, 0x32, 0xad, 0xc1, 0xff, 0x62, 0x9c, 0x9b, /* data */
	0x66, 0xf2, 0x27, 0x49, 0xad, 0x66, 0x7e, 0x6b, /* data */
	0xea, 0xdf, 0x14, 0x4b, /* data */
	0x81, 0x42, 0x30, 0x00, /* opcode: LOAD PCR3, v0 */
	0x81, 0x42, 0x40, 0x00, /* opcode: LOAD PCR4, v0 */
};

static int second_stage_init(void)
{
	static const char mac_suffix[] = ".mac";
	bool did_first_stage_run = true;
	int result = 0;
	char *cptr, *mmcdev = NULL;
	struct key_program *hmac_blob = NULL;
	const char *image_path = "/ccdm.itb";
	char *mac_path = NULL;
	ulong image_addr;
	loff_t image_size;
	struct udevice *tpm;
	uint32_t err;
	int ret;

	printf("CCDM S2\n");
	ret = get_tpm(&tpm);
	if (ret || tpm_init(tpm))
		return 1;
	err = tpm_startup(tpm, TPM_ST_CLEAR);
	if (err != TPM_INVALID_POSTINIT)
		did_first_stage_run = false;

#ifdef CCDM_AUTO_FIRST_STAGE
	if (!did_first_stage_run && first_stage_actions(tpm))
		goto failure;
#else
	if (!did_first_stage_run)
		goto failure;
#endif

	if (hre_run_program(tpm, prg_stage2_prepare,
			    sizeof(prg_stage2_prepare)))
		goto failure;

	/* run "prepboot" from env to get "mmcdev" set */
	cptr = env_get("prepboot");
	if (cptr && !run_command(cptr, 0))
		mmcdev = env_get("mmcdev");
	if (!mmcdev)
		goto failure;

	cptr = env_get("ramdiskimage");
	if (cptr)
		image_path = cptr;

	mac_path = malloc(strlen(image_path) + strlen(mac_suffix) + 1);
	if (mac_path == NULL)
		goto failure;
	strcpy(mac_path, image_path);
	strcat(mac_path, mac_suffix);

	/* read image from mmcdev (ccdm.itb) */
	image_addr = (ulong)get_image_location();
	if (fs_set_blk_dev("mmc", mmcdev, FS_TYPE_EXT))
		goto failure;
	if (fs_read(image_path, image_addr, 0, 0, &image_size) < 0)
		goto failure;
	if (image_size <= 0)
		goto failure;
	printf("CCDM image found on %s, %lld bytes\n", mmcdev, image_size);

	hmac_blob = load_key_chunk("mmc", mmcdev, FS_TYPE_EXT, mac_path);
	if (!hmac_blob) {
		puts("failed to load mac file\n");
		goto failure;
	}
	if (verify_program(hmac_blob)) {
		puts("corrupted mac file\n");
		goto failure;
	}
	if (check_hmac(hmac_blob, (u8 *)image_addr, image_size)) {
		puts("image integrity could not be verified\n");
		goto failure;
	}
	puts("CCDM image OK\n");

	hre_run_program(tpm, prg_stage2_success, sizeof(prg_stage2_success));

	goto end;
failure:
	result = 1;
	hre_run_program(tpm, prg_stage_fail, sizeof(prg_stage_fail));
end:
	if (hmac_blob)
		free(hmac_blob);
	if (mac_path)
		free(mac_path);

	return result;
}
#endif

int show_self_hash(void)
{
	struct h_reg *hash_ptr;
#ifdef CCDM_SECOND_STAGE
	struct h_reg hash;

	hash_ptr = &hash;
	if (compute_self_hash(hash_ptr))
		return 1;
#else
	hash_ptr = &fix_hregs[FIX_HREG_SELF_HASH];
#endif
	puts("self hash: ");
	if (hash_ptr && hash_ptr->valid)
		print_buffer(0, hash_ptr->digest, 1, 20, 20);
	else
		puts("INVALID\n");

	return 0;
}

/**
 * @brief let the system hang.
 *
 * Called on error.
 * Will stop the boot process; display a message and signal the error condition
 * by blinking the "status" and the "finder" LED of the controller board.
 *
 * @note the develop version runs the blink cycle 2 times and then returns.
 * The release version never returns.
 */
static void ccdm_hang(void)
{
	static const u64 f0 = 0x0ba3bb8ba2e880; /* blink code "finder" LED */
	static const u64 s0 = 0x00f0f0f0f0f0f0; /* blink code "status" LED */
	u64 f, s;
	int i;
#ifdef CCDM_DEVELOP
	int j;
#endif

	I2C_SET_BUS(I2C_SOC_0);
	pca9698_direction_output(0x22, 0, 0); /* Finder */
	pca9698_direction_output(0x22, 4, 0); /* Status */

	puts("### ERROR ### Please RESET the board ###\n");
	bootstage_error(BOOTSTAGE_ID_NEED_RESET);
#ifdef CCDM_DEVELOP
	puts("*** ERROR ******** THIS WOULD HANG ******** ERROR ***\n");
	puts("** but we continue since this is a DEVELOP version **\n");
	puts("*** ERROR ******** THIS WOULD HANG ******** ERROR ***\n");
	for (j = 2; j-- > 0;) {
		putc('#');
#else
	for (;;) {
#endif
		f = f0;
		s = s0;
		for (i = 54; i-- > 0;) {
			pca9698_set_value(0x22, 0, !(f & 1));
			pca9698_set_value(0x22, 4, (s & 1));
			f >>= 1;
			s >>= 1;
			mdelay(120);
		}
	}
	puts("\ncontinue...\n");
}

int startup_ccdm_id_module(void)
{
	int result = 0;
	unsigned int orig_i2c_bus;

	orig_i2c_bus = i2c_get_bus_num();
	i2c_set_bus_num(I2C_SOC_1);

	/* goto end; */

#ifdef CCDM_DEVELOP
	show_self_hash();
#endif
#ifdef CCDM_FIRST_STAGE
	result = first_stage_init();
	if (result) {
		puts("1st stage init failed\n");
		goto failure;
	}
#endif
#ifdef CCDM_SECOND_STAGE
	result = second_stage_init();
	if (result) {
		puts("2nd stage init failed\n");
		goto failure;
	}
#endif

	goto end;
failure:
	result = 1;
end:
	i2c_set_bus_num(orig_i2c_bus);
	if (result)
		ccdm_hang();

	return result;
}
