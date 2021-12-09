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
* @file  simio_atr.h
*
* @brief SIMIO ATR Processor interface
*
* @note
*****************************************************************************/

#ifndef __SIMIO_ATR_H
#define __SIMIO_ATR_H


/**
* Supported action codes
*****************************************************************************/
typedef enum
{
    SIMATR_ACTION_NOP,             ///< No action required for client
    SIMATR_ACTION_PTS_SEND,        ///< Client should send PTS
    SIMATR_ACTION_PTS_RECEIVE,     ///< Tell client PTS response received
    SIMATR_ACTION_CONFIG_HW        ///< Tell client ATR-PTS done
} SIMATR_ACTION_t;

/**
* Supported error codes
*****************************************************************************/
typedef enum
{
    SIMATR_ERROR_NONE,             ///< No error
    SIMATR_ERROR_CORRUPTED,        ///< Find corruption in ATR-PTS
    SIMATR_ERROR_WRONG_VOLTAGE,    ///< Wrong voltage being used
} SIMATR_ERROR_t;

/**
* Supported error codes (direct map to those in uicc.h)
*
* BRCM/LesterChan 02/01/2006: add "UICC_CLOCK_NOT_DEFINED" to indicate
* that the UICC characteristics TLV (see Section 11.1.1.4.6.1 of
* 3GPP TS 31.101) is not present in the response of "USIM-ADF" selection.
*
* This legacy enum has a mix of clock stop info in ATR and the requests
* from client. Need to clean this up.
*****************************************************************************/
typedef enum {
    SIMIO_CLOCK_STOP_ALLOWED,      ///< indicate to allow clock stop
    SIMIO_NO_PREFERRED_LEVEL,      ///< ATR doesn't care clock stop level
    SIMIO_HIGH_LEVEL_PREFERRED,    ///< ATR prefers clock stop at high level
    SIMIO_LOW_LEVEL_PREFERRED,     ///< ATR prefers clock stop at low level
    SIMIO_CLOCK_STOP_NOT_ALLOWED,  ///< ATR doesn't allow clock stop
    SIMIO_CLOCK_STOP_ONLY_HIGH,    ///< indicate to stop clock at high
    SIMIO_CLOCK_STOP_ONLY_LOW,     ///< indicate to stop clock at low
    SIMIO_CLOCK_NOT_DEFINED        ///< indicate no TLV not present
} SIMIO_CLOCK_STOP_MODE_t;


/**
* The object passed around
*****************************************************************************/
typedef struct
{
    UInt8       id;                ///< instance ID
    void*       priv;              ///< data hidden from client
} SIMIO_ATR_t;


/**
*
*  @brief  Initialize SIMIO ATR processor
*
*  @param  index  (in) which instance
*
*  @return handle for this instance
*****************************************************************************/
SIMIO_ATR_t *SIMIO_ATR_Init(UInt8 index);


/**
*
*  @brief  De-Initialize SIMIO ATR processor
*
*  @param  handle  (in) which instance
*
*  @return none
*****************************************************************************/
void SIMIO_ATR_Cleanup(SIMIO_ATR_t *handle);


/**
*
*  @brief  Start SIMIO ATR processor
*
*  @param  handle  (in) which instance
*  @param  voltage  (in) VCC voltage being applied
*  @param  pref_protocol (in) preferred protocol for T1 capable cards
*
*  @return none
*****************************************************************************/
void SIMIO_ATR_Start(SIMIO_ATR_t *handle,
                     SimVoltageLevel_t voltage, 
                     PROTOCOL_t pref_protocol);


/**
*
*  @brief  Process a byte based on specification. This function runs a state
*          machine to processes one byte at a time for the Answer-To-Reset
*          message. It returns an action code to the caller.
*
*  @param  handle  (in) which instance
*  @param  rcvd_byte  (in) new data
*
*  @return action code
*****************************************************************************/
SIMATR_ACTION_t SIMIO_ATR_Process_A_Byte(SIMIO_ATR_t *handle, UInt8 rcvd_byte);


