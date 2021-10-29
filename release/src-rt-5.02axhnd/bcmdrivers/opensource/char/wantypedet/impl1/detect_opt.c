/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#include <linux/delay.h>
#include "board.h"
#include "rdpa_types.h"
#include "wan_drv.h"
#include "phy_drv.h"
#include "opticaldet.h"


#if defined(CONFIG_BCM_PON_WAN_TYPE_AUTO_DETECT)
extern int try_wan_type_sensing(void);
#endif


#ifdef CONFIG_BCM96838
#include <mdio_drv_impl1.h>

typedef enum {
    VpbLifR = 0,
    VpbEpnR,
    VpbNcoR,
    VpbClkDivR
} VpbResetModule_e;

typedef uint8_t VpbResetModule;

#define EPONREG_BASE (uint32_t)(0x130fc000 | 0xA0000000)
#define TkOnuAddr(off) (EPONREG_BASE + ((off) << 2))
#define TkOnuReg(off) (*(volatile uint32_t *)TkOnuAddr(off))
#define VpbCfg TkOnuReg(0x0000)
#define LifSecCtl TkOnuReg(0x0103)
#define LifPonCtl TkOnuReg(0x0100)
#define LifPonCtlNotRxRst 0x00000004UL
#define LifSecCtlNotDnRst 0x00000004UL
#define LifPonCtlRxEn 0x00000001UL
#define LifPonCtlCfgRxDataBitFlip 0x00100000UL
#define LifPonCtlCfOneGigPonDn 0x20000000UL

#define SD_POLLFREQ 100        /* in ms */
#define SIGNAL_DETECTED 1
#define SOACKING_CYCLES 3

static void OnuRegAndEqNot(volatile uint32_t* addr, uint32_t val)
{
    *addr &= (~val);
}

static void OnuRegOrEq(volatile uint32_t* addr, uint32_t val)
{
    *addr |= val;
}

static void VpbReset(VpbResetModule id, int en)
{
    if (en)
        OnuRegAndEqNot(&VpbCfg, 1Ul<<id);    	
    else
        OnuRegOrEq(&VpbCfg, 1Ul<<id);
}

static void OnuRegWrite(volatile uint32_t* addr, uint32_t val)
{
    *(addr) = (val);
}

static void LifRxEnable(void)
{
    OnuRegOrEq(&LifPonCtl, LifPonCtlNotRxRst);
    OnuRegOrEq(&LifSecCtl, LifSecCtlNotDnRst);
}

static int is_epon(serdes_wan_type_t wan_type)
{
    uint32_t regVal, sync;

    wan_serdes_config(wan_type);

    *(uint32_t *)0xb30fc410 = 0x7; /* Clear int. register */

    //release lif  
    VpbReset(VpbLifR, 0);

    OnuRegWrite(&LifPonCtl, 0);

    if (wan_type == SERDES_WAN_TYPE_EPON_1G)
        regVal = LifPonCtlRxEn | LifPonCtlCfgRxDataBitFlip | LifPonCtlCfOneGigPonDn;
    else
        regVal = LifPonCtlRxEn | LifPonCtlCfgRxDataBitFlip;

    // Clear reset
    OnuRegWrite(&LifPonCtl, regVal);

    // Security Controls
    LifRxEnable();

    msleep(3000);
    sync = (*(uint32_t *)0xb30fc410 & 0x2) == 0x2;
    pr_debug("lif: %d %d gbps\n", sync, wan_type == SERDES_WAN_TYPE_EPON_1G ? 1 : 2);
    if (!sync)
        return 0;

    sync = *(uint32_t *)0xb30fc544 > 0;
    pr_debug("epon: %d 0x%x \n", sync, *(uint32_t *)0xb30fc544);

    return sync;
}

static int is_gpon(void)
{
/* CDR expected range for GPON link */
#define THRES_MIN 0x90
#define THRES_MAX 0x100
    uint16_t mdio_data;
    
    wan_serdes_config(SERDES_WAN_TYPE_GPON);

    mdio_data = 0x81c0; /* PMDrx */
    mdio_write_c22_register(MDIO_SATA, 0x1, 0x1f, mdio_data);
    msleep(1000);

    mdio_read_c22_register(MDIO_SATA, 0x1, 0x1a, &mdio_data);

    pr_debug("gpon sync %x\n", mdio_data);

    return mdio_data > THRES_MIN && mdio_data < THRES_MAX;
}

#define PCS_MDIO_PHYID 2
static int is_ae(void)
{
    int ret = 0;
    phy_dev_t *phy_dev;

    wan_serdes_config(SERDES_WAN_TYPE_AE);

    if ((phy_dev = phy_dev_get(PHY_TYPE_PCS, PCS_MDIO_PHYID)))
    {
        phy_dev_init(phy_dev);
        msleep(2000);
        phy_dev_read_status(phy_dev);
        ret = phy_dev->link;
    }

    pr_debug("AE link status %x\n", ret);

    return ret;
}

int signalDetect(void)
{
    int ret, value, soack=0;
    unsigned short sd_gpio=0;

    ret = BpGetWanSignalDetectedGpio(&sd_gpio);

    if (ret != BP_SUCCESS || sd_gpio == BP_GPIO_NONE)
    {
        printk("\nERROR: signalDetect: GPIO not set for SD\n");
    }
    else
    {
        while (1)
        {
            value = kerSysGetGpioValue(sd_gpio);

            if (value == SIGNAL_DETECTED)
            {
                if (++soack == SOACKING_CYCLES)
                {
                    break;
                }
            }
            else
            {
                soack = 0;
            }

            msleep(SD_POLLFREQ);
        }

        printk("\nsignalDetect: Signal Detected\n");
    }

    return ret;
}
#endif


int opticaldetect(void)
{
#if defined(CONFIG_BCM96838)
    if (is_epon(SERDES_WAN_TYPE_EPON_1G))
        return rdpa_wan_epon;

    if (is_epon(SERDES_WAN_TYPE_EPON_2G))
        return rdpa_wan_epon | EPON2G;

    if (is_gpon())
        return rdpa_wan_gpon;

    if (is_ae())
        return rdpa_wan_gbe;
#elif defined(CONFIG_BCM_PON_WAN_TYPE_AUTO_DETECT)
    return try_wan_type_sensing();
#endif

    return rdpa_wan_none;
}
