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

#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"
#include "cfe.h"
#include "bcm_map.h"
#include "bcm_hwdefs.h"
#include "bcm63xx_impl1_ddr_cinit.h"
#include "armmacros.h"
#include "rom_parms.h"

//#define AUTO_SIZE

#define print_log(...)

int is_safemode = 0;
int is_16bit = 0;
int is_ht = 0;
int is_ssc = 0;
int total_size_mbit = 0;

extern void board_setleds(unsigned long);
extern int is_aliased(int max_bits);
extern void armv7_branch_predict_enable(void);
extern void armv7_branch_predict_disable(void);

extern mcbindex MCB[];

static void memc_delay(unsigned int ns);
static void mc_cfg_init(void);
static void init_memc_dram_profile(int size, int clock);
static void cfg_memc_timing_ctrl(void);
static void cfg_map_addr_bits_to_row_col_ba(void);
static void refresh_ctrl_cfg(uint32_t dram_clk_freq, int ref_disable);
static int  init_memc(int size, int clock);
#if 0
static void init_mc_arb(void);
static void init_mc_ubus_if(void);
static void init_mc_axi_if(void);
#endif
static int memc_get_speed(uint32_t memcfg);
static int memc_get_size(uint32_t memcfg);
static int memc_is_high_temp(uint32_t memcfg);
static void memc_print_cfg(int size, int clock);
static mcbindex* memc_get_mcb(uint32_t memcfg);

/* string tables */
char* str_speed[8] = { 
                         "800 CL6",
                         "1066 CL7",
                         "1066 CL8",
                         "1333 CL9",
                         "1333 CL10",
                         "1600 CL10",
                         "1600 CL11",
                         "Custom Speed",
                     };

char* str_ssc[4] = {
                         "None",   /* BP_DDR_SSC_CONFIG_NONE */
                         "%1",     /* BP_DDR_SSC_CONFIG_1 */
                         "%0.5",   /* BP_DDR_SSC_CONFIG_2 */
                         "Custom", /* BP_DDR_SSC_CONFIG_CUSTOM */
                     };

/********************** per chip memory size to col/row/ba address bits mapping table **************************/
uint32_t addr_map_tbl[3][2][4] = {   /*  size_Mbits, row_bits, col_bits, ba_bits */
                                    {
                                        {   1024,       14,       10,       3      }, // 8bit, 16 Meg x 8 x 8 banks, 1Gb  
                                        {   1024,       13,       10,       3      }  // 16bit, 8 Meg x 16 x 8 banks, 1Gb  
                                    },
                                    {
                                        {   2048,       15,       10,       3      }, // 8bit, 32 Meg x 8 x 8 banks, 2Gb
                                        {   2048,       14,       10,       3      }  // 16bit, 16 Meg x 16 x 8 banks, 2Gb  
				    },
                                    {
				        {   4096,       16,       10,       3      }, // 8bit, 64 Meg x 8 x 8 banks, 4Gb
                                        {   4096,       15,       10,       3      }  // 16bit, 32 Meg x 16 x 8 banks, 4Gb
				    }
                              }; 

/********************** memc col/row/ba addr register data table ****************************************/
uint32_t memc_addr_tbl[2][3][16] =  {
                                        {
                                    // Index:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
					    { 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 }, // Row
                                            {  0,  0,  0,  4,  5,  9, 10, 11, 12, 13,  0,  0,  0,  0,  0 }, // Column
                                            {  6,  7,  8 }                                                  // Bank
                                        },
                                        {
                                   //  Index:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
					     { 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 }, // Row
                                             {  0,  0,  0,  4,  5,  6,  7,  8,  9, 10,  0,  0,  0,  0,  0 }, // Column
                                             { 11, 12, 13 }                                                  // Bank
                                        }
                                    };

/********************** DDR timing parameter table  ***********************************************************************************************/
uint32_t ddr_timing_tbl[7][16] = {    /* tRCD, tCL,  tWR,  tCwl, tRP,  tRRD, tRCr, tFAW, tRFC, tW2R, tR2W, tR2R, tAL, tRTP, tW2W, tWTR */
                                      {   7,    6,    6,    5,    6,    4,    21,   20,   44,   2,    4,    0,    0,    4,    0,    4   }, // DDR400 speed grade sg25 CL6
                                      {   8,    7,    8,    6,    8,    4,    26,   22,   59,   2,    3,    0,    0,    4,    0,    4   }, // DDR533 speed grade sg187E CL7
                                      {   8,    8,    8,    6,    8,    6,    28,   27,   59,   2,    3,    0,    0,    4,    0,    4   }, // DDR533 speed grade sg187 CL8
                                      {   9,    9,    10,   7,    9,    6,    34,   30,   74,   2,    2,    0,    0,    5,    0,    5   }, // DDR667 speed grade sg15E CL9  -- to be verfied with Herman
                                      {   9,    9,    10,   7,    9,    6,    34,   30,   74,   2,    2,    0,    0,    5,    0,    5   }, // DDR667 speed grade sg15E CL10  -- to be verfied with Herman
                                      {   10,   10,   12,   8,    10,   6,    38,   32,   88,   2,    3,    0,    0,    6,    0,    6   }, // DDR800 speed grade sg125E CL10 -- to be verfied with Herman
                                      {   11,   11,   12,   8,    11,   6,    39,   32,   88,   2,    4,    0,    0,    6,    0,    6   }  // DDR800 speed grade sg125 CL11

                                 };

/* per chip tRFC = tRFC_ns/mclk_period_ns. 
   tRFC_ns: from DDR datasheet, depending on the memory size. 
   mclk_period: MEMC clock. 2.5ns for 400, 1.87ns for 533, 1.48ns for 677 1.25ns for 800 
*/
uint32_t tRFC_tbl[3][7] = {
                              /* 400-CL6, 533-CL7, 533-CL8, 677-CL9, 677-CL10, 800-CL10, 800-CL11 */  
                              {  44,      59,      59,      74,      74,       88,       88 },     //1Gb
                              {  64,      86,      86,      107,     107,      128,      128 },    //2Gb
                              {  104,     140,     140,     175,     175,      208,      208 },    //4Gb
                          };

