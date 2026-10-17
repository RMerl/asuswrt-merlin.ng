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


#ifndef _XRDP_DRV_CRDP_DS_CMN_BBH_TX_AG_H_
#define _XRDP_DRV_CRDP_DS_CMN_BBH_TX_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    bdmf_boolean freenocntxt;
    bdmf_boolean mc_type_3;
    bdmf_boolean mc_type_2;
    bdmf_boolean second_bn_dis;
    bdmf_boolean bufsize;
    bdmf_boolean enrrspread;
    uint8_t maxgn;
    uint16_t listfreetimer;
    uint8_t listfreethr;
    uint8_t smid;
} crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg;

typedef struct
{
    bdmf_boolean cntxtrst;
    bdmf_boolean pdfiforst;
    bdmf_boolean bpmfiforst;
    bdmf_boolean sbpmfiforst;
    bdmf_boolean ddrfiforst;
    bdmf_boolean sramfiforst;
    bdmf_boolean skbptrrst;
    bdmf_boolean gnxtfiforst;
    bdmf_boolean fbnfiforst;
    uint8_t port;
} crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd;

typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl;

typedef struct
{
    bdmf_boolean macdrop;
    bdmf_boolean discopycrdt;
    bdmf_boolean ecnrmrk;
    bdmf_boolean cntraddcrc;
    bdmf_boolean crcforward;
    bdmf_boolean multi_tcont;
    bdmf_boolean distargetperport;
    bdmf_boolean discrdtchk;
} crdp_ds_cmn_bbh_tx_common_configurations_general_cfg;

typedef struct
{
    uint8_t ecnbyteipv4;
    uint8_t ecnbitipv4;
    uint8_t ecnbyteipv6;
    uint8_t ecnbitipv6;
    uint8_t chksumbyteipv4;
    uint8_t assignedqcnt;
} crdp_ds_cmn_bbh_tx_common_configurations_ecncfg;

typedef struct
{
    uint8_t num_active_dram;
    bdmf_boolean en_ddr;
    bdmf_boolean en_epsr;
    bdmf_boolean simple_spread;
    uint8_t active_dram_0;
    uint8_t active_dram_1;
    bdmf_boolean block_spread_zone_en;
    uint8_t block_spread_zone;
    uint16_t spare;
} crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader;

typedef struct
{
    bdmf_boolean pdsel;
    bdmf_boolean pdvsel;
    bdmf_boolean pdemptysel;
    bdmf_boolean pdfullsel;
    bdmf_boolean pdbemptysel;
    bdmf_boolean pdffwkpsel;
    bdmf_boolean fbn_0_sel;
    bdmf_boolean fbnv_0_sel;
    bdmf_boolean fbnempty_0_sel;
    bdmf_boolean fbnfull_0_sel;
    bdmf_boolean getnext_0_sel;
    bdmf_boolean getnextv_0_sel;
    bdmf_boolean getnextempty_0_sel;
    bdmf_boolean getnextfull_0_sel;
    bdmf_boolean fpm_0_sel;
    bdmf_boolean sbpm_0_sel;
    bdmf_boolean fbnv_1_sel;
    bdmf_boolean fbnempty_1_sel;
    bdmf_boolean fbnfull_1_sel;
    bdmf_boolean getnext_1_sel;
    bdmf_boolean getnextv_1_sel;
    bdmf_boolean getnextempty_1_sel;
    bdmf_boolean getnextfull_1_sel;
    bdmf_boolean fpm_1_sel;
    bdmf_boolean sbpm_1_sel;
    bdmf_boolean fbn_1_sel;
} crdp_ds_cmn_bbh_tx_debug_counters_swrden;

typedef struct
{
    uint32_t debug_out_reg[8];
} crdp_ds_cmn_bbh_tx_debug_counters_dbgoutreg;


