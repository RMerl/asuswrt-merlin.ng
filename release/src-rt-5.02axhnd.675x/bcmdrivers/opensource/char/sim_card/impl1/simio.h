/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
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

/**
	@file
	@brief	This include file contains the function prototypes for
			SIM T=0 device driver.

	Reference:  ISO/IEC 7816-3

	@defgroup SIMDeviceDrivers SIM Device Drivers
	@brief This group is the device driver interface to the SIM.
	@ingroup DeviceDriverGroup
*/

#ifndef _SIMIO_H_
#define _SIMIO_H_

/* SIM voltage class supported by ME: 1.8V/3V; 3V_only; 3V/5V */
typedef enum
{
	SIM_SUPPORT_1P8V_3V,	
	SIM_SUPPORT_3V_ONLY,
	SIM_SUPPORT_3V_5V		
} SimVoltageClass_t;

/* PTS (Explicit Protocol Type Selection) states: See Section 7.4 of ISO/IEC 7816-3. 
 * The work ETU (bit period) is equal to F/D/f, where f is our default clock rate (3.25 MHz).
 */
typedef enum
{
	PTS_NOT_REQ,            // PTS not required, use default speed (F = 372, D = 1, F/D = 372)
	PTS_372_1,              // PTS required, not faster than 512/8, use default (F = 372, D = 1, F/D = 372)
	PTS_512_8,              // PTS required, F = 512, D = 8, F/D = 64
	PTS_512_16,             // PTS required, F = 512, D = 16, F/D = 32
	PTS_512_32,             // PTS required, F = 512, D = 32, F/D = 16
	PTS_512_64              // PTS required, F = 512, D = 64, F/D = 8
} PTS_t;

#define SIMIO_PARAM_CLOCKSTOP	1
#define SIMIO_PARAM_VOLTAGE		2
#define SIMIO_PARAM_T0_WWT		3

typedef struct 
{
    UInt8            TS_Char;
    PTS_t            PTS_Required;
    Boolean          is_T1_protocol;
    UInt8            f_used;
    UInt8            d_used;
    UInt32           FDratio;
    
    UInt8            nof_hist_characters;
    void             *hist_characters;
    UInt8            ATR_ExtGuardTime;
    UInt8            ATR_Wi;
    UInt8            ATR_TA1;
    UInt8            ATR_TAi_T15;
    UInt8            atr_T1_ifsc;
    UInt8            atr_T1_edc;    
    UInt8            atr_T1_cwi;
    UInt8            atr_T1_bwi;

    Boolean          ATR_Received_Voltage;
} SIMIO_USIMAP_ATR_PARAM_t; 

#define SIMIO_MAX_RAW_ATR_LEN 33
typedef struct
{
	Boolean atr_valid;
	UInt8 len;
	UInt8 data[SIMIO_MAX_RAW_ATR_LEN];
}SIMIO_RAW_ATR_INFO_t;


typedef Boolean (*SIMIO_ActivateLDO_CB_t)(SIMIO_ID_t id, SimVoltageLevel_t v);
typedef Boolean (*SIMIO_IsLDOReady_CB_t)(SIMIO_ID_t id);
typedef struct 
{
    SIMIO_ActivateLDO_CB_t activate;
    SIMIO_IsLDOReady_CB_t  isReady;
} SIMIO_LDO_CB_t; 


/** @addtogroup SIMDeviceDrivers
	@{
*/


/**
*
*  This function sets LDO callbacks for LDO control. Called before calling init.
*
*  @param		cb          (in) LDO callbacks
*
*  @return	    void
*
*****************************************************************************/
void SIMIO_SetLDO_CB(SIMIO_LDO_CB_t cb);

/**
*
*  This function initialize the SIMIO driver
*
*  @param		id      (in) SIM id
*
*  @return	    Boolean
*
*****************************************************************************/ 
Boolean SIMIO_Init(SIMIO_ID_t id, int vcc_polarity);

/**
*
*  This function start SIMIO task
*
*  @param		id      (in) SIM id
*
*  @return	    Boolean
*
*****************************************************************************/ 
Boolean SIMIO_Run(SIMIO_ID_t id);					///< start SIMIO task

/**
*
*  This function shutdown the SIMIO driver
*
*  @param		id      (in) SIM id
*
*  @return	    Boolean
*
*****************************************************************************/ 
Boolean SIMIO_Shutdown(SIMIO_ID_t id);			///< Shutdown the SIMIO driver

/**
*
*  This function reset SIM card
*
*  @param		id      (in) SIM id
*
*  @return	    void
*
*****************************************************************************/ 
void SIMIO_ResetCard(SIMIO_ID_t id);           ///< Reset SIM card

/**
*
*  This function warm reset SIM card
*
*  @param		id      (in) SIM id
*
*  @return	    void
*
*****************************************************************************/ 
void SIMIO_WarmResetCard(SIMIO_ID_t id);       ///< Warm Reset SIM card

/**
*
*  This function deactive SIM card
*
*  @param		id      (in) SIM id
*
*  @return	    void
*
*****************************************************************************/ 
void SIMIO_DeactiveCard(SIMIO_ID_t id);				///< Deactive SIM card
void SIMIO_DeactiveCard_SustainVcc(SIMIO_ID_t id);	///< Deactive SIM card while sustaining Vcc

/** @} */

/**
*
*  This function send a command to the card
*
*  @param		id          (in) SIM id
*  @param		sim_lgth    (in) total chars to be sent
*  @param		sim_cmdPtr  (in) SIM command pointer
*
*  @return	    void
*
*****************************************************************************/ 
void SIMIO_WriteCmd(					// send a command to the card
	SIMIO_ID_t id,
	UInt32 sim_lgth,					// total chars to be sent
	UInt8 *sim_cmdPtr
	);

