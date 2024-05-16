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
 * \file dm_trans.c
 *
 * Implements data model transactions
 *
 * Implements data model transactions
 * These transactions wrap the underlying database and also call notifies when the transaction has been committed
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_defs.h"
#include "dm_trans.h"
#include "database.h"
#include "device.h"
#include "dm_inst_vector.h"
#include "vendor_api.h"



//--------------------------------------------------------------------
// Current transaction to add operations to
static dm_trans_vector_t *cur_transaction = NULL;

//--------------------------------------------------------------------
// Array used to convert an enumeration to a string, for debug purposes
static const char *op_to_str[kDMOp_Max] =
{
    "SetParameterValues",       // kDMOp_Set
    "AddObjectInstance"         // kDMOp_Add
    "DeleteObjectInstance"      // kDMOp_Del
};

//--------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void ClearTransaction(dm_trans_vector_t *trans);

/*********************************************************************//**
**
** DM_TRANS_Start
**
** Starts a transaction for subsequent data model operations
**
** \param   trans - pointer to vector to use when building up the transaction
**                  All operations in the transaction will be recorded in this vector
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_TRANS_Start(dm_trans_vector_t *trans)
{
    int err;
    dm_vendor_start_trans_cb_t   start_trans_cb;

    // Initialise the vector of operations to notify
    trans->num_entries = 0;
    trans->vector = NULL;

    // Save this vector - it will be used when adding all subsequent operations
    USP_ASSERT(cur_transaction == NULL);
    cur_transaction = trans;

    // Exit if unable to start a database transaction
    err = DATABASE_StartTransaction();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to start the vendor's database transaction (if registered)
    start_trans_cb = vendor_hook_callbacks.start_trans_cb;
    if (start_trans_cb != NULL)
    {
        err = start_trans_cb();
        if (err != USP_ERR_OK)
        {
            DM_TRANS_Abort();
            return err;
        }
    }


    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_TRANS_Add
**
** Adds an operation to the current transaction
**
** \param   op - operation (e.g. add, delete, set)
** \param   path - pointer to full data model path to parameter or object
** \param   value - pointer to string containing the value which was set
** \param   val_union - pointer to union containing the value converted to its native type
** \param   node - pointer to node in data model representing this parameter
** \param   inst - pointer to parsed instance numbers for the object and it's parents
**
** \return  None
**
**************************************************************************/
void DM_TRANS_Add(dm_op_t op, char *path, char *value, dm_val_union_t *val_union, dm_node_t *node, dm_instances_t *inst)
{
    int new_num_entries;
    dm_trans_t *dt;
    int i;

    USP_ASSERT(cur_transaction != NULL);

    // Do not add set operations, if they are part of a larger add operation - we only want to notify the add
    // NOTE: When processing a USP AddRequest message, default values are not added to the transaction, only the overridden default values (in the USP AddRequest message)
    if (op == kDMOp_Set)
    {
        // Iterate over all operations, seeing if any were an add operation
        for (i=0; i < cur_transaction->num_entries; i++)
        {
            dt = &cur_transaction->vector[i];
            if (dt->op == kDMOp_Add)
            {
                if (inst->order >= dt->inst.order)
                {
                    // If this set is for a parameter whose parent instances match that of the add, then do not add to transaction
                    if ( (memcmp(inst->nodes, dt->node->instance_nodes, (dt->inst.order)*sizeof(dm_node_t *)) == 0) &&
                         (memcmp(inst->instances, dt->inst.instances, (dt->inst.order)*sizeof(int)) == 0) )
                    {
                        return;
                    }
                }
            }
        }
    }

    // For us to detect that a delete operation matches a resolved path, we need to resolve the list of
    // ObjectDeletion notify objects, whilst the objects are still in the data model
    if (op == kDMOp_Del)
    {
        DEVICE_SUBSCRIPTION_ResolveObjectDeletionPaths();
    }

    // Increase the size of the current transaction vector
    new_num_entries = cur_transaction->num_entries + 1;
    cur_transaction->vector = USP_REALLOC(cur_transaction->vector, new_num_entries * sizeof(dm_trans_t));

    // And store this operation
    dt = &cur_transaction->vector[ new_num_entries-1 ];
    dt->op = op;
    dt->path = USP_STRDUP(path);
    dt->node = node;
    memcpy(&dt->inst, inst, sizeof(dm_req_instances_t));

    // Save a copy of the value (if applicable to the operation being stored)
    if (value != NULL)
    {
        USP_ASSERT(val_union != NULL);
        USP_ASSERT(IsParam(node)==true);

        // Save the string and native form of the value
        dt->value = USP_STRDUP(value);
        dt->val_union = *val_union;
    }
    else
    {
        dt->value = NULL;
        memset(&dt->val_union, 0, sizeof(dt->val_union));
    }

    cur_transaction->num_entries = new_num_entries;
}

