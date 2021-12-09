/*
   <:copyright-BRCM:2016:DUAL/GPL:standard

      Copyright (c) 2016 Broadcom
      All Rights Reserved

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
 * PHY driver for BCM53125-compatible switches
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_brcm.h"
#include <board.h>

#define _EXT_SWITCH_INIT_
#include "bcm_map_part.h"
#include "mii_shared.h"
#include "bcm_gpio.h"

#ifdef EXT_BCM53134
void pseudo_phy_read(phy_dev_t *phy_dev, uint32_t page, uint32_t reg, uint8_t *data, int len)
{
    bus_drv_t *bus_drv =phy_dev->phy_drv->bus_drv;
    uint16_t val16;
    uint16_t res = 0;
    int retries = 10;

    if (!data || len > 8) {
        printk("%s invalid input\n", __FUNCTION__);
        return;
    }

    val16 = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, val16);

    val16 = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_READ;
    bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, val16);

    do {
        bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &res);
        udelay(10);
    } while (((res & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_WRITE))
              != REG_PPM_REG17_OP_DONE) && (retries-- >= 0));

    if (retries < 0) {
        printk("Error reading pseudo phy switch register 0x%0x, page 0x%0x\n", reg, page);
        return;
    }

    switch (len) {
         case 8:
            bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG27, &val16);
            data[7] = (val16 >> 8);
            data[6] = (val16 & 0xff);
         case 6:
            bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &val16);
            data[5] = (val16 >> 8);
            data[4] = (val16 & 0xff);
            bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &val16);
            data[3] = (val16 >> 8);
            data[2] = (val16 & 0xff);
            bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &val16);
            data[1] = (val16 >> 8);
            data[0] = (val16 & 0xff);
            break;
         case 4:
            bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &val16);
            *(uint32 *)data = (((uint32)val16) << 16);
            bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &val16);
            *(uint32 *)data |= val16;
            break;
        case 2:
            bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &val16);
            ((uint16 *)data)[0] = (uint16)val16;
            break;
        case 1:
            bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &val16);
            data[0] = (uint8)val16;
            break;
    }

#if 0
    {
        int tt;
        printk("pseudo phy read\n");
        for(tt=0;tt<len;tt++) {
            printk("data[%d] 0x%02x\n", tt, data[tt]);
        }
    
        printk("\n");
    }
#endif
}
EXPORT_SYMBOL(pseudo_phy_read);


void pseudo_phy_write(phy_dev_t *phy_dev, uint32_t page, uint32_t reg, uint8_t *data, int len)
{
    bus_drv_t *bus_drv = phy_dev->phy_drv->bus_drv;
    uint16_t val16;
    uint16_t res = 0;
    int retries = 10;

    if (!data || len > 8) {
        printk("%s invalid input\n", __FUNCTION__);
	return;
    }

#if 0
    {
        int tt;
        printk("pseudo phy write\n");
        for(tt=0;tt<len;tt++) {
            printk("data[%d] 0x%02x\n", tt, data[tt]);
        }
    
        printk("\n");
    }
#endif

    val16 = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, val16);

    switch (len) {
        case 8:
            val16 = data[7];
            val16 = ((val16 << 8) | data[6]);
            bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG27, val16);
        case 6:
            val16 = data[5];
            val16 = ((val16 << 8) | data[4]);
            bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, val16);
            val16 = data[3];
            val16 = ((val16 << 8) | data[2]);
            bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, val16);
            val16 = data[1];
            val16 = ((val16 << 8) | data[0]);
            bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, val16);
            break;
        case 4:
            val16 = (uint16)((*(uint32 *)data) >> 16);	
            bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, val16);
            val16  = (uint16)(*(uint32 *)data);
            bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, val16);
            break;
        case 2:
            val16 = *(uint16 *)data;
            bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, val16);
            break;
        case 1:
            val16 = data[0];
            bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, val16);
            break;
    }

    val16 = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_WRITE;
    bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, val16);

    do {
        bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &res);
        udelay(10);
    } while (((res & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_WRITE))
              != REG_PPM_REG17_OP_DONE) && (retries-- >= 0));

    if (retries < 0)
        printk("Error reading pseudo phy switch register 0x%0x, page 0x%0x\n", reg, page);
}
EXPORT_SYMBOL(pseudo_phy_write);

#else
static void pseudo_phy_read(phy_dev_t *phy_dev, uint32_t page, uint32_t reg, uint32_t *data)
{
    bus_drv_t *bus_drv =phy_dev->phy_drv->bus_drv;
    uint32_t cmd;
    uint16_t res = 0;
    uint16_t ret = 0;
    int retries = 5;

    *data = 0;
    cmd = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, cmd);

    cmd = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_READ;
    bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, cmd);

    do {
        bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &res);
        udelay(10);
    } while (((res & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_WRITE))
              != REG_PPM_REG17_OP_DONE) && (retries-- >= 0));

    if (retries < 0) {
        printk("Error reading pseudo phy switch register 0x%0x, page 0x%0x\n", reg, page);
        return;
    }

    bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &ret);
    *data |= ret;
    bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &ret);
    *data |= (uint32_t)ret << 16;
}


static void pseudo_phy_write(phy_dev_t *phy_dev, uint32_t page, uint32_t reg, uint32_t data)
{
    bus_drv_t *bus_drv = phy_dev->phy_drv->bus_drv;
    uint16_t cmd;
    uint16_t res = 0;
    int retries = 5;

    cmd = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, cmd);

    cmd = data & 0xffff;
    bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, cmd);
    cmd = (data >> 16) & 0xffff;
    bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, cmd);

    cmd = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_WRITE;
    bus_drv->c22_write(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, cmd);

    do {
        bus_drv->c22_read(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &res);
        udelay(10);
    } while (((res & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_WRITE))
              != REG_PPM_REG17_OP_DONE) && (retries-- >= 0));

    if (retries < 0)
        printk("Error reading pseudo phy switch register 0x%0x, page 0x%0x\n", reg, page);
}
#endif

/******************************************************************************/
#ifdef EXT_BCM53134
#define BCM53134S 0x5075
#define BCM53134_PAGE_EEE 0x92
  #define BCM53134_REG_EEE_EN_CTRL 0x0

