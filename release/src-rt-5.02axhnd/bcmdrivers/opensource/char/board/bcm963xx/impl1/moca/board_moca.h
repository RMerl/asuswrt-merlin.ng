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

#ifndef _BOARD_MOCA_H_
#define _BOARD_MOCA_H_

void board_mocaInit(void);
int board_mocaPreInit(void);
int boardGetMocaDevNum(void);
PBP_MOCA_INFO boardGetMocaInfo(int dev);

typedef struct
{
    int dev;
    MocaHostIntrCallback mocaCallback;
    void * userArg;
    int irq;
    int intrGpio;
    atomic_t disableCount;
} MOCA_INTR_ARG, *PMOCA_INTR_ARG;


#endif
