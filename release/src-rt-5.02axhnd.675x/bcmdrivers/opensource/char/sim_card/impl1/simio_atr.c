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
*   @file   simio_atr.c
*
*   @brief  This include file contains the functions required to process ATR
*
*   @Reference: ISO/IEC 7816-3
*               GSM 11.11, ETSI TS 102221
*               Protocol T=0, Asynchronous, half duplex char transmission protocol(3G)
*               Protocol T=1, Asynchronous, half duplex char transmission protocol(3G)
*
****************************************************************************/
#include <bcmtypes.h>
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>

#include "chal_types.h"
#include "chal_simio.h"

#include <simio_def_common.h>
#include "simio.h"
#include "simio_atr.h"

#include "sim_export_defs.h" 

//******************************************************************************
//                      Constant and Macro Definition
//******************************************************************************
#if DEBUG_SIM_CARDI
#define _DBG_ATR_(a) a      /* Enable SIM ATR logging */
#else
#define _DBG_ATR_(a)      /* Disable SIM ATR logging */
#endif

#define IS_TA_PRESENT(x)    ( ((x) & 0x01) == 0x01 )
#define IS_TB_PRESENT(x)    ( ((x) & 0x02) == 0x02 )
#define IS_TC_PRESENT(x)    ( ((x) & 0x04) == 0x04 )
#define IS_TD_PRESENT(x)    ( ((x) & 0x08) == 0x08 )

#define PI1(x)              ((x) & 0x1F)    


/* Voltage class indicator defined in Section 6.5.6 of ISO 7816-3 */
#define UI_CLASS_A      0x01
#define UI_CLASS_B      0x02
#define UI_CLASS_A_B    0x03
#define UI_CLASS_B_C    0x06
#define UI_CLASS_RFU    0x00

// Coding Convention specified in TS character received in ATR
#define INVERSE                 0x03
#define DIRECT                  0x3B


//--- definition for byte14 of status
#define SIM_STOP_ALLOWED_BIT    0x01
#define SIM_LEVEL_HIGT_BIT      0x04
#define SIM_LEVEL_LOW_BIT       0x08
#define SIM_VOLTAGE_BIT         0x10

/* Voltage levels supported by SIM: indicated in byte 14 of the MF selection response. 
 * See Section 4.3 of GSM 11.18. 
 */
#define SIM_VOLTAGE_BIT_MASK    0x70

#define T1_EPILOGUE_LRC         0x00
#define T1_EPILOGUE_CRC         0x01

#define ATR_U_AONLY             0x01
#define ATR_U_BONLY             0x02
#define ATR_U_AandB             0x03

// Try twice per 27.11.2.6
#define ATR_PTS_HIGHSPEEDTRY_MAX 3  

#if !(defined(CONFIG_BCM96838)) && !(defined(CONFIG_BCM96848)) && !(defined(CONFIG_BCM96858))
/* Initial character in PTS request, see Section 7.4 of ISO/IEC 7816-3 */
#define PTSS_CHAR                       0xFF

/* Format character in PTS request, see Section 7.4 of ISO/IEC 7816-3 */
#define PTS0_INDICATE_NO_PTS1           0x00
#define PTS0_INDICATE_PTS1_EXIST        0x10
#define PTS0_INDICATE_PTS2_EXIST        0x20
#define PTS0_INDICATE_PTS1PTS2_EXIST    0x30

#define PTS0_INDICATE_T1_SUPPORT        0x01            
#define PTS0_INDICATE_T1_NO_PTS1        0x01
#define PTS0_INDICATE_T1_PTS1_EXIST     0x11
#define PTS0_INDICATE_T1_PTS2_EXIST     0x21
#define PTS0_INDICATE_T1_PTS1PTS2_EXIST 0x31

/* Parameter character 1 (PTS1) value, see Section 7.4 of ISO/IEC 7816-3 */
#define PTS1_F512_D8                    0x94
#define PTS1_F512_D16                   0x95
#define PTS1_F512_D32                   0x96
#define PTS1_F512_D64					0x97

/* Parameter character 2 (PTS2) value */
#define PTS2_LIB                        0x90   // Low Impedance Buffer (LIB) support
#define PTS2_UICC_CLF                   0xA0   // UICC-CLF Interface support. 
#define PTS2_NO_INTERFACE               0x00   // Global Interface Not Supported. 

// Max. history character
#define MAX_HIST_CHARACTERS             15
#endif

typedef enum
{
    SimATR_NOT_START,
    SimATR_TS,
    SimATR_T0,
    SimATR_TA,
    SimATR_TB,
    SimATR_TC,
    SimATR_TD,
    SimATR_THC,
    SimATR_TCK,
    SimATR_PTS,
    SimATR_PTSS,
    SimATR_PTS0,
    SimATR_PTS1,
    SimATR_PTS2,      // added to handle 1st TB after T=15 (only works for T=0 & basic speed)
    SimATR_PCK,
    SimATR_PASSED
} SimATR_t;

typedef struct
{
    Boolean      used;                   // busy flag

    // read-only opertioanl environment from client
    UInt8       id;                      // instance ID
    SimVoltageLevel_t Sim_Voltage;       // SIM VCC voltage applied
    PROTOCOL_t   pref_protocol;          // preferred protocol for T1

    Boolean      ClockStopModeExistInAtr;// TRUE if clock stop mode information exists in ATR.
    SIMIO_CLOCK_STOP_MODE_t  ClockStopMode;

    // PTS to send
    UInt8        PTSBuffer[8];
    UInt16       PTSLen;

    // saved ATR parameters to populate SIMIO_USIMAP_ATR_PARAM_t
    PTS_t        PTS_Required;           // PTS Sequence Require status
    PROTOCOL_t   Current_Protocol;       // Current Protocol set by PTS
    UInt8        ATR_T1_IFSC;            // see ISO7816-3, IFSC
    UInt8        ATR_T1_EDC;             // see ISO7816-3, Error detection code
    UInt8        ATR_ExtGuardTime;       // see ISO7816-3, Extra Guard Time
    UInt8        ATR_T1_CWI;             // see ISO7816-3, Character Waiting time
    UInt8        ATR_T1_BWI;             // see ISO7816-3, block waiting time
    UInt32       FDratio;
    Boolean      ATR_Received_Voltage;
    UInt8        nof_hist_ch;            // number of historial characters
    UInt8        historial_characters[MAX_HIST_CHARACTERS];// saved historial characters
    UInt8        ATR_Wi;                 // see ISO7816-3
    UInt8        ATR_TA1;                // see ISO7816-3
    UInt8        ATR_TAi_T15;            // see ISO7816-3
    UInt8        PTS_fd_value;           // PTS, F(i)(bit8,7,6,5) and D(i)(bit4,3,2,1)
    UInt8        ATR_TB_LIB;             // see ISO7816-3, first TB after T=15

    // other variables
    SIMIO_RAW_ATR_INFO_t  atr_info;      // saved raw ATR excluding PTS
    Boolean      ATR_Wrong_Voltage;
    Boolean      ATR_Corrupted;
    SimATR_t     ATR_Index;              // ATR data index used by state machine    
    UInt8        TS_Char;                // Initial character to determine encoding
    UInt8        Ti;                     // iterator
    UInt8        ATR_Protocol;           // ATR protocol(T) for this iteration
    Boolean      TCK_Present;            // Check Char. Presetn Flag
    UInt8        Checksum;               // Checksum of ATR response
    UInt8        K_Nibble;               // Number of Historical Characters 
    UInt8        Y_Nibble;               // Interface Char. Presence Indicator
    UInt8        ATR_PTS_HighSpeedTry;
    Boolean      ATR_OK;                 // Result of ATR processing
    Boolean      PTS_OK;                 // Result of PTS
    Boolean      ATR_PTS_Done;           // Indicate ATR and PTS is done
    Boolean      T0_Support;             // Indicate whether T=0 protocol is supported
    Boolean      T1_Support;             // Indicate whether T=1 protocols is supported
} SIMIO_ATR_PRIV_t;

#define SIMIO_ATR_NUM    2
static SIMIO_ATR_PRIV_t atr_private_data[SIMIO_ATR_NUM];
static SIMIO_ATR_t atr[SIMIO_ATR_NUM];

//******************************************************************************
//      Variable
//******************************************************************************

