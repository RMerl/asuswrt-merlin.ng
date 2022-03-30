// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX23/i.MX28 SB image generator
 *
 * Copyright (C) 2012-2013 Marek Vasut <marex@denx.de>
 */

#ifdef CONFIG_MXS

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <openssl/evp.h>

#include "imagetool.h"
#include "mxsimage.h"
#include "pbl_crc32.h"
#include <image.h>

/*
 * OpenSSL 1.1.0 and newer compatibility functions:
 * https://wiki.openssl.org/index.php/1.1_API_Changes
 */
#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    (defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x2070000fL)
static void *OPENSSL_zalloc(size_t num)
{
	void *ret = OPENSSL_malloc(num);

	if (ret != NULL)
		memset(ret, 0, num);
	return ret;
}

EVP_MD_CTX *EVP_MD_CTX_new(void)
{
	return OPENSSL_zalloc(sizeof(EVP_MD_CTX));
}

void EVP_MD_CTX_free(EVP_MD_CTX *ctx)
{
	EVP_MD_CTX_cleanup(ctx);
	OPENSSL_free(ctx);
}

int EVP_CIPHER_CTX_reset(EVP_CIPHER_CTX *ctx)
{
	return EVP_CIPHER_CTX_cleanup(ctx);
}
#endif

/*
 * DCD block
 * |-Write to address command block
 * |  0xf00 == 0xf33d
 * |  0xba2 == 0xb33f
 * |-ORR address with mask command block
 * |  0xf00 |= 0x1337
 * |-Write to address command block
 * |  0xba2 == 0xd00d
 * :
 */
#define SB_HAB_DCD_WRITE	0xccUL
#define SB_HAB_DCD_CHECK	0xcfUL
#define SB_HAB_DCD_NOOP		0xc0UL
#define SB_HAB_DCD_MASK_BIT	(1 << 3)
#define SB_HAB_DCD_SET_BIT	(1 << 4)

/* Addr.n = Value.n */
#define	SB_DCD_WRITE	\
	(SB_HAB_DCD_WRITE << 24)
/* Addr.n &= ~Value.n */
#define	SB_DCD_ANDC	\
	((SB_HAB_DCD_WRITE << 24) | SB_HAB_DCD_SET_BIT)
/* Addr.n |= Value.n */
#define	SB_DCD_ORR	\
	((SB_HAB_DCD_WRITE << 24) | SB_HAB_DCD_SET_BIT | SB_HAB_DCD_MASK_BIT)
/* (Addr.n & Value.n) == 0 */
#define	SB_DCD_CHK_EQZ	\
	(SB_HAB_DCD_CHECK << 24)
/* (Addr.n & Value.n) == Value.n */
#define	SB_DCD_CHK_EQ	\
	((SB_HAB_DCD_CHECK << 24) | SB_HAB_DCD_SET_BIT)
/* (Addr.n & Value.n) != Value.n */
#define	SB_DCD_CHK_NEQ	\
	((SB_HAB_DCD_CHECK << 24) | SB_HAB_DCD_MASK_BIT)
/* (Addr.n & Value.n) != 0 */
#define	SB_DCD_CHK_NEZ	\
	((SB_HAB_DCD_CHECK << 24) | SB_HAB_DCD_SET_BIT | SB_HAB_DCD_MASK_BIT)
/* NOP */
#define	SB_DCD_NOOP	\
	(SB_HAB_DCD_NOOP << 24)

struct sb_dcd_ctx {
	struct sb_dcd_ctx		*dcd;

	uint32_t			id;

	/* The DCD block. */
	uint32_t			*payload;
	/* Size of the whole DCD block. */
	uint32_t			size;

	/* Pointer to previous DCD command block. */
	uint32_t			*prev_dcd_head;
};

/*
 * IMAGE
 *   |-SECTION
 *   |    |-CMD
 *   |    |-CMD
 *   |    `-CMD
 *   |-SECTION
 *   |    |-CMD
 *   :    :
 */
struct sb_cmd_list {
	char				*cmd;
	size_t				len;
	unsigned int			lineno;
};

struct sb_cmd_ctx {
	uint32_t			size;

	struct sb_cmd_ctx		*cmd;

	uint8_t				*data;
	uint32_t			length;

	struct sb_command		payload;
	struct sb_command		c_payload;
};

struct sb_section_ctx {
	uint32_t			size;

	/* Section flags */
	unsigned int			boot:1;

	struct sb_section_ctx		*sect;

	struct sb_cmd_ctx		*cmd_head;
	struct sb_cmd_ctx		*cmd_tail;

	struct sb_sections_header	payload;
};

struct sb_image_ctx {
	unsigned int			in_section:1;
	unsigned int			in_dcd:1;
	/* Image configuration */
	unsigned int			display_progress:1;
	unsigned int			silent_dump:1;
	char				*input_filename;
	char				*output_filename;
	char				*cfg_filename;
	uint8_t				image_key[16];

	/* Number of section in the image */
	unsigned int			sect_count;
	/* Bootable section */
	unsigned int			sect_boot;
	unsigned int			sect_boot_found:1;

	struct sb_section_ctx		*sect_head;
	struct sb_section_ctx		*sect_tail;

	struct sb_dcd_ctx		*dcd_head;
	struct sb_dcd_ctx		*dcd_tail;

	EVP_CIPHER_CTX			*cipher_ctx;
	EVP_MD_CTX			*md_ctx;
	uint8_t				digest[32];
	struct sb_key_dictionary_key	sb_dict_key;

	struct sb_boot_image_header	payload;
};

/*
 * Instruction semantics:
 * NOOP
 * TAG [LAST]
 * LOAD       address file
 * LOAD  IVT  address IVT_entry_point
 * FILL address pattern length
 * JUMP [HAB] address [r0_arg]
 * CALL [HAB] address [r0_arg]
 * MODE mode
 *      For i.MX23, mode = USB/I2C/SPI1_FLASH/SPI2_FLASH/NAND_BCH
 *                         JTAG/SPI3_EEPROM/SD_SSP0/SD_SSP1
 *      For i.MX28, mode = USB/I2C/SPI2_FLASH/SPI3_FLASH/NAND_BCH
 *                         JTAG/SPI2_EEPROM/SD_SSP0/SD_SSP1
 */

/*
 * AES libcrypto
 */
static int sb_aes_init(struct sb_image_ctx *ictx, uint8_t *iv, int enc)
{
	EVP_CIPHER_CTX *ctx;
	int ret;

	/* If there is no init vector, init vector is all zeroes. */
	if (!iv)
		iv = ictx->image_key;

	ctx = EVP_CIPHER_CTX_new();
	ret = EVP_CipherInit(ctx, EVP_aes_128_cbc(), ictx->image_key, iv, enc);
	if (ret == 1) {
		EVP_CIPHER_CTX_set_padding(ctx, 0);
		ictx->cipher_ctx = ctx;
	}
	return ret;
}

static int sb_aes_crypt(struct sb_image_ctx *ictx, uint8_t *in_data,
			uint8_t *out_data, int in_len)
{
	EVP_CIPHER_CTX *ctx = ictx->cipher_ctx;
	int ret, outlen;
	uint8_t *outbuf;

	outbuf = malloc(in_len);
	if (!outbuf)
		return -ENOMEM;
	memset(outbuf, 0, sizeof(in_len));

	ret = EVP_CipherUpdate(ctx, outbuf, &outlen, in_data, in_len);
	if (!ret) {
		ret = -EINVAL;
		goto err;
	}

	if (out_data)
		memcpy(out_data, outbuf, outlen);

err:
	free(outbuf);
	return ret;
}

static int sb_aes_deinit(EVP_CIPHER_CTX *ctx)
{
	return EVP_CIPHER_CTX_reset(ctx);
}

static int sb_aes_reinit(struct sb_image_ctx *ictx, int enc)
{
	int ret;
	EVP_CIPHER_CTX *ctx = ictx->cipher_ctx;
	struct sb_boot_image_header *sb_header = &ictx->payload;
	uint8_t *iv = sb_header->iv;

	ret = sb_aes_deinit(ctx);
	if (!ret)
		return ret;
	return sb_aes_init(ictx, iv, enc);
}

/*
 * Debug
 */
static void soprintf(struct sb_image_ctx *ictx, const char *fmt, ...)
{
	va_list ap;

	if (ictx->silent_dump)
		return;

	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
}

/*
 * Code
 */
static time_t sb_get_timestamp(void)
{
	struct tm time_2000 = {
		.tm_yday	= 1,	/* Jan. 1st */
		.tm_year	= 100,	/* 2000 */
	};
	time_t seconds_to_2000 = mktime(&time_2000);
	time_t seconds_to_now = time(NULL);

	return seconds_to_now - seconds_to_2000;
}

