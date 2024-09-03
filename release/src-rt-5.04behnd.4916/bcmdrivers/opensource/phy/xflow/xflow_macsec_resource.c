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

#include "soc/mcm/enum_types.h"
#include "soc/mem.h"
#include "soc/memory.h"
#include "soc/field.h"
#include <soc/mcm/allenum.h>
#include "soc/feature.h"
#include "xflow_macsec_defs.h"
#include "xflow_macsec.h"
#include "xflow_macsec_common.h"
#include "xflow_macsec_firelight.h"
#include "xflow_macsec_cfg_params.h"
#include "macsec_dev.h"
#include "macsec_types.h"
#include "macsec_macros.h"

extern soc_control_t *soc_control[SOC_MAX_NUM_DEVICES];

extern soc_mem_info_t *soc_memories_bcm56070_a0[];

static SHR_BITDCL sc_encrypt_bmp[BCM_MAX_NUM_UNITS][_SHR_BITDCLSIZE(_XFLOW_MACSEC_SC_ENCRYPT_MAX)];
static SHR_BITDCL policy_bmp[BCM_MAX_NUM_UNITS][_SHR_BITDCLSIZE(_XFLOW_MACSEC_POLICY_MAX)];

uint8* sc_table_dma;
uint8* sa_table_dma;

/*
 * Datastructures for TCAM management.
 */

/* sc_tcam will be allocated as an array of 1024 (can change based on the device).*/
xflow_macsec_tcam_map_t sc_tcam_map[BCM_MAX_NUM_UNITS+1];

/* sp_tcam will be allocated as an array of 1024 entries (can change based on device). */
xflow_macsec_tcam_map_t sp_tcam_map[BCM_MAX_NUM_UNITS+1];

/*    Reverse get    */

int _xflow_macsec_sc_encrypt_index_logical_index_get(int unit, int hw_index, int *index)
{
    if (index == NULL)
        return BCM_E_PARAM;

    if (hw_index >= _XFLOW_MACSEC_SC_ENCRYPT_MAX)
        return BCM_E_PARAM;

    if (!SHR_BITGET(sc_encrypt_bmp[unit], hw_index))
        return BCM_E_NOT_FOUND;

    *index = hw_index;

    return BCM_E_NONE;
}

int _xflow_macsec_sc_decrypt_index_logical_index_get(int unit, int sc_hw_index, int *sc_logical_index)
{
    if (sc_hw_index >= _XFLOW_MACSEC_SC_DECRYPT_MAX)
        return BCM_E_PARAM;

    if (sc_tcam_map[unit].tcam_ptr[sc_hw_index].in_use != 1)
        return BCM_E_NOT_FOUND;

    if (sc_logical_index)
        *sc_logical_index = sc_tcam_map[unit].tcam_ptr[sc_hw_index].tcam_logical_map_id;

    return BCM_E_NONE;
}

int _xflow_macsec_sc_encrypt_index_reserve(int unit, int prio, int *index, uint8 flag)
{
    int max_index;

    if (index == NULL)
        return BCM_E_PARAM;

    max_index = _XFLOW_MACSEC_SC_ENCRYPT_MAX;

    if (XFLOW_MACSEC_4_SA_PER_SC(unit))
    {
        max_index = max_index / 4;
    }

    if (flag == FALSE)
    {
        _SHR_BIT_FIND_FREE(sc_encrypt_bmp[unit], max_index, *index);

        if (*index >= max_index)
            return BCM_E_FULL;
    }
    else
    {
        if (*index >= max_index)
            return BCM_E_PARAM;

        if (SHR_BITGET(sc_encrypt_bmp[unit], *index))
            return BCM_E_EXISTS;
    }

    SHR_BITSET(sc_encrypt_bmp[unit], *index);

    return BCM_E_NONE;
}

int _xflow_macsec_tcam_logical_map_find_free_index(int unit,
                                               xflow_macsec_tcam_map_t tcam_map,
                                               int *free_index)
{
    int i;

    if (!free_index)
        return BCM_E_PARAM;

    for (i = 0; i < tcam_map.tcam_index_count; i++)
    {
        if (tcam_map.tcam_logical_map[i].in_use == 0)
        {
            *free_index = i;
            return BCM_E_NONE;
        }
    }

    return BCM_E_FULL;
}

int _xflow_macsec_find_tcam_free_direction(xflow_macsec_tcam_map_t tcam_map,
                                                int source_index, _xflow_macsec_tcam_shift_t *dir)
{
    int lower_index, higher_index, i;

    if (dir == NULL)
        return BCM_E_PARAM;

    if (!tcam_map.tcam_ptr[source_index].in_use)
        return BCM_E_NONE;

    /* Find which direction would result in less number
     * of moves.
     */
    lower_index = -1;
    for (i = source_index; i >= 0; i--)
    {
        if (tcam_map.tcam_ptr[i].in_use == 0)
        {
            lower_index = i;
            break;
        }
    }

    higher_index = -1;
    for (i = source_index; i <= (tcam_map.tcam_index_count - 1); i++)
    {
        if (tcam_map.tcam_ptr[i].in_use == 0)
        {
            higher_index = i;
            break;
        }
    }

    if ((lower_index == -1) && (higher_index == -1))
    {
        /* No free index. Should not happen here. */
        return BCM_E_INTERNAL;
    }

    /* Source index should be of same priority or higher priority.
     * Hence if shift is from lower index to high, source index
     * should be incremented.
     */
    if (lower_index == -1)
    {
        *dir = _XFLOW_MACSEC_TCAM_SHIFT_UP;
    }
    else if (higher_index == -1)
    {
        *dir = _XFLOW_MACSEC_TCAM_SHIFT_DOWN;
    }
    else if ((higher_index - source_index) <
                (source_index - lower_index))
    {
        *dir = _XFLOW_MACSEC_TCAM_SHIFT_UP;
    }
    else
    {
        *dir = _XFLOW_MACSEC_TCAM_SHIFT_DOWN;
    }

    return BCM_E_NONE;
}

