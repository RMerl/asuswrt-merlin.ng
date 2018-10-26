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
/***********************************************************************/
/*                                                                     */
/*   MODULE:  merlin_serdes_api.c                                      */
/*   DATE:    30/11/2015                                               */
/*   PURPOSE: BCM APIs for Merlin Serdes                               */
/*                                                                     */
/***********************************************************************/
#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#define printk  printf
extern void *lib_memset(void *dest,int c,size_t cnt);
#define memset lib_memset
#else // Linux
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>
#endif

#include "lport_defs.h"
#include "serdes_access.h"
#include "merlin_serdes.h"

#include "merlin_mptwo_functions.h"
#include "merlin_mptwo_interface.h"
#include "merlin_mptwo_functions.c"
#include "merlin_mptwo_ucode_image.h"

#include "merlin_serdes_internal.c"

#define MERLIN_SCRIPT
#define MERLIN_NO_SDK

#ifdef MERLIN_DEBUG
#ifndef _CFE_
#define MERLIN_DBG(a)  printk(a)
#else
#define MERLIN_DBG(a)  xprintf(a)
#endif
#else
#define MERLIN_DBG(a)  
#endif

#ifndef _CFE_
#define MERLIN_ERR  printk
#else
#define MERLIN_ERR  xprintf
#endif

merlin_config_status_t g_merlin_status = {};

#define MERLIN_RETURN_CHECK(ret)    \
        do {                \
            if(ret != ERR_CODE_NONE)    \
            {       \
                _error(ret);        \
                return ret; \
            }       \
        } while (0);

#ifdef MERLIN_NO_SDK

#define MASK_BIT(a) (1<<(a))
#define MASK_BITS_TO(x) ((1<<(x))-1)
#define MASK_BITS_BETWEEN(l, h) (MASK_BITS_TO(((h)+1)) & ~(MASK_BITS_TO(l)))
#define MASK_ALL_BITS_16 (0xFFFF)

static int merlin_no_sdk_microcode_load(E_MERLIN_ID merlin_id, uint8_t *code_image, uint16_t code_len)
{
    uint32_t lane_id;
    uint16_t is_init_done;
    uint16_t padded_len, data, count = 0;
    uint8_t lsb_data;
    uint16_t load_result0 = 0;
    uint16_t load_result1 = 0;

    lane_id = merlin_id * MERLIN_LANES_PER_CORE;

    MERLIN_WRITE_REG(0x1, lane_id, 0xd0f2, MASK_BIT(6), 6, 0x1); /*uc_active*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0f2, MASK_BIT(0), 0, 0x1); /*core_dp_s_rstb*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd20d, MASK_BIT(1), 1, 0x1); /*micro_system_reset_n*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd20d, MASK_BIT(0), 0, 0x1); /*micro_system_clk_en*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(15), 15, 0x1); /*init_cmd*/

    merlin_mptwo_delay_us(300);
    
    MERLIN_READ_REG(0x1, lane_id, 0xd205, MASK_BIT(15), 15, &is_init_done);
    if (!is_init_done)
    {
        MERLIN_ERR("Failed to init serdes %d engine\n", merlin_id);
        return -1;
    }

    padded_len = (code_len + 7) & 0xfff8 ; /* number of bytes to be writen, have to be multiple of 4 */ 


    MERLIN_WRITE_REG(0x1, lane_id, 0xd200, MASK_ALL_BITS_16, 0, padded_len - 1); /*ram_cnt*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd201, MASK_ALL_BITS_16, 0, 0x0); /*ram_addr*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(1), 1, 0x0); /*stop*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(3), 3, 0x1); /*write */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(0), 0, 0x1); /*run*/
    for (count = 0; count < padded_len; count+=2)
    {
        lsb_data = (count < code_len) ? code_image[count] : 0x0;
        data = ((count + 1) < code_len) ? code_image[count + 1] : 0x0;
        data = (data << 8) | lsb_data;
    
        MERLIN_WRITE_REG(0x1, lane_id, 0xd203, MASK_ALL_BITS_16, 0, data);
    }

    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(3), 3, 0x0); /*write */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(0), 0, 0x0); /*run*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(1), 1, 0x1); /* stop */
    /* Verify microcode load */
    MERLIN_READ_REG(0x1, lane_id, 0xd205, MASK_BIT(0), 0, &load_result0);
    MERLIN_READ_REG(0x1, lane_id, 0xd205, MASK_BIT(1), 1, &load_result1);
    if (load_result0 || load_result1)
    {
        MERLIN_ERR("Failed load firmware ERR0: %d ERR1: %d\n", load_result0, load_result1);
        return -2;
    }

    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(4), 4, 0x1); /* mdio_dw8051_reset_n */
    return 0;
}


