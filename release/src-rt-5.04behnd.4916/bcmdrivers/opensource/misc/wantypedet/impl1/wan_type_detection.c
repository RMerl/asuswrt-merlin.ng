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
#include "pmd.h"
#include "board.h"
#include "wan_drv.h"
#include "opticaldet.h"
#include "rdpa_types.h"
#include "trxbus.h"

#if defined(CONFIG_BRCM_SMC_BOOT)
#include <bp3_license.h>
#include <pmc_wan.h>
#endif

static int get_optics_type(uint16_t *optics_type);
static int get_supported_wan_type_bm(SUPPORTED_WAN_TYPES_BITMAP *wan_type_bm);


#define SENSE_RESULT_TRUE       (1)
#define SENSE_RESULT_FALSE      (0)
#define SKIP_SENSING_WAN_TYPE   (1)
#define KEEP_SENSING_WAN_TYPE   (0)

static int configure_pmd_wan_type(uint16_t optics_type, PMD_WAN_TYPES new_pmd_wan_type, pmd_calibration_parameters
    *calibration_parameters_from_json);

static get_non_brcm_pmd_supported_wan_type_bm   get_non_brcm_pmd_supported_wan_type_bm_cb = NULL;
static configure_non_brcm_pmd_wan_type          configure_non_brcm_pmd_wan_type_cb = NULL;

#define REG_CHECK_TIMES            (15)
#define SLEEP_IN_MILI_SECONDS      (50)
#define MID_SLEEP_IN_MILI_SECONDS  (100)
#define LONG_SLEEP_IN_MILI_SECONDS (1500)

/* GPON General Config */
#define GPON_RCVR_STATUS_REG_OFFSET            (0x00)
#define GPON_RCVR_CONFIG_REG_OFFSET            (0x04)
#define GPON_RCVR_CONFIG_LOF_PARAMS_REG_OFFSET (0x08)
 
#define GPON_RCVR_STATUS_BIT_LOF                (         1)
#define GPON_RCVR_CONFIG_BIT_RX_DISABLE         (    1 << 2)
#define GPON_RCVR_CONFIG_LOF_DELTA_SHIFT        (         0)
#define GPON_RCVR_CONFIG_LOF_DELTA              (         4)
#define GPON_RCVR_CONFIG_LOF_ALPHA_SHIFT        (         4)
#define GPON_RCVR_CONFIG_LOF_ALPHA              (         1)
#define GPON_RCVR_CONFIG_LOF_PARAMS_CLEAR_MASK  (     ~0xFF)
 
/* EPON Top */
#define EPON_TOP_RESET_REG_OFFSET	(0x004)
#define EPON_TOP_CONTROL_REG_OFFSET	(0x010)
 
#define EPON_TOP_RESET_ALL              (         0)
#define EPON_TOP_LIF_RST_CLR            (         2)
#define EPON_TOP_CONTROL_2G_DS_MODE     (         4)
#define EPON_TOP_CONTROL_1G_DS_MODE     (         0)
#if defined(CONFIG_BCM96858)
#define EPON_TOP_RESET_XPCS_RX_RST_CLR          (0x00000040)
#else
#define EPON_TOP_RESET_XPCS_RX_RST_CLR          (0x00000080)
#endif
 
/* EPON LIF */
#define LIF_PON_CONTROL_REG_OFFSET	(0x000)
#define LIF_SEC_CONTROL_REG_OFFSET	(0x00c)
#define LIF_INT_STATUS_REG_OFFSET	(0x014)
#if defined(CONFIG_BCM96858)
#define LIF_RX_AGG_MPCP_FRM_REG_OFFSET	(0x100)
#else
#define LIF_RX_AGG_MPCP_FRM_REG_OFFSET	(0x0C0)
#endif
 
#define LIF_PON_CONTROL_RX_EN_NORM_FLIP (0x00100005)
#define LIF_PON_CONTROL_BIT_RX_EN       (         1)
#define LIF_SEC_CONTROL_VALUE           (0x00000004)
#define LIF_INT_STATUS_ALL_CLR          (0x003fffff)
#define LIF_INT_RX_OUT_OF_SYNCH_STAT    (         1)
 
/* EPON XPCSR */
#define XPCSRX_RST_REG_OFFSET                   (0x000)
#define XPCSRX_FRAMER_CTL_REG_OFFSET            (0x00c)
#define XPCSRX_RAM_ECC_INT_STAT_REG_OFFSET      (0x108)
#define XPCSRX_FEC_CTL_REG_OFFSET               (0x010)
#define XPCSRX_INT_STAT_REG_OFFSET              (0x004)
#define XPCSRX_64B_66B_START_CNT_REG_OFFSET     (0x0dc)
#define XPCSRX_IDLE_GOOD_PKT_CNT_REG_OFFSET     (0x0e0)
#define XPCSRX_STATUS_REG_OFFSET                (0x048)

