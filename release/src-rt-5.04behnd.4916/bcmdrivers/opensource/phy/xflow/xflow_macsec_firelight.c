/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2012:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 *
 ************************************************************************/

#include "macsec_dev.h"
#include "macsec_types.h"
#include "soc/drv.h"
#include "soc/mcm/enum_max.h"
#include "soc/mcm/enum_types.h"
#include "soc/mem.h"
#include "soc/mcm/memregs.h"
#include "soc/memory.h"
#include "soc/field.h"
#include <soc/mcm/allenum.h>
#include "soc/feature.h"
#include "xflow_macsec_defs.h"
#include "xflow_macsec.h"
#include "xflow_macsec_esw_defs.h"
#include "xflow_macsec_common.h"
#include "xflow_macsec_firelight.h"
#include "xflow_macsec_cfg_params.h"
#include "macsec_common.h"

#include "bchp_regs_int.h"

static int fl_macsec_lport_map[CMBB_FL_MACSEC_MAX_PORT_NUM];

/* Indexed using packet type. */
const xflow_macsec_flow_udf_map_t udf_map_firelight[] =
{
    {_xflowMacsecUdfPktEII,
        {
         {_xflowMacsecUdfOuterDstMac, 48}, {_xflowMacsecUdfOuterSrcMac, 48},
         {_xflowMacsecUdfFirstVlan, 16}, {_xflowMacsecUdfSecondVlan, 16},
         {_xflowMacsecUdfThirdVlan, 16},
         {_xflowMacsecUdfFourthVlan, 16}, {_xflowMacsecUdfPayload, 256}
        }
    },
    {_xflowMacsecUdfPktMpls3,
        {
         {_xflowMacsecUdfOuterDstMac, 48}, {_xflowMacsecUdfOuterSrcMac, 48},
         {_xflowMacsecUdfFirstMplsSbit, 24},
         {_xflowMacsecUdfSecondMplsSbit, 24},
         {_xflowMacsecUdfThirdMplsSbit, 24},
         {_xflowMacsecUdfFourthMplsSbit, 24}, {_xflowMacsecUdfPayload, 224}
        }
    },
    {_xflowMacsecUdfPktPbb,
        {
         {_xflowMacsecUdfOuterDstMac, 48}, {_xflowMacsecUdfOuterSrcMac, 48},
         {_xflowMacsecUdfBbtagVidPCP, 16}, {_xflowMacsecUdfItagPcpIsid, 32},
         {_xflowMacsecUdfInnerDstMac, 48}, {_xflowMacsecUdfInnerSrcMac, 48},
         {_xflowMacsecUdfInnerFirstVlan, 16},
         {_xflowMacsecUdfInnerSecondVlan, 16},
         {_xflowMacsecUdfPayload, 144}
        }
    },
    {_xflowMacsecUdfPktVntag,
        {
         {_xflowMacsecUdfOuterDstMac, 48}, {_xflowMacsecUdfOuterSrcMac, 48},
         {_xflowMacsecUdfPayload, 320},
        }
    },
    {_xflowMacsecUdfPktEtag,
        {
         {_xflowMacsecUdfOuterDstMac, 48}, {_xflowMacsecUdfOuterSrcMac, 48},
         {_xflowMacsecUdfFirstVlan, 16}, {_xflowMacsecUdfSecondVlan, 16},
         {_xflowMacsecUdfEtagTci, 48},
         {_xflowMacsecUdfPayload, 240},
        }
    },
    {_xflowMacsecUdfPktIPv4,
        {
         {_xflowMacsecUdfOuterDstMac, 48}, {_xflowMacsecUdfOuterSrcMac, 48},
         {_xflowMacsecUdfFirstVlan, 16}, {_xflowMacsecUdfSecondVlan, 16},
         {_xflowMacsecUdfDipAddr, 32}, {_xflowMacsecUdfSipAddr, 32},
         {_xflowMacsecUdfProtocolId, 8},
         {_xflowMacsecUdfPayload, 240},
        }
    },
    {_xflowMacsecUdfPktL4IPv4,
        {
         {_xflowMacsecUdfOuterDstMac, 48}, {_xflowMacsecUdfOuterSrcMac, 48},
         {_xflowMacsecUdfFirstVlan, 16}, {_xflowMacsecUdfSecondVlan, 16},
         {_xflowMacsecUdfDipAddr, 32}, {_xflowMacsecUdfSipAddr, 32},
         {_xflowMacsecUdfProtocolId, 8},
         {_xflowMacsecUdfSourcePort, 16}, {_xflowMacsecUdfDestPort, 16},
         {_xflowMacsecUdfPayload, 184},
        }
    },
    {_xflowMacsecUdfPktL4IPv6,
        {
         {_xflowMacsecUdfOuterDstMac, 48}, {_xflowMacsecUdfFirstVlan, 16},
         {_xflowMacsecUdfSecondVlan, 16},
         {_xflowMacsecUdfDipAddr, 128}, {_xflowMacsecUdfSipAddr, 128},
         {_xflowMacsecUdfProtocolId, 8},
         {_xflowMacsecUdfSourcePort, 16}, {_xflowMacsecUdfDestPort, 16},
         {_xflowMacsecUdfPayload, 40}
        }
    },
    {_xflowMacsecUdfPktIPv6,
        {
         {_xflowMacsecUdfOuterDstMac, 48}, {_xflowMacsecUdfFirstVlan, 16},
         {_xflowMacsecUdfSecondVlan, 16},
         {_xflowMacsecUdfDipAddr, 128}, {_xflowMacsecUdfSipAddr, 128},
         {_xflowMacsecUdfProtocolId, 8},
         {_xflowMacsecUdfPayload, 72},
        }
    },
};

_xflow_macsec_stat_map_t _xflow_macsec_fl_stat_map_list[xflowMacsecStatTypeCount] =
{
    {xflowMacsecStatTypeInvalid,                    /* 0 */
                _xflowMacsecCidCount, INVALIDf,
                XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE, xflowMacsecIdTypeCount,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecUnctrlPortInOctets,                 /* 1 */
                _xflowMacsecCid0, INOCTETSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecUnctrlPortInUcastPkts,              /* 2 */
                _xflowMacsecCid0, INUCASTPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecUnctrlPortInMulticastPkts,          /* 3 */
                _xflowMacsecCid0, INMULTICASTPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecUnctrlPortInBroadcastPkts,          /* 4 */
                _xflowMacsecCid0, INBROADCASTPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecUnctrlPortInDiscards,               /* 5 */
                _xflowMacsecCidCount, INVALIDf,
                XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE, xflowMacsecIdTypeCount,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecUnctrlPortOutOctets,                /* 6 */
                _xflowMacsecCid6, OUTOCTETSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecUnctrlPortOutUcastPkts,             /* 7 */
                _xflowMacsecCid6, OUTUCASTPKTSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecUnctrlPortOutMulticastPkts,         /* 8 */
                _xflowMacsecCid6, OUTMULTICASTPKTSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecUnctrlPortOutBroadcastPkts,         /* 9 */
                _xflowMacsecCid6, OUTBROADCASTPKTSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecUnctrlPortOutErrors,                /* 10 */
                _xflowMacsecCid6, OUTERRORSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecCtrlPortInOctets,                   /* 11 */
                _xflowMacsecCid2, INOCTETSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecCtrlPortInUcastPkts,                /* 12 */
                _xflowMacsecCid2, INUCASTPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecCtrlPortInMulticastPkts,            /* 13 */
                _xflowMacsecCid2, INMULTICASTPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecCtrlPortInBroadcastPkts,            /* 14 */
                _xflowMacsecCid2, INBROADCASTPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecCtrlPortInDiscards,                 /* 15 */
                _xflowMacsecCid2, INDISCARDSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecCtrlPortInErrors,                   /* 16 */
                _xflowMacsecCid2, INERRORSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecCtrlPortOutOctets,                  /* 17 */
                _xflowMacsecCid7, OUTOCTETSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecCtrlPortOutUcastPkts,               /* 18 */
                _xflowMacsecCid7, OUTUCASTPKTSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecCtrlPortOutMulticastPkts,           /* 19 */
                _xflowMacsecCid7, OUTMULTICASTPKTSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecCtrlPortOutBroadcastPkts,           /* 20 */
                _xflowMacsecCid7, OUTBROADCASTPKTSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecCtrlPortOutErrors,                  /* 21 */
                _xflowMacsecCid7, OUTERRORSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyStatsTxUntaggedPkts,            /* 22 */
                _xflowMacsecCid8, SECYSTATSTXUNTAGGEDPKTSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyStatsTxTooLongPkts,             /* 23 */
                _xflowMacsecCid8, SECYSTATSTXTOOLONGPKTSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyStatsRxUntaggedPkts,            /* 24 */
                _xflowMacsecCid1, SECYSTATSRXUNTAGGEDPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyStatsRxNoTagPkts,               /* 25 */
                _xflowMacsecCid1, SECYSTATSRXNOTAGPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyStatsRxBadTagPkts,              /* 26 */
                _xflowMacsecCid1, SECYSTATSRXBADTAGPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyStatsRxUnknownSCIPkts,          /* 27 */
                _xflowMacsecCid1, SECYSTATSRXUNKNOWNSCIPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyStatsRxNoSCIPkts,               /* 28 */
                _xflowMacsecCid1, SECYSTATSRXNOSCIPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyStatsRxOverrunPkts,             /* 29 */
                _xflowMacsecCidSpecial, INVALIDf,
                XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyTxSCStatsProtectedPkts,         /* 30 */
                _xflowMacsecCidSpecial, SECYTXSASTATSPROTECTEDPKTSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyTxSCStatsEncryptedPkts,         /* 31 */
                _xflowMacsecCidSpecial, SECYTXSASTATSENCRYPTEDPKTSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyTxSCStatsOctetsProtected,       /* 32 */
                _xflowMacsecCid8, SECYTXSCSTATSOCTETSPROTECTEDf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyTxSCStatsOctetsEncrypted,       /* 33 */
                _xflowMacsecCid8, SECYTXSCSTATSOCTETSENCRYPTEDf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSCStatsUnusedSAPkts,          /* 34 */
                _xflowMacsecCidSpecial, SECYRXSASTATSUNUSEDSAPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSCStatsNotUsingSAPkts,        /* 35 */
                _xflowMacsecCidSpecial, SECYRXSASTATSNOTUSINGSAPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSCStatsLatePkts,              /* 36 */
                _xflowMacsecCid3, SECYRXSCSTATSLATEPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSCStatsNotValidPkts,          /* 37 */
                _xflowMacsecCidSpecial, SECYRXSASTATSNOTVALIDPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSCStatsInvalidPkts,           /* 38 */
                _xflowMacsecCidSpecial, SECYRXSASTATSINVALIDPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSCStatsDelayedPkts,           /* 39 */
                _xflowMacsecCid3, SECYRXSCSTATSDELAYEDPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSCStatsUncheckedPkts,         /* 40 */
                _xflowMacsecCid3, SECYRXSCSTATSUNCHECKEDPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSCStatsOKPkts,                /* 41 */
                _xflowMacsecCidSpecial, SECYRXSASTATSOKPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSCStatsOctetsValidated,       /* 42 */
                _xflowMacsecCid3, SECYRXSCSTATSOCTETSVALIDATEDf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSCStatsOctetsDecrypted,       /* 43 */
                _xflowMacsecCid3, SECYRXSCSTATSOCTETSDECRYPTEDf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyTxSAStatsProtectedPkts,         /* 44 */
                _xflowMacsecCid9, SECYTXSASTATSPROTECTEDPKTSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSecureAssoc,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyTxSAStatsEncryptedPkts,         /* 45 */
                _xflowMacsecCid9, SECYTXSASTATSENCRYPTEDPKTSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSecureAssoc,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSAStatsUnusedSAPkts,          /* 46 */
                _xflowMacsecCid4, SECYRXSASTATSUNUSEDSAPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureAssoc,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSAStatsNotUsingSAPkts,        /* 47 */
                _xflowMacsecCid4, SECYRXSASTATSNOTUSINGSAPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureAssoc,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSAStatsNotValidPkts,          /* 48 */
                _xflowMacsecCid4, SECYRXSASTATSNOTVALIDPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureAssoc,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSAStatsInvalidPkts,           /* 49 */
                _xflowMacsecCid4, SECYRXSASTATSINVALIDPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureAssoc,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecSecyRxSAStatsOKPkts,                /* 50 */
                _xflowMacsecCid4, SECYRXSASTATSOKPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureAssoc,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecInMgmtPkts,                         /* 51 */
                _xflowMacsecCid0, INMGMTPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecFlowTcamHitCntr,                    /* 52 */
                _xflowMacsecCid10, SPTCAM_HITf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeFlow,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecFlowTcamMissCntr,                   /* 53 */
                _xflowMacsecCid11, SPTCAM_MISSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypePort,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecScTcamHitCntr,                      /* 54 */
                _xflowMacsecCid12, SCTCAM_HITf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSecureChan,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecScTcamMissCntr,                     /* 55 */
                _xflowMacsecCid11, SCTCAM_MISSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypePort,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecOutMgmtPkts,                        /* 56 */
                _xflowMacsecCid6, OUTMGMTPKTSf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecInPacketDropCntr,                   /* 57 */
                _xflowMacsecCid11, PKTDROPf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypePort,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecOutPacketDropCntr,                  /* 58 */
                _xflowMacsecCid5, PKTDROPf,
                XFLOW_MACSEC_ENCRYPT, xflowMacsecIdTypePort,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecBadOlpHdrCntr,                      /* 59 */
                _xflowMacsecCidCount, INVALIDf,
                XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE, xflowMacsecIdTypeInvalid,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecBadSvtagHdrCntr,                    /* 60 */
                _xflowMacsecCid5, BADCUSTOMHDRf,
                XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE, xflowMacsecIdTypePort,
                _xflowMacsecCidInvalid, 0, NULL},
    {xflowMacsecUnctrlPortInKayPkts,                /* 61 */
                _xflowMacsecCid0, INKAYPKTSf,
                XFLOW_MACSEC_DECRYPT, xflowMacsecIdTypeSubportNum,
                _xflowMacsecCidInvalid, 0, NULL}
};

