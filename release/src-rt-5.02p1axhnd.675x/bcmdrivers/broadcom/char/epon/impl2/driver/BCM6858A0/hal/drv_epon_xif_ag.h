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
    bdmf_boolean cfgpmcrxencrc8chk;
    bdmf_boolean cfgfecen;
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
bdmf_error_t ag_drv_xif_llid__set(uint8_t llid_index, uint32_t cfgonullid);
bdmf_error_t ag_drv_xif_llid__get(uint8_t llid_index, uint32_t *cfgonullid);
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

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xif_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

