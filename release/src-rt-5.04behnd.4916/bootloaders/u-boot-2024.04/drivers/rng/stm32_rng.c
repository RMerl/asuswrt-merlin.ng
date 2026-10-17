// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019, Linaro Limited
 */

#define LOG_CATEGORY UCLASS_RNG

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <reset.h>
#include <rng.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>

#define RNG_CR			0x00
#define RNG_CR_RNGEN		BIT(2)
#define RNG_CR_CED		BIT(5)
#define RNG_CR_CONFIG1		GENMASK(11, 8)
#define RNG_CR_NISTC		BIT(12)
#define RNG_CR_CONFIG2		GENMASK(15, 13)
#define RNG_CR_CLKDIV_SHIFT	16
#define RNG_CR_CLKDIV		GENMASK(19, 16)
#define RNG_CR_CONFIG3		GENMASK(25, 20)
#define RNG_CR_CONDRST		BIT(30)
#define RNG_CR_ENTROPY_SRC_MASK	(RNG_CR_CONFIG1 | RNG_CR_NISTC | RNG_CR_CONFIG2 | RNG_CR_CONFIG3)
#define RNG_CR_CONFIG_MASK	(RNG_CR_ENTROPY_SRC_MASK | RNG_CR_CED | RNG_CR_CLKDIV)

#define RNG_SR		0x04
#define RNG_SR_SEIS	BIT(6)
#define RNG_SR_CEIS	BIT(5)
#define RNG_SR_SECS	BIT(2)
#define RNG_SR_DRDY	BIT(0)

#define RNG_DR		0x08

#define RNG_NSCR		0x0C
#define RNG_NSCR_MASK		GENMASK(17, 0)

#define RNG_HTCR	0x10

#define RNG_NB_RECOVER_TRIES	3

/*
 * struct stm32_rng_data - RNG compat data
 *
 * @max_clock_rate:	Max RNG clock frequency, in Hertz
 * @cr:			Entropy source configuration
 * @nscr:		Noice sources control configuration
 * @htcr:		Health tests configuration
 * @has_cond_reset:	True if conditionnal reset is supported
 *
 */
struct stm32_rng_data {
	uint max_clock_rate;
	u32 cr;
	u32 nscr;
	u32 htcr;
	bool has_cond_reset;
};

struct stm32_rng_plat {
	fdt_addr_t base;
	struct clk clk;
	struct reset_ctl rst;
	const struct stm32_rng_data *data;
	bool ced;
};

/*
 * Extracts from the STM32 RNG specification when RNG supports CONDRST.
 *
 * When a noise source (or seed) error occurs, the RNG stops generating
 * random numbers and sets to “1” both SEIS and SECS bits to indicate
 * that a seed error occurred. (...)
 *
 * 1. Software reset by writing CONDRST at 1 and at 0 (see bitfield
 * description for details). This step is needed only if SECS is set.
 * Indeed, when SEIS is set and SECS is cleared it means RNG performed
 * the reset automatically (auto-reset).
 * 2. If SECS was set in step 1 (no auto-reset) wait for CONDRST
 * to be cleared in the RNG_CR register, then confirm that SEIS is
 * cleared in the RNG_SR register. Otherwise just clear SEIS bit in
 * the RNG_SR register.
 * 3. If SECS was set in step 1 (no auto-reset) wait for SECS to be
 * cleared by RNG. The random number generation is now back to normal.
 */
static int stm32_rng_conceal_seed_error_cond_reset(struct stm32_rng_plat *pdata)
{
	u32 sr = readl_relaxed(pdata->base + RNG_SR);
	u32 cr = readl_relaxed(pdata->base + RNG_CR);
	int err;

	if (sr & RNG_SR_SECS) {
		/* Conceal by resetting the subsystem (step 1.) */
		writel_relaxed(cr | RNG_CR_CONDRST, pdata->base + RNG_CR);
		writel_relaxed(cr & ~RNG_CR_CONDRST, pdata->base + RNG_CR);
	} else {
		/* RNG auto-reset (step 2.) */
		writel_relaxed(sr & ~RNG_SR_SEIS, pdata->base + RNG_SR);
		return 0;
	}

	err = readl_relaxed_poll_timeout(pdata->base + RNG_SR, sr, !(sr & RNG_CR_CONDRST), 100000);
	if (err) {
		log_err("%s: timeout %x\n", __func__, sr);
		return err;
	}

	/* Check SEIS is cleared (step 2.) */
	if (readl_relaxed(pdata->base + RNG_SR) & RNG_SR_SEIS)
		return -EINVAL;

	err = readl_relaxed_poll_timeout(pdata->base + RNG_SR, sr, !(sr & RNG_SR_SECS), 100000);
	if (err) {
		log_err("%s: timeout %x\n", __func__, sr);
		return err;
	}

	return 0;
}