/*
 * Function:
 *      _xflow_macsec_firelight_counters_cleanup
 * Description:
 *      Cleanup and free the counter map.
 * Parameters:
 *      unit  - (IN) BCM unit number
 * Return Value:
 *      BCM_E_NONE
 */
int _xflow_macsec_firelight_counters_cleanup(int unit)
{
    int i;
    _xflow_macsec_counter_regs_t *counter;
    xflow_macsec_db_t *db;

    BCM_IF_ERROR_RETURN(xflow_macsec_db_get(unit, &db));

    for (i = 0; i < _xflowMacsecCidCount; i++)
    {
        counter = &(db->_xflow_macsec_counter_array[i]);
        if ((counter->mem != INVALIDm) || (counter->reg != INVALIDr))
        {
            soc_cm_sfree(unit, counter->buf);
            counter->buf = NULL;
        }
    }

    for (i = 1; i < xflowMacsecStatTypeCount; i++)
    {
        sal_free(db->_xflow_macsec_stat_map_list[i].sw_value);
        db->_xflow_macsec_stat_map_list[i].sw_value = NULL;
    }

    sal_free(db->_xflow_macsec_counter_array);
    db->_xflow_macsec_counter_array = NULL;

    return BCM_E_NONE;
}

/*
 * Function:
 *      xflow_macsec_firelight_control_set
 * Description:
 *      Configure the control parameters
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      flags - (IN) Encrypt/Decrypt
 *      type - (IN) Control parameter
 *      value - (OUT) Configured value
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 */
int xflow_macsec_firelight_control_set(int unit,
                                       uint32 flags,
                                       xflow_macsec_control_t type,
                                       uint64 value)
{
    uint32 rval;
    uint64 rval64;
    soc_ubus_reg_t reg = INVALIDreg;
    soc_ubus_field_t fld = INVALID_fld;
    uint32 val, fwidth, val_max;
    int oper = XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE;

    if (flags & XFLOW_MACSEC_ENCRYPT)
    {
        oper = XFLOW_MACSEC_ENCRYPT;
    }
    else if (flags & XFLOW_MACSEC_DECRYPT)
    {
        oper = XFLOW_MACSEC_DECRYPT;
    }

    switch (type)
    {
        case xflowMacsecControlPNThreshold:
            if (oper == XFLOW_MACSEC_ENCRYPT)
            {
                reg = ESEC_PN_THDreg;
                fld = PN_EXPIRE_THD_fld;
            }
            else if (oper == XFLOW_MACSEC_DECRYPT)
            {
                reg = ISEC_PN_EXPIRE_THDreg;
                fld = PN_EXPIRE_THD_fld;
            }
            break;
        case xflowMacsecControlXPNThreshold:
            if (oper == XFLOW_MACSEC_ENCRYPT)
            {
                reg = ESEC_XPN_THDreg;
                fld = XPN_EXPIRE_THD_fld;
            }
            else if (oper == XFLOW_MACSEC_DECRYPT)
            {
                reg = ISEC_XPN_EXPIRE_THDreg;
                fld = XPN_EXPIRE_THD_fld;
            }
            break;
        case xflowMacsecControlMgmtMTU:
            if (oper == XFLOW_MACSEC_ENCRYPT)
            {
                reg = ESEC_EGRESS_MTU_FOR_MGMT_PKTreg;
                fld = MGMT_MTU_fld;
            }
            else if (oper == XFLOW_MACSEC_DECRYPT)
            {
                return BCM_E_PARAM;
            }
            break;
        case xflowMacsecControlVxLANSecDestPort:
            if (oper == XFLOW_MACSEC_ENCRYPT)
            {
                reg = ESEC_VXLANSEC_DEST_PORT_NOreg;
                fld = DEST_PORT_NO_fld;
            }
            else if (oper == XFLOW_MACSEC_DECRYPT)
            {
                reg = ISEC_VXLANSEC_DEST_PORT_NOreg;
                fld = DEST_PORT_NO_fld;
            }
            break;
        case xflowMacsecControlSVTagTPIDEtype:
            if (oper == XFLOW_MACSEC_ENCRYPT)
            {
                reg = ESEC_SVTAG_ETYPEreg;
                fld = ESETYPE_fld;
            }
            else if (oper == XFLOW_MACSEC_DECRYPT)
            {
                reg = ISEC_SVTAG_CTRLreg;
                fld = TPID_fld;
            }
            break;
        case xflowMacsecControlSVTagEnable:
            if (oper == XFLOW_MACSEC_ENCRYPT)
            {
                reg = ESEC_SVTAG_ETYPEreg;
                fld = ESETYPE_MASK_fld;
                if (value)
                {
                    value = 0xff;
                }
            }
            else
            {
                return BCM_E_PARAM;
            }
        default:
            break;
    }

    if (reg == INVALIDreg)
    {
        if (oper == XFLOW_MACSEC_ENCRYPT)
        {
            return BCM_E_PARAM;
        }
        switch (type)
        {
            case xflowMacsecControlPTPDestPortGeneral:
                reg = ISEC_PTP_DEST_PORT_NOreg;
                fld = DEST_PORT_NO_1_fld;
                break;
            case xflowMacsecControlPTPDestPortEvent:
                reg = ISEC_PTP_DEST_PORT_NOreg;
                fld = DEST_PORT_NO_2_fld;
                break;
            case xflowMacsecControlPbbTpidBTag:
                reg = ISEC_PBB_TPIDreg;
                fld = B_TPID_fld;
                break;
            case xflowMacsecControlPbbTpidITag:
                reg = ISEC_PBB_TPIDreg;
                fld = I_TPID_fld;
                break;
            case xflowMacsecControlEtypeNIV:
                reg = ISEC_NIV_ETYPEreg;
                fld = NIV_ETYPE_fld;
                break;
            case xflowMacsecControlEtypePE:
                reg = ISEC_PE_ETYPEreg;
                fld = PE_ETYPE_fld;
                break;
            case xflowMacsecControlEtypeMgmt0:
                reg = ISEC_MGMTRULE_CFG_ETYPE_0reg;
                fld = CFG0_ETYPE_0_fld;
                break;
            case xflowMacsecControlEtypeMgmt1:
                reg = ISEC_MGMTRULE_CFG_ETYPE_0reg;
                fld = CFG0_ETYPE_1_fld;
                break;
            case xflowMacsecControlEtypeMgmt2:
                reg = ISEC_MGMTRULE_CFG_ETYPE_0reg;
                fld = CFG0_ETYPE_2_fld;
                break;
            case xflowMacsecControlEtypeMgmt3:
                reg = ISEC_MGMTRULE_CFG_ETYPE_0reg;
                fld = CFG0_ETYPE_3_fld;
                break;
            case xflowMacsecControlEtypeMgmt4:
                reg = ISEC_MGMTRULE_CFG_ETYPE_1reg;
                fld = CFG1_ETYPE_0_fld;
                break;
            case xflowMacsecControlEtypeMgmt5:
                reg = ISEC_MGMTRULE_CFG_ETYPE_1reg;
                fld = CFG1_ETYPE_1_fld;
                break;
            case xflowMacsecControlEtypeMgmt6:
                reg = ISEC_MGMTRULE_CFG_ETYPE_1reg;
                fld = CFG1_ETYPE_2_fld;
                break;
            case xflowMacsecControlEtypeMgmt7:
                reg = ISEC_MGMTRULE_CFG_ETYPE_1reg;
                fld = CFG1_ETYPE_3_fld;
                break;
            case xflowMacsecControlOutDestPort:
                reg = ISEC_OUT_DESTPORT_NOreg;
                fld = DESTPORT_NO_fld;
                break;
            case xflowMacsecControlMplsEtype0:
                reg = ISEC_MPLS_ETYPEreg;
                fld = MPLS_ETYPE0_fld;
                break;
            case xflowMacsecControlMplsEtype1:
                reg = ISEC_MPLS_ETYPEreg;
                fld = MPLS_ETYPE1_fld;
                break;
            case xflowMacsecControlMplsEtype2:
                reg = ISEC_MPLS_ETYPEreg;
                fld = MPLS_ETYPE2_fld;
                break;
            case xflowMacsecControlMplsEtype3:
                reg = ISEC_MPLS_ETYPEreg;
                fld = MPLS_ETYPE3_fld;
                break;
            default:
                return BCM_E_PARAM;
        }
    }

    if ((reg != INVALIDreg) && (fld != INVALID_fld))
    {
        val = (uint32)value;
        fwidth = soc_ubus_reg_field_length(unit, reg, fld);

        if (fwidth <= 32)
        {
            if (fwidth < 32)
            {
                val_max = (1 << fwidth) - 1;
            }
            else
            {
                val_max = 0xffffffff;
            }
            if (val > val_max)
            {
                return BCM_E_PARAM;
            }
        }

        if (SOC_UBUS_REG_IS_64(unit, reg))
        {
            BCM_IF_ERROR_RETURN(soc_ubus_reg_get(unit, reg, REG_PORT_ANY, &rval64));

            soc_ubus_reg64_field_set(unit, reg, &rval64, fld, value);

            BCM_IF_ERROR_RETURN(soc_ubus_reg_set(unit, reg, REG_PORT_ANY, rval64));
        }
        else
        {
            BCM_IF_ERROR_RETURN(soc_ubus_reg32_get(unit, reg, REG_PORT_ANY, &rval));

            soc_ubus_reg_field_set(unit, reg, &rval, fld, val);

            BCM_IF_ERROR_RETURN(soc_ubus_reg32_set(unit, reg, REG_PORT_ANY, rval));
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _xflow_macsec_fl_sc_get_encrypt
 * Description:
 *      Get the encrypt secure channel info
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      sc_index - (IN) SC physical index
 *      chan - (OUT) SC info
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 */
int _xflow_macsec_fl_sc_get_encrypt(int unit,
                                    int sc_index,
                                    xflow_macsec_secure_chan_info_t *chan)
{
    esec_sc_table_entry_t sc_entry;
    uint64 sci;
    int crypto = 0;
    uint32 rval;

    if ((sc_index < 0) || (sc_index > soc_mem_index_max(unit, ESEC_SC_TABLEm)))
        return BCM_E_PARAM;

    sal_memset(chan, 0, sizeof(xflow_macsec_secure_chan_info_t));
    chan->tci &= 0x3;
    BCM_IF_ERROR_RETURN(READ_ESEC_SC_TABLEm(unit, MEM_BLOCK_ALL, sc_index, &sc_entry));

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, TCI_SCf);
    if (rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_INFO_INCLUDE_SCI;
        chan->tci |= (1 << 5);
    }

    soc_mem_field64_get(unit, ESEC_SC_TABLEm, (uint32 *)&sc_entry, SCIf, &sci);
    memcpy(&chan->sci, &sci, 8);

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, TCI_ESf);
    if (rval)
    {
        chan->tci |= (1 << 6);
    }

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, TCI_SCBf);
    if (rval)
    {
        chan->tci |= (1 << 4);
    }

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, TCI_Cf);
    if (rval)
    {
        chan->tci |= (1 << 2);
    }

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, TCI_Ef);
    if (rval)
    {
        chan->tci |= (1 << 3);
    }
    chan->active_an = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, ANf);

    chan->tci |= (chan->active_an & 0x3);

    chan->vxlansec_hdr_update = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, VXLANSEC_L3L4_HDR_UPDATEf);

    chan->vxlansec_pkt_type = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, VXLANSEC_ENC_PKT_TYPEf);

    switch (chan->vxlansec_pkt_type)
    {
        case xflowMacsecSecureChanReserved0:
        case xflowMacsecSecureChanReserved1:
        case xflowMacsecSecureChanReserved2:
        case xflowMacsecSecureChanReserved3:
        case xflowMacsecSecureChanReserved4:
        case xflowMacsecSecureChanReserved5:
        case xflowMacsecSecureChanReserved6:
        case xflowMacsecSecureChanReserved7:
        case xflowMacsecSecureChanReserved8:
        case xflowMacsecSecureChanReserved9:
        case xflowMacsecSecureChanReserved10:
        case xflowMacsecSecureChanReserved11:
        case xflowMacsecSecureChanReserved12:
            return BCM_E_INTERNAL;
        default:
            break;
    }

    chan->an_control = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, ANCONTROLf);
    chan->sectag_etype_sel = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, SECTAG_ETYPE_SELf);
    chan->mtu = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, MTUf);

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, CIPHERSUITEf);
    switch (rval)
    {
        case 0:
            crypto = xflowMacsecCryptoAes128Gcm;
            break;
        case 1:
            crypto = xflowMacsecCryptoAes256Gcm;
            break;
        case 2:
            crypto = xflowMacsecCryptoAes128GcmXpn;
            break;
        case 3:
            crypto = xflowMacsecCryptoAes256GcmXpn;
            break;
        default:
            return BCM_E_PARAM;
    }
    chan->crypto = crypto;

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, IEEE_STD_802PT1AE_AUTHENTICATIONDISABLEf);
    if (rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_RANGE_AUTHENTICATE;
    }

    chan->first_auth_range_offset_start = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, NONIEEEAUTHSTARTf);
    chan->first_auth_range_offset_end = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, NONIEEEAUTHENDf);
    chan->first_auth_range_offset_end -= chan->first_auth_range_offset_start;

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, ADDOFFSET2VXLANSECOFFSETSf);
    if (rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_VXLANSEC;
    }

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, DROP_OR_ZERO_OUT_SA_INVALID_PKTSf);
    if (rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_ZERO_OUT_SA_INVALID_PKT;
    }

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, ADDOFFSET2NONIEEEAUTHSTARTf);
    if (rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_RANGE_START;
    }

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, ADDOFFSET2NONIEEEAUTHENDf);
    if (rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_RANGE_END;
    }

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, ADDOFFSET2SECTAGLOCATIONf);
    if (rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_SECTAG;
    }

    chan->confidentiality_offset = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, CONFIDENTIALITYOFFSETf);
    chan->sectag_offset = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, SECTAGLOCATIONf);

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, PROTECTFRAMESf);
    if (!rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_PROTECT_DISABLE;
    }

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, SECUREDDATAPKTENABLEDf);
    if (rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_CONTROLLED_SECURED_DATA;
    }

    rval = soc_mem_field32_get(unit, ESEC_SC_TABLEm, &sc_entry, UNSECUREDDATAPKTENABLEDf);
    if (rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_CONTROLLED_UNSECURED_DATA;
    }

    if ((chan->flags & XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_CONTROLLED_SECURED_DATA) &&
        (chan->flags & XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_CONTROLLED_UNSECURED_DATA))
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_INFO_CONTROLLED_PORT;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _xflow_macsec_fl_sc_get_decrypt
 * Description:
 *      Get the decrypt secure channel info
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      sc_index - (IN) SC physical index
 *      chan - (OUT) SC info
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 */
int _xflow_macsec_fl_sc_get_decrypt(int unit,
                                    int sc_index,
                                    xflow_macsec_secure_chan_info_t *chan)
{
    isec_sc_table_entry_t sc_entry;
    isec_sc_tcam_entry_t sc_tcam_entry;
    uint64 sci;
    int policy_index;
    uint32 rval;

    if ((sc_index < 0) || (sc_index > soc_mem_index_max(unit, ISEC_SC_TABLEm)))
        return BCM_E_PARAM;

    sal_memset(chan, 0, sizeof(xflow_macsec_secure_chan_info_t));
    sal_memset(&sc_entry, 0, sizeof(isec_sc_table_entry_t));
    sal_memset(&sc_tcam_entry, 0, sizeof(isec_sc_tcam_entry_t));

    BCM_IF_ERROR_RETURN(READ_ISEC_SC_TABLEm(unit, MEM_BLOCK_ALL, sc_index, &sc_entry));
    BCM_IF_ERROR_RETURN(READ_ISEC_SC_TCAMm(unit, MEM_BLOCK_ALL, sc_index, &sc_tcam_entry));

    chan->vxlansec_hdr_update = soc_mem_field32_get(unit, ISEC_SC_TABLEm, &sc_entry, VXLANSEC_L3L4_HDR_UPDATEf);
    rval = soc_mem_field32_get(unit, ISEC_SC_TABLEm, &sc_entry, REPLAYPROTECTf);

    if (rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_INFO_REPLAY_PROTECT_ENABLE;
        chan->replay_protect_window_size = soc_mem_field32_get(unit, ISEC_SC_TABLEm, &sc_entry, REPLAYWINDOWf);
    }
    rval = soc_mem_field32_get(unit, ISEC_SC_TABLEm, &sc_entry, CIPHERSUITEf);

    switch (rval)
    {
        case 0:
            chan->crypto = xflowMacsecCryptoAes128Gcm;
            break;
        case 1:
            chan->crypto = xflowMacsecCryptoAes256Gcm;
            break;
        case 2:
            chan->crypto = xflowMacsecCryptoAes128GcmXpn;
            break;
        case 3:
            chan->crypto = xflowMacsecCryptoAes256GcmXpn;
            break;
        default:
            return BCM_E_PARAM;
    }

    rval = soc_mem_field32_get(unit, ISEC_SC_TABLEm, &sc_entry, IEEE_STD_802PT1AE_AUTHENTICATIONDISABLEf);
    if (rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_RANGE_AUTHENTICATE;
    }

    rval = soc_mem_field32_get(unit, ISEC_SC_TABLEm, &sc_entry, ADDOFFSET2NONIEEEAUTHSTARTf);
    if (rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_DECRYPT_ADJUST_RANGE_START;
    }

    rval = soc_mem_field32_get(unit, ISEC_SC_TABLEm, &sc_entry, ADDOFFSET2NONIEEEAUTHENDf);
    if (rval)
    {
        chan->flags |= XFLOW_MACSEC_SECURE_CHAN_DECRYPT_ADJUST_RANGE_END;
    }

    chan->first_auth_range_offset_start = soc_mem_field32_get(unit, ISEC_SC_TABLEm, &sc_entry, NONIEEEAUTHSTARTf);
    chan->first_auth_range_offset_end = soc_mem_field32_get(unit, ISEC_SC_TABLEm, &sc_entry, NONIEEEAUTHENDf);
    chan->first_auth_range_offset_end -= chan->first_auth_range_offset_start;
    chan->confidentiality_offset = soc_mem_field32_get(unit, ISEC_SC_TABLEm, &sc_entry, CONFIDENTIALITYOFFSETf);

    soc_mem_field64_get(unit, ISEC_SC_TCAMm, (uint32 *)&sc_tcam_entry, SCIf, &sci);
    if (!COMPILER_64_IS_ZERO(sci))
    {
        memcpy(&chan->sci, &sci, 8);
    }

    soc_mem_field64_get(unit, ISEC_SC_TCAMm, (uint32 *)&sc_tcam_entry, SCI_MASKf, &sci);
    if (!COMPILER_64_IS_ZERO(sci))
    {
        memcpy(&chan->sci_mask, &sci, 8);
    }

    if (soc_mem_field32_get(unit, ISEC_SC_TCAMm, &sc_tcam_entry, MS_SUBPORT_NUM_MASKf) != 0)
    {
        policy_index = soc_mem_field32_get(unit, ISEC_SC_TCAMm, &sc_tcam_entry, MS_SUBPORT_NUMf);
        chan->policy_id = XFLOW_MACSEC_POLICY_ID_CREATE(XFLOW_MACSEC_DECRYPT, policy_index);
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      xflow_macsec_sp_tcam_special_write
 * Description:
 *      Get the UDF packet type based on the Frame type.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      flow_info - (IN) Decrypt flow configuration
 *      udf_type - (IN) UDF packet type
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_NOT_FOUND
 */
static int _xflow_macsec_sp_tcam_special_write(int unit,
                                               int sp_tcam_index,
                                               uint32 *sp_entry,
                                               uint32* sp_entry_mask)
{
    static soc_mem_t bulk_mem[2] = {ISEC_SP_TCAM_KEYm, ISEC_SP_TCAM_MASKm};

    int copyno[2] = {MEM_BLOCK_ALL, MEM_BLOCK_ALL};
    int tcam_index[2] = {sp_tcam_index, sp_tcam_index};
    uint32 *entry_data[2];
    int rv = BCM_E_NONE;

    if (!sp_entry || !sp_entry_mask)
        return BCM_E_PARAM;

    entry_data[0] = sp_entry;
    entry_data[1] = sp_entry_mask;

    rv = soc_mem_bulk_write(unit, bulk_mem, tcam_index, copyno, entry_data, 2);

    return rv;
}

/*
 * Function:
 *      xflow_macsec_firelight_flow_enable_set
 * Description:
 *      Enable or disable a decrypt flow.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      sp_tcam_index - (IN) Subport TCAM index
 *      enable - (IN) Enable
 * Return Value:
 *      BCM_E_NONE
 */
int xflow_macsec_firelight_flow_enable_set(int unit,
                                           int sp_tcam_index,
                                           int enable)
{
    uint32 *sp_entry = NULL;
    uint32 *sp_entry_mask = NULL;
    int rv = BCM_E_NONE, alloc_size;

    if ((sp_tcam_index < 0) || (sp_tcam_index > soc_mem_index_max(unit, ISEC_SP_TCAM_KEYm)))
        return BCM_E_PARAM;

    alloc_size = SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32);
    sp_entry = sal_alloc(alloc_size, "xflow_macsec sp_entry");
    sp_entry_mask = sal_alloc(alloc_size, "xflow_macsec sp_entry_mask");

    if (!sp_entry || !sp_entry_mask)
    {
        rv = BCM_E_MEMORY;
        goto exit;
    }

    sal_memset(sp_entry, 0, alloc_size);
    sal_memset(sp_entry_mask, 0, alloc_size);

    rv = READ_ISEC_SP_TCAM_KEYm(unit, MEM_BLOCK_ALL, sp_tcam_index, sp_entry);
    if (BCM_FAILURE(rv))
        goto exit;

    rv = READ_ISEC_SP_TCAM_MASKm(unit, MEM_BLOCK_ALL, sp_tcam_index, sp_entry_mask);
    if (BCM_FAILURE(rv))
        goto exit;

    soc_mem_field32_set(unit, ISEC_SP_TCAM_MASKm, sp_entry_mask, VALIDf, enable);
    rv = _xflow_macsec_sp_tcam_special_write(unit, sp_tcam_index, sp_entry, sp_entry_mask);

exit:
    if (sp_entry)
        sal_free(sp_entry);

    if (sp_entry_mask)
        sal_free(sp_entry_mask);

    BCM_IF_ERROR_RETURN(rv);

    return BCM_E_NONE;
}

/*
 * Function:
 *      xflow_macsec_firelight_flow_enable_get
 * Description:
 *      Check if the decrypt flow is enabled.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      sp_tcam_index - (IN) Subport TCAM index
 *      enable - (OUT) Enable/Disable
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 *      BCM_E_MEMORY
 */
int xflow_macsec_firelight_flow_enable_get(int unit,
                                           int sp_tcam_index,
                                           int *enable)
{
    uint32 *sp_entry_mask = NULL;
    int rv = BCM_E_NONE, alloc_size;

    if ((sp_tcam_index < 0) || (sp_tcam_index > soc_mem_index_max(unit, ISEC_SP_TCAM_MASKm)))
        return BCM_E_PARAM;

    alloc_size = SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32);
    sp_entry_mask = sal_alloc(alloc_size, "xflow_macsec sp_entry_mask");

    if (!sp_entry_mask)
    {
        rv = BCM_E_MEMORY;
        goto exit;
    }
    sal_memset(sp_entry_mask, 0, alloc_size);

    rv = READ_ISEC_SP_TCAM_MASKm(unit, MEM_BLOCK_ALL, sp_tcam_index, sp_entry_mask);
    if (BCM_FAILURE(rv))
    {
        goto exit;
    }
    *enable = soc_mem_field32_get(unit, ISEC_SP_TCAM_MASKm, sp_entry_mask, VALIDf);

exit:

    if (sp_entry_mask)
        sal_free(sp_entry_mask);

    BCM_IF_ERROR_RETURN(rv);

    return BCM_E_NONE;
}

