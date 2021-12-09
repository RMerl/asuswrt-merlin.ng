/*
<:copyright-BRCM:2012:DUAL/GPL:standard 

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

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

/***************************************************************************
 * File Name  : flash_api.h
 *
 * Description: This file contains definitions and prototypes for a public
 *              flash device interface and an internal flash device interface.
 ***************************************************************************/

#if !defined(_FLASH_API_H)
#define _FLASH_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* Flash definitions. */
#define FLASH_API_OK                1
#define FLASH_API_OK_BLANK          2
#define FLASH_API_ERROR            -1
#define FLASH_API_CORR             -2

/* Flashes supported by this flash API */
#define FLASH_IFC_UNKNOWN           0
#define FLASH_IFC_PARALLEL          1
#define FLASH_IFC_SPI               2
#define FLASH_IFC_HS_SPI            3
#define FLASH_IFC_NAND              4
#define FLASH_IFC_SPINAND           5

/* Flashes NOT supported by this flash API */
#define FLASH_IFC_UNSUP_EMMC        6

#define FLASH_FEATURES          0x80000000

/* device specific commands for function flash_dev_specific_cmd */
#define WRITE_WITHOUT_ECC       0
#define NAND_REINIT_FLASH       1
#define NAND_REINIT_FLASH_BAD   2
#define CHECK_BAD_BLOCK         3
#define MARK_BLOCK_BAD          4
#define FORCE_ERASE             5
#define GET_PAGE_SIZE           6
#define GET_SPARE_SIZE          7
#define GET_ECC_OFFSET          8
#define GET_FEATURE             9
#define SET_FEATURE             10

/* MTD determintation of filesystem type */
#define JFFS2_FS   0
#define SPLIT_FS   1  /* split JFFS2/UBI */
#define UBIFS_FS   2  /* pureUBI */
#define SQSPLIT_FS 3  /* squashfs over split JFFS2/UBI */
#define SQUBI_FS   4  /* squashfs over pureUBI */

/* Partition type */
#define JFFS2_IMAGE  1 // JFFS2 or split JFFS2/UBI, old image type
#define UBI_IMAGE    2 // new pureUBI image type


/* Public Interface Prototypes. */
int flash_init(void);
int flash_sector_erase_int(unsigned short sector);
int flash_read_buf(unsigned short sector, int offset, unsigned char *buffer,
    int numbytes);
int flash_write_buf(unsigned short sector, int offset, unsigned char *buffer,
    int numbytes);
int flash_get_numsectors(void);
int flash_get_sector_size(unsigned short sector);
unsigned char *flash_get_memptr(unsigned short sector);
int flash_get_blk(int addr);
unsigned long flash_get_total_size(void);
int flash_dev_specific_cmd (unsigned int command, void * inBuf, void * outBuf);
int flash_get_flash_type(void);
void flash_change_flash_type(int type);


/* Internal Flash Device Driver Information. */
typedef struct flash_device_info_s
{
    unsigned short flash_device_id;
    unsigned short flash_type;
    char flash_device_name[30];

    int (*fn_flash_sector_erase_int) (unsigned short sector);
    int (*fn_flash_read_buf) (unsigned short sector, int offset,
        unsigned char *buffer, int numbytes);
    int (*fn_flash_write_buf) (unsigned short sector, int offset,
        unsigned char *buffer, int numbytes);
    int (*fn_flash_get_numsectors) (void);
    int (*fn_flash_get_sector_size) (unsigned short sector);
    unsigned char * (*fn_flash_get_memptr) (unsigned short sector);
    int (*fn_flash_get_blk) (int addr);
    int (*fn_flash_get_total_size) (void);
    int (*fn_flash_dev_specific_cmd) (unsigned int command, void * inBuf, void * outBuf);
} flash_device_info_t;

struct flash_name_from_id {
    unsigned short fnfi_id;
    char fnfi_name[30];
};

typedef struct flash_write_data_s {
    unsigned short block;
    unsigned int page;
    unsigned int offset;
    unsigned int amount;
    unsigned char * data;
} flash_write_data_t;


#ifdef __cplusplus
}
#endif

#endif /* _FLASH_API_H */

