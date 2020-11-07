/*
  <:copyright-BRCM:2015:proprietary:standard
  
     Copyright (c) 2015 Broadcom 
     All Rights Reserved
  
   This program is the proprietary software of Broadcom and/or its
   licensors, and may only be used, duplicated, modified or distributed pursuant
   to the terms and conditions of a separate, written license agreement executed
   between you and Broadcom (an "Authorized License").  Except as set forth in
   an Authorized License, Broadcom grants no license (express or implied), right
   to use, or waiver of any kind with respect to the Software, and Broadcom
   expressly reserves all rights in and to the Software and all intellectual
   property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
   NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
   BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  
   Except as expressly set forth in the Authorized License,
  
   1. This program, including its structure, sequence and organization,
      constitutes the valuable trade secrets of Broadcom, and you shall use
      all reasonable efforts to protect the confidentiality thereof, and to
      use this information only in connection with your use of Broadcom
      integrated circuit products.
  
   2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
      AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
      WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
      RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
      ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
      FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
      COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
      TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
      PERFORMANCE OF THE SOFTWARE.
  
   3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
      ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
      INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
      WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
      IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
      OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
      SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
      SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
      LIMITED REMEDY.
  :>
*/

/****************************************************************************
 *
 * pon_drv.c -- Bcm Pon driver
 *
 * Description:
 *      This file contains BCM Wan Serdes, Wan Top, Wan Gearbox driver
 *
 * Authors: Fuguo Xu, Akiva Sadovski
 *
 * $Revision: 1.2 $
 *
 * $Id: pon_drv.c,v 1.1 2015/12/21 Fuguo Exp $
 *
 * $Log: pon_drv.c,v $
 * Revision 1.1  2015/12/21 Fuguo
 * Initial version.
 * Revision 1.2  2017/7 VZ, MJ
 * Update
 *
 ****************************************************************************/

#include <linux/delay.h>
#include "pmc_drv.h"
#include "pmc_wan.h"
#include "bcm_pinmux.h"
#include "bcm_gpio.h"
#include "board.h"
#include "wan_drv.h"
#include "pon_drv.h"
#include <bcmsfp_i2c.h>
#include "gpon_ag.h"
#include "opticaldet.h"
#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE)
#include "ngpon_gearbox_ag.h"
#endif
#include "ru_types.h"
#include "bcm_map_part.h"

#include "eagle_onu10g_interface.h"
#include "eagle_onu10g_ucode_image.h"
#include "eagle_onu10g_functions.c"
#include "tod_ag.h"
#include "misc_ag.h"
#include "wan_serdes_ag.h"
#include "serdes_status_ag.h"
#include "top_scratch_ag.h"
#include "pmi_ag.h"
#include "pon_drv_serdes_util.h"
#include "ru.h"
#include "pon_drv_gpon_init.h"
#include "early_txen_ag.h"

#define TX_LASER_ON_OUT_P  53
#define TX_LASER_ON_OUT_N  54
#define A_WAN_EARLY_TXEN   55
#define B_WAN_EARLY_TXEN   56

#define FIRMWARE_READY_TIMEOUT_MS 1000

typedef struct wan_info_s
{
    char *name;
} wan_info_t;

static uint8_t *serdes_ucode = eagle_onu10g_ucode_image;
static uint16_t serdes_ucode_len = EAGLE_ONU10G_UCODE_IMAGE_SIZE;
static serdes_wan_type_t wan_serdes_type = SERDES_WAN_TYPE_NONE;
uint16_t optics_type;
static PMD_DEV_ENABLE_PRBS pmd_prbs_callback;

extern const ru_block_rec *WAN_TOP_BLOCKS[];

void remap_ru_block_addrs(uint32_t block_index, const ru_block_rec *ru_blocks[]);

