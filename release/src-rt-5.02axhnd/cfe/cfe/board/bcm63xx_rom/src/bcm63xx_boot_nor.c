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
#include "bcm_otp.h"
#include "flash_api.h"
#if (!defined(_BCM960333_) && !defined(_BCM96848_) && !defined(_BCM947189_) && !defined(_BCM963381_)) || (defined(_BCM963381_) && (INC_BTRM_BOOT==1))
#include "btrm_if.h"
#include "bcm63xx_storage_dev.h"
#include "bcm63xx_sec.h"
#endif

extern void _binArrayStart(void);
extern void _binArrayEnd(void);
#if defined(_BCM96838_) && !(INC_NAND_FLASH_DRIVER==1)
extern int spi_flash_init(flash_device_info_t **flash_info);
extern int spi_flash_get_sector_size(unsigned short sector);
extern int spi_flash_read_buf(unsigned short sector, int offset, unsigned char *buffer, int nbytes);
#endif

void bootNor(void)
{
#if (CFG_BOOT_PSRAM==0)
    unsigned char *pucSrc;
    unsigned char *pucDst;
    unsigned int *entryPoint;   
    uintptr_t binArrayStart = (uintptr_t)_binArrayStart;
    uintptr_t binArrayEnd = (uintptr_t) _binArrayEnd;
    unsigned int dataLen = binArrayEnd - binArrayStart - 4;
    int ret;
#endif

#if (CFG_BOOT_PSRAM==0)
#if defined(_BCM96838_) && !(INC_NAND_FLASH_DRIVER==1) && !(INC_EMMC_FLASH_DRIVER==1)
{
    flash_device_info_t* flash_info;
    volatile unsigned long start_sector, end_sector, sector_size;
    unsigned short i;
    unsigned int buf_addr = DRAM_BASE;
    
    spi_flash_init(&flash_info);
    sector_size = spi_flash_get_sector_size(0); //assume all sectors have the same size

    start_sector = (binArrayStart-PSRAM_BASE_KSEG0)/sector_size;
    end_sector = (binArrayEnd-PSRAM_BASE_KSEG0)/sector_size;
    
    for (i=start_sector; i<=end_sector; i++)
    {
        int offset, nbytes;
        unsigned char* buff = (unsigned char*)buf_addr;
        
        if (i==start_sector)
        {
            offset = (binArrayStart - PSRAM_BASE_KSEG0)%sector_size;
            nbytes = sector_size - offset;
        }
        else if (i==end_sector)
        {
            offset = 0;
            nbytes = (binArrayEnd - PSRAM_BASE_KSEG0)%sector_size;
        }
        else
        {
            offset = 0;
            nbytes = sector_size;
        }
        spi_flash_read_buf(i, offset, buff, nbytes);
        buf_addr += nbytes;
    }

    entryPoint = (unsigned int*) DRAM_BASE;
    pucSrc = (unsigned char*)(DRAM_BASE+4);
}
#else

#if defined(_BCM96848_)
    binArrayStart = ((unsigned int)_binArrayStart - PSRAM_BASE_KSEG0) + FLASH_BASE;
    binArrayEnd = ((unsigned int)_binArrayEnd - PSRAM_BASE_KSEG0) + FLASH_BASE;
#endif

    entryPoint = (unsigned int*) (binArrayStart);
    pucSrc = (unsigned char *) (binArrayStart + 4);

#if (INC_BTRM_BOOT==1) && ( defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined (_BCM963148_) )
    if (bcm_otp_is_boot_secure())
    {
       /* Decryption comes before decompression ... get the AES keys ready */
       /* Move pucDest to point to where the decrypted (but still compressed) CFE RAM will be put */
       unsigned char *pucDest = (unsigned char *)(BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR - 4);

       /* Get ready to decrypt the CFE RAM bootloader */
       /* decryptWithEk() will whack the content of the iv structure, therefore create a copy and pass that in */
       unsigned char origIv[CIPHER_IV_LEN];
       Booter1Args* bootArgs = cfe_sec_get_bootrom_args();
       if (!bootArgs) {
	    die();
       }
       memcpy((void *)origIv, (void *)bootArgs->encrArgs.biv, CIPHER_IV_LEN);

       /* Decrypt the CFE RAM bootloader */
       decryptWithEk(pucDest, pucSrc, bootArgs->encrArgs.bek, dataLen, origIv);

       /* The reference sw is done with the bek/biv at this point ... cleaning it up */
       /* Any remnants of the keys on the stack will be cleaned up when cfe_launch() runs */
       cfe_sec_reset_keys(); 
       memset((void *)origIv, 0, CIPHER_IV_LEN);

       /* Get the size of the compressed file */
       dataLen = *(unsigned int *)pucDest;

       /* pucSrc now becomes the location of where we just decrypted the image into because */
       /* decompression comes next */
       pucSrc = (unsigned char *)(BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR);
    }

#endif

#endif

    pucDst = (unsigned char *) ((uintptr_t)(*entryPoint));

    ret = decompressLZMA((unsigned char*)pucSrc,
        (unsigned int)dataLen,
        (unsigned char *) pucDst,
        23*1024*1024);

#if (INC_BTRM_BOOT==1) && ( defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined (_BCM963148_) )
    if (bcm_otp_is_boot_secure())
    {
       Booter1Args* bootArgs = cfe_sec_get_bootrom_args();
       if (!bootArgs) {
	    die();
       }
       /* If decompression is a success, place the authentication credentials just before the */
       /* cferam. Otherwise, perform clean up */
       if (ret != 0)
       {
          board_setleds(0x434d5046); /* CMPF for deCoMPression Failed */
          cfe_launch(0);
       }
       else
       {
#if defined(_BCM963138_) || defined(_BCM963148_)
          memcpy((void *)(pucDst - (16*1024) - sizeof(Booter1AuthArgs)), (void *)&bootArgs->authArgs, sizeof(Booter1AuthArgs));
#else
          memcpy((void *)(pucDst - sizeof(Booter1AuthArgs)), (void *)&bootArgs->authArgs, sizeof(Booter1AuthArgs));
#endif
       } 
    }
#else
    if (ret != 0) 
        while (1);          // if not decompressed ok, loop for EJTAG
#endif

    cfe_launch((unsigned long) pucDst); // never return...
#endif

    return;
}