/*
 * Extracts from the STM32 RNG specification, when CONDRST is not supported
 *
 * When a noise source (or seed) error occurs, the RNG stops generating
 * random numbers and sets to “1” both SEIS and SECS bits to indicate
 * that a seed error occurred. (...)
 *
 * The following sequence shall be used to fully recover from a seed
 * error after the RNG initialization:
 * 1. Clear the SEIS bit by writing it to “0”.
 * 2. Read out 12 words from the RNG_DR register, and discard each of
 * them in order to clean the pipeline.
 * 3. Confirm that SEIS is still cleared. Random number generation is
 * back to normal.
 */
static int stm32_rng_conceal_seed_error_sw_reset(struct stm32_rng_plat *pdata)
{
	uint i = 0;
	u32 sr = readl_relaxed(pdata->base + RNG_SR);

	writel_relaxed(sr & ~RNG_SR_SEIS, pdata->base + RNG_SR);

	for (i = 12; i != 0; i--)
		(void)readl_relaxed(pdata->base + RNG_DR);

	if (readl_relaxed(pdata->base + RNG_SR) & RNG_SR_SEIS)
		return -EINVAL;

	return 0;
}

static int stm32_rng_conceal_seed_error(struct stm32_rng_plat *pdata)
{
	log_debug("Concealing RNG seed error\n");

	if (pdata->data->has_cond_reset)
		return stm32_rng_conceal_seed_error_cond_reset(pdata);
	else
		return stm32_rng_conceal_seed_error_sw_reset(pdata);
};

static int stm32_rng_read(struct udevice *dev, void *data, size_t len)
{
	int retval;
	u32 sr, reg;
	size_t increment;
	struct stm32_rng_plat *pdata = dev_get_plat(dev);
	uint tries = 0;

	while (len > 0) {
		retval = readl_poll_timeout(pdata->base + RNG_SR, sr,
					    sr, 10000);
		if (retval) {
			log_err("%s: Timeout RNG no data",  __func__);
			return retval;
		}

		if (sr != RNG_SR_DRDY) {
			if (sr & RNG_SR_SEIS) {
				retval = stm32_rng_conceal_seed_error(pdata);
				tries++;
				if (retval || tries > RNG_NB_RECOVER_TRIES) {
					log_err("%s: Couldn't recover from seed error",  __func__);
					return -ENOTRECOVERABLE;
				}

				/* Start again */
				continue;
			}

			if (sr & RNG_SR_CEIS) {
				log_info("RNG clock too slow");
				writel_relaxed(0, pdata->base + RNG_SR);
			}
		}

		/*
		 * Once the DRDY bit is set, the RNG_DR register can
		 * be read up to four consecutive times.
		 */
		reg = readl(pdata->base + RNG_DR);
		/* Late seed error case: DR being 0 is an error status */
		if (!reg) {
			retval = stm32_rng_conceal_seed_error(pdata);
			tries++;

			if (retval || tries > RNG_NB_RECOVER_TRIES) {
				log_err("%s: Couldn't recover from seed error",  __func__);
				return -ENOTRECOVERABLE;
			}

			/* Start again */
			continue;
		}

		increment = min(len, sizeof(u32));
		memcpy(data, &reg, increment);
		data += increment;
		len -= increment;

		tries = 0;
	}

	return 0;
}

static uint stm32_rng_clock_freq_restrain(struct stm32_rng_plat *pdata)
{
	ulong clock_rate = 0;
	uint clock_div = 0;

	clock_rate = clk_get_rate(&pdata->clk);

	/*
	 * Get the exponent to apply on the CLKDIV field in RNG_CR register.
	 * No need to handle the case when clock-div > 0xF as it is physically
	 * impossible.
	 */
	while ((clock_rate >> clock_div) > pdata->data->max_clock_rate)
		clock_div++;

	log_debug("RNG clk rate : %lu\n", clk_get_rate(&pdata->clk) >> clock_div);

	return clock_div;
}

