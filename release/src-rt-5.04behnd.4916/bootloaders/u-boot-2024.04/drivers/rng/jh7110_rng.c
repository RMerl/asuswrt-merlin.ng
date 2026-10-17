// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * TRNG driver for the StarFive JH7110 SoC
 *
 */

#include <clk.h>
#include <dm.h>
#include <reset.h>
#include <rng.h>
#include <asm/io.h>
#include <linux/iopoll.h>

/* trng register offset */
#define STARFIVE_CTRL			0x00
#define STARFIVE_STAT			0x04
#define STARFIVE_MODE			0x08
#define STARFIVE_SMODE			0x0C
#define STARFIVE_IE			0x10
#define STARFIVE_ISTAT			0x14
#define STARFIVE_RAND0			0x20
#define STARFIVE_RAND1			0x24
#define STARFIVE_RAND2			0x28
#define STARFIVE_RAND3			0x2C
#define STARFIVE_RAND4			0x30
#define STARFIVE_RAND5			0x34
#define STARFIVE_RAND6			0x38
#define STARFIVE_RAND7			0x3C
#define STARFIVE_AUTO_RQSTS		0x60
#define STARFIVE_AUTO_AGE		0x64

/* CTRL CMD */
#define STARFIVE_CTRL_EXEC_NOP		0x0
#define STARFIVE_CTRL_GENE_RANDNUM	0x1
#define STARFIVE_CTRL_EXEC_RANDRESEED	0x2

/* STAT */
#define STARFIVE_STAT_NONCE_MODE	BIT(2)
#define STARFIVE_STAT_R256		BIT(3)
#define STARFIVE_STAT_MISSION_MODE	BIT(8)
#define STARFIVE_STAT_SEEDED		BIT(9)
#define STARFIVE_STAT_LAST_RESEED(x)	((x) << 16)
#define STARFIVE_STAT_SRVC_RQST		BIT(27)
#define STARFIVE_STAT_RAND_GENERATING	BIT(30)
#define STARFIVE_STAT_RAND_SEEDING	BIT(31)
#define STARFIVE_STAT_RUNNING		(STARFIVE_STAT_RAND_GENERATING | \
					 STARFIVE_STAT_RAND_SEEDING)

/* MODE */
#define STARFIVE_MODE_R256		BIT(3)

/* SMODE */
#define STARFIVE_SMODE_NONCE_MODE	BIT(2)
#define STARFIVE_SMODE_MISSION_MODE	BIT(8)
#define STARFIVE_SMODE_MAX_REJECTS(x)	((x) << 16)

/* IE */
#define STARFIVE_IE_RAND_RDY_EN		BIT(0)
#define STARFIVE_IE_SEED_DONE_EN	BIT(1)
#define STARFIVE_IE_LFSR_LOCKUP_EN	BIT(4)
#define STARFIVE_IE_GLBL_EN		BIT(31)

#define STARFIVE_IE_ALL			(STARFIVE_IE_GLBL_EN | \
					 STARFIVE_IE_RAND_RDY_EN | \
					 STARFIVE_IE_SEED_DONE_EN | \
					 STARFIVE_IE_LFSR_LOCKUP_EN)

/* ISTAT */
#define STARFIVE_ISTAT_RAND_RDY		BIT(0)
#define STARFIVE_ISTAT_SEED_DONE	BIT(1)
#define STARFIVE_ISTAT_LFSR_LOCKUP	BIT(4)

#define STARFIVE_RAND_LEN		sizeof(u32)

enum mode {
	PRNG_128BIT,
	PRNG_256BIT,
};

struct starfive_trng_plat {
	void *base;
	struct clk *hclk;
	struct clk *ahb;
	struct reset_ctl *rst;
	u32 mode;
};

static inline int starfive_trng_wait_idle(struct starfive_trng_plat *trng)
{
	u32 stat;

	return readl_relaxed_poll_timeout(trng->base + STARFIVE_STAT, stat,
					  !(stat & STARFIVE_STAT_RUNNING),
					  100000);
}

static inline void starfive_trng_irq_mask_clear(struct starfive_trng_plat *trng)
{
	/* clear register: ISTAT */
	u32 data = readl(trng->base + STARFIVE_ISTAT);

	writel(data, trng->base + STARFIVE_ISTAT);
}