/*
 * Function:
 *      xflow_macsec_fl_flow_destroy
 * Description:
 *      Delete a decrypt flow entry.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      sp_tcam_index - (IN) Subport TCAM index
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_MEMORY
 */
int xflow_macsec_fl_flow_destroy (int unit,
                                  int sp_tcam_index)
{
    int rv = BCM_E_NONE, alloc_size;
    uint32 *sp_entry = NULL;
    uint32 *sp_entry_mask = NULL;

    alloc_size = SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32);
    sp_entry = sal_alloc(alloc_size, "xflow_macsec sp_entry");
    sp_entry_mask = sal_alloc(alloc_size, "xflow_macsec sp_entry_mask");

    if (!sp_entry || !sp_entry_mask)
    {
        rv = BCM_E_MEMORY;
        goto exit;
    }
    sal_memset(sp_entry, 0, alloc_size);
    sal_memset(sp_entry_mask, 0, alloc_size);

    rv = _xflow_macsec_sp_tcam_special_write(unit, sp_tcam_index, sp_entry, sp_entry_mask);

exit:

    if (sp_entry)
        sal_free(sp_entry);

    if (sp_entry_mask)
        sal_free(sp_entry_mask);

    return rv;
}

/*
 * Function:
 *      xflow_macsec_fl_sp_tcam_move_single_entry
 * Description:
 *      Move an SP TCAM entry.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      to_index - (IN) Target subport TCAM index
 *      from_index - (IN) Source subport TCAM index
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_MEMORY
 */
