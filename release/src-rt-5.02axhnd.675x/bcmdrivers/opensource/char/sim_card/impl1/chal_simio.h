/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
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

/**
*
* @file  chal_simio.h
*
* @brief SIMIO cHAL interface
*
* @note
*****************************************************************************/

#ifndef _CHAL_SIMIO_H_
#define _CHAL_SIMIO_H_

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup cHAL_Interface 
 * @{
 */


/**
* Supported SIMIO speeds
*****************************************************************************/
typedef enum{
    CHAL_SIMIO_SPD_372_1 = 0,      ///< F=373 D=1
    CHAL_SIMIO_SPD_512_8,          ///< F=512 D=8
    CHAL_SIMIO_SPD_512_16,         ///< F=512 D=16
    CHAL_SIMIO_SPD_512_32,         ///< F=512 D=32
    CHAL_SIMIO_SPD_512_64,         ///< F=512 D=64
    CHAL_SIMIO_SPD_1024_8,         ///< F=1024 D=8
    CHAL_SIMIO_SPD_1024_16,        ///< F=1024 D=16
    CHAL_SIMIO_SPD_1024_32,        ///< F=1024 D=32
    CHAL_SIMIO_SPD_1024_64,        ///< F=1024 D=64
    CHAL_SIMIO_SPD_2048_8,         ///< F=2048 D=8
    CHAL_SIMIO_SPD_2048_16,        ///< F=2048 D=16
    CHAL_SIMIO_SPD_2048_32,        ///< F=2048 D=32
    CHAL_SIMIO_SPD_2048_64,        ///< F=2048 D=64
    CHAL_SIMIO_SPD_LAST,
} CHAL_SIMIO_SPEED_t;


/**
* Supported SIMIO presence signal debounce time
*****************************************************************************/
typedef enum{
    CHAL_SIMIO_DEBOUNCE_TIME_32  = 0x0,      ///< 32us
    CHAL_SIMIO_DEBOUNCE_TIME_64  = 0x1,      ///< 64us
    CHAL_SIMIO_DEBOUNCE_TIME_128 = 0x2,      ///< 128us
    CHAL_SIMIO_DEBOUNCE_TIME_192 = 0x3,      ///< 192us
    CHAL_SIMIO_DEBOUNCE_TIME_256 = 0x4,      ///< 256us
    CHAL_SIMIO_DEBOUNCE_TIME_320 = 0x5,      ///< 320us
    CHAL_SIMIO_DEBOUNCE_TIME_384 = 0x6,      ///< 384us
    CHAL_SIMIO_DEBOUNCE_TIME_448 = 0x7,      ///< 448us
    CHAL_SIMIO_DEBOUNCE_TIME_512 = 0x8,      ///< 512us
    CHAL_SIMIO_DEBOUNCE_TIME_585 = 0x9,      ///< 585us
    CHAL_SIMIO_DEBOUNCE_TIME_640 = 0xA,      ///< 640us
    CHAL_SIMIO_DEBOUNCE_TIME_704 = 0xB,      ///< 704us
    CHAL_SIMIO_DEBOUNCE_TIME_768 = 0xC,      ///< 768us
    CHAL_SIMIO_DEBOUNCE_TIME_832 = 0xD,      ///< 832us
    CHAL_SIMIO_DEBOUNCE_TIME_896 = 0xE,      ///< 896us
    CHAL_SIMIO_DEBOUNCE_TIME_960 = 0xF,      ///< 960us
} CHAL_SIMIO_DEBOUNCE_TIME_t;


/**
* Supported SIMIO presence signal debounce mode
*****************************************************************************/
typedef enum{
    CHAL_SIMIO_DEBOUNCE_MODE_PULSEEXT = 0x0, ///< pulse extender
    CHAL_SIMIO_DEBOUNCE_MODE_1USPULSE = 0x1, ///< 1 us pulse filter
    CHAL_SIMIO_DEBOUNCE_MODE_NONE     = 0x3, ///< none of above
} CHAL_SIMIO_DEBOUNCE_MODE_t;


/**
* Supported SIMIO presence signal level
*****************************************************************************/
typedef enum{
    CHAL_SIMIO_PRESENCE_HIGH = 0x0,          ///< active high
    CHAL_SIMIO_PRESENCE_LOW  = 0x1,          ///< active low
} CHAL_SIMIO_PRESENCE_t;

/**
* Supported SIMIO interrupts
*****************************************************************************/

#define CHAL_SIMIO_INT_RX_ABORT         0x00000800
#define CHAL_SIMIO_INT_RX_REPEAT        0x00000400
#define CHAL_SIMIO_INT_TXIDLE           0x00000200
#define CHAL_SIMIO_INT_GCNTI            0x00000100
#define CHAL_SIMIO_INT_TXTHRE           0x00000080
#define CHAL_SIMIO_INT_RXTOUT           0x00000040
#define CHAL_SIMIO_INT_RXTHRE           0x00000020
#define CHAL_SIMIO_INT_ROVF             0x00000010
#define CHAL_SIMIO_INT_RDR              0x00000008
#define CHAL_SIMIO_INT_TXDONE           0x00000004
#define CHAL_SIMIO_INT_TERR             0x00000002
#define CHAL_SIMIO_INT_PERR             0x00000001
#define CHAL_SIMIO_INT_ALL              (CHAL_SIMIO_INT_RX_ABORT \
                                       | CHAL_SIMIO_INT_RX_REPEAT \
                                       | CHAL_SIMIO_INT_TXIDLE \
                                       | CHAL_SIMIO_INT_GCNTI  | CHAL_SIMIO_INT_TXTHRE \
                                       | CHAL_SIMIO_INT_RXTOUT | CHAL_SIMIO_INT_RXTHRE \
                                       | CHAL_SIMIO_INT_ROVF | CHAL_SIMIO_INT_RDR \
                                       | CHAL_SIMIO_INT_TXDONE | CHAL_SIMIO_INT_PERR)

