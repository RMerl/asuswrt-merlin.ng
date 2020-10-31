
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
#include <linux/bcm_assert_locks.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <linux/compiler.h>
#include <linux/ctype.h>
#if defined(CONFIG_MTD_NAND)
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include "bcm_crc.c"
#include "bcm_ubi.c"
#define PRINTK(...)
extern bool kerSysIsRootfsSet(void);
#endif

#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <board.h>
#include <bcmTag.h>
#include "flash_api.h"
#include "boardparms.h"
#include "boardparms_voice.h"

#if defined(CONFIG_BCM963381) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
#include "bcm_otp.h"
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
#include "pmc_drv.h"
#endif

#include <linux/fs_struct.h>
//#define DEBUG_FLASH

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM963268)
int bcm_otp_is_btrm_boot(void);
int bcm_otp_is_boot_secure(void);
#endif

extern int kerSysGetSequenceNumber(int);
extern PFILE_TAG kerSysUpdateTagSequenceNumber(int);

/*
 * inMemNvramData an in-memory copy of the nvram data that is in the flash.
 * This in-memory copy is used by NAND.  It is also used by NOR flash code
 * because it does not require a mutex or calls to memory allocation functions
 * which may sleep.  It is kept in sync with the flash copy by
 * updateInMemNvramData.
 */
static unsigned char *inMemNvramData_buf;
static NVRAM_DATA inMemNvramData;
static DEFINE_SPINLOCK(inMemNvramData_spinlock);
static void updateInMemNvramData(const unsigned char *data, int len, int offset);
#define UNINITIALIZED_FLASH_DATA_CHAR  0xff
static FLASH_ADDR_INFO fInfo;
static struct semaphore semflash;

// mutex is preferred over semaphore to provide simple mutual exclusion
// spMutex protects scratch pad writes
static DEFINE_MUTEX(spMutex);
extern struct mutex flashImageMutex;
static int bootFromNand = 0;

static int setScratchPad(char *buf, int len);
static char *getScratchPad(int len);
static int nandNvramSet(const char *nvramString );

// Variables not used in the simplified API used for the IKOS target
#if !defined(CONFIG_BRCM_IKOS)
static char bootCfeVersion[CFE_VERSION_MARK_SIZE+CFE_VERSION_SIZE];
#endif



#define ALLOC_TYPE_KMALLOC   0
#define ALLOC_TYPE_VMALLOC   1

static void *retriedKmalloc(size_t size)
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

static void retriedKfree(void *pBuf)
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
static char *getSharedBlks(int start_blk, int num_blks)
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
static int setSharedBlks(int start_blk, int num_blks, char *pTempBuf)
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
// Initialize the flash and fill out the fInfo structure
void kerSysEarlyFlashInit( void )
{
    int flash_type;
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

    flash_init();

    flash_type = flash_get_flash_type();
    if ((flash_type == FLASH_IFC_NAND) || (flash_type == FLASH_IFC_SPINAND))
        bootFromNand = 1;
    else
        bootFromNand = 0;


    /* Load boot board parameters saved by function bootNandImageFromRootfs
    * in file bcm63xx_ram\bcm63xx_cmd.c, bypassing requirement for early driver support
    */
    kerSysBlParmsGetStr(BOARD_ID_NAME, inMemNvramData.szBoardId, NVRAM_BOARD_ID_STRING_LEN);
    kerSysBlParmsGetStr(VOICE_BOARD_ID_NAME, inMemNvramData.szVoiceBoardId, NVRAM_BOARD_ID_STRING_LEN);
    kerSysBlParmsGetInt(BOARD_STUFF_NAME, &inMemNvramData.ulBoardStuffOption);

#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328) || defined(CONFIG_BCM963268) || defined(CONFIG_BCM96838) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
    if (flash_type == FLASH_IFC_NAND)
    {
        unsigned int bootCfgSave =  NAND->NandNandBootConfig;

        NAND->NandNandBootConfig = NBC_AUTO_DEV_ID_CFG | 0x101;
#if defined(NAC_PREFETCH_EN)
        /* turn off prefetch to ensure the direct access below is reliable */
        NAND->NandAccControl &= ~NAC_PREFETCH_EN;
#endif
        NAND->NandCsNandXor = 1;
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
        memcpy((unsigned char *)&bootCfeVersion, (unsigned char *)
            NANDFLASH_BASE + CFE_VERSION_OFFSET + 8192, sizeof(bootCfeVersion)); // dummy read to force fetch
        memcpy((unsigned char *)&bootCfeVersion, (unsigned char *)
            NANDFLASH_BASE + CFE_VERSION_OFFSET, sizeof(bootCfeVersion));
        memcpy(inMemNvramData_buf, (unsigned char *)
            NANDFLASH_BASE + NVRAM_DATA_OFFSET, sizeof(NVRAM_DATA));
#else
        memcpy((unsigned char *)&bootCfeVersion, (unsigned char *)
            FLASH_BASE + CFE_VERSION_OFFSET + 8192, sizeof(bootCfeVersion)); // dummy read to force fetch
        memcpy((unsigned char *)&bootCfeVersion, (unsigned char *)
            FLASH_BASE + CFE_VERSION_OFFSET, sizeof(bootCfeVersion));
        memcpy(inMemNvramData_buf, (unsigned char *)
            FLASH_BASE + NVRAM_DATA_OFFSET, sizeof(NVRAM_DATA));
#endif
        NAND->NandNandBootConfig = bootCfgSave;
        NAND->NandCsNandXor = 0;
    }
    else
#endif
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

        flash_read_buf (NVRAM_SECTOR, CFE_VERSION_OFFSET,
            (unsigned char *)&bootCfeVersion, sizeof(bootCfeVersion));

        /* Read the flash contents into NVRAM buffer */
        flash_read_buf (NVRAM_SECTOR, NVRAM_DATA_OFFSET,
                        inMemNvramData_buf, sizeof (NVRAM_DATA)) ;
    }

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


/** set nvram data
 * Must be called with flashImageMutex held
 *
 * @return 0 on success, -1 on failure.
 */
int kerSysNvRamSet(const char *string, int strLen, int offset)
{
    int sts = -1;  // initialize to failure
    char *pBuf = NULL;

    BCM_ASSERT_HAS_MUTEX_C(&flashImageMutex);
    BCM_ASSERT_R(offset+strLen <= NVRAM_LENGTH, sts);

    if (bootFromNand == 0)
    {
        if ((pBuf = getSharedBlks(NVRAM_SECTOR, 1)) == NULL)
            return sts;

        // set string to the memory buffer
        memcpy((pBuf + NVRAM_DATA_OFFSET + offset), string, strLen);

        sts = setSharedBlks(NVRAM_SECTOR, 1, pBuf);

        retriedKfree(pBuf);       
    }
    else
    {
        sts = nandNvramSet(string);
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
    unsigned long flags;

    spin_lock_irqsave(&inMemNvramData_spinlock, flags);
    memcpy(string, inMemNvramData_buf + offset, strLen);
    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);

    return;
}

/** load nvram data from mtd device
 */
void kerSysNvRamLoad(void * mtd_ptr)
{
    unsigned long flags;
    size_t retlen = 0;
    struct mtd_info * mtd = mtd_ptr;

    spin_lock_irqsave(&inMemNvramData_spinlock, flags);

    /* Read the whole cfe rom nand block 0 */
    mtd_read(mtd, (NVRAM_SECTOR * mtd->erasesize) + CFE_VERSION_OFFSET,
        sizeof(bootCfeVersion), &retlen, (unsigned char *)&bootCfeVersion);

    mtd_read(mtd, (NVRAM_SECTOR * mtd->erasesize) + NVRAM_DATA_OFFSET,
        sizeof(NVRAM_DATA), &retlen, inMemNvramData_buf);

    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);

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

    BCM_ASSERT_NOT_HAS_MUTEX_C(&flashImageMutex);

    if (bootFromNand == 0)
    {
        char *tempStorage;
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
    }
    else
    {
        printk("kerSysEraseNvram: not supported when bootFromNand == 1\n");
        sts = 0;
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
    if( bootFromNand )
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
    unsigned long flags;

    spin_lock_irqsave(&inMemNvramData_spinlock, flags);
    memcpy(inMemNvramData_buf + offset, data, len);
    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);
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
    memcpy(bootline, inMemNvramData.szBootline,
                     sizeof(inMemNvramData.szBootline));
}
EXPORT_SYMBOL(kerSysNvRamGetBootlineLocked);


/** Get the bootline string from the NVRAM data.
 *
 * @param bootline (OUT) a buffer of NVRAM_BOOTLINE_LEN bytes for the result
 */
void kerSysNvRamGetBootline(char *bootline)
{
    unsigned long flags;

    spin_lock_irqsave(&inMemNvramData_spinlock, flags);
    kerSysNvRamGetBootlineLocked(bootline);
    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);
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
    memcpy(boardId, inMemNvramData.szBoardId,
                    sizeof(inMemNvramData.szBoardId));
}
EXPORT_SYMBOL(kerSysNvRamGetBoardIdLocked);


/** Get the BoardId string from the NVRAM data.
 *
 * @param boardId (OUT) a buffer of NVRAM_BOARD_ID_STRING_LEN
 */
void kerSysNvRamGetBoardId(char *boardId)
{
    unsigned long flags;

    spin_lock_irqsave(&inMemNvramData_spinlock, flags);
    kerSysNvRamGetBoardIdLocked(boardId);
    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);
}
EXPORT_SYMBOL(kerSysNvRamGetBoardId);
void kerSysNvRamGetNoUpdatingFirmwareLocked(unsigned char *noUpdatingFirmware)
{
    memcpy(noUpdatingFirmware, &inMemNvramData.noUpdatingFirmware,
                    sizeof(inMemNvramData.noUpdatingFirmware));
}
EXPORT_SYMBOL(kerSysNvRamGetNoUpdatingFirmwareLocked);

void kerSysNvRamGetNoUpdatingFirmware(unsigned char *noUpdatingFirmware)
{
    unsigned long flags;

    spin_lock_irqsave(&inMemNvramData_spinlock, flags);
    kerSysNvRamGetNoUpdatingFirmwareLocked(noUpdatingFirmware);
    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);
}
EXPORT_SYMBOL(kerSysNvRamGetNoUpdatingFirmware);

/** Get the base mac addr from the NVRAM data.  This is 6 bytes, not
 * a string.
 *
 * @param baseMacAddr (OUT) a buffer of NVRAM_MAC_ADDRESS_LEN
 */
void kerSysNvRamGetBaseMacAddr(unsigned char *baseMacAddr)
{
    unsigned long flags;

    spin_lock_irqsave(&inMemNvramData_spinlock, flags);
    memcpy(baseMacAddr, inMemNvramData.ucaBaseMacAddr,
                        sizeof(inMemNvramData.ucaBaseMacAddr));
    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);
}
EXPORT_SYMBOL(kerSysNvRamGetBaseMacAddr);