int xflow_macsec_fl_sp_tcam_move_single_entry (int unit,
                                               int to_index,
                                               int from_index)
{
    int rv = BCM_E_NONE, alloc_size;
    uint32 *sp_entry = NULL;
    uint32 *sp_entry_mask = NULL;

    alloc_size = SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32);
    sp_entry = sal_alloc(alloc_size, "xflow_macsec sp_entry");
    sp_entry_mask = sal_alloc(alloc_size, "xflow_macsec sp_entry_mask");

    if (!sp_entry || !sp_entry_mask)
    {
        rv = BCM_E_MEMORY;
        goto exit;
    }
    sal_memset(sp_entry, 0, alloc_size);
    sal_memset(sp_entry_mask, 0, alloc_size);

    /*
     * Read the TCAM entries from from_index.
     */
    rv = READ_ISEC_SP_TCAM_KEYm(unit, MEM_BLOCK_ALL, from_index, sp_entry);
    if (BCM_FAILURE(rv))
        goto exit;

    rv = READ_ISEC_SP_TCAM_MASKm(unit, MEM_BLOCK_ALL, from_index, sp_entry_mask);
    if (BCM_FAILURE(rv))
        goto exit;

    /*
     * Write the TCAM entries to to_index using bulk write.
     */
    rv = _xflow_macsec_sp_tcam_special_write(unit, to_index, sp_entry, sp_entry_mask);
    if (BCM_FAILURE(rv))
        goto exit;

    /*
     * Read the SP Map table entries from from_index.
     */
    rv = soc_mem_read(unit, SUB_PORT_MAP_TABLEm, MEM_BLOCK_ALL, from_index, sp_entry);
    if (BCM_FAILURE(rv))
        goto exit;

    /*
     * Write the SP Map table entries to to_index.
     */
    rv = soc_mem_write(unit, SUB_PORT_MAP_TABLEm, MEM_BLOCK_ALL, to_index, sp_entry);
    if (BCM_FAILURE(rv)) {
        goto exit;
    }

    /*
     * Clear entries in from_index from both SP TCAM and Map table.
     */
    rv = xflow_macsec_fl_flow_destroy(unit, from_index);
    if (BCM_FAILURE(rv))
        goto exit;

    sal_memset(sp_entry, 0, alloc_size);
    rv = soc_mem_write(unit, SUB_PORT_MAP_TABLEm, MEM_BLOCK_ALL, from_index, sp_entry);
    if (BCM_FAILURE(rv))
        goto exit;

exit:

    if (sp_entry)
        sal_free(sp_entry);

    if (sp_entry_mask)
        sal_free(sp_entry_mask);

    return rv;
}

/*
 * Function:
 *      xflow_macsec_firelight_sc_get
 * Description:
 *      Get the configured secure channel info.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      oper - (IN) Encrypt/Decrypt
 *      sc_hw_index - (IN) SC physical index
 *      sc_info - (OUT) Configured SC info
 * Return Value:
 *      BCM_E_NONE
 */
int xflow_macsec_firelight_sc_get(int unit,
                                  uint32 oper,
                                  int sc_hw_index,
                                  xflow_macsec_secure_chan_info_t *sc_info)
{
    if (sc_info == NULL)
        return BCM_E_PARAM;

    if (oper == XFLOW_MACSEC_ENCRYPT)
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_fl_sc_get_encrypt(unit, sc_hw_index, sc_info));
    }
    else if (oper == XFLOW_MACSEC_DECRYPT)
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_fl_sc_get_decrypt(unit, sc_hw_index, sc_info));
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      xflow_macsec_firelight_port_control_get
 * Description:
 *      Get the per port control parameters.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      flags - (IN) Encrypt/Decrypt
 *      port - (IN) Logical port
 *      type - (IN) Control paramter type
 *      value - (OUT) Configured value
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 */
int xflow_macsec_firelight_port_control_get(int unit,
                                            uint32 flags,
                                            bcm_port_t port,
                                            xflow_macsec_port_control_t type,
                                            uint64 *value)
{
    uint64 reg64;
    soc_ubus_reg_t reg = ISEC_PP_CTRLreg;
    soc_ubus_field_t fld;
    int oper = XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE;

    switch (type)
    {
        case xflowMacsecPortSectagEtypeSel:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_ETYPE_SEL_fld;
            break;
        case xflowMacsecPortSectagVersion:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_V_fld;
            break;
        case xflowMacsecPortSectagRuleEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_VLD_RULE_EN_fld;
            break;
        case xflowMacsecPortTPIDEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = TPID_EN_fld;
            break;
        case xflowMacsecPortPBBEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = PBB_EN_fld;
            break;
        case xflowMacsecPortMPLSEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = MPLS_ETYPE_EN_fld;
            break;
        case xflowMacsecPortIPv4EtypeEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = IPV4_ETYPE_EN_fld;
            break;
        case xflowMacsecPortIPv6EtypeEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = IPV6_ETYPE_EN_fld;
            break;
        case xflowMacsecPortPTPEtypeEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = PTP_ETYPE_EN_fld;
            break;
        case xflowMacsecPortNIVEtypeEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = NIV_ETYPE_EN_fld;
            break;
        case xflowMacsecPortPEEtypeEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = PE_ETYPE_EN_fld;
            break;
        case xflowMacsecPortUDPEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = UDP_PROTO_EN_fld;
            break;
        case xflowMacsecPortTCPEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = TCP_PROTO_EN_fld;
            break;
        case xflowMacsecPortPTPDestPortEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = PTP_DEST_PORT_EN_fld;
            break;
        case xflowMacsecPortPTPMatchRuleEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = PTP_MATCH_RULE_EN_fld;
            break;
        case xflowMacsecPortSectagAfterIPv4Enable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_AFTER_IPV4_HDR_EN_fld;
            break;
        case xflowMacsecPortSectagAfterIPv6Enable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_AFTER_IPV6_HDR_EN_fld;
            break;
        case xflowMacsecPortSectagAfterTCPEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_AFTER_TCP_HDR_EN_fld;
            break;
        case xflowMacsecPortSectagAfterUDPEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_AFTER_UDP_HDR_EN_fld;
            break;
        case xflowMacsecPortIPv4ChecksumEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = IPV4_CHKSUM_CHK_EN_fld;
            break;
        case xflowMacsecPortVxLANIpv6UDPVNIMatchEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = VXLAN_IPV6_W_UDP_SPTCAM_UDF_PAYLD_SEL_fld;
            break;
        case xflowMacsecPortMTU:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = MTU_fld;
            break;
        case xflowMacsecPortMgmtPktRulesEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_RUD_MGMT_RULE_CTRLreg;
            fld = RULE_EN_fld;
            break;
        case xflowMacsecPortMgmtDefaultSubPort:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_RUD_MGMT_RULE_CTRLreg;
            fld = RULE_SP_NUM_fld;
            break;
        default:
            return BCM_E_PARAM;
    }

    if ((oper != XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE) && !(flags & oper))
        return BCM_E_PARAM;

    BCM_IF_ERROR_RETURN(soc_ubus_reg_get(unit, reg, port, &reg64));

    *value = soc_ubus_reg64_field_get(unit, reg, reg64, fld);

    return BCM_E_NONE;
}

