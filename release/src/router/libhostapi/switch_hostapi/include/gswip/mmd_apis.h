/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _MXL_MMD_APIS_H_
#define _MXL_MMD_APIS_H_

#include "gsw_api.h"
#include "gsw_ss.h"
#include "mdio_relay.h"
#include "sys_led.h"
#include "sys_misc.h"
#include "loop_detect_cfg.h"

#define GSW_MMD_SMDIO_DEV			0
#define GSW_MMD_DEV				30
#define GSW_MMD_REG_CTRL			0
#define GSW_MMD_REG_LEN_RET			1
#define GSW_MMD_REG_DATA_FIRST			2
#define GSW_MMD_REG_DATA_LAST			95
#define GSW_MMD_REG_DATA_MAX_SIZE		\
	(GSW_MMD_REG_DATA_LAST - GSW_MMD_REG_DATA_FIRST + 1)

typedef union mmd_api_data {
	uint16_t data[GSW_MMD_REG_DATA_MAX_SIZE * 3]; //Maximum data size is GSW_PCE_rule_t (508)
	GSW_register_t GSW_register_t_data;
	GSW_register_mod_t GSW_register_mod_t_data;
	GSW_CPU_Port_t GSW_CPU_Port_t_data;
	GSW_CPU_PortCfg_t GSW_CPU_PortCfg_t_data;
	GSW_portLinkCfg_t GSW_portLinkCfg_t_data;
	GSW_portCfg_t GSW_portCfg_t_data;
	GSW_cfg_t GSW_cfg_t_data;
	GSW_monitorPortCfg_t GSW_monitorPortCfg_t_data;
	GSW_PCE_rule_t GSW_PCE_rule_t_data;
	GSW_PCE_ruleEntry_t GSW_PCE_ruleEntry_t_data;
	GSW_PCE_rule_alloc_t GSW_PCE_rule_alloc_t_data;
	GSW_PCE_rule_move_t GSW_PCE_rule_move_t_data;
	GSW_BRIDGE_config_t GSW_BRIDGE_config_t_data;
	GSW_BRIDGE_alloc_t GSW_BRIDGE_alloc_t_data;
	GSW_BRIDGE_portAlloc_t GSW_BRIDGE_portAlloc_t_data;
	GSW_BRIDGE_portConfig_t GSW_BRIDGE_portConfig_t_data;
	GSW_BRIDGE_portLoopRead_t GSW_BRIDGE_portLoopRead_t_data;
	GSW_HitStatusRead_t GSW_HitStatusRead_t_data;
	GSW_MiscPortCfg_t GSW_MiscPortCfg_t_data;
	GSW_CTP_portAssignment_t GSW_CTP_portAssignment_t_data;
	GSW_CTP_portConfig_t GSW_CTP_portConfig_t_data;
	GSW_QoS_DSCP_ClassCfg_t GSW_QoS_DSCP_ClassCfg_t_data;
	GSW_QoS_DSCP_DropPrecedenceCfg_t GSW_QoS_DSCP_DropPrecedenceCfg_t_data;
	GSW_QoS_portRemarkingCfg_t GSW_QoS_portRemarkingCfg_t_data;
	GSW_QoS_PCP_ClassCfg_t GSW_QoS_PCP_ClassCfg_t_data;
	GSW_QoS_portCfg_t GSW_QoS_portCfg_t_data;
	GSW_QoS_queuePort_t GSW_QoS_queuePort_t_data;
	GSW_QoS_schedulerCfg_t GSW_QoS_schedulerCfg_t_data;
	GSW_QoS_ShaperCfg_t GSW_QoS_ShaperCfg_t_data;
	GSW_QoS_ShaperQueue_t GSW_QoS_ShaperQueue_t_data;
	GSW_QoS_ShaperQueueGet_t GSW_QoS_ShaperQueueGet_t_data;
	GSW_QoS_stormCfg_t GSW_QoS_stormCfg_t_data;
	GSW_QoS_WRED_Cfg_t GSW_QoS_WRED_Cfg_t_data;
	GSW_QoS_WRED_QueueCfg_t GSW_QoS_WRED_QueueCfg_t_data;
	GSW_QoS_WRED_PortCfg_t GSW_QoS_WRED_PortCfg_t_data;
	GSW_QoS_FlowCtrlCfg_t GSW_QoS_FlowCtrlCfg_t_data;
	GSW_QoS_FlowCtrlPortCfg_t GSW_QoS_FlowCtrlPortCfg_t_data;
	GSW_QoS_QueueBufferReserveCfg_t GSW_QoS_QueueBufferReserveCfg_t_data;
	GSW_QoS_colorMarkingEntry_t GSW_QoS_colorMarkingEntry_t_data;
	GSW_QoS_colorRemarkingEntry_t GSW_QoS_colorRemarkingEntry_t_data;
	GSW_QoS_meterCfg_t GSW_QoS_meterCfg_t_data;
	GSW_DSCP2PCP_map_t GSW_DSCP2PCP_map_t_data;
	GSW_PMAPPER_t GSW_PMAPPER_t_data;
	GSW_QoS_SVLAN_PCP_ClassCfg_t GSW_QoS_SVLAN_PCP_ClassCfg_t_data;
	GSW_RMON_Port_cnt_t GSW_RMON_Port_cnt_t_data;
	GSW_RMON_mode_t GSW_RMON_mode_t_data;
	GSW_RMON_Meter_cnt_t GSW_RMON_Meter_cnt_t_data;
	GSW_RMON_clear_t GSW_RMON_clear_t_data;
	GSW_RMON_flowGet_t GSW_RMON_flowGet_t_data;
	GSW_TflowCmodeConf_t GSW_TflowCmodeConf_t_data;
	GSW_Debug_RMON_Port_cnt_t GSW_Debug_RMON_Port_cnt_t_data;
	GSW_table_t GSW_table_t_data;
	GSW_debug_t GSW_debug_t_data;
	GSW_PMAC_Cnt_t GSW_PMAC_Cnt_t_data;
	GSW_PMAC_Glbl_Cfg_t GSW_PMAC_Glbl_Cfg_t_data;
	GSW_PMAC_BM_Cfg_t GSW_PMAC_BM_Cfg_t_data;
	GSW_PMAC_Ig_Cfg_t GSW_PMAC_Ig_Cfg_t_data;
	GSW_PMAC_Eg_Cfg_t GSW_PMAC_Eg_Cfg_t_data;
	GSW_MAC_tableClearCond_t GSW_MAC_tableClearCond_t_data;
	GSW_MAC_tableAdd_t GSW_MAC_tableAdd_t_data;
	GSW_MAC_tableRead_t GSW_MAC_tableRead_t_data;
	GSW_MAC_tableQuery_t GSW_MAC_tableQuery_t_data;
	GSW_MAC_tableRemove_t GSW_MAC_tableRemove_t_data;
	GSW_MACFILTER_default_t GSW_MACFILTER_default_t_data;
	GSW_MAC_tableLoopDetect_t GSW_MAC_tableLoopDetect_t_data;
	GSW_EXTENDEDVLAN_config_t GSW_EXTENDEDVLAN_config_t_data;
	GSW_EXTENDEDVLAN_alloc_t GSW_EXTENDEDVLAN_alloc_t_data;
	GSW_VLANFILTER_config_t GSW_VLANFILTER_config_t_data;
	GSW_VLANFILTER_alloc_t GSW_VLANFILTER_alloc_t_data;
	GSW_VLAN_RMON_control_t GSW_VLAN_RMON_control_t_data;
	GSW_VLAN_RMON_cnt_t GSW_VLAN_RMON_cnt_t_data;
	GSW_VlanCounterMapping_config_t GSW_VlanCounterMapping_config_t_data;
	GSW_multicastRouterRead_t GSW_multicastRouterRead_t_data;
	GSW_multicastRouter_t GSW_multicastRouter_t_data;
	GSW_multicastSnoopCfg_t GSW_multicastSnoopCfg_t_data;
	GSW_multicastTableRead_t GSW_multicastTableRead_t_data;
	GSW_multicastTable_t GSW_multicastTable_t_data;
	GSW_trunkingCfg_t GSW_trunkingCfg_t_data;
	GSW_STP_portCfg_t GSW_STP_portCfg_t_data;
	GSW_STP_BPDU_Rule_t GSW_STP_BPDU_Rule_t_data;
	GSW_PBB_Tunnel_Template_Config_t GSW_PBB_Tunnel_Template_Config_t_data;

	struct gsw_ss_sptag gsw_ss_sptag_data;

	struct mdio_relay_data mdio_relay_data;
	struct mdio_relay_mod_data mdio_relay_mod_data;
	struct sys_fw_image_version img_ver_data;
	struct sys_sensor_value pvt_sensor_data;
	struct sys_delay delay_data;
	struct sys_gpio_config gpio_config_data;
	struct sys_reg_rw reg_rw_data;
	struct sys_reg_mod reg_mod_data;
	struct sys_cml_clk cml_clk_data;
	struct sys_sfp_cfg sfp_cfg_data;
	struct loop_detect_passive_cfg loop_detect_passive_cfg_data;
	struct loop_detect_active_cfg loop_detect_active_cfg_data;
	struct loop_prevention_cfg loop_prevention_cfg_data;
	struct mxl_led_sys_cfg mxl_led_sys_cfg_data;
} mmd_api_data_t;