/** Get the nvram version from the NVRAM data.
 *
 * @return nvram version number.
 */
unsigned long kerSysNvRamGetVersion(void)
{
    return (inMemNvramData.ulVersion);
}
EXPORT_SYMBOL(kerSysNvRamGetVersion);

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

    if( bootFromNand )
    {
        /* Root file system is on a writable NAND flash.  Read PSI from
         * a file on the NAND flash.
         */
        struct file *fp;
        mm_segment_t fs;
        int len;

        memset(string, 0x00, strLen);
        fp = filp_open(PSI_FILE_NAME, O_RDONLY, 0);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
        if (!IS_ERR(fp) && fp->f_op && fp->f_op->read)
#else
        if (!IS_ERR(fp))
#endif
        {
            fs = get_fs();
            set_fs(get_ds());

            fp->f_pos = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
            if((len = (int) fp->f_op->read(fp, (void *) string, strLen,
               &fp->f_pos)) <= 0)
#else
            if((len = vfs_read(fp, (void *) string, strLen,
               &fp->f_pos)) <= 0)
#endif
                printk("Failed to read psi from '%s'\n", PSI_FILE_NAME);

            filp_close(fp, NULL);
            set_fs(fs);
        }

        return 0;
    }

    if ((pBuf = getSharedBlks(fInfo.flash_persistent_start_blk,
        fInfo.flash_persistent_number_blk)) == NULL)
        return -1;

    // get string off the memory buffer
    memcpy(string, (pBuf + fInfo.flash_persistent_blk_offset + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}

int kerSysBackupPsiGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if( bootFromNand )
    {
        /* Root file system is on a writable NAND flash.  Read backup PSI from
         * a file on the NAND flash.
         */
        struct file *fp;
        mm_segment_t fs;
        int len;

        memset(string, 0x00, strLen);
        fp = filp_open(PSI_BACKUP_FILE_NAME, O_RDONLY, 0);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
        if (!IS_ERR(fp) && fp->f_op && fp->f_op->read)
#else
        if (!IS_ERR(fp))
#endif
        {
            fs = get_fs();
            set_fs(get_ds());

            fp->f_pos = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
            if((len = (int) fp->f_op->read(fp, (void *) string, strLen,
               &fp->f_pos)) <= 0)
#else
            if((len = (int) vfs_read(fp, (void *) string, strLen,
               &fp->f_pos)) <= 0)
#endif
                printk("Failed to read psi from '%s'\n", PSI_BACKUP_FILE_NAME);

            filp_close(fp, NULL);
            set_fs(fs);
        }

        return 0;
    }

    if (fInfo.flash_backup_psi_number_blk <= 0)
    {
        printk("No backup psi blks allocated, change it in CFE\n");
        return -1;
    }

    if (fInfo.flash_persistent_start_blk == 0)
        return -1;

    if ((pBuf = getSharedBlks(fInfo.flash_backup_psi_start_blk,
                              fInfo.flash_backup_psi_number_blk)) == NULL)
        return -1;

    // get string off the memory buffer
    memcpy(string, (pBuf + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}

int kerSysFsFileSet(const char *filename, char *buf, int len)
{
    int ret = -1;
    struct file *fp;
    mm_segment_t fs;

    fs = get_fs();
    set_fs(get_ds());

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

// set psi 
// return:
//  0 - ok
//  -1 - fail
int kerSysPersistentSet(char *string, int strLen, int offset)
{
    int sts = 0;
    char *pBuf = NULL;

    if( bootFromNand )
    {
        /* Root file system is on a writable NAND flash.  Write PSI to
         * a file on the NAND flash.
         */
        return kerSysFsFileSet(PSI_FILE_NAME, string, strLen);
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

    return sts;
}

int kerSysBackupPsiSet(char *string, int strLen, int offset)
{
    int i;
    int sts = 0;
    int usedBlkSize = 0;
    char *pBuf = NULL;

    if( bootFromNand )
    {
        /* Root file system is on a writable NAND flash.  Write backup PSI to
         * a file on the NAND flash.
         */
        return kerSysFsFileSet(PSI_BACKUP_FILE_NAME, string, strLen);
    }

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

    if( bootFromNand )
    {
        /* Root file system is on a writable NAND flash.  Read syslog from
         * a file on the NAND flash.
         */
        struct file *fp;
        mm_segment_t fs;

        memset(string, 0x00, strLen);
        fp = filp_open(SYSLOG_FILE_NAME, O_RDONLY, 0);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 00)
        if (!IS_ERR(fp) && fp->f_op && fp->f_op->read)
#else
        if (!IS_ERR(fp))
#endif
        {
            fs = get_fs();
            set_fs(get_ds());

            fp->f_pos = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 00)
            if((int) fp->f_op->read(fp, (void *) string, strLen,
               &fp->f_pos) <= 0)
#else
            if((int) vfs_read(fp, (void *) string, strLen,
               &fp->f_pos) <= 0)
#endif
                printk("Failed to read psi from '%s'\n", SYSLOG_FILE_NAME);

            filp_close(fp, NULL);
            set_fs(fs);
        }

        return 0;
    }

    if (fInfo.flash_syslog_number_blk <= 0)
    {
        printk("No syslog blks allocated, change it in CFE\n");
        return -1;
    }
    
    if (strLen > fInfo.flash_syslog_length)
        return -1;

    if ((pBuf = getSharedBlks(fInfo.flash_syslog_start_blk,
                              fInfo.flash_syslog_number_blk)) == NULL)
        return -1;

    // get string off the memory buffer
    memcpy(string, (pBuf + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}

int kerSysSyslogSet(char *string, int strLen, int offset)
{
    int i;
    int sts = 0;
    int usedBlkSize = 0;
    char *pBuf = NULL;

    if( bootFromNand )
    {
        /* Root file system is on a writable NAND flash.  Write PSI to
         * a file on the NAND flash.
         */
        return kerSysFsFileSet(PSI_FILE_NAME, string, strLen);
    }

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

    return sts;
}

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM963268)
int bcm_otp_is_btrm_boot(void)
{
    int btrm_boot = (*((uint32_t *)(OTP_BASE + OTP_SHADOW_ADDR_BTRM_ENABLE_CUST_ROW)));
    btrm_boot = (btrm_boot & OTP_CUST_BTRM_BOOT_ENABLE_MASK) >> OTP_CUST_BTRM_BOOT_ENABLE_SHIFT;
    return btrm_boot;
}

int bcm_otp_is_boot_secure(void)
{
    int boot_secure = bcm_otp_is_btrm_boot();
    if (boot_secure)
    {
        /* Bootrom is enabled ... check mid */
        boot_secure = (*((uint32_t *)(OTP_BASE + OTP_SHADOW_ADDR_MARKET_ID_CUST_ROW)));
        boot_secure = (boot_secure & OTP_MFG_MRKTID_OTP_BITS_MASK) >> OTP_MFG_MRKTID_OTP_BITS_SHIFT;
    }
    return boot_secure;
}
#endif


/*******************************************************************************
 * Writing software image to flash operations
 * This procedure should be serialized.  Look for flashImageMutex.
 *******************************************************************************/


#define je16_to_cpu(x) ((x).v16)
#define je32_to_cpu(x) ((x).v32)

/*
 * nandUpdateSeqNum
 * 
 * Read the sequence number from each rootfs partition.  The sequence number is
 * the extension on the cferam file.  Add one to the highest sequence number
 * and change the extenstion of the cferam in the image to be flashed to that
 * number.
 */
static char *nandUpdateSeqNum(unsigned char *imagePtr, int imageSize, int blkLen)
{
    char fname[] = NAND_CFE_RAM_NAME;
    int fname_actual_len = strlen(fname);
    int fname_cmp_len = strlen(fname) - 3; /* last three are digits */
    int seq = -1;
    int seq2 = -1;
    char *ret = NULL;

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
    /* If full secure boot is in play, the CFE RAM file is the encrypted version */
    if (bcm_otp_is_boot_secure())
        strcpy(fname, NAND_CFE_RAM_SECBT_NAME);
#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
    else
    {
       if (bcm_otp_is_boot_mfg_secure())
          strcpy(fname, NAND_CFE_RAM_SECBT_MFG_NAME);
    }
#endif
#endif

    seq = kerSysGetSequenceNumber(1);
    seq2 = kerSysGetSequenceNumber(2);

    /* deal with wrap around case */
    if ((seq == 0 && seq2 == 999) || (seq == 999 && seq2 == 0))
        seq = 0;
    else
        seq = (seq >= seq2) ? seq : seq2;

    if( seq != -1 )
    {
        unsigned char *buf, *p;
        struct jffs2_raw_dirent *pdir;
        unsigned long version = 0;
        int done = 0;

        while (( *(unsigned short *) imagePtr != JFFS2_MAGIC_BITMASK ) && (imageSize > 0))
        {
            imagePtr += blkLen;
            imageSize -= blkLen;
        }

        /* Confirm that we did find a JFFS2_MAGIC_BITMASK. If not, we are done */
        if (imageSize <= 0)
           done = 1;

        /* Increment the new highest sequence number. Add it to the CFE RAM
         * file name.
         */
        seq++;
        if (seq == 1000)
            seq = 0;

        /* Search the image and replace the last three characters of file
         * cferam.000 with the new sequence number.
         */
        for(buf = imagePtr; buf < imagePtr+imageSize && done == 0; buf += blkLen)
        {
            p = buf;
            while( p < buf + blkLen )
            {
                pdir = (struct jffs2_raw_dirent *) p;
                if( je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK )
                {
                    if( je16_to_cpu(pdir->nodetype) == JFFS2_NODETYPE_DIRENT &&
                        fname_actual_len == pdir->nsize &&
                        !memcmp(fname, pdir->name, fname_cmp_len) &&
                        je32_to_cpu(pdir->version) > version &&
                        je32_to_cpu(pdir->ino) != 0 )
                     {
                        /* File cferam.000 found. Change the extension to the
                         * new sequence number and recalculate file name CRC.
                         */
                        p = pdir->name + fname_cmp_len;
                        p[0] = (seq / 100) + '0';
                        p[1] = ((seq % 100) / 10) + '0';
                        p[2] = ((seq % 100) % 10) + '0';
                        p[3] = '\0';

                        je32_to_cpu(pdir->name_crc) =
                            crc32(0, pdir->name, (unsigned int)
                            fname_actual_len);

                        version = je32_to_cpu(pdir->version);

                        /* Setting 'done = 1' assumes there is only one version
                         * of the directory entry.
                         */
                        done = 1;
                        ret = buf;  /* Pointer to the block containing CFERAM directory entry in the image to be flashed */
                        break;
                    }

                    p += (je32_to_cpu(pdir->totlen) + 0x03) & ~0x03;
                }
                else
                {
                    done = 1;
                    break;
                }
            }
        }
    }

    return(ret);
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
    erase.mtd = mtd;

    if( (sts = mtd_block_isbad(mtd, blk_addr)) == 0 )
    {
        sts = mtd_erase(mtd, &erase);

        /* Function local_bh_disable has been called and this
         * is the only operation that should be occurring.
         * Therefore, spin waiting for erase to complete.
         */
        if( 0 == sts )
        {
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

#ifdef CRASHLOG
#define CRASHLOG_MAX_SIZE	(64*1024)
#define MIN(a,b)		(((a)<(b))?(a):(b))
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

// NAND flash bcm image 
// return: 
// 0 - ok
// !0 - the sector number fail to be flashed (should not be 0)
int kerSysBcmNandImageSet( char *rootfs_part, char *image_ptr, int img_size,
    int should_yield )
{
    int sts = -1;
    int blk_addr;
    int old_img = 0;
    char *cferam_ptr;
    int rsrvd_for_cferam;
    char *end_ptr = image_ptr + img_size;
    struct mtd_info *mtd0 = get_mtd_device_nm("image");
    WFI_TAG wt = {0};
    int nvramXferSize;

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
    uint32_t  btrmEnabled = bcm_otp_is_btrm_boot();
#endif

    /* Reserve room to flash block containing directory entry for CFERAM. */
    rsrvd_for_cferam = 8 * mtd0->erasesize;

    if( !IS_ERR_OR_NULL(mtd0) )
    {
        unsigned int chip_id = kerSysGetChipId();
        int blksize = mtd0->erasesize / 1024;

        memcpy(&wt, end_ptr, sizeof(wt));

#if defined(CHIP_FAMILY_ID_HEX)
        chip_id = CHIP_FAMILY_ID_HEX;
#endif

        if( (wt.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
            wt.wfiChipId != chip_id )
        {
            int id_ok = 0;

            if (id_ok == 0) {
                printk("Chip Id error.  Image Chip Id = %04x, Board Chip Id = "
                    "%04x.\n", wt.wfiChipId, chip_id);
                put_mtd_device(mtd0);
                return -1;
            }
        }
        else if( wt.wfiFlashType == WFI_NOR_FLASH )
        {
            printk("\nERROR: Image does not support a NAND flash device.\n\n");
            put_mtd_device(mtd0);
            return -1;
        }
        else if( (wt.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
            ((wt.wfiFlashType < WFI_NANDTYPE_FLASH_MIN && wt.wfiFlashType > WFI_NANDTYPE_FLASH_MAX) ||
              blksize != WFI_NANDTYPE_TO_BKSIZE(wt.wfiFlashType) ) )
        {
            printk("\nERROR: NAND flash block size %dKB does not work with an "
                "image built with %dKB block size\n\n", blksize,WFI_NANDTYPE_TO_BKSIZE(wt.wfiFlashType));
            put_mtd_device(mtd0);
            return -1;
        }
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
        else if (((  (wt.wfiFlags & WFI_FLAG_SUPPORTS_BTRM)) && (! btrmEnabled)) ||
                 ((! (wt.wfiFlags & WFI_FLAG_SUPPORTS_BTRM)) && (  btrmEnabled)))
        {
            printk("ERROR: Secure Boot Conflict. The image type does not match the SoC OTP configuration. Aborting.\n");
            put_mtd_device(mtd0);
            return -1;
        }
#endif
        else
        {
            mtd0 = get_mtd_device_nm(rootfs_part);

            /* If the image version indicates that it uses a 1MB data partition
             * size and the image is intended to be flashed to the second file
             * system partition, change to the flash to the first partition.
             * After new image is flashed, delete the second file system and
             * data partitions (at the bottom of this function).
             */
            if( wt.wfiVersion == WFI_VERSION_NAND_1MB_DATA )
            {
                unsigned long rootfs_ofs;
                kerSysBlParmsGetInt(NAND_RFS_OFS_NAME, (int *) &rootfs_ofs);
                
                if(rootfs_ofs == inMemNvramData.ulNandPartOfsKb[NP_ROOTFS_1] &&
                    mtd0)
                {
                    printk("Old image, flashing image to first partition.\n");
                    put_mtd_device(mtd0);
                    mtd0 = NULL;
                    old_img = 1;
                }
            }

            if( IS_ERR_OR_NULL(mtd0) || mtd0->size == 0LL )
            {
                /* Flash device is configured to use only one file system. */
                if( !IS_ERR_OR_NULL(mtd0) )
                    put_mtd_device(mtd0);
                mtd0 = get_mtd_device_nm("image");
            }
        }
    }

    if( !IS_ERR_OR_NULL(mtd0) )
    {
        int ofs;
        unsigned long flags=0;
        int writelen;
        int writing_ubifs;

        if( *(unsigned short *) image_ptr == JFFS2_MAGIC_BITMASK )
        { /* Downloaded image does not contain CFE ROM boot loader */
            ofs = 0;
        }
        else
        {
            /* Downloaded image contains CFE ROM boot loader. */
            PNVRAM_DATA pnd = (PNVRAM_DATA) (image_ptr + NVRAM_SECTOR*((unsigned int)flash_get_sector_size(0))+NVRAM_DATA_OFFSET);

            ofs = mtd0->erasesize;
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
            /* check if it is zero padded for backward compatiblity */
           if( (wt.wfiFlags&WFI_FLAG_HAS_PMC) == 0 )
           {
               unsigned int *pImg  = (unsigned int*)image_ptr;
               unsigned char * pBuf = image_ptr;
               int block_start, block_end, remain, block;
               struct mtd_info *mtd1;
               size_t readlen;
              
               if( *pImg == 0 && *(pImg+1) == 0 && *(pImg+2) == 0 && *(pImg+3) == 0 )
               {
                   /* the first 64KB are for PMC in 631x8, need to preserve that for cfe/linux image update if it is not for PMC image update. */
                   block_start = 0;
                   block_end = IMAGE_OFFSET/mtd0->erasesize;
                   remain = IMAGE_OFFSET%mtd0->erasesize;

                   mtd1 = get_mtd_device_nm("nvram");
                   if( !IS_ERR_OR_NULL(mtd1) )
                   {
                       for( block = block_start; block < block_end; block++ )
                       {
                           mtd_read(mtd1, block*mtd1->erasesize, mtd1->erasesize, &readlen, pBuf);
                           pBuf += mtd1->erasesize;
                       }

                       if( remain )
                       {
                           block = block_end;
                           mtd_read(mtd1, block*mtd1->erasesize, remain, &readlen, pBuf);
                       }

                       put_mtd_device(mtd1);
                   }
                   else 
                   {
                       printk("Failed to get nvram mtd device\n");
                       put_mtd_device(mtd0);
                       return -1;
                   }
               }
               else
               {
                   printk("Invalid NAND image.No PMC image or padding\n");
                   put_mtd_device(mtd0);
                   return -1;
               }
           }
#endif

            nvramXferSize = sizeof(NVRAM_DATA);
#if defined(CONFIG_BCM963268)
            if ((wt.wfiFlags & WFI_FLAG_SUPPORTS_BTRM) && bcm_otp_is_boot_secure())
            {
               /* Upgrading a secure-boot 63268 SoC. Nvram is 3k. do not preserve the old */
               /* security credentials kept in nvram but rather use the new credentials   */
               /* embedded within the new image (ie the last 2k of the 3k nvram) */
               nvramXferSize = 1024;
            }
#endif

            /* Copy NVRAM data to block to be flashed so it is preserved. */
            spin_lock_irqsave(&inMemNvramData_spinlock, flags);
            memcpy((unsigned char *) pnd, inMemNvramData_buf, nvramXferSize);
            spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);

            /* Recalculate the nvramData CRC. */
            pnd->ulCheckSum = 0;
            pnd->ulCheckSum = crc32(CRC32_INIT_VALUE, pnd, sizeof(NVRAM_DATA));
        }

        /* 
         * Scan downloaded image for cferam.000 directory entry and change file externsion 
         * to cfe.YYY where YYY is the current cfe.XXX + 1. If full secure boot is in play,
         * the file to be updated is secram.000 and not cferam.000
         */
        cferam_ptr = nandUpdateSeqNum((unsigned char *) image_ptr, img_size, mtd0->erasesize);

        if( cferam_ptr == NULL )
        {
            printk("\nERROR: Invalid image. ram.000 not found.\n\n");
            put_mtd_device(mtd0);
            return -1; 
        }

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
        if ((wt.wfiFlags & WFI_FLAG_SUPPORTS_BTRM) && (ofs != 0))
        {
            /* These targets support bootrom boots which is currently enabled. the "nvram" */
            /* mtd device may be bigger than just the first nand block. Check that the new */
            /* image plays nicely with the current partition table settings. */
            struct mtd_info *mtd1 = get_mtd_device_nm("nvram");
            if( !IS_ERR_OR_NULL(mtd1) )
            {
                uint32_t *pHdr = (uint32_t *)image_ptr;
                pHdr += (mtd1->erasesize / 4); /* pHdr points to the top of the 2nd nand block */
                for( blk_addr = mtd1->erasesize; blk_addr < mtd1->size; blk_addr += mtd1->erasesize )
                { 
                    /* If we are inside the for() loop, "nvram" mtd is larger than 1 block */
                    pHdr += (mtd1->erasesize / 4);
                }
   
                if (*(unsigned short *)pHdr != JFFS2_MAGIC_BITMASK)
                {
                    printk("New sw image does not match the partition table. Aborting.\n");
                    put_mtd_device(mtd0);
                    put_mtd_device(mtd1);
                    return -1; 
                }
                put_mtd_device(mtd1);
            }
            else
            {
                printk("Failed to get nvram mtd device\n");
                put_mtd_device(mtd0);
                return -1;
            }
        }
#endif

        /*
         * All checks complete ... write image to flash memory.
         * In theory, all calls to flash_write_buf() must be done with
         * semflash held, so I added it here.  However, in reality, all
         * flash image writes are protected by flashImageMutex at a higher
         * level.
         */
        down(&semflash);

        // Once we have acquired the flash semaphore, we can
        // disable activity on other processor and also on local processor.
        // Need to disable interrupts so that RCU stall checker will not complain.
        if (!should_yield)
        {
            stopOtherCpu();
            local_irq_save(flags);
        }

        local_bh_disable();

        if( 0 != ofs ) /* Image contains CFE ROM boot loader. */
        {
            /* Prepare to flash the CFE ROM boot loader. */
            struct mtd_info *mtd1 = get_mtd_device_nm("nvram");
            if( !IS_ERR_OR_NULL(mtd1) )
            {
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
                StallPmc();
#endif
                if (nandEraseBlk(mtd1, 0) == 0)
                    nandWriteBlk(mtd1, 0,  mtd1->erasesize, image_ptr,TRUE);

                image_ptr += ofs;

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
                if (wt.wfiFlags & WFI_FLAG_SUPPORTS_BTRM)
                {
                   /* We have already checked that the new sw image matches the partition table. Therefore */
                   /* burn the rest of the "nvram" mtd (if any) */
                   for( blk_addr = mtd1->erasesize; blk_addr < mtd1->size; blk_addr += mtd1->erasesize )
                   { 
                      if (nandEraseBlk( mtd1, blk_addr ) == 0)
                      { 
                         nandWriteBlk(mtd1, blk_addr, mtd1->erasesize, image_ptr, TRUE);
                         image_ptr += ofs;
                      }
                   }
                }
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
                UnstallPmc();
#endif

                put_mtd_device(mtd1);
            }
            else
            {
                printk("Failed to get nvram mtd device\n");
                put_mtd_device(mtd0);
                return -1;
            }
        }


        /* Erase blocks containing directory entry for CFERAM before flashing the image. */
        for( blk_addr = 0; blk_addr < rsrvd_for_cferam; blk_addr += mtd0->erasesize )
            nandEraseBlk( mtd0, blk_addr );

        /* Flash the image except for CFERAM directory entry, during which all the blocks in the partition (other than CFE) will be erased */
        writing_ubifs = 0;
        for( blk_addr = rsrvd_for_cferam; blk_addr < mtd0->size; blk_addr += mtd0->erasesize )
        {
            if (nandEraseBlk( mtd0, blk_addr ) == 0)
            { // block was erased successfully
                if ( image_ptr == cferam_ptr )
                { // skip CFERAM directory entry block
                    image_ptr += mtd0->erasesize;
                }
                else
                { /* Write a block of the image to flash. */
                    if( image_ptr < end_ptr )
                    { // if any data left, prepare to write it out
                        writelen = ((image_ptr + mtd0->erasesize) <= end_ptr)
                            ? mtd0->erasesize : (int) (end_ptr - image_ptr);
                    }
                    else
                        writelen = 0;

                    if (writelen) /* Write data with or without JFFS2 clean marker */
                    {
                        /* Write clean markers only to JFFS2 blocks */
                        if (nandWriteBlk(mtd0, blk_addr, writelen, image_ptr, !writing_ubifs) != 0 )
                            printk("Error writing Block 0x%8.8x\n", blk_addr);
                        else
                        { // successful write
                            printk(".");

                            if ( writelen )
                            { // increment counter and check for UBIFS split marker if data was written
                                image_ptr += writelen;

                                if (!strncmp(BCM_BCMFS_TAG, image_ptr - 0x100, strlen(BCM_BCMFS_TAG)))
                                    if (!strncmp(BCM_BCMFS_TYPE_UBIFS, image_ptr - 0x100 + strlen(BCM_BCMFS_TAG), strlen(BCM_BCMFS_TYPE_UBIFS)))
                                    { // check for UBIFS split marker
                                        writing_ubifs = 1;
                                        printk("U");
                                    }
                            }
                        }
                    }

                    if (should_yield)
                    {
                        local_bh_enable();
                        yield();
                        local_bh_disable();
                    }
                }
            }
        }

        /* Flash the block containing directory entry for CFERAM. */
        for( blk_addr = 0; blk_addr < rsrvd_for_cferam; blk_addr += mtd0->erasesize )
        {
            if (mtd_block_isbad(mtd0, blk_addr) == 0)
            {
                /* Write a block of the image to flash. */
                if (nandWriteBlk(mtd0, blk_addr, cferam_ptr ? mtd0->erasesize : 0, cferam_ptr, TRUE) == 0)
                {
                    printk(".");
                    cferam_ptr = NULL;
                }

                if (should_yield)
                {
                    local_bh_enable();
                    yield();
                    local_bh_disable();
                }
            }
        }

        if( cferam_ptr == NULL )
        {
            /* block containing directory entry for CFERAM was written successfully! */
            /* Whole flash image is programmed successfully! */
            sts = 0;
        }

        up(&semflash);
        printk("\n\n");

        if (should_yield)
        {
            local_bh_enable();
        }

        if( sts )
        {
            /*
             * Even though we try to recover here, this is really bad because
             * we have stopped the other CPU and we cannot restart it.  So we
             * really should try hard to make sure flash writes will never fail.
             */
            printk(KERN_ERR "nandWriteBlk: write failed at blk=%d\n", blk_addr);
            sts = (blk_addr > mtd0->erasesize) ? blk_addr / mtd0->erasesize : 1;
            if (!should_yield)
            {
                local_irq_restore(flags);
                local_bh_enable();
            }
        }
    }

    if( !IS_ERR_OR_NULL(mtd0) )
        put_mtd_device(mtd0);

    if( sts == 0 && old_img == 1 )
    {
        printk("\nOld image, deleting data and secondary file system partitions\n");
        mtd0 = get_mtd_device_nm("data");
        for( blk_addr = 0; blk_addr < mtd0->size; blk_addr += mtd0->erasesize )
        { // write JFFS2 clean markers
            if (nandEraseBlk( mtd0, blk_addr ) == 0)
                nandWriteBlk(mtd0, blk_addr, 0, NULL, TRUE);
        }

        mtd0 = get_mtd_device_nm("image_update");
        for( blk_addr = 0; blk_addr < mtd0->size; blk_addr += mtd0->erasesize )
        {
            if (nandEraseBlk( mtd0, blk_addr ) == 0)
                nandWriteBlk(mtd0, blk_addr, 0, NULL, TRUE);
        }
    }

    return sts;
}

    // NAND flash overwrite nvram block.    
    // return: 
    // 0 - ok
    // !0 - the sector number fail to be flashed (should not be 0)
static int nandNvramSet(const char *nvramString )
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

    /* Read the whole cfe rom nand block 0 */
    mtd_read(mtd, 0, mtd->erasesize, &retlen, cferom_ptr);

    /* Copy the nvram string into place */
    memcpy(cferom_ptr + NVRAM_DATA_OFFSET, nvramString, sizeof(NVRAM_DATA));
    
    /* Flash the CFE ROM boot loader. */
    if (nandEraseBlk( mtd, 0 ) == 0)
        nandWriteBlk(mtd, 0, mtd->erasesize, cferom_ptr, TRUE);

    retriedKfree(cferom_ptr);
    return 0;
}
           

// flash bcm image 
// return: 
// 0 - ok
// !0 - the sector number fail to be flashed (should not be 0)
// Must be called with flashImageMutex held.
int kerSysBcmImageSet( int flash_start_addr, char *string, int size,
    int should_yield)
{
    int sts;
    int sect_size;
    int blk_start;
    int savedSize = size;
    int whole_image = 0;
    unsigned long flags=0;
    int is_cfe_write=0;
    WFI_TAG wt = {0};
#if defined(CONFIG_BCM963268)
    PNVRAM_DATA pnd;
#endif

    BCM_ASSERT_HAS_MUTEX_C(&flashImageMutex);
    if (flash_start_addr == FLASH_BASE)
    {
        unsigned int chip_id = kerSysGetChipId();
        whole_image = 1;
        memcpy(&wt, string + size, sizeof(wt));
        if( (wt.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
            wt.wfiChipId != chip_id )
        {
            int id_ok = 0;
#if defined(CHIP_FAMILY_ID_HEX)
            if (wt.wfiChipId == CHIP_FAMILY_ID_HEX) {
                id_ok = 1;
            }
#endif
            if (id_ok == 0) {
                printk("Chip Id error.  Image Chip Id = %04x, Board Chip Id = "
                    "%04x.\n", wt.wfiChipId, chip_id);
                return -1;
            }
        }
    }

    if( whole_image && (wt.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
        wt.wfiFlashType != WFI_NOR_FLASH )
    {
        printk("ERROR: Image does not support a NOR flash device.\n");
        return -1;
    }

#if defined(CONFIG_BCM963268)
    if ( whole_image && ( (   (wt.wfiFlags & WFI_FLAG_SUPPORTS_BTRM)  && (! bcm_otp_is_btrm_boot())) ||
                          ((! (wt.wfiFlags & WFI_FLAG_SUPPORTS_BTRM)) &&    bcm_otp_is_btrm_boot() ) ))
    {
        printk("ERROR: Secure Boot Conflict. The image type does not match the SoC OTP configuration. Aborting.\n");
        return -1;
    }
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
    /* check if it is zero padded or no 64K padding at all for backward compatiblity */
    if(whole_image && ( (wt.wfiFlags&WFI_FLAG_HAS_PMC) == 0) )
    {
        unsigned int *pImg  = (unsigned int*)string;
        unsigned char * pBuf = string;
        int block_start, block_end, sect_size, remain, block;
        
        if( *pImg == 0 && *(pImg+1) == 0 && *(pImg+2) == 0 && *(pImg+3) == 0 )
        {
           /* the first 64KB are for PMC in 631x8, need to preserve that for cfe/linux image update
           if it is not for PMC image update. */
           sect_size = flash_get_sector_size(0);
           block_start = 0;
           block_end = IMAGE_OFFSET/sect_size;
           remain = IMAGE_OFFSET%sect_size;

           for( block = block_start; block < block_end; block++ )
           {
               flash_read_buf(block, 0, pBuf, sect_size);
               pBuf += sect_size;
           }
         
           if( remain )
           {
              block = block_end;
               flash_read_buf(block, 0, pBuf, remain);
           }
        }
        else
        {
            /* does not have PMC at all, the input data starting from 64KB offset */
            if( (flash_get_flash_type() == FLASH_IFC_SPI) || (flash_get_flash_type() == FLASH_IFC_HS_SPI)  )
                flash_start_addr += IMAGE_OFFSET;
        }
    }
#endif

#if defined(DEBUG_FLASH)
    printk("kerSysBcmImageSet: flash_start_addr=0x%x string=%p len=%d whole_image=%d\n",
           flash_start_addr, string, size, whole_image);
#endif

    blk_start = flash_get_blk(flash_start_addr);

    if( blk_start < 0 )
        return( -1 );

    is_cfe_write = ((NVRAM_SECTOR == blk_start) &&
                    (size <= FLASH_LENGTH_BOOT_ROM));
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
    /* if PMC image is included, it is also considered as CFE write */
    if( blk_start == 0 && size <= (FLASH_LENGTH_BOOT_ROM+IMAGE_OFFSET) )
        is_cfe_write = 1;
#endif

    /*
     * write image to flash memory.
     * In theory, all calls to flash_write_buf() must be done with
     * semflash held, so I added it here.  However, in reality, all
     * flash image writes are protected by flashImageMutex at a higher
     * level.
     */
    down(&semflash);


    // Once we have acquired the flash semaphore, we can
    // disable activity on other processor and also on local processor.
    // Need to disable interrupts so that RCU stall checker will not complain.
    if (!is_cfe_write && !should_yield)
    {
        stopOtherCpu();
        local_irq_save(flags);
    }

    local_bh_disable();
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    /* stall pmc if we are writing to the pmc flash address*/
    if( flash_start_addr == FLASH_BASE )
         StallPmc();
#endif
    do 
    {
        sect_size = flash_get_sector_size(blk_start);
        /* better to do read/modify/write for PMC code on 63138 at first 64K.
           so far SPI flash only has sector size up to 64KB. So we are ok for now */
        flash_sector_erase_int(blk_start);     // erase blk before flash

        if (sect_size > size) 
        {
            if (size & 1) 
                size++;
            sect_size = size;
        }

#if defined(CONFIG_BCM963268)
        if ((NVRAM_SECTOR == blk_start) && (wt.wfiFlags & WFI_FLAG_SUPPORTS_BTRM) && bcm_otp_is_boot_secure())
        {
            /* Upgrading a secure-boot 63268 SoC, and we are about to update the sector  */
            /* that contains the 3k nvram. Ensure the new security credentials make it   */
            /* into flash and the in-memory copy of nvram. Do this by making nvram valid */
            /* within the new image before writing it to flash */
            pnd = (PNVRAM_DATA)(string + NVRAM_DATA_OFFSET);
            spin_lock_irqsave(&inMemNvramData_spinlock, flags);
            memcpy((unsigned char *)pnd, inMemNvramData_buf, 1024); /* do not change 1024 to be sizeof(NVRAM_DATA) */
            spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);
            pnd->ulCheckSum = 0;
            pnd->ulCheckSum = crc32(CRC32_INIT_VALUE, pnd, sizeof(NVRAM_DATA));
         }
#endif

        if (flash_write_buf(blk_start, 0, string, sect_size) != sect_size) {
            break;
        }

        // check if we just wrote into the sector where the NVRAM is.
        // update our in-memory copy
        if (NVRAM_SECTOR == blk_start)
        {
            updateInMemNvramData(string+NVRAM_DATA_OFFSET, NVRAM_LENGTH, 0);
        }

        printk(".");
        blk_start++;
        string += sect_size;
        size -= sect_size; 

        if (should_yield)
        {
            local_bh_enable();
            yield();
            local_bh_disable();
        }
    } while (size > 0);

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    /* stall pmc if we are writing to the pmc flash address*/
    if( flash_start_addr == FLASH_BASE )
         UnstallPmc();
#endif
    if (whole_image && savedSize > fInfo.flash_rootfs_start_offset)
    {
        // If flashing a whole image, erase to end of flash.
        int total_blks = flash_get_numsectors();
        while( blk_start < total_blks )
        {
            flash_sector_erase_int(blk_start);
            printk(".");
            blk_start++;

            if (should_yield)
            {
                local_bh_enable();
                yield();
                local_bh_disable();
            }
        }
    }

    up(&semflash);

    printk("\n\n");

    if (is_cfe_write || should_yield)
    {
        local_bh_enable();
    }

    if( size == 0 )
    {
        sts = 0;  // ok
    }
    else
    {
        /*
         * Even though we try to recover here, this is really bad because
         * we have stopped the other CPU and we cannot restart it.  So we
         * really should try hard to make sure flash writes will never fail.
         */
        printk(KERN_ERR "kerSysBcmImageSet: write failed at blk=%d\n",
                        blk_start);
        sts = blk_start;    // failed to flash this sector
        if (!is_cfe_write && !should_yield)
        {
            local_irq_restore(flags);
            local_bh_enable();
        }
    }

    return sts;
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
    char *ret = NULL;

    if( (ret = retriedKmalloc(len)) != NULL )
    {
        struct file *fp;
        mm_segment_t fs;

        memset(ret, 0x00, len);
        fp = filp_open(SCRATCH_PAD_FILE_NAME, O_RDONLY, 0);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 00)
        if (!IS_ERR(fp) && fp->f_op && fp->f_op->read)
#else
        if (!IS_ERR(fp))
#endif
        {
            fs = get_fs();
            set_fs(get_ds());

            fp->f_pos = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 00)
            if((int) fp->f_op->read(fp, (void *) ret, len, &fp->f_pos) <= 0)
#else
            if((int) vfs_read(fp, (void *) ret, len, &fp->f_pos) <= 0)
#endif
                printk("Failed to read scratch pad from '%s'\n",
                    SCRATCH_PAD_FILE_NAME);

            filp_close(fp, NULL);
            set_fs(fs);
        }
    }
    else
        printk("Could not allocate scratch pad memory.\n");

    return( ret );
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

    if( bootFromNand )
    {
        if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL) {
            mutex_unlock(&spMutex);
            return sts;
        }

        pBuf = pShareBuf;
    }
    else
    {
        if( (pShareBuf = getSharedBlks(fInfo.flash_scratch_pad_start_blk,
            fInfo.flash_scratch_pad_number_blk)) == NULL )
        {
            printk("could not getSharedBlks.\n");
            mutex_unlock(&spMutex);
            return sts;
        }

        // pBuf points to SP buf
        pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  
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

    while( isalnum(pToken->tokenName[0]) && pToken->tokenLen > 0 &&
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

    if( bootFromNand )
    {
        if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL) {
            mutex_unlock(&spMutex);
            return sts;
        }

        pBuf = pShareBuf;
    }
    else
    {
        if( (pShareBuf = getSharedBlks(fInfo.flash_scratch_pad_start_blk,
            fInfo.flash_scratch_pad_number_blk)) == NULL )
        {
            printk("could not getSharedBlks.\n");
            mutex_unlock(&spMutex);
            return sts;
        }

        // pBuf points to SP buf
        pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;
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
    while( isalnum(pToken->tokenName[0]) && pToken->tokenLen > 0 &&
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

    if( bufLen >= fInfo.flash_scratch_pad_length - sizeof(SP_HEADER) -
        sizeof(SP_TOKEN) )
    {
        printk("Scratch pad overflow by %d bytes.  Information not saved.\n",
            bufLen  - fInfo.flash_scratch_pad_length - (int)sizeof(SP_HEADER) -
            (int)sizeof(SP_TOKEN));
        return sts;
    }

    if( !tokenId || !isalnum(tokenId[0]) )
    {
        printk("Invalid scratch pad key name. Must start with a letter or number.\n");
        return sts;
    }

    mutex_lock(&spMutex);

    if( bootFromNand )
    {
        if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL)
        {
            mutex_unlock(&spMutex);
            return sts;
        }

        pBuf = pShareBuf;
    }
    else
    {
        if( (pShareBuf = getSharedBlks( fInfo.flash_scratch_pad_start_blk,
            fInfo.flash_scratch_pad_number_blk)) == NULL )
        {
            mutex_unlock(&spMutex);
            return sts;
        }

        // pBuf points to SP buf
        pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  
    }

    // form header info.
    memset((char *)&SPHead, 0, sizeof(SP_HEADER));
    memcpy(SPHead.SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN);
    SPHead.SPVersion = SP_VERSION;

    // form token info.
    memset((char*)&SPToken, 0, sizeof(SP_TOKEN));
    strncpy(SPToken.tokenName, tokenId, TOKEN_NAME_LEN - 1);
    SPToken.tokenLen = bufLen;

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0)
    {
        // new sp, so just flash the token
        printk("No scratch pad found.  Initialize scratch pad...\n");
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
        while( isalnum(pToken->tokenName[0]) &&
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

    if( bootFromNand )
        sts = setScratchPad(pShareBuf, fInfo.flash_scratch_pad_length);
    else
        sts = setSharedBlks(fInfo.flash_scratch_pad_start_blk, 
            fInfo.flash_scratch_pad_number_blk, pShareBuf);
    
    retriedKfree(pShareBuf);

    mutex_unlock(&spMutex);

    return sts;

    
}

#if defined (CONFIG_BCM96838) || defined (CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM94908) | defined(CONFIG_BCM96858)
EXPORT_SYMBOL(kerSysScratchPadGet);
EXPORT_SYMBOL(kerSysScratchPadSet);
#endif

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

    if( bootFromNand )
    {
        if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL)
        {
            mutex_unlock(&spMutex);
            return sts;
        }

        memset(pShareBuf, 0x00, fInfo.flash_scratch_pad_length);

        setScratchPad(pShareBuf, fInfo.flash_scratch_pad_length);
    }
    else
    {
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
    }

    retriedKfree(pShareBuf);

    mutex_unlock(&spMutex);

    //printk ("kerSysScratchPadClearAll Done.... \n") ;
    return sts;
}

int kerSysFlashSizeGet(void)
{
    int ret = 0;

    if( bootFromNand )
    {
        struct mtd_info *mtd;

        if( (mtd = get_mtd_device_nm("image")) != NULL )
        {
            ret = mtd->size;
            put_mtd_device(mtd);
        }
    }
    else
        ret = flash_get_total_size();

    return ret;
}

/***********************************************************************
 * Function Name: writeBootImageState
 * Description  : Persistently sets the state of an image update.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
static int writeBootImageState( int currState, int newState )
{
    int ret = -1;

    if(bootFromNand == 0)
    {
        /* NOR flash */
        char *pShareBuf = NULL;

        if( (pShareBuf = getSharedBlks( fInfo.flash_scratch_pad_start_blk,
            fInfo.flash_scratch_pad_number_blk)) != NULL )
        {
            PSP_HEADER pHdr = (PSP_HEADER) pShareBuf;
            unsigned long *pBootImgState=(unsigned long *)&pHdr->NvramData2[0];

            /* The boot image state is stored as a word in flash memory where
             * the most significant three bytes are a "magic number" and the
             * least significant byte is the state constant.
             */
            if( (*pBootImgState & 0xffffff00) == (BLPARMS_MAGIC & 0xffffff00) )
            {
                *pBootImgState &= ~0x000000ff;
                *pBootImgState |= newState;

                ret = setSharedBlks(fInfo.flash_scratch_pad_start_blk,    
                    fInfo.flash_scratch_pad_number_blk,  pShareBuf);
            }

            retriedKfree(pShareBuf);
        }
    }
    else
    {
        mm_segment_t fs;
        struct file *fp;

        /* NAND flash */
        char state_name[] = "/data/" NAND_BOOT_STATE_FILE_NAME;

        fs = get_fs();
        set_fs(get_ds());

        /* Remove old file:
         * This must happen before a new file is created so that the new file-
         * name will have a higher version in the FS, this is also the reason
         * why renaming might not work well (higher version might be exist as
         * deleted in FS) */
        if( currState != -1 )
        {
            state_name[strlen(state_name) - 1] = currState;
            sys_unlink(state_name);
        }

        /* Create new state file name. */
        state_name[strlen(state_name) - 1] = newState;
        fp = filp_open(state_name, O_RDWR | O_TRUNC | O_CREAT,
                S_IRUSR | S_IWUSR);

        if (!IS_ERR(fp))
        {
            fp->f_pos = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
            vfs_write(fp, (void *) "boot state\n",
                    strlen("boot state\n"), &fp->f_pos);
#else

            fp->f_op->write(fp, (void *) "boot state\n",
                    strlen("boot state\n"), &fp->f_pos);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
            vfs_fsync(fp, 0);
#else
            vfs_fsync(fp, fp->f_path.dentry, 0);
#endif
            filp_close(fp, NULL);
        }
        else
            printk("Unable to open '%s'.\n", state_name);

        set_fs(fs);
        ret = 0;
    }

    return( ret );
}

/***********************************************************************
 * Function Name: readBootImageState
 * Description  : Reads the current boot image state from flash memory.
 * Returns      : state constant or -1 for failure
 ***********************************************************************/
static int readBootImageState( void )
{
    int ret = -1;

    if(bootFromNand == 0)
    {
        /* NOR flash */
        char *pShareBuf = NULL;

        if( (pShareBuf = getSharedBlks( fInfo.flash_scratch_pad_start_blk,
            fInfo.flash_scratch_pad_number_blk)) != NULL )
        {
            PSP_HEADER pHdr = (PSP_HEADER) pShareBuf;
            unsigned long *pBootImgState=(unsigned long *)&pHdr->NvramData2[0];

            /* The boot image state is stored as a word in flash memory where
             * the most significant three bytes are a "magic number" and the
             * least significant byte is the state constant.
             */
            if( (*pBootImgState & 0xffffff00) == (BLPARMS_MAGIC & 0xffffff00) )
            {
                ret = *pBootImgState & 0x000000ff;
            }

            retriedKfree(pShareBuf);
        }
    }
    else
    {
        /* NAND flash */
        int i;
        char states[] = {BOOT_SET_NEW_IMAGE, BOOT_SET_OLD_IMAGE,
            BOOT_SET_NEW_IMAGE_ONCE};
        char boot_state_name[] = "/data/" NAND_BOOT_STATE_FILE_NAME;

        /* The boot state is stored as the last character of a file name on
         * the data partition, /data/boot_state_X, where X is
         * BOOT_SET_NEW_IMAGE, BOOT_SET_OLD_IMAGE or BOOT_SET_NEW_IMAGE_ONCE.
         */
        for( i = 0; i < sizeof(states); i++ )
        {
            struct file *fp;
            boot_state_name[strlen(boot_state_name) - 1] = states[i];
            fp = filp_open(boot_state_name, O_RDONLY, 0);
            if (!IS_ERR(fp) )
            {
                filp_close(fp, NULL);

                ret = (int) states[i];
                break;
            }
        }

        if( ret == -1 && writeBootImageState( -1, BOOT_SET_NEW_IMAGE ) == 0 )
            ret = BOOT_SET_NEW_IMAGE;
    }

    return( ret );
}

/***********************************************************************
 * Function Name: dirent_rename
 * Description  : Renames file oldname to newname by parsing NAND flash
 *                blocks with JFFS2 file system entries.  When the JFFS2
 *                directory entry inode for oldname is found, it is modified
 *                for newname.  This differs from a file system rename
 *                operation that creates a new directory entry with the same
 *                inode value and greater version number.  The benefit of
 *                this method is that by having only one directory entry
 *                inode, the CFE ROM can stop at the first occurance which
 *                speeds up the boot by not having to search the entire file
 *                system partition.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
static int dirent_rename(char *oldname, char *newname)
{
    int ret = -1;
    struct mtd_info *mtd;
    unsigned char *buf;

    mtd = get_mtd_device_nm("bootfs_update");

    if( IS_ERR_OR_NULL(mtd) )
        mtd = get_mtd_device_nm("rootfs_update"); 

    buf = (mtd) ? retriedKmalloc(mtd->erasesize) : NULL;

    if( mtd && buf && strlen(oldname) == strlen(newname) )
    {
        int namelen = strlen(oldname);
        int blk, done;
        size_t retlen;

        /* Read NAND flash blocks in the rootfs_update JFFS2 file system
         * partition to search for a JFFS2 directory entry for the oldname
         * file.
         */
        for(blk = 0, done = 0; blk < mtd->size && !done; blk += mtd->erasesize)
        {
            if(mtd_read(mtd, blk, mtd->erasesize, &retlen, buf) == 0)
            {
                struct jffs2_raw_dirent *pdir;
                unsigned char *p = buf;

                while( p < buf + mtd->erasesize )
                {
                    pdir = (struct jffs2_raw_dirent *) p;
                    if( je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK )
                    {
                        if( je16_to_cpu(pdir->nodetype) ==
                                JFFS2_NODETYPE_DIRENT &&
                            !memcmp(oldname, pdir->name, namelen) &&
                            je32_to_cpu(pdir->ino) != 0 )
                        {
                            /* Found the oldname directory entry.  Change the
                             * name to newname.
                             */
                            memcpy(pdir->name, newname, namelen);
                            je32_to_cpu(pdir->name_crc) = crc32(0, pdir->name,
                                (unsigned int) namelen);

                            /* Write the modified block back to NAND flash. */
                            if(nandEraseBlk( mtd, blk ) == 0)
                            {
                                if( nandWriteBlk(mtd, blk, mtd->erasesize, buf, TRUE) == 0 )
                                {
                                    ret = 0;
                                }
                            }

                            done = 1;
                            break;
                        }

                        p += (je32_to_cpu(pdir->totlen) + 0x03) & ~0x03;
                    }
                    else
                        break;
                }
            }
        }
    }
    else
        printk("%s: Error renaming file\n", __FUNCTION__);

    if( mtd )
        put_mtd_device(mtd);

    if( buf )
        retriedKfree(buf);

    return( ret );
}

/***********************************************************************
 * Function Name: updateSequenceNumber
 * Description  : updates the sequence number on the specified partition
 *                to be the highest.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
static int updateSequenceNumber(int incSeqNumPart, int seqPart1, int seqPart2)
{
    int ret = -1;

    mutex_lock(&flashImageMutex);
    if(bootFromNand == 0)
    {
        /* NOR flash */
        char *pShareBuf = NULL;
        PFILE_TAG pTag;
        int blk;

        pTag = kerSysUpdateTagSequenceNumber(incSeqNumPart);
        blk = *(int *) (pTag + 1);

        if ((pShareBuf = getSharedBlks(blk, 1)) != NULL)
        {
            memcpy(pShareBuf, pTag, sizeof(FILE_TAG));
            setSharedBlks(blk, 1, pShareBuf);
            retriedKfree(pShareBuf);
        }
    }
    else
    {
        /* NAND flash */

        /* The sequence number on NAND flash is updated by renaming file
         * cferam.XXX where XXX is the sequence number. The rootfs partition
         * is usually read-only. Therefore, only the cferam.XXX file on the
         * rootfs_update partiton is modified. Increase or decrase the
         * sequence number on the rootfs_update partition so the desired
         * partition boots.
         */
        int seq = -1;
        unsigned long rootfs_ofs = 0xff;

        kerSysBlParmsGetInt(NAND_RFS_OFS_NAME, (int *) &rootfs_ofs);

        if(rootfs_ofs == inMemNvramData.ulNandPartOfsKb[NP_ROOTFS_1] )
        {
            /* Partition 1 is the booted partition. */
            if( incSeqNumPart == 2 )
            {
                /* Increase sequence number in rootfs_update partition. */
                if( seqPart1 >= seqPart2 )
                    seq = seqPart1 + 1;
            }
            else
            {
                /* Decrease sequence number in rootfs_update partition. */
                if( seqPart2 >= seqPart1 && seqPart1 != 0 )
                    seq = seqPart1 - 1;
            }
        }
        else
        {
            /* Partition 2 is the booted partition. */
            if( incSeqNumPart == 1 )
            {
                /* Increase sequence number in rootfs_update partition. */
                if( seqPart2 >= seqPart1 )
                    seq = seqPart2 + 1;
            }
            else
            {
                /* Decrease sequence number in rootfs_update partition. */
                if( seqPart1 >= seqPart2 && seqPart2 != 0 )
                    seq = seqPart2 - 1;
            }
        }

        if( seq != -1 )
        {
            struct mtd_info *mtd;
            char mtd_part[32];

            /* Find the sequence number of the non-booted partition. */
            mm_segment_t fs;

            fs = get_fs();
            set_fs(get_ds());

            mtd = get_mtd_device_nm("bootfs_update");
            if( !IS_ERR_OR_NULL(mtd) )
            {
                strcpy(mtd_part, "mtd:bootfs_update");
                put_mtd_device(mtd);
            }
            else
                strcpy(mtd_part, "mtd:rootfs_update");


            if( sys_mount(mtd_part, "/mnt", "jffs2", 0, NULL) == 0 )
            {
                char fname[] = NAND_CFE_RAM_NAME;
                char cferam_old[32], cferam_new[32], cferam_fmt[32]; 
                int i;

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
                /* If full secure boot is in play, the CFE RAM file is the encrypted version */
                if (bcm_otp_is_boot_secure())
                   strcpy(fname, NAND_CFE_RAM_SECBT_NAME);
#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
                else
                {
                   if (bcm_otp_is_boot_mfg_secure())
                      strcpy(fname, NAND_CFE_RAM_SECBT_MFG_NAME);
                }
#endif
#endif
                fname[strlen(fname) - 3] = '\0'; /* remove last three chars */
                strcpy(cferam_fmt, "/mnt/");
                strcat(cferam_fmt, fname);
                strcat(cferam_fmt, "%3.3d");

                for( i = 0; i < 999; i++ )
                {
                    struct file *fp;
                    sprintf(cferam_old, cferam_fmt, i);
                    fp = filp_open(cferam_old, O_RDONLY, 0);
                    if (!IS_ERR(fp) )
                    {
                        filp_close(fp, NULL);

                        /* Change the boot sequence number in the rootfs_update
                         * partition by renaming file cferam.XXX where XXX is
                         * a sequence number.
                         */
                        sprintf(cferam_new, cferam_fmt, seq);
                        if( NAND_FULL_PARTITION_SEARCH )
                        {
                            sys_rename(cferam_old, cferam_new);
                            sys_umount("/mnt", 0);
                        }
                        else
                        {
                            sys_umount("/mnt", 0);
                            dirent_rename(cferam_old + strlen("/mnt/"),
                                cferam_new + strlen("/mnt/"));
                        }
                        break;
                    }
                }
            }
            set_fs(fs);
        }
    }
    mutex_unlock(&flashImageMutex);

    return( ret );
}

