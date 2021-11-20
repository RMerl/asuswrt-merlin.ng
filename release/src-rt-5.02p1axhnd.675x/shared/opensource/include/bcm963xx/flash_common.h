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
/*!\file flash_common.h
 * \brief This file contains definitions and prototypes used by both
 *        CFE and kernel.
 *
 */

#if !defined(_FLASH_COMMON_H)
#define _FLASH_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif


/** default scratch pad length */
#define SP_MAX_LEN          (8 * 1024)


/** Flash storage address information that is determined by the flash driver.
 *
 *  This structure is used by CFE and kernel.
 */
typedef struct flashaddrinfo
{
    int flash_persistent_start_blk;   /**< primary psi, for config file */
    int flash_persistent_number_blk;
    int flash_persistent_length;      /**< in bytes */
    unsigned long flash_persistent_blk_offset;
    int flash_scratch_pad_start_blk;  /**< start of scratch pad */
    int flash_scratch_pad_number_blk;
    int flash_scratch_pad_length;     /**< in bytes */
    unsigned long flash_scratch_pad_blk_offset;
    unsigned long flash_rootfs_start_offset; /**< offset from start of flash to fs+kernel image */
    int flash_backup_psi_start_blk;  /**< starting block of backup psi. Length is
                                          the same as primary psi.  
                                          Start at begining of sector, so offset is always 0.
                                          No sharing sectors with anybody else. */
    int flash_backup_psi_number_blk;  /**< The number of sectors for primary and backup
                                       *   psi may be different due to the irregular
                                       *   sector sizes at the end of the flash. */
    int flash_syslog_start_blk;  /**< starting block of persistent syslog. */
    int flash_syslog_number_blk; /**< number of blocks */
    int flash_syslog_length;     /**< in bytes, set from CFE, note busybox syslogd uses 16KB buffer.
                                      Like backup_psi, always start at beginning of sector,
                                      so offset is 0, and no sharing of sectors. */
    int flash_meta_start_blk; /**< The first block which is used for meta info such
                                   as the psi, scratch pad, syslog, backup psi.
                                   The kernel can use everything above this sector. */
} FLASH_ADDR_INFO, *PFLASH_ADDR_INFO;



/*
 * A partition may be filled from the tail end of the partition. If the total
 * length of all blocks in the partition is not equal to the total length 
 * needed, an offset into the first block may be used. In this case, the
 * memptr will be at a blk_offset from the memptr associated with start_blk.
 */
typedef struct flash_partition_info {

    char name[32];
		/* Partition coordinates */
    int start_blk;		/* start block id */
    int number_blk;		/* total number of blocks in partition */
    int total_len;		/* total length of all blocks in partition */
    int sect_size;		/* size of each block in the sector */
				/* =0 if all sectors are not of the same size*/
    unsigned long blk_offset;	/* offset of start memory in start block */

		/* Memory usage coordinates in partition */
    unsigned long mem_base;	/* start memory pointer in partition */
    int mem_length;		/* length of memory used in partition */
} FLASH_PARTITION_INFO, *PFLASH_PARTITION_INFO;



extern void kerSysFlashPartInfoGet(PFLASH_PARTITION_INFO pflash_partition_info);

/** Fill in the fInfo structure with primary PSI, scratch pad, syslog, secondary PSI info.
 *
 * @param nvRam (IN) nvram info.
 * @param fInfo (OUT) flash addr info that will be filled out by this function.
 */
void flash_init_info(const NVRAM_DATA *nvRam, FLASH_ADDR_INFO *fInfo);


/** Get the total number of bytes at the bottom of the flash used for PSI, scratch pad, etc.
 *
 * Even though this function is returning the number of bytes, it it guaranteed to
 * return the number of bytes of whole sectors at the end which are in use.
 * If customer enables backup PSI and persistent syslog, the number of bytes
 * may go above 64KB. This function replaces the old FLASH_RESERVED_AT_END define.
 *
 * @param fInfo (IN) Pointer to flash_addr_info struct.
 *
 * @return number of bytes reserved at the end.
 */
unsigned int flash_get_reserved_bytes_at_end(const FLASH_ADDR_INFO *fInfo);
unsigned int flash_get_reserved_bytes_auxfs(void);

#if !defined(_CFE_)
NVRAM_DATA* get_inMemNvramData(void);
int eNvramGet(char *param, char *value, int len);
int eNvramSet(char *param, char *value);
int is_cfe_boot(void);
void sync_nvram_with_flash(void);
void lock_inMemNvramData_mutex(void);
void unlock_inMemNvramData_mutex(void);
static inline int envram_get_locked(char *param, char *value, int len)
{
   int ret=0;
   lock_inMemNvramData_mutex();
   ret=eNvramGet(param, value, len);
   unlock_inMemNvramData_mutex();
   return ret;
}

static inline int envram_set_locked(char *param, char *value)
{
   int ret=0;
   lock_inMemNvramData_mutex();
   ret=eNvramSet(param, value);
   unlock_inMemNvramData_mutex();
   return ret;
}

static inline int envram_sync_set_locked(char *param, char *value)
{
   int ret=0;
   lock_inMemNvramData_mutex();
   ret=eNvramSet(param, value);
   unlock_inMemNvramData_mutex();
   sync_nvram_with_flash();
   return ret;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _FLASH_COMMON_H */

