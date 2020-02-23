/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
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
 */


/*******************************************************************
 * bdmf_type.c
 *
 * Data path builder - device type hadling code
 *
 * This file is Copyright (c) 2011, Broadlight Communications.
 * This file is licensed under GNU Public License, except that if
 * you have entered in to a signed, written license agreement with
 * Broadlight covering this file, that agreement applies to this
 * file instead of the GNU Public License.
 *
 * This file is free software: you can redistribute and/or modify it
 * under the terms of the GNU Public License, Version 2, as published
 * by the Free Software Foundation, unless a different license
 * applies as provided above.
 *
 * This program is distributed in the hope that it will be useful,
 * but AS-IS and WITHOUT ANY WARRANTY; without even the implied
 * warranties of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * TITLE or NONINFRINGEMENT. Redistribution, except as permitted by
 * the GNU Public License or another license agreement between you
 * and Broadlight, is prohibited.
 *
 * You should have received a copy of the GNU Public License,
 * Version 2 along with this file; if not, see
 * <http://www.gnu.org/licenses>.
 *
 * Author: Igor Ternovsky
 *******************************************************************/

#include <bdmf_dev.h>

static TAILQ_HEAD(type_list, bdmf_type) bdmf_drv_list = TAILQ_HEAD_INITIALIZER(bdmf_drv_list);
static DEFINE_BDMF_FASTLOCK(drv_lock);

static void _bdmf_type_dec_inuse(struct bdmf_type *drv);

static void _bdmf_type_inc_inuse(struct bdmf_type *drv)
{
    bdmf_fastlock_lock(&drv->lock);
    ++drv->usecount;
    BDMF_TRACE_DBG("%s - %d\n", drv->name, drv->usecount);
    bdmf_fastlock_unlock(&drv->lock);
}

static void _bdmf_type_dec_inuse(struct bdmf_type *drv)
{
    struct bdmf_attr *a;

    bdmf_fastlock_lock(&drv->lock);
    --drv->usecount;
    BDMF_TRACE_DBG("%s - %d\n", drv->name, drv->usecount);
    if (drv->usecount)
    {
        bdmf_fastlock_unlock(&drv->lock);
        return;
    }
    /* Usecount is 0. remove */
    TAILQ_REMOVE(&bdmf_drv_list, drv, types_list);
    bdmf_fastlock_unlock(&drv->lock);

    a=drv->aattr;
    if (a)
    {
        while(a->name)
        {
            bdmf_attr_unmake(a);
            ++a;
        }
    }

    if (drv->po)
        _bdmf_type_dec_inuse(drv->po);
}


/*
 * Type control
 */
int bdmf_type_register_ext(struct bdmf_type *root, struct bdmf_type *drv)
{
    struct bdmf_attr *a;
    int rc;

    /* Optional driver init */
    if (drv->drv_init)
    {
        rc = drv->drv_init(drv);
        if (rc)
            return rc;
    }

    if (!drv->name)
        return BDMF_ERR_PARM;

    /* Set trace based on the global trace level if not set explicitly in the driver structure */
    if (drv->trace_level == bdmf_trace_level_none)
        drv->trace_level = bdmf_trace_level(NULL);

    /* Root type is the default */
    if (drv != root)
    {
        if (!drv->parent)
            drv->parent = root->name;

        /* Identify parent */
        rc = bdmf_type_find_get(drv->parent, &drv->po);
        if (rc)
        {
            BDMF_TRACE_ERR_DRV(drv, "%s: parent type %s is not registered\n",
                           drv->name, drv->parent);
            return rc;
        }
    }

    /* Verify and finalise attributes */
    a = drv->aattr;
    if (a)
    {
        while(a->name)
        {
            /* Fill-in default callbacks */
            rc = bdmf_attr_make(drv, a);
            if (rc)
                goto cleanup;
            ++a;
        }
    }
    drv->magic = BDMF_TYPE_MAGIC;
    drv->nattrs = (a - drv->aattr);
    drv->usecount = 1;
    drv->extra_size = (drv->extra_size+sizeof(long)-1) & ~(sizeof(long)-1);
    drv->seg_size[0] = drv->extra_size;
    DLIST_INIT(&drv->obj_list);
    bdmf_fastlock_init(&drv->lock);

    bdmf_fastlock_lock(&drv_lock);
    TAILQ_INSERT_TAIL(&bdmf_drv_list, drv, types_list);
    bdmf_fastlock_unlock(&drv_lock);

    return 0;

cleanup:
    {
        struct bdmf_attr *a1=drv->aattr;
        if (a1)
        {
            while(a1->name && a1 != a)
            {
                bdmf_attr_unmake(a1);
                ++a1;
            }
        }
        if (drv->po)
            bdmf_type_put(drv->po);
    }
    return rc;
}


