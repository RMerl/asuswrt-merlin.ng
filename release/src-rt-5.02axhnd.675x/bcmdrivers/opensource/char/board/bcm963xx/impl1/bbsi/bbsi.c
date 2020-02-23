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

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <bcm_map_part.h>
#include <linux/device.h>
#include <bcmSpiRes.h>
#include <board.h>
#include <boardparms.h>
#include <linux/mii.h>
#include <6802_map_part.h>
#include "bbsi.h"
#include "../moca/board_moca.h"

/***************************************************************************
* File Name  : bbsi.c
*
* Description: This file contains the functions for communicating between a brcm
*              cpe chip(eg 6818) to another brcm chip(6802) which is connected 
*              as a spi slave device. This protocol used to communicate is BBSI.
*
***************************************************************************/

/*********************************************************************************************************
 * Eg. configuration required for spi slave devices
 * 
 * 6368: BcmSpiReserveSlave2(HS_SPI_BUS_NUM, 7, 781000, SPI_MODE_3, SPI_CONTROLLER_STATE_GATE_CLK_SSOFF);
 *
 *
 **********************************************************************************************************/

#define BCM_SPI_SLAVE_ID     3
#define BCM_SPI_SLAVE_FREQ   20000000
#define MAX_STATUS_RETRY 5

#define bcmSpiSlaveResetGpio    pSpiDev->resetGpio
#define bcmSpiSlaveBootModeGpio pSpiDev->bootModeGpio
#define bcmSpiSlaveBus          pSpiDev->busNum
#define bcmSpiSlaveId           pSpiDev->select
#define bcmSpiSlaveMaxFreq      pSpiDev->maxFreq
#define bcmSpiSlaveMode         pSpiDev->mode
#define bcmSpiSlaveCtrState     pSpiDev->ctrlState

PBP_SPISLAVE_INFO spiSlaveInfo[MAX_SPISLAVE_DEV_NUM];


static int getSpiSlaveDeviceInfo(int dev);
static void resetSpiSlaveDevice(int dev);
static PBP_SPISLAVE_INFO getMocaSpidDev(int dev);


static struct mutex bcmSpiSlaveMutex[MAX_SPISLAVE_DEV_NUM];

static PBP_SPISLAVE_INFO getMocaSpidDev(int dev)
{
    PBP_MOCA_INFO pMoca = NULL;

    pMoca = boardGetMocaInfo(dev);
    if(pMoca == NULL)
    {
        printk(KERN_ERR "getMocaSpidDev invalid dev number %d\n", dev);
        return NULL;
    }

    return &pMoca->spiDevInfo;
}

static int isBBSIDone(PBP_SPISLAVE_INFO pSpiDev)
{
    uint8_t read_status[2] = { BBSI_COMMAND_BYTE,// | 0x1, // Do a Read
                               STATUS_REGISTER_ADDR
                             };
    uint8_t read_rx;
    int status;
    int i;
    int ret = 0;

    for (i=0; i<MAX_STATUS_RETRY; i++)
    {   
        status = BcmSpiSyncTrans(read_status, &read_rx, 2, 1, bcmSpiSlaveBus, bcmSpiSlaveId);
        if ( SPI_STATUS_OK != status )
        {
           printk(KERN_ERR "isBBSIDone: BcmSpiSyncTrans returned error\n");          
           ret = 0;
           break;
        }
        status = read_rx;
        
        if (status & 0xF)
        {
           printk(KERN_ERR "isBBSIDone: BBSI transaction error, status=0x%x\n", status);          
           ret = 0; 
           break;
        }
        else if ( (status & (1<<BUSY_SHIFT)) == 0 )
        {
            ret = 1;
            break;
        }
    }   

    return ret;
}

