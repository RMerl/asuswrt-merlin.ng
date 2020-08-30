/*  *********************************************************************
    *
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
    ********************************************************************* */

#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bcm_map.h"
#include "bcm_hwdefs.h"
#include "bcm63xx_impl2_ddr_cinit.h"
#include "boardparms.h"
#include "rom_parms.h"
#include "memsys_top_api.h"
#include "bcm_sec.h"

#if defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM96856_)
#define MEMC_DEBUG       1
#endif
#ifdef MEMC_DEBUG
#define print_log xprintf
#else
#define print_log(format, ...)
#endif

#define SHMOO_TEST_SIZE               131072    /*128KB */

#if defined(_BCM94908_) || defined(_BCM963158_) || defined(_BCM96858_)
#define MEMSTRT_UNSCRAM_WINDOW_SIZE    (0) /* Size in bytes, of unscrambled region at start of DRAM */
#define MEMEND_UNSCRAM_WINDOW_SIZE     (0) /* Size in bytes, of unscrambled region at end of DRAM */
#endif

int is_safemode = 0;
uint32_t memcfg = 0;

/*************************************************************************
       MEMC VARIABLES
*************************************************************************/
static memc_bus_cfg                   mc_cfg;
static memc_dram_profile_struct       memc_dram_profile;
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
static mcbindex* memc_get_mcb(uint32_t memcfg);
#if defined(_BCM94908_) || defined(_BCM963158_) || defined(_BCM96858_)
static int memc_waitfor_scrambler_status(uint32_t status);
static int memc_setup_scrambler(uint32_t memsize_Mbit);
#endif
static int memc_alias_test(uint32_t memsize);
static int memc_memory_test(uint32_t memsize);
static void memc_print_cfg(void);

/*************** CALLBACK FUNCTIONS ************************************/
extern int memsys_register_flow_control_user_callbacks(memsys_flow_control_user_callbacks_t *memsys_flow_cb);
static int memsys_begin(memsys_top_params_t *params, void *args);
static int pre_shmoo(memsys_top_params_t *params, void *args);
static int memsys_end(memsys_top_params_t *params, void *args);
static int disable_dram_refresh(memsys_top_params_t *params, void *args);
static int enable_dram_refresh(memsys_top_params_t *params, void *args);
static int memsys_udelay(uint32_t us);
static int memsys_putchar(char c);
#if defined(_BCM94908_)
uint32_t memc_init(memsys_top_params_t *params, uint32_t options);
#elif defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_)
static int memc_init(memsys_top_params_t *params, void *args);
extern uint64_t cfe_get_utime(void);
#endif

/* string tables */
char* str_speed[] = {
                         "800 CL6",
                         "1066 CL7",
                         "1066 CL8",
                         "1333 CL9",
                         "1333 CL10",
                         "1600 CL10",
                         "1600 CL11",
                         "2133 CL11",
                         "2133 CL12",
                         "2133 CL13",
                         "2133 CL14",
                         "1866 CL10",
                         "1866 CL11",
                         "1866 CL12",
                         "1866 CL13",
                         "Custom Speed",
                     };

char* str_ssc[4] = {
                         "None",   /* BP_DDR_SSC_CONFIG_NONE */
                         "%1",     /* BP_DDR_SSC_CONFIG_1 */
                         "%0.5",   /* BP_DDR_SSC_CONFIG_2 */
                         "Custom", /* BP_DDR_SSC_CONFIG_CUSTOM */
                     };

/********************** memc col/row/ba addr register data table ****************************************/
uint32_t memc_addr_tbl[][3][16]  =  {
      {
      // DRAM width 16 bits
      // Index:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
      // -------------------------------------------------------------------------------
              { 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 }, // Row
              {  0,  0,  0,  4,  5,  9, 10, 11, 12, 13 },                         // Column
              {  6,  7,  8 },                                                     // Bank
      },
      {
      // DRAM width 16 bits and large_page version of 0
      // Index:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 
      // -------------------------------------------------------------------------------
              { 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 }, // Row
              {  0,  0,  0,  4,  5,  6,  7,  8,  9, 10 },                         // Column
              { 11, 12, 13 },                                                     // Bank
      },
      {
      // DRAM width 32 bits
      // Index:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
      // -------------------------------------------------------------------------------
              { 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 }, // Row
              {  0,  0,  0,  5,  9, 10, 11, 12, 13, 14 },                         // Column
              {  6,  7,  8 },                                                     // Bank
      },
      {
      // DRAM width 32 bits and large_page version of 2
      // Index:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
      // -------------------------------------------------------------------------------
              { 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 }, // Row
              {  0,  0,  0,  5,  6,  7,  8,  9, 10, 11 },                         // Column
              { 12, 13, 14 },                                                     // Bank
      },
      {
      // DDR3 DRAM width 16 bits, col 11 bits
      // Index:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
      // -------------------------------------------------------------------------------
              { 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 }, // Row
              {  0,  0,  0,  4,  5,  9, 10, 11, 12, 13,  0, 14 },                 // Column (note: bit 10 is skipped)
              {  6,  7,  8 },                                                     // Bank
      },
      {
      // DDR3 DRAM width 16 bits, col 11 bits and large_page 
      // Index:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 
      // -------------------------------------------------------------------------------
              { 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 }, // Row
              {  0,  0,  0,  4,  5,  6,  7,  8,  9, 10,  0, 11 },                 // Column (note: bit 10 is skipped)
              { 12, 13, 14 },                                                     // Bank
      },
      {
      // DDR3 DRAM width 32 bit, col 11 bits
      // Index:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
      // -------------------------------------------------------------------------------
              { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 }, // Row
              {  0,  0,  0,  5,  9, 10, 11, 12, 13, 14,  0, 15 },                 // Column (note: bit 10 is skipped)
              {  6,  7,  8 },                                                     // Bank
      },
      {
      // DDR3 DRAM width 32 bits, col 11 bits and large_page
      // Index:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
      // -------------------------------------------------------------------------------
              { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 }, // Row
              {  0,  0,  0,  5,  6,  7,  8,  9, 10, 11,  0, 12 },                 // Column (note: bit 10 is skipped)
              { 13, 14, 15 },                                                     // Bank
      }
};

/************* END - MEMC TABLES *********************************************/

extern void cfe_usleep(int usec);
extern void board_putc(char c);

