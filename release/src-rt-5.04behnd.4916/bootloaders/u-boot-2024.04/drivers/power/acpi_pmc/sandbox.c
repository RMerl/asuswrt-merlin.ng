// SPDX-License-Identifier: GPL-2.0
/*
 * Sandbox PMC for testing
 *
 * Copyright 2019 Google LLC
 */

#define LOG_CATEGORY UCLASS_ACPI_PMC

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/io.h>
#include <power/acpi_pmc.h>

#define GPIO_GPE_CFG		0x1050

/* Memory mapped IO registers behind PMC_BASE_ADDRESS */
#define PRSTS			0x1000
#define GEN_PMCON1		0x1020
#define GEN_PMCON2		0x1024
#define GEN_PMCON3		0x1028

/* Offset of TCO registers from ACPI base I/O address */
#define TCO_REG_OFFSET		0x60
#define TCO1_STS	0x64
#define TCO2_STS	0x66
#define TCO1_CNT	0x68
#define TCO2_CNT	0x6a

struct sandbox_pmc_priv {
	ulong base;
};

static int sandbox_pmc_fill_power_state(struct udevice *dev)
{
	struct acpi_pmc_upriv *upriv = dev_get_uclass_priv(dev);

	upriv->tco1_sts = inw(upriv->acpi_base + TCO1_STS);
	upriv->tco2_sts = inw(upriv->acpi_base + TCO2_STS);

	upriv->prsts = readl(upriv->pmc_bar0 + PRSTS);
	upriv->gen_pmcon1 = readl(upriv->pmc_bar0 + GEN_PMCON1);
	upriv->gen_pmcon2 = readl(upriv->pmc_bar0 + GEN_PMCON2);
	upriv->gen_pmcon3 = readl(upriv->pmc_bar0 + GEN_PMCON3);

	return 0;
}

static int sandbox_prev_sleep_state(struct udevice *dev, int prev_sleep_state)
{
	return prev_sleep_state;
}

static int sandbox_disable_tco(struct udevice *dev)
{
	struct acpi_pmc_upriv *upriv = dev_get_uclass_priv(dev);

	pmc_disable_tco_base(upriv->acpi_base + TCO_REG_OFFSET);

	return 0;
}

static int sandbox_pmc_probe(struct udevice *dev)
{
	struct acpi_pmc_upriv *upriv = dev_get_uclass_priv(dev);
	struct udevice *bus;
	ulong base;

	uclass_first_device(UCLASS_PCI, &bus);
	base = dm_pci_read_bar32(dev, 0);
	if (base == FDT_ADDR_T_NONE)
		return log_msg_ret("No base address", -EINVAL);
	upriv->pmc_bar0 = map_sysmem(base, 0x2000);
	upriv->gpe_cfg = (u32 *)(upriv->pmc_bar0 + GPIO_GPE_CFG);

	return pmc_ofdata_to_uc_plat(dev);
}

static struct acpi_pmc_ops sandbox_pmc_ops = {
	.init			= sandbox_pmc_fill_power_state,
	.prev_sleep_state	= sandbox_prev_sleep_state,
	.disable_tco		= sandbox_disable_tco,
};

static const struct udevice_id sandbox_pmc_ids[] = {
	{ .compatible = "sandbox,pmc" },
	{ }
};

U_BOOT_DRIVER(pmc_sandbox) = {
	.name = "pmc_sandbox",
	.id = UCLASS_ACPI_PMC,
	.of_match = sandbox_pmc_ids,
	.probe = sandbox_pmc_probe,
	.ops = &sandbox_pmc_ops,
	.priv_auto	= sizeof(struct sandbox_pmc_priv),
};
