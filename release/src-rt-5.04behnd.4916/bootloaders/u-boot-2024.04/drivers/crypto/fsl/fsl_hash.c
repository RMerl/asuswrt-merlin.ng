// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 */

#include <common.h>
#include <cpu_func.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include "jobdesc.h"
#include "desc.h"
#include "jr.h"
#include "fsl_hash.h"
#include <hw_sha.h>
#include <asm/cache.h>
#include <linux/errno.h>

#define CRYPTO_MAX_ALG_NAME	80
#define SHA1_DIGEST_SIZE        20
#define SHA256_DIGEST_SIZE      32

struct caam_hash_template {
	char name[CRYPTO_MAX_ALG_NAME];
	unsigned int digestsize;
	u32 alg_type;
};

enum caam_hash_algos {
	SHA1 = 0,
	SHA256
};

static struct caam_hash_template driver_hash[] = {
	{
		.name = "sha1",
		.digestsize = SHA1_DIGEST_SIZE,
		.alg_type = OP_ALG_ALGSEL_SHA1,
	},
	{
		.name = "sha256",
		.digestsize = SHA256_DIGEST_SIZE,
		.alg_type = OP_ALG_ALGSEL_SHA256,
	},
};

static enum caam_hash_algos get_hash_type(struct hash_algo *algo)
{
	if (!strcmp(algo->name, driver_hash[SHA1].name))
		return SHA1;
	else
		return SHA256;
}

/* Create the context for progressive hashing using h/w acceleration.
 *
 * @ctxp: Pointer to the pointer of the context for hashing
 * @caam_algo: Enum for SHA1 or SHA256
 * Return: 0 if ok, -ENOMEM on error
 */
static int caam_hash_init(void **ctxp, enum caam_hash_algos caam_algo)
{
	*ctxp = calloc(1, sizeof(struct sha_ctx));
	if (*ctxp == NULL) {
		debug("Cannot allocate memory for context\n");
		return -ENOMEM;
	}
	return 0;
}

/*
 * Update sg table for progressive hashing using h/w acceleration
 *
 * The context is freed by this function if an error occurs.
 * We support at most 32 Scatter/Gather Entries.
 *
 * @hash_ctx: Pointer to the context for hashing
 * @buf: Pointer to the buffer being hashed
 * @size: Size of the buffer being hashed
 * @is_last: 1 if this is the last update; 0 otherwise
 * @caam_algo: Enum for SHA1 or SHA256
 * Return: 0 if ok, -EINVAL on error
 */
static int caam_hash_update(void *hash_ctx, const void *buf,
			    unsigned int size, int is_last,
			    enum caam_hash_algos caam_algo)
{
	uint32_t final;
	caam_dma_addr_t addr = virt_to_phys((void *)buf);
	struct sha_ctx *ctx = hash_ctx;

	if (ctx->sg_num >= MAX_SG_32) {
		free(ctx);
		return -EINVAL;
	}

#ifdef CONFIG_CAAM_64BIT
	sec_out32(&ctx->sg_tbl[ctx->sg_num].addr_hi, (uint32_t)(addr >> 32));
#else
	sec_out32(&ctx->sg_tbl[ctx->sg_num].addr_hi, 0x0);
#endif
	sec_out32(&ctx->sg_tbl[ctx->sg_num].addr_lo, (caam_dma_addr_t)addr);

	sec_out32(&ctx->sg_tbl[ctx->sg_num].len_flag,
		  (size & SG_ENTRY_LENGTH_MASK));

	ctx->sg_num++;

	if (is_last) {
		final = sec_in32(&ctx->sg_tbl[ctx->sg_num - 1].len_flag) |
			SG_ENTRY_FINAL_BIT;
		sec_out32(&ctx->sg_tbl[ctx->sg_num - 1].len_flag, final);
	}

	return 0;
}

/*
 * Perform progressive hashing on the given buffer and copy hash at
 * destination buffer
 *
 * The context is freed after successful completion of hash operation.
 * In case of failure, context is not freed.
 * @hash_ctx: Pointer to the context for hashing
 * @dest_buf: Pointer to the destination buffer where hash is to be copied
 * @size: Size of the buffer being hashed
 * @caam_algo: Enum for SHA1 or SHA256
 * Return: 0 if ok, -EINVAL on error
 */
