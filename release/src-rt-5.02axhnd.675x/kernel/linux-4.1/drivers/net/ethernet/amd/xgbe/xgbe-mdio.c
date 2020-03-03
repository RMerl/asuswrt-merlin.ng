/*
 * AMD 10Gb Ethernet driver
 *
 * This file is available to you under your choice of the following two
 * licenses:
 *
 * License 1: GPLv2
 *
 * Copyright (c) 2014 Advanced Micro Devices, Inc.
 *
 * This file is free software; you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *     The Synopsys DWC ETHER XGMAC Software Driver and documentation
 *     (hereinafter "Software") is an unsupported proprietary work of Synopsys,
 *     Inc. unless otherwise expressly agreed to in writing between Synopsys
 *     and you.
 *
 *     The Software IS NOT an item of Licensed Software or Licensed Product
 *     under any End User Software License Agreement or Agreement for Licensed
 *     Product with Synopsys or any supplement thereto.  Permission is hereby
 *     granted, free of charge, to any person obtaining a copy of this software
 *     annotated with this license and the Software, to deal in the Software
 *     without restriction, including without limitation the rights to use,
 *     copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *     of the Software, and to permit persons to whom the Software is furnished
 *     to do so, subject to the following conditions:
 *
 *     The above copyright notice and this permission notice shall be included
 *     in all copies or substantial portions of the Software.
 *
 *     THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS"
 *     BASIS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *     TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *     PARTICULAR PURPOSE ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS
 *     BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *     CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *     SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *     INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *     ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *     THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * License 2: Modified BSD
 *
 * Copyright (c) 2014 Advanced Micro Devices, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Advanced Micro Devices, Inc. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *     The Synopsys DWC ETHER XGMAC Software Driver and documentation
 *     (hereinafter "Software") is an unsupported proprietary work of Synopsys,
 *     Inc. unless otherwise expressly agreed to in writing between Synopsys
 *     and you.
 *
 *     The Software IS NOT an item of Licensed Software or Licensed Product
 *     under any End User Software License Agreement or Agreement for Licensed
 *     Product with Synopsys or any supplement thereto.  Permission is hereby
 *     granted, free of charge, to any person obtaining a copy of this software
 *     annotated with this license and the Software, to deal in the Software
 *     without restriction, including without limitation the rights to use,
 *     copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *     of the Software, and to permit persons to whom the Software is furnished
 *     to do so, subject to the following conditions:
 *
 *     The above copyright notice and this permission notice shall be included
 *     in all copies or substantial portions of the Software.
 *
 *     THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS"
 *     BASIS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *     TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *     PARTICULAR PURPOSE ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS
 *     BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *     CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *     SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *     INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *     ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *     THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/of.h>

#include "xgbe.h"
#include "xgbe-common.h"

static int xgbe_mdio_read(struct mii_bus *mii, int prtad, int mmd_reg)
{
	struct xgbe_prv_data *pdata = mii->priv;
	struct xgbe_hw_if *hw_if = &pdata->hw_if;
	int mmd_data;

	DBGPR_MDIO("-->xgbe_mdio_read: prtad=%#x mmd_reg=%#x\n",
		   prtad, mmd_reg);

	mmd_data = hw_if->read_mmd_regs(pdata, prtad, mmd_reg);

	DBGPR_MDIO("<--xgbe_mdio_read: mmd_data=%#x\n", mmd_data);

	return mmd_data;
}

static int xgbe_mdio_write(struct mii_bus *mii, int prtad, int mmd_reg,
			   u16 mmd_val)
{
	struct xgbe_prv_data *pdata = mii->priv;
	struct xgbe_hw_if *hw_if = &pdata->hw_if;
	int mmd_data = mmd_val;

	DBGPR_MDIO("-->xgbe_mdio_write: prtad=%#x mmd_reg=%#x mmd_data=%#x\n",
		   prtad, mmd_reg, mmd_data);

	hw_if->write_mmd_regs(pdata, prtad, mmd_reg, mmd_data);

	DBGPR_MDIO("<--xgbe_mdio_write\n");

	return 0;
}

void xgbe_dump_phy_registers(struct xgbe_prv_data *pdata)
{
	struct device *dev = pdata->dev;
	struct phy_device *phydev = pdata->mii->phy_map[XGBE_PRTAD];
	int i;

	dev_alert(dev, "\n************* PHY Reg dump **********************\n");

	dev_alert(dev, "PCS Control Reg (%#04x) = %#04x\n", MDIO_CTRL1,
		  XMDIO_READ(pdata, MDIO_MMD_PCS, MDIO_CTRL1));
	dev_alert(dev, "PCS Status Reg (%#04x) = %#04x\n", MDIO_STAT1,
		  XMDIO_READ(pdata, MDIO_MMD_PCS, MDIO_STAT1));
	dev_alert(dev, "Phy Id (PHYS ID 1 %#04x)= %#04x\n", MDIO_DEVID1,
		  XMDIO_READ(pdata, MDIO_MMD_PCS, MDIO_DEVID1));
	dev_alert(dev, "Phy Id (PHYS ID 2 %#04x)= %#04x\n", MDIO_DEVID2,
		  XMDIO_READ(pdata, MDIO_MMD_PCS, MDIO_DEVID2));
	dev_alert(dev, "Devices in Package (%#04x)= %#04x\n", MDIO_DEVS1,
		  XMDIO_READ(pdata, MDIO_MMD_PCS, MDIO_DEVS1));
	dev_alert(dev, "Devices in Package (%#04x)= %#04x\n", MDIO_DEVS2,
		  XMDIO_READ(pdata, MDIO_MMD_PCS, MDIO_DEVS2));

	dev_alert(dev, "Auto-Neg Control Reg (%#04x) = %#04x\n", MDIO_CTRL1,
		  XMDIO_READ(pdata, MDIO_MMD_AN, MDIO_CTRL1));
	dev_alert(dev, "Auto-Neg Status Reg (%#04x) = %#04x\n", MDIO_STAT1,
		  XMDIO_READ(pdata, MDIO_MMD_AN, MDIO_STAT1));
	dev_alert(dev, "Auto-Neg Ad Reg 1 (%#04x) = %#04x\n",
		  MDIO_AN_ADVERTISE,
		  XMDIO_READ(pdata, MDIO_MMD_AN, MDIO_AN_ADVERTISE));
	dev_alert(dev, "Auto-Neg Ad Reg 2 (%#04x) = %#04x\n",
		  MDIO_AN_ADVERTISE + 1,
		  XMDIO_READ(pdata, MDIO_MMD_AN, MDIO_AN_ADVERTISE + 1));
	dev_alert(dev, "Auto-Neg Ad Reg 3 (%#04x) = %#04x\n",
		  MDIO_AN_ADVERTISE + 2,
		  XMDIO_READ(pdata, MDIO_MMD_AN, MDIO_AN_ADVERTISE + 2));
	dev_alert(dev, "Auto-Neg Completion Reg (%#04x) = %#04x\n",
		  MDIO_AN_COMP_STAT,
		  XMDIO_READ(pdata, MDIO_MMD_AN, MDIO_AN_COMP_STAT));

	dev_alert(dev, "MMD Device Mask = %#x\n",
		  phydev->c45_ids.devices_in_package);
	for (i = 0; i < ARRAY_SIZE(phydev->c45_ids.device_ids); i++)
		dev_alert(dev, "  MMD %d: ID = %#08x\n", i,
			  phydev->c45_ids.device_ids[i]);

	dev_alert(dev, "\n*************************************************\n");
}

int xgbe_mdio_register(struct xgbe_prv_data *pdata)
{
	struct mii_bus *mii;
	struct phy_device *phydev;
	int ret = 0;

	DBGPR("-->xgbe_mdio_register\n");

	mii = mdiobus_alloc();
	if (!mii) {
		dev_err(pdata->dev, "mdiobus_alloc failed\n");
		return -ENOMEM;
	}

	/* Register on the MDIO bus (don't probe any PHYs) */
	mii->name = XGBE_PHY_NAME;
	mii->read = xgbe_mdio_read;
	mii->write = xgbe_mdio_write;
	snprintf(mii->id, sizeof(mii->id), "%s", pdata->mii_bus_id);
	mii->priv = pdata;
	mii->phy_mask = ~0;
	mii->parent = pdata->dev;
	ret = mdiobus_register(mii);
	if (ret) {
		dev_err(pdata->dev, "mdiobus_register failed\n");
		goto err_mdiobus_alloc;
	}
	DBGPR("  mdiobus_register succeeded for %s\n", pdata->mii_bus_id);

	/* Probe the PCS using Clause 45 */
	phydev = get_phy_device(mii, XGBE_PRTAD, true);
	if (IS_ERR(phydev) || !phydev ||
	    !phydev->c45_ids.device_ids[MDIO_MMD_PCS]) {
		dev_err(pdata->dev, "get_phy_device failed\n");
		ret = phydev ? PTR_ERR(phydev) : -ENOLINK;
		goto err_mdiobus_register;
	}
	request_module(MDIO_MODULE_PREFIX MDIO_ID_FMT,
		       MDIO_ID_ARGS(phydev->c45_ids.device_ids[MDIO_MMD_PCS]));

	ret = phy_device_register(phydev);
	if (ret) {
		dev_err(pdata->dev, "phy_device_register failed\n");
		goto err_phy_device;
	}
	if (!phydev->dev.driver) {
		dev_err(pdata->dev, "phy driver probe failed\n");
		ret = -EIO;
		goto err_phy_device;
	}

	/* Add a reference to the PHY driver so it can't be unloaded */
	pdata->phy_module = phydev->dev.driver->owner;
	if (!try_module_get(pdata->phy_module)) {
		dev_err(pdata->dev, "try_module_get failed\n");
		ret = -EIO;
		goto err_phy_device;
	}

	pdata->mii = mii;
	pdata->mdio_mmd = MDIO_MMD_PCS;

	phydev->autoneg = pdata->default_autoneg;
	if (phydev->autoneg == AUTONEG_DISABLE) {
		phydev->speed = pdata->default_speed;
		phydev->duplex = DUPLEX_FULL;

		phydev->advertising &= ~ADVERTISED_Autoneg;
	}

	pdata->phydev = phydev;

	DBGPHY_REGS(pdata);

	DBGPR("<--xgbe_mdio_register\n");

	return 0;

err_phy_device:
	phy_device_free(phydev);

err_mdiobus_register:
	mdiobus_unregister(mii);

err_mdiobus_alloc:
	mdiobus_free(mii);

	return ret;
}

void xgbe_mdio_unregister(struct xgbe_prv_data *pdata)
{
	DBGPR("-->xgbe_mdio_unregister\n");

	pdata->phydev = NULL;

	module_put(pdata->phy_module);
	pdata->phy_module = NULL;

	mdiobus_unregister(pdata->mii);
	pdata->mii->priv = NULL;

	mdiobus_free(pdata->mii);
	pdata->mii = NULL;

	DBGPR("<--xgbe_mdio_unregister\n");
}
