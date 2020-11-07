/*
  <:copyright-BRCM:2017:proprietary:standard
  
     Copyright (c) 2017 Broadcom 
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
 * pon_drv_serdes_util.c -- Bcm Pon driver: utilities for SerDes fine tuning
 * and debug
 *
 * Description:
 *  Originated at Mixed Signals team
 *
 * Authors: Akiva Sadovski, Vitaly Zborovski
 *
 * $Revision: 1.1 $
 *
 * 2017.July: updated by VZ
 *****************************************************************************/

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include "pon_drv.h"
#include "gpon_ag.h"
#include "ru_types.h"
#include "pon_drv_serdes_util.h"
#include "ru.h"
#include "boardparms.h"
#include "wan_drv.h"

uint32_t calculate_fracn_from_ppm(int16_t cdr_integ16, int16_t ppm_target, uint32_t fracn_ndiv_int, uint32_t fracn_div32);

static void store_fracn_to_flash_handler(struct work_struct *work);

struct fracn_values {
    struct work_struct my_work;
    int curr_div, curr_ndiv;
};

void writePONSerdesReg(uint16_t lane, uint16_t address, uint16_t value, uint16_t  mask)
{
    uint32_t serdes_addr  = (lane<<16) | address;
    uint16_t mask_inv  = ~mask;
    uint16_t rd_serdes = 0;

    RU_FIELD_WRITE(0, PMI, LP_1, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_ADDR), serdes_addr);

    RU_FIELD_WRITE(0, PMI, LP_2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_WRDATA)   , value);
    RU_FIELD_WRITE(0, PMI, LP_2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_MASKDATA) , mask_inv);

    RU_FIELD_WRITE(0, PMI, LP_0, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_WRITE), 1);  /* Select Write access */
    RU_FIELD_WRITE(0, PMI, LP_0, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_EN)   , 1);  /* Initiate SerDes reg access */

    udelay(5);
    RU_FIELD_WRITE(0, PMI, LP_0, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_EN)   , 0);
    RU_FIELD_WRITE(0, PMI, LP_0, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_WRITE), 0);

    if (SERDES_DEBUG)
    {
        __logError("\n --> Write-SerDes: Address =0x%08x  Data=0x%04x  Mask=0x%04x [mask_inv=0x%04x]", serdes_addr, value, mask, mask_inv);
        rd_serdes = readPONSerdesReg(lane, address);
        __logError("\n     READ back   : Address =0x%08x  Data=0x%04x \n", serdes_addr, rd_serdes);
    }
}


uint16_t readPONSerdesReg(uint16_t lane, uint16_t address)
{
    uint32_t serdes_addr  = (lane << 16) | address;
    uint32_t rd32_data = 0;

    RU_FIELD_WRITE(0, PMI, LP_1, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_ADDR), serdes_addr);

    RU_FIELD_WRITE(0, PMI, LP_0, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_WRITE), 0); /* Select Read access */
    RU_FIELD_WRITE(0, PMI, LP_0, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_EN)   , 1); /* Initiate SerDes reg access */
    udelay(5);

    RU_REG_READ(0, PMI, LP_3, rd32_data);

    RU_FIELD_WRITE(0, PMI, LP_0, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_EN)   , 0);
    udelay(5);

    return (rd32_data & 0xffff);
}


void PCSwriteSerdes(uint16_t lane, uint16_t address, uint16_t value, uint16_t  mask)
{
#if defined(CONFIG_BCM963158)
    uint32_t serdes_addr = (lane << 16) | address;
    uint16_t mask_inv  = ~mask;
    uint16_t rd_serdes = 0;
    uint32_t wr32_data = 0;

    RU_REG_WRITE(0, PMI, LP_0, wr32_data);

    RU_FIELD_WRITE(0, PMI, LP_1, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_ADDR), serdes_addr);

    RU_FIELD_WRITE(0, PMI, LP_2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_WRDATA)  , value);
    RU_FIELD_WRITE(0, PMI, LP_2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_MASKDATA), mask_inv);

    wr32_data = 5; /* Write  PCS-register : { [2]pcs_pmi_lp_en = 1, [1]misc_pmi_lp_en = 0, [0]pmi_lp_write = 1} */
    RU_REG_WRITE(0, PMI, LP_0, wr32_data);

    udelay(5);
    wr32_data = 0; /* end pmi_lp transaction */
    RU_REG_WRITE(0, PMI, LP_0, wr32_data);

    if (SERDES_DEBUG)
    {
        __logError("\n --> Write-SerDes: Address =0x%08x  Data=0x%04x  Mask=0x%04x [mask_inv=0x%04x]", serdes_addr, value, mask, mask_inv);
        rd_serdes = PCSreadSerdes(lane, address);
        __logError("\n     READ back   : Address =0x%08x  Data=0x%04x \n", serdes_addr, rd_serdes);
    }
#else
    return;
#endif
}


uint16_t PCSreadSerdes(uint16_t lane, uint16_t address)
{
#if defined(CONFIG_BCM963158)
    uint32_t serdes_addr = (lane << 16) | address;
    uint32_t rd32_data = 0;
    uint32_t wr32_data = 0;
    RU_REG_WRITE(0, PMI, LP_0, wr32_data);
    RU_FIELD_WRITE(0, PMI, LP_1, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMI_LP_ADDR), serdes_addr);

    wr32_data = 4; /* Read PCS-register : { [2]pcs_pmi_lp_en = 1, [1]misc_pmi_lp_en = 0, [0]pmi_lp_write = 0} */
    RU_REG_WRITE(0, PMI, LP_0, wr32_data);

    udelay(5);
    RU_REG_READ(0, PMI, LP_4, rd32_data);  /* read back PCS reg value from 'WAN_TOP_PMI_LP_4' */

    wr32_data = 0; /* end pmi_lp transaction */
    RU_REG_WRITE(0, PMI, LP_0, wr32_data);

    return (rd32_data & 0xffff);
