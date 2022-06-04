/*
    Copyright 2000-2010 Broadcom Corporation

   <:label-BRCM:2012:GPL/GPL:standard
   
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

#ifdef _CFE_ 
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bcm_map_part.h"  
#define  printk  printf
#else
#include <linux/version.h>
#include <generated/autoconf.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>

#include <bcm_map_part.h>
#include <bcm_intr.h>
#endif
#include "bcmSpiRes.h"
#include "bcmSpi.h"

/* if SPI is defined then the legacy SPI controller is available, otherwise do not compile this code */
#ifdef SPI
#define LEG_SPI_STATE_ONE_WIRE          (1 << 31)
#define LEG_SPI_CONTROLLER_STATE_DEF    0

int BcmLegSpiRead( const unsigned char *pTxBuf, unsigned char *pRxBuf, 
                   int prependcnt, int nbytes, int devId, int freqHz );
int BcmLegSpiWrite(const unsigned char * msg_buf, int nbytes, int devId, 
                   int freqHz);
unsigned int BcmLegSpiGetMaxRWSize( int bAutoXfer );

#ifndef _CFE_
static struct bcmspi BcmLegSpi = {
   .lock = __SPIN_LOCK_UNLOCKED(BcmLegSpi.lock),
};
#endif

#define LEG_SPI_CLOCK_DEF          2 /* 781kHz */
#define LEG_SPI_PREPEND_CNT_MAX    7
#define LEG_SPI_MAX_TRANSFER_SIZE  0xFFFFFFFF /* no limit */
unsigned int legSpiMaxRW = sizeof(SPI->spiMsgData);

/* following are the frequency tables for the SPI controllers 
   they are ordered by frequency in descending order with column 
   2 represetning the register value */
#define LEG_SPI_FREQ_TABLE_SIZE  7
int legSpiClockFreq[LEG_SPI_FREQ_TABLE_SIZE][2] = { 
            { 20000000, 0},
            { 12500000, 6},
            {  6250000, 5},
            {  3125000, 4},
            {  1563000, 3},
            {   781000, 2},
            {   391000, 1} };

static int legSpiRead( const unsigned char *pTxBuf, int prependcnt, int nbytes, int devId )
{
    int i;

    SPI->spiMsgCtl = (HALF_DUPLEX_R << SPI_MSG_TYPE_SHIFT) | (nbytes << SPI_BYTE_CNT_SHIFT);

    for (i = 0; i < prependcnt; i++)
    {
        SPI->spiMsgData[i] = pTxBuf[i];
    }

    SPI->spiCmd = (SPI_CMD_START_IMMEDIATE << SPI_CMD_COMMAND_SHIFT | 
                   devId << SPI_CMD_DEVICE_ID_SHIFT | 
                   prependcnt << SPI_CMD_PREPEND_BYTE_CNT_SHIFT |
                   0 << SPI_CMD_ONE_BYTE_SHIFT);    
 
    return SPI_STATUS_OK;
    
}

static int legSpiWrite( const unsigned char *pTxBuf, int nbytes, int devId )
{
    int            i;

    SPI->spiMsgCtl = (HALF_DUPLEX_W << SPI_MSG_TYPE_SHIFT) | (nbytes << SPI_BYTE_CNT_SHIFT);
    for (i = 0; i < nbytes; i++)
    {
        SPI->spiMsgData[i] = pTxBuf[i];
    }

    SPI->spiCmd = (SPI_CMD_START_IMMEDIATE << SPI_CMD_COMMAND_SHIFT | 
                  devId << SPI_CMD_DEVICE_ID_SHIFT | 
                  0 << SPI_CMD_PREPEND_BYTE_CNT_SHIFT |
                  0 << SPI_CMD_ONE_BYTE_SHIFT);    
 
    return SPI_STATUS_OK;
    
}


static int legSpiTransEnd( unsigned char *rxBuf, int nbytes )
{
    int i;
    if ( NULL != rxBuf )
    {        
        for (i = 0; i < nbytes; i++)
        {
            rxBuf[i] = SPI->spiRxDataFifo[i];
        }
    }

    return SPI_STATUS_OK;
    
}

