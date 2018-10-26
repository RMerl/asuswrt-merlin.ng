/*  *********************************************************************
    *
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
    ********************************************************************* */

#include "rom_main.h"
#include "bcm63xx_ipc.h"
#if (INC_PMC_DRIVER==1)
#include "pmc_drv.h"
#include "clk_rst.h"
#include "BPCM.h"
#endif
#if (INC_EMMC_FLASH_DRIVER==1)
#include "rom_emmc_drv.h"
#endif
#include "bcm_otp.h"
#include "bcm63xx_sec.h"

#if defined (_BCM963381_)
static void apply_ddr_ssc(void);

#if (INC_NAND_FLASH_DRIVER==1)
static void bump_nand_phase(int n) {
    int i;
    uint32_t val, val2;

    for (i = 0 ; i < n ; i++) {
        ReadBPCMRegister(PMB_ADDR_SYSPLL1, PLLBPCMRegOffset(ch01_cfg), &val);
        WriteBPCMRegister(PMB_ADDR_SYSPLL1, PLLBPCMRegOffset(ch01_cfg), ((uint32_t)val)|(0x1<<27));
        ReadBPCMRegister(PMB_ADDR_SYSPLL1, PLLBPCMRegOffset(ch01_cfg), &val2);
        WriteBPCMRegister(PMB_ADDR_SYSPLL1, PLLBPCMRegOffset(ch01_cfg), ((uint32_t)val));
    }
}
#endif

static void apply_ddr_ssc(void)
{
    uint32_t val;

    val = 0x1003200;
    WriteBPCMRegister(PMB_ADDR_SYSPLL, BPCMRegOffset(pwd_accum_control), val);
    val = 0x7AE1;
    WriteBPCMRegister(PMB_ADDR_SYSPLL, BPCMRegOffset(sr_control), val);

    cfe_usleep(10);

    val = 0x80007AE1;
    WriteBPCMRegister(PMB_ADDR_SYSPLL, BPCMRegOffset(sr_control), val);

    return;
}
#endif

#if defined(_BCM96846_) || defined (_BCM96856_) || defined(_BCM96858_)
extern void swrw(unsigned int ps, unsigned int reg, unsigned int val);
#endif

int bootInit(void)
{
    
#if defined(_BCM96846_)
    uint32_t revId = PERF->RevID & REV_ID_MASK;
#endif
#if (CFG_ROM_PRINTF==1)
    xprinthook = board_puts;
#endif
 
#if defined(_BCM960333_)
    xprintf("Boot Strap Register:  0x%x\n", STRAP->strapOverrideBus);
#elif defined(_BCM96838_)
    xprintf("Boot Strap Register:  0x%x\n", GPIO->strap_bus);
#elif !defined(_BCM947189_)
    xprintf("Boot Strap Register:  0x%x\n", MISC->miscStrapBus);
#endif

#if (INC_PMC_DRIVER==1)
    pmc_initmode();
#endif

#if defined(_BCM96846_)
        swrw(0,3,0x5171);
    if (revId == 0xA1)
    {
        swrw(0,6,0xb000);
        swrw(0,7,0x0029);
    }

    /* 1.8 SWREG  set bit reg3[8] & reg3[4]  pll_en and pll_phase_en */
        swrw(1,3,0x5170);
    if (revId == 0xA1)
        swrw(1,7,0x0029);

    /* 1.5 SWREG  set bit reg3[8] & reg3[4]  pll_en and pll_phase_en */
    swrw(2,3,0x5170);
    if (revId == 0xA1)
        swrw(2,7,0x0029);

    /* 1.0 Analog SWREG  set bit reg3[8] & reg3[4]  pll_en and pll_phase_en */
        swrw(3,3,0x5170);
    if (revId == 0xA1)
        swrw(3,7,0x0029);

    if (!(MISC->miscStrapBus & MISC_STRAP_ENABLE_INT_1p8V))
    {
        xprintf("1.8V External LDO detected.\n");
        xprintf("Disabling Internal 1.8V SWREG.\n");
        swrw(1,0,0xc691);
    }
#endif
#if defined(_BCM96856_)
    swrw(0,3,0x5372);
    swrw(0,6,0xb000);

    swrw(1,3,0x5370);
    swrw(2,3,0x5370);
    swrw(3,3,0x5370);
#endif

#if defined(_BCM96858_)
    swrw(1,3,0x5170);
    swrw(1,7,0x4829);
    swrw(2,3,0x5172);
    swrw(2,7,0x4829);
#endif

#if defined(_BCM963381_)
    /* Make sure PMC is up running if using PMC */
    if ( (MISC->miscStrapBus & MISC_STRAP_BUS_PMC_ROM_BOOT)
#if (INC_NAND_FLASH_DRIVER==1)
        && !bcm_otp_is_btrm_boot() && !strap_check_spinand()
#endif
    )
        while( (((PMC->ctrl.hostMboxIn)>>24)&0x7) < 2 );

    apply_ddr_ssc();
#endif

    clk_init();

#if defined(_BCM96846_) || defined (_BCM96856_)
    cfe_usleep(3000); /* Let to Voltage to stabilized */
#endif
    
#if !defined(_BCM947189_)
    if (rom_option & 0x1)
        cfe_mailbox_message_set(CFE_MAILBOX_SAFEMODE_SET(1));

    cfe_mailbox_message_set(CFE_MAILBOX_VER_SET(cfe_get_api_version()));
    cfe_mailbox_status_set();
#endif

#if (INC_NAND_FLASH_DRIVER==1)
#if (INC_SPI_NAND_DRIVER==1)
    if (strap_check_spinand())
        rom_spi_nand_init();
    else
#endif
    {
#if defined(_BCM963381_)
        /* apply the NAND phase bump patch */    
        board_setleds(0x4E44434B);  //NDCK
        bump_nand_phase(16);
#endif
        rom_nand_flash_init();
    }
#else
#if (INC_EMMC_FLASH_DRIVER==1)
    rom_emmc_init();
#endif        
#endif
#if defined(BOARD_SEC_ARCH)
    cfe_sec_init();
#endif

#if (CFG_COPY_PSRAM==1)
    /* copy NVRAM to DDR */
    memcpy((unsigned char *) (DRAM_BASE + NVRAM_DATA_OFFSET), (unsigned char *)
        PSRAM_BASE + NVRAM_DATA_OFFSET, sizeof(NVRAM_DATA));
#endif

    return 0;
}
