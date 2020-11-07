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
 * bdmf_codegen.c
 *
 * BDMF framework - code generator
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
#define _GNU_SOURCE
#include <bdmf_dev.h>
#include <bdmf_session.h>
#include <bdmf_shell.h>

#define BDMF_ACTION_MASK_ATTR_ENUM              1
#define BDMF_ACTION_MASK_KEY                    2
#define BDMF_ACTION_MASK_ATTR_AGGREGATE         4
#define BDMF_ACTION_MASK_ATTR_ALL               8
#define BDMF_ACTION_MASK_ATTR_ACCESS            0x10
#define BDMF_ACTION_MASK_ATTR_MACCESS           0x20
#define BDMF_ACTION_MASK_ATTR_GEN_ACCESS        0x40

#define BDMF_ACTION_MASK_ALL (  BDMF_ACTION_MASK_ATTR_ENUM    | \
                                BDMF_ACTION_MASK_KEY          | \
                                BDMF_ACTION_MASK_ATTR_ALL     | \
                                BDMF_ACTION_MASK_ATTR_ACCESS  | \
                                BDMF_ACTION_MASK_ATTR_MACCESS | \
                                BDMF_ACTION_MASK_ATTR_GEN_ACCESS )
#define BDMF_ACTION_MASK_ALL_NO_GEN_ACCESS (BDMF_ACTION_MASK_ALL & ~BDMF_ACTION_MASK_ATTR_GEN_ACCESS)

#define BDMF_MAX_TYPE_STACK_DEPTH 32

#define BDMF_FILE_HEADER      1
#define BDMF_FILE_GPL_SHIM    2
#define BDMF_FILE_USR         4

#define BDMF_TAB "    "
#define BDMF_TAB2 BDMF_TAB BDMF_TAB
#define BDMF_TAB3 BDMF_TAB2 BDMF_TAB
#define BDMF_TAB4 BDMF_TAB2 BDMF_TAB2
#define BDMF_TAB5 BDMF_TAB4 BDMF_TAB

#define BDMF_MAX_PYGEN_MANGLE_NAME_LEN 64
#define BDMF_MAX_CODEGEN_FILENAME_LEN 256

/* Function / description.
 * Used for extending auto-generated text
 */
struct func_desc {
    char *name;
    char *desc;
    int is_tagged;      /* 1 if fdesc contains \param or \return tag */
    STAILQ_ENTRY(func_desc) list;
};

static STAILQ_HEAD(_func_desc_list, func_desc) func_desc_list;

#define CODEGEN_FSTART_TAG      "_FUNC"
#define CODEGEN_FEND_TAG        "_EOF"

/* name, type list */
typedef struct codegen_name_type
{
    const char *name;
    bdmf_attr_type_t type;
    TAILQ_ENTRY(codegen_name_type) list;
} codegen_name_type;

/* codegen parameter block */
typedef struct {
    bdmf_session_handle session;
    const char *odir;
    const char *name_prefix;
    uint32_t action_mask;
    int attr_level;
    int gpl_shim;
    int user_file;
    struct bdmf_type *drv;
    FILE *hf;
    char attr_id[64];
    char keytype[64];
    char keyname[64];
    TAILQ_HEAD(codegen_name_type_list, codegen_name_type) name_type_list;
} codegen_parm_t;

typedef enum {
    func_read,
    func_write,
    func_add,
    func_delete,
    func_get_next,
    func_find
} bdmf_func_type;

#define MAX_STR 256

typedef enum {
    PYGEN_GEN_TYPE_CLASS,
    PYGEN_GEN_TYPE_VALIDATOR,
} pygen_gen_type;

static int bdmf_codegen_pygen_types_from_attr_list(codegen_parm_t *p, const struct bdmf_attr *aa, pygen_gen_type gen_type);

static const char *_bdmf_mon_attr_val_type(bdmf_session_handle session, const struct bdmf_attr *attr, int is_read)
{
    static char type_name[64];
    switch(attr->type)
    {
    case bdmf_attr_number:       /**< Numeric attribute */
    case bdmf_attr_enum_mask:    /**< Enum mask attribute */
        if (attr->data_type_name)
            snprintf(type_name, sizeof(type_name), "%s", attr->data_type_name);
        else
            strcpy(type_name, "bdmf_number");
        break;
    case bdmf_attr_string:       /**< 0-terminated string */
        strcpy(type_name, is_read ? "char *" : "const char *");
        break;
    case bdmf_attr_buffer:       /**< Buffer with binary data */
        strcpy(type_name, is_read ? "void *" : "const void *");
        break;
    case bdmf_attr_pointer:      /**< A pointer */
        strcpy(type_name, "void *");
        break;
    case bdmf_attr_object:      /**< Object reference */
        strcpy(type_name, "bdmf_object_handle");
        break;
    case bdmf_attr_ether_addr:   /**< 6-byte Ethernet h/w address */
        strcpy(type_name, is_read ? "bdmf_mac_t *" : "const bdmf_mac_t *");
        break;
    case bdmf_attr_ip_addr:      /**< 4-byte IPv4 address or 16-byte IPv6 address */
        strcpy(type_name, is_read ? "bdmf_ip_t *" : "const bdmf_ip_t *");
        break;
    case bdmf_attr_ipv4_addr:    /**< 4-byte IPv4 address */
        strcpy(type_name, "bdmf_ipv4");
        break;
    case bdmf_attr_ipv6_addr:    /**< 16-byte IPv6 address */
        strcpy(type_name, is_read ? "bdmf_ipv6_t *" : "const bdmf_ipv6_t *");
        break;
    case bdmf_attr_boolean:      /**< boolean. default(first) value = true (1) */
        strcpy(type_name, "bdmf_boolean");
        break;
    case bdmf_attr_enum:         /**< enumeration with list of values in static table */
        if (attr->ts.enum_table->type_name)
            snprintf(type_name, sizeof(type_name), "%s", attr->ts.enum_table->type_name);
        else
            strcpy(type_name, "int");
        break;
    case bdmf_attr_dyn_enum:     /**< dynamic enumeration with list of values generated by callback */
        strcpy(type_name, "int");
        break;
    case bdmf_attr_aggregate:    /**< aggregate type: "structure" consisting of multiple attributes */
        if (!attr->aggr_type->struct_name)
        {
            bdmf_session_print(session, "CODEGEN: can't generate value type for aggregate type %s, attribute %s\n",
                    attr->ts.aggr_type_name, attr->name);
            return NULL;
        }
        if (is_read)
            snprintf(type_name, sizeof(type_name), "%s *", attr->aggr_type->struct_name);
        else
            snprintf(type_name, sizeof(type_name), "const %s *", attr->aggr_type->struct_name);
        break;
    default:
        bdmf_session_print(session, "CODEGEN: can't generate value type for attribute %s\n", attr->name);
        return NULL;
    }
    return type_name;
}

static const char *_bdmf_mon_attr_index_type(bdmf_session_handle session, const struct bdmf_attr *attr, bdmf_func_type func_type)
{
    int is_read = (func_type==func_add || func_type==func_find || func_type==func_get_next);
    int is_can_modify = is_read || func_type==func_read || func_type==func_write;
    static char type_name[64];

    switch(attr->index_type)
    {
    case bdmf_attr_number:       /**< Numeric attribute */
        strcpy(type_name, is_read ? "bdmf_index *" : "bdmf_index");
        break;
    case bdmf_attr_string:       /**< 0-terminated string */
        strcpy(type_name, is_read ? "char *" : "const char *");
        break;
    case bdmf_attr_buffer:       /**< Buffer with binary data */
        strcpy(type_name, is_read ? "void *" : "const void *");
        break;
    case bdmf_attr_pointer:      /**< A pointer */
        strcpy(type_name, is_read ? "void **" : "void *");
        break;
    case bdmf_attr_object:      /**< Object reference */
        strcpy(type_name, is_read ? "bdmf_object_handle *" : "bdmf_object_handle");
        break;
    case bdmf_attr_ether_addr:   /**< 6-byte Ethernet h/w address */
        strcpy(type_name, is_read ? "bdmf_mac_t *" : "const bdmf_mac_t *");
        break;
    case bdmf_attr_ip_addr:      /**< 4-byte IPv4 address or 16-byte IPv6 address */
        strcpy(type_name, is_read ? "bdmf_ip_t *" : "const bdmf_ip_t *");
        break;
    case bdmf_attr_ipv4_addr:    /**< 4-byte IPv4 address */
        strcpy(type_name, is_read ? "bdmf_ipv4 *" : "bdmf_ipv4");
        break;
    case bdmf_attr_ipv6_addr:    /**< 16-byte IPv6 address */
        strcpy(type_name, is_read ? "bdmf_ipv6_t *" : "const bdmf_ipv6_t *");
        break;
    case bdmf_attr_enum:         /**< enumeration with list of values in static table */
        if (attr->index_type == bdmf_attr_enum && attr->index_ts.enum_table && attr->index_ts.enum_table->type_name)
        {
            snprintf(type_name, sizeof(type_name), "%s%s",
                attr->index_ts.enum_table->type_name, is_read ? " *" : "");
        }
        else
            strcpy(type_name, is_read ? "bdmf_index *" : "bdmf_index");
        break;
    case bdmf_attr_aggregate:    /**< aggregate type: "structure" consisting of multiple attributes */
        if (!attr->index_aggr_type || !attr->index_aggr_type->struct_name)
        {
            bdmf_session_print(session, "CODEGEN: can't generate index type for attribute %s\n", attr->name);
            return NULL;
        }
        if (is_can_modify)
            snprintf(type_name, sizeof(type_name), "%s *", attr->index_aggr_type->struct_name);
        else
            snprintf(type_name, sizeof(type_name), "const %s *", attr->index_aggr_type->struct_name);
        break;
    default:
        bdmf_session_print(session, "CODEGEN: can't generate index type for attribute %s\n", attr->name);
        return NULL;
    }
    return type_name;
}

