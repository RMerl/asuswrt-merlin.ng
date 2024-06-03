// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Broadcom Corporation.
 */

#include <common.h>
#include <malloc.h>
#include <sdhci.h>
#include <linux/errno.h>
#include <asm/kona-common/clk.h>

#define SDHCI_CORECTRL_OFFSET		0x00008000
#define SDHCI_CORECTRL_EN		0x01
#define SDHCI_CORECTRL_RESET		0x02

#define SDHCI_CORESTAT_OFFSET		0x00008004
#define SDHCI_CORESTAT_CD_SW		0x01

#define SDHCI_COREIMR_OFFSET		0x00008008
#define SDHCI_COREIMR_IP		0x01

static int init_kona_mmc_core(struct sdhci_host *host)
{
	unsigned int mask;
	unsigned int timeout;

	if (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & SDHCI_RESET_ALL) {
		printf("%s: sd host controller reset error\n", __func__);
		return -EBUSY;
	}

	/* For kona a hardware reset before anything else. */
	mask = sdhci_readl(host, SDHCI_CORECTRL_OFFSET) | SDHCI_CORECTRL_RESET;
	sdhci_writel(host, mask, SDHCI_CORECTRL_OFFSET);

	/* Wait max 100 ms */
	timeout = 1000;
	do {
		if (timeout == 0) {
			printf("%s: reset timeout error\n", __func__);
			return -ETIMEDOUT;
		}
		timeout--;
		udelay(100);
	} while (0 ==
		 (sdhci_readl(host, SDHCI_CORECTRL_OFFSET) &
		  SDHCI_CORECTRL_RESET));

	/* Clear the reset bit. */
	mask = mask & ~SDHCI_CORECTRL_RESET;
	sdhci_writel(host, mask, SDHCI_CORECTRL_OFFSET);

	/* Enable AHB clock */
	mask = sdhci_readl(host, SDHCI_CORECTRL_OFFSET);
	sdhci_writel(host, mask | SDHCI_CORECTRL_EN, SDHCI_CORECTRL_OFFSET);

	/* Enable interrupts */
	sdhci_writel(host, SDHCI_COREIMR_IP, SDHCI_COREIMR_OFFSET);

	/* Make sure Card is detected in controller */
	mask = sdhci_readl(host, SDHCI_CORESTAT_OFFSET);
	sdhci_writel(host, mask | SDHCI_CORESTAT_CD_SW, SDHCI_CORESTAT_OFFSET);

	/* Wait max 100 ms */
	timeout = 1000;
	while (!(sdhci_readl(host, SDHCI_PRESENT_STATE) & SDHCI_CARD_PRESENT)) {
		if (timeout == 0) {
			printf("%s: CARD DETECT timeout error\n", __func__);
			return -ETIMEDOUT;
		}
		timeout--;
		udelay(100);
	}
	return 0;
}

int kona_sdhci_init(int dev_index, u32 min_clk, u32 quirks)
{
	int ret = 0;
	u32 max_clk;
	void *reg_base;
	struct sdhci_host *host = NULL;

	host = (struct sdhci_host *)malloc(sizeof(struct sdhci_host));
	if (!host) {
		printf("%s: sdhci host malloc fail!\n", __func__);
		return -ENOMEM;
	}
	switch (dev_index) {
	case 0:
		reg_base = (void *)CONFIG_SYS_SDIO_BASE0;
		ret = clk_sdio_enable(reg_base, CONFIG_SYS_SDIO0_MAX_CLK,
				      &max_clk);
		break;
	case 1:
		reg_base = (void *)CONFIG_SYS_SDIO_BASE1;
		ret = clk_sdio_enable(reg_base, CONFIG_SYS_SDIO1_MAX_CLK,
				      &max_clk);
		break;
	case 2:
		reg_base = (void *)CONFIG_SYS_SDIO_BASE2;
		ret = clk_sdio_enable(reg_base, CONFIG_SYS_SDIO2_MAX_CLK,
				      &max_clk);
		break;
	case 3:
		reg_base = (void *)CONFIG_SYS_SDIO_BASE3;
		ret = clk_sdio_enable(reg_base, CONFIG_SYS_SDIO3_MAX_CLK,
				      &max_clk);
		break;
	default:
		printf("%s: sdio dev index %d not supported\n",
		       __func__, dev_index);
		ret = -EINVAL;
	}
	if (ret) {
		free(host);
		return ret;
	}

	host->name = "kona-sdhci";
	host->ioaddr = reg_base;
	host->quirks = quirks;
	host->max_clk = max_clk;

	if (init_kona_mmc_core(host)) {
		free(host);
		return -EINVAL;
	}

	add_sdhci(host, 0, min_clk);
	return ret;
}