/**
* Supported SIMIO ESD and detection interrupts
*****************************************************************************/
#define CHAL_SIMIO_INT_WATDOG_ESD_ISR    0x00000080
#define CHAL_SIMIO_INT_BATRM_ESD_ISR     0x00000020
#define CHAL_SIMIO_INT_CARD_OUT_ESD_ISR  0x00000004
#define CHAL_SIMIO_INT_CARD_OUT_ISR      0x00000002
#define CHAL_SIMIO_INT_CARD_IN_ISR       0x00000001
#define CHAL_SIMIO_INT_ESD_DET_ALL       (CHAL_SIMIO_INT_WATDOG_ESD_ISR \
                                        | CHAL_SIMIO_INT_BATRM_ESD_ISR \
                                        | CHAL_SIMIO_INT_CARD_OUT_ESD_ISR \
                                        | CHAL_SIMIO_INT_CARD_OUT_ISR \
                                        | CHAL_SIMIO_INT_CARD_IN_ISR)

#define CHAL_SIMIO_INT_CARD_OUT_IER      0x00020000
#define CHAL_SIMIO_INT_CARD_IN_IER       0x00010000
/**
*
*  @brief  Initialize CHAL SIMIO for the passed SIMIO instance
*
*  @param  baseAddr  (in) mapped address of this SIMIO instance
*
*  @return CHAL handle for this SIMIO instance
*****************************************************************************/
CHAL_HANDLE chal_simio_init(cUInt32 baseAddr);


/**
*
*  @brief  De-Initialize CHAL SIMI for the passed SIMI instance
*
*  @param  handle  (in) this SIMI instance handle
*
*  @return none
*****************************************************************************/
cVoid chal_simio_deinit(CHAL_HANDLE handle);


/**
*
*  @brief  Start SIMI device
*
*  @param  handle  (in) this SIMI instance handle
*
*  @return none
*****************************************************************************/
cVoid chal_simio_start(CHAL_HANDLE handle, Boolean warmReset);


/**
*
*  @brief  Stop SIMI device
*
*  @param  handle  (in) this SIMI instance handle
*  @param  boolean (in) option to leave the SIM Clock ON or turn
*                  it OFF
*
*  @return none
*****************************************************************************/
cVoid chal_simio_stop(CHAL_HANDLE handle, Boolean clk_on);


/**
*
*  @brief  Set odd/even parity check
*
*  @param  handle  (in) this SIMI instance handle
*          odd     (in) TRUE for odd parity
*          tx_off  (in) TRUE for turning off TX parity check
*          rx_off  (in) TRUE for turning off RX parity check
*
*  @return none
*****************************************************************************/
cVoid chal_simio_set_parity(
    CHAL_HANDLE handle, 
    cBool odd, 
    cBool tx_off, 
    cBool rx_off
);


/**
*
*  @brief  turn on/off SIMI device VPP
*
*  @param  handle  (in) this SIMI instance handle
*          on  (in) TRUE for on
*
*  @return none
*****************************************************************************/
cVoid chal_simio_enable_vpp(CHAL_HANDLE handle, cBool on);


