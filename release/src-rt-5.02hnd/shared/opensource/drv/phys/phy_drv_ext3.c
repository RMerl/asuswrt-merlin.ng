/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/

/*
 *  Created on: Jan 2016
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Phy driver for external 2.5G/10G phys: BCM84860, BCM84861
 */

#include "phy_drv.h"
#include "lport_drv.h"

extern unsigned int bcm8486x_image[];
extern unsigned int bcm8486x_image_size;
extern phy_drv_t phy_drv_ext3;

static uint32_t enabled_phys;

#define BUS_WRITE(a, b, c, d)       if ((ret = _bus_write(a, b, c, d))) goto Exit;
#define BUS_WRITE_ALL(a, b, c, d)   if ((ret = _bus_write_all(a, b, c, d))) goto Exit;

#define PHY_READ(a, b, c, d)        if ((ret = phy_bus_c45_read(a, b, c, d))) goto Exit;
#define PHY_WRITE(a, b, c, d)       if ((ret = phy_bus_c45_write(a, b, c, d))) goto Exit;

/* Command codes */
#define CMD_GET_PAIR_SWAP                           0x8000
#define CMD_SET_PAIR_SWAP                           0x8001
#define CMD_GET_1588_ENABLE                         0x8004
#define CMD_SET_1588_ENABLE                         0x8005
#define CMD_GET_SHORT_REACH_MODE_ENABLE             0x8006
#define CMD_SET_SHORT_REACH_MODE_ENABLE             0x8007
#define CMD_GET_EEE_MODE                            0x8008
#define CMD_SET_EEE_MODE                            0x8009
#define CMD_GET_EMI_MODE_ENABLE                     0x800A
#define CMD_SET_EMI_MODE_ENABLE                     0x800B
#define CMD_GET_SUB_LF_RF_STATUS                    0x800D
#define CMD_GET_SERDES_KR_MODE_ENABLE               0x800E
#define CMD_SET_SERDES_KR_MODE_ENABLE               0x800F
#define CMD_CLEAR_SUB_LF_RF                         0x8010
#define CMD_SET_SUB_LF_RF                           0x8011
#define CMD_READ_INDIRECT_GPHY_REG_BITS             0x8014
#define CMD_WRITE_INDIRECT_GPHY_REG_BITS            0x8015
#define CMD_GET_XFI_2P5G_MODE                       0x8016	
#define CMD_SET_XFI_2P5G_MODE                       0x8017
#define CMD_GET_2XGE_MODE                           0x8018
#define CMD_SET_2XGE_MODE                           0x8019
#define CMD_SETUP_EEE_STATISTICS                    0x801A
#define CMD_GET_EEE_STATISTICS                      0x801B
#define CMD_SET_JUMBO_PACKET                        0x801C
#define CMD_GET_JUMBO_PACKET                        0x801D
#define CMD_GET_MSE                                 0x801E
#define CMD_GET_INTERLEAVER                         0x8029
#define CMD_SET_INTERLEAVER                         0x802A
#define CMD_GET_XFI_TX_FILTERS                      0x802B
#define CMD_SET_XFI_TX_FILTERS                      0x802C
#define CMD_GET_XFI_POLARITY                        0x802D
#define CMD_SET_XFI_POLARITY                        0x802E
#define CMD_GET_CURRENT_VOLTAGE                     0x802F
#define CMD_GET_SNR                                 0x8030
#define CMD_GET_CURRENT_TEMP                        0x8031
#define CMD_SET_UPPER_TEMP_WARNING_LEVEL            0x8032
#define CMD_GET_UPPER_TEMP_WARNING_LEVEL            0x8033
#define CMD_SET_LOWER_TEMP_WARNING_LEVEL            0x8034
#define CMD_GET_LOWER_TEMP_WARNING_LEVEL            0x8035
#define CMD_GET_HW_FR_EMI_MODE_ENABLE               0x803A
#define CMD_SET_HW_FR_EMI_MODE_ENABLE               0x803B
#define CMD_GET_CUSTOMER_REQUESTED_TX_PWR_ADJUST    0x8040
#define CMD_SET_CUSTOMER_REQUESTED_TX_PWR_ADJUST    0x8041
#define CMD_GET_DYNAMIC_PARTITION_SELECT            0x8042
#define CMD_SET_DYNAMIC_PARTITION_SELECT            0x8043
#define CMD_RESET_STAT_LOG                          0xC017

