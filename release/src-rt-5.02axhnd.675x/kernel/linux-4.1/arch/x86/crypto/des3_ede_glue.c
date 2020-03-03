/*
 * Glue Code for assembler optimized version of 3DES
 *
 * Copyright © 2014 Jussi Kivilinna <jussi.kivilinna@mbnet.fi>
 *
 * CBC & ECB parts based on code (crypto/cbc.c,ecb.c) by:
 *   Copyright (c) 2006 Herbert Xu <herbert@gondor.apana.org.au>
 * CTR part based on code (crypto/ctr.c) by:
 *   (C) Copyright IBM Corp. 2007 - Joy Latten <latten@us.ibm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <asm/processor.h>
#include <crypto/des.h>
#include <linux/crypto.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <crypto/algapi.h>

struct des3_ede_x86_ctx {
	u32 enc_expkey[DES3_EDE_EXPKEY_WORDS];
	u32 dec_expkey[DES3_EDE_EXPKEY_WORDS];
};

/* regular block cipher functions */
asmlinkage void des3_ede_x86_64_crypt_blk(const u32 *expkey, u8 *dst,
					  const u8 *src);

/* 3-way parallel cipher functions */
asmlinkage void des3_ede_x86_64_crypt_blk_3way(const u32 *expkey, u8 *dst,
					       const u8 *src);

static inline void des3_ede_enc_blk(struct des3_ede_x86_ctx *ctx, u8 *dst,
				    const u8 *src)
{
	u32 *enc_ctx = ctx->enc_expkey;

	des3_ede_x86_64_crypt_blk(enc_ctx, dst, src);
}

static inline void des3_ede_dec_blk(struct des3_ede_x86_ctx *ctx, u8 *dst,
				    const u8 *src)
{
	u32 *dec_ctx = ctx->dec_expkey;

	des3_ede_x86_64_crypt_blk(dec_ctx, dst, src);
}

static inline void des3_ede_enc_blk_3way(struct des3_ede_x86_ctx *ctx, u8 *dst,
					 const u8 *src)
{
	u32 *enc_ctx = ctx->enc_expkey;

	des3_ede_x86_64_crypt_blk_3way(enc_ctx, dst, src);
}

static inline void des3_ede_dec_blk_3way(struct des3_ede_x86_ctx *ctx, u8 *dst,
					 const u8 *src)
{
	u32 *dec_ctx = ctx->dec_expkey;

	des3_ede_x86_64_crypt_blk_3way(dec_ctx, dst, src);
}

static void des3_ede_x86_encrypt(struct crypto_tfm *tfm, u8 *dst, const u8 *src)
{
	des3_ede_enc_blk(crypto_tfm_ctx(tfm), dst, src);
}

static void des3_ede_x86_decrypt(struct crypto_tfm *tfm, u8 *dst, const u8 *src)
{
	des3_ede_dec_blk(crypto_tfm_ctx(tfm), dst, src);
}

static int ecb_crypt(struct blkcipher_desc *desc, struct blkcipher_walk *walk,
		     const u32 *expkey)
{
	unsigned int bsize = DES3_EDE_BLOCK_SIZE;
	unsigned int nbytes;
	int err;

	err = blkcipher_walk_virt(desc, walk);

	while ((nbytes = walk->nbytes)) {
		u8 *wsrc = walk->src.virt.addr;
		u8 *wdst = walk->dst.virt.addr;

		/* Process four block batch */
		if (nbytes >= bsize * 3) {
			do {
				des3_ede_x86_64_crypt_blk_3way(expkey, wdst,
							       wsrc);

				wsrc += bsize * 3;
				wdst += bsize * 3;
				nbytes -= bsize * 3;
			} while (nbytes >= bsize * 3);

			if (nbytes < bsize)
				goto done;
		}

		/* Handle leftovers */
		do {
			des3_ede_x86_64_crypt_blk(expkey, wdst, wsrc);

			wsrc += bsize;
			wdst += bsize;
			nbytes -= bsize;
		} while (nbytes >= bsize);

done:
		err = blkcipher_walk_done(desc, walk, nbytes);
	}

	return err;
}

static int ecb_encrypt(struct blkcipher_desc *desc, struct scatterlist *dst,
		       struct scatterlist *src, unsigned int nbytes)
{
	struct des3_ede_x86_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
	struct blkcipher_walk walk;

	blkcipher_walk_init(&walk, dst, src, nbytes);
	return ecb_crypt(desc, &walk, ctx->enc_expkey);
}