#define GSW_COMMON_MAGIC			0x0100
#define GSW_TFLOW_MAGIC				0x0200
#define GSW_BRDG_MAGIC				0x0300
#define GSW_BRDGPORT_MAGIC			0x0400
#define GSW_CTP_MAGIC				0x0500
#define GSW_QOS_MAGIC				0x0600
#define GSW_RMON_MAGIC				0x0700
#define GSW_DEBUG_MAGIC				0x0800
#define GSW_PMAC_MAGIC				0x0900
#define GSW_SWMAC_MAGIC				0x0A00
#define GSW_EXTVLAN_MAGIC			0x0B00
#define GSW_VLANFILTER_MAGIC			0x0C00
#define GSW_MULTICAST_MAGIC			0x0D00
#define GSW_TRUNKING_MAGIC			0x0E00
#define GSW_STP_MAGIC				0x0F00
#define GSW_PBB_MAGIC				0x1000
#define GSW_VLAN_RMON_MAGIC			0x1100

#define GSW_SS_MAGIC				0x1600

#define GSW_MAC_MAGIC				0x1700

#define GPY_GPY2XX_MAGIC			0x1800

#define SYS_MISC_MAGIC				0x1900

#ifdef MMD_API_TEST
#define MMD_API_SIMPLE_TEST			(0x0 + 0x1)
#endif
#define MMD_API_SET_DATA_0			(0x0 + 0x2)
#define MMD_API_SET_DATA_1			(0x0 + 0x3)
#define MMD_API_SET_DATA_2			(0x0 + 0x4)
#define MMD_API_GET_DATA_0			(0x0 + 0x5)
#define MMD_API_GET_DATA_1			(0x0 + 0x6)
#define MMD_API_GET_DATA_2			(0x0 + 0x7)
#define MMD_API_RST_DATA			(0x0 + 0x8)

