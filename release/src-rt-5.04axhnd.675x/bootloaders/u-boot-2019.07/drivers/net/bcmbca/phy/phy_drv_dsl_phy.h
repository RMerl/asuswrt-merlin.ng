// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

*/

/*
 *  Created on: Dec 2016
 *      Author: steven.hsieh@broadcom.com
 */

#ifndef __PHY_DRV_DSL_DSL_H__
#define __PHY_DRV_DSL_DSL_H__

#ifndef __UBOOT__
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include "linux/mutex.h"
#include "bcmsfp_i2c.h"
#endif
#include "bus_drv.h"
#include "phy_drv_mii.h"
#include "phy_drv_brcm.h"
//#include "bcm_map_part.h"
//#include "bcmnet.h"
//#include "board.h"
//#include "bcm_otp.h"
//#include "bcm_OS_Deps.h"
#include "bcmmii.h"
#include "bcmmii_xtn.h"
//#include "opticaldet.h"
#include "phy_drv.h"
//#include "pmc_wan.h"
//#include "bcm_physical_map_part.h"
//#include "phy_drv_crossbar.h"
//#include "crossbar_dev_plat.h"

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;
typedef unsigned long long  uint64;
typedef signed char     int8;
typedef signed short    int16;
typedef signed int     int32;
typedef signed long long    int64;

#define IsPortConnectedToExternalSwitch(id) (((id) & EXTSW_CONNECTED)?1:0)
#define BUG()

#include <linux/delay.h>            // translate ndelay() to udelay()
#define msleep(a) udelay(a * 1000)

// from bcmnet.h
enum {
    SERDES_NO_POWER_SAVING, 
    SERDES_BASIC_POWER_SAVING, 
    SERDES_ADVANCED_POWER_SAVING, 
    SERDES_FORCE_OFF,
    SERDES_POWER_MODE_MAX};

#define REG_WRITE_32(reg, val) do{*(volatile uint32_t *)(reg) = (val);} while(0)
#define REG_READ_32(reg, var) do{(var) = *(volatile uint32_t *)(reg);} while(0)
#define REG_DIR_WRITE_32(reg, val) REG_WRITE_32((reg), val)
#define REG_DIR_READ_32(reg, var) REG_READ_32((reg), var)

#define ErrClr "\e[0;93;41m"
#define WrnClr "\e[0;30;103m"
#define NtcClr "\e[0;30;102m"
#define DflClr "\e[0m"
#define BUG_CHECK(fmt, args...) do { \
    printk(ErrClr "*** Fatal Error in %s %s L%d: " fmt DflClr, __FILE__, __func__, __LINE__, ##args); \
    BUG(); printk(DflClr); \
} while(0)
#define WARN_CHECK(fmt, args...) do { \
    printk(WrnClr "*** Serious Warning in %s %s L%d: " fmt, __FILE__, __func__, __LINE__, ##args); \
    WARN_ON(1); printk(DflClr); \
} while(0)
#define printkwarn(fmt, args...) printk(WrnClr fmt DflClr "\n", ##args)
#define printknotice(fmt, args...) do {printk(NtcClr fmt DflClr "\n", ##args);} while(0)

#define phy_dsl_priv_m struct { \
    dt_handle_t handle; \
    phy_dev_t *phy_dev; \
    phy_speed_t config_speed; \
    phy_speed_t current_speed; \
    phy_speed_t hw_speed; \
    uint32_t speed_caps; \
    uint32_t common_speed_caps; \
    uint32_t inter_phy_types; \
    int link_changed;   /* Flag to create a link down for Kernel to notify speed change */ \
}

#define IS_USER_CONFIG()    0  //(!strstr(current->comm, "kworker"))
typedef struct phy_dsl_priv_s {
    phy_dsl_priv_m;
} phy_dsl_priv_t;

int dsl_phy_reset(phy_dev_t *phy_dev);

enum {
    PHY_OP_RD_MDIO,
    PHY_OP_WR_MDIO,
};

int dsl_phy_exp_op(phy_dev_t *phy_dev, int op, va_list ap);
#endif //__PHY_DRV_DSL_SERDES_H__
