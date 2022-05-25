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

#include "rdd.h"


extern RDD_64_BIT_TABLE_CFG  g_hash_table_cfg[ BL_LILAC_RDD_MAX_HASH_TABLE ];


BL_LILAC_RDD_ERROR_DTE rdd_find_empty_hash_entry_64_bit ( RDD_64_BIT_TABLE_CFG *, uint8_t *, uint32_t, uint32_t * );
BL_LILAC_RDD_ERROR_DTE rdd_find_empty_cam_entry_64_bit ( RDD_64_BIT_TABLE_CFG  *, uint32_t * );
BL_LILAC_RDD_ERROR_DTE rdd_find_cam_entry_64_bit ( RDD_64_BIT_TABLE_CFG *, uint8_t *, uint32_t, uint32_t, uint32_t * );
BL_LILAC_RDD_ERROR_DTE rdd_write_entry_64_bit ( uint8_t *, RDD_64_BIT_TABLE_ENTRY_DTS *, uint32_t, uint32_t );
BL_LILAC_RDD_ERROR_DTE rdd_write_entry_32_bit ( uint8_t *, RDD_32_BIT_TABLE_ENTRY_DTS *, uint32_t );
BL_LILAC_RDD_ERROR_DTE rdd_write_entry_16_bit ( uint8_t *, RDD_16_BIT_TABLE_ENTRY_DTS *, uint32_t );
BL_LILAC_RDD_ERROR_DTE rdd_write_entry_8_bit ( uint8_t *, RDD_8_BIT_TABLE_ENTRY_DTS *, uint32_t );
BL_LILAC_RDD_ERROR_DTE rdd_write_external_context ( uint8_t *, uint8_t *, BL_LILAC_RDD_CONTEXT_ENTRY_SIZE_DTS, uint32_t, uint32_t, uint32_t );
BL_LILAC_RDD_ERROR_DTE rdd_left_shift_entry_64_bit ( uint8_t *, uint8_t, uint8_t * );
BL_LILAC_RDD_ERROR_DTE rdd_mask_entry_32_bit ( uint8_t *, uint32_t, uint8_t * );
BL_LILAC_RDD_ERROR_DTE rdd_find_empty_hash_entry_ddr ( RDD_DDR_TABLE_CFG *, uint8_t *, uint32_t * );
void rdd_write_entry_ddr ( uint8_t *, RDD_DDR_TABLE_ENTRY_DTS * );
void rdd_mask_entry_ddr ( uint8_t *, uint32_t, uint8_t *);
uint32_t ddr_hash_table_get_bucket_index ( RDD_DDR_TABLE_CFG *, uint8_t * );



/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_find_empty_hash_entry_64_bit                                         */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Find an empty entry in hash table                                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function finds an empty entry in a generic hash table and returns   */
/*    the status of the operation.                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_hash_table_cfg - hash table configuration data                      */
/*    *xi_hash_key - key to be searched in hash table                         */
/*    xi_crc_init_value - initial value to be used in hashing the key         */
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
BL_LILAC_RDD_ERROR_DTE rdd_find_empty_hash_entry_64_bit ( RDD_64_BIT_TABLE_CFG  *xi_hash_table_cfg,
                                                          uint8_t               *xi_hash_key,
                                                          uint32_t              xi_crc_init_value,
                                                          uint32_t              *xo_entry_index )
{
    RDD_64_BIT_TABLE_ENTRY_DTS  *entry_ptr;
    uint32_t                    tries;
    uint32_t                    hash_index, entry_index;
    uint32_t                    entry_valid, entry_skip;

    if ( xi_crc_init_value == 0 )
    {
        xi_crc_init_value = get_crc_init_value ( RDD_CRC_TYPE_16 );
    }

    xi_crc_init_value = crcbitbybit ( &xi_hash_key[ 0 ], 1, 4, xi_crc_init_value, RDD_CRC_TYPE_16 );

    /* calculate the CRC on the key */
    hash_index = crcbitbybit ( &xi_hash_key[ 2 ], 6, 0, xi_crc_init_value, RDD_CRC_TYPE_16 );

    hash_index = hash_index % xi_hash_table_cfg->hash_table_size;

    /* search for an empty line in the hash table for the new entry */
    /* limit the search to a preconfigured search depth             */
    for ( tries = 0; tries < xi_hash_table_cfg->hash_table_search_depth; tries++ )
    {
        entry_index = ( hash_index + tries ) % xi_hash_table_cfg->hash_table_size;

        /* read table entry */
        entry_ptr = xi_hash_table_cfg->hash_table_ptr + entry_index;

        /* empty line is either not valid or set as skip */
        RDD_HASH_TABLE_ENTRY_VALID_READ ( entry_valid, entry_ptr );
        RDD_HASH_TABLE_ENTRY_SKIP_READ ( entry_skip, entry_ptr );

        if ( !( entry_valid ) || ( entry_skip ) )
        {
            *xo_entry_index = entry_index;
            return ( BL_LILAC_RDD_OK );
        }
    }

    return ( BL_LILAC_RDD_ERROR_HASH_TABLE_NO_EMPTY_ENTRY );
}


