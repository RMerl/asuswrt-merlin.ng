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

#include "bcm_assert_locks.h"
#include "bcmtypes.h"
#include "board.h"
#include "psp.h"
#include "flash_api.h"

#define CRC32_INIT_VALUE 0xffffffff /* Initial CRC32 checksum value */
#define CRC_LEN 4

#include "board_util.h"
#include "board_image.h"
#include "bcm_otp.h"
#include "emmc_linux_defs.h"

#include <linux/fs_struct.h>
//#define DEBUG_FLASH
//

DEFINE_MUTEX(flashImageMutex);

#include "board_nvram.h"

#if defined(CONFIG_BRCM_SMC_BOOT)
#include "vfbio_lvm.h"
#else
static int nandEraseBlk( struct mtd_info *mtd, int blk_addr );
static int nandWriteBlk(struct mtd_info *mtd, int blk_addr, int data_len, char *data_ptr, bool write_JFFS2_clean_marker);
#endif


#define ONEK                            1024
#define FLASH_LENGTH_SECURE_BOOT_ROM    (64*ONEK)

#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
#define FLASH_LENGTH_BOOT_ROM           (64*5*ONEK)
#else 
#if defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
    defined(_BCM947622_) || defined(CONFIG_BCM947622) || \
    defined(_BCM96756_) || defined(CONFIG_BCM96756)
/*
 when build cfe_fs_kernel nor image, bcmImageBuilder will add address for the first rootfs exactly
 after cfe, e.g. cfe is 128k, and in the brcm format and the first filetag for the first rootfs will be 0x20000
 but in the code, the fInfo.flash_rootfs_start_offset = FLASH_LENGTH_BOOT_ROM, that is 192k in previous
 setting, this will make update spi nor flash image( and spi nor .w image) fail in getting the first filetag
 so change it to 128k for now. will further investigate the impact 
 this is the case for both enable bootrom but not enable secure boot. but I think for legacy spi flash image, it 
 should also has this problem?
*/
/*as 128K is not enough for one spi cferom, need to change it back to 192k*/
#define FLASH_LENGTH_BOOT_ROM           (64*3*ONEK)
#else
#define FLASH_LENGTH_BOOT_ROM           (64*3*ONEK)
#endif
#endif




FLASH_ADDR_INFO fInfo;
struct semaphore semflash;
int flash_type = 0;

// mutex is preferred over semaphore to provide simple mutual exclusion
// spMutex protects scratch pad writes
DEFINE_MUTEX(spMutex);

extern struct mutex flashImageMutex;

/*
 * inMemNvramData is an in-memory copy of the nvram data that is in the flash.
 * This in-memory copy is used by NAND.  It is also used by NOR flash code
 * because it does not require a mutex or calls to memory allocation functions
 * which may sleep.  It is kept in sync with the flash copy by
 * updateInMemNvramData.
 */
DEFINE_MUTEX(inMemNvramData_mutex);
#define UNINITIALIZED_FLASH_DATA_CHAR  0xff
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

void sync_nvram_with_flash(void)
{
    int offset=0, ret=0;
    char *ptr = get_uboot_env_area(0);
    int length = get_uboot_env_area_size();

    BCM_ASSERT_NOT_HAS_MUTEX_C(&flashImageMutex);

    ((unsigned int*)ptr)[0]=0x75456e76;//uEnv
    ((unsigned int*)ptr)[1]=length;
    ((unsigned int*)ptr)[2]=crc32(CRC32_INIT_VALUE, ptr+UBOOT_HEADER_LEN, length-4)^CRC32_INIT_VALUE;

    mutex_lock(&flashImageMutex);
    length += UBOOT_HEADER_LEN-4;
    ret=set_uboot_env_flash(ptr, length, offset);
    if (ret != 0)
        printk("saving nvram/uboot_env failed %d\n", ret);
    mutex_unlock(&flashImageMutex);
}

static int setScratchPad(char *buf, int len);
static char *getScratchPad(int len);

