/*
    Copyright 2000-2010 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard

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

#ifndef _WAN_DRV_H_
#define _WAN_DRV_H_


#if defined(__KERNEL__)

#include <linux/types.h>
#include <linux/bcm_log.h>

#define wd_log(fmt, ...) printk("SERDES: %s "fmt, __FUNCTION__, ##__VA_ARGS__)
#if 0
#define wd_log_debug(fmt, ...) printk("SERDES: %s "fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define wd_log_debug(fmt, ...)
#endif

#endif /* __KERNEL__ */


/* For both kernel/user space */
typedef enum
{
    SERDES_WAN_TYPE_NONE,
    SERDES_WAN_TYPE_GPON,
    SERDES_WAN_TYPE_EPON_1G,
    SERDES_WAN_TYPE_EPON_2G,
    SERDES_WAN_TYPE_AE,
    SERDES_WAN_TYPE_AE_2_5G,
    /* XGPON/NGPON types */
    SERDES_WAN_TYPE_AE_10G,
    SERDES_WAN_TYPE_EPON_10G_SYM,
    SERDES_WAN_TYPE_EPON_10G_ASYM,
    SERDES_WAN_TYPE_XGPON_10G_2_5G,
    SERDES_WAN_TYPE_NGPON_10G_10G,
    SERDES_WAN_TYPE_NGPON_10G_10G_8B10B,
    SERDES_WAN_TYPE_NGPON_10G_2_5G_8B10B,
    SERDES_WAN_TYPE_NGPON_10G_2_5G,
    SERDES_WAN_TYPE_AE_5G,
    SERDES_WAN_TYPE_AE_2_5G_R,
    SERDES_WAN_TYPE_MAX,
} serdes_wan_type_t;


#if defined(__KERNEL__)

int wan_serdes_config(serdes_wan_type_t wan_type);
int wan_serdes_deconfig(void);
serdes_wan_type_t wan_serdes_type_get(void);
int wan_serdes_type_try_get(serdes_wan_type_t *wan_type);
void wan_prbs_gen(uint32_t enable, int enable_host_tracking, int mode, serdes_wan_type_t wan_type, int *valid);
void wan_prbs_status(int *valid, uint32_t *errors);
void pll_ppm_adj_FracN_10G(uint8_t mode, int32_t ppm_target, bool pll_adj_en);

//WAN TYPES
#define     GPON_2_1    0x1
#define     XGPON_10_2  0x2
#define     NGPON_10_10 0x3
#define     NGPON_10_10_TWO_PLLS 0x4

#define     EPON_1_1    0x11
#define     EPON_2_1    0x12
#define     EPON_10_1   0x13
#define     EPON_10_10  0x14
#define     EPON_2_2    0x15

#define     AE_1_1      0x21
#define     AE_2_1      0x22
#define     AE_10_1     0x23
#define     AE_10_10    0x24
#define     AE_2_2      0x25
#define     AE_2_2_R    0x26
#define     AE_5_5      0x27

void pon_wan_top_reset_gearbox_tx(void);

void wan_top_tod_ts48_get(uint16_t *ts48_msb, uint32_t *ts48_lsb);

enum transmitter_control {
    laser_off,
    mac_control,
    laser_on,
};
void wan_top_transmitter_control(enum transmitter_control mode);
int ngpon_wan_post_init(uint32_t sync_frame_length);
void ngpon_wan_top_reset_gearbox_rx(void);
int wantop_bus_get(void);
void wantop_resync(int cdr_relock, int tx_fifo_reset, int delay_in_msec);

void pon_serdes_lof_fixup_irq(int lof);
void pon_serdes_fixup_on_enable(void);
void pon_serdes_fixup_on_disable(void);

static inline int wan_i2c_bus_get(void)
{
    bcmFun_t *cb = bcmFun_get(BCM_FUN_ID_WAN_SERDES_BUS_GET);
    return cb ? cb(NULL) : -1;
}

#if defined (CONFIG_BCM96858) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96855)
void wan_serdes_10g_epon_sgb_clk_enable(int enable);
#endif
#endif /* __KERNEL__ */

#endif