int merlin_no_sdk_core_init(E_MERLIN_ID merlin_id, E_MERLIN_VCO pll)
{
    int rc;
    uint32_t lane_id;
    int i;

    rc = merlin_no_sdk_microcode_load(merlin_id, merlin_mptwo_ucode_image, MERLIN_MPTWO_UCODE_IMAGE_SIZE);
    if (rc)
    {
        MERLIN_ERR("Failed to load microcode to serdes%d %d\n",merlin_id, rc);
        return rc;
    }

    if (pll == MERLIN_VCO_9375_MHZ)
    {
        for (i = 0; i < 2; i++)
        {
            lane_id = (merlin_id * MERLIN_LANES_PER_CORE) + i;
            MERLIN_WRITE_REG(0x3, lane_id, 0x9201, MASK_ALL_BITS_16, 0 , 0x2999); /*os_mode_cl36*/
            MERLIN_WRITE_REG(0x3, lane_id, 0x9202, MASK_ALL_BITS_16, 0 , 0x6620); /*pmd_osr_mode*/
            MERLIN_WRITE_REG(0x3, lane_id, 0x9270, MASK_BITS_BETWEEN(0, 13), 0, 0x3); /* reg2p5G_ClockCount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9271, MASK_BITS_BETWEEN(0, 12), 0, 0x1); /* reg2p5G_CGC */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9272, MASK_ALL_BITS_16, 0, 0x102); /*reg2p5G_loopcount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9273, MASK_BITS_BETWEEN(0, 12), 0, 0x6); /* reg2p5G_loopcount1 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x926a, MASK_BITS_BETWEEN(0, 13), 0, 0x19); /* reg1G_ClockCount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x926b, MASK_BITS_BETWEEN(0, 12), 0, 0x4); /* reg1G_CGC */
            MERLIN_WRITE_REG(0x3, lane_id, 0x926c, MASK_ALL_BITS_16, 0, 0x105); /*reg1G_loopcount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x926d, MASK_BITS_BETWEEN(0, 12), 0, 0xa); /* reg1G_loopcount1 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9232, MASK_BITS_BETWEEN(0, 8), 0, 0x3); /* reg1G_modulo */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9265, MASK_BITS_BETWEEN(0, 13), 0, 0x177); /* reg100M_ClockCount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9268, MASK_BITS_BETWEEN(0, 12), 0, 0x17); /* reg100M_CGC */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9266, MASK_ALL_BITS_16, 0, 0x100); /*reg100M_loopcount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9269, MASK_BITS_BETWEEN(13, 15), 13, 0x0); /* reg100M_loopcount1_hi */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9268, MASK_BITS_BETWEEN(13, 15), 13, 0x0); /* reg100M_loopcount1_lo */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9267, MASK_BITS_BETWEEN(0, 13), 0, 0x4b); /* reg100M_PCS_ClockCount0*/
            MERLIN_WRITE_REG(0x3, lane_id, 0x9269, MASK_BITS_BETWEEN(0, 12), 0, 0x25); /* reg100M_PCS_CGC */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9231, MASK_BITS_BETWEEN(0, 8), 0, 0x16); /* reg100M_modulo */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9260, MASK_BITS_BETWEEN(0, 13), 0, 0x753); /* reg10M_ClockCount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9263, MASK_BITS_BETWEEN(0, 12), 0, 0xea); /* reg10M_CGC */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9261, MASK_ALL_BITS_16, 0, 0x100); /*reg10M_loopcount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9264, MASK_BITS_BETWEEN(13, 15), 13, 0x0); /* reg10M_loopcount1_hi */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9263, MASK_BITS_BETWEEN(13, 15), 13, 0x0); /* reg10M_loopcount1_lo */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9262, MASK_BITS_BETWEEN(0, 13), 0, 0x4b); /* reg10M_PCS_ClockCount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9264, MASK_BITS_BETWEEN(0, 12), 0, 0x25); /* reg10M_PCS_CGC */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9230, MASK_BITS_BETWEEN(0, 8), 0, 0xe9); /* reg10M_modulo */
        }
    }

    lane_id = merlin_id * MERLIN_LANES_PER_CORE;

    MERLIN_WRITE_REG(0x1, lane_id, 0xd0b9, MASK_BIT(0), 0 , 0x0);

    if (pll == MERLIN_VCO_9375_MHZ)
    {
        MERLIN_WRITE_REG(0x1, lane_id, 0xd0b3, MASK_BITS_BETWEEN(11, 15), 11, 0x1a);
    }
    else
    {
        MERLIN_WRITE_REG(0x1, lane_id, 0xd0b3, MASK_BITS_BETWEEN(11, 15), 11, 0x4);
        MERLIN_WRITE_REG(0x1, lane_id, 0xd0b6, MASK_BITS_BETWEEN(12, 15), 12, 0x0);
        MERLIN_WRITE_REG(0x1, lane_id, 0xd0b7, MASK_BITS_BETWEEN(0, 13), 0 , 0x1000);
        MERLIN_WRITE_REG(0x1, lane_id, 0xd0b8, MASK_BITS_BETWEEN(0, 14), 0 , 0x6cce);
        MERLIN_WRITE_REG(0x1, lane_id, 0xd0b9, MASK_BITS_BETWEEN(0, 2), 0 , 0x0003);
    }
    MERLIN_WRITE_REG(0x3, lane_id, 0x9100, MASK_BITS_BETWEEN(13, 15), 13, 0x6);
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0e3, MASK_BIT(1), 1 , 0x1); /* for lane 0 of current core*/
    MERLIN_WRITE_REG(0x1, (lane_id+1), 0xd0e3, MASK_BIT(1), 1 , 0x1);/*for lane 1 of current core*/

    /*AN speed up needed for IEEE CL37 AN mode*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9107, MASK_BIT(15), 15, 0x1); /*tick_override*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9107, MASK_BITS_BETWEEN(0, 14), 0, 0x1); /*tick_nummerator_upper*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9108, MASK_BITS_BETWEEN(2, 11), 2, 0x1); /*tick_denummerator*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9250, MASK_ALL_BITS_16, 0, 0x1F); /*cl37_restart_timer*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9251, MASK_ALL_BITS_16, 0, 0x1F); /*cl37_ack_timer*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9252, MASK_ALL_BITS_16, 0, 0x0); /*cl37_error_timer*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9253, MASK_ALL_BITS_16, 0, 0x5); /*cl73_break_link_timer*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9256, MASK_ALL_BITS_16, 0, 0x1f); /*cl73_link_up*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x925c, MASK_ALL_BITS_16, 0, 0xf); /*ignore_link_timer*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x924a, MASK_ALL_BITS_16, 0, 0xf); /*tx_reset_timer_period */

    return 0;
}