static int ecb_decrypt(struct blkcipher_desc *desc, struct scatterlist *dst,
		       struct scatterlist *src, unsigned int nbytes)
{
	struct des3_ede_x86_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
	struct blkcipher_walk walk;

	blkcipher_walk_init(&walk, dst, src, nbytes);
	return ecb_crypt(desc, &walk, ctx->dec_expkey);
}

static unsigned int __cbc_encrypt(struct blkcipher_desc *desc,
				  struct blkcipher_walk *walk)
{
	struct des3_ede_x86_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
	unsigned int bsize = DES3_EDE_BLOCK_SIZE;
	unsigned int nbytes = walk->nbytes;
	u64 *src = (u64 *)walk->src.virt.addr;
	u64 *dst = (u64 *)walk->dst.virt.addr;
	u64 *iv = (u64 *)walk->iv;

	do {
		*dst = *src ^ *iv;
		des3_ede_enc_blk(ctx, (u8 *)dst, (u8 *)dst);
		iv = dst;

		src += 1;
		dst += 1;
		nbytes -= bsize;
	} while (nbytes >= bsize);

	*(u64 *)walk->iv = *iv;
	return nbytes;
}

static int cbc_encrypt(struct blkcipher_desc *desc, struct scatterlist *dst,
		       struct scatterlist *src, unsigned int nbytes)
{
	struct blkcipher_walk walk;
	int err;

	blkcipher_walk_init(&walk, dst, src, nbytes);
	err = blkcipher_walk_virt(desc, &walk);

	while ((nbytes = walk.nbytes)) {
		nbytes = __cbc_encrypt(desc, &walk);
		err = blkcipher_walk_done(desc, &walk, nbytes);
	}

	return err;
}

static unsigned int __cbc_decrypt(struct blkcipher_desc *desc,
				  struct blkcipher_walk *walk)
{
	struct des3_ede_x86_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
	unsigned int bsize = DES3_EDE_BLOCK_SIZE;
	unsigned int nbytes = walk->nbytes;
	u64 *src = (u64 *)walk->src.virt.addr;
	u64 *dst = (u64 *)walk->dst.virt.addr;
	u64 ivs[3 - 1];
	u64 last_iv;

	/* Start of the last block. */
	src += nbytes / bsize - 1;
	dst += nbytes / bsize - 1;

	last_iv = *src;

	/* Process four block batch */
	if (nbytes >= bsize * 3) {
		do {
			nbytes -= bsize * 3 - bsize;
			src -= 3 - 1;
			dst -= 3 - 1;

			ivs[0] = src[0];
			ivs[1] = src[1];

			des3_ede_dec_blk_3way(ctx, (u8 *)dst, (u8 *)src);

			dst[1] ^= ivs[0];
			dst[2] ^= ivs[1];

			nbytes -= bsize;
			if (nbytes < bsize)
				goto done;

			*dst ^= *(src - 1);
			src -= 1;
			dst -= 1;
		} while (nbytes >= bsize * 3);
	}

	/* Handle leftovers */
	for (;;) {
		des3_ede_dec_blk(ctx, (u8 *)dst, (u8 *)src);

		nbytes -= bsize;
		if (nbytes < bsize)
			break;

		*dst ^= *(src - 1);
		src -= 1;
		dst -= 1;
	}

done:
	*dst ^= *(u64 *)walk->iv;
	*(u64 *)walk->iv = last_iv;

	return nbytes;
}

static int cbc_decrypt(struct blkcipher_desc *desc, struct scatterlist *dst,
		       struct scatterlist *src, unsigned int nbytes)
{
	struct blkcipher_walk walk;
	int err;

	blkcipher_walk_init(&walk, dst, src, nbytes);
	err = blkcipher_walk_virt(desc, &walk);

	while ((nbytes = walk.nbytes)) {
		nbytes = __cbc_decrypt(desc, &walk);
		err = blkcipher_walk_done(desc, &walk, nbytes);
	}

	return err;
}

static void ctr_crypt_final(struct des3_ede_x86_ctx *ctx,
			    struct blkcipher_walk *walk)
{
	u8 *ctrblk = walk->iv;
	u8 keystream[DES3_EDE_BLOCK_SIZE];
	u8 *src = walk->src.virt.addr;
	u8 *dst = walk->dst.virt.addr;
	unsigned int nbytes = walk->nbytes;

	des3_ede_enc_blk(ctx, keystream, ctrblk);
	crypto_xor(keystream, src, nbytes);
	memcpy(dst, keystream, nbytes);