/*
 * Function:
 *      xflow_macsec_firelight_port_control_set
 * Description:
 *      Configure the per port control parameters.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      flags - (IN) Encrypt/Decrypt
 *      port - (IN) Logical port
 *      type - (IN) Control paramter type
 *      value - (IN) Configured value
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 */
int xflow_macsec_firelight_port_control_set(int unit,
                                            uint32 flags,
                                            bcm_port_t port,
                                            xflow_macsec_port_control_t type,
                                            uint64 value)
{
    uint64 rval64;
    soc_ubus_reg_t reg = ISEC_PP_CTRLreg;
    soc_ubus_field_t fld = INVALIDf;
    int oper = XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE;
    uint32 rval, val, fwidth, val_max;
    val = (uint32)value;

    switch (type)
    {
        case xflowMacsecPortSectagEtypeSel:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_ETYPE_SEL_fld;
            break;
        case xflowMacsecPortSectagVersion:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_V_fld;
            break;
        case xflowMacsecPortSectagRuleEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_VLD_RULE_EN_fld;
            break;
        case xflowMacsecPortTPIDEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = TPID_EN_fld;
            break;
        case xflowMacsecPortPBBEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = PBB_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortMPLSEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = MPLS_ETYPE_EN_fld;
            break;
        case xflowMacsecPortIPv4EtypeEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = IPV4_ETYPE_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortIPv6EtypeEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = IPV6_ETYPE_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortPTPEtypeEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = PTP_ETYPE_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortNIVEtypeEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = NIV_ETYPE_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortPEEtypeEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = PE_ETYPE_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortUDPEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = UDP_PROTO_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortTCPEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = TCP_PROTO_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortPTPDestPortEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = PTP_DEST_PORT_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortPTPMatchRuleEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = PTP_MATCH_RULE_EN_fld;
            break;
        case xflowMacsecPortSectagAfterIPv4Enable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_AFTER_IPV4_HDR_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortSectagAfterIPv6Enable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_AFTER_IPV6_HDR_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortSectagAfterTCPEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_AFTER_TCP_HDR_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortSectagAfterUDPEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = SECTAG_AFTER_UDP_HDR_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortIPv4ChecksumEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = IPV4_CHKSUM_CHK_EN_fld;
            val = !!val;
            break;
        case xflowMacsecPortVxLANIpv6UDPVNIMatchEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = VXLAN_IPV6_W_UDP_SPTCAM_UDF_PAYLD_SEL_fld;
            val = !!val;
            break;
        case xflowMacsecPortMTU:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_PP_CTRLreg;
            fld = MTU_fld;
            break;
        case xflowMacsecPortMgmtPktRulesEnable:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_RUD_MGMT_RULE_CTRLreg;
            fld = RULE_EN_fld;
            break;
        case xflowMacsecPortMgmtDefaultSubPort:
            oper = XFLOW_MACSEC_DECRYPT;
            reg = ISEC_RUD_MGMT_RULE_CTRLreg;
            fld = RULE_SP_NUM_fld;
            break;
        default:
            return BCM_E_PARAM;
    }

    if ((oper != XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE) && !(flags & oper))
    {
        PR_ERR("xflow_macsec_firelight_port_control_set: ERROR 1");
        return BCM_E_PARAM;
    }
    
    if (fld != INVALIDf)
    {
        fwidth = soc_ubus_reg_field_length(unit, reg, fld);

        if (fwidth < 32)
        {
            val_max = (1 << fwidth) - 1;
        }
        else
        {
            val_max = 0xffffffff;
        }

        if (val > val_max)
        {
            PR_ERR("xflow_macsec_firelight_port_control_set: ERROR 2");
            return BCM_E_PARAM;
        }

        if (SOC_UBUS_REG_IS_64(unit, reg))
        {
            BCM_IF_ERROR_RETURN(soc_ubus_reg_get(unit, reg, port, &rval64));

            soc_ubus_reg64_field32_set(unit, reg, &rval64, fld, val);

            BCM_IF_ERROR_RETURN(soc_ubus_reg_set(unit, reg, port, rval64));
        }
        else
        {
            BCM_IF_ERROR_RETURN(soc_ubus_reg32_get(unit, reg, port, &rval));

            soc_ubus_reg_field_set(unit, reg, &rval, fld, val);

            BCM_IF_ERROR_RETURN(soc_ubus_reg32_set(unit, reg, port, rval));
        }
    }
    else 
        PR_ERR("xflow_macsec_firelight_port_control_set: ERROR 3");

    return BCM_E_NONE;
}
/*
 * Function:
 *      xflow_macsec_firelight_control_get
 * Description:
 *      Get the configured control parameters.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      flags - (IN) Encrypt/Decrypt
 *      type - (IN) Control parameter
 *      value - (OUT) Configured value
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 */
int xflow_macsec_firelight_control_get(int unit,
                                       uint32 flags,
                                       xflow_macsec_control_t type,
                                       uint64 *value)
{
    uint32 rval;
    uint64 rval64;
    soc_ubus_reg_t reg = INVALIDreg;
    soc_ubus_field_t fld = INVALID_fld;
    uint32 val_lo = 0xffffffff;
    uint32 val_hi = 0xffffffff;
    int oper = XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE;

    if (flags & XFLOW_MACSEC_ENCRYPT)
    {
        oper = XFLOW_MACSEC_ENCRYPT;
    }
    else if (flags & XFLOW_MACSEC_DECRYPT)
    {
        oper = XFLOW_MACSEC_DECRYPT;
    }

    switch (type)
    {
        case xflowMacsecControlPNThreshold:
            if (oper == XFLOW_MACSEC_ENCRYPT)
            {
                reg = ESEC_PN_THDreg;
                fld = PN_EXPIRE_THD_fld;
            }
            else if (oper == XFLOW_MACSEC_DECRYPT)
            {
                reg = ISEC_PN_EXPIRE_THDreg;
                fld = IS_PN_EXPIRE_THD_fld;
            }
            break;
        case xflowMacsecControlXPNThreshold:
            if (oper == XFLOW_MACSEC_ENCRYPT)
            {
                reg = ESEC_XPN_THDreg;
                fld = XPN_EXPIRE_THD_fld;
            }
            else if (oper == XFLOW_MACSEC_DECRYPT)
            {
                reg = ISEC_XPN_EXPIRE_THDreg;
                fld = IS_XPN_EXPIRE_THD_fld;
            }
            break;
        case xflowMacsecControlMgmtMTU:
            if (oper == XFLOW_MACSEC_ENCRYPT)
            {
                reg = ESEC_EGRESS_MTU_FOR_MGMT_PKTreg;
                fld = MGMT_MTU_fld;
            }
            else if (oper == XFLOW_MACSEC_DECRYPT)
            {
                return BCM_E_PARAM;
            }
            break;
        case xflowMacsecControlVxLANSecDestPort:
            if (oper == XFLOW_MACSEC_ENCRYPT)
            {
                reg = ESEC_VXLANSEC_DEST_PORT_NOreg;
                fld = DEST_PORT_NO_fld;
            }
            else if (oper == XFLOW_MACSEC_DECRYPT)
            {
                reg = ISEC_VXLANSEC_DEST_PORT_NOreg;
                fld = IS_DEST_PORT_NO_fld;
            }
            break;
        case xflowMacsecControlSVTagTPIDEtype:
            if (oper == XFLOW_MACSEC_ENCRYPT)
            {
                reg = ESEC_SVTAG_ETYPEreg;
                fld = ESETYPE_fld;
            }
            else if (oper == XFLOW_MACSEC_DECRYPT)
            {
                reg = ISEC_SVTAG_CTRLreg;
                fld = TPID_fld;
            }
            break;
        case xflowMacsecControlSVTagEnable:
            if (oper == XFLOW_MACSEC_ENCRYPT)
            {
                reg = ESEC_SVTAG_ETYPEreg;
                fld = ESETYPE_MASK_fld;
                val_lo = 0x1;
                val_hi = 0;
            }
            else if (oper == XFLOW_MACSEC_DECRYPT)
            {
                return BCM_E_PARAM;
            }
            break;
        default:
            break;
    }

    if (reg == INVALIDreg)
    {
        if (oper == XFLOW_MACSEC_ENCRYPT)
            return BCM_E_PARAM;

        switch (type)
        {
            case xflowMacsecControlPTPDestPortGeneral:
                reg = ISEC_PTP_DEST_PORT_NOreg;
                fld = DEST_PORT_NO_1_fld;
                break;
            case xflowMacsecControlPTPDestPortEvent:
                reg = ISEC_PTP_DEST_PORT_NOreg;
                fld = DEST_PORT_NO_2_fld;
                break;
            case xflowMacsecControlPbbTpidBTag:
                reg = ISEC_PBB_TPIDreg;
                fld = B_TPID_fld;
                break;
            case xflowMacsecControlPbbTpidITag:
                reg = ISEC_PBB_TPIDreg;
                fld = I_TPID_fld;
                break;
            case xflowMacsecControlEtypeNIV:
                reg = ISEC_NIV_ETYPEreg;
                fld = ETYPE_fld;
                break;
            case xflowMacsecControlEtypePE:
                reg = ISEC_PE_ETYPEreg;
                fld = NIV_ETYPE_fld;
                break;
            case xflowMacsecControlEtypeMgmt0:
                reg = ISEC_MGMTRULE_CFG_ETYPE_0reg;
                fld = CFG0_ETYPE_0_fld;
                break;
            case xflowMacsecControlEtypeMgmt1:
                reg = ISEC_MGMTRULE_CFG_ETYPE_0reg;
                fld = CFG0_ETYPE_1_fld;
                break;
            case xflowMacsecControlEtypeMgmt2:
                reg = ISEC_MGMTRULE_CFG_ETYPE_0reg;
                fld = CFG0_ETYPE_2_fld;
                break;
            case xflowMacsecControlEtypeMgmt3:
                reg = ISEC_MGMTRULE_CFG_ETYPE_0reg;
                fld = CFG0_ETYPE_3_fld;
                break;
            case xflowMacsecControlEtypeMgmt4:
                reg = ISEC_MGMTRULE_CFG_ETYPE_1reg;
                fld = CFG1_ETYPE_0_fld;
                break;
            case xflowMacsecControlEtypeMgmt5:
                reg = ISEC_MGMTRULE_CFG_ETYPE_1reg;
                fld = CFG1_ETYPE_1_fld;
                break;
            case xflowMacsecControlEtypeMgmt6:
                reg = ISEC_MGMTRULE_CFG_ETYPE_1reg;
                fld = CFG1_ETYPE_2_fld;
                break;
            case xflowMacsecControlEtypeMgmt7:
                reg = ISEC_MGMTRULE_CFG_ETYPE_1reg;
                fld = CFG1_ETYPE_3_fld;
                break;
            case xflowMacsecControlOutDestPort:
                reg = ISEC_OUT_DESTPORT_NOreg;
                fld = DESTPORT_NO_fld;
                break;
            case xflowMacsecControlMplsEtype0:
                reg = ISEC_MPLS_ETYPEreg;
                fld = MPLS_ETYPE0_fld;
                break;
            case xflowMacsecControlMplsEtype1:
                reg = ISEC_MPLS_ETYPEreg;
                fld = MPLS_ETYPE1_fld;
                break;
            case xflowMacsecControlMplsEtype2:
                reg = ISEC_MPLS_ETYPEreg;
                fld = MPLS_ETYPE2_fld;
                break;
            case xflowMacsecControlMplsEtype3:
                reg = ISEC_MPLS_ETYPEreg;
                fld = MPLS_ETYPE3_fld;
                break;
            default:
                return BCM_E_PARAM;
        }
    }

    if ((reg != INVALIDreg) && (fld != INVALID_fld))
    {
        if (SOC_UBUS_REG_IS_64(unit, reg))
        {
            BCM_IF_ERROR_RETURN(soc_ubus_reg_get(unit, reg, REG_PORT_ANY, &rval64));

            val_hi &= COMPILER_64_HI(soc_ubus_reg64_field_get(unit, reg, rval64, fld));
            val_lo &= COMPILER_64_LO(soc_ubus_reg64_field_get(unit, reg, rval64, fld));
        }
        else
        {
            val_hi = 0;
            BCM_IF_ERROR_RETURN(soc_ubus_reg32_get(unit, reg, REG_PORT_ANY, &rval));

            val_lo &= soc_ubus_reg_field_get(unit, reg, rval, fld);
        }

        COMPILER_64_SET(*value, val_hi, val_lo);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _xflow_macsec_fl_sc_set_decrypt
 * Description:
 *      Set the decrypt secure channel info
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      sc_index - (IN) SC physical index
 *      chan - (IN) SC info
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 */
int _xflow_macsec_fl_sc_set_decrypt(int unit,
                                    int sc_index,
                                    xflow_macsec_secure_chan_info_t *chan)
{
    isec_sc_table_entry_t sc_entry;
    isec_sc_tcam_entry_t sc_tcam_entry;
    uint64 sci;
    int crypto = -1;
    int policy_index;
    uint32 mask[SOC_MAX_MEM_FIELD_WORDS];
    uint32 fval0 = 0, fval1 = 0;

    if ((sc_index < 0) || (sc_index > soc_mem_index_max(unit, ISEC_SC_TABLEm)))
        return BCM_E_PARAM;

    _BCM_FIELD32_LEN_CHECK(unit, ISEC_SC_TABLEm, VXLANSEC_L3L4_HDR_UPDATEf, chan->vxlansec_hdr_update);
    _BCM_FIELD32_LEN_CHECK(unit, ISEC_SC_TABLEm, NONIEEEAUTHSTARTf, chan->first_auth_range_offset_start);

    _BCM_FIELD32_LEN_CHECK(unit, ISEC_SC_TABLEm, NONIEEEAUTHENDf,
                           (chan->first_auth_range_offset_start + chan->first_auth_range_offset_end));

    _BCM_FIELD32_LEN_CHECK(unit, ISEC_SC_TABLEm, CONFIDENTIALITYOFFSETf, chan->confidentiality_offset);
    _BCM_FIELD32_LEN_CHECK(unit, ISEC_SC_TABLEm, REPLAYWINDOWf, chan->replay_protect_window_size);

    sal_memset(&sc_entry, 0, sizeof(isec_sc_table_entry_t));
    sal_memset(&sc_tcam_entry, 0, sizeof(isec_sc_tcam_entry_t));
    sal_memset(&mask, 0, sizeof(mask));

    soc_mem_field32_set(unit, ISEC_SC_TABLEm, &sc_entry, VXLANSEC_L3L4_HDR_UPDATEf, chan->vxlansec_hdr_update);

    if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_INFO_REPLAY_PROTECT_ENABLE)
    {
        soc_mem_field32_set(unit, ISEC_SC_TABLEm, &sc_entry, REPLAYPROTECTf, 1);
        soc_mem_field32_set(unit, ISEC_SC_TABLEm, &sc_entry, REPLAYWINDOWf, chan->replay_protect_window_size);
    }
    if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_DECRYPT_ADJUST_RANGE_START)
    {
        soc_mem_field32_set(unit, ISEC_SC_TABLEm, &sc_entry, ADDOFFSET2NONIEEEAUTHSTARTf, 1);
    }

    if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_DECRYPT_ADJUST_RANGE_END)
    {
        soc_mem_field32_set(unit, ISEC_SC_TABLEm, &sc_entry, ADDOFFSET2NONIEEEAUTHENDf, 1);
    }
    soc_mem_field32_set(unit, ISEC_SC_TABLEm, &sc_entry, SECTAG_ICV_MODEf, 3);

    switch (chan->crypto)
    {
        case xflowMacsecCryptoAes128Gcm:
            crypto = 0;
            break;
        case xflowMacsecCryptoAes256Gcm:
            crypto = 1;
            break;
        case xflowMacsecCryptoAes128GcmXpn:
            crypto = 2;
            break;
        case xflowMacsecCryptoAes256GcmXpn:
            crypto = 3;
            break;
        default:
            return BCM_E_PARAM;
    }

    soc_mem_field32_set(unit, ISEC_SC_TABLEm, &sc_entry, CIPHERSUITEf, crypto);

    if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_RANGE_AUTHENTICATE)
    {
        soc_mem_field32_set(unit, ISEC_SC_TABLEm, &sc_entry, IEEE_STD_802PT1AE_AUTHENTICATIONDISABLEf, 1);
        /*
         * first_auth_range_offset_start == 0 and
         * first_auth_range_offset_end == 0 excludes all the bytes before
         * SecTAG from the authentication process.
         */
        if ((chan->first_auth_range_offset_end == 0) && (chan->first_auth_range_offset_start != 0))
        {
            return BCM_E_PARAM;
        }
        fval0 = chan->first_auth_range_offset_start;
        fval1 = chan->first_auth_range_offset_start + chan->first_auth_range_offset_end;

        soc_mem_field32_set(unit, ISEC_SC_TABLEm, &sc_entry, NONIEEEAUTHSTARTf, fval0);

        soc_mem_field32_set(unit, ISEC_SC_TABLEm, &sc_entry, NONIEEEAUTHENDf, fval1);
    }
    soc_mem_field32_set(unit, ISEC_SC_TABLEm, &sc_entry, CONFIDENTIALITYOFFSETf, chan->confidentiality_offset);
    BCM_IF_ERROR_RETURN(WRITE_ISEC_SC_TABLEm(unit, MEM_BLOCK_ALL, sc_index, &sc_entry));
    BCM_IF_ERROR_RETURN(READ_ISEC_SC_TCAMm(unit, MEM_BLOCK_ALL, sc_index, &sc_tcam_entry));

    memcpy(&sci, &chan->sci, 8);
    soc_mem_field64_set(unit, ISEC_SC_TCAMm, &sc_tcam_entry, SCIf, sci);

    memcpy(&sci, &chan->sci_mask, 8);
    soc_mem_field64_set(unit, ISEC_SC_TCAMm, &sc_tcam_entry, SCI_MASKf, sci);

    if (chan->policy_id)
    {
        policy_index = XFLOW_MACSEC_POLICY_ID_INDEX_GET(chan->policy_id);
        BCM_IF_ERROR_RETURN(policy_index);

        BCM_IF_ERROR_RETURN(_xflow_macsec_policy_index_hw_index_get(unit, policy_index, &policy_index, NULL));

        soc_mem_field32_set(unit, ISEC_SC_TCAMm, &sc_tcam_entry, MS_SUBPORT_NUMf, policy_index);
        soc_mem_field32_set(unit, ISEC_SC_TCAMm, &sc_tcam_entry, MS_SUBPORT_NUM_MASKf, 0xff);
    }

    BCM_IF_ERROR_RETURN(WRITE_ISEC_SC_TCAMm(unit, MEM_BLOCK_ALL, sc_index, &sc_tcam_entry));
    return BCM_E_NONE;
}


