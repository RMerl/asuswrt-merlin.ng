/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
       All Rights Reserved
    
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

/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation of the Lilac IH driver               */
/*                                                                            */
/******************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "rdp_subsystem_common.h"
#include "rdp_drv_ih.h"

#define IH_ENG_TRIPLE_TAG_DETECTION_BIT_SHIFT 8

/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

/* "all ones" (bitwise) value */
#define CS_ALL_ONES_VALUE ( 0xFFFFFFFF )

/* bit 12 in an address to common memory, selectes between the two sections */
#define CS_COMMON_MEMORY_SECTION_SELECTION_BIT  ( 12 )

/* there are 2 registesr which store the classifier-to-class mapping.
   each register stores mapping of 8 classifiers */
#define CS_NUMBER_OF_CLASSIFIERS_IN_CLASSIFIER_TO_CLASS_MAPPING_REGISTERS  ( 8 )

/* each register stores configuration of 4 ingress queues */
#define CS_NUMBER_OF_INGRESS_QUEUES_IN_PRIORITY_AND_CONGESTION_THRESHOLD_REGISTERS  ( 4 )

/* each DSCP-to-TCI table resides in 8 registers */
#define CS_NUMBER_OF_REGISTERS_OF_DSCP_TO_TCI_TABLE  ( 8 )

/* each register stores 8 entries */
#define CS_NUMBER_OF_ENTRIES_IN_DSCP_TO_TCI_TABLE_REGISTER  ( 8 )

static  DRV_IH_TARGET_MATRIX_PER_SP_CONFIG trgt_mtrx_sp_shadow[DRV_IH_TARGET_MATRIX_NUMBER_OF_DESTINATION_PORTS] = {};


/******************************************************************************/
/*                                                                            */
/* Macros definitions                                                         */
/*                                                                            */
/******************************************************************************/

/* gets bit #i from a given number */
#define MS_GET_BIT_I( number , i )   ( ( ( 1 << ( i ) ) & ( number ) ) >> ( i ) )
/* sets bit #i of a given number to a given value */
#define MS_SET_BIT_I( number , i , bit_value )   ( ( number ) &= ( ~ ( 1 << ( i ) ) )  , ( number ) |= ( ( bit_value ) << ( i ) ) )


/* the following macros are for accessing, using the same macro, a register
   in a sequence of several identical registers. these macros use arrays of
   addresses of the corresponding registers. it is needed since the offset
   betweeen the registers is not uniform */

uint32_t gs_lookup_configuration_lkup_tbl_lut_cfg_address [ DRV_IH_NUMBER_OF_LOOKUP_TABLES ] =
{
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_LUT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL1_LUT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL2_LUT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL3_LUT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL4_LUT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL5_LUT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL6_LUT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL7_LUT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL8_LUT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL9_LUT_CFG_ADDRESS
} ;


uint32_t gs_lookup_configuration_lkup_tbl_cam_cfg_address [ DRV_IH_NUMBER_OF_LOOKUP_TABLES ] =
{
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_CAM_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL1_CAM_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL2_CAM_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL3_CAM_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL4_CAM_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL5_CAM_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL6_CAM_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL7_CAM_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL8_CAM_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL9_CAM_CFG_ADDRESS
} ;



uint32_t gs_lookup_configuration_lkup_tbl_lut_cnxt_cfg_address [ DRV_IH_NUMBER_OF_LOOKUP_TABLES ] =
{
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_LUT_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL1_LUT_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL2_LUT_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL3_LUT_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL4_LUT_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL5_LUT_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL6_LUT_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL7_LUT_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL8_LUT_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL9_LUT_CNXT_CFG_ADDRESS
} ;



uint32_t gs_lookup_configuration_lkup_tbl_cam_cnxt_cfg_address [ DRV_IH_NUMBER_OF_LOOKUP_TABLES ] =
{
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_CAM_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL1_CAM_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL2_CAM_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL3_CAM_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL4_CAM_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL5_CAM_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL6_CAM_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL7_CAM_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL8_CAM_CNXT_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL9_CAM_CNXT_CFG_ADDRESS
} ;



uint32_t gs_lookup_configuration_lkup_tbl_key_cfg_address [ DRV_IH_NUMBER_OF_LOOKUP_TABLES ] =
{
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL1_KEY_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL2_KEY_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL3_KEY_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL4_KEY_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL5_KEY_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL6_KEY_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL7_KEY_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL8_KEY_CFG_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL9_KEY_CFG_ADDRESS
} ;


uint32_t gs_lookup_configuration_lkup_tbl_key_p0_maskl_address [ DRV_IH_NUMBER_OF_LOOKUP_TABLES ] =
{
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_P0_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL1_KEY_P0_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL2_KEY_P0_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL3_KEY_P0_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL4_KEY_P0_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL5_KEY_P0_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL6_KEY_P0_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL7_KEY_P0_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL8_KEY_P0_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL9_KEY_P0_MASKL_ADDRESS
} ;


uint32_t gs_lookup_configuration_lkup_tbl_key_p0_maskh_address [ DRV_IH_NUMBER_OF_LOOKUP_TABLES ] =
{
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_P0_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL1_KEY_P0_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL2_KEY_P0_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL3_KEY_P0_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL4_KEY_P0_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL5_KEY_P0_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL6_KEY_P0_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL7_KEY_P0_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL8_KEY_P0_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL9_KEY_P0_MASKH_ADDRESS
} ;


uint32_t gs_lookup_configuration_lkup_tbl_key_p1_maskl_address [ DRV_IH_NUMBER_OF_LOOKUP_TABLES ] =
{
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_P1_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL1_KEY_P1_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL2_KEY_P1_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL3_KEY_P1_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL4_KEY_P1_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL5_KEY_P1_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL6_KEY_P1_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL7_KEY_P1_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL8_KEY_P1_MASKL_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL9_KEY_P1_MASKL_ADDRESS
} ;


uint32_t gs_lookup_configuration_lkup_tbl_key_p1_maskh_address [ DRV_IH_NUMBER_OF_LOOKUP_TABLES ] =
{
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_P1_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL1_KEY_P1_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL2_KEY_P1_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL3_KEY_P1_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL4_KEY_P1_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL5_KEY_P1_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL6_KEY_P1_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL7_KEY_P1_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL8_KEY_P1_MASKH_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL9_KEY_P1_MASKH_ADDRESS
} ;



uint32_t gs_lookup_configuration_lkup_tbl_gl_mask_address [ DRV_IH_NUMBER_OF_LOOKUP_TABLES ] =
{
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_GL_MASK_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL1_GL_MASK_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL2_GL_MASK_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL3_GL_MASK_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL4_GL_MASK_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL5_GL_MASK_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL6_GL_MASK_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL7_GL_MASK_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL8_GL_MASK_ADDRESS ,
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL9_GL_MASK_ADDRESS
} ;


uint32_t gs_general_configuration_ih_class_search_cfg_address [ DRV_IH_NUMBER_OF_CLASSES ] =
{
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS0_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS1_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS2_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS3_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS4_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS5_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS6_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS7_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS8_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS9_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS10_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS11_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS12_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS13_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS14_SEARCH_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS15_SEARCH_CFG_ADDRESS
} ;


uint32_t gs_general_configuration_ih_class_general_cfg_address [ DRV_IH_NUMBER_OF_CLASSES ] =
{
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS0_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS1_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS2_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS3_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS4_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS5_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS6_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS7_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS8_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS9_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS10_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS11_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS12_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS13_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS14_GENERAL_CFG_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS15_GENERAL_CFG_ADDRESS
} ;


uint32_t gs_general_configuration_ih_class_key_address [ DRV_IH_NUMBER_OF_CLASSIFIERS ] =
{
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY0_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY1_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY2_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY3_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY4_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY5_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY6_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY7_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY8_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY9_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY10_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY11_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY12_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY13_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY14_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY15_ADDRESS
} ;


uint32_t gs_general_configuration_ih_class_mask_address [ DRV_IH_NUMBER_OF_CLASSIFIERS ] =
{
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK0_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK1_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK2_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK3_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK4_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK5_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK6_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK7_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK8_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK9_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK10_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK11_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK12_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK13_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK14_ADDRESS ,
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK15_ADDRESS
} ;


uint32_t gs_parser_core_configuration_ip_filter_cfg_address [ DRV_IH_NUMBER_OF_IP_FILTERS ] =
{
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTER2_CFG_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTER3_CFG_ADDRESS
} ;


uint32_t gs_parser_core_configuration_ip_filter_mask_cfg_address [ DRV_IH_NUMBER_OF_IP_FILTERS ] =
{
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTER2_MASK_CFG_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTER3_MASK_CFG_ADDRESS
} ;


uint32_t gs_parser_core_configuration_dscp2tci_tbl0_r_address [ CS_NUMBER_OF_REGISTERS_OF_DSCP_TO_TCI_TABLE ] =
{
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R0_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R1_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R2_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R3_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R4_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R5_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R6_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R7_ADDRESS
} ;


uint32_t gs_parser_core_configuration_dscp2tci_tbl1_r_address [ CS_NUMBER_OF_REGISTERS_OF_DSCP_TO_TCI_TABLE ] =
{
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL1_R0_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL1_R1_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL1_R2_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL1_R3_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL1_R4_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL1_R5_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL1_R6_ADDRESS ,
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL1_R7_ADDRESS
} ;


/*** Lookup table configuration ***/

#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TLUT_CFG_READ_I( r , i )                  READ_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_lut_cfg_address [ i ] ) , (r) )
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TLUT_CFG_WRITE_I( v , i )                 WRITE_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_lut_cfg_address [ i ] ) , (v) )
                                                                                         
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TCAM_CFG_READ_I( r , i )                  READ_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_cam_cfg_address [ i ] ) , (r) )
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TCAM_CFG_WRITE_I( v , i )                 WRITE_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_cam_cfg_address [ i ] ) , (v) )
                                                                                         
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TLUT_CNXT_CFG_READ_I( r , i )             READ_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_lut_cnxt_cfg_address [ i ] ) , (r) )
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TLUT_CNXT_CFG_WRITE_I( v , i )            WRITE_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_lut_cnxt_cfg_address [ i ] ) , (v) )

#define MS_DRV_IH_LOOKUP_LOOKUP_CONFIGURATION_LKUP_TCAM_CNXT_CFG_READ_I( r , i )      READ_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_cam_cnxt_cfg_address [ i ] ) , (r) )
#define MS_DRV_IH_LOOKUP_LOOKUP_CONFIGURATION_LKUP_TCAM_CNXT_CFG_WRITE_I( v , i )     WRITE_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_cam_cnxt_cfg_address [ i ] ) , (v) )

#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_CFG_READ_I( r , i )                  READ_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_key_cfg_address [ i ] ) , (r) )
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_CFG_WRITE_I( v , i )                 WRITE_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_key_cfg_address [ i ] ) , (v) )

#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P0_MASKL_READ_I( r , i )             READ_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_key_p0_maskl_address [ i ] ) , (r) )
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P0_MASKL_WRITE_I( v , i )            WRITE_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_key_p0_maskl_address [ i ] ) , (v) )
                                                                                         
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P0_MASKH_READ_I( r , i )             READ_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_key_p0_maskh_address [ i ] ) , (r) )
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P0_MASKH_WRITE_I( v , i )            WRITE_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_key_p0_maskh_address [ i ] ) , (v) )
                                                                                         
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P1_MASKL_READ_I( r , i )             READ_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_key_p1_maskl_address [ i ] ) , (r) )
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P1_MASKL_WRITE_I( v , i )            WRITE_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_key_p1_maskl_address [ i ] ) , (v) )
                                                                                         
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P1_MASKH_READ_I( r , i )             READ_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_key_p1_maskh_address [ i ] ) , (r) )
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P1_MASKH_WRITE_I( v , i )            WRITE_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_key_p1_maskh_address [ i ] ) , (v) )

#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TBL0_GL_MASK_READ_I( r , i )                 READ_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_gl_mask_address [ i ] ) , (r) )
#define MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TBL0_GL_MASK_WRITE_I( v , i )                WRITE_32( DEVICE_ADDRESS( gs_lookup_configuration_lkup_tbl_gl_mask_address [ i ] ) , (v) )


/*** Class configuration ***/

#define MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_SEARCH_CFG_READ_I( r , i )              READ_32( DEVICE_ADDRESS( gs_general_configuration_ih_class_search_cfg_address [ i ] ) , (r) )
#define MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_SEARCH_CFG_WRITE_I( v , i )             WRITE_32( DEVICE_ADDRESS( gs_general_configuration_ih_class_search_cfg_address [ i ] ) , (v) )

#define MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_GENERAL_CFG_READ_I( r , i )             READ_32( DEVICE_ADDRESS( gs_general_configuration_ih_class_general_cfg_address [ i ] ) , (r) )
#define MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_GENERAL_CFG_WRITE_I( v , i )            WRITE_32( DEVICE_ADDRESS( gs_general_configuration_ih_class_general_cfg_address [ i ] ) , (v) )

/*** Classifier configuration ***/

#define MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_KEY_READ_I( r , i )                     READ_32( DEVICE_ADDRESS( gs_general_configuration_ih_class_key_address [ i ] ) , (r) )
#define MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_KEY_WRITE_I( v , i )                    WRITE_32( DEVICE_ADDRESS( gs_general_configuration_ih_class_key_address [ i ] ) , (v) )

#define MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_MASK_READ_I( r , i )                    READ_32( DEVICE_ADDRESS( gs_general_configuration_ih_class_mask_address [ i ] ) , (r) )
#define MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_MASK_WRITE_I( v , i )                   WRITE_32( DEVICE_ADDRESS( gs_general_configuration_ih_class_mask_address [ i ] ) , (v) )

/*** IP filters configuration ***/

#define MS_DRV_IH_PARSER_CORE_CONFIGURATION_IP_FILTER_CFG_READ_I( r , i )                READ_32( DEVICE_ADDRESS( gs_parser_core_configuration_ip_filter_cfg_address [ i ] ) , (r) )
#define MS_DRV_IH_PARSER_CORE_CONFIGURATION_IP_FILTER_CFG_WRITE_I( v , i )               WRITE_32( DEVICE_ADDRESS( gs_parser_core_configuration_ip_filter_cfg_address [ i ] ) , (v) )

#define MS_DRV_IH_PARSER_CORE_CONFIGURATION_IP_FILTER_MASK_CFG_READ_I( r , i )           READ_32( DEVICE_ADDRESS( gs_parser_core_configuration_ip_filter_mask_cfg_address [ i ] ) , (r) )
#define MS_DRV_IH_PARSER_CORE_CONFIGURATION_IP_FILTER_MASK_CFG_WRITE_I( v , i )          WRITE_32( DEVICE_ADDRESS( gs_parser_core_configuration_ip_filter_mask_cfg_address [ i ] ) , (v) )

/*** DSCP to TCI table configuration ***/

#define MS_DRV_IH_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R_READ_I( r , i )              READ_32( DEVICE_ADDRESS( gs_parser_core_configuration_dscp2tci_tbl0_r_address [ i ] ) , (r) )
#define MS_DRV_IH_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R_WRITE_I( v , i )             WRITE_32( DEVICE_ADDRESS( gs_parser_core_configuration_dscp2tci_tbl0_r_address [ i ] ) , (v) )

#define MS_DRV_IH_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL1_R_READ_I( r , i )              READ_32( DEVICE_ADDRESS( gs_parser_core_configuration_dscp2tci_tbl1_r_address [ i ] ) , (r) )
#define MS_DRV_IH_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL1_R_WRITE_I( v , i )             WRITE_32( DEVICE_ADDRESS( gs_parser_core_configuration_dscp2tci_tbl1_r_address [ i ] ) , (v) )


/******************************************************************************/
/* Common memory section                                                      */
/* There are 2 sections in the runners common memory: A and B.                */
/******************************************************************************/
typedef enum
{
    CS_COMMON_MEMORY_SECTION_A ,
    CS_COMMON_MEMORY_SECTION_B
}
DRV_IH_COMMON_MEMORY_SECTION_DTS ;


/******************************************************************************/
/*                                                                            */
/* Global variables definitions                                               */
/*                                                                            */
/******************************************************************************/

int32_t gs_class_is_configured [ DRV_IH_NUMBER_OF_CLASSES ] = { 0 } ;
int32_t gs_classifier_is_configured [ DRV_IH_NUMBER_OF_CLASSIFIERS ] = { 0 } ;


/******************************************************************************/
/*                                                                            */
/* Functions prototypes                                                       */
/*                                                                            */
/******************************************************************************/

int32_t f_check_item_index( uint32_t xi_item_index ,
                                    uint32_t xi_number_of_items ) ;

DRV_IH_ERROR f_configure_lut_all_parameters ( uint8_t xi_table_index ,
                                                           const DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG * xi_lookup_table_60_bit_key_config ,
                                                           int32_t xi_five_tupple_enable ) ;


DRV_IH_ERROR fi_get_lut_all_parameters ( uint8_t xi_table_index ,
                                                      DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG * const xo_lookup_table_60_bit_key_config ,
                                                      int32_t * const xo_five_tupple_enable ) ;

DRV_IH_COMMON_MEMORY_SECTION_DTS f_get_lookup_table_location( uint8_t xi_table_index ) ;

int32_t f_verify_class_search_validity( DRV_IH_CLASS_SEARCH xi_class_search ,
                                                DRV_IH_COMMON_MEMORY_SECTION_DTS xi_desired_lookup_table_location ) ;

/* In 120 bit key lookup table, each key value actually occupies two table
   entries. therefore it is needed to multiply by 2 the maximal search depth
   and table size parameters obtained from the user. */
int32_t f_multiply_by_2_table_size ( DRV_IH_LOOKUP_TABLE_SIZE xi_table_size ,
                                             DRV_IH_LOOKUP_TABLE_SIZE * xo_multiplied_table_size ) ;
int32_t f_divide_by_2_table_size ( DRV_IH_LOOKUP_TABLE_SIZE xi_table_size ,
                                           DRV_IH_LOOKUP_TABLE_SIZE * xo_divided_table_size ) ;
int32_t f_multiply_by_2_maximal_search_depth ( DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH xi_maximal_search_depth ,
                                                       DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH * xo_multiplied_maximal_search_depth ) ;
int32_t f_divide_by_2_maximal_search_depth ( DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH xi_maximal_search_depth ,
                                                     DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH * xo_divided_maximal_search_depth ) ;

uint8_t f_translate_ingress_queue_priority ( uint8_t xi_priority_in_hw_format ) ;

void p_mac_address_array_to_hw_format ( uint8_t xi_mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ,
                                        uint32_t * xo_address_4_ls_bytes ,
                                        uint16_t * xo_addres_2_ms_bytes ) ;

void p_mac_address_hw_format_to_array ( uint32_t xi_address_4_ls_bytes ,
                                        uint16_t xi_addres_2_ms_bytes ,
                                        uint8_t xo_mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ) ;