static int sb_get_time(time_t time, struct tm *tm)
{
	struct tm time_2000 = {
		.tm_yday	= 1,	/* Jan. 1st */
		.tm_year	= 0,	/* 1900 */
	};
	const time_t seconds_to_2000 = mktime(&time_2000);
	const time_t seconds_to_now = seconds_to_2000 + time;
	struct tm *ret;
	ret = gmtime_r(&seconds_to_now, tm);
	return ret ? 0 : -EINVAL;
}

static void sb_encrypt_sb_header(struct sb_image_ctx *ictx)
{
	EVP_MD_CTX *md_ctx = ictx->md_ctx;
	struct sb_boot_image_header *sb_header = &ictx->payload;
	uint8_t *sb_header_ptr = (uint8_t *)sb_header;

	/* Encrypt the header, compute the digest. */
	sb_aes_crypt(ictx, sb_header_ptr, NULL, sizeof(*sb_header));
	EVP_DigestUpdate(md_ctx, sb_header_ptr, sizeof(*sb_header));
}

static void sb_encrypt_sb_sections_header(struct sb_image_ctx *ictx)
{
	EVP_MD_CTX *md_ctx = ictx->md_ctx;
	struct sb_section_ctx *sctx = ictx->sect_head;
	struct sb_sections_header *shdr;
	uint8_t *sb_sections_header_ptr;
	const int size = sizeof(*shdr);

	while (sctx) {
		shdr = &sctx->payload;
		sb_sections_header_ptr = (uint8_t *)shdr;

		sb_aes_crypt(ictx, sb_sections_header_ptr,
			     ictx->sb_dict_key.cbc_mac, size);
		EVP_DigestUpdate(md_ctx, sb_sections_header_ptr, size);

		sctx = sctx->sect;
	};
}

static void sb_encrypt_key_dictionary_key(struct sb_image_ctx *ictx)
{
	EVP_MD_CTX *md_ctx = ictx->md_ctx;

	sb_aes_crypt(ictx, ictx->image_key, ictx->sb_dict_key.key,
		     sizeof(ictx->sb_dict_key.key));
	EVP_DigestUpdate(md_ctx, &ictx->sb_dict_key, sizeof(ictx->sb_dict_key));
}

static void sb_decrypt_key_dictionary_key(struct sb_image_ctx *ictx)
{
	EVP_MD_CTX *md_ctx = ictx->md_ctx;

	EVP_DigestUpdate(md_ctx, &ictx->sb_dict_key, sizeof(ictx->sb_dict_key));
	sb_aes_crypt(ictx, ictx->sb_dict_key.key, ictx->image_key,
		     sizeof(ictx->sb_dict_key.key));
}

static void sb_encrypt_tag(struct sb_image_ctx *ictx,
		struct sb_cmd_ctx *cctx)
{
	EVP_MD_CTX *md_ctx = ictx->md_ctx;
	struct sb_command *cmd = &cctx->payload;

	sb_aes_crypt(ictx, (uint8_t *)cmd,
		     (uint8_t *)&cctx->c_payload, sizeof(*cmd));
	EVP_DigestUpdate(md_ctx, &cctx->c_payload, sizeof(*cmd));
}

static int sb_encrypt_image(struct sb_image_ctx *ictx)
{
	/* Start image-wide crypto. */
	ictx->md_ctx = EVP_MD_CTX_new();
	EVP_DigestInit(ictx->md_ctx, EVP_sha1());

	/*
	 * SB image header.
	 */
	sb_aes_init(ictx, NULL, 1);
	sb_encrypt_sb_header(ictx);

	/*
	 * SB sections header.
	 */
	sb_encrypt_sb_sections_header(ictx);

	/*
	 * Key dictionary.
	 */
	sb_aes_reinit(ictx, 1);
	sb_encrypt_key_dictionary_key(ictx);

	/*
	 * Section tags.
	 */
	struct sb_cmd_ctx *cctx;
	struct sb_command *ccmd;
	struct sb_section_ctx *sctx = ictx->sect_head;

	while (sctx) {
		cctx = sctx->cmd_head;

		sb_aes_reinit(ictx, 1);

		while (cctx) {
			ccmd = &cctx->payload;

			sb_encrypt_tag(ictx, cctx);

			if (ccmd->header.tag == ROM_TAG_CMD) {
				sb_aes_reinit(ictx, 1);
			} else if (ccmd->header.tag == ROM_LOAD_CMD) {
				sb_aes_crypt(ictx, cctx->data, cctx->data,
					     cctx->length);
				EVP_DigestUpdate(ictx->md_ctx, cctx->data,
						 cctx->length);
			}

			cctx = cctx->cmd;
		}

		sctx = sctx->sect;
	};

	/*
	 * Dump the SHA1 of the whole image.
	 */
	sb_aes_reinit(ictx, 1);

	EVP_DigestFinal(ictx->md_ctx, ictx->digest, NULL);
	EVP_MD_CTX_free(ictx->md_ctx);
	sb_aes_crypt(ictx, ictx->digest, ictx->digest, sizeof(ictx->digest));

	/* Stop the encryption session. */
	sb_aes_deinit(ictx->cipher_ctx);

	return 0;
}

static int sb_load_file(struct sb_cmd_ctx *cctx, char *filename)
{
	long real_size, roundup_size;
	uint8_t *data;
	long ret;
	unsigned long size;
	FILE *fp;

	if (!filename) {
		fprintf(stderr, "ERR: Missing filename!\n");
		return -EINVAL;
	}

	fp = fopen(filename, "r");
	if (!fp)
		goto err_open;

	ret = fseek(fp, 0, SEEK_END);
	if (ret < 0)
		goto err_file;

	real_size = ftell(fp);
	if (real_size < 0)
		goto err_file;

	ret = fseek(fp, 0, SEEK_SET);
	if (ret < 0)
		goto err_file;

	roundup_size = roundup(real_size, SB_BLOCK_SIZE);
	data = calloc(1, roundup_size);
	if (!data)
		goto err_file;

	size = fread(data, 1, real_size, fp);
	if (size != (unsigned long)real_size)
		goto err_alloc;

	cctx->data = data;
	cctx->length = roundup_size;

	fclose(fp);
	return 0;

err_alloc:
	free(data);
err_file:
	fclose(fp);
err_open:
	fprintf(stderr, "ERR: Failed to load file \"%s\"\n", filename);
	return -EINVAL;
}

static uint8_t sb_command_checksum(struct sb_command *inst)
{
	uint8_t *inst_ptr = (uint8_t *)inst;
	uint8_t csum = 0;
	unsigned int i;

	for (i = 0; i < sizeof(struct sb_command); i++)
		csum += inst_ptr[i];

	return csum;
}

static int sb_token_to_long(char *tok, uint32_t *rid)
{
	char *endptr;
	unsigned long id;

	if (tok[0] != '0' || tok[1] != 'x') {
		fprintf(stderr, "ERR: Invalid hexadecimal number!\n");
		return -EINVAL;
	}

	tok += 2;

	errno = 0;
	id = strtoul(tok, &endptr, 16);
	if ((errno == ERANGE && id == ULONG_MAX) || (errno != 0 && id == 0)) {
		fprintf(stderr, "ERR: Value can't be decoded!\n");
		return -EINVAL;
	}

	/* Check for 32-bit overflow. */
	if (id > 0xffffffff) {
		fprintf(stderr, "ERR: Value too big!\n");
		return -EINVAL;
	}

	if (endptr == tok) {
		fprintf(stderr, "ERR: Deformed value!\n");
		return -EINVAL;
	}

	*rid = (uint32_t)id;
	return 0;
}

static int sb_grow_dcd(struct sb_dcd_ctx *dctx, unsigned int inc_size)
{
	uint32_t *tmp;

	if (!inc_size)
		return 0;

	dctx->size += inc_size;
	tmp = realloc(dctx->payload, dctx->size);
	if (!tmp)
		return -ENOMEM;

	dctx->payload = tmp;

	/* Assemble and update the HAB DCD header. */
	dctx->payload[0] = htonl((SB_HAB_DCD_TAG << 24) |
				 (dctx->size << 8) |
				 SB_HAB_VERSION);

	return 0;
}

static int sb_build_dcd(struct sb_image_ctx *ictx, struct sb_cmd_list *cmd)
{
	struct sb_dcd_ctx *dctx;

	char *tok;
	uint32_t id;
	int ret;

	dctx = calloc(1, sizeof(*dctx));
	if (!dctx)
		return -ENOMEM;

	ret = sb_grow_dcd(dctx, 4);
	if (ret)
		goto err_dcd;

	/* Read DCD block number. */
	tok = strtok(cmd->cmd, " ");
	if (!tok) {
		fprintf(stderr, "#%i ERR: DCD block without number!\n",
			cmd->lineno);
		ret = -EINVAL;
		goto err_dcd;
	}

	/* Parse the DCD block number. */
	ret = sb_token_to_long(tok, &id);
	if (ret) {
		fprintf(stderr, "#%i ERR: Malformed DCD block number!\n",
			cmd->lineno);
		goto err_dcd;
	}

	dctx->id = id;

	/*
	 * The DCD block is now constructed. Append it to the list.
	 * WARNING: The DCD size is still not computed and will be
	 * updated while parsing it's commands.
	 */
	if (!ictx->dcd_head) {
		ictx->dcd_head = dctx;
		ictx->dcd_tail = dctx;
	} else {
		ictx->dcd_tail->dcd = dctx;
		ictx->dcd_tail = dctx;
	}

	return 0;

err_dcd:
	free(dctx->payload);
	free(dctx);
	return ret;
}

