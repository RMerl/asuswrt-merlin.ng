// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
     
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

void p_mac_address_array_to_hw_format ( uint8_t xi_mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ,
                                        uint32_t * xo_address_4_ls_bytes ,
                                        uint16_t * xo_addres_2_ms_bytes ) ;

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