int merlin_no_sdk_deassert_core(E_MERLIN_ID merlin_id)
{
    uint32_t lane_id;

    lane_id = merlin_id * MERLIN_LANES_PER_CORE;
    //deassert core
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0f2, MASK_ALL_BITS_16, 0, 0x1);

    return 0;
}

int merlin_no_sdk_lane_init(E_MERLIN_LANE lane_id, lport_serdes_cfg_s *serdes_cfg)
{
    merlin_lane_type_t  lane_type;
    merlin_lane_type_get(serdes_cfg->prt_mux_sel, &lane_type);
    g_merlin_status.lane_type[lane_id] = lane_type;
    g_merlin_status.an_mode[lane_id] = serdes_cfg->autoneg_mode;

    switch (serdes_cfg->prt_mux_sel)
    {
    case PORT_XFI:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*creadit_sw_en*/
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x1); /*SW_actual_speed_force_en*/
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0xf); /* SW_actual_speed */
       break;
    case PORT_SGMII_SLAVE:
    case PORT_SGMII_1000BASE_X:
        switch (serdes_cfg->autoneg_mode)
        {
        case MERLIN_AN_NONE:
            /* Force 1G */
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*creadit_sw_en*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x1); /*SW_actual_speed_force_en*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x2); /* SW_actual_speed */
            break; 
        case MERLIN_AN_IEEE_CL37:
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x1); /*use_ieee_reg_ctrl_sel*/
            MERLIN_WRITE_REG(0x0, lane_id, 0x0004, MASK_BIT(5), 5, 0x1); /* dplex */
            MERLIN_WRITE_REG(0x0, lane_id, 0x0000, MASK_BIT(12), 12, 0x1); /*AN enable*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*credit_sw_en*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x0); /*SW_actual_speed_force_en*/
            break;
        case MERLIN_AN_USER_CL37:
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x2); /*use_ieee_reg_ctrl_sel*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc481, MASK_BIT(4), 4, 0x1); /* dplex */
            MERLIN_WRITE_REG(0x3, lane_id, 0xc480, MASK_BIT(6), 6, 0x1); /*AN enable*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*credit_sw_en*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x0); /*SW_actual_speed_force_en*/
            break;
        case MERLIN_AN_SGMII_MASTER:
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x2); /*use_ieee_reg_ctrl_sel*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc481, MASK_BIT(2), 2, 0x1); /* dplex */
            MERLIN_WRITE_REG(0x3, lane_id, 0xc481, MASK_BITS_BETWEEN(0,1), 0, 0x2); /* speed */
            MERLIN_WRITE_REG(0x3, lane_id, 0xc481, MASK_BIT(9), 9, 0x1); /* mater */
            MERLIN_WRITE_REG(0x3, lane_id, 0xc480, MASK_BITS_BETWEEN(6,7), 6, 0x3); /*AN enable*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*credit_sw_en*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x0); /*SW_actual_speed_force_en*/
            break;
        case MERLIN_AN_SGMII_SLAVE:
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x2); /*use_ieee_reg_ctrl_sel*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc480, MASK_BITS_BETWEEN(6,7), 6, 0x3); /*AN enable*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*credit_sw_en*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x0); /*SW_actual_speed_force_en*/
            break;
        }
        break;
    case PORT_HSGMII:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*creadit_sw_en*/
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x1); /*SW_actual_speed_force_en*/
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x3); /* SW_actual_speed */
       break;
    default:
        return -1;
    }

    MERLIN_WRITE_REG(0x3, lane_id, 0xc457, MASK_BIT(0), 0, 0x1); /*rx_rstb_lane*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(7), 7, 0x1); /*mac_credentable*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, MASK_BIT(1), 1, 0x1); /*tx_rstb_lane*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, MASK_BIT(0), 0, 0x1); /*enable_tx_lane*/
 
    return 0;
}

int merlin_no_sdk_lane_enable(E_MERLIN_LANE lane_id)
{
    MERLIN_WRITE_REG(0x3, lane_id, 0xc457, ~0xfffe, 0, 0x0);
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, ~0xfffc, 0, 0x0);
    MERLIN_WRITE_REG(0x3, lane_id, 0xc457, ~0xfffe, 0, 0x1);
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, ~0xfffc, 0, 0x3);

    MERLIN_WRITE_REG(0x1, lane_id, 0xd081, MASK_BIT(1), 1, 0x1);
    return 0;
}