static int sb_build_dcd_block(struct sb_image_ctx *ictx,
			      struct sb_cmd_list *cmd,
			      uint32_t type)
{
	char *tok;
	uint32_t address, value, length;
	int ret;

	struct sb_dcd_ctx *dctx = ictx->dcd_tail;
	uint32_t *dcd;

	if (dctx->prev_dcd_head && (type != SB_DCD_NOOP) &&
	    ((dctx->prev_dcd_head[0] & 0xff0000ff) == type)) {
		/* Same instruction as before, just append it. */
		ret = sb_grow_dcd(dctx, 8);
		if (ret)
			return ret;
	} else if (type == SB_DCD_NOOP) {
		ret = sb_grow_dcd(dctx, 4);
		if (ret)
			return ret;

		/* Update DCD command block pointer. */
		dctx->prev_dcd_head = dctx->payload +
				dctx->size / sizeof(*dctx->payload) - 1;

		/* NOOP has only 4 bytes and no payload. */
		goto noop;
	} else {
		/*
		 * Either a different instruction block started now
		 * or this is the first instruction block.
		 */
		ret = sb_grow_dcd(dctx, 12);
		if (ret)
			return ret;

		/* Update DCD command block pointer. */
		dctx->prev_dcd_head = dctx->payload +
				dctx->size / sizeof(*dctx->payload) - 3;
	}

	dcd = dctx->payload + dctx->size / sizeof(*dctx->payload) - 2;

	/*
	 * Prepare the command.
	 */
	tok = strtok(cmd->cmd, " ");
	if (!tok) {
		fprintf(stderr, "#%i ERR: Missing DCD address!\n",
			cmd->lineno);
		ret = -EINVAL;
		goto err;
	}

	/* Read DCD destination address. */
	ret = sb_token_to_long(tok, &address);
	if (ret) {
		fprintf(stderr, "#%i ERR: Incorrect DCD address!\n",
			cmd->lineno);
		goto err;
	}

	tok = strtok(NULL, " ");
	if (!tok) {
		fprintf(stderr, "#%i ERR: Missing DCD value!\n",
			cmd->lineno);
		ret = -EINVAL;
		goto err;
	}

	/* Read DCD operation value. */
	ret = sb_token_to_long(tok, &value);
	if (ret) {
		fprintf(stderr, "#%i ERR: Incorrect DCD value!\n",
			cmd->lineno);
		goto err;
	}

	/* Fill in the new DCD entry. */
	dcd[0] = htonl(address);
	dcd[1] = htonl(value);

noop:
	/* Update the DCD command block. */
	length = dctx->size -
		 ((dctx->prev_dcd_head - dctx->payload) *
		 sizeof(*dctx->payload));
	dctx->prev_dcd_head[0] = htonl(type | (length << 8));

err:
	return ret;
}

static int sb_build_section(struct sb_image_ctx *ictx, struct sb_cmd_list *cmd)
{
	struct sb_section_ctx *sctx;
	struct sb_sections_header *shdr;
	char *tok;
	uint32_t bootable = 0;
	uint32_t id;
	int ret;

	sctx = calloc(1, sizeof(*sctx));
	if (!sctx)
		return -ENOMEM;

	/* Read section number. */
	tok = strtok(cmd->cmd, " ");
	if (!tok) {
		fprintf(stderr, "#%i ERR: Section without number!\n",
			cmd->lineno);
		ret = -EINVAL;
		goto err_sect;
	}

	/* Parse the section number. */
	ret = sb_token_to_long(tok, &id);
	if (ret) {
		fprintf(stderr, "#%i ERR: Malformed section number!\n",
			cmd->lineno);
		goto err_sect;
	}

	/* Read section's BOOTABLE flag. */
	tok = strtok(NULL, " ");
	if (tok && (strlen(tok) == 8) && !strncmp(tok, "BOOTABLE", 8))
		bootable = SB_SECTION_FLAG_BOOTABLE;

	sctx->boot = bootable;

	shdr = &sctx->payload;
	shdr->section_number = id;
	shdr->section_flags = bootable;

	/*
	 * The section is now constructed. Append it to the list.
	 * WARNING: The section size is still not computed and will
	 * be updated while parsing it's commands.
	 */
	ictx->sect_count++;

	/* Mark that this section is bootable one. */
	if (bootable) {
		if (ictx->sect_boot_found) {
			fprintf(stderr,
				"#%i WARN: Multiple bootable section!\n",
				cmd->lineno);
		} else {
			ictx->sect_boot = id;
			ictx->sect_boot_found = 1;
		}
	}

	if (!ictx->sect_head) {
		ictx->sect_head = sctx;
		ictx->sect_tail = sctx;
	} else {
		ictx->sect_tail->sect = sctx;
		ictx->sect_tail = sctx;
	}

	return 0;

err_sect:
	free(sctx);
	return ret;
}

static int sb_build_command_nop(struct sb_image_ctx *ictx)
{
	struct sb_section_ctx *sctx = ictx->sect_tail;
	struct sb_cmd_ctx *cctx;
	struct sb_command *ccmd;

	cctx = calloc(1, sizeof(*cctx));
	if (!cctx)
		return -ENOMEM;

	ccmd = &cctx->payload;

	/*
	 * Construct the command.
	 */
	ccmd->header.checksum	= 0x5a;
	ccmd->header.tag	= ROM_NOP_CMD;

	cctx->size = sizeof(*ccmd);

	/*
	 * Append the command to the last section.
	 */
	if (!sctx->cmd_head) {
		sctx->cmd_head = cctx;
		sctx->cmd_tail = cctx;
	} else {
		sctx->cmd_tail->cmd = cctx;
		sctx->cmd_tail = cctx;
	}

	return 0;
}

static int sb_build_command_tag(struct sb_image_ctx *ictx,
				struct sb_cmd_list *cmd)
{
	struct sb_section_ctx *sctx = ictx->sect_tail;
	struct sb_cmd_ctx *cctx;
	struct sb_command *ccmd;
	char *tok;

	cctx = calloc(1, sizeof(*cctx));
	if (!cctx)
		return -ENOMEM;

	ccmd = &cctx->payload;

	/*
	 * Prepare the command.
	 */
	/* Check for the LAST keyword. */
	tok = strtok(cmd->cmd, " ");
	if (tok && !strcmp(tok, "LAST"))
		ccmd->header.flags = ROM_TAG_CMD_FLAG_ROM_LAST_TAG;

	/*
	 * Construct the command.
	 */
	ccmd->header.checksum	= 0x5a;
	ccmd->header.tag	= ROM_TAG_CMD;

	cctx->size = sizeof(*ccmd);

	/*
	 * Append the command to the last section.
	 */
	if (!sctx->cmd_head) {
		sctx->cmd_head = cctx;
		sctx->cmd_tail = cctx;
	} else {
		sctx->cmd_tail->cmd = cctx;
		sctx->cmd_tail = cctx;
	}

	return 0;
}

static int sb_build_command_load(struct sb_image_ctx *ictx,
				 struct sb_cmd_list *cmd)
{
	struct sb_section_ctx *sctx = ictx->sect_tail;
	struct sb_cmd_ctx *cctx;
	struct sb_command *ccmd;
	char *tok;
	int ret, is_ivt = 0, is_dcd = 0;
	uint32_t dest, dcd = 0;

	cctx = calloc(1, sizeof(*cctx));
	if (!cctx)
		return -ENOMEM;

	ccmd = &cctx->payload;

	/*
	 * Prepare the command.
	 */
	tok = strtok(cmd->cmd, " ");
	if (!tok) {
		fprintf(stderr, "#%i ERR: Missing LOAD address or 'IVT'!\n",
			cmd->lineno);
		ret = -EINVAL;
		goto err;
	}

	/* Check for "IVT" flag. */
	if (!strcmp(tok, "IVT"))
		is_ivt = 1;
	if (!strcmp(tok, "DCD"))
		is_dcd = 1;
	if (is_ivt || is_dcd) {
		tok = strtok(NULL, " ");
		if (!tok) {
			fprintf(stderr, "#%i ERR: Missing LOAD address!\n",
				cmd->lineno);
			ret = -EINVAL;
			goto err;
		}
	}

	/* Read load destination address. */
	ret = sb_token_to_long(tok, &dest);
	if (ret) {
		fprintf(stderr, "#%i ERR: Incorrect LOAD address!\n",
			cmd->lineno);
		goto err;
	}

