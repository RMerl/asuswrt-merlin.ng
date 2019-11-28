/*
* <:copyright-BRCM:2013:proprietary:standard
* 
*    Copyright (c) 2013 Broadcom 
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

#ifndef _EPONCTL_API_H_
#define _EPONCTL_API_H_

#include "bcm_epon_common.h"

//used for internal debug
////////////////////////////////////////////////////////////////////////////////
/// \brief API to Configure the PERS file to EPON stack. 
///
/// \param id            
///  ope     Set or Get operator.
///  value   The pointer to pers structure.
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
int eponStack_CtlCfgEponCfg(EponCtlOpe ope,EponCfgParam *value);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to Configure the Register value for hardware. 
///
/// \param id            
///  ope          Set or Get operator.
///  regStart   The register start address.
///  count      The register operator count.
///  regVal     The pointer to register value.
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
int eponStack_CtlCfgRegister(EponCtlOpe ope,U32 regStart ,U8 count,U32 *regVal);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Configure the Debug value for EPON stack. 
///
/// \param id            
///  ope          Set or Get operator.
///  value        The pointer to debug parameter.
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
int eponStack_CtlCfgDebug(EponCtlOpe ope,DebugPara *value);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Dump the stats for EPON MAC (LIF and EPN). Because of EPON statistics are read and clear.
///So we should disable the gather firstly.
///
/// \param id            
///  dumpid          Which block statistics to dump.
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
int eponStack_DumpStats(U8 dumpid);



////////////////////////////////////////////////////////////////////////////////
/// \brief API to Enable and Disable the statistics gather function.
///
/// \param id            
///  flag          Enable or Disable flag.
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
int eponStack_StatsGather(BOOL flag);

//used for external

////////////////////////////////////////////////////////////////////////////////
/// \brief API to Set the pers file field to EPON stack.
///
/// \param id            
///  index         The pers filed index.
///  value         The pointer to pers parameter.
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
typedef struct{
    EponCfgItem index;
    char     *name;
    U8       len;
} PACK EponCfgMoPara;


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Load the pers file field to EPON stack.
///
/// \param id            
///  flag         The flag if store the New pers data to kernel file system. 
///(EponPersLoadEnStore and EponPersLoadEnNoStore).
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlLoadEponCfg(U8 flag);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to configure queue(L1 and L2).
///
/// \param id            
///  ope          Set or Get operator.
///  cfg           The pointer to queue configuration parameter.
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfgMacQueue(EponCtlOpe ope, epon_mac_q_cfg_t *cfg);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to cfg L2(pon port) serveic state  according to wan state(epon0.x) .
///
/// \param id            
///  ope          Set or Get operator.
///  value        The pointer for Ctrl l2 state parameter.
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CfgL2PonState(EponCtlOpe ope,WanStatePara *value);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to configure FEC.
///
/// \param id            
///  ope          Set or Get operator.
///  value        The pointer for Fec parameter.
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfgFec(EponCtlOpe ope,FecPara *value);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to configure Holdover.
///
/// \param id            
///  ope          Set or Get operator.
///  time         The pointer for holdover time parameter.
///  flag          The pointer for holdover flag parameter.(The flag is supported in future)
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfgHoldover (EponCtlOpe ope, U16 *time, 
    EponHoldoverFlags *flag);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to configure L1 Byte Limit.
///
/// \param id            
///  ope          Set or Get operator.
///  queue       the queue to get/set.
///  limit          the limit value to set or get
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfgByteLimit (EponCtlOpe ope, U8* queue, U8* limit);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to configure Protect Switch.
///
/// \param id            
///  ope          Set or Get operator.
///  state        The pointer for protect time parameter.
///
/// \return
/// Note:The API is defined to use in future.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfgProtectSwitch (EponCtlOpe ope, PsOnuState *state);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to configure the PON los check time. After EPON stack detect the LOS, the EPON stack will
///continue to check for some times according to the parameter configuration. it is defined according to the 
///CTC spec.
///
/// \param id            
///  ope          Set or Get operator.
///  state        The pointer for los check time parameter.
///
/// \return
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfgLosCheckTime (EponCtlOpe ope, LosCheckPara *para);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to configure the Tx Power.
///
/// \param id            
///  actOpt        The flag for active PON interface.(this parameter should be ingore if only one PON interface)
///  enableTime The time for power shutdown.
///
/// \return
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlSetTxPower (BOOL actOpt, U16 enableTime);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to enable/disable laser rx/tx functions.
///
/// \param id            
///  dir      tx == 0, rx == 1.  
///  enable   specify status of function
///
/// \return
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlSetLaserEnable (Direction dir, BOOL enable);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to configure silence parameter.
///
/// \param id            
///  flag        The flag for silence enabled or disabled if receive the deregister MPCP message.
///  silence   The time for silence(the unit is second).
///
/// \return
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlSetSilence (BOOL flag, U8 silence);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get key used index for a special link.
///
/// \param id            
///  link           The link index.
///  keyindex   The pointer for key index parameter used in the link.
///
/// \return
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlGetKeyInuse (U8 link, U8 *keyindex);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Set key Mode for a special link.
///
/// \param id            
///  link           The link index.
///  mode       The Enctypt mode for this link.
///  opts        The Enctype options for this mode.(Only EncryptModeAes mode need this options)
///
/// \return
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlSetKeyMode (U8 link, EncryptMode mode, EncryptOptions opts);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Load key Data to hardware for a special link.
///
/// \param id            
///  link           The link index.
///  dir            The direction (upstream and downstream).
///  keyindex  The key index used for this link.
///  length       The key data length(the unit is bytes).
///
/// \return
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlSetKeyData (U8 link, Direction dir,U8 keyindex,U32 *key,U8 length, U32 *sci);


////////////////////////////////////////////////////////////////////////////////
/// \brief API toSet and Get the current Link number.
///
/// \param id            
///  link           The Pointer to  link number.
///
/// \return
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CfgLinkNum(EponCtlOpe ope,U8 *num, U8 *onlinelinks);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get the Current MPCP status for special link.
///
/// \param id            
///  link           The link index.
///  status       The MPCP status of this link.
///
/// \return
////////////////////////////////////////////////////////////////////////////////
int eponStack_CtlGetLinkMpcpStatus (U8 link , EponMpcpStatus *status);



////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get the physic LLID  for specified logic link.
///
/// \param id            
///  link      The link index.
///  llid       The physic LLID value.
///
/// \return
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlGetLLID(U8 link ,U16 *llid);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get the logic link index from specified physic LLID.
///
/// \param id            
///  llid      The physic LLID value.
///  link      The link index.       
///
/// \return
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlGetLinkIndex(U16 llid ,U8 *link);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get the Max Phy Llid Number.
///
/// \param id            
///  num      The max phy LLID number.
///
/// \return
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlGetMaxPhyLlidNum(U16 *num);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to configure the CTC alsrm state.
///
/// \param id            
///  ope        Set or Get operator.
///  alarmid   The pointer to the CTC alarm id.
///  count      The alarm count.
///  enable     Enable or Disable the alarm.
///
/// Note:Because this API is allow to configure multi-alarm at the same time. So each enable or disable status
///should be match for each alarmid.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfgCtcAlarmState (EponCtlOpe ope, U16 *alarmid,U8 count, BOOL *enable);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to configure the CTC alarm threshold.
///
/// \param id            
///  ope        Set or Get operator.
///  alarmid   The pointer to the CTC alarm id.
///  count      The alarm count.
///  setthreshold    The alarm raise threshold.
///  clearthreshold  The alarm clear threshold.
///
/// Note:Because this API is allow to configure multi-alarm at the same time. So each setthreshold and clearthreshold 
/// value should be match for each alarmid.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfgCtcAlarmThreshold (
											EponCtlOpe ope,
											U16 *alarmid,
											U8 count ,
											U32 *setthreshold, 
											U32 *clearthreshold);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to configure the CTC stats period.
///
/// \param id            
///  ope        Set or Get operator.
///  enable    Enable or Disalbe the CTC performance monitor for PON side.
///  period    The period value.
/// value should be match for each alarmid.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfgCtcStatsPeriod (EponCtlOpe ope, BOOL *enable,U32 *period);



////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get the CTC stats .
///
/// \param id            
///  history    Current or History stats.
///  statsid    Stats index.
///  dst         The stats value for each stats.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlGetCtcStats (BOOL history, U8 *statsid, U8 count, U64 *dst);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Clear the CTC stats .
///
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlClearCtcStats (void);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Configure the shaper profile for EPON MAC .
///
/// \param id            
///  ope         Set or Get operator.
///  shaperL1Map   L1 bitmap.
///  rate        The shaper profile rate.
///  size         The shaper profile burst size.
///  shp          The shaper profile index.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfgShaper (EponCtlOpe ope ,U32 *shaperL1Map,	U32 *rate,

						  U16 *size,U8 *shp) ;


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Delete a shaper profile  .
///
/// \param id            
///  shp          The shaper profile index.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlDelShaper (U8 shp) ;


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Configure the burst cap for EPON MAC .
///
/// \param id            
///  ope         Set or Get operator.
///  link         The logic link index.
///  size         The  burst cap size.
///  count        The queue count.
////////////////////////////////////////////////////////////////////////////////

extern
int eponStack_CtCfgBurstCap (EponCtlOpe ope ,U8 link,  U16 *size,U8 *count);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to set the current process id to epon stack to send nectlink message .
///
/// \param id            
/// pid process id.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlSetPid (int pid);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get the lif stats.
///
/// \param id            
/// stats Stats id.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlLifStatsGet(StatsCntOne *stats,U8 count);

#ifdef CONFIG_EPON_10G_SUPPORT
////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get the Xif stats.
///
/// \param id            
/// stats Stats id.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlXifStatsGet(StatsCntOne *stats,U8 count);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get the Xpcs32 stats.
///
/// \param id            
/// stats Stats id.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlXpcs32StatsGet(StatsCntOne *stats,U8 count);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get the Xpcs40 stats.
///
/// \param id            
/// stats Stats id.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlXpcs40StatsGet(StatsCntOne *stats,U8 count);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to Set/Get 10G FEC auto-detection setting.
///
/// \param             
///  ope          Set or Get operator.
///  enable      enable or not.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfg10gFecAutoDet(EponCtlOpe ope, Bool* enable);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to force switch 10G FEC mode once.
///
/// \param             
///  none
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfg10gFecSwitchOnce(void);
#endif

////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get the Link stats.
///
/// \param id            
/// stats Stats id.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlLinkStatsGet(U8 link, StatsCntOne *stats,U8 count);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get the EPON port stats.
///
/// \param id            
/// stats Stats id.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlPonStatsGet(StatsCntOne *stats,U8 count);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to Clear the EPON port stats.
///
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlPonStatsClear(void);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Clear the EPON link stats.
///
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlLinkStatsClear(U8 link);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to Assign Mcast Link.
///
/// \param id
///  link         The logic link index.
///  phyLlid    The link's LLID.
///  flags       Action (assign or deallocate).
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlAssignMcast (U8 link, U16 phyLlid, U8 flags, BOOL isStandalone);

/// \brief API to Get Mcast Link.
///
/// \param id
///  link         The logic link index.
///  phyLlid    The link's LLID.
///  IdxMcast   The link index of Mcast.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlGetMcast (U8 link, U16 *phyLlid, U16 *idxMcast, U8 flags);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to set/get link loopback state.
///
/// \param id
///  ope         Set or Get operator.
///  link         The logic link index.
///  loopback       the loopback state to get/set.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlLinkLoopback (EponCtlOpe ope, U8 link, BOOL* loopback);


#ifdef CLOCK_TRANSPORT
////////////////////////////////////////////////////////////////////////////////
/// \brief API to set clock transport.
///
/// \param id
///  para       flag and data .
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlSetClkTrans (ClkTransPara *para);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to set clock transport.
///
/// \param id
///  para       nextPulseTime .
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlSetClk1ppsTickTrans(OamExtMpcpClock * para);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get clock transport configuration.
///
/// \param id
///  para       key, reinit, tod, pulse, adj, rtt, adj.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlGetClk1ppsCompTrans(TkOamOnuClkTransConfig * para);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to set clock transport.
///
/// \param id
///  para       key, reinit, tod, pulse, adj, rtt, adj.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlSetClk1ppsCompTrans(TkOamOnuClkTransConfig * para);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to set clock transport.
///
/// \param id
///  para       seqNum, tod .
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlSetClkTodTrans(OamExtTimeOfDay * para);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to Get clock transport transfer time
///
/// \param id
///  para       refClock, todstring.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlGetClockTransferTime(DpoeClockTransferTime * para);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to Set clock transport transfer time
///
/// \param id
///  para       refClock, todstring.
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlSetClockTransferTime(DpoeClockTransferTime * para);

#endif

/******************************************************************************/
/*  \brief API to get epon report mode                                        */
/*                                                                            */
/*  \param id                                                                 */
/*   PonMgrRptMode     report mode                                            */
/******************************************************************************/
extern
int eponStack_CtlGetReportMode(PonMgrRptMode * mode);