#define XPCSRX_RST_CLR                          (         1)
#define EPON_TOP_CONTROL_10G_DS_MODE            (         1)
#define INT_RX_IDLE_RAM_INIT_DONE               (0x00000010)
#define XPCSRX_FRAMER_CTL_VALUE                 (0x00000094)
#define XPCSRX_FRAMER_CTL_FEC_VALUE             (0x00000095)
#define XPCSRX_FEC_CTL_FEC_BYPASSED             (0x00000004)
#define XPCSRX_FEC_CTL_FEC_ENABLED              (0x00000013)
#define XPCSRX_INT_STAT_CLR                     (0x7fcfffff)
#define XPCSRX_INT_STAT_RX_FRAMER_CW_LOCK       (0x00000020)
#define XPCSRX_RST_ACTIVE_LOW_RESET             (         0)
#define XPCSRX_FRAMER_CTL_DISABLE_VALUE         (0x00000090)
#define XPCSRX_STATUS_STATRXFRAMERCWLOCK        (0x00000020)

/* NGPON RX Gen */
#define NGPON_RX_GEN_RCVRSTAT_REG_OFFSET           (0x00)
#define NGPON_RX_GEN_RCVRCFG_REG_OFFSET            (0x04)
#define NGPON_RX_GEN_SYNC_CFG_REG_OFFSET           (0x08)
 
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
 
extern void *g_gpon_rcvr_base;
extern void *g_epon_top_base;
extern void *g_epon_lif_base;
extern void *g_epon_xpcsr_base;
extern void *g_ngpon_rxgen_base;
#define GPON_RCVR_ADDR(offset)		((unsigned int *)(g_gpon_rcvr_base + offset))
#define EPON_TOP_ADDR(offset)		((unsigned int *)(g_epon_top_base + offset))
#define EPON_LIF_ADDR(offset)		((unsigned int *)(g_epon_lif_base + offset))
#define EPON_XPCSR_ADDR(offset)		((unsigned int *)(g_epon_xpcsr_base + offset))
#define NGPON_RXGEN_ADDR(offset)	((unsigned int *)(g_ngpon_rxgen_base + offset))
 
static int wan_serdes_conf(serdes_wan_type_t wan_type)
{
    bcmFun_t *cb = bcmFun_get(BCM_FUN_ID_WAN_SERDES_CONFIG);
    return cb ? cb(&wan_type) : -1;
}
#define wan_serdes_config wan_serdes_conf

static int is_gpon_lof_synced(uint16_t optics_type, pmd_calibration_parameters *calibration_parameters_from_json)
{
    unsigned int ret, reg_value, is_sensed;
    volatile unsigned int *reg_address;

    ret = configure_pmd_wan_type(optics_type, PMD_GPON_2_1_WAN, calibration_parameters_from_json);
    if (SKIP_SENSING_WAN_TYPE == ret) return SENSE_RESULT_FALSE;

    wan_serdes_config(SERDES_WAN_TYPE_GPON);

    reg_address = GPON_RCVR_ADDR(GPON_RCVR_CONFIG_LOF_PARAMS_REG_OFFSET);
    reg_value = *reg_address;
    reg_value &= GPON_RCVR_CONFIG_LOF_PARAMS_CLEAR_MASK;
    reg_value |= GPON_RCVR_CONFIG_LOF_DELTA << GPON_RCVR_CONFIG_LOF_DELTA_SHIFT;
    reg_value |= GPON_RCVR_CONFIG_LOF_ALPHA << GPON_RCVR_CONFIG_LOF_ALPHA_SHIFT;
    *reg_address = reg_value;

    reg_address = GPON_RCVR_ADDR(GPON_RCVR_CONFIG_REG_OFFSET);
    reg_value = *reg_address;

    reg_value &= ~GPON_RCVR_CONFIG_BIT_RX_DISABLE;
    *reg_address = reg_value;
    msleep(SLEEP_IN_MILI_SECONDS);

    reg_value = *GPON_RCVR_ADDR(GPON_RCVR_STATUS_REG_OFFSET);
    is_sensed = (0 == (GPON_RCVR_STATUS_BIT_LOF & reg_value));
 
    reg_value= *reg_address;
    reg_value &= GPON_RCVR_CONFIG_BIT_RX_DISABLE;
    *reg_address = reg_value;    

    if (is_sensed) BCM_LOG_NOTICE(BCM_LOG_ID_WANTYPEDET, "GPON was detected\n");

    return is_sensed;
}
 
