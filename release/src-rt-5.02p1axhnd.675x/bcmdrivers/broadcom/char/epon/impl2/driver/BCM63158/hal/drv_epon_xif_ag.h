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

#ifndef _DRV_EPON_XIF_AG_H_
#define _DRV_EPON_XIF_AG_H_

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
#include "drv_epon_link_array_ag.h"

typedef struct
{
    bdmf_boolean rxencrypten;
    bdmf_boolean cfgdisrxdasaencrpt;
    uint8_t rxencryptmode;
    bdmf_boolean txencrypten;
    bdmf_boolean cfgdistxdasaencrpt;
    uint8_t txencryptmode;
    bdmf_boolean cfgllidmodemsk;
    bdmf_boolean cfgxpnbadcrc32;
    bdmf_boolean cfgdisdiscinfo;
    bdmf_boolean cfgpmctx2rxlpbk;
    bdmf_boolean cfgpmctxencrc8bad;
    bdmf_boolean cfgenp2p;
    bdmf_boolean cfgllidpromiscuousmode;
    bdmf_boolean cfgenidlepktsup;
    bdmf_boolean cfgpmcrxencrc8chk;
    bdmf_boolean cfgen1stidlepktconvert;
    bdmf_boolean cfgfecen;
    bdmf_boolean cfglegacyrcvtsupd;
    bdmf_boolean cfgxpnencrcpassthru;
    bdmf_boolean cfgxpndistimestampmod;
    bdmf_boolean xifnotrdy;
    bdmf_boolean xifdtportrstn;
    bdmf_boolean xpntxrstn;
    bdmf_boolean pmctxrstn;
    bdmf_boolean sectxrstn;
    bdmf_boolean cfgdistxoamencrpt;
    bdmf_boolean cfgdistxmpcpencrpt;
    bdmf_boolean pmcrxrstn;
    bdmf_boolean secrxrstn;
} xif_ctl;

typedef struct
{
    bdmf_boolean secrxrplyprtctabrtint;
    bdmf_boolean sectxpktnummaxint;
    bdmf_boolean tsfullupdint;
    bdmf_boolean txhangint;
    bdmf_boolean negtimeint;
    bdmf_boolean pmctsjttrint;
    bdmf_boolean secrxoutffovrflwint;
} xif_int_status;

typedef struct
{
    bdmf_boolean msksecrxreplayprotctabort;
    bdmf_boolean mskpktnumthreshint;
    bdmf_boolean msktsfullupdint;
    bdmf_boolean msktxhangint;
    bdmf_boolean msknegtimeint;
    bdmf_boolean mskpmctsjttrint;
    bdmf_boolean msksecrxoutffint;
} xif_int_mask;

typedef struct
{
    uint16_t cfgonuburstsize;
    bdmf_boolean cfgenbck2bckpktgen;
    bdmf_boolean cfgenallmpcppktgen;
    bdmf_boolean cfgxpnstartpktgen;
    bdmf_boolean cfgxpnenpktgen;
} xif_xpn_pktgen_ctl;

typedef struct
{
    uint8_t cfgmpcpupdperiod;
    bdmf_boolean cfgdis4idleb4startchar;
    bdmf_boolean cfgenidledscrd;
    bdmf_boolean cfgseltxpontime;
    bdmf_boolean cfgmpcpcontupd;
    bdmf_boolean cfgenmaxmpcpupd;
    bdmf_boolean cfgennegtimeabort;
} xif_pmctx_ctl;

typedef struct
{
    bdmf_boolean cfgsecrxenshortlen;
    bdmf_boolean cfgensectxfakeaes;
    bdmf_boolean cfgensecrxfakeaes;
    bdmf_boolean cfgsecrxenpktnumrlovr;
    bdmf_boolean cfgsectxenpktnumrlovr;
    bdmf_boolean cfgenaereplayprct;
} xif_sec_ctl;