/* generate get/set attr functions */
static int _bdmf_mon_codegen_gen_attr_gen_access(codegen_parm_t *parm)
{
    const struct bdmf_type *drv=parm->drv;
    const char *name_prefix=parm->name_prefix;
    const char *keytype=parm->keytype;
    const char *keyname=parm->keyname;
    FILE *hf=parm->hf;

    if (!parm->drv->aattr)
        return 0;

    if (strlen(keyname))
    {
        /* has key */
        fprintf(hf, "\n");
        fprintf(hf, "/** Get %s object's attr by key as number\n", drv->name);
        fprintf(hf, " * \\param[in]     %s Object key\n", keyname);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[out]    val Attribute value in numeric format\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_get_as_num(\n", name_prefix, drv->name);
        fprintf(hf, "    %s %s,\n"
                    "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    bdmf_number *val)\n",
                    keytype, keyname, name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get(%s);\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_get_as_num(mo, id, index, val);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name, keyname);


        fprintf(hf, "\n");
        fprintf(hf, "/** Get %s object's attr by key as string\n", drv->name);
        fprintf(hf, " * \\param[in]     %s Object key\n", keyname);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[out]    buffer Attribute value in string format\n");
        fprintf(hf, " * \\param[in]     size buffer size\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_get_as_string(\n", name_prefix, drv->name);
        fprintf(hf, "    %s %s,\n"
                    "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    char *buffer,\n"
                    "    uint32_t size)\n",
                    keytype, keyname, name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get(%s);\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_get_as_string(mo, id, index, buffer, size);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name, keyname);

        fprintf(hf, "\n");
        fprintf(hf, "/** Get %s object's attr by key as buffer\n", drv->name);
        fprintf(hf, " * \\param[in]     %s Object key\n", keyname);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[out]    buffer Attribute value in string format\n");
        fprintf(hf, " * \\param[in]     size buffer size\n");
        fprintf(hf, " * \\return >=0-number of bytes copied, or error code < 0\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_get_as_buf(\n", name_prefix, drv->name);
        fprintf(hf, "    %s %s,\n"
                    "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    void *buffer,\n"
                    "    uint32_t size)\n",
                    keytype, keyname, name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get(%s);\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_get_as_buf(mo, id, index, buffer, size);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name, keyname);

        fprintf(hf, "\n");
        fprintf(hf, "/** Set %s object's attr by key as number\n", drv->name);
        fprintf(hf, " * \\param[in]     %s Object key\n", keyname);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[in]     val Attribute value in numeric format\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_set_as_num(\n", name_prefix, drv->name);
        fprintf(hf, "    %s %s,\n"
                    "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    bdmf_number val)\n",
                    keytype, keyname, name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get(%s);\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_set_as_num(mo, id, index, val);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name, keyname);

        fprintf(hf, "\n");
        fprintf(hf, "/** Set %s object's attr by key as string\n", drv->name);
        fprintf(hf, " * \\param[in]     %s Object key\n", keyname);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[in]     buffer Attribute value in string format\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_set_as_string(\n", name_prefix, drv->name);
        fprintf(hf, "    %s %s,\n"
                    "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    const char *buffer)\n",
                    keytype, keyname, name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get(%s);\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_set_as_string(mo, id, index, buffer);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name, keyname);

        fprintf(hf, "\n");
        fprintf(hf, "/** Set %s object's attr by key as buffer\n", drv->name);
        fprintf(hf, " * \\param[in]     %s Object key\n", keyname);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[in]     buffer Attribute value in string format\n");
        fprintf(hf, " * \\param[in]     size buffer size\n");
        fprintf(hf, " * \\return >=0-number of bytes copied, or error code < 0\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_set_as_buf(\n", name_prefix, drv->name);
        fprintf(hf, "    %s %s,\n"
                    "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    const void *buffer,\n"
                    "    uint32_t size)\n",
                    keytype, keyname, name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get(%s);\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_set_as_buf(mo, id, index, buffer, size);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name, keyname);
    }
    else
    {
        /* no key */
        fprintf(hf, "\n");
        fprintf(hf, "/** Get %s object's attr as number\n", drv->name);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[out]    val Attribute value in numeric format\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_get_as_num(\n", name_prefix, drv->name);
        fprintf(hf, "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    bdmf_number *val)\n",
                    name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get();\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_get_as_num(mo, id, index, val);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name);

        fprintf(hf, "\n");
        fprintf(hf, "/** Get %s object's attr as string\n", drv->name);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[out]    buffer Attribute value in string format\n");
        fprintf(hf, " * \\param[in]     size buffer size\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_get_as_string(\n", name_prefix, drv->name);
        fprintf(hf, "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    char *buffer,\n"
                    "    uint32_t size)\n",
                    name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get();\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_get_as_string(mo, id, index, buffer, size);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name);

        fprintf(hf, "\n");
        fprintf(hf, "/** Get %s object's attr as buffer\n", drv->name);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[out]    buffer Attribute value in string format\n");
        fprintf(hf, " * \\param[in]     size buffer size\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_get_as_buf(\n", name_prefix, drv->name);
        fprintf(hf, "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    void *buffer,\n"
                    "    uint32_t size)\n",
                    name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get();\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_get_as_buf(mo, id, index, buffer, size);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name);

        fprintf(hf, "\n");
        fprintf(hf, "/** Set %s object's attr as number\n", drv->name);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[in]     val Attribute value in numeric format\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_set_as_num(\n", name_prefix, drv->name);
        fprintf(hf, "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    bdmf_number val)\n",
                    name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get();\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_set_as_num(mo, id, index, val);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name);

        fprintf(hf, "\n");
        fprintf(hf, "/** Set %s object's attr as string\n", drv->name);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[in]     buffer Attribute value in string format\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_set_as_string(\n", name_prefix, drv->name);
        fprintf(hf, "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    const char *buffer)\n",
                    name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get();\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_set_as_string(mo, id, index, buffer);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name);

        fprintf(hf, "\n");
        fprintf(hf, "/** Set %s object's attr as buffer\n", drv->name);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[in]     buffer Attribute value in string format\n");
        fprintf(hf, " * \\param[in]     size buffer size\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_set_as_buf(\n", name_prefix, drv->name);
        fprintf(hf, "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    const void *buffer,\n"
                    "    uint32_t size)\n",
                    name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get();\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_set_as_buf(mo, id, index, buffer, size);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name);
    }

    return 0;
}

struct func_desc *_bdmf_mon_codegen_get_fdesc(const char *fname)
{
    struct func_desc *fdesc;
    STAILQ_FOREACH(fdesc, &func_desc_list, list)
    {
        if (!strcmp(fdesc->name, fname))
            break;
    }
    return fdesc;
}

static int _bdmf_mon_codegen_is_numeric(const struct bdmf_attr *attr)
{
    int numeric;
    switch(attr->type)
    {
    case bdmf_attr_number:       /**< Numeric attribute */
    case bdmf_attr_object:       /**< Object reference */
    case bdmf_attr_pointer:      /**< A pointer */
    case bdmf_attr_ipv4_addr:    /**< 4-byte IPv4 address */
    case bdmf_attr_boolean:      /**< boolean. default(first) value = true (1) */
    case bdmf_attr_enum:         /**< enumeration with list of values in static table */
    case bdmf_attr_enum_mask:    /**< enumeration mask */
    case bdmf_attr_dyn_enum:     /**< dynamic enumeration with list of values generated by callback */
        numeric=1;
        break;
    default:
        numeric=0;
        break;
    }
    return numeric;
}

#define CODEGEN_FUNC_SUFFIX_SIZE 32

static void _bdmf_mon_codegen_attr_suffix(const struct bdmf_attr *attr, bdmf_func_type func_type, char *comment, char *suffix)
{
    switch(func_type)
    {
    case func_read:
        strncpy(comment, "Get", CODEGEN_FUNC_SUFFIX_SIZE);
        strncpy(suffix, "_get", CODEGEN_FUNC_SUFFIX_SIZE);
        break;
    case func_write:
    {
        int no_value = (attr->flags & BDMF_ATTR_NO_VALUE) != 0;
        if (no_value)
        {
            strncpy(comment, "Invoke", CODEGEN_FUNC_SUFFIX_SIZE);
            *suffix = 0;
        }
        else
        {
            strncpy(comment, "Set", CODEGEN_FUNC_SUFFIX_SIZE);
            strncpy(suffix, "_set", CODEGEN_FUNC_SUFFIX_SIZE);
        }
        break;
    }
    case func_add:
        strncpy(comment, "Add", CODEGEN_FUNC_SUFFIX_SIZE);
        strncpy(suffix, "_add", CODEGEN_FUNC_SUFFIX_SIZE);
        break;
    case func_get_next:
        strncpy(comment, "Get next", CODEGEN_FUNC_SUFFIX_SIZE);
        strncpy(suffix, "_get_next", CODEGEN_FUNC_SUFFIX_SIZE);
        break;
    case func_find:
        strncpy(comment, "Find", CODEGEN_FUNC_SUFFIX_SIZE);
        strncpy(suffix, "_find", CODEGEN_FUNC_SUFFIX_SIZE);
        break;
    case func_delete:
        strncpy(comment, "Delete", CODEGEN_FUNC_SUFFIX_SIZE);
        strncpy(suffix, "_delete", CODEGEN_FUNC_SUFFIX_SIZE);
        break;
    }
}

/* generate attr read access function */
static int _bdmf_mon_codegen_gen_attr1(codegen_parm_t *parm, const struct bdmf_attr *attr, bdmf_func_type func_type)
{
    const char *attr_val_type;
    const char *attr_index_type;
    char func_name[64];
    int is_read = (func_type==func_read || func_type==func_find);
    struct func_desc *fdesc = NULL;
    char func_suffix_comment[CODEGEN_FUNC_SUFFIX_SIZE], func_suffix[CODEGEN_FUNC_SUFFIX_SIZE];
    int no_value = (func_type == func_write) && (attr->flags & BDMF_ATTR_NO_VALUE) != 0;

    attr_val_type = _bdmf_mon_attr_val_type(parm->session, attr, is_read);
    if (!attr_val_type)
    {
        bdmf_session_print(parm->session, "CODEGEN: can't generate attribute access function for %s/%s\n", parm->drv->name, attr->name);
        return 0;
    }
    attr_index_type = _bdmf_mon_attr_index_type(parm->session, attr, func_type);
    if (!attr_index_type)
    {
        bdmf_session_print(parm->session, "CODEGEN: can't generate attribute access function for %s/%s\n", parm->drv->name, attr->name);
        return 0;
    }
    _bdmf_mon_codegen_attr_suffix(attr, func_type, func_suffix_comment, func_suffix);

    /* "delete" function */
    if (func_type == func_delete)
    {
        fprintf(parm->hf, "\n");

        snprintf(func_name, sizeof(func_name), "%s_%s_%s_delete",
            parm->name_prefix, parm->drv->name, attr->name);
        fdesc = _bdmf_mon_codegen_get_fdesc(func_name);
        if (fdesc && fdesc->is_tagged)
        {
            /* Replace description */
            fprintf(parm->hf, "%s", fdesc->desc);
        }
        else
        {
            fprintf(parm->hf, "/** Delete %s/%s attribute entry.\n",
                parm->drv->name, attr->name);
            fprintf(parm->hf, " *\n");
            fprintf(parm->hf, " * Delete %s.\n", attr->help);
            if (fdesc)
                fprintf(parm->hf, "%s", fdesc->desc);
            if ((attr->flags & BDMF_ATTR_DEPRECATED))
            {
                if (attr->deprecated_text)
                    fprintf(parm->hf, "\n * \\deprecated This function has been deprecated, %s.\n *\n",attr->deprecated_text);
                else
                    fprintf(parm->hf, "\n * \\deprecated This function has been deprecated.\n *\n");
            }
            fprintf(parm->hf, " * \\param[in]   mo_ %s object handle\n", parm->drv->name);
            fprintf(parm->hf, " * \\param[in]   ai_ Attribute array index\n");
            fprintf(parm->hf, " * \\return 0 or error code < 0\n");
            if ((attr->flags & BDMF_ATTR_NOLOCK))
                fprintf(parm->hf, " * The function can be called in task and softirq contexts.\n");
            else
                fprintf(parm->hf, " * The function can be called in task context only.\n");
            fprintf(parm->hf, " */\n");
        }

        fprintf(parm->hf, "static inline int %s(", func_name);
        fprintf(parm->hf, "bdmf_object_handle mo_, %s ai_)\n", attr_index_type);
        fprintf(parm->hf, "{\n");
        fprintf(parm->hf, "    return bdmf_attrelem_delete(mo_, %s, (bdmf_index)ai_);\n", parm->attr_id);
        fprintf(parm->hf, "}\n\n");
        return 0;
    }

    fprintf(parm->hf, "\n");
    snprintf(func_name, sizeof(func_name), "%s_%s_%s%s",
        parm->name_prefix, parm->drv->name, attr->name, func_suffix);
    fdesc = _bdmf_mon_codegen_get_fdesc(func_name);

    if (fdesc && fdesc->is_tagged)
    {
        /* Replace description */
        fprintf(parm->hf, "%s", fdesc->desc);
    }
    else
    {
        /* Auto-generate description */
        fprintf(parm->hf, "/** %s %s/%s attribute%s\n",
            func_suffix_comment,
            parm->drv->name, attr->name, (attr->array_size > 1) ? " entry." : ".");
        fprintf(parm->hf, " *\n");
        fprintf(parm->hf, " * %s %s.\n", func_suffix_comment, attr->help);
        if (fdesc)
            fprintf(parm->hf, "%s", fdesc->desc);
        if ((attr->flags & BDMF_ATTR_DEPRECATED))
        {
            if (attr->deprecated_text)
                fprintf(parm->hf, "\n * \\deprecated This function has been deprecated, %s.\n *\n",attr->deprecated_text);
            else
                fprintf(parm->hf, "\n * \\deprecated This function has been deprecated.\n *\n");
        }
        fprintf(parm->hf, " * \\param[in]   mo_ %s object handle or mattr transaction handle\n",
            parm->drv->name);
        if ((attr->array_size > 1) || (func_type==func_add))
            fprintf(parm->hf, " * \\param[%s]   ai_ Attribute array index\n",
                (func_type==func_add || func_type==func_get_next || func_type==func_find)?"in,out":"in");
        if (func_type==func_read)
            fprintf(parm->hf, " * \\param[out]  %s_ Attribute value\n", attr->name);
        else if (func_type == func_find)
            fprintf(parm->hf, " * \\param[in,out]   %s_ Attribute value\n", attr->name);
        else if ((func_type != func_get_next) && !no_value)
            fprintf(parm->hf, " * \\param[in]   %s_ Attribute value\n", attr->name);
        if ((func_type != func_get_next && func_type != func_find) &&
            (attr->type==bdmf_attr_buffer || (func_type==func_read && attr->type==bdmf_attr_string)))
        {
            fprintf(parm->hf, " * \\param[in]   size_ buffer size\n");
            if (func_type==func_read)
                fprintf(parm->hf, " * \\return number of bytes read >=0 or error code < 0\n");
            else
                fprintf(parm->hf, " * \\return number of bytes written >=0 or error code < 0\n");
        }
        else
            fprintf(parm->hf, " * \\return 0 or error code < 0\n");
        if ((attr->flags & BDMF_ATTR_NOLOCK))
            fprintf(parm->hf, " * The function can be called in task and softirq contexts.\n");
        else
            fprintf(parm->hf, " * The function can be called in task context only.\n");

        fprintf(parm->hf, " */\n");
    }
    fprintf(parm->hf, "static inline int %s(", func_name);
    fprintf(parm->hf, "bdmf_object_handle mo_");
    if (attr->array_size || (func_type==func_add))
        fprintf(parm->hf, ", %s ai_", attr_index_type);
    if ((func_type != func_get_next) && !(func_type == func_write && no_value))
    {
        fprintf(parm->hf, ", %s %s%s_", attr_val_type,
            (is_read && _bdmf_mon_codegen_is_numeric(attr)) ? "*" :"", attr->name);
    }
    if ((func_type != func_get_next && func_type != func_find) &&
         (attr->type==bdmf_attr_buffer || (func_type==func_read && attr->type==bdmf_attr_string)))
        fprintf(parm->hf, ", uint32_t size_)\n");
    else
        fprintf(parm->hf, ")\n");
    fprintf(parm->hf, "{\n");
    if (_bdmf_mon_codegen_is_numeric(attr) && func_type==func_read)
    {
        fprintf(parm->hf, "    bdmf_number _nn_;\n");
        fprintf(parm->hf, "    int _rc_;\n");
    }
    if (func_type==func_read)
    {
        if (attr->type == bdmf_attr_string)
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    return bdmf_attrelem_get_as_string(mo_, %s, (bdmf_index)ai_, %s_, size_);\n",
                        parm->attr_id, attr->name);
            else
                fprintf(parm->hf, "    return bdmf_attr_get_as_string(mo_, %s, %s_, size_);\n",
                        parm->attr_id, attr->name);
        }
        else if (_bdmf_mon_codegen_is_numeric(attr))
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    _rc_ = bdmf_attrelem_get_as_num(mo_, %s, (bdmf_index)ai_, &_nn_);\n",
                        parm->attr_id);
            else
                fprintf(parm->hf, "    _rc_ = bdmf_attr_get_as_num(mo_, %s, &_nn_);\n",
                        parm->attr_id);
            if (attr->type == bdmf_attr_object)
                fprintf(parm->hf, "    *%s_ = (bdmf_object_handle)(long)_nn_;\n", attr->name);
            else if (attr->type == bdmf_attr_pointer)
                fprintf(parm->hf, "    *%s_ = (void *)(long)_nn_;\n", attr->name);
            else
                fprintf(parm->hf, "    *%s_ = (%s)_nn_;\n", attr->name, attr_val_type);
            fprintf(parm->hf, "    return _rc_;\n");
        }
        else if (attr->type == bdmf_attr_buffer)
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    return bdmf_attrelem_get_as_buf(mo_, %s, (bdmf_index)ai_, %s_, size_);\n",
                        parm->attr_id, attr->name);
            else
                fprintf(parm->hf, "    return bdmf_attr_get_as_buf(mo_, %s, %s_, size_);\n",
                        parm->attr_id, attr->name);
        }
        else
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    return bdmf_attrelem_get_as_buf(mo_, %s, (bdmf_index)ai_, %s_, sizeof(*%s_));\n",
                        parm->attr_id, attr->name, attr->name);
            else
                fprintf(parm->hf, "    return bdmf_attr_get_as_buf(mo_, %s, %s_, sizeof(*%s_));\n",
                        parm->attr_id, attr->name, attr->name);
        }
    }
    else if (func_type==func_write)
    {
        if (attr->type == bdmf_attr_string)
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    return bdmf_attrelem_set_as_string(mo_, %s, (bdmf_index)ai_, %s_);\n",
                        parm->attr_id, attr->name);
            else
                fprintf(parm->hf, "    return bdmf_attr_set_as_string(mo_, %s, %s_);\n",
                        parm->attr_id, attr->name);
        }
        else if (_bdmf_mon_codegen_is_numeric(attr))
        {
            if ((attr->type == bdmf_attr_object) || (attr->type == bdmf_attr_pointer))
            {
                if (attr->array_size > 1)
                    fprintf(parm->hf, "    return bdmf_attrelem_set_as_num(mo_, %s, (bdmf_index)ai_, (long)%s_);\n",
                            parm->attr_id, attr->name);
                else
                    fprintf(parm->hf, "    return bdmf_attr_set_as_num(mo_, %s, (long)%s_);\n",
                            parm->attr_id, attr->name);
            }
            else
            {
                char value_name[64];
                if (no_value)
                    strcpy(value_name, "1");
                else
                    snprintf(value_name, sizeof(value_name), "%s_", attr->name);
                if (attr->array_size > 1)
                    fprintf(parm->hf, "    return bdmf_attrelem_set_as_num(mo_, %s, (bdmf_index)ai_, %s);\n",
                            parm->attr_id, value_name);
                else
                    fprintf(parm->hf, "    return bdmf_attr_set_as_num(mo_, %s, %s);\n",
                            parm->attr_id, value_name);
            }
        }
        else if (attr->type == bdmf_attr_buffer)
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    return bdmf_attrelem_set_as_buf(mo_, %s, (bdmf_index)ai_, %s_, size_);\n",
                        parm->attr_id, attr->name);
            else
                fprintf(parm->hf, "    return bdmf_attr_set_as_buf(mo_, %s, %s_, size_);\n",
                        parm->attr_id, attr->name);
        }
        else
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    return bdmf_attrelem_set_as_buf(mo_, %s, (bdmf_index)ai_, %s_, sizeof(*%s_));\n",
                        parm->attr_id, attr->name, attr->name);
            else
                fprintf(parm->hf, "    return bdmf_attr_set_as_buf(mo_, %s, %s_, sizeof(*%s_));\n",
                        parm->attr_id, attr->name, attr->name);
        }
    }
    else if (func_type==func_add)
    {
        char index_name[16]= "ai_";
        fprintf(parm->hf, "    int rc;\n");
        if (attr->index_type == bdmf_attr_enum && attr->index_ts.enum_table && attr->index_ts.enum_table->type_name)
        {
            fprintf(parm->hf, "    bdmf_index _ai_tmp_ = ai_ ? (bdmf_index)(*ai_) : BDMF_INDEX_UNASSIGNED;\n");
            strcpy(index_name, "&_ai_tmp_");
        }
        if (attr->type == bdmf_attr_string)
        {
            fprintf(parm->hf, "    rc = bdmf_attrelem_add_as_string(mo_, %s, (bdmf_index *)%s, %s_);\n",
                    parm->attr_id, index_name, attr->name);
        }
        else if (_bdmf_mon_codegen_is_numeric(attr))
        {
            if ((attr->type == bdmf_attr_object) || (attr->type == bdmf_attr_pointer))
            {
                fprintf(parm->hf, "    rc = bdmf_attrelem_add_as_num(mo_, %s, (bdmf_index *)ai_, (long)%s_);\n",
                        parm->attr_id, attr->name);
            }
            else
            {
                fprintf(parm->hf, "    rc = bdmf_attrelem_add_as_num(mo_, %s, (bdmf_index *)%s, %s_);\n",
                        parm->attr_id, index_name, attr->name);
            }
        }
        else if (attr->type == bdmf_attr_buffer)
        {
            fprintf(parm->hf, "    rc = bdmf_attrelem_add_as_buf(mo_, %s, (bdmf_index *)%s, %s_, size_);\n",
                    parm->attr_id, index_name, attr->name);
        }
        else
        {
            fprintf(parm->hf, "    rc = bdmf_attrelem_add_as_buf(mo_, %s, (bdmf_index *)%s, %s_, sizeof(*%s_));\n",
                    parm->attr_id, index_name, attr->name, attr->name);
        }
        if (attr->index_type == bdmf_attr_enum && attr->index_ts.enum_table && attr->index_ts.enum_table->type_name)
            fprintf(parm->hf, "    *ai_ = (%s)_ai_tmp_;\n", attr->index_ts.enum_table->type_name);
        fprintf(parm->hf, "    return rc;\n");
    }
    else if (func_type==func_get_next)
    {
        char index_name[16]= "ai_";
        int need_rc = 0;
        if (attr->index_type==bdmf_attr_enum && attr->index_ts.enum_table && attr->index_ts.enum_table->type_name)
        {
            fprintf(parm->hf, "    int rc;\n");
            fprintf(parm->hf, "    bdmf_index _ai_tmp_ = *ai_;\n");
            strcpy(index_name, "&_ai_tmp_");
            need_rc = 1;
        }
        fprintf(parm->hf, "    %s bdmf_attrelem_get_next(mo_, %s, (bdmf_index *)%s);\n",
                need_rc ? "rc =" : "return", parm->attr_id, index_name);
        if (need_rc)
        {
            fprintf(parm->hf, "    *ai_ = (%s)_ai_tmp_;\n", attr->index_ts.enum_table->type_name);
            fprintf(parm->hf, "    return rc;\n");
        }
    }
    else if (func_type==func_find)
    {
        char index_name[16]= "ai_";
        fprintf(parm->hf, "    int rc;\n");
        if (attr->index_type == bdmf_attr_enum && attr->index_ts.enum_table && attr->index_ts.enum_table->type_name)
        {
            fprintf(parm->hf, "    bdmf_index _ai_tmp_ = *ai_;\n");
            strcpy(index_name, "&_ai_tmp_");
        }
        fprintf(parm->hf, "    rc = bdmf_attrelem_find(mo_, %s, (bdmf_index *)%s, %s_, sizeof(*%s_));\n",
                parm->attr_id, index_name, attr->name, attr->name);
        if (attr->index_type == bdmf_attr_enum && attr->index_ts.enum_table && attr->index_ts.enum_table->type_name)
            fprintf(parm->hf, "    *ai_ = (%s)_ai_tmp_;\n", attr->index_ts.enum_table->type_name);
        fprintf(parm->hf, "    return rc;\n");
    }
    fprintf(parm->hf, "}\n\n");

    return 0;
}

/* generate get/set attr functions */
static int _bdmf_mon_codegen_gen_attr_access(codegen_parm_t *parm)
{
    const struct bdmf_attr *attr=parm->drv->aattr;
    int rc=0;
    while(attr && attr->name)
    {
        if ((attr->flags & parm->attr_level)!=0 &&
            !(attr->flags & BDMF_ATTR_NO_AUTO_GEN) &&
            ((parm->action_mask & BDMF_ACTION_MASK_ATTR_ALL)!=0 ||
             (attr->type==bdmf_attr_aggregate)) )
        {
            snprintf(parm->attr_id, sizeof(parm->attr_id), "%s_%s_attr_%s",
                    parm->name_prefix, parm->drv->name, attr->name);
            if ((attr->flags & BDMF_ATTR_READ))
                rc = rc ? rc : _bdmf_mon_codegen_gen_attr1(parm, attr, func_read);
            if ((attr->flags & BDMF_ATTR_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
            {
                if ((attr->flags & BDMF_ATTR_UDEF_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
                    rc = rc ? rc : _bdmf_mon_codegen_gen_attr1(parm, attr, func_write);
                if (attr->add)
                    rc = rc ? rc : _bdmf_mon_codegen_gen_attr1(parm, attr, func_add);
                if (attr->del)
                    rc = rc ? rc : _bdmf_mon_codegen_gen_attr1(parm, attr, func_delete);
            }
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_GET_NEXT))
                rc = rc ? rc : _bdmf_mon_codegen_gen_attr1(parm, attr, func_get_next);
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_FIND))
                rc = rc ? rc : _bdmf_mon_codegen_gen_attr1(parm, attr, func_find);
        }
        ++attr;
    }

    return rc;
}


void to_upper_case(char *str)
{
    while(*str)
    {
        *str = toupper(*str);
        ++str;
    }
}