static int is_epon_ae_in_sync(SUPPORTED_WAN_TYPES_BITMAP wan_type, uint16_t optics_type, pmd_calibration_parameters
    *calibration_parameters_from_json)
{
 
    unsigned int reg_value, is_sensed;
    volatile unsigned int *reg_address;
    char *wan_type_string;
    int ret;
    int i = 0;
    
    switch (wan_type)
    {
        case SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1:
            ret = configure_pmd_wan_type(optics_type, PMD_EPON_2_1_WAN, calibration_parameters_from_json);
            break;
        case SUPPORTED_WAN_TYPES_BIT_EPON_1_1:
            ret = configure_pmd_wan_type(optics_type, PMD_EPON_1_1_WAN, calibration_parameters_from_json);
            break;
        case SUPPORTED_WAN_TYPES_BIT_AE_1_1:
            ret = configure_pmd_wan_type(optics_type, PMD_GBE_1_1_WAN, calibration_parameters_from_json);
            break;
        default:
            ret = KEEP_SENSING_WAN_TYPE;
    }

    if (SKIP_SENSING_WAN_TYPE == ret) return SENSE_RESULT_FALSE;

#if !defined(CONFIG_BCM96855)
    reg_address = EPON_TOP_ADDR(EPON_TOP_RESET_REG_OFFSET);
    *reg_address = EPON_TOP_RESET_ALL;
#endif
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
 
    reg_address = EPON_TOP_ADDR(EPON_TOP_CONTROL_REG_OFFSET);
    *reg_address = (SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1 == wan_type) ? EPON_TOP_CONTROL_2G_DS_MODE : EPON_TOP_CONTROL_1G_DS_MODE;
 
    reg_address = EPON_TOP_ADDR(EPON_TOP_RESET_REG_OFFSET);
    *reg_address = EPON_TOP_LIF_RST_CLR;
    msleep(SLEEP_IN_MILI_SECONDS);
 
    reg_address = EPON_LIF_ADDR(LIF_PON_CONTROL_REG_OFFSET);
    *reg_address = LIF_PON_CONTROL_RX_EN_NORM_FLIP;
 
    reg_address = EPON_LIF_ADDR(LIF_SEC_CONTROL_REG_OFFSET);
    *reg_address = LIF_SEC_CONTROL_VALUE;
    msleep(SLEEP_IN_MILI_SECONDS);

    /*
    serdes may periodicity relock which will make lif raise intRxOutOfSynchStat.
    here use multiple short-term detections to avoid the influence of serdes relock action.
    */
    for (i= 0; i < REG_CHECK_TIMES; i++)
    {
        reg_address = EPON_LIF_ADDR(LIF_INT_STATUS_REG_OFFSET);
        *reg_address = LIF_INT_STATUS_ALL_CLR;
        msleep(MID_SLEEP_IN_MILI_SECONDS);

        reg_value = *EPON_LIF_ADDR(LIF_INT_STATUS_REG_OFFSET);
        is_sensed = (0 == (LIF_INT_RX_OUT_OF_SYNCH_STAT & reg_value));

        if (is_sensed && (SUPPORTED_WAN_TYPES_BIT_EPON_1_1 == wan_type))
        {
            reg_value = *EPON_LIF_ADDR(LIF_RX_AGG_MPCP_FRM_REG_OFFSET);
            is_sensed = reg_value > 0;
        }

        if (is_sensed)
        {
            break;
        }
    }
 
    reg_address = EPON_LIF_ADDR(LIF_PON_CONTROL_REG_OFFSET);
    reg_value= *reg_address;
    reg_value &= ~LIF_PON_CONTROL_BIT_RX_EN;
    *reg_address = reg_value;

    if (is_sensed) BCM_LOG_NOTICE(BCM_LOG_ID_WANTYPEDET, "%s was detected\n", wan_type_string);
    
    return is_sensed;
}
 
static void disable_10g_epon_ae_mac(void)
{
    volatile unsigned int *reg_address;
 
    reg_address = EPON_XPCSR_ADDR(XPCSRX_FRAMER_CTL_REG_OFFSET);
    *reg_address = XPCSRX_FRAMER_CTL_DISABLE_VALUE;
 
    reg_address = EPON_XPCSR_ADDR(XPCSRX_RST_REG_OFFSET);
    *reg_address = XPCSRX_RST_ACTIVE_LOW_RESET;
 
    reg_address = EPON_TOP_ADDR(EPON_TOP_RESET_REG_OFFSET);
    *reg_address = EPON_TOP_RESET_ALL;
}
 
