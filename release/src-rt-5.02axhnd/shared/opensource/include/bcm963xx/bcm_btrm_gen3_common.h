/*
    Copyright 2000-2016 Broadcom Corporation

    <:label-BRCM:2016:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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
