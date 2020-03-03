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
 * bdmf_attr.c
 *
 * Data path builder - attribute handling code
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

static bdmf_attr_enum_table_t bdmf_bool_table =
{
    .type_name = "bdmf_boolean",
    .values = {
        { .name = "yes", .val = 1 },
        { .name = "true", .val = 1 },
        { .name = "no", .val = 0 },
        { .name = "false", .val = 0 }, BDMF_ATTR_LAST
    }
};


struct bdmf_num_attr_format
{
    char *lln;
    char *ln;
    char *n;
    char *hn;
    char *hhn;
};

const static struct bdmf_num_attr_format bdmf_signed_n_format = {
    .lln = "%lli",
    .ln  = "%li",
    .n  = "%i",
    .hn = "%hi",
    .hhn = "%hhi"
};

const static struct bdmf_num_attr_format bdmf_unsigned_n_format = {
    .lln = "%llu",
    .ln  = "%lu",
    .n  = "%u",
    .hn = "%hu",
    .hhn = "%hhu"
};

const static struct bdmf_num_attr_format bdmf_hex_n_format = {
    .lln = "0x%llx",
    .ln  = "0x%lx",
    .n  = "0x%x",
    .hn = "0x%hx",
    .hhn = "0x%hhx"
};

const static char *bdmf_attr_format_s = "%s";
const static char *bdmf_attr_format_p = "%p";
const static char *bdmf_attr_format_invalid = "*Invalid use of ->format\n";

/* Aggregate types list */
static TAILQ_HEAD(aggr_list, bdmf_aggr_type) bdmf_aggr_type_list =
        TAILQ_HEAD_INITIALIZER(bdmf_aggr_type_list);

static int _bdmf_configure(bdmf_object_handle mo, struct bdmf_attr aattr[],
        char *set, int lock);

#define BDMF_ATTR_DISABLE_STRING        "disable"

/*
 * Attribute level lock/unlock
 */

static inline void bdmf_attr_lock(struct bdmf_object *mo, struct bdmf_attr *attr)
{
    if (!(attr->flags & BDMF_ATTR_NOLOCK))
    {
#ifdef BDMF_SYSTEM_LINUX
        if (in_irq())
            BDMF_TRACE_ERR_OBJ(mo, "Attempt to access attribute %s in interrupt context\n", attr->name);
#endif
        bdmf_lock();
    }
}

static inline void bdmf_attr_unlock(struct bdmf_object *mo, struct bdmf_attr *attr)
{
    if (!(attr->flags & BDMF_ATTR_NOLOCK))
        bdmf_unlock();
}


/* find aggregate type */
static struct bdmf_aggr_type *bdmf_aggr_type_find(const char *name)
{
    struct bdmf_aggr_type *at, *at_tmp;
    TAILQ_FOREACH_SAFE(at, &bdmf_aggr_type_list, list, at_tmp)
    {
        if (!strcmp(at->name, name))
            return at;
    }
    return NULL;
}


/*
 * mattr helpers
 */

/* Allocate mattr entry and add it to mattr descriptor */
static bdmf_mattr_entry_t *bdmf_mattr_entry_add(bdmf_mattr_t *mattr, bdmf_attr_op_t oper,
    bdmf_attr_id aid, bdmf_index index)
{
    bdmf_mattr_entry_t *entry;
    if (mattr->oper != oper && mattr->oper != bdmf_attr_op_any)
    {
        BDMF_TRACE_ERR("Mixing set/get operations in the same mattr is not supported\n");
        return NULL;
    }
    if (mattr->num_entries >= mattr->max_entries)
    {
        BDMF_TRACE_ERR("mattr block overflow\n");
        BUG();
    }
    entry = &mattr->entries[mattr->num_entries++];
    mattr->oper = oper;
    entry->aid = aid;
    entry->index = index;
    return entry;
}

/* Add "get_as_num" operation to mattr */
static int bdmf_mattr_get_as_num(bdmf_mattr_t *mattr, bdmf_attr_id aid,
    bdmf_index index, bdmf_number *pval)
{
    bdmf_mattr_entry_t *entry = bdmf_mattr_entry_add(mattr, bdmf_attr_op_get, aid, index);
    if (!entry)
        return BDMF_ERR_NOMEM;
    entry->val.val_type = bdmf_attr_number;
    entry->val.x.pnum = pval;
    return 0;
}

/* Add "set_as_num" operation to mattr */
static int bdmf_mattr_set_as_num(bdmf_mattr_t *mattr, bdmf_attr_id aid,
    bdmf_index index, bdmf_number val)
{
    bdmf_mattr_entry_t *entry = bdmf_mattr_entry_add(mattr, bdmf_attr_op_set, aid, index);
    if (!entry)
        return BDMF_ERR_NOMEM;
    entry->val.val_type = bdmf_attr_number;
    entry->val.x.num = val;
    return 0;
}

/* Add "get_as_string" operation to mattr */
static int bdmf_mattr_get_as_string(bdmf_mattr_t *mattr, bdmf_attr_id aid,
    bdmf_index index, char *buffer, uint32_t size)
{
    bdmf_mattr_entry_t *entry = bdmf_mattr_entry_add(mattr, bdmf_attr_op_get, aid, index);
    if (!entry)
        return BDMF_ERR_NOMEM;
    entry->val.val_type = bdmf_attr_string;
    entry->val.x.buf.ptr = buffer;
    entry->val.x.buf.len = size;
    return 0;
}

/* Add "set_as_string" operation to mattr */
static int bdmf_mattr_set_as_string(bdmf_mattr_t *mattr, bdmf_attr_id aid,
    bdmf_index index, const char *val)
{
    bdmf_mattr_entry_t *entry = bdmf_mattr_entry_add(mattr, bdmf_attr_op_set, aid, index);
    if (!entry)
        return BDMF_ERR_NOMEM;

    entry->val.x.s = bdmf_alloc(strlen(val) + 1);
    if (!entry->val.x.s) {
        mattr->num_entries--;
        return BDMF_ERR_NOMEM;
    }

    strcpy((char *)entry->val.x.s, val);
    entry->val.val_type = bdmf_attr_string;
    return 0;
}

/* Add "set_as_buf"/"get_as_buf" operation to mattr */
static int bdmf_mattr_add_as_buf(bdmf_mattr_t *mattr, bdmf_attr_op_t oper,
    bdmf_attr_id aid, bdmf_index index, void *buf, uint16_t size)
{
    bdmf_mattr_entry_t *entry = bdmf_mattr_entry_add(mattr, oper, aid, index);
    if (!entry)
        return BDMF_ERR_NOMEM;

    entry->val.x.buf.ptr = bdmf_alloc(size);
    if (!entry->val.x.buf.ptr){
        mattr->num_entries--;
        return BDMF_ERR_NOMEM;
    }

    memcpy(entry->val.x.buf.ptr, buf, size);
    entry->val.val_type = bdmf_attr_buffer;
    entry->val.x.buf.len = size;
    return 0;
}

/* Check access permission */
static inline int bdmf_attr_check_access(struct bdmf_object *mo, struct bdmf_attr *a,
        bdmf_access_type_t access)
{
    bdmf_access_right_t session_rights = bdmf_session_access_right(NULL);

    if (session_rights == BDMF_ACCESS_DEBUG)
        return BDMF_ERR_OK;

    if (mo && mo->state==bdmf_state_deleted)
        return BDMF_ERR_DELETED;

    /* Attributes marked for debug access are only available
     * in session with DEBUG access level
     */
    if ((a->flags & BDMF_ATTR_ACCESS_DEBUG))
        return BDMF_ERR_PERM;

    /* Guest session doesn't have any access to hidden attributes */
    if ((session_rights == BDMF_ACCESS_GUEST) && (a->flags & BDMF_ATTR_HIDDEN))
        return BDMF_ERR_PERM;

    /* Guest doesn't have write access */
    if ((access == bdmf_access_write) && (session_rights == BDMF_ACCESS_GUEST))
        return BDMF_ERR_PERM;

    if ((a->flags & BDMF_ATTR_ACCESS_ADMIN)
            && (session_rights == BDMF_ACCESS_GUEST))
        return BDMF_ERR_PERM;

    return BDMF_ERR_OK;
}


static inline char _bdmf_attr_close_bracket(char open_bracket)
{
    char cb = (open_bracket == '<') ? '>' :
        (open_bracket == '{') ? '}' :
        (open_bracket == '(') ? ')' : open_bracket;
    return cb;
}

static int _bdmf_attr_index_val(char **ppbuf, char **index_val, bdmf_boolean report_error)
{
    char *pbuf = *ppbuf;
    char ob = *pbuf;

    *index_val = pbuf;
    if (strchr("<{(\"\'", ob))
    {
        int nbrackets = 1;
        char cb = _bdmf_attr_close_bracket(*pbuf);
        char c = *(++pbuf);
        int quote = (ob == '"' || ob == '\'');

        *index_val = pbuf; /* Skip the opening bracket */

        while(c && nbrackets)
        {
            if (c == '"' || c == '\'')
            {
                quote = 1 - quote;
                if (quote)
                    ++nbrackets;
                else
                    --nbrackets;
            }
            else if (c == cb)
                --nbrackets;
            else if (c == '<' || c == '{' || c == '(')
            {
                ++nbrackets;
                cb = _bdmf_attr_close_bracket(c);
            }
            c = *(++pbuf);
        }
        if (nbrackets)
        {
            if (report_error)
                BDMF_TRACE_ERR("%s: '%c' is missing\n", *ppbuf, cb);
            return BDMF_ERR_PARSE;
        }
        *(pbuf - 1) = 0; /* Remove the closing bracket */
    }
    else
        pbuf = strpbrk(pbuf + 1, ",]= ");
    *ppbuf = pbuf;
    return 0;
}

int _bdmf_attr_string_split(char *attr, struct bdmf_attr_name_value **p_pairs,
    int *p_npairs, char **parser_stop_pos)
{
    char *pbuf = attr;
    int ncommas = 0;
    int nattrs = 0;
    char *p;
    struct bdmf_attr_name_value *pairs;
    bdmf_boolean report_error = (parser_stop_pos == NULL); /* FALSE if called from tab-completion logic */
    int rc = 0;

    if (!attr || !attr[0])
    {
        *p_pairs = NULL;
        *p_npairs = 0;
        return 0;
    }

    /* Number of "," is the upper limit of the total number of attributes
     * in the string. The actual number can be less because of aggregate
     * attributes and "," being a part of string value
     */
    p = attr;
    while ((p = strchr(p + 1, ',')))
        ncommas++;

    /* Allocate pairs array */
    pairs = bdmf_calloc(sizeof(struct bdmf_attr_name_value) * (ncommas + 1));
    if (!pairs)
        return BDMF_ERR_NOMEM;

    while (pbuf && *pbuf)
    {
        char *name = pbuf, *value = NULL;
        char *index = NULL;

        /* Parse one of the following:
         name
         name=value
         name[index]=value
         where final delimiter is either ',' or end of string
         */
        pbuf = strpbrk(pbuf, ",[=");
        if (pbuf)
        {
            char cdel = *pbuf;
            *pbuf = 0;
            switch (cdel)
            {
            case ',': /* no value */
                ++pbuf;
                break;

            case '[': /* indexed attribute */
                ++pbuf;
                rc = _bdmf_attr_index_val(&pbuf, &index, report_error);
                if (rc)
                    goto out_of_loop;
                if (!pbuf || *pbuf != ']')
                {
                    if (report_error)
                        BDMF_TRACE_ERR("can't parse attribute index for %s\n", name);
                    goto out_of_loop;
                }
                *(pbuf++) = 0;
                if (! *pbuf)
                    break;
                if (*pbuf == '=')
                    value = pbuf + 1;
                else if (*pbuf != ',')
                {
                    if (report_error)
                        BDMF_TRACE_ERR("'=' or ',' is expected after %s[%s]\n", name, index);
                    goto out_of_loop;
                }
                ++pbuf;
                break;

            case '=': /* simple attribute value */
                value = pbuf + 1;
                break;
            }
        }

        pairs[nattrs].name = name;
        pairs[nattrs].array_index = index;

        /* Value can be a string in '"' */
        if (value)
        {
            pbuf = value;

            /* aggregate attributes require special handling.
             * aggregate value is enclosed in <> brackets and can itself
             * contain aggregate values. Therefore, <> nesting must be handled properly
             */
            rc = _bdmf_attr_index_val(&pbuf, &value, report_error);
            if (rc)
                goto out_of_loop;
            pairs[nattrs].value = value;
            if (pbuf && *pbuf)
            {
                if (*pbuf != ',')
                {
                    BDMF_TRACE_ERR(
                            "',' or end of string is expected after %s\n",
                            value);
                    goto out_of_loop;
                }
                *(pbuf++) = 0;
            }
        }

        ++nattrs;
    }

    *p_pairs = pairs;
    *p_npairs = nattrs;
    return rc;

out_of_loop:
    if (parser_stop_pos == NULL)
        bdmf_free(pairs);
    else
    {
        /* This mode is used by tab-extension parser */
        *p_pairs = pairs;
        *p_npairs = nattrs;
        *parser_stop_pos = pbuf;
    }
    return BDMF_ERR_PARSE;
}

/** Split attribute string into { name, value } pairs
 *
 * This function is a helper that can be used by drivers.
 *
 * \param[in]   attr        Attribute string in format described in bdmf_new() and
 *                          bdmf_configure() functions
 * \param[out]  p_pairs     Array of { name, value } pairs. Must be deallocated by
 *                          the caller when no longer needed using
 *                          bdmf_attr_pairs_release()
 * \param[out]  p_npairs    Number of elements in the pairs array
 *
 * \return
 *     0    - OK\n
 *    <0    - error in parameters, parsing error or no memory
 */
int bdmf_attr_string_split(const char *attr,
        struct bdmf_attr_name_value **p_pairs, int *p_npairs)
{
    char *buf;

    if (!p_pairs || !p_npairs)
        return BDMF_ERR_PARM;

    if (!attr || !attr[0])
    {
        *p_pairs = NULL;
        *p_npairs = 0;
        return 0;
    }

    /* Copy attr string */
    buf = bdmf_alloc(strlen(attr) + 1);
    if (!buf)
        return BDMF_ERR_NOMEM;
    strcpy(buf, attr);

    /* Parse attribute string */
    return _bdmf_attr_string_split(buf, p_pairs, p_npairs, NULL);
}

/** Release {name, value} pairs array allocated by bdmf_attr_string_split()
 *
 * \param[in]   pairs   pairs array pointer returned by bdmf_attr_string_split()
 */
void bdmf_attr_pairs_release(struct bdmf_attr_name_value *pairs)
{
    if (!pairs)
        return;
    /* Release copy of attr string */
    if (pairs[0].name)
        bdmf_free(pairs[0].name);
    /* Release pairs array */
    bdmf_free(pairs);
}

/** Find attribute by its name given the name and attribute array
 * internal helper
 */
struct bdmf_attr *_bdmf_attr_by_name(struct bdmf_attr *aattr,
        const char *name)
{
    struct bdmf_attr *a = aattr;
    if (!a)
        return NULL;
    while (a->name)
    {
        if (!strcmp(a->name, name))
            break;
        ++a;
    }
    if (!a->name)
        return NULL;
    return a;
}

/** Find attribute by its name given its name and mo.
 * The function scans all parent types as well
 */
static struct bdmf_attr *_bdmf_parent_attr_by_name(const char *name,
        struct bdmf_object *mo, struct bdmf_object **pmo)
{
    struct bdmf_object *parent = mo->owner;
    struct bdmf_attr *attr = NULL;
    while (parent)
    {
        if ((attr = _bdmf_attr_by_name(parent->drv->aattr, name)))
        {
            *pmo = parent;
            break;
        }
        parent = parent->owner;
    }
    return attr;
}

/** Find attribute by its name
 * \param[in]   drv     Managed object type handle
 * \param[in]   name    Attribute name
 * \param[out]  p_attr  Attribute handle
 * \return
 *     0        - OK\n
 *     BDMF_ERR_NOENT  - no attribute with such name\n
 *     BDMF_ERR_PERM   - no permission\n
 *     BDMF_ERR_PARM  - error in parameters
 */
int bdmf_attr_by_name(bdmf_type_handle drv, const char *name,
        bdmf_attr_id *p_attr)
{
    struct bdmf_attr *a;
    int rc;
    if (!drv || !p_attr || !name)
        return BDMF_ERR_PARM;
    a = _bdmf_attr_by_name(drv->aattr, name);
    if (!a)
        return BDMF_ERR_NOENT;
    rc = bdmf_attr_check_access(NULL, a, bdmf_access_list);
    if (!rc)
        *p_attr = bdmf_attr_to_aid(drv, a);
    return rc;
}

#define BDMF_ATTR_ID_TO_ATTR(mo_or_mattr, id, mo, attr) \
    mo = (struct bdmf_object *)mo_or_mattr;\
    if (mo->magic != BDMF_OBJECT_MAGIC) return BDMF_ERR_PARM;\
    if (id >= mo->drv->nattrs) return BDMF_ERR_PARM;\
    attr = &mo->drv->aattr[id]


#define _BDMF_ATTR_CHECK_INDEX_RET(_attr, _index) \
    do { \
        if (!(_attr)->array_size) \
        { \
            if ((_index) > 0) \
                return BDMF_ERR_RANGE; \
        } \
        else {\
            if (((_index) < 0) && !((_attr)->flags & BDMF_ATTR_ALLOW_UNSET_INDEX)) \
                return BDMF_ERR_RANGE; \
            if (((_index) >= (_attr)->array_size) && \
                !((_attr)->flags & BDMF_ATTR_NO_RANGE_CHECK)) \
                return BDMF_ERR_RANGE; \
        } \
    } \
    while(0)

/* Get attribute array element value as binary array. */
static int _bdmf_attrelem_get_as_buf(bdmf_object_handle mo, struct bdmf_attr *attr,
        bdmf_index index, void *buffer, uint32_t size)
{
    int rc;
    if (!buffer || !size)
        return BDMF_ERR_PARM;
    if (!(attr->flags & BDMF_ATTR_READ))
        return BDMF_ERR_INVALID_OP;
    if (!(attr->flags & BDMF_ATTR_NO_RANGE_CHECK))
        _BDMF_ATTR_CHECK_INDEX_RET(attr, index);
    if ((rc = bdmf_attr_check_access(mo, attr, bdmf_access_read)))
        return rc;
    if (size < attr->size && attr->type != bdmf_attr_buffer)
        return BDMF_ERR_OVERFLOW;
    rc = attr->read(mo, attr, index, buffer, size);
    if (rc > 0 && attr->type != bdmf_attr_buffer)
        rc = 0;
    return rc;
}