static int is_10g_epon_ae_in_sync(SUPPORTED_WAN_TYPES_BITMAP wan_type, uint16_t optics_type, pmd_calibration_parameters
    *calibration_parameters_from_json)
{
    unsigned int reg_value, is_sensed, is_locked;
    volatile unsigned int *reg_address;
    char *wan_type_string;
    int ret;

    switch (wan_type)
    {
        case SUPPORTED_WAN_TYPES_BIT_EPON_10_1:
            ret = configure_pmd_wan_type(optics_type, PMD_EPON_10_1_WAN, calibration_parameters_from_json);
            break;
        default:
            ret = KEEP_SENSING_WAN_TYPE;
    }

    if (SKIP_SENSING_WAN_TYPE == ret) return SENSE_RESULT_FALSE;

    if(EPON_XPCSR_ADDR(0) == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_WANTYPEDET, "Error: EPON XPCSR address was not initialized, check board Device tree\n", wan_type);
        return SENSE_RESULT_FALSE;
    }

#if !defined(CONFIG_BCM96855)
    reg_address = EPON_TOP_ADDR(EPON_TOP_RESET_REG_OFFSET);
    *reg_address = EPON_TOP_RESET_ALL;

    reg_address = EPON_XPCSR_ADDR(XPCSRX_RST_REG_OFFSET);
    *reg_address = XPCSRX_RST_ACTIVE_LOW_RESET;
#endif

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

    reg_address = EPON_TOP_ADDR(EPON_TOP_CONTROL_REG_OFFSET);
    *reg_address = EPON_TOP_CONTROL_10G_DS_MODE;

    reg_address = EPON_TOP_ADDR(EPON_TOP_RESET_REG_OFFSET);
    *reg_address = EPON_TOP_RESET_XPCS_RX_RST_CLR;

    msleep(SLEEP_IN_MILI_SECONDS);

    reg_address = EPON_XPCSR_ADDR(XPCSRX_FEC_CTL_REG_OFFSET);
    *reg_address = XPCSRX_FEC_CTL_FEC_BYPASSED;

    reg_address = EPON_XPCSR_ADDR(XPCSRX_RST_REG_OFFSET);
    *reg_address = XPCSRX_RST_CLR;

    msleep(SLEEP_IN_MILI_SECONDS);
	
    reg_address = EPON_XPCSR_ADDR(XPCSRX_FRAMER_CTL_REG_OFFSET);
    *reg_address = XPCSRX_FRAMER_CTL_VALUE;

    reg_address = EPON_XPCSR_ADDR(XPCSRX_INT_STAT_REG_OFFSET);
    *reg_address = XPCSRX_INT_STAT_CLR;

    msleep(LONG_SLEEP_IN_MILI_SECONDS);
#if defined(CONFIG_BCM96855)
    reg_value = *EPON_XPCSR_ADDR(XPCSRX_STATUS_REG_OFFSET);
    is_locked = ((reg_value & XPCSRX_STATUS_STATRXFRAMERCWLOCK) == XPCSRX_STATUS_STATRXFRAMERCWLOCK);
#else
    reg_value = *reg_address;
    is_locked = ((reg_value & XPCSRX_INT_STAT_RX_FRAMER_CW_LOCK) == XPCSRX_INT_STAT_RX_FRAMER_CW_LOCK);
