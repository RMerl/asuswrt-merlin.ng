/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom
 *  All Rights Reserved
 *
<:label-BRCM:2018:proprietary:standard

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
 *
 ************************************************************************/

/*
 * NVRAM-GET memory management
 *
 * $Id:$
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "nvram_hash.h"

static int move_to_deadlist(char *value);
static unsigned long djb2_hash(const char *str);

static NvmHashEntry *nvm_hash_table = NULL;
static Buffer *dead_buffers_list = NULL;


/*djb2 hash algorithem.*/
static unsigned long djb2_hash(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++) != 0)
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

UBOOL8 nvram_hash_init()
{
    if (nvm_hash_table == NULL)
    {
        nvm_hash_table = (NvmHashEntry *) calloc(HASH_TABLE_SIZE, sizeof(NvmHashEntry));
        if (nvm_hash_table == NULL)
        {
            return FALSE;
        }
    }
    return TRUE;
}

char *nvram_hash_get(const char *name)
{
    assert(name);
    unsigned long hash = djb2_hash(name);
    unsigned int index = hash % HASH_TABLE_SIZE;
    NvmPair *p;

    for (p = nvm_hash_table[index].nvm_list; p != NULL; p = p->next)
    {
        if (strcmp(p->name, name) == 0)
        {
            return p->value;
        }
    }
    return NULL;
}

UBOOL8 nvram_hash_update(const char *name, const char *value)
{
    unsigned int hash = djb2_hash(name);
    unsigned int index = hash % HASH_TABLE_SIZE;
    NvmHashEntry *h;
    NvmPair *p;

    assert(name);
    assert(value);

    h = &nvm_hash_table[index];
    for (p = h->nvm_list; p != NULL; p = p->next)
    {
        if (strcmp(p->name, name) == 0)
        {
            break;
        }
    }

    if (p == NULL)
    {
        /*
         * We didn't find any previously allocated buffer for this NVRAM configuration
         * in the hash table. So create a bucket for the name-value pair and add it
         * to the hash table.
         */
        p = (NvmPair *) calloc(1, sizeof(NvmPair));
        if (p == NULL)
        {
            return FALSE;
        }

        p->name = strdup(name);
        if (p->name == NULL)
        {
            free(p);
            return FALSE;
        }

        p->value = strdup(value);
        if (p->value == NULL)
        {
            free(p->name);
            free(p);
            return FALSE;
        }
        p->vlen = strlen(p->value) + 1;
        p->next = h->nvm_list;
        h->nvm_list = p;
    }
    else
    {
        /*
         * There is a previously allocated buffer for the NVRAM configuration.
         * Compare the value and update the buffer in hash table if needed.
         */
        if (strcmp(p->value, value) != 0)
        {
            size_t len;
            len = strlen(value);
            if (len + 1 <= p->vlen)
            {
                strcpy(p->value, value);
            }
            else
            {
                char *c = strdup(value);
                if (c != NULL)
                {
                    move_to_deadlist(p->value);
                    p->value = c;
                    p->vlen = len + 1;
                }
            }
        }
    }
    return TRUE;
}

static int move_to_deadlist(char *value)
{
    Buffer *b = (Buffer *) calloc(1, sizeof(Buffer));
    if (b == NULL)
    {
        return -1;
    }

    b->value = value;
    if (dead_buffers_list == NULL)
    {
        dead_buffers_list = b;
    }
    else
    {
        b->next = dead_buffers_list;
        dead_buffers_list = b;
    }
    return 0;
}
