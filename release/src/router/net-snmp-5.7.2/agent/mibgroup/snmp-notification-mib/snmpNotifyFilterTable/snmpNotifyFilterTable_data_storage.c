/*
 * Note: this file was created to separate data storage from MIB implementation
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

/*
 * standard Net-SNMP includes
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/*
 * include our parent header
 */
#include "snmpNotifyFilterTable_data_storage.h"
#include "notification/snmpNotifyTable_data.h"
#include "notification/snmpNotifyFilterProfileTable_data.h"

#include <net-snmp/agent/table_container.h>
#include <net-snmp/library/container.h>

#include <ctype.h>


netsnmp_feature_require(CONTAINER_FREE_ALL)


/**********************************************************************
 **********************************************************************
 ***
 *** Table snmpNotifyFilter
 ***
 **********************************************************************
 **********************************************************************/
static netsnmp_container *_container = NULL;
static int _active = 0;

static void
_snmpNotifyFilter_parse(const char *token, char *buf);

static void 
_nf_free_item(void *data, void *dontcare)
{
    snmpNotifyFilter_storage_dispose((snmpNotifyFilter_data_storage *)data);
}

netsnmp_container *
snmpNotifyFilter_storage_container_create(void)
{
    if (NULL == _container) {
        _container =
            netsnmp_container_find("snmpNotifyFilter:table_container");
        if (NULL == _container) {
            snmp_log(LOG_ERR, "error creating container in "
                     "snmpNotifyFilter_storage_container_create\n");
            return NULL;
        }
        _container->container_name =
            strdup("snmpNotifyFilterTable_data_storage");
        _container->free_item = _nf_free_item;
    }
    return _container;
}

void
init_snmpNotifyFilterTable_data_storage(void)
{
    if (NULL == _container)
        (void) snmpNotifyFilter_storage_container_create();

    register_config_handler(NULL, "notificationFilter",
                            _snmpNotifyFilter_parse, NULL, NULL);

}

void
shutdown_snmpNotifyFilterTable_data_storage(void)
{
    if (NULL == _container)
        return;

    CONTAINER_FREE_ALL(_container, NULL);
    CONTAINER_FREE(_container);
    _container = NULL;
    DEBUGMSGTL(("trap:notifyFilter:storage:shutdown",
                "active count %d\n", _active));
    if (_active != 0) {
        DEBUGMSGTL(("trap:notifyFilter:storage:shutdown",
                    "unexpected count %d after cleanup!\n",_active));
        snmp_log(LOG_WARNING,
                 "notifyFilter count %d, not 0, after shutdown.\n", _active);
    }



}

snmpNotifyFilter_data_storage *
snmpNotifyFilter_storage_create(const u_char *name, size_t name_len,
                                const oid *subtree, size_t subtree_len)
{
    snmpNotifyFilter_data_storage *data;
    int subtree_bytes = subtree_len * sizeof(oid);

    DEBUGMSGTL(("verbose:snmpNotifyFilter:storage:create", "called\n"));

    /*
     * check that neither Name or Subtree are larger that maximun sizes
     * and that their combined length doesn't exceed the table max index len.
     * (+ 1 is for name length)
     */
    if ((name_len > sizeof(data->snmpNotifyFilterProfileName)) ||
        ((subtree_bytes > sizeof(data->snmpNotifyFilterSubtree)) ||
         ((name_len + subtree_len + 1) > MAX_snmpNotifyFilterTable_IDX_LEN))) {
        DEBUGMSGTL(("snmpNotifyFilter:storage:create",
                    "index(es) too long\n"));
        return NULL;
    }

    /** allocate memory */
    data = SNMP_MALLOC_TYPEDEF(snmpNotifyFilter_data_storage);
    if (NULL == data) {
        snmp_log(LOG_ERR, "memory allocation failed\n");
        return NULL;
    }
    ++_active;

    /** copy data */
    data->snmpNotifyFilterProfileName_len = name_len;
    memcpy(data->snmpNotifyFilterProfileName, name, name_len);

    data->snmpNotifyFilterSubtree_len = subtree_len;
    memcpy(data->snmpNotifyFilterSubtree, subtree, subtree_bytes);

    return data;
}

void
snmpNotifyFilter_storage_dispose(snmpNotifyFilter_data_storage *data)
{
    free(data);
    --_active;
}

