/***************************************************************************
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
 ***************************************************************************
 * File Name  : bcm63xx_flash.c
 *
 * Description: This file contains the flash device driver for bcm63xx board. Very similar to
 *              board.c in linux development.
 *
 * Created on :  4/18/2002  seanl
 *
 ***************************************************************************/


/* Includes. */
#include "lib_types.h"
#include "lib_malloc.h"
#include "lib_string.h"
#include "lib_printf.h"

#include "bcm_map.h"       
#include "bcm_hwdefs.h"
#include "dev_bcm63xx_flash.h"
#include "flash_api.h"
#include "boardparms.h"
#include "boardparms_voice.h"
#include "bcm63xx_util.h"
#include "rom_parms.h"
#include "bcm_memory.h"
#include "lib_math.h"
#include "bcm63xx_nvram.h"
#include "bcm_otp.h"

//#define DEBUG_FLASH 

/* This driver determines the NVRAM and persistent storage flash address and
 * length.
 */

static char flashBootCfeVersion[CFE_VERSION_MARK_SIZE+CFE_VERSION_SIZE] = {0}; 
static FLASH_ADDR_INFO fInfo;

//**************************************************************************************
// Flash read/write and image downloading..
//**************************************************************************************
void kerSysFlashInit( void )
{
    if( flash_init() != FLASH_API_ERROR )
    {
#if (SKIP_FLASH==0)
        if(NVRAM_INIT()) {
           printf("ERROR: Can't initialize NVRAM\n");
        } 
        while ((BpSetBoardId((char*)NVRAM.szBoardId) != BP_SUCCESS))
        {
            printf("\n*** Board is not initialized properly ***\n\n");
            setBoardParam();
        }

        fInfo.flash_rootfs_start_offset = flash_get_sector_size(0);

#if defined (_BCM96838_) && (INC_NAND_FLASH_DRIVER==0)
        if( fInfo.flash_rootfs_start_offset < PSRAM_SIZE )
            fInfo.flash_rootfs_start_offset = PSRAM_SIZE;
#elif defined (_BCM96848_) && (INC_NAND_FLASH_DRIVER==0)
            fInfo.flash_rootfs_start_offset = 192*1024;
#else
        if( fInfo.flash_rootfs_start_offset < FLASH_LENGTH_BOOT_ROM )
            fInfo.flash_rootfs_start_offset = FLASH_LENGTH_BOOT_ROM;
#endif

        fInfo.flash_rootfs_start_offset += IMAGE_OFFSET;

        flash_init_info(NVRAM_RP, &fInfo);

#if (INC_NAND_FLASH_DRIVER==1)
        validateNandPartTbl(0, 0);
#endif
#endif
    }
}
 
#if (INC_NAND_FLASH_DRIVER==1) || (INC_SPI_PROG_NAND==1)

void erase_misc_partition(int pnum, const NVRAM_DATA *nvram_data)
{
unsigned long total_dps, data_partition_size=0, ulBlockSizeKb, offset=0, partition_size=0, start_of_partition=0;
int parts_to_iterate;

    ulBlockSizeKb=flash_get_sector_size(0);
    total_dps=0;

    if(nvram_data->part_info[pnum].size != 0xffff) {
        printf("\nErasing Misc partition %d", pnum+1); // 0 indexed
        data_partition_size=convert_to_data_partition_entry_to_bytes(nvram_data->part_info[pnum].size);
        parts_to_iterate=3-pnum;
        pnum=BCM_MAX_EXTRA_PARTITIONS-2;//always start with 2,  iterate to 0 as per parts_to_iterate

        while(parts_to_iterate > 0) {

            partition_size=convert_to_data_partition_entry_to_bytes(nvram_data->part_info[pnum].size);
            if ((partition_size & ~(ulBlockSizeKb-1)) != partition_size)
                partition_size=partition_size+ulBlockSizeKb;

            partition_size = partition_size&~(ulBlockSizeKb-1);
            total_dps += partition_size;
            pnum--;    
            parts_to_iterate--;

        }
        start_of_partition=offset = (nvram_data->ulNandPartOfsKb[NP_DATA] * 1024)-total_dps;

        printf(" from %d to %d\n", offset/ulBlockSizeKb, (start_of_partition+data_partition_size)/ulBlockSizeKb);

        while(offset < start_of_partition+data_partition_size)
        {
                flash_sector_erase_int(offset/ulBlockSizeKb);
                printf(".");
                offset+=ulBlockSizeKb;
        }
    }
}