/* Get attribute array element value as binary array. */
int bdmf_attrelem_get_as_buf(bdmf_object_handle mo_or_mattr, bdmf_attr_id aid,
        bdmf_index index, void *buffer, uint32_t size)
{
    struct bdmf_attr *attr;
    struct bdmf_object *mo;
    int rc;
    if (!mo_or_mattr)
        return BDMF_ERR_PARM;
    if (((bdmf_mattr_t *)mo_or_mattr)->magic == BDMF_MATTR_MAGIC)
    {
        return bdmf_mattr_add_as_buf((bdmf_mattr_t *)mo_or_mattr, bdmf_attr_op_get,
            aid, index, buffer, size);
    }
    BDMF_ATTR_ID_TO_ATTR(mo_or_mattr, aid, mo, attr);
    bdmf_attr_lock(mo, attr);
    rc = _bdmf_attrelem_get_as_buf(mo, attr, index, buffer, size);
    bdmf_attr_unlock(mo, attr);
    return rc;
}

/* write number to buffer of the given size */
static int _bdmf_attr_num_to_buf(struct bdmf_object *mo, struct bdmf_attr *attr, bdmf_number n, void *buf, uint32_t size)
{
    bdmf_number n1;
    if (attr->max_val > attr->min_val)
    {
        if (n < attr->min_val || n > attr->max_val)
            goto range_err;
    }
    switch(size)
    {
    case 1:
        n1 = n & 0xff;
        if ((n1 != n) && (~(n1 ^ n) != 0xff))
            goto range_err;
        *(int8_t *)buf = (int8_t)n;
        break;
    case 2:
        n1 = n & 0xffff;
        if ((n1 != n) && (~(n1 ^ n) != 0xffff))
            goto range_err;
        *(int16_t *)buf = (int16_t)n;
        break;
    case 4:
        n1 = n & 0xffffffff;
        if ((n1 != n) && (~(n1 ^ n) != 0xffffffff))
            goto range_err;
        *(int32_t *)buf = (int32_t)n;
        break;
    case 8:
        *(int64_t *)buf = (int64_t)n;
        break;
    default:
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "%s: size %d is not supported\n", attr->name, size);
        break;
    }
    return 0;

range_err:
    BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo, "%s: value %lld is out of range\n", attr->name, (long long)n);
}

/* read number from buffer of the given size */
static int _bdmf_attr_buf_to_num(struct bdmf_object *mo, struct bdmf_attr *attr, const void *buf, uint32_t size, bdmf_number *n)
{
    switch(size)
    {
    case 1:
        if ((attr->flags & BDMF_ATTR_UNSIGNED))
    	    *n = *(const uint8_t *)buf;
        else
            *n = *(const int8_t *)buf;
        break;
    case 2:
        if ((attr->flags & BDMF_ATTR_UNSIGNED))
            *n = *(const uint16_t *)buf;
        else
            *n = *(const int16_t *)buf;
        break;
    case 4:
        if ((attr->flags & BDMF_ATTR_UNSIGNED))
            *n = *(const uint32_t *)buf;
        else
            *n = *(const int32_t *)buf;
        break;
    case 8:
        /*cannot support unsigned flag value BDMF_ATTR_UNSIGNED */
        /* due to signed type bdmf_number of n*/
        *n = *(const bdmf_number *)buf;
        break;
    default:
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "%s: size %d is not supported\n", attr->name, size);
        break;
    }
    return 0;
}

/* Reference from aggregate type */
struct bdmf_attr_ref
{
    struct bdmf_ref *old_ref;
    struct bdmf_ref *new_ref;
};
#define BDMF_MAX_REFS_PER_AGGREGATE 32

static int _bdmf_attr_ref_update(bdmf_object_handle mo, struct bdmf_attr *main_attr, struct bdmf_attr *attr,
    bdmf_index index, int offset, struct bdmf_attr_ref *ar, struct bdmf_object *ref_obj)
{
    /* Validate referenced object type */
    if (ref_obj && attr->ref_obj_type && ref_obj->drv != attr->ref_obj_type)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "%s: Invalid reference object type. Expected %s got %s\n",
            attr->name, attr->ref_obj_type->name, ref_obj->drv->name);

    ar->old_ref = ar->new_ref = NULL;

    /* Find existing reference if any */
    if ((index == BDMF_INDEX_UNASSIGNED) && !attr->array_size)
        index = 0;
    if (index != BDMF_INDEX_ANY)
        ar->old_ref = _bdmf_ref_find_by_attr(mo, main_attr, index, offset);

    /* Allocate new reference */
    if (ref_obj)
    {
        ar->new_ref =_bdmf_ref_alloc(mo, ref_obj, main_attr);
        if (!ar->new_ref)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NOMEM, mo, "%s: can't allocate reference to %s\n",
                attr->name, ref_obj->name);
        ar->new_ref->attr = main_attr;
        if (bdmf_attr_type_is_numeric(main_attr->index_type))
            ar->new_ref->attr_index = index;
        else if (index && index != BDMF_INDEX_ANY && index != BDMF_INDEX_UNASSIGNED)
            memcpy((void *)ar->new_ref->attr_index, (void *)index, main_attr->index_size);
        ar->new_ref->attr_offset = offset;
    }

    return 0;
}

/* Examine aggregate fields:
 * - prepare list of referenced objects in aggregate
 * - range-check
 */
static int _bdmf_aggr_validate(bdmf_object_handle mo, struct bdmf_attr *main_attr, struct bdmf_attr *attr,
    bdmf_index index, const char *buf, int offset, struct bdmf_attr_ref *refs, int *nrefs)
{
    struct bdmf_attr *a = attr->aggr_type->fields;
    int rc = 0;
    int nr = *nrefs;
    while(a->name && !rc)
    {
        if (a->type == bdmf_attr_object)
        {
            struct bdmf_object *ref_obj;
            if (buf)
                memcpy(&ref_obj, buf+offset+a->offset, sizeof(ref_obj));
            else
                ref_obj = NULL;
            rc = _bdmf_attr_ref_update(mo, main_attr, a, index, offset + a->offset,
                    &refs[nr], ref_obj);
            if (!rc)
                ++nr;
        }
        else if ((a->max_val > a->min_val) && buf)
        {
            bdmf_number n;
            rc = _bdmf_attr_buf_to_num(mo, a, buf+offset+a->offset, a->size, &n);
            if (rc)
                return rc;
            if (n < a->min_val || n > a->max_val)
            {
                BDMF_TRACE_ERR_OBJ(mo, "%s..%s: value %lld is out of range\n",
                    main_attr->name, a->name, (long long)n);
                rc = BDMF_ERR_RANGE;
            }
        }
        else if (a->type == bdmf_attr_aggregate)
        {
            rc = _bdmf_aggr_validate(mo, main_attr, a, index, buf, offset + a->offset, refs, &nr);
        }
        ++a;
    }
    *nrefs = nr;
    return rc;
}

/* If aggregate type contains references - set it field-by-field */
static int _bdmf_delete_aggr(bdmf_object_handle mo, struct bdmf_attr *attr, bdmf_index index)
{
    struct bdmf_attr_ref refs[BDMF_MAX_REFS_PER_AGGREGATE];
    int num_refs = 0;
    int rc = 0;
    int i;

    /* Create list of old and new references */
    if (attr->aggr_type->need_validation)
        rc = _bdmf_aggr_validate(mo, attr, attr, index, NULL, 0, refs, &num_refs);
    rc = rc ? rc : attr->del(mo, attr, index);
    if (!rc)
    {
        /* all good. release old references, keep new */
        for(i=0; i<num_refs; i++)
        {
            if (refs[i].old_ref)
                _bdmf_ref_free(refs[i].old_ref);
        }
    }
    return rc;
}

/* If aggregate type contains references - set it field-by-field */
static int _bdmf_set_aggr(bdmf_object_handle mo, struct bdmf_attr *attr,
    bdmf_index *index, const void *buf, uint32_t size, int is_add)
{
    struct bdmf_attr_ref refs[BDMF_MAX_REFS_PER_AGGREGATE];
    int num_refs = 0;
    int i;
    int rc;

    /* Create list of old and new references */
    if (attr->aggr_type->need_validation)
    {
        rc = _bdmf_aggr_validate(mo, attr, attr, is_add ? BDMF_INDEX_ANY : *index, buf, 0, refs, &num_refs);
        if (rc)
            goto cleanup;
    }

    /* write */
    if (is_add)
        rc = attr->add(mo, attr, index, buf, size);
    else
        rc = attr->write(mo, attr, *index, buf, size);
    if (rc < 0)
        goto cleanup;

    /* all good. release old references, keep new */
    for(i=0; i<num_refs; i++)
    {
        if (refs[i].old_ref)
            _bdmf_ref_free(refs[i].old_ref);
        if (is_add && refs[i].new_ref)
        {
            if (bdmf_attr_type_is_numeric(attr->index_type))
                refs[i].new_ref->attr_index = *index;
            else if (index)
                memcpy((void *)refs[i].new_ref->attr_index, (void *)index, attr->index_size);
        }
    }
    return 0;

cleanup:
    /* failure. Release new references, keep old intact */
    for(i=0; i<num_refs; i++)
    {
        if (refs[i].new_ref)
            _bdmf_ref_free(refs[i].new_ref);
    }
    return rc;
}

/* Set attribute array element value as object reference */
static int _bdmf_attrelem_set_as_ref(bdmf_object_handle mo, struct bdmf_attr *attr,
        bdmf_index *index, struct bdmf_object *ref_obj, int is_add)
{
    struct bdmf_attr_ref ar = { NULL, NULL };
    int rc = 0;

    rc = _bdmf_attr_ref_update(mo, attr, attr, is_add ? BDMF_INDEX_ANY : *index , 0, &ar, ref_obj);
    if (rc)
        return rc;

    /* Set new value */
    {
        if (is_add)
            rc = attr->add(mo, attr, index, &ref_obj, sizeof(ref_obj));
        else
            rc = attr->write(mo, attr, *index, &ref_obj, sizeof(ref_obj));
        if (rc < 0)
            goto cleanup;
    }

    /* Unreference old referenced object */
    if (ar.old_ref)
        _bdmf_ref_free(ar.old_ref);
    if (is_add && ar.new_ref)
        ar.new_ref->attr_index = *index;

    return 0;

cleanup:
    if (ar.new_ref)
        _bdmf_ref_free(ar.new_ref);
    BDMF_TRACE_RET_OBJ(rc, mo, "%s: setting reference to %s\n",
        attr->name, ref_obj ? ref_obj->name : "(none)");
}

/* Set attribute array element value as object reference */
static int _bdmf_delete_ref(bdmf_object_handle mo, struct bdmf_attr *attr, bdmf_index index)
{
    struct bdmf_attr_ref ar = { NULL, NULL };
    int rc;

    rc = _bdmf_attr_ref_update(mo, attr, attr, index , 0, &ar, NULL);
    rc = rc ? rc : attr->del(mo, attr, index);
    if (rc)
        return rc;

    if (ar.old_ref)
        _bdmf_ref_free(ar.old_ref);

    return 0;
}

/* Clear attribute containing referenced object */
int _bdmf_ref_attr_clear(struct bdmf_ref *ref)
{
    char *buf;
    struct bdmf_attr *attr = ref->attr;
    int rc;
    if (!attr || !ref->client || !ref->ref_obj)
        return 0;

    buf = bdmf_alloc(attr->size);
    if (!buf)
        return BDMF_ERR_NOMEM;

    if (attr->type == bdmf_attr_aggregate)
    {
        rc = _bdmf_attrelem_get_as_buf(ref->client, attr, ref->attr_index, buf, attr->size);
        if (rc < 0)
            BDMF_TRACE_RET_OBJ(rc, ref->client, "%s: can't read aggregate in order to clear referenced object %s\n",
                attr->name, ref->ref_obj->name);
    }
    memset(&buf[ref->attr_offset], 0, sizeof(void *));
    rc = attr->write(ref->client, attr, ref->attr_index, buf, attr->size);

    bdmf_free(buf);
    return (rc < 0) ? rc : 0;
}

/* Set attribute array element value from binary array. */
static int _bdmf_attrelem_set_as_buf(bdmf_object_handle mo, struct bdmf_attr *attr,
        bdmf_index *index, const void *buffer, uint32_t size, int is_add)
{
    int rc;
    if (!buffer || !size)
    {
        if (!(attr->flags & BDMF_ATTR_NO_NULLCHECK))
            return BDMF_ERR_PARM;
    }
    if (!(attr->flags & BDMF_ATTR_WRITE)
            && !((attr->flags & BDMF_ATTR_WRITE_INIT)
                    && (mo->state == bdmf_state_init)))
        return BDMF_ERR_INVALID_OP;
    if (!is_add && !(attr->flags & BDMF_ATTR_NO_RANGE_CHECK))
        _BDMF_ATTR_CHECK_INDEX_RET(attr, *index);
    if ((rc = bdmf_attr_check_access(mo, attr, bdmf_access_write)))
        return rc;
    if (size < attr->size && attr->type != bdmf_attr_buffer && buffer)
        return BDMF_ERR_OVERFLOW;
    if (attr->type == bdmf_attr_aggregate )
        rc = _bdmf_set_aggr(mo, attr, index, buffer, size, is_add);
    else if (attr->type == bdmf_attr_object)
    {
        if (buffer)
            rc = _bdmf_attrelem_set_as_ref(mo, attr, index, *(struct bdmf_object **)buffer, is_add);
        else
            rc = BDMF_ERR_PARM;
    }
    else if (is_add)
        rc = attr->add(mo, attr, index, buffer, size);
    else
        rc = attr->write(mo, attr, *index, buffer, size);

    /* If object has been modified - notify client objects referencing it */
    if (rc >= 0)
        _bdmf_ref_notify_change(mo);

    return rc;
}

/* Set attribute array element value from binary array. */
int bdmf_attrelem_set_as_buf(bdmf_object_handle mo_or_mattr, bdmf_attr_id aid,
        bdmf_index index, const void *buffer, uint32_t size)
{
    struct bdmf_object *mo;
    struct bdmf_attr *attr;
    int rc;
    if (!mo_or_mattr)
        return BDMF_ERR_PARM;
    if (((bdmf_mattr_t *)mo_or_mattr)->magic == BDMF_MATTR_MAGIC)
    {
        return bdmf_mattr_add_as_buf((bdmf_mattr_t *)mo_or_mattr, bdmf_attr_op_set,
            aid, index, (void *)buffer, size);
    }
    BDMF_ATTR_ID_TO_ATTR(mo_or_mattr, aid, mo, attr);
    bdmf_attr_lock(mo, attr);
    rc = _bdmf_attrelem_set_as_buf(mo, attr, &index, buffer, size, 0);
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_set_as_buf, bdmf_hist_point_both, mo, aid, index, buffer, size, rc);
    bdmf_attr_unlock(mo, attr);
    return rc;
}

/* Get attribute array element value as number - internal */
static int _bdmf_attrelem_get_as_num(bdmf_object_handle mo,
        struct bdmf_attr *attr, bdmf_index index, bdmf_number *pval)
{
    uint8_t buf[sizeof(long long)];
    uint32_t size = sizeof(buf);
    int rc;

    if (!pval)
        return BDMF_ERR_PARM;
    if ((attr->type != bdmf_attr_number)    &&
        (attr->type != bdmf_attr_object)    &&
        (attr->type != bdmf_attr_enum)      &&
        (attr->type != bdmf_attr_enum_mask) &&
        (attr->type != bdmf_attr_dyn_enum)  &&
        (attr->type != bdmf_attr_ipv4_addr) &&
        (attr->type != bdmf_attr_pointer))
        return BDMF_ERR_INVALID_OP;

    if (attr->size)
        size = attr->size;
    if (size > sizeof(buf))
    {
        BDMF_TRACE_ERR_OBJ(mo,
                "%s:%u - numbers larger than 8 bytes are not supported\n",
                attr->name, attr->size);
        return BDMF_ERR_NOT_SUPPORTED;
    }

    rc = _bdmf_attrelem_get_as_buf(mo, attr, index, buf, size);
    if (rc <= 0)
    {
        if (rc < 0)
            return rc;
        rc = attr->size;
    }
    size = rc;
    return _bdmf_attr_buf_to_num(mo, attr, buf, size, pval);
}

/* Get attribute array element value as number */
int bdmf_attrelem_get_as_num(bdmf_object_handle mo_or_mattr, bdmf_attr_id aid,
        bdmf_index index, bdmf_number *pval)
{
    struct bdmf_object *mo;
    struct bdmf_attr *attr;
    int rc;
    if (!mo_or_mattr)
        return BDMF_ERR_PARM;
    if (((bdmf_mattr_t *)mo_or_mattr)->magic == BDMF_MATTR_MAGIC)
    {
        return bdmf_mattr_get_as_num((bdmf_mattr_t *)mo_or_mattr, aid, index, pval);
    }
    BDMF_ATTR_ID_TO_ATTR(mo_or_mattr, aid, mo, attr);
    bdmf_attr_lock(mo, attr);
    rc = _bdmf_attrelem_get_as_num(mo, attr, index, pval);
    bdmf_attr_unlock(mo, attr);
    return rc;
}

/* Set attribute array element value as number */
static int _bdmf_attrelem_set_as_num(bdmf_object_handle mo, struct bdmf_attr *attr,
        bdmf_index *index, bdmf_number val, int is_add)
{
    uint8_t buf[sizeof(long long)];
    int rc;

    if ((attr->type != bdmf_attr_number)    &&
        (attr->type != bdmf_attr_enum)      &&
        (attr->type != bdmf_attr_enum_mask) &&
        (attr->type != bdmf_attr_dyn_enum)  &&
        (attr->type != bdmf_attr_ipv4_addr) &&
        (attr->type != bdmf_attr_pointer)   &&
        (attr->type != bdmf_attr_object))
    {
        return BDMF_ERR_INVALID_OP;
    }

    rc = _bdmf_attr_num_to_buf(mo, attr, val, buf, attr->size);
    rc = rc ? rc : _bdmf_attrelem_set_as_buf(mo, attr, index, buf, attr->size, is_add);

    return (rc >= 0) ? 0 : rc;
}