static int doRead(PBP_SPISLAVE_INFO pSpiDev, unsigned int addr, unsigned int *data, unsigned int len)
{
    uint8_t buf[12];
    int status;   

    buf[0]  = BBSI_COMMAND_BYTE | 0x1;          
    buf[1]  = CONFIG_REGISTER_ADDR;       /* Start the writes from this addr */
    buf[2]  = ( (4-len) << XFER_MODE_SHIFT ) |  0x1; /* Indicates the transaction is 32bit, 24bit, 16bit or 8bit. Len is 1..4 */
    buf[3]  = (addr >> 24) & 0xFF;  /* Assuming MSB bytes are always sent first */
    buf[4]  = (addr >> 16) & 0xFF;
    buf[5]  = (addr >> 8)  & 0xFF;
    buf[6]  = (addr >> 0)  & 0xFF;
  
    status = BcmSpiSyncTrans(buf, NULL, 0, 7, bcmSpiSlaveBus, bcmSpiSlaveId);
    
    if ( SPI_STATUS_OK != status )
    {
       printk(KERN_ERR "SPI Slave Read: BcmSpiSyncTrans returned error\n");
       return(-1);
    }

    if (!isBBSIDone(pSpiDev))
    {
       printk(KERN_ERR "SPI Slave Read: read to addr:0x%x failed\n", addr);
       return(-1);
    }

    buf[0] = BBSI_COMMAND_BYTE; //read
    buf[1] = DATA0_REGISTER_ADDR;
    status = BcmSpiSyncTrans(buf, buf, 2, 4, bcmSpiSlaveBus, bcmSpiSlaveId);
    if ( SPI_STATUS_OK != status )
    {
       printk(KERN_ERR "kerSysBcmSpiSlaveRead: BcmSpiSyncTrans returned error\n");
       return(-1);
    }

    *data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];

    return(0);
}

static int doWrite(PBP_SPISLAVE_INFO pSpiDev, unsigned int addr, unsigned int data, unsigned int len)
{
   uint8_t buf[12]; 
   int status;

   data <<= (8 * (4 - len)); // Do we have to do this? It will matter only for len = 1 or 2.

   buf[0]  = BBSI_COMMAND_BYTE | 0x1;          /* Assumes write signal is 0 */
   buf[1]  = CONFIG_REGISTER_ADDR;       /* Start the writes from this addr */
   buf[2]  = (4-len) << XFER_MODE_SHIFT ; /* Indicates the transaction is 32bit, 24bit, 16bit or 8bit. Len is 1..4 */
   buf[3]  = (addr >> 24) & 0xFF;  /* Assuming MSB bytes are always sent first */
   buf[4]  = (addr >> 16) & 0xFF;
   buf[5]  = (addr >> 8)  & 0xFF;
   buf[6]  = (addr >> 0)  & 0xFF;
   buf[7]  = (data >> 24) & 0xFF;
   buf[8]  = (data >> 16) & 0xFF;
   buf[9]  = (data >> 8)  & 0xFF;
   buf[10] = (data >> 0)  & 0xFF;

   
   status = BcmSpiSyncTrans(buf, NULL, 0,11, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "SPI Slave Write: BcmSpiSyncTrans returned error\n");
      return(-1);
   }

   if (!isBBSIDone(pSpiDev))
   {
      printk(KERN_ERR "SPI Slave Write: write to addr:0x%x failed\n", addr);
      return(-1);
   }

   return(0);
}

static int doReadBuffer(PBP_SPISLAVE_INFO pSpiDev, unsigned int addr, unsigned int *data, unsigned int len)
{
    uint8_t buf[12];
    int status;   

    buf[0]  = BBSI_COMMAND_BYTE | 0x1;          
    buf[1]  = CONFIG_REGISTER_ADDR;       /* Start the writes from this addr */
    buf[2]  = 0x3; /* Indicates the transaction is 32bit, 24bit, 16bit or 8bit. Len is 1..4 */
    buf[3]  = (addr >> 24) & 0xFF;  /* Assuming MSB bytes are always sent first */
    buf[4]  = (addr >> 16) & 0xFF;
    buf[5]  = (addr >> 8)  & 0xFF;
    buf[6]  = (addr >> 0)  & 0xFF;
  
    status = BcmSpiSyncTrans(buf, NULL, 0, 7, bcmSpiSlaveBus, bcmSpiSlaveId);
    
    if ( SPI_STATUS_OK != status )
    {
       printk(KERN_ERR "SPI Slave Read: BcmSpiSyncTrans returned error\n");
       return(-1);
    }

    if (!isBBSIDone(pSpiDev))
    {
       printk(KERN_ERR "SPI Slave Read: read to addr:0x%x failed\n", addr);
       return(-1);
    }

    buf[0] = BBSI_COMMAND_BYTE; //read
    buf[1] = DATA0_REGISTER_ADDR;

    while (len)
    {
       unsigned int count;
  
       count = (len > 4?4:len);

       status = BcmSpiSyncTrans(buf, (uint8_t *) data, 2, count, bcmSpiSlaveBus, bcmSpiSlaveId);
       if ( SPI_STATUS_OK != status )
       {
          printk(KERN_ERR "kerSysBcmSpiSlaveRead: BcmSpiSyncTrans returned error\n");
          return(-1);
       }

       if (!isBBSIDone(pSpiDev))
       {
          printk(KERN_ERR "SPI Slave Read: read to addr:0x%x failed\n", addr);
          return(-1);
       }

       len -= count;
       data += count / 4;
    }

    return(0);
}

