/*
    Copyright 2000-2010 Broadcom Corporation

<:label-BRCM:2012:DUAL/GPL:standard

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
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/spi/spi.h>

#include <bcm_map_part.h>
#endif
#include "bcmSpiRes.h"
#include "bcmSpi.h"
#include <boardparms.h>
#include "bcm_hwdefs.h"

extern int BcmLegSpiRead( const unsigned char *pTxBuf, unsigned char *pRxBuf, 
                   int prependcnt, int nbytes, int devId, int freqHz );
extern int BcmLegSpiWrite(const unsigned char * msg_buf, int nbytes, int devId, int freqHz);
extern unsigned int BcmLegSpiGetMaxRWSize( int bAutoXfer );
extern int BcmHsSpiRead( const unsigned char *pTxBuf, unsigned char *pRxBuf, int prependcnt,
                  int nbytes, int devId, int freqHz, int ctrlState );
extern int BcmHsSpiWrite(const unsigned char * msg_buf, int nbytes, int devId, int freqHz, int ctrlState);
extern int BcmHsSpiMultibitRead( struct spi_transfer *xfer, int devId, int ctrlState );
extern unsigned int BcmHsSpiGetMaxRWSize( int bAutoXfer );
extern int BcmHsSpiSetFlashCtrl( int opCode, int addrBytes, int dummyBytes, int busNum,
                          int devId, int clockHz, int multibitEn );
extern int BcmHsSpiSetup(int spiMode, int ctrlState);

#ifdef HS_SPI
unsigned int bcmSpiCtrlState[8] = {SPI_CONTROLLER_STATE_DEFAULT, SPI_CONTROLLER_STATE_DEFAULT,
                                   SPI_CONTROLLER_STATE_DEFAULT, SPI_CONTROLLER_STATE_DEFAULT, 
                                   SPI_CONTROLLER_STATE_DEFAULT, SPI_CONTROLLER_STATE_DEFAULT, 
                                   SPI_CONTROLLER_STATE_DEFAULT, SPI_CONTROLLER_STATE_DEFAULT};
#endif

#ifndef _CFE_
#ifdef SPI
/* the BCM legacy controller supports up to 8 devices */
static struct spi_board_info bcmLegSpiDevInfo[8] =
{
    {
        .modalias      = "bcm_LegSpiDev0",
        .chip_select   = 0,
        .max_speed_hz  = 781000,
        .bus_num       = LEG_SPI_BUS_NUM,
        .mode          = SPI_MODE_3,
    },
    {
        .modalias      = "bcm_LegSpiDev1",
        .chip_select   = 1,
        .max_speed_hz  = 781000,
        .bus_num       = LEG_SPI_BUS_NUM,
        .mode          = SPI_MODE_3,
    },
    {
        .modalias      = "bcm_LegSpiDev2",
        .chip_select   = 2,
        .max_speed_hz  = 781000,
        .bus_num       = LEG_SPI_BUS_NUM,
        .mode          = SPI_MODE_3,
    },
    {
        .modalias      = "bcm_LegSpiDev3",
        .chip_select   = 3,
        .max_speed_hz  = 781000,
        .bus_num       = LEG_SPI_BUS_NUM,
        .mode          = SPI_MODE_3,
    },
    {
        .modalias      = "bcm_LegSpiDev4",
        .chip_select   = 4,
        .max_speed_hz  = 781000,
        .bus_num       = LEG_SPI_BUS_NUM,
        .mode          = SPI_MODE_3,
    },
    {
        .modalias      = "bcm_LegSpiDev5",
        .chip_select   = 5,
        .max_speed_hz  = 781000,
        .bus_num       = LEG_SPI_BUS_NUM,
        .mode          = SPI_MODE_3,
    },
    {
        .modalias      = "bcm_LegSpiDev6",
        .chip_select   = 6,
        .max_speed_hz  = 781000,
        .bus_num       = LEG_SPI_BUS_NUM,
        .mode          = SPI_MODE_3,
    },
    {
        .modalias      = "bcm_LegSpiDev7",
        .chip_select   = 7,
        .max_speed_hz  = 781000,
        .bus_num       = LEG_SPI_BUS_NUM,
        .mode          = SPI_MODE_3,
    },
};

