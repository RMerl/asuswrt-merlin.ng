/*
   <:copyright-BRCM:2010:DUAL/GPL:standard
   
      Copyright (c) 2010 Broadcom 
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