/* The following tables contain the characters that need to be sent to the SIM
 * in a PTS (explicit Protocol Type Selection) request. See Section 7.4 of
 * ISO/IEC 7816-3. 
 *
 * Pts_Req_F372_D1 - Select default speed: F = 372, D = 1 (slowest).
 * Pts_Req_F512_D8 - Select enhanced speed: F = 512, D = 8.
 * Pts_Req_F512_D16 - Select enhanced speed: F = 512, D = 16. 
 * Pts_Req_F512_D32 - Select enhanced speed: F = 512, D = 32 (fastest).
 * Pts_Req_F512_D64 - Select enhanced speed: F = 512, D = 64 (fastest).
 * 
 * The first byte is PTSS, the initial character of 0xFF. The second byte is
 * PTS0, the format character. The last character is PCK, the check character
 * so that the XOR of all bytes are 0x00. 
 */

#if !(defined(CONFIG_BCM96838)) && !(defined(CONFIG_BCM96848)) && !(defined(CONFIG_BCM96858))
static const UInt8 Pts_Req_F372_D1[] = {PTSS_CHAR, PTS0_INDICATE_NO_PTS1, (PTSS_CHAR ^ PTS0_INDICATE_NO_PTS1)};
static const UInt8 Pts_Req_F372_D1_PPS2_LIB[] = {PTSS_CHAR, PTS0_INDICATE_PTS2_EXIST, PTS2_LIB, (PTSS_CHAR^PTS0_INDICATE_PTS2_EXIST^PTS2_LIB)};

static const UInt8 Pts_Req_F512_D8[] = 
{PTSS_CHAR, PTS0_INDICATE_PTS1_EXIST, PTS1_F512_D8, (PTSS_CHAR ^ PTS0_INDICATE_PTS1_EXIST ^ PTS1_F512_D8)};
static const UInt8 Pts_Req_F512_D8_PPS2_LIB[] = 
{PTSS_CHAR, PTS0_INDICATE_PTS1PTS2_EXIST, PTS1_F512_D8, PTS2_LIB, (PTSS_CHAR ^ PTS0_INDICATE_PTS1PTS2_EXIST ^ PTS2_LIB ^ PTS1_F512_D8)};

static const UInt8 Pts_Req_F512_D16[] = 
{PTSS_CHAR, PTS0_INDICATE_PTS1_EXIST, PTS1_F512_D16, (PTSS_CHAR ^ PTS0_INDICATE_PTS1_EXIST ^ PTS1_F512_D16)};
static const UInt8 Pts_Req_F512_D16_PPS2_LIB[] = 
{PTSS_CHAR, PTS0_INDICATE_PTS1PTS2_EXIST, PTS1_F512_D16, PTS2_LIB, (PTSS_CHAR ^ PTS0_INDICATE_PTS1PTS2_EXIST ^ PTS2_LIB ^ PTS1_F512_D16)};

static const UInt8 Pts_Req_F512_D32[] = 
{PTSS_CHAR, PTS0_INDICATE_PTS1_EXIST, PTS1_F512_D32, (PTSS_CHAR ^ PTS0_INDICATE_PTS1_EXIST ^ PTS1_F512_D32)};
static const UInt8 Pts_Req_F512_D32_PPS2_LIB[] = 
{PTSS_CHAR, PTS0_INDICATE_PTS1PTS2_EXIST, PTS1_F512_D32, PTS2_LIB, (PTSS_CHAR ^ PTS0_INDICATE_PTS1PTS2_EXIST ^ PTS2_LIB ^ PTS1_F512_D32)};
#endif
#ifdef SIMIO_SPEED_512_64_SUPPORT
static const UInt8 Pts_Req_F512_D64[] = 
{PTSS_CHAR, PTS0_INDICATE_PTS1_EXIST, PTS1_F512_D64, (PTSS_CHAR ^ PTS0_INDICATE_PTS1_EXIST ^ PTS1_F512_D64)};
static const UInt8 Pts_Req_F512_D64_PPS2_LIB[] = 
{PTSS_CHAR, PTS0_INDICATE_PTS1PTS2_EXIST, PTS1_F512_D64, PTS2_LIB, (PTSS_CHAR ^ PTS0_INDICATE_PTS1PTS2_EXIST ^ PTS2_LIB ^ PTS1_F512_D64)};
#endif

/* Some SIM's have trouble supporting high speed. We need to disable high speed to work with these SIM's */
static const UInt8 RS_Usim_Atr[] = /* RS T=1 test USIM */
{
	0x3B, 0xFF, 0x95, 0x00, 0xFF, 0xC0, 0x0A, 0xF1, 0xFE, 0x45, 
	0x00, 0x1F, 0x43, 0x80, 0x31, 0xE0, 0x73, 0xF6, 0x21, 0x13, 
	0x57, 0x4A, 0x33, 0x48, 0x61, 0x32, 0x41, 0x47, 0x9C
};

//--- following table is for inverse convention
static const UInt8 Rotate_Table[16] =
{
    0x00,       // 0x00
    0x08,       // 0x01
    0x04,       // 0x02
    0x0C,       // 0x03
    0x02,       // 0x04
    0x0A,       // 0x05
    0x06,       // 0x06
    0x0E,       // 0x07
    0x01,       // 0x08
    0x09,       // 0x09
    0x05,       // 0x0A
    0x0D,       // 0x0B
    0x03,       // 0x0C
    0x0B,       // 0x0D
    0x07,       // 0x0E
    0x0F        // 0x0F
};

typedef struct
{
	const UInt8 *atr_ptr;
	UInt8 atr_len;
} HIGH_SPEED_DISABLED_SIM_ATR_t;

static const HIGH_SPEED_DISABLED_SIM_ATR_t High_Speed_Disabled_Sim_Atr[] =
{
	{ RS_Usim_Atr,	sizeof(RS_Usim_Atr) }
};

/* Some SIM's have trouble supporting T=1. We need to select T=0 to work with these SIM's */
static const UInt8 GD_Usim_Atr[] = /* GD HD65146C T=1 USIM */
{
	0x3B, 0xFF, 0x95, 0x00, 0xFF, 0xC0, 0x0A, 0xF1, 0xFE, 0x44, 
	0x00, 0x1F, 0x43, 0x80, 0x31, 0xE0, 0x73, 0xF6, 0x21, 0x13, 
	0x57, 0x4A, 0x55, 0x48, 0x60, 0x32, 0x41, 0x00, 0xBD
};

typedef struct
{
	const UInt8 *atr_ptr;
	UInt8 atr_len;
} T1_DISABLED_SIM_ATR_t;

static const T1_DISABLED_SIM_ATR_t T1_Disabled_Sim_Atr[] =
{
	{ GD_Usim_Atr,	sizeof(GD_Usim_Atr) }
};


//******************************************************************************
//
// Function Name: _ATR_GetPtsValue
//
// Description: This function returns the high speed value based on the passed 
//				PTS1 byte received from the SIM card. 
//
// Return: SIM high speed value. If PTS1 value is incorrect, PTS_NOT_REQ is returned. 
//
//******************************************************************************
static PTS_t _ATR_GetPtsValue(UInt8 pts1)
{
    PTS_t pts_value;

    switch (pts1)
    {
    case PTS1_F512_D8:
        pts_value = PTS_512_8;
        break;

    case PTS1_F512_D16:
        pts_value = PTS_512_16;
        break;

    case PTS1_F512_D32:
        pts_value = PTS_512_32;
    break;

#ifdef SIMIO_SPEED_512_64_SUPPORT
    case PTS1_F512_D64:
        pts_value = PTS_512_64;
        break;
#endif

    default:
        pts_value = PTS_NOT_REQ;
        break;
    }

    return pts_value;
}


