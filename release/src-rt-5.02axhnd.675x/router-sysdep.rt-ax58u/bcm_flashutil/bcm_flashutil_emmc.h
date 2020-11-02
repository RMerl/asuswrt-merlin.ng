/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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

#endif /* _BCM_FLASHUTIL_EMMC_H_ */
