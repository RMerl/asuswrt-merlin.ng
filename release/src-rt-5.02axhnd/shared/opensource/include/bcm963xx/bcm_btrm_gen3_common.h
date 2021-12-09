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
#ifndef __BCM_BTRM_GEN3_COMMON_H
#define __BCM_BTRM_GEN3_COMMON_H

#define BTRM_SBI_UNAUTH_HDR_MAX_SIZE            64 
#define BTRM_SBI_AUTH_HDR_MAX_SIZE              (8*1024)
#if defined(_BCM96836_) || defined(_BCM96846_) || defined(_BCM6856_)
#define BTRM_SBI_IMAGE_MAX_ALLOWED_SIZE		(120*1024) /* only a total of 192K sram to play with */
#elif defined(_BCM963158_)
#define BTRM_SBI_IMAGE_MAX_ALLOWED_SIZE		(BTRM_INT_MEM_END_ADDR-BTRM_INT_MEM_BEGIN_ADDR-BTRM_INT_MEM_DOWNLOAD_OFFSET)
#else
#define BTRM_SBI_IMAGE_MAX_ALLOWED_SIZE		(128*1024)
#endif

#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_EMMC   0xffc00000
#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_SPI    0xffd00000
#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_NAND   0xffe00000

#if ! defined(__ASSEMBLER__)
typedef struct
{
   uint32_t     magic_1;
   uint32_t     magic_2;
   uint32_t     ver;
   uint32_t     modeElegible;
   uint32_t     hdrLen;
   uint32_t     sbiSize;
} __attribute__((packed)) SbiUnauthHdrBeginning;

typedef struct
{
   uint32_t     ver;
   uint32_t     hdrLen;
   uint32_t     authLen; 
   uint32_t     mfgRoeCotOfs;
   uint32_t     mfgOemCotOfs;
   uint32_t     fldRotCotOfs;
   uint32_t     fldOemCotOfs;
} __attribute__((packed)) SbiAuthHdrBeginning;

#endif

#endif