#define GSW_COMMON_REGISTERGET			(GSW_COMMON_MAGIC + 0x1)
#define	GSW_COMMON_REGISTERSET			(GSW_COMMON_MAGIC + 0x2)
#define	GSW_COMMON_CPU_PORTCFGGET		(GSW_COMMON_MAGIC + 0x3)
#define	GSW_COMMON_CPU_PORTCFGSET		(GSW_COMMON_MAGIC + 0x4)
#define	GSW_COMMON_PORTLINKCFGGET		(GSW_COMMON_MAGIC + 0x5)
#define	GSW_COMMON_PORTLINKCFGSET		(GSW_COMMON_MAGIC + 0x6)
#define	GSW_COMMON_PORTCFGGET			(GSW_COMMON_MAGIC + 0x7)
#define	GSW_COMMON_PORTCFGSET			(GSW_COMMON_MAGIC + 0x8)
#define	GSW_COMMON_CFGGET			(GSW_COMMON_MAGIC + 0x9)
#define	GSW_COMMON_CFGSET			(GSW_COMMON_MAGIC + 0xA)
#define	GSW_COMMON_MONITORPORTCFGGET		(GSW_COMMON_MAGIC + 0xD)
#define	GSW_COMMON_MONITORPORTCFGSET		(GSW_COMMON_MAGIC + 0xE)
#define	GSW_COMMON_FREEZE			(GSW_COMMON_MAGIC + 0xF)
#define	GSW_COMMON_UNFREEZE			(GSW_COMMON_MAGIC + 0x10)
#define	GSW_COMMON_REGISTERMOD			(GSW_COMMON_MAGIC + 0x11)
#define GSW_COMMON_GETHITSTS			(GSW_COMMON_MAGIC + 0x12)
#define GSW_COMMON_MISCPORTCFGGET		(GSW_COMMON_MAGIC + 0x13)
#define GSW_COMMON_MISCPORTCFGSET		(GSW_COMMON_MAGIC + 0x14)
#define	GSW_COMMON_CPU_PORTGET			(GSW_COMMON_MAGIC + 0x15)
#define	GSW_COMMON_CPU_PORTSET			(GSW_COMMON_MAGIC + 0x16)

