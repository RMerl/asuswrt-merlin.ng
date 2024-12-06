/******************************************************************************
 *  Copyright (C) 2021 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * <:label-BRCM:2022:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 *
 * Module Description:
 */
 
#include "bchp_isec_port_0.h"
#include "bchp_isec_general.h"
#include "bchp_esec_port_0.h"
#include "bchp_esec_general.h"
#include "bchp_macsec_port_0.h"
#include "bchp_macsec_general.h"
#include "bchp_eth_phy_top_reg.h"
#include "bchp_xport_xlmac_core_0.h"
#include "bchp_eth_r2sbus_bridge.h"

#ifndef BCHP_REGS_INT_H__
#define BCHP_REGS_INT_H__

#define BCHP_MACSEC_BASE_ADDR               0x83700000
#define BCM_PHYS_REG_OFFSET(x)              (x - BCHP_MACSEC_BASE_ADDR)

extern soc_ubus_reg_t MACSEC_CNTRLreg;
extern soc_ubus_reg_t ISEC_PP_CTRLreg;
extern soc_ubus_reg_t ISEC_RUD_MGMT_RULE_CTRLreg;
extern soc_ubus_reg_t ISEC_AES_ICV_FAIL_CNTreg;
extern soc_ubus_reg_t ISEC_MTU_FAIL_CNTreg;
extern soc_ubus_reg_t ISEC_PDF_STATUSreg;
extern soc_ubus_reg_t ISEC_PCF_BANK0_STATUSreg;
extern soc_ubus_reg_t ISEC_PCF_BANK1_STATUSreg;
extern soc_ubus_reg_t ISEC_CB_STR_FIFO_STATUSreg;
extern soc_ubus_reg_t ISEC_IDF_STATUSreg;
extern soc_ubus_reg_t ISEC_ICF_STATUSreg;
extern soc_ubus_reg_t ISEC_ODF_STATUSreg;
extern soc_ubus_reg_t ISEC_TAG_FIFO_STATUSreg;
extern soc_ubus_reg_t ISEC_CW_FIFO_STATUSreg;
//extern ;

extern soc_ubus_reg_t ISEC_RUD_MGMT_RULE_CTRLreg;
extern soc_ubus_reg_t ISEC_MGMTRULE_CFG_DA_0reg;
extern soc_ubus_reg_t ISEC_MGMTRULE_CFG_DA_1reg;
extern soc_ubus_reg_t ISEC_MGMTRULE_CFG_DA_2reg;
extern soc_ubus_reg_t ISEC_MGMTRULE_CFG_DA_3reg;
extern soc_ubus_reg_t ISEC_MGMTRULE_CFG_DA_4reg;
extern soc_ubus_reg_t ISEC_MGMTRULE_CFG_DA_5reg;
extern soc_ubus_reg_t ISEC_MGMTRULE_CFG_DA_6reg;
extern soc_ubus_reg_t ISEC_MGMTRULE_CFG_DA_7reg;
extern soc_ubus_reg_t ISEC_MGMTRULE_DA_RANGE_LOWreg;
extern soc_ubus_reg_t ISEC_MGMTRULE_DA_RANGE_HIGHreg;
extern soc_ubus_reg_t ISEC_MGMTRULE_DA_ETYPE_1STreg;
extern soc_ubus_reg_t ISEC_MGMTRULE_DA_ETYPE_2NDreg;
extern soc_ubus_reg_t ISEC_CTRLreg;
extern soc_ubus_reg_t ISEC_MISC_CTRLreg;
extern soc_ubus_reg_t ESEC_CONFIGreg;
extern soc_ubus_reg_t ESEC_EGRESS_MTU_FOR_MGMT_PKTreg;
extern soc_ubus_reg_t ESEC_VXLANSEC_DEST_PORT_NOreg;
extern soc_ubus_reg_t ISEC_VXLANSEC_DEST_PORT_NOreg;
extern soc_ubus_reg_t ESEC_SVTAG_ETYPEreg;
extern soc_ubus_reg_t ISEC_PN_EXPIRE_THDreg;
extern soc_ubus_reg_t ISEC_XPN_EXPIRE_THDreg;
extern soc_ubus_reg_t ISEC_SVTAG_CTRLreg;
extern soc_ubus_reg_t ISEC_PTP_DEST_PORT_NOreg;
extern soc_ubus_reg_t ESEC_PN_THDreg;
extern soc_ubus_reg_t ISEC_PBB_TPIDreg;
extern soc_ubus_reg_t ISEC_NIV_ETYPEreg;
extern soc_ubus_reg_t ISEC_SER_CONTROLreg;
extern soc_ubus_reg_t ISEC_PE_ETYPEreg;
extern soc_ubus_reg_t ISEC_MGMTRULE_CFG_ETYPE_0reg;
extern soc_ubus_reg_t ISEC_MGMTRULE_CFG_ETYPE_1reg;
extern soc_ubus_reg_t ISEC_OUT_DESTPORT_NOreg;
extern soc_ubus_reg_t ISEC_MPLS_ETYPEreg;
extern soc_ubus_reg_t ESEC_XPN_THDreg;
extern soc_ubus_reg_t ESEC_XPN_THDreg;
extern soc_ubus_reg_t XLMAC_CTRLreg;
extern soc_ubus_reg_t XLMAC_TX_CTRLreg;
extern soc_ubus_reg_t XLMAC_RX_CTRLreg;
extern soc_ubus_reg_t MACSEC_INTR_ENABLEreg;
extern soc_ubus_reg_t MACSEC_CTRLreg;
extern soc_ubus_reg_t MACSEC_ESEC_ISEC_STATUSreg;
extern soc_ubus_reg_t ESEC_STATUSreg;
extern soc_ubus_reg_t ESEC_IDF_STATUSreg;
extern soc_ubus_reg_t ESEC_ICF_STATUSreg;
extern soc_ubus_reg_t ESEC_ODF_STATUSreg;
extern soc_ubus_reg_t ESEC_TAG_FIFO_STATUSreg;
extern soc_ubus_reg_t ESEC_CW_FIFO_STATUSreg;
extern soc_ubus_reg_t MACSEC_HW_RESET_CONTROLreg;
extern soc_ubus_reg_t MACSEC_TDM_WRAP_PTRreg;

#endif

