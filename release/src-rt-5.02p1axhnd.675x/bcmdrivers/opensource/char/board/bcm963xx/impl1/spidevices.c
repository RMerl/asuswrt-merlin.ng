/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <bcm_map_part.h>
#include <linux/device.h>
#include <bcmSpiRes.h>
#include <spidevices.h>
#include <board.h>
#include <boardparms.h>
#include <linux/mii.h>

/***************************************************************************
* File Name  : spidevices.c
*
* Description: This file contains the functions for communicating between a brcm
*              cpe chip(63268) to another brcm cpe chip(6368) which is connected
*              as a spi slave device.
*
***************************************************************************/

/*********************************************************************************************************
 * Eg. configuration required for spi slave devices
 *
 * 6368: BcmSpiReserveSlave2(HS_SPI_BUS_NUM, 7, 781000, SPI_MODE_3, SPI_CONTROLLER_STATE_GATE_CLK_SSOFF);
 *
 *
 **********************************************************************************************************/

extern spinlock_t bcm_gpio_spinlock;

#define BCM_SPI_SLAVE_ID     1
#define BCM_SPI_SLAVE_FREQ   6250000

static unsigned int bcmSpiSlaveResetGpio = 0xFF;
static unsigned int bcmSpiSlaveBus       = LEG_SPI_BUS_NUM;
static unsigned int bcmSpiSlaveId        = BCM_SPI_SLAVE_ID;
static unsigned int bcmSpiSlaveMaxFreq   = BCM_SPI_SLAVE_FREQ;
static unsigned int bcmSpiSlaveMode      = SPI_MODE_DEFAULT;
static unsigned int bcmSpiSlaveCtrState  = SPI_CONTROLLER_STATE_CPHA_EXT;
static unsigned int bcmSpiSlaveProtoRev  = 1;

// HW SPI supports multiple modes
static int kerSysBcmSpiSlaveReset_rev0(int dev);
static int kerSysBcmSpiSlaveInit_rev0(int dev);
static int kerSysBcmSpiSlaveRead_rev0(int dev, unsigned int addr, unsigned int *data, unsigned int len);
static int kerSysBcmSpiSlaveWrite_rev0(int dev, unsigned int addr, unsigned int data, unsigned int len);
static int kerSysBcmSpiSlaveWriteBuf_rev0(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize);

static int kerSysBcmSpiSlaveReset_rev1(int dev);
static int kerSysBcmSpiSlaveInit_rev1(int dev);
static int kerSysBcmSpiSlaveRead_rev1(int dev, unsigned int addr, unsigned int *data, unsigned int len);
static int kerSysBcmSpiSlaveWrite_rev1(int dev, unsigned int addr, unsigned int data, unsigned int len);
static int kerSysBcmSpiSlaveWriteBuf_rev1(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize);

static int kerSysBcmSpiSlaveReset_rev2(int dev);
static int kerSysBcmSpiSlaveInit_rev2(int dev);
static int kerSysBcmSpiSlaveRead_rev2(int dev, unsigned int addr, unsigned int *data, unsigned int len);
static int kerSysBcmSpiSlaveWrite_rev2(int dev, unsigned int addr, unsigned int data, unsigned int len);
static int kerSysBcmSpiSlaveWriteBuf_rev2(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize);

static void getSpiSlaveDeviceInfo( void );

typedef int (*spiSlaveReset)(int dev);
typedef int (*spiSlaveInit)(int dev);
typedef int (*spiSlaveRead)(int dev, unsigned int addr, unsigned int *data, unsigned int len);
typedef int (*spiSlaveWrite)(int dev, unsigned int addr, unsigned int data, unsigned int len);
typedef int (*spiSlaveWriteBuf)(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize);

typedef struct
{
    spiSlaveReset slaveReset;
    spiSlaveInit slaveInit;
    spiSlaveRead slaveRead;
    spiSlaveWrite slaveWrite;
    spiSlaveWriteBuf slaveWriteBuf;
} spiSlaveOps;

static spiSlaveOps spiOps[3] = {
                            {kerSysBcmSpiSlaveReset_rev0, kerSysBcmSpiSlaveInit_rev0, kerSysBcmSpiSlaveRead_rev0, kerSysBcmSpiSlaveWrite_rev0, kerSysBcmSpiSlaveWriteBuf_rev0},
                            {kerSysBcmSpiSlaveReset_rev1, kerSysBcmSpiSlaveInit_rev1, kerSysBcmSpiSlaveRead_rev1, kerSysBcmSpiSlaveWrite_rev1, kerSysBcmSpiSlaveWriteBuf_rev1},
                            {kerSysBcmSpiSlaveReset_rev2, kerSysBcmSpiSlaveInit_rev2, kerSysBcmSpiSlaveRead_rev2, kerSysBcmSpiSlaveWrite_rev2, kerSysBcmSpiSlaveWriteBuf_rev2}
                        };


#define BCM_SPI_MAX_NB_SLAVES 8
typedef struct
{
    unsigned int nbSlaves;
    unsigned int resetGpio[BCM_SPI_MAX_NB_SLAVES];
    unsigned int id[BCM_SPI_MAX_NB_SLAVES];
    unsigned int bus;
    unsigned int maxFreq;
    unsigned int mode;
    unsigned int ctrState;
} spiSlaveInfo;