#define	GSW_TFLOW_PCERULEREAD			(GSW_TFLOW_MAGIC + 0x1)
#define	GSW_TFLOW_PCERULEWRITE			(GSW_TFLOW_MAGIC + 0x2)
#define	GSW_TFLOW_PCERULEDELETE			(GSW_TFLOW_MAGIC + 0x3)
#define GSW_TFLOW_PCERULEALLOC			(GSW_TFLOW_MAGIC + 0x4)
#define GSW_TFLOW_PCERULEFREE			(GSW_TFLOW_MAGIC + 0x5)
#define GSW_TFLOW_PCERULEENABLE			(GSW_TFLOW_MAGIC + 0x6)
#define GSW_TFLOW_PCERULEDISABLE		(GSW_TFLOW_MAGIC + 0x7)
#define GSW_TFLOW_PCERULEBLOCKSIZE		(GSW_TFLOW_MAGIC + 0x8)
#define GSW_TFLOW_PCERULEMOVE			(GSW_TFLOW_MAGIC + 0x9)

#define	GSW_BRIDGE_ALLOC			(GSW_BRDG_MAGIC + 0x1)
#define	GSW_BRIDGE_CONFIGSET			(GSW_BRDG_MAGIC + 0x2)
#define	GSW_BRIDGE_CONFIGGET			(GSW_BRDG_MAGIC + 0x3)
#define	GSW_BRIDGE_FREE				(GSW_BRDG_MAGIC	+ 0x4)

#define	GSW_BRIDGEPORT_ALLOC			(GSW_BRDGPORT_MAGIC + 0x1)
#define	GSW_BRIDGEPORT_CONFIGSET		(GSW_BRDGPORT_MAGIC + 0x2)
#define	GSW_BRIDGEPORT_CONFIGGET		(GSW_BRDGPORT_MAGIC + 0x3)
#define	GSW_BRIDGEPORT_FREE			(GSW_BRDGPORT_MAGIC + 0x4)
#define	GSW_BRIDGEPORT_LOOPREAD			(GSW_BRDGPORT_MAGIC + 0x5)

#define	GSW_CTP_PORTASSIGNMENTALLOC		(GSW_CTP_MAGIC + 0x1)
#define	GSW_CTP_PORTASSIGNMENTFREE		(GSW_CTP_MAGIC + 0x2)
#define	GSW_CTP_PORTASSIGNMENTSET		(GSW_CTP_MAGIC + 0x3)
#define	GSW_CTP_PORTASSIGNMENTGET		(GSW_CTP_MAGIC + 0x4)
#define	GSW_CTP_PORTCONFIGSET			(GSW_CTP_MAGIC + 0x5)
#define	GSW_CTP_PORTCONFIGGET			(GSW_CTP_MAGIC + 0x6)
#define	GSW_CTP_PORTCONFIGRESET			(GSW_CTP_MAGIC + 0x7)

