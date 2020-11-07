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

#include "brcm_rdb_simi.h"
#include <bcmtypes.h>
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>

#include "chal_types.h"
#include "chal_simio.h"
#include <simio_def_common.h>
#include "simio.h"
#include "simio_atr.h"

#include "brcm_rdb_util.h"
#include "sim_export_defs.h"

#define CHAL_LOG      0
#define DPRINT CHAL_DBG


/* =========================  begin to put stuff missing in rdb here */
#define BCMRDB_SIMI_BLK_CNT       2

#define SCR_ETUCKS_SPD_F372_D1    0x173

#define SCR_ETUCKS_SPD_F512_D8    0x3F
#define SCR_ETUCKS_SPD_F512_D16   0x1F
#define SCR_ETUCKS_SPD_F512_D32   0xF
#define SCR_ETUCKS_SPD_F512_D64   0x7

#define SCR_ETUCKS_SPD_F1024_D8   0x7F
#define SCR_ETUCKS_SPD_F1024_D16  0x3F
#define SCR_ETUCKS_SPD_F1024_D32  0x1F
#define SCR_ETUCKS_SPD_F1024_D64  0xF

#define SCR_ETUCKS_SPD_F2048_D8   0xFF
#define SCR_ETUCKS_SPD_F2048_D16  0x7F
#define SCR_ETUCKS_SPD_F2048_D32  0x3F
#define SCR_ETUCKS_SPD_F2048_D64  0x1F

#define BCMRDB_SIMI_FIFO_SIZE     0x40
/* =========================== end of stuff missing in rdb */


typedef struct chal_simi_s {
    cUInt32 base;     // SIMI registers
} chal_simi_t, *p_chal_simi_t;

static chal_simi_t simi_dev[BCMRDB_SIMI_BLK_CNT];

/**
*
*  @brief  Initialize CHAL SIMIO for the passed SIMIO instance
*
*  @param  baseAddr  (in) mapped address of this SIMIO instance
*
*  @return CHAL handle for this SIMIO instance
*****************************************************************************/
CHAL_HANDLE chal_simio_init(cUInt32 baseAddr)
{
    chal_simi_t *dev = NULL;
    int i;
  
    chal_dprintf( CDBG_INFO, "chal_simio_init, base=0x%x\n", baseAddr);
  
    // Don't re-init a block
    for (i = 0; i < BCMRDB_SIMI_BLK_CNT; i++)
        if (simi_dev[i].base == baseAddr) {
            chal_dprintf(CDBG_ERRO, "ERROR: chal_simio_init: %d already initialized\n", i);
            return (CHAL_HANDLE)dev;
        }
        
    // find a free slot for this new block
    for (i = 0; i < BCMRDB_SIMI_BLK_CNT; i++) {
        if (!simi_dev[i].base)
            break;
    }
    if (i != BCMRDB_SIMI_BLK_CNT) {
        simi_dev[i].base = baseAddr;
        dev = &simi_dev[i];
        
        chal_simio_enable_clock((CHAL_HANDLE)dev, 0);
        BRCM_WRITE_REG_FIELD(baseAddr, SIMI_SCR, SIMEN, 0);  
    }
    else
        chal_dprintf(CDBG_ERRO, "ERROR: chal_simio_init: bad instance\n");
         
    return (CHAL_HANDLE)dev;
}

/**
*
*  @brief  De-Initialize CHAL SIMI for the passed SIMI instance
*
*  @param  handle  (in) this SIMI instance handle
*
*  @return none
*****************************************************************************/
cVoid chal_simio_deinit(CHAL_HANDLE handle)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    chal_dprintf( CDBG_INFO, "chal_simio_deinit\n");  
  
    if (dev == NULL)
    {
        chal_dprintf(CDBG_ERRO, "ERROR: chal_simio_deinit: NULL handle\n");
        return; 
    } 
    
    // Make sure the block is at the lowest power mode
    chal_simio_enable_clock((CHAL_HANDLE)dev, 0);
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, SIMEN, 0);

    dev->base = 0;    
}