//******************************************************************************
//
// Function Name: _ATR_ChkPckRsp
//
// Description: This function checks if PCK in the PTS response received from
//              the SIM is correct. According to Section 7.4 of ISO/IEC 7816-3, 
//              the SIM needs to send back the same PCK value in the PTS request
//              sent to the SIM. 
//
// Return: TRUE if PCK is correct; FALSE otherwise.
//
//******************************************************************************
static Boolean _ATR_ChkPckRsp(PTS_t pts_value, PROTOCOL_t protocol, UInt8 pck, Boolean pts2)
{
    Boolean result;
    UInt8 pts0_indicate_no_pts1;
    UInt8 pts0_indicate_pts1_exist;

    if (protocol != SIM_PROTOCOL_T1)      // T=0 Protocol
    {
        if (!pts2)
        {
            pts0_indicate_no_pts1 = PTS0_INDICATE_NO_PTS1;
            pts0_indicate_pts1_exist = PTS0_INDICATE_PTS1_EXIST;
        }
        else
        {
            pts0_indicate_no_pts1 = PTS0_INDICATE_PTS2_EXIST ^ PTS2_LIB;
            pts0_indicate_pts1_exist = PTS0_INDICATE_PTS1PTS2_EXIST ^ PTS2_LIB;
        }
    }
    else // T=1 Protocol
    {
        if (!pts2)
        {
            pts0_indicate_no_pts1 = PTS0_INDICATE_T1_NO_PTS1;
            pts0_indicate_pts1_exist = PTS0_INDICATE_T1_PTS1_EXIST;
        }
        else
        {
            pts0_indicate_no_pts1 = PTS0_INDICATE_T1_PTS2_EXIST ^ PTS2_LIB;
            pts0_indicate_pts1_exist = PTS0_INDICATE_T1_PTS1PTS2_EXIST ^ PTS2_LIB;   
        }
    }
    switch(pts_value)
    {
    case PTS_372_1:
        result = (pck == (PTSS_CHAR ^ pts0_indicate_no_pts1));
        break;

    case PTS_512_8:
        result = (pck == (PTSS_CHAR ^ pts0_indicate_pts1_exist ^ PTS1_F512_D8));
        break;

    case PTS_512_16:
        result = (pck == (PTSS_CHAR ^ pts0_indicate_pts1_exist ^ PTS1_F512_D16));
        break;

    case PTS_512_32:
        result = (pck == (PTSS_CHAR ^ pts0_indicate_pts1_exist ^ PTS1_F512_D32));
        break;

#ifdef SIMIO_SPEED_512_64_SUPPORT
    case PTS_512_64:
        result = (pck == (PTSS_CHAR ^ pts0_indicate_pts1_exist ^ PTS1_F512_D64));
        break;
#endif

    default:
        /* We should not receive any PCK response from SIM */
        result = FALSE;
        break;
    }

    return result;
}


//******************************************************************************
//
// Function Name: _ATR_IsCorrectVoltage
//
// Description: This function checks if voltage indicator in ATR is allowed by
//              the activated voltage. 
//
// Return: TRUE if voltage is correct; FALSE otherwise.
//
//******************************************************************************
static Boolean _ATR_IsCorrectVoltage(SimVoltageLevel_t Sim_Voltage,
                                     UInt8 class_indicator_u)
{
    Boolean result;

    SIM_LOGV("_ATR_IsCorrectVoltage:",Sim_Voltage);
                    
    switch(class_indicator_u)
    {
    case UI_CLASS_A:    // 5V
        result = (Sim_Voltage == SIM_5V);
        break;

    case UI_CLASS_B:    // 3V
        result = (Sim_Voltage == SIM_3V);
        break;

    case UI_CLASS_A_B:  // 5V and 3V
        result = ((Sim_Voltage == SIM_3V) || (Sim_Voltage == SIM_5V));
        break;
    
    case UI_CLASS_B_C:  // 3V and 1.8V
        result = ((Sim_Voltage == SIM_3V) || (Sim_Voltage == SIM_1P8V));
        break;

    case UI_CLASS_RFU:
    default:
        result = TRUE;
        break;
    }

    return result;
	
}



//******************************************************************************
//
// Function Name: _ATR_IsPtsRequired
//
// Description: This function checks if PTS request needs to be sent to the
//              SIM. If PTS request needs to be sent, it also loads the PTS
//              request data into the SIM output buffer and requests to send
//              out the data. 
//
//
// Return: TRUE if PTS request needs to be sent; FALSE otherwise. 
//
//******************************************************************************
static Boolean _ATR_IsPtsRequired(SIMIO_ATR_t *handle)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;
    Boolean pts_required = TRUE;

    switch (priv->PTS_Required)
    {
    case PTS_NOT_REQ:
    default:
        _DBG_ATR_( SIM_LOG("SIMIO speed: MS does not need to send PTS request") );

        pts_required = FALSE;
        break;

    case PTS_372_1:
        _DBG_ATR_( SIM_LOG("SIMIO speed: MS requests default speed in PTS request") );

        if ((priv->ATR_TB_LIB & PTS2_LIB) != PTS2_LIB)
        {
            memcpy(priv->PTSBuffer, Pts_Req_F372_D1, sizeof(Pts_Req_F372_D1));
            priv->PTSLen = sizeof(Pts_Req_F372_D1);
        }
        else
        {
            memcpy(priv->PTSBuffer, Pts_Req_F372_D1_PPS2_LIB, sizeof(Pts_Req_F372_D1_PPS2_LIB));
            priv->PTSLen = sizeof(Pts_Req_F372_D1_PPS2_LIB);
        }
        break;

    case PTS_512_8:
        _DBG_ATR_( SIM_LOG("SIMIO speed: MS requests F = 512, D = 8 in PTS request") );

        if ((priv->ATR_TB_LIB & PTS2_LIB) != PTS2_LIB)
        {
            memcpy(priv->PTSBuffer, Pts_Req_F512_D8, sizeof(Pts_Req_F512_D8));
            priv->PTSLen = sizeof(Pts_Req_F512_D8);
        }
        else
        {
            memcpy(priv->PTSBuffer, Pts_Req_F512_D8_PPS2_LIB, sizeof(Pts_Req_F512_D8_PPS2_LIB));
            priv->PTSLen = sizeof(Pts_Req_F512_D8_PPS2_LIB);
        }
        priv->ATR_PTS_HighSpeedTry++;
        break;

    case PTS_512_16:
        _DBG_ATR_( SIM_LOG("SIMIO speed: MS requests F = 512, D = 16 in PTS request") );

        if ((priv->ATR_TB_LIB & PTS2_LIB) != PTS2_LIB)
        {
            memcpy(priv->PTSBuffer, Pts_Req_F512_D16, sizeof(Pts_Req_F512_D16));
            priv->PTSLen = sizeof(Pts_Req_F512_D16);
        }
        else
        {
            memcpy(priv->PTSBuffer, Pts_Req_F512_D16_PPS2_LIB, sizeof(Pts_Req_F512_D16_PPS2_LIB));
            priv->PTSLen = sizeof(Pts_Req_F512_D16_PPS2_LIB);
        }
        priv->ATR_PTS_HighSpeedTry++;
        break;

    case PTS_512_32:
        _DBG_ATR_( SIM_LOG("SIMIO speed: MS requests F = 512, D = 32 in PTS request") );

        if ((priv->ATR_TB_LIB & PTS2_LIB) != PTS2_LIB)
        {
            memcpy(priv->PTSBuffer, Pts_Req_F512_D32, sizeof(Pts_Req_F512_D32));
            priv->PTSLen = sizeof(Pts_Req_F512_D32);
        }
        else
        {
            memcpy(priv->PTSBuffer, Pts_Req_F512_D32_PPS2_LIB, sizeof(Pts_Req_F512_D32_PPS2_LIB));
            priv->PTSLen = sizeof(Pts_Req_F512_D32_PPS2_LIB);
        }
        priv->ATR_PTS_HighSpeedTry++;
        break;

#ifdef SIMIO_SPEED_512_64_SUPPORT
    case PTS_512_64:
        _DBG_ATR_( SIM_LOG("SIMIO speed: MS requests F = 512, D = 64 in PTS request") );

        if ((priv->ATR_TB_LIB & PTS2_LIB) != PTS2_LIB)
        {
            memcpy(priv->PTSBuffer, Pts_Req_F512_D64, sizeof(Pts_Req_F512_D64));
            priv->PTSLen = sizeof(Pts_Req_F512_D64);
        }
        else
        {
            memcpy(priv->PTSBuffer, Pts_Req_F512_D64_PPS2_LIB, sizeof(Pts_Req_F512_D64_PPS2_LIB));
            priv->PTSLen = sizeof(Pts_Req_F512_D64_PPS2_LIB);            
        }
        priv->ATR_PTS_HighSpeedTry++;
        break;
#endif

    }
    
    if(priv->T1_Support == TRUE)
    {       
        if(priv->PTS_Required == PTS_NOT_REQ)
        {
            priv->PTS_Required = PTS_372_1;
            if ((priv->ATR_TB_LIB & PTS2_LIB) != PTS2_LIB)
            {
                memcpy(priv->PTSBuffer, Pts_Req_F372_D1, sizeof(Pts_Req_F372_D1));
                priv->PTSLen = sizeof(Pts_Req_F372_D1);
            }
            else
            {
                memcpy(priv->PTSBuffer, Pts_Req_F372_D1_PPS2_LIB, sizeof(Pts_Req_F372_D1_PPS2_LIB));
                priv->PTSLen = sizeof(Pts_Req_F372_D1_PPS2_LIB);                
            }
        }
        // Replace (bit4,3,2,1) for T=1 negotiation
        priv->PTSBuffer[1] |= PTS0_INDICATE_T1_SUPPORT;   

        if(priv->PTSLen == 3)
        {
            priv->PTSBuffer[2] = priv->PTSBuffer[0] ^ priv->PTSBuffer[1];
        }
        else if(priv->PTSLen == 4)/* Must be ATR_proc_parm.pps_len == 4 */
        {
            priv->PTSBuffer[3] =  priv->PTSBuffer[0]^
                            priv->PTSBuffer[1]^
                            priv->PTSBuffer[2];// parity check calculation
        }
        else   /* we have PTS2 so the length can be 5 bytes */
        {
            priv->PTSBuffer[4] =  priv->PTSBuffer[0]^
                            priv->PTSBuffer[1]^
                            priv->PTSBuffer[2]^
                            priv->PTSBuffer[3];// parity check calculation
        }
        
        
        // Set PROTOCOL=T1              
        priv->Current_Protocol = SIM_PROTOCOL_T1; 
        pts_required = TRUE;
    }
    
    if(!pts_required)
        priv->PTSLen = 0;

    return pts_required;
}


