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
 * bdmf_object.c
 *
 * Broadlight Device Management Framework - object handling
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

/* #define DEBUG */

#include <bdmf_dev.h>

static void _bdmf_unlink(struct bdmf_object *ds, struct bdmf_object *us, struct bdmf_link *link);

void _bdmf_ref_free(struct bdmf_ref *ref)
{
    bdmf_fastlock_lock(&ref->client->drv->lock);
    DLIST_REMOVE(ref, ref_list);
    DLIST_REMOVE(ref, client_list);
    bdmf_fastlock_unlock(&ref->client->drv->lock);
    if (ref->client->drv->ref_changed)
    {
        bdmf_fastlock_lock(&ref->ref_obj->drv->lock);
        --ref->ref_obj->num_ref_notify_change;
        bdmf_fastlock_unlock(&ref->ref_obj->drv->lock);
    }
    bdmf_put(ref->ref_obj);
    bdmf_free(ref);
}

struct bdmf_ref *_bdmf_ref_alloc(struct bdmf_object *mo, struct bdmf_object *ref_obj, struct bdmf_attr *attr)
{
    struct bdmf_ref *ref;
    uint32_t size = sizeof(struct bdmf_ref);
    int is_numeric = (attr==NULL) || bdmf_attr_type_is_numeric(attr->index_type);
    if (!is_numeric)
        size += attr->index_size;
    ref = bdmf_calloc(size);
    if (!ref)
        return NULL;
    ref->client = mo;
    ref->ref_obj = ref_obj;
    if (!is_numeric)
        ref->attr_index = (bdmf_index)(ref + 1);
    bdmf_get(ref_obj);
    DLIST_INSERT_HEAD(&mo->references, ref, ref_list);
    DLIST_INSERT_HEAD(&ref_obj->clients, ref, client_list);
    if (mo->drv->ref_changed)
    {
        bdmf_fastlock_lock(&ref_obj->drv->lock);
        ++ref_obj->num_ref_notify_change;
        bdmf_fastlock_unlock(&ref_obj->drv->lock);
    }
    return ref;
}

struct bdmf_ref *_bdmf_ref_find_by_attr(struct bdmf_object *mo, struct bdmf_attr *attr, bdmf_index index, int offset)
{
    struct bdmf_ref *ref, *ref_tmp;
    int is_numeric = bdmf_attr_type_is_numeric(attr->index_type);

    DLIST_FOREACH_SAFE(ref, &mo->references, ref_list, ref_tmp)
    {
        if (ref->attr != attr || ref->attr_offset != offset)
            continue;
        if (is_numeric)
        {
            if (ref->attr_index == index)
                break;
        }
        else if (index)
        {
            if (!memcmp((void *)ref->attr_index, (void *)index, attr->index_size))
                break;
        }
    }
    return ref;
}

/* Notify client object that the object it is referencing
 * is being destroyed
 */
static void _bdmf_ref_notify_destroy(struct bdmf_ref *ref)
{
    BUG_ON(!ref->client || !ref->ref_obj);
    /* Inform client that referenced object has been removed */
    _bdmf_ref_attr_clear(ref);
    /* Notify via common callback. It takes care of references
     * created explicitly, rather than via attributes
     */
    if (ref->client->drv->ref_destroy)
        ref->client->drv->ref_destroy(ref->client, ref->ref_obj);
    _bdmf_ref_free(ref);
}

/* Nitify all client objects that referenced object
 * has been changed
 */
void _bdmf_ref_notify_change(struct bdmf_object *mo)
{
    struct bdmf_ref *ref, *ref_tmp;

    /* If there are no clients interested to know that *this* object has changed
     * - just return */
    if (!mo->num_ref_notify_change)
        return;

    DLIST_FOREACH_SAFE(ref, &mo->clients, client_list, ref_tmp)
    {
        if (ref->client->drv->ref_changed)
        {
            ref->client->drv->ref_changed(ref->client, mo, ref->attr,
                ref->attr_index, ref->attr_offset);
        }
    }
}

static void _bdmf_obj_destroy(struct bdmf_object *mo)
{
    struct bdmf_type *drv = mo->drv;
    struct bdmf_ref *ref, *ref_tmp;
    struct bdmf_object *owner;
    int i;

    BUG_ON(!DLIST_EMPTY(&mo->children));
    if (mo->drv->destroy)
        mo->drv->destroy(mo);
    owner = mo->owner;
    if (owner)
    {
        if (owner->drv->child_destroy)
            owner->drv->child_destroy(owner, mo);
        bdmf_put(owner);
    }
    /* remove from various lists under lock */
    bdmf_fastlock_lock(&drv->lock);
    if (mo->owner)
        DLIST_REMOVE(mo, osiblings);
    --drv->nobjs;
    bdmf_fastlock_unlock(&drv->lock);
    /* Object is about to finally die. If it is still referencing other objects
     * - decrement reference count of those objects
     */
    DLIST_FOREACH_SAFE(ref, &mo->references, ref_list, ref_tmp)
        _bdmf_ref_free(ref);
    for(i=0; i<BDMF_MAX_MEM_SEGS; i++)
    {
        if (drv->seg_auto_alloc[i] && mo->mem_seg_base[i])
            bdmf_mem_free(drv->seg_type[i], mo->mem_seg_base[i]);
    }
    bdmf_free((void *)((unsigned long)mo-mo->drv->extra_size));
    bdmf_type_put(drv);
}