/***********************************************************************
 * Function Name: getNumBootBlks
 * Description  : Retrieves the number of boot blocks currently defined 
 *                within the partition table
 * Returns      : 0 if partition table is untrustworthy
 *                1 if legacy flash partitioning
 *                >1 if either lilac, or headered cferoms exist on flash
 ***********************************************************************/
uint32_t getNumBootBlks(const NVRAM_DATA* pNvramData)
{
    uint32_t numBootBlocks = 0;
    unsigned long ulBlockSizeKb = (unsigned long)flash_get_sector_size(0)/1024;
    unsigned long data_partition_size = min_data_partition_size_kb();


    if(pNvramData->part_info[3].size != 0xffff ) {
        data_partition_size=((convert_to_data_partition_entry_to_bytes(pNvramData->part_info[3].size))/ulBlockSizeKb) * ulBlockSizeKb;
        data_partition_size = data_partition_size /1024; // we need to get the partition size in KBs, instead of absolute size
    }

#if (INC_SPI_PROG_NAND==1)
    if( flash_get_flash_type() != FLASH_IFC_NAND )
       return numBootBlocks;
#endif
    if( pNvramData->ulNandPartSizeKb[NP_DATA] == data_partition_size &&
        (pNvramData->ulNandPartSizeKb[NP_BBT] == NAND_BBT_SMALL_SIZE_KB ||
         pNvramData->ulNandPartSizeKb[NP_BBT] == NAND_BBT_BIG_SIZE_KB) )
    {
       /* Assume the partition table is correct */
        numBootBlocks = pNvramData->ulNandPartSizeKb[NP_BOOT] / ulBlockSizeKb;
    }
    return numBootBlocks;
}
	
/***********************************************************************
 * Function Name: validateNandPartTbl
 * Description  : Checks the NAND partition table fields in NVRAM data.
 *                If any of the fields are not valid, new values are set.
 * Returns      : 0 - if update to NVRAM is requried  
 ***********************************************************************/