//******************************************************************************
//
// Function Name: _ATR_IsHighSpeedDisabledSim
//
// This function returns TRUE if high speed needs to be disabled for the inserted
// SIM. These SIM's are typically found to have problems with high speed even though
// their ATR indicates they support high speed. 
//
//******************************************************************************
static Boolean _ATR_IsHighSpeedDisabledSim(const SIMIO_ATR_t *handle)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;
	UInt8 i;

	for (i = 0; i < (sizeof(High_Speed_Disabled_Sim_Atr) / sizeof(HIGH_SPEED_DISABLED_SIM_ATR_t)); i++)
	{
		if ( (priv->atr_info.len == High_Speed_Disabled_Sim_Atr[i].atr_len) && 
			 (memcmp(priv->atr_info.data, High_Speed_Disabled_Sim_Atr[i].atr_ptr, priv->atr_info.len) == 0) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

//******************************************************************************
//
// Function Name: _ATR_DecideSimSpeed
//
// Description: This function returns the SIM speed the MS will use after the
//              PTS (explicit Protocol Type Selection) precedure. 
// 
// UInt8 ta1 - The TA1 characer (third character of ATR data) received. 
//             Per Section 6.1.4.4 of ISO/IEC 7816-3, F occupies the upper
//             nibble (most significant 4 bits) of TA1 and D occupies the lower nibble. 
//
//******************************************************************************
static void _ATR_DecideSimSpeed(SIMIO_ATR_t *handle)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;
    PTS_t pts_sim_speed;

	if ( _ATR_IsHighSpeedDisabledSim(handle) )
	{
		_DBG_ATR_( SIM_LOG("SIMIO speed: high speed disabled") );
		pts_sim_speed = (priv->ATR_TA1 == 0x11) ? PTS_NOT_REQ : PTS_372_1;
		priv->FDratio = 372;
	}
	else
	{
		switch(priv->ATR_TA1)
		{
		case 0x11:
			/* SIM card wants our default speed: F = 372, D = 1, F/D=372 */
			_DBG_ATR_( SIM_LOG("SIMIO speed: default speed is requested by SIM card!") );

			if ((priv->ATR_TB_LIB & PTS2_LIB) != PTS2_LIB)
			    pts_sim_speed = PTS_NOT_REQ;
            else
                pts_sim_speed = PTS_372_1;
            
			priv->FDratio = 372;
			break;

		case 0x94:
			/* SIM card wants enhanced speed: F = 512, D = 8, F/D = 64, 
			 * almost 5 times faster than the default speed (372/64 = about 6). 
			 */
			_DBG_ATR_( SIM_LOG("SIMIO speed: F/D = 64 is requested by SIM card!") );
			pts_sim_speed = PTS_512_8;
			priv->FDratio = 64;                     
			break;

		case 0x95:
			/* SIM card wants enhanced speed: F = 512, D = 16, F/D = 32, 
			 * almost 11 times faster than the default speed (372/32 = about 12). 
			 */
			_DBG_ATR_( SIM_LOG("SIMIO speed: F/D = 32 is requested by SIM card!") );
			pts_sim_speed = PTS_512_16;
			priv->FDratio = 32; 
			break;

		case 0x96:
			/* SIM card wants enhanced speed: F = 512, D = 32, F/D = 16, 
			 * almost 22 times faster than the default speed (372/16 = about 23). 
			 */
			_DBG_ATR_( SIM_LOG("SIMIO speed: F/D = 16 is requested by SIM card!") );
			pts_sim_speed = PTS_512_32; 
			priv->FDratio = 16;
			break;

		case 0x97: 
			/* SIM card wants enhanced speed: F = 512, D = 64, F/D = 8, 
			 * almost 47 times faster than the default speed (372/8 = about 47). 
			 */
			_DBG_ATR_( SIM_LOG("SIMIO speed: F/D = 8 is requested by SIM card!") );
#ifdef SIMIO_SPEED_512_64_SUPPORT
			pts_sim_speed = PTS_512_64; 
			priv->FDratio = 8;
#else
			pts_sim_speed = PTS_512_32; 
			priv->FDratio = 16;
#endif			
			break;

		default:
			_DBG_ATR_( SIM_LOGV("SIMIO speed: unsupported speed is requested by SIM card: ", priv->ATR_TA1) );
			pts_sim_speed = PTS_372_1;
			priv->FDratio = 372;
			break;
		}
	}

    priv->PTS_Required = pts_sim_speed;
}

//******************************************************************************
//
// Function Name: _ATR_IsT1DisabledSim
//
// This function returns TRUE if T=1 protocol needs to be disabled for the inserted
// SIM. These SIM's are typically found to have problems with T=1 even though
// their ATR indicates they support it. 
//
//******************************************************************************
static Boolean _ATR_IsT1DisabledSim(const SIMIO_ATR_t *handle)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;
	UInt8 i;

	SIM_LOG("_ATR_IsT1DisabledSim: Check if T=1 disabled");
	  
	for (i = 0; i < (sizeof(T1_Disabled_Sim_Atr) / sizeof(T1_DISABLED_SIM_ATR_t)); i++)
	{
		if ( (priv->atr_info.len == T1_Disabled_Sim_Atr[i].atr_len) && 
			 (memcmp(priv->atr_info.data, T1_Disabled_Sim_Atr[i].atr_ptr, priv->atr_info.len) == 0) )
		{
			return TRUE;
		}
	}

	return FALSE;
}


//******************************************************************************
//
// Function Name: _ATR_Index_Change
//
// Description: This function will findout next Index
//
// Notes:
// If the SIM does not answer the PTS request within the initial waiting time the ME shall reset the SIM. After
// two failed PTS attempts using F=512 and D=8 or values indicated in TA1, (no PTS response from the
// SIM) the ME shall initiate PTS procedure using default values. If this also fails (no PTS response from the
// SIM) the ME may proceed using default values without requesting PTS
// If the SIM does not support the values requested by the ME, the SIM shall respond to the PTS request
// indicating the use of default values.
//
//******************************************************************************
static Boolean _ATR_Index_Change(SIMIO_ATR_t *handle) 
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;
    Boolean ATR_IndexDone = FALSE;

    while(ATR_IndexDone == FALSE)
    {
        switch(priv->ATR_Index)
        {   
            case SimATR_TA:
                if( IS_TA_PRESENT(priv->Y_Nibble) )
                {
                    ATR_IndexDone = TRUE;
                }
                else
                {
                    priv->ATR_Index = SimATR_TB;
                }
                
                break;
            
            case SimATR_TB:
                if( IS_TB_PRESENT(priv->Y_Nibble) )
                {
                    ATR_IndexDone = TRUE;
                }
                else
                {
                    priv->ATR_Index = SimATR_TC;
                }
                
                break;
            
            case SimATR_TC:
                if( IS_TC_PRESENT(priv->Y_Nibble) )
                {
                    ATR_IndexDone = TRUE;
                }
                else
                {
                    priv->ATR_Index = SimATR_TD;
                }
                
                break;
            
            case SimATR_TD:
                if( IS_TD_PRESENT(priv->Y_Nibble) )
                {
                    ATR_IndexDone = TRUE;
                }
                else
                {
                    priv->ATR_Index = SimATR_THC;
                }
                
                break;

            case SimATR_THC:
                if(priv->K_Nibble-- != 0)
                {
                    ATR_IndexDone = TRUE;
                }
                else
                {
                    priv->ATR_Index = SimATR_TCK;
                }
                
                break;

            case SimATR_TCK:
                if(priv->TCK_Present)
                {
                    ATR_IndexDone = TRUE;
                }
                else
                {
                    priv->ATR_Index = SimATR_PTS;
                }

                break;

            case SimATR_PTS:
				priv->atr_info.atr_valid = TRUE;
 
                if(priv->ATR_PTS_HighSpeedTry == (ATR_PTS_HIGHSPEEDTRY_MAX-1))
                {
                    _DBG_ATR_(SIM_LOG("SIMIO High speed fail, set to default speed")) ;
                    priv->ATR_PTS_HighSpeedTry++;
                    priv->PTS_Required = PTS_372_1;
                } 
				else if (priv->ATR_PTS_HighSpeedTry == ATR_PTS_HIGHSPEEDTRY_MAX)
                {   
                    _DBG_ATR_(SIM_LOG("SIMIO: Try fouth time. Now do not PPS, just continue with default speed"));
                    priv->PTS_Required = PTS_NOT_REQ;                 
                }
				else if (priv->ATR_TA1 != 0)
				{
					_ATR_DecideSimSpeed(handle);
				}

                /* Revert to T=0 if this SIM is known to have problems T=1 although it supports it */
				if(_ATR_IsT1DisabledSim(handle))
				{
					priv->ATR_Protocol = 0;
					priv->T0_Support = TRUE;
					priv->T1_Support = FALSE;
					SIM_LOG("_ATR_IsT1DisabledSim: Revert to T=0 protocol");
				}

                if( !_ATR_IsPtsRequired(handle) )
                {
                    priv->ATR_PTS_Done = TRUE;
                    return FALSE;   
                }
                
                priv->ATR_Index = SimATR_PTSS;
                ATR_IndexDone = TRUE;
                
                break;

            default:
                break;
        }
    }

    return TRUE;
}