/* Command hanlder status codes */
#define CMD_RECEIVED                                0x0001
#define CMD_IN_PROGRESS                             0x0002
#define CMD_COMPLETE_PASS                           0x0004
#define CMD_COMPLETE_ERROR                          0x0008
#define CMD_SYSTEM_BUSY                             0xBBBB

static int _wait_for_cmd_ready(phy_dev_t *phy_dev)
{ 
    int ret, i;
    uint16_t val;

    for (i = 0; i < 1000; i++)
    {
        /* Read status of command */
        PHY_READ(phy_dev, 0x1e, 0x4037, &val);

        if (val != CMD_IN_PROGRESS && val != CMD_SYSTEM_BUSY)
            return 0;

        udelay(2000);
    }

    printk("Timed out waiting for command ready");

Exit:
    return -1;
}

static uint16_t _wait_for_cmd_complete(phy_dev_t *phy_dev)
{ 
    int ret, i;
    uint16_t val = 0;

    for (i = 0; i < 1000; i++)
    {
        /* Read status of command */
        PHY_READ(phy_dev, 0x1e, 0x4037, &val);

        if (val == CMD_COMPLETE_PASS || val == CMD_COMPLETE_ERROR)
            goto Exit;

        udelay(2000);
    }

    printk("Timed out waiting for command complete\n");

Exit:
    return val;
}