static int doWriteBuffer(PBP_SPISLAVE_INFO pSpiDev, unsigned int addr, unsigned int *data, unsigned int len)
{
   uint8_t buf[512]; 
   int status;

   if (len > 504)  // 7 bytes are used for addressing and BBSI protocol
   {
      printk(KERN_ERR "SPI Slave Write: write to addr:0x%x failed.  Len (%d) too long.\n", addr, len);
      return(-1);
   }

   buf[0]  = BBSI_COMMAND_BYTE | 0x1;            /* Assumes write signal is 0 */
   buf[1]  = CONFIG_REGISTER_ADDR;         /* Start the writes from this addr */
   buf[2]  = 0 ;                                  /* Transactions are 32-bits */
   buf[3]  = (addr >> 24) & 0xFF; /* Assuming MSB bytes are always sent first */
   buf[4]  = (addr >> 16) & 0xFF;
   buf[5]  = (addr >> 8)  & 0xFF;
   buf[6]  = (addr >> 0)  & 0xFF;

   memcpy(&buf[7], data, len);
 
   status = BcmSpiSyncTrans(buf, NULL, 0, 7+len, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "SPI Slave Write: BcmSpiSyncTrans returned error\n");
      return(-1);
   }

   if (!isBBSIDone(pSpiDev))
   {
      printk(KERN_ERR "SPI Slave Write: write to addr:0x%x failed\n", addr);
      return(-1);
   }
   return(0);
}

int kerSysBcmSpiSlaveInit( int dev )
{
    unsigned int data;
    int32_t       retVal = 0;
    int           status;
    PBP_SPISLAVE_INFO pSpiDev;

    if(dev >= MAX_SPISLAVE_DEV_NUM)
        return(SPI_STATUS_ERR);

    printk(KERN_INFO "%s called for dev %d\n", __FUNCTION__, dev);
    mutex_init(&bcmSpiSlaveMutex[dev]);
    if( getSpiSlaveDeviceInfo(dev) != 0 )
        return(SPI_STATUS_ERR);

    resetSpiSlaveDevice(dev);

    pSpiDev = spiSlaveInfo[dev];
    status = BcmSpiReserveSlave2(bcmSpiSlaveBus, bcmSpiSlaveId, bcmSpiSlaveMaxFreq, bcmSpiSlaveMode, bcmSpiSlaveCtrState);
    if ( SPI_STATUS_OK != status )
    {
      printk(KERN_ERR "%s: BcmSpiReserveSlave2 returned error %d\n", __FUNCTION__, status);
      return(SPI_STATUS_ERR);
    }
  
     
    if ((kerSysBcmSpiSlaveRead(dev, SUN_TOP_CTRL_CHIP_FAMILY_ID, &data, 4) == -1) ||
       (data == 0) || (data == 0xffffffff))
    {   
      printk(KERN_ERR "%s: Failed to read the SUN_TOP_CTRL_CHIP_FAMILY_ID: 0x%08x\n", __FUNCTION__, (unsigned int)data);
      return -1;
    }
    else
    {
      printk(KERN_INFO "%s: SUN_TOP_CTRL_CHIP_FAMILY_ID: 0x%08x\n", __FUNCTION__, (unsigned int)data);
    }
    
    if ((kerSysBcmSpiSlaveRead(dev, SUN_TOP_CTRL_PRODUCT_ID, &data, 4) == -1) ||
       (data == 0) || (data == 0xffffffff))
    {
      printk(KERN_ERR "%s: Failed to read the SUN_TOP_CTRL_PRODUCT_ID: 0x%08x\n", __FUNCTION__, (unsigned int)data);
      return -1;
    }
    else
    {
      printk(KERN_INFO "%s: SUN_TOP_CTRL_PRODUCT_ID: 0x%08x\n", __FUNCTION__, (unsigned int)data);
    }

    return( retVal );

}

