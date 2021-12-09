/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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
#include "rdp_common.h"
#include "rdp_drv_tcam.h"

/* Key size */
static rdp_tcam_key_type tcam_key_type;
static uint32_t tcam_key_size; /* Key size in bytes */

/* Per-priority entry count */
static uint32_t tcam_priority_count[RDP_TCAM_MAX_PRIORITIES];

/* Max number of TCAM entries */
static uint32_t tcam_max_entries;

/* Total number of TCAM rows used */
static uint32_t tcam_entries;

/* Key or mask address type */
typedef enum
{
    RDP_TCAM_ADDRESS_KEY0       = 0,
    RDP_TCAM_ADDRESS_KEY1       = 1,
} rdp_tcam_key_index;

/*
 * Internal helpers
 */
static void _drv_tcam_wait_operation_done(void);
static void _drv_tcam_shift_down(uint32_t insert_index);
static void _drv_tcam_shift_up(uint32_t delete_index);
static void _drv_tcam_invalidate(uint32_t entry);
static void _drv_tcam_read(uint32_t entry, rdp_tcam_key_index key_index, bdmf_boolean *is_valid, uint8_t key[]);
static void _drv_tcam_write(uint32_t entry, rdp_tcam_key_index key_index, uint8_t key[]);
static void _drv_tcam_key_mask_to_key0_key1(const rdp_tcam_key_area_t *key, const rdp_tcam_key_area_t *mask,
    rdp_tcam_key_area_t *key0, rdp_tcam_key_area_t *key1);
static void _drv_tcam_context_read(uint32_t entry, rdp_tcam_context_t *context);
static void _drv_tcam_context_write(uint32_t entry, const rdp_tcam_context_t *context);
static bdmf_error_t _drv_tcam_rule_find_by_key_mask(const rdp_tcam_key_area_t *key,
    const rdp_tcam_key_area_t *mask, uint16_t *rule_index);
static uint32_t _drv_tcam_u8_to_u32(const uint8_t b[]);
static void _drv_tcam_u32_to_u8(uint32_t w, uint8_t b[]);

#ifdef RDP_SIM
static void tcam_sim_execute(void);
#endif

/* Words in TCAM key area are counted from right to left.
 * This driver counts words from left to right.
 * Account for this here
 */
static inline bdmf_error_t _drv_tcam_key_in_set(uint8_t word_idx, uint32_t value)
{
    return ag_drv_tcam_key_in_set(7 - word_idx, value);
}

static inline bdmf_error_t _drv_tcam_key_out_get(uint8_t word_idx, uint32_t *p_value)
{
    return ag_drv_tcam_key_out_get(7 - word_idx, p_value);
}

/* Set TCAM mode */
bdmf_error_t drv_tcam_mode_set(rdp_tcam_key_type key_type)
{
    /* Can change key size only when TCAM table is empty */
    if (tcam_entries)
        return BDMF_ERR_ALREADY;

    if (key_type == RDP_TCAM_KEY_256)
    {
        tcam_max_entries = RDP_TCAM_ALL_TABLE_SIZE;
        tcam_key_size = 32;
    }
    else
    {
        tcam_max_entries = RDP_TCAM_TABLE_SIZE * (RDP_TCAM_NUM_ENGINES / 2);
        tcam_key_size = 64;
    }
    tcam_key_type = key_type;

    return BDMF_ERR_OK;
}

/*
 * Get tcam mode
 */
bdmf_error_t drv_tcam_mode_get(rdp_tcam_key_type *key_type)
{
    if (!key_type)
        return BDMF_ERR_PARM;
    if (!tcam_key_size)
        return BDMF_ERR_STATE;
    *key_type = tcam_key_type;
    return BDMF_ERR_OK;
}

/*
 * Add classification rule
 */
bdmf_error_t drv_tcam_rule_add(
    rdp_tcam_priority                       priority,
    const rdp_tcam_key_area_t              *key,
    const rdp_tcam_key_area_t              *mask,
    const rdp_tcam_context_t               *context)
{
    rdp_tcam_key_area_t key0, key1;
    uint32_t insert_index = 0;
    int i;

    if (!tcam_key_size)
        return BDMF_ERR_STATE;          /* Not initialized */
    if (tcam_entries >= tcam_max_entries)
        return BDMF_ERR_OVERFLOW;       /* Table is full */
    if (priority >= RDP_TCAM_MAX_PRIORITIES)
        return BDMF_ERR_PARM;

    /* higher priority should configured in the lower entry indexes*/
    priority = (RDP_TCAM_MAX_PRIORITIES - 1) - priority;

    /* Calculate number of flows with the same or higher priority */
    for (i = 0; i <= priority; i++)
    {
        insert_index += tcam_priority_count[i];
    }

    /* Shift TCAM table from insert_index down, free insert_index slot */
    _drv_tcam_shift_down(insert_index);

    /*
     * Insert the new entry. Note that at this point TCAM entry at insert_index
     * in invalid and can be updated safely
     */

    /* Set context */
    _drv_tcam_context_write(insert_index, context);

    /* Encode key and mask the way what h/w wants it */
    _drv_tcam_key_mask_to_key0_key1(key, mask, &key0, &key1);

    /* Write key1 */
    _drv_tcam_write(insert_index, RDP_TCAM_ADDRESS_KEY1, key1.b);

    /* write key0 + valid */
    _drv_tcam_write(insert_index, RDP_TCAM_ADDRESS_KEY0, key0.b);

    ++tcam_entries;
    ++tcam_priority_count[priority];

    return BDMF_ERR_OK;
}