/*
 * The function helps to find a target index location in the tcam_ptr
 * (which is in a sorted sequence). Entries with higher numbered priority
 * are in lower indices. The target_index found may or maynot be in use.
 * All entries with same priority will be together. However, there may be
 * holes (with in_use = 0) in the array.
 * Target index would be of same or last higher priority index.
 */
static int _xflow_macsec_tcam_ptr_find_target_index(xflow_macsec_tcam_map_t tcam_map,
                                         int priority, int *target_index)
{
    int i, start_index, last_in_use;
    _xflow_macsec_tcam_shift_t dir;
    xflow_macsec_tcam_ptr_t *tcam_ptr;

    if (!target_index)
        return BCM_E_PARAM;

    tcam_ptr = tcam_map.tcam_ptr;

    /* start_index is set to the first entry index which has the same
     * priority as the given priority.
     */
    start_index = -1;

    /* last_in_use is necessary in cases where the given priority is the
     * lowest. Example: last index is 10 with priority 200. Input priority
     * is 100. last_in_use will be set to 10 and target index is 11.
     */
    last_in_use = -1;
    for (i = 0; i < tcam_map.tcam_index_count; i++)
    {
        if(tcam_ptr[i].in_use == 0)
        {
            continue;
        }
        last_in_use = i;

        /* When the table contains no entry with given priority and we
         * hit an entry with a lower priority, the target index is
         * calculated based on the direction of the nearest free index.
         */
        if(tcam_ptr[i].priority < priority)
        {
            BCM_IF_ERROR_RETURN
                (_xflow_macsec_find_tcam_free_direction(tcam_map,
                                                        i, &dir));
            if (dir == _XFLOW_MACSEC_TCAM_SHIFT_UP)
            {
                *target_index = i;
            }
            else
            {
                *target_index = i - 1;
            }
            return BCM_E_NONE;
        }

        /* When the table contains entries with same priority, the logic
         * below identifies the last index with the same priority. That
         * index + 1 becomes the target index.
         */
        if((tcam_ptr[i].priority == priority) && (start_index == -1))
        {
            start_index = i;
        }
        if((tcam_ptr[i].priority != priority) && (start_index != -1))
        {
            if(tcam_ptr[i].priority > priority)
            {
                /* The priorities are not in descending order!*/
                return BCM_E_INTERNAL;
            }
            BCM_IF_ERROR_RETURN(_xflow_macsec_find_tcam_free_direction(tcam_map,
                                                        i, &dir));
            if (dir == _XFLOW_MACSEC_TCAM_SHIFT_UP)
            {
                *target_index = i;
            }
            else
            {
                *target_index = i - 1;
            }
            return BCM_E_NONE;
        }
    }
    *target_index = last_in_use + 1;

    return BCM_E_NONE;
}

static int _xflow_macsec_tcam_ptr_move_single_tcam_entry_step1(int unit,
                                        xflow_macsec_tcam_map_t tcam_map,
                                        int target_index, int from_index)
{
    int rv = BCM_E_NONE;
    uint32 *buf = NULL;
    soc_mem_t mem;
    int alloc_size, max_alloc_size, mem_iter;
    int num_sa, j;

    max_alloc_size = 0;

    /* Get the max buffer size required among all
     * TCAMs/tables.
     */
    for (mem_iter = tcam_map.tcam_mem_count - 1; mem_iter >= 0; mem_iter--)
    {
        mem = tcam_map.tcam_mem[mem_iter];
        alloc_size = sizeof(uint32) * soc_mem_entry_words(unit, mem);
        if (alloc_size > max_alloc_size)
        {
            max_alloc_size = alloc_size;
        }
    }

    if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, XFLOW_MACSEC_DECRYPT))
    {
        num_sa = XFLOW_MACSEC_NUM_SA_PER_SC(unit, XFLOW_MACSEC_DECRYPT);
    }
    else if (XFLOW_MACSEC_4_SA_PER_SC(unit))
    {
        num_sa = 4;
    }
    else
    {
        num_sa = 1;
    }

    buf = sal_alloc(max_alloc_size, "tcam_copy_alloc");

    _XFLOW_MACSEC_MEM_NULL_CHECK_SET_RV_GOTO_ERROR(buf, max_alloc_size, rv, move_single_tcam_entry_error);

    for (mem_iter = tcam_map.tcam_mem_count - 1; mem_iter >= 0; mem_iter--)
    {
        mem = tcam_map.tcam_mem[mem_iter];
        sal_memset(buf, 0, max_alloc_size);

        /* SA tables require special handling as each SC index
         * can correspond to 4 SA entries.
         */
        if (mem == ISEC_SA_TABLEm)
        {
            if (((from_index * num_sa) + (num_sa - 1)) >= soc_mem_index_count(unit, mem) ||
                ((target_index * num_sa) + (num_sa - 1)) >= soc_mem_index_count(unit, mem))
            {
                rv =  BCM_E_INTERNAL;
                _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, move_single_tcam_entry_error);
            }
            for (j = 0; j < num_sa; j++)
            {
                rv = soc_mem_read(unit, mem, MEM_BLOCK_ALL, (from_index * num_sa) + j, buf);
                _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, move_single_tcam_entry_error);

                rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL,
                                   (target_index * num_sa) + j, buf);
                _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, move_single_tcam_entry_error);
            }
            continue;
        }

        rv = soc_mem_read(unit, mem, MEM_BLOCK_ALL, from_index, buf);
        _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, move_single_tcam_entry_error);

        rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL, target_index, buf);
        _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, move_single_tcam_entry_error);

    }

    /* Now clear contents in from_index */
    sal_memset(buf, 0, max_alloc_size);
    for (mem_iter = 0; mem_iter < tcam_map.tcam_mem_count; mem_iter++)
    {
        mem = tcam_map.tcam_mem[mem_iter];

        /* SA tables require special handling as each SC index
         * can correspond to num_sa SA entries.
         */
        if (mem == ISEC_SA_TABLEm)
        {
            if (((from_index * num_sa) + (num_sa - 1)) >= soc_mem_index_count(unit, mem))
            {
                rv =  BCM_E_INTERNAL;
                _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, move_single_tcam_entry_error);
            }
            for (j = 0; j < num_sa; j++)
            {
                rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL, (from_index * num_sa) + j, buf);
                _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, move_single_tcam_entry_error);
            }
            continue;
        }

        rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL, from_index, buf);
        _XFLOW_MACSEC_BCM_FAILURE_GOTO_ERROR(rv, move_single_tcam_entry_error);
    }

