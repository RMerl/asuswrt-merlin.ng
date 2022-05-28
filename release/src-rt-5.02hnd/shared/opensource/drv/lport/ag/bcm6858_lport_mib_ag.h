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

#ifndef _BCM6858_LPORT_MIB_AG_H_
#define _BCM6858_LPORT_MIB_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"
int ag_drv_lport_mib_grx64_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grx64_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grx127_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grx127_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grx255_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grx255_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grx511_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grx511_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grx1023_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grx1023_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grx1518_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grx1518_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grx1522_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grx1522_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grx2047_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grx2047_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grx4095_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grx4095_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grx9216_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grx9216_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grx16383_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grx16383_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpkt_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpkt_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxuca_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxuca_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxmca_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxmca_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxbca_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxbca_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxfcs_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxfcs_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxcf_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxcf_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpf_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpf_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpp_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpp_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxuo_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxuo_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxuda_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxuda_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxwsa_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxwsa_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxaln_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxaln_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxflr_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxflr_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxfrerr_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxfrerr_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxfcr_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxfcr_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxovr_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxovr_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxjbr_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxjbr_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxmtue_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxmtue_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxmcrc_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxmcrc_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxprm_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxprm_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxvln_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxvln_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxdvln_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxdvln_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxtrfu_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxtrfu_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpok_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpok_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcoff0_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcoff0_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcoff1_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcoff1_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcoff2_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcoff2_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcoff3_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcoff3_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcoff4_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcoff4_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcoff5_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcoff5_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcoff6_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcoff6_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcoff7_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcoff7_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcp0_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcp0_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcp1_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcp1_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcp2_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcp2_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcp3_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcp3_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcp4_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcp4_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcp5_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcp5_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcp6_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcp6_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxpfcp7_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxpfcp7_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxschcrc_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxschcrc_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxbyt_set(uint8_t port_id, uint64_t count48);
int ag_drv_lport_mib_grxbyt_get(uint8_t port_id, uint64_t *count48);
int ag_drv_lport_mib_grxrpkt_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxrpkt_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxund_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxund_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxfrg_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxfrg_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxrbyt_set(uint8_t port_id, uint64_t count48);
int ag_drv_lport_mib_grxrbyt_get(uint8_t port_id, uint64_t *count48);
int ag_drv_lport_mib_gtx64_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtx64_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtx127_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtx127_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtx255_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtx255_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtx511_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtx511_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtx1023_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtx1023_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtx1518_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtx1518_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtx1522_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtx1522_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtx2047_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtx2047_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtx4095_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtx4095_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtx9216_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtx9216_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtx16383_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtx16383_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxpok_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxpok_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxpkt_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxpkt_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxuca_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxuca_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxmca_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxmca_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxbca_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxbca_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxpf_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxpf_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxpfc_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxpfc_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxjbr_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxjbr_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxfcs_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxfcs_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxcf_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxcf_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxovr_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxovr_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxdfr_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxdfr_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxedf_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxedf_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxscl_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxscl_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxmcl_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxmcl_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxlcl_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxlcl_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxxcl_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxxcl_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxfrg_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxfrg_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxerr_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxerr_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxvln_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxvln_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxdvln_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxdvln_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxrpkt_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxrpkt_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxufl_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxufl_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxpfcp0_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxpfcp0_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxpfcp1_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxpfcp1_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxpfcp2_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxpfcp2_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxpfcp3_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxpfcp3_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxpfcp4_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxpfcp4_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxpfcp5_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxpfcp5_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxpfcp6_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxpfcp6_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxpfcp7_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxpfcp7_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxncl_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxncl_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxbyt_set(uint8_t port_id, uint64_t count48);
int ag_drv_lport_mib_gtxbyt_get(uint8_t port_id, uint64_t *count48);
int ag_drv_lport_mib_grxlpi_set(uint8_t port_id, uint32_t count32);
int ag_drv_lport_mib_grxlpi_get(uint8_t port_id, uint32_t *count32);
int ag_drv_lport_mib_grxdlpi_set(uint8_t port_id, uint32_t count32);
int ag_drv_lport_mib_grxdlpi_get(uint8_t port_id, uint32_t *count32);
int ag_drv_lport_mib_gtxlpi_set(uint8_t port_id, uint32_t count32);
int ag_drv_lport_mib_gtxlpi_get(uint8_t port_id, uint32_t *count32);
int ag_drv_lport_mib_gtxdlpi_set(uint8_t port_id, uint32_t count32);
int ag_drv_lport_mib_gtxdlpi_get(uint8_t port_id, uint32_t *count32);
int ag_drv_lport_mib_grxptllfc_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxptllfc_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxltllfc_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxltllfc_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_grxllfcfcs_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_grxllfcfcs_get(uint8_t port_id, uint64_t *count40);
int ag_drv_lport_mib_gtxltllfc_set(uint8_t port_id, uint64_t count40);
int ag_drv_lport_mib_gtxltllfc_get(uint8_t port_id, uint64_t *count40);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_lport_mib_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

