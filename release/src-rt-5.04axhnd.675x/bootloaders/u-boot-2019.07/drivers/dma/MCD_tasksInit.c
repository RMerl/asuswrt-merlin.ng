// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 */

#include <common.h>

/* Functions for initializing variable tables of different types of tasks. */

/*
 * Do not edit!
 */

#include <MCD_dma.h>

extern dmaRegs *MCD_dmaBar;

/* Task 0 */

void MCD_startDmaChainNoEu(int *currBD, short srcIncr, short destIncr,
			   int xferSize, short xferSizeIncr, int *cSave,
			   volatile TaskTableEntry * taskTable, int channel)
{
	volatile TaskTableEntry *taskChan = taskTable + channel;

	MCD_SET_VAR(taskChan, 2, (u32) currBD);	/* var[2] */
	MCD_SET_VAR(taskChan, 25, (u32) (0xe000 << 16) | (0xffff & srcIncr));	/* inc[1] */
	MCD_SET_VAR(taskChan, 24, (u32) (0xe000 << 16) | (0xffff & destIncr));	/* inc[0] */
	MCD_SET_VAR(taskChan, 11, (u32) xferSize);	/* var[11] */
	MCD_SET_VAR(taskChan, 26, (u32) (0x2000 << 16) | (0xffff & xferSizeIncr));	/* inc[2] */
	MCD_SET_VAR(taskChan, 0, (u32) cSave);	/* var[0] */
	MCD_SET_VAR(taskChan, 1, (u32) 0x00000000);	/* var[1] */
	MCD_SET_VAR(taskChan, 3, (u32) 0x00000000);	/* var[3] */
	MCD_SET_VAR(taskChan, 4, (u32) 0x00000000);	/* var[4] */
	MCD_SET_VAR(taskChan, 5, (u32) 0x00000000);	/* var[5] */
	MCD_SET_VAR(taskChan, 6, (u32) 0x00000000);	/* var[6] */
	MCD_SET_VAR(taskChan, 7, (u32) 0x00000000);	/* var[7] */
	MCD_SET_VAR(taskChan, 8, (u32) 0x00000000);	/* var[8] */
	MCD_SET_VAR(taskChan, 9, (u32) 0x00000000);	/* var[9] */
	MCD_SET_VAR(taskChan, 10, (u32) 0x00000000);	/* var[10] */
	MCD_SET_VAR(taskChan, 12, (u32) 0x00000000);	/* var[12] */
	MCD_SET_VAR(taskChan, 13, (u32) 0x80000000);	/* var[13] */
	MCD_SET_VAR(taskChan, 14, (u32) 0x00000010);	/* var[14] */
	MCD_SET_VAR(taskChan, 15, (u32) 0x00000004);	/* var[15] */
	MCD_SET_VAR(taskChan, 16, (u32) 0x08000000);	/* var[16] */
	MCD_SET_VAR(taskChan, 27, (u32) 0x00000000);	/* inc[3] */
	MCD_SET_VAR(taskChan, 28, (u32) 0x80000000);	/* inc[4] */
	MCD_SET_VAR(taskChan, 29, (u32) 0x80000001);	/* inc[5] */
	MCD_SET_VAR(taskChan, 30, (u32) 0x40000000);	/* inc[6] */

	/* Set the task's Enable bit in its Task Control Register */
	MCD_dmaBar->taskControl[channel] |= (u16) 0x8000;
}

/* Task 1 */