move_single_tcam_entry_error:

    if (buf)
        sal_free(buf);

    return rv;
}


/* This routine moves an entry in the TCAM list from the given from_index
 * to the given target_index. The related tables/memories are shifted first
 * and the TCAM entry itself is shifted last. After the TCAM entry move, the
 * old entry is disabled to allow seamless traffic flow.
 * from_index - The index from which table data must be moved
 * target_index - The index to which the table data must be moved
 */
static int _xflow_macsec_tcam_ptr_move_single_tcam_entry(int unit,
                                        xflow_macsec_tcam_map_t tcam_map,
                                        int target_index, int from_index)
{
    int rv = BCM_E_NONE;
    int tcam_logical_index;

    if (tcam_map.tcam_ptr[target_index].in_use == 1)
        return BCM_E_FULL;

    if (from_index == target_index)
        return BCM_E_INTERNAL;

    if (((tcam_map.tcam_mem[0] == ISEC_SP_TCAM_KEYm) || (tcam_map.tcam_mem[0] == ISEC_SP_TCAM_MASKm)))
    {
        /*
         * Move SP TCAM key and mask entries using a special function.
         */
        rv = xflow_macsec_fl_sp_tcam_move_single_entry(unit, target_index, from_index);
    }
    else
    {
        rv = _xflow_macsec_tcam_ptr_move_single_tcam_entry_step1(unit, tcam_map,
                                            target_index, from_index);
    }

    if (BCM_FAILURE(rv))
        return rv;

#if 0 //NOT_YET
    rv = _xflow_macsec_tcam_ptr_counter_move_single_tcam_entry(unit,
                              target_index, from_index, tcam_map.tcam_mem,
                              tcam_map.tcam_mem_count);
    if (BCM_FAILURE(rv))
    {
        return rv;
    }
#endif

    /* Update the Tcam_Map entries for bookkeeping. */
    tcam_map.tcam_ptr[target_index] = tcam_map.tcam_ptr[from_index];
    tcam_logical_index = tcam_map.tcam_ptr[target_index].tcam_logical_map_id;
    tcam_map.tcam_logical_map[tcam_logical_index].hw_index = target_index;
    tcam_map.tcam_ptr[from_index].in_use = 0;
    tcam_map.tcam_ptr[from_index].priority = 0;
    tcam_map.tcam_ptr[from_index].tcam_logical_map_id = 0;

    return rv;
}

/*
 * This routine shifts the entries in the TCAM from the given from_index
 * to the given to_index.
 * target_index: This is the tcam_index to which entries from
 *               (from_index) to (target_index +/- 1) must be moved to.
 * from_index: This index is the start index for the shift.
 */
static int _xflow_macsec_tcam_ptr_move_tcam_entries(int unit,
                                         xflow_macsec_tcam_map_t tcam_map,
                                         int target_index, int from_index)
{
    int t, f;
    int iter_valid;

    if (tcam_map.tcam_ptr[target_index].in_use != 0)
        return BCM_E_FULL;

    if (from_index == target_index)
        return BCM_E_INTERNAL;

    iter_valid = 1;
    for (t = target_index; iter_valid; ((target_index < from_index) ? t++ : t--))
    {
        if (target_index < from_index)
        {
            if (t >= from_index)
            {
                iter_valid = 0;
                break;
            }
            f = t + 1;
        }
        else
        {
            if (t <= from_index)
            {
                iter_valid = 0;
                break;
            }
            f = t - 1;
        }
        BCM_IF_ERROR_RETURN(_xflow_macsec_tcam_ptr_move_single_tcam_entry(unit, tcam_map, t, f));

    }

    return BCM_E_NONE;
}