/*
 * Modify classification context
 */
bdmf_error_t drv_tcam_rule_modify(
    uint16_t                               rule_index,
    rdp_tcam_context_t                     *context)
{
    /* Set context */
    _drv_tcam_context_write(rule_index, context);
    return BDMF_ERR_OK;
}

/*
 * Delete classification rule
 */
bdmf_error_t drv_tcam_rule_delete(
    const rdp_tcam_key_area_t              *key,
    const rdp_tcam_key_area_t              *mask)
{
    uint16_t rule_index;
    rdp_tcam_priority priority;
    uint32_t first_entry_for_priority = 0;
    bdmf_error_t err;

    /* Is mode set ? */
    if (!tcam_key_size)
        return BDMF_ERR_STATE;

    /* Find TCAM entry */
    err =  _drv_tcam_rule_find_by_key_mask(key, mask, &rule_index);
    if (err != BDMF_ERR_OK)
        return err;

    /* Find out rule's priority from its position in the table */
    for (priority = 0; priority < RDP_TCAM_MAX_PRIORITIES; priority++)
    {
        if (first_entry_for_priority + tcam_priority_count[priority] > rule_index)
            break;
        first_entry_for_priority += tcam_priority_count[priority];
    }
    if (priority >= RDP_TCAM_MAX_PRIORITIES)
        return BDMF_ERR_INTERNAL;

    /* Remove entry and pack the table */
    _drv_tcam_shift_up(rule_index);

    /* Update use counts */
    --tcam_priority_count[priority];
    --tcam_entries;

    return BDMF_ERR_OK;
}

/*
 * Find classification rule
 */
bdmf_error_t drv_tcam_rule_find(
    const rdp_tcam_key_area_t              *key,
    uint16_t                               *rule_index,
    rdp_tcam_context_t                     *context)
{
    rdp_tcam_key_area_t key0, key1;
    bdmf_boolean is_valid;
    int tcam_key_word_size = tcam_key_size / sizeof(uint32_t);
    int entry;
    int i;

    if (!key || !rule_index || !context)
        return BDMF_ERR_PARM;

    if (!tcam_entries)
        return BDMF_ERR_NOENT;

    /* For 256 bit key use TCAM command.
     * For 512 bit key we have no other recourse, but scanning the table row by row
     */
    if (tcam_key_type == RDP_TCAM_KEY_256)
    {
        for (i = 0; i < 8; i++)
        {
            /* Convert key to words in CPU byte order. The words will be
             * swapped to BIG ENDIAN as part of register write operation
             */
            _drv_tcam_key_in_set(i, _drv_tcam_u8_to_u32(&key->b[i*4]));
        }
        ag_drv_tcam_op_set(TCAM_CMD_COMPARE);
        _drv_tcam_wait_operation_done();
        ag_drv_tcam_result_get(&is_valid, rule_index);
        if (!is_valid)
            return BDMF_ERR_NOENT;
        /* Read context */
        _drv_tcam_context_read(*rule_index, context);

        return BDMF_ERR_OK;
    }

    /*
     * 512 bit key. Read TCAM tables row by row
     */
    for (entry = 0; entry < tcam_entries; entry++)
    {
        _drv_tcam_read(entry, RDP_TCAM_ADDRESS_KEY0, &is_valid, key0.b);
        if (!is_valid)
            continue;
        _drv_tcam_read(entry, RDP_TCAM_ADDRESS_KEY1, &is_valid, key1.b);

        /* Decode key0, key1 --> key, mask
         *      mask = ~key1 | key0
         *      key = key0
         * and check match
         *      key & mask == key & mask
         */
        for (i = 0; i < tcam_key_word_size; i++)
        {
            uint32_t key_word, mask_word;
            mask_word = key1.w[i] & ~key0.w[i];
            key_word = key0.w[i];
            if ((key->w[i] & mask_word) != (key_word & mask_word))
                break;
        }
        if (i == tcam_key_word_size)
            break;
    }

    if (entry >= tcam_entries)
        return BDMF_ERR_NOENT;

    *rule_index = entry;
    _drv_tcam_context_read(entry, context);

    return BDMF_ERR_OK;
}

