/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2012:DUAL/GPL:standard
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

#ifndef CMBB_XFLOW_MACSEC_CFG_PARAMS_H
#define CMBB_XFLOW_MACSEC_CFG_PARAMS_H

#define CBB_GEN_MACSEC_PARAM_TYPE(p)    cmbb_gen_param_type_##p
#define CBB_GEN_MACSEC_PARAM(p)         cmbb_gen_param_##p

#define CBB_PORT_MACSEC_PARAM_TYPE(p)   cmbb_port_param_type_##p
#define CBB_PORT_MACSEC_PARAM(p)        cmbb_port_param_##p

#define spn_XFLOW_MACSEC_SKIP_DECRYPT_PKT_PARSER    100

/* Configuration parameter types for RG XFlow MACSEC sub-system */
#define cmbb_gen_param_type_XFLOW_MACSEC_DECRYPT_NON_KAY_MGMT_COPY_TO_CPU       1
#define cmbb_gen_param_type_XFLOW_MACSEC_DECRYPT_KAY_COPY_TO_CPU                2
#define cmbb_gen_param_type_XFLOW_MACSEC_DECRYPT_AUTO_SECURE_ASSOC_INVALIDATE   3
#define cmbb_gen_param_type_XFLOW_MACSEC_DECRYPT_UNKNOWN_POLICY_DROP            4
#define cmbb_gen_param_type_XFLOW_MACSEC_DECRYPT_TAG_CTRL_PORT_ERROR_DROP       5
#define cmbb_gen_param_type_XFLOW_MACSEC_DECRYPT_UNTAG_CTRL_PORT_ERROR_DROP     6
#define cmbb_gen_param_type_XFLOW_MACSEC_DECRYPT_IPV4_MPLS_ERROR_DROP           7
#define cmbb_gen_param_type_XFLOW_MACSEC_DECRYPT_INVALID_SECTAG_DROP            8
#define cmbb_gen_param_type_XFLOW_MACSEC_DECRYPT_UNKNOWN_SECURE_CHAN_DROP       9
#define cmbb_gen_param_type_XFLOW_MACSEC_DECRYPT_UNKNOWN_SECURE_ASSOC_DROP      10
#define cmbb_gen_param_type_XFLOW_MACSEC_DECRYPT_REPLAY_FAILURE_DROP            11
#define cmbb_port_param_type_XFLOW_MACSEC_ENCRYPT_DROP_SVTAG_ERROR_PACKET       12
#define cmbb_port_param_type_XFLOW_MACSEC_ENCRYPT_PHY_PORT_BASED_MACSEC         13

/* Configuration parameter values for RG XFlow MACSEC sub-system */
#define cmbb_gen_param_XFLOW_MACSEC_DECRYPT_NON_KAY_MGMT_COPY_TO_CPU            1
#define cmbb_gen_param_XFLOW_MACSEC_DECRYPT_KAY_COPY_TO_CPU                     0
#define cmbb_gen_param_XFLOW_MACSEC_DECRYPT_AUTO_SECURE_ASSOC_INVALIDATE        0       
#define cmbb_gen_param_XFLOW_MACSEC_DECRYPT_UNKNOWN_POLICY_DROP                 1
#define cmbb_gen_param_XFLOW_MACSEC_DECRYPT_TAG_CTRL_PORT_ERROR_DROP            1
#define cmbb_gen_param_XFLOW_MACSEC_DECRYPT_UNTAG_CTRL_PORT_ERROR_DROP          1
#define cmbb_gen_param_XFLOW_MACSEC_DECRYPT_IPV4_MPLS_ERROR_DROP                1
#define cmbb_gen_param_XFLOW_MACSEC_DECRYPT_INVALID_SECTAG_DROP                 1
#define cmbb_gen_param_XFLOW_MACSEC_DECRYPT_UNKNOWN_SECURE_CHAN_DROP            1
#define cmbb_gen_param_XFLOW_MACSEC_DECRYPT_UNKNOWN_SECURE_ASSOC_DROP           1
#define cmbb_gen_param_XFLOW_MACSEC_DECRYPT_REPLAY_FAILURE_DROP                 0