/***********************************************************************
 * Function Name: kerSysSetBootImageState
 * Description  : Persistently sets the state of an image update.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int kerSysSetBootImageState( int newState )
{
    int ret = -1;
    int incSeqNumPart = -1;
    int writeImageState = 0;
    int currState = readBootImageState();
    int seq1 = kerSysGetSequenceNumber(1);
    int seq2 = kerSysGetSequenceNumber(2);

    /* Update the image state persistently using "new image" and "old image"
     * states.  Convert "partition" states to "new image" state for
     * compatibility with the non-OMCI image update.
     */
    mutex_lock(&spMutex);
    switch(newState)
    {
    case BOOT_SET_PART1_IMAGE:
        if( seq1 != -1 )
        {
            if( seq1 < seq2 )
                incSeqNumPart = 1;
            newState = BOOT_SET_NEW_IMAGE;
            writeImageState = 1;
        }
        break;

    case BOOT_SET_PART2_IMAGE:
        if( seq2 != -1 )
        {
            if( seq2 < seq1 )
                incSeqNumPart = 2;
            newState = BOOT_SET_NEW_IMAGE;
            writeImageState = 1;
        }
        break;

    case BOOT_SET_PART1_IMAGE_ONCE:
        if( seq1 != -1 )
        {
            if( seq1 < seq2 )
                incSeqNumPart = 1;
            newState = BOOT_SET_NEW_IMAGE_ONCE;
            writeImageState = 1;
        }
        break;

    case BOOT_SET_PART2_IMAGE_ONCE:
        if( seq2 != -1 )
        {
            if( seq2 < seq1 )
                incSeqNumPart = 2;
            newState = BOOT_SET_NEW_IMAGE_ONCE;
            writeImageState = 1;
        }
        break;

    case BOOT_SET_OLD_IMAGE:
    case BOOT_SET_NEW_IMAGE:
    case BOOT_SET_NEW_IMAGE_ONCE:
        /* The boot image state is stored as a word in flash memory where
         * the most significant three bytes are a "magic number" and the
         * least significant byte is the state constant.
         */
        if( currState == newState )
        {
            ret = 0;
        }
        else
        {
            writeImageState = 1;

            if(newState==BOOT_SET_NEW_IMAGE && currState==BOOT_SET_OLD_IMAGE)
            {
                /* The old (previous) image is being set as the new
                 * (current) image. Make sequence number of the old
                 * image the highest sequence number in order for it
                 * to become the new image.
                 */
                incSeqNumPart = 0;
            }
        }
        break;

    default:
        break;
    }

    if( writeImageState )
        ret = writeBootImageState(currState, newState);

    mutex_unlock(&spMutex);

    if( incSeqNumPart != -1 )
        updateSequenceNumber(incSeqNumPart, seq1, seq2);

    return( ret );
}