	crypto_inc(ctrblk, DES3_EDE_BLOCK_SIZE);
}

static unsigned int __ctr_crypt(struct blkcipher_desc *desc,
				struct blkcipher_walk *walk)
{
	struct des3_ede_x86_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
	unsigned int bsize = DES3_EDE_BLOCK_SIZE;
	unsigned int nbytes = walk->nbytes;
	__be64 *src = (__be64 *)walk->src.virt.addr;
	__be64 *dst = (__be64 *)walk->dst.virt.addr;
	u64 ctrblk = be64_to_cpu(*(__be64 *)walk->iv);
	__be64 ctrblocks[3];

	/* Process four block batch */
	if (nbytes >= bsize * 3) {
		do {
			/* create ctrblks for parallel encrypt */
			ctrblocks[0] = cpu_to_be64(ctrblk++);
			ctrblocks[1] = cpu_to_be64(ctrblk++);
			ctrblocks[2] = cpu_to_be64(ctrblk++);

			des3_ede_enc_blk_3way(ctx, (u8 *)ctrblocks,
					      (u8 *)ctrblocks);

			dst[0] = src[0] ^ ctrblocks[0];
			dst[1] = src[1] ^ ctrblocks[1];
			dst[2] = src[2] ^ ctrblocks[2];

			src += 3;
			dst += 3;
		} while ((nbytes -= bsize * 3) >= bsize * 3);

		if (nbytes < bsize)
			goto done;
	}

	/* Handle leftovers */
	do {
		ctrblocks[0] = cpu_to_be64(ctrblk++);

		des3_ede_enc_blk(ctx, (u8 *)ctrblocks, (u8 *)ctrblocks);

		dst[0] = src[0] ^ ctrblocks[0];

		src += 1;
		dst += 1;
	} while ((nbytes -= bsize) >= bsize);

done:
	*(__be64 *)walk->iv = cpu_to_be64(ctrblk);
	return nbytes;
}

static int ctr_crypt(struct blkcipher_desc *desc, struct scatterlist *dst,
		     struct scatterlist *src, unsigned int nbytes)
{
	struct blkcipher_walk walk;
	int err;

	blkcipher_walk_init(&walk, dst, src, nbytes);
	err = blkcipher_walk_virt_block(desc, &walk, DES3_EDE_BLOCK_SIZE);

	while ((nbytes = walk.nbytes) >= DES3_EDE_BLOCK_SIZE) {
		nbytes = __ctr_crypt(desc, &walk);
		err = blkcipher_walk_done(desc, &walk, nbytes);
	}

	if (walk.nbytes) {
		ctr_crypt_final(crypto_blkcipher_ctx(desc->tfm), &walk);
		err = blkcipher_walk_done(desc, &walk, 0);
	}

	return err;
}

static int des3_ede_x86_setkey(struct crypto_tfm *tfm, const u8 *key,
			       unsigned int keylen)
{
	struct des3_ede_x86_ctx *ctx = crypto_tfm_ctx(tfm);
	u32 i, j, tmp;
	int err;

	/* Generate encryption context using generic implementation. */
	err = __des3_ede_setkey(ctx->enc_expkey, &tfm->crt_flags, key, keylen);
	if (err < 0)
		return err;

	/* Fix encryption context for this implementation and form decryption
	 * context. */
	j = DES3_EDE_EXPKEY_WORDS - 2;
	for (i = 0; i < DES3_EDE_EXPKEY_WORDS; i += 2, j -= 2) {
		tmp = ror32(ctx->enc_expkey[i + 1], 4);
		ctx->enc_expkey[i + 1] = tmp;

		ctx->dec_expkey[j + 0] = ctx->enc_expkey[i + 0];
		ctx->dec_expkey[j + 1] = tmp;
	}

	return 0;
}