memc_bus_cfg            mc_cfg;
memc_dram_profile       mc_dram_profile;

static void memc_delay(unsigned int ns)
{
    while(ns--);
}

static void mc_cfg_init(void)
{
    int i;

    print_log("mc_cfg_init(): Initialize the default values on mc_cfg\n");

    mc_cfg.arb_burst_mode = 0;
    mc_cfg.arb_rr_mode = 0;
    for (i=0; i < MC_ARB_SP_Q_NUM; i++) {
      mc_cfg.arb_sp_sel[i] = 0;
      mc_cfg.arb_sp_pri[i] = i;
    }
    mc_cfg.arb_sp_sel[MC_ARB_SP_Q_NUM-1] = 1; // RR winner is always the last SP Q
    for (i=0; i < MC_ARB_RR_Q_NUM; i++) 
        mc_cfg.arb_rr_qqw[i] = 1;
    for (i=0; i < MC_UB_SRC_ID_NUM; i++)
        mc_cfg.ub0_srcid_q[i] = 0;
    
    mc_cfg.ub0_oob_err_mask   = 0;
    mc_cfg.ub0_ib_err_mask    = 0;
    mc_cfg.ub0_wburst_mode    = 0;
    mc_cfg.ub0_wrep_mode      = 0;
    //mc_cfg.ub0_repreq_mode    = 0;
    mc_cfg.ub0_fifo_mode      = 0;
    mc_cfg.ub1_oob_err_mask   = 0;
    mc_cfg.ub1_ib_err_mask    = 0;
    mc_cfg.ub1_wburst_mode    = 0;
    mc_cfg.ub1_wrep_mode      = 0;
    //mc_cfg.ub1_repreq_mode    = 0;
    mc_cfg.ub1_fifo_mode      = 0;
    mc_cfg.axirif_fifo_mode   = 0;
    mc_cfg.axiwif_wburst_mode = 0;
    mc_cfg.axiwif_wrep_mode   = 0;
    mc_cfg.axiwif_fifo_mode   = 0;

    mc_cfg.gcfg_max_age       = 0x2;
    mc_cfg.large_page         = 0x0;

#if 0
    if (parse_sim_opts("-d MC_AXIR_STRICT_FIFO_MODE")) 
        mc_cfg.axirif_fifo_mode   |= 0x2;
    if (parse_sim_opts("-d MC_AXIR_TRANS_INTER_MODE")) 
        mc_cfg.axirif_fifo_mode   |= 0x1;
    if (parse_sim_opts("-d MC_AXIW_STRICT_FIFO_MODE")) 
        mc_cfg.axiwif_fifo_mode   = 0x1;
    if (parse_sim_opts("-d MC_AXIW_CONSERV_REP_MODE")) 
        mc_cfg.axiwif_wrep_mode   = 0x1;
    if (parse_sim_opts("-d MC_AXIW_BURST_MODE")) 
        mc_cfg.axiwif_wburst_mode = 0x1;
    if (parse_sim_opts("-d MC_UB0_CONSERV_REP_MODE")) 
        mc_cfg.ub0_wrep_mode      = 0x1;
    if (parse_sim_opts("-d MC_UB0_OOB_ERR_MASK")) 
        mc_cfg.ub0_oob_err_mask   = 0x1;
    if (parse_sim_opts("-d MC_UB0_IB_ERR_MASK")) 
        mc_cfg.ub0_ib_err_mask    = 0x1;
#endif 

    return;
}

static void init_memc_dram_profile(int size, int clock) 
{
    int index; 
    print_log("init_memc_dram_profile(): Initializing MEMC DRAM profile\n");
  
    // Setup profile values
    /* caller pass size index from the largest size to smallest */
    index = size;
    mc_dram_profile.size_Mbits = addr_map_tbl[index][is_16bit][0];
    mc_dram_profile.row_bits   = addr_map_tbl[index][is_16bit][1];
    mc_dram_profile.col_bits   = addr_map_tbl[index][is_16bit][2];
    mc_dram_profile.ba_bits    = addr_map_tbl[index][is_16bit][3];

    //[DH] For now, only supporting 16-bit DRAM width with 16 byte access.
    mc_dram_profile.width_bits   = 16;
    mc_dram_profile.access_bytes = 16;
  
    // Setup timing values
    index = clock;
    mc_dram_profile.dram_clk = 400;
    if( clock == DDR_CLK_400MHZ_CL6 )
       mc_dram_profile.dram_clk = 400;
    else if( clock == DDR_CLK_533MHZ_CL7 || clock == DDR_CLK_533MHZ_CL8 )
       mc_dram_profile.dram_clk = 533;
    else if( clock == DDR_CLK_667MHZ_CL9 || clock == DDR_CLK_667MHZ_CL10)
       mc_dram_profile.dram_clk = 667;
    else if( clock == DDR_CLK_800MHZ_CL10 || clock == DDR_CLK_800MHZ_CL11 )
       mc_dram_profile.dram_clk = 800;

    mc_dram_profile.ref_clk = 50000000;
    mc_dram_profile.timing_cfg.tRCD =  ddr_timing_tbl[index][ 0];
    mc_dram_profile.timing_cfg.tCL  =  ddr_timing_tbl[index][ 1];
    mc_dram_profile.timing_cfg.tWR  =  ddr_timing_tbl[index][ 2];
    mc_dram_profile.timing_cfg.tCwl =  ddr_timing_tbl[index][ 3];
    mc_dram_profile.timing_cfg.tRP  =  ddr_timing_tbl[index][ 4];
    mc_dram_profile.timing_cfg.tRRD =  ddr_timing_tbl[index][ 5];

    mc_dram_profile.timing_cfg.tRCr =  ddr_timing_tbl[index][ 6];
    mc_dram_profile.timing_cfg.tFAW =  ddr_timing_tbl[index][ 7];
    mc_dram_profile.timing_cfg.tRFC =  tRFC_tbl[size][index];

    mc_dram_profile.timing_cfg.tW2R =  ddr_timing_tbl[index][ 9];
    mc_dram_profile.timing_cfg.tR2W =  ddr_timing_tbl[index][10];
    mc_dram_profile.timing_cfg.tR2R =  ddr_timing_tbl[index][11];

    mc_dram_profile.timing_cfg.tAL  =  ddr_timing_tbl[index][12];
    mc_dram_profile.timing_cfg.tRTP =  ddr_timing_tbl[index][13];
    mc_dram_profile.timing_cfg.tW2W =  ddr_timing_tbl[index][14];
    mc_dram_profile.timing_cfg.tWTR =  ddr_timing_tbl[index][15];
   
    //For PHY init
    mc_dram_profile.timing_cfg.tRAS = 28;
    mc_dram_profile.timing_cfg.tCCD = 4;

    return;
}