/******************************************************************************/
/*  \brief API to set epon report mode                                        */
/*                                                                            */
/*  \param id                                                                 */
/*   PonMgrRptMode     report mode                                            */
/******************************************************************************/
extern
int eponStack_CtlSetReportMode(PonMgrRptMode mode);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to set/get epon rogue onu detection configuration
///
/// \param id
///  ope           Set or Get operator.
///  enable       enable detection or not
///  threshold    threshold to set for detection
///  times         raise alarm after how many times' detection of error
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfgRogueOnuDet(
        EponCtlOpe ope, Bool* enable, U32* threshold, U8* times);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to set/get epon failsafe configuration
///
/// \param id
///  ope           Set or Get operator.
///  enable       enable failsafe or not
////////////////////////////////////////////////////////////////////////////////
extern 
int eponStack_CtlCfgFailSafe(EponCtlOpe ope, Bool* enable);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to set received max frame size of epon mac
///
///  maxFrameSize  max frame size, <=2000
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlSetMaxFrameSize(U16 maxFrameSize);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to set/get Epon Mac L1 Acc info
///
/// l1Index  < 32
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlDumpEpnInfo(EponCtlOpe ope,EpnInfoId_e eponInfoId,U32 l1Index);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to set/get EPON Mac MPCP registration progress control(enable/disable) status
///
/// \param id
///  ope           Set or Get operator.
///  enable       enable MPCP registration or not
/// \return
/// 0: success, other: fail
////////////////////////////////////////////////////////////////////////////////
extern 
int eponStack_CtlMpcpRegister(EponCtlOpe ope, BOOL* enable);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to do EPON Mac flush
///
/// \param id
///  ope               Set 
///  linkIndex       link to flush
/// \return
/// 0: success, other: fail
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlFlushLlid(U8 linkIndex);

extern
int eponStack_CtlPowerSaveCfg(EponCtlOpe ope, Bool * earlyWakeupEnable, U64 * refreshTime);

extern
int eponStack_CtlSleepCtrl(EponCtlOpe ope, SleepCtrlPara * sleep);

extern
int eponStack_GetPowerSavingDebugState(EponPowerSavingRunningState * state);

////////////////////////////////////////////////////////////////////////////////
/// \brief API to enale/disable hold Mac state when detect offline event(NoGate, de-register...)
///         just for debug.
///
/// \param id
///  isHold       enable/disable
/// \return
/// 0: success, other: fail
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlHoldMacState(BOOL isHold);


////////////////////////////////////////////////////////////////////////////////
/// \brief API to set/get epon fatal error auto reset configuration
///
/// \param id
///  ope           Set or Get operator.
///  enable       enable auto reset
////////////////////////////////////////////////////////////////////////////////
extern
int eponStack_CtlCfgFatalErrRst(EponCtlOpe ope, Bool* enable);

#endif /* _EPONCTL_API_H_ */