int merlin_no_sdk_get_status(E_MERLIN_LANE lane_id, lport_port_status_s *port_status)
{
    uint16_t port_speed;
    uint16_t port_up;

    MERLIN_READ_REG(0x3, lane_id, 0xc474, MASK_BIT(1), 1, &port_up);
    MERLIN_READ_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, &port_speed);
    port_status->port_up = port_up;
    switch (port_speed)
    {
    case 0:
        port_status->rate = LPORT_RATE_10MB;
        break;
    case 1:
        port_status->rate = LPORT_RATE_100MB;
        break;
    case 2:
        port_status->rate = LPORT_RATE_1000MB;
        break;
    case 3:
        port_status->rate = LPORT_RATE_2500MB;
        break;
    case 4:
        port_status->rate = LPORT_RATE_2500MB;
        break;
    case 0xD:
    case 0x34:
        port_status->rate = LPORT_RATE_1000MB;
        break;
    case 0xf:
    case 0x1b:
    case 0x1f:
    case 0x33:
        port_status->rate = LPORT_RATE_10G;
        break;
    default:
        port_status->rate = LPORT_RATE_UNKNOWN;
        break;
    }
    port_status->duplex = LPORT_FULL_DUPLEX; 
    return 0;
}

int merlin_no_sdk_change_speed(E_MERLIN_LANE lane_id, LPORT_PORT_RATE port_rate)
{
    MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*creadit_sw_en*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x1); /*SW_actual_speed_force_en*/
    switch (port_rate)
    {
    case LPORT_RATE_10G:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0xf); /* SW_actual_speed */
        break;
    case LPORT_RATE_2500MB:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x3); /* SW_actual_speed */
        break;
    case LPORT_RATE_1000MB:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x2); /* SW_actual_speed */
        break;
    case LPORT_RATE_100MB:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x1); /* SW_actual_speed */
        break;
    default:
        MERLIN_ERR("Trying to set invalid port speed %d \n", port_rate);
        return -1;
        break;
    }

    MERLIN_WRITE_REG(0x3, lane_id, 0xc457, MASK_BIT(0), 0, 0x1); /*rx_rstb_lane*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(7), 7, 0x1); /*mac_credentable*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, MASK_BIT(1), 1, 0x1); /*tx_rstb_lane*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, MASK_BIT(0), 0, 0x1); /*enable_tx_lane*/
 
    MERLIN_WRITE_REG(0x1, lane_id, 0xd081, MASK_BIT(1), 1, 0x1);
    return 0;
}

#endif

#ifdef MERLIN_SCRIPT
int merlin_init(E_MERLIN_ID merlin_id, E_MERLIN_VCO pll)
{
    uint32_t lane_index;

    printk("merlin script sequence\n");
    
    //pll
    lane_index = merlin_id * MERLIN_LANES_PER_CORE;
    MERLIN_WRITE_REG(0x1, lane_index, 0xd0b9, ~0x0000/*mask*/, 0/*shift*/, 0x0000);
    MERLIN_WRITE_REG(0x1, lane_index, 0xd0b3, ~0x7ff, 11, 0x4);
    MERLIN_WRITE_REG(0x1, lane_index, 0xd0b6, ~0xfff,  12, 0x0);
    MERLIN_WRITE_REG(0x1, lane_index, 0xd0b7, ~0xc000, 0, 0x1000);
    MERLIN_WRITE_REG(0x1, lane_index, 0xd0b8, ~0x8000, 0, 0x6cce);
    MERLIN_WRITE_REG(0x1, lane_index, 0xd0b9, ~0xfff8, 0, 0x0003);
    MERLIN_WRITE_REG(0x3, lane_index, 0x9100, ~0x1fff, 13, 0x6);
    MERLIN_WRITE_REG(0x1, lane_index, 0xd0e3, ~0xfffd, 1, 0x1);

    //1G
    MERLIN_WRITE_REG(0x3, lane_index, 0xc30b, ~0xf780, 0,0x0042);
    MERLIN_WRITE_REG(0x3, lane_index, 0xc457, ~0xfffe, 0,0x0001);
    MERLIN_WRITE_REG(0x3, lane_index, 0xc30b, ~0xff7f, 7,0x0001);
    MERLIN_WRITE_REG(0x3, lane_index, 0xc433, ~0xfffc, 0, 0x0003);

    //deassert core
    MERLIN_WRITE_REG(0x1, lane_index, 0xd0f2, ~0x0000, 0, 0x1);

    merlin_mptwo_delay_us(10000);

    MERLIN_WRITE_REG(0x3, lane_index, 0xc457, ~0xfffe, 0, 0x0);
    MERLIN_WRITE_REG(0x3, lane_index, 0xc433, ~0xfffc, 0, 0x0);
    MERLIN_WRITE_REG(0x3, lane_index, 0xc457, ~0xfffe, 0, 0x1);
    MERLIN_WRITE_REG(0x3, lane_index, 0xc433, ~0xfffc, 0, 0x3);
  return 0;  
  }