static spiSlaveInfo spiInfo;
static struct mutex bcmSpiSlaveMutex;


static uint8_t  init_seq_rev0[3] = { 0x11, 0x01, 0xfc };
static uint8_t  init_adr_rev0[8] = { 0x11, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uint8_t  init_seq_rev1[3] = { 0x11, 0x01, 0xfd };
static uint8_t  init_cfg_rev1[3] = { 0x11, 0x03, 0x58 };
static uint8_t  init_adr_rev1[7] = { 0x11, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 };


static int spi_setup_addr( uint32_t addr, uint32_t len )
{
   uint8_t buf[7];
   int     status;

   if ((addr & ~(len-1)) != addr)
   {
      printk(KERN_ERR "spi_setup_addr: Invalid address - bad alignment\n");
      return(-1);
   }

   buf[0] = 0x11;
   buf[1] = 0x01;
   buf[2] = ((1 << len) - 1) << ((4 - len) - (addr & 3));
   buf[3] = (uint8_t)(addr >> 0);
   buf[4] = (uint8_t)(addr >> 8);
   buf[5] = (uint8_t)(addr >> 16);
   buf[6] = (uint8_t)(addr >> 24);

   status = BcmSpiSyncTrans(buf, NULL, 0, 7, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "spi_setup_addr: BcmSpiSyncTrans error\n");
      return(-1);
   }

   return(0);
}

static int spi_read_status(uint8_t *data)
{
   uint8_t read_status[2] = {0x10, 0x00};
   int     status;

   status = BcmSpiSyncTrans(read_status, &read_status[0], 2, 1, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "spi_read_status: BcmSpiSyncTrans returned error\n");
      *data = read_status[0];
      return(-1);
   }

   *data = read_status[0];
   return(0);
}

static int kerSysBcmSpiSlaveRead_rev0(int dev, unsigned int addr, unsigned int *data, unsigned int len)
{
   uint8_t buf[4] = { 0, 0, 0, 0 };
   int     status;
   UNUSED_PARAM(dev);

   *data = 0;
   mutex_lock(&bcmSpiSlaveMutex);

   addr &= 0x1fffffff;
   spi_setup_addr( addr, len );

   buf[0] = 0x12;
   buf[1] = (uint8_t)(addr >> 0);
   status = BcmSpiSyncTrans(&buf[0], &buf[0], 2, len, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveRead: BcmSpiSyncTrans returned error\n");
      mutex_unlock(&bcmSpiSlaveMutex);
      return(-1);
   }

   *data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
   *data >>= ((4 - len) * 8);

   if((spi_read_status(&buf[0]) == -1) || (buf[0] & 0x0f))
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveRead: spi_read_status returned error - %02x\n", buf[0]);
      mutex_unlock(&bcmSpiSlaveMutex);
      return(-1);
   }

   mutex_unlock(&bcmSpiSlaveMutex);

   return(0);
}

static int kerSysBcmSpiSlaveWrite_rev0(int dev, unsigned int addr, unsigned int data, unsigned int len)
{
   uint8_t buf[6];
   int     status;
   UNUSED_PARAM(dev);

   mutex_lock(&bcmSpiSlaveMutex);

   addr &= 0x1fffffff;
   if(spi_setup_addr(addr, len) == -1)
   {
      mutex_unlock(&bcmSpiSlaveMutex);
      return(-1);
   }

   data <<= 8 * (4 - len);

   buf[0] = 0x13;
   buf[1] = addr & 0xff;
   buf[2] = data >> 24;
   buf[3] = data >> 16;
   buf[4] = data >> 8;
   buf[5] = data >> 0;
   status = BcmSpiSyncTrans(buf, NULL, 0, 2 + len, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveWrite: BcmSpiSyncTrans returned error\n");
      mutex_unlock(&bcmSpiSlaveMutex);
      return(-1);
   }

   if((spi_read_status(buf) == -1) || (buf[0] & 0x0f))
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveWrite: spi_read_status returned error - %02x\n", buf[0]);
      mutex_unlock(&bcmSpiSlaveMutex);
      return(-1);
   }

   mutex_unlock(&bcmSpiSlaveMutex);

   return(0);
}


