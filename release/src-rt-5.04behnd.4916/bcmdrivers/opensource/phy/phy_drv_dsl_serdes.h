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
 *  Created on: Dec 2016
 *      Author: steven.hsieh@broadcom.com
 */

#ifndef __PHY_DRV_DSL_SERDES_H__
#define __PHY_DRV_DSL_SERDES_H__

#include "phy_drv_dsl_phy.h"
#include "serdes_access.h"

extern struct mutex serdes_mutex;
#define MAX_SERDES_NUMBER 9
typedef enum
{
    SFP_FIXED_PHY,
    SFP_NO_MODULE,
    SFP_GPON_MODULE,
    SFP_AE_MODULE,
} serdes_sfp_type_t;

enum SERDES_POWER_LEVEL {SERDES_POWER_DOWN, SERDES_POWER_UP};
typedef enum {
    LASER_FLOAT,
    LASER_ON,
    LASER_OFF,
} laser_op_t;

typedef struct {
    int current_long_timer;
    int speed_detect_interval;
    int speed_detect_counts;
    int short_value_ms;
    int long_value_ms;
    int short_timer_weight;
    int long_timer_weight;
} spd_dtc_tmr_t;

typedef struct phy_serdes_s {
    phy_dsl_priv_m;
    void *priv;
    int phy_type;
    int used;
    int power_mode;
    int silent_start_enabled;
    int power_admin_on;
    int sfp_status;
    int core_num;
    serdes_sfp_type_t sfp_module_type;
    int cur_power_level;
    s16 signal_detect_gpio;
    int serdes_rate_mode;
    int (*sfp_module_detected)(phy_dev_t *phy_dev);
    int signal_detect_invert;
    phy_speed_t lowest_speed, highest_speed;
    phy_speed_t common_lowest_speed, common_highest_speed;  /* Hi/Lo of cascaded common */
    int lowest_speed_cap, highest_speed_cap;        /* Hi/Lo in speed_cap format */
    int common_lowest_speed_cap, common_highest_speed_cap;
    void (*link_stats)(phy_dev_t *phy_dev);
    int (*serdes_init)(phy_dev_t *phy_dev);
    //void (*dsl_serdes_speed_detect)(phy_dev_t *phy_dev);
    int (*speed_set)(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
    int (*power_set)(phy_dev_t *phy_dev, int powerLevel);
    int (*enable_an)(phy_dev_t *phy_dev);
    int (*enable_multi_speed_an)(phy_dev_t *phy_dev);
    int (*light_detected)(phy_dev_t *phy_dev);
    int (*silent_start_light_detected)(phy_dev_t *phy_dev);   /* For PON/AE dual role device */
    void (*lbe_op)(phy_dev_t *phy_dev, laser_op_t lbe_op);
    int flag;
        #define FORCE_SPEED_SET_ON_SIG  (1<<0)
        #define LANE_STRUCTURE          (1<<1)
        #define CORE_STRUCTURE          (1<<2)
        #define LINK_WARNING            (1<<3)
    int sgmii_mode;
    int sgmii_an_status;
    int serdes_speed_mode;
    int bp_intf_type;
    int inited;
    int print_log;
    int apd_inited;
    int vco;
    laser_op_t laser_status;
    int usxgmii_m_index;
    spd_dtc_tmr_t tmr;
    int adv_caps;
    int (*read_txfir_reg)(phy_dev_t *phy_dev, phy_txfir_t *txfir);
    int (*write_txfir_reg)(phy_dev_t *phy_dev, phy_txfir_t *txfir);
} phy_serdes_t;

inline static int dsl_phy_tx_cfg_get(phy_dev_t *phy_dev, uint32_t *pre, uint32_t *main, uint32_t *post1, uint32_t *post2, uint32_t *hpf)
{
    int8_t _pre, _main, _post1, _post2, _hpf;
    int rc;

    if (!phy_dev->phy_drv->tx_cfg_get)
        return -1;

    rc = phy_dev_tx_cfg_get(phy_dev, &_pre, &_main, &_post1, &_post2, &_hpf);
    *pre = _pre;
    *main = _main;
    *post1 = _post1;
    *post2 = _post2;
    *hpf = _hpf;
    return rc;
}

static inline int dsl_read_txfir_reg(phy_dev_t *phy_dev, phy_txfir_t *txfir)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (!phy_serdes->read_txfir_reg)
        return -1;
    return phy_serdes->read_txfir_reg(phy_dev, txfir);
}

