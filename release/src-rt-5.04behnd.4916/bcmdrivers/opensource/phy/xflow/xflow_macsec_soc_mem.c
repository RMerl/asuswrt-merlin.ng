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

#include "soc/mcm/enum_max.h"
#include "soc/mcm/enum_types.h"
#include "soc/mem.h"
#include "soc/memory.h"
#include "soc/field.h"
#include <soc/mcm/allenum.h>
#include "soc/feature.h"
#include "soc/68880/bchp_regs_int.h"
#include "xflow_macsec_defs.h"
#include "xflow_macsec.h"
#include "xflow_macsec_common.h"
#include "xflow_macsec_firelight.h"
#include "xflow_macsec_cfg_params.h"
#include "macsec_dev.h"
#include "macsec_types.h"
#include "macsec_macros.h"
#include "macsec_defs.h"
#include "macsec_common.h"
#include <linux/delay.h>

extern soc_control_t *soc_control[SOC_MAX_NUM_DEVICES];
int soc_mem_read_entry(int unit, soc_mem_info_t *meminfo, int copyno, int index, void *entry_data);
/*
 * Macro used by memory accessor functions to fix order
 */
#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? \
                                BYTES2WORDS((m)->bytes)-1-(v) : \
                                (v))

#define SOC_MEM_COMPARE_RETURN(a, b) {       \
        if ((a) < (b)) { return -1; }        \
        if ((a) > (b)) { return  1; }        \
}

uint32_t _soc_mem_entry_null_zeroes[SOC_MAX_MEM_WORDS];

#define SOC_MEM_TIMEOUT     100

#define START_BUSY_SHIFT    31
#define ACC_TYPE_SHIFT      28
#define READ_WRITE_SHIFT    27
#define DLEN_SHIFT          20
#define MEM_SEL_SHIFT       17
#define MEM_ID_SHIFT        12
#define INDEX_SHIFT         0

uint32_t mem_swap_32(uint32_t val)
{
    uint32_t swap_val = 0;

    swap_val = ((val >> 24) & 0x000000ff);
    swap_val |= ((val >> 8) & 0x0000ff00);
    swap_val |= ((val << 8) & 0x00ff0000);
    swap_val |= ((val << 24) & 0xff000000);

    return swap_val;
}

/*
 * Function:     soc_mem_field_length
 * Purpose:      Return the length of a memory field in bits.
 *               Value is 0 if field is not found.
 * Returns:      bits in field
 */
int soc_mem_field_length_slim(int unit, soc_mem_info_t *meminfo, soc_field_t field)
{
    soc_field_info_t    *fld;

    SOC_FIND_FIELD(field, meminfo->fields, meminfo->nFields, fld);
    if (fld == NULL)
    {
        PR_ERR("Could not find field: %d\n", field);
        return 0;
    }

    return fld->len;
}

/*
 * Function:     soc_mem_field_length
 * Purpose:      Return the length of a memory field in bits.
 *               Value is 0 if field is not found.
 * Returns:      bits in field
 */
int soc_mem_field_length(int unit, soc_mem_t mem, soc_field_t field)
{
    soc_field_info_t    *fld;

    SOC_FIND_FIELD(field, SOC_MEM_INFO(unit, mem).fields, SOC_MEM_INFO(unit, mem).nFields, fld);
    if (fld == NULL)
    {
        PR_ERR("Could not find field: %d\n", field);
        return 0;
    }

    return fld->len;
}

int _soc_mem_cmp_undef(int unit, void *ent_a, void *ent_b)
{
    COMPILER_REFERENCE(ent_a);
    COMPILER_REFERENCE(ent_b);

    return 0;
}

/*
 * TCAM memory cells require content data to be encoded before
 * writing, This function takes the encoded data and returns 
 * it in true memory format.
 */
static void _soc_mem_tcam_xy_to_dm_decode(int unit, uint32_t *key, uint32_t *mask, int bit_length, int xy_lpt)
{
    uint32_t xor_value;
    uint32_t converted_key, converted_mask;
    int word_length, word;

    xor_value = soc_feature(unit, soc_feature_xy_tcam_28nm) ? 0 : 0xffffffff;
    word_length = (bit_length + 31) / 32;

    for (word = 0; word < word_length; word++)
    {
        /*
         * LPT -> XY
         * xy_x[1:0] = {(lpt_y[0] & lpt_x[0]), (lpt_y[1] & lpt_y[0])}
         * xy_y[1:0] = {(lpt_y[1] & lpt_x[1]), (lpt_x[1] & lpt_x[0])}
         */
        if (xy_lpt)
        {
            converted_key = (((key[word] & mask[word]) << 1) & 0xaaaaaaaa) |
                            ((mask[word] & (mask[word] >> 1)) & 0x55555555);
            converted_mask = ((key[word] & mask[word]) & 0xaaaaaaaa) |
                             ((key[word] & (key[word] >> 1)) & 0x55555555);
            key[word] = converted_key;
            mask[word] = converted_mask;
        }
        /*
         * 40nm:
         *    Encode: K0 = MASK & KEY
         *            K1 = ~MASK | KEY
         *    Decode: KEY = K0
         *            MASK = K0 | ~K1
         *              (encode)  (decode)
         *    KEY MASK   K0  K1   KEY MASK
         *    --------   ------   --------
         *     0   0     0   1     0   0
         *     1   0     0   1     0   0  =====> info loss
         *     0   1     0   0     0   1
         *     1   1     1   1     1   1
         * 28nm:
         *     Encode: K0 = MASK & KEY
         *             K1 = MASK & ~KEY
         *     Decode: KEY = K0
         *             MASK = K0 | K1
         *     KEY MASK   K0  K1   KEY MASK
         *     --------   ------   --------
         *      0   0     0   0     0   0
         *      1   0     0   0     0   0  =====> info loss
         *      0   1     0   1     0   1
         *      1   1     1   0     1   1
         * Notes:
         *     - Mask value of 1 means to compare against key
         *     - K0 is in KEY field of the entry
         *     - K1 is in MASK field of the entry
         */
        mask[word] = key[word] | (mask[word] ^ xor_value);
    }

    if ((bit_length & 0x1f) != 0)
    {
        if (xy_lpt)
        {
            key[word - 1] &= (1 << (bit_length & 0x1f)) - 1;
        }
        mask[word - 1] &= (1 << (bit_length & 0x1f)) - 1;
    }
}

/*
 * TCAM memory cells require content data to be encoded before
 * writing, This function takes the raw data and returns 
 * it in encoded format for writing.
 */