/** Lock managed object, by increasing its use count.
 *
 * An object cannot be destroyed unless its use count is zero.
 * When no longer needed, the managed object handle must be released by bdmf_put() function.
 *
 * \param[in]   mo      Managed object handle
 * \return
 *     void
 */
void bdmf_get(bdmf_object_handle mo)
{
    bdmf_fastlock_lock(&mo->drv->lock);
    ++mo->usecount;
    BDMF_TRACE_DBG("%s - %d\n", mo->name, mo->usecount);
    bdmf_fastlock_unlock(&mo->drv->lock);
}


/** Release managed object handle locked by one of bdmf_get(),
 * bdmf_find_get(), bdmf_find_get_by_name(), bdmf_get_next()
 *
 * When usecount reaches 0, active object is destroyed.
 * Following bdmf_put call the object handle can become invalid.
 *
 * \param[in]   mo      Managed object handle
 */
void bdmf_put(bdmf_object_handle mo)
{
    bdmf_fastlock_lock(&mo->drv->lock);
    BUG_ON(!mo->usecount);
    --mo->usecount;
    BDMF_TRACE_DBG("%s - %d\n", mo->name, mo->usecount);
    if (!mo->usecount)
    {
        if (mo->state == bdmf_state_active)
        {
            /* bump usecount back because bdmf_destroy will call bdmf_put() again */
            ++mo->usecount;
            bdmf_fastlock_unlock(&mo->drv->lock);
            bdmf_destroy(mo);
        }
        else
        {
            bdmf_fastlock_unlock(&mo->drv->lock);
            /* Object was destroyed earlier. Now it is safe to finish cleaning up and release memory */
            _bdmf_obj_destroy(mo);
        }
    }
    else
        bdmf_fastlock_unlock(&mo->drv->lock);
}


/** Allocate and initialise object structure
 * \param[in]   drv         Management object type
 * \return  new object pointer
 */
struct bdmf_object *bdmf_object_alloc(struct bdmf_type *drv)
{
    void *alloc_ptr;
    uint32_t size;
    struct bdmf_object *mo;

    size = sizeof(struct bdmf_object);
    alloc_ptr = bdmf_calloc(size+drv->extra_size);
    if (!alloc_ptr)
        return NULL;
    mo = (struct bdmf_object *)((unsigned long)alloc_ptr+drv->extra_size);
    ++drv->usecount;
    ++drv->nobjs;
    mo->drv = drv;
    mo->usecount = 1;
    mo->state = bdmf_state_init;
    DLIST_INIT(&mo->us_links);
    DLIST_INIT(&mo->ds_links);
    DLIST_INIT(&mo->clients);
    DLIST_INIT(&mo->references);
    DLIST_INIT(&mo->children);
    mo->magic = BDMF_OBJECT_MAGIC;
    return mo;
}


static int bdmf_new_alloc(bdmf_type_handle drv,
             bdmf_object_handle owner, const char *attr,
             bdmf_object_handle *pmo)
{
    struct bdmf_object *mo;
    int i;
    int rc=0;

    if (bdmf_session_access_right(NULL) == BDMF_ACCESS_GUEST)
        return BDMF_ERR_PERM;
    if (drv->max_objs && drv->nobjs >= drv->max_objs)
        return BDMF_ERR_TOO_MANY;
    if (owner)
        bdmf_get(owner);
    /* If port object is not specified - try to find it */
    if (drv->po && !owner && bdmf_find_get(drv->po, NULL, attr, &owner))
    {
        BDMF_TRACE_ERR("parent for object of type %s is not found\n", drv->name);
        return BDMF_ERR_NODEV;
    }

    mo = bdmf_object_alloc(drv);
    if (!mo)
    {
        if (owner)
            bdmf_put(owner);
        return BDMF_ERR_NOMEM;
    }
    mo->owner = owner;

    /* Segment[0] is mapped to object's private data */
    mo->mem_seg_base[0] = bdmf_obj_data(mo);

    /* Allocate attribute areas */
    for(i=1; i<BDMF_MAX_MEM_SEGS; i++)
    {
        if (drv->seg_auto_alloc[i])
        {
            mo->mem_seg_base[i] = bdmf_mem_alloc(mo, drv->seg_type[i], drv->seg_size[i], 0);
            if (!mo->mem_seg_base[i])
            {
                rc = BDMF_ERR_NOMEM;
                goto out;
            }
            memset(mo->mem_seg_base[i], 0, drv->seg_size[i]);
        }
    }

    /* Pre-init, set attributes, post-init and enable */
    if (drv->pre_init)
        rc = drv->pre_init(mo);

out:
    if (rc && mo)
    {
        bdmf_put(mo);
        mo = NULL;
    }
    *pmo = mo;
    return rc;
}

/** Set/modify object parent (owner)
 * \param[in]   mo      Managed object pointer
 * \param[in]   owner   New owner handle
 */
void bdmf_object_parent_set(struct bdmf_object *mo, struct bdmf_object *parent)
{
    bdmf_object_handle old_owner = mo->owner;
    if (old_owner == parent)
        return;
    bdmf_fastlock_lock(&mo->drv->lock);
    if (old_owner)
        DLIST_REMOVE(mo, osiblings);
    mo->owner = parent;
    if (parent)
        DLIST_INSERT_HEAD(&parent->children, mo, osiblings);
    bdmf_fastlock_unlock(&mo->drv->lock);
    if (old_owner)
        bdmf_put(old_owner);
    if (parent)
        bdmf_get(parent);
}