static inline int dsl_write_txfir_reg(phy_dev_t *phy_dev, phy_txfir_t *txfir)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (!phy_serdes->write_txfir_reg)
        return -1;
    return phy_serdes->write_txfir_reg(phy_dev, txfir);
}

enum {NONSGMII_MODE, SGMII_AUTO, SGMII_FORCE};

#define PHY_IS_SERDES(phy_dev) ( \
    phy_dev && phy_dev->phy_drv && ( \
    phy_dev->phy_drv->phy_type == PHY_TYPE_138CLASS_SERDES || \
    phy_dev->phy_drv->phy_type == PHY_TYPE_158CLASS_SERDES || \
    phy_dev->phy_drv->phy_type == PHY_TYPE_146CLASS_SERDES ))

int dsl_serdes_priv_fun(phy_dev_t *phy_dev, int op_code, va_list ap);
static inline void dsl_serdes_sfp_lbe_op(phy_dev_t *phy_dev,  laser_op_t lbe_op)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    /* Only skip for laser off since power off can also change laser status */
    if (phy_serdes->laser_status == LASER_OFF && lbe_op == LASER_OFF)
        return ;

    if(phy_serdes->lbe_op)
        (*phy_serdes->lbe_op)(phy_dev, lbe_op);
    phy_serdes->laser_status = lbe_op;
}

void phy_serdes_polarity_config(phy_dev_t *phy_dev);

static inline void dsl_powerup_serdes(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (phy_serdes->power_set)
        phy_serdes->power_set(phy_dev, SERDES_POWER_UP);
}

static inline void dsl_powerdown_serdes(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    phy_serdes->link_changed = phy_serdes->cur_power_level == SERDES_POWER_UP;

    if (phy_serdes->power_set && !PhyIsSharedRefClkSet(phy_dev))
        phy_serdes->power_set(phy_dev, SERDES_POWER_DOWN);
}

static inline void dsl_serdes_power_set(phy_dev_t *phy_dev, int enable)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (!phy_serdes->power_set)
        return;

    if(enable) {
        if(phy_serdes->power_mode == SERDES_NO_POWER_SAVING)
            dsl_powerup_serdes(phy_dev);
    }
    else
        dsl_powerdown_serdes(phy_dev);
}


static inline int phy_serdes_sfp_module_detected(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (phy_serdes->sfp_module_detected == NULL)
        return 0;
    return phy_serdes->sfp_module_detected(phy_dev);
}

static inline void phy_dsl_serdes_access_get_an_status(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int lane_index = phy_dev->lane_index;
    uint8_t an_link_status;

    if (phy_dev->usxgmii_m_type != USXGMII_M_NONE)
        phy_dev->lane_index = phy_serdes->usxgmii_m_index;
    serdes_access_get_an_status(phy_dev, &an_link_status);
    phy_dev->lane_index = lane_index;
    phy_dev->link = an_link_status;
}

static inline void phy_dsl_serdes_access_get_speed(phy_dev_t *phy_dev)
{
    int lane_index = phy_dev->lane_index;
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (phy_dev->usxgmii_m_type != USXGMII_M_NONE)
        phy_dev->lane_index = phy_serdes->usxgmii_m_index;
    serdes_access_get_speed(phy_dev, &phy_dev->speed);
    phy_dev->lane_index = lane_index;
}

