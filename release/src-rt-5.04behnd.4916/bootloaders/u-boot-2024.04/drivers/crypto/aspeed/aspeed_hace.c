// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2021 ASPEED Technology Inc.
 */
#include <config.h>
#include <common.h>
#include <dm.h>
#include <clk.h>
#include <log.h>
#include <asm/io.h>
#include <malloc.h>
#include <watchdog.h>
#include <u-boot/hash.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/iopoll.h>

/* register offsets*/
#define HACE_STS		0x1C
#define   HACE_HASH_DATA_OVF		BIT(23)
#define   HACE_HASH_INT			BIT(9)
#define   HACE_HASH_BUSY		BIT(0)
#define HACE_HASH_DATA		0x20
#define HACE_HASH_DIGEST	0x24
#define HACE_HASH_HMAC_KEY	0x28
#define HACE_HASH_DATA_LEN	0x2C
#define HACE_HASH_CMD		0x30
#define   HACE_HASH_MODE_ACCUM		BIT(8)
#define   HACE_HASH_ALGO_SHA1		BIT(5)
#define   HACE_HASH_ALGO_SHA256		(BIT(6) | BIT(4))
#define   HACE_HASH_ALGO_SHA384		(BIT(10) | BIT(6) | BIT(5))
#define   HACE_HASH_ALGO_SHA512		(BIT(6) | BIT(5))
#define   HACE_HASH_SHA_BE_EN		BIT(3)

/* buffer size based on SHA-512 need*/
#define HASH_BLOCK_BUFSZ	128
#define HASH_DIGEST_BUFSZ	64

struct aspeed_hace_ctx {
	uint8_t digest[HASH_DIGEST_BUFSZ];

	uint32_t cmd;
	enum HASH_ALGO algo;

	uint32_t blk_size;
	uint32_t pad_size;
	uint64_t total[2];

	uint8_t buf[HASH_BLOCK_BUFSZ];
	uint32_t buf_cnt;
} __aligned((8));

struct aspeed_hace {
	phys_addr_t base;
	struct clk clk;
};

static const uint32_t iv_sha1[8] = {
	0x01234567, 0x89abcdef, 0xfedcba98, 0x76543210,
	0xf0e1d2c3, 0, 0, 0
};

static const uint32_t iv_sha256[8] = {
	0x67e6096a, 0x85ae67bb, 0x72f36e3c, 0x3af54fa5,
	0x7f520e51, 0x8c68059b, 0xabd9831f, 0x19cde05bUL
};

static const uint32_t iv_sha384[16] = {
	0x5d9dbbcb, 0xd89e05c1, 0x2a299a62, 0x07d57c36,
	0x5a015991, 0x17dd7030, 0xd8ec2f15, 0x39590ef7,
	0x67263367, 0x310bc0ff, 0x874ab48e, 0x11155868,
	0x0d2e0cdb, 0xa78ff964, 0x1d48b547, 0xa44ffabeUL
};

static const uint32_t iv_sha512[16] = {
	0x67e6096a, 0x08c9bcf3, 0x85ae67bb, 0x3ba7ca84,
	0x72f36e3c, 0x2bf894fe, 0x3af54fa5, 0xf1361d5f,
	0x7f520e51, 0xd182e6ad, 0x8c68059b, 0x1f6c3e2b,
	0xabd9831f, 0x6bbd41fb, 0x19cde05b, 0x79217e13UL
};

static int aspeed_hace_wait_completion(uint32_t reg, uint32_t flag, int timeout_us)
{
	uint32_t val;

	return readl_poll_timeout(reg, val, (val & flag) == flag, timeout_us);
}

static int aspeed_hace_process(struct udevice *dev, void *ctx, const void *ibuf, uint32_t ilen)
{
	struct aspeed_hace *hace = dev_get_priv(dev);
	struct aspeed_hace_ctx *hace_ctx = (struct aspeed_hace_ctx *)ctx;
	uint32_t sts = readl(hace->base + HACE_STS);

	if (sts & HACE_HASH_BUSY) {
		debug("HACE engine busy\n");
		return -EBUSY;
	}

	writel(HACE_HASH_INT, hace->base + HACE_STS);

	writel((uint32_t)ibuf, hace->base + HACE_HASH_DATA);
	writel((uint32_t)hace_ctx->digest, hace->base + HACE_HASH_DIGEST);
	writel((uint32_t)hace_ctx->digest, hace->base + HACE_HASH_HMAC_KEY);
	writel(ilen, hace->base + HACE_HASH_DATA_LEN);
	writel(hace_ctx->cmd, hace->base + HACE_HASH_CMD);

	return aspeed_hace_wait_completion(hace->base + HACE_STS,
					   HACE_HASH_INT,
					   1000 + (ilen >> 3));
}