static int should_lock(bdmf_type_handle drv)
{
    struct bdmf_object *mo;

    if (drv->flags & BDMF_DRV_FLAG_ROOT)
        return 0;

    mo = bdmf_get_next_child(bdmf_root_object, NULL, NULL);

    return mo && mo->state == bdmf_state_active;
}

/** Create a new managed object of the specified type
 *
 * \param[in]   owner   Object owner. Can be NULL
 * \param[in]   drv     Managed object type handle
 * \param[in]   attr    An ASCII string of attribute values to be set for the new object.\n
 *                      The string is comma-delimited list in format name=value.\n
 *                      See attr format description in bdmf_configure()
 * \param[out]  pmo     New managed object handle
 * \return
 *     0      - OK \n
 *    <0      - error
 */
int bdmf_new_and_configure(bdmf_type_handle drv,
             bdmf_object_handle owner, const char *attr,
             bdmf_object_handle *pmo)
{
    struct bdmf_object *mo = NULL;
    int rc, lock;

    lock = should_lock(drv);

    if (lock)
        bdmf_lock();
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_new_and_configure, bdmf_hist_point_start, drv, owner, attr);
    rc = bdmf_new_alloc(drv, owner, attr, &mo);
    if (rc)
        goto cleanup;

    /* Configure and make sure that all mandatory attributes are set */
    rc = bdmf_configure(mo, attr);

    mo->state = bdmf_state_inactive;
    if (drv->post_init)
        rc = rc ? rc : drv->post_init(mo);
    if (rc)
        goto cleanup;

    bdmf_fastlock_lock(&mo->drv->lock);
    DLIST_INSERT_HEAD(&drv->obj_list, mo, siblings);
    if (mo->owner)
        DLIST_INSERT_HEAD(&mo->owner->children, mo, osiblings);
    bdmf_fastlock_unlock(&mo->drv->lock);
    *pmo = mo;
    mo->state = bdmf_state_active;
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_new_and_configure, bdmf_hist_point_end, rc, mo);
    if (lock)
        bdmf_unlock();

    BDMF_TRACE_INFO_OBJ(mo, "success: config:%s\n", attr?attr:"none");

    return 0;

cleanup:
    if (mo)
        bdmf_destroy(mo);
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_new_and_configure, bdmf_hist_point_end, rc);
    if (lock)
        bdmf_unlock();
    BDMF_TRACE_ERR_DRV(drv, "config:%s  error:%s (%d)\n",
            attr?attr:"none", bdmf_strerror(rc), rc);
    return rc;
}

/** Create a new managed object using parameters in attribute set
 *
 * \param[in]   drv     Managed object type handle
 * \param[in]   owner   Object owner. If NULL, framework tries to identify owner using attributes
 * \param[in]   mattr   attribute set in the same format as in bdmf_mattr_set(). mattr is released automatically.
 * \param[out]  pmo     New managed object handle
 * \return
 *     0      - OK \n
 *    <0      - error
 */
int bdmf_new_and_set(bdmf_type_handle drv,
             bdmf_object_handle owner, bdmf_mattr_handle hmattr,
             bdmf_object_handle *pmo)
{
    const bdmf_mattr_t *mattr = (const bdmf_mattr_t *)hmattr;
    struct bdmf_object *mo = NULL;
    int rc = 0, lock;

    lock = should_lock(drv);

    if (lock)
        bdmf_lock();
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_new_and_set, bdmf_hist_point_start, drv, owner, mattr);
    rc = bdmf_new_alloc(drv, owner, NULL, &mo);
    if (rc)
        goto cleanup;

    /* Configure and make sure that all mandatory attributes are set */
    if (hmattr)
    {
        rc = bdmf_mattr_set(mo, hmattr);
        hmattr = NULL;
    }

    mo->state = bdmf_state_inactive;
    if (drv->post_init)
        rc = rc ? rc : drv->post_init(mo);
    if (rc)
        goto cleanup;

    bdmf_fastlock_lock(&mo->drv->lock);
    DLIST_INSERT_HEAD(&drv->obj_list, mo, siblings);
    if (mo->owner)
        DLIST_INSERT_HEAD(&mo->owner->children, mo, osiblings);
    bdmf_fastlock_unlock(&mo->drv->lock);
    *pmo = mo;
    mo->state = bdmf_state_active;
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_new_and_set, bdmf_hist_point_end, rc, mo);
    if (lock)
        bdmf_unlock();

    BDMF_TRACE_INFO_DRV(drv, "success: object:%s\n", mo->name);

    return 0;

cleanup:
    if (mo)
        bdmf_destroy(mo);
    if (hmattr)
        bdmf_mattr_free(hmattr);
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_new_and_set, bdmf_hist_point_end, rc);
    if (lock)
        bdmf_unlock();
    BDMF_TRACE_ERR_DRV(drv, "error:%s (%d)\n", bdmf_strerror(rc), rc);
    return rc;
}

/** Destroy managed bject
 *
 * \param[out]  mo      Managed object handle
 * \return
 *     0      - OK \n
 *    <0      - error
 */