// This function is used to configure the refresh control register.
static void refresh_ctrl_cfg(uint32_t dram_clk_freq, uint32_t high_temp, int ref_disable)
{
    uint32_t dram_clk_freq_MHz = dram_clk_freq;
    uint32_t ref_rate;
    // This is the formula by which the refresh rate is calculated:
    // Refresh Interval = Value x 32 x DDR clock period
    // The upper limit is 7.8us, so we'll configure to get close to it (let's say 7.5us) so that the refresh gets issued infrequently.
    // Value = 7.5us / (32 * (1/dram_clk_freq))
    // Value = (7500ns * dram_clk_freq[in MHz]) / (32 * 1000)
    // Examples:
    // 200MHz : (7500 * 200) / (32 * 1000) = 0x2E
    // 667MHz : (7500 * 667) / (32 * 1000) = 0x9C
    uint32 ref_interval_ns = 7300; // Need to set a little slower than 7500, since sometimes DDR clock is generated slower than ideal

    ref_rate = (ref_interval_ns * dram_clk_freq_MHz) / (32 * 1000);
    if( high_temp )
        ref_rate >>=  1;

    print_log("refresh_ctrl_cfg writing to refresh control register (dram_clk_freq_MHz=%d ; ref_rate=0x%x ; ref_disable=%d)\n", dram_clk_freq_MHz, ref_rate, ref_disable);
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->CHN_TIM_CLKS), (ref_rate << MEMC_CHN_TIM_CLKS_REF_RATE_SHIFT) | (ref_disable << MEMC_CHN_TIM_CLKS_REF_DISABLE_SHIFT));
#if defined(_BCM96856_)
    MEMC->CHN_TIM_CLKS = (0x4 << MEMC_CHN_TIM_CLKS_REF_MAX_DELAY_SHIFT) | (ref_rate << MEMC_CHN_TIM_CLKS_REF_RATE_SHIFT) | (ref_disable << MEMC_CHN_TIM_CLKS_REF_DISABLE_SHIFT);
#else
    MEMC->CHN_TIM_CLKS = (ref_rate << MEMC_CHN_TIM_CLKS_REF_RATE_SHIFT) | (ref_disable << MEMC_CHN_TIM_CLKS_REF_DISABLE_SHIFT);
#endif

    return;
}

static int disable_dram_refresh(memsys_top_params_t *params, void *args)
{
    print_log("disable_dram_refresh\n\r");
    refresh_ctrl_cfg(memc_dram_profile.dram_clk, memc_dram_profile.high_temp, 1);

    return MEMSYS_ERROR_MEMC_NONE;
}

static int enable_dram_refresh(memsys_top_params_t *params, void *args)
{
    print_log("enable_dram_refresh\n\r");
    refresh_ctrl_cfg(memc_dram_profile.dram_clk, memc_dram_profile.high_temp, 0);

    return MEMSYS_ERROR_MEMC_NONE;
}

static int memsys_putchar(char c)
{
    board_putc(c);

    return MEMSYS_ERROR_MEMC_NONE;
}

static int memsys_udelay(uint32_t us)
{
#ifndef CONFIG_BRCM_IKOS
    cfe_usleep(us);
#endif
    return MEMSYS_ERROR_MEMC_NONE;
}

#if defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_)
static uint32_t memsys_get_time_us(void)
{
    return (uint32_t)cfe_get_utime();
}
#endif

int memsys_register_flow_control_user_callbacks(memsys_flow_control_user_callbacks_t *memsys_flow_cb)
{
    memsys_flow_cb->fp_memsys_begin = memsys_begin;
    memsys_flow_cb->fp_pre_memc_init = 0;
#if defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_)
    memsys_flow_cb->fp_memc_init = memc_init;
#endif
    memsys_flow_cb->fp_pre_shmoo = pre_shmoo;
    memsys_flow_cb->fp_memsys_end = memsys_end;
    memsys_flow_cb->fp_disable_dram_refresh = disable_dram_refresh;
    memsys_flow_cb->fp_enable_dram_refresh = enable_dram_refresh;
    memsys_flow_cb->fp_edis_int_enable = 0;
    memsys_flow_cb->fp_edis_int_ready = 0;
    memsys_flow_cb->fp_edis_int_clear = 0;

    return MEMSYS_ERROR_MEMC_NONE;
}


/* return 1 if there is alias, 0 no alias.  memsize in Mbits */
static int memc_alias_test(uint32_t memsize)
{
    volatile uint32_t *base_addr;
    volatile uint32_t *test_addr;
    uint64_t test_size, total_size;
#ifdef CONFIG_BRCM_IKOS
    uint32_t data;
#endif
    int ret = 0;

    total_size = ((uint64_t)(memsize))<<17;
    base_addr = (volatile uint32_t*)((uintptr_t)DRAM_BASE_NOCACHE);

    for (test_size = 256; test_size < total_size ; test_size = test_size << 1) {
        test_addr = (volatile uint32_t*)((uintptr_t)base_addr + (uintptr_t)test_size);
#if defined(DRAM_BASE_2)
        /* if we are over the lower memory region from 0 to 2GB, we shift to the upper memory region */
        if( test_size >= 0x80000000 )
            test_addr = (volatile uint32_t*)((uintptr_t)test_addr + 0x80000000);
#endif
#ifdef CONFIG_BRCM_IKOS
        data = *test_addr;
#endif
        *base_addr = 0;
        *test_addr = 0xaa55beef;
        if (*base_addr == *test_addr) {
            ret = 1;
            break;
        }
#ifdef CONFIG_BRCM_IKOS
        *test_addr = data;
#endif
    }

    return ret;
}

/* return 1 if there is error, 0 no error.  memsize in Mbits */
static int memc_memory_test(uint32_t memsize)
{
    volatile uint32_t* temp;
    volatile uint32_t* addr;
    int i, ret = 0;
    uint32_t data;

    addr  = (volatile uint32_t*)DRAM_BASE_NOCACHE;
    for( temp = addr, i = 0; i < 1024; i++ )
       *temp++ = i;

    for( temp = addr, i = 0; i < 1024; i++ )
    {
        data = *temp++;
        if( data != i )
            break;
    }

    if( i == 1024 )  //pass
        printf("DDR test done successfully\n\r");
    else
    {
        xprintf("DDR test failed i=%d\n\r", i);
        ret = -2;
    }

    return ret;
}