/********* Below are interface APIs *****************************************/

//******************************************************************************
//
// Function Name: SIMIO_ATR_Detect_Invers
//
// Description: detect Invesre encoding token
//
// Return: TRUE if Inverse token is found; FALSE otherwise.
//
//******************************************************************************
Boolean SIMIO_ATR_Detect_Inverse(SIMIO_ATR_t *handle, UInt8 token)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;

    if ( (priv->ATR_Index == SimATR_TS) && ((UInt8) token == INVERSE) )
    {
        priv->TS_Char = INVERSE;

        _DBG_ATR_(SIM_LOG("INVERSE token received"));
        
        // don't advance the state machine
        return TRUE;
    }
    return FALSE;
}


//******************************************************************************
//
// Function Name: SIMIO_ATR_Convert
//
// Description: Convert data based on encoding scheme. 
//              See ISO/IEC 7816-3 section 6.1.4 
//
// Return: None. In-Place conversion.
//
//******************************************************************************
void SIMIO_ATR_Convert(const SIMIO_ATR_t *handle, UInt8 *buffer, UInt16 size)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;

    if (priv->TS_Char == INVERSE) 
    {
        while (size-- > 0)
        {
            *buffer ^= 0xff;
            *buffer = ((Rotate_Table[*buffer & 0x0F] << 4) | (Rotate_Table[*buffer >> 4]));

            //SIM_LOGV("SIMIO Convert data: ", *value);

            buffer++;
        }
    }
}


//******************************************************************************
//
// Function Name: SIMIO_ATR_Get_PTS
//
// Description: Get PTS bytes into the passed buffer  
//
// Return: PTS data in bytes
//
//******************************************************************************
UInt16 SIMIO_ATR_Get_PTS(const SIMIO_ATR_t *handle, UInt8* buffer, UInt16 size)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;
    UInt16 len;
    
    len = priv->PTSLen;
    if (len > size)
        len = size;
    
    memcpy(buffer, priv->PTSBuffer, len);
    
    return len;    
}


//******************************************************************************
//
// Function Name: SIMIO_ATR_Started
//
// Description: Check if ATR process is started  
//
// Return: TRUE if ATR is started FALSE otherwise
//
//******************************************************************************
Boolean SIMIO_ATR_Started(const SIMIO_ATR_t *handle)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;

    return (!(priv->ATR_Index == SimATR_TS || priv->ATR_Index == SimATR_NOT_START));    
}


//******************************************************************************
//
// Function Name: SIMIO_ATR_Passed
//
// Description: Check if ATR process is passed  
//
// Return: TRUE if ATR is passed FALSE otherwise
//
//******************************************************************************
Boolean SIMIO_ATR_Passed(const SIMIO_ATR_t *handle)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;

    return (priv->ATR_Index == SimATR_PASSED);    
}


//******************************************************************************
//
// Function Name: SIMIO_ATR_Check_Error
//
// Description: Check ATR process final status  
//
// Return: error code
//
//******************************************************************************
SIMATR_ERROR_t SIMIO_ATR_Check_Error(const SIMIO_ATR_t *handle)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;

    if (priv->ATR_Wrong_Voltage)
        return SIMATR_ERROR_WRONG_VOLTAGE;
    else if (priv->ATR_Corrupted)
        return SIMATR_ERROR_CORRUPTED;
    return SIMATR_ERROR_NONE;
}


//******************************************************************************
//
// Function Name: SIMIO_ATR_Get_ClkStpMode
//
// Description: Get native (ATR-based) clock stop mode 
//
// Return: native clock stop mode
//
//******************************************************************************
SIMIO_CLOCK_STOP_MODE_t SIMIO_ATR_Get_ClkStpMode(const SIMIO_ATR_t *handle)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;
    return priv->ClockStopMode;    
}


//******************************************************************************
//
// Function Name: SIMIO_ATR_Change_ClkStpMode
//
// Description: Negotiate a new clock stop mode 
//
// Return: Negotiated clock stop mode
//
//******************************************************************************
SIMIO_CLOCK_STOP_MODE_t SIMIO_ATR_Change_ClkStpMode(const SIMIO_ATR_t *handle,
                                                SIMIO_CLOCK_STOP_MODE_t mode)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;
    SIMIO_CLOCK_STOP_MODE_t new_mode = priv->ClockStopMode;
    
    switch (mode)
    {
        case SIMIO_CLOCK_STOP_ALLOWED:
        case SIMIO_NO_PREFERRED_LEVEL:
            if ((!priv->ClockStopModeExistInAtr) || (new_mode == SIMIO_CLOCK_STOP_NOT_ALLOWED))
            {
                new_mode = SIMIO_NO_PREFERRED_LEVEL;                
            }
            /* Else: use the ATR clock stop mode setting */
            break;
                
        case SIMIO_HIGH_LEVEL_PREFERRED:
			if (priv->ClockStopModeExistInAtr)
			{
				if ( (new_mode == SIMIO_NO_PREFERRED_LEVEL) || (new_mode == SIMIO_CLOCK_STOP_ALLOWED) || (new_mode == SIMIO_CLOCK_STOP_NOT_ALLOWED) )
				{
					new_mode = SIMIO_HIGH_LEVEL_PREFERRED;
				}
				/* Else: use the ATR clock stop mode setting */
			}
			else
			{
				new_mode = SIMIO_HIGH_LEVEL_PREFERRED;
			}
			break;

        case SIMIO_CLOCK_STOP_ONLY_HIGH:
            if (priv->ClockStopModeExistInAtr)
            {
               if (new_mode != SIMIO_LOW_LEVEL_PREFERRED)
	       {
               		new_mode = SIMIO_CLOCK_STOP_ONLY_HIGH;
	       }
            }
            else
            {
                new_mode = SIMIO_CLOCK_STOP_ONLY_HIGH;
            }
            break;
                
        case SIMIO_LOW_LEVEL_PREFERRED:
			if (priv->ClockStopModeExistInAtr)
			{
				if ( (new_mode == SIMIO_NO_PREFERRED_LEVEL) || (new_mode == SIMIO_CLOCK_STOP_ALLOWED) ||
                     		     (new_mode == SIMIO_LOW_LEVEL_PREFERRED) || (new_mode == SIMIO_CLOCK_STOP_NOT_ALLOWED) )
				{
					new_mode = SIMIO_LOW_LEVEL_PREFERRED;
				}
				/* Else: use the ATR clock stop mode setting */
			}
			else
			{
				new_mode = SIMIO_LOW_LEVEL_PREFERRED;
			}
			break;

        case SIMIO_CLOCK_STOP_ONLY_LOW:
            if (priv->ClockStopModeExistInAtr)
            {
                if (new_mode != SIMIO_HIGH_LEVEL_PREFERRED)
                {
			new_mode = SIMIO_CLOCK_STOP_ONLY_LOW;
		}
            }
            else
            {
                new_mode = SIMIO_CLOCK_STOP_ONLY_LOW;
            }
            break;
                    
        case SIMIO_CLOCK_NOT_DEFINED:
            if (!priv->ClockStopModeExistInAtr)
            {
                /* Neither ATR or the response of "USIM-ADF" selection indicates any Clock Stop Mode
                 * setting, we just assume Clock Stop Mode can not be enabled. 
                 */
                new_mode = SIMIO_CLOCK_STOP_NOT_ALLOWED;
            }
            /* Else: use the ATR clock stop mode setting */

            break;
                    
        case SIMIO_CLOCK_STOP_NOT_ALLOWED:
        default:
	    if ((!priv->ClockStopModeExistInAtr) || (new_mode == SIMIO_CLOCK_STOP_NOT_ALLOWED))
            {
                 new_mode = SIMIO_CLOCK_STOP_NOT_ALLOWED; 
	    }
	    /* Else: use the ATR clock stop mode setting */   
            break;
    }
    
    return new_mode;
}