// This function is used to program the MEMC timing registers. 
// It's almost fully based on the prog_timing_cntrl legacy function.
static void cfg_memc_timing_ctrl(void) 
{
    uint32_t tim1_0 = 0;
    uint32_t tim1_1 = 0;
    uint32_t tim2   = 0;
    uint32_t mcregval;

    mcregval = mc_dram_profile.timing_cfg.tAL;
   
    /* adjust for DDR3, assume DDR3 for now */
    mcregval = 0;
    if (mc_dram_profile.timing_cfg.tAL > 0) {
        mcregval = mc_dram_profile.timing_cfg.tCL - mc_dram_profile.timing_cfg.tAL;
        print_log("cfg_memc_timing_ctrl() : Adjusted tAL cfg = %d (original: tAL=%d; tCL=%d)\n", 
            mcregval, mc_dram_profile.timing_cfg.tAL, mc_dram_profile.timing_cfg.tCL);
    }
  

    tim1_0 = ( (mc_dram_profile.timing_cfg.tRCD & 0xf)        | 
               (mc_dram_profile.timing_cfg.tCL  & 0xf)  << 4  | 
               ((mc_dram_profile.timing_cfg.tCL >> 4) & 0x7) << 24  | 
               (mc_dram_profile.timing_cfg.tWR  & 0xf) << 8   |
               (mc_dram_profile.timing_cfg.tCwl & 0xf) << 12  |
               (mc_dram_profile.timing_cfg.tRP  & 0xf) << 16  |
               (mc_dram_profile.timing_cfg.tRRD & 0xf) << 20  );
    
    tim1_1 = ( (mc_dram_profile.timing_cfg.tRCr & 0x3f)       | 
               (mc_dram_profile.timing_cfg.tFAW & 0x3f) <<  8 | 
               (mc_dram_profile.timing_cfg.tRFC & 0xff) << 16 |
               (mc_dram_profile.timing_cfg.tW2R & 0x7)  << 24 |
               (mc_dram_profile.timing_cfg.tR2W & 0x7)  << 27 |
               (mc_dram_profile.timing_cfg.tR2R & 0x3)  << 30 );
  
    tim2   = ( (mcregval & 0xf)        | 
               (mc_dram_profile.timing_cfg.tRTP & 0x7) <<  4 | 
               (mc_dram_profile.timing_cfg.tW2W & 0x7) <<  8 |
               (mc_dram_profile.timing_cfg.tWTR & 0xf) << 12 );

    MEMC->CHN_TIM_TIM1_0 = tim1_0;
    MEMC->CHN_TIM_TIM1_1 = tim1_1;
    MEMC->CHN_TIM_TIM2 = tim2;

    print_log("cfg_memc_timing_ctrl tim1_0 0x%08x tim1_1 0x%08x tim2 0x%08x\n", tim1_0, tim1_1, tim2); 
    return;
}

// This function is used to configure physical address to column, row, bank mapping.
static void cfg_map_addr_bits_to_row_col_ba(void) 
{
    int i, j, mi, mii, miii; // m_val indices: - mi is for choice of map - mii is for choice of row (0), column (1), bank (2) -miii is for values in row, column, bank
    uint32_t reg_val, max_bits;
    int   reg_update_flag;
    volatile uint32_t* cfg_regs[9] = { &MEMC->CHN_CFG_ROW00_0 ,
                             &MEMC->CHN_CFG_ROW00_1 ,
                             &MEMC->CHN_CFG_ROW01_0 ,
                             &MEMC->CHN_CFG_ROW01_1 ,
                             &MEMC->CHN_CFG_COL00_0 , 
                             &MEMC->CHN_CFG_COL00_1 , 
                             &MEMC->CHN_CFG_COL01_0 , 
                             &MEMC->CHN_CFG_COL01_1 , 
                             &MEMC->CHN_CFG_BNK10     };

    if ((mc_dram_profile.width_bits == 16) && (mc_dram_profile.access_bytes == 16)) 
         mi = (mc_cfg.large_page==0) ? 0 : 1;
    else {
         print_log("cfg_map_addr_bits_to_row_col_ba() : The following profile is not supported: width_bits=%d ; access_bytes=%d\n",
              mc_dram_profile.width_bits, mc_dram_profile.access_bytes);
         return;
     }

    for ( i=0; i<9; i++) {    // i is to iterate over registers
        reg_val = 0;
        reg_update_flag = 0;

        if      (i == 0) { mii = 0; miii = 0; max_bits = mc_dram_profile.row_bits; } // Row 
        else if (i == 4) { mii = 1; miii = 0; max_bits = mc_dram_profile.col_bits; } // Column
        else if (i == 8) { mii = 2; miii = 0; max_bits = mc_dram_profile.ba_bits;  } // Bank

        for (j=0; j<4; j++) {  // j is to iterate over fields
            if (miii < max_bits) {
                reg_update_flag = 1;
                reg_val += memc_addr_tbl[mi][mii][miii] << (8 * j);
                miii++;
            }
        }
        if (reg_update_flag) {
            print_log("cfg_map_addr_bits_to_row_col_ba writing to address=0x%x ; data=0x%x\n", cfg_regs[i], reg_val);
            *(cfg_regs[i]) = reg_val;
        }
    }
  
    return;
}

