/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/
/**
   \file smdio_access.c
    GPHY mode flashless load FW example.

*/

#include "smdio_access.h"
#include "lif_api.h"

#define SMIDO_SB_PHY_ADDR_REG 0x1F
#define SMIDO_SB_PHY_DATA_REG 0x0

int smdio_write(uint8_t lif_id, uint8_t phy, uint16_t phy_reg, uint16_t phy_reg_data)
{
    int ret = 0;

    /* Write address to register 0x1f */
    ret = lif_mdio_c22_write(lif_id, phy, SMIDO_SB_PHY_ADDR_REG, phy_reg);
    if (ret == 0)
    {
        /* Write data to offset 0x0 */
        ret = lif_mdio_c22_write(lif_id, phy, SMIDO_SB_PHY_DATA_REG, phy_reg_data);
    }

    return ret;
}

int smdio_cont_write(uint8_t lif_id, uint8_t phy, uint16_t phy_reg, uint16_t phy_reg_data[8], uint8_t num)
{
    int ret = 0;

    /* Write address to register 0x1f */
    ret = lif_mdio_c22_write(lif_id, phy, SMIDO_SB_PHY_ADDR_REG, phy_reg);
    if (ret == 0)
    {
        /* Write data to offset 0x0 */
        for (uint8_t i = 0; i < (num & 0xF); i++)
        {
            ret = lif_mdio_c22_write(lif_id, phy, SMIDO_SB_PHY_DATA_REG, phy_reg_data[i]);
        }
    }

    return ret;
}

int smdio_read(uint8_t lif_id, uint8_t phy, uint16_t phy_reg)
{
    int ret = 0;
    uint16_t readdata = -1;

    /* Write address to register 0x1f */
    ret = lif_mdio_c22_write(lif_id, phy, SMIDO_SB_PHY_ADDR_REG, phy_reg);
    if (ret == 0)
    {
        /* Read data from offset 0x0 */
        readdata = lif_mdio_c22_read(lif_id, phy, SMIDO_SB_PHY_DATA_REG);
    }

    return readdata;
}

int mdio_read(uint8_t lif_id, uint8_t phy, uint16_t phy_reg)
{
    int ret = 0;

    ret = lif_mdio_c22_read(lif_id, phy, phy_reg);

    return ret;
}

int smdio_init(void)
{
    /* Ensure the MDIO link is operational.
       this initialization is performed at the startup
       please check "cmds" entry point in cmds.c for an initialization
       example for RPI4 Host. */

    return 0;
}
