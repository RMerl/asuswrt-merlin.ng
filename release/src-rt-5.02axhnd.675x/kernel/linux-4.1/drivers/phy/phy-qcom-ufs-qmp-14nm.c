/*
 * Copyright (c) 2013-2015, Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "phy-qcom-ufs-qmp-14nm.h"

#define UFS_PHY_NAME "ufs_phy_qmp_14nm"
#define UFS_PHY_VDDA_PHY_UV	(925000)

static
int ufs_qcom_phy_qmp_14nm_phy_calibrate(struct ufs_qcom_phy *ufs_qcom_phy,
					bool is_rate_B)
{
	int tbl_size_A = ARRAY_SIZE(phy_cal_table_rate_A);
	int tbl_size_B = ARRAY_SIZE(phy_cal_table_rate_B);
	int err;

	err = ufs_qcom_phy_calibrate(ufs_qcom_phy, phy_cal_table_rate_A,
		tbl_size_A, phy_cal_table_rate_B, tbl_size_B, is_rate_B);

	if (err)
		dev_err(ufs_qcom_phy->dev,
			"%s: ufs_qcom_phy_calibrate() failed %d\n",
			__func__, err);
	return err;
}

static
void ufs_qcom_phy_qmp_14nm_advertise_quirks(struct ufs_qcom_phy *phy_common)
{
	phy_common->quirks =
		UFS_QCOM_PHY_QUIRK_HIBERN8_EXIT_AFTER_PHY_PWR_COLLAPSE;
}

static int ufs_qcom_phy_qmp_14nm_init(struct phy *generic_phy)
{
	struct ufs_qcom_phy_qmp_14nm *phy = phy_get_drvdata(generic_phy);
	struct ufs_qcom_phy *phy_common = &phy->common_cfg;
	int err;

	err = ufs_qcom_phy_init_clks(generic_phy, phy_common);
	if (err) {
		dev_err(phy_common->dev, "%s: ufs_qcom_phy_init_clks() failed %d\n",
			__func__, err);
		goto out;
	}

	err = ufs_qcom_phy_init_vregulators(generic_phy, phy_common);
	if (err) {
		dev_err(phy_common->dev, "%s: ufs_qcom_phy_init_vregulators() failed %d\n",
			__func__, err);
		goto out;
	}
	phy_common->vdda_phy.max_uV = UFS_PHY_VDDA_PHY_UV;
	phy_common->vdda_phy.min_uV = UFS_PHY_VDDA_PHY_UV;

	ufs_qcom_phy_qmp_14nm_advertise_quirks(phy_common);

out:
	return err;
}

static
void ufs_qcom_phy_qmp_14nm_power_control(struct ufs_qcom_phy *phy, bool val)
{
	writel_relaxed(val ? 0x1 : 0x0, phy->mmio + UFS_PHY_POWER_DOWN_CONTROL);
	/*
	 * Before any transactions involving PHY, ensure PHY knows
	 * that it's analog rail is powered ON (or OFF).
	 */
	mb();
}

static inline
void ufs_qcom_phy_qmp_14nm_set_tx_lane_enable(struct ufs_qcom_phy *phy, u32 val)
{
	/*
	 * 14nm PHY does not have TX_LANE_ENABLE register.
	 * Implement this function so as not to propagate error to caller.
	 */
}

static inline void ufs_qcom_phy_qmp_14nm_start_serdes(struct ufs_qcom_phy *phy)
{
	u32 tmp;

	tmp = readl_relaxed(phy->mmio + UFS_PHY_PHY_START);
	tmp &= ~MASK_SERDES_START;
	tmp |= (1 << OFFSET_SERDES_START);
	writel_relaxed(tmp, phy->mmio + UFS_PHY_PHY_START);
	/* Ensure register value is committed */
	mb();
}

static int ufs_qcom_phy_qmp_14nm_is_pcs_ready(struct ufs_qcom_phy *phy_common)
{
	int err = 0;
	u32 val;

	err = readl_poll_timeout(phy_common->mmio + UFS_PHY_PCS_READY_STATUS,
		val, (val & MASK_PCS_READY), 10, 1000000);
	if (err)
		dev_err(phy_common->dev, "%s: poll for pcs failed err = %d\n",
			__func__, err);
	return err;
}

static struct phy_ops ufs_qcom_phy_qmp_14nm_phy_ops = {
	.init		= ufs_qcom_phy_qmp_14nm_init,
	.exit		= ufs_qcom_phy_exit,
	.power_on	= ufs_qcom_phy_power_on,
	.power_off	= ufs_qcom_phy_power_off,
	.owner		= THIS_MODULE,
};

static struct ufs_qcom_phy_specific_ops phy_14nm_ops = {
	.calibrate_phy		= ufs_qcom_phy_qmp_14nm_phy_calibrate,
	.start_serdes		= ufs_qcom_phy_qmp_14nm_start_serdes,
	.is_physical_coding_sublayer_ready = ufs_qcom_phy_qmp_14nm_is_pcs_ready,
	.set_tx_lane_enable	= ufs_qcom_phy_qmp_14nm_set_tx_lane_enable,
	.power_control		= ufs_qcom_phy_qmp_14nm_power_control,
};

static int ufs_qcom_phy_qmp_14nm_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct phy *generic_phy;
	struct ufs_qcom_phy_qmp_14nm *phy;
	int err = 0;

	phy = devm_kzalloc(dev, sizeof(*phy), GFP_KERNEL);
	if (!phy) {
		dev_err(dev, "%s: failed to allocate phy\n", __func__);
		err = -ENOMEM;
		goto out;
	}

	generic_phy = ufs_qcom_phy_generic_probe(pdev, &phy->common_cfg,
				&ufs_qcom_phy_qmp_14nm_phy_ops, &phy_14nm_ops);

	if (!generic_phy) {
		dev_err(dev, "%s: ufs_qcom_phy_generic_probe() failed\n",
			__func__);
		err = -EIO;
		goto out;
	}

	phy_set_drvdata(generic_phy, phy);

	strlcpy(phy->common_cfg.name, UFS_PHY_NAME,
		sizeof(phy->common_cfg.name));

out:
	return err;
}

static int ufs_qcom_phy_qmp_14nm_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct phy *generic_phy = to_phy(dev);
	struct ufs_qcom_phy *ufs_qcom_phy = get_ufs_qcom_phy(generic_phy);
	int err = 0;

	err = ufs_qcom_phy_remove(generic_phy, ufs_qcom_phy);
	if (err)
		dev_err(dev, "%s: ufs_qcom_phy_remove failed = %d\n",
			__func__, err);

	return err;
}

static const struct of_device_id ufs_qcom_phy_qmp_14nm_of_match[] = {
	{.compatible = "qcom,ufs-phy-qmp-14nm"},
	{},
};
MODULE_DEVICE_TABLE(of, ufs_qcom_phy_qmp_14nm_of_match);

static struct platform_driver ufs_qcom_phy_qmp_14nm_driver = {
	.probe = ufs_qcom_phy_qmp_14nm_probe,
	.remove = ufs_qcom_phy_qmp_14nm_remove,
	.driver = {
		.of_match_table = ufs_qcom_phy_qmp_14nm_of_match,
		.name = "ufs_qcom_phy_qmp_14nm",
		.owner = THIS_MODULE,
	},
};

module_platform_driver(ufs_qcom_phy_qmp_14nm_driver);

MODULE_DESCRIPTION("Universal Flash Storage (UFS) QCOM PHY QMP 14nm");
MODULE_LICENSE("GPL v2");
