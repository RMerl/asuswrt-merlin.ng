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

#ifndef _SOC_ALLENUM_H
#define _SOC_ALLENUM_H


#ifndef EXTERN
# ifdef __cplusplus
#  define EXTERN extern "C"
# else
#  define EXTERN extern
# endif
#endif

#include <soc/mcm/enum_types.h>
#include <soc/mcm/enum_max.h>

#define INVALIDr -1
#define INVALIDm -1
#define INVALIDf -1

#define NUM_SOC_MEM 30

/*
 * Global memory table enums 
 */
#define MACSEC_TDM_CALENDARm            0
#define MACSEC_MACSEC_TDM_2_CALENDARm   1
#define ESEC_SA_EXPIRE_STATUSm          2
#define ISEC_SA_EXPIRE_STATUSm          3
#define ESEC_MIB_ROLLOVER_FIFOm         4
#define ISEC_MIB_ROLLOVER_FIFOm         5
#define ESEC_SC_TABLEm                  6
#define ESEC_SA_TABLEm                  7
#define ESEC_SA_HASH_TABLEm             8
#define SUB_PORT_MAP_TABLEm             9
#define SUB_PORT_POLICY_TABLEm          10
#define ISEC_SC_TABLEm                  11
#define ISEC_SA_TABLEm                  12
#define ISEC_SA_HASH_TABLEm             13
#define ISEC_SP_TCAM_KEYm               14
#define ISEC_SP_TCAM_MASKm              15
#define ISEC_SC_TCAMm                   16
#define ESEC_MIB_MISCm                  17
#define ESEC_MIB_SC_UNCTRLm             18
#define ESEC_MIB_SC_CTRLm               19
#define ESEC_MIB_SCm                    20
#define ESEC_MIB_SAm                    21
#define ISEC_SPTCAM_HIT_COUNTm          22
#define ISEC_SCTCAM_HIT_COUNTm          23
#define ISEC_PORT_COUNTERSm             24
#define ISEC_MIB_SP_UNCTRLm             25
#define ISEC_MIB_SP_CTRL_1m             26
#define ISEC_MIB_SP_CTRL_2m             27
#define ISEC_MIB_SCm                    28
#define ISEC_MIB_SAm                    29

/*
 * Global memory table sub-field enums 
 */