/* Set attribute array element value as number. */
int bdmf_attrelem_set_as_num(bdmf_object_handle mo_or_mattr, bdmf_attr_id aid,
        bdmf_index index, bdmf_number val)
{
    struct bdmf_object *mo;
    struct bdmf_attr *attr;
    int rc;
    if (!mo_or_mattr)
        return BDMF_ERR_PARM;

    if (((bdmf_mattr_t *)mo_or_mattr)->magic == BDMF_MATTR_MAGIC)
        return bdmf_mattr_set_as_num((bdmf_mattr_t *)mo_or_mattr, aid, index, val);

    BDMF_ATTR_ID_TO_ATTR(mo_or_mattr, aid, mo, attr);
    bdmf_attr_lock(mo, attr);
    rc = _bdmf_attrelem_set_as_num(mo, attr, &index, val, 0);
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_set_as_num, bdmf_hist_point_both, mo, aid, index, val, rc);
    bdmf_attr_unlock(mo, attr);
    BDMF_TRACE_RET_OBJ(rc, mo, "attribute:%s  value:%lld\n", attr->name, (long long)val);
    return rc;
}

/* Get attribute array element value in string (external) format. */
int _bdmf_attrelem_get_as_string(bdmf_object_handle mo, struct bdmf_attr *attr,
        bdmf_index index, char *buffer, uint32_t size)
{
    int rc;
    char *int_val;

    if (!buffer || !size)
        return BDMF_ERR_PARM;
    if (!(attr->flags & BDMF_ATTR_READ))
        return BDMF_ERR_INVALID_OP;
    if (!attr->val_to_s || !attr->size)
        return BDMF_ERR_NOT_SUPPORTED;
    if (!(attr->flags & BDMF_ATTR_NO_RANGE_CHECK))
        _BDMF_ATTR_CHECK_INDEX_RET(attr, index);
    if ((rc = bdmf_attr_check_access(mo, attr, bdmf_access_write)))
        return rc;

    int_val = bdmf_alloc(attr->size);
    if (!int_val)
        return BDMF_ERR_NOMEM;

    rc = attr->read(mo, attr, index, int_val, attr->size);
    rc = (rc < 0) ? rc : attr->val_to_s(mo, attr, int_val, buffer, size);
    bdmf_free(int_val);

    return (rc < 0) ? rc : 0;
}

/* Get attribute array element value in string (external) format. */
int bdmf_attrelem_get_as_string(bdmf_object_handle mo_or_mattr, bdmf_attr_id aid,
        bdmf_index index, char *buffer, uint32_t size)
{
    struct bdmf_object *mo;
    struct bdmf_attr *attr;
    int rc;
    if (!mo_or_mattr)
        return BDMF_ERR_PARM;
    if (((bdmf_mattr_t *)mo_or_mattr)->magic == BDMF_MATTR_MAGIC)
    {
        return bdmf_mattr_get_as_string((bdmf_mattr_t *)mo_or_mattr, aid, index,
            buffer, size);
    }
    BDMF_ATTR_ID_TO_ATTR(mo_or_mattr, aid, mo, attr);
    bdmf_attr_lock(mo, attr);
    rc = _bdmf_attrelem_get_as_string(mo, attr, index, buffer, size);
    bdmf_attr_unlock(mo, attr);
    return rc;
}

/* Set attribute array element using value in string (external) format. */
static int _bdmf_attrelem_set_as_string(bdmf_object_handle mo, struct bdmf_attr *attr,
        bdmf_index *index, const char *val, int is_add)
{
    int rc = 0;
    char *int_val;
    char *pval;
    int size = attr->size;

    if (!val)
        return BDMF_ERR_PARM;

    /* Further checks are done in _bdmf_attrelem_set_as_buf */
    if (!attr->s_to_val || !attr->size)
        return BDMF_ERR_NOT_SUPPORTED;

    int_val = bdmf_alloc(attr->size);
    if (!int_val)
        return BDMF_ERR_NOMEM;

    pval = int_val;

    /* Read the current aggregate value for read-modify-write operation.
     * Ignore error
     */
    memset(int_val, 0, attr->size);

    /* Support for NULL pointer */
    if (attr->type == bdmf_attr_aggregate || attr->type == bdmf_attr_buffer)
    {
        if (!val || !strcmp(val, "null") || !strcmp(val, "NULL"))
            pval = NULL;
        else if (!(attr->flags & BDMF_ATTR_CLEAR_FIELDS) &&
                    (attr->flags & BDMF_ATTR_READ) && !is_add)
        {
            attr->read(mo, attr, *index, int_val, attr->size);
        }
    }
    if (pval)
    {
        rc = attr->s_to_val(mo, attr, val, int_val, attr->size);
        if ((rc >= 0) && (attr->type==bdmf_attr_buffer))
            size = rc;
    }

    if (rc >= 0)
    {
        rc = _bdmf_attrelem_set_as_buf(mo, attr, index, pval, size, is_add);
        if (rc > 0)
            rc = 0;
    }

    bdmf_free(int_val);

    return rc;
}

/* Set attribute array element using value in string (external) format. */
int bdmf_attrelem_set_as_string(bdmf_object_handle mo_or_mattr, bdmf_attr_id aid,
        bdmf_index index, const char *val)
{
    struct bdmf_object *mo;
    struct bdmf_attr *attr;
    int rc;
    if (!mo_or_mattr)
        return BDMF_ERR_PARM;
    if (((bdmf_mattr_t *)mo_or_mattr)->magic == BDMF_MATTR_MAGIC)
    {
        return bdmf_mattr_set_as_string((bdmf_mattr_t *)mo_or_mattr, aid, index, val);
    }
    BDMF_ATTR_ID_TO_ATTR(mo_or_mattr, aid, mo, attr);

    bdmf_attr_lock(mo, attr);
    rc = _bdmf_attrelem_set_as_string(mo, attr, &index, val, 0);
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_set_as_string, bdmf_hist_point_both,
        mo, aid, index, val, rc);
    bdmf_attr_unlock(mo, attr);

    BDMF_TRACE_RET_OBJ(rc, mo, "attribute:%s  value:%s\n", attr->name, val);
}


/** Convert array index from string to internal format */
int bdmf_attr_string_to_array_index(struct bdmf_object *mo, struct bdmf_attr *attr, const char *sindex, void *index_buf)
{
    if (!sindex)
    {
        *(bdmf_index *)index_buf = BDMF_INDEX_UNASSIGNED;
        return 0;
    }
    if (!attr->s_to_index)
        return BDMF_ERR_PARM;
    return attr->s_to_index(mo, attr, sindex, index_buf, attr->index_size);
}

/** Convert array index from internal to string format */
int bdmf_attr_array_index_to_string(struct bdmf_object *mo, struct bdmf_attr *attr, bdmf_index index, char *sindex, uint32_t size)
{
    void *index_buf = bdmf_attr_type_is_numeric(attr->index_type) ? &index : (void *)index;
    if (!attr->index_to_s)
        return BDMF_ERR_PARM;
    return attr->index_to_s(mo, attr, index_buf, sindex, size);
}

/** Set a number of attributes in a single call
 * - internal helper.
 * See comments to bdmf_configure
 */
static int _bdmf_configure(bdmf_object_handle mo, struct bdmf_attr aattr[],
        char *set, int lock)
{
    int rc = 0;
    struct bdmf_attr *attr;
    int n_mandatory = 0, n_mandatory_set = 0;
    int *mand_indexes = NULL;
    struct bdmf_attr_name_value *pairs = NULL;
    int npairs;
    int i;

    /* If object is being created or attribute being set is an aggregate,
     * we need to make sure that all mandatory attributes/fields are set
     */
    if (mo->state == bdmf_state_init || aattr != mo->drv->aattr)
    {
        attr = &aattr[0];
        while (attr && attr->name)
        {
            if ((attr->flags & BDMF_ATTR_MANDATORY))
                ++n_mandatory;
            ++attr;
        }
        if (n_mandatory)
        {
            int i = 0;
            mand_indexes = bdmf_alloc(sizeof(int) * n_mandatory);
            if (!mand_indexes)
                return BDMF_ERR_NOMEM;
            attr = &aattr[0];
            while (attr && attr->name)
            {
                if ((attr->flags & BDMF_ATTR_MANDATORY))
                    mand_indexes[i++] = (attr - aattr);
                ++attr;
            }
        }
    }

    /* Convert attribute string to { name, index, value } array */
    rc = _bdmf_attr_string_split(set, &pairs, &npairs, NULL);
    if (rc)
    {
        if (mand_indexes)
            bdmf_free(mand_indexes);
        return rc;
    }

    for (i = 0; i < npairs; i++)
    {
        char *name = pairs[i].name;
        char *value = pairs[i].value;

        /* Search attribute and assign the value */
        attr = _bdmf_attr_by_name(aattr, name);
        if (!attr)
        {
            struct bdmf_object *parent;
            /* Not found. Perhaps it is parent's attribute used for identification ? */
            attr = _bdmf_parent_attr_by_name(name, mo, &parent);
            if (attr)
                continue;

            BDMF_TRACE_ERR_OBJ(mo, "attribute %s is not supported\n", name);
            rc = BDMF_ERR_NOENT;
            goto out_of_loop;
        }

        /* Value of any type, but enum must be set */
        if (!value)
        {
            if (attr->type == bdmf_attr_enum)
                value = (char *) attr->ts.enum_table->values[0].name;
            else
            {
                rc = BDMF_ERR_PARM;
                BDMF_TRACE_ERR_OBJ(mo, "=<value> expected after %s\n", name);
                goto out_of_loop;
            }
        }

        /* Convert array index to internal format */
        {
            char index_buf[attr->index_size];
            bdmf_index aindex;

            memset(index_buf, 0, attr->index_size);
            rc = bdmf_attr_string_to_array_index(mo, attr, pairs[i].array_index, index_buf);
            if (rc)
                goto out_of_loop;
            /* Numeric indexes are passed by value, other - by address */
            if (bdmf_attr_type_is_numeric(attr->index_type))
                aindex = *(bdmf_index *)index_buf;
            else
                aindex = (bdmf_index)&index_buf[0];
            if (lock && (attr->flags & BDMF_ATTR_NOLOCK))
                bdmf_unlock();
            rc = _bdmf_attrelem_set_as_string(mo, attr, &aindex, value, 0);
            if (lock && (attr->flags & BDMF_ATTR_NOLOCK))
                bdmf_lock();
            if (rc)
                goto out_of_loop;
        }

        if ((attr->flags & BDMF_ATTR_MANDATORY) != 0 && mand_indexes)
        {
            /* Clear index of mandatory attribute just set */
            int i;
            for (i = 0; i < n_mandatory; i++)
            {
                if (mand_indexes[i] == (attr - aattr))
                {
                    mand_indexes[i] = -1;
                    n_mandatory_set++;
                    break;
                }
            }
        }
    }

    /* Make sure that all mandatory attributes have been assigned */
    if (n_mandatory_set < n_mandatory)
    {
        int i;
        BDMF_TRACE_ERR_OBJ(mo, "mandatory attribute(s) not set:");
        for (i = 0; i < n_mandatory; i++)
            if (mand_indexes[i] >= 0)
                bdmf_print(" %s", aattr[mand_indexes[i]].name);
        bdmf_print("\n");
        rc = BDMF_ERR_PARM;
    }

    out_of_loop: if (pairs)
        bdmf_free(pairs);
    if (mand_indexes)
        bdmf_free(mand_indexes);
    return rc;
}

/** Set a number of attributes in a single call.
 *
 * The attributes and values are passed as a comma-delimted
 * ASCII string of name=value pairs.\n
 * For enum attrubutes "=value" can be ommitted. In this case
 * the 1st enum value is assumed. For example, if "bool_attr" is
 * boolean attribute, specifying "bool_attr" in the attribute string without value
 * means settings its value "=true", because "true" is the 1st value of "boolean"
 * enumeration.\n
 * For aggregate attributes the value must be surrounded by "<>" brackets and
 * has the following format:\n
 * aggr_attr1=<field_name1=value1,field_name2=value2>\n
 * Nested "<>" brackets are allowed to accomodate the case of aggregate attribute's
 * field itself being an aggregate.\n
 *
 * Example of attribute string:\n
 * attr1=25,attr2=string1,attr3=enum_value3,attr4="string 4",enum_attr5,aggr_attr1=<f1=v1,f2=v2>\n
 *
 * \param[in]   mo      Managed object handle
 * \param[in]   set     Comma delimited list of name=value pairs
 * \return
 *     0 - OK\n
 *     error code otherwise
 */
int bdmf_configure(bdmf_object_handle mo, const char *set)
{
    char *buf = NULL;
    char *pbuf = buf;
    int rc = 0;

    if (!mo)
        return BDMF_ERR_PARM;

    if (set && *set)
    {
        int len = strlen(set);
        int nquotes=0;
        int i;

        buf = bdmf_alloc(len + 1);
        if (!buf)
            return BDMF_ERR_NOMEM;
        /* Copy skipping white spaces */
        pbuf = buf;
        for(i=0; i<len; i++)
        {
            char c = set[i];
            if (!isspace(c) || nquotes)
                *pbuf++ = c;
            if (c == '"')
                nquotes = 1 - nquotes;
            else if (c == '\'')
                nquotes = 2 - nquotes;
        }
        *pbuf = 0;
        pbuf = buf;
        /* Remove outer quotes if any */
        if (*pbuf && strchr("<{(\"\'", *pbuf))
        {
            char cb = _bdmf_attr_close_bracket(*pbuf);
            if (buf[len-1] != cb)
            {
                bdmf_free(buf);
                BDMF_TRACE_RET_OBJ(BDMF_ERR_PARSE, mo, "Closing '%c' is missing\n", cb);
            }
            ++pbuf;
            buf[len-1] = 0;
        }
    }

    bdmf_lock();
    rc = _bdmf_configure(mo, mo->drv->aattr, pbuf, 1);
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_configure, bdmf_hist_point_both, mo, set, rc);
    bdmf_unlock();

    if (buf)
        bdmf_free(buf);

    /* Only print trace if configure is called for "real" object,
     * not temporary created by s_to_val_aggregate
     */
    if ((mo->drv->flags & BDMF_DRV_TMP))
        return rc;
    BDMF_TRACE_RET_OBJ(rc, mo, "config:%s\n", set);
}

/* Find attribute in attribute spec helper */
int bdmf_attr_get_hlp(const char *attrs, const char *name, char *val,
        uint32_t val_length)
{
    char *buf = NULL;
    char *pbuf = NULL;
    char *token, *attr_val;
    int rc = BDMF_ERR_NOENT;

    if (!attrs || !attrs[0])
        return rc;

    buf = bdmf_alloc(strlen(attrs) + 1);
    pbuf = buf;

    if (!buf)
        return BDMF_ERR_NOMEM;

    strcpy(buf, attrs);
    token = strsep(&pbuf, ",");

    /* "Normal" mode. Attribute in format name[=value].
     If there is no value, "yes" is assumed.
     */
    do
    {
        attr_val = token;
        strsep(&attr_val, "=");
        if (!strcmp(token, name))
        {
            if (!attr_val || !attr_val[0])
                attr_val = "yes";
            strncpy(val, attr_val, val_length);
            rc = BDMF_ERR_OK;
            break;
        }
    } while ((token = strsep(&pbuf, ",")));

    bdmf_free(buf);

    return rc;
}

int bdmf_attr_get_num_hlp(const char *attrs, const char *name, bdmf_enum *pval)
{
    char buf[20];
    char *p_end;
    bdmf_enum val;
    int rc;
    buf[sizeof(buf) - 1] = 0;
    rc = bdmf_attr_get_hlp(attrs, name, buf, sizeof(buf) - 1);
    if (rc)
        return rc;
    val = strtol(buf, &p_end, 0);
    if (p_end && *p_end)
    {
        BDMF_TRACE_ERR("%s - expected number, got <%s>\n", name, buf);
        return BDMF_ERR_PARM;
    }
    *pval = val;
    return 0;
}

/** Get enum value given the string value
 *
 * The function maps enum value to its string representation
 * \param[in]   table enum values table
 * \param[in]   text  enum text value
 * \param[out]  val   enum value
 * \return 0 if OK, BDMF_ERR_NOENT if not found
 */
int bdmf_attr_get_enum_val_hlp(const bdmf_attr_enum_table_t *table,
        const char *text, bdmf_enum *pval)
{
    const bdmf_attr_enum_val_t *vals=table->values;
    while (vals->name)
    {
        if (!strcmp(vals->name, text))
        {
            *pval = vals->val;
            return 0;
        }
        ++vals;
    }
    return BDMF_ERR_PARM;
}

int bdmf_attr_get_enum_hlp(const char *attr, const char *name,
        const bdmf_attr_enum_table_t *table, bdmf_enum *pval)
{
    char buf[20];
    int rc;
    rc = bdmf_attr_get_hlp(attr, name, buf, sizeof(buf) - 1);
    if (rc)
        strncpy(buf, attr, sizeof(buf) - 1); /* Allow for no-named attribute */
    buf[sizeof(buf) - 1] = 0;
    return bdmf_attr_get_enum_val_hlp(table, buf, pval);
}

/** Get enum string given its value
 *
 * The function maps enum value to its string representation
 * \param[in]   table enum values table
 * \param[in]   val   enum value
 * \return enum string value
 */
const char *bdmf_attr_get_enum_text_hlp(const bdmf_attr_enum_table_t *table,
    bdmf_enum val)
{
    const bdmf_attr_enum_val_t *vals=table->values;
    while (vals->name)
    {
        if (vals->val == val)
            return vals->name;
        ++vals;
    }
    return NULL;
}