#endif

    if (is_locked)
    {
        reg_value = *EPON_XPCSR_ADDR(XPCSRX_64B_66B_START_CNT_REG_OFFSET);
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
        reg_address = EPON_XPCSR_ADDR(XPCSRX_RST_REG_OFFSET);
        *reg_address = XPCSRX_RST_ACTIVE_LOW_RESET;

        msleep(SLEEP_IN_MILI_SECONDS);

        reg_address = EPON_XPCSR_ADDR(XPCSRX_FEC_CTL_REG_OFFSET);
        *reg_address = XPCSRX_FEC_CTL_FEC_ENABLED;

        reg_address = EPON_XPCSR_ADDR(XPCSRX_RST_REG_OFFSET);
        *reg_address = XPCSRX_RST_CLR;

        msleep(SLEEP_IN_MILI_SECONDS);

        reg_address = EPON_XPCSR_ADDR(XPCSRX_FRAMER_CTL_REG_OFFSET);
        *reg_address = XPCSRX_FRAMER_CTL_FEC_VALUE;

        reg_address = EPON_XPCSR_ADDR(XPCSRX_INT_STAT_REG_OFFSET);
        *reg_address = XPCSRX_INT_STAT_CLR;

        msleep(LONG_SLEEP_IN_MILI_SECONDS);
        reg_value = *reg_address;
 
        if ((reg_value & XPCSRX_INT_STAT_RX_FRAMER_CW_LOCK) == XPCSRX_INT_STAT_RX_FRAMER_CW_LOCK)
        {
            reg_value = *EPON_XPCSR_ADDR(XPCSRX_IDLE_GOOD_PKT_CNT_REG_OFFSET);
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


static int is_10g_itu_pon_frame_sync(SUPPORTED_WAN_TYPES_BITMAP wan_type, uint16_t optics_type,
    pmd_calibration_parameters *calibration_parameters_from_json)
{
    unsigned int reg_value, is_sensed, mac_mode;
    volatile unsigned int *reg_address;
    char *wan_type_string;
    int ret;

    switch (wan_type)
    {
        case SUPPORTED_WAN_TYPES_BIT_XGPON:
            ret = configure_pmd_wan_type(optics_type, PMD_XGPON1_10_2_WAN, calibration_parameters_from_json);
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
            __attribute__ ((__fallthrough__));
        case SUPPORTED_WAN_TYPES_BIT_XGSPON:
            mac_mode = GPON_RX_GEN_RCVRCFG_MAC_MODE_NGPON2_10G;
            wan_serdes_config(SERDES_WAN_TYPE_NGPON_10G_10G);
            if (SUPPORTED_WAN_TYPES_BIT_XGSPON == wan_type) wan_type_string = "XGSPON";
            break;
        default:
            return SENSE_RESULT_FALSE;
    }
    
    msleep(SLEEP_IN_MILI_SECONDS);
 
    if(NGPON_RXGEN_ADDR(0) == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_WANTYPEDET, "Error: NGPON Rx Gen address was not initialized, check board Device tree\n", wan_type);
        return SENSE_RESULT_FALSE;
    }
 
    reg_address = NGPON_RXGEN_ADDR(NGPON_RX_GEN_RCVRCFG_REG_OFFSET);
    *reg_address = GPON_RX_GEN_RCVRCFG_MAC_MODE_RESET;
    
    msleep(SLEEP_IN_MILI_SECONDS);
 
    reg_address = NGPON_RXGEN_ADDR(NGPON_RX_GEN_SYNC_CFG_REG_OFFSET);
    reg_value = *reg_address;
    reg_value &= NGPON_RX_GEN_SYNC_CFG_CLEAR_MASK;
    reg_value |= NGPON_RX_GEN_SYNC_CFG_PONID_ACQ_THR << NGPON_RX_GEN_SYNC_CFG_PONID_ACQ_THR_SHIFT;
    reg_value |= NGPON_RX_GEN_SYNC_CFG_SYNC_ACQ_THR  << NGPON_RX_GEN_SYNC_CFG_SYNC_ACQ_THR_SHIFT;
    reg_value |= NGPON_RX_GEN_SYNC_CFG_SYNC_LOSS_THR << NGPON_RX_GEN_SYNC_CFG_SYNC_LOSS_THR_SHIFT;
    *reg_address = reg_value;

    msleep(SLEEP_IN_MILI_SECONDS);
 
    reg_address = NGPON_RXGEN_ADDR(NGPON_RX_GEN_RCVRCFG_REG_OFFSET);
    reg_value = *reg_address;

    reg_value &= ~(NGPON_RX_GEN_RCVRCFG_MAC_MODE_MASK << NGPON_RX_GEN_RCVRCFG_MAC_MODE_SHIFT);
    reg_value |= mac_mode << NGPON_RX_GEN_RCVRCFG_MAC_MODE_SHIFT;
    *reg_address = reg_value;

    msleep(SLEEP_IN_MILI_SECONDS);

    reg_value |= NGPON_RX_GEN_RCVRCFG_BIT_RX_ENABLE;
    *reg_address = reg_value;

    msleep(LONG_SLEEP_IN_MILI_SECONDS);
    
    reg_value = *NGPON_RXGEN_ADDR(NGPON_RX_GEN_RCVRSTAT_REG_OFFSET);
    is_sensed = NGPON_RX_GEN_RCVRSTAT_BIT_FRAME_SYNC_STATE & reg_value;
    
    reg_value= *reg_address;
    reg_value &= ~(NGPON_RX_GEN_RCVRCFG_MAC_MODE_MASK << NGPON_RX_GEN_RCVRCFG_MAC_MODE_SHIFT);
    reg_value &= ~NGPON_RX_GEN_RCVRCFG_BIT_RX_ENABLE;
    *reg_address = reg_value;
    
    if (is_sensed) BCM_LOG_NOTICE(BCM_LOG_ID_WANTYPEDET, "%s was detected\n", wan_type_string);

    return is_sensed;
}
 
static int configure_pmd_wan_type(uint16_t optics_type, PMD_WAN_TYPES new_pmd_wan_type, pmd_calibration_parameters
    *calibration_parameters_from_json)
{
#ifdef CONFIG_BCM_PMD     
    int rc;
#endif    
    
    if (BCM_I2C_PON_OPTICS_TYPE_NON_BRCM_PMD == optics_type)
    {
        configure_non_brcm_pmd_wan_type_cb(new_pmd_wan_type);
        return KEEP_SENSING_WAN_TYPE;        
    }

    if (BCM_I2C_PON_OPTICS_TYPE_PMD != optics_type)
    {
        return KEEP_SENSING_WAN_TYPE;
    }
#ifdef CONFIG_BCM_PMD     
    rc = pmd_dev_reconfigure_wan_type(new_pmd_wan_type, calibration_parameters_from_json);
    if (rc)
    {
        return SKIP_SENSING_WAN_TYPE;
    }
#endif    
    return KEEP_SENSING_WAN_TYPE;
}

static int power_wan_type(SUPPORTED_WAN_TYPES_BITMAP wan_type, int enabled)
{
#if defined(CONFIG_BRCM_SMC_BOOT)
    int bp3_feature, wan_intf;

    switch (wan_type) {
    case SUPPORTED_WAN_TYPES_BIT_GPON:
    {
        bp3_feature = BP3_FEATURE_GPON;
        wan_intf = WAN_INTF_GPON;
        break;
    }
    case SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1:
    case SUPPORTED_WAN_TYPES_BIT_EPON_1_1:
    {
        bp3_feature = BP3_FEATURE_EPON;
        wan_intf = WAN_INTF_EPON;
        break;
    }
    case SUPPORTED_WAN_TYPES_BIT_AE_1_1:
    {
        bp3_feature = BP3_FEATURE_ACTIVE_ETH_2_5;
        wan_intf = WAN_INTF_AE;
        break;
    }
    case SUPPORTED_WAN_TYPES_BIT_EPON_10_10:
    case SUPPORTED_WAN_TYPES_BIT_EPON_10_1:
    {
        bp3_feature = BP3_FEATURE_10G_EPON;
        wan_intf = WAN_INTF_10GEPON;
        break;
    }
    case SUPPORTED_WAN_TYPES_BIT_AE_10_10:
    {
        bp3_feature = BP3_FEATURE_ACTIVE_ETH_10;
        wan_intf = WAN_INTF_AE;
        break;
    }
    case SUPPORTED_WAN_TYPES_BIT_XGPON:
    case SUPPORTED_WAN_TYPES_BIT_NGPON2_10_25:
    case SUPPORTED_WAN_TYPES_BIT_NGPON2_10_10:
    case SUPPORTED_WAN_TYPES_BIT_XGSPON:
    {
        bp3_feature = BP3_FEATURE_10G_GPON;
        wan_intf = WAN_INTF_XGPON;
        break;
    }
    default:
        return -1;
    }

    if (bcm_license_check_msg(bp3_feature) <= 0)
        return -1;

    if (pmc_wan_interface_power_control(wan_intf, enabled))
        return -1;
#endif

    return 0;
}

static int is_specific_wan_type_sensed(SUPPORTED_WAN_TYPES_BITMAP wan_type, uint16_t optics_type,
    pmd_calibration_parameters *calibration_parameters_from_json)
{
    int ret = SENSE_RESULT_FALSE;

    if (power_wan_type(wan_type, 1))
        goto Exit;

    switch (wan_type)
    {
        case SUPPORTED_WAN_TYPES_BIT_GPON:
            ret = is_gpon_lof_synced(optics_type, calibration_parameters_from_json);
            break;
        case SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1:
        case SUPPORTED_WAN_TYPES_BIT_EPON_1_1:
#if defined (CONFIG_BCM96858) || defined (CONFIG_BCM96856) || defined(CONFIG_BCM96855) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878)
        case SUPPORTED_WAN_TYPES_BIT_AE_1_1:
#endif
            ret = is_epon_ae_in_sync(wan_type, optics_type, calibration_parameters_from_json);
            break;
        case SUPPORTED_WAN_TYPES_BIT_EPON_10_10:
        case SUPPORTED_WAN_TYPES_BIT_EPON_10_1:
#if defined (CONFIG_BCM96858) || defined (CONFIG_BCM96856) || defined(CONFIG_BCM96855) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878)
        case SUPPORTED_WAN_TYPES_BIT_AE_10_10:
#endif
            ret = is_10g_epon_ae_in_sync(wan_type, optics_type, calibration_parameters_from_json);
            break;
        case SUPPORTED_WAN_TYPES_BIT_XGPON:
        case SUPPORTED_WAN_TYPES_BIT_NGPON2_10_25:
        case SUPPORTED_WAN_TYPES_BIT_NGPON2_10_10:
        case SUPPORTED_WAN_TYPES_BIT_XGSPON:
            ret = is_10g_itu_pon_frame_sync(wan_type, optics_type, calibration_parameters_from_json);
            break;
#if defined (CONFIG_BCM_MPCS)
        case SUPPORTED_WAN_TYPES_BIT_AE_1_1:
        case SUPPORTED_WAN_TYPES_BIT_AE_10_10:
            ret = 1; /* Detect AE signal */
            break;
#endif
        default:
            BCM_LOG_ERROR(BCM_LOG_ID_WANTYPEDET, "Error: Can't detect 0x%x WAN type\n", wan_type);
            break;
    }

    if (power_wan_type(wan_type, 0))
        goto Exit;

Exit:
    return ret;
}


static int is_wan_type_bit_set_in_bitmap(SUPPORTED_WAN_TYPES_BITMAP supported_wan_type_bit, SUPPORTED_WAN_TYPES_BITMAP
    supported_wan_type_bm)
{
    return (supported_wan_type_bit & supported_wan_type_bm);
}

static int is_only_single_wan_type_bit_set_in_bitmap(SUPPORTED_WAN_TYPES_BITMAP supported_wan_type_bit,
    SUPPORTED_WAN_TYPES_BITMAP supported_wan_type_bm)
{
    return (supported_wan_type_bit == supported_wan_type_bm);
}

static int do_sensing(SUPPORTED_WAN_TYPES_BITMAP supported_wan_type_bit,
    SUPPORTED_WAN_TYPES_BITMAP supported_wan_type_bm, uint16_t optics_type,
    pmd_calibration_parameters *calibration_parameters_from_json, int signal_detect_required)
{
    if (!is_wan_type_bit_set_in_bitmap(supported_wan_type_bit, supported_wan_type_bm))
        return SENSE_RESULT_FALSE;
    if (!signal_detect_required &&
        is_only_single_wan_type_bit_set_in_bitmap(supported_wan_type_bit, supported_wan_type_bm))
        return SENSE_RESULT_TRUE;
    if (is_specific_wan_type_sensed(supported_wan_type_bit, optics_type, calibration_parameters_from_json))
        return SENSE_RESULT_TRUE;

    return SENSE_RESULT_FALSE;
}


int try_wan_type_sensing(pmd_calibration_parameters *calibration_parameters_from_json,
    int signal_detect_required,
    wan_type_auto_detect_result *result)
{
    int ret = 0;
    SUPPORTED_WAN_TYPES_BITMAP supported_wan_type_bm;
    uint16_t optics_type;

    ret = get_optics_type(&optics_type);
    if (ret)
    {
        return ret;
    }

    ret = get_supported_wan_type_bm(&supported_wan_type_bm);
    if (OPTICALDET_SUCCESS != ret)
    {
        return ret;
    }
    BCM_LOG_DEBUG(BCM_LOG_ID_WANTYPEDET, "supported_wan_type_bm = 0x%x\n", supported_wan_type_bm);

#define SPECIFIC_TYPE_SENSING(wan_type_bit) do_sensing(wan_type_bit, \
    supported_wan_type_bm, optics_type, calibration_parameters_from_json, signal_detect_required)
#define SET_WAN_DETECT_RESULT(token, value) do    \
    {    \
        memcpy(token, value, strlen(value)); \
    } while (0)

    if (SPECIFIC_TYPE_SENSING(SUPPORTED_WAN_TYPES_BIT_GPON))
    {
        SET_WAN_DETECT_RESULT(result->wan_type, RDPA_WAN_TYPE_VALUE_GPON);
        SET_WAN_DETECT_RESULT(result->wan_rate, "0101");
        return ret;
    }
    if (SPECIFIC_TYPE_SENSING(SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1))
    {   /* Notice that EPON 2/1 is checked before EPON 1/1 on purpose to allow TURBO mode when possible */
        SET_WAN_DETECT_RESULT(result->wan_type, RDPA_WAN_TYPE_VALUE_EPON);
        SET_WAN_DETECT_RESULT(result->wan_rate, "2501");
        return ret;
    }
    if (SPECIFIC_TYPE_SENSING(SUPPORTED_WAN_TYPES_BIT_EPON_1_1))
    {
        SET_WAN_DETECT_RESULT(result->wan_type, RDPA_WAN_TYPE_VALUE_EPON);
        SET_WAN_DETECT_RESULT(result->wan_rate, "0101");
        return ret;
    }
    if (SPECIFIC_TYPE_SENSING(SUPPORTED_WAN_TYPES_BIT_XGSPON))
    {
        SET_WAN_DETECT_RESULT(result->wan_type, RDPA_WAN_TYPE_VALUE_XGS);
        SET_WAN_DETECT_RESULT(result->wan_rate, "1010");
        return ret;
    }
    if (SPECIFIC_TYPE_SENSING(SUPPORTED_WAN_TYPES_BIT_EPON_10_1))
    {
        SET_WAN_DETECT_RESULT(result->wan_type, RDPA_WAN_TYPE_VALUE_EPON);
        SET_WAN_DETECT_RESULT(result->wan_rate, "1001");
        return ret;
    }
    if (SPECIFIC_TYPE_SENSING(SUPPORTED_WAN_TYPES_BIT_EPON_10_10))
    {
        SET_WAN_DETECT_RESULT(result->wan_type, RDPA_WAN_TYPE_VALUE_EPON);
        SET_WAN_DETECT_RESULT(result->wan_rate, "1010");
        return ret;
    }
    if (SPECIFIC_TYPE_SENSING(SUPPORTED_WAN_TYPES_BIT_AE_1_1))
    {
        SET_WAN_DETECT_RESULT(result->wan_type, RDPA_WAN_TYPE_VALUE_GBE);
        SET_WAN_DETECT_RESULT(result->wan_rate, "0101");
        return ret;
    }
    if (SPECIFIC_TYPE_SENSING(SUPPORTED_WAN_TYPES_BIT_AE_10_10))
    {
        SET_WAN_DETECT_RESULT(result->wan_type, RDPA_WAN_TYPE_VALUE_GBE);
        SET_WAN_DETECT_RESULT(result->wan_rate, "1010");
        return ret;
    }
    if (SPECIFIC_TYPE_SENSING(SUPPORTED_WAN_TYPES_BIT_XGPON))
    {
        SET_WAN_DETECT_RESULT(result->wan_type, RDPA_WAN_TYPE_VALUE_XGPON1);
        return ret;
    }
    if (SPECIFIC_TYPE_SENSING(SUPPORTED_WAN_TYPES_BIT_NGPON2_10_25))
    {
        SET_WAN_DETECT_RESULT(result->wan_type, RDPA_WAN_TYPE_VALUE_NGPON2);
        SET_WAN_DETECT_RESULT(result->wan_rate, "1025");
        return ret;
    }
    if (SPECIFIC_TYPE_SENSING(SUPPORTED_WAN_TYPES_BIT_NGPON2_10_10))
    {
        SET_WAN_DETECT_RESULT(result->wan_type, RDPA_WAN_TYPE_VALUE_NGPON2);
        SET_WAN_DETECT_RESULT(result->wan_rate, "1010");
        return ret;
    }

    BCM_LOG_ERROR(BCM_LOG_ID_WANTYPEDET, "Error: Can't detect any supported WAN type at MAC layer\n");
    return ret;
}