#else
int merlin_init(E_MERLIN_ID merlin_id, E_MERLIN_VCO pll)
{
    merlin_access_t ma_in, *ma=&ma_in;
    err_code_t ret = ERR_CODE_NONE;
    struct merlin_mptwo_uc_core_config_st core_config;
    uint32_t i=0;
    uint8_t gp_uc_req, supp_info;
    
    MERLIN_DBG(("%s: merllin_id %d, pll %d\n", __FUNCTION__, merlin_id, pll));

    ma_in.index = merlin_id * MERLIN_LANES_PER_CORE;

    wrc_pmd_s_rstb(0);
    wrc_pmd_s_rstb(1);

    merlin_mptwo_uc_reset(ma, 1);
    ret = merlin_mptwo_ucode_mdio_load(ma, merlin_mptwo_ucode_image, MERLIN_MPTWO_UCODE_IMAGE_SIZE);
    MERLIN_RETURN_CHECK(ret);
    
    ret = merlin_mptwo_ucode_load_verify(ma, merlin_mptwo_ucode_image, MERLIN_MPTWO_UCODE_IMAGE_SIZE);
    MERLIN_RETURN_CHECK(ret);
    
    ret = merlin_mptwo_uc_active_enable(ma, 1);
    MERLIN_RETURN_CHECK(ret);

    merlin_mptwo_uc_reset(ma, 0);
    
    EFUN(wrc_micro_mdio_dw8051_reset_n(1));
    
    merlin_mptwo_delay_us(10000);
    
    for(i = 0; i < MERLIN_LANES_PER_CORE; i++)
    {
        ma_in.index = (merlin_id * MERLIN_LANES_PER_CORE) + i;
        ESTM(ret = rd_uc_dsc_ready_for_cmd());
        if (!ret)
        {
            ESTM(ret = rd_uc_dsc_error_found());
            if (ret)
            {
                ESTM(gp_uc_req = rd_uc_dsc_gp_uc_req());
                ESTM(supp_info = rd_uc_dsc_supp_info());
                logger_write(-1, "DSC command returned error (after cmd) cmd = x%x, supp_info = x%x!", gp_uc_req, supp_info);
                return(_error(ERR_CODE_UC_CMD_RETURN_ERROR));
            }
            return (ERR_CODE_NONE);
        }
    }

    ma_in.index = merlin_id * MERLIN_LANES_PER_CORE;

    merlin_mptwo_uc_reset(ma, 1);
    wrc_pmd_s_rstb(0);
    wrc_pmd_s_rstb(1);
    ret = merlin_mptwo_uc_active_enable(ma, 1);
    MERLIN_RETURN_CHECK(ret);
    merlin_mptwo_uc_reset(ma, 0);

    ret = merlin_mptwo_uc_active_enable(ma, 1);
    MERLIN_RETURN_CHECK(ret);
    ret = merlin_mptwo_isolate_core_ctrl_pins(ma, 1);
    MERLIN_RETURN_CHECK(ret);
    
    wrc_heartbeat_count_1us(500);
    memset(&core_config, 0 , sizeof(struct merlin_mptwo_uc_core_config_st));
    core_config.field.vco_rate = 19;							// Setting = Round(Frequency_GHz/0.25) – 22
    wrcv_afe_hardware_version(0x1);								// Set AFE HW version to 0x1
    ret = merlin_mptwo_set_uc_core_config(ma, core_config);
    MERLIN_RETURN_CHECK(ret);
    
    if(pll == MERLIN_VCO_103125_MHZ)
         ret = merlin_mptwo_configure_pll(ma,MERLIN_MPTWO_pll_10p3125GHz_50MHz);
    else
         ret = merlin_mptwo_configure_pll(ma,MERLIN_MPTWO_pll_9p375GHz_50MHz);

    for (i = 0; i < MERLIN_LANES_PER_CORE; i++) {
        ma_in.index = (merlin_id * MERLIN_LANES_PER_CORE) + i;
        wr_ln_dp_s_rstb(0);   
    }

    ma_in.index = merlin_id * MERLIN_LANES_PER_CORE;
    wrc_core_dp_s_rstb(1);

    return ret;
}
#endif

int merlin_link_config_set(E_MERLIN_LANE lane_index, lport_serdes_cfg_s *serdes_cfg)
{
#ifndef MERLIN_SCRIPT
    merlin_access_t ma_in, *ma=&ma_in;
    err_code_t ret = ERR_CODE_NONE;
    merlin_lane_type_t  lane_type;
    
    MERLIN_DBG(("%s: lane_index %d, prt_mux_sel %d, rate %d\n", __FUNCTION__, lane_index, serdes_cfg->prt_mux_sel, serdes_cfg->speed));
  
    ma->index = lane_index;
    merlin_lane_type_get(serdes_cfg->prt_mux_sel, &lane_type);
    g_merlin_status.lane_type[lane_index] = lane_type;
   
    if(lane_type == MERLIN_INTERFACE_UNUSED)
        MERLIN_RETURN_CHECK(ERR_CODE_BAD_LANE);

    merlin_mptwo_isolate_lane_ctrl_pins(ma, 1);
    merlin_lane_assert_reset(ma, 1);
    merlin_link_optical_configure(lane_index, SFI_OPTICS_OSx1, 0);
    ret = merlin_link_type_configure(lane_index, lane_type);
    MERLIN_RETURN_CHECK(ret);
    merlin_lane_assert_reset(ma, 0);
#endif
    return ERR_CODE_NONE;
}   

