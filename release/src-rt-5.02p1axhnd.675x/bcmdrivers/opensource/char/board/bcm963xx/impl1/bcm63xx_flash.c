/* 
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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

/*
 ***************************************************************************
 * File Name  : bcm63xx_flash.c
 *
 * Description: This file contains the flash device driver APIs for bcm63xx board. 
 *
 * Created on :  8/10/2002  seanl:  use cfiflash.c, cfliflash.h (AMD specific)
 *
 ***************************************************************************/


/* Includes. */
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/preempt.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/mtd/mtd.h>
#include <linux/kernel.h>
#include <linux/fcntl.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/unistd.h>
#include <linux/jffs2.h>
#include <linux/mount.h>
#include <linux/crc32.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <linux/compiler.h>
#include <linux/ctype.h>
#include <linux/math64.h>

#include "bcm_assert_locks.h"
#include "bcmtypes.h"
#include "bcm_map_part.h"
#include "board.h"
#include "bcmTag.h"
#include "flash_api.h"
#include "boardparms.h"
#include "boardparms_voice.h"
#include "board_util.h"
#include "board_image.h"
#include "bcm_otp.h"
#include "emmc_linux_defs.h"


#include <linux/fs_struct.h>
//#define DEBUG_FLASH

FLASH_ADDR_INFO fInfo;
struct semaphore semflash;
int flash_type = 0;

// mutex is preferred over semaphore to provide simple mutual exclusion
// spMutex protects scratch pad writes
DEFINE_MUTEX(spMutex);

void updateInMemNvramData(const unsigned char *data, int len, int offset);

extern struct mutex flashImageMutex;

/*
 * inMemNvramData an in-memory copy of the nvram data that is in the flash.
 * This in-memory copy is used by NAND.  It is also used by NOR flash code
 * because it does not require a mutex or calls to memory allocation functions
 * which may sleep.  It is kept in sync with the flash copy by
 * updateInMemNvramData.
 */
static unsigned char *inMemNvramData_buf;
static NVRAM_DATA inMemNvramData;
DEFINE_MUTEX(inMemNvramData_mutex);
#define UNINITIALIZED_FLASH_DATA_CHAR  0xff
static char cfe_boot[14]={0};
#ifdef CONFIG_BCM_DISABLE_NOR_RAW_PARTITION
static int disable_nor_raw_partition=1;
#else
static int disable_nor_raw_partition=0;
#endif

void lock_inMemNvramData_mutex()
{
    mutex_lock(&inMemNvramData_mutex);
}
void unlock_inMemNvramData_mutex()
{
    mutex_unlock(&inMemNvramData_mutex);
}

NVRAM_DATA* get_inMemNvramData(void)
{
return &inMemNvramData;
}
int is_cfe_boot(void)
{
    int ret=0;
    //if bootloader string is missing or is "cfe"
    if(strcmp(cfe_boot, BOOTLOADER_CFE_STR)==0)
        ret=1;
return ret;
}

void sync_nvram_with_flash(void)
{
    BCM_ASSERT_NOT_HAS_MUTEX_C(&flashImageMutex);
    inMemNvramData.ulCheckSum = 0;
    inMemNvramData.ulCheckSum = crc32(CRC32_INIT_VALUE, &inMemNvramData, sizeof(NVRAM_DATA));
    mutex_lock(&flashImageMutex);
    kerSysNvRamSet((char *)&inMemNvramData, NVRAM_LENGTH, 0);
    mutex_unlock(&flashImageMutex);
}

static int setScratchPad(char *buf, int len);
static char *getScratchPad(int len);
static int nandNvramSet(const char *nvramString );

static void kerSysNvRamMirrorWrite(const PNVRAM_DATA nv);
static int nandNvramSetEx(const char *nvramString, int block, int offset);

// Variables not used in the simplified API used for the IKOS target
#if !defined(CONFIG_BRCM_IKOS)
static char bootCfeVersion[CFE_VERSION_MARK_SIZE+CFE_VERSION_SIZE];
#endif

#define ALLOC_TYPE_KMALLOC   0
#define ALLOC_TYPE_VMALLOC   1

void *retriedKmalloc(size_t size)
{
    void *pBuf;
    unsigned char *bufp8 ;

    size += 4 ; /* 4 bytes are used to store the housekeeping information used for freeing */

    // Memory allocation changed from kmalloc() to vmalloc() as the latter is not susceptible to memory fragmentation under low memory conditions
    // We have modified Linux VM to search all pages by default, it is no longer necessary to retry here
    if (!in_interrupt() ) {
        pBuf = vmalloc(size);
        if (pBuf) {
            memset(pBuf, 0, size);
            bufp8 = (unsigned char *) pBuf ;
            *bufp8 = ALLOC_TYPE_VMALLOC ;
            pBuf = bufp8 + 4 ;
        }
    }
    else { // kmalloc is still needed if in interrupt
        pBuf = kmalloc(size, GFP_ATOMIC);
        if (pBuf) {
            memset(pBuf, 0, size);
            bufp8 = (unsigned char *) pBuf ;
            *bufp8 = ALLOC_TYPE_KMALLOC ;
            pBuf = bufp8 + 4 ;
        }
    }

    return pBuf;
}

void retriedKfree(void *pBuf)
{
    unsigned char *bufp8  = (unsigned char *) pBuf ;
    bufp8 -= 4 ;

    if (*bufp8 == ALLOC_TYPE_KMALLOC)
        kfree(bufp8);
    else
        vfree(bufp8);
}

// get shared blks into *** pTempBuf *** which has to be released bye the caller!
// return: if pTempBuf != NULL, poits to the data with the dataSize of the buffer
// !NULL -- ok
// NULL  -- fail
char *getSharedBlks(int start_blk, int num_blks)
{
    int i = 0;
    int usedBlkSize = 0;
    int sect_size = 0;
    char *pTempBuf = NULL;
    char *pBuf = NULL;

    down(&semflash);

    for (i = start_blk; i < (start_blk + num_blks); i++)
        usedBlkSize += flash_get_sector_size((unsigned short) i);

    if ((pTempBuf = (char *) retriedKmalloc(usedBlkSize)) == NULL)
    {
        printk("failed to allocate memory with size: %d\n", usedBlkSize);
        up(&semflash);
        return pTempBuf;
    }
    
    pBuf = pTempBuf;
    for (i = start_blk; i < (start_blk + num_blks); i++)
    {
        sect_size = flash_get_sector_size((unsigned short) i);

#if defined(DEBUG_FLASH)
        printk("getSharedBlks: blk=%d, sect_size=%d\n", i, sect_size);
#endif
        flash_read_buf((unsigned short)i, 0, pBuf, sect_size);
        pBuf += sect_size;
    }
    up(&semflash);
    
    return pTempBuf;
}

// Set the pTempBuf to flash from start_blk for num_blks
// return:
// 0 -- ok
// -1 -- fail
int setSharedBlks(int start_blk, int num_blks, char *pTempBuf)
{
    int i = 0;
    int sect_size = 0;
    int sts = 0;
    char *pBuf = pTempBuf;

    down(&semflash);

    for (i = start_blk; i < (start_blk + num_blks); i++)
    {
        sect_size = flash_get_sector_size((unsigned short) i);
        flash_sector_erase_int(i);

        if (flash_write_buf(i, 0, pBuf, sect_size) != sect_size)
        {
            printk("Error writing flash sector %d.", i);
            sts = -1;
            break;
        }

#if defined(DEBUG_FLASH)
        printk("setShareBlks: blk=%d, sect_size=%d\n", i, sect_size);
#endif

        pBuf += sect_size;
    }

    up(&semflash);

    return sts;
}

#if !defined(CONFIG_BRCM_IKOS)


#ifdef CONFIG_OF
#define BRCM_NVRAM_PROP "brcm_nvram"
#define BRCM_CFEVER_PROP "brcm_cfever"
extern int __init bcm_get_root_propdata( const char * prop_name, char * data, int prop_size);
#endif

static void update_spi_flash_rootfs_info(int flash_type)
{
    if( (flash_type == FLASH_IFC_SPI) 
             || (flash_type == FLASH_IFC_HS_SPI)
             || (flash_type == FLASH_IFC_SPINAND))
    {
                fInfo.flash_rootfs_start_offset = flash_get_sector_size(0);
#if defined (CONFIG_BCM96838)
                if( fInfo.flash_rootfs_start_offset < PSRAM_SIZE )
                    fInfo.flash_rootfs_start_offset = PSRAM_SIZE;
#elif defined(CONFIG_BCM96848)
                fInfo.flash_rootfs_start_offset = 192*1024;
#else
                if( fInfo.flash_rootfs_start_offset < FLASH_LENGTH_BOOT_ROM )
                {
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
                    if (bcm_otp_is_btrm_boot())
                    fInfo.flash_rootfs_start_offset = FLASH_LENGTH_SECURE_BOOT_ROM;
                    else
#endif
                    fInfo.flash_rootfs_start_offset = FLASH_LENGTH_BOOT_ROM;
                }
#endif
                fInfo.flash_rootfs_start_offset += IMAGE_OFFSET;



       }
}