static int legSpiTransPoll(void)
{
    while ( 1 )
    {
        if ( SPI->spiIntStatus & SPI_INTR_CMD_DONE )
        {
            break;
        }
    }

    return SPI_STATUS_OK;
}

static void legSpiClearIntStatus(void)
{
    SPI->spiIntStatus = SPI_INTR_CLEAR_ALL; 
}

static int legSpiSetClock( int clockHz )
{
    int  i;
    int  clock = -1;

    for( i = 0; i < LEG_SPI_FREQ_TABLE_SIZE; i++ )
    {
        /* look for the closest frequency that is less than the frequency passed in */
        if ( legSpiClockFreq[i][0] <= clockHz )
        {
            clock = legSpiClockFreq[i][1];
            break;
        }
    }
    /* if no clock was found set to default */
    if ( -1 == clock )
    {
        clock = LEG_SPI_CLOCK_DEF;
    }
    SPI->spiClkCfg = (SPI->spiClkCfg & ~SPI_CLK_MASK) | clock;

    return SPI_STATUS_OK;
}

/* BcmLegSpiRead and BcmLegSpiWrite are availble for the CFE and spi flash driver only
   the linux kernel spi framework must be used in all other cases */
int BcmLegSpiRead( const unsigned char *pTxBuf, unsigned char *pRxBuf, 
                   int prependcnt, int nbytes, int devId, int freqHz )
{
#ifndef _CFE_
    struct bcmspi *pBcmSpi = &BcmLegSpi;

    spin_lock(&pBcmSpi->lock);
#endif
    legSpiSetClock(freqHz);
    legSpiClearIntStatus();
    legSpiRead(pTxBuf, prependcnt, nbytes, devId);
    legSpiTransPoll();
    legSpiTransEnd(pRxBuf, nbytes);
    legSpiClearIntStatus();
#ifndef _CFE_
    spin_unlock(&pBcmSpi->lock);
#endif

    return( SPI_STATUS_OK );
}

int BcmLegSpiWrite( const unsigned char *msg_buf, int nbytes, int devId, int freqHz )
{
#ifndef _CFE_
    struct bcmspi *pBcmSpi = &BcmLegSpi;

    spin_lock(&pBcmSpi->lock);
#endif
    legSpiSetClock(freqHz);
    legSpiClearIntStatus();
    legSpiWrite(msg_buf, nbytes, devId);
    legSpiTransPoll();
    legSpiClearIntStatus();
#ifndef _CFE_
    spin_unlock(&pBcmSpi->lock);
#endif

    return( SPI_STATUS_OK );
}

unsigned int BcmLegSpiGetMaxRWSize( int bAutoXfer )
{
   /* the transfer length is limited by contrtollers fifo size
      however, the controller driver is capable of breaking a transfer
      down into smaller chunks, appending a header to each chunk and
      increment an address in the header. If the device supports this,
      bAutoXfer will be set to 1 and this call will return an appropriate
      maximum transaction size. If the device does not support this then
      the maximum transaction size is limited by the fifo size */
   if (bAutoXfer)
   {
      return legSpiMaxRW;
   }
   else
   {
      return sizeof(SPI->spiMsgData);
   }
}

#ifndef _CFE_
static int legSpiSetup(struct spi_device *spi)
{
   struct bcmspi *pBcmSpi;
   unsigned int   spiCtrlState = 0;

   pBcmSpi = spi_master_get_devdata(spi->master);

   if (pBcmSpi->stopping)
   {
      return -ESHUTDOWN;
   }

   if ( 0 != (SPI_3WIRE & spi->mode) )
   {
      spiCtrlState |= LEG_SPI_STATE_ONE_WIRE;
   }
   spi_set_ctldata(spi, (void *)spiCtrlState);

   return 0;
}

