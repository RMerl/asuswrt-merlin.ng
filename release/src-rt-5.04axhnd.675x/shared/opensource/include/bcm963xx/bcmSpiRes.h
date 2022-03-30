/*
    Copyright (c) 2020 Broadcom Corporation
    All Rights Reserved

    <:label-BRCM:2020:DUAL/GPL:standard
    
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

#ifndef __BCMSPIRES_H__
#define __BCMSPIRES_H__ 

#ifndef __KERNEL__
struct spi_transfer {
   const void   *tx_buf;
   void         *rx_buf;
   unsigned      len;
   unsigned int  speed_hz;
   unsigned char prepend_cnt;
   unsigned char multi_bit_en;
   unsigned char multi_bit_start_offset;
   unsigned char hdr_len;
   unsigned char unit_size;
   unsigned char addr_len;
   unsigned char addr_offset;
};
#else
#include <linux/spi/spi.h> 
#endif
/* used to specify ctrlState for the interface BcmSpiReserveSlave2 
   SPI_CONTROLLER_STATE_SET is used to differentiate a value of 0 which results in
   the controller using default values and the case where CPHA_EXT, GATE_CLK_SSOFF,
   CLK_POLARITY, and ASYNC_CLOCK all need to be 0 */
#define SPI_CONTROLLER_STATE_SET             (1 << 31)
#define SPI_CONTROLLER_STATE_CPHA_EXT        (1 << 30)
#define SPI_CONTROLLER_STATE_GATE_CLK_SSOFF  (1 << 29)
#define SPI_CONTROLLER_STATE_ASYNC_CLOCK     (1 << 28)
#define SPI_CONTROLLER_STATE_MASK            (0xf0000000)

/*
   SPI MODE sets the values for CPOL and CPHA
   SPI_CONTROLLER_STATE_CPHA_EXT will extend these modes
CPOL = 0 -> base value of clock is 0
CPHA = 0, CPHA_EXT = 0 -> latch data on rising edge, launch data on falling edge
CPHA = 1, CPHA_EXT = 0 -> latch data on falling edge, launch data on rising edge
CPHA = 0, CPHA_EXT = 1 -> latch data on rising edge, launch data on rising edge
CPHA = 1, CPHA_EXT = 1 -> latch data on falling edge, launch data on falling edge

CPOL = 1 -> base value of clock is 1
CPHA = 0, CPHA_EXT = 0 -> latch data on falling edge, launch data on rising edge
CPHA = 1, CPHA_EXT = 0 -> latch data on rising edge, launch data on falling edge
CPHA = 0, CPHA_EXT = 1 -> latch data on falling edge, launch data on falling edge
CPHA = 1, CPHA_EXT = 1 -> latch data on rising edge, launch data on rising edge
*/

#ifndef SPI_CPHA
#define SPI_CPHA   0x1
#endif
#ifndef SPI_CPOL
#define SPI_CPOL   0x2
#endif
#ifndef SPI_MODE_0
#define SPI_MODE_0 (0)
#endif
#ifndef SPI_MODE_1
#define SPI_MODE_1 (SPI_CPHA)
#endif
#ifndef SPI_MODE_2
#define SPI_MODE_2 (SPI_CPOL)
#endif
#ifndef SPI_MODE_3
#define SPI_MODE_3 (SPI_CPOL|SPI_CPHA)
#endif

#define SPI_CONTROLLER_MAX_SYNC_CLOCK 30000000

/* set mode and controller state based on CHIP defaults 
   these values do not apply to the legacy controller
   legacy controller uses SPI_MODE_3 and clock is not
   gated */

#define SPI_MODE_DEFAULT              SPI_MODE_0
#define SPI_CONTROLLER_STATE_DEFAULT  (SPI_CONTROLLER_STATE_GATE_CLK_SSOFF)  

int BcmSpiReserveSlave(int busNum, int slaveId, int maxFreq);
int BcmSpiReserveSlave2(int busNum, int slaveId, int maxFreq, int mode, int ctrlState);
int BcmSpiReleaseSlave(int busNum, int slaveId);
int BcmSpiSyncTrans(unsigned char *txBuf, unsigned char *rxBuf, int prependcnt, int nbytes, int busNum, int slaveId);
int BcmSpiSyncMultTrans(struct spi_transfer *pSpiTransfer, int numTransfers, int busNum, int slaveId);

#define SPI_STATUS_OK                (0)
#define SPI_STATUS_INVALID_LEN      (-1)
#define SPI_STATUS_ERR              (-2)

/* legacy and HS controllers can coexist - use bus num to differentiate */
#define LEG_SPI_BUS_NUM  0
#define HS_SPI_BUS_NUM   1

int BcmSpi_SetFlashCtrl( int opCode, int addrBytes, int dummyBytes, int busNum, int devId, int clockHz, int multibitEn );
unsigned int BcmSpi_GetMaxRWSize( int busNum, int bAutoXfer);
int BcmSpi_Read( unsigned char *msg_buf, int prependcnt, int nbytes, int busNum, int devId, int freqHz );
int BcmSpi_Write( const unsigned char *msg_buf, int nbytes, int busNum, int devId, int freqHz );
int BcmSpi_MultibitRead(struct spi_transfer * xfer, int busNum, int devId);
int BcmSpi_SetCtrlState(int busNum, int slaveId, int spiMode, int ctrlState);

#endif /* __BCMSPIRES_H__ */