int
snmpNotifyFilter_storage_insert(snmpNotifyFilter_data_storage *data)
{
    int     rc, i;
    oid     *pos;

    if (NULL == data)
        return SNMPERR_GENERR;

   /*
     * copy oid index and insert row
     */
    pos = data->oid_idx.oids = data->oid_tmp;
    *pos++ = data->snmpNotifyFilterProfileName_len;
    data->oid_idx.len = 1;

    /** index: name */
    for( i=0; i < data->snmpNotifyFilterProfileName_len;
         ++i, ++data->oid_idx.len)
        *pos++ = data->snmpNotifyFilterProfileName[i];

    /** index: subtree */
    memcpy(pos, data->snmpNotifyFilterSubtree,
           data->snmpNotifyFilterSubtree_len * sizeof(oid));
    data->oid_idx.len += data->snmpNotifyFilterSubtree_len;

    DEBUGMSGTL(("internal:snmpNotifyFilter", "inserting row\n"));
    rc = CONTAINER_INSERT(_container, data);
    if (0 != rc)
        return SNMPERR_GENERR;

    return SNMPERR_SUCCESS;
}

int
snmpNotifyFilter_storage_remove(snmpNotifyFilter_data_storage *data)
{
    int     rc;

    if (NULL == data)
        return SNMPERR_GENERR;

    DEBUGMSGTL(("internal:snmpNotifyFilter", "removing row\n"));
    rc = CONTAINER_REMOVE(_container, data);
    if (0 != rc)
        return SNMPERR_GENERR;

    return SNMPERR_SUCCESS;
}

snmpNotifyFilter_data_storage *
snmpNotifyFilter_storage_add(const u_char *profileName, size_t profileName_len,
                             const oid *filterSubtree,
                             size_t filterSubtree_len, u_char *filterMask,
                             size_t filterMask_len, u_long filterType)
{
    snmpNotifyFilter_data_storage *data;

    data = snmpNotifyFilter_storage_create(profileName, profileName_len,
                                           filterSubtree, filterSubtree_len);
    if (NULL == data)
        return NULL;

    memcpy(data->snmpNotifyFilterMask, filterMask, filterMask_len);

    data->snmpNotifyFilterType = filterType;

    if (snmpNotifyFilter_storage_insert(data) != SNMPERR_SUCCESS) {
        snmpNotifyFilter_storage_dispose(data);
        return NULL;
    }

    return data;
}

/*
 * ugly, inefficient hack: create a dummy viewEntry list from the filter table
 * entries matching a profile name. This lets us use the existing vacm
 * routines for matching oids to views.
 */
struct vacm_viewEntry *
snmpNotifyFilter_vacm_view_subtree(const char *profile)
{
    oid             tmp_oid[MAX_OID_LEN];
    netsnmp_index   tmp_idx;
    size_t          i, j;
    netsnmp_void_array *s;
    struct vacm_viewEntry *tmp;
    snmpNotifyFilter_data_storage *data;

   if ((NULL == profile) || (NULL == _container))
        return NULL;

    tmp_idx.len = 0;
    tmp_idx.oids = tmp_oid;

    /*
     * get the profile subset
     */
    tmp_idx.oids[0] = strlen(profile);
    tmp_idx.len = tmp_idx.oids[0] + 1;
    for (i = 0; i < tmp_idx.len; ++i)
        tmp_idx.oids[i + 1] = profile[i];
    s = _container->get_subset(_container, &tmp_idx);
    if (NULL == s)
        return NULL;

    /*
     * allocate temporary storage
     */
    tmp = (struct vacm_viewEntry*)calloc(sizeof(struct vacm_viewEntry),
                                         s->size + 1);
    if (NULL == tmp) {
        free(s->array);
        free(s);
        return NULL;
    }

    /*
     * copy data
     */
    for (i = 0, j = 0; i < s->size; ++i) {
        data = (snmpNotifyFilter_data_storage *) s->array[i];

        /*
         * must match profile name exactly, and subset will return
         * longer matches, if they exist.
         */
        if (tmp_idx.oids[0] != data->snmpNotifyFilterProfileName_len)
            continue;

        /*
         * exact match, copy data
         * vacm_viewEntry viewName and viewSubtree are prefixed with length
         */
        tmp[j].viewName[0] = data->snmpNotifyFilterProfileName_len;
        memcpy(&tmp[j].viewName[1], data->snmpNotifyFilterProfileName,
               tmp[j].viewName[0]);

        tmp[j].viewSubtree[0] = data->snmpNotifyFilterSubtree_len;
        memcpy(&tmp[j].viewSubtree[1], data->snmpNotifyFilterSubtree,
               tmp[j].viewSubtree[0] * sizeof(oid));
        tmp[j].viewSubtreeLen = tmp[j].viewSubtree[0] + 1;

        tmp[j].viewMaskLen = data->snmpNotifyFilterMask_len;
        memcpy(tmp[j].viewMask, data->snmpNotifyFilterMask,
               tmp[j].viewMaskLen * sizeof(oid));

        tmp[j].viewType = data->snmpNotifyFilterType;

        tmp[j].next = &tmp[j + 1];
        ++j;
    }
    if (j)
        tmp[j - 1].next = NULL;
    else {
        SNMP_FREE(tmp);
    }

    free(s->array);
    free(s);

    return tmp;
}