static void _soc_mem_tcam_dm_to_xy_encode(int unit, uint32_t *key, uint32_t *mask, int bit_length, int xy_lpt)
{
    uint32_t xor_value;
    int no_trans = FALSE;
    uint32_t converted_key, converted_mask;
    int word_length, word;

    if (!soc_feature(unit, soc_feature_xy_tcam_direct))
    {
        /* Only clear the "don't care" key bits */
        no_trans = TRUE;
    }
    xor_value = soc_feature(unit, soc_feature_xy_tcam_28nm) ? 0xffffffff : 0;

    word_length = (bit_length + 31) / 32;

    for (word = 0; word < word_length; word++)
    {
        /*
         * 40nm:
         *    Encode: K0 = MASK & KEY
         *            K1 = ~MASK | KEY
         *    Decode: KEY = K0
         *            MASK = K0 | ~K1
         *              (encode)  (decode)
         *    KEY MASK   K0  K1   KEY MASK
         *    --------   ------   --------
         *     0   0     0   1     0   0
         *     1   0     0   1     0   0  =====> info loss
         *     0   1     0   0     0   1
         *     1   1     1   1     1   1
         * 28nm:
         *     Encode: K0 = MASK & KEY
         *             K1 = MASK & ~KEY
         *     Decode: KEY = K0
         *             MASK = K0 | K1
         *     KEY MASK   K0  K1   KEY MASK
         *     --------   ------   --------
         *      0   0     0   0     0   0
         *      1   0     0   0     0   0  =====> info loss
         *      0   1     0   1     0   1
         *      1   1     1   0     1   1
         * Notes:
         *     - Mask value of 1 means to compare against key
         *     - K0 is in KEY field of the entry
         *     - K1 is in MASK field of the entry
         */
        converted_key = key[word] & mask[word];

        if (!no_trans)
        {
            converted_mask = (key[word] | ~mask[word]) ^ xor_value;
            mask[word] = converted_mask;
        }
        key[word] = converted_key;
        /*
         * XY -> LPT
         * lpt_x[1:0] = {(xy_y[1] | xy_y[0]), (xy_x[1] | xy_y[0])}
         * lpt_y[1:0] = {(xy_y[1] | xy_x[0]), (xy_x[1] | xy_x[0])}
         */
        if (xy_lpt)
        {
            converted_key = ((mask[word] | (mask[word] << 1)) & 0xaaaaaaaa) |
                            ((mask[word] | (key[word] >> 1)) & 0x55555555);
            converted_mask = ((mask[word] | (key[word] << 1)) & 0xaaaaaaaa) |
                             ((key[word] | (key[word] >> 1)) & 0x55555555);
            key[word] = converted_key;
            mask[word] = converted_mask;
        }
    }

    if ((bit_length & 0x1f) != 0)
    {
        if (xy_lpt)
        {
            key[word - 1] &= (1 << (bit_length & 0x1f)) - 1;
        }
        mask[word - 1] &= (1 << (bit_length & 0x1f)) - 1;
    }
}

void _soc_mem_tcam_xy_to_dm_coupled_slim(int unit, soc_mem_t key_mem, soc_mem_t mask_mem,
                                        soc_mem_info_t *key_meminfo, soc_mem_info_t *mask_meminfo,
                                        uint32_t *xy_key_entry, uint32_t *xy_mask_entry,
                                        uint32_t *dm_key_entry, uint32_t *dm_mask_entry,
                                        int count)
{
    uint32_t key[SOC_MAX_MEM_WORDS * 4] = {};
    uint32_t *mask;
    soc_field_t key_field[4], mask_field[4];
    int bit_length[4], field_count;
    int index, i, key_data_words, key_data_bytes;
    int mask_data_words, mask_data_bytes;
    int xy_lpt;

    xy_lpt = (soc_feature(unit, soc_feature_xy_tcam_lpt) && key_meminfo->flags & SOC_MEM_FLAG_TCAM_ENCODING_LPT);

    if (key_mem == ISEC_SP_TCAM_KEYm && mask_mem == ISEC_SP_TCAM_MASKm)
    {
        uint32_t isec_ctrl = 0;

        key_field[0] = KEYf;
        mask_field[0] = MASKf;
        field_count = 1;

        soc_ubus_reg32_get(unit, ISEC_SER_CONTROLreg, REG_PORT_ANY, &isec_ctrl);
        xy_lpt = soc_ubus_reg_field_get(unit, ISEC_SER_CONTROLreg, isec_ctrl, SP_TCAM_LPT_fld);
    }
    else
    {
        /* Not supported for other devices */
        return;
    }

    mask = &key[SOC_MAX_MEM_WORDS];

    for (i = 0; i < field_count; i++)
    {
        /* Expect key and mask has the same bit length, using key bit length
           for both */
        bit_length[i] = soc_mem_field_length_slim(unit, key_meminfo, key_field[i]);
    }

    key_data_bytes = key_meminfo->bytes;
    key_data_words = (key_data_bytes/sizeof(uint32_t));

    mask_data_bytes = mask_meminfo->bytes;
    mask_data_words = (mask_data_bytes/sizeof(uint32_t));

    for (index = 0; index < count; index++)
    {
        if (xy_key_entry != dm_key_entry)
        {
            memcpy(dm_key_entry, xy_key_entry, key_data_bytes);
        }

        if (xy_mask_entry != dm_mask_entry)
        {
            memcpy(dm_mask_entry, xy_mask_entry, mask_data_bytes);
        }

        for (i = 0; i < field_count; i++)
        {
            soc_meminfo_field_get(key_mem, key_meminfo, xy_key_entry, key_field[i], key, key_data_words);
            soc_meminfo_field_get(mask_mem, mask_meminfo, xy_mask_entry, mask_field[i], mask, mask_data_words);

            _soc_mem_tcam_xy_to_dm_decode(unit, key, mask, bit_length[i], xy_lpt);
            if (xy_lpt)
            {
                soc_meminfo_field_set(unit, key_meminfo, dm_key_entry, key_field[i], key);
            }
            soc_meminfo_field_set(unit, mask_meminfo, dm_mask_entry, mask_field[i], mask);
        }

        dm_key_entry += key_data_words;
        xy_key_entry += key_data_words;
        dm_mask_entry += mask_data_words;
        xy_mask_entry += mask_data_words;
    }
}

void _soc_mem_tcam_xy_to_dm_coupled(int unit, soc_mem_t key_mem, soc_mem_t mask_mem, uint32_t *xy_key_entry,
                                    uint32_t *xy_mask_entry, uint32_t *dm_key_entry, uint32_t *dm_mask_entry, int count)
{
    uint32_t key[SOC_MAX_MEM_FIELD_WORDS], mask[SOC_MAX_MEM_FIELD_WORDS];
    soc_field_t key_field[4], mask_field[4];
    int bit_length[4], field_count;
    int index, i, key_data_words, key_data_bytes;
    int mask_data_words, mask_data_bytes;
    int xy_lpt;

    xy_lpt = soc_feature(unit, soc_feature_xy_tcam_lpt) &&
             (SOC_MEM_INFO(unit, key_mem).flags &
              SOC_MEM_FLAG_TCAM_ENCODING_LPT);

    if (SOC_IS_FIRELIGHT(unit) &&
        key_mem == ISEC_SP_TCAM_KEYm &&
        mask_mem == ISEC_SP_TCAM_MASKm)
    {
        uint32_t isec_ctrl = 0;

        key_field[0] = KEYf;
        mask_field[0] = MASKf;
        field_count = 1;

        soc_ubus_reg32_get(unit, ISEC_SER_CONTROLreg, REG_PORT_ANY, &isec_ctrl);
        xy_lpt = soc_ubus_reg_field_get(unit, ISEC_SER_CONTROLreg, isec_ctrl, SP_TCAM_LPT_fld);
    }
    else
    {
        /* Not supported for other devices */
        return;
    }

    for (i = 0; i < field_count; i++)
    {
        /* Expect key and mask has the same bit length, using key bit length
           for both */
        bit_length[i] = soc_mem_field_length(unit, key_mem, key_field[i]);
    }

    key_data_words = soc_mem_entry_words(unit, key_mem);
    key_data_bytes = key_data_words * sizeof(uint32_t);

    mask_data_words = soc_mem_entry_words(unit, mask_mem);
    mask_data_bytes = mask_data_words * sizeof(uint32_t);

    for (index = 0; index < count; index++)
    {
        if (xy_key_entry != dm_key_entry)
        {
            memcpy(dm_key_entry, xy_key_entry, key_data_bytes);
        }
        if (xy_mask_entry != dm_mask_entry)
        {
            memcpy(dm_mask_entry, xy_mask_entry, mask_data_bytes);
        }

        for (i = 0; i < field_count; i++)
        {
            soc_mem_field_get(
                unit, key_mem, xy_key_entry, key_field[i], key);
            soc_mem_field_get(
                unit, mask_mem, xy_mask_entry, mask_field[i], mask);
            _soc_mem_tcam_xy_to_dm_decode(
                unit, key, mask, bit_length[i], xy_lpt);
            if (xy_lpt)
            {
                soc_mem_field_set(
                    unit, key_mem, dm_key_entry, key_field[i], key);
            }
            soc_mem_field_set(
                unit, mask_mem, dm_mask_entry, mask_field[i], mask);
        }
        dm_key_entry += key_data_words;
        xy_key_entry += key_data_words;
        dm_mask_entry += mask_data_words;
        xy_mask_entry += mask_data_words;
    }
}