/*
 * Internal helpers
 */

/* Copy 4 bytes to word in CPU byte order */
static uint32_t _drv_tcam_u8_to_u32(const uint8_t b[])
{
    uint32_t w;
    w = (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
    return w;
}

/* Convert 4 byte word in CPU bute order to 4 byte array */
static void _drv_tcam_u32_to_u8(uint32_t w, uint8_t b[])
{
    b[0] = (w >> 24) & 0xff;
    b[1] = (w >> 16) & 0xff;
    b[2] = (w >> 8) & 0xff;
    b[3] = w & 0xff;
}

/* convert key + mask to key0 + key1
 * key1 = ~mask | key
 * key0 = mask & key
 */
static void _drv_tcam_key_mask_to_key0_key1(const rdp_tcam_key_area_t *key, const rdp_tcam_key_area_t *mask,
    rdp_tcam_key_area_t *key0, rdp_tcam_key_area_t *key1)
{
    int i;

    for (i = 0; i < RDP_TCAM_MAX_KEY_SIZE / sizeof(uint32_t); i++)
    {
        key1->w[i] = mask->w[i] & ~key->w[i];
        key0->w[i] = mask->w[i] & key->w[i];
    }
}

/* Wait for "operation_done */
static void _drv_tcam_wait_operation_done(void)
{
    bdmf_boolean done;
#ifdef RDP_SIM
    tcam_sim_execute();
#endif
    do
    {
        ag_drv_tcam_op_done_get(&done);
    } while (!done);
}

/* Invalidate 1 table entry */
static void _drv_tcam_invalidate1(uint32_t entry)
{
    ag_drv_tcam_address_set(RDP_TCAM_ADDRESS_KEY0, entry);
    ag_drv_tcam_valid_in_set(0);
    ag_drv_tcam_op_set(TCAM_CMD_WRITE);
    _drv_tcam_wait_operation_done();
}

/* Invalidate 256 or 512 bit entry */
static void _drv_tcam_invalidate(uint32_t entry)
{
    _drv_tcam_invalidate1(entry);
    if (tcam_key_type == RDP_TCAM_KEY_512)
        _drv_tcam_invalidate1(entry + RDP_TCAM_TABLE_SIZE);
}

/* Read 256 bit key0 or key1 */
static void _drv_tcam_read1(uint32_t entry, rdp_tcam_key_index key_index, bdmf_boolean *is_valid, uint8_t key[])
{
    int i;

    ag_drv_tcam_address_set(key_index, entry);
    ag_drv_tcam_op_set(TCAM_CMD_READ);
    _drv_tcam_wait_operation_done();
    for (i = 0; i < 8; i++)
    {
        uint32_t word;
        /* Convert key to words in CPU byte order. The words were swapped
         * BIG ENDIAN as part of register read operation
         */
        _drv_tcam_key_out_get(i, &word);
        _drv_tcam_u32_to_u8(word, &key[i*4]);
    }
    ag_drv_tcam_valid_out_get(is_valid);
}

/* Read 256 or 512 bit key0 or key1 */
static void _drv_tcam_read(uint32_t entry, rdp_tcam_key_index key_index, bdmf_boolean *is_valid, uint8_t key[])
{
    _drv_tcam_read1(entry, key_index, is_valid, key);
    if (tcam_key_type == RDP_TCAM_KEY_512)
    {
        bdmf_boolean is_valid1;
        _drv_tcam_read1(entry + RDP_TCAM_TABLE_SIZE, key_index, &is_valid1, &key[32]);
        *is_valid &= is_valid1;
    }
}

/* Write 256 bit key0 or key1 */
static void _drv_tcam_write1(uint32_t entry, rdp_tcam_key_index key_index, uint8_t key[])
{
    int i;

    /* Convert key to words in CPU byte order. The words will be swapped
     * to BIG ENDIAN as part of register write operation
     */
    for (i = 0; i < 8; i++)
    {
        _drv_tcam_key_in_set(i, _drv_tcam_u8_to_u32(&key[i*4]));
    }
    ag_drv_tcam_address_set(key_index, entry);
    if (key_index == RDP_TCAM_ADDRESS_KEY0)
        ag_drv_tcam_valid_in_set(1);
    ag_drv_tcam_op_set(TCAM_CMD_WRITE);
    _drv_tcam_wait_operation_done();
}

/* Write 256 or 512 bit key0 or key1 */
static void _drv_tcam_write(uint32_t entry, rdp_tcam_key_index key_index, uint8_t key[])
{
    _drv_tcam_write1(entry, key_index, key);
    if (tcam_key_type == RDP_TCAM_KEY_512)
    {
        _drv_tcam_write1(entry + RDP_TCAM_TABLE_SIZE, key_index, &key[32]);
    }
}

/* copy tcam table entry key or mask */
static void _drv_tcam_copy_key0_or_key1(uint32_t to, uint32_t from, rdp_tcam_key_index key_index)
{
    rdp_tcam_key_area_t key0_key1;
    bdmf_boolean is_valid;

    /* Read source entry mask */
    _drv_tcam_read(from, key_index, &is_valid, key0_key1.b);

    /* Write to destination entry */
    _drv_tcam_write(to, key_index, key0_key1.b);
}

/* read TCAM context associated with table entry */
static void _drv_tcam_context_read(uint32_t entry, rdp_tcam_context_t *context)
{
    uint32_t i, ctx_index = entry * RDP_TCAM_CONTEXT_SIZE; /* Each table entry corresponds to 2/4 context entries (64/128 bit in total) */

    if (ctx_index > TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_REG_RAM_CNT)
    {
        ag_drv_tcam_debug_bus_tcam_debug_bus_select_set(1);
        ctx_index -= (TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_REG_RAM_CNT + 1);
    }
    else
        ag_drv_tcam_debug_bus_tcam_debug_bus_select_set(0);

    for (i = 0; i < RDP_TCAM_CONTEXT_SIZE; i++)
        ag_drv_tcam_context_get(ctx_index + i, &context->word[(RDP_TCAM_CONTEXT_SIZE - 1) - i]);
}

/* write TCAM context associated with table entry */
static void _drv_tcam_context_write(uint32_t entry, const rdp_tcam_context_t *context)
{
    uint32_t i, ctx_index = entry * RDP_TCAM_CONTEXT_SIZE; /* Each table entry corresponds to 2/4 context entries (64/128 bit in total) */

    if (ctx_index > TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_REG_RAM_CNT)
    {
        ag_drv_tcam_debug_bus_tcam_debug_bus_select_set(1);
        ctx_index -= (TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_REG_RAM_CNT + 1);
    }
    else
        ag_drv_tcam_debug_bus_tcam_debug_bus_select_set(0);

    for (i = 0; i < RDP_TCAM_CONTEXT_SIZE; i++)
        ag_drv_tcam_context_set(ctx_index + i, context->word[(RDP_TCAM_CONTEXT_SIZE - 1) - i]);
}

/* copy tcam table entry safely */
static void _drv_tcam_copy_entry(uint32_t to, uint32_t from)
{
    rdp_tcam_context_t context;

    /* invalidate destination entry */
    _drv_tcam_invalidate(to);

    /* Read source context. Each table entry corresponds to 2 context entries (64 bit in total) */
    _drv_tcam_context_read(from, &context);

    /* Update destination entries's context */
    _drv_tcam_context_write(to, &context);

    /* Copy key0 */
    _drv_tcam_copy_key0_or_key1(to, from, RDP_TCAM_ADDRESS_KEY1);

    /* Copy key1 and make the entry valid */
    _drv_tcam_copy_key0_or_key1(to, from, RDP_TCAM_ADDRESS_KEY0);
}

/* shift entries down to open room for a new entry.
 * - In 512bit key mode both TCAM tables are shifted synchronously
 * - in 256bit key mode  the last entry of TCAM0 is shifted to the 1st entry of TCAM1
 * */
static void _drv_tcam_shift_down(uint32_t insert_index)
{
    int i;

    for (i = tcam_entries; i > insert_index; i--)
    {
        _drv_tcam_copy_entry(i, i - 1);
    }

    /* invalidate entry we've just freed */
    _drv_tcam_invalidate(insert_index);
}


static void _drv_tcam_shift_up(uint32_t delete_index)
{
    int i;

    for (i = delete_index + 1; i < tcam_entries; i++)
    {
        _drv_tcam_copy_entry(i - 1, i);
    }

    /* Invalidate the last entry */
    _drv_tcam_invalidate(tcam_entries - 1);
}

/* Find classification rule by key & mask */
static bdmf_error_t _drv_tcam_rule_find_by_key_mask(
    const rdp_tcam_key_area_t              *key,
    const rdp_tcam_key_area_t              *mask,
    uint16_t                               *rule_index)
{
    rdp_tcam_key_area_t key0, key1;
    rdp_tcam_key_area_t entry_key0, entry_key1;
    bdmf_boolean is_valid;
    bdmf_boolean is_match = 0;
    int tcam_key_word_size = tcam_key_size / sizeof(uint32_t);
    int entry;
    int i;

    /* Convert key and mask to what h/w needs */
    _drv_tcam_key_mask_to_key0_key1(key, mask, &key0, &key1);

    /* Go over TCAM table and look for valid entry that matches the search entry exactly */
    for (entry = 0; (entry < tcam_entries) && !is_match; entry++)
    {
        _drv_tcam_read(entry, RDP_TCAM_ADDRESS_KEY0, &is_valid, entry_key0.b);
        if (!is_valid)
            continue;
        is_match = 1;
        _drv_tcam_read(entry, RDP_TCAM_ADDRESS_KEY1, &is_valid, entry_key1.b);
        for (i = 0; (i < tcam_key_word_size) && is_match; i++)
        {
            if ((key0.w[i] != entry_key0.w[i]) || (key1.w[i] != entry_key1.w[i]))
                is_match = 0;
        }
    }

    if (!is_match)
        return BDMF_ERR_NOENT;

    *rule_index = entry - 1; /* -1 because of post-increment in the outer loop */

    return BDMF_ERR_OK;
}

bdmf_error_t drv_tcam_rule_lkup(
    const rdp_tcam_key_area_t              *key,
    const rdp_tcam_key_area_t              *mask,
    uint16_t  *rule_index)
{
    /* Find TCAM entry */
    return _drv_tcam_rule_find_by_key_mask(key, mask, rule_index);
}

#ifdef USE_BDMF_SHELL

/*
 * CLI support
 */
extern int bdmf_strhex(const char *src, uint8_t *dst, uint16_t dst_len);

/* Set/get TCAM Mode
 * BDMFMON_MAKE_PARM_ENUM( "key_size", "TCAM key size", tcam_key_size_enum_table, 0) );
 */
static int _tcam_cli_mode_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    rdp_tcam_key_type key_type = (rdp_tcam_key_type)parm[0].value.number;
    bdmf_error_t err;

    /* Set_mode request ? */
    if (n_parms > 0)
        return drv_tcam_mode_set(key_type);

    /* Get mode request */
    err = drv_tcam_mode_get(&key_type);
    if (err)
        return err;

    bdmf_session_print(session, "TCAM key size: %d bit\n", (key_type == RDP_TCAM_KEY_256) ? 256 : 512);
    return BDMF_ERR_OK;
}