#else
    return 0xDEAD;
#endif
}


void sgb_rescal_init(void)
{
    uint32_t rd32_data = 0x0;
    uint32_t wr32_data = 0x8000;

#if defined(CONFIG_BCM963158)
    RU_REG_WRITE  (0, RESCAL, AL_CFG, wr32_data);
    udelay(100);
    WAN_TOP_READ_32(0x020, rd32_data);
#else
    RU_REG_WRITE(0, RESCAL, CFG, wr32_data);  /* CFG_WAN_RESCAL_RSTB = 1 */
    udelay(100);
    RU_REG_READ(0, RESCAL, STATUS_0, rd32_data); 
#endif
    __logInfo("\n WAN_TOP_RESCAL_STATUS_0 =0x%x", rd32_data);
}


void set_clkp1_offset(uint8_t desired_p_d_offset)      /* clkp1_offset (p-d) */
{
    uint8_t  step, cnt, d_location, now_p_location, next_p_location;
    uint16_t rd_serdes;

    desired_p_d_offset = desired_p_d_offset % 64;       /* offset smaller than UI */
    rd_serdes       = readPONSerdesReg(0x0800, DSC_A_rx_pi_cnt_bin_d); /* Read current slicer possitions */
    d_location      = (rd_serdes & 0x7f);               /* {0xd007}cnt_bin_d_dreg[06:00] */
    now_p_location  = (rd_serdes & 0x7f00) >> 8;        /* {0xd008}cnt_bin_p1_dreg[14:08] */
    next_p_location = d_location + desired_p_d_offset;

    /* calculate number of movement steps and direction */
    if (next_p_location >= now_p_location)
    {
        step = next_p_location - now_p_location;
        writePONSerdesReg(0x0800, DSC_A_rx_pi_control, 0x2401, 0x74ff); /*  rx_pi_phase_step_dir= 0x1 (Increment, right shift), rx_pi_slicers_en= 0x2, step_cnt = 1  */

    }
    else
    {
        step = now_p_location - next_p_location;
        writePONSerdesReg(0x0800, DSC_A_rx_pi_control, 0x2001, 0x74ff); /*  rx_pi_phase_step_dir= 0x0 (Decrements, left shift), rx_pi_slicers_en= 0x2, step_cnt = 1 */
    }

    /* move the Slicer(P) the required steps */
    for (cnt = 0; cnt < step; cnt++)
    {
        writePONSerdesReg(0x0800, DSC_A_rx_pi_control, 0x0200, 0x0200); /* rx_pi_manual_strobe= 0x1 = Increments/Decrements by 1 every strobe */
    }


    if (SERDES_DEBUG)  /* Verify rx-pi adjustment correctly */
    {
        rd_serdes       = readPONSerdesReg(0x0800, DSC_A_rx_pi_cnt_bin_d); /* Read adjusted slicer possitions */
        d_location      = (rd_serdes & 0x7f);               /* {0xd007}cnt_bin_d_dreg[06:00] */
        now_p_location  = (rd_serdes & 0x7f00) >> 8;        /* {0xd008}cnt_bin_p1_dreg[14:08] */
        __logError(" 0xd007 =0x%04x\n d_location =%d\n now_p_location =%d\n Done Slicer-P-Adjustment", rd_serdes, d_location, now_p_location);
    }
}


void set_clk90_offset(uint8_t desired_m1_d_offset)       /* clk90 offset (m1-d) */
{
    uint8_t  step, cnt, d_location, now_m1_location, next_m1_location;
    uint16_t rd_serdes;

    desired_m1_d_offset = desired_m1_d_offset % 64;      /* offset smaller than UI */
    rd_serdes        = readPONSerdesReg(0x0800, DSC_A_rx_pi_cnt_bin_m); /* Read current slicer possitions */
    now_m1_location  = (rd_serdes & 0x7f);               /* {0xd009}cnt_bin_m1_mreg[06:00] */
    d_location       = (rd_serdes & 0x7f00) >> 8;        /* {0xd009}cnt_bin_d_mreg[14:08]  */
    next_m1_location = d_location + desired_m1_d_offset;

    /* calculate number of movement steps and direction */
    if (next_m1_location >= now_m1_location)
    {
        step = next_m1_location - now_m1_location;
        writePONSerdesReg(0x0800, DSC_A_rx_pi_control, 0x4401, 0x74ff); /*  rx_pi_phase_step_dir= 0x1 (Increment, right shift), rx_pi_slicers_en= 0x4, step_cnt = 1  */
    }
    else
    {
        step = now_m1_location - next_m1_location;
        writePONSerdesReg(0x0800, DSC_A_rx_pi_control, 0x4001, 0x74ff); /*  rx_pi_phase_step_dir= 0x0 (Decrements, left shift), rx_pi_slicers_en= 0x4, step_cnt = 1 */
    }

    /* move the Slicer(P) the required steps  */
    for (cnt = 0; cnt < step; cnt++)
    {
        writePONSerdesReg(0x0800, DSC_A_rx_pi_control, 0x0200, 0x0200); /* rx_pi_manual_strobe= 0x1 = Increments/Decrements by 1 every strobe */
    }

    if(SERDES_DEBUG) /* Verify rx-pi adjustment correctly */
    {
        rd_serdes        = readPONSerdesReg(0x0800, DSC_A_rx_pi_cnt_bin_m); /* Read adjusted slicer possitions */
        now_m1_location  = (rd_serdes & 0x7f);               /* {0xd009}cnt_bin_m1_mreg[06:00] */
        d_location       = (rd_serdes & 0x7f00) >> 8;        /* {0xd009}cnt_bin_d_mreg[14:08] */
        next_m1_location = d_location + desired_m1_d_offset;
        __logError(" 0xd009 =0x%04x\n now_m1_location =%d\n d_location =%d\n Done Slicer-M1-Adjustment", rd_serdes, now_m1_location, d_location);
    }
}