/***********************************************************************
 * Function Name: kerSysGetBootImageState
 * Description  : Gets the state of an image update from flash.
 * Returns      : state constant or -1 for failure
 ***********************************************************************/
int kerSysGetBootImageState( void )
{
    int ret = readBootImageState();

    if( ret != -1 )
    {
        int seq1 = kerSysGetSequenceNumber(1);
        int seq2 = kerSysGetSequenceNumber(2);

        switch(ret)
        {
        case BOOT_SET_NEW_IMAGE:
            if( seq1 == -1 || seq1 < seq2)
                ret = BOOT_SET_PART2_IMAGE;
            else
                ret = BOOT_SET_PART1_IMAGE;
            break;

        case BOOT_SET_NEW_IMAGE_ONCE:
            if( seq1 == -1 || seq1 < seq2)
                ret = BOOT_SET_PART2_IMAGE_ONCE;
            else
                ret = BOOT_SET_PART1_IMAGE_ONCE;
            break;

        case BOOT_SET_OLD_IMAGE:
            if( seq1 == -1 || seq1 > seq2)
                ret = BOOT_SET_PART2_IMAGE;
            else
                ret = BOOT_SET_PART1_IMAGE;
            break;

        default:
            ret = -1;
            break;
        }
    }

    return( ret );
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
    NVRAM_DATA *pNd;
    int ret = -1;
    BCM_ASSERT_NOT_HAS_MUTEX_C(&flashImageMutex);
    if ((pNd = kmalloc(NVRAM_LENGTH, GFP_KERNEL)) != NULL)
    {
       memcpy((char *)pNd, inMemNvramData_buf, NVRAM_LENGTH);

       pNd->opticRxPwrReading = rxReading;
       pNd->opticRxPwrOffset  = rxOffset;
       pNd->opticTxPwrReading = txReading;

       pNd->ulCheckSum = 0;
       pNd->ulCheckSum = crc32(CRC32_INIT_VALUE, pNd, sizeof(NVRAM_DATA));

       mutex_lock(&flashImageMutex);
       ret = kerSysNvRamSet((char *)pNd, NVRAM_LENGTH, 0);
       mutex_unlock(&flashImageMutex);
       kfree(pNd);
    }
    return(ret);

#if 0
    NVRAM_DATA nd;

    kerSysNvRamGet((char *) &nd, sizeof(nd), 0);

    nd.opticRxPwrReading = rxReading;
    nd.opticRxPwrOffset  = rxOffset;
    nd.opticTxPwrReading = txReading;
    
    nd.ulCheckSum = 0;
    nd.ulCheckSum = crc32(CRC32_INIT_VALUE, &nd, sizeof(NVRAM_DATA));

    return(kerSysNvRamSet((char *) &nd, sizeof(nd), 0));

#endif

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
#if 0
    NVRAM_DATA nd;

    kerSysNvRamGet((char *) &nd, sizeof(nd), 0);

    *prxReading = nd.opticRxPwrReading;
    *prxOffset  = nd.opticRxPwrOffset;
    *ptxReading = nd.opticTxPwrReading;
#else
    unsigned long flags;

    spin_lock_irqsave(&inMemNvramData_spinlock, flags);

    *prxReading = inMemNvramData.opticRxPwrReading;
    *prxOffset  = inMemNvramData.opticRxPwrOffset;
    *ptxReading = inMemNvramData.opticTxPwrReading;

    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);
