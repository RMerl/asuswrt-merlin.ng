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

typedef enum
{
    SERDES_WAN_TYPE_NONE,
    SERDES_WAN_TYPE_GPON,
    SERDES_WAN_TYPE_EPON_1G,
    SERDES_WAN_TYPE_EPON_2G,
    SERDES_WAN_TYPE_AE,
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

typedef void (*PMD_DEV_ENABLE_PRBS)(uint16_t enable, uint8_t prbs_mode);
void wan_register_pmd_prbs_callback(PMD_DEV_ENABLE_PRBS callback);


#ifdef CONFIG_BCM96858
int ngpon_wan_post_init(uint32_t sync_frame_length);
void wan_top_tod_ts48_get(uint16_t *ts48_msb, uint32_t *ts48_lsb);
void ngpon_wan_top_enable_transmitter(int enabled);
#endif

#ifdef CONFIG_BCM96848
void onu2g_read(uint32_t addr, uint16_t *data);
void onu2g_write(uint32_t addr, uint16_t data);
#endif

#ifdef CONFIG_BCM96838
serdes_error_t wan_serdes_amplitude_set(uint16_t val);
serdes_error_t wan_serdes_amplitude_get(uint16_t *val);
#endif

#endif