int dsl_txfir_get(phy_dev_t *phy_dev, phy_txfir_t *txfir);
int dsl_txfir_set(phy_dev_t *phy_dev, phy_txfir_t *txfir);
int phy_dsl_serdes_read_status_lock(phy_dev_t *phy_dev);
int phy_dsl_serdes_read_status(phy_dev_t *phy_dev);
int dsl_serdes_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps);
int dsl_serdes_caps_set(phy_dev_t *phy_dev, uint32_t caps);
int dsl_serdes_caps_set_lock(phy_dev_t *phy_dev, uint32_t caps);
int phy_dsl_serdes_init(phy_dev_t *phy_dev);
int phy_dsl_serdes_post_init(phy_dev_t *phy_dev);
int phy_dsl_serdes_power_set_lock(phy_dev_t *phy_dev, int enable);
int phy_dsl_serdes_power_get(phy_dev_t *phy_dev, int *enable);
int dsl_phy_enable_an(phy_dev_t *phy_dev);
#if defined(PHY_138CLASS_SERDES)
void serdes_work_around(phy_dev_t *phy_dev);
#else
#define serdes_work_around(phy)
#endif
enum {SFP_MODULE_NOT_DETECTED, SFP_MODULE_DETECTED, SFP_MODULE_I2C_CONF_DETECTED};

int phy_i2c_module_detect(phy_dev_t *phy_dev);
void phy_i2c_module_type_detect(phy_dev_t *phy_dev);
int phy_dev_sltstt_get(phy_dev_t *phy_dev, int *mode);
int phy_dev_sltstt_set(phy_dev_t *phy_dev, int mode);
int dsl_serdes_power_mode_get(phy_dev_t *phy_dev, int *mode);
int dsl_serdes_power_mode_set(phy_dev_t *phy_dev, int mode);
int dsl_serdes_apd_get(phy_dev_t *phy_dev, int *enable);
int dsl_serdes_apd_set_lock(phy_dev_t *phy_dev, int enable);
int dsl_serdes_cfg_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
int dsl_serdes_cfg_speed_set_lock(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
int dsl_serdes_speed_get_lock(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex);
int dsl_serdes_inter_phy_types_get(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, uint32_t *types);
int phy_dsl_serdes_dt_priv(dt_handle_t handle, uint32_t addr, uint32_t phy_mode, void **_priv);
int phy_dsl_serdes_dev_add_lock(phy_dev_t *phy_dev);
char *dsl_serdes_get_phy_name(phy_dev_t *phy_dev);
int dsl_serdes_check_power(phy_dev_t *phy_dev);

static inline int dsl_serdes_silent_start_supported(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = (phy_serdes_t *)phy_dev->priv;
    return phy_serdes->silent_start_light_detected != 0;
}

static inline int dsl_serdes_is_silent_start_enabled(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = (phy_serdes_t *)phy_dev->priv;
    return phy_serdes->silent_start_enabled;
}

static inline int dsl_serdes_silent_start_light_detected(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = (phy_serdes_t *)phy_dev->priv;
    phy_dev_t *phy_next = phy_dev->cascade_next;

    /* Always return detected for Copper PHY or Copper SFP connection */
    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY || (phy_next->flag & PHY_FLAG_COPPER_SFP_TYPE) ||
        !dsl_serdes_is_silent_start_enabled(phy_dev))
        return 1;
    
    if (!phy_serdes->silent_start_light_detected)
        return 1;
    return phy_serdes->silent_start_light_detected(phy_dev);
}

static inline int dsl_serdes_light_detected(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = (phy_serdes_t *)phy_dev->priv;
    phy_dev_t *phy_next = phy_dev->cascade_next;

    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY || (phy_next->flag & PHY_FLAG_COPPER_SFP_TYPE))   
        return 1;
    
    if (!phy_serdes->light_detected)
        return 1;
    return phy_serdes->light_detected(phy_dev);
}
#if defined(CONFIG_I2C) && defined(CONFIG_BCM_OPTICALDET)
int phy_drv_dsl_i2c_create_lock(phy_dev_t *phy_dev);
#endif
int dsl_serdes_cfg_speed_mode_set_lock(phy_dev_t *phy_dev, int adv_phy_caps, phy_duplex_t duplex,
        inter_phy_types_dir_t dir, int inter_type, int cfg_an_enable);
int phy_dsl_txfir_init(phy_dev_t *phy_dev);

#endif //__PHY_DRV_DSL_SERDES_H__