void _soc_mem_tcam_dm_to_xy_coupled(int unit, soc_mem_t key_mem, soc_mem_t mask_mem, uint32_t *dm_key_entry,
                                    uint32_t *dm_mask_entry, uint32_t *xy_key_entry, uint32_t *xy_mask_entry, int count)
{
    uint32_t key[SOC_MAX_MEM_FIELD_WORDS], mask[SOC_MAX_MEM_FIELD_WORDS];
    soc_field_t key_field[4], mask_field[4];
    int bit_length[4], field_count;
    int index, i, key_data_words, key_data_bytes;
    int mask_data_words, mask_data_bytes;
    int xy_lpt;

    xy_lpt = soc_feature(unit, soc_feature_xy_tcam_lpt) &&
                        (SOC_MEM_INFO(unit, key_mem).flags & SOC_MEM_FLAG_TCAM_ENCODING_LPT);

    if (SOC_IS_FIRELIGHT(unit) && key_mem == ISEC_SP_TCAM_KEYm && mask_mem == ISEC_SP_TCAM_MASKm)
    {
        uint32_t isec_ctrl=0;

        key_field[0] = KEYf;
        mask_field[0] = MASKf;
        field_count = 1;

        soc_ubus_reg32_get(unit, ISEC_SER_CONTROLreg, REG_PORT_ANY, &isec_ctrl);
        xy_lpt = soc_ubus_reg_field_get(unit, ISEC_SER_CONTROLreg, isec_ctrl, SP_TCAM_LPT_fld);
    }
    else
    {
        /* Not supported for other devices */
        return;
    }

    for (i = 0; i < field_count; i++)
    {
        /* Expect key and mask has the same bit length, using key bit length
           for both */
        bit_length[i] = soc_mem_field_length(unit, key_mem, key_field[i]);
    }

    key_data_words = soc_mem_entry_words(unit, key_mem);
    key_data_bytes = key_data_words * sizeof(uint32_t);

    mask_data_words = soc_mem_entry_words(unit, mask_mem);
    mask_data_bytes = mask_data_words * sizeof(uint32_t);

    for (index = 0; index < count; index++)
    {
        if (xy_key_entry != dm_key_entry)
        {
            memcpy(xy_key_entry, dm_key_entry, key_data_bytes);
        }
        if (xy_mask_entry != dm_mask_entry)
        {
            memcpy(xy_mask_entry, dm_mask_entry, mask_data_bytes);
        }

        for (i = 0; i < field_count; i++)
        {
            soc_mem_field_get(
                unit, key_mem, dm_key_entry, key_field[i], key);
            soc_mem_field_get(
                unit, mask_mem, dm_mask_entry, mask_field[i], mask);
            _soc_mem_tcam_dm_to_xy_encode(
                unit, key, mask, bit_length[i], xy_lpt);

            soc_mem_field_set(
                unit, key_mem, xy_key_entry, key_field[i], key);
            soc_mem_field_set(
                unit, mask_mem, xy_mask_entry, mask_field[i], mask);
        }

        dm_key_entry += key_data_words;
        xy_key_entry += key_data_words;
        dm_mask_entry += mask_data_words;
        xy_mask_entry += mask_data_words;
    }
}

/*
 * Bulk write for memory entry array.
 */
int soc_mem_bulk_write(int unit, soc_mem_t *mem, int *index, int *copyno, uint32_t **entry_data, int count)
{
    int rv = SOC_E_NONE;

    if (mem[0] == ISEC_SP_TCAM_KEYm && mem[1] == ISEC_SP_TCAM_MASKm)
    {
        uint32_t xy_entry_0[SOC_MAX_MEM_WORDS];
        uint32_t xy_entry_1[SOC_MAX_MEM_WORDS];

        _soc_mem_tcam_dm_to_xy_coupled(unit, mem[0], mem[1], entry_data[0], entry_data[1], xy_entry_0, xy_entry_1, 1);

        rv = soc_mem_write(unit, mem[0], MEM_BLOCK_ALL, index[0], xy_entry_0);
        if (SOC_FAILURE(rv))
        {
            return rv;
        }

        rv = soc_mem_write(unit, mem[1], MEM_BLOCK_ALL, index[1], xy_entry_1);
    }
    return rv;
}

/*
 * Function:     soc_meminfo_field32_force
 * Purpose:      Set a <=32 bit field out of a memory entry
 *               without checking for field width.  Lower
 *               bits of value are taken.
 * Returns:      Value put in field.
 */
void soc_meminfo_field32_force(soc_mem_t mem, soc_mem_info_t *meminfo, void *entry, soc_field_t field, uint32_t value)
{
    soc_field_info_t    *fieldinfo;
    int                 len;

    SOC_FIND_FIELD(field, meminfo->fields, meminfo->nFields, fieldinfo);

    /* coverity[var_deref_opl : FALSE] */
    len = fieldinfo->len;

    if (len < 32)                     /* Force value to fit in field width */
    {
        value &= (1 << len) - 1;
    }

    /*
     * COVERITY
     *
     * We do this intentionally to use the generic function
     * soc_meminfo_field_set() which does the necessary handling.
     *
     */
    /* coverity[address_of] */
    /* coverity[callee_ptr_arith] */
    soc_meminfo_field_set(mem, meminfo, entry, field, &value);
}

/*
 * Function:      _soc_field_value_fit
 * Purpose:       Check if value will fit into a memory field.
 * Parameters:    fieldinfo --  (IN)Direct reference to field description.
 *                value     --  (IN)Value to be checked.
 * Returns:
 *      TRUE  - buffer fits into the field.
 *      FALSE - Otherwise.
 */
static int _soc_field_value_fit(soc_field_info_t *fieldinfo, uint32_t *value)
{
    uint32_t      mask;    
    uint16_t      len;     
    int         idx;     

    idx = (fieldinfo->len - 1) >> 5;
    len = fieldinfo->len % 32;

    if (!len)
       return TRUE;

    mask = (1 << len) - 1;

    if ((value[idx] & ~mask) != 0)
        return FALSE;
    
    return TRUE;
}

/*
 * Function:
 *      soc_mem_field32_fit
 * Purpose:
 *      Check if uint32_t value fits into a memory field.
 * Parameters:
 *      unit    - (IN)SOC unit number.
 *      mem     - (IN)Memory id.
 *      field   - (IN)Field id.
 *      value   - (IN)Value to be checked.
 * Return:
 *      SOC_E_PARAM -If value is too big for field, or some other error.
 *      SOC_E_NONE  -Otherwise.
 */
int soc_mem_field32_fit(int unit, soc_mem_t mem, soc_field_t field, uint32_t value)
{
    soc_mem_info_t      *meminfo;
    soc_field_info_t    *fieldinfo;

    /* Get memory info. */
    if (!SOC_MEM_IS_VALID(unit, mem))
       return SOC_E_PARAM;

    meminfo = &SOC_MEM_INFO(unit, mem);

    /* Get field properties. */
    SOC_FIND_FIELD(field, meminfo->fields, meminfo->nFields, fieldinfo);

    if (!fieldinfo)
        return (SOC_E_PARAM);

    /* coverity[callee_ptr_arith : FALSE] */
    return (_soc_field_value_fit(fieldinfo, &value)) ? SOC_E_NONE : SOC_E_PARAM;
}

/*
 * Function:
 *      soc_mem_field_valid
 * Purpose:
 *      Verify if field is valid & present in memory
 * Parameters:
 *      mem     - (IN)Memory id.
 *      field   - (IN)Field id.
 * Return:
 *      TRUE  -If field is present & valid.
 *      FALSE -Otherwise.
 */