int bdmf_destroy(bdmf_object_handle mo)
{
    struct bdmf_object *ch, *ch_tmp;
    bdmf_link_handle link;
    struct bdmf_ref *ref, *ref_tmp;

    if (!mo)
        return BDMF_ERR_PARM;

    if (bdmf_session_access_right(NULL) == BDMF_ACCESS_GUEST)
        return BDMF_ERR_PERM;

    bdmf_lock();
    if (mo->state == bdmf_state_deleted)
    {
        bdmf_unlock();
        return BDMF_ERR_ALREADY;
    }

    if (mo->state == bdmf_state_active)
        mo->state = bdmf_state_deleted;

    /* Notify object that it is being destroyed */
    if (mo->drv->pre_destroy)
        mo->drv->pre_destroy(mo);

    /* Remove object from siblings list */
    DLIST_REMOVE(mo, siblings);

    /* Unlink object */
    while ((link = bdmf_get_next_us_link(mo, NULL)))
        _bdmf_unlink(mo, bdmf_us_link_to_object(link), link);

    while ((link = bdmf_get_next_ds_link(mo, NULL)))
        _bdmf_unlink(bdmf_ds_link_to_object(link), mo, link);

    /* Notify all objects that reference *this* object */
    DLIST_FOREACH_SAFE(ref, &mo->clients, client_list, ref_tmp)
        _bdmf_ref_notify_destroy(ref);

    /* Kill children */
    DLIST_FOREACH_SAFE(ch, &mo->children, osiblings, ch_tmp)
        bdmf_destroy(ch);

    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_destroy, bdmf_hist_point_both, mo);

    bdmf_unlock();

    if (!(mo->drv->flags & BDMF_DRV_TMP))
        BDMF_TRACE_INFO_OBJ(mo, "destroyed\n");

    bdmf_put(mo);

    return BDMF_ERR_OK;
}


/* Check attribute values of an object.
 * Returns 1 if all match, 0 - if no match
 */
#define BDMF_ATTR_MAX_VALUE_LENGTH 256
static int _bdmf_check_object_match(struct bdmf_object *mo,
                                    struct bdmf_attr_name_value *apairs, int npairs)
{
    char buffer[BDMF_ATTR_MAX_VALUE_LENGTH];
    int i;
    int rc;
    for(i=0; i<npairs; i++)
    {
        struct bdmf_attr *attr = apairs[i].attr;
        /* Skip elements for which attribute handle is not set */
        if (attr)
        {
            char *index_buf[attr->index_size];
            bdmf_index aindex;
            rc = bdmf_attr_string_to_array_index(mo, attr, apairs[i].array_index, index_buf);
            /* for numerical indexes (the majority of cases) array_index is just a number
             * passed by value. For non-numerical index we pass pointer to the buffer containing index value
             */
            /* Numeric indexes are passed by value, other - by address */
            if (bdmf_attr_type_is_numeric(attr->index_type))
                aindex = *(bdmf_index *)index_buf;
            else
                aindex = (bdmf_index)&index_buf[0];
            rc = (rc < 0) ? rc : _bdmf_attrelem_get_as_string(mo, attr, aindex, buffer, sizeof(buffer));
            if (rc || strcmp(buffer, apairs[i].value))
                return 0;
        }
    }
    /* All good, all match */
    return 1;
}

/* Default function used in bdmf_find_get if type->get callback is not set */
static int bdmf_default_get_cb(struct bdmf_type *drv,
                                struct bdmf_object *owner, const char *discr,
                                struct bdmf_object **pmo, int suppress_trace)
{
    struct bdmf_attr *attr;
    struct bdmf_object *mo, *mo_tmp;
    struct bdmf_attr_name_value *akey_attrs;
    struct bdmf_attr_name_value *aall_attrs=NULL;
    int nkeys=0;
    int key=0;
    int nattrs;
    int rc = BDMF_ERR_NOENT;
    int i, j;
    
    /* If type supports a single object - no trouble finding it */
    if (drv->max_objs == 1)
    {
        mo = DLIST_FIRST(&drv->obj_list);
        if (!mo)
            return BDMF_ERR_NOENT;
        *pmo = mo;
        return 0;
    }

    /* Identify all key attributes */
    attr = &drv->aattr[0];
    while(attr && attr->name)
    {
        if ((attr->flags & BDMF_ATTR_KEY))
            ++nkeys;
        ++attr;
    }
    if (!nkeys)
        return BDMF_ERR_NOENT;
    akey_attrs = bdmf_calloc(sizeof(struct bdmf_attr_name_value)*nkeys);
    if (!akey_attrs)
        return BDMF_ERR_NOMEM;
    attr = &drv->aattr[0];
    while(attr && attr->name && key<nkeys)
    {
        if ((attr->flags & BDMF_ATTR_KEY))
            akey_attrs[key++].attr = attr;
        ++attr;
    }

    /* Split discriminator string to { name, value } pairs */
    rc = bdmf_attr_string_split(discr, &aall_attrs, &nattrs);
    if (rc)
        goto get_cb_done;

    /* Make sure that all key attributes are set */
    for(i=0; i<nkeys; i++)
    {
        const char *name;
        attr = akey_attrs[i].attr;
        name = attr->name;
        for(j=0;
            j<nattrs && strcmp(name, aall_attrs[j].name);
            j++);
        if (j >= nattrs)
        {
            if (!suppress_trace)
                BDMF_TRACE_ERR_DRV(drv, "Can't find object of type %s. key attribute %s is missing\n",
                                    drv->name, name);
            rc = BDMF_ERR_NOENT;
            goto get_cb_done;
        }

        akey_attrs[i].array_index = aall_attrs[j].array_index;
        if (!aall_attrs[j].value)
        {
            if (attr->type == bdmf_attr_enum)
                akey_attrs[i].value = (char *)attr->ts.enum_table->values[0].name; /* 1st enum's value is default */
            else
            {
                BDMF_TRACE_ERR_DRV(drv, "%s: attribute value is missing\n", attr->name);
                rc = BDMF_ERR_PARSE;
                goto get_cb_done;
            }
        }
        else
            akey_attrs[i].value = aall_attrs[j].value;
    }