static wan_info_t wan_info [] =
{
    [SERDES_WAN_TYPE_NONE]                  = {.name="Unknown Wan type"},
    [SERDES_WAN_TYPE_GPON]                  = {.name="GPON"},
    [SERDES_WAN_TYPE_EPON_1G]               = {.name="EPON_1G"},
    [SERDES_WAN_TYPE_EPON_2G]               = {.name="EPON_2G"},
    [SERDES_WAN_TYPE_AE]                    = {.name="AE"},
    [SERDES_WAN_TYPE_AE_2_5G]               = {.name="AE_2_5G"},
    [SERDES_WAN_TYPE_AE_10G]                = {.name="AE_10G"},
    [SERDES_WAN_TYPE_EPON_10G_SYM]          = {.name="EPON_10G_SYM"},
    [SERDES_WAN_TYPE_EPON_10G_ASYM]         = {.name="EPON_10G_ASYM"},
    [SERDES_WAN_TYPE_XGPON_10G_2_5G]        = {.name="XGPON_10G_2_5G"},
    [SERDES_WAN_TYPE_NGPON_10G_10G]         = {.name="NGPON_10G_10G"},
    [SERDES_WAN_TYPE_NGPON_10G_10G_8B10B]   = {.name="NGPON_10G_10G_8B10B"},
    [SERDES_WAN_TYPE_NGPON_10G_2_5G_8B10B]  = {.name="NGPON_10G_2_5G_8B10B"},
    [SERDES_WAN_TYPE_NGPON_10G_2_5G]        = {.name="NGPON_10G_2_5G"},
};


static int i2c_sfp_cb(struct notifier_block *nb, unsigned long action, void *data);
static struct notifier_block i2c_sfp_nb = {
    .notifier_call = i2c_sfp_cb,
};

static int i2c_sfp_cb(struct notifier_block *nb, unsigned long action, void *data)
{
    serdes_wan_type_t wan_type = wan_serdes_type_get();

    if ((action == SFP_STATUS_REMOVED) && 
        (wan_type != SERDES_WAN_TYPE_AE) &&
        (wan_type != SERDES_WAN_TYPE_AE_2_5G) &&
        (wan_type != SERDES_WAN_TYPE_AE_10G))
    {
        bcm_i2c_optics_tx_control(BCM_I2C_OPTICS_DISABLE);
    }

    return NOTIFY_OK;
}

err_code_t serdes_poll_uc_dsc_ready_for_cmd(uint32_t timeout_ms)
{
    int32_t loop;
    uint8_t ready_for_cmd;
    uint8_t error_found;
    uint8_t gp_uc_req;
    uint8_t supp_info;
    uint8_t state;

    /* read quickly for 10 tries */
    for (loop = 0; loop < 100; loop++)
    {
        ESTM(ready_for_cmd = rd_uc_dsc_ready_for_cmd());
        if (ready_for_cmd)
        {
            ESTM(error_found = rd_uc_dsc_error_found());
            if (error_found)
            {
                ESTM(gp_uc_req = rd_uc_dsc_gp_uc_req());
                ESTM(supp_info = rd_uc_dsc_supp_info());
                __logError("DSC command returned error (after cmd) cmd = x%x, supp_info = x%x.", gp_uc_req, supp_info);
                return(_error(ERR_CODE_UC_CMD_RETURN_ERROR));
            }
            return (ERR_CODE_NONE);
        }
        if (loop > 10)
            udelay(10 * timeout_ms);
    }
    __logError("DSC ready for command is not working, applying workaround and getting debug info!");
    ESTM(gp_uc_req = rd_uc_dsc_gp_uc_req());
    ESTM(supp_info = rd_uc_dsc_supp_info());
    ESTM(state = rd_dsc_state());
    __logError("DSC: supp_info = x%x\nDSC: gp_uc_req = x%x\nDSC: state = x%x", supp_info, gp_uc_req, state);
    /* artifically terminate the command to re-enable the command interface */
    wr_uc_dsc_ready_for_cmd(0x1);
    return (_error(ERR_CODE_POLLING_TIMEOUT));
}

