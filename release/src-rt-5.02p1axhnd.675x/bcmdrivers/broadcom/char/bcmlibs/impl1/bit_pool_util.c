/*
<:copyright-BRCM:2018:proprietary:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/

/*
 *******************************************************************************
 * File Name  : bit_pool_util.c
 * This file provides implementation for free index pool.
 *******************************************************************************
 */
#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/slab.h>
#include "idx_pool_util.h"
#include <linux/bcm_log.h>
/* ***************  defines **************/
#define IDX_POOL_SUCCESS	    (0)
#define IDX_POOL_ERROR		    (-1)

#define SHIFT_BITS(b)	        (1ULL<<(b))

#define __bitPoolDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_BCMLIBS_BIT_POOL, "%u:" "%s:" fmt, __LINE__,owner,##arg)
#define __bitPoolInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_BCMLIBS_BIT_POOL, "%u:" "%s:" fmt, __LINE__,owner,##arg)
#define __bitPoolNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_BCMLIBS_BIT_POOL, "%u:" "%s:" fmt, __LINE__,owner,##arg)
#define __bitPoolError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_BCMLIBS_BIT_POOL, "%u:" "%s:" fmt, __LINE__,owner,##arg)

/* Below structure contains all the nodes of the tree at a given level
 * level=0 for the last leaf nodes and root will have the highest level */
typedef struct BitPoolNode_s {
    uint16_t level;                         /* Level of the tree nodes - DEBUG use only */
    uint16_t num_nodes;
    uint32_t *bit_nodes;                    /* Array of all the nodes at this level */
    struct BitPoolNode_s *next_lvl_node;    /* next/child of this parent/level */
}BitPoolNode_t;

typedef struct BitPool_s {
    char owner[16];         /* Owner name - for debugging */
    BitPoolNode_t *pPool;
}BitPool_t;

#define BITS_PER_NODE			(sizeof(uint32_t)*8)


/*
 *------------------------------------------------------------------------------
 * Function     : _bit_pool_create_nodes
 * Description  : Creates nodes required to accomodate all the bits
 *------------------------------------------------------------------------------
 */
