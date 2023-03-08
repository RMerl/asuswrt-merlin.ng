/***********************************************************************
 *
 *  Copyright (c) 2011  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 ************************************************************************/


#ifndef _BCM_FLASHUTIL_NAND_H__
#define _BCM_FLASHUTIL_NAND_H_

#include <stdint.h>
#include <mtd/mtd-user.h>

#include "bcm_hwdefs.h"


/* Used by bcm_imgif, bcm_imgutil, image.c*/
unsigned char *nandUpdateSeqNum(unsigned char *imagePtr, int imageSize, int blkLen, int seq, int *found);
int nand_image_type(unsigned char * buf);
mtd_info_t *get_mtd_device_handle(const char *check, int *mtd_fd, int *mtdblock_fd);
void put_mtd_device(mtd_info_t *mtd, int mtd_fd, int mtdblock_fd);
int get_mtd_device_name(const char * check __attribute__((unused)), char * device __attribute__((unused)));
int get_mtd_master_size(unsigned int *psize);
int nandEraseBlk(mtd_info_t *mtd, int blk_addr, int mtd_fd);
int nandWriteBlk(mtd_info_t *mtd, int blk_addr, int data_len, unsigned char *data_ptr,
  int mtd_fd, int write_JFFS2_clean_marker);
int flashCferam(mtd_info_t *mtd, int mtd_fd, int rsrvd_for_cferam,
  unsigned char *cferam_ptr, unsigned char *cferam_ptr2);
int handleCferom(mtd_info_t *mtd0, char *image_ptr, unsigned int image_size, unsigned int wfiFlags,
  void *inMemNvramData_buf);
int flashCferom(unsigned char *image_ptr, unsigned int size);
int validateWfiTag(void *wt, int blksize, uint32_t btrmEnabled);

unsigned int nvramDataOffset(const mtd_info_t * mtd __attribute__((unused)));

/* Used by bcm_flashutil.c */
int nandReadNvramData(void *nvramData);
int nandGetSequenceNumber(int imageNumber);
int nandGetImageVersion(uint8_t *imagePtr, int imageSize, char *image_name, int image_name_len);
int nandWriteBootImageState( int newState );
int nandReadBootImageState( void );
int nandUpdateSequenceNumber(int incSeqNumPart, int seqPart1, int seqPart2);
int nandIsBootDevice(void);
int nandGetBootedValue(void);
int nandGetBootPartition(void);
int nandCommit( int partition, char *commit_flag );
int nandSetImgValidStatus( int img_idx, int valid);
int nandIsLegacyFlashLayout(void);
int nandUbiVolDevNodeExists( char * dev_path );
uint64_t nandGetAvailImgSpace( int update_img_idx );
uint64_t nandGetAvailLoaderSpace(void);
int getNandMetadata( char * data, int size , int mdata_idx);
int setNandMetadata( char * data, int size, int mdata_idx );
int nandFlashLoader(unsigned char *file_name, uint64_t file_size);

/* used by housekeeping task that keeps nvram mirror and cferom integrity */
int is_cferom_offset(unsigned char *buffer, int offset, int *img_size, int *cferom_crc, unsigned char **);
int is_nvram_offset(unsigned char *buffer, int offset, int nvram_sign_check, int *img_size, PNVRAM_DATA);
void create_boot_block_info_file(char *hunt_ptr, int size_to_search, PNVRAM_DATA pnd, mtd_info_t *mtd);
#define RSA_S_MODULUS       2048                 /* Bits in a key modulus     */
#define RSA_S_MODULUS8      (RSA_S_MODULUS/8)    /* Bytes in a key modulus    */
#define BTRM_SBI_UNAUTH_MGC_NUM_1            183954
#define BTRM_SBI_UNAUTH_MGC_NUM_2            145257


#endif /* _BCM_FLASHUTIL_NAND_H_ */