/* Add TCAM rule
 * BDMFMON_MAKE_PARM_RANGE( "priority", "Rule priority", BDMFMON_PARM_NUMBER, 0, 0, RDP_TCAM_MAX_PRIORITIES-1),
 * BDMFMON_MAKE_PARM( "key", "Key (16/32 byte hex string)", BDMFMON_PARM_STRING, 0),
 * BDMFMON_MAKE_PARM( "mask", "Mask (16/32 byte hex string)", BDMFMON_PARM_STRING, 0),
 * BDMFMON_MAKE_PARM( "ctx0", "Context[0]", BDMFMON_PARM_NUMBER, 0),
 * BDMFMON_MAKE_PARM_DEFVAL( "ctx1", "Context[1]", BDMFMON_PARM_NUMBER, 0, 0) );
 */
static int _tcam_cli_add_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    rdp_tcam_priority priority = (rdp_tcam_priority)parm[0].value.number;
    const char *key_str = (const char *)parm[1].value.string;
    const char *mask_str = (const char *)parm[2].value.string;
    rdp_tcam_context_t ctx = { .word = {parm[3].value.number, parm[4].value.number } };
    rdp_tcam_key_area_t key, mask;
    rdp_tcam_key_type key_type;
    int key_size;
    int n;
    bdmf_error_t err;

    err = drv_tcam_mode_get(&key_type);
    if (err)
        return err;
    key_size = (key_type == RDP_TCAM_KEY_256) ? 256/8 : 512/8;

    n =  bdmf_strhex(key_str, key.b, sizeof(key.b));
    if (n != key_size)
    {
        bdmf_session_print(session, "Invalid key: expected %d bytes hex string\n", key_size);
        return BDMF_ERR_PARM;
    }

    n =  bdmf_strhex(mask_str, mask.b, sizeof(mask.b));
    if (n != key_size)
    {
        bdmf_session_print(session, "Invalid mask: expected %d bytes hex string\n", key_size);
        return BDMF_ERR_PARM;
    }

    err = drv_tcam_rule_add(((RDP_TCAM_MAX_PRIORITIES-1) - priority), &key, &mask, &ctx);
    return err;
}

