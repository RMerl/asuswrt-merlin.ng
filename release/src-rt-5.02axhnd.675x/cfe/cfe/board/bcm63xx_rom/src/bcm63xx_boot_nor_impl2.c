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

/*4k legacy cferom for 1st nvram*/
#define NOR_LEGACY_CFEROM_FOR_NVRAM 4096
/*FLASH_BASE for SPI is 0xFFD00000
 can use BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_SPI too, the same value*/
#define NOR_CFE_OFFSET FLASH_BASE + IMAGE_OFFSET
#define NOR_CFERAM_OFFSET_LOCATION (NOR_CFE_OFFSET + NOR_LEGACY_CFEROM_FOR_NVRAM - 4)

/* bcm image header */
/*copy from cfe\board\bcm63xx_ram\include\bcm63xx_util.h*/
typedef struct bcm_image_hdr {
    uint32_t la;
    uint32_t entrypt;
    uint32_t len;
    uint32_t magic;
    uint32_t len_uncomp;
} bcm_image_hdr_t;

static uint32_t get_cferam_offset(void)
{
	return *((uint32_t *)(NOR_CFERAM_OFFSET_LOCATION));
}


void bootNor(void)
{
    //char brcmMagic[] = {'B','R','C','M'}; // B-42 R-52 C-43 M 4D
    uint32_t brcmMagic = 0x4D435242; //little endian value for {'B','R','C','M'}
    bcm_image_hdr_t image_hdr;
	uint32_t cferam_offset;
	uint32_t image_hdr_size;
	unsigned char *pucDst;
    int ret = 1;

	cferam_offset = get_cferam_offset();
	image_hdr_size = sizeof(image_hdr);
	memcpy(&image_hdr, (void*)(NOR_CFE_OFFSET + cferam_offset), image_hdr_size);
	if (image_hdr.magic == brcmMagic) {
		//for cferam, image_hdr.la should equal image_hdr.entrypt
		pucDst = (unsigned char*)(unsigned long)image_hdr.entrypt;
		ret = decompressLZMA((unsigned char*)(NOR_CFE_OFFSET + cferam_offset + image_hdr_size), 
			                  image_hdr.len, 
			                  (unsigned char *)pucDst,
			                  23*1024*1024);
	}

	if (ret != 0) {
		xprintf("No Cferam\n");
		die();
	}
	else {
		xprintf("Booting Cferam...\n");
		cfe_launch((unsigned long) pucDst);
	}

    return;
}