	/* Read filename or IVT entrypoint or DCD block ID. */
	tok = strtok(NULL, " ");
	if (!tok) {
		fprintf(stderr,
			"#%i ERR: Missing LOAD filename or IVT ep or DCD block ID!\n",
			cmd->lineno);
		ret = -EINVAL;
		goto err;
	}

	if (is_ivt) {
		/* Handle IVT. */
		struct sb_ivt_header *ivt;
		uint32_t ivtep;
		ret = sb_token_to_long(tok, &ivtep);

		if (ret) {
			fprintf(stderr,
				"#%i ERR: Incorrect IVT entry point!\n",
				cmd->lineno);
			goto err;
		}

		ivt = calloc(1, sizeof(*ivt));
		if (!ivt) {
			ret = -ENOMEM;
			goto err;
		}

		ivt->header = sb_hab_ivt_header();
		ivt->entry = ivtep;
		ivt->self = dest;

		cctx->data = (uint8_t *)ivt;
		cctx->length = sizeof(*ivt);
	} else if (is_dcd) {
		struct sb_dcd_ctx *dctx = ictx->dcd_head;
		uint32_t dcdid;
		uint8_t *payload;
		uint32_t asize;
		ret = sb_token_to_long(tok, &dcdid);

		if (ret) {
			fprintf(stderr,
				"#%i ERR: Incorrect DCD block ID!\n",
				cmd->lineno);
			goto err;
		}

		while (dctx) {
			if (dctx->id == dcdid)
				break;
			dctx = dctx->dcd;
		}

		if (!dctx) {
			fprintf(stderr, "#%i ERR: DCD block %08x not found!\n",
				cmd->lineno, dcdid);
			goto err;
		}

		asize = roundup(dctx->size, SB_BLOCK_SIZE);
		payload = calloc(1, asize);
		if (!payload) {
			ret = -ENOMEM;
			goto err;
		}

		memcpy(payload, dctx->payload, dctx->size);

		cctx->data = payload;
		cctx->length = asize;

		/* Set the Load DCD flag. */
		dcd = ROM_LOAD_CMD_FLAG_DCD_LOAD;
	} else {
		/* Regular LOAD of a file. */
		ret = sb_load_file(cctx, tok);
		if (ret) {
			fprintf(stderr, "#%i ERR: Cannot load '%s'!\n",
				cmd->lineno, tok);
			goto err;
		}
	}

	if (cctx->length & (SB_BLOCK_SIZE - 1)) {
		fprintf(stderr, "#%i ERR: Unaligned payload!\n",
			cmd->lineno);
	}

	/*
	 * Construct the command.
	 */
	ccmd->header.checksum	= 0x5a;
	ccmd->header.tag	= ROM_LOAD_CMD;
	ccmd->header.flags	= dcd;

	ccmd->load.address	= dest;
	ccmd->load.count	= cctx->length;
	ccmd->load.crc32	= pbl_crc32(0,
					    (const char *)cctx->data,
					    cctx->length);

	cctx->size = sizeof(*ccmd) + cctx->length;

	/*
	 * Append the command to the last section.
	 */
	if (!sctx->cmd_head) {
		sctx->cmd_head = cctx;
		sctx->cmd_tail = cctx;
	} else {
		sctx->cmd_tail->cmd = cctx;
		sctx->cmd_tail = cctx;
	}

	return 0;

err:
	free(cctx);
	return ret;
}

static int sb_build_command_fill(struct sb_image_ctx *ictx,
				 struct sb_cmd_list *cmd)
{
	struct sb_section_ctx *sctx = ictx->sect_tail;
	struct sb_cmd_ctx *cctx;
	struct sb_command *ccmd;
	char *tok;
	uint32_t address, pattern, length;
	int ret;

	cctx = calloc(1, sizeof(*cctx));
	if (!cctx)
		return -ENOMEM;

	ccmd = &cctx->payload;

	/*
	 * Prepare the command.
	 */
	tok = strtok(cmd->cmd, " ");
	if (!tok) {
		fprintf(stderr, "#%i ERR: Missing FILL address!\n",
			cmd->lineno);
		ret = -EINVAL;
		goto err;
	}

	/* Read fill destination address. */
	ret = sb_token_to_long(tok, &address);
	if (ret) {
		fprintf(stderr, "#%i ERR: Incorrect FILL address!\n",
			cmd->lineno);
		goto err;
	}

	tok = strtok(NULL, " ");
	if (!tok) {
		fprintf(stderr, "#%i ERR: Missing FILL pattern!\n",
			cmd->lineno);
		ret = -EINVAL;
		goto err;
	}

	/* Read fill pattern address. */
	ret = sb_token_to_long(tok, &pattern);
	if (ret) {
		fprintf(stderr, "#%i ERR: Incorrect FILL pattern!\n",
			cmd->lineno);
		goto err;
	}

	tok = strtok(NULL, " ");
	if (!tok) {
		fprintf(stderr, "#%i ERR: Missing FILL length!\n",
			cmd->lineno);
		ret = -EINVAL;
		goto err;
	}

	/* Read fill pattern address. */
	ret = sb_token_to_long(tok, &length);
	if (ret) {
		fprintf(stderr, "#%i ERR: Incorrect FILL length!\n",
			cmd->lineno);
		goto err;
	}

	/*
	 * Construct the command.
	 */
	ccmd->header.checksum	= 0x5a;
	ccmd->header.tag	= ROM_FILL_CMD;

	ccmd->fill.address	= address;
	ccmd->fill.count	= length;
	ccmd->fill.pattern	= pattern;

	cctx->size = sizeof(*ccmd);

	/*
	 * Append the command to the last section.
	 */
	if (!sctx->cmd_head) {
		sctx->cmd_head = cctx;
		sctx->cmd_tail = cctx;
	} else {
		sctx->cmd_tail->cmd = cctx;
		sctx->cmd_tail = cctx;
	}

	return 0;

err:
	free(cctx);
	return ret;
}

static int sb_build_command_jump_call(struct sb_image_ctx *ictx,
				      struct sb_cmd_list *cmd,
				      unsigned int is_call)
{
	struct sb_section_ctx *sctx = ictx->sect_tail;
	struct sb_cmd_ctx *cctx;
	struct sb_command *ccmd;
	char *tok;
	uint32_t dest, arg = 0x0;
	uint32_t hab = 0;
	int ret;
	const char *cmdname = is_call ? "CALL" : "JUMP";

	cctx = calloc(1, sizeof(*cctx));
	if (!cctx)
		return -ENOMEM;

	ccmd = &cctx->payload;

	/*
	 * Prepare the command.
	 */
	tok = strtok(cmd->cmd, " ");
	if (!tok) {
		fprintf(stderr,
			"#%i ERR: Missing %s address or 'HAB'!\n",
			cmd->lineno, cmdname);
		ret = -EINVAL;
		goto err;
	}

	/* Check for "HAB" flag. */
	if (!strcmp(tok, "HAB")) {
		hab = is_call ? ROM_CALL_CMD_FLAG_HAB : ROM_JUMP_CMD_FLAG_HAB;
		tok = strtok(NULL, " ");
		if (!tok) {
			fprintf(stderr, "#%i ERR: Missing %s address!\n",
				cmd->lineno, cmdname);
			ret = -EINVAL;
			goto err;
		}
	}
	/* Read load destination address. */
	ret = sb_token_to_long(tok, &dest);
	if (ret) {
		fprintf(stderr, "#%i ERR: Incorrect %s address!\n",
			cmd->lineno, cmdname);
		goto err;
	}

	tok = strtok(NULL, " ");
	if (tok) {
		ret = sb_token_to_long(tok, &arg);
		if (ret) {
			fprintf(stderr,
				"#%i ERR: Incorrect %s argument!\n",
				cmd->lineno, cmdname);
			goto err;
		}
	}

	/*
	 * Construct the command.
	 */
	ccmd->header.checksum	= 0x5a;
	ccmd->header.tag	= is_call ? ROM_CALL_CMD : ROM_JUMP_CMD;
	ccmd->header.flags	= hab;

	ccmd->call.address	= dest;
	ccmd->call.argument	= arg;

	cctx->size = sizeof(*ccmd);

	/*
	 * Append the command to the last section.
	 */
	if (!sctx->cmd_head) {
		sctx->cmd_head = cctx;
		sctx->cmd_tail = cctx;
	} else {
		sctx->cmd_tail->cmd = cctx;
		sctx->cmd_tail = cctx;
	}

	return 0;

err:
	free(cctx);
	return ret;
}

static int sb_build_command_jump(struct sb_image_ctx *ictx,
				 struct sb_cmd_list *cmd)
{
	return sb_build_command_jump_call(ictx, cmd, 0);
}

static int sb_build_command_call(struct sb_image_ctx *ictx,
				 struct sb_cmd_list *cmd)
{
	return sb_build_command_jump_call(ictx, cmd, 1);
}

