/***********************************************************************
 *
 * Copyright (c) 2021  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2021:DUAL/GPL:standard
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
 ************************************************************************/

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/io.h>
#include <linux/delay.h>
#include "proc_cmd.h"
#include <asm/uaccess.h>
#include "macsec_common.h"
#include "macsec_dev.h"
#include "macsec_debug.h"
#include "bchp_regs_int.h"

#define PROC_DIR        "driver/macsec"
#define CMD_PROC_FILE   "cmd"

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *cmd_proc_file;

static int macsec_port_validate(int argc, char *argv[], int port_pos, int min, int max)
{
    int port;

    if (argc < port_pos + 1)
        goto error;

    if (kstrtos32(argv[port_pos], 10, &port))
        goto error;

    if (port < min || port > max)
        goto error;

    return port;

error:
    return -1;
}

#define F32G(reg, fld) soc_ubus_reg32_field_get(mdev->macsec_unit, reg, rval, fld)
#define F64G(reg, fld) (uint32_t)soc_ubus_reg64_field_get(mdev->macsec_unit, reg, rval64, fld)

#if defined(CONFIG_BCM968880)
#define PORT_ID "<port id (0-2)>"
#define VALIDATE_PORT(port, argc, argv) \
{ \
    if ((port = macsec_port_validate(argc, argv, 1, 0, 2)) == -1) \
    { \
        pr_info("%s\n", usage); \
        return -1; \
    } \
}
#else
#error undefined chip
#endif

