/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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


#ifndef _XRDP_DRV_BBH_RX_AG_H_
#define _XRDP_DRV_BBH_RX_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    bdmf_boolean dispdropdis;
} bbh_rx_dispatcher_drop_disable;

typedef struct
{
    uint32_t patterndatalsb;
    uint32_t patterndatamsb;
    uint32_t patternmasklsb;
    uint32_t patternmaskmsb;
    uint8_t pattenoffset;
} bbh_rx_pattern_recog;

typedef struct
{
    bdmf_boolean dispdropdis;
    bdmf_boolean sdmadropdis;
    bdmf_boolean sbpmdropdis;
} bbh_rx_flow_ctrl_drops_config;

typedef struct
{
    uint8_t numofcd;
    uint8_t exclth;
    uint8_t database;
    uint8_t descbase;
} bbh_rx_sdma_config;

typedef struct
{
    uint32_t crc_err_ploam;
    uint32_t third_flow;
    uint32_t sop_after_sop;
    uint32_t no_sbpm_bn_ploam;
} bbh_rx_error_pm_counters;

typedef struct
{
    uint32_t inpkt;
    uint32_t inbyte;
    uint32_t crc_err;
    uint32_t too_short;
    uint32_t too_long;
    uint32_t no_sbpm_sbn;
    uint32_t disp_cong;
    uint32_t no_sdma_cd;
    uint32_t ploam_no_sdma_cd;
    uint32_t ploam_disp_cong;
} bbh_rx_pm_counters;

typedef struct
{
    bdmf_boolean macmode;
    bdmf_boolean gponmode;
    bdmf_boolean macvdsl;
    bdmf_boolean maciswanen;
    bdmf_boolean maciswan;
} bbh_rx_mac_mode;

typedef struct
{
    bdmf_boolean inbufrst;
    bdmf_boolean burstbufrst;
    bdmf_boolean ingresscntxt;
    bdmf_boolean cmdfiforst;
    bdmf_boolean sbpmfiforst;
    bdmf_boolean coherencyfiforst;
    bdmf_boolean cntxtrst;
    bdmf_boolean sdmarst;
    bdmf_boolean dispnormal;
    bdmf_boolean dispexclusive;
    bdmf_boolean upldfiforst;
    uint8_t dispcredit;
} bbh_rx_general_configuration_rxrstrst;

typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} bbh_rx_general_configuration_clk_gate_cntrl;

typedef struct
{
    bdmf_boolean inreass;
    bdmf_boolean sop;
    uint8_t priority;
    uint8_t flowid;
    uint16_t curoffset;
} bbh_rx_debug_cntxtx0ingress;

typedef struct
{
    bdmf_boolean inreass;
    bdmf_boolean sop;
    uint8_t priority;
    uint8_t flowid;
    uint16_t curoffset;
} bbh_rx_debug_cntxtx1ingress;

typedef struct
{
    uint32_t sbn_fifo[8];
} bbh_rx_debug_sbnfifo;

typedef struct
{
    uint32_t cmd_fifo[4];
} bbh_rx_debug_cmdfifo;

typedef struct
{
    uint32_t sbn_recycle_fifo[2];
} bbh_rx_debug_sbnrecyclefifo;