/*********************************************************************//**
**
** DM_TRANS_Commit
**
** Commits the current transaction, calling all data model notify functions
** If the commit fails, it will be aborted within this function.
** NOTE: Notify functions may themselves alter the database and cause other notify functions to be called (cascading)
**       This is handled by this function being recursive. Errors from notify functions and cascaded commits are not returned.
**
** \param   None
**
** \return  USP_ERR_OK if the database commit itself was successful. Errors from callbacks and cascaded commits do not return errors.
**          An error is only returned if the commit itself fails.
**
**************************************************************************/
int DM_TRANS_Commit(void)
{
    int i;
    int err;
    dm_trans_vector_t *trans;
    dm_trans_vector_t cascade_trans;
    dm_vendor_commit_trans_cb_t   commit_trans_cb;

    USP_ASSERT(cur_transaction != NULL);


    // Exit if unable to commit the vendor's database successfully, aborting the transaction
    commit_trans_cb = vendor_hook_callbacks.commit_trans_cb;
    if (commit_trans_cb != NULL)
    {
        err = commit_trans_cb();
        if (err != USP_ERR_OK)
        {
            DM_TRANS_Abort();
            return err;
        }
    }

    // Exit if unable to commit the database successfully, aborting the transaction
    err = DATABASE_CommitTransaction();
    if (err != USP_ERR_OK)
    {
        DM_TRANS_Abort();
        return err;
    }

    // From now on, errors are not reported back to the caller, as the commit itself has happened successfully

    // Exit if there are no operations in the transaction
    if (cur_transaction->num_entries == 0)
    {
        cur_transaction = NULL;
        return USP_ERR_OK;
    }

    // This code needs to cope with the fact that notify functions may themselves need to alter the data model/database
    // Hence we need to cope with cascading, and we do this by starting a new transaction for the notify functions

    trans = cur_transaction;            // Save a copy of the current transaction as we will be starting a new transaction to wrap the notifies
    cur_transaction = NULL;
    DM_TRANS_Start(&cascade_trans);     // Any operations performed by the notifies will be captured and committed in the cascade_trans transaction

    // Call the notify function for all operations
    for (i=0; i < trans->num_entries; i++)
    {
        // Declare local variables here, so that they are not filling up the stack if this function is called recursively
        dm_node_t *node;
        dm_notify_set_cb_t notify_set_cb;
        dm_notify_add_cb_t notify_add_cb;
        dm_notify_add_cb_t notify_del_cb;
        dm_req_t req;
        dm_trans_t *dt;

        dt = &trans->vector[i];
        node = dt->node;

        USP_ERR_ClearMessage();

        switch(dt->op)
        {
            case kDMOp_Set:
                // Internal notify callback
                notify_set_cb = node->registered.param_info.notify_set_cb;
                if (notify_set_cb != NULL)
                {
                    DM_PRIV_RequestInit(&req, node, dt->path, (dm_instances_t *) &dt->inst);
                    req.val_union = dt->val_union;
                    err = notify_set_cb(&req, dt->value);
                }
                break;

            case kDMOp_Add:
                // Internal notify callback
                notify_add_cb = node->registered.object_info.notify_add_cb;
                if (notify_add_cb != NULL)
                {
                    DM_PRIV_RequestInit(&req, node, dt->path, (dm_instances_t *) &dt->inst);
                    err = notify_add_cb(&req);
                }

                // Notify external controllers
                DEVICE_SUBSCRIPTION_NotifyObjectLifeEvent(dt->path, kSubNotifyType_ObjectCreation);
                break;

            case kDMOp_Del:
                // Internal notify callback
                notify_del_cb = node->registered.object_info.notify_del_cb;
                if (notify_del_cb != NULL)
                {
                    DM_PRIV_RequestInit(&req, node, dt->path, (dm_instances_t *) &dt->inst);
                    err = notify_del_cb(&req);
                }

                // Notify external controllers
                DEVICE_SUBSCRIPTION_NotifyObjectLifeEvent(dt->path, kSubNotifyType_ObjectDeletion);
                break;

            default:
                TERMINATE_BAD_CASE(dt->op);
                break;
        }

        // Continue calling all notifies, even if one returned an error
        if (err != USP_ERR_OK)
        {
            USP_ERR_ReplaceEmptyMessage("%s: Notify callback for path %s (%s operation) returned error %d", __FUNCTION__, dt->path, op_to_str[dt->op], err);
        }
    }

    // Clear the transaction being committed
    ClearTransaction(trans);

    // Commit the cascaded transaction
    // NOTE: This is recursive
    // NOTE: We do not return any errors from this, as the original commit has happened successfully
    DM_TRANS_Commit();

    USP_ASSERT(cur_transaction == NULL);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_TRANS_Abort
**
** Aborts the current transaction
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_TRANS_Abort(void)
{
    int err;
    int i;
    dm_trans_t *dt;
    dm_instances_t inst;
    dm_node_t *node;
    dm_vendor_abort_trans_cb_t   abort_trans_cb;

    // Exit if no tranasaction to abort
    // This situation could occur if DM_TRANS_Commit() fails and internally calls DM_TRANS_Abort(),
    // then the caller of DM_TRANS_Commit() also calls DM_TRANS_Abort()
    if (cur_transaction == NULL)
    {
        return USP_ERR_OK;
    }

    // Remove all instance add operations which have been aborted from the data model

    // Iterate over all transactions which have been aborted
    for (i=0; i < cur_transaction->num_entries; i++)
    {
        dt = &cur_transaction->vector[i];

        // If the aborted operation was an Add or Delete, then we need to undo the operation in the instance vector
        if ((dt->op == kDMOp_Add) || (dt->op == kDMOp_Del))
        {
            node = dt->node;
            USP_ASSERT(node != NULL);

            // Form object instances array
            memset(&inst, 0, sizeof(inst));
            memcpy(&inst, &dt->inst, sizeof(dt->inst));
            memcpy(&inst.nodes, &node->instance_nodes, node->order*sizeof(dm_node_t *));

            if (dt->op == kDMOp_Add)
            {
                // Remove an aborted added object
                DM_INST_VECTOR_Remove(&inst);
            }
            else if (dt->op == kDMOp_Del)
            {
                // Add back an aborted deleted object
                DM_INST_VECTOR_Add(&inst);
            }
        }
    }

    // Empty the pending notify queue, freeing the entries
    ClearTransaction(cur_transaction);

    cur_transaction = NULL;


    // Exit if unable to abort the vendor's database transaction (if registered)
    abort_trans_cb = vendor_hook_callbacks.abort_trans_cb;
    if (abort_trans_cb != NULL)
    {
        err = abort_trans_cb();
        if (err != USP_ERR_OK)
        {
            DATABASE_AbortTransaction();
            return err;
        }
    }

    // Exit if unable to abort our database's transaction
    err = DATABASE_AbortTransaction();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_TRANS_IsWithinTransaction
**
** Returns whether we are currently operating within a transaction or not
**
** \param   None
**
** \return  true if we are currently operating within a transaction
**
**************************************************************************/
bool DM_TRANS_IsWithinTransaction(void)
{
    if (cur_transaction != NULL)
    {
        return true;
    }

    return false;
}


/*********************************************************************//**
**
** ClearTransaction
**
** Clears the queue of operations in the transaction
**
** \param   trans - dm_trans_vector to free all memory in
**
** \return  None
**
**************************************************************************/
void ClearTransaction(dm_trans_vector_t *trans)
{
    int i;
    dm_trans_t *dt;

    // Exit if nothing to do
    if (trans->vector == NULL)
    {
        goto exit;
    }

    // Free all dynamically allocated strings referenced by the vector
    for (i=0; i < trans->num_entries; i++)
    {
        dt = &trans->vector[i];
        USP_FREE(dt->path);
        USP_SAFE_FREE(dt->value);
        // NOTE: There is nothiing to free in dt->val_union. If it points to a string, the string is dt->value, and hence has already been freed
    }

    USP_FREE(trans->vector);

exit:
    // Ensure queue is re-initialised to empty state
    trans->vector = NULL;
    trans->num_entries = 0;
}


