// Variables not used in the simplified API used for the IKOS target
#if !defined(CONFIG_BRCM_IKOS)
#define MAX_BOOT_LOADER_VERSION_LENGTH 125
static char emmcBootPart = '1';
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
extern int __init bcm_get_root_propdata( const char * prop_name, char * data, int prop_size);
extern int __init bcm_get_root_propdata_raw( const char * prop_name, char ** data, int *prop_size );


#endif
#define BRCM_EMMC_BOOT_PART_PROP "emmc_boot_part"

#ifndef CONFIG_BCM_QEMU_SIM
void __init  kerSysEarlyFlashInitUbootEnv(void)
{
    char *uboot_env=NULL;
    char uboot_hdr[UBOOT_HEADER_LEN], *uboot_env_temp;
    int max_uboot_len=UBOOT_MAX_ENV_LEN;
    //if u-boot didn't set the u-boot env len , read the uboot_env from with its size
    if(bcm_get_root_propdata(BRCM_UBOOT_PROP, (char *)&uboot_hdr, sizeof(uboot_hdr)) != 0 || max_uboot_len >= 0x10000 )
    {
        if(bcm_get_root_propdata_raw(BRCM_UBOOT_PROP, &uboot_env_temp, &max_uboot_len) != 0)
        {
            printk("uboot_max_len missing, using defaults %d\n", UBOOT_MAX_ENV_LEN);
            max_uboot_len=UBOOT_MAX_ENV_LEN;
        }
        else
        {
	    //deduct the UBOOT_HEADER_LEN from the read prop size
            max_uboot_len-=UBOOT_HEADER_LEN;
        }
    }
    else
        max_uboot_len=((int*)uboot_hdr)[1];

    uboot_env=get_uboot_env_area(max_uboot_len); //get_uboot_env_area will allocate extra bytes for UBOOT_HEADER_LEN

    if(uboot_env)
    {
        if(bcm_get_root_propdata(BRCM_UBOOT_PROP, uboot_env, max_uboot_len+UBOOT_HEADER_LEN) != 0)
            printk("uboot env missing\n");
    }
}
#else
extern void kerSysEarlyFlashInitUbootEnv(void);
#endif

// Initialize the flash and fill out the fInfo structure
void __init kerSysEarlyFlashInit( void )
{

    flash_init();

    /* Get flash type detected by flash API */
    flash_type = flash_get_flash_type();

    kerSysEarlyFlashInitUbootEnv();

    if((flash_type == FLASH_IFC_UNSUP_EMMC))
    {
        if( bcm_get_root_propdata(BRCM_EMMC_BOOT_PART_PROP, (char*)&emmcBootPart, sizeof(emmcBootPart)) )
            printk("Failed to retrieve emmc boot partition number from dtb!\n");
        else
        {
            printk("Booted from eMMC boot partition %c\n", emmcBootPart);
        }
    }

}

#if !defined(CONFIG_BRCM_SMC_BOOT)
static int write_to_flash_at_offset(const char *input_data, int input_data_length, int address  )
{
    /* Image contains CFE ROM boot loader. */
    struct mtd_info *mtd = get_mtd_device_nm("loader"); 
    char *flash_data = NULL;
    size_t retlen = 0;
    unsigned int block=0, offset=0;
    
    if( IS_ERR_OR_NULL(mtd) )
        return -1;

    block = address/mtd->erasesize;
    offset = address%mtd->erasesize;

    if ( (flash_data = (char *)retriedKmalloc(mtd->erasesize)) == NULL )
    {
        printk("\n Failed to allocate memory in nandNvramSet();");
        return -1;
    }
    /* Read the whole cfe rom nand block */
    if( mtd_read(mtd, (block*mtd->erasesize), mtd->erasesize, &retlen, flash_data) < 0 )
    { 
        printk("\n Failed to read environment blocks in write_to_flash_at_offset();");
        retriedKfree(flash_data);
        return -1;
    }


    /* Copy the nvram string into place */
    memcpy(flash_data + offset, input_data, input_data_length);

    
    /* Flash the CFE ROM boot loader. */
    if (nandEraseBlk( mtd, (block*mtd->erasesize) ) == 0)
        nandWriteBlk(mtd, (block*mtd->erasesize), mtd->erasesize, flash_data, TRUE);

    retriedKfree(flash_data);
    return 0;
}

