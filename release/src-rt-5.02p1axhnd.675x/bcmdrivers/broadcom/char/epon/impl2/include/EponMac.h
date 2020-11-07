/*
*  Copyright 2011, Broadcom Corporation
*
* <:copyright-BRCM:2011:proprietary:standard
* 
*    Copyright (c) 2011 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

/*                      Copyright(c) 2010 Broadcom                            */

#if !defined(Epon_h)
#define Epon_h
////////////////////////////////////////////////////////////////////////////////
/// \file TkOnuEpon.h
/// \brief Routines to manipulate Epon module
///
////////////////////////////////////////////////////////////////////////////////

#include "Ethernet.h"
#ifdef EPON_NORMAL_MODE
#include "PonConfigDb.h"
#endif
#include "Laser.h"
#include "EponStatId.h"
#include "drv_epon_epn_ag.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define EponUpstreamDisableWaitMs       6
#define EponUpstreamEnableWaitMs        2
#define MpcpNoGrantTime           1000000/262 // 1 second in 262us units

typedef U8 L1Index;
typedef U8 L2Index;
typedef U32 L1Map;
typedef U32 L2Map;


#define EponTopEpnR				0
#define EponTopLifR				1
#define EponTopNcoR				2
#define EponTopClkDivR			3
#define EponTopXifR             4
#define EponTopXpcsTxRst        5
#define EponTopXpcsRxRst        6
#define EponTopTodR             7

//below is the best value for 'ReportLengths1G' according to exprience
#define ModIpgPreambleBytes 0x14
#define CfgRptLen1G         0x2A
#define CfgRptLen10G        0x05
#define CfgRptLen10GFec     0x0D
#define CfgFecRptLen1G      0x39
#define CfgFecRptLen10G     0x05
#define CfgFecRptLen10GFec  0x0D
#define CfgFecIpgLength     0x0A

#define BbhQueStatDelay10G     0x40
#define BbhQueStatDelay1G     0x16
#define UpPacketFetchMargin 223

typedef enum
    {
    EPON_MAC_UNCONFIGURED,
    EPON_MAC_NORMAL_MODE,
    EPON_MAC_AE_MODE    
    } epon_mac_mode_e;
    
extern
epon_mac_mode_e EponGetMacMode(void);

extern
void EponSetMacMode(epon_mac_mode_e mac_mode);

extern
void EponMapInit(void);

extern 
void EponTopClearReset (U8 id,BOOL en);

extern
void EponTopInit(void);

////////////////////////////////////////////////////////////////////////////////
/// \brief  Returns MAC address of the given link
///
/// \param link     Link to query
/// \param mac      Pointer to return MAC addr to get
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponGetMac (LinkIndex link, MacAddr* mac);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Sets MAC address of the given link
///
/// \param link     Link to change
/// \param mac      Pointer to MAC addr to set
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetMac (LinkIndex link, const MacAddr* mac);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Sets MAC address for OLT
///
/// \param mac  Address to provision for OLT
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetOltMac (const MacAddr* mac);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Set burst cap for l2
///
/// The burst capacity registers limit the size of a Report FIFO. This creates
/// an upper limit on the queue size indicated in the report frames.
///
/// \param l2   L2 index to set the burst cap
/// \param cap  Max bytes per burst on this link
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetBurstCap (L2Index l2, U32 cap);


