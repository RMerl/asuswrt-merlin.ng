/*
<:copyright-BRCM:2014-2016:DUAL/GPL:standard

   Copyright (c) 2014-2016 Broadcom 
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
#include "rdd_lookup_engine.h"

extern uint8_t* g_runner_ddr_base_addr;
extern rdd_64_bit_table_cfg_t  g_hash_table_cfg[RDD_MAX_HASH_TABLE];


int rdd_find_empty_hash_entry_64_bit(rdd_64_bit_table_cfg_t *, uint8_t *, uint32_t, uint32_t *);
int rdd_find_empty_cam_entry_64_bit(rdd_64_bit_table_cfg_t  *, uint32_t *);
int rdd_find_cam_entry_64_bit(rdd_64_bit_table_cfg_t *, uint8_t *, uint32_t, uint32_t, uint32_t *);
void rdd_write_entry_64_bit(uint8_t *, rdd_64_bit_table_entry_t *, uint32_t, uint32_t);
void rdd_write_entry_32_bit(uint8_t *, rdd_32_bit_table_entry_t *, uint32_t);
void rdd_write_entry_16_bit(uint8_t *, rdd_16_bit_table_entry_t *, uint32_t);
void rdd_write_entry_8_bit(uint8_t *, rdd_8_bit_table_entry_t *, uint32_t);
void rdd_write_external_context(uint8_t *, uint8_t *, rdd_context_entry_size_t, uint32_t, uint32_t, uint32_t);
int rdd_left_shift_entry_64_bit(uint8_t *, uint8_t, uint8_t *);
int rdd_mask_entry_32_bit(uint8_t *, uint32_t, uint8_t *);
int rdd_find_empty_hash_entry_ddr(rdd_ddr_table_cfg_t *, uint8_t *, uint32_t *);
void rdd_write_entry_ddr(uint8_t *, rdd_ddr_table_entry_t *);
void rdd_mask_entry_ddr(uint8_t *, uint32_t, uint8_t *);
uint32_t ddr_hash_table_get_bucket_index(rdd_ddr_table_cfg_t *, uint8_t *);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_find_empty_hash_entry_64_bit                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function finds an empty entry in a generic hash table and returns   */
/*    the status of the operation.                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *hash_table_cfg - hash table configuration data                         */
/*    *hash_key - key to be searched in hash table                            */
/*    crc_init_value - initial value to be used in hashing the key            */
/*    mask_high - mask for the high part of the entry                         */
/*    mask_low - mask for the low part of the entry                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *entry_index - used for returning the index that was found (in          */
/*       case it was found)                                                   */
/*                                      .                                     */
/******************************************************************************/
int rdd_find_empty_hash_entry_64_bit(rdd_64_bit_table_cfg_t *hash_table_cfg, uint8_t *hash_key, uint32_t crc_init_value,
    uint32_t *entry_index_output)
{
    rdd_64_bit_table_entry_t *entry_ptr;
    uint32_t tries, hash_index, entry_index, entry_valid, entry_skip;

    if (crc_init_value == 0)
    {
        crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_16);
    }

    crc_init_value = rdd_crc_bit_by_bit(&hash_key[0], 1, 4, crc_init_value, RDD_CRC_TYPE_16);

    /* calculate the CRC on the key */
    hash_index = rdd_crc_bit_by_bit(&hash_key[2], 6, 0, crc_init_value, RDD_CRC_TYPE_16);

    hash_index = hash_index % hash_table_cfg->hash_table_size;

    /* search for an empty line in the hash table for the new entry */
    /* limit the search to a preconfigured search depth             */
    for (tries = 0; tries < hash_table_cfg->hash_table_search_depth; tries++)
    {
        entry_index = (hash_index + tries) % hash_table_cfg->hash_table_size;

        /* read table entry */
        entry_ptr = hash_table_cfg->hash_table_ptr + entry_index;

        /* empty line is either not valid or set as skip */
        RDD_HASH_TABLE_ENTRY_VALID_READ(entry_valid, entry_ptr);
        RDD_HASH_TABLE_ENTRY_SKIP_READ(entry_skip, entry_ptr);

        if (!(entry_valid) || (entry_skip))
        {
            *entry_index_output = entry_index;
            return 0;
        }
    }

    return BDMF_ERR_NO_MORE;
}