static mcbindex* memc_get_mcb(uint32_t memcfg)
{
    mcbindex* pMCB = &MCB[0], *pRet = NULL;

#if defined(_BCM96858_)
    unsigned int is_32b_ddr = ((MISC->miscStrapBus & MISC_STRAP_DDR_16B_EN_MASK) >> MISC_STRAP_DDR_16B_EN_SHIFT) ? 0 : 1;
    unsigned int ddr_density = ((MISC->miscStrapBus & MISC_STRAP_DDR_DENSITY_MASK) >> MISC_STRAP_DDR_DENSITY_SHIFT);   // 1-8Gb, 2-4Gb, 3-2Gb
    unsigned int ddr_override = ((MISC->miscStrapBus & MISC_STRAP_DDR_OVERRIDE_N_MASK) >> MISC_STRAP_DDR_OVERRIDE_N_SHIFT) ? 0: 1;
    if (!ddr_override)
        pRet = &MCB[ddr_density + 3*is_32b_ddr];
    else
#endif
    {
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
                if( pRet == NULL )
                    pRet = pMCB;

                if( (pMCB->config&BP_DDR_CONFIG_MASK) == (memcfg&BP_DDR_CONFIG_MASK) )
                {
                    pRet = pMCB;
                    break;
                }
            }
            pMCB++;
        }
    }
    return pRet;
}
#ifdef MEMC_DEBUG
static void memc_print_cfg(void) {
    print_log("---------------------------------------------------------------\n");
    print_log("MEMC DRAM profile (memc_dram_profile_struct) values:\n");
    print_log("====================================================\n");

    print_log("PART values:\n");
    print_log("  part_speed_grade    = %d \n", memc_dram_profile.speed_grade);
    print_log("  part_size_Mbits     = %d (DRAM size in MegaBits)\n", memc_dram_profile.size_Mbits);
    print_log("  part_row_bits       = %d (number of row bits)\n", memc_dram_profile.row_bits);
    print_log("  part_col_bits       = %d (number of column bits)\n", memc_dram_profile.col_bits);
    print_log("  part_ba_bits        = %d (number of bank bits)\n",memc_dram_profile.ba_bits);
    print_log("  part_width_bits     = %d (DRAM width in bits)\n", memc_dram_profile.width_bits);


    print_log("NUMER OF PARTS:\n");
    print_log("  part_num            = %d (Number of parts)\n", memc_dram_profile.num_part);

    print_log("TOTAL values:\n");
    print_log("  total_size_Mbits    = %d (DRAM size in MegaBits)\n", memc_dram_profile.total_size_Mbits);
    print_log("  total_cs_bits       = %d (number of cs bits, for dual_rank mode)\n", memc_dram_profile.total_cs_bits);
    print_log("  total_width_bits    = %d (DRAM width in bits)\n", memc_dram_profile.total_width_bits);
    print_log("  total_burst_bytes   = %d (Number of bytes per DRAM access)\n", memc_dram_profile.total_burst_bytes);
    print_log("  total_max_byte_addr = 0x%x (Maximum/last DRAM byte address)\n", memc_dram_profile.total_max_byte_addr);
    print_log("                        (Number of bits in total_max_byte_addr is %d)\n", memc_dram_profile.total_max_byte_addr_bits);
    print_log("                        (i.e. total_max_byte_addr goes from bit 0 to bit %d)\n", (memc_dram_profile.total_max_byte_addr_bits - 1));

    print_log("  ddr_2T_mode         = %d\n", memc_dram_profile.ddr_2T_mode);
    print_log("  ddr_hdp_mode        = %d\n", memc_dram_profile.ddr_hdp_mode);
    print_log("  large_page          = %d\n", memc_dram_profile.large_page);
    print_log("  ddr_dual_rank       = %d\n", memc_dram_profile.ddr_dual_rank);
    print_log("  cs_mode             = %d\n", memc_dram_profile.cs_mode);

    print_log("MEMC timing (memc_dram_timing_cfg_struct) values:\n");
    print_log("====================================================\n");
    print_log("  MC_CHN_TIM_TIM1_0 register fields:\n");
    print_log("    tCwl   = %d\n", memc_dram_profile.timing_cfg.tCwl);
    print_log("    tRP    = %d\n", memc_dram_profile.timing_cfg.tRP);
    print_log("    tCL    = %d\n", memc_dram_profile.timing_cfg.tCL);
    print_log("    tRCD   = %d\n", memc_dram_profile.timing_cfg.tRCD);
    print_log("  MC_CHN_TIM_TIM1_1 register fields:\n");
    print_log("    tCCD_L = %d\n", memc_dram_profile.timing_cfg.tCCD_L);
    print_log("    tCCD   = %d\n", memc_dram_profile.timing_cfg.tCCD);
    print_log("    tRRD_L = %d\n", memc_dram_profile.timing_cfg.tRRD_L);
    print_log("    tRRD   = %d\n", memc_dram_profile.timing_cfg.tRRD);
    print_log("  MC_CHN_TIM_TIM1_2 register fields:\n");
    print_log("    tFAW   = %d\n", memc_dram_profile.timing_cfg.tFAW);
    print_log("    tRTP   = %d\n", memc_dram_profile.timing_cfg.tRTP);
    print_log("    tRCr   = %d\n", memc_dram_profile.timing_cfg.tRCr);
    print_log("  MC_CHN_TIM_TIM1_3 register fields:\n");
    print_log("    tWTR_L = %d\n", memc_dram_profile.timing_cfg.tWTR_L);
    print_log("    tWTR   = %d\n", memc_dram_profile.timing_cfg.tWTR);
    print_log("    tWR_L  = %d\n", memc_dram_profile.timing_cfg.tWR_L);
    print_log("    tWR    = %d\n", memc_dram_profile.timing_cfg.tWR);
    print_log("  MC_CHN_TIM_TIM2 register fields:\n");
    print_log("    tR2R   = %d\n", memc_dram_profile.timing_cfg.tR2R);
    print_log("    tR2W   = %d\n", memc_dram_profile.timing_cfg.tR2W);
    print_log("    tW2R   = %d\n", memc_dram_profile.timing_cfg.tW2R);
    print_log("    tW2W   = %d\n", memc_dram_profile.timing_cfg.tW2W);
    print_log("    tAL    = %d\n", memc_dram_profile.timing_cfg.tAL);
    print_log("    tRFC   = %d\n", memc_dram_profile.timing_cfg.tRFC);
}
#else
static void memc_print_cfg(void)
{
    int sg, ssc;
    
    sg = memc_dram_profile.speed_grade;
    ssc = memc_dram_profile.ssc;
    if( sg >= sizeof(str_speed) )
        sg = sizeof(str_speed) - 1;
    if( ssc >= sizeof(str_ssc) )
        ssc = sizeof(str_ssc) - 1;

    printf("DDR3-%s total %dMB %d %dbits part[s]",
        str_speed[sg], memc_dram_profile.total_size_Mbits/8, 
        memc_dram_profile.num_part, memc_dram_profile.width_bits);

    if( ssc )
        printf(" %s SSC", str_ssc[ssc]);

    if( memc_dram_profile.high_temp )
      printf(" High Temperature %s", memc_dram_profile.high_temp == (BP_DDR_TEMP_EXTENDED_SRT>>BP_DDR_TEMP_SHIFT) ? "SRT" : "ASR" );

    printf("\n\n");

    return;
}
#endif