void legSpiStartXfer(struct bcmspi *pBcmSpi)
{
   struct bcmspi_xferInfo *pSpiXfer;
   unsigned short          msgCtrl;
   unsigned short          dataLen;
   unsigned short          i;
   unsigned short          spiCmd;
   unsigned int            ctrlState;

   while(1)
   {
      ctrlState = (unsigned int)spi_get_ctldata(pBcmSpi->pCurMsg->spi);
      pSpiXfer  = &pBcmSpi->spiXfer[0];

      /* no transfers started or the last transfer finished */
      if (0 == pSpiXfer->remTxLen)
      {
         struct spi_transfer *pXfer;

         pXfer = pSpiXfer->pCurXfer;
         if ( NULL == pXfer ) 
         {
            /* get the first transfer from the message */
            pXfer = list_entry(pBcmSpi->pCurMsg->transfers.next, struct spi_transfer, transfer_list);
         }
         else
         {
            /* get next transfer in the message */
            if (pBcmSpi->pCurMsg->transfers.prev == &pXfer->transfer_list)
            {
               /* no more transfers in the message */
               return;
            }
            pXfer = list_entry(pXfer->transfer_list.next, struct spi_transfer, transfer_list);               
         }

         pSpiXfer->pCurXfer    = pXfer;
         pSpiXfer->rxBuf       = pXfer->rx_buf;
         pSpiXfer->txBuf       = pXfer->tx_buf;
         pSpiXfer->pHdr        = (unsigned char *)pXfer->tx_buf;
         pSpiXfer->delayUsecs  = pXfer->delay_usecs;
         if (pSpiXfer->rxBuf)
         {
            pSpiXfer->remRxLen = pXfer->len;
         }
         pSpiXfer->remTxLen    = pXfer->len;

         if (pXfer->hdr_len)
         {           
            pSpiXfer->prependCnt = pXfer->hdr_len;
            pSpiXfer->addrLen    = pXfer->addr_len;
            pSpiXfer->addrOffset = pXfer->addr_offset;
            pSpiXfer->addr       = 0;

            /* if addrLen is non 0 then the header contains an address and 
               we need to increment after each transfer */
            if (0 != pSpiXfer->addrLen)
            {
               memcpy(&pSpiXfer->header[0], (char *)pXfer->tx_buf, pSpiXfer->prependCnt);
               pSpiXfer->pHdr = &pSpiXfer->header[0];
               for ( i=0; i<pSpiXfer->addrLen; i++ )
               {
                  if (i)
                  {
                     pSpiXfer->addr <<= 8;
                  }
                  pSpiXfer->addr  |= pSpiXfer->pHdr[pSpiXfer->addrOffset+i];
               }

            }
            pSpiXfer->txBuf += pSpiXfer->prependCnt;
         }
         else
         {
            pSpiXfer->prependCnt = pXfer->prepend_cnt;
         }

         if ( pXfer->prepend_cnt )
         {
            pSpiXfer->msgType = (HALF_DUPLEX_R << SPI_MSG_TYPE_SHIFT);
         }
         else
         {
            if ( (NULL != pSpiXfer->rxBuf) && (NULL != pSpiXfer->txBuf) )
            {
               pSpiXfer->msgType = (FULL_DUPLEX_RW<<SPI_MSG_TYPE_SHIFT);
            }
            else if ( NULL != pSpiXfer->rxBuf )
            {
               pSpiXfer->msgType = (HALF_DUPLEX_R<<SPI_MSG_TYPE_SHIFT);
            }
            else
            {
               pSpiXfer->msgType = (HALF_DUPLEX_W<<SPI_MSG_TYPE_SHIFT);               
            }
         }

         pSpiXfer->maxLen = sizeof(SPI->spiMsgData);
         if (pXfer->unit_size)
         {
            if ( pSpiXfer->msgType != (HALF_DUPLEX_R<<SPI_MSG_TYPE_SHIFT) )
            {
               pSpiXfer->maxLen -= pSpiXfer->prependCnt;
            }

            pSpiXfer->maxLen /= pXfer->unit_size;
            pSpiXfer->maxLen *= pXfer->unit_size;
         }
      }

      /* all information required for transfer is in pSpiXfer */

      //printk("len %d, pre %d, tx %p, rx %p, remTx %d, remRx %d, type %x, maxLen = %d, addr %x\n", pSpiXfer->pCurXfer->len,
      //        pSpiXfer->pCurXfer->prepend_cnt, pSpiXfer->pCurXfer->tx_buf, pSpiXfer->pCurXfer->rx_buf ,
      //        pSpiXfer->remTxLen, pSpiXfer->remRxLen, pSpiXfer->msgType, pSpiXfer->maxLen, pSpiXfer->addr);

      /* determine how much data we need to request/transmit
         this does not count the header tx */
      if ( pSpiXfer->remTxLen > pSpiXfer->maxLen )
      {
         dataLen = pSpiXfer->maxLen;
      }
      else
      {
         dataLen = pSpiXfer->remTxLen;
      }

      legSpiSetClock(pSpiXfer->pCurXfer->speed_hz);

      /* clear all interrupts */
      SPI->spiIntStatus = SPI_INTR_CLEAR_ALL;

      /* copy the prepend bytes to the FIFO */
      msgCtrl = pSpiXfer->msgType;
      if (pSpiXfer->msgType == (HALF_DUPLEX_R<<SPI_MSG_TYPE_SHIFT))
      {
         /* copy the prepend bytes to the FIFO */
         msgCtrl |= (dataLen << SPI_BYTE_CNT_SHIFT);
         if ( pSpiXfer->prependCnt )
         {
            for (i = 0; i < pSpiXfer->prependCnt; i++)
            {
               SPI->spiMsgData[i] = pSpiXfer->pHdr[i];
            }
         }
         spiCmd = pSpiXfer->prependCnt << SPI_CMD_PREPEND_BYTE_CNT_SHIFT;
      }
      else
      {
         /* copy the header and TX bytes to the FIFO */
         msgCtrl |= ((dataLen+pSpiXfer->prependCnt)<< SPI_BYTE_CNT_SHIFT);
         for (i = 0; i < pSpiXfer->prependCnt; i++)
         {
            SPI->spiMsgData[i] = pSpiXfer->pHdr[i];
         }
         for (i = 0; i < dataLen; i++)
         {
            SPI->spiMsgData[i+pSpiXfer->prependCnt] = pSpiXfer->txBuf[i];
         }
         spiCmd = 0 << SPI_CMD_PREPEND_BYTE_CNT_SHIFT;
      }
      SPI->spiMsgCtl = msgCtrl;

      spiCmd |= (SPI_CMD_START_IMMEDIATE << SPI_CMD_COMMAND_SHIFT | 
                 pBcmSpi->pCurMsg->spi->chip_select << SPI_CMD_DEVICE_ID_SHIFT | 
                 0 << SPI_CMD_ONE_BYTE_SHIFT);
      if (ctrlState & LEG_SPI_STATE_ONE_WIRE)
      {
         spiCmd |= (1<<SPI_CMD_ONE_WIRE_SHIFT);
      }

      SPI->spiCmd = spiCmd;

      pSpiXfer->txBuf    += dataLen;
      pSpiXfer->remTxLen -= dataLen;

      if ( pSpiXfer->prependCnt )
      {
         /* increment address and update the header */
         if ( pSpiXfer->addrLen )
         {
            pSpiXfer->addr += dataLen;
            for (i=0; i < pSpiXfer->addrLen; i++)
            {
               pSpiXfer->pHdr[pSpiXfer->addrOffset+pSpiXfer->addrLen-i-1] = (pSpiXfer->addr >>(i*8)) & 0xFF;
            }
         }
      }
      break;
   }

   return;
}