static err_code_t serdes_firmware_download(void)
{
    err_code_t err = ERR_CODE_NONE;

    __logInfo("ucode_len=%d", serdes_ucode_len);

    eagle_onu10g_uc_reset(1);

    /* Load a new firmware. */
    err = eagle_onu10g_ucode_mdio_load(serdes_ucode, serdes_ucode_len);
    if (err != ERR_CODE_NONE)
    {
        __logError("Firmware download failed");
        return err;
    }
    __logInfo("Firmware download succeeded\nFirmware verify ...");

    eagle_onu10g_delay_us(500);

    err = eagle_onu10g_ucode_load_verify(serdes_ucode, serdes_ucode_len);
    if (err != ERR_CODE_NONE)
    {
        __logError("Firmware verify failed");
        return err;
    }
    __logInfo("Firmware verify ok");

    /* Activate the firmware.*/
    eagle_onu10g_uc_active_enable(1);

    /* De-assert micro reset */
    eagle_onu10g_uc_reset(0);

    /* Wait until the firmware is executing (ready). */
    err = serdes_poll_uc_dsc_ready_for_cmd(FIRMWARE_READY_TIMEOUT_MS);
    if (err != ERR_CODE_NONE)
    {
        __logError("Firmware not ready.");
        return err;
    }

    __logInfo("Firmware is ready");
    return ERR_CODE_NONE;
}

void wan_serdes_temp_init(void)
{
#if defined (CONFIG_BCM96878)
/* map temp range -40C..125C --> 924..588 for TMONT register, w/o fp.  eg 0C --> 843,  25C --> 0x318 */
#define SLOPE   (588-924) / (125000 - (-40000))      /* do NOT surround with (), for precision */
#define X0      (843)
    int32_t ret = -1, temp = 25000, adc = 0;

    ret = GetPVT(kTEMPERATURE, 0, &adc);
    if (ret)
    {
        __logError("\n Failed to get RAIL 0 die temperature, ret=%d\n", ret);
    }
    temp = pmc_convert_pvtmon(kTEMPERATURE, adc);      /* milliCelsius */
    __logDebug("\n DieTemp: %d.%03dC\n", temp / 1000, abs(temp) % 1000);
    temp = X0 + temp * SLOPE;
    RU_FIELD_WRITE(0, TOP_OSR, TEMP_CTL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES, WAN_TEMPERATURE_READ    ),  temp);
    mdelay(1);
    RU_FIELD_WRITE(0, TOP_OSR, TEMP_CTL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES, WAN_TEMPERATURE_STROBE  ),  1);
    mdelay(1);
    RU_FIELD_WRITE(0, TOP_OSR, TEMP_CTL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES, WAN_TEMPERATURE_STROBE  ),  0);
    mdelay(1);
#endif
}

static void pon_lof_handler(void)
{
    wr_cdr_freq_override_val(0x690);
    wr_cdr_freq_override_en(1);
    wr_cdr_freq_override_en(0);
#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE)
    ngpon_wan_top_reset_gearbox_rx();
#endif
}

static void data_polarity_invert(void)
{
    int32_t rc;
    unsigned short polarity;

    /* rx_pmd_dp_invert */
    rc = BpGetPmdInvSerdesRxPol(&polarity);
    if ((rc == BP_SUCCESS) && (polarity == pmd_polarity_invert))
        wr_rx_pmd_dp_invert(1);

    /* tx_pmd_dp_invert */
    rc = BpGetPmdInvSerdesTxPol(&polarity);
    if ((rc == BP_SUCCESS) && (polarity == pmd_polarity_invert))
        wr_tx_pmd_dp_invert(1);
}

static void config_wan_ewake(uint32_t toff_time, uint32_t setup_time, uint32_t hold_time)
{
    early_txen_txen txen;

    ag_drv_early_txen_txen_get(&txen);
    txen.cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass = 0;
    txen.cr_xgwan_top_wan_misc_early_txen_cfg_toff_time = toff_time;
    txen.cr_xgwan_top_wan_misc_early_txen_cfg_setup_time = setup_time;
    txen.cr_xgwan_top_wan_misc_early_txen_cfg_hold_time = hold_time;
    ag_drv_early_txen_txen_set(&txen);
}

