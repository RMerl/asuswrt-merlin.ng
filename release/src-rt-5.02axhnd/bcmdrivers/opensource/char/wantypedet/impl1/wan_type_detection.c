/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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

#include <linux/string.h>
#include <linux/delay.h>
#include <linux/bcm_log.h>
#include <bcm_map_part.h>
#include "pmd.h"
#include "board.h"
#include "wan_drv.h"
#include "opticaldet.h"
#include "rdpa_types.h"
#include "bcmsfp_i2c.h"


static int get_optics_type(uint16_t *optics_type);
static int get_supported_wan_type_bm(SUPPORTED_WAN_TYPES_BITMAP *wan_type_bm);


#define SENSE_RESULT_TRUE       (1)
#define SENSE_RESULT_FALSE      (0)
#define SKIP_SENSING_WAN_TYPE   (1)
#define KEEP_SENSING_WAN_TYPE   (0)

static int configure_pmd_wan_type(uint16_t optics_type, PMD_WAN_TYPES new_pmd_wan_type);

static get_non_brcm_pmd_supported_wan_type_bm   get_non_brcm_pmd_supported_wan_type_bm_cb = NULL;
static configure_non_brcm_pmd_wan_type          configure_non_brcm_pmd_wan_type_cb = NULL;

#if defined(CONFIG_BCM96858)
#define GET_GPON_REG_ADDRESS(physical_address) ( (unsigned int *)( (physical_address) - GPON_PHYS_BASE + bcm_io_block_address[GPON_IDX] ) )
#else
#define GET_GPON_REG_ADDRESS(physical_address) ( (unsigned int *)( (physical_address) - XRDP_PHYS_BASE + bcm_io_block_address[XRDP_IDX] ) )
#endif

#define SLEEP_IN_MILI_SECONDS      (  50)
#define LONG_SLEEP_IN_MILI_SECONDS (1500)

static int is_gpon_lof_synced(uint16_t optics_type)
{
#if defined(CONFIG_BCM96858)
#define GPON_RCVR_STATUS_REG_ADDRESS            (0x80150000)
#define GPON_RCVR_CONFIG_REG_ADDRESS            (0x80150004)
#define GPON_RCVR_CONFIG_LOF_PARAMS_REG_ADDRESS (0x80150008)
#else
#define GPON_RCVR_STATUS_REG_ADDRESS            (0x82db1000)
#define GPON_RCVR_CONFIG_REG_ADDRESS            (0x82db1004)
#define GPON_RCVR_CONFIG_LOF_PARAMS_REG_ADDRESS (0x82db1008)
#endif
#define GPON_RCVR_STATUS_BIT_LOF                (         1)
#define GPON_RCVR_CONFIG_BIT_RX_DISABLE         (    1 << 2)
#define GPON_RCVR_CONFIG_LOF_DELTA_SHIFT        (         0)
#define GPON_RCVR_CONFIG_LOF_DELTA              (         4)
#define GPON_RCVR_CONFIG_LOF_ALPHA_SHIFT        (         4)
#define GPON_RCVR_CONFIG_LOF_ALPHA              (         1)
#define GPON_RCVR_CONFIG_LOF_PARAMS_CLEAR_MASK  (     ~0xFF)
    unsigned int ret, reg_value, *reg_address, is_sensed;

    ret = configure_pmd_wan_type(optics_type, PMD_GPON_2_1_WAN);
    if (SKIP_SENSING_WAN_TYPE == ret) return SENSE_RESULT_FALSE;

    wan_serdes_config(SERDES_WAN_TYPE_GPON);

    reg_address = GET_GPON_REG_ADDRESS(GPON_RCVR_CONFIG_LOF_PARAMS_REG_ADDRESS);
    reg_value = *reg_address;
    reg_value &= GPON_RCVR_CONFIG_LOF_PARAMS_CLEAR_MASK;
    reg_value |= GPON_RCVR_CONFIG_LOF_DELTA << GPON_RCVR_CONFIG_LOF_DELTA_SHIFT;
    reg_value |= GPON_RCVR_CONFIG_LOF_ALPHA << GPON_RCVR_CONFIG_LOF_ALPHA_SHIFT;
    *reg_address = reg_value;

    reg_address = GET_GPON_REG_ADDRESS(GPON_RCVR_CONFIG_REG_ADDRESS);
    reg_value = *reg_address;

    reg_value &= ~GPON_RCVR_CONFIG_BIT_RX_DISABLE;
    *reg_address = reg_value;
    msleep(SLEEP_IN_MILI_SECONDS);

    reg_value = *GET_GPON_REG_ADDRESS(GPON_RCVR_STATUS_REG_ADDRESS);
    is_sensed = (0 == (GPON_RCVR_STATUS_BIT_LOF & reg_value));

    reg_value= *reg_address;
    reg_value &= GPON_RCVR_CONFIG_BIT_RX_DISABLE;
    *reg_address = reg_value;    

    if (is_sensed) BCM_LOG_NOTICE(BCM_LOG_ID_WANTYPEDET, "GPON was detected\n");

    return is_sensed;
}