/*    Get    */

int _xflow_macsec_sc_encrypt_index_hw_index_get(int unit, int index, int *hw_index, int *prio)
{
    if (hw_index == NULL)
        return BCM_E_PARAM;
   
    if (index >= _XFLOW_MACSEC_SC_ENCRYPT_MAX)
        return BCM_E_PARAM;

    if (!SHR_BITGET(sc_encrypt_bmp[unit], index))
        return BCM_E_NOT_FOUND;

    *hw_index = index;

    return BCM_E_NONE;
}


int _xflow_macsec_sc_decrypt_index_hw_index_get(int unit, int index, int *hw_index, int *prio)
{
    if (hw_index == NULL)
        return BCM_E_PARAM;

    if (index >= sc_tcam_map[unit].tcam_index_count)
        return BCM_E_PARAM;

    if (sc_tcam_map[unit].tcam_logical_map[index].in_use != 1)
        return BCM_E_NOT_FOUND;

    *hw_index = sc_tcam_map[unit].tcam_logical_map[index].hw_index;

    if (prio)
        *prio = sc_tcam_map[unit].tcam_logical_map[index].priority;

    return BCM_E_NONE;
}

int _xflow_macsec_flow_index_hw_index_get(int unit, int index, int *hw_index, int *prio)
{
    if (hw_index == NULL)
        return BCM_E_PARAM;

    if (index >= sp_tcam_map[unit].tcam_index_count)
        return BCM_E_PARAM;

    if (sp_tcam_map[unit].tcam_logical_map[index].in_use != 1)
        return BCM_E_NOT_FOUND;

    *hw_index = sp_tcam_map[unit].tcam_logical_map[index].hw_index;

    if (prio)
        *prio = sp_tcam_map[unit].tcam_logical_map[index].priority;

    return BCM_E_NONE;
}

/*
 * Free the given logical index. Also free the corresponding
 * TCAM ptr entry.
 * Note: The HW entry associated with the given logical index must have
 *       been unset approriately by the caller. This function
 *       clears only the SW data structure.
 */
int _xflow_macsec_tcam_map_index_free(int unit,
                                             xflow_macsec_tcam_map_t *tcam_map,
                                             int logical_index)
{
    int tcam_index;

    if (logical_index >= tcam_map->tcam_index_count)
        return BCM_E_PARAM;

    if (tcam_map->tcam_logical_map[logical_index].in_use == 0)
        return BCM_E_PARAM;

    tcam_index = tcam_map->tcam_logical_map[logical_index].hw_index;

    if (tcam_map->tcam_ptr[tcam_index].in_use == 0)
        return BCM_E_PARAM;

    tcam_map->tcam_ptr[tcam_index].priority = 0;
    tcam_map->tcam_ptr[tcam_index].tcam_logical_map_id = 0;
    tcam_map->tcam_ptr[tcam_index].in_use = 0;
    tcam_map->tcam_logical_map[logical_index].priority = 0;
    tcam_map->tcam_logical_map[logical_index].hw_index = 0;
    tcam_map->tcam_logical_map[logical_index].in_use = 0;
    tcam_map->free_count++;
    return BCM_E_NONE;
}

int _xflow_macsec_sc_decrypt_index_free(int unit, int index)
{
    return(_xflow_macsec_tcam_map_index_free(unit, &sc_tcam_map[unit], index));
}

int _xflow_macsec_flow_index_free(int unit, int index)
{
    return(_xflow_macsec_tcam_map_index_free(unit, &sp_tcam_map[unit], index));
}

/*
 * Entry function to get a resource.
 */
int xflow_macsec_index_get(int unit, int dir,
                        xflow_macsec_index_type_t type,
                        int index, int *hw_index)
{
    int rv = BCM_E_NONE;

    if ((dir != XFLOW_MACSEC_ENCRYPT) && (dir != XFLOW_MACSEC_DECRYPT))
        return BCM_E_PARAM;

    if (type >= xflowMacsecIdTypeCount)
        return BCM_E_PARAM;

    if(dir == XFLOW_MACSEC_DECRYPT)
    {
        if(type == xflowMacsecIdTypeSecureChan)
            rv = _xflow_macsec_sc_decrypt_index_hw_index_get(unit, index, hw_index, NULL);
        else if(type == xflowMacsecIdTypePolicy)
            rv = _xflow_macsec_policy_index_hw_index_get(unit, index, hw_index, NULL);
        else if(type == xflowMacsecIdTypeFlow)
            rv = _xflow_macsec_flow_index_hw_index_get(unit, index, hw_index, NULL);
        else
            return BCM_E_PARAM;

    }
    else if(dir == XFLOW_MACSEC_ENCRYPT)
    {
        if(type == xflowMacsecIdTypeSecureChan)
            rv = _xflow_macsec_sc_encrypt_index_hw_index_get(unit, index, hw_index, NULL);
        else
            return BCM_E_PARAM;
    }
    else
        return BCM_E_PARAM;

    return rv;
}

/*
 * Entry function to free a resource.
 */
