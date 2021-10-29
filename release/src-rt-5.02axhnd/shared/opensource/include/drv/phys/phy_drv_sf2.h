/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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

/*
 *  Created on: Dec 2016
 *      Author: steven.hsieh@broadcom.com
 */

#ifndef __PHY_DRV_SF2_H__
#define __PHY_DRV_SF2_H__

#define REG_WRITE_32(reg, val) do{*(volatile uint32_t *)(reg) = (val);} while(0)
#define REG_READ_32(reg, var) do{(var) = *(volatile uint32_t *)(reg);} while(0)
#define REG_DIR_WRITE_32(reg, val) REG_WRITE_32(bcm_dev_phy2vir(reg), val)
#define REG_DIR_READ_32(reg, var) REG_READ_32(bcm_dev_phy2vir(reg), var)

#include "phy_drv.h"
#include "bcm_map_part.h"
#if !(defined(_BCM94908_) && defined(CFG_2P5G_LAN))
#include "phy_drv_xgae.h"
#endif /* ! (_BCM94908_ && CFG_2P5G_LAN) */

#if defined(_BCM94908_) && defined(CFG_2P5G_LAN)
#ifndef s8
typedef signed char s8;
#endif /* ! s8 */

#ifndef u8
typedef unsigned char u8;
#endif /* ! u8 */

#ifndef s16
typedef signed short s16;
#endif /* ! s16 */

#ifndef u16
typedef unsigned short u16;
#endif /* ! u16 */

#ifndef s32
typedef signed int s32;
#endif /* ! s32 */

#ifndef u32
typedef unsigned int u32;
#endif /* ! u32 */

#ifndef s64
typedef signed long s64;
#endif /* ! s64 */

#ifndef u64
typedef unsigned long u64;
#endif /* ! u64 */
#endif /* _BCM94908_ && CFG_2P5G_LAN */

typedef enum
{
    SFP_NO_MODULE,
    SFP_FIBER,
    SFP_COPPER,
} serdes_sfp_type_t;

typedef enum
{
    SERDES_NOT_INITED,
    SERDES_INITED,
    SERDES_PCS_CONFIGURED,
} serdes_status_t;

enum SERDES_POWER_LEVEL {SERDES_POWER_DOWN, SERDES_POWER_STANDBY, SERDES_POWER_ON, SERDES_POWER_DOWN_FORCE};
typedef struct phy_serdes_s {
    phy_dev_t *phy_dev;
    int phy_type;
    int used;
    int power_mode;
    int power_admin_on;
    int sfp_status;
    serdes_status_t serdes_status;
    serdes_sfp_type_t sfp_module_type;
    int cur_power_level;
    int config_speed;
    int current_speed;
    int i2cDetectDelay;
    int i2cInitDetectDelay;
    int link_changed;   /* Flag to create a link down for Kernel to notify speed change */ 
    u16 signal_detect_gpio;
    u16 sfp_module_detect_gpio;
    int signal_detect_invert;
    int speed_caps;
    int highest_speed, highest_speed_cap;
    void (*link_stats)(phy_dev_t *phy_dev);
    int (*serdes_init)(phy_dev_t *phy_dev);
    //void (*ethsw_serdes_speed_detect)(phy_dev_t *phy_dev);
    int (*speed_set)(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
    int (*power_set)(phy_dev_t *phy_dev, int powerLevel);
    int (*enable_an)(phy_dev_t *phy_dev);
    int flag;
        #define FORCE_SPEED_SET_ON_SIG (1<<0)
#define I2CDetectDelay 8
#define I2CInitDetectDelay 8
    int sgmii_mode;
    #define XGAE_NONSGMII_MODE  0
    #define XGAE_SGMII_AUTO     1
    #define XGAE_SGMII_FORCE    2
} phy_serdes_t;

typedef struct phy_cl45_s {
    int config_speed;
    int duplex;
    void *descriptor;
} phy_cl45_t;

void serdes_work_around(phy_dev_t *phy_dev);
static inline int phy_bus_c45_read32(phy_dev_t *phy_dev, uint32_t reg32, uint16_t *val_p)
{
    return phy_bus_c45_read(phy_dev, ((reg32)>>16)&0xffff, reg32&0xffff, val_p) +
           phy_bus_c45_read(phy_dev, ((reg32)>>16)&0xffff, reg32&0xffff, val_p);
}
#define phy_bus_c45_write32(phy_dev, reg32, val) \
    phy_bus_c45_write(phy_dev, ((reg32)>>16)&0xffff, reg32&0xffff, val)
#define IsC45Phy(phy) (phy->phy_drv->phy_type == PHY_TYPE_EXT3 || phy->phy_drv->phy_type == PHY_TYPE_GPY211)

int ethsw_phy_exp_rw(phy_dev_t *phy_dev, u32 reg, u16 *v16_p, int rd);
static inline int ethsw_phy_exp_read32(phy_dev_t *phy_dev, u32 reg, u32 *v32_p)
{
    u16 v16; 
    int rc;
    rc = ethsw_phy_exp_rw(phy_dev, reg, &v16, 1); 
    *v32_p = v16;
    return rc;
}

#define ethsw_phy_exp_read(phy_dev, reg, v16_p) ethsw_phy_exp_rw(phy_dev, reg, v16_p, 1)
static inline int ethsw_phy_exp_write(phy_dev_t *phy_dev, u32 reg, u32 v)
{
    u16 v16=v; 
    int rc;
    rc = ethsw_phy_exp_rw(phy_dev, reg, &v16, 0); 
    return rc;
}

int sf2_serdes_phy_read_status(phy_dev_t *phy_dev);
int sf2_serdes_init(phy_dev_t *phy_dev);
int ethsw_serdes_power_mode_get(phy_dev_t *phy_dev, int *mode);
int ethsw_serdes_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps);
int ethsw_serdes_cfg_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
int ethsw_serdes_speed_get(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex);

static inline void ethsw_powerup_serdes(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (phy_serdes->power_set)
        phy_serdes->power_set(phy_dev, SERDES_POWER_ON);
}

static inline void ethsw_powerstandby_serdes(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (phy_serdes->power_set)
        phy_serdes->power_set(phy_dev, SERDES_POWER_STANDBY);
}

static inline void ethsw_powerdown_serdes(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (phy_serdes->power_set)
        phy_serdes->power_set(phy_dev, SERDES_POWER_DOWN);
}

static inline void ethsw_powerdown_forceoff_serdes(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (phy_serdes->power_set)
        phy_serdes->power_set(phy_dev, SERDES_POWER_DOWN_FORCE);
}

#endif