//******************************************************************************
//
// Function Name: SIMIO_ATR_Process_A_Byte
//
// Description: Process a byte based on specification. This function runs a state
//              machine to processes one byte at a time for the Answer-To-Reset
//              message. It returns an action code to the caller. 
//
// Return: action code for the caller to act on.
//
//******************************************************************************
SIMATR_ACTION_t SIMIO_ATR_Process_A_Byte(SIMIO_ATR_t *handle, UInt8 rcvd_byte)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;
    SIMATR_ACTION_t action = SIMATR_ACTION_NOP;
    PTS_t pts_value;

    _DBG_ATR_( SIM_LOGV("SIMIO ATR:", rcvd_byte) );

	if (priv->ATR_Index == SimATR_NOT_START)
	{
        priv->TS_Char = DIRECT;
        priv->Current_Protocol = SIM_PROTOCOL_T0;     // initial protocol
        priv->T0_Support = TRUE;              // Initialize T0_support TRUE
        priv->T1_Support = FALSE;             // Initialize T1_support FALSE

        priv->ATR_Received_Voltage = FALSE;
        priv->ClockStopModeExistInAtr = FALSE;
        priv->ClockStopMode = SIMIO_CLOCK_STOP_NOT_ALLOWED;

        priv->Ti = 1;
        priv->ATR_Wi = 10;
        priv->ATR_ExtGuardTime = 0;
        priv->ATR_T1_EDC = T1_EPILOGUE_LRC;           // 1=one byte LRC
        priv->ATR_T1_IFSC = 32;
        priv->ATR_T1_CWI = 13;                        // Default Character Waiting Time Index
        priv->ATR_T1_BWI = 4;                         // Default Block Waiting Time Index

        priv->ATR_TA1 = 0;
        priv->ATR_TB_LIB = 0;

        priv->atr_info.len = 0;
        priv->atr_info.atr_valid = FALSE;


        priv->ATR_Wrong_Voltage = FALSE;
        priv->ATR_Corrupted = FALSE;

        priv->FDratio = 372;
        priv->PTS_Required = PTS_NOT_REQ;     // Initialize PTS_Required
        priv->TCK_Present = FALSE;            // Initialize TCK present to FALSE
        priv->ATR_OK = TRUE;                  // Initialize ATR OK TRUE
        priv->PTS_OK = TRUE;                  // Initialize PTS OK TRUE
        priv->Checksum = 0;                   // Checksum of ATR response
        
        priv->ATR_PTS_HighSpeedTry = 0;

        
        priv->ATR_PTS_Done = FALSE;           // Initialize ATR and PTS is done
        priv->ATR_Index = SimATR_TS;
        return action;
	}

	if (!priv->atr_info.atr_valid)
	{
		priv->atr_info.data[priv->atr_info.len++] = rcvd_byte;
	}

    // Either of them shorcuts this fuction
    if((priv->ATR_Corrupted == TRUE) || (priv->ATR_Wrong_Voltage == TRUE))
    {
        _DBG_ATR_( SIM_LOGV("SIMIO ATR is Corrupted/WrongVoltage", rcvd_byte) );
        return action;
    }

    switch(priv->ATR_Index)
    {
        case SimATR_TS:
            if(rcvd_byte == DIRECT)
            {
                _DBG_ATR_( SIM_LOGV("SIMIO Even Parity", rcvd_byte) );
                // This is default
                //chal_simio_set_parity(chal_handle, 0);
            }
            else if((rcvd_byte == INVERSE) || rcvd_byte==0x3F)
            {
                _DBG_ATR_( SIM_LOGV("SIMIO ODD Parity", rcvd_byte) );
            }
            else
            {
                _DBG_ATR_( SIM_LOGV("SIMIO Not Odd-Even : goto reset", rcvd_byte) );
                // not needed. task does it anyway
                //WATCH_FOR_RECEIVING_OFF();
                priv->ATR_Corrupted = TRUE;
                return action;
            }

            // Let's start ATR processing and expect T0 byte
            priv->ATR_Index = SimATR_T0;

            break;

        case SimATR_T0:
            priv->Checksum ^= rcvd_byte;          // Start checksum
            priv->nof_hist_ch = priv->K_Nibble = rcvd_byte & 0x0F;  // Get number of historical characters
            priv->Y_Nibble = rcvd_byte >> 4;      // Get Presnece Indicator Mask
            
            priv->ATR_Index = SimATR_TA;
            _ATR_Index_Change(handle);
            _DBG_ATR_( SIM_LOGV("SIMIO T0=", rcvd_byte) );
            
            break;

        case SimATR_TA:         // ISO 7816-3( 6.1.4.4 )
            priv->Checksum ^= rcvd_byte;
            
            if(priv->Ti == 1)
            {
                priv->ATR_TA1 = rcvd_byte;
            }
            else if(priv->ATR_Protocol == 1)
			{   
                priv->ATR_T1_IFSC = rcvd_byte;
                _DBG_ATR_( SIM_LOGV("SIM ATR_T1_IFSC", priv->ATR_T1_IFSC) );
            }
            else if(priv->ATR_Protocol == 0x0F)
            { 
                /* 1. The ATR carries voltage information. This is mandatory for USIM and optional for 2G SIM. 
                 * This means if ATR does not have voltage information, it must be a 2G SIM.
                 *
                 * 2. Can not simply configure the Clock Stop Mode by the information coming from ATR.
                 * For some USIM, after USIM-ADF has been selected, it is possible to get the different clock stop information. 
                 * If there is discrepancy, we disallow Clock Stop Mode. It is more appropriate to set the Clock Stop Mode 
                 * as attribute as USIM file characteristics, not ATR.
                 */
                priv->ATR_Received_Voltage = TRUE;
                priv->ATR_TAi_T15 = rcvd_byte;

                priv->ClockStopModeExistInAtr = TRUE;

                // "rcvd_byte >> 6": Clock Stop(X), ISO/IEC 7816-3 6.5.5
                switch (rcvd_byte >> 6)
                {
                    case 0:                                                         // SIMIO_CLOCK_STOP_NOT_ALLOWED
                        priv->ClockStopMode = SIMIO_CLOCK_STOP_NOT_ALLOWED;
                        break;
                    case 1:                                                         // SIMIO_CLOCK_STOP_ONLY_LOW
                        priv->ClockStopMode = SIMIO_LOW_LEVEL_PREFERRED;
                        break;
                    case 2:                                                         // SIMIO_CLOCK_STOP_ONLY_HIGH
                        priv->ClockStopMode = SIMIO_HIGH_LEVEL_PREFERRED;
                        break;
                    case 3:                                                         // SIMIO_NO_PREFERRED_LEVEL
                        priv->ClockStopMode = SIMIO_NO_PREFERRED_LEVEL;
                        break;
                    default:
                        priv->ClockStopModeExistInAtr = FALSE;
                        priv->ClockStopMode = SIMIO_CLOCK_STOP_NOT_ALLOWED;
                        break;
                }
                
                // "rcvd_byte & 0x3f": Voltage Class(U), ISO/IEC 7816-3 6.5.5
                if( !_ATR_IsCorrectVoltage(priv->Sim_Voltage, rcvd_byte & 0x3f) )
                {
                    _DBG_ATR_(SIM_LOGV("SIMIO ATR Wrong voltage: ", rcvd_byte & 0x3f));
                    priv->ATR_Wrong_Voltage = TRUE;
                    return action;                                                  // Wait for RESET pin to low
                }

                _DBG_ATR_( SIM_LOGV4("SIMIO ClockStopMode|ClockStopModeExistInAtr", 
                           priv->ClockStopMode, priv->ClockStopModeExistInAtr, 0, 0) );
            }

            priv->ATR_Index = SimATR_TB;
            _ATR_Index_Change(handle);
            
            break;

        case SimATR_TB:
            priv->Checksum ^= rcvd_byte;
            
            if( (priv->Ti == 1) && (PI1(rcvd_byte) != 0) )
            {
                _DBG_ATR_(SIM_LOG("SIMIO PI1 Invalid"));
                priv->ATR_OK = FALSE;
            }
            else if(priv->ATR_Protocol == 1) {                    //T=1 
                priv->ATR_T1_CWI = rcvd_byte & 0x0F;
                priv->ATR_T1_BWI = rcvd_byte >>4;
                _DBG_ATR_( SIM_LOGV("SIMIO ATR_T1_BWI/CWI", rcvd_byte) );
            } 
            else if(priv->ATR_Protocol == 0x0F) {
                _DBG_ATR_( SIM_LOGV("SIMIO ATR_LIB", rcvd_byte) );
                priv->ATR_TB_LIB = rcvd_byte;
                if (rcvd_byte == PTS2_NO_INTERFACE)
                    _DBG_ATR_( SIM_LOG("SIMIO ATR_LIB: not supported TBi") );
            } 

            priv->ATR_Index = SimATR_TC;
            _ATR_Index_Change(handle);

            break;

        case SimATR_TC:
            priv->Checksum ^= rcvd_byte;

            if(priv->Ti == 1)
            {
                if ( (rcvd_byte != 0) && (rcvd_byte != 0xFF) )
                {
                    /* GSM test case 27.11.1.3a.3 (N=35) of 51.010
                     * We only support character delay of 12 ETU, which is the default.
                     */
                    _DBG_ATR_(SIM_LOGV("ATR not supports N=", rcvd_byte));
                    priv->ATR_Corrupted = TRUE;
                    return action;
                }
                else    // Good Case
                {
                    priv->ATR_ExtGuardTime = rcvd_byte;   //set the extra guard time
                }
            }
            else
            {
                if(priv->ATR_Protocol == 1) {                     // T=1 
                    priv->ATR_T1_EDC = rcvd_byte & 0x01;
                    _DBG_ATR_( SIM_LOGV("SIMIO ATR_T1_EDC", priv->ATR_T1_EDC) );
                }
                else if( priv->Ti == 2 )                          //TC(2)=WI for T=0
                {
                    priv->ATR_Wi = rcvd_byte;
                }
            }
            
            priv->ATR_Index = SimATR_TD;
            _ATR_Index_Change(handle);
            
            break;

        case SimATR_TD:
            priv->Checksum ^= rcvd_byte;

            // Get Presence Indicator Mask for Next set of interface characters
            priv->Y_Nibble = rcvd_byte >> 4;      
                                
            priv->ATR_Protocol = rcvd_byte & 0x0F;

			if ((priv->pref_protocol == SIM_PROTOCOL_T0) && (priv->ATR_Protocol == 1))
			    priv->ATR_Protocol = 0;

            /* Section 6.4.3 of ISO/IEC 7816-3 says T=15 is not allowed in TD1. 
             * GCF test case 5.1.5.5 of 3GPP 102.230. 
             */
            if ( (priv->Ti == 1) && (priv->ATR_Protocol == 15) )
            {
                _DBG_ATR_(SIM_LOG("SIMIO ATR T=15 in TD1"));
                priv->ATR_Corrupted = TRUE;
                return action;
            }

            if (priv->ATR_Protocol == 0) {
                priv->T0_Support = TRUE;
                _DBG_ATR_( SIM_LOG("SIMIO T0 Support SIM") );
            }
            else if(priv->ATR_Protocol == 1) 
            {
                priv->T1_Support = TRUE;
                _DBG_ATR_( SIM_LOG("SIMIO T1 Support SIM") );
            }
            // TCK is present if more than T=0 is indicated
            if( (rcvd_byte & 0x0F) != 0 )
            {
                priv->TCK_Present = TRUE;
            }

            priv->Ti++;
            priv->ATR_Index = SimATR_TA;
            _ATR_Index_Change(handle);
            
            break;
            
        case SimATR_THC:
            priv->Checksum ^= rcvd_byte;
            
			if ((priv->nof_hist_ch - priv->K_Nibble -1) < MAX_HIST_CHARACTERS)
                priv->historial_characters[priv->nof_hist_ch - priv->K_Nibble -1] = rcvd_byte;
            
			_ATR_Index_Change(handle);
            
            break;

        case SimATR_TCK:
            if(priv->Checksum != rcvd_byte)
            {
                _DBG_ATR_( SIM_LOG("SIMIO ATR Checksum Failed") );
                priv->ATR_OK = FALSE;
                priv->ATR_Corrupted = TRUE;
                return action;             
            }
            
#if 0
            // This was added to support Mega SIM and it shortcuts ISO SIM PPS
            if(priv->ATR_Protocol == 15)
            {
                 priv->ATR_PTS_Done = TRUE;
                priv->PTS_Required = PTS_NOT_REQ;   
            }
            else
#endif            
            {
                priv->ATR_Index = SimATR_PTS;
                _ATR_Index_Change(handle);                
            }
            
            break;

        case SimATR_PTSS:
            _DBG_ATR_(SIM_LOG("SIMIO PPS received"));
            action = SIMATR_ACTION_PTS_RECEIVE;
        
            _DBG_ATR_( SIM_LOGV("SIMIO speed: received char in SimATR_PTSS state: ", rcvd_byte) );

            if(rcvd_byte != PTSS_CHAR)
            {
                _DBG_ATR_( SIM_LOGV("SIMIO speed: received wrong char in SimATR_PTSS state!!!!", rcvd_byte) );

                priv->ATR_Corrupted = TRUE;
                return action;
                //PTS_OK = FALSE;
            }

            priv->ATR_Index = SimATR_PTS0;
            
            break;          

        case SimATR_PTS0:
            _DBG_ATR_( SIM_LOGV("SIMIO speed: received char in SimATR_PTS0 state: ", rcvd_byte) );

			if ( (rcvd_byte == PTS0_INDICATE_NO_PTS1) || (rcvd_byte == PTS0_INDICATE_T1_NO_PTS1) ||
			     (rcvd_byte == PTS0_INDICATE_PTS2_EXIST) || (rcvd_byte == PTS0_INDICATE_T1_PTS2_EXIST) )
            {
                if (priv->PTS_Required != PTS_372_1)
				{
                    _DBG_ATR_( SIM_LOG("SIMIO PTS0: SIM changes speed back to default"));
                    priv->PTS_Required = PTS_372_1;
                }
                else
                {
                    _DBG_ATR_( SIM_LOG("SIMIO PTS0: default speed OK"));
                }

                priv->ATR_Index = SimATR_PCK;

				if ((rcvd_byte == PTS0_INDICATE_PTS2_EXIST) || (rcvd_byte == PTS0_INDICATE_T1_PTS2_EXIST))
				    // expect PTS2 before PCK
				    priv->ATR_Index = SimATR_PTS2;
            }
			else if ( (rcvd_byte == PTS0_INDICATE_PTS1_EXIST) || (rcvd_byte == PTS0_INDICATE_T1_PTS1_EXIST) ||
			          (rcvd_byte == PTS0_INDICATE_PTS1PTS2_EXIST) || (rcvd_byte == PTS0_INDICATE_T1_PTS1PTS2_EXIST) )
			{
                if (priv->PTS_Required == PTS_372_1)
                {
                    _DBG_ATR_( SIM_LOG("SIMIO PTS0: to keep at default speed"));
                }
                else
                {
                    _DBG_ATR_( SIM_LOG("SIMIO PTS0: high speed OK"));
                }

                priv->ATR_Index = SimATR_PTS1;
            }
            else
            {
                _DBG_ATR_(SIM_LOGV("SIMIO PTS0: wrong value: ", priv->PTS_Required));
                priv->ATR_Corrupted = TRUE;
                return action;
            }

            break;

        case SimATR_PTS1:
            _DBG_ATR_( SIM_LOGV("SIMIO speed: received char in SimATR_PTS1 state: ", rcvd_byte) );

            pts_value = _ATR_GetPtsValue(rcvd_byte);
            if (pts_value == PTS_NOT_REQ)
            {
                _DBG_ATR_(SIM_LOGV("SIMIO PTS1: wrong value: ", priv->PTS_Required));
                priv->ATR_Corrupted = TRUE;
                return action;
            }
            else if (pts_value != priv->PTS_Required)
            {
                _DBG_ATR_(SIM_LOGV4("SIMIO PTS1: SIM changes high speed: ", pts_value, priv->PTS_Required, 0, 0));
				priv->PTS_Required = pts_value;
            }
            else
            {
                _DBG_ATR_(SIM_LOG("SIMIO PTS1: OK"));
            }

            if ((priv->ATR_TB_LIB & PTS2_LIB) != PTS2_LIB)
                priv->ATR_Index = SimATR_PCK;
            else
                priv->ATR_Index = SimATR_PTS2;
            
            break;
            
        case SimATR_PTS2:
            _DBG_ATR_( SIM_LOGV("SIMIO LIB: received char in SimATR_PTS2 state: ", rcvd_byte) );

			if (rcvd_byte != PTS2_LIB)
			{
				_DBG_ATR_(SIM_LOGV("SIMIO PTS2: wrong value, expect: ", PTS2_LIB));
				priv->PTS_OK = FALSE;
			}
			else
			{
				_DBG_ATR_(SIM_LOG("SIMIO PTS2 LIB: OK"));
			}

            priv->ATR_Index = SimATR_PCK;
            
            break;

        case SimATR_PCK:
            _DBG_ATR_( SIM_LOGV("SIMIO speed: received char in SimATR_PCK state: ", rcvd_byte) );

            if( !_ATR_ChkPckRsp(priv->PTS_Required, priv->Current_Protocol, rcvd_byte, (priv->ATR_TB_LIB & PTS2_LIB) == PTS2_LIB ) )
            {
                _DBG_ATR_( SIM_LOG("SIMIO speed: received wrong char in SimATR_PCK state!!!!") );
                priv->PTS_OK = FALSE;
                priv->Current_Protocol = SIM_PROTOCOL_T0;
            }
            else
            {
                _DBG_ATR_(SIM_LOG("SIMIO PCK: OK"));
            }

            priv->ATR_PTS_Done = TRUE;
            
            break;

        default:
            break;
    }

    if (priv->ATR_Index == SimATR_PTSS && priv->PTSLen != 0)
    {
        action = SIMATR_ACTION_PTS_SEND;
    }
    else
    if(priv->ATR_PTS_Done)
    {
        if(priv->ATR_OK && priv->PTS_OK)
        {
            // tell the caller to configure hardware accordingly 
            action = SIMATR_ACTION_CONFIG_HW; 
            
            // ME and SIM interface will use new speed from now on (GSM11.11 5.8.2)
            
            switch(priv->PTS_Required)
            {
                case PTS_512_8:
                    priv->PTS_fd_value=0x94;                  
                    break;  
                case PTS_512_16:
                    priv->PTS_fd_value=0x95;                  
                    break;
                case PTS_512_32:
                    priv->PTS_fd_value=0x96;                  
                    break;
#ifdef SIMIO_SPEED_512_64_SUPPORT
                case PTS_512_64:
                    priv->PTS_fd_value=0x97;                  
                    break;
#endif                    
                case PTS_NOT_REQ:
                case PTS_372_1:
                default:
                    priv->PTS_fd_value=0x01;
                    break;				
            }

            priv->ATR_PTS_HighSpeedTry = 0;
            priv->ATR_Index = SimATR_PASSED;
            priv->atr_info.atr_valid = TRUE;
		}
        else
        {
            priv->ATR_Corrupted = TRUE;
            _DBG_ATR_( SIM_LOGV4("SIMIO(ATR): Done with error", priv->ATR_OK, priv->PTS_OK, 0, 0) );
        }
    }

    return action;
}