int soc_mem_field_valid(int unit, soc_mem_t mem, soc_field_t field)
{
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *finfop;

    /* Verify that memory is present on the device. */
    if (!SOC_MEM_IS_VALID(unit, mem))
        return FALSE;

    meminfo = &SOC_MEM_INFO(unit, mem);

    SOC_FIND_FIELD(field, meminfo->fields, meminfo->nFields, finfop);

    return (finfop != NULL) ? TRUE : FALSE;
}

/*
 * Function:    soc_mem_mask_field_set
 * Purpose:     Set a memory mask field
 * Parameters:  unit - device
 *              mem - table
 *              entbuf - table entry buffer
 *              field - which field to set
 *              fldbuf - field buffer
 */
void soc_mem_mask_field_set(int unit, soc_mem_t mem, uint32_t *entbuf, soc_field_t field, uint32_t *fldbuf)
{
    soc_mem_info_t *meminfo = &SOC_MEM_INFO(unit, mem);

    soc_meminfo_field_set(mem, meminfo, entbuf, field, fldbuf);
}

/*
 * Function:    soc_mem_mask_field_get
 * Purpose:     Get a memory mask field
 * Parameters:  unit - device
 *              mem - table
 *              entbuf - table entry buffer
 *              field - which field to get
 *              fldbuf - field buffer
 */
uint32_t *soc_mem_mask_field_get(int unit, soc_mem_t mem, const uint32_t *entbuf, soc_field_t field, uint32_t *fldbuf)
{
    uint32_t *rfldbuf;
    soc_mem_info_t *meminfo = &SOC_MEM_INFO(unit, mem);

    rfldbuf = soc_meminfo_field_get(mem, meminfo, entbuf, field, fldbuf, SOC_MAX_MEM_WORDS);

    return rfldbuf;
}

/*
 * Function:      soc_meminfo_field_get
 * Purpose:       Get a memory field without reference to chip.
 * Parameters:    meminfo   --  direct reference to memory description
 */
uint32_t *soc_meminfo_field_get(soc_mem_t mem, soc_mem_info_t *meminfo, const uint32_t *entbuf, 
                              soc_field_t field, uint32_t *fldbuf, uint32_t fldbuf_size)
{
    soc_field_info_t    *fieldinfo;
    int                 i, wp, bp, len;

    SOC_FIND_FIELD(field, meminfo->fields, meminfo->nFields, fieldinfo);

    bp = fieldinfo->bp;
    len = fieldinfo->len;
    
    if (len == 1)
    {     /* special case single bits */
        wp = bp / 32;
        bp = bp & (32 - 1);
        if (entbuf[FIX_MEM_ORDER_E(wp, meminfo)] & (1<<bp))
        {
            fldbuf[0] = 1;
        }
        else
        {
            fldbuf[0] = 0;
        }
        return fldbuf;
    }

    if (fieldinfo->flags & SOCF_LE)
    {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;

        for (; len > 0; len -= 32)
        {
            if (bp)
            {
                fldbuf[i] = entbuf[FIX_MEM_ORDER_E(wp++, meminfo)] >> bp & ((1 << (32 - bp)) - 1);
                if ( len > (32 - bp) )
                {
                    fldbuf[i] |= entbuf[FIX_MEM_ORDER_E(wp, meminfo)] << (32 - bp);
                }
            }
            else
            {
                fldbuf[i] = entbuf[FIX_MEM_ORDER_E(wp++, meminfo)];
            }

            if (len < 32)
            {
                fldbuf[i] &= ((1 << len) - 1);
            }

            i++;
        }
    }
    else
    {
        i = (len - 1) / 32;

        while (len > 0)
        {
            fldbuf[i] = 0;

            do
            {
                fldbuf[i] = (fldbuf[i] << 1) | ((entbuf[FIX_MEM_ORDER_E(bp / 32, meminfo)] >> (bp & (32 - 1))) & 1);
                len--;
                bp++;
            } while (len & (32 - 1));

            i--;
        }
    }

    return fldbuf;
}

/*
 * Function:    soc_mem_field_get
 * Purpose:     Get a memory field
 * Parameters:  unit - device
 *              mem - table
 *              entbuf - table entry buffer
 *              field - which field to get
 *              fldbuf - field buffer
 */
uint32_t *soc_mem_field_get(int unit, soc_mem_t mem, const uint32_t *entbuf, soc_field_t field, uint32_t *fldbuf)
{
    soc_mem_info_t *meminfo = &SOC_MEM_INFO(unit, mem);

    return soc_meminfo_field_get(mem, meminfo, entbuf, field, fldbuf, SOC_MAX_MEM_WORDS);
}

/*
 * Function:     soc_mem_field32_get
 * Purpose:      Get a <=32 bit field out of a memory entry
 * Returns:      The value of the field
 */
uint32_t soc_mem_field32_get(int unit, soc_mem_t mem, const void *entbuf, soc_field_t field)
{
    uint32_t              value;

    soc_meminfo_field_get(mem, &SOC_MEM_INFO(unit, mem), entbuf, field, &value, 1);

    return value;
}

/*
 * Function:      soc_meminfo_field_set
 * Purpose:       Set a memory field without reference to chip.
 * Parameters:    meminfo   --  direct reference to memory description
 */
void soc_meminfo_field_set(soc_mem_t mem, soc_mem_info_t *meminfo, uint32_t *entbuf, soc_field_t field, uint32_t *fldbuf)
{
    soc_field_info_t    *fieldinfo;
    uint32_t              mask;
    int                 i, wp, bp, len;

    SOC_FIND_FIELD(field, meminfo->fields, meminfo->nFields, fieldinfo);

    /* Make sure value fits into the field. */
    if (!_soc_field_value_fit(fieldinfo, fldbuf))
    {
        PR_ERR("value not fits into the field");
        return;
    }

    bp = fieldinfo->bp;

    if (fieldinfo->flags & SOCF_LE)
    {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;

        for (len = fieldinfo->len; len > 0; len -= 32)
        {
            if (bp)
            {
                if (len < 32)
                {
                    mask = (1 << len) - 1;
                }
                else
                {
                    mask = -1;
                }

                entbuf[FIX_MEM_ORDER_E(wp, meminfo)] &= ~(mask << bp);
                entbuf[FIX_MEM_ORDER_E(wp++, meminfo)] |= fldbuf[i] << bp;
                if (len > (32 - bp))
                {
                    entbuf[FIX_MEM_ORDER_E(wp, meminfo)] &= ~(mask >> (32 - bp));
                    entbuf[FIX_MEM_ORDER_E(wp, meminfo)] |= fldbuf[i] >> (32 - bp) & ((1 << bp) - 1);
                }
            }
            else
            {
                if (len < 32)
                {
                    mask = (1 << len) - 1;
                    entbuf[FIX_MEM_ORDER_E(wp, meminfo)] &= ~mask;
                    entbuf[FIX_MEM_ORDER_E(wp++, meminfo)] |= fldbuf[i] << bp;
                }
                else
                {
                    entbuf[FIX_MEM_ORDER_E(wp++, meminfo)] = fldbuf[i];
                }
            }

            i++;
        }
    }
    else
    {   /* Big endian: swap bits */
        len = fieldinfo->len;

        while (len > 0)
        {
            len--;
            entbuf[FIX_MEM_ORDER_E(bp / 32, meminfo)] &= ~(1 << (bp & (32-1)));
            entbuf[FIX_MEM_ORDER_E(bp / 32, meminfo)] |= (fldbuf[len / 32] >> (len & (32-1)) & 1) << (bp & (32-1));
            bp++;
        }
    }
}

/*
 * Function:    soc_mem_field_set
 * Purpose:     Set a memory field
 * Parameters:  unit - device
 *              mem - table
 *              entbuf - table entry buffer
 *              field - which field to set
 *              fldbuf - field buffer
 */