static int starfive_trng_cmd(struct starfive_trng_plat *trng, u32 cmd)
{
	u32 stat, flg;
	int ret;

	switch (cmd) {
	case STARFIVE_CTRL_GENE_RANDNUM:
		writel(cmd, trng->base + STARFIVE_CTRL);
		flg = STARFIVE_ISTAT_RAND_RDY;
		break;
	case STARFIVE_CTRL_EXEC_RANDRESEED:
		writel(cmd, trng->base + STARFIVE_CTRL);
		flg = STARFIVE_ISTAT_SEED_DONE;
		break;
	default:
		return -EINVAL;
	}

	ret = readl_relaxed_poll_timeout(trng->base + STARFIVE_ISTAT, stat,
					 (stat & flg), 1000);
	writel(flg, trng->base + STARFIVE_ISTAT);

	return ret;
}

static int starfive_trng_read(struct udevice *dev, void *data, size_t len)
{
	struct starfive_trng_plat *trng = dev_get_plat(dev);
	u8 *buffer = data;
	int iter_mask;

	if (trng->mode == PRNG_256BIT)
		iter_mask = 7;
	else
		iter_mask = 3;

	for (int i = 0; len; ++i, i &= iter_mask) {
		u32 val;
		size_t step;
		int ret;

		if (!i) {
			ret = starfive_trng_cmd(trng,
						STARFIVE_CTRL_GENE_RANDNUM);
			if (ret)
				return ret;
		}

		val = readl(trng->base + STARFIVE_RAND0 +
			    (i * STARFIVE_RAND_LEN));
		step = min_t(size_t, len, STARFIVE_RAND_LEN);
		memcpy(buffer, &val, step);
		buffer += step;
		len -= step;
	}

	return 0;
}

static int starfive_trng_init(struct starfive_trng_plat *trng)
{
	u32 mode, intr = 0;

	/* setup Auto Request/Age register */
	writel(0, trng->base + STARFIVE_AUTO_AGE);
	writel(0, trng->base + STARFIVE_AUTO_RQSTS);

	/* clear register: ISTAT */
	starfive_trng_irq_mask_clear(trng);

	intr |= STARFIVE_IE_ALL;
	writel(intr, trng->base + STARFIVE_IE);

	mode = readl(trng->base + STARFIVE_MODE);

	switch (trng->mode) {
	case PRNG_128BIT:
		mode &= ~STARFIVE_MODE_R256;
		break;
	case PRNG_256BIT:
		mode |= STARFIVE_MODE_R256;
		break;
	default:
		mode |= STARFIVE_MODE_R256;
		break;
	}

	writel(mode, trng->base + STARFIVE_MODE);

	return starfive_trng_cmd(trng, STARFIVE_CTRL_EXEC_RANDRESEED);
}

static int starfive_trng_probe(struct udevice *dev)
{
	struct starfive_trng_plat *pdata = dev_get_plat(dev);
	int err;

	err = clk_enable(pdata->hclk);
	if (err)
		return err;

	err = clk_enable(pdata->ahb);
	if (err)
		goto err_ahb;

	err = reset_deassert(pdata->rst);
	if (err)
		goto err_reset;

	pdata->mode = PRNG_256BIT;

	err = starfive_trng_init(pdata);
	if (err)
		goto err_trng_init;

	return 0;

err_trng_init:
	reset_assert(pdata->rst);
err_reset:
	clk_disable(pdata->ahb);
err_ahb:
	clk_disable(pdata->hclk);

	return err;
}

static int starfive_trng_of_to_plat(struct udevice *dev)
{
	struct starfive_trng_plat *pdata = dev_get_plat(dev);

	pdata->base = (void *)dev_read_addr(dev);
	if (!pdata->base)
		return -ENODEV;

	pdata->hclk = devm_clk_get(dev, "hclk");
	if (IS_ERR(pdata->hclk))
		return -ENODEV;

	pdata->ahb = devm_clk_get(dev, "ahb");
	if (IS_ERR(pdata->ahb))
		return -ENODEV;

	pdata->rst = devm_reset_control_get(dev, NULL);
	if (IS_ERR(pdata->rst))
		return -ENODEV;

	return 0;
}

static const struct dm_rng_ops starfive_trng_ops = {
	.read = starfive_trng_read,
};

static const struct udevice_id starfive_trng_match[] = {
	{
		.compatible = "starfive,jh7110-trng",
	},
	{},
};

U_BOOT_DRIVER(starfive_trng) = {
	.name = "jh7110-trng",
	.id = UCLASS_RNG,
	.of_match = starfive_trng_match,
	.probe = starfive_trng_probe,
	.ops = &starfive_trng_ops,
	.plat_auto = sizeof(struct starfive_trng_plat),
	.of_to_plat = starfive_trng_of_to_plat,
};