/**
*
*  @brief  Start SIMI device
*
*  @param  handle  (in) this SIMI instance handle
*
*  @return none
*****************************************************************************/
cVoid chal_simio_start(CHAL_HANDLE handle, Boolean warmReset)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    
    if (!warmReset)
    {
        // First zero out main control and FIFO control registers
        BRCM_WRITE_REG(dev->base, SIMI_SCR, 0);
        BRCM_WRITE_REG(dev->base, SIMI_SFCR, 0);
    }
    
    // No retry in ATR phase
    chal_simio_set_tx_retry(handle, 0);
    chal_simio_set_rx_retry(handle, 0);

    // clear all ints
    chal_simio_read_intr_status(handle);

    /* Set Rx FIFO timeout interrupt for 5 ms */
    /* timeout is in unit of 0.1 ms. The valid range for timeout is 0-1023 inclusive.*/
    chal_simio_set_rx_timeout(handle, 12, 100, 50);

    // Set initial RX threshold to be 0 so we can respond quicker
    chal_simio_set_rx_threshold(handle, 0);

    // Set initial TX threshold to be 1
    chal_simio_set_tx_threshold(handle, 1);

    // enable FIFO
    chal_simio_enable_fifo(handle, 1);
    
    // start with even parity
    chal_simio_set_parity(handle, 0, 0, 0);
    
    // start with speed SPEED_F372_D1
    chal_simio_set_speed(handle, CHAL_SIMIO_SPD_372_1);
 
    // set RST to Low
    chal_simio_set_reset_level(handle, 0);
 
    // VPP should be in IDLE mode (ME should not touch it, GSM 11.11.5.3)
    chal_simio_enable_vpp(handle, 0);
    
    // set CLK level to Low and enable clock
    chal_simio_set_clockstop_level(handle, 0);
    chal_simio_enable_clock(handle, 1);

    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, DATEN, 1);

    // enable start bit check back
	BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, CHECKBACK_EN, 1);

    
    /* 12 ETU is the default for half-duplex character transmission protocol. 
     * See Section 6.1.4.4 of ISO/IEC 7816-3.
     */
    // Need to add 1 etu for set Guard Time between two consecutive transmitting characters.
    chal_simio_set_extra_guard_time(handle, 1);
    
    // Add an extra guard time for RX-to-TX turnaround     
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_STGTR, STGTR, 4);	// 12 + 4 extra etu's

    chal_simio_enable_dma(handle, 0);
    
    // Enable controller
    // SIMEN is bit31. We explicitly cast to avoid the sign change warning from the macro
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, SIMEN, (BRCM_REGTYPE(SIMI_SCR))1);
}

#if !defined(_RHEA_)
    /* want a delay of at least 1us, for this, need to know the maximum speed of the core (TURBO frequency) */
    #define MAX_CPU_SPEED_MHZ 550
#endif
#define NUM_NOPS_FOR_1us  MAX_CPU_SPEED_MHZ
static void IO_Wait_us(int num_us)
{
    Int32 i;

    for(i = num_us*NUM_NOPS_FOR_1us/10; i > 0; i--)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
    }
}

static void IO_Wait_100ns(int num_100ns)
{
    Int32 i;

    for(i = num_100ns*NUM_NOPS_FOR_1us/10; i > 0; i--)
    {
        __nop();
    }
}

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
cVoid chal_simio_stop(CHAL_HANDLE handle, Boolean clk_on)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    
    // clear all ints
    chal_simio_read_intr_status(handle);
    // disable FIFO
    chal_simio_enable_fifo(handle, 0);
    // disable all ints
    chal_simio_disable_intr(handle, CHAL_SIMIO_INT_ALL);
    IO_Wait_100ns(5);

    // set RST to L
    chal_simio_set_reset_level(handle, 0);
    IO_Wait_100ns(5);

    //need to leave the clock ON during warm reset as per the spec.
    // other cases, turn the clock OFF
    // Leaving other lines (IO and VPP) and the SIM controller in their current state when leaving CLK ON.

    if (!clk_on) {
        // set CLK level to Low and disable clock
        chal_simio_set_clockstop_level(handle, 0);
        chal_simio_enable_clock(handle, 0);
        IO_Wait_100ns(5);
    
        // VPP inactive (should not touch it by ME, GSM 11.11.5.3)
        chal_simio_enable_vpp(handle, 0);

        // need to keep IO HIGH when CLK is running to satisfy the spec requirement during card deactivation.
        // set IO to A - low voltage
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, DATEN, 0);
        IO_Wait_us(1);                   

        //Disabling the SIM controller will stop the SIM CLK and therefore check for it here.
        // Disable controller
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, SIMEN, 0);
    }

}


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
)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
	
    if (odd)
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, PARSEL, 1);
    else
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, PARSEL, 0);

    if (tx_off)
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, TX_PARITY_OFF, 1);
    else
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, TX_PARITY_OFF, 0);

    if (rx_off)
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, RX_PARITY_OFF, 1);
    else
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, RX_PARITY_OFF, 0);
}