#endif

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

#if defined(CONFIG_MTD_NAND)

static struct mtd_partition bcm63XX_nand_parts[] =
{
    { name: "rootfs",        offset: 0,           size: 0 },
    { name: "rootfs_update", offset: 0,           size: 0 },
    { name: "data",          offset: 0,           size: 0 },
    { name: "nvram",         offset: 0,           size: 0 },
    { name: "image",         offset: 0,           size: 0 },
    { name: "image_update",  offset: 0,           size: 0 },
    { name: "dummy1",        offset: 0,           size: 0 },
    { name: "dummy2",        offset: 0,           size: 0 },
    { name: "dummy3",        offset: 0,           size: 0 },
    { name: "dummy4",        offset: 0,           size: 0 },
    { name: "dummy5",        offset: 0,           size: 0 },
    { name: "dummy6",        offset: 0,           size: 0 },
    { name: NULL,            offset: 0,           size: 0 }
};

static char* misc_mtd_partition_names[BCM_MAX_EXTRA_PARTITIONS] =
{
    "misc1",
    "misc2",
    "misc3",
    NULL,
};


static unsigned int read_blk(unsigned char * start, unsigned int block, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtdi, int mtd_fd)
{
    struct mtd_info* mtd = mtdi;
    size_t retlen;

    mtd->_read(mtd, (block * blk_size) + offset, amount, &retlen, buf + offset);

    return(retlen == amount);
}


