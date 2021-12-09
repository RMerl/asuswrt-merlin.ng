/*
    Copyright 2000-2011 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard
    
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

#ifndef _SPIDEVICES_H
#define _SPIDEVICES_H

int kerSysBcmSpiSlaveInit(int dev);
int kerSysBcmSpiSlaveRead(int dev, unsigned int addr, unsigned int * data, unsigned int len);
unsigned int kerSysBcmSpiSlaveReadReg32(int dev, unsigned int addr);
int kerSysBcmSpiSlaveWrite(int dev, unsigned int addr, unsigned int data, unsigned int len);
void kerSysBcmSpiSlaveWriteReg32(int dev, unsigned int addr, unsigned int data);
int kerSysBcmSpiSlaveWriteBuf(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize);

/* wrapper for 6829 spi device. Only one slave device */
#define Bcm6829SpiSlaveRead(addr, data, len)          kerSysBcmSpiSlaveRead(0, addr, data, len)
#define Bcm6829SpiSlaveRead32(addr)                   kerSysBcmSpiSlaveReadReg32(0, addr)
#define Bcm6829SpiSlaveWrite(addr, data, len)         kerSysBcmSpiSlaveWrite(0, addr, data, len)
#define Bcm6829SpiSlaveWrite32(addr, data)            kerSysBcmSpiSlaveWriteReg32(0, addr, data)

#endif