static const char *_bdmf_attr_typename(const struct bdmf_attr *attr, int is_index)
{
    static char *type_name[] = {
        [bdmf_attr_number]="number", [bdmf_attr_string]="string",
        [bdmf_attr_pointer]="pointer", [bdmf_attr_object]="object",
        [bdmf_attr_buffer]="buffer", [bdmf_attr_ether_addr]="ether_addr",
        [bdmf_attr_ip_addr]="ip", [bdmf_attr_ipv4_addr]="ipv4",
        [bdmf_attr_ipv6_addr]="ipv6", [bdmf_attr_boolean]="bool",
        [bdmf_attr_enum]="enum", [bdmf_attr_enum_mask]="enum_mask",
        [bdmf_attr_dyn_enum]="dyn_enum", [bdmf_attr_aggregate]="aggregate",
        [bdmf_attr_custom]="custom"
    };
    static const char *inval = "*invalid*";
    bdmf_attr_type_t type = is_index ? attr->index_type : attr->type;
    if (type >= sizeof(type_name) / sizeof(char *))
        return inval;
    if (!type_name[type])
        return inval;
    if (type == bdmf_attr_enum || type == bdmf_attr_enum_mask)
    {
        const bdmf_attr_enum_table_t *et = is_index ? attr->index_ts.enum_table : attr->ts.enum_table;
        if (type == bdmf_attr_enum && et == &bdmf_bool_table)
            return type_name[bdmf_attr_boolean];
        /* return "enum" for the main type and underlying enum type for index */
        if (is_index && et && et->type_name)
            return et->type_name;
    }
    else if (is_index && type == bdmf_attr_aggregate)
    {
        /* return "aggregate" for the main type and underlying aggregate name for index */
        if (attr->index_ts.aggr_type_name)
            return attr->index_ts.aggr_type_name;
    }
    return type_name[type];
}

#define BDMF_NAME_TYPE_SECTION_LENGTH 48

static int _bdmf_attr_help_enum_list(const bdmf_attr_enum_table_t *et, int min_val, int max_val,
    char *buffer, uint32_t size)
{
    int total_len = 0;
    int len;
    const struct bdmf_attr_enum_val *vals = et->values;
    const char *format = " {%s";
    while (size && vals && vals->name)
    {
        if ((max_val <= min_val) || (vals->val >= min_val && vals->val <= max_val))
        {
            len = snprintf(buffer, size, format, vals->name);
            buffer += len;
            size -= len;
            format = ", %s";
            total_len += len;
        }
        ++vals;
    }
    len = snprintf(buffer, size, "}");
    total_len += len;
    if (et->type_name)
    {
        buffer += len;
        size -= len;
        len = snprintf(buffer, size, "(%s)", et->type_name);
        total_len += len;
    }
    return total_len;
}

/** Format attribute help string.
 *
 * The function formats attribute help string based on the
 * attribute description.
 *
 * \param[in]   attr    attribute descriptor
 * \param[out]  buffer  buffer where help string is returned
 * \param[out]  size    buffer size
 */
void bdmf_attr_help(const struct bdmf_attr *attr, char *buffer, uint32_t size)
{
    char *buffer0 = buffer;
    int step;
    int step_len;
    bdmf_attr_type_t type;
    if (!buffer || !size)
        return;
    buffer[--size] = 0;
    type = attr->type;
    if (type == bdmf_attr_enum && attr->ts.enum_table == &bdmf_bool_table)
        type = bdmf_attr_boolean;
    for (step = 0; size && step <= 10; step++)
    {
        step_len = 0;
        switch (step)
        {
        case 0:
            step_len = snprintf(buffer, size, "%-16s ", attr->name);
            break;
        case 1:
            step_len = snprintf(buffer, size, "%c",
                    (attr->flags & BDMF_ATTR_MANDATORY) ? 'M' : ' ');
            break;
        case 2:
            step_len = snprintf(buffer, size, "%c",
                    (attr->flags & BDMF_ATTR_KEY) ? 'K' : ' ');
            break;
        case 3:
            step_len = snprintf(buffer, size, "%c",
                    (attr->flags & BDMF_ATTR_READ) ? 'R' : ' ');
            break;
        case 4:
            if ((attr->flags & BDMF_ATTR_WRITE_INIT))
                step_len = snprintf(buffer, size, "%c", 'I');
            else if ((attr->flags & BDMF_ATTR_WRITE))
                step_len = snprintf(buffer, size, "%c", 'W');
            else
                step_len = snprintf(buffer, size, "%c", ' ');
            break;
        case 5:
            step_len = snprintf(buffer, size, "%c", attr->add ? 'A' : ' ');
            break;
        case 6:
            step_len = snprintf(buffer, size, "%c", attr->del ? 'D' : ' ');
            break;
        case 7:
            step_len = snprintf(buffer, size, "%c", attr->find ? 'F' : ' ');
            break;
        case 8:
            step_len = snprintf(buffer, size, " %s", _bdmf_attr_typename(attr, 0));
            break;
        case 9:
            step_len = snprintf(buffer, size, "/%d", attr->size);
            break;
        case 10:
            if (attr->array_size)
            {
                if (attr->index_type == bdmf_attr_number)
                    step_len = snprintf(buffer, size, "[%d]", attr->array_size);
                else if (attr->index_type == bdmf_attr_enum || attr->index_type == bdmf_attr_enum_mask)
                {
                    step_len = snprintf(buffer, size, "[%d(%s)", attr->array_size,
                        (attr->index_type == bdmf_attr_enum) ? "enum" : "enum_mask");
                    size -= step_len;
                    buffer += step_len;
                    step_len = _bdmf_attr_help_enum_list(attr->index_ts.enum_table, 0, 0, buffer, size);
                    size -= step_len;
                    buffer += step_len;
                    step_len = snprintf(buffer, size, "]");
                }
                else if (attr->index_type == bdmf_attr_aggregate && attr->index_ts.aggr_type_name)
                    step_len = snprintf(buffer, size, "[%d(aggregate %s)]",
                        attr->array_size, attr->index_ts.aggr_type_name);
                else
                    step_len = snprintf(buffer, size, "[%d(%s)]", attr->array_size, _bdmf_attr_typename(attr, 1));
            }
            break;
        }
        buffer += step_len;
        size -= step_len;
    }
    step_len = 0;
    if (type == bdmf_attr_enum || type == bdmf_attr_enum_mask)
        step_len = _bdmf_attr_help_enum_list(attr->ts.enum_table, attr->min_val, attr->max_val, buffer, size);
    else if (type == bdmf_attr_dyn_enum)
    {
        F_get_next_enum_val f_get_next = attr->ts.f_get_next_value;
        const char *name = NULL;
        const char *format = " {%s";
        bdmf_enum n;
        while ((name = f_get_next(name, &n)))
        {
            step_len = snprintf(buffer, size, format, name);
            buffer += step_len;
            size -= step_len;
            format = ",%s";
        }
        step_len = snprintf(buffer, size, "}");
    }
    else if (type == bdmf_attr_aggregate && attr->aggr_type)
    {
        step_len = snprintf(buffer, size, " %s", attr->ts.aggr_type_name);
        if (attr->aggr_type->struct_name)
        {
            buffer += step_len;
            size -= step_len;
            step_len = snprintf(buffer, size, "(%s)", attr->aggr_type->struct_name);
        }
    }
    else if (type == bdmf_attr_number)
    {
        if (attr->min_val < attr->max_val)
            step_len = snprintf(buffer, size, " %d-%d", (int)attr->min_val, (int)attr->max_val);
        if (attr->data_type_name)
        {
            buffer += step_len;
            size -= step_len;
            step_len = snprintf(buffer, size, " (%s)", attr->data_type_name);
        }
    }

    buffer += step_len;
    size -= step_len;
    if (attr->help)
    {
        if (buffer - buffer0 < BDMF_NAME_TYPE_SECTION_LENGTH)
        {
            /* Align description */
            int i;
            int spaces = BDMF_NAME_TYPE_SECTION_LENGTH - (buffer - buffer0);
            for (i = 0; i < spaces && size; i++)
            {
                step_len = snprintf(buffer, size, " ");
                buffer += step_len;
                size -= step_len;
            }
        }
        step_len = snprintf(buffer, size, " %s", attr->help);
    }
}

/** Format attribute help string in compact form.
 *
 * The function formats attribute help string based on the
 * attribute description.
 *
 * \param[in]   attr    attribute descriptor
 * \param[out]  buffer  buffer where help string is returned
 * \param[out]  size    buffer size
 */
void bdmf_attr_help_compact(const struct bdmf_attr *attr, char *buffer,
        uint32_t size)
{
    int step;
    int step_len;
    bdmf_attr_type_t type;
    if (!buffer || !size)
        return;
    buffer[--size] = 0;
    type = attr->type;
    if (type == bdmf_attr_enum && attr->ts.enum_table == &bdmf_bool_table)
        type = bdmf_attr_boolean;
    for (step = 0; size && step < 10; step++)
    {
        step_len = 0;
        switch (step)
        {
        case 0:
            step_len = snprintf(buffer, size, "%s : ", attr->name);
            break;
        case 1:
            if ((attr->flags & BDMF_ATTR_MANDATORY))
                step_len = snprintf(buffer, size, "M");
            break;
        case 2:
            if ((attr->flags & BDMF_ATTR_KEY))
                step_len = snprintf(buffer, size, "K");
            break;
        case 3:
            if ((attr->flags & BDMF_ATTR_READ))
                step_len = snprintf(buffer, size, "R");
            break;
        case 4:
            if ((attr->flags & BDMF_ATTR_WRITE_INIT))
                step_len = snprintf(buffer, size, "%c", 'I');
            else if ((attr->flags & BDMF_ATTR_WRITE))
                step_len = snprintf(buffer, size, "%c", 'W');
            break;
        case 5:
            if (attr->add)
                step_len = snprintf(buffer, size, "%c", 'A');
            break;
        case 6:
            if (attr->del)
                step_len = snprintf(buffer, size, "%c", 'D');
            break;
        case 7:
            if (attr->find)
                step_len = snprintf(buffer, size, "%c", 'F');
            break;
        case 8:
            step_len = snprintf(buffer, size, " : %s",  _bdmf_attr_typename(attr, 0));
            break;
        case 9:
            if (attr->array_size)
            {
                if (attr->index_type == bdmf_attr_number)
                {
                    /* step_len = snprintf(buffer, size, "[%d]", attr->array_size); */
                    step_len = snprintf(buffer, size, "[]");
                }
                else
                {
                    /* step_len = snprintf(buffer, size, "[%d(%s)]", attr->array_size, _bdmf_attr_typename(attr, 1)); */
                    step_len = snprintf(buffer, size, "[(%s)]", _bdmf_attr_typename(attr, 1));
                }
            }
            break;
        }
        buffer += step_len;
        size -= step_len;
    }
    if (type == bdmf_attr_aggregate && attr->aggr_type)
    {
        step_len = snprintf(buffer, size, " %s", attr->ts.aggr_type_name);
        buffer += step_len;
        size -= step_len;
        if (attr->aggr_type->struct_name)
        {
            step_len = snprintf(buffer, size, "(%s)", attr->aggr_type->struct_name);
            buffer += step_len;
            size -= step_len;
        }
    }
    if (attr->help)
        snprintf(buffer, size, " : %s", attr->help);
}

/*********************************************************************
 * Attribute access callbacks
 ********************************************************************/

static int _bdmf_attr_read(struct bdmf_object *mo, struct bdmf_attr *attr,
        bdmf_index index, void *val, uint32_t size)
{
    uint8_t *base = mo->mem_seg_base[attr->mem_seg];
    bdmf_mem_type_t mem_type = mo->drv->seg_type[attr->mem_seg];
    int offset;
    if (!base)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_IO, mo, "bdmf-read: mem area for attr %s is not allocated\n",
                attr->name);
    }
    if (!attr->array_size)
        index = 0;
    else if ((unsigned) index >= attr->array_size)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_IO, mo,
                "bdmf-read: array index %ld is out of range for attr %s\n",
                index, attr->name);
    }
    if (size > attr->size)
        size = attr->size;
    offset = attr->offset + attr->size * index;
    if (offset + size > mo->drv->seg_size[attr->mem_seg])
        return BDMF_ERR_IO;
    return bdmf_mem_read(mem_type, val, &base[offset], size);
}

/* write value in native format */
static int _bdmf_attr_write(struct bdmf_object *mo, struct bdmf_attr *attr,
        bdmf_index index, const void *val, uint32_t size)
{
    uint8_t *base = mo->mem_seg_base[attr->mem_seg];
    bdmf_mem_type_t mem_type = mo->drv->seg_type[attr->mem_seg];
    int offset;
    if (!base)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_IO, mo, "bdmf-write: mem area for attr %s is not allocated\n",
                attr->name);
    }
    if (!attr->array_size)
        index = 0;
    else if ((unsigned)index >= attr->array_size)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_IO, mo,
                "bdmf-write: array index %ld is out of range for attr %s\n",
                index, attr->name);
    }
    if (size > attr->size)
        size = attr->size;
    offset = attr->offset + attr->size * index;
    if (offset + size > mo->drv->seg_size[attr->mem_seg])
        return BDMF_ERR_IO;
    return bdmf_mem_write(mem_type, &base[offset], val, size);
}

/* internal --> string format: string attribute  */
static int _bdmf_attr_val_to_s_s(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    uint32_t len=strlen(val);
    if (len >= size)
        len = size - 1;
    memcpy(sbuf, val, len);
    sbuf[len] = 0;
    return len;
}

/* string --> internal format: string attribute  */
static int _bdmf_attr_s_to_val_s(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    uint32_t len=strlen(sbuf);
    char *sval = (char *)val;
    if (len >= size)
        len = size - 1;
    memcpy(sval, sbuf, len);
    sval[len] = 0;
    return len;
}


/* internal --> string format: numeric attribute or index */
static int _bdmf_attr_val_index_to_s_n(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val,
    uint32_t val_size, char *sbuf, uint32_t size, const char *format)
{
    bdmf_number n = 0;
    int rc = _bdmf_attr_buf_to_num(mo, ad, val, val_size, &n);

    switch (val_size)
    {
    case 1:
        snprintf(sbuf, size, format, (uint8_t)n);
        break;
    case 2:
        snprintf(sbuf, size, format, (uint16_t)n);
        break;
    case 4:
        snprintf(sbuf, size, format, (uint32_t)n);
        break;
    case 8:
        snprintf(sbuf, size, format, n);
        break;
    default:
        rc = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return rc;
}


/* internal --> string format: numeric attribute  */
static int _bdmf_attr_val_to_s_n(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    if ((ad->flags & BDMF_ATTR_HAS_DISABLE))
    {
        bdmf_number n = 0;
        _bdmf_attr_buf_to_num(mo, ad, val, ad->size, &n);
        if (n == ad->disable_val)
        {
            snprintf(sbuf, size, BDMF_ATTR_DISABLE_STRING);
            return 0;
        }
    }
    return _bdmf_attr_val_index_to_s_n(mo, ad, val, ad->size, sbuf, size, ad->ts.format);
}

/* internal --> string format: numeric index  */
static int _bdmf_attr_index_to_s_n(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    return _bdmf_attr_val_index_to_s_n(mo, ad, val, ad->index_size, sbuf, size, ad->index_ts.format);
}

/* string --> internal format: numeric attribute  */
static int _bdmf_attr_s_to_val_n(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    bdmf_number n;
    int format = 0;
    char *pend;

    if ((ad->flags & BDMF_ATTR_HAS_DISABLE))
    {
        if (!strcmp(sbuf, BDMF_ATTR_DISABLE_STRING))
        {
            n = ad->disable_val;
            return _bdmf_attr_num_to_buf(mo, ad, n, val, ad->size);
        }
    }

    if (!memcmp(sbuf, "0x", 2))
    {
        sbuf += 2;
        format = 16;
    }
    if ((ad->flags & BDMF_ATTR_HEX_FORMAT) || (ad->type==bdmf_attr_pointer))
        n = strtoull(sbuf, &pend, 16);
    else if ((ad->flags & BDMF_ATTR_UNSIGNED))
        n = strtoull(sbuf, &pend, format);
    else
        n = strtoll(sbuf, &pend, format);
    if (pend && *pend)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARSE, mo,
                "%s not set. cannot convert <%s> to number. Parsing stopped at %s\n",
                ad->name, sbuf, pend);
    }
    return _bdmf_attr_num_to_buf(mo, ad, n, val, ad->size);
}

/* string --> internal format: numeric index  */
static int _bdmf_attr_s_to_index_n(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    bdmf_number n;
#if defined(__LP64__) || defined(_LP64)
    if (!sscanf(sbuf, "%li", (long *)&n))
#else
    if (!sscanf(sbuf, "%lli", &n))
#endif
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARSE, mo,
                "%s not set. cannot convert <%s> to number using format <%s>\n",
                ad->name, sbuf, ad->ts.format);
    }
    return _bdmf_attr_num_to_buf(mo, ad, n, val, ad->index_size);
}

/* internal --> string format: mac attribute  */
static int _bdmf_attr_val_to_s_mac(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    const uint8_t *smac=(uint8_t *)val;
    if (size < 18)
        return BDMF_ERR_PARM;
    snprintf(sbuf, size, "%02x:%02x:%02x:%02x:%02x:%02x", smac[0], smac[1],
            smac[2], smac[3], smac[4], smac[5]);
    return 0;
}

/* string --> internal format: mac attribute  */
static int _bdmf_attr_s_to_val_mac(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    uint32_t umac[6];
    bdmf_mac_t *mac = (bdmf_mac_t *)val;
    int n;

    n = sscanf(sbuf, "%x:%x:%x:%x:%x:%x", &umac[0], &umac[1], &umac[2],
            &umac[3], &umac[4], &umac[5]);
    if ((n != 6) || (size < 6))
        return BDMF_ERR_PARM;
    for (n = 0; n < 6; n++)
    {
        if (umac[n] > 0xff)
            return BDMF_ERR_RANGE;
        mac->b[n] = (char)umac[n];
    }
    return 0;
}

/* const char *
 * inet_ntop6(src, dst, size)
 *      convert IPv6 binary address into presentation (printable) format
 * author:
 *      Paul Vixie, 1996.
 */