/**
*
*  @brief  turn on/off SIMI device VPP
*
*  @param  handle  (in) this SIMI instance handle
*          on  (in) TRUE for on
*
*  @return none
*****************************************************************************/
cVoid chal_simio_enable_vpp(CHAL_HANDLE handle, cBool on)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    if (on)
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, VPPEN, 1);
    else
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, VPPEN, 0);
}


/**
*
*  @brief  Soft Reset ESD 
*
*  @param  handle  (in) this SIMI instance handle
*
*  @return none
*****************************************************************************/
cVoid chal_simio_soft_reset_esd(CHAL_HANDLE handle)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    BRCM_WRITE_REG_FIELD(dev->base, SIMI_DESDCR, SIM_DET_ESD_SOFT_RST, 1);

    mdelay(2);

    BRCM_WRITE_REG_FIELD(dev->base, SIMI_DESDCR, SIM_DET_ESD_SOFT_RST, 0); 
}

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
)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    if(!on)
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_DESDCR, SIM_ESD_EN, 0);  
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SLDOCR, SIMVCC_EMERGENCY_EN, 0);  
    }
    else
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_DESDCR, SIM_ESD_EN, 1);  
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SLDOCR, SIMVCC_EMERGENCY_EN, 1);  
    }

    if(!batrm_on)
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_DESDCR, SIM_BATRM_ESD_EN, 0);  
    }
    else
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_DESDCR, SIM_BATRM_ESD_EN, 1);  
    }

    if(!cardout_on)
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCARDSR, CARDOUT_ESHUTDOWN, 0);  
    }
    else
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCARDSR, CARDOUT_ESHUTDOWN, 1);  
    }

    if(!reset_on)
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_DESDCR, SIM_WATCHDOG_ESD_EN, 0);  
    }
    else
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_DESDCR, SIM_WATCHDOG_ESD_EN, 1);  
    }

}







/**
*
*  @brief  turn on/off SIMI DMA
*
*  @param  handle  (in) this SIMI instance handle
*          on  (in) TRUE to turn on the DMA feature
*
*  @return none
*****************************************************************************/
cVoid chal_simio_enable_dma(CHAL_HANDLE handle, cBool on)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    if (on)
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, DMAEN, 1);
    else
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, DMAEN, 0);
	
}

/**
*
*  @brief  turn on/off SIMI mask off data in feature
*
*  @param  handle  (in) this SIMI instance handle
*          on  (in) TRUE to turn on the feature
*
*  @return none
*****************************************************************************/
cVoid chal_simio_mask_data_in(CHAL_HANDLE handle, cBool on)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    if (on)
        // SIMI ignores any data in SIMDAT line
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, SIMDAT_MASK_EN, 1);
    else
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, SIMDAT_MASK_EN, 0);
}

/**
*
*  @brief  turn on/off SIMI device clock
*
*  @param  handle  (in) this SIMI instance handle
*          on  (in) TRUE to turn on the clock
*
*  @return none
*****************************************************************************/
cVoid chal_simio_enable_clock(CHAL_HANDLE handle, cBool on)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    if (on)
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, STOP, 0);
    else
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, STOP, 1);
}

/**
*
*  @brief  check SIMI device clock on/off state
*
*  @param  handle  (in) this SIMI instance handle
*
*  @return TRUE of clock is stopped
*****************************************************************************/
cBool chal_simio_is_clock_off(CHAL_HANDLE handle)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    return (cBool)BRCM_READ_REG_FIELD(dev->base, SIMI_SCR, STOP);
}

/**
*
*  @brief  set SIMI device clock stop level
*
*  @param  handle  (in) this SIMI instance handle
*          high  (in) TRUE to set stop level high
*
*  @return none
*****************************************************************************/
cVoid chal_simio_set_clockstop_level(CHAL_HANDLE handle, cBool high)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    if (high)
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, STPS, 1);
    else
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, STPS, 0);
}