/* PAGE_CONTROL */
  #define BCM53134_REG_IMP_RGMII_DELAY 0x60
    #define RGMII_TXCLK_DELAY   (1 << 0)
    #define RGMII_RXCLK_DELAY   (1 << 1)
  #define BCM53134_REG_SW_CTRL 0x22
    #define MII_DUMB_FWDG_EN    (1 << 6)
static void bcm53134_init(phy_dev_t *phy_dev)
{
    uint8_t data8;
    uint16_t data16;

    printk("%s: reset gpio 0x%x\n", __FUNCTION__, phy_dev->resetGpio);
    /* reset external switch */
    if (phy_dev->resetGpio != BP_GPIO_NONE) {
        /* set gpio dir to output */
        bcm_gpio_set_dir(phy_dev->resetGpio, 1);

        /* hold reset */
        if (phy_dev->resetGpio & BP_ACTIVE_LOW)
            bcm_gpio_set_data((phy_dev->resetGpio & BP_GPIO_NUM_MASK), 0);
        else
            bcm_gpio_set_data((phy_dev->resetGpio & BP_GPIO_NUM_MASK), 1);

	udelay(10000);

        /* release reset */
        if (phy_dev->resetGpio & BP_ACTIVE_LOW)
            bcm_gpio_set_data((phy_dev->resetGpio & BP_GPIO_NUM_MASK), 1);
        else
            bcm_gpio_set_data((phy_dev->resetGpio & BP_GPIO_NUM_MASK), 0);
        udelay(5000);
    }

    /* disable EEE */
    data16 = 0;
    pseudo_phy_write(phy_dev, BCM53134_PAGE_EEE, BCM53134_REG_EEE_EN_CTRL,
        (uint8 *)&data16, sizeof(data16));

    /* disable IMP Port */
    data8 = 0;
    pseudo_phy_write(phy_dev, PAGE_MANAGEMENT, REG_GLOBAL_CONFIG,
        (uint8 *)&data8, sizeof(data8));

    /* unmanged mode, enable forwarding */
    pseudo_phy_read(phy_dev, PAGE_CONTROL, BCM53134_REG_SW_CTRL,
        (uint8 *)&data16, sizeof(data16));
    data16 |= MII_DUMB_FWDG_EN;
    pseudo_phy_write(phy_dev, PAGE_CONTROL, BCM53134_REG_SW_CTRL,
        (uint8 *)&data16, sizeof(data16));
 
    pseudo_phy_read(phy_dev, PAGE_CONTROL, REG_SWITCH_MODE,
        (uint8 *)&data8, sizeof(data8));
    data8 &= ~REG_SWITCH_MODE_FRAME_MANAGE_MODE;
    data8 |= REG_SWITCH_MODE_SW_FWDG_EN;
    pseudo_phy_write(phy_dev, PAGE_CONTROL, REG_SWITCH_MODE,
        (uint8 *)&data8, sizeof(data8));

    /* RGMII delay clk */
    pseudo_phy_read(phy_dev, PAGE_CONTROL, BCM53134_REG_IMP_RGMII_DELAY,
        (uint8 *)&data8, sizeof(data8));
    data8 |= (RGMII_TXCLK_DELAY | RGMII_RXCLK_DELAY);
    pseudo_phy_write(phy_dev, PAGE_CONTROL, BCM53134_REG_IMP_RGMII_DELAY,
        (uint8 *)&data8, sizeof(data8));
}
#endif