int bdmf_type_register(struct bdmf_type *drv)
{
    return bdmf_type_register_ext(bdmf_root_type, drv);
}

void bdmf_type_unregister(struct bdmf_type *drv)
{
    struct bdmf_object *mo, *mo_tmp;

    if (!drv || !drv->usecount)
        return;

    DLIST_FOREACH_SAFE(mo, &drv->obj_list, siblings, mo_tmp)
        bdmf_destroy(mo);

    /* Optional driver cleanup */
    if (drv->drv_exit)
        drv->drv_exit(drv);

    _bdmf_type_dec_inuse(drv);
}

/* returns 1 if type is visible */
static inline int bdmf_type_check_visibility(struct bdmf_type *drv)
{
    return 1;
}

/** Managed object type iterator.
 * The function returns the first or the next registered object type handle.
 *
 * The handle returned by bdmf_type_get_next function must be
 * released by passing it as a parameter to bdmf_type_get_next or bdmf_type_put()
 *
 * \param[in]   drv     previous type handle. NULL=get first
 * \return
 *     next type handle or NULL if end of list is reached.\n
 */
bdmf_type_handle bdmf_type_get_next(bdmf_type_handle drv)
{
    struct bdmf_type *d;

    bdmf_fastlock_lock(&drv_lock);
    if (drv)
    {
        d = TAILQ_NEXT(drv, types_list);
        _bdmf_type_dec_inuse(drv);
    }
    else
        d = TAILQ_FIRST(&bdmf_drv_list);
    while(d && !bdmf_type_check_visibility(d))
        d = TAILQ_NEXT(d, types_list);
    bdmf_fastlock_unlock(&drv_lock);

    if (!d)
        return NULL;

    _bdmf_type_inc_inuse(d);

    return d;
}


/** Get managed object type handle given its name
 *
 * The handle must be released using bdmf_put() function.
 *
 * \param[in]   name    Managed object type name. 
 * \param[out]  pdrv    type handle or NULL if object type with the requested name is not found.\n
 *                      It is guaranteed that *drv contains valid handle if bdmf_type_find_get function returns 0.
 * \return
 *     0          - OK\n
 *    <0          - error\n
 */
int bdmf_type_find_get(const char *name, bdmf_type_handle *pdrv)
{
    struct bdmf_type *drv = NULL;
    const char *pname = name;

    if (!name)
        return BDMF_ERR_PARM;
    while((drv=bdmf_type_get_next(drv)))
    {
        if (!strcmp(drv->name, pname))
        {
            *pdrv = drv;
            return 0;
        }
    }
    return BDMF_ERR_NODEV;
}


/** Lock managed object type handle
 *
 * The handle must be released using bdmf_put() function.
 *
 * \param[in]   drv     Managed object type handle
 */
void bdmf_type_get(bdmf_type_handle drv)
{
    _bdmf_type_inc_inuse(drv);
}


/** Release managed object type handle.
 * The function "unlocks" the handle by decrementing usecount in the
 * underlying structure. Following this call the handle may become invalid.
 *
 * \param[in]   drv     Managed object type handle
 * \return
 *     void
 */