static int is_split_partition(struct mtd_info* mtd, unsigned long offset, unsigned long size, unsigned long *split_offset)
{
    uint8_t buf[0x100];
    size_t retlen;
    struct ubi_ec_hdr ec;

    *split_offset = offset;
    while (mtd->_block_isbad(mtd, *split_offset))
        *split_offset += mtd->erasesize;

    // check first CFERAM block to see if it's UBI, and if so it is new image format
    mtd->_read(mtd, *split_offset, UBI_EC_HDR_SIZE, &retlen, (void*)&ec);

    if (be32_to_cpu(ec.magic) == UBI_EC_HDR_MAGIC) {
        unsigned char * buf = kmalloc(mtd->erasesize, GFP_KERNEL);
        char squash;
        int try, ret = UBIFS_FS;

        if (buf)
        {
            for (try = 0; try < 2; try++)
            {
                if ( (parse_ubi(0, buf, *split_offset / mtd->erasesize, (offset + size) / mtd->erasesize, mtd->erasesize, (try ? VOLID_METADATA_COPY : VOLID_METADATA), "squash", &squash, 0, 0, read_blk, 0, 0, mtd, 0) == 1) && (squash == '1') )
                {
                    ret = SQUBI_FS;
                    break;
                }
            }
        }

        if (buf)
            kfree(buf);

        return(ret);
    }

    /* Search RootFS partion for split marker.
     * Marker is located in the last 0x100 bytes of the last BootFS Erase Block
     * If marker is found, we have separate Boot and Root Partitions.
     */
    for (*split_offset = offset + mtd->erasesize; *split_offset <= offset + size; *split_offset += mtd->erasesize) {
        if (mtd->_block_isbad(mtd, *split_offset - mtd->erasesize)) {
            continue;
        }

        mtd->_read(mtd, *split_offset - 0x100, 0x100, &retlen, buf);

        if (!strncmp(BCM_BCMFS_TAG, buf, strlen(BCM_BCMFS_TAG))) {
            if (!strncmp(BCM_BCMFS_TYPE_SQUBIFS, &buf[strlen(BCM_BCMFS_TAG)], strlen(BCM_BCMFS_TYPE_SQUBIFS))) {
                return(SQSPLIT_FS);
            }  /* Note: determining SQSPLIT must be done before UBIFS!! Why? pls refer to clarification in the define of 'BCM_BCMFS_TYPE_SQUBIFS'*/
            else if (!strncmp(BCM_BCMFS_TYPE_UBIFS, &buf[strlen(BCM_BCMFS_TAG)], strlen(BCM_BCMFS_TYPE_UBIFS))) {
                return(SPLIT_FS);
            }
        }
    }
    return(JFFS2_FS);
}


