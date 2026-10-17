// SPDX-License-Identifier: GPL-2.0+
/**
 * cdns-platform.c - Platform driver for Cadence UFSHCI device
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <ufs.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/time.h>

#include "ufs.h"

#define CDNS_UFS_REG_HCLKDIV	0xFC
#define CDNS_UFS_REG_PHY_XCFGD1	0x113C

static int cdns_ufs_link_startup_notify(struct ufs_hba *hba,
					enum ufs_notify_change_status status)
{
	hba->quirks |= UFSHCD_QUIRK_BROKEN_LCC;
	switch (status) {
	case PRE_CHANGE:
		return ufshcd_dme_set(hba,
				      UIC_ARG_MIB(PA_LOCAL_TX_LCC_ENABLE),
				      0);
	case POST_CHANGE:
	;
	}

	return 0;
}

static int cdns_ufs_set_hclkdiv(struct ufs_hba *hba)
{
	struct clk clk;
	unsigned long core_clk_rate = 0;
	u32 core_clk_div = 0;
	int ret;

	ret = clk_get_by_name(hba->dev, "core_clk", &clk);
	if (ret) {
		dev_err(hba->dev, "failed to get core_clk clock\n");
		return ret;
	}

	core_clk_rate = clk_get_rate(&clk);
	if (IS_ERR_VALUE(core_clk_rate)) {
		dev_err(hba->dev, "%s: unable to find core_clk rate\n",
			__func__);
		return core_clk_rate;
	}

	core_clk_div = core_clk_rate / USEC_PER_SEC;
	ufshcd_writel(hba, core_clk_div, CDNS_UFS_REG_HCLKDIV);

	return 0;
}

static int cdns_ufs_hce_enable_notify(struct ufs_hba *hba,
				      enum ufs_notify_change_status status)
{
	switch (status) {
	case PRE_CHANGE:
		return cdns_ufs_set_hclkdiv(hba);
	case POST_CHANGE:
	;
	}

	return 0;
}

static int cdns_ufs_init(struct ufs_hba *hba)
{
	u32 data;

	/* Increase RX_Advanced_Min_ActivateTime_Capability */
	data = ufshcd_readl(hba, CDNS_UFS_REG_PHY_XCFGD1);
	data |= BIT(24);
	ufshcd_writel(hba, data, CDNS_UFS_REG_PHY_XCFGD1);

	return 0;
}

static struct ufs_hba_ops cdns_pltfm_hba_ops = {
	.init = cdns_ufs_init,
	.hce_enable_notify = cdns_ufs_hce_enable_notify,
	.link_startup_notify = cdns_ufs_link_startup_notify,
};

static int cdns_ufs_pltfm_probe(struct udevice *dev)
{
	int err = ufshcd_probe(dev, &cdns_pltfm_hba_ops);
	if (err)
		dev_err(dev, "ufshcd_probe() failed %d\n", err);

	return err;
}

static int cdns_ufs_pltfm_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;

	return ufs_scsi_bind(dev, &scsi_dev);
}

static const struct udevice_id cdns_ufs_pltfm_ids[] = {
	{
		.compatible = "cdns,ufshc-m31-16nm",
	},
	{},
};

U_BOOT_DRIVER(cdns_ufs_pltfm) = {
	.name		= "cdns-ufs-pltfm",
	.id		= UCLASS_UFS,
	.of_match	= cdns_ufs_pltfm_ids,
	.probe		= cdns_ufs_pltfm_probe,
	.bind		= cdns_ufs_pltfm_bind,
};
