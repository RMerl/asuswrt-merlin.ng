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
#include "lib_printf.h"
#include "bcm_map.h"
#include "bcm_hwdefs.h"
#include "bcm63xx_impl1_ddr_cinit.h"
#include "boardparms.h"
#include "memsys_top_api.h"
#include "pmc_drv.h"

#define MEMC_DEBUG	1

extern int board_puts(const char*);
extern uint32_t otp_get_max_ddr_freq(void);
extern uint32_t otp_get_max_clk_sel(void);

#ifdef MEMC_DEBUG
#define print_log xprintf
#else
#define print_log(format, ...) 
#endif

#define KSEG1_TO_PHYS(x)	(x & 0x1FFFFFFF)

#define SHMOO_TEST_SIZE			131072    /*128KB */


/*************************************************************************
              MEMC VARIABLES
*************************************************************************/
static memc_bus_cfg            mc_cfg;
static memc_dram_profile       mc_dram_profile;
/************** END - MEMC VARIABLES * **********************************/

/********************************************************************
     INTERNAL MEMC FUNCTIONS
********************************************************************/
static void mc_cfg_init(unsigned int* pMCB);
static void init_memc_dram_profile(unsigned int* pMCB);
static void cfg_memc_timing_ctrl(void);
static void cfg_map_addr_bits_to_row_col_ba(void);

/********** END - INTERNAL MEMC FUNCTIONS **************************/

extern mcbindex MCB[];
static mcbindex* memc_get_mcb(void);
uint32_t clks_per_usec;

/*************** CALLBACK FUNCTIONS ************************************/
extern int memsys_register_flow_control_user_callbacks(memsys_flow_control_user_callbacks_t *memsys_flow_cb);
static int memsys_begin(memsys_top_params_t *params, void *args);
static int pre_shmoo(memsys_top_params_t *params, void *args);
static int memsys_end(memsys_top_params_t *params, void *args);
static int disable_dram_refresh(memsys_top_params_t *params, void *args);
static int enable_dram_refresh(memsys_top_params_t *params, void *args);
static int memsys_udelay(uint32_t us);
static int memsys_putchar(char c);
uint32_t memc_init(memsys_top_params_t *params, uint32_t options);

/********************** memc col/row/ba addr register data table ****************************************/
uint32_t memc_addr_tbl[2][3][16] =  {
                                        {
                                    // Index:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
                                            { 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 }, // Row
                                            {  0,  0,  0,  4,  5,  9, 10, 11, 12, 13,  0,  0,  0,  0,  0 },     // Column
                                            {  6,  7,  8 }                                                      // Bank
                                        },
                                        {
                                   //  Index:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
                                            { 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 }, // Row
                                            {  0,  0,  0,  4,  5,  6,  7,  8,  9, 10,  0,  0,  0,  0,  0 }, // Column
                                            { 11, 12, 13 }                                                  // Bank
                                        }
                                    };

/************* END - MEMC TABLES *********************************************/

extern void cfe_usleep(int usec);
extern void board_putc(char c);
#if (INC_NAND_FLASH_DRIVER==1)
extern void rom_nand_flash_init(void);
extern int nand_read_buf(unsigned short blk, int offset, unsigned char *buffer, int len);
#if (INC_SPI_NAND_DRIVER==1)
extern void rom_spi_nand_init(void);
extern int strap_check_spinand(void);
#endif
#endif

static void memc_delay(unsigned int ns)
{
    while(ns--);
}