void MCD_startDmaSingleNoEu(char *srcAddr, short srcIncr, char *destAddr,
			    short destIncr, int dmaSize, short xferSizeIncr,
			    int flags, int *currBD, int *cSave,
			    volatile TaskTableEntry * taskTable, int channel)
{
	volatile TaskTableEntry *taskChan = taskTable + channel;

	MCD_SET_VAR(taskChan, 7, (u32) srcAddr);	/* var[7] */
	MCD_SET_VAR(taskChan, 25, (u32) (0xe000 << 16) | (0xffff & srcIncr));	/* inc[1] */
	MCD_SET_VAR(taskChan, 2, (u32) destAddr);	/* var[2] */
	MCD_SET_VAR(taskChan, 24, (u32) (0xe000 << 16) | (0xffff & destIncr));	/* inc[0] */
	MCD_SET_VAR(taskChan, 3, (u32) dmaSize);	/* var[3] */
	MCD_SET_VAR(taskChan, 26, (u32) (0x2000 << 16) | (0xffff & xferSizeIncr));	/* inc[2] */
	MCD_SET_VAR(taskChan, 5, (u32) flags);	/* var[5] */
	MCD_SET_VAR(taskChan, 1, (u32) currBD);	/* var[1] */
	MCD_SET_VAR(taskChan, 0, (u32) cSave);	/* var[0] */
	MCD_SET_VAR(taskChan, 4, (u32) 0x00000000);	/* var[4] */
	MCD_SET_VAR(taskChan, 6, (u32) 0x00000000);	/* var[6] */
	MCD_SET_VAR(taskChan, 8, (u32) 0x00000000);	/* var[8] */
	MCD_SET_VAR(taskChan, 9, (u32) 0x00000004);	/* var[9] */
	MCD_SET_VAR(taskChan, 10, (u32) 0x08000000);	/* var[10] */
	MCD_SET_VAR(taskChan, 27, (u32) 0x00000000);	/* inc[3] */
	MCD_SET_VAR(taskChan, 28, (u32) 0x80000001);	/* inc[4] */
	MCD_SET_VAR(taskChan, 29, (u32) 0x40000000);	/* inc[5] */

	/* Set the task's Enable bit in its Task Control Register */
	MCD_dmaBar->taskControl[channel] |= (u16) 0x8000;
}

/* Task 2 */

void MCD_startDmaChainEu(int *currBD, short srcIncr, short destIncr,
			 int xferSize, short xferSizeIncr, int *cSave,
			 volatile TaskTableEntry * taskTable, int channel)
{
	volatile TaskTableEntry *taskChan = taskTable + channel;

	MCD_SET_VAR(taskChan, 3, (u32) currBD);	/* var[3] */
	MCD_SET_VAR(taskChan, 25, (u32) (0xe000 << 16) | (0xffff & srcIncr));	/* inc[1] */
	MCD_SET_VAR(taskChan, 24, (u32) (0xe000 << 16) | (0xffff & destIncr));	/* inc[0] */
	MCD_SET_VAR(taskChan, 12, (u32) xferSize);	/* var[12] */
	MCD_SET_VAR(taskChan, 26, (u32) (0x2000 << 16) | (0xffff & xferSizeIncr));	/* inc[2] */
	MCD_SET_VAR(taskChan, 0, (u32) cSave);	/* var[0] */
	MCD_SET_VAR(taskChan, 1, (u32) 0x00000000);	/* var[1] */
	MCD_SET_VAR(taskChan, 2, (u32) 0x00000000);	/* var[2] */
	MCD_SET_VAR(taskChan, 4, (u32) 0x00000000);	/* var[4] */
	MCD_SET_VAR(taskChan, 5, (u32) 0x00000000);	/* var[5] */
	MCD_SET_VAR(taskChan, 6, (u32) 0x00000000);	/* var[6] */
	MCD_SET_VAR(taskChan, 7, (u32) 0x00000000);	/* var[7] */
	MCD_SET_VAR(taskChan, 8, (u32) 0x00000000);	/* var[8] */
	MCD_SET_VAR(taskChan, 9, (u32) 0x00000000);	/* var[9] */
	MCD_SET_VAR(taskChan, 10, (u32) 0x00000000);	/* var[10] */
	MCD_SET_VAR(taskChan, 11, (u32) 0x00000000);	/* var[11] */
	MCD_SET_VAR(taskChan, 13, (u32) 0x00000000);	/* var[13] */
	MCD_SET_VAR(taskChan, 14, (u32) 0x80000000);	/* var[14] */
	MCD_SET_VAR(taskChan, 15, (u32) 0x00000010);	/* var[15] */
	MCD_SET_VAR(taskChan, 16, (u32) 0x00000001);	/* var[16] */
	MCD_SET_VAR(taskChan, 17, (u32) 0x00000004);	/* var[17] */
	MCD_SET_VAR(taskChan, 18, (u32) 0x08000000);	/* var[18] */
	MCD_SET_VAR(taskChan, 27, (u32) 0x00000000);	/* inc[3] */
	MCD_SET_VAR(taskChan, 28, (u32) 0x80000000);	/* inc[4] */
	MCD_SET_VAR(taskChan, 29, (u32) 0xc0000000);	/* inc[5] */
	MCD_SET_VAR(taskChan, 30, (u32) 0x80000001);	/* inc[6] */
	MCD_SET_VAR(taskChan, 31, (u32) 0x40000000);	/* inc[7] */

	/* Set the task's Enable bit in its Task Control Register */
	MCD_dmaBar->taskControl[channel] |= (u16) 0x8000;
}