static int kerSysBcmSpiSlaveWriteBuf_rev0(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize)
{
   int            status = SPI_STATUS_ERR;
   int            maxSize;
   unsigned char *pWriteData;
   unsigned int  nBytes = 0;
   unsigned int  length = len;
   UNUSED_PARAM(dev);

   maxSize    = BcmSpi_GetMaxRWSize(bcmSpiSlaveBus, 0);
   maxSize   -= 2;
   maxSize   &= ~(unitSize - 1);
   pWriteData = kmalloc(maxSize+2, GFP_KERNEL);
   if ( NULL == pWriteData )
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveWriteBuf: Out of memory\n");
      return(SPI_STATUS_ERR);
   }

   mutex_lock(&bcmSpiSlaveMutex);

   addr &= 0x1fffffff;
   while ( length > 0 )
   {
      if(spi_setup_addr(addr, unitSize) == -1)
      {
         mutex_unlock(&bcmSpiSlaveMutex);
         status = SPI_STATUS_ERR;
         goto out;
      }

      nBytes        = (length > maxSize) ? maxSize : length;
      pWriteData[0] = 0x13;
      pWriteData[1] = addr & 0xff;
      memcpy(&pWriteData[2], data, nBytes);
      status = BcmSpiSyncTrans(&pWriteData[0], NULL, 0, nBytes+2, bcmSpiSlaveBus, bcmSpiSlaveId);
      if ( SPI_STATUS_OK != status )
      {
         printk(KERN_ERR "kerSysBcmSpiSlaveWriteBuf: BcmSpiSyncTrans returned error\n");
         status = SPI_STATUS_ERR;
         goto out;
      }

      if((spi_read_status(pWriteData) == -1) || (pWriteData[0] & 0x0f))
      {
         printk(KERN_ERR "kerSysBcmSpiSlaveWrite: spi_read_status returned error - %02x\n", pWriteData[0]);
         status = SPI_STATUS_ERR;
         goto out;
      }
      addr    = (unsigned int)addr + nBytes;
      data    = (unsigned int *)((uintptr_t)data + nBytes);
      length -= nBytes;
   }

out:
   mutex_unlock(&bcmSpiSlaveMutex);
   kfree(pWriteData);

   return( status );
}

static int kerSysBcmSpiSlaveRead_rev1(int dev, unsigned int addr, unsigned int *data, unsigned int len)
{
   struct spi_transfer xfer[2];
   uint8_t buf_0[3]  = { 0 };
   uint8_t buf_1[20] = { 0 };
   int     status;
   int     i;
   UNUSED_PARAM(dev);

   *data = 0;
   switch ( len )
   {
      /* a read includes up to 10 status bytes,
         if read completes with one status byte slave will
         start reading next address. Disable address auto
         increment to avoid memory faults */
      case 1: buf_0[2] = 0x00 | 0x08; break;
      case 2: buf_0[2] = 0x20 | 0x08; break;
      case 4: buf_0[2] = 0x40 | 0x08; break;
      default: return(SPI_STATUS_INVALID_LEN);
   }

   memset(xfer, 0, (sizeof xfer));

   buf_0[0]         = 0x11;
   buf_0[1]         = 0x03;
   xfer[0].len      = 3;
   xfer[0].speed_hz = bcmSpiSlaveMaxFreq;
   xfer[0].tx_buf   = &buf_0[0];
   xfer[0].cs_change = 1;

   addr            &= 0x1fffffff;
   buf_1[0]         = 0x10;
   buf_1[1]         = 0xC0 | ((addr >>  0) & 0x3f);
   buf_1[2]         = 0x80 | ((addr >>  6) & 0x7f);
   buf_1[3]         = 0x80 | ((addr >> 13) & 0x7f);
   buf_1[4]         = 0x80 | ((addr >> 20) & 0x7f);
   buf_1[5]         = 0x00 | ((addr >> 27) & 0x1f);
   xfer[1].len      = 20; /* 6 cmd bytes, 1-4 data bytes, 10-13 status bytes */
   xfer[1].speed_hz = bcmSpiSlaveMaxFreq;
   xfer[1].tx_buf   = &buf_1[0];
   xfer[1].rx_buf   = &buf_1[0];
   xfer[1].cs_change = 1;

   status = BcmSpiSyncMultTrans(&xfer[0], 2, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveRead: BcmSpiSyncMultTrans returned error\n");
      return(SPI_STATUS_ERR);
   }

   /* there can be up to 10 status bytes starting at index 6 */
   for ( i = 6; i < (20 - len); i++)
   {
      if ( 0x01 & buf_1[i] )
      {
         /* completed successfully */
         switch ( len )
         {
            case 1:
               *data = buf_1[i+1];
               break;
            case 2:
               *data = (buf_1[i+1] << 8) | buf_1[i+2];
               break;
            case 4:
            default:
               *data = (buf_1[i+1] << 24) | (buf_1[i+2] << 16) |
                       (buf_1[i+3] <<  8) | buf_1[i+4];
               break;
         }
         return SPI_STATUS_OK;
      }
      else if ( 0x02 & buf_1[i] )
      {
          buf_0[0]         = 0x10;
          buf_0[1]         = 0x02;
          xfer[0].len      = 3;
          xfer[0].speed_hz = bcmSpiSlaveMaxFreq;
          xfer[0].tx_buf   = &buf_0[0];
          xfer[0].rx_buf   = &buf_0[0];

          BcmSpiSyncMultTrans(&xfer[0], 1, bcmSpiSlaveBus, bcmSpiSlaveId);
          printk(KERN_ERR "kerSysBcmSpiSlaveRead: SPI error: %x\n", buf_0[2] );
          return SPI_STATUS_ERR;
      }
   }

   /* read did not complete - read status register and return error
      note that number of status bytes read should prevent this from happening */
   buf_0[0]         = 0x10;
   buf_0[1]         = 0x02;
   xfer[0].len      = 3;
   xfer[0].speed_hz = bcmSpiSlaveMaxFreq;
   xfer[0].tx_buf   = &buf_0[0];
   xfer[0].rx_buf   = &buf_0[0];
   BcmSpiSyncMultTrans(&xfer[0], 1, bcmSpiSlaveBus, bcmSpiSlaveId);
   printk(KERN_ERR "kerSysBcmSpiSlaveRead: SPI timeout: %x\n", buf_0[2] );

   return( SPI_STATUS_ERR );

}