// This function is used to configure the refresh control register.
static void refresh_ctrl_cfg(uint32_t dram_clk_freq, int ref_disable) 
{
    uint32_t freq = dram_clk_freq;
    uint32_t ref_rate = 0x2C;
    int rate_adj = is_ht ? 1 : 0;

    print_log("refresh_ctrl_cfg called (dram_clk_freq=%d ; ref_disable=%d)\n", dram_clk_freq, ref_disable);


    if (freq < 800+10 && freq > 800-10) freq = 800;
    if (freq < 666+10 && freq > 666-10) freq = 666;
    if (freq < 533+10 && freq > 533-10) freq = 533;
    if (freq < 400+10 && freq > 400-10) freq = 400;
    if (freq < 333+10 && freq > 333-10) freq = 333;
    if (freq < 266+10 && freq > 266-10) freq = 266;
    if (freq < 200+10 && freq > 200-10) freq = 200;
    if (freq < 133+10 && freq > 133-10) freq = 133;
    if (freq <  66+10 && freq >  66-10) freq =  66;

    // These values are taken from 6318 MEMC reg doc
    // 2x mode:
    if (freq == 200) ref_rate = (0x2C>>rate_adj); 
    if (freq == 333) ref_rate = (0x4D>>rate_adj); 
    if (freq == 400) ref_rate = (0x5D>>rate_adj); 
    if (freq == 533) ref_rate = (0x7B>>rate_adj); 
    // 4x mode:
    if (freq == 667) ref_rate = (0x9A>>rate_adj);
    if (freq == 800) ref_rate = (0xBA>>rate_adj); 
 
    print_log("refresh_ctrl_cfg writing to refresh control register (freq=%d ; ref_rate=0x%x ; ref_disable=%d)\n", freq, ref_rate, ref_disable);
    MEMC->CHN_TIM_CLKS =  (ref_rate << 8) | (ref_disable << 16);
} 

#if 0
static void init_mc_arb(void)
{
    int i, j;
    uint32_t reg_wr;
    volatile uint32_t* reg;

    print_log("init_mc_arb(): Initialize the MC_ARB registers\n");
    MEMC->GLB_CFG = ((mc_cfg.arb_burst_mode << MEMC_GLB_CFG_BURST_MODE_SHIFT) & MEMC_GLB_CFG_BURST_MODE_MASK) | 
                                      ((mc_cfg.arb_rr_mode << MEMC_GLB_CFG_RR_MODE_SHIFT) & MEMC_GLB_CFG_RR_MODE_MASK);

    print_log("init_mc_arb write MEMC->GLB_CFG 0x%08x\n", MEMC->GLB_CFG);

#if 0  /* do we need this? */
  // hermanl for backwards compatibility before BCM63138A0 tapesout, should remove checck after that
  if(mc_cfg.gcfg_max_age!=2) {
    print_log("Setting the GCFG_MaxAge to 0x%x\n",mc_cfg.gcfg_max_age);
    reg_wr = m_regs->read32(uint32(MC_GLB_GCFG));
    reg_wr |= ((mc_cfg.gcfg_max_age << MC_GLB_GCFG_GCFG_MaxAge_SHIFT) & MC_GLB_GCFG_GCFG_MaxAge_MASK);         
    m_regs->write32(uint32(MC_GLB_GCFG), reg_wr);
  }
#endif

    reg_wr = 0;
    for (i=0; i < MC_ARB_SP_Q_NUM; i++) 
        reg_wr |= (mc_cfg.arb_sp_sel[i] & 0x1) << i;
    reg_wr &= MEMC_GLB_SP_SEL_SELECT_MASK;
    MEMC->GLB_SP_SEL = reg_wr;
    print_log("init_mc_arb write MEMC->GLB_SP_SEL 0x08%x\n", reg_wr);

    reg = &MEMC->GLB_SP_PRI_0;
    for ( i=0; i < 6; i++) {
        reg_wr = 0;
        for (j=0; j < 4; j++) {
          if ( !((i == 5) && (j > 1)) ) 
              reg_wr |= (mc_cfg.arb_sp_pri[j+(i*4)] & 0x1f) << (j*8);
        }
        *(reg+i) = reg_wr;
	print_log("init_mc_arb write sp pri reg 0x%08x with 0x%08x\n", (unsigned int)(reg+i), reg_wr);
    }

    reg = &MEMC->GLB_RR_QUANTUM[0];
    for (i=0; i < 11; i++) {
        reg_wr = 0;
        for (j=0; j < 2; j++) {
            if ( !((i == 10) && (j == 1)) ) 
                reg_wr |= (mc_cfg.arb_rr_qqw[j+(i*2)] & 0x3ff) << (j*16);
        }
        *(reg+i) = reg_wr;
	print_log("init_mc_arb write rr quantum reg 0x%08x with 0x%08x\n", (unsigned int)(reg+i), reg_wr);
    }
}