static int sb_build_command_mode(struct sb_image_ctx *ictx,
				 struct sb_cmd_list *cmd)
{
	struct sb_section_ctx *sctx = ictx->sect_tail;
	struct sb_cmd_ctx *cctx;
	struct sb_command *ccmd;
	char *tok;
	int ret;
	unsigned int i;
	uint32_t mode = 0xffffffff;

	cctx = calloc(1, sizeof(*cctx));
	if (!cctx)
		return -ENOMEM;

	ccmd = &cctx->payload;

	/*
	 * Prepare the command.
	 */
	tok = strtok(cmd->cmd, " ");
	if (!tok) {
		fprintf(stderr, "#%i ERR: Missing MODE boot mode argument!\n",
			cmd->lineno);
		ret = -EINVAL;
		goto err;
	}

	for (i = 0; i < ARRAY_SIZE(modetable); i++) {
		if (!strcmp(tok, modetable[i].name)) {
			mode = modetable[i].mode;
			break;
		}

		if (!modetable[i].altname)
			continue;

		if (!strcmp(tok, modetable[i].altname)) {
			mode = modetable[i].mode;
			break;
		}
	}

	if (mode == 0xffffffff) {
		fprintf(stderr, "#%i ERR: Invalid MODE boot mode argument!\n",
			cmd->lineno);
		ret = -EINVAL;
		goto err;
	}

	/*
	 * Construct the command.
	 */
	ccmd->header.checksum	= 0x5a;
	ccmd->header.tag	= ROM_MODE_CMD;

	ccmd->mode.mode		= mode;

	cctx->size = sizeof(*ccmd);

	/*
	 * Append the command to the last section.
	 */
	if (!sctx->cmd_head) {
		sctx->cmd_head = cctx;
		sctx->cmd_tail = cctx;
	} else {
		sctx->cmd_tail->cmd = cctx;
		sctx->cmd_tail = cctx;
	}

	return 0;

err:
	free(cctx);
	return ret;
}

static int sb_prefill_image_header(struct sb_image_ctx *ictx)
{
	struct sb_boot_image_header *hdr = &ictx->payload;

	/* Fill signatures */
	memcpy(hdr->signature1, "STMP", 4);
	memcpy(hdr->signature2, "sgtl", 4);

	/* SB Image version 1.1 */
	hdr->major_version = SB_VERSION_MAJOR;
	hdr->minor_version = SB_VERSION_MINOR;

	/* Boot image major version */
	hdr->product_version.major = htons(0x999);
	hdr->product_version.minor = htons(0x999);
	hdr->product_version.revision = htons(0x999);
	/* Boot image major version */
	hdr->component_version.major = htons(0x999);
	hdr->component_version.minor = htons(0x999);
	hdr->component_version.revision = htons(0x999);

	/* Drive tag must be 0x0 for i.MX23 */
	hdr->drive_tag = 0;

	hdr->header_blocks =
		sizeof(struct sb_boot_image_header) / SB_BLOCK_SIZE;
	hdr->section_header_size =
		sizeof(struct sb_sections_header) / SB_BLOCK_SIZE;
	hdr->timestamp_us = sb_get_timestamp() * 1000000;

	hdr->flags = ictx->display_progress ?
		SB_IMAGE_FLAG_DISPLAY_PROGRESS : 0;

	/* FIXME -- We support only default key */
	hdr->key_count = 1;

	return 0;
}

static int sb_postfill_image_header(struct sb_image_ctx *ictx)
{
	struct sb_boot_image_header *hdr = &ictx->payload;
	struct sb_section_ctx *sctx = ictx->sect_head;
	uint32_t kd_size, sections_blocks;
	EVP_MD_CTX *md_ctx;

	/* The main SB header size in blocks. */
	hdr->image_blocks = hdr->header_blocks;

	/* Size of the key dictionary, which has single zero entry. */
	kd_size = hdr->key_count * sizeof(struct sb_key_dictionary_key);
	hdr->image_blocks += kd_size / SB_BLOCK_SIZE;

	/* Now count the payloads. */
	hdr->section_count = ictx->sect_count;
	while (sctx) {
		hdr->image_blocks += sctx->size / SB_BLOCK_SIZE;
		sctx = sctx->sect;
	}

	if (!ictx->sect_boot_found) {
		fprintf(stderr, "ERR: No bootable section selected!\n");
		return -EINVAL;
	}
	hdr->first_boot_section_id = ictx->sect_boot;

	/* The n * SB section size in blocks. */
	sections_blocks = hdr->section_count * hdr->section_header_size;
	hdr->image_blocks += sections_blocks;

	/* Key dictionary offset. */
	hdr->key_dictionary_block = hdr->header_blocks + sections_blocks;

	/* Digest of the whole image. */
	hdr->image_blocks += 2;

	/* Pointer past the dictionary. */
	hdr->first_boot_tag_block =
		hdr->key_dictionary_block + kd_size / SB_BLOCK_SIZE;

	/* Compute header digest. */
	md_ctx = EVP_MD_CTX_new();

	EVP_DigestInit(md_ctx, EVP_sha1());
	EVP_DigestUpdate(md_ctx, hdr->signature1,
			 sizeof(struct sb_boot_image_header) -
			 sizeof(hdr->digest));
	EVP_DigestFinal(md_ctx, hdr->digest, NULL);
	EVP_MD_CTX_free(md_ctx);

	return 0;
}

static int sb_fixup_sections_and_tags(struct sb_image_ctx *ictx)
{
	/* Fixup the placement of sections. */
	struct sb_boot_image_header *ihdr = &ictx->payload;
	struct sb_section_ctx *sctx = ictx->sect_head;
	struct sb_sections_header *shdr;
	struct sb_cmd_ctx *cctx;
	struct sb_command *ccmd;
	uint32_t offset = ihdr->first_boot_tag_block;

	while (sctx) {
		shdr = &sctx->payload;

		/* Fill in the section TAG offset. */
		shdr->section_offset = offset + 1;
		offset += shdr->section_size;

		/* Section length is measured from the TAG block. */
		shdr->section_size--;

		/* Fixup the TAG command. */
		cctx = sctx->cmd_head;
		while (cctx) {
			ccmd = &cctx->payload;
			if (ccmd->header.tag == ROM_TAG_CMD) {
				ccmd->tag.section_number = shdr->section_number;
				ccmd->tag.section_length = shdr->section_size;
				ccmd->tag.section_flags = shdr->section_flags;
			}

			/* Update the command checksum. */
			ccmd->header.checksum = sb_command_checksum(ccmd);

			cctx = cctx->cmd;
		}

		sctx = sctx->sect;
	}

	return 0;
}