#define	GSW_QOS_METERCFGGET			(GSW_QOS_MAGIC + 0x1)
#define	GSW_QOS_METERCFGSET			(GSW_QOS_MAGIC + 0x2)
#define	GSW_QOS_DSCP_CLASSGET			(GSW_QOS_MAGIC + 0x4)
#define	GSW_QOS_DSCP_CLASSSET			(GSW_QOS_MAGIC + 0x5)
#define	GSW_QOS_DSCP_DROPPRECEDENCECFGGET	(GSW_QOS_MAGIC + 0x6)
#define	GSW_QOS_DSCP_DROPPRECEDENCECFGSET	(GSW_QOS_MAGIC + 0x7)
#define	GSW_QOS_PORTREMARKINGCFGGET		(GSW_QOS_MAGIC + 0x8)
#define	GSW_QOS_PORTREMARKINGCFGSET		(GSW_QOS_MAGIC + 0x9)
#define	GSW_QOS_PCP_CLASSGET			(GSW_QOS_MAGIC + 0xA)
#define	GSW_QOS_PCP_CLASSSET			(GSW_QOS_MAGIC + 0xB)
#define	GSW_QOS_PORTCFGGET			(GSW_QOS_MAGIC + 0xC)
#define	GSW_QOS_PORTCFGSET			(GSW_QOS_MAGIC + 0xD)
#define	GSW_QOS_QUEUEPORTGET			(GSW_QOS_MAGIC + 0xE)
#define	GSW_QOS_QUEUEPORTSET			(GSW_QOS_MAGIC + 0xF)
#define	GSW_QOS_SCHEDULERCFGGET			(GSW_QOS_MAGIC + 0x10)
#define	GSW_QOS_SCHEDULERCFGSET			(GSW_QOS_MAGIC + 0x11)
#define	GSW_QOS_SHAPERCFGGET			(GSW_QOS_MAGIC + 0x12)
#define	GSW_QOS_SHAPERCFGSET			(GSW_QOS_MAGIC + 0x13)
#define	GSW_QOS_SHAPERQUEUEASSIGN		(GSW_QOS_MAGIC + 0x14)
#define	GSW_QOS_SHAPERQUEUEDEASSIGN		(GSW_QOS_MAGIC + 0x15)
#define	GSW_QOS_SHAPERQUEUEGET			(GSW_QOS_MAGIC + 0x16)
#define	GSW_QOS_STORMCFGSET			(GSW_QOS_MAGIC + 0x17)
#define	GSW_QOS_STORMCFGGET			(GSW_QOS_MAGIC + 0x18)
#define	GSW_QOS_WREDCFGGET			(GSW_QOS_MAGIC + 0x19)
#define	GSW_QOS_WREDCFGSET			(GSW_QOS_MAGIC + 0x1A)
#define	GSW_QOS_WREDQUEUECFGGET			(GSW_QOS_MAGIC + 0x1B)
#define	GSW_QOS_WREDQUEUECFGSET			(GSW_QOS_MAGIC + 0x1C)
#define	GSW_QOS_WREDPORTCFGGET			(GSW_QOS_MAGIC + 0x1D)
#define	GSW_QOS_WREDPORTCFGSET			(GSW_QOS_MAGIC + 0x1E)
#define	GSW_QOS_FLOWCTRLCFGGET			(GSW_QOS_MAGIC + 0x1F)
#define	GSW_QOS_FLOWCTRLCFGSET			(GSW_QOS_MAGIC + 0x20)
#define	GSW_QOS_FLOWCTRLPORTCFGGET		(GSW_QOS_MAGIC + 0x21)
#define	GSW_QOS_FLOWCTRLPORTCFGSET		(GSW_QOS_MAGIC + 0x22)
#define	GSW_QOS_QUEUEBUFFERRESERVECFGGET	(GSW_QOS_MAGIC + 0x23)
#define	GSW_QOS_QUEUEBUFFERRESERVECFGSET	(GSW_QOS_MAGIC + 0x24)
#define	GSW_QOS_COLORMARKINGTABLEGET		(GSW_QOS_MAGIC + 0x26)
#define	GSW_QOS_COLORMARKINGTABLESET		(GSW_QOS_MAGIC + 0x27)
#define	GSW_QOS_COLORREMARKINGTABLESET		(GSW_QOS_MAGIC + 0x28)
#define	GSW_QOS_COLORREMARKINGTABLEGET		(GSW_QOS_MAGIC + 0x29)
#define	GSW_QOS_METERALLOC			(GSW_QOS_MAGIC + 0x2A)
#define	GSW_QOS_METERFREE			(GSW_QOS_MAGIC + 0x2B)
#define	GSW_QOS_DSCP2PCPTABLESET		(GSW_QOS_MAGIC + 0x2C)
#define	GSW_QOS_DSCP2PCPTABLEGET		(GSW_QOS_MAGIC + 0x2D)
#define	GSW_QOS_PMAPPERTABLESET			(GSW_QOS_MAGIC + 0x2E)
#define	GSW_QOS_PMAPPERTABLEGET			(GSW_QOS_MAGIC + 0x2F)
#define	GSW_QOS_SVLAN_PCP_CLASSGET		(GSW_QOS_MAGIC + 0x30)
#define	GSW_QOS_SVLAN_PCP_CLASSSET		(GSW_QOS_MAGIC + 0x31)
#define	GSW_QOS_QUEUECFGGET			(GSW_QOS_MAGIC + 0x32)
#define	GSW_QOS_QUEUECFGSET			(GSW_QOS_MAGIC + 0x33)