static void init_mc_ubus_if(void)
{
    uint32_t reg_wr;
    int i, j;
    volatile uint32_t* reg;

    print_log("init_mc_ubus_if(): Initialize the MC_UBUSIF_0 and MC_UBUSIF_1 registers\n");
    MEMC->UBUSIF0.CFG = ((mc_cfg.ub0_wburst_mode << UBUSIF_CFG_WRITE_BURST_MODE_SHIFT) & UBUSIF_CFG_WRITE_BURST_MODE_MASK) |
                                           ((mc_cfg.ub0_oob_err_mask << UBUSIF_CFG_OOB_ERR_MASK_SHIFT) & UBUSIF_CFG_OOB_ERR_MASK_MASK) |
                                           ((mc_cfg.ub0_ib_err_mask << UBUSIF_CFG_INBAND_ERR_MASK_SHIFT) & UBUSIF_CFG_INBAND_ERR_MASK_MASK) |
                                           (mc_cfg.ub0_wrep_mode & UBUSIF_CFG_WRITE_REPLY_MODE_MASK);
    //m_regs->write32(uint32(MC_UBUSIF_0_REP_ARB_MODE), ((mc_cfg.ub0_repreq_mode << MC_UBUSIF_0_REP_ARB_MODE_UBUS_REP_REQ_MODE_SHIFT) & MC_UBUSIF_0_REP_ARB_MODE_UBUS_REP_REQ_MODE_MASK) |
    MEMC->UBUSIF0.REP_ARB_MODE = (mc_cfg.ub0_fifo_mode & UBUSIF_REP_ARB_MODE_FIFO_MODE_MASK);
    reg = &MEMC->UBUSIF0.SRC_QUEUE_CTRL_0;
    for (i=0; i < 4; i++) {
        reg_wr = 0;
        for (j=0; j < 8; j++)
            reg_wr |= (mc_cfg.ub0_srcid_q[j+(i*8)] & 0xf) << (j*4);
        *(reg+i) = reg_wr;
	print_log("init_mc_ubus_if write ubusif0 reg 0x%08x with 0x%08x\n", (unsigned int)(reg+i), reg_wr);
    }

    MEMC->UBUSIF1.CFG = ((mc_cfg.ub1_wburst_mode << UBUSIF_CFG_WRITE_BURST_MODE_SHIFT) & UBUSIF_CFG_WRITE_BURST_MODE_MASK) |
                                           ((mc_cfg.ub1_oob_err_mask << UBUSIF_CFG_OOB_ERR_MASK_SHIFT) & UBUSIF_CFG_OOB_ERR_MASK_MASK) |
                                           ((mc_cfg.ub1_ib_err_mask << UBUSIF_CFG_INBAND_ERR_MASK_SHIFT) & UBUSIF_CFG_INBAND_ERR_MASK_MASK) |
                                           (mc_cfg.ub1_wrep_mode & UBUSIF_CFG_WRITE_REPLY_MODE_MASK);
    //m_regs->write32(uint32(MC_UBUSIF_1_REP_ARB_MODE), ((mc_cfg.ub1_repreq_mode << MC_UBUSIF_1_REP_ARB_MODE_UBUS_REP_REQ_MODE_SHIFT) & MC_UBUSIF_1_REP_ARB_MODE_UBUS_REP_REQ_MODE_MASK) |
   MEMC->UBUSIF1.REP_ARB_MODE = (mc_cfg.ub1_fifo_mode & UBUSIF_REP_ARB_MODE_FIFO_MODE_MASK);
}

static void init_mc_axi_if(void)
{
    print_log("init_mc_axi_if(): Initialize the MC_AXIRIF and MC_AXIWIF registers\n");
    MEMC->AXIRIF_0_REP_ARB_MODE =  mc_cfg.axirif_fifo_mode & MEMC_AXIRIF_0_REP_ARB_MODE_FIFO_MODE_MASK;
    MEMC->AXIWIF_0_CFG = ((mc_cfg.axiwif_wburst_mode << MEMC_AXIWIF_0_CFG_WRITE_BURST_MODE_SHIFT) & MEMC_AXIWIF_0_CFG_WRITE_BURST_MODE_MASK) |
                                           ((mc_cfg.axiwif_wrep_mode << MEMC_AXIWIF_0_CFG_WRITE_REPLY_MODE_SHIFT) & MEMC_AXIWIF_0_CFG_WRITE_REPLY_MODE_MASK);
   MEMC->AXIWIF_0_REP_ARB_MODE = mc_cfg.axiwif_fifo_mode & MEMC_AXIWIF_0_REP_ARB_MODE_FIFO_MODE_MASK;
}
#endif

/* return 1 if there is alias, 0 no alias.  memsize in Mbits */
int memc_alias_test(uint32_t memsize)
{
    volatile uint32 *base_addr;
    volatile uint32 *test_addr;
    uint32 test_size;
    int ret = 0, i = 8;

    memsize = memsize<<17; 
    base_addr = (uint32*)DRAM_BASE_NOCACHE;

    for (test_size = 256; test_size < memsize ; test_size = test_size << 1) {
        test_addr = (volatile uint32*)((uint32_t)base_addr + test_size);
        *base_addr = 0;
        *test_addr = 0xaa55beef;
        if (*base_addr == *test_addr) {
            ret = 1;
            break;
        }
        i++;
    }

    return ret;
}

