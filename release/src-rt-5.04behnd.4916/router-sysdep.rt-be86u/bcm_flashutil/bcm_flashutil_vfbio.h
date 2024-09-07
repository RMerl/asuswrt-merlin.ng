/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
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


#ifndef _BCM_FLASHUTIL_VFBIO_H_
#define _BCM_FLASHUTIL_VFBIO_H_

/* Names of VFBIO logical partitions */
#define VFBIO_PNAME_ARMBL    "armbl"
#define VFBIO_PNAME_NVRAM    "nvram"
#define VFBIO_PNAME_BOOTFS   "bootfs"
#define VFBIO_PNAME_ROOTFS   "rootfs"
#define VFBIO_PNAME_SMCBL    "smcbl"
#define VFBIO_PNAME_SMCOS    "smcos"
#define VFBIO_PNAME_MEMINIT  "meminit"

/* Strings of VFBIO CFE logical part names e.g. "bootfs1", "nvram", "rootfs1" */
#define VFBIO_PNAME_FMT_ARMBL       VFBIO_PNAME_ARMBL "%d"
#define VFBIO_PNAME_FMT_BOOTFS      VFBIO_PNAME_BOOTFS "%d"
#define VFBIO_PNAME_FMT_ROOTFS      VFBIO_PNAME_ROOTFS "%d"
#define VFBIO_PNAME_FMT_SMCBL       VFBIO_PNAME_SMCBL "%d"
#define VFBIO_PNAME_FMT_SMCOS       VFBIO_PNAME_SMCOS "%d"
#define VFBIO_PNAME_FMT_MEMINIT     VFBIO_PNAME_MEMINIT "%d"

#define VFBIO_DEV_LINUX_PREFIX      "/dev/flash-"

/* Full pathnames of VFBIO linux logical partitions e.g "/dev/bootfsx" */
#define VFBIO_DEV_PNAME_NVRAM       VFBIO_DEV_LINUX_PREFIX VFBIO_PNAME_NVRAM
#define VFBIO_DEV_PNAME_FMT_ARMBL   VFBIO_DEV_LINUX_PREFIX VFBIO_PNAME_FMT_ARMBL
#define VFBIO_DEV_PNAME_FMT_BOOTFS  VFBIO_DEV_LINUX_PREFIX VFBIO_PNAME_FMT_BOOTFS
#define VFBIO_DEV_PNAME_FMT_ROOTFS  VFBIO_DEV_LINUX_PREFIX VFBIO_PNAME_FMT_ROOTFS
#define VFBIO_DEV_PNAME_FMT_SMCBL   VFBIO_DEV_LINUX_PREFIX VFBIO_PNAME_FMT_SMCBL
#define VFBIO_DEV_PNAME_FMT_SMCOS   VFBIO_DEV_LINUX_PREFIX VFBIO_PNAME_FMT_SMCOS
#define VFBIO_DEV_PNAME_FMT_MEMINIT VFBIO_DEV_LINUX_PREFIX VFBIO_PNAME_FMT_MEMINIT

int vfbioReadNvramData(void *nvramData);
int vfbioGetSequenceNumber(int imageNumber);
int vfbioGetImageVersion(uint8_t *imagePtr, int imageSize, char *image_name, int image_name_len);
int vfbioWriteBootImageState( int newState );
int vfbioReadBootImageState( void );
int vfbioIsBootDevice(void);
int vfbioGetBootedValue(void);
int vfbioGetNvramOffset( char * cfe_start_addr );
int vfbioGetBootPartition(void);
int vfbioCommit( int partition, char *string );
int vfbioVerifyImageDDRType( uint32_t imageFlags );
int vfbioIsLegacyFlashLayout(void);
uint64_t vfbioGetAvailImgSpace( int update_img_idx );
uint64_t vfbioGetAvailLoaderSpace( void );
uint64_t vfbioGetAvailSmcblSpace( void );
uint64_t vfbioGetAvailSmcosSpace( void );
uint64_t vfbioGetAvailMeminitSpace( void );
int vfbioGetFlashSize(unsigned int *psize);
int vfbioGetBootPartIndex( void );
int vfbioWriteImage(int lun_id, uint8_t *imagePtr, int imageSize);
#endif /* _BCM_FLASHUTIL_VFBIO_H_ */