/**
*
*  @brief  Soft Reset ESD 
*
*  @param  handle  (in) this SIMI instance handle
*
*  @return none
*****************************************************************************/
cVoid chal_simio_soft_reset_esd(CHAL_HANDLE handle);

/**
*
*  @brief  Turn on/off SIM emergency shutdown
*
*  @param  handle     (in) this SIMIO instance handle
*  @param  on         (in) TRUE if turn on emergency shutdown clock
*  @param  batrm_on   (in) TRUE if turn on emergency shutdown from battery 
*                          removal
*  @param  cardout_on (in) TRUE if turn on emergency shutdown from card out
*  @param  reset_on   (in) TRUE if turn on emergency shutdown from 
*                          watchdog/system reset
*
*****************************************************************************/
cVoid chal_simio_enable_eshutdown(
    CHAL_HANDLE handle, 
    cBool on,
    cBool batrm_on,
    cBool cardout_on,
    cBool reset_on
);


/**
*
*  @brief  turn on/off SIMI DMA
*
*  @param  handle  (in) this SIMI instance handle
*          on  (in) TRUE to turn on the DMA feature
*
*  @return none
*****************************************************************************/
cVoid chal_simio_enable_dma(CHAL_HANDLE handle, cBool on);


/**
*
*  @brief  turn on/off SIMI mask off data in feature
*
*  @param  handle  (in) this SIMI instance handle
*          on  (in) TRUE to turn on the feature
*
*  @return none
*****************************************************************************/
cVoid chal_simio_mask_data_in(CHAL_HANDLE handle, cBool on);


/**
*
*  @brief  turn on/off SIMI device clock
*
*  @param  handle  (in) this SIMI instance handle
*          on  (in) TRUE to turn on the clock
*
*  @return none
*****************************************************************************/
cVoid chal_simio_enable_clock(CHAL_HANDLE handle, cBool on);


/**
*
*  @brief  check SIMI device clock on/off state
*
*  @param  handle  (in) this SIMI instance handle
*
*  @return TRUE of clock is stopped
*****************************************************************************/
cBool chal_simio_is_clock_off(CHAL_HANDLE handle);


/**
*
*  @brief  set SIMI device clock stop level
*
*  @param  handle  (in) this SIMI instance handle
*          high  (in) TRUE to set stop level high
*
*  @return none
*****************************************************************************/
cVoid chal_simio_set_clockstop_level(CHAL_HANDLE handle, cBool high);


/**
*
*  @brief  Get SIMI device clock stop level
*
*  @param  handle  (in) this SIMI instance handle
*
*  @return TRUE if clock stop level is high
*****************************************************************************/
cBool chal_simio_get_clockstop_level(CHAL_HANDLE handle);


/**
*
*  @brief  set SIMI device reset level
*
*  @param  handle  (in) this SIMI instance handle
*          high  (in) TRUE to set reset line level high
*
*  @return none
*****************************************************************************/
cVoid chal_simio_set_reset_level(CHAL_HANDLE handle, cBool high);


/**
*
*  @brief  Get SIMI device reset level
*
*  @param  handle  (in) this SIMI instance handle
*
*  @return TRUE if reset line level is high
*****************************************************************************/
cBool chal_simio_get_reset_level(CHAL_HANDLE handle);


/**
*
*  @brief  Set SIMIO speed
*
*  @param  handle  (in) this SIMI instance handle
*  @param  speed   (in) speed to be set
*
*  @return none
*****************************************************************************/
cVoid chal_simio_set_speed(CHAL_HANDLE handle, CHAL_SIMIO_SPEED_t speed);


/**
*
*  @brief  Set SIMIO protocol
*
*  @param  handle  (in) this SIMI instance handle
*  @param  t1      (in) true for t1, false for t0
*
*  @return none
*****************************************************************************/
cVoid chal_simio_set_protocol(CHAL_HANDLE handle, cBool t1);


/**
*
*  @brief  Read/Clear SIMIO interrupt status 
*
*  @param  handle      (in) this SIMI instance handle
*
*  @return status mask 
*
*  @note status is cleared after read
*****************************************************************************/
cUInt32 chal_simio_read_intr_status(CHAL_HANDLE handle);


/**
*
*  @brief  Enable SIMIO interrupts
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  mask     (in) interrupts to enable
*
*  @return none
*****************************************************************************/
cVoid chal_simio_enable_intr(CHAL_HANDLE handle, cUInt32 mask);