/* Delete TCAM rule
 * BDMFMON_MAKE_PARM( "key", "Key (16/32 byte hex string)", BDMFMON_PARM_STRING, 0),
 * BDMFMON_MAKE_PARM( "mask", "Mask (16/32 byte hex string)", BDMFMON_PARM_STRING, 0),
 */
static int _tcam_cli_delete_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    const char *key_str = (const char *)parm[0].value.string;
    const char *mask_str = (const char *)parm[1].value.string;
    rdp_tcam_key_area_t key, mask;
    rdp_tcam_key_type key_type;
    int key_size;
    int n;
    bdmf_error_t err;

    err = drv_tcam_mode_get(&key_type);
    if (err)
        return err;
    key_size = (key_type == RDP_TCAM_KEY_256) ? 256/8 : 512/8;

    n =  bdmf_strhex(key_str, key.b, sizeof(key.b));
    if (n != key_size)
    {
        bdmf_session_print(session, "Invalid key: expected %d bytes hex string\n", key_size);
        return BDMF_ERR_PARM;
    }

    n =  bdmf_strhex(mask_str, mask.b, sizeof(mask.b));
    if (n != key_size)
    {
        bdmf_session_print(session, "Invalid mask: expected %d bytes hex string\n", key_size);
        return BDMF_ERR_PARM;
    }

    err = drv_tcam_rule_delete(&key, &mask);

    return err;
}