BL_LILAC_RDD_ERROR_DTE rdd_find_hash_entry_64_bit ( RDD_64_BIT_TABLE_CFG  *xi_hash_table_cfg,
                                                    uint8_t               *xi_hash_key,
                                                    uint32_t              xi_mask_high,
                                                    uint32_t              xi_mask_low,
                                                    uint32_t              xi_crc_init_value,
                                                    uint32_t              *xo_entry_index )
{
    RDD_64_BIT_TABLE_ENTRY_DTS  *entry_ptr;
    uint8_t                     entry[ 8 ];
    uint8_t                     key_container[ 8 ];
    uint32_t                    tries;
    uint32_t                    hash_index, entry_index;
    uint32_t                    entry_valid, entry_skip;
    uint32_t                    i;
    uint8_t                     hash_key[ 8 ];

    /* Calculate the CRC on the key */
    if ( xi_crc_init_value == 0 )
    {
        xi_crc_init_value = get_crc_init_value ( RDD_CRC_TYPE_16 );
    }

    /* Mask the key container */
    rdd_mask_entry_32_bit ( &xi_hash_key[ 0 ], xi_mask_high, &hash_key[ 0 ] );
    rdd_mask_entry_32_bit ( &xi_hash_key[ 4 ], xi_mask_low, &hash_key[ 4 ] );

    xi_crc_init_value = crcbitbybit ( &hash_key[ 0 ], 1, 4, xi_crc_init_value, RDD_CRC_TYPE_16 );

    hash_index = crcbitbybit ( &hash_key[ 2 ], 6, 0, xi_crc_init_value, RDD_CRC_TYPE_16 );

    hash_index = hash_index % xi_hash_table_cfg->hash_table_size;

    /* Prepare key for comparison */
    rdd_left_shift_entry_64_bit ( hash_key, 4, key_container );

    /* Search for an empty line in the hash table for the new entry */
    /* Limit the search to a preconfigured search depth             */
    for ( tries = 0; tries < xi_hash_table_cfg->hash_table_search_depth; tries++ )
    {
        entry_index = ( hash_index + tries ) % xi_hash_table_cfg->hash_table_size;

        /* Read table entry */
        entry_ptr = xi_hash_table_cfg->hash_table_ptr + entry_index;

        /* Skip over reusable entries */
        RDD_HASH_TABLE_ENTRY_SKIP_READ ( entry_skip, entry_ptr );

        if ( entry_skip )
        {
            continue;
        }

        /* Valid but not skipped entries should be searched */
        RDD_HASH_TABLE_ENTRY_VALID_READ ( entry_valid, entry_ptr );

        if ( entry_valid )
        {
            /* Read 64 bit entry */
            for ( i = 0; i < 8; i++ )
            {
                RDD_HASH_TABLE_ENTRY_READ ( entry[ i ], entry_ptr, i );
            }

            /* Mask the entry */
            rdd_mask_entry_32_bit ( &entry[ 0 ], ( xi_mask_high << 4 ) | ( xi_mask_low >> 28 ), &entry[ 0 ] );
            rdd_mask_entry_32_bit ( &entry[ 4 ], xi_mask_low << 4, &entry[ 4 ] );

            /* Compare the entry to the key container */
            if ( ( entry[ 0 ] == key_container[ 0 ] ) &&
                 ( entry[ 1 ] == key_container[ 1 ] ) &&
                 ( entry[ 2 ] == key_container[ 2 ] ) &&
                 ( entry[ 3 ] == key_container[ 3 ] ) &&
                 ( entry[ 4 ] == key_container[ 4 ] ) &&
                 ( entry[ 5 ] == key_container[ 5 ] ) &&
                 ( entry[ 6 ] == key_container[ 6 ] ) &&
                 ( entry[ 7 ] == key_container[ 7 ] ) )
            {
                *xo_entry_index = entry_index;
                return ( BL_LILAC_RDD_OK );
            }
        }
        else
        {
            break;
        }
    }

    return ( BL_LILAC_RDD_ERROR_HASH_TABLE_NO_MATCHING_KEY );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_find_empty_cam_entry_64_bit                                          */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Find an empty entry in cam table                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function finds an empty entry in a generic cam table and returns    */
/*    the status of the operation.                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_table_cfg - cam table configuration data                            */
/*    *xi_hash_key - key to be searched in cam table                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *xo_entry_index - used for returning the index that was found ( in      */
/*       case it was found )                                                  */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_find_empty_cam_entry_64_bit ( RDD_64_BIT_TABLE_CFG  *xi_table_cfg,
                                                         uint32_t              *xo_entry_index )
{
   RDD_64_BIT_TABLE_ENTRY_DTS  *entry_ptr;
   uint32_t                    entry_index;
   uint32_t                    entry_valid;
   uint32_t                    entry_skip;

    for ( entry_index = 0; entry_index < xi_table_cfg->cam_table_size; entry_index++ )
    {
        /* read CAM table entry */
        entry_ptr = xi_table_cfg->cam_table_ptr + entry_index;

        /* empty line is either not valid or set as skip */
        RDD_HASH_TABLE_ENTRY_VALID_READ ( entry_valid, entry_ptr );
        RDD_HASH_TABLE_ENTRY_SKIP_READ ( entry_skip, entry_ptr );

        if ( !entry_valid || entry_skip )
        {
            /* return the index of the entry in the table */
            *xo_entry_index = entry_index;
            return ( BL_LILAC_RDD_OK );
        }
    }

    return ( BL_LILAC_RDD_ERROR_HASH_TABLE_NO_EMPTY_ENTRY );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_find_cam_entry_64_bit                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Find key in cam table                                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function finds a key in a generic cam table and returns the         */
/*   status of the operation.                                                 */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_table_cfg - table configuration data                                */
/*    *xi_cam_key - key to be searched in cam table                           */
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
BL_LILAC_RDD_ERROR_DTE rdd_find_cam_entry_64_bit ( RDD_64_BIT_TABLE_CFG  *xi_table_cfg,
                                                   uint8_t               *xi_cam_key,
                                                   uint32_t              xi_mask_high,
                                                   uint32_t              xi_mask_low,
                                                   uint32_t              *xo_entry_index )
{
    RDD_64_BIT_TABLE_ENTRY_DTS  *entry_ptr;
    uint8_t                     entry[ 8 ];
    uint8_t                     key_container[ 8 ];
    uint32_t                    entry_index;
    uint32_t                    entry_valid;
    uint32_t                    entry_skip;
    uint32_t                    i;

    /* Prepare key for comparison */
    rdd_left_shift_entry_64_bit ( xi_cam_key, 4, key_container );

    /* Mask the key container */
    rdd_mask_entry_32_bit ( &key_container[ 0 ], ( xi_mask_high << 4 ) | ( xi_mask_low >> 28 ), &key_container[ 0 ] );
    rdd_mask_entry_32_bit ( &key_container[ 4 ], xi_mask_low << 4, &key_container[ 4 ] );

    /* Search for an empty line in the hash table for the new entry */
    /* Limit the search to a preconfigured search depth             */
    for ( entry_index = 0; entry_index < xi_table_cfg->cam_table_size; entry_index++ )
    {
        /* Read table entry */
        entry_ptr = xi_table_cfg->cam_table_ptr + entry_index;

        /* Valid but not skipped entries should be searched */
        RDD_HASH_TABLE_ENTRY_VALID_READ ( entry_valid, entry_ptr );
        RDD_HASH_TABLE_ENTRY_SKIP_READ ( entry_skip, entry_ptr );

        if ( entry_valid )
        {
            /* Read 64 bit entry */
            for ( i = 0; i < 8; i++ )
            {
                RDD_HASH_TABLE_ENTRY_READ ( entry[ i ], entry_ptr, i );
            }

            /* Mask the entry */
            rdd_mask_entry_32_bit ( &entry[ 0 ], ( xi_mask_high << 4 ) | ( xi_mask_low >> 28 ), &entry[ 0 ] );
            rdd_mask_entry_32_bit ( &entry[ 4 ], xi_mask_low << 4, &entry[ 4 ] );

            /* Compare the entry to the key container */
            if ( ( entry[ 0 ] == key_container[ 0 ] ) &&
                 ( entry[ 1 ] == key_container[ 1 ] ) &&
                 ( entry[ 2 ] == key_container[ 2 ] ) &&
                 ( entry[ 3 ] == key_container[ 3 ] ) &&
                 ( entry[ 4 ] == key_container[ 4 ] ) &&
                 ( entry[ 5 ] == key_container[ 5 ] ) &&
                 ( entry[ 6 ] == key_container[ 6 ] ) &&
                 ( entry[ 7 ] == key_container[ 7 ] ) )
            {
                *xo_entry_index = entry_index;
                return ( BL_LILAC_RDD_OK );
            }
        }
        else if ( entry_skip )
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return ( BL_LILAC_RDD_ERROR_HASH_TABLE_NO_MATCHING_KEY );
}


BL_LILAC_RDD_ERROR_DTE rdd_find_entry_64_bit ( RDD_64_BIT_TABLE_CFG  *xi_table_cfg,
                                               uint8_t               *xi_key,
                                               uint32_t              xi_mask_high,
                                               uint32_t              xi_mask_low,
                                               uint32_t              xi_crc_init_value,
                                               uint32_t              *xo_entry_index )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    rdd_error = rdd_find_hash_entry_64_bit ( xi_table_cfg,
                                             xi_key,
                                             xi_mask_high,
                                             xi_mask_low,
                                             xi_crc_init_value,
                                             xo_entry_index );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        return ( BL_LILAC_RDD_OK );
    }

    /* find the requested entry in the CAM */
    rdd_error = rdd_find_cam_entry_64_bit ( xi_table_cfg,
                                            xi_key,
                                            xi_mask_high,
                                            xi_mask_low,
                                            xo_entry_index );

    *xo_entry_index += xi_table_cfg->hash_table_size;

    return ( rdd_error );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_write_entry_64_bit                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Write to hash table entry                                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function writes the passed entry (the masked part), to the passed   */
/*   entry pointer and and returns the status of the operation.               */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_entry - entry to be written                                         */
/*    *xi_entry_ptr - pointer to the entry in the hash table to where the     */
/*       entry should be written                                              */
/*    xi_mask_hi - mask for the high part of the entry                        */
/*    xi_mask_lo - mask for the low part of the entry                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    none.                                                                   */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_write_entry_64_bit ( uint8_t                     *xi_entry,
                                                RDD_64_BIT_TABLE_ENTRY_DTS  *xi_entry_ptr,
                                                uint32_t                    xi_mask_high,
                                                uint32_t                    xi_mask_low )
{
    uint8_t   entry_container[ 8 ];
    uint32_t  i;

    /* Read the contents of the entry */
    for ( i = 0; i < 8; i++ )
    {
        RDD_HASH_TABLE_ENTRY_READ ( entry_container[ i ], xi_entry_ptr, i );
    }

    rdd_mask_entry_32_bit ( &entry_container[ 0 ], ~xi_mask_high, &entry_container[ 0 ] );
    rdd_mask_entry_32_bit ( &xi_entry[ 0 ], xi_mask_high, &xi_entry[ 0 ] );

    rdd_mask_entry_32_bit ( &entry_container[ 4 ], ~xi_mask_low, &entry_container[ 4 ] );
    rdd_mask_entry_32_bit ( &xi_entry[ 4 ], xi_mask_low, &xi_entry[ 4 ] );

    for ( i = 0; i < 8; i++ )
    {
        entry_container[ i ] |= xi_entry[ i ];
        RDD_HASH_TABLE_ENTRY_WRITE ( entry_container[ i ], xi_entry_ptr, i );
    }

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_write_entry_32_bit                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Write to table entry                                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function writes the passed entry (the masked part), to the passed   */
/*   entry pointer and and returns the status of the operation.               */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_entry - entry to be written                                         */
/*    *xi_entry_ptr - pointer to the entry in the hash table to where the     */
/*       entry should be written                                              */
/*    xi_mask - mask for the entry                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    none.                                                                   */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_write_entry_32_bit ( uint8_t                     *xi_entry,
                                                RDD_32_BIT_TABLE_ENTRY_DTS  *xi_entry_ptr,
                                                uint32_t                    xi_mask )
{
    uint8_t   entry_container[ 4 ];
    uint32_t  i;

    /* Read the contents of the entry */
    for ( i = 0; i < 4; i++ )
    {
        RDD_HASH_TABLE_ENTRY_READ ( entry_container[ i ], xi_entry_ptr, i );
    }

    rdd_mask_entry_32_bit ( entry_container, ~xi_mask, entry_container );
    rdd_mask_entry_32_bit ( xi_entry, xi_mask, xi_entry );

    for ( i = 0; i < 4; i++ )
    {
        entry_container[ i ] |= xi_entry[ i ];
        RDD_HASH_TABLE_ENTRY_WRITE ( entry_container[ i ], xi_entry_ptr, i );
    }

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_write_entry_16_bit                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Write to table entry                                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function writes the passed entry (the masked part), to the passed   */
/*   entry pointer and and returns the status of the operation.               */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_entry - entry to be written                                         */
/*    *xi_entry_ptr - pointer to the entry in the hash table to where the     */
/*       entry should be written                                              */
/*    xi_mask - mask for the entry                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    none.                                                                   */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_write_entry_16_bit ( uint8_t                     *xi_entry,
                                                RDD_16_BIT_TABLE_ENTRY_DTS  *xi_entry_ptr,
                                                uint32_t                    xi_mask )
{
    uint8_t   entry_container[ 2 ];
    uint32_t  i;

    /* Read the contents of the entry */
    for ( i = 0; i < 2; i++ )
    {
        RDD_HASH_TABLE_ENTRY_READ ( entry_container[ i ], xi_entry_ptr, i );
    }

    for ( i = 0; i < 2; i++ )
    {
        entry_container[ 1 - i ] &= ( ( ~xi_mask >> ( i * 8 ) ) & 0xFF );
        xi_entry[ 1 - i ] &= ( ( xi_mask >> ( i * 8 ) ) & 0xFF );
    }

    for ( i = 0; i < 2; i++ )
    {
        entry_container[ i ] |= xi_entry[ i ];
        RDD_HASH_TABLE_ENTRY_WRITE ( entry_container[ i ], xi_entry_ptr, i );
    }

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_write_entry_8_bit                                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Write to table entry                                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function writes the passed entry (the masked part), to the passed   */
/*   entry pointer and and returns the status of the operation.               */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_entry - entry to be written                                         */
/*    *xi_entry_ptr - pointer to the entry in the hash table to where the     */
/*       entry should be written                                              */
/*    xi_mask - mask for the entry                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    none.                                                                   */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_write_entry_8_bit ( uint8_t                    *xi_entry,
                                               RDD_8_BIT_TABLE_ENTRY_DTS  *xi_entry_ptr,
                                               uint32_t                   xi_mask )
{
    uint8_t  entry_container;

    /* Read the contents of the entry */
    RDD_HASH_TABLE_ENTRY_READ ( entry_container, xi_entry_ptr, 0 );

    entry_container &= ( ~xi_mask & 0xFF );
    *xi_entry &= ( xi_mask & 0xFF );

    entry_container |= *xi_entry;
    RDD_HASH_TABLE_ENTRY_WRITE ( entry_container, xi_entry_ptr, 0 );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_write_external_context                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Write external context                                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function writes external context                                    */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_context_entry_ptr - context entry to be written                     */
/*    *xi_context_ptr - pointer to the first entry in the hash table          */
/*       where the entry should be written                                    */
/*    xi_entry_index - index in the table to where the entry should be        */
/*       written                                                              */
/*    xi_context_size - context entry size                                    */
/*    xi_mask_high - mask for the high part of the entry                      */
/*    xi_mask_low - mask for the low part of the entry                        */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    none.                                                                   */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_write_external_context ( uint8_t                              *xi_context_entry_ptr,
                                                    uint8_t                              *xi_context_ptr,
                                                    BL_LILAC_RDD_CONTEXT_ENTRY_SIZE_DTS  xi_context_size,
                                                    uint32_t                             xi_entry_index,
                                                    uint32_t                             xi_mask_high,
                                                    uint32_t                             xi_mask_low )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error = BL_LILAC_RDD_OK;

    switch ( xi_context_size )
    {
    case BL_LILAC_RDD_CONTEXT_8_BIT:

        rdd_error = rdd_write_entry_8_bit ( xi_context_entry_ptr, ( ( RDD_8_BIT_TABLE_ENTRY_DTS * )xi_context_ptr ) + xi_entry_index, xi_mask_high & 0xFF );
        break;

    case BL_LILAC_RDD_CONTEXT_16_BIT:

        rdd_error = rdd_write_entry_16_bit ( xi_context_entry_ptr, ( ( RDD_16_BIT_TABLE_ENTRY_DTS * )xi_context_ptr ) + xi_entry_index, xi_mask_high & 0xFFFF );
        break;

    case BL_LILAC_RDD_CONTEXT_32_BIT:

        rdd_error = rdd_write_entry_32_bit ( xi_context_entry_ptr, ( ( RDD_32_BIT_TABLE_ENTRY_DTS * )xi_context_ptr ) + xi_entry_index, xi_mask_high );
        break;

    case BL_LILAC_RDD_CONTEXT_64_BIT:

        rdd_error = rdd_write_entry_64_bit ( xi_context_entry_ptr, ( ( RDD_64_BIT_TABLE_ENTRY_DTS * )xi_context_ptr ) + xi_entry_index, xi_mask_high, xi_mask_low );
        break;
    }

    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE rdd_write_control_bits ( RDD_64_BIT_TABLE_ENTRY_DTS              *xi_table_ptr,
                                                uint32_t                                xi_table_size,
                                                uint32_t                                xi_entry_index,
                                                BL_LILAC_RDD_HASH_TABLE_WRITE_TYPE_DTS  xi_write_type )
{
    RDD_64_BIT_TABLE_ENTRY_DTS  *entry_ptr;
    uint32_t                    entry_index;
    uint32_t                    entry_valid, entry_skip;

    entry_ptr = xi_table_ptr + xi_entry_index;

    switch ( xi_write_type )
    {
    case BL_LILAC_RDD_ADD_ENTRY:
        RDD_HASH_TABLE_ENTRY_AGING_WRITE ( LILAC_RDD_FALSE, entry_ptr );
        RDD_HASH_TABLE_ENTRY_SKIP_WRITE  ( LILAC_RDD_FALSE, entry_ptr );
        RDD_HASH_TABLE_ENTRY_VALID_WRITE ( LILAC_RDD_TRUE, entry_ptr );
        break;

    case BL_LILAC_RDD_MODIFY_ENTRY:
        break;

    case BL_LILAC_RDD_REMOVE_ENTRY:
        /* set the entry as a skipped if a match was found */
        RDD_HASH_TABLE_ENTRY_SKIP_WRITE ( LILAC_RDD_TRUE, entry_ptr );

        /* hash table optimization - remove skipped entries */
        entry_index = ( xi_entry_index + 1 ) % xi_table_size;
        entry_ptr = xi_table_ptr + entry_index;

        /* if the next entry is not a valid entry, then all the skipped */
        /* entries above it can be cleared.                             */
        RDD_HASH_TABLE_ENTRY_VALID_READ ( entry_valid, entry_ptr );

        if ( !entry_valid )
        {
            while ( LILAC_RDD_TRUE )
            {
                /* climb up in the hash table */
                entry_index = ( entry_index - 1 ) % xi_table_size;

                entry_ptr = xi_table_ptr + entry_index;

                /* clear skipped entry - reset valid and skip bits */
                RDD_HASH_TABLE_ENTRY_SKIP_READ ( entry_skip, entry_ptr );

                if ( entry_skip )
                {
                    RDD_HASH_TABLE_ENTRY_VALID_WRITE ( LILAC_RDD_FALSE, entry_ptr );
                    RDD_HASH_TABLE_ENTRY_SKIP_WRITE ( LILAC_RDD_FALSE, entry_ptr );
                }
                else
                {
                    /* no more skipped entries stop the clearing process */
                    return ( BL_LILAC_RDD_OK );
                }
            }
        }

        break;
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_add_hash_entry_64_bit ( RDD_64_BIT_TABLE_CFG  *xi_table_cfg,
                                                   uint8_t               *xi_hash_entry_ptr,
                                                   uint8_t               *xi_context_entry_ptr,
                                                   uint32_t              xi_key_mask_high,
                                                   uint32_t              xi_key_mask_low,
                                                   uint32_t              xi_crc_init_value,
                                                   uint32_t              *xo_entry_index )
{
    RDD_64_BIT_TABLE_ENTRY_DTS  *table_ptr;
    uint32_t                    entry_index;
    uint8_t                     shifted_entry[ 8 ];
    uint8_t                     *context_ptr;
    uint32_t                    table_size;
    uint8_t                     is_cam_flag;
    BL_LILAC_RDD_ERROR_DTE      rdd_error;

    table_ptr = xi_table_cfg->hash_table_ptr;
    context_ptr = xi_table_cfg->context_table_ptr;
    table_size = xi_table_cfg->hash_table_size;

    is_cam_flag = 0;

    /* shift entry to write it later to the hash table  */
    rdd_left_shift_entry_64_bit ( xi_hash_entry_ptr, 4, shifted_entry );

    /* mask the original entry */
    rdd_mask_entry_32_bit ( &xi_hash_entry_ptr[ 0 ], xi_key_mask_high, &xi_hash_entry_ptr[ 0 ] );
    rdd_mask_entry_32_bit ( &xi_hash_entry_ptr[ 4 ], xi_key_mask_low, &xi_hash_entry_ptr[ 4 ] );

    /* find empty entry in table */
    rdd_error = rdd_find_empty_hash_entry_64_bit ( xi_table_cfg, xi_hash_entry_ptr, xi_crc_init_value, &entry_index );

    if ( rdd_error != BL_LILAC_RDD_OK && xi_table_cfg->is_extension_cam )
    {
        rdd_error = rdd_find_empty_cam_entry_64_bit ( xi_table_cfg, &entry_index );

        table_ptr = xi_table_cfg->cam_table_ptr;
        context_ptr = xi_table_cfg->cam_context_table_ptr;
        table_size = xi_table_cfg->cam_table_size;

        is_cam_flag = 1;
    }

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    /* write entry and context */
    rdd_error = rdd_write_entry_64_bit ( shifted_entry, table_ptr + entry_index, 0xFFFFFFFF, 0xFFFFFFFF );

    if ( xi_table_cfg->is_external_context )
    {
        rdd_write_external_context ( xi_context_entry_ptr, context_ptr, xi_table_cfg->context_size, entry_index, 0xFFFFFFFF, 0xFFFFFFFF );
    }

    rdd_error = rdd_write_control_bits ( table_ptr, table_size, entry_index, BL_LILAC_RDD_ADD_ENTRY );

    *xo_entry_index = entry_index + is_cam_flag * xi_table_cfg->hash_table_size;

    return ( BL_LILAC_RDD_OK );
}

#ifdef UNDEF
BL_LILAC_RDD_ERROR_DTE f_lilac_rdd_add_hash_entry_128_bit ( BL_LILAC_RDD_64_BIT_TABLE_CFG_DTE  *xi_table_cfg_ptr,
                                                            uint8_t                            *xi_hash_entry_ptr,
                                                            uint8_t                            *xi_context_entry_ptr,
                                                            uint32_t                           *xo_entry_index )
{
    RDD_128_BIT_TABLE_ENTRY_DTS  *table_ptr;
    uint8_t                      *context_ptr;
    uint32_t                     table_size;
    uint8_t                      masked_entry[16];
    uint32_t                     entry_index;
    BL_LILAC_RDD_ERROR_DTE       rdd_error;

    /* extract table params */
    table_ptr   = xi_table_cfg_ptr->hash_table_ptr;
    context_ptr = xi_table_cfg_ptr->context_table_ptr;
    table_size  = xi_table_cfg_ptr->hash_table_size;

    /* mask the original entry */
    rdd_mask_entry_128_bit ( xi_hash_entry_ptr, xi_table_cfg_ptr->global_mask, masked_entry );

    /* find empty entry in table */
    rdd_error = f_lilac_rdd_find_empty_hash_entry_128_bit ( xi_table_cfg_ptr, masked_entry, &entry_index );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    /* write entry and context */
    f_lilac_rdd_write_entry_128_bit ( xi_hash_entry_ptr, table_ptr + entry_index );

    if ( xi_table_cfg_ptr->is_external_context )
    {
        f_lilac_rdd_write_external_context ( xi_context_entry_ptr, context_ptr, xi_table_cfg_ptr->context_size, entry_index, 0xFFFFFFFF, 0xFFFFFFFF );
    }
    
    /* set the valid bit */
    LILAC_RDD_DDR_HASH_TABLE_ENTRY_VALID_WRITE ( TRUE, table_ptr + entry_index );

    /* increment the valid entries counter */
    f_ddr_hash_table_increment_counter( xi_table_cfg_ptr );
    *xo_entry_index = entry_index;

    return ( BL_LILAC_RDD_OK );
}
#endif

BL_LILAC_RDD_ERROR_DTE rdd_modify_hash_entry_64_bit ( RDD_64_BIT_TABLE_CFG  *xi_table_cfg,
                                                      uint8_t               *xi_hash_entry_ptr,
                                                      uint8_t               *xi_context_entry_ptr,
                                                      uint32_t              xi_key_mask_high,
                                                      uint32_t              xi_key_mask_low,
                                                      uint32_t              xi_internal_context_mask_high,
                                                      uint32_t              xi_internal_context_mask_low,
                                                      uint32_t              xi_crc_init_value,
                                                      uint32_t              *xo_entry_index )
{
    RDD_64_BIT_TABLE_ENTRY_DTS  *table_ptr;
    uint32_t                    entry_index;
    uint8_t                     shifted_entry[ 8 ];
    uint8_t                     *context_ptr;
    uint32_t                    is_cam_flag;
    BL_LILAC_RDD_ERROR_DTE      rdd_error;

    table_ptr = xi_table_cfg->hash_table_ptr;
    context_ptr = xi_table_cfg->context_table_ptr;
    is_cam_flag = 0;

    /* shift entry to write it later to the hash table  */
    rdd_left_shift_entry_64_bit ( xi_hash_entry_ptr, 4, shifted_entry );

    /* mask the original entry */
    rdd_mask_entry_32_bit ( &xi_hash_entry_ptr[ 0 ], xi_key_mask_high, &xi_hash_entry_ptr[ 0 ] );
    rdd_mask_entry_32_bit ( &xi_hash_entry_ptr[ 4 ], xi_key_mask_low, &xi_hash_entry_ptr[ 4 ] );

    rdd_error = rdd_find_hash_entry_64_bit ( xi_table_cfg, xi_hash_entry_ptr, xi_key_mask_high, xi_key_mask_low, xi_crc_init_value, &entry_index );

    if ( rdd_error != BL_LILAC_RDD_OK && xi_table_cfg->is_extension_cam )
    {
        rdd_error = rdd_find_cam_entry_64_bit ( xi_table_cfg, xi_hash_entry_ptr, xi_key_mask_high, xi_key_mask_low, &entry_index );

        table_ptr = xi_table_cfg->cam_table_ptr;
        context_ptr = xi_table_cfg->cam_context_table_ptr;
        is_cam_flag = 1;
    }

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    /* write entry ( and if exists, internal context ) */
    rdd_error = rdd_write_entry_64_bit ( shifted_entry, table_ptr + entry_index, xi_internal_context_mask_high, xi_internal_context_mask_low );

    /* write external context */
    if ( xi_table_cfg->is_external_context )
    {
        rdd_write_external_context ( xi_context_entry_ptr, context_ptr, xi_table_cfg->context_size, entry_index, 0xFFFFFFFF, 0xFFFFFFFF );
    }

    *xo_entry_index = entry_index + is_cam_flag * xi_table_cfg->hash_table_size;

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_remove_hash_entry_64_bit ( RDD_64_BIT_TABLE_CFG                       *xi_table_cfg,
                                                      uint8_t                                    *xi_hash_entry_ptr,
                                                      uint32_t                                   xi_key_mask_high,
                                                      uint32_t                                   xi_key_mask_low,
                                                      uint32_t                                   xi_crc_init_value,
                                                      BL_LILAC_RDD_CAM_OPTIMIZATION_CONTROL_DTS  xi_cam_optimization_control,
                                                      uint32_t                                   *xo_entry_index )
{
    RDD_64_BIT_TABLE_ENTRY_DTS  *table_ptr;
    uint32_t                    entry_index;
    uint32_t                    last_entry_index;
    uint8_t                     empty_entry[ 8 ];
    uint8_t                     forward_entry[ 8 ];
    uint8_t                     hash_entry[ 8 ];
    uint32_t                    i;
    uint8_t                     *context_ptr;
    uint32_t                    table_size;
    uint32_t                    is_entry_in_cam_table;
    BL_LILAC_RDD_ERROR_DTE      rdd_error;

    table_ptr = xi_table_cfg->hash_table_ptr;
    context_ptr = xi_table_cfg->context_table_ptr;
    table_size = xi_table_cfg->hash_table_size;
    is_entry_in_cam_table = LILAC_RDD_FALSE;

    rdd_error = rdd_find_hash_entry_64_bit ( xi_table_cfg, xi_hash_entry_ptr, xi_key_mask_high, xi_key_mask_low, xi_crc_init_value, &entry_index );

    if ( rdd_error != BL_LILAC_RDD_OK && xi_table_cfg->is_extension_cam )
    {
        rdd_error = rdd_find_cam_entry_64_bit ( xi_table_cfg, xi_hash_entry_ptr, xi_key_mask_high, xi_key_mask_low, &entry_index );

        table_ptr = xi_table_cfg->cam_table_ptr;
        context_ptr = xi_table_cfg->cam_context_table_ptr;
        table_size = xi_table_cfg->cam_table_size;
        is_entry_in_cam_table = LILAC_RDD_TRUE;
    }

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    /* Optimization for CAM extension table */
    if ( is_entry_in_cam_table && xi_cam_optimization_control )
    {
        rdd_error = rdd_find_empty_cam_entry_64_bit ( xi_table_cfg, &last_entry_index );

        if ( rdd_error == BL_LILAC_RDD_ERROR_HASH_TABLE_NO_EMPTY_ENTRY )
        {
            last_entry_index = xi_table_cfg->cam_table_size - 1;
        }
        else
        {
            last_entry_index--;
        }

        if ( entry_index != last_entry_index )
        {
            RDD_HASH_TABLE_ENTRY_SKIP_WRITE ( LILAC_RDD_TRUE, ( RDD_64_BIT_TABLE_ENTRY_DTS * )( table_ptr + entry_index ) );

            if ( xi_table_cfg->is_external_context )
            {
                for ( i = 0; i < xi_table_cfg->context_size; i++ )
                {
                    RDD_HASH_TABLE_ENTRY_READ ( forward_entry[ i ], context_ptr + last_entry_index * xi_table_cfg->context_size, i );
                    RDD_HASH_TABLE_ENTRY_WRITE ( forward_entry[ i ], context_ptr + entry_index * xi_table_cfg->context_size, i );
                }
            }

            /* write the last entry in the CAM instead the deleted entry */
            for ( i = 0; i < 8; i++ )
            {
                RDD_HASH_TABLE_ENTRY_READ ( hash_entry[ i ], ( RDD_64_BIT_TABLE_ENTRY_DTS * )( table_ptr + last_entry_index ), i );
                RDD_HASH_TABLE_ENTRY_WRITE ( hash_entry[ i ], ( RDD_64_BIT_TABLE_ENTRY_DTS * )( table_ptr + entry_index ), i );
            }

            rdd_error = rdd_write_control_bits ( table_ptr, table_size, entry_index, BL_LILAC_RDD_ADD_ENTRY );

            /* set the last entry in the CAM as invalid */
            RDD_HASH_TABLE_ENTRY_VALID_WRITE ( LILAC_RDD_FALSE, ( RDD_64_BIT_TABLE_ENTRY_DTS * )( table_ptr + last_entry_index ) );

            /* continue deleting the last entry in CAM */
            entry_index = last_entry_index;
        }
        else
        {
            RDD_HASH_TABLE_ENTRY_VALID_WRITE ( LILAC_RDD_FALSE, ( RDD_64_BIT_TABLE_ENTRY_DTS * )( table_ptr + entry_index ) );
        }
    }
    else
    {
        rdd_error = rdd_write_control_bits ( table_ptr, table_size, entry_index, BL_LILAC_RDD_REMOVE_ENTRY );
    }

    memset ( ( void * )empty_entry, 0, 8 );

    rdd_error = rdd_write_entry_64_bit ( empty_entry, table_ptr + entry_index, 0xFFFFFFFF, 0xFFFFFFF0 );

    if ( xi_table_cfg->is_external_context )
    {
        rdd_write_external_context ( empty_entry, context_ptr, xi_table_cfg->context_size, entry_index, 0xFFFFFFFF, 0xFFFFFFFF );
    }

    *xo_entry_index = entry_index + is_entry_in_cam_table * xi_table_cfg->hash_table_size;

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_left_shift_entry_64_bit                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   left shift value                                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function shifts a 64 bit value to the left                          */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_value - pointer to the array to be shifted                          */
/*    xi_key_offset - number of bits to shift                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    xo_new_value - shifted array                                            */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_left_shift_entry_64_bit ( uint8_t  *xi_value,
                                                     uint8_t  xi_offset,
                                                     uint8_t  *xo_new_value )
{
    uint32_t  i;

    for ( i = 0; i < 8; i++ )
    {
        xo_new_value[ i ] = xi_value[ i ] << xi_offset;

        if ( i != 7 )
        {
            xo_new_value[ i ] |= xi_value[ i + 1 ] >> ( 8 - xi_offset );
        }
    }

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_mask_entry_32_bit                                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   mask a 32 bit entry                                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function masks the passed 4 byte array with the passed 4 byte mask  */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_value - pointer to the array to be masked                           */
/*    xi_mask - mask                                                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    xo_new_value - masked array                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_mask_entry_32_bit ( uint8_t   *xi_value,
                                               uint32_t  xi_mask,
                                               uint8_t   *xo_new_value )
{
    uint32_t  i;

    for ( i = 0; i < 4; i++ )
    {
        xo_new_value[ i ] = xi_value[ i ] & ( ( uint8_t )( xi_mask >> ( 3 - i ) * 8 ) );
    }

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_find_empty_hash_entry_ddr                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Find an empty entry in hash table                                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function finds an empty entry in a generic hash table and returns   */
/*    the status of the operation.                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_hash_key - key to be searched in hash table                         */
/*    xi_table_cfg_ptr - hash table configuration data                        */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *xo_entry_index - used for returning the index that was found ( in      */
/*       case it was found )                                                  */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_find_empty_hash_entry_ddr ( RDD_DDR_TABLE_CFG  *xi_table_cfg_ptr,
                                                       uint8_t            *xi_hash_key,
                                                       uint32_t           *xo_entry_index )
{
    RDD_DDR_TABLE_ENTRY_DTS  *entry_ptr;
    uint32_t                 tries;
    uint32_t                 hash_index;
    uint32_t                 entry_index;
    uint32_t                 entry_valid;

    hash_index = ddr_hash_table_get_bucket_index ( xi_table_cfg_ptr, xi_hash_key );
    hash_index = hash_index * xi_table_cfg_ptr->hash_table_bucket_size;

    for ( tries = 0; tries < xi_table_cfg_ptr->hash_table_bucket_size; tries++ )
    {
        entry_index = hash_index + tries;

        /* read table entry */
        entry_ptr = xi_table_cfg_ptr->hash_table_ptr + entry_index;

        /* empty entry is not valid */
        RDD_DDR_HASH_TABLE_ENTRY_VALID_READ ( entry_valid, entry_ptr );

        if ( !entry_valid )
        {
            *xo_entry_index = entry_index;
            return ( BL_LILAC_RDD_OK );
        }
    }

    return ( BL_LILAC_RDD_ERROR_HASH_TABLE_NO_EMPTY_ENTRY );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_mask_ddr_entry                                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   mask the 32 msb of a 128 bit entry                                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function masks the first 4 byte from the passed array with the      */
/*   passed 32 bits uint mask                                                 */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_hash_entry_ptr - pointer to the array to be masked                  */
/*    xi_key_mask - mask                                                      */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    xo_masked_entry - masked array                                          */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
void rdd_mask_ddr_entry ( uint8_t   *xi_hash_entry_ptr,
                          uint32_t  xi_key_mask,
                          uint8_t   *xo_masked_entry )
{
    uint32_t  i;

    /* mask first 4 bytes */
    for ( i = 0; i < 4; i++ )
    {
        xo_masked_entry[ i ] = xi_hash_entry_ptr[ i ] & ( ( uint8_t )( xi_key_mask >> ( 3 - i ) * 8 ) );
    }

    /* copy the rest of the entry */
    for ( i = 4; i < 16; i++ )
    {
        xo_masked_entry[ i ] = xi_hash_entry_ptr[ i ];
    }

    return;
}


BL_LILAC_RDD_ERROR_DTE rdd_find_hash_entry_ddr ( RDD_DDR_TABLE_CFG  *xi_table_cfg_ptr,
                                                 uint8_t            *xi_hash_key,
                                                 uint32_t           *xo_entry_index )
{
    RDD_DDR_TABLE_ENTRY_DTS  *entry_ptr;
    uint8_t                  key[ 16 ];
    uint8_t                  entry[ 16 ];
    uint32_t                 hash_index;
    uint32_t                 entry_index; 
    uint32_t                 entry_valid;
    uint32_t                 tries;
    uint32_t                 i;

    rdd_mask_ddr_entry ( xi_hash_key, xi_table_cfg_ptr->global_mask, key );

    hash_index = ddr_hash_table_get_bucket_index ( xi_table_cfg_ptr, key );
    hash_index = hash_index * xi_table_cfg_ptr->hash_table_bucket_size;

    for ( tries = 0; tries < xi_table_cfg_ptr->hash_table_bucket_size; tries++ )
    {
        entry_index = hash_index + tries;

        entry_ptr = xi_table_cfg_ptr->hash_table_ptr + entry_index;

        RDD_DDR_HASH_TABLE_ENTRY_VALID_READ ( entry_valid, entry_ptr );

        if ( !entry_valid )
        {
            continue;
        }

        for ( i = 0; i < 16; i++ )
        {
            RDD_HASH_TABLE_ENTRY_READ ( entry[ i ], entry_ptr, i );
        }

        rdd_mask_ddr_entry ( entry, xi_table_cfg_ptr->global_mask, entry );

        for ( i = 0; i < 16; i++ )
        {
            if ( entry[ i ] != key[ i ] )
            {
                break;
            }
        }

        if ( i == 16 )
        {
            *xo_entry_index = entry_index;
            return ( BL_LILAC_RDD_OK );
        }
        else
        {
            continue;
        }
    }

    return ( BL_LILAC_RDD_ERROR_HASH_TABLE_NO_MATCHING_KEY );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_write_entry_ddr                                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Write to hash table entry                                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function writes the passed entry (the masked part), to the passed   */
/*   entry pointer.                                                           */
/*                                                                            */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *xi_entry - entry to be written                                         */
/*    *xi_entry_ptr - pointer to the entry in the hash table to where the     */
/*       entry should be written                                              */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    none.                                                                   */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
void rdd_write_entry_ddr ( uint8_t                  *xi_entry,
                           RDD_DDR_TABLE_ENTRY_DTS  *xi_entry_ptr )
{
    uint32_t  i;

    for ( i = 0; i < 16; i++ )
    {
        RDD_HASH_TABLE_ENTRY_WRITE ( xi_entry[ i ], xi_entry_ptr, i );
    }

    return;
}


BL_LILAC_RDD_ERROR_DTE rdd_add_hash_entry_ddr ( RDD_DDR_TABLE_CFG  *xi_table_cfg_ptr,
                                                uint8_t            *xi_hash_entry_ptr,
                                                uint8_t            *xi_context_entry_ptr,
                                                uint32_t           xi_cache_flag,
                                                uint32_t           *xo_entry_index )
{
    RDD_DDR_TABLE_ENTRY_DTS  *table_ptr;
    uint8_t                  *context_ptr;
    uint8_t                  masked_entry[ 16 ];
    uint32_t                 entry_index;
    uint32_t                 context_entry_index;
    uint32_t                 context_entry_address;
    BL_LILAC_RDD_ERROR_DTE   rdd_error;

    table_ptr = xi_table_cfg_ptr->hash_table_ptr;
    context_ptr = xi_table_cfg_ptr->context_table_ptr;

    rdd_mask_ddr_entry ( xi_hash_entry_ptr, xi_table_cfg_ptr->global_mask, masked_entry );

    rdd_error = rdd_find_empty_hash_entry_ddr ( xi_table_cfg_ptr, masked_entry, &entry_index );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    if ( xi_table_cfg_ptr->is_external_context )
    {
        /* allocate a new context entry */
        context_entry_index = xi_table_cfg_ptr->context_entries_free_list_head;
        xi_table_cfg_ptr->context_entries_free_list_head = *( xi_table_cfg_ptr->context_entries_free_list + xi_table_cfg_ptr->context_entries_free_list_head );

        RDD_DDR_HASH_TABLE_CONTEXT_ENTRY_VALID_WRITE ( LILAC_RDD_TRUE , xi_context_entry_ptr );

        rdd_write_external_context ( xi_context_entry_ptr, context_ptr, xi_table_cfg_ptr->context_size, context_entry_index, 0xFFFFFFFF, 0xFFFFFFFF );

        context_entry_address = xi_table_cfg_ptr->context_table_offset + context_entry_index * xi_table_cfg_ptr->context_size;

        /* write the context entry address */
        RDD_DDR_HASH_TABLE_ENTRY_CONTEXT_PTR_WRITE ( context_entry_address , xi_hash_entry_ptr );
    }

    if ( !( xi_cache_flag ) )
    {
        xi_table_cfg_ptr->non_cached_entries_counter++;
        MWRITE_8( xi_table_cfg_ptr->search_ddr_flag_address, 1 );
    }
    else
    {
        RDD_DDR_HASH_TABLE_ENTRY_CACHE_FLAG_WRITE ( LILAC_RDD_TRUE, xi_hash_entry_ptr );
    }

    RDD_DDR_HASH_TABLE_ENTRY_VALID_WRITE ( LILAC_RDD_TRUE , xi_hash_entry_ptr );

    rdd_write_entry_ddr ( xi_hash_entry_ptr, table_ptr + entry_index );

    *xo_entry_index = entry_index;

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_remove_hash_entry_ddr ( RDD_DDR_TABLE_CFG  *xi_table_cfg_ptr,
                                                   uint8_t            *xi_hash_entry_ptr,
                                                   uint32_t           *xo_entry_index )
{
    RDD_DDR_TABLE_ENTRY_DTS *hash_entry_ptr;
    uint32_t                entry_index;
    uint32_t                context_entry_address;
    uint32_t                context_entry_index;
    uint32_t                cache_flag;
    uint8_t                 empty_entry[ 16 ];
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    rdd_error = rdd_find_hash_entry_ddr ( xi_table_cfg_ptr, xi_hash_entry_ptr, &entry_index );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    memset ( empty_entry, 0, 16 );

    hash_entry_ptr = xi_table_cfg_ptr->hash_table_ptr + entry_index ;

    if ( xi_table_cfg_ptr->is_external_context )
    {
        RDD_DDR_HASH_TABLE_ENTRY_CONTEXT_PTR_READ ( context_entry_address , hash_entry_ptr );

        context_entry_index = ( context_entry_address - xi_table_cfg_ptr->context_table_offset ) / xi_table_cfg_ptr->context_size;

        rdd_write_external_context ( empty_entry, xi_table_cfg_ptr->context_table_ptr, xi_table_cfg_ptr->context_size, context_entry_index, 0xFFFFFFFF, 0xFFFFFFFF );

        * ( xi_table_cfg_ptr->context_entries_free_list + xi_table_cfg_ptr->context_entries_free_list_tail ) = context_entry_index;

        * ( xi_table_cfg_ptr->context_entries_free_list + context_entry_index ) = xi_table_cfg_ptr->context_table_size;

        xi_table_cfg_ptr->context_entries_free_list_tail = context_entry_index;

        if ( xi_table_cfg_ptr->context_entries_free_list_head == xi_table_cfg_ptr->context_table_size )
        {
            xi_table_cfg_ptr->context_entries_free_list_head = context_entry_index;
        }
    }

    RDD_DDR_HASH_TABLE_ENTRY_CACHE_FLAG_READ ( cache_flag, hash_entry_ptr );

    if ( ! ( cache_flag ) )
    {
        xi_table_cfg_ptr->non_cached_entries_counter--;

        if ( xi_table_cfg_ptr->non_cached_entries_counter == 0 )
        {
            MWRITE_8( xi_table_cfg_ptr->search_ddr_flag_address, 0 );
        }
    }

    rdd_write_entry_ddr ( empty_entry, hash_entry_ptr );

    *xo_entry_index = entry_index;

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   ddr_hash_table_get_bucket_index                                          */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   calculate bucket index accoring to 128 bit entry                         */
/*                                                                            */
/* Input:                                                                     */
/*   xi_hash_table_cfg_ptr - the hash table configuration struct              */
/*   xi_hash_key - the pointer to the 128 bit entry                           */
/*                                                                            */
/* Output:                                                                    */
/*   the bucket index                                                         */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
uint32_t ddr_hash_table_get_bucket_index ( RDD_DDR_TABLE_CFG  *xi_table_cfg_ptr,
                                             uint8_t          *xi_hash_key )
{
    uint32_t  crc_value;
    uint32_t  bucket_index;

    crc_value = crcbitbybit ( &xi_hash_key[ 0 ], 16, 0, 0xffffffff, RDD_CRC_TYPE_32 );
    bucket_index = crc_value % xi_table_cfg_ptr->hash_table_size;

    return bucket_index;
}