/**
*
*  @brief  Disable SIMIO interrupts
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  mask     (in) interrupts to disable
*
*  @return none
*****************************************************************************/
cVoid chal_simio_disable_intr(CHAL_HANDLE handle, cUInt32 mask);


/**
*
*  @brief  Enable SIMIO generic compare counter
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  val     (in) val to compare
*
*  @return none
*****************************************************************************/
cVoid chal_simio_enable_counter(CHAL_HANDLE handle, cUInt16 val);


/**
*
*  @brief  Disable SIMIO generic compare counter
*
*  @param  handle   (in) this SIMIO instance handle
*
*  @return none
*****************************************************************************/
cVoid chal_simio_disable_counter(CHAL_HANDLE handle);

/**
*
*  @brief  Set extra guard time
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  time (in) new guard time
*
*  @return None
*****************************************************************************/
cVoid chal_simio_set_extra_guard_time(CHAL_HANDLE handle, cUInt16 time);

/**
*
*  @brief  Set SIMIO TX FIFO threshold
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  threshold (in) new threshold
*
*  @return TRUE if new threshold is accepted
*****************************************************************************/
cBool chal_simio_set_tx_threshold(CHAL_HANDLE handle, cUInt8 threshold);


/**
*
*  @brief  Set SIMIO RX FIFO threshold
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  threshold (in) new threshold
*
*  @return TRUE if new threshold is accepted
*****************************************************************************/
cBool chal_simio_set_rx_threshold(CHAL_HANDLE handle, cUInt8 threshold);

/**
*
*  @brief  Set SIMIO RX timout
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  prescale (in) prescale to generate 1MHz clock
*  @param  divisor  (in) divisor to generate tick from 1MHz clock
*  @param  timeout  (in) timeout counter
*
*  @return TRUE if new timeout is accepted
*****************************************************************************/
cBool chal_simio_set_rx_timeout(
    CHAL_HANDLE handle, 
    cUInt8 prescale,
    cUInt8 divisor,
    cUInt16 timeout);


/**
*
*  @brief  Set tx retry times
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  retry   (in) retry times
*
*  @return TRUE if new threshold is accepted
*****************************************************************************/
cBool chal_simio_set_tx_retry(CHAL_HANDLE handle, cUInt8 retry);


/**
*
*  @brief  Set rx try times
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  retry   (in) retry times
*
*  @return TRUE if new threshold is accepted
*****************************************************************************/
cBool chal_simio_set_rx_retry(CHAL_HANDLE handle, cUInt8 retry);

/**
*
*  @brief  enable FIFO
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  on   (in) TRUE to enable
*
*  @return None
*****************************************************************************/
cVoid chal_simio_enable_fifo(CHAL_HANDLE handle, cBool on);


/**
*
*  @brief  Flush FIFO
*
*  @param  handle   (in) this SIMIO instance handle
*
*  @return None
*****************************************************************************/
cVoid chal_simio_flush_fifo(CHAL_HANDLE handle);


/**
*
*  @brief  Get TX/RX FIFO size
*
*  @param  handle   (in) this SIMIO instance handle
*
*  @return TX-remaining bytes, RX-received bytes
*****************************************************************************/
cUInt32 chal_simio_get_fifo_size(CHAL_HANDLE handle);


/**
*
*  @brief  Read raw data from SIMIO RX FIFO
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  pBuffer  (i/o) Buffer for data read with parity flag
*  @param  size     (in) Buffer size
*
*  @return Number of bytes read
*****************************************************************************/
cUInt32 chal_simio_read_data(CHAL_HANDLE handle, cUInt16 *pBuffer, cUInt32 size);


/**
*
*  @brief  Write data to SIMIO TX FIFO. Notes: the calling function needs to 
*          guarantee not to send more than 5 bytes if the data begins with the
*          command header due to NULL byte procedure in T = 0 protocol
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  pBuffer  (in) Buffer to write data from
*  @param  size     (in) Buffer size
*
*  @return Number of bytes written
*****************************************************************************/
cUInt32 chal_simio_write_data(CHAL_HANDLE handle, cUInt8 *pBuffer, cUInt32 size);


/**
*
*  @brief  Read SIMIO SCR, SIER and SSR register values from ASIC for debugging purpose.
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  scrReg   (out) SIMIO SCR register value
*  @param  sierReg   (out) SIMIO SIER register value
*  @param  ssrReg   (out) SIMIO SSR register value
*  @param  sfcrReg   (out) SIMIO SFCR register value
*  @param  sdebugReg   (out) SIMIO SIMDEBUG register value
*
*****************************************************************************/
cVoid chal_simio_read_reg(CHAL_HANDLE handle, cUInt32* scrReg, cUInt32* sierReg, cUInt32* ssrReg,
						  cUInt32* sfcrReg, cUInt32* sdebugReg);