/******************************************************************************/
/*                                                                            */
/* Init & cleanup module, license                                             */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* API functions implementations                                              */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_general_configuration                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set general configuration                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets general configuration of the IH block.                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ih_general_config - IH general configuration.                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_general_configuration ( const DRV_IH_GENERAL_CONFIG * xi_ih_general_config )
{
    IH_REGS_GENERAL_CONFIGURATION_RNRA_IHRSP_ADDR rnra_ihrsp_addr ;
    IH_REGS_GENERAL_CONFIGURATION_RNRB_IHRSP_ADDR rnrb_ihrsp_addr ;
    IH_REGS_GENERAL_CONFIGURATION_RNRA_CNGS_RPT_ADDR rnra_cngs_rpt_addr ;
    IH_REGS_GENERAL_CONFIGURATION_RNRB_CNGS_RPT_ADDR rnrb_cngs_rpt_addr ;
    IH_REGS_GENERAL_CONFIGURATION_RNR_CNGS_RPT_CFG rnr_cngs_rpt_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IH_MISC_CFG ih_misc_cfg ;


    IH_REGS_GENERAL_CONFIGURATION_RNRA_IHRSP_ADDR_READ( rnra_ihrsp_addr ) ;
    rnra_ihrsp_addr.rnra_ihrsp_addr = xi_ih_general_config->runner_a_ih_response_address ;
    IH_REGS_GENERAL_CONFIGURATION_RNRA_IHRSP_ADDR_WRITE( rnra_ihrsp_addr ) ;

    IH_REGS_GENERAL_CONFIGURATION_RNRB_IHRSP_ADDR_READ( rnrb_ihrsp_addr ) ;
    rnrb_ihrsp_addr.rnrb_ihrsp_addr = xi_ih_general_config->runner_b_ih_response_address ;
    IH_REGS_GENERAL_CONFIGURATION_RNRB_IHRSP_ADDR_WRITE( rnrb_ihrsp_addr ) ;

    IH_REGS_GENERAL_CONFIGURATION_RNRA_CNGS_RPT_ADDR_READ( rnra_cngs_rpt_addr ) ;
    rnra_cngs_rpt_addr.rnra_cngs_rpt_addr = xi_ih_general_config->runner_a_ih_congestion_report_address ;
    IH_REGS_GENERAL_CONFIGURATION_RNRA_CNGS_RPT_ADDR_WRITE( rnra_cngs_rpt_addr ) ;

    IH_REGS_GENERAL_CONFIGURATION_RNRB_CNGS_RPT_ADDR_READ( rnrb_cngs_rpt_addr ) ;
    rnrb_cngs_rpt_addr.rnrb_cngs_rpt_addr = xi_ih_general_config->runner_b_ih_congestion_report_address ;
    IH_REGS_GENERAL_CONFIGURATION_RNRB_CNGS_RPT_ADDR_WRITE( rnrb_cngs_rpt_addr ) ;

    IH_REGS_GENERAL_CONFIGURATION_RNR_CNGS_RPT_CFG_READ( rnr_cngs_rpt_cfg ) ;
    rnr_cngs_rpt_cfg.rnra_cngs_rpt_en = xi_ih_general_config->runner_a_ih_congestion_report_enable ;
    rnr_cngs_rpt_cfg.rnrb_cngs_rpt_en = xi_ih_general_config->runner_b_ih_congestion_report_enable ;
    IH_REGS_GENERAL_CONFIGURATION_RNR_CNGS_RPT_CFG_WRITE( rnr_cngs_rpt_cfg ) ;

    IH_REGS_GENERAL_CONFIGURATION_IH_MISC_CFG_READ( ih_misc_cfg ) ;
    ih_misc_cfg.lut_en_direct_mode = xi_ih_general_config->lut_searches_enable_in_direct_mode ;
    ih_misc_cfg.sn_stamp_dm_pkt = xi_ih_general_config->sn_stamping_enable_in_direct_mode ;
    ih_misc_cfg.hlength_min_trsh = xi_ih_general_config->header_length_minimum ;
    ih_misc_cfg.cngs_dscrd_dis = xi_ih_general_config->congestion_discard_disable ;
    ih_misc_cfg.nval_cam_search_en = xi_ih_general_config->cam_search_enable_upon_invalid_lut_entry ;
    IH_REGS_GENERAL_CONFIGURATION_IH_MISC_CFG_WRITE( ih_misc_cfg ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_bl_drv_ih_set_general_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_general_configuration                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get general configuration                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets general configuration of the IH block.                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_ih_general_config - IH general configuration.                         */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_general_configuration ( DRV_IH_GENERAL_CONFIG * const xo_ih_general_config )
{
    IH_REGS_GENERAL_CONFIGURATION_RNRA_IHRSP_ADDR rnra_ihrsp_addr ;
    IH_REGS_GENERAL_CONFIGURATION_RNRB_IHRSP_ADDR rnrb_ihrsp_addr ;
    IH_REGS_GENERAL_CONFIGURATION_RNRA_CNGS_RPT_ADDR rnra_cngs_rpt_addr ;
    IH_REGS_GENERAL_CONFIGURATION_RNRB_CNGS_RPT_ADDR rnrb_cngs_rpt_addr ;
    IH_REGS_GENERAL_CONFIGURATION_RNR_CNGS_RPT_CFG rnr_cngs_rpt_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IH_MISC_CFG ih_misc_cfg ;


    IH_REGS_GENERAL_CONFIGURATION_RNRA_IHRSP_ADDR_READ( rnra_ihrsp_addr ) ;
    xo_ih_general_config->runner_a_ih_response_address = rnra_ihrsp_addr.rnra_ihrsp_addr ;

    IH_REGS_GENERAL_CONFIGURATION_RNRB_IHRSP_ADDR_READ( rnrb_ihrsp_addr ) ;
    xo_ih_general_config->runner_b_ih_response_address = rnrb_ihrsp_addr.rnrb_ihrsp_addr ;

    IH_REGS_GENERAL_CONFIGURATION_RNRA_CNGS_RPT_ADDR_READ( rnra_cngs_rpt_addr ) ;
    xo_ih_general_config->runner_a_ih_congestion_report_address = rnra_cngs_rpt_addr.rnra_cngs_rpt_addr ;

    IH_REGS_GENERAL_CONFIGURATION_RNRB_CNGS_RPT_ADDR_READ( rnrb_cngs_rpt_addr ) ;
    xo_ih_general_config->runner_b_ih_congestion_report_address = rnrb_cngs_rpt_addr.rnrb_cngs_rpt_addr ;

    IH_REGS_GENERAL_CONFIGURATION_RNR_CNGS_RPT_CFG_READ( rnr_cngs_rpt_cfg ) ;
    xo_ih_general_config->runner_a_ih_congestion_report_enable = rnr_cngs_rpt_cfg.rnra_cngs_rpt_en ;
    xo_ih_general_config->runner_b_ih_congestion_report_enable = rnr_cngs_rpt_cfg.rnrb_cngs_rpt_en ;

    IH_REGS_GENERAL_CONFIGURATION_IH_MISC_CFG_READ( ih_misc_cfg ) ;
    xo_ih_general_config->lut_searches_enable_in_direct_mode = ih_misc_cfg.lut_en_direct_mode ;
    xo_ih_general_config->sn_stamping_enable_in_direct_mode = ih_misc_cfg.sn_stamp_dm_pkt ;
    xo_ih_general_config->header_length_minimum = ih_misc_cfg.hlength_min_trsh ;
    xo_ih_general_config->congestion_discard_disable = ih_misc_cfg.cngs_dscrd_dis ;
    xo_ih_general_config->cam_search_enable_upon_invalid_lut_entry = ih_misc_cfg.nval_cam_search_en ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_bl_drv_ih_get_general_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_packet_header_offsets                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set packet header offsets                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets packet header offset for each physical port.          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_packet_header_offsets - Packet header offsets                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_packet_header_offsets ( const DRV_IH_PACKET_HEADER_OFFSETS * xi_packet_header_offsets )
{
    IH_REGS_GENERAL_CONFIGURATION_PHL_OFFSET_CFG phl_offset_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_PHH_OFFSET_CFG phh_offset_cfg ;


    IH_REGS_GENERAL_CONFIGURATION_PHL_OFFSET_CFG_READ( phl_offset_cfg ) ;
    phl_offset_cfg.eth0_ph_offset = xi_packet_header_offsets->eth0_packet_header_offset ;
    phl_offset_cfg.eth1_ph_offset = xi_packet_header_offsets->eth1_packet_header_offset ;
    phl_offset_cfg.eth2_ph_offset = xi_packet_header_offsets->eth2_packet_header_offset ;
    phl_offset_cfg.eth3_ph_offset = xi_packet_header_offsets->eth3_packet_header_offset ;
    IH_REGS_GENERAL_CONFIGURATION_PHL_OFFSET_CFG_WRITE( phl_offset_cfg ) ;

    IH_REGS_GENERAL_CONFIGURATION_PHH_OFFSET_CFG_READ( phh_offset_cfg ) ;
    phh_offset_cfg.eth4_ph_offset = xi_packet_header_offsets->eth4_packet_header_offset ;
    phh_offset_cfg.gpon_ph_offset = xi_packet_header_offsets->gpon_packet_header_offset ;
    phh_offset_cfg.rnra_ph_offset = xi_packet_header_offsets->runner_a_packet_header_offset ;
    phh_offset_cfg.rnrb_ph_offset = xi_packet_header_offsets->runner_b_packet_header_offset ;
    IH_REGS_GENERAL_CONFIGURATION_PHH_OFFSET_CFG_WRITE( phh_offset_cfg ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_bl_drv_ih_set_packet_header_offsets ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_packet_header_offsets                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get packet header offsets                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets packet header offset for each physical port.          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_packet_header_offsets - Packet header offsets                         */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_packet_header_offsets ( DRV_IH_PACKET_HEADER_OFFSETS * const xo_packet_header_offsets )
{
    IH_REGS_GENERAL_CONFIGURATION_PHL_OFFSET_CFG phl_offset_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_PHH_OFFSET_CFG phh_offset_cfg ;


    IH_REGS_GENERAL_CONFIGURATION_PHL_OFFSET_CFG_READ( phl_offset_cfg ) ;
    xo_packet_header_offsets->eth0_packet_header_offset = phl_offset_cfg.eth0_ph_offset ;
    xo_packet_header_offsets->eth1_packet_header_offset = phl_offset_cfg.eth1_ph_offset ;
    xo_packet_header_offsets->eth2_packet_header_offset = phl_offset_cfg.eth2_ph_offset ;
    xo_packet_header_offsets->eth3_packet_header_offset = phl_offset_cfg.eth3_ph_offset ;

    IH_REGS_GENERAL_CONFIGURATION_PHH_OFFSET_CFG_READ( phh_offset_cfg ) ;
    xo_packet_header_offsets->eth4_packet_header_offset = phh_offset_cfg.eth4_ph_offset ;
    xo_packet_header_offsets->gpon_packet_header_offset = phh_offset_cfg.gpon_ph_offset ;
    xo_packet_header_offsets->runner_a_packet_header_offset = phh_offset_cfg.rnra_ph_offset ;
    xo_packet_header_offsets->runner_b_packet_header_offset = phh_offset_cfg.rnrb_ph_offset ;
    
    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_bl_drv_ih_get_packet_header_offsets ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_runner_buffers_configuration                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Runner Buffers configuration                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets runner-buffers related configuration, for each        */
/*   runner.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_runner_buffers_config - Runner Buffers Configuration                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_runner_buffers_configuration ( const DRV_IH_RUNNER_BUFFERS_CONFIG * xi_runner_buffers_config )
{
    IH_REGS_GENERAL_CONFIGURATION_RNRA_RB_BASE rnra_rb_base ;
    IH_REGS_GENERAL_CONFIGURATION_RNRB_RB_BASE rnrb_rb_base ;
    IH_REGS_GENERAL_CONFIGURATION_RBPM_BAT_CFG rbpm_bat_cfg ;


    IH_REGS_GENERAL_CONFIGURATION_RNRA_RB_BASE_READ( rnra_rb_base ) ;
    rnra_rb_base.rnra_common_rb_base = xi_runner_buffers_config->runner_a_ih_managed_rb_base_address ;
    rnra_rb_base.rnra_asigned_rb_base = xi_runner_buffers_config->runner_a_runner_managed_rb_base_address ;
    IH_REGS_GENERAL_CONFIGURATION_RNRA_RB_BASE_WRITE( rnra_rb_base ) ;

    IH_REGS_GENERAL_CONFIGURATION_RNRB_RB_BASE_READ( rnrb_rb_base ) ;
    rnrb_rb_base.rnrb_common_rb_base = xi_runner_buffers_config->runner_b_ih_managed_rb_base_address ;
    rnrb_rb_base.rnrb_asigned_rb_base = xi_runner_buffers_config->runner_b_runner_managed_rb_base_address ;
    IH_REGS_GENERAL_CONFIGURATION_RNRB_RB_BASE_WRITE( rnrb_rb_base ) ;

    IH_REGS_GENERAL_CONFIGURATION_RBPM_BAT_CFG_READ( rbpm_bat_cfg ) ;
    rbpm_bat_cfg.rnra_bpm_bat = xi_runner_buffers_config->runner_a_maximal_number_of_buffers ;
    rbpm_bat_cfg.rnrb_bpm_bat = xi_runner_buffers_config->runner_b_maximal_number_of_buffers ;
    IH_REGS_GENERAL_CONFIGURATION_RBPM_BAT_CFG_WRITE( rbpm_bat_cfg ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_bl_drv_ih_set_runner_buffers_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_runner_buffers_configuration                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Runner Buffers configuration                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets runner-buffers related configuration, for each        */
/*   runner.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_runner_buffers_config - Runner Buffers Configuration                  */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_runner_buffers_configuration ( DRV_IH_RUNNER_BUFFERS_CONFIG * const xo_runner_buffers_config )
{
    IH_REGS_GENERAL_CONFIGURATION_RNRA_RB_BASE rnra_rb_base ;
    IH_REGS_GENERAL_CONFIGURATION_RNRB_RB_BASE rnrb_rb_base ;
    IH_REGS_GENERAL_CONFIGURATION_RBPM_BAT_CFG rbpm_bat_cfg ;


    IH_REGS_GENERAL_CONFIGURATION_RNRA_RB_BASE_READ( rnra_rb_base ) ;
    xo_runner_buffers_config->runner_a_ih_managed_rb_base_address = rnra_rb_base.rnra_common_rb_base ;
    xo_runner_buffers_config->runner_a_runner_managed_rb_base_address = rnra_rb_base.rnra_asigned_rb_base ;

    IH_REGS_GENERAL_CONFIGURATION_RNRB_RB_BASE_READ( rnrb_rb_base ) ;
    xo_runner_buffers_config->runner_b_ih_managed_rb_base_address = rnrb_rb_base.rnrb_common_rb_base ;
    xo_runner_buffers_config->runner_b_runner_managed_rb_base_address = rnrb_rb_base.rnrb_asigned_rb_base ;
    
    IH_REGS_GENERAL_CONFIGURATION_RBPM_BAT_CFG_READ( rbpm_bat_cfg ) ;
    xo_runner_buffers_config->runner_a_maximal_number_of_buffers = rbpm_bat_cfg.rnra_bpm_bat ;
    xo_runner_buffers_config->runner_b_maximal_number_of_buffers = rbpm_bat_cfg.rnrb_bpm_bat ;
    
    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_bl_drv_ih_get_runner_buffers_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_runners_load_thresholds                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Runners Load Thresholds                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets thresholds related to runner load, for each runner.   */
/*   The thresholds are in number of occupied Runner Buffers.                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_runners_load_thresholds - Runners Load Thresholds (in number of       */
/*     occupied Runner Buffers)                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_runners_load_thresholds ( const DRV_IH_RUNNERS_LOAD_THRESHOLDS * xi_runners_load_thresholds )
{
    IH_REGS_GENERAL_CONFIGURATION_RNRA_CNGS_TRSH_CFG rnra_cngs_trsh_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_RNRB_CNGS_TRSH_CFG rnrb_cngs_trsh_cfg ;


    if ( xi_runners_load_thresholds->runner_a_high_congestion_threshold > DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS )
    {
        return ( DRV_IH_ERROR_INVALID_NUMBER_OF_RUNNER_BUFFERS ) ;
    }

    if ( xi_runners_load_thresholds->runner_b_high_congestion_threshold > DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS )
    {
        return ( DRV_IH_ERROR_INVALID_NUMBER_OF_RUNNER_BUFFERS ) ;
    }

    if ( xi_runners_load_thresholds->runner_a_exclusive_congestion_threshold > DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS )
    {
        return ( DRV_IH_ERROR_INVALID_NUMBER_OF_RUNNER_BUFFERS ) ;
    }

    if ( xi_runners_load_thresholds->runner_b_exclusive_congestion_threshold > DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS )
    {
        return ( DRV_IH_ERROR_INVALID_NUMBER_OF_RUNNER_BUFFERS ) ;
    }

    if ( xi_runners_load_thresholds->runner_a_load_balancing_threshold > DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS )
    {
        return ( DRV_IH_ERROR_INVALID_NUMBER_OF_RUNNER_BUFFERS ) ;
    }

    if ( xi_runners_load_thresholds->runner_b_load_balancing_threshold > DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS )
    {
        return ( DRV_IH_ERROR_INVALID_NUMBER_OF_RUNNER_BUFFERS ) ;
    }

    if ( xi_runners_load_thresholds->runner_a_load_balancing_hysteresis > DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS )
    {
        return ( DRV_IH_ERROR_INVALID_NUMBER_OF_RUNNER_BUFFERS ) ;
    }

    if ( xi_runners_load_thresholds->runner_b_load_balancing_hysteresis > DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS )
    {
        return ( DRV_IH_ERROR_INVALID_NUMBER_OF_RUNNER_BUFFERS ) ;
    }


    IH_REGS_GENERAL_CONFIGURATION_RNRA_CNGS_TRSH_CFG_READ( rnra_cngs_trsh_cfg ) ;
    rnra_cngs_trsh_cfg.high_cngs_trsh = xi_runners_load_thresholds->runner_a_high_congestion_threshold ;
    rnra_cngs_trsh_cfg.excl_cngs_trsh = xi_runners_load_thresholds->runner_a_exclusive_congestion_threshold ;
    rnra_cngs_trsh_cfg.lb_thsh = xi_runners_load_thresholds->runner_a_load_balancing_threshold ;
    rnra_cngs_trsh_cfg.lb_hyst = xi_runners_load_thresholds->runner_a_load_balancing_hysteresis ;
    IH_REGS_GENERAL_CONFIGURATION_RNRA_CNGS_TRSH_CFG_WRITE( rnra_cngs_trsh_cfg ) ;

    IH_REGS_GENERAL_CONFIGURATION_RNRB_CNGS_TRSH_CFG_READ( rnrb_cngs_trsh_cfg ) ;
    rnrb_cngs_trsh_cfg.high_cngs_trsh = xi_runners_load_thresholds->runner_b_high_congestion_threshold ;
    rnrb_cngs_trsh_cfg.excl_cngs_trsh = xi_runners_load_thresholds->runner_b_exclusive_congestion_threshold ;
    rnrb_cngs_trsh_cfg.lb_thsh = xi_runners_load_thresholds->runner_b_load_balancing_threshold ;
    rnrb_cngs_trsh_cfg.lb_hyst = xi_runners_load_thresholds->runner_b_load_balancing_hysteresis ;
    IH_REGS_GENERAL_CONFIGURATION_RNRB_CNGS_TRSH_CFG_WRITE( rnrb_cngs_trsh_cfg ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_bl_drv_ih_set_runners_load_thresholds ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_runners_load_thresholds                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Runners Load Thresholds                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets thresholds related to runner load, for each runner.   */
/*   The thresholds are in number of occupied Runner Buffers.                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_runners_load_thresholds - Runners Load Thresholds (in number of       */
/*     occupied Runner Buffers)                                               */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_runners_load_thresholds ( DRV_IH_RUNNERS_LOAD_THRESHOLDS * const xo_runners_load_thresholds )
{
    IH_REGS_GENERAL_CONFIGURATION_RNRA_CNGS_TRSH_CFG rnra_cngs_trsh_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_RNRB_CNGS_TRSH_CFG rnrb_cngs_trsh_cfg ;


    IH_REGS_GENERAL_CONFIGURATION_RNRA_CNGS_TRSH_CFG_READ( rnra_cngs_trsh_cfg ) ;
    xo_runners_load_thresholds->runner_a_high_congestion_threshold = rnra_cngs_trsh_cfg.high_cngs_trsh ;
    xo_runners_load_thresholds->runner_a_exclusive_congestion_threshold = rnra_cngs_trsh_cfg.excl_cngs_trsh ;
    xo_runners_load_thresholds->runner_a_load_balancing_threshold = rnra_cngs_trsh_cfg.lb_thsh ;
    xo_runners_load_thresholds->runner_a_load_balancing_hysteresis = rnra_cngs_trsh_cfg.lb_hyst ;

    IH_REGS_GENERAL_CONFIGURATION_RNRB_CNGS_TRSH_CFG_READ( rnrb_cngs_trsh_cfg ) ;
    xo_runners_load_thresholds->runner_b_high_congestion_threshold = rnrb_cngs_trsh_cfg.high_cngs_trsh ;
    xo_runners_load_thresholds->runner_b_exclusive_congestion_threshold = rnrb_cngs_trsh_cfg.excl_cngs_trsh ;
    xo_runners_load_thresholds->runner_b_load_balancing_threshold = rnrb_cngs_trsh_cfg.lb_thsh ;
    xo_runners_load_thresholds->runner_b_load_balancing_hysteresis = rnrb_cngs_trsh_cfg.lb_hyst ;
    
    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_bl_drv_ih_get_runners_load_thresholds ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_route_addresses                                         */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Route Addresses                                          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets route address for each physical port. The route       */
/*   address is used for broad-bus access for sending responses, message and  */
/*   data.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_route_addresses - Route Addresses                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_route_addresses ( const DRV_IH_ROUTE_ADDRESSES * xi_route_addresses )
{
    IH_REGS_GENERAL_CONFIGURATION_RADDR0_CFG raddr0_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_RADDR1_CFG raddr1_cfg ;


    IH_REGS_GENERAL_CONFIGURATION_RADDR0_CFG_READ( raddr0_cfg ) ;
    raddr0_cfg.eth0_raddr = xi_route_addresses->eth0_route_address ;
    raddr0_cfg.eth1_raddr = xi_route_addresses->eth1_route_address ;
    raddr0_cfg.eth2_raddr = xi_route_addresses->eth2_route_address ;
    raddr0_cfg.eth3_raddr = xi_route_addresses->eth3_route_address ;
    IH_REGS_GENERAL_CONFIGURATION_RADDR0_CFG_WRITE( raddr0_cfg ) ;

    IH_REGS_GENERAL_CONFIGURATION_RADDR1_CFG_READ( raddr1_cfg ) ;
    raddr1_cfg.eth4_raddr = xi_route_addresses->eth4_route_address ;
    raddr1_cfg.gpon_raddr = xi_route_addresses->gpon_route_address ;
    raddr1_cfg.rnra_raddr = xi_route_addresses->runner_a_route_address ;
    raddr1_cfg.rnrb_raddr = xi_route_addresses->runner_b_route_address ;
    IH_REGS_GENERAL_CONFIGURATION_RADDR1_CFG_WRITE( raddr1_cfg ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_bl_drv_ih_set_route_addresses ) ;

#ifndef _CFE_
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_route_addresses                                         */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Route Addresses                                          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets route address for each physical port. The route       */
/*   address is used for broad-bus access for sending responses, message and  */
/*   data.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_route_addresses - Route Addresses                                     */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_route_addresses ( DRV_IH_ROUTE_ADDRESSES * const xo_route_addresses )
{
    IH_REGS_GENERAL_CONFIGURATION_RADDR0_CFG raddr0_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_RADDR1_CFG raddr1_cfg ;


    IH_REGS_GENERAL_CONFIGURATION_RADDR0_CFG_READ( raddr0_cfg ) ;
    xo_route_addresses->eth0_route_address = raddr0_cfg.eth0_raddr ;
    xo_route_addresses->eth1_route_address = raddr0_cfg.eth1_raddr ;
    xo_route_addresses->eth2_route_address = raddr0_cfg.eth2_raddr ;
    xo_route_addresses->eth3_route_address = raddr0_cfg.eth3_raddr ;

    IH_REGS_GENERAL_CONFIGURATION_RADDR1_CFG_READ( raddr1_cfg ) ;
    xo_route_addresses->eth4_route_address = raddr1_cfg.eth4_raddr ;
    xo_route_addresses->gpon_route_address = raddr1_cfg.gpon_raddr ;
    xo_route_addresses->runner_a_route_address = raddr1_cfg.rnra_raddr ;
    xo_route_addresses->runner_b_route_address = raddr1_cfg.rnrb_raddr ;
    
    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_bl_drv_ih_get_route_addresses ) ;
#endif

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_logical_ports_configuration                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Logical Ports Configuration                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets configuration of the following logical ports:         */
/*   Ethernet 0-4, GPON, Runner A, Runner B and PCIE 0-1. The following       */
/*   parameters are configured per port: Parsing layer depth, Proprietary tag */
/*   size.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_logical_ports_config - Logical Ports Configuration                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_logical_ports_configuration ( const DRV_IH_LOGICAL_PORTS_CONFIG * xi_logical_ports_config )
{
    IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG parse_layer_per_port_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_PROP_SIZE_PER_PORT_CFG0 prop_size_per_port_cfg0 ;
    IH_REGS_GENERAL_CONFIGURATION_PROP_SIZE_PER_PORT_CFG1 prop_size_per_port_cfg1 ;


    IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_READ( parse_layer_per_port_cfg ) ;
    parse_layer_per_port_cfg.eth0_parse_layer_stg = xi_logical_ports_config->eth0_config.parsing_layer_depth ;
    parse_layer_per_port_cfg.eth1_parse_layer_stg = xi_logical_ports_config->eth1_config.parsing_layer_depth ;
    parse_layer_per_port_cfg.eth2_parse_layer_stg = xi_logical_ports_config->eth2_config.parsing_layer_depth ;
    parse_layer_per_port_cfg.eth3_parse_layer_stg = xi_logical_ports_config->eth3_config.parsing_layer_depth ;
    parse_layer_per_port_cfg.eth4_parse_layer_stg = xi_logical_ports_config->eth4_config.parsing_layer_depth ;
    parse_layer_per_port_cfg.gpon_parse_layer_stg = xi_logical_ports_config->gpon_config.parsing_layer_depth ;
    parse_layer_per_port_cfg.rnra_parse_layer_stg = xi_logical_ports_config->runner_a_config.parsing_layer_depth ;
    parse_layer_per_port_cfg.rnrb_parse_layer_stg = xi_logical_ports_config->runner_b_config.parsing_layer_depth ;
    parse_layer_per_port_cfg.pcie0_parse_layer_stg = xi_logical_ports_config->pcie0_config.parsing_layer_depth ;
    parse_layer_per_port_cfg.pcie1_parse_layer_stg = xi_logical_ports_config->pcie1_config.parsing_layer_depth ;
    IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_WRITE( parse_layer_per_port_cfg ) ;

    IH_REGS_GENERAL_CONFIGURATION_PROP_SIZE_PER_PORT_CFG0_READ( prop_size_per_port_cfg0 ) ;
    prop_size_per_port_cfg0.eth0_prop_tag_size = xi_logical_ports_config->eth0_config.proprietary_tag_size ;
    prop_size_per_port_cfg0.eth1_prop_tag_size = xi_logical_ports_config->eth1_config.proprietary_tag_size ;
    prop_size_per_port_cfg0.eth2_prop_tag_size = xi_logical_ports_config->eth2_config.proprietary_tag_size ;
    prop_size_per_port_cfg0.eth3_prop_tag_size = xi_logical_ports_config->eth3_config.proprietary_tag_size ;
    prop_size_per_port_cfg0.eth4_prop_tag_size = xi_logical_ports_config->eth4_config.proprietary_tag_size ;
    prop_size_per_port_cfg0.gpon_prop_tag_size = xi_logical_ports_config->gpon_config.proprietary_tag_size ;
    prop_size_per_port_cfg0.rnra_prop_tag_size = xi_logical_ports_config->runner_a_config.proprietary_tag_size ;
    prop_size_per_port_cfg0.rnrb_prop_tag_size = xi_logical_ports_config->runner_b_config.proprietary_tag_size ;
    IH_REGS_GENERAL_CONFIGURATION_PROP_SIZE_PER_PORT_CFG0_WRITE( prop_size_per_port_cfg0 ) ;

    IH_REGS_GENERAL_CONFIGURATION_PROP_SIZE_PER_PORT_CFG1_READ( prop_size_per_port_cfg1 ) ;
    prop_size_per_port_cfg1.pcie0_prop_tag_size = xi_logical_ports_config->pcie0_config.proprietary_tag_size ;
    prop_size_per_port_cfg1.pcie1_prop_tag_size = xi_logical_ports_config->pcie1_config.proprietary_tag_size ;
    IH_REGS_GENERAL_CONFIGURATION_PROP_SIZE_PER_PORT_CFG1_WRITE( prop_size_per_port_cfg1 ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_bl_drv_ih_set_logical_ports_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_logical_ports_configuration                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Logical Ports Configuration                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets configuration of the following logical ports:         */
/*   Ethernet 0-4, GPON, Runner A, Runner B and PCIE 0-1. The following       */
/*   parameters are configured per port: Parsing layer depth, Proprietary tag */
/*   size.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_logical_ports_config - Logical Ports Configuration                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_logical_ports_configuration ( DRV_IH_LOGICAL_PORTS_CONFIG * const xo_logical_ports_config )
{
    IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG parse_layer_per_port_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_PROP_SIZE_PER_PORT_CFG0 prop_size_per_port_cfg0 ;
    IH_REGS_GENERAL_CONFIGURATION_PROP_SIZE_PER_PORT_CFG1 prop_size_per_port_cfg1 ;


    IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_READ( parse_layer_per_port_cfg ) ;
    xo_logical_ports_config->eth0_config.parsing_layer_depth = parse_layer_per_port_cfg.eth0_parse_layer_stg ;
    xo_logical_ports_config->eth1_config.parsing_layer_depth = parse_layer_per_port_cfg.eth1_parse_layer_stg ;
    xo_logical_ports_config->eth2_config.parsing_layer_depth = parse_layer_per_port_cfg.eth2_parse_layer_stg ;
    xo_logical_ports_config->eth3_config.parsing_layer_depth = parse_layer_per_port_cfg.eth3_parse_layer_stg ;
    xo_logical_ports_config->eth4_config.parsing_layer_depth = parse_layer_per_port_cfg.eth4_parse_layer_stg ;
    xo_logical_ports_config->gpon_config.parsing_layer_depth = parse_layer_per_port_cfg.gpon_parse_layer_stg ;
    xo_logical_ports_config->runner_a_config.parsing_layer_depth = parse_layer_per_port_cfg.rnra_parse_layer_stg ;
    xo_logical_ports_config->runner_b_config.parsing_layer_depth = parse_layer_per_port_cfg.rnrb_parse_layer_stg ;
    xo_logical_ports_config->pcie0_config.parsing_layer_depth = parse_layer_per_port_cfg.pcie0_parse_layer_stg ;
    xo_logical_ports_config->pcie1_config.parsing_layer_depth = parse_layer_per_port_cfg.pcie1_parse_layer_stg ;
    
    IH_REGS_GENERAL_CONFIGURATION_PROP_SIZE_PER_PORT_CFG0_READ( prop_size_per_port_cfg0 ) ;
    xo_logical_ports_config->eth0_config.proprietary_tag_size = prop_size_per_port_cfg0.eth0_prop_tag_size ;
    xo_logical_ports_config->eth1_config.proprietary_tag_size = prop_size_per_port_cfg0.eth1_prop_tag_size ;
    xo_logical_ports_config->eth2_config.proprietary_tag_size = prop_size_per_port_cfg0.eth2_prop_tag_size ;
    xo_logical_ports_config->eth3_config.proprietary_tag_size = prop_size_per_port_cfg0.eth3_prop_tag_size ;
    xo_logical_ports_config->eth4_config.proprietary_tag_size = prop_size_per_port_cfg0.eth4_prop_tag_size ;
    xo_logical_ports_config->gpon_config.proprietary_tag_size = prop_size_per_port_cfg0.gpon_prop_tag_size ;
    xo_logical_ports_config->runner_a_config.proprietary_tag_size = prop_size_per_port_cfg0.rnra_prop_tag_size ;
    xo_logical_ports_config->runner_b_config.proprietary_tag_size = prop_size_per_port_cfg0.rnrb_prop_tag_size ;
    
    IH_REGS_GENERAL_CONFIGURATION_PROP_SIZE_PER_PORT_CFG1_READ( prop_size_per_port_cfg1 ) ;
    xo_logical_ports_config->pcie0_config.proprietary_tag_size = prop_size_per_port_cfg1.pcie0_prop_tag_size ;
    xo_logical_ports_config->pcie1_config.proprietary_tag_size = prop_size_per_port_cfg1.pcie1_prop_tag_size ;
    
    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_bl_drv_ih_get_logical_ports_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_lut_60_bit_key                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure Lookup Table 60 bit key                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures a lookup table with 60-bit key. There is a      */
/*   total of 10 tables. Note that when configuring a lookup table with       */
/*   120-bit key (with dedicated API), it occupies 2 tables. The lookup key   */
/*   is obtained by ORing two 60-bit parts taken from the parser result. Each */
/*   part has a configurable offset, and optionally a shift & rotate          */
/*   operation. Initially 64 bits are taken from the configured offset, then  */
/*   shift & rotate is optionally done, then the 4 MS-bits are omitted. Then  */
/*   each part is masked with its own mask. Then ORing the left 60 bits of    */
/*   the 2 parts yields the key. Then a key-extension can optionally be done, */
/*   i.e. ORing the MS-bits of the key with one of the following values: (1)  */
/*   5-bit Source Port from Header Descriptor. (2) 8-bit GEM Flow ID from     */
/*   Header Descriptor. (3) 1-bit WAN/LAN indication extracted from           */
/*   configuration of the source port. The global mask is applied on both key */
/*   & LUT entry when comparing between them. Move indication: When "Source   */
/*   port search enable" parameter is enabled, additional comparison will be  */
/*   done, between source-port (from Header descriptor) and bits 56:52 of LUT */
/*   entry, where source-port value should reside. In this case, the global   */
/*   mask must mask these bits for the regular comparison between the key and */
/*   LUT entry, which would be a MAC address comparison. If both comparisons  */
/*   match, the result would be "hit". If only MAC address comparison         */
/*   matches, the result would be "move".                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/*   xi_lookup_table_60_bit_key_config - Lookup table 60 bit key              */
/*     configuration                                                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_lut_60_bit_key ( uint8_t xi_table_index ,
                                                            const DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG * xi_lookup_table_60_bit_key_config )
{
    DRV_IH_ERROR error_code ;

    /* five_tupple_enable parameter is false here since it's a 60 bit key */
    error_code = f_configure_lut_all_parameters( xi_table_index ,
                                                 xi_lookup_table_60_bit_key_config ,
                                                 0 ) ;

    return ( error_code ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_configure_lut_60_bit_key ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_lut_60_bit_key_configuration                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Lookup Table 60 bit key configuration                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the configuration of Lookup Table 60 bit key.         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_lookup_table_60_bit_key_config - Lookup table 60 bit key              */
/*     configuration                                                          */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*     DRV_IH_ERROR_TABLE_IS_NOT_60_BIT_KEY - Table is not 60 bit key      */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_lut_60_bit_key_configuration ( uint8_t xi_table_index ,
                                                                    DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG * const xo_lookup_table_60_bit_key_config )
{
    DRV_IH_ERROR error_code ;
    int32_t five_tupple_enable ;

    error_code = fi_get_lut_all_parameters( xi_table_index ,
                                            xo_lookup_table_60_bit_key_config ,
                                            & five_tupple_enable ) ;
    if ( error_code != DRV_IH_NO_ERROR )
    {
        return ( error_code ) ;
    }
    if ( five_tupple_enable != 0 )
    {
        return ( DRV_IH_ERROR_TABLE_IS_NOT_60_BIT_KEY ) ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_lut_60_bit_key_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_lut_120_bit_key                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure Lookup Table 120 bit key                           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures a lookup table with 120-bit key. Configuring a  */
/*   lookup table with 120-bit key occupies 2 tables (out of total of 10):    */
/*   primary table and secondary table. The secondary table is used only for  */
/*   generation of the secondary key (see below). The primary table must be   */
/*   defined as search 1 or 3 of a class and the secondary must be 2 of 4     */
/*   respectively. Two 60 bit keys are defined per table: primary key and     */
/*   secondary key. Each one of these keys is generated the same way as       */
/*   explained in Configure Lookup Table 60 bit key API description. The 120  */
/*   bit key is composed of these two 60 bit keys. The 60 bit global mask is  */
/*   applied on secondary key only. SA search (for Move indication) is not    */
/*   supported for 120 bit key.                                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_primary_table_index - Primary table index                             */
/*                                                                            */
/*   xi_secondary_table_index - Secondary table index                         */
/*                                                                            */
/*   xi_lookup_table_120_bit_key_config - Lookup table 120 bit key            */
/*     configuration                                                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_lut_120_bit_key ( uint8_t xi_primary_table_index ,
                                                             uint8_t xi_secondary_table_index ,
                                                             const DRV_IH_LOOKUP_TABLE_120_BIT_KEY_CONFIG * xi_lookup_table_120_bit_key_config )
{
    DRV_IH_ERROR error_code ;
    DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG primary_table_config ;
    DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG secondary_table_config ;
    DRV_IH_LOOKUP_TABLE_SIZE multiplied_table_size ;
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH multiplied_maximal_search_depth ;
    int32_t result ;


    /* table_size and maximal_search_depth should be multiplied by 2, since each entry (in SW perspective) actually occupies
       2 entries (in HW perspective) */

    result = f_multiply_by_2_table_size( xi_lookup_table_120_bit_key_config->table_size ,
                                         & multiplied_table_size ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_TABLE_SIZE ) ;
    }

    result = f_multiply_by_2_maximal_search_depth( xi_lookup_table_120_bit_key_config->maximal_search_depth ,
                                                   & multiplied_maximal_search_depth ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_MAXIMAL_SEARCH_DEPTH ) ;
    }


    /* prepare configuration of primary table */

    primary_table_config.table_base_address_in_8_byte = xi_lookup_table_120_bit_key_config->table_base_address_in_8_byte ;
    primary_table_config.table_size = multiplied_table_size ;
    primary_table_config.maximal_search_depth = multiplied_maximal_search_depth ;
    primary_table_config.hash_type = xi_lookup_table_120_bit_key_config->hash_type ;
    /* SA search is not supported in 120 bit key table */
    primary_table_config.sa_search_enable = 0 ;
    primary_table_config.aging_enable = xi_lookup_table_120_bit_key_config->aging_enable ;
    primary_table_config.cam_enable = xi_lookup_table_120_bit_key_config->cam_enable;
    primary_table_config.cam_base_address_in_8_byte = xi_lookup_table_120_bit_key_config->cam_base_address_in_8_byte ;
    primary_table_config.context_table_base_address_in_8_byte = xi_lookup_table_120_bit_key_config->context_table_base_address_in_8_byte ;
    primary_table_config.context_table_entry_size = xi_lookup_table_120_bit_key_config->context_table_entry_size ;
    primary_table_config.cam_context_base_address_in_8_byte = xi_lookup_table_120_bit_key_config->cam_context_base_address_in_8_byte ;
    primary_table_config.part_0_start_offset_in_4_byte = xi_lookup_table_120_bit_key_config->primary_key_part_0_start_offset_in_4_byte ;
    primary_table_config.part_0_shift_offset_in_4_bit = xi_lookup_table_120_bit_key_config->primary_key_part_0_shift_offset_in_4_bit ;
    primary_table_config.part_1_start_offset_in_4_byte = xi_lookup_table_120_bit_key_config->primary_key_part_1_start_offset_in_4_byte ;
    primary_table_config.part_1_shift_offset_in_4_bit = xi_lookup_table_120_bit_key_config->primary_key_part_1_shift_offset_in_4_bit ;
    primary_table_config.key_extension = xi_lookup_table_120_bit_key_config->primary_key_extension ;
    primary_table_config.part_0_mask_low = xi_lookup_table_120_bit_key_config->primary_key_part_0_mask_low ;
    primary_table_config.part_0_mask_high = xi_lookup_table_120_bit_key_config->primary_key_part_0_mask_high ;
    primary_table_config.part_1_mask_low = xi_lookup_table_120_bit_key_config->primary_key_part_1_mask_low ;
    primary_table_config.part_1_mask_high = xi_lookup_table_120_bit_key_config->primary_key_part_1_mask_high ;
    primary_table_config.global_mask_in_4_bit = xi_lookup_table_120_bit_key_config->global_mask_in_4_bit ;


    /* prepare configuration of secondary table - only paramters related to key
       generation are relevant here (HW ingnores the others) */

    /* initialize to 0. the irrelevant parameters will stay 0 (anyway, HW ignores them) */
    memset (& secondary_table_config , 0 , sizeof ( secondary_table_config ) ) ;

    secondary_table_config.part_0_start_offset_in_4_byte = xi_lookup_table_120_bit_key_config->secondary_key_part_0_start_offset_in_4_byte ;
    secondary_table_config.part_0_shift_offset_in_4_bit = xi_lookup_table_120_bit_key_config->secondary_key_part_0_shift_offset_in_4_bit ;
    secondary_table_config.part_1_start_offset_in_4_byte = xi_lookup_table_120_bit_key_config->secondary_key_part_1_start_offset_in_4_byte ;
    secondary_table_config.part_1_shift_offset_in_4_bit = xi_lookup_table_120_bit_key_config->secondary_key_part_1_shift_offset_in_4_bit ;
    secondary_table_config.key_extension = xi_lookup_table_120_bit_key_config->secondary_key_extension ;
    secondary_table_config.part_0_mask_low = xi_lookup_table_120_bit_key_config->secondary_key_part_0_mask_low ;
    secondary_table_config.part_0_mask_high = xi_lookup_table_120_bit_key_config->secondary_key_part_0_mask_high ;
    secondary_table_config.part_1_mask_low = xi_lookup_table_120_bit_key_config->secondary_key_part_1_mask_low ;
    secondary_table_config.part_1_mask_high = xi_lookup_table_120_bit_key_config->secondary_key_part_1_mask_high ;

    /* configure the primary table. five_tupple_enable parameter is true here since it's a 120 bit key */
    error_code = f_configure_lut_all_parameters( xi_primary_table_index ,
                                                 & primary_table_config ,
                                                 1 ) ;
    if ( error_code != DRV_IH_NO_ERROR )
    {
        return ( error_code ) ;
    }

    /* configure the secondary table. five_tupple_enable parameter is set to false here due to VLSI request
       (actually this paramter is ignored by HW). */
    error_code = f_configure_lut_all_parameters( xi_secondary_table_index ,
                                                 & secondary_table_config ,
                                                 0 ) ;
    if ( error_code != DRV_IH_NO_ERROR )
    {
        return ( error_code ) ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_configure_lut_120_bit_key ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_lut_120_bit_key_configuration                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Lookup Table 120 bit key configuration                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the configuration of Lookup Table 120 bit key.        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_primary_table_index - Primary table index                             */
/*                                                                            */
/*   xi_secondary_table_index - Secondary table index                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_lookup_table_120_bit_key_config - Lookup table 120 bit key            */
/*     configuration                                                          */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*     DRV_IH_ERROR_TABLE_IS_NOT_120_BIT_KEY - Table is not 120 bit key    */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_lut_120_bit_key_configuration ( uint8_t xi_primary_table_index ,
                                                                     uint8_t xi_secondary_table_index ,
                                                                     DRV_IH_LOOKUP_TABLE_120_BIT_KEY_CONFIG * const xo_lookup_table_120_bit_key_config )
{
    DRV_IH_ERROR error_code ;
    DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG primary_table_config ;
    DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG secondary_table_config ;
    int32_t five_tupple_enable ;
    DRV_IH_LOOKUP_TABLE_SIZE divided_table_size ;
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH divided_maximal_search_depth ;
    int32_t result ;


    /* get primary table configuration */
    error_code = fi_get_lut_all_parameters( xi_primary_table_index ,
                                            & primary_table_config ,
                                            & five_tupple_enable ) ;
    if ( error_code != DRV_IH_NO_ERROR )
    {
        return ( error_code ) ;
    }
    if ( five_tupple_enable != 1 )
    {
        return ( DRV_IH_ERROR_TABLE_IS_NOT_120_BIT_KEY ) ;
    }

    /* get secondary table configuration */
    error_code = fi_get_lut_all_parameters( xi_secondary_table_index ,
                                            & secondary_table_config ,
                                            & five_tupple_enable ) ;
    if ( error_code != DRV_IH_NO_ERROR )
    {
        return ( error_code ) ;
    }


    /* table_size and maximal_search_depth should be divided by 2, since each entry (in SW perspective) actually occupies
       2 entries (in HW perspective). these value were multiplied by 2 when configured. */

    result = f_divide_by_2_table_size( primary_table_config.table_size ,
                                       & divided_table_size ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_TABLE_SIZE ) ;
    }

    result = f_divide_by_2_maximal_search_depth( primary_table_config.maximal_search_depth ,
                                                 & divided_maximal_search_depth ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_MAXIMAL_SEARCH_DEPTH ) ;
    }


    xo_lookup_table_120_bit_key_config->table_base_address_in_8_byte = primary_table_config.table_base_address_in_8_byte ;
    xo_lookup_table_120_bit_key_config->table_size = divided_table_size ;
    xo_lookup_table_120_bit_key_config->maximal_search_depth = divided_maximal_search_depth ;
    xo_lookup_table_120_bit_key_config->hash_type = primary_table_config.hash_type ;
    xo_lookup_table_120_bit_key_config->aging_enable = primary_table_config.aging_enable ;
    xo_lookup_table_120_bit_key_config->cam_enable= primary_table_config.cam_enable ;
    xo_lookup_table_120_bit_key_config->cam_base_address_in_8_byte = primary_table_config.cam_base_address_in_8_byte ;
    xo_lookup_table_120_bit_key_config->context_table_base_address_in_8_byte = primary_table_config.context_table_base_address_in_8_byte ;
    xo_lookup_table_120_bit_key_config->context_table_entry_size = primary_table_config.context_table_entry_size ;
    xo_lookup_table_120_bit_key_config->cam_context_base_address_in_8_byte = primary_table_config.cam_context_base_address_in_8_byte ;
    xo_lookup_table_120_bit_key_config->primary_key_part_0_start_offset_in_4_byte = primary_table_config.part_0_start_offset_in_4_byte ;
    xo_lookup_table_120_bit_key_config->primary_key_part_0_shift_offset_in_4_bit = primary_table_config.part_0_shift_offset_in_4_bit ;
    xo_lookup_table_120_bit_key_config->primary_key_part_1_start_offset_in_4_byte = primary_table_config.part_1_start_offset_in_4_byte ;
    xo_lookup_table_120_bit_key_config->primary_key_part_1_shift_offset_in_4_bit = primary_table_config.part_1_shift_offset_in_4_bit ;
    xo_lookup_table_120_bit_key_config->primary_key_extension = primary_table_config.key_extension ;
    xo_lookup_table_120_bit_key_config->primary_key_part_0_mask_low = primary_table_config.part_0_mask_low ;
    xo_lookup_table_120_bit_key_config->primary_key_part_0_mask_high = primary_table_config.part_0_mask_high ;
    xo_lookup_table_120_bit_key_config->primary_key_part_1_mask_low = primary_table_config.part_1_mask_low ;
    xo_lookup_table_120_bit_key_config->primary_key_part_1_mask_high = primary_table_config.part_1_mask_high ;
    xo_lookup_table_120_bit_key_config->global_mask_in_4_bit = primary_table_config.global_mask_in_4_bit ;

    xo_lookup_table_120_bit_key_config->secondary_key_part_0_start_offset_in_4_byte = secondary_table_config.part_0_start_offset_in_4_byte ;
    xo_lookup_table_120_bit_key_config->secondary_key_part_0_shift_offset_in_4_bit = secondary_table_config.part_0_shift_offset_in_4_bit ;
    xo_lookup_table_120_bit_key_config->secondary_key_part_1_start_offset_in_4_byte = secondary_table_config.part_1_start_offset_in_4_byte ;
    xo_lookup_table_120_bit_key_config->secondary_key_part_1_shift_offset_in_4_bit = secondary_table_config.part_1_shift_offset_in_4_bit ;
    xo_lookup_table_120_bit_key_config->secondary_key_extension = secondary_table_config.key_extension ;
    xo_lookup_table_120_bit_key_config->secondary_key_part_0_mask_low = secondary_table_config.part_0_mask_low ;
    xo_lookup_table_120_bit_key_config->secondary_key_part_0_mask_high = secondary_table_config.part_0_mask_high ;
    xo_lookup_table_120_bit_key_config->secondary_key_part_1_mask_low = secondary_table_config.part_1_mask_low ;
    xo_lookup_table_120_bit_key_config->secondary_key_part_1_mask_high = secondary_table_config.part_1_mask_high ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_lut_120_bit_key_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_class                                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure Class                                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures an IH class. Ingress handler class is type of   */
/*   ingress traffic, e.g. IPTV, bridged, routed. Each class includes         */
/*   predefined set of settings, such as: target runner, destination memory,  */
/*   QoS, definition of look-up searches. There are up to 16 classes. Each    */
/*   physical port has a default class (For GPON port, default class is per   */
/*   GEM flow). IH may override the default class according to                */
/*   enable-override configuration and to classification based on reduced     */
/*   Parser results, called "Classifier Key Word" (Parser Summary Word plus   */
/*   source port). Default classes and override-enable configurations are in  */
/*   BBH. In runner flow, runner assigns an initial class (which can be       */
/*   overridden by IH). Class override is done using classifiers, which are   */
/*   configured using Configure Classifier API.                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_class_index - Class index                                             */
/*                                                                            */
/*   xi_class_config - Class Configuration                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*     DRV_IH_ERROR_CLASS_SEARCH_AND_LUT_LOCATION_MISMATCH - mismatch      */
/*       between class search and lookup table location                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_class ( uint8_t xi_class_index ,
                                                   const DRV_IH_CLASS_CONFIG * xi_class_config )
{
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS0_SEARCH_CFG ih_class_search_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS0_GENERAL_CFG ih_class_general_cfg ;
    int32_t result ;

    result = f_check_item_index( xi_class_index ,
                                 DRV_IH_NUMBER_OF_CLASSES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    result = f_check_item_index( xi_class_config->dscp_to_tci_table_index ,
                                 DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    /* Tables used for searches 1 & 2 must be located at common memory A */

    result = f_verify_class_search_validity( xi_class_config->class_search_1 ,
                                             CS_COMMON_MEMORY_SECTION_A ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_CLASS_SEARCH_AND_LUT_LOCATION_MISMATCH ) ;
    }

    result = f_verify_class_search_validity( xi_class_config->class_search_2 ,
                                             CS_COMMON_MEMORY_SECTION_A ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_CLASS_SEARCH_AND_LUT_LOCATION_MISMATCH ) ;
    }

    /* Tables used for searches 3 & 4 must be located at common memory B */

    result = f_verify_class_search_validity( xi_class_config->class_search_3 ,
                                             CS_COMMON_MEMORY_SECTION_B ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_CLASS_SEARCH_AND_LUT_LOCATION_MISMATCH ) ;
    }

    result = f_verify_class_search_validity( xi_class_config->class_search_4 ,
                                             CS_COMMON_MEMORY_SECTION_B ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_CLASS_SEARCH_AND_LUT_LOCATION_MISMATCH ) ;
    }

    MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_SEARCH_CFG_READ_I( ih_class_search_cfg , xi_class_index ) ;
    ih_class_search_cfg.search1_lkup_tbl_ref = xi_class_config->class_search_1 ;
    ih_class_search_cfg.search2_lkup_tbl_ref = xi_class_config->class_search_2 ;
    ih_class_search_cfg.search3_lkup_tbl_ref = xi_class_config->class_search_3 ;
    ih_class_search_cfg.search4_lkup_tbl_ref = xi_class_config->class_search_4 ;
    ih_class_search_cfg.dp_extr_cfg = xi_class_config->destination_port_extraction ;
    ih_class_search_cfg.drop_on_miss_extr_cfg = xi_class_config->drop_on_miss ;
    ih_class_search_cfg.qos_extr_cfg = xi_class_config->ingress_qos_override ;
    MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_SEARCH_CFG_WRITE_I( ih_class_search_cfg , xi_class_index ) ;

    MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_GENERAL_CFG_READ_I( ih_class_general_cfg , xi_class_index ) ;
    ih_class_general_cfg.dscp2tci_trans_tbl = xi_class_config->dscp_to_tci_table_index ;
    ih_class_general_cfg.dm_default = xi_class_config->direct_mode_default ;
    ih_class_general_cfg.dm_override = xi_class_config->direct_mode_override ;
    ih_class_general_cfg.tm_default = xi_class_config->target_memory_default ;
    ih_class_general_cfg.tm_override = xi_class_config->target_memory_override ;
    ih_class_general_cfg.qos_default = xi_class_config->ingress_qos_default ;
    /* qos_override paramter is actually redundant (qos_extr_cfg is sufficient), so it is not exposed to the user */
    ih_class_general_cfg.qos_override = ( xi_class_config->ingress_qos_override == DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_OPERATION_DISABLED ? 0 : 1 ) ;
    ih_class_general_cfg.tr_default = xi_class_config->target_runner_default ;
    ih_class_general_cfg.rnr_ovr_dm = xi_class_config->target_runner_override_in_direct_mode ;
    ih_class_general_cfg.rnr_default_dm = xi_class_config->target_runner_for_direct_mode ;
    ih_class_general_cfg.lb_en = xi_class_config->load_balancing_enable ;
    ih_class_general_cfg.pref_lb_en = xi_class_config->preference_load_balancing_enable ;
    MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_GENERAL_CFG_WRITE_I( ih_class_general_cfg , xi_class_index ) ;

    gs_class_is_configured [ xi_class_index ] = 1 ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_configure_class ) ;

#ifndef _CFE_
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_class_configuration                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Class configuration                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the configuration of a Class.                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_class_index - Class index                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_class_config - Class Configuration                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_class_configuration ( uint8_t xi_class_index ,
                                                           DRV_IH_CLASS_CONFIG * const xo_class_config )
{
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS0_SEARCH_CFG ih_class_search_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS0_GENERAL_CFG ih_class_general_cfg ;
    int32_t result ;


    result = f_check_item_index( xi_class_index ,
                                 DRV_IH_NUMBER_OF_CLASSES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }


    MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_SEARCH_CFG_READ_I( ih_class_search_cfg , xi_class_index ) ;
    xo_class_config->class_search_1 = ih_class_search_cfg.search1_lkup_tbl_ref ;
    xo_class_config->class_search_2 = ih_class_search_cfg.search2_lkup_tbl_ref ;
    xo_class_config->class_search_3 = ih_class_search_cfg.search3_lkup_tbl_ref ;
    xo_class_config->class_search_4 = ih_class_search_cfg.search4_lkup_tbl_ref ;
    xo_class_config->destination_port_extraction = ih_class_search_cfg.dp_extr_cfg ;
    xo_class_config->drop_on_miss = ih_class_search_cfg.drop_on_miss_extr_cfg ;
    xo_class_config->ingress_qos_override = ih_class_search_cfg.qos_extr_cfg ;
    
    MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_GENERAL_CFG_READ_I( ih_class_general_cfg , xi_class_index ) ;
    xo_class_config->dscp_to_tci_table_index = ih_class_general_cfg.dscp2tci_trans_tbl ;
    xo_class_config->direct_mode_default = ih_class_general_cfg.dm_default ;
    xo_class_config->direct_mode_override = ih_class_general_cfg.dm_override ;
    xo_class_config->target_memory_default = ih_class_general_cfg.tm_default ;
    xo_class_config->target_memory_override = ih_class_general_cfg.tm_override ;
    xo_class_config->ingress_qos_default = ih_class_general_cfg.qos_default ;
    xo_class_config->target_runner_default = ih_class_general_cfg.tr_default ;
    xo_class_config->target_runner_override_in_direct_mode = ih_class_general_cfg.rnr_ovr_dm ;
    xo_class_config->target_runner_for_direct_mode = ih_class_general_cfg.rnr_default_dm ;
    xo_class_config->load_balancing_enable = ih_class_general_cfg.lb_en ;
    xo_class_config->preference_load_balancing_enable = ih_class_general_cfg.pref_lb_en ;
    
    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_class_configuration ) ;
#endif

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_classifier                                        */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure Classifier                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures a classifier. There are up to 16 classifiers. A */
/*   classifier is a pair of key and mask, and a resulting class. When class  */
/*   override is enabled, the key is compared to the masked Classifier Key    */
/*   Word (the mask is NOT applied on the key, so user is responsible to set  */
/*   0 at the masked fields/bits in the key). In case of match, the default   */
/*   class is overridden by the classifier's resulting class. If there is a   */
/*   match in more than one classifier, the one with the lower index is       */
/*   chosen.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_classifier_index - Classifier index                                   */
/*                                                                            */
/*   xi_classifier_config - Classifier configuration                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_classifier ( uint8_t xi_classifier_index ,
                                                        const DRV_IH_CLASSIFIER_CONFIG * xi_classifier_config )
{
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY0 ih_class_key ;
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK0 ih_class_mask ;
    IH_REGS_GENERAL_CONFIGURATION_IH_CLSF_MAPL_CFG ih_clsf_mapl_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IH_CLSF_MAPH_CFG ih_clsf_maph_cfg ;
    int32_t result ;


    result = f_check_item_index( xi_classifier_index ,
                                 DRV_IH_NUMBER_OF_CLASSIFIERS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }


    MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_KEY_READ_I( ih_class_key , xi_classifier_index ) ;
    ih_class_key.ih_class_key_l2 = xi_classifier_config->l2_protocol ;
    ih_class_key.ih_class_key_l3 = xi_classifier_config->l3_protocol ;
    ih_class_key.ih_class_key_l4 = xi_classifier_config->l4_protocol ;
    ih_class_key.ih_class_key_da_anyhit = xi_classifier_config->da_filter_any_hit ;
    ih_class_key.ih_class_key_da_fltr = xi_classifier_config->matched_da_filter ;
    ih_class_key.ih_class_key_mc_fltr = xi_classifier_config->multicast_da_indication ;
    ih_class_key.ih_class_key_bc_fltr = xi_classifier_config->broadcast_da_indication ;
    ih_class_key.ih_class_key_vid_anyhit = xi_classifier_config->vid_filter_any_hit ;
    ih_class_key.ih_class_key_vid_fltr = xi_classifier_config->matched_vid_filter ;
    ih_class_key.ih_class_key_ip_anyhit = xi_classifier_config->ip_filter_any_hit ;
    ih_class_key.ih_class_key_ip_fltr = xi_classifier_config->matched_ip_filter ;
    ih_class_key.ih_class_key_wan_fltr = xi_classifier_config->wan_indication ;
    ih_class_key.ih_class_key_5tpl_fltr = xi_classifier_config->five_tuple_valid ;
    ih_class_key.ih_class_key_sp = xi_classifier_config->logical_source_port ;
    ih_class_key.ih_class_key_err = xi_classifier_config->error ;
    MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_KEY_WRITE_I( ih_class_key , xi_classifier_index ) ;

    MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_MASK_READ_I( ih_class_mask , xi_classifier_index ) ;
    ih_class_mask.ih_class_mask = xi_classifier_config->mask ;
    MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_MASK_WRITE_I( ih_class_mask , xi_classifier_index ) ;

    /* classifiers 0-7 are stored in the "low" register.
       classifiers 8-15 are stored in the "high" register. */
    if ( xi_classifier_index < CS_NUMBER_OF_CLASSIFIERS_IN_CLASSIFIER_TO_CLASS_MAPPING_REGISTERS )
    {
        IH_REGS_GENERAL_CONFIGURATION_IH_CLSF_MAPL_CFG_READ( ih_clsf_mapl_cfg ) ;

        switch ( xi_classifier_index )
        {
        case 0:
            ih_clsf_mapl_cfg.clsf_set0_map = xi_classifier_config->resulting_class ;
            break ;
        case 1:
            ih_clsf_mapl_cfg.clsf_set1_map = xi_classifier_config->resulting_class ;
            break ;
        case 2:
            ih_clsf_mapl_cfg.clsf_set2_map = xi_classifier_config->resulting_class ;
            break ;
        case 3:
            ih_clsf_mapl_cfg.clsf_set3_map = xi_classifier_config->resulting_class ;
            break ;
        case 4:
            ih_clsf_mapl_cfg.clsf_set4_map = xi_classifier_config->resulting_class ;
            break ;
        case 5:
            ih_clsf_mapl_cfg.clsf_set5_map = xi_classifier_config->resulting_class ;
            break ;
        case 6:
            ih_clsf_mapl_cfg.clsf_set6_map = xi_classifier_config->resulting_class ;
            break ;
        case 7:
            ih_clsf_mapl_cfg.clsf_set7_map = xi_classifier_config->resulting_class ;
            break ;
        }

        IH_REGS_GENERAL_CONFIGURATION_IH_CLSF_MAPL_CFG_WRITE( ih_clsf_mapl_cfg ) ;
    }
    else
    {
        IH_REGS_GENERAL_CONFIGURATION_IH_CLSF_MAPH_CFG_READ( ih_clsf_maph_cfg ) ;

        switch ( xi_classifier_index )
        {
        case 8:
            ih_clsf_maph_cfg.clsf_set8_map = xi_classifier_config->resulting_class ;
            break ;
        case 9:
            ih_clsf_maph_cfg.clsf_set9_map = xi_classifier_config->resulting_class ;
            break ;
        case 10:
            ih_clsf_maph_cfg.clsf_set10_map = xi_classifier_config->resulting_class ;
            break ;
        case 11:
            ih_clsf_maph_cfg.clsf_set11_map = xi_classifier_config->resulting_class ;
            break ;
        case 12:
            ih_clsf_maph_cfg.clsf_set12_map = xi_classifier_config->resulting_class ;
            break ;
        case 13:
            ih_clsf_maph_cfg.clsf_set13_map = xi_classifier_config->resulting_class ;
            break ;
        case 14:
            ih_clsf_maph_cfg.clsf_set14_map = xi_classifier_config->resulting_class ;
            break ;
        case 15:
            ih_clsf_maph_cfg.clsf_set15_map = xi_classifier_config->resulting_class ;
            break ;
        }

        IH_REGS_GENERAL_CONFIGURATION_IH_CLSF_MAPH_CFG_WRITE( ih_clsf_maph_cfg ) ;
    }


    gs_classifier_is_configured [ xi_classifier_index ] = 1 ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_configure_classifier ) ;

#ifndef _CFE_
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_classifier_configuration                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Classifier configuration                                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the configuration of a Classifier.                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_classifier_index - Classifier index                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_classifier_config - Classifier configuration                          */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_classifier_configuration ( uint8_t xi_classifier_index ,
                                                                DRV_IH_CLASSIFIER_CONFIG * const xo_classifier_config )
{
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_KEY0 ih_class_key ;
    IH_REGS_GENERAL_CONFIGURATION_IH_CLASS_MASK0 ih_class_mask ;
    IH_REGS_GENERAL_CONFIGURATION_IH_CLSF_MAPL_CFG ih_clsf_mapl_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IH_CLSF_MAPH_CFG ih_clsf_maph_cfg ;
    int32_t result ;


    result = f_check_item_index( xi_classifier_index ,
                                 DRV_IH_NUMBER_OF_CLASSIFIERS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }


    MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_KEY_READ_I( ih_class_key , xi_classifier_index ) ;
    xo_classifier_config->l2_protocol = ih_class_key.ih_class_key_l2 ;
    xo_classifier_config->l3_protocol = ih_class_key.ih_class_key_l3 ;
    xo_classifier_config->l4_protocol = ih_class_key.ih_class_key_l4 ;
    xo_classifier_config->da_filter_any_hit = ih_class_key.ih_class_key_da_anyhit ;
    xo_classifier_config->matched_da_filter = ih_class_key.ih_class_key_da_fltr ;
    xo_classifier_config->multicast_da_indication = ih_class_key.ih_class_key_mc_fltr ;
    xo_classifier_config->broadcast_da_indication = ih_class_key.ih_class_key_bc_fltr ;
    xo_classifier_config->vid_filter_any_hit = ih_class_key.ih_class_key_vid_anyhit ;
    xo_classifier_config->matched_vid_filter = ih_class_key.ih_class_key_vid_fltr ;
    xo_classifier_config->ip_filter_any_hit = ih_class_key.ih_class_key_ip_anyhit ;
    xo_classifier_config->matched_ip_filter = ih_class_key.ih_class_key_ip_fltr ;
    xo_classifier_config->wan_indication = ih_class_key.ih_class_key_wan_fltr ;
    xo_classifier_config->five_tuple_valid = ih_class_key.ih_class_key_5tpl_fltr ;
    xo_classifier_config->logical_source_port = ih_class_key.ih_class_key_sp ;
    xo_classifier_config->error = ih_class_key.ih_class_key_err ;
    
    MS_DRV_IH_GENERAL_CONFIGURATION_IH_CLASS_MASK_READ_I( ih_class_mask , xi_classifier_index ) ;
    xo_classifier_config->mask = ih_class_mask.ih_class_mask ;
    
    /* classifiers 0-7 are stored in the "low" register.
       classifiers 8-15 are stored in the "high" register. */
    if ( xi_classifier_index < CS_NUMBER_OF_CLASSIFIERS_IN_CLASSIFIER_TO_CLASS_MAPPING_REGISTERS )
    {
        IH_REGS_GENERAL_CONFIGURATION_IH_CLSF_MAPL_CFG_READ( ih_clsf_mapl_cfg ) ;

        switch ( xi_classifier_index )
        {
        case 0:
            xo_classifier_config->resulting_class = ih_clsf_mapl_cfg.clsf_set0_map ;
            break ;
        case 1:
            xo_classifier_config->resulting_class = ih_clsf_mapl_cfg.clsf_set1_map ;
            break ;
        case 2:
            xo_classifier_config->resulting_class = ih_clsf_mapl_cfg.clsf_set2_map ;
            break ;
        case 3:
            xo_classifier_config->resulting_class = ih_clsf_mapl_cfg.clsf_set3_map ;
            break ;
        case 4:
            xo_classifier_config->resulting_class = ih_clsf_mapl_cfg.clsf_set4_map ;
            break ;
        case 5:
            xo_classifier_config->resulting_class = ih_clsf_mapl_cfg.clsf_set5_map ;
            break ;
        case 6:
            xo_classifier_config->resulting_class = ih_clsf_mapl_cfg.clsf_set6_map ;
            break ;
        case 7:
            xo_classifier_config->resulting_class = ih_clsf_mapl_cfg.clsf_set7_map ;
            break ;
        }
    }
    else
    {
        IH_REGS_GENERAL_CONFIGURATION_IH_CLSF_MAPH_CFG_READ( ih_clsf_maph_cfg ) ;

        switch ( xi_classifier_index )
        {
        case 8:
            xo_classifier_config->resulting_class = ih_clsf_maph_cfg.clsf_set8_map ;
            break ;
        case 9:
            xo_classifier_config->resulting_class = ih_clsf_maph_cfg.clsf_set9_map ;
            break ;
        case 10:
            xo_classifier_config->resulting_class = ih_clsf_maph_cfg.clsf_set10_map ;
            break ;
        case 11:
            xo_classifier_config->resulting_class = ih_clsf_maph_cfg.clsf_set11_map ;
            break ;
        case 12:
            xo_classifier_config->resulting_class = ih_clsf_maph_cfg.clsf_set12_map ;
            break ;
        case 13:
            xo_classifier_config->resulting_class = ih_clsf_maph_cfg.clsf_set13_map ;
            break ;
        case 14:
            xo_classifier_config->resulting_class = ih_clsf_maph_cfg.clsf_set14_map ;
            break ;
        case 15:
            xo_classifier_config->resulting_class = ih_clsf_maph_cfg.clsf_set15_map ;
            break ;
        }
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_classifier_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_remove_classifier                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Remove Classifier                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function removes a classifier which was configured by Configure     */
/*   Classifier API. There is no "enable" bit - this function actually sets   */
/*   the mask to 0 and the key to 11...1 (bitwise) so there will never be a   */
/*   match.                                                                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_classifier_index - Classifier index                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_remove_classifier ( uint8_t xi_classifier_index )
{
    DRV_IH_ERROR error_code ;
    DRV_IH_CLASSIFIER_CONFIG classifier_config ;

    classifier_config.l2_protocol = ( DRV_IH_L2_PROTOCOL ) CS_ALL_ONES_VALUE ;
    classifier_config.l3_protocol = ( DRV_IH_L3_PROTOCOL ) CS_ALL_ONES_VALUE ;
    classifier_config.l4_protocol = ( DRV_IH_L4_PROTOCOL ) CS_ALL_ONES_VALUE ;
    classifier_config.da_filter_any_hit = ( int32_t ) CS_ALL_ONES_VALUE ;
    classifier_config.matched_da_filter = ( uint8_t ) CS_ALL_ONES_VALUE ;
    classifier_config.multicast_da_indication = ( int32_t ) CS_ALL_ONES_VALUE ;
    classifier_config.broadcast_da_indication = ( int32_t ) CS_ALL_ONES_VALUE ;
    classifier_config.vid_filter_any_hit = ( int32_t ) CS_ALL_ONES_VALUE ;
    classifier_config.matched_vid_filter = ( uint8_t ) CS_ALL_ONES_VALUE ;
    classifier_config.ip_filter_any_hit = ( int32_t ) CS_ALL_ONES_VALUE ;
    classifier_config.matched_ip_filter = ( uint8_t ) CS_ALL_ONES_VALUE ;
    classifier_config.wan_indication = ( int32_t ) CS_ALL_ONES_VALUE ;
    classifier_config.five_tuple_valid = ( int32_t ) CS_ALL_ONES_VALUE ;
    classifier_config.logical_source_port = ( DRV_IH_LOGICAL_PORT ) CS_ALL_ONES_VALUE ;
    classifier_config.error = ( int32_t ) CS_ALL_ONES_VALUE ;

    classifier_config.mask = 0 ;

    /* configuration of resulting_class is actually meaningless here */
    classifier_config.resulting_class = 0 ;

    error_code = fi_bl_drv_ih_configure_classifier ( xi_classifier_index ,
                                                           & classifier_config ) ;

    if ( error_code == DRV_IH_NO_ERROR )
    {
        gs_classifier_is_configured [ xi_classifier_index ] = 0 ;
    }

    return ( error_code ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_remove_classifier ) ;
#endif

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_source_port_to_ingress_queue_mapping                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set source port to ingress queue mapping                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets the mapping of physical source ports (eth0-4, GPON,   */
/*   runner A, runner B) to ingress queues. There are 8 ingress queues. BBH   */
/*   or runner (in case of runner flow) writes the Header Descriptor to one   */
/*   of these queues, according to the configuration of the source port.      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_source_port_to_ingress_queue_mapping - Source port to ingress queue   */
/*     mapping                                                                */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_source_port_to_ingress_queue_mapping ( const DRV_IH_SOURCE_PORT_TO_INGRESS_QUEUE_MAPPING * xi_source_port_to_ingress_queue_mapping )
{
    IH_REGS_GENERAL_CONFIGURATION_SP2IQ_MAP_CFG sp2iq_map_cfg ;


    if ( ( xi_source_port_to_ingress_queue_mapping->eth0_ingress_queue >= DRV_IH_NUMBER_OF_INGRESS_QUEUES ) ||
         ( xi_source_port_to_ingress_queue_mapping->eth1_ingress_queue >= DRV_IH_NUMBER_OF_INGRESS_QUEUES ) ||
         ( xi_source_port_to_ingress_queue_mapping->eth2_ingress_queue >= DRV_IH_NUMBER_OF_INGRESS_QUEUES ) ||
         ( xi_source_port_to_ingress_queue_mapping->eth3_ingress_queue >= DRV_IH_NUMBER_OF_INGRESS_QUEUES ) ||
         ( xi_source_port_to_ingress_queue_mapping->eth4_ingress_queue >= DRV_IH_NUMBER_OF_INGRESS_QUEUES ) ||
         ( xi_source_port_to_ingress_queue_mapping->gpon_ingress_queue >= DRV_IH_NUMBER_OF_INGRESS_QUEUES ) ||
         ( xi_source_port_to_ingress_queue_mapping->runner_a_ingress_queue >= DRV_IH_NUMBER_OF_INGRESS_QUEUES ) ||
         ( xi_source_port_to_ingress_queue_mapping->runner_b_ingress_queue >= DRV_IH_NUMBER_OF_INGRESS_QUEUES ) )
    {
        return ( DRV_IH_ERROR_INVALID_INGRESS_QUEUE ) ;
    }


    IH_REGS_GENERAL_CONFIGURATION_SP2IQ_MAP_CFG_READ( sp2iq_map_cfg ) ;
    sp2iq_map_cfg.eth0_iq_map = xi_source_port_to_ingress_queue_mapping->eth0_ingress_queue ;
    sp2iq_map_cfg.eth1_iq_map = xi_source_port_to_ingress_queue_mapping->eth1_ingress_queue ;
    sp2iq_map_cfg.eth2_iq_map = xi_source_port_to_ingress_queue_mapping->eth2_ingress_queue ;
    sp2iq_map_cfg.eth3_iq_map = xi_source_port_to_ingress_queue_mapping->eth3_ingress_queue ;
    sp2iq_map_cfg.eth4_iq_map = xi_source_port_to_ingress_queue_mapping->eth4_ingress_queue ;
    sp2iq_map_cfg.gpon_iq_map = xi_source_port_to_ingress_queue_mapping->gpon_ingress_queue ;
    sp2iq_map_cfg.rnra_iq_map = xi_source_port_to_ingress_queue_mapping->runner_a_ingress_queue ;
    sp2iq_map_cfg.rnrb_iq_map = xi_source_port_to_ingress_queue_mapping->runner_b_ingress_queue ;
    IH_REGS_GENERAL_CONFIGURATION_SP2IQ_MAP_CFG_WRITE( sp2iq_map_cfg ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_set_source_port_to_ingress_queue_mapping ) ;

#ifndef _CFE_
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_source_port_to_ingress_queue_mapping                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get source port to ingress queue mapping                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the source port to ingress queue mapping.             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_source_port_to_ingress_queue_mapping - Source port to ingress queue   */
/*     mapping                                                                */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_source_port_to_ingress_queue_mapping ( DRV_IH_SOURCE_PORT_TO_INGRESS_QUEUE_MAPPING * const xo_source_port_to_ingress_queue_mapping )
{
    IH_REGS_GENERAL_CONFIGURATION_SP2IQ_MAP_CFG sp2iq_map_cfg ;


    IH_REGS_GENERAL_CONFIGURATION_SP2IQ_MAP_CFG_READ( sp2iq_map_cfg ) ;
    xo_source_port_to_ingress_queue_mapping->eth0_ingress_queue = sp2iq_map_cfg.eth0_iq_map ;
    xo_source_port_to_ingress_queue_mapping->eth1_ingress_queue = sp2iq_map_cfg.eth1_iq_map ;
    xo_source_port_to_ingress_queue_mapping->eth2_ingress_queue = sp2iq_map_cfg.eth2_iq_map ;
    xo_source_port_to_ingress_queue_mapping->eth3_ingress_queue = sp2iq_map_cfg.eth3_iq_map ;
    xo_source_port_to_ingress_queue_mapping->eth4_ingress_queue = sp2iq_map_cfg.eth4_iq_map ;
    xo_source_port_to_ingress_queue_mapping->gpon_ingress_queue = sp2iq_map_cfg.gpon_iq_map ;
    xo_source_port_to_ingress_queue_mapping->runner_a_ingress_queue = sp2iq_map_cfg.rnra_iq_map ;
    xo_source_port_to_ingress_queue_mapping->runner_b_ingress_queue = sp2iq_map_cfg.rnrb_iq_map ;
    
    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_source_port_to_ingress_queue_mapping ) ;
#endif

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_ingress_queue                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure ingress queue                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures an ingress queue. There are 8 queues. All of    */
/*   them reside in the same Ingress-queue (IQ) array of 16 entries (ingress  */
/*   buffers). E.g. queue 0 occupies entries 0-1, queue 1 occupies entries    */
/*   2-3, etc.                                                                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ingress_queue_index - Ingress queue index                             */
/*                                                                            */
/*   xi_ingress_queue_config - Ingress queue configuration                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_ingress_queue ( uint8_t xi_ingress_queue_index ,
                                                           const DRV_IH_INGRESS_QUEUE_CONFIG * xi_ingress_queue_config )
{
    IH_REGS_GENERAL_CONFIGURATION_IQ_BASE_CFG iq_base_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IQ_SIZE_CFG iq_size_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IQ_WEIGHT_CFG iq_weight_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IQL_PRIOR_CFG iql_prior_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IQH_PRIOR_CFG iqh_prior_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IQL_CNGS_THRS_CFG iql_cngs_thrs_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IQH_CNGS_THRS_CFG iqh_cngs_thrs_cfg ;
    int32_t result ;
    uint8_t queue_size_encoding ;


    result = f_check_item_index( xi_ingress_queue_index ,
                                 DRV_IH_NUMBER_OF_INGRESS_QUEUES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    if ( xi_ingress_queue_config->base_location >= DRV_IH_INGRESS_QUEUES_ARRAY_SIZE )
    {
        return ( DRV_IH_ERROR_INVALID_INGRESS_QUEUE_BASE_LOCATION ) ;
    }

    if ( ( xi_ingress_queue_config->size > DRV_IH_INGRESS_QUEUES_ARRAY_SIZE ) ||
         ( xi_ingress_queue_config->size < DRV_IH_MINIMAL_INGRESS_QUEUE_SIZE) )
    {
        return ( DRV_IH_ERROR_INVALID_INGRESS_QUEUE_SIZE ) ;
    }

    if ( xi_ingress_queue_config->priority > DRV_IH_MAXIMAL_INGRESS_QUEUE_PRIORITY )
    {
        return ( DRV_IH_ERROR_INVALID_INGRESS_QUEUE_PRIORITY ) ;
    }

    if ( xi_ingress_queue_config->weight > DRV_IH_MAXIMAL_INGRESS_QUEUE_WEIGHT )
    {
        return ( DRV_IH_ERROR_INVALID_INGRESS_QUEUE_WEIGHT ) ;
    }

    if ( xi_ingress_queue_config->congestion_threshold > DRV_IH_MAXIMAL_INGRESS_QUEUE_CONGESTION_THRESHOLD )
    {
        return ( DRV_IH_ERROR_INVALID_NUMBER_OF_RUNNER_BUFFERS ) ;
    }


    /* in HW, the value 0 for size means 16 (it is 4-bit field) */
    queue_size_encoding = xi_ingress_queue_config->size ;
    if ( queue_size_encoding == DRV_IH_INGRESS_QUEUES_ARRAY_SIZE )
    {
        queue_size_encoding = 0 ;
    }


    /* handle configuration which resides in one register for all queues */

    IH_REGS_GENERAL_CONFIGURATION_IQ_BASE_CFG_READ( iq_base_cfg ) ;
    IH_REGS_GENERAL_CONFIGURATION_IQ_SIZE_CFG_READ( iq_size_cfg ) ;
    IH_REGS_GENERAL_CONFIGURATION_IQ_WEIGHT_CFG_READ( iq_weight_cfg ) ;

    switch ( xi_ingress_queue_index )
    {
    case 0:
        iq_base_cfg.iq0_base = xi_ingress_queue_config->base_location ;
        iq_size_cfg.iq0_size = queue_size_encoding ;
        iq_weight_cfg.iq0_weight = xi_ingress_queue_config->weight ;
        break ;
    case 1:
        iq_base_cfg.iq1_base = xi_ingress_queue_config->base_location ;
        iq_size_cfg.iq1_size = queue_size_encoding ;
        iq_weight_cfg.iq1_weight = xi_ingress_queue_config->weight ;
        break ;
    case 2:
        iq_base_cfg.iq2_base = xi_ingress_queue_config->base_location ;
        iq_size_cfg.iq2_size = queue_size_encoding ;
        iq_weight_cfg.iq2_weight = xi_ingress_queue_config->weight ;
        break ;
    case 3:
        iq_base_cfg.iq3_base = xi_ingress_queue_config->base_location ;
        iq_size_cfg.iq3_size = queue_size_encoding ;
        iq_weight_cfg.iq3_weight = xi_ingress_queue_config->weight ;
        break ;
    case 4:
        iq_base_cfg.iq4_base = xi_ingress_queue_config->base_location ;
        iq_size_cfg.iq4_size = queue_size_encoding ;
        iq_weight_cfg.iq4_weight = xi_ingress_queue_config->weight ;
        break ;
    case 5:
        iq_base_cfg.iq5_base = xi_ingress_queue_config->base_location ;
        iq_size_cfg.iq5_size = queue_size_encoding ;
        iq_weight_cfg.iq5_weight = xi_ingress_queue_config->weight ;
        break ;
    case 6:
        iq_base_cfg.iq6_base = xi_ingress_queue_config->base_location ;
        iq_size_cfg.iq6_size = queue_size_encoding ;
        iq_weight_cfg.iq6_weight = xi_ingress_queue_config->weight ;
        break ;
    case 7:
        iq_base_cfg.iq7_base = xi_ingress_queue_config->base_location ;
        iq_size_cfg.iq7_size = queue_size_encoding ;
        iq_weight_cfg.iq7_weight = xi_ingress_queue_config->weight ;
        break ;
    }
    
    IH_REGS_GENERAL_CONFIGURATION_IQ_BASE_CFG_WRITE( iq_base_cfg ) ;
    IH_REGS_GENERAL_CONFIGURATION_IQ_SIZE_CFG_WRITE( iq_size_cfg ) ;
    IH_REGS_GENERAL_CONFIGURATION_IQ_WEIGHT_CFG_WRITE( iq_weight_cfg ) ;

    /* handle configuration which is split into 2 registers */

    if ( xi_ingress_queue_index < CS_NUMBER_OF_INGRESS_QUEUES_IN_PRIORITY_AND_CONGESTION_THRESHOLD_REGISTERS )
    {
        IH_REGS_GENERAL_CONFIGURATION_IQL_PRIOR_CFG_READ( iql_prior_cfg ) ;
        IH_REGS_GENERAL_CONFIGURATION_IQL_CNGS_THRS_CFG_READ( iql_cngs_thrs_cfg ) ;

        switch ( xi_ingress_queue_index )
        {
        case 0:
            /* "one hot" format */
            iql_prior_cfg.iq0_prior = ( 1 << xi_ingress_queue_config->priority ) ;
            iql_cngs_thrs_cfg.iq0_cngs_thrs = xi_ingress_queue_config->congestion_threshold ;
            break ;
        case 1:
            /* "one hot" format */
            iql_prior_cfg.iq1_prior = ( 1 << xi_ingress_queue_config->priority ) ;
            iql_cngs_thrs_cfg.iq1_cngs_thrs = xi_ingress_queue_config->congestion_threshold ;
            break ;
        case 2:
            /* "one hot" format */
            iql_prior_cfg.iq2_prior = ( 1 << xi_ingress_queue_config->priority ) ;
            iql_cngs_thrs_cfg.iq2_cngs_thrs = xi_ingress_queue_config->congestion_threshold ;
            break ;
        case 3:
            /* "one hot" format */
            iql_prior_cfg.iq3_prior = ( 1 << xi_ingress_queue_config->priority ) ;
            iql_cngs_thrs_cfg.iq3_cngs_thrs = xi_ingress_queue_config->congestion_threshold ;
            break ;
        }

        IH_REGS_GENERAL_CONFIGURATION_IQL_PRIOR_CFG_WRITE( iql_prior_cfg ) ;
        IH_REGS_GENERAL_CONFIGURATION_IQL_CNGS_THRS_CFG_WRITE( iql_cngs_thrs_cfg ) ;
    }
    else
    {
        IH_REGS_GENERAL_CONFIGURATION_IQH_PRIOR_CFG_READ( iqh_prior_cfg ) ;
        IH_REGS_GENERAL_CONFIGURATION_IQH_CNGS_THRS_CFG_READ( iqh_cngs_thrs_cfg ) ;

        switch ( xi_ingress_queue_index )
        {
        case 4:
            /* "one hot" format */
            iqh_prior_cfg.iq4_prior = ( 1 << xi_ingress_queue_config->priority ) ;
            iqh_cngs_thrs_cfg.iq4_cngs_thrs = xi_ingress_queue_config->congestion_threshold ;
            break ;
        case 5:
            /* "one hot" format */
            iqh_prior_cfg.iq5_prior = ( 1 << xi_ingress_queue_config->priority ) ;
            iqh_cngs_thrs_cfg.iq5_cngs_thrs = xi_ingress_queue_config->congestion_threshold ;
            break ;
        case 6:
            /* "one hot" format */
            iqh_prior_cfg.iq6_prior = ( 1 << xi_ingress_queue_config->priority ) ;
            iqh_cngs_thrs_cfg.iq6_cngs_thrs = xi_ingress_queue_config->congestion_threshold ;
            break ;
        case 7:
            /* "one hot" format */
            iqh_prior_cfg.iq7_prior = ( 1 << xi_ingress_queue_config->priority ) ;
            iqh_cngs_thrs_cfg.iq7_cngs_thrs = xi_ingress_queue_config->congestion_threshold ;
            break ;
        }

        IH_REGS_GENERAL_CONFIGURATION_IQH_PRIOR_CFG_WRITE( iqh_prior_cfg ) ;
        IH_REGS_GENERAL_CONFIGURATION_IQH_CNGS_THRS_CFG_WRITE( iqh_cngs_thrs_cfg ) ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_configure_ingress_queue ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_ingress_queue_configuration                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get ingress queue configuration                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the configuration of an ingress queue.                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ingress_queue_index - Ingress queue index                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_ingress_queue_config - Ingress queue configuration                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_ingress_queue_configuration ( uint8_t xi_ingress_queue_index ,
                                                                   DRV_IH_INGRESS_QUEUE_CONFIG * const xo_ingress_queue_config )
{
    IH_REGS_GENERAL_CONFIGURATION_IQ_BASE_CFG iq_base_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IQ_SIZE_CFG iq_size_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IQ_WEIGHT_CFG iq_weight_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IQL_PRIOR_CFG iql_prior_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IQH_PRIOR_CFG iqh_prior_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IQL_CNGS_THRS_CFG iql_cngs_thrs_cfg ;
    IH_REGS_GENERAL_CONFIGURATION_IQH_CNGS_THRS_CFG iqh_cngs_thrs_cfg ;
    int32_t result ;


    result = f_check_item_index( xi_ingress_queue_index ,
                                 DRV_IH_NUMBER_OF_INGRESS_QUEUES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }


    /* handle configuration which resides in one register for all queues */

    IH_REGS_GENERAL_CONFIGURATION_IQ_BASE_CFG_READ( iq_base_cfg ) ;
    IH_REGS_GENERAL_CONFIGURATION_IQ_SIZE_CFG_READ( iq_size_cfg ) ;
    IH_REGS_GENERAL_CONFIGURATION_IQ_WEIGHT_CFG_READ( iq_weight_cfg ) ;

    switch ( xi_ingress_queue_index )
    {
    case 0:
        xo_ingress_queue_config->base_location = iq_base_cfg.iq0_base ;
        xo_ingress_queue_config->size = iq_size_cfg.iq0_size ;
        xo_ingress_queue_config->weight = iq_weight_cfg.iq0_weight ;
        break ;
    case 1:
        xo_ingress_queue_config->base_location = iq_base_cfg.iq1_base ;
        xo_ingress_queue_config->size = iq_size_cfg.iq1_size ;
        xo_ingress_queue_config->weight = iq_weight_cfg.iq1_weight ;
        break ;
    case 2:
        xo_ingress_queue_config->base_location = iq_base_cfg.iq2_base ;
        xo_ingress_queue_config->size = iq_size_cfg.iq2_size ;
        xo_ingress_queue_config->weight = iq_weight_cfg.iq2_weight ;
        break ;
    case 3:
        xo_ingress_queue_config->base_location = iq_base_cfg.iq3_base ;
        xo_ingress_queue_config->size = iq_size_cfg.iq3_size ;
        xo_ingress_queue_config->weight = iq_weight_cfg.iq3_weight ;
        break ;
    case 4:
        xo_ingress_queue_config->base_location = iq_base_cfg.iq4_base ;
        xo_ingress_queue_config->size = iq_size_cfg.iq4_size ;
        xo_ingress_queue_config->weight = iq_weight_cfg.iq4_weight ;
        break ;
    case 5:
        xo_ingress_queue_config->base_location = iq_base_cfg.iq5_base ;
        xo_ingress_queue_config->size = iq_size_cfg.iq5_size ;
        xo_ingress_queue_config->weight = iq_weight_cfg.iq5_weight ;
        break ;
    case 6:
        xo_ingress_queue_config->base_location = iq_base_cfg.iq6_base ;
        xo_ingress_queue_config->size = iq_size_cfg.iq6_size ;
        xo_ingress_queue_config->weight = iq_weight_cfg.iq6_weight ;
        break ;
    case 7:
        xo_ingress_queue_config->base_location = iq_base_cfg.iq7_base ;
        xo_ingress_queue_config->size = iq_size_cfg.iq7_size ;
        xo_ingress_queue_config->weight = iq_weight_cfg.iq7_weight ;
        break ;
    }


    /* in HW, the value 0 for size means 16 (it is 4-bit field) */
    if ( xo_ingress_queue_config->size == 0 )
    {
        xo_ingress_queue_config->size = DRV_IH_INGRESS_QUEUES_ARRAY_SIZE ;
    }


    /* handle configuration which is split into 2 registers */

    if ( xi_ingress_queue_index < CS_NUMBER_OF_INGRESS_QUEUES_IN_PRIORITY_AND_CONGESTION_THRESHOLD_REGISTERS )
    {
        IH_REGS_GENERAL_CONFIGURATION_IQL_PRIOR_CFG_READ( iql_prior_cfg ) ;
        IH_REGS_GENERAL_CONFIGURATION_IQL_CNGS_THRS_CFG_READ( iql_cngs_thrs_cfg ) ;

        switch ( xi_ingress_queue_index )
        {
        case 0:
            xo_ingress_queue_config->priority = iql_prior_cfg.iq0_prior ;
            xo_ingress_queue_config->congestion_threshold = iql_cngs_thrs_cfg.iq0_cngs_thrs ;
            break ;
        case 1:
            xo_ingress_queue_config->priority = iql_prior_cfg.iq1_prior ;
            xo_ingress_queue_config->congestion_threshold = iql_cngs_thrs_cfg.iq1_cngs_thrs ;
            break ;
        case 2:
            xo_ingress_queue_config->priority = iql_prior_cfg.iq2_prior ;
            xo_ingress_queue_config->congestion_threshold = iql_cngs_thrs_cfg.iq2_cngs_thrs ;
            break ;
        case 3:
            xo_ingress_queue_config->priority = iql_prior_cfg.iq3_prior ;
            xo_ingress_queue_config->congestion_threshold = iql_cngs_thrs_cfg.iq3_cngs_thrs ;
            break ;
        }
    }
    else
    {
        IH_REGS_GENERAL_CONFIGURATION_IQH_PRIOR_CFG_READ( iqh_prior_cfg ) ;
        IH_REGS_GENERAL_CONFIGURATION_IQH_CNGS_THRS_CFG_READ( iqh_cngs_thrs_cfg ) ;

        switch ( xi_ingress_queue_index )
        {
        case 4:
            xo_ingress_queue_config->priority = iqh_prior_cfg.iq4_prior ;
            xo_ingress_queue_config->congestion_threshold = iqh_cngs_thrs_cfg.iq4_cngs_thrs ;
            break ;
        case 5:
            xo_ingress_queue_config->priority = iqh_prior_cfg.iq5_prior ;
            xo_ingress_queue_config->congestion_threshold = iqh_cngs_thrs_cfg.iq5_cngs_thrs ;
            break ;
        case 6:
            xo_ingress_queue_config->priority = iqh_prior_cfg.iq6_prior ;
            xo_ingress_queue_config->congestion_threshold = iqh_cngs_thrs_cfg.iq6_cngs_thrs ;
            break ;
        case 7:
            xo_ingress_queue_config->priority = iqh_prior_cfg.iq7_prior ;
            xo_ingress_queue_config->congestion_threshold = iqh_cngs_thrs_cfg.iq7_cngs_thrs ;
            break ;
        }
    }

    xo_ingress_queue_config->priority = f_translate_ingress_queue_priority ( xo_ingress_queue_config->priority ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_ingress_queue_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_target_matrix                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Target Matrix                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets the per-source-port configuration in the target       */
/*   matrix, i.e. all entries which belong to the given source port.          */
/*   Each entry consists of the following parameters: target memory           */
/*   (DDR/SRAM), direct mode (true/false).                                    */
/*   The function will fail when trying to configure an "Always DDR" entry    */
/*   with Target memory = SRAM, or "Always SRAM" entry with                   */
/*   Target memory = DDR.                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_source_port - Source port                                             */
/*                                                                            */
/*   xi_per_sp_config - Per-source-port configuration                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_target_matrix ( DRV_IH_TARGET_MATRIX_SOURCE_PORT xi_source_port ,
                                                     const DRV_IH_TARGET_MATRIX_PER_SP_CONFIG * xi_per_sp_config )
{
    /* we will use this variable for any source port, not only for eth0 (the registers are similar) */
    IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH0_SP_CFG trgt_mtrx_sp_cfg ;


    if ( xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_DDR ].target_memory == DRV_IH_TARGET_MEMORY_SRAM )
    {
        return ( DRV_IH_ERROR_DESTINATION_PORT_AND_TARGET_MEMORY_MISMATCH ) ;
    }

    if ( xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_SRAM ].target_memory == DRV_IH_TARGET_MEMORY_DDR )
    {
        return ( DRV_IH_ERROR_DESTINATION_PORT_AND_TARGET_MEMORY_MISMATCH ) ;
    }


    /* Here we don't do read-modify-write, we only write.
       this is because these registers are write-only. */

    trgt_mtrx_sp_cfg.dp_eth0_tm_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0 ].target_memory ;
    trgt_mtrx_sp_cfg.dp_eth0_ls_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0 ].direct_mode ;

    trgt_mtrx_sp_cfg.dp_eth1_tm_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH1 ].target_memory ;
    trgt_mtrx_sp_cfg.dp_eth1_ls_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH1 ].direct_mode ;

    trgt_mtrx_sp_cfg.dp_eth2_tm_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH2 ].target_memory ;
    trgt_mtrx_sp_cfg.dp_eth2_ls_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH2 ].direct_mode ;

    trgt_mtrx_sp_cfg.dp_eth3_tm_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH3 ].target_memory ;
    trgt_mtrx_sp_cfg.dp_eth3_ls_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH3 ].direct_mode ;

    trgt_mtrx_sp_cfg.dp_eth4_tm_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH4 ].target_memory ;
    trgt_mtrx_sp_cfg.dp_eth4_ls_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH4 ].direct_mode ;

    trgt_mtrx_sp_cfg.dp_gpon_tm_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON ].target_memory ;
    trgt_mtrx_sp_cfg.dp_gpon_ls_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON ].direct_mode ;

    trgt_mtrx_sp_cfg.dp_pcie0_tm_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0 ].target_memory ;
    trgt_mtrx_sp_cfg.dp_pcie0_ls_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0 ].direct_mode ;

    trgt_mtrx_sp_cfg.dp_pcie1_tm_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE1 ].target_memory ;
    trgt_mtrx_sp_cfg.dp_pcie1_ls_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE1 ].direct_mode ;

    trgt_mtrx_sp_cfg.dp_cpu_tm_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_CPU ].target_memory ;
    trgt_mtrx_sp_cfg.dp_cpu_ls_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_CPU ].direct_mode ;

    trgt_mtrx_sp_cfg.dp_mc_tm_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_MULTICAST ].target_memory ;
    trgt_mtrx_sp_cfg.dp_mc_ls_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_MULTICAST ].direct_mode ;

    /* no configuration of target memory for "ALWAYS_DDR" port */
    trgt_mtrx_sp_cfg.dp_ddr_ls_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_DDR ].direct_mode ;

    /* no configuration of target memory for "ALWAYS_SRAM" port */
    trgt_mtrx_sp_cfg.dp_sram_ls_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_SRAM ].direct_mode ;

    trgt_mtrx_sp_cfg.dp_spare_tm_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_SPARE ].target_memory ;
    trgt_mtrx_sp_cfg.dp_spare_ls_cfg = xi_per_sp_config->entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_SPARE ].direct_mode ;

    trgt_mtrx_sp_cfg.rsv1 = IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH0_SP_CFG_RSV1_RSV_VALUE ;
    trgt_mtrx_sp_cfg.rsv2 = IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH0_SP_CFG_RSV2_RSV_VALUE ;
    
    /* write entry according to souce port */
    switch ( xi_source_port )
    {
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH0_SP_CFG_WRITE( trgt_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH1:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH1_SP_CFG_WRITE( trgt_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH2:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH2_SP_CFG_WRITE( trgt_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH3:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH3_SP_CFG_WRITE( trgt_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH4:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH4_SP_CFG_WRITE( trgt_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_GPON_SP_CFG_WRITE( trgt_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE0:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_PCIE0_SP_CFG_WRITE( trgt_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE1:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_PCIE1_SP_CFG_WRITE( trgt_mtrx_sp_cfg ) ;
        break ;
    default:
        return ( DRV_IH_ERROR_INVALID_PORT ) ;
        break ;
    }

    /* create shadow */
    memcpy((uint8_t *)&(trgt_mtrx_sp_shadow[xi_source_port]), (uint8_t *)xi_per_sp_config, sizeof(DRV_IH_TARGET_MATRIX_PER_SP_CONFIG));

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_set_target_matrix ) ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_target_matrix_shadow_entry                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Target Matrix entry shadow (last write)                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets an entry in the target matrix.                        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_source_port - Source port                                             */
/*                                                                            */
/*   xi_destination_port - Destination port                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_entry - Entry                                                         */
/*                                                                            */
/******************************************************************************/
void fi_bl_drv_ih_get_target_matrix_shadow_entry ( DRV_IH_TARGET_MATRIX_SOURCE_PORT xi_source_port ,
                                            DRV_IH_TARGET_MATRIX_DESTINATION_PORT xi_destination_port ,
                                            DRV_IH_TARGET_MATRIX_ENTRY * const xo_entry )
{
    xo_entry->target_memory = trgt_mtrx_sp_shadow[xi_source_port].entry[xi_destination_port].target_memory;
    xo_entry->direct_mode = trgt_mtrx_sp_shadow[xi_source_port].entry[xi_destination_port].direct_mode;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_target_matrix_shadow_entry ) ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_target_matrix_entry                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Target Matrix entry                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets an entry in the target matrix.                        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_source_port - Source port                                             */
/*                                                                            */
/*   xi_destination_port - Destination port                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_entry - Entry                                                         */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_target_matrix_entry ( DRV_IH_TARGET_MATRIX_SOURCE_PORT xi_source_port ,
                                                           DRV_IH_TARGET_MATRIX_DESTINATION_PORT xi_destination_port ,
                                                           DRV_IH_TARGET_MATRIX_ENTRY * const xo_entry )
{
    /* we will use this variable for any source port, not only for eth0 (the registers are similar) */
    IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH0_SP_CFG trgt_mtrx_sp_cfg ;

    if ( ( xi_destination_port == DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE1 ) ||
         ( xi_destination_port == DRV_IH_TARGET_MATRIX_DESTINATION_PORT_SPARE ) ||
         ( xi_source_port == DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE0 ) ||
         ( xi_source_port == DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE1 )
       )
    {
        return ( DRV_IH_ERROR_VALUE_IS_WRITE_ONLY ) ;
    }


    /* read entry according to souce port */
    switch ( xi_source_port )
    {
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH0_SP_CFG_READ( trgt_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH1:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH1_SP_CFG_READ( trgt_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH2:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH2_SP_CFG_READ( trgt_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH3:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH3_SP_CFG_READ( trgt_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH4:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_ETH4_SP_CFG_READ( trgt_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON:
        IH_REGS_GENERAL_CONFIGURATION_TRGT_MTRX_GPON_SP_CFG_READ( trgt_mtrx_sp_cfg ) ;
        break ;
    default:
        return ( DRV_IH_ERROR_INVALID_PORT ) ;
        break ;
    }

    /* extract entry fields according to destination port */
    switch ( xi_destination_port )
    {
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0:
        xo_entry->target_memory = trgt_mtrx_sp_cfg.dp_eth0_tm_cfg ;
        xo_entry->direct_mode = trgt_mtrx_sp_cfg.dp_eth0_ls_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH1:
        xo_entry->target_memory = trgt_mtrx_sp_cfg.dp_eth1_tm_cfg ;
        xo_entry->direct_mode = trgt_mtrx_sp_cfg.dp_eth1_ls_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH2:
        xo_entry->target_memory = trgt_mtrx_sp_cfg.dp_eth2_tm_cfg ;
        xo_entry->direct_mode = trgt_mtrx_sp_cfg.dp_eth2_ls_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH3:
        xo_entry->target_memory = trgt_mtrx_sp_cfg.dp_eth3_tm_cfg ;
        xo_entry->direct_mode = trgt_mtrx_sp_cfg.dp_eth3_ls_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH4:
        xo_entry->target_memory = trgt_mtrx_sp_cfg.dp_eth4_tm_cfg ;
        xo_entry->direct_mode = trgt_mtrx_sp_cfg.dp_eth4_ls_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON:
        xo_entry->target_memory = trgt_mtrx_sp_cfg.dp_gpon_tm_cfg ;
        xo_entry->direct_mode = trgt_mtrx_sp_cfg.dp_gpon_ls_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0:
        xo_entry->target_memory = trgt_mtrx_sp_cfg.dp_pcie0_tm_cfg ;
        xo_entry->direct_mode = trgt_mtrx_sp_cfg.dp_pcie0_ls_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_CPU:
        xo_entry->target_memory = trgt_mtrx_sp_cfg.dp_cpu_tm_cfg ;
        xo_entry->direct_mode = trgt_mtrx_sp_cfg.dp_cpu_ls_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_MULTICAST:
        xo_entry->target_memory = trgt_mtrx_sp_cfg.dp_mc_tm_cfg ;
        xo_entry->direct_mode = trgt_mtrx_sp_cfg.dp_mc_ls_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_DDR:
        /* no configuration of target memory here */
        xo_entry->target_memory = DRV_IH_TARGET_MEMORY_DDR ;
        xo_entry->direct_mode = trgt_mtrx_sp_cfg.dp_ddr_ls_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_SRAM:
        /* no configuration of target memory here */
        xo_entry->target_memory = DRV_IH_TARGET_MEMORY_SRAM ;
        xo_entry->direct_mode = trgt_mtrx_sp_cfg.dp_sram_ls_cfg ;
        break ;
    default:
        return ( DRV_IH_ERROR_INVALID_PORT ) ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_target_matrix_entry ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_forward                                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Target Forward                                           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets the "forward-enable" bit for the given source         */
/*   port and destination port.                                               */
/*   The "forward-enable" is only indication to FW (IH doesn't drop if        */
/*   forwarding is disabled).                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_source_port - Source port                                             */
/*                                                                            */
/*   xi_destination_port - Destination port                                   */
/*                                                                            */
/*   xi_forward - Forward                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_forward ( DRV_IH_TARGET_MATRIX_SOURCE_PORT xi_source_port ,
                                               DRV_IH_TARGET_MATRIX_DESTINATION_PORT xi_destination_port ,
                                               int32_t xi_forward )
{
    /* we will use this variable for any source port, not only for eth0 (the registers are similar) */
    IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH0_SP_CFG fw_en_mtrx_sp_cfg ;


    /* read entry according to souce port */
    switch ( xi_source_port )
    {
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH0_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH1:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH1_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH2:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH2_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH3:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH3_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH4:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH4_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_GPON_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE0:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_PCIE0_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE1:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_PCIE1_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    default:
        return ( DRV_IH_ERROR_INVALID_PORT ) ;
        break ;
    }

    /* modify entry according to destination port */
    switch ( xi_destination_port )
    {
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0:
        fw_en_mtrx_sp_cfg.dp_eth0_fw_en_cfg = xi_forward ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH1:
        fw_en_mtrx_sp_cfg.dp_eth1_fw_en_cfg = xi_forward ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH2:
        fw_en_mtrx_sp_cfg.dp_eth2_fw_en_cfg = xi_forward ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH3:
        fw_en_mtrx_sp_cfg.dp_eth3_fw_en_cfg = xi_forward ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH4:
        fw_en_mtrx_sp_cfg.dp_eth4_fw_en_cfg = xi_forward ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON:
        fw_en_mtrx_sp_cfg.dp_gpon_fw_en_cfg = xi_forward ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0:
        fw_en_mtrx_sp_cfg.dp_pcie0_fw_en_cfg = xi_forward ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE1:
        fw_en_mtrx_sp_cfg.dp_pcie1_fw_en_cfg = xi_forward ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_CPU:
        fw_en_mtrx_sp_cfg.dp_cpu_fw_en_cfg = xi_forward ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_MULTICAST:
        fw_en_mtrx_sp_cfg.dp_mc_fw_en_cfg = xi_forward ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_DDR:
        fw_en_mtrx_sp_cfg.dp_ddr_fw_en_cfg = xi_forward ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_SRAM:
        fw_en_mtrx_sp_cfg.dp_sram_fw_en_cfg = xi_forward ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_SPARE:
        fw_en_mtrx_sp_cfg.dp_spare_fw_en_cfg = xi_forward ;
        break ;
    default:
        return ( DRV_IH_ERROR_INVALID_PORT ) ;
        break ;
    }
    
    /* write entry according to souce port */
    switch ( xi_source_port )
    {
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH0_SP_CFG_WRITE( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH1:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH1_SP_CFG_WRITE( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH2:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH2_SP_CFG_WRITE( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH3:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH3_SP_CFG_WRITE( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH4:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH4_SP_CFG_WRITE( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_GPON_SP_CFG_WRITE( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE0:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_PCIE0_SP_CFG_WRITE( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE1:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_PCIE1_SP_CFG_WRITE( fw_en_mtrx_sp_cfg ) ;
        break ;
    default:
        return ( DRV_IH_ERROR_INVALID_PORT ) ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_set_forward ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_forward                                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Target Forward                                           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the "forward-enable" bit for the given source         */
/*   port and destination port.                                               */
/*   The "forward-enable" is only indication to FW (IH doesn't drop if        */
/*   forwarding is disabled).                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_source_port - Source port                                             */
/*                                                                            */
/*   xi_destination_port - Destination port                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_forward - Forward                                                     */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_forward ( DRV_IH_TARGET_MATRIX_SOURCE_PORT xi_source_port ,
                                               DRV_IH_TARGET_MATRIX_DESTINATION_PORT xi_destination_port ,
                                               int32_t * const xo_forward )
{
    /* we will use this variable for any source port, not only for eth0 (the registers are similar) */
    IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH0_SP_CFG fw_en_mtrx_sp_cfg ;


    /* read entry according to souce port */
    switch ( xi_source_port )
    {
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH0_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH1:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH1_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH2:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH2_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH3:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH3_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH4:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_ETH4_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_GPON_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE0:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_PCIE0_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    case DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE1:
        IH_REGS_GENERAL_CONFIGURATION_FW_EN_MTRX_PCIE1_SP_CFG_READ( fw_en_mtrx_sp_cfg ) ;
        break ;
    default:
        return ( DRV_IH_ERROR_INVALID_PORT ) ;
        break ;
    }

    /* extract the forward bit according to destination port */
    switch ( xi_destination_port )
    {
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0:
        * xo_forward = fw_en_mtrx_sp_cfg.dp_eth0_fw_en_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH1:
        * xo_forward = fw_en_mtrx_sp_cfg.dp_eth1_fw_en_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH2:
        * xo_forward = fw_en_mtrx_sp_cfg.dp_eth2_fw_en_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH3:
        * xo_forward = fw_en_mtrx_sp_cfg.dp_eth3_fw_en_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH4:
        * xo_forward = fw_en_mtrx_sp_cfg.dp_eth4_fw_en_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON:
        * xo_forward = fw_en_mtrx_sp_cfg.dp_gpon_fw_en_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0:
        * xo_forward = fw_en_mtrx_sp_cfg.dp_pcie0_fw_en_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE1:
        * xo_forward = fw_en_mtrx_sp_cfg.dp_pcie1_fw_en_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_CPU:
        * xo_forward = fw_en_mtrx_sp_cfg.dp_cpu_fw_en_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_MULTICAST:
        * xo_forward = fw_en_mtrx_sp_cfg.dp_mc_fw_en_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_DDR:
        * xo_forward = fw_en_mtrx_sp_cfg.dp_ddr_fw_en_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_SRAM:
        * xo_forward = fw_en_mtrx_sp_cfg.dp_sram_fw_en_cfg ;
        break ;
    case DRV_IH_TARGET_MATRIX_DESTINATION_PORT_SPARE:
        * xo_forward = fw_en_mtrx_sp_cfg.dp_spare_fw_en_cfg ;
        break ;
    default:
        return ( DRV_IH_ERROR_INVALID_PORT ) ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_forward ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_wan_ports                                         */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure WAN ports                                          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures, for each logical port, whether it belongs to   */
/*   WAN traffic. IH uses this configuration for WAN indication in the parser */
/*   result (and Classifier Key Word).                                        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_wan_ports_config - WAN ports configuration                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_wan_ports ( const DRV_IH_WAN_PORTS_CONFIG * xi_wan_ports_config )
{
    IH_REGS_GENERAL_CONFIGURATION_WAN_PER_PORT_CFG wan_per_port_cfg ;

    IH_REGS_GENERAL_CONFIGURATION_WAN_PER_PORT_CFG_READ( wan_per_port_cfg ) ;
    wan_per_port_cfg.eth0_trf_map = xi_wan_ports_config->eth0 ;
    wan_per_port_cfg.eth1_trf_map = xi_wan_ports_config->eth1 ;
    wan_per_port_cfg.eth2_trf_map = xi_wan_ports_config->eth2 ;
    wan_per_port_cfg.eth3_trf_map = xi_wan_ports_config->eth3 ;
    wan_per_port_cfg.eth4_trf_map = xi_wan_ports_config->eth4 ;
    wan_per_port_cfg.gpon_trf_map = xi_wan_ports_config->gpon ;
    wan_per_port_cfg.rnra_trf_map = xi_wan_ports_config->runner_a ;
    wan_per_port_cfg.rnrb_trf_map = xi_wan_ports_config->runner_b ;
    wan_per_port_cfg.pcie0_trf_map = xi_wan_ports_config->pcie0 ;
    wan_per_port_cfg.pcie1_trf_map = xi_wan_ports_config->pcie1 ;
    IH_REGS_GENERAL_CONFIGURATION_WAN_PER_PORT_CFG_WRITE( wan_per_port_cfg ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_configure_wan_ports ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_wan_ports_configuration                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get WAN ports configuration                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the WAN ports configuration                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_wan_ports_config - WAN ports configuration                            */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_wan_ports_configuration ( DRV_IH_WAN_PORTS_CONFIG * const xo_wan_ports_config )
{
    IH_REGS_GENERAL_CONFIGURATION_WAN_PER_PORT_CFG wan_per_port_cfg ;

    IH_REGS_GENERAL_CONFIGURATION_WAN_PER_PORT_CFG_READ( wan_per_port_cfg ) ;
    xo_wan_ports_config->eth0 = wan_per_port_cfg.eth0_trf_map ;
    xo_wan_ports_config->eth1 = wan_per_port_cfg.eth1_trf_map ;
    xo_wan_ports_config->eth2 = wan_per_port_cfg.eth2_trf_map ;
    xo_wan_ports_config->eth3 = wan_per_port_cfg.eth3_trf_map ;
    xo_wan_ports_config->eth4 = wan_per_port_cfg.eth4_trf_map ;
    xo_wan_ports_config->gpon = wan_per_port_cfg.gpon_trf_map ;
    xo_wan_ports_config->runner_a = wan_per_port_cfg.rnra_trf_map ;
    xo_wan_ports_config->runner_b = wan_per_port_cfg.rnrb_trf_map ;
    xo_wan_ports_config->pcie0 = wan_per_port_cfg.pcie0_trf_map ;
    xo_wan_ports_config->pcie1 = wan_per_port_cfg.pcie1_trf_map ;
    
    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_wan_ports_configuration ) ;

#ifndef _CFE_
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_allocated_runner_buffers_counters                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get allocated runner buffers counters                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the current number of allocated Runner Buffers of  */
/*   each runner.                                                             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_runner_a_counter - Runner A counter                                   */
/*                                                                            */
/*   xo_runner_b_counter - Runner B counter                                   */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_allocated_runner_buffers_counters ( uint32_t * const xo_runner_a_counter ,
                                                                         uint32_t * const xo_runner_b_counter )
{
    IH_REGS_GENERAL_CONFIGURATION_RBPM_BAC_STAT rbpm_bac_stat ;

    IH_REGS_GENERAL_CONFIGURATION_RBPM_BAC_STAT_READ( rbpm_bac_stat ) ;

    * xo_runner_a_counter = rbpm_bac_stat.rnra_bpm_bac ;
    * xo_runner_b_counter = rbpm_bac_stat.rnrb_bpm_bac ;
    
    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_allocated_runner_buffers_counters ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_critical_bits                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Critical Bits                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the status of the critical bits (debug             */
/*   indications).                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_critical_bits - Critical Bits                                         */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_critical_bits ( DRV_IH_CRITICAL_BITS * const xo_critical_bits )
{
    IH_REGS_GENERAL_CONFIGURATION_DBG_IQ_STAT dbg_iq_stat ;
    IH_REGS_GENERAL_CONFIGURATION_DBG_CRITICAL_STAT dbg_critical_stat ;

    IH_REGS_GENERAL_CONFIGURATION_DBG_IQ_STAT_READ( dbg_iq_stat ) ;
    xo_critical_bits->iq_fifo_full = dbg_iq_stat.iq_fifo_empty ;
    xo_critical_bits->iq0_fifo_full = dbg_iq_stat.iq0_fifo_empty ;
    xo_critical_bits->iq1_fifo_full = dbg_iq_stat.iq1_fifo_empty ;
    xo_critical_bits->iq2_fifo_full = dbg_iq_stat.iq2_fifo_empty ;
    xo_critical_bits->iq3_fifo_full = dbg_iq_stat.iq3_fifo_empty ;
    xo_critical_bits->iq4_fifo_full = dbg_iq_stat.iq4_fifo_empty ;
    xo_critical_bits->iq5_fifo_full = dbg_iq_stat.iq5_fifo_empty ;
    xo_critical_bits->iq6_fifo_full = dbg_iq_stat.iq6_fifo_empty ;
    xo_critical_bits->iq7_fifo_full = dbg_iq_stat.iq7_fifo_empty ;

    IH_REGS_GENERAL_CONFIGURATION_DBG_CRITICAL_STAT_READ( dbg_critical_stat ) ;
    /* the lookup-stuck indication is active low */
    xo_critical_bits->lookup_1_stuck = ! dbg_critical_stat.lkup1_stuck_n ;
    xo_critical_bits->lookup_2_stuck = ! dbg_critical_stat.lkup2_stuck_n ;
    xo_critical_bits->lookup_3_stuck = ! dbg_critical_stat.lkup3_stuck_n ;
    xo_critical_bits->lookup_4_stuck = ! dbg_critical_stat.lkup4_stuck_n ;
    xo_critical_bits->look_up_packet_command_fifo_full = dbg_critical_stat.lut_pkt_cmd_fifo_full ;
    xo_critical_bits->egress_tx_data_fifo_full = dbg_critical_stat.eq_datatx_fifo_full ;
    xo_critical_bits->egress_tx_message_fifo_full = dbg_critical_stat.eq_msgtx_fifo_full ;
    xo_critical_bits->egress_queue_packet_command_fifo_full = dbg_critical_stat.eq_pkt_cmd_fifo_full ;

    
    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_critical_bits ) ;
#endif

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_parser                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure Parser                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures general parameters in the parser accelerator in */
/*   IH.                                                                      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_parser_config - Parser Configuration                                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_parser ( const DRV_IH_PARSER_CONFIG * xi_parser_config )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_PARSER_CFG parser_cfg ;
    IH_REGS_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG ipv6_hdr_ext_fltr_mask_cfg ;
    IH_REGS_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE snap_org_code ;
    IH_REGS_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG gre_protocol_cfg ;
    IH_REGS_PARSER_CORE_CONFIGURATION_ENG  eng_cfg ;

    IH_REGS_PARSER_CORE_CONFIGURATION_PARSER_CFG_READ( parser_cfg ) ;
    parser_cfg.tcp_flags_filt = xi_parser_config->tcp_flags ;
    parser_cfg.exception_en = xi_parser_config->exception_status_bits ;
    parser_cfg.ppp_code_1_ipv6 = xi_parser_config->ppp_code_1_ipv6 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_PARSER_CFG_WRITE( parser_cfg ) ;

    IH_REGS_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_READ( ipv6_hdr_ext_fltr_mask_cfg ) ;
    ipv6_hdr_ext_fltr_mask_cfg.hop_by_hop_match = MS_GET_BIT_I( xi_parser_config->ipv6_extension_header_bitmask , 0 ) ;
    ipv6_hdr_ext_fltr_mask_cfg.routing_eh = MS_GET_BIT_I( xi_parser_config->ipv6_extension_header_bitmask , 1 ) ;
    ipv6_hdr_ext_fltr_mask_cfg.dest_opt_eh = MS_GET_BIT_I( xi_parser_config->ipv6_extension_header_bitmask , 2 ) ;
    IH_REGS_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_WRITE( ipv6_hdr_ext_fltr_mask_cfg ) ;

    IH_REGS_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_READ( snap_org_code ) ;
    snap_org_code.code = xi_parser_config->snap_user_defined_organization_code ;
    snap_org_code.en_rfc1042 = xi_parser_config->snap_rfc1042_encapsulation_enable ;
    snap_org_code.en_8021q = xi_parser_config->snap_802_1q_encapsulation_enable ;
    IH_REGS_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_WRITE( snap_org_code ) ;

    IH_REGS_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_READ( gre_protocol_cfg ) ;
    gre_protocol_cfg.gre_protocol = xi_parser_config->gre_protocol ;
    IH_REGS_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_WRITE( gre_protocol_cfg ) ;
    IH_REGS_PARSER_CORE_CONFIGURATION_ENG_READ( eng_cfg ) ;
    eng_cfg.cfg = xi_parser_config->eng_cfg ;
    IH_REGS_PARSER_CORE_CONFIGURATION_ENG_WRITE( eng_cfg ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_configure_parser ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_parser_configuration                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Parser configuration                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the general parameters configuration in the parser    */
/*   accelerator in IH.                                                       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_parser_config - Parser Configuration                                  */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_parser_configuration ( DRV_IH_PARSER_CONFIG * const xo_parser_config )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_PARSER_CFG parser_cfg ;
    IH_REGS_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE snap_org_code ;
    IH_REGS_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG gre_protocol_cfg ;
    IH_REGS_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG ipv6_hdr_ext_fltr_mask_cfg ;

    IH_REGS_PARSER_CORE_CONFIGURATION_PARSER_CFG_READ( parser_cfg ) ;
    xo_parser_config->tcp_flags = parser_cfg.tcp_flags_filt ;
    xo_parser_config->exception_status_bits = parser_cfg.exception_en ;
    xo_parser_config->ppp_code_1_ipv6 = parser_cfg.ppp_code_1_ipv6 ;

    IH_REGS_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_READ( ipv6_hdr_ext_fltr_mask_cfg ) ;
    MS_SET_BIT_I( xo_parser_config->ipv6_extension_header_bitmask , 0 , ipv6_hdr_ext_fltr_mask_cfg.hop_by_hop_match ) ;
    MS_SET_BIT_I( xo_parser_config->ipv6_extension_header_bitmask , 1 , ipv6_hdr_ext_fltr_mask_cfg.routing_eh ) ;
    MS_SET_BIT_I( xo_parser_config->ipv6_extension_header_bitmask , 2 , ipv6_hdr_ext_fltr_mask_cfg.dest_opt_eh ) ;
    
    IH_REGS_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_READ( snap_org_code ) ;
    xo_parser_config->snap_user_defined_organization_code = snap_org_code.code ;
    xo_parser_config->snap_rfc1042_encapsulation_enable = snap_org_code.en_rfc1042 ;
    xo_parser_config->snap_802_1q_encapsulation_enable = snap_org_code.en_8021q ;
    
    IH_REGS_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_READ( gre_protocol_cfg ) ;
    xo_parser_config->gre_protocol = gre_protocol_cfg.gre_protocol ;
    
    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_parser_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_da_filter_with_mask                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set DA Filter with Mask                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets a DA filter with mask. Allowed filter index: 0-1      */
/*   (only these filters has mask). The filter should be enabled using Enable */
/*   DA Filter API in order to take effect.                                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_mac_address - MAC address                                             */
/*                                                                            */
/*   xi_mask - Mask                                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_da_filter_with_mask ( uint8_t xi_filter_index ,
                                                           uint8_t xi_mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ,
                                                           uint8_t xi_mask [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] )
{
    /* we will use these variables for any filter, not only for #0 (the registers are similar) */
    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L da_filt_val_l ;
    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L da_filt_mask_l ;
    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT0_CFG_H da_filt_cfg_h ;
    int32_t result ;
    uint32_t address_4_ls_bytes ;
    uint16_t addres_2_ms_bytes ;


    result = f_check_item_index( xi_filter_index ,
                                 DRV_IH_NUMBER_OF_DA_FILTERS_WITH_MASK ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }


    /* in these register no need to read before modify, we are going to set the entire registers. */

    p_mac_address_array_to_hw_format ( xi_mac_address , & address_4_ls_bytes , & addres_2_ms_bytes ) ;
    da_filt_val_l.da_filt_lsb = address_4_ls_bytes ;
    da_filt_cfg_h.da_filt_val_msb = addres_2_ms_bytes ;

    p_mac_address_array_to_hw_format ( xi_mask , & address_4_ls_bytes , & addres_2_ms_bytes ) ;
    da_filt_mask_l.da_filt0_mask_l = address_4_ls_bytes ;
    da_filt_cfg_h.da_filt_mask_msb = addres_2_ms_bytes ;

    /* write to registers */
    switch ( xi_filter_index )
    {
    case 0:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_WRITE( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_WRITE( da_filt_mask_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT0_CFG_H_WRITE( da_filt_cfg_h ) ;
        break ;
    case 1:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_WRITE( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_WRITE( da_filt_mask_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT1_CFG_H_WRITE( da_filt_cfg_h ) ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_set_da_filter_with_mask ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_da_filter_with_mask                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get DA Filter with Mask                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets a DA filter with mask.                                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_mac_address - MAC address                                             */
/*                                                                            */
/*   xo_mask - Mask                                                           */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_da_filter_with_mask ( uint8_t xi_filter_index ,
                                                           uint8_t xo_mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ,
                                                           uint8_t xo_mask [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] )
{
    /* we will use these variables for any filter, not only for #0 (the registers are similar) */
    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L da_filt_val_l  	= {0};
    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L da_filt_mask_l	= {0};
    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT0_CFG_H da_filt_cfg_h		= {0};
    int32_t result ;


    result = f_check_item_index( xi_filter_index ,
                                 DRV_IH_NUMBER_OF_DA_FILTERS_WITH_MASK ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    /* read from registers */
    switch ( xi_filter_index )
    {
    case 0:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_READ( da_filt_mask_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT0_CFG_H_READ( da_filt_cfg_h ) ;
        break ;
    case 1:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_READ( da_filt_mask_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT1_CFG_H_READ( da_filt_cfg_h ) ;
        break ;
    }

    p_mac_address_hw_format_to_array ( da_filt_val_l.da_filt_lsb ,
                                       da_filt_cfg_h.da_filt_val_msb ,
                                       xo_mac_address ) ;

    p_mac_address_hw_format_to_array ( da_filt_mask_l.da_filt0_mask_l ,
                                       da_filt_cfg_h.da_filt_mask_msb ,
                                       xo_mask ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_da_filter_with_mask ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_da_filter_without_mask                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set DA Filter without mask                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets DA filter without mask. Allowed filter index: 2-7     */
/*   (only these filters don't have mask). The filter should be enabled using */
/*   Enable Filter API in order to take effect.                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_mac_address - MAC address                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_da_filter_without_mask ( uint8_t xi_filter_index ,
                                                              uint8_t xi_mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] )
{
    /* we will use these variables for any filter, not only for #2 (the registers are similar) */
    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L da_filt_val_l ;
    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H da_filt_val_h ;
    uint32_t address_4_ls_bytes ;
    uint16_t addres_2_ms_bytes ;


    if ( ( xi_filter_index < DRV_IH_NUMBER_OF_DA_FILTERS_WITH_MASK ) ||
         ( xi_filter_index >= DRV_IH_NUMBER_OF_DA_FILTERS ))
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    switch ( xi_filter_index )
    {
    case 2:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_READ( da_filt_val_h ) ;
        break ;
    case 3:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_READ( da_filt_val_h ) ;
        break ;
    case 4:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_READ( da_filt_val_h ) ;
        break ;
    case 5:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_READ( da_filt_val_h ) ;
        break ;
    case 6:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_READ( da_filt_val_h ) ;
        break ;
    case 7:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_READ( da_filt_val_h ) ;
        break ;
    }

    p_mac_address_array_to_hw_format ( xi_mac_address , & address_4_ls_bytes , & addres_2_ms_bytes ) ;
    da_filt_val_l.da_filt_lsb = address_4_ls_bytes ;
    da_filt_val_h.da_filt_msb = addres_2_ms_bytes ;

    switch ( xi_filter_index )
    {
    case 2:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_WRITE( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_WRITE( da_filt_val_h ) ;
        break ;
    case 3:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_WRITE( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_WRITE( da_filt_val_h ) ;
        break ;
    case 4:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_WRITE( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_WRITE( da_filt_val_h ) ;
        break ;
    case 5:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_WRITE( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_WRITE( da_filt_val_h ) ;
        break ;
    case 6:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_WRITE( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_WRITE( da_filt_val_h ) ;
        break ;
    case 7:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_WRITE( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_WRITE( da_filt_val_h ) ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_set_da_filter_without_mask ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_da_filter_without_mask                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get DA Filter without mask                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets DA filter without mask.                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_mac_address - MAC address                                             */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_da_filter_without_mask ( uint8_t xi_filter_index ,
                                                              uint8_t xo_mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] )
{
    /* we will use these variables for any filter, not only for #2 (the registers are similar) */
    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L da_filt_val_l ;
    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H da_filt_val_h ;

    if ( ( xi_filter_index < DRV_IH_NUMBER_OF_DA_FILTERS_WITH_MASK ) ||
         ( xi_filter_index >= DRV_IH_NUMBER_OF_DA_FILTERS ))
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    switch ( xi_filter_index )
    {
    case 2:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_READ( da_filt_val_h ) ;
        break ;
    case 3:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_READ( da_filt_val_h ) ;
        break ;
    case 4:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_READ( da_filt_val_h ) ;
        break ;
    case 5:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_READ( da_filt_val_h ) ;
        break ;
    case 6:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_READ( da_filt_val_h ) ;
        break ;
    case 7:
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_READ( da_filt_val_l ) ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_READ( da_filt_val_h ) ;
        break ;
    }

    p_mac_address_hw_format_to_array ( da_filt_val_l.da_filt_lsb ,
                                       da_filt_val_h.da_filt_msb ,
                                       xo_mac_address ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_da_filter_without_mask ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_enable_da_filter                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Enable DA Filter                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function enables/disables a DA filter. Before enabling a DA filter, */
/*   it has to be configured by Configure DA Filter with Mask API or          */
/*   Configure DA Filter without Mask API.                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_enable - Enable                                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_enable_da_filter ( uint8_t xi_filter_index ,
                                                    int32_t xi_enable )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG da_filt_valid_cfg ;
    int32_t result ;


    result = f_check_item_index( xi_filter_index ,
                                 DRV_IH_NUMBER_OF_DA_FILTERS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_READ( da_filt_valid_cfg ) ;

    switch ( xi_filter_index )
    {
    case 0:
        da_filt_valid_cfg.da_filt0_valid = xi_enable ;
        break ;
    case 1:
        da_filt_valid_cfg.da_filt1_valid = xi_enable ;
        break ;
    case 2:
        da_filt_valid_cfg.da_filt2_valid = xi_enable ;
        break ;
    case 3:
        da_filt_valid_cfg.da_filt3_valid = xi_enable ;
        break ;
    case 4:
        da_filt_valid_cfg.da_filt4_valid = xi_enable ;
        break ;
    case 5:
        da_filt_valid_cfg.da_filt5_valid = xi_enable ;
        break ;
    case 6:
        da_filt_valid_cfg.da_filt6_valid = xi_enable ;
        break ;
    case 7:
        da_filt_valid_cfg.da_filt7_valid = xi_enable ;
        break ;
    }
    
    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_WRITE( da_filt_valid_cfg ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_enable_da_filter ) ;

#ifndef _CFE_
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_da_filter_enable_status                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get DA filter enable status                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the enable status of a DA filter.                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_enable - Enable                                                       */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_da_filter_enable_status ( uint8_t xi_filter_index ,
                                                               int32_t * const xo_enable )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG da_filt_valid_cfg ;
    int32_t result ;


    result = f_check_item_index( xi_filter_index ,
                                 DRV_IH_NUMBER_OF_DA_FILTERS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_READ( da_filt_valid_cfg ) ;

    switch ( xi_filter_index )
    {
    case 0:
        * xo_enable = da_filt_valid_cfg.da_filt0_valid ;
        break ;
    case 1:
        * xo_enable = da_filt_valid_cfg.da_filt1_valid ;
        break ;
    case 2:
        * xo_enable = da_filt_valid_cfg.da_filt2_valid ;
        break ;
    case 3:
        * xo_enable = da_filt_valid_cfg.da_filt3_valid ;
        break ;
    case 4:
        * xo_enable = da_filt_valid_cfg.da_filt4_valid ;
        break ;
    case 5:
        * xo_enable = da_filt_valid_cfg.da_filt5_valid ;
        break ;
    case 6:
        * xo_enable = da_filt_valid_cfg.da_filt6_valid ;
        break ;
    case 7:
        * xo_enable = da_filt_valid_cfg.da_filt7_valid ;
        break ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_WRITE( da_filt_valid_cfg ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_da_filter_enable_status ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_ethertypes_for_qtag_identification                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Ethertypes for QTAG identification                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets two user-defined Ethertypes for QTAG (VLAN tag)       */
/*   identification.                                                          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_0 - Ethertype 0                                             */
/*                                                                            */
/*   xi_ethertype_1 - Ethertype 1                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_ethertypes_for_qtag_identification ( uint16_t xi_ethertype_0 ,
                                                                          uint16_t xi_ethertype_1 )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE qtag_ethtype ;

    IH_REGS_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_READ( qtag_ethtype ) ;
    qtag_ethtype.ethtype_qtag_0 = xi_ethertype_0 ;
    qtag_ethtype.ethtype_qtag_1 = xi_ethertype_1 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_WRITE( qtag_ethtype ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_set_ethertypes_for_qtag_identification ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_ethertypes_for_qtag_identification                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Ethertypes for QTAG identification                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the two user-defined Ethertypes for QTAG (VLAN tag)   */
/*   identification                                                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_ethertype_0 - Ethertype 0                                             */
/*                                                                            */
/*   xo_ethertype_1 - Ethertype 1                                             */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_ethertypes_for_qtag_identification ( uint16_t * const xo_ethertype_0 ,
                                                                          uint16_t * const xo_ethertype_1 )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE qtag_ethtype ;

    IH_REGS_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_READ( qtag_ethtype ) ;
    * xo_ethertype_0 = qtag_ethtype.ethtype_qtag_0 ;
    * xo_ethertype_1 = qtag_ethtype.ethtype_qtag_1 ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_ethertypes_for_qtag_identification ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_qtag_nesting                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure QTAG Nesting                                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures, for 6 possible Ethertypes, whether each        */
/*   Ethertype can be used for QTAG identification, as inner and as outer     */
/*   tag. Note that when packet has a single tag, parser treats it as outer   */
/*   tag. The first 2 Ethertypes indices are for the user defined Ethertypes  */
/*   configured by Set Ethertypes for QTAG Identification API.                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_index - Ethertype index                                     */
/*                                                                            */
/*   xi_use_as_outer - Use as outer                                           */
/*                                                                            */
/*   xi_use_as_inner - Use as inner                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_qtag_nesting ( DRV_IH_ETHERTYPE_FOR_QTAG_NESTING xi_ethertype_index ,
                                                          int32_t xi_use_as_outer ,
                                                          int32_t xi_use_as_inner )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_QTAG_NEST qtag_nest ;
    uint8_t qtag_nesting_coding = 0 ;
    int32_t result ;


    result = f_check_item_index( xi_ethertype_index ,
                                 DRV_IH_NUMBER_OF_ETHERTYPES_FOR_QTAG_NESTING ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    /* the coding is of 2 bits: the LSB is use_as_outer, the MSB is use_as_inner */
    MS_SET_BIT_I( qtag_nesting_coding , 0 , xi_use_as_outer ) ;
    MS_SET_BIT_I( qtag_nesting_coding , 1 , xi_use_as_inner ) ;

    IH_REGS_PARSER_CORE_CONFIGURATION_QTAG_NEST_READ( qtag_nest ) ;

    switch ( xi_ethertype_index )
    {
    case DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_USER_DEFIEND_0:
        qtag_nest.qtag0_nest = qtag_nesting_coding ;
        break ;
    case DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_USER_DEFIEND_1:
        qtag_nest.qtag1_nest = qtag_nesting_coding ;
        break ;
    case DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_8100:
        qtag_nest.qtag2_nest = qtag_nesting_coding ;
        break ;
    case DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_88A8:
        qtag_nest.qtag3_nest = qtag_nesting_coding ;
        break ;
    case DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_9100:
        qtag_nest.qtag4_nest = qtag_nesting_coding ;
        break ;
    case DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_9200:
        qtag_nest.qtag5_nest = qtag_nesting_coding ;
        break ;
    /* just to avoid compilation warning (we already checked validity of the index) */
    default:
        break ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_QTAG_NEST_WRITE( qtag_nest ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_configure_qtag_nesting ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_qtag_nesting_configuration                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get QTAG Nesting configuration                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the QTAG Nesting configuration of an Ethertype.       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_index - Ethertype index                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_use_as_outer - Use as outer                                           */
/*                                                                            */
/*   xo_use_as_inner - Use as inner                                           */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_qtag_nesting_configuration ( DRV_IH_ETHERTYPE_FOR_QTAG_NESTING xi_ethertype_index ,
                                                                  int32_t * const xo_use_as_outer ,
                                                                  int32_t * const xo_use_as_inner )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_QTAG_NEST qtag_nest ;
    uint8_t qtag_nesting_coding = 0 ;
    int32_t result ;


    result = f_check_item_index( xi_ethertype_index ,
                                 DRV_IH_NUMBER_OF_ETHERTYPES_FOR_QTAG_NESTING ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_QTAG_NEST_READ( qtag_nest ) ;

    switch ( xi_ethertype_index )
    {
    case DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_USER_DEFIEND_0:
        qtag_nesting_coding = qtag_nest.qtag0_nest ;
        break ;
    case DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_USER_DEFIEND_1:
        qtag_nesting_coding = qtag_nest.qtag1_nest ;
        break ;
    case DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_8100:
        qtag_nesting_coding = qtag_nest.qtag2_nest ;
        break ;
    case DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_88A8:
        qtag_nesting_coding = qtag_nest.qtag3_nest ;
        break ;
    case DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_9100:
        qtag_nesting_coding = qtag_nest.qtag4_nest ;
        break ;
    case DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_9200:
        qtag_nesting_coding = qtag_nest.qtag5_nest ;
        break ;
    /* just to avoid compilation warning (we already checked validity of the index) */
    default:
        break ;
    }

    /* the coding is of 2 bits: the LSB is use_as_outer, the MSB is use_as_inner */
    * xo_use_as_outer = MS_GET_BIT_I( qtag_nesting_coding , 0 ) ;
    * xo_use_as_inner = MS_GET_BIT_I( qtag_nesting_coding , 1 ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_qtag_nesting_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_user_ethertype                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure User Ethertype                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures user defined Ethertype, to be indicated in "L2  */
/*   Protocol" field in parser result. There are up to 4 user defined         */
/*   Ethertypes. For such an Ethertype, the API configures which L3 protocol  */
/*   comes after it, and its offset (for L3 parsing). In order to take        */
/*   effect, the user Ethertype should be enabled using Enable User Ethertype */
/*   API.                                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_index - Ethertype index                                     */
/*                                                                            */
/*   xi_ethertype_value - Ethertype value                                     */
/*                                                                            */
/*   xi_l3_protocol - L3 protocol                                             */
/*                                                                            */
/*   xi_l3_offset - L3 offset                                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_user_ethertype ( uint8_t xi_ethertype_index ,
                                                            uint16_t xi_ethertype_value ,
                                                            DRV_IH_L3_PROTOCOL xi_l3_protocol ,
                                                            uint8_t xi_l3_offset )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1 user_ethtype_0_1 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3 user_ethtype_2_3 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG user_ethtype_config ;
    int32_t result ;


    result = f_check_item_index( xi_ethertype_index ,
                                 DRV_IH_NUMBER_OF_USER_DEFINED_ETHERTYPES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    if ( xi_l3_offset > DRV_IH_MAXIMAL_L3_OFFSET )
    {
        return ( DRV_IH_ERROR_INVALID_L3_OFFSET ) ;
    }


    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_READ( user_ethtype_config ) ;

    switch ( xi_ethertype_index )
    {
    case 0:
        user_ethtype_config.ethtype_user_prot_0 = xi_l3_protocol ;
        user_ethtype_config.ethtype_user_offset_0 = xi_l3_offset ;

        IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_READ( user_ethtype_0_1 ) ;
        user_ethtype_0_1.ethype_0 = xi_ethertype_value ;
        IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_WRITE( user_ethtype_0_1 ) ;
        break ;
    case 1:
        user_ethtype_config.ethtype_user_prot_1 = xi_l3_protocol ;
        user_ethtype_config.ethtype_user_offset_1 = xi_l3_offset ;

        IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_READ( user_ethtype_0_1 ) ;
        user_ethtype_0_1.ethype_1 = xi_ethertype_value ;
        IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_WRITE( user_ethtype_0_1 ) ;
        break ;
    case 2:
        user_ethtype_config.ethtype_user_prot_2 = xi_l3_protocol ;
        user_ethtype_config.ethtype_user_offset_2 = xi_l3_offset ;

        IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_READ( user_ethtype_2_3 ) ;
        user_ethtype_2_3.ethype_2 = xi_ethertype_value ;
        IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_WRITE( user_ethtype_2_3 ) ;
        break ;
    case 3:
        user_ethtype_config.ethtype_user_prot_3 = xi_l3_protocol ;
        user_ethtype_config.ethtype_user_offset_3 = xi_l3_offset ;

        IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_READ( user_ethtype_2_3 ) ;
        user_ethtype_2_3.ethype_3 = xi_ethertype_value ;
        IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_WRITE( user_ethtype_2_3 ) ;
        break ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_WRITE( user_ethtype_config ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_configure_user_ethertype ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_user_ethertype_configuration                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get User Ethertype configuration                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets user defined Ethertype configuration.                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_index - Ethertype index                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_ethertype_value - Ethertype value                                     */
/*                                                                            */
/*   xo_l3_protocol - L3 protocol                                             */
/*                                                                            */
/*   xo_l3_offset - L3 offset                                                 */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_user_ethertype_configuration ( uint8_t xi_ethertype_index ,
                                                                    uint16_t * const xo_ethertype_value ,
                                                                    DRV_IH_L3_PROTOCOL * const xo_l3_protocol ,
                                                                    uint8_t * const xo_l3_offset )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1 user_ethtype_0_1 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3 user_ethtype_2_3 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG user_ethtype_config ;
    int32_t result ;


    result = f_check_item_index( xi_ethertype_index ,
                                 DRV_IH_NUMBER_OF_USER_DEFINED_ETHERTYPES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_READ( user_ethtype_config ) ;

    switch ( xi_ethertype_index )
    {
    case 0:
        * xo_l3_protocol = user_ethtype_config.ethtype_user_prot_0 ;
        * xo_l3_offset = user_ethtype_config.ethtype_user_offset_0 ;

        IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_READ( user_ethtype_0_1 ) ;
        * xo_ethertype_value = user_ethtype_0_1.ethype_0 ;
        break ;
    case 1:
        * xo_l3_protocol = user_ethtype_config.ethtype_user_prot_1 ;
        * xo_l3_offset = user_ethtype_config.ethtype_user_offset_1 ;

        IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_READ( user_ethtype_0_1 ) ;
        * xo_ethertype_value = user_ethtype_0_1.ethype_1 ;
        break ;
    case 2:
        * xo_l3_protocol = user_ethtype_config.ethtype_user_prot_2 ;
        * xo_l3_offset = user_ethtype_config.ethtype_user_offset_2 ;

        IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_READ( user_ethtype_2_3 ) ;
        * xo_ethertype_value = user_ethtype_2_3.ethype_2 ;
        break ;
    case 3:
        * xo_l3_protocol = user_ethtype_config.ethtype_user_prot_3 ;
        * xo_l3_offset = user_ethtype_config.ethtype_user_offset_3 ;

        IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_READ( user_ethtype_2_3 ) ;
        * xo_ethertype_value = user_ethtype_2_3.ethype_3 ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_user_ethertype_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_enable_user_ethertype                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Enable User Ethertype                                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function enables/disables a user-defined Ethertype which was        */
/*   configured by Configure User Ethertype API.                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_index - Ethertype index                                     */
/*                                                                            */
/*   xi_enable - Enable                                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_enable_user_ethertype ( uint8_t xi_ethertype_index ,
                                                         int32_t xi_enable )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG user_ethtype_config ;
    int32_t result ;


    result = f_check_item_index( xi_ethertype_index ,
                                 DRV_IH_NUMBER_OF_USER_DEFINED_ETHERTYPES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_READ( user_ethtype_config ) ;
    /* bit i of ethtype_user_en is the enable status of ethertype i */
    MS_SET_BIT_I( user_ethtype_config.ethtype_user_en , xi_ethertype_index , xi_enable ) ;
    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_WRITE( user_ethtype_config ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_enable_user_ethertype ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_user_ethertype_enable_status                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get User Ethertype enable status                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the enable status of a user-defined Ethertype.        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_index - Ethertype index                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_enable - Enable                                                       */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_user_ethertype_enable_status ( uint8_t xi_ethertype_index ,
                                                                    int32_t * const xo_enable )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG user_ethtype_config ;
    int32_t result ;


    result = f_check_item_index( xi_ethertype_index ,
                                 DRV_IH_NUMBER_OF_USER_DEFINED_ETHERTYPES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_READ( user_ethtype_config ) ;
    /* bit i of ethtype_user_en is the enable status of ethertype i */
    * xo_enable = MS_GET_BIT_I( user_ethtype_config.ethtype_user_en , xi_ethertype_index ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_user_ethertype_enable_status ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_user_ip_l4_protocol                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set user IP L4 protocol                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets a user-defined L4 Protocol ID to be matched to        */
/*   Protocol field in IP header and to be indicated in the output summary    */
/*   word. There are up to 4 user-defined L4 protocols.                       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_l4_protocol_index - L4 protocol index                                 */
/*                                                                            */
/*   xi_l4_protocol_value - L4 protocol value                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_user_ip_l4_protocol ( uint8_t xi_l4_protocol_index ,
                                                           uint8_t xi_l4_protocol_value )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_USER_IP_PROT user_ip_prot ;
    int32_t result ;


    result = f_check_item_index( xi_l4_protocol_index ,
                                 DRV_IH_NUMBER_OF_USER_DEFINED_IP_L4_PROTOCOLS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_USER_IP_PROT_READ( user_ip_prot ) ;

    switch ( xi_l4_protocol_index )
    {
    case 0:
        user_ip_prot.user_ip_prot_0 = xi_l4_protocol_value ;
        break ;
    case 1:
        user_ip_prot.user_ip_prot_1 = xi_l4_protocol_value ;
        break ;
    case 2:
        user_ip_prot.user_ip_prot_2 = xi_l4_protocol_value ;
        break ;
    case 3:
        user_ip_prot.user_ip_prot_3 = xi_l4_protocol_value ;
        break ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_USER_IP_PROT_WRITE( user_ip_prot ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_set_user_ip_l4_protocol ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_user_ip_l4_protocol                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get user IP L4 protocol                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets a user-defined L4 Protocol.                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_l4_protocol_index - L4 protocol index                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_l4_protocol_value - L4 protocol value                                 */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_user_ip_l4_protocol ( uint8_t xi_l4_protocol_index ,
                                                           uint8_t * const xo_l4_protocol_value )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_USER_IP_PROT user_ip_prot ;
    int32_t result ;


    result = f_check_item_index( xi_l4_protocol_index ,
                                 DRV_IH_NUMBER_OF_USER_DEFINED_IP_L4_PROTOCOLS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_USER_IP_PROT_READ( user_ip_prot ) ;

    switch ( xi_l4_protocol_index )
    {
    case 0:
        * xo_l4_protocol_value = user_ip_prot.user_ip_prot_0 ;
        break ;
    case 1:
        * xo_l4_protocol_value = user_ip_prot.user_ip_prot_1 ;
        break ;
    case 2:
        * xo_l4_protocol_value = user_ip_prot.user_ip_prot_2 ;
        break ;
    case 3:
        * xo_l4_protocol_value = user_ip_prot.user_ip_prot_3 ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_user_ip_l4_protocol ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_ppp_code                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set PPP code                                                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets PPP Protocol Code to indicate L3 is IP.               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ppp_code_index - PPP code index                                       */
/*                                                                            */
/*   xi_ppp_code - PPP code                                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_ppp_code ( uint8_t xi_ppp_code_index ,
                                                uint16_t xi_ppp_code )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE ppp_ip_prot_code ;
    int32_t result ;


    result = f_check_item_index( xi_ppp_code_index ,
                                 DRV_IH_NUMBER_OF_PPP_PROTOCOL_CODES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_READ( ppp_ip_prot_code ) ;

    switch ( xi_ppp_code_index )
    {
    case 0:
        ppp_ip_prot_code.ppp_code_0 = xi_ppp_code ;
        break ;
    case 1:
        ppp_ip_prot_code.ppp_code_1 = xi_ppp_code ;
        break ;
    }
    IH_REGS_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_WRITE( ppp_ip_prot_code ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_set_ppp_code ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_ppp_code                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get PPP code                                                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets PPP Protocol Code.                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ppp_code_index - PPP code index                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_ppp_code - PPP code                                                   */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_ppp_code ( uint8_t xi_ppp_code_index ,
                                                uint16_t * const xo_ppp_code )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE ppp_ip_prot_code ;
    int32_t result ;


    result = f_check_item_index( xi_ppp_code_index ,
                                 DRV_IH_NUMBER_OF_PPP_PROTOCOL_CODES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_READ( ppp_ip_prot_code ) ;

    switch ( xi_ppp_code_index )
    {
    case 0:
        * xo_ppp_code = ppp_ip_prot_code.ppp_code_0 ;
        break ;
    case 1:
        * xo_ppp_code = ppp_ip_prot_code.ppp_code_1 ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_ppp_code ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_vid_filter                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set VID filter                                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets a VID filter. There are up to 12 VID filters. The     */
/*   filter has to be enabled by Enable VID filter API in order to take       */
/*   effect.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_vid - VID                                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_vid_filter ( uint8_t xi_filter_index ,
                                                  uint16_t xi_vid )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1 vid_0_1 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3 vid_2_3 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5 vid_4_5 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7 vid_6_7 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9 vid_8_9 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11 vid_10_11 ;
    int32_t result ;


    result = f_check_item_index( xi_filter_index ,
                                 DRV_IH_NUMBER_OF_VID_FILTERS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    if ( xi_vid > DRV_IH_MAXIMAL_VID_VALUE )
    {
        return ( DRV_IH_ERROR_INVALID_VID ) ;
    }

    switch ( xi_filter_index )
    {
    case 0:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1_READ( vid_0_1 ) ;
        vid_0_1.vid_0 = xi_vid ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1_WRITE( vid_0_1 ) ;
        break ;
    case 1:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1_READ( vid_0_1 ) ;
        vid_0_1.vid_1 = xi_vid ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1_WRITE( vid_0_1 ) ;
        break ;
    case 2:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3_READ( vid_2_3 ) ;
        vid_2_3.vid_2 = xi_vid ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3_WRITE( vid_2_3 ) ;
        break ;
    case 3:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3_READ( vid_2_3 ) ;
        vid_2_3.vid_3 = xi_vid ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3_WRITE( vid_2_3 ) ;
        break ;
    case 4:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5_READ( vid_4_5 ) ;
        vid_4_5.vid_4 = xi_vid ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5_WRITE( vid_4_5 ) ;
        break ;
    case 5:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5_READ( vid_4_5 ) ;
        vid_4_5.vid_5 = xi_vid ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5_WRITE( vid_4_5 ) ;
        break ;
    case 6:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7_READ( vid_6_7 ) ;
        vid_6_7.vid_6 = xi_vid ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7_WRITE( vid_6_7 ) ;
        break ;
    case 7:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7_READ( vid_6_7 ) ;
        vid_6_7.vid_7 = xi_vid ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7_WRITE( vid_6_7 ) ;
        break ;
    case 8:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9_READ( vid_8_9 ) ;
        vid_8_9.vid_8 = xi_vid ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9_WRITE( vid_8_9 ) ;
        break ;
    case 9:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9_READ( vid_8_9 ) ;
        vid_8_9.vid_9 = xi_vid ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9_WRITE( vid_8_9 ) ;
        break ;
    case 10:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11_READ( vid_10_11 ) ;
        vid_10_11.vid_10 = xi_vid ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11_WRITE( vid_10_11 ) ;
        break ;
    case 11:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11_READ( vid_10_11 ) ;
        vid_10_11.vid_11 = xi_vid ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11_WRITE( vid_10_11 ) ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_set_vid_filter ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_vid_filter                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get VID filter                                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets a VID filter. There are up to 12 VID filters.         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_vid - VID                                                             */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_vid_filter ( uint8_t xi_filter_index ,
                                                  uint16_t * const xo_vid )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1 vid_0_1 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3 vid_2_3 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5 vid_4_5 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7 vid_6_7 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9 vid_8_9 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11 vid_10_11 ;
    int32_t result ;


    result = f_check_item_index( xi_filter_index ,
                                 DRV_IH_NUMBER_OF_VID_FILTERS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    switch ( xi_filter_index )
    {
    case 0:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1_READ( vid_0_1 ) ;
        * xo_vid = vid_0_1.vid_0 ;
        break ;
    case 1:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1_READ( vid_0_1 ) ;
        * xo_vid = vid_0_1.vid_1 ;
        break ;
    case 2:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3_READ( vid_2_3 ) ;
        * xo_vid = vid_2_3.vid_2 ;
        break ;
    case 3:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3_READ( vid_2_3 ) ;
        * xo_vid = vid_2_3.vid_3 ;
        break ;
    case 4:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5_READ( vid_4_5 ) ;
        * xo_vid = vid_4_5.vid_4 ;
        break ;
    case 5:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5_READ( vid_4_5 ) ;
        * xo_vid = vid_4_5.vid_5 ;
        break ;
    case 6:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7_READ( vid_6_7 ) ;
        * xo_vid = vid_6_7.vid_6 ;
        break ;
    case 7:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7_READ( vid_6_7 ) ;
        * xo_vid = vid_6_7.vid_7 ;
        break ;
    case 8:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9_READ( vid_8_9 ) ;
        * xo_vid = vid_8_9.vid_8 ;
        break ;
    case 9:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9_READ( vid_8_9 ) ;
        * xo_vid = vid_8_9.vid_9 ;
        break ;
    case 10:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11_READ( vid_10_11 ) ;
        * xo_vid = vid_10_11.vid_10 ;
        break ;
    case 11:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11_READ( vid_10_11 ) ;
        * xo_vid = vid_10_11.vid_11 ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_vid_filter ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_enable_vid_filter                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Enable VID filter                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function enables/disables a VID filter. There are up to 12 VID      */
/*   filters. Before enabling a filter, it should be configured by Set VID    */
/*   filter API.                                                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_enable - Enable                                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_enable_vid_filter ( uint8_t xi_filter_index ,
                                                     int32_t xi_enable )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1 vid_0_1 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3 vid_2_3 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5 vid_4_5 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7 vid_6_7 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9 vid_8_9 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11 vid_10_11 ;
    int32_t result ;


    result = f_check_item_index( xi_filter_index ,
                                 DRV_IH_NUMBER_OF_VID_FILTERS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    switch ( xi_filter_index )
    {
    case 0:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1_READ( vid_0_1 ) ;
        vid_0_1.vid_0_en = xi_enable ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1_WRITE( vid_0_1 ) ;
        break ;
    case 1:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1_READ( vid_0_1 ) ;
        vid_0_1.vid_1_en = xi_enable ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1_WRITE( vid_0_1 ) ;
        break ;
    case 2:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3_READ( vid_2_3 ) ;
        vid_2_3.vid_2_en = xi_enable ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3_WRITE( vid_2_3 ) ;
        break ;
    case 3:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3_READ( vid_2_3 ) ;
        vid_2_3.vid_3_en = xi_enable ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3_WRITE( vid_2_3 ) ;
        break ;
    case 4:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5_READ( vid_4_5 ) ;
        vid_4_5.vid_4_en = xi_enable ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5_WRITE( vid_4_5 ) ;
        break ;
    case 5:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5_READ( vid_4_5 ) ;
        vid_4_5.vid_5_en = xi_enable ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5_WRITE( vid_4_5 ) ;
        break ;
    case 6:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7_READ( vid_6_7 ) ;
        vid_6_7.vid_6_en = xi_enable ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7_WRITE( vid_6_7 ) ;
        break ;
    case 7:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7_READ( vid_6_7 ) ;
        vid_6_7.vid_7_en = xi_enable ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7_WRITE( vid_6_7 ) ;
        break ;
    case 8:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9_READ( vid_8_9 ) ;
        vid_8_9.vid_8_en = xi_enable ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9_WRITE( vid_8_9 ) ;
        break ;
    case 9:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9_READ( vid_8_9 ) ;
        vid_8_9.vid_9_en = xi_enable ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9_WRITE( vid_8_9 ) ;
        break ;
    case 10:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11_READ( vid_10_11 ) ;
        vid_10_11.vid_10_en = xi_enable ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11_WRITE( vid_10_11 ) ;
        break ;
    case 11:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11_READ( vid_10_11 ) ;
        vid_10_11.vid_11_en = xi_enable ;
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11_WRITE( vid_10_11 ) ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_enable_vid_filter ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_vid_filter_enable_status                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get VID filter enable status                                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the enable status of a VID filter. There are up to 12 */
/*   VID filters.                                                             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_enable - Enable                                                       */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_vid_filter_enable_status ( uint8_t xi_filter_index ,
                                                                int32_t * const xo_enable )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1 vid_0_1 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3 vid_2_3 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5 vid_4_5 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7 vid_6_7 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9 vid_8_9 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11 vid_10_11 ;
    int32_t result ;


    result = f_check_item_index( xi_filter_index ,
                                 DRV_IH_NUMBER_OF_VID_FILTERS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    switch ( xi_filter_index )
    {
    case 0:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1_READ( vid_0_1 ) ;
        * xo_enable = vid_0_1.vid_0_en ;
        break ;
    case 1:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_0_1_READ( vid_0_1 ) ;
        * xo_enable = vid_0_1.vid_1_en ;
        break ;
    case 2:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3_READ( vid_2_3 ) ;
        * xo_enable = vid_2_3.vid_2_en ;
        break ;
    case 3:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_2_3_READ( vid_2_3 ) ;
        * xo_enable = vid_2_3.vid_3_en ;
        break ;
    case 4:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5_READ( vid_4_5 ) ;
        * xo_enable = vid_4_5.vid_4_en ;
        break ;
    case 5:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_4_5_READ( vid_4_5 ) ;
        * xo_enable = vid_4_5.vid_5_en ;
        break ;
    case 6:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7_READ( vid_6_7 ) ;
        * xo_enable = vid_6_7.vid_6_en ;
        break ;
    case 7:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_6_7_READ( vid_6_7 ) ;
        * xo_enable = vid_6_7.vid_7_en ;
        break ;
    case 8:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9_READ( vid_8_9 ) ;
        * xo_enable = vid_8_9.vid_8_en ;
        break ;
    case 9:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_8_9_READ( vid_8_9 ) ;
        * xo_enable = vid_8_9.vid_9_en ;
        break ;
    case 10:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11_READ( vid_10_11 ) ;
        * xo_enable = vid_10_11.vid_10_en ;
        break ;
    case 11:
        IH_REGS_PARSER_CORE_CONFIGURATION_VID_10_11_READ( vid_10_11 ) ;
        * xo_enable = vid_10_11.vid_11_en ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_vid_filter_enable_status ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_ip_filter                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set IP filter                                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets an IP filter. There are up to 4 IP filters. The       */
/*   filter has to be enabled by Enable IP filter API in order to take        */
/*   effect.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_ip_address - IP address                                               */
/*                                                                            */
/*   xi_ip_address_mask - IP address mask                                     */
/*                                                                            */
/*   xi_selection - selection (SIP/DIP)                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_ip_filter ( uint8_t xi_filter_index ,
                                                 uint32_t xi_ip_address ,
                                                 uint32_t xi_ip_address_mask ,
                                                 DRV_IH_IP_FILTER_SELECTION xi_selection )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG ip_filter_cfg ;
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG ip_filter_mask_cfg ;
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG ip_filters_cfg ;
    int32_t result ;


    result = f_check_item_index( xi_filter_index ,
                                 DRV_IH_NUMBER_OF_IP_FILTERS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }


    MS_DRV_IH_PARSER_CORE_CONFIGURATION_IP_FILTER_CFG_READ_I( ip_filter_cfg , xi_filter_index ) ;
    ip_filter_cfg.ip_address = xi_ip_address ;
    MS_DRV_IH_PARSER_CORE_CONFIGURATION_IP_FILTER_CFG_WRITE_I( ip_filter_cfg , xi_filter_index ) ;

    MS_DRV_IH_PARSER_CORE_CONFIGURATION_IP_FILTER_MASK_CFG_READ_I( ip_filter_mask_cfg , xi_filter_index ) ;
    ip_filter_mask_cfg.ip_address_mask = xi_ip_address_mask ;
    MS_DRV_IH_PARSER_CORE_CONFIGURATION_IP_FILTER_MASK_CFG_WRITE_I( ip_filter_mask_cfg , xi_filter_index ) ;


    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_READ( ip_filters_cfg ) ;

    switch( xi_filter_index )
    {
    case 0:
        ip_filters_cfg.ip_filter0_dip_en = xi_selection ;
        break ;
    case 1:
        ip_filters_cfg.ip_filter1_dip_en = xi_selection ;
        break ;
    case 2:
        ip_filters_cfg.ip_filter2_dip_en = xi_selection ;
        break ;
    case 3:
        ip_filters_cfg.ip_filter3_dip_en = xi_selection ;
        break ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_WRITE( ip_filters_cfg ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_set_ip_filter ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_ip_filter                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get IP filter                                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets an IP filter. There are up to 4 IP filters.           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_ip_address - IP address                                               */
/*                                                                            */
/*   xo_ip_address_mask - IP address mask                                     */
/*                                                                            */
/*   xo_selection - selection (SIP/DIP)                                       */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_ip_filter ( uint8_t xi_filter_index ,
                                                 uint32_t * const xo_ip_address ,
                                                 uint32_t * const xo_ip_address_mask ,
                                                 DRV_IH_IP_FILTER_SELECTION * const xo_selection )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG ip_filter_cfg ;
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG ip_filter_mask_cfg ;
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG ip_filters_cfg ;
    int32_t result ;


    result = f_check_item_index( xi_filter_index ,
                                 DRV_IH_NUMBER_OF_IP_FILTERS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }


    MS_DRV_IH_PARSER_CORE_CONFIGURATION_IP_FILTER_CFG_READ_I( ip_filter_cfg , xi_filter_index ) ;
    * xo_ip_address = ip_filter_cfg.ip_address ;
    
    MS_DRV_IH_PARSER_CORE_CONFIGURATION_IP_FILTER_MASK_CFG_READ_I( ip_filter_mask_cfg , xi_filter_index ) ;
    * xo_ip_address_mask = ip_filter_mask_cfg.ip_address_mask ;
    

    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_READ( ip_filters_cfg ) ;

    switch( xi_filter_index )
    {
    case 0:
        * xo_selection = ip_filters_cfg.ip_filter0_dip_en ;
        break ;
    case 1:
        * xo_selection = ip_filters_cfg.ip_filter1_dip_en ;
        break ;
    case 2:
        * xo_selection = ip_filters_cfg.ip_filter2_dip_en ;
        break ;
    case 3:
        * xo_selection = ip_filters_cfg.ip_filter3_dip_en ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_ip_filter ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_enable_ip_filter                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Enable IP filter                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function enables/disables an IP filter. There are up to 4 IP        */
/*   filters. Before enabling a filter, it should be configured by Set IP     */
/*   filter API.                                                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_enable - Enable                                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_enable_ip_filter ( uint8_t xi_filter_index ,
                                                    int32_t xi_enable )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG ip_filters_cfg ;
    int32_t result ;


    result = f_check_item_index( xi_filter_index ,
                                 DRV_IH_NUMBER_OF_IP_FILTERS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }


    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_READ( ip_filters_cfg ) ;

    switch( xi_filter_index )
    {
    case 0:
        ip_filters_cfg.ip_filter0_valid = xi_enable ;
        break ;
    case 1:
        ip_filters_cfg.ip_filter1_valid = xi_enable ;
        break ;
    case 2:
        ip_filters_cfg.ip_filter2_valid = xi_enable ;
        break ;
    case 3:
        ip_filters_cfg.ip_filter3_valid = xi_enable ;
        break ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_WRITE( ip_filters_cfg ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_enable_ip_filter ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_ip_filter_enable_status                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get IP filter enable status                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the enable status of an IP filter. There are up to 4  */
/*   IP filters.                                                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_enable - Enable                                                       */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_ip_filter_enable_status ( uint8_t xi_filter_index ,
                                                               int32_t * const xo_enable )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG ip_filters_cfg ;
    int32_t result ;


    result = f_check_item_index( xi_filter_index ,
                                 DRV_IH_NUMBER_OF_IP_FILTERS ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }


    IH_REGS_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_READ( ip_filters_cfg ) ;

    switch( xi_filter_index )
    {
    case 0:
        * xo_enable = ip_filters_cfg.ip_filter0_valid ;
        break ;
    case 1:
        * xo_enable = ip_filters_cfg.ip_filter1_valid ;
        break ;
    case 2:
        * xo_enable = ip_filters_cfg.ip_filter2_valid ;
        break ;
    case 3:
        * xo_enable = ip_filters_cfg.ip_filter3_valid ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_ip_filter_enable_status ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_dscp_to_tci_table_entry                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set DSCP to TCI table entry                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets an entry in a DSCP to TCI table. There are 2 such     */
/*   tables. Each class is configured with one of these tables. The table has */
/*   to be enabled by Enable DSCP to TCI table API in order to take effect.   */
/*   If a class is configured with a table which is not enabled, the TCI will */
/*   be 0.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/*   xi_dscp - DSCP                                                           */
/*                                                                            */
/*   xi_tci - TCI                                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_dscp_to_tci_table_entry ( uint8_t xi_table_index ,
                                                               uint8_t xi_dscp ,
                                                               uint8_t xi_tci )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R0 dscp2tci_tbl_r ;
    uint8_t register_index ;
    uint8_t entry_index_in_register ;
    int32_t result ;


    result = f_check_item_index( xi_table_index ,
                                 DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    if ( xi_dscp > DRV_IH_MAXIMAL_DSCP_VALUE )
    {
        return ( DRV_IH_ERROR_INVALID_DSCP ) ;
    }

    if ( xi_tci > DRV_IH_MAXIMAL_TCI_VALUE )
    {
        return ( DRV_IH_ERROR_INVALID_TCI ) ;
    }


    /* the table is divided into 8 registers. each register contains 8 entries. */
    register_index = xi_dscp / CS_NUMBER_OF_ENTRIES_IN_DSCP_TO_TCI_TABLE_REGISTER ;
    entry_index_in_register = xi_dscp % CS_NUMBER_OF_ENTRIES_IN_DSCP_TO_TCI_TABLE_REGISTER ;


    if ( xi_table_index == 0 )
    {
        MS_DRV_IH_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R_READ_I( dscp2tci_tbl_r , register_index ) ;
    }
    else
    {
        MS_DRV_IH_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL1_R_READ_I( dscp2tci_tbl_r , register_index ) ;
    }

    switch ( entry_index_in_register )
    {
    case 0:
        dscp2tci_tbl_r.dscp_o0 = xi_tci ;
        break ;
    case 1:
        dscp2tci_tbl_r.dscp_o1 = xi_tci ;
        break ;
    case 2:
        dscp2tci_tbl_r.dscp_o2 = xi_tci ;
        break ;
    case 3:
        dscp2tci_tbl_r.dscp_o3 = xi_tci ;
        break ;
    case 4:
        dscp2tci_tbl_r.dscp_o4 = xi_tci ;
        break ;
    case 5:
        dscp2tci_tbl_r.dscp_o5 = xi_tci ;
        break ;
    case 6:
        dscp2tci_tbl_r.dscp_o6 = xi_tci ;
        break ;
    case 7:
        dscp2tci_tbl_r.dscp_o7 = xi_tci ;
        break ;
    }

    if ( xi_table_index == 0 )
    {
        MS_DRV_IH_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R_WRITE_I( dscp2tci_tbl_r , register_index ) ;
    }
    else
    {
        MS_DRV_IH_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL1_R_WRITE_I( dscp2tci_tbl_r , register_index ) ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_set_dscp_to_tci_table_entry ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_dscp_to_tci_table_entry                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get DSCP to TCI table entry                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets an entry in a DSCP to TCI table. There are 2 such     */
/*   tables.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/*   xi_dscp - DSCP                                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_tci - TCI                                                             */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_dscp_to_tci_table_entry ( uint8_t xi_table_index ,
                                                               uint8_t xi_dscp ,
                                                               uint8_t * const xo_tci )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R0 dscp2tci_tbl_r ;
    uint8_t register_index ;
    uint8_t entry_index_in_register ;
    int32_t result ;


    result = f_check_item_index( xi_table_index ,
                                 DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    if ( xi_dscp > DRV_IH_MAXIMAL_DSCP_VALUE )
    {
        return ( DRV_IH_ERROR_INVALID_DSCP ) ;
    }


    /* the table is divided into 8 registers. each register contains 8 entries. */
    register_index = xi_dscp / CS_NUMBER_OF_ENTRIES_IN_DSCP_TO_TCI_TABLE_REGISTER ;
    entry_index_in_register = xi_dscp % CS_NUMBER_OF_ENTRIES_IN_DSCP_TO_TCI_TABLE_REGISTER ;


    if ( xi_table_index == 0 )
    {
        MS_DRV_IH_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL0_R_READ_I( dscp2tci_tbl_r , register_index ) ;
    }
    else
    {
        MS_DRV_IH_PARSER_CORE_CONFIGURATION_DSCP2TCI_TBL1_R_READ_I( dscp2tci_tbl_r , register_index ) ;
    }

    switch ( entry_index_in_register )
    {
    case 0:
        * xo_tci = dscp2tci_tbl_r.dscp_o0 ;
        break ;
    case 1:
        * xo_tci = dscp2tci_tbl_r.dscp_o1 ;
        break ;
    case 2:
        * xo_tci = dscp2tci_tbl_r.dscp_o2 ;
        break ;
    case 3:
        * xo_tci = dscp2tci_tbl_r.dscp_o3 ;
        break ;
    case 4:
        * xo_tci = dscp2tci_tbl_r.dscp_o4 ;
        break ;
    case 5:
        * xo_tci = dscp2tci_tbl_r.dscp_o5 ;
        break ;
    case 6:
        * xo_tci = dscp2tci_tbl_r.dscp_o6 ;
        break ;
    case 7:
        * xo_tci = dscp2tci_tbl_r.dscp_o7 ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_dscp_to_tci_table_entry ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_default_tci                                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set default TCI                                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets default TCI, per DSCP to TCI table. The default is    */
/*   used in case of non-IP untagged packet. The default TCI will take effect */
/*   only after enabling the DSCP to TCI table, by Enable DSCP to TCI table   */
/*   API. If a class is configured with a table which is not enabled, the TCI */
/*   will be 0.                                                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/*   xi_default_tci - Default TCI                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_default_tci ( uint8_t xi_table_index ,
                                                   uint8_t xi_default_tci )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_DEFAULT_TCI_TBL0 default_tci_tbl0 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_DEFAULT_TCI_TBL1 default_tci_tbl1 ;
    int32_t result ;


    result = f_check_item_index( xi_table_index ,
                                 DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    if ( xi_default_tci > DRV_IH_MAXIMAL_TCI_VALUE )
    {
        return ( DRV_IH_ERROR_INVALID_TCI ) ;
    }


    switch ( xi_table_index )
    {
    case 0:
        IH_REGS_PARSER_CORE_CONFIGURATION_DEFAULT_TCI_TBL0_READ( default_tci_tbl0 ) ;
        default_tci_tbl0.default_tci = xi_default_tci ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DEFAULT_TCI_TBL0_WRITE( default_tci_tbl0 ) ;
        break ;
    case 1:
        IH_REGS_PARSER_CORE_CONFIGURATION_DEFAULT_TCI_TBL1_READ( default_tci_tbl1 ) ;
        default_tci_tbl1.default_tci = xi_default_tci ;
        IH_REGS_PARSER_CORE_CONFIGURATION_DEFAULT_TCI_TBL1_WRITE( default_tci_tbl1 ) ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_set_default_tci ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_default_tci                                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get default TCI                                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the default TCI, per DSCP to TCI table.               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_default_tci - Default TCI                                             */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_default_tci ( uint8_t xi_table_index ,
                                                   uint8_t * const xo_default_tci )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_DEFAULT_TCI_TBL0 default_tci_tbl0 ;
    IH_REGS_PARSER_CORE_CONFIGURATION_DEFAULT_TCI_TBL1 default_tci_tbl1 ;
    int32_t result ;


    result = f_check_item_index( xi_table_index ,
                                 DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }


    switch ( xi_table_index )
    {
    case 0:
        IH_REGS_PARSER_CORE_CONFIGURATION_DEFAULT_TCI_TBL0_READ( default_tci_tbl0 ) ;
        * xo_default_tci = default_tci_tbl0.default_tci ;
        break ;
    case 1:
        IH_REGS_PARSER_CORE_CONFIGURATION_DEFAULT_TCI_TBL1_READ( default_tci_tbl1 ) ;
        * xo_default_tci = default_tci_tbl1.default_tci ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_default_tci ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_enable_dscp_to_tci_table                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Enable DSCP to TCI table                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function enables/disables a DSCP to TCI table. There are 2 such     */
/*   tables. Each class is configured with one of these tables. Before        */
/*   enabling a table, it should be configured by Set DSCP to TCI table API   */
/*   and Set default TCI API.                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/*   xi_enable - Enable                                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_enable_dscp_to_tci_table ( uint8_t xi_table_index ,
                                                            int32_t xi_enable )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP_TBL_VALID_CFG dscp_tbl_valid_cfg ;
    int32_t result ;


    result = f_check_item_index( xi_table_index ,
                                 DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }


    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP_TBL_VALID_CFG_READ( dscp_tbl_valid_cfg ) ;

    switch ( xi_table_index )
    {
    case 0:
        dscp_tbl_valid_cfg.tbl0_valid = xi_enable ;
        break ;
    case 1:
        dscp_tbl_valid_cfg.tbl1_valid = xi_enable ;
        break ;
    }

    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP_TBL_VALID_CFG_WRITE( dscp_tbl_valid_cfg ) ;


    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_enable_dscp_to_tci_table ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_dscp_to_tci_table_enable_status                         */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get DSCP to TCI table enable status                          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the enable status of a DSCP to TCI table. There are 2 */
/*   such tables.                                                             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_enable - Enable                                                       */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_dscp_to_tci_table_enable_status ( uint8_t xi_table_index ,
                                                                       int32_t * const xo_enable )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP_TBL_VALID_CFG dscp_tbl_valid_cfg ;
    int32_t result ;


    result = f_check_item_index( xi_table_index ,
                                 DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }


    IH_REGS_PARSER_CORE_CONFIGURATION_DSCP_TBL_VALID_CFG_READ( dscp_tbl_valid_cfg ) ;

    switch ( xi_table_index )
    {
    case 0:
        * xo_enable = dscp_tbl_valid_cfg.tbl0_valid ;
        break ;
    case 1:
        * xo_enable = dscp_tbl_valid_cfg.tbl1_valid ;
        break ;
    }

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_get_dscp_to_tci_table_enable_status ) ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_parser_core_cfg_eng_3rd_tag_detection             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure Parser ENG register                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures the ENG register (bits 8-13 are for triple      */
/*   tag detection)                                                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_enable - ENG register bit value                                      */
/*                                                                            */
/*   xi_tpid_index - ENG register internal bit index                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                               */
/*     DRV_IH_NO_ERROR - No error                                             */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                             */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_parser_core_cfg_eng_3rd_tag_detection ( uint32_t xi_enable,
		uint8_t xi_tpid_index )
{
    IH_REGS_PARSER_CORE_CONFIGURATION_ENG  eng_cfg ;

    xi_tpid_index = xi_tpid_index + IH_ENG_TRIPLE_TAG_DETECTION_BIT_SHIFT;

    IH_REGS_PARSER_CORE_CONFIGURATION_ENG_READ( eng_cfg ) ;
    MS_SET_BIT_I( eng_cfg.cfg , xi_tpid_index , xi_enable );
    IH_REGS_PARSER_CORE_CONFIGURATION_ENG_WRITE( eng_cfg ) ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL( fi_bl_drv_ih_configure_parser_core_cfg_eng_3rd_tag_detection ) ;
#endif


/******************************************************************************/
/*                                                                            */
/* Internal functions implementation                                          */
/*                                                                            */
/******************************************************************************/


/* this function checks validity of item index, comparing to number of items.
   this function is intended for items whose legal indices are 0 to xi_number_of_items - 1.
   in case of invalid index, this function returns false and prints to logger a compatible
   message. otherwise, true is returned. */
int32_t f_check_item_index( uint32_t xi_item_index ,
                                    uint32_t xi_number_of_items )
{
    if ( xi_item_index >= xi_number_of_items )
    {

        return ( 0 ) ;
    }

    return ( 1 ) ;
}


/* this function configures all paramters of a lookup table, including
   "five tupple enable" paramter which is not exposed to the user.
   this function perfroms validity checks of parameters. */
DRV_IH_ERROR f_configure_lut_all_parameters ( uint8_t xi_table_index ,
                                                           const DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG * xi_lookup_table_60_bit_key_config ,
                                                           int32_t xi_five_tupple_enable )
{
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_LUT_CFG lkup_tbl_lut_cfg ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_CAM_CFG lkup_tbl_cam_cfg ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_LUT_CNXT_CFG lkup_tbl_lut_cnxt_cfg ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_CAM_CNXT_CFG lkup_tbl_cam_cnxt_cfg ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_CFG lkup_tbl_key_cfg ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_P0_MASKL lkup_tbl_key_p0_maskl ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_P0_MASKH lkup_tbl_key_p0_maskh ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_P1_MASKL lkup_tbl_key_p1_maskl ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_P1_MASKH lkup_tbl_key_p1_maskh ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_GL_MASK lkup_tbl_gl_mask ;
    int32_t result ;


    result = f_check_item_index( xi_table_index ,
                                 DRV_IH_NUMBER_OF_LOOKUP_TABLES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    if ( ( xi_lookup_table_60_bit_key_config->part_0_start_offset_in_4_byte > DRV_IH_MAXIMAL_START_OFFSET_SEATCH_KEY_PART ) || 
         ( xi_lookup_table_60_bit_key_config->part_1_start_offset_in_4_byte > DRV_IH_MAXIMAL_START_OFFSET_SEATCH_KEY_PART ) )
    {
        return ( DRV_IH_ERROR_INVALID_START_OFFSET_SEATCH_KEY_PART ) ;
    }

    if ( ( xi_lookup_table_60_bit_key_config->part_0_shift_offset_in_4_bit > DRV_IH_MAXIMAL_SHIFT_OFFSET_SEATCH_KEY_PART ) || 
         ( xi_lookup_table_60_bit_key_config->part_1_shift_offset_in_4_bit > DRV_IH_MAXIMAL_SHIFT_OFFSET_SEATCH_KEY_PART ) )
    {
        return ( DRV_IH_ERROR_INVALID_SHIFT_OFFSET_SEATCH_KEY_PART ) ;
    }


    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TLUT_CFG_READ_I( lkup_tbl_lut_cfg , xi_table_index ) ;
    lkup_tbl_lut_cfg.base_address = xi_lookup_table_60_bit_key_config->table_base_address_in_8_byte ;

    lkup_tbl_lut_cfg.table_size = xi_lookup_table_60_bit_key_config->table_size ;

    lkup_tbl_lut_cfg.max_hop = xi_lookup_table_60_bit_key_config->maximal_search_depth ;

    lkup_tbl_lut_cfg.hash_type = xi_lookup_table_60_bit_key_config->hash_type ;

    lkup_tbl_lut_cfg.sa_search_en = xi_lookup_table_60_bit_key_config->sa_search_enable ;

    lkup_tbl_lut_cfg.aging_en = xi_lookup_table_60_bit_key_config->aging_enable ;

    lkup_tbl_lut_cfg.five_tuple_en = xi_five_tupple_enable ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TLUT_CFG_WRITE_I( lkup_tbl_lut_cfg , xi_table_index ) ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TCAM_CFG_READ_I( lkup_tbl_cam_cfg , xi_table_index ) ;
    lkup_tbl_cam_cfg.cam_en = xi_lookup_table_60_bit_key_config->cam_enable ;
    lkup_tbl_cam_cfg.base_address = xi_lookup_table_60_bit_key_config->cam_base_address_in_8_byte ;
    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TCAM_CFG_WRITE_I( lkup_tbl_cam_cfg , xi_table_index ) ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TLUT_CNXT_CFG_READ_I( lkup_tbl_lut_cnxt_cfg , xi_table_index ) ;
    lkup_tbl_lut_cnxt_cfg.base_address = xi_lookup_table_60_bit_key_config->context_table_base_address_in_8_byte ;
    lkup_tbl_lut_cnxt_cfg.cnxt_entry_size = xi_lookup_table_60_bit_key_config->context_table_entry_size ;
    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TLUT_CNXT_CFG_WRITE_I( lkup_tbl_lut_cnxt_cfg , xi_table_index ) ;

    MS_DRV_IH_LOOKUP_LOOKUP_CONFIGURATION_LKUP_TCAM_CNXT_CFG_READ_I( lkup_tbl_cam_cnxt_cfg , xi_table_index ) ;
    lkup_tbl_cam_cnxt_cfg.base_address = xi_lookup_table_60_bit_key_config->cam_context_base_address_in_8_byte ;
    MS_DRV_IH_LOOKUP_LOOKUP_CONFIGURATION_LKUP_TCAM_CNXT_CFG_WRITE_I( lkup_tbl_cam_cnxt_cfg , xi_table_index ) ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_CFG_READ_I( lkup_tbl_key_cfg , xi_table_index ) ;
    lkup_tbl_key_cfg.start_offset_p0 = xi_lookup_table_60_bit_key_config->part_0_start_offset_in_4_byte ;
    lkup_tbl_key_cfg.shift_offset_p0 = xi_lookup_table_60_bit_key_config->part_0_shift_offset_in_4_bit ;
    lkup_tbl_key_cfg.start_offset_p1 = xi_lookup_table_60_bit_key_config->part_1_start_offset_in_4_byte ;
    lkup_tbl_key_cfg.shift_offset_p1 = xi_lookup_table_60_bit_key_config->part_1_shift_offset_in_4_bit ;
    lkup_tbl_key_cfg.key_ext = xi_lookup_table_60_bit_key_config->key_extension ;
    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_CFG_WRITE_I( lkup_tbl_key_cfg , xi_table_index ) ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P0_MASKL_READ_I( lkup_tbl_key_p0_maskl , xi_table_index ) ;
    lkup_tbl_key_p0_maskl.maskl = xi_lookup_table_60_bit_key_config->part_0_mask_low ;
    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P0_MASKL_WRITE_I( lkup_tbl_key_p0_maskl , xi_table_index ) ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P0_MASKH_READ_I( lkup_tbl_key_p0_maskh , xi_table_index ) ;
    lkup_tbl_key_p0_maskh.maskh = xi_lookup_table_60_bit_key_config->part_0_mask_high ;
    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P0_MASKH_WRITE_I( lkup_tbl_key_p0_maskh , xi_table_index ) ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P1_MASKL_READ_I( lkup_tbl_key_p1_maskl , xi_table_index ) ;
    lkup_tbl_key_p1_maskl.maskl = xi_lookup_table_60_bit_key_config->part_1_mask_low ;
    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P1_MASKL_WRITE_I( lkup_tbl_key_p1_maskl , xi_table_index ) ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P1_MASKH_READ_I( lkup_tbl_key_p1_maskh , xi_table_index ) ;
    lkup_tbl_key_p1_maskh.maskh = xi_lookup_table_60_bit_key_config->part_1_mask_high ;
    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P1_MASKH_WRITE_I( lkup_tbl_key_p1_maskh , xi_table_index ) ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TBL0_GL_MASK_READ_I( lkup_tbl_gl_mask , xi_table_index ) ;
    lkup_tbl_gl_mask.mask_nibble_code = xi_lookup_table_60_bit_key_config->global_mask_in_4_bit ;
    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TBL0_GL_MASK_WRITE_I( lkup_tbl_gl_mask , xi_table_index ) ;

    return ( DRV_IH_NO_ERROR ) ;
}


/* this function gets all paramters configuration of a lookup table, including
   "five tupple enable" paramter which is not exposed to the user. */
/* this function is exported for IHD shell commands */
DRV_IH_ERROR fi_get_lut_all_parameters ( uint8_t xi_table_index ,
                                                      DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG * const xo_lookup_table_60_bit_key_config ,
                                                      int32_t * const xo_five_tupple_enable )
{
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_LUT_CFG lkup_tbl_lut_cfg ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_CAM_CFG lkup_tbl_cam_cfg ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_LUT_CNXT_CFG lkup_tbl_lut_cnxt_cfg ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_CAM_CNXT_CFG lkup_tbl_cam_cnxt_cfg ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_CFG lkup_tbl_key_cfg ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_P0_MASKL lkup_tbl_key_p0_maskl ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_P0_MASKH lkup_tbl_key_p0_maskh ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_P1_MASKL lkup_tbl_key_p1_maskl ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_KEY_P1_MASKH lkup_tbl_key_p1_maskh ;
    IH_REGS_LOOKUP_CONFIGURATION_LKUP_TBL0_GL_MASK lkup_tbl_gl_mask ;
    int32_t result ;

    result = f_check_item_index( xi_table_index ,
                                 DRV_IH_NUMBER_OF_LOOKUP_TABLES ) ;
    if ( result == 0 )
    {
        return ( DRV_IH_ERROR_INVALID_INDEX ) ;
    }

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TLUT_CFG_READ_I( lkup_tbl_lut_cfg , xi_table_index ) ;
    xo_lookup_table_60_bit_key_config->table_base_address_in_8_byte = lkup_tbl_lut_cfg.base_address ;
    xo_lookup_table_60_bit_key_config->table_size = lkup_tbl_lut_cfg.table_size ;
    xo_lookup_table_60_bit_key_config->maximal_search_depth = lkup_tbl_lut_cfg.max_hop ;
    xo_lookup_table_60_bit_key_config->hash_type = lkup_tbl_lut_cfg.hash_type ;
    xo_lookup_table_60_bit_key_config->sa_search_enable = lkup_tbl_lut_cfg.sa_search_en ;
    xo_lookup_table_60_bit_key_config->aging_enable = lkup_tbl_lut_cfg.aging_en ;
    * xo_five_tupple_enable = lkup_tbl_lut_cfg.five_tuple_en ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TCAM_CFG_READ_I( lkup_tbl_cam_cfg , xi_table_index ) ;
    xo_lookup_table_60_bit_key_config->cam_enable = lkup_tbl_cam_cfg.cam_en ;
    xo_lookup_table_60_bit_key_config->cam_base_address_in_8_byte = lkup_tbl_cam_cfg.base_address ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TLUT_CNXT_CFG_READ_I( lkup_tbl_lut_cnxt_cfg , xi_table_index ) ;
    xo_lookup_table_60_bit_key_config->context_table_base_address_in_8_byte = lkup_tbl_lut_cnxt_cfg.base_address ;
    xo_lookup_table_60_bit_key_config->context_table_entry_size = lkup_tbl_lut_cnxt_cfg.cnxt_entry_size ;

    MS_DRV_IH_LOOKUP_LOOKUP_CONFIGURATION_LKUP_TCAM_CNXT_CFG_READ_I( lkup_tbl_cam_cnxt_cfg , xi_table_index ) ;
    xo_lookup_table_60_bit_key_config->cam_context_base_address_in_8_byte = lkup_tbl_cam_cnxt_cfg.base_address ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_CFG_READ_I( lkup_tbl_key_cfg , xi_table_index ) ;
    xo_lookup_table_60_bit_key_config->part_0_start_offset_in_4_byte = lkup_tbl_key_cfg.start_offset_p0 ;
    xo_lookup_table_60_bit_key_config->part_0_shift_offset_in_4_bit = lkup_tbl_key_cfg.shift_offset_p0 ;
    xo_lookup_table_60_bit_key_config->part_1_start_offset_in_4_byte = lkup_tbl_key_cfg.start_offset_p1 ;
    xo_lookup_table_60_bit_key_config->part_1_shift_offset_in_4_bit = lkup_tbl_key_cfg.shift_offset_p1 ;
    xo_lookup_table_60_bit_key_config->key_extension = lkup_tbl_key_cfg.key_ext ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P0_MASKL_READ_I( lkup_tbl_key_p0_maskl , xi_table_index ) ;
    xo_lookup_table_60_bit_key_config->part_0_mask_low = lkup_tbl_key_p0_maskl.maskl ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P0_MASKH_READ_I( lkup_tbl_key_p0_maskh , xi_table_index ) ;
    xo_lookup_table_60_bit_key_config->part_0_mask_high = lkup_tbl_key_p0_maskh.maskh ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P1_MASKL_READ_I( lkup_tbl_key_p1_maskl , xi_table_index ) ;
    xo_lookup_table_60_bit_key_config->part_1_mask_low = lkup_tbl_key_p1_maskl.maskl ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TKEY_P1_MASKH_READ_I( lkup_tbl_key_p1_maskh , xi_table_index ) ;
    xo_lookup_table_60_bit_key_config->part_1_mask_high = lkup_tbl_key_p1_maskh.maskh ;

    MS_DRV_IH_LOOKUP_CONFIGURATION_LKUP_TBL0_GL_MASK_READ_I( lkup_tbl_gl_mask , xi_table_index ) ;
    xo_lookup_table_60_bit_key_config->global_mask_in_4_bit = lkup_tbl_gl_mask.mask_nibble_code ;

    return ( DRV_IH_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_get_lut_all_parameters ) ;


/* checks in which common memory section a lookup table is located.
   this function assumes that xi_table_index is valid! */
DRV_IH_COMMON_MEMORY_SECTION_DTS f_get_lookup_table_location( uint8_t xi_table_index )
{
    DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG lookup_table_config ;
    int32_t five_tupple_enable ;
    uint8_t common_memory_section_selection_bit ;

    fi_get_lut_all_parameters ( xi_table_index ,
                                & lookup_table_config ,
                                & five_tupple_enable ) ;

    common_memory_section_selection_bit = MS_GET_BIT_I( lookup_table_config.table_base_address_in_8_byte ,
                                                        CS_COMMON_MEMORY_SECTION_SELECTION_BIT ) ;

    if ( common_memory_section_selection_bit == 0 )
    {

        return ( CS_COMMON_MEMORY_SECTION_A ) ;
    }
    else
    {

        return ( CS_COMMON_MEMORY_SECTION_B ) ;
    }
}


/* verifies that the lookup table of a given class search is located at the desired location */
int32_t f_verify_class_search_validity( DRV_IH_CLASS_SEARCH xi_class_search ,
                                                DRV_IH_COMMON_MEMORY_SECTION_DTS xi_desired_lookup_table_location )
{
    DRV_IH_COMMON_MEMORY_SECTION_DTS lookup_table_location ;

    if ( xi_class_search != DRV_IH_CLASS_SEARCH_DISABLED )
    {
        lookup_table_location = f_get_lookup_table_location( xi_class_search ) ;

        if ( lookup_table_location != xi_desired_lookup_table_location )
        {

            return ( 0 ) ;
        }
    }


    return ( 1 ) ;
}


/* returns true on success, false on failure */
int32_t f_multiply_by_2_table_size ( DRV_IH_LOOKUP_TABLE_SIZE xi_table_size ,
                                             DRV_IH_LOOKUP_TABLE_SIZE * xo_multiplied_table_size )
{
    /* table size is up to 4096 */
    if ( xi_table_size >= DRV_IH_LOOKUP_TABLE_SIZE_4096_ENTRIES )
    {
        return ( 0 ) ;
    }

    /* in the enumeration, each value means twice-size than previous value */
    * xo_multiplied_table_size = xi_table_size + 1 ;

    return ( 1 ) ;
}


/* returns true on success, false on failure */
int32_t f_divide_by_2_table_size ( DRV_IH_LOOKUP_TABLE_SIZE xi_table_size ,
                                           DRV_IH_LOOKUP_TABLE_SIZE * xo_divided_table_size )
{
    /* table size is at least 32 */
    if ( xi_table_size <= DRV_IH_LOOKUP_TABLE_SIZE_32_ENTRIES )
    {
        return ( 0 ) ;
    }

    /* in the enumeration, each value means twice-size than previous value */
    * xo_divided_table_size = xi_table_size - 1 ;

    return ( 1 ) ;
}


/* returns true on success, false on failure */
int32_t f_multiply_by_2_maximal_search_depth ( DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH xi_maximal_search_depth ,
                                                       DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH * xo_multiplied_maximal_search_depth )
{
    /* maximal search depth is up to 1024 */
    if ( xi_maximal_search_depth >= DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_1024_STEPS )
    {
        return ( 0 ) ;
    }

    /* in the enumeration, each value means twice-size than previous value */
    * xo_multiplied_maximal_search_depth = xi_maximal_search_depth + 1 ;

    return ( 1 ) ;
}


/* returns true on success, false on failure */
int32_t f_divide_by_2_maximal_search_depth ( DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH xi_maximal_search_depth ,
                                                     DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH * xo_divided_maximal_search_depth )
{
    /* maximal search depth is at least 1 */
    if ( xi_maximal_search_depth <= DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_1_STEP )
    {
        return ( 0 ) ;
    }

    /* in the enumeration, each value means twice-size than previous value */
    * xo_divided_maximal_search_depth = xi_maximal_search_depth - 1 ;

    return ( 1 ) ;
}


/* translates ingress queue priority from hw format (one hot) to 0-7 */
uint8_t f_translate_ingress_queue_priority ( uint8_t xi_priority_in_hw_format )
{
    uint8_t index ;

    for ( index = 0 ; index <= DRV_IH_MAXIMAL_INGRESS_QUEUE_PRIORITY ; ++ index )
    {
        if ( MS_GET_BIT_I( xi_priority_in_hw_format , index ) == 1 )
        {
            return ( index ) ;
        }
    }

    /* we shouldn't get here */
    return ( 0 ) ;
}


/* converts mac address from array to the format used by HW */
void p_mac_address_array_to_hw_format ( uint8_t xi_mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ,
                                        uint32_t * xo_address_4_ls_bytes ,
                                        uint16_t * xo_addres_2_ms_bytes )
{
    * xo_address_4_ls_bytes = ( xi_mac_address [ 2 ] << 24 ) |
                              ( xi_mac_address [ 3 ] << 16 ) |
                              ( xi_mac_address [ 4 ] << 8  ) |
                              ( xi_mac_address [ 5 ]       ) ;

    * xo_addres_2_ms_bytes =  ( xi_mac_address [ 0 ] << 8  ) |
                              ( xi_mac_address [ 1 ]       ) ;
}


/* converts mac address from the format used by HW to array */
void p_mac_address_hw_format_to_array ( uint32_t xi_address_4_ls_bytes ,
                                        uint16_t xi_addres_2_ms_bytes ,
                                        uint8_t xo_mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] )
{
    int  byte_index ;

    /* handle 4 LS bytes */
    for ( byte_index = DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS - 1 ; byte_index > 1 ; -- byte_index )
    {
        /* take the LS-byte only */
        xo_mac_address [ byte_index ] =   xi_address_4_ls_bytes & 0xFF ;
        /* one byte right shift */
        xi_address_4_ls_bytes >>= 8 ;
    }

    /* handle 2 MS bytes */
    for ( byte_index = 1 ; byte_index >= 0 ; -- byte_index )
    {
        /* take the LS-byte only */
        xo_mac_address [ byte_index ] =  xi_addres_2_ms_bytes & 0xFF;
        /* one byte right shift */
        xi_addres_2_ms_bytes >>= 8 ;
    }
}


/* this function is exported for IHD shell commands */
int32_t fi_is_class_configured ( uint8_t xi_class_index )
{
    return ( gs_class_is_configured [ xi_class_index ] ) ;
}
EXPORT_SYMBOL ( fi_is_class_configured ) ;

/* this function is exported for IHD shell commands */
int32_t fi_is_classifier_configured ( uint8_t xi_classifier_index )
{
    return ( gs_classifier_is_configured [ xi_classifier_index ] ) ;
}
EXPORT_SYMBOL ( fi_is_classifier_configured ) ;