#define PORTIDf                                      103773
#define ISEC_SPTCAM_HIT_COUNTSPTCAM_HITf             69514
#define ISEC_SPTCAM_HIT_COUN_TSPTCAM_HITf            69517
#define ISEC_SPTCAM_INDEXf                           69518
#define ESEC_SC_INDEXf                               45157
#define ESEC_SA_INDEXf                               45155
#define ESEC_PORT_INDEXf                             45152
#define ESEC_MIB_SC_UNCTRLOUTUCASTPKTSf              45133
#define ESEC_MIB_SC_UNCTRLOUTMGMTPKTSf               45130
#define ESEC_MIB_SC_UNCTRLOUTMULTICASTPKTSf          45131
#define ESEC_MIB_SC_UNCTRLOUTOCTETSf                 45132
#define ESEC_MIB_SC_UNCTRLOUTERRORSf                 45129
#define ESEC_MIB_SC_UNCTRLOUTBROADCASTPKTSf          45128
#define ESEC_MIB_SC_CTRLOUTOCTETSf                   45114
#define ESEC_MIB_SC_CTRLOUTMULTICASTPKTSf            45113
#define ESEC_MIB_SC_CTRLOUTERRORSf                   45112
#define ESEC_MIB_SC_CTRLOUTUCASTPKTSf                45115
#define ESEC_MIB_SC_CTRLOUTBROADCASTPKTSf            45111
#define ESEC_MIB_SCSECYTXSCSTATSOCTETSPROTECTEDf     45110
#define ESEC_MIB_SCSECYTXSCSTATSOCTETSENCRYPTEDf     45109
#define ESEC_MIB_SCSECYSTATSTXUNTAGGEDPKTSf          45108
#define ESEC_MIB_SCSECYSTATSTXTOOLONGPKTSf           45107
#define ESEC_MIB_SASECYTXSASTATSENCRYPTEDPKTSf       45103
#define ESEC_MIB_SASECYTXSASTATSPROTECTEDPKTSf       45104
#define ESEC_MIB_MISCPKTDROPf                        45095
#define ESEC_MIB_MISCBADCUSTOMHDRf                   45094
#define CONFIDENTIALITY_OFFSETf                      21230
#define CIPHERSUITE_PROTECTIONf                      18523
#define REPLAY_PROTECTf                              115664
#define REPLAY_PROTECT_WINDOWf                       115665
#define UNTAG_VLDT_FRM_STRCTf                        144411
#define TAG_VLDT_FRM_STRCTf                          135646
#define OPER_POINT2POINT_MACf                        93529
#define MPLS_SECTAG_OFFSETf                          88565
#define LOOPBACK_PKTf                                78816
#define PKT_TYPEf                                    100450
#define VLAN_TAG_STATUSf                             147285
#define MAC_SAf                                      80664
#define MAC_DAf                                      80516
#define PORT_NUMf                                    104731
#define MTU_SELf                                     89160
#define INCLUDE_SCIf                                 62855
#define CIPHERSUITEPROTECTIONf                       18522
#define CONTROLLEDPORTENABLEDf                       21621
#define AUTHENTICATION_END_OFFSET1f                  2865
#define AUTHENTICATION_START_OFFSET1f                2868
#define NEXT_PNf                                     89915
#define FULL_KEYf                                    55836
#define FULL_MASKf                                   55838
#define NEXTPNf                                      89826
#define OUTOCTETSf                                   94321
#define OUTUCASTPKTSf                                94424
#define OUTBROADCASTPKTSf                            94200
#define OUTERRORSf                                   94203
#define OUTMGMTPKTSf                                 94319
#define OUTMULTICASTPKTSf                            94320
#define OUTOCTETSf                                   94321
#define OUTUCASTPKTSf                                94424
#define ADDOFFSET2SECTAGLOCATIONf                    1041
#define ADDOFFSET2VXLANSECOFFSETSf                   1042
#define ANf                                          2256
#define ANCONTROLf                                   2264
#define AN_CONTROLf                                  2303
#define DROP_OR_ZERO_OUT_SA_INVALID_PKTSf            34324
#define EG_SC_RSVDf                                  39973
#define FORWARDSECUREDDATAPKTf                       54378
#define FORWARDUNSECUREDDATAPKTf                     54379
#define PROTECTFRAMESf                               106788
#define SECTAGLOCATIONf                              125906
#define SECUREDDATAPKTENABLEDf                       126000
#define TAG_FORWARDFRAMESf                           135595
#define TCIf                                         136445
#define TCI_Ef                                       136447
#define TCI_Cf                                       136446
#define TCI_ESf                                      136448
#define TCI_SCf                                      136449
#define TCI_SCBf                                     136450
#define UNSECUREDDATAPKTENABLEDf                     144398
#define VXLANSEC_ENC_PKT_TYPEf                       149379
#define HASHf                                        57757
#define SAKf                                         124387
#define SALTf                                        124394
#define SSCIf                                        132966
#define STARTEDTIMEf                                 133159
#define STATUSf                                      133338
#define STOPPEDTIMEf                                 133630
#define ISEC_MIB_SASECYRXSASTATSINVALIDPKTSf         69418
#define ISEC_MIB_SASECYRXSASTATSNOTUSINGSAPKTSf      69419
#define ISEC_MIB_SASECYRXSASTATSNOTVALIDPKTSf        69420
#define ISEC_MIB_SASECYRXSASTATSOKPKTSf              69421
#define ISEC_MIB_SASECYRXSASTATSUNUSEDSAPKTSf        69422
#define ISEC_MIB_SCSECYRXSCSTATSDELAYEDPKTSf         69426
#define ISEC_MIB_SCSECYRXSCSTATSLATEPKTSf            69427
#define ISEC_MIB_SCSECYRXSCSTATSOCTETSDECRYPTEDf     69428
#define ISEC_MIB_SCSECYRXSCSTATSOCTETSVALIDATEDf     69429
#define ISEC_MIB_SCSECYRXSCSTATSUNCHECKEDPKTSf       69430
#define ISEC_MIB_SP_CTRL_1SECYSTATSRXBADTAGPKTSf     69433
#define ISEC_MIB_SP_CTRL_1SECYSTATSRXNOSCIPKTSf      69434
#define ISEC_MIB_SP_CTRL_1SECYSTATSRXNOTAGPKTSf      69435
#define ISEC_MIB_SP_CTRL_1SECYSTATSRXUNKNOWNSCIPKTSf 69436
#define ISEC_MIB_SP_CTRL_1SECYSTATSRXUNTAGGEDPKTSf   69437
#define ISEC_MIB_SP_CTRL_2INBROADCASTPKTSf           69445
#define ISEC_MIB_SP_CTRL_2INDISCARDSf                69446
#define ISEC_MIB_SP_CTRL_2INERRORSf                  69447
#define ISEC_MIB_SP_CTRL_2INMULTICASTPKTSf           69448
#define ISEC_MIB_SP_CTRL_2INOCTETSf                  69449
#define ISEC_MIB_SP_CTRL_2INUCASTPKTSf               69450
#define ISEC_MIB_SP_UNCTRLINBROADCASTPKTSf           69459
#define ISEC_MIB_SP_UNCTRLINKAYPKTSf                 69460
#define ISEC_MIB_SP_UNCTRLINMGMTPKTSf                69461
#define ISEC_MIB_SP_UNCTRLINMULTICASTPKTSf           69462
#define ISEC_MIB_SP_UNCTRLINOCTETSf                  69463
#define ISEC_MIB_SP_UNCTRLINUCASTPKTSf               69464
#define ISEC_PORT_COUNTERSPKTDROPf                   69490
#define ISEC_PORT_COUNTERSSCTCAM_MISSf               69491
#define ISEC_PORT_COUNTERSSPTCAM_MISSf               69492
#define ISEC_PORT_INDEXf                             69498
#define ISEC_SA_INDEXf                               69502
#define ISEC_SCTCAM_HIT_COUNTSCTCAM_HITf             69504
#define ISEC_SA_INDEXf                               69502
#define ISEC_SCTCAM_HIT_COUNTSCTCAM_HITf             69504
#define ISEC_SCTCAM_INDEXf                           69508
#define ISEC_SC_INDEXf                               69509
#define ISEC_SECY_INDEXf                             69513
#define SECYRXSASTATSINVALIDPKTSf                    126023
#define SECYRXSASTATSNOTUSINGSAPKTSf                 126024
#define SECYRXSASTATSNOTVALIDPKTSf                   126025
#define SECYRXSASTATSOKPKTSf                         126026
#define SECYRXSASTATSUNUSEDSAPKTSf                   126027
#define SECYRXSCSTATSDELAYEDPKTSf                    126028
#define SECYRXSCSTATSLATEPKTSf                       126029
#define SECYRXSCSTATSOCTETSDECRYPTEDf                126030
#define SECYRXSCSTATSOCTETSVALIDATEDf                126031
#define SECYRXSCSTATSUNCHECKEDPKTSf                  126032
#define SECYSTATSRXBADTAGPKTSf                       126033
#define SECYSTATSRXNOSCIPKTSf                        126034
#define SECYSTATSRXNOTAGPKTSf                        126035
#define SECYSTATSRXUNKNOWNSCIPKTSf                   126036
#define SECYSTATSRXUNTAGGEDPKTSf                     126037
#define SECYSTATSTXTOOLONGPKTSf                      126038
#define SECYSTATSTXUNTAGGEDPKTSf                     126039
#define SECYTXSASTATSENCRYPTEDPKTSf                  126040
#define SECYTXSASTATSPROTECTEDPKTSf                  126041
#define SECYTXSCSTATSOCTETSENCRYPTEDf                126042
#define SECYTXSCSTATSOCTETSPROTECTEDf                126043
#define INDISCARDSf                                  62976
#define INERRORSf                                    62983
#define INBROADCASTPKTSf                             62851
#define INKAYPKTSf                                   64261
#define INMGMTPKTSf                                  64276
#define INMULTICASTPKTSf                             64277
#define UNTAG_FORWARDFRAMESf                         144406
#define UNTAG_CTRL_PORT_ENf                          144403
#define TAG_VALIDATEFRAMESf                          135644
#define TAG_LABEL_ADJ_ENf                            135599
#define TAG_CTRL_PORT_ENf                            135583
#define SECTAG_OFFSETf                               125931
#define SECTAG_ETYPE_SELf                            125927
#define SCIf                                         125453
#define RSVDf                                        119568
#define OPERPOINTTOPOINTMACf                         93528
#define MTUf                                         89124
#define INNER_L2_VALIDf                              64317
#define INNER_L2_OFFSETf                             64316
#define DROP_IPV4_CHKCUM_FAIL_AND_MPLS_BOS_MISSf     34205
#define CUSTOM_PROTO_SPf                             26491
#define MS_SUBPORT_NUMf                              88913
#define MANAGEMENT_PACKETf                           80710
#define PORTIDf                                      103773
#define VALIDf                                       145090
#define UDF_MASKf                                    143830
#define TAG_LABEL_STATUS_MASKf                       135601
#define RSVD2_MASKf                                  119576
#define RSVD1_MASKf                                  119572
#define PORT_MAP_MASKf                               104644
#define MASKf                                        81104
#define FRAME_FORMAT_MASKf                           55093
#define UDFf                                         143717
#define TAG_LABEL_STATUSf                            135600
#define BADCUSTOMHDRf                                4298
#define RSVD1f                                       119570
#define RSVD2f                                       119573
#define PORT_MAPf                                    104640
#define KEYf                                         72952
#define FRAME_FORMATf                                55092
#define SPTCAM_HITf                                  131376
#define SCI_MASKf                                    125457
#define MS_SUBPORT_NUM_MASKf                         88914
#define VXLANSEC_L3L4_HDR_UPDATEf                    149380
#define SECTAG_ICV_MODEf                             125930
#define REPLAYWINDOWf                                115656
#define REPLAYPROTECTf                               115655
#define NONIEEEAUTHSTARTf                            90586
#define NONIEEEAUTHENDf                              90585
#define IG_SC_RSVDf                                  60840
#define IEEE_STD_802PT1AE_AUTHENTICATIONDISABLEf     60193
#define CONFIDENTIALITYOFFSETf                       21229
#define CIPHERSUITEf                                 18521
#define ADDOFFSET2NONIEEEAUTHSTARTf                  1040
#define ADDOFFSET2NONIEEEAUTHENDf                    1039
#define SCTCAM_HITf                                  125598
#define SA_INDEXf                                    124733
#define EXPIRY_TYPEf                                 46750
#define CORRUPT_ENTRYf                               22110
#define SPTCAM_MISSf                                 131377
#define SCTCAM_MISSf                                 125599
#define PKTDROPf                                     99685
#define INUCASTPKTSf                                 65636
#define INOCTETSf                                    64362

#endif  /* !_SOC_ALLENUM_H */
