#ifndef _FAPI_GSW_HOST_H_

/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/

#include <mmd_apis.h>

GSW_return_t fapi_GSW_RegisterGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_RegisterSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PortLinkCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PortLinkCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_RMON_Clear(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MonitorPortCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MonitorPortCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_PortCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_PortCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_DSCP_ClassGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_DSCP_ClassSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_PCP_ClassGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_PCP_ClassSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_SVLAN_PCP_ClassGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_SVLAN_PCP_ClassSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_ShaperCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_ShaperCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_ShaperQueueDeassign(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_ShaperQueueAssign(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_ShaperQueueGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_SchedulerCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_SchedulerCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_WredCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_WredCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_WredQueueCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_WredQueueCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_WredPortCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_WredPortCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_TrunkingCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_TrunkingCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MAC_TableClear(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_CfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_CfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MAC_TableEntryRemove(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MAC_TableEntryQuery(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_FlowctrlCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_FlowctrlCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_FlowctrlPortCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_FlowctrlPortCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MAC_TableEntryAdd(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MAC_TableEntryRead(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_QueuePortSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_QueuePortGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_QueueCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_QueueCfgGet(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_CPU_PortCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_CPU_PortCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_CPU_PortSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_CPU_PortGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_BridgePortConfigGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_BridgePortConfigSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_CtpPortConfigGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_CtpPortConfigSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_BridgeAlloc(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_BridgeFree(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_BridgeConfigSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_BridgeConfigGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_ExtendedVlanAlloc(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_ExtendedVlanFree(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_ExtendedVlanSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_ExtendedVlanGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_VlanFilterFree(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_VlanFilterAlloc(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_VlanFilterSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_VlanFilterGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_STP_PortCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_STP_PortCfgGet(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_STP_BPDU_RuleSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_STP_BPDU_RuleGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_RegisterMod(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_Debug_RMON_Port_Get(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_DEBUG_RMON_Port_Get_All(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_QoS_MeterCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_MeterCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MAC_DefaultFilterGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MAC_DefaultFilterSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_CTP_PortAssignmentGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_CTP_PortAssignmentSet(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_PMAC_GLBL_CfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PMAC_GLBL_CfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PMAC_BM_CfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PMAC_BM_CfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PMAC_EG_CfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PMAC_EG_CfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PMAC_IG_CfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PMAC_IG_CfgSet(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_PceRuleRead(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PceRuleWrite(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PceRuleDelete(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PceRuleAlloc(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PceRuleFree(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PceRuleEnable(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PceRuleDisable(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_MulticastRouterPortAdd(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MulticastRouterPortRemove(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MulticastSnoopCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MulticastSnoopCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MulticastRouterPortRead(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MulticastTableEntryAdd(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MulticastTableEntryRead(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_MulticastTableEntryRemove(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_FW_Update(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_FW_Version(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PVT_Meas(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_Delay(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_GPIO_Configure(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_Reboot(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_SysReg_Rd(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_SysReg_Wr(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_SysReg_Mod(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_Cml_Clk_Get(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_Cml_Clk_Set(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_Sfp_Get(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_Sfp_Set(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_MAC_TableCondClear(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_VlanCounterMapSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_VlanCounterMapGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_Vlan_RMON_Get(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_Vlan_RMON_Clear(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_Vlan_RMONControl_Set(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_Vlan_RMONControl_Get(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_PBB_TunnelTempate_Config_Set(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PBB_TunnelTempate_Config_Get(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PBB_TunnelTempate_Alloc(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PBB_TunnelTempate_Free(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_RMON_PortGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_RMON_ModeSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_RMON_MeterGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_RMON_FlowGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_RMON_TFlowClear(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_BridgePortAlloc(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_BridgePortFree(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_Freeze(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_UnFreeze(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_QoS_MeterFree(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_MeterAlloc(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_PMAC_RMON_Get(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_Debug_PMAC_RMON_Get_All(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_SS_Sptag_Get(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_SS_Sptag_Set(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_DSCP_DropPrecedenceCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_DSCP_DropPrecedenceCfgSet(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_QoS_ColorMarkingTableGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_ColorMarkingTableSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_ColorReMarkingTableGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_ColorReMarkingTableSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_DSCP2_PCPTableGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_DSCP2_PCPTableSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_PortReMarkingCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_PortReMarkingCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_StormCfgGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_StormCfgSet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_PmapperTableGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_QoS_PmapperTableSet(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_Pce_RuleBlockSize(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_BridgePort_LoopRead(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_TflowCountModeGet(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_TflowCountModeSet(int prmc, char *prmv[]);

GSW_return_t fapi_GSW_Mac_TableLoopDetect(int prmc, char *prmv[]);
GSW_return_t fapi_GSW_PCE_RuleMove(int prmc, char *prmv[]);


#endif /* _FAPI_GSW_HOST_H_ */
