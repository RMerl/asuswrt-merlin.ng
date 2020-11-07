/*
   Copyright (c) 2015 Broadcom Corporation
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

#ifndef _DRV_EPON_LIF_AG_H_
#define _DRV_EPON_LIF_AG_H_

#include "access_macros.h"
#if !defined(_CFE_)
#include "bdmf_interface.h"
#else
#include "bdmf_data_types.h"
#include "bdmf_errno.h"
#endif
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
typedef struct
{
    bdmf_boolean cfgdisruntfilter;
    uint8_t cfmaxcommaerrcnt;
    uint8_t cfsyncsmselect;
    bdmf_boolean cfponrxforcenonfecabort;
    bdmf_boolean cfponrxforcefecabort;
    bdmf_boolean cfgrxdatabitflip;
    bdmf_boolean cfenablesoftwaresynchold;
    bdmf_boolean cfenableextendsync;
    bdmf_boolean cfenablequicksync;
    bdmf_boolean cfppsen;
    bdmf_boolean cfppsclkrbc;
    bdmf_boolean cfrx2txlpback;
    bdmf_boolean cftx2rxlpback;
    bdmf_boolean cftxdataendurlon;
    bdmf_boolean cfp2pmode;
    bdmf_boolean cfp2pshortpre;
    bdmf_boolean cflaseren;
    bdmf_boolean cftxlaseron;
    bdmf_boolean cftxlaseronacthi;
    bdmf_boolean liftxrstn_pre;
    bdmf_boolean lifrxrstn_pre;
    bdmf_boolean liftxen;
    bdmf_boolean lifrxen;
} lif_pon_control;

typedef struct
{
    uint8_t cfipgfilter;
    bdmf_boolean cfdisableloslaserblock;
    bdmf_boolean cfgllidpromiscuousmode;
    bdmf_boolean cfgllidmodmsk;
    bdmf_boolean cfusefecipg;
    bdmf_boolean cfrxcrc8invchk;
    bdmf_boolean cfrxcrc8bitswap;
    bdmf_boolean cfrxcrc8msb2lsb;
    bdmf_boolean cfrxcrc8disable;
    bdmf_boolean cftxllidbit15set;
    bdmf_boolean cftxcrc8inv;
    bdmf_boolean cftxcrc8bad;
    bdmf_boolean cftxcrc8bitswap;
    bdmf_boolean cftxcrc8msb2lsb;
    bdmf_boolean cftxshortpre;
    uint8_t cftxipgcnt;
    uint8_t cftxaasynclen;
    uint8_t cftxpipedelay;
} lif_pon_inter_op_control;

typedef struct
{
    bdmf_boolean cffecrxerrorprop;
    bdmf_boolean cffecrxforcenonfecabort;
    bdmf_boolean cffecrxforcefecabort;
    bdmf_boolean cffecrxenable;
    bdmf_boolean cffectxfecperllid;
    bdmf_boolean cffectxenable;
} lif_fec_control;

typedef struct
{
    bdmf_boolean cfgdismpcpencrypt;
    bdmf_boolean cfgdisoamencrypt;
    bdmf_boolean cfgsecenshortlen;
    bdmf_boolean cfgsecdnenpktnumrlovr;
    bdmf_boolean cfgsecupenpktnumrlovr;
    bdmf_boolean cfgenaereplayprct;
    bdmf_boolean cfgenlegacyrcc;
    bdmf_boolean enfakeupaes;
    bdmf_boolean enfakednaes;
    uint8_t cfgfecipglen;
    bdmf_boolean disdndasaencrpt;
    bdmf_boolean entriplechurn;
    bdmf_boolean enepnmixencrypt;
    bdmf_boolean disupdasaencrpt;
    uint8_t secupencryptscheme;
    uint8_t secdnencryptscheme;
    bdmf_boolean secuprstn_pre;
    bdmf_boolean secdnrstn_pre;
    bdmf_boolean secenup;
    bdmf_boolean secendn;
} lif_sec_control;

typedef struct
{
    bdmf_boolean int_sop_sfec_ipg_violation;
    bdmf_boolean laseronmax;
    bdmf_boolean laseroff;
    bdmf_boolean secdnreplayprotctabort;
    bdmf_boolean secuppktnumoverflow;
    bdmf_boolean intlaseroffdurburst;
    bdmf_boolean intrxberthreshexc;
    bdmf_boolean intfecrxfecrecvstatus;
    bdmf_boolean intfecrxcorerrfifofullstatus;
    bdmf_boolean intfecrxcorerrfifounexpempty;
    bdmf_boolean intfecbufpopemptypush;
    bdmf_boolean intfecbufpopemptynopush;
    bdmf_boolean intfecbufpushfull;
    bdmf_boolean intuptimefullupdstat;
    bdmf_boolean intfroutofalignstat;
    bdmf_boolean intgrntstarttimelagstat;
    bdmf_boolean intabortrxfrmstat;
    bdmf_boolean intnorxclkstat;
    bdmf_boolean intrxmaxlenerrstat;
    bdmf_boolean intrxerraftalignstat;
    bdmf_boolean intrxsynchacqstat;
    bdmf_boolean intrxoutofsynchstat;
} lif_int_status;

typedef struct
{
    bdmf_boolean int_sop_sfec_ipg_violation_mask;
    bdmf_boolean laseronmaxmask;
    bdmf_boolean laseroffmask;
    bdmf_boolean secdnreplayprotctabortmsk;
    bdmf_boolean secuppktnumoverflowmsk;
    bdmf_boolean intlaseroffdurburstmask;
    bdmf_boolean intrxberthreshexcmask;
    bdmf_boolean intfecrxfecrecvmask;
    bdmf_boolean intfecrxcorerrfifofullmask;
    bdmf_boolean intfecrxcorerrfifounexpemptymask;
    bdmf_boolean intfecbufpopemptypushmask;
    bdmf_boolean intfecbufpopemptynopushmask;
    bdmf_boolean intfecbufpushfullmask;
    bdmf_boolean intuptimefullupdmask;
    bdmf_boolean intfroutofalignmask;
    bdmf_boolean intgrntstarttimelagmask;
    bdmf_boolean intabortrxfrmmask;
    bdmf_boolean intnorxclkmask;
    bdmf_boolean intrxmaxlenerrmask;
    bdmf_boolean intrxerraftalignmask;
    bdmf_boolean intrxsynchacqmask;
    bdmf_boolean intrxoutofsynchmask;
} lif_int_mask;

typedef struct
{
    bdmf_boolean data_port_busy;
    bdmf_boolean data_port_error;
    uint8_t ram_select;
    uint8_t data_port_op_code;
    uint16_t data_port_addr;
} lif_data_port_command;

bdmf_error_t ag_drv_lif_pon_control_set(const lif_pon_control *pon_control);
bdmf_error_t ag_drv_lif_pon_control_get(lif_pon_control *pon_control);
bdmf_error_t ag_drv_lif_pon_inter_op_control_set(const lif_pon_inter_op_control *pon_inter_op_control);
bdmf_error_t ag_drv_lif_pon_inter_op_control_get(lif_pon_inter_op_control *pon_inter_op_control);
bdmf_error_t ag_drv_lif_fec_control_set(const lif_fec_control *fec_control);
bdmf_error_t ag_drv_lif_fec_control_get(lif_fec_control *fec_control);
bdmf_error_t ag_drv_lif_sec_control_set(const lif_sec_control *sec_control);
bdmf_error_t ag_drv_lif_sec_control_get(lif_sec_control *sec_control);
bdmf_error_t ag_drv_lif_macsec_set(uint16_t cfgmacsecethertype);
bdmf_error_t ag_drv_lif_macsec_get(uint16_t *cfgmacsecethertype);
bdmf_error_t ag_drv_lif_int_status_set(const lif_int_status *int_status);
bdmf_error_t ag_drv_lif_int_status_get(lif_int_status *int_status);
bdmf_error_t ag_drv_lif_int_mask_set(const lif_int_mask *int_mask);
bdmf_error_t ag_drv_lif_int_mask_get(lif_int_mask *int_mask);
bdmf_error_t ag_drv_lif_data_port_command_set(const lif_data_port_command *data_port_command);
bdmf_error_t ag_drv_lif_data_port_command_get(lif_data_port_command *data_port_command);
bdmf_error_t ag_drv_lif_data_port_data_set(uint8_t portidx, uint32_t pbiportdata);
bdmf_error_t ag_drv_lif_data_port_data_get(uint8_t portidx, uint32_t *pbiportdata);
bdmf_error_t ag_drv_lif_llid_set(uint8_t llid_index, uint32_t cfgllid0);
bdmf_error_t ag_drv_lif_llid_get(uint8_t llid_index, uint32_t *cfgllid0);
bdmf_error_t ag_drv_lif_time_ref_cnt_set(uint8_t cffullupdatevalue, uint8_t cfmaxnegvalue, uint8_t cfmaxposvalue);
bdmf_error_t ag_drv_lif_time_ref_cnt_get(uint8_t *cffullupdatevalue, uint8_t *cfmaxnegvalue, uint8_t *cfmaxposvalue);
bdmf_error_t ag_drv_lif_timestamp_upd_per_set(uint16_t cftimestampupdper);
bdmf_error_t ag_drv_lif_timestamp_upd_per_get(uint16_t *cftimestampupdper);
bdmf_error_t ag_drv_lif_tp_time_set(uint32_t cftransporttime);
bdmf_error_t ag_drv_lif_tp_time_get(uint32_t *cftransporttime);
bdmf_error_t ag_drv_lif_mpcp_time_get(uint32_t *ltmpcptime);
bdmf_error_t ag_drv_lif_maxlen_ctr_set(uint16_t cfrxmaxframelength);
bdmf_error_t ag_drv_lif_maxlen_ctr_get(uint16_t *cfrxmaxframelength);
bdmf_error_t ag_drv_lif_laser_on_delta_set(uint16_t cftxlaserondelta);
bdmf_error_t ag_drv_lif_laser_on_delta_get(uint16_t *cftxlaserondelta);
bdmf_error_t ag_drv_lif_laser_off_idle_set(uint16_t cftxinitidle, uint8_t cftxlaseroffdelta);
bdmf_error_t ag_drv_lif_laser_off_idle_get(uint16_t *cftxinitidle, uint8_t *cftxlaseroffdelta);
bdmf_error_t ag_drv_lif_fec_init_idle_set(uint16_t cftxfecinitidle);
bdmf_error_t ag_drv_lif_fec_init_idle_get(uint16_t *cftxfecinitidle);
bdmf_error_t ag_drv_lif_fec_err_allow_set(uint8_t cfrxtfecbiterrallow, uint8_t cfrxsfecbiterrallow);
bdmf_error_t ag_drv_lif_fec_err_allow_get(uint8_t *cfrxtfecbiterrallow, uint8_t *cfrxsfecbiterrallow);
bdmf_error_t ag_drv_lif_sec_key_sel_get(uint8_t link_idx, bdmf_boolean *data);
bdmf_error_t ag_drv_lif_dn_encrypt_stat_set(uint8_t link_idx, bdmf_boolean enEncrypt);
bdmf_error_t ag_drv_lif_dn_encrypt_stat_get(uint8_t link_idx, bdmf_boolean *enEncrypt);
bdmf_error_t ag_drv_lif_sec_up_key_stat_get(uint8_t link_idx, bdmf_boolean *keyUpSel);
bdmf_error_t ag_drv_lif_sec_up_encrypt_stat_get(uint8_t link_idx, bdmf_boolean *enUpEncrypt);
bdmf_error_t ag_drv_lif_sec_up_mpcp_offset_set(uint32_t secupmpcpoffset);
bdmf_error_t ag_drv_lif_sec_up_mpcp_offset_get(uint32_t *secupmpcpoffset);
bdmf_error_t ag_drv_lif_fec_per_llid_set(uint8_t link_idx, bdmf_boolean cfFecTxFecEnLlid);
bdmf_error_t ag_drv_lif_fec_per_llid_get(uint8_t link_idx, bdmf_boolean *cfFecTxFecEnLlid);
bdmf_error_t ag_drv_lif_rx_line_code_err_cnt_set(uint32_t rxlinecodeerrcnt);
bdmf_error_t ag_drv_lif_rx_line_code_err_cnt_get(uint32_t *rxlinecodeerrcnt);
bdmf_error_t ag_drv_lif_rx_agg_mpcp_frm_set(uint32_t rxaggmpcpcnt);
bdmf_error_t ag_drv_lif_rx_agg_mpcp_frm_get(uint32_t *rxaggmpcpcnt);
bdmf_error_t ag_drv_lif_rx_agg_good_frm_set(uint32_t rxagggoodcnt);
bdmf_error_t ag_drv_lif_rx_agg_good_frm_get(uint32_t *rxagggoodcnt);
bdmf_error_t ag_drv_lif_rx_agg_good_byte_set(uint32_t rxagggoodbytescnt);
bdmf_error_t ag_drv_lif_rx_agg_good_byte_get(uint32_t *rxagggoodbytescnt);
bdmf_error_t ag_drv_lif_rx_agg_undersz_frm_set(uint32_t rxaggunderszcnt);
bdmf_error_t ag_drv_lif_rx_agg_undersz_frm_get(uint32_t *rxaggunderszcnt);
bdmf_error_t ag_drv_lif_rx_agg_oversz_frm_set(uint32_t rxaggoverszcnt);
bdmf_error_t ag_drv_lif_rx_agg_oversz_frm_get(uint32_t *rxaggoverszcnt);
bdmf_error_t ag_drv_lif_rx_agg_crc8_frm_set(uint32_t rxaggcrc8errcnt);
bdmf_error_t ag_drv_lif_rx_agg_crc8_frm_get(uint32_t *rxaggcrc8errcnt);
bdmf_error_t ag_drv_lif_rx_agg_fec_frm_set(uint32_t rxaggfec);
bdmf_error_t ag_drv_lif_rx_agg_fec_frm_get(uint32_t *rxaggfec);
bdmf_error_t ag_drv_lif_rx_agg_fec_byte_set(uint32_t rxaggfecbytes);
bdmf_error_t ag_drv_lif_rx_agg_fec_byte_get(uint32_t *rxaggfecbytes);
bdmf_error_t ag_drv_lif_rx_agg_fec_exc_err_frm_set(uint32_t rxaggfecexceederrs);
bdmf_error_t ag_drv_lif_rx_agg_fec_exc_err_frm_get(uint32_t *rxaggfecexceederrs);
bdmf_error_t ag_drv_lif_rx_agg_nonfec_good_frm_set(uint32_t rxaggnonfecgood);
bdmf_error_t ag_drv_lif_rx_agg_nonfec_good_frm_get(uint32_t *rxaggnonfecgood);
bdmf_error_t ag_drv_lif_rx_agg_nonfec_good_byte_set(uint32_t rxaggnonfecgoodbytes);
bdmf_error_t ag_drv_lif_rx_agg_nonfec_good_byte_get(uint32_t *rxaggnonfecgoodbytes);
bdmf_error_t ag_drv_lif_rx_agg_err_bytes_set(uint32_t rxaggerrbytes);
bdmf_error_t ag_drv_lif_rx_agg_err_bytes_get(uint32_t *rxaggerrbytes);
bdmf_error_t ag_drv_lif_rx_agg_err_zeroes_set(uint32_t rxaggerrzeroes);
bdmf_error_t ag_drv_lif_rx_agg_err_zeroes_get(uint32_t *rxaggerrzeroes);
bdmf_error_t ag_drv_lif_rx_agg_no_err_blks_set(uint32_t rxaggnoerrblks);
bdmf_error_t ag_drv_lif_rx_agg_no_err_blks_get(uint32_t *rxaggnoerrblks);
bdmf_error_t ag_drv_lif_rx_agg_cor_blks_set(uint32_t rxaggcorrblks);
bdmf_error_t ag_drv_lif_rx_agg_cor_blks_get(uint32_t *rxaggcorrblks);
bdmf_error_t ag_drv_lif_rx_agg_uncor_blks_set(uint32_t rxagguncorrblks);
bdmf_error_t ag_drv_lif_rx_agg_uncor_blks_get(uint32_t *rxagguncorrblks);
bdmf_error_t ag_drv_lif_rx_agg_err_ones_set(uint32_t rxaggerrones);
bdmf_error_t ag_drv_lif_rx_agg_err_ones_get(uint32_t *rxaggerrones);
bdmf_error_t ag_drv_lif_rx_agg_err_frm_set(uint32_t rxaggerroredcnt);
bdmf_error_t ag_drv_lif_rx_agg_err_frm_get(uint32_t *rxaggerroredcnt);
bdmf_error_t ag_drv_lif_tx_pkt_cnt_set(uint32_t txframecnt);
bdmf_error_t ag_drv_lif_tx_pkt_cnt_get(uint32_t *txframecnt);
bdmf_error_t ag_drv_lif_tx_byte_cnt_set(uint32_t txbytecnt);
bdmf_error_t ag_drv_lif_tx_byte_cnt_get(uint32_t *txbytecnt);
bdmf_error_t ag_drv_lif_tx_non_fec_pkt_cnt_set(uint32_t txnonfecframecnt);
bdmf_error_t ag_drv_lif_tx_non_fec_pkt_cnt_get(uint32_t *txnonfecframecnt);
bdmf_error_t ag_drv_lif_tx_non_fec_byte_cnt_set(uint32_t txnonfecbytecnt);
bdmf_error_t ag_drv_lif_tx_non_fec_byte_cnt_get(uint32_t *txnonfecbytecnt);
bdmf_error_t ag_drv_lif_tx_fec_pkt_cnt_set(uint32_t txfecframecnt);
bdmf_error_t ag_drv_lif_tx_fec_pkt_cnt_get(uint32_t *txfecframecnt);
bdmf_error_t ag_drv_lif_tx_fec_byte_cnt_set(uint32_t txfecbytecnt);
bdmf_error_t ag_drv_lif_tx_fec_byte_cnt_get(uint32_t *txfecbytecnt);
bdmf_error_t ag_drv_lif_tx_fec_blk_cnt_set(uint32_t txfecblkscnt);
bdmf_error_t ag_drv_lif_tx_fec_blk_cnt_get(uint32_t *txfecblkscnt);
bdmf_error_t ag_drv_lif_tx_mpcp_pkt_cnt_set(uint32_t txmpcpframecnt);
bdmf_error_t ag_drv_lif_tx_mpcp_pkt_cnt_get(uint32_t *txmpcpframecnt);
bdmf_error_t ag_drv_lif_debug_tx_data_pkt_cnt_set(uint32_t txdataframecnt);
bdmf_error_t ag_drv_lif_debug_tx_data_pkt_cnt_get(uint32_t *txdataframecnt);
bdmf_error_t ag_drv_lif_fec_llid_status_set(uint16_t stkyfecrevcllidbmsk);
bdmf_error_t ag_drv_lif_fec_llid_status_get(uint16_t *stkyfecrevcllidbmsk);
bdmf_error_t ag_drv_lif_sec_rx_tek_ig_iv_llid_set(uint32_t cfigivnullllid);
bdmf_error_t ag_drv_lif_sec_rx_tek_ig_iv_llid_get(uint32_t *cfigivnullllid);
bdmf_error_t ag_drv_lif_pon_ber_interv_thresh_set(uint32_t cfrxlifberinterval, uint16_t cfrxlifberthreshld, uint8_t cfrxlifbercntrl);
bdmf_error_t ag_drv_lif_pon_ber_interv_thresh_get(uint32_t *cfrxlifberinterval, uint16_t *cfrxlifberthreshld, uint8_t *cfrxlifbercntrl);
bdmf_error_t ag_drv_lif_lsr_mon_a_ctrl_set(bdmf_boolean iopbilaserens1a, bdmf_boolean cfglsrmonacthi, bdmf_boolean pbilasermonrsta_n_pre);
bdmf_error_t ag_drv_lif_lsr_mon_a_ctrl_get(bdmf_boolean *iopbilaserens1a, bdmf_boolean *cfglsrmonacthi, bdmf_boolean *pbilasermonrsta_n_pre);
bdmf_error_t ag_drv_lif_lsr_mon_a_max_thr_set(uint32_t cfglasermonmaxa);
bdmf_error_t ag_drv_lif_lsr_mon_a_max_thr_get(uint32_t *cfglasermonmaxa);
bdmf_error_t ag_drv_lif_lsr_mon_a_bst_len_get(uint32_t *laserontimea);
bdmf_error_t ag_drv_lif_lsr_mon_a_bst_cnt_get(uint32_t *lasermonbrstcnta);
bdmf_error_t ag_drv_lif_debug_pon_sm_get(uint8_t *aligncsqq, uint8_t *rxfecifcsqq);
bdmf_error_t ag_drv_lif_debug_fec_sm_get(uint8_t *rxsyncsqq, uint8_t *rxcorcs, uint8_t *fecrxoutcs);
bdmf_error_t ag_drv_lif_ae_pktnum_window_set(uint32_t cfgaepktnumwnd);
bdmf_error_t ag_drv_lif_ae_pktnum_window_get(uint32_t *cfgaepktnumwnd);
bdmf_error_t ag_drv_lif_ae_pktnum_thresh_set(uint32_t cfgpktnummaxthresh);
bdmf_error_t ag_drv_lif_ae_pktnum_thresh_get(uint32_t *cfgpktnummaxthresh);
bdmf_error_t ag_drv_lif_ae_pktnum_stat_get(uint8_t *secupindxwtpktnummax, uint8_t *secdnindxwtpktnumabort);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_lif_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

