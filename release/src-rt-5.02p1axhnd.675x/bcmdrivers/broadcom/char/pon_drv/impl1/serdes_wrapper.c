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
 * serdes_wrapper.c -- Bcm Wan Serdes Wrapper
 *
 * Description:
 *      This file contains wrapper for Serdes driver
 *
 * Authors: Fuguo Xu, Akiva Sadovski
 *
 * $Revision: 1.1 $
 *
 * $Id: serdes_wrapper.c,v 1.1 2015/12/21 Fuguo Exp $
 *
 * $Log: serdes_wrapper.c,v $
 * Revision 1.1  2015/12/21 Fuguo
 * Initial version.
 *
 ****************************************************************************/

#include <linux/delay.h>
#include "common/srds_api_err_code.h"
#include "pon_drv.h"
#include "pmi_ag.h"


/* Serdes registers in full chip address map */
#define ADDR_DEVID_1                      0x08000000
#define ADDR_PLL_0                        0x00000000
#define ADDR_PLL_1                        0x01000000
#define ADDR_LANE_0                       0x00000000
#define ADDR_LANE_1                       0x00010000
#define ADDR_LANE_2                       0x00020000
#define ADDR_LANE_3                       0x00030000
#define ADDR_LANE_BRDCST                  0x00FF0000
#define ADDR_LANE_01                      0x02000000
#define ADDR_LANE_23                      0x02010000

DEFINE_SPINLOCK(serdes_lock);

/* LP Operation CMD */
#define LP_READ_OP       0
#define LP_WRITE_OP      1

static uint8_t g_pll_id = 0;
static uint8_t g_lane_id = 0;
static uint32_t g_pll_addr = ADDR_PLL_0;
static uint32_t g_lane_addr = ADDR_LANE_0;
#define MAPPED_ADDR(addr) (ADDR_DEVID_1 | g_pll_addr | g_lane_addr | addr)

typedef struct pmi_lp_3_s
{
    bdmf_boolean pmi_lp_err;
    bdmf_boolean pmi_lp_ack;
    uint16_t pmi_lp_rddata;
} pmi_lp_3_t;

static err_code_t access_serdes_reg(bool ctrl_cmd, bool do_read, uint16_t addr, uint16_t wr_data, uint16_t mask_data, uint16_t *rddata)
{
    pmi_lp_3_t pmi_lp_3;

    spin_lock_bh(&serdes_lock);

    ag_drv_pmi_lp_1_set(MAPPED_ADDR(addr));
    ag_drv_pmi_lp_2_set(wr_data, mask_data);
#if defined(CONFIG_BCM963158)
    ag_drv_pmi_lp_0_set(1, 1, ctrl_cmd);
#else
    ag_drv_pmi_lp_0_set(1, ctrl_cmd);
#endif

    /*  wait for ack to set after enable is set */
    udelay(5);

    ag_drv_pmi_lp_3_get(&pmi_lp_3.pmi_lp_err, &pmi_lp_3.pmi_lp_ack, &pmi_lp_3.pmi_lp_rddata);

    /* log errors */
    if (0 == pmi_lp_3.pmi_lp_ack)
        __logNotice("No pmi_lp_ack");

    if (1 == pmi_lp_3.pmi_lp_err)
        __logNotice("pmi_lp_err");

    /* clear enable */
#if defined(CONFIG_BCM963158)
    ag_drv_pmi_lp_0_set(0, 0, 0);
#else
    ag_drv_pmi_lp_0_set(0, 0);
#endif

    /* wait for ack to clear after enable is cleared */
    udelay(5);

    *rddata = pmi_lp_3.pmi_lp_rddata;

    spin_unlock_bh(&serdes_lock);

    return ((!pmi_lp_3.pmi_lp_ack || pmi_lp_3.pmi_lp_err) ? ERR_CODE_POLLING_TIMEOUT : ERR_CODE_NONE);
}

err_code_t eagle_onu10g_pmd_mwr_reg(uint16_t addr, uint16_t mask, uint8_t lsb, uint16_t val)
{
    uint16_t data;
    return access_serdes_reg (LP_WRITE_OP, 0, addr, val << lsb, ~mask, &data);
}

err_code_t eagle_onu10g_pmd_rdt_reg(uint16_t address, uint16_t *data)
{
    return access_serdes_reg (LP_READ_OP, 1, address, 0x0000, 0x0000, data);
}

err_code_t eagle_onu10g_pmd_wr_reg(uint16_t address, uint16_t val)
{
    uint16_t data;
    return access_serdes_reg (LP_WRITE_OP, 0, address, val, 0x0000, &data);
}

int logger_write(int message_verbose_level, const char *format, ...)
{
    va_list args;
    int r;

    va_start(args, format);
    r = vprintk(format, args);
    va_end(args);
    return r;
}

err_code_t eagle_onu10g_set_lane(uint8_t lane_index)
{
    switch (lane_index)
    {
    case 0:
        g_lane_addr = ADDR_LANE_0;
        break;

    default:
        __logError("lane_index(%d) unknown", lane_index);
        return ERR_CODE_INVALID_RAM_ADDR;
    }
    g_lane_id = lane_index;
    return ERR_CODE_NONE;
}

uint8_t eagle_onu10g_get_lane(void)
{
    return g_lane_id;
}

err_code_t eagle_onu10g_set_pll(uint8_t pll_index)
{
    switch (pll_index)
    {
    case 0:
        g_pll_addr = ADDR_PLL_0;
        break;

    case 1:
        g_pll_addr = ADDR_PLL_1;
        break;

    default:
        __logError("pll_index(%d) unknown", pll_index);
        return ERR_CODE_INVALID_RAM_ADDR;
    }
    g_pll_id = pll_index;
    return ERR_CODE_NONE;
}

uint8_t eagle_onu10g_get_pll(void)
{
    return g_pll_id;
}

err_code_t eagle_onu10g_delay_us(uint32_t delay_us)
{
    udelay(delay_us);
    return ERR_CODE_NONE;
}

err_code_t eagle_onu10g_delay_ns(uint16_t delay_ns)
{
    ndelay(delay_ns);
    return ERR_CODE_NONE;
}

uint8_t eagle_onu10g_get_core(void)
{
    return 0;
}

err_code_t eagle_onu10g_uc_lane_idx_to_system_id(char string[16], uint8_t uc_lane_idx)
{
    sprintf(string, "%u", uc_lane_idx);
    return ERR_CODE_NONE;
}

