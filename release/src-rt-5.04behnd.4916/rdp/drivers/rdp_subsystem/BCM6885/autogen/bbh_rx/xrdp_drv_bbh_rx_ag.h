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
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} bbh_rx_general_configuration_clk_gate_cntrl;

typedef struct
{
    bdmf_boolean portrst;
    bdmf_boolean psramrst;
    bdmf_boolean pdfiforst;
    bdmf_boolean sbpmfiforst;
    bdmf_boolean dispcreditrst;
    bdmf_boolean upldfiforst;
    uint8_t pfbch;
    uint8_t dispcredit;
    uint16_t dispviq;
} bbh_rx_general_configuration_rxrstrst;

typedef struct
{
    uint32_t pattern_data_config[2];
} bbh_rx_general_configuration_patterndata;

typedef struct
{
    uint32_t pm_counters_inpkt[2];
} bbh_rx_pm_counters_inpkt;

typedef struct
{
    uint32_t pm_counters_inbyte[2];
} bbh_rx_pm_counters_inbyte;

typedef struct
{
    uint32_t pm_counters_too_short[2];
} bbh_rx_pm_counters_tooshort;

typedef struct
{
    uint32_t pm_counters_too_long[2];
} bbh_rx_pm_counters_toolong;

typedef struct
{
    uint32_t pm_counters_crc_error[2];
} bbh_rx_pm_counters_crcerror;

typedef struct
{
    uint32_t pm_counters_fc_drop[2];
} bbh_rx_pm_counters_fcdrop;

typedef struct
{
    uint32_t pm_counters_truncate[2];
} bbh_rx_pm_counters_truncate;

typedef struct
{
    uint32_t pm_counters_total_drop[2];
} bbh_rx_pm_counters_totaldrop;

typedef struct
{
    uint32_t debug_pfbuw[8];
} bbh_rx_debug_pfbuw;

typedef struct
{
    uint32_t debug_patternuw[8];
} bbh_rx_debug_patternuw;

typedef struct
{
    uint32_t debug_excuw[8];
} bbh_rx_debug_excuw;

typedef struct
{
    uint32_t debug_portuw[8];
} bbh_rx_debug_portuw;

typedef struct
{
    uint32_t debug_first_sbn[8];
} bbh_rx_debug_firstsbn;

typedef struct
{
    uint32_t debug_current_sbn[8];
} bbh_rx_debug_currentsbn;

typedef struct
{
    uint32_t debug_sn[8];
} bbh_rx_debug_sn;

typedef struct
{
    uint32_t debug_viq_mask[8];
} bbh_rx_debug_viqmask;

typedef struct
{
    uint32_t debug_plen[8];
} bbh_rx_debug_plen;

typedef struct
{
    uint32_t debug_dbg_veq[8];
} bbh_rx_debug_dbgvec;

typedef struct
{
    uint32_t debug_pduw[8];
} bbh_rx_debug_pduw;

typedef struct
{
    uint32_t debug_sbpm_ack[8];
} bbh_rx_debug_sbpmack;

typedef struct
{
    uint32_t debug_write_replay_ack[8];
} bbh_rx_debug_writereplyack;

typedef struct
{
    uint32_t debug_disp_credit_ack[8];
} bbh_rx_debug_dispcreditack;

typedef struct
{
    uint32_t debug_pdrd_data[8];
} bbh_rx_debug_pdrddata;

typedef struct
{
    uint32_t sbn_fifo[8];
} bbh_rx_debug_sbnfifo;

typedef struct
{
    uint32_t debug_exc_fifo[8];
} bbh_rx_debug_excfifo;

typedef struct
{
    uint32_t debug_gem_mode[8];
} bbh_rx_gem_mode_gemmode;


