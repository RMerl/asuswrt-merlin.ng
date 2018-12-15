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

#if !defined(JTAG_DOWNLOAD)
static void set_boot_secure_params(cfe_rom_media_params *media_params);
#endif 

void boot_media(void)
{
    /* boot cfe ram and will not return. */
#if !defined(JTAG_DOWNLOAD)
    cfe_rom_media_params media_params;
    
	set_boot_secure_params(&media_params);
	
#if (INC_NAND_FLASH_DRIVER==1)
    bootNand(&media_params);
#endif

#if (INC_EMMC_FLASH_DRIVER==1)
    bootEmmc(&media_params);
#endif

#if (INC_SPI_NAND_DRIVER==1)
    bootSpiNand(&media_params);
#endif

#endif

#if (INC_SPI_FLASH_DRIVER==1)
    bootNor();
#endif	
}




#if !defined(JTAG_DOWNLOAD)
void set_boot_secure_params(cfe_rom_media_params *media_params)
{
    int boot_secure = 0;

	memset(media_params, 0, sizeof(cfe_rom_media_params));
	strncpy(media_params->boot_file_name, NAND_CFE_RAM_NAME, CFE_BOOT_FILE_NAME_MAX_LENGTH-1);
#if (!defined(_BCM960333_) && !defined(_BCM96848_) && !defined(_BCM947189_) && !defined(_BCM963381_)) || (defined(_BCM963381_) && (INC_BTRM_BOOT==1))

    /* If secure boot is in play, the CFE RAM file name is different */
    boot_secure = bcm_otp_is_boot_secure();
    if (boot_secure)
    {       
        boot_secure = BCM_OTP_BOOT_SECURED;
	    strncpy(media_params->boot_file_name, NAND_CFE_RAM_SECBT_NAME, CFE_BOOT_FILE_NAME_MAX_LENGTH-1);
	    strncpy(media_params->hash_file_name, NAND_HASH_SECBT_NAME, CFE_BOOT_FILE_NAME_MAX_LENGTH-1);
    }
#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_)
    else
    {
       /* Gen3 bootrom. If secure boot is in play, it can be mfg-secure or field-secure, hence two CFE RAM names */
       int boot_mfg_secure = bcm_otp_is_boot_mfg_secure();
       if (boot_mfg_secure)
       {
           boot_secure = BCM_OTP_BOOT_MFG_SECURED;
   	       strncpy(media_params->boot_file_name, NAND_CFE_RAM_SECBT_MFG_NAME, CFE_BOOT_FILE_NAME_MAX_LENGTH-1);
   	       strncpy(media_params->hash_file_name, NAND_HASH_SECBT_MFG_NAME, CFE_BOOT_FILE_NAME_MAX_LENGTH-1);
       }
    }
#endif
#endif  
    media_params->boot_secure = boot_secure;
}
#endif 



/* parse_boot_hashes, traverse the hashes TLV, retrieve the bootable entry, and populate media_params */
void parse_boot_hashes(char *hashes, cfe_rom_media_params *media_params)
{
   char *cp;
   struct boot_hash_tlv thead;
   cp = hashes;
   do {
      memcpy(&thead,cp,sizeof(thead));
      if ((thead.type == BOOT_HASH_TYPE_NAME_LEN_SHA256) && ((thead.options & BOOT_FILE_FLAG_HASH_BOOT) != 0)) {
         media_params->boot_file_flags = thead.options;
         strncpy(media_params->boot_file_name, cp + sizeof(thead), CFE_BOOT_FILE_NAME_MAX_LENGTH -1);
         // skip over name and len to sha256
         memcpy(media_params->boot_file_hash, cp + thead.length - SHA256_S_DIGEST8, SHA256_S_DIGEST8);
         media_params->boot_file_hash_valid = 1;
         break;
      }

   cp = cp + thead.length;
   } 
   while ( thead.type != BOOT_HASH_TYPE_END) ;

}