// Initialize the flash and fill out the fInfo structure
void __init kerSysEarlyFlashInit( void )
{
#ifdef CONFIG_BCM_ASSERTS
    // ASSERTS and bug() may be too unfriendly this early in the bootup
    // sequence, so just check manually
    if (sizeof(NVRAM_DATA) != NVRAM_LENGTH)
        printk("kerSysEarlyFlashInit: nvram size mismatch! "
               "NVRAM_LENGTH=%d sizeof(NVRAM_DATA)=%lu\n",
               NVRAM_LENGTH, (unsigned long)sizeof(NVRAM_DATA));
#endif
    inMemNvramData_buf = (unsigned char *) &inMemNvramData;
    memset(inMemNvramData_buf, UNINITIALIZED_FLASH_DATA_CHAR, NVRAM_LENGTH);
    memset((unsigned char *)&bootCfeVersion, 0, sizeof(bootCfeVersion));

    flash_init();

    /* Get flash type detected by flash API */
    flash_type = flash_get_flash_type();
    
#ifdef CONFIG_OF
    /* Try and retrieve NVRAM data from dtb */
    if( bcm_get_root_propdata(BRCM_NVRAM_PROP, (char*)&inMemNvramData, sizeof(NVRAM_DATA)) == 0 )
    {
        printk("Successfully retrieved NVRAM data from dtb\n");

        memcpy(cfe_boot, BOOTLOADER_CFE_STR, ( (strlen(BOOTLOADER_CFE_STR)>sizeof(cfe_boot))? sizeof(cfe_boot):strlen(BOOTLOADER_CFE_STR) ) );

        /* Try and retrieve cfe version string from dtb */
        if( bcm_get_root_propdata(BRCM_CFEVER_PROP, (char*)&bootCfeVersion, sizeof(bootCfeVersion)) )
            printk("Failed to retrieve boot CFE version from dtb!\n");
        else
        {
            printk("%s: %d.%d.%d-%d.%d\n",
                                    bootCfeVersion,
                                    bootCfeVersion[CFE_VERSION_MARK_SIZE+0],
                                    bootCfeVersion[CFE_VERSION_MARK_SIZE+1],
                                    bootCfeVersion[CFE_VERSION_MARK_SIZE+2],
                                    bootCfeVersion[CFE_VERSION_MARK_SIZE+3],
                                    bootCfeVersion[CFE_VERSION_MARK_SIZE+4]);
        }
    }
    else
#endif
    {
        /* We should never get here, NVRAM should be present in dtb */
         printk("\n*** ERROR: Board is not configured properly! Unable to load initial NVRAM data ***\n\n");
    }

    /* we still need to get the rootfs information by reading the flash device */
    if( (flash_type == FLASH_IFC_SPI)
           || (flash_type == FLASH_IFC_HS_SPI)
           || (flash_type == FLASH_IFC_SPINAND))
        update_spi_flash_rootfs_info(flash_type);


#if defined(DEBUG_FLASH)
    printk("reading nvram into inMemNvramData\n");
    printk("ulPsiSize 0x%x\n", (unsigned int)inMemNvramData.ulPsiSize);
    printk("backupPsi 0x%x\n", (unsigned int)inMemNvramData.backupPsi);
    printk("ulSyslogSize 0x%x\n", (unsigned int)inMemNvramData.ulSyslogSize);
#endif

    if ((BpSetBoardId(inMemNvramData.szBoardId) != BP_SUCCESS))
        printk("\n*** Board is not initialized properly ***\n\n");
#if defined(CONFIG_BCM_VOICE_SUPPORT)
    if ((BpSetVoiceBoardId(inMemNvramData.szVoiceBoardId) != BP_SUCCESS))
        printk("\n*** Voice Board id is not initialized properly ***\n\n");

#if defined(OTP_GET_USER_BIT) && defined(OTP_DECT_DISABLE)
    //if DECT is OTP'd out, disable it and save
    if(OTP_GET_USER_BIT(OTP_DECT_DISABLE))
       BpSetDectPopulatedData(0);
    else
#endif
    if(BpGetVoiceDectType(inMemNvramData.szBoardId) != BP_VOICE_NO_DECT) 
       BpSetDectPopulatedData((int)(inMemNvramData.ulBoardStuffOption & DECT_SUPPORT_MASK));
#endif /* CONFIG_BCM_VOICE_SUPPORT */

}

/***********************************************************************
 * Function Name: kerSysCfeVersionGet
 * Description  : Get CFE Version.
 * Returns      : 1 -- ok, 0 -- fail
 ***********************************************************************/
int kerSysCfeVersionGet(char *string, int stringLength)
{
    memcpy(string, (unsigned char *)&bootCfeVersion, stringLength);
    return(0);
}

/****************************************************************************
 * NVRAM functions
 * NVRAM data could share a sector with the CFE.  So a write to NVRAM
 * data is actually a read/modify/write operation on a sector.  Protected
 * by a higher level mutex, flashImageMutex.
 * Nvram data is cached in memory in a variable called inMemNvramData, so
 * writes will update this variable and reads just read from this variable.
 ****************************************************************************/

#define EMMC_NVRAM_DEV_NAME     "/dev/nvram"

/** set nvram data
 * Must be called with flashImageMutex held
 *
 * @return 0 on success, -1 on failure.
 */
int kerSysNvRamSet(const char *string, int strLen, int offset)
{
    int sts = -1;  // initialize to failure
    char *pBuf = NULL;

    if(offset != 0) {
        printk("************************************\n");
        printk("INFO: THIS API will be deprecated\n");
        printk("Consider using eNvramGet/eNvramSet calls\n");
        printk("kerSysNvRamSet %d %d\n", offset, strLen);
        printk("************************************\n");
        //dump_stack();
    }

    BCM_ASSERT_HAS_MUTEX_C(&flashImageMutex);
    BCM_ASSERT_R(offset+strLen <= NVRAM_LENGTH, sts);

    switch (flash_type) 
    {
        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if ((pBuf = getSharedBlks(NVRAM_SECTOR, 1)) == NULL)
                return sts;

            // set string to the memory buffer
            memcpy((pBuf + NVRAM_DATA_OFFSET + offset), string, strLen);

            sts = setSharedBlks(NVRAM_SECTOR, 1, pBuf);

            retriedKfree(pBuf);       
            break;

        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            sts = nandNvramSet(string);
            kerSysNvRamMirrorWrite((const PNVRAM_DATA)string);
            break;

        case FLASH_IFC_UNSUP_EMMC:
            sts =  kerSysFsFileSet(EMMC_NVRAM_DEV_NAME, (char*)string, strLen);
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }

    if (0 == sts)
    {
        // write to flash was OK, now update in-memory copy
        updateInMemNvramData((unsigned char *) string, strLen, offset);
    }

    return sts;
}


/** get nvram data
 *
 * since it reads from in-memory copy of the nvram data, always successful.
 */
void kerSysNvRamGet(char *string, int strLen, int offset)
{

    if(offset != 0) {
        printk("************************************\n");
        printk("INFO: THIS API will be deprecated\n");
        printk("Consider using eNvramGet/eNvramSet calls\n");
        printk("kerSysNvRamGet %d %d\n", offset, strLen);
        printk("************************************\n");
        //dump_stack();
    }
    mutex_lock(&inMemNvramData_mutex);
    memcpy(string, inMemNvramData_buf + offset, strLen);
    mutex_unlock(&inMemNvramData_mutex);

    return;
}

#define MAX_NVRAM_MIRROR_LOOKUP_OFFSET 20*1024*1024
/* bcm_ubi requires mapping getCrc32 to our local impl (in board_image.c) */
#define getCrc32 genCrc32
#include "bcm_ubi.h"


static int kerSysNvRamMirrorSearchNext(void * mtd_ptr, unsigned int *offset, PNVRAM_DATA nv)
{
    uint32_t crc=0; 
    int search_complete=0;
    char buff[256];
    size_t len_read=0;
    struct mtd_info * mtd = (struct mtd_info * ) mtd_ptr;


        for(;search_complete == 0 && *offset < MAX_NVRAM_MIRROR_LOOKUP_OFFSET;*offset += 2048) {
            memset(buff, '\0', sizeof(buff));
            if(mtd_read(mtd, *offset,
                sizeof(buff), &len_read, buff) == 0) {
                if(*offset%mtd->erasesize == 0) {
                    if(check_jffs_ubi_magic(buff) == 1) {
                        printk("NVRAM_MIRROR SCAN: OFFSET blk [%x] \n",  *offset);
                        search_complete = 2;
                        continue;
                    } 
                } 
                //check nvram data signature
                if(strncmp((const char *)buff, NVRAM_DATA_SIGN, strlen(NVRAM_DATA_SIGN)) == 0) {
                    //check nvram crc
                    printk("NVRAM_MIRROR SCAN: NVRAM back up found at address %x \n", *offset);
                    if(mtd_read(mtd,*offset+strlen(NVRAM_DATA_SIGN),
                        sizeof(NVRAM_DATA), &len_read, (char*)nv) == 0)
                    crc=nv->ulCheckSum;
                    nv->ulCheckSum = 0;  
                    if (crc == (nv->ulCheckSum=crc32(CRC32_INIT_VALUE,(unsigned char *)nv, sizeof(NVRAM_DATA)))) {
                          search_complete=1;
                          break;
                    }
                nv->ulCheckSum=crc;
            }
        }
    }
    return search_complete;
}

static void kerSysNvRamMirrorSearch(void * mtd_ptr, PNVRAM_DATA nv)
{
    unsigned int offset=0;
    int ret=0;

    do {
        ret=kerSysNvRamMirrorSearchNext(mtd_ptr, &offset, nv);
    }while(ret == 0 && offset < MAX_NVRAM_MIRROR_LOOKUP_OFFSET);
}
static void kerSysNvRamMirrorWrite(PNVRAM_DATA nv)
{
    unsigned int offset=0;
    int ret=0;
    PNVRAM_DATA temp_nv=NULL;
    struct mtd_info *mtd = get_mtd_device_nm("nvram"); 

    temp_nv=kmalloc(NVRAM_LENGTH, GFP_KERNEL);

    if(temp_nv != NULL) {
        do {
            ret=kerSysNvRamMirrorSearchNext(mtd, &offset, temp_nv);
            if(ret == 1) {
                nandNvramSetEx((const char*)nv, (offset+strlen(NVRAM_DATA_SIGN))/mtd->erasesize, (offset+strlen(NVRAM_DATA_SIGN))%mtd->erasesize);
                offset+=2048; // we need to skip 2048 bytes, as we will hit the same NV backup again
            }
        }while(offset < MAX_NVRAM_MIRROR_LOOKUP_OFFSET);
        kfree(temp_nv);
    }
}