static void generate_func_call_in_ioctl(codegen_parm_t *parm, const struct bdmf_attr *attr, bdmf_func_type func_type, char *func_name, const char *attr_index_type, int is_pointer, const char *attr_val_type, int is_idx_pointer)
{
    FILE *hf = parm->hf;
    int no_value = (func_type == func_write) && (attr->flags & BDMF_ATTR_NO_VALUE) != 0;

    fprintf(hf, "\n\tif ((pa->ret = %s(pa->mo", func_name);
    if (func_type==func_add || func_type==func_find || func_type==func_get_next || attr->array_size)
    {
        if(is_idx_pointer)
            fprintf(hf, ", &ai");
        else
            fprintf(hf, ", pa->ai");
    }
    if ((func_type != func_get_next) && !(func_type == func_write && no_value))
    {
        if (attr->type==bdmf_attr_string)
            fprintf(hf, ", parm");
        else if (is_pointer)
            fprintf(hf, ", &parm");
        else if (attr->type==bdmf_attr_object)
            fprintf(hf, ", pa->object");
        else
            fprintf(hf, ", (%s)(long)pa->parm", attr_val_type);
    }
    if ((func_type != func_get_next && func_type != func_find) &&
        (attr->type==bdmf_attr_buffer ||
        (func_type==func_read && attr->type==bdmf_attr_string)))
        fprintf(hf, ", pa->size)))\n");
    else
        fprintf(hf, ")))\n");
    fprintf(hf, "\t{\n"
            "\t\tBDMF_TRACE_DBG(\"%s failed, ret:%%d\\n\", pa->ret);\n"
            "\t}\n",
            func_name);
    if ((func_type==func_add || func_type==func_find || func_type==func_get_next))
        fprintf(hf, "\n\tif (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(%s)))\n"
                "\t{\n"
                "\t\tBDMF_TRACE_ERR(\"failed to copy to user\\n\");\n"
                "\t\treturn -1;\n"
                "\t}\n",
                attr_index_type);
    if ((func_type==func_read) || (func_type==func_find))
    {
        if(attr->type==bdmf_attr_string)
        {
             fprintf(hf, "\n\tif (copy_to_user((void *)(long)pa->ptr, (void *)parm, %d))\n"
                     "\t{\n"
                     "\t\tBDMF_TRACE_ERR(\"failed to copy to user\\n\");\n"
                     "\t\treturn -1;\n"
                     "\t}\n",
                     attr->size);
        }
        else
        {
            fprintf(hf, "\n\tif (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(%s)))\n"
                    "\t{\n"
                    "\t\tBDMF_TRACE_ERR(\"failed to copy to user\\n\");\n"
                    "\t\treturn -1;\n"
                    "\t}\n",
                    attr_val_type);
        }

    }

    fprintf(hf, "\n\treturn 0;\n"
            "}\n\n");

}

static void generate_delete_func_in_ioctl(codegen_parm_t *parm, const struct bdmf_attr *attr, const char *attr_index_type, int is_idx_pointer)
{
    char func_name[64];
    FILE *hf = parm->hf;

    snprintf(func_name, sizeof(func_name), "%s_%s_%s_delete",
            parm->name_prefix, parm->drv->name, attr->name);
    fprintf(hf, "static int %s_user_%s_%s_delete(rdpa_ioctl_cmd_t *pa)\n"
            "{\n",
            parm->name_prefix, parm->drv->name, attr->name);
    if (is_idx_pointer)
        fprintf(hf, "\t%s ai;\n", attr_index_type);
    fprintf(hf, "\tBDMF_TRACE_DBG(\"inside %s_%s_delete\\n\");\n\n",
            parm->drv->name, attr->name);
    if (is_idx_pointer)
    {
        fprintf(hf, "\n\tif (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(%s)))\n"
                "\t{\n"
                "\t\tBDMF_TRACE_ERR(\"failed to copy from user\\n\");\n"
                "\t\treturn -1;\n"
                "\t}\n",
                attr_index_type);
    }

    if (is_idx_pointer)
        fprintf(hf, "\tif ((pa->ret = %s(pa->mo, &ai)))\n", func_name);
    else
        fprintf(hf, "\tif ((pa->ret = %s(pa->mo, (%s)pa->ai)))\n", func_name, attr_index_type);

    fprintf(hf,"\t{\n"
            "\t\tBDMF_TRACE_ERR(\"%s failed\\n\");\n"
            "\t}\n\n"
            "\treturn 0;\n"
            "}\n\n",
            func_name);
}

static inline void remove_prefix(char *str, const char *prefix)
{
    int str_len;
    int prefix_len = strlen(prefix);

    if (!strncmp(str, prefix, prefix_len))
    {
        str_len = strlen(str) + 1;
        memmove(str, str + prefix_len, str_len - prefix_len);
    }
}

static int user_ioctl_attr_impl_gen(codegen_parm_t *parm, const struct bdmf_attr *attr, bdmf_func_type func_type)
{
    const char *attr_val_type;
    const char *attr_index_type;
    FILE *hf = parm->hf;
    char func_name[MAX_STR + 1] = {0};
    int is_read = (func_type==func_read || func_type==func_find);
    char func_suffix_comment[CODEGEN_FUNC_SUFFIX_SIZE], func_suffix[CODEGEN_FUNC_SUFFIX_SIZE];
    char attr_val_type_no_pointer[MAX_STR + 1] = {0};
    char attr_index_type_no_pointer[MAX_STR + 1] = {0};
    int attr_name_len, idx_name_len, is_pointer;
    int is_idx_pointer = 0;

    attr_val_type = _bdmf_mon_attr_val_type(parm->session, attr, is_read);
    if (!attr_val_type)
    {
        bdmf_session_print(parm->session, "CODEGEN: can't generate attribute access function for %s/%s\n", parm->drv->name, attr->name);
        return 0;
    }
    attr_index_type = _bdmf_mon_attr_index_type(parm->session, attr, func_type);
    if (!attr_index_type)
    {
        bdmf_session_print(parm->session, "CODEGEN: can't generate attribute access function for %s/%s\n", parm->drv->name, attr->name);
        return 0;
    }

    idx_name_len = strlen(attr_index_type);
    if (idx_name_len >= MAX_STR)
    {
        bdmf_session_print(parm->session, "CODEGEN: can't generate attribute access function for %s/%s due to attr idx length %d\n",
            parm->drv->name, attr->name, idx_name_len);
        return 0;
    }
    strncpy(attr_index_type_no_pointer, attr_index_type, MAX_STR);

    if(attr_index_type_no_pointer[idx_name_len - 1] == '*')
    {
        attr_index_type_no_pointer[idx_name_len - 1] = '\0';
        is_idx_pointer = 1;
    }
    remove_prefix(attr_index_type_no_pointer, "const ");

    attr_name_len = strlen(attr_val_type);
    if (attr_name_len >= MAX_STR)
    {
        bdmf_session_print(parm->session, "CODEGEN: can't generate attribute access function for %s/%s due to attr name length %d\n",
            parm->drv->name, attr->name, attr_name_len);
        return 0;
    }
    strncpy(attr_val_type_no_pointer, attr_val_type, MAX_STR);

    is_pointer = (func_type==func_read || func_type==func_find || attr_val_type[attr_name_len - 1] == '*');
    if(attr_val_type_no_pointer[attr_name_len - 1] == '*')
        attr_val_type_no_pointer[attr_name_len - 1] = '\0';
    remove_prefix(attr_val_type_no_pointer, "const ");

    _bdmf_mon_codegen_attr_suffix(attr, func_type, func_suffix_comment, func_suffix);

    if (func_type == func_delete)
    {
        generate_delete_func_in_ioctl(parm, attr, attr_index_type_no_pointer, is_idx_pointer);
        return 0;
    }

    snprintf(func_name, sizeof(func_name), "%s_%s_%s%s",
        parm->name_prefix, parm->drv->name, attr->name, func_suffix);

    if (func_type==func_read)
    {
        fprintf(hf, "static int %s_user_%s_%s_get(rdpa_ioctl_cmd_t *pa)\n"
                "{\n",
                parm->name_prefix,parm->drv->name,attr->name);
        if (attr->array_size && is_idx_pointer)
            fprintf(hf, "\t%s ai;\n", attr_index_type_no_pointer);
        if(attr->type==bdmf_attr_string)
            fprintf(hf, "\tchar parm[%d] = {0};\n", attr->size);
        else
            fprintf(hf, "\t%s parm;\n", attr_val_type_no_pointer);
        fprintf(hf, "\n\tBDMF_TRACE_DBG(\"inside %s_user_%s_get\\n\");\n",
                parm->drv->name, attr->name);
    }
    else if (func_type==func_write)
    {
        fprintf(hf, "static int %s_user_%s_%s_set(rdpa_ioctl_cmd_t *pa)\n"
                "{\n",
                parm->name_prefix,parm->drv->name,attr->name);
        if (attr->array_size && is_idx_pointer)
            fprintf(hf, "\t%s ai;\n", attr_index_type_no_pointer);
        if (is_pointer)
        {
            if(attr->type==bdmf_attr_string)
                fprintf(hf, "\tchar parm[%d] = {0};\n", attr->size);
            else
                fprintf(hf, "\t%s parm;\n", attr_val_type_no_pointer);
        }
        fprintf(hf, "\n\tBDMF_TRACE_DBG(\"inside %s_user_%s_set\\n\");\n",
                parm->drv->name, attr->name);
    }
    else if (func_type==func_add)
    {
        fprintf(hf, "static int %s_user_%s_%s_add(rdpa_ioctl_cmd_t *pa)\n",
                parm->name_prefix,parm->drv->name,attr->name);
        fprintf(hf, "{\n");
        if (is_idx_pointer)
            fprintf(hf, "\t%s ai;\n", attr_index_type_no_pointer);
        if (is_pointer)
            fprintf(hf, "\t%s parm;\n", attr_val_type_no_pointer);

        fprintf(hf, "\n\tBDMF_TRACE_DBG(\"inside %s_user_%s_add\\n\");\n",
            parm->drv->name, attr->name);
    }
    else if (func_type==func_get_next)
    {
        fprintf(hf, "static int %s_user_%s_%s_get_next(rdpa_ioctl_cmd_t *pa)\n",
                parm->name_prefix,parm->drv->name,attr->name);
        fprintf(hf, "{\n"
                "\t%s ai;\n"
                "\n\tBDMF_TRACE_DBG(\"inside %s_user_%s_get_next\\n\");\n",
                attr_index_type_no_pointer, parm->drv->name, attr->name);
    }
    else if (func_type==func_find)
    {
        fprintf(hf, "static int %s_user_%s_%s_find(rdpa_ioctl_cmd_t *pa)\n",
                parm->name_prefix,parm->drv->name,attr->name);
        fprintf(hf, "{\n"
                "\t%s ai;\n"
                "\t%s parm;\n"
                "\n\tBDMF_TRACE_DBG(\"inside %s_user_%s_find\\n\");\n",
                attr_index_type_no_pointer, attr_val_type_no_pointer, parm->drv->name, attr->name);
    }

    if((func_type==func_add || func_type==func_find || func_type==func_get_next || attr->array_size) && is_idx_pointer)
    {
        fprintf(hf, "\n\tif (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(%s)))\n"
                "\t{\n"
                "\t\tBDMF_TRACE_ERR(\"failed to copy from user\\n\");\n"
                "\t\treturn -1;\n"
                "\t}\n",
                attr_index_type_no_pointer);
    }
    if(attr->type==bdmf_attr_string)
    {
        fprintf(hf, "\n\tif (copy_from_user((void *)parm, (void *)(long)pa->ptr, %d))\n"
                "\t{\n"
                "\t\tBDMF_TRACE_ERR(\"failed to copy from user\\n\");\n"
                "\t\treturn -1;\n"
                "\t}\n",
                attr->size - 1);
    }
    else if((func_type==func_write || func_type==func_find || func_type==func_add) && is_pointer)
    {
        fprintf(hf, "\n\tif (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(%s)))\n"
                "\t{\n"
                "\t\tBDMF_TRACE_ERR(\"failed to copy from user\\n\");\n"
                "\t\treturn -1;\n"
                "\t}\n",
                attr_val_type_no_pointer);
    }

    generate_func_call_in_ioctl(parm, attr, func_type, func_name, attr_index_type_no_pointer, is_pointer, attr_val_type_no_pointer, is_idx_pointer);
    return 0;
}


static void user_drv_get_ioctl_gen(codegen_parm_t *parm)
{
    FILE *hf = parm->hf;

    fprintf(hf, "static int %s_user_%s_drv(rdpa_ioctl_cmd_t *pa)\n",
            parm->name_prefix,parm->drv->name);
    fprintf(hf,"{\n"
            "\tBDMF_TRACE_DBG(\"inside %s_user_%s_drv\\n\");\n\n",
            parm->name_prefix, parm->drv->name);
    fprintf(hf, "\tif (!(pa->drv = %s_%s_drv()))\n"
            "\t{\n"
            "\t\tBDMF_TRACE_DBG(\"%s_%s_drv failed\\n\");\n",
            parm->name_prefix, parm->drv->name, parm->name_prefix, parm->drv->name);
    fprintf(hf, "\t}\n\n"
            "\treturn 0;\n"
            "}\n\n");
}

static void user_obj_get_ioctl_gen(codegen_parm_t *parm)
{
    FILE *hf = parm->hf;
    int is_string = !strcmp(parm->keytype, "const char *");
    void *ptr;
    char key_not_const[256] = {0};

    fprintf(hf, "static int %s_user_%s_get(rdpa_ioctl_cmd_t *pa)\n"
            "{\n",
            parm->name_prefix,parm->drv->name);
    if (is_string)
        fprintf(hf, "\tchar parm[%d] = {0};\n\n", BDMF_OBJ_NAME_LEN);
    else if ((ptr = strstr(parm->keytype, "_key_t *")) != NULL)
    {
        ((char *)ptr)[7] = '\0';
        strcpy(key_not_const, parm->keytype);
        remove_prefix(key_not_const, "const ");
        fprintf(hf, "\t%s parm;\n\n", key_not_const);
    }
    fprintf(hf, "\tBDMF_TRACE_DBG(\"inside %s_user_%s_drv\\n\");\n\n",
            parm->name_prefix, parm->drv->name);
    if (is_string)
    {
        fprintf(hf, "\tif (copy_from_user((void *)parm, (void *)(long)pa->ptr, %d))\n"
                "\t{\n"
                "\t\tBDMF_TRACE_ERR(\"failed to copy from user\\n\");\n"
                "\t\treturn -1;\n"
                "\t}\n",
                BDMF_OBJ_NAME_LEN - 1);
    }
    else if (ptr)
    {
        fprintf(hf, "\tif (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(%s)))\n"
                "\t{\n"
                "\t\tBDMF_TRACE_ERR(\"failed to copy from user\\n\");\n"
                "\t\treturn -1;\n"
                "\t}\n",
                key_not_const);
    }


    fprintf(hf, "\tif ((pa->ret = %s_%s_get(",
            parm->name_prefix, parm->drv->name);

    if (is_string)
        fprintf(hf, "parm, ");
    else if (ptr)
        fprintf(hf, "&parm, ");
    else if(*parm->keyname != '\0')
        fprintf(hf, "(%s)(long)pa->parm, ", parm->keytype);
    fprintf(hf, "&pa->mo)))\n"
            "\t{\n"
            "\t\tBDMF_TRACE_DBG(\"%s_%s_get failed ret: %%d\\n\", pa->ret);\n",
            parm->name_prefix, parm->drv->name);
    fprintf(hf, "\t}\n\n"
            "\treturn 0;\n"
            "}\n\n");
}

static void user_ioctl_drv_and_obj_case_gen(codegen_parm_t *parm)
{
    FILE *hf = parm->hf;
    char str[MAX_STR] = "";

    snprintf(str, sizeof(str), "%s_%s_drv", parm->name_prefix, parm->drv->name);
    to_upper_case(str);
    fprintf(hf, "\t\tcase %s:\n"
            "\t\t\tret = %s_user_%s_drv(pa);\n"
            "\t\t\tbreak;\n\n",
            str, parm->name_prefix, parm->drv->name);

    snprintf(str, sizeof(str), "%s_%s_get", parm->name_prefix, parm->drv->name);
    to_upper_case(str);
    fprintf(hf, "\t\tcase %s:\n"
            "\t\t\tret = %s_user_%s_get(pa);\n"
            "\t\t\tbreak;\n\n",
            str, parm->name_prefix, parm->drv->name);
}

static int is_attr_contain_buffer_or_pointer(const struct bdmf_attr *attr)
{
    const struct bdmf_attr *fields = attr->aggr_type->fields;

    while (fields->name)
    {
        if (fields->type == bdmf_attr_buffer || fields->type == bdmf_attr_pointer)
            return 1;
        else if (fields->type == bdmf_attr_aggregate)
        {
             if (is_attr_contain_buffer_or_pointer(fields))
                 return 1;
        }
        ++fields;
    }

    return 0;
}

static void user_ioctl_attr_case_gen(codegen_parm_t *parm)
{
    const struct bdmf_attr *attr = parm->drv->aattr;
    FILE *hf = parm->hf;
    char str[MAX_STR] = "";

    while(attr && attr->name)
    {
        if ((attr->flags & parm->attr_level)!=0 &&
            !(attr->flags & BDMF_ATTR_NO_AUTO_GEN) &&
            ((parm->action_mask & BDMF_ACTION_MASK_ATTR_ALL)!=0 ||
             (attr->type==bdmf_attr_aggregate)) )
        {
            /* aggregate type containig buffer can't be generated in userspace */
            if ((attr->type==bdmf_attr_aggregate && is_attr_contain_buffer_or_pointer(attr)) || 
                (attr->type==bdmf_attr_pointer))
            {
                ++attr;
                continue;
            }


            if ((attr->flags & BDMF_ATTR_READ))
            {
                snprintf(str, sizeof(str), "%s_%s_%s_get", parm->name_prefix, parm->drv->name, attr->name);
                to_upper_case(str);
                fprintf(hf, "\t\tcase %s:\n"
                        "\t\t\tret = %s_user_%s_%s_get(pa);\n"
                        "\t\t\tbreak;\n\n",
                        str, parm->name_prefix, parm->drv->name, attr->name);
            }
            if ((attr->flags & BDMF_ATTR_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
            {
                if ((attr->flags & BDMF_ATTR_UDEF_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
                {
                    snprintf(str, sizeof(str), "%s_%s_%s_set", parm->name_prefix, parm->drv->name, attr->name);
                    to_upper_case(str);
                    fprintf(hf, "\t\tcase %s:\n"
                            "\t\t\tret = %s_user_%s_%s_set(pa);\n"
                            "\t\t\tbreak;\n\n",
                            str, parm->name_prefix, parm->drv->name, attr->name);
                }
                if (attr->add)
                {
                    snprintf(str, sizeof(str), "%s_%s_%s_add", parm->name_prefix, parm->drv->name, attr->name);
                    to_upper_case(str);
                    fprintf(hf, "\t\tcase %s:\n"
                            "\t\t\tret = %s_user_%s_%s_add(pa);\n"
                            "\t\t\tbreak;\n\n",
                            str, parm->name_prefix, parm->drv->name, attr->name);
                }
                if (attr->del)
                {
                    snprintf(str, sizeof(str), "%s_%s_%s_delete", parm->name_prefix, parm->drv->name, attr->name);
                    to_upper_case(str);
                    fprintf(hf, "\t\tcase %s:\n"
                            "\t\t\tret = %s_user_%s_%s_delete(pa);\n"
                            "\t\t\tbreak;\n\n",
                            str, parm->name_prefix, parm->drv->name, attr->name);
                }
            }
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_GET_NEXT))
            {
                snprintf(str, sizeof(str), "%s_%s_%s_get_next", parm->name_prefix, parm->drv->name, attr->name);
                to_upper_case(str);
                fprintf(hf, "\t\tcase %s:\n"
                        "\t\t\tret = %s_user_%s_%s_get_next(pa);\n"
                        "\t\t\tbreak;\n\n",
                        str, parm->name_prefix, parm->drv->name, attr->name);
            }
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_FIND))
            {
                snprintf(str, sizeof(str), "%s_%s_%s_find", parm->name_prefix, parm->drv->name, attr->name);
                to_upper_case(str);
                fprintf(hf, "\t\tcase %s:\n"
                        "\t\t\tret = %s_user_%s_%s_find(pa);\n"
                        "\t\t\tbreak;\n\n",
                        str, parm->name_prefix, parm->drv->name, attr->name);
            }
        }
        ++attr;
    }
}

