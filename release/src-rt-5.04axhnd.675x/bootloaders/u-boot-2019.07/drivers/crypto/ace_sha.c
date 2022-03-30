// SPDX-License-Identifier: GPL-2.0+
/*
 * Advanced Crypto Engine - SHA Firmware
 * Copyright (c) 2012  Samsung Electronics
 */
#include <common.h>
#include "ace_sha.h"

#ifdef CONFIG_SHA_HW_ACCEL
#include <u-boot/sha256.h>
#include <u-boot/sha1.h>
#include <linux/errno.h>

/* SHA1 value for the message of zero length */
static const unsigned char sha1_digest_emptymsg[SHA1_SUM_LEN] = {
	0xDA, 0x39, 0xA3, 0xEE, 0x5E, 0x6B, 0x4B, 0x0D,
	0x32, 0x55, 0xBF, 0xFF, 0x95, 0x60, 0x18, 0x90,
	0xAF, 0xD8, 0x07, 0x09};

/* SHA256 value for the message of zero length */
static const unsigned char sha256_digest_emptymsg[SHA256_SUM_LEN] = {
	0xE3, 0xB0, 0xC4, 0x42, 0x98, 0xFC, 0x1C, 0x14,
	0x9A, 0xFB, 0xF4, 0xC8, 0x99, 0x6F, 0xB9, 0x24,
	0x27, 0xAE, 0x41, 0xE4, 0x64, 0x9B, 0x93, 0x4C,
	0xA4, 0x95, 0x99, 0x1B, 0x78, 0x52, 0xB8, 0x55};

int ace_sha_hash_digest(const unsigned char *pbuf, unsigned int buf_len,
			unsigned char *pout, unsigned int hash_type)
{
	unsigned int i, reg, len;
	unsigned int *pdigest;
	struct exynos_ace_sfr *ace_sha_reg =
		(struct exynos_ace_sfr *)samsung_get_base_ace_sfr();

	if (buf_len == 0) {
		/* ACE H/W cannot compute hash value for empty string */
		if (hash_type == ACE_SHA_TYPE_SHA1)
			memcpy(pout, sha1_digest_emptymsg, SHA1_SUM_LEN);
		else
			memcpy(pout, sha256_digest_emptymsg, SHA256_SUM_LEN);
		return 0;
	}

	/* Flush HRDMA */
	writel(ACE_FC_HRDMACFLUSH_ON, &ace_sha_reg->fc_hrdmac);
	writel(ACE_FC_HRDMACFLUSH_OFF, &ace_sha_reg->fc_hrdmac);

	/* Set byte swap of data in */
	writel(ACE_HASH_SWAPDI_ON | ACE_HASH_SWAPDO_ON | ACE_HASH_SWAPIV_ON,
	       &ace_sha_reg->hash_byteswap);

	/* Select Hash input mux as external source */
	reg = readl(&ace_sha_reg->fc_fifoctrl);
	reg = (reg & ~ACE_FC_SELHASH_MASK) | ACE_FC_SELHASH_EXOUT;
	writel(reg, &ace_sha_reg->fc_fifoctrl);

	/* Set Hash as SHA1 or SHA256 and start Hash engine */
	reg = (hash_type == ACE_SHA_TYPE_SHA1) ?
		ACE_HASH_ENGSEL_SHA1HASH : ACE_HASH_ENGSEL_SHA256HASH;
	reg |= ACE_HASH_STARTBIT_ON;
	writel(reg, &ace_sha_reg->hash_control);

	/* Enable FIFO mode */
	writel(ACE_HASH_FIFO_ON, &ace_sha_reg->hash_fifo_mode);

	/* Set message length */
	writel(buf_len, &ace_sha_reg->hash_msgsize_low);
	writel(0, &ace_sha_reg->hash_msgsize_high);

	/* Set HRDMA */
	writel((unsigned int)pbuf, &ace_sha_reg->fc_hrdmas);
	writel(buf_len, &ace_sha_reg->fc_hrdmal);

	while ((readl(&ace_sha_reg->hash_status) & ACE_HASH_MSGDONE_MASK) ==
		ACE_HASH_MSGDONE_OFF) {
		/*
		 * PRNG error bit goes HIGH if a PRNG request occurs without
		 * a complete seed setup. We are using this bit to check h/w
		 * fault because proper setup is not expected in that case.
		 */
		if ((readl(&ace_sha_reg->hash_status)
			& ACE_HASH_PRNGERROR_MASK) == ACE_HASH_PRNGERROR_ON)
			return -EBUSY;
	}

	/* Clear MSG_DONE bit */
	writel(ACE_HASH_MSGDONE_ON, &ace_sha_reg->hash_status);

	/* Read hash result */
	pdigest = (unsigned int *)pout;
	len = (hash_type == ACE_SHA_TYPE_SHA1) ? SHA1_SUM_LEN : SHA256_SUM_LEN;

	for (i = 0; i < len / 4; i++)
		pdigest[i] = readl(&ace_sha_reg->hash_result[i]);

	/* Clear HRDMA pending bit */
	writel(ACE_FC_HRDMA, &ace_sha_reg->fc_intpend);

	return 0;
}

