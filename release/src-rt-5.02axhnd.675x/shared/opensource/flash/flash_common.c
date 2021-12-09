/*
    Copyright 2000-2010 Broadcom Corporation

   <:label-BRCM:2012:DUAL/GPL:standard
   
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

/*!\file flash_common.c
 * \brief This file contains NOR flash related functions used by both
 *        CFE and kernel.
 *
 */

/** Includes. */
#ifdef _CFE_                                                
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bcm_map_part.h"  
#define printk  printf
#else // Linux
#include <linux/kernel.h>
#include "bcm_map_part.h"
#include <linux/string.h>
#endif

#include "bcmtypes.h"
#include "bcm_hwdefs.h"
#include "flash_api.h"
#include "flash_common.h"

// #define DEBUG_FLASH

#if defined(CONFIG_BCM_DISABLE_NOR_RAW_PARTITION) || defined(DISABLE_NOR_RAW_PARTITION)
static int disable_nor_raw_partition=1;
#else
static int disable_nor_raw_partition=0;
#endif

FLASH_PARTITION_INFO fAuxFsInfo;

void flash_init_info(const NVRAM_DATA *nvRam, FLASH_ADDR_INFO *fInfo)
{
    int i = 0;
    int totalBlks = 0;
    int totalSize = 0;
    int auxFsSize = 0;
    int psiStartAddr = 0;
    int spStartAddr = 0;
    int usedBlkSize = 0;
    int needBytes = 0;
    int sBlk, eBlk;
    int blkSize, sectSize;      /* Size of all blocks in a partition or 0 */
    int totalPartSize;          /* Size in bytes for a partition */
    unsigned long  offset;      /* Offset calulations */
    int flash_type = flash_get_flash_type();

    if ((flash_type == FLASH_IFC_NAND) || (flash_type == FLASH_IFC_SPINAND) || (flash_type == FLASH_IFC_UNSUP_EMMC))
    {
        /* When using NAND flash disable Bcm_flash */
        totalSize = 0;
    }
    else {
        totalBlks = flash_get_numsectors();
        totalSize = flash_get_total_size();
    }

    if (totalSize <= FLASH_LENGTH_BOOT_ROM) {
        /* NAND flash settings. NAND flash does not use raw flash partitioins
         * to store psi, backup psi, scratch pad and syslog.  These data items
         * are stored as files on a JFFS2 file system.
         */
        if ((nvRam->ulPsiSize != -1) && (nvRam->ulPsiSize != 0))
            fInfo->flash_persistent_length = nvRam->ulPsiSize * ONEK;
        else
            fInfo->flash_persistent_length = DEFAULT_PSI_SIZE * ONEK;

        fInfo->flash_persistent_start_blk = 0;
        fInfo->flash_rootfs_start_offset = 0;
        fInfo->flash_scratch_pad_length = SP_MAX_LEN;
        fInfo->flash_syslog_length = nvRam->ulSyslogSize * 1024;

        /* This is a boolean field for NAND flash. */
        fInfo->flash_backup_psi_number_blk = nvRam->backupPsi;
        return;
    }

    /*
    * calculate mandatory primary PSI size and set its fInfo parameters.
    */
    if ((nvRam->ulPsiSize != -1) && (nvRam->ulPsiSize != 0))
        fInfo->flash_persistent_length = nvRam->ulPsiSize * ONEK;
    else
        fInfo->flash_persistent_length = DEFAULT_PSI_SIZE * ONEK;

    if (disable_nor_raw_partition)
    {
        fInfo->flash_persistent_number_blk = 0;
        fInfo->flash_scratch_pad_start_blk = 0;
        fInfo->flash_scratch_pad_length = SP_MAX_LEN;
        fInfo->flash_backup_psi_number_blk = 0;
        fInfo->flash_syslog_length = nvRam->ulSyslogSize * 1024;
        fInfo->flash_syslog_number_blk = 0;
        /*should set flash_backup_psi_number_blk for spi nor flash?*/
        fInfo->flash_backup_psi_number_blk = nvRam->backupPsi;
        fInfo->flash_meta_start_blk = totalBlks;

        if (nvRam->ucAuxFSPercent != 0)
        {
            /* Estimate the Auxillary File System size */
            auxFsSize = (int)nvRam->ucAuxFSPercent*NOR_AUFS_SIZE_UNIT;
            /* JFFS_AUXFS offset */
            offset = totalSize - auxFsSize;

            sBlk = flash_get_blk(offset+FLASH_BASE);
            eBlk = totalBlks;
            goto adjust;
        }
    }

    psiStartAddr = totalSize - fInfo->flash_persistent_length;
    fInfo->flash_persistent_start_blk = flash_get_blk(FLASH_BASE+psiStartAddr);
    fInfo->flash_persistent_number_blk = totalBlks - fInfo->flash_persistent_start_blk;

    usedBlkSize = 0;
    for (i = fInfo->flash_persistent_start_blk; 
        i < (fInfo->flash_persistent_start_blk + fInfo->flash_persistent_number_blk); i++)
    {
        usedBlkSize += flash_get_sector_size((unsigned short) i);
    }
    fInfo->flash_persistent_blk_offset =  usedBlkSize - fInfo->flash_persistent_length;
    fInfo->flash_meta_start_blk = fInfo->flash_persistent_start_blk;

    /*
    * Next is the optional scratch pad, which is on top of the primary PSI.
    * Old code allowed scratch pad to share a sector with primary PSI.
    * That is retained for backward compatibility.  (Although depending on your
    * NOR flash sector sizes, they may still be in different sectors.)
    * If you have a new deployment, consider forcing separate sectors.
    */
    if ((fInfo->flash_persistent_blk_offset > 0) &&
        (fInfo->flash_persistent_blk_offset < SP_MAX_LEN))
    {
        /*
        * there is some room left in the first persistent sector, but it is
        * not big enough for the scratch pad. (Use this line unconditionally
        * if you want to guarentee scratch pad and primary PSI are on different
        * sectors.)
        */
        spStartAddr = psiStartAddr - fInfo->flash_persistent_blk_offset - SP_MAX_LEN;
    }
    else
    {
        /* either the primary PSI starts on a sector boundary, or there is
        * enough room at the top of the first sector for the scratch pad. */
        spStartAddr = psiStartAddr - SP_MAX_LEN ;
    }

    fInfo->flash_scratch_pad_start_blk = flash_get_blk(FLASH_BASE+spStartAddr);
    fInfo->flash_scratch_pad_length = SP_MAX_LEN;

    if (fInfo->flash_persistent_start_blk == fInfo->flash_scratch_pad_start_blk)  // share blk
    {
#if 0 /* do not used scratch pad unless it's in its own sector */
        printk("Scratch pad is not used for this flash part.\n");  
        fInfo->flash_scratch_pad_length = 0;     // no sp
#else /* allow scratch pad to share a sector with another section such as PSI */
        fInfo->flash_scratch_pad_number_blk = 1;
        fInfo->flash_scratch_pad_blk_offset = fInfo->flash_persistent_blk_offset - fInfo->flash_scratch_pad_length;
#endif
    }
    else // on different blk
    {
        fInfo->flash_scratch_pad_number_blk = fInfo->flash_persistent_start_blk - fInfo->flash_scratch_pad_start_blk;
        // find out the offset in the start_blk
        usedBlkSize = 0;
        for (i = fInfo->flash_scratch_pad_start_blk; 
            i < (fInfo->flash_scratch_pad_start_blk + fInfo->flash_scratch_pad_number_blk); i++)
            usedBlkSize += flash_get_sector_size((unsigned short) i);
        fInfo->flash_scratch_pad_blk_offset =  usedBlkSize - fInfo->flash_scratch_pad_length;
    }

    if (fInfo->flash_scratch_pad_length > 0) {

        fInfo->flash_meta_start_blk = fInfo->flash_scratch_pad_start_blk;
    }

    /*
    * Next is the optional backup PSI.
    */
    if (nvRam->backupPsi == 0x01)
    {
        needBytes = fInfo->flash_persistent_length;
        i = fInfo->flash_meta_start_blk;
        while (needBytes > 0)
        {
            i--;
            needBytes -= flash_get_sector_size((unsigned short) i);
        }
        fInfo->flash_backup_psi_start_blk = i;
        /* calclate how many blocks we actually consumed */
        needBytes = fInfo->flash_persistent_length;
        fInfo->flash_backup_psi_number_blk = 0;
        while (needBytes > 0)
        {
            needBytes -= flash_get_sector_size((unsigned short) i);
            i++;
            fInfo->flash_backup_psi_number_blk++;
        }

        fInfo->flash_meta_start_blk = fInfo->flash_backup_psi_start_blk;
    }
    else
    {
        fInfo->flash_backup_psi_number_blk = 0;
    }

    /*
    * Next is the optional persistent syslog.
    */
    if (nvRam->ulSyslogSize != 0 && nvRam->ulSyslogSize != -1)
    {
        fInfo->flash_syslog_length = nvRam->ulSyslogSize * 1024;
        needBytes = fInfo->flash_syslog_length;
        i = fInfo->flash_meta_start_blk;
        while (needBytes > 0)
        {
            i--;
            needBytes -= flash_get_sector_size((unsigned short) i);
        }
        fInfo->flash_syslog_start_blk = i;
        /* calclate how many blocks we actually consumed */
        needBytes = fInfo->flash_syslog_length;
        fInfo->flash_syslog_number_blk = 0;
        while (needBytes > 0)
        {
            needBytes -= flash_get_sector_size((unsigned short) i);
            i++;
            fInfo->flash_syslog_number_blk++;
        }

        fInfo->flash_meta_start_blk = fInfo->flash_syslog_start_blk;
    }
    else
    {
        fInfo->flash_syslog_length = 0;
        fInfo->flash_syslog_number_blk = 0;
    }

#if 1   //for AUXFS 

   if ( (nvRam->ucAuxFSPercent != 0)
    && (nvRam->ucAuxFSPercent <= MAX_AUXFS_PERCENT))
    {
        /* Estimate the Auxillary File System size */
        auxFsSize = (totalSize * (int)nvRam->ucAuxFSPercent)/100;
        
        /* JFFS_AUXFS offset */
        offset = totalSize - auxFsSize - flash_get_reserved_bytes_at_end(fInfo);

        sBlk = flash_get_blk(offset+FLASH_BASE);
        eBlk = fInfo->flash_meta_start_blk;

        /*
         * Implementation Note:
         * Ensure that we have even number of blocks for
         * ROOTFS+KERNEL to support dual image booting
        */
adjust:
        if ( ( (sBlk+1) < eBlk)
          && ((((sBlk+1) - flash_get_blk(fInfo->flash_rootfs_start_offset + FLASH_BASE)) % 2) == 0))
        {
            sBlk += 1;  /* Round up */
        }

        blkSize = flash_get_sector_size(sBlk);
        for ( i=sBlk+1, totalPartSize = blkSize; i<eBlk; i++)
        {
            sectSize = flash_get_sector_size(i);
            //if ( blkSize != sectSize ) blkSize = 0;
            if ( blkSize != sectSize ) break;
            totalPartSize += sectSize;
        }
        fAuxFsInfo.sect_size = blkSize;
        auxFsSize = totalPartSize;


        printk("Flash split %d : AuxFS[%d]\n",
               (int)nvRam->ucAuxFSPercent,auxFsSize );
    }
    else
    {
    /*
     * Implementation Note: When there is no AuxFS Partition.
     * Total number of rootfs/kernel blocks will always be ODD.
     * Option: Increase RESERVED section ??? but this would
     * decrease the space available for a single kernel image
     */
        sBlk = eBlk = 0;
        fAuxFsInfo.sect_size = 0;
        auxFsSize = 0;
        printk("Flash not used for Auxillary File System\n");
     }


    /*------------*/
    /* JFFS_AUXFS */
    /*------------*/

    sprintf(fAuxFsInfo.name, "JFFS_AUXFS");
    fAuxFsInfo.start_blk = sBlk;
    fAuxFsInfo.number_blk = eBlk - sBlk;
    fAuxFsInfo.blk_offset = 0;
    fAuxFsInfo.total_len = auxFsSize;
    fAuxFsInfo.mem_base = (uintptr_t) flash_get_memptr( sBlk )
                                   +  fAuxFsInfo.blk_offset;
    fAuxFsInfo.mem_length = auxFsSize;
    
#endif

#ifdef DEBUG_FLASH_TOO_MUCH
    /* dump sizes of all sectors in flash */
    for (i=0; i<totalBlks; i++)
        printk("blk %03d: %d\n", i, flash_get_sector_size((unsigned short) i));
#endif

#if defined(DEBUG_FLASH)
    printk("FLASH_BASE                    =0x%08x\n\n", (unsigned int)FLASH_BASE);

    printk("fInfo->flash_rootfs_start_offset =0x%08x\n\n", (unsigned int)fInfo->flash_rootfs_start_offset);

    printk("fInfo->flash_meta_start_blk = %d\n\n", fInfo->flash_meta_start_blk);

    printk("fInfo->flash_syslog_start_blk  = %d\n", fInfo->flash_syslog_start_blk);
    printk("fInfo->flash_syslog_number_blk = %d\n", fInfo->flash_syslog_number_blk);
    printk("fInfo->flash_syslog_length=0x%x\n\n", (unsigned int)fInfo->flash_syslog_length);

    printk("fInfo->flash_backup_psi_start_blk = %d\n", fInfo->flash_backup_psi_start_blk);
    printk("fInfo->flash_backup_psi_number_blk = %d\n\n", fInfo->flash_backup_psi_number_blk);

    printk("sp startAddr = %x\n", (unsigned int) (FLASH_BASE+spStartAddr));
    printk("fInfo->flash_scratch_pad_start_blk = %d\n", fInfo->flash_scratch_pad_start_blk);
    printk("fInfo->flash_scratch_pad_number_blk = %d\n", fInfo->flash_scratch_pad_number_blk);
    printk("fInfo->flash_scratch_pad_length = 0x%x\n", fInfo->flash_scratch_pad_length);
    printk("fInfo->flash_scratch_pad_blk_offset = 0x%x\n\n", (unsigned int)fInfo->flash_scratch_pad_blk_offset);

    printk("psi startAddr = %x\n", (unsigned int) (FLASH_BASE+psiStartAddr));
    printk("fInfo->flash_persistent_start_blk = %d\n", fInfo->flash_persistent_start_blk);
    printk("fInfo->flash_persistent_number_blk = %d\n", fInfo->flash_persistent_number_blk);
    printk("fInfo->flash_persistent_length=0x%x\n", (unsigned int)fInfo->flash_persistent_length);
    printk("fInfo->flash_persistent_blk_offset = 0x%x\n\n", (unsigned int)fInfo->flash_persistent_blk_offset);
    printk("AuxFs.start_blk = %d\n",fAuxFsInfo.start_blk );
    printk("AuxFs,number_blk = %d\n", fAuxFsInfo.number_blk);
    printk("AuxFs.total_len = 0x%x\n",fAuxFsInfo.total_len);
    printk("AuxFs.sect_size = 0x%x\n",fAuxFsInfo.sect_size);
#endif
}


void kerSysFlashPartInfoGet(PFLASH_PARTITION_INFO pflash_partition_info)
{
    memcpy((void*)pflash_partition_info,(const void*)(&fAuxFsInfo), sizeof(FLASH_PARTITION_INFO));
}


unsigned int flash_get_reserved_bytes_auxfs()
{
    int flash_type = flash_get_flash_type();

    if ((flash_type == FLASH_IFC_NAND) || (flash_type == FLASH_IFC_SPINAND))
    {
        return 0;
    }
    else
    {
        return fAuxFsInfo.total_len;
    }
}

unsigned int flash_get_reserved_bytes_at_end(const FLASH_ADDR_INFO *fInfo)
{
    unsigned int reserved=0;
    int i = fInfo->flash_meta_start_blk;
    int totalBlks = flash_get_numsectors();

    while (i < totalBlks)
    {
        reserved += flash_get_sector_size((unsigned short) i);
        i++;
    }

#if defined(DEBUG_FLASH)
    printk("reserved at bottom=%dKB\n", reserved/1024);
#endif

    return reserved;
}