static int bdmf_mon_codegen_standard_gpl_comments(codegen_parm_t *parm)
{
    static char *gpl_string =
        "// %scopyright-BRCM:2013:DUAL/GPL:standard\n"
        "// :>\n";
    int rc;

    rc = fprintf(parm->hf, gpl_string, "<:");
    return (rc > 0) ? 0 : BDMF_ERR_IO;
}

static int user_ioctl_impl_gen(codegen_parm_t *parm)
{
    FILE *hf = parm->hf;
    const struct bdmf_attr *attr=parm->drv->aattr;
    int rc=0;

    /* do nothing for root */
    if (!parm->drv->po)
        return 0;

    rc = bdmf_mon_codegen_standard_gpl_comments(parm);
    if (rc)
        return rc;

    
    fprintf(hf, "/*\n"
            " * %s object ioctl functions implementation file.\n"
            " * This ioctl file is generated automatically. Do not edit!\n"
            " */\n", parm->drv->name);
    fprintf(hf, "#include \"rdpa_api.h\"\n"
            "#include \"rdpa_user.h\"\n"
            "#include \"rdpa_user_int.h\"\n"
            "#include \"rdpa_%s_user_ioctl_ag.h\"\n\n",
            parm->drv->name);

    user_drv_get_ioctl_gen(parm);
    user_obj_get_ioctl_gen(parm);

    while(attr && attr->name)
    {
        if ((attr->flags & parm->attr_level)!=0 &&
            !(attr->flags & BDMF_ATTR_NO_AUTO_GEN) &&
            ((parm->action_mask & BDMF_ACTION_MASK_ATTR_ALL)!=0 ||
             (attr->type==bdmf_attr_aggregate)) )
        {
            /* aggregate type containig buffer can't be generated in userspace */
            if ((attr->type==bdmf_attr_aggregate && is_attr_contain_buffer_or_pointer(attr)) || 
                (attr->type==bdmf_attr_pointer))
            {
                ++attr;
                continue;
            }
            if ((attr->flags & BDMF_ATTR_READ))
                rc = rc ? rc : user_ioctl_attr_impl_gen(parm, attr, func_read);
            if ((attr->flags & BDMF_ATTR_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
            {
                if ((attr->flags & BDMF_ATTR_UDEF_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
                    rc = rc ? rc : user_ioctl_attr_impl_gen(parm, attr, func_write);
                if (attr->add)
                    rc = rc ? rc : user_ioctl_attr_impl_gen(parm, attr, func_add);
                if (attr->del)
                    rc = rc ? rc : user_ioctl_attr_impl_gen(parm, attr, func_delete);
            }
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_GET_NEXT))
                rc = rc ? rc : user_ioctl_attr_impl_gen(parm, attr, func_get_next);
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_FIND))
                rc = rc ? rc : user_ioctl_attr_impl_gen(parm, attr, func_find);
        }
        ++attr;
    }

    fprintf(hf, "long rdpa_%s_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)\n"
            "{\n"
            "\tint ret;\n\n"
            "\tswitch (op){\n",
            parm->drv->name);

    user_ioctl_drv_and_obj_case_gen(parm);
    user_ioctl_attr_case_gen(parm);

    fprintf(hf, "\t\tdefault:\n"
            "\t\t\tBDMF_TRACE_ERR(\"no such ioctl cmd: %%u\\n\", op);\n"
            "\t\t\tret = EINVAL;\n"
            "\t\t}\n\n"
            "\treturn ret;\n"
            "}\n");

    return rc;
}

static int key_gen(codegen_parm_t *parm, int *nkeys)
{
    const struct bdmf_type *d = parm->drv;
    const struct bdmf_type *drv_stack[BDMF_MAX_TYPE_STACK_DEPTH];
    FILE *hf = parm->hf;
    const struct bdmf_attr *a;
    int stack_depth=0;

    /* Calculate driver stack */
    while(d)
    {
        if (stack_depth >= BDMF_MAX_TYPE_STACK_DEPTH)
        {
            bdmf_session_print(parm->session, "CODEGEN: parent type stack overflow for %s\n", parm->drv->name);
            return 0;
        }
        drv_stack[stack_depth++] = d;

        /* calculate total number of keys */
        a = d->aattr;
        while(a && a->name)
        {
            if ((a->flags & BDMF_ATTR_KEY))
                ++*nkeys;
            ++a;
        }

        d = d->po;
    }

    /* Generate key structure */
    if (*nkeys)
    {
        int i=stack_depth;
        if (*nkeys > 1)
        {
            snprintf(parm->keytype, sizeof(parm->keytype), "const %s_%s_key_t *", parm->name_prefix, parm->drv->name);
            snprintf(parm->keyname, sizeof(parm->keyname), "key_");
            if (!parm->gpl_shim)
            {
                fprintf(hf, "\n/** %s object key. */\n", parm->drv->name);
                fprintf(hf, "typedef struct {\n");
                while(--i >= 0)
                {
                    d = drv_stack[i];
                    /* generate structure field for each key field */
                    a = d->aattr;
                    while(a && a->name)
                    {
                        if ((a->flags & BDMF_ATTR_KEY))
                        {
                            fprintf(hf, "    %s %s; /**< %s: %s */\n",
                                    _bdmf_mon_attr_val_type(parm->session, a, 0), a->name,
                                    d->name, a->help?a->help:"");
                        }
                        ++a;
                    }
                };
                fprintf(hf, "} %s_%s_key_t;\n\n", parm->name_prefix, parm->drv->name);
            }
        }
        else
        {
            while(--i >= 0)
            {
                d = drv_stack[i];
                /* generate structure field for each key field */
                a = d->aattr;
                while(a && a->name)
                {
                    if ((a->flags & BDMF_ATTR_KEY))
                    {
                        snprintf(parm->keytype, sizeof(parm->keytype), "%s",
                                _bdmf_mon_attr_val_type(parm->session, a, 0));
                        snprintf(parm->keyname, sizeof(parm->keyname), "%s_", a->name);
                        break;
                    }
                    ++a;
                }
            };
        }
    }

    return 0;
}



/* Generate "object_key structure */
static int _bdmf_mon_codegen_gen_key(codegen_parm_t *parm)
{
    const struct bdmf_type *d = parm->drv;
    const struct bdmf_type *drv_stack[BDMF_MAX_TYPE_STACK_DEPTH];
    FILE *hf = parm->hf;
    const struct bdmf_attr *a;
    int stack_depth=0;
    int nkeys=0;

    /* Calculate driver stack */
    while(d)
    {
        if (stack_depth >= BDMF_MAX_TYPE_STACK_DEPTH)
        {
            bdmf_session_print(parm->session, "CODEGEN: parent type stack overflow for %s\n", parm->drv->name);
            return 0;
        }
        drv_stack[stack_depth++] = d;

        /* calculate total number of keys */
        a = d->aattr;
        while(a && a->name)
        {
            if ((a->flags & BDMF_ATTR_KEY))
                ++nkeys;
            ++a;
        }

        d = d->po;
    }

    /* Generate key structure */
    if (nkeys)
    {
        int i=stack_depth;
        if (nkeys > 1)
        {
            snprintf(parm->keytype, sizeof(parm->keytype), "const %s_%s_key_t *", parm->name_prefix, parm->drv->name);
            snprintf(parm->keyname, sizeof(parm->keyname), "key_");
            if (!parm->gpl_shim)
            {
                fprintf(hf, "\n/** %s object key. */\n", parm->drv->name);
                fprintf(hf, "typedef struct {\n");
                while(--i >= 0)
                {
                    d = drv_stack[i];
                    /* generate structure field for each key field */
                    a = d->aattr;
                    while(a && a->name)
                    {
                        if ((a->flags & BDMF_ATTR_KEY))
                        {
                            fprintf(hf, "    %s %s; /**< %s: %s */\n",
                                    _bdmf_mon_attr_val_type(parm->session, a, 0), a->name,
                                    d->name, a->help?a->help:"");
                        }
                        ++a;
                    }
                };
                fprintf(hf, "} %s_%s_key_t;\n\n", parm->name_prefix, parm->drv->name);
            }
        }
        else
        {
            while(--i >= 0)
            {
                d = drv_stack[i];
                /* generate structure field for each key field */
                a = d->aattr;
                while(a && a->name)
                {
                    if ((a->flags & BDMF_ATTR_KEY))
                    {
                        snprintf(parm->keytype, sizeof(parm->keytype), "%s",
                                _bdmf_mon_attr_val_type(parm->session, a, 0));
                        snprintf(parm->keyname, sizeof(parm->keyname), "%s_", a->name);
                        break;
                    }
                    ++a;
                }
            };
        }
    }

    /* Generate "get object" function */
    fprintf(hf, "\n");
    if (nkeys)
    {
        fprintf(hf, "%sint (*f_%s_%s_get)(%s %s, bdmf_object_handle *pmo);\n",
            parm->gpl_shim ? "" : "extern ",
            parm->name_prefix, parm->drv->name, parm->keytype, parm->keyname);
    }
    else
    {
        fprintf(hf, "%sint (*f_%s_%s_get)(bdmf_object_handle *pmo);\n",
            parm->gpl_shim ? "" : "extern ",
            parm->name_prefix, parm->drv->name);
    }
    if (parm->gpl_shim)
        fprintf(hf, "EXPORT_SYMBOL(f_%s_%s_get);\n", parm->name_prefix, parm->drv->name);

    fprintf(hf, "\n");
    fprintf(hf, "/** Get %s object%s", parm->drv->name, nkeys ? " by key." : ".");
    fprintf(hf, "\n\n");
    fprintf(hf, " * This function returns %s object instance%s",
        parm->drv->name, nkeys ? " by key." : ".");
    fprintf(hf, "\n");
    if (nkeys)
        fprintf(hf, " * \\param[in] %s    Object key\n", parm->keyname);
    fprintf(hf, " * \\param[out] %s_obj    Object handle\n", parm->drv->name);
    fprintf(hf, " * \\return    0=OK or error <0\n");
    fprintf(hf, " */\n");
    fprintf(hf, "int %s_%s_get(", parm->name_prefix, parm->drv->name);
    if (nkeys)
        fprintf(hf, "%s %s, ", parm->keytype, parm->keyname);
    fprintf(hf, "bdmf_object_handle *%s_obj)", parm->drv->name);
    if (parm->gpl_shim)
    {
        fprintf(hf, "\n{\n"
                    "   if (!f_%s_%s_get)\n"
                    "       return BDMF_ERR_STATE;\n"
                    "   return f_%s_%s_get(",
                    parm->name_prefix, parm->drv->name, parm->name_prefix, parm->drv->name);
        if (nkeys)
            fprintf(hf, "%s, ", parm->keyname);
        fprintf(hf, "%s_obj);\n", parm->drv->name);
        fprintf(hf, "}\n");
        fprintf(hf, "EXPORT_SYMBOL(%s_%s_get);\n", parm->name_prefix, parm->drv->name);
    }
    else
        fprintf(hf, ";\n");

    return 0;
}

/* Get "a" / "an" article */
static const char *_bdmf_codegen_get_article(const char *name)
{
    static char *a_article = "a";
    static char *an_article = "an";

    if (strchr("aeuio", name[0]))
        return an_article;
    return a_article;
}

/* head comments for xx_drv() function */
static void _bdmf_mon_xx_drv_header(codegen_parm_t *parm)
{
    FILE *hf = parm->hf;
    const struct bdmf_type *drv = parm->drv;
    const char *article = _bdmf_codegen_get_article(drv->name);

    fprintf(hf, "\n"
                "/** Get %s type handle.\n"
                " *\n"
                " * This handle should be passed to bdmf_new_and_set() function in\n"
                " * order to create %s %s object.\n"
                " * \\return %s type handle\n"
                " */\n", drv->name, article, drv->name, drv->name);
}


/* Generate object-access header file for a single object type
 */
static int _bdmf_mon_codegen_gen1_header(codegen_parm_t *parm)
{
    const struct bdmf_attr *attr=parm->drv->aattr;
    FILE *hf = parm->hf;
    const struct bdmf_type *drv = parm->drv;
    char attrhlp[128];
    int rc = 0;

    /* do nothing for root */
    if (!parm->drv->po)
        return 0;

    /* Doxygen description */
    fprintf(hf, "/** \\addtogroup %s\n", drv->name);
    fprintf(hf, " * @{\n */\n\n");

    /* Generate "xx_drv_get" function */
    _bdmf_mon_xx_drv_header(parm);
    fprintf(hf, "bdmf_type_handle %s_%s_drv(void);\n", parm->name_prefix, drv->name);

    /* Generate attribute enums */
    if ((parm->action_mask & (BDMF_ACTION_MASK_ATTR_ENUM | BDMF_ACTION_MASK_ATTR_ACCESS)) && attr)
    {
        char *doxygen_hdr_comment = (parm->action_mask & BDMF_ACTION_MASK_ATTR_ENUM) ? "*" : "";
        char *doxygen_body_comment = (parm->action_mask & BDMF_ACTION_MASK_ATTR_ENUM) ? "*<" : "";

        fprintf(hf, "\n/*%s %s: Attribute types */\n", doxygen_hdr_comment, drv->name);
        fprintf(hf, "typedef enum {\n");
        while(attr && attr->name)
        {
            if ((attr->flags & parm->attr_level) && !(attr->flags & BDMF_ATTR_NO_AUTO_GEN))
            {
                bdmf_attr_help_compact(attr, attrhlp, sizeof(attrhlp));
                fprintf(hf, "    %s_%s_attr_%s = %d, /*%s %s */\n",
                    parm->name_prefix, drv->name,
                    attr->name, (int)(attr-drv->aattr),
                    doxygen_body_comment, attrhlp);
            }
            ++attr;
        }
        fprintf(hf, "} %s_%s_attr_types;\n", parm->name_prefix, drv->name);
    }

    /* Generate key and attribute access by key */
    if ((parm->action_mask & BDMF_ACTION_MASK_KEY))
        rc = _bdmf_mon_codegen_gen_key(parm);

    /* Generate specific attribute access functions */
    if ((parm->action_mask & BDMF_ACTION_MASK_ATTR_ACCESS))
        rc = rc ? rc : _bdmf_mon_codegen_gen_attr_access(parm);

    /* Generate generic attribute access functions */
    if ((parm->action_mask & BDMF_ACTION_MASK_ATTR_GEN_ACCESS))
        rc = rc ? rc : _bdmf_mon_codegen_gen_attr_gen_access(parm);

    fprintf(hf, "/** @} end of %s Doxygen group */\n\n\n", drv->name);

    return rc;
}



static void gen_delete_attr_user(codegen_parm_t *parm,
                                const char *attr_index_type,
                                const struct bdmf_attr *attr,
                                struct func_desc *fdesc,
                                int is_idx_pointer)
{
    FILE *hf = parm->hf;
    char func_name[MAX_STR];

    fprintf(hf, "\n");
    snprintf(func_name, sizeof(func_name), "%s_%s_%s_delete",
            parm->name_prefix, parm->drv->name, attr->name);
    fdesc = _bdmf_mon_codegen_get_fdesc(func_name);
    if (fdesc && fdesc->is_tagged)
    {
        /* Replace description */
        fprintf(hf, "%s", fdesc->desc);
    }
    else
    {
        fprintf(hf, "/** Delete %s/%s attribute entry.\n",
                parm->drv->name, attr->name);
        fprintf(hf, " *\n");
        fprintf(hf, " * Delete %s.\n", attr->help);
        if (fdesc)
            fprintf(hf, "%s", fdesc->desc);
        if ((attr->flags & BDMF_ATTR_DEPRECATED))
        {
            if (attr->deprecated_text)
                fprintf(hf, "\n * \\deprecated This function has been deprecated, %s.\n *\n",attr->deprecated_text);
            else
                fprintf(hf, "\n * \\deprecated This function has been deprecated.\n *\n");
        }
        fprintf(hf, " * \\param[in]   mo_ %s object handle\n", parm->drv->name);
        fprintf(hf, " * \\param[in]   ai_ Attribute array index\n");
        fprintf(hf, " * \\return 0 or error code < 0\n");
        if ((attr->flags & BDMF_ATTR_NOLOCK))
            fprintf(hf, " * The function can be called in task and softirq contexts.\n");
        else
            fprintf(hf, " * The function can be called in task context only.\n");
        fprintf(hf, " */\n");
    }

    fprintf(hf, "static inline int %s(", func_name);
    fprintf(hf, "bdmf_object_handle mo_, %s ai_)\n", attr_index_type);
    to_upper_case(func_name);
    fprintf(hf, "{\n"
            "\trdpa_ioctl_cmd_t pa = {0};\n"
            "\tint fd, ret;\n\n"
            "\tpa.mo = mo_;\n"
            "\tpa.cmd = %s;\n",
            func_name);
    if(is_idx_pointer)
        fprintf(hf, "\tpa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;\n");
    else
        fprintf(hf, "\tpa.ai = (bdmf_index)(long)ai_;\n");
}