/**
*
*  @brief  Turn on/off clock divisor
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  on       (in) TRUE if turn 
*  @param  divisor  (in) 8-bit divisor
*
*****************************************************************************/
cVoid chal_simio_set_divisor(CHAL_HANDLE handle, cBool on, cUInt8 divisor);


/**
*
*  @brief  Turn on/off extra two samples on input SIMDAT signal
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  on       (in) TRUE if turn 
*  @param  divisor  (in) 9-bit extra sample offset
*
*****************************************************************************/
cVoid chal_simio_set_extra_sample(CHAL_HANDLE handle, cBool on, cUInt16 divisor);


/**
*
*  @brief  Turn on/off TXENDQUICK
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  on       (in) TRUE if turn on
*
*****************************************************************************/
cVoid chal_simio_set_txendquick(CHAL_HANDLE handle, cBool on);


/**
*
*  @brief  Turn on/off SIM presence detection
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  on       (in) TRUE if turn on
*  @param  time     (in) debounce time
*  @param  mode     (in) debounce mode
*  @param  presence (in) presence type
*
*****************************************************************************/
cVoid chal_simio_set_detection(
    CHAL_HANDLE handle, 
    cBool on,
    CHAL_SIMIO_DEBOUNCE_TIME_t debouce_time,
    CHAL_SIMIO_DEBOUNCE_MODE_t debounce_mode,
    CHAL_SIMIO_PRESENCE_t presence_type
);


/**
*
*  @brief  Read/Clear SIMIO ESD and detection interrupt status 
*
*  @param  handle      (in) this SIMI instance handle
*
*  @return status mask 
*
*  @note status is cleared after read
*****************************************************************************/
cUInt32 chal_simio_read_esd_det_intr_status(CHAL_HANDLE handle);


/**
*
*  @brief  Enable SIMIO ESD and DET interrupts
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  mask     (in) interrupts to enable
*
*  @return none
*****************************************************************************/
cVoid chal_simio_enable_esd_det_intr(CHAL_HANDLE handle, cUInt32 mask);


/**
*
*  @brief  Read SIM detection status 
*
*  @param  handle      (in) this SIMI instance handle
*
*  @return status mask 
*
*  @note 
*****************************************************************************/
cUInt32 chal_simio_read_detection_status(CHAL_HANDLE handle);


/**
*
*  @brief  Set order/sense based on the card encoding
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  inverse  (in) TRUE for inverse card, otherwise for direct card
*
*  @return 
*****************************************************************************/
cVoid chal_simio_set_order_sense(CHAL_HANDLE handle, cBool inverse);


/**
*
*  @brief  Soft reset controller
*
*  @param  handle   (in) this SIMIO instance handle
*
*  @return 
*****************************************************************************/
cVoid chal_simio_soft_reset(CHAL_HANDLE handle);


/**
*
*  @brief  Turn on/off SIMVCC driven by SIMLDO
*
*  @param  handle     (in) this SIMIO instance handle
*  @param  on         (in) TRUE if turn on SIMVCC driven by SIMLDO
*
*****************************************************************************/
cVoid chal_simio_simldo_simvcc(
    CHAL_HANDLE handle, 
    cBool on
);


/**
*
*  @brief  Select SIMVCC voltage
*
*  @param  handle     (in) this SIMIO instance handle
*  @param  on         (in) TRUE if select 3V SIMVCC
*
*****************************************************************************/
cVoid chal_simio_simvcc_sel(
    CHAL_HANDLE handle, 
    cBool on
);


/**
*
*  @brief  Soft reset DET&ESD
*
*  @param  handle   (in) this SIMIO instance handle
*
*  @return 
*****************************************************************************/
cVoid chal_simio_soft_reset_det_esd(CHAL_HANDLE handle);

/**
*
*  @brief  usim control register
*
*  @param  control the voltage polarity of the vpp and vcc and the emergency shutdown input
*
*  @return 
*****************************************************************************/
cVoid chal_simio_usim_control_register(
    cBool vpp_polarity_low,
    cBool vcc_polarity_low,
    cBool batrm_on
);

/** @} */
 
#ifdef __cplusplus
}
#endif
#endif