static void wan_top_tod_cfg(serdes_wan_type_t wan_type)
{
    tod_config_0 config_0 = {};

    /* WAN source select */
    switch (wan_type)
    {
        case SERDES_WAN_TYPE_GPON:
        case SERDES_WAN_TYPE_NONE:
            config_0.cfg_ts48_mac_select = 0x2;
            break;

        case SERDES_WAN_TYPE_EPON_1G:
        case SERDES_WAN_TYPE_EPON_2G:
            config_0.cfg_ts48_mac_select = 0x0;
            break;

        case SERDES_WAN_TYPE_EPON_10G_ASYM:
        case SERDES_WAN_TYPE_EPON_10G_SYM:
            config_0.cfg_ts48_mac_select = 0x1;
            break;

        case SERDES_WAN_TYPE_XGPON_10G_2_5G:
        case SERDES_WAN_TYPE_NGPON_10G_10G:
        case SERDES_WAN_TYPE_NGPON_10G_2_5G:
#if defined(CONFIG_BCM963158)
            config_0.cfg_ts48_mac_select = 0x2;
#else
            config_0.cfg_ts48_mac_select = 0x3;
#endif
            break;

        case SERDES_WAN_TYPE_AE:
        case SERDES_WAN_TYPE_AE_2_5G:
        case SERDES_WAN_TYPE_AE_10G:
            config_0.cfg_ts48_mac_select = 0x4;
            break;

        default:
            config_0.cfg_ts48_mac_select = 0x0;
            break;
    }

#if !defined(CONFIG_BCM963158)
    config_0.cfg_ts48_enable = 1;
#endif
#if defined (CONFIG_BCM96858) || defined(CONFIG_BCM963158)
    config_0.cfg_ts48_pre_sync_fifo_load_rate = 0x6;
#endif
    ag_drv_tod_config_0_set(&config_0);

    /* Assert in order to set configuration */
#if !defined(CONFIG_BCM963158)
    config_0.cfg_ts48_enable = 0;
#endif
    ag_drv_tod_config_0_set(&config_0);

#if defined(CONFIG_BCM963158)
    RU_FIELD_WRITE(0, CLOCK_SYNC_CONFIG, IG, CFG_GPIO_1PPS_SRC,  0);
#endif
}

void serdes_eye_graph_display(void)
{
    int16_t ret;

    ret = eagle_onu10g_display_eye_scan();
    mdelay(40);
}
EXPORT_SYMBOL(serdes_eye_graph_display);

static uint8_t get_gpon_lbe_invert_bit_val(void)
{
    TRX_SIG_ACTIVE_POLARITY lbe_polarity ;
    TRX_SIG_ACTIVE_POLARITY tx_sd_polarity ;
    int                     rc1 ;
    int                     rc2 ;
    uint8_t                 lbe_pol_val ;
    uint8_t                 tx_sd_pol_val ;
    int                     bus = -1;

    opticaldet_get_xpon_i2c_bus_num(&bus);
    rc1 = trx_get_lbe_polarity(bus, &lbe_polarity) ;
    rc2 = trx_get_tx_sd_polarity(bus, &tx_sd_polarity) ;

    lbe_pol_val = (lbe_polarity == TRX_ACTIVE_LOW)? 0 : 1 ;
    tx_sd_pol_val = (tx_sd_polarity == TRX_ACTIVE_LOW)? 0 : 1 ;

    if ((rc1 == 0) && (rc2 == 0))
    {
        return lbe_pol_val ^ tx_sd_pol_val ;
    }
    else
    {
        return (rc1 == 0)? lbe_pol_val : 0 ;
    }
}