static int get_optics_type(uint16_t *optics_type)
{
    int ret;

    if (get_non_brcm_pmd_supported_wan_type_bm_cb || configure_non_brcm_pmd_wan_type_cb)
    {
        *optics_type = BCM_I2C_PON_OPTICS_TYPE_NON_BRCM_PMD;
        return OPTICALDET_SUCCESS;
    }

    ret = trxbus_is_pmd(wan_i2c_bus_get());
    if (ret < 0)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_WANTYPEDET,
            "Error: The PMD board profile is not configured and the optics type cannot be determined\n");
        return -1;
    }

    if (ret > 0)
        *optics_type = BCM_I2C_PON_OPTICS_TYPE_PMD;
    else 
        *optics_type = BCM_I2C_PON_OPTICS_TYPE_LEGACY;

    return OPTICALDET_SUCCESS;
}

static int get_supported_wan_type_bm(SUPPORTED_WAN_TYPES_BITMAP *wan_type_bm)
{
    int ret, bus;

    if (get_non_brcm_pmd_supported_wan_type_bm_cb)
    {
        get_non_brcm_pmd_supported_wan_type_bm_cb(wan_type_bm);
        return OPTICALDET_SUCCESS;
    }

    bus = wan_i2c_bus_get();
    if (bus < 0)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_WANTYPEDET, "Error: Can't get optical device i2c bus number\n");
        return bus;
    }
    ret = trx_get_supported_wan_type_bm(bus, wan_type_bm);
    if (OPTICALDET_SUCCESS != ret)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_WANTYPEDET, "Error: No optical device detection\n");
    }

    return ret;
}


void register_non_brcm_pmd(get_non_brcm_pmd_supported_wan_type_bm get_bitmap_callback, configure_non_brcm_pmd_wan_type
    configure_callback)
{
    get_non_brcm_pmd_supported_wan_type_bm_cb = get_bitmap_callback;
    configure_non_brcm_pmd_wan_type_cb = configure_callback;
}
EXPORT_SYMBOL(register_non_brcm_pmd);