/**
*
*  @brief  Get SIMI device clock stop level
*
*  @param  handle  (in) this SIMI instance handle
*
*  @return TRUE if clock stop level is high
*****************************************************************************/
cBool chal_simio_get_clockstop_level(CHAL_HANDLE handle)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    return (cBool)BRCM_READ_REG_FIELD(dev->base, SIMI_SCR, STPS);
}

/**
*
*  @brief  set SIMI device reset level
*
*  @param  handle  (in) this SIMI instance handle
*          high  (in) TRUE to set reset line level high
*
*  @return none
*****************************************************************************/
cVoid chal_simio_set_reset_level(CHAL_HANDLE handle, cBool high)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    if (high)
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, RSTS, 1);
    else
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, RSTS, 0);
}

/**
*
*  @brief  Get SIMI device reset level
*
*  @param  handle  (in) this SIMI instance handle
*
*  @return TRUE if reset line level is high
*****************************************************************************/
cBool chal_simio_get_reset_level(CHAL_HANDLE handle)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    return (cBool)BRCM_READ_REG_FIELD(dev->base, SIMI_SCR, RSTS);
}


/**
*
*  @brief  Set SIMIO speed
*
*  @param  handle  (in) this SIMI instance handle
*  @param  speed   (in) speed to be set
*
*  @return none
*****************************************************************************/
cVoid chal_simio_set_speed(CHAL_HANDLE handle, CHAL_SIMIO_SPEED_t speed)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    cUInt32 val;    

    //chal_dprintf( CDBG_INFO, "chal_simio_set_speed, %d\n", speed);
    
	switch (speed)
	{
		case CHAL_SIMIO_SPD_512_8:
		    val = SCR_ETUCKS_SPD_F512_D8;
		    break;
		case CHAL_SIMIO_SPD_512_16:
		    val = SCR_ETUCKS_SPD_F512_D16;
		    break;
		case CHAL_SIMIO_SPD_512_32:
		    val = SCR_ETUCKS_SPD_F512_D32;
		    break;
		case CHAL_SIMIO_SPD_512_64:
		    val = SCR_ETUCKS_SPD_F512_D64;
		    break;
		case CHAL_SIMIO_SPD_1024_8:
		    val = SCR_ETUCKS_SPD_F1024_D8;
		    break;
		case CHAL_SIMIO_SPD_1024_16:
		    val = SCR_ETUCKS_SPD_F1024_D16;
		    break;
		case CHAL_SIMIO_SPD_1024_32:
		    val = SCR_ETUCKS_SPD_F1024_D32;
		    break;
		case CHAL_SIMIO_SPD_1024_64:
		    val = SCR_ETUCKS_SPD_F1024_D64;
		    break;
		case CHAL_SIMIO_SPD_2048_8:
		    val = SCR_ETUCKS_SPD_F2048_D8;
		    break;
		case CHAL_SIMIO_SPD_2048_16:
		    val = SCR_ETUCKS_SPD_F2048_D16;
		    break;
		case CHAL_SIMIO_SPD_2048_32:
		    val = SCR_ETUCKS_SPD_F2048_D32;
		    break;
		case CHAL_SIMIO_SPD_2048_64:
		    val = SCR_ETUCKS_SPD_F2048_D64;
		    break;
		case CHAL_SIMIO_SPD_372_1:
		default:
		    val = SCR_ETUCKS_SPD_F372_D1;
		    break;
	}
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SFDRR, FD_RATIO, val);
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SFDRR, FD_RATIO_MODE_EN, 1);
}


/**
*
*  @brief  Set SIMIO protocol
*
*  @param  handle  (in) this SIMI instance handle
*  @param  t1      (in) true for t1, false for t0
*
*  @return none
*****************************************************************************/
cVoid chal_simio_set_protocol(CHAL_HANDLE handle, cBool t1)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    if (t1)
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, PTS, 1);
    else
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, PTS, 0);
}


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
cUInt32 chal_simio_read_intr_status(CHAL_HANDLE handle)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    cUInt32 mask = BRCM_READ_REG(dev->base, SIMI_SSR);

    // clear first
    BRCM_WRITE_REG(dev->base, SIMI_SSR, mask);

    // we have bit-by-bit mapping so we save the mask translation.
    // Note, the old driver doesn't mask RDR when checking for RDR interrupt.
    mask &= BRCM_READ_REG(dev->base, SIMI_SIER);
    mask &= CHAL_SIMIO_INT_ALL;

    return mask;
}