static void wan_top_set_lbe_invert(uint8_t lbe_invert_bit)
{
#undef MISC  // to allow access by name to "MISC" registers in Gearbox
    misc_misc_3 misc3_reg; 

    ag_drv_misc_misc_3_get(&misc3_reg);

#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878)
    misc3_reg.laser_invert = lbe_invert_bit;
#else
    misc3_reg.cr_xgwan_top_wan_misc_wan_cfg_laser_invert = lbe_invert_bit;
#endif

    ag_drv_misc_misc_3_set(&misc3_reg);
}

int wan_serdes_config(serdes_wan_type_t wan_type)
{
    int pll50mhz = 1; //Fixed serdes PLL clock as 50MHz
    static int serdes_init_done = 0;
    int lbe_invert_bit;

    __logLevelSet(BCM_LOG_LEVEL_INFO);

    if (wan_type >= sizeof(wan_info)/sizeof(wan_info_t) || wan_type == SERDES_WAN_TYPE_NONE)
    {
        __logError("wan_type=%d.  %s", wan_type, wan_info[SERDES_WAN_TYPE_NONE].name);
        return ERR_CODE_INVALID_RAM_ADDR;
    }
    __logInfo("Wan type: %s", wan_info[wan_type].name);

    if (serdes_init_done == 0)
    {
#if !defined(CONFIG_BCM96846) && !defined(CONFIG_BCM96856) && !defined(CONFIG_BCM96878)
        /* ioRemap virtual addresses of WAN */
        remap_ru_block_addrs(WAN_IDX, WAN_TOP_BLOCKS);
#else
        remap_ru_block_addrs(XRDP_IDX, WAN_TOP_BLOCKS);
#endif
        serdes_debug_init();
        bcm_i2c_sfp_register_notifier(&i2c_sfp_nb);
    }

    if ((wan_type == SERDES_WAN_TYPE_XGPON_10G_2_5G) ||
        (wan_type == SERDES_WAN_TYPE_NGPON_10G_10G) ||
        (wan_type == SERDES_WAN_TYPE_NGPON_10G_10G_8B10B) ||
        (wan_type == SERDES_WAN_TYPE_NGPON_10G_2_5G_8B10B) ||
        (wan_type == SERDES_WAN_TYPE_NGPON_10G_2_5G))
    {
        pon_serdes_lof_fixup_cfg(NULL, &pon_lof_handler);
    }

    if (bcm_i2c_pon_optics_type_get(&optics_type) != BP_SUCCESS)
    {
        __logError("bcm_i2c_pon_optics_type_get failed");
        return -1;
    }

    switch (wan_type)
    {
        case SERDES_WAN_TYPE_EPON_1G:
        case SERDES_WAN_TYPE_AE:
        case SERDES_WAN_TYPE_AE_2_5G:
        case SERDES_WAN_TYPE_EPON_2G:
        case SERDES_WAN_TYPE_EPON_10G_ASYM:
        case SERDES_WAN_TYPE_EPON_10G_SYM:
        case SERDES_WAN_TYPE_AE_10G:
            __logInfo("Configured with %s Mhz PLL", !pll50mhz ? "156.25" : "50");
            pon_drv_epon_init(wan_type, pll50mhz);
            break;
        case SERDES_WAN_TYPE_GPON:
#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE)
        case SERDES_WAN_TYPE_XGPON_10G_2_5G:
        case SERDES_WAN_TYPE_NGPON_10G_2_5G:
        case SERDES_WAN_TYPE_NGPON_10G_10G:
#endif
            /* Set WANTOP LBE before laser output enable */
            lbe_invert_bit = get_gpon_lbe_invert_bit_val();
            wan_top_set_lbe_invert(lbe_invert_bit);

            __logInfo("Configured with %s Mhz PLL", !pll50mhz ? "155.52" : "50");
            pon_drv_gpon_init(wan_type, pll50mhz);
            break;

        default:
            __logInfo("Wan type: %s -- not supported yet", wan_info[wan_type].name);
    }

    wan_top_tod_cfg(wan_type);

    data_polarity_invert();
    if (optics_type == BCM_I2C_PON_OPTICS_TYPE_PMD)
    {
        config_wan_ewake(PMD_EWAKE_OFF_TIME, PMD_EWAKE_SETUP_TIME, PMD_EWAKE_HOLD_TIME);
    }

    /* skip EPON AE 10G mode */
    if ((wan_type == SERDES_WAN_TYPE_NGPON_10G_10G) || (wan_type == SERDES_WAN_TYPE_EPON_10G_SYM))
    {
        __logInfo("Load & verify microcode ...");
        serdes_firmware_download();
        mdelay(1000);
    }

    wan_serdes_type = wan_type;
    serdes_init_done = 1;

    return 0;
}
EXPORT_SYMBOL(wan_serdes_config);

