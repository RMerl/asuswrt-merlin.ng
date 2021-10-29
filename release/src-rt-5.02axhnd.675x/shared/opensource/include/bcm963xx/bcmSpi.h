/*
   <:copyright-BRCM:2010:DUAL/GPL:standard
   
      Copyright (c) 2010 Broadcom 
      All Rights Reserved
   
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

#ifndef __BCM_SPI_H__
#define __BCM_SPI_H__

#ifndef _CFE_ 


#include <linux/version.h>
#include <generated/autoconf.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>

struct bcmspi_xferInfo
{
    struct spi_transfer *pCurXfer;
    u32                  remTxLen;
    u32                  remRxLen;
    u32                  prependCnt;
    u16                  maxLen;
    char                *rxBuf;
    const char          *txBuf;
    u16                  msgType;
    u32                  addr;
    u8                   addrLen;
    u8                   addrOffset;
    unsigned char        header[16];
    unsigned char       *pHdr;
    u8                   bitRedux;
    u32                  delayUsecs;
};

struct bcmspi
{
   spinlock_t               lock;
   unsigned int             bus_num;
   unsigned int             num_chipselect;
   struct list_head         queue;
   struct platform_device  *pdev;
   u8                       stopping;
   struct spi_message      *pCurMsg;
   u8                       xferIdx;
   u8                       pingProgNext;
   u8                       pingEndNext;
   u8                       ping0Started;
   u8                       ping0Xfer;

   u8                       ping1Started;
   u8                       ping1Xfer;
   struct bcmspi_xferInfo   spiXfer[2];
};
#endif


#endif /* __BCM_SPI_H__ */