static int aspeed_hace_init(struct udevice *dev, enum HASH_ALGO algo, void **ctxp)
{
	struct aspeed_hace_ctx *hace_ctx;

	hace_ctx = memalign(8, sizeof(struct aspeed_hace_ctx));
	if (!hace_ctx)
		return -ENOMEM;

	memset(hace_ctx, 0, sizeof(struct aspeed_hace_ctx));

	hace_ctx->algo = algo;
	hace_ctx->cmd = HACE_HASH_MODE_ACCUM | HACE_HASH_SHA_BE_EN;

	switch (algo) {
	case HASH_ALGO_SHA1:
		hace_ctx->blk_size = 64;
		hace_ctx->pad_size = 8;
		hace_ctx->cmd |= HACE_HASH_ALGO_SHA1;
		memcpy(hace_ctx->digest, iv_sha1, sizeof(iv_sha1));
		break;
	case HASH_ALGO_SHA256:
		hace_ctx->blk_size = 64;
		hace_ctx->pad_size = 8;
		hace_ctx->cmd |= HACE_HASH_ALGO_SHA256;
		memcpy(hace_ctx->digest, iv_sha256, sizeof(iv_sha256));
		break;
	case HASH_ALGO_SHA384:
		hace_ctx->blk_size = 128;
		hace_ctx->pad_size = 16;
		hace_ctx->cmd |= HACE_HASH_ALGO_SHA384;
		memcpy(hace_ctx->digest, iv_sha384, sizeof(iv_sha384));
		break;
	case HASH_ALGO_SHA512:
		hace_ctx->blk_size = 128;
		hace_ctx->pad_size = 16;
		hace_ctx->cmd |= HACE_HASH_ALGO_SHA512;
		memcpy(hace_ctx->digest, iv_sha512, sizeof(iv_sha512));
		break;
	default:
		debug("Unsupported hash algorithm '%s'\n", hash_algo_name(algo));
		goto free_n_out;
	};

	*ctxp = hace_ctx;

	return 0;

free_n_out:
	free(hace_ctx);

	return -EINVAL;
}

static int aspeed_hace_update(struct udevice *dev, void *ctx, const void *ibuf, uint32_t ilen)
{
	int rc;
	uint32_t left, fill;
	struct aspeed_hace_ctx *hace_ctx = ctx;

	left = hace_ctx->total[0] & (hace_ctx->blk_size - 1);
	fill = hace_ctx->blk_size - left;

	hace_ctx->total[0] += ilen;
	if (hace_ctx->total[0] < ilen)
		hace_ctx->total[1]++;

	if (left && ilen >= fill) {
		memcpy(hace_ctx->buf + left, ibuf, fill);
		rc = aspeed_hace_process(dev, ctx, hace_ctx->buf, hace_ctx->blk_size);
		if (rc) {
			debug("failed to process hash, rc=%d\n", rc);
			return rc;
		}
		ilen -= fill;
		ibuf += fill;
		left = 0;
	}

	while (ilen >= hace_ctx->blk_size) {
		rc = aspeed_hace_process(dev, ctx, ibuf, hace_ctx->blk_size);
		if (rc) {
			debug("failed to process hash, rc=%d\n", rc);
			return rc;
		}

		ibuf += hace_ctx->blk_size;
		ilen -= hace_ctx->blk_size;
	}

	if (ilen)
		memcpy(hace_ctx->buf + left, ibuf, ilen);

	return 0;
}