serdes_wan_type_t wan_serdes_type_get(void)
{
    return wan_serdes_type;
}
EXPORT_SYMBOL(wan_serdes_type_get);

void wan_register_pmd_prbs_callback(PMD_DEV_ENABLE_PRBS callback)
{
    pmd_prbs_callback = callback;
}
EXPORT_SYMBOL(wan_register_pmd_prbs_callback);

void wan_prbs_status(bdmf_boolean *valid, uint32_t *errors)
{
    err_code_t err;
    uint8_t lock_lost;
    uint8_t lock;

    /* Check for PRBS lock */
    err= eagle_onu10g_prbs_chk_lock_state(&lock);
    if (ERR_CODE_NONE != err)
    {
        __logError("Get prbs_chk_lock_state failed. err=%d", err);
        return;
    }

    /* Check for PRBS lock lost */
    err= eagle_onu10g_prbs_err_count_state(errors, &lock_lost);
    if (ERR_CODE_NONE != err)
    {
        __logError("Get prbs_err_count_state failed. err=%d", err);
        return;
    }

    if (lock && !lock_lost && *errors == 0)
        *valid = 1;
    else
        *valid = 0;
}
EXPORT_SYMBOL(wan_prbs_status);

void wan_prbs_gen(uint32_t enable, int enable_host_tracking, int mode, serdes_wan_type_t wan_type, bdmf_boolean *valid)
{
    enum srds_prbs_polynomial_enum prbs_poly_mode;
    enum srds_prbs_checker_mode_enum prbs_checker_mode;
    uint32_t errors;

    *valid = 0;

    if (!enable)
    {
        if ((wan_type != SERDES_WAN_TYPE_AE) &&
            (wan_type != SERDES_WAN_TYPE_AE_2_5G) &&
            (wan_type != SERDES_WAN_TYPE_AE_10G))
        {
            bcm_i2c_optics_tx_control(BCM_I2C_OPTICS_DISABLE);
        }
        wan_top_transmitter_control(mac_control);

        /* Disable PRBS */
        eagle_onu10g_tx_prbs_en(0);
        eagle_onu10g_rx_prbs_en(0);

        if (pmd_prbs_callback)
            pmd_prbs_callback((uint16_t)enable, 1);
    }
    else
    {
        if (mode == 0) /* PRBS 7 */
            prbs_poly_mode = PRBS_7;
        else if (mode == 1) /* PRBS 15 */
            prbs_poly_mode = PRBS_15;
        else if (mode == 2) /* PRBS 23 */
            prbs_poly_mode = PRBS_23;
        else if (mode == 3) /* PRBS 31 */
            prbs_poly_mode = PRBS_31;
        else
        {
            __logError("wan_prbs_gen unknown mode %d", mode);
            return;
        }
        
        if (pmd_prbs_callback)
            pmd_prbs_callback((uint16_t)enable, 1);

        wan_top_transmitter_control(laser_on); /* Sets LBE */
        bcm_i2c_optics_tx_control(BCM_I2C_OPTICS_ENABLE);

        /* config Generator and checker */
        prbs_checker_mode = PRBS_SELF_SYNC_HYSTERESIS;
        eagle_onu10g_config_tx_prbs(prbs_poly_mode, 0);
        eagle_onu10g_config_rx_prbs(prbs_poly_mode, prbs_checker_mode, 0);

        /* Enable PRBS Generator*/
        eagle_onu10g_tx_prbs_en(1);

        /* Enable PRBS checker */
        eagle_onu10g_rx_prbs_en(1);
        wan_prbs_status(valid, &errors); /* Clear counters */
        udelay(100);

        wan_prbs_status(valid, &errors);
    }
}
EXPORT_SYMBOL(wan_prbs_gen);

