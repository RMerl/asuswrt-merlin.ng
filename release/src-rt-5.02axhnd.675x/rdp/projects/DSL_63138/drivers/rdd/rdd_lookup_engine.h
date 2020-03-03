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

#ifndef _BL_LILAC_DRV_RUNNER_LOOKUP_ENGINE_H
#define _BL_LILAC_DRV_RUNNER_LOOKUP_ENGINE_H


#define MAC_ENTRY_KEY_MASK_HIGH                         0x0000FFFF
#define MAC_ENTRY_KEY_MASK_LOW                          0xFFFFFFFF
#define MAC_ENTRY_INTERNAL_CONTEXT_MASK_HIGH            0xFFF00000
#define MAC_ENTRY_INTERNAL_CONTEXT_MASK_LOW             0x00000000
#ifdef UNDEF
#define DS_FLOW_CLASSIFICATION_ENTRY_KEY_MASK_HIGH      0x00000000
#define DS_FLOW_CLASSIFICATION_ENTRY_KEY_MASK_LOW       0x00FFFFFF
#define US_FLOW_CLASSIFICATION_ENTRY_KEY_MASK_HIGH      0x00000000
#define US_FLOW_CLASSIFICATION_ENTRY_KEY_MASK_LOW       0x00FFFFFF
#else
#define INGRESS_CLASSIFICATION_IH_ENTRY_KEY_MASK_HIGH          0x000FFFFF
#define INGRESS_CLASSIFICATION_IH_ENTRY_KEY_MASK_LOW           0xFFFFFFFF
#define INGRESS_CLASSIFICATION_OPTIMIZED_ENTRY_KEY_MASK_HIGH   0x000FFFFF
#define INGRESS_CLASSIFICATION_OPTIMIZED_ENTRY_KEY_MASK_LOW    0xFFFFFFFF
#define INGRESS_CLASSIFICATION_SHORT_ENTRY_KEY_MASK_HIGH       0x000FFFFF
#define INGRESS_CLASSIFICATION_SHORT_ENTRY_KEY_MASK_LOW        0xFFFFFFFF
#define INGRESS_CLASSIFICATION_LONG_ENTRY_KEY_MASK_HIGH        0x00FFFFFF
#define INGRESS_CLASSIFICATION_LONG_ENTRY_KEY_MASK_LOW         0xFFFFFFFF
#endif
#define IPTV_ENTRY_KEY_MASK_HIGH                        0x0FFFFFFF
#define IPTV_ENTRY_KEY_MASK_LOW                         0xFFFFFFFF
#define IPTV_ENTRY_INTERNAL_CONTEXT_MASK_HIGH           0xFFF00000
#define IPTV_ENTRY_INTERNAL_CONTEXT_MASK_LOW            0x00000000
#define IPTV_L3_ENTRY_KEY_MASK_HIGH                     0x00000000
#define IPTV_L3_ENTRY_KEY_MASK_LOW                      0xFFFFFFFF
#define IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_HIGH          0x0FFFFFFF
#define IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_LOW           0xFFFFFFFF


typedef enum
{
    BL_LILAC_RDD_MAC_TABLE                             = 0,
#ifdef UNDEF
    BL_LILAC_RDD_DS_FLOW_CLASSIFICATION_TABLE = 1,
    BL_LILAC_RDD_US_FLOW_CLASSIFICATION_TABLE = 2,
#else
    BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_SHORT_TABLE = 1,
    BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_SHORT_TABLE = 2,
    BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_LONG_TABLE  = 3,
    BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_LONG_TABLE  = 4,
#endif
    BL_LILAC_RDD_IPTV_TABLE                            = 7,
    BL_LILAC_RDD_IPTV_SRC_IP_TABLE                     = 8,
    BL_LILAC_RDD_MAX_HASH_TABLE                        = 9,
}
BL_LILAC_RDD_HASH_TABLE_NUMBER_DTS;


