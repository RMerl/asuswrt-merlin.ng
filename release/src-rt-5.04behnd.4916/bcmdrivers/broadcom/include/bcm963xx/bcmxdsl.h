/*
<:copyright-BRCM:2002:DUAL/GPL:standard

   Copyright (c) 2002 Broadcom 
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
//**************************************************************************
// File Name  : bcmxdsl.h
//
// Description: This file contains the definitions, structures and function
//              prototypes for XDSL status info.
//
//**************************************************************************
#if !defined(_BCMXDSL_H_)
#define _BCMXDSL_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* Incldes. */
#include "VdslInfoDef.h"

//**************************************************************************
// Type Definitions
//**************************************************************************

// Return status values
typedef enum BcmXdslStatus
{
    BCMXDSL_STATUS_SUCCESS = 0,
    BCMXDSL_STATUS_ERROR
} BCMXDSL_STATUS;

// Return status values
typedef enum XdslLinkState
{
    BCM_XDSL_LINK_UP = 0,
    BCM_XDSL_LINK_DOWN,
	BCM_XDSL_TRAINING_G992_EXCHANGE,
	BCM_XDSL_TRAINING_G992_CHANNEL_ANALYSIS,
	BCM_XDSL_TRAINING_G992_STARTED,
	BCM_XDSL_TRAINING_G994,
	BCM_XDSL_G994_NONSTDINFO_RECEIVED,
    BCM_XDSL_BERT_COMPLETE,
	BCM_XDSL_ATM_IDLE,
    BCM_XDSL_EVENT,
    BCM_XDSL_G997_FRAME_RECEIVED,
    BCM_XDSL_G997_FRAME_SENT
} XDSL_LINK_STATE;

#define	XDSL_LINK_UP		BCM_XDSL_LINK_UP
#define	XDSL_LINK_DOWN		BCM_XDSL_LINK_DOWN

typedef enum XdslTrafficType
{
    BCM_XDSL_TRAFFIC_INACTIVE = 0,
    BCM_XDSL_TRAFFIC_ATM,
    BCM_XDSL_TRAFFIC_PTM   
} XDSL_TRAFFIC_TYPE;

// XDSL_CHANNEL_ADDR Contains XDSL Utopia PHY addresses
typedef struct XdslChannelAddr
{
    UINT16 usFastChannelAddr;
    UINT16 usInterleavedChannelAddr;
} XDSL_CHANNEL_ADDR, *PXDSL_CHANNEL_ADDR;

// XDSL_CONNECTION_INFO Contains XDSL Connection Info
typedef struct XdslConnectionInfo
{
	XDSL_LINK_STATE LinkState; 
	XDSL_TRAFFIC_TYPE TrafficType[2]; /* B0 and B1 */
    UINT32 ulUpStreamRate[2];         /* B0 and B1 */
    UINT32 ulDnStreamRate[2];         /* B0 and B1 */
} XDSL_CONNECTION_INFO, *PXDSL_CONNECTION_INFO;

/* OEM parameter definition */
#define XDSL_OEM_G994_VENDOR_ID       1    /* Vendor ID used during G.994 handshake */
#define XDSL_OEM_G994_XMT_NS_INFO     2    /* G.994 non-standard info field to send */
#define XDSL_OEM_G994_RCV_NS_INFO     3    /* G.994 received non-standard */
#define XDSL_OEM_EOC_VENDOR_ID        4    /* EOC reg. 0 */
#define XDSL_OEM_EOC_VERSION          5    /* EOC reg. 1 */
#define XDSL_OEM_EOC_SERIAL_NUMBER    6    /* EOC reg. 2 */
#define XDSL_OEM_T1413_VENDOR_ID      7    /* Vendor ID used during T1.413 handshake */
#define XDSL_OEM_T1413_EOC_VENDOR_ID  8    /* EOC reg. 0 (vendor ID) in T1.413 mode */

/* XMT gain definitions */
#define XDSL_XMT_GAIN_AUTO			  0x80000000

typedef struct
{
    short			x;
    short			y;
} XDSL_CONSTELLATION_POINT, *PXDSL_CONSTELLATION_POINT;

#define XDSL_CONSTEL_DATA_ID		0
#define XDSL_CONSTEL_PILOT_ID		1

typedef void (*XDSL_FN_NOTIFY_CB) (XDSL_LINK_STATE XdslLinkState, UINT32 ulParm); 

#if defined(__cplusplus)
}
#endif

#endif // _BCMXDSL_H_