static int stm32_rng_init(struct stm32_rng_plat *pdata)
{
	int err;
	u32 cr, sr;

	err = clk_enable(&pdata->clk);
	if (err)
		return err;

	cr = readl(pdata->base + RNG_CR);

	/*
	 * Keep default RNG configuration if none was specified, that is when conf.cr is set to 0.
	 */
	if (pdata->data->has_cond_reset && pdata->data->cr) {
		uint clock_div = stm32_rng_clock_freq_restrain(pdata);

		cr &= ~RNG_CR_CONFIG_MASK;
		cr |= RNG_CR_CONDRST | (pdata->data->cr & RNG_CR_ENTROPY_SRC_MASK) |
		      (clock_div << RNG_CR_CLKDIV_SHIFT);
		if (pdata->ced)
			cr &= ~RNG_CR_CED;
		else
			cr |= RNG_CR_CED;
		writel(cr, pdata->base + RNG_CR);

		/* Health tests and noise control registers */
		writel_relaxed(pdata->data->htcr, pdata->base + RNG_HTCR);
		writel_relaxed(pdata->data->nscr & RNG_NSCR_MASK, pdata->base + RNG_NSCR);

		cr &= ~RNG_CR_CONDRST;
		cr |= RNG_CR_RNGEN;
		writel(cr, pdata->base + RNG_CR);
		err = readl_poll_timeout(pdata->base + RNG_CR, cr,
					 (!(cr & RNG_CR_CONDRST)), 10000);
		if (err) {
			log_err("%s: Timeout!",  __func__);
			return err;
		}
	} else {
		if (pdata->data->has_cond_reset)
			cr |= RNG_CR_CONDRST;

		if (pdata->ced)
			cr &= ~RNG_CR_CED;
		else
			cr |= RNG_CR_CED;

		writel(cr, pdata->base + RNG_CR);

		if (pdata->data->has_cond_reset)
			cr &= ~RNG_CR_CONDRST;

		cr |= RNG_CR_RNGEN;

		writel(cr, pdata->base + RNG_CR);
	}

	/* clear error indicators */
	writel(0, pdata->base + RNG_SR);

	err = readl_poll_timeout(pdata->base + RNG_SR, sr,
				 sr & RNG_SR_DRDY, 10000);
	if (err)
		log_err("%s: Timeout!",  __func__);

	return err;
}

static int stm32_rng_cleanup(struct stm32_rng_plat *pdata)
{
	writel(0, pdata->base + RNG_CR);

	return clk_disable(&pdata->clk);
}

static int stm32_rng_probe(struct udevice *dev)
{
	struct stm32_rng_plat *pdata = dev_get_plat(dev);

	pdata->data = (struct stm32_rng_data *)dev_get_driver_data(dev);

	reset_assert(&pdata->rst);
	udelay(20);
	reset_deassert(&pdata->rst);

	return stm32_rng_init(pdata);
}

static int stm32_rng_remove(struct udevice *dev)
{
	struct stm32_rng_plat *pdata = dev_get_plat(dev);

	return stm32_rng_cleanup(pdata);
}

static int stm32_rng_of_to_plat(struct udevice *dev)
{
	struct stm32_rng_plat *pdata = dev_get_plat(dev);
	int err;

	pdata->base = dev_read_addr(dev);
	if (!pdata->base)
		return -ENOMEM;

	err = clk_get_by_index(dev, 0, &pdata->clk);
	if (err)
		return err;

	err = reset_get_by_index(dev, 0, &pdata->rst);
	if (err)
		return err;

	pdata->ced = dev_read_bool(dev, "clock-error-detect");

	return 0;
}

static const struct dm_rng_ops stm32_rng_ops = {
	.read = stm32_rng_read,
};

static const struct stm32_rng_data stm32mp13_rng_data = {
	.has_cond_reset = true,
	.max_clock_rate = 48000000,
	.htcr = 0x969D,
	.nscr = 0x2B5BB,
	.cr = 0xF00D00,
};

static const struct stm32_rng_data stm32_rng_data = {
	.has_cond_reset = false,
	.max_clock_rate = 3000000,
	/* Not supported */
	.htcr = 0,
	.nscr = 0,
	.cr = 0,
};

static const struct udevice_id stm32_rng_match[] = {
	{.compatible = "st,stm32mp13-rng", .data = (ulong)&stm32mp13_rng_data},
	{.compatible = "st,stm32-rng", .data = (ulong)&stm32_rng_data},
	{},
};

U_BOOT_DRIVER(stm32_rng) = {
	.name = "stm32-rng",
	.id = UCLASS_RNG,
	.of_match = stm32_rng_match,
	.ops = &stm32_rng_ops,
	.probe = stm32_rng_probe,
	.remove = stm32_rng_remove,
	.plat_auto	= sizeof(struct stm32_rng_plat),
	.of_to_plat = stm32_rng_of_to_plat,
};