typedef enum
{
    BL_LILAC_RDD_ADD_ENTRY    = 0,
    BL_LILAC_RDD_MODIFY_ENTRY = 1,
    BL_LILAC_RDD_REMOVE_ENTRY = 2,
}
BL_LILAC_RDD_HASH_TABLE_WRITE_TYPE_DTS;


typedef enum
{
    BL_LILAC_RDD_CONTEXT_8_BIT  = 1,
    BL_LILAC_RDD_CONTEXT_16_BIT = 2,
    BL_LILAC_RDD_CONTEXT_32_BIT = 4,
    BL_LILAC_RDD_CONTEXT_64_BIT = 8,
}
BL_LILAC_RDD_CONTEXT_ENTRY_SIZE_DTS;


typedef enum
{
    BL_LILAC_RDD_EXTERNAL_CONTEXT_DISABLE = 0,
    BL_LILAC_RDD_EXTERNAL_CONTEXT_ENABLE  = 1,
}
BL_LILAC_RDD_EXTERNAL_CONTEXT_FLAG_DTS;


typedef enum
{
    BL_LILAC_RDD_CAM_OPTIMIZATION_DISABLE = 0,
    BL_LILAC_RDD_CAM_OPTIMIZATION_ENABLE  = 1,
}
BL_LILAC_RDD_CAM_OPTIMIZATION_CONTROL_DTS;


/* This enumerated type can only include up to 4 values as a hardware limitation */
typedef enum
{
    BL_LILAC_RDD_DDR_GLOBAL_MASK_FIVE_TUPLE_KEY = 0x000001FF,
    BL_LILAC_RDD_DDR_GLOBAL_MASK_96_BIT_KEY     = 0x00000000,
    BL_LILAC_RDD_DDR_GLOBAL_MASK_127_BIT_KEY    = 0x7FFFFFFF,
    BL_LILAC_RDD_DDR_GLOBAL_MASK_RESERVED       = 0xFFFFFFFF
}
BL_LILAC_RDD_DDR_GLOBAL_MASK_DTS;


typedef struct
{
    uint8_t entry[ 16 ];
}
RDD_DDR_TABLE_ENTRY_DTS;


typedef struct
{
    uint8_t entry[ 8 ];
}
RDD_64_BIT_TABLE_ENTRY_DTS;


typedef struct
{
    uint8_t entry[ 4 ];
}
RDD_32_BIT_TABLE_ENTRY_DTS;


typedef struct
{
    uint8_t entry[ 2 ];
}
RDD_16_BIT_TABLE_ENTRY_DTS;


typedef struct
{
    uint8_t entry;
}
RDD_8_BIT_TABLE_ENTRY_DTS;


typedef struct
{
    RDD_64_BIT_TABLE_ENTRY_DTS           *hash_table_ptr;
    uint8_t                              *context_table_ptr;
    uint32_t                             hash_table_size;
    uint32_t                             hash_table_search_depth;
    uint32_t                             is_external_context;
    BL_LILAC_RDD_CONTEXT_ENTRY_SIZE_DTS  context_size;
    uint32_t                             is_extension_cam;
    RDD_64_BIT_TABLE_ENTRY_DTS           *cam_table_ptr;
    uint8_t                              *cam_context_table_ptr;
    uint32_t                             cam_table_size;
}
RDD_64_BIT_TABLE_CFG;