static int cmd_handler(phy_dev_t *phy_dev, uint16_t cmd_code, uint16_t *data1, uint16_t *data2, uint16_t *data3, uint16_t *data4, uint16_t *data5)
{	 
    int ret = 0;
    uint16_t cmd_status = 0;

	/* Make sure command interface is open */
    if ((ret = _wait_for_cmd_ready(phy_dev)))
        goto Exit;

	switch (cmd_code)
    {
    case CMD_SET_PAIR_SWAP:
    case CMD_SET_1588_ENABLE:
    case CMD_SET_SHORT_REACH_MODE_ENABLE:
    case CMD_SET_EEE_MODE:		
    case CMD_SET_EMI_MODE_ENABLE:
    case CMD_SET_SERDES_KR_MODE_ENABLE:
    case CMD_CLEAR_SUB_LF_RF:
    case CMD_SET_SUB_LF_RF:
    case CMD_WRITE_INDIRECT_GPHY_REG_BITS:
    case CMD_SET_XFI_2P5G_MODE:
    case CMD_SET_2XGE_MODE:
    case CMD_SETUP_EEE_STATISTICS:
    case CMD_SET_JUMBO_PACKET:
    case CMD_SET_INTERLEAVER:
    case CMD_SET_XFI_TX_FILTERS:
    case CMD_SET_XFI_POLARITY:
    case CMD_SET_UPPER_TEMP_WARNING_LEVEL:
    case CMD_SET_LOWER_TEMP_WARNING_LEVEL:
    case CMD_SET_HW_FR_EMI_MODE_ENABLE:
    case CMD_SET_CUSTOMER_REQUESTED_TX_PWR_ADJUST:
    case CMD_SET_DYNAMIC_PARTITION_SELECT:
    {
        if (data1)
            PHY_WRITE(phy_dev, 0x1e, 0x4038, *data1);
        if (data2)
            PHY_WRITE(phy_dev, 0x1e, 0x4039, *data2);
        if (data3)
            PHY_WRITE(phy_dev, 0x1e, 0x403a, *data3);
        if (data4)
            PHY_WRITE(phy_dev, 0x1e, 0x403b, *data4);
        if (data5)
            PHY_WRITE(phy_dev, 0x1e, 0x403c, *data5);

        PHY_WRITE(phy_dev, 0x1e, 0x4005, cmd_code);
        cmd_status = _wait_for_cmd_complete(phy_dev);

        break;
    }
    case CMD_GET_PAIR_SWAP:
    case CMD_GET_1588_ENABLE:
    case CMD_GET_SHORT_REACH_MODE_ENABLE:
    case CMD_GET_EEE_MODE:		
    case CMD_GET_EMI_MODE_ENABLE:
    case CMD_GET_SERDES_KR_MODE_ENABLE:
    case CMD_GET_SUB_LF_RF_STATUS:
    case CMD_GET_XFI_2P5G_MODE:
    case CMD_GET_2XGE_MODE:
    case CMD_GET_JUMBO_PACKET:
    case CMD_GET_MSE:
    case CMD_GET_INTERLEAVER:
    case CMD_GET_XFI_TX_FILTERS:
    case CMD_GET_XFI_POLARITY:
    case CMD_GET_CURRENT_VOLTAGE:
    case CMD_GET_SNR:
    case CMD_GET_CURRENT_TEMP:
    case CMD_GET_UPPER_TEMP_WARNING_LEVEL:
    case CMD_GET_LOWER_TEMP_WARNING_LEVEL:
    case CMD_GET_HW_FR_EMI_MODE_ENABLE:
    case CMD_GET_CUSTOMER_REQUESTED_TX_PWR_ADJUST:
    case CMD_GET_DYNAMIC_PARTITION_SELECT:
    case CMD_RESET_STAT_LOG:
    {
        PHY_WRITE(phy_dev, 0x1e, 0x4005, cmd_code);
        cmd_status = _wait_for_cmd_complete(phy_dev);

        if (data1)
            PHY_READ(phy_dev, 0x1e, 0x4038, data1);
        if (data2)
            PHY_READ(phy_dev, 0x1e, 0x4039, data2);
        if (data3)
            PHY_READ(phy_dev, 0x1e, 0x403a, data3);
        if (data4)
            PHY_READ(phy_dev, 0x1e, 0x403b, data4);
        if (data5)
            PHY_READ(phy_dev, 0x1e, 0x403c, data5);

        break;
    }
    case CMD_READ_INDIRECT_GPHY_REG_BITS:
    {
        if (data1)
            PHY_WRITE(phy_dev, 0x1e, 0x4038, *data1);
        if (data2)
            PHY_WRITE(phy_dev, 0x1e, 0x4039, *data2);
        if (data3)
            PHY_WRITE(phy_dev, 0x1e, 0x403a, *data3);
        if (data4)
            PHY_WRITE(phy_dev, 0x1e, 0x403b, *data4);

        PHY_WRITE(phy_dev, 0x1e, 0x4005, cmd_code);
        cmd_status = _wait_for_cmd_complete(phy_dev);

        if (data5)
            PHY_READ(phy_dev, 0x1e, 0x403c, data5);

        break;
    }
    case CMD_GET_EEE_STATISTICS:
    {
        if (data1)
            PHY_WRITE(phy_dev, 0x1e, 0x4038, *data1);

        PHY_WRITE(phy_dev, 0x1e, 0x4005, cmd_code);
        cmd_status = _wait_for_cmd_complete(phy_dev);

        if (data2)
            PHY_READ(phy_dev, 0x1e, 0x4039, data2);
        if (data3)
            PHY_READ(phy_dev, 0x1e, 0x403a, data3);
        if (data4)
            PHY_READ(phy_dev, 0x1e, 0x403b, data4);
        if (data5)
            PHY_READ(phy_dev, 0x1e, 0x403c, data5);

        break;
    }
    default:
        printk("Unsupported cmd code: 0x%x\n", cmd_code);
        break;
    }

    if (cmd_status != CMD_COMPLETE_PASS)
    {
        printk("Failed to execute cmd code: 0x%x\n", cmd_code);
        return -1;
    }

Exit:
    return ret;
}

static int _phy_autoneg_restart(phy_dev_t *phy_dev)
{
    int ret;
    uint16_t val;

    /* Restart auto negotiation */
    PHY_READ(phy_dev, 0x07, 0xffe0, &val);

    val |= (1 << 12); /* Enable auto negotiation */
    val |= (1 << 9); /* Restart auto negotiation */

    PHY_WRITE(phy_dev, 0x07, 0xffe0, val);

Exit:
    return ret;
}