static struct spi_driver bcmLegSpiDevDrv[8] = 
{
    {
        .driver = 
            {
            .name     = "bcm_LegSpiDev0",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
    {
        .driver = 
            {
            .name     = "bcm_LegSpiDev1",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
    {
        .driver = 
            {
            .name     = "bcm_LegSpiDev2",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
    {
        .driver = 
            {
            .name     = "bcm_LegSpiDev3",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
    {
        .driver = 
            {
            .name     = "bcm_LegSpiDev4",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    }, 
    {
        .driver = 
            {
            .name     = "bcm_LegSpiDev5",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
    {
        .driver = 
            {
            .name     = "bcm_LegSpiDev6",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
    {
        .driver = 
            {
            .name     = "bcm_LegSpiDev7",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
};

static struct spi_device * bcmLegSpiDevices[8];
#endif

#ifdef HS_SPI
/* the BCM HS controller supports up to 8 devices */
static struct spi_board_info bcmHSSpiDevInfo[8] =
{
    {
        .modalias        = "bcm_HSSpiDev0",
        .controller_data = (void *)SPI_CONTROLLER_STATE_DEFAULT,
        .chip_select     = 0,
        .max_speed_hz    = 781000,
        .bus_num         = HS_SPI_BUS_NUM,
        .mode            = SPI_MODE_DEFAULT,
    },
    {
        .modalias        = "bcm_HSSpiDev1",
        .controller_data = (void *)SPI_CONTROLLER_STATE_DEFAULT,
        .chip_select     = 1,
        .max_speed_hz    = 781000,
        .bus_num         = HS_SPI_BUS_NUM,
        .mode            = SPI_MODE_DEFAULT,
    },
    {
        .modalias        = "bcm_HSSpiDev2",
        .controller_data = (void *)SPI_CONTROLLER_STATE_DEFAULT,
        .chip_select     = 2,
        .max_speed_hz    = 781000,
        .bus_num         = HS_SPI_BUS_NUM,
        .mode            = SPI_MODE_DEFAULT,
    },
    {
        .modalias        = "bcm_HSSpiDev3",
        .controller_data = (void *)SPI_CONTROLLER_STATE_DEFAULT,
        .chip_select     = 3,
        .max_speed_hz    = 781000,
        .bus_num         = HS_SPI_BUS_NUM,
        .mode            = SPI_MODE_DEFAULT,
    },
    {
        .modalias        = "bcm_HSSpiDev4",
        .controller_data = (void *)SPI_CONTROLLER_STATE_DEFAULT,
        .chip_select     = 4,
        .max_speed_hz    = 781000,
        .bus_num         = HS_SPI_BUS_NUM,
        .mode            = SPI_MODE_DEFAULT,
    },
    {
        .modalias        = "bcm_HSSpiDev5",
        .controller_data = (void *)SPI_CONTROLLER_STATE_DEFAULT,
        .chip_select     = 5,
        .max_speed_hz    = 781000,
        .bus_num         = HS_SPI_BUS_NUM,
        .mode            = SPI_MODE_DEFAULT,
    },
    {
        .modalias        = "bcm_HSSpiDev6",
        .controller_data = (void *)SPI_CONTROLLER_STATE_DEFAULT,
        .chip_select     = 6,
        .max_speed_hz    = 781000,
        .bus_num         = HS_SPI_BUS_NUM,
        .mode            = SPI_MODE_DEFAULT,
    },
    {
        .modalias        = "bcm_HSSpiDev7",
        .controller_data = (void *)SPI_CONTROLLER_STATE_DEFAULT,
        .chip_select     = 7,
        .max_speed_hz    = 781000,
        .bus_num         = HS_SPI_BUS_NUM,
        .mode            = SPI_MODE_DEFAULT,
    },
};

static struct spi_driver bcmHSSpiDevDrv[8] = 
{
    {
        .driver = 
            {
            .name     = "bcm_HSSpiDev0",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
    {
        .driver = 
            {
            .name     = "bcm_HSSpiDev1",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
    {
        .driver = 
            {
            .name     = "bcm_HSSpiDev2",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
    {
        .driver = 
            {
            .name     = "bcm_HSSpiDev3",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
    {
        .driver = 
            {
            .name     = "bcm_HSSpiDev4",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    }, 
    {
        .driver = 
            {
            .name     = "bcm_HSSpiDev5",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
    {
        .driver = 
            {
            .name     = "bcm_HSSpiDev6",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
    {
        .driver = 
            {
            .name     = "bcm_HSSpiDev7",
            .bus      = &spi_bus_type,
            .owner    = THIS_MODULE,
            },
    },
};

static struct spi_device * bcmHSSpiDevices[8];
#endif


int BcmSpiReserveSlave2(int busNum, int slaveId, int maxFreq, int spiMode, int ctrlState)
{
    struct spi_master * pSpiMaster;
    struct spi_driver * pSpiDriver;
    
    if ( slaveId > 7 )
    {
        return SPI_STATUS_ERR;
    }

    if ( LEG_SPI_BUS_NUM == busNum )
    {
#ifndef SPI
        return( SPI_STATUS_ERR );
#else
        if ( NULL != bcmLegSpiDevices[slaveId] )
        {
            printk(KERN_ERR "BcmSpiReserveSlave - slaveId %d, already registerd\n", slaveId);
            return( SPI_STATUS_ERR );
        }

        bcmLegSpiDevInfo[slaveId].max_speed_hz    = maxFreq;
        bcmLegSpiDevInfo[slaveId].controller_data = (void *)ctrlState;
        bcmLegSpiDevInfo[slaveId].mode            = spiMode;
        
        pSpiMaster                = spi_busnum_to_master( busNum );
        bcmLegSpiDevices[slaveId] = spi_new_device(pSpiMaster, &bcmLegSpiDevInfo[slaveId]);
        pSpiDriver                = &bcmLegSpiDevDrv[slaveId];
#endif
    }
    else if ( HS_SPI_BUS_NUM == busNum )
    {
#ifndef HS_SPI
        return( SPI_STATUS_ERR );
#else
        if ( NULL != bcmHSSpiDevices[slaveId] )
        {
            printk(KERN_ERR "BcmSpiReserveSlave - slaveId %d, already registerd\n", slaveId);
            return( SPI_STATUS_ERR );
        }

        bcmHSSpiDevInfo[slaveId].max_speed_hz    = maxFreq;
        bcmHSSpiDevInfo[slaveId].controller_data = (void *)((uintptr_t)ctrlState);
        bcmHSSpiDevInfo[slaveId].mode            = spiMode;
        
        pSpiMaster               = spi_busnum_to_master( busNum );
        bcmHSSpiDevices[slaveId] = spi_new_device(pSpiMaster, &bcmHSSpiDevInfo[slaveId]);
        pSpiDriver               = &bcmHSSpiDevDrv[slaveId];
#endif        
    }
    else
        return( SPI_STATUS_ERR );

    /* register the SPI driver */
    spi_register_driver(pSpiDriver);

    return 0;
    
}
EXPORT_SYMBOL(BcmSpiReserveSlave2);

int BcmSpiReserveSlave(int busNum, int slaveId, int maxFreq)
{
   return( BcmSpiReserveSlave2(busNum, slaveId, maxFreq, SPI_MODE_DEFAULT, SPI_CONTROLLER_STATE_DEFAULT) );
}
EXPORT_SYMBOL(BcmSpiReserveSlave);

int BcmSpiReleaseSlave(int busNum, int slaveId)
{
    if ( slaveId > 7 )
    {
        return SPI_STATUS_ERR;
    }

    if ( LEG_SPI_BUS_NUM == busNum )
    {
#ifndef SPI
        return( SPI_STATUS_ERR );
#else
        if ( NULL == bcmLegSpiDevices[slaveId] )
        {
            printk(KERN_ERR "BcmSpiReleaseSlave - slaveId %d, already released\n", slaveId);
            return( SPI_STATUS_ERR );
        }

        bcmLegSpiDevInfo[slaveId].max_speed_hz = 781000;
        spi_unregister_driver(&bcmLegSpiDevDrv[slaveId]);
        spi_unregister_device(bcmLegSpiDevices[slaveId]);
        bcmLegSpiDevices[slaveId] = 0;
#endif
    }
    else if ( HS_SPI_BUS_NUM == busNum )
    {
#ifndef HS_SPI
        return( SPI_STATUS_ERR );
#else
        if ( NULL == bcmHSSpiDevices[slaveId] )
        {
            printk(KERN_ERR "BcmSpiReleaseSlave - slaveId %d, already released\n", slaveId);
            return( SPI_STATUS_ERR );
        }

        bcmHSSpiDevInfo[slaveId].max_speed_hz = 781000;
        spi_unregister_driver(&bcmHSSpiDevDrv[slaveId]);
        spi_unregister_device(bcmHSSpiDevices[slaveId]);
        bcmHSSpiDevices[slaveId] = 0;
#endif        
    }
    else
        return( SPI_STATUS_ERR );

    return 0;
    
}
EXPORT_SYMBOL(BcmSpiReleaseSlave);


int BcmSpiSyncTrans(unsigned char *txBuf, unsigned char *rxBuf, int prependcnt, int nbytes, int busNum, int slaveId)
{
    struct spi_message  message;
    struct spi_transfer xfer;
    int                 status;
    struct spi_device  *pSpiDevice;
    
    if ( slaveId > 7 )
    {
        printk(KERN_ERR "ERROR BcmSpiSyncTrans: invalid slave id %d\n", slaveId);
        return SPI_STATUS_ERR;
    }

    if ( LEG_SPI_BUS_NUM == busNum )
    {
#ifndef SPI
        return( SPI_STATUS_ERR );
#else
        if ( NULL == bcmLegSpiDevices[slaveId] )
        {
            printk(KERN_ERR "ERROR BcmSpiSyncTrans: device not registered\n");
            return SPI_STATUS_ERR;
        } 
        pSpiDevice = bcmLegSpiDevices[slaveId];
#endif
    }
    else if ( HS_SPI_BUS_NUM == busNum )
    {
#ifndef HS_SPI
        return( SPI_STATUS_ERR );
#else
        if ( NULL == bcmHSSpiDevices[slaveId] )
        {
            printk(KERN_ERR "ERROR BcmSpiSyncTrans: device not registered\n");
            return SPI_STATUS_ERR;
        } 
        pSpiDevice = bcmHSSpiDevices[slaveId];
#endif
    }
    else
        return( SPI_STATUS_ERR );

    spi_message_init(&message);
    memset(&xfer, 0, (sizeof xfer));
    xfer.prepend_cnt = prependcnt;
    xfer.len         = nbytes;
    xfer.speed_hz    = pSpiDevice->max_speed_hz;
    xfer.rx_buf      = rxBuf;
    xfer.tx_buf      = txBuf;

    spi_message_add_tail(&xfer, &message);
#if defined(CONFIG_SPI_BCM63XX_HSSPI)
    status = spi_sync(pSpiDevice, &message);
#else
    /* the controller does not support asynchronous transfer
       when spi_async returns the transfer will be complete
       don't use spi_sync to avoid the call to schedule */
    status = spi_async(pSpiDevice, &message);
#endif
    if (status >= 0)
    {
        status = SPI_STATUS_OK;
    } 
    else
    {
        status = SPI_STATUS_ERR;
    }

    return( status );

}
EXPORT_SYMBOL(BcmSpiSyncTrans);

int BcmSpiSyncMultTrans(struct spi_transfer *pSpiTransfer, int numTransfers, int busNum, int slaveId)
{
    struct spi_message  message;
    int                 status;
    struct spi_device  *pSpiDevice;
    int                 i;

    if ( (slaveId > 7) || (busNum > 1) )
    {
        printk(KERN_ERR "ERROR BcmSpiSyncTrans: invalid slave id (%d) or busNum (%d)\n", slaveId, busNum);
        return SPI_STATUS_ERR;
    }

    if ( LEG_SPI_BUS_NUM == busNum )
    {
#ifndef SPI
        return( SPI_STATUS_ERR );
#else
        if ( NULL == bcmLegSpiDevices[slaveId] )
        {
            printk(KERN_ERR "ERROR BcmSpiSyncTrans: device not registered\n");
            return SPI_STATUS_ERR;
        } 
        pSpiDevice = bcmLegSpiDevices[slaveId];
#endif
    }
    else if ( HS_SPI_BUS_NUM == busNum )
    {
#ifndef HS_SPI
        return( SPI_STATUS_ERR );
#else
        if ( NULL == bcmHSSpiDevices[slaveId] )
        {
            printk(KERN_ERR "ERROR BcmSpiSyncTrans: device not registered\n");
            return SPI_STATUS_ERR;
        } 
        pSpiDevice = bcmHSSpiDevices[slaveId];
#endif
    }
    else
        return( SPI_STATUS_ERR );

    spi_message_init(&message);
    for ( i = 0; i < numTransfers; i++ )
    {
        spi_message_add_tail(&pSpiTransfer[i], &message);
    }

#if defined(CONFIG_SPI_BCM63XX_HSSPI)
    status = spi_sync(pSpiDevice, &message);
#else
    /* the controller does not support asynchronous transfer
       when spi_async returns the transfer will be complete
       don't use spi_sync to avoid the call to schedule */
    status = spi_async(pSpiDevice, &message);
#endif
    if (status >= 0)
    {
        status = SPI_STATUS_OK;
    } 
    else
    {
        status = SPI_STATUS_ERR;
    }

    return( status );

}
EXPORT_SYMBOL(BcmSpiSyncMultTrans);
#endif

int BcmSpi_SetCtrlState( int busNum, int slaveId, int spiMode, int ctrlState )
{
    if ( (slaveId > 7) || (busNum > 1) )
    {
        printk("ERROR BcmSpi_SetCtrlState: invalid slave id (%d) or busNum (%d)\n", slaveId, busNum);
        return SPI_STATUS_ERR;
    }

#ifdef HS_SPI
    bcmSpiCtrlState[slaveId] = BcmHsSpiSetup(spiMode, ctrlState);
#endif
    return SPI_STATUS_OK;
}

int BcmSpi_SetFlashCtrl( int opCode, int addrBytes, int dummyBytes, int busNum, 
                         int devId, int clockHz, int multibitEn )
{
    if ( HS_SPI_BUS_NUM == busNum )
    {
#ifndef HS_SPI
        return SPI_STATUS_ERR;
#else
        BcmHsSpiSetFlashCtrl(opCode, addrBytes, dummyBytes, busNum, devId, 
                             clockHz, multibitEn );
#endif
    }
    else if ( LEG_SPI_BUS_NUM == busNum )
    {
#ifndef SPI
        return SPI_STATUS_ERR;
#endif 
    }
    else
        return SPI_STATUS_ERR;
            
    return SPI_STATUS_OK;
    
}

/* retrieve the maximum size for a transfer on the specified bus
   the controller may have to break down the trasnfer into smaller
   transfers in this case the device driver may need to specify a 
   header and address fields that the controller will prepend to each 
   transfer. If this is not supported by the device bAutoXfer should
   be 0 to get the correct maximum size 
   Note that prior to the initialization of the controller module 
   long reads and long writes are not supported.*/
unsigned int BcmSpi_GetMaxRWSize( int busNum, int bAutoXfer )
{
    unsigned int maxRWSize = 0;

    if ( HS_SPI_BUS_NUM == busNum )
    {
#ifdef HS_SPI
        maxRWSize = BcmHsSpiGetMaxRWSize( bAutoXfer);
#endif
    }
    else if ( LEG_SPI_BUS_NUM == busNum )
    {
#ifdef SPI
        maxRWSize = BcmLegSpiGetMaxRWSize( bAutoXfer );
#endif
    }

    return(maxRWSize);

}


/* The interfaces bcmSpi_Read, BcmSpi_MultibitRead and bcmSpi_Write provide direct
   access to the SPI controller. These interfaces should only be called by CFE and 
   early spi flash code */
int BcmSpi_Read( unsigned char *msg_buf, int prependcnt, int nbytes, int busNum, int devId, int freqHz )
{
    if ( LEG_SPI_BUS_NUM == busNum )
    {
#ifndef SPI
        return SPI_STATUS_ERR;
#else
        return BcmLegSpiRead( msg_buf, msg_buf, prependcnt, nbytes, devId, freqHz );
#endif
    }
    else if ( HS_SPI_BUS_NUM == busNum )
    {
#ifndef HS_SPI
        return SPI_STATUS_ERR;
#else
        return BcmHsSpiRead( msg_buf, msg_buf, prependcnt, nbytes, devId, freqHz,
                             bcmSpiCtrlState[devId] );
#endif
    }
    else
    {
        return SPI_STATUS_ERR;
    }

}

int BcmSpi_Write( const unsigned char *msg_buf, int nbytes, int busNum, int devId, int freqHz )
{
    if ( LEG_SPI_BUS_NUM == busNum )
    {
#ifndef SPI
        return SPI_STATUS_ERR;
#else
        return BcmLegSpiWrite( msg_buf, nbytes, devId, freqHz );
#endif
    }
    else if ( HS_SPI_BUS_NUM == busNum )
    {
#ifndef HS_SPI
        return SPI_STATUS_ERR;
#else
        return BcmHsSpiWrite( msg_buf, nbytes, devId, freqHz, 
                              bcmSpiCtrlState[devId] );
#endif
    }
    else
    {
        return SPI_STATUS_ERR;
    }

}

int BcmSpi_MultibitRead( struct spi_transfer *xfer, int busNum, int devId)
{
    if ( LEG_SPI_BUS_NUM == busNum )
    {
#ifndef SPI
        return SPI_STATUS_ERR;
#else
        if ( xfer->multi_bit_en )
           return SPI_STATUS_ERR;

        return BcmLegSpiRead(xfer->tx_buf, xfer->rx_buf, xfer->prepend_cnt, xfer->len, 
                             devId, xfer->speed_hz);
#endif
    }
    else if ( HS_SPI_BUS_NUM == busNum )
    {
#ifndef HS_SPI
        return SPI_STATUS_ERR;
#else
        return BcmHsSpiMultibitRead( xfer, devId, bcmSpiCtrlState[devId] );
#endif
    }
    else
    {
        return SPI_STATUS_ERR;
    }
    
}

#ifndef _CFE_
EXPORT_SYMBOL(BcmSpi_SetFlashCtrl);
EXPORT_SYMBOL(BcmSpi_GetMaxRWSize);
#endif