static int sb_parse_line(struct sb_image_ctx *ictx, struct sb_cmd_list *cmd)
{
	char *tok;
	char *line = cmd->cmd;
	char *rptr = NULL;
	int ret;

	/* Analyze the identifier on this line first. */
	tok = strtok_r(line, " ", &rptr);
	if (!tok || (strlen(tok) == 0)) {
		fprintf(stderr, "#%i ERR: Invalid line!\n", cmd->lineno);
		return -EINVAL;
	}

	cmd->cmd = rptr;

	/* set DISPLAY_PROGRESS flag */
	if (!strcmp(tok, "DISPLAYPROGRESS")) {
		ictx->display_progress = 1;
		return 0;
	}

	/* DCD */
	if (!strcmp(tok, "DCD")) {
		ictx->in_section = 0;
		ictx->in_dcd = 1;
		sb_build_dcd(ictx, cmd);
		return 0;
	}

	/* Section */
	if (!strcmp(tok, "SECTION")) {
		ictx->in_section = 1;
		ictx->in_dcd = 0;
		sb_build_section(ictx, cmd);
		return 0;
	}

	if (!ictx->in_section && !ictx->in_dcd) {
		fprintf(stderr, "#%i ERR: Data outside of a section!\n",
			cmd->lineno);
		return -EINVAL;
	}

	if (ictx->in_section) {
		/* Section commands */
		if (!strcmp(tok, "NOP")) {
			ret = sb_build_command_nop(ictx);
		} else if (!strcmp(tok, "TAG")) {
			ret = sb_build_command_tag(ictx, cmd);
		} else if (!strcmp(tok, "LOAD")) {
			ret = sb_build_command_load(ictx, cmd);
		} else if (!strcmp(tok, "FILL")) {
			ret = sb_build_command_fill(ictx, cmd);
		} else if (!strcmp(tok, "JUMP")) {
			ret = sb_build_command_jump(ictx, cmd);
		} else if (!strcmp(tok, "CALL")) {
			ret = sb_build_command_call(ictx, cmd);
		} else if (!strcmp(tok, "MODE")) {
			ret = sb_build_command_mode(ictx, cmd);
		} else {
			fprintf(stderr,
				"#%i ERR: Unsupported instruction '%s'!\n",
				cmd->lineno, tok);
			return -ENOTSUP;
		}
	} else if (ictx->in_dcd) {
		char *lptr;
		uint32_t ilen = '1';

		tok = strtok_r(tok, ".", &lptr);
		if (!tok || (strlen(tok) == 0) || (lptr && strlen(lptr) != 1)) {
			fprintf(stderr, "#%i ERR: Invalid line!\n",
				cmd->lineno);
			return -EINVAL;
		}

		if (lptr &&
		    (lptr[0] != '1' && lptr[0] != '2' && lptr[0] != '4')) {
			fprintf(stderr, "#%i ERR: Invalid instruction width!\n",
				cmd->lineno);
			return -EINVAL;
		}

		if (lptr)
			ilen = lptr[0] - '1';

		/* DCD commands */
		if (!strcmp(tok, "WRITE")) {
			ret = sb_build_dcd_block(ictx, cmd,
						 SB_DCD_WRITE | ilen);
		} else if (!strcmp(tok, "ANDC")) {
			ret = sb_build_dcd_block(ictx, cmd,
						 SB_DCD_ANDC | ilen);
		} else if (!strcmp(tok, "ORR")) {
			ret = sb_build_dcd_block(ictx, cmd,
						 SB_DCD_ORR | ilen);
		} else if (!strcmp(tok, "EQZ")) {
			ret = sb_build_dcd_block(ictx, cmd,
						 SB_DCD_CHK_EQZ | ilen);
		} else if (!strcmp(tok, "EQ")) {
			ret = sb_build_dcd_block(ictx, cmd,
						 SB_DCD_CHK_EQ | ilen);
		} else if (!strcmp(tok, "NEQ")) {
			ret = sb_build_dcd_block(ictx, cmd,
						 SB_DCD_CHK_NEQ | ilen);
		} else if (!strcmp(tok, "NEZ")) {
			ret = sb_build_dcd_block(ictx, cmd,
						 SB_DCD_CHK_NEZ | ilen);
		} else if (!strcmp(tok, "NOOP")) {
			ret = sb_build_dcd_block(ictx, cmd, SB_DCD_NOOP);
		} else {
			fprintf(stderr,
				"#%i ERR: Unsupported instruction '%s'!\n",
				cmd->lineno, tok);
			return -ENOTSUP;
		}
	} else {
		fprintf(stderr, "#%i ERR: Unsupported instruction '%s'!\n",
			cmd->lineno, tok);
		return -ENOTSUP;
	}

	/*
	 * Here we have at least one section with one command, otherwise we
	 * would have failed already higher above.
	 *
	 * FIXME -- should the updating happen here ?
	 */
	if (ictx->in_section && !ret) {
		ictx->sect_tail->size += ictx->sect_tail->cmd_tail->size;
		ictx->sect_tail->payload.section_size =
			ictx->sect_tail->size / SB_BLOCK_SIZE;
	}

	return ret;
}

static int sb_load_cmdfile(struct sb_image_ctx *ictx)
{
	struct sb_cmd_list cmd;
	int lineno = 1;
	FILE *fp;
	char *line = NULL;
	ssize_t rlen;
	size_t len;

	fp = fopen(ictx->cfg_filename, "r");
	if (!fp)
		goto err_file;

	while ((rlen = getline(&line, &len, fp)) > 0) {
		memset(&cmd, 0, sizeof(cmd));

		/* Strip the trailing newline. */
		line[rlen - 1] = '\0';

		cmd.cmd = line;
		cmd.len = rlen;
		cmd.lineno = lineno++;

		sb_parse_line(ictx, &cmd);
	}

	free(line);

	fclose(fp);

	return 0;

err_file:
	fclose(fp);
	fprintf(stderr, "ERR: Failed to load file \"%s\"\n",
		ictx->cfg_filename);
	return -EINVAL;
}

static int sb_build_tree_from_cfg(struct sb_image_ctx *ictx)
{
	int ret;

	ret = sb_load_cmdfile(ictx);
	if (ret)
		return ret;

	ret = sb_prefill_image_header(ictx);
	if (ret)
		return ret;

	ret = sb_postfill_image_header(ictx);
	if (ret)
		return ret;

	ret = sb_fixup_sections_and_tags(ictx);
	if (ret)
		return ret;

	return 0;
}

static int sb_verify_image_header(struct sb_image_ctx *ictx,
				  FILE *fp, long fsize)
{
	/* Verify static fields in the image header. */
	struct sb_boot_image_header *hdr = &ictx->payload;
	const char *stat[2] = { "[PASS]", "[FAIL]" };
	struct tm tm;
	int sz, ret = 0;
	unsigned char digest[20];
	EVP_MD_CTX *md_ctx;
	unsigned long size;

	/* Start image-wide crypto. */
	ictx->md_ctx = EVP_MD_CTX_new();
	EVP_DigestInit(ictx->md_ctx, EVP_sha1());

	soprintf(ictx, "---------- Verifying SB Image Header ----------\n");

	size = fread(&ictx->payload, 1, sizeof(ictx->payload), fp);
	if (size != sizeof(ictx->payload)) {
		fprintf(stderr, "ERR: SB image header too short!\n");
		return -EINVAL;
	}

	/* Compute header digest. */
	md_ctx = EVP_MD_CTX_new();
	EVP_DigestInit(md_ctx, EVP_sha1());
	EVP_DigestUpdate(md_ctx, hdr->signature1,
			 sizeof(struct sb_boot_image_header) -
			 sizeof(hdr->digest));
	EVP_DigestFinal(md_ctx, digest, NULL);
	EVP_MD_CTX_free(md_ctx);

	sb_aes_init(ictx, NULL, 1);
	sb_encrypt_sb_header(ictx);

	if (memcmp(digest, hdr->digest, 20))
		ret = -EINVAL;
	soprintf(ictx, "%s Image header checksum:        %s\n", stat[!!ret],
		 ret ? "BAD" : "OK");
	if (ret)
		return ret;

	if (memcmp(hdr->signature1, "STMP", 4) ||
	    memcmp(hdr->signature2, "sgtl", 4))
		ret = -EINVAL;
	soprintf(ictx, "%s Signatures:                   '%.4s' '%.4s'\n",
		 stat[!!ret], hdr->signature1, hdr->signature2);
	if (ret)
		return ret;

	if ((hdr->major_version != SB_VERSION_MAJOR) ||
	    ((hdr->minor_version != 1) && (hdr->minor_version != 2)))
		ret = -EINVAL;
	soprintf(ictx, "%s Image version:                v%i.%i\n", stat[!!ret],
		 hdr->major_version, hdr->minor_version);
	if (ret)
		return ret;

	ret = sb_get_time(hdr->timestamp_us / 1000000, &tm);
	soprintf(ictx,
		 "%s Creation time:                %02i:%02i:%02i %02i/%02i/%04i\n",
		 stat[!!ret], tm.tm_hour, tm.tm_min, tm.tm_sec,
		 tm.tm_mday, tm.tm_mon, tm.tm_year + 2000);
	if (ret)
		return ret;

	soprintf(ictx, "%s Product version:              %x.%x.%x\n", stat[0],
		 ntohs(hdr->product_version.major),
		 ntohs(hdr->product_version.minor),
		 ntohs(hdr->product_version.revision));
	soprintf(ictx, "%s Component version:            %x.%x.%x\n", stat[0],
		 ntohs(hdr->component_version.major),
		 ntohs(hdr->component_version.minor),
		 ntohs(hdr->component_version.revision));

	if (hdr->flags & ~SB_IMAGE_FLAGS_MASK)
		ret = -EINVAL;
	soprintf(ictx, "%s Image flags:                  %s\n", stat[!!ret],
		 hdr->flags & SB_IMAGE_FLAG_DISPLAY_PROGRESS ?
		 "Display_progress" : "");
	if (ret)
		return ret;

	if (hdr->drive_tag != 0)
		ret = -EINVAL;
	soprintf(ictx, "%s Drive tag:                    %i\n", stat[!!ret],
		 hdr->drive_tag);
	if (ret)
		return ret;

	sz = sizeof(struct sb_boot_image_header) / SB_BLOCK_SIZE;
	if (hdr->header_blocks != sz)
		ret = -EINVAL;
	soprintf(ictx, "%s Image header size (blocks):   %i\n", stat[!!ret],
		 hdr->header_blocks);
	if (ret)
		return ret;

	sz = sizeof(struct sb_sections_header) / SB_BLOCK_SIZE;
	if (hdr->section_header_size != sz)
		ret = -EINVAL;
	soprintf(ictx, "%s Section header size (blocks): %i\n", stat[!!ret],
		 hdr->section_header_size);
	if (ret)
		return ret;

	soprintf(ictx, "%s Sections count:               %i\n", stat[!!ret],
		 hdr->section_count);
	soprintf(ictx, "%s First bootable section        %i\n", stat[!!ret],
		 hdr->first_boot_section_id);

	if (hdr->image_blocks != fsize / SB_BLOCK_SIZE)
		ret = -EINVAL;
	soprintf(ictx, "%s Image size (blocks):          %i\n", stat[!!ret],
		 hdr->image_blocks);
	if (ret)
		return ret;

	sz = hdr->header_blocks + hdr->section_header_size * hdr->section_count;
	if (hdr->key_dictionary_block != sz)
		ret = -EINVAL;
	soprintf(ictx, "%s Key dict offset (blocks):     %i\n", stat[!!ret],
		 hdr->key_dictionary_block);
	if (ret)
		return ret;

	if (hdr->key_count != 1)
		ret = -EINVAL;
	soprintf(ictx, "%s Number of encryption keys:    %i\n", stat[!!ret],
		 hdr->key_count);
	if (ret)
		return ret;

	sz = hdr->header_blocks + hdr->section_header_size * hdr->section_count;
	sz += hdr->key_count *
		sizeof(struct sb_key_dictionary_key) / SB_BLOCK_SIZE;
	if (hdr->first_boot_tag_block != (unsigned)sz)
		ret = -EINVAL;
	soprintf(ictx, "%s First TAG block (blocks):     %i\n", stat[!!ret],
		 hdr->first_boot_tag_block);
	if (ret)
		return ret;

	return 0;
}