static int _phy_init(phy_dev_t *phy_dev)
{
#ifdef EXT_BCM53134
    uint32_t data32;

    pseudo_phy_read(phy_dev, PAGE_MANAGEMENT, REG_DEVICE_ID,
        (uint8 *)&data32, sizeof(data32));

    printk("_phy_init external switch devid is 0x%x\n", data32);

    if (data32 == BCM53134S) {
    	bcm53134_init(phy_dev);
        return 0;
    }
#else
    uint32_t data;
    int i;

    pseudo_phy_read(phy_dev, PAGE_MANAGEMENT, REG_DEVICE_ID, &data);
#endif

#ifndef EXT_BCM53134
    if (data == 0x53125) {
        /* Setup switch MII1 port state override */
        data = (REG_CONTROL_MPSO_MII_SW_OVERRIDE
                | REG_CONTROL_MPSO_SPEED1000
                | REG_CONTROL_MPSO_FDX
                | REG_CONTROL_MPSO_LINKPASS
                | REG_CONTROL_MPSO_TX_FLOW_CONTROL
                | REG_CONTROL_MPSO_RX_FLOW_CONTROL);
        pseudo_phy_write(phy_dev, PAGE_CONTROL, REG_CONTROL_MII1_PORT_STATE_OVERRIDE, data);

        /* Unmanaged mode, enable forwarding */
        pseudo_phy_read(phy_dev, PAGE_CONTROL, REG_SWITCH_MODE, &data);
        data &= ~REG_SWITCH_MODE_FRAME_MANAGE_MODE;
        data |= REG_SWITCH_MODE_SW_FWDG_EN;
        pseudo_phy_write(phy_dev, PAGE_CONTROL, REG_SWITCH_MODE, data);

        /* Enable IMP port */
        data = ENABLE_MII_PORT;
        pseudo_phy_write(phy_dev, PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, data);

        /* Disable BRCM tag for IMP */
        data = 0;
        pseudo_phy_write(phy_dev, PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL, data);

        /* Enable RX broadcast, unicast and multicast of IMP port */
        data = (REG_MII_PORT_CONTROL_RX_UCST_EN
                | REG_MII_PORT_CONTROL_RX_MCST_EN
                | REG_MII_PORT_CONTROL_RX_BCST_EN);
        pseudo_phy_write(phy_dev, PAGE_CONTROL, REG_MII_PORT_CONTROL, data);

        /* Enable APD compatibility bit on all ports */
        for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
            /* TODO */
        }

        /* No spanning tree for unmanaged mode */
        data = 0;
        for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
            /* TODO: Complete */
            pseudo_phy_write(phy_dev, PAGE_CONTROL, i, data);
        }

        /* No spanning tree on IMP port either */
        pseudo_phy_write(phy_dev, PAGE_CONTROL, REG_MII_PORT_CONTROL, data);
    }
#endif
    return 0;
}

static int _phy_dev_add(phy_dev_t *phy_dev)
{
    return 0;
}

static int _phy_dev_del(phy_dev_t *phy_dev)
{
    return 0;
}

static int _phy_drv_init(phy_drv_t *phy_drv)
{
    return 0;
}

static int _phy_read_status(phy_dev_t *phy_dev)
{
    /* Don't poll the switch gphys, assume link up and Gigabit Eth */
    phy_dev->link = 1;
    phy_dev->speed = PHY_SPEED_1000;
    phy_dev->duplex = PHY_DUPLEX_FULL;

    return 0;
}



phy_drv_t phy_drv_53125_sw =
{
    .phy_type = PHY_TYPE_53125,
    .name = "BCM External switch",
    .read_status = _phy_read_status,
    .init = _phy_init,
    .dev_add = _phy_dev_add,
    .dev_del = _phy_dev_del,
    .drv_init = _phy_drv_init,
};
