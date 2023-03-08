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


#ifndef _BCM_FLASHUTIL_EMMC_H__
#define _BCM_FLASHUTIL_EMMC_H_
#include "emmc_linux_defs.h"

int emmcReadNvramData(void *nvramData);
int emmcGetSequenceNumber(int imageNumber);
int emmcGetImageVersion(uint8_t *imagePtr, int imageSize, char *image_name, int image_name_len);
int emmcWriteBootImageState( int newState );
int emmcReadBootImageState( void );
int emmcUpdateSequenceNumber(int incSeqNumPart, int seqPart1, int seqPart2);
int emmcIsBootDevice(void);
int emmcGetBootedValue(void);
int emmcGetNvramOffset( char * cfe_start_addr );
int emmcUpdateMdataSeqnum( char * mdata_ptr, int image_num, int seq_num );
int emmcUpdateMdataCommitFlag(char * mdata_ptr, int image_num, int flag );
int emmcGetBootPartition(void);
int emmcCommit( int partition, char *string );
int emmcVerifyImageDDRType( uint32_t imageFlags );
int emmcIsLegacyFlashLayout(void);
uint64_t emmcGetAvailImgSpace( int update_img_idx );
uint64_t emmcGetAvailLoaderSpace( void );
int setEmmcMetadata( char * data, int size, int mdata_idx );
int getEmmcMetadata( char * data, int size , int mdata_idx);
int emmcGetFlashSize(unsigned int *psize);
int emmcGetBootPartIndex( void );
#endif /* _BCM_FLASHUTIL_EMMC_H_ */