    /* If there is an owner - scan list of objects owned by the same owner.
     * Otherwise, scan list of all objects of the given type.
     * Look for key match
     */
    rc = BDMF_ERR_NOENT;
    if (owner)
    {
        DLIST_FOREACH_SAFE(mo, &owner->children, osiblings, mo_tmp)
        {
            if (mo->drv==drv && _bdmf_check_object_match(mo, akey_attrs, nkeys))
            {
                rc = 0;
                break;
            }
        }
    }
    else
    {
        DLIST_FOREACH_SAFE(mo, &drv->obj_list, siblings, mo_tmp)
        {
            if (_bdmf_check_object_match(mo, akey_attrs, nkeys))
            {
                rc = 0;
                break;
            }
        }
    }
    if (!rc)
        *pmo = mo;
    
get_cb_done:
    if (aall_attrs)
        bdmf_attr_pairs_release(aall_attrs);
    if (akey_attrs)
        bdmf_free(akey_attrs);

    return rc;
}


/** Find managed object - internal
 *
 * When no longer needed, the managed object handle must be released by bdmf_put() function.
 *
 * \param[in]   drv     Managed object type handle
 * \param[in]   owner   Object's owner (parent). If not set, the framework tries to identify
 *                      the parent using attributes in the discr string.
 * \param[in]   discr   List of attributes with values in the same format as in bdmf_configure()\n
 *                      attrs string is passed transparently to the driver and must be
 *                      sufficient to uniquely identify the object.\n
 *                      discr string can contain also attributes necessary to identify
 *                      object's parent, parent's parent, etc.
 * \param[out]  pmo     Managed object handle
 * \return
 *     0      - OK \n
 *    <0      - error
 */
static int _bdmf_find_get(bdmf_type_handle drv,
                  bdmf_object_handle owner, const char *discr,
                  bdmf_object_handle *pmo, int suppress_trace)
{
    struct bdmf_object *mo;
    int rc;
    int release_owner=0;

    /* Try to identify object's owner if not set.
     * If not found - ignore it and keep looking for the object
     */
    if (drv->po && !owner && !drv->get && !(drv->po->flags & BDMF_DRV_FLAG_ROOT))
    {
        _bdmf_find_get(drv->po, NULL, discr, &owner, 1);
        if (owner)
            release_owner = 1;
    }

    if (drv->get)
        rc = drv->get(drv, owner, discr, &mo);
    else
        rc = bdmf_default_get_cb(drv, owner, discr, &mo, suppress_trace);
    if (release_owner)
        bdmf_put(owner);
    if (rc)
        return rc;

    bdmf_get(mo);
    *pmo = mo;

    return 0;
}


/** Find managed object
 *
 * When no longer needed, the managed object handle must be released by bdmf_put() function.
 *
 * \param[in]   drv     Managed object type handle
 * \param[in]   owner   Object's owner (parent). If not set, the framework tries to identify
 *                      the parent using attributes in the discr string.
 * \param[in]   discr   List of attributes with values in the same format as in bdmf_configure()\n
 *                      attrs string is passed transparently to the driver and must be
 *                      sufficient to uniquely identify the object.\n
 *                      discr string can contain also attributes necessary to identify
 *                      object's parent, parent's parent, etc.
 * \param[out]  pmo     Managed object handle
 * \return
 *     0      - OK \n
 *    <0      - error
 */
int bdmf_find_get(bdmf_type_handle drv,
                  bdmf_object_handle owner, const char *discr,
                  bdmf_object_handle *pmo)
{
    if (!drv || !pmo)
        return BDMF_ERR_PARM;
    return _bdmf_find_get(drv, owner, discr, pmo, 0);
}


/** Managed object iterator.
 *
 * Get next managed object.
 *
 * \param[in]   drv     Managed object type handle
 * \param[in]   mo      Current managed object or NULL
 * \param[in]   filter  Optional filter string in the same format as in bdmf_configure. Can be NULL
 * \return
 *     0         - next object not found\n
 *     otherwise - next object handle
 */
bdmf_object_handle bdmf_get_next(bdmf_type_handle drv,
                                 bdmf_object_handle mo, const char *filter)
{
    struct bdmf_object *mo_next;

    if (drv->get_next)
        mo_next = drv->get_next(drv, mo, filter);
    else
    {
        if (mo)
            mo_next = DLIST_NEXT(mo, siblings);
        else
            mo_next = DLIST_FIRST(&drv->obj_list);

        /* Match against filter if any */
        if (filter && mo_next)
        {
            struct bdmf_attr_name_value *apairs=NULL;
            int npairs;
            int rc;
            int i;
            
            rc = bdmf_attr_string_split(filter, &apairs, &npairs);
            if (rc)
                mo_next = NULL;

            /* Identify all attributes */
            for(i=0; i<npairs; i++)
            {
                bdmf_attr_id aid;
                rc = bdmf_attr_by_name(drv, apairs[i].name, &aid);
                if (rc)
                {
                    mo_next = NULL;
                    break;
                }
                apairs[i].attr = bdmf_aid_to_attr(drv, aid);
            }

            /* Look for object with specified attribute values */
            while(mo_next)
            {
                /* Compare values */
                if (_bdmf_check_object_match(mo_next, apairs, npairs))
                    break;

                mo_next = DLIST_NEXT(mo_next, siblings);;
            }
            if (apairs)
                bdmf_attr_pairs_release(apairs);
        }
    }
    
    if (mo)
        bdmf_put(mo);
    if (mo_next)
        bdmf_get(mo_next);

    return mo_next;
}