/**********************************************************************************************************************
 * timer: 
 *     Timer value before de-asserting the flow control indication.
 *     The duration of the time is determined according to the BBH clock frequency.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_timer_set(uint8_t bbh_id, uint8_t index, uint16_t timer);
bdmf_error_t ag_drv_bbh_rx_timer_get(uint8_t bbh_id, uint8_t index, uint16_t *timer);

/**********************************************************************************************************************
 * fcforce: 
 *     Asserting this bit will force a flow control indication towards the MAC
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_flow_control_force_set(uint8_t bbh_id, uint8_t index, bdmf_boolean fcforce);
bdmf_error_t ag_drv_bbh_rx_flow_control_force_get(uint8_t bbh_id, uint8_t index, bdmf_boolean *fcforce);

/**********************************************************************************************************************
 * fcrnren: 
 *     Enables Runner to send flow control messages
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_flow_control_runner_enable_set(uint8_t bbh_id, uint8_t index, bdmf_boolean fcrnren);
bdmf_error_t ag_drv_bbh_rx_flow_control_runner_enable_get(uint8_t bbh_id, uint8_t index, bdmf_boolean *fcrnren);

/**********************************************************************************************************************
 * fcqmen: 
 *     Enables QM to send flow control messages
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_flow_control_qm_enable_set(uint8_t bbh_id, uint8_t index, bdmf_boolean fcqmen);
bdmf_error_t ag_drv_bbh_rx_flow_control_qm_enable_get(uint8_t bbh_id, uint8_t index, bdmf_boolean *fcqmen);

/**********************************************************************************************************************
 * timer: 
 *     Timer value before de-asserting the flow control indication.
 *     The duration of the time is determined according to the BBH clock frequency.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_timer_set(uint8_t bbh_id, uint8_t index, uint16_t timer);
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_timer_get(uint8_t bbh_id, uint8_t index, uint16_t *timer);

/**********************************************************************************************************************
 * fcforce: 
 *     Asserting this bit will force a flow control indication towards the MAC
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_force_set(uint8_t bbh_id, uint8_t index, bdmf_boolean fcforce);
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_force_get(uint8_t bbh_id, uint8_t index, bdmf_boolean *fcforce);

/**********************************************************************************************************************
 * fcrnren: 
 *     Enables Runner to send flow control messages
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_rnr_en_set(uint8_t bbh_id, uint8_t index, bdmf_boolean fcrnren);
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_rnr_en_get(uint8_t bbh_id, uint8_t index, bdmf_boolean *fcrnren);

/**********************************************************************************************************************
 * dispbbid: 
 *     Dispatcher BB ID. This ID defines the BB ID of the Dispatcher that the BBH communicates with.
 * sbpmbbid: 
 *     SBPM BB ID. This ID defines the BB ID of the SBPM that the BBH communicates with.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_bbcfg_set(uint8_t bbh_id, uint16_t dispbbid, uint16_t sbpmbbid);
bdmf_error_t ag_drv_bbh_rx_general_configuration_bbcfg_get(uint8_t bbh_id, uint16_t *dispbbid, uint16_t *sbpmbbid);

/**********************************************************************************************************************
 * pkten: 
 *     When de-asserted, the BBH will read and discard new fragment/packet from the MAC.
 *     The BBH will Gracefully enable/disable (on fragment boundary for N/X/GPON/2 and on packet boundary for the
 *     rest).
 *     Bit n refers to PFB channel n.
 *     TRIGGERS_HW
 *     
 * sbpmen: 
 *     When de-asserted, the BBH will not pre-fetch SBPM buffers
 *     TRIGGERS_HW
 *     
 * geminit: 
 *     Initialize the GEM mode memory to all zero.
 *     SW may check the done bit (separated register) to verify that the init operation is complete.
 * grace_dis: 
 *     When Packet_enable (PKTEN) is de-asserted (disable BBH RX), according to GRACE_DIS:
 *     0: complete receiving the current packet and drop all the next ones.
 *     1: Truncate the correct packet with error and drop all the next ones.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_enable_set(uint8_t bbh_id, uint8_t pkten, bdmf_boolean sbpmen, bdmf_boolean geminit, bdmf_boolean grace_dis);
bdmf_error_t ag_drv_bbh_rx_general_configuration_enable_get(uint8_t bbh_id, uint8_t *pkten, bdmf_boolean *sbpmen, bdmf_boolean *geminit, bdmf_boolean *grace_dis);

/**********************************************************************************************************************
 * minpkt_normal: 
 *     Normal packets shorter than this threshold will be discarded.
 * minpkt_omci: 
 *     OMCI packets shorter than this threshold will be discarded.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_minpkt_set(uint8_t bbh_id, uint8_t minpkt_normal, uint8_t minpkt_omci);
bdmf_error_t ag_drv_bbh_rx_general_configuration_minpkt_get(uint8_t bbh_id, uint8_t *minpkt_normal, uint8_t *minpkt_omci);

/**********************************************************************************************************************
 * maxpkt: 
 *     Packets longer than this threshold will be discarded.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_maxpkt_set(uint8_t bbh_id, uint16_t maxpkt);
bdmf_error_t ag_drv_bbh_rx_general_configuration_maxpkt_get(uint8_t bbh_id, uint16_t *maxpkt);

/**********************************************************************************************************************
 * bypass_clk_gate: 
 *     If set to 1b1 will disable the clock gate logic such to always enable the clock
 *     TRIGGERS_HW
 *     
 * timer_val: 
 *     For how long should the clock stay active once all conditions for clock disable are met.
 *     TRIGGERS_HW
 *     
 * keep_alive_en: 
 *     Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being
 *     removed completely will occur
 *     TRIGGERS_HW
 *     
 * keep_alive_intrvl: 
 *     If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active
 *     TRIGGERS_HW
 *     
 * keep_alive_cyc: 
 *     If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled
 *     (minus the KEEP_ALIVE_INTERVAL)
 *     
 *     So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(uint8_t bbh_id, const bbh_rx_general_configuration_clk_gate_cntrl *general_configuration_clk_gate_cntrl);
bdmf_error_t ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_get(uint8_t bbh_id, bbh_rx_general_configuration_clk_gate_cntrl *general_configuration_clk_gate_cntrl);

/**********************************************************************************************************************
 * rxdbgsel: 
 *     Selects one out of the debug vectors
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_rxdbgsel_set(uint8_t bbh_id, uint8_t rxdbgsel);
bdmf_error_t ag_drv_bbh_rx_general_configuration_rxdbgsel_get(uint8_t bbh_id, uint8_t *rxdbgsel);

/**********************************************************************************************************************
 * portrst: 
 *     Writing 1 to this register will reset the port module according to PFBCH.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * psramrst: 
 *     Writing 1 to this register will reset the psram module.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * pdfiforst: 
 *     Writing 1 to this register will reset the PD FIFO module.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * sbpmfiforst: 
 *     Writing 1 to this register will reset the SBPM FIFO.
 *     The reset is done immediately. Reading this register will always return 0.
 * dispcreditrst: 
 *     Writing 1 to this register will reset the dispatcher credits of a configured VIQ (DISPVIQ) to a configurable
 *     value DISPCREDIT.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * upldfiforst: 
 *     Writing 1 to this register will reset the UPLD FIFO.
 *     For a reset operation the SW should assert and then de-assert this bit.
 * pfbch: 
 *     Setting PORTRST will reset the relevant module according to this field.
 * dispcredit: 
 *     Setting DISPCREDITRST will init the relevant credit counter according to this field.
 * dispviq: 
 *     Setting DISPCREDITRST will init the relevant credit counter according to this field.
 *     This is a one hot field, which selects one out of 9 VIQ, e.g. a value of 0x4 will init VIQ2.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_rxrstrst_set(uint8_t bbh_id, const bbh_rx_general_configuration_rxrstrst *general_configuration_rxrstrst);
bdmf_error_t ag_drv_bbh_rx_general_configuration_rxrstrst_get(uint8_t bbh_id, bbh_rx_general_configuration_rxrstrst *general_configuration_rxrstrst);

/**********************************************************************************************************************
 * max_otf_sbpm_req: 
 *     Configure max on the fly requests to SBPM
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_sbpmcfg_set(uint8_t bbh_id, uint8_t max_otf_sbpm_req);
bdmf_error_t ag_drv_bbh_rx_general_configuration_sbpmcfg_get(uint8_t bbh_id, uint8_t *max_otf_sbpm_req);

/**********************************************************************************************************************
 * psrambase: 
 *     PSRAM UBUS base address.
 *     The address is in 1MByte resolution.
 *     Real address: {PSRAMBASE, 20h0}
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_psrambase_set(uint8_t bbh_id, uint32_t psrambase);
bdmf_error_t ag_drv_bbh_rx_general_configuration_psrambase_get(uint8_t bbh_id, uint32_t *psrambase);

/**********************************************************************************************************************
 * ingresschannel: 
 *     BBH RX ingress channel
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_ingresschannel_set(uint8_t bbh_id, uint8_t ingresschannel);
bdmf_error_t ag_drv_bbh_rx_general_configuration_ingresschannel_get(uint8_t bbh_id, uint8_t *ingresschannel);

/**********************************************************************************************************************
 * route: 
 *     route address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_bbovrdroute_set(uint8_t bbh_id, uint32_t route);
bdmf_error_t ag_drv_bbh_rx_general_configuration_bbovrdroute_get(uint8_t bbh_id, uint32_t *route);

/**********************************************************************************************************************
 * dest: 
 *     destination source id
 * en: 
 *     enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_bbovrdctrl_set(uint8_t bbh_id, uint16_t dest, bdmf_boolean en);
bdmf_error_t ag_drv_bbh_rx_general_configuration_bbovrdctrl_get(uint8_t bbh_id, uint16_t *dest, bdmf_boolean *en);

/**********************************************************************************************************************
 * gemdone: 
 *     GEM mode init done
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_gemdone_get(uint8_t bbh_id, bdmf_boolean *gemdone);

/**********************************************************************************************************************
 * mactype: 
 *     Configuration of the MAC type:
 *     4d0   : OLT 1G EPON
 *     4d1   : OLT 10G EPON
 *     4d2   : OLT 25G EPON / ONU 25G EPON
 *     4d3   : OLT Optical AE
 *     4d4   : OLT GPON
 *     4d5   : OLT XGPON/XGS
 *     4d6   : OLT HSP
 *     4d7   : NNI
 *     4d8   : Reserved
 *     4d9   : Reserved
 *     4d10  : ONU HSP
 *     4d11  : Reserved
 *     4d12  : Reserved
 *     4d13  : Reserved
 *     4d14  : Reserved
 *     4d15  : Reserved
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_mactype_set(uint8_t bbh_id, uint8_t index, uint8_t mactype);
bdmf_error_t ag_drv_bbh_rx_general_configuration_mactype_get(uint8_t bbh_id, uint8_t index, uint8_t *mactype);

/**********************************************************************************************************************
 * portthr: 
 *     Threshold configuration in 32bytes resolution.
 *     The FIFO size is 1KBytes - 32 x 256b.
 *     Max value is 0x20.
 *     0x3f should be used to disable the effect of this threshold.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_portth_set(uint8_t bbh_id, uint8_t index, uint8_t portthr);
bdmf_error_t ag_drv_bbh_rx_general_configuration_portth_get(uint8_t bbh_id, uint8_t index, uint8_t *portthr);

/**********************************************************************************************************************
 * sopoffset: 
 *     The SOP offset in bytes.
 *     Allowed values: 0-127.
 *     This value should match the relevant configuration in the Runner block.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_sopoffset_set(uint8_t bbh_id, uint8_t index, uint8_t sopoffset);
bdmf_error_t ag_drv_bbh_rx_general_configuration_sopoffset_get(uint8_t bbh_id, uint8_t index, uint8_t *sopoffset);

/**********************************************************************************************************************
 * ingresspid: 
 *     Ingress PID configuration (will be sent in the PD)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_ingresspid_set(uint8_t bbh_id, uint8_t channel_num, uint8_t ingresspid);
bdmf_error_t ag_drv_bbh_rx_general_configuration_ingresspid_get(uint8_t bbh_id, uint8_t channel_num, uint8_t *ingresspid);

/**********************************************************************************************************************
 * portweight: 
 *     Ports weight for WRR arbitration
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_portweight_set(uint8_t bbh_id, uint8_t index, uint8_t portweight);
bdmf_error_t ag_drv_bbh_rx_general_configuration_portweight_get(uint8_t bbh_id, uint8_t index, uint8_t *portweight);

/**********************************************************************************************************************
 * portviqmask: 
 *     Ports VIQ mask - one hot vector
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_portviqmask_set(uint8_t bbh_id, uint8_t viq_index, uint16_t portviqmask);
bdmf_error_t ag_drv_bbh_rx_general_configuration_portviqmask_get(uint8_t bbh_id, uint8_t viq_index, uint16_t *portviqmask);

/**********************************************************************************************************************
 * excviqmask: 
 *     Exclusive VIQ mask - one hot vector
 * excviqen: 
 *     Exclusive VIQ enable.
 *     Each port may enable/disable this VIQ.
 * excviqoamen: 
 *     Exclusive VIQ enable due to OAM indication.
 *     Each port may enable/disable this VIQ.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_excviqmask_set(uint8_t bbh_id, uint8_t viq_index, uint16_t excviqmask, bdmf_boolean excviqen, bdmf_boolean excviqoamen);
bdmf_error_t ag_drv_bbh_rx_general_configuration_excviqmask_get(uint8_t bbh_id, uint8_t viq_index, uint16_t *excviqmask, bdmf_boolean *excviqen, bdmf_boolean *excviqoamen);

/**********************************************************************************************************************
 * viq: 
 *     Defines the Dispatchers VIQs for PFB channels 0-7
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_dispviq_set(uint8_t bbh_id, uint8_t viq_index, uint8_t viq);
bdmf_error_t ag_drv_bbh_rx_general_configuration_dispviq_get(uint8_t bbh_id, uint8_t viq_index, uint8_t *viq);

/**********************************************************************************************************************
 * viqweight: 
 *     VIQ weight for WRR arbitration
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_viqweight_set(uint8_t bbh_id, uint8_t index, uint8_t viqweight);
bdmf_error_t ag_drv_bbh_rx_general_configuration_viqweight_get(uint8_t bbh_id, uint8_t index, uint8_t *viqweight);

/**********************************************************************************************************************
 * location: 
 *     The pattern is compared against the 32 first bytes of each packet. This field defines the start offset for
 *     matching (1-byte resolution) within these 32 bytes.
 *     Location max value is: 32 - (pattern size in bytes)
 * no_drop_en: 
 *     1 means this pattern match is used for not dropping high priority (if drop is enabled).
 *     0 means this pattern is not used for not dropping high priority (if drop is enabled).
 * viqclassen: 
 *     1 means this pattern match is used for VIQ classification.
 *     0 means this pattern is not used for VIQ classification.
 * viq: 
 *     9 bits (one hot - out of 9 VIQs) " relevant only if NO_DROP = 0
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_patterncfg_set(uint8_t bbh_id, uint8_t viq_index, uint8_t location, bdmf_boolean no_drop_en, bdmf_boolean viqclassen, uint16_t viq);
bdmf_error_t ag_drv_bbh_rx_general_configuration_patterncfg_get(uint8_t bbh_id, uint8_t viq_index, uint8_t *location, bdmf_boolean *no_drop_en, bdmf_boolean *viqclassen, uint16_t *viq);

/**********************************************************************************************************************
 * patterndata: 
 *     Pattern Data (reg index 0 refers to 32 LSBits)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_patterndata_set(uint8_t bbh_id, uint8_t zero, const bbh_rx_general_configuration_patterndata *general_configuration_patterndata);
bdmf_error_t ag_drv_bbh_rx_general_configuration_patterndata_get(uint8_t bbh_id, uint8_t zero, bbh_rx_general_configuration_patterndata *general_configuration_patterndata);

/**********************************************************************************************************************
 * patternmask: 
 *     Pattern mask. Reg index 0 refers to 32 LSBits
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_patternmask_set(uint8_t bbh_id, uint8_t index, uint32_t patternmask);
bdmf_error_t ag_drv_bbh_rx_general_configuration_patternmask_get(uint8_t bbh_id, uint8_t index, uint32_t *patternmask);

/**********************************************************************************************************************
 * patterndata: 
 *     Pattern Data (reg index 0 refers to 32 LSBits)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_patterndata8b_set(uint8_t bbh_id, uint8_t viq_index, uint32_t patterndata);
bdmf_error_t ag_drv_bbh_rx_general_configuration_patterndata8b_get(uint8_t bbh_id, uint8_t viq_index, uint32_t *patterndata);

/**********************************************************************************************************************
 * patternmask: 
 *     Pattern mask. Reg index 0 refers to 32 LSBits
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_patternmask8b_set(uint8_t bbh_id, uint8_t viq_index, uint32_t patternmask);
bdmf_error_t ag_drv_bbh_rx_general_configuration_patternmask8b_get(uint8_t bbh_id, uint8_t viq_index, uint32_t *patternmask);

/**********************************************************************************************************************
 * patternen: 
 *     Each of the global 8 patterns can be enables per port according to this vector (bit per pattern match).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_patternen_set(uint8_t bbh_id, uint8_t viq_index, uint8_t patternen);
bdmf_error_t ag_drv_bbh_rx_general_configuration_patternen_get(uint8_t bbh_id, uint8_t viq_index, uint8_t *patternen);

/**********************************************************************************************************************
 * viqfcen: 
 *     VIQ flow control enable vector
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_viqfcen_set(uint8_t bbh_id, uint8_t viq_index, uint16_t viqfcen);
bdmf_error_t ag_drv_bbh_rx_general_configuration_viqfcen_get(uint8_t bbh_id, uint8_t viq_index, uint16_t *viqfcen);

/**********************************************************************************************************************
 * flowid: 
 *     Non Ethernet flow ID
 * enable: 
 *     When asserted, CRC errors will not be counted for that flow.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_general_configuration_noneth_set(uint8_t bbh_id, uint8_t viq_index, uint16_t flowid, bdmf_boolean enable);
bdmf_error_t ag_drv_bbh_rx_general_configuration_noneth_get(uint8_t bbh_id, uint8_t viq_index, uint16_t *flowid, bdmf_boolean *enable);

/**********************************************************************************************************************
 * inpkt: 
 *     This counter counts the number of incoming good packets.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_inpkt_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_inpkt *pm_counters_inpkt);

/**********************************************************************************************************************
 * inbyte: 
 *     This counter counts the number of incoming good bytes.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_inbyte_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_inbyte *pm_counters_inbyte);

/**********************************************************************************************************************
 * too_short: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_tooshort_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_tooshort *pm_counters_tooshort);

/**********************************************************************************************************************
 * too_long: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_toolong_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_toolong *pm_counters_toolong);

/**********************************************************************************************************************
 * crc_err: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_crcerror_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_crcerror *pm_counters_crcerror);

/**********************************************************************************************************************
 * fc_drop: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_fcdrop_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_fcdrop *pm_counters_fcdrop);

/**********************************************************************************************************************
 * truncate: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_truncate_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_truncate *pm_counters_truncate);

/**********************************************************************************************************************
 * total_drop: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_totaldrop_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_totaldrop *pm_counters_totaldrop);

/**********************************************************************************************************************
 * pmvalue: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pm_counters_dummypdcnt_get(uint8_t bbh_id, uint32_t *pmvalue);

/**********************************************************************************************************************
 * r64: 
 *     This counter counts the number of R64 packets.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r64_get(uint8_t bbh_id, uint32_t *r64);

/**********************************************************************************************************************
 * r127: 
 *     This counter counts the number of R127 packets.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r127_get(uint8_t bbh_id, uint32_t *r127);

/**********************************************************************************************************************
 * r255: 
 *     This counter counts the number of R255 packets.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r255_get(uint8_t bbh_id, uint32_t *r255);

/**********************************************************************************************************************
 * r511: 
 *     This counter counts the number of R511 packets.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r511_get(uint8_t bbh_id, uint32_t *r511);

/**********************************************************************************************************************
 * r1023: 
 *     This counter counts the number of R1023 packets.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r1023_get(uint8_t bbh_id, uint32_t *r1023);

/**********************************************************************************************************************
 * r1518: 
 *     This counter counts the number of R1518 packets.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r1518_get(uint8_t bbh_id, uint32_t *r1518);

/**********************************************************************************************************************
 * r1518: 
 *     This counter counts the number of R1518 packets.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r2047_get(uint8_t bbh_id, uint32_t *r1518);

/**********************************************************************************************************************
 * r4095: 
 *     This counter counts the number of R4095 packets.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r4095_get(uint8_t bbh_id, uint32_t *r4095);

/**********************************************************************************************************************
 * r9216: 
 *     This counter counts the number of R9216 packets.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r9216_get(uint8_t bbh_id, uint32_t *r9216);

/**********************************************************************************************************************
 * r16383: 
 *     This counter counts the number of R18383 packets.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r16383_get(uint8_t bbh_id, uint32_t *r16383);

/**********************************************************************************************************************
 * uw: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_pfbuw_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_pfbuw *debug_pfbuw);

/**********************************************************************************************************************
 * uw: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_patternuw_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_patternuw *debug_patternuw);

/**********************************************************************************************************************
 * uw: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_excuw_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_excuw *debug_excuw);

/**********************************************************************************************************************
 * uw: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_portuw_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_portuw *debug_portuw);

/**********************************************************************************************************************
 * context: 
 *     Context value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_firstsbn_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_firstsbn *debug_firstsbn);

/**********************************************************************************************************************
 * context: 
 *     Context value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_currentsbn_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_currentsbn *debug_currentsbn);

/**********************************************************************************************************************
 * context: 
 *     Context value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_sn_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_sn *debug_sn);

/**********************************************************************************************************************
 * context: 
 *     Context value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_viqmask_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_viqmask *debug_viqmask);

/**********************************************************************************************************************
 * context: 
 *     Context value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_plen_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_plen *debug_plen);

/**********************************************************************************************************************
 * context: 
 *     Context value - bit per port.
 *     bit 0 for Port 0
 *     ...
 *     Bit 7 for port 7
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_eopok_get(uint8_t bbh_id, uint8_t *context);

/**********************************************************************************************************************
 * context: 
 *     Context value - bit per port.
 *     bit 0 for Port 0
 *     ...
 *     Bit 7 for port 7
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_viqfull_get(uint8_t bbh_id, uint8_t *context);

/**********************************************************************************************************************
 * context: 
 *     Context value - bit per port.
 *     bit 0 for Port 0
 *     ...
 *     Bit 7 for port 7
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_portreq_get(uint8_t bbh_id, uint8_t *context);

/**********************************************************************************************************************
 * context: 
 *     Context value - bit per port.
 *     bit 0 for Port 0
 *     ...
 *     Bit 7 for port 7
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_dummyreq_get(uint8_t bbh_id, uint8_t *context);

/**********************************************************************************************************************
 * context: 
 *     Context value - bit per port.
 *     bit 0 for Port 0
 *     ...
 *     Bit 7 for port 7
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_inreassembly_get(uint8_t bbh_id, uint8_t *context);

/**********************************************************************************************************************
 * context: 
 *     Context value - bit per port.
 *     bit 0 for Port 0
 *     ...
 *     Bit 7 for port 7
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_dummypd_get(uint8_t bbh_id, uint8_t *context);

/**********************************************************************************************************************
 * uw: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_ufuw_get(uint8_t bbh_id, uint8_t *uw);

/**********************************************************************************************************************
 * uw: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_pendconnectuw_get(uint8_t bbh_id, uint8_t *uw);

/**********************************************************************************************************************
 * dbgvec: 
 *     DEBUG vector.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_dbgvec_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_dbgvec *debug_dbgvec);

/**********************************************************************************************************************
 * uw: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_pduw_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_pduw *debug_pduw);

/**********************************************************************************************************************
 * ackcnt: 
 *     ACK counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_sbpmack_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_sbpmack *debug_sbpmack);

/**********************************************************************************************************************
 * ackcnt: 
 *     ACK counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_writereplyack_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_writereplyack *debug_writereplyack);

/**********************************************************************************************************************
 * ackcnt: 
 *     ACK counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_dispcreditack_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_dispcreditack *debug_dispcreditack);

/**********************************************************************************************************************
 * rddata: 
 *     Read Data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_pdrddata_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_pdrddata *debug_pdrddata);

/**********************************************************************************************************************
 * oddbnentry: 
 *     BN
 * oddvalid: 
 *     SBN is Valid
 * evenbnentry: 
 *     BN
 * evenvalid: 
 *     SBN is Valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_sbnfifo_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_sbnfifo *debug_sbnfifo);

/**********************************************************************************************************************
 * oddbnentry: 
 *     BN
 * oddvalid: 
 *     SBN is Valid
 * evenbnentry: 
 *     BN
 * evenvalid: 
 *     SBN is Valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_sbnprefetch_get(uint8_t bbh_id, uint16_t *oddbnentry, bdmf_boolean *oddvalid, uint16_t *evenbnentry, bdmf_boolean *evenvalid);

/**********************************************************************************************************************
 * bnentry: 
 *     BN
 * valid: 
 *     SBN is Valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_debug_excfifo_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_excfifo *debug_excfifo);

/**********************************************************************************************************************
 * pmvalue: 
 *     PM counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range_get(uint8_t bbh_id, uint32_t *pmvalue);

/**********************************************************************************************************************
 * flow: 
 *     Out of range flow.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow_get(uint8_t bbh_id, uint16_t *flow);

/**********************************************************************************************************************
 * usedwords: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_usedwords_get(uint8_t bbh_id, uint8_t *usedwords);

/**********************************************************************************************************************
 * gemmode: 
 *     GEM mode entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_rx_gem_mode_gemmode_set(uint8_t bbh_id, uint8_t zero, const bbh_rx_gem_mode_gemmode *gem_mode_gemmode);
bdmf_error_t ag_drv_bbh_rx_gem_mode_gemmode_get(uint8_t bbh_id, uint8_t zero, bbh_rx_gem_mode_gemmode *gem_mode_gemmode);

#ifdef USE_BDMF_SHELL
enum
{
    cli_bbh_rx_timer,
    cli_bbh_rx_flow_control_force,
    cli_bbh_rx_flow_control_runner_enable,
    cli_bbh_rx_flow_control_qm_enable,
    cli_bbh_rx_flow_ctrl_timer,
    cli_bbh_rx_flow_ctrl_force,
    cli_bbh_rx_flow_ctrl_rnr_en,
    cli_bbh_rx_general_configuration_bbcfg,
    cli_bbh_rx_general_configuration_enable,
    cli_bbh_rx_general_configuration_minpkt,
    cli_bbh_rx_general_configuration_maxpkt,
    cli_bbh_rx_general_configuration_clk_gate_cntrl,
    cli_bbh_rx_general_configuration_rxdbgsel,
    cli_bbh_rx_general_configuration_rxrstrst,
    cli_bbh_rx_general_configuration_sbpmcfg,
    cli_bbh_rx_general_configuration_psrambase,
    cli_bbh_rx_general_configuration_ingresschannel,
    cli_bbh_rx_general_configuration_bbovrdroute,
    cli_bbh_rx_general_configuration_bbovrdctrl,
    cli_bbh_rx_general_configuration_gemdone,
    cli_bbh_rx_general_configuration_mactype,
    cli_bbh_rx_general_configuration_portth,
    cli_bbh_rx_general_configuration_sopoffset,
    cli_bbh_rx_general_configuration_ingresspid,
    cli_bbh_rx_general_configuration_portweight,
    cli_bbh_rx_general_configuration_portviqmask,
    cli_bbh_rx_general_configuration_excviqmask,
    cli_bbh_rx_general_configuration_dispviq,
    cli_bbh_rx_general_configuration_viqweight,
    cli_bbh_rx_general_configuration_patterncfg,
    cli_bbh_rx_general_configuration_patterndata,
    cli_bbh_rx_general_configuration_patternmask,
    cli_bbh_rx_general_configuration_patterndata8b,
    cli_bbh_rx_general_configuration_patternmask8b,
    cli_bbh_rx_general_configuration_patternen,
    cli_bbh_rx_general_configuration_viqfcen,
    cli_bbh_rx_general_configuration_noneth,
    cli_bbh_rx_pm_counters_inpkt,
    cli_bbh_rx_pm_counters_inbyte,
    cli_bbh_rx_pm_counters_tooshort,
    cli_bbh_rx_pm_counters_toolong,
    cli_bbh_rx_pm_counters_crcerror,
    cli_bbh_rx_pm_counters_fcdrop,
    cli_bbh_rx_pm_counters_truncate,
    cli_bbh_rx_pm_counters_totaldrop,
    cli_bbh_rx_pm_counters_dummypdcnt,
    cli_bbh_rx_pon_rmon_counters_r64,
    cli_bbh_rx_pon_rmon_counters_r127,
    cli_bbh_rx_pon_rmon_counters_r255,
    cli_bbh_rx_pon_rmon_counters_r511,
    cli_bbh_rx_pon_rmon_counters_r1023,
    cli_bbh_rx_pon_rmon_counters_r1518,
    cli_bbh_rx_pon_rmon_counters_r2047,
    cli_bbh_rx_pon_rmon_counters_r4095,
    cli_bbh_rx_pon_rmon_counters_r9216,
    cli_bbh_rx_pon_rmon_counters_r16383,
    cli_bbh_rx_debug_pfbuw,
    cli_bbh_rx_debug_patternuw,
    cli_bbh_rx_debug_excuw,
    cli_bbh_rx_debug_portuw,
    cli_bbh_rx_debug_firstsbn,
    cli_bbh_rx_debug_currentsbn,
    cli_bbh_rx_debug_sn,
    cli_bbh_rx_debug_viqmask,
    cli_bbh_rx_debug_plen,
    cli_bbh_rx_debug_eopok,
    cli_bbh_rx_debug_viqfull,
    cli_bbh_rx_debug_portreq,
    cli_bbh_rx_debug_dummyreq,
    cli_bbh_rx_debug_inreassembly,
    cli_bbh_rx_debug_dummypd,
    cli_bbh_rx_debug_ufuw,
    cli_bbh_rx_debug_pendconnectuw,
    cli_bbh_rx_debug_dbgvec,
    cli_bbh_rx_debug_pduw,
    cli_bbh_rx_debug_sbpmack,
    cli_bbh_rx_debug_writereplyack,
    cli_bbh_rx_debug_dispcreditack,
    cli_bbh_rx_debug_pdrddata,
    cli_bbh_rx_debug_sbnfifo,
    cli_bbh_rx_debug_sbnprefetch,
    cli_bbh_rx_debug_excfifo,
    cli_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range,
    cli_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow,
    cli_bbh_rx_bbh_rx_pon_cnpl_debug_usedwords,
    cli_bbh_rx_gem_mode_gemmode,
};

int bcm_bbh_rx_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_bbh_rx_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