//******************************************************************************
//
// Function Name: SIMIO_ATR_Get_Params
//
// Description: Get key ATR parameters
//
// Return: none
//
//******************************************************************************
void SIMIO_ATR_Get_Params(const SIMIO_ATR_t *handle, SIMIO_USIMAP_ATR_PARAM_t *ATR_Param)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;

    memset(ATR_Param, 0, sizeof(SIMIO_USIMAP_ATR_PARAM_t));

    ATR_Param->TS_Char = priv->TS_Char;
    ATR_Param->ATR_Received_Voltage = priv->ATR_Received_Voltage;
    ATR_Param->ATR_TAi_T15 = priv->ATR_TAi_T15;
    ATR_Param->PTS_Required = priv->PTS_Required;
    
    // SIM protocol
    if (priv->Current_Protocol == SIM_PROTOCOL_T1)
    {
        ATR_Param->is_T1_protocol = TRUE;
        ATR_Param->atr_T1_ifsc = priv->ATR_T1_IFSC;
        ATR_Param->atr_T1_edc = priv->ATR_T1_EDC;        
        ATR_Param->atr_T1_cwi = priv->ATR_T1_CWI;
        ATR_Param->atr_T1_bwi = priv->ATR_T1_BWI;        
    }
    else
        ATR_Param->is_T1_protocol = FALSE;
    

    // F and D
    ATR_Param->FDratio = priv->FDratio;
    ATR_Param->d_used = (priv->PTS_fd_value & 0x0f);
    ATR_Param->f_used = priv->PTS_fd_value >> 4;

    // historical character
    ATR_Param->nof_hist_characters = priv->nof_hist_ch;
    ATR_Param->hist_characters = (char *) priv->historial_characters;

    ATR_Param->ATR_Wi = priv->ATR_Wi;
    ATR_Param->ATR_TA1 = priv->ATR_TA1;
    ATR_Param->ATR_ExtGuardTime = priv->ATR_ExtGuardTime;
    
}