int merlin_link_config_get(E_MERLIN_LANE lane_index, lport_serdes_cfg_s *serdes_cfg)
{
    switch(g_merlin_status.lane_type[lane_index])
    {
        case MERLIN_INTERFACE_XFI:
            serdes_cfg->prt_mux_sel = PORT_XFI;
            serdes_cfg->speed = LPORT_RATE_10G;
            serdes_cfg->autoneg_mode = g_merlin_status.an_mode[lane_index];
            break;
        case MERLIN_INTERFACE_HSGMII:
            serdes_cfg->prt_mux_sel = PORT_HSGMII;
            serdes_cfg->speed = LPORT_RATE_2500MB;
            serdes_cfg->autoneg_mode = g_merlin_status.an_mode[lane_index];
            break;
        case MERLIN_INTERFACE_SGMII:
            serdes_cfg->speed = LPORT_RATE_1000MB;
            serdes_cfg->autoneg_mode = g_merlin_status.an_mode[lane_index];
            if (serdes_cfg->autoneg_mode == MERLIN_AN_IEEE_CL37)
                serdes_cfg->prt_mux_sel = PORT_SGMII_1000BASE_X;
            else
                serdes_cfg->prt_mux_sel = PORT_SGMII_SLAVE;
            break;
        default:
            serdes_cfg->prt_mux_sel = PORT_UNAVAIL;
            break;
    }
    
    return 0;
}
     
int merlin_datapath_reset(E_MERLIN_ID merlin_id)
{ 
#ifndef MERLIN_SCRIPT
    merlin_access_t ma_in, *ma=&ma_in;
   
    MERLIN_DBG(("%s: merlin_id %d\n", __FUNCTION__,merlin_id));

    ma_in.index = merlin_id*MERLIN_LANES_PER_CORE;
    wrc_core_dp_s_rstb(1);
    merlin_mptwo_delay_us(1);
#endif
    return ERR_CODE_NONE;
}

int merlin_read_register(uint16_t dev_type, uint16_t lane_index, uint16_t reg_addr, 
                                                    uint16_t mask, uint8_t shift, uint16_t* value)
{
    MERLIN_READ_REG(dev_type, lane_index, reg_addr, mask, shift, value);
    
    return ERR_CODE_NONE;
}

int merlin_write_register(uint16_t dev_type, uint16_t lane_index, uint16_t reg_addr, 
                                                    uint16_t mask, uint8_t shift, uint16_t value)
{
    MERLIN_WRITE_REG(dev_type, lane_index, reg_addr, mask, shift, value);

    return ERR_CODE_NONE;
}

int merlin_link_force_speed(uint16_t lane_index, LPORT_PORT_RATE speed)
{
    merlin_access_t ma_in, *ma=&ma_in;

    if(g_merlin_status.lane_type[lane_index] != MERLIN_INTERFACE_SGMII)
        MERLIN_RETURN_CHECK(ERR_CODE_BAD_LANE);

    ma->index = lane_index;
    merlin_lane_assert_reset(ma, 1);
    wr_pcs_an_cl37_enable(0);
    wr_pcs_an_cl37_sgmii_enable(0);
    wr_pcs_an_sgmii_full_duplex(1);
    
    switch(speed)
    {
        case LPORT_RATE_10MB:
            wr_pcs_an_sgmii_speed(0);
            break;
        case LPORT_RATE_100MB:
            wr_pcs_an_sgmii_speed(1);
            break;
        case LPORT_RATE_1000MB:
            wr_pcs_an_sgmii_speed(2);
            break;
        case LPORT_RATE_2500MB:
        case LPORT_RATE_10G:
        case LPORT_RATE_UNKNOWN:
            return ERR_CODE_NOT_SUPPORTED;
            break;
    }
    merlin_lane_assert_reset(ma, 0);
    return ERR_CODE_NONE;
}

int merlin_link_enable(uint16_t lane_index)
{
    merlin_access_t ma_in, *ma=&ma_in;
    
    if(g_merlin_status.lane_type[lane_index] == MERLIN_INTERFACE_UNUSED)
        MERLIN_RETURN_CHECK(ERR_CODE_BAD_LANE);
    
    ma->index = lane_index;
    wr_pcs_tx_enable_tx_lane(1);
    return ERR_CODE_NONE;
}


int merlin_link_disable(uint16_t lane_index)
{
    merlin_access_t ma_in, *ma=&ma_in;
        
    if(g_merlin_status.lane_type[lane_index] == MERLIN_INTERFACE_UNUSED)
        MERLIN_RETURN_CHECK(ERR_CODE_BAD_LANE);
    
    ma->index = lane_index;
    wr_pcs_tx_enable_tx_lane(0);

    return ERR_CODE_NONE;
}

int merlin_link_status_get(uint16_t lane_index, uint8_t *link_up)
{
    merlin_access_t ma_in, *ma=&ma_in;
	
    ma->index = lane_index;
    ESTM(*link_up = rd_pcs_rx_link_status());

    return ERR_CODE_NONE;
}

