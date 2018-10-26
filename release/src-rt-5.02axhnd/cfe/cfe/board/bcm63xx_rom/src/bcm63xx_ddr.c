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
#if INC_EMMC_FLASH_DRIVER
#include "rom_emmc_drv.h"
#endif
#include "initdata.h"

extern int ddr_init(uint32_t mcb_selector, uint32_t memcfg_from_flash);
#ifdef INC_DDR_DIAGS
extern void cde_main_entry(int value);
#endif


#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_NAND_DRIVER || INC_SPI_PROG_NAND == 1) && (INC_NVRAM_MIRRORING == 1)
#include "bcm63xx_utils.h"
#endif

static uint32_t read_memcfg(void)
{
    /* Get mcb from NVRAM */
    uint32_t memcfg = 0;
#ifdef CONFIG_BRCM_IKOS
    /* IKOS */
    memcfg = BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_2048MB| BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_32BIT;
    printf("Overwrite to memcfg 0x%x\n", memcfg);
#else
#if (INC_NAND_FLASH_DRIVER==1)
    /* NAND */
#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_NAND_DRIVER || INC_SPI_PROG_NAND == 1) && (INC_NVRAM_MIRRORING == 1)
    read_memcfg_from_nvram(&memcfg);
#else
    nand_read_buf(0, IMAGE_OFFSET + 0x580 + offsetof(NVRAM_DATA,ulMemoryConfig), (unsigned char *)&memcfg, 4);
#endif
#else
#if (INC_EMMC_FLASH_DRIVER==1)              
    /* EMMC */
    rom_emmc_get_nvram_memcfg(&memcfg, (unsigned char *) mem_heapstart);
#elif (INC_SPLIT_NOR_SECONDARY_STAGE==1)
#define NVRAM_NOR_BASE 0x1c000
    /*
     * In the split NOR configuration, due to limited space, we have a single
     * copy of the board NVRAM and it's at a fixed location. This is also
     * defined in u-boot and the two copies need to stay in sync.
     */
    memcfg = *((uint32*)((FLASH_BASE + NVRAM_NOR_BASE + offsetof(NVRAM_DATA,ulMemoryConfig))));
#else
    /* NOR */
    memcfg = *((uint32*)((FLASH_BASE + IMAGE_OFFSET + 0x580 + offsetof(NVRAM_DATA,ulMemoryConfig))));
#endif /* INC_EMMC_FLASH_DRIVER==1 */       
#endif /* INC_NAND_FLASH_DRIVER==1 */
#endif /* CONFIG_BRCM_IKOS */

   return memcfg;
}

#ifdef INC_DDR_DIAGS
static void ddr_diag(void)
{

#if defined(_BCM963138_)
    /* disable AIP fast ack as cde will modify the memc/phy config 
       and want to take effect right away */
    ARMAIPCTRL->cfg &= ~0x2;
#endif
#if defined(_BCM963148_)
    /* disable ubus on the fly multiple read/write transaction */
    B15CTRL->cpu_ctrl.ubus_cfg &= ~0x70;
#endif

    cde_main_entry(0);

#if defined(_BCM963138_)
    ARMAIPCTRL->cfg |= 0x2;
#endif
#if defined(_BCM963148_)
    B15CTRL->cpu_ctrl.ubus_cfg |= 0x70;
#endif

    return;
}

/* additonal DDR DIAG library required API */
#if defined(_BCM963158_)
/* strstr is macro to lib_strstr function in library header. But ddr diag wants 
   strstr.  Don't add ifdef in library header. Just undefine here locally
   for ddr diag build */
#ifdef strstr
#undef strstr
#endif
char * strstr(const char *in, const char *str)
{
    return lib_strstr(in, str);
}

uint32_t reg_read(uint32_t phys_reg_addr)
{
    return *((volatile uint32_t*)((uintptr_t)phys_reg_addr));
}

void reg_write(uint32_t phys_reg_addr, uint32_t value)
{
    *((volatile uint32_t*)((uintptr_t)phys_reg_addr)) = value;
}

int has_char(void)
{
    return board_stsc();
}

char read_char(void)
{
    return board_getc();
}
#endif

#endif


int ddrInit(void)
{
    int rc;

    rc = ddr_init((uint32_t)rom_option, read_memcfg());
#ifdef INC_DDR_DIAGS
    ddr_diag();
#else
    if( rc != 0 )
        stopCfeRom();
#endif

    return rc;
}