static int init_memc(int size, int clock)
{
    uint32_t data = 0;

    /*printf("MEMC init size %d clock %d\n", size, clock);*/

    data = MEMC->GLB_GCFG;
    data &= ~MEMC_GLB_GCFG_MEMINITDONE_MASK;
    data &= ~MEMC_GLB_GCFG_DRAM_EN_MASK;
    MEMC->GLB_GCFG = data;

    init_memc_dram_profile(size, clock);
    print_log("Disable Auto-Refresh\n");
    refresh_ctrl_cfg(mc_dram_profile.dram_clk, 1);

    cfg_map_addr_bits_to_row_col_ba();
    cfg_memc_timing_ctrl();

    print_log("Setting the DDR3 mode\n");
    data = MEMC->CHN_TIM_DRAM_CFG;
    data |= (data & ~MEMC_CHN_TIM_DRAM_CFG_DRAM_CFG_DRAMTYPE_MASK) | MC_DRAM_CFG_DDR3;
    print_log("Write %x to MC_CHN_TIM_DRAM_CFG Register\n", data);
    MEMC->CHN_TIM_DRAM_CFG = data;

    print_log("Enable Auto-Refresh\n");
    refresh_ctrl_cfg(mc_dram_profile.dram_clk , 0);
        
    print_log("Set the flag indicating that memory initialization is done\n");
    data = MEMC->GLB_GCFG;
    data |= MEMC_GLB_GCFG_MEMINITDONE_MASK;
    data |= MEMC_GLB_GCFG_DRAM_EN_MASK;
    MEMC->GLB_GCFG = data;

    return 0; 
}

static mcbindex* memc_get_mcb(uint32_t memcfg)
{
    mcbindex* pMCB = &MCB[0], *pRet = NULL;

    if((memcfg&BP_DDR_SPEED_MASK) == BP_DDR_SPEED_SAFE)
    {
         printf("board configured to use safe mode DDR setting\n");
         return NULL;
    }

    while( pMCB->pMcbData )
    {
        if( (pMCB->config&pMCB->mask) == (memcfg&pMCB->mask) )
	{
 	    /* only take the first one that match unless there is one exact match */
	    if( pRet == NULL) 
	        pRet = pMCB;

            if( (pMCB->config&BP_DDR_CONFIG_MASK) == (memcfg&BP_DDR_CONFIG_MASK) )
	    {
 	        pRet = pMCB;
                break;
	    }
	}
        pMCB++;
    }

    return pRet;
}

static int memc_get_size(uint32_t memcfg)
{
    uint32_t bp_size;
    int size;

    bp_size = (memcfg&BP_DDR_TOTAL_SIZE_MASK)>>BP_DDR_TOTAL_SIZE_SHIFT;
    is_16bit = ((memcfg&BP_DDR_DEVICE_WIDTH_MASK) == BP_DDR_DEVICE_WIDTH_16) ? 1:0;

    /* find out the per chip memory size */
    if( bp_size >= (BP_DDR_TOTAL_SIZE_128MB>>BP_DDR_TOTAL_SIZE_SHIFT) && 
        bp_size <= (BP_DDR_TOTAL_SIZE_1024MB>>BP_DDR_TOTAL_SIZE_SHIFT) )
    {
        size = (bp_size-(BP_DDR_TOTAL_SIZE_128MB>>BP_DDR_TOTAL_SIZE_SHIFT))+MEM_SIZE_1024Mb;     
        if( !is_16bit )
    	    size = size -1;   
        total_size_mbit = addr_map_tbl[size][is_16bit][0];
        if( !is_16bit )
	    total_size_mbit = total_size_mbit<<1;
    }
    else
        size = -1;

    return size;
}

static int memc_get_speed(uint32_t memcfg)
{
    uint32_t bp_speed;
    int speed;

    bp_speed = (memcfg&BP_DDR_SPEED_MASK)>>BP_DDR_SPEED_SHIFT;
    if(bp_speed == BP_DDR_SPEED_SAFE )
        speed = DDR_CLK_533MHZ_CL8;
    else if( bp_speed >= BP_DDR_SPEED_400_6_6_6 && bp_speed <= BP_DDR_SPEED_800_11_11_11 )
        speed = (bp_speed - BP_DDR_SPEED_400_6_6_6) + DDR_CLK_400MHZ_CL6;
    /* add custom speed handling here */
    else 
        speed = -1; 
   
    return speed;

}

static int memc_is_high_temp(uint32_t memcfg)
{
    return( (memcfg&BP_DDR_TEMP_MASK) >> BP_DDR_TEMP_SHIFT );
}

static int memc_is_ssc(uint32_t memcfg)
{
    return( (memcfg&BP_DDR_SSC_CONFIG_MASK) >> BP_DDR_SSC_CONFIG_SHIFT );
}

static void memc_print_cfg(int size, int clock)
{
   if( clock >= sizeof(str_speed) )
        clock = sizeof(str_speed) - 1;

    if( is_ssc >= sizeof(str_ssc) )
        is_ssc = sizeof(str_ssc) - 1;

    printf("DDR3-%s ", str_speed[clock]);
    printf("%dMB", 128<<size);
    if( !is_16bit )
        printf("x2");
    if( is_ssc )
        printf(" %s SSC", str_ssc[is_ssc]);
    if( is_ht )
      printf(" High Temperature %s", is_ht == (BP_DDR_TEMP_EXTENDED_SRT>>BP_DDR_TEMP_SHIFT) ? "SRT" : "ASR" );

    printf("\r\n");
    
    return;
}