int merlin_link_rate_and_duplex_get(uint16_t lane_index, uint16_t *speed, uint16_t *duplex)
{
    merlin_access_t ma_in, *ma=&ma_in;

    ma->index = lane_index;
    MERLIN_DBG(("%s: lane_index %d, lane type is %d\n", __FUNCTION__, lane_index, g_merlin_status.lane_type[lane_index]));
    switch(g_merlin_status.lane_type[lane_index])
        {
        case MERLIN_INTERFACE_XFI:
            *speed = LPORT_RATE_10G;
            *duplex = LPORT_FULL_DUPLEX;
            break;
        case MERLIN_INTERFACE_HSGMII:
            *speed = LPORT_RATE_2500MB;
            *duplex = LPORT_FULL_DUPLEX;
            break;
        case MERLIN_INTERFACE_SGMII:
            {
                uint16_t act_speed;
                ESTM(act_speed = rd_pcs_dig_sw_actual_speed());
                switch(act_speed){
                    case 0:
                        *speed = LPORT_RATE_10MB;
                        break;
                    case 1:
                        *speed = LPORT_RATE_100MB;
                        break;
                    case 2:
                        *speed = LPORT_RATE_1000MB;
                        break;
                    default:
                        *speed = LPORT_RATE_UNKNOWN;
                        break;
                }

                *duplex = LPORT_FULL_DUPLEX;
            }
            break;
        case MERLIN_INTERFACE_UNUSED:
            MERLIN_RETURN_CHECK(ERR_CODE_BAD_LANE);
            break;
        }
    return ERR_CODE_NONE;
}

int merlin_advertise_cap_set(uint16_t lane_index, merlin_autoneg_ability_t capability)
{
    uint16_t speed=0, duplex=0;
    merlin_access_t ma_in, *ma=&ma_in;

    if(g_merlin_status.lane_type[lane_index] != MERLIN_INTERFACE_SGMII)
        MERLIN_RETURN_CHECK(ERR_CODE_BAD_LANE);
    
    ma->index = lane_index;
    merlin_lane_assert_reset(ma, 1);
    
    if(capability.mode == MERLIN_AN_MODE_SGMII_SLAVE)
    {
        wr_pcs_an_sgmii_master_mode(0);         //Slave mode
        merlin_lane_assert_reset(ma, 0);
        return ERR_CODE_NONE;
    }

    if(capability.an_cap & MERLIN_CAP_10M)
    {
        speed = 0;
    }else if(capability.an_cap & MERLIN_CAP_100M)
    {
        speed = 1;
    }else if(capability.an_cap & MERLIN_CAP_1000M)
    {
        speed = 2;
    }

    if(capability.an_cap & MERLIN_CAP_FULL)
    {
        duplex = 1;
    }else if(capability.an_cap & MERLIN_CAP_HALF)
    {
        duplex = 0;
    }

    ma->index = lane_index;
    wr_pcs_an_sgmii_master_mode(1);             //Master mode
    wr_pcs_an_sgmii_speed(speed);
    wr_pcs_an_sgmii_full_duplex(duplex);
    merlin_lane_assert_reset(ma, 0);
    return ERR_CODE_NONE;
}

int merlin_advertise_cap_get(uint16_t lane_index, merlin_autoneg_ability_t *capability)
{
    uint16_t speed, duplex, mode;
    merlin_access_t ma_in, *ma=&ma_in;

    if(g_merlin_status.lane_type[lane_index] != MERLIN_INTERFACE_SGMII)
        MERLIN_RETURN_CHECK(ERR_CODE_BAD_LANE);

    ma->index = lane_index;
    ESTM(mode = rd_pcs_an_sgmii_master_mode());
    if(mode == 1)
    {
        ESTM(speed = rd_pcs_an_sgmii_speed());
        ESTM(duplex = rd_pcs_an_sgmii_full_duplex());
        switch(speed){
            case 0:
                capability->an_cap = MERLIN_CAP_10M;
                break;
            case 1:
                capability->an_cap = MERLIN_CAP_100M;
                break;
            case 2:
                capability->an_cap = MERLIN_CAP_1000M;
                break;
        }

        switch(duplex){
            case 0:
                capability->an_cap = MERLIN_CAP_HALF;
                break;
            case 1:
                capability->an_cap = MERLIN_CAP_FULL;
                break;
        }
        capability->mode = MERLIN_AN_MODE_SGMII_MASTER;
     }else
     {
        capability->an_cap = MERLIN_CAP_10M|MERLIN_CAP_100M|MERLIN_CAP_1000M|MERLIN_CAP_FULL;
        capability->mode = MERLIN_AN_MODE_SGMII_SLAVE;
     }
        
    return ERR_CODE_NONE;
}


int merlin_auto_enable(uint16_t lane_index)
{
    merlin_access_t ma_in, *ma=&ma_in;

    if(g_merlin_status.lane_type[lane_index] != MERLIN_INTERFACE_SGMII)
        MERLIN_RETURN_CHECK(ERR_CODE_BAD_LANE);

    ma->index = lane_index;  
    wr_pcs_an_cl37_enable(1);
    wr_pcs_an_cl37_sgmii_enable(1);
    wr_pcs_an_cl37_an_restart(1);   /*restart*/
    
    return ERR_CODE_NONE;
}