static BitPoolNode_t* _bit_pool_create_nodes(uint32_t num_bits, uint32_t *level, BitPoolNode_t *pNxtLvlNode,
                                             const char *const owner)
{
    int num_nodes = (num_bits + BITS_PER_NODE - 1) / BITS_PER_NODE;
    BitPoolNode_t * pNode,*pParent;
    int i;
    uint32_t bit;

    pNode = kmalloc(sizeof(BitPoolNode_t), GFP_ATOMIC);
    if (!pNode)
    {
        return NULL;
    }
    pNode->bit_nodes = (uint32_t *)(((uintptr_t)kmalloc((sizeof(uint32_t) * (num_nodes)), GFP_ATOMIC)));
    pNode->num_nodes = num_nodes;
    if (!pNode->bit_nodes)
    {
        kfree(pNode);
        return NULL;
    }
    pNode->next_lvl_node = pNxtLvlNode;

    __bitPoolDebug("num_bits=%d, pNxtLvlNode=0x%p num_nodes=%d\n", num_bits, pNxtLvlNode, num_nodes);

    for (i = 0; i < num_nodes; i++)
    {
        bit = num_bits / BITS_PER_NODE ? BITS_PER_NODE : num_bits % BITS_PER_NODE;
        num_bits -= bit;
        pNode->bit_nodes[i] = 0;
    }

    if (num_nodes > 1)
    {
        pParent = _bit_pool_create_nodes(num_nodes, level, pNode, owner);
        *level += 1;
    }
    else
    {
        pParent = pNode;
        *level = 0;
    }

    pNode->level = *level;
    return pParent;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _bit_pool_get_first_set
 * Description  : Traverse the tree to find the first set bit
 *------------------------------------------------------------------------------
 */
static int _bit_pool_get_first_set(BitPoolNode_t *pNode, uint32_t node_idx, uint32_t *bucket_empty,
                                   const char *const owner)
{
    uint32_t value;
    uint32_t bit;
    int free_index;

    __bitPoolDebug(" [%u]  node_idx=%u CurBits=0x%08x", pNode->level, node_idx, pNode->bit_nodes[node_idx]);
    value = pNode->bit_nodes[node_idx];
    bit = ffs(value);
    if (!bit)
    {
        __bitPoolDebug(" [%u]  node_idx=%u NewBits=0x%08x", pNode->level, node_idx, pNode->bit_nodes[node_idx]);
        return -1;
    }
    bit--; /* ffs return 1-based position */
    if (pNode->next_lvl_node)
    {
        free_index = _bit_pool_get_first_set(pNode->next_lvl_node, (node_idx * BITS_PER_NODE) + bit, bucket_empty, owner);
        if (free_index >= 0 && *bucket_empty)
        {
            *bucket_empty = 0; /* Reset bucket_empty for this level */
            pNode->bit_nodes[node_idx] &= ~(SHIFT_BITS(bit));
            if (!pNode->bit_nodes[node_idx])
            {
                *bucket_empty = 1;
            }
            __bitPoolDebug(" [%u]  node_idx=%u bit=%u NewBits=0x%08x Empty=%u", pNode->level, node_idx, bit, pNode->bit_nodes[node_idx], *bucket_empty);
        }
    }
    else
    {
        /* Last level keeps the actual bit value */
        free_index = (bit)+node_idx * BITS_PER_NODE;
        pNode->bit_nodes[node_idx] &= ~(SHIFT_BITS(bit));
        if (!pNode->bit_nodes[node_idx])
        {
            *bucket_empty = 1;
        }
        __bitPoolDebug(" [%u]  node_idx=%u bit=%u NewBits=0x%08x Empty=%u", pNode->level, node_idx, bit, pNode->bit_nodes[node_idx], *bucket_empty);
    }
    return free_index;
}
/*
 *------------------------------------------------------------------------------
 * Function     : _bit_pool_find_first_zero
 * Description  : Traverse the tree to find the first zero bit 
 *                (TODO : Inefficient - please optimize )
 *------------------------------------------------------------------------------
 */
static int _bit_pool_find_first_zero(BitPoolNode_t *pNode, const char *const owner)
{
    uint32_t value;
    uint32_t bit;
    uint32_t node_idx;
    int zero_bit_idx = -1;

    if (pNode->next_lvl_node)
    {
        zero_bit_idx = _bit_pool_find_first_zero(pNode->next_lvl_node, owner);
    }
    else
    {
        for (node_idx = 0; node_idx < pNode->num_nodes; node_idx++)
        {
            __bitPoolDebug(" [%u]  node_idx=%u CurBits=0x%08x", pNode->level, node_idx, pNode->bit_nodes[node_idx]);
            value = ~pNode->bit_nodes[node_idx];
            bit = ffs(value);
            if (!bit)
            {
                continue;
            }
            bit--; /* ffs return 1-based position */
            /* Last level keeps the actual bit value */
            zero_bit_idx = (bit)+node_idx * BITS_PER_NODE;
            __bitPoolDebug(" [%u]  node_idx=%u bit=%u CurBits=0x%08x ", pNode->level, node_idx, bit, pNode->bit_nodes[node_idx]);
            break;
        }
    }
    return zero_bit_idx;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _bit_pool_find_next_zero
 * Description  : Traverse the tree to find the next zero bit
 *                (TODO : Inefficient - please optimize )
 *------------------------------------------------------------------------------
 */
static int _bit_pool_find_next_zero(BitPoolNode_t *pNode, const uint32_t bit_num, const char *const owner)
{
    uint32_t value;
    uint32_t bit;
    uint32_t node_idx;
    int zero_bit_idx = -1;

    if (pNode->next_lvl_node)
    {
        return _bit_pool_find_next_zero(pNode->next_lvl_node, bit_num, owner);
    }
    else
    {
        for (node_idx = (bit_num/BITS_PER_NODE); node_idx < pNode->num_nodes; node_idx++)
        {
            __bitPoolDebug(" [%u]  node_idx=%u CurBits=0x%08x", pNode->level, node_idx, pNode->bit_nodes[node_idx]);
            value = ~pNode->bit_nodes[node_idx];
            while (value)
            {
                bit = ffs(value);
                if (!bit)
                {
                    break;
                }
                bit--; /* ffs return 1-based position */
                /* Last level keeps the actual bit value */
                zero_bit_idx = (bit)+node_idx * BITS_PER_NODE;
                if (zero_bit_idx > bit_num)
                {
                    __bitPoolDebug(" [%u]  node_idx=%u bit=%u zero_bit_idx=%d CurBits=0x%08x ", pNode->level, node_idx, bit, 
                                   zero_bit_idx, pNode->bit_nodes[node_idx]);
                    return zero_bit_idx;
                }
                value &= ~(SHIFT_BITS(bit));
            }
        }
    }
    __bitPoolDebug(" [%u]  No next index after %u ", pNode->level, bit_num);
    return -1;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _bit_pool_set_bit
 * Description  : Set the specified bit in the bit pool
 *------------------------------------------------------------------------------
 */
static int _bit_pool_set_bit(BitPoolNode_t *pNode, uint32_t bit_num, const char *const owner)
{
    int idx;
    uint32_t bit;

    if (pNode->next_lvl_node)
    {
        idx = _bit_pool_set_bit(pNode->next_lvl_node, bit_num, owner);
        if (idx >= 0)
        {
            bit = idx % BITS_PER_NODE; /* First extract the bit position before modifying idx */
            idx = idx / BITS_PER_NODE;
            pNode->bit_nodes[idx] |= (SHIFT_BITS(bit));
            __bitPoolDebug(" [%u]  bit_num = %u idx=%u bit=%u NewFree=0x%08x", pNode->level, bit_num, idx, bit, pNode->bit_nodes[idx]);
        }
    }
    else
    {
        /* Last level keeps the index */
        bit = bit_num % BITS_PER_NODE;
        idx = bit_num / BITS_PER_NODE;
        if (pNode->bit_nodes[idx] & (SHIFT_BITS(bit)))
        {
            __bitPoolError("Already set bit_num = %u", bit_num);
            /* Duplicate free */
            return -1;
        }
        pNode->bit_nodes[idx] |= (SHIFT_BITS(bit));
        __bitPoolDebug(" [%u]  bit_num = %u idx=%u bit=%u NewFree=0x%08x", pNode->level, bit_num, idx, bit, pNode->bit_nodes[idx]);
    }
    return idx;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _bit_pool_is_set
 * Description  : Check if the specified bit is set
 *------------------------------------------------------------------------------
 */
static int _bit_pool_is_set(BitPoolNode_t *pNode, uint32_t bit_num, const char *const owner)
{
    int idx;
    int is_set = 0;
    uint32_t bit;

    if (pNode->next_lvl_node)
    {
        is_set = _bit_pool_is_set(pNode->next_lvl_node, bit_num, owner);
    }
    else
    {
        /* Last level keeps the actual bit value */
        bit = bit_num % BITS_PER_NODE;
        idx = bit_num / BITS_PER_NODE;
        if (pNode->bit_nodes[idx] & (SHIFT_BITS(bit)))
        {
            is_set = 1;
        }
        __bitPoolDebug(" [%u]  bit_num = %u idx=%u bit=%u is_set=%d NewFree=0x%08x", pNode->level, bit_num, idx, bit, is_set, pNode->bit_nodes[idx]);
    }
    return is_set;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _bit_pool_delete_nodes
 * Description  : Delete all the tree nodes and release memory
 *------------------------------------------------------------------------------
 */
static int _bit_pool_delete_nodes(BitPoolNode_t *pNode, const char *const owner)
{
    if (!pNode)
    {
        return 0;
    }
    _bit_pool_delete_nodes(pNode->next_lvl_node, owner);

    kfree(pNode->bit_nodes);
    kfree(pNode);

    return 0;
}

/* ************* Interface Functions ***************** */

/*
 *------------------------------------------------------------------------------
 * Function     : idx_pool_get_index
 * Description  : User interface to get the first available index from the pool
 * Return       : -ve = Error/No free index, otherwise the free index value
 *------------------------------------------------------------------------------
 */
int idx_pool_get_index(IdxPool_t *pIdxPool)
{
    int free_index;
    uint32_t bucket_empty = 0;
    BitPool_t *pBitPool = (BitPool_t *)(pIdxPool->pTree);
    const char *const owner = pBitPool->owner;

    free_index = _bit_pool_get_first_set(pBitPool->pPool, 0, &bucket_empty, owner);
    __bitPoolDebug("Free_Index = %d Empty=%u", free_index, bucket_empty);
    if (free_index < 0)
    {
        __bitPoolInfo("No more Free_Index = %d Pool_size=%u idxs_in_use=%u",
                      free_index, pIdxPool->pool_size, pIdxPool->idxs_in_use);
        return free_index; /* Just for accouting below */
    }
    pIdxPool->idxs_in_use++;
    __bitPoolInfo("Free_Index = %u idxs_in_use=%u ", free_index, pIdxPool->idxs_in_use);
    return free_index;
}
EXPORT_SYMBOL(idx_pool_get_index);

/*
 *------------------------------------------------------------------------------
 * Function     : idx_pool_return_index
 * Description  : User interface to return the previously allocated index to the pool
 * Return       : -ve = Error (Duplicate/Out-of-range), otherwise success
 *------------------------------------------------------------------------------
 */
int idx_pool_return_index(IdxPool_t *pIdxPool, uint32_t free_index)
{
    int err;
    BitPool_t *pBitPool = (BitPool_t *)(pIdxPool->pTree);
    const char *const owner = pBitPool->owner;
    if (free_index >= pIdxPool->pool_size) /* free_index 0-based; pool-size 1-based */
    {
        __bitPoolError("Error : Invalid index <%u> to free; Index Pool size <%u> ", free_index, pIdxPool->pool_size);
        return -1;
    }
    err = _bit_pool_set_bit(pBitPool->pPool, free_index, owner);
    if (err < 0) /* +ive values are index */
    {
        __bitPoolError("Error : Invalid index <%u> to free; Index Pool size <%u> idxs_in_use <%u>",
                       free_index, pIdxPool->pool_size, pIdxPool->idxs_in_use);
        return -1;
    }
    pIdxPool->idxs_in_use--;
    __bitPoolInfo("Free_Index = %u  Index Pool size = %u idxs_in_use=%u ",
                  free_index, pIdxPool->pool_size, pIdxPool->idxs_in_use);
    return 0;
}
EXPORT_SYMBOL(idx_pool_return_index);

/*
 *------------------------------------------------------------------------------
 * Function     : idx_pool_index_in_use
 * Description  : User interface to check if index is allocated or not
 * Return       : 0 = No, 1 = Yes
 *------------------------------------------------------------------------------
 */
int idx_pool_index_in_use(IdxPool_t *pIdxPool, uint32_t index)
{
    int in_use = 0;
    BitPool_t *pBitPool = (BitPool_t *)(pIdxPool->pTree);
    const char *const owner = pBitPool->owner;
    if (index < pIdxPool->pool_size) /* index 0-based; pool-size 1-based */
    {
        in_use = !_bit_pool_is_set(pBitPool->pPool, index, owner);
    }
    __bitPoolInfo("Index = %u  in_use = %u idxs_in_use=%u ",
                  index, in_use, pIdxPool->idxs_in_use);
    return in_use;
}
EXPORT_SYMBOL(idx_pool_index_in_use);

/*
 *------------------------------------------------------------------------------
 * Function     : idx_pool_num_in_use
 * Description  : User interface to get number of index in use
 * Return       : Indexes in use
 *------------------------------------------------------------------------------
 */
uint32_t idx_pool_num_in_use(IdxPool_t *pIdxPool)
{
    return pIdxPool->idxs_in_use;
}
EXPORT_SYMBOL(idx_pool_num_in_use);

/*
 *------------------------------------------------------------------------------
 * Function     : idx_pool_get_pool_size
 * Description  : User interface to get pool size
 * Return       : Pool size
 *------------------------------------------------------------------------------
 */
uint32_t idx_pool_get_pool_size(IdxPool_t *pIdxPool)
{
    return pIdxPool->pool_size;
}
EXPORT_SYMBOL(idx_pool_get_pool_size);

/*
 *------------------------------------------------------------------------------
 * Function     : idx_pool_first_in_use
 * Description  : User interface to get first used index if any
 * Return       : 0 - found in use index; -1 no indexes in use
 *------------------------------------------------------------------------------
 */
int idx_pool_first_in_use(IdxPool_t *pIdxPool, uint32_t *index)
{
    int in_use_index;
    BitPool_t *pBitPool = (BitPool_t *)(pIdxPool->pTree);
    const char *const owner = pBitPool->owner;

    in_use_index = _bit_pool_find_first_zero(pBitPool->pPool, owner);
    __bitPoolInfo("in_use_index = %d", in_use_index);
    if (in_use_index < 0 || in_use_index >= pIdxPool->pool_size)
    {
        __bitPoolInfo("No in_use_index = %d Pool_size=%u idxs_in_use=%u",
                      in_use_index, pIdxPool->pool_size, pIdxPool->idxs_in_use);
        return -1; /* Just for accouting below */
    }

    *index = in_use_index;

    return 0;
}
EXPORT_SYMBOL(idx_pool_first_in_use);

/*
 *------------------------------------------------------------------------------
 * Function     : idx_pool_next_in_use
 * Description  : User interface to get next used index if any
 * Return       : 0 - found in use next index; -1 no indexes in use
 *------------------------------------------------------------------------------
 */
int idx_pool_next_in_use(IdxPool_t *pIdxPool, uint32_t index, uint32_t *next_index)
{
    int in_use_index;
    BitPool_t *pBitPool = (BitPool_t *)(pIdxPool->pTree);
    const char *const owner = pBitPool->owner;

    in_use_index = _bit_pool_find_next_zero(pBitPool->pPool, index, owner);
    __bitPoolInfo("index=%u next_in_use_index = %d", index, in_use_index);
    if (in_use_index < 0 || in_use_index >= pIdxPool->pool_size)
    {
        __bitPoolInfo("No in_use_index = %d Pool_size=%u idxs_in_use=%u",
                      in_use_index, pIdxPool->pool_size, pIdxPool->idxs_in_use);
        return -1; /* Just for accouting below */
    }

    *next_index = in_use_index;

    return 0;
}
EXPORT_SYMBOL(idx_pool_next_in_use);

/*
 *------------------------------------------------------------------------------
 * Function     : idx_pool_init
 * Description  : Initialize the pool with indexes based on pool_size (i.e. number of indexes)
 * Return       : -ve = Error (Memory),
 *                 otherwise success and Pool structure is updated with tree/root node pointer
 *------------------------------------------------------------------------------
 */
int idx_pool_init(IdxPool_t *pIdxPool, const uint32_t pool_size, const char *const owner)
{
    uint32_t bit_num;
    uint32_t level = 0;
    BitPool_t *pBitPool = (BitPool_t *)(((uintptr_t)kmalloc((sizeof(BitPool_t)), GFP_ATOMIC)));
    if (!pBitPool)
    {
        __bitPoolError("Failed to allocate memory = %u", pool_size);
        return -1;
    }

    strncpy(pBitPool->owner, owner, sizeof(pBitPool->owner));
    pBitPool->owner[sizeof(pBitPool->owner) - 1] = '\0';

    __bitPoolNotice("Create Index Pool_Size = %u", pool_size);
    pBitPool->pPool = _bit_pool_create_nodes(pool_size, &level, NULL, owner);
    if (!pBitPool->pPool)
    {
        kfree(pBitPool);
        return -1;
    }
    /* Fill all the indexes i.e. set bits */
    for (bit_num = 0; bit_num < pool_size; bit_num++)
    {
        _bit_pool_set_bit(pBitPool->pPool, bit_num, owner);
    }
    pIdxPool->pTree = pBitPool;
    pIdxPool->pool_size = pool_size;
    pIdxPool->idxs_in_use = 0;
    return 0;
}
EXPORT_SYMBOL(idx_pool_init);

/*
 *------------------------------------------------------------------------------
 * Function     : idx_pool_exit
 * Description  : Release the pool (i.e. all memory)
 * Return       : -Success
 *------------------------------------------------------------------------------
 */
int idx_pool_exit(IdxPool_t *pIdxPool)
{
    BitPool_t *pBitPool = (BitPool_t *)(pIdxPool->pTree);
    const char *const owner = pBitPool->owner;
    if (pIdxPool->pool_size)
    {
        _bit_pool_delete_nodes(pBitPool->pPool, owner);
        pIdxPool->pTree = NULL;
    }
    pIdxPool->pool_size = 0;
    pIdxPool->idxs_in_use = 0;
    return IDX_POOL_SUCCESS;
}
EXPORT_SYMBOL(idx_pool_exit);
#endif /* __KERNEL__ */