static int kerSysBcmSpiSlaveWrite_rev1(int dev, unsigned int addr, unsigned int data, unsigned int len)
{
   struct spi_transfer xfer[3];
   int                 status;
   uint8_t buf_0[3]  = { 0 };
   uint8_t buf_1[10] = { 0 };
   uint8_t buf_2[3]  = { 0 };
   UNUSED_PARAM(dev);

   switch ( len )
   {
      case 1: buf_0[2] = 0x00; break;
      case 2: buf_0[2] = 0x20; break;
      case 4: buf_0[2] = 0x40; break;
      default: return(SPI_STATUS_INVALID_LEN);
   }

   memset(xfer, 0, (sizeof xfer));
   buf_0[0]          = 0x11;
   buf_0[1]          = 0x03;
   xfer[0].len       = 3;
   xfer[0].speed_hz  = bcmSpiSlaveMaxFreq;
   xfer[0].tx_buf    = &buf_0[0];
   xfer[0].cs_change = 1;

   addr               &= 0x1fffffff;
   data              <<= 8 * (4 - len);
   buf_1[0]            = 0x11;
   buf_1[1]            = 0xC0 | ((addr >>  0) & 0x3f);
   buf_1[2]            = 0x80 | ((addr >>  6) & 0x7f);
   buf_1[3]            = 0x80 | ((addr >> 13) & 0x7f);
   buf_1[4]            = 0x80 | ((addr >> 20) & 0x7f);
   buf_1[5]            = 0x00 | ((addr >> 27) & 0x1f);
   buf_1[6]            = data >> 24;
   buf_1[7]            = data >> 16;
   buf_1[8]            = data >> 8;
   buf_1[9]            = data >> 0;
   xfer[1].len         = 6 + len;
   xfer[1].speed_hz    = bcmSpiSlaveMaxFreq;
   xfer[1].tx_buf      = &buf_1[0];
   xfer[1].rx_buf      = &buf_1[0];
   xfer[1].cs_change   = 1;
   xfer[1].delay_usecs = 10; /* delay to allow write to complete */

   buf_2[0]          = 0x10;
   buf_2[1]          = 0x02;
   xfer[2].len       = 3;
   xfer[2].speed_hz  = bcmSpiSlaveMaxFreq;
   xfer[2].tx_buf    = &buf_2[0];
   xfer[2].rx_buf    = &buf_2[0];
   xfer[2].cs_change = 1;

   status = BcmSpiSyncMultTrans(&xfer[0], 3, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveWrite: BcmSpiSyncMultTrans returned error\n");
      return(status);
   }

   if ( buf_2[2] != 0 )
   {
      /* transfer timed out or there was an error */
      printk(KERN_ERR "kerSysBcmSpiSlaveWrite: SPI error: %x\n", buf_2[2] );
      return SPI_STATUS_ERR;
   }

   return SPI_STATUS_OK;

}


