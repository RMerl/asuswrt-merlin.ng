/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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


#ifndef __BCMSPIVOICE_H__
#define __BCMSPIVOICE_H__

#if defined(CONFIG_BCM_SPIDEV_VOICE)

#include <linux/spi/spi.h>

#define SPI_STATUS_OK                (0)
#define SPI_STATUS_INVALID_LEN      (-1)
#define SPI_STATUS_ERR              (-2)

#define LEG_SPI_BUS_NUM  0
#define HS_SPI_BUS_NUM   1

/* used to specify ctrlState for the interface BcmSpiReserveSlave2 
   SPI_CONTROLLER_STATE_SET is used to differentiate a value of 0 which results in
   the controller using default values and the case where CPHA_EXT, GATE_CLK_SSOFF,
   CLK_POLARITY, and ASYNC_CLOCK all need to be 0 */
#define SPI_CONTROLLER_STATE_SET             (1 << 31)
#define SPI_CONTROLLER_STATE_CPHA_EXT        (1 << 30)
#define SPI_CONTROLLER_STATE_GATE_CLK_SSOFF  (1 << 29)
#define SPI_CONTROLLER_STATE_ASYNC_CLOCK     (1 << 28)
#define SPI_CONTROLLER_STATE_MASK            (0xf000000)

#define SPI_MODE_DEFAULT              SPI_MODE_0
#define SPI_CONTROLLER_STATE_DEFAULT  (SPI_CONTROLLER_STATE_GATE_CLK_SSOFF)

int bcm_spi_voice_trans(unsigned char *txBuf, unsigned char *rxBuf, int prependcnt,
	int nbytes, int busNum, int slaveId);
int bcm_spi_voice_reserve_dev_ex(int busNum, int slaveId, int maxFreq, int spiMode,
	 int ctrlStat);
int bcm_spi_voice_reserve_dev(int busNum, int slaveId, int maxFreq);
int bcm_spi_voice_release_dev(int busNum, int slaveId);
int bcm_spi_voice_reset_dev(int busNum, int slaveId, int active);
#else
/* 
 * Temporary compatiable legacy api for the dsl chips that does not support
 * voice config through device. These will be eventually removed when all
 * dsl chips support this
 */
#include <bcmSpiRes.h>
#define bcm_spi_voice_trans           BcmSpiSyncTrans
#define bcm_spi_voice_reserve_dev_ex  BcmSpiReserveSlave2
#define bcm_spi_voice_reserve_dev     BcmSpiReserveSlave
#define bcm_spi_voice_release_dev     BcmSpiReleaseSlave
#define bcm_spi_voice_reset_dev       BcmSpiResetSlave
#endif
#endif