////////////////////////////////////////////////////////////////////////////////
/// \brief Set CTC burst limit for L2 queue
///
/// The burst limit registers set the maximum number of bytes that an L2 queue
/// can transmit in any given round. A round ends when all L2s queues have
/// reached their respective burst limit or there is no more data to transmit.
/// Note that setting a burst limit to zero enables the respective L2s queue to
/// transmit in strict priority. Also, any burst limits that are set to 1 will
/// cause those L2Map queues to transmit in "round-robin" fashion.
///
/// \param l2     Link for new burst cap
/// \param pct    Percent weight for this queue (0 for SP, -1 for RR)
/// \param base The smallest weight in use for any priority on the link.
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetBurstLimit(L2Index l2, U16 pct, U16 base);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Sets idle time for EPON
///
/// From the beginning a grant window, the ONT must turn on the laser, wait
/// for it to come on (idling), then continue to idle while waiting for the
/// OLT AGC time, then continue to idle while waiting for the OLT CDR time,
/// and then idle 600ns per IEEE802.3ah.  This routine sets the total idle
/// time from the end of the laser on period.
///
/// \param  front    Should correspond to Lon + Preamble
/// \param  back     Should correspond to Loff
/// \param  idleType It can be a discovery or non discovery idle time
///
/// \return
/// Actual grant overhead provisionned in hardware
////////////////////////////////////////////////////////////////////////////////
extern
U16 EponSetIdleTime (U16 front, U16 back, IdleTimeType idleType, U16 register_req_packet_size_in_tq);


////////////////////////////////////////////////////////////////////////////////
/// \brief delay from start of window to transmit
///
/// Usually, transmission begins when a grant window starts.  For replies
/// to discovery gates, though, a random delay needs to be added.
///
/// \param offset   Time delay, in 16-bit-time increments
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetGateOffset (U16 offset);


////////////////////////////////////////////////////////////////////////////////
/// \brief provision L1 Event Queue
///
/// See description to the function this one is calling and apply it to L1
///
/// \param  l1            Indicates the l1 queue we are talking about (0 based)
/// \param  sizeInFrames  Max number of frames in the queue
/// \param  startAddress  What is the current end address in the table
///
/// \return
/// next available address that can be used as start address for next l1
////////////////////////////////////////////////////////////////////////////////
extern
U16 EponSetL1EventQueue (L1Index l1, U32 sizeInFrames, U16 startAddress);


////////////////////////////////////////////////////////////////////////////////
/// \brief provision L1 Event Queue
///
/// See description to the function this one is calling and apply it to L1
///
/// \param  l2            Indicates the l2 queue we are talking about (0 based)
/// \param  sizeInFrames  Max number of frames in the queue
/// \param  startAddress  What is the current end address in the table
///
/// \return
/// next available address that can be used as start address for next l1
////////////////////////////////////////////////////////////////////////////////
extern
U16 EponSetL2EventQueue (L2Index l2, U32 sizeInFrames, U16 startAddress);


////////////////////////////////////////////////////////////////////////////////
/// \brief Flush the Grant FIFO for some links
///
///
/// \param linkMap  Map of links to flush grant FIFO
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponFlushGrantFifo (LinkMap linkMap);


////////////////////////////////////////////////////////////////////////////////
/// \brief Flush the Grant FIFO for some links
///
///
/// \param linkMap  Map of links to flush grant FIFO
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void EponFlushL2Fifo (U32 l2map, U32 l1map);


////////////////////////////////////////////////////////////////////////////////
/// \brief Flush the L2 FIFO repeatly
///
///
/// \param l2map  Map of l2 to flush
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponRepeatlyFlushL2Fifo(LinkMap links, U32 l2map, U32 l1map);


////////////////////////////////////////////////////////////////////////////////
extern
void EponGrantReset(LinkMap linkMap);


////////////////////////////////////////////////////////////////////////////////
extern
void EponGrantClearReset(LinkMap linkMap);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Enable grants for some links
///
/// \param linkMap  Bitmap of links
///
/// \return None
////////////////////////////////////////////////////////////////////////////////
extern
void EponGrantEnable(LinkMap linkMap);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Disable grants for some links
///
/// \param linkMap  Bitmap of links
///
/// \return None
////////////////////////////////////////////////////////////////////////////////
extern
void EponGrantDisable(LinkMap linkMap);