#define	GSW_RMON_PORT_GET			(GSW_RMON_MAGIC + 0x1)
#define	GSW_RMON_MODE_SET			(GSW_RMON_MAGIC + 0x2)
#define	GSW_RMON_METER_GET			(GSW_RMON_MAGIC + 0x3)
#define	GSW_RMON_CLEAR				(GSW_RMON_MAGIC + 0x4)
#define	GSW_RMON_TFLOWGET			(GSW_RMON_MAGIC + 0x5)
#define	GSW_RMON_TFLOWCLEAR			(GSW_RMON_MAGIC + 0x6)
#define	GSW_RMON_TFLOWCOUNTMODESET		(GSW_RMON_MAGIC + 0x7)
#define	GSW_RMON_TFLOWCOUNTMODEGET		(GSW_RMON_MAGIC + 0x8)

#define	GSW_DEBUG_RMON_PORT_GET			(GSW_DEBUG_MAGIC + 0x1)

#define	GSW_PMAC_COUNTGET			(GSW_PMAC_MAGIC + 0x1)
#define	GSW_PMAC_GBL_CFGSET			(GSW_PMAC_MAGIC + 0x2)
#define	GSW_PMAC_GBL_CFGGET			(GSW_PMAC_MAGIC + 0x3)
#define	GSW_PMAC_BM_CFGSET			(GSW_PMAC_MAGIC + 0x4)
#define	GSW_PMAC_BM_CFGGET			(GSW_PMAC_MAGIC + 0x5)
#define	GSW_PMAC_IG_CFGSET			(GSW_PMAC_MAGIC + 0x6)
#define	GSW_PMAC_IG_CFGGET			(GSW_PMAC_MAGIC + 0x7)
#define	GSW_PMAC_EG_CFGSET			(GSW_PMAC_MAGIC + 0x8)
#define	GSW_PMAC_EG_CFGGET			(GSW_PMAC_MAGIC + 0x9)

