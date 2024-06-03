/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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