static int kerSysBcmSpiSlaveWriteBuf_rev1(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize)
{
   struct spi_transfer xfer[8];
   uint8_t             buf_0[3] = { 0 };
   uint8_t             buf_1[3] = { 0 };
   int                 status;
   unsigned int        length   = len;
   unsigned int        nBytes   = 0;
   int                 maxSize;
   unsigned char      *pWriteData;
   UNUSED_PARAM(dev);

   switch ( unitSize )
   {
      case 1: buf_0[2] = 0x00; break;
      case 2: buf_0[2] = 0x20; break;
      case 4: buf_0[2] = 0x40; break;
      default: return(SPI_STATUS_INVALID_LEN);
   }


   maxSize    = BcmSpi_GetMaxRWSize(bcmSpiSlaveBus, 0); // No Autobuffer
   maxSize   -= 6;      /* account for command bytes */
   maxSize   &= ~(unitSize - 1);
   pWriteData = kmalloc(maxSize+6, GFP_KERNEL);
   if ( NULL == pWriteData )
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveWriteBuf: Out of memory\n");
      return(SPI_STATUS_ERR);
   }

   memset(&xfer[0], 0, sizeof(struct spi_transfer)*8);
   addr &= 0x1fffffff;
   while ( length > 0 )
   {
      buf_0[0]          = 0x11;
      buf_0[1]          = 0x03;
      xfer[0].len       = 3;
      xfer[0].speed_hz  = bcmSpiSlaveMaxFreq;
      xfer[0].tx_buf    = &buf_0[0];
      xfer[0].cs_change = 1;

      nBytes              = (length > maxSize) ? maxSize : length;
      pWriteData[0]            = 0x11;
      pWriteData[1]            = 0xC0 | ((addr >>  0) & 0x3f);
      pWriteData[2]            = 0x80 | ((addr >>  6) & 0x7f);
      pWriteData[3]            = 0x80 | ((addr >> 13) & 0x7f);
      pWriteData[4]            = 0x80 | ((addr >> 20) & 0x7f);
      pWriteData[5]            = 0x00 | ((addr >> 27) & 0x1f);
      memcpy(&pWriteData[6], data, nBytes);
      xfer[1].len         = 6 + nBytes;
      xfer[1].speed_hz    = bcmSpiSlaveMaxFreq;
      xfer[1].tx_buf      = pWriteData;
      xfer[1].cs_change   = 1;
      xfer[1].delay_usecs = 30; /* delay to allow write to complete */

      buf_1[0]          = 0x10;
      buf_1[1]          = 0x02;
      xfer[2].len       = 3;
      xfer[2].speed_hz  = bcmSpiSlaveMaxFreq;
      xfer[2].tx_buf    = &buf_1[0];
      xfer[2].rx_buf    = &buf_1[0];
      xfer[2].cs_change = 1;

      status = BcmSpiSyncMultTrans(&xfer[0], 3, bcmSpiSlaveBus, bcmSpiSlaveId);
      if ( SPI_STATUS_OK != status )
      {
         printk(KERN_ERR "kerSysBcmSpiSlaveWriteBuf: BcmSpiSyncMultTrans returned error\n");
         kfree(pWriteData);
         return(status);
      }

      if ( buf_1[2] != 0 )
      {
         /* transfer timed out or there was an error */
         printk(KERN_ERR "kerSysBcmSpiSlaveWriteBuf: SPI error: %x\n", buf_1[2] );
         kfree(pWriteData);
         return SPI_STATUS_ERR;
      }
      addr    = (unsigned int)addr + nBytes;
      data    = (unsigned int *)((uintptr_t)data + nBytes);
      length -= nBytes;
   }

   kfree(pWriteData);
   return SPI_STATUS_OK;
}

typedef enum {
    SPI_WRITE = 0x02,
    SPI_READ = 0x03,
    SPI_FAST_READ = 0x0b
} spi_command_rev2;


// Add spi command and address to a buffer, return prepend count.
// In case of write, prepend count is not used by hardware.
static int rev2_add_command_and_address_to_buffer(uint8_t *buf,
        spi_command_rev2 cmd, unsigned int addr)
{
    buf[0] = cmd;
    buf[1] = addr >> 8;
    buf[2] = addr & 0xff;
    return (cmd == SPI_FAST_READ? 4: 3);
}

static int rev2_is_dev_in_range(int dev)
{
  int is_in_range = (dev >= 0) && (dev < spiInfo.nbSlaves);
  if (!is_in_range)
    printk(KERN_ERR "Entering %s: spi slave device %d is out of valid range\n", __FUNCTION__, dev);
  return is_in_range;
}

static int kerSysBcmSpiSlaveRead_rev2(int dev, unsigned int addr, unsigned int *data, unsigned int len)
{
    uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0};
    int prependcnt;
    int status;

    if (!rev2_is_dev_in_range(dev)) return SPI_STATUS_ERR;

    prependcnt = rev2_add_command_and_address_to_buffer(buf, SPI_READ, addr);

    mutex_lock(&bcmSpiSlaveMutex);
    status = BcmSpiSyncTrans(&buf[0], &buf[0], prependcnt, len+prependcnt, spiInfo.bus, spiInfo.id[dev]);
    mutex_unlock(&bcmSpiSlaveMutex);

    if ( SPI_STATUS_OK != status )
    {
        printk(KERN_ERR "kerSysBcmSpiSlaveRead: BcmSpiSyncTrans returned error\n");
        return SPI_STATUS_ERR;
    }

    *data = (uint32_t) le32_to_cpu(*((uint32 *)buf));

    return SPI_STATUS_OK;
}


// kerSysBcmSpiSlaveWrite((uint32_t)ctrlParms.buf, ctrlParms.offset, ctrlParms.result, ctrlParms.strLen)
static int kerSysBcmSpiSlaveWrite_rev2(int dev, unsigned int addr, unsigned int data, unsigned int len)
{
    UNUSED_PARAM(dev);
    UNUSED_PARAM(addr);
    UNUSED_PARAM(data);
    UNUSED_PARAM(len);
    return SPI_STATUS_ERR;
}


static int kerSysBcmSpiSlaveWriteBuf_rev2(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize)
{
    uint8_t buf[7] = {0, 0, 0, 0, 0, 0, 0};
    int status;
    uint32_t le32_data = cpu_to_le32(*data);
    int prependcnt = rev2_add_command_and_address_to_buffer(buf, SPI_WRITE, addr);

    if (!rev2_is_dev_in_range(dev)) return SPI_STATUS_ERR;

    buf[prependcnt+0] = (uint8_t) ((le32_data >>  0) & 0xff);
    buf[prependcnt+1] = (uint8_t) ((le32_data >>  8) & 0xff);
    buf[prependcnt+2] = (uint8_t) ((le32_data >> 16) & 0xff);
    buf[prependcnt+3] = (uint8_t) ((le32_data >> 24) & 0xff);

    mutex_lock(&bcmSpiSlaveMutex);
    status = BcmSpiSyncTrans(&buf[0], /* txBuf */
                             NULL, /* rxBuf */
                             0, /* prependcnt */
                             len+prependcnt, /* nbytes */
                             spiInfo.bus, /* busNum */
                             spiInfo.id[dev]); /* slaveId */
    mutex_unlock(&bcmSpiSlaveMutex);

    if ( SPI_STATUS_OK != status )
    {
        printk(KERN_ERR "kerSysBcmSpiSlaveWrite: BcmSpiSyncTrans returned error\n");
        return SPI_STATUS_ERR;
    }

    return SPI_STATUS_OK;
}