#define	GSW_MAC_TABLECLEAR			(GSW_SWMAC_MAGIC + 0x1)
#define	GSW_MAC_TABLEENTRYADD			(GSW_SWMAC_MAGIC + 0x2)
#define	GSW_MAC_TABLEENTRYREAD			(GSW_SWMAC_MAGIC + 0x3)
#define	GSW_MAC_TABLEENTRYQUERY			(GSW_SWMAC_MAGIC + 0x4)
#define	GSW_MAC_TABLEENTRYREMOVE		(GSW_SWMAC_MAGIC + 0x5)
#define	GSW_MAC_DEFAULTFILTERSET		(GSW_SWMAC_MAGIC + 0x6)
#define	GSW_MAC_DEFAULTFILTERGET		(GSW_SWMAC_MAGIC + 0x7)
#define	GSW_MAC_TABLECLEARCOND			(GSW_SWMAC_MAGIC + 0x8)
#define GSW_MAC_TABLE_LOOP_DETECT		(GSW_SWMAC_MAGIC + 0x9)

#define	GSW_EXTENDEDVLAN_ALLOC			(GSW_EXTVLAN_MAGIC + 0x1)
#define	GSW_EXTENDEDVLAN_SET			(GSW_EXTVLAN_MAGIC + 0x2)
#define	GSW_EXTENDEDVLAN_GET			(GSW_EXTVLAN_MAGIC + 0x3)
#define	GSW_EXTENDEDVLAN_FREE			(GSW_EXTVLAN_MAGIC + 0x4)

#define	GSW_VLANFILTER_ALLOC			(GSW_VLANFILTER_MAGIC + 0x1)
#define	GSW_VLANFILTER_SET			(GSW_VLANFILTER_MAGIC + 0x2)
#define	GSW_VLANFILTER_GET			(GSW_VLANFILTER_MAGIC + 0x3)
#define	GSW_VLANFILTER_FREE			(GSW_VLANFILTER_MAGIC + 0x4)

#define	GSW_VLAN_COUNTER_MAPPING_SET 		(GSW_VLAN_RMON_MAGIC + 0x1)
#define	GSW_VLAN_COUNTER_MAPPING_GET		(GSW_VLAN_RMON_MAGIC + 0x2)
#define	GSW_VLAN_RMON_GET			(GSW_VLAN_RMON_MAGIC + 0x3)
#define	GSW_VLAN_RMON_CLEAR			(GSW_VLAN_RMON_MAGIC + 0x4)
#define	GSW_VLAN_RMON_CONTROL_SET		(GSW_VLAN_RMON_MAGIC + 0x5)
#define	GSW_VLAN_RMON_CONTROL_GET		(GSW_VLAN_RMON_MAGIC + 0x6)

#define	GSW_MULTICAST_ROUTERPORTADD		(GSW_MULTICAST_MAGIC + 0x1)
#define	GSW_MULTICAST_ROUTERPORTREAD		(GSW_MULTICAST_MAGIC + 0x2)
#define	GSW_MULTICAST_ROUTERPORTREMOVE		(GSW_MULTICAST_MAGIC + 0x3)
#define	GSW_MULTICAST_SNOOPCFGGET		(GSW_MULTICAST_MAGIC + 0x4)
#define	GSW_MULTICAST_SNOOPCFGSET		(GSW_MULTICAST_MAGIC + 0x5)
#define	GSW_MULTICAST_TABLEENTRYADD		(GSW_MULTICAST_MAGIC + 0x6)
#define	GSW_MULTICAST_TABLEENTRYREAD		(GSW_MULTICAST_MAGIC + 0x7)
#define	GSW_MULTICAST_TABLEENTRYREMOVE		(GSW_MULTICAST_MAGIC + 0x8)

#define	GSW_TRUNKING_CFGGET			(GSW_TRUNKING_MAGIC + 0x1)
#define	GSW_TRUNKING_CFGSET			(GSW_TRUNKING_MAGIC + 0x2)
#define	GSW_TRUNKING_PORTCFGGET			(GSW_TRUNKING_MAGIC + 0x3)
#define	GSW_TRUNKING_PORTCFGSET			(GSW_TRUNKING_MAGIC + 0x4)

