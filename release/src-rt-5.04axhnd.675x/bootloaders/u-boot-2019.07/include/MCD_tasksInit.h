/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 */

#ifndef MCD_TSK_INIT_H
#define MCD_TSK_INIT_H 1

/*
 * Do not edit!
 */

/* Task 0 */
void MCD_startDmaChainNoEu(int *currBD, short srcIncr, short destIncr,
			   int xferSize, short xferSizeIncr, int *cSave,
			   volatile TaskTableEntry * taskTable, int channel);

/* Task 1 */
void MCD_startDmaSingleNoEu(char *srcAddr, short srcIncr, char *destAddr,
			    short destIncr, int dmaSize, short xferSizeIncr,
			    int flags, int *currBD, int *cSave,
			    volatile TaskTableEntry * taskTable, int channel);

/* Task 2 */
void MCD_startDmaChainEu(int *currBD, short srcIncr, short destIncr,
			 int xferSize, short xferSizeIncr, int *cSave,
			 volatile TaskTableEntry * taskTable, int channel);

/* Task 3 */
void MCD_startDmaSingleEu(char *srcAddr, short srcIncr, char *destAddr,
			  short destIncr, int dmaSize, short xferSizeIncr,
			  int flags, int *currBD, int *cSave,
			  volatile TaskTableEntry * taskTable, int channel);

/* Task 4 */
void MCD_startDmaENetRcv(char *bDBase, char *currBD, char *rcvFifoPtr,
			 volatile TaskTableEntry * taskTable, int channel);

/* Task 5 */
void MCD_startDmaENetXmit(char *bDBase, char *currBD, char *xmitFifoPtr,
			  volatile TaskTableEntry * taskTable, int channel);

#endif				/* MCD_TSK_INIT_H */