unsigned int kerSysBcmSpiSlaveReadReg32(int dev, unsigned int addr)
{
   unsigned int data = 0;

   BUG_ON(addr & 3);
   addr &= 0x1fffffff;

   if(kerSysBcmSpiSlaveRead(dev,addr, &data, 4) < 0)
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveReadReg32: can't read %08x\n", (unsigned int)addr);
   }

   return(data);
}

void kerSysBcmSpiSlaveWriteReg32(int dev, unsigned int addr, unsigned int data)
{
   BUG_ON(addr & 3);
   addr &= 0x1fffffff;

   if(kerSysBcmSpiSlaveWrite(dev, addr, data, 4) < 0)
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveWriteReg32: can't write %08x (data %08x)\n", (unsigned int)addr, (unsigned int)data);
   }

}

static void getSpiSlaveDeviceInfo(void)
{
    unsigned short gpio;
    unsigned short slaveId;
    unsigned short slaveBus;
    unsigned short slaveMode;
    unsigned int   ctrlState;
    unsigned int   slaveMaxFreq;
    unsigned short protoRev;

    if ( BpGetSpiSlaveProtoRev(&protoRev) == BP_SUCCESS )
    {
        bcmSpiSlaveProtoRev = protoRev;
        printk(KERN_INFO "%s: bcmSpiSlaveProtoRev = %d\n", __FUNCTION__, bcmSpiSlaveProtoRev);
    }

#if defined(_BCM96838_) || defined(CONFIG_BCM96838) || defined(_BCM96858_) || defined(CONFIG_BCM96858)
    if (bcmSpiSlaveProtoRev == 2)
    {
        XDSL_DISTPOINT_INFO xdslDistpointInfo;
        BpGetXdslDistpointInfo(&xdslDistpointInfo);

        spiInfo.nbSlaves = 0;
        spiInfo.bus = xdslDistpointInfo.spi.busNum;
        spiInfo.maxFreq = xdslDistpointInfo.spi.maxFreq;
        spiInfo.mode = xdslDistpointInfo.spi.mode;
        spiInfo.ctrState = xdslDistpointInfo.spi.ctrlState;
        printk(KERN_INFO "%s: bcmSpiSlaveBus = %d\n", __FUNCTION__, spiInfo.bus);
        printk(KERN_INFO "%s: bcmSpiSlaveMode = %d\n", __FUNCTION__, spiInfo.mode);
        printk(KERN_INFO "%s: bcmSpiSlaveCtrState = 0x%x\n", __FUNCTION__, spiInfo.ctrState);
        printk(KERN_INFO "%s: bcmSpiSlaveMaxFreq = %d\n", __FUNCTION__, spiInfo.maxFreq);

        while ((spiInfo.nbSlaves < xdslDistpointInfo.spi.nbSlaves) &&
               (spiInfo.nbSlaves < BCM_SPI_MAX_NB_SLAVES))
        {
            spiInfo.id[spiInfo.nbSlaves] = xdslDistpointInfo.spi.selectNum[spiInfo.nbSlaves];
            spiInfo.resetGpio[spiInfo.nbSlaves] = xdslDistpointInfo.spi.reset[spiInfo.nbSlaves];
            printk(KERN_INFO "%s: slave%d, bcmSpiSlaveId = %d\n", __FUNCTION__, spiInfo.nbSlaves, spiInfo.id[spiInfo.nbSlaves]);
            printk(KERN_INFO "%s: slave%d, bcmSpiSlaveResetGpio = %d\n", __FUNCTION__, spiInfo.nbSlaves, spiInfo.resetGpio[spiInfo.nbSlaves] & BP_GPIO_NUM_MASK);
            spiInfo.nbSlaves++;
        }
        if (spiInfo.nbSlaves != xdslDistpointInfo.spi.nbSlaves)
        {
            printk(KERN_ERR"%s: request to configure to many spi slaves (%d).\n", __FUNCTION__, xdslDistpointInfo.spi.nbSlaves);
        }
    }
    else
#endif
    {
        if ( BpGetSpiSlaveResetGpio(&gpio) == BP_SUCCESS )
        {
            bcmSpiSlaveResetGpio = gpio;
            printk(KERN_INFO "%s: bcmSpiSlaveResetGpio = %d\n", __FUNCTION__, bcmSpiSlaveResetGpio);
        }

        if ( BpGetSpiSlaveSelectNum(&slaveId) == BP_SUCCESS )
        {
            bcmSpiSlaveId = slaveId;
            printk(KERN_INFO"%s: bcmSpiSlaveId = %d\n", __FUNCTION__, bcmSpiSlaveId);
        }

        if ( BpGetSpiSlaveBusNum(&slaveBus) == BP_SUCCESS )
        {
            bcmSpiSlaveBus = slaveBus;
            printk(KERN_INFO "%s: bcmSpiSlaveBus = %d\n", __FUNCTION__, bcmSpiSlaveBus);
        }

        if ( BpGetSpiSlaveMode(&slaveMode) == BP_SUCCESS )
        {
            bcmSpiSlaveMode = slaveMode;
            printk(KERN_INFO "%s: bcmSpiSlaveMode = %d\n", __FUNCTION__, bcmSpiSlaveMode);
        }

        if ( BpGetSpiSlaveCtrlState(&ctrlState) == BP_SUCCESS )
        {
            bcmSpiSlaveCtrState = ctrlState;
            printk(KERN_INFO "%s: bcmSpiSlaveCtrState = 0x%x\n", __FUNCTION__, bcmSpiSlaveCtrState);
        }

        if ( BpGetSpiSlaveMaxFreq(&slaveMaxFreq) == BP_SUCCESS )
        {
            bcmSpiSlaveMaxFreq = slaveMaxFreq;
            printk(KERN_INFO "%s: bcmSpiSlaveMaxFreq = %d\n", __FUNCTION__, bcmSpiSlaveMaxFreq);
        }
    }
}