/** load nvram data from mtd device
 */
void kerSysNvRamLoad(void * mtd_ptr)
{
    uint32_t crc;
    size_t retlen = 0;
    struct mtd_info * mtd = mtd_ptr;

    mutex_lock(&inMemNvramData_mutex);

    /* Read the whole cfe rom nand block 0 */
    mtd_read(mtd, (NVRAM_SECTOR * mtd->erasesize) + CFE_VERSION_OFFSET,
        sizeof(bootCfeVersion), &retlen, (unsigned char *)&bootCfeVersion);

    mtd_read(mtd, (NVRAM_SECTOR * mtd->erasesize) + NVRAM_DATA_OFFSET,
        sizeof(NVRAM_DATA), &retlen, inMemNvramData_buf);

    crc = ((PNVRAM_DATA)inMemNvramData_buf)->ulCheckSum;
    ((PNVRAM_DATA)inMemNvramData_buf)->ulCheckSum = 0;
    if( crc  != crc32(CRC32_INIT_VALUE, inMemNvramData_buf, sizeof(NVRAM_DATA))) {
        kerSysNvRamMirrorSearch(mtd, (PNVRAM_DATA)inMemNvramData_buf);
    }
    else
       ((PNVRAM_DATA)inMemNvramData_buf)->ulCheckSum = crc;
    mutex_unlock(&inMemNvramData_mutex);


    return;
}


/** Erase entire nvram area.
 *
 * Currently there are no callers of this function.  THe return value is
 * the opposite of kerSysNvramSet.  Kept this way for compatibility.
 *
 * @return 0 on failure, 1 on success.
 */
int kerSysEraseNvRam(void)
{
    int sts = 1;
    char *tempStorage;

    BCM_ASSERT_NOT_HAS_MUTEX_C(&flashImageMutex);

    switch (flash_type) 
    {
        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if (NULL == (tempStorage = kmalloc(NVRAM_LENGTH, GFP_KERNEL)))
            {
                sts = 0;
            }
            else
            {
                // just write the whole buf with '0xff' to the flash
                memset(tempStorage, UNINITIALIZED_FLASH_DATA_CHAR, NVRAM_LENGTH);
                mutex_lock(&flashImageMutex);
                if (kerSysNvRamSet(tempStorage, NVRAM_LENGTH, 0) != 0)
                    sts = 0;
                mutex_unlock(&flashImageMutex);
                kfree(tempStorage);
            }
            break;

        case FLASH_IFC_UNSUP_EMMC:
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            printk("kerSysEraseNvram: not supported when booting from nand\n");
            sts = 0;
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }

    return sts;
}

#else // CONFIG_BRCM_IKOS
static NVRAM_DATA ikos_nvram_data =
    {
    NVRAM_VERSION_NUMBER,
    "",
    "ikos",
    0,
    DEFAULT_PSI_SIZE,
    11,
    {0x02, 0x10, 0x18, 0x01, 0x00, 0x01},
    0x00, 0x00,
    0x720c9f60
    };

void kerSysEarlyFlashInit( void )
{
    inMemNvramData_buf = (unsigned char *) &inMemNvramData;
    memset(inMemNvramData_buf, UNINITIALIZED_FLASH_DATA_CHAR, NVRAM_LENGTH);

    memcpy(inMemNvramData_buf, (unsigned char *)&ikos_nvram_data,
        sizeof (NVRAM_DATA));
    fInfo.flash_scratch_pad_length = 0;
    fInfo.flash_persistent_start_blk = 0;
}

int kerSysCfeVersionGet(char *string, int stringLength)
{
    *string = '\0';
    return(0);
}

void kerSysNvRamGet(char *string, int strLen, int offset)
{
    memcpy(string, (unsigned char *) &ikos_nvram_data, sizeof(NVRAM_DATA));
}

int kerSysNvRamSet(const char *string, int strLen, int offset)
{
    if ((flash_type == FLASH_IFC_NAND) || (flash_type == FLASH_IFC_SPINAND))
        nandNvramSet(string);
    return(0);
}

int kerSysEraseNvRam(void)
{
    return(0);
}

unsigned long kerSysReadFromFlash( void *toaddr, unsigned long fromaddr,
    unsigned long len )
{
    return((unsigned long) memcpy((unsigned char *) toaddr, (unsigned char *) fromaddr, len));
}
#endif  // CONFIG_BRCM_IKOS

/** Update the in-Memory copy of the nvram with the given data.
 *
 * @data: pointer to new nvram data
 * @len: number of valid bytes in nvram data
 * @offset: offset of the given data in the nvram data
 */
void updateInMemNvramData(const unsigned char *data, int len, int offset)
{

    mutex_lock(&inMemNvramData_mutex);
    memcpy(inMemNvramData_buf + offset, data, len);
    mutex_unlock(&inMemNvramData_mutex);
}

/** Get the bootline string from the NVRAM data.
 * Assumes the caller has the inMemNvramData locked.
 * Special case: this is called from prom.c without acquiring the
 * spinlock.  It is too early in the bootup sequence for spinlocks.
 *
 * @param bootline (OUT) a buffer of NVRAM_BOOTLINE_LEN bytes for the result
 */
void kerSysNvRamGetBootlineLocked(char *bootline)
{
    eNvramGet(NVRAM_SZBOOTLINE,bootline, NVRAM_BOOTLINE_LEN);
}
EXPORT_SYMBOL(kerSysNvRamGetBootlineLocked);


/** Get the bootline string from the NVRAM data.
 *
 * @param bootline (OUT) a buffer of NVRAM_BOOTLINE_LEN bytes for the result
 */
void kerSysNvRamGetBootline(char *bootline)
{

    mutex_lock(&inMemNvramData_mutex);
    kerSysNvRamGetBootlineLocked(bootline);
    mutex_unlock(&inMemNvramData_mutex);
}
EXPORT_SYMBOL(kerSysNvRamGetBootline);


/** Get the BoardId string from the NVRAM data.
 * Assumes the caller has the inMemNvramData locked.
 * Special case: this is called from prom_init without acquiring the
 * spinlock.  It is too early in the bootup sequence for spinlocks.
 *
 * @param boardId (OUT) a buffer of NVRAM_BOARD_ID_STRING_LEN
 */
void kerSysNvRamGetBoardIdLocked(char *boardId)
{
    eNvramGet(NVRAM_SZBOARDID,boardId, NVRAM_BOARD_ID_STRING_LEN);
}
EXPORT_SYMBOL(kerSysNvRamGetBoardIdLocked);


/** Get the BoardId string from the NVRAM data.
 *
 * @param boardId (OUT) a buffer of NVRAM_BOARD_ID_STRING_LEN
 */
void kerSysNvRamGetBoardId(char *boardId)
{

    mutex_lock(&inMemNvramData_mutex);
    kerSysNvRamGetBoardIdLocked(boardId);
    mutex_unlock(&inMemNvramData_mutex);
}
EXPORT_SYMBOL(kerSysNvRamGetBoardId);

#ifdef CRASHLOG
#include <linux/kmsg_dump.h>
static struct kmsg_dumper flash_oops_dump;
static void flash_oops_notify_register(void);
#endif /* CRASHLOG */

void kerSysFlashInit( void )
{
    sema_init(&semflash, 1);

    // too early in bootup sequence to acquire spinlock, not needed anyways
    // only the kernel is running at this point
    flash_init_info(&inMemNvramData, &fInfo);

#ifdef CRASHLOG
    memset(&flash_oops_dump, 0, sizeof(struct kmsg_dumper));
    flash_oops_notify_register();
#endif /* CRASHLOG */
}

/***********************************************************************
 * Function Name: kerSysFlashAddrInfoGet
 * Description  : Fills in a structure with information about the NVRAM
 *                and persistent storage sections of flash memory.  
 *                Fro physmap.c to mount the fs vol.
 * Returns      : None.
 ***********************************************************************/
void kerSysFlashAddrInfoGet(PFLASH_ADDR_INFO pflash_addr_info)
{
    memcpy(pflash_addr_info, &fInfo, sizeof(FLASH_ADDR_INFO));
}

/*******************************************************************************
 * PSI functions
 * PSI is where we store the config file.  There is also a "backup" PSI
 * that stores an extra copy of the PSI.  THe idea is if the power goes out
 * while we are writing the primary PSI, the backup PSI will still have
 * a good copy from the last write.  No additional locking is required at
 * this level.
 *******************************************************************************/
#define PSI_FILE_NAME           "/data/psi"
#define PSI_BACKUP_FILE_NAME    "/data/psibackup"
#define SCRATCH_PAD_FILE_NAME   "/data/scratchpad"


// get psi data
// return:
//  0 - ok
//  -1 - fail
int kerSysPersistentGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;
    int ret = -1;

    switch (flash_type) 
    {
        case FLASH_IFC_UNSUP_EMMC:
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            ret = kerSysFsFileGet(PSI_FILE_NAME, string, strLen);
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if ((pBuf = getSharedBlks(fInfo.flash_persistent_start_blk,
                fInfo.flash_persistent_number_blk)) == NULL)
                break;

            // get string off the memory buffer
            memcpy(string, (pBuf + fInfo.flash_persistent_blk_offset + offset), strLen);
            retriedKfree(pBuf);
            ret = 0;
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }

    return ret;
}

int kerSysBackupPsiGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;
    int ret = -1;

    switch (flash_type) 
    {
        case FLASH_IFC_UNSUP_EMMC:
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            ret = kerSysFsFileGet(PSI_BACKUP_FILE_NAME, string, strLen);
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if (fInfo.flash_backup_psi_number_blk <= 0)
            {
                printk("No backup psi blks allocated, change it in CFE\n");
            }

            if (fInfo.flash_persistent_start_blk == 0)
                break;

            if ((pBuf = getSharedBlks(fInfo.flash_backup_psi_start_blk,
                                      fInfo.flash_backup_psi_number_blk)) == NULL)
                break;

            // get string off the memory buffer
            memcpy(string, (pBuf + offset), strLen);
            retriedKfree(pBuf);
            ret = 0;
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
        break;
    }

    return ret;
}

int kerSysFsFileSet(const char *filename, char *buf, int len)
{
    int ret = -1;
    struct file *fp;
    mm_segment_t fs;

    if( in_atomic() )
    {
        printk("Error: %s called in atomic context. FS r/w calls may block!\n",
            __FUNCTION__ );
        dump_stack();
        return ret;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);

    fp = filp_open(filename, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);

    if (!IS_ERR(fp))
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 00)
        if (fp->f_op && fp->f_op->write)
#endif
        {
            fp->f_pos = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 00)
            if((int) fp->f_op->write(fp, (void *) buf, len, &fp->f_pos) == len)
#else
            if((int) vfs_write(fp, (void *) buf, len, &fp->f_pos) == len)
#endif
            {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)                
                vfs_fsync(fp, 0);
#else
                vfs_fsync(fp, fp->f_path.dentry, 0);
#endif
                ret = 0;
            }
        }

        filp_close(fp, NULL);
    }

    set_fs(fs);

    if (ret != 0)
    {
        printk("Failed to write to '%s'.\n", filename);
    }

    return( ret );
}

int kerSysFsFileGet(const char *filename, char *buf, int len)
{
    int ret = -1;
    struct file *fp;
    mm_segment_t fs;

    if( in_atomic() )
    {
        printk("Error: %s called in atomic context. FS r/w calls may block!\n",
            __FUNCTION__ );
        dump_stack();
        return ret;
    }

    memset(buf, 0x00, len);
    fp = filp_open(filename, O_RDONLY| O_NONBLOCK, 0);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 00)
    if (!IS_ERR(fp) && fp->f_op && fp->f_op->read)
#else
    if (!IS_ERR(fp))
#endif
    {
        fs = get_fs();
        set_fs(KERNEL_DS);

        fp->f_pos = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 00)
        if((int) fp->f_op->read(fp, (void *) buf, len,
           &fp->f_pos) <= 0)
#else
        if((int) vfs_read(fp, (void *) buf, len,
           &fp->f_pos) <= 0)
#endif
        {
            printk("Failed to get data from '%s'\n", filename);
        }
        else
        {
            ret = 0;
        }

        filp_close(fp, NULL);
        set_fs(fs);
    }

    return ret;
}

// set psi 
// return:
//  0 - ok
//  -1 - fail
int kerSysPersistentSet(char *string, int strLen, int offset)
{
    int sts = 0;
    char *pBuf = NULL;

    switch (flash_type) 
    {
        case FLASH_IFC_UNSUP_EMMC:
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            /* Root file system is on a writable NAND flash.  Write PSI to
             * a file on the NAND flash.
             */
            sts =  kerSysFsFileSet(PSI_FILE_NAME, string, strLen);
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if (disable_nor_raw_partition)
            {
                sts =  kerSysFsFileSet(PSI_FILE_NAME, string, strLen);
                break;
            }

            if ((pBuf = getSharedBlks(fInfo.flash_persistent_start_blk,
                fInfo.flash_persistent_number_blk)) == NULL)
                return -1;

            // set string to the memory buffer
            memcpy((pBuf + fInfo.flash_persistent_blk_offset + offset), string, strLen);

            if (setSharedBlks(fInfo.flash_persistent_start_blk, 
                fInfo.flash_persistent_number_blk, pBuf) != 0)
                sts = -1;
            
            retriedKfree(pBuf);
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }

    return sts;
}

int kerSysBackupPsiSet(char *string, int strLen, int offset)
{
    int i;
    int sts = 0;
    int usedBlkSize = 0;
    char *pBuf = NULL;

    switch (flash_type) 
    {
        case FLASH_IFC_UNSUP_EMMC:
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            /* Root file system is on a writable NAND flash.  Write backup PSI to
             * a file on the NAND flash.
             */
            sts = kerSysFsFileSet(PSI_BACKUP_FILE_NAME, string, strLen);
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if (fInfo.flash_backup_psi_number_blk <= 0)
            {
                printk("No backup psi blks allocated, change it in CFE\n");
                return -1;
            }

            if (fInfo.flash_persistent_start_blk == 0)
                return -1;

            /*
             * The backup PSI does not share its blocks with anybody else, so I don't have
             * to read the flash first.  But now I have to make sure I allocate a buffer
             * big enough to cover all blocks that the backup PSI spans.
             */
            for (i=fInfo.flash_backup_psi_start_blk;
                 i < (fInfo.flash_backup_psi_start_blk + fInfo.flash_backup_psi_number_blk); i++)
            {
               usedBlkSize += flash_get_sector_size((unsigned short) i);
            }

            if ((pBuf = (char *) retriedKmalloc(usedBlkSize)) == NULL)
            {
               printk("failed to allocate memory with size: %d\n", usedBlkSize);
               return -1;
            }

            memset(pBuf, 0, usedBlkSize);

            // set string to the memory buffer
            memcpy((pBuf + offset), string, strLen);

            if (setSharedBlks(fInfo.flash_backup_psi_start_blk, fInfo.flash_backup_psi_number_blk, 
                              pBuf) != 0)
                sts = -1;
            
            retriedKfree(pBuf);
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }

    return sts;
}


/*******************************************************************************
 * "Kernel Syslog" is one or more sectors allocated in the flash
 * so that we can persist crash dump or other system diagnostics info
 * across reboots.  This feature is current not implemented.
 *******************************************************************************/

#define SYSLOG_FILE_NAME        "/etc/syslog"

int kerSysSyslogGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;
    int ret = -1;

    switch (flash_type) 
    {
        case FLASH_IFC_UNSUP_EMMC:
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            ret = kerSysFsFileGet(SYSLOG_FILE_NAME, string, strLen);
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if (fInfo.flash_syslog_number_blk <= 0)
            {
                printk("No syslog blks allocated, change it in CFE\n");
                break;
            }
            
            if (strLen > fInfo.flash_syslog_length)
                break;

            if ((pBuf = getSharedBlks(fInfo.flash_syslog_start_blk,
                                      fInfo.flash_syslog_number_blk)) == NULL)
                break;

            // get string off the memory buffer
            memcpy(string, (pBuf + offset), strLen);
            retriedKfree(pBuf);
            ret = 0;
            break;

        default:
            printk("ERROR: Unknown flash type\n");
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }
    return ret;
}

int kerSysSyslogSet(char *string, int strLen, int offset)
{
    int i;
    int sts = 0;
    int usedBlkSize = 0;
    char *pBuf = NULL;

    switch (flash_type) 
    {
        case FLASH_IFC_UNSUP_EMMC:
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            /* Root file system is on a writable NAND flash.  Write PSI to
             * a file on the NAND flash.
             */
            sts = kerSysFsFileSet(SYSLOG_FILE_NAME, string, strLen);
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if (fInfo.flash_syslog_number_blk <= 0)
            {
                printk("No syslog blks allocated, change it in CFE\n");
                return -1;
            }
            
            if (strLen > fInfo.flash_syslog_length)
                return -1;

            /*
             * The syslog does not share its blocks with anybody else, so I don't have
             * to read the flash first.  But now I have to make sure I allocate a buffer
             * big enough to cover all blocks that the syslog spans.
             */
            for (i=fInfo.flash_syslog_start_blk;
                 i < (fInfo.flash_syslog_start_blk + fInfo.flash_syslog_number_blk); i++)
            {
                usedBlkSize += flash_get_sector_size((unsigned short) i);
            }

            if ((pBuf = (char *) retriedKmalloc(usedBlkSize)) == NULL)
            {
               printk("failed to allocate memory with size: %d\n", usedBlkSize);
               return -1;
            }

            memset(pBuf, 0, usedBlkSize);

            // set string to the memory buffer
            memcpy((pBuf + offset), string, strLen);

            if (setSharedBlks(fInfo.flash_syslog_start_blk, fInfo.flash_syslog_number_blk, pBuf) != 0)
                sts = -1;

            retriedKfree(pBuf);
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }

    return sts;
}

/* Erase the specified NAND flash block. */
static int nandEraseBlk( struct mtd_info *mtd, int blk_addr )
{
    struct erase_info erase;
    int sts;

    /* Erase the flash block. */
    memset(&erase, 0x00, sizeof(erase));
    erase.addr = blk_addr;
    erase.len = mtd->erasesize;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
    erase.mtd = mtd;
#endif

    if( (sts = mtd_block_isbad(mtd, blk_addr)) == 0 )
    {
        sts = mtd_erase(mtd, &erase);

        /* Function local_bh_disable has been called and this
         * is the only operation that should be occurring.
         * Therefore, spin waiting for erase to complete.
         */
        if( 0 == sts )
        {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
            int i;

            for(i = 0; i < 10000 && erase.state != MTD_ERASE_DONE &&
                erase.state != MTD_ERASE_FAILED; i++ )
            {
                udelay(100);
            }

            if( erase.state != MTD_ERASE_DONE )
            {
                sts = -1;
                printk("nandEraseBlk - Block 0x%8.8x. Error erase block timeout.\n", blk_addr);
            }
#endif
        }
        else
            printk("nandEraseBlk - Block 0x%8.8x. Error erasing block.\n", blk_addr);
    }
    else
        printk("nandEraseBlk - Block 0x%8.8x. Bad block.\n", blk_addr);

    return (sts);
}

