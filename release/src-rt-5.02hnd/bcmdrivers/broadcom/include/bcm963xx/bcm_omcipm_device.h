/*
<:copyright-broadcom

 Copyright (c) 2007 Broadcom Corporation
 All Rights Reserved
 No portions of this material may be reproduced in any form without the
 written permission of:
          Broadcom Corporation
          5300 California Avenue
          Irvine, California 92617
 All information contained in this document is Broadcom Corporation
 company private, proprietary, and trade secret.

:>
*/
#ifndef _BCM_OMCI_PM_DEVICE_
#define _BCM_OMCI_PM_DEVICE_

#include "bcm_omcipm_api.h"

/**
 * OMCI_PM Driver kernel API
 **/

/**
 * Typedefs.
 **/

/**
 * GEM Port performance monitoring
 *
 * Monitored Entity: GEM port network CTP
 **/

/* GEM Port PM History Data */
typedef struct 
{
    OUT   UINT32 lostPackets;          /*Partially Compliant: counts aggregate uncorrectable HEC. Not buffer overflows*/
    OUT   UINT32 misinsertedPackets;   /*Not Compliant - Always returns 0*/
    OUT   UINT32 receivedPackets;      /*Compliant*/
    OUT   UINT32 receivedBlocks;       /*Compliant*/
    OUT   UINT32 transmittedBlocks;    /*Compliant*/
    OUT   UINT32 impairedBlocks;       /*Not Compliant - Always returns 0*/
    OUT   UINT32 transmittedPackets;   /*Compliant - Non-standard Counter*/
} BCM_OMCI_PM_GEM_PORT_COUNTER, *PBCM_OMCI_PM_GEM_PORT_COUNTER;

/**
 * FEC performance monitoring history data
 *
 * Monitored Entity: ANI-G
 **/

/* FEC PM History Data */
typedef struct 
{
    OUT   UINT32 correctedBytes;         /*Not Compliant. Always returns 0*/
    OUT   UINT32 correctedCodeWords;     /*Compliant*/
    OUT   UINT32 uncorrectedCodeWords;   /*Compliant*/
    OUT   UINT32 totalCodeWords;         /*Compliant*/
    OUT   UINT32 fecSeconds;             /*Compliant*/
} BCM_OMCI_PM_FEC_COUNTER;

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
} BCM_OMCI_PM_ETHERNET_COUNTER;

/* Ethernet PM History Data 2 */
typedef struct 
{
    OUT   UINT32 pppoeFilterFrameCounter;
} BCM_OMCI_PM_ETHERNET_2_COUNTER;

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
} BCM_OMCI_PM_ETHERNET_3_COUNTER;

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
    OUT   UINT32 incomingOctets;
    OUT   UINT32 outgoingUnicastPackets;
    OUT   UINT32 outgoingDiscardedPackets;
    OUT   UINT32 outgoingErroredPackets;
    OUT   UINT32 outgoingUnknownPackets;
    OUT   UINT32 outgoingMulticastPackets;
    OUT   UINT32 outgoingBroadcastPackets;
    OUT   UINT32 outgoingOctets;
} BCM_OMCI_PM_MOCA_ETHERNET_COUNTER;

/**
 * MoCA interface performance monitoring
 *
 * Monitored entity: MoCA UNI
 **/

/* MoCA Interface PM History Data */
typedef struct 
{
    OUT   UINT32 phyTxBroadcastRate;
} BCM_OMCI_PM_MOCA_INTERFACE_COUNTER;

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
} BCM_OMCI_PM_MOCA_INTERFACE_ENTRY_COUNTER;

/**
 * GAL Ethernet performance monitoring
 *
 * Monitored Entity: GEM interworking TP
 **/

/* GAL Ethernet PM History Data */
typedef struct 
{
    OUT   UINT32 discardedFrames;     /*Not Compliant. Always returns 0*/
    OUT   UINT32 transmittedFrames;   /*Compliant - Non-standard Counter*/
    OUT   UINT32 receivedFrames;      /*Compliant - Non-standard Counter*/
} BCM_OMCI_PM_GAL_ETHERNET_COUNTER;

/**
 * MAC bridge performance monitoring history data
 *
 * Monitored Entity: MAC bridge
 **/

/* MAC bridge PM History Data */
typedef struct 
{
    OUT   UINT32 discardCount;
} BCM_OMCI_PM_MAC_BRIDGE_COUNTER;

/* OMCI PM Device ID values. */
typedef enum
{
    BCM_OMCI_PM_DEVICE_GPON=0,
    BCM_OMCI_PM_DEVICE_ENET,
    BCM_OMCI_PM_DEVICE_MOCA,
    BCM_OMCI_PM_DEVICE_BRIDGE,
    BCM_OMCI_PM_DEVICE_MAX
} BCM_OMCI_PM_DEVICE_ID;

typedef struct
{
    BCM_OMCI_PM_DEVICE_ID omcipmDeviceId;
    BCM_OMCI_PM_STATUS (*getCounters)(IN  UINT16 physPortId,
                                      OUT void   *counters,
                                      IN  BOOL   reset);
} BCM_OMCI_PM_DEVICE, *PBCM_OMCI_PM_DEVICE;

/**
 * Functions are exported by OMCI PM driver and used by other drivers
 **/

BCM_OMCI_PM_STATUS bcm_omcipm_dev_register(PBCM_OMCI_PM_DEVICE device);
BCM_OMCI_PM_STATUS bcm_omcipm_dev_unregister(BCM_OMCI_PM_DEVICE_ID deviceId);

#endif /*_BCM_OMCI_PM_DEVICE_*/