static char *_inet_ntop6(const uint8_t *src, char *dst, uint32_t size)
{
    /*
     * Note that int32_t and int16_t need only be "at least" large enough
     * to contain a value of the specified size.  On some systems, like
     * Crays, there is no such thing as an integer variable with 16 bits.
     * Keep this in mind if you think this function should have been coded
     * to use pointer overlays.  All the world's not a VAX.
     */
    char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;
    struct { int base, len; } best, cur;
#define NS_IN6ADDRSZ    16
#define NS_INT16SZ      2
    uint16_t words[NS_IN6ADDRSZ / NS_INT16SZ];
    int i;

    /*
     * Preprocess:
     *      Copy the input (bytewise) array into a wordwise array.
     *      Find the longest run of 0x00's in src[] for :: shorthanding.
     */
    memset(words, '\0', sizeof words);
    for (i = 0; i < NS_IN6ADDRSZ; i++)
        words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
    best.base = -1;
    best.len = 0;
    cur.base = -1;
    cur.len = 0;
    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++)
    {
        if (words[i] == 0)
        {
            if (cur.base == -1)
                cur.base = i, cur.len = 1;
            else
                cur.len++;
        } else
        {
            if (cur.base != -1)
            {
                if (best.base == -1 || cur.len > best.len)
                    best = cur;
                cur.base = -1;
            }
        }
    }
    if (cur.base != -1)
    {
        if (best.base == -1 || cur.len > best.len)
            best = cur;
    }
    if (best.base != -1 && best.len < 2)
        best.base = -1;

    /*
     * Format the result.
     */
    tp = tmp;
    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++)
    {
        /* Are we inside the best run of 0x00's? */
            if (best.base != -1 && i >= best.base &&
                i < (best.base + best.len))
            {
                if (i == best.base)
                    *tp++ = ':';
                continue;
            }
            /* Are we following an initial run of 0x00s or any real hex? */
            if (i != 0)
                *tp++ = ':';
            /* Is this address an encapsulated IPv4? */
            //if (i == 6 && best.base == 0 && (best.len == 6 ||
            //    (best.len == 7 && words[7] != 0x0001) ||
            //    (best.len == 5 && words[5] == 0xffff))) {
            //    if (!inet_ntop4(src+12, tp, sizeof tmp - (tp - tmp)))
            //        return (NULL);
            //    tp += strlen(tp);
            //    break;
            //}
            tp += sprintf(tp, "%x", words[i]);
    }
    /* Was it a trailing run of 0x00's? */
    if (best.base != -1 && (best.base + best.len) ==
        (NS_IN6ADDRSZ / NS_INT16SZ))
        *tp++ = ':';
    *tp++ = '\0';

    /*
     * Check for overflow, copy, and we're done.
     */
    if ((uint32_t)(tp - tmp) > size)
        return (NULL);
    strcpy(dst, tmp);
    return (dst);
}


/* int
 * inet_pton6(src, dst)
 *      convert presentation level address to network order binary form.
 * return:
 *      1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *      (1) does not touch `dst' unless it's returning 1.
 *      (2) :: in a full address is silently ignored.
 * credit:
 *      inspired by Mark Andrews.
 * author:
 *      Paul Vixie, 1996.
 */
static int _inet_pton6(const char *src, uint8_t *dst)
{
    static const char xdigits_l[] = "0123456789abcdef",
        xdigits_u[] = "0123456789ABCDEF";
#define NS_IN6ADDRSZ    16
#define NS_INT16SZ      2
    uint8_t tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
    const char *xdigits;
    int ch, seen_xdigits;
    uint16_t val;

    memset((tp = tmp), '\0', NS_IN6ADDRSZ);
    endp = tp + NS_IN6ADDRSZ;
    colonp = NULL;
    /* Leading :: requires some special handling. */
    if (*src == ':')
        if (*++src != ':')
            return (0);
    //curtok = src;
    seen_xdigits = 0;
    val = 0;
    while ((ch = *src++) != '\0')
    {
        const char *pch;

        if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
            pch = strchr((xdigits = xdigits_u), ch);
        if (pch != NULL)
        {
            val <<= 4;
            val |= (pch - xdigits);
            if (++seen_xdigits > 4)
                return (0);
            continue;
        }
        if (ch == ':')
        {
            //curtok = src;
            if (!seen_xdigits)
            {
                if (colonp)
                    return (0);
                colonp = tp;
                continue;
            }
            else if (*src == '\0')
                return (0);
            if (tp + NS_INT16SZ > endp)
                return (0);
            *tp++ = (uint8_t) (val >> 8) & 0xff;
            *tp++ = (uint8_t) val & 0xff;
            seen_xdigits = 0;
            val = 0;
            continue;
        }
        //
        //if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
        //    inet_pton4(curtok, tp) > 0) {
        //        tp += NS_INADDRSZ;
        //        seen_xdigits = 0;
        //        break;  /*%< '\\0' was seen by inet_pton4(). */
        //}
        return (0);
    }
    if (seen_xdigits)
    {
        if (tp + NS_INT16SZ > endp)
            return (0);
        *tp++ = (uint8_t) (val >> 8) & 0xff;
        *tp++ = (uint8_t) val & 0xff;
    }
    if (colonp != NULL)
    {
        /*
         * Since some memmove()'s erroneously fail to handle
         * overlapping regions, we'll do the shift by hand.
         */
        const int n = tp - colonp;
        int i;

        if (tp == endp)
            return (0);
        for (i = 1; i <= n; i++)
        {
            endp[- i] = colonp[n - i];
            colonp[n - i] = 0;
        }
        tp = endp;
    }
    if (tp != endp)
        return (0);
    memcpy(dst, tmp, NS_IN6ADDRSZ);
    return (1);
}

/* string --> internal format: ipv4 attribute  */
static int _bdmf_attr_s_to_val_ipv4(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    uint32_t ip3, ip2, ip1, ip0;
    bdmf_number n;

    if (size < 4 || sscanf(sbuf, "%u.%u.%u.%u", &ip3, &ip2, &ip1, &ip0) != 4)
        return BDMF_ERR_PARM;
    if (ip3 > 255 || ip2 > 255 || ip1 > 255 || ip0 > 255)
        return BDMF_ERR_PARM;
    n = (ip3 << 24) | (ip2 << 16) | (ip1 << 8) | ip0;
    return _bdmf_attr_num_to_buf(mo, ad, n, val, size);
}

/* internal --> string format: ipv4 attribute  */
static int _bdmf_attr_val_to_s_ipv4(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    bdmf_number n = 0;
    int rc = _bdmf_attr_buf_to_num(mo, ad, val, sizeof(bdmf_ipv4), &n);
    uint32_t ip=(uint32_t)n;

    if (size < 18)
        return BDMF_ERR_PARM;
    snprintf(sbuf, size, "%u.%u.%u.%u", (uint8_t) ((ip >> 24) & 0xff),
            (uint8_t) ((ip >> 16) & 0xff), (uint8_t) ((ip >> 8) & 0xff),
            (uint8_t) (ip & 0xff));
    return rc;
}

/* string --> internal format: ipv6 attribute  */
static int _bdmf_attr_s_to_val_ipv6(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    if (size < sizeof(bdmf_ipv6_t))
        return BDMF_ERR_OVERFLOW;
    if (!_inet_pton6(sbuf, val))
        return BDMF_ERR_PARSE;
    return 0;
}

/* internal --> string format: ipv6 attribute  */
static int _bdmf_attr_val_to_s_ipv6(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    const bdmf_ipv6_t *ip = (const bdmf_ipv6_t *)val;
    if (!_inet_ntop6(ip->data, sbuf, size))
        return BDMF_ERR_PARM;
    return 0;
}

/* internal --> string format: ip attribute  */
static int _bdmf_attr_val_to_s_ip(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    const bdmf_ip_t *ip = (const bdmf_ip_t *)val;

    if (ip->family == bdmf_ip_family_ipv4)
        return _bdmf_attr_val_to_s_ipv4(mo, ad, &ip->addr.ipv4, sbuf, size);
    if (ip->family == bdmf_ip_family_ipv6)
        return _bdmf_attr_val_to_s_ipv6(mo, ad, &ip->addr.ipv6, sbuf, size);
    return BDMF_ERR_PARM;
}

/* string --> internal format: ip attribute  */
static int _bdmf_attr_s_to_val_ip(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    bdmf_ip_t *pip = (bdmf_ip_t *)val;
    int rc;

    if (size < sizeof(bdmf_ip_t))
        return BDMF_ERR_OVERFLOW;
    rc = _bdmf_attr_s_to_val_ipv4(mo, ad, sbuf, &pip->addr.ipv4, sizeof(pip->addr.ipv4));
    if (!rc)
    {
        pip->family = bdmf_ip_family_ipv4;
        return 0;
    }

    rc = _bdmf_attr_s_to_val_ipv6(mo, ad, sbuf, &pip->addr.ipv6, sizeof(pip->addr.ipv6));
    if (rc)
        return rc;
    pip->family = bdmf_ip_family_ipv6;
    return 0;
}

/* internal --> string format: enum attribute  */
static int _bdmf_attr_val_to_s_enum(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    const struct bdmf_attr_enum_val *vals = ad->ts.enum_table->values;
    bdmf_number n = 0;
    int rc = _bdmf_attr_buf_to_num(mo, ad, val, ad->size, &n);

    if (rc)
        return rc;

    while (vals->name)
    {
        if (vals->val == (long)n)
        {
            strncpy(sbuf, vals->name, size);
            return 0;
        }
        ++vals;
    }
    return BDMF_ERR_PARM;
}

/* internal --> string format: enum index  */
static int _bdmf_attr_index_to_s_enum(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    const struct bdmf_attr_enum_val *vals = ad->index_ts.enum_table->values;
    bdmf_number n = 0;
    int rc = _bdmf_attr_buf_to_num(mo, ad, val, ad->index_size, &n);

    if (rc)
        return rc;

    while (vals->name)
    {
        if (vals->val == (long)n)
        {
            strncpy(sbuf, vals->name, size);
            return 0;
        }
        ++vals;
    }
    return BDMF_ERR_PARM;
}

/* string --> internal format: enum attribute  */
static int _bdmf_attr_s_to_val_enum(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)

{
    const struct bdmf_attr_enum_val *vals = ad->ts.enum_table->values;

    while (vals->name)
    {
        if (((ad->min_val >= ad->max_val) || (vals->val >= ad->min_val && vals->val <= ad->max_val)) &&
            !strcmp(vals->name, sbuf))
            return _bdmf_attr_num_to_buf(mo, ad, vals->val, val, size);
        ++vals;
    }
    return BDMF_ERR_PARM;
}

/* string --> internal format: enum index  */
static int _bdmf_attr_s_to_index_enum(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)

{
    const struct bdmf_attr_enum_val *vals = ad->index_ts.enum_table->values;
    while (vals->name)
    {
        if (!strcmp(vals->name, sbuf))
            return _bdmf_attr_num_to_buf(mo, ad, vals->val, val, size);
        ++vals;
    }
    return BDMF_ERR_PARM;
}

/* internal --> string format: dyn enum attribute  */
static int _bdmf_attr_val_to_s_dyn_enum(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    F_get_next_enum_val f_get_next = ad->ts.f_get_next_value;
    const char *name = NULL;
    bdmf_number n = 0;
    int rc = _bdmf_attr_buf_to_num(mo, ad, val, ad->size, &n);
    bdmf_enum enum_val;

    if (rc)
        return rc;

    while ((name = f_get_next(name, &enum_val)))
    {
        if (enum_val == n)
        {
            strncpy(sbuf, name, size);
            return 0;
        }
    }
    return BDMF_ERR_PARM;
}

/* string --> internal format: enum attribute  */
static int _bdmf_attr_s_to_val_dyn_enum(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    F_get_next_enum_val f_get_next = ad->ts.f_get_next_value;
    bdmf_enum enum_val;
    const char *name = NULL;
    while ((name = f_get_next(name, &enum_val)))
    {
        if (!strcmp(name, sbuf))
            return _bdmf_attr_num_to_buf(mo, ad, enum_val, val, size);
    }
    return BDMF_ERR_PARM;
}

#define MIN_BYTES_FOR_REPEATER 3

/* internal --> string format: buffer attribute  */
static int _bdmf_attr_val_to_s_buffer(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    uint8_t *hbuf = (uint8_t *)val;
    int i;

    if (size < ad->size*2 + 1)
        return BDMF_ERR_OVERFLOW;
    for (i = 0; i < ad->size; i++)
    {
        /* Check if byte repeats at leat MIN_BYTES_FOR_REPEATER times.
         * If yes - print in notation with repeater
         */
        int j;
        uint8_t b = hbuf[i];

        for (j=i+1; j<ad->size && hbuf[j]==b; j++)
            ;
        if (j - i >= MIN_BYTES_FOR_REPEATER)
        {
            sbuf += sprintf(sbuf, "%2.2x[%d]", b, j-i);
            i = j - 1;
        }
        else
            sbuf += sprintf(sbuf, "%2.2x", b);
    }
    return 0;
}

/* string --> internal format: buffer attribute (hex string) */
static int _bdmf_attr_s_to_val_buffer(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)

{
    int rc;
    rc = bdmf_strhex(sbuf, val, size);
    if (rc != strlen(sbuf)/2)
        return BDMF_ERR_PARM;
    return rc;
}

/* string format: object attribute  */
int bdmf_attr_val_to_s_object(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    static char *no_ref = "null";
    const struct bdmf_object *ro = *(const struct bdmf_object **)val;
    if (ro && (long)ro != BDMF_INDEX_UNASSIGNED)
        snprintf(sbuf, size-1, "{%s}", ro->name);
    else
        strncpy(sbuf, no_ref, size-1);
    sbuf[size-1] = 0;
    return 0;
}

/* string --> internal format: object attribute */
int bdmf_attr_s_to_val_object(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    struct bdmf_object *o;
    int rc;
    if (size != sizeof(void *))
        return BDMF_ERR_PARM;
    if (!sbuf || !sbuf[0] || !strcmp(sbuf, "0") || !strcmp(sbuf, "null"))
        o = NULL;
    else
    {
        rc = bdmf_find_get_by_name(sbuf, &o);
        if (rc)
            return rc;

        bdmf_put(o);
    }
    *(struct bdmf_object **)val = o;
    return 0;
}

/* internal --> string format: aggregate attribute or index */
static int _bdmf_attr_val_index_to_s_aggregate(struct bdmf_object *mo, struct bdmf_attr *ad, struct bdmf_aggr_type *aggr,
    const void *val, char *sbuf, uint32_t size)
{
    struct bdmf_attr *a;
    const char *name_format = "%s=";
    uint32_t len;

    BUG_ON(!aggr);
    len = aggr->size;
    BUG_ON(!len);
    if (size < 3)
        return BDMF_ERR_PARM;

    if (!val)
    {
        len = snprintf(sbuf, size, "null");
        size -= len;
        return 0;
    }

    a = aggr->fields;
    strcpy(sbuf++, "{");
    size -= 2; /* reserve room for terminator */

    /* Iterate aggregate attributes */
    while (a->name && size)
    {
        int rc;
        const char *buf = (const char *)val + a->offset;
        uint32_t l;

        /* Skip disabled fields */
        if (ad->is_field_visible && !ad->is_field_visible(mo, ad, val, aggr, a))
        {
            ++a;
            continue;
        }

        l = snprintf(sbuf, size, name_format, a->name);
        if (l >= size)
            break;

        sbuf += l;
        size -= l;
        name_format = ",%s=";
        *sbuf = 0;
        rc = a->val_to_s(mo, a, buf, sbuf, size);
        if (rc < 0)
            BDMF_TRACE_ERR_OBJ(mo, "Error when converting %s to string. Failed in field %s\n", ad->name, a->name);
        l = strlen(sbuf);
        sbuf += l;
        size -= l;
        ++a;
    }
    strcat(sbuf, "}");
    return 0;
}

/* internal --> string format: aggregate attribute  */
static int _bdmf_attr_val_to_s_aggregate(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    return _bdmf_attr_val_index_to_s_aggregate(mo, ad, ad->aggr_type, val, sbuf, size);
}

/* internal --> string format: aggregate index */
static int _bdmf_attr_index_to_s_aggregate(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    if (!val || *(bdmf_index *)val == BDMF_INDEX_UNASSIGNED)
    {
        snprintf(sbuf, size, "%ld", (long)BDMF_INDEX_UNASSIGNED);
        return 0;
    }
    return _bdmf_attr_val_index_to_s_aggregate(mo, ad, ad->index_aggr_type, val, sbuf, size);
}

/* string --> internal format: aggregate attribute */
static int _bdmf_attr_s_to_val_index_aggregate(struct bdmf_object *mo, struct bdmf_attr *ad, struct bdmf_aggr_type *aggr,
    const char *sbuf, void *val, uint32_t size)
{
    int rc;

    if (size != aggr->size)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                "%s: internal value of aggregate attribute has incorrect length %u/%u\n",
                ad->name, size, aggr->size);
    }

    /* If aggregate is itself an aggregate field, that means that
     * this function is called when setting an aggregate container.
     * In this case, there is no need to allocate a temporary buffer, because it has
     * already been done for the container
     */
    if (!(ad->flags & BDMF_ATTR_AGGR_FIELD))
    {
        struct bdmf_type type_tmp;
        struct bdmf_object *mo_tmp;

        /* Init fake object to keep attribute access callbacks happy */
        memset(&type_tmp, 0, sizeof(type_tmp));
        type_tmp.name = mo->name;
        type_tmp.seg_size[0] = size;
        type_tmp.trace_level = mo->drv->trace_level;
        type_tmp.aattr = aggr->fields;
        type_tmp.nattrs = 0xffff; /* big number to bypass attr id validation */
        type_tmp.flags = BDMF_DRV_TMP;
        DLIST_INIT(&type_tmp.obj_list);
        bdmf_fastlock_init(&type_tmp.lock);
        mo_tmp = bdmf_object_alloc(&type_tmp);
        if (!mo_tmp)
            return BDMF_ERR_NOMEM;
        mo_tmp->state = bdmf_state_active;
        mo_tmp->drv_priv = mo;
        mo_tmp->mem_seg_base[0] = val;

        /* Configure temporary object and then set the whole buffer in the real object.
         * This way, if something goes wrong, the "real" object remains untouched.
         */
        rc = bdmf_configure(mo_tmp, sbuf);
        bdmf_destroy(mo_tmp);
    }
    else
    {
        /* Aggregate is itself a field. In this case current object here is in fact temporary,
         * allocated when setting up the container.
         */
        void *old_base = mo->mem_seg_base[0];
        mo->mem_seg_base[0] = val;
        rc = _bdmf_configure(mo, aggr->fields, (char *) sbuf, 0);
        mo->mem_seg_base[0] = old_base;
    }

    return rc;
}


/* string --> internal format: aggregate attribute */
static int _bdmf_attr_s_to_val_aggregate(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    return _bdmf_attr_s_to_val_index_aggregate(mo, ad, ad->aggr_type, sbuf, val, size);
}