/**
*
*  @brief  Enable SIMIO interrupts
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  mask     (in) interrupts to enable
*
*  @return none
*****************************************************************************/
cVoid chal_simio_enable_intr(CHAL_HANDLE handle, cUInt32 mask)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    
    BRCM_WRITE_REG(dev->base, SIMI_SIER,
                   BRCM_READ_REG(dev->base, SIMI_SIER) | mask);
}

/**
*
*  @brief  Disable SIMIO interrupts
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  mask     (in) interrupts to disable
*
*  @return none
*****************************************************************************/
cVoid chal_simio_disable_intr(CHAL_HANDLE handle, cUInt32 mask)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    
    BRCM_WRITE_REG(dev->base, SIMI_SIER,
                   BRCM_READ_REG(dev->base, SIMI_SIER) & ~mask);
}

/**
*
*  @brief  Enable SIMIO generic compare counter
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  val     (in) val to compare
*
*  @return none
*****************************************************************************/
// Generic counter API follows the old code sequence. They do look strange. Need to revisit.
cVoid chal_simio_enable_counter(CHAL_HANDLE handle, cUInt16 val)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    BRCM_WRITE_REG(dev->base, SIMI_SSR, CHAL_SIMIO_INT_GCNTI);
    
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, GCTEN, 1);

    //have to clear and disable the interrupt first. Otherwise, after reset, the next interrupt will come soon after because it equals 0 at first moment.
    chal_simio_disable_intr(handle, CHAL_SIMIO_INT_GCNTI);
    BRCM_WRITE_REG(dev->base, SIMI_SSR, CHAL_SIMIO_INT_GCNTI);
    
    //toggle the SIMVPP in order to reset the generic counter, see 2133 data sheet
    chal_simio_enable_vpp(handle, 1);
    chal_simio_enable_vpp(handle, 0);

    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SGCCR, SGCCR, val);
    BRCM_WRITE_REG(dev->base, SIMI_SSR, CHAL_SIMIO_INT_GCNTI);
    chal_simio_enable_intr(handle, CHAL_SIMIO_INT_GCNTI);
    
    // this is needed otherwise we see interrupt immediately
    BRCM_WRITE_REG(dev->base, SIMI_SSR, CHAL_SIMIO_INT_GCNTI);
}

/**
*
*  @brief  Disable SIMIO generic compare counter
*
*  @param  handle   (in) this SIMIO instance handle
*
*  @return none
*****************************************************************************/
cVoid chal_simio_disable_counter(CHAL_HANDLE handle)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    // prepare for next time
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SGCCR, SGCCR, 0xffffff);

    chal_simio_disable_intr(handle, CHAL_SIMIO_INT_GCNTI);
    BRCM_WRITE_REG(dev->base, SIMI_SSR, CHAL_SIMIO_INT_GCNTI);
    
    //Generic timer itself  has to be enable in order for the reset to take effect(Jian:???) 
    //So, disable the timer from the controller at last
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, GCTEN, 0);
}


/**
*
*  @brief  Set extra guard time
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  time (in) new guard time
*
*  @return None
*****************************************************************************/
cVoid chal_simio_set_extra_guard_time(CHAL_HANDLE handle, cUInt16 time)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SECGTR, SECGTR, time);
}


/**
*
*  @brief  Set SIMIO TX FIFO threshold
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  threshold (in) new threshold
*
*  @return TRUE if new threshold is accepted
*****************************************************************************/
cBool chal_simio_set_tx_threshold(CHAL_HANDLE handle, cUInt8 threshold)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    if (threshold >= BCMRDB_SIMI_FIFO_SIZE)
        return 0;
    
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SFCR, TXTHRE, threshold);
    return 1;
}


/**
*
*  @brief  Set SIMIO RX FIFO threshold
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  threshold (in) new threshold
*
*  @return TRUE if new threshold is accepted
*****************************************************************************/
cBool chal_simio_set_rx_threshold(CHAL_HANDLE handle, cUInt8 threshold)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    if (threshold >= BCMRDB_SIMI_FIFO_SIZE)
        return 0;
    
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SFCR, RXTHRE, threshold);
    return 1;
}


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
    cUInt16 timeout)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
	