int kerSysBcmSpiSlaveRead(int dev, unsigned int addr, unsigned int *data, unsigned int len)
{
    PBP_SPISLAVE_INFO pSpiDev;
    int ret;

    if( dev >= MAX_SPISLAVE_DEV_NUM || (pSpiDev = spiSlaveInfo[dev]) == NULL )
       return(-1);

    mutex_lock(&bcmSpiSlaveMutex[dev]);

    ret = doRead(pSpiDev, addr, data, len);

    mutex_unlock(&bcmSpiSlaveMutex[dev]);

    return(ret);
}

int kerSysBcmSpiSlaveWrite(int dev, unsigned int addr, unsigned int data, unsigned int len)
{
   PBP_SPISLAVE_INFO pSpiDev;
   int ret;

   if( dev >= MAX_SPISLAVE_DEV_NUM || (pSpiDev = spiSlaveInfo[dev]) == NULL )
      return(-1);

   mutex_lock(&bcmSpiSlaveMutex[dev]);

   ret = doWrite(pSpiDev, addr, data, len);
   mutex_unlock(&bcmSpiSlaveMutex[dev]);

   return(ret);
}

int kerSysBcmSpiSlaveReadBuf(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize)
{
    int ret = SPI_STATUS_ERR;  

    PBP_SPISLAVE_INFO pSpiDev;

    if( dev >= MAX_SPISLAVE_DEV_NUM || (pSpiDev = spiSlaveInfo[dev]) == NULL )
       return(-1);

    addr &= 0x1fffffff;

    mutex_lock(&bcmSpiSlaveMutex[dev]);
    ret = doReadBuffer(pSpiDev, addr, data, len);
    mutex_unlock(&bcmSpiSlaveMutex[dev]);

    return ret;
}

int kerSysBcmSpiSlaveWriteBuf(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize)
{
    int ret = SPI_STATUS_ERR;  
    int count = 0;

    PBP_SPISLAVE_INFO pSpiDev;

    if( dev >= MAX_SPISLAVE_DEV_NUM || (pSpiDev = spiSlaveInfo[dev]) == NULL )
       return(-1);

    addr &= 0x1fffffff;

    mutex_lock(&bcmSpiSlaveMutex[dev]);
    while (len)
    {
        count = (len>500?500:len);

        ret = doWriteBuffer(pSpiDev, addr, data, count);
        if (ret)
            break;

        len -= count;
        addr += count;
        data += count/sizeof(unsigned int);
    }
    mutex_unlock(&bcmSpiSlaveMutex[dev]);

    return ret;
}

int kerSysBcmSpiSlaveModify(int dev, unsigned int addr, unsigned int data, unsigned int mask, unsigned int len)
{
   PBP_SPISLAVE_INFO pSpiDev;
   unsigned int regVal;
   int ret;

   if( dev >= MAX_SPISLAVE_DEV_NUM || (pSpiDev = spiSlaveInfo[dev]) == NULL )
      return(-1);

   mutex_lock(&bcmSpiSlaveMutex[dev]);

   ret = doRead(pSpiDev, addr, &regVal, len);

   if (ret == 0)
   {
      /* Make sure that the bits that are set in both data and mask are set */
      regVal |= (data & mask);

      /* Make sure that the bits that are unset in data and set in mask are unset */
      regVal &= ~((~data) & mask);
      
      ret = doWrite(pSpiDev, addr, regVal, len);
   }
   mutex_unlock(&bcmSpiSlaveMutex[dev]);

   return(ret);
}

unsigned int kerSysBcmSpiSlaveReadReg32(int dev, unsigned int addr)
{
   unsigned int data = 0;
   BUG_ON(addr & 3);
   addr &= 0x1fffffff;
   
   if(kerSysBcmSpiSlaveRead(dev, addr, &data, 4) < 0)
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveReadReg32: dev %d can't read %08x\n", dev, (unsigned int)addr);
   }

   return(data);
}

