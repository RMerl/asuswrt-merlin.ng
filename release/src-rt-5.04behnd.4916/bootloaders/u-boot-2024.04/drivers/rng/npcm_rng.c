// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <rng.h>
#include <uboot_aes.h>
#include <asm/io.h>

#define RNGCS_RNGE              BIT(0)
#define RNGCS_DVALID            BIT(1)
#define RNGCS_CLKP(range)       ((0x0f & (range)) << 2)
#define RNGMODE_M1ROSEL_VAL     (0x02) /* Ring Oscillator Select for Method I */

enum {
	RNG_CLKP_80_100_MHZ = 0x00, /*default */
	RNG_CLKP_60_80_MHZ  = 0x01,
	RNG_CLKP_50_60_MHZ  = 0x02,
	RNG_CLKP_40_50_MHZ  = 0x03,
	RNG_CLKP_30_40_MHZ  = 0x04,
	RNG_CLKP_25_30_MHZ  = 0x05,
	RNG_CLKP_20_25_MHZ  = 0x06,
	RNG_CLKP_5_20_MHZ   = 0x07,
	RNG_CLKP_2_15_MHZ   = 0x08,
	RNG_CLKP_9_12_MHZ   = 0x09,
	RNG_CLKP_7_9_MHZ    = 0x0A,
	RNG_CLKP_6_7_MHZ    = 0x0B,
	RNG_CLKP_5_6_MHZ    = 0x0C,
	RNG_CLKP_4_5_MHZ    = 0x0D,
	RNG_CLKP_3_4_MHZ    = 0x0E,
	RNG_NUM_OF_CLKP
};

struct npcm_rng_regs {
	unsigned int rngcs;
	unsigned int rngd;
	unsigned int rngmode;
};

struct npcm_rng_priv {
	struct npcm_rng_regs *regs;
};

static struct npcm_rng_priv *rng_priv;

void npcm_rng_init(void)
{
	struct npcm_rng_regs *regs = rng_priv->regs;
	int init;

	/* check if rng enabled */
	init = readb(&regs->rngcs);
	if ((init & RNGCS_RNGE) == 0) {
		/* init rng */
		writeb(RNGCS_CLKP(RNG_CLKP_20_25_MHZ) | RNGCS_RNGE, &regs->rngcs);
		writeb(RNGMODE_M1ROSEL_VAL, &regs->rngmode);
	}
}

void npcm_rng_disable(void)
{
	struct npcm_rng_regs *regs = rng_priv->regs;

	/* disable rng */
	writeb(0, &regs->rngcs);
	writeb(0, &regs->rngmode);
}

void srand(unsigned int seed)
{
	/* no need to seed for now */
}

int npcm_rng_read(struct udevice *dev, void *data, size_t max)
{
	struct npcm_rng_regs *regs = rng_priv->regs;
	int  i;
	int ret_val = 0;
	char *buf = data;

	npcm_rng_init();

	printf("NPCM HW RNG\n");
	/* Wait for RNG done (max bytes) */
	for (i = 0; i < max; i++) {
		 /* wait until DVALID is set */
		while ((readb(&regs->rngcs) & RNGCS_DVALID) == 0)
			;
		buf[i] = ((unsigned int)readb(&regs->rngd) & 0x000000FF);
	}

	return ret_val;
}

unsigned int rand_r(unsigned int *seedp)
{
	struct npcm_rng_regs *regs = rng_priv->regs;
	int  i;
	unsigned int ret_val = 0;

	npcm_rng_init();

	/* Wait for RNG done (4 bytes) */
	for (i = 0; i < 4 ; i++) {
		/* wait until DVALID is set */
		while ((readb(&regs->rngcs) & RNGCS_DVALID) == 0)
			;
		ret_val |= (((unsigned int)readb(&regs->rngd) & 0x000000FF) << (i * 8));
	}

	return ret_val;
}

unsigned int rand(void)
{
	return rand_r(NULL);
}

static int npcm_rng_bind(struct udevice *dev)
{
	rng_priv = calloc(1, sizeof(struct npcm_rng_priv));
	if (!rng_priv)
		return -ENOMEM;

	rng_priv->regs = dev_remap_addr_index(dev, 0);
	if (!rng_priv->regs) {
		printf("Cannot find rng reg address, binding failed\n");
		return -EINVAL;
	}

	printf("RNG: NPCM RNG module bind OK\n");

	return 0;
}

static const struct udevice_id npcm_rng_ids[] = {
	{ .compatible = "nuvoton,npcm845-rng" },
	{ .compatible = "nuvoton,npcm750-rng" },
	{ }
};

static const struct dm_rng_ops npcm_rng_ops = {
	.read = npcm_rng_read,
};

U_BOOT_DRIVER(npcm_rng) = {
	.name = "npcm_rng",
	.id = UCLASS_RNG,
	.ops = &npcm_rng_ops,
	.of_match = npcm_rng_ids,
	.priv_auto = sizeof(struct npcm_rng_priv),
	.bind = npcm_rng_bind,
};