#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE)
int ngpon_wan_post_init (uint32_t sync_frame_length)
{
    bdmf_error_t rc = 0;
    ngpon_gearbox_tx_ctl tx_ctl;

    rc = ag_drv_ngpon_gearbox_tx_ctl_get(&tx_ctl);
    if (rc != BDMF_ERR_OK)
    {
        __logError("Cannot read ngpon_gearbox_tx_ctl rc=%d", rc);
    }

    /*
    * Disable Gearbox Tx
    */
    tx_ctl.cfngpongboxtxen = 0;

    rc = ag_drv_ngpon_gearbox_tx_ctl_set(&tx_ctl);
    if (rc != BDMF_ERR_OK)
    {
        __logError("Cannot write ngpon_gearbox_tx_ctl rc=%d", rc);
    }

    udelay(10);

    /*
    * Re-enable Gearbox Tx thus syncing the FIFO
    */
    tx_ctl.cfngpongboxtxen = 1;

    rc = ag_drv_ngpon_gearbox_tx_ctl_set(&tx_ctl);
    if (rc != BDMF_ERR_OK)
    {
        __logError("Cannot write ngpon_gearbox_tx_ctl rc=%d", rc);
    }

    return (int) rc;
}
EXPORT_SYMBOL(ngpon_wan_post_init);
#endif

#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878)
int PowerOffDevice(int devAddr, int repower)
{
    __logError("Should be implemented in pmc drv!!!");
    return 0;
}

int PowerOnZone(int devAddr, int zone)
{
    __logError("Should be implemented in pmc drv!!!");
    return 0;
}

int PowerOnDevice(int devAddr)
{
    __logError("Should be implemented in pmc drv!!!");
    return 0;
}
#endif

void wan_top_tod_ts48_get(uint16_t *ts48_msb, uint32_t *ts48_lsb)
{
    tod_config_0 config_0 = {};

    /* Arm the update */
    ag_drv_tod_config_0_get(&config_0);
#if defined(CONFIG_BCM963158)
    config_0.cfg_tod_read = 1;
#else
    config_0.cfg_ts48_read = 1;
#endif
    ag_drv_tod_config_0_set(&config_0);

    /* Read timestamp */
#if defined(CONFIG_BCM963158)
    ag_drv_tod_ts48_msb_get(ts48_msb);
    ag_drv_tod_ts48_lsb_get(ts48_lsb);
#else
    ag_drv_tod_msb_get(ts48_msb);
    ag_drv_tod_lsb_get(ts48_lsb);
#endif
    /* Clear */
#if defined(CONFIG_BCM963158)
    config_0.cfg_tod_read = 0;
#else
    config_0.cfg_ts48_read = 0;
#endif
    ag_drv_tod_config_0_set(&config_0);
}
EXPORT_SYMBOL(wan_top_tod_ts48_get);

void pon_wan_top_reset_gearbox_tx(void)
{
#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE)
    ngpon_wan_post_init(0);
#endif
}
EXPORT_SYMBOL(pon_wan_top_reset_gearbox_tx);

