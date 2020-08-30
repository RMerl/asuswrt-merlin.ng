
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

/*Simple wrap over read/write primitives from flash_api and emmc
  This typically achieved via cfe driver framework which is normally linked to cfe_ram
This a greatly simplified version
*/
#include "rom_main.h"
#include "lib_byteorder.h"
#include "initdata.h"

#if (INC_EMMC_FLASH_DRIVER==1) 
#include "dev_bcm63xx_emmc_common.h"
#include "rom_emmc_drv.h"
#include "bcmTag.h"
#endif
#include "bcm63xx_storage_dev.h"

#if (INC_NAND_FLASH_DRIVER==1) || (INC_SPI_NAND_DRIVER==1)
extern int nand_flash_get_sector_size(unsigned short sector);
extern int nand_flash_get_numsectors(void);
extern int nand_flash_read_buf(unsigned short blk, int offset,
    unsigned char *buffer, int len);
extern int spi_nand_get_sector_size(unsigned short sector);
extern int spi_nand_get_numsectors(void);
extern int spi_nand_read_buf(unsigned short blk, int offset, 
    unsigned char *buffer, int len);
extern int strap_check_spinand(void);
#elif (INC_EMMC_FLASH_DRIVER!=1)
/*
 * For the default case, which is usually SPI NOR, there are no bad blocks or
 * translation layers to worry about. We can read the data up to the first 1MB
 * via memcpy since this area is directly addressable by the hardware.
 */
#include "bcm_btrm_gen3_common.h"
#define SECT_4K                  4096         /* Default, smallest sector size */
#define ADDRESSABLE_NOR_AREA    (1024 * 1024)

/*
 * This function is here for compatibility reasons. It really does not matter
 * what we return as long as sector size times number of sectors equal to the
 * addresseable area.
 */
static int nor_get_sector_size(unsigned short sector)
{
        return SECT_4K;
}
/*
 * This function is here for compatibility reasons. See above
 * nor_get_sector_size()
 */
static int nor_get_numsectors(void)
{
        return ADDRESSABLE_NOR_AREA / SECT_4K;
}
/*
 * Copy from the addressable area into the buffer using the helper functions
 */
static int nor_read_buf(unsigned short sector, int offset, unsigned char *buffer, int nbytes)
{
        uintptr_t src = BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_SPI +
                        (sector * nor_get_sector_size(sector)) + offset;
        memcpy (buffer, (const void *) src, nbytes);
        return nbytes;
}

#endif
typedef struct _flash_dev {
    unsigned short flash_type;
    int (*fn_flash_read_buf) (unsigned short sector, int offset,
        unsigned char *buffer, int numbytes);
    int (*fn_flash_get_numsectors) (void);
    int (*fn_flash_get_sector_size) (unsigned short sector);

} flash_dev_t;


typedef struct _cfe_storage_dev_obj {
        cfe_storage_dev_t dptr;
        cfe_storage_dev_info_t info;
        flash_dev_t dev;
        int state;
#if (INC_EMMC_FLASH_DRIVER==1)
	unsigned int boot_offs;
#endif
} _obj_t;

static _obj_t _obj;


#if (INC_EMMC_FLASH_DRIVER==1)
extern int cferom_emmc_read_full_boot_blocks( unsigned long long byte_offset, unsigned char * dest_buf, int length );
static inline int _read_buf(unsigned short sector, 
                           int offset,
                           unsigned char *buffer, 
                           int numbytes)
{
        /*aligning to 512 bytes boundary*/
        unsigned int address =  sector*EMMC_DFLT_BLOCK_SIZE + offset;
        int res = cferom_emmc_read_full_boot_blocks(address&(~(EMMC_DFLT_BLOCK_SIZE-1)), buffer, (address+numbytes) - (address&~(EMMC_DFLT_BLOCK_SIZE-1)));
	lib_memmove(buffer,buffer+(address%EMMC_DFLT_BLOCK_SIZE), numbytes);
        return res;
}

static  inline int _get_numsectors (void)
{
        return (1024*1024)/EMMC_DFLT_BLOCK_SIZE;
}
static int inline _get_sector_size(unsigned short sector)
{
        return EMMC_DFLT_BLOCK_SIZE;
}
#endif

static inline void _init_flash_device(flash_dev_t * info)
{
#if (INC_NAND_FLASH_DRIVER==1) || (INC_SPI_NAND_DRIVER==1)
#if  (INC_SPI_NAND_DRIVER==1)
    if (strap_check_spinand()) {
       info->fn_flash_read_buf = spi_nand_read_buf;
       info->fn_flash_get_sector_size = spi_nand_get_sector_size;
       info->fn_flash_get_numsectors = spi_nand_get_numsectors;
    } else 
#endif
    {
    /*if (strap_check_nand()) {*/
       info->fn_flash_read_buf = nand_flash_read_buf;
       info->fn_flash_get_sector_size = nand_flash_get_sector_size;
       info->fn_flash_get_numsectors = nand_flash_get_numsectors;
    }
#elif (INC_EMMC_FLASH_DRIVER==1)
       info->fn_flash_read_buf = _read_buf;
       info->fn_flash_get_sector_size = _get_sector_size;
       info->fn_flash_get_numsectors = _get_numsectors;
#else
       info->fn_flash_read_buf = nor_read_buf;
       info->fn_flash_get_sector_size = nor_get_sector_size;
       info->fn_flash_get_numsectors = nor_get_numsectors;
#endif
}

static int _read_raw(unsigned int offs, 
                unsigned int size,
                void* buf)
{
        unsigned int block;
        if (!_obj.state) {
                return -1;
        }
        if (offs+size > _obj.info.size) {
                return -1;
        }
        block = offs/_obj.info.block_size;
        /* Zero based block addressing */
        return _obj.dev.fn_flash_read_buf(block, (offs%_obj.info.block_size), buf, size);
}

static int _get_info(cfe_storage_dev_info_t **info)
{
        if (!_obj.state) {
                return -1;
        }
        *info = &_obj.info;
        return 0;
}

int cfe_storage_dev_init(void)
{
        if (_obj.state) {
                return 0;
        }
	_init_flash_device(&_obj.dev);
        _obj.dptr.read_raw = _read_raw;
        _obj.dptr.get_info = _get_info;
        _obj.info.block_size = _obj.dev.fn_flash_get_sector_size(0);
        _obj.info.size = _obj.dev.fn_flash_get_numsectors()*_obj.info.block_size;
        _obj.state = 1;
        return 0;
}

void cfe_storage_dev_reset()
{
        if (_obj.state) {
                memset(&_obj,0,sizeof(_obj));
        } 
}

cfe_storage_dev_t* cfe_storage_dev_get()
{
        if (!_obj.state) {
                return NULL;
        }
        return &_obj.dptr;
}
