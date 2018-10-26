/*
    Copyright 2000-2016 Broadcom Corporation

    <:label-BRCM:2016:DUAL/GPL:standard
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
    :>
*/                       
#ifndef __BCM_BTRM_GEN2_COMMON_H
#define __BCM_BTRM_GEN2_COMMON_H

/* Sizes of elements that comprise the SBI image */
#define BTRM_SBI_SIGNED_COT_ELEM_SIZE           584
#define BTRM_SBI_DAUTH_ELEM_SIZE                  0  
#define BTRM_SBI_AUTH_HDR_SIZE                    8
#define BTRM_SBI_UNAUTH_HDR_SIZE                 20 

#if defined(_BCM963138_) || defined(_BCM963148_)

#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_EMMC   0xffc00000
#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_SPI    0xffd00000
#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_NAND   0xffe00000
#define BTRM_SBI_IMAGE_MAX_ALLOWED_SIZE		(192*1024)

#elif defined(_BCM96838_)

#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_SPI    0xbfc00000
#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_NAND   0xbfc00000
#define BTRM_SBI_IMAGE_MAX_ALLOWED_SIZE		(128*1024)

#elif defined(_BCM963381_) 

#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_SPI    FLASH_BASE
#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_NAND   FLASH_BASE
#define BTRM_SBI_IMAGE_MAX_ALLOWED_SIZE		(128*1024)

#elif defined(_BCM96848_)

#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_SPI    FLASH_BASE
#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_NAND   FLASH_BASE
#define BTRM_SBI_IMAGE_MAX_ALLOWED_SIZE		(128*1024)

#endif

typedef struct
{
   uint32_t     magic_1;
   uint32_t     magic_2;
   uint32_t     ver;
   uint32_t     len;
   uint32_t     crc;
} __attribute__((packed)) SbiUnauthHdr;

typedef struct
{
   uint16_t     type;
   uint16_t     ver;
   uint16_t     len;
   uint16_t     config;
} __attribute__((packed)) SbiAuthHdr;

#endif