#define RDD_HASH_TABLE_ENTRY_READ( r, p, i )       MREAD_I_8( ( uint8_t *)p, i, r )
#define RDD_HASH_TABLE_ENTRY_WRITE( v, p, i )      MWRITE_I_8( ( uint8_t *)p, i, v )
#define RDD_HASH_TABLE_ENTRY_AGING_READ( r, p )    FIELD_MREAD_32( ( (uint8_t *)p + 4),  2,  1, r )
#define RDD_HASH_TABLE_ENTRY_SKIP_READ( r, p )     FIELD_MREAD_32( ( (uint8_t *)p + 4),  1,  1, r )
#define RDD_HASH_TABLE_ENTRY_VALID_READ( r, p )    FIELD_MREAD_32( ( (uint8_t *)p + 4),  0,  1, r )
#define RDD_HASH_TABLE_ENTRY_AGING_WRITE( v, p )   FIELD_MWRITE_32( ( (uint8_t *)p + 4 ), 2,  1, v )
#define RDD_HASH_TABLE_ENTRY_SKIP_WRITE( v, p )    FIELD_MWRITE_32( ( (uint8_t *)p + 4 ), 1,  1, v )
#define RDD_HASH_TABLE_ENTRY_VALID_WRITE( v, p )   FIELD_MWRITE_32( ( (uint8_t *)p + 4 ), 0,  1, v )