static int aspeed_hace_finish(struct udevice *dev, void *ctx, void *obuf)
{
	int rc = 0;
	uint8_t pad[HASH_BLOCK_BUFSZ * 2];
	uint32_t last, padn;
	uint64_t ibits_h, ibits_l;
	uint64_t ibits_be_h, ibits_be_l;
	struct aspeed_hace_ctx *hace_ctx = ctx;

	memset(pad, 0, sizeof(pad));
	pad[0] = 0x80;

	ibits_h = (hace_ctx->total[0] >> 61) | (hace_ctx->total[1] << 3);
	ibits_be_h = cpu_to_be64(ibits_h);

	ibits_l = (hace_ctx->total[0] << 3);
	ibits_be_l = cpu_to_be64(ibits_l);

	last = hace_ctx->total[0] & (hace_ctx->blk_size - 1);

	switch (hace_ctx->algo) {
	case HASH_ALGO_SHA1:
	case HASH_ALGO_SHA256:
		padn = (last < 56) ? (56 - last) : (120 - last);

		rc = aspeed_hace_update(dev, ctx, pad, padn);
		if (rc) {
			debug("failed to append padding, rc=%d\n", rc);
			goto free_n_out;
		}

		rc = aspeed_hace_update(dev, ctx, &ibits_be_l, sizeof(ibits_be_l));
		if (rc) {
			debug("failed to append message bits length, rc=%d\n", rc);
			goto free_n_out;
		}

		break;
	case HASH_ALGO_SHA384:
	case HASH_ALGO_SHA512:
		padn = (last < 112) ? (112 - last) : (240 - last);

		rc = aspeed_hace_update(dev, ctx, pad, padn);
		if (rc) {
			debug("failed to append padding, rc=%d\n", rc);
			goto free_n_out;
		}

		rc = aspeed_hace_update(dev, ctx, &ibits_be_h, sizeof(ibits_be_h)) |
		     aspeed_hace_update(dev, ctx, &ibits_be_l, sizeof(ibits_be_l));
		if (rc) {
			debug("failed to append message bits length, rc=%d\n", rc);
			goto free_n_out;
		}

		break;
	default:
		rc = -EINVAL;
		break;
	}

	memcpy(obuf, hace_ctx->digest, hash_algo_digest_size(hace_ctx->algo));

free_n_out:
	free(ctx);

	return rc;
}

static int aspeed_hace_digest_wd(struct udevice *dev, enum HASH_ALGO algo,
			      const void *ibuf, const uint32_t ilen,
			      void *obuf, uint32_t chunk_sz)
{
	int rc;
	void *ctx;
	const void *cur, *end;
	uint32_t chunk;

	rc = aspeed_hace_init(dev, algo, &ctx);
	if (rc)
		return rc;

	if (IS_ENABLED(CONFIG_HW_WATCHDOG) || CONFIG_IS_ENABLED(WATCHDOG)) {
		cur = ibuf;
		end = ibuf + ilen;

		while (cur < end) {
			chunk = end - cur;
			if (chunk > chunk_sz)
				chunk = chunk_sz;

			rc = aspeed_hace_update(dev, ctx, cur, chunk);
			if (rc)
				return rc;

			cur += chunk;
			schedule();
		}
	} else {
		rc = aspeed_hace_update(dev, ctx, ibuf, ilen);
		if (rc)
			return rc;
	}

	rc = aspeed_hace_finish(dev, ctx, obuf);
	if (rc)
		return rc;

	return 0;
}

static int aspeed_hace_digest(struct udevice *dev, enum HASH_ALGO algo,
			      const void *ibuf, const uint32_t ilen,
			      void *obuf)
{
	/* re-use the watchdog version with input length as the chunk_sz */
	return aspeed_hace_digest_wd(dev, algo, ibuf, ilen, obuf, ilen);
}

static int aspeed_hace_probe(struct udevice *dev)
{
	int rc;
	struct aspeed_hace *hace = dev_get_priv(dev);

	rc = clk_get_by_index(dev, 0, &hace->clk);
	if (rc < 0) {
		debug("cannot get clock for %s: %d\n", dev->name, rc);
		return rc;
	}

	rc = clk_enable(&hace->clk);
	if (rc) {
		debug("cannot enable clock for %s: %d\n", dev->name, rc);
		return rc;
	}

	hace->base = devfdt_get_addr(dev);

	return rc;
}

static int aspeed_hace_remove(struct udevice *dev)
{
	struct aspeed_hace *hace = dev_get_priv(dev);

	clk_disable(&hace->clk);

	return 0;
}

static const struct hash_ops aspeed_hace_ops = {
	.hash_init = aspeed_hace_init,
	.hash_update = aspeed_hace_update,
	.hash_finish = aspeed_hace_finish,
	.hash_digest_wd = aspeed_hace_digest_wd,
	.hash_digest = aspeed_hace_digest,
};

static const struct udevice_id aspeed_hace_ids[] = {
	{ .compatible = "aspeed,ast2600-hace" },
	{ }
};

U_BOOT_DRIVER(aspeed_hace) = {
	.name = "aspeed_hace",
	.id = UCLASS_HASH,
	.of_match = aspeed_hace_ids,
	.ops = &aspeed_hace_ops,
	.probe = aspeed_hace_probe,
	.remove	= aspeed_hace_remove,
	.priv_auto = sizeof(struct aspeed_hace),
	.flags = DM_FLAG_PRE_RELOC,
};
