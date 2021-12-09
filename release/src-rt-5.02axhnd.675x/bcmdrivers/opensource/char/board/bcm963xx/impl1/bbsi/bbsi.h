/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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

#ifndef _BBSI_H
#define _BBSI_H


#define BBSI_COMMAND_BYTE 0x80


#define STATUS_REGISTER_ADDR  0x6


#define CPU_RUNNING_SHIFT 0x6
#define CPU_RUNNING_MASK  0x1

#define HAB_REQ_SHIFT     0x5
#define HAB_REQ_MASK      0x1

#define BUSY_SHIFT        0x4
#define BUSY_MASK         0x1

#define RBUS_UNEXP_TX_SHIFT  0x3
#define RBUS_UNEXP_TX_MASK   0x1

#define RBUS_TIMEOUT_SHIFT   0x2
#define RBUS_TIMEOUT_MASK    0x1

#define RBUS_ERR_ACK_SHIFT   0x1
#define RBUS_ERR_ACK_MASK    0x1

#define ERROR_SHIFT   0x0
#define ERROR_MASK    0x1



#define CONFIG_REGISTER_ADDR  0x7

#define DATA0_REGISTER_ADDR  0xC

#define XFER_MODE_SHIFT  0x3
#define XFER_MODE_MASK   0x3

#define NO_RBUS_ADDR_INC_SHIFT     0x2
#define NO_RBUS_ADDR_INC_MASK      0x1

#define SPECULATIVE_READ_EN_SHIFT   0x1
#define SPECULATIVE_READ_EN_MASK    0x1

#define READ_RBUS_SHIFT   0x0
#define READ_RBUS_MASK    0x1

#define MAX_SPISLAVE_DEV_NUM	   BP_MOCA_MAX_NUM

int kerSysBcmSpiSlaveInit(int dev);
int kerSysBcmSpiSlaveRead(int dev, unsigned int addr, unsigned int * data, unsigned int len);
unsigned int kerSysBcmSpiSlaveReadReg32(int dev, unsigned int addr);
int kerSysBcmSpiSlaveWrite(int dev, unsigned int addr, unsigned int data, unsigned int len);
void kerSysBcmSpiSlaveWriteReg32(int dev, unsigned int addr, unsigned int data);
int kerSysBcmSpiSlaveWriteBuf(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize);
int kerSysBcmSpiSlaveModify(int dev, unsigned int addr, unsigned int data, unsigned int mask, unsigned int len);
int kerSysBcmSpiSlaveReadBuf(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize);


#endif