int xflow_macsec_index_free(int unit, int dir,
                         xflow_macsec_index_type_t type,
                         int index)
{
    int rv = BCM_E_NONE;

    if ((dir != XFLOW_MACSEC_ENCRYPT) && (dir != XFLOW_MACSEC_DECRYPT))
        return BCM_E_PARAM;

    if (type >= xflowMacsecIdTypeCount)
        return BCM_E_PARAM;

    if(dir == XFLOW_MACSEC_DECRYPT)
    {
        if(type == xflowMacsecIdTypeSecureChan)
            rv = _xflow_macsec_sc_decrypt_index_free(unit, index);
        else if(type == xflowMacsecIdTypePolicy)
            rv = _xflow_macsec_policy_index_free(unit, index);
        else if(type == xflowMacsecIdTypeFlow)
            rv = _xflow_macsec_flow_index_free(unit, index);
        else
            return BCM_E_PARAM;

    }
    else if(dir == XFLOW_MACSEC_ENCRYPT)
    {
        if(type == xflowMacsecIdTypeSecureChan)
            rv = _xflow_macsec_sc_encrypt_index_free(unit, index);
        else
            return BCM_E_PARAM;
    }
    else
        return BCM_E_PARAM;

    return rv;
}

int _xflow_macsec_tcam_ptr_move_and_free(int unit, xflow_macsec_tcam_map_t tcam_map,
                                                int source_index)
{
    int i, rv;
    int lower_index, higher_index, target_index;

    if (!tcam_map.tcam_ptr[source_index].in_use)
        return BCM_E_NONE;

    /* Find which direction would result in less number
     * of moves.
     */
    lower_index = -1;
    for (i = source_index; i >= 0; i--)
    {
        if (tcam_map.tcam_ptr[i].in_use == 0)
        {
            lower_index = i;
            break;
        }
    }

    higher_index = -1;
    for (i = source_index; i <= (tcam_map.tcam_index_count - 1); i++)
    {
        if (tcam_map.tcam_ptr[i].in_use == 0)
        {
            higher_index = i;
            break;
        }
    }

    target_index = -1;
    if ((lower_index == -1) && (higher_index == -1))
    {
        /* No free index. Should not happen here. */
        return BCM_E_INTERNAL;
    }

    if (lower_index == -1)
    {
        target_index = higher_index;
    }
    else if (higher_index == -1)
    {
        target_index = lower_index;
    }
    else if ((higher_index - source_index) < (source_index - lower_index))
    {
        target_index = higher_index;
    }
    else
    {
        target_index = lower_index;
    }

    rv = _xflow_macsec_tcam_ptr_move_tcam_entries(unit, tcam_map, target_index, source_index);
    if (BCM_FAILURE(rv))
        return rv;

    return BCM_E_NONE;
}

/*
 * Find a free logical index and reserve it. Also search for an appropriate
 * HW index based on priority and allocate.
 * Note: This function allocates the SW data structure and returns the
 *       allocated index. This does not program the TCAM entry at that index.
 *       The caller must program the HW entries appropriately.
 */
int _xflow_macsec_tcam_map_index_reserve(int unit, xflow_macsec_tcam_map_t *tcam_map,
                                                int prio, int *index, uint8 flag)
{
    int logical_index, tcam_index;
    int rv = BCM_E_NONE;
    
    if ((index == NULL) || (tcam_map == NULL))
        return BCM_E_PARAM;

    if (tcam_map->free_count == 0)
        return BCM_E_FULL;

    if (flag == FALSE)
    {
        rv = _xflow_macsec_tcam_logical_map_find_free_index(unit, *tcam_map, &logical_index);
        if (BCM_FAILURE(rv))
            return rv;
    }
    else
    {
        logical_index = *index;
        if (logical_index >= tcam_map->tcam_index_count)
            return BCM_E_PARAM;

        if (tcam_map->tcam_logical_map[logical_index].in_use == 1)
            return BCM_E_EXISTS;
    }

    if (soc_feature(unit, soc_feature_xflow_macsec_restricted_move))
    {
        /* Valid prio range is [0 - max_index] */
        tcam_index = logical_index; /* MSN REVISIT tcam_map->tcam_index_count - prio - 1; */
        
        if (tcam_index < 0)
            return BCM_E_PARAM;

        if (tcam_index >= tcam_map->tcam_index_count)
            return BCM_E_PARAM;

        if (tcam_map->tcam_ptr[tcam_index].in_use == 1)
            return BCM_E_PARAM;
    }
    else
    {
        rv = _xflow_macsec_tcam_ptr_find_target_index(*tcam_map, prio, &tcam_index);
        if (BCM_FAILURE(rv))
            return rv;

        if (tcam_index < 0)
            return BCM_E_FULL;

        if (tcam_map->tcam_ptr[tcam_index].in_use)
        {
            rv = _xflow_macsec_tcam_ptr_move_and_free(unit, *tcam_map, tcam_index);
            if (BCM_FAILURE(rv))
                return rv;
        }
    }

    tcam_map->free_count--;
    tcam_map->tcam_ptr[tcam_index].priority = prio;
    tcam_map->tcam_ptr[tcam_index].tcam_logical_map_id = logical_index;
    tcam_map->tcam_ptr[tcam_index].in_use = 1;

    tcam_map->tcam_logical_map[logical_index].priority = prio;
    tcam_map->tcam_logical_map[logical_index].hw_index = tcam_index;
    tcam_map->tcam_logical_map[logical_index].in_use = 1;

    *index = logical_index;

    return BCM_E_NONE;
}

int _xflow_macsec_sc_decrypt_index_reserve(int unit, int prio, int *index, uint8 flag)
{
    return _xflow_macsec_tcam_map_index_reserve(unit, &sc_tcam_map[unit], prio, index, flag);
}