//*****************************************
// This is the main initialization function
//*****************************************
int ddr_init(uint32_t mcb_selector, uint32_t memcfg_from_flash)
{
    uint32_t data = 0, memcfg = 0;
    uint32_t memsys_init_opt[8];
    uint32_t Shmoo_Test_0_Base, Shmoo_Test_Size,EDIS_Info;
    uint32_t option_phy_init_only, option_shmoo_only, option_dbgmsg_disable = 0;
    mcbindex* pMCB = NULL;
    int ddr_clk, mem_size;
    int rc = 0, i, size = 0, size_detected = 0;
#ifdef AUTO_SIZE  
    int  size_index;
#endif
#if (INC_NAND_FLASH_DRIVER==1) && (defined(_BCM963138_)||defined(_BCM963148_))
     /* check if it is SPI to NAND boot, we don't need to run ddr init again if so */
    if( (MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SPI_NOR) == MISC_STRAP_BUS_BOOT_SPI_NOR && 
	(MEMC->GLB_GCFG&MEMC_GLB_GCFG_MEMINITDONE_MASK) == MEMC_GLB_GCFG_MEMINITDONE_MASK )
         return 0;
#endif

    /* set the internal voltage regulator gain to 8. This reduces the response time and keeps
       voltage supply to ddr stable.*/
    PROCMON->SSBMaster.wr_data = 0x800; 
    PROCMON->SSBMaster.control = 0x3440;
    PROCMON->SSBMaster.control = 0xb440;
    while(PROCMON->SSBMaster.control&0x8000);

    PROCMON->SSBMaster.wr_data = 0x802;
    PROCMON->SSBMaster.control = 0x3440;
    PROCMON->SSBMaster.control = 0xb440;
    while(PROCMON->SSBMaster.control&0x8000);

    PROCMON->SSBMaster.wr_data = 0x800;
    PROCMON->SSBMaster.control = 0x3440;
    PROCMON->SSBMaster.control = 0xb440;
    while(PROCMON->SSBMaster.control&0x8000);

    board_setledsC('D','R','A','M'); // DRAM

    if( mcb_selector == MCB_SEL_SAFEMODE ) {
        is_safemode = 1;
     } else {
        /* read memory config from nvram */
                if ((mcb_selector&MCB_SEL_OVERRIDE) == MCB_SEL_OVERRIDE) {
                     memcfg = mcb_selector&MCB_SEL_MASK;
                } else {
                 memcfg = memcfg_from_flash;
                }
                printf("NVRAM memcfg 0x%x\n", memcfg);
                pMCB = memc_get_mcb(memcfg);
                if( pMCB == NULL ) {
                    printf("No valid MCB found for memcfg 0x%x, use safe mode setting\n", memcfg);
                    is_safemode = 1;
                }
    }

    if (is_safemode == 1) {
        pMCB = &MCB[0];
        printf("DDR init safe mode\n");
    }

    printf("MCB chksum 0x%x\n", pMCB->pMcbData[3]);    

    /* validate config */
    mem_size = memc_get_size(pMCB->config);
    ddr_clk = memc_get_speed(pMCB->config);
    is_ht = memc_is_high_temp(pMCB->config);
    is_ssc = memc_is_ssc(pMCB->config);
    if( !is_safemode )
        option_dbgmsg_disable = (memcfg&BP_DDR_CONFIG_DEBUG) ? 0:1;

    if( ddr_clk < 0 || mem_size < 0 )
    {
        printf("invalid MCB config 0x%x, clk %d, size %d\n", pMCB->config, ddr_clk, mem_size);
        return -1;
    }

    if( is_safemode == 0 ) 
        memc_print_cfg(mem_size, ddr_clk);

    /* the pranch predict may prefetch code/data from ddr adddress. this cause B15 hang during shmoo. 
       disable that until ddr init is done */
    armv7_branch_predict_disable();

    mc_cfg_init();

    // make sure we always use the always on rbus clock before phy pll is locked. 
    data = MEMC->GLB_GCFG;
    data &= ~MEMC_GLB_GCFG_MCLKSRC_MASK;
    MEMC->GLB_GCFG = data;

    print_log("Setting the GCFG_4X_MODE bit for phy_4x_mode configuration\n");
    data = MEMC->GLB_GCFG;
    data |= MEMC_GLB_GCFG_PHY_4X_MASK;
    MEMC->GLB_GCFG = data;
  

    // Poll the PHY Status Register
    print_log("Poll PHY Status register\n");
    data = MEMC->CHN_TIM_PHY_ST;
    print_log("PHY Status= %x\n", data);
    // - check Power_Up to see if PHY is ready to receive register access
    // - check SW_Reset
    // - check HW_Reset
    while (!((data & MEMC_CHN_TIM_PHY_ST_PHY_ST_POWER_UP) ||
          (data & MEMC_CHN_TIM_PHY_ST_PHY_ST_SW_RESET) ||
          (data & MEMC_CHN_TIM_PHY_ST_PHY_ST_HW_RESET))) {
        memc_delay(200);
        data = MEMC->CHN_TIM_PHY_ST;
        print_log("PHY Status= %x\n", data);
    }

    // Setup EDIS information: only use 1 EDIS block
    EDIS_Info = 1;

    // Setup option, common for all MEMSYS
    // Call PHY init only, but do not run Shmoo
    option_phy_init_only = 
        option_dbgmsg_disable |
        (1 << 6) | // reserved, set to 1
        (1 << 5) | // reserved, set to 1
        (1 << 4);  // skip shmoo

    // Run Shmoo only, do not run PHY init
    option_shmoo_only = 
        option_dbgmsg_disable |
        (1 << 9) | // skip phy pll init
        (1 << 8) | // skip phy dram init
        (1 << 7) | // skip phy init
        (1 << 6) | // reserved, set to 1
        (1 << 5);  // reserved, set to 1

    // Setup Shmoo test size, common for all MEMSYS
    Shmoo_Test_Size = 128 * 1024;
    Shmoo_Test_0_Base = 0;

    /* init PHY */
    memsys_init_opt[0] = 0;             // 0 = not using Andover MEMC
    memsys_init_opt[1] = DDRPHY_BASE;   // PHY register base
    memsys_init_opt[2] = 0;             // 0 = not using Andover MEMC
    memsys_init_opt[3] = MEMC_BASE + 0x500; // EDIS register base 
    memsys_init_opt[4] = EDIS_Info;     // right now only use 1 EDIS
    memsys_init_opt[5] = option_phy_init_only;  // do not run shmoo at this time
    memsys_init_opt[6] = Shmoo_Test_0_Base;
    memsys_init_opt[7] = Shmoo_Test_Size;


    rc = memsys_init((uint32_t*)pMCB->pMcbData, memsys_init_opt);
    if( rc )
        printf("memsys_init call phy init returns 0x%08x\r\n", rc);
    
    // [RW] switching the mclksrc muxsel to use PLL clk instead of RCLK
    print_log("memc_dram_init(): memsys_init() completed, Set MCLKSRC_SEL bit in memc to use PLL clk\n");
    data = MEMC->GLB_GCFG;
    data |= MEMC_GLB_GCFG_MCLKSRC_MASK;
    MEMC->GLB_GCFG = data;
#if 0
    init_mc_arb();
    init_mc_ubus_if();
    init_mc_axi_if();
#endif
    // This was added because after the DDR PHY init sequence (i.e. ddr34_phy_init()) finishes, CKE is disabled.
    MEMC->CHN_TIM_DCMD = 0x00000305; // Enable CKE

#ifdef AUTO_SIZE
    for( size_index = MEM_SIZE_2048Mb; size_index >= MEM_SIZE_1024Mb; size_index-- )
    {  
        board_setledsC('S','I','Z','0'+ size_index); // SIZ0

        init_memc(size_index, DDR_CLK_533MHZ);

        /* make sure the memc register write is completed before we access DDR */
        data = MEMC->GLB_GCFG;
	__asm__ __volatile__ ("dsb");

        if(memc_alias_test(mc_dram_profile.size_Mbits) == 0)
	{
	    size_detected = 1;
	    break;
	}
    }
#else

     init_memc(mem_size, ddr_clk);

     memsys_init_opt[5] = option_shmoo_only;         // run SHMOO only at this time
     rc = memsys_init((uint32_t*)pMCB->pMcbData, memsys_init_opt);
     if( rc )
         printf("memsys_init call shmoo returns 0x%08x\r\n", rc);

     /* make sure the memc register write is completed before we access DDR */
     data = MEMC->GLB_GCFG;
     __asm__ __volatile__ ("dsb");
#endif

#if defined(_BCM963138_)
     /* 63138 A0 can only support up to 1x256MBor 2x128MB, limit the total size to 256MB for now */
     if( PERF->RevID == 0x631380a0 && total_size_mbit > 2048 )
         total_size_mbit = 2048;
#endif

     /* don't know the exact memory size in safe mode, limit to the minimum size */
     if( is_safemode )
         total_size_mbit = 1024;

     rc = 0;
#ifdef AUTO_SIZE    
    if( size_detected )
    {
        //set the size the GCFG register 
        MEMC->GLB_GCFG &= ~MEMC_GLB_GCFG_SIZE1_MASK;
        size = mc_dram_profile.size_Mbits>>3; 
        i = 0;
        while((size = size >> 1))
	    i++;
        MEMC->GLB_GCFG &= ~MEMC_GLB_GCFG_SIZE1_MASK;
        MEMC->GLB_GCFG |= (i<<MEMC_GLB_GCFG_SIZE1_SHIFT);  /* cfg_size1 = log2(memsize in MB) */
    }
    else
    {
        board_setledsC('S','I','Z','X');   // SIZX
        board_setledsC('F','A','I','L');   // FAIL
        rc = -1;
    }
#else
     size = total_size_mbit>>3; 
     i = 0;
     while((size = size >> 1))
         i++;
     MEMC->GLB_GCFG &= ~MEMC_GLB_GCFG_SIZE1_MASK;
     MEMC->GLB_GCFG |= (i<<MEMC_GLB_GCFG_SIZE1_SHIFT);  /* cfg_size1 = log2(memsize in MB) */
     size_detected = 1;
#endif

#ifdef AUTO_SIZE  
    // Init PHY and Run SHMOO with optimal speed and reconfigure the memc
    if( is_safemode == 0 && rc == 0 )
    {
        // reconfigure the memc with detected size and optimal speed
        init_memc(size_index,DDR_CLK_800MHZ);  

        memsys_init_opt[0] = 0;             // 0 = not using Andover MEMC
        memsys_init_opt[1] = DDRPHY_BASE;   // PHY register base
        memsys_init_opt[2] = 0;             // 0 = not using Andover MEMC
        memsys_init_opt[3] = 0x80002500;    // EDIS register base 
        memsys_init_opt[4] = 1;             // right now only use 1 EDIS
        memsys_init_opt[5] = 0x70|option_dbgmsg_disable; // do not run shmoo at this time
        memsys_init_opt[6] = 0;
        memsys_init_opt[7] = 0;
        data = memsys_init((uint32_t*)mcb, memsys_init_opt);
        if( data )
            printf("memsys_init call phy init returns 0x%08x\r\n", data);

      
        memsys_init_opt[5] = option_shmoo_only; // run SHMOO only at this time
        data = memsys_init((uint32_t*)mcb, memsys_init_opt);
        if( data )
            printf("memsys_init call shmoo returns 0x%08x\r\n", data);

    }
#endif

    armv7_branch_predict_enable();

#ifndef AUTO_SIZE  
    /* if user specify the size, we need to make sure it is not larger the actual size */
    if( is_safemode == 0 )
    {
        if( memc_alias_test(total_size_mbit))
	{
	    printf("\nMemory alias detected. Probably wrong memory size is specified or memory subsystem not working\n");
	    return -3;
	}
    }
#endif

    if( size_detected )
    {
        uint32_t val = 0, i;
        uint32_t* addr = (uint32*)DRAM_BASE_NOCACHE, *temp;
        
        for( temp = addr, i = 0; i < 1024; i++ )
	     *temp++ = i;
        
       for( temp = addr, i = 0; i < 1024; i++ )
       {
	   val = *temp++;
           if( val != i )
	     break;
       }
       
       if( i == 1024 )
            board_setledsC('P','A','S','S'); // PASS
       else
       {
            board_setledsC('F','A','I','L'); // FAIL
            rc = -2;
       }
    }

    return rc;
}