/**
*
*  @brief  Trying to detect Invesre encoding token
*
*  @param  handle  (in) which instance
*  @param  token  (in) token to be checked
*
*  @return TRUE if Inverse encoding is detected
*****************************************************************************/
Boolean SIMIO_ATR_Detect_Inverse(SIMIO_ATR_t *handle, UInt8 token);


/**
*
*  @brief  Convert data based on encoding scheme. 
*          See ISO/IEC 7816-3 section 6.1.4 
*
*  @param  handle  (in) which instance
*  @param  buffer  (in/out) data to be converted
*  @param  size  (in) data size in bytes
*
*  @return none
*****************************************************************************/
void SIMIO_ATR_Convert(const SIMIO_ATR_t *handle, UInt8 *buffer, UInt16 size);


/**
*
*  @brief  Check if ATR process is started
*
*  @param  handle  (in) which instance
*
*  @return TRUE if started
*****************************************************************************/
Boolean SIMIO_ATR_Started(const SIMIO_ATR_t *handle);


/**
*
*  @brief  Check if ATR process is passed
*
*  @param  handle  (in) which instance
*
*  @return TRUE if passed
*****************************************************************************/
Boolean SIMIO_ATR_Passed(const SIMIO_ATR_t *handle);


/**
*
*  @brief  Check ATR process final status
*
*  @param  handle  (in) which instance
*
*  @return error code
*****************************************************************************/
SIMATR_ERROR_t SIMIO_ATR_Check_Error(const SIMIO_ATR_t *handle);


/**
*
*  @brief  Get key ATR parameters
*
*  @param  handle  (in) which instance
*  @param  ATR_Param  (out) paramters
*
*  @return none
*****************************************************************************/
void SIMIO_ATR_Get_Params(const SIMIO_ATR_t *handle, 
                          SIMIO_USIMAP_ATR_PARAM_t *ATR_Param);


/**
*
*  @brief  Get raw ATR data
*
*  @param  handle  (in) which instance
*
*  @return Raw ATR data
*****************************************************************************/
SIMIO_RAW_ATR_INFO_t *SIMIO_ATR_Get_Raw_ATR(const SIMIO_ATR_t *handle);


/**
*
*  @brief  Get PTS bytes
*
*  @param  handle  (in) which instance
*  @param  buffer  (out) PTS data
*  @param  size  (in) buffer size in bytes
*
*  @return PTS data in bytes
*****************************************************************************/
UInt16 SIMIO_ATR_Get_PTS(const SIMIO_ATR_t *handle, UInt8* buffer, UInt16 size);


/**
*
*  @brief  Get native (ATR-based) clock stop mode
*
*  @param  handle  (in) which instance
*
*  @return native clock stop mode
*****************************************************************************/
SIMIO_CLOCK_STOP_MODE_t SIMIO_ATR_Get_ClkStpMode(const SIMIO_ATR_t *handle);


/**
*
*  @brief  Negotiate a new clock stop mode
*
*  @param  handle  (in) which instance
*  @param  mode  (in) proposed mode
*
*  @return Negotiated clock stop mode
*****************************************************************************/
SIMIO_CLOCK_STOP_MODE_t SIMIO_ATR_Change_ClkStpMode(const SIMIO_ATR_t *handle,
                                                SIMIO_CLOCK_STOP_MODE_t mode);



/** 
* 
*  @brief   Checks if "UICC-CLF" Interface is Supported 
* 
*  @param   handle  (in) which instance 
*  @return  TRUE if UICC-CLF is Suppoted; FALSE otherwise. 
* 
*******************************************************************************/ 
Boolean SIMIO_ATR_Is_UICC_CLF_Interface_Support(const SIMIO_ATR_t *handle); 


#endif /*__SIMIO_ATR_H*/