#if defined(_BCM94908_) || defined(_BCM963158_) || defined(_BCM96858_)
static int memc_init_rng_seed(uint32_t* seed)
{
    int timeout = 0, i = 0;
    uint32_t random[16];
    uint32_t digest[8];

    /* make sure RNG is ready for random number. In 4908 platform it is on automaticall by hardware */
    while( (RNG->intStatus&RNG_INT_STATUS_NIST_FAIL) == 0x0 
            && (RNG->intStatus&RNG_INT_STATUS_FIFO_FULL) == 0x0 ) {
        cfe_usleep(1);
        timeout++;
        if( timeout > 500000 ) {
            printf("memc_setup_scrambler RNG time out\n");
            return -1;
        }
    }

    /* fetch 16 words from RNG and run them through the SHA256 to get zeros and ones balanced RN */
    i = 0;
    while( i < 16 ) {
        if( RNG->fifoCnt & 0xff ) {
            random[i] = RNG->rngFifoData;
            i++;
        }
    } 
    Sec_sha256((uint8_t const*)random, 512, (uint32_t*)digest);

    for( i = 0; i < 4; i++ ) {
        seed[i] = digest[i];
    }

    return 0;
}

static int memc_waitfor_scrambler_status(uint32_t status)
{
    int timeout = 0;

    while( (MEMC_SCRAM_CTRL->key_status&status) == 0 ) {
        cfe_usleep(1);
        timeout++;
        if( timeout > 500000 ) {
            printf("memc_waitfor_scrambler_status 0x%x time out\n", status);
            return -1;
        }
    }
    
    return 0;
}

static int memc_setup_scrambler(uint32_t memsize_Mbit)
{
    int rc = 0, i;
    uint32_t seed[4];
    uint64_t total_size = ((uint64_t)(memsize_Mbit))<<17;
    uint64_t start_address_inclusive = 0 + MEMSTRT_UNSCRAM_WINDOW_SIZE;
    uint64_t end_address_inclusive = total_size - MEMEND_UNSCRAM_WINDOW_SIZE - 1;

    if( (rc = memc_init_rng_seed(seed)) != 0 )
        return rc;

    for( i = 0; i < 4; i++ ) {
        MEMC_SCRAM_CTRL->seed[i] = seed[i];
    }

    for( i = 0; i < 4; i++ ) {
        MEMC_SCRAM_CTRL->range[i].start_addr_low   = (start_address_inclusive >> SCRAMBLER_ADDR_LOW_SHIFT) & SCRAMBLER_ADDR_LOW_MASK; 
        MEMC_SCRAM_CTRL->range[i].start_addr_upper = (start_address_inclusive >> SCRAMBLER_ADDR_UPP_SHIFT) & SCRAMBLER_ADDR_UPP_MASK;
        MEMC_SCRAM_CTRL->range[i].end_addr_low     = (  end_address_inclusive >> SCRAMBLER_ADDR_LOW_SHIFT) & SCRAMBLER_ADDR_LOW_MASK;
        MEMC_SCRAM_CTRL->range[i].end_addr_upper   = (  end_address_inclusive >> SCRAMBLER_ADDR_UPP_SHIFT) & SCRAMBLER_ADDR_UPP_MASK;
    }

    MEMC_SCRAM_CTRL->manual_keys_trigger = 1;
    if ( (rc = memc_waitfor_scrambler_status(SCRAMBLER_KEY_STATUS_GENERATED)) != 0 )
        return rc;
  
    MEMC_SCRAM_CTRL->secure_mode_ctrl = SCRAMBLER_ENABLE;
    if ( (rc = memc_waitfor_scrambler_status(SCRAMBLER_KEY_STATUS_ENABLED)) != 0 )
        return rc;

    /* disable scrambler access */
#if defined(_BCM94908_)
    MEMC_ACC_CTRL->SCRAM_ACCCTL = 0x10;
    MEMC_ACC_CTRL->SCRAM_PERMCTL = 0x0;
#else
    MEMC_ACC_CTRL->SCRAM_PERMCTL = 0x80000000;
#endif
    return rc;
}
#endif

static void mc_cfg_init(unsigned int* pMCB)
{
    int i;
    print_log("mc_cfg_init(): Initialize the default values on mc_cfg\n");

    memset(&mc_cfg, 0x0, sizeof(mc_cfg));
    mc_cfg.arb_burst_mode = 1;
    mc_cfg.arb_rr_mode = 1;
    for (i=0; i < MC_ARB_SP_Q_NUM; i++) {
        mc_cfg.arb_sp_sel[i] = 0;
        mc_cfg.arb_sp_pri[i] = i;
    }
    mc_cfg.arb_sp_sel[MC_ARB_SP_Q_NUM-1] = 1; // RR winner is always the last SP Q
    for (i=0; i < MC_ARB_RR_Q_NUM; i++)
    mc_cfg.arb_rr_qqw[i] = 1;

    mc_cfg.gcfg_max_age       = (pMCB[MCB_EXT_CFG_WORD] & MCB_EXT_CFG_MAX_AGE_MASK) >> MCB_EXT_CFG_MAX_AGE_SHIFT;
    mc_cfg.gcfg_perf_tweaks   = (pMCB[MCB_EXT_CFG_WORD] & MCB_EXT_CFG_PERF_MASK) >> MCB_EXT_CFG_PERF_SHIFT;

    mc_cfg.gcfg_seq_mode = 0;
    mc_cfg.chn_arb_ab4_disable = 0; // New default, according to Herman
    mc_cfg.chn_arb_ab4_burst = 1; // New default, according to Herman
    mc_cfg.chn_arb_ab4_rdwr_weight[0] = 2;
    mc_cfg.chn_arb_ab4_rdwr_weight[1] = 2;

    mc_cfg.chn_arb_ab4_act_pri        = 0xd9c8fbea; // rdb default
    mc_cfg.chn_arb_ab4_cmd_pri_opened = 0xd9c8fbea; // rdb default
    mc_cfg.chn_arb_ab4_cmd_pri_closed = 0x51417362; // rdb default

    mc_cfg.chn_arb_pkt_mode = 0; // Default

    for (i=0; i < MC_UB_SRC_ID_NUM; i++)
        mc_cfg.ub0_srcid_q[i] = 0;
    mc_cfg.ub0_oob_err_mask   = 0;
    mc_cfg.ub0_ib_err_mask    = 0;
    mc_cfg.ub0_wrep_mode      = 0;
    mc_cfg.ub0_fifo_mode      = 0;
    mc_cfg.ub0_wburst_mode    = 0;

    for (i=0; i < MC_UB_SRC_ID_NUM; i++)
        mc_cfg.ub1_srcid_q[i] = 0;
    mc_cfg.ub1_oob_err_mask   = 0;
    mc_cfg.ub1_ib_err_mask    = 0;
    mc_cfg.ub1_wrep_mode      = 0;
    mc_cfg.ub1_fifo_mode      = 0;
    mc_cfg.ub1_wburst_mode    = 0;

    mc_cfg.mcp_wr_com_thd     = 0xF;
    mc_cfg.mcp_wr_req_thd     = 0;

    return;
}