static void gen_all_attr_user(codegen_parm_t *parm,
                              const struct bdmf_attr *attr,
                              char *func_name,
                              bdmf_func_type func_type,
                              const char *attr_val_type,
                              const char *attr_index_type,
                              int is_read,
                              int no_value,
                              int is_idx_pointer)
{
    FILE *hf = parm->hf;

    fprintf(hf, "static inline int %s(", func_name);
    fprintf(hf, "bdmf_object_handle mo_");
    if (attr->array_size || (func_type==func_add))
        fprintf(hf, ", %s ai_", attr_index_type);
    if ((func_type != func_get_next) && !(func_type == func_write && no_value))
    {
        fprintf(hf, ", %s %s%s_", attr_val_type,
                (is_read && _bdmf_mon_codegen_is_numeric(attr)) ? "*" :"", attr->name);
    }
    if ((func_type != func_get_next && func_type != func_find) &&
            (attr->type==bdmf_attr_buffer || (func_type==func_read && attr->type==bdmf_attr_string)))
        fprintf(hf, ", uint32_t size_)\n");
    else
        fprintf(hf, ")\n");
    fprintf(hf, "{\n");

    fprintf(hf, "\trdpa_ioctl_cmd_t pa = {0};\n"
            "\tint fd, ret;\n\n"
            "\tpa.mo = mo_;\n");
    if (func_type==func_read)
    {
        if ((attr->array_size > 1))
        {
            if(is_idx_pointer)
                fprintf(hf, "\tpa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;\n");
            else
                fprintf(hf, "\tpa.ai = (bdmf_index)(long)ai_;\n");
        }
        if (attr->type==bdmf_attr_string)
            fprintf(hf, "\tpa.size = size_;\n");
        fprintf(hf, "\tpa.ptr = (bdmf_ptr)(unsigned long)%s_;\n", attr->name);
    }
    else if (func_type==func_write)
    {
        if (attr->array_size > 1)
        {
            if(is_idx_pointer)
                fprintf(hf, "\tpa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;\n");
            else
                fprintf(hf, "\tpa.ai = (bdmf_index)(long)ai_;\n");
        }
        if (attr->type == bdmf_attr_string)
        {
            fprintf(hf, "\tpa.ptr = (bdmf_ptr)(unsigned long)%s_;\n", attr->name);
        }
        else if (attr->type == bdmf_attr_object)
            fprintf(hf, "\tpa.object = %s_;\n", attr->name);
        else if (_bdmf_mon_codegen_is_numeric(attr))
        {
            if (no_value)
                fprintf(hf, "\tpa.parm = 1;\n");
            else
                fprintf(hf, "\tpa.parm = (uint64_t)(long)%s_;\n", attr->name);
        }
        else
            fprintf(hf, "\tpa.ptr = (bdmf_ptr)(unsigned long)%s_;\n", attr->name);

        if (attr->type == bdmf_attr_buffer)
            fprintf(hf, "\tpa.size = size_;\n");
    }
    else
    {
        if(is_idx_pointer)
            fprintf(hf, "\tpa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;\n");
        else
            fprintf(hf, "\tpa.ai = (bdmf_index)(long)ai_;\n");
        if (func_type!=func_get_next)
        {
            if (func_type==func_add && (attr->type == bdmf_attr_object))
                fprintf(hf, "\tpa.object = %s_;\n", attr->name);
            else if(_bdmf_mon_codegen_is_numeric(attr) && func_type!=func_find)
                fprintf(hf, "\tpa.parm = (uint64_t)(long)%s_;\n", attr->name);
            else
                fprintf(hf, "\tpa.ptr = (bdmf_ptr)(unsigned long)%s_;\n", attr->name);
            if (func_type==func_add && attr->type == bdmf_attr_buffer)
                fprintf(hf, "\tpa.size = size_;\n");
        }
    }
    to_upper_case(func_name);
    fprintf(hf, "\tpa.cmd = %s;\n", func_name);
}

static void auto_generate_description(codegen_parm_t *parm,
                                        const struct bdmf_attr *attr,
                                        struct func_desc *fdesc,
                                        char *func_suffix_comment,
                                        char *func_suffix,
                                        bdmf_func_type func_type,
                                        int no_value)
{
    FILE *hf = parm->hf;

    if (fdesc && fdesc->is_tagged)
    {
        /* Replace description */
        fprintf(hf, "%s", fdesc->desc);
    }
    else
    {
        /* Auto-generate description */
        fprintf(hf, "/** %s %s/%s attribute%s\n",
                func_suffix_comment,
                parm->drv->name, attr->name, (attr->array_size > 1) ? " entry." : ".");
        fprintf(hf, " *\n");
        fprintf(hf, " * %s %s.\n", func_suffix_comment, attr->help);
        if (fdesc)
            fprintf(hf, "%s", fdesc->desc);
        if ((attr->flags & BDMF_ATTR_DEPRECATED))
        {
            if (attr->deprecated_text)
                fprintf(hf, "\n * \\deprecated This function has been deprecated, %s.\n *\n",attr->deprecated_text);
            else
                fprintf(hf, "\n * \\deprecated This function has been deprecated.\n *\n");
        }
        fprintf(hf, " * \\param[in]   mo_ %s object handle or mattr transaction handle\n",
                parm->drv->name);
        if ((attr->array_size > 1) || (func_type==func_add))
            fprintf(hf, " * \\param[%s]   ai_ Attribute array index\n",
                    (func_type==func_add || func_type==func_get_next || func_type==func_find)?"in,out":"in");
        if (func_type==func_read)
            fprintf(hf, " * \\param[out]  %s_ Attribute value\n", attr->name);
        else if (func_type == func_find)
            fprintf(hf, " * \\param[in,out]   %s_ Attribute value\n", attr->name);
        else if ((func_type != func_get_next) && !no_value)
            fprintf(hf, " * \\param[in]   %s_ Attribute value\n", attr->name);
        if ((func_type != func_get_next && func_type != func_find) &&
                (attr->type==bdmf_attr_buffer || (func_type==func_read && attr->type==bdmf_attr_string)))
        {
            fprintf(hf, " * \\param[in]   size_ buffer size\n");
            if (func_type==func_read)
                fprintf(hf, " * \\return number of bytes read >=0 or error code < 0\n");
            else
                fprintf(hf, " * \\return number of bytes written >=0 or error code < 0\n");
        }
        else
            fprintf(hf, " * \\return 0 or error code < 0\n");
        if ((attr->flags & BDMF_ATTR_NOLOCK))
            fprintf(hf, " * The function can be called in task and softirq contexts.\n");
        else
            fprintf(hf, " * The function can be called in task context only.\n");

        fprintf(hf, " */\n");
    }
}

static int user_func_api_attr_impl_gen(codegen_parm_t *parm, const struct bdmf_attr *attr, bdmf_func_type func_type, char *ioctl_str)
{
    FILE *hf = parm->hf;
    const char *attr_val_type;
    const char *attr_index_type;
    char func_name[64];
    int is_read = (func_type==func_read || func_type==func_find);
    struct func_desc *fdesc = NULL;
    char func_suffix_comment[CODEGEN_FUNC_SUFFIX_SIZE], func_suffix[CODEGEN_FUNC_SUFFIX_SIZE];
    int no_value = (func_type == func_write) && (attr->flags & BDMF_ATTR_NO_VALUE) != 0;
    int is_idx_pointer = 0;

    attr_val_type = _bdmf_mon_attr_val_type(parm->session, attr, is_read);
    if (!attr_val_type)
    {
        bdmf_session_print(parm->session, "CODEGEN: can't generate attribute access function for %s/%s\n", parm->drv->name, attr->name);
        return 0;
    }
    attr_index_type = _bdmf_mon_attr_index_type(parm->session, attr, func_type);
    if (!attr_index_type)
    {
        bdmf_session_print(parm->session, "CODEGEN: can't generate attribute access function for %s/%s\n", parm->drv->name, attr->name);
        return 0;
    }
    _bdmf_mon_codegen_attr_suffix(attr, func_type, func_suffix_comment, func_suffix);

    if(attr_index_type[strlen(attr_index_type) - 1] == '*')
        is_idx_pointer = 1;

    /* "delete" function */
    if (func_type == func_delete)
    {

        gen_delete_attr_user(parm, attr_index_type, attr, fdesc, is_idx_pointer);
    }
    else
    {
        fprintf(hf, "\n");
        snprintf(func_name, sizeof(func_name), "%s_%s_%s%s",
                parm->name_prefix, parm->drv->name, attr->name, func_suffix);
        fdesc = _bdmf_mon_codegen_get_fdesc(func_name);
        auto_generate_description(parm, attr, fdesc, func_suffix_comment, func_suffix, func_type, no_value);
        gen_all_attr_user(parm, attr, func_name, func_type, attr_val_type, attr_index_type, is_read, no_value, is_idx_pointer);
    }

    fprintf(hf, "\n\tfd = open(RDPA_USR_DEV_NAME, O_RDWR);\n"
            "\tif (fd < 0)\n"
            "\t{\n"
            "\t\trdpa_usr_error(\"%%s: %%s\\n\", RDPA_USR_DEV_NAME, strerror(errno));\n"
            "\t\treturn -EINVAL;\n"
            "\t}\n"
            "\tret = ioctl(fd, %s, &pa);\n"
            "\tif (ret)\n"
            "\t{\n"
            "\t\trdpa_usr_error(\"ioctl failed, ret=%%d\\n\", ret);\n"
            "\t\tclose(fd);\n"
            "\t\treturn ret;\n"
            "\t}\n\n"
            "\tclose(fd);\n"
            "\treturn pa.ret;\n"
            "}\n\n", ioctl_str);

    return 0;
}


static int user_obj_get_gen(codegen_parm_t *parm, char *ioctl_str)
{
    FILE *hf = parm->hf;
    char str[256] = "";
    int nkeys=0;
    int is_string = 0;

    snprintf(str, sizeof(str), "%s_%s_get",
                        parm->name_prefix, parm->drv->name);
    to_upper_case(str);
    key_gen(parm, &nkeys);
    is_string = !strcmp(parm->keytype, "const char *");

    fprintf(hf, "/** Get %s object%s", parm->drv->name, nkeys ? " by key." : ".");
    fprintf(hf, "\n\n");
    fprintf(hf, " * This function returns %s object instance%s",
        parm->drv->name, nkeys ? " by key." : ".");
    fprintf(hf, "\n");
    if (nkeys)
        fprintf(hf, " * \\param[in] %s    Object key\n", parm->keyname);
    fprintf(hf, " * \\param[out] %s_obj    Object handle\n", parm->drv->name);
    fprintf(hf, " * \\return    0=OK or error <0\n");
    fprintf(hf, " */\n");
    fprintf(hf, "static inline int %s_%s_get(", parm->name_prefix, parm->drv->name);
    if (nkeys)
        fprintf(hf, "%s %s, ", parm->keytype, parm->keyname);
    fprintf(hf, "bdmf_object_handle *%s_obj)\n", parm->drv->name);
    fprintf(hf, "{\n"
                "\trdpa_ioctl_cmd_t pa = {0};\n"
                "\tint fd, ret = 0;\n\n"
                "\tpa.cmd = %s;\n",
                str);
    if (nkeys > 1 || is_string)
        fprintf(hf, "\tpa.ptr = (bdmf_ptr)(unsigned long)%s;\n\n", parm->keyname);
    else if (nkeys == 1)
        fprintf(hf, "\tpa.parm = (uint64_t)(long)%s;\n\n", parm->keyname);
    fprintf(hf,  "\tfd = open(RDPA_USR_DEV_NAME, O_RDWR);\n");
    fprintf(hf, "\tif (fd < 0)\n"
                "\t{\n"
                "\t\trdpa_usr_error(\"%%s: %%s\\n\", RDPA_USR_DEV_NAME, strerror(errno));\n"
                "\t\treturn -EINVAL;\n"
                "\t}\n"
                "\tret = ioctl(fd, %s, &pa);\n", ioctl_str);
    fprintf(hf, "\tif (ret)\n"
                "\t{\n"
                "\t\trdpa_usr_error(\"ioctl failed, Errno[%%s] ret=%%d\\n\", strerror(errno), ret);\n"
                "\t\tclose(fd);\n"
                "\t\treturn ret;\n"
                "\t}\n\n"
                "\t*%s_obj = pa.mo;\n"
                "\tclose(fd);\n"
                "\treturn pa.ret;\n"
                "}\n", parm->drv->name);

    return 0;
}

static void user_drv_get_gen(codegen_parm_t *parm, char *ioctl_str)
{
    char str[256] = "";
    FILE *hf = parm->hf;

    _bdmf_mon_xx_drv_header(parm);
    snprintf(str, sizeof(str), "%s_%s_drv",
                        parm->name_prefix, parm->drv->name);
    to_upper_case(str);

    fprintf(hf, "static inline bdmf_type_handle %s_%s_drv(void)\n", parm->name_prefix, parm->drv->name);
    fprintf(hf, "{\n"
            "\trdpa_ioctl_cmd_t pa = {0};\n"
            "\tint fd, ret = 0;\n\n"
            "\tpa.cmd = %s;\n"
            "\tfd = open(RDPA_USR_DEV_NAME, O_RDWR);\n"
            "\tif (fd < 0)\n"
            "\t{\n"
            "\t\trdpa_usr_error(\"%%s: %%s\\n\", RDPA_USR_DEV_NAME, strerror(errno));\n"
            "\t\treturn 0;\n"
            "\t}\n\n", str);
    fprintf(hf, "\tret = ioctl(fd, %s, &pa);\n"
            "\tif (ret)\n"
            "\t{\n"
            "\t\trdpa_usr_error(\"ioctl failed, Errno[%%s] ret=%%d\\n\", strerror(errno), ret);\n"
            "\t\tclose(fd);\n"
            "\t\treturn 0;\n"
            "\t}\n\n", ioctl_str);
    fprintf(hf, "\tclose(fd);\n"
	        "\treturn pa.drv;\n"
            "}\n\n");

}

static int user_func_api_header_gen(codegen_parm_t *parm)
{
    FILE *hf = parm->hf;
    char hdr_def[MAX_STR] = "";
    char ioctl_str[MAX_STR] = "";
    const struct bdmf_attr *attr = parm->drv->aattr;
    const struct bdmf_type *drv = parm->drv;
    int rc = 0;

    /* do nothing for root */
    if (!parm->drv->po)
        return 0;

    /* generate file header */
    snprintf(hdr_def, sizeof(hdr_def), "_%s_AG_%s_USR_H_", parm->name_prefix,
            parm->drv->name);
    to_upper_case(hdr_def);

    fprintf(hf, "/*\n"
            " * %s object user header file.\n"
            " * This header file is generated automatically. Do not edit!\n"
            " */\n", parm->drv->name);
    fprintf(hf, "#ifndef %s\n"
            "#define %s\n\n", hdr_def, hdr_def);
    fprintf(hf, "#include <sys/ioctl.h>\n"
            "#include \"rdpa_user.h\"\n"
            "#include \"rdpa_user_types.h\"\n"
            "#include \"rdpa_%s_user_ioctl_ag.h\"\n\n",
            parm->drv->name);

    /* Doxygen description */
    fprintf(hf, "/** \\addtogroup %s\n", drv->name);
    fprintf(hf, " * @{\n */\n\n");

    snprintf(ioctl_str, sizeof(ioctl_str), "%s_%s_IOCTL", parm->name_prefix,
            parm->drv->name);
    to_upper_case(ioctl_str);

    user_drv_get_gen(parm, ioctl_str);
    if ((parm->action_mask & BDMF_ACTION_MASK_KEY))
        user_obj_get_gen(parm, ioctl_str);

    attr = parm->drv->aattr;
    while(attr && attr->name)
    {
        if ((attr->flags & parm->attr_level)!=0 &&
            !(attr->flags & BDMF_ATTR_NO_AUTO_GEN) &&
            ((parm->action_mask & BDMF_ACTION_MASK_ATTR_ALL)!=0 ||
             (attr->type==bdmf_attr_aggregate)) )
        {
            /* aggregate type containig buffer can't be generated in userspace */
            if ((attr->type==bdmf_attr_aggregate && is_attr_contain_buffer_or_pointer(attr)) || 
                (attr->type==bdmf_attr_pointer))
            {
                ++attr;
                continue;
            }
            if ((attr->flags & BDMF_ATTR_READ))
                rc = rc ? rc : user_func_api_attr_impl_gen(parm, attr, func_read, ioctl_str);
            if ((attr->flags & BDMF_ATTR_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
            {
                if ((attr->flags & BDMF_ATTR_UDEF_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
                    rc = rc ? rc : user_func_api_attr_impl_gen(parm, attr, func_write, ioctl_str);
                if (attr->add)
                    rc = rc ? rc : user_func_api_attr_impl_gen(parm, attr, func_add, ioctl_str);
                if (attr->del)
                    rc = rc ? rc : user_func_api_attr_impl_gen(parm, attr, func_delete, ioctl_str);
            }
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_GET_NEXT))
                rc = rc ? rc : user_func_api_attr_impl_gen(parm, attr, func_get_next, ioctl_str);
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_FIND))
                rc = rc ? rc : user_func_api_attr_impl_gen(parm, attr, func_find, ioctl_str);
        }
        ++attr;
    }

    fprintf(hf, "/** @} end of %s Doxygen group */\n\n\n", drv->name);
    fprintf(parm->hf, "\n\n#endif /* %s */\n", hdr_def);

    return rc;
}