static int kerSysBcmSpiSlaveReset_rev0(int dev)
{
    UNUSED_PARAM(dev);

    printk(KERN_ERR "Entering %s: bcmSpiSlaveResetGpio = %d\n", __FUNCTION__, bcmSpiSlaveResetGpio);

    if ( bcmSpiSlaveResetGpio != 0xFF )
    {
        kerSysSetGpioState(bcmSpiSlaveResetGpio,  kGpioInactive);
        mdelay(1);
        kerSysSetGpioState(bcmSpiSlaveResetGpio,  kGpioActive);
        mdelay(350);
    }
    return(SPI_STATUS_OK);
}

int kerSysBcmSpiSlaveInit_rev0(int dev)
{
    unsigned int data;
    int32_t       retVal = 0;
    int           status;
    struct spi_transfer xfer[2];
    UNUSED_PARAM(dev);

    mutex_init(&bcmSpiSlaveMutex);

    status = BcmSpiReserveSlave2(bcmSpiSlaveBus, bcmSpiSlaveId, bcmSpiSlaveMaxFreq, bcmSpiSlaveMode, bcmSpiSlaveCtrState);
    if ( SPI_STATUS_OK != status )
    {
      printk(KERN_ERR "%s: BcmSpiReserveSlave2 returned error %d\n", __FUNCTION__, status);
      return(SPI_STATUS_ERR);
    }

    memset(xfer, 0, (sizeof xfer));
    xfer[0].len         = 3;
    xfer[0].speed_hz    = bcmSpiSlaveMaxFreq;
    xfer[0].tx_buf      = &init_seq_rev0[0];
    xfer[0].cs_change   = 1;
    xfer[0].delay_usecs = 10;

    xfer[1].len         = 8;
    xfer[1].speed_hz    = bcmSpiSlaveMaxFreq;
    xfer[1].tx_buf      = &init_adr_rev0[0];
    xfer[1].cs_change   = 1;
    xfer[1].delay_usecs = 10;

    status = BcmSpiSyncMultTrans(&xfer[0], 2, bcmSpiSlaveBus, bcmSpiSlaveId);
    if ( SPI_STATUS_OK != status )
    {
      printk(KERN_ERR "%s: BcmSpiSyncMultTrans returned error\n", __FUNCTION__);
      return(SPI_STATUS_ERR);
    }

    if ((kerSysBcmSpiSlaveRead(0, 0x10000000, &data, 4) == -1) ||
       (data == 0) || (data == 0xffffffff))
    {
      printk(KERN_ERR "%s: Failed to read the Chip ID: 0x%08x\n", __FUNCTION__, (unsigned int)data);
      return -1;
    }
    else
    {
      printk(KERN_INFO "%s: Chip ID: 0x%08x\n", __FUNCTION__, (unsigned int)data);
    }


    return( retVal );

}

static int kerSysBcmSpiSlaveReset_rev1(int dev)
{
    UNUSED_PARAM(dev);

    printk(KERN_ERR "Entering %s: bcmSpiSlaveResetGpio = %d\n", __FUNCTION__, bcmSpiSlaveResetGpio);

    if ( bcmSpiSlaveResetGpio != 0xFF )
    {
        kerSysSetGpioState(bcmSpiSlaveResetGpio,  kGpioInactive);
        mdelay(1);
        kerSysSetGpioState(bcmSpiSlaveResetGpio,  kGpioActive);
        mdelay(350);
    }

    return(SPI_STATUS_OK);
}