static int macsec_proc_cmd_status(int argc, char *argv[])
{
    int port;
    macsec_dev_t *mdev;
    uint32_t rval;
    uint64_t rval64;
    char *usage = "Usage: status " PORT_ID;

    VALIDATE_PORT(port, argc, argv);

    mdev = &msec_devs[FL_UNIT][port];
    pr_info("Port %d status:\n", port);
    pr_info("=== MACSEC ===");
    pr_info("control");
    soc_ubus_reg32_get(mdev->macsec_unit, MACSEC_CTRLreg, mdev->macsec_port, &rval);
    pr_info("  SOFT_RESET: %d\n",                       F32G(MACSEC_CTRLreg, SOFT_RESET_fld));
    pr_info("  BYPASS_EN: %d\n",                        F32G(MACSEC_CTRLreg, BYPASS_EN_fld));
    pr_info("ESEC and ISEC status");
    soc_ubus_reg32_get(mdev->macsec_unit, MACSEC_ESEC_ISEC_STATUSreg, mdev->macsec_port, &rval);
    pr_info("  ISEC_STATUS: %d\n",                      F32G(MACSEC_ESEC_ISEC_STATUSreg, ISEC_STATUS_fld));
    pr_info("  ESEC_STATUS: %d\n",                      F32G(MACSEC_ESEC_ISEC_STATUSreg, ESEC_STATUS_fld));
    pr_info("=== ESEC ===");
    soc_ubus_reg32_get(mdev->macsec_unit, ESEC_CONFIGreg, mdev->macsec_port, &rval);
    pr_info("config");
    pr_info("  TX_THRESHOLD: %d\n",                     F32G(ESEC_CONFIGreg, TX_THRESHOLD_fld));
    pr_info("  LAG_FAILOVER_EN: %d\n",                  F32G(ESEC_CONFIGreg, LAG_FAILOVER_EN_fld));
    pr_info("  EN_PORT_BASED_SC: %d\n",                 F32G(ESEC_CONFIGreg, EN_PORT_BASED_SC_fld));
    pr_info("  DROP_OR_FORWARD_BAD_SVTAG_PKTS: %d\n",   F32G(ESEC_CONFIGreg, DROP_OR_FORWARD_BAD_SVTAG_PKTS_fld));
    soc_ubus_reg32_get(mdev->macsec_unit, ESEC_STATUSreg, mdev->macsec_port, &rval);
    pr_info("status");
    pr_info("  LAG_FAILOVER_EN: %d\n",                  F32G(ESEC_STATUSreg, LAG_FAILOVER_STATUS_fld));
    soc_ubus_reg32_get(mdev->macsec_unit, ESEC_IDF_STATUSreg, mdev->macsec_port, &rval);
    pr_info("IDF status");
    pr_info("  CELL_CNT: %d\n",                         F32G(ESEC_IDF_STATUSreg, CELL_CNT_fld));
    pr_info("  CELL_REQ_CNT: %d\n",                     F32G(ESEC_IDF_STATUSreg, CELL_REQ_CNT_fld));
    pr_info("  EMPTY: %d\n",                            F32G(ESEC_IDF_STATUSreg, EMPTY_fld));
    pr_info("  OVERFLOW: %d\n",                         F32G(ESEC_IDF_STATUSreg, OVERFLOW_fld));
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ESEC_IDF_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ESEC_IDF_STATUSreg, SINGLE_BIT_ERR_fld));
    soc_ubus_reg32_get(mdev->macsec_unit, ESEC_ICF_STATUSreg, mdev->macsec_port, &rval);
    pr_info("ICF status");
    pr_info("  CELL_CNT: %d\n",                         F32G(ESEC_ICF_STATUSreg, ICF_CELL_CNT_fld));
    pr_info("  EMPTY: %d\n",                            F32G(ESEC_ICF_STATUSreg, EMPTY_fld));
    pr_info("  OVERFLOW: %d\n",                         F32G(ESEC_ICF_STATUSreg, OVERFLOW_fld));
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ESEC_ICF_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ESEC_ICF_STATUSreg, SINGLE_BIT_ERR_fld));
    soc_ubus_reg32_get(mdev->macsec_unit, ESEC_ODF_STATUSreg, mdev->macsec_port, &rval);
    pr_info("ODF status");
    pr_info("  CREDIT_RESIDUE: %d\n",                   F32G(ESEC_ODF_STATUSreg, CREDIT_RESIDUE_fld));
    pr_info("  CELL_CNT: %d\n",                         F32G(ESEC_ODF_STATUSreg, CELL_CNT_fld));
    pr_info("  CELL_REQ_CNT: %d\n",                     F32G(ESEC_ODF_STATUSreg, CELL_REQ_CNT_fld));
    pr_info("  EMPTY: %d\n",                            F32G(ESEC_ODF_STATUSreg, EMPTY_fld));
    pr_info("  OVERFLOW: %d\n",                         F32G(ESEC_ODF_STATUSreg, OVERFLOW_fld));
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ESEC_ODF_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ESEC_ODF_STATUSreg, SINGLE_BIT_ERR_fld));
    soc_ubus_reg32_get(mdev->macsec_unit, ESEC_TAG_FIFO_STATUSreg, mdev->macsec_port, &rval);
    pr_info("TAG FIFO status");
    pr_info("  EMPTY: %d\n",                            F32G(ESEC_TAG_FIFO_STATUSreg, EMPTY_fld));
    pr_info("  OVERFLOW: %d\n",                         F32G(ESEC_TAG_FIFO_STATUSreg, OVERFLOW_fld));
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ESEC_TAG_FIFO_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ESEC_TAG_FIFO_STATUSreg, SINGLE_BIT_ERR_fld));
    soc_ubus_reg32_get(mdev->macsec_unit, ESEC_CW_FIFO_STATUSreg, mdev->macsec_port, &rval);
    pr_info("CW FIFO status");
    pr_info("  CREDIT_RESIDUE: %d\n",                   F32G(ESEC_CW_FIFO_STATUSreg, CREDIT_RESIDUE_fld));
    pr_info("  CELL_CNT: %d\n",                         F32G(ESEC_CW_FIFO_STATUSreg, CELL_CNT_fld));
    pr_info("  CELL_REQ_CNT: %d\n",                     F32G(ESEC_CW_FIFO_STATUSreg, CELL_REQ_CNT_fld));
    pr_info("  EMPTY: %d\n",                            F32G(ESEC_CW_FIFO_STATUSreg, EMPTY_fld));
    pr_info("  OVERFLOW: %d\n",                         F32G(ESEC_CW_FIFO_STATUSreg, OVERFLOW_fld));
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ESEC_CW_FIFO_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ESEC_CW_FIFO_STATUSreg, SINGLE_BIT_ERR_fld));

    pr_info("=== ISEC ===");
    pr_info("Per-Port ISEC control");
    soc_ubus_reg_get(mdev->macsec_unit, ISEC_PP_CTRLreg, mdev->macsec_port, &rval64);
    pr_info("  MTU: %d\n", F64G(ISEC_PP_CTRLreg, MTU_fld));
    pr_info("  VXLAN_IPV6_W_UDP_SPTCAM_UDF_PAYLD_SEL: %d\n", F64G(ISEC_PP_CTRLreg, VXLAN_IPV6_W_UDP_SPTCAM_UDF_PAYLD_SEL_fld));
    pr_info("  IPV4_CHKSUM_CHK_EN: %d\n", F64G(ISEC_PP_CTRLreg, IPV4_CHKSUM_CHK_EN_fld));
    pr_info("  SECTAG_AFTER_UDP_HDR_EN: %d\n", F64G(ISEC_PP_CTRLreg, SECTAG_AFTER_UDP_HDR_EN_fld));
    pr_info("  SECTAG_AFTER_TCP_HDR_EN: %d\n", F64G(ISEC_PP_CTRLreg, SECTAG_AFTER_TCP_HDR_EN_fld));
    pr_info("  SECTAG_AFTER_IPV6_HDR_EN: %d\n", F64G(ISEC_PP_CTRLreg, SECTAG_AFTER_IPV6_HDR_EN_fld));
    pr_info("  SECTAG_AFTER_IPV4_HDR_EN: %d\n", F64G(ISEC_PP_CTRLreg, SECTAG_AFTER_IPV4_HDR_EN_fld));
    pr_info("  PTP_MATCH_RULE_EN: %d\n", F64G(ISEC_PP_CTRLreg, PTP_MATCH_RULE_EN_fld));
    pr_info("  PTP_DEST_PORT_EN: %d\n", F64G(ISEC_PP_CTRLreg, PTP_DEST_PORT_EN_fld));
    pr_info("  TCP_PROTO_EN: %d\n", F64G(ISEC_PP_CTRLreg, TCP_PROTO_EN_fld));
    pr_info("  UDP_PROTO_EN: %d\n", F64G(ISEC_PP_CTRLreg, UDP_PROTO_EN_fld));
    pr_info("  PE_ETYPE_EN: %d\n", F64G(ISEC_PP_CTRLreg, PE_ETYPE_EN_fld));
    pr_info("  NIV_ETYPE_EN: %d\n", F64G(ISEC_PP_CTRLreg, NIV_ETYPE_EN_fld));
    pr_info("  PTP_ETYPE_EN: %d\n", F64G(ISEC_PP_CTRLreg, PTP_ETYPE_EN_fld));
    pr_info("  IPV6_ETYPE_EN: %d\n", F64G(ISEC_PP_CTRLreg, IPV6_ETYPE_EN_fld));
    pr_info("  IPV4_ETYPE_EN: %d\n", F64G(ISEC_PP_CTRLreg, IPV4_ETYPE_EN_fld));
    pr_info("  PBB_EN: %d\n", F64G(ISEC_PP_CTRLreg, PBB_EN_fld));
    pr_info("  MPLS_ETYPE_EN: %d\n", F64G(ISEC_PP_CTRLreg, MPLS_ETYPE_EN_fld));
    pr_info("  TPID_EN: %d\n", F64G(ISEC_PP_CTRLreg, TPID_EN_fld));
    pr_info("  SECTAG_VLD_RULE_EN: %d\n", F64G(ISEC_PP_CTRLreg, SECTAG_VLD_RULE_EN_fld));
    pr_info("  SECTAG_V: %d\n", F64G(ISEC_PP_CTRLreg, SECTAG_V_fld));
    pr_info("  SECTAG_ETYPE_SEL: %d\n", F64G(ISEC_PP_CTRLreg, SECTAG_ETYPE_SEL_fld));
    pr_info("Rudimentary Management packet detection");
    soc_ubus_reg32_get(mdev->macsec_unit, ISEC_RUD_MGMT_RULE_CTRLreg, mdev->macsec_port, &rval);
    pr_info("  RULE_EN: %x\n", F32G(ISEC_RUD_MGMT_RULE_CTRLreg, RULE_EN_fld));
    pr_info("  RULE_SP_NUM: %d\n", F32G(ISEC_RUD_MGMT_RULE_CTRLreg, RULE_SP_NUM_fld));
    pr_info("ICV Fail counter");
    soc_ubus_reg32_get(mdev->macsec_unit, ISEC_AES_ICV_FAIL_CNTreg, mdev->macsec_port, &rval);
    pr_info("  ICV_FAIL_CNT: %d\n", rval);
    pr_info("MTU Fail counter");
    soc_ubus_reg32_get(mdev->macsec_unit, ISEC_MTU_FAIL_CNTreg, mdev->macsec_port, &rval);
    pr_info("  MTU_FAIL_CNT: %d\n", rval);
    pr_info("Parser-data FIFO STATUS");
    soc_ubus_reg32_get(mdev->macsec_unit, ISEC_PDF_STATUSreg, mdev->macsec_port, &rval);
    pr_info("  CELL_CNT: %d\n",                         F32G(ISEC_PDF_STATUSreg, ISEC_CELL_CNT_fld));
    pr_info("  EMPTY: %d\n",                            F32G(ISEC_PDF_STATUSreg, EMPTY_fld));
    pr_info("  OVERFLOW: %d\n",                         F32G(ISEC_PDF_STATUSreg, OVERFLOW_fld));
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ISEC_PDF_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ISEC_PDF_STATUSreg, SINGLE_BIT_ERR_fld));
    pr_info("Parser-control FIFO BANK0 STATUS");
    soc_ubus_reg32_get(mdev->macsec_unit, ISEC_PCF_BANK0_STATUSreg, mdev->macsec_port, &rval);
    pr_info("  CELL_CNT: %d\n",                         F32G(ISEC_PCF_BANK0_STATUSreg, ISEC_CELL_CNT_fld));
    pr_info("  EMPTY: %d\n",                            F32G(ISEC_PCF_BANK0_STATUSreg, EMPTY_fld));
    pr_info("  OVERFLOW: %d\n",                         F32G(ISEC_PCF_BANK0_STATUSreg, OVERFLOW_fld));
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ISEC_PCF_BANK0_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ISEC_PCF_BANK0_STATUSreg, SINGLE_BIT_ERR_fld));
    pr_info("Parser-control FIFO BANK1 STATUS");
    soc_ubus_reg32_get(mdev->macsec_unit, ISEC_PCF_BANK1_STATUSreg, mdev->macsec_port, &rval);
    pr_info("  CELL_CNT: %d\n",                         F32G(ISEC_PCF_BANK0_STATUSreg, ISEC_CELL_CNT_fld));
    pr_info("  EMPTY: %d\n",                            F32G(ISEC_PCF_BANK0_STATUSreg, EMPTY_fld));
    pr_info("  OVERFLOW: %d\n",                         F32G(ISEC_PCF_BANK0_STATUSreg, OVERFLOW_fld));
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ISEC_PCF_BANK0_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ISEC_PCF_BANK0_STATUSreg, SINGLE_BIT_ERR_fld));
    pr_info("Control Bus Storage FIFO STATUS");
    soc_ubus_reg32_get(mdev->macsec_unit, ISEC_CB_STR_FIFO_STATUSreg, mdev->macsec_port, &rval);
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ISEC_CB_STR_FIFO_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ISEC_CB_STR_FIFO_STATUSreg, SINGLE_BIT_ERR_fld));
    pr_info("Input-data FIFO STATUS");
    soc_ubus_reg32_get(mdev->macsec_unit, ISEC_IDF_STATUSreg, mdev->macsec_port, &rval);
    pr_info("  CELL_CNT: %d\n",                         F32G(ISEC_IDF_STATUSreg, ISEC_CELL_CNT_fld));
    pr_info("  EMPTY: %d\n",                            F32G(ISEC_IDF_STATUSreg, EMPTY_fld));
    pr_info("  OVERFLOW: %d\n",                         F32G(ISEC_IDF_STATUSreg, OVERFLOW_fld));
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ISEC_IDF_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ISEC_IDF_STATUSreg, SINGLE_BIT_ERR_fld));
    pr_info("Input-control FIFO STATUS");
    soc_ubus_reg32_get(mdev->macsec_unit, ISEC_ICF_STATUSreg, mdev->macsec_port, &rval);
    pr_info("  CELL_CNT: %d\n",                         F32G(ISEC_ICF_STATUSreg, ISEC_CELL_CNT_fld));
    pr_info("  EMPTY: %d\n",                            F32G(ISEC_ICF_STATUSreg, EMPTY_fld));
    pr_info("  OVERFLOW: %d\n",                         F32G(ISEC_ICF_STATUSreg, OVERFLOW_fld));
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ISEC_ICF_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ISEC_ICF_STATUSreg, SINGLE_BIT_ERR_fld));
    pr_info("Output-data FIFO STATUS");
    soc_ubus_reg32_get(mdev->macsec_unit, ISEC_ODF_STATUSreg, mdev->macsec_port, &rval);
    pr_info("  CELL_CNT: %d\n",                         F32G(ISEC_ODF_STATUSreg, ISEC_CELL_CNT_fld));
    pr_info("  EMPTY: %d\n",                            F32G(ISEC_ODF_STATUSreg, EMPTY_fld));
    pr_info("  OVERFLOW: %d\n",                         F32G(ISEC_ODF_STATUSreg, OVERFLOW_fld));
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ISEC_ODF_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ISEC_ODF_STATUSreg, SINGLE_BIT_ERR_fld));
    pr_info("Tag FIFO STATUS");
    soc_ubus_reg32_get(mdev->macsec_unit, ISEC_TAG_FIFO_STATUSreg, mdev->macsec_port, &rval);
    pr_info("  EMPTY: %d\n",                            F32G(ISEC_TAG_FIFO_STATUSreg, EMPTY_fld));
    pr_info("  OVERFLOW: %d\n",                         F32G(ISEC_TAG_FIFO_STATUSreg, OVERFLOW_fld));
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ISEC_TAG_FIFO_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ISEC_TAG_FIFO_STATUSreg, SINGLE_BIT_ERR_fld));
    pr_info("CW FIFO STATUS");
    soc_ubus_reg32_get(mdev->macsec_unit, ISEC_CW_FIFO_STATUSreg, mdev->macsec_port, &rval);
    pr_info("  CREDIT_RESIDUE: %d\n",                   F32G(ISEC_CW_FIFO_STATUSreg, CREDIT_RESIDUE_fld));
    pr_info("  CELL_CNT: %d\n",                         F32G(ISEC_CW_FIFO_STATUSreg, CELL_CNT_fld));
    pr_info("  CELL_REQ_CNT: %d\n",                     F32G(ISEC_CW_FIFO_STATUSreg, CELL_REQ_CNT_fld));
    pr_info("  EMPTY: %d\n",                            F32G(ISEC_CW_FIFO_STATUSreg, EMPTY_fld));
    pr_info("  OVERFLOW: %d\n",                         F32G(ISEC_CW_FIFO_STATUSreg, OVERFLOW_fld));
    pr_info("  DOUBLE_BIT_ERR: %d\n",                   F32G(ISEC_CW_FIFO_STATUSreg, DOUBLE_BIT_ERR_fld));
    pr_info("  SINGLE_BIT_ERR: %d\n",                   F32G(ISEC_CW_FIFO_STATUSreg, SINGLE_BIT_ERR_fld));

    return 0;
}