static int user_ioctl_header_gen(codegen_parm_t *parm )
{
    char hdr_def[MAX_STR] = "";
    FILE *hf = parm->hf;
    const struct bdmf_attr *attr = parm->drv->aattr;
    char str[MAX_STR] = "";
    int rc;

    /* do nothing for root */
    if (!parm->drv->po)
        return 0;

    /* Generate head comments */
    rc = bdmf_mon_codegen_standard_gpl_comments(parm);
    if (rc)
        return rc;

    /* generate file header */
    snprintf(hdr_def, sizeof(hdr_def), "_%s_%s_USER_IOCTL_H_", parm->name_prefix,
            parm->drv->name);
    to_upper_case(hdr_def);

    fprintf(hf, "/*\n"
            " * %s user ioctl define header file.\n"
            " * This header file is generated automatically. Do not edit!\n"
            " */\n",  parm->name_prefix);
    fprintf(hf, "#ifndef %s\n"
            "#define %s\n\n"
            "enum\n"
            "{\n",
            hdr_def, hdr_def);

    /* create ioctls for get object and get driver */
    snprintf(str, sizeof(str), "%s_%s_drv,", parm->name_prefix, parm->drv->name);
    to_upper_case(str);
    fprintf(hf, "\t%s\n",str);


    snprintf(str, sizeof(str), "%s_%s_get,", parm->name_prefix, parm->drv->name);
    to_upper_case(str);
    fprintf(hf, "\t%s\n",str);

    /* create ioctls for all the attributs */
    while(attr && attr->name)
    {
        if ((attr->flags & parm->attr_level)!=0 &&
            !(attr->flags & BDMF_ATTR_NO_AUTO_GEN) &&
            ((parm->action_mask & BDMF_ACTION_MASK_ATTR_ALL)!=0 ||
             (attr->type==bdmf_attr_aggregate)) )
        {
            /* aggregate type containig buffer can't be generated in userspace */
            if ((attr->type==bdmf_attr_aggregate && is_attr_contain_buffer_or_pointer(attr)) || 
                (attr->type==bdmf_attr_pointer))
            {
                ++attr;
                continue;
            }
            if ((attr->flags & BDMF_ATTR_READ))
            {
                snprintf(str, sizeof(str), "%s_%s_%s_get,",
                        parm->name_prefix, parm->drv->name, attr->name);
                to_upper_case(str);
                fprintf(hf, "\t%s\n",str);
            }
            if ((attr->flags & BDMF_ATTR_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
            {
                if ((attr->flags & BDMF_ATTR_UDEF_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
                {
                    snprintf(str, sizeof(str), "%s_%s_%s_set,",
                            parm->name_prefix, parm->drv->name, attr->name);
                    to_upper_case(str);
                    fprintf(hf, "\t%s\n",str);
                }
                if (attr->add)
                {
                    snprintf(str, sizeof(str), "%s_%s_%s_add,",
                            parm->name_prefix, parm->drv->name, attr->name);
                    to_upper_case(str);
                    fprintf(hf, "\t%s\n",str);
                }
                if (attr->del)
                {
                    snprintf(str, sizeof(str), "%s_%s_%s_delete,",
                            parm->name_prefix, parm->drv->name, attr->name);
                    to_upper_case(str);
                    fprintf(hf, "\t%s\n",str);
                }
            }
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_GET_NEXT))
            {
                snprintf(str, sizeof(str), "%s_%s_%s_get_next,",
                        parm->name_prefix, parm->drv->name, attr->name);
                to_upper_case(str);
                fprintf(hf, "\t%s\n",str);
            }
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_FIND))
            {
                snprintf(str, sizeof(str), "%s_%s_%s_find,",
                        parm->name_prefix, parm->drv->name, attr->name);
                to_upper_case(str);
                fprintf(hf, "\t%s\n",str);
            }
        }
        ++attr;
    }

    fprintf(hf, "};\n\n"
            "#endif /* %s */\n", hdr_def);
    return 0;
}

int user_ioctl_decleration_gen(codegen_parm_t *parm)
{
    char hdr_def[MAX_STR] = "";
    FILE *hf = parm->hf;
    const struct bdmf_attr *attr = parm->drv->aattr;

    /* do nothing for root */
    if (!parm->drv->po)
        return 0;

    /* generate file header */
    snprintf(hdr_def, sizeof(hdr_def), "_%s_IOCTL_%s_H_", parm->name_prefix,
            parm->drv->name);
    to_upper_case(hdr_def);
    fprintf(hf, "/*\n"
            " * %s user ioctl declaration file.\n"
            " * This header file is generated automatically. Do not edit!\n"
            " */\n",  parm->name_prefix);
    fprintf(hf, "#ifndef %s\n"
            "#define %s\n\n",
            hdr_def, hdr_def );
    fprintf(hf, "int %s_user_%s_get(rdpa_ioctl_cmd_t *pa);\n", parm->name_prefix, parm->drv->name);
    fprintf(hf, "int %s_user_%s_drv(rdpa_ioctl_cmd_t *pa);\n", parm->name_prefix, parm->drv->name);

    while(attr && attr->name)
    {
        if ((attr->flags & parm->attr_level)!=0 &&
            !(attr->flags & BDMF_ATTR_NO_AUTO_GEN) &&
            ((parm->action_mask & BDMF_ACTION_MASK_ATTR_ALL)!=0 ||
             (attr->type==bdmf_attr_aggregate)) )
        {
            if ((attr->flags & BDMF_ATTR_READ))
                 fprintf(hf, "int %s_user_%s_%s_get(rdpa_ioctl_cmd_t *pa);\n", parm->name_prefix, parm->drv->name, attr->name);

            if ((attr->flags & BDMF_ATTR_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
            {
                if ((attr->flags & BDMF_ATTR_UDEF_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
                    fprintf(hf, "int %s_user_%s_%s_set(rdpa_ioctl_cmd_t *pa);\n", parm->name_prefix, parm->drv->name, attr->name);
                if (attr->add)
                    fprintf(hf, "int %s_user_%s_%s_add(rdpa_ioctl_cmd_t *pa);\n", parm->name_prefix, parm->drv->name, attr->name);
                if (attr->del)
                    fprintf(hf, "int %s_user_%s_%s_delete(rdpa_ioctl_cmd_t *pa);\n", parm->name_prefix, parm->drv->name, attr->name);
            }
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_GET_NEXT))
                fprintf(hf, "int %s_user_%s_%s_get_next(rdpa_ioctl_cmd_t *pa);\n", parm->name_prefix, parm->drv->name, attr->name);
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_FIND))
                fprintf(hf, "int %s_user_%s_%s_find(rdpa_ioctl_cmd_t *pa);\n", parm->name_prefix, parm->drv->name, attr->name);
        }
        ++attr;
    }

    fprintf(hf, "\n#endif /* %s */\n", hdr_def);
    return 0;
}




int user_ioctl_drv_gen(codegen_parm_t *parm)
{
    FILE *hf = parm->hf;
    static int is_first = 1;

    /* do nothing for root */
    if (!parm->drv->po)
        return 0;

    if(is_first)
    {
        fprintf(hf, "/*\n"
                " * %s user driver ioctl implementation file.\n"
                " * This header file is generated automatically. Do not edit!\n"
                " */\n", parm->name_prefix);
        fprintf(hf, "#include \"rdpa_user.h\"\n"
                "#include \"rdpa_user_int.h\"\n"
                "#include \"rdpa_user_ioctl_ag.h\"\n"
                "#include \"rdpa_ioctl_func_ag.h\"\n\n"
                "long rdpa_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)\n"
                "{\n"
                "int ret;\n\n"
                "\tswitch (op){\n");
        is_first = 0;
    }

    user_ioctl_drv_and_obj_case_gen(parm);
    user_ioctl_attr_case_gen(parm);

    fprintf(hf, "\t\tdefault:\n"
            "\t\t\tBDMF_TRACE_ERR(\"no such ioctl cmd: %%u\\n\", op);\n"
            "\t\t\tret = EINVAL;\n"
            "\t\t}\n\n"
            "\treturn ret;\n"
            "}\n");
    return 0;
}

/* Generate object-access gpl shim file for a single object type
 */
static int _bdmf_mon_codegen_gen1_gpl_shim(codegen_parm_t *parm)
{
    FILE *hf = parm->hf;
    const struct bdmf_type *drv = parm->drv;
    int rc = 0;

    /* do nothing for root */
    if (!parm->drv->po)
        return 0;

    /* Generate "xx_type_get" function */
    fprintf(hf, "\nbdmf_type_handle (*f_%s_%s_drv)(void);\n", parm->name_prefix, drv->name);
    fprintf(hf, "\nEXPORT_SYMBOL(f_%s_%s_drv);\n", parm->name_prefix, drv->name);
    _bdmf_mon_xx_drv_header(parm);
    fprintf(hf, "bdmf_type_handle %s_%s_drv(void)\n"
                "{\n"
                "   if (!f_%s_%s_drv)\n"
                "       return NULL;\n"
                "   return f_%s_%s_drv();\n"
                "}\n",
                parm->name_prefix, drv->name, parm->name_prefix, drv->name,
                parm->name_prefix, drv->name);
    fprintf(hf, "\nEXPORT_SYMBOL(%s_%s_drv);\n", parm->name_prefix, drv->name);

    /* Generate key and attribute access by key */
    if ((parm->action_mask & BDMF_ACTION_MASK_KEY))
        rc = _bdmf_mon_codegen_gen_key(parm);

    return rc;
}

/* Generate object-access header/gpl shim file for object type tree
 * starting from drv's children
 */
static int _bdmf_mon_codegen_gen_tree(codegen_parm_t *parm, struct bdmf_type *drv,
    int (*gen1)(codegen_parm_t *parm))
{
    int rc = 0;
    struct bdmf_type *d=(struct bdmf_type *)drv;
    parm->drv = drv;
    rc = gen1(parm);
    bdmf_type_get(parm->drv); /* will be reduced by "get_next" */
    while((d=bdmf_type_get_next(d)))
    {
        if (d->po == drv)
        {
            parm->drv = d;
            rc = rc ? rc : gen1(parm);
        }
    }
    parm->drv = drv;
    return rc;
}

static int open_user_files(char *dir_name,
                            FILE **ioctl_def_h_file,
                            FILE **user_ioctl_c_file,
                            codegen_parm_t *parm)
{
    char new_name[256];
    char *dir;

    dir = strstr(dir_name, "include/autogen");
    if (dir)
        *dir = '\0';
    else
        goto error;

    snprintf(new_name, sizeof(new_name),"%sautogen/rdpa_%s_user_ioctl_ag.h",dir_name, parm->drv->name);
    *ioctl_def_h_file = fopen(new_name, "w");
    if (!*ioctl_def_h_file)
    {
        bdmf_session_print(parm->session, "CODEGEN:2 errno %d \n", errno);

        bdmf_session_print(parm->session, "CODEGEN: can't open file %s for writing\n", new_name);
        goto error;
    }
    bdmf_session_print(parm->session, "CODEGEN: generating %s\n", new_name);

    snprintf(new_name, sizeof(new_name),"%srdpa_%s_ioctl_ag.c", dir_name, parm->drv->name);
    *user_ioctl_c_file = fopen(new_name, "w");
    if (!*user_ioctl_c_file)
    {
        bdmf_session_print(parm->session, "CODEGEN:3 errno %d \n", errno);

        bdmf_session_print(parm->session, "CODEGEN: can't open file %s for writing\n", new_name);
        goto error;
    }
    bdmf_session_print(parm->session, "CODEGEN: generating %s\n", new_name);

    return 0;

error:
    if(*ioctl_def_h_file)
        fclose(*ioctl_def_h_file);
    if(*user_ioctl_c_file)
        fclose(*user_ioctl_c_file);
    return BDMF_ERR_PARM;
}

static int bdmf_mon_codegen_gen_file(codegen_parm_t *parm, const char *fname, int hierarchical)
{
    FILE *user_ioctl_h_file = 0;
    FILE *user_ioctl_c_file = 0;
    char ch_filename[BDMF_MAX_CODEGEN_FILENAME_LEN];
    char *pdot;
    char hdr_def[64]="";
    int rc = 0, size_get = 0, size_left = BDMF_MAX_CODEGEN_FILENAME_LEN;

    /* Generate .h or .c file name */
    size_get = snprintf(ch_filename, size_left, "%s", fname);
    size_left -= size_get;

    pdot = strchr(ch_filename, '.');
    while(pdot && *(pdot+1)!= 'h' && *(pdot+1)!= 'c')
        pdot = strchr(pdot+1, '.');
    if (pdot)
        *pdot = 0;
    strncat(ch_filename, parm->gpl_shim ? ".c" : ".h", size_left - 1);

    parm->hf = fopen(ch_filename, "w");
    if (!parm->hf)
    {
        bdmf_session_print(parm->session, "CODEGEN: errno %d \n", errno);

        bdmf_session_print(parm->session, "CODEGEN: can't open file %s for writing\n", ch_filename);
        return BDMF_ERR_PARM;
    }
    bdmf_session_print(parm->session, "CODEGEN: generating %s\n", ch_filename);
    if(parm->user_file)
    {
        rc = open_user_files(ch_filename, &user_ioctl_h_file, &user_ioctl_c_file, parm);
        if (rc)
            goto exit;
    }

    /* Generate head comments */
    rc = bdmf_mon_codegen_standard_gpl_comments(parm);
    if (rc)
        goto exit;

    /* Generate file-specific comments */

    if (parm->gpl_shim)
    {
        fprintf(parm->hf, "/*\n"
                " * %s object GPL shim file.\n"
                " * This file is generated automatically. Do not edit!\n"
                " */\n", parm->drv->name);
        if (hierarchical)
            rc = _bdmf_mon_codegen_gen_tree(parm, parm->drv, _bdmf_mon_codegen_gen1_gpl_shim);
        else
            rc = _bdmf_mon_codegen_gen1_gpl_shim(parm);
        fprintf(parm->hf, "\nMODULE_LICENSE(\"GPL\");\n");
    }
    else if (parm->user_file)
    {
        FILE *temp;

        if (hierarchical)
            rc = _bdmf_mon_codegen_gen_tree(parm, parm->drv, user_func_api_header_gen);
        else
            rc = user_func_api_header_gen(parm);
        if (rc)
            goto exit;

        temp = parm->hf;
        parm->hf = user_ioctl_h_file;
        user_ioctl_header_gen(parm);
        parm->hf = user_ioctl_c_file;
        user_ioctl_impl_gen(parm);
        parm->hf = temp;
    }
    else
    {
        /* generate file header */
        snprintf(hdr_def, sizeof(hdr_def), "_%s_AG_%s_H_", parm->name_prefix,
            parm->drv->name);
        to_upper_case(hdr_def);
        fprintf(parm->hf, "/*\n"
                " * %s object header file.\n"
                " * This header file is generated automatically. Do not edit!\n"
                " */\n", parm->drv->name);
        fprintf(parm->hf, "#ifndef %s\n"
                "#define %s\n\n", hdr_def, hdr_def);
        if (hierarchical)
            rc = _bdmf_mon_codegen_gen_tree(parm, parm->drv, _bdmf_mon_codegen_gen1_header);
        else
            rc = _bdmf_mon_codegen_gen1_header(parm);
        fprintf(parm->hf, "\n\n#endif /* %s */\n", hdr_def);
    }

exit:
    if (parm->hf)
        fclose(parm->hf);
    if (user_ioctl_h_file)
    	fclose(user_ioctl_h_file);
    if (user_ioctl_c_file)
    	fclose(user_ioctl_c_file);

    return rc;
}

static int fdesc_line;

/* Find and read function name.
 * Returns function name in dynamically-allocated memory
 */
static char *_bdmf_codegen_get_fname(bdmf_session_handle session, FILE *hf)
{
    char buf[256];
    char *fname;
    char *pbuf;
    char *pend;

    while((pbuf=fgets(buf, sizeof(buf), hf)))
    {
        ++fdesc_line;
        if (!memcmp(buf, CODEGEN_FSTART_TAG, sizeof(CODEGEN_FSTART_TAG)-1))
            break;
    }
    if (!pbuf)
        return NULL;
    pbuf += sizeof(CODEGEN_FSTART_TAG);
    while(isspace(*pbuf) && *pbuf)
        ++pbuf;
    pend = pbuf;
    while(!isspace(*pend) && *pend)
        ++pend;
    *pend = 0;
    if (pend == pbuf)
    {
        bdmf_session_print(session, "CODEGEN: name is missing after %s in line %d\n", CODEGEN_FSTART_TAG, fdesc_line);
        return NULL;
    }
    fname = bdmf_alloc(strlen(pbuf)+1);
    if (!fname)
        return NULL;
    strcpy(fname, pbuf);
    return fname;
}

/* function description
 * Returns function description in dynamically-allocated memory
 */
static char *_bdmf_codegen_get_fdesc(bdmf_session_handle session, FILE *hf)
{
    char buf[256];
    char *pbuf;
    char *fdesc = NULL;

    while((pbuf=fgets(buf, sizeof(buf), hf)))
    {
        char *fdesc_tmp;
        ++fdesc_line;
        if (!memcmp(buf, CODEGEN_FEND_TAG, sizeof(CODEGEN_FEND_TAG)-1))
            break;
        if (fdesc)
            fdesc_tmp = realloc(fdesc, strlen(fdesc)+strlen(buf)+2);
        else
            fdesc_tmp = bdmf_alloc(strlen(buf)+1);
        if (!fdesc_tmp)
            return NULL;
        if (!fdesc)
            *fdesc_tmp = 0;
        strcat(fdesc_tmp, buf);
        fdesc = fdesc_tmp;
    }
    if (!pbuf)
    {
    	if (fdesc)
            bdmf_free(fdesc);
        bdmf_session_print(session, "CODEGEN: unexpected end of file. %s is missing\n", CODEGEN_FEND_TAG);
        return NULL;
    }
    return fdesc;
}

/* Load extended function descriptions
    BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
*/
static int bdmf_mon_codegen_load(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    const char *filename = parm[0].value.string;
    char *name, *desc;
    struct func_desc *fdesc;
    FILE *hf;
    int rc = BDMF_ERR_OK;

    if (!STAILQ_EMPTY(&func_desc_list))
    {
        bdmf_session_print(session, "CODEGEN: already loaded\n");
        return BDMF_ERR_ALREADY;
    }

    hf = fopen(filename, "r");
    if (!hf)
    {
        bdmf_session_print(session, "CODEGEN: can't open file %s for reading\n", filename);
        return BDMF_ERR_PARM;
    }

    do
    {
        name=_bdmf_codegen_get_fname(session, hf);
        desc=_bdmf_codegen_get_fdesc(session, hf);

        if (!name || !desc)
            break;

        fdesc = bdmf_calloc(sizeof(*fdesc));
        if (!fdesc)
        {
            rc = BDMF_ERR_NOMEM;
            break;
        }
        fdesc->name = name;
        fdesc->desc = desc;
        fdesc->is_tagged = (strstr(desc, "\\param") != NULL || strstr(desc, "\\return") != NULL);
        STAILQ_INSERT_TAIL(&func_desc_list, fdesc, list);
        name = NULL;
        desc = NULL;
    }
    while (1);
    bdmf_session_print(session, "CODEGEN: %d lines read from %s\n", fdesc_line, filename);

    if (name)
        bdmf_free(name);
    if (desc)
        bdmf_free(desc);
    if (hf)
        fclose(hf);

    return rc;
}