#if defined(CONFIG_BCM96858)
#define EPON_TOP_RESET_REG_ADDRESS      (0x80140004)
#define EPON_TOP_CONTROL_REG_ADDRESS    (0x80140010)
#define LIF_PON_CONTROL_REG_ADDRESS     (0x80141800)
#define LIF_SEC_CONTROL_REG_ADDRESS     (0x8014180c)
#define LIF_INT_STATUS_REG_ADDRESS      (0x80141814)
#define LIF_RX_AGG_MPCP_FRM_REG_ADDRESS (0x80141900)
#else
#define EPON_PHYS_BASE  XRDP_PHYS_BASE
#define EPON_IDX        XRDP_IDX
#define EPON_TOP_RESET_REG_ADDRESS      (0x82dac004)
#define EPON_TOP_CONTROL_REG_ADDRESS    (0x82dac010)
#define LIF_PON_CONTROL_REG_ADDRESS     (0x82dad800)
#define LIF_SEC_CONTROL_REG_ADDRESS     (0x82dad80c)
#define LIF_INT_STATUS_REG_ADDRESS      (0x82dad814)
#define LIF_RX_AGG_MPCP_FRM_REG_ADDRESS (0x82dad8c0)
#endif
#define EPON_TOP_RESET_ALL              (         0)
#define XPCSRX_RST_ACTIVE_LOW_RESET     (         0)
#define XPCSRX_FRAMER_CTL_DISABLE_VALUE (0x00000090)