static int caam_hash_finish(void *hash_ctx, void *dest_buf,
			    int size, enum caam_hash_algos caam_algo)
{
	uint32_t len = 0, sg_entry_len;
	struct sha_ctx *ctx = hash_ctx;
	int i = 0, ret = 0;
	caam_dma_addr_t addr;

	if (size < driver_hash[caam_algo].digestsize) {
		return -EINVAL;
	}

	flush_dcache_range((ulong)ctx->sg_tbl,
			   (ulong)(ctx->sg_tbl) + (ctx->sg_num * sizeof(struct sg_entry)));
	for (i = 0; i < ctx->sg_num; i++) {
		sg_entry_len = (sec_in32(&ctx->sg_tbl[i].len_flag) &
				SG_ENTRY_LENGTH_MASK);
		len += sg_entry_len;
#ifdef CONFIG_CAAM_64BIT
		addr = sec_in32(&ctx->sg_tbl[i].addr_hi);
		addr = (addr << 32) | sec_in32(&ctx->sg_tbl[i].addr_lo);
#else
		addr = sec_in32(&ctx->sg_tbl[i].addr_lo);
#endif
		flush_dcache_range(addr, addr + sg_entry_len);
	}
	inline_cnstr_jobdesc_hash(ctx->sha_desc, (uint8_t *)ctx->sg_tbl, len,
				  ctx->hash,
				  driver_hash[caam_algo].alg_type,
				  driver_hash[caam_algo].digestsize,
				  1);

	flush_dcache_range((ulong)ctx->sha_desc,
			   (ulong)(ctx->sha_desc) + (sizeof(uint32_t) * MAX_CAAM_DESCSIZE));
	flush_dcache_range((ulong)ctx->hash,
			   (ulong)(ctx->hash) + driver_hash[caam_algo].digestsize);

	ret = run_descriptor_jr(ctx->sha_desc);

	if (ret) {
		debug("Error %x\n", ret);
		return ret;
	} else {
		invalidate_dcache_range((ulong)ctx->hash,
					(ulong)(ctx->hash) + driver_hash[caam_algo].digestsize);
		memcpy(dest_buf, ctx->hash, sizeof(ctx->hash));
	}
	free(ctx);
	return ret;
}

int caam_hash(const unsigned char *pbuf, unsigned int buf_len,
	      unsigned char *pout, enum caam_hash_algos algo)
{
	int ret = 0;
	uint32_t *desc;
	unsigned int size;

	desc = malloc_cache_aligned(sizeof(int) * MAX_CAAM_DESCSIZE);
	if (!desc) {
		debug("Not enough memory for descriptor allocation\n");
		return -ENOMEM;
	}

	size = ALIGN(buf_len, ARCH_DMA_MINALIGN);
	flush_dcache_range((unsigned long)pbuf, (unsigned long)pbuf + size);

	inline_cnstr_jobdesc_hash(desc, pbuf, buf_len, pout,
				  driver_hash[algo].alg_type,
				  driver_hash[algo].digestsize,
				  0);

	size = ALIGN(sizeof(int) * MAX_CAAM_DESCSIZE, ARCH_DMA_MINALIGN);
	flush_dcache_range((unsigned long)desc, (unsigned long)desc + size);
	size = ALIGN(driver_hash[algo].digestsize, ARCH_DMA_MINALIGN);
	invalidate_dcache_range((unsigned long)pout, (unsigned long)pout + size);

	ret = run_descriptor_jr(desc);

	size = ALIGN(driver_hash[algo].digestsize, ARCH_DMA_MINALIGN);
	invalidate_dcache_range((unsigned long)pout,
				(unsigned long)pout + size);

	free(desc);
	return ret;
}

void hw_sha256(const unsigned char *pbuf, unsigned int buf_len,
			unsigned char *pout, unsigned int chunk_size)
{
	if (caam_hash(pbuf, buf_len, pout, SHA256))
		printf("CAAM was not setup properly or it is faulty\n");
}

void hw_sha1(const unsigned char *pbuf, unsigned int buf_len,
			unsigned char *pout, unsigned int chunk_size)
{
	if (caam_hash(pbuf, buf_len, pout, SHA1))
		printf("CAAM was not setup properly or it is faulty\n");
}

int hw_sha_init(struct hash_algo *algo, void **ctxp)
{
	return caam_hash_init(ctxp, get_hash_type(algo));
}

int hw_sha_update(struct hash_algo *algo, void *ctx, const void *buf,
			    unsigned int size, int is_last)
{
	return caam_hash_update(ctx, buf, size, is_last, get_hash_type(algo));
}

int hw_sha_finish(struct hash_algo *algo, void *ctx, void *dest_buf,
		     int size)
{
	return caam_hash_finish(ctx, dest_buf, size, get_hash_type(algo));
}