void rx_pi_spacing(uint8_t desired_m1_d_offset, uint8_t desired_p_d_offset)
{
    writePONSerdesReg(0x800, DSC_A_rx_pi_control, 0x0800, 0x0800);  /* Freeze RX-PI, rx_pi_manual_mode = 1 */
    set_clk90_offset(desired_m1_d_offset);             /* clk90_offset */
    set_clkp1_offset(desired_p_d_offset);             /* clkp1_offset */
    writePONSerdesReg(0x800, DSC_A_rx_pi_control, 0x0000, 0x7800);  /* UnFreeze RX-PI, rx_pi_manual_mode = 0  */
}


/* Configure the PLL/clock input selection:  LCREF (50MHz) or PADREF (155.52MHz or 156.25MHz)
  since refclk 78Mhz is not supported, pll0_lcref is hard wired to 0 ==> lcref_sel should always select ppl1_lcref
  set refin to 1 to select lcref_clk (50MHz)
  afe constraint: refin_en ==1 and refout_en ==1 => illegal setting */
void set_pll_refclk(uint8_t pll_refclk)
{
    uint32_t rd32_data;

    if (pll_refclk == LCREF)  /* -->  WAN_SERDES_PLL_CTL = 0x105 */
    {
        __logDebug("\n PLL RefClock = LCREF (50MHz) ");
        RU_FIELD_WRITE(0, WAN_SERDES, PLL_CTL, CHOP(CFG_, PLL1_LCREF_SEL), 0); /* 0 = select pll1_lcref. 1 = pll0_lcref. */
        RU_FIELD_WRITE(0, WAN_SERDES, PLL_CTL, CHOP(CFG_, PLL1_REFIN_EN),  1); /* Reference select. 0 = select pad_pll1_refclkp/n. 1 - select pll1_lcrefp/n. */
        RU_FIELD_WRITE(0, WAN_SERDES, PLL_CTL, CHOP(CFG_, PLL0_LCREF_SEL), 1); /* 0 = pll0_lcref.  1 = pll1_lcref. */
        RU_FIELD_WRITE(0, WAN_SERDES, PLL_CTL, CHOP(CFG_, PLL0_REFIN_EN),  1); /* Reference select: 0 = select pad_pll0_refclkp/n. 1 = select pll0_lcrefp/n. */
    }
    else /* pll_refclk = PADREF  -->  WAN_SERDES_PLL_CTL =0x400 */
    {
        __logDebug("\n PLL RefClock = PADREF (155.52MHz or 156.25MHz)");
        RU_FIELD_WRITE(0, WAN_SERDES, PLL_CTL, CHOP(CFG_, PLL1_LCREF_SEL), 1);
        RU_FIELD_WRITE(0, WAN_SERDES, PLL_CTL, CHOP(CFG_, PLL1_REFIN_EN),  0);
        RU_FIELD_WRITE(0, WAN_SERDES, PLL_CTL, CHOP(CFG_, PLL0_LCREF_SEL), 0);
        RU_FIELD_WRITE(0, WAN_SERDES, PLL_CTL, CHOP(CFG_, PLL0_REFIN_EN),  0);
    }

    if(SERDES_DEBUG)
    {
        RU_REG_READ(0, WAN_SERDES, PLL_CTL, rd32_data);
        printk("\n WAN_SERDES_PLL_CTL = 0x%x", rd32_data);
    }
    /* wait for stable Ref Clk before continue */
    udelay(100);
}