//******************************************************************************
//
// Function Name: SIMIO_ATR_Get_Raw_ATR
//
// Description: Get raw ATR data
//
// Return: raw ATR
//
//******************************************************************************
SIMIO_RAW_ATR_INFO_t *SIMIO_ATR_Get_Raw_ATR(const SIMIO_ATR_t *handle)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;

	return (&(priv->atr_info));    
}


//******************************************************************************
//
// Function Name: SIMIO_ATR_Start
//
// Description: Start SIMIO ATR processor
//
// Return: none
//
//******************************************************************************
void SIMIO_ATR_Start(SIMIO_ATR_t *handle, 
                     SimVoltageLevel_t voltage, PROTOCOL_t pref_protocol)
{
    UInt8 id;
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;
    
    //save
    id = handle->id;
    
    // clean the objects
    memset(handle, 0, sizeof(SIMIO_ATR_t));
    memset(priv, 0, sizeof(SIMIO_ATR_PRIV_t));

    // restore
    handle->id = priv->id = id;
    priv->used = TRUE;
    
    // save opertional env
    priv->Sim_Voltage = voltage;
    priv->pref_protocol = pref_protocol;
    
    // binding
    handle->priv = (void *)priv;

    priv->ATR_Index = SimATR_NOT_START;

}


//******************************************************************************
//
// Function Name: SIMIO_ATR_Cleanup
//
// Description: De-Initialize SIMIO ATR processor
//
// Return: none
//
//******************************************************************************
void SIMIO_ATR_Cleanup(SIMIO_ATR_t *handle)
{
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv;

    // clear everything
    memset(handle, 0, sizeof(SIMIO_ATR_t));
    memset(priv, 0, sizeof(SIMIO_ATR_PRIV_t));
    
    // mark it free
    priv->used = FALSE;
}


//******************************************************************************
//
// Function Name: SIMIO_ATR_Init
//
// Description: Initialize SIMIO ATR processor
//
// Return: the handle that other API need to pass
//
//******************************************************************************
SIMIO_ATR_t *SIMIO_ATR_Init(UInt8 index)
{
    SIMIO_ATR_t *handle = NULL;
    
     _DBG_ATR_(SIM_LOGV("SIMIO_ATR_Init", index));

    if (index >= SIMIO_ATR_NUM)
    {
        _DBG_ATR_(SIM_LOG("SIMIO_ATR_Init: bad index!"));
        return handle;
        
    }
    
    if (atr_private_data[index].used)
    {
        _DBG_ATR_(SIM_LOG("SIMIO_ATR_Init: busy!"));
        return handle;        
    }
    
    // Zero the objects
    memset(&atr[index], 0, sizeof(SIMIO_ATR_t));
    memset(&atr_private_data[index], 0, sizeof(SIMIO_ATR_PRIV_t));

    atr_private_data[index].id = index;
    atr_private_data[index].used = TRUE;
    atr[index].id = index;

    handle = &atr[index];
    handle->priv = &atr_private_data[index];
    
    return handle;
}


//****************************************************************************** 
// 
// Function Name:   SIMIO_ATR_Is_UICC_CLF_Interface_Support 
// 
// Description:     Check if the "UICC-CLF" Interface is Supported. 
// 
// Boolean:	        1: UICC-CLF Interface Supported. 
//         	        0: UICC-CLF Interface NOT Supported. 
// 
//****************************************************************************** 
Boolean SIMIO_ATR_Is_UICC_CLF_Interface_Support(const SIMIO_ATR_t *handle) 
{ 
    SIMIO_ATR_PRIV_t* priv = (SIMIO_ATR_PRIV_t *)handle->priv; 
     
    if((priv->ATR_TB_LIB & PTS2_UICC_CLF) == PTS2_UICC_CLF)  
        return TRUE; // Supported. 
 
    return FALSE; // Not Supported. 
} 