// This function is used to configure the refresh control register.
static void refresh_ctrl_cfg(uint32_t dram_clk_freq, int ref_disable)
{
    uint32_t freq = dram_clk_freq;
    uint32_t ref_rate;
    uint32_t data;

    if (freq < 800+10 && freq > 800-10) freq = 800;
    if (freq < 666+10 && freq > 666-10) freq = 666;
    if (freq < 533+10 && freq > 533-10) freq = 533;
    if (freq < 400+10 && freq > 400-10) freq = 400;
    if (freq < 333+10 && freq > 333-10) freq = 333;
    if (freq < 266+10 && freq > 266-10) freq = 266;
    if (freq < 200+10 && freq > 200-10) freq = 200;
    if (freq < 133+10 && freq > 133-10) freq = 133;
    if (freq <  66+10 && freq >  66-10) freq =  66;

    // 2x mode:
    if (freq == 200) ref_rate = 0x2C;
    if (freq == 333) ref_rate = 0x4D;
    if (freq == 400) ref_rate = 0x5D;
    if (freq == 533) ref_rate = 0x7B;
    // 4x mode:
    if (freq == 667) ref_rate = 0x9A;
    if (freq == 800) ref_rate = 0xBA;

    data = (ref_rate << MEMC_CHN_TIM_CLKS_REF_RATE_SHIFT) | (ref_disable << MEMC_CHN_TIM_CLKS_REF_DISABLE_SHIFT);
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->CHN_TIM_CLKS), data);
    MEMC->CHN_TIM_CLKS =  data;
}

static int disable_dram_refresh(memsys_top_params_t *params, void *args)
{
    print_log("disable_dram_refresh\n\r");
    refresh_ctrl_cfg(mc_dram_profile.dram_clk, 1);

    return MEMSYS_ERROR_MEMC_NONE;
}

static int enable_dram_refresh(memsys_top_params_t *params, void *args)
{
    print_log("enable_dram_refresh\n\r");
    refresh_ctrl_cfg(mc_dram_profile.dram_clk, 0);

    return MEMSYS_ERROR_MEMC_NONE;
}

static int memsys_putchar(char c)
{
    board_putc(c);

    return MEMSYS_ERROR_MEMC_NONE;
}


static int memsys_udelay(uint32_t us)
{
    cfe_usleep(us);

    return MEMSYS_ERROR_MEMC_NONE;
}


int memsys_register_flow_control_user_callbacks(memsys_flow_control_user_callbacks_t *memsys_flow_cb)
{
    memsys_flow_cb->fp_memsys_begin = memsys_begin;
    memsys_flow_cb->fp_pre_memc_init = 0;
    memsys_flow_cb->fp_pre_shmoo = pre_shmoo;
    memsys_flow_cb->fp_memsys_end = memsys_end;
    memsys_flow_cb->fp_disable_dram_refresh = disable_dram_refresh;
    memsys_flow_cb->fp_enable_dram_refresh = enable_dram_refresh;
    memsys_flow_cb->fp_edis_int_enable = 0;
    memsys_flow_cb->fp_edis_int_ready = 0;
    memsys_flow_cb->fp_edis_int_clear = 0;

    return MEMSYS_ERROR_MEMC_NONE;
}