/** Child iterator.
 *
 * Get next child.
 *
 * \param[in]   owner   Owner's handle
 * \param[in]   type    Optional type name. If !=NULL, only childs of the given type are iterated.
 * \param[in]   mo      Current managed object or NULL
 * \return
 *     0         - next object not found\n
 *     otherwise - next object handle
 */
bdmf_object_handle bdmf_get_next_child(bdmf_object_handle owner,
                                       const char *type, bdmf_object_handle mo)
{
    struct bdmf_object *child;
    BUG_ON(!owner);

    if (mo)
    {
        child = DLIST_NEXT(mo, osiblings);
        bdmf_put(mo);
    }
    else
        child = DLIST_FIRST(&owner->children);
    while(child)
    {
        /* Break if type filter is not set or matches */
        if (!type ||
            !strcmp(type, child->drv->name))
        {
            bdmf_get(child);
            break;
        }
        child = DLIST_NEXT(child, osiblings);
    }
    return child;
}


/** Get an object help string
 *
 * \param[in]   drv     Managed object type handle
 * \param[in]   what    Optional string identifying help chapter.
 * \param[out]  buffer  Buffer where help info should be stored
 * \param[in]   size    Buffer size
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_help(bdmf_type_handle drv,
              const char *what, char *buffer, uint32_t size)
{
    const char *help=NULL;
    if (!drv || !buffer)
        return BDMF_ERR_PARM;
    if (drv->help)
        help = drv->help(drv, what);
    if (help)
        strncpy(buffer, help, size);
    else
        snprintf(buffer, size, "%s: %s", drv->name, drv->description?drv->description:"");
    if (drv->po)
    {
        uint32_t len=strlen(buffer);
        snprintf(buffer+len, size-len, "\nParent type %s: %s",
                   drv->po->name, drv->po->description?drv->po->description:"");
    }
    return 0;
}

/** Check if objects are linked.
 *
 * One of the objects is a "downstream" object
 * and the other is "upstream".
 *
 * \param[in]   ds      Downstream object handle
 * \param[in]   us      Upstream object handle
 * \param[out]  plink   Optional link pointer.\n
 *                      In case of mux-mux link, link pointer is returned
 *                      
 * \return
 *     1    - link found\n
 *     0    - link not found
 */
int bdmf_is_linked(struct bdmf_object *ds, struct bdmf_object *us, struct bdmf_link **plink)
{
    int rc = 0;
    struct bdmf_link *link, *link_tmp;
    
    bdmf_lock();
    DLIST_FOREACH_SAFE(link, &us->ds_links, usl, link_tmp)
    {
        if (bdmf_ds_link_to_object(link) == ds)
        {
            rc = 1;
            *plink = link;
            break;
        }
    }
    bdmf_unlock();
    return rc;
}