typedef struct
{
    uint32_t  valid		             :1  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t  cache_flag             :1  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t  context_entry_ptr      :25 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t  reserved_0             :5  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t  reserved_1             :32 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t  reserved_2             :32 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t  reserved_3             :32 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DDR_HASH_TABLE_ENTRY;


#define RDD_DDR_HASH_TABLE_ENTRY_VALID_READ( r, p )           FIELD_MREAD_32( ( (uint8_t *)p + 0 ), 31,  1, r )
#define RDD_DDR_HASH_TABLE_ENTRY_CACHE_FLAG_READ( r, p )      FIELD_MREAD_32( ( (uint8_t *)p + 0 ), 30,  1, r )
#define RDD_DDR_HASH_TABLE_ENTRY_CONTEXT_PTR_READ( r , p )    FIELD_MREAD_32( ( (uint8_t *)p + 0 ), 5,  25, r )
#define RDD_DDR_HASH_TABLE_ENTRY_VALID_WRITE( v, p )          FIELD_MWRITE_32( ( (uint8_t *)p + 0 ), 31,  1, v )
#define RDD_DDR_HASH_TABLE_ENTRY_CACHE_FLAG_WRITE( v, p )     FIELD_MWRITE_32( ( (uint8_t *)p + 0 ), 30,  1, v )
#define RDD_DDR_HASH_TABLE_ENTRY_CONTEXT_PTR_WRITE( v , p )   FIELD_MWRITE_32( ( (uint8_t *)p + 0 ), 5,  25, v )



typedef struct
{
    uint32_t  valid		             :1  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t  reserved_0             :31 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t  reserved_1             :32 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DDR_HASH_TABLE_CONTEXT_ENTRY;


#define RDD_DDR_HASH_TABLE_CONTEXT_ENTRY_VALID_READ( r, p )    FIELD_MREAD_32( ( (uint8_t *)p + 0 ), 31,  1, r )
#define RDD_DDR_HASH_TABLE_CONTEXT_ENTRY_VALID_WRITE( v, p )   FIELD_MWRITE_32( ( (uint8_t *)p + 0 ), 31,  1, v )



typedef struct
{
    RDD_DDR_TABLE_ENTRY_DTS              *hash_table_ptr;
    uint32_t                             hash_table_size;
    uint32_t                             hash_table_bucket_size;
    uint32_t                             is_external_context;
    uint8_t                              *context_table_ptr;
    BL_LILAC_RDD_CONTEXT_ENTRY_SIZE_DTS  context_size;
    uint32_t                             context_table_size;
    uint32_t                             non_cached_entries_counter;
    BL_LILAC_RDD_DDR_GLOBAL_MASK_DTS     global_mask;
    uint32_t                             context_entries_free_list_head;
    uint32_t                             context_entries_free_list_tail;
    uint16_t                             *context_entries_free_list;
    uint32_t                             context_table_offset;
    uint32_t                             *search_ddr_flag_address;
}
RDD_DDR_TABLE_CFG;



/* CRC */
#define RDD_CRC_TYPE_16                            0
#define RDD_CRC_TYPE_32                            1


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_find_hash_entry_64_bit                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Find key in hash table                                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function finds a key in a generic hash table and returns the        */
/*   status of the operation.                                                 */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_hash_table_cfg - hash table configuration data                      */
/*    *xi_hash_key - key to be searched in hash table                         */
/*    *xi_crc_init_value - initial value to be used in hashing the key        */
/*    xi_mask_high - mask for the high part of the entry                      */
/*    xi_mask_low - mask for the low part of the entry                        */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *xo_entry_index - used for returning the index that was found ( in      */
/*       case it was found )                                                  */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_find_hash_entry_64_bit ( RDD_64_BIT_TABLE_CFG  *xi_hash_table_cfg,
                                                    uint8_t               *xi_hash_key,
                                                    uint32_t              xi_mask_high,
                                                    uint32_t              xi_mask_low,
                                                    uint32_t              xi_crc_init_value,
                                                    uint32_t              *xo_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_find_entry_64_bit                                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Find key in hash or in cam table                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function searches hash, and if no match, cam tables                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_table_cfg - table configuration data                                */
/*    *xi_cam_key - key to be searched in table                               */
/*    xi_mask_high - mask for the high part of the entry                      */
/*    xi_mask_low - mask for the low part of the entry                        */
/*    xi_crc_init_value - initial crc value for hash                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *xo_entry_index - used for returning the index that was found ( in      */
/*    case it was found ). hit in CAM is marked by index >= hash table size   */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_find_entry_64_bit ( RDD_64_BIT_TABLE_CFG  *xi_table_cfg,
                                               uint8_t               *xi_key,
                                               uint32_t              xi_mask_high,
                                               uint32_t              xi_mask_low,
                                               uint32_t              xi_crc_init_value,
                                               uint32_t              *xo_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_write_control_bits                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Write control bits to hash table entry                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function writes control bits to entry, according to the passed      */
/*       write type.                                                          */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_table_ptr - pointer to first entry                                  */
/*    xi_table_size - table size ( used only for remove entry write type )    */
/*    xi_entry_index - index of the entry                                     */
/*    xi_write_type - add/modify/remove ( for hash table control bits )       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    none.                                                                   */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_write_control_bits ( RDD_64_BIT_TABLE_ENTRY_DTS              *xi_table_ptr,
                                                uint32_t                                xi_table_size,
                                                uint32_t                                xi_entry_index,
                                                BL_LILAC_RDD_HASH_TABLE_WRITE_TYPE_DTS  xi_write_type );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_add_hash_entry_64_bit                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   add hash table entry                                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function adds an entry (and optionally its context) to a hash       */
/*       table.                                                               */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_table_cfg - hash table configuration data                           */
/*    *xi_hash_entry_ptr - key to be hashed                                   */
/*    *xi_context_entry_ptr - external context to be written                  */
/*    xi_key_mask_high - mask for the high part of the entry                  */
/*    xi_key_mask_low - mask for the low part of the entry                    */
/*    xi_crc_init_value - initial value to be used in hashing the key         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *xo_entry_index - the index in table where the entry was added          */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_add_hash_entry_64_bit ( RDD_64_BIT_TABLE_CFG  *xi_table_cfg,
                                                   uint8_t               *xi_hash_entry_ptr,
                                                   uint8_t               *xi_context_entry_ptr,
                                                   uint32_t              xi_key_mask_high,
                                                   uint32_t              xi_key_mask_low,
                                                   uint32_t              xi_crc_init_value,
                                                   uint32_t              *xo_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_modify_hash_entry_64_bit                                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   modify hash table entry                                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function modifies an entry (and optionally its context) in a hash   */
/*       table.                                                               */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_table_cfg - table configuration data                                */
/*    *xi_hash_entry_ptr - key to be hashed                                   */
/*    xi_context_entry_ptr - context to be written                            */
/*    xi_key_mask_high - mask for the high part of the key                    */
/*    xi_key_mask_low - mask for the low part of the key                      */
/*    xi_internal_context_mask_high - mask for the high part of the entry     */
/*    xi_internal_context_mask_low - mask for the low part of the entry       */
/*    xi_crc_init_value - initial value to be used in hashing the key         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *xo_entry_index - the index in table where the entry was modified       */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_modify_hash_entry_64_bit ( RDD_64_BIT_TABLE_CFG  *xi_table_cfg,
                                                      uint8_t               *xi_hash_entry_ptr,
                                                      uint8_t               *xi_context_entry_ptr,
                                                      uint32_t              xi_key_mask_high,
                                                      uint32_t              xi_key_mask_low,
                                                      uint32_t              xi_internal_context_mask_high,
                                                      uint32_t              xi_internal_context_mask_low,
                                                      uint32_t              xi_crc_init_value,
                                                      uint32_t              *xo_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_remove_hash_entry_64_bit                                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   remove hash table entry                                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function removes an entry from a hash table                         */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_table_cfg - hash table configuration data                           */
/*    *xi_hash_entry_ptr - key to be hashed                                   */
/*    xi_crc_init_value - initial value to be used in hashing the key         */
/*    xi_key_mask_high - mask for the high part of the key                    */
/*    xi_key_mask_low - mask for the low part of the key                      */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *xo_entry_index - the index in table where the entry was removed        */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_remove_hash_entry_64_bit ( RDD_64_BIT_TABLE_CFG                       *xi_table_cfg,
                                                      uint8_t                                    *xi_hash_entry_ptr,
                                                      uint32_t                                   xi_key_mask_high,
                                                      uint32_t                                   xi_key_mask_low,
                                                      uint32_t                                   xi_crc_init_value,
                                                      BL_LILAC_RDD_CAM_OPTIMIZATION_CONTROL_DTS  xi_cam_optimization_control,
                                                      uint32_t                                   *xo_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_find_hash_entry_ddr                                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Find key in hash table                                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function finds a key in a generic hash table and returns the        */
/*   status of the operation.                                                 */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_hash_key - key to be searched in hash table                         */
/*    xi_hash_table_cfg_ptr - hash table configuration data                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *xo_entry_index - used for returning the index that was found ( in      */
/*       case it was found )                                                  */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_find_hash_entry_ddr ( RDD_DDR_TABLE_CFG  *xi_table_cfg_ptr,
                                                 uint8_t            *xi_hash_key,
                                                 uint32_t           *xo_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_add_hash_entry_ddr                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   add hash table entry                                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function adds an entry (and optionally its context) to a hash       */
/*       table.                                                               */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    xi_table_cfg_ptr - hash table configuration data                        */
/*    *xi_hash_entry_ptr - key to be hashed                                   */
/*    *xi_context_entry_ptr - external context to be written                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *xo_entry_index - the index in table where the entry was added          */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_add_hash_entry_ddr ( RDD_DDR_TABLE_CFG  *xi_table_cfg_ptr,
                                                uint8_t            *xi_hash_entry_ptr,
                                                uint8_t            *xi_context_entry_ptr,
                                                uint32_t           cache_flag,
                                                uint32_t           *xo_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_remove_hash_entry_ddr                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   remove hash table entry                                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function removes an entry from a hash table                         */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    xi_table_cfg_ptr - hash table configuration data                        */
/*    *xi_hash_entry_ptr - key to be hashed                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *xo_entry_index - the index in table where the entry was removed        */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_remove_hash_entry_ddr ( RDD_DDR_TABLE_CFG  *xi_table_cfg_ptr,
                                                   uint8_t            *xi_hash_entry_ptr,
                                                   uint32_t           *xo_entry_index );


#endif /* _BL_LILAC_DRV_RUNNER_LOOKUP_ENGINE_H */