static mcbindex* memc_get_mcb(void)
{
    mcbindex* pRet = NULL;

    uint32_t size_index;   // 0-128MB,  1-256MB 
    uint32_t speed_index, speed_otp, mcb_index, mips_otp, mips_index, clk_sel_otp, clk_sel_strap, memcfg;

    clk_sel_otp = otp_get_max_clk_sel();
    clk_sel_strap = ((MISC->miscStrapBus & MISC_STRAP_CLOCK_SEL_MASK) >> MISC_STRAP_CLOCK_SEL_SHIFT);
    if ( (clk_sel_otp==0x7) && (clk_sel_strap==0x7) )    
    {
        int ret;
        uint32 data;

        ret = ReadBPCMRegister(5, 11, &data);
        data |= 0x80000000;
        ret |= WriteBPCMRegister(5, 11, data);
        ret |= ReadBPCMRegister(5, 11, &data);
        data |= 0x00070000;
        ret |= WriteBPCMRegister(5, 11, data);
        if (ret != 0)
        {
            printf("failed getting configuration\n");
            while(1);
        }

        mips_otp = 600;
        mips_index = 600;
    }
    else
    {
    // get mips clk configuration (used for sleep function).
    mips_otp = (otp_get_max_clk_sel() & MISC_STRAP_CLOCK_SEL_400) ? 400 : 250;
    mips_index = (((MISC->miscStrapBus & MISC_STRAP_CLOCK_SEL_MASK) >> MISC_STRAP_CLOCK_SEL_SHIFT) & MISC_STRAP_CLOCK_SEL_400) ? 400 : 250;
    }
    clks_per_usec = (mips_index <= mips_otp) ? mips_index : mips_otp;

    // get size of DDR
#if (INC_NAND_FLASH_DRIVER==1)
#if (INC_SPI_NAND_DRIVER==1)
    if (strap_check_spinand())
        rom_spi_nand_init();
    else
#endif
    rom_nand_flash_init();
    nand_read_buf(0, IMAGE_OFFSET + 0x580 + offsetof(NVRAM_DATA,ulMemoryConfig), (unsigned char *)&memcfg, 4);
#else
    memcfg = *((uint32*)((FLASH_BASE + IMAGE_OFFSET + 0x580 + offsetof(NVRAM_DATA,ulMemoryConfig))));
#endif
    printf("NVRAM memcfg 0x%x\n\r", memcfg);
    if ((memcfg&BP_DDR_TOTAL_SIZE_MASK)==BP_DDR_TOTAL_SIZE_128MB)
        size_index = 0;
    else if ((memcfg&BP_DDR_TOTAL_SIZE_MASK)==BP_DDR_TOTAL_SIZE_256MB)
        size_index = 1;
    else if ((memcfg&BP_DDR_TOTAL_SIZE_MASK)==BP_DDR_TOTAL_SIZE_64MB)
        size_index = 2;
    else
        size_index = ((MISC->miscStrapBus & MISC_STRAP_DDR_256M_EN_MASK) >> MISC_STRAP_DDR_256M_EN_SHIFT) ? 0 : 1;

    speed_index = (MISC->miscStrapBus & MISC_STRAP_DDR_FREQ_MASK) >> MISC_STRAP_DDR_FREQ_SHIFT;
    if (!speed_index)
        speed_index = 3;
    speed_otp = otp_get_max_ddr_freq();
    speed_index = (speed_otp < speed_index) ? speed_otp:speed_index;

    if (speed_otp == 0)
    {
        xprintf("CFE hangs - OTP is not written\n");
        while(1);
    }

    mcb_index = 3*size_index + speed_index;
    printf("mcb configuration = %d\n\r", mcb_index);

    if ( (mcb_index<1) || (mcb_index>9) || !MCB[mcb_index].pMcbData )
        printf("Error: Wrong strap configuration %d \n\r", mcb_index);
    else
        pRet = &MCB[mcb_index];

    if ( (pRet->config & BP_DDR_TOTAL_SIZE_MASK) == BP_DDR_TOTAL_SIZE_128MB )
        mc_dram_profile.size_Mbits = 128 * 8;
    else if ( (pRet->config & BP_DDR_TOTAL_SIZE_MASK) == BP_DDR_TOTAL_SIZE_256MB )
        mc_dram_profile.size_Mbits = 256 * 8;
    else if ( (pRet->config & BP_DDR_TOTAL_SIZE_MASK) == BP_DDR_TOTAL_SIZE_64MB )
        mc_dram_profile.size_Mbits = 64 * 8;

    return pRet;
}