/** Link managed objects together to form a path.
 *
 * One of the objects is a "downstream" object
 * and the other is "upstream".
 *
 * \param[in]   ds      Downstream object handle
 * \param[in]   us      Upstream object handle
 * \param[in]   attrs   Optional attribute string.\n
 *                      This string is passed to objects' link_up, link_down callbacks
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_link(bdmf_object_handle ds, bdmf_object_handle us, const char *attrs)
{
    int rc = 0;
    struct bdmf_link *link=NULL;

    if (bdmf_session_access_right(NULL) == BDMF_ACCESS_GUEST)
        return BDMF_ERR_PERM;

    if (!ds || !us)
        return BDMF_ERR_PARM;

    if (ds->state == bdmf_state_deleted ||
        us->state == bdmf_state_deleted)
        return BDMF_ERR_DELETED;

    bdmf_lock();
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_link, bdmf_hist_point_start, ds, us);
    if (!(ds->drv->flags & BDMF_DRV_FLAG_MUXUP) && !DLIST_EMPTY(&ds->us_links))
    {
        BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_link, bdmf_hist_point_end, BDMF_ERR_TOO_MANY);
        bdmf_unlock();
        BDMF_TRACE_RET(BDMF_ERR_TOO_MANY, "Object %s doesn't support more than 1 link\n", ds->name);
    }
    if (!(us->drv->flags & BDMF_DRV_FLAG_MUXDOWN) && !DLIST_EMPTY(&us->ds_links))
    {
        BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_link, bdmf_hist_point_end, BDMF_ERR_TOO_MANY);
        bdmf_unlock();
        BDMF_TRACE_RET(BDMF_ERR_TOO_MANY, "Object %s doesn't support more than 1 link\n", us->name);
    }
    if (bdmf_is_linked(ds, us, &link))
    {
        BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_link, bdmf_hist_point_end, BDMF_ERR_ALREADY);
        bdmf_unlock();
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "Objects %s and %s are already linked\n", ds->name, us->name);
    }
    if ((ds->drv->flags & BDMF_DRV_FLAG_MUXUP) ||
        (us->drv->flags & BDMF_DRV_FLAG_MUXDOWN))
    {
        link = bdmf_alloc(sizeof(struct bdmf_link));
        if (!link)
        {
            BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_link, bdmf_hist_point_end, BDMF_ERR_NOMEM);
            bdmf_unlock();
            BDMF_TRACE_RET(BDMF_ERR_NOMEM, "%s->%s: Can't allocate link structure\n",
                    ds->name, us->name);
        }
        link->magic = BDMF_LINK_MAGIC;
        link->usmo = us;
        link->dsmo = ds;
        //bdmf_print("!!! %s->%s: us=%p ds_links=%p ds=%p us_links=%p\n",
        //      ds->name, us->name, us, &us->ds_links, ds, &ds->us_links);
        //bdmf_print("!!! %s->%s: l=%p usl=%p dsl=%p\n", ds->name, us->name, link, &link->usl, &link->dsl);
    }

    if (us->drv->link_down)
        rc = us->drv->link_down(us, ds, attrs);
    if (ds->drv->link_up && !rc)
    {
        rc = ds->drv->link_up(ds, us, attrs);
        if (rc && us->drv->unlink_down)
            us->drv->unlink_down(us, ds);
    }
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_link, bdmf_hist_point_end, rc);
    if (rc)
    {
        bdmf_unlock();
        if (link)
            bdmf_free(link);
        BDMF_TRACE_RET(rc, "%s -> %s\n", ds->name, us->name);
    }

    if (link)
    {
        DLIST_INSERT_HEAD(&ds->us_links, link, dsl);
        DLIST_INSERT_HEAD(&us->ds_links, link, usl);
    }
    else
    {
        DLIST_INSERT_HEAD(&ds->us_links, (struct bdmf_link *)us, dsl);
        DLIST_INSERT_HEAD(&us->ds_links, (struct bdmf_link *)ds, usl);
    }

    bdmf_get(ds);
    bdmf_get(us);
    bdmf_unlock();

    BDMF_TRACE_INFO("success: %s -> %s\n", ds->name, us->name);
    
    return 0;
}


static void _bdmf_unlink(struct bdmf_object *ds, struct bdmf_object *us, struct bdmf_link *link)
{
    if (ds->drv->unlink_up)
        ds->drv->unlink_up(ds, us);
    if (us->drv->unlink_down)
        us->drv->unlink_down(us, ds);

    /* Detach link lists. Init list header in the object which is not mux */
    if (link->magic == BDMF_LINK_MAGIC)
    {
        /* mux-mux using intermediary object */
        DLIST_REMOVE(link, usl);
        DLIST_REMOVE(link, dsl);
        bdmf_free(link);
    }
    else
    {
        DLIST_REMOVE(us, dsl);
        DLIST_REMOVE(ds, usl);
    }
    bdmf_put(ds);
    bdmf_put(us);
}

/** Unlink managed objects from each other
 *
 * \param[in]   ds      Downstream object handle
 * \param[in]   us      Upstream object handle
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_unlink(bdmf_object_handle ds, bdmf_object_handle us)
{
    struct bdmf_link *link=NULL;

    if (bdmf_session_access_right(NULL) == BDMF_ACCESS_GUEST)
        return BDMF_ERR_PERM;

    if (!us || !ds || (ds==us))
        return BDMF_ERR_PARM;

    bdmf_lock();
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_unlink, bdmf_hist_point_start, ds, us);
    /* Make sure that flows are linked */
    if (!bdmf_is_linked(ds, us, &link))
    {
        BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_unlink, bdmf_hist_point_end, BDMF_ERR_NOT_LINKED);
        bdmf_unlock();
        BDMF_TRACE_RET(BDMF_ERR_NOT_LINKED, "Objects %s and %s are not linked\n", ds->name, us->name);
    }

    _bdmf_unlink(ds, us, link);

    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_unlink, bdmf_hist_point_end, 0);
    bdmf_unlock();

    BDMF_TRACE_INFO("success: %s -\\> %s\n", ds->name, us->name);

    return 0;
}


/** Upstream link iterator
 *
 * \param[in]   mo      Managed object
 * \param[in]   prev    Previous upstream link handle. NULL=get first
 * \return   next us link handle or NULL
 */
bdmf_link_handle bdmf_get_next_us_link(bdmf_object_handle mo, bdmf_link_handle prev)
{
    struct bdmf_link *next;
    BUG_ON(!mo);
    bdmf_lock();
    if (prev)
        next = DLIST_NEXT(prev, dsl);
    else
        next = DLIST_FIRST(&mo->us_links);
    bdmf_unlock();
    if (!next || next == (struct bdmf_link *)mo)
        return NULL;
    return next;
}


/** Downstream link iterator
 *
 * \param[in]   mo      Managed object
 * \param[in]   prev    Previous downstream link handle. NULL=get first
 * \return   next ds link handle or NULL
 */
bdmf_link_handle bdmf_get_next_ds_link(bdmf_object_handle mo, bdmf_link_handle prev)
{
    struct bdmf_link *next;
    BUG_ON(!mo);
    bdmf_lock();
    if (prev)
        next = DLIST_NEXT(prev, usl);
    else
        next = DLIST_FIRST(&mo->ds_links);
    bdmf_unlock();
    if (!next || next == (struct bdmf_link *)mo)
        return NULL;
    return next;
}