void hw_sha256(const unsigned char *pbuf, unsigned int buf_len,
			unsigned char *pout, unsigned int chunk_size)
{
	if (ace_sha_hash_digest(pbuf, buf_len, pout, ACE_SHA_TYPE_SHA256))
		debug("ACE was not setup properly or it is faulty\n");
}

void hw_sha1(const unsigned char *pbuf, unsigned int buf_len,
			unsigned char *pout, unsigned int chunk_size)
{
	if (ace_sha_hash_digest(pbuf, buf_len, pout, ACE_SHA_TYPE_SHA1))
		debug("ACE was not setup properly or it is faulty\n");
}
#endif /* CONFIG_SHA_HW_ACCEL */

#ifdef CONFIG_LIB_HW_RAND
static unsigned int seed_done;

void srand(unsigned int seed)
{
	struct exynos_ace_sfr *reg =
		(struct exynos_ace_sfr *)samsung_get_base_ace_sfr();
	int i, status;

	/* Seed data */
	for (i = 0; i < ACE_HASH_PRNG_REG_NUM; i++)
		writel(seed << i, &reg->hash_seed[i]);

	/* Wait for seed setup done */
	while (1) {
		status = readl(&reg->hash_status);
		if ((status & ACE_HASH_SEEDSETTING_MASK) ||
		    (status & ACE_HASH_PRNGERROR_MASK))
			break;
	}

	seed_done = 1;
}

unsigned int rand(void)
{
	struct exynos_ace_sfr *reg =
		(struct exynos_ace_sfr *)samsung_get_base_ace_sfr();
	int i, status;
	unsigned int seed = (unsigned int)&status;
	unsigned int ret = 0;

	if (!seed_done)
		srand(seed);

	/* Start PRNG */
	writel(ACE_HASH_ENGSEL_PRNG | ACE_HASH_STARTBIT_ON, &reg->hash_control);

	/* Wait for PRNG done */
	while (1) {
		status = readl(&reg->hash_status);
		if (status & ACE_HASH_PRNGDONE_MASK)
			break;
		if (status & ACE_HASH_PRNGERROR_MASK) {
			seed_done = 0;
			return 0;
		}
	}

	/* Clear Done IRQ */
	writel(ACE_HASH_PRNGDONE_MASK, &reg->hash_status);

	/* Read a PRNG result */
	for (i = 0; i < ACE_HASH_PRNG_REG_NUM; i++)
		ret += readl(&reg->hash_prng[i]);

	seed_done = 0;
	return ret;
}

unsigned int rand_r(unsigned int *seedp)
{
	srand(*seedp);

	return rand();
}
#endif /* CONFIG_LIB_HW_RAND */
