/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/
#ifndef _OMCI_PM_API_
#define _OMCI_PM_API_

/**
 * OMCI_PM Driver user API
 **/

/**
 * Parameter Tags indicating whether the parameter is an input, output, or input/output argument
 **/

#ifndef IN
#define IN
#endif /*IN*/

#ifndef OUT
#define OUT
#endif /*OUT*/

#ifndef INOUT
#define INOUT
#endif /*INOUT*/


#define OMCIPM_LOG_NAME "omcipm"

#define OMCIPM_LOG_ERROR(fmt, arg...)                                     \
	printf("[ERROR " "%s" "] %-10s, %d:" fmt,   OMCIPM_LOG_NAME, __FUNCTION__, __LINE__, ##arg);


#define BCM_OMCI_MAC_STR_SIZE    6

/**
 * Typedefs.
 **/

/* Return status values. */
typedef enum OMCI_PM_Status
{
    OMCI_PM_STATUS_SUCCESS = 0,
    OMCI_PM_STATUS_INIT_FAILED,
    OMCI_PM_STATUS_ERROR,
    OMCI_PM_STATUS_LOAD_ERROR,
    OMCI_PM_STATUS_STATE_ERROR,
    OMCI_PM_STATUS_PARAMETER_ERROR,
    OMCI_PM_STATUS_ALLOC_ERROR,
    OMCI_PM_STATUS_RESOURCE_ERROR,
    OMCI_PM_STATUS_IN_USE,
    OMCI_PM_STATUS_NOT_FOUND,
    OMCI_PM_STATUS_NOT_SUPPORTED,
    OMCI_PM_STATUS_NOT_REGISTERED,
    OMCI_PM_STATUS_TIMEOUT
} BCM_OMCI_PM_STATUS;

/* OMCI PM Counter ID values. */
typedef enum
{
    BCM_OMCI_PM_CLASS_GEM_PORT=0,
    BCM_OMCI_PM_CLASS_FEC,
    BCM_OMCI_PM_CLASS_ENET,
    BCM_OMCI_PM_CLASS_ENET2,
    BCM_OMCI_PM_CLASS_ENET3,
    BCM_OMCI_PM_CLASS_ENETDN,
    BCM_OMCI_PM_CLASS_ENETUP,
    BCM_OMCI_PM_CLASS_MOCA_ENET,
    BCM_OMCI_PM_CLASS_MOCA_INTF,
    BCM_OMCI_PM_CLASS_GAL_ENET,
    BCM_OMCI_PM_CLASS_BRIDGE,
    BCM_OMCI_PM_CLASS_BRIDGE_PORT,
    BCM_OMCI_PM_CLASS_RTP,
    BCM_OMCI_PM_CLASS_IPHOST,
    BCM_OMCI_PM_CLASS_MAX
} BCM_OMCI_PM_CLASS_ID;

/**
 * GEM Port performance monitoring
 *
 * Monitored Entity: GEM port network CTP
 **/

/* GEM Port PM History Data */
typedef struct 
{
    OUT   UINT32 transmittedGEMFrames;        // Compliant - Non-standard Counter
    OUT   UINT32 receivedGEMFrames;           // Compliant
    OUT   UINT32 receivedPayloadBytes;        // Compliant
    OUT   UINT32 transmittedPayloadBytes;     // Compliant
} BCM_OMCI_PM_GEM_PORT_COUNTER, *PBCM_OMCI_PM_GEM_PORT_COUNTER;

/**
 * FEC performance monitoring history data
 *
 * Monitored Entity: ANI-G
 **/

/* FEC PM History Data */
typedef struct 
{
    OUT   UINT32 correctedBytes;          /*Compliant*/
    OUT   UINT32 correctedCodeWords;      /*Compliant*/
    OUT   UINT32 uncorrectedCodeWords;    /*Compliant*/
    OUT   UINT32 totalCodeWords;          /*Compliant*/
    OUT   UINT32 fecSeconds;              /*Compliant*/
} BCM_OMCI_PM_FEC_COUNTER, *PBCM_OMCI_PM_FEC_COUNTER;

/**
 * Ethernet performance monitoring
 *
 * Montored entity: Ethernet UNI
 **/

/* Ethernet PM History Data */
typedef struct 
{
    OUT   UINT32 fcsErrors;
    OUT   UINT32 excessiveCollisionCounter;
    OUT   UINT32 lateCollisionCounter;
    OUT   UINT32 frameTooLongs;
    OUT   UINT32 bufferOverflowsOnReceive;
    OUT   UINT32 bufferOverflowsOnTransmit;
    OUT   UINT32 singleCollisionFrameCounter;
    OUT   UINT32 multipleCollisionsFrameCounter;
    OUT   UINT32 sqeCounter;
    OUT   UINT32 deferredTransmissionCounter;
    OUT   UINT32 internalMacTransmitErrorCounter;
    OUT   UINT32 carrierSenseErrorCounter;
    OUT   UINT32 alignmentErrorCounter;
    OUT   UINT32 internalMacReceiveErrorCounter;
} BCM_OMCI_PM_ETHERNET_COUNTER, *PBCM_OMCI_PM_ETHERNET_COUNTER;

/* Ethernet PM History Data 2 */
typedef struct 
{
    OUT   UINT32 pppoeFilterFrameCounter;
} BCM_OMCI_PM_ETHERNET_2_COUNTER, *PBCM_OMCI_PM_ETHERNET_2_COUNTER;

/* Ethernet PM History Data 3 */
typedef struct 
{
    OUT   UINT32 dropEvents;
    OUT   UINT32 octets;
    OUT   UINT32 packets;
    OUT   UINT32 broadcastPackets;
    OUT   UINT32 multicastPackets;
    OUT   UINT32 undersizePackets;
    OUT   UINT32 fragments;
    OUT   UINT32 jabbers;
    OUT   UINT32 packets64Octets;
    OUT   UINT32 packets127Octets;
    OUT   UINT32 packets255Octets;
    OUT   UINT32 packets511Octets;
    OUT   UINT32 packets1023Octets;
    OUT   UINT32 packets1518Octets;
} BCM_OMCI_PM_ETHERNET_3_COUNTER, *PBCM_OMCI_PM_ETHERNET_3_COUNTER;

/* Ethernet PM History Data Upstream & Downstream */
typedef struct 
{
    OUT   UINT32 dropEvents;
    OUT   UINT32 octets;
    OUT   UINT32 packets;
    OUT   UINT32 broadcastPackets;
    OUT   UINT32 multicastPackets;
    OUT   UINT32 crcErroredPackets;
    OUT   UINT32 undersizePackets;
    OUT   UINT32 oversizePackets;
    OUT   UINT32 packets64Octets;
    OUT   UINT32 packets127Octets;
    OUT   UINT32 packets255Octets;
    OUT   UINT32 packets511Octets;
    OUT   UINT32 packets1023Octets;
    OUT   UINT32 packets1518Octets;
} BCM_OMCI_PM_ETHERNET_UPDN_COUNTER, *PBCM_OMCI_PM_ETHERNET_UPDN_COUNTER;

/**
 * MoCA ethernet performance monitoring
 *
 * Monitored entity: MoCA UNI
 **/

/* MoCA Ethernet PM History Data */
typedef struct 
{
    OUT   UINT32 incomingUnicastPackets;
    OUT   UINT32 incomingDiscardedPackets;
    OUT   UINT32 incomingErroredPackets;
    OUT   UINT32 incomingUnknownPackets;
    OUT   UINT32 incomingMulticastPackets;
    OUT   UINT32 incomingBroadcastPackets;
    OUT   UINT32 incomingOctets_hi;
    OUT   UINT32 incomingOctets_low;
    OUT   UINT32 outgoingUnicastPackets;
    OUT   UINT32 outgoingDiscardedPackets;
    OUT   UINT32 outgoingErroredPackets;
    OUT   UINT32 outgoingUnknownPackets;
    OUT   UINT32 outgoingMulticastPackets;
    OUT   UINT32 outgoingBroadcastPackets;
    OUT   UINT32 outgoingOctets_hi;
    OUT   UINT32 outgoingOctets_low;
} BCM_OMCI_PM_MOCA_ETHERNET_COUNTER, *PBCM_OMCI_PM_MOCA_ETHERNET_COUNTER;

/**
 * MoCA interface performance monitoring
 *
 * Monitored entity: MoCA UNI
 **/

/* MoCA Interface PM History Data */
typedef struct 
{
    OUT   UINT32 phyTxBroadcastRate;
    OUT   UINT32 phyTxRate;
    OUT   UINT32 txPowerControlReduction;
    OUT   UINT32 phyRxRate;
    OUT   UINT32 rxPowerLevel;
    OUT   UINT32 phyRxBroadcastRate;
    OUT   UINT32 rxBroadcastPowerLevel;
    OUT   UINT32 txPackets;
    OUT   UINT32 rxPackets;
    OUT   UINT32 erroredMissedRxPackets;
    OUT   UINT32 erroredRxPackets;
    OUT   UINT8  mac[BCM_OMCI_MAC_STR_SIZE];
} BCM_OMCI_PM_MOCA_INTERFACE_COUNTER, *PBCM_OMCI_PM_MOCA_INTERFACE_COUNTER;

/* MoCA Interface Entry PM History Data */
typedef struct 
{
    OUT   UINT32 phyTxRate;
    OUT   UINT32 txPowerControlReduction;
    OUT   UINT32 phyRxRate;
    OUT   UINT32 rxPowerLevel;
    OUT   UINT32 phyRxBroadcastRate;
    OUT   UINT32 rxBroadcastPowerLevel;
    OUT   UINT32 txPackets;
    OUT   UINT32 rxPackets;
    OUT   UINT32 erroredMissedRxPackets;
    OUT   UINT32 erroredRxPackets;
    OUT   UINT8  mac[BCM_OMCI_MAC_STR_SIZE];
} BCM_OMCI_PM_MOCA_INTERFACE_ENTRY_COUNTER, *PBCM_OMCI_PM_MOCA_INTERFACE_ENTRY_COUNTER;

/**
 * GAL Ethernet performance monitoring
 *
 * Monitored Entity: GEM interworking TP
 **/

/* GAL Ethernet PM History Data */
typedef struct 
{
    OUT   UINT32 discardedFrames;     /*Partially Compliant: not count erroneous FCS*/
    OUT   UINT32 transmittedFrames;   /*Compliant - Non-standard Counter*/
    OUT   UINT32 receivedFrames;      /*Compliant - Non-standard Counter*/
} BCM_OMCI_PM_GAL_ETHERNET_COUNTER, *PBCM_OMCI_PM_GAL_ETHERNET_COUNTER;

/**
 * MAC bridge performance monitoring history data
 *
 * Monitored Entity: MAC bridge
 **/

/* MAC bridge PM History Data */
typedef struct 
{
    OUT   UINT32 learningDiscaredEntries;   /*Not Compliant. Always returns 0*/
} BCM_OMCI_PM_MAC_BRIDGE_COUNTER, *PBCM_OMCI_PM_MAC_BRIDGE_COUNTER;

/**
 * MAC bridge port performance monitoring history data
 *
 * Monitored Entity: MAC bridge port
 **/

/* MAC bridge port PM History Data */
typedef struct 
{
    OUT   UINT32 forwardedFrames;
    OUT   UINT32 delayDiscardedFrames;
    OUT   UINT32 mtuDiscardedFrames;
    OUT   UINT32 receivedFrames;
    OUT   UINT32 receivedDiscardedFrames;
} BCM_OMCI_PM_MAC_BRIDGE_PORT_COUNTER, *PBCM_OMCI_PM_MAC_BRIDGE_PORT_COUNTER;

/**
 * RTP performance monitoring history data
 *
 * Monitored Entity: RTP
 **/

/* RTP PM History Data */
typedef struct 
{
    OUT   UINT32 rtpErrors;
    OUT   UINT32 packetLoss;
    OUT   UINT32 maxJitter;
    OUT   UINT32 maxTimeBetweenRtcpPackets;
    OUT   UINT32 bufferUnderflows;
    OUT   UINT32 bufferOverflows;
} BCM_OMCI_PM_RTP_COUNTER, *PBCM_OMCI_PM_RTP_COUNTER;

/**
 * ipHost performance monitoring history data
 *
 * Monitored Entity: ipHost
 **/

/* ipHost PM History Data */
typedef struct 
{
    OUT   UINT32 icmpErrors;
    OUT   UINT32 dnsErrors;
} BCM_OMCI_PM_IP_HOST_COUNTER, *PBCM_OMCI_PM_IP_HOST_COUNTER;

typedef BCM_OMCI_PM_STATUS (*PM_GET_RTP_STATS_CALLBACK)(UINT16 phyPortId,
  void *counterP);
typedef BCM_OMCI_PM_STATUS (*PM_GET_DNS_STATS_CALLBACK)(void *counterP);


/**
 * Function in OMCI PM driver API
 **/

BCM_OMCI_PM_STATUS bcm_omcipm_getCounters(BCM_OMCI_PM_CLASS_ID classId,
                                          UINT16 physPortId,
                                          void *counters);

BCM_OMCI_PM_STATUS bcm_omcipm_getCountersNext(BCM_OMCI_PM_CLASS_ID classId,
                                              UINT16 *physPortId,
                                              void *counters);

void bcm_omcipm_usrRtpStatsCbRegister(PM_GET_RTP_STATS_CALLBACK cbFuncP);

void bcm_omcipm_usrDnsStatsCbRegister(PM_GET_DNS_STATS_CALLBACK cbFuncP);


#endif /*_BCM_OMCI_PM_API_*/