/* Find TCAM rule
 * BDMFMON_MAKE_PARM( "key", "Key (16/32 byte hex string)", BDMFMON_PARM_STRING, 0),
 */
static int _tcam_cli_find_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    const char *key_str = (const char *)parm[0].value.string;
    rdp_tcam_key_area_t key;
    rdp_tcam_key_type key_type;
    uint16_t entry;
    rdp_tcam_context_t context;
    int key_size;
    int n;
    bdmf_error_t err;

    err = drv_tcam_mode_get(&key_type);
    if (err)
        return err;
    key_size = (key_type == RDP_TCAM_KEY_256) ? 256/8 : 512/8;

    n =  bdmf_strhex(key_str, key.b, sizeof(key.b));
    if (n != key_size)
    {
        bdmf_session_print(session, "Invalid key: expected %d bytes hex string\n", key_size);
        return BDMF_ERR_PARM;
    }

    err = drv_tcam_rule_find(&key, &entry, &context);

    if (err)
        return err;

    bdmf_session_print(session, "Found rule: index=%u context=0x%08x,0x%08x\n",
        entry, context.word[0], context.word[1]);

    return BDMF_ERR_OK;
}

/* Print TCAM table
 */
static int _tcam_cli_print_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    rdp_tcam_key_area_t key0, key1;
    rdp_tcam_key_area_t key, mask;
    rdp_tcam_context_t context;
    int valid_records = 0;
    bdmf_boolean is_valid;
    int entry;
    int i;

    bdmf_session_print(session, "TCAM table (%d x %d). Expected valid records: %u\n",
        tcam_max_entries, tcam_key_size, tcam_entries);
    for (entry = 0; entry < tcam_max_entries; entry++)
    {
        _drv_tcam_read(entry, RDP_TCAM_ADDRESS_KEY0, &is_valid, key0.b);
        if (!is_valid)
            continue;
        _drv_tcam_read(entry, RDP_TCAM_ADDRESS_KEY1, &is_valid, key1.b);
        _drv_tcam_context_read(entry, &context);
        /* Decode key0, key1 --> key, mask
         *      mask = key0 | key1
         *      key = key0
         * and check match
         *      key & mask == key & mask
         */
        for (i = 0; i < tcam_key_size / sizeof(uint32_t); i++)
        {
            key.w[i] = key0.w[i];
            mask.w[i] = key0.w[i] | key1.w[i];
        }


        bdmf_session_print(session, "%4.4d: context: 0x%08x 0x%08x 0x%08x 0x%08x\n", entry, context.word[0], context.word[1], context.word[2], context.word[3]);
        bdmf_session_print(session, "key :\n");
        bdmf_session_hexdump(session, key.b, 0, tcam_key_size);
        bdmf_session_print(session, "mask:\n");
        bdmf_session_hexdump(session, &mask.b, 0, tcam_key_size);
        bdmf_session_print(session, "\n");
        ++valid_records;
    }
    bdmf_session_print(session, "Valid records: %d\n", valid_records);
    return BDMF_ERR_OK;
}