/* string --> internal format: aggregate index */
static int _bdmf_attr_s_to_index_aggregate(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    memset(val, 0, size);
    if (!sbuf || !strcmp(sbuf, "-1"))
    {
        *(bdmf_index *)val = BDMF_INDEX_UNASSIGNED;
        return 0;
    }
    return _bdmf_attr_s_to_val_index_aggregate(mo, ad, ad->index_aggr_type, sbuf, val, size);
}

/** Translate enum_mask from string
 * value+value+value
 * returns 0 on success or error code <0
 */
int _bdmf_attr_s_to_val_index_enum_mask(struct bdmf_object *mo, struct bdmf_attr *ad,
    const bdmf_attr_enum_table_t *enum_table, const char *sbuf, bdmf_number *pmask)
{
    bdmf_number mask = 0;
    char valname[32];
    const char *pbuf = sbuf;
    char *pplus;

    if (!sbuf || !sbuf[0])
    {
        *pmask = 0;
        return 0;
    }

    /* If string starts from digit - assume that it is a bitmask in hex format */
    if (isdigit(sbuf[0]))
    {
        int radix = memcmp(sbuf, "0x", 2) ? 16 : 0;
        char *pend;
        mask = strtoll(sbuf, &pend, radix);
        if (pend && *pend)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARSE, mo, "Parsing error at %s\n", pbuf);
        *pmask = mask;
        return 0;

    }

    /* Convert from port+port+port.. string */
    do
    {
        int len;
        bdmf_enum eval;

        pplus = strchr(pbuf, '+');
        if (pplus)
            len = pplus - pbuf;
        else
            len = strlen(pbuf);
        if (!len || len > sizeof(valname) - 1)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARSE, mo, "Parsing error at %s\n", pbuf);
        memcpy(valname, pbuf, len);
        valname[len] = 0;

        /* Translate ifname to rdpa_if */
        if (bdmf_attr_get_enum_val_hlp(enum_table, valname, &eval))
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARSE, mo, "Can't translate %s to %s\n", valname, enum_table->type_name);

        mask |= 1LL << eval;
        pbuf = pplus + 1;
    } while (pplus);

    *pmask = mask;

    return 0;
}


/** Translate rdpa_ports mask to string
 * port+port+port
 * returns 0 on success or error code <0
 */
int _bdmf_attr_val_index_to_s_enum_mask(struct bdmf_object *mo, struct bdmf_attr *ad,
    const bdmf_attr_enum_table_t *enum_table, bdmf_number mask, char *sbuf, uint32_t size)
{
    int i;
    char *pbuf = sbuf;

    if (!mask)
    {
        snprintf(sbuf, size, "0");
        return 0;
    }

    for (i=0; mask && i < 64; i++)
    {
        const char *valname;
        bdmf_number bit=(1LL << i);
        int len;

        if (!(mask & bit))
            continue;
        valname = bdmf_attr_get_enum_text_hlp(enum_table, i);
        if (!valname)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARSE, mo, "Can't translate enum value=%d to string\n", i);

        len = strlen(valname) + 1;
        if (len >= size)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_OVERFLOW, mo, "String buffer overflow\n");
        strcpy(pbuf, valname);
        strcat(sbuf, "+");
        size -= len;
        pbuf += len;
        mask &= ~bit;

    }
    /* cut final '+' */
    if (pbuf != sbuf)
        *(--pbuf) = 0;

    return 0;
}


/* string --> internal format: aggregate attribute */
static int _bdmf_attr_s_to_val_enum_mask(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    bdmf_number mask;
    int rc;
    if (size < ad->size)
        return BDMF_ERR_OVERFLOW;
    rc = _bdmf_attr_s_to_val_index_enum_mask(mo, ad, ad->ts.enum_table, sbuf, &mask);
    rc = rc ? rc : _bdmf_attr_num_to_buf(mo, ad, mask, val, ad->size);
    return rc;
}


/* string --> internal format: aggregate index */
static int _bdmf_attr_s_to_index_enum_mask(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size)
{
    bdmf_number mask;
    int rc;
    if (size < ad->index_size)
        return BDMF_ERR_OVERFLOW;
    rc = _bdmf_attr_s_to_val_index_enum_mask(mo, ad, ad->index_ts.enum_table, sbuf, &mask);
    rc = rc ? rc : _bdmf_attr_num_to_buf(mo, ad, mask, val, ad->index_size);
    return rc;
}

/* internal --> string format: enum attribute  */
static int _bdmf_attr_val_to_s_enum_mask(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    bdmf_number mask;
    int rc;

    rc = _bdmf_attr_buf_to_num(mo, ad, val, ad->size, &mask);
    rc = rc ? rc : _bdmf_attr_val_index_to_s_enum_mask(mo, ad, ad->ts.enum_table, mask, sbuf, size);
    return rc;
}

/* internal --> string format: enum mask  */
static int _bdmf_attr_index_to_s_enum_mask(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    bdmf_number mask;
    int rc;

    rc = _bdmf_attr_buf_to_num(mo, ad, val, ad->index_size, &mask);
    rc = rc ? rc : _bdmf_attr_val_index_to_s_enum_mask(mo, ad, ad->index_ts.enum_table, mask, sbuf, size);
    return rc;
}


/* Increment reference to aggregate type */
static void bdmf_aggr_type_get(struct bdmf_aggr_type *at)
{
    ++at->use_count;
    BDMF_TRACE_DBG("%s - %d\n", at->name, at->use_count);
}

/* Make sure that all object references in aggregate type are valid */
static int _bdmf_aggr_attr_ref_check(struct bdmf_type *drv, struct bdmf_aggr_type *at)
{
    struct bdmf_attr *a;
    int rc;

    if (at->references_verified)
        return 0;

    a = at->fields;
    while (a->name)
    {
        if (a->type == bdmf_attr_object)
        {
            if (a->ts.ref_type_name && *a->ts.ref_type_name && !a->ref_obj_type)
            {
                rc = bdmf_type_find_get(a->ts.ref_type_name, &a->ref_obj_type);
                if (rc)
                {
                    BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv,
                        "%s.%s: referenced object type %s is not registered\n",
                            at->name, a->name, a->ts.ref_type_name);
                }
            }
        }
        else if (a->aggr_type && a->aggr_type->has_references)
        {
            rc = _bdmf_aggr_attr_ref_check(drv, a->aggr_type);
            if (rc)
                return rc;
        }
        ++a;
    }
    at->references_verified = 1;
    return 0;
}

/* Default get_next callback */
static int _bdmf_attr_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    if (!index)
        return BDMF_ERR_PARM;
    if (*index == BDMF_INDEX_UNASSIGNED)
    {
        *index = 0;
        return 0;
    }
    if ((unsigned)*index >= ad->array_size)
        return BDMF_ERR_RANGE;
    if (*index == ad->array_size - 1)
        return BDMF_ERR_NO_MORE;
    *index = *index + 1;
    return 0;
}

/* Default find callback */
static int _bdmf_attr_find(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, void *val, uint32_t size)
{
    int rc = 0;
    char *int_val;

    if (size != ad->size)
        return BDMF_ERR_PARM;

    int_val = bdmf_alloc(ad->size);
    if (!int_val)
        return BDMF_ERR_NOMEM;

    *index = -1;
    while((rc = ad->get_next(mo, ad, index)) != BDMF_ERR_NO_MORE)
    {
        if (rc == BDMF_ERR_NOENT)
            continue;
        rc = rc ? rc : ad->read(mo, ad, *index, int_val, size);
        if (rc < 0)
            break;
        if (!memcmp(int_val, val, size))
            break;
    }
   
    bdmf_free(int_val);

    return (rc < 0) ? BDMF_ERR_NOENT : 0;
}

static const char *attr_dft_n_format_by_size(int size, uint32_t flags)
{
    const struct bdmf_num_attr_format *n_format;

    if ((flags & BDMF_ATTR_HEX_FORMAT))
        n_format = &bdmf_hex_n_format;
    else if ((flags & BDMF_ATTR_UNSIGNED))
        n_format = &bdmf_unsigned_n_format;
    else
        n_format = &bdmf_signed_n_format;

    if (size == sizeof(long long))
        return n_format->lln;
    if (size == sizeof(long))
        return n_format->ln;
    if (size == sizeof(short))
        return n_format->hn;
    if (size == sizeof(char))
        return n_format->hhn;
    return n_format->n;
}

int bdmf_attr_make(struct bdmf_type *drv, struct bdmf_attr *attr)
{
    int rc;

    if (attr->mem_seg >= BDMF_MAX_MEM_SEGS)
        return BDMF_ERR_PARM;

    if (attr->type == bdmf_attr_boolean)
    {
        attr->type = bdmf_attr_enum;
        attr->ts.enum_table = &bdmf_bool_table;
    }

    if ((attr->type == bdmf_attr_enum || attr->type == bdmf_attr_enum_mask) && !attr->ts.enum_table)
        BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv, "%s: enum table is missing\n", attr->name);

    attr->owner = drv;

    /* Aggregate type handling */
    attr->aggr_type = NULL;
    if (attr->type == bdmf_attr_aggregate)
    {
        struct bdmf_aggr_type *at = attr->ts.aggr_type_name ?
            bdmf_aggr_type_find(attr->ts.aggr_type_name) : NULL;
        if (!at)
        {
            BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv,
                    "%s: aggregate type %s is not registered\n",
                    attr->name, attr->ts.aggr_type_name);
        }
        if (attr->size && attr->size < at->size)
        {
            BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv,
                    "%s: attribute size %u is invalid. Expected %d\n",
                    attr->name, attr->size, at->size);
        }
        /* If called from type registration - verify object references */
        if (drv && at->has_references)
        {
            rc = _bdmf_aggr_attr_ref_check(drv, at);
            if (rc)
            {
                BDMF_TRACE_RET_DRV(rc, drv,
                        "%s: unresolved object reference in aggregate type %s\n",
                        attr->name, at->name);
            }
        }
        bdmf_aggr_type_get(at);
        attr->size = at->size;
        attr->aggr_type = at;
    }

    /* Index aggregate type handling */
    attr->index_size = 0;
    attr->index_aggr_type = NULL;
    if (attr->index_type == bdmf_attr_aggregate)
    {
        struct bdmf_aggr_type *at = attr->index_ts.aggr_type_name ?
            bdmf_aggr_type_find(attr->index_ts.aggr_type_name) : NULL;
        if (!at)
        {
            BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv,
                    "%s: aggregate type %s is not registered\n",
                    attr->name, attr->index_ts.aggr_type_name);
        }
        /* If called from type registration - verify object references */
        if (drv && at->has_references)
        {
            rc = _bdmf_aggr_attr_ref_check(drv, at);
            if (rc)
            {
                BDMF_TRACE_RET_DRV(rc, drv,
                        "%s: unresolved object reference in aggregate type %s\n",
                        attr->name, at->name);
            }
        }
        bdmf_aggr_type_get(at);
        attr->index_size = at->size;
        attr->index_aggr_type = at;
    }

    /* Object reference handling. Only check at type registration.
     * When aggregates are being registered - it is too early, because aggregates
     * are registered before types.
     */
    attr->ref_obj_type = NULL;
    if (drv && attr->type == bdmf_attr_object && attr->ts.ref_type_name && *attr->ts.ref_type_name)
    {
        if (!strcmp(attr->ts.ref_type_name, drv->name))
            attr->ref_obj_type = drv;
        else
        {
            rc = bdmf_type_find_get(attr->ts.ref_type_name, &attr->ref_obj_type);
            if (rc)
            {
                BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv,
                    "%s: referenced object type %s is not registered\n",
                        attr->name, attr->ts.ref_type_name);
            }
        }
    }

    if (!attr->size)
    {
        switch (attr->type)
        {
        case bdmf_attr_pointer:
        case bdmf_attr_object:
            attr->size = sizeof(void *);
            break;

        case bdmf_attr_ether_addr:
            attr->size = 6;
            break;

        case bdmf_attr_ip_addr:
            attr->size = sizeof(bdmf_ip_t);
            break;

        case bdmf_attr_ipv4_addr:
            attr->size = sizeof(bdmf_ipv4);
            break;

        case bdmf_attr_ipv6_addr:
            attr->size = sizeof(bdmf_ipv6_t);
            break;

        case bdmf_attr_enum:
            if (attr->ts.enum_table == &bdmf_bool_table)
                attr->size = sizeof(bdmf_boolean);
            else
            {
                typedef enum { _tst_enum_ } _tst_enum_t;
                attr->size = sizeof(_tst_enum_t);
            }
            break;

        default:
            BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv, "%s: attribute size is not set\n",  attr->name);
            break;
        }
    }

    /* Handle index */
    if ((attr->index_type == bdmf_attr_enum || attr->index_type == bdmf_attr_enum_mask) &&
        !attr->index_ts.enum_table)
    {
        BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv, "%s: index_ts.enum_table is missing\n", attr->name);
    }

    if (!attr->index_size)
    {
        attr->index_size = sizeof(bdmf_index);
        switch (attr->index_type)
        {
        case bdmf_attr_pointer:
        case bdmf_attr_object:
            attr->index_size = sizeof(void *);
            break;

        case bdmf_attr_ether_addr:
            attr->index_size = 6;
            break;

        case bdmf_attr_ip_addr:
            attr->index_size = sizeof(bdmf_ip_t);
            break;

        case bdmf_attr_ipv4_addr:
            attr->index_size = sizeof(bdmf_ipv4);
            break;

        case bdmf_attr_ipv6_addr:
            attr->index_size = sizeof(bdmf_ipv6_t);
            break;

        default:
            break;
        }
    }

    if (attr->index_size < sizeof(bdmf_index))
        attr->index_size = sizeof(bdmf_index);

    /* Check size */
    rc = 0;
    switch (attr->type)
    {
    case bdmf_attr_pointer:
    case bdmf_attr_object:
        if (attr->size != sizeof(void *))
            rc = BDMF_ERR_PARM;
        break;

    case bdmf_attr_ether_addr:
        if (attr->size != sizeof(bdmf_mac_t))
            rc = BDMF_ERR_PARM;
        break;

    case bdmf_attr_ip_addr:
        if (attr->size != sizeof(bdmf_ip_t))
            rc = BDMF_ERR_PARM;
        break;

    case bdmf_attr_ipv4_addr:
        if (attr->size != sizeof(bdmf_ipv4))
            rc = BDMF_ERR_PARM;
        break;

    case bdmf_attr_ipv6_addr:
        if (attr->size != sizeof(bdmf_ipv6_t))
            rc = BDMF_ERR_PARM;
        break;

    default:
        break;
    }
    if (rc)
        BDMF_TRACE_RET_DRV(rc, drv, "%s: size %d doesn't match the attribute type\n", attr->name, attr->size);

    /* If range is set - make sure that attribute is numeric */
    if (attr->max_val > attr->min_val)
    {
        if ((attr->type != bdmf_attr_number)    &&
            (attr->type != bdmf_attr_enum)      &&
            (attr->type != bdmf_attr_dyn_enum))
        {
            BDMF_TRACE_RET_DRV(rc, drv, "%s: range check is not supported for this attribute type\n", attr->name);
        }
    }

    /* Set DISABLE flag if disable value is set */
    if (attr->disable_val)
        attr->flags |= BDMF_ATTR_HAS_DISABLE;

    /* If disable is set - make sure that attribute is numeric */
    if ((attr->flags & BDMF_ATTR_HAS_DISABLE) && (attr->type != bdmf_attr_number))
    {
        BDMF_TRACE_RET_DRV(rc, drv, "%s: disable value is not supported for this attribute type\n", attr->name);
    }

    /* "No_value" is only supported for numeric and boolean attributes */
    if ((attr->flags & BDMF_ATTR_NO_VALUE) &&
        !((attr->type == bdmf_attr_number) ||
            (attr->type == bdmf_attr_enum && attr->ts.enum_table == &bdmf_bool_table)))
    {
        BDMF_TRACE_RET_DRV(rc, drv, "%s: BDMF_ATTR_NO_VALUE flag is only supported for numeric and boolean attributes\n",
            attr->name);
    }

    /* Set defaults. Note that format, aggr_type_name, f_get_next_value and enum_table
     * fields overlap.
     */
    if (!attr->ts.format)
    {
        switch (attr->type)
        {
        case bdmf_attr_number:
            attr->ts.format = attr_dft_n_format_by_size(attr->size, attr->flags);
            break;
        case bdmf_attr_string:
            attr->ts.format = bdmf_attr_format_s;
            break;
        case bdmf_attr_pointer:
            attr->ts.format = bdmf_attr_format_p;
            break;
        case bdmf_attr_enum:
            BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv, "%s: enum table is missing\n", attr->name);
            break;
        case bdmf_attr_dyn_enum:
            BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv, "%s: get_next function is missing\n", attr->name);
            break;
        case bdmf_attr_aggregate:
            BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv, "%s: aggr_type_name is missing\n", attr->name);
            break;
        case bdmf_attr_object:
            break;
        default:
            attr->ts.format = bdmf_attr_format_invalid;
            break;
        }
    }

    /* Set defaults format for index conversion */
    if (!attr->index_ts.format)
    {
        switch (attr->index_type)
        {
        case bdmf_attr_number:
            attr->index_ts.format = attr_dft_n_format_by_size(attr->index_size, 0);
            break;
        case bdmf_attr_string:
            attr->index_ts.format = bdmf_attr_format_s;
            break;
        case bdmf_attr_pointer:
            attr->index_ts.format = bdmf_attr_format_p;
            break;
        case bdmf_attr_enum:
            BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv, "%s: index enum table is missing\n", attr->name);
            break;
        case bdmf_attr_aggregate:
            BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv, "%s: index aggr_type_name is missing\n", attr->name);
            break;
        case bdmf_attr_object:
            break;
        default:
            attr->ts.format = bdmf_attr_format_invalid;
            break;
        }
    }

    if (!attr->s_to_val)
    {
        switch (attr->type)
        {
        case bdmf_attr_number:
        case bdmf_attr_pointer:
            attr->s_to_val = _bdmf_attr_s_to_val_n;
            break;
        case bdmf_attr_object:
            attr->s_to_val = bdmf_attr_s_to_val_object;
            break;
        case bdmf_attr_string:
            attr->s_to_val = _bdmf_attr_s_to_val_s;
            break;
        case bdmf_attr_ether_addr:
            attr->s_to_val = _bdmf_attr_s_to_val_mac;
            break;
        case bdmf_attr_ip_addr:
            attr->s_to_val = _bdmf_attr_s_to_val_ip;
            break;
        case bdmf_attr_ipv4_addr:
            attr->s_to_val = _bdmf_attr_s_to_val_ipv4;
            break;
        case bdmf_attr_ipv6_addr:
            attr->s_to_val = _bdmf_attr_s_to_val_ipv6;
            break;
        case bdmf_attr_enum:
            attr->s_to_val = _bdmf_attr_s_to_val_enum;
            break;
        case bdmf_attr_dyn_enum:
            attr->s_to_val = _bdmf_attr_s_to_val_dyn_enum;
            break;
        case bdmf_attr_buffer:
            attr->s_to_val = _bdmf_attr_s_to_val_buffer;
            break;
        case bdmf_attr_aggregate:
            attr->s_to_val = _bdmf_attr_s_to_val_aggregate;
            break;
        case bdmf_attr_enum_mask:
            attr->s_to_val = _bdmf_attr_s_to_val_enum_mask;
            break;
        default:
            break;
        }
    }
    if (!attr->val_to_s)
    {
        switch (attr->type)
        {
        case bdmf_attr_number:
        case bdmf_attr_pointer:
            attr->val_to_s = _bdmf_attr_val_to_s_n;
            break;
        case bdmf_attr_object:
            attr->val_to_s = bdmf_attr_val_to_s_object;
            break;
        case bdmf_attr_string:
            attr->val_to_s = _bdmf_attr_val_to_s_s;
            break;
        case bdmf_attr_ether_addr:
            attr->val_to_s = _bdmf_attr_val_to_s_mac;
            break;
        case bdmf_attr_ip_addr:
            attr->val_to_s = _bdmf_attr_val_to_s_ip;
            break;
        case bdmf_attr_ipv4_addr:
            attr->val_to_s = _bdmf_attr_val_to_s_ipv4;
            break;
        case bdmf_attr_ipv6_addr:
            attr->val_to_s = _bdmf_attr_val_to_s_ipv6;
            break;
        case bdmf_attr_enum:
            attr->val_to_s = _bdmf_attr_val_to_s_enum;
            break;
        case bdmf_attr_dyn_enum:
            attr->val_to_s = _bdmf_attr_val_to_s_dyn_enum;
            break;
        case bdmf_attr_buffer:
            attr->val_to_s = _bdmf_attr_val_to_s_buffer;
            break;
        case bdmf_attr_aggregate:
            attr->val_to_s = _bdmf_attr_val_to_s_aggregate;
            break;
        case bdmf_attr_enum_mask:
            attr->val_to_s = _bdmf_attr_val_to_s_enum_mask;
            break;
        default:
            break;
        }
    }
    if (!attr->s_to_index)
    {
        switch (attr->index_type)
        {
        case bdmf_attr_number:
        case bdmf_attr_pointer:
            attr->s_to_index = _bdmf_attr_s_to_index_n;
            break;
        case bdmf_attr_object:
            attr->s_to_index = bdmf_attr_s_to_val_object;
            break;
        case bdmf_attr_string:
            attr->s_to_index = _bdmf_attr_s_to_val_s;
            break;
        case bdmf_attr_ether_addr:
            attr->s_to_index = _bdmf_attr_s_to_val_mac;
            break;
        case bdmf_attr_ip_addr:
            attr->s_to_index = _bdmf_attr_s_to_val_ip;
            break;
        case bdmf_attr_ipv4_addr:
            attr->s_to_index = _bdmf_attr_s_to_val_ipv4;
            break;
        case bdmf_attr_enum:
            attr->s_to_index = _bdmf_attr_s_to_index_enum;
            break;
        case bdmf_attr_enum_mask:
            attr->s_to_index = _bdmf_attr_s_to_index_enum_mask;
            break;
        case bdmf_attr_buffer:
            attr->s_to_index = _bdmf_attr_s_to_val_buffer;
            break;
        case bdmf_attr_aggregate:
            attr->s_to_index = _bdmf_attr_s_to_index_aggregate;
            break;
        default:
            break;
        }
    }
    if (!attr->index_to_s)
    {
        switch (attr->index_type)
        {
        case bdmf_attr_number:
        case bdmf_attr_pointer:
            attr->index_to_s = _bdmf_attr_index_to_s_n;
            break;
        case bdmf_attr_object:
            attr->index_to_s = bdmf_attr_val_to_s_object;
            break;
        case bdmf_attr_string:
            attr->index_to_s = _bdmf_attr_val_to_s_s;
            break;
        case bdmf_attr_ether_addr:
            attr->index_to_s = _bdmf_attr_val_to_s_mac;
            break;
        case bdmf_attr_ip_addr:
            attr->index_to_s = _bdmf_attr_val_to_s_ip;
            break;
        case bdmf_attr_ipv4_addr:
            attr->index_to_s = _bdmf_attr_val_to_s_ipv4;
            break;
        case bdmf_attr_enum:
            attr->index_to_s = _bdmf_attr_index_to_s_enum;
            break;
        case bdmf_attr_enum_mask:
            attr->index_to_s = _bdmf_attr_index_to_s_enum_mask;
            break;
        case bdmf_attr_buffer:
            attr->index_to_s = _bdmf_attr_val_to_s_buffer;
            break;
        case bdmf_attr_aggregate:
            attr->index_to_s = _bdmf_attr_index_to_s_aggregate;
            break;
        default:
            break;
        }
    }
    if (!attr->read)
    {
        if (!attr->size && (attr->flags & BDMF_ATTR_READ))
            BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv, "%s: size is missing\n", attr->name);
        attr->read = _bdmf_attr_read;
    }
    if (!attr->write)
    {
        if (!attr->size && (attr->flags & (BDMF_ATTR_WRITE | BDMF_ATTR_WRITE_INIT)))
            BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv, "%s: size is missing\n", attr->name);
        attr->write = _bdmf_attr_write;
    }
    else
        attr->flags |= BDMF_ATTR_UDEF_WRITE;

    if (!(attr->flags & BDMF_ATTR_LEVEL_MASK))
        attr->flags |= BDMF_ATTR_MAJOR;

    /* Set default .get_next callback for array attributes */
    if ((attr->array_size > 0) && (attr->flags & BDMF_ATTR_READ))
    {
        if (!attr->get_next)
        {
            if (attr->index_type != bdmf_attr_number && attr->index_type != bdmf_attr_enum)
                BDMF_TRACE_RET_DRV(BDMF_ERR_PARM, drv, "%s: get_next is missing for abstract attribute array\n", attr->name);
            attr->get_next = _bdmf_attr_get_next;
        }
        else
            attr->flags |= BDMF_ATTR_UDEF_GET_NEXT;
        if (!attr->find)
            attr->find = _bdmf_attr_find;
        else
            attr->flags |= BDMF_ATTR_UDEF_FIND;
    }

    if (attr->array_size && !bdmf_attr_type_is_numeric(attr->index_type))
        attr->flags |= BDMF_ATTR_NO_RANGE_CHECK;

    /* Set attribute level NOLOCK flag if driver-level NOLOCK is set */
    if (drv && (drv->flags & BDMF_DRV_NOLOCK))
        attr->flags |= BDMF_ATTR_NOLOCK;

    /* Set attribute-level NOLOCK flag for KEY and WRITE-INIT attributes */
    if ((attr->flags & (BDMF_ATTR_WRITE_INIT | BDMF_ATTR_KEY)))
        attr->flags |= BDMF_ATTR_NOLOCK;

    return 0;
}