/*
 * parse values from configuration file
 */
static void
_snmpNotifyFilter_parse(const char *token, char *buf)
{
    snmpNotifyFilter_data_storage  data, *row;
    size_t                 len, tmp_len;
    char                   type_buf[9]; /* include/excluded */

    if (strcmp(token, "notificationFilter") != 0) {
        snmp_log(LOG_ERR,
                 "unknown token in _snmpNotifyFilter_parse\n");
        return;
    }

    DEBUGMSGTL(("internal:snmpNotifyFilter:parse", "line '%s'\n", buf));

    if (NULL == buf)
        goto bail;

    /*
     * profile name
     */
    data.snmpNotifyFilterProfileName_len =
        sizeof(data.snmpNotifyFilterProfileName);
    buf = read_config_read_memory(ASN_OCTET_STR, buf,
                                  (char *) &data.snmpNotifyFilterProfileName,
                                  (size_t *) &data.snmpNotifyFilterProfileName_len);
    if (NULL == buf)
        goto bail;

    /*
     * subtree
     */
    tmp_len = sizeof(data.snmpNotifyFilterSubtree);
    buf = read_config_read_memory(ASN_OBJECT_ID, buf,
                                  (char *) &data.snmpNotifyFilterSubtree,
                                  (size_t *) &tmp_len);
    if (NULL == buf)
        goto bail;
    data.snmpNotifyFilterSubtree_len = tmp_len / sizeof(oid);

    /*
     * mask
     */
    data.snmpNotifyFilterMask_len = sizeof(data.snmpNotifyFilterMask);
    buf = read_config_read_memory(ASN_OCTET_STR, buf,
                                  (char *) &data.snmpNotifyFilterMask,
                                  (size_t *) &data.snmpNotifyFilterMask_len);
    if (buf == NULL)
        goto bail;

    /*
     * type
     */
    type_buf[0] = 0; /* empty string, in case read_config_read_memory fails */
    len = sizeof(type_buf);
    buf = read_config_read_memory(ASN_OCTET_STR, buf, type_buf,
                                  (size_t *) &len);
    if (strcasecmp(type_buf, "included") == 0)
        data.snmpNotifyFilterType = 1;
    else if (strcasecmp(type_buf, "excluded") == 0)
        data.snmpNotifyFilterType = 2;
    else {
        config_perror("type must be 'excluded' or 'included'");
        goto bail;
    }

    /** ignore any other data */
    if (NULL != buf)
        snmp_log(LOG_WARNING, "ignoring data after notificationFilter\n");

    row = snmpNotifyFilter_storage_add(data.snmpNotifyFilterProfileName,
                                       data.snmpNotifyFilterProfileName_len,
                                       data.snmpNotifyFilterSubtree,
                                       data.snmpNotifyFilterSubtree_len,
                                       data.snmpNotifyFilterMask,
                                       data.snmpNotifyFilterMask_len,
                                       data.snmpNotifyFilterType);
    if (NULL == row)
        snmp_log(LOG_ERR,"couldn't add notification filter\n");

    return;

  bail:
    config_perror("error parsing notificationFilter\n");

   return;
}