/* Generate object-access header file
    BDMFMON_MAKE_PARM("type", "Object type", BDMFMON_PARM_STRING, 0),
    BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
    BDMFMON_MAKE_PARM("name_prefix", "Type/function name prefix", BDMFMON_PARM_STRING, 0),
    BDMFMON_MAKE_PARM_RANGE_DEFVAL("what_generate", "1=header,2=GPL shim,3=both", BDMFMON_PARM_NUMBER, 0, 1, 3, 3),
    BDMFMON_MAKE_PARM_DEFVAL("mask", "What to gen bitmask: 1-attr enum,2-access funcs,4-maccess helpers",
                    BDMFMON_PARM_NUMBER, 0, 7),
    BDMFMON_MAKE_PARM_ENUM_DEFVAL("level", "Attribute level", attr_level_table, 0, "minor"),
    BDMFMON_MAKE_PARM_ENUM_DEFVAL("children", "Hierarchical", bdmfmon_enum_bool_table, 0, "no"),
*/
static int bdmf_mon_codegen_generate(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    const char *type = parm[0].value.string;
    const char *filename = parm[1].value.string;
    const char *name_prefix = parm[2].value.string;
    int what_generate = parm[3].value.unumber;
    uint32_t action_mask = parm[4].value.unumber;
    int level = parm[5].value.unumber;
    int children = parm[6].value.unumber;

    codegen_parm_t p;
    int rc;

    memset(&p, 0, sizeof(p));
    p.action_mask = action_mask;
    p.attr_level = level;
    p.name_prefix = name_prefix;
    p.session = session;

    if (!(action_mask & BDMF_ACTION_MASK_ALL))
    {
        bdmf_session_print(session, "CODEGEN: action mask is not set. Nothing to do\n");
        return -EINVAL;
    }
    bdmf_session_print(session, "CODEGEN: processing type %s. action=0x%x\n", type, action_mask);
    rc = bdmf_type_find_get(type, &p.drv);
    if (rc)
        return rc;

    if (what_generate & BDMF_FILE_USR)
    {
       p.user_file = 1;
       rc = bdmf_mon_codegen_gen_file(&p, filename, children);
    }
    if ((what_generate & BDMF_FILE_HEADER))
        rc = bdmf_mon_codegen_gen_file(&p, filename, children);
    if ((what_generate & BDMF_FILE_GPL_SHIM))
    {
        p.gpl_shim = 1;
        rc = rc ? rc : bdmf_mon_codegen_gen_file(&p, filename, children);
    }
    bdmf_type_put(p.drv);
    return rc;
}

/*
 * Python class generator
 */

/* Free name/type pair list */
static void bdmf_codegen_name_type_list_free(codegen_parm_t *p)
{
    codegen_name_type *nt, *tmp;
    TAILQ_FOREACH_SAFE(nt, &p->name_type_list, list, tmp)
    {
        TAILQ_REMOVE(&p->name_type_list, nt, list);
        bdmf_free(nt);
    }
}

/* Lookup entry in name/type list and add new entry if not fount */
static int bdmf_codegen_name_type_find_add(codegen_parm_t *p, const char *name,
    bdmf_attr_type_t type, bdmf_boolean *is_new)
{
    codegen_name_type *nt;
    TAILQ_FOREACH(nt, &p->name_type_list, list)
    {
        if (nt->type==type && !strcmp(nt->name, name))
            break;
    }
    if (nt)
    {
        *is_new = 0;
    }
    else
    {
        nt = bdmf_calloc(sizeof(*nt));
        if (!nt)
            return BDMF_ERR_NOMEM;
        nt->name = name;
        nt->type = type;
        TAILQ_INSERT_TAIL(&p->name_type_list, nt, list);
        *is_new = 1;
    }
    return BDMF_ERR_OK;
}

static int bdmf_codegen_pygen_openfile(codegen_parm_t *p, const char *name)
{
    char *fname;
    int rc;

    if (p->odir[0] && p->odir[strlen(p->odir)-1]!='/')
    {
        rc = asprintf(&fname, "%s/%s%s.py", p->odir, p->name_prefix, name);
    }
    else
    {
        rc = asprintf(&fname, "%s%s%s.py", p->odir, p->name_prefix, name);
    }
    if (rc < 0)
        return BDMF_ERR_NOMEM;

    p->hf = fopen(fname, "w");
    if (!p->hf)
    {
        bdmf_session_print(p->session, "PYGEN: can't open file %s for writing\n", fname);
        return BDMF_ERR_IO;
    }
    free(fname);

    return BDMF_ERR_OK;
}

/* Generate license header for Python class */
static int bdmf_codegen_pygen_license(codegen_parm_t *p, const char *name)
{
    return BDMF_ERR_OK;
}

/* Write a line and return BDMF error code */
static int bdmf_codegen_pygen_fprintf(codegen_parm_t *p, const char *fmt, ...)
{
    va_list ap;
    int rc;

    va_start(ap, fmt);
    rc = vfprintf(p->hf, fmt, ap);
    va_end(ap);

    return (rc <=0) ? BDMF_ERR_IO : BDMF_ERR_OK;
}

/* Generate "import" directive */
static int bdmf_codegen_pygen_add_import(codegen_parm_t *p,
    const char *prefix, const char *name)
{
    int rc;
    rc = bdmf_codegen_pygen_fprintf(p, "from %s%s import *\n", prefix ? prefix : "", name);
    return rc;
}

/* Generate imports */
static int bdmf_codegen_pygen_add_imports(codegen_parm_t *p)
{
    int rc = 0;
    //rc = bdmf_codegen_pygen_add_import(p, NULL, "CommonLib", NULL);
    rc = bdmf_codegen_pygen_add_import(p, NULL, "rdpa_object");
    rc = rc ? rc : bdmf_codegen_pygen_add_import(p, NULL, "rdpa_types");
    rc = rc ? rc : bdmf_codegen_pygen_add_import(p, NULL, "rdpa_validator");
    return rc;
}

/* Mangle name if it coinsides with reserved word in python */
static const char *bdmf_codegen_pygen_mangle_name(const char *name)
{
    const char *mangled_name;
    if (!strcmp(name, "as"))
        mangled_name = "_as";
    else
        mangled_name = name;
    return mangled_name;
}

/* Mangle class name: convert class name to camel case */
static const char *bdmf_codegen_pygen_mangle_class_name(const char *name, int is_type)
{
    static char mangled_name[BDMF_MAX_PYGEN_MANGLE_NAME_LEN];
    static char part[BDMF_MAX_PYGEN_MANGLE_NAME_LEN];
    const char *pname = name;
    const char *p;
    int size_left = BDMF_MAX_PYGEN_MANGLE_NAME_LEN;

    if (is_type)
        mangled_name[0] = 0;
    else
    {
        strcpy(mangled_name, "s");
        size_left -= 1;
    }
    while ((p=strchr(pname, '_')))
    {
        memcpy(part, pname, p-pname);
        part[p-pname] = 0;
        part[0] = toupper(part[0]);
        strncat(mangled_name, part, size_left - 1);
        size_left -= p-pname; /* p - pname is added string length*/
        pname = p+1;
    }
    strncpy(part, pname, sizeof(part));
    part[BDMF_MAX_PYGEN_MANGLE_NAME_LEN - 1] = 0;
    part[0] = toupper(part[0]);
    strncat(mangled_name, part, size_left - 1);
    return mangled_name;
}


int bdmf_codegen_pygen_print_attr_type_name(codegen_parm_t *p, const struct bdmf_attr *a, int is_index)
{
    bdmf_attr_type_t type = is_index ? a->index_type : a->type;
    int rc;

    switch (type)
    {
    case bdmf_attr_number:
        rc = bdmf_codegen_pygen_fprintf(p, "number");
        break;
    case bdmf_attr_string:
        rc = bdmf_codegen_pygen_fprintf(p, "string");
        break;
    case bdmf_attr_buffer:
        rc = bdmf_codegen_pygen_fprintf(p, "buffer");
        break;
    case bdmf_attr_pointer:
        rc = bdmf_codegen_pygen_fprintf(p, "pointer");
        break;
    case bdmf_attr_object:
        rc = bdmf_codegen_pygen_fprintf(p, "object");
        break;
    case bdmf_attr_ether_addr:
        rc = bdmf_codegen_pygen_fprintf(p, "ether_addr");
        break;
    case bdmf_attr_ip_addr:
        rc = bdmf_codegen_pygen_fprintf(p, "ip_v4_v6_addr");
        break;
    case bdmf_attr_ipv4_addr:
        rc = bdmf_codegen_pygen_fprintf(p, "ipv4_addr");
        break;
    case bdmf_attr_ipv6_addr:
        rc = bdmf_codegen_pygen_fprintf(p, "ipv6_addr");
        break;
    case bdmf_attr_boolean:
        rc = bdmf_codegen_pygen_fprintf(p, "boolean");
        break;
    case bdmf_attr_enum:
        rc = bdmf_codegen_pygen_fprintf(p, "enum %s",
            is_index ? a->index_ts.enum_table->type_name : a->ts.enum_table->type_name);
        break;
    case bdmf_attr_dyn_enum:
        rc = bdmf_codegen_pygen_fprintf(p, "dyn_enum");
        break;
    case bdmf_attr_enum_mask:
        rc = bdmf_codegen_pygen_fprintf(p, "enum mask %s",
            is_index ? a->index_ts.enum_table->type_name : a->ts.enum_table->type_name);
        break;
    case bdmf_attr_aggregate:
        rc = bdmf_codegen_pygen_fprintf(p, "%s()",
            is_index ? bdmf_codegen_pygen_mangle_class_name(a->index_ts.aggr_type_name, 0) :
                bdmf_codegen_pygen_mangle_class_name(a->ts.aggr_type_name, 0));
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* Add attributre parameter */
static int bdmf_codegen_pygen_add_parm(codegen_parm_t *p, const struct bdmf_attr *a)
{
    int rc = 0;
    char dft_buf[64];
    const char *dft = NULL;
    const char *mname = bdmf_codegen_pygen_mangle_name(a->name);

    /* Default value by type */
    switch(a->type)
    {
    case bdmf_attr_number:
    case bdmf_attr_string:
    case bdmf_attr_buffer:
    case bdmf_attr_pointer:
    case bdmf_attr_object:
    case bdmf_attr_ether_addr:
    case bdmf_attr_ip_addr:
    case bdmf_attr_ipv4_addr:
    case bdmf_attr_ipv6_addr:
    case bdmf_attr_boolean:
    case bdmf_attr_enum:
    case bdmf_attr_dyn_enum:
    case bdmf_attr_enum_mask:
        if (a->array_size)
            dft = "None";
        else
            dft = "''";
        break;

    case bdmf_attr_aggregate:
        if (a->array_size)
        {
            dft = "None";
        }
        else
        {
            snprintf(dft_buf, sizeof(dft_buf)-1, "%s()", bdmf_codegen_pygen_mangle_class_name(a->aggr_type->name, 0));
            dft = dft_buf;
        }
        break;

    default:
        break;
    }
    if (a->flags & (BDMF_ATTR_MANDATORY | BDMF_ATTR_KEY))
        dft = NULL;

    if (dft)
        rc = bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 ",%s=%s", mname, dft);
    else
        rc = bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 ",%s", mname);

    if (a->array_size)
    {
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, " # { ");
        rc = rc ? rc : bdmf_codegen_pygen_print_attr_type_name(p, a, 1);
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, " , ");
        rc = rc ? rc : bdmf_codegen_pygen_print_attr_type_name(p, a, 0);
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, " }");
    }
    else
    {
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, " # ");
        rc = rc ? rc : bdmf_codegen_pygen_print_attr_type_name(p, a, 0);
    }
    if ((a->flags & BDMF_ATTR_MANDATORY) != 0)
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, " - mandatory");
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, "\n");

    return rc;
}

/* Add code for aggregate attr assignment validation */
static int bdmf_codegen_pygen_add_validate_aggr(codegen_parm_t *p, const struct bdmf_attr *a, struct bdmf_aggr_type *ag,
    const char *value, const char *tabs)
{
    int rc;
    rc = bdmf_codegen_pygen_fprintf(p, "%sif type(%s) != %s:\n",
        tabs, value, bdmf_codegen_pygen_mangle_class_name(ag->name, 0));
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB "%sraise ValueError(str(type(self)), '%s', str(%s))\n",
        tabs, bdmf_codegen_pygen_mangle_name(a->name), value);
    return 0;
}

/* Add attribute assignment validators if necessary */
static int bdmf_codegen_pygen_add_attr_validators(codegen_parm_t *p, const struct bdmf_attr *a,
    const char *index,const char *value, const char *tabs)
{
    const char *mname = bdmf_codegen_pygen_mangle_name(a->name);
    int rc = BDMF_ERR_OK;

    if (value)
    {
        if (a->type == bdmf_attr_enum)
        {
            rc = bdmf_codegen_pygen_fprintf(p, "%sself.validate_enum('%s_value', %s)\n",
                tabs, mname, value);
        }
        else if (a->type == bdmf_attr_enum_mask)
        {
            rc = bdmf_codegen_pygen_fprintf(p, "%sself.validate_enum_mask('%s_value', %s)\n",
                tabs, mname, value);
        }
        else if (a->type == bdmf_attr_aggregate)
        {
            rc = bdmf_codegen_pygen_add_validate_aggr(p, a, a->aggr_type, value, tabs);
        }
        else if (a->type == bdmf_attr_ipv4_addr)
        {
            rc = bdmf_codegen_pygen_fprintf(p, "%sself.validate_ipv4_addr('%s_value', %s)\n",
                tabs, mname, value);
        }
        else if (a->type == bdmf_attr_ipv6_addr)
        {
            rc = bdmf_codegen_pygen_fprintf(p, "%sself.validate_ipv6_addr('%s_value', %s)\n",
                tabs, mname, value);
        }
        else if (a->type == bdmf_attr_ip_addr)
        {
            rc = bdmf_codegen_pygen_fprintf(p, "%sself.validate_ip_addr('%s_value', %s)\n",
                tabs, mname, value);
        }
        else if (a->type == bdmf_attr_ether_addr)
        {
            rc = bdmf_codegen_pygen_fprintf(p, "%sself.validate_ether_addr('%s_value', %s)\n",
                tabs, mname, value);
        }
    }
    if (index && !rc)
    {
        if (a->index_type == bdmf_attr_enum)
        {
            rc = bdmf_codegen_pygen_fprintf(p, "%sself.validate_enum('%s_index', %s)\n",
                tabs, mname, index);
        }
        else if (a->index_type == bdmf_attr_aggregate)
        {
            rc = bdmf_codegen_pygen_add_validate_aggr(p, a, a->index_aggr_type, index, tabs);
        }
        else if (a->index_type == bdmf_attr_ipv4_addr)
        {
            rc = bdmf_codegen_pygen_fprintf(p, "%sself.validate_ipv4_addr('%s_index', %s)\n",
                tabs, mname, index);
        }
        else if (a->index_type == bdmf_attr_ipv6_addr)
        {
            rc = bdmf_codegen_pygen_fprintf(p, "%sself.validate_ipv6_addr('%s_index', %s)\n",
                tabs, mname, index);
        }
        else if (a->index_type == bdmf_attr_ip_addr)
        {
            rc = bdmf_codegen_pygen_fprintf(p, "%sself.validate_ip_addr('%s_index', %s)\n",
                tabs, mname, index);
        }
        else if (a->index_type == bdmf_attr_ether_addr)
        {
            rc = bdmf_codegen_pygen_fprintf(p, "%sself.validate_ether_addr('%s_index', %s)\n",
                tabs, mname, value);
        }
    }

    return rc;
}

/* Generate string assigning all non-empty attributes  */
static int bdmf_codegen_pygen_attrlist(codegen_parm_t *p, const struct bdmf_attr *aa,
    uint32_t filter, uint32_t not_filter, int is_type)
{
    const struct bdmf_attr *a;
    char value_str[64];
    int rc;

    rc = bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "delim=''\n");
    for (a = aa; a->name; ++a)
    {
        if ((a->flags & (BDMF_ATTR_WRITE | BDMF_ATTR_WRITE_INIT)) &&
            (a->flags & p->attr_level) && (a->add == NULL) &&
            (!filter || (a->flags & filter) !=  0) &&
            ((a->flags & not_filter) == 0))
        {
            const char *mname = bdmf_codegen_pygen_mangle_name(a->name);
            if (!a->array_size)
            {
                snprintf(value_str, sizeof(value_str)-1, "self.%s", mname);
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "if self.%s != self.__prev_%s:\n",
                    mname, mname);
                /* Validate assignment */
                rc = rc ? rc : bdmf_codegen_pygen_add_attr_validators(p, a, NULL, value_str, BDMF_TAB3);
                /* Don't modify __prev value for key attributes to make sure they are always included */
                if ((a->flags & BDMF_ATTR_KEY) == 0)
                {
                    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB3 "self.__prev_%s = self.%s\n",
                        mname, mname);
                }
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB3 "s = s + delim + '%s=' + str(self.%s)\n", mname, mname);
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB3 "delim = ','\n");
            }
            else
            {
                /* If we are here - it is object type. Aggregates don't support arrays */
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "if self.%s != self.__prev_%s:\n",
                    mname, mname);
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB3 "items = self.%s.items()\n", mname);
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB3 "for it in items:\n");
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p,
                    BDMF_TAB4 "if self.__prev_%s.get(it[0]) != it[1]:\n", mname);
                /* Validate assignment */
                rc = rc ? rc : bdmf_codegen_pygen_add_attr_validators(p, a, "it[0]", "it[1]", BDMF_TAB5);
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p,
                    BDMF_TAB5 "s = s + delim + '%s[' + str(it[0]) + ']=' + str(it[1])\n", mname);
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB5 "delim = ','\n");
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB3 "self.__prev_%s = self.%s.copy()\n",
                    mname, mname);
            }
        }
    }
    return rc;
}

/* Add method for attributes having "add" callback */
static int bdmf_codegen_pygen_add_method(codegen_parm_t *p, const char *name, const struct bdmf_attr *a)
{
    int rc;
    rc = bdmf_codegen_pygen_fprintf(p, BDMF_TAB "def %s_add(self,value):\n", a->name);
    /* Validate value */
    rc = rc ? rc : bdmf_codegen_pygen_add_attr_validators(p, a, NULL, "value", BDMF_TAB2);
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "super(%s, self).attr_add('%s', value)\n",
        bdmf_codegen_pygen_mangle_class_name(name, 1), a->name);
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, "\n\n");
    return rc;
}