static void mc_cfg_init(unsigned int* pMCB)
{
    int i;

    print_log("mc_cfg_init(): Initialize the default values on mc_cfg\n\r");

    mc_cfg.arb_burst_mode = 1;
    mc_cfg.arb_rr_mode = 1;
    mc_cfg.gcfg_seq_mode = 0;

    for (i=0; i < MC_ARB_SP_Q_NUM; i++) 
    {
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
    mc_cfg.ub0_wrep_mode      = 0;
    mc_cfg.ub0_wburst_mode    = 0;
    mc_cfg.ub0_fifo_mode      = 0;

    for (i=0; i < MC_UB_SRC_ID_NUM; i++)
        mc_cfg.ub1_srcid_q[i] = 0;
    mc_cfg.ub1_oob_err_mask   = 0;
    mc_cfg.ub1_ib_err_mask    = 0;
    mc_cfg.ub1_wrep_mode      = 0;
    mc_cfg.ub1_wburst_mode    = 0;
    mc_cfg.ub1_fifo_mode      = 0;

    mc_cfg.gcfg_max_age       = (pMCB[MCB_EXT_CFG_WORD] & MCB_EXT_CFG_MAX_AGE_MASK) >> MCB_EXT_CFG_MAX_AGE_SHIFT;
    mc_cfg.gcfg_perf_tweaks   = (pMCB[MCB_EXT_CFG_WORD] & MCB_EXT_CFG_PERF_MASK) >> MCB_EXT_CFG_PERF_SHIFT;

    return;
}

static void init_memc_dram_profile(unsigned int* pMCB)
{
    print_log("init_memc_dram_profile(): Initializing MEMC DRAM profile\n\r");
  
    // Setup profile values
    mc_dram_profile.row_bits   = ((pMCB[MCB_DENSITY_WORD] & MCB_DENSITY_ROWS_MASK) >> MCB_DENSITY_ROWS_SHIFT) + 12;
    mc_dram_profile.col_bits   = ((pMCB[MCB_DENSITY_WORD] & MCB_DENSITY_COLUMNS_MASK) >> MCB_DENSITY_COLUMNS_SHIFT) + 9;
    mc_dram_profile.ba_bits    = ((pMCB[MCB_DENSITY_WORD] & MCB_DENSITY_BANKS_MASK) >> MCB_DENSITY_BANKS_SHIFT) ? 3 : 2;

    //[DH] For now, only supporting 16-bit DRAM width with 16 byte access.
    mc_dram_profile.width_bits   = 16;
    mc_dram_profile.access_bytes = 16;
    mc_dram_profile.ref_clk = pMCB[MCB_PHY_REF_CLK_WORD];

    // Setup timing values
    mc_dram_profile.dram_clk = pMCB[MCB_DRAM_DATA_CLK_WORD];

    mc_dram_profile.ddr_2T_mode  = 1;
    mc_dram_profile.ddr_hdp_mode = 0;
    mc_dram_profile.large_page   = 1;

    mc_dram_profile.timing_cfg.tRCD =  (pMCB[MCB_TIMING0_WORD] & MCB_TIMING0_tRCD_MASK) >> MCB_TIMING0_tRCD_SHIFT;
    mc_dram_profile.timing_cfg.tCL  =  (pMCB[MCB_TIMING1_WORD] & MCB_TIMING1_tCL_MASK) >> MCB_TIMING1_tCL_SHIFT;
    mc_dram_profile.timing_cfg.tWR  =  (pMCB[MCB_TIMING1_WORD] & MCB_TIMING1_tWR_MASK) >> MCB_TIMING1_tWR_SHIFT;
    mc_dram_profile.timing_cfg.tCwl =  (pMCB[MCB_TIMING1_WORD] & MCB_TIMING1_tCWL_MASK) >> MCB_TIMING1_tCWL_SHIFT;
    mc_dram_profile.timing_cfg.tRP  =  (pMCB[MCB_TIMING0_WORD] & MCB_TIMING0_tRP_MASK) >> MCB_TIMING0_tRP_SHIFT;
    mc_dram_profile.timing_cfg.tRAS =  (pMCB[MCB_TIMING0_WORD] & MCB_TIMING0_tRAS_MASK) >> MCB_TIMING0_tRAS_SHIFT;
    mc_dram_profile.timing_cfg.tRCr =  mc_dram_profile.timing_cfg.tRP + mc_dram_profile.timing_cfg.tRAS;
    mc_dram_profile.timing_cfg.tAL  =  (pMCB[MCB_TIMING2_WORD] & MCB_TIMING2_tAL_MASK) >> MCB_TIMING2_tAL_SHIFT;
    mc_dram_profile.timing_cfg.tRTP =  (pMCB[MCB_TIMING1_WORD] & MCB_TIMING1_tRTP_MASK) >> MCB_TIMING1_tRTP_SHIFT;
    mc_dram_profile.timing_cfg.tWTR =  (pMCB[MCB_TIMING2_WORD] & MCB_TIMING2_tWTR_MASK) >> MCB_TIMING2_tWTR_SHIFT;
    mc_dram_profile.timing_cfg.tW2R =  (pMCB[MCB_EXT_CFG_WORD] & MCB_EXT_CFG_tW2R_MASK) >> MCB_EXT_CFG_tW2R_SHIFT;
    mc_dram_profile.timing_cfg.tW2W =  (pMCB[MCB_EXT_CFG_WORD] & MCB_EXT_CFG_tW2W_MASK) >> MCB_EXT_CFG_tW2W_SHIFT;
    mc_dram_profile.timing_cfg.tR2R =  (pMCB[MCB_EXT_CFG_WORD] & MCB_EXT_CFG_tR2R_MASK) >> MCB_EXT_CFG_tR2R_SHIFT;
    mc_dram_profile.timing_cfg.tR2W =  (pMCB[MCB_EXT_CFG_WORD] & MCB_EXT_CFG_tR2W_MASK) >> MCB_EXT_CFG_tR2W_SHIFT;
    mc_dram_profile.timing_cfg.tCCD =  4;
    mc_dram_profile.timing_cfg.tRRD =  (pMCB[MCB_TIMING0_WORD] & MCB_TIMING0_tRRD_MASK) >> MCB_TIMING0_tRRD_SHIFT;
    mc_dram_profile.timing_cfg.tFAW =  (pMCB[MCB_TIMING3_WORD] & MCB_TIMING3_tFAW_MASK) >> MCB_TIMING3_tFAW_SHIFT;
    mc_dram_profile.timing_cfg.tRFC =  (pMCB[MCB_TIMING2_WORD] & MCB_TIMING2_tRFC_MASK) >> MCB_TIMING2_tRFC_SHIFT;
   
    return;
}

// This function is used to program the MEMC timing registers. 
// It's almost fully based on the prog_timing_cntrl legacy function.
static void cfg_memc_timing_ctrl(void) 
{
    uint32_t tim1_0 = 0;
    uint32_t tim1_1 = 0;
    uint32_t tim1_2 = 0;
    uint32_t tim1_3 = 0;
    uint32_t tim2   = 0;
    uint32_t mcregval;

	print_log("FUNC cfg_memc_timing_ctrl\n\r");

    mcregval = mc_dram_profile.timing_cfg.tAL;
   
    /* adjust for DDR3, assume DDR3 for now */
    mcregval = 0;
    if (mc_dram_profile.timing_cfg.tAL > 0) 
    {
        mcregval = mc_dram_profile.timing_cfg.tCL - mc_dram_profile.timing_cfg.tAL;
        print_log("cfg_memc_timing_ctrl() : Adjusted tAL cfg = %d (original: tAL=%d; tCL=%d)\n\r", 
            mcregval, mc_dram_profile.timing_cfg.tAL, mc_dram_profile.timing_cfg.tCL);
        mc_dram_profile.timing_cfg.tAL = mcregval;
    }
  
    tim1_0 = (
                ((mc_dram_profile.timing_cfg.tCwl << MC_CHN_TIM_TIM1_0_TIM1_tWL_SHIFT)  & MC_CHN_TIM_TIM1_0_TIM1_tWL_MASK) |
                ((mc_dram_profile.timing_cfg.tRP  << MC_CHN_TIM_TIM1_0_TIM1_tRP_SHIFT)  & MC_CHN_TIM_TIM1_0_TIM1_tRP_MASK) |
                ((mc_dram_profile.timing_cfg.tCL  << MC_CHN_TIM_TIM1_0_TIM1_tCL_SHIFT)  & MC_CHN_TIM_TIM1_0_TIM1_tCL_MASK) |
                ((mc_dram_profile.timing_cfg.tRCD << MC_CHN_TIM_TIM1_0_TIM1_tRCD_SHIFT) & MC_CHN_TIM_TIM1_0_TIM1_tRCD_MASK) 
             );

    tim1_1 = (
                ((mc_dram_profile.timing_cfg.tCCD << MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_SHIFT) & MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_MASK) |
                ((mc_dram_profile.timing_cfg.tRRD << MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_SHIFT) & MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_MASK)
             );

    tim1_2 = (
                ((mc_dram_profile.timing_cfg.tFAW << MC_CHN_TIM_TIM1_2_TIM1_tFAW_SHIFT) & MC_CHN_TIM_TIM1_2_TIM1_tFAW_MASK) |
                ((mc_dram_profile.timing_cfg.tRTP << MC_CHN_TIM_TIM1_2_TIM1_tRTP_SHIFT) & MC_CHN_TIM_TIM1_2_TIM1_tRTP_MASK) |
                ((mc_dram_profile.timing_cfg.tRCr << MC_CHN_TIM_TIM1_2_TIM1_tRC_SHIFT)  & MC_CHN_TIM_TIM1_2_TIM1_tRC_MASK)
             );

    tim1_3 = (
                ((mc_dram_profile.timing_cfg.tWTR << MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_SHIFT) & MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_MASK) |
                ((mc_dram_profile.timing_cfg.tWR  << MC_CHN_TIM_TIM1_3_TIM1_tWR_S_SHIFT)  & MC_CHN_TIM_TIM1_3_TIM1_tWR_S_MASK)
             );

    tim2   = (
                ((mc_dram_profile.timing_cfg.tR2R << MC_CHN_TIM_TIM2_TIM2_tR2R_SHIFT) & MC_CHN_TIM_TIM2_TIM2_tR2R_MASK) |
                ((mc_dram_profile.timing_cfg.tR2W << MC_CHN_TIM_TIM2_TIM2_tR2W_SHIFT) & MC_CHN_TIM_TIM2_TIM2_tR2W_MASK) |
                ((mc_dram_profile.timing_cfg.tW2R << MC_CHN_TIM_TIM2_TIM2_tW2R_SHIFT) & MC_CHN_TIM_TIM2_TIM2_tW2R_MASK) |
                ((mc_dram_profile.timing_cfg.tW2W << MC_CHN_TIM_TIM2_TIM2_tW2W_SHIFT) & MC_CHN_TIM_TIM2_TIM2_tW2W_MASK) |
                ((mc_dram_profile.timing_cfg.tAL  << MC_CHN_TIM_TIM2_TIM2_tAL_SHIFT)  & MC_CHN_TIM_TIM2_TIM2_tAL_MASK)  |
                ((mc_dram_profile.timing_cfg.tRFC << MC_CHN_TIM_TIM2_TIM2_tRFC_SHIFT) & MC_CHN_TIM_TIM2_TIM2_tRFC_MASK)
             );

    MEMC->CHN_TIM_TIM1_0 = tim1_0;
    MEMC->CHN_TIM_TIM1_1 = tim1_1;
    MEMC->CHN_TIM_TIM1_2 = tim1_2;
    MEMC->CHN_TIM_TIM1_3 = tim1_3;
    MEMC->CHN_TIM_TIM2   = tim2;

    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->CHN_TIM_TIM1_0), tim1_0);
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->CHN_TIM_TIM1_1), tim1_1);
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->CHN_TIM_TIM1_2), tim1_2);
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->CHN_TIM_TIM1_3), tim1_3);
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->CHN_TIM_TIM2), tim2);

    print_log("END FUNC cfg_memc_timing_ctrl\n\r");

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
         mi = (mc_dram_profile.large_page==0) ? 0 : 1;
    else 
    {
        print_log("cfg_map_addr_bits_to_row_col_ba() : The following profile is not supported: width_bits=%d ; access_bytes=%d\n\r",
                  mc_dram_profile.width_bits, mc_dram_profile.access_bytes);
        return;
    }

    for ( i=0; i<9; i++)   // i is to iterate over registers
    {
        reg_val = 0;
        reg_update_flag = 0;

        if      (i == 0) { mii = 0; miii = 0; max_bits = mc_dram_profile.row_bits; } // Row
        else if (i == 4) { mii = 1; miii = 0; max_bits = mc_dram_profile.col_bits; } // Column
        else if (i == 8) { mii = 2; miii = 0; max_bits = mc_dram_profile.ba_bits;  } // Bank

        for (j=0; j<4; j++)   // j is to iterate over fields
        {
            if (miii < max_bits) 
            {
                reg_update_flag = 1;
                reg_val += memc_addr_tbl[mi][mii][miii] << (8 * j);
                miii++;
            }
        }
        if (reg_update_flag) 
        {
            print_log("[0x%08x] = 0x%08x\n\r", cfg_regs[i], reg_val);
            *(cfg_regs[i]) = reg_val;
        }
    }

    return;
}