int _xflow_macsec_flow_index_reserve(int unit, int prio, int *index, uint8 flag)
{
    return _xflow_macsec_tcam_map_index_reserve(unit, &sp_tcam_map[unit], prio, index, flag);
}

int _xflow_macsec_policy_index_reserve(int unit, int prio, int *index, uint8 flag)
{
    if (index == NULL)
        return BCM_E_PARAM;

    if (flag == FALSE)
    {
        _SHR_BIT_FIND_FREE(policy_bmp[unit], _XFLOW_MACSEC_POLICY_MAX, *index);
        if (*index >= _XFLOW_MACSEC_POLICY_MAX)
        {
            return BCM_E_FULL;
        }
    }
    else
    {
        if (*index >= _XFLOW_MACSEC_POLICY_MAX)
            return BCM_E_PARAM;

        if (SHR_BITGET(policy_bmp[unit], *index))
            return BCM_E_EXISTS;
    }
    SHR_BITSET(policy_bmp[unit], *index);

   return BCM_E_NONE;
}

int _xflow_macsec_policy_index_free(int unit, int index)
{
    if (index >= _XFLOW_MACSEC_POLICY_MAX)
        return BCM_E_PARAM;

    if (SHR_BITGET(policy_bmp[unit], index))
        SHR_BITCLR(policy_bmp[unit], index);

    return BCM_E_NONE;
}

int _xflow_macsec_policy_index_hw_index_get(int unit, int index, int *hw_index, int *prio)
{
    if (hw_index == NULL)
        return BCM_E_PARAM;
    if (index >= _XFLOW_MACSEC_POLICY_MAX)
        return BCM_E_PARAM;

    if (!SHR_BITGET(policy_bmp[unit], index))
        return BCM_E_NOT_FOUND;

    *hw_index = index;

    return BCM_E_NONE;
}



int _xflow_macsec_sc_encrypt_index_free(int unit, int index)
{

    if (index >= _XFLOW_MACSEC_SC_ENCRYPT_MAX)
        return BCM_E_PARAM;

    if (SHR_BITGET(sc_encrypt_bmp[unit], index))
        SHR_BITCLR(sc_encrypt_bmp[unit], index);

    return BCM_E_NONE;
}

/*
 * Update the priority of a given tcam entry. The HW entries are
 * adjusted according to the new priority.
 */
int xflow_macsec_index_update_priority(int unit, xflow_macsec_index_type_t type,
                                   int index, int priority)
{
    xflow_macsec_tcam_map_t tcam_map;
    int old_priority, new_tcam_index;
    int tcam_index;

    if (type == xflowMacsecIdTypeSecureChan)
    {
        tcam_map = sc_tcam_map[unit];
    }
    else if (type == xflowMacsecIdTypeFlow)
    {
        tcam_map = sp_tcam_map[unit];
    }
    else
    {
        return BCM_E_PARAM;
    }

    if (index >= tcam_map.tcam_index_count)
        return BCM_E_PARAM;

    /* If priority is the same, there is no update. */
    old_priority = tcam_map.tcam_logical_map[index].priority;
    if (old_priority == priority)
        return BCM_E_NONE;

    if (soc_feature(unit, soc_feature_xflow_macsec_restricted_move))
    {
        /* Valid prio range is [0 - max_index] */
        new_tcam_index = tcam_map.tcam_index_count - priority - 1;

        if (new_tcam_index < 0)
            return BCM_E_PARAM;

        if (new_tcam_index >= tcam_map.tcam_index_count)
            return BCM_E_PARAM;

        if (tcam_map.tcam_ptr[new_tcam_index].in_use == 1)
            return BCM_E_PARAM;
    }
    else
    {
        /* For the new priority, find an index in the tcam_ptr. */
        BCM_IF_ERROR_RETURN(_xflow_macsec_tcam_ptr_find_target_index(tcam_map, priority, &new_tcam_index));
        if (new_tcam_index < 0)
            return BCM_E_FULL;

        /* If the new index is in use, find the next free entry in either
         * direction and move all entries. This involves HW move.
         * After this operation, the new index must be free.
         */
        if (tcam_map.tcam_ptr[new_tcam_index].in_use)
        {
            BCM_IF_ERROR_RETURN(_xflow_macsec_tcam_ptr_move_and_free(unit, tcam_map, new_tcam_index));
        }
    }

    /* Move the HW entry from the original index to the new index. */
    tcam_index = tcam_map.tcam_logical_map[index].hw_index;
    BCM_IF_ERROR_RETURN(_xflow_macsec_tcam_ptr_move_single_tcam_entry(unit, tcam_map,
                                                                    new_tcam_index,
                                                                    tcam_index));

    /* Update the priority to the new one */
    tcam_map.tcam_logical_map[index].priority = priority;
    tcam_map.tcam_logical_map[index].hw_index = new_tcam_index;
    tcam_map.tcam_ptr[new_tcam_index].priority = priority;

    return BCM_E_NONE;
}