/* Unmake attribute. Remove reference to complex types */
void bdmf_attr_unmake(struct bdmf_attr *attr)
{
    if (attr->aggr_type)
    {
        bdmf_attr_aggr_type_put(attr->aggr_type);
        attr->aggr_type = NULL;
    }
    if (attr->index_aggr_type)
    {
        bdmf_attr_aggr_type_put(attr->index_aggr_type);
        attr->index_aggr_type = NULL;
    }
    if (attr->ref_obj_type)
    {
        /* There is a special case when attribute references its own type.
         * In this case the type's reference count is not incremented/decremented
         * by the reference
         */
        if (attr->owner != attr->ref_obj_type)
            bdmf_type_put(attr->ref_obj_type);
        attr->ref_obj_type = NULL;
    }
    if ((attr->flags & BDMF_ATTR_AGGR_FIELD))
    {
        attr->read = NULL;
        attr->write = NULL;
    }
    if (!(attr->flags & BDMF_ATTR_UDEF_GET_NEXT))
        attr->get_next = NULL;
    if (!(attr->flags & BDMF_ATTR_UDEF_FIND))
        attr->find = NULL;
    if (!(attr->flags & BDMF_ATTR_UDEF_WRITE))
        attr->write = NULL;
}

/** Register aggregate attribute type.
 *
 * Aggregate type is a composite consisting from multiple attributes similarly
 * to structure consisting from fields. Attributes in aggregate can be of any type,
 * including aggregate. Once registered, aggregate type can be used as attribute type as following:\n
 * .type=bdmf_attr_aggregate, .bdmf_attr_aggr_name=aggregate_type_name\n
 *
 * \param[in]   aggr_type    aggregate type sructure
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_attr_aggregate_type_register(struct bdmf_aggr_type *aggr_type)
{
    const char *name;
    struct bdmf_aggr_type *at;
    struct bdmf_attr *a;
    struct bdmf_attr *stop_field; /* stop cleanup at this attribute */
    int cur_size = 0;
    int rc = 0;

    if (!aggr_type || !aggr_type->name || !aggr_type->fields)
        return BDMF_ERR_PARM;

    name = aggr_type->name;
    at = bdmf_aggr_type_find(name);
    if (at)
    {
        BDMF_TRACE_ERR("%s: aggregate type is already registered\n",
                name);
        return BDMF_ERR_ALREADY;
    }

    /* Verify attribute descriptors and calculate number and size of attributes */
    a = stop_field = aggr_type->fields;
    while (a->name)
    {
        /* Field validation */

        /* user-defined read/write callbacks are not supported */
        if (a->read || a->write || a->add)
        {
            BDMF_TRACE_ERR(
                    "%s/%s: user read/write callback is not supported for aggregate type field\n",
                    name, a->name);
            rc = BDMF_ERR_PARM;
            goto bdmf_aggr_cleanup;
        }

        if (a->array_size)
        {
            BDMF_TRACE_ERR("%s/%s: array field is not supported\n",
                    name, a->name);
            rc = BDMF_ERR_PARM;
            goto bdmf_aggr_cleanup;
        }

        /* All fields are read/writeable */
        a->flags &= ~BDMF_ATTR_WRITE_INIT;
        a->flags |= (BDMF_ATTR_READ | BDMF_ATTR_WRITE);
        a->flags |= BDMF_ATTR_AGGR_FIELD;
        a->flags |= aggr_type->extra_flags;

        /* Fill-in default callbacks */
        rc = bdmf_attr_make(NULL, a);
        if (rc)
            goto bdmf_aggr_cleanup;

        /* At this point the field (a) might already been modified by bdmf_attr_make.
         * We want to clean it up in case of failure
         */
        stop_field = a + 1;

        if (a->type == bdmf_attr_object ||
            (a->type == bdmf_attr_aggregate && a->aggr_type->has_references))
        {
            aggr_type->has_references += a->aggr_type ? a->aggr_type->has_references : 1;
            if (aggr_type->has_references > BDMF_MAX_REFS_PER_AGGREGATE)
            {
                BDMF_TRACE_ERR("%s: too many object references in aggregate\n", name);
                rc = BDMF_ERR_PARM;
            }
            aggr_type->need_validation = 1;
        }

        /* Field size must be set */
        if (!a->size)
        {
            BDMF_TRACE_ERR("%s/%s: aggregate field size is not set\n",
                    name, a->name);
            rc = BDMF_ERR_PARM;
            goto bdmf_aggr_cleanup;
        }

        if (a->offset < cur_size)
        {
            BDMF_TRACE_ERR("%s/%s: field offset %u is too small\n",
                    name, a->name, a->offset);
            rc = BDMF_ERR_PARM;
            goto bdmf_aggr_cleanup;
        }

        if (a->max_val > a->min_val)
            aggr_type->need_validation = 1;

        cur_size = a->offset + a->size;

        ++a;
    }

    aggr_type->use_count = 1;
    //aggr_type->size = cur_size;
    if (!aggr_type->size)
        aggr_type->size = cur_size;
    else if (aggr_type->size < cur_size)
    {
        BDMF_TRACE_ERR("%s: user-defined size %u is too small\n", name, aggr_type->size);
        rc = BDMF_ERR_PARM;
        goto bdmf_aggr_cleanup;   
    }

    TAILQ_INSERT_TAIL(&bdmf_aggr_type_list, aggr_type, list);

    return BDMF_ERR_OK;

    /* Some error. Unmake aggregate fields and exit */
    bdmf_aggr_cleanup:
    {
        struct bdmf_attr *a1 = aggr_type->fields;
        while (a1->name && a1 != stop_field)
        {
            bdmf_attr_unmake(a1);
            ++a1;
        }
    }
    return rc;

}

/** Unregister aggregate attribute type.
 * The function unregisters aggregate type registered by bdmf_attr_aggregate_type_register().
 * Following unregistration, all attributes of this aggregate type become inaccessible.
 * \param[in]   aggr_type    aggregate type sructure
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_attr_aggregate_type_unregister(struct bdmf_aggr_type *aggr_type)
{
    if (!aggr_type || !aggr_type->use_count || aggr_type->deleted)
        return BDMF_ERR_PARM;
    aggr_type->deleted = 1;
    bdmf_attr_aggr_type_put(aggr_type);
    return BDMF_ERR_OK;
}

/** Find aggregate type by name
 * \param[in]   name          aggregate type name
 * \param[out]  paggr_type    aggregate type structure
 * The function increments usecount of the returned aggregate type
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_attr_aggregate_type_find(const char *name,
        struct bdmf_aggr_type **paggr_type)
{
    struct bdmf_aggr_type *aggr_type;
    if (!name || !paggr_type)
        return BDMF_ERR_PARM;
    aggr_type = bdmf_aggr_type_find(name);
    if (!aggr_type)
        return BDMF_ERR_NOENT;
    bdmf_aggr_type_get(aggr_type);
    *paggr_type = aggr_type;
    return BDMF_ERR_OK;
}

/** Decrement aggregate type's usecount
 * \param[in]   aggr_type    aggregate type structure
 */
void bdmf_attr_aggr_type_put(struct bdmf_aggr_type *aggr_type)
{
    BUG_ON(aggr_type->use_count<=0);
    --aggr_type->use_count;
    BDMF_TRACE_DBG("%s - %d\n", aggr_type->name, aggr_type->use_count);
    if (!aggr_type->use_count)
    {
        struct bdmf_attr *a = aggr_type->fields;
        while (a->name)
        {
            bdmf_attr_unmake(a);
            ++a;
        }
        TAILQ_REMOVE(&bdmf_aggr_type_list, aggr_type, list);
    }
}

/** Aggregate type iterator
 * \param[in]   prev          Previous type. NULL=get first
 * The function increments usecount of the returned aggregate type
 * and decrements usecount of the previous aggregate type
 * \return
 *    next type pointer or NULL
 */
const struct bdmf_aggr_type *bdmf_attr_aggregate_type_get_next(
        const struct bdmf_aggr_type *prev)
{
    struct bdmf_aggr_type *next_at;
    if (prev)
    {
        next_at = TAILQ_NEXT(prev, list);
        bdmf_attr_aggr_type_put((struct bdmf_aggr_type *)prev);
    }
    else
        next_at = TAILQ_FIRST(&bdmf_aggr_type_list);
    if (!next_at)
        return NULL;
    bdmf_aggr_type_get(next_at);
    return next_at;
}

/*
 * MATTR support
 */

/* increment roll-back buffer pointer by attr size aligned to
 * bdmf_number size
 */
static char *mattr_rollback_inc(const struct bdmf_attr *attr, char *ptr)
{
    long size = attr->size;
    size += sizeof(bdmf_number) - 1;
    size &= ~(sizeof(bdmf_number) - 1);
    return ptr + size;
}

/* increment roll-back buffer pointer by attr size aligned to
 * bdmf_number size
 */
static char *mattr_rollback_dec(const struct bdmf_attr *attr, char *ptr)
{
    long size = attr->size;
    size += sizeof(bdmf_number) - 1;
    size &= ~(sizeof(bdmf_number) - 1);
    return ptr - size;
}

static char *mattr_rollback_alloc(bdmf_object_handle mo, const bdmf_mattr_t *mattr)
{
    const bdmf_mattr_entry_t *entry;
    struct bdmf_attr *attr;
    int i;
    char *ptr=NULL;
    /* calculate roll-back buffer size */
    for(i=0; i<mattr->num_entries; i++)
    {
        entry = &mattr->entries[i];
        attr = &mo->drv->aattr[entry->aid];
        ptr = mattr_rollback_inc(attr, ptr);
    }
    /* since initial ptr value was 0, it now contains the buffer size */
    return bdmf_alloc((long)ptr);
}

static int bdmf_mset_read_old(bdmf_object_handle mo, const bdmf_mattr_entry_t *entry, char **pbuf)
{
    struct bdmf_attr *attr = &mo->drv->aattr[entry->aid];
    int rc;

    if ((attr->type != bdmf_attr_number)    &&
        (attr->type != bdmf_attr_enum)      &&
        (attr->type != bdmf_attr_dyn_enum)  &&
        (attr->type != bdmf_attr_ipv4_addr) &&
        (attr->type != bdmf_attr_pointer))
    {
        rc = _bdmf_attrelem_get_as_buf(mo, attr, entry->index, *pbuf, attr->size);
    }
    else
    {
        rc = _bdmf_attrelem_get_as_num(mo, attr, entry->index, (bdmf_number *)(*pbuf));
    }
    *pbuf = mattr_rollback_inc(attr, *pbuf);
    if (rc < 0)
    {
        BDMF_TRACE_RET_OBJ(rc, mo, "bdmf_mattr_set: failed to read attribute %s for rollback\n",
            attr->name);
    }
    return 0;
}