/**********************************************************************************************************************
 * ploamen: 
 *     Direct this packet type to Exclusive VIQ in the Dispatcher
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_ploam_en_set(uint8_t bbh_id, bdmf_boolean ploamen);
bdmf_error_t ag_drv_bbh_rx_ploam_en_get(uint8_t bbh_id, bdmf_boolean *ploamen);

/**********************************************************************************************************************
 * pri3en: 
 *     Direct this packet type to Exclusive VIQ in the Dispatcher
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_user_priority3_en_set(uint8_t bbh_id, bdmf_boolean pri3en);
bdmf_error_t ag_drv_bbh_rx_user_priority3_en_get(uint8_t bbh_id, bdmf_boolean *pri3en);

/**********************************************************************************************************************
 * pauseen: 
 *     Direct this packet type to Exclusive VIQ in the Dispatcher
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pause_en_set(uint8_t bbh_id, bdmf_boolean pauseen);
bdmf_error_t ag_drv_bbh_rx_pause_en_get(uint8_t bbh_id, bdmf_boolean *pauseen);

/**********************************************************************************************************************
 * pfcen: 
 *     Direct this packet type to Exclusive VIQ in the Dispatcher
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pfc_en_set(uint8_t bbh_id, bdmf_boolean pfcen);
bdmf_error_t ag_drv_bbh_rx_pfc_en_get(uint8_t bbh_id, bdmf_boolean *pfcen);

/**********************************************************************************************************************
 * ctrlen: 
 *     Direct this packet type to Exclusive VIQ in the Dispatcher
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_ctrl_en_set(uint8_t bbh_id, bdmf_boolean ctrlen);
bdmf_error_t ag_drv_bbh_rx_ctrl_en_get(uint8_t bbh_id, bdmf_boolean *ctrlen);

/**********************************************************************************************************************
 * patternen: 
 *     Must be enabled if pattern recognition is used
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pattern_en_set(uint8_t bbh_id, bdmf_boolean patternen);
bdmf_error_t ag_drv_bbh_rx_pattern_en_get(uint8_t bbh_id, bdmf_boolean *patternen);

/**********************************************************************************************************************
 * excen: 
 *     Must be enabled if Exclusive VIQ is used
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_exc_en_set(uint8_t bbh_id, bdmf_boolean excen);
bdmf_error_t ag_drv_bbh_rx_exc_en_get(uint8_t bbh_id, bdmf_boolean *excen);

/**********************************************************************************************************************
 * timer: 
 *     Timer value before de-asserting the flow control indication.
 *     The duration of the time is determined according to the BBH clock frequency.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_timer_set(uint8_t bbh_id, uint32_t timer);
bdmf_error_t ag_drv_bbh_rx_timer_get(uint8_t bbh_id, uint32_t *timer);

/**********************************************************************************************************************
 * dispdropdis: 
 *     Disable dropping packets due to no space in the Dispatcher.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_dispatcher_drop_disable_set(uint8_t bbh_id, const bbh_rx_dispatcher_drop_disable *dispatcher_drop_disable);
bdmf_error_t ag_drv_bbh_rx_dispatcher_drop_disable_get(uint8_t bbh_id, bbh_rx_dispatcher_drop_disable *dispatcher_drop_disable);

/**********************************************************************************************************************
 * sdmadropdis: 
 *     Disable dropping packets due to no space in the SDMA.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_sdma_drop_disable_set(uint8_t bbh_id, bdmf_boolean sdmadropdis);
bdmf_error_t ag_drv_bbh_rx_sdma_drop_disable_get(uint8_t bbh_id, bdmf_boolean *sdmadropdis);

/**********************************************************************************************************************
 * sbpmdropdis: 
 *     Disable dropping packets due to no space in the SBPM.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_sbpm_drop_disable_set(uint8_t bbh_id, bdmf_boolean sbpmdropdis);
bdmf_error_t ag_drv_bbh_rx_sbpm_drop_disable_get(uint8_t bbh_id, bdmf_boolean *sbpmdropdis);

/**********************************************************************************************************************
 * fcforce: 
 *     Asserting this bit will force a flow control indication towards the MAC
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_flow_control_force_set(uint8_t bbh_id, bdmf_boolean fcforce);
bdmf_error_t ag_drv_bbh_rx_flow_control_force_get(uint8_t bbh_id, bdmf_boolean *fcforce);

/**********************************************************************************************************************
 * fcrnren: 
 *     Enables Runner to send flow control messages
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_flow_control_runner_enable_set(uint8_t bbh_id, bdmf_boolean fcrnren);
bdmf_error_t ag_drv_bbh_rx_flow_control_runner_enable_get(uint8_t bbh_id, bdmf_boolean *fcrnren);

/**********************************************************************************************************************
 * fcqmen: 
 *     Enables QM to send flow control messages
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_flow_control_qm_enable_set(uint8_t bbh_id, bdmf_boolean fcqmen);
bdmf_error_t ag_drv_bbh_rx_flow_control_qm_enable_get(uint8_t bbh_id, bdmf_boolean *fcqmen);

/**********************************************************************************************************************
 * patterndatalsb: 
 *     Pattern Data[31:0]
 * patterndatamsb: 
 *     Pattern Data[63:32]
 * patternmasklsb: 
 *     Pattern mask[31:0]
 * patternmaskmsb: 
 *     Pattern Mask[63:32]
 * pattenoffset: 
 *     Defines the pattern recognition offset within the packet. Offset is 8 bytes resolution
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pattern_recog_set(uint8_t bbh_id, const bbh_rx_pattern_recog *pattern_recog);
bdmf_error_t ag_drv_bbh_rx_pattern_recog_get(uint8_t bbh_id, bbh_rx_pattern_recog *pattern_recog);

/**********************************************************************************************************************
 * timer: 
 *     Timer value before de-asserting the flow control indication.
 *     The duration of the time is determined according to the BBH clock frequency.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_timer_set(uint8_t bbh_id, uint32_t timer);
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_timer_get(uint8_t bbh_id, uint32_t *timer);

/**********************************************************************************************************************
 * fcforce: 
 *     Asserting this bit will force a flow control indication towards the MAC
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_force_set(uint8_t bbh_id, bdmf_boolean fcforce);
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_force_get(uint8_t bbh_id, bdmf_boolean *fcforce);

/**********************************************************************************************************************
 * fcrnren: 
 *     Enables Runner to send flow control messages
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_rnr_en_set(uint8_t bbh_id, bdmf_boolean fcrnren);
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_rnr_en_get(uint8_t bbh_id, bdmf_boolean *fcrnren);

/**********************************************************************************************************************
 * dispdropdis: 
 *     Disable dropping packets due to no space in the Dispatcher.
 * sdmadropdis: 
 *     Disable dropping packets due to no space in the SDMA.
 * sbpmdropdis: 
 *     Disable dropping packets due to no space in the SBPM.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_drops_config_set(uint8_t bbh_id, const bbh_rx_flow_ctrl_drops_config *flow_ctrl_drops_config);
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_drops_config_get(uint8_t bbh_id, bbh_rx_flow_ctrl_drops_config *flow_ctrl_drops_config);

/**********************************************************************************************************************
 * sdmabbid: 
 *     SDMA BB ID. This ID defines the BB ID of the SDMA that the BBH communicates with.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_sdma_bb_id_set(uint8_t bbh_id, uint8_t sdmabbid);
bdmf_error_t ag_drv_bbh_rx_sdma_bb_id_get(uint8_t bbh_id, uint8_t *sdmabbid);

/**********************************************************************************************************************
 * dispbbid: 
 *     Dispatcher BB ID. This ID defines the BB ID of the Dispatcher that the BBH communicates with.
 * sbpmbbid: 
 *     SBPM BB ID. This ID defines the BB ID of the SBPM that the BBH communicates with.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_dispatcher_sbpm_bb_id_set(uint8_t bbh_id, uint8_t dispbbid, uint8_t sbpmbbid);
bdmf_error_t ag_drv_bbh_rx_dispatcher_sbpm_bb_id_get(uint8_t bbh_id, uint8_t *dispbbid, uint8_t *sbpmbbid);

/**********************************************************************************************************************
 * normalviq: 
 *     Defines the Dispatchers Virtual Ingress Queue for normal packets
 * exclviq: 
 *     Defines the Dispatchers Virtual Ingress Queue for exclusive packets
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_dispatcher_virtual_queues_set(uint8_t bbh_id, uint8_t normalviq, uint8_t exclviq);
bdmf_error_t ag_drv_bbh_rx_dispatcher_virtual_queues_get(uint8_t bbh_id, uint8_t *normalviq, uint8_t *exclviq);

/**********************************************************************************************************************
 * numofcd: 
 *     Defines the size of the Chunk descripors FIFO in the DMA.
 * exclth: 
 *     This field defines the number of occupied write chunks for dropping normal or high priority packets.
 * database: 
 *     The Data FIFO base address within the SDMA address space.
 *     The address is in chunk resolution (128 bytes).
 *     The value should be identical to the relevant configuration in the SDMA.
 * descbase: 
 *     The Descriptor FIFO base address within the SDMA address space.
 *     The address is in chunk descriptor resolution (8 bytes).
 *     The value  should be identical to the relevant configuration in the SDMA.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_sdma_config_set(uint8_t bbh_id, const bbh_rx_sdma_config *sdma_config);
bdmf_error_t ag_drv_bbh_rx_sdma_config_get(uint8_t bbh_id, bbh_rx_sdma_config *sdma_config);

/**********************************************************************************************************************
 * minpkt0: 
 *     Packets shorter than this threshold will be discarded.
 * maxpkt0: 
 *     Packets longer than this threshold will be discarded.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pkt_size0_set(uint8_t bbh_id, uint8_t minpkt0, uint16_t maxpkt0);
bdmf_error_t ag_drv_bbh_rx_pkt_size0_get(uint8_t bbh_id, uint8_t *minpkt0, uint16_t *maxpkt0);

/**********************************************************************************************************************
 * minpkt1: 
 *     Packets shorter than this threshold will be discarded.
 * maxpkt1: 
 *     Packets longer than this threshold will be discarded.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pkt_size1_set(uint8_t bbh_id, uint8_t minpkt1, uint16_t maxpkt1);
bdmf_error_t ag_drv_bbh_rx_pkt_size1_get(uint8_t bbh_id, uint8_t *minpkt1, uint16_t *maxpkt1);

/**********************************************************************************************************************
 * minpkt2: 
 *     Packets shorter than this threshold will be discarded.
 * maxpkt2: 
 *     Packets longer than this threshold will be discarded.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pkt_size2_set(uint8_t bbh_id, uint8_t minpkt2, uint16_t maxpkt2);
bdmf_error_t ag_drv_bbh_rx_pkt_size2_get(uint8_t bbh_id, uint8_t *minpkt2, uint16_t *maxpkt2);

/**********************************************************************************************************************
 * minpkt3: 
 *     Packets shorter than this threshold will be discarded.
 * maxpkt3: 
 *     Packets longer than this threshold will be discarded.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pkt_size3_set(uint8_t bbh_id, uint8_t minpkt3, uint16_t maxpkt3);
bdmf_error_t ag_drv_bbh_rx_pkt_size3_get(uint8_t bbh_id, uint8_t *minpkt3, uint16_t *maxpkt3);

/**********************************************************************************************************************
 * minpktsel0: 
 *     Set 0 of the general configuration.
 *     Selects between 4 global minimum packet size.
 * maxpktsel0: 
 *     Set 0 of the general configuration.
 *     Selects between 4 global maximum packet size.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pkt_sel_group_0_set(uint8_t bbh_id, uint8_t minpktsel0, uint8_t maxpktsel0);
bdmf_error_t ag_drv_bbh_rx_pkt_sel_group_0_get(uint8_t bbh_id, uint8_t *minpktsel0, uint8_t *maxpktsel0);

/**********************************************************************************************************************
 * minpktsel1: 
 *     Set 1 of the general configuration.
 *     Selects between 4 global minimum packet size.
 * maxpktsel1: 
 *     Set 1 of the general configuration.
 *     Selects between 4 global maximum packet size.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pkt_sel_group_1_set(uint8_t bbh_id, uint8_t minpktsel1, uint8_t maxpktsel1);
bdmf_error_t ag_drv_bbh_rx_pkt_sel_group_1_get(uint8_t bbh_id, uint8_t *minpktsel1, uint8_t *maxpktsel1);

/**********************************************************************************************************************
 * macflow: 
 *     Configured FW value for MAC flow field in the PD (previously was BB ID)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_mac_flow_set(uint8_t bbh_id, uint8_t macflow);
bdmf_error_t ag_drv_bbh_rx_mac_flow_get(uint8_t bbh_id, uint8_t *macflow);

/**********************************************************************************************************************
 * crc_err_ploam: 
 *     PM counter value.
 * third_flow: 
 *     PM counter value.
 * sop_after_sop: 
 *     PM counter value.
 * no_sbpm_bn_ploam: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_error_pm_counters_get(uint8_t bbh_id, bbh_rx_error_pm_counters *error_pm_counters);

/**********************************************************************************************************************
 * inpkt: 
 *     This counter counts the number of incoming good packets.
 * inbyte: 
 *     This counter counts the number of incoming good bytes.
 * crc_err: 
 *     PM counter value.
 * too_short: 
 *     PM counter value.
 * too_long: 
 *     PM counter value.
 * no_sbpm_sbn: 
 *     PM counter value.
 * disp_cong: 
 *     PM counter value.
 * no_sdma_cd: 
 *     PM counter value.
 * ploam_no_sdma_cd: 
 *     PM counter value.
 * ploam_disp_cong: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_get(uint8_t bbh_id, bbh_rx_pm_counters *pm_counters);

/**********************************************************************************************************************
 * macmode: 
 *     Relevant for PON BBH only.
 *     Distinguish between GPON (GPON, XGPON, NGPON2) to EPON (EPON, 10GEPON):
 *     0: N/X/GPON/2
 *     1: 10G/EPON
 * gponmode: 
 *     Relevant for GPON BBH only.
 *     Distinguish between GPON and XGPON (XGPON, NGPON2):
 *     0: GPON
 *     1: N/X/GPON/2
 * macvdsl: 
 *     Relevant for VDSL BBH only.
 *     Distinguish between VDSL and non VDSL:
 *     0: Non VDSL
 *     1: VDSL
 * maciswanen: 
 *     This configuration enables a global configuration per BBH to determine the WAN/LAN bit in the PD according to
 *     Mac_is_WAN.
 *     If disabled (default), The WAN/LAN bit in the PD is set to 1 (LAN) for Ethernet ports and to 0 (WAN) for
 *     non-Ethernet ports.
 * maciswan: 
 *     Determine the WAN/LAN bit in the PD.
 *     If the feature is enabled and the value is asserted (==1) => WAN_LAN == 0
 *     Enabled by Mac_is_WAN_EN.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_mac_mode_set(uint8_t bbh_id, const bbh_rx_mac_mode *mac_mode);
bdmf_error_t ag_drv_bbh_rx_mac_mode_get(uint8_t bbh_id, bbh_rx_mac_mode *mac_mode);

/**********************************************************************************************************************
 * sopoffset: 
 *     The SOP offset in bytes.
 *     Allowed values: 0-127.
 *     This value should match the relevant configuration in the Runner block.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_sopoffset_set(uint8_t bbh_id, uint8_t sopoffset);
bdmf_error_t ag_drv_bbh_rx_general_configuration_sopoffset_get(uint8_t bbh_id, uint8_t *sopoffset);

/**********************************************************************************************************************
 * crcomitdis: 
 *     Disable CRC omitting.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_crcomitdis_set(uint8_t bbh_id, bdmf_boolean crcomitdis);
bdmf_error_t ag_drv_bbh_rx_general_configuration_crcomitdis_get(uint8_t bbh_id, bdmf_boolean *crcomitdis);

/**********************************************************************************************************************
 * pkten: 
 *     When de-asserted, the BBH will not read new fragment/packet from the MAC.
 *     The BBH will Gracefully enable/disable (on fragment boundary for N/X/GPON/2 and on packet boundary for the rest)
 * sbpmen: 
 *     When de-asserted, the BBH will not pre-fetch SBPM buffers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_enable_set(uint8_t bbh_id, bdmf_boolean pkten, bdmf_boolean sbpmen);
bdmf_error_t ag_drv_bbh_rx_general_configuration_enable_get(uint8_t bbh_id, bdmf_boolean *pkten, bdmf_boolean *sbpmen);

/**********************************************************************************************************************
 * enable: 
 *     Enable G999.1
 * bytes4_7enable: 
 *     Enable G999.1 transfer of bytes 4-7 instead of bytes 0-3
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_g9991en_set(uint8_t bbh_id, bdmf_boolean enable, bdmf_boolean bytes4_7enable);
bdmf_error_t ag_drv_bbh_rx_general_configuration_g9991en_get(uint8_t bbh_id, bdmf_boolean *enable, bdmf_boolean *bytes4_7enable);

/**********************************************************************************************************************
 * flowth: 
 *     According to this threshold:
 *     Flows 32 - th will have set 0 configurations.
 *     Flows (th+1) - 255 will have set 1 configurations.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_perflowth_set(uint8_t bbh_id, uint8_t flowth);
bdmf_error_t ag_drv_bbh_rx_general_configuration_perflowth_get(uint8_t bbh_id, uint8_t *flowth);

/**********************************************************************************************************************
 * minpktsel: 
 *     Selects one of the 4 global configurations for minimum packet size.
 *     Bits {2n, 2n+1} refers to flow n.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_min_pkt_sel_flows_0_15_set(uint8_t bbh_id, uint8_t flow_id, uint8_t sel_id);
bdmf_error_t ag_drv_bbh_rx_min_pkt_sel_flows_0_15_get(uint8_t bbh_id, uint8_t flow_id, uint8_t *sel_id);

/**********************************************************************************************************************
 * minpktsel: 
 *     Selects one of the 4 global configurations for minimum packet size.
 *     Bits {2n, 2n+1} refers to flow n+16.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_min_pkt_sel_flows_16_31_set(uint8_t bbh_id, uint8_t flow_id, uint8_t sel_id);
bdmf_error_t ag_drv_bbh_rx_min_pkt_sel_flows_16_31_get(uint8_t bbh_id, uint8_t flow_id, uint8_t *sel_id);

/**********************************************************************************************************************
 * maxpktsel: 
 *     Selects one of the 4 global configurations for maximum packet size.
 *     Bits {2n, 2n+1} refers to flow n.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_max_pkt_sel_flows_0_15_set(uint8_t bbh_id, uint8_t flow_id, uint8_t sel_id);
bdmf_error_t ag_drv_bbh_rx_max_pkt_sel_flows_0_15_get(uint8_t bbh_id, uint8_t flow_id, uint8_t *sel_id);

/**********************************************************************************************************************
 * maxpktsel: 
 *     Selects one of the 4 global configurations for maximum packet size.
 *     Bits {2n, 2n+1} refers to flow n+16.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_max_pkt_sel_flows_16_31_set(uint8_t bbh_id, uint8_t flow_id, uint8_t sel_id);
bdmf_error_t ag_drv_bbh_rx_max_pkt_sel_flows_16_31_get(uint8_t bbh_id, uint8_t flow_id, uint8_t *sel_id);

/**********************************************************************************************************************
 * max_otf_sbpm_req: 
 *     Configure max on the fly requests to SBPM
 * pridropen: 
 *     This feature enables SBPM drop according to priority.
 *     2 bits according to SBPM congestion message: EXC low and EXC high
 *     1 configuration bit in BBH defines which of the SBPM indications are used (CNGSEL). 0 = SBPM EXC low; 1 = SBPM
 *     EXC high
 *     
 *     The following is added to the drop condition:
 *     SBPM_congestion = CNGSEL ? SBPM EXC high : SBPM EXC low
 *     drop if (packet priority != (GPON PLOAM or GPON EXC or EPON OAM) and SBPM_congestion)
 * cngsel: 
 *     Defines which of the SBPM indications are used. 0 = SBPM EXC low; 1 = SBPM EXC high
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_sbpmcfg_set(uint8_t bbh_id, uint8_t max_otf_sbpm_req, bdmf_boolean pridropen, bdmf_boolean cngsel);
bdmf_error_t ag_drv_bbh_rx_general_configuration_sbpmcfg_get(uint8_t bbh_id, uint8_t *max_otf_sbpm_req, bdmf_boolean *pridropen, bdmf_boolean *cngsel);

/**********************************************************************************************************************
 * inbufrst: 
 *     Writing 1 to this register will reset the input buffer.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * burstbufrst: 
 *     Writing 1 to this register will reset the Burst buffer.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * ingresscntxt: 
 *     Writing 1 to this register will reset the ingress context.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * cmdfiforst: 
 *     Writing 1 to this register will reset the IH buffer enable.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * sbpmfiforst: 
 *     Writing 1 to this register will reset the SBPM FIFO.
 *     The reset is done immediately. Reading this register will always return 0.
 * coherencyfiforst: 
 *     Writing 1 to this register will reset the coherency FIFO.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * cntxtrst: 
 *     Writing 1 to this register will reset the reassembly context table.
 *     The reset is done immediately. Reading this register will always return 0.
 * sdmarst: 
 *     Writing 1 to this register will reset the SDMA write pointer.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * dispnormal: 
 *     Writing 1 to this register will reset the dispatcher normal credits to a configurable value DISPCREDIT.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * dispexclusive: 
 *     Writing 1 to this register will reset the dispatcher exclusive credits to a configurable value DISPCREDIT.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * upldfiforst: 
 *     Writing 1 to this register will reset the UPLD FIFO.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * dispcredit: 
 *     The field is used for both DISPNORMAL and DISPEXCLUSIVE reset bit.
 *     Setting DISPNORMAL and/or DISPEXCLUSIVE will init the relevant credit counter according to this field.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_rxrstrst_set(uint8_t bbh_id, const bbh_rx_general_configuration_rxrstrst *general_configuration_rxrstrst);
bdmf_error_t ag_drv_bbh_rx_general_configuration_rxrstrst_get(uint8_t bbh_id, bbh_rx_general_configuration_rxrstrst *general_configuration_rxrstrst);

/**********************************************************************************************************************
 * rxdbgsel: 
 *     Selects one out of 10 possible debug vectors
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_rxdbgsel_set(uint8_t bbh_id, uint8_t rxdbgsel);
bdmf_error_t ag_drv_bbh_rx_general_configuration_rxdbgsel_get(uint8_t bbh_id, uint8_t *rxdbgsel);

/**********************************************************************************************************************
 * id_2overwr: 
 *     This field contains the users BB id for override
 * overwr_ra: 
 *     The new RA
 * overwr_en: 
 *     the overwr mechanism will be used only if this bit is active (1).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_bbhrx_raddr_decoder_set(uint8_t bbh_id, uint8_t id_2overwr, uint16_t overwr_ra, bdmf_boolean overwr_en);
bdmf_error_t ag_drv_bbh_rx_general_configuration_bbhrx_raddr_decoder_get(uint8_t bbh_id, uint8_t *id_2overwr, uint16_t *overwr_ra, bdmf_boolean *overwr_en);

/**********************************************************************************************************************
 * flowid: 
 *     Non Ethernet flow ID
 * enable: 
 *     When asserted, CRC errors will not be counted for that flow.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_noneth_set(uint8_t bbh_id, uint8_t flowid, bdmf_boolean enable);
bdmf_error_t ag_drv_bbh_rx_general_configuration_noneth_get(uint8_t bbh_id, uint8_t *flowid, bdmf_boolean *enable);

/**********************************************************************************************************************
 * bypass_clk_gate: 
 *     If set to 1b1 will disable the clock gate logic such to always enable the clock
 * timer_val: 
 *     For how long should the clock stay active once all conditions for clock disable are met.
 *     
 *     
 * keep_alive_en: 
 *     Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being
 *     removed completely will occur
 * keep_alive_intrvl: 
 *     If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active
 * keep_alive_cyc: 
 *     If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled
 *     (minus the KEEP_ALIVE_INTERVAL)
 *     
 *     So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(uint8_t bbh_id, const bbh_rx_general_configuration_clk_gate_cntrl *general_configuration_clk_gate_cntrl);
bdmf_error_t ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_get(uint8_t bbh_id, bbh_rx_general_configuration_clk_gate_cntrl *general_configuration_clk_gate_cntrl);

/**********************************************************************************************************************
 * runneraddr: 
 *     Defines the target address in the TM Runner, to which the PFC vector should be written.
 *     Address is in 8-bytes resolution.
 * pfcen: 
 *     Enables BBH RX PFC.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_pfccontrol_set(uint8_t bbh_id, uint16_t runneraddr, bdmf_boolean pfcen);
bdmf_error_t ag_drv_bbh_rx_general_configuration_pfccontrol_get(uint8_t bbh_id, uint16_t *runneraddr, bdmf_boolean *pfcen);

/**********************************************************************************************************************
 * disable: 
 *     Disables EPON type sequence error fix
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_eponseqdis_set(uint8_t bbh_id, bdmf_boolean disable);
bdmf_error_t ag_drv_bbh_rx_general_configuration_eponseqdis_get(uint8_t bbh_id, bdmf_boolean *disable);

/**********************************************************************************************************************
 * encry_type_err: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_encrypterror_get(uint8_t bbh_id, uint32_t *encry_type_err);

/**********************************************************************************************************************
 * inploam: 
 *     This counter counts the number of incoming PLOAMs.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_inploam_get(uint8_t bbh_id, uint32_t *inploam);

/**********************************************************************************************************************
 * pmvalue: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_epontyperror_get(uint8_t bbh_id, uint32_t *pmvalue);

/**********************************************************************************************************************
 * pmvalue: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_runterror_get(uint8_t bbh_id, uint16_t *pmvalue);

/**********************************************************************************************************************
 * pmvalue: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_flownotcounted_get(uint8_t bbh_id, uint16_t *pmvalue);

/**********************************************************************************************************************
 * inreass: 
 *     In reassembly.
 *     Not relevant for Ethernet.
 * flowid: 
 *     Flow ID
 * curoffset: 
 *     Current offset
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_cntxtx0lsb_get(uint8_t bbh_id, bdmf_boolean *inreass, uint8_t *flowid, uint16_t *curoffset);

/**********************************************************************************************************************
 * curbn: 
 *     Current BN
 * firstbn: 
 *     First BN
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_cntxtx0msb_get(uint8_t bbh_id, uint16_t *curbn, uint16_t *firstbn);

/**********************************************************************************************************************
 * inreass: 
 *     In reassembly.
 *     Not relevant for Ethernet.
 * flowid: 
 *     Flow ID
 * curoffset: 
 *     Current offset
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_cntxtx1lsb_get(uint8_t bbh_id, bdmf_boolean *inreass, uint8_t *flowid, uint16_t *curoffset);

/**********************************************************************************************************************
 * curbn: 
 *     Current BN
 * firstbn: 
 *     First BN
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_cntxtx1msb_get(uint8_t bbh_id, uint16_t *curbn, uint16_t *firstbn);

/**********************************************************************************************************************
 * inreass: 
 *     In reassembly.
 *     Not relevant for Ethernet.
 * sop: 
 *     SOP
 * priority: 
 *     Priority
 * flowid: 
 *     Flow ID
 * curoffset: 
 *     Current offset
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_cntxtx0ingress_get(uint8_t bbh_id, bbh_rx_debug_cntxtx0ingress *debug_cntxtx0ingress);

/**********************************************************************************************************************
 * inreass: 
 *     In reassembly.
 *     Not relevant for Ethernet.
 * sop: 
 *     SOP
 * priority: 
 *     Priority
 * flowid: 
 *     Flow ID
 * curoffset: 
 *     Current offset
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_cntxtx1ingress_get(uint8_t bbh_id, bbh_rx_debug_cntxtx1ingress *debug_cntxtx1ingress);

/**********************************************************************************************************************
 * uw: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_ibuw_get(uint8_t bbh_id, uint8_t *uw);

/**********************************************************************************************************************
 * uw: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_bbuw_get(uint8_t bbh_id, uint8_t *uw);

/**********************************************************************************************************************
 * uw: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_cfuw_get(uint8_t bbh_id, uint8_t *uw);

/**********************************************************************************************************************
 * sdma: 
 *     SDMA ACK counter
 * connect: 
 *     Connect ACK counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_ackcnt_get(uint8_t bbh_id, uint8_t *sdma, uint8_t *connect);

/**********************************************************************************************************************
 * normal: 
 *     Normal
 * exclusive: 
 *     Exclusive
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_coherencycnt_get(uint8_t bbh_id, uint8_t *normal, uint8_t *exclusive);

/**********************************************************************************************************************
 * dbgvec: 
 *     selected debug vector
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_dbgvec_get(uint8_t bbh_id, uint32_t *dbgvec);

/**********************************************************************************************************************
 * uw: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_ufuw_get(uint8_t bbh_id, uint8_t *uw);

/**********************************************************************************************************************
 * normal: 
 *     Normal
 * exclusive: 
 *     Exclusive
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_creditcnt_get(uint8_t bbh_id, uint8_t *normal, uint8_t *exclusive);

/**********************************************************************************************************************
 * ucd: 
 *     Used CDs
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_sdmacnt_get(uint8_t bbh_id, uint8_t *ucd);

/**********************************************************************************************************************
 * uw: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_cmfuw_get(uint8_t bbh_id, uint8_t *uw);

/**********************************************************************************************************************
 * bnentry: 
 *     BN
 * valid: 
 *     SBN is Valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_sbnfifo_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_sbnfifo *debug_sbnfifo);

/**********************************************************************************************************************
 * cmdentry: 
 *     CMD
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_cmdfifo_get(uint8_t bbh_id, uint32_t zero, bbh_rx_debug_cmdfifo *debug_cmdfifo);

/**********************************************************************************************************************
 * bnentry: 
 *     BN
 * valid: 
 *     SBN is Valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_sbnrecyclefifo_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_sbnrecyclefifo *debug_sbnrecyclefifo);

/**********************************************************************************************************************
 * cdsent: 
 *     CD sent
 * ackreceived: 
 *     EOP ACK received
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_coherencycnt2_get(uint8_t bbh_id, uint8_t *cdsent, uint8_t *ackreceived);

/**********************************************************************************************************************
 * dispstatus: 
 *     Dispatcher drop due to coherency FIFO full.
 *     Writing 1 to this bit clears it
 *     
 * sdmastatus: 
 *     SDMA drop due to coherency method 2 counters over 63 (dec).
 *     Writing 1 to this bit clears it
 * flowexceed: 
 *     Asserted when flow >= 130
 *     Writing 1 to this bit clears it
 *     
 * flowfull: 
 *     Asserted when flow drop pre-fifo in full.
 *     Writing 1 to this bit clears it
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_dropstatus_set(uint8_t bbh_id, bdmf_boolean dispstatus, bdmf_boolean sdmastatus, bdmf_boolean flowexceed, bdmf_boolean flowfull);
bdmf_error_t ag_drv_bbh_rx_debug_dropstatus_get(uint8_t bbh_id, bdmf_boolean *dispstatus, bdmf_boolean *sdmastatus, bdmf_boolean *flowexceed, bdmf_boolean *flowfull);

/**********************************************************************************************************************
 * init: 
 *     when asserted, the HW will start initializing the counters with value of 0. Should not be done during traffic.
 * initdone: 
 *     Asserted by the HW, this bit indicates the HW finished initializing the counters with value of 0.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_wan_flow_counters_gemctrinit_set(uint8_t bbh_id, bdmf_boolean init, bdmf_boolean initdone);
bdmf_error_t ag_drv_bbh_rx_wan_flow_counters_gemctrinit_get(uint8_t bbh_id, bdmf_boolean *init, bdmf_boolean *initdone);

/**********************************************************************************************************************
 * rdaddress: 
 *     the counter to be read
 * rd: 
 *     rd
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_wan_flow_counters_gemctrrd_set(uint8_t bbh_id, uint8_t rdaddress, bdmf_boolean rd);
bdmf_error_t ag_drv_bbh_rx_wan_flow_counters_gemctrrd_get(uint8_t bbh_id, uint8_t *rdaddress, bdmf_boolean *rd);

/**********************************************************************************************************************
 * rddata: 
 *     read data:
 *     pkts count 27:0, byte cnt 35:32
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_wan_flow_counters_gemctrrd0_get(uint8_t bbh_id, uint32_t *rddata);

/**********************************************************************************************************************
 * rddata: 
 *     read data:
 *     byte cnt 31:0
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_wan_flow_counters_gemctrrd1_get(uint8_t bbh_id, uint32_t *rddata);

#ifdef USE_BDMF_SHELL
enum
{
    cli_bbh_rx_ploam_en,
    cli_bbh_rx_user_priority3_en,
    cli_bbh_rx_pause_en,
    cli_bbh_rx_pfc_en,
    cli_bbh_rx_ctrl_en,
    cli_bbh_rx_pattern_en,
    cli_bbh_rx_exc_en,
    cli_bbh_rx_timer,
    cli_bbh_rx_dispatcher_drop_disable,
    cli_bbh_rx_sdma_drop_disable,
    cli_bbh_rx_sbpm_drop_disable,
    cli_bbh_rx_flow_control_force,
    cli_bbh_rx_flow_control_runner_enable,
    cli_bbh_rx_flow_control_qm_enable,
    cli_bbh_rx_pattern_recog,
    cli_bbh_rx_flow_ctrl_timer,
    cli_bbh_rx_flow_ctrl_force,
    cli_bbh_rx_flow_ctrl_rnr_en,
    cli_bbh_rx_flow_ctrl_drops_config,
    cli_bbh_rx_sdma_bb_id,
    cli_bbh_rx_dispatcher_sbpm_bb_id,
    cli_bbh_rx_dispatcher_virtual_queues,
    cli_bbh_rx_sdma_config,
    cli_bbh_rx_pkt_size0,
    cli_bbh_rx_pkt_size1,
    cli_bbh_rx_pkt_size2,
    cli_bbh_rx_pkt_size3,
    cli_bbh_rx_pkt_sel_group_0,
    cli_bbh_rx_pkt_sel_group_1,
    cli_bbh_rx_mac_flow,
    cli_bbh_rx_error_pm_counters,
    cli_bbh_rx_pm_counters,
    cli_bbh_rx_mac_mode,
    cli_bbh_rx_general_configuration_sopoffset,
    cli_bbh_rx_general_configuration_crcomitdis,
    cli_bbh_rx_general_configuration_enable,
    cli_bbh_rx_general_configuration_g9991en,
    cli_bbh_rx_general_configuration_perflowth,
    cli_bbh_rx_min_pkt_sel_flows_0_15,
    cli_bbh_rx_min_pkt_sel_flows_16_31,
    cli_bbh_rx_max_pkt_sel_flows_0_15,
    cli_bbh_rx_max_pkt_sel_flows_16_31,
    cli_bbh_rx_general_configuration_sbpmcfg,
    cli_bbh_rx_general_configuration_rxrstrst,
    cli_bbh_rx_general_configuration_rxdbgsel,
    cli_bbh_rx_general_configuration_bbhrx_raddr_decoder,
    cli_bbh_rx_general_configuration_noneth,
    cli_bbh_rx_general_configuration_clk_gate_cntrl,
    cli_bbh_rx_general_configuration_pfccontrol,
    cli_bbh_rx_general_configuration_eponseqdis,
    cli_bbh_rx_pm_counters_encrypterror,
    cli_bbh_rx_pm_counters_inploam,
    cli_bbh_rx_pm_counters_epontyperror,
    cli_bbh_rx_pm_counters_runterror,
    cli_bbh_rx_pm_counters_flownotcounted,
    cli_bbh_rx_debug_cntxtx0lsb,
    cli_bbh_rx_debug_cntxtx0msb,
    cli_bbh_rx_debug_cntxtx1lsb,
    cli_bbh_rx_debug_cntxtx1msb,
    cli_bbh_rx_debug_cntxtx0ingress,
    cli_bbh_rx_debug_cntxtx1ingress,
    cli_bbh_rx_debug_ibuw,
    cli_bbh_rx_debug_bbuw,
    cli_bbh_rx_debug_cfuw,
    cli_bbh_rx_debug_ackcnt,
    cli_bbh_rx_debug_coherencycnt,
    cli_bbh_rx_debug_dbgvec,
    cli_bbh_rx_debug_ufuw,
    cli_bbh_rx_debug_creditcnt,
    cli_bbh_rx_debug_sdmacnt,
    cli_bbh_rx_debug_cmfuw,
    cli_bbh_rx_debug_sbnfifo,
    cli_bbh_rx_debug_cmdfifo,
    cli_bbh_rx_debug_sbnrecyclefifo,
    cli_bbh_rx_debug_coherencycnt2,
    cli_bbh_rx_debug_dropstatus,
    cli_bbh_rx_wan_flow_counters_gemctrinit,
    cli_bbh_rx_wan_flow_counters_gemctrrd,
    cli_bbh_rx_wan_flow_counters_gemctrrd0,
    cli_bbh_rx_wan_flow_counters_gemctrrd1,
};

int bcm_bbh_rx_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_bbh_rx_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