/* Write data with or without JFFS2 clean marker, must pass function an aligned block address */
static int nandWriteBlk(struct mtd_info *mtd, int blk_addr, int data_len, char *data_ptr, bool write_JFFS2_clean_marker)
{
#ifdef CONFIG_CPU_LITTLE_ENDIAN
    const unsigned short jffs2_clean_marker[] = {JFFS2_MAGIC_BITMASK, JFFS2_NODETYPE_CLEANMARKER, 0x0008, 0x0000};
#else
    const unsigned short jffs2_clean_marker[] = {JFFS2_MAGIC_BITMASK, JFFS2_NODETYPE_CLEANMARKER, 0x0000, 0x0008};
#endif
    struct mtd_oob_ops ops;
    int sts = 0;
    int page_addr, byte;

    for (page_addr = 0; page_addr < data_len; page_addr += mtd->writesize)
    {
        memset(&ops, 0x00, sizeof(ops));

        // check to see if whole page is FFs
        for (byte = 0; (byte < mtd->writesize) && ((page_addr + byte) < data_len); byte += 4)
        {
            if ( *(uint32 *)(data_ptr + page_addr + byte) != 0xFFFFFFFF )
            {
                ops.len = mtd->writesize < (data_len - page_addr) ? mtd->writesize : data_len - page_addr;
                ops.datbuf = data_ptr + page_addr;
                break;
            }
        }

        if (write_JFFS2_clean_marker)
        {
            ops.mode = MTD_OPS_AUTO_OOB;
            ops.oobbuf = (char *)jffs2_clean_marker;
            ops.ooblen = sizeof(jffs2_clean_marker);
            write_JFFS2_clean_marker = 0; // write clean marker to first page only
        }

        if (ops.len || ops.ooblen)
        {
            if( (sts = mtd_write_oob(mtd, blk_addr + page_addr, &ops)) != 0 )
            {
                printk("nandWriteBlk - Block 0x%8.8x. Error writing page.\n", blk_addr + page_addr);
                nandEraseBlk(mtd, blk_addr);
                mtd_block_markbad(mtd, blk_addr);
                break;
            }
        }
    }

    return(sts);
}

 int nandNvramSetEx(const char *nvramString, int block, int offset  )
{
    /* Image contains CFE ROM boot loader. */
    struct mtd_info *mtd = get_mtd_device_nm("nvram"); 
    char *cferom_ptr = NULL;
    size_t retlen = 0;
    
    if( IS_ERR_OR_NULL(mtd) )
        return -1;
    
    if ( (cferom_ptr = (char *)retriedKmalloc(mtd->erasesize)) == NULL )
    {
        printk("\n Failed to allocate memory in nandNvramSet();");
        return -1;
    }
    /* Read the whole cfe rom nand block */
    mtd_read(mtd, (block*mtd->erasesize), mtd->erasesize, &retlen, cferom_ptr);



    /* Copy the nvram string into place */
    memcpy(cferom_ptr + offset, nvramString, sizeof(NVRAM_DATA));

    
    /* Flash the CFE ROM boot loader. */
    if (nandEraseBlk( mtd, (block*mtd->erasesize) ) == 0)
        nandWriteBlk(mtd, (block*mtd->erasesize), mtd->erasesize, cferom_ptr, TRUE);

    retriedKfree(cferom_ptr);
    return 0;
}

#ifdef CRASHLOG
#define CRASHLOG_MAX_SIZE (64*1024)
#define MIN(a,b)        (((a)<(b))?(a):(b))
char crashlog_filename[SYSCTL_CRASHLOG_FILENAME_LEN] = {0};
char crashlog_mtd[SYSCTL_CRASHLOG_MTD_LEN] = {0};
static unsigned char crashLogBuf[CRASHLOG_MAX_SIZE] = {0};
static unsigned int cLogOffset = 0;
/* e.g. misc3 in raw partition*/
extern char crashlog_mtd[10];

/* Commit buffer to flash. Simple try write wrap around buffer.
May took two NAND flash block but to prevent malloc during panic. */
static int crashLogCommit(void)
{
    struct mtd_info *mtd;
    int block = 0, data_offset = 0, tot_mtdblocks;

    mtd = get_mtd_device_nm(crashlog_mtd);
    if (IS_ERR_OR_NULL(mtd)) {
        printk("\n crashLogCommit: Failed to get mtd !\n");
        return -1;
    }

    sprintf(&crashLogBuf[0], "%s", "LOG");
    tot_mtdblocks = mtd->size / mtd->erasesize;
    block = 0;
    data_offset = 0;
    for (; (block < tot_mtdblocks) && (data_offset < cLogOffset); block++) {
        /* skip bad block */
        if (nandEraseBlk(mtd, (block * mtd->erasesize)) == 0)  {
            nandWriteBlk(mtd, (block * mtd->erasesize), mtd->erasesize, &crashLogBuf[data_offset], FALSE);
            data_offset += mtd->erasesize;
        }
    }

    printk("crashLogCommit: write %d blocks  data_offset %d \n", block, data_offset);

    return 0;
}
/* Read from nand flash and save to file */
int crashFileSet(const char* filename)
{
    struct mtd_info *mtd;
    char *pbuffer = NULL;
    size_t retlen = 0, size = CRASHLOG_MAX_SIZE;
    int ret;
    int i;
    mtd = get_mtd_device_nm(crashlog_mtd);
    if (IS_ERR_OR_NULL(mtd)) {
        printk("\n %s: Failed to get mtd !\n", __FUNCTION__);
        return -1;
    }

    pbuffer = kmalloc(size, GFP_ATOMIC);
    if (pbuffer == NULL) {
        printk("\n %s: fail to allocate memory", __FUNCTION__);
        return -1;
    }
    memset(pbuffer, 0, size);
    ret = mtd_read(mtd, 0, size, &retlen, pbuffer);

    /* override NAND flash erased content */
    for (i = 0; i < CRASHLOG_MAX_SIZE; i++) {
        if (pbuffer[i] == 0xff)
            pbuffer[i] = 0x0;
    }

    if (ret != 0 || retlen <= 4 || pbuffer[0] != 'L' || pbuffer[1] != 'O' || pbuffer[2] != 'G') {
        printk("crashFileSet: log signature invalid ! \n");
	printk("ret %d retlen %ld buff %c %c %c\n",
		ret, retlen, pbuffer[0], pbuffer[1], pbuffer[2]);
        //size = 0; /* zero-length file */
    }
    kerSysFsFileSet(filename, pbuffer, size);
    kfree(pbuffer);
    return 0;
}

static void flash_oops_do_dump(struct kmsg_dumper *dumper, enum kmsg_dump_reason reason)
{
    size_t len = 0;

    if (reason == KMSG_DUMP_OOPS)
        return;

    kmsg_dump_get_buffer(dumper, true, crashLogBuf,
			     CRASHLOG_MAX_SIZE, &len);

    if (len > CRASHLOG_MAX_SIZE)
        len = CRASHLOG_MAX_SIZE;

    cLogOffset = len;
    crashLogCommit();
}

static void flash_oops_notify_register(void)
{
    if (flash_oops_dump.registered)
        return;

    flash_oops_dump.max_reason = KMSG_DUMP_OOPS;
    flash_oops_dump.dump = flash_oops_do_dump;
    kmsg_dump_register(&flash_oops_dump);
}
#endif /* CRASHLOG */

// NAND flash overwrite nvram block.    
// return: 
// 0 - ok
// !0 - the sector number fail to be flashed (should not be 0)

static int nandNvramSet(const char *nvramString)
{
    return nandNvramSetEx(nvramString, 0, NVRAM_DATA_OFFSET);
}

/*******************************************************************************
 * SP functions
 * SP = ScratchPad, one or more sectors in the flash which user apps can
 * store small bits of data referenced by a small tag at the beginning.
 * kerSysScratchPadSet() and kerSysScratchPadCLearAll() must be protected by
 * a mutex because they do read/modify/writes to the flash sector(s).
 * kerSysScratchPadGet() and KerSysScratchPadList() do not need to acquire
 * the mutex, however, I acquire the mutex anyways just to make this interface
 * symmetrical.  High performance and concurrency is not needed on this path.
 *
 *******************************************************************************/

// get scratch pad data into *** pTempBuf *** which has to be released by the
//      caller!
// return: if pTempBuf != NULL, points to the data with the dataSize of the
//      buffer
// !NULL -- ok
// NULL  -- fail
static char *getScratchPad(int len)
{
    /* Root file system is on a writable NAND flash.  Read scratch pad from
     * a file on the NAND flash.
     */
    char *buf = NULL;

    if( (buf = retriedKmalloc(len)) != NULL )
    {
        kerSysFsFileGet(SCRATCH_PAD_FILE_NAME, buf, len);
    }
    else
        printk("Could not allocate scratch pad memory.\n");

    return( buf );
}

// set scratch pad - write the scratch pad file
// return:
// 0 -- ok
// -1 -- fail
static int setScratchPad(char *buf, int len)
{
    return kerSysFsFileSet(SCRATCH_PAD_FILE_NAME, buf, len);
}

/*
 * get list of all keys/tokenID's in the scratch pad.
 * NOTE: memcpy work here -- not using copy_from/to_user
 *
 * return:
 *         greater than 0 means number of bytes copied to tokBuf,
 *         0 means fail,
 *         negative number means provided buffer is not big enough and the
 *         absolute value of the negative number is the number of bytes needed.
 */