void bdmf_type_put(bdmf_type_handle drv)
{
    /* ignore temp types created on stack */
    if (!drv->types_list.tqe_prev)
        return;
    _bdmf_type_dec_inuse(drv);
}


/** Get managed object type info
 *
 * \param[in]   drv     Managed object type handle
 * \param[out]  info    Managed object type info
 * \return
 *     0          - OK\n
 *    <0          - error\n
 */
int bdmf_type_info(bdmf_type_handle drv, bdmf_type_info_t *info)
{
    if (!info || !drv)
        return BDMF_ERR_PARM;
    strncpy(info->name, drv->name, sizeof(info->name) - 1);
    strncpy(info->help, drv->description, sizeof(info->help) - 1);
    info->n_attrs = drv->nattrs;
    info->n_objects = drv->nobjs;
    return 0;
}

/** Get number of attributes supported by managed object type */
int bdmf_type_num_attrs(bdmf_type_handle drv)
{
    if (!drv)
        return 0;
    return drv->nattrs;
}

/** find object by discriminator string */
static int root_object_get(struct bdmf_type *drv,
           struct bdmf_object *owner, const char *discr,
           struct bdmf_object **pmo)
{
    *pmo = bdmf_root_object;
    return 0;
}

static struct bdmf_type root_type = {
    .name = "root",
    .description = "Dummy root type",
    .get = root_object_get,
    .max_objs = 1,
    .flags = BDMF_DRV_FLAG_ROOT
};
struct bdmf_type *bdmf_root_type = &root_type;


/*
 * Global lock support
 */

/* static bdmf_ta_mutex bdmf_global_lock; */
static bdmf_reent_fastlock bdmf_global_lock;

/** Acquire global lock.
 * The functions takes ownership of global recursive mutex.
 * Typically it is used to protect multiple operations in the same transaction.
 * For example, read-modify-write. \n
 * There is no need to check return code if calling task doesn't expect signals.
 * The global lock is recursive. That is,
 * - a task can take the lock it owns multiple times.
 * - if task B tries to take the lock held by task A - it blocks
 * - lock can only be released by the task that owns it
 * \return  0 - OK \n
 *          BDMF_ERR_INTR - interrupted by signal
 */
int bdmf_lock(void)
{
/*    return bdmf_ta_mutex_lock(&bdmf_global_lock); */
    return bdmf_reent_fastlock_lock(&bdmf_global_lock);
}

/** Release global lock.
 * Release global lock acquired by bdmf_lock() call.
 */
void bdmf_unlock(void)
{
/*    bdmf_ta_mutex_unlock(&bdmf_global_lock); */
    return bdmf_reent_fastlock_unlock(&bdmf_global_lock);
}


int bdmf_type_module_init(void)
{
    TAILQ_INIT(&bdmf_drv_list);
/*    bdmf_ta_mutex_init(&bdmf_global_lock); */
    bdmf_reent_fastlock_init(&bdmf_global_lock);
    return bdmf_type_register(&root_type);
}

void bdmf_type_module_exit(void)
{
    struct bdmf_type *d, *d_tmp;
    TAILQ_FOREACH_SAFE(d, &bdmf_drv_list, types_list, d_tmp)
        bdmf_type_unregister(d);
}

/*
 * Exports
 */
EXPORT_SYMBOL(bdmf_type_register_ext);
EXPORT_SYMBOL(bdmf_type_register);
EXPORT_SYMBOL(bdmf_type_unregister);
EXPORT_SYMBOL(bdmf_type_get_next);
EXPORT_SYMBOL(bdmf_type_find_get);
EXPORT_SYMBOL(bdmf_type_get);
EXPORT_SYMBOL(bdmf_type_put);
EXPORT_SYMBOL(bdmf_type_info);
EXPORT_SYMBOL(bdmf_type_num_attrs);
EXPORT_SYMBOL(bdmf_lock);
EXPORT_SYMBOL(bdmf_unlock);