static int memsys_begin(memsys_top_params_t *params, void *args)
{
    uint32_t data;
 
    print_log("start of memsys_begin\n\r");

    mc_cfg_init((unsigned int*)params->mcb_addr);
    init_memc_dram_profile((unsigned int*)params->mcb_addr);

    // Poll the PHY Status Register
    print_log("Poll PHY Status register\n\r");
    data = MEMC->CHN_TIM_PHY_ST;
    print_log("PHY Status= %x\n\r", data);
    // - check Power_Up to see if PHY is ready to receive register access
    // - check SW_Reset
    // - check HW_Reset
    while (!((data & MEMC_CHN_TIM_PHY_ST_PHY_ST_POWER_UP) ||
          (data & MEMC_CHN_TIM_PHY_ST_PHY_ST_SW_RESET) ||
          (data & MEMC_CHN_TIM_PHY_ST_PHY_ST_HW_RESET)))
    {
        memc_delay(200);
        data = MEMC->CHN_TIM_PHY_ST;
        print_log("PHY Status= %x\n\r", data);
    }

    print_log("Disable Auto-Refresh\n\r");
    refresh_ctrl_cfg(mc_dram_profile.dram_clk, 1);

    // This was added because after the DDR PHY init sequence (i.e. ddr34_phy_init()) finishes, CKE is disabled.
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->CHN_TIM_DCMD), 0x00000305);
    MEMC->CHN_TIM_DCMD = 0x00000305; // Enable CKE

    print_log("End of memsys_begin\n\r");

    return MEMSYS_ERROR_MEMC_NONE;
}