int rdd_find_hash_entry_64_bit(rdd_64_bit_table_cfg_t *hash_table_cfg, uint8_t *hash_key_input, uint32_t mask_high,
    uint32_t mask_low, uint32_t crc_init_value, uint32_t *entry_index_output)
{
    rdd_64_bit_table_entry_t  *entry_ptr;
    uint8_t entry[8], key_container[8], hash_key[8];
    uint32_t tries, hash_index, entry_index, entry_valid, entry_skip, i;        

    /* Calculate the CRC on the key */
    if (crc_init_value == 0)
    {
        crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_16);
    }

    /* Mask the key container */
    rdd_mask_entry_32_bit(&hash_key_input[0], mask_high, &hash_key[0]);
    rdd_mask_entry_32_bit(&hash_key_input[4], mask_low, &hash_key[4]);

    crc_init_value = rdd_crc_bit_by_bit(&hash_key[0], 1, 4, crc_init_value, RDD_CRC_TYPE_16);

    hash_index = rdd_crc_bit_by_bit(&hash_key[2], 6, 0, crc_init_value, RDD_CRC_TYPE_16);

    hash_index = hash_index % hash_table_cfg->hash_table_size;

    /* Prepare key for comparison */
    rdd_left_shift_entry_64_bit(hash_key, 4, key_container);

    /* Search for an empty line in the hash table for the new entry */
    /* Limit the search to a preconfigured search depth             */
    for (tries = 0; tries < hash_table_cfg->hash_table_search_depth; tries++)
    {
        entry_index = (hash_index + tries) % hash_table_cfg->hash_table_size;

        /* Read table entry */
        entry_ptr = hash_table_cfg->hash_table_ptr + entry_index;

        /* Skip over reusable entries */
        RDD_HASH_TABLE_ENTRY_SKIP_READ(entry_skip, entry_ptr);

        if (entry_skip)
        {
            continue;
        }

        /* Valid but not skipped entries should be searched */
        RDD_HASH_TABLE_ENTRY_VALID_READ(entry_valid, entry_ptr);

        if (entry_valid)
        {
            /* Read 64 bit entry */
            for (i = 0; i < 8; i++)
            {
                RDD_HASH_TABLE_ENTRY_READ(entry[i], entry_ptr, i);
            }

            /* Mask the entry */
            rdd_mask_entry_32_bit(&entry[0], (mask_high << 4) | (mask_low >> 28), &entry[0]);
            rdd_mask_entry_32_bit(&entry[4], mask_low << 4, &entry[4]);

            /* Compare the entry to the key container */
            if ((entry[0] == key_container[0]) &&
                (entry[1] == key_container[1]) &&
                (entry[2] == key_container[2]) &&
                (entry[3] == key_container[3]) &&
                (entry[4] == key_container[4]) &&
                (entry[5] == key_container[5]) &&
                (entry[6] == key_container[6]) &&
                (entry[7] == key_container[7]))
            {
                *entry_index_output = entry_index;
                return 0;
            }
        }
        else
        {
            break;
        }
    }

    return BDMF_ERR_NOENT;
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_find_empty_cam_entry_64_bit                                          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function finds an empty entry in a generic cam table and returns    */
/*    the status of the operation.                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *table_cfg - cam table configuration data                               */
/*    *hash_key - key to be searched in cam table                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *entry_index - used for returning the index that was found (in          */
/*       case it was found)                                                   */
/*                                                                            */
/******************************************************************************/
int rdd_find_empty_cam_entry_64_bit(rdd_64_bit_table_cfg_t *table_cfg, uint32_t *entry_index_output)
{
   rdd_64_bit_table_entry_t  *entry_ptr;
   uint32_t entry_index, entry_valid, entry_skip;

    for (entry_index = 0; entry_index < table_cfg->cam_table_size; entry_index++)
    {
        /* read CAM table entry */
        entry_ptr = table_cfg->cam_table_ptr + entry_index;

        /* empty line is either not valid or set as skip */
        RDD_HASH_TABLE_ENTRY_VALID_READ(entry_valid, entry_ptr);
        RDD_HASH_TABLE_ENTRY_SKIP_READ(entry_skip, entry_ptr);

        if (!entry_valid || entry_skip)
        {
            /* return the index of the entry in the table */
            *entry_index_output = entry_index;
            return 0;
        }
    }

    return BDMF_ERR_NO_MORE;
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_find_cam_entry_64_bit                                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function finds a key in a generic cam table and returns the         */
/*   status of the operation.                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *table_cfg - table configuration data                                   */
/*    *cam_key - key to be searched in cam table                              */
/*    mask_high - mask for the high part of the entry                         */
/*    mask_low - mask for the low part of the entry                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *entry_index - used for returning the index that was found (in          */
/*       case it was found)                                                   */
/*                                      .                                     */
/******************************************************************************/
int rdd_find_cam_entry_64_bit(rdd_64_bit_table_cfg_t *table_cfg, uint8_t *cam_key, uint32_t mask_high, 
    uint32_t mask_low, uint32_t *entry_index_output)
{
    rdd_64_bit_table_entry_t  *entry_ptr;
    uint8_t  entry[8], key_container[8];
    uint32_t entry_index, entry_valid, entry_skip, i;

    /* Prepare key for comparison */
    rdd_left_shift_entry_64_bit(cam_key, 4, key_container);

    /* Mask the key container */
    rdd_mask_entry_32_bit(&key_container[0], (mask_high << 4) | (mask_low >> 28), &key_container[0]);
    rdd_mask_entry_32_bit(&key_container[4], mask_low << 4, &key_container[4]);

    /* Search for an empty line in the hash table for the new entry */
    /* Limit the search to a preconfigured search depth             */
    for (entry_index = 0; entry_index < table_cfg->cam_table_size; entry_index++)
    {
        /* Read table entry */
        entry_ptr = table_cfg->cam_table_ptr + entry_index;

        /* Valid but not skipped entries should be searched */
        RDD_HASH_TABLE_ENTRY_VALID_READ(entry_valid, entry_ptr);
        RDD_HASH_TABLE_ENTRY_SKIP_READ(entry_skip, entry_ptr);

        if (entry_valid)
        {
            /* Read 64 bit entry */
            for (i = 0; i < 8; i++)
            {
                RDD_HASH_TABLE_ENTRY_READ(entry[i], entry_ptr, i);
            }

            /* Mask the entry */
            rdd_mask_entry_32_bit(&entry[0], (mask_high << 4) | (mask_low >> 28), &entry[0]);
            rdd_mask_entry_32_bit(&entry[4], mask_low << 4, &entry[4]);

            /* Compare the entry to the key container */
            if ((entry[0] == key_container[0]) &&
                (entry[1] == key_container[1]) &&
                (entry[2] == key_container[2]) &&
                (entry[3] == key_container[3]) &&
                (entry[4] == key_container[4]) &&
                (entry[5] == key_container[5]) &&
                (entry[6] == key_container[6]) &&
                (entry[7] == key_container[7]))
            {
                *entry_index_output = entry_index;
                return 0;
            }
        }
        else if (entry_skip)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return BDMF_ERR_NOENT;
}


int rdd_find_entry_64_bit(rdd_64_bit_table_cfg_t  *table_cfg, uint8_t *key, uint32_t mask_high, uint32_t mask_low,
    uint32_t crc_init_value, uint32_t *entry_index)
{
    int  rdd_error;

    rdd_error = rdd_find_hash_entry_64_bit(table_cfg,
                                             key,
                                             mask_high,
                                             mask_low,
                                             crc_init_value,
                                             entry_index);
    if (rdd_error == 0)
    {
        return 0;
    }

    /* find the requested entry in the CAM */
    rdd_error = rdd_find_cam_entry_64_bit(table_cfg,
                                            key,
                                            mask_high,
                                            mask_low,
                                            entry_index);

    *entry_index += table_cfg->hash_table_size;

    return rdd_error;
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_write_entry_64_bit                                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function writes the passed entry (the masked part), to the passed   */
/*   entry pointer and and returns the status of the operation.               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *entry - entry to be written                                            */
/*    *entry_ptr - pointer to the entry in the hash table to where the        */
/*       entry should be written                                              */
/*    mask_hi - mask for the high part of the entry                           */
/*    mask_lo - mask for the low part of the entry                            */
/*                                                                            */
/******************************************************************************/
void rdd_write_entry_64_bit(uint8_t *entry, rdd_64_bit_table_entry_t *entry_ptr, uint32_t mask_high, uint32_t mask_low)
{
    uint8_t  entry_container[8];
    uint32_t i;

    /* Read the contents of the entry */
    for (i = 0; i < 8; i++)
    {
        RDD_HASH_TABLE_ENTRY_READ(entry_container[i], entry_ptr, i);
    }

    rdd_mask_entry_32_bit(&entry_container[0], ~mask_high, &entry_container[0]);
    rdd_mask_entry_32_bit(&entry[0], mask_high, &entry[0]);

    rdd_mask_entry_32_bit(&entry_container[4], ~mask_low, &entry_container[4]);
    rdd_mask_entry_32_bit(&entry[4], mask_low, &entry[4]);

    for (i = 0; i < 8; i++)
    {
        entry_container[i] |= entry[i];
        RDD_HASH_TABLE_ENTRY_WRITE(entry_container[i], entry_ptr, i);
    }
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_write_entry_32_bit                                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function writes the passed entry (the masked part), to the passed   */
/*   entry pointer and and returns the status of the operation.               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *entry - entry to be written                                            */
/*    *entry_ptr - pointer to the entry in the hash table to where the        */
/*       entry should be written                                              */
/*    mask - mask for the entry                                               */
/*                                                                            */
/******************************************************************************/
void rdd_write_entry_32_bit(uint8_t *entry, rdd_32_bit_table_entry_t *entry_ptr, uint32_t mask)
{
    uint8_t   entry_container[4];
    uint32_t  i;

    /* Read the contents of the entry */
    for (i = 0; i < 4; i++)
    {
        RDD_HASH_TABLE_ENTRY_READ(entry_container[i], entry_ptr, i);
    }

    rdd_mask_entry_32_bit(entry_container, ~mask, entry_container);
    rdd_mask_entry_32_bit(entry, mask, entry);

    for (i = 0; i < 4; i++)
    {
        entry_container[i] |= entry[i];
        RDD_HASH_TABLE_ENTRY_WRITE(entry_container[i], entry_ptr, i);
    }
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_write_entry_16_bit                                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function writes the passed entry (the masked part), to the passed   */
/*   entry pointer and and returns the status of the operation.               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *entry - entry to be written                                            */
/*    *entry_ptr - pointer to the entry in the hash table to where the        */
/*       entry should be written                                              */
/*    mask - mask for the entry                                               */
/*                                                                            */
/******************************************************************************/
void rdd_write_entry_16_bit(uint8_t *entry, rdd_16_bit_table_entry_t *entry_ptr, uint32_t mask)
{
    uint8_t   entry_container[2];
    uint32_t  i;

    /* Read the contents of the entry */
    for (i = 0; i < 2; i++)
    {
        RDD_HASH_TABLE_ENTRY_READ(entry_container[i], entry_ptr, i);
    }

    for (i = 0; i < 2; i++)
    {
        entry_container[1 - i] &= ((~mask >> (i * 8)) & 0xFF);
        entry[1 - i] &= ((mask >> (i * 8)) & 0xFF);
    }

    for (i = 0; i < 2; i++)
    {
        entry_container[i] |= entry[i];
        RDD_HASH_TABLE_ENTRY_WRITE(entry_container[i], entry_ptr, i);
    }
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_write_entry_8_bit                                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function writes the passed entry (the masked part), to the passed   */
/*   entry pointer and and returns the status of the operation.               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *entry - entry to be written                                            */
/*    *entry_ptr - pointer to the entry in the hash table to where the        */
/*       entry should be written                                              */
/*    mask - mask for the entry                                               */
/*                                                                            */
/******************************************************************************/
void rdd_write_entry_8_bit(uint8_t *entry, rdd_8_bit_table_entry_t *entry_ptr, uint32_t mask)
{
    uint8_t  entry_container;

    /* Read the contents of the entry */
    RDD_HASH_TABLE_ENTRY_READ(entry_container, entry_ptr, 0);

    entry_container &= (~mask & 0xFF);
    *entry &= (mask & 0xFF);

    entry_container |= *entry;
    RDD_HASH_TABLE_ENTRY_WRITE(entry_container, entry_ptr, 0);
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_write_external_context                                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function writes external context                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *context_entry_ptr - context entry to be written                        */
/*    *context_ptr - pointer to the first entry in the hash table             */
/*       where the entry should be written                                    */
/*    entry_index - index in the table to where the entry should be           */
/*       written                                                              */
/*    context_size - context entry size                                       */
/*    mask_high - mask for the high part of the entry                         */
/*    mask_low - mask for the low part of the entry                           */
/*                                                                            */
/******************************************************************************/
void rdd_write_external_context(uint8_t *context_entry_ptr, uint8_t *context_ptr, rdd_context_entry_size_t context_size,
    uint32_t entry_index, uint32_t mask_high, uint32_t mask_low)
{
    switch (context_size)
    {
    case RDD_CONTEXT_8_BIT:

        rdd_write_entry_8_bit(context_entry_ptr, ((rdd_8_bit_table_entry_t *)context_ptr) + entry_index, mask_high & 0xFF);
        break;

    case RDD_CONTEXT_16_BIT:

        rdd_write_entry_16_bit(context_entry_ptr, ((rdd_16_bit_table_entry_t *)context_ptr) + entry_index, mask_high & 0xFFFF);
        break;

    case RDD_CONTEXT_32_BIT:

        rdd_write_entry_32_bit(context_entry_ptr, ((rdd_32_bit_table_entry_t *)context_ptr) + entry_index, mask_high);
        break;

    case RDD_CONTEXT_64_BIT:

        rdd_write_entry_64_bit(context_entry_ptr, ((rdd_64_bit_table_entry_t *)context_ptr) + entry_index, mask_high, mask_low);
        break;
    }
}


int rdd_write_control_bits(rdd_64_bit_table_entry_t *table_ptr, uint32_t table_size, uint32_t entry_index_input,
    rdd_hash_table_write_type_t  write_type)
{
    rdd_64_bit_table_entry_t  *entry_ptr;
    uint32_t entry_index, entry_valid, entry_skip;

    entry_ptr = table_ptr + entry_index_input;

    switch (write_type)
    {
    case RDD_ADD_ENTRY:
        RDD_HASH_TABLE_ENTRY_AGING_WRITE(0, entry_ptr);
        RDD_HASH_TABLE_ENTRY_SKIP_WRITE(0, entry_ptr);
        RDD_HASH_TABLE_ENTRY_VALID_WRITE(1, entry_ptr);
        break;

    case RDD_MODIFY_ENTRY:
        break;

    case RDD_REMOVE_ENTRY:
        /* set the entry as a skipped if a match was found */
        RDD_HASH_TABLE_ENTRY_SKIP_WRITE(1, entry_ptr);

        /* hash table optimization - remove skipped entries */
        entry_index = (entry_index_input + 1) % table_size;
        entry_ptr = table_ptr + entry_index;

        /* if the next entry is not a valid entry, then all the skipped */
        /* entries above it can be cleared.                             */
        RDD_HASH_TABLE_ENTRY_VALID_READ(entry_valid, entry_ptr);

        if (!entry_valid)
        {
            while (1)
            {
                /* climb up in the hash table */
                entry_index = (entry_index - 1) % table_size;

                entry_ptr = table_ptr + entry_index;

                /* clear skipped entry - reset valid and skip bits */
                RDD_HASH_TABLE_ENTRY_SKIP_READ(entry_skip, entry_ptr);

                if (entry_skip)
                {
                    RDD_HASH_TABLE_ENTRY_VALID_WRITE(0, entry_ptr);
                    RDD_HASH_TABLE_ENTRY_SKIP_WRITE(0, entry_ptr);
                }
                else
                {
                    /* no more skipped entries stop the clearing process */
                    return 0;
                }
            }
        }

        break;
    }

    return 0;
}


int rdd_add_hash_entry_64_bit(rdd_64_bit_table_cfg_t  *table_cfg, uint8_t *hash_entry_ptr, uint8_t *context_entry_ptr,
    uint32_t key_mask_high, uint32_t key_mask_low, uint32_t crc_init_value, uint32_t *entry_index_output)
{
    rdd_64_bit_table_entry_t  *table_ptr;
    uint32_t entry_index, table_size;
    uint8_t  shifted_entry[8], *context_ptr, is_cam_flag;
    int      rdd_error;

    table_ptr = table_cfg->hash_table_ptr;
    context_ptr = table_cfg->context_table_ptr;
    table_size = table_cfg->hash_table_size;

    is_cam_flag = 0;

    /* shift entry to write it later to the hash table  */
    rdd_left_shift_entry_64_bit(hash_entry_ptr, 4, shifted_entry);

    /* mask the original entry */
    rdd_mask_entry_32_bit(&hash_entry_ptr[0], key_mask_high, &hash_entry_ptr[0]);
    rdd_mask_entry_32_bit(&hash_entry_ptr[4], key_mask_low, &hash_entry_ptr[4]);

    /* find empty entry in table */
    rdd_error = rdd_find_empty_hash_entry_64_bit(table_cfg, hash_entry_ptr, crc_init_value, &entry_index);

    if (rdd_error != 0 && table_cfg->is_extension_cam)
    {
        rdd_error = rdd_find_empty_cam_entry_64_bit(table_cfg, &entry_index);

        table_ptr = table_cfg->cam_table_ptr;
        context_ptr = table_cfg->cam_context_table_ptr;
        table_size = table_cfg->cam_table_size;

        is_cam_flag = 1;
    }

    if (rdd_error != 0)
    {
        return rdd_error;
    }

    /* write entry and context */
    rdd_write_entry_64_bit(shifted_entry, table_ptr + entry_index, 0xFFFFFFFF, 0xFFFFFFFF);

    if (table_cfg->is_external_context)
    {
        rdd_write_external_context(context_entry_ptr, context_ptr, table_cfg->context_size, entry_index, 0xFFFFFFFF, 0xFFFFFFFF);
    }

    rdd_error = rdd_write_control_bits(table_ptr, table_size, entry_index, RDD_ADD_ENTRY);

    *entry_index_output = entry_index + is_cam_flag * table_cfg->hash_table_size;

    return 0;
}

#ifdef UNDEF
int f_lilac_rdd_add_hash_entry_128_bit(BL_LILAC_rdd_64_bit_table_cfg_t_DTE  *table_cfg_ptr, uint8_t *hash_entry_ptr,
    uint8_t *context_entry_ptr, uint32_t *entry_index)
{
    RDD_128_BIT_TABLE_ENTRY_DTS  *table_ptr;
    uint8_t  *context_ptr, masked_entry[16];
    uint32_t table_size, entry_index;
    int      rdd_error;

    /* extract table params */
    table_ptr   = table_cfg_ptr->hash_table_ptr;
    context_ptr = table_cfg_ptr->context_table_ptr;
    table_size  = table_cfg_ptr->hash_table_size;

    /* mask the original entry */
    rdd_mask_entry_128_bit(hash_entry_ptr, table_cfg_ptr->global_mask, masked_entry);

    /* find empty entry in table */
    rdd_error = rdd_find_empty_hash_entry_128_bit(table_cfg_ptr, masked_entry, &entry_index);

    if (rdd_error != 0)
    {
        return rdd_error;
    }

    /* write entry and context */
    f_lilac_rdd_write_entry_128_bit(hash_entry_ptr, table_ptr + entry_index);

    if (table_cfg_ptr->is_external_context)
    {
        rdd_write_external_context(context_entry_ptr, context_ptr, table_cfg_ptr->context_size, entry_index, 0xFFFFFFFF, 0xFFFFFFFF);
    }
    
    /* set the valid bit */
    LILAC_RDD_DDR_HASH_TABLE_ENTRY_VALID_WRITE(TRUE, table_ptr + entry_index);

    /* increment the valid entries counter */
    f_ddr_hash_table_increment_counter(table_cfg_ptr);
    *entry_index = entry_index;

    return 0;
}
#endif

int rdd_modify_hash_entry_64_bit(rdd_64_bit_table_cfg_t *table_cfg, uint8_t *hash_entry_ptr, uint8_t *context_entry_ptr,
    uint32_t key_mask_high, uint32_t key_mask_low, uint32_t internal_context_mask_high, 
    uint32_t internal_context_mask_low, uint32_t crc_init_value, uint32_t *entry_index_output)
{
    rdd_64_bit_table_entry_t  *table_ptr;
    uint32_t entry_index, is_cam_flag;
    uint8_t  shifted_entry[8], *context_ptr;
    int      rdd_error;

    table_ptr = table_cfg->hash_table_ptr;
    context_ptr = table_cfg->context_table_ptr;
    is_cam_flag = 0;

    /* shift entry to write it later to the hash table  */
    rdd_left_shift_entry_64_bit(hash_entry_ptr, 4, shifted_entry);

    /* mask the original entry */
    rdd_mask_entry_32_bit(&hash_entry_ptr[0], key_mask_high, &hash_entry_ptr[0]);
    rdd_mask_entry_32_bit(&hash_entry_ptr[4], key_mask_low, &hash_entry_ptr[4]);

    rdd_error = rdd_find_hash_entry_64_bit(table_cfg, hash_entry_ptr, key_mask_high, key_mask_low, crc_init_value, &entry_index);

    if (rdd_error != 0 && table_cfg->is_extension_cam)
    {
        rdd_error = rdd_find_cam_entry_64_bit(table_cfg, hash_entry_ptr, key_mask_high, key_mask_low, &entry_index);

        table_ptr = table_cfg->cam_table_ptr;
        context_ptr = table_cfg->cam_context_table_ptr;
        is_cam_flag = 1;
    }

    if (rdd_error != 0)
    {
        return rdd_error;
    }

    /* write entry (and if exists, internal context) */
    rdd_write_entry_64_bit(shifted_entry, table_ptr + entry_index, internal_context_mask_high, internal_context_mask_low);

    /* write external context */
    if (table_cfg->is_external_context)
    {
        rdd_write_external_context(context_entry_ptr, context_ptr, table_cfg->context_size, entry_index, 0xFFFFFFFF, 0xFFFFFFFF);
    }

    *entry_index_output = entry_index + is_cam_flag * table_cfg->hash_table_size;

    return 0;
}


int rdd_remove_hash_entry_64_bit(rdd_64_bit_table_cfg_t *table_cfg, uint8_t *hash_entry_ptr, uint32_t key_mask_high,
    uint32_t key_mask_low, uint32_t crc_init_value, rdd_cam_optimization_control_t  cam_optimization_control,
    uint32_t *entry_index_output)
{
    rdd_64_bit_table_entry_t  *table_ptr;
    uint32_t entry_index, last_entry_index, i, table_size, is_entry_in_cam_table;
    uint8_t  empty_entry[8], forward_entry[8], hash_entry[8], *context_ptr;
    int      rdd_error;

    table_ptr = table_cfg->hash_table_ptr;
    context_ptr = table_cfg->context_table_ptr;
    table_size = table_cfg->hash_table_size;
    is_entry_in_cam_table = 0;

    rdd_error = rdd_find_hash_entry_64_bit(table_cfg, hash_entry_ptr, key_mask_high, key_mask_low, crc_init_value, &entry_index);

    if (rdd_error != 0 && table_cfg->is_extension_cam)
    {
        rdd_error = rdd_find_cam_entry_64_bit(table_cfg, hash_entry_ptr, key_mask_high, key_mask_low, &entry_index);

        table_ptr = table_cfg->cam_table_ptr;
        context_ptr = table_cfg->cam_context_table_ptr;
        table_size = table_cfg->cam_table_size;
        is_entry_in_cam_table = 1;
    }

    if (rdd_error != 0)
    {
        return rdd_error;
    }

    /* Optimization for CAM extension table */
    if (is_entry_in_cam_table && cam_optimization_control)
    {
        rdd_error = rdd_find_empty_cam_entry_64_bit(table_cfg, &last_entry_index);

        if (rdd_error == BDMF_ERR_NO_MORE)
        {
            last_entry_index = table_cfg->cam_table_size - 1;
        }
        else
        {
            last_entry_index--;
        }

        if (entry_index != last_entry_index)
        {
            RDD_HASH_TABLE_ENTRY_SKIP_WRITE(1, (rdd_64_bit_table_entry_t *)(table_ptr + entry_index));

            if (table_cfg->is_external_context)
            {
                for (i = 0; i < table_cfg->context_size; i++)
                {
                    RDD_HASH_TABLE_ENTRY_READ(forward_entry[i], context_ptr + last_entry_index * table_cfg->context_size, i);
                    RDD_HASH_TABLE_ENTRY_WRITE(forward_entry[i], context_ptr + entry_index * table_cfg->context_size, i);
                }
            }

            /* write the last entry in the CAM instead the deleted entry */
            for (i = 0; i < 8; i++)
            {
                RDD_HASH_TABLE_ENTRY_READ(hash_entry[i], (rdd_64_bit_table_entry_t *)(table_ptr + last_entry_index), i);
                RDD_HASH_TABLE_ENTRY_WRITE(hash_entry[i], (rdd_64_bit_table_entry_t *)(table_ptr + entry_index), i);
            }

            rdd_error = rdd_write_control_bits(table_ptr, table_size, entry_index, RDD_ADD_ENTRY);

            /* set the last entry in the CAM as invalid */
            RDD_HASH_TABLE_ENTRY_VALID_WRITE(0, (rdd_64_bit_table_entry_t *)(table_ptr + last_entry_index));

            /* continue deleting the last entry in CAM */
            entry_index = last_entry_index;
        }
        else
        {
            RDD_HASH_TABLE_ENTRY_VALID_WRITE(0, (rdd_64_bit_table_entry_t *)(table_ptr + entry_index));
        }
    }
    else
    {
        rdd_error = rdd_write_control_bits(table_ptr, table_size, entry_index, RDD_REMOVE_ENTRY);
    }

    memset((void *)empty_entry, 0, 8);

    rdd_write_entry_64_bit(empty_entry, table_ptr + entry_index, 0xFFFFFFFF, 0xFFFFFFF0);

    if (table_cfg->is_external_context)
    {
        rdd_write_external_context(empty_entry, context_ptr, table_cfg->context_size, entry_index, 0xFFFFFFFF, 0xFFFFFFFF);
    }

    *entry_index_output = entry_index + is_entry_in_cam_table * table_cfg->hash_table_size;

    return 0;
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_left_shift_entry_64_bit                                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function shifts a 64 bit value to the left                          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *value - pointer to the array to be shifted                             */
/*    key_offset - number of bits to shift                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    new_value - shifted array                                               */
/*                                      .                                     */
/******************************************************************************/
int rdd_left_shift_entry_64_bit(uint8_t *value, uint8_t offset, uint8_t *new_value)
{
    uint32_t  i;

    for (i = 0; i < 8; i++)
    {
        new_value[i] = value[i] << offset;

        if (i != 7)
        {
            new_value[i] |= value[i + 1] >> (8 - offset);
        }
    }

    return 0;
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_mask_entry_32_bit                                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function masks the passed 4 byte array with the passed 4 byte mask  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *value - pointer to the array to be masked                              */
/*    mask - mask                                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    new_value - masked array                                                */
/*                                                                            */
/******************************************************************************/
int rdd_mask_entry_32_bit(uint8_t *value, uint32_t mask, uint8_t *new_value)
{
    uint32_t  i;

    for (i = 0; i < 4; i++)
    {
        new_value[i] = value[i] & ((uint8_t)(mask >> (3 - i) * 8));
    }

    return 0;
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_find_empty_hash_entry_ddr                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function finds an empty entry in a generic hash table and returns   */
/*    the status of the operation.                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *hash_key - key to be searched in hash table                            */
/*    table_cfg_ptr - hash table configuration data                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    *entry_index - used for returning the index that was found (in          */
/*       case it was found)                                                   */
/*                                                                            */
/******************************************************************************/
int rdd_find_empty_hash_entry_ddr(rdd_ddr_table_cfg_t *table_cfg_ptr, uint8_t *hash_key, uint32_t *entry_index_output)
{
    rdd_ddr_table_entry_t *entry_ptr;
    uint32_t  tries, hash_index, entry_index, entry_valid;

    hash_index = ddr_hash_table_get_bucket_index(table_cfg_ptr, hash_key);
    hash_index = hash_index * table_cfg_ptr->hash_table_bucket_size;

    for (tries = 0; tries < table_cfg_ptr->hash_table_bucket_size; tries++)
    {
        entry_index = hash_index + tries;

        /* read table entry */
        entry_ptr = table_cfg_ptr->hash_table_ptr + entry_index;

        /* empty entry is not valid */
        RDD_DDR_HASH_TABLE_ENTRY_VALID_READ(entry_valid, entry_ptr);

        if (!entry_valid)
        {
            *entry_index_output = entry_index;
            return 0;
        }
    }

    return BDMF_ERR_NO_MORE;
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_mask_ddr_entry                                                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   mask the 32 msb of a 128 bit entry                                       */
/*   This function masks the first 4 byte from the passed array with the      */
/*   passed 32 bits uint mask                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *hash_entry_ptr - pointer to the array to be masked                     */
/*    key_mask - mask                                                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*    masked_entry - masked array                                             */
/*                                                                            */
/******************************************************************************/
void rdd_mask_ddr_entry(uint8_t *hash_entry_ptr, uint32_t key_mask, uint8_t *masked_entry)
{
    uint32_t  i;

    /* mask first 4 bytes */
    for (i = 0; i < 4; i++)
    {
        masked_entry[i] = hash_entry_ptr[i] & ((uint8_t)(key_mask >> (3 - i) * 8));
    }

    /* copy the rest of the entry */
    for (i = 4; i < 16; i++)
    {
        masked_entry[i] = hash_entry_ptr[i];
    }
}


int rdd_find_hash_entry_ddr(rdd_ddr_table_cfg_t *table_cfg_ptr, uint8_t *hash_key, uint32_t *entry_index_output)
{
    rdd_ddr_table_entry_t *entry_ptr;
    uint8_t  key[16], entry[16];
    uint32_t hash_index, entry_index, entry_valid, tries, i;

    rdd_mask_ddr_entry(hash_key, table_cfg_ptr->global_mask, key);

    hash_index = ddr_hash_table_get_bucket_index(table_cfg_ptr, key);
    hash_index = hash_index * table_cfg_ptr->hash_table_bucket_size;

    for (tries = 0; tries < table_cfg_ptr->hash_table_bucket_size; tries++)
    {
        entry_index = hash_index + tries;

        entry_ptr = table_cfg_ptr->hash_table_ptr + entry_index;

        RDD_DDR_HASH_TABLE_ENTRY_VALID_READ(entry_valid, entry_ptr);

        if (!entry_valid)
        {
            continue;
        }

        for (i = 0; i < 16; i++)
        {
            RDD_HASH_TABLE_ENTRY_READ(entry[i], entry_ptr, i);
        }

        rdd_mask_ddr_entry(entry, table_cfg_ptr->global_mask, entry);

        for (i = 0; i < 16; i++)
        {
            if (entry[i] != key[i])
            {
                break;
            }
        }

        if (i == 16)
        {
            *entry_index_output = entry_index;
            return 0;
        }
        else
        {
            continue;
        }
    }

    return BDMF_ERR_NOENT;
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_write_entry_ddr                                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function writes the passed entry (the masked part), to the passed   */
/*   entry pointer.                                                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    *entry - entry to be written                                            */
/*    *entry_ptr - pointer to the entry in the hash table to where the        */
/*       entry should be written                                              */
/*                                                                            */
/******************************************************************************/
void rdd_write_entry_ddr(uint8_t *entry, rdd_ddr_table_entry_t *entry_ptr)
{
    uint32_t  i;

    for (i = 0; i < 16; i++)
    {
        RDD_HASH_TABLE_ENTRY_WRITE(entry[i], entry_ptr, i);
    }
}


int rdd_add_hash_entry_ddr(rdd_ddr_table_cfg_t *table_cfg_ptr, uint8_t *hash_entry_ptr, uint8_t *context_entry_ptr,
    uint32_t cache_flag, uint32_t *entry_index_output)
{
    rdd_ddr_table_entry_t *table_ptr;
    uint8_t  *context_ptr, masked_entry[16];
    uint32_t entry_index, context_entry_index, context_entry_address;
    int      rdd_error;

    table_ptr = table_cfg_ptr->hash_table_ptr;
    context_ptr = table_cfg_ptr->context_table_ptr;

    rdd_mask_ddr_entry(hash_entry_ptr, table_cfg_ptr->global_mask, masked_entry);

    rdd_error = rdd_find_empty_hash_entry_ddr(table_cfg_ptr, masked_entry, &entry_index);

    if (rdd_error != 0)
    {
        return rdd_error;
    }

    if (table_cfg_ptr->is_external_context)
    {
        /* allocate a new context entry */
        context_entry_index = table_cfg_ptr->context_entries_free_list_head;
        table_cfg_ptr->context_entries_free_list_head = *(table_cfg_ptr->context_entries_free_list + table_cfg_ptr->context_entries_free_list_head);

        RDD_DDR_HASH_TABLE_CONTEXT_ENTRY_VALID_WRITE(1 , context_entry_ptr);

        rdd_write_external_context(context_entry_ptr, context_ptr, table_cfg_ptr->context_size, context_entry_index, 0xFFFFFFFF, 0xFFFFFFFF);

        context_entry_address = table_cfg_ptr->context_table_offset + context_entry_index * table_cfg_ptr->context_size;

        /* write the context entry address */
        RDD_DDR_HASH_TABLE_ENTRY_CONTEXT_PTR_WRITE(context_entry_address , hash_entry_ptr);
    }

    if (!(cache_flag))
    {
        table_cfg_ptr->non_cached_entries_counter++;
        MWRITE_8(table_cfg_ptr->search_ddr_flag_address, 1);
    }
    else
    {
        RDD_DDR_HASH_TABLE_ENTRY_CACHE_FLAG_WRITE(1, hash_entry_ptr);
    }

    RDD_DDR_HASH_TABLE_ENTRY_VALID_WRITE(1 , hash_entry_ptr);

    rdd_write_entry_ddr(hash_entry_ptr, table_ptr + entry_index);

    *entry_index_output = entry_index;

    return 0;
}


int rdd_remove_hash_entry_ddr(rdd_ddr_table_cfg_t *table_cfg_ptr, uint8_t *hash_entry_ptr, uint32_t *entry_index_output)
{
    rdd_ddr_table_entry_t *hash_entry;
    uint32_t  entry_index, context_entry_address, context_entry_index, cache_flag;
    uint8_t   empty_entry[16];
    int       rdd_error;

    rdd_error = rdd_find_hash_entry_ddr(table_cfg_ptr, hash_entry_ptr, &entry_index);

    if (rdd_error != 0)
    {
        return rdd_error;
    }

    memset(empty_entry, 0, 16);

    hash_entry = table_cfg_ptr->hash_table_ptr + entry_index;

    if (table_cfg_ptr->is_external_context)
    {
        RDD_DDR_HASH_TABLE_ENTRY_CONTEXT_PTR_READ(context_entry_address, hash_entry);

        context_entry_index = (context_entry_address - table_cfg_ptr->context_table_offset) / table_cfg_ptr->context_size;

        rdd_write_external_context(empty_entry, table_cfg_ptr->context_table_ptr, table_cfg_ptr->context_size, context_entry_index, 0xFFFFFFFF, 0xFFFFFFFF);

        *(table_cfg_ptr->context_entries_free_list + table_cfg_ptr->context_entries_free_list_tail) = context_entry_index;

        *(table_cfg_ptr->context_entries_free_list + context_entry_index) = table_cfg_ptr->context_table_size;

        table_cfg_ptr->context_entries_free_list_tail = context_entry_index;

        if (table_cfg_ptr->context_entries_free_list_head == table_cfg_ptr->context_table_size)
        {
            table_cfg_ptr->context_entries_free_list_head = context_entry_index;
        }
    }

    RDD_DDR_HASH_TABLE_ENTRY_CACHE_FLAG_READ(cache_flag, hash_entry);

    if (!(cache_flag))
    {
        table_cfg_ptr->non_cached_entries_counter--;

        if (table_cfg_ptr->non_cached_entries_counter == 0)
        {
            MWRITE_8(table_cfg_ptr->search_ddr_flag_address, 0);
        }
    }

    rdd_write_entry_ddr(empty_entry, hash_entry);

    *entry_index_output = entry_index;

    return 0;
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   ddr_hash_table_get_bucket_index                                          */
/*                                                                            */
/* Input:                                                                     */
/*   calculate bucket index accoring to 128 bit entry                         */
/*   hash_table_cfg_ptr - the hash table configuration struct                 */
/*   hash_key - the pointer to the 128 bit entry                              */
/*                                                                            */
/* Output:                                                                    */
/*   the bucket index                                                         */
/*                                                                            */
/******************************************************************************/
uint32_t ddr_hash_table_get_bucket_index(rdd_ddr_table_cfg_t *table_cfg_ptr, uint8_t *hash_key)
{
    uint32_t  crc_value;
    uint32_t  bucket_index;

    crc_value = rdd_crc_bit_by_bit(&hash_key[0], 16, 0, 0xffffffff, RDD_CRC_TYPE_32);
    bucket_index = crc_value % table_cfg_ptr->hash_table_size;

    return bucket_index;
}