////////////////////////////////////////////////////////////////////////////////
/// \brief Disables "normal" reporting / traffic send on a link.
/// note this operation waits on the feedback register.
///
/// If you disable the upstream in order to change reporting threshold, you
/// need to wait after calling this function. How long will you ask?
///
/// We need to wait a little to be sure the tear down is taken into account
/// The worst case is that we are at the begining of the biggest burst ever
/// and we have 12 max frames in the EPON pipeline.
/// The biggest burst is 0xFFFF TQ which is a little over 1ms
/// 12 big frames correpond to 12Frames * 2048Bytes/Frame * 8bits/Byte / 1Gps
/// which equates to .2ms
/// So waiting 2ms is plenty enough.
///
/// \param linkMap  Bitmap of links to disable
///
/// \return None
////////////////////////////////////////////////////////////////////////////////
extern
void EponUpstreamDisable (LinkMap linkMap);


////////////////////////////////////////////////////////////////////////////////
/// \brief Enables "normal" reporting / traffic send on a link.
/// note this operation waits on the feedback register.
///
/// \param  l2Map bitmap of L2 to enable
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponUpstreamEnable (L2Map l2Map);


////////////////////////////////////////////////////////////////////////////////
/// \brief Enable or Disable RX path.
/// TRUE:  Enable Rx
/// FALSE: Disable Rx
///
/// \param flag: TRUE or FALSE
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern 
void EponEnableRx (BOOL flag);

////////////////////////////////////////////////////////////////////////////////
/// \brief Enable gates to go to processor for some given links
///
/// \param  linkMap Map of link index for which we want to allow gates
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponAllowGates (LinkMap linkMap);


////////////////////////////////////////////////////////////////////////////////
/// \brief Disable gates to go to processor for some given links
///
/// \param  linkMap Map of link index for which we want to disallow gates
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponDisallowGates (LinkMap links);


////////////////////////////////////////////////////////////////////////////////
/// \brief Get the map of links that had missing grants since last call
///
/// \param
/// None
///
/// \return
/// map of links that had missing grants since last call
////////////////////////////////////////////////////////////////////////////////
extern
LinkMap EponMissedGrants (void);