int legSpiProcMsg(struct bcmspi *pBcmSpi )
{
   unsigned long flags;
   struct bcmspi_xferInfo *pSpiXfer;
   int                     dataLen;

   /* the following code is written such that any thread can finish
      any transfer. This means the code does not need to disable
      preemption or interrupts while waiting for a transfer to complete. 
      The outer while loop feeds messages into the innner loop */
   while (1)
   {
      spin_lock_irqsave(&pBcmSpi->lock, flags);
      if (NULL == pBcmSpi->pCurMsg)
      {
         /* if list is empty or stop flag is set just return */
         if (list_empty(&pBcmSpi->queue) || pBcmSpi->stopping)
         {
            /* nothing to process or we need to stop */
            spin_unlock_irqrestore(&pBcmSpi->lock, flags);
            break;
         }
         else
         {
            pBcmSpi->pCurMsg = list_entry(pBcmSpi->queue.next, 
                                          struct spi_message, queue);
            memset(&pBcmSpi->spiXfer[0], 0, sizeof(struct bcmspi_xferInfo));

            /* start the next transfer */
            legSpiStartXfer(pBcmSpi); 
         }
      }
      spin_unlock_irqrestore(&pBcmSpi->lock, flags);

      /* process all transfers in the message */
      while(1)
      {
         spin_lock_irqsave(&pBcmSpi->lock, flags);
         if ( NULL == pBcmSpi->pCurMsg )
         {
            spin_unlock_irqrestore(&pBcmSpi->lock, flags);
            /* break out of inner while loop */
            break;
         }

         if ( !(SPI->spiIntStatus & SPI_INTR_CMD_DONE) )
         {
            spin_unlock_irqrestore(&pBcmSpi->lock, flags);
            continue;
         }

         /* copy data if required */
         pSpiXfer = &pBcmSpi->spiXfer[0];
         if ( pSpiXfer->rxBuf )
         {
            if ( pSpiXfer->remRxLen > pSpiXfer->maxLen )
            {
               dataLen = pSpiXfer->maxLen;
            }
            else
            {
               dataLen = pSpiXfer->remRxLen;
            }

            legSpiTransEnd( pSpiXfer->rxBuf, dataLen);
            pSpiXfer->rxBuf    += dataLen;
            pSpiXfer->remRxLen -= dataLen;
         }

         /* clear all interrupts */
         SPI->spiIntStatus = SPI_INTR_CLEAR_ALL;

         /* if all data in the transfer has been transmitted and a delay was specified 
            then delay */
         if ((0==pSpiXfer->remRxLen) && (0==pSpiXfer->remTxLen) && (pSpiXfer->delayUsecs))
         {
            udelay(pSpiXfer->delayUsecs);
         }

         if ((pBcmSpi->pCurMsg->transfers.prev == &pSpiXfer->pCurXfer->transfer_list) &&
             (0==pSpiXfer->remRxLen) && (0==pSpiXfer->remTxLen))
         {
            /* processing the last transfer in the message and all data 
               for the transfer has been pushed to the controller. */
            list_del(&(pBcmSpi->pCurMsg->queue));
            pBcmSpi->pCurMsg->status = SPI_STATUS_OK;
            pBcmSpi->pCurMsg         = NULL;
            spin_unlock_irqrestore(&pBcmSpi->lock, flags);

            /* the message may specify a callback
               the callback will be called when legSpiProcMsg returns
               to ensure that it is called from the original
               callers context */

            /* break out of inner while loop */
            break;
         }

         /* start next transfer */
         legSpiStartXfer(pBcmSpi);
         spin_unlock_irqrestore(&pBcmSpi->lock, flags);
      }
   }

   return SPI_STATUS_OK;
}