int kerSysBcmSpiSlaveInit_rev1(int dev)
{
   unsigned int data;
   int32_t       retVal = 0;
   int           status;
   struct spi_transfer xfer[3];
   UNUSED_PARAM(dev);

   status = BcmSpiReserveSlave2(bcmSpiSlaveBus, bcmSpiSlaveId, bcmSpiSlaveMaxFreq, bcmSpiSlaveMode, bcmSpiSlaveCtrState);
   if ( SPI_STATUS_OK != status )
   {
       printk(KERN_ERR "%s: BcmSpiSyncMultTrans returned error\n", __FUNCTION__);
       return(SPI_STATUS_ERR);
   }

   memset(xfer, 0, (sizeof xfer));
   xfer[0].len         = 3;
   xfer[0].speed_hz    = bcmSpiSlaveMaxFreq;
   xfer[0].tx_buf      = &init_seq_rev1[0];
   xfer[0].cs_change   = 1;
   xfer[0].delay_usecs = 10;

   xfer[1].len         = 3;
   xfer[1].speed_hz    = bcmSpiSlaveMaxFreq;
   xfer[1].tx_buf      = &init_cfg_rev1[0];
   xfer[1].cs_change   = 1;
   xfer[1].delay_usecs = 10;

   xfer[2].len         = 7;
   xfer[2].speed_hz    = bcmSpiSlaveMaxFreq;
   xfer[2].tx_buf      = &init_adr_rev1[0];
   xfer[2].cs_change   = 1;
   xfer[2].delay_usecs = 10;

   status = BcmSpiSyncMultTrans(&xfer[0], 3, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "%s: BcmSpiSyncMultTrans returned error\n", __FUNCTION__);
      return(SPI_STATUS_ERR);
   }

   if ((kerSysBcmSpiSlaveRead(0, 0x10000000, &data, 4) == -1) ||
       (data == 0) || (data == 0xffffffff))
   {
      printk(KERN_ERR "%s: Failed to read the Chip ID: 0x%08x\n", __FUNCTION__, (unsigned int)data);
      return -1;
   }
   else
   {
      printk(KERN_INFO "%s: Chip ID: 0x%08x\n", __FUNCTION__, (unsigned int)data);
   }


   return( retVal );

}

static int kerSysBcmSpiSlaveReset_rev2(int dev)
{
    if (!rev2_is_dev_in_range(dev)) return SPI_STATUS_ERR;

    printk(KERN_INFO "Entering %s: dev = %d, reset gpio = %d\n", __FUNCTION__, dev, spiInfo.resetGpio[dev] & BP_GPIO_NUM_MASK );

    if ( spiInfo.resetGpio[dev] != 0xFF )
    {
        kerSysSetGpioState(spiInfo.resetGpio[dev], kGpioActive);
        mdelay(100);
        kerSysSetGpioState(spiInfo.resetGpio[dev], kGpioInactive);
        mdelay(100);
    }

    return(SPI_STATUS_OK);
}

int kerSysBcmSpiSlaveInit_rev2(int dev)
{
    int status;

    if (!rev2_is_dev_in_range(dev)) return SPI_STATUS_ERR;

    mutex_init(&bcmSpiSlaveMutex);

    status = BcmSpiReserveSlave2(spiInfo.bus, spiInfo.id[dev], spiInfo.maxFreq, spiInfo.mode, spiInfo.ctrState);
    if ( SPI_STATUS_OK != status )
    {
      printk(KERN_ERR "%s: dev%d BcmSpiReserveSlave2 returned error %d\n", __FUNCTION__, dev, status);
      return(SPI_STATUS_ERR);
    }

    return SPI_STATUS_OK;
}

int kerSysBcmSpiSlaveInit( int dev )
{
    int status;

    getSpiSlaveDeviceInfo();
    status = spiOps[bcmSpiSlaveProtoRev].slaveReset(dev);
    if (SPI_STATUS_OK != status)
    {
      printk(KERN_ERR "%s: dev %d failed to reset, error %d\n", __FUNCTION__, dev, status);
      return(SPI_STATUS_ERR);
    }
    return spiOps[bcmSpiSlaveProtoRev].slaveInit(dev);
}

int kerSysBcmSpiSlaveRead(int dev, unsigned int addr, unsigned int * data, unsigned int len)
{
    return spiOps[bcmSpiSlaveProtoRev].slaveRead(dev, addr, data, len);
}

int kerSysBcmSpiSlaveWrite(int dev, unsigned int addr, unsigned int data, unsigned int len)
{
    return spiOps[bcmSpiSlaveProtoRev].slaveWrite(dev, addr, data, len);
}

int kerSysBcmSpiSlaveWriteBuf(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize)
{
    return spiOps[bcmSpiSlaveProtoRev].slaveWriteBuf(dev, addr, data, len, unitSize);
}


#if 0
void board_Reset6829( void )
{
   unsigned char portInfo6829;
   int           retVal;

   retVal = BpGet6829PortInfo(&portInfo6829);
   if ( (BP_SUCCESS == retVal) && (0 != portInfo6829))
   {
      Bcm6829SpiSlaveWrite32(0xb0000008, 0x1);
      Bcm6829SpiSlaveWrite32(0xb0000008, 0x0);
   }
}
#endif
