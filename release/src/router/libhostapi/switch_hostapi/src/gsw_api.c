/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#include "host_adapt.h"
#include "host_api_impl.h"
#include <gsw_api.h>

GSW_return_t GSW_RegisterGet(const GSW_Device_t *dev, GSW_register_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_REGISTERGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_RegisterSet(const GSW_Device_t *dev, GSW_register_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_REGISTERSET,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_RegisterMod(const GSW_Device_t *dev, GSW_register_mod_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_REGISTERMOD,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_CPU_PortGet(const GSW_Device_t *dev, GSW_CPU_Port_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_CPU_PORTGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_CPU_PortSet(const GSW_Device_t *dev, GSW_CPU_Port_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_CPU_PORTSET,
			    parm,
			    sizeof(*parm),
			    GSW_COMMON_CPU_PORTGET,
			    0);
}

GSW_return_t GSW_CPU_PortCfgGet(const GSW_Device_t *dev, GSW_CPU_PortCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_CPU_PORTCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_CPU_PortCfgSet(const GSW_Device_t *dev, GSW_CPU_PortCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_CPU_PORTCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_COMMON_CPU_PORTCFGGET,
			    0);
}

GSW_return_t GSW_PortLinkCfgGet(const GSW_Device_t *dev, GSW_portLinkCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_PORTLINKCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PortLinkCfgSet(const GSW_Device_t *dev, GSW_portLinkCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_PORTLINKCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_COMMON_PORTLINKCFGGET,
			    0);
}

GSW_return_t GSW_PortCfgGet(const GSW_Device_t *dev, GSW_portCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_PORTCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PortCfgSet(const GSW_Device_t *dev, GSW_portCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_PORTCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_COMMON_PORTCFGGET,
			    0);
}

GSW_return_t GSW_CfgGet(const GSW_Device_t *dev, GSW_cfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_CFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_CfgSet(const GSW_Device_t *dev, GSW_cfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_CFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_COMMON_CFGGET,
			    0);
}

GSW_return_t GSW_MonitorPortCfgGet(const GSW_Device_t *dev, GSW_monitorPortCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_MONITORPORTCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_MonitorPortCfgSet(const GSW_Device_t *dev, GSW_monitorPortCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_MONITORPORTCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_COMMON_MONITORPORTCFGGET,
			    0);
}

GSW_return_t GSW_Freeze(const GSW_Device_t *dev)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_FREEZE,
			    NULL,
			    0,
			    0,
			    0);
}

GSW_return_t GSW_UnFreeze(const GSW_Device_t *dev)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_UNFREEZE,
			    NULL,
			    0,
			    0,
			    0);
}