/* wait & check for PLL_Lock =1 */
static int chk_pll_locked(uint8_t pll_use)
{
    uint16_t cnt, pll0_status = 0, pll1_status = 0;
    uint16_t rd_serdes;
    const uint16_t pll_chk_limit = 10000;

    /*  Check pll0_status   */
    ////////////////////////
    for (cnt = 0; cnt < pll_chk_limit; cnt++)
    {
        /* read PLL0 Lock status */
        rd_serdes = readPONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_0), PLL_CAL_COM_CTL_STATUS_0);
        pll0_status = rd_serdes & 0x0100;
        if (pll0_status)
            break;
        udelay(5); /* wait 'udelay' before next check */
    }

    if (!pll0_status)
    {
        __logError("[!] %s: PLL0 not locked ! \n", __FUNCTION__);
    }
    if (SERDES_DEBUG)
    {
        __logInfo("[!] %s: pll0_lock =%d  {Reg 0xD128}=0x%x \n", __FUNCTION__, (pll0_status >> 8), rd_serdes);
    }

    /*  Check DSC0_Lock    */
    ////////////////////////
    for (cnt = 0; cnt < pll_chk_limit; cnt++)
    {
        /* read Rx DSC Lock status 0xD01A */
        rd_serdes = readPONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_0), DSC_B_dsc_sm_status_dsc_lock);
        pll0_status = rd_serdes & 0x1;
        if (pll0_status == 1)
            break;
        udelay(5); /* wait 'udelay' before next check */
    }

    if (!pll0_status)
    {
        __logError("[!] %s: DSC0 not locked ! \n", __FUNCTION__);
    }
    if (SERDES_DEBUG)
    {
        __logInfo("[!] %s: DSC0_LOCK =%d  {Reg 0xD01A}=0x%x \n", __FUNCTION__, pll0_status, rd_serdes);
    }

    /* read RxDSC done 0xD01E */
    rd_serdes = readPONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_0), DSC_B_dsc_sm_status_dsc_state);
    pll0_status = (rd_serdes  & 0xF800)>>11;

    if (SERDES_DEBUG)
    {
        __logInfo("    %s: DSC0_STATE=0x%x {Reg 0xD01E}=0x%x  \n", __FUNCTION__, pll0_status, rd_serdes);
    }


    if (pll_use == ONE_PLL)
    {
        return pll0_status;
    }
    else /* if using TWO_PLLS,  check also for PLL1 Lock status */
    {
        for (cnt = 0; cnt < pll_chk_limit; cnt++)
        {
            /*  Check pll1_status   */
            ////////////////////////
            rd_serdes = readPONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_1), PLL_CAL_COM_CTL_STATUS_0);
            pll1_status = rd_serdes & 0x0100;
            if (pll1_status)
                break;
            udelay(5); /* wait 'udelay' before next check */
        }
        if (!pll1_status)
        {
            __logError("[!] %s: PLL1 not locked ! \n", __FUNCTION__);
        }
        if (SERDES_DEBUG)
        {
            __logInfo("[!] %s: pll1_lock =%d  {Reg 0xD128}=0x%x \n", __FUNCTION__, (pll1_status >> 8), rd_serdes);
        }

        /*  Check DSC1_Lock    */
        ////////////////////////
        for (cnt = 0; cnt < pll_chk_limit; cnt++)
        {
            /* read Rx DSC Lock status 0xD01A */
            rd_serdes = readPONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_1), DSC_B_dsc_sm_status_dsc_lock);
            pll0_status = rd_serdes & 0x1;
            if (pll0_status == 1)
                break;
            udelay(5); /* wait 'udelay' before next check */
        }

        if (!pll1_status)
        {
            __logError("[!] %s: DSC1 not locked ! \n", __FUNCTION__);
        }

        if (SERDES_DEBUG)
        {
            __logInfo("[!] %s: DSC1_LOCK =%d  {Reg 0xD01A}=0x%x \n", __FUNCTION__, pll1_status, rd_serdes);
        }

        /* read RxDSC done 0xD01E */
        rd_serdes = readPONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_1), DSC_B_dsc_sm_status_dsc_state);
        pll1_status = (rd_serdes & 0xF800) >> 11;

        if (SERDES_DEBUG)
        {
            __logInfo("    %s: DSC1_STATE=0x%x {Reg 0xD01E}=0x%x  \n", __FUNCTION__, pll1_status, rd_serdes);
        }
        return pll0_status | pll1_status;
    }
}


/* average the value of 'cdr_integ_reg'  */
static uint8_t average_cdr_integ(int16_t *ret_cdr_integ16)
{
    int16_t  cdr_integ16     = 0;
    int16_t  cdr_min         =  32767; // max posible initial value to be replaced by measurement of cdr_integ
    int16_t  cdr_max         = -32768; // min posible initial value to be replaced by measurement of cdr_integ
    uint16_t cnt             = 0;
    int32_t  cdr_integ_sum32 = 0;

    // calculate RX-PPM (cdr_integ) Statistics : Average, Min, Max
    for (cnt = 0; cnt < PPM_AVG_NUM; cnt++)
    {
        cdr_integ16 = readPONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_0), DSC_A_cdr_status_integ_reg);
        // if (SERDES_DEBUG) printk("%04x\n", cdr_integ16); //printk("[%03d] =%04x\n", cnt, cdr_integ16);

        cdr_min = (cdr_integ16 < cdr_min) ? cdr_integ16 : cdr_min;
        cdr_max = (cdr_integ16 > cdr_max) ? cdr_integ16 : cdr_max;

        cdr_integ_sum32 += cdr_integ16;
        udelay(CDR_INTEG_AVRG_TIME); /* delay for cdr_integ status to update*/
    }

    // calculate the cdr_integ average value
    cdr_integ16 = cdr_integ_sum32 / PPM_AVG_NUM;

    if (SERDES_DEBUG)
    {
        __logInfo("[*]%s: %d  Average cdr_integ[dec]=%d --> Average PPM =%d  (ppm_min=%d  ppm_max=%d)\n", __FUNCTION__, PPM_AVG_NUM, cdr_integ16, cdr_integ16 /(PPM_FACTOR), cdr_max/(PPM_FACTOR), cdr_min/(PPM_FACTOR));
    }

    // [*] Check if ppm min/max within expected limits
    //     if ppm outside the limits it is indication of absent(fiber disconnect) or unstable rx signal
    if ((cdr_max - cdr_min) > 2 * (PPM_GOOD_LMT) * (PPM_FACTOR))
    {
        __logError("[!] Input Signal Unstable or Absent (Fiber Disconnected)\n");
        *ret_cdr_integ16 =  0;
        return (PPM_UNSTABLE);
    }

    *ret_cdr_integ16 = cdr_integ16;
    return(0);
}