int legSpiTransfer(struct spi_device *spi, struct spi_message *msg)
{
   struct bcmspi          *pBcmSpi = &BcmLegSpi;
   struct spi_transfer    *xfer;
   int                     xferCnt;
   int                     bCsChange;
   int                     rejectMsg;
   unsigned long           flags;
   int                     retVal;

   if (unlikely(list_empty(&msg->transfers)))
   {
      return -EINVAL;
   }

   if (pBcmSpi->stopping)
   {
      return -ESHUTDOWN;
   }
  
   xferCnt      = 0;
   bCsChange    = 0;
   rejectMsg    = 0;
   list_for_each_entry(xfer, &msg->transfers, transfer_list)
   {
      if ( rejectMsg )
      {
         return -EINVAL;
      }

      /* check transfer parameters */
      if (!(xfer->tx_buf || xfer->rx_buf))
      {
         return -EINVAL;
      }

      if ( ((xfer->len > sizeof(SPI->spiMsgData)) && 
            (0 == xfer->unit_size)) ||
           (xfer->prepend_cnt > LEG_SPI_PREPEND_CNT_MAX) ||
           (xfer->hdr_len > LEG_SPI_PREPEND_CNT_MAX)
         )
      {
         printk("Invalid length for transfer\n");
         return -EINVAL;
      }
 
      /* check the clock setting - if it is 0 then set to max clock of the device */
      if ( 0 == xfer->speed_hz )
      {
         if ( 0 == spi->max_speed_hz )
         {
             return -EINVAL;
         }
         xfer->speed_hz = spi->max_speed_hz;
      }
 
      xferCnt++;
      bCsChange |= xfer->cs_change;
 
      /* controller does not support keeping the chip select active
         across multiple transfers. if this is not the last transfer
         then reject message if cs_change is not set */
      if ( 0 == xfer->cs_change )
      {
         rejectMsg = 1;
      }
   }
   msg->status        = -EINPROGRESS;
   msg->actual_length = 0;

   /* add the message to the queue */
   spin_lock_irqsave(&pBcmSpi->lock, flags);
   list_add_tail(&msg->queue, &pBcmSpi->queue);
   spin_unlock_irqrestore(&pBcmSpi->lock, flags);

   retVal = legSpiProcMsg(pBcmSpi);
   if ( SPI_STATUS_OK == retVal )
   {
      /* the callback is made here to ensure that it is made from
         the callers context */   
      if (msg->complete)
      {
         msg->complete(msg->context);  
      }
   }
   
   return retVal;

}