void drv_tcam_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t tcam_cli_dir;
    static bdmfmon_enum_val_t tcam_key_size_enum_table[] = {
        { .name = "256", .val = RDP_TCAM_KEY_256},
        { .name = "512", .val = RDP_TCAM_KEY_512},
        BDMFMON_ENUM_LAST
    };
    ag_drv_tcam_cli_init(driver_dir);

    tcam_cli_dir = bdmfmon_dir_find(driver_dir, "tcam");
    if (!tcam_cli_dir)
        return;

    BDMFMON_MAKE_CMD(tcam_cli_dir, "mode", "Set / get TCAM mode", _tcam_cli_mode_handler,
        BDMFMON_MAKE_PARM_ENUM("key_size", "TCAM key size", tcam_key_size_enum_table, BDMFMON_PARM_FLAG_OPTIONAL));

    BDMFMON_MAKE_CMD(tcam_cli_dir, "new", "Add rule", _tcam_cli_add_handler,
        BDMFMON_MAKE_PARM_RANGE("priority", "Rule priority", BDMFMON_PARM_NUMBER, 0, 0, RDP_TCAM_MAX_PRIORITIES-1),
        BDMFMON_MAKE_PARM("key", "Key (32/64 byte hex string)", BDMFMON_PARM_STRING, 0),
        BDMFMON_MAKE_PARM("mask", "Mask (32/64 byte hex string)", BDMFMON_PARM_STRING, 0),
        BDMFMON_MAKE_PARM("ctx0", "Context[0]", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_DEFVAL("ctx1", "Context[1]", BDMFMON_PARM_NUMBER, 0, 0));

    BDMFMON_MAKE_CMD(tcam_cli_dir, "remove", "Delete rule", _tcam_cli_delete_handler,
        BDMFMON_MAKE_PARM("key", "Key (32/64 byte hex string)", BDMFMON_PARM_STRING, 0),
        BDMFMON_MAKE_PARM("mask", "Mask (32/64 byte hex string)", BDMFMON_PARM_STRING, 0));

    BDMFMON_MAKE_CMD(tcam_cli_dir, "find", "Find rule", _tcam_cli_find_handler,
        BDMFMON_MAKE_PARM("key", "Key (32/64 byte hex string)", BDMFMON_PARM_STRING, 0));

    BDMFMON_MAKE_CMD_NOPARM(tcam_cli_dir, "print", "Print TCAM table", _tcam_cli_print_handler);

}

void drv_tcam_cli_exit(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t tcam_cli_dir;

    tcam_cli_dir = bdmfmon_dir_find(driver_dir, "tcam");
    if (!tcam_cli_dir)
        return;
    bdmfmon_token_destroy(tcam_cli_dir);
}

#endif /* USE_BDMF_SHELL */

/*
 * TCAM indirect interface simulation
 */
#ifdef RDP_SIM

static inline bdmf_error_t _drv_tcam_key_in_get(uint8_t word_idx, uint32_t *p_value)
{
    return ag_drv_tcam_key_in_get(7 - word_idx, p_value);
}

/* TCAM record: valid, key, mask, context */
typedef struct
{
    bdmf_boolean valid;
    uint32_t key0[8];
    uint32_t key1[8];
} tcam_record_t;

static tcam_record_t tcam_mem[RDP_TCAM_ALL_TABLE_SIZE];

static void tcam_sim_set_done(void)
{
    uint32_t reg_op_done = 0;
    reg_op_done = RU_FIELD_SET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE, DONE, reg_op_done, 1);
    RU_REG_WRITE(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE, reg_op_done);
}

/* execute READ command */
static void tcam_sim_execute_read(void)
{
    bdmf_boolean key_ind;
    uint16_t entry_addr;
    uint32_t *key;
    int i;

    ag_drv_tcam_address_get(&key_ind, &entry_addr);
    if (entry_addr >= RDP_TCAM_ALL_TABLE_SIZE)
    {
        BDMF_TRACE_ERR("entry_addr %u is insane\n", entry_addr);
        return;
    }

    key = (key_ind == RDP_TCAM_ADDRESS_KEY0) ? tcam_mem[entry_addr].key0 : tcam_mem[entry_addr].key1;

    for (i = 0; i < 8; i++)
    {
        RU_REG_RAM_WRITE(0, i, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT, key[7-i]);
    }
    ag_drv_tcam_valid_out_set(tcam_mem[entry_addr].valid);
    tcam_sim_set_done();
}

/* execute WRITE command */
static void tcam_sim_execute_write(void)
{
    /* Read registers and update in memory */
    bdmf_boolean key_ind;
    uint16_t entry_addr;
    uint32_t *key;
    int i;

    ag_drv_tcam_address_get(&key_ind, &entry_addr);
    if (entry_addr >= RDP_TCAM_ALL_TABLE_SIZE)
    {
        BDMF_TRACE_ERR("entry_addr %u is insane\n", entry_addr);
        return;
    }
    key = (key_ind == RDP_TCAM_ADDRESS_KEY0) ? tcam_mem[entry_addr].key0 : tcam_mem[entry_addr].key1;
    ag_drv_tcam_valid_in_get(&tcam_mem[entry_addr].valid);
    /* Read key */
    for (i = 0; i < 8; i++)
    {
        _drv_tcam_key_in_get(i, &key[i]);
    }
    tcam_sim_set_done();
}