/* Configure PLL0 to Fractional mode: using Int and FracN values  */
void set_pll_FracN(uint16_t fracn_ndiv_int, uint32_t fracn_div32)
{
    uint16_t wr_serdes, wr_mask;

    //////////////////////////////////////////////////////
    // Set new FracN values and restart the PLL         //
    //////////////////////////////////////////////////////

    /* AMS_COM_PLL_CONTROL_7 [0xd0b7] */
    wr_mask = 0xFFFF;
    wr_serdes = fracn_div32 & wr_mask;
    writePONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_0), AMS_COM_PLL_CONTROL_7, wr_serdes, wr_mask);  /* set 'fracn_div_l' */

    /* AMS_COM_PLL_CONTROL_8 [0xd0b8] */
    wr_serdes = ( ((fracn_ndiv_int << 4) & (0x03ff << 4)) | ( (fracn_div32 >>16) & (0x0003)));
    wr_mask   = (                          (0x03ff << 4)  |                        (0x0003));
    writePONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_0),  AMS_COM_PLL_CONTROL_8, wr_serdes, wr_mask);

    /* restart PLL calibration by reset of toggling 'core_dp_s_rstb' */
    udelay(1);
    writePONSerdesReg(0x0800, CORE_PLL_COM_TOP_USER_CONTROL, 0x0000, 0x2000);   /* # [13] core_dp_s_rstb = 1 */
    udelay(1);
    writePONSerdesReg(0x0800, CORE_PLL_COM_TOP_USER_CONTROL, 0x2000, 0x2000);   /* # [13] core_dp_s_rstb = 1 */
    udelay(1);
    chk_pll_locked(ONE_PLL);
    udelay(1);
}