/* Erase the specified spinor flash block mtd. */
static int spinorEraseBlk( struct mtd_info *mtd, int blk_addr )
{
    struct erase_info erase;
    int sts;

    /* Erase the flash block. */
    memset(&erase, 0x00, sizeof(erase));
    erase.addr = blk_addr;
    erase.len = mtd->erasesize;

    sts = mtd_erase(mtd, &erase);

    if( 0 != sts )
        printk("spinorEraseBlk - Block 0x%8.8x. Error erasing block.\n", blk_addr);

    return (sts);
}

static int write_to_spinorflash_at_offset(const char *input_data, int input_data_length, int address  )
{
    /* Image contains loader. */
    struct mtd_info *mtd = get_mtd_device_nm("loader"); 
    char *flash_data = NULL;
    size_t retlen = 0;
    unsigned int block=0, offset=0, block_num=0,total_size=0,i;
    int ret = -1;
    
    if( IS_ERR_OR_NULL(mtd) )
        return -1;

    block = address/mtd->erasesize;
    offset = address%mtd->erasesize;
    block_num = (input_data_length + mtd->erasesize - 1)/mtd->erasesize;
    total_size= block_num*mtd->erasesize;
    
    
    if ( (flash_data = (char *)retriedKmalloc(total_size)) == NULL )
    {
        printk("\n Failed to allocate memory in write_to_spinorflash_at_offset();");
        return -1;
    }
    /* Read the environment  blocks */
    if( mtd_read(mtd, (block*mtd->erasesize), total_size, &retlen, flash_data) < 0 )
    {
        printk("\n Failed to read environment blocks in write_to_spinorflash_at_offset();");
        retriedKfree(flash_data);
        return -1;
    }

    /* Copy the environment string into place */
    memcpy(flash_data + offset, input_data, input_data_length);
    
    /* Flash the environemnt blocks  loader. */
    for(i = 0; i < block_num; i++)
    {
        ret = spinorEraseBlk( mtd, ((block+i)*mtd->erasesize));
        if(ret)
            break;
    }
    if(ret == 0)
    {
       ret = mtd_write(mtd, (block*mtd->erasesize), total_size, &retlen, flash_data);
    }
    retriedKfree(flash_data);
    return ret;
}
#endif
#endif //  !defined(CONFIG_BRCM_IKOS)

int kerSysFsFileGetOffset(const char *filename, char *buf, int len, loff_t offset)
{
    struct file *fp;
    int ret = 0;

    if( in_atomic() )
    {
        printk("Error: %s called in atomic context. FS r/w calls may block!\n",
            __FUNCTION__ );
        dump_stack();
        return -1;
    }

    if (!buf)
        return -1;

    memset(buf, 0, len);
	        
    fp = filp_open(filename, O_RDONLY| O_NONBLOCK, 0);
    if (IS_ERR(fp))
        return -1;

    fp->f_pos = offset;

    if((int)kernel_read(fp, buf, len, &fp->f_pos) <= 0)            
    {
        printk("Failed to get data from '%s'\n", filename);
        ret = -1;
    }

    filp_close(fp, NULL);

    return ret;
}
int kerSysFsFileSetOffset(const char *filename, char *buf, int len, loff_t offset)
{
    int ret = -1;
    struct file *fp;

    if( in_atomic() )
    {
        printk("Error: %s called in atomic context. FS r/w calls may block!\n",
            __FUNCTION__ );
        dump_stack();
        return ret;
    }


    fp = filp_open(filename, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);

    if (!IS_ERR(fp))
    {
        fp->f_pos = offset;

        if((int) kernel_write(fp, (void *) buf, len, &fp->f_pos) == len)
        {
            vfs_fsync(fp, 0);
            ret = 0;
        }
        filp_close(fp, NULL);
    }

    if (ret != 0)
    {
        printk("Failed to write to '%s'.\n", filename);
    }

    return( ret );
}

