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

#include <bdmf_data_types.h>

#define wd_log(fmt, ...) printk("SERDES: %s "fmt, __FUNCTION__, ##__VA_ARGS__)
#if 0
#define wd_log_debug(fmt, ...) printk("SERDES: %s "fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define wd_log_debug(fmt, ...)
#endif

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
    SERDES_WAN_TYPE_NGPON_10G_2_5G
} serdes_wan_type_t;

typedef enum
{
    DRV_SERDES_NO_ERROR, 
    DRV_SERDES_AMPLITUDE_IS_OUT_OF_RANGE,
}
serdes_error_t;


#define PMD_EWAKE_OFF_TIME   0x2f
#define PMD_EWAKE_SETUP_TIME 0x2f
#define PMD_EWAKE_HOLD_TIME  0x2f

int wan_serdes_config(serdes_wan_type_t wan_type);
serdes_wan_type_t wan_serdes_type_get(void);
void wan_prbs_gen(uint32_t enable, int enable_host_tracking, int mode, serdes_wan_type_t wan_type, bdmf_boolean *valid);
void wan_prbs_status(bdmf_boolean *valid, uint32_t *errors);
void pll_ppm_adj_FracN_10G(uint8_t mode, int16_t ppm_target, bool pll_adj_en);

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

void pon_wan_top_reset_gearbox_tx(void);

typedef void (*PMD_DEV_ENABLE_PRBS)(uint16_t enable, uint8_t prbs_mode);
void wan_register_pmd_prbs_callback(PMD_DEV_ENABLE_PRBS callback);


#ifdef CONFIG_BCM_XRDP
int ngpon_wan_post_init(uint32_t sync_frame_length);
void wan_top_tod_ts48_get(uint16_t *ts48_msb, uint32_t *ts48_lsb);
void gpon_ngpon_wan_top_set_lbe_invert(uint8_t lbe_invert_bit);
#endif

#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE)
void ngpon_wan_top_enable_transmitter(int enabled);
int ngpon_wan_post_init(uint32_t sync_frame_length);
void ngpon_wan_top_reset_gearbox_rx(void);
#endif

#ifdef CONFIG_BCM96848
void onu2g_read(uint32_t addr, uint16_t *data);
void onu2g_write(uint32_t addr, uint16_t data);
void wan_reset_rx_and_tx(void);
#endif

#ifdef CONFIG_BCM96838
serdes_error_t wan_serdes_amplitude_set(uint16_t val);
serdes_error_t wan_serdes_amplitude_get(uint16_t *val);
#endif

typedef struct pon_serdes_fixup_fifo_t
{
    void (*reset_fifo)(void);
    int (*gearbox_drift_test)(const int just_synced);
} pon_serdes_lof_fixup_fifo_t;
typedef void (*pon_serdes_lof_fixup_reset_cdr_t)(void);
int pon_serdes_lof_fixup_cfg(pon_serdes_lof_fixup_fifo_t *_fifo_cb, pon_serdes_lof_fixup_reset_cdr_t reset_cb);
void pon_serdes_lof_fixup_irq(int lof);
#endif