void soc_mem_field_set(int unit, soc_mem_t mem, uint32_t *entbuf, soc_field_t field, uint32_t *fldbuf)
{
    soc_mem_info_t *meminfo = &SOC_MEM_INFO(unit, mem);

    soc_meminfo_field_set(mem, meminfo, entbuf, field, fldbuf);
}

/*
 * Function:
 *     soc_mem_field64_get
 * Purpose:
 *     Get a field in a memory for a uint64_t type
 * Parameters:
 *     unit  - (IN) BCM device number.
 *     mem   - (IN) Memory id.
 *     entry - (IN) HW entry buffer.
 *     field - (IN) Memory field id.
 *     val64 - (OUT) SW uint64_t field buffer.
 * Returns:      void
 */
void soc_mem_field64_get(int unit, soc_mem_t mem, void *entry, soc_field_t field, uint64_t *val64)
{
    uint32_t              val64_field[2] = {0, 0};

    soc_mem_field_get(unit, mem, entry, field, val64_field);

    COMPILER_64_SET(*val64, val64_field[1], val64_field[0]);
}

/*
 * Function:     soc_mem_field32_set
 * Purpose:      Set a <=32 bit field out of a memory entry
 * Returns:      void
 */
void soc_mem_field32_set(int unit, soc_mem_t mem, void *entbuf, soc_field_t field, uint32_t value)
{
    soc_mem_field_set(unit, mem, entbuf, field, &value);
}

/*
 * Function:
 *     soc_mem_field64_set
 * Purpose:
 *     Set a field in a memory from a uint64_t type
 * Parameters:
 *     unit  - (IN) BCM device number.
 *     mem   - (IN) Memory id.
 *     entry - (IN) HW entry buffer.
 *     field - (IN) Memory field id.
 *     val64 - (IN) SW uint64_t field buffer.
 * Returns:      void
 */
void soc_mem_field64_set(int unit, soc_mem_t mem, void *entry, soc_field_t field, const uint64_t val64)
{
    uint32_t              val64_field[2];

    val64_field[0] = COMPILER_64_LO(val64);
    val64_field[1] = COMPILER_64_HI(val64);

    soc_mem_field_set(unit, mem, entry, field, val64_field);
}

/*
 * Function:     soc_mem_mac_addr_set
 * Purpose:      Set a mac address field in a memory from a mac addr type
 * Returns:      void
 */
void soc_mem_mac_addr_set(int unit, soc_mem_t mem, void *entry, soc_field_t field, const sal_mac_addr_t mac)
{
    uint32_t              mac_field[2];

    SAL_MAC_ADDR_TO_UINT32(mac, mac_field);

    soc_mem_field_set(unit, mem, entry, field, mac_field);
}

/*
 * Function:     soc_mem_mac_addr_get
 * Purpose:      Get a mac address field in a memory from a mac addr type
 * Returns:      SOC_E_xxx
 */
void soc_mem_mac_addr_get(int unit, soc_mem_t mem, const void *entry, soc_field_t field, sal_mac_addr_t mac)
{
    uint32_t              mac_field[2];

    soc_mem_field_get(unit, mem, entry, field, mac_field);

    SAL_MAC_ADDR_FROM_UINT32(mac, mac_field);
}