static void legSpiCleanup(struct spi_device *spi)
{
    /* would free spi_controller memory here if any was allocated */

}

static int __init legSpiProbe(struct platform_device *pdev)
{
    int                ret;
    struct spi_master *master;
    struct bcmspi     *pBcmSpi;    

    ret = -ENOMEM;
    master = spi_alloc_master(&pdev->dev, 0);
    if (!master)
    {
        goto out_free;
    }

    master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_3WIRE;
    master->bus_num        = pdev->id;
    master->num_chipselect = 8;
    master->setup          = legSpiSetup;
    master->transfer       = legSpiTransfer;
    master->cleanup        = legSpiCleanup;
    platform_set_drvdata(pdev, master);

    spi_master_set_devdata(master, (void *)&BcmLegSpi);
    pBcmSpi = spi_master_get_devdata(master);

    INIT_LIST_HEAD(&pBcmSpi->queue);
    pBcmSpi->pdev           = pdev;
    pBcmSpi->bus_num        = LEG_SPI_BUS_NUM;
    pBcmSpi->num_chipselect = 8;

    /* Initialize the hardware */

    /* register and we are done */
    ret = spi_register_master(master);
    if (ret)
    {
        goto out_free;
    }

    return 0;

out_free:  
    spi_master_put(master);  
    
    return ret;
}


static int __exit legSpiRemove(struct platform_device *pdev)
{
    struct spi_master   *master  = platform_get_drvdata(pdev);
    struct bcmspi       *pBcmSpi = spi_master_get_devdata(master);
    struct spi_message  *msg;

    /* reset the hardware and block queue progress */
    spin_lock(&pBcmSpi->lock);
    pBcmSpi->stopping = 1;

    /* HW shutdown */
    
    spin_unlock(&pBcmSpi->lock);

    /* Terminate remaining queued transfers */
    list_for_each_entry(msg, &pBcmSpi->queue, queue)
    {
        list_del(&msg->queue);
        msg->status = -ESHUTDOWN;
        if ( msg->complete )
        {
           msg->complete(msg->context);
        }
    } 

    spi_unregister_master(master);

    return 0;
}

#define   legSpiSuspend   NULL
#define   legSpiResume    NULL


static struct platform_device bcm_legacyspi_device = {
    .name        = "bcmleg_spi",
    .id          = LEG_SPI_BUS_NUM,
};

static struct platform_driver bcm_legspi_driver = {
    .driver      =
    {
        .name    = "bcmleg_spi",
        .owner   = THIS_MODULE,
    },
    .suspend     = legSpiSuspend,
    .resume      = legSpiResume,
    .remove      = __exit_p(legSpiRemove),
};

int __init legSpiModInit( void )
{
   int retVal;

   platform_device_register(&bcm_legacyspi_device);
   retVal = platform_driver_probe(&bcm_legspi_driver, legSpiProbe);

   /* the linux driver supports header prepend and address auto increment.
      increase the max length to reflect this */
   legSpiMaxRW = LEG_SPI_MAX_TRANSFER_SIZE;
   
   return retVal;

}
subsys_initcall(legSpiModInit);
#endif

#endif /* SPI */