#if defined(_ATHENA_)
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SRTOR, CLK100US_PRESCALE, prescale);
#else
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SRTOR, CLK1MHZ_PRESCALE, prescale);
#endif

    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SRTOR, CLK100US_DIV, divisor);

    timeout &= SIMI_SRTOR_TIMEOUT_VALUE_MASK;
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SRTOR, TIMEOUT_VALUE, timeout);

    return 1;
}

/**
*
*  @brief  Set tx retry times
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  retry   (in) retry times
*
*  @return TRUE if new threshold is accepted
*****************************************************************************/
cBool chal_simio_set_tx_retry(CHAL_HANDLE handle, cUInt8 retry)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, TXRETRY, retry);
    return 1;
}


/**
*
*  @brief  Set rx try times
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  retry   (in) retry times
*
*  @return TRUE if new threshold is accepted
*****************************************************************************/
cBool chal_simio_set_rx_retry(CHAL_HANDLE handle, cUInt8 retry)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, RXRETRY, retry);
    return 1;
}


/**
*
*  @brief  enable FIFO
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  on   (in) TRUE to enable
*
*  @return None
*****************************************************************************/
cVoid chal_simio_enable_fifo(CHAL_HANDLE handle, cBool on)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    
    if (on) 
    {
         BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, FIFOEN, 1);
         chal_simio_flush_fifo(handle);
    }
    else
    {
        chal_simio_flush_fifo(handle);
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, FIFOEN, 0);
    }
}

/**
*
*  @brief  Flush FIFO
*
*  @param  handle   (in) this SIMIO instance handle
*
*  @return None
*****************************************************************************/
cVoid chal_simio_flush_fifo(CHAL_HANDLE handle)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    
    // FLUSH is bit31. We explicitly cast to avoid the sign change warning from the macro 
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SFCR, FLUSH, (BRCM_REGTYPE(SIMI_SFCR))1);
}

/**
*
*  @brief  Get TX/RX FIFO size
*
*  @param  handle   (in) this SIMIO instance handle
*
*  @return TX-remaining bytes, RX-received bytes
*****************************************************************************/
cUInt32 chal_simio_get_fifo_size(CHAL_HANDLE handle)
{
        return BCMRDB_SIMI_FIFO_SIZE;
}

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
cUInt32 chal_simio_read_data(CHAL_HANDLE handle, cUInt16 *pBuffer, cUInt32 size)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    cUInt16 len;
	cUInt16 i = 0;
	cBool continue_flag = 1;
    
    if (pBuffer == NULL || size == 0)
	{
        return 0;
	}

	len = BRCM_READ_REG_FIELD(dev->base, SIMI_SFCR, FIFOCNT); 
    
	while (continue_flag)
	{
		while (len != 0)
		{
			pBuffer[i++] = BRCM_READ_REG(dev->base, SIMI_SDR); 
			len--;

			if (i >= size)
			{
				continue_flag = 0;
				break;
			}
		}

		// Read the Rx counter again in case new data is received while we are reading data
		if (len == 0)
		{
			len = BRCM_READ_REG_FIELD(dev->base, SIMI_SFCR, FIFOCNT); 

			if (len == 0)
			{
				// Clear the SSR Rx bits since there is no more data in FIFO
				BRCM_WRITE_REG(dev->base, SIMI_SSR, (CHAL_SIMIO_INT_RXTOUT | CHAL_SIMIO_INT_RXTHRE | CHAL_SIMIO_INT_ROVF | CHAL_SIMIO_INT_RDR) & CHAL_SIMIO_INT_ALL);
				continue_flag = 0;
			}
			/* Else: we will read the new data in while loop */
		}
	}

    return i;
}


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
cUInt32 chal_simio_write_data(CHAL_HANDLE handle, cUInt8 *pBuffer, cUInt32 size)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    cUInt8 len, i;
    
    if (pBuffer == NULL || size == 0)
        return 0;
    
#if 1
    // The old code does MIN(READ_TXTHRE_VAL(), MAX_SIM_FIFO_SIZE-1)
    // This might impact performance.
    len = BRCM_READ_REG_FIELD(dev->base, SIMI_SFCR, TXTHRE); 
    //(pRegs->m_SIMI_SFCR & SIMI_SFCR_TxTHRE) >> 8;