static int is_epon_ae_in_sync(SUPPORTED_WAN_TYPES_BITMAP wan_type, uint16_t optics_type)
{
#define EPON_TOP_LIF_RST_CLR            (         2)
#define EPON_TOP_CONTROL_2G_DS_MODE     (         4)
#define EPON_TOP_CONTROL_1G_DS_MODE     (         0)
#define LIF_PON_CONTROL_RX_EN_NORM_FLIP (0x00100005)
#define LIF_PON_CONTROL_BIT_RX_EN       (         1)
#define LIF_SEC_CONTROL_VALUE           (0x00000004)
#define LIF_INT_STATUS_ALL_CLR          (0x003fffff)
#define LIF_INT_RX_OUT_OF_SYNCH_STAT    (         1)

    unsigned int reg_value, *reg_address, is_sensed;
    char *wan_type_string;
    int ret;

    switch (wan_type)
    {
        case SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1:
            ret = configure_pmd_wan_type(optics_type, PMD_EPON_2_1_WAN);
            break;
        case SUPPORTED_WAN_TYPES_BIT_EPON_1_1:
            ret = configure_pmd_wan_type(optics_type, PMD_EPON_1_1_WAN);
            break;
        case SUPPORTED_WAN_TYPES_BIT_AE_1_1:
            ret = configure_pmd_wan_type(optics_type, PMD_GBE_1_1_WAN);
            break;
        default:
            ret = KEEP_SENSING_WAN_TYPE;
    }

    if (SKIP_SENSING_WAN_TYPE == ret) return SENSE_RESULT_FALSE;

    reg_address = (unsigned int *)(EPON_TOP_RESET_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = EPON_TOP_RESET_ALL;
    msleep(SLEEP_IN_MILI_SECONDS);

    switch (wan_type)
    {
        case SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1:
            wan_serdes_config(SERDES_WAN_TYPE_EPON_2G);
            wan_type_string = "EPON 2G";
            break;
        case SUPPORTED_WAN_TYPES_BIT_EPON_1_1:
            wan_serdes_config(SERDES_WAN_TYPE_EPON_1G);
            wan_type_string = "EPON 1G";
            break;
        case SUPPORTED_WAN_TYPES_BIT_AE_1_1:
            wan_serdes_config(SERDES_WAN_TYPE_AE);
            wan_type_string = "AE";
            break;
        default:
            return 0;
    }

    reg_address = (unsigned int *)(EPON_TOP_CONTROL_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = (SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1 == wan_type) ? EPON_TOP_CONTROL_2G_DS_MODE : EPON_TOP_CONTROL_1G_DS_MODE;

    reg_address = (unsigned int *)(EPON_TOP_RESET_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = EPON_TOP_LIF_RST_CLR;
    msleep(SLEEP_IN_MILI_SECONDS);

    reg_address = (unsigned int *)(LIF_PON_CONTROL_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = LIF_PON_CONTROL_RX_EN_NORM_FLIP;

    reg_address = (unsigned int *)(LIF_SEC_CONTROL_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = LIF_SEC_CONTROL_VALUE;
    msleep(SLEEP_IN_MILI_SECONDS);

    reg_address = (unsigned int *)(LIF_INT_STATUS_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = LIF_INT_STATUS_ALL_CLR;
    msleep(LONG_SLEEP_IN_MILI_SECONDS);

    reg_value = *(unsigned int *)(LIF_INT_STATUS_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    is_sensed = (0 == (LIF_INT_RX_OUT_OF_SYNCH_STAT & reg_value));

    if (is_sensed && (SUPPORTED_WAN_TYPES_BIT_EPON_1_1 == wan_type))
    {
        reg_value = *(unsigned int *)(LIF_RX_AGG_MPCP_FRM_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
        is_sensed = reg_value > 0;
    }

    reg_address = (unsigned int *)(LIF_PON_CONTROL_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    reg_value= *reg_address;
    reg_value &= ~LIF_PON_CONTROL_BIT_RX_EN;
    *reg_address = reg_value;

    if (is_sensed) BCM_LOG_NOTICE(BCM_LOG_ID_WANTYPEDET, "%s was detected\n", wan_type_string);
    
    return is_sensed;
}


#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)

#if defined(CONFIG_BCM96858)
#define XPCSRX_RST_REG_ADDRESS          (0x80143000)
#define XPCSRX_FRAMER_CTL_REG_ADDRESS   (0x8014300c)
#else
#define XPCSRX_RST_REG_ADDRESS          (0x82daf000)
#define XPCSRX_FRAMER_CTL_REG_ADDRESS   (0x82daf00c)
#endif

static void disable_10g_epon_ae_mac(void)
{
    unsigned int *reg_address;

    reg_address = (unsigned int *)(XPCSRX_FRAMER_CTL_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = XPCSRX_FRAMER_CTL_DISABLE_VALUE;

    reg_address = (unsigned int *)(XPCSRX_RST_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = XPCSRX_RST_ACTIVE_LOW_RESET;

    reg_address = (unsigned int *)(EPON_TOP_RESET_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = EPON_TOP_RESET_ALL;
}

static int is_10g_epon_ae_in_sync(SUPPORTED_WAN_TYPES_BITMAP wan_type, uint16_t optics_type)
{
#if defined(CONFIG_BCM96858)
#define XPCSRX_RAM_ECC_INT_STAT_REG_ADDRESS     (0x80143108)
#define XPCSRX_FEC_CTL_REG_ADDRESS              (0x80143010)
#define XPCSRX_INT_STAT_REG_ADDRESS             (0x80143004)
#define XPCSRX_64B_66B_START_CNT_REG_ADDRESS    (0x801430dc)
#define XPCSRX_IDLE_GOOD_PKT_CNT_REG_ADDRESS    (0x801430e0)
#define EPON_TOP_RESET_XPCS_RX_RST_CLR          (0x00000040)
#else
#define XPCSRX_RAM_ECC_INT_STAT_REG_ADDRESS     (0x82daf108)
#define XPCSRX_FEC_CTL_REG_ADDRESS              (0x82daf010)
#define XPCSRX_INT_STAT_REG_ADDRESS             (0x82daf004)
#define XPCSRX_64B_66B_START_CNT_REG_ADDRESS    (0x82daf0dc)
#define XPCSRX_IDLE_GOOD_PKT_CNT_REG_ADDRESS    (0x82daf0e0)
#define EPON_TOP_RESET_XPCS_RX_RST_CLR          (0x00000080)
#endif
#define XPCSRX_RST_CLR                          (         1)
#define EPON_TOP_CONTROL_10G_DS_MODE            (         1)
#define INT_RX_IDLE_RAM_INIT_DONE               (0x00000010)
#define XPCSRX_FRAMER_CTL_VALUE                 (0x00000094)
#define XPCSRX_FRAMER_CTL_FEC_VALUE             (0x00000095)
#define XPCSRX_FEC_CTL_FEC_BYPASSED             (0x00000004)
#define XPCSRX_FEC_CTL_FEC_ENABLED              (0x00000013)
#define XPCSRX_INT_STAT_CLR                     (0x7fcfffff)
#define XPCSRX_INT_STAT_RX_FRAMER_CW_LOCK       (0x00000020)

    unsigned int reg_value, *reg_address, is_sensed;
    char *wan_type_string;
    int ret;

    switch (wan_type)
    {
        case SUPPORTED_WAN_TYPES_BIT_EPON_10_1:
            ret = configure_pmd_wan_type(optics_type, PMD_EPON_10_1_WAN);
            break;
        default:
            ret = KEEP_SENSING_WAN_TYPE;
    }

    if (SKIP_SENSING_WAN_TYPE == ret) return SENSE_RESULT_FALSE;

    reg_address = (unsigned int *)(EPON_TOP_RESET_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = EPON_TOP_RESET_ALL;

    reg_address = (unsigned int *)(XPCSRX_RST_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = XPCSRX_RST_ACTIVE_LOW_RESET;

    msleep(SLEEP_IN_MILI_SECONDS);

    switch (wan_type)
    {
        case SUPPORTED_WAN_TYPES_BIT_EPON_10_10:
            wan_serdes_config(SERDES_WAN_TYPE_EPON_10G_SYM);
            wan_type_string = "EPON 10G/10G SYM";
            break;
        case SUPPORTED_WAN_TYPES_BIT_EPON_10_1:
            wan_serdes_config(SERDES_WAN_TYPE_EPON_10G_ASYM);
            wan_type_string = "EPON 10G/1G";
            break;
        case SUPPORTED_WAN_TYPES_BIT_AE_10_10:
            wan_serdes_config(SERDES_WAN_TYPE_AE_10G);
            wan_type_string = "AE 10G/10G";
            break;
        default:
            return 0;
    }
    msleep(SLEEP_IN_MILI_SECONDS);

    reg_address = (unsigned int *)(EPON_TOP_CONTROL_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = EPON_TOP_CONTROL_10G_DS_MODE;

    reg_address = (unsigned int *)(EPON_TOP_RESET_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = EPON_TOP_RESET_XPCS_RX_RST_CLR;

    msleep(SLEEP_IN_MILI_SECONDS);

    reg_address = (unsigned int *)(XPCSRX_RST_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = XPCSRX_RST_CLR;

    reg_address = (unsigned int *)(XPCSRX_FRAMER_CTL_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = XPCSRX_FRAMER_CTL_VALUE;
    
    reg_address = (unsigned int *)(XPCSRX_FEC_CTL_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = XPCSRX_FEC_CTL_FEC_BYPASSED;
    
    reg_address = (unsigned int *)(XPCSRX_INT_STAT_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
    *reg_address = XPCSRX_INT_STAT_CLR;

    msleep(LONG_SLEEP_IN_MILI_SECONDS);
    reg_value = *reg_address;

    if ((reg_value & XPCSRX_INT_STAT_RX_FRAMER_CW_LOCK) == XPCSRX_INT_STAT_RX_FRAMER_CW_LOCK)
    {
        reg_value = *(unsigned int *)(XPCSRX_64B_66B_START_CNT_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
        if (reg_value > 0)
        {
            is_sensed = (SUPPORTED_WAN_TYPES_BIT_AE_10_10 != wan_type);
        }
        else
        {
            is_sensed = (SUPPORTED_WAN_TYPES_BIT_AE_10_10 == wan_type);
        }
        
        if (is_sensed)
        {
            BCM_LOG_NOTICE(BCM_LOG_ID_WANTYPEDET, "%s was detected (FEC disabled)\n", wan_type_string);
        }
        
        disable_10g_epon_ae_mac();
        return is_sensed;
    }

    if (SUPPORTED_WAN_TYPES_BIT_AE_10_10 != wan_type)
    {
        reg_address = (unsigned int *)(XPCSRX_RST_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
        *reg_address = XPCSRX_RST_ACTIVE_LOW_RESET;

        reg_address = (unsigned int *)(XPCSRX_FRAMER_CTL_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
        *reg_address = XPCSRX_FRAMER_CTL_FEC_VALUE;

        reg_address = (unsigned int *)(XPCSRX_FEC_CTL_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
        *reg_address = XPCSRX_FEC_CTL_FEC_ENABLED;

        reg_address = (unsigned int *)(XPCSRX_RST_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
        *reg_address = XPCSRX_RST_CLR;

        msleep(LONG_SLEEP_IN_MILI_SECONDS);
    
        reg_value = *(unsigned int *)(XPCSRX_INT_STAT_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);

        if ((reg_value & XPCSRX_INT_STAT_RX_FRAMER_CW_LOCK) == XPCSRX_INT_STAT_RX_FRAMER_CW_LOCK)
        {
            reg_value = *(unsigned int *)(XPCSRX_IDLE_GOOD_PKT_CNT_REG_ADDRESS - EPON_PHYS_BASE + bcm_io_block_address[EPON_IDX]);
            if (reg_value > 0)
            {
                disable_10g_epon_ae_mac();
                BCM_LOG_NOTICE(BCM_LOG_ID_WANTYPEDET, "%s was detected (FEC enabled)\n", wan_type_string);
                return SENSE_RESULT_TRUE;
            }
        }
    
    }

    disable_10g_epon_ae_mac();
    return SENSE_RESULT_FALSE;
}


static int is_10g_itu_pon_frame_sync(SUPPORTED_WAN_TYPES_BITMAP wan_type, uint16_t optics_type)
{
#if defined(CONFIG_BCM96858)
#define NGPON_RX_GEN_RCVRSTAT_REG_ADDRESS           (0x80160000)
#define NGPON_RX_GEN_RCVRCFG_REG_ADDRESS            (0x80160004)
#define NGPON_RX_GEN_SYNC_CFG_REG_ADDRESS           (0x80160008)
#else
#define NGPON_RX_GEN_RCVRSTAT_REG_ADDRESS           (0x82db4000)
#define NGPON_RX_GEN_RCVRCFG_REG_ADDRESS            (0x82db4004)
#define NGPON_RX_GEN_SYNC_CFG_REG_ADDRESS           (0x82db4008)
#endif
#define NGPON_RX_GEN_RCVRSTAT_BIT_FRAME_SYNC_STATE  (         1)
#define NGPON_RX_GEN_RCVRCFG_MAC_MODE_SHIFT         (         2)
#define NGPON_RX_GEN_RCVRCFG_MAC_MODE_MASK          (         3)
#define NGPON_RX_GEN_RCVRCFG_BIT_RX_ENABLE          (         1)
#define GPON_RX_GEN_RCVRCFG_MAC_MODE_RESET          (         0)
#define GPON_RX_GEN_RCVRCFG_MAC_MODE_NGPON2_10G     (         2)
#define GPON_RX_GEN_RCVRCFG_MAC_MODE_NGPON2_2_5G    (         3)
#define NGPON_RX_GEN_SYNC_CFG_SYNC_LOSS_THR_SHIFT   (         0)
#define NGPON_RX_GEN_SYNC_CFG_SYNC_LOSS_THR         (         2)
#define NGPON_RX_GEN_SYNC_CFG_SYNC_ACQ_THR_SHIFT    (         4)
#define NGPON_RX_GEN_SYNC_CFG_SYNC_ACQ_THR          (         3)
#define NGPON_RX_GEN_SYNC_CFG_PONID_ACQ_THR_SHIFT   (         8)
#define NGPON_RX_GEN_SYNC_CFG_PONID_ACQ_THR         (         2)
#define NGPON_RX_GEN_SYNC_CFG_CLEAR_MASK            (    ~0xFFF)

    unsigned int reg_value, *reg_address, is_sensed, mac_mode;
    char *wan_type_string;
    int ret;

    switch (wan_type)
    {
        case SUPPORTED_WAN_TYPES_BIT_XGPON:
            ret = configure_pmd_wan_type(optics_type, PMD_XGPON1_10_2_WAN);
            if (SKIP_SENSING_WAN_TYPE == ret) return SENSE_RESULT_FALSE;

            mac_mode = GPON_RX_GEN_RCVRCFG_MAC_MODE_NGPON2_2_5G;
            wan_serdes_config(SERDES_WAN_TYPE_XGPON_10G_2_5G);
            wan_type_string = "XGPON";
            break;
        case SUPPORTED_WAN_TYPES_BIT_NGPON2_10_25:
            mac_mode = GPON_RX_GEN_RCVRCFG_MAC_MODE_NGPON2_2_5G;
            wan_serdes_config(SERDES_WAN_TYPE_NGPON_10G_2_5G);
            wan_type_string = "NGPON2 10G/2.5G";
            break;
        case SUPPORTED_WAN_TYPES_BIT_NGPON2_10_10:
            wan_type_string = "NGPON2 10G/10G";
        case SUPPORTED_WAN_TYPES_BIT_XGSPON:
            mac_mode = GPON_RX_GEN_RCVRCFG_MAC_MODE_NGPON2_10G;
            wan_serdes_config(SERDES_WAN_TYPE_NGPON_10G_10G);
            if (SUPPORTED_WAN_TYPES_BIT_XGSPON == wan_type) wan_type_string = "XGSPON";
            break;
        default:
            return SENSE_RESULT_FALSE;
    }
    
    msleep(SLEEP_IN_MILI_SECONDS);

    reg_address = (unsigned int *)(NGPON_RX_GEN_RCVRCFG_REG_ADDRESS - NGPON2_PHYS_BASE + bcm_io_block_address[NGPON2_IDX]);
    *reg_address = GPON_RX_GEN_RCVRCFG_MAC_MODE_RESET;
    
    msleep(SLEEP_IN_MILI_SECONDS);

    reg_address = (unsigned int *)(NGPON_RX_GEN_SYNC_CFG_REG_ADDRESS - NGPON2_PHYS_BASE + bcm_io_block_address[NGPON2_IDX]);
    reg_value = *reg_address;
    reg_value &= NGPON_RX_GEN_SYNC_CFG_CLEAR_MASK;
    reg_value |= NGPON_RX_GEN_SYNC_CFG_PONID_ACQ_THR << NGPON_RX_GEN_SYNC_CFG_PONID_ACQ_THR_SHIFT;
    reg_value |= NGPON_RX_GEN_SYNC_CFG_SYNC_ACQ_THR  << NGPON_RX_GEN_SYNC_CFG_SYNC_ACQ_THR_SHIFT;
    reg_value |= NGPON_RX_GEN_SYNC_CFG_SYNC_LOSS_THR << NGPON_RX_GEN_SYNC_CFG_SYNC_LOSS_THR_SHIFT;
    *reg_address = reg_value;

    msleep(SLEEP_IN_MILI_SECONDS);

    reg_address = (unsigned int *)(NGPON_RX_GEN_RCVRCFG_REG_ADDRESS - NGPON2_PHYS_BASE + bcm_io_block_address[NGPON2_IDX]);
    reg_value = *reg_address;

    reg_value &= ~(NGPON_RX_GEN_RCVRCFG_MAC_MODE_MASK << NGPON_RX_GEN_RCVRCFG_MAC_MODE_SHIFT);
    reg_value |= mac_mode << NGPON_RX_GEN_RCVRCFG_MAC_MODE_SHIFT;
    *reg_address = reg_value;

    msleep(SLEEP_IN_MILI_SECONDS);

    reg_value |= NGPON_RX_GEN_RCVRCFG_BIT_RX_ENABLE;
    *reg_address = reg_value;

    msleep(LONG_SLEEP_IN_MILI_SECONDS);
    
    reg_value = *(unsigned int *)(NGPON_RX_GEN_RCVRSTAT_REG_ADDRESS - NGPON2_PHYS_BASE + bcm_io_block_address[NGPON2_IDX]);
    is_sensed = NGPON_RX_GEN_RCVRSTAT_BIT_FRAME_SYNC_STATE & reg_value;
    
    reg_value= *reg_address;
    reg_value &= ~(NGPON_RX_GEN_RCVRCFG_MAC_MODE_MASK << NGPON_RX_GEN_RCVRCFG_MAC_MODE_SHIFT);
    reg_value &= ~NGPON_RX_GEN_RCVRCFG_BIT_RX_ENABLE;
    *reg_address = reg_value;
    
    if (is_sensed) BCM_LOG_NOTICE(BCM_LOG_ID_WANTYPEDET, "%s was detected\n", wan_type_string);

    return is_sensed;
}
#endif


static int configure_pmd_wan_type(uint16_t optics_type, PMD_WAN_TYPES new_pmd_wan_type)
{
    int rc;
    
    if (BCM_I2C_PON_OPTICS_TYPE_NON_BRCM_PMD == optics_type)
    {
        configure_non_brcm_pmd_wan_type_cb(new_pmd_wan_type);
        return KEEP_SENSING_WAN_TYPE;        
    }

    if (BCM_I2C_PON_OPTICS_TYPE_PMD != optics_type)
    {
        return KEEP_SENSING_WAN_TYPE;
    }
    
    rc = pmd_dev_reconfigure_wan_type(new_pmd_wan_type);
    if (rc)
    {
        return SKIP_SENSING_WAN_TYPE;
    }
    
    return KEEP_SENSING_WAN_TYPE;
}


static int is_specific_wan_type_sensed(SUPPORTED_WAN_TYPES_BITMAP wan_type, uint16_t optics_type)
{
    switch (wan_type)
    {
        case SUPPORTED_WAN_TYPES_BIT_GPON:
            return is_gpon_lof_synced(optics_type);
        case SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1:
        case SUPPORTED_WAN_TYPES_BIT_EPON_1_1:
        case SUPPORTED_WAN_TYPES_BIT_AE_1_1:
            return is_epon_ae_in_sync(wan_type, optics_type);
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)
        case SUPPORTED_WAN_TYPES_BIT_EPON_10_10:
        case SUPPORTED_WAN_TYPES_BIT_EPON_10_1:
        case SUPPORTED_WAN_TYPES_BIT_AE_10_10:
            return is_10g_epon_ae_in_sync(wan_type, optics_type);
        case SUPPORTED_WAN_TYPES_BIT_XGPON:
        case SUPPORTED_WAN_TYPES_BIT_NGPON2_10_25:
        case SUPPORTED_WAN_TYPES_BIT_NGPON2_10_10:
        case SUPPORTED_WAN_TYPES_BIT_XGSPON:
            return is_10g_itu_pon_frame_sync(wan_type, optics_type);
#endif
        default:
            BCM_LOG_ERROR(BCM_LOG_ID_WANTYPEDET, "Error: Can't detect 0x%x WAN type\n", wan_type);
    }
    return SENSE_RESULT_FALSE;
}


static int is_wan_type_bit_set_in_bitmap(SUPPORTED_WAN_TYPES_BITMAP supported_wan_type_bit, SUPPORTED_WAN_TYPES_BITMAP supported_wan_type_bm)
{
    return (supported_wan_type_bit & supported_wan_type_bm);
}
static int is_only_single_wan_type_bit_set_in_bitmap(SUPPORTED_WAN_TYPES_BITMAP supported_wan_type_bit, SUPPORTED_WAN_TYPES_BITMAP supported_wan_type_bm)
{
    return (supported_wan_type_bit == supported_wan_type_bm);
}
static int should_set_wan_type_in_scratchpad(SUPPORTED_WAN_TYPES_BITMAP supported_wan_type_bit, SUPPORTED_WAN_TYPES_BITMAP supported_wan_type_bm, uint16_t optics_type)
{
    if (!is_wan_type_bit_set_in_bitmap(supported_wan_type_bit, supported_wan_type_bm))
        return SENSE_RESULT_FALSE;
    if (is_only_single_wan_type_bit_set_in_bitmap(supported_wan_type_bit, supported_wan_type_bm))
        return SENSE_RESULT_TRUE;
    if (is_specific_wan_type_sensed(supported_wan_type_bit, optics_type))
        return SENSE_RESULT_TRUE;

    return SENSE_RESULT_FALSE;
}


rdpa_wan_type try_wan_type_sensing(void)
{
    int ret;
    SUPPORTED_WAN_TYPES_BITMAP supported_wan_type_bm;
    uint16_t optics_type;

    ret = get_optics_type(&optics_type);
    if (ret)
    {
        return rdpa_wan_none;
    }

    ret = get_supported_wan_type_bm(&supported_wan_type_bm);
    if (OPTICALDET_SUCCESS != ret)
    {
        return rdpa_wan_none;
    }
    BCM_LOG_DEBUG(BCM_LOG_ID_WANTYPEDET, "supported_wan_type_bm = 0x%x\n", supported_wan_type_bm);

#define SHOULD_SET_WAN_TYPE_IN_SCRATCHPAD(wan_type_bit) should_set_wan_type_in_scratchpad(wan_type_bit, supported_wan_type_bm, optics_type)
#define SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(token, value) do    \
    {                                                               \
        if (kerSysScratchPadSet(token, value, strlen(value)))       \
            return rdpa_wan_none;                                   \
    } while (0)

    if (SHOULD_SET_WAN_TYPE_IN_SCRATCHPAD(SUPPORTED_WAN_TYPES_BIT_GPON))
    {
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_TYPE_PSP_KEY, RDPA_WAN_TYPE_VALUE_GPON);
        return rdpa_wan_gpon;
    }

    if (SHOULD_SET_WAN_TYPE_IN_SCRATCHPAD(SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1))
    {   /* Notice that EPON 2/1 is checked before EPON 1/1 on purpose to allow TURBO mode when possible */
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_TYPE_PSP_KEY, RDPA_WAN_TYPE_VALUE_EPON);
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_RATE_PSP_KEY, "2501");
        return rdpa_wan_epon;
    }

    if (SHOULD_SET_WAN_TYPE_IN_SCRATCHPAD(SUPPORTED_WAN_TYPES_BIT_EPON_1_1))
    {
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_TYPE_PSP_KEY, RDPA_WAN_TYPE_VALUE_EPON);
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_RATE_PSP_KEY, "0101");
        return rdpa_wan_epon;
    }

    if (SHOULD_SET_WAN_TYPE_IN_SCRATCHPAD(SUPPORTED_WAN_TYPES_BIT_EPON_10_1))
    {
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_TYPE_PSP_KEY, RDPA_WAN_TYPE_VALUE_EPON);
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_RATE_PSP_KEY, "1001");
        return rdpa_wan_xepon;
    }

    if (SHOULD_SET_WAN_TYPE_IN_SCRATCHPAD(SUPPORTED_WAN_TYPES_BIT_EPON_10_10))
    {
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_TYPE_PSP_KEY, RDPA_WAN_TYPE_VALUE_EPON);
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_RATE_PSP_KEY, "1010");
        return rdpa_wan_xepon;
    }

    if (SHOULD_SET_WAN_TYPE_IN_SCRATCHPAD(SUPPORTED_WAN_TYPES_BIT_AE_1_1))
    {
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_TYPE_PSP_KEY, RDPA_WAN_TYPE_VALUE_GBE);
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_OEMAC_PSP_KEY, RDPA_WAN_OEMAC_VALUE_EPONMAC);
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_RATE_PSP_KEY, "0101");
        return rdpa_wan_gbe;
    }

    if (SHOULD_SET_WAN_TYPE_IN_SCRATCHPAD(SUPPORTED_WAN_TYPES_BIT_AE_10_10))
    {
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_TYPE_PSP_KEY, RDPA_WAN_TYPE_VALUE_GBE);
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_OEMAC_PSP_KEY, RDPA_WAN_OEMAC_VALUE_EPONMAC);
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_RATE_PSP_KEY, "1010");
        return rdpa_wan_xepon;
    }

    if (SHOULD_SET_WAN_TYPE_IN_SCRATCHPAD(SUPPORTED_WAN_TYPES_BIT_XGPON))
    {
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_TYPE_PSP_KEY, RDPA_WAN_TYPE_VALUE_XGPON1);
        return rdpa_wan_xgpon;
    }

    if (SHOULD_SET_WAN_TYPE_IN_SCRATCHPAD(SUPPORTED_WAN_TYPES_BIT_NGPON2_10_25))
    {
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_TYPE_PSP_KEY, RDPA_WAN_TYPE_VALUE_NGPON2);
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_RATE_PSP_KEY, "1025");
        return rdpa_wan_xgpon;
    }

    if (SHOULD_SET_WAN_TYPE_IN_SCRATCHPAD(SUPPORTED_WAN_TYPES_BIT_NGPON2_10_10))
    {
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_TYPE_PSP_KEY, RDPA_WAN_TYPE_VALUE_NGPON2);
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_RATE_PSP_KEY, "1010");
        return rdpa_wan_xgpon;
    }

    if (SHOULD_SET_WAN_TYPE_IN_SCRATCHPAD(SUPPORTED_WAN_TYPES_BIT_XGSPON))
    {
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_TYPE_PSP_KEY, RDPA_WAN_TYPE_VALUE_XGS);
        SET_SCRATCHPAD_RETURN_WAN_NONE_ON_ERROR(RDPA_WAN_RATE_PSP_KEY, "1010");
        return rdpa_wan_xgpon;
    }

    BCM_LOG_ERROR(BCM_LOG_ID_WANTYPEDET, "Error: Can't detect any supported WAN type at MAC layer\n");
    return rdpa_wan_none;
}


static int get_optics_type(uint16_t *optics_type)
{
    int ret;

    if (get_non_brcm_pmd_supported_wan_type_bm_cb || configure_non_brcm_pmd_wan_type_cb)
    {
        *optics_type = BCM_I2C_PON_OPTICS_TYPE_NON_BRCM_PMD;
        return OPTICALDET_SUCCESS;
    }

    ret = bcm_i2c_pon_optics_type_get(optics_type);
    if (ret)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_WANTYPEDET, "Error: The PMD board profile is not configured and the optics type cannot be determined\n");
    }

    return ret;
}


static int get_supported_wan_type_bm(SUPPORTED_WAN_TYPES_BITMAP *wan_type_bm)
{
    int ret, bus;

    if (get_non_brcm_pmd_supported_wan_type_bm_cb)
    {
        get_non_brcm_pmd_supported_wan_type_bm_cb(wan_type_bm);
        return OPTICALDET_SUCCESS;
    }

    ret = opticaldet_get_xpon_i2c_bus_num(&bus);
    if (OPTICALDET_SUCCESS != ret)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_WANTYPEDET, "Error: Can't get optical device i2c bus number\n");
        return ret;
    }
    ret = trx_get_supported_wan_type_bm(bus, wan_type_bm);
    if (OPTICALDET_SUCCESS != ret)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_WANTYPEDET, "Error: No optical device detection\n");
    }

    return ret;
}


void register_non_brcm_pmd(get_non_brcm_pmd_supported_wan_type_bm get_bitmap_callback, configure_non_brcm_pmd_wan_type configure_callback)
{
    get_non_brcm_pmd_supported_wan_type_bm_cb = get_bitmap_callback;
    configure_non_brcm_pmd_wan_type_cb = configure_callback;
}
EXPORT_SYMBOL(register_non_brcm_pmd);