uint32_t memc_init(memsys_top_params_t *params, uint32_t options)
{
    uint32_t data;

    print_log("start of memc_init\n\r");

    /* set 4x mode */
    data = MEMC->GLB_GCFG;
    data |= MEMC_GLB_GCFG_PHY_4X_MASK;
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->GLB_GCFG), data);
    MEMC->GLB_GCFG = data;

    // use pll clock
    data = MEMC->GLB_GCFG;
    data |= MEMC_GLB_GCFG_MCLKSRC_MASK;
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->GLB_GCFG), data);
    MEMC->GLB_GCFG = data;

    /* Enable DFI */
    data = MEMC->GLB_GCFG;
    data |= MEMC_GLB_GCFG_DFI_EN_MASK;
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->GLB_GCFG), data);
    MEMC->GLB_GCFG = data;

    /* Set DDR3 mode */
    data = MEMC->CHN_TIM_DRAM_CFG;
    data |= (data & ~MEMC_CHN_TIM_DRAM_CFG_DRAMTYPE_MASK) | MC_DRAM_CFG_DDR3;
    data = (data & ~MEMC_CHN_TIM_DRAM_CFG_2TADDRCMD_MASK) | ((mc_dram_profile.ddr_2T_mode & 0x1) << MEMC_CHN_TIM_DRAM_CFG_2TADDRCMD_SHIFT);
    data = (data & ~MEMC_CHN_TIM_DRAM_CFG_HDP_MASK) | ((mc_dram_profile.ddr_hdp_mode & 0x1) << MEMC_CHN_TIM_DRAM_CFG_HDP_SHIFT);
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->CHN_TIM_DRAM_CFG), data);
    MEMC->CHN_TIM_DRAM_CFG = data;

    print_log("Enable Auto-Refresh\n\r");
    refresh_ctrl_cfg(mc_dram_profile.dram_clk, 0);

    cfg_map_addr_bits_to_row_col_ba();
    cfg_memc_timing_ctrl();

    print_log("End of memc_init\n\r");

    return MEMSYS_ERROR_MEMC_NONE;
}