#define cmbb_port_param_XFLOW_MACSEC_ENCRYPT_DROP_SVTAG_ERROR_PACKET            0
#define cmbb_port_param_XFLOW_MACSEC_ENCRYPT_PHY_PORT_BASED_MACSEC              1

/* Default SC settings used before key exchange setup */
#define MACSEC_CONFIG_DEFAULT_SECTAG_OFFSET                                     12
#define MACSEC_CONFIG_DEFAULT_CRYPTO_ALG                                        xflowMacsecCryptoAes128Gcm

/* SC settings used to establish a new session via key exchange */
#define MACSEC_CONFIG_CRYPTO_ALG_SETTING                                        bcmXflowMacsecCryptoAes128GcmXpn
#define MACSEC_CONFIG_AN_CONTROL_SETTING                                        bcmXflowMacsecSecureAssocAnRollover

/* Decrypt policy settings flags */
#define MACSEC_CONFIG_DECRYPT_POLICY_FLAG_SETTINGS                              (XFLOW_MACSEC_DECRYPT_POLICY_UNTAGGED_CONTROL_PORT_ENABLE |\
                                                                                 XFLOW_MACSEC_DECRYPT_POLICY_TAGGED_CONTROL_PORT_ENABLE |\
                                                                                 BCM_XFLOW_MACSEC_DECRYPT_POLICY_POINT_TO_POINT_ENABLE)
                                                                                 

#define MACSEC_CONFIG_DECRYPT_MAX_MTU_SIZE                                      1536
#define MACSEC_CONFIG_DECRYPT_SECTAG_OFFSET                                     12

#define MACSEC_CONFIG_ENCRYPT_MAX_MTU_SIZE                                      1536
#define MACSEC_CONFIG_ENCRYPT_SECTAG_OFFSET                                     12
/*
 * Options:
 *      xflowMacsecTagValidateBypassMacsec
 *      xflowMacsecTagValidateCheckICV
 *      xflowMacsecTagValidateStrict
 *      xflowMacsecTagValidateCheckNone
 *      xflowMacsecTagValidateDenyAll
 *      xflowMacsecTagValidateCount
 */
#define MACSEC_CONFIG_DECRYPT_TAG_VALIDATION_POLICY                             xflowMacsecTagValidateCheckNone //Strict

#define MACSEC_CONFIG_DECRYPT_VLAN_MPLS_LABEL_FLAGS                             XFLOW_MACSEC_NO_TAGS_NO_LABELS
#define MACSEC_CONFIG_DECRYPT_FRAME_TYPE_FILTER                                 xflowMacsecFlowFrameAny

#define MACSEC_CONFIG_ENCRYPT_TCI_E_BIT_SETTING                                 0x1
#define MACSEC_CONFIG_ENCRYPT_TCI_C_BIT_SETTING                                 0x1

#define MACSEC_CONFIG_ENCRYPT_POLICY_FLAG_SETTINGS                              (BCM_XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_CONTROLLED_SECURED_DATA |\
                                                                                 BCM_XFLOW_MACSEC_SECURE_CHAN_INFO_CONTROLLED_PORT)

/*
 *  SecTAG_ICV_MODE
 *      Strip all or parts of the MACsec encapsulation after packets have been permitted.
 *      Leaving the SecTAG in place allows for downstream blocks to do additional policy checks.
 *      The field is encoded as follows:
 *          2'b00 : Leave both the SecTAG and ICV in the packet
 *          2'b01 : Leave the SecTAG in the packet, but strip the ICV
 *          2'b10 : Reserved
 *          2'b11 : Strip both the SecTAG and ICV (Normal MACsec mode of operation)
 *      Support for leaving the ICV in place, but stripping the SecTAG is not required and 
 *      therefore not called out in the encoding of the field
 */
#define MACSEC_CONFIG_SECTAG_ICV_MODE                                           3

/* 
 * SA Configuration Settings
 */
#define MACSEC_CONFIG_SA_NEXT_PKT_NUM_FLAG                                      BCM_XFLOW_MACSEC_SECURE_ASSOC_INFO_SET_NEXT_PKT_NUM

#define MACSEC_CONFIG_EAPOL_MGMT_PKT_TYPE                                       0x888e

unsigned int cmbb_soc_property_get(int unit, int param_type);

#endif
