/*  *********************************************************************
    *
    <:copyright-BRCM:2012:proprietary:standard
    
       Copyright (c) 2012 Broadcom 
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
    ********************************************************************* */

#ifndef __BCM63XX_IMP1_DDR_CINIT_H__
#define __BCM63XX_IMP1_DDR_CINIT_H__

#include "lib_types.h"

#define MC_DRAM_CFG_DDR3         (1)
// SP has one more than RR
#define MC_ARB_RR_Q_NUM          (21)
#define MC_ARB_SP_Q_NUM          (MC_ARB_RR_Q_NUM+1)
#define MC_UB_SRC_ID_NUM         (32)

#define MEM_SIZE_1024Mb           0 
#define MEM_SIZE_2048Mb           1 
#define MEM_SIZE_4096Mb           2

#define DDR_CLK_400MHZ_CL6        0
#define DDR_CLK_533MHZ_CL7        1
#define DDR_CLK_533MHZ_CL8        2
#define DDR_CLK_667MHZ_CL9        3
#define DDR_CLK_667MHZ_CL10       4
#define DDR_CLK_800MHZ_CL10       5
#define DDR_CLK_800MHZ_CL11       6


// MEMC configuration struct
typedef struct memc_bus_cfg_struct
{
    uint32_t gcfg_max_age;
    uint32_t arb_burst_mode;
    uint32_t arb_rr_mode;
    uint32_t arb_sp_sel[MC_ARB_SP_Q_NUM];
    uint32_t arb_sp_pri[MC_ARB_SP_Q_NUM];
    uint32_t arb_rr_qqw[MC_ARB_RR_Q_NUM];
    uint32_t ub0_srcid_q[MC_UB_SRC_ID_NUM];
    uint32_t ub0_oob_err_mask;
    uint32_t ub0_ib_err_mask;
    uint32_t ub0_wburst_mode;
    uint32_t ub0_wrep_mode;
    //uint32_t ub0_repreq_mode;
    uint32_t ub0_fifo_mode;
    uint32_t ub1_oob_err_mask;
    uint32_t ub1_ib_err_mask;
    uint32_t ub1_wburst_mode;
    uint32_t ub1_wrep_mode;
    //uint32_t ub1_repreq_mode;
    uint32_t ub1_fifo_mode;
    uint32_t axirif_fifo_mode;
    uint32_t axiwif_wburst_mode;
    uint32_t axiwif_wrep_mode;
    uint32_t axiwif_fifo_mode;
    uint32_t large_page;
} memc_bus_cfg;

// MEMC timing configuration struct
typedef struct memc_timing_cfg_struct {
    // MC_CHN_TIM_TIM1_0 register fields
    uint32_t tRCD;
    uint32_t tCL;
    uint32_t tWR;
    uint32_t tCwl;
    uint32_t tRP;
    uint32_t tRRD;

    // MC_CHN_TIM_TIM1_1 register fields
    uint32_t tRCr;
    uint32_t tFAW;
    uint32_t tRFC;
    uint32_t tW2R;
    uint32_t tR2W;
    uint32_t tR2R;

    // MC_CHN_TIM_TIM2 register fields
    uint32_t tAL;
    uint32_t tRTP;
    uint32_t tW2W;
    uint32_t tWTR;

    // For PHY int
    uint32_t tRAS;
    uint32_t tCCD;
}memc_timing_cfg;

// MEMC DRAM profile struct
typedef struct memc_dram_profile_struct {
    uint32_t dram_clk;  // the DRAM data clock speed in MHz (1/2 data rate)
    uint32_t ref_clk;  // the PHY reference clock speed in Hz(!!!)

    // DRAM size, number of banks, rows, columns
    uint32_t size_Mbits;
    uint32_t row_bits;
    uint32_t col_bits;
    uint32_t ba_bits;

    // DRAM width 
    uint32_t width_bits;
    // How many bytes at a time will DRAM acces?
    uint32_t access_bytes;
   
    // MEMC timing configuration is stored here
    memc_timing_cfg timing_cfg; 
}memc_dram_profile;

/* MCB related structure */
typedef struct mcbindex {
    uint32_t config;
    uint32_t mask;
    unsigned int *pMcbData;
}mcbindex;


int ddr_init(unsigned long mcb_selector, uint32_t memcfg_from_flash);

extern uint32_t memsys_init(uint32_t *, uint32_t *);

#endif