/* Add method for for generation /bdmf/attr/set commands */
static int bdmf_codegen_pygen_set_method(codegen_parm_t *p, const char *name, const struct bdmf_attr *a)
{
    int rc;
    const char *index_or_none;
    const char *mname = bdmf_codegen_pygen_mangle_name(a->name);

    if (a->array_size)
    {
        rc = bdmf_codegen_pygen_fprintf(p, BDMF_TAB "def %s_set(self, index, value):\n", a->name);
        /* Validate index */
        rc = rc ? rc : bdmf_codegen_pygen_add_attr_validators(p, a, "index", NULL, BDMF_TAB2);
        index_or_none = "index";
    }
    else
    {
        rc = bdmf_codegen_pygen_fprintf(p, BDMF_TAB "def %s_set(self, value):\n", a->name);
        index_or_none = "None";
    }
    /* Validate value */
    rc = rc ? rc : bdmf_codegen_pygen_add_attr_validators(p, a, NULL, "value", BDMF_TAB2);
    if (a->array_size)
    {
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "self.%s[str(index)] = value\n", mname);
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "self.__prev_%s = self.%s.copy()\n",
            mname, mname);
    }
    else
    {
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "self.%s = value\n", mname);
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "self.__prev_%s = self.%s\n",
            mname, mname);
    }
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "super(%s, self).attr_set('%s', %s, value)\n",
        bdmf_codegen_pygen_mangle_class_name(name, 1), a->name, index_or_none);
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, "\n\n");
    return rc;
}

/* Delete method for attributes having "delete" callback */
static int bdmf_codegen_pygen_delete_method(codegen_parm_t *p, const char *name, const struct bdmf_attr *a)
{
    int rc;
    rc = bdmf_codegen_pygen_fprintf(p, BDMF_TAB "def %s_delete(self,index):\n", a->name);
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "super(%s, self).attr_delete('%s', index)\n",
        bdmf_codegen_pygen_mangle_class_name(name, 1), a->name);
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, "\n\n");
    return rc;
}


/* Generate Python class for an aggregate or object type */
static int bdmf_codegen_pygen_class(codegen_parm_t *p, const char *name, const char *descr,
    const char *suffix, const struct bdmf_attr *aa, int is_type)
{
    const struct bdmf_attr *a;
    int rc = BDMF_ERR_OK;

    bdmf_session_print(p->session, "PYGEN: generating class %s%s\n",
        bdmf_codegen_pygen_mangle_class_name(name, is_type), suffix);

    /* Now add class definition */
    if (descr)
        rc = bdmf_codegen_pygen_fprintf(p, "\n# %s", descr);
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, "\nclass %s%s%s:\n",
        bdmf_codegen_pygen_mangle_class_name(name, is_type), suffix, is_type ? "(Obj)" : "(Validator)");
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB "def __init__(self\n");
    if (is_type)
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 ",session\n");

    /* Add class variable for each attribute */
    for (a = aa; a->name; ++a)
    {
        if ((a->flags & (BDMF_ATTR_WRITE | BDMF_ATTR_WRITE_INIT)) &&
            (a->flags & p->attr_level))
        {
            rc = rc ? rc : bdmf_codegen_pygen_add_parm(p, a);
        }
    }
    if (is_type)
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 ",parent = None\n");
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "):\n");

    /* Base class initialization */
    if (is_type)
    {
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "super(%s%s, self).__init__('%s%s', session, parent)\n",
            bdmf_codegen_pygen_mangle_class_name(name, is_type), suffix, name, suffix);
    }
    else
    {
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "super(%s%s, self).__init__()\n",
            bdmf_codegen_pygen_mangle_class_name(name, is_type), suffix);
    }

    /* Add self.member=parm for each attribute */
    for (a = aa; a->name; ++a)
    {
        if ((a->flags & (BDMF_ATTR_WRITE | BDMF_ATTR_WRITE_INIT)) &&
            (a->flags & p->attr_level))
        {
            const char *mname = bdmf_codegen_pygen_mangle_name(a->name);
            if (a->array_size)
            {
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "if %s is None:\n", mname);
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB3 "%s = {}\n", mname);
            }
            rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "self.%s = %s\n", mname, mname);
            if (a->array_size)
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "self.__prev_%s = {}\n", mname);
            else if ((a->flags & (BDMF_ATTR_MANDATORY | BDMF_ATTR_KEY)) != 0)
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "self.__prev_%s = '*invalid*'\n", mname);
            else if (a->type != bdmf_attr_aggregate)
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "self.__prev_%s = ''\n", mname);
            else
                rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "self.__prev_%s = %s()\n",
                    mname, bdmf_codegen_pygen_mangle_class_name(a->aggr_type->name, 0));
        }
    }
    /* Add validators */
    rc = rc ? rc : bdmf_codegen_pygen_types_from_attr_list(p, aa, PYGEN_GEN_TYPE_VALIDATOR);
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, "\n");

    /* Emit __str__ method only for aggregates. For object types it is implemented in the base class */
    if (!is_type)
    {
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB "def __str__(self):\n");
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "s = '{'\n");
        rc = rc ? rc : bdmf_codegen_pygen_attrlist(p, aa, 0, 0, is_type);
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "s = s + '}'\n");
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "s=s.replace(' ','')\n");
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "return s\n");
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, "\n");
    }

    /* Emit __eq__ method */
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB "def __eq__(self, other):\n");
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "if type(self) != type(other):\n");
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB3 "return False\n");
    for (a = aa; a->name; ++a)
    {
        if ((a->flags & (BDMF_ATTR_WRITE | BDMF_ATTR_WRITE_INIT)) != 0)
        {
            const char *mname = bdmf_codegen_pygen_mangle_name(a->name);
            rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "if self.%s != other.%s:\n", mname, mname);
            rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB3 "return False\n");
        }
    }
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "return True\n\n");

    /* Emit __ne__ method */
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB "def __ne__(self, other):\n");
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "return not (self == other)\n\n");

    if (is_type)
    {
        /* Emit assigned_attrs_str method */
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB "def assigned_attrs_str(self):\n");
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "s=''\n");
        rc = rc ? rc : bdmf_codegen_pygen_attrlist(p, aa, 0, 0, is_type);
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "s=s.replace(' ','')\n");
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "return s\n");
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, "\n");

        /* Emit key_attrs_str method */
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB "def key_attrs_str(self):\n");
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "s=''\n");
        rc = rc ? rc : bdmf_codegen_pygen_attrlist(p, aa, BDMF_ATTR_KEY, 0, is_type);
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "s=s.replace(' ','')\n");
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "return s\n");
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, "\n");

        /* Emit assigned_not_key_attrs_str method */
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB "def assigned_not_key_attrs_str(self):\n");
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "s=''\n");
        rc = rc ? rc : bdmf_codegen_pygen_attrlist(p, aa, 0, BDMF_ATTR_KEY, is_type);
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "s=s.replace(' ','')\n");
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "return s\n");
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, "\n");

        /* For each attribute having "add" callback, emit <attr_name>_add method */
        for (a = aa; a->name; ++a)
        {
            if ((a->flags & (BDMF_ATTR_WRITE | BDMF_ATTR_WRITE_INIT)) &&
                (a->flags & p->attr_level) && (a->add != NULL))
            {
                rc = rc ? rc : bdmf_codegen_pygen_add_method(p, name, a);
            }
        }

        /* For each attribute with user-defined write callback emit <attr_name>_set method */
        for (a = aa; a->name; ++a)
        {
            if ((a->flags & (BDMF_ATTR_UDEF_WRITE | BDMF_ATTR_WRITE_INIT)))
            {
                rc = rc ? rc : bdmf_codegen_pygen_set_method(p, name, a);
            }
        }

        /* For each attribute having "delete" callback, emit <attr_name>_delete method */
        for (a = aa; a->name; ++a)
        {
            if ((a->flags & (BDMF_ATTR_WRITE | BDMF_ATTR_WRITE_INIT)) &&
                (a->flags & p->attr_level) && (a->del != NULL))
            {
                rc = rc ? rc : bdmf_codegen_pygen_delete_method(p, name, a);
            }
        }
    }

    /* Emit "validate_enum()" method */

    return rc;
}

/* Generate Python method for enum validation */
static int bdmf_codegen_pygen_enum_validator(codegen_parm_t *p, const struct bdmf_attr *a, bdmf_boolean is_index)
{
    const bdmf_attr_enum_table_t *enum_table = is_index ? a->index_ts.enum_table : a->ts.enum_table;
    int rc = BDMF_ERR_OK;
    const bdmf_attr_enum_val_t *v;

    /* Now add class definition */
    if (enum_table->help)
        rc = bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "# %s\n", enum_table->help);
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, BDMF_TAB2 "self.enum_validator['%s_%s'] = [",
        a->name, is_index ? "index" : "value");

    for (v = enum_table->values; v && v->name; v++)
    {
        rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, "%s'%s'", (v==enum_table->values) ? "" : ",", v->name);
    }
    rc = rc ? rc : bdmf_codegen_pygen_fprintf(p, "]\n");

    return rc;
}

/* Generate Python class for an aggregate */
static int bdmf_codegen_pygen_aggr(codegen_parm_t *p, const struct bdmf_aggr_type *ag)
{
    int rc;
    /* Generate types for all aggregate's attributes (if necessary) */
    rc = bdmf_codegen_pygen_types_from_attr_list(p, ag->fields, PYGEN_GEN_TYPE_CLASS);
    rc = rc ? rc : bdmf_codegen_pygen_class(p, ag->name, ag->help, "", ag->fields, 0);
    return rc;
}

/* Generate Python class for an attribute type */
static int bdmf_codegen_pygen_attr_type(codegen_parm_t *p, const struct bdmf_attr *a,
    bdmf_boolean is_index, pygen_gen_type gen_type)
{
    const char *name = NULL;
    bdmf_attr_type_t type;
    bdmf_boolean is_new;
    int rc = BDMF_ERR_OK;

    if (!is_index)
    {
        type = a->type;
        if (type == bdmf_attr_aggregate)
            name = a->ts.aggr_type_name;
        else if (type == bdmf_attr_enum || type == bdmf_attr_enum_mask)
            name = a->ts.enum_table->type_name;
    }
    else
    {
        type = a->index_type;
        if (type == bdmf_attr_aggregate)
            name = a->index_ts.aggr_type_name;
        else if (type == bdmf_attr_enum || type == bdmf_attr_enum_mask)
            name = a->index_ts.enum_table->type_name;
    }

    if (name == NULL)
        return BDMF_ERR_OK;

    /* Generate class */
    switch (type)
    {
    case bdmf_attr_aggregate:
        /* See if class haven't been generated already */
        rc = bdmf_codegen_name_type_find_add(p, name, type, &is_new);
        if (rc)
            return rc;
        if (!is_new)
            return BDMF_ERR_OK;
        rc = bdmf_codegen_pygen_aggr(p, is_index ? a->index_aggr_type : a->aggr_type);
        break;
    case bdmf_attr_enum:
    case bdmf_attr_enum_mask:
        if (gen_type == PYGEN_GEN_TYPE_VALIDATOR)
            rc = bdmf_codegen_pygen_enum_validator(p, a, is_index);
        break;
    default:
        break;
    }

    return rc;
}

/* Generate Python types for attribute list */
static int bdmf_codegen_pygen_types_from_attr_list(codegen_parm_t *p, const struct bdmf_attr *aa, pygen_gen_type gen_type)
{
    const struct bdmf_attr *a;
    int rc = BDMF_ERR_OK;

    if (!aa)
        return BDMF_ERR_OK;

    /* Generate python classes / validation methods for write-able attributes of the following types:
     * - aggregate
     * - boolean
     * - enum
     * - enum_mask
     */
    for (a = aa; a->name && rc == BDMF_ERR_OK; ++a)
    {
        if ((a->flags & (BDMF_ATTR_WRITE | BDMF_ATTR_WRITE_INIT)) &&
            (a->flags & p->attr_level))
        {
            rc = bdmf_codegen_pygen_attr_type(p, a, 0, gen_type);
            rc = rc ? rc : bdmf_codegen_pygen_attr_type(p, a, 1, gen_type);
        }
    }

    return rc;
}

/* Generate Python class for an object type */
static int bdmf_codegen_pygen_type(codegen_parm_t *p, bdmf_type_handle ot)
{
    struct bdmf_type *t = (struct bdmf_type *)(long)ot;
    int rc = 0;
    if (t->aattr)
    {
        rc = bdmf_codegen_pygen_openfile(p, t->name);
        rc = rc ? rc : bdmf_codegen_pygen_license(p, t->name);
        rc = rc ? rc : bdmf_codegen_pygen_add_imports(p);
        rc = rc ? rc : bdmf_codegen_pygen_class(p, t->name, t->description, "", t->aattr, 1);
        if (p->hf != NULL)
        {
            fclose(p->hf);
            p->hf = NULL;
        }
    }
    return rc;
}

/* Generate Python classes
    BDMFMON_MAKE_PARM("odir", "Output directory", BDMFMON_PARM_STRING, 0),
    BDMFMON_MAKE_PARM("name_prefix", "Type/function name prefix", BDMFMON_PARM_STRING, 0),
    BDMFMON_MAKE_PARM_ENUM_DEFVAL("level", "Attribute level", attr_level_table, 0, "minor"),
*/
static int bdmf_mon_codegen_pygen(bdmf_session_handle session,
    const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    const char *odir = parm[0].value.string;
    const char *name_prefix = parm[1].value.string;
    int level = parm[2].value.unumber;

    char *mkdir_cmd;
    codegen_parm_t p = {};
    bdmf_type_handle ot = 0;
    int rc;

    /* Create output directory */
    if (asprintf(&mkdir_cmd, "mkdir -p %s", odir) <= 0)
        return BDMF_ERR_NOMEM;

    rc = system(mkdir_cmd);
    free(mkdir_cmd);
    if (rc)
    {
        bdmf_session_print(session, "PYGEN: couldn't create output directory %s.\n", odir);
        return BDMF_ERR_IO;
    }

    p.odir = odir;
    p.attr_level = level;
    p.name_prefix = name_prefix;
    p.session = session;
    TAILQ_INIT(&p.name_type_list);

    /* Go over all attributes and generate classes for non-trivial types */
    rc = bdmf_codegen_pygen_openfile(&p, "types");
    rc = rc ? rc : bdmf_codegen_pygen_license(&p, "RDPA Data Types");
    rc = rc ? rc : bdmf_codegen_pygen_add_import(&p, NULL, "rdpa_validator");
    ot = NULL;
    while ((ot=bdmf_type_get_next(ot)) != 0)
    {
        struct bdmf_type *t = (struct bdmf_type *)(long)ot;
        rc = rc ? rc : bdmf_codegen_pygen_types_from_attr_list(&p, t->aattr, PYGEN_GEN_TYPE_CLASS);
    }
    if (p.hf != NULL)
    {
        fclose(p.hf);
        p.hf = NULL;
    }

    /* Generate class per object type */
    ot = NULL;
    while ((ot=bdmf_type_get_next(ot)) != 0)
    {
        rc = rc ? rc : bdmf_codegen_pygen_type(&p, ot);
    }

    bdmf_codegen_name_type_list_free(&p);

    return rc;
}

bdmfmon_handle_t bdmf_codegen_mon_init(void)
{
    bdmfmon_handle_t bdmf_dir;
    bdmfmon_handle_t codegen_dir;

    STAILQ_INIT(&func_desc_list);

    bdmf_dir=bdmfmon_dir_find(NULL, "bdmf");
    codegen_dir = bdmfmon_dir_add(bdmf_dir, "codegen",
                             "Code Generator",
                             BDMF_ACCESS_ADMIN, NULL);

    /* Load extended function descriptions
     * Available only in simulation environment
     */
    {
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(codegen_dir, "load", bdmf_mon_codegen_load,
                      "Load extended function descriptions",
                      BDMF_ACCESS_GUEST, NULL, parms);
    }

    /* Object access header  file generation.
     * Available only in simulation environment
     */
    {
        static bdmfmon_enum_val_t attr_level_table[] = {
            { .name="major",  .val=BDMF_ATTR_MAJOR},          /* Major only */
            { .name="minor",  .val=BDMF_ATTR_MAJOR | BDMF_ATTR_MINOR }, /* Major+minor */
            { .name="debug",  .val=BDMF_ATTR_MAJOR | BDMF_ATTR_MINOR | BDMF_ATTR_HIDDEN }, /* all */
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM("type", "Object type", BDMFMON_PARM_STRING, 0),
            BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
            BDMFMON_MAKE_PARM("name_prefix", "Type/function name prefix", BDMFMON_PARM_STRING, 0),
            BDMFMON_MAKE_PARM_RANGE_DEFVAL("what_generate", "1=header,2=GPL shim,3=both,4=usr", BDMFMON_PARM_NUMBER, 0,
                BDMF_FILE_HEADER, BDMF_FILE_USR, BDMF_FILE_USR),
            BDMFMON_MAKE_PARM_DEFVAL("action_mask", "What bitmask: 1-attr enum,2-key,4-ag,8-all; 10-access,20-maccess,40-gen access",
                            BDMFMON_PARM_NUMBER, 0, BDMF_ACTION_MASK_ALL_NO_GEN_ACCESS),
            BDMFMON_MAKE_PARM_ENUM_DEFVAL("level", "Attribute level", attr_level_table, 0, "minor"),
            BDMFMON_MAKE_PARM_ENUM_DEFVAL("children", "Hierarchical", bdmfmon_enum_bool_table, 0, "no"),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(codegen_dir, "generate", bdmf_mon_codegen_generate,
                      "Generate header file(s)",
                      BDMF_ACCESS_GUEST, NULL, parms);
    }

    /* Python classes generation.
     */
    {
        static bdmfmon_enum_val_t attr_level_table[] = {
            { .name="major",  .val=BDMF_ATTR_MAJOR},          /* Major only */
            { .name="minor",  .val=BDMF_ATTR_MAJOR | BDMF_ATTR_MINOR }, /* Major+minor */
            { .name="debug",  .val=BDMF_ATTR_MAJOR | BDMF_ATTR_MINOR | BDMF_ATTR_HIDDEN }, /* all */
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM("odir", "Output directory", BDMFMON_PARM_STRING, 0),
            BDMFMON_MAKE_PARM("name_prefix", "Type/function name prefix", BDMFMON_PARM_STRING, 0),
            BDMFMON_MAKE_PARM_ENUM_DEFVAL("level", "Attribute level", attr_level_table, 0, "minor"),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(codegen_dir, "pygen", bdmf_mon_codegen_pygen,
                      "Generate Python classes",
                      BDMF_ACCESS_GUEST, NULL, parms);
    }
    return codegen_dir;
}