int validateNandPartTbl(uint32_t frcUpdt, uint32_t numBtBlks)
{
    unsigned long ulBlockSizeKb = (unsigned long)flash_get_sector_size(0)/1024;
    unsigned long ulTotalSizeKb = (unsigned long)flash_get_total_size() / 1024;
    int i, res = 1;
    unsigned long extra = 0, data_partition_size = min_data_partition_size_kb();

    if(NVRAM.part_info[3].size != 0xffff && NVRAM.part_info[3].size != 0x000 ) {
        data_partition_size=convert_to_data_partition_entry_to_bytes(NVRAM.part_info[3].size);
        data_partition_size=( data_partition_size /ulBlockSizeKb) * ulBlockSizeKb;
        data_partition_size = data_partition_size /1024; // we need to get the partition size in KBs, instead of absolute size

    }

#if (INC_SPI_PROG_NAND==1)
    if( flash_get_flash_type() != FLASH_IFC_NAND )
        return res;
#endif
    /*
     * if frcUpdt is 0, that means this function gets called from places other than the setPartitionsSizes,
     *    if the pNvramData->part_info[3] is not same as pNvramData->ulNandPartSizeKb[NP_DATA], and the 
     *    pNvramData->ulNandPartSizeKb[NP_DATA] is not 4MB, do not try to repartition,
     *    We will force a partition update when the user changes the partition
     *    In case of an upgrade from 4.16.02 to latest, if the parti_info holds garbage data, it will not match with ulNandPartSizeKb[NP_DATA]
     *    then don't try to update it. 
     * */
    if((((NVRAM.ulNandPartSizeKb[NP_DATA] != data_partition_size) && (min_data_partition_size_kb() != NVRAM.ulNandPartSizeKb[NP_DATA])) ||
        (NVRAM.ulNandPartSizeKb[NP_BBT] != NAND_BBT_SMALL_SIZE_KB &&
         NVRAM.ulNandPartSizeKb[NP_BBT] != NAND_BBT_BIG_SIZE_KB) ) ||
         (frcUpdt) )
    {
        /* The CFE ROM boot loader saved the rootfs partition index at the
         * memory location before CFE RAM load address.
         */
        /* 63138 and 63148 use 16KB below the cfe ram image as the mmu table,
         * so rfs number, cfe number has to be saved 16K down further */
#ifndef SINGLE_IMAGE
        int rootfs = (int)CFE_RAM_ROM_PARMS_GET_ROOTFS;
#endif /* ! SINGLE_IMAGE */
        /* Initialize NAND flash partition table. */
        unsigned long ulRootfsSizeKb;
        unsigned long ulBbtSizeKb = (ulTotalSizeKb > NAND_BBT_THRESHOLD_KB)
            ? NAND_BBT_BIG_SIZE_KB : NAND_BBT_SMALL_SIZE_KB;
#ifndef SINGLE_IMAGE
        unsigned long ulOldDataSizeKb = NVRAM.ulNandPartSizeKb[NP_DATA];
        unsigned long ulOldRfs2OfsKb = NVRAM.ulNandPartOfsKb[NP_ROOTFS_2];
        unsigned long ulOldRfs2SizeKb = NVRAM.ulNandPartSizeKb[NP_ROOTFS_2];
#endif /* ! SINGLE_IMAGE */

        printf("Updating the NAND Flash Partition Table\n");
        printf("Old Table\n");
        printf("boot    offset=0x%8.8lx, size=0x%8.8lx\n",
            NVRAM.ulNandPartOfsKb[NP_BOOT] * 1024,
            NVRAM.ulNandPartSizeKb[NP_BOOT] * 1024);
        printf("rootfs1 offset=0x%8.8lx, size=0x%8.8lx\n",
            NVRAM.ulNandPartOfsKb[NP_ROOTFS_1] * 1024,
            NVRAM.ulNandPartSizeKb[NP_ROOTFS_1] * 1024);
        printf("rootfs2 offset=0x%8.8lx, size=0x%8.8lx\n",
            NVRAM.ulNandPartOfsKb[NP_ROOTFS_2] * 1024,
            NVRAM.ulNandPartSizeKb[NP_ROOTFS_2] * 1024);
        printf("data    offset=0x%8.8lx, size=0x%8.8lx\n",
            NVRAM.ulNandPartOfsKb[NP_DATA] * 1024,
            NVRAM.ulNandPartSizeKb[NP_DATA] * 1024);
        printf("bbt     offset=0x%8.8lx, size=0x%8.8lx\n\n",
            NVRAM.ulNandPartOfsKb[NP_BBT] * 1024,
            NVRAM.ulNandPartSizeKb[NP_BBT] * 1024);

        /* The Boot partition is first, so offset is 0 */
        NVRAM_SET(ulNandPartOfsKb[NP_BOOT], unsigned int, 0);

#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || \
        defined(_BCM96846_) || defined(_BCM96856_)
        if(numBtBlks == 0) {
            i = findBootBlock();
            if(i==-1)
                numBtBlks=1;
            else
                numBtBlks=i;
        }
#endif

#if defined(_BCM96838_) || defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || \
        defined (_BCM963148_) || defined(_BCM94908_) || defined(_BCM96858_) || \
        defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_)
	/* The boot partition is numBtBlks in size */
        if (((frcUpdt) && (numBtBlks > 1)) || (bcm_otp_is_btrm_boot()))
          { NVRAM_SET(ulNandPartSizeKb[NP_BOOT], unsigned int, ulBlockSizeKb * numBtBlks);}
        else
#endif
          { NVRAM_SET(ulNandPartSizeKb[NP_BOOT], unsigned int, ulBlockSizeKb);}

        /* The Bad Block Table partition is last and is a constant size. */
        NVRAM_SET(ulNandPartOfsKb[NP_BBT], unsigned int, ulTotalSizeKb - ulBbtSizeKb);
        NVRAM_SET(ulNandPartSizeKb[NP_BBT], unsigned int, ulBbtSizeKb);

        if(NVRAM.part_info[3].size != 0xffff && NVRAM.part_info[3].size != 0x0) {
            data_partition_size=(( convert_to_data_partition_entry_to_bytes( NVRAM.part_info[3].size ))/ulBlockSizeKb) * ulBlockSizeKb;
            data_partition_size = data_partition_size /1024; // we need to get the partition size in KBs, instead of absolute size
        }

        /* The Data partition is before the BBT. */
        NVRAM_SET(ulNandPartOfsKb[NP_DATA], unsigned int, NVRAM.ulNandPartOfsKb[NP_BBT] - data_partition_size);
        NVRAM_SET(ulNandPartSizeKb[NP_DATA], unsigned int, data_partition_size);

        /* The first rootfs partition starts after Boot partition */
        NVRAM_SET(ulNandPartOfsKb[NP_ROOTFS_1], unsigned int, NVRAM.ulNandPartOfsKb[NP_BOOT] + 
            NVRAM.ulNandPartSizeKb[NP_BOOT]);

        for(i = 0; i < 3; i++) {
            if(NVRAM.part_info[i].size != 0xffff ) {
                data_partition_size=(( convert_to_data_partition_entry_to_bytes( NVRAM.part_info[i].size ))/ulBlockSizeKb) * ulBlockSizeKb;
                data_partition_size = data_partition_size /1024; // we need to get the partition size in KBs, instead of absolute size
                extra+=data_partition_size;
            }
        }
        /* The size of the two root file system partitions is whatever is left
         * after the Boot, Data and BBT partitions divided by 2 and evenly
         * divisible by the NAND flash block size.
         */
#ifdef SINGLE_IMAGE
	ulRootfsSizeKb = ((NVRAM.ulNandPartOfsKb[NP_DATA] - extra -
            NVRAM.ulNandPartOfsKb[NP_ROOTFS_1]));
#else
	ulRootfsSizeKb = ((NVRAM.ulNandPartOfsKb[NP_DATA] - extra -
	    NVRAM.ulNandPartOfsKb[NP_ROOTFS_1]) / 2);
#endif
        ulRootfsSizeKb = (ulRootfsSizeKb / ulBlockSizeKb) * ulBlockSizeKb;

#ifdef SINGLE_IMAGE
        printf("single image mode .................\n");
        NVRAM_SET(ulNandPartSizeKb[NP_ROOTFS_1], unsigned int, ulRootfsSizeKb);
        NVRAM_SET(ulNandPartOfsKb[NP_ROOTFS_2], unsigned int,
            NVRAM.ulNandPartOfsKb[NP_ROOTFS_1]);
        NVRAM_SET(ulNandPartSizeKb[NP_ROOTFS_2], unsigned int,
            NVRAM.ulNandPartSizeKb[NP_ROOTFS_1]);
#else /* ! SINGLE_IMAGE */
        NVRAM_SET(ulNandPartSizeKb[NP_ROOTFS_1], unsigned int, ulRootfsSizeKb);

        NVRAM_SET(ulNandPartOfsKb[NP_ROOTFS_2], unsigned int,
            NVRAM.ulNandPartOfsKb[NP_ROOTFS_1] + ulRootfsSizeKb);
        NVRAM_SET(ulNandPartSizeKb[NP_ROOTFS_2], unsigned int, ulRootfsSizeKb);

        /* If the CFE RAM booted from the second partition, the partition table
         * is valid (it's been previously written) and the size of the data
         * partition changed, then the start of the second partition will change
         * which will cause the CFE RAM boot loader to not find vmlinux.  So
         * adjust the addresses and lengths so the second rootfs remains at the
         * address that it was set to before the data partition size changed.
         */
        if( (rootfs == NP_ROOTFS_2) && ulOldRfs2SizeKb &&
            ((ulOldRfs2SizeKb & 0xff000000) != 0xff000000) &&
            ulOldDataSizeKb && (ulOldDataSizeKb < min_data_partition_size_kb()) &&
            (ulOldRfs2OfsKb > NVRAM.ulNandPartOfsKb[NP_ROOTFS_2]) )
        {
            unsigned long ulOfsDiff = 
                ulOldRfs2OfsKb - NVRAM.ulNandPartOfsKb[NP_ROOTFS_2];

            int blk_size = flash_get_sector_size(0) / 1024;
            int blk_start = NVRAM.ulNandPartOfsKb[NP_DATA] / blk_size;
            int total_blks =
                blk_start + (min_data_partition_size_kb() - ulOldDataSizeKb) / blk_size;

            NVRAM_SET(ulNandPartOfsKb[NP_ROOTFS_2], unsigned int, NVRAM.ulNandPartOfsKb[NP_ROOTFS_2] + ulOfsDiff);
            NVRAM_SET(ulNandPartSizeKb[NP_ROOTFS_2], unsigned int, NVRAM.ulNandPartSizeKb[NP_ROOTFS_2] - ulOfsDiff);
            NVRAM_SET(ulNandPartSizeKb[NP_ROOTFS_1], unsigned int, NVRAM.ulNandPartSizeKb[NP_ROOTFS_1] + ulOfsDiff);

            /* Erase the flash blocks from the start of the new data partition
             * to the start of the old data partition.
             */
            while( blk_start < total_blks ) {
                flash_sector_erase_int(blk_start);
                blk_start++;
            }
        }
#endif /* ! SINGLE_IMAGE */

        NVRAM_UPDATE(NULL);

        printf("New Table\n");
        printf("boot    offset=0x%8.8lx, size=0x%8.8lx\n",
            NVRAM.ulNandPartOfsKb[NP_BOOT] * 1024,
            NVRAM.ulNandPartSizeKb[NP_BOOT] * 1024);
        printf("rootfs1 offset=0x%8.8lx, size=0x%8.8lx\n",
            NVRAM.ulNandPartOfsKb[NP_ROOTFS_1] * 1024,
            NVRAM.ulNandPartSizeKb[NP_ROOTFS_1] * 1024);
        printf("rootfs2 offset=0x%8.8lx, size=0x%8.8lx\n",
            NVRAM.ulNandPartOfsKb[NP_ROOTFS_2] * 1024,
            NVRAM.ulNandPartSizeKb[NP_ROOTFS_2] * 1024);
        printf("data    offset=0x%8.8lx, size=0x%8.8lx\n",
            NVRAM.ulNandPartOfsKb[NP_DATA] * 1024,
            NVRAM.ulNandPartSizeKb[NP_DATA] * 1024);
        printf("bbt     offset=0x%8.8lx, size=0x%8.8lx\n\n",
            NVRAM.ulNandPartOfsKb[NP_BBT] * 1024,
            NVRAM.ulNandPartSizeKb[NP_BBT] * 1024);

        { // erase data partition if partition table has changed
            unsigned short block, end;
            int size;

            size = flash_get_sector_size(0);
            block = (NVRAM.ulNandPartOfsKb[NP_DATA] * 1024) / size;
            end = (NVRAM.ulNandPartOfsKb[NP_DATA] + NVRAM.ulNandPartSizeKb[NP_DATA]) * 1024 / size;

            printf("\nErasing data partition ");
            printf("from block %d to %d\n", block, end);
            for(; block < end; block++)
            {
                flash_sector_erase_int(block);
                printf(".");
            }
            printf("\n");
        }
        res = 0;
    }

    return res;
}
#else
int validateNandPartTbl(uint32_t frcUpdt, uint32_t numBtBlks)
{
     return 0;
}
uint32_t getNumBootBlks(const NVRAM_DATA* pNvramData)
{
   return 1;
}
void erase_misc_partition(int pnum, const NVRAM_DATA *nvram_data)
{
}
#endif