/** Map DS link to object
 *
 * \param[in]   ds_link
 * \return object handle
 */
bdmf_object_handle bdmf_ds_link_to_object(bdmf_link_handle ds_link)
{
    if (ds_link->magic == BDMF_OBJECT_MAGIC)
        return (struct bdmf_object *)ds_link;
    BUG_ON(ds_link->magic != BDMF_LINK_MAGIC);
    return ds_link->dsmo;
}


/** Map US link to object
 *
 * \param[in]   us_link
 * \return object handle
 */
bdmf_object_handle bdmf_us_link_to_object(bdmf_link_handle us_link)
{
    if (us_link->magic == BDMF_OBJECT_MAGIC)
        return (struct bdmf_object *)us_link;
    BUG_ON(us_link->magic != BDMF_LINK_MAGIC);
    return us_link->usmo;
}


/** Find managed object given its type name and attributes.
 *
 * The function combines bdmf_type_find_get() and bdmf_find_get() .
 * When no longer needed, the managed object handle must be released by bdmf_put() function.
 *
 * \param[in]   discr   Object discriminator: \n
 *                      type name followed by optional "/attribute_string",
 *                      whereas attr_string is in the same format as in bdmf_new().
 *                      This string is passed transparently to the driver and must be
 *                      sufficient to uniquely identify the object.
 *                      Note that type name can contain "@location" to identify
 *                      remote subsystem implementing the type. If "@location"
 *                      is omitted, any location is assumed.
 * \param[out]  pmo     Managed object handle
 * \return
 *     0      - OK \n
 *    <0      - error
 */
int bdmf_find_get_by_name(const char *discr, bdmf_object_handle *pmo)
{
    char type_name[20];
    const char *attrs;
    struct bdmf_type *drv;
    char *p;
    int rc;

    if (!discr)
        return BDMF_ERR_PARM;
    if ((p=strchr(discr, '/')))
    {
        if ((p-discr)>=sizeof(type_name))
            return BDMF_ERR_PARSE;
        memcpy(type_name, discr, p-discr);
        type_name[p-discr] = 0;
        attrs = p+1;
    }
    else
    {
        strncpy(type_name, discr, sizeof(type_name)-1);
        type_name[sizeof(type_name)-1]=0;
        attrs=NULL;
    }

    rc = bdmf_type_find_get(type_name, &drv);
    if (rc)
        return rc;
    rc = bdmf_find_get(drv, NULL, attrs, pmo);
    bdmf_type_put(drv);

    return rc;
}

/** Get object name
 * \param[in]   mo      Current managed object or NULL
 * \return object name
 */
const char *bdmf_object_name(bdmf_object_handle mo)
{
    if (!mo)
        return NULL;
    return mo->name;
}

/** Inform framework that service is being used.
 *
 * \param[in]   mo      Managed object
 * \param[in]   ref_obj Managed object being used by fo
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_reference_add(struct bdmf_object *mo, struct bdmf_object *ref_obj)
{
    struct bdmf_ref *ref = _bdmf_ref_alloc(mo, ref_obj, NULL);
    return ref ? 0 : BDMF_ERR_NOMEM;
}


/** Inform framework that service is no longer being used by the object.
 *
 * \param[in]   mo      Managed object
 * \param[in]   ref_obj Managed object being used by mo
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_reference_free(struct bdmf_object *mo, struct bdmf_object *ref_obj)
{
    struct bdmf_ref *ref, *ref_tmp;
    DLIST_FOREACH_SAFE(ref, &mo->references, ref_list, ref_tmp)
    {
        if ((ref->ref_obj == ref_obj) && !ref->attr)
        {
            _bdmf_ref_free(ref);
            return 0;
        }
    }
    return BDMF_ERR_NOENT;
}

/** Get object owner.
 *
 * \param[in]   mo      Managed object
 * \param[out]  owner   Object owner
 */

void bdmf_get_owner(const bdmf_object_handle mo, bdmf_object_handle *owner)
{
    *owner = mo->owner;
}

/*
 * Exports
 */


EXPORT_SYMBOL(bdmf_get);
EXPORT_SYMBOL(bdmf_put);
EXPORT_SYMBOL(bdmf_new_and_configure);
EXPORT_SYMBOL(bdmf_new_and_set);
EXPORT_SYMBOL(bdmf_destroy);
EXPORT_SYMBOL(bdmf_find_get);
EXPORT_SYMBOL(bdmf_get_next);
EXPORT_SYMBOL(bdmf_get_next_child);
EXPORT_SYMBOL(bdmf_help);
EXPORT_SYMBOL(bdmf_object_name);
EXPORT_SYMBOL(bdmf_object_parent_set);
EXPORT_SYMBOL(bdmf_is_linked);
EXPORT_SYMBOL(bdmf_link);
EXPORT_SYMBOL(bdmf_unlink);
EXPORT_SYMBOL(bdmf_get_next_us_link);
EXPORT_SYMBOL(bdmf_get_next_ds_link);
EXPORT_SYMBOL(bdmf_ds_link_to_object);
EXPORT_SYMBOL(bdmf_us_link_to_object);
EXPORT_SYMBOL(bdmf_find_get_by_name);
EXPORT_SYMBOL(bdmf_reference_add);
EXPORT_SYMBOL(bdmf_reference_free);
EXPORT_SYMBOL(bdmf_get_owner);