int merlin_auto_disable(uint16_t lane_index)
{
    merlin_access_t ma_in, *ma=&ma_in;

    if(g_merlin_status.lane_type[lane_index] != MERLIN_INTERFACE_SGMII)
        MERLIN_RETURN_CHECK(ERR_CODE_BAD_LANE);

    ma->index = lane_index;  
    wr_pcs_an_cl37_enable(0);
    wr_pcs_an_cl37_sgmii_enable(0);

    return ERR_CODE_NONE;
}

int merlin_loopback_set(uint16_t lane_index, merlin_loopback_mode mode, uint8_t enable)
{
    merlin_access_t ma_in, *ma=&ma_in;
    uint8_t lane_map;

    ma->index = lane_index;

    switch(mode)
    {
        case MERLIN_PCS_LOCAL_LOOPBACK:
            ESTM(lane_map = rdc_pcs_main_local_pcs_loopback_enable());
            if(enable != 0)
                lane_map |= 1<<lane_index;
            else
                lane_map &= ~(1<<lane_index);
            wr_signal_detect_frc(enable!=0);
            wrc_pcs_main_local_pcs_loopback_enable(lane_map);
            break;
        case MERLIN_PCS_REMOTE_LOOPBACK:
            ESTM(lane_map = rdc_pcs_main_remote_pcs_loopback_enable());
            if(enable != 0)
                lane_map |= 1<<lane_index;
            else
                lane_map &= ~(1<<lane_index);
            wrc_pcs_main_remote_pcs_loopback_enable(lane_map);
            break;
        case MERLIN_PMD_LOCAL_LOOPBACK:
            wr_signal_detect_frc(enable!=0);
            merlin_mptwo_dig_lpbk(ma,enable!=0);
            break;
        case MERLIN_PMD_REMOTE_LOOPBACK:
            merlin_mptwo_rmt_lpbk(ma,enable!=0);
            break;
        default:
            MERLIN_RETURN_CHECK(ERR_CODE_NOT_SUPPORTED);
            break;
    }

    return ERR_CODE_NONE;
}

int  merlin_control(uint16_t lane_index, merlin_command_t cmd, merlin_control_t *data)
{
    merlin_access_t ma_in, *ma=&ma_in;
    err_code_t ret = ERR_CODE_NONE;

    ma->index = lane_index;
    
    switch(cmd)
    {
        case MERLIN_CMD_STATUS_GET:
            ret = merlin_lane_status_get(ma, &data->status);
            break;
        case MERLIN_CMD_STATS_GET:
            ret = merlin_lane_stats_get(ma, &data->stats);
        case MERLIN_CMD_LP_STATUS_GET:
            break;
        case MERLIN_CMD_PRBS_STATS_GET:
            ret = merlin_lane_prbs_stats_get(ma, &data->prbs_stats);
            break;
        case MERLIN_CMD_FORCE_LINK_DOWN:
            MERLIN_WRITE_REG(MERLIN_PMD_PMA, lane_index, 0x09, 0x1, 1+lane_index, 0x0);
            break;
        case MERLIN_CMD_FORCE_LINK_UP:
            MERLIN_WRITE_REG(MERLIN_PMD_PMA, lane_index, 0x09, 0x1, 1+lane_index, 0x1);
            break;
        case MERLIN_CMD_ID:
            merlin_core_id_get(ma, &data->serdes_id);
            break;
        case MERLIN_CMD_UC_RAM_WRITE:
            //merlin_sequoia_wrb_uc_ram();
            break;
        case MERLIN_CMD_UC_RAM_READ:
            //merlin_sequoia_wrw_uc_ram();
            break;
        case MERLIN_CMD_REG_READ:
            MERLIN_READ_REG(data->reg_data.device_type, lane_index, data->reg_data.reg_addr, 0xffff, 0,
                &data->reg_data.reg_value);
            break;
        case MERLIN_CMD_REG_WRITE:
            MERLIN_WRITE_REG(data->reg_data.device_type, lane_index, data->reg_data.reg_addr, 0xffff, 0,
                data->reg_data.reg_value);
            break;
        default:
            MERLIN_RETURN_CHECK(ERR_CODE_NOT_SUPPORTED);
            break;
    }
    
    return ret;
}

int merlin_prbs_enable_set(uint16_t lane_index, merlin_prbs_mode_t mode, uint8_t enable)
{
    merlin_access_t ma_in, *ma=&ma_in;

    ma->index = lane_index;
    if(enable){
        wr_prbs_gen_mode_sel(mode);
        wr_prbs_gen_en(1);
    }
    else
    {
        wr_prbs_gen_en(0);
    }
    
    return ERR_CODE_NONE;
}

int merlin_prbs_check(uint16_t lane_index, uint8_t *locked, merlin_prbs_stats_t *stats)
{
    merlin_access_t ma_in, *ma=&ma_in;

    ma->index = lane_index;
    wr_prbs_chk_en(1);
    ESTM(*locked = rd_prbs_chk_lock());
    merlin_lane_prbs_stats_get(ma, stats);
    
    return ERR_CODE_NONE;
}