static void sb_decrypt_tag(struct sb_image_ctx *ictx,
		struct sb_cmd_ctx *cctx)
{
	EVP_MD_CTX *md_ctx = ictx->md_ctx;
	struct sb_command *cmd = &cctx->payload;

	sb_aes_crypt(ictx, (uint8_t *)&cctx->c_payload,
		     (uint8_t *)&cctx->payload, sizeof(*cmd));
	EVP_DigestUpdate(md_ctx, &cctx->c_payload, sizeof(*cmd));
}

static int sb_verify_command(struct sb_image_ctx *ictx,
			     struct sb_cmd_ctx *cctx, FILE *fp,
			     unsigned long *tsize)
{
	struct sb_command *ccmd = &cctx->payload;
	unsigned long size, asize;
	char *csum, *flag = "";
	int ret;
	unsigned int i;
	uint8_t csn, csc = ccmd->header.checksum;
	ccmd->header.checksum = 0x5a;
	csn = sb_command_checksum(ccmd);
	ccmd->header.checksum = csc;

	if (csc == csn)
		ret = 0;
	else
		ret = -EINVAL;
	csum = ret ? "checksum BAD" : "checksum OK";

	switch (ccmd->header.tag) {
	case ROM_NOP_CMD:
		soprintf(ictx, " NOOP # %s\n", csum);
		return ret;
	case ROM_TAG_CMD:
		if (ccmd->header.flags & ROM_TAG_CMD_FLAG_ROM_LAST_TAG)
			flag = "LAST";
		soprintf(ictx, " TAG %s # %s\n", flag, csum);
		sb_aes_reinit(ictx, 0);
		return ret;
	case ROM_LOAD_CMD:
		soprintf(ictx, " LOAD addr=0x%08x length=0x%08x # %s\n",
			 ccmd->load.address, ccmd->load.count, csum);

		cctx->length = ccmd->load.count;
		asize = roundup(cctx->length, SB_BLOCK_SIZE);
		cctx->data = malloc(asize);
		if (!cctx->data)
			return -ENOMEM;

		size = fread(cctx->data, 1, asize, fp);
		if (size != asize) {
			fprintf(stderr,
				"ERR: SB LOAD command payload too short!\n");
			return -EINVAL;
		}

		*tsize += size;

		EVP_DigestUpdate(ictx->md_ctx, cctx->data, asize);
		sb_aes_crypt(ictx, cctx->data, cctx->data, asize);

		if (ccmd->load.crc32 != pbl_crc32(0,
						  (const char *)cctx->data,
						  asize)) {
			fprintf(stderr,
				"ERR: SB LOAD command payload CRC32 invalid!\n");
			return -EINVAL;
		}
		return 0;
	case ROM_FILL_CMD:
		soprintf(ictx,
			 " FILL addr=0x%08x length=0x%08x pattern=0x%08x # %s\n",
			 ccmd->fill.address, ccmd->fill.count,
			 ccmd->fill.pattern, csum);
		return 0;
	case ROM_JUMP_CMD:
		if (ccmd->header.flags & ROM_JUMP_CMD_FLAG_HAB)
			flag = " HAB";
		soprintf(ictx,
			 " JUMP%s addr=0x%08x r0_arg=0x%08x # %s\n",
			 flag, ccmd->fill.address, ccmd->jump.argument, csum);
		return 0;
	case ROM_CALL_CMD:
		if (ccmd->header.flags & ROM_CALL_CMD_FLAG_HAB)
			flag = " HAB";
		soprintf(ictx,
			 " CALL%s addr=0x%08x r0_arg=0x%08x # %s\n",
			 flag, ccmd->fill.address, ccmd->jump.argument, csum);
		return 0;
	case ROM_MODE_CMD:
		for (i = 0; i < ARRAY_SIZE(modetable); i++) {
			if (ccmd->mode.mode == modetable[i].mode) {
				soprintf(ictx, " MODE %s # %s\n",
					 modetable[i].name, csum);
				break;
			}
		}
		fprintf(stderr, " MODE !INVALID! # %s\n", csum);
		return 0;
	}

	return ret;
}

static int sb_verify_commands(struct sb_image_ctx *ictx,
			      struct sb_section_ctx *sctx, FILE *fp)
{
	unsigned long size, tsize = 0;
	struct sb_cmd_ctx *cctx;
	int ret;

	sb_aes_reinit(ictx, 0);

	while (tsize < sctx->size) {
		cctx = calloc(1, sizeof(*cctx));
		if (!cctx)
			return -ENOMEM;
		if (!sctx->cmd_head) {
			sctx->cmd_head = cctx;
			sctx->cmd_tail = cctx;
		} else {
			sctx->cmd_tail->cmd = cctx;
			sctx->cmd_tail = cctx;
		}

		size = fread(&cctx->c_payload, 1, sizeof(cctx->c_payload), fp);
		if (size != sizeof(cctx->c_payload)) {
			fprintf(stderr, "ERR: SB command header too short!\n");
			return -EINVAL;
		}

		tsize += size;

		sb_decrypt_tag(ictx, cctx);

		ret = sb_verify_command(ictx, cctx, fp, &tsize);
		if (ret)
			return -EINVAL;
	}

	return 0;
}

static int sb_verify_sections_cmds(struct sb_image_ctx *ictx, FILE *fp)
{
	struct sb_boot_image_header *hdr = &ictx->payload;
	struct sb_sections_header *shdr;
	unsigned int i;
	int ret;
	struct sb_section_ctx *sctx;
	unsigned long size;
	char *bootable = "";

	soprintf(ictx, "----- Verifying  SB Sections and Commands -----\n");

	for (i = 0; i < hdr->section_count; i++) {
		sctx = calloc(1, sizeof(*sctx));
		if (!sctx)
			return -ENOMEM;
		if (!ictx->sect_head) {
			ictx->sect_head = sctx;
			ictx->sect_tail = sctx;
		} else {
			ictx->sect_tail->sect = sctx;
			ictx->sect_tail = sctx;
		}

		size = fread(&sctx->payload, 1, sizeof(sctx->payload), fp);
		if (size != sizeof(sctx->payload)) {
			fprintf(stderr, "ERR: SB section header too short!\n");
			return -EINVAL;
		}
	}

	size = fread(&ictx->sb_dict_key, 1, sizeof(ictx->sb_dict_key), fp);
	if (size != sizeof(ictx->sb_dict_key)) {
		fprintf(stderr, "ERR: SB key dictionary too short!\n");
		return -EINVAL;
	}

	sb_encrypt_sb_sections_header(ictx);
	sb_aes_reinit(ictx, 0);
	sb_decrypt_key_dictionary_key(ictx);

	sb_aes_reinit(ictx, 0);

	sctx = ictx->sect_head;
	while (sctx) {
		shdr = &sctx->payload;

		if (shdr->section_flags & SB_SECTION_FLAG_BOOTABLE) {
			sctx->boot = 1;
			bootable = " BOOTABLE";
		}

		sctx->size = (shdr->section_size * SB_BLOCK_SIZE) +
			     sizeof(struct sb_command);
		soprintf(ictx, "SECTION 0x%x%s # size = %i bytes\n",
			 shdr->section_number, bootable, sctx->size);

		if (shdr->section_flags & ~SB_SECTION_FLAG_BOOTABLE)
			fprintf(stderr, " WARN: Unknown section flag(s) %08x\n",
				shdr->section_flags);

		if ((shdr->section_flags & SB_SECTION_FLAG_BOOTABLE) &&
		    (hdr->first_boot_section_id != shdr->section_number)) {
			fprintf(stderr,
				" WARN: Bootable section does ID not match image header ID!\n");
		}

		ret = sb_verify_commands(ictx, sctx, fp);
		if (ret)
			return ret;

		sctx = sctx->sect;
	}

	/*
	 * FIXME IDEA:
	 * check if the first TAG command is at sctx->section_offset
	 */
	return 0;
}

