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
#include "rom_emmc_drv.h"
#include "initdata.h"

static int strap_check_emmc(void)
{
#if defined(_BCM94908_) || defined(_BCM963158_) || defined(_BCM96856_)
    return ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_EMMC) == MISC_STRAP_BUS_BOOT_EMMC);
#elif defined(_BCM96858_)
    {
        uint32 bootsel = ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL0_4_MASK) >> MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT) |
                         ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL5_MASK) >> BOOT_SEL5_STRAP_ADJ_SHIFT);

        if ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_MASK) == BOOT_SEL_STRAP_EMMC)
            return 1;
        else
            return 0;
    }
#elif defined(_BCM963138_)
    return ( (MISC->miscStrapBus & MISC_STRAP_BUS_SW_BOOT_SPI_SPINAND_EMMC_MASK) &&
            ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_OPT_MASK) == MISC_STRAP_BUS_BOOT_EMMC) );
#else
    return 0;

#endif 
}

void bootImageFromEmmc(unsigned long rom_param, cfe_rom_media_params *media_params)
{
#if (CFG_COPY_PSRAM==1)
    unsigned char *buf = (unsigned char *) ((mem_heapstart & 0x0000ffff) | DRAM_BASE);
#else
#if (defined(_BCM963138_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || \
    defined(_BCM96856_))
    /* 631x8 run cfe rom in internal memory which is only 512KB, switch heap to ddr 
       in case nand block size is 512KB or larger */
    unsigned char *buf = (unsigned char *) DRAM_BASE + 0x1000;
#else
    unsigned char *buf = (unsigned char *) mem_heapstart;
#endif
#endif
    rom_emmc_boot(buf, rom_param, media_params);
    xprintf("eMMC Boot Failed!\n");
    while(1)
    { 
    }

}

void bootEmmc(cfe_rom_media_params *media_params)
{
    if (strap_check_emmc())
    {
        bootImageFromEmmc(rom_option, media_params); /* Will not return. */
    }
}