static int pre_shmoo(memsys_top_params_t *params, void *args)
{
    uint32_t data;

    print_log("start of pre_shmoo\n\r");

    data = MEMC->GLB_GCFG;
    data |= MEMC_GLB_GCFG_MEMINITDONE_MASK;
    data |= MEMC_GLB_GCFG_DRAM_EN_MASK;
    data |= MEMC_GLB_GCFG_FORCE_SEQ_MASK;
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->GLB_GCFG), data);
    MEMC->GLB_GCFG = data;

    print_log("end of pre_shmoo\n\r");

    return MEMSYS_ERROR_MEMC_NONE;
}

static int memsys_end(memsys_top_params_t *params, void *args)
{
    uint32_t data, size, i;

    print_log("start of memsys_end\n\r");

    data = MEMC->GLB_GCFG;
    data &= ~MEMC_GLB_GCFG_FORCE_SEQ_MASK;
    data &= ~MEMC_GLB_GCFG_RBF_REORDER_MASK;
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->GLB_GCFG), data);
    MEMC->GLB_GCFG = data;

    /* set DDR size */
    size = mc_dram_profile.size_Mbits >> 3;
    i = 0;
    while(size)
    {
        size = (size >> 1);
        if (size)
            i++;
    }
    MEMC->GLB_GCFG &= ~MEMC_GLB_GCFG_SIZE1_MASK;
    MEMC->GLB_GCFG |= (i<<MEMC_GLB_GCFG_SIZE1_SHIFT);  /* cfg_size1 = log2(memsize in MB) */
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->GLB_GCFG), MEMC->GLB_GCFG);

    /* Configure memory range accessed by LMB (0x0000_0000 - 0x0FFF_FFFF) */
    MEMC->LMBIF0_SDRAM_SPACE = MEMC_LMBIF0_SDRAM_SPACE_256M;

    print_log("end of memsys_end\n\r");

    return MEMSYS_ERROR_MEMC_NONE;
}

