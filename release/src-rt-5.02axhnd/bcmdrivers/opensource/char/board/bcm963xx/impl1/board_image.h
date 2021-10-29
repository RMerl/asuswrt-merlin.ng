/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
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
*/

#ifndef _BOARD_IMAGE_H_
#define _BOARD_IMAGE_H_

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