void kerSysBcmSpiSlaveWriteReg32(int dev, unsigned int addr, unsigned int data)
{
   BUG_ON(addr & 3);
   addr &= 0x1fffffff;

   if(kerSysBcmSpiSlaveWrite(dev, addr, data, 4) < 0)
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveWriteReg32: dev %d can't write %08x (data %08x)\n", dev, (unsigned int)addr, (unsigned int)data);
   }

}

 // Move these functions to a moca board file.
static void resetSpiSlaveDevice(int dev)
{
    PBP_SPISLAVE_INFO pSpiDev;

    if( dev >= MAX_SPISLAVE_DEV_NUM || (pSpiDev = spiSlaveInfo[dev]) == NULL )
        return;

    printk(KERN_ERR "Entering %s: bcmSpiSlaveResetGpio = %d\n", __FUNCTION__, bcmSpiSlaveResetGpio);

    if ( bcmSpiSlaveBootModeGpio != 0xFF )
    {
        kerSysSetGpioState(bcmSpiSlaveBootModeGpio, kGpioActive);
    }
    if ( bcmSpiSlaveResetGpio != 0xFF )
    {
        kerSysSetGpioState(bcmSpiSlaveResetGpio,  kGpioInactive);
        mdelay(30);
        kerSysSetGpioState(bcmSpiSlaveResetGpio,  kGpioActive); 
        mdelay(50);
        kerSysSetGpioState(bcmSpiSlaveResetGpio,  kGpioInactive);
        mdelay(300);
    }
  
}

 
static int getSpiSlaveDeviceInfo(int dev)
{
    PBP_SPISLAVE_INFO pSpiDev;

    if( dev >= MAX_SPISLAVE_DEV_NUM )
        return(SPI_STATUS_ERR);

    if( (pSpiDev = getMocaSpidDev(dev)) == NULL )
        return(SPI_STATUS_ERR);

    spiSlaveInfo[dev] = pSpiDev;

    if ( pSpiDev->resetGpio == BP_NOT_DEFINED )
    {
        pSpiDev->resetGpio = 0xff;
    }
    printk(KERN_INFO "%s: bcmSpiSlaveResetGpio = %d\n", __FUNCTION__, pSpiDev->resetGpio);

    if ( pSpiDev->bootModeGpio == BP_NOT_DEFINED )
    {
        pSpiDev->bootModeGpio = 0xff;
    }
    printk(KERN_INFO "%s: bcmSpiSlaveBootModeGpio = %d\n", __FUNCTION__, pSpiDev->bootModeGpio);

    if ( pSpiDev->busNum == BP_NOT_DEFINED )
    {
        printk(KERN_ERR "spi device slave bus num not defined!\n");
        return -1;
    }
    printk(KERN_INFO "%s: bcmSpiSlaveBus = %d\n", __FUNCTION__, pSpiDev->busNum);

    if ( pSpiDev->select == BP_NOT_DEFINED )
    {
        printk(KERN_ERR "spi device slave id not defined!\n");
        return -1;
    }
    printk(KERN_INFO"%s: bcmSpiSlaveId = %d\n", __FUNCTION__, pSpiDev->select);


    if ( pSpiDev->mode == BP_NOT_DEFINED )
    {
        pSpiDev->mode = SPI_MODE_3;
    }
    printk(KERN_INFO "%s: bcmSpiSlaveMode = %d\n", __FUNCTION__, pSpiDev->mode);
    
    if ( pSpiDev->ctrlState == BP_NOT_DEFINED )
    {
        pSpiDev->ctrlState = SPI_CONTROLLER_STATE_GATE_CLK_SSOFF;
    }
    printk(KERN_INFO "%s: bcmSpiSlaveCtrState = 0x%x\n", __FUNCTION__, pSpiDev->ctrlState);

    if ( pSpiDev->maxFreq == BP_NOT_DEFINED )
    {
        pSpiDev->maxFreq = BCM_SPI_SLAVE_FREQ;
    }
    printk(KERN_INFO "%s: bcmSpiSlaveMaxFreq = %d\n", __FUNCTION__, pSpiDev->maxFreq);
    
    return 0;
}