#else
    // enhanced way: we need to write min(size, free_space) bytes to TX FIFO
    // load len with number of bytes in TX FIFO
    i = (pRegs->m_SIMI_SFCR & SIMI_SFCR_FIFOCNT) >> 26;
    // convert to free space
    len = BCMRDB_SIMI_FIFO_SIZE - i;
    // don't over write
    if (len > size)
        len = size;    
#endif
 
    for (i = 0; i < len; i++)
        BRCM_WRITE_REG(dev->base, SIMI_SDR, pBuffer[i]);
                
	// Clear the TXDONE bit since there are data in FIFO
	BRCM_WRITE_REG(dev->base, SIMI_SSR, CHAL_SIMIO_INT_TXDONE);

    return len;
}


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
						  cUInt32* sfcrReg, cUInt32* sdebugReg)
{
	chal_simi_t *dev = (chal_simi_t *) handle;

	if (scrReg != NULL)
	{
		*scrReg = BRCM_READ_REG(dev->base, SIMI_SCR);
	}

	if (sierReg != NULL)
	{
		*sierReg = BRCM_READ_REG(dev->base, SIMI_SIER);
	}

	if (ssrReg != NULL)
	{
		*ssrReg = BRCM_READ_REG(dev->base, SIMI_SSR);
	}

	if (sfcrReg != NULL)
	{
		*sfcrReg = BRCM_READ_REG(dev->base, SIMI_SFCR);
	}

	if (sdebugReg != NULL)
	{
		*sdebugReg = BRCM_READ_REG(dev->base, SIMI_SIMDEBUG);
	}
}

/**
*
*  @brief  Turn on/off clock divisor
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  on       (in) TRUE if turn 
*  @param  divisor  (in) 8-bit divisor
*
*****************************************************************************/
cVoid chal_simio_set_divisor(CHAL_HANDLE handle, cBool on, cUInt8 divisor)
{
	chal_simi_t *dev = (chal_simi_t *) handle;
	cUInt32 val;
 
    if (!on)
    {
        // Clear bit15
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCDR, SIMCLK_DIV_EN, 0);

    }
    else
    {
        val = (1<<SIMI_SCDR_SIMCLK_DIV_EN_SHIFT) | divisor;
        
        BRCM_WRITE_REG(dev->base, SIMI_SCDR, val);
    }
}


/**
*
*  @brief  Turn on/off extra two samples on input SIMDAT signal
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  on       (in) TRUE if turn 
*  @param  divisor  (in) 9-bit extra sample offset
*
*****************************************************************************/
cVoid chal_simio_set_extra_sample(CHAL_HANDLE handle, cBool on, cUInt16 divisor)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    cUInt32 val;

    if(!on)
    {
        // Clear bit EXTRA_SAMPLE_EN
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SESR, EXTRA_SAMPLE_EN, 0);
    }
    else
    {
        val = divisor & SIMI_SESR_EXTRA_SAMPLE_OFFSET_MASK;
        val |= SIMI_SESR_EXTRA_SAMPLE_EN_MASK;
        
        BRCM_WRITE_REG(dev->base, SIMI_SESR, val);
    }
}


/**
*
*  @brief  Turn on/off TXENDQUICK
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  on       (in) TRUE if turn on
*
*****************************************************************************/
cVoid chal_simio_set_txendquick(CHAL_HANDLE handle, cBool on)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    if(!on)
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, TXENDQUICK, 0);  
    }
    else
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, TXENDQUICK, 1);  
    }
}


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
)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    if(!on)
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCARDSR, PRESENCE_DEBOUNCE_EN, 0);  
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_DESDCR, SIM_DET_EN, 0);  
    }
    else
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_DESDCR, SIM_DET_EN, 1);  
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCARDSR, PRESENCE_DEBOUNCE_EN, 1);  
    }
 
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCARDSR, SIM_DEBOUNCE_TIME, 
                         debouce_time);  

    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCARDSR, PRESENCE_PRE_DEBOUNCE_MODE, 
                         debounce_mode);  

    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCARDSR, PRESENCE_LOW, presence_type);  
}


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
cUInt32 chal_simio_read_esd_det_intr_status(CHAL_HANDLE handle)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    cUInt32 mask = BRCM_READ_REG(dev->base, SIMI_DESDISR);

    // clear first
    BRCM_WRITE_REG(dev->base, SIMI_DESDISR, mask);

    mask &= CHAL_SIMIO_INT_ESD_DET_ALL;

    return mask;
}