/*
 * Function:
 *      _xflow_macsec_fl_sc_set_encrypt
 * Description:
 *      Set the encrypt secure channel info
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      sc_index - (IN) SC physical index
 *      chan - (IN) SC info
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 */
int _xflow_macsec_fl_sc_set_encrypt(int unit,
                                    int sc_index,
                                    xflow_macsec_secure_chan_info_t *chan)
{
    esec_sc_table_entry_t sc_entry;
    uint64 sci;
    int crypto = -1;
    int sectag_size = 8;
    uint32 fval0 = 0, fval1 = 0;

    if (chan->active_an > 1)
        return BCM_E_PARAM;
    _BCM_FIELD32_LEN_CHECK(unit, ESEC_SC_TABLEm, SECTAGLOCATIONf, chan->sectag_offset);
    _BCM_FIELD32_LEN_CHECK(unit, ESEC_SC_TABLEm, VXLANSEC_ENC_PKT_TYPEf, chan->vxlansec_pkt_type);
    _BCM_FIELD32_LEN_CHECK(unit, ESEC_SC_TABLEm, VXLANSEC_L3L4_HDR_UPDATEf, chan->vxlansec_hdr_update);
    _BCM_FIELD32_LEN_CHECK(unit, ESEC_SC_TABLEm, ANCONTROLf, chan->an_control);
    _BCM_FIELD32_LEN_CHECK(unit, ESEC_SC_TABLEm, SECTAG_ETYPE_SELf, chan->sectag_etype_sel);
    _BCM_FIELD32_LEN_CHECK(unit, ESEC_SC_TABLEm, MTUf, chan->mtu);
    _BCM_FIELD32_LEN_CHECK(unit, ESEC_SC_TABLEm, NONIEEEAUTHSTARTf, chan->first_auth_range_offset_start);

    _BCM_FIELD32_LEN_CHECK(unit, ESEC_SC_TABLEm, NONIEEEAUTHENDf,
                           (chan->first_auth_range_offset_start + chan->first_auth_range_offset_end));

    _BCM_FIELD32_LEN_CHECK(unit, ESEC_SC_TABLEm, CONFIDENTIALITYOFFSETf, chan->confidentiality_offset);

    sal_memset(&sc_entry, 0, sizeof(esec_sc_table_entry_t));

    if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_INFO_INCLUDE_SCI)
    {
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, TCI_SCf, 1);
        sectag_size = 16;
    }

    memcpy(&sci, &chan->sci, 8);
    soc_mem_field64_set(unit, ESEC_SC_TABLEm, (uint32 *)&sc_entry, SCIf, sci);

    /*
     * Sectag can start after 12 bytes and have a maximum offset
     * of 112 bytes.
     */
    if ((chan->sectag_offset < 12) || (chan->sectag_offset > 112) ||
       ((chan->sectag_offset + chan->confidentiality_offset + sectag_size) > 192))
    {
        return BCM_E_PARAM;
    }

    soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, TCI_ESf, ((chan->tci & 0x40) >> 6));
    soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, TCI_SCBf, ((chan->tci & 0x10) >> 4));
    soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, TCI_Cf, ((chan->tci & 0x04) >> 2));
    soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, TCI_Ef, ((chan->tci & 0x08) >> 3));
    soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, ANf, chan->active_an);
    soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, VXLANSEC_L3L4_HDR_UPDATEf, chan->vxlansec_hdr_update);
    soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, VXLANSEC_ENC_PKT_TYPEf, chan->vxlansec_pkt_type);
    soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, ANCONTROLf, chan->an_control);
    soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, SECTAG_ETYPE_SELf, chan->sectag_etype_sel);
    soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, MTUf, chan->mtu);

    switch (chan->crypto)
    {
        case xflowMacsecCryptoAes128Gcm:
            crypto = 0;
            break;
        case xflowMacsecCryptoAes256Gcm:
            crypto = 1;
            break;
        case xflowMacsecCryptoAes128GcmXpn:
            crypto = 2;
            break;
        case xflowMacsecCryptoAes256GcmXpn:
            crypto = 3;
            break;
        default:
            return BCM_E_PARAM;
    }

    soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, CIPHERSUITEf, crypto);
    if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_RANGE_AUTHENTICATE)
    {
        /*
         * first_auth_range_offset_start == 0 and
         * first_auth_range_offset_end == 0 excludes all the bytes before
         * SecTAG from the authentication process.
         */
        if ((chan->first_auth_range_offset_start >= chan->sectag_offset) ||
            ((chan->first_auth_range_offset_start + chan->first_auth_range_offset_end)
                                                >= chan->sectag_offset) ||
            ((chan->first_auth_range_offset_end == 0) &&
             (chan->first_auth_range_offset_start != 0)))
        {
            return BCM_E_PARAM;
        }
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, IEEE_STD_802PT1AE_AUTHENTICATIONDISABLEf, 1);
        fval0 = chan->first_auth_range_offset_start;
        fval1 = chan->first_auth_range_offset_start + chan->first_auth_range_offset_end;
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, NONIEEEAUTHSTARTf, fval0);
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, NONIEEEAUTHENDf, fval1);
    }

    if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_VXLANSEC)
    {
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, ADDOFFSET2VXLANSECOFFSETSf, 1);
    }

    if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_ZERO_OUT_SA_INVALID_PKT)
    {
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, DROP_OR_ZERO_OUT_SA_INVALID_PKTSf, 1);
    }

    if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_RANGE_START)
    {
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, ADDOFFSET2NONIEEEAUTHSTARTf, 1);
    }

    if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_RANGE_END)
    {
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, ADDOFFSET2NONIEEEAUTHENDf, 1);
    }

    if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_SECTAG)
    {
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, ADDOFFSET2SECTAGLOCATIONf, 1);
    }

    soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, CONFIDENTIALITYOFFSETf, chan->confidentiality_offset);
    soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, SECTAGLOCATIONf, chan->sectag_offset);

    if (!(chan->flags & XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_PROTECT_DISABLE))
    {
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, PROTECTFRAMESf, 1);
    }

    if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_INFO_CONTROLLED_PORT)
    {
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, SECUREDDATAPKTENABLEDf, 1);
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, FORWARDSECUREDDATAPKTf, 1);
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, UNSECUREDDATAPKTENABLEDf, 1);
        soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, FORWARDUNSECUREDDATAPKTf, 1);
    }
    else
    {
        if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_CONTROLLED_SECURED_DATA)
        {
            soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, SECUREDDATAPKTENABLEDf, 1);
            soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, FORWARDSECUREDDATAPKTf, 1);
        }
        if (chan->flags & XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_CONTROLLED_UNSECURED_DATA)
        {
            soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, UNSECUREDDATAPKTENABLEDf, 1);
            soc_mem_field32_set(unit, ESEC_SC_TABLEm, &sc_entry, FORWARDUNSECUREDDATAPKTf, 1);
        }
    }

    BCM_IF_ERROR_RETURN(WRITE_ESEC_SC_TABLEm(unit, MEM_BLOCK_ALL, sc_index, &sc_entry));
    return BCM_E_NONE;
}

/*
 * Function:
 *      xflow_macsec_firelight_policy_set
 * Description:
 *      Get the configured policy info based on
 *      decrypt policy index (subport number).
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      policy_index - (IN) Policy index
 *      policy_info - (IN) Decrypt policy info
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 */
int xflow_macsec_firelight_policy_set(int unit,
                                      int policy_index,
                                      xflow_macsec_policy_info_t *policy_info)
{
    int tag_validate = 0;
    sub_port_policy_table_entry_t policy_entry;

    sal_memset(&policy_entry, 0, sizeof(sub_port_policy_table_entry_t));

    if ((policy_index < 0) || (policy_index > soc_mem_index_max(unit, SUB_PORT_POLICY_TABLEm)))
        return BCM_E_PARAM;

    _BCM_FIELD32_LEN_CHECK(unit, SUB_PORT_POLICY_TABLEm, SECTAG_OFFSETf, policy_info->sectag_offset);
    _BCM_FIELD32_LEN_CHECK(unit, SUB_PORT_POLICY_TABLEm, MTUf, policy_info->mtu);
    _BCM_FIELD32_LEN_CHECK(unit, SUB_PORT_POLICY_TABLEm, SECTAG_ETYPE_SELf, policy_info->etype_sel);

    if (policy_info->flags & XFLOW_MACSEC_DECRYPT_POLICY_UNTAGGED_CONTROL_PORT_ENABLE)
    {
        soc_mem_field32_set(unit, SUB_PORT_POLICY_TABLEm, &policy_entry, UNTAG_CTRL_PORT_ENf, 1);
    }

    if (policy_info->flags & XFLOW_MACSEC_DECRYPT_POLICY_TAGGED_CONTROL_PORT_ENABLE)
    {
        soc_mem_field32_set(unit, SUB_PORT_POLICY_TABLEm, &policy_entry, TAG_CTRL_PORT_ENf, 1);
    }

    if (!(policy_info->flags & XFLOW_MACSEC_DECRYPT_POLICY_UNTAGGED_FRAME_DENY))
    {
        soc_mem_field32_set(unit, SUB_PORT_POLICY_TABLEm, &policy_entry, UNTAG_FORWARDFRAMESf, 1);
    }

    if (policy_info->flags & XFLOW_MACSEC_DECRYPT_POLICY_POINT_TO_POINT_ENABLE)
    {
        soc_mem_field32_set(unit, SUB_PORT_POLICY_TABLEm, &policy_entry, OPERPOINTTOPOINTMACf, 1);
        soc_mem_field64_set(unit, SUB_PORT_POLICY_TABLEm, (uint32 *)&policy_entry, SCIf, policy_info->sci);
    }

    if ((policy_info->flags & XFLOW_MACSEC_DECRYPT_POLICY_CUSTOM_PROTOCOL))
    {
        soc_mem_field32_set(unit, SUB_PORT_POLICY_TABLEm, &policy_entry, CUSTOM_PROTO_SPf, 1);
    }

    if ((policy_info->flags & XFLOW_MACSEC_DECRYPT_POLICY_SECTAG_OFFSET_ADJUST))
    {
        soc_mem_field32_set(unit, SUB_PORT_POLICY_TABLEm, &policy_entry, TAG_LABEL_ADJ_ENf, 1);
    }

    if ((policy_info->flags & XFLOW_MACSEC_DECRYPT_POLICY_INNER_L2_VALID))
    {
        soc_mem_field32_set(unit, SUB_PORT_POLICY_TABLEm, &policy_entry, INNER_L2_VALIDf, 1);
        if (policy_info->inner_l2_offset > 100)
        {
            return BCM_E_PARAM;
        }
        soc_mem_field32_set(unit, SUB_PORT_POLICY_TABLEm, &policy_entry, INNER_L2_OFFSETf,
                            policy_info->inner_l2_offset);
    }

    if ((policy_info->flags & XFLOW_MACSEC_DECRYPT_POLICY_IPV4_CHKSUM_FAIL_AND_MPLS_BOS_MISS_DENY))
    {
        soc_mem_field32_set(unit, SUB_PORT_POLICY_TABLEm, &policy_entry, TAG_LABEL_ADJ_ENf, 1);
    }

    switch (policy_info->tag_validate)
    {
        case xflowMacsecTagValidateBypassMacsec:
            tag_validate = 0;
            break;
        case xflowMacsecTagValidateStrict:
            tag_validate = 1;
            break;
        case xflowMacsecTagValidateCheckICV:
            tag_validate = 2;
            break;
        case xflowMacsecTagValidateCheckNone:
            tag_validate = 3;
            break;
        case xflowMacsecTagValidateDenyAll:
            tag_validate = 4;
            break;
        default:
            return BCM_E_PARAM;
    }
    soc_mem_field32_set(unit, SUB_PORT_POLICY_TABLEm, &policy_entry, TAG_VALIDATEFRAMESf, tag_validate);

    soc_mem_field32_set(unit, SUB_PORT_POLICY_TABLEm, &policy_entry, SECTAG_OFFSETf, policy_info->sectag_offset);

    soc_mem_field32_set(unit, SUB_PORT_POLICY_TABLEm, &policy_entry, MTUf, policy_info->mtu);

    soc_mem_field32_set(unit, SUB_PORT_POLICY_TABLEm, &policy_entry, SECTAG_ETYPE_SELf, policy_info->etype_sel);

    BCM_IF_ERROR_RETURN(WRITE_SUB_PORT_POLICY_TABLEm(unit, MEM_BLOCK_ALL, policy_index, &policy_entry));

    return BCM_E_NONE;
}

/*
 * Function:
 *      xflow_macsec_firelight_sc_set
 * Description:
 *      Get the configured secure channel info.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      oper - (IN) Encrypt/Decrypt
 *      sc_hw_index - (IN) SC physical index
 *      sc_info - (IN) Configured SC info
 * Return Value:
 *      BCM_E_NONE
 */
int xflow_macsec_firelight_sc_set(int unit,
                                  uint32 oper,
                                  int sc_hw_index,
                                  xflow_macsec_secure_chan_info_t *sc_info)
{

    if (sc_info == NULL)
        return BCM_E_PARAM;

    if (oper == XFLOW_MACSEC_ENCRYPT)
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_fl_sc_set_encrypt(unit, sc_hw_index, sc_info));
    }
    else if (oper == XFLOW_MACSEC_DECRYPT)
    {
        BCM_IF_ERROR_RETURN(_xflow_macsec_fl_sc_set_decrypt(unit, sc_hw_index, sc_info));
    }

    return BCM_E_NONE;
}
#if 0
/*
 * Function:
 *      xflow_macsec_firelight_lport_to_macsec_get
 * Description:
 *      Get the macsec port mapped to the given logical port.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      lport - (IN) Logical port
 *      macsec_port - (OUT) Macsec port
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 */
static int xflow_macsec_firelight_lport_to_macsec_get (int unit, int lport, int *macsec_port)
{
    /* For inline xflow MASEC logical and physical ports are 1 to 1 */
    *macsec_port = lport;
    return BCM_E_NONE;
}
#endif
/*
 * Function:
 *      _xflow_macsec_flow_param_to_udf_pkt_type
 * Description:
 *      Get the UDF packet type based on the Frame type.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      flow_info - (IN) Decrypt flow configuration
 *      udf_type - (IN) UDF packet type
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_NOT_FOUND
 */
