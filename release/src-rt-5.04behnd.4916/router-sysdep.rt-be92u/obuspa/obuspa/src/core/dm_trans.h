/*
 *
 * Copyright (C) 2019, Broadband Forum
 * Copyright (C) 2016-2019  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file dm_trans.h
 *
 * Implements data model transactions
 * These transactions wrap the underlying database and also call notifies whwen the transaction has been comitted
 *
 */

#ifndef DM_TRANS_H
#define DM_TRANS_H

#include "data_model.h"

//-----------------------------------------------------------------------
// Enumeration for type of operation that the Data model supports
typedef enum
{
    kDMOp_Set = 0,
    kDMOp_Add,
    kDMOp_Del,

    // Next entry is always the last - used to size arrays
    kDMOp_Max
} dm_op_t;

//-----------------------------------------------------------------------
// Queue of parameters or objects whose notify function will be called when the transaction has been committed successfully
typedef struct
{
    dm_op_t op;         // Operation (add/delete/set)
    char *path;         // full data model path to parameter or objet to notify
    char *value;        // value of parameter (only used by kTransType_Set)
    dm_node_t *node;    // Node in the data model representing the parameter or object
    dm_req_instances_t inst;    // Instance array containing the instance numbers in the path
    dm_val_union_t val_union;  // Stores the native value of the parameter (only used by kTransType_Set). If the parameter is a string, then it will point to the 'value' parameter in this structure
} dm_trans_t;

//-----------------------------------------------------------------------
// Vector storing all operations made during a transaction
typedef struct
{
    int num_entries;
    dm_trans_t *vector;
} dm_trans_vector_t;

//-----------------------------------------------------------------------------------------
// API
int DM_TRANS_Start(dm_trans_vector_t *trans);
void DM_TRANS_Add(dm_op_t op, char *path, char *value, dm_val_union_t *val_union, dm_node_t *node, dm_instances_t *inst);
int DM_TRANS_Commit(void);
int DM_TRANS_Abort(void);
bool DM_TRANS_IsWithinTransaction(void);

#endif