/* Task 3 */

void MCD_startDmaSingleEu(char *srcAddr, short srcIncr, char *destAddr,
			  short destIncr, int dmaSize, short xferSizeIncr,
			  int flags, int *currBD, int *cSave,
			  volatile TaskTableEntry * taskTable, int channel)
{
	volatile TaskTableEntry *taskChan = taskTable + channel;

	MCD_SET_VAR(taskChan, 8, (u32) srcAddr);	/* var[8] */
	MCD_SET_VAR(taskChan, 25, (u32) (0xe000 << 16) | (0xffff & srcIncr));	/* inc[1] */
	MCD_SET_VAR(taskChan, 3, (u32) destAddr);	/* var[3] */
	MCD_SET_VAR(taskChan, 24, (u32) (0xe000 << 16) | (0xffff & destIncr));	/* inc[0] */
	MCD_SET_VAR(taskChan, 4, (u32) dmaSize);	/* var[4] */
	MCD_SET_VAR(taskChan, 26, (u32) (0x2000 << 16) | (0xffff & xferSizeIncr));	/* inc[2] */
	MCD_SET_VAR(taskChan, 6, (u32) flags);	/* var[6] */
	MCD_SET_VAR(taskChan, 2, (u32) currBD);	/* var[2] */
	MCD_SET_VAR(taskChan, 0, (u32) cSave);	/* var[0] */
	MCD_SET_VAR(taskChan, 1, (u32) 0x00000000);	/* var[1] */
	MCD_SET_VAR(taskChan, 5, (u32) 0x00000000);	/* var[5] */
	MCD_SET_VAR(taskChan, 7, (u32) 0x00000000);	/* var[7] */
	MCD_SET_VAR(taskChan, 9, (u32) 0x00000000);	/* var[9] */
	MCD_SET_VAR(taskChan, 10, (u32) 0x00000001);	/* var[10] */
	MCD_SET_VAR(taskChan, 11, (u32) 0x00000004);	/* var[11] */
	MCD_SET_VAR(taskChan, 12, (u32) 0x08000000);	/* var[12] */
	MCD_SET_VAR(taskChan, 27, (u32) 0x00000000);	/* inc[3] */
	MCD_SET_VAR(taskChan, 28, (u32) 0xc0000000);	/* inc[4] */
	MCD_SET_VAR(taskChan, 29, (u32) 0x80000000);	/* inc[5] */
	MCD_SET_VAR(taskChan, 30, (u32) 0x80000001);	/* inc[6] */
	MCD_SET_VAR(taskChan, 31, (u32) 0x40000000);	/* inc[7] */

	/* Set the task's Enable bit in its Task Control Register */
	MCD_dmaBar->taskControl[channel] |= (u16) 0x8000;
}

/* Task 4 */