//*****************************************
// This is the main initialization function
//*****************************************
void board_draminit(void) 
{
    mcbindex* pMCB;
    uint32_t data, ret;
    memsys_top_params_t params;
    memsys_system_callbacks_t callbacks = {
        memsys_udelay,
        memsys_putchar,
        0,
        0,
    };
    uint32_t options = 0;
    uint32_t* temp;
    uint32_t* addr = (uint32_t*)DRAM_BASE_NOCACHE;
    int i;

    xprinthook = board_puts;

    xprintf(" ***** DDR INITIALIZATION START ******\n\r");

    pMCB = memc_get_mcb();
    if (!pMCB)
        while(1);

    params.version = MEMSYS_FW_VERSION;
    params.edis_info = 1;
    params.mem_test_size_bytes = 128 * 1024;      // test size for SHMOO
    params.phys_mem_test_base = 0;                // test physical address base for SHMOO
    params.phys_memc_reg_base = (MEMC_BASE);
    params.phys_phy_reg_base = ((uint32_t)(&MEMC->PhyControl[0]));
    params.phys_shim_reg_base = 0;
    params.phys_edis_reg_base = ((uint32_t)(&MEMC->EDIS_0));
    params.saved_state_base = 0;
    params.callbacks = &callbacks;
    params.mcb_addr = (uint32_t*)pMCB->pMcbData;
    params.options = options;

    ret = memsys_top(&params); 

 	// return on error
	if (ret != 0)
	{ 
        int idx;
        xprintf("MEMSYS init failed, return code %08X\n\r", ret);

        xprintf("MEMC error: ");
        for (idx = 0; idx < MEMSYS_ERROR_MEMC_MAX_WORDS; idx++)
            xprintf(" 0x%08x \n\r", params.error.memc[idx]);
        xprintf("PHY error: ");
        for (idx = 0; idx < MEMSYS_ERROR_PHY_MAX_WORDS; idx++)
            xprintf(" 0x%08x \n\r", params.error.phy[idx]);
        xprintf("SHMOO error: ");
        for (idx = 0; idx < MEMSYS_ERROR_SHMOO_MAX_WORDS; idx++)
            xprintf(" 0x%08x \n\r", params.error.shmoo[idx]);
        xprintf("\n\r");
        goto init_ddr_fail;
    }

    for( temp = addr, i = 0; i < 1024; i++ )
        *temp++ = i;
        
    for( temp = addr, i = 0; i < 1024; i++ )
    {
        data = *temp++;
        if( data != i )
            break;
    }
       
    if( i == 1024 )  //pass
        xprintf("DDR test done successfully\n\r");
    else
    {
        xprintf("DDR test failed i=%d\n\r", i);
        goto init_ddr_fail;
    }

    xprintf("***** DDR INITIALIZATION DONE ********\n\r");

    return;

init_ddr_fail:

    while(1);
}