GSW_return_t GSW_GetHitSts(const GSW_Device_t *dev, GSW_HitStatusRead_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_GETHITSTS,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_GetMiscPortCfgGet(const GSW_Device_t *dev, GSW_MiscPortCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_MISCPORTCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_GetMiscPortCfgSet(const GSW_Device_t *dev, GSW_MiscPortCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_COMMON_MISCPORTCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_COMMON_MISCPORTCFGGET,
			    sizeof(*parm));
}

GSW_return_t GSW_PceRuleAlloc(const GSW_Device_t *dev,
			      GSW_PCE_rule_alloc_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_TFLOW_PCERULEALLOC,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PceRuleFree(const GSW_Device_t *dev,
			     GSW_PCE_rule_alloc_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_TFLOW_PCERULEFREE,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PceRuleBlockSize(const GSW_Device_t *dev,
				  GSW_PCE_rule_alloc_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_TFLOW_PCERULEBLOCKSIZE,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PceRuleEnable(const GSW_Device_t *dev,
			       GSW_PCE_ruleEntry_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_TFLOW_PCERULEENABLE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_PceRuleDisable(const GSW_Device_t *dev,
				GSW_PCE_ruleEntry_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_TFLOW_PCERULEDISABLE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_PceRuleRead(const GSW_Device_t *dev, GSW_PCE_rule_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_TFLOW_PCERULEREAD,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PceRuleWrite(const GSW_Device_t *dev, GSW_PCE_rule_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_TFLOW_PCERULEWRITE,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PceRuleDelete(const GSW_Device_t *dev, GSW_PCE_ruleEntry_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_TFLOW_PCERULEDELETE,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PceRuleMove(const GSW_Device_t *dev, GSW_PCE_rule_move_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_TFLOW_PCERULEMOVE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_BridgeAlloc(const GSW_Device_t *dev, GSW_BRIDGE_alloc_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_BRIDGE_ALLOC,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_BridgeConfigSet(const GSW_Device_t *dev, GSW_BRIDGE_config_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_BRIDGE_CONFIGSET,
			    parm,
			    sizeof(*parm),
			    GSW_BRIDGE_CONFIGGET,
			    0);
}

GSW_return_t GSW_BridgeConfigGet(const GSW_Device_t *dev, GSW_BRIDGE_config_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_BRIDGE_CONFIGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_BridgeFree(const GSW_Device_t *dev, GSW_BRIDGE_alloc_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_BRIDGE_FREE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_BridgePortAlloc(const GSW_Device_t *dev, GSW_BRIDGE_portAlloc_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_BRIDGEPORT_ALLOC,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_BridgePortConfigSet(const GSW_Device_t *dev, GSW_BRIDGE_portConfig_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_BRIDGEPORT_CONFIGSET,
			    parm,
			    sizeof(*parm),
			    GSW_BRIDGEPORT_CONFIGGET,
			    0);
}

GSW_return_t GSW_BridgePortConfigGet(const GSW_Device_t *dev, GSW_BRIDGE_portConfig_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_BRIDGEPORT_CONFIGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_BridgePortFree(const GSW_Device_t *dev, GSW_BRIDGE_portAlloc_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_BRIDGEPORT_FREE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_BridgePortLoopRead(const GSW_Device_t *dev, GSW_BRIDGE_portLoopRead_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_BRIDGEPORT_LOOPREAD,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_CTP_PortAssignmentAlloc(const GSW_Device_t *dev, GSW_CTP_portAssignment_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_CTP_PORTASSIGNMENTALLOC,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_CTP_PortAssignmentFree(const GSW_Device_t *dev, GSW_CTP_portAssignment_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_CTP_PORTASSIGNMENTFREE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_CTP_PortAssignmentSet(const GSW_Device_t *dev, GSW_CTP_portAssignment_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_CTP_PORTASSIGNMENTSET,
			    parm,
			    sizeof(*parm),
			    GSW_CTP_PORTASSIGNMENTGET,
			    0);
}

GSW_return_t GSW_CTP_PortAssignmentGet(const GSW_Device_t *dev, GSW_CTP_portAssignment_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_CTP_PORTASSIGNMENTGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_CtpPortConfigSet(const GSW_Device_t *dev, GSW_CTP_portConfig_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_CTP_PORTCONFIGSET,
			    parm,
			    sizeof(*parm),
			    GSW_CTP_PORTCONFIGGET,
			    0);
}

GSW_return_t GSW_CtpPortConfigGet(const GSW_Device_t *dev, GSW_CTP_portConfig_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_CTP_PORTCONFIGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_CtpPortConfigReset(const GSW_Device_t *dev, GSW_CTP_portConfig_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_CTP_PORTCONFIGRESET,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_QoS_MeterCfgGet(const GSW_Device_t *dev, GSW_QoS_meterCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_METERCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_MeterCfgSet(const GSW_Device_t *dev, GSW_QoS_meterCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_METERCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_METERCFGGET,
			    0);
}

GSW_return_t GSW_QoS_DSCP_ClassGet(const GSW_Device_t *dev, GSW_QoS_DSCP_ClassCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_DSCP_CLASSGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_DSCP_ClassSet(const GSW_Device_t *dev, GSW_QoS_DSCP_ClassCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_DSCP_CLASSSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_DSCP_CLASSGET,
			    0);
}

GSW_return_t GSW_QoS_DSCP_DropPrecedenceCfgGet(const GSW_Device_t *dev, GSW_QoS_DSCP_DropPrecedenceCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_DSCP_DROPPRECEDENCECFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_DSCP_DropPrecedenceCfgSet(const GSW_Device_t *dev, GSW_QoS_DSCP_DropPrecedenceCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_DSCP_DROPPRECEDENCECFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_DSCP_DROPPRECEDENCECFGGET,
			    0);
}

GSW_return_t GSW_QoS_PortRemarkingCfgGet(const GSW_Device_t *dev, GSW_QoS_portRemarkingCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_PORTREMARKINGCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_PortRemarkingCfgSet(const GSW_Device_t *dev, GSW_QoS_portRemarkingCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_PORTREMARKINGCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_PORTREMARKINGCFGGET,
			    0);
}

GSW_return_t GSW_QoS_PCP_ClassGet(const GSW_Device_t *dev, GSW_QoS_PCP_ClassCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_PCP_CLASSGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_PCP_ClassSet(const GSW_Device_t *dev, GSW_QoS_PCP_ClassCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_PCP_CLASSSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_PCP_CLASSGET,
			    0);
}

GSW_return_t GSW_QoS_PortCfgGet(const GSW_Device_t *dev, GSW_QoS_portCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_PORTCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_PortCfgSet(const GSW_Device_t *dev, GSW_QoS_portCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_PORTCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_PORTCFGGET,
			    0);
}

GSW_return_t GSW_QoS_QueueCfgGet(const GSW_Device_t *dev, GSW_QoS_queueCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_QUEUECFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_QueueCfgSet(const GSW_Device_t *dev, GSW_QoS_queueCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_QUEUECFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_QUEUECFGGET,
			    0);
}

GSW_return_t GSW_QoS_QueuePortGet(const GSW_Device_t *dev, GSW_QoS_queuePort_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_QUEUEPORTGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_QueuePortSet(const GSW_Device_t *dev, GSW_QoS_queuePort_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_QUEUEPORTSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_QUEUEPORTGET,
			    0);
}

GSW_return_t GSW_QoS_SchedulerCfgGet(const GSW_Device_t *dev, GSW_QoS_schedulerCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_SCHEDULERCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_SchedulerCfgSet(const GSW_Device_t *dev, GSW_QoS_schedulerCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_SCHEDULERCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_SCHEDULERCFGGET,
			    0);
}

GSW_return_t GSW_QoS_ShaperCfgGet(const GSW_Device_t *dev, GSW_QoS_ShaperCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_SHAPERCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_ShaperCfgSet(const GSW_Device_t *dev, GSW_QoS_ShaperCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_SHAPERCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_SHAPERCFGGET,
			    0);
}

GSW_return_t GSW_QoS_ShaperQueueAssign(const GSW_Device_t *dev, GSW_QoS_ShaperQueue_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_SHAPERQUEUEASSIGN,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_QoS_ShaperQueueDeassign(const GSW_Device_t *dev, GSW_QoS_ShaperQueue_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_SHAPERQUEUEDEASSIGN,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_QoS_ShaperQueueGet(const GSW_Device_t *dev, GSW_QoS_ShaperQueueGet_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_SHAPERQUEUEGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_StormCfgSet(const GSW_Device_t *dev, GSW_QoS_stormCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_STORMCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_STORMCFGGET,
			    0);
}

GSW_return_t GSW_QoS_StormCfgGet(const GSW_Device_t *dev, GSW_QoS_stormCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_STORMCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_WredCfgGet(const GSW_Device_t *dev, GSW_QoS_WRED_Cfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_WREDCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_WredCfgSet(const GSW_Device_t *dev, GSW_QoS_WRED_Cfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_WREDCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_WREDCFGGET,
			    0);
}

GSW_return_t GSW_QoS_WredQueueCfgGet(const GSW_Device_t *dev, GSW_QoS_WRED_QueueCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_WREDQUEUECFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_WredQueueCfgSet(const GSW_Device_t *dev, GSW_QoS_WRED_QueueCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_WREDQUEUECFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_WREDQUEUECFGGET,
			    0);
}

GSW_return_t GSW_QoS_WredPortCfgGet(const GSW_Device_t *dev, GSW_QoS_WRED_PortCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_WREDPORTCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_WredPortCfgSet(const GSW_Device_t *dev, GSW_QoS_WRED_PortCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_WREDPORTCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_WREDPORTCFGGET,
			    0);
}

GSW_return_t GSW_QoS_FlowctrlCfgGet(const GSW_Device_t *dev, GSW_QoS_FlowCtrlCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_FLOWCTRLCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_FlowctrlCfgSet(const GSW_Device_t *dev, GSW_QoS_FlowCtrlCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_FLOWCTRLCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_FLOWCTRLCFGGET,
			    0);
}

GSW_return_t GSW_QoS_FlowctrlPortCfgGet(const GSW_Device_t *dev, GSW_QoS_FlowCtrlPortCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_FLOWCTRLPORTCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_FlowctrlPortCfgSet(const GSW_Device_t *dev, GSW_QoS_FlowCtrlPortCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_FLOWCTRLPORTCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_FLOWCTRLPORTCFGGET,
			    0);
}

GSW_return_t GSW_QoS_QueueBufferReserveCfgGet(const GSW_Device_t *dev, GSW_QoS_QueueBufferReserveCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_QUEUEBUFFERRESERVECFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_QueueBufferReserveCfgSet(const GSW_Device_t *dev, GSW_QoS_QueueBufferReserveCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_QUEUEBUFFERRESERVECFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_QUEUEBUFFERRESERVECFGGET,
			    0);
}

GSW_return_t GSW_QOS_ColorMarkingTableGet(const GSW_Device_t *dev, GSW_QoS_colorMarkingEntry_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_COLORMARKINGTABLEGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QOS_ColorMarkingTableSet(const GSW_Device_t *dev, GSW_QoS_colorMarkingEntry_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_COLORMARKINGTABLESET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_COLORMARKINGTABLEGET,
			    0);
}

GSW_return_t GSW_QOS_ColorReMarkingTableSet(const GSW_Device_t *dev, GSW_QoS_colorRemarkingEntry_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_COLORREMARKINGTABLESET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_COLORREMARKINGTABLEGET,
			    0);
}

GSW_return_t GSW_QOS_ColorReMarkingTableGet(const GSW_Device_t *dev, GSW_QoS_colorRemarkingEntry_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_COLORREMARKINGTABLEGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QOS_MeterAlloc(const GSW_Device_t *dev, GSW_QoS_meterCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_METERALLOC,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QOS_MeterFree(const GSW_Device_t *dev, GSW_QoS_meterCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_METERFREE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_QOS_Dscp2PcpTableSet(const GSW_Device_t *dev, GSW_DSCP2PCP_map_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_DSCP2PCPTABLESET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_DSCP2PCPTABLEGET,
			    0);
}

GSW_return_t GSW_QOS_Dscp2PcpTableGet(const GSW_Device_t *dev, GSW_DSCP2PCP_map_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_DSCP2PCPTABLEGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QOS_PmapperTableSet(const GSW_Device_t *dev, GSW_PMAPPER_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_PMAPPERTABLESET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_PMAPPERTABLEGET,
			    0);
}

GSW_return_t GSW_QOS_PmapperTableGet(const GSW_Device_t *dev, GSW_PMAPPER_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_PMAPPERTABLEGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_SVLAN_PCP_ClassGet(const GSW_Device_t *dev, GSW_QoS_SVLAN_PCP_ClassCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_SVLAN_PCP_CLASSGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_QoS_SVLAN_PCP_ClassSet(const GSW_Device_t *dev, GSW_QoS_SVLAN_PCP_ClassCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_QOS_SVLAN_PCP_CLASSSET,
			    parm,
			    sizeof(*parm),
			    GSW_QOS_SVLAN_PCP_CLASSGET,
			    0);
}

GSW_return_t GSW_RMON_Port_Get(const GSW_Device_t *dev, GSW_RMON_Port_cnt_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_RMON_PORT_GET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_RMON_Mode_Set(const GSW_Device_t *dev, GSW_RMON_mode_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_RMON_MODE_SET,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_RMON_Meter_Get(const GSW_Device_t *dev, GSW_RMON_Meter_cnt_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_RMON_METER_GET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_RMON_Clear(const GSW_Device_t *dev, GSW_RMON_clear_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_RMON_CLEAR,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_RMON_FlowGet(const GSW_Device_t *dev, GSW_RMON_flowGet_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_RMON_TFLOWGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_RmonTflowClear(const GSW_Device_t *dev, GSW_RMON_flowGet_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_RMON_TFLOWCLEAR,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_TflowCountModeSet(const GSW_Device_t *dev, GSW_TflowCmodeConf_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_RMON_TFLOWCOUNTMODESET,
			    parm,
			    sizeof(*parm),
			    GSW_RMON_TFLOWCOUNTMODEGET,
			    0);
}

GSW_return_t GSW_TflowCountModeGet(const GSW_Device_t *dev, GSW_TflowCmodeConf_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_RMON_TFLOWCOUNTMODEGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_Debug_RMON_Port_Get(const GSW_Device_t *dev, GSW_Debug_RMON_Port_cnt_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_DEBUG_RMON_PORT_GET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PMAC_CountGet(const GSW_Device_t *dev, GSW_PMAC_Cnt_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_PMAC_COUNTGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PMAC_GLBL_CfgSet(const GSW_Device_t *dev, GSW_PMAC_Glbl_Cfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_PMAC_GBL_CFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_PMAC_GBL_CFGGET,
			    0);
}

GSW_return_t GSW_PMAC_GLBL_CfgGet(const GSW_Device_t *dev, GSW_PMAC_Glbl_Cfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_PMAC_GBL_CFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PMAC_BM_CfgSet(const GSW_Device_t *dev, GSW_PMAC_BM_Cfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_PMAC_BM_CFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_PMAC_BM_CFGGET,
			    0);
}

GSW_return_t GSW_PMAC_BM_CfgGet(const GSW_Device_t *dev, GSW_PMAC_BM_Cfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_PMAC_BM_CFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PMAC_IG_CfgSet(const GSW_Device_t *dev, GSW_PMAC_Ig_Cfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_PMAC_IG_CFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_PMAC_IG_CFGGET,
			    0);
}

GSW_return_t GSW_PMAC_IG_CfgGet(const GSW_Device_t *dev, GSW_PMAC_Ig_Cfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_PMAC_IG_CFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PMAC_EG_CfgSet(const GSW_Device_t *dev, GSW_PMAC_Eg_Cfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_PMAC_EG_CFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_PMAC_EG_CFGGET,
			    0);
}

GSW_return_t GSW_PMAC_EG_CfgGet(const GSW_Device_t *dev, GSW_PMAC_Eg_Cfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_PMAC_EG_CFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_MAC_TableClear(const GSW_Device_t *dev)
{
	return gsw_api_wrap(dev,
			    GSW_MAC_TABLECLEAR,
			    NULL,
			    0,
			    0,
			    0);
}

GSW_return_t GSW_MAC_TableClearCond(const GSW_Device_t *dev,
				    GSW_MAC_tableClearCond_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MAC_TABLECLEARCOND,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_MAC_TableEntryAdd(const GSW_Device_t *dev, GSW_MAC_tableAdd_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MAC_TABLEENTRYADD,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_MAC_TableEntryRead(const GSW_Device_t *dev, GSW_MAC_tableRead_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MAC_TABLEENTRYREAD,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_MAC_TableEntryQuery(const GSW_Device_t *dev, GSW_MAC_tableQuery_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MAC_TABLEENTRYQUERY,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_MAC_TableEntryRemove(const GSW_Device_t *dev, GSW_MAC_tableRemove_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MAC_TABLEENTRYREMOVE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_DefaultMacFilterSet(const GSW_Device_t *dev, GSW_MACFILTER_default_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MAC_DEFAULTFILTERSET,
			    parm,
			    sizeof(*parm),
			    GSW_MAC_DEFAULTFILTERGET,
			    0);
}

GSW_return_t GSW_DefaultMacFilterGet(const GSW_Device_t *dev, GSW_MACFILTER_default_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MAC_DEFAULTFILTERGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_MAC_TableLoopDetect(const GSW_Device_t *dev, GSW_MAC_tableLoopDetect_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MAC_TABLE_LOOP_DETECT,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

#ifdef CONFIG_GSWIP_EVLAN
GSW_return_t GSW_ExtendedVlanAlloc(const GSW_Device_t *dev, GSW_EXTENDEDVLAN_alloc_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_EXTENDEDVLAN_ALLOC,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_ExtendedVlanSet(const GSW_Device_t *dev, GSW_EXTENDEDVLAN_config_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_EXTENDEDVLAN_SET,
			    parm,
			    sizeof(*parm),
			    GSW_EXTENDEDVLAN_GET,
			    0);
}

GSW_return_t GSW_ExtendedVlanGet(const GSW_Device_t *dev, GSW_EXTENDEDVLAN_config_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_EXTENDEDVLAN_GET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_ExtendedVlanFree(const GSW_Device_t *dev, GSW_EXTENDEDVLAN_alloc_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_EXTENDEDVLAN_FREE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_VlanFilterAlloc(const GSW_Device_t *dev, GSW_VLANFILTER_alloc_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_VLANFILTER_ALLOC,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_VlanFilterSet(const GSW_Device_t *dev, GSW_VLANFILTER_config_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_VLANFILTER_SET,
			    parm,
			    sizeof(*parm),
			    GSW_VLANFILTER_GET,
			    0);
}

GSW_return_t GSW_VlanFilterGet(const GSW_Device_t *dev, GSW_VLANFILTER_config_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_VLANFILTER_GET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_VlanFilterFree(const GSW_Device_t *dev, GSW_VLANFILTER_alloc_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_VLANFILTER_FREE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_Vlan_RMONControl_Set(const GSW_Device_t *dev, GSW_VLAN_RMON_control_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_VLAN_RMON_CONTROL_SET,
			    parm,
			    sizeof(*parm),
			    GSW_VLAN_RMON_CONTROL_GET,
			    0);
}

GSW_return_t GSW_Vlan_RMONControl_Get(const GSW_Device_t *dev, GSW_VLAN_RMON_control_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_VLAN_RMON_CONTROL_GET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_Vlan_RMON_Get(const GSW_Device_t *dev, GSW_VLAN_RMON_cnt_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_VLAN_RMON_GET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_Vlan_RMON_Clear(const GSW_Device_t *dev, GSW_VLAN_RMON_cnt_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_VLAN_RMON_CLEAR,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_VlanCounterMapSet(const GSW_Device_t *dev, GSW_VlanCounterMapping_config_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_VLAN_COUNTER_MAPPING_SET,
			    parm,
			    sizeof(*parm),
			    GSW_VLAN_COUNTER_MAPPING_GET,
			    0);
}

GSW_return_t GSW_VlanCounterMapGet(const GSW_Device_t *dev, GSW_VlanCounterMapping_config_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_VLAN_COUNTER_MAPPING_GET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}
#endif /* CONFIG_GSWIP_EVLAN */

GSW_return_t GSW_MulticastRouterPortAdd(const GSW_Device_t *dev, GSW_multicastRouter_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MULTICAST_ROUTERPORTADD,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_MulticastRouterPortRead(const GSW_Device_t *dev, GSW_multicastRouterRead_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MULTICAST_ROUTERPORTREAD,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_MulticastRouterPortRemove(const GSW_Device_t *dev, GSW_multicastRouter_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MULTICAST_ROUTERPORTREMOVE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_MulticastSnoopCfgGet(const GSW_Device_t *dev, GSW_multicastSnoopCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MULTICAST_SNOOPCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_MulticastSnoopCfgSet(const GSW_Device_t *dev, GSW_multicastSnoopCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MULTICAST_SNOOPCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_MULTICAST_SNOOPCFGGET,
			    0);
}

GSW_return_t GSW_MulticastTableEntryAdd(const GSW_Device_t *dev, GSW_multicastTable_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MULTICAST_TABLEENTRYADD,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_MulticastTableEntryRead(const GSW_Device_t *dev, GSW_multicastTableRead_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MULTICAST_TABLEENTRYREAD,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_MulticastTableEntryRemove(const GSW_Device_t *dev, GSW_multicastTable_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_MULTICAST_TABLEENTRYREMOVE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_TrunkingCfgGet(const GSW_Device_t *dev, GSW_trunkingCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_TRUNKING_CFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_TrunkingCfgSet(const GSW_Device_t *dev, GSW_trunkingCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_TRUNKING_CFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_TRUNKING_CFGGET,
			    0);
}

GSW_return_t GSW_STP_PortCfgGet(const GSW_Device_t *dev, GSW_STP_portCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_STP_PORTCFGGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_STP_PortCfgSet(const GSW_Device_t *dev, GSW_STP_portCfg_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_STP_PORTCFGSET,
			    parm,
			    sizeof(*parm),
			    GSW_STP_PORTCFGGET,
			    0);
}

GSW_return_t GSW_STP_BPDU_RuleGet(const GSW_Device_t *dev, GSW_STP_BPDU_Rule_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_STP_BPDU_RULEGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_STP_BPDU_RuleSet(const GSW_Device_t *dev, GSW_STP_BPDU_Rule_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_STP_BPDU_RULESET,
			    parm,
			    sizeof(*parm),
			    GSW_STP_BPDU_RULEGET,
			    0);
}

GSW_return_t GSW_PBB_TunnelTempate_Alloc(const GSW_Device_t *dev, GSW_PBB_Tunnel_Template_Config_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_PBB_TEMPLATEALLOC,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

GSW_return_t GSW_PBB_TunnelTempate_Free(const GSW_Device_t *dev, GSW_PBB_Tunnel_Template_Config_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_PBB_TEMPLATEFREE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

GSW_return_t GSW_PBB_TunnelTempate_Config_Set(const GSW_Device_t *dev, GSW_PBB_Tunnel_Template_Config_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_PBB_TEMPLATESET,
			    parm,
			    sizeof(*parm),
			    GSW_PBB_TEMPLATEGET,
			    0);
}

GSW_return_t GSW_PBB_TunnelTempate_Config_Get(const GSW_Device_t *dev, GSW_PBB_Tunnel_Template_Config_t *parm)
{
	return gsw_api_wrap(dev,
			    GSW_PBB_TEMPLATEGET,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(*parm));
}