int kerSysScratchPadList(char *tokBuf, int bufLen)
{
    PSP_TOKEN pToken = NULL;
    char *pBuf = NULL;
    char *pShareBuf = NULL;
    char *startPtr = NULL;
    int usedLen;
    int tokenNameLen=0;
    int copiedLen=0;
    int needLen=0;
    int sts = 0;

    BCM_ASSERT_NOT_HAS_MUTEX_R(&spMutex, 0);

    mutex_lock(&spMutex);

    switch (flash_type) 
    {
        case FLASH_IFC_UNSUP_EMMC:
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL) {
                mutex_unlock(&spMutex);
                return sts;
            }

            pBuf = pShareBuf;
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if( (pShareBuf = getSharedBlks(fInfo.flash_scratch_pad_start_blk,
                fInfo.flash_scratch_pad_number_blk)) == NULL )
            {
                printk("could not getSharedBlks.\n");
                mutex_unlock(&spMutex);
                return sts;
            }

            // pBuf points to SP buf
            pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
    {
        printk("Scratch pad is not initialized.\n");
        retriedKfree(pShareBuf);
        mutex_unlock(&spMutex);
        return sts;
    }

    // Walk through all the tokens
    usedLen = sizeof(SP_HEADER);
    startPtr = pBuf + sizeof(SP_HEADER);
    pToken = (PSP_TOKEN) startPtr;

    while( isalnum(pToken->tokenName[0]) && isascii(pToken->tokenName[0]) && pToken->tokenLen > 0 &&
           ((usedLen + pToken->tokenLen) <= fInfo.flash_scratch_pad_length))
    {
        tokenNameLen = strlen(pToken->tokenName);
        needLen += tokenNameLen + 1;
        if (needLen <= bufLen)
        {
            strcpy(&tokBuf[copiedLen], pToken->tokenName);
            copiedLen += tokenNameLen + 1;
        }

        usedLen += ((pToken->tokenLen + 0x03) & ~0x03);
        startPtr += sizeof(SP_TOKEN) + ((pToken->tokenLen + 0x03) & ~0x03);
        pToken = (PSP_TOKEN) startPtr;
    }

    if ( needLen > bufLen )
    {
        // User may purposely pass in a 0 length buffer just to get
        // the size, so don't log this as an error.
        sts = needLen * (-1);
    }
    else
    {
        sts = copiedLen;
    }

    retriedKfree(pShareBuf);

    mutex_unlock(&spMutex);

    return sts;
}

/*
 * get sp data.  NOTE: memcpy work here -- not using copy_from/to_user
 * return:
 *         greater than 0 means number of bytes copied to tokBuf,
 *         0 means fail,
 *         negative number means provided buffer is not big enough and the
 *         absolute value of the negative number is the number of bytes needed.
 */
int kerSysScratchPadGet(char *tokenId, char *tokBuf, int bufLen)
{
    PSP_TOKEN pToken = NULL;
    char *pBuf = NULL;
    char *pShareBuf = NULL;
    char *startPtr = NULL;
    int usedLen;
    int sts = 0;

    mutex_lock(&spMutex);

    switch (flash_type) 
    {
        case FLASH_IFC_UNSUP_EMMC:
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL) {
                mutex_unlock(&spMutex);
                return sts;
            }

            pBuf = pShareBuf;
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if (disable_nor_raw_partition)
            {
                if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL) {
                    mutex_unlock(&spMutex);
                    return sts;
                }

                pBuf = pShareBuf;
                break;
            }

            if( (pShareBuf = getSharedBlks(fInfo.flash_scratch_pad_start_blk,
                fInfo.flash_scratch_pad_number_blk)) == NULL )
            {
                printk("could not getSharedBlks.\n");
                mutex_unlock(&spMutex);
                return sts;
            }

            // pBuf points to SP buf
            pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
    {
        printk("Scratch pad is not initialized.\n");
        retriedKfree(pShareBuf);
        mutex_unlock(&spMutex);
        return sts;
    }

    // search for the token
    usedLen = sizeof(SP_HEADER);
    startPtr = pBuf + sizeof(SP_HEADER);
    pToken = (PSP_TOKEN) startPtr;
    while( isalnum(pToken->tokenName[0]) && isascii(pToken->tokenName[0]) && pToken->tokenLen > 0 &&
        pToken->tokenLen < fInfo.flash_scratch_pad_length &&
        usedLen < fInfo.flash_scratch_pad_length )
    {

        if (strncmp(pToken->tokenName, tokenId, TOKEN_NAME_LEN) == 0)
        {
            if ( pToken->tokenLen > bufLen )
            {
               // User may purposely pass in a 0 length buffer just to get
               // the size, so don't log this as an error.
               // printk("The length %d of token %s is greater than buffer len %d.\n", pToken->tokenLen, pToken->tokenName, bufLen);
                sts = pToken->tokenLen * (-1);
            }
            else
            {
                memcpy(tokBuf, startPtr + sizeof(SP_TOKEN), pToken->tokenLen);
                sts = pToken->tokenLen;
            }
            break;
        }

        usedLen += ((pToken->tokenLen + 0x03) & ~0x03);
        startPtr += sizeof(SP_TOKEN) + ((pToken->tokenLen + 0x03) & ~0x03);
        pToken = (PSP_TOKEN) startPtr;
    }

    retriedKfree(pShareBuf);

    mutex_unlock(&spMutex);

    return sts;
}

// set sp.  NOTE: memcpy work here -- not using copy_from/to_user
// return:
//  0 - ok
//  -1 - fail
int kerSysScratchPadSet(char *tokenId, char *tokBuf, int bufLen)
{
    PSP_TOKEN pToken = NULL;
    char *pShareBuf = NULL;
    char *pBuf = NULL;
    SP_HEADER SPHead;
    SP_TOKEN SPToken;
    char *curPtr;
    int sts = -1;
    int tokenNameLen = 0;

    if( bufLen >= fInfo.flash_scratch_pad_length - sizeof(SP_HEADER) -
        sizeof(SP_TOKEN) )
    {
        printk("Scratch pad overflow by %d bytes.  Information not saved.\n",
            bufLen  - fInfo.flash_scratch_pad_length - (int)sizeof(SP_HEADER) -
            (int)sizeof(SP_TOKEN));
        return sts;
    }

    if( !tokenId || !isalnum(tokenId[0]) || !isascii(tokenId[0]) )
    {
        printk("Invalid scratch pad key name. Must start with an ascii letter or number.\n");
        return sts;
    }

    tokenNameLen = strlen(tokenId);
    if( tokenNameLen >= TOKEN_NAME_LEN )
    {
        printk("Token name length %d large than maximum length %d\n", tokenNameLen, TOKEN_NAME_LEN-1);
        return sts;
    }

    mutex_lock(&spMutex);

    switch (flash_type) 
    {
        case FLASH_IFC_UNSUP_EMMC:
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL)
            {
                mutex_unlock(&spMutex);
                return sts;
            }

            pBuf = pShareBuf;
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if (disable_nor_raw_partition)
            {
                if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL)
                {
                    mutex_unlock(&spMutex);
                    return sts;
                }

                pBuf = pShareBuf;
                break;
            }

            if( (pShareBuf = getSharedBlks( fInfo.flash_scratch_pad_start_blk,
                fInfo.flash_scratch_pad_number_blk)) == NULL )
            {
                mutex_unlock(&spMutex);
                return sts;
            }

            // pBuf points to SP buf
            pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }

    // form header info.
    memset((char *)&SPHead, 0, sizeof(SP_HEADER));
    memcpy(SPHead.SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN);
    SPHead.SPVersion = SP_VERSION;

    // form token info.
    memset((char*)&SPToken, 0, sizeof(SP_TOKEN));
    strncpy(SPToken.tokenName, tokenId, tokenNameLen);
    SPToken.tokenLen = bufLen;

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0)
    {
        // new sp, so just flash the token
        printk("No scratch pad found.  Initialize scratch pad...\n");
        memset(pBuf, 0x0, fInfo.flash_scratch_pad_length);
        memcpy(pBuf, (char *)&SPHead, sizeof(SP_HEADER));
        curPtr = pBuf + sizeof(SP_HEADER);
        memcpy(curPtr, (char *)&SPToken, sizeof(SP_TOKEN));
        curPtr += sizeof(SP_TOKEN);
        if( tokBuf )
            memcpy(curPtr, tokBuf, bufLen);
    }
    else  
    {
        int putAtEnd = 1;
        int curLen;
        int usedLen;
        int skipLen;

        /* Calculate the used length. */
        usedLen = sizeof(SP_HEADER);
        curPtr = pBuf + sizeof(SP_HEADER);
        pToken = (PSP_TOKEN) curPtr;
        skipLen = (pToken->tokenLen + 0x03) & ~0x03;
        while( isalnum(pToken->tokenName[0]) && isascii(pToken->tokenName[0]) &&
            strlen(pToken->tokenName) < TOKEN_NAME_LEN &&
            pToken->tokenLen > 0 &&
            pToken->tokenLen < fInfo.flash_scratch_pad_length &&
            usedLen < fInfo.flash_scratch_pad_length )
        {
            usedLen += sizeof(SP_TOKEN) + skipLen;
            curPtr += sizeof(SP_TOKEN) + skipLen;
            pToken = (PSP_TOKEN) curPtr;
            skipLen = (pToken->tokenLen + 0x03) & ~0x03;
        }

        if( usedLen + SPToken.tokenLen + sizeof(SP_TOKEN) >
            fInfo.flash_scratch_pad_length )
        {
            printk("Scratch pad overflow by %d bytes.  Information not saved.\n",
                (usedLen + SPToken.tokenLen + (int)sizeof(SP_TOKEN)) -
                fInfo.flash_scratch_pad_length);
            mutex_unlock(&spMutex);
            return sts;
        }

        curPtr = pBuf + sizeof(SP_HEADER);
        curLen = sizeof(SP_HEADER);
        while( curLen < usedLen )
        {
            pToken = (PSP_TOKEN) curPtr;
            skipLen = (pToken->tokenLen + 0x03) & ~0x03;
            if (strncmp(pToken->tokenName, tokenId, TOKEN_NAME_LEN) == 0)
            {
                // The token id already exists.
                if( tokBuf && pToken->tokenLen == bufLen )
                {
                    // The length of the new data and the existing data is the
                    // same.  Overwrite the existing data.
                    memcpy((curPtr+sizeof(SP_TOKEN)), tokBuf, bufLen);
                    putAtEnd = 0;
                }
                else
                {
                    // The length of the new data and the existing data is
                    // different.  Shift the rest of the scratch pad to this
                    // token's location and put this token's data at the end.
                    char *nextPtr = curPtr + sizeof(SP_TOKEN) + skipLen;
                    int copyLen = usedLen - (curLen+sizeof(SP_TOKEN) + skipLen);
                    memcpy( curPtr, nextPtr, copyLen );
                    memset( curPtr + copyLen, 0x00, 
                        fInfo.flash_scratch_pad_length - (curLen + copyLen) );
                    usedLen -= sizeof(SP_TOKEN) + skipLen;
                }
                break;
            }

            // get next token
            curPtr += sizeof(SP_TOKEN) + skipLen;
            curLen += sizeof(SP_TOKEN) + skipLen;
        } // end while

        if( putAtEnd )
        {
            if( tokBuf )
            {
                memcpy( pBuf + usedLen, &SPToken, sizeof(SP_TOKEN) );
                memcpy( pBuf + usedLen + sizeof(SP_TOKEN), tokBuf, bufLen );
            }
            memcpy( pBuf, &SPHead, sizeof(SP_HEADER) );
        }

    } // else if not new sp

    switch (flash_type) 
    {
        case FLASH_IFC_UNSUP_EMMC:
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            sts = setScratchPad(pShareBuf, fInfo.flash_scratch_pad_length);
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if (disable_nor_raw_partition)
            {
                sts = setScratchPad(pShareBuf, fInfo.flash_scratch_pad_length);
                break;
            }
			
            sts = setSharedBlks(fInfo.flash_scratch_pad_start_blk, 
                fInfo.flash_scratch_pad_number_blk, pShareBuf);
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }
    
    retriedKfree(pShareBuf);
    mutex_unlock(&spMutex);

    return sts;

    
}