/***********************************************************************
 * Function Name: kerSysFlashAddrInfoGet
 * Description  : Fills in a structure with information about the NVRAM
 *                and persistent storage sections of flash memory.
 * Returns      : None.
 ***********************************************************************/
void kerSysFlashAddrInfoGet(PFLASH_ADDR_INFO pflash_addr_info)
{
    memcpy(pflash_addr_info, &fInfo, sizeof(FLASH_ADDR_INFO));
}

#if (INC_NAND_FLASH_DRIVER!=1)
// get shared blks into *** pTempBuf *** which has to be released bye the caller!
// return: if pTempBuf != NULL, poits to the data with the dataSize of the buffer
// !NULL -- ok
// NULL  -- fail
static unsigned char *getSharedBlks(int start_blk, int num_blks)
{
    int i = 0;
    int usedBlkSize = 0;
    int sect_size = 0;
    unsigned char *pTempBuf = NULL;
    unsigned char *pBuf = NULL;

    for (i = start_blk; i < (start_blk + num_blks); i++)
        usedBlkSize += flash_get_sector_size((unsigned short) i);

    if ((pTempBuf = (unsigned char *) KMALLOC(usedBlkSize, sizeof(long))) == NULL)
    {
        printf("failed to allocate memory with size: %d\n", usedBlkSize);
        return pTempBuf;
    }
    
    pBuf = pTempBuf;
    for (i = start_blk; i < (start_blk + num_blks); i++)
    {
        sect_size = flash_get_sector_size((unsigned short) i);
#if defined(DEBUG_FLASH)
        printf("getShareBlks: blk=%d, sect_size=%d\n", i, sect_size);
#endif
        flash_read_buf((unsigned short)i, 0, pBuf, sect_size);
        pBuf += sect_size;
    }
    
    return pTempBuf;
}

