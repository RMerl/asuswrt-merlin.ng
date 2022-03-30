// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/arch/clock.h>
#include <linux/ctype.h>
#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>
#endif
#include <tsec.h>
#include <asm/arch/immap_ls102xa.h>
#include <fsl_sec.h>

DECLARE_GLOBAL_DATA_PTR;

void ft_fixup_enet_phy_connect_type(void *fdt)
{
	struct eth_device *dev;
	struct tsec_private *priv;
	const char *enet_path, *phy_path;
	char enet[16];
	char phy[16];
	int phy_node;
	int i = 0;
	uint32_t ph;
	char *name[3] = { "eTSEC1", "eTSEC2", "eTSEC3" };

	for (; i < ARRAY_SIZE(name); i++) {
		dev = eth_get_dev_by_name(name[i]);
		if (dev) {
			sprintf(enet, "ethernet%d", i);
			sprintf(phy, "enet%d_rgmii_phy", i);
		} else {
			continue;
		}

		priv = dev->priv;
		if (priv->flags & TSEC_SGMII)
			continue;

		enet_path = fdt_get_alias(fdt, enet);
		if (!enet_path)
			continue;

		phy_path = fdt_get_alias(fdt, phy);
		if (!phy_path)
			continue;

		phy_node = fdt_path_offset(fdt, phy_path);
		if (phy_node < 0)
			continue;

		ph = fdt_create_phandle(fdt, phy_node);
		if (ph)
			do_fixup_by_path_u32(fdt, enet_path,
					     "phy-handle", ph, 1);

		do_fixup_by_path(fdt, enet_path, "phy-connection-type",
				 phy_string_for_interface(
				 PHY_INTERFACE_MODE_RGMII_ID),
				 strlen(phy_string_for_interface(
				 PHY_INTERFACE_MODE_RGMII_ID)) + 1,
				 1);
	}
}

void ft_cpu_setup(void *blob, bd_t *bd)
{
	int off;
	int val;
	const char *sysclk_path;
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	unsigned int svr;
	svr = in_be32(&gur->svr);

	unsigned long busclk = get_bus_freq(0);

	/* delete crypto node if not on an E-processor */
	if (!IS_E_PROCESSOR(svr))
		fdt_fixup_crypto_node(blob, 0);
#if CONFIG_SYS_FSL_SEC_COMPAT >= 4
	else {
		ccsr_sec_t __iomem *sec;

		sec = (void __iomem *)CONFIG_SYS_FSL_SEC_ADDR;
		fdt_fixup_crypto_node(blob, sec_in32(&sec->secvid_ms));
	}
#endif

	off = fdt_node_offset_by_prop_value(blob, -1, "device_type", "cpu", 4);
	while (off != -FDT_ERR_NOTFOUND) {
		val = gd->cpu_clk;
		fdt_setprop(blob, off, "clock-frequency", &val, 4);
		off = fdt_node_offset_by_prop_value(blob, off,
						    "device_type", "cpu", 4);
	}

	do_fixup_by_prop_u32(blob, "device_type", "soc",
			     4, "bus-frequency", busclk, 1);

	ft_fixup_enet_phy_connect_type(blob);

#ifdef CONFIG_SYS_NS16550
	do_fixup_by_compat_u32(blob, "fsl,16550-FIFO64",
			       "clock-frequency", CONFIG_SYS_NS16550_CLK, 1);
#endif

	sysclk_path = fdt_get_alias(blob, "sysclk");
	if (sysclk_path)
		do_fixup_by_path_u32(blob, sysclk_path, "clock-frequency",
				     CONFIG_SYS_CLK_FREQ, 1);
	do_fixup_by_compat_u32(blob, "fsl,qoriq-sysclk-2.0",
			       "clock-frequency", CONFIG_SYS_CLK_FREQ, 1);

#if defined(CONFIG_DEEP_SLEEP) && defined(CONFIG_SD_BOOT)
#define UBOOT_HEAD_LEN	0x1000
	/*
	 * Reserved memory in SD boot deep sleep case.
	 * Second stage uboot binary and malloc space should be reserved.
	 * If the memory they occupied has not been reserved, then this
	 * space would be used by kernel and overwritten in uboot when
	 * deep sleep resume, which cause deep sleep failed.
	 * Since second uboot binary has a head, that space need to be
	 * reserved either(assuming its size is less than 0x1000).
	 */
	off = fdt_add_mem_rsv(blob, CONFIG_SYS_TEXT_BASE - UBOOT_HEAD_LEN,
			CONFIG_SYS_MONITOR_LEN + CONFIG_SYS_SPL_MALLOC_SIZE +
			UBOOT_HEAD_LEN);
	if (off < 0)
		printf("Failed to reserve memory for SD boot deep sleep: %s\n",
		       fdt_strerror(off));
#endif

#if defined(CONFIG_FSL_ESDHC)
	fdt_fixup_esdhc(blob, bd);
#endif

	/*
	 * platform bus clock = system bus clock/2
	 * Here busclk = system bus clock
	 * We are using the platform bus clock as 1588 Timer reference
	 * clock source select
	 */
	do_fixup_by_compat_u32(blob, "fsl, gianfar-ptp-timer",
			       "timer-frequency", busclk / 2, 1);

	/*
	 * clock-freq should change to clock-frequency and
	 * flexcan-v1.0 should change to p1010-flexcan respectively
	 * in the future.
	 */
	do_fixup_by_compat_u32(blob, "fsl, flexcan-v1.0",
			       "clock_freq", busclk / 2, 1);

	do_fixup_by_compat_u32(blob, "fsl, flexcan-v1.0",
			       "clock-frequency", busclk / 2, 1);

	do_fixup_by_compat_u32(blob, "fsl, ls1021a-flexcan",
			       "clock-frequency", busclk / 2, 1);

#if defined(CONFIG_QSPI_BOOT) || defined(CONFIG_SD_BOOT_QSPI)
	off = fdt_node_offset_by_compat_reg(blob, FSL_IFC_COMPAT,
					    CONFIG_SYS_IFC_ADDR);
	fdt_set_node_status(blob, off, FDT_STATUS_DISABLED, 0);
#else
	off = fdt_node_offset_by_compat_reg(blob, FSL_QSPI_COMPAT,
					    QSPI0_BASE_ADDR);
	fdt_set_node_status(blob, off, FDT_STATUS_DISABLED, 0);
	off = fdt_node_offset_by_compat_reg(blob, FSL_DSPI_COMPAT,
					    DSPI1_BASE_ADDR);
	fdt_set_node_status(blob, off, FDT_STATUS_DISABLED, 0);
#endif
}