//////////////////////////////////////////////////////////////////
//  mode            NGPON_10_10 / EPON_10_10 / AE_10_10
//
//  ppm_target      optimial ppm-offset for the RX-CDR
//
//  pll_adj_en      Enable or Block the adjustment of the PLL to optimize the ppm.
//    pll_adj_en =1    PLL adjustment allowed during serdes initialization only, before MAC is activated.
//    pll_adj_en =0    after MAC activation, ppm can be evaluated and stored to Flash for next power-up/serdes-init.
//
//////////////////////////////////////////////////////////////////
void pll_ppm_adj_FracN_10G(uint8_t mode, int16_t ppm_target, bool pll_adj_en)
{
    uint8_t         ret_val;
    uint16_t        rd_serdes;
    uint16_t        pll_fracn_sel, fracn_div_l, fracn_ndiv_int;
    uint16_t        nom_pll_fracn_sel, nom_fracn_div_h, nom_fracn_div_l, nom_fracn_ndiv_int;
    int16_t         cdr_integ16;
    uint32_t        fracn_div_h;
    uint32_t        fracn_div32, nom_fracn_div32, fracn_lmt_low, fracn_lmt_high;
    static uint64_t prev_stamp = 0;
    const uint64_t  now_stamp = jiffies;
    const uint64_t  gap_stamp = now_stamp - prev_stamp;

#ifndef PLL_PPM_ADJ
    __logError("%s called with PLL_PPM_ADJ undefined\n", __FUNCTION__);
    return;
#endif

    prev_stamp = now_stamp;
    if (gap_stamp < msecs_to_jiffies(1000))
    {
        return;
    }

    if(SERDES_DEBUG) __logInfo("[*]%s: mode=%d  , ppm_target=%d \n", __FUNCTION__, mode, ppm_target);

    // Default FracN PLL values for ITU.T & IEEE 10G modes
    if (mode == NGPON_10_10)  /* 10G - ITU modes */
    {
        nom_pll_fracn_sel  = 0x1;
        nom_fracn_ndiv_int = 0xC7;
        nom_fracn_div_h    = 0x0;
        nom_fracn_div_l    = 0x432C;
        nom_fracn_div32    = 0x0432C;

        fracn_lmt_high     = G_FRACN_HIGH;  // ITU-T 10G PLL: max allowed FracN value
        fracn_lmt_low      = G_FRACN_LOW;   // ITU-T 10G PLL: min allowed FracN value

    }
    else if ((mode == EPON_10_10) || (mode == AE_10_10))  /* 10G- IEEE modes */
    {
        nom_pll_fracn_sel  = 0x1;
        nom_fracn_ndiv_int = 0xCE;
        nom_fracn_div_h    = 0x1;  /* fracn_div[17:00] = 0x10000 */
        nom_fracn_div_l    = 0x0;
        nom_fracn_div32    = 0x10000;

        fracn_lmt_high     = E_FRACN_HIGH;  // IEEE 10G PLL: max allowed FracN value
        fracn_lmt_low      = E_FRACN_LOW;   // IEEE 10G PLL: min allowed FracN value

    }
    else
    {
        __logError("\n[!]%s: Error.  Mode NOT supportting 'pll-ppm-adjustment'\n", __FUNCTION__);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Read current PLL FracN configuration                                       //
    ////////////////////////////////////////////////////////////////////////////////
    rd_serdes = readPONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_0), AMS_COM_PLL_CONTROL_8);
    pll_fracn_sel  = (rd_serdes & 0x8000) >> 15;
    fracn_ndiv_int = (rd_serdes & 0x3FF0) >> 4;
    fracn_div_h    = (rd_serdes & 0x0003);

    rd_serdes = readPONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_0), AMS_COM_PLL_CONTROL_7);
    fracn_div_l    = rd_serdes;
    fracn_div32    = (fracn_div_h << 16) | fracn_div_l;
    if (SERDES_DEBUG)
    {
        __logInfo("[*]%s: Current PLL configuration: pll_fracn_sel=0x%x, fracn_ndiv_int= 0x%x, fracn_div32= 0x%x\n", __FUNCTION__, pll_fracn_sel, fracn_ndiv_int, fracn_div32);
    }

    // [*] IF current FracN values outside Low- High limits- Re-configure PLL back to DEFAULT FracN values
    if ((fracn_div32 > fracn_lmt_high) || (fracn_div32 < fracn_lmt_low))
    {
        __logError("[*]%s: Current FracN value 0x=%x outside LOW[0x%x] - HIGH[0x%x] range.\n", __FUNCTION__, fracn_div32, fracn_lmt_low, fracn_lmt_high);

        if (pll_adj_en)
        {
            /*  Reset PLL to default FracN values.  Can happen during init, but not after starting MAC !!! */
            __logError("[!] Reseting the PLL to nominal FracN values\n");
            set_pll_FracN(nom_fracn_ndiv_int, nom_fracn_div32);
        }
        else
        {
            __logError("[!] Unexpected out of range PLL FracN values.\n");
        }
        return;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Get the current RX-PPM-Offset (cdr_integ status)                           //
    ////////////////////////////////////////////////////////////////////////////////

    ret_val = average_cdr_integ(&cdr_integ16);      /* average the value of 'cdr_integ_reg'  */

    if (SERDES_DEBUG)
    {
        __logInfo("[*]%s: cdr_integ16 = 0x%x   , %d[dec] \n", __FUNCTION__, cdr_integ16, cdr_integ16);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Check if current ppm needs adjustments/correction                          //
    ////////////////////////////////////////////////////////////////////////////////

    // [*] IF input signal is unstable or absent (fiber dissconnect) --> Exit.
    if (ret_val == PPM_UNSTABLE)
    {
        if (SERDES_DEBUG)
        {
            __logInfo("[!] PPM_FAULT = Input signal unstable.  No adjustment performed.  Exit 'pll_ppm_adj' \n");
        }
        return;
    }

    //[*] If ppm within (ppm_target +/-PPM_GOOD_LMT) limits --> then NO PPM-Adjustments required
    if (abs(cdr_integ16 - ppm_target * PPM_FACTOR) <= PPM_GOOD_LMT * PPM_FACTOR)
    {
        if (SERDES_DEBUG)
        {
            __logInfo("[!]PPM @ target range.  No adjustment required.\n");
        }
    }
    else
    {
        // [*] IF rx-ppm bigger than +/-100[ppm] , Signal outside the range / fiber disconnected, No adjustment possible
        if ((cdr_integ16 >= PPM_MAX_LMT*PPM_FACTOR) || (cdr_integ16 <= -PPM_MAX_LMT*PPM_FACTOR))
        {
            __logInfo("[!]Rx-PPM = %d is outside the +/-%d MAX allowed ppm limits.  No adjustment performed.  Exit 'pll_ppm_adj' \n", cdr_integ16/PPM_FACTOR, PPM_MAX_LMT);
            return;
        }

        ////////////////////////////////////////////////////////////////////////////////
        // Calculate new FracN value for PLL  !!!                                     //
        ////////////////////////////////////////////////////////////////////////////////
        fracn_div32 = calculate_fracn_from_ppm(cdr_integ16, ppm_target, fracn_ndiv_int, fracn_div32);

        // [*] IF new FracN values exceeds Low- High limits.  Abandon PLL update !
        if ((fracn_div32 > fracn_lmt_high) || (fracn_div32 < fracn_lmt_low))
        {
            __logError("[!]  Error.  Cannot perform ppm adjustment.\n[!]  new FracN = %u exceeds Low - High limits.  Exit 'pll_pmm_adj'. \n", fracn_div32);
            return;
        }

        ////////////////////////////////////////////////////////////////////////////////
        // Reconfigre the PLL with new FracN values   !!!                             //
        ////////////////////////////////////////////////////////////////////////////////
        if (pll_adj_en)
        {
            __logInfo("[!] Reconfiguring the PLL to new FracN values \n");
            set_pll_FracN(fracn_ndiv_int,  fracn_div32);

            ////////////////////////////////////////////////////////////////////////////////
            // Check the new ppm
            ////////////////////////////////////////////////////////////////////////////////

            ret_val= average_cdr_integ(&cdr_integ16);

            //[*] validate ppm is within expected limits (ppm_target +/-PPM_GOOD_LMT)
            if (abs(cdr_integ16 - ppm_target * PPM_FACTOR) <= PPM_GOOD_LMT * PPM_FACTOR)
            {
                if (SERDES_DEBUG)
                {
                    __logInfo("[!]PPM @ target range.  SUCCESS and DONE.\n");
                }
            }
            else // Changed PLL but didn't reach target - NOT GOOD --> reset PLL to default values
            {
                __logError("[!]%s: Changed PLL but didn't reach ppm-target.  Reseting the PLL to nominal FracN values\n", __FUNCTION__);
                /*  Reset PLL to default FracN values !!! */
                set_pll_FracN(nom_fracn_ndiv_int, nom_fracn_div32);
            }
            return; // if pll_adj_en we are in init, let's wait for ds_sync to update flash
        }
    }

    //////////////////////////////////////
    // store new PLL values to Flash
    //////////////////////////////////////
    {
        static struct fracn_values curr;

        INIT_WORK((struct work_struct *)&curr, store_fracn_to_flash_handler);
        curr.curr_div = fracn_div32;
        curr.curr_ndiv = fracn_ndiv_int;
        schedule_work((struct work_struct *)&curr);
    }
}
EXPORT_SYMBOL(pll_ppm_adj_FracN_10G);


/* Disable/Bypass Signal-Detect  */
void rx_sigdet_dis(void)
{

//   0xD0C1 bits 6, 5 and 0
//   energy_detect_frc_val -> 1
//   energy_detect_frc -> 1
//   afe_signal_detect_dis -> 1
    writePONSerdesReg(0x0800, SIGDET_SIGDET_CTRL_1, 0x0061, 0x0061);

//   0xD0C2 bits 0-2 and 4-6:
//   los_thresh -> 0
//   signal_detect_thresh -> 0

    writePONSerdesReg(0x0800, SIGDET_SIGDET_CTRL_2, 0x0000, 0x0077);

//  0xD0C3 bits 6-7:
//  analog_sd_override -> 1
    writePONSerdesReg(0x0800, SIGDET_SIGDET_CTRL_3, 0x0040, 0x00C0);

} // END of 'rx_sigdet_dis()' function


uint32_t calculate_fracn_from_ppm(int16_t cdr_integ16, int16_t ppm_target, uint32_t fracn_ndiv_int, uint32_t fracn_div32)
{
    int16_t    delta_target16;
    int32_t    delta_fracn32;
    uint64_t   dividend64;
    int        pos;

    ////////////////////////////////////////////////////////////////////////////////
    // Calculate new FracN value for PLL  !!!                                     //
    ////////////////////////////////////////////////////////////////////////////////
    //                                                                            //
    //   FracN PLL Frequency = F(0) = RefClk*(Int + FracN(0)/2^18)                //
    //                                                                            //
    //   PPM = (F(1)/F(0) -1)*1e6                                                 //
    //                                                                            //
    //   delta(FracN) =  delta(PPM)/1e6 * (Int*2^18 + FracN(0))                   //
    //                                                                            //
    ////////////////////////////////////////////////////////////////////////////////

    // calculate PPM delta //
    delta_target16 = ppm_target - cdr_integ16/PPM_FACTOR;
    pos = (delta_target16 > 0);

    if(SERDES_DEBUG)
    {
        __logInfo("[A]  cdr_integ16=%d \n[B]  cdr_integ16/PPM_FACTOR =%d \n[C]  delta_target16 =%d \n", cdr_integ16, cdr_integ16/PPM_FACTOR, delta_target16);
    }

    // Calculate new FracN value //
    dividend64 = ((uint64_t) abs(delta_target16)) * ((fracn_ndiv_int << 18) + fracn_div32);  // Calculate the new FracN delta
    do_div(dividend64, 1000000);
    delta_fracn32 = pos ? dividend64 : -dividend64;
    fracn_div32 += delta_fracn32;

    if(SERDES_DEBUG)
    {
        __logInfo("[calc_result] delta_taraget16 =%d \n[calc_result] delta_fracn32   = %d  \n[calc_result] new_fracn32 0x=%05x , %d[dec] \n", delta_target16, delta_fracn32, fracn_div32, fracn_div32);
    }

    return(fracn_div32);
}

int get_pll_fracn_from_flash(int *div, int *ndiv)
{
    char buf[32] = {};

    if ((kerSysFsFileGet(DATA_FRACN, buf, sizeof(buf)) < 0) || (sscanf(buf, " %d %d", div, ndiv) != 2))
    {
        if (SERDES_DEBUG)
        {
            __logError("Failed to read fracn from %s.\n", DATA_FRACN);
        }
        return -1;
    }
    if (SERDES_DEBUG)
    {
        __logInfo("%s: flash_pll_fracn32=0x%05x, flash_pll_ndiv16=0x%04x\n", __FUNCTION__, *div, *ndiv);
    }
    return 0;
}

void set_pll_fracn_to_flash(int div, int ndiv)
{
    char buf[32] = {};

    if ((sprintf(buf, "%d %d\n", div, ndiv) < 0) || (kerSysFsFileSet(DATA_FRACN, buf, sizeof(buf)) < 0))
    {
        __logError("Failed to write fracn to %s.\n", DATA_FRACN);
    }
    if (SERDES_DEBUG)
    {
        __logInfo("%s: flash_pll_fracn32=0x%05x, flash_pll_ndiv16=0x%04x\n", __FUNCTION__, div, ndiv);
    }
}

static void store_fracn_to_flash_handler(struct work_struct *work)
{
    struct fracn_values *curr = (struct fracn_values *)work;
    uint32_t flash_fracn_div, flash_fracn_ndiv;

    if (!get_pll_fracn_from_flash(&flash_fracn_div, &flash_fracn_ndiv) && (abs(curr->curr_div - flash_fracn_div) <= FRACN_GOOD_LMT))
    {
        if (SERDES_DEBUG)
        {
            __logInfo("[!] previous FracN values good enough.  No need to store new values.\n");
        }
    }
    else
    {
        if (SERDES_DEBUG)
        {
            __logInfo("[!] Storing to flash PLL FracN values.\n");
        }
        set_pll_fracn_to_flash(curr->curr_div, curr->curr_ndiv);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  gearbox_reg_dump:
  Read WAN-Gearbox registers.  based on their address
  future update -> to read the values based on reg.name using "RU_REG_READ(i, b, r, rv);" function
*/

// old address
//#define WAN_START_ADDRS 0x80144000
//#define WAN_END_ADDRS   0x801440c8
#define WAN_START_ADDRS 0x000
#define WAN_END_ADDRS   0x0c8


void gearbox_reg_dump(void)
{
    uint32_t rd_data, addr;

    __logError("\n------------------GEARBOX REGISTERS DUMP---------------------- ");

    for (addr = WAN_START_ADDRS; addr <= WAN_END_ADDRS; addr += 4)
    {
        WAN_TOP_READ_32(addr, rd_data);
        __logError("\nReg{0x%x} = 0x%x", addr, rd_data);
    }
}

/*
    serdes_reg_dump
    Read the values of SerDes internal registers based on their address
*/
void serdes_reg_dump(void)
{
    uint16_t rd_serdes, addr_serdes;

    __logError("\n------------------SerDes REGISTERS DUMP---------------------- ");
    __logError("\nDSC_Block:");
    for (addr_serdes = 0xd000; addr_serdes <= 0xd053; addr_serdes++)
    {
        rd_serdes = readPONSerdesReg(0x0800, addr_serdes);
        __logError("\n[0x%x] = 0x%x", addr_serdes, rd_serdes);
    }
    __logError("\nTX_PI_LBE ");
    for (addr_serdes = 0xd070; addr_serdes <= 0xd07c; addr_serdes++)
    {
        rd_serdes = readPONSerdesReg(0x0800, addr_serdes);
        __logError("\n[0x%x] = 0x%x", addr_serdes, rd_serdes);
    }
    __logError("\nCKRST_CTRL ");
    for (addr_serdes = 0xd080; addr_serdes <= 0xd08E; addr_serdes++)
    {
        rd_serdes = readPONSerdesReg(0x0800, addr_serdes);
        __logError("\n[0x%x] = 0x%x", addr_serdes, rd_serdes);
    }
    __logError("\nAMS_RX/TX_CTRL ");
    for (addr_serdes = 0xd090; addr_serdes <= 0xd0A9; addr_serdes++)
    {
        rd_serdes = readPONSerdesReg(0x0800, addr_serdes);
        __logError("\n[0x%x] = 0x%x", addr_serdes, rd_serdes);
    }
    __logError("\nAMS_COM:PLL0 [0x0800]");
    for (addr_serdes = 0xd0B0; addr_serdes <= 0xd0BA; addr_serdes++)
    {
        rd_serdes = readPONSerdesReg(0x0800, addr_serdes);
        __logError("\n[0x%x] = 0x%x", addr_serdes, rd_serdes);
    }
    __logError("\nAMS_COM:PLL1 [0x0900]");
    for (addr_serdes = 0xd0B0; addr_serdes <= 0xd0BA; addr_serdes++)
    {
        rd_serdes = readPONSerdesReg(0x0900, addr_serdes);
        __logError("\n[0x%x] = 0x%x", addr_serdes, rd_serdes);
    }
    __logError("\nSIGDET, TLB_RX/TX, DIG_COM, PATT_GEN, TX_FED ");
    for (addr_serdes = 0xD0C0; addr_serdes <= 0xD11B; addr_serdes++)
    {
        rd_serdes = readPONSerdesReg(0x0800, addr_serdes);
        __logError("\n[0x%x] = 0x%x", addr_serdes, rd_serdes);
    }
    __logError("\nPLL_CAL_COM:PLL0 [0x0800]");
    for (addr_serdes = 0xD120; addr_serdes <= 0xD129; addr_serdes++)
    {
        rd_serdes = readPONSerdesReg(0x0800, addr_serdes);
        __logError("\n[0x%x] = 0x%x", addr_serdes, rd_serdes);
    }
    __logError("\nPLL_CAL_COM:PLL1 [0x0900]");
    for (addr_serdes = 0xD120; addr_serdes <= 0xD129; addr_serdes++)
    {
        rd_serdes = readPONSerdesReg(0x0900, addr_serdes);
        __logError("\n[0x%x] = 0x%x", addr_serdes, rd_serdes);
    }
    __logError("\nCORE_PLL_COM:PLL0 [0x0800]");
    for (addr_serdes = 0xD150; addr_serdes <= 0xD159; addr_serdes++)
    {
        rd_serdes = readPONSerdesReg(0x0800, addr_serdes);
        __logError("\n[0x%x] = 0x%x", addr_serdes, rd_serdes);
    }
    __logError("\nCORE_PLL_COM:PLL1 [0x0900]");
    for (addr_serdes = 0xD150; addr_serdes <= 0xD159; addr_serdes++)
    {
        rd_serdes = readPONSerdesReg(0x0900, addr_serdes);
        __logError("\n[0x%x] = 0x%x", addr_serdes, rd_serdes);
    }
}