/**********************************************************************************************************************
 * type: 
 *     MAC type
 *     
 *     Note:
 *     xgpon - refers also to xgs, ngpon, hsp olt
 *     nni - refers also to nni and xfi
 *     xepon - refers also to 25g epon
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_mac_type_set(uint8_t bbh_id, uint8_t zero, uint8_t type);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_mac_type_get(uint8_t bbh_id, uint8_t zero, uint8_t *type);

/**********************************************************************************************************************
 * ddrbyte: 
 *     This counter counts the number of transmitted bytes from the DDr.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ddrbyte_get(uint8_t bbh_id, uint32_t *ddrbyte);

/**********************************************************************************************************************
 * srambyte: 
 *     This counter counts the number of transmitted bytes from the SRAM.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram0byte_get(uint8_t bbh_id, uint32_t *srambyte);

/**********************************************************************************************************************
 * srambyte: 
 *     This counter counts the number of transmitted bytes from the SRAM.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram1byte_get(uint8_t bbh_id, uint32_t *srambyte);

/**********************************************************************************************************************
 * sbpmsrc: 
 *     source id. This id is used to determine the route to the module.
 * fpmsrc: 
 *     source id. This id is used to determine the route to the module.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx_set(uint8_t bbh_id, uint16_t sbpmsrc, uint16_t fpmsrc);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx_get(uint8_t bbh_id, uint16_t *sbpmsrc, uint16_t *fpmsrc);

/**********************************************************************************************************************
 * freenocntxt: 
 *     When this bit is enabled, the BBH will use free without context command.
 * mc_type_3: 
 *     When this bit is enabled, when freeing a multicast packet, the bbh will use free type 3, meaning the sbpm
 *     should check the counter of both the first and the second buffers.
 * mc_type_2: 
 *     Same as above but with type 2, meaning the sbpm should check only the second buffer
 * second_bn_dis: 
 *     The bbh tx uses the second BN from the PD if the packet is 2 buffers long. This bit disables this optimization,
 *     so the BBH tx will always use get next command to understand what is the next BN
 * bufsize: 
 *     buffer size - relevant only for epsram
 * enrrspread: 
 *     enable rr spread option in the address spreader
 * maxgn: 
 *     maximum number of pending on the fly get next commands
 * listfreetimer: 
 *     timer value in 1024 clock cycles resolution for the case of free of list of buffers. If the list does not reach
 *     8 buffers for a timer time, then the free will occur anyway.
 * listfreethr: 
 *     threshold value for the case of free of list of buffers. When the number of bns in the list reaches thr value +
 *     1, the free command will be sent.
 * smid: 
 *     id of the sm in the sbpm for get next
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg *common_configurations_sbpmcfg);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg *common_configurations_sbpmcfg);

/**********************************************************************************************************************
 * bbsrcid: 
 *     bb_id
 * byp: 
 *     if set, the value will be taken from this register, else it will be taken from an input with the same name
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid_set(uint8_t bbh_id, uint16_t bbsrcid, bdmf_boolean byp);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid_get(uint8_t bbh_id, uint16_t *bbsrcid, bdmf_boolean *byp);

/**********************************************************************************************************************
 * bufsize0: 
 *     The data is arranged in the DDR in a fixed size FPM buffers.
 *     
 *     This register determines the FPM0 buffer size.
 *     For the FPMini (multicast headers), the size is fixed - 4 Kbytes.
 *     
 *     The
 * bufsize1: 
 *     The data is arranged in the DDR in a fixed size FPM1 buffers.
 *     
 *     This register determines the FPM buffer size.
 *     For the FPMini (multicast headers), the size is fixed - 4 Kbytes.
 *     
 *     The
 * hnsize: 
 *     The size of the HN (Header number) in bytes.
 *     In multicast, several headers are located in the same DDR buffer. the offset of header N within a buffer is
 *     (N-1)*64bytes, regardless of the HN actual size.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx_set(uint8_t bbh_id, uint8_t bufsize0, uint8_t bufsize1, uint8_t hnsize);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx_get(uint8_t bbh_id, uint8_t *bufsize0, uint8_t *bufsize1, uint8_t *hnsize);

/**********************************************************************************************************************
 * wkpaddr: 
 *     the address in which to write the wakeup info
 * task: 
 *     defines the tx task to wakeup
 *     
 * bbsrcid: 
 *     defines the bb source id of the TM core to wakeup
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_set(uint8_t bbh_id, uint8_t pd_rnr_cfg_index, uint16_t wkpaddr, uint8_t task, uint16_t bbsrcid);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_get(uint8_t bbh_id, uint8_t pd_rnr_cfg_index, uint16_t *wkpaddr, uint8_t *task, uint16_t *bbsrcid);

/**********************************************************************************************************************
 * cntxtrst: 
 *     Writing 1 to this register will reset the segmentation context table.
 *     The reset is done immediately. Reading this register will always return 0.
 *     TRIGGERS_HW
 *     
 * pdfiforst: 
 *     Writing 1 to this register will reset the PDs FIFOs.
 *     The reset is done immediately. Reading this register will always return 0.
 *     TRIGGERS_HW
 *     
 * bpmfiforst: 
 *     Writing 1 to this register will reset the BPM FIFO.
 *     The reset is done immediately. Reading this register will always return 0.
 *     TRIGGERS_HW
 *     
 * sbpmfiforst: 
 *     Writing 1 to this register will reset the SBPM FIFO.
 *     The reset is done immediately. Reading this register will always return 0.
 *     This register is relevalt only for Ethernet.
 *     TRIGGERS_HW
 *     
 * ddrfiforst: 
 *     Writing 1 to this register will reset the DDR data FIFO.
 *     The reset is done immediately. Reading this register will always return 0.
 *     This register is relevalt only for Ethernet.
 *     TRIGGERS_HW
 *     
 * sramfiforst: 
 *     Writing 1 to this register will reset the SRAM data FIFO.
 *     The reset is done immediately. Reading this register will always return 0.
 *     This register is relevalt only for Ethernet.
 *     TRIGGERS_HW
 *     
 * skbptrrst: 
 *     Writing 1 to this register will reset the SKB pointers.
 *     The reset is done immediately. Reading this register will always return 0.
 *     TRIGGERS_HW
 *     
 * gnxtfiforst: 
 *     Writing 1 to this register will reset the GET NEXT FIFOs
 *     The reset is done immediately. Reading this register will always return 0.
 *     TRIGGERS_HW
 *     
 * fbnfiforst: 
 *     Writing 1 to this register will reset the FIRST BN FIFOs
 *     The reset is done immediately. Reading this register will always return 0.
 *     TRIGGERS_HW
 *     
 * port: 
 *     relevant for resetting specific fifo, for example, PD FIFO of a certain port.
 *     
 *     PD FIFO
 *     GET NEXT FIFO
 *     FIRST_BN FIFO
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd *common_configurations_txrstcmd);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd *common_configurations_txrstcmd);

/**********************************************************************************************************************
 * dbgsel: 
 *     This register selects 1 of 8 debug vectors.
 *     The selected vector is reflected to DBGOUTREG.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel_set(uint8_t bbh_id, uint8_t dbgsel);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel_get(uint8_t bbh_id, uint8_t *dbgsel);

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
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl *common_configurations_clk_gate_cntrl);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl *common_configurations_clk_gate_cntrl);

/**********************************************************************************************************************
 * gpr: 
 *     general purpose register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_gpr_set(uint8_t bbh_id, uint32_t gpr);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_gpr_get(uint8_t bbh_id, uint32_t *gpr);

/**********************************************************************************************************************
 * macdrop: 
 *     when asserted, the BBH TX will drop the packet data of an invalid MAC flow
 * discopycrdt: 
 *     For the copy BBH TX, there is no need to check for credits in the FE buffer. When this bit is asserted, the
 *     segmentation SM will not check for credits, and the unified IF will not read from the data FIFO is the FE
 *     buffers are full.
 * ecnrmrk: 
 *     when asserted, the BBH TX will change the ecn bit according to the information in the PD + embedded header
 * cntraddcrc: 
 *     when asserted, the BBH TX will add 4 bytes per packet to all the byte counters, compensating the crc bytes
 * crcforward: 
 *     If asserted the BBH will mark the forward CRC (meaning not append or replace) towards the NNI
 * multi_tcont: 
 *     when asserted, the bbh will work in a multi tcont mode (affects segmentation SM and runner SM).
 * distargetperport: 
 *     the bbh manages credits per target memory per port. When this bit is asserted, the credits will be managed
 *     globally (per target mem, not per port). The credits configurations will be done in the configuration set of
 *     port 0.
 * discrdtchk: 
 *     when asserted the segmentation sm will not check for credits for preventing HOL before issuing request. It will
 *     only check for physical space in order to verify no overrun occurs.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_common_configurations_general_cfg *common_configurations_general_cfg);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_common_configurations_general_cfg *common_configurations_general_cfg);

/**********************************************************************************************************************
 * ecnbyteipv4: 
 *     the number of byte in which the ecn bits are located in ipv4 packets, relative to the ip header offset
 * ecnbitipv4: 
 *     the bit offset of the ECN bits within the byte. 0 means the ecn bits are [1:0], 6 means the ecn bits are [7:6]
 * ecnbyteipv6: 
 *     the number of byte in which the ecn bits are located in ipv6 packets, relative to the ip header offset
 * ecnbitipv6: 
 *     the bit offset of the ECN bits within the byte. 0 means the ecn bits are [1:0], 6 means the ecn bits are [7:6]
 * chksumbyteipv4: 
 *     the offset of the checksum field related to ip header offset in ipv4 packets. The offset must be multiplication
 *     of 2
 * assignedqcnt: 
 *     to which port to assign the one set of ecn marked packets and bytes counters
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_common_configurations_ecncfg *common_configurations_ecncfg);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_common_configurations_ecncfg *common_configurations_ecncfg);

/**********************************************************************************************************************
 * ddrfpminibase: 
 *     DDR FPMINI base.
 *     The address is in bytes resolution.
 *     The address should be aligned to 128 bytes.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_set(uint8_t bbh_id, uint8_t ddr_fpmini_basel_index, uint32_t ddrfpminibase);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_get(uint8_t bbh_id, uint8_t ddr_fpmini_basel_index, uint32_t *ddrfpminibase);

/**********************************************************************************************************************
 * ddrfpminibase: 
 *     DDR FPMINI base.
 *     The address is in bytes resolution.
 *     The address should be aligned to 128 bytes.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_set(uint8_t bbh_id, uint8_t ddr_fpmini_baseh_index, uint8_t ddrfpminibase);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_get(uint8_t bbh_id, uint8_t ddr_fpmini_baseh_index, uint8_t *ddrfpminibase);

/**********************************************************************************************************************
 * route: 
 *     route address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute_set(uint8_t bbh_id, uint32_t route);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute_get(uint8_t bbh_id, uint32_t *route);

/**********************************************************************************************************************
 * dest: 
 *     destination source id
 * en: 
 *     enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl_set(uint8_t bbh_id, uint16_t dest, bdmf_boolean en);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl_get(uint8_t bbh_id, uint16_t *dest, bdmf_boolean *en);

/**********************************************************************************************************************
 * ddrtmbase: 
 *     DDR TM base.
 *     The address is in bytes resolution.
 *     The address should be aligned to 128 bytes.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_set(uint8_t bbh_id, uint8_t tm_basel_index, uint32_t ddrtmbase);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_get(uint8_t bbh_id, uint8_t tm_basel_index, uint32_t *ddrtmbase);

/**********************************************************************************************************************
 * ddrtmbase: 
 *     MSB of DDR TM base.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_set(uint8_t bbh_id, uint8_t tm_baseh_index, uint8_t ddrtmbase);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_get(uint8_t bbh_id, uint8_t tm_baseh_index, uint8_t *ddrtmbase);

/**********************************************************************************************************************
 * size: 
 *     FIFO size in 32 bytes resolution
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_set(uint8_t bbh_id, uint8_t index, uint16_t size);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_get(uint8_t bbh_id, uint8_t index, uint16_t *size);

/**********************************************************************************************************************
 * credits: 
 *     credits
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_set(uint8_t bbh_id, uint8_t index, uint8_t credits);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_get(uint8_t bbh_id, uint8_t index, uint8_t *credits);

/**********************************************************************************************************************
 * init: 
 *     per esponder
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_creditinit_set(uint8_t bbh_id, uint32_t init);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_creditinit_get(uint8_t bbh_id, uint32_t *init);

/**********************************************************************************************************************
 * wkpaddr: 
 *     the address in which to write the wakeup info
 * task: 
 *     defines the tx task to wakeup
 *     
 * bbsrcid: 
 *     defines the bb source id of the TM core to wakeup
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_set(uint8_t bbh_id, uint8_t index, uint16_t wkpaddr, uint8_t task, uint16_t bbsrcid);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_get(uint8_t bbh_id, uint8_t index, uint16_t *wkpaddr, uint8_t *task, uint16_t *bbsrcid);

/**********************************************************************************************************************
 * wkpgran: 
 *     wakeup granularity - upon how many pd pops to wakeup the runner.
 *     One hot vector - possible options:
 *     1,2,4,8,16,32,64,128
 *     If configured to 0, there will not be a wakeup.
 * ptrrst: 
 *     resetting the pd wr pointer
 * enable: 
 *     enable mirroring.
 * timer: 
 *     timer value in 1024 cycles resolution. In cases in which the granularity is larger than 1, the bbh tx will send
 *     wakeup to the runner if the granularity is achieved or if the timer expired. The timer starts ticking when the
 *     bbh finished handling the first pd after wakeup.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_set(uint8_t bbh_id, uint8_t index, uint8_t wkpgran, bdmf_boolean ptrrst, bdmf_boolean enable, uint8_t timer);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_get(uint8_t bbh_id, uint8_t index, uint8_t *wkpgran, bdmf_boolean *ptrrst, bdmf_boolean *enable, uint8_t *timer);

/**********************************************************************************************************************
 * sninitval: 
 *     initial value of the serial number. The BBH increments it with the number of packets every time a PD arrives.
 *     When sending it to the PMA, the BBH will increment each packet one by one.
 * sninit: 
 *     init the serial number
 * lbctrrst: 
 *     option to reset the bytes counter per BBH. Total of 4 bytes counters (one per BBH). Used for load balancing
 * mode: 
 *     200 or 400G mode
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg_set(uint8_t bbh_id, uint16_t sninitval, bdmf_boolean sninit, uint8_t lbctrrst, uint8_t mode);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg_get(uint8_t bbh_id, uint16_t *sninitval, bdmf_boolean *sninit, uint8_t *lbctrrst, uint8_t *mode);

/**********************************************************************************************************************
 * bbsrcid: 
 *     bb source id of the other BBHs
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_set(uint8_t bbh_id, uint8_t index, uint16_t bbsrcid);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_get(uint8_t bbh_id, uint8_t index, uint16_t *bbsrcid);

/**********************************************************************************************************************
 * num_active_dram: 
 *     num_of_active_drams
 * en_ddr: 
 *     enable_ddr
 * en_epsr: 
 *     enable_epsram
 * simple_spread: 
 *     simple_spread
 * active_dram_0: 
 *     active_dram_0
 *     in case only 2 drams are active we get their selection
 * active_dram_1: 
 *     active_dram_1
 *     in case only 2 drams are active we get their selection
 * block_spread_zone_en: 
 *     block_spread_zone_en
 * block_spread_zone: 
 *     block_spread_zone
 * spare: 
 *     spare
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader *common_configurations_addr_spreader);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader *common_configurations_addr_spreader);

/**********************************************************************************************************************
 * numports: 
 *     the number of ports that share the pool of counters
 * numctrs: 
 *     the size of the share of counters each port is using from the general pool
 * macflowcnten: 
 *     enable counting according to mac flow field in the EH
 * ctridxcnten: 
 *     enable counting according to counter index field in the EH
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg_set(uint8_t bbh_id, bdmf_boolean numports, uint8_t numctrs, uint8_t macflowcnten, uint8_t ctridxcnten);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg_get(uint8_t bbh_id, bdmf_boolean *numports, uint8_t *numctrs, uint8_t *macflowcnten, uint8_t *ctridxcnten);

/**********************************************************************************************************************
 * wkpaddr: 
 *     the address in which to write the wakeup info
 * task: 
 *     defines the tx task to wakeup
 *     
 * bbsrcid: 
 *     defines the bb source id of the TM core to wakeup
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_set(uint8_t bbh_id, uint8_t index, uint16_t wkpaddr, uint8_t task, uint16_t bbsrcid);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_get(uint8_t bbh_id, uint8_t index, uint16_t *wkpaddr, uint8_t *task, uint16_t *bbsrcid);

/**********************************************************************************************************************
 * fc_stblfilter: 
 *     change stability filter - If there is a change in the flow control bits, for how long it should be stable for
 *     the BBH to send wake up the TM. This is used for the filtering frequent changes.
 *     Value is in clock cycles
 * fc_en: 
 *     when disabled, the BBH will not wakeup the TM
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fccfg_set(uint8_t bbh_id, uint16_t fc_stblfilter, bdmf_boolean fc_en);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fccfg_get(uint8_t bbh_id, uint16_t *fc_stblfilter, bdmf_boolean *fc_en);

/**********************************************************************************************************************
 * q: 
 *     vector of a bit per q
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack_set(uint8_t bbh_id, uint8_t q);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack_get(uint8_t bbh_id, uint8_t *q);

/**********************************************************************************************************************
 * fifosize: 
 *     PD fifo size.
 *     Note that value of X means X+1.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_set(uint8_t bbh_id, uint8_t index, uint16_t fifosize);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_get(uint8_t bbh_id, uint8_t index, uint16_t *fifosize);

/**********************************************************************************************************************
 * wkupthresh: 
 *     The wakeup threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_set(uint8_t bbh_id, uint8_t index, uint16_t wkupthresh);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_get(uint8_t bbh_id, uint8_t index, uint16_t *wkupthresh);

/**********************************************************************************************************************
 * empty: 
 *     EPON PD FIFO empty threshold.
 *     A queue which its PD FIFO occupancy is below this threshold will have high priority in PD ordering arbitration.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_set(uint8_t bbh_id, uint8_t index, uint16_t empty);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_get(uint8_t bbh_id, uint8_t index, uint16_t *empty);

/**********************************************************************************************************************
 * bytelimit: 
 *     Defines the number of bytes for PDs pre fetch limited according to the total number of bytes.
 *     The value is in 8-bytes resolution.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_set(uint8_t bbh_id, uint8_t index, uint32_t bytelimit);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_get(uint8_t bbh_id, uint8_t index, uint32_t *bytelimit);

/**********************************************************************************************************************
 * pdlimit: 
 *     Defines the max number of PDs to pre fetch in a single order from the TM runner
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_set(uint8_t bbh_id, uint8_t index, uint16_t pdlimit);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_get(uint8_t bbh_id, uint8_t index, uint16_t *pdlimit);

/**********************************************************************************************************************
 * pdbytelimiten: 
 *     This 8 bit vector (bit per port) enables the above feature (PDs pre fetch limited according to the total number
 *     of bytes).
 * pdmaxlimiten: 
 *     This bit vector (bit per port) enables the feature of PDs pre fetch limited according to max number of pds in a
 *     single batch.
 * disfesf: 
 *     Disable the store and forward mechanism in the FE buffer. When disable the BBH will transmit the data as soon
 *     as it gets to the FE buffer.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en_set(uint8_t bbh_id, uint8_t pdbytelimiten, uint8_t pdmaxlimiten, uint8_t disfesf);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en_get(uint8_t bbh_id, uint8_t *pdbytelimiten, uint8_t *pdmaxlimiten, uint8_t *disfesf);

/**********************************************************************************************************************
 * offset0: 
 *     offset0
 * offset1: 
 *     offset1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_set(uint8_t bbh_id, uint8_t index, uint16_t offset0, uint16_t offset1);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_get(uint8_t bbh_id, uint8_t index, uint16_t *offset0, uint16_t *offset1);

/**********************************************************************************************************************
 * thr: 
 *     threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_set(uint8_t bbh_id, uint8_t index, uint16_t thr);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_get(uint8_t bbh_id, uint8_t index, uint16_t *thr);

/**********************************************************************************************************************
 * fifobase0: 
 *     The base of FE FIFO in 32 bytes resolution
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_set(uint8_t bbh_id, uint8_t index, uint16_t fifobase0);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_get(uint8_t bbh_id, uint8_t index, uint16_t *fifobase0);

/**********************************************************************************************************************
 * fifosize: 
 *     The size of PD FIFO in 32 bytes resolution
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_set(uint8_t bbh_id, uint8_t index, uint16_t fifosize);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_get(uint8_t bbh_id, uint8_t index, uint16_t *fifosize);

/**********************************************************************************************************************
 * fifobase0: 
 *     The base of FE PD FIFO
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_set(uint8_t bbh_id, uint8_t index, uint16_t fifobase0);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_get(uint8_t bbh_id, uint8_t index, uint16_t *fifobase0);

/**********************************************************************************************************************
 * fifosize0: 
 *     The size of FE PD FIFO
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_set(uint8_t bbh_id, uint8_t index, uint16_t fifosize0);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_get(uint8_t bbh_id, uint8_t index, uint16_t *fifosize0);

/**********************************************************************************************************************
 * w0: 
 *     weight
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_set(uint8_t bbh_id, uint8_t index, uint8_t w0);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_get(uint8_t bbh_id, uint8_t index, uint8_t *w0);

/**********************************************************************************************************************
 * init: 
 *     initialization of the credits counter with the size of the FE FIFOS (pd and data)
 *     TRIGGERS_HW
 *     
 * min_pd: 
 *     minimum number of credits for the FE PD FIFO in order to perform an ordering of chuck of data from the SRAM/DDR
 *     TRIGGERS_HW
 *     
 * min_data: 
 *     minimum number of credits for the FE TX FIFO in order to perform an ordering of chuck of data from the SRAM/DDR
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits_set(uint8_t bbh_id, uint8_t init, uint16_t min_pd, uint16_t min_data);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits_get(uint8_t bbh_id, uint8_t *init, uint16_t *min_pd, uint16_t *min_data);

/**********************************************************************************************************************
 * init: 
 *     init per port, each have 2b:
 *     00: reset value, init done.
 *     01: reset the port memory to value 0, meaning disable all gems (functional mode).
 *     10: reset the port memory to value 1, meaning enable all gems(verification mode).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init_set(uint8_t bbh_id, uint16_t init);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init_get(uint8_t bbh_id, uint16_t *init);

/**********************************************************************************************************************
 * init: 
 *     2b init for table:
 *     00: reset value, init done.
 *     01: reset the port memory to value 0, meaning disable all gems (functional mode).
 *     10: reset the port memory to value 1, meaning enable all gems(verification mode).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init_set(uint8_t bbh_id, uint8_t init);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init_get(uint8_t bbh_id, uint8_t *init);

/**********************************************************************************************************************
 * ddr: 
 *     OTF commands for DDR responders
 * psram0: 
 *     OTF commands for PSRAM0 responders
 * psram1: 
 *     OTF commands for PSRAM1 responders
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_set(uint8_t bbh_id, uint8_t index, uint16_t ddr, uint16_t psram0, uint16_t psram1);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_get(uint8_t bbh_id, uint8_t index, uint16_t *ddr, uint16_t *psram0, uint16_t *psram1);

/**********************************************************************************************************************
 * ddr: 
 *     OTF commands for DDR responders
 * psram0: 
 *     OTF commands for PSRAM0 responders
 * psram1: 
 *     OTF commands for PSRAM1 responders
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_set(uint8_t bbh_id, uint8_t index, uint8_t ddr, uint8_t psram0, uint8_t psram1);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_get(uint8_t bbh_id, uint8_t index, uint8_t *ddr, uint8_t *psram0, uint8_t *psram1);

/**********************************************************************************************************************
 * numports: 
 *     number of active ports
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits_set(uint8_t bbh_id, uint8_t numports);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits_get(uint8_t bbh_id, uint8_t *numports);

/**********************************************************************************************************************
 * srampd: 
 *     This counter counts the number of packets which were transmitted from the SRAM.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txsram0pd_get(uint8_t bbh_id, uint32_t *srampd);

/**********************************************************************************************************************
 * srampd: 
 *     This counter counts the number of packets which were transmitted from the SRAM.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txsram1pd_get(uint8_t bbh_id, uint32_t *srampd);

/**********************************************************************************************************************
 * ddrpd: 
 *     This counter counts the number of packets which were transmitted from the DDR.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txddrpd_get(uint8_t bbh_id, uint32_t *ddrpd);

/**********************************************************************************************************************
 * pddrop: 
 *     This counter counts the number of PDs which were dropped due to PD FIFO full.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pddrop_get(uint8_t bbh_id, uint16_t *pddrop);

/**********************************************************************************************************************
 * getnextnull: 
 *     This counter counts the number Get next responses with a null BN.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_getnextnull_get(uint8_t bbh_id, uint16_t *getnextnull);

/**********************************************************************************************************************
 * getnextdrop: 
 *     This counter counts the number Get next responses that are dropped.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_getnextdrop_get(uint8_t bbh_id, uint16_t *getnextdrop);

/**********************************************************************************************************************
 * inputfifodrop: 
 *     This counter counts the number of PDs which were dropped due to input PD FIFO full.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_inputfifodrop_get(uint8_t bbh_id, uint16_t *inputfifodrop);

/**********************************************************************************************************************
 * srampkt: 
 *     This counter counts the number of packets which were transmitted from the SRAM.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram0pkt_get(uint8_t bbh_id, uint32_t *srampkt);

/**********************************************************************************************************************
 * srampkt: 
 *     This counter counts the number of packets which were transmitted from the SRAM.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram1pkt_get(uint8_t bbh_id, uint32_t *srampkt);

/**********************************************************************************************************************
 * ddrpkt: 
 *     This counter counts the number of packets which were transmitted from the DDR.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ddrpkt_get(uint8_t bbh_id, uint32_t *ddrpkt);

/**********************************************************************************************************************
 * pdsel: 
 *     rd from the PD FIFO
 *     TRIGGERS_HW
 *     
 * pdvsel: 
 *     rd from the PD valid array
 *     TRIGGERS_HW
 *     
 * pdemptysel: 
 *     rd from the PD empty array
 *     TRIGGERS_HW
 *     
 * pdfullsel: 
 *     rd from the PD Full array
 *     TRIGGERS_HW
 *     
 * pdbemptysel: 
 *     rd from the PD beliow empty array
 *     TRIGGERS_HW
 *     
 * pdffwkpsel: 
 *     rd from the PD full for wakeup empty array
 *     TRIGGERS_HW
 *     
 * fbn_0_sel: 
 *     rd from the first BN 0 array
 *     TRIGGERS_HW
 *     
 * fbnv_0_sel: 
 *     rd from the first BN 0 valid array
 *     TRIGGERS_HW
 *     
 * fbnempty_0_sel: 
 *     rd from the first BN 0 empty array
 *     TRIGGERS_HW
 *     
 * fbnfull_0_sel: 
 *     rd from the first BN 0 full array
 *     TRIGGERS_HW
 *     
 * getnext_0_sel: 
 *     rd from the first Get Next 0 array
 *     TRIGGERS_HW
 *     
 * getnextv_0_sel: 
 *     rd from the get_next valid 0 array
 *     TRIGGERS_HW
 *     
 * getnextempty_0_sel: 
 *     rd from the get next 0 empty array
 *     TRIGGERS_HW
 *     
 * getnextfull_0_sel: 
 *     rd from the get next 0 full array
 *     TRIGGERS_HW
 *     
 * fpm_0_sel: 
 *     rd from the FPM FIFO
 *     TRIGGERS_HW
 *     
 * sbpm_0_sel: 
 *     rd from the SBPM 0 FIFO
 *     TRIGGERS_HW
 *     
 * fbnv_1_sel: 
 *     rd from the first BN 1 valid array
 *     TRIGGERS_HW
 *     
 * fbnempty_1_sel: 
 *     rd from the first BN 1 empty array
 *     TRIGGERS_HW
 *     
 * fbnfull_1_sel: 
 *     rd from the first BN 1 full array
 *     TRIGGERS_HW
 *     
 * getnext_1_sel: 
 *     rd from the first Get Next 1 array
 *     TRIGGERS_HW
 *     
 * getnextv_1_sel: 
 *     rd from the get_next valid 1 array
 *     TRIGGERS_HW
 *     
 * getnextempty_1_sel: 
 *     rd from the get next 1 empty array
 *     TRIGGERS_HW
 *     
 * getnextfull_1_sel: 
 *     rd from the get next 1 full array
 *     TRIGGERS_HW
 *     
 * fpm_1_sel: 
 *     rd from the FPM 1 FIFO
 *     TRIGGERS_HW
 *     
 * sbpm_1_sel: 
 *     rd from the SBPM 1 FIFO
 *     TRIGGERS_HW
 *     
 * fbn_1_sel: 
 *     first bn 1 array
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrden_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_debug_counters_swrden *debug_counters_swrden);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrden_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_debug_counters_swrden *debug_counters_swrden);

/**********************************************************************************************************************
 * rdaddr: 
 *     The address inside the array the sw wants to read
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr_set(uint8_t bbh_id, uint16_t rdaddr);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr_get(uint8_t bbh_id, uint16_t *rdaddr);

/**********************************************************************************************************************
 * data: 
 *     data
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrddata_get(uint8_t bbh_id, uint32_t *data);

/**********************************************************************************************************************
 * unified_pkt: 
 *     This counter counts the number of packets transmitted per unified port.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unifiedpkt_get(uint8_t bbh_id, uint8_t debug_unified_pkt_ctr_idx, uint32_t *unified_pkt);

/**********************************************************************************************************************
 * ddrbyte: 
 *     This counter counts the number of transmitted bytes from the DDr.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unifiedbyte_get(uint8_t bbh_id, uint8_t debug_unified_byte_ctr_idx, uint32_t *ddrbyte);

/**********************************************************************************************************************
 * dbgvec: 
 *     Selected debug vector.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_dbgoutreg_get(uint8_t bbh_id, uint8_t zero, crdp_ds_cmn_bbh_tx_debug_counters_dbgoutreg *debug_counters_dbgoutreg);

/**********************************************************************************************************************
 * in_segmentation: 
 *     in_segmentation indication
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_in_segmentation_get(uint8_t bbh_id, uint32_t *in_segmentation);

/**********************************************************************************************************************
 * credits: 
 *     credits
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unified_data_credits_get(uint8_t bbh_id, uint8_t bbh_tx_debug_counters_unified_data_credits_byte_ctr_idx, uint16_t *credits);

/**********************************************************************************************************************
 * credits: 
 *     credits
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unified_pd_credits_get(uint8_t bbh_id, uint8_t bbh_tx_debug_counters_unified_pd_credits_byte_ctr_idx, uint16_t *credits);

/**********************************************************************************************************************
 * droppkt: 
 *     This counter counts the number of dropped pkts due to GEM disable
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_disflowdroppkt_get(uint8_t bbh_id, uint32_t *droppkt);

/**********************************************************************************************************************
 * dropbyte: 
 *     This counter counts the number of dropped bytes due to GEM disable
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_disflowdropbyte_get(uint8_t bbh_id, uint32_t *dropbyte);

/**********************************************************************************************************************
 * ecnpkt: 
 *     This counter counts the number of packets that were ecn remarked
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnpkt_get(uint8_t bbh_id, uint32_t *ecnpkt);

/**********************************************************************************************************************
 * ecnbytes: 
 *     This counter counts the number of bytes of ecn marked packets.
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnbytes_get(uint8_t bbh_id, uint32_t *ecnbytes);

/**********************************************************************************************************************
 * v1: 
 *     value1
 *     CHANGED_BY_HW
 *     
 * v2: 
 *     value2
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnmarkerr_get(uint8_t bbh_id, uint16_t *v1, uint16_t *v2);

/**********************************************************************************************************************
 * v1: 
 *     value1
 *     CHANGED_BY_HW
 *     
 * v2: 
 *     value2
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnlenerr_get(uint8_t bbh_id, uint16_t *v1, uint16_t *v2);

/**********************************************************************************************************************
 * sn: 
 *     bbh 200/400 serial number
 * pdfifoempty: 
 *     pd_fifo_empty - bit per port
 * pdarrayempty: 
 *     pd_array_empty - bit per port
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_flush_info_get(uint8_t bbh_id, uint16_t *sn, uint8_t *pdfifoempty, uint8_t *pdarrayempty);

/**********************************************************************************************************************
 * pfbingempty: 
 *     pfb ingress fifo empty - bit per port
 * fepdempty: 
 *     FE PD FIFO empty - bit per port
 * fedataempty: 
 *     FE data FIFO empty - bit per port
 * portvld: 
 *     indication that the port has credits and a pending pd
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_flush_info2_get(uint8_t bbh_id, uint8_t *pfbingempty, uint8_t *fepdempty, uint8_t *fedataempty, uint8_t *portvld);

/**********************************************************************************************************************
 * cnt: 
 *     cnt
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch_get(uint8_t bbh_id, uint16_t *cnt);

/**********************************************************************************************************************
 * port: 
 *     port
 * plen: 
 *     plen
 * pktnum: 
 *     number of aggregated packets
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch_data_get(uint8_t bbh_id, uint8_t *port, uint16_t *plen, uint8_t *pktnum);

/**********************************************************************************************************************
 * macflow: 
 *     mac flow field in eh is out of range
 * ctridx: 
 *     counter index field in eh is out of range
 * port: 
 *     the port in which the error occurred
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_outofrange_errors_get(uint8_t bbh_id, bdmf_boolean *macflow, bdmf_boolean *ctridx, uint8_t *port);

/**********************************************************************************************************************
 * pkt: 
 *     gem dropped pkt cnt
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gemdroppkt_get(uint8_t bbh_id, uint8_t index, uint32_t *pkt);

/**********************************************************************************************************************
 * bytes: 
 *     gem dropped bytes cnt
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gemdropbytes_get(uint8_t bbh_id, uint8_t index, uint32_t *bytes);

/**********************************************************************************************************************
 * value: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfddr_get(uint8_t bbh_id, uint8_t index, uint8_t *value);

/**********************************************************************************************************************
 * value: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfddr_get(uint8_t bbh_id, uint8_t index, uint16_t *value);

/**********************************************************************************************************************
 * value: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfepsram_get(uint8_t bbh_id, uint8_t index, uint8_t *value);

/**********************************************************************************************************************
 * value: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfepsram_get(uint8_t bbh_id, uint8_t index, uint16_t *value);

/**********************************************************************************************************************
 * entry: 
 *     entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gnext_get(uint8_t bbh_id, uint8_t index, uint32_t *entry);

/**********************************************************************************************************************
 * entry: 
 *     entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pdarray_get(uint8_t bbh_id, uint8_t index, uint32_t *entry);

/**********************************************************************************************************************
 * entry: 
 *     entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_curpd_get(uint8_t bbh_id, uint8_t index, uint32_t *entry);

/**********************************************************************************************************************
 * resps: 
 *     there are enough credits in the responders reorder fifos.
 * target_mem: 
 *     the on the fly commands and bytes counters per port per target memory did not cross the thresholds.
 * fe: 
 *     there are enough credits in the front_end pd and data buffers.
 * dp: 
 *     there is space in the per port pd,pc,reorder fifos in the BBH TX data path.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_space4req_get(uint8_t bbh_id, uint8_t *resps, uint8_t *target_mem, uint8_t *fe, uint8_t *dp);

/**********************************************************************************************************************
 * pdempty: 
 *     pd_array emtpy
 * gnrdy0: 
 *     get_next_ready0
 * gnrdy1: 
 *     get_next_ready1
 * qrdy: 
 *     q_is_ready
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_seginfo_get(uint8_t bbh_id, uint8_t *pdempty, uint8_t *gnrdy0, uint8_t *gnrdy1, uint8_t *qrdy);

/**********************************************************************************************************************
 * val: 
 *     valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ehdatval_get(uint8_t bbh_id, uint8_t *val);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ehdat_get(uint8_t bbh_id, uint8_t index, uint32_t *data);

/**********************************************************************************************************************
 * occ1: 
 *     occupancy1
 * occ2: 
 *     occupancy2
 * occ3: 
 *     occupancy3
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtpdocc_get(uint8_t bbh_id, uint16_t *occ1, uint16_t *occ2, uint16_t *occ3);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtbytes_get(uint8_t bbh_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtlbcnt_get(uint8_t bbh_id, uint8_t index, uint32_t *val);

/**********************************************************************************************************************
 * credit: 
 *     credit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respspd_get(uint8_t bbh_id, uint8_t index, uint8_t *credit);

/**********************************************************************************************************************
 * credit: 
 *     credit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respsdata_get(uint8_t bbh_id, uint8_t index, uint8_t *credit);

/**********************************************************************************************************************
 * value: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmpdcnt_get(uint8_t bbh_id, uint8_t index, uint32_t *value);

/**********************************************************************************************************************
 * value: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmbytecnt_get(uint8_t bbh_id, uint8_t index, uint32_t *value);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_portdbg_get(uint8_t bbh_id, uint8_t index, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respdbg_get(uint8_t bbh_id, uint8_t index, uint32_t *data);

/**********************************************************************************************************************
 * flow: 
 *     flow_index
 * gem: 
 *     gem port-id
 * valid: 
 *     valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid);

/**********************************************************************************************************************
 * flow: 
 *     flow_index
 * gem: 
 *     gem port-id
 * valid: 
 *     valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid);

/**********************************************************************************************************************
 * flow: 
 *     flow_index
 * gem: 
 *     gem port-id
 * valid: 
 *     valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid);

/**********************************************************************************************************************
 * flow: 
 *     flow_index
 * gem: 
 *     gem port-id
 * valid: 
 *     valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid);

/**********************************************************************************************************************
 * flow: 
 *     flow_index
 * gem: 
 *     gem port-id
 * valid: 
 *     valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid);

/**********************************************************************************************************************
 * flow: 
 *     flow_index
 * gem: 
 *     gem port-id
 * valid: 
 *     valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid);

/**********************************************************************************************************************
 * flow: 
 *     flow_index
 * gem: 
 *     gem port-id
 * valid: 
 *     valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid);

/**********************************************************************************************************************
 * flow: 
 *     flow_index
 * gem: 
 *     gem port-id
 * valid: 
 *     valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid);

/**********************************************************************************************************************
 * val: 
 *     val
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_set(uint8_t bbh_id, uint8_t index, uint32_t val);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_get(uint8_t bbh_id, uint8_t index, uint32_t *val);

/**********************************************************************************************************************
 * enable: 
 *     enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_set(uint8_t bbh_id, uint8_t index, uint32_t enable);
bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_get(uint8_t bbh_id, uint8_t index, uint32_t *enable);

#ifdef USE_BDMF_SHELL
enum
{
    cli_crdp_ds_cmn_bbh_tx_mac_type,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_ddrbyte,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_sram0byte,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_sram1byte,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_gpr,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_creditinit,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg,
    cli_crdp_ds_cmn_bbh_tx_common_configurations_fccfg,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_febase,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd,
    cli_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_txsram0pd,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_txsram1pd,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_txddrpd,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_pddrop,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_getnextnull,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_getnextdrop,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_inputfifodrop,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_sram0pkt,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_sram1pkt,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_ddrpkt,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_swrden,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_swrddata,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_unifiedpkt,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_unifiedbyte,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_dbgoutreg,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_in_segmentation,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_unified_data_credits,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_unified_pd_credits,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_disflowdroppkt,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_disflowdropbyte,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_ecnpkt,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_ecnbytes,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_ecnmarkerr,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_ecnlenerr,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_flush_info,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_flush_info2,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch_data,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_outofrange_errors,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_gemdroppkt,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_gemdropbytes,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfddr,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfddr,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfepsram,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfepsram,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_gnext,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_pdarray,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_curpd,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_space4req,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_seginfo,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_ehdatval,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_ehdat,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_rmtpdocc,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_rmtbytes,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_rmtlbcnt,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_respspd,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_respsdata,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmpdcnt,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmbytecnt,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_portdbg,
    cli_crdp_ds_cmn_bbh_tx_debug_counters_respdbg,
    cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0,
    cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1,
    cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2,
    cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3,
    cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4,
    cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5,
    cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6,
    cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7,
    cli_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val,
    cli_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en,
};

int bcm_crdp_ds_cmn_bbh_tx_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_crdp_ds_cmn_bbh_tx_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