static int macsec_proc_cmd_stats(int argc, char *argv[])
{
    int port, i;
    macsec_dev_t *mdev;
    uint64_t val;
    char *usage = "Usage: stats " PORT_ID;

    VALIDATE_PORT(port, argc, argv);

    mdev = &msec_devs[FL_UNIT][port];

    _xflow_macsec_counters_collect(mdev->macsec_unit);

    pr_info("Port %d stats:\n", port);
    pr_info("=== ESEC ===");

    pr_info("Uncontrolled port\n");
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_ENCRYPT, port), xflowMacsecUnctrlPortOutOctets, &val);
    pr_info("  UnctrlPortOutOctets:          %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_ENCRYPT, port), xflowMacsecUnctrlPortOutUcastPkts, &val);
    pr_info("  UnctrlPortOutUcastPkts:       %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_ENCRYPT, port), xflowMacsecUnctrlPortOutMulticastPkts, &val);
    pr_info("  UnctrlPortOutMulticastPkts:   %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_ENCRYPT, port), xflowMacsecUnctrlPortOutBroadcastPkts, &val);
    pr_info("  UnctrlPortOutBroadcastPkts:   %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_ENCRYPT, port), xflowMacsecUnctrlPortOutErrors, &val);
    pr_info("  UnctrlPortOutErrors:          %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_ENCRYPT, port), xflowMacsecOutMgmtPkts, &val);
    pr_info("  UnctrlPortOutMgmtPkts:        %llu\n", val);

    pr_info("Controlled port\n");
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_ENCRYPT, port), xflowMacsecCtrlPortOutOctets, &val);
    pr_info("  CtrlPortOutOctets:            %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_ENCRYPT, port), xflowMacsecCtrlPortOutUcastPkts, &val);
    pr_info("  CtrlPortOutUcastPkts:         %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_ENCRYPT, port), xflowMacsecCtrlPortOutMulticastPkts, &val);
    pr_info("  CtrlPortOutMulticastPkts:     %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_ENCRYPT, port), xflowMacsecCtrlPortOutBroadcastPkts, &val);
    pr_info("  CtrlPortOutBroadcastPkts:     %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_ENCRYPT, port), xflowMacsecCtrlPortOutErrors, &val);
    pr_info("  CtrlPortOutErrors:            %llu\n", val);

    pr_info("Secure Channel\n");
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_encrypt, xflowMacsecSecyTxSCStatsProtectedPkts, &val);
    pr_info("  secyTxSCStatsProtectedPkts:   %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_encrypt, xflowMacsecSecyTxSCStatsOctetsProtected, &val);
    pr_info("  secyTxSCStatsOctetsProtected: %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_encrypt, xflowMacsecSecyTxSCStatsEncryptedPkts, &val);
    pr_info("  secyTxSCStatsEncryptedPkts:   %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_encrypt, xflowMacsecSecyTxSCStatsOctetsEncrypted, &val);
    pr_info("  secyTxSCStatsOctetsEncrypted: %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_encrypt, xflowMacsecSecyStatsTxUntaggedPkts, &val);
    pr_info("  secyStatsTxUntaggedPkts:      %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_encrypt, xflowMacsecSecyStatsTxTooLongPkts, &val);
    pr_info("  secyStatsTxTooLongPkts:       %llu\n", val);

    if (mdev->assoc_id_encrypt[0])
    {
        pr_info("Secure Association 0\n");
        xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_encrypt[0], xflowMacsecSecyTxSAStatsProtectedPkts, &val);
        pr_info("  secyTxSAStatsProtectedPkts:   %llu\n", val);
        xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_encrypt[0], xflowMacsecSecyTxSAStatsEncryptedPkts, &val);
        pr_info("  secyTxSAStatsEncryptedPkts:   %llu\n", val);
    }
    if (mdev->assoc_id_encrypt[1])
    {
        pr_info("Secure Association 1\n");
        xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_encrypt[1], xflowMacsecSecyTxSAStatsProtectedPkts, &val);
        pr_info("  secyTxSAStatsProtectedPkts:   %llu\n", val);
        xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_encrypt[1], xflowMacsecSecyTxSAStatsEncryptedPkts, &val);
        pr_info("  secyTxSAStatsEncryptedPkts:   %llu\n", val);
    }
    pr_info("=== ISEC ===");

    pr_info("Uncontrolled port\n");
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecUnctrlPortInOctets, &val);
    pr_info("  UnctrlPortInOctets:           %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecUnctrlPortInUcastPkts, &val);
    pr_info("  UnctrlPortInUcastPkts:        %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecUnctrlPortInMulticastPkts, &val);
    pr_info("  UnctrlPortInMulticastPkts:    %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecUnctrlPortInBroadcastPkts, &val);
    pr_info("  UnctrlPortInBroadcastPkts:    %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecUnctrlPortInKayPkts, &val);
    pr_info("  UnctrlPortInKayPkts:          %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecInMgmtPkts, &val);
    pr_info("  UnctrlPortInMgmtPkts:         %llu\n", val);

    pr_info("Controlled port\n");
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecCtrlPortInOctets, &val);
    pr_info("  CtrlPortInOctets:             %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecCtrlPortInUcastPkts, &val);
    pr_info("  CtrlPortInUcastPkts:          %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecCtrlPortInMulticastPkts, &val);
    pr_info("  CtrlPortInMulticastPkts:      %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecCtrlPortInBroadcastPkts, &val);
    pr_info("  CtrlPortInBroadcastPkts:      %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecCtrlPortInDiscards, &val);
    pr_info("  CtrlPortInDiscards:           %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecCtrlPortInErrors, &val);
    pr_info("  CtrlPortInErrors:             %llu\n", val);
 
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecSecyStatsRxUntaggedPkts, &val);
    pr_info("  SecyStatsRxUntaggedPkts:      %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecSecyStatsRxNoTagPkts, &val);
    pr_info("  SecyStatsRxNoTagPktsCtrl:     %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecSecyStatsRxBadTagPkts, &val);
    pr_info("  SecyStatsRxBadTagPkts:        %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecSecyStatsRxUnknownSCIPkts, &val);
    pr_info("  SecyStatsRxUnknownSCIPkts:    %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecCtrlPortInDiscards, &val);
    pr_info("  SecyStatsRxNoSCIPkts:         %llu\n", val);

    pr_info("Secure Channel\n");
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsOctetsValidated, &val);
    pr_info("  SecyRxSCStatsOctetsValidated: %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsOctetsDecrypted, &val);
    pr_info("  SecyRxSCStatsOctetsDecrypted: %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsLatePkts, &val);
    pr_info("  SecyRxSCStatsLatePkts:        %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsNotValidPkts, &val);
    pr_info("  SecyRxSCStatsNotValidPkts:    %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsInvalidPkts, &val);
    pr_info("  SecyRxSCStatsInvalidPkts:     %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsDelayedPkts, &val);
    pr_info("  SecyRxSCStatsDelayedPkts:     %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsUncheckedPkts, &val);
    pr_info("  SecyRxSCStatsUncheckedPkts:   %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsOKPkts, &val);
    pr_info("  SecyRxSCStatsOKPkts:          %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsOctetsValidated, &val);
    pr_info("  SecyRxSCStatsOctetsValidated: %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsOctetsDecrypted, &val);
    pr_info("  SecyRxSCStatsOctetsDecrypted: %llu\n", val);

    for (i = 0; i < 4; i++)
    {
        if (!mdev->assoc_id_decrypt[i])
            continue;

        pr_info("Secure Association %d\n", i);
        xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_decrypt[i], xflowMacsecSecyRxSAStatsUnusedSAPkts, &val);
        pr_info("  SecyRxSAStatsUnusedSAPkts:    %llu\n", val);
        xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_decrypt[i], xflowMacsecSecyRxSAStatsNotUsingSAPkts, &val);
        pr_info("  SecyRxSAStatsNotUsingSAPkts:  %llu\n", val);
        xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_decrypt[i], xflowMacsecSecyRxSAStatsNotValidPkts, &val);
        pr_info("  SecyRxSAStatsNotValidPkts:    %llu\n", val);
        xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_decrypt[i], xflowMacsecSecyRxSAStatsInvalidPkts, &val);
        pr_info("  SecyRxSAStatsInvalidPkts:     %llu\n", val);
        xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_decrypt[i], xflowMacsecSecyRxSAStatsOKPkts, &val);
        pr_info("  SecyRxSAStatsOKPkts:          %llu\n", val);
    }

    pr_info("=== Miscellaneous ===\n");
    xflow_macsec_stat_get(mdev->macsec_unit, 0, 0, xflowMacsecBadSvtagHdrCntr, &val);
    pr_info("  BadSvtagHdrCntr:              %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecScTcamHitCntr, &val);
    pr_info("  ScTcamHitCntr:                %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecScTcamMissCntr, &val);
    pr_info("  ScTcamMissCntr:               %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, 0, xflowMacsecInPacketDropCntr, &val);
    pr_info("  InPacketDropCntr:             %llu\n", val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, 0, xflowMacsecOutPacketDropCntr, &val);
    pr_info("  OutPacketDropCntr:            %llu\n", val);

#if 0 
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecSecyStatsRxUntaggedPkts, &val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecSecyStatsRxNoTagPkts, &val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecSecyStatsRxBadTagPkts, &val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecSecyStatsRxUnknownSCIPkts, &val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecSecyStatsRxNoSCIPkts, &val);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port), xflowMacsecSecyStatsRxOverrunPkts, &val);
#endif
    return 0;
}

static int macsec_proc_cmd_xpn_enable(int argc, char *argv[])
{
    int port;
    macsec_dev_t *mdev;
    char *usage = "Usage: xpn_enable " PORT_ID " <ssci_rx> <salt_rx> <ssci_tx> <salt_tx>";

    VALIDATE_PORT(port, argc, argv);

    if (argc < 6)
        goto error;

    mdev = &msec_devs[FL_UNIT][port];
    if (kstrtou32(argv[2], 16, &mdev->ssci_decrypt))
        goto error;
    if (kstrtou32(argv[4], 16, &mdev->ssci_encrypt))
        goto error;
    if (hex2bin(mdev->salt_decrypt, argv[3], sizeof(mdev->salt_encrypt)))
        goto error;
    if (hex2bin(mdev->salt_encrypt, argv[5], sizeof(mdev->salt_encrypt)))
        goto error;
    return 0;
error:
    pr_info("%s\n", usage);
    return -1;
}

static int macsec_proc_cmd_xpn_disable(int argc, char *argv[])
{
    int port;
    macsec_dev_t *mdev;
    char *usage = "Usage: xpn_disable " PORT_ID;

    VALIDATE_PORT(port, argc, argv);

    mdev = &msec_devs[FL_UNIT][port];

    mdev->ssci_encrypt = mdev->ssci_decrypt = 0;
    memset(mdev->salt_encrypt, 0, sizeof(mdev->salt_encrypt));
    memset(mdev->salt_decrypt, 0, sizeof(mdev->salt_decrypt));

    return 0;
}

static int macsec_proc_cmd_mdev_dump(int argc, char *argv[])
{
    int port;
    macsec_dev_t *mdev;
    char *usage = "Usage: mdev_dump " PORT_ID;

    VALIDATE_PORT(port, argc, argv);

    mdev = &msec_devs[FL_UNIT][port];
    pr_info("Port %d mdev:\n", port);
    pr_info("  macsec_unit       %d\n", mdev->macsec_unit);
    pr_info("  macsec_port       %d\n", mdev->macsec_port);
    pr_info("  assoc_num         %d\n", mdev->assoc_num);
    pr_info("  include_sci       %d\n", mdev->include_sci);
    pr_info("  chan_id_encrypt   %08x\n", mdev->chan_id_encrypt);
    pr_info("  chan_id_decrypt   %08x\n", mdev->chan_id_decrypt);
    pr_info("  assoc_id_encrypt0 %08x\n", mdev->assoc_id_encrypt[0]);
    pr_info("  assoc_id_encrypt1 %08x\n", mdev->assoc_id_encrypt[1]);
    pr_info("  assoc_id_decrypt0 %08x\n", mdev->assoc_id_decrypt[0]);
    pr_info("  assoc_id_decrypt1 %08x\n", mdev->assoc_id_decrypt[1]);
    pr_info("  assoc_id_decrypt2 %08x\n", mdev->assoc_id_decrypt[2]);
    pr_info("  assoc_id_decrypt3 %08x\n", mdev->assoc_id_decrypt[3]);
    pr_info("  policy_id_encrypt %08x\n", mdev->policy_id_encrypt);
    pr_info("  policy_id_decrypt %08x\n", mdev->policy_id_decrypt);
    pr_info("  flow_id_decrypt   %08x\n", mdev->flow_id_decrypt);
    pr_info("  egress_sci        %llx\n", mdev->egress_sci);
    pr_info("  ingress_sci       %llx\n", mdev->ingress_sci);
    pr_info("  egress_sci_mask   %llx\n", mdev->egress_sci_mask);
    pr_info("  ingress_sci_mask  %llx\n", mdev->ingress_sci_mask);
    pr_info("  svtag_sig         %02x\n", mdev->svtag_sig);
    pr_info("  ssci_encrypt      %08x\n", mdev->ssci_encrypt);
    pr_info("  ssci_decrypt      %08x\n", mdev->ssci_decrypt);
    pr_info("  salt_encrypt      %08x%08x%08x\n", *((uint32_t*)&mdev->salt_encrypt[8]),
        *((uint32_t*)&mdev->salt_encrypt[4]), *((uint32_t*)&mdev->salt_encrypt[0]));
    pr_info("  salt_decrypt      %08x%08x%08x\n", *((uint32_t*)&mdev->salt_decrypt[8]),
        *((uint32_t*)&mdev->salt_decrypt[4]), *((uint32_t*)&mdev->salt_decrypt[0]));

    return 0;
}

static int macsec_proc_cmd_table_dump(int argc, char *argv[])
{
    int port, index;
    char *table;
    macsec_dev_t *mdev;
    char *usage =
        "Usage: table_dump " PORT_ID " <table> <index>\n"
        "=== Tables ===\n"
        "TDM_CALENDAR""\n"
        "TDM_2_CALENDAR""\n"
        "ESEC_SC_TABLE""\n"
        "ESEC_SA_TABLE""\n"
        "ESEC_SA_HASH_TABLE""\n"
        "SUB_PORT_MAP_TABLE""\n"
        "SUB_PORT_POLICY_TABLE""\n"
        "ISEC_SC_TABLE""\n"
        "ISEC_SA_TABLE""\n"
        "ISEC_SA_HASH_TABLE""\n"
        "ISEC_SP_TCAM_KEY""\n"
        "ISEC_SP_TCAM_MASK""\n"
        "ISEC_SC_TCAM""\n"
        "ESEC_MIB_MISC""\n"
        "ESEC_MIB_ROLLOVER_FIFO""\n"
        "ESEC_MIB_SC_UNCTRL""\n"
        "ESEC_MIB_SC_CTRL""\n"
        "ESEC_MIB_SC""\n"
        "ESEC_MIB_SA""\n"
        "ISEC_SPTCAM_HIT_COUNT""\n"
        "ISEC_SCTCAM_HIT_COUNT""\n"
        "ISEC_PORT_COUNTERS""\n"
        "ISEC_MIB_SP_UNCTRL""\n"
        "ISEC_MIB_SP_CTRL_1""\n"
        "ISEC_MIB_SP_CTRL_2""\n"
        "ISEC_MIB_SC""\n"
        "ISEC_MIB_SA""\n";

    VALIDATE_PORT(port, argc, argv);

    mdev = &msec_devs[FL_UNIT][port];
    if (argc < 4)
        goto error;
    table = argv[2];
    if (kstrtou32(argv[3], 10, &index))
        goto error;

    pr_info("Port %d table %s index %d:\n", port, table, index);
    debug_dump_table(mdev, table, index);
    return 0;
error:
    pr_info("%s", usage);
    return -1;
}

static int macsec_proc_cmd_mib_dump(int argc, char *argv[])
{
    int port, index;
    char *name;
    macsec_dev_t *mdev;
    char *usage = "Usage: mib " PORT_ID " <isec|esec> <index>\n";

    VALIDATE_PORT(port, argc, argv);

    mdev = &msec_devs[FL_UNIT][port];
    if (argc < 4)
        goto error;
    name = argv[2];
    if (kstrtou32(argv[3], 10, &index))
        goto error;

    pr_info("Port %d name %s index %d:\n", port, name, index);
    xflow_macsec_debug_mode_handler(mdev, 1, name, index);
    return 0;
error:
    pr_info("%s", usage);
    return -1;
}

static int macsec_proc_cmd_cfg_dump(int argc, char *argv[])
{
    int port, index;
    char *name;
    macsec_dev_t *mdev;
    char *usage = "Usage: cfg " PORT_ID " <isec|esec> <index>\n";

    VALIDATE_PORT(port, argc, argv);

    mdev = &msec_devs[FL_UNIT][port];
    if (argc < 4)
        goto error;
    name = argv[2];
    if (kstrtou32(argv[3], 10, &index))
        goto error;

    pr_info("Port %d name %s index %d:\n", port, name, index);
    xflow_macsec_debug_mode_handler(mdev, 2, name, index);
    return 0;
error:
    pr_info("%s", usage);
    return -1;
}

static struct proc_cmd_ops command_entries[] = {
    { .name = "status", .do_command         = macsec_proc_cmd_status },
    { .name = "stats", .do_command          = macsec_proc_cmd_stats },
    { .name = "xpn_enable", .do_command     = macsec_proc_cmd_xpn_enable },
    { .name = "xpn_disable", .do_command    = macsec_proc_cmd_xpn_disable },
    { .name = "mdev_dump", .do_command	    = macsec_proc_cmd_mdev_dump },
    { .name = "md", .do_command	            = macsec_proc_cmd_mdev_dump },
    { .name = "table_dump", .do_command	    = macsec_proc_cmd_table_dump },
    { .name = "td", .do_command	            = macsec_proc_cmd_table_dump },
    { .name = "mib", .do_command	        = macsec_proc_cmd_mib_dump },
    { .name = "cfg", .do_command            = macsec_proc_cmd_cfg_dump },
};

static struct proc_cmd_table macsec_command_table = {
    .module_name = "MACSEC_FL",
    .size = ARRAY_SIZE(command_entries),
    .ops = command_entries
};

int macsec_proc_init(void)
{
    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir)
    {
        PR_ERR("failed to create PROC directory %s\n", PROC_DIR);
        return -1;
    }
    cmd_proc_file = proc_create_cmd(CMD_PROC_FILE, proc_dir, &macsec_command_table);
    if (!cmd_proc_file) 
    {
        pr_err("Failed to create %s\n", CMD_PROC_FILE);
        goto error;
    }
    return 0;
error:
    return -1;
}