/* Should be called before optical module TX power is enabled / MAC is set to transmit */
void wan_top_transmitter_control(enum transmitter_control mode)
{
    uint32_t force_lbe_control_reg;
    /* bit0 is force, bit1 selects the value to set when force is set.  if bit0 == 0, lbe is controlled by MAC and bit1 is d/c */
    const uint32_t lbe_control[][2] = {
        [laser_off]   [TRX_ACTIVE_LOW] = 0b11, [laser_off]   [TRX_ACTIVE_HIGH] = 0b01,
        [mac_control] [TRX_ACTIVE_LOW] = 0b00, [mac_control] [TRX_ACTIVE_HIGH] = 0b00,
        [laser_on]    [TRX_ACTIVE_LOW] = 0b01, [laser_on]    [TRX_ACTIVE_HIGH] = 0b11, };
    TRX_SIG_ACTIVE_POLARITY trx_lbe_polarity = TRX_ACTIVE_LOW; /* Init to arbitrary value to prevent troubles if TRX not inserted */
    int bus = -1;
    int lbe_invert_bit;
    serdes_wan_type_t wan_type = wan_serdes_type_get();

    opticaldet_get_xpon_i2c_bus_num(&bus);
    trx_get_lbe_polarity(bus, &trx_lbe_polarity);

    /* Set WANTOP LBE */
    if (!(wan_type == SERDES_WAN_TYPE_EPON_1G ||
          wan_type == SERDES_WAN_TYPE_AE ||
          wan_type == SERDES_WAN_TYPE_AE_2_5G ||
          wan_type == SERDES_WAN_TYPE_EPON_2G ||
          wan_type == SERDES_WAN_TYPE_EPON_10G_ASYM ||
          wan_type == SERDES_WAN_TYPE_EPON_10G_SYM ||
          wan_type == SERDES_WAN_TYPE_AE_10G))
    {
        lbe_invert_bit = get_gpon_lbe_invert_bit_val();
        wan_top_set_lbe_invert(lbe_invert_bit);
    }

    /* Set FORCE_LBE or MAC control */
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878)
    RU_REG_READ(0, FORCE_LBE_CONTROL, L, force_lbe_control_reg);
#else
    RU_REG_READ(0, FORCE_LBE_CONTROL, CONTROL, force_lbe_control_reg);
#endif
    force_lbe_control_reg &= ~0x3;
    force_lbe_control_reg |= lbe_control[mode][trx_lbe_polarity];
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878)
    RU_REG_WRITE(0, FORCE_LBE_CONTROL, L, force_lbe_control_reg);
#else
    RU_REG_WRITE(0, FORCE_LBE_CONTROL, CONTROL, force_lbe_control_reg);
#endif
}
EXPORT_SYMBOL(wan_top_transmitter_control);

#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE)
void ngpon_wan_top_reset_gearbox_rx(void)
{
    bdmf_error_t            rc;
    ngpon_gearbox_rx_ctl_0  rx_ctl_0;

    rc = ag_drv_ngpon_gearbox_rx_ctl_0_get (&rx_ctl_0);
    if (rc != BDMF_ERR_OK)
    {
        __logError("Cannot read ngpon_gearbox_rx_ctl_0 rc=%d", rc);
        return;
    }

    rx_ctl_0.cfngpongboxrstn = 0;

    rc = ag_drv_ngpon_gearbox_rx_ctl_0_set (&rx_ctl_0);
    if (rc != BDMF_ERR_OK)
    {
        __logError("Cannot write ngpon_gearbox_rx_ctl_0 rc=%d  (1)", rc);
        return;
    }

    udelay(250);   /* Wait ~ 2 frames */

    rx_ctl_0.cfngpongboxrstn = 1;

    rc = ag_drv_ngpon_gearbox_rx_ctl_0_set (&rx_ctl_0);
    if (rc != BDMF_ERR_OK)
    {
        __logError("Cannot write ngpon_gearbox_rx_ctl_0 rc=%d  (2)", rc);
        return;
    }
}
EXPORT_SYMBOL(ngpon_wan_top_reset_gearbox_rx);

#endif

MODULE_LICENSE("Proprietary");