int _xflow_macsec_resource_init(int unit)
{
    int alloc_size;
    int mem_count;
    int rv = BCM_E_NONE;
    xflow_macsec_db_t *macsec_db = NULL;

    BCM_IF_ERROR_RETURN(xflow_macsec_db_get(unit, &macsec_db));
    if (macsec_db == NULL)
    {
        macsec_db = sal_alloc(sizeof(xflow_macsec_db_t), "Xflow Macsec DB");
        if (!macsec_db)
            return BCM_E_MEMORY;

        memset(macsec_db, 0, sizeof(xflow_macsec_db_t));
        macsec_db->reserved_policy_index = -1;
        rv = xflow_macsec_db_set(unit, macsec_db);

        if (BCM_FAILURE(rv))
            goto resource_init_error;
    }

    /* Encrypt Init */
    memset(sc_encrypt_bmp[unit], 0, sizeof(sc_encrypt_bmp[unit]));

    /* Decrypt Policy Init */
    memset(policy_bmp[unit], 0, sizeof(policy_bmp[unit]));

    /*
     * Decrypt Secure Channel
     */
    mem_count = 3;
    alloc_size = mem_count * sizeof(soc_mem_t);
    sc_tcam_map[unit].tcam_mem = sal_alloc(alloc_size, "sc_tcam_map.tcam_mem");
    _XFLOW_MACSEC_MEM_NULL_CHECK_SET_RV_GOTO_ERROR(sc_tcam_map[unit].tcam_mem, alloc_size, rv, resource_init_error);

    if (soc_feature(unit, soc_feature_xflow_macsec_inline))
    {
        /*
         * Inline Macsec requires SC to be configured first.
         */
        sc_tcam_map[unit].tcam_mem[0] = ISEC_SA_TABLEm;
        sc_tcam_map[unit].tcam_mem[1] = ISEC_SC_TABLEm;
        sc_tcam_map[unit].tcam_mem[2] = ISEC_SC_TCAMm;
    }
    else
    {
        sc_tcam_map[unit].tcam_mem[0] = ISEC_SC_TCAMm;
        sc_tcam_map[unit].tcam_mem[1] = ISEC_SC_TABLEm;
        sc_tcam_map[unit].tcam_mem[2] = ISEC_SA_TABLEm;
    }
    sc_tcam_map[unit].tcam_mem_count = 3;

    mem_count = soc_mem_index_count(unit, sc_tcam_map[unit].tcam_mem[0]);

    if (XFLOW_MACSEC_4_SA_PER_SC(unit))
    {
        mem_count = mem_count / 4;
    }
    else if (XFLOW_MACSEC_NUM_SA_PER_SC(unit, XFLOW_MACSEC_DECRYPT))
    {
        mem_count = mem_count / XFLOW_MACSEC_NUM_SA_PER_SC(unit, XFLOW_MACSEC_DECRYPT);
    }

    alloc_size = mem_count * sizeof(xflow_macsec_tcam_logical_map_t);
    sc_tcam_map[unit].tcam_logical_map = sal_alloc(alloc_size, "SC_tcam_map.tcam_logical_map");

    _XFLOW_MACSEC_MEM_NULL_CHECK_SET_RV_GOTO_ERROR(sc_tcam_map[unit].tcam_logical_map, alloc_size, rv,
                                                    resource_init_error);

    alloc_size = mem_count * sizeof(xflow_macsec_tcam_ptr_t);
    sc_tcam_map[unit].tcam_ptr = sal_alloc(alloc_size, "sc_tcam_map.tcam_ptr");
    _XFLOW_MACSEC_MEM_NULL_CHECK_SET_RV_GOTO_ERROR(sc_tcam_map[unit].tcam_ptr, alloc_size, rv, resource_init_error);

    if (soc_feature(unit, soc_feature_xflow_macsec_restricted_move))
    {
        sc_tcam_map[unit].free_count = mem_count;
        sc_tcam_map[unit].tcam_index_count = mem_count;
    }
    else
    {
        /* This reserves a mem descriptor entry at the end of the pointer arrays */
        sc_tcam_map[unit].reserved_index = mem_count - 1;
        sc_tcam_map[unit].free_count = mem_count - 1;
        sc_tcam_map[unit].tcam_index_count = mem_count;
        sc_tcam_map[unit].tcam_logical_map[mem_count - 1].in_use = 1;
        sc_tcam_map[unit].tcam_logical_map[mem_count - 1].hw_index = (mem_count - 1);
        sc_tcam_map[unit].tcam_ptr[mem_count - 1].in_use = 1;
    }

    /*
     * Decrypt Flows (Sub Port)
     */
    mem_count = 3;
    alloc_size = mem_count * sizeof(soc_mem_t);
    sp_tcam_map[unit].tcam_mem = sal_alloc(alloc_size, "sp_tcam_map.tcam_mem");
    _XFLOW_MACSEC_MEM_NULL_CHECK_SET_RV_GOTO_ERROR(sp_tcam_map[unit].tcam_mem, alloc_size, rv, resource_init_error);

    sp_tcam_map[unit].tcam_mem_count = 3;
    sp_tcam_map[unit].tcam_mem[0] = ISEC_SP_TCAM_KEYm;
    sp_tcam_map[unit].tcam_mem[1] = ISEC_SP_TCAM_MASKm;
    sp_tcam_map[unit].tcam_mem[2] = SUB_PORT_MAP_TABLEm;

    mem_count = soc_mem_index_count(unit, sp_tcam_map[unit].tcam_mem[0]);
    alloc_size = mem_count * sizeof(xflow_macsec_tcam_logical_map_t);
    sp_tcam_map[unit].tcam_logical_map = sal_alloc(alloc_size, "sp_tcam_map.tcam_logical_map");

    _XFLOW_MACSEC_MEM_NULL_CHECK_SET_RV_GOTO_ERROR(sp_tcam_map[unit].tcam_logical_map, alloc_size, rv,
                                                    resource_init_error);

    alloc_size = mem_count * sizeof(xflow_macsec_tcam_ptr_t);
    sp_tcam_map[unit].tcam_ptr = sal_alloc(alloc_size, "sp_tcam_map.tcam_ptr");
    _XFLOW_MACSEC_MEM_NULL_CHECK_SET_RV_GOTO_ERROR(sp_tcam_map[unit].tcam_ptr, alloc_size, rv, resource_init_error);

    if (soc_feature(unit, soc_feature_xflow_macsec_restricted_move))
    {
        sp_tcam_map[unit].free_count = mem_count;
        sp_tcam_map[unit].tcam_index_count = mem_count;
    }
    else
    {
        sp_tcam_map[unit].reserved_index = mem_count - 1;
        sp_tcam_map[unit].free_count = mem_count - 1;
        sp_tcam_map[unit].tcam_index_count = mem_count;
        sp_tcam_map[unit].tcam_logical_map[mem_count - 1].in_use = 1;
        sp_tcam_map[unit].tcam_logical_map[mem_count - 1].hw_index = (mem_count - 1);
        sp_tcam_map[unit].tcam_ptr[mem_count - 1].in_use = 1;
    }

    if (soc_feature(unit, soc_feature_xflow_macsec_sa_expiry_war) ||
        soc_feature(unit, soc_feature_xflow_macsec_sa_next_pn_update_error_war))
    {
        if (SOC_MEM_TABLE_BYTES(unit, ESEC_SC_TABLEm) > SOC_MEM_TABLE_BYTES(unit, ISEC_SC_TABLEm))
        {
            alloc_size = SOC_MEM_TABLE_BYTES(unit, ESEC_SC_TABLEm);
        }
        else
        {
            alloc_size = SOC_MEM_TABLE_BYTES(unit, ISEC_SC_TABLEm);
        }

        sc_table_dma = soc_cm_salloc(unit, alloc_size, "sc_table_dma");
        _XFLOW_MACSEC_MEM_NULL_CHECK_SET_RV_GOTO_ERROR(sc_table_dma, alloc_size, rv, resource_init_error);

        if (SOC_MEM_TABLE_BYTES(unit, ESEC_SA_TABLEm) > SOC_MEM_TABLE_BYTES(unit, ISEC_SA_TABLEm))
        {
            alloc_size = SOC_MEM_TABLE_BYTES(unit, ESEC_SA_TABLEm);
        }
        else
        {
            alloc_size = SOC_MEM_TABLE_BYTES(unit, ISEC_SA_TABLEm);
        }

        sa_table_dma = soc_cm_salloc(unit, alloc_size, "sa_table_dma");

        _XFLOW_MACSEC_MEM_NULL_CHECK_SET_RV_GOTO_ERROR(sa_table_dma, alloc_size, rv, resource_init_error);
    }

    return rv;

resource_init_error:

    printk("ERROR:ERRROR:ERROR: resource init ERROR!\n");

    if (sc_tcam_map[unit].tcam_mem)
    {
        sal_free(sc_tcam_map[unit].tcam_mem);
        sc_tcam_map[unit].tcam_mem = NULL;
    }

    if (sc_tcam_map[unit].tcam_logical_map)
    {
        sal_free(sc_tcam_map[unit].tcam_logical_map);
        sc_tcam_map[unit].tcam_logical_map = NULL;
    }

    if (sc_tcam_map[unit].tcam_ptr)
    {
        sal_free(sc_tcam_map[unit].tcam_ptr);
        sc_tcam_map[unit].tcam_ptr = NULL;
    }

    if (sp_tcam_map[unit].tcam_mem)
    {
        sal_free(sp_tcam_map[unit].tcam_mem);
        sp_tcam_map[unit].tcam_mem = NULL;
    }

    if (sp_tcam_map[unit].tcam_logical_map)
    {
        sal_free(sp_tcam_map[unit].tcam_logical_map);
        sp_tcam_map[unit].tcam_logical_map = NULL;
    }

    if (sp_tcam_map[unit].tcam_ptr)
    {
        sal_free(sp_tcam_map[unit].tcam_ptr);
        sp_tcam_map[unit].tcam_ptr = NULL;
    }

    sc_tcam_map[unit].reserved_index = 0;
    sc_tcam_map[unit].free_count = 0;
    sc_tcam_map[unit].tcam_mem_count = 0;
    sc_tcam_map[unit].tcam_index_count = 0;
    sp_tcam_map[unit].reserved_index = 0;
    sp_tcam_map[unit].free_count = 0;
    sp_tcam_map[unit].tcam_mem_count = 0;
    sp_tcam_map[unit].tcam_index_count = 0;

    if (soc_feature(unit, soc_feature_xflow_macsec_sa_expiry_war) ||
        soc_feature(unit, soc_feature_xflow_macsec_sa_next_pn_update_error_war))
    {
        if (sc_table_dma)
        {
            soc_cm_sfree(unit, sc_table_dma);
            sc_table_dma = NULL;
        }

        if (sa_table_dma)
        {
            soc_cm_sfree(unit, sa_table_dma);
            sa_table_dma = NULL;
        }
    }

    return rv;
}