#if 0
int _soc_mem_cmp_egr_vp_vlan_membership(int unit, void *ent_a, void *ent_b)
{
    uint32_t val_a, val_b;

    val_a = soc_mem_field32_get(unit, EGR_VP_VLAN_MEMBERSHIPm, ent_a, VLANf);
    val_b = soc_mem_field32_get(unit, EGR_VP_VLAN_MEMBERSHIPm, ent_b, VLANf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    val_a = soc_mem_field32_get(unit, EGR_VP_VLAN_MEMBERSHIPm, ent_a, VPf);
    val_b = soc_mem_field32_get(unit, EGR_VP_VLAN_MEMBERSHIPm, ent_b, VPf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    return 0;
}

int _soc_mem_cmp_endpoint_queue_map(int unit, void *ent_a, void *ent_b)
{
    uint32_t val_a, val_b;

    val_a = soc_mem_field32_get(unit, ENDPOINT_QUEUE_MAPm, ent_a, KEY_TYPEf);
    val_b = soc_mem_field32_get(unit, ENDPOINT_QUEUE_MAPm, ent_b, KEY_TYPEf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    switch (val_a)
    {
        case 0: /* endpoint queueing */
            val_a = soc_mem_field32_get(unit, ENDPOINT_QUEUE_MAPm, ent_a, EH_QUEUE_TAGf);
            val_b = soc_mem_field32_get(unit, ENDPOINT_QUEUE_MAPm, ent_b, EH_QUEUE_TAGf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            val_a = soc_mem_field32_get(unit, ENDPOINT_QUEUE_MAPm, ent_a, DEST_PORTf);
            val_b = soc_mem_field32_get(unit, ENDPOINT_QUEUE_MAPm, ent_b, DEST_PORTf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            return 0;
        default:
            return 1;
    }

    return 0;
}
#endif 

/*
 * Function:
 *    _soc_mem_tcam_dm_to_xy
 * Purpose:
 *    Encode direct memory buffer to the XY format for TCAMs
 * Notes:
 * 
 */
void _soc_mem_tcam_dm_to_xy(int unit, soc_mem_t mem, int count, uint32_t *dm_entry, uint32_t *xy_entry,
                            uint32_t *cache_entry)
{
    uint32_t key[SOC_MAX_MEM_FIELD_WORDS], mask[SOC_MAX_MEM_FIELD_WORDS];
    soc_field_t key_field[4], mask_field[4];
    int bit_length[4], word_length[4], field_count;
    int index, i, word, data_words, data_bytes;
    int xy_lpt;

    xy_lpt = (SOC_MEM_INFO(unit, mem).flags & SOC_MEM_FLAG_TCAM_ENCODING_LPT);

    if (SOC_IS_FIRELIGHT(unit))
    {
        if (mem == ISEC_SC_TCAMm)
        {
            uint32_t isec_ctrl = 0;

            soc_ubus_reg32_get(unit, ISEC_SER_CONTROLreg, REG_PORT_ANY, &isec_ctrl);
            xy_lpt = soc_ubus_reg_field_get(unit, ISEC_SER_CONTROLreg, isec_ctrl, SC_TCAM_LPT_fld);
        }
    }

    key_field[0] = KEYf;
    mask_field[0] = MASKf;
    field_count = 1;

    for (i = 0; i < field_count; i++)
    {
        bit_length[i] = soc_mem_field_length(unit, mem, key_field[i]);
        word_length[i] = (bit_length[i] + 31) / 32;
    }

    data_words = soc_mem_entry_words(unit, mem);
    data_bytes = data_words * sizeof(uint32_t);

    for (index = 0; index < count; index++)
    {
        if (xy_entry != dm_entry)
        {
            memcpy(xy_entry, dm_entry, data_bytes);
        }

        if (cache_entry)
        {
            memcpy(cache_entry, dm_entry, data_bytes);
        }

        for (i = 0; i < field_count; i++)
        {
            soc_mem_field_get(unit, mem, dm_entry, key_field[i], key);
            soc_mem_field_get(unit, mem, dm_entry, mask_field[i], mask);

            if (cache_entry)
            {
                for (word = 0; word < word_length[i]; word++)
                {
                    key[word] &= mask[word];
                }
                soc_mem_field_set(unit, mem, cache_entry, key_field[i], key);
            }

            _soc_mem_tcam_dm_to_xy_encode(unit, key, mask, bit_length[i], xy_lpt);

            soc_mem_field_set(unit, mem, xy_entry, key_field[i], key);
            soc_mem_field_set(unit, mem, xy_entry, mask_field[i], mask);
        }

        dm_entry += data_words;
        xy_entry += data_words;
        if (cache_entry)
        {
            cache_entry += data_words;
        }
    }
}

/*
 * Function:
 *    _soc_mem_tcam_xy_to_dm
 * Purpose:
 *    Decode XY memory buffer from the TCAM to direct read buffer
 * Notes:
 * 
 */
void _soc_mem_tcam_xy_to_dm(int unit, soc_mem_t mem, int count, uint32_t *xy_entry, uint32_t *dm_entry)
{
    
    uint32_t key[SOC_MAX_MEM_FIELD_WORDS], mask[SOC_MAX_MEM_FIELD_WORDS];
    soc_field_t key_field[4], mask_field[4];
    int bit_length[4], field_count;
    int index, i, data_words, data_bytes;
    int xy_lpt;

    xy_lpt = soc_feature(unit, soc_feature_xy_tcam_lpt) &&
                        (SOC_MEM_INFO(unit, mem).flags & SOC_MEM_FLAG_TCAM_ENCODING_LPT);

    if (SOC_IS_FIRELIGHT(unit))
    {
        if (mem == ISEC_SC_TCAMm)
        {
            uint32_t isec_ctrl=0;

            soc_ubus_reg32_get(unit, ISEC_SER_CONTROLreg, REG_PORT_ANY, &isec_ctrl);
            xy_lpt = soc_ubus_reg_field_get(unit, ISEC_SER_CONTROLreg, isec_ctrl, SC_TCAM_LPT_fld);
        }
    }

    if (SOC_MEM_FIELD_VALID(unit, mem, FULL_KEYf))
    {
        key_field[0] = FULL_KEYf;
        mask_field[0] = FULL_MASKf;
    }
    else
    {
        key_field[0] = KEYf;
        mask_field[0] = MASKf;
    }
    field_count = 1;

    for (i = 0; i < field_count; i++)
    {
        bit_length[i] = soc_mem_field_length(unit, mem, key_field[i]);
    }

    data_words = soc_mem_entry_words(unit, mem);
    data_bytes = data_words * sizeof(uint32_t);

    for (index = 0; index < count; index++)
    {
        if (dm_entry != xy_entry)
        {
            memcpy(dm_entry, xy_entry, data_bytes);
        }
        for (i = 0; i < field_count; i++)
        {
            soc_mem_field_get(unit, mem, xy_entry, key_field[i], key);
            soc_mem_field_get(unit, mem, xy_entry, mask_field[i], mask);

            _soc_mem_tcam_xy_to_dm_decode(unit, key, mask, bit_length[i], xy_lpt);

            if (xy_lpt)
            {
                soc_mem_field_set(unit, mem, dm_entry, key_field[i], key);
            }

            soc_mem_field_set(unit, mem, dm_entry, mask_field[i], mask);
        }

        xy_entry += data_words;
        dm_entry += data_words;
    }
}

/*
 * TCAM access, encode/decode functions used for debug only.
 */
extern soc_mem_info_t *soc_memories[];

void _soc_mem_tcam_xy_to_dm_slim(int unit, soc_mem_t mem, soc_mem_info_t *meminfo, int count, uint32_t *xy_entry,
                                 uint32_t *dm_entry)
{
    uint32_t key[SOC_MAX_MEM_FIELD_WORDS], mask[SOC_MAX_MEM_FIELD_WORDS];
    soc_field_t key_field[4], mask_field[4];
    int bit_length[4], field_count;
    int index, i, data_words, data_bytes;
    int xy_lpt;

    xy_lpt = (soc_feature(unit, soc_feature_xy_tcam_lpt) && meminfo->flags & SOC_MEM_FLAG_TCAM_ENCODING_LPT);

    if (SOC_IS_FIRELIGHT(unit))
    {
        if (mem == ISEC_SC_TCAMm)
        {
            uint32_t isec_ctrl=0;

            soc_ubus_reg32_get(unit, ISEC_SER_CONTROLreg, REG_PORT_ANY, &isec_ctrl);
            xy_lpt = soc_ubus_reg_field_get(unit, ISEC_SER_CONTROLreg, isec_ctrl, SC_TCAM_LPT_fld);
        }
    }

    key_field[0] = KEYf;
    mask_field[0] = MASKf;
    field_count = 1;

    for (i = 0; i < field_count; i++)
    {
        bit_length[i] = soc_mem_field_length_slim(unit, meminfo, key_field[i]);
    }

    data_bytes = meminfo->bytes;
    data_words = data_bytes / sizeof(uint32_t);

    for (index = 0; index < count; index++)
    {
        if (dm_entry != xy_entry)
        {
            memcpy(dm_entry, xy_entry, data_bytes);
        }
        for (i = 0; i < field_count; i++)
        {
            soc_meminfo_field_get(mem, meminfo, xy_entry, key_field[i], key, data_words);
            soc_meminfo_field_get(mem, meminfo, xy_entry, mask_field[i], mask, data_words);

            _soc_mem_tcam_xy_to_dm_decode(unit, key, mask, bit_length[i], xy_lpt);

            if (xy_lpt)
            {
                soc_meminfo_field_set(mem, meminfo, dm_entry, key_field[i], key);
            }

            soc_meminfo_field_set(mem, meminfo, dm_entry, mask_field[i], mask);
        }

        xy_entry += data_words;
        dm_entry += data_words;
    }
}

/*
 * TCAM access, encode/decode functions used for debug only.
 */
int soc_mem_read_slim(int unit, soc_mem_t mem, int mem_entry, void *entry_data)
{
    int rv = 0;
    uint32_t entry_data_encoded[SOC_MAX_MEM_WORDS];
    uint32_t sp_key_entry_data_encoded[SOC_MAX_MEM_WORDS * 4];
    uint32_t *sp_mask_entry_data_encoded;
    uint32_t *sp_key_entry_data;
    uint32_t *sp_mask_entry_data;

    soc_mem_info_t *meminfo = soc_memories[mem];
    soc_mem_info_t *meminfo_coupled;

    if (meminfo->flags & SOC_MEM_FLAG_CAM)
    {
        /* For SPTCAM the buffer arrives pre encoded. Just write it to the memory. */
        if ((mem == ISEC_SP_TCAM_KEYm) || (mem == ISEC_SP_TCAM_MASKm))
        {
            sp_mask_entry_data_encoded = sp_key_entry_data_encoded + SOC_MAX_MEM_WORDS;
            sp_key_entry_data = sp_mask_entry_data_encoded + SOC_MAX_MEM_WORDS;
            sp_mask_entry_data = sp_key_entry_data + SOC_MAX_MEM_WORDS;

            if (mem == ISEC_SP_TCAM_KEYm)
            {
                meminfo_coupled = soc_memories[ISEC_SP_TCAM_MASKm];
                soc_mem_read_entry(unit, meminfo, 0, mem_entry, sp_key_entry_data_encoded);
                soc_mem_read_entry(unit, meminfo_coupled, 0, mem_entry, sp_mask_entry_data_encoded);

                _soc_mem_tcam_xy_to_dm_coupled_slim(unit,
                                                ISEC_SP_TCAM_KEYm, ISEC_SP_TCAM_MASKm,
                                                meminfo, meminfo_coupled,
                                                sp_key_entry_data_encoded, sp_mask_entry_data_encoded,
                                                sp_key_entry_data, sp_mask_entry_data, 1);
            }
            else
            {
                meminfo_coupled = soc_memories[ISEC_SP_TCAM_KEYm];
                soc_mem_read_entry(unit, meminfo, 0, mem_entry, sp_mask_entry_data_encoded);
                soc_mem_read_entry(unit, meminfo_coupled, 0, mem_entry, sp_key_entry_data_encoded);
                
                _soc_mem_tcam_xy_to_dm_coupled_slim(unit, ISEC_SP_TCAM_KEYm, ISEC_SP_TCAM_MASKm, meminfo_coupled,
                                                    meminfo, sp_key_entry_data_encoded, sp_mask_entry_data_encoded,
                                                    sp_key_entry_data, sp_mask_entry_data, 1);
            }

            if (mem == ISEC_SP_TCAM_KEYm)
                memcpy(entry_data, sp_key_entry_data, meminfo->bytes);
            else
                memcpy(entry_data, sp_mask_entry_data, meminfo->bytes);
        }
        /* For SCTCAM the buffer arrives unencoded. We need to format it first. */
        else
        {
            soc_mem_read_entry(unit, meminfo, 0, mem_entry, entry_data_encoded);
            _soc_mem_tcam_xy_to_dm_slim(unit, mem, meminfo, 1, entry_data_encoded, (void *)entry_data);
        }
    }
    else
    {
        soc_mem_read_entry(unit, meminfo, 0, mem_entry, entry_data);
    }

    return rv;
}

/*
 * Function:
 *    soc_mem_read_entry
 * Purpose:
 *    Read a single index entry from the memory internal to the SOC.
 * Notes:
 *    GBP/CBP memory should only accessed when MMU is in DEBUG mode.
 */
int soc_mem_read_entry(int unit, soc_mem_info_t *meminfo, int copyno, int index, void *entry_data)
{
    int rv = BCM_E_NONE;
    int i;
    int beats;
    int mem_timeout;
    uint32_t start_busy_mask = (1 << 31);
    uint32_t acc_type_mask = (1 << 28);
    uint32_t read_write_mask = (1 << 27);
    uint32_t dlen_mask = (0x7f << 20);
    uint32_t mem_sel_mask = (0x7 << 17);
    uint32_t mem_id_mask = (0x1f << 12);
    uint32_t index_mask = 0xfff;
    uint32_t indirect_command = 0;
    uint32_t command_reg_read;
    uint32_t timeout_status;
    uint32_t indirect_read_data;
    soc_ubus_reg_s rbus_command0_reg;
    soc_ubus_reg_s indirect_read_data_reg0;
    soc_ubus_reg_s rbus_timeout_reg;
    soc_ubus_reg_s rbus_timeout_status_reg;

    rbus_command0_reg.offset = BCHP_ETH_R2SBUS_BRIDGE_INDIRECT0_COMMAND;
    rbus_timeout_reg.offset = BCHP_ETH_R2SBUS_BRIDGE_TIMEOUT;
    rbus_timeout_status_reg.offset = BCHP_ETH_R2SBUS_BRIDGE_TIMEOUT_STATUS;
    indirect_read_data_reg0.offset = BCHP_ETH_R2SBUS_BRIDGE_INDIRECT0_READ_DATA0;

    beats = (meminfo->bytes / 4);
    beats += (meminfo->bytes % 4)? 1 : 0;

    indirect_command = ((index << INDEX_SHIFT) & index_mask);
    indirect_command |= ((meminfo->mem_id << MEM_ID_SHIFT) & mem_id_mask);
    indirect_command |= ((meminfo->mem_sel << MEM_SEL_SHIFT) & mem_sel_mask);
    indirect_command |= ((meminfo->bytes << DLEN_SHIFT) & dlen_mask);
    indirect_command |= ((1 << READ_WRITE_SHIFT) & read_write_mask);
    indirect_command |= ((1 << ACC_TYPE_SHIFT) & acc_type_mask);
    indirect_command |= ((1 << START_BUSY_SHIFT) & start_busy_mask);

#if 0
    printf("soc_mem_read ... mem = %d, mem list index = %d\n", meminfo->mem_id, index);
    printf("soc_mem_read ... mem = %d, mem list index = %d\n", meminfo->mem_id, index);
    printf("MEM INFO: \n");
    printf("    Base = %08x\n", meminfo->base);
    printf("    Width = %d\n", meminfo->data_bits);
    printf("    Index Max = %d\n", meminfo->index_max);
    printf("    Num Fields = %d\n", meminfo->nFields);
    printf("    beats = %d\n", beats);
    printf("    mem_sel = %d\n", meminfo->mem_sel);
    printf("    mem_id = %d\n", meminfo->mem_id);
    printf("    bytes = %d\n", meminfo->bytes);
#endif

    /* Initiate the hardware memory access state machine */
    soc_ubus_reg32_set(unit, &rbus_timeout_reg, REG_PORT_ANY, 0x0000ffff);
    soc_ubus_reg32_set(unit, &rbus_command0_reg, REG_PORT_ANY, indirect_command);

    /* Wait for the hardware state machine to complete the internal access */
    mem_timeout = 0;
    while (1)
    {
        soc_ubus_reg32_get(unit, &rbus_timeout_status_reg, REG_PORT_ANY, &timeout_status);
        soc_ubus_reg32_get(unit, &rbus_command0_reg, REG_PORT_ANY, &command_reg_read);

        if (!(command_reg_read & start_busy_mask))
            break;
        else
        {
            if (mem_timeout < SOC_MEM_TIMEOUT)
            {
                mem_timeout++;
                udelay(1000);
            }
            else
            {
                rv = BCM_E_TIMEOUT;
                break;
            }
        }
    }

    /* Fill up the data return buffer */
    if (rv == BCM_E_NONE)
    {
        for (i = 0; i < beats; i++)
        {
            soc_ubus_reg32_get(unit, &indirect_read_data_reg0, REG_PORT_ANY, &indirect_read_data);
            ((uint32_t *)entry_data)[i] = indirect_read_data;
            indirect_read_data_reg0.offset += 4;
        }
    }

    return rv;
}

/*
 * Function:
 *    soc_mem_read
 * Purpose:
 *    Generic entry point for all memory read types. Different memories
 *    require different read mechanisms. This function will dispatch to the
 *    various encode/decode functions.
 * Notes:
 *    GBP/CBP memory should only accessed when MMU is in DEBUG mode.
 */
int soc_mem_read(int unit, soc_mem_t mem, int copyno, int index, void *entry_data)
{
    int rv = 0;
    uint32_t entry_data_encoded[SOC_MAX_MEM_WORDS];
    uint32_t sp_key_entry_data_encoded[SOC_MAX_MEM_WORDS * 4];
    uint32_t *sp_mask_entry_data_encoded;
    uint32_t *sp_key_entry_data;
    uint32_t *sp_mask_entry_data;

    soc_mem_info_t *meminfo = SOC_DRIVER(unit)->mem_info[mem];
    soc_mem_info_t *meminfo_coupled;

    if (meminfo->flags & SOC_MEM_FLAG_CAM)
    {
        /* For SPTCAM the buffer arrives pre encoded. Just write it to the memory. */
        if ((mem == ISEC_SP_TCAM_KEYm) || (mem == ISEC_SP_TCAM_MASKm))
        {
            sp_mask_entry_data_encoded = (sp_key_entry_data_encoded + SOC_MAX_MEM_WORDS);
            sp_key_entry_data = (sp_mask_entry_data_encoded + SOC_MAX_MEM_WORDS);
            sp_mask_entry_data = (sp_key_entry_data + SOC_MAX_MEM_WORDS);

            if (mem == ISEC_SP_TCAM_KEYm)
            {
                meminfo_coupled = SOC_DRIVER(unit)->mem_info[ISEC_SP_TCAM_MASKm];

                soc_mem_read_entry(unit, meminfo, 0, index, sp_key_entry_data_encoded);
                soc_mem_read_entry(unit, meminfo_coupled, 0, index, sp_mask_entry_data_encoded);

                _soc_mem_tcam_xy_to_dm_coupled(unit, ISEC_SP_TCAM_KEYm, ISEC_SP_TCAM_MASKm, sp_key_entry_data_encoded, sp_mask_entry_data_encoded,
                                               sp_key_entry_data, sp_mask_entry_data, 1);
            }
            else
            {
                meminfo_coupled = SOC_DRIVER(unit)->mem_info[ISEC_SP_TCAM_KEYm];

                soc_mem_read_entry(unit, meminfo, 0, index, sp_mask_entry_data_encoded);
                soc_mem_read_entry(unit, meminfo_coupled, 0, index, sp_key_entry_data_encoded);

                _soc_mem_tcam_xy_to_dm_coupled(unit, ISEC_SP_TCAM_KEYm, ISEC_SP_TCAM_MASKm, sp_key_entry_data_encoded, sp_mask_entry_data_encoded,
                                               sp_key_entry_data, sp_mask_entry_data, 1);
            }

            if(mem == ISEC_SP_TCAM_KEYm)
                memcpy(entry_data, sp_key_entry_data, meminfo->bytes);
            else
                memcpy(entry_data, sp_mask_entry_data, meminfo->bytes);
        }
        /* For SCTCAM the buffer arrives unencoded. We need to format it first. */
        else
        {
            soc_mem_read_entry(unit, meminfo, 0, index, entry_data_encoded);
            _soc_mem_tcam_xy_to_dm(unit, mem, 1, entry_data_encoded, (void *)entry_data);
        }
    }
    else
    {
        soc_mem_read_entry(unit, meminfo, 0, index, entry_data);
    }

    return rv;
}

/*
 * Function:
 *    soc_mem_read_range
 * Purpose:
 *    Read a range of memory entries from the SOC
 */
int soc_mem_read_range(int unit, soc_mem_t mem, int copyno, int index_min, int index_max, void *entry_array)
{
    int i, offset = 0;
    for (i = index_min; i <= index_max; i++)
    {
        soc_mem_read(unit, mem, copyno, i, (void *)((uint32_t *)entry_array + offset));
        offset += soc_mem_entry_words(unit, mem);
    }

    return 0;
}


/*
 * Function:
 *    soc_mem_write
 * Purpose:
 *    Write a memory internal to the SOC.
 * Notes:
 *    GBP/CBP memory should only accessed when MMU is in DEBUG mode.
 */
int soc_mem_write(int unit, soc_mem_t mem, int copyno, int index_in, void *entry_data)
{
    int rv = BCM_E_NONE;
    int i;
    int index;
    int offset = 0;
    int beats;
    int mem_timeout;
    uint32_t start_busy_mask = (1 << 31);
    uint32_t acc_type_mask = (1 << 28);
//    uint32_t read_write_mask = (1 << 27);
    uint32_t dlen_mask = (0x7f << 20);
    uint32_t mem_sel_mask = (0x7 << 17);
    uint32_t mem_id_mask = (0x1f << 12);
    uint32_t index_mask = 0xfff;
    uint32_t indirect_command = 0;
    uint32_t command_reg_read;
    uint32_t timeout_status;
    uint32_t indirect_write_data;
    soc_mem_info_t *meminfo = &SOC_MEM_INFO(unit, mem);
    soc_ubus_reg_s rbus_command0_reg;
    soc_ubus_reg_s indirect_write_data_reg0;
    soc_ubus_reg_s rbus_timeout_reg;
    soc_ubus_reg_s rbus_timeout_status_reg;

    uint32_t converted_entry_data[SOC_MAX_MEM_WORDS];
    uint32_t cache_entry_data[SOC_MAX_MEM_WORDS];

    void *entry_data_ptr = entry_data;

    rbus_command0_reg.offset = BCHP_ETH_R2SBUS_BRIDGE_INDIRECT0_COMMAND;
    rbus_timeout_reg.offset = BCHP_ETH_R2SBUS_BRIDGE_TIMEOUT;
    rbus_timeout_status_reg.offset = BCHP_ETH_R2SBUS_BRIDGE_TIMEOUT_STATUS;
    indirect_write_data_reg0.offset = BCHP_ETH_R2SBUS_BRIDGE_INDIRECT0_WRITE_DATA0;

    /* Encode data for TCAM memories - if required */
    if (meminfo->flags & SOC_MEM_FLAG_CAM)
    {
        /* For SPTCAM the buffer arrives pre encoded. Just write it to the memory. */
        if ((mem == ISEC_SP_TCAM_KEYm) || (mem == ISEC_SP_TCAM_MASKm))
        {
#if 0            
            printf("SPTCAM WRITE!! entry_data(preswap) = ");
            for(i = 0; i < 60; i++)
            {
                printf("%02x:", ((unsigned char *)entry_data_ptr)[i]);
            }
            printf("\n");
#endif
            entry_data_ptr = entry_data;
        }
        /* For SCTCAM the buffer arrives unencoded. We need to format it first. */
        else
        {
            entry_data_ptr = entry_data;
            entry_data_ptr = converted_entry_data;
            _soc_mem_tcam_dm_to_xy(unit, mem, 1, entry_data, entry_data_ptr, cache_entry_data);
        }
    }

    beats = ((*SOC_DRIVER(unit)->mem_info[mem]).bytes / 4);
    beats += ((*SOC_DRIVER(unit)->mem_info[mem]).bytes % 4)? 1 : 0;
    for (i = 0; i < beats; i++)
    {
        indirect_write_data = ((uint32_t *)entry_data_ptr)[i];
        soc_ubus_reg32_set(unit, &indirect_write_data_reg0, REG_PORT_ANY, indirect_write_data);
        indirect_write_data_reg0.offset += 4;
    }

    index = index_in;
    offset = 0;

    indirect_command = ((index << INDEX_SHIFT) & index_mask);
    indirect_command |= (((*SOC_DRIVER(unit)->mem_info[mem]).mem_id << MEM_ID_SHIFT) & mem_id_mask);
    indirect_command |= (((*SOC_DRIVER(unit)->mem_info[mem]).mem_sel << MEM_SEL_SHIFT) & mem_sel_mask);
    indirect_command |= (((*SOC_DRIVER(unit)->mem_info[mem]).bytes << DLEN_SHIFT) & dlen_mask);
    /* indirect_command |= ((1 << READ_WRITE_SHIFT) & read_write_mask); */
    indirect_command |= ((1 << ACC_TYPE_SHIFT) & acc_type_mask);
    indirect_command |= ((1 << START_BUSY_SHIFT) & start_busy_mask);

#if 0
    printf("soc_mem_write ... mem = %d, mem list index = %d\n", mem, index);
    printf("MEM INFO: \n");
    printf("    Base = %08x\n", (*SOC_DRIVER(unit)->mem_info[mem]).base);
    printf("    Width = %d\n", (*SOC_DRIVER(unit)->mem_info[mem]).data_bits);
    printf("    Index Max = %d\n", (*SOC_DRIVER(unit)->mem_info[mem]).index_max);
    printf("    Num Fields = %d\n", (*SOC_DRIVER(unit)->mem_info[mem]).nFields);
    printf("    beats = %d\n", beats);
    printf("    mem_sel = %d\n", (*SOC_DRIVER(unit)->mem_info[mem]).mem_sel);
    printf("    mem_id = %d\n", (*SOC_DRIVER(unit)->mem_info[mem]).mem_id);
    printf("    bytes = %d\n", (*SOC_DRIVER(unit)->mem_info[mem]).bytes);
    printf("    indirect_command = %08x\n", indirect_command);
    printf("    target index = %d\n", index);
    for(i = 0; i < beats; i++)
        printf("Entry Data[i]: %08x\n",((uint32_t *)entry_data)[i]);
#endif

    /* Initiate the hardware memory access state machine */
    soc_ubus_reg32_set(unit, &rbus_timeout_reg, REG_PORT_ANY, 0x0000ffff);
    soc_ubus_reg32_set(unit, &rbus_command0_reg, REG_PORT_ANY, indirect_command);
    
    /* Wait for the hardware state machine to complete the internal access */
    mem_timeout = 0;
    while (1)
    {
        soc_ubus_reg32_get(unit, &rbus_timeout_status_reg, REG_PORT_ANY, &timeout_status);
        soc_ubus_reg32_get(unit, &rbus_command0_reg, REG_PORT_ANY, &command_reg_read);

        if (!(command_reg_read & start_busy_mask))
            break;
        else
        {
            if (mem_timeout < SOC_MEM_TIMEOUT)
            {
                mem_timeout++;
                udelay(1000);
            }
            else
            {
                rv = BCM_E_TIMEOUT;
                break;
            }
        }
    }

    return rv;
}