int set_uboot_env_flash(const char *input_data, int length, int offset)
{
    int ret=0, idx=0;
#if defined(CONFIG_BRCM_SMC_BOOT)
    char name[32];
    vfbio_lun_descr lun_info;
    int id;
    
    ret = -1;
    for (idx = 1; idx <= 2; idx++)
    {
        sprintf(name, "ubootenv%d", idx);
        if (vfbio_lun_get_id(name, &id))
        {
            continue;
        }
        vfbio_lun_get_info(id, &lun_info);
        if((length + lun_info.block_size - 1)/lun_info.block_size > lun_info.size_in_blocks)
        {
            printk(KERN_ERR "Not enough space to burn %s image - available %d, needed %u blocks\n", name, lun_info.size_in_blocks * lun_info.block_size, length);
            continue;
        }

        if(vfbio_lun_write(id, (void *)input_data, length))
            printk(KERN_ERR "\n** Unable to write env to LUN %s (%d) **\n", name, id);
        else
            ret = 0;
    }
#else
    char temp_buff[128]; // 128 bytes is enough to hold more many backup locations  
    char *ptr=NULL;
    unsigned int address;

    //get env_boot_magic=16384@0x40000,0xa0000
    if (eNvramGet("env_boot_magic", temp_buff, sizeof(temp_buff)) > 0)
    {
        ptr=strchr(temp_buff, '@');
        if(ptr)
        {
            ptr++;
            while(ptr)
            {
                address=simple_strtoul(ptr, &ptr, 16);
                if(address)
                {
                    switch(flash_type)
                    {
                        case FLASH_IFC_SPI:
                        case FLASH_IFC_HS_SPI:
                            ret = write_to_spinorflash_at_offset(input_data, length, address);
                            break;
                        case FLASH_IFC_NAND:
                        case FLASH_IFC_SPINAND:
                            ret = write_to_flash_at_offset(input_data, length, address);
                            break;
                        case FLASH_IFC_UNSUP_EMMC:
                            idx = emmcBootPart - '1';
                            if(idx == 0 || idx == 1)
                            { 
                                snprintf(temp_buff, sizeof(temp_buff), "/sys/block/mmcblk0boot%d/force_ro", idx);
                                ret = kerSysFsFileSetOffset(temp_buff, "0", strlen("0"), 0 );
                                snprintf(temp_buff, sizeof(temp_buff), "/dev/mmcblk0boot%d", idx);
                                ret = kerSysFsFileSetOffset(temp_buff, (char*)input_data, length, (loff_t)address );
                            }
                            break;
                    }
                    if(ret != 0)
                    {
                        printk("failed to write uboot_env at %d [%d]\n", address, ret); 
                    } 
                }
                if(ptr[0] == ',')
                {
                    ptr++; 
                }
                else
                    break; 
            }
        }

    }
    else
       ret=-1;
#endif
    
    return ret;
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
    return(kerSysFsFileSetOffset(filename, buf, len, 0));
}