static void init_memc_dram_profile(unsigned int* pMCB)
{
    uint32_t temp;

    print_log("init_memc_dram_profile(): Initializing MEMC DRAM profile\n\r");

    memset(&memc_dram_profile, 0x0, sizeof(memc_dram_profile));

    // Setup profile values
    temp = (memcfg&BP_DDR_SPEED_MASK)>>BP_DDR_SPEED_SHIFT;
    if(temp == BP_DDR_SPEED_SAFE )
        memc_dram_profile.speed_grade = DDR_CLK_533MHZ_CL8;
    else
        memc_dram_profile.speed_grade = (temp - BP_DDR_SPEED_400_6_6_6) + DDR_CLK_400MHZ_CL6;

    memc_dram_profile.total_size_Mbits =
        1 << (8+((memcfg&BP_DDR_TOTAL_SIZE_MASK)>>BP_DDR_TOTAL_SIZE_SHIFT));

    memc_dram_profile.total_width_bits =
        16 << ((memcfg&BP_DDR_TOTAL_WIDTH_MASK)>>BP_DDR_TOTAL_WIDTH_SHIFT);
    memc_dram_profile.width_bits =
        8 << ((memcfg&BP_DDR_DEVICE_WIDTH_MASK)>>BP_DDR_DEVICE_WIDTH_SHIFT);
    memc_dram_profile.total_burst_bytes = memc_dram_profile.total_width_bits;

    memc_dram_profile.num_part = memc_dram_profile.total_width_bits/memc_dram_profile.width_bits;
    memc_dram_profile.size_Mbits = memc_dram_profile.total_size_Mbits/memc_dram_profile.num_part;

    memc_dram_profile.row_bits   = ((pMCB[MCB_DENSITY_WORD] & MCB_DENSITY_ROWS_MASK) >> MCB_DENSITY_ROWS_SHIFT) + 12;
    memc_dram_profile.col_bits   = ((pMCB[MCB_DENSITY_WORD] & MCB_DENSITY_COLUMNS_MASK) >> MCB_DENSITY_COLUMNS_SHIFT) + 9;
    memc_dram_profile.ba_bits    = ((pMCB[MCB_DENSITY_WORD] & MCB_DENSITY_BANKS_MASK) >> MCB_DENSITY_BANKS_SHIFT) ? 3 : 2;

    temp  = memc_dram_profile.total_size_Mbits;
    memc_dram_profile.total_max_byte_addr_bits = (uint32_t)(-1);
    while(temp)
    {
        temp = temp >> 1;
        memc_dram_profile.total_max_byte_addr_bits++;
    }
    memc_dram_profile.total_max_byte_addr_bits += 17;
    memc_dram_profile.total_max_byte_addr = (1 << memc_dram_profile.total_max_byte_addr_bits) - 1;
    memc_dram_profile.high_temp = (memcfg&BP_DDR_TEMP_MASK) >> BP_DDR_TEMP_SHIFT;
    memc_dram_profile.ssc = (memcfg&BP_DDR_SSC_CONFIG_MASK) >> BP_DDR_SSC_CONFIG_SHIFT;

    memc_dram_profile.ref_clk = pMCB[MCB_PHY_REF_CLK_WORD];
    memc_dram_profile.dram_clk = pMCB[MCB_DRAM_DATA_CLK_WORD];

    memc_dram_profile.ddr_2T_mode  = (pMCB[MCB_2T_MODE_WORD] & MCB_2T_MODE_MASK) >> MCB_2T_MODE_SHIFT;
#if defined(_BCM96846_)
    memc_dram_profile.ddr_hdp_mode = 0;
#else
    memc_dram_profile.ddr_hdp_mode = (memc_dram_profile.total_width_bits == 16);
#endif
    memc_dram_profile.large_page   = 1;
    memc_dram_profile.ddr_dual_rank = 0;

    memc_dram_profile.timing_cfg.tRAS =  (pMCB[MCB_TIMING0_WORD] & MCB_TIMING0_tRAS_MASK) >> MCB_TIMING0_tRAS_SHIFT;
    memc_dram_profile.timing_cfg.tCwl =  (pMCB[MCB_TIMING1_WORD] & MCB_TIMING1_tCWL_MASK) >> MCB_TIMING1_tCWL_SHIFT;
    memc_dram_profile.timing_cfg.tRP  =  (pMCB[MCB_TIMING0_WORD] & MCB_TIMING0_tRP_MASK) >> MCB_TIMING0_tRP_SHIFT;
    memc_dram_profile.timing_cfg.tCL  =  (pMCB[MCB_TIMING1_WORD] & MCB_TIMING1_tCL_MASK) >> MCB_TIMING1_tCL_SHIFT;
    memc_dram_profile.timing_cfg.tRCD =  (pMCB[MCB_TIMING0_WORD] & MCB_TIMING0_tRCD_MASK) >> MCB_TIMING0_tRCD_SHIFT;

    memc_dram_profile.timing_cfg.tCCD =  4;
    memc_dram_profile.timing_cfg.tCCD_L =   memc_dram_profile.timing_cfg.tCCD;
    memc_dram_profile.timing_cfg.tRRD =  (pMCB[MCB_TIMING0_WORD] & MCB_TIMING0_tRRD_MASK) >> MCB_TIMING0_tRRD_SHIFT;
    memc_dram_profile.timing_cfg.tRRD_L = memc_dram_profile.timing_cfg.tRRD;

    memc_dram_profile.timing_cfg.tFAW =  (pMCB[MCB_TIMING3_WORD] & MCB_TIMING3_tFAW_MASK) >> MCB_TIMING3_tFAW_SHIFT;
    memc_dram_profile.timing_cfg.tRTP =  (pMCB[MCB_TIMING1_WORD] & MCB_TIMING1_tRTP_MASK) >> MCB_TIMING1_tRTP_SHIFT;
    memc_dram_profile.timing_cfg.tRCr =  memc_dram_profile.timing_cfg.tRP + memc_dram_profile.timing_cfg.tRAS;

    memc_dram_profile.timing_cfg.tWTR =  (pMCB[MCB_TIMING2_WORD] & MCB_TIMING2_tWTR_MASK) >> MCB_TIMING2_tWTR_SHIFT;
    memc_dram_profile.timing_cfg.tWTR_L = memc_dram_profile.timing_cfg.tWTR;
    memc_dram_profile.timing_cfg.tWR  =  (pMCB[MCB_TIMING1_WORD] & MCB_TIMING1_tWR_MASK) >> MCB_TIMING1_tWR_SHIFT;
    memc_dram_profile.timing_cfg.tWR_L = memc_dram_profile.timing_cfg.tWR;

    memc_dram_profile.timing_cfg.tR2R =  (pMCB[MCB_EXT_CFG_WORD] & MCB_EXT_CFG_tR2R_MASK) >> MCB_EXT_CFG_tR2R_SHIFT;
#if defined(_BCM94908_)
    memc_dram_profile.timing_cfg.tR2W =  (pMCB[MCB_EXT_CFG_WORD] & MCB_EXT_CFG_tR2W_MASK) >> MCB_EXT_CFG_tR2W_SHIFT;
#else
    /* tR2W depends on the trace length. For new MEMC controller, we start with a conservative value and will use shmoo
       to figure the optimum value and update memc setting */
    memc_dram_profile.timing_cfg.tR2W =  ((pMCB[MCB_EXT_CFG_WORD] & MCB_EXT_CFG_tR2W_MASK) >> MCB_EXT_CFG_tR2W_SHIFT) + 1;
#endif
    memc_dram_profile.timing_cfg.tW2R =  (pMCB[MCB_EXT_CFG_WORD] & MCB_EXT_CFG_tW2R_MASK) >> MCB_EXT_CFG_tW2R_SHIFT;
    memc_dram_profile.timing_cfg.tW2W =  (pMCB[MCB_EXT_CFG_WORD] & MCB_EXT_CFG_tW2W_MASK) >> MCB_EXT_CFG_tW2W_SHIFT;
#if defined(_BCM96846_)
     memc_dram_profile.timing_cfg.tAL = 0;
#else
    memc_dram_profile.timing_cfg.tAL  =  (pMCB[MCB_TIMING2_WORD] & MCB_TIMING2_tAL_MASK) >> MCB_TIMING2_tAL_SHIFT;
#endif
    memc_dram_profile.timing_cfg.tRFC =  (pMCB[MCB_TIMING2_WORD] & MCB_TIMING2_tRFC_MASK) >> MCB_TIMING2_tRFC_SHIFT;

    if (memc_dram_profile.ddr_dual_rank)
    {
      memc_dram_profile.total_cs_bits = 1;
      memc_dram_profile.cs_mode = 2;
      memc_dram_profile.cs_addr_bits_map = memc_dram_profile.total_max_byte_addr_bits - 1;
    }

    memc_print_cfg();

    return;
}