static int _phy_power_set(phy_dev_t *phy_dev, int enable)
{
    uint16_t val;
    int ret;

    PHY_READ(phy_dev, 0x07, 0xffe0, &val);

    if (enable)
        val &= ~(1 << 11); /* Power up */
    else
        val |= (1 << 11); /* Power down */

    PHY_WRITE(phy_dev, 0x07, 0xffe0, val);

Exit:
    return ret;
}

static int _phy_apd_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t data1, data2, data3, data4, data5;

    data1 = 0x1c;
    data2 = 0x0a;
    data3 = 0xffff;
    data4 = 0;
    data5 = 0;

    if ((ret = cmd_handler(phy_dev, CMD_READ_INDIRECT_GPHY_REG_BITS, &data1, &data2, &data3, &data4, &data5)))
        goto Exit;

	if (enable)
    {
        data5 |= (1 << 5); /* Auto power-down mode enabled */
        data5 |= (1 << 8); /* Enable energy detect single link pulse */
	}
    else
    {
        data5 &= ~(1 << 5); /* Auto power-down mode disabled */
        data5 &= ~(1 << 8); /* Disable energy detect single link pulse */
	}

    if ((ret = cmd_handler(phy_dev, CMD_WRITE_INDIRECT_GPHY_REG_BITS, &data1, &data2, &data3, &data4, &data5)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_eee_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    /* EEE Resolution Status */
    PHY_READ(phy_dev, 7, 0x803e, &val);

    /* Check if the link partner auto-negotiated EEE capability */
    *enable = (val & (1 << 1) || val & (1 << 2) || val & (1 << 3));

Exit:
    return ret;
}

static int _phy_eee_set(phy_dev_t *phy_dev, int enable)
{
    int ret = 0;
    uint16_t val = 0, adv, data1, data2, data3, data4;

    /* Advertise PHY as 100/1000/10000BASE-T EEE capable */
    PHY_READ(phy_dev, 0x07, 0x003c, &val);

    adv = (1 << 1) | (1 << 2) | (1 << 3);

    if (enable)
        val |= adv;
    else
        val &= ~adv;

    PHY_WRITE(phy_dev, 0x07, 0x003c, val);

    /* Enable/disable EEE */
    data1 = enable ? 0x0f : 0x0;
    data2 = 0;
    data3 = 2 * 15625;
    data4 = 0;

    cmd_handler(phy_dev, CMD_SET_EEE_MODE, &data1, &data2, &data3, &data4, NULL);

    /* Restart auto negotiation to kick off EEE setting */
    if ((ret = _phy_autoneg_restart(phy_dev)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_read_status(phy_dev_t *phy_dev)
{
    int ret, mode, i;
    uint16_t val;
    lport_port_status_s port_status;
    uint32_t port = (uint64_t)phy_dev->priv;

    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;

    if ((ret = lport_get_port_status(port, &port_status)))
        goto Exit;

    if (!port_status.port_up)
        goto Exit;

    for (i = 0; i < 200; i++)
    {
        /* Read the status register */
        PHY_READ(phy_dev, 0x1e, 0x400d, &val);

        /* Break if copper is not detected, or copper link is already up */
        if (!((val >> 1) & 0x1) || ((val >> 5) & 0x1))
            break;

        udelay(10000);
    }

    /* Copper link status */
    phy_dev->link = ((val >> 5) & 0x1);

    if (!phy_dev->link)
        goto Exit;

    /* Copper speed */
    mode = ((val >> 2) & 0x7);

    if (mode == 0)
        phy_dev->speed = PHY_SPEED_10;
    else if (mode == 1)
        phy_dev->speed = PHY_SPEED_2500;
    else if (mode == 2)
        phy_dev->speed = PHY_SPEED_100;
    else if (mode == 3)
        phy_dev->speed = PHY_SPEED_5000;
    else if (mode == 4)
        phy_dev->speed = PHY_SPEED_1000;
    else if (mode == 6)
        phy_dev->speed = PHY_SPEED_10000;

    phy_dev->duplex = (port_status.duplex == LPORT_FULL_DUPLEX) ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;

Exit:
    return ret;
}

static int _phy_set_mode(phy_dev_t *phy_dev, int line_side)
{
    int ret;
    uint16_t val1, val2, val3;

    val1 = line_side ? 0x0001 : 0x2004;
    val2 = line_side ? 0x0001 : 0x2004;
    val3 = line_side ? 0x1002 : 0x2004;

    /* Set base-pointer mode */
    PHY_WRITE(phy_dev, 0x1e, 0x4110, val1);
    PHY_WRITE(phy_dev, 0x1e, 0x4111, val2);
    PHY_WRITE(phy_dev, 0x1e, 0x4113, val3);

Exit:
    return ret;
}

static int _phy_init(phy_dev_t *phy_dev)
{
    int ret, i;
    uint16_t val, bits;

    /* Reset the phy */
    PHY_WRITE(phy_dev, 0x01, 0x0000, 0x8000);

    /* Verify that the processor is running */
    i = 1000;
    do 
    {
        udelay(2000);
        PHY_READ(phy_dev, 0x01, 0x0000, &val);
        ret = (val != 0x2040);
    } while (ret && i--);

    if (ret)
        goto Exit;

    /* Set base-pointer mode to line side to access PMD/PCS/AN for BASE-T */
    if ((ret = _phy_set_mode(phy_dev, 1)))
        goto Exit;

    /* 100M Advertisement */
    PHY_READ(phy_dev, 0x07, 0xffe4, &val);

    bits = 0;
    bits |= (1 << 7); /* Advertise 100BASE-T half-duplex capability */
    bits |= (1 << 8); /* Advertise 100BASE-T full-duplex capability */

    if (phy_dev->mii_type == PHY_MII_TYPE_SGMII)
        val |= bits;
    else
        val &= ~bits;

    PHY_WRITE(phy_dev, 0x07, 0xffe4, val);

    /* 1G Advertisement */
    PHY_READ(phy_dev, 0x07, 0xffe9, &val);

    bits = 0;
    bits |= (1 << 8); /* Advertise 1000BASE-T half-duplex capability */
    bits |= (1 << 9); /* Advertise 1000BASE-T full-duplex capability */
    bits |= (1 << 10); /* Advertise Repeater mode */

    if (phy_dev->mii_type == PHY_MII_TYPE_SGMII)
        val |= bits;
    else
        val &= ~bits;

    PHY_WRITE(phy_dev, 0x07, 0xffe9, val);

    /* 2.5G Advertisement */
    PHY_READ(phy_dev, 0x1e, 0x0000, &val);

    bits = 0;
    bits |= (1 << 1); /* Advertise PHY as 2.5GBASE-T capable */

    if (phy_dev->mii_type == PHY_MII_TYPE_HSGMII)
        val |= bits;
    else
        val &= ~bits;

    val |= (1 << 0); /* Enable MGBASE-T mode (regardless of speed) */
    PHY_WRITE(phy_dev, 0x1e, 0x0000, val);

    /* 10G Advertisement */
    PHY_READ(phy_dev, 0x07, 0x0020, &val);

    bits = 0;
    bits |= (1 << 12); /* Advertise PHY as 10GBASE-T capable */

    if (phy_dev->mii_type == PHY_MII_TYPE_XFI)
        val |= bits;
    else
        val &= ~bits;

    PHY_WRITE(phy_dev, 0x07, 0x0020, val);

    /* Restart auto negotiation */
    if ((ret = _phy_autoneg_restart(phy_dev)))
        goto Exit;

Exit:
    return ret;
}

static int inline _bus_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return phy_drv_ext3.bus_drv->c45_read(addr, dev, reg, val);
}

static int inline _bus_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return phy_drv_ext3.bus_drv->c45_write(addr, dev, reg, val);
}

static int _bus_read_all(uint32_t phy_map, uint16_t dev, uint16_t reg, uint16_t val, uint32_t mask)
{
    int ret = 0;
    uint32_t i;
    uint16_t _val;

    for (i = 0; i < 32; i++)
    {
        if (!(phy_map & (1 << i)))
            continue;

        if ((ret = _bus_read(i + 1, dev, reg, &_val)))
            return ret;

        if ((_val & mask) != val)
            return -1;
    }

    return 0;
}

static int _bus_write_all(uint32_t phy_map, uint16_t dev, uint16_t reg, uint16_t val)
{
    int ret = 0;
    uint32_t i;

    for (i = 0; i < 32; i++)
    {
        if (!(phy_map & (1 << i)))
            continue;

        if ((ret = _bus_write(i + 1, dev, reg, val)))
            return ret;
    }

    return 0;
}

static int _phy_cfg(uint32_t phy_map)
{
    int i, cnt, step, ret;

    if (!phy_map)
        return 0;

    cnt = bcm8486x_image_size / sizeof(unsigned int);
    step = cnt / 100;

    printk("BCM8486x phy driver initialization\n");

    /* Download firmware with broadcast mode to up to 32 phys, according to the phy map */
    printk("Loading firmware into phys: map=0x%x\n", phy_map);

    /* 1. Turn on broadcast mode to accept write operations for addr = 0 */
    printk("Turn on broadcast mode to accept write operations\n");

    BUS_WRITE_ALL(phy_map, 0x1e, 0x4117, 0xf003);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x4107, 0x0401);

    /* 2. Halt the BCM8486X processors operation */
    printk("Halt the phys processors operation\n");

    BUS_WRITE_ALL(phy_map, 0x1e, 0x4188, 0x48f0);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x4186, 0x8000);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x4181, 0x017c);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x4181, 0x0040);

    BUS_WRITE(0, 0x01, 0xa819, 0x0000);
    BUS_WRITE(0, 0x01, 0xa81a, 0xc300);
    BUS_WRITE(0, 0x01, 0xa81b, 0x0010);
    BUS_WRITE(0, 0x01, 0xa81c, 0x0000);
    BUS_WRITE(0, 0x01, 0xa817, 0x0009);
    BUS_WRITE(0, 0x01, 0xa819, 0x0000);
    BUS_WRITE(0, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(0, 0x01, 0xa81b, 0x1018);
    BUS_WRITE(0, 0x01, 0xa81c, 0xe59f);
    BUS_WRITE(0, 0x01, 0xa817, 0x0009);
    BUS_WRITE(0, 0x01, 0xa819, 0x0004);
    BUS_WRITE(0, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(0, 0x01, 0xa81b, 0x1f11);
    BUS_WRITE(0, 0x01, 0xa81c, 0xee09);
    BUS_WRITE(0, 0x01, 0xa817, 0x0009);
    BUS_WRITE(0, 0x01, 0xa819, 0x0008);
    BUS_WRITE(0, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(0, 0x01, 0xa81b, 0x0000);
    BUS_WRITE(0, 0x01, 0xa81c, 0xe3a0);
    BUS_WRITE(0, 0x01, 0xa817, 0x0009);
    BUS_WRITE(0, 0x01, 0xa819, 0x000c);

    BUS_WRITE(0, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(0, 0x01, 0xa81b, 0x1806);
    BUS_WRITE(0, 0x01, 0xa81c, 0xe3a0);
    BUS_WRITE(0, 0x01, 0xa817, 0x0009);
    BUS_WRITE(0, 0x01, 0xa819, 0x0010);
    BUS_WRITE(0, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(0, 0x01, 0xa81b, 0x0002);
    BUS_WRITE(0, 0x01, 0xa81c, 0xe8a0);
    BUS_WRITE(0, 0x01, 0xa817, 0x0009);
    BUS_WRITE(0, 0x01, 0xa819, 0x0014);
    BUS_WRITE(0, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(0, 0x01, 0xa81b, 0x0001);
    BUS_WRITE(0, 0x01, 0xa81c, 0xe150);
    BUS_WRITE(0, 0x01, 0xa817, 0x0009);
    BUS_WRITE(0, 0x01, 0xa819, 0x0018);
    BUS_WRITE(0, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(0, 0x01, 0xa81b, 0xfffc);
    BUS_WRITE(0, 0x01, 0xa81c, 0x3aff);
    BUS_WRITE(0, 0x01, 0xa817, 0x0009);
    BUS_WRITE(0, 0x01, 0xa819, 0x001c);
    BUS_WRITE(0, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(0, 0x01, 0xa81b, 0xfffe);
    BUS_WRITE(0, 0x01, 0xa81c, 0xeaff);
    BUS_WRITE(0, 0x01, 0xa817, 0x0009);
    BUS_WRITE(0, 0x01, 0xa819, 0x0020);
    BUS_WRITE(0, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(0, 0x01, 0xa81b, 0x0021);
    BUS_WRITE(0, 0x01, 0xa81c, 0x0004);
    BUS_WRITE(0, 0x01, 0xa817, 0x0009);

    BUS_WRITE_ALL(phy_map, 0x1e, 0x4181, 0x0000);

    /* 3. Upload the firmware into the on-chip memory of the devices */
    printk("Upload the firmware into the on-chip memory\n");

    BUS_WRITE(0, 0x01, 0xa81a, 0x0000);
    BUS_WRITE(0, 0x01, 0xa819, 0x0000);
    BUS_WRITE(0, 0x01, 0xa817, 0x0038);

    for (i = 0; i < cnt; i++)
    {
        BUS_WRITE(0, 0x01, 0xA81C, bcm8486x_image[i] >> 16); /* upper 16 bits */
        BUS_WRITE(0, 0x01, 0xA81B, bcm8486x_image[i] & 0xffff); /* lower 16 bits */

        if (i == i / step * step)
            printk("\r%d%%", i / step);
    }
    printk("\n");

    BUS_WRITE(0, 0x01, 0xa817, 0x0000);
    BUS_WRITE(0, 0x01, 0xa819, 0x0000);
    BUS_WRITE(0, 0x01, 0xa81a, 0xc300);
    BUS_WRITE(0, 0x01, 0xa81b, 0x0000);
    BUS_WRITE(0, 0x01, 0xa81c, 0x0000);
    BUS_WRITE(0, 0x01, 0xa817, 0x0009);

    /* 4. Reset the processors to start execution of the code in the on-chip memory */
    printk("Reset the processors to start execution of the code in the on-chip memory\n");

    BUS_WRITE(0, 0x01, 0xa008, 0x0000);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x8004, 0x5555);
    BUS_WRITE(0, 0x01, 0x0000, 0x8000);

    /* 5. Verify that the processors are running */
    printk("Verify that the processors are running: ");

    i = 1000;
    do 
    {
        udelay(2000);
        ret  = _bus_read_all(phy_map, 0x01, 0x0000, 0x2040, 0xffff);
    } while (ret && i--);

    printk("%s\n", ret ? "Failed" : "OK");

    if (ret)
        goto Exit;

    /* 6. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
    printk("Verify that the firmware has been loaded with good CRC: ");

    i = 1000;
    do 
    {
        udelay(2000);
        ret  = _bus_read_all(phy_map, 0x1e, 0x400d, 0x4000, 0xc000);
    } while (ret && i--);

    printk("%s\n", ret ? "Failed" : "OK");

    if (ret)
        goto Exit;

    printk("Firmware loading completed successfully\n");

Exit:
    return ret;
}

static int _phy_dev_add(phy_dev_t *phy_dev)
{
    enabled_phys |= (1 << (phy_dev->addr - 1));

    return 0;
}

static int _phy_dev_del(phy_dev_t *phy_dev)
{
    enabled_phys &= ~(1 << (phy_dev->addr - 1));

    return 0;
}


static int _phy_drv_init(phy_drv_t *phy_drv)
{
    if (_phy_cfg(enabled_phys))
    {
        printk("Failed to initialize the 8486x driver\n");
        return -1;
    }

    phy_drv->initialized = 1;

    return 0;
}

phy_drv_t phy_drv_ext3 =
{
    .phy_type = PHY_TYPE_EXT3,
    .name = "EXT3",
    .power_set = _phy_power_set,
    .apd_set = _phy_apd_set,
    .eee_get = _phy_eee_get,
    .eee_set = _phy_eee_set,
    .read_status = _phy_read_status,
    .init = _phy_init,
    .dev_add = _phy_dev_add,
    .dev_del = _phy_dev_del,
    .drv_init = _phy_drv_init,
};