static struct crypto_alg des3_ede_algs[4] = { {
	.cra_name		= "des3_ede",
	.cra_driver_name	= "des3_ede-asm",
	.cra_priority		= 200,
	.cra_flags		= CRYPTO_ALG_TYPE_CIPHER,
	.cra_blocksize		= DES3_EDE_BLOCK_SIZE,
	.cra_ctxsize		= sizeof(struct des3_ede_x86_ctx),
	.cra_alignmask		= 0,
	.cra_module		= THIS_MODULE,
	.cra_u = {
		.cipher = {
			.cia_min_keysize	= DES3_EDE_KEY_SIZE,
			.cia_max_keysize	= DES3_EDE_KEY_SIZE,
			.cia_setkey		= des3_ede_x86_setkey,
			.cia_encrypt		= des3_ede_x86_encrypt,
			.cia_decrypt		= des3_ede_x86_decrypt,
		}
	}
}, {
	.cra_name		= "ecb(des3_ede)",
	.cra_driver_name	= "ecb-des3_ede-asm",
	.cra_priority		= 300,
	.cra_flags		= CRYPTO_ALG_TYPE_BLKCIPHER,
	.cra_blocksize		= DES3_EDE_BLOCK_SIZE,
	.cra_ctxsize		= sizeof(struct des3_ede_x86_ctx),
	.cra_alignmask		= 0,
	.cra_type		= &crypto_blkcipher_type,
	.cra_module		= THIS_MODULE,
	.cra_u = {
		.blkcipher = {
			.min_keysize	= DES3_EDE_KEY_SIZE,
			.max_keysize	= DES3_EDE_KEY_SIZE,
			.setkey		= des3_ede_x86_setkey,
			.encrypt	= ecb_encrypt,
			.decrypt	= ecb_decrypt,
		},
	},
}, {
	.cra_name		= "cbc(des3_ede)",
	.cra_driver_name	= "cbc-des3_ede-asm",
	.cra_priority		= 300,
	.cra_flags		= CRYPTO_ALG_TYPE_BLKCIPHER,
	.cra_blocksize		= DES3_EDE_BLOCK_SIZE,
	.cra_ctxsize		= sizeof(struct des3_ede_x86_ctx),
	.cra_alignmask		= 0,
	.cra_type		= &crypto_blkcipher_type,
	.cra_module		= THIS_MODULE,
	.cra_u = {
		.blkcipher = {
			.min_keysize	= DES3_EDE_KEY_SIZE,
			.max_keysize	= DES3_EDE_KEY_SIZE,
			.ivsize		= DES3_EDE_BLOCK_SIZE,
			.setkey		= des3_ede_x86_setkey,
			.encrypt	= cbc_encrypt,
			.decrypt	= cbc_decrypt,
		},
	},
}, {
	.cra_name		= "ctr(des3_ede)",
	.cra_driver_name	= "ctr-des3_ede-asm",
	.cra_priority		= 300,
	.cra_flags		= CRYPTO_ALG_TYPE_BLKCIPHER,
	.cra_blocksize		= 1,
	.cra_ctxsize		= sizeof(struct des3_ede_x86_ctx),
	.cra_alignmask		= 0,
	.cra_type		= &crypto_blkcipher_type,
	.cra_module		= THIS_MODULE,
	.cra_u = {
		.blkcipher = {
			.min_keysize	= DES3_EDE_KEY_SIZE,
			.max_keysize	= DES3_EDE_KEY_SIZE,
			.ivsize		= DES3_EDE_BLOCK_SIZE,
			.setkey		= des3_ede_x86_setkey,
			.encrypt	= ctr_crypt,
			.decrypt	= ctr_crypt,
		},
	},
} };

static bool is_blacklisted_cpu(void)
{
	if (boot_cpu_data.x86_vendor != X86_VENDOR_INTEL)
		return false;

	if (boot_cpu_data.x86 == 0x0f) {
		/*
		 * On Pentium 4, des3_ede-x86_64 is slower than generic C
		 * implementation because use of 64bit rotates (which are really
		 * slow on P4). Therefore blacklist P4s.
		 */
		return true;
	}

	return false;
}

static int force;
module_param(force, int, 0);
MODULE_PARM_DESC(force, "Force module load, ignore CPU blacklist");

static int __init des3_ede_x86_init(void)
{
	if (!force && is_blacklisted_cpu()) {
		pr_info("des3_ede-x86_64: performance on this CPU would be suboptimal: disabling des3_ede-x86_64.\n");
		return -ENODEV;
	}

	return crypto_register_algs(des3_ede_algs, ARRAY_SIZE(des3_ede_algs));
}

static void __exit des3_ede_x86_fini(void)
{
	crypto_unregister_algs(des3_ede_algs, ARRAY_SIZE(des3_ede_algs));
}

module_init(des3_ede_x86_init);
module_exit(des3_ede_x86_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Triple DES EDE Cipher Algorithm, asm optimized");
MODULE_ALIAS_CRYPTO("des3_ede");
MODULE_ALIAS_CRYPTO("des3_ede-asm");
MODULE_AUTHOR("Jussi Kivilinna <jussi.kivilinna@iki.fi>");