int setup_mtd_parts(struct mtd_info* mtd)
{
    int i = 0;
    uint64_t extra = 0, extra_single_part_size = 0;
    static NVRAM_DATA nvram;
    unsigned int rootfs_ofs;
    int nr_parts = 6;
    int rootfs, rootfs_update, split, image = 4, image_update = 5;
    unsigned long split_offset;


    kerSysNvRamLoad(mtd);
    kerSysBlParmsGetInt(NAND_RFS_OFS_NAME, &rootfs_ofs);
    kerSysNvRamGet((char*)&nvram, sizeof(nvram), 0);

    /* Root FS.  The CFE RAM boot loader saved the rootfs offset that the
     * Linux image was loaded from.
     */
    PRINTK("rootfs_ofs=0x%8.8x, part1ofs=0x%8.8x, part2ofs=0x%8.8x\n",
        rootfs_ofs, nvram.ulNandPartOfsKb[NP_ROOTFS_1],
        nvram.ulNandPartOfsKb[NP_ROOTFS_2]);

    if ( rootfs_ofs == nvram.ulNandPartOfsKb[NP_ROOTFS_1] ) {
        rootfs = NP_ROOTFS_1;
        rootfs_update = NP_ROOTFS_2;
    }else  {
        if ( rootfs_ofs == nvram.ulNandPartOfsKb[NP_ROOTFS_2] ) {
            rootfs = NP_ROOTFS_2;
            rootfs_update = NP_ROOTFS_1;
        }else  {
            /* Backward compatibility with old cferam. */
            extern unsigned char _text;
            unsigned int rootfs_ofs = *(unsigned int*)(&_text - 4);

            if ( rootfs_ofs == nvram.ulNandPartOfsKb[NP_ROOTFS_1] ) {
                rootfs = NP_ROOTFS_1;
                rootfs_update = NP_ROOTFS_2;
            }else  {
                rootfs = NP_ROOTFS_2;
                rootfs_update = NP_ROOTFS_1;
            }
        }
    }

    /* RootFS partition */
    bcm63XX_nand_parts[0].offset = nvram.ulNandPartOfsKb[rootfs] * 1024;
    bcm63XX_nand_parts[0].size = nvram.ulNandPartSizeKb[rootfs] * 1024;
    bcm63XX_nand_parts[0].ecclayout = mtd->ecclayout;


    if (nvram.ulNandPartOfsKb[rootfs] > nvram.ulNandPartOfsKb[rootfs_update]) { // put these in order of partiton so they may be identified later
        image = 5;
        image_update = 4;
    } else { // put these in order of partiton so they may be identified later
        image = 4;
        image_update = 5;
    }

    /* This partition is used for flashing images */
    bcm63XX_nand_parts[image].name = "image";
    bcm63XX_nand_parts[image].offset = bcm63XX_nand_parts[0].offset;
    bcm63XX_nand_parts[image].size = bcm63XX_nand_parts[0].size;
    bcm63XX_nand_parts[image].ecclayout = mtd->ecclayout;

    split = is_split_partition(mtd, bcm63XX_nand_parts[0].offset, bcm63XX_nand_parts[0].size, &split_offset);
    if ((SPLIT_FS == split) || (SQSPLIT_FS == split)) {
        /* RootFS partition */
        bcm63XX_nand_parts[0].offset = split_offset;
        bcm63XX_nand_parts[0].size -= (split_offset - nvram.ulNandPartOfsKb[rootfs] * 1024);

        /* BootFS partition */
        bcm63XX_nand_parts[nr_parts].name = "bootfs";
        bcm63XX_nand_parts[nr_parts].offset = nvram.ulNandPartOfsKb[rootfs] * 1024;
        bcm63XX_nand_parts[nr_parts].size = split_offset - nvram.ulNandPartOfsKb[rootfs] * 1024;
        bcm63XX_nand_parts[nr_parts].ecclayout = mtd->ecclayout;
    }

    printk(">>>>> For primary mtd partition %s, ", bcm63XX_nand_parts[0].name);
    if (split == JFFS2_FS)
        printk("cferam/vmlinux.lz and vmlinux fs mounted as JFFS2 <<<<<\n");
    if (split == SPLIT_FS)
        printk("cferam/vmlinux.lz mounted as JFFS2, vmlinux fs mounted as UBIFS <<<<<\n");
    if (split == UBIFS_FS)
        printk("cferam/vmlinux.lz UBI volume, vmlinux fs mounted as UBIFS <<<<<\n");
    if (split == SQSPLIT_FS)
        printk("cferam/vmlinux.lz mounted as JFFS2, vmlinux fs mounted as squash fs on UBI <<<<<\n");
    if (split == SQUBI_FS)
        printk("cferam/vmlinux.lz UBI volume, vmlinux fs mounted as squash fs on UBI <<<<<\n");

    if ((split == SPLIT_FS) || (split == UBIFS_FS)) {
        if (kerSysIsRootfsSet() == false) {
            kerSysSetBootParm("ubi.mtd", "0");
            kerSysSetBootParm("root=", "ubi:rootfs_ubifs");
            kerSysSetBootParm("rootfstype=", "ubifs");
        }
    }
    else if ((split == SQSPLIT_FS) || (split == SQUBI_FS)) {
        if (kerSysIsRootfsSet() == false) {
            kerSysSetBootParm("ubi.mtd", "0");
            kerSysSetBootParm("root=", "/dev/ubiblock0_0");
            kerSysSetBootParm("rootfstype=", "squashfs");
            kerSysSetBootParm("ubi.block", "0,0");
        }
    } else { // JFFS2 filesystem
        if (kerSysIsRootfsSet() == false) {
            kerSysSetBootParm("root=", "mtd:rootfs");
            kerSysSetBootParm("rootfstype=", "jffs2");
        }
    }

    nr_parts++;

    /* RootFS_update partition */
    bcm63XX_nand_parts[1].offset = nvram.ulNandPartOfsKb[rootfs_update] * 1024;
    bcm63XX_nand_parts[1].size = nvram.ulNandPartSizeKb[rootfs_update] * 1024;
    bcm63XX_nand_parts[1].ecclayout = mtd->ecclayout;

    /* This partition is used for flashing images */
    bcm63XX_nand_parts[image_update].name = "image_update";
    bcm63XX_nand_parts[image_update].offset = bcm63XX_nand_parts[1].offset;
    bcm63XX_nand_parts[image_update].size = bcm63XX_nand_parts[1].size;
    bcm63XX_nand_parts[image_update].ecclayout = mtd->ecclayout;

    split = is_split_partition(mtd, bcm63XX_nand_parts[1].offset, bcm63XX_nand_parts[1].size, &split_offset);
    if ((SPLIT_FS == split) || (SQSPLIT_FS == split)) {
        /* rootfs_update partition */
        bcm63XX_nand_parts[1].offset = split_offset;
        bcm63XX_nand_parts[1].size -= (split_offset - nvram.ulNandPartOfsKb[rootfs_update] * 1024);

        /* bootfs_update partition */
        bcm63XX_nand_parts[nr_parts].name = "bootfs_update";
        bcm63XX_nand_parts[nr_parts].offset = nvram.ulNandPartOfsKb[rootfs_update] * 1024;
        bcm63XX_nand_parts[nr_parts].size = split_offset - nvram.ulNandPartOfsKb[rootfs_update] * 1024;
        bcm63XX_nand_parts[nr_parts].ecclayout = mtd->ecclayout;
    }

    printk("Secondary mtd partition %s detected as ", bcm63XX_nand_parts[1].name);
    if (split == JFFS2_FS)
        printk("JFFS2 for cferam/vmlinux source and vmlinux filesystem\n");
    if (split == SPLIT_FS)
        printk("JFFS2 for cferam/vmlinux source and UBIFS for vmlinux filesystem\n");
    if (split == UBIFS_FS)
        printk("UBI for cferam/vmlinux source and UBIFS for vmlinux filesystem\n");
    if (split == SQSPLIT_FS)
        printk("JFFS2 for cferam/vmlinux source and squash fs on UBI for vmlinux filesystem\n");
    if (split == SQUBI_FS)
        printk("UBI for cferam/vmlinux source and squash fs on UBI for vmlinux filesystem\n");

    nr_parts++;

    /* Data (psi, scratch pad) */
    bcm63XX_nand_parts[2].offset = nvram.ulNandPartOfsKb[NP_DATA] * 1024;
    bcm63XX_nand_parts[2].size = nvram.ulNandPartSizeKb[NP_DATA] * 1024;
    bcm63XX_nand_parts[2].ecclayout = mtd->ecclayout;

#ifdef CRASHLOG
    /* Ares hack to adjust partition. misc2 is 64 (or 48) MB, misc3 0 MB, --> modify to misc2 63 (or 47) MB, misc3 1 MB */
#ifdef GTAC5300
    if (nvram.part_info[1].size == 64 && nvram.part_info[2].size == 0) {
        nvram.part_info[1].size = 63;
#else
    if (nvram.part_info[1].size == 48 && nvram.part_info[2].size == 0) {
        nvram.part_info[1].size = 47;
#endif
        nvram.part_info[2].size = 1;
    }
#endif /* CRASHLOG */

    // skip DATA partition
    for (i = BCM_MAX_EXTRA_PARTITIONS - 2; i >= 0; i--) {
        printk("setup_mtd_parts: misc indx %d name %s nvram configured size %d \n", i,misc_mtd_partition_names[i], nvram.part_info[i].size);

        if (nvram.part_info[i].size == 0xffff)
            continue;

        //sz_bits -- 0b01 -- MB, 0b10 - GB , 0b10, 0b11 - reserved
        switch ((nvram.part_info[i].size & 0xc000) >> 14) {
        case 0:
            extra_single_part_size = 1 << 20; //1024*1024;
            break;
        case 1:
            extra_single_part_size = 1 << 30; //1024*1024*1024;
            break;
        default:
            extra_single_part_size = 0;
            break;
        }

        extra_single_part_size = (nvram.part_info[i].size & 0x3fff) * extra_single_part_size;
        if ((extra_single_part_size & (~((uint64_t)mtd->erasesize - 1))) != extra_single_part_size) {
            extra_single_part_size = extra_single_part_size + mtd->erasesize;
        }
        extra_single_part_size = extra_single_part_size & (~((uint64_t)mtd->erasesize - 1));

        if (extra_single_part_size > mtd->erasesize) {
            extra += extra_single_part_size;
            bcm63XX_nand_parts[nr_parts].name = misc_mtd_partition_names[i];
            bcm63XX_nand_parts[nr_parts].offset = (nvram.ulNandPartOfsKb[NP_DATA] * 1024) - extra;
            bcm63XX_nand_parts[nr_parts].size = extra_single_part_size;
            bcm63XX_nand_parts[nr_parts].ecclayout = mtd->ecclayout;

            printk("setup_mtd_parts: name %s configured size 0x%08x offset 0x%llX\n"
                , misc_mtd_partition_names[i]
                , (int)bcm63XX_nand_parts[nr_parts].size
                , bcm63XX_nand_parts[nr_parts].offset);

            nr_parts++;
        }
    }

    /* Boot and NVRAM data */
    bcm63XX_nand_parts[3].offset = nvram.ulNandPartOfsKb[NP_BOOT] * 1024;
    bcm63XX_nand_parts[3].size = nvram.ulNandPartSizeKb[NP_BOOT] * 1024;
    bcm63XX_nand_parts[3].ecclayout = mtd->ecclayout;

    PRINTK("Part[0] name=%s, size=%llx, ofs=%llx\n", bcm63XX_nand_parts[0].name,
           bcm63XX_nand_parts[0].size, bcm63XX_nand_parts[0].offset);
    PRINTK("Part[1] name=%s, size=%llx, ofs=%llx\n", bcm63XX_nand_parts[1].name,
           bcm63XX_nand_parts[1].size, bcm63XX_nand_parts[1].offset);
    PRINTK("Part[2] name=%s, size=%llx, ofs=%llx\n", bcm63XX_nand_parts[2].name,
           bcm63XX_nand_parts[2].size, bcm63XX_nand_parts[2].offset);
    PRINTK("Part[3] name=%s, size=%llx, ofs=%llx\n", bcm63XX_nand_parts[3].name,
           bcm63XX_nand_parts[3].size, bcm63XX_nand_parts[3].offset);

    mtd_device_register(mtd, bcm63XX_nand_parts, nr_parts);

    return(nr_parts);
}
EXPORT_SYMBOL(setup_mtd_parts);
#endif