// This function is used to program the MEMC timing registers.
// It's almost fully based on the prog_timing_cntrl legacy function.
static void cfg_memc_timing_ctrl(void)
{
    print_log("cfg_memc_timing_ctrl() Called\n");

    // Program the MEMC timing based on the timing struct
    uint32_t tim1_0 = 0;
    uint32_t tim1_1 = 0;
    uint32_t tim1_2 = 0;
    uint32_t tim1_3 = 0;
    uint32_t tim2   = 0;
    uint32_t mcregval;

    mcregval = memc_dram_profile.timing_cfg.tAL;

    /* adjust for DDR3, assume DDR3 for now */
    mcregval = 0;
    if (memc_dram_profile.timing_cfg.tAL > 0) {
        mcregval = memc_dram_profile.timing_cfg.tCL - memc_dram_profile.timing_cfg.tAL;
        print_log("cfg_memc_timing_ctrl() : Adjusted tAL cfg = %d (original: tAL=%d; tCL=%d)\n", 
                mcregval, memc_dram_profile.timing_cfg.tAL, memc_dram_profile.timing_cfg.tCL);
        memc_dram_profile.timing_cfg.tAL = mcregval;
    }

    tim1_0 = (
                ((memc_dram_profile.timing_cfg.tCwl << MC_CHN_TIM_TIM1_0_TIM1_tWL_SHIFT)  & MC_CHN_TIM_TIM1_0_TIM1_tWL_MASK) | 
                ((memc_dram_profile.timing_cfg.tRP  << MC_CHN_TIM_TIM1_0_TIM1_tRP_SHIFT)  & MC_CHN_TIM_TIM1_0_TIM1_tRP_MASK) |
                ((memc_dram_profile.timing_cfg.tCL  << MC_CHN_TIM_TIM1_0_TIM1_tCL_SHIFT)  & MC_CHN_TIM_TIM1_0_TIM1_tCL_MASK) | 
                ((memc_dram_profile.timing_cfg.tRCD << MC_CHN_TIM_TIM1_0_TIM1_tRCD_SHIFT) & MC_CHN_TIM_TIM1_0_TIM1_tRCD_MASK) 
            );

    tim1_1 = (
                ((memc_dram_profile.timing_cfg.tCCD_L << MC_CHN_TIM_TIM1_1_TIM1_tCCD_L_SHIFT) & MC_CHN_TIM_TIM1_1_TIM1_tCCD_L_MASK) | 
                ((memc_dram_profile.timing_cfg.tCCD   << MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_SHIFT) & MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_MASK) | 
                ((memc_dram_profile.timing_cfg.tRRD_L << MC_CHN_TIM_TIM1_1_TIM1_tRRD_L_SHIFT) & MC_CHN_TIM_TIM1_1_TIM1_tRRD_L_MASK) |
                ((memc_dram_profile.timing_cfg.tRRD   << MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_SHIFT) & MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_MASK) 
            );

    tim1_2 = (
                ((memc_dram_profile.timing_cfg.tFAW << MC_CHN_TIM_TIM1_2_TIM1_tFAW_SHIFT) & MC_CHN_TIM_TIM1_2_TIM1_tFAW_MASK) | 
                ((memc_dram_profile.timing_cfg.tRTP << MC_CHN_TIM_TIM1_2_TIM1_tRTP_SHIFT) & MC_CHN_TIM_TIM1_2_TIM1_tRTP_MASK) | 
                ((memc_dram_profile.timing_cfg.tRCr << MC_CHN_TIM_TIM1_2_TIM1_tRC_SHIFT)  & MC_CHN_TIM_TIM1_2_TIM1_tRC_MASK)  
            );

    tim1_3 = (
                ((memc_dram_profile.timing_cfg.tWTR_L << MC_CHN_TIM_TIM1_3_TIM1_tWTR_L_SHIFT) & MC_CHN_TIM_TIM1_3_TIM1_tWTR_L_MASK) |
                ((memc_dram_profile.timing_cfg.tWTR   << MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_SHIFT) & MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_MASK) |
                ((memc_dram_profile.timing_cfg.tWR_L  << MC_CHN_TIM_TIM1_3_TIM1_tWR_L_SHIFT)  & MC_CHN_TIM_TIM1_3_TIM1_tWR_L_MASK)  | 
                ((memc_dram_profile.timing_cfg.tWR    << MC_CHN_TIM_TIM1_3_TIM1_tWR_S_SHIFT)  & MC_CHN_TIM_TIM1_3_TIM1_tWR_S_MASK)  
            );

    tim2   = (
                ((memc_dram_profile.timing_cfg.tR2R << MC_CHN_TIM_TIM2_TIM2_tR2R_SHIFT) & MC_CHN_TIM_TIM2_TIM2_tR2R_MASK) | 
                ((memc_dram_profile.timing_cfg.tR2W << MC_CHN_TIM_TIM2_TIM2_tR2W_SHIFT) & MC_CHN_TIM_TIM2_TIM2_tR2W_MASK) |
                ((memc_dram_profile.timing_cfg.tW2R << MC_CHN_TIM_TIM2_TIM2_tW2R_SHIFT) & MC_CHN_TIM_TIM2_TIM2_tW2R_MASK) |
                ((memc_dram_profile.timing_cfg.tW2W << MC_CHN_TIM_TIM2_TIM2_tW2W_SHIFT) & MC_CHN_TIM_TIM2_TIM2_tW2W_MASK) |
                ((memc_dram_profile.timing_cfg.tAL  << MC_CHN_TIM_TIM2_TIM2_tAL_SHIFT)  & MC_CHN_TIM_TIM2_TIM2_tAL_MASK)  | 
                ((memc_dram_profile.timing_cfg.tRFC << MC_CHN_TIM_TIM2_TIM2_tRFC_SHIFT) & MC_CHN_TIM_TIM2_TIM2_tRFC_MASK) 
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

    mi = 0;
    if ( memc_dram_profile.total_width_bits == 16 )
        mi = (memc_dram_profile.large_page==1) ? 1 : 0;
    else if ( memc_dram_profile.total_width_bits == 32 )
        mi = (memc_dram_profile.large_page==1) ? 3 : 2;
    else
    {
        printf("cfg_map_addr_bits_to_row_col_ba() : total_width_bits=%d not supported!\n\r",
            memc_dram_profile.total_width_bits);
        return;
    }
    /* for large ddr part that use 12 column bits(11 actually as bit 10 is not column bit) */
    if ( memc_dram_profile.col_bits == 12 )
        mi += 4;

    for ( i=0; i<9; i++)   // i is to iterate over registers
    {
        reg_val = 0;
        reg_update_flag = 0;

        if      (i == 0) { mii = 0; miii = 0; max_bits = memc_dram_profile.row_bits; } // Row
        else if (i == 4) { mii = 1; miii = 0; max_bits = memc_dram_profile.col_bits; } // Column
        else if (i == 8) { mii = 2; miii = 0; max_bits = memc_dram_profile.ba_bits;  } // Bank

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

    reg_val = (( memc_dram_profile.cs_mode << MEMC_CHN_CFG_CNFG_CS_MODE_SHIFT) & MEMC_CHN_CFG_CNFG_CS_MODE_MASK);
    print_log(" Writing to MC_CHN_CFG_CNFG reg; data=0x%08x\n", reg_val);
    print_log("[0x%08x] = 0x%08x\n\r", &MEMC->CHN_CFG_CNFG, reg_val);
    MEMC->CHN_CFG_CNFG = reg_val;

    if (memc_dram_profile.ddr_dual_rank)
    {
        reg_val = memc_dram_profile.cs_addr_bits_map;
        print_log("DDR_DUAL_RANK cfg: Writing to MC_CHN_CFG_CS_0 reg; data=0x%08x\n", reg_val);
        print_log("[0x%08x] = 0x%08x\n\r", &MEMC->CHN_CFG_CSST, reg_val);
        MEMC->CHN_CFG_CSST = reg_val;
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
        memsys_udelay(1);
        data = MEMC->CHN_TIM_PHY_ST;
        print_log("PHY Status= %x\n\r", data);
    }

    print_log("Disable Auto-Refresh\n\r");
    refresh_ctrl_cfg(memc_dram_profile.dram_clk, memc_dram_profile.high_temp, 1);

    // This was added because after the DDR PHY init sequence (i.e. ddr34_phy_init()) finishes, CKE is disabled.
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->CHN_TIM_DCMD), 0x00000305);
    MEMC->CHN_TIM_DCMD = 0x00000305; // Enable CKE

    print_log("End of memsys_begin\n\r");

    return MEMSYS_ERROR_MEMC_NONE;
}

