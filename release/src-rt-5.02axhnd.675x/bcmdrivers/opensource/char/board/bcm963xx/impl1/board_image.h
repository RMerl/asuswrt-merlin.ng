/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
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
*/

#ifndef _BOARD_IMAGE_H_
#define _BOARD_IMAGE_H_

uint32_t genCrc32(byte *pdata, uint32_t size, uint32_t crc);

void *retriedKmalloc(size_t size);
void retriedKfree(void *pBuf);

PNVRAM_DATA readNvramData(void);
int writeNvramData(PNVRAM_DATA pNvramData);
void updateInMemNvramData(const unsigned char *data, int len, int offset);

char *getSharedBlks(int start_blk, int num_blks);
int setSharedBlks(int start_blk, int num_blks, char *pTempBuf);

int commonImageWrite(int flash_start_addr, char *string, int size, int *pnoReboot, int partition);
int flashFsKernelImage( unsigned char *imagePtr, int imageLen, int flashPartition, int *numPartitions );

int getBootedValue(int getBootedPartition);
int getImageVersion( int imageNumber, char *verStr, int verStrSize);
void clearImageVersion(void);

int setUserNvRam(char *string, int strLen, int offset);

#endif