void MCD_startDmaENetRcv(char *bDBase, char *currBD, char *rcvFifoPtr,
			 volatile TaskTableEntry * taskTable, int channel)
{
	volatile TaskTableEntry *taskChan = taskTable + channel;

	MCD_SET_VAR(taskChan, 0, (u32) bDBase);	/* var[0] */
	MCD_SET_VAR(taskChan, 3, (u32) currBD);	/* var[3] */
	MCD_SET_VAR(taskChan, 6, (u32) rcvFifoPtr);	/* var[6] */
	MCD_SET_VAR(taskChan, 1, (u32) 0x00000000);	/* var[1] */
	MCD_SET_VAR(taskChan, 2, (u32) 0x00000000);	/* var[2] */
	MCD_SET_VAR(taskChan, 4, (u32) 0x00000000);	/* var[4] */
	MCD_SET_VAR(taskChan, 5, (u32) 0x00000000);	/* var[5] */
	MCD_SET_VAR(taskChan, 7, (u32) 0x00000000);	/* var[7] */
	MCD_SET_VAR(taskChan, 8, (u32) 0x00000000);	/* var[8] */
	MCD_SET_VAR(taskChan, 9, (u32) 0x0000ffff);	/* var[9] */
	MCD_SET_VAR(taskChan, 10, (u32) 0x30000000);	/* var[10] */
	MCD_SET_VAR(taskChan, 11, (u32) 0x0fffffff);	/* var[11] */
	MCD_SET_VAR(taskChan, 12, (u32) 0x00000008);	/* var[12] */
	MCD_SET_VAR(taskChan, 24, (u32) 0x00000000);	/* inc[0] */
	MCD_SET_VAR(taskChan, 25, (u32) 0x60000000);	/* inc[1] */
	MCD_SET_VAR(taskChan, 26, (u32) 0x20000004);	/* inc[2] */
	MCD_SET_VAR(taskChan, 27, (u32) 0x40000000);	/* inc[3] */

	/* Set the task's Enable bit in its Task Control Register */
	MCD_dmaBar->taskControl[channel] |= (u16) 0x8000;
}

/* Task 5 */

void MCD_startDmaENetXmit(char *bDBase, char *currBD, char *xmitFifoPtr,
			  volatile TaskTableEntry * taskTable, int channel)
{
	volatile TaskTableEntry *taskChan = taskTable + channel;

	MCD_SET_VAR(taskChan, 0, (u32) bDBase);	/* var[0] */
	MCD_SET_VAR(taskChan, 3, (u32) currBD);	/* var[3] */
	MCD_SET_VAR(taskChan, 11, (u32) xmitFifoPtr);	/* var[11] */
	MCD_SET_VAR(taskChan, 1, (u32) 0x00000000);	/* var[1] */
	MCD_SET_VAR(taskChan, 2, (u32) 0x00000000);	/* var[2] */
	MCD_SET_VAR(taskChan, 4, (u32) 0x00000000);	/* var[4] */
	MCD_SET_VAR(taskChan, 5, (u32) 0x00000000);	/* var[5] */
	MCD_SET_VAR(taskChan, 6, (u32) 0x00000000);	/* var[6] */
	MCD_SET_VAR(taskChan, 7, (u32) 0x00000000);	/* var[7] */
	MCD_SET_VAR(taskChan, 8, (u32) 0x00000000);	/* var[8] */
	MCD_SET_VAR(taskChan, 9, (u32) 0x00000000);	/* var[9] */
	MCD_SET_VAR(taskChan, 10, (u32) 0x00000000);	/* var[10] */
	MCD_SET_VAR(taskChan, 12, (u32) 0x00000000);	/* var[12] */
	MCD_SET_VAR(taskChan, 13, (u32) 0x0000ffff);	/* var[13] */
	MCD_SET_VAR(taskChan, 14, (u32) 0xffffffff);	/* var[14] */
	MCD_SET_VAR(taskChan, 15, (u32) 0x00000004);	/* var[15] */
	MCD_SET_VAR(taskChan, 16, (u32) 0x00000008);	/* var[16] */
	MCD_SET_VAR(taskChan, 24, (u32) 0x00000000);	/* inc[0] */
	MCD_SET_VAR(taskChan, 25, (u32) 0x60000000);	/* inc[1] */
	MCD_SET_VAR(taskChan, 26, (u32) 0x40000000);	/* inc[2] */
	MCD_SET_VAR(taskChan, 27, (u32) 0xc000fffc);	/* inc[3] */
	MCD_SET_VAR(taskChan, 28, (u32) 0xe0000004);	/* inc[4] */
	MCD_SET_VAR(taskChan, 29, (u32) 0x80000000);	/* inc[5] */
	MCD_SET_VAR(taskChan, 30, (u32) 0x4000ffff);	/* inc[6] */
	MCD_SET_VAR(taskChan, 31, (u32) 0xe0000001);	/* inc[7] */

	/* Set the task's Enable bit in its Task Control Register */
	MCD_dmaBar->taskControl[channel] |= (u16) 0x8000;
}