static int _xflow_macsec_flow_param_to_udf_pkt_type(int unit,
                                                    xflow_macsec_flow_info_t *flow_info,
                                                    xflow_macsec_flow_udf_pkt_type_t *udf_type)
{
    switch (flow_info->frame_type)
    {
        case xflowMacsecFlowFrameEII:
        case xflowMacsecFlowFrameSnap:
        case xflowMacsecFlowFrameLlc:
            *udf_type = _xflowMacsecUdfPktEII;
            break;
        case xflowMacsecFlowFrameMpls:
            *udf_type = _xflowMacsecUdfPktMpls3;
            break;
        case xflowMacsecFlowFramePBB:
            *udf_type = _xflowMacsecUdfPktPbb;
            break;
        case xflowMacsecFlowFrameVNTag:
            *udf_type = _xflowMacsecUdfPktVntag;
            break;
        case xflowMacsecFlowFrameETag:
            *udf_type = _xflowMacsecUdfPktEtag;
            break;
        case xflowMacsecFlowFrameIPv4:
            *udf_type = _xflowMacsecUdfPktIPv4;
            break;
        case xflowMacsecFlowFrameIPv6:
            *udf_type = _xflowMacsecUdfPktIPv6;
            break;
        case xflowMacsecFlowFrameUDPIPv4:
        case xflowMacsecFlowFrameTCPIPv4:
            *udf_type = _xflowMacsecUdfPktL4IPv4;
            break;
        case xflowMacsecFlowFrameUDPIPv6:
        case xflowMacsecFlowFrameTCPIPv6:
            *udf_type = _xflowMacsecUdfPktL4IPv6;
            break;
        default:
            return BCM_E_NOT_FOUND;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      xflow_macsec_firelight_flow_set
 * Description:
 *      Set the decrypt flow configuration.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      sp_tcam_index - (IN) Subport TCAM index
 *      flow_info - (IN) Decrypt flow info
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 *      BCM_E_MEMORY
 */
int xflow_macsec_firelight_flow_set(int unit,
                                    int sp_tcam_index,
                                    xflow_macsec_flow_info_t *flow_info)
{
    xflow_macsec_flow_udf_pkt_type_t udf_pkt_type;
    uint32 f_val, f_mask;
    uint32 *sp_entry = NULL;
    uint32 *sp_entry_mask = NULL;
    uint32 *mask = NULL;
    uint32 *udf = NULL;
    uint32 *udf_mask = NULL;
    uint64 f_val_64, f_mask_64;
    xflow_macsec_udf_populate_t udf_populate;
    int rv = BCM_E_NONE, alloc_size;
//    int port_count = 0;
//    int port, macsec_port;

    alloc_size = SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32);
    sp_entry = sal_alloc(alloc_size, "xflow_macsec sp_entry");
    sp_entry_mask = sal_alloc(alloc_size, "xflow_macsec sp_entry_mask");
    mask = sal_alloc(alloc_size, "xflow_macsec flow mask");
    udf = sal_alloc(alloc_size, "xflow_macsec udf");
    udf_mask = sal_alloc(alloc_size, "xflow_macsec udf mask");

    if (!mask || !udf || !udf_mask || !sp_entry || !sp_entry_mask)
    {
        rv = BCM_E_MEMORY;
        goto exit;
    }

    sal_memset(sp_entry, 0, alloc_size);
    sal_memset(sp_entry_mask, 0, alloc_size);
    sal_memset(mask, 0, alloc_size);
    sal_memset(udf, 0, alloc_size);
    sal_memset(udf_mask, 0, alloc_size);

    if (flow_info == NULL)
    {
        rv = BCM_E_PARAM;
        goto exit;
    }

    if ((sp_tcam_index < 0) || (sp_tcam_index > soc_mem_index_max(unit, ISEC_SP_TCAM_KEYm)))
    {
        rv = BCM_E_PARAM;
        goto exit;
    }

    soc_mem_field32_set(unit, ISEC_SP_TCAM_KEYm, sp_entry, FRAME_FORMATf, flow_info->frame_type);

    if (flow_info->frame_type == xflowMacsecFlowFrameAny)
    {
        soc_mem_field32_set(unit, ISEC_SP_TCAM_MASKm, sp_entry_mask, FRAME_FORMAT_MASKf, 0);
    }
    else
    {
        soc_mem_field32_set(unit, ISEC_SP_TCAM_MASKm, sp_entry_mask, FRAME_FORMAT_MASKf, 0x1f);
    }

    f_val = 0;
    f_mask = flow_info->vlan_tag_mpls_label_flags ? 0x3f : 0;


    switch(flow_info->vlan_tag_mpls_label_flags)
    {
        case XFLOW_MACSEC_NO_TAGS_NO_LABELS:
        case XFLOW_MACSEC_1_VLAN_TAG_1_MPLS_LABEL:
        case XFLOW_MACSEC_2_VLAN_TAG_2_MPLS_LABEL:
        case XFLOW_MACSEC_3_VLAN_TAG_3_MPLS_LABEL:
        case XFLOW_MACSEC_4_VLAN_TAG_4_MPLS_LABEL:
        case XFLOW_MACSEC_GREATER_4_VLAN_TAG_5_MPLS_LABEL:
            f_val = flow_info->vlan_tag_mpls_label_flags;
            break;
    }

    /* Mask out if multiple bits set. */
    if (!f_val && flow_info->vlan_tag_mpls_label_flags)
    {
        f_mask = (0x3f ^ flow_info->vlan_tag_mpls_label_flags);
    }

    soc_mem_field32_set(unit, ISEC_SP_TCAM_KEYm, sp_entry, TAG_LABEL_STATUSf, f_val);
    soc_mem_field32_set(unit, ISEC_SP_TCAM_MASKm, sp_entry_mask, TAG_LABEL_STATUS_MASKf, f_mask);

    COMPILER_64_ZERO(f_val_64);
    COMPILER_64_ZERO(f_mask_64);
    
    /* TODO: REVISIT */
#if 0
    /* BCM_PBMP_COUNT(flow_info->src_pbm, port_count); */
    port_count = 4;

    if (port_count != 0)
    {
        COMPILER_64_SET(f_mask_64, 0xffff, 0xffffffff);
        if (port_count == 1)
        {
            /* BCM_PBMP_ITER(flow_info->src_pbm, port) */
            {
                rv = (xflow_macsec_firelight_lport_to_macsec_get(unit, port, &macsec_port));
                if (BCM_FAILURE(rv))
                {
                    goto exit;
                }
                COMPILER_64_COPY(f_val_64, (1ULL << macsec_port));
                /* break; */
            }
        }
        else
        {
            /*BCM_PBMP_ITER(flow_info->src_pbm, port)*/
            {
                rv = (xflow_macsec_firelight_lport_to_macsec_get(unit, port, &macsec_port));
                if (BCM_FAILURE(rv))
                {
                    goto exit;
                }
                COMPILER_64_XOR(f_mask_64, (1ULL << macsec_port));
            }
        }
    }
#endif

    /* TODO: REVISIT */
    f_val_64 = (1 << (sp_tcam_index ));
    f_mask_64 = (1 << (sp_tcam_index ));

    soc_mem_field64_set(unit, ISEC_SP_TCAM_KEYm, sp_entry, PORT_MAPf, f_val_64);
    soc_mem_field64_set(unit, ISEC_SP_TCAM_MASKm, sp_entry_mask, PORT_MAP_MASKf, f_mask_64);

    rv = _xflow_macsec_flow_param_to_udf_pkt_type(unit, flow_info, &udf_pkt_type);

    if (rv == BCM_E_NONE)
    {
        udf_populate.udf_num_bits = XFLOW_MACSEC_UDF_NUM_BITS;
        udf_populate.udf_map = udf_map_firelight;

        udf_populate.udf_map_count = sizeof(udf_map_firelight) / sizeof(xflow_macsec_flow_udf_map_t);
        udf_populate.udf_pkt_type = udf_pkt_type;

        rv = (xflow_macsec_flow_udf_populate(unit, &flow_info->udf, &udf_populate, udf));
        if (BCM_FAILURE(rv))
            goto exit;

        rv = (xflow_macsec_flow_udf_populate(unit, &flow_info->udf_mask, &udf_populate, udf_mask));
        if (BCM_FAILURE(rv))
            goto exit;

        soc_mem_field_set(unit, ISEC_SP_TCAM_KEYm, sp_entry, UDFf, udf);
        soc_mem_field_set(unit, ISEC_SP_TCAM_MASKm, sp_entry_mask, UDF_MASKf, udf_mask);
    }

    rv = _xflow_macsec_sp_tcam_special_write(unit, sp_tcam_index, sp_entry, sp_entry_mask);

exit:

    if (mask)
        sal_free(mask);

    if (udf)
        sal_free(udf);

    if (udf_mask)
        sal_free(udf_mask);

    if (sp_entry)
        sal_free(sp_entry);

    if (sp_entry_mask)
        sal_free(sp_entry_mask);

    BCM_IF_ERROR_RETURN(rv);

    return BCM_E_NONE;
}

/*
 * Function:
 *      xflow_macsec_firelight_flow_default_policy_reserve
 * Description:
 *      Reserve the default policy id which is assigned upon
 *      a Flow TCAM (SP TCAM) miss.
 * Parameters:
 *      unit  - (IN) BCM unit number
 * Return Value:
 *      BCM_E_NONE
 */
static int xflow_macsec_firelight_flow_default_policy_reserve(int unit)
{
    int flags;
    xflow_macsec_db_t *macsec_db;
    xflow_macsec_policy_info_t policy_info;
    xflow_macsec_policy_id_t policy_id;

    BCM_IF_ERROR_RETURN(xflow_macsec_db_get(unit, &macsec_db));

    flags = XFLOW_MACSEC_DECRYPT | XFLOW_MACSEC_POLICY_WITH_ID;

    policy_id = XFLOW_MACSEC_POLICY_ID_CREATE(XFLOW_MACSEC_DECRYPT, macsec_db->reserved_policy_index);

    memset(&policy_info, 0, sizeof(xflow_macsec_policy_info_t));

    BCM_IF_ERROR_RETURN( xflow_macsec_policy_create(unit, flags, &policy_info, &policy_id));

    return BCM_E_NONE;
}
#if 0
/*
 * Function:
 *      xflow_macsec_fl_port_setup
 * Description:
 *      Configure per port parameters
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      lport - (IN) Logical port
 *      macsec_enable - (IN) Enable per port macsec
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PORT
 */
static int xflow_macsec_fl_port_setup(int unit,
                                      int lport,
                                      int macsec_enable)
{
    soc_ubus_field_t fld;
    uint32 rval;
    int crc_out, crc_in;

    if (macsec_enable)
    {
        crc_out = 0;
        crc_in = 1;
    }
    else
    {
        crc_out = 2;
        crc_in = 0;
    }
    fld = CRC_MODE_fld;
    rval = crc_out;
    BCM_IF_ERROR_RETURN(soc_ubus_xlmac_reg_fields32_modify(unit, XLMAC_TX_CTRLreg, lport, &fld, &rval));
    fld = STRIP_CRC_fld;
    rval = crc_in;
    BCM_IF_ERROR_RETURN(soc_ubus_xlmac_reg_fields32_modify(unit, XLMAC_RX_CTRLreg, lport, &fld, &rval));
    return BCM_E_NONE;
}
#endif
/*
 * Function:
 *      xflow_macsec_firelight_intr_enable
 * Description:
 *      Enable the corresponding interrupt
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      intr  - (IN) Interrupt type
 * Return Value:
 *      BCM_E_NONE
 */
int xflow_macsec_firelight_intr_enable(int unit,
                                       int intr)
{
#ifdef MACSEC_INTERRUPT_SUPPORT
    uint64 rval64 = 0;

    soc_ubus_reg_t reg = MACSEC_INTR_ENABLEreg;

    BCM_IF_ERROR_RETURN(soc_ubus_reg_get(unit, reg, REG_PORT_ANY, &rval64));

    SHR_BITSET((uint32*)&rval64, intr);

    BCM_IF_ERROR_RETURN(soc_ubus_reg_set(unit, reg, REG_PORT_ANY, rval64));
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *      _xflow_macsec_firelight_counters_init
 * Description:
 *      Initialize the counter map.
 * Parameters:
 *      unit  - (IN) BCM unit number
 * Return Value:
 *      BCM_E_NONE
 */
int _xflow_macsec_firelight_counters_init(int unit)
{
    int alloc_size;
    int i;
    xflow_macsec_db_t *db;
    _xflow_macsec_counter_regs_t *counter;
    _xflow_macsec_counter_regs_t fl_counter_map[] =
        {{_xflowMacsecCid0, ISEC_MIB_SP_UNCTRLm, INVALIDr, 0, 4},
         {_xflowMacsecCid1, ISEC_MIB_SP_CTRL_1m, INVALIDr, 0, 4},
         {_xflowMacsecCid2, ISEC_MIB_SP_CTRL_2m, INVALIDr, 0, 4},
         {_xflowMacsecCid3, ISEC_MIB_SCm, INVALIDr, 0, 4},
         {_xflowMacsecCid4, ISEC_MIB_SAm, INVALIDr, 0, 16},
         {_xflowMacsecCid5, ESEC_MIB_MISCm, INVALIDr, 0, 4},
         {_xflowMacsecCid6, ESEC_MIB_SC_UNCTRLm, INVALIDr, 0, 4},
         {_xflowMacsecCid7, ESEC_MIB_SC_CTRLm, INVALIDr, 0, 4},
         {_xflowMacsecCid8, ESEC_MIB_SCm, INVALIDr, 0, 4},
         {_xflowMacsecCid9, ESEC_MIB_SAm, INVALIDr, 0, 8},
         {_xflowMacsecCid10, ISEC_SPTCAM_HIT_COUNTm, INVALIDr, 0, 4},
         {_xflowMacsecCid11, ISEC_PORT_COUNTERSm, INVALIDr, 0, 4},
         {_xflowMacsecCid12, ISEC_SCTCAM_HIT_COUNTm, INVALIDr, 0, 4},
         {_xflowMacsecCid13, INVALIDm, INVALIDr},
         {_xflowMacsecCidCount, INVALIDm, INVALIDr},
         {_xflowMacsecCidInvalid, INVALIDm, INVALIDr},
         {_xflowMacsecCidSpecial, INVALIDm, INVALIDr},
        };

    char *_xflow_macsec_cid_names[] =
    {
        "_xflowMacsecCid0", "_xflowMacsecCid1", "_xflowMacsecCid2",
        "_xflowMacsecCid3", "_xflowMacsecCid4", "_xflowMacsecCid5",
        "_xflowMacsecCid6", "_xflowMacsecCid7", "_xflowMacsecCid8",
        "_xflowMacsecCid9", "_xflowMacsecCid10", "_xflowMacsecCid11",
        "_xflowMacsecCid12", "_xflowMacsecCid13", "_xflowMacsecCidCount",
        "_xflowMacsecCidInvalid", "_xflowMacsecCidSpecial"
    };

    BCM_IF_ERROR_RETURN(xflow_macsec_db_get(unit, &db));
    alloc_size = (_xflowMacsecCidSpecial + 1) * sizeof(_xflow_macsec_counter_regs_t);

    db->_xflow_macsec_counter_array = sal_alloc(alloc_size, "_xflow_macsec_counter_array");

    _XFLOW_MACSEC_MEM_NULL_CHECK_AND_SET(db->_xflow_macsec_counter_array, alloc_size);

    /*
     * The following set of counter allocations will be used by
     * the counter thread to DMA the counters to the allocated
     * buffers.
     */
    for (i = _xflowMacsecCid0; i <= _xflowMacsecCidSpecial; i++)
    {
        counter = &(db->_xflow_macsec_counter_array[i]);
        counter->id = i;
        counter->mem = fl_counter_map[i].mem;
        counter->reg = fl_counter_map[i].reg;
        counter->min_index = 0;
        if (counter->mem != INVALIDm)
        {
            counter->num_entries = fl_counter_map[i].num_entries ? : soc_mem_index_count(unit, counter->mem);
            alloc_size = counter->num_entries * sizeof(uint32)
                            * soc_mem_entry_words(unit, counter->mem);
            counter->buf = soc_cm_salloc(unit, alloc_size,
                                         _xflow_macsec_cid_names[i]);
            _XFLOW_MACSEC_MEM_NULL_CHECK_AND_SET(counter->buf, alloc_size);
        }
        else if (counter->reg != INVALIDr)
        {
            alloc_size = 2 * sizeof(uint32);
            counter->buf = soc_cm_salloc(unit, alloc_size,
                                         _xflow_macsec_cid_names[i]);
        }
    }

    for (i = 1; i < xflowMacsecStatTypeCount; i++)
    {
        alloc_size = _XFLOW_MACSEC_ID_MAX * sizeof(uint64);
        _xflow_macsec_fl_stat_map_list[i].sw_value = sal_alloc(alloc_size,
                                        "_xflow_macsec_fl_stat_map_list.sw_value");
        _XFLOW_MACSEC_MEM_NULL_CHECK_AND_SET
                (_xflow_macsec_fl_stat_map_list[i].sw_value, alloc_size);

    }
    db->_xflow_macsec_stat_map_list = _xflow_macsec_fl_stat_map_list;

    return BCM_E_NONE;
}

/*
 * Function:
 *      xflow_macsec_firelight_mac_addr_control_set
 * Description:
 *      Configure the mac address control parameters.
 * Parameters:
 *      unit  - (IN) BCM unit number
 *      flags - (IN) Encrypt/Decrypt
 *      type - (IN) Control paramter type
 *      mac_addr - (IN) Configured mac address
 *      value - (IN) Configured value
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 *      BCM_E_UNAVAIL
 *      BCM_E_INTERNAL
 */
int xflow_macsec_firelight_mac_addr_control_set(int unit,
                                                uint32 flags,
                                                xflow_macsec_mac_addr_control_t type,
                                                bcm_mac_t mac_addr,
                                                bcm_ethertype_t value)
{
    uint64 reg64, macda;
    uint32 fval;
    int rv;
    soc_ubus_reg_t reg;
    soc_ubus_field_t fld = DA_fld, fld1 = INVALID_fld;
    uint32 fwidth;
    uint64 val_max = 0;

    switch (type)
    {
        case xflowMacsecMgmtDstMac0:
            reg = ISEC_MGMTRULE_CFG_DA_0reg;
            break;
        case xflowMacsecMgmtDstMac1:
            reg = ISEC_MGMTRULE_CFG_DA_1reg;
            break;
        case xflowMacsecMgmtDstMac2:
            reg = ISEC_MGMTRULE_CFG_DA_2reg;
            break;
        case xflowMacsecMgmtDstMac3:
            reg = ISEC_MGMTRULE_CFG_DA_3reg;
            break;
        case xflowMacsecMgmtDstMac4:
            reg = ISEC_MGMTRULE_CFG_DA_4reg;
            break;
        case xflowMacsecMgmtDstMac5:
            reg = ISEC_MGMTRULE_CFG_DA_5reg;
            break;
        case xflowMacsecMgmtDstMac6:
            reg = ISEC_MGMTRULE_CFG_DA_6reg;
            break;
        case xflowMacsecMgmtDstMac7:
            reg = ISEC_MGMTRULE_CFG_DA_7reg;
            break;
        case xflowMacsecMgmtDstMacRangeLow:
            reg = ISEC_MGMTRULE_DA_RANGE_LOWreg;
            break;
        case xflowMacsecMgmtDstMacRangeHigh:
            reg = ISEC_MGMTRULE_DA_RANGE_HIGHreg;
            break;
        case xflowMacsecMgmtDstMacEthertype0:
            reg = ISEC_MGMTRULE_DA_ETYPE_1STreg;
            fld1 = ETYPE_fld;
            break;
        case xflowMacsecMgmtDstMacEthertype1:
            reg = ISEC_MGMTRULE_DA_ETYPE_2NDreg;
            fld1 = ETYPE_fld;
            break;
        default:
            return BCM_E_UNAVAIL;
    }

    COMPILER_64_ZERO(reg64);

    if (fld1 != INVALID_fld)
    {
        fval = COMPILER_64_LO(value);
        fwidth = soc_ubus_reg_field_length(unit, reg, fld1);
        if (fwidth > 32)
        {
            val_max = (1 << fwidth) - 1;
        }
        else
        {
            val_max = 0xffffffff;
        }

        if (fval > val_max)
        {
            return BCM_E_PARAM;
        }
        soc_ubus_reg64_field32_set(unit, reg, &reg64, fld1, fval);
    }
    else
    {
        SAL_MAC_ADDR_TO_UINT64(mac_addr, macda);
        soc_ubus_reg64_field_set(unit, reg, &reg64, fld, macda);
    }

    rv = soc_ubus_reg_set(unit, reg, REG_PORT_ANY, reg64);

    if (BCM_FAILURE(rv))
    {
        return BCM_E_INTERNAL;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      xflow_macsec_firelight_init
 * Description:
 *      Firelight Macsec initialization routine.
 * Parameters:
 *      unit  - (IN) BCM unit number
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_MEMORY
 *      BCM_E_TIMEOUT
 */
int xflow_macsec_firelight_init(int unit)
{
    uint32 rval = 0;
    uint64 rval64 = 0;
    soc_ubus_reg_t reg;
    xflow_macsec_db_t *macsec_db = NULL;
    int port;
    uint32 val;
    int macsec_port, iter;
    uint32 intr_enable[] = {
        XFLOW_MACSEC_ENCRYPT_INTR_SA_EXP_NON_EMPTY,
        XFLOW_MACSEC_DECRYPT_INTR_SA_EXP_NON_EMPTY
    };

    BCM_IF_ERROR_RETURN(xflow_macsec_db_get(unit, &macsec_db));

    num_sa_per_sc[unit] = 0;

    macsec_db->reserved_policy_index = _XFLOW_MACSEC_FL_RESERVED_POLICY_INDEX;

    /* Setup the General ISEC Control Register *********************************************************************/
    BCM_IF_ERROR_RETURN(soc_ubus_reg_get(unit, ISEC_CTRLreg, REG_PORT_ANY, &rval64));

    reg = ISEC_CTRLreg;
    val = cmbb_soc_property_get(unit, CBB_GEN_MACSEC_PARAM_TYPE(XFLOW_MACSEC_DECRYPT_NON_KAY_MGMT_COPY_TO_CPU));
    soc_ubus_reg64_field32_set(unit, reg, &rval64, SET_SVTAG_CPU_FOR_MGMT_fld, !!val);

    val = cmbb_soc_property_get(unit, CBB_GEN_MACSEC_PARAM_TYPE(XFLOW_MACSEC_DECRYPT_KAY_COPY_TO_CPU));
    soc_ubus_reg64_field32_set(unit, reg, &rval64, SET_SVTAG_CPU_FOR_KAY_fld, !!val);

    val = cmbb_soc_property_get(unit, CBB_GEN_MACSEC_PARAM_TYPE(XFLOW_MACSEC_DECRYPT_AUTO_SECURE_ASSOC_INVALIDATE));
    soc_ubus_reg64_field32_set(unit, reg, &rval64, INVALIDATE_SA_EN_fld, !!val);

    BCM_IF_ERROR_RETURN(xflow_macsec_firelight_flow_default_policy_reserve(unit));

    val = macsec_db->reserved_policy_index;
    soc_ubus_reg64_field32_set(unit, reg, &rval64, SPNUM_SPTCAM_MIS_fld, val);

    val = 0xff;
    if (cmbb_soc_property_get(unit, CBB_GEN_MACSEC_PARAM_TYPE(XFLOW_MACSEC_DECRYPT_UNKNOWN_POLICY_DROP)))
    {
        val &=0xfe;
    }

    if (cmbb_soc_property_get(unit, CBB_GEN_MACSEC_PARAM_TYPE(XFLOW_MACSEC_DECRYPT_TAG_CTRL_PORT_ERROR_DROP)))
    {
        val &=0xfd;
    }

    if (cmbb_soc_property_get(unit, CBB_GEN_MACSEC_PARAM_TYPE(XFLOW_MACSEC_DECRYPT_UNTAG_CTRL_PORT_ERROR_DROP)))
    {
        val &=0xfb;
    }

    if (cmbb_soc_property_get(unit, CBB_GEN_MACSEC_PARAM_TYPE(XFLOW_MACSEC_DECRYPT_IPV4_MPLS_ERROR_DROP)))
    {
        val &=0xf7;
    }

    if (cmbb_soc_property_get(unit, CBB_GEN_MACSEC_PARAM_TYPE(XFLOW_MACSEC_DECRYPT_INVALID_SECTAG_DROP)))
    {
        val &=0xef;
    }

    if (cmbb_soc_property_get(unit, CBB_GEN_MACSEC_PARAM_TYPE(XFLOW_MACSEC_DECRYPT_UNKNOWN_SECURE_CHAN_DROP)))
    {
        val &=0xdf;
    }

    if (cmbb_soc_property_get(unit, CBB_GEN_MACSEC_PARAM_TYPE(XFLOW_MACSEC_DECRYPT_UNKNOWN_SECURE_ASSOC_DROP)))
    {
        val &=0xbf;
    }

    if (cmbb_soc_property_get(unit, CBB_GEN_MACSEC_PARAM_TYPE(XFLOW_MACSEC_DECRYPT_REPLAY_FAILURE_DROP)))
    {
        val &=0x7f;
    }

    soc_ubus_reg64_field32_set(unit, reg, &rval64, SVTAG_SOP_ERROR_CODE_SET_CPU_EN_fld, val);
    soc_ubus_reg64_field32_set(unit, reg, &rval64, SVTAG_SOP_ERROR_CODE_PKT_FWD_EN_fld, val);

    BCM_IF_ERROR_RETURN(soc_ubus_reg_set(unit, ISEC_CTRLreg, REG_PORT_ANY, rval64));

    /* Setup the General ISEC MISC Register ************************************************************************/
    BCM_IF_ERROR_RETURN(soc_ubus_reg32_get(unit, ISEC_MISC_CTRLreg, REG_PORT_ANY, &rval));

    soc_ubus_reg_field_set(unit, ISEC_MISC_CTRLreg, &rval, USE_PORTID_FOR_SVTAG_SC_INDEX_fld, 1);
    BCM_IF_ERROR_RETURN(soc_ubus_reg32_set(unit, ISEC_MISC_CTRLreg, REG_PORT_ANY, rval));

    memset(&fl_macsec_lport_map, -1, sizeof(int) * CMBB_FL_MACSEC_MAX_PORT_NUM);

    for(port = 0; port < CMBB_FL_MACSEC_MAX_PORT_NUM; port++)
    {
        /* For inline MACSEC physical and logical ports are 1 to 1 */
        macsec_port = port;

        if ((macsec_port != -1) && (macsec_port < CMBB_FL_MACSEC_MAX_PORT_NUM))
        {
            fl_macsec_lport_map[macsec_port] = port;

            BCM_IF_ERROR_RETURN(soc_ubus_reg32_get(unit, ESEC_CONFIGreg, port, &rval));

            val = cmbb_soc_property_port_get(unit, port, 
                                            CBB_PORT_MACSEC_PARAM_TYPE(XFLOW_MACSEC_ENCRYPT_DROP_SVTAG_ERROR_PACKET));

            val = 0;
            val = (val == 0) ? 1 : 0;
            soc_ubus_reg_field_set(unit, ESEC_CONFIGreg, &rval, DROP_OR_FORWARD_BAD_SVTAG_PKTS_fld, val);

            val = cmbb_soc_property_port_get(unit, port,
                                            CBB_PORT_MACSEC_PARAM_TYPE(XFLOW_MACSEC_ENCRYPT_PHY_PORT_BASED_MACSEC));
            
            val = 1;
            val = (val == 0) ? 0 : 1;
            macsec_db->port_based_macsec |= val;

            soc_ubus_reg_field_set(unit, ESEC_CONFIGreg, &rval, EN_PORT_BASED_SC_fld, val);

            soc_ubus_reg_field_set(unit, ESEC_CONFIGreg, &rval, TX_THRESHOLD_fld, XFLOW_MACSEC_TX_THRESHOLD);


            BCM_IF_ERROR_RETURN(soc_ubus_reg32_set(unit, ESEC_CONFIGreg, port, rval));
//            BCM_IF_ERROR_RETURN(xflow_macsec_fl_port_setup(unit, port, 1));
        }
    }

    for (iter = 0; iter < XFLOW_MACSEC_NUM_INTR; iter++)
    {
        BCM_IF_ERROR_RETURN(xflow_macsec_firelight_intr_enable(unit, intr_enable[iter]));
    }

    return BCM_E_NONE;
}