// Set the pTempBuf to flash from start_blk for num_blks
// return:
// 0 -- ok
// -1 -- fail
static int setSharedBlks(int start_blk, int num_blks, unsigned char *pTempBuf)
{
    int i = 0;
    int sect_size = 0;
    int sts = 0;
    unsigned char *pBuf = pTempBuf;

    for (i = start_blk; i < (start_blk + num_blks); i++)
    {
        sect_size = flash_get_sector_size((unsigned short) i);
        flash_sector_erase_int(i);
        if (flash_write_buf(i, 0, pBuf, sect_size) != sect_size)
        {
            printf("Error writing flash sector %d.", i);
            sts = -1;
            break;
        }

#if defined(DEBUG_FLASH)
        printf("setShareBlks: blk=%d, sect_size=%d\n", i, sect_size);
#endif

        pBuf += sect_size;
    }

    return sts;
}
#endif




/*******************************************************************************
 * PSI functions
 *******************************************************************************/

#if (INC_NAND_FLASH_DRIVER!=1)
/** set psi while preserving any other data that might be sharing sectors with
 *  the psi, e.g. scratch pad.
 *
 * @param string (IN) buffer that holds the data to be written.
 * @param strLen (IN) length of buffer.
 *
 * @return 0 if OK, -1 on failure.
 */