/**
*
*  @brief  Enable SIMIO ESD and DET interrupts
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  mask     (in) interrupts to enable
*
*  @return none
*****************************************************************************/
cVoid chal_simio_enable_esd_det_intr(CHAL_HANDLE handle, cUInt32 mask)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    
    BRCM_WRITE_REG(dev->base, SIMI_DESDISR,
                   BRCM_READ_REG(dev->base, SIMI_DESDISR) | mask);
}


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
cUInt32 chal_simio_read_detection_status(CHAL_HANDLE handle)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    cUInt32 mask = BRCM_READ_REG(dev->base, SIMI_SCARDSR);

    mask &= SIMI_SCARDSR_SIM_PRESENCE_DEBED_MASK;

    mask = mask >> SIMI_SCARDSR_SIM_PRESENCE_DEBED_SHIFT;

    return mask;
}


/**
*
*  @brief  Set order/sense based on the card encoding
*
*  @param  handle   (in) this SIMIO instance handle
*  @param  inverse  (in) TRUE for inverse card, otherwise for direct card
*
*  @return 
*****************************************************************************/
cVoid chal_simio_set_order_sense(CHAL_HANDLE handle, cBool inverse)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
 
    if (inverse)
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SIMDEBUG, ORDER, 1);
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SIMDEBUG, SENSE, 1);
    }
    else
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SIMDEBUG, ORDER, 0);
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SIMDEBUG, SENSE, 0);
    }
}


/**
*
*  @brief  Soft reset controller
*
*  @param  handle   (in) this SIMIO instance handle
*
*  @return 
*****************************************************************************/
cVoid chal_simio_soft_reset(CHAL_HANDLE handle)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, SRST, 1);
    
    IO_Wait_100ns(5);

    BRCM_WRITE_REG_FIELD(dev->base, SIMI_SCR, SRST, 0);
}


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
)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    chal_dprintf( CDBG_INFO, "chal_simio_simldo_simvcc, dev->base=0x%x\n", dev->base);

    if(!on)
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SLDOCR, SIMVCC_EN, 0);  
    }
    else
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SLDOCR, SIMVCC_EN, 1);  
    }
    chal_dprintf( CDBG_INFO, "Eirlys: chal_simio_simldo_simvcc, SIMI_SLDOCR=0x%x\n", *(UInt32 *)(0x3500506c));
}


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
)
{
    chal_simi_t *dev = (chal_simi_t *) handle;

    chal_dprintf( CDBG_INFO, "chal_simio_simvcc_sel, dev->base=0x%x\n", dev->base);

    if(!on)
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SLDOCR, SIMVCC_SEL, 0);  
    }
    else
    {
        BRCM_WRITE_REG_FIELD(dev->base, SIMI_SLDOCR, SIMVCC_SEL, 1);  
    }

    chal_dprintf( CDBG_INFO, "Eirlys: chal_simio_simvcc_sel, SIMI_SLDOCR=0x%x\n", *(UInt32 *)(0x3500506c));

}

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
    )
{
    BRCM_WRITE_REG_FIELD(USIM_Control_Register_Address, GPIO_USIM_CONTROL, VPP_EN_POLARITY, vpp_polarity_low ? 1 : 0);
    BRCM_WRITE_REG_FIELD(USIM_Control_Register_Address, GPIO_USIM_CONTROL, VCC_EN_POLARITY, vcc_polarity_low ? 1 : 0);
    BRCM_WRITE_REG_FIELD(USIM_Control_Register_Address, GPIO_USIM_CONTROL, USIM_BATRM_N, batrm_on ? 1: 0);
}


/**
*
*  @brief  Soft reset DET&ESD
*
*  @param  handle   (in) this SIMIO instance handle
*
*  @return 
*****************************************************************************/
cVoid chal_simio_soft_reset_det_esd(CHAL_HANDLE handle)
{
    chal_simi_t *dev = (chal_simi_t *) handle;
    
    BRCM_WRITE_REG_FIELD(dev->base, SIMI_DESDCR, SIM_DET_ESD_SOFT_RST, 1);
    
    IO_Wait_100ns(5);

    BRCM_WRITE_REG_FIELD(dev->base, SIMI_DESDCR, SIM_DET_ESD_SOFT_RST, 0);
}