////////////////////////////////////////////////////////////////////////////////
/// \brief Get the map of links that received grants since last call
///
/// \param
/// None
///
/// \return
/// map of links that received grants since last call
////////////////////////////////////////////////////////////////////////////////
extern
LinkMap EponRcvdGrants (void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Reads a link stat to provided buffer
///
/// \param linkIndex    Link to retrieve stat for based on LIF/XIF mapping table
/// \param eponStat     Link stat to look at
/// \param dataPtr      buffer to write value to
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponReadLinkStat (LinkIndex linkIndex,
                       EponLinkStatId eponStat,
                       U64* dataPtr, BOOL isFake);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Reads a statistic
///
/// \param eponStat     Link stat to look at
///
/// \return
/// value of the statistic
////////////////////////////////////////////////////////////////////////////////
extern
U32 EponReadStat (U8 eponStat);

////////////////////////////////////////////////////////////////////////////////
/// \brief  tests whether local EPON clock is synced to network
///
/// Since this function directly reads and clear interrupts, it should only be
/// called by one caller. It is expected for this function to be called in a
/// polling loop.
///
/// \param force    TRUE to force a refresh of the sync state
///
/// \return
/// TRUE if in sync, FALSE otherwise.
////////////////////////////////////////////////////////////////////////////////
extern
BOOL EponInSync (BOOL force);

extern
BOOL EponAnyFatalEvent(void);

extern
BOOL EponBBHUpsHaultGet(void);

extern
void EponBBHUpsHaultClr(void);


extern
void EponBBHUpsHaultStatusClr(void);


////////////////////////////////////////////////////////////////////////////////
/// \brief  sets the grant interval register
///
/// \param time     Time to set the grant interval to
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetGrantInterval (U16 time);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Set correct EPON overhead depending on FEC tx setting
///
/// \param linkMap  Bitmap of links that have FEC tx enabled
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetFecTx (LinkMap linkMap);


////////////////////////////////////////////////////////////////////////////////
/// \brief  Epon needs to account for Aes overhead
///
/// \param  links   Bitmap of links on which we enable the AES
/// \param  mode    AES mode to set
///
/// \return None
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetAesTx (LinkMap links, EponAesMode mode);


////////////////////////////////////////////////////////////////////////////////
/// \brief Put L1 queues into reset
///
/// \param l1Map  Bitmap of L1 queues
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetL1Reset (L1Map l1Map);


////////////////////////////////////////////////////////////////////////////////
/// \brief Bring L1 queues out of reset
///
/// \param l1Map  Bitmap of L1 queues
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponClearL1Reset (L1Map l1Map);


////////////////////////////////////////////////////////////////////////////////
/// \brief Clear shaper configuration
///
///
/// \param shpr       Shaper element
///
/// \return
/// BOOL
////////////////////////////////////////////////////////////////////////////////
extern
void EponClearShaperConfig (EponShpElement shpr);


////////////////////////////////////////////////////////////////////////////////
/// \brief Sets the shaper rate for the specified element
///
/// Per register spec:
///     The cfgShpRate value represents the number of 1/(2^19) bytes
///     (~1/2 millionth of a byte) that are added to the shaper's byte
///     credit accumulator each clock cycle.  Given a 125 MHz clock-cycle;
///     the cfgShpRate is units of 1/ (2^19) Gbps (~2kbps).
///     The maximum burst size is in units of 256 bytes.
///
/// \param shpr       Shaper element
/// \param shaperRate Shaper rate
///
/// \return
/// BOOL
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetShaperRate (EponShpElement shpr, EponShaperRate shaperRate);


////////////////////////////////////////////////////////////////////////////////
/// \brief Sets the shaper max burst size for the specified element
///
/// Per register spec:
///     The cfgShpRate value represents the number of 1/(2^19) bytes
///     (~1/2 millionth of a byte) that are added to the shaper's byte
///     credit accumulator each clock cycle.  Given a 125 MHz clock-cycle;
///     the cfgShpRate is units of 1/ (2^19) Gbps (~2kbps).
///     The maximum burst size is in units of 256 bytes.
///
/// \param shpr       Shaper element
/// \param mbs        Shaper maximum burst size
///
/// \return
/// none
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetShaperMbs (EponShpElement shpr, EponMaxBurstSize mbs);


////////////////////////////////////////////////////////////////////////////////
/// \brief Sets the shaper IPG for the specified element
///
///
/// \param shpr       Shaper element
/// \param ipg        Include overhead in BW calculation
///
/// \return
/// none
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetShaperIpg (EponShpElement shpr, BOOL ipg);


////////////////////////////////////////////////////////////////////////////////
/// \brief Sets the shaper L1 map for the specified element
///
///
/// \param shpr       Shaper element
/// \param map        Map of L1s for element to shape on above port
///
/// \return
/// none
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetShaperL1Map (EponShpElement shpr, U32 map);

////////////////////////////////////////////////////////////////////////////////
/// \brief Gets the shaper L1 map for the specified element
///
///
/// \param shpr       Shaper element
/// \param map        Map of L1s for element to shape on above port
///
/// \return
/// none
////////////////////////////////////////////////////////////////////////////////
extern
void EponGetShaperL1Map (EponShpElement shpr, U32 *map);


////////////////////////////////////////////////////////////////////////////////
/// \brief Put L2 queues into reset
///
/// \param l2Map  Bitmap of L2 queues
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetL2Reset (L2Map l2Map);


////////////////////////////////////////////////////////////////////////////////
/// \brief Bring L2 queues out of reset
///
/// \param l2Map  Bitmap of L2 queues
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponClearL2Reset (L2Map l2Map);


////////////////////////////////////////////////////////////////////////////////
/// \brief Enable schedulers for L1 queues
///
/// \param l1Map  Bitmap of L1 queues
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponEnableL1Sched (L1Map l1Map);


////////////////////////////////////////////////////////////////////////////////
/// \brief Disable schedulers for L1 queues
///
/// \param l1Map  Bitmap of L1 queues
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponDisableL1Sched (L1Map l1Map);

#ifdef EPON_NORMAL_MODE

////////////////////////////////////////////////////////////////////////////////
/// \brief  Set the EPON to filter discovery gates
///
///  Note here that we don't filter out on the OLT capability but only on
///  Discovery window type. It would be strange for an OLT to open a window
///  that it is not capable to handle but in this case we would attempt
///  registration.
///
/// Parameter:
/// \param capability   will the ONU attemp 1G registration or 10G?
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponFilterDiscoveryGates (MpcpDiscoveryInfo capability);

////////////////////////////////////////////////////////////////////////////////
/// \brief Setup EPON for given number of priorities
///
/// \param numPri number of priority to set (between 1 and 8). Any other value
/// will lead to undetermined behavior.
/// 
/// \param Pon scheduling mode (reporting options).
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetNumPri (U8 numPri, PonSchMode mode);
#endif

extern
void EponDropUnmapLink (BOOL flag);


////////////////////////////////////////////////////////////////////////////////
/// \brief check if the L1 queue is empty
///
/// \param qIndex the queue to check
///
/// \return
/// True: empty; otherwise not empty
////////////////////////////////////////////////////////////////////////////////
extern
BOOL EponL1QueueEmptyCheck(U32 qIndex);


////////////////////////////////////////////////////////////////////////////////
/// \brief Set received max frame size of epon mac
///
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponSetMaxFrameSize (U16 maxFrameSize);

////////////////////////////////////////////////////////////////////////////////
/// \brief select which L1 Accumulator to report
///\input L1 Accumulator index
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponL1AccSel (U8 l1suvasizesel,U8 l1ssvasizesel);

////////////////////////////////////////////////////////////////////////////////
/// \brief get Signed number of bytes in the selected L1S Shaped/UnShaped Virtual Accumulator
///
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponL1AccBytesGet(U32 *l1ssvasize,U32 *l1suvasize);


extern
void EponSetPollSize (uint16_t pollsizefec, uint16_t pollsize);

////////////////////////////////////////////////////////////////////////////////
/// \brief get Epon Mac L2 queue empty/full/stoped status
///
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponTxL2QueStatusGet(U8 l2QueIndx,U8 *l2QueEmpty,U8 *l2QueFull,U8 *l2QueStopped);

////////////////////////////////////////////////////////////////////////////////
/// \brief get Epon Mac L1 queue Shaped/Unshaped status
///
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponTxL1QueStatusGet(U8 l1AccIndx,U8 *l1sDquQueEmpty,U8 *l1sUnshapedQueEmpty);

////////////////////////////////////////////////////////////////////////////////
/// \brief get Epon Mac L1 queue Shaped/Unshaped Overflow status
///
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponTxL1QueOverflowStatusGet(U32 *l1shpOverflow,U32 *l1unshpOverflow);

////////////////////////////////////////////////////////////////////////////////
/// \brief get Epon mac main interrupt status
///
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponMainInrpStatusGet(epn_main_int_status *main_int_status);

////////////////////////////////////////////////////////////////////////////////
/// \brief initialize EPON port
///
/// Resets EPON MAC to default values
///
/// \param up10G TRUE if upstream PON speed is 10G, FALSE otherwise
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void EponEpnInit (U16 maxFrameSize, BOOL preDraft2Dot1, BOOL add16TqToFront,
               LaserRate upRate);

////////////////////////////////////////////////////////////////////////////////
/// \brief Check if EPON Queue is empty
///
///
/// \param  None
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
Bool EponIsEmpty(void);

#if defined(__cplusplus)
}
#endif

#endif // Epon.h.h