static int kerSysPsiSet(const unsigned char *string, int strLen)
{
    int sts = 0;
    unsigned char *pBuf = NULL;

    if ((pBuf = getSharedBlks(fInfo.flash_persistent_start_blk,
        fInfo.flash_persistent_number_blk)) == NULL)
        return -1;

    // set string to the memory buffer
    memcpy((pBuf + fInfo.flash_persistent_blk_offset), string, strLen);

    if (setSharedBlks(fInfo.flash_persistent_start_blk, 
        fInfo.flash_persistent_number_blk, pBuf) != 0)
        sts = -1;
   
    KFREE(pBuf);

    return sts;
}

/** set backup psi 
 * 
 * Backup PSI does not share its sectors with anything else, so this 
 * function does not need to read first and write.  Just write.
 * This function expects the length of the buffer to be exactly the 
 * length of the entire PSI.
 *
 * @param string (IN) buffer that holds the data to be written.
 *
 * @return 0 if OK, -1 on failure.
 */
static int kerSysBackupPsiSet(const unsigned char *string)
{
    int sts = 0;

    if (setSharedBlks(fInfo.flash_backup_psi_start_blk, 
                      fInfo.flash_backup_psi_number_blk,
                      (unsigned char *) string) != 0)
        sts = -1;

    return sts;
}

/***********************************************************************
 * Function Name: kerSysErasePsi
 * Description  : Erase the Psi storage section of flash memory.
 * Returns      : 1 -- ok, 0 -- fail
 ***********************************************************************/
int kerSysErasePsi(void)
{
    int sts = 1;
    unsigned char *tempStorage;
    
    if (fInfo.flash_persistent_start_blk == 0) {
        sts = 0;
    }
    else {
        tempStorage = KMALLOC(fInfo.flash_persistent_length, sizeof(long));
        // just write the whole buf with '0xff' to the flash
        if (!tempStorage)
            sts = 0;
        else
        {
            memset(tempStorage, 0xff, fInfo.flash_persistent_length);
            if (kerSysPsiSet(tempStorage, fInfo.flash_persistent_length) != 0)
                sts = 0;

            // Also erase backup psi if it is there
            if (fInfo.flash_backup_psi_number_blk > 0)
            {
               if (kerSysBackupPsiSet(tempStorage) != 0)
                  sts = 0;
            }

            KFREE(tempStorage);
        }
    }
    return sts;
}
#else
int kerSysErasePsi(void)
{
    int sts = 1;
    int blk_size = flash_get_sector_size(0) / 1024;
    int blk_start = NVRAM.ulNandPartOfsKb[NP_DATA] / blk_size;
    int total_blks =
    blk_start + (NVRAM.ulNandPartSizeKb[NP_DATA]) / blk_size;
    while( blk_start < total_blks ) {
           flash_sector_erase_int(blk_start);
           blk_start++;
    }
    sts = 0;
    return(sts);
}
#endif