#if defined(_BCM94908_)
uint32_t memc_init(memsys_top_params_t *params, uint32_t options)
#elif defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_)
static int memc_init(memsys_top_params_t *params, void *args)
#else 
#error CHECK MEMC API
#endif
{
    uint32_t data;

    print_log("start of memc_init\n\r");

    /* set 4x mode */
    data = MEMC->GLB_GCFG;
    data &= ~MEMC_GLB_GCFG_PHY_DFI_MODE_MASK;
    data |= (MEMC_GLB_GCFG_PHY_DFI_4X<<MEMC_GLB_GCFG_PHY_DFI_MODE_SHIFT);

    // use pll clock
    data |= MEMC_GLB_GCFG_MCLKSRC_MASK;

    /* Enable DFI */
    data |= MEMC_GLB_GCFG_DFI_EN_MASK;

    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->GLB_GCFG), data);
    MEMC->GLB_GCFG = data;

#if defined(_BCM96856_)
    // set AXI fast ack mode
    data = MEMC->AXIWIF.CFG;
    data |= MEMC_AXI_FAST_ACK_MODE_MASK;
    MEMC->AXIWIF.CFG = data;
#endif

    /* Set DDR3 mode */
    data = MEMC->CHN_TIM_DRAM_CFG;
    data |= (data & ~MEMC_CHN_TIM_DRAM_CFG_DRAMTYPE_MASK) | MC_DRAM_CFG_DDR3;
    data = (data & ~MEMC_CHN_TIM_DRAM_CFG_2TADDRCMD_MASK) | ((memc_dram_profile.ddr_2T_mode & 0x1) << MEMC_CHN_TIM_DRAM_CFG_2TADDRCMD_SHIFT);
    data = (data & ~MEMC_CHN_TIM_DRAM_CFG_HDP_MASK) | ((memc_dram_profile.ddr_hdp_mode & 0x1) << MEMC_CHN_TIM_DRAM_CFG_HDP_SHIFT);
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->CHN_TIM_DRAM_CFG), data);
    MEMC->CHN_TIM_DRAM_CFG = data;

    print_log("Enable Auto-Refresh\n\r");
    refresh_ctrl_cfg(memc_dram_profile.dram_clk, memc_dram_profile.high_temp, 0);

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
#if defined(_BCM963158_) || defined(_BCM96856_)
    uint32_t tR2W = 0, dly_cyc = 0, tim2, num_lane;
#endif

    print_log("start of memsys_end\n\r");

    data = MEMC->GLB_GCFG;
    data &= ~MEMC_GLB_GCFG_FORCE_SEQ_MASK;
    data &= ~MEMC_GLB_GCFG_RBF_REORDER_MASK;
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->GLB_GCFG), data);
    MEMC->GLB_GCFG = data;

    /* don't know the exact memory size in safe mode, limit to the minimum size */
    if( is_safemode )
        memc_dram_profile.total_size_Mbits = 1024;

    /* set DDR size */
    size = memc_dram_profile.total_size_Mbits >> 3;
    i = 0;
    while(size)
    {
        size = (size >> 1);
        if (size)
            i++;
    }