static int sb_verify_image_end(struct sb_image_ctx *ictx,
			       FILE *fp, off_t filesz)
{
	uint8_t digest[32];
	unsigned long size;
	off_t pos;
	int ret;

	soprintf(ictx, "------------- Verifying image end -------------\n");

	size = fread(digest, 1, sizeof(digest), fp);
	if (size != sizeof(digest)) {
		fprintf(stderr, "ERR: SB key dictionary too short!\n");
		return -EINVAL;
	}

	pos = ftell(fp);
	if (pos != filesz) {
		fprintf(stderr, "ERR: Trailing data past the image!\n");
		return -EINVAL;
	}

	/* Check the image digest. */
	EVP_DigestFinal(ictx->md_ctx, ictx->digest, NULL);
	EVP_MD_CTX_free(ictx->md_ctx);

	/* Decrypt the image digest from the input image. */
	sb_aes_reinit(ictx, 0);
	sb_aes_crypt(ictx, digest, digest, sizeof(digest));

	/* Check all of 20 bytes of the SHA1 hash. */
	ret = memcmp(digest, ictx->digest, 20) ? -EINVAL : 0;

	if (ret)
		soprintf(ictx, "[FAIL] Full-image checksum:          BAD\n");
	else
		soprintf(ictx, "[PASS] Full-image checksum:          OK\n");

	return ret;
}


static int sb_build_tree_from_img(struct sb_image_ctx *ictx)
{
	long filesize;
	int ret;
	FILE *fp;

	if (!ictx->input_filename) {
		fprintf(stderr, "ERR: Missing filename!\n");
		return -EINVAL;
	}

	fp = fopen(ictx->input_filename, "r");
	if (!fp)
		goto err_open;

	ret = fseek(fp, 0, SEEK_END);
	if (ret < 0)
		goto err_file;

	filesize = ftell(fp);
	if (filesize < 0)
		goto err_file;

	ret = fseek(fp, 0, SEEK_SET);
	if (ret < 0)
		goto err_file;

	if (filesize < (signed)sizeof(ictx->payload)) {
		fprintf(stderr, "ERR: File too short!\n");
		goto err_file;
	}

	if (filesize & (SB_BLOCK_SIZE - 1)) {
		fprintf(stderr, "ERR: The file is not aligned!\n");
		goto err_file;
	}

	/* Load and verify image header */
	ret = sb_verify_image_header(ictx, fp, filesize);
	if (ret)
		goto err_verify;

	/* Load and verify sections and commands */
	ret = sb_verify_sections_cmds(ictx, fp);
	if (ret)
		goto err_verify;

	ret = sb_verify_image_end(ictx, fp, filesize);
	if (ret)
		goto err_verify;

	ret = 0;

err_verify:
	soprintf(ictx, "-------------------- Result -------------------\n");
	soprintf(ictx, "Verification %s\n", ret ? "FAILED" : "PASSED");

	/* Stop the encryption session. */
	sb_aes_deinit(ictx->cipher_ctx);

	fclose(fp);
	return ret;

err_file:
	fclose(fp);
err_open:
	fprintf(stderr, "ERR: Failed to load file \"%s\"\n",
		ictx->input_filename);
	return -EINVAL;
}

static void sb_free_image(struct sb_image_ctx *ictx)
{
	struct sb_section_ctx *sctx = ictx->sect_head, *s_head;
	struct sb_dcd_ctx *dctx = ictx->dcd_head, *d_head;
	struct sb_cmd_ctx *cctx, *c_head;

	while (sctx) {
		s_head = sctx;
		c_head = sctx->cmd_head;

		while (c_head) {
			cctx = c_head;
			c_head = c_head->cmd;
			if (cctx->data)
				free(cctx->data);
			free(cctx);
		}

		sctx = sctx->sect;
		free(s_head);
	}

	while (dctx) {
		d_head = dctx;
		dctx = dctx->dcd;
		free(d_head->payload);
		free(d_head);
	}
}

/*
 * MXSSB-MKIMAGE glue code.
 */
static int mxsimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_MXSIMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static void mxsimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
}

int mxsimage_check_params(struct image_tool_params *params)
{
	if (!params)
		return -1;
	if (!strlen(params->imagename)) {
		fprintf(stderr,
			"Error: %s - Configuration file not specified, it is needed for mxsimage generation\n",
			params->cmdname);
		return -1;
	}

	/*
	 * Check parameters:
	 * XIP is not allowed and verify that incompatible
	 * parameters are not sent at the same time
	 * For example, if list is required a data image must not be provided
	 */
	return	(params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag)) ||
		(params->xflag) || !(strlen(params->imagename));
}

static int mxsimage_verify_print_header(char *file, int silent)
{
	int ret;
	struct sb_image_ctx ctx;

	memset(&ctx, 0, sizeof(ctx));

	ctx.input_filename = file;
	ctx.silent_dump = silent;

	ret = sb_build_tree_from_img(&ctx);
	sb_free_image(&ctx);

	return ret;
}

char *imagefile;
static int mxsimage_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	struct sb_boot_image_header *hdr;

	if (!ptr)
		return -EINVAL;

	hdr = (struct sb_boot_image_header *)ptr;

	/*
	 * Check if the header contains the MXS image signatures,
	 * if so, do a full-image verification.
	 */
	if (memcmp(hdr->signature1, "STMP", 4) ||
	    memcmp(hdr->signature2, "sgtl", 4))
		return -EINVAL;

	imagefile = params->imagefile;

	return mxsimage_verify_print_header(params->imagefile, 1);
}

static void mxsimage_print_header(const void *hdr)
{
	if (imagefile)
		mxsimage_verify_print_header(imagefile, 0);
}

static int sb_build_image(struct sb_image_ctx *ictx,
			  struct image_type_params *tparams)
{
	struct sb_boot_image_header *sb_header = &ictx->payload;
	struct sb_section_ctx *sctx;
	struct sb_cmd_ctx *cctx;
	struct sb_command *ccmd;
	struct sb_key_dictionary_key *sb_dict_key = &ictx->sb_dict_key;

	uint8_t *image, *iptr;

	/* Calculate image size. */
	uint32_t size = sizeof(*sb_header) +
		ictx->sect_count * sizeof(struct sb_sections_header) +
		sizeof(*sb_dict_key) + sizeof(ictx->digest);

	sctx = ictx->sect_head;
	while (sctx) {
		size += sctx->size;
		sctx = sctx->sect;
	};

	image = malloc(size);
	if (!image)
		return -ENOMEM;
	iptr = image;

	memcpy(iptr, sb_header, sizeof(*sb_header));
	iptr += sizeof(*sb_header);

	sctx = ictx->sect_head;
	while (sctx) {
		memcpy(iptr, &sctx->payload, sizeof(struct sb_sections_header));
		iptr += sizeof(struct sb_sections_header);
		sctx = sctx->sect;
	};

	memcpy(iptr, sb_dict_key, sizeof(*sb_dict_key));
	iptr += sizeof(*sb_dict_key);

	sctx = ictx->sect_head;
	while (sctx) {
		cctx = sctx->cmd_head;
		while (cctx) {
			ccmd = &cctx->payload;

			memcpy(iptr, &cctx->c_payload, sizeof(cctx->payload));
			iptr += sizeof(cctx->payload);

			if (ccmd->header.tag == ROM_LOAD_CMD) {
				memcpy(iptr, cctx->data, cctx->length);
				iptr += cctx->length;
			}

			cctx = cctx->cmd;
		}

		sctx = sctx->sect;
	};

	memcpy(iptr, ictx->digest, sizeof(ictx->digest));
	iptr += sizeof(ictx->digest);

	/* Configure the mkimage */
	tparams->hdr = image;
	tparams->header_size = size;

	return 0;
}

static int mxsimage_generate(struct image_tool_params *params,
	struct image_type_params *tparams)
{
	int ret;
	struct sb_image_ctx ctx;

	/* Do not copy the U-Boot image! */
	params->skipcpy = 1;

	memset(&ctx, 0, sizeof(ctx));

	ctx.cfg_filename = params->imagename;
	ctx.output_filename = params->imagefile;

	ret = sb_build_tree_from_cfg(&ctx);
	if (ret)
		goto fail;

	ret = sb_encrypt_image(&ctx);
	if (!ret)
		ret = sb_build_image(&ctx, tparams);

fail:
	sb_free_image(&ctx);

	return ret;
}

/*
 * mxsimage parameters
 */
U_BOOT_IMAGE_TYPE(
	mxsimage,
	"Freescale MXS Boot Image support",
	0,
	NULL,
	mxsimage_check_params,
	mxsimage_verify_header,
	mxsimage_print_header,
	mxsimage_set_header,
	NULL,
	mxsimage_check_image_types,
	NULL,
	mxsimage_generate
);
#endif