// flash bcm image 
// return: 
// 0 - ok
// !0 - the sector number fail to be flashed (should not be 0)
int kerSysBcmImageSet( int flash_start_addr, unsigned char *string, int size, int fWholeImage)
{
    int sts = -1;
    int sect_size;
    int blk_start;
    int savedSize = size;
    int total_blks = flash_get_numsectors();
    if(( flash_get_flash_type() == FLASH_IFC_NAND ) ||
       ( flash_get_flash_type() == FLASH_IFC_SPINAND )) {
      if( flash_start_addr == FLASH_BASE ) {
            total_blks = 1;
      } else {
            {
                int rootfs = -1;
                unsigned long start_ofs_kb = ((unsigned long)
                    flash_start_addr - FLASH_BASE) / 1024;

                if( start_ofs_kb > NVRAM.ulNandPartOfsKb[NP_ROOTFS_1] &&
                    start_ofs_kb < (NVRAM.ulNandPartOfsKb[NP_ROOTFS_1] +
                        NVRAM.ulNandPartSizeKb[NP_ROOTFS_1]) )
                {
                    rootfs = NP_ROOTFS_1;
                }
                else
                {
                    if(start_ofs_kb>NVRAM.ulNandPartOfsKb[NP_ROOTFS_2] &&
                       start_ofs_kb<(NVRAM.ulNandPartOfsKb[NP_ROOTFS_2] +
                            NVRAM.ulNandPartSizeKb[NP_ROOTFS_2]))
                    {
                        rootfs = NP_ROOTFS_2;
                    }
                }

                if( rootfs != -1 )
                {
                    total_blks = (NVRAM.ulNandPartOfsKb[rootfs] +
                        NVRAM.ulNandPartSizeKb[rootfs]) /
                        (flash_get_sector_size(0) / 1024);
                }
            }
        }
    }

#if defined(DEBUG_FLASH)
    printf("kerSysBcmImageSet: flash_start_addr=0x%x string=%p len=%d wholeImage=%d\n",
           flash_start_addr, string, size, fWholeImage);
#endif

    blk_start = flash_get_blk(flash_start_addr);
    if( blk_start < 0 ) {
        goto err_out;
        
    }

    /* write image to flash memory */
    do {
        sect_size = flash_get_sector_size(blk_start);

        flash_sector_erase_int(blk_start);     // erase blk before flash

        if (sect_size > size) {
            if (size & 1) 
                size++;
            sect_size = size;
        }
        if (flash_write_buf(blk_start, 0, string, sect_size) != sect_size) {
            if( ((flash_get_flash_type() !=  FLASH_IFC_NAND) && (flash_get_flash_type() !=  FLASH_IFC_SPINAND)) ||
                blk_start >= total_blks )
                break;
            blk_start++;
        } else { 
            printf(".");
            blk_start++;
            string += sect_size;
            size -= sect_size; 
        }
    } while (size > 0);

    if (size == 0 && fWholeImage && savedSize > 4*FLASH_LENGTH_BOOT_ROM)  
    {
        /* If flashing a whole linux image, erase to end of flash. CFE some time gets very
           big when compile with SPI PROG NAND or some diagnostic feature. Linux image
           is way big than 4*FLASH_LENGTH_BOOT_ROM */
        while( blk_start < total_blks )
        {
            flash_sector_erase_int(blk_start);
            printf(".");
            blk_start++;
        }
    }

    printf("\n\n");

    if( size == 0 ) 
    {
        kerSysSetBootImageState(BOOT_SET_NEW_IMAGE); // in case JFFS2 boot is set to something other than new image; pureUBI will have the commit flag set so will boot the latest image
        sts = 0;  // ok
    }
    else  
        sts = blk_start;    // failed to flash this sector
 err_out:
    return sts;
}

unsigned long kerSysReadFromFlash( void *toaddr, unsigned long fromaddr,
    unsigned long len )
{
    int sect = flash_get_blk((int) fromaddr);
    unsigned char *start = flash_get_memptr(sect);
    flash_read_buf( sect, (int)( fromaddr - (unsigned long) start), toaddr, len );

    return(len);
}

void get_flash_boot_cfe_version(char **version, int *size)
{
    flash_read_buf (NVRAM_SECTOR, CFE_VERSION_OFFSET,
                (unsigned char *)&flashBootCfeVersion, sizeof(flashBootCfeVersion));
    *version=flashBootCfeVersion;
    *size=sizeof(flashBootCfeVersion);
}

/*******************************************************************************
 * Image State functions
 *******************************************************************************/

#if (INC_EMMC_FLASH_DRIVER)
/***********************************************************************
 * Function Name: kerSysSetBootImageState
 * Description  : This function is disabled for eMMC and pureUBI
 ***********************************************************************/
int kerSysSetBootImageState( int state )
{
    return(BOOT_SET_NEW_IMAGE);
}

/***********************************************************************
 * Function Name: kerSysGetBootImageState
 * Description  : This function is disabled for eMMC and pureUBI
 ***********************************************************************/