#if defined(_BCM96858_) || defined(_BCM94908_)
    MEMC->GLB_GCFG &= ~MEMC_GLB_GCFG_SIZE1_MASK;
    MEMC->GLB_GCFG |= (i<<MEMC_GLB_GCFG_SIZE1_SHIFT);  /* cfg_size1 = log2(memsize in MB) */
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->GLB_GCFG), MEMC->GLB_GCFG);
#else
    MEMC->GLB_FSBL_STATE &= ~MEMC_GLB_FSBL_DRAM_SIZE_MASK;
    MEMC->GLB_FSBL_STATE |= (i<<MEMC_GLB_FSBL_DRAM_SIZE_SHIFT);
    print_log("[0x%08x] = 0x%08x\n\r", &(MEMC->GLB_FSBL_STATE), MEMC->GLB_FSBL_STATE);
#endif

    /* check tR2W from shmoo result and update if necessary */
#if defined(_BCM963158_) || defined(_BCM96856_)
    num_lane = memc_dram_profile.total_width_bits >> 3;
    for( i = 0; i < num_lane; i++ )
    {
        dly_cyc = (MEMC->PhyByteLaneControl[i].RD_EN_DLY_CYC&RD_EN_DLY_CYC_CS0_CYCLES_MASK)>>RD_EN_DLY_CYC_CS0_CYCLES_SHIFT;
        if( dly_cyc > tR2W )
            tR2W = dly_cyc;
    }

    /* tR2W add 1 for margin but MEMC want to minus 1 as it internally add 1 back. So no change */
    if( tR2W > 0 && memc_dram_profile.timing_cfg.tR2W != tR2W ) 
    { 
        printf("update tR2W based on shmoo result from %d to %d\n", memc_dram_profile.timing_cfg.tR2W, tR2W);  
        memc_dram_profile.timing_cfg.tR2W = tR2W;
        tim2 = MEMC->CHN_TIM_TIM2;
        tim2 = (tim2&~MC_CHN_TIM_TIM2_TIM2_tR2W_MASK) | ((memc_dram_profile.timing_cfg.tR2W << MC_CHN_TIM_TIM2_TIM2_tR2W_SHIFT) & MC_CHN_TIM_TIM2_TIM2_tR2W_MASK);
        MEMC->CHN_TIM_TIM2   = tim2;
    }

#endif
    print_log("end of memsys_end\n\r");

    return MEMSYS_ERROR_MEMC_NONE;
}

#if defined(_BCM94908_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96858_) || defined(_BCM96856_)
#ifdef CFE_ROM_STOP
static mcbindex * select_default_mcb(mcbindex *pMCB)
{
    //spare register reset value is 0
    if(GPIO->spare[2] != 0)
    {
        pMCB = memc_get_mcb(GPIO->spare[2]);
        printf("selecting mcb %x\n", pMCB->config);
    }
    return pMCB;
}
#endif
#endif

//*****************************************
// This is the main initialization function
//*****************************************
int ddr_init(uint32_t mcb_selector, uint32_t memcfg_from_flash)
{
    mcbindex* pMCB = NULL;
    uint32_t ret;
    memsys_top_params_t params;
    memsys_system_callbacks_t callbacks = {
        memsys_udelay,
        memsys_putchar,
        0,
#if defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_)
        memsys_get_time_us,
#elif defined(_BCM94908_)
        0,
#endif
    };
    uint32_t options = 0;//MEMSYS_OPTION_SKIP_SHMOO;

#if defined(_BCM94908_) || defined(_BCM963158_) || defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM96856_)
#if !defined(_BCM96858_) 
    if (ROM_ARG_ISSET(ROM_ARG_DRAM_INIT_BYPASS)) {
        return 0;
    }
#endif
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
#endif

    if (is_safemode == 1) {
        pMCB = &MCB[0];
        printf("DDR init safe mode\n");
    }

#if defined(_BCM94908_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96858_) || defined(_BCM96856_)
#ifdef CFE_ROM_STOP
        pMCB=select_default_mcb(pMCB);
#endif
#endif
#if 0 /* enable this when ddr testing is done */
    if( !is_safemode && !(memcfg&BP_DDR_CONFIG_DEBUG) )
        options |= MEMSYS_OPTION_CONSOLE_OUTPUT_DISABLED;
#endif

    memcfg = pMCB->config;
    printf("MCB chksum 0x%x, config 0x%x\n", pMCB->pMcbData[3], memcfg);

    params.version = MEMSYS_FW_VERSION;
    params.edis_info = 1;
    params.mem_test_size_bytes = SHMOO_TEST_SIZE;      // test size for SHMOO
    params.phys_mem_test_base = DRAM_BASE_NOCACHE; // test physical address base for SHMOO
    params.phys_memc_reg_base = (MEMC_BASE);
    params.phys_phy_reg_base = ((uint32_t)((uintptr_t)&MEMC->PhyControl));
    params.phys_shim_reg_base = 0;
    params.phys_edis_reg_base = ((uint32_t)((uintptr_t)&MEMC->EDIS_0));
    params.saved_state_base = 0;
    params.callbacks = &callbacks;
    params.mcb_addr = (uint32_t*)pMCB->pMcbData;
    params.options = options; 


    ret = memsys_top(&params);

    // return on error
    if (ret != 0)
    {
        int idx;
        printf("MEMSYS init failed, return code %08X\n\r", ret);

        printf("MEMC error: ");
        for (idx = 0; idx < MEMSYS_ERROR_MEMC_MAX_WORDS; idx++)
            printf(" 0x%08x \n\r", params.error.memc[idx]);
        printf("PHY error: ");
        for (idx = 0; idx < MEMSYS_ERROR_PHY_MAX_WORDS; idx++)
            printf(" 0x%08x \n\r", params.error.phy[idx]);
        printf("SHMOO error: ");
        for (idx = 0; idx < MEMSYS_ERROR_SHMOO_MAX_WORDS; idx++)
            printf(" 0x%08x \n\r", params.error.shmoo[idx]);
        printf("\n\r");

        memsys_end(&params, NULL);
#ifndef CONFIG_BRCM_IKOS
        return -1;
#endif
    }

#if defined(_BCM94908_) || defined(_BCM963158_) || defined(_BCM96858_)
    memc_setup_scrambler(memc_dram_profile.total_size_Mbits);
#endif

    /* make sure configure register write are really carried over to target block */
    __asm__ __volatile__ ("dsb	sy");
    __asm__ __volatile__ ("isb");

    /* Make sure it is configured size is not larger the actual size */
    if( is_safemode == 0 )
    {
        if( memc_alias_test(memc_dram_profile.total_size_Mbits))
        {
            printf("\nMemory alias detected. Probably wrong memory size is specified or memory subsystem not working\n");
            return -3;
        }
    }

    if( (ret = memc_memory_test(memc_dram_profile.total_size_Mbits)) )
        return ret;

    return ret;
}

