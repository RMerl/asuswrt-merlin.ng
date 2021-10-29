/*
    Copyright 2000-2011 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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