int kerSysGetBootImageState( void )
{
    return(BOOT_SET_NEW_IMAGE);
}
#elif (INC_NAND_FLASH_DRIVER!=1)
/***********************************************************************
 * Function Name: kerSysSetBootImageState
 * Description  : Persistently sets the state of an image update.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int kerSysSetBootImageState( int state )
{
    int ret = -1;
    unsigned char *pShareBuf = NULL;

    switch(state)
    {
    case BOOT_SET_OLD_IMAGE:
    case BOOT_SET_NEW_IMAGE:
    case BOOT_SET_NEW_IMAGE_ONCE:
        if( (pShareBuf = getSharedBlks( fInfo.flash_scratch_pad_start_blk,
            fInfo.flash_scratch_pad_number_blk)) != NULL )
        {
            PSP_HEADER pHdr = (PSP_HEADER) pShareBuf;
            unsigned int *pBootImgState=(unsigned int *)&pHdr->NvramData2[0];

            /* The boot image state is stored as a word in flash memory where
             * the most significant three bytes are a "magic number" and the
             * least significant byte is the state constant.
             */
            if((*pBootImgState & 0xffffff00) == (BLPARMS_MAGIC & 0xffffff00) &&
               (*pBootImgState & 0x000000ff) == (state & 0x000000ff))
            {
                ret = 0;
            }
            else
            {
                *pBootImgState = (BLPARMS_MAGIC & 0xffffff00);
                *pBootImgState |= (state & 0x000000ff);

                ret = setSharedBlks(fInfo.flash_scratch_pad_start_blk,    
                    fInfo.flash_scratch_pad_number_blk,  pShareBuf);
            }

            KFREE(pShareBuf);
        }
        break;

    default:
        break;
    }

    return( ret );
}

/***********************************************************************
 * Function Name: kerSysGetBootImageState
 * Description  : Gets the state of an image update from flash.
 * Returns      : state constant or -1 for failure
 ***********************************************************************/
int kerSysGetBootImageState( void )
{
    int ret = -1;
    unsigned char *pShareBuf = NULL;

    if( (pShareBuf = getSharedBlks( fInfo.flash_scratch_pad_start_blk,
        fInfo.flash_scratch_pad_number_blk)) != NULL )
    {
        PSP_HEADER pHdr = (PSP_HEADER) pShareBuf;
        unsigned int *pBootImgState=(unsigned int *)&pHdr->NvramData2[0];

        /* The boot image state is stored as a word in flash memory where
         * the most significant three bytes are a "magic number" and the
         * least significant byte is the state constant.
         */
        if( (*pBootImgState & 0xffffff00) == (BLPARMS_MAGIC & 0xffffff00) )
        {
            switch(ret = (*pBootImgState & 0x000000ff))
            {
            case BOOT_SET_OLD_IMAGE:
            case BOOT_SET_NEW_IMAGE:
            case BOOT_SET_NEW_IMAGE_ONCE:
                break;

            default:
                ret = -1;
                break;
            }
        }

        KFREE(pShareBuf);
    }

    return( ret );
}
#else /* NAND flash */

/***********************************************************************
 * Function Name: setBootImageState (kerSysSetBootImageState)
 * Description  : Persistently sets the state of an image update.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int kerSysSetBootImageState( int state )
{
    /* Rewrites the JFFS2 directory entry with the specified boot state value, pureUBI BOOT_SET_IMAGE_*_ONCE state was already cleared in CFEROM */
    return(findBootImageDirEntry(state));
}

/***********************************************************************
 * Function Name: getBootImageState (kerSysGetBootImageState)
 * Description  : Gets the state of an image update from flash.
 * Returns      : state constant or -1 for failure
 ***********************************************************************/
int kerSysGetBootImageState(void)
{
    int seq1 = getSeqNum(1);
    int seq2 = getSeqNum(2);
    char commitflag1 = 0;
    char commitflag2 = 0;
    int ret = -1;
    int pureubi1 = commit(1, &commitflag1);
    int pureubi2 = commit(2, &commitflag2);

    if ((pureubi1 != -1) && (pureubi2 != -1))
    { // pureUBI images
        if ((seq1 == 0) && (seq2 == 999))
            seq1 = 1000;
        if ((seq2 == 0) && (seq1 == 999))
            seq2 = 1000;

        if ( ((seq1 > seq2) && (commitflag1 == '0') && (commitflag2 == '1')) ||
             ((seq2 > seq1) && (commitflag2 == '0') && (commitflag1 == '1')) )
           ret = BOOT_SET_OLD_IMAGE;
        else // default to this case even though technically not committed for either image is an undefined state
           ret = BOOT_SET_NEW_IMAGE;
    }
    else if ((seq1 != -1) && (seq2 != -1))
    { // two images and at least one JFFS2 image, boot state is in data partition
        switch(ret = (findBootImageDirEntry(-1)))
        {
            case BOOT_SET_OLD_IMAGE:
            case BOOT_SET_NEW_IMAGE:
            case BOOT_SET_NEW_IMAGE_ONCE:
            case BOOT_SET_OLD_IMAGE_ONCE:
                break;

            default: // no boot_state_x file found, will boot latest image
                ret = BOOT_SET_NEW_IMAGE;
                break;
        }
    }
    else // a single image is in the flash, boot that
        ret = BOOT_SET_NEW_IMAGE;

    return( ret );
}

#endif /* EMMC_FLASH_DRIVER */


