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
#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_PROG_NAND == 1 || INC_SPI_NAND_DRIVER == 1) && (INC_NVRAM_MIRRORING == 1)
#include "bcm_ubi.h"
#include "bcm63xx_utils.h"
static NVRAM_DATA nvram_data;
#include "lib_crc.h"
#endif
#if defined(_BCM96858_) || defined(_BCM94908_) || defined(_BCM963158_) || defined(_BCM96856_) 
extern void  armv8_disable_mmu(void);
#elif defined(_BCM96846_)
extern void  armv7_disable_mmu(void);
#endif

int cfe_size_ram(void);
void stop_cferom(void);
// fake functions for not to modifying init_mips.S
void _exc_entry(void);
void cfe_command_restart(void);
void cfe_doxreq(void);
void cfe_ledstr(const char *leds);

void stop_cferom(void)
{
    while(1);
}

int cfe_size_ram(void)
{
#if defined(IKOS_NO_DDRINIT)
    return 64 << 10;    /* assuming 64MB memory for IKOS */
#endif
#if defined(_BCM960333_) || defined(_BCM963381_)
    uint32 memCfg;

#ifdef _BCM963381_
    memCfg = MEMC->SDR_CFG.SDR_CFG;
#else
    memCfg = MEMC->SDR_CFG;
#endif

    memCfg = (memCfg&MEMC_SDRAM_SPACE_MASK)>>MEMC_SDRAM_SPACE_SHIFT;

    return 1<<(memCfg+20);
#elif defined (_BCM963158_) || defined (_BCM96846_) || defined (_BCM96856_)
    return 1<<(((MEMC->GLB_FSBL_STATE&MEMC_GLB_FSBL_DRAM_SIZE_MASK)>>MEMC_GLB_FSBL_DRAM_SIZE_SHIFT)+20);
#elif defined(_BCM947189_)
    /*
     * 47189: Complete this with a suitable implementation if necessary (this
     * function is not really useful at all)
     */
    return 0;
#elif defined(_BCM96838_) || defined(_BCM963268_)
    return (MEMC->CSEND << 24);
#else
    return 1<<(((MEMC->GLB_GCFG&MEMC_GLB_GCFG_SIZE1_MASK)>>MEMC_GLB_GCFG_SIZE1_SHIFT)+20);
#endif
}

void _exc_entry(void)
{
}

void cfe_command_restart(void)
{
}

void cfe_doxreq(void)
{
}

void cfe_ledstr(const char *leds)
{
}

void stopCfeRom(void)
{
#if defined(_BCM96858_) || defined(_BCM94908_) || defined(_BCM963158_) || defined (_BCM96856_)
    armv8_disable_mmu();
#elif defined(_BCM96846_)
    armv7_disable_mmu();
#endif
    stop_cferom();

    return;
};

#if defined(IKOS_BD_LINUX_ROM)
void launchLinux(void)
{
#if defined (_BCM963138_) || defined(_BCM963148_) || defined(_BCM96846_)
   static unsigned long linuxStartAddr=0x8000;
//   printf("L2C_FILT_START 0x%x L2C_FILT_END 0x%x\n", *((unsigned int*)0x8001dc00), *((unsigned int*)0x8001dc04));
#elif defined(_BCM963381_)
   static unsigned long linuxStartAddr=0x80010400;
#elif defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined (_BCM96856_)
   static unsigned long linuxStartAddr=0x80000;/* 512KB offset */
#else
    /*0x694b6f32 (iKo2) is replaced with actual addr during the build process*/
    static unsigned long linuxStartAddr=0x694b6f32;
#endif

    board_setleds(0x4c494e58);  //LINX
    cfe_size_ram();

    cfe_launch(linuxStartAddr);

    return;
}
#endif

#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_PROG_NAND == 1 || INC_SPI_NAND_DRIVER == 1) && (INC_NVRAM_MIRRORING == 1)
static int validate_nvram(PNVRAM_DATA nvram_data, uint32_t offset)
{

    UINT32 crc=0;
    int ret=1;
    nand_read_buf(0, offset, (unsigned char *)nvram_data, sizeof(NVRAM_DATA));

    if(check_jffs_ubi_magic((unsigned char *)nvram_data)) {
        ret=2;
    }
    else
    {
        crc = nvram_data->ulCheckSum;
        nvram_data->ulCheckSum=0;
        if (crc == getCrc32((unsigned char *)nvram_data, sizeof(NVRAM_DATA),
                            CRC32_INIT_VALUE)) {
            ret=0;
        }
    }

return ret;

} 

static int validate_nvram_get_memcfg(PNVRAM_DATA nvram_data, uint32_t offset, uint32_t *memcfg)
{
    int ret=0;
    *memcfg=-1;


    if((ret=validate_nvram(nvram_data, offset)) == 0)
        *memcfg=nvram_data->ulMemoryConfig;
    
return ret;
}

void nvram_read(PNVRAM_DATA nvram_data)
{

    uint32_t offset=IMAGE_OFFSET + 0x580;
    int ret=0, search_complete=0;

    if(validate_nvram(nvram_data, offset) != 0) {
        printf("Primary NV unreadable, searching backup NV, please wait... \n");
        for(offset=0; search_complete == 0 && offset < MAX_BOOT_BLOCK_MIRROR_LOOKUP_OFFSET; offset += 1024)
        {
            ret=validate_nvram(nvram_data, offset+strlen(NVRAM_DATA_SIGN));
            switch(ret) {
                case 0:
                    printf("Backup NVRAM_DATA found at %x \n", offset);
                    search_complete = 1;
                    break;
                case 2:
                    search_complete = 1;
                    break;
            }
           
        }
    }
}
void read_memcfg_from_nvram(uint32_t *memcfg)
{

    uint32_t offset=IMAGE_OFFSET + 0x580;
    int ret=0, search_complete=0;

    ret=validate_nvram_get_memcfg(&nvram_data, offset, memcfg);
    if(ret != 0) {
        printf("Primary NV unreadable, searching backup NV, please wait... \n");
        for(offset=0; search_complete == 0 && offset < MAX_BOOT_BLOCK_MIRROR_LOOKUP_OFFSET; offset += 1024)
        {
            ret=validate_nvram_get_memcfg(&nvram_data, offset+strlen(NVRAM_DATA_SIGN), memcfg);
            switch(ret) {
                case 0:
                    printf("Backup NVRAM_DATA found at %x \n", offset);
                    search_complete = 1;
                    break;
                case 2:
                    search_complete = 1;
                    break;
            }
           
        }
    }
}
#endif