#define	GSW_STP_PORTCFGGET			(GSW_STP_MAGIC + 0x1)
#define	GSW_STP_PORTCFGSET			(GSW_STP_MAGIC + 0x2)
#define	GSW_STP_BPDU_RULEGET			(GSW_STP_MAGIC + 0x3)
#define	GSW_STP_BPDU_RULESET			(GSW_STP_MAGIC + 0x4)

#define	GSW_PBB_TEMPLATEALLOC			(GSW_PBB_MAGIC + 0x1)
#define	GSW_PBB_TEMPLATEFREE			(GSW_PBB_MAGIC + 0x2)
#define	GSW_PBB_TEMPLATESET			(GSW_PBB_MAGIC + 0x3)
#define	GSW_PBB_TEMPLATEGET			(GSW_PBB_MAGIC + 0x4)

#define GSW_SS_SPTAG_GET			(GSW_SS_MAGIC + 0x01)
#define GSW_SS_SPTAG_SET			(GSW_SS_MAGIC + 0x02)

#define INT_GPHY_READ				(GPY_GPY2XX_MAGIC + 0x01)
#define INT_GPHY_WRITE				(GPY_GPY2XX_MAGIC + 0x02)
#define INT_GPHY_MOD				(GPY_GPY2XX_MAGIC + 0x03)
#define EXT_MDIO_READ				(GPY_GPY2XX_MAGIC + 0x11)
#define EXT_MDIO_WRITE				(GPY_GPY2XX_MAGIC + 0x12)
#define EXT_MDIO_MOD				(GPY_GPY2XX_MAGIC + 0x13)

#define SYS_MISC_FW_UPDATE			(SYS_MISC_MAGIC + 0x01)
#define SYS_MISC_FW_VERSION			(SYS_MISC_MAGIC + 0x02)
#define SYS_MISC_PVT_TEMP			(SYS_MISC_MAGIC + 0x03)
#define SYS_MISC_PVT_VOLTAGE			(SYS_MISC_MAGIC + 0x04)
#define SYS_MISC_DELAY				(SYS_MISC_MAGIC + 0x05)
#define SYS_MISC_GPIO_CONFIGURE			(SYS_MISC_MAGIC + 0x06)
#define SYS_MISC_REBOOT				(SYS_MISC_MAGIC + 0x07)
#define SYS_MISC_REG_RD				(SYS_MISC_MAGIC + 0x08)
#define SYS_MISC_REG_WR				(SYS_MISC_MAGIC + 0x09)
#define SYS_MISC_REG_MOD			(SYS_MISC_MAGIC + 0x0A)
#define SYS_MISC_CML_CLK_GET			(SYS_MISC_MAGIC + 0x0B)
#define SYS_MISC_CML_CLK_SET			(SYS_MISC_MAGIC + 0x0C)
#define SYS_MISC_SFP_GET			(SYS_MISC_MAGIC + 0x0D)
#define SYS_MISC_SFP_SET			(SYS_MISC_MAGIC + 0x0E)
#define SYS_MISC_LOOP_PASSIVE_START		(SYS_MISC_MAGIC + 0x13)
#define SYS_MISC_LOOP_PASSIVE_STOP		(SYS_MISC_MAGIC + 0x14)
#define SYS_MISC_LOOP_ACTIVE_START		(SYS_MISC_MAGIC + 0x15)
#define SYS_MISC_LOOP_ACTIVE_STOP		(SYS_MISC_MAGIC + 0x16)
#define SYS_MISC_LOOP_PREVENTION_START		(SYS_MISC_MAGIC + 0x17)
#define SYS_MISC_LOOP_PREVENTION_STOP		(SYS_MISC_MAGIC + 0x18)
#define SYS_MISC_SYS_LED_CFG			(SYS_MISC_MAGIC + 0x19)

#define	MMD_API_MAXIMUM_ID			0x7FFF

#endif /* _MXL_MMD_APIS_H_ */