/* execute COMPARE command */
static void tcam_sim_execute_compare(void)
{
    int entry;
    uint32_t search_key[8];
    int is_match = 0;
    uint32_t reg_rslt = 0;
    int i;

    /* Read the requested key */
    for (i = 0; i < 8; i++)
    {
        _drv_tcam_key_in_get(i, &search_key[i]);
    }

    /*
     * Read TCAM tables row by row
     */
    for (entry = 0; entry < RDP_TCAM_ALL_TABLE_SIZE && !is_match; entry++)
    {
        if (!tcam_mem[entry].valid)
            continue;
        is_match = 1;
        for (i = 0; (i < 8) && is_match; i++)
        {
            uint32_t key, mask;
            /* Decode key0, key1 --> key, mask
             *      mask = key0 | key1
             *      key = key0
             * and check match
             *      key & mask == key & mask
             */
            mask = tcam_mem[entry].key0[i] | tcam_mem[entry].key1[i];
            key = tcam_mem[entry].key0[i];
            if ((mask & key) != (mask & search_key[i]))
                is_match = 0;
        }
    }

    /* Set result */
    reg_rslt = RU_FIELD_SET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_RSLT, MATCH, reg_rslt, is_match);
    reg_rslt = RU_FIELD_SET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_RSLT, INDEX, reg_rslt, (entry - 1));
    RU_REG_WRITE(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_RSLT, reg_rslt);

    tcam_sim_set_done();
}

/* execute INVALIDATE command */
static void tcam_sim_execute_invalidate(void)
{
    printf("%s: NOT SUPPORTED\n", __FUNCTION__);
}

/* Execute TCAM command */
static void tcam_sim_execute(void)
{
    uint8_t cmd;

    /* Read command */
    ag_drv_tcam_op_get(&cmd);

    /* Execute */
    switch (cmd)
    {
    case TCAM_CMD_READ:
        tcam_sim_execute_read();
        break;
    case TCAM_CMD_WRITE:
        tcam_sim_execute_write();
        break;
    case TCAM_CMD_COMPARE:
        tcam_sim_execute_compare();
        break;
    case TCAM_CMD_INVALIDATE:
        tcam_sim_execute_invalidate();
        break;
    default:
        BDMF_TRACE_ERR("Operation %u is insane\n", cmd);
        return;
    }
}

#endif /* #ifdef RDP_SIM */

#ifdef USE_BDMF_SHELL

/* Dump TCAM table for simulation */
bdmf_error_t drv_tcam_mem_dump(bdmf_session_handle session, const char *filename)
{
#ifdef RDP_SIM
    FILE *f;
    int i;

    f = fopen(filename, "w+");
    if (!f)
    {
        bdmf_session_print(session, "TCAM Dump: Can't open file %s for writing\n", filename);
        return BDMF_ERR_PARM;
    }

    /* Write records in binary big endian format. 
        1024 x 
           0     valid
           1-32  key0
           33-64 key1
           65-72 context
    */
    for (i = 0; i < RDP_TCAM_ALL_TABLE_SIZE; i++)
    {
        uint8_t valid = tcam_mem[i].valid;
        uint8_t b[4];
        rdp_tcam_context_t context;
        int j;
        int n;

        n = fwrite(&valid, 1, 1, f);
        if (n != 1)
            goto write_error;

        /* Write key0 */
        for (j = 0; j < 8; j++)
        {
            _drv_tcam_u32_to_u8(tcam_mem[i].key0[j], b);
            n = fwrite(b, 1, 4, f);
            if (n != 4)
                goto write_error;
        }

        /* Write key1 */
        for (j = 0; j < 8; j++)
        {
            _drv_tcam_u32_to_u8(tcam_mem[i].key1[j], b);
            n = fwrite(b, 1, 4, f);
            if (n != 4)
                goto write_error;
        }

        /* Write context */
        _drv_tcam_context_read(i, &context);
		
        for (j = 0; j < RDP_TCAM_CONTEXT_SIZE; j++)
        {
            _drv_tcam_u32_to_u8(context.word[j], b);
            n = fwrite(b, 1, 4, f);
            if (n != 4)
                goto write_error;
        }
    }
    fclose(f);
    return 0;

write_error:
    fclose(f);
    bdmf_session_print(session, "TCAM Dump: write error in record %d\n", i);
    return BDMF_ERR_IO;
#else
    bdmf_session_print(session, "TCAM Dump: TCAM simulation requires RDP_SIM define to be set\n");
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

#endif