typedef void (*SIMIO_DET_CB_t)(Boolean insert);

typedef void (*SIMIO_CB_t)(
    SIMIO_SIGNAL_t simio_sig,
    UInt16 rsp_len,
    UInt8 *rsp_data
    );

typedef Boolean (*SIMIO_Recov_Stat_CB_t)(void);

#define SIMIO_SIM_BUFFER_SIZE         261

/* Maximum length for response data is 256, plus two bytes for SW1/SW2 status bytes */
#define SIMIO_RCVD_BUFFER_SIZE        258

/**
*
*  This function set SIM detection callback function
*
*  @param		id          (in) SIM id
*  @param		cb          (in) callback function pointer
*
*  @return	    void
*
*****************************************************************************/ 
void SIMIO_RegisterDetectionCB(SIMIO_ID_t id, SIMIO_DET_CB_t cb);

/**
*
*  This function set callback function
*
*  @param		id          (in) SIM id
*  @param		cb          (in) callback function pointer
*  @param		recov_cb    (in) callback function pointer to get SIM recovery status
*
*  @return	    void
*
*****************************************************************************/ 
void SIMIO_RegisterCB(SIMIO_ID_t id, SIMIO_CB_t cb, SIMIO_Recov_Stat_CB_t recov_cb);

/**
*
*  This function set parameters
*
*  @param		id          (in) SIM id
*  @param		type        (in) type
*  @param		value32     (in) value
*
*  @return	    void
*
*****************************************************************************/
void SIMIO_SetParam(SIMIO_ID_t id, UInt8 type,  UInt32 value32 );

/**
*
*  This function delay
*
*  @param		id          (in) SIM id
*  @param		etu         (in) 
*
*  @return	    void
*
*****************************************************************************/
void SIMIO_Delay(SIMIO_ID_t id, UInt8 etu);

/**
*
*  This function get raw ATR
*
*  @param		id          (in) SIM id
*
*  @return	    SIMIO_RAW_ATR_INFO_t *
*
*****************************************************************************/
SIMIO_RAW_ATR_INFO_t *SIMIO_GetRawATR(SIMIO_ID_t id);

/**
*
*  This function get key ATR parameters
*
*  @param		id          (in) SIM id
*
*  @return	    SIMIO_USIMAP_ATR_PARAM_t *
*
*****************************************************************************/
SIMIO_USIMAP_ATR_PARAM_t *SIMIO_GetATRParam(SIMIO_ID_t id);

/**
*
*  This function set protocol
*
*  @param		id          (in) SIM id
*  @param		protocol    (in) protocol
*
*  @return	    void
*
*****************************************************************************/
void SIMIO_SetPrefProtocol(SIMIO_ID_t id, PROTOCOL_t protocol);


/**
*
*  This function returns the current voltage used to activate with SIM card.
*
*  @param		id          (in) SIM id
*
*  @return	    SimVoltageLevel_t 
*
*****************************************************************************/
SimVoltageLevel_t SIMIO_GetCurrVoltage(SIMIO_ID_t id);


/**
*
*  This function returns the current clock stop mode.
*
*  @param		id          (in) SIM id
*
*  @return	    mode 
*
*****************************************************************************/
UInt32 SIMIO_GetClockStop(SIMIO_ID_t id);


/**
*
*  This function returns the UICC clock stop mode.
*
*  @param		id          (in) SIM id
*
*  @return	    mode 
*
*****************************************************************************/
UInt32 SIMIO_GetUICCClockStop(SIMIO_ID_t id);

/**
*
*  This function get Pts
*
*  @param		id          (in) SIM id
*
*  @return	    PTS_t
*
*****************************************************************************/
PTS_t SIMIO_GetPts(SIMIO_ID_t id);

/**
*
*  This function set skip clock stop
*
*  @param		id                  (in) SIM id
*  @param		clk_stop_skipped    (in) 
*
*  @return	    void
*
*****************************************************************************/
void SIMIO_SetSkipClkStop(SIMIO_ID_t id, Boolean clk_stop_skipped);

/**
*
*  This function get command byte
*
*  @param		id      (in) SIM id
*
*  @return	    UInt8
*
*****************************************************************************/
UInt8 SIMIO_GetCmdByte(SIMIO_ID_t id);

/**
*
*  This function initializes ATR handle for SIMIO driver ID
*
*  @param		id      (in) SIM id
*
*  @return	    void
*
*****************************************************************************/
void SIMIO_ATR_Init_ById(SIMIO_ID_t id);

/**
*
*  This function cleans up the ATR handle for SIMIO driver ID
*
*  @param		id      (in) SIM id
*
*  @return	    void
*
*****************************************************************************/
void SIMIO_ATR_Cleanup_ById(SIMIO_ID_t id);

/**
*
*  This function return SIMIO driver version
*
*  @return	    driver version
*
*****************************************************************************/
UInt32 SIMIO_Get_Version(void);


/**
*
*  This function prints the passed SIM data bytes by limiting the number of characters
*  printed in each SIM_LOG_ARRAY() call. This is needed since SIM_LOG_ARRAY()
*  prints up to 42 bytes of data only. 
*
*  @param		msg      (in) Debug message
*  @param		buffer	 (in) Buffer to store the output bytes
*  @param		numOfBytes (in) Number of bytes to output
*
*****************************************************************************/
void SIMIO_PrintBytes(const char* msg, const UInt8* buffer, UInt16 numOfBytes);


/** 
*  
*  This function returns TRUE if UICC-CLF Interface is 
*  supported. 
* 
*  @param		id      (in) SIM id 
*  @return		Boolean 
*  
* ******************************************************************************/ 
Boolean SIMIO_IsUiccClfInterfaceSupported(SIMIO_ID_t id); 


#endif