static void bdmf_mset_write_old(bdmf_object_handle mo, const bdmf_mattr_entry_t *entry, char **pbuf)
{
    struct bdmf_attr *attr = &mo->drv->aattr[entry->aid];
    bdmf_index index = entry->index;
    int rc;

    *pbuf = mattr_rollback_dec(attr, *pbuf);
    if ((attr->type != bdmf_attr_number)    &&
        (attr->type != bdmf_attr_enum)      &&
        (attr->type != bdmf_attr_dyn_enum)  &&
        (attr->type != bdmf_attr_ipv4_addr) &&
        (attr->type != bdmf_attr_pointer))
    {
        rc = _bdmf_attrelem_set_as_buf(mo, attr, &index, *pbuf, attr->size, 0);
    }
    else
    {
        rc = _bdmf_attrelem_set_as_num(mo, attr, &index, *((bdmf_number *)(*pbuf)), 0);
    }
    if (rc < 0)
    {
        BDMF_TRACE_ERR_OBJ(mo, "bdmf_mattr_set: failed to roll-back attribute %s for rollback\n",
            attr->name);
    }
}

static void bdmf_mattr_rollback(bdmf_object_handle mo, const bdmf_mattr_t *mattr, char *roll_back_buf,
    const bdmf_mattr_entry_t *entry)
{
    int nentry = entry - mattr->entries;
    struct bdmf_attr *attr = &mo->drv->aattr[entry->aid];
    char *pbuf = roll_back_buf;
    int i;

    /* Skip the failed attribute that we are not going to roll-back */
    pbuf = mattr_rollback_dec(attr, pbuf);

    /* Roll-back in reverse order entries preceding the failed entry.
     * In theory, rollback can fail, but there is nothing we can do about it,
     * apart from reporting
     */
    for(i=nentry - 1; i>=0; i--)
    {
        entry = &mattr->entries[i];
        attr = &mo->drv->aattr[entry->aid];
        bdmf_mset_write_old(mo, entry, &pbuf);
    }
}

/* Set a number of object attributes in a single call */
int bdmf_mattr_set(bdmf_object_handle mo, bdmf_mattr_handle hmattr)
{
    const bdmf_mattr_t *mattr = (const bdmf_mattr_t *)hmattr;
    const bdmf_mattr_entry_t *entry = NULL;
    char *roll_back_buf=NULL, *pbuf;
    struct bdmf_attr *attr;
    bdmf_index index;
    int i;
    int rc = 0;

    if (!mo || !mattr || !mattr->drv)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "bdmf_mattr_set: got NULL in parameters. Next time I'll crash on you!\n");
    }
    if (!mattr->num_entries)
        return 0;
    if (mattr->oper == bdmf_attr_op_get)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "mattr/get is not allowed in bdmf_mattr_set\n");
    if (mattr->drv != mo->drv)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "%s: mattr_set used with wrong object type %s\n",
            mattr->drv->name, mo->drv->name);
    }
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_mattr_set, bdmf_hist_point_start, mo, mattr);

    /* If object is being created we need to make sure that all mandatory attributes are set.
     * If we are here to configure "active" object - take global lock
     */
    if (mo->state == bdmf_state_init)
    {
        if (mo->drv->aattr)
        {
            attr = &mo->drv->aattr[0];
            while (attr && attr->name)
            {
                if ((attr->flags & BDMF_ATTR_MANDATORY))
                {
                    bdmf_attr_id aid = (bdmf_attr_id) (attr - mo->drv->aattr);
                    /* Make sure that the attr is present in the mattr */
                    for(i=0; i<mattr->num_entries; i++)
                    {
                        entry = &mattr->entries[i];
                        if (entry->aid == aid)
                            break;
                    }
                    if (!entry)
                    {
                        BDMF_TRACE_ERR_OBJ(mo, "mandatory attribute(s) not set: %s\n", attr->name);
                        rc = BDMF_ERR_PARM;
                        goto mattr_set_exit;
                    }
                }
                ++attr;
            }
        }
    }
    else
    {
        /* Configuring "active" object */
        /* allocate roll-back buffer */
        roll_back_buf = mattr_rollback_alloc(mo, mattr);
        if (!roll_back_buf)
        {
            BDMF_TRACE_ERR_OBJ(mo, "mattr_set: can't allocate roll-back buffer\n");
            rc = BDMF_ERR_NOMEM;
            goto mattr_set_exit;
        }
        bdmf_lock();
    }

    pbuf = roll_back_buf;
    for(i=0; i<mattr->num_entries; i++)
    {
        entry = &mattr->entries[i];
        index = entry->index;
        attr = &mo->drv->aattr[entry->aid];
        /* Read old value for rollback 1st if configuring active object.
         * don't read for the last entry
         */
        if (roll_back_buf && i < mattr->num_entries - 1)
            rc = bdmf_mset_read_old(mo, entry, &pbuf);
        else
            rc = 0;
        switch (entry->val.val_type)
        {
        case bdmf_attr_number: /**< Numeric attribute */
            rc = rc ? rc : _bdmf_attrelem_set_as_num(mo, attr, &index, entry->val.x.num, 0);
            break;
        case bdmf_attr_string: /**< 0-terminated string */
            rc = rc ? rc : _bdmf_attrelem_set_as_string(mo, attr, &index, entry->val.x.s, 0);
            break;
        case bdmf_attr_buffer: /**< Buffer with binary data */
            rc = rc ? rc : _bdmf_attrelem_set_as_buf(mo, attr, &index,
                    entry->val.x.buf.ptr, entry->val.x.buf.len, 0);
            break;
        default:
            rc = BDMF_ERR_NOT_SUPPORTED;
            BUG();
            break;
        }
        if (rc < 0)
            break;
    }
    if (rc > 0)
        rc = 0;
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_mattr_set, bdmf_hist_point_end, rc);

    /* Roll-back if failed and not creating new objects */
    if (rc && roll_back_buf)
        bdmf_mattr_rollback(mo, mattr, pbuf, entry);

    /* Release lock  */
    if (mo->state != bdmf_state_init)
        bdmf_unlock();

    if (roll_back_buf)
        bdmf_free(roll_back_buf);

mattr_set_exit:
    bdmf_mattr_free(hmattr);
    BDMF_TRACE_RET_OBJ(rc, mo, "bdmf_mattr_set()\n");
}

/** Get a number of object attributes in a single call.
 * \param[in]       mo      Managed object handle
 * \param[in,out]   mattr   Attribute set to be fetched
 * \return
 *     0      - OK \n
 *    <0      - error
 */
int bdmf_mattr_get(bdmf_object_handle mo, bdmf_mattr_handle hmattr)
{
    bdmf_mattr_t *mattr = (bdmf_mattr_t *)hmattr;
    bdmf_mattr_entry_t *entry;
    struct bdmf_attr *attr;
    int i;
    int rc = 0;

    if (!mo || !mattr || !mattr->drv)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "bdmf_mattr_get: got NULL in parameters. Next time I'll crash on you!\n");
    }
    if (!mattr->num_entries)
        return 0;
    if (mattr->oper == bdmf_attr_op_set)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "mattr/set is not allowed in bdmf_mattr_get\n");
    if (mattr->drv != mo->drv)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "mattr_get used with wrong object type. Should be %s\n",
            mattr->drv->name);
    }

    bdmf_lock();

    for(i=0; i<mattr->num_entries; i++)
    {
        entry = &mattr->entries[i];
        attr = &mo->drv->aattr[entry->aid];
        switch (entry->val.val_type)
        {
        case bdmf_attr_number: /**< Numeric attribute */
            rc = _bdmf_attrelem_get_as_num(mo, attr, entry->index,
                    entry->val.x.pnum);
            break;
        case bdmf_attr_string: /**< 0-terminated string */
            rc = _bdmf_attrelem_get_as_string(mo, attr, entry->index,
                    entry->val.x.buf.ptr, entry->val.x.buf.len);
            break;
        case bdmf_attr_buffer: /**< Buffer with binary data */
            rc = _bdmf_attrelem_get_as_buf(mo, attr, entry->index,
                    entry->val.x.buf.ptr, entry->val.x.buf.len);
            break;
        default:
            rc = BDMF_ERR_NOT_SUPPORTED;
            BUG();
            break;
        }
        if (rc < 0)
            break;
    }

    bdmf_unlock();

    return (rc < 0) ? rc : 0;
}

/** Allocate mattr descriptor */
bdmf_mattr_handle bdmf_mattr_alloc(bdmf_type_handle drv)
{
    bdmf_mattr_t *mattr = (bdmf_mattr_t *)bdmf_alloc(
        sizeof(bdmf_mattr_t) + drv->nattrs*sizeof(bdmf_mattr_entry_t));
    if (!mattr)
        return NULL;
    bdmf_mattr_init(mattr, drv);
    mattr->dynamic = 1;
    return (bdmf_mattr_handle)mattr;
}

/** Release mattr chain
 *
 * \param[in]       mattr   Mattr to be released
 */
void bdmf_mattr_free(bdmf_mattr_handle hmattr)
{
    bdmf_mattr_t *mattr = (bdmf_mattr_t *)hmattr;
    bdmf_mattr_entry_t *entry = NULL;
    int i;

    if (!mattr)
        return;
    for (i = 0; i < mattr->num_entries; i++) {
        entry = &mattr->entries[i];
        switch (entry->val.val_type) {
        case bdmf_attr_number: /**< Numeric attribute */
            break;
        case bdmf_attr_string: /**< 0-terminated string */
            bdmf_free((void *)entry->val.x.s);
            break;
        case bdmf_attr_buffer: /**< Buffer with binary data */
            bdmf_free(entry->val.x.buf.ptr);
            break;
        default:
            BDMF_TRACE_ERR("attr type %d not supported", entry->val.val_type);
            break;
        }
    }
    if (mattr->dynamic)
        bdmf_free(mattr);
    else
        mattr->num_entries = 0;
}


/** Add attribute array element value as number. */
int bdmf_attrelem_add_as_num(bdmf_object_handle mo, bdmf_attr_id aid, bdmf_index *index, bdmf_number val)
{
    struct bdmf_attr *attr;
    int rc;
    if (!index)
        return BDMF_ERR_PARM;
    BDMF_ATTR_ID_TO_ATTR(mo, aid, mo, attr);
    if (!attr->add || !attr->del)
        return BDMF_ERR_NOT_SUPPORTED;
    bdmf_attr_lock(mo, attr);
    rc = _bdmf_attrelem_set_as_num(mo, attr, index, val, 1);
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_add_as_num, bdmf_hist_point_both, mo, aid, index, val, rc, index);
    bdmf_attr_unlock(mo, attr);
    BDMF_TRACE_RET_OBJ(rc, mo, "attribute:%s  index:%ld value:%lld\n", attr->name, *index, (long long)val);
    return rc;
}

/** Add attribute array element value as string. */
int bdmf_attrelem_add_as_string(bdmf_object_handle mo, bdmf_attr_id aid, bdmf_index *index, const char *val)
{
    struct bdmf_attr *attr;
    int rc;
    if (!index)
        return BDMF_ERR_PARM;
    BDMF_ATTR_ID_TO_ATTR(mo, aid, mo, attr);
    if (!attr->add)
        return BDMF_ERR_NOT_SUPPORTED;
    bdmf_attr_lock(mo, attr);
    rc = _bdmf_attrelem_set_as_string(mo, attr, index, val, 1);
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_add_as_string, bdmf_hist_point_both,
        mo, aid, index, val, rc, index);
    bdmf_attr_unlock(mo, attr);
    BDMF_TRACE_RET_OBJ(rc, mo, "attribute:%s  index:%ld value:%s\n", attr->name, *index, val);
    return rc;
}

/** Add attribute array element value as buffer. */
int bdmf_attrelem_add_as_buf(bdmf_object_handle mo, bdmf_attr_id aid, bdmf_index *index, const void *buffer,
    uint32_t size)
{
    struct bdmf_attr *attr;
    int rc;
    if (!index)
        return BDMF_ERR_PARM;
    BDMF_ATTR_ID_TO_ATTR(mo, aid, mo, attr);
    if (!attr->add)
        return BDMF_ERR_NOT_SUPPORTED;
    bdmf_attr_lock(mo, attr);
    rc = _bdmf_attrelem_set_as_buf(mo, attr, index, buffer, size, 1);
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_add_as_buf, bdmf_hist_point_both, mo, aid, index, buffer, size, rc, index);
    bdmf_attr_unlock(mo, attr);
    BDMF_TRACE_RET_OBJ(rc, mo, "attribute:%s  index:%ld\n", attr->name, *index);
    return rc;
}


/** Delete attribute array element */
int bdmf_attrelem_delete(bdmf_object_handle mo, bdmf_attr_id aid, bdmf_index index)
{
    struct bdmf_attr *attr;
    int rc;
    BDMF_ATTR_ID_TO_ATTR(mo, aid, mo, attr);
    if (!attr->del)
        return BDMF_ERR_NOT_SUPPORTED;
    bdmf_attr_lock(mo, attr);
    if (attr->type == bdmf_attr_aggregate)
        rc = _bdmf_delete_aggr(mo, attr, index);
    else if (attr->type == bdmf_attr_object)
        rc = _bdmf_delete_ref(mo, attr, index);
    else
        rc = attr->del(mo, attr, index);
    BDMF_HISTORY_BI_EVENT(bdmf_hist_ev_delete, bdmf_hist_point_both, mo, aid, index, rc);
    bdmf_attr_unlock(mo, attr);
    BDMF_TRACE_RET_OBJ(rc, mo, "attribute:%s  index:%ld\n", attr->name, index);
    return rc;
}

/** Get next attribute array index */
int bdmf_attrelem_get_next(bdmf_object_handle mo, bdmf_attr_id aid, bdmf_index *index)
{
    struct bdmf_attr *attr;
    BDMF_ATTR_ID_TO_ATTR(mo, aid, mo, attr);
    if (!index)
        return BDMF_ERR_PARM;
    if (!attr->get_next)
        return BDMF_ERR_NOT_SUPPORTED;
    return attr->get_next(mo, attr, index);
}


/** Find attribute array element index given its value as string.
 *
 * \param[in]       mo          Managed object handle
 * \param[in]       attr        Attribute handle
 * \param[in, out]  index       Array element index (handle). On input may contain a hint
 * \param[in]       val         Value in external format - 0-terminated string
 * \return
 *     0 - OK\n
 *     error code otherwise
 */
int bdmf_attrelem_find_by_string(bdmf_object_handle mo, bdmf_attr_id aid, bdmf_index *index, const char *val)
{
    struct bdmf_attr *attr;
    int rc;
    char *int_val;

    BDMF_ATTR_ID_TO_ATTR(mo, aid, mo, attr);
    if (!index)
        return BDMF_ERR_PARM;
    if (!attr->s_to_val || !attr->size)
        return BDMF_ERR_NOT_SUPPORTED;

    int_val = bdmf_alloc(attr->size);
    if (!int_val)
        return BDMF_ERR_NOMEM;

    rc = attr->s_to_val(mo, attr, val, int_val, attr->size);
    rc = (rc < 0) ? rc : bdmf_attrelem_find(mo, aid, index, int_val, attr->size);
    bdmf_free(int_val);

    return rc;
}


/** Find attribute array element index given its value in internal format.
 *
 * \param[in]       mo          Managed object handle
 * \param[in]       attr        Attribute handle
 * \param[in,out]   buffer      Buffer containing search value or partial search value. Can be updated.
 * \param[in, out]  index       Array element index (handle). On input may contain a hint
 * \param[in]       size        Buffer size
 * \return
 *     >=0 - number of bytes copied
 *     error code otherwise
 */
int bdmf_attrelem_find(bdmf_object_handle mo, bdmf_attr_id aid, bdmf_index *index, void *buffer,
    uint32_t size)
{
    struct bdmf_attr *attr;
    BDMF_ATTR_ID_TO_ATTR(mo, aid, mo, attr);
    if (!index)
        return BDMF_ERR_PARM;
    if (!attr->find || !attr->array_size)
        return BDMF_ERR_NOT_SUPPORTED;
    return attr->find(mo, attr, index, buffer, size);
}

/*
 * Exports
 */
EXPORT_SYMBOL(bdmf_attr_by_name);
EXPORT_SYMBOL(bdmf_attrelem_get_as_buf);
EXPORT_SYMBOL(bdmf_attrelem_set_as_buf);
EXPORT_SYMBOL(bdmf_attrelem_get_as_num);
EXPORT_SYMBOL(bdmf_attrelem_set_as_num);
EXPORT_SYMBOL(bdmf_attrelem_get_as_string);
EXPORT_SYMBOL(bdmf_attrelem_set_as_string);
EXPORT_SYMBOL(bdmf_configure);
EXPORT_SYMBOL(bdmf_attr_get_hlp);
EXPORT_SYMBOL(bdmf_attr_get_num_hlp);
EXPORT_SYMBOL(bdmf_attr_get_enum_val_hlp);
EXPORT_SYMBOL(bdmf_attr_get_enum_hlp);
EXPORT_SYMBOL(bdmf_attr_get_enum_text_hlp);
EXPORT_SYMBOL(bdmf_attr_help);
EXPORT_SYMBOL(bdmf_attr_help_compact);
EXPORT_SYMBOL(bdmf_attr_aggregate_type_register);
EXPORT_SYMBOL(bdmf_attr_aggregate_type_unregister);
EXPORT_SYMBOL(bdmf_attr_aggregate_type_find);
EXPORT_SYMBOL(bdmf_attr_aggr_type_put);
EXPORT_SYMBOL(bdmf_attr_aggregate_type_get_next);
EXPORT_SYMBOL(bdmf_mattr_set);
EXPORT_SYMBOL(bdmf_mattr_get);
EXPORT_SYMBOL(bdmf_mattr_alloc);
EXPORT_SYMBOL(bdmf_mattr_free);
EXPORT_SYMBOL(bdmf_attrelem_add_as_num);
EXPORT_SYMBOL(bdmf_attrelem_add_as_string);
EXPORT_SYMBOL(bdmf_attrelem_add_as_buf);
EXPORT_SYMBOL(bdmf_attrelem_delete);
EXPORT_SYMBOL(bdmf_attrelem_get_next);
EXPORT_SYMBOL(bdmf_attrelem_find_by_string);
EXPORT_SYMBOL(bdmf_attrelem_find);
EXPORT_SYMBOL(bdmf_attr_val_to_s_object);
EXPORT_SYMBOL(bdmf_attr_s_to_val_object);