int kerSysFsFileGet(const char *filename, char *buf, int len)
{
    return(kerSysFsFileGetOffset(filename, buf, len, 0));
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

#if !defined(CONFIG_BRCM_SMC_BOOT)
/* Erase the specified NAND flash block. */
static int nandEraseBlk( struct mtd_info *mtd, int blk_addr )
{
    struct erase_info erase;
    int sts;

    /* Erase the flash block. */
    memset(&erase, 0x00, sizeof(erase));
    erase.addr = blk_addr;
    erase.len = mtd->erasesize;

    if( (sts = mtd_block_isbad(mtd, blk_addr)) == 0 )
    {
        sts = mtd_erase(mtd, &erase);

        /* Function local_bh_disable has been called and this
         * is the only operation that should be occurring.
         * Therefore, spin waiting for erase to complete.
         */
        if( 0 != sts )
            printk("nandEraseBlk - Block 0x%8.8x. Error erasing block.\n", blk_addr);
    }
    else
        printk("nandEraseBlk - Block 0x%8.8x. Bad block.\n", blk_addr);

    return (sts);
}

#define WAR_NAND_WRITE_BLK       1        /* This WAR is for MXIC parallel NAND, for ex. MX30LFxG28AD */
#if defined(WAR_NAND_WRITE_BLK)
#define WRITE_OOB_RETRY_COUNT    3
#endif

/* Write data with or without JFFS2 clean marker, must pass function an aligned block address */
static int nandWriteBlk(struct mtd_info *mtd, int blk_addr, int data_len, char *data_ptr, bool write_JFFS2_clean_marker)
{
#if !defined(CONFIG_CPU_BIG_ENDIAN)
    const unsigned short jffs2_clean_marker[] = {JFFS2_MAGIC_BITMASK, JFFS2_NODETYPE_CLEANMARKER, 0x0008, 0x0000};
#else
    const unsigned short jffs2_clean_marker[] = {JFFS2_MAGIC_BITMASK, JFFS2_NODETYPE_CLEANMARKER, 0x0000, 0x0008};
#endif
    struct mtd_oob_ops ops;
    int sts = 0;
    int page_addr, byte;
#if defined(WAR_NAND_WRITE_BLK)
    int retry;
#endif

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
#if defined(WAR_NAND_WRITE_BLK)
            retry = WRITE_OOB_RETRY_COUNT;
            while (retry--)
            {
                if( (sts = mtd_write_oob(mtd, blk_addr + page_addr, &ops)) == 0 )
                    break;
                else
                {
                    printk("nandWriteBlk - Block 0x%8.8x. Error writing page. retry count %d\n",
                            blk_addr + page_addr, WRITE_OOB_RETRY_COUNT - retry);
                    /* Just skip this page, don't erase this block and markbad */
                }
            }
#else
            if( (sts = mtd_write_oob(mtd, blk_addr + page_addr, &ops)) != 0 )
            {
                printk("nandWriteBlk - Block 0x%8.8x. Error writing page.\n", blk_addr + page_addr);
                nandEraseBlk(mtd, blk_addr);
                mtd_block_markbad(mtd, blk_addr);
                break;
            }
#endif
        }
    }

    return(sts);
}
#endif //#if !defined(CONFIG_BRCM_SMC_BOOT)

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
            if((pShareBuf = getScratchPad(SP_MAX_LEN)) == NULL) {
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

    if(pBuf == NULL || memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
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
           ((usedLen + pToken->tokenLen) <= SP_MAX_LEN))
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

    /* Always clean the top Buf to avoid returning random value. */
    if(tokBuf)
        memset(tokBuf, 0, bufLen);
    else
    {
        mutex_unlock(&spMutex);
        return sts;
    }
 

    switch (flash_type) 
    {
        case FLASH_IFC_UNSUP_EMMC:
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:
            if((pShareBuf = getScratchPad(SP_MAX_LEN)) == NULL) {
                mutex_unlock(&spMutex);
                return sts;
            }

            pBuf = pShareBuf;
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if (disable_nor_raw_partition)
            {
                if((pShareBuf = getScratchPad(SP_MAX_LEN)) == NULL) {
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

    if(pBuf == NULL || memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
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
        pToken->tokenLen < SP_MAX_LEN &&
        usedLen < SP_MAX_LEN )
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

    if( bufLen >= SP_MAX_LEN - sizeof(SP_HEADER) -
        sizeof(SP_TOKEN) )
    {
        printk("Scratch pad overflow by %d bytes.  Information not saved.\n",
            bufLen  - SP_MAX_LEN - (int)sizeof(SP_HEADER) -
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
            if((pShareBuf = getScratchPad(SP_MAX_LEN)) == NULL)
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
                if((pShareBuf = getScratchPad(SP_MAX_LEN)) == NULL)
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

    if(pBuf == NULL)
        return sts;

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0)
    {
        // new sp, so just flash the token
        printk("No scratch pad found.  Initialize scratch pad...\n");
        memset(pBuf, 0x0, SP_MAX_LEN);
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
            pToken->tokenLen < SP_MAX_LEN &&
            usedLen < SP_MAX_LEN )
        {
            usedLen += sizeof(SP_TOKEN) + skipLen;
            curPtr += sizeof(SP_TOKEN) + skipLen;
            pToken = (PSP_TOKEN) curPtr;
            skipLen = (pToken->tokenLen + 0x03) & ~0x03;
        }

        if( usedLen + SPToken.tokenLen + sizeof(SP_TOKEN) >
            SP_MAX_LEN )
        {
            printk("Scratch pad overflow by %d bytes.  Information not saved.\n",
                (usedLen + SPToken.tokenLen + (int)sizeof(SP_TOKEN)) -
                SP_MAX_LEN);
            retriedKfree(pShareBuf);
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
                        SP_MAX_LEN - (curLen + copyLen) );
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
            sts = setScratchPad(pShareBuf, SP_MAX_LEN);
            break;

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
            if (disable_nor_raw_partition)
            {
                sts = setScratchPad(pShareBuf, SP_MAX_LEN);
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
            if((pShareBuf = getScratchPad(SP_MAX_LEN)) == NULL)
            {
                mutex_unlock(&spMutex);
                return sts;
            }

            memset(pShareBuf, 0x00, SP_MAX_LEN);

            setScratchPad(pShareBuf, SP_MAX_LEN);
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
                memset(pShareBuf + fInfo.flash_scratch_pad_blk_offset, 0x00, SP_MAX_LEN) ;
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


#if !defined(CONFIG_BRCM_SMC_BOOT)
static unsigned int getPartitionSize(char * dev_name)
{
    // Open file
    unsigned long num_512b_blocks = 0;
    struct file* f = filp_open(dev_name, O_RDONLY, 0);
    if (IS_ERR(f)) 
    {
        f = NULL;
    }
    else
    {
        /* Get size of device in 512Bytes blocks */
        f->f_op->unlocked_ioctl(f, BLKGETSIZE, (unsigned long)&num_512b_blocks);
        
        // Close file
        filp_close(f, 0);
    }

    return num_512b_blocks * 512;
}
#endif //if !defined(CONFIG_BRCM_SMC_BOOT)

#define SPI_NOR_FLASH_NAME "spi-nor.0"
int kerSysFlashSizeGet(void)
{
    int ret = 0;
#if !defined(CONFIG_BRCM_SMC_BOOT)
    struct mtd_info *mtd;
#endif //#if !defined(CONFIG_BRCM_SMC_BOOT)

    switch (flash_type) 
    {
#if !defined(CONFIG_BRCM_SMC_BOOT)
        case FLASH_IFC_UNSUP_EMMC:
            ret += getPartitionSize(EMMC_DEV_PNAME_CFE);
            ret += getPartitionSize(EMMC_DEV_PNAME_BOOTFS(1));
            ret += getPartitionSize(EMMC_DEV_PNAME_ROOTFS(1));
           break;
        case FLASH_IFC_NAND:
        case FLASH_IFC_SPINAND:

            if( (mtd = get_mtd_device_nm("image")) != NULL )
            {
                ret = mtd->size;
                put_mtd_device(mtd);
            }
            break;
#endif //#if !defined(CONFIG_BRCM_SMC_BOOT)

        case FLASH_IFC_SPI:
        case FLASH_IFC_HS_SPI:
#if (defined(CONFIG_SPI_BCM63XX_HSSPI) || defined(CONFIG_SPI_BCMBCA_HSSPI)) && defined(CONFIG_MTD_SPI_NOR)
                if( (mtd = get_mtd_device_nm(SPI_NOR_FLASH_NAME)) != NULL )
                {
                    ret = mtd->size;
                    put_mtd_device(mtd);
                }
#else
            ret = flash_get_total_size();
#endif
            break;

        default:
            printk("ERROR: %s:Unknown flash type\n", __FUNCTION__);
            break;
    }

    return ret;
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