bdmf_error_t ag_drv_xif_ctl_set(const xif_ctl *ctl);
bdmf_error_t ag_drv_xif_ctl_get(xif_ctl *ctl);
bdmf_error_t ag_drv_xif_int_status_set(const xif_int_status *int_status);
bdmf_error_t ag_drv_xif_int_status_get(xif_int_status *int_status);
bdmf_error_t ag_drv_xif_int_mask_set(const xif_int_mask *int_mask);
bdmf_error_t ag_drv_xif_int_mask_get(xif_int_mask *int_mask);
bdmf_error_t ag_drv_xif_port_command_set(bdmf_boolean dataportbusy, uint8_t portselect, uint8_t portopcode, uint16_t portaddress);
bdmf_error_t ag_drv_xif_port_command_get(bdmf_boolean *dataportbusy, uint8_t *portselect, uint8_t *portopcode, uint16_t *portaddress);
bdmf_error_t ag_drv_xif_port_data__set(uint8_t portidx, uint32_t portdata);
bdmf_error_t ag_drv_xif_port_data__get(uint8_t portidx, uint32_t *portdata);
bdmf_error_t ag_drv_xif_macsec_set(uint16_t cfgmacsecethertype);
bdmf_error_t ag_drv_xif_macsec_get(uint16_t *cfgmacsecethertype);
bdmf_error_t ag_drv_xif_xpn_xmt_offset_set(uint16_t cfgxpnxmtoffset);
bdmf_error_t ag_drv_xif_xpn_xmt_offset_get(uint16_t *cfgxpnxmtoffset);
bdmf_error_t ag_drv_xif_xpn_timestamp_offset_set(uint32_t cfgxpnmpcptsoffset);
bdmf_error_t ag_drv_xif_xpn_timestamp_offset_get(uint32_t *cfgxpnmpcptsoffset);
bdmf_error_t ag_drv_xif_xpn_pktgen_ctl_set(const xif_xpn_pktgen_ctl *xpn_pktgen_ctl);
bdmf_error_t ag_drv_xif_xpn_pktgen_ctl_get(xif_xpn_pktgen_ctl *xpn_pktgen_ctl);
bdmf_error_t ag_drv_xif_xpn_pktgen_llid_set(uint16_t cfgxpnpktgenllid1, uint16_t cfgxpnpktgenllid0);
bdmf_error_t ag_drv_xif_xpn_pktgen_llid_get(uint16_t *cfgxpnpktgenllid1, uint16_t *cfgxpnpktgenllid0);
bdmf_error_t ag_drv_xif_xpn_pktgen_pkt_cnt_set(bdmf_boolean cfgxpnpktgenburstmode, uint32_t cfgxpnpktgenburstsize);
bdmf_error_t ag_drv_xif_xpn_pktgen_pkt_cnt_get(bdmf_boolean *cfgxpnpktgenburstmode, uint32_t *cfgxpnpktgenburstsize);
bdmf_error_t ag_drv_xif_xpn_pktgen_pkt_size_set(bdmf_boolean cfgxpnpktgensizeincr, uint16_t cfgxpnpktgensizeend, uint16_t cfgxpnpktgensizestart);
bdmf_error_t ag_drv_xif_xpn_pktgen_pkt_size_get(bdmf_boolean *cfgxpnpktgensizeincr, uint16_t *cfgxpnpktgensizeend, uint16_t *cfgxpnpktgensizestart);
bdmf_error_t ag_drv_xif_xpn_pktgen_ipg_set(uint16_t cfgxpnpktgenbck2bckipg, uint16_t cfgxpnpktgenipg);
bdmf_error_t ag_drv_xif_xpn_pktgen_ipg_get(uint16_t *cfgxpnpktgenbck2bckipg, uint16_t *cfgxpnpktgenipg);
bdmf_error_t ag_drv_xif_ts_jitter_thresh_set(uint32_t cfgtsjttrthresh);
bdmf_error_t ag_drv_xif_ts_jitter_thresh_get(uint32_t *cfgtsjttrthresh);
bdmf_error_t ag_drv_xif_ts_update_set(uint16_t cfgtsfullupdthr, bdmf_boolean cfgenautotsupd, uint8_t cfgtsupdper);
bdmf_error_t ag_drv_xif_ts_update_get(uint16_t *cfgtsfullupdthr, bdmf_boolean *cfgenautotsupd, uint8_t *cfgtsupdper);
bdmf_error_t ag_drv_xif_gnt_overhead_set(uint16_t cfggntoh);
bdmf_error_t ag_drv_xif_gnt_overhead_get(uint16_t *cfggntoh);
bdmf_error_t ag_drv_xif_discover_overhead_set(uint16_t cfgdiscoh);
bdmf_error_t ag_drv_xif_discover_overhead_get(uint16_t *cfgdiscoh);
bdmf_error_t ag_drv_xif_discover_info_set(uint16_t cfgdiscinfofld);
bdmf_error_t ag_drv_xif_discover_info_get(uint16_t *cfgdiscinfofld);
bdmf_error_t ag_drv_xif_xpn_oversize_thresh_set(uint16_t cfgxpnovrszthresh);
bdmf_error_t ag_drv_xif_xpn_oversize_thresh_get(uint16_t *cfgxpnovrszthresh);
bdmf_error_t ag_drv_xif_secrx_keynum_get(uint32_t *keystatrx);
bdmf_error_t ag_drv_xif_secrx_encrypt_get(uint32_t *encrstatrx);
bdmf_error_t ag_drv_xif_pmc_frame_rx_cnt_get(uint32_t *pmcrxframecnt);
bdmf_error_t ag_drv_xif_pmc_byte_rx_cnt_get(uint32_t *pmcrxbytecnt);
bdmf_error_t ag_drv_xif_pmc_runt_rx_cnt_get(uint32_t *pmcrxruntcnt);
bdmf_error_t ag_drv_xif_pmc_cw_err_rx_cnt_get(uint32_t *pmcrxcwerrcnt);
bdmf_error_t ag_drv_xif_pmc_crc8_err_rx_cnt_get(uint32_t *pmcrxcrc8errcnt);
bdmf_error_t ag_drv_xif_xpn_data_frm_cnt_get(uint32_t *xpndtframecnt);
bdmf_error_t ag_drv_xif_xpn_data_byte_cnt_get(uint32_t *xpndtbytecnt);
bdmf_error_t ag_drv_xif_xpn_mpcp_frm_cnt_get(uint32_t *xpnmpcpframecnt);
bdmf_error_t ag_drv_xif_xpn_oam_frm_cnt_get(uint32_t *xpnoamframecnt);
bdmf_error_t ag_drv_xif_xpn_oam_byte_cnt_get(uint32_t *xpnoambytecnt);
bdmf_error_t ag_drv_xif_xpn_oversize_frm_cnt_get(uint32_t *xpndtoversizecnt);
bdmf_error_t ag_drv_xif_sec_abort_frm_cnt_get(uint32_t *secrxabortfrmcnt);
bdmf_error_t ag_drv_xif_pmc_tx_neg_event_cnt_get(uint8_t *pmctxnegeventcnt);
bdmf_error_t ag_drv_xif_xpn_idle_pkt_cnt_get(uint16_t *xpnidleframecnt);
bdmf_error_t ag_drv_xif_llid_0_set(uint32_t cfgonullid0);
bdmf_error_t ag_drv_xif_llid_0_get(uint32_t *cfgonullid0);
bdmf_error_t ag_drv_xif_llid_1_set(uint32_t cfgonullid1);
bdmf_error_t ag_drv_xif_llid_1_get(uint32_t *cfgonullid1);
bdmf_error_t ag_drv_xif_llid_2_set(uint32_t cfgonullid2);
bdmf_error_t ag_drv_xif_llid_2_get(uint32_t *cfgonullid2);
bdmf_error_t ag_drv_xif_llid_3_set(uint32_t cfgonullid3);
bdmf_error_t ag_drv_xif_llid_3_get(uint32_t *cfgonullid3);
bdmf_error_t ag_drv_xif_llid_4_set(uint32_t cfgonullid4);
bdmf_error_t ag_drv_xif_llid_4_get(uint32_t *cfgonullid4);
bdmf_error_t ag_drv_xif_llid_5_set(uint32_t cfgonullid5);
bdmf_error_t ag_drv_xif_llid_5_get(uint32_t *cfgonullid5);
bdmf_error_t ag_drv_xif_llid_6_set(uint32_t cfgonullid6);
bdmf_error_t ag_drv_xif_llid_6_get(uint32_t *cfgonullid6);
bdmf_error_t ag_drv_xif_llid_7_set(uint32_t cfgonullid7);
bdmf_error_t ag_drv_xif_llid_7_get(uint32_t *cfgonullid7);
bdmf_error_t ag_drv_xif_llid_8_set(uint32_t cfgonullid8);
bdmf_error_t ag_drv_xif_llid_8_get(uint32_t *cfgonullid8);
bdmf_error_t ag_drv_xif_llid_9_set(uint32_t cfgonullid9);
bdmf_error_t ag_drv_xif_llid_9_get(uint32_t *cfgonullid9);
bdmf_error_t ag_drv_xif_llid_10_set(uint32_t cfgonullid10);
bdmf_error_t ag_drv_xif_llid_10_get(uint32_t *cfgonullid10);
bdmf_error_t ag_drv_xif_llid_11_set(uint32_t cfgonullid11);
bdmf_error_t ag_drv_xif_llid_11_get(uint32_t *cfgonullid11);
bdmf_error_t ag_drv_xif_llid_12_set(uint32_t cfgonullid12);
bdmf_error_t ag_drv_xif_llid_12_get(uint32_t *cfgonullid12);
bdmf_error_t ag_drv_xif_llid_13_set(uint32_t cfgonullid13);
bdmf_error_t ag_drv_xif_llid_13_get(uint32_t *cfgonullid13);
bdmf_error_t ag_drv_xif_llid_14_set(uint32_t cfgonullid14);
bdmf_error_t ag_drv_xif_llid_14_get(uint32_t *cfgonullid14);
bdmf_error_t ag_drv_xif_llid_15_set(uint32_t cfgonullid15);
bdmf_error_t ag_drv_xif_llid_15_get(uint32_t *cfgonullid15);
bdmf_error_t ag_drv_xif_llid_16_set(uint32_t cfgonullid16);
bdmf_error_t ag_drv_xif_llid_16_get(uint32_t *cfgonullid16);
bdmf_error_t ag_drv_xif_llid_17_set(uint32_t cfgonullid17);
bdmf_error_t ag_drv_xif_llid_17_get(uint32_t *cfgonullid17);
bdmf_error_t ag_drv_xif_llid_18_set(uint32_t cfgonullid18);
bdmf_error_t ag_drv_xif_llid_18_get(uint32_t *cfgonullid18);
bdmf_error_t ag_drv_xif_llid_19_set(uint32_t cfgonullid19);
bdmf_error_t ag_drv_xif_llid_19_get(uint32_t *cfgonullid19);
bdmf_error_t ag_drv_xif_llid_20_set(uint32_t cfgonullid20);
bdmf_error_t ag_drv_xif_llid_20_get(uint32_t *cfgonullid20);
bdmf_error_t ag_drv_xif_llid_21_set(uint32_t cfgonullid21);
bdmf_error_t ag_drv_xif_llid_21_get(uint32_t *cfgonullid21);
bdmf_error_t ag_drv_xif_llid_22_set(uint32_t cfgonullid22);
bdmf_error_t ag_drv_xif_llid_22_get(uint32_t *cfgonullid22);
bdmf_error_t ag_drv_xif_llid_23_set(uint32_t cfgonullid23);
bdmf_error_t ag_drv_xif_llid_23_get(uint32_t *cfgonullid23);
bdmf_error_t ag_drv_xif_llid_24_set(uint32_t cfgonullid24);
bdmf_error_t ag_drv_xif_llid_24_get(uint32_t *cfgonullid24);
bdmf_error_t ag_drv_xif_llid_25_set(uint32_t cfgonullid25);
bdmf_error_t ag_drv_xif_llid_25_get(uint32_t *cfgonullid25);
bdmf_error_t ag_drv_xif_llid_26_set(uint32_t cfgonullid26);
bdmf_error_t ag_drv_xif_llid_26_get(uint32_t *cfgonullid26);
bdmf_error_t ag_drv_xif_llid_27_set(uint32_t cfgonullid27);
bdmf_error_t ag_drv_xif_llid_27_get(uint32_t *cfgonullid27);
bdmf_error_t ag_drv_xif_llid_28_set(uint32_t cfgonullid28);
bdmf_error_t ag_drv_xif_llid_28_get(uint32_t *cfgonullid28);
bdmf_error_t ag_drv_xif_llid_29_set(uint32_t cfgonullid29);
bdmf_error_t ag_drv_xif_llid_29_get(uint32_t *cfgonullid29);
bdmf_error_t ag_drv_xif_llid_30_set(uint32_t cfgonullid30);
bdmf_error_t ag_drv_xif_llid_30_get(uint32_t *cfgonullid30);
bdmf_error_t ag_drv_xif_llid_31_set(uint32_t cfgonullid31);
bdmf_error_t ag_drv_xif_llid_31_get(uint32_t *cfgonullid31);
bdmf_error_t ag_drv_xif_max_mpcp_update_set(uint32_t cfgmaxposmpcpupd);
bdmf_error_t ag_drv_xif_max_mpcp_update_get(uint32_t *cfgmaxposmpcpupd);
bdmf_error_t ag_drv_xif_ipg_insertion_set(bdmf_boolean cfgshortipg, bdmf_boolean cfginsertipg, uint8_t cfgipgword);
bdmf_error_t ag_drv_xif_ipg_insertion_get(bdmf_boolean *cfgshortipg, bdmf_boolean *cfginsertipg, uint8_t *cfgipgword);
bdmf_error_t ag_drv_xif_transport_time_set(uint32_t cftransporttime);
bdmf_error_t ag_drv_xif_transport_time_get(uint32_t *cftransporttime);
bdmf_error_t ag_drv_xif_mpcp_time_get(uint32_t *curmpcpts);
bdmf_error_t ag_drv_xif_overlap_gnt_oh_set(uint32_t cfgovrlpoh);
bdmf_error_t ag_drv_xif_overlap_gnt_oh_get(uint32_t *cfgovrlpoh);
bdmf_error_t ag_drv_xif_mac_mode_set(bdmf_boolean cfgennogntxmt);
bdmf_error_t ag_drv_xif_mac_mode_get(bdmf_boolean *cfgennogntxmt);
bdmf_error_t ag_drv_xif_pmctx_ctl_set(const xif_pmctx_ctl *pmctx_ctl);
bdmf_error_t ag_drv_xif_pmctx_ctl_get(xif_pmctx_ctl *pmctx_ctl);
bdmf_error_t ag_drv_xif_sec_ctl_set(const xif_sec_ctl *sec_ctl);
bdmf_error_t ag_drv_xif_sec_ctl_get(xif_sec_ctl *sec_ctl);
bdmf_error_t ag_drv_xif_ae_pktnum_window_set(uint32_t cfgaepktnumwnd);
bdmf_error_t ag_drv_xif_ae_pktnum_window_get(uint32_t *cfgaepktnumwnd);
bdmf_error_t ag_drv_xif_ae_pktnum_thresh_set(uint32_t cfgpktnummaxthresh);
bdmf_error_t ag_drv_xif_ae_pktnum_thresh_get(uint32_t *cfgpktnummaxthresh);
bdmf_error_t ag_drv_xif_sectx_keynum_get(uint32_t *keystattx);
bdmf_error_t ag_drv_xif_sectx_encrypt_get(uint32_t *encrstattx);
bdmf_error_t ag_drv_xif_ae_pktnum_stat_get(uint8_t *sectxindxwtpktnummax, uint8_t *secrxindxwtpktnumabort);
bdmf_error_t ag_drv_xif_mpcp_update_get(uint32_t *mpcpupdperiod);
bdmf_error_t ag_drv_xif_burst_prelaunch_offset_set(uint32_t cfgburstprelaunchoffset);
bdmf_error_t ag_drv_xif_burst_prelaunch_offset_get(uint32_t *cfgburstprelaunchoffset);
bdmf_error_t ag_drv_xif_vlan_type_set(uint16_t cfgvlantype);
bdmf_error_t ag_drv_xif_vlan_type_get(uint16_t *cfgvlantype);
bdmf_error_t ag_drv_xif_p2p_ae_sci_en_set(uint16_t cfgp2pscien);
bdmf_error_t ag_drv_xif_p2p_ae_sci_en_get(uint16_t *cfgp2pscien);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_0_set(uint32_t cfgp2psci_lo_0);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_0_get(uint32_t *cfgp2psci_lo_0);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_0_set(uint32_t cfgp2psci_hi_0);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_0_get(uint32_t *cfgp2psci_hi_0);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_1_set(uint32_t cfgp2psci_lo_1);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_1_get(uint32_t *cfgp2psci_lo_1);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_1_set(uint32_t cfgp2psci_hi_1);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_1_get(uint32_t *cfgp2psci_hi_1);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_2_set(uint32_t cfgp2psci_lo_2);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_2_get(uint32_t *cfgp2psci_lo_2);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_2_set(uint32_t cfgp2psci_hi_2);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_2_get(uint32_t *cfgp2psci_hi_2);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_3_set(uint32_t cfgp2psci_lo_3);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_3_get(uint32_t *cfgp2psci_lo_3);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_3_set(uint32_t cfgp2psci_hi_3);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_3_get(uint32_t *cfgp2psci_hi_3);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_4_set(uint32_t cfgp2psci_lo_4);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_4_get(uint32_t *cfgp2psci_lo_4);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_4_set(uint32_t cfgp2psci_hi_4);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_4_get(uint32_t *cfgp2psci_hi_4);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_5_set(uint32_t cfgp2psci_lo_5);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_5_get(uint32_t *cfgp2psci_lo_5);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_5_set(uint32_t cfgp2psci_hi_5);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_5_get(uint32_t *cfgp2psci_hi_5);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_6_set(uint32_t cfgp2psci_lo_6);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_6_get(uint32_t *cfgp2psci_lo_6);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_6_set(uint32_t cfgp2psci_hi_6);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_6_get(uint32_t *cfgp2psci_hi_6);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_7_set(uint32_t cfgp2psci_lo_7);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_7_get(uint32_t *cfgp2psci_lo_7);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_7_set(uint32_t cfgp2psci_hi_7);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_7_get(uint32_t *cfgp2psci_hi_7);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_8_set(uint32_t cfgp2psci_lo_8);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_8_get(uint32_t *cfgp2psci_lo_8);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_8_set(uint32_t cfgp2psci_hi_8);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_8_get(uint32_t *cfgp2psci_hi_8);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_9_set(uint32_t cfgp2psci_lo_9);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_9_get(uint32_t *cfgp2psci_lo_9);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_9_set(uint32_t cfgp2psci_hi_9);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_9_get(uint32_t *cfgp2psci_hi_9);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_10_set(uint32_t cfgp2psci_lo_10);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_10_get(uint32_t *cfgp2psci_lo_10);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_10_set(uint32_t cfgp2psci_hi_10);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_10_get(uint32_t *cfgp2psci_hi_10);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_11_set(uint32_t cfgp2psci_lo_11);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_11_get(uint32_t *cfgp2psci_lo_11);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_11_set(uint32_t cfgp2psci_hi_11);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_11_get(uint32_t *cfgp2psci_hi_11);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_12_set(uint32_t cfgp2psci_lo_12);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_12_get(uint32_t *cfgp2psci_lo_12);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_12_set(uint32_t cfgp2psci_hi_12);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_12_get(uint32_t *cfgp2psci_hi_12);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_13_set(uint32_t cfgp2psci_lo_13);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_13_get(uint32_t *cfgp2psci_lo_13);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_13_set(uint32_t cfgp2psci_hi_13);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_13_get(uint32_t *cfgp2psci_hi_13);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_14_set(uint32_t cfgp2psci_lo_14);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_14_get(uint32_t *cfgp2psci_lo_14);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_14_set(uint32_t cfgp2psci_hi_14);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_14_get(uint32_t *cfgp2psci_hi_14);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_15_set(uint32_t cfgp2psci_lo_15);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_15_get(uint32_t *cfgp2psci_lo_15);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_15_set(uint32_t cfgp2psci_hi_15);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_15_get(uint32_t *cfgp2psci_hi_15);
bdmf_error_t ag_drv_xif_sectx_keynum_1_get(uint32_t *keystattx_hi);
bdmf_error_t ag_drv_xif_secrx_keynum_1_get(uint32_t *keystatrx_hi);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xif_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