EXPORT_SYMBOL(kerSysScratchPadGet);
EXPORT_SYMBOL(kerSysScratchPadSet);

// wipe out the scratchPad
// return:
//  0 - ok
//  -1 - fail
int kerSysScratchPadClearAll(void)
{ 
    int sts = -1;
    char *pShareBuf = NULL;
    int j ;
    int usedBlkSize = 0;

    // printk ("kerSysScratchPadClearAll.... \n") ;
    mutex_lock(&spMutex);

    switch (flash_type) 
    {
        case FLASH_IFC_UNSUP_EMMC:
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL)
            {
                mutex_unlock(&spMutex);
                return sts;
            }

            memset(pShareBuf, 0x00, fInfo.flash_scratch_pad_length);

            setScratchPad(pShareBuf, fInfo.flash_scratch_pad_length);
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if( (pShareBuf = getSharedBlks( fInfo.flash_scratch_pad_start_blk,
                fInfo.flash_scratch_pad_number_blk)) == NULL )
            {
                mutex_unlock(&spMutex);
                return sts;
            }

            if (fInfo.flash_scratch_pad_number_blk == 1)
                memset(pShareBuf + fInfo.flash_scratch_pad_blk_offset, 0x00, fInfo.flash_scratch_pad_length) ;
            else
            {
                for (j = fInfo.flash_scratch_pad_start_blk;
                    j < (fInfo.flash_scratch_pad_start_blk + fInfo.flash_scratch_pad_number_blk);
                    j++)
                {
                    usedBlkSize += flash_get_sector_size((unsigned short) j);
                }

                memset(pShareBuf, 0x00, usedBlkSize) ;
            }

            sts = setSharedBlks(fInfo.flash_scratch_pad_start_blk,    
                fInfo.flash_scratch_pad_number_blk,  pShareBuf);
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }

    retriedKfree(pShareBuf);

    mutex_unlock(&spMutex);

    //printk ("kerSysScratchPadClearAll Done.... \n") ;
    return sts;
}


static unsigned int getPartitionSize(char * dev_name)
{
    // Open file
    unsigned long num_512b_blocks = 0;
    struct file* f = filp_open(dev_name, O_RDONLY, 0);
    // Replace user space with kernel space
    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);
    
    /* Get size of device in 512Bytes blocks */
    f->f_op->unlocked_ioctl(f, BLKGETSIZE, (unsigned long)&num_512b_blocks);
    
    // Restore space
    set_fs(old_fs);
    
    // Close file
    filp_close(f, 0);

    return num_512b_blocks * 512;
}

int kerSysFlashSizeGet(void)
{
    int ret = 0;
    struct mtd_info *mtd;

    switch (flash_type) 
    {
        case FLASH_IFC_UNSUP_EMMC:
            ret += getPartitionSize(EMMC_DEV_PNAME_CFE);
            ret += getPartitionSize(EMMC_DEV_PNAME_BOOTFS(1));
            ret += getPartitionSize(EMMC_DEV_PNAME_ROOTFS(1));
            ret += getPartitionSize(EMMC_DEV_PNAME_MDATA(1,1));
            ret += getPartitionSize(EMMC_DEV_PNAME_MDATA(1,2));
            break;
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:

            if( (mtd = get_mtd_device_nm("image")) != NULL )
            {
                ret = mtd->size;
                put_mtd_device(mtd);
            }
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            ret = flash_get_total_size();
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }

    return ret;
}


/***********************************************************************
 * Function Name: kerSysSetOpticalPowerValues
 * Description  : Saves optical power values to flash that are obtained
 *                during the  manufacturing process. These values are
 *                stored in NVRAM_DATA which should not be erased.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int kerSysSetOpticalPowerValues(UINT16 rxReading, UINT16 rxOffset, 
    UINT16 txReading)
{
    int ret = -1;
    char param[256];

    BCM_ASSERT_NOT_HAS_MUTEX_C(&flashImageMutex);

    mutex_lock(&inMemNvramData_mutex);
    snprintf(param, sizeof(param), "%d", rxReading);
    ret=eNvramSet(NVRAM_OPTICRXPWRREADING, param);
    if(ret < 0) return ret;


    snprintf(param, sizeof(param), "%d", rxOffset);
    ret=eNvramSet(NVRAM_OPTICRXPWROFFSET, param);
    if(ret < 0) return ret;

    snprintf(param, sizeof(param), "%d", txReading);
    ret=eNvramSet(NVRAM_OPTICTXPWRREADING, param);

    sync_nvram_with_flash();
    mutex_unlock(&inMemNvramData_mutex);

    return(ret);

}

/***********************************************************************
 * Function Name: kerSysGetOpticalPowerValues
 * Description  : Retrieves optical power values from flash that were
 *                saved during the manufacturing process.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int kerSysGetOpticalPowerValues(UINT16 *prxReading, UINT16 *prxOffset, 
    UINT16 *ptxReading)
{
    char value[25];
    int ret=0;

    mutex_lock(&inMemNvramData_mutex);

    ret=eNvramGet(NVRAM_OPTICRXPWRREADING, value, sizeof(value));
    if(ret < 0) return ret;
    sscanf(value, "%hi", prxReading);


    ret=eNvramGet(NVRAM_OPTICRXPWROFFSET, value, sizeof(value));
    if(ret < 0) return ret;
    sscanf(value, "%hi", prxOffset);

    ret=eNvramGet(NVRAM_OPTICTXPWRREADING, value, sizeof(value));
    if(ret < 0) return ret;
    sscanf(value, "%hi", ptxReading);

    mutex_unlock(&inMemNvramData_mutex);

    return(0);
}


#if !defined(CONFIG_BRCM_IKOS)

int kerSysEraseFlash(unsigned long eraseaddr, unsigned long len)
{
    int blk;
    int bgnBlk = flash_get_blk(eraseaddr);
    int endBlk = flash_get_blk(eraseaddr + len);
    unsigned long bgnAddr = (unsigned long) flash_get_memptr(bgnBlk);
    unsigned long endAddr = (unsigned long) flash_get_memptr(endBlk);

#ifdef DEBUG_FLASH
    printk("kerSysEraseFlash blk[%d] eraseaddr[0x%08x] len[%lu]\n",
    bgnBlk, (int)eraseaddr, len);
#endif

	/*corner situation, if the erase is the last sector*/
	if (( eraseaddr + len ) >= ((unsigned long)flash_get_memptr(0) + flash_get_total_size()) )
	{
		endAddr = (unsigned long)flash_get_memptr(0)+flash_get_total_size();
		endBlk  = flash_get_numsectors();
	}
	
    if ( bgnAddr != eraseaddr)
    {
       printk("ERROR: kerSysEraseFlash eraseaddr[0x%08x]"
              " != first block start[0x%08x]\n",
              (int)eraseaddr, (int)bgnAddr);
        return (len);
    }

    if ( (endAddr - bgnAddr) != len)
    {
        printk("ERROR: kerSysEraseFlash eraseaddr[0x%08x] + len[%lu]"
               " != last+1 block start[0x%08x]\n",
               (int)eraseaddr, len, (int) endAddr);
        return (len);
    }

    for (blk=bgnBlk; blk<endBlk; blk++)
        flash_sector_erase_int(blk);

    return 0;
}



unsigned long kerSysReadFromFlash( void *toaddr, unsigned long fromaddr,
    unsigned long len )
{
    int blk, offset, bytesRead;
    unsigned long blk_start;
    char * trailbyte = (char*) NULL;
    char val[2];

    blk = flash_get_blk((int)fromaddr); /* sector in which fromaddr falls */
    blk_start = (unsigned long)flash_get_memptr(blk); /* sector start address */
    offset = (int)(fromaddr - blk_start); /* offset into sector */

#ifdef DEBUG_FLASH
    printk("kerSysReadFromFlash blk[%d] fromaddr[0x%08x]\n",
           blk, (int)fromaddr);
#endif

    bytesRead = 0;

        /* cfiflash : hardcoded for bankwidths of 2 bytes. */
    if ( offset & 1 )   /* toaddr is not 2 byte aligned */
    {
        flash_read_buf(blk, offset-1, val, 2);
        *((char*)toaddr) = val[1];

        toaddr = (void*)((char*)toaddr+1);
        fromaddr += 1;
        len -= 1;
        bytesRead = 1;

        /* if len is 0 we could return here, avoid this if */

        /* recompute blk and offset, using new fromaddr */
        blk = flash_get_blk(fromaddr);
        blk_start = (unsigned long)flash_get_memptr(blk);
        offset = (int)(fromaddr - blk_start);
    }

        /* cfiflash : hardcoded for len of bankwidths multiples. */
    if ( len & 1 )
    {
        len -= 1;
        trailbyte = (char *)toaddr + len;
    }

        /* Both len and toaddr will be 2byte aligned */
    if ( len )
    {
       flash_read_buf(blk, offset, toaddr, len);
       bytesRead += len;
    }

        /* write trailing byte */
    if ( trailbyte != (char*) NULL )
    {
        fromaddr += len;
        blk = flash_get_blk(fromaddr);
        blk_start = (unsigned long)flash_get_memptr(blk);
        offset = (int)(fromaddr - blk_start);
        flash_read_buf(blk, offset, val, 2 );
        *trailbyte = val[0];
        bytesRead += 1;
    }

    return( bytesRead );
}

/*
 * Function: kerSysWriteToFlash
 *
 * Description:
 * This function assumes that the area of flash to be written was
 * previously erased. An explicit erase is therfore NOT needed 
 * prior to a write. This function ensures that the offset and len are
 * two byte multiple. [cfiflash hardcoded for bankwidth of 2 byte].
 *
 * Parameters:
 *      toaddr : destination flash memory address
 *      fromaddr: RAM memory address containing data to be written
 *      len : non zero bytes to be written
 * Return:
 *      FAILURE: number of bytes remaining to be written
 *      SUCCESS: 0 (all requested bytes were written)
 */
int kerSysWriteToFlash( unsigned long toaddr,
                        void * fromaddr, unsigned long len)
{
    int blk, offset, size, blk_size, bytesWritten;
    unsigned long blk_start;
    char * trailbyte = (char*) NULL;
    unsigned char val[2];

#ifdef DEBUG_FLASH
    printk("kerSysWriteToFlash flashAddr[0x%08x] fromaddr[0x%08x] len[%lu]\n",
    (int)toaddr, (int)fromaddr, len);
#endif

    blk = flash_get_blk(toaddr);    /* sector in which toaddr falls */
    blk_start = (unsigned long)flash_get_memptr(blk); /* sector start address */
    offset = (int)(toaddr - blk_start); /* offset into sector */

    /* cfiflash : hardcoded for bankwidths of 2 bytes. */
    if ( offset & 1 )   /* toaddr is not 2 byte aligned */
    {
        val[0] = 0xFF; // ignored
        val[1] = *((char *)fromaddr); /* write the first byte */
        bytesWritten = flash_write_buf(blk, offset-1, val, 2);
        if ( bytesWritten != 2 )
        {
#ifdef DEBUG_FLASH
           printk("ERROR kerSysWriteToFlash ... remaining<%lui>\n", len); 
#endif
           return len;
        }

        toaddr += 1;
        fromaddr = (void*)((char*)fromaddr+1);
        len -= 1;

    /* if len is 0 we could return bytesWritten, avoid this if */

    /* recompute blk and offset, using new toaddr */
        blk = flash_get_blk(toaddr);
        blk_start = (unsigned long)flash_get_memptr(blk);
        offset = (int)(toaddr - blk_start);
    }

    /* cfiflash : hardcoded for len of bankwidths multiples. */
    if ( len & 1 )
    {
    /* need to handle trailing byte seperately */
        len -= 1;
        trailbyte = (char *)fromaddr + len;
        toaddr += len;
    }

    /* Both len and toaddr will be 2byte aligned */
    while ( len > 0 )
    {
        blk_size = flash_get_sector_size(blk);
        if (FLASH_API_ERROR == blk_size) {
           return len;
        }
        size = blk_size - offset; /* space available in sector from offset */
        if ( size > len )
            size = len;

        bytesWritten = flash_write_buf(blk, offset, fromaddr, size); 
        if ( bytesWritten !=  size )
        {
#ifdef DEBUG_FLASH
           printk("ERROR kerSysWriteToFlash ... remaining<%lui>\n", 
               (len - bytesWritten + ((trailbyte == (char*)NULL)? 0 : 1)));
#endif
           return (len - bytesWritten + ((trailbyte == (char*)NULL)? 0 : 1));
        }

        fromaddr += size;
        len -= size;

        blk++;      /* Move to the next block */
        offset = 0; /* All further blocks will be written at offset 0 */
    }

    /* write trailing byte */
    if ( trailbyte != (char*) NULL )
    {
        blk = flash_get_blk(toaddr);
        blk_start = (unsigned long)flash_get_memptr(blk);
        offset = (int)(toaddr - blk_start);
        val[0] = *trailbyte; /* trailing byte */
        val[1] = 0xFF; // ignored
        bytesWritten = flash_write_buf(blk, offset, val, 2 );
        if ( bytesWritten != 2 )
        {
#ifdef DEBUG_FLASH
           printk("ERROR kerSysWriteToFlash ... remaining<%d>\n",1);
#endif
           return 1;
        }
    } 

    return len;
}
/*
 * Function: kerSysWriteToFlashREW
 * 
 * Description:
 * This function does not assume that the area of flash to be written was erased.
 * An explicit erase is therfore needed prior to a write.  
 * kerSysWriteToFlashREW uses a sector copy  algorithm. The first and last sectors
 * may need to be first read if they are not fully written. This is needed to
 * avoid the situation that there may be some valid data in the sector that does
 * not get overwritten, and would be erased.
 *
 * Due to run time costs for flash read, optimizations to read only that data
 * that will not be overwritten is introduced.
 *
 * Parameters:
 *  toaddr : destination flash memory address
 *  fromaddr: RAM memory address containing data to be written
 *  len : non zero bytes to be written
 * Return:
 *  FAILURE: number of bytes remaining to be written 
 *  SUCCESS: 0 (all requested bytes were written)
 *
 */
int kerSysWriteToFlashREW( unsigned long toaddr,
                        void * fromaddr, unsigned long len)
{
    int blk, offset, size, blk_size, bytesWritten;
    unsigned long sect_start;
    int mem_sz = 0;
    char * mem_p = (char*)NULL;

#ifdef DEBUG_FLASH
    printk("kerSysWriteToFlashREW flashAddr[0x%08x] fromaddr[0x%08x] len[%lu]\n",
    (int)toaddr, (int)fromaddr, len);
#endif

    blk = flash_get_blk( toaddr );
    sect_start = (unsigned long) flash_get_memptr(blk);
    offset = toaddr - sect_start;

    while ( len > 0 )
    {
        blk_size = flash_get_sector_size(blk);
        size = blk_size - offset; /* space available in sector from offset */

        /* bound size to remaining len in final block */
        if ( size > len )
            size = len;

        /* Entire blk written, no dirty data to read */
        if ( size == blk_size )
        {
            flash_sector_erase_int(blk);

            bytesWritten = flash_write_buf(blk, 0, fromaddr, blk_size);

            if ( bytesWritten != blk_size )
            {
                if ( mem_p != NULL )
                    retriedKfree(mem_p);
                return (len - bytesWritten);    /* FAILURE */
            }
        }
        else
        {
                /* Support for variable sized blocks, paranoia */
            if ( (mem_p != NULL) && (mem_sz < blk_size) )
            {
                retriedKfree(mem_p);    /* free previous temp buffer */
                mem_p = (char*)NULL;
            }

            if ( (mem_p == (char*)NULL)
              && ((mem_p = (char*)retriedKmalloc(blk_size)) == (char*)NULL) )
            {
                printk("\tERROR kerSysWriteToFlashREW fail to allocate memory\n");
                return len;
            }
            else
                mem_sz = blk_size;

            if ( offset ) /* First block */
            {
                if ( (offset + size) == blk_size)
                {
                   flash_read_buf(blk, 0, mem_p, offset);
                }
                else
                {  
                   /*
                    * Potential for future optimization:
                    * Should have read the begining and trailing portions
                    * of the block. If the len written is smaller than some
                    * break even point.
                    * For now read the entire block ... move on ...
                    */
                   flash_read_buf(blk, 0, mem_p, blk_size);
                }
            }
            else
            {
                /* Read the tail of the block which may contain dirty data*/
                flash_read_buf(blk, len, mem_p+len, blk_size-len );
            }

            flash_sector_erase_int(blk);

            memcpy(mem_p+offset, fromaddr, size); /* Rebuild block contents */

            bytesWritten = flash_write_buf(blk, 0, mem_p, blk_size);

            if ( bytesWritten != blk_size )
            {
                if ( mem_p != (char*)NULL )
                    retriedKfree(mem_p);
                return (len + (blk_size - size) - bytesWritten );
            }
        }

        /* take into consideration that size bytes were copied */
        fromaddr += size;
        toaddr += size;
        len -= size;

        blk++;          /* Move to the next block */
        offset = 0;     /* All further blocks will be written at offset 0 */

    }

    if ( mem_p != (char*)NULL )
        retriedKfree(mem_p);

    return ( len );
}

#else //!defined(CONFIG_BRCM_IKOS)
int kerSysEraseFlash(unsigned long eraseaddr, unsigned long len)
{
    return 0;
}

int kerSysWriteToFlash( unsigned long toaddr,
                        void * fromaddr, unsigned long len)
{
    return 0;
}

#endif
