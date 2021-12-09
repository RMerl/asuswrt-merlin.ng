/*
* <:copyright-BRCM:2013-2015:GPL/GPL:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
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
*/


/*******************************************************************
 * bdmf_dev.h
 *
 * Broadlight Device Management Framework - device driver interface
 *
 * This file is Copyright (c) 2011, Broadlight.
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

#ifndef _BDMF_DEV_H_
#define _BDMF_DEV_H_

/* Indicate that we are in BDMF driver rather than application */
#define BDMF_DRIVER

#include <bdmf_system.h>
#include <bdmf_session.h>

/** \defgroup bdmf_dev Broadlight Device Management Framework - Device Driver interface
 * This header file complements bdmf_interface.h and is intended for use
 * by BDMF plug-in device drivers in addition to bdmf_interface.h
 * @{
 */

#include <bdmf_interface.h>

/** Access type */
typedef enum
{
    bdmf_access_read,   /**< Read access */
    bdmf_access_write,  /**< Write access */
    bdmf_access_list    /**< List access */
} bdmf_access_type_t;

/** \defgroup bdmf_memarea Memory area control
 * \ingroup bdmf_dev
 * Memory areas facilitate generic access to object attributes located in different memories.
 * For instance, attributes located in host memory and in SRAM
 * may have to be accessed differently.
 * @{
 */

/** Memory types
 */
typedef enum
{
    BDMF_MEM_CACHE,         /**< Cacheable MIPS DDR */
    BDMF_MEM_NCACHE,        /**< Non-cacheable MIPS DDR */
    BDMF_MEM_TM_SRAM,       /**< TM SRAM */

    BDMF_MEM__NUMBER_OF
} bdmf_mem_type_t;

/** Memory area descriptor
 */
struct bdmf_mem_area
{
    const char *name;           /**< Memory area name */
    bdmf_mem_type_t mem_type;   /**< Memory area type */
    void *drv_priv;             /**< Field for driver use */
    /** Memory read callback function.
     * Returns number of bytes read >=0 or error code <0
     */
    int (*read)(struct bdmf_mem_area *area, void *dst, const void *src, uint32_t size);
    /** Memory write callback function
     * Returns number of bytes written >=0 or error code <0
     */
    int (*write)(struct bdmf_mem_area *area, void *dst, const void *src, uint32_t size);
    /** Memory allocation/reservation callback function */
    void  *(*alloc)(struct bdmf_mem_area *area, unsigned long start,
                    uint32_t size, uint32_t align);
    /** Memory de-allocation callback function */
    void  (*free)(struct bdmf_mem_area *area, void *ptr);
};
/** @} */

/** \defgroup bdmf_dev_attr Attribute control
 * \ingroup bdmf_dev
 * Managed objects are configured by setting their attributes.
 * List of attributes supported by managed object is specified by plug-in driver
 * at managed object type's registration time (bdmf_type_register()).
 * Driver can use built-in attribute types from #bdmf_attr_type_t list
 * or register new aggregate sub-type of #bdmf_attr_aggregate of attributes
 * or use fully customizable #bdmf_attr_custom type.
 * @{
 */

/** Enum attribute value */
typedef struct bdmf_attr_enum_val
{
    const char *name;   /**< Attribute name */
    long val;           /**< Attribute value */
} bdmf_attr_enum_val_t;

/** Enum attribute table */
typedef struct bdmf_attr_enum_table
{
    const char *type_name;          /**< Optional underlying enum type name if any */
    const char *help;               /**< Optional help string */
    bdmf_attr_enum_val_t values[];  /**< Enum values array. Terminated by entry with name==NULL */
} bdmf_attr_enum_table_t;

/** aggregate attribute type descriptor
 */
struct bdmf_aggr_type
{
    const char *name;               /**< Type name */
    const char *help;               /**< Descriptor */
    const char *struct_name;        /**< Underlying structure name */
    struct bdmf_attr *fields;       /**< Fields array */
    int size;                       /**< Aggregate size. Can be calculated automatically */
    uint32_t extra_flags;           /**< Extra flags set automatically in all fields */

    /* internal fields */
    int use_count;                  /* Number of types referring to the type */
    int deleted;                    /* set=1 when type is deleted */
    int has_references;             /* aggregate contains references */
    int references_verified;        /* set when all references have been verified */
    int need_validation;            /* fields has to be validated when setting attribute.
                                       It happens of aggregate contains object references or
                                       numbers that have to be range-checked
                                    */
    TAILQ_ENTRY(bdmf_aggr_type) list; /* Aggregate type list */
};

/** get_next_val function prototype for dynamic enums (#bdmf_attr_dyn_enum).
    On input it accepts string val=the current enum string value (NULL=get first).
    On output it returns the next string value - via the return value
    and its numeric value in *nval
*/
typedef const char *(*F_get_next_enum_val)(const char *val, bdmf_enum *nval);

/** Attribute descriptor
 */
struct bdmf_attr
{
    const char *name;           /**< Attribute name */
    const char *help;           /**< Attribute help string */
    const char *deprecated_text;/**< Deprecated help string */
    bdmf_attr_type_t type;      /**< Attribute type */
    bdmf_attr_type_t index_type;/**< Optional index type */
    uint8_t mem_seg;            /**< Memory segment index - for automatic access */
    uint16_t offset;            /**< Optional attribute offset in memory segment - for automatic access */
    uint16_t bit_offset;        /**< Optional attribute bit offset - for automatic offset.
                                     Total bit offset in memory segment = (offset*8)+bit_offset
                                */
    uint16_t size;              /**< Attribute size in bytes if bit_offset is 0 or bits if bit_offset is > 0*/
    uint32_t array_size;        /**< Attribute array dimension. Must be set >=1 for attribute arrays */
    uint16_t index_size;        /* Attribute array index size. Set internally. Do not touch */
    uint32_t flags;             /**< Attribute flags: a combination of BDMF_ATTR_xx constants (\ref bdmf_attr_flags) */
    bdmf_number min_val;        /**< Min value for range check */
    bdmf_number max_val;        /**< Max value for range check */
    bdmf_number disable_val;    /**< "Disable" value for CLI */

    /** Type-specific fields */
    union {
        const char *format;     /**< optional prints/scanf format string */
        const bdmf_attr_enum_table_t *enum_table;   /**< Enum values table */
        F_get_next_enum_val f_get_next_value;       /**< Function returning next value of dynamic enum */
        const char *aggr_type_name; /**< Aggregate type name */
        const char *ref_type_name;  /**< Referenced type name */
    } ts;

    /** Index-type-specific fields */
    union {
        const char *format;     /**< optional prints/scanf format string */
        const bdmf_attr_enum_table_t *enum_table;   /**< Enum values table */
        const char *aggr_type_name; /**< Aggregate type name */
    } index_ts;

    /** Optional read callback: value is in native format.
     * returns number of bytes read>=0 or error code <0
     */
    int (*read)(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size);

    /** Optional write callback: value is in native format
     * returns number of bytes written>=0 or error code <0
     */
    int (*write)(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size);

    /** Optional callback that converts value from string to internal format.
     * returns 0 on success or error code <0
     */
    int (*s_to_val)(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val, uint32_t size);

    /** Optional callback that converts value from internal to string format.
     * returns 0 on success or error code <0
     */
    int (*val_to_s)(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf, uint32_t size);

    /** Optional add callback: value is in native format.
      * optionally takes a hint in *index. updates *index
      * returns 0 on success or error code <0
      */
    int (*add)(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, const void *val, uint32_t size);

    /** Optional delete callback: takes attr index to be deleted */
    int (*del)(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index);

    /** Optional get_next callback: next "filled" array index is set in *index.
     * returns 0=OK, BDMF_ERR_NOENT-no more,other error codes
     */
    int (*get_next)(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index);

    /** Optional find callback: looks up attribute array entry.
     * Updates *index and can update *val as well.
     * returns 0=OK, BDMF_ERR_NOENT-not found,other error codes
     */
    int (*find)(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, void *val, uint32_t size);

    /** Optional is_field_visible callback allows
     * selectively hiding aggregate fields for CLI
     * returns 1=enabled, 0=disabled
     */
    int (*is_field_visible)(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val,
        struct bdmf_aggr_type *aggr, struct bdmf_attr *field);

    /** Optional underlying data type name. Used by code generator */
    const char *data_type_name;

    /** Optional callback that converts index from string to internal format.
     * returns 0 on success or error code <0
     */
    int (*s_to_index)(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *index, uint32_t size);

    /** Optional callback that converts index from internal to string format.
     * returns 0 on success or error code <0
     */
    int (*index_to_s)(struct bdmf_object *mo, struct bdmf_attr *ad, const void *index, char *sbuf, uint32_t size);

    /* Device type that owns the attribute */
    struct bdmf_type *owner;

    /* Aggregate type descriptor. Set by the framework */
    struct bdmf_aggr_type *aggr_type;

    /* Index aggregate type descriptor. Set by the framework */
    struct bdmf_aggr_type *index_aggr_type;

    /* Referenced object type. NULL=any */
    struct bdmf_type *ref_obj_type;
};

#define BDMF_ATTR_LAST { .name=NULL }   /**< This macro must be the last entry in attrubte descriptors array */

/** \defgroup bdmf_attr_flags Attribute flags
 * @{
 */
#define BDMF_ATTR_WRITE         0x00000001  /**< Write access */
#define BDMF_ATTR_WRITE_INIT    0x00000002  /**< Write access at object init time */
#define BDMF_ATTR_READ          0x00000004  /**< Read access */
#define BDMF_ATTR_MANDATORY     0x00000008  /**< Must be set at object init time or when containing aggregate attribute is set */
#define BDMF_ATTR_KEY           0x00000010  /**< Attribute is a key for identifying the object */
#define BDMF_ATTR_CLEAR_FIELDS  0x00000020  /**< Do not preserve values of unspecified fields of aggregate attribute */
#define BDMF_ATTR_ALLOW_UNSET_INDEX 0x00000040  /**< Allow using "unset" array index (-1) */
#define BDMF_ATTR_NO_RANGE_CHECK 0x00000080 /**< Disable index range check */


    /* attribute classification for display */
#define BDMF_ATTR_MAJOR         0x00000100  /**< Attribute level: Major */
#define BDMF_ATTR_MINOR         0x00000200  /**< Attribute level: Minor */
#define BDMF_ATTR_HIDDEN        0x00000400  /**< Attribute level: Hidden */
#define BDMF_ATTR_LEVEL_MASK    (BDMF_ATTR_MAJOR | BDMF_ATTR_MINOR | BDMF_ATTR_HIDDEN)
#define BDMF_ATTR_CONFIG        0x00001000  /**< Attribute class: Configuration */
#define BDMF_ATTR_STAT          0x00002000  /**< Attribute class: Statistics */
#define BDMF_ATTR_CLASS_MASK    (BDMF_ATTR_CONFIG | BDMF_ATTR_STAT)

    /* attribute IO format */
#define BDMF_ATTR_UNSIGNED      0x00004000  /**< Attribute is unsigned number */
#define BDMF_ATTR_HEX_FORMAT    0x00008000  /**< Attribute CLI IO should be in unsigned hex format */

    /* attribute access mask */
#define BDMF_ATTR_ACCESS_ADMIN  0x00010000  /**< Attribute access level: requires administrative rights */
#define BDMF_ATTR_ACCESS_DEBUG  0x00020000  /**< Attribute access level: requires debug access rights */

#define BDMF_ATTR_NOLOCK        0x00040000  /**< Do not use global lock when accessing attribute.
                                                 It can be needed if attribute can be accessed in interrupt context.
                                             */
#define BDMF_ATTR_NO_NULLCHECK  0x00080000  /**< Allow passing NULL as value */
#define BDMF_ATTR_NO_AUTO_GEN   0x00100000  /**< Ignore attribute when auto-generating access functions */
#define BDMF_ATTR_NO_VALUE      0x00200000  /**< Attribute is an action. Value doesn't matter.
                                                 Generate xx_<obj>_<attr>() instead of xx_<obj>_<attr>_set() */
#define BDMF_ATTR_DEPRECATED    0x00400000  /**< Don't show in examine operation */

    /* Internal fields. Do not set in the drivers */
#define BDMF_ATTR_AGGR_FIELD    0x01000000  /**< Attribute is a field of an aggregate */
#define BDMF_ATTR_UDEF_GET_NEXT 0x02000000  /**< User-defined get_next() */
#define BDMF_ATTR_UDEF_FIND     0x04000000  /**< User-defined find() */
#define BDMF_ATTR_UDEF_WRITE    0x08000000  /**< User-defined write() */
#define BDMF_ATTR_HAS_DISABLE   0x10000000  /**< Numeric attribute has "disable" value */


/** @} */

/** Attribute { name, value } pair. Used in helpers
 */
struct bdmf_attr_name_value
{
    char *name;               /**< Attribute name */
    char *array_index;        /**< Index in attribute array */
    char *value;              /**< Attribute value */
    struct bdmf_attr *attr;   /**< Attribute control block */
};

/** Declare BDMF type.
 * This macro replaces bdmf_attr_aggregate_type_register() call.
 * It can be used only in drivers included in the same module as the
 * framework.
 */
#define DECLARE_BDMF_AGGREGATE_TYPE(type) \
    const void * __BDMF_aggr_type_##type   \
        __attribute__((section("BDMF_aggr_init"),unused)) =  \
        &type

/** Declare BDMF type external.
 * Calling this macro should be followed by separate aggregate 
 * registration using bdmf_attr_aggregate_type_register() 
 * function. It can be called from any kernel object. 
 */
#define DECLARE_BDMF_AGGREGATE_TYPE_EXT(type) \
    const void * __BDMF_aggr_type_##type 

/** @} end of group bdmf_dev_attr */

/** \defgroup bdmf_dev_trace Event Trace
 * \ingroup bdmf_dev
 * Broadlight Device Management Framework Trace supports selective tracing based on
 * - trace level
 * - object type
 * @{
 */

/** Trace flags
 */
#define BDMF_TRACE_FLAG_HAS_TIMESTAMP 0x00000001 /**< If set - the consumer adds timestamp automatically */

/** @} end of bdmf_dev_trace group */

/** \defgroup bdmf_dev_type Object type registration
 * \ingroup bdmf_dev
 *
 * Each plug-in driver registers one or multiple Managed Object Types
 * using bdmf_type_register() function. Managed Object Type is akin to typedef
 * and specifies what to do at different stages of Managed Object lifetime.
 * Once Managed Object Type is registered, it can be instantiated using bdmf_new()
 * function creating Managed Objects.
 * @{
 */

/** Object list */
DLIST_HEAD(bdmf_obj_list, bdmf_object);

/** Link list */
DLIST_HEAD(bdmf_link_list, bdmf_link);

/** Reference list */
DLIST_HEAD(bdmf_ref_list, bdmf_ref);

/** Managed Object Type descriptor
 */
struct bdmf_type
{
    const char *name;        /**< Managed object type name */
    const char *description; /**< description (free text) */
    uint32_t extra_size;     /**< Extra memory to allocate along with BDMF object */
    uint32_t flags;          /**< a combination of bit-flags (\ref bdmf_dev_flags) */

    uint32_t max_objs;       /**< Max number of objects of this type. 0=no limit */
    const char *parent;      /**< Optional parent type */
    struct bdmf_attr *aattr; /**< Array of attribute descriptors terminated by \ref BDMF_ATTR_LAST macro */
    bdmf_trace_level_t trace_level; /**< Trace level */

    /* Memory segments */
    bdmf_mem_type_t seg_type[BDMF_MAX_MEM_SEGS];/**< memory space type */
    uint16_t seg_size[BDMF_MAX_MEM_SEGS];       /**< size of memory blocks */
    char seg_auto_alloc[BDMF_MAX_MEM_SEGS];     /**< Bool: 1=allocate automatically */

    void *drv_priv; /**< Field for driver private use */

    /** init driver structure. This callback is called from bdmf_type_register()
     * It is useful in drivers that declare type using BDMF_DECLARE_TYPE()
     * and need to perform driver-specific init at run time.
     * Returning error code <0 will fail the driver registration
     */
    int (*drv_init)(struct bdmf_type *drv);

    /** exit driver structure. This callback is called from bdmf_type_unregister()
     * It is useful in drivers that need to perform driver-specific cleanup
     */
    void (*drv_exit)(struct bdmf_type *drv);

    /** Sequence of calls when a new object is created
        - allocate and init generic fields
        , [pre_init]
        , [bdmf_configure]
        , [post_init]
    */
    /** called at object init time before initial attributes are set */
    int (*pre_init)(struct bdmf_object *mo);

    /** called at object init time after initial attributes are set */
    int (*post_init)(struct bdmf_object *mo);

    /** find object by discriminator string */
    int (*get)(struct bdmf_type *drv,
               struct bdmf_object *owner, const char *discr,
               struct bdmf_object **pmo);
    
    /** "get next" object iterator  */
    struct bdmf_object *(*get_next)(struct bdmf_type *drv,
                                    struct bdmf_object *mo, const char *discr);

    /** called when object is about to be destroyed */
    void (*pre_destroy)(struct bdmf_object *mo);

    /** called in the end of object destruction, after all its children have been killed */
    void (*destroy)(struct bdmf_object *mo);

    /** called when object's child is destroyed */
    void (*child_destroy)(struct bdmf_object *mo, struct bdmf_object *child);

    /** called when object referenced by *this* object is destroyed */
    void (*ref_destroy)(struct bdmf_object *mo, struct bdmf_object *ref);

    /** called when object referenced by *this* object is modified
     * has been modified
     */
    void (*ref_changed)(struct bdmf_object *mo, struct bdmf_object *ref_obj,
        struct bdmf_attr *ad, bdmf_index index, uint16_t attr_offset);

    /** help routine */
    const char *(*help)(struct bdmf_type *drv, const char *what);

    /** Called when THIS object is linked on top of other object */
    int (*link_down)(struct bdmf_object *_this, struct bdmf_object *mo_downstream,
        const char *link_attrs);

    /** Called when other object is linked on top of THIS object */
    int (*link_up)(struct bdmf_object *_this, struct bdmf_object *mo_upstream,
        const char *link_attrs);

    /** Called when object's downlink is disconnected */
    void (*unlink_down)(struct bdmf_object *_this, struct bdmf_object *mo_downstream);

    /** Called when object's uplink is disconnected */
    void (*unlink_up)(struct bdmf_object *_this, struct bdmf_object *mo_upstream);

    /* The following fields are for internal use and should not be touched
       by drivers
    */
    struct bdmf_type *po;               /* Parent type */
    int usecount;
    bdmf_fastlock lock;
    TAILQ_ENTRY(bdmf_type) types_list;
    struct bdmf_obj_list obj_list;
    uint32_t nobjs;
    uint32_t nattrs;
    void *frm_priv; /* Field for framework use. Don't touch */
    int magic;                      /* magic - for sanity checks */
};

/** \defgroup bdmf_dev_flags Object Type Flags
 * @{
 */
#define BDMF_DRV_FLAG_MUXUP    0x00000001 /**< Supports multiple links on the upper edge */
#define BDMF_DRV_FLAG_MUXDOWN  0x00000002 /**< Supports multiple links on the lower edge */
#define BDMF_DRV_FLAG_ROOT     0x00000004 /**< Root owner */
    /* access flags */
#define BDMF_DRV_ACCESS_ADMIN  0x00010000 /**< Access level: requires administrative rights to see the type */
#define BDMF_DRV_ACCESS_DEBUG  0x00020000 /**< Access level: requires debug access rights to see the type */

#define BDMF_DRV_NOLOCK        0x00040000  /**< Do not use global lock when working with object type */

#define BDMF_DRV_TMP           0x01000000  /**< Temporary type */

/** @} */

/** Declare BDMF type
 * This macro replaces bdmf_type_register() call.
 * It can be used only in drivers included in the same module as the
 * framework.
 */
#ifdef BDMF_DRIVER_GPL_LAYER
#define DECLARE_BDMF_TYPE(name,type) \
    bdmf_type_handle name##_drv(void)\
    { return &type; }\
    extern bdmf_type_handle (*f_##name##_drv)(void);\
    const void *__BDMF_type_##type __attribute__((section("BDMF_init"),unused)) = \
    &type
#define DECLARE_BDMF_TYPE_EXT(name,type) \
    bdmf_type_handle name##_drv(void)\
    { return &type; }\
    extern bdmf_type_handle (*f_##name##_drv)(void);
#else
#define DECLARE_BDMF_TYPE(name,type) \
    bdmf_type_handle name##_drv(void)\
    { return &type; }\
    EXPORT_SYMBOL(name##_drv); \
    const void *__BDMF_type_##type __attribute__((section("BDMF_init"),unused)) = \
    &type
#define DECLARE_BDMF_TYPE_EXT(name,type) \
    bdmf_type_handle name##_drv(void)\
    { return &type; }\
    EXPORT_SYMBOL(name##_drv); 
#endif

/** Dummy root type that owns BDMF type hierarchy
 */
extern struct bdmf_type *bdmf_root_type;

#define BDMF_ROOT_TYPE_NAME "root" /**< Dummy root type name */

/** @} end of bdmf_dev_type sub-group */


/** \defgroup bdmf_dev_obj Managed Objects control
 * \ingroup bdmf_dev
 *
 * As soon as driver registers Managed Object type, it becomes possible
 * to instantiate it creating Managed Objects.
 * The framework allocates Managed Object structure and passes this structure
 * to the driver as a parameter in all callback functions.\n
 * \n
 * Managed Object can reference other objects using bdmf_attr_objref attributes.
 * When referenced object is destroyed, framework automatically update all
 * objects & attributes that reference it.
 * \n
 * Managed objects can be linked together forming data paths.
 * @{
 */

/* BDMF object state
 */
typedef enum
{
    bdmf_state_init,
    bdmf_state_inactive,
    bdmf_state_active,
    bdmf_state_deleted
} bdmf_object_state;

#define BDMF_OBJ_NAME_LEN 32

/** BDMF object structure.
 * This structure is allocated by the framework when new object is created using bdmf_new()
 * function call. In addition to the object structure, framework can allocate an additional
 * area for driver's private use. This extra area size is specified in extra_size field in
 * #bdmf_type descriptor.
 */
struct bdmf_object
{
    /* the 1st 3 fields overlaps with struct bdmf_link and must not be touched */
    int magic;                      /* magic == BDMF_OBJECT_MAGIC */
    DLIST_ENTRY(bdmf_link) usl;     /* Entry in US object's DS link's list */
    DLIST_ENTRY(bdmf_link) dsl;     /* Entry in DS object's US link's list */

    char name[BDMF_OBJ_NAME_LEN];   /**< Object name. An ASCII string usually filled up by the driver */
    struct bdmf_type *drv;          /**< Driver object pointer */
    void *drv_priv;                 /**< Private pointer for driver use */
    void *app_priv;                 /**< Private pointer for application use */
    void *mem_seg_base[BDMF_MAX_MEM_SEGS];  /**< Memory segments - for driver use */
    struct bdmf_object *owner;      /**< Owner object */

    /* The following fields are for internal use and should not be touched
       by drivers and applications
    */
    DLIST_ENTRY(bdmf_object) osiblings; /* Objects with the same owner */
    DLIST_ENTRY(bdmf_object) siblings;  /* Objects of the same type */
    struct bdmf_ref_list clients;       /* List of objects referring to *this* object */
    struct bdmf_ref_list references;    /* List of references by *this* object */
    uint32_t num_ref_notify_change;     /* Number of references to be notified about *this* object being modified */
    struct bdmf_obj_list children;      /* Objects's children */
    struct bdmf_link_list us_links;     /* US links */
    struct bdmf_link_list ds_links;     /* DS links */
    bdmf_object_state state;        /* Object state */
    int usecount;                   /* Use count */
};

/** Root object - parent of the whole object hierarchy */
extern struct bdmf_object *bdmf_root_object;

/** BDMF link structure.
 * This structure is internal. It is allocated when mux is linked to mux.
 * Note that the 1st 3 fields are the same as for object.
 */
struct bdmf_link
{
    int magic;                      /* magic == BDMF_LINK_MAGIC */
    DLIST_ENTRY(bdmf_link) usl;     /* Entry in US object's DS link's list */
    DLIST_ENTRY(bdmf_link) dsl;     /* Entry in DS object's US link's list */

    struct bdmf_object *usmo;       /* US object */
    struct bdmf_object *dsmo;       /* DS object */
};

/* Object reference.
 * This structure is internal. Typically driver doesn't need to access it directly.
 */
struct bdmf_ref
{
    DLIST_ENTRY(bdmf_ref) ref_list;     /* entry in "references" -  list of objects referred by client object */
    DLIST_ENTRY(bdmf_ref) client_list;  /* entry in "clients" - list of objects referencing object */
    struct bdmf_object *client;         /* object referencing ref_obj. owns this struct bdmf_ref */
    struct bdmf_object *ref_obj;        /* object referenced by client */
    struct bdmf_attr *attr;             /* cleint's attribute handle if referenced by attribute */
    bdmf_index attr_index;              /* attribute index (if attr != NULL) */
    uint16_t attr_offset;               /* offset in aggregate attribute */
};

/** @} end of bdmf_dev_obj subgroup */

/*
 * APIs
 */


/*
 * Library init
 */

/** BDMF initial configuration
 */
struct bdmf_init_config
{
    bdmf_trace_level_t trace_level;
};

/** Initialize Broadlight Device Management Framefork
 *
 *  This function should be called once at init time.
 * \param[in]    init_config     Initial configuration
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_init(const struct bdmf_init_config *init_config);

/** Un-init  Broadlight Device Management Framefork.
 *
 * This function is typically called from driver unload
 */
void bdmf_exit(void);


/** \ingroup bdmf_memarea
 * @{
 */

/*
 * Memory area access
 */

/** Register memory area
 * \param[in]   area    Memory area descriptor
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_mem_area_register(struct bdmf_mem_area *area);


/** Unregister memory area
 * \param[in]   area    Memory area descriptor
 * \return
 *     0    - OK\n
 *    <0    - error
 */
void bdmf_mem_area_unregister(struct bdmf_mem_area *area);

/** Allocate memory of specific type
 * \param[in]   mo          Managed object that owns the allocation. NULL=root
 * \param[in]   mem_type    Memory type. Must be registered using bdmf_mem_area_register()
 * \param[in]   start       Optional start location. 0=use any
 * \param[in]   size        Memory block size
 * \param[in]   align       Memory block alignment
 * \return  pointer to the allocated memory block or NULL
 */
void *bdmf_mem_alloc_ext(struct bdmf_object *mo, bdmf_mem_type_t mem_type,
                unsigned long start, uint32_t size, uint32_t align);

/** Allocate memory of specific type
 * This is a shortcur calling bdmf_mem_alloc_ext() without "start" parameter
 * \param[in]   mo          Managed object that owns the allocation. NULL=root
 * \param[in]   mem_type    Memory type. Must be registered using bdmf_mem_area_register()
 * \param[in]   size        Memory block size
 * \param[in]   align       Memory block alignment
 * \return  pointer to the allocated memory block or NULL
 */
static inline void *bdmf_mem_alloc(struct bdmf_object *mo, bdmf_mem_type_t mem_type,
                uint32_t size, uint32_t align)
{
    return bdmf_mem_alloc_ext(mo, mem_type, 0, size, align);
}

/** Reserve memory of specific type at specific location
 * This is a shortcur calling bdmf_mem_alloc_ext() without "start" parameter
 * \param[in]   mo          Managed object that owns the allocation. NULL=root
 * \param[in]   mem_type    Memory type. Must be registered using bdmf_mem_area_register()
 * \param[in]   start       Memory block start address
 * \param[in]   size        Memory block size
 * \return  pointer to the reserved memory block or NULL
 */
static inline void *bdmf_mem_reserve(struct bdmf_object *mo, bdmf_mem_type_t mem_type,
                unsigned long start, uint32_t size)
{
    return bdmf_mem_alloc_ext(mo, mem_type, start, size, 0);
}

/** Release memory block allocated by bdmf_mem_alloc()
 * \param[in]   mem_type    Memory type. Must be registered using bdmf_mem_area_register()
 * \param[in]   ptr         Address of memory block to be released
 */
void bdmf_mem_free(bdmf_mem_type_t mem_type, void *ptr);

/** Copy from memory area to the destination in the host memory
 * \param[in]   mem_type    Memory type. Must be registered using bdmf_mem_area_register()
 * \param[in]   dst         Destination address in the host memory space
 * \param[in]   src         Source address in the memory area's memory space
 * \param[in]   size        Memory block size
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_mem_read(bdmf_mem_type_t mem_type, void *dst, const void *src, uint32_t size);


/** Copy from the host memory to memory area
 * \param[in]   mem_type    Memory type. Must be registered using bdmf_mem_area_register()
 * \param[in]   dst         Destination address in the memory area's memory space
 * \param[in]   src         Source address in the host memory space
 * \param[in]   size        Memory block size
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_mem_write(bdmf_mem_type_t mem_type, void *dst, const void *src, uint32_t size);

/** @} end of bdmf_memarea subgroup */


/** \ingroup bdmf_dev_type
 * @{
 */

/*
 * Managed object type registration
 */

/** Register Managed Object Type
 * The function is typically called at driver init time.
 * The driver structure passed in function parameter is NOT re-allocated
 * by the framework. Driver is expected to pass pointer to static or dynamically-allocated
 * structure that continues to exist after bdmf_type_register call.
 * 
 * \param[in]   drv     Type descriptor
 * \return
 *     0    - OK\n
 *    <0    - error
 */ 
int bdmf_type_register(struct bdmf_type *drv);


/** Register Managed Object Type - extended
 * The function is typically called when local BDMF is connected to remote BDMF.
 * The driver structure passed in function parameter is NOT re-allocated
 * by the framework. Driver is expected to pass pointer to static or dynamically-allocated
 * structure that continues to exist after bdmf_type_register call.
 *
 * \param[in]   root    root type in the type hierarchy
 * \param[in]   drv     new type descriptor
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_type_register_ext(struct bdmf_type *root, struct bdmf_type *drv);


/** Unregister Managed Object TYpe
 * The function is typically called at driver unload time.
 * All objects of the type being un-registered are destroyed.
 *
 * \param[in]   drv     Type descriptor that was previously passed to bdmf_type_register() function.
 */
void bdmf_type_unregister(struct bdmf_type *drv);

/** @} end of bdmf_dev_type subgroup */

/** \ingroup bdmf_dev_obj
 * @{
 */

/** Allocate and initialise object structure
 * \param[in]   drv         Management object type
 * \return  new object pointer
 */
struct bdmf_object *bdmf_object_alloc(struct bdmf_type *drv);

/** Get driver's private memory block allocated along with the Managed Object.
 * Framework can allocate an extra memory for driver's use when creting a new object.
 * The area size is specified in extra_size field at object type registration time.
 * bdmf_obj_data() function is guatanteed to return non-NULL pointer if drv->extra_size > 0
 * was specified at object type registration time.
 *
 * \param[in]   mo  Managed object pointer
 * \return      pointer to driver's private memory block associated with the object
 */
static inline void *bdmf_obj_data(struct bdmf_object *mo)
{
    if (!mo->drv->extra_size)
        return NULL;
    return (void *)((unsigned long)mo - mo->drv->extra_size);
}

/** Set/modify object parent (owner)
 * \param[in]   mo      Managed object pointer
 * \param[in]   owner   New owner handle
 */
void bdmf_object_parent_set(struct bdmf_object *mo, struct bdmf_object *parent);


/** Convert attribute handle to id
 * \param[in]   drv     Driver
 * \param[in]   attr    Attribute handle
 * \return attribute id
 */
static inline bdmf_attr_id bdmf_attr_to_aid(struct bdmf_type *drv, struct bdmf_attr *attr)
{
    return (bdmf_attr_id)(attr - drv->aattr);
}

/** Convert attribute id to handle
 * \param[in]   drv     Driver
 * \param[in]   aid     Attribute id
 * \return attribute handle
 */
static inline struct bdmf_attr *bdmf_aid_to_attr(struct bdmf_type *drv, bdmf_attr_id aid)
{
    return &drv->aattr[aid];
}

/** Inform framework that object is being referenced.
 * \param[in]   mo      Managed object
 * \param[in]   ref_obj Managed object being referenced by fo
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_reference_add(struct bdmf_object *mo, struct bdmf_object *ref_obj);


/** Inform framework that object is no longer being referenced.
 * \param[in]   mo      Managed object
 * \param[in]   ref_obj Managed object being referenced by mo
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_reference_free(struct bdmf_object *mo, struct bdmf_object *ref_obj);

/** @} end of bdmf_dev_obj subgroup */

/** \ingroup bdmf_dev_attr
 * @{
 */

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
int bdmf_attr_aggregate_type_register(struct bdmf_aggr_type *aggr_type);


/** Unregister aggregate attribute type.
 * The function unregisters aggregate type registered by bdmf_attr_aggregate_type_register().
 * Following unregistration, all attributes of this aggregate type become inaccessible.
 * \param[in]   aggr_type    aggregate type structure
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_attr_aggregate_type_unregister(struct bdmf_aggr_type *aggr_type);


/** Find aggregate type by name
 * \param[in]   name          aggregate type name
 * \param[out]  paggr_type    aggregate type structure
 * The function increments usecount of the returned aggregate type
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_attr_aggregate_type_find(const char *name, struct bdmf_aggr_type **paggr_type);


/** Decrement aggregate type's usecount
 * \param[in]   aggr_type    aggregate type structure
 */
void bdmf_attr_aggr_type_put(struct bdmf_aggr_type *aggr_type);


/** Aggregate type iterator
 * \param[in]   prev          Previous type. NULL=get first
 * The function increments usecount of the returned aggregate type
 * and decrements usecount of the previous aggregate type
 * \return
 *    next type pointer or NULL
 */
const struct bdmf_aggr_type *bdmf_attr_aggregate_type_get_next(
                        const struct bdmf_aggr_type *prev);


/** Find attribute in string spec and get its value
 *
 * The function parses attribute spec in format of comma-delimited
 * list of name=value pairs, look for attribute with the specified name
 * and copies its value to the provided buffer.
 *
 * \param[in]   attr  string containing comma-delimited list of name=value pairs
 * \param[in]   name  name of attribute to be found
 * \param[out]  val   buffer to copy value to
 * \param[in]   size  val buffer size
 * \return
 *     0    - OK\n
 *    <0    - error in parameters,name is not found
 */
int bdmf_attr_get_hlp(const char *attr, const char *name,
                      char *val, uint32_t size);


/** Find numeric attribute in string spec and return its value.
 *
 * The function is similar to bdmf_attr_get_hlp() and goes a step further.
 * After locating the attribute and its value it attempts to convert the value
 * into numeric format.
 * 
 * \param[in]   attr  string containing comma-delimited list of name=value pairs
 * \param[in]   name  name of attribute to be found
 * \param[out]  pval  attribute value
 * 
 * \return
 *     0    - OK\n
 *    <0    - error in parameters,name is not found or value is not number
 */
int bdmf_attr_get_num_hlp(const char *attr, const char *name, bdmf_enum *pval);


/** Get enum string given its value
 *
 * The function maps enum value to its string representation
 * \param[in]   table enum values table
 * \param[in]   val   enum value
 * \return enum string value or NULL if not found
 */
const char *bdmf_attr_get_enum_text_hlp(const bdmf_attr_enum_table_t *table, bdmf_enum val);


/** Get enum value given the string value
 *
 * The function maps enum value to its string representation
 * \param[in]   table enum values table
 * \param[in]   text  enum text value
 * \param[out]  pval  enum value
 * \return 0 if OK, BDMF_ERR_PARM if not found
 */
int bdmf_attr_get_enum_val_hlp(const bdmf_attr_enum_table_t *table, const char *text, bdmf_enum *pval);


/** Find enum attribute in string spec and return its value.
 *
 * The function is similar to bdmf_attr_get_hlp() and goes a step further.
 * After locating the attribute and its value it attempts to convert the
 * external value into internal format using the provided enum values table.
 *
 * \param[in]   attr  string containing comma-delimited list of name=value pairs
 * \param[in]   name  name of attribute to be found
 * \param[in]   table enum values table
 * \param[out]  pval  attribute value
 *
 * \return
 *     0    - OK\n
 *    <0    - error in parameters,name is not found or value is invalid
 */
int bdmf_attr_get_enum_hlp(const char *attr, const char *name,
                           const bdmf_attr_enum_table_t *table, bdmf_enum *pval);


/** Split attribute string into { name, value } pairs
 *
 * This function is a helper that can be used by drivers.
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
int bdmf_attr_string_split( const char *attr,
                           struct bdmf_attr_name_value **p_pairs, int *p_npairs);

/** Is attribute type numeric
 * \param[in]   attr_type
 * \return 1 if type is numeric
 */
static inline int bdmf_attr_type_is_numeric(bdmf_attr_type_t attr_type)
{
    if (attr_type == bdmf_attr_number       ||
        attr_type == bdmf_attr_enum         ||
        attr_type == bdmf_attr_dyn_enum     ||
        attr_type == bdmf_attr_ipv4_addr    ||
        attr_type == bdmf_attr_pointer      ||
        attr_type == bdmf_attr_object)
    {
        return 1;
    }
    return 0;
}

/** Convert array index from string to internal format
 *
 * This function is a helper that can be used by drivers.
 * \param[in]   mo          object handle
 * \param[in]   attr        attribute handle
 * \param[in]   sindex      Array index in string format
 * \param[out]  index_buf   Array index stored in memory buffer of at least attr->index_size size
 *
 * \return
 *     0    - OK\n
 *    <0    - error in parameters, parsing error or no memory
 */
int bdmf_attr_string_to_array_index(struct bdmf_object *mo, struct bdmf_attr *attr, const char *sindex, void *index_buf);

/** Convert array index from internal to string format
 *
 * This function is a helper that can be used by drivers.
 * \param[in]   mo          object handle
 * \param[in]   attr        attribute handle
 * \param[in]   index       Array index passed by value for numeric indexes or by address
 *                          for non-numeric indexes
 * \param[out]  sindex      Array index in string format
 * \param[in]   size        sindex buffer size
 *
 * \return
 *     0    - OK\n
 *    <0    - error in parameters, parsing error or no memory
 */
int bdmf_attr_array_index_to_string(struct bdmf_object *mo, struct bdmf_attr *attr, bdmf_index index, char *sindex, uint32_t size);


/** Release {name, value} pairs array allocated by bdmf_attr_string_split()
 *
 * \param[in]   pairs   pairs array pointer returned by bdmf_attr_string_split()
 */
void bdmf_attr_pairs_release(struct bdmf_attr_name_value *pairs);

/** val_to_s callback target for object reference conversion
 * The function can be used when object reference attribute is declared as
 * bdmf_attr_pointer instead of bdmf_attr_object.
 * It might be useful if built-in reference management mechanism should be
 * disabled.
 * \param[in]   mo          object handle
 * \param[in]   attr        attribute handle
 * \param[in]   val         Object pointer or NULL
 * \param[out]  sbuf        String buffer
 * \param[in]   size        sbuf buffer size
 *
 * \return
 *     0    - OK\n
 *    <0    - error in parameters, parsing error or no memory
 */
int bdmf_attr_val_to_s_object(struct bdmf_object *mo, struct bdmf_attr *attr, const void *val, char *sbuf,
    uint32_t size);

/** s_to_val callback target for object reference conversion
 * The function can be used when object reference attribute is declared as
 * bdmf_attr_pointer instead of bdmf_attr_object.
 * It might be useful if built-in reference management mechanism should be
 * disabled.
 * \param[in]   mo          object handle
 * \param[in]   attr        attribute handle
 * \param[in]   sbuf        String buffer: object name or "null"
 * \param[out]  val         Object pointer or NULL
 * \param[in]   size        val size
 *
 * \return
 *     0    - OK\n
 *    <0    - error in parameters, parsing error or no memory
 */
int bdmf_attr_s_to_val_object(struct bdmf_object *mo, struct bdmf_attr *attr, const char *sbuf, void *val,
    uint32_t size);

/** Format attribute help string.
 *
 * The function formats attribute help string based on the
 * attribute description.
 *
 * \param[in]   attr    attribute descriptor
 * \param[out]  buffer  buffer where help string is returned
 * \param[out]  size    buffer size
 */
void bdmf_attr_help(const struct bdmf_attr *attr, char *buffer, uint32_t size);


/** Format attribute help string in compact form.
 *
 * The function formats attribute help string based on the
 * attribute description.
 *
 * \param[in]   attr    attribute descriptor
 * \param[out]  buffer  buffer where help string is returned
 * \param[out]  size    buffer size
 */
void bdmf_attr_help_compact(const struct bdmf_attr *attr, char *buffer, uint32_t size);


/** Convert hex-string to binary data
 *
 * The hex string must contain an even number of hexadecimal characters.
 *
 * \param[in]   src     0-terminated hex string
 * \param[out]  dst     buffer for binary output
 * \param[in]   dst_len dst buffer size
 *
 * \return   Returns number of data bytes if conversion succeded\n
 *  or error code < 0 if convertion failed.
 */
int bdmf_strhex(const char *src, uint8_t *dst, uint16_t dst_len);

/** @} end of bdmf_dev_attr subgroup */

/** Complete attribute struct population
 *
 * When a driver registers an attributes structure, it does not supply all
 * the members of bdmf_attr struct.
 *
 * \param[in]   drv     Driver
 * \param[out]  attr    Attribute handle
 *
 * \return   Returns 0-ok, <=error\n
 */
int bdmf_attr_make(struct bdmf_type *drv, struct bdmf_attr *attr);

void bdmf_attr_unmake(struct bdmf_attr *attr);

int _bdmf_attrelem_get_as_string(bdmf_object_handle mo,
                                struct bdmf_attr *attr, bdmf_index index,
                                char *buffer, uint32_t size);

int _bdmf_ref_attr_clear(struct bdmf_ref *ref);
struct bdmf_ref *_bdmf_ref_alloc(struct bdmf_object *mo, struct bdmf_object *ref_obj, struct bdmf_attr *attr);
void _bdmf_ref_free(struct bdmf_ref *ref);
void _bdmf_ref_notify_change(struct bdmf_object *mo);
struct bdmf_ref *_bdmf_ref_find_by_attr(struct bdmf_object *mo, struct bdmf_attr *attr, bdmf_index index, int offset);
int _bdmf_attr_string_split(char *attr, struct bdmf_attr_name_value **p_pairs,
    int *p_npairs, char **parser_stop_pos);
struct bdmf_attr *_bdmf_attr_by_name(struct bdmf_attr *aattr, const char *name);


/* Module initialization */
extern int bdmf_type_module_init(void);
extern void bdmf_type_module_exit(void);
extern int bdmf_area_module_init(void);
extern void bdmf_area_module_exit(void);
/** \defgroup bdmf_dev_history History logging
 * \ingroup bdmf_dev
 * History logging feature allows
 * - logging of configuration history in a file
 * - playing back saved history file
 * @{
 */

#ifdef BDMF_HISTORY

/** Init history module
 * \return 0=OK or error code <0
 */
int bdmf_history_module_init(void);

/** Exit history module
 */
void bdmf_history_module_exit(void);

/** Init and start history recording
 * \param[in]   size        History buffer size
 * \param[in]   record_on   Initial recording state (true=enabled)
 * \return 0=OK or error code <0
 */
int bdmf_history_init(uint32_t size, bdmf_boolean record_on);

/** Stop history recording
 */
void bdmf_history_stop(void);

/** Resume history recording
 * \return 0=OK or error code <0
 */
int bdmf_history_resume(void);

/** Get history buffer
 * \param[out]  buffer      History buffer pointer
 * \param[out]  size        History buffer size
 * \param[out]  rec_size    Recorded history size
 * \return 0=OK or error code <0
 */
int bdmf_history_get(void **buffer, uint32_t *size, uint32_t *rec_size);

/** Reset history buffer.
 * All recorded history is discarded
 * \return 0=OK or error code <0
 */
void bdmf_history_reset(void);

/** Release history buffer.
 * All recorded history is discarded
 * \return 0=OK or error code <0
 */
void bdmf_history_free(void);

/** Save history buffer
 * \param[in]   fname   History file name
 * \return 0=OK or error code <0
 */
int bdmf_history_save(const char *fname);

/** Play-back history file.
 * \param[in]   session             CLI session. Can be NULL
 * \param[in]   fname               History file name
 * \param[in]   stop_on_mismatch    true=stop on saved / new result mismatch
 * \return 0=OK or error code < 0.\n
 * error indicates problem with reading or parsing history file
 */
int bdmf_history_play(bdmf_session_handle session, const char *fname, bdmf_boolean stop_on_mismatch);

/** Event recording point */
typedef enum
{
    bdmf_hist_point_none  = 0,      /**< Middle point */
    bdmf_hist_point_start = 0x1,    /**< Start point (beginning of event recording) */
    bdmf_hist_point_end   = 0x2,    /**< End of event recording */
    bdmf_hist_point_both  = 0x3,    /**< Both start and end */
} bdmf_history_point_t;

/* Record custom event
 *
 * History event is represented by a string terminated by "\n" \n
 * event_name event_parameters_separated_by_a_single_space # rc optional_results\\n \n
 * API itself takes care of recording the event name. format and the following arguments
 * should deal only with event parameters, return code and output.
 * The function can be called multiple times when recording the same event.
 * When it is called for the 1st time, point parameter must have \ref bdmf_hist_point_start bit set.
 * When it is called for the last time, point parameter must have \ref bdmf_hist_point_end bit set.
 *
 * \param[in]   event   Event name - must match event in bdmf_history_event_register()
 * \param[in]   point   Recording point: start of event, end of event or both
 * \param[in]   format  printf-like format
 */
void bdmf_history_event(const char *event, bdmf_history_point_t point, const char *format, ...)
    __attribute__((format(printf, 3, 4)));

#define BDMF_HISTORY_EVENT(ev, point, format, args...) bdmf_history_event(ev, point, format, ## args)

/*
 * The following types and functions are internal
 */

/* Built-in event */
typedef enum
{
    bdmf_hist_ev_none,

    bdmf_hist_ev_new_and_configure, /* bdmf_new_and_configure() */
    bdmf_hist_ev_new_and_set,       /* bdmf_new_and_set() */
    bdmf_hist_ev_destroy,           /* bdmf_destroy() */
    bdmf_hist_ev_link,              /* bdmf_link() */
    bdmf_hist_ev_unlink,            /* bdmf_unlink() */
    bdmf_hist_ev_configure,         /* bdmf_configure() */
    bdmf_hist_ev_mattr_set,         /* bdmf_mattr_set() */
    bdmf_hist_ev_set_as_num,        /* bdmf_attrelem_set_as_num() */
    bdmf_hist_ev_set_as_string,     /* bdmf_attrelem_set_as_string() */
    bdmf_hist_ev_set_as_buf,        /* bdmf_attrelem_set_as_buf() */
    bdmf_hist_ev_add_as_num,        /* bdmf_attrelem_add_as_num() */
    bdmf_hist_ev_add_as_string,     /* bdmf_attrelem_add_as_string() */
    bdmf_hist_ev_add_as_buf,        /* bdmf_attrelem_add_as_buf() */
    bdmf_hist_ev_delete,            /* bdmf_attrelem_delete() */

    bdmf_hist_ev__num_of            /* Number of event types */
} bdmf_history_event_t;

/* Record built-in history event.
 */
void bdmf_history_bi_event(bdmf_history_event_t ev, bdmf_history_point_t point, ...);

#define BDMF_HISTORY_BI_EVENT(ev, point, args...) bdmf_history_bi_event(ev, point, ## args)

#else

static inline int bdmf_history_init(uint32_t size, bdmf_boolean record_on)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

static inline void bdmf_history_stop(void)
{
}

static inline int bdmf_history_resume(void)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

static inline int bdmf_history_get(void **buffer, uint32_t *size, uint32_t *rec_size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

static inline void bdmf_history_reset(void)
{
}

static inline void bdmf_history_free(void)
{
}

static inline int bdmf_history_save(const char *fname)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

static inline int bdmf_history_play(bdmf_session_handle session, const char *fname, bdmf_boolean stop_on_mismatch)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

#define BDMF_HISTORY_EVENT(ev, point, format, args...)
#define BDMF_HISTORY_BI_EVENT(ev, point, args...)

#endif /* #ifdef BDMF_HISTORY */

/** @} end of bdmf_dev_history group */

/** \ingroup bdmf_dev_trace
 * @{
 */

/** Initialise tracer.
 * \return
 *     0    - OK\n
 */
int bdmf_trace_init(void);


/* Add trace session.
 * Each trace entry is "printed" to all configured sessions.
 * \param[in]   session     Trace output session
 * \param[in]   flags       Additional output channel parameters. A combination
 *                          of BDMF_TRACE_FLAG_.. constants
 * \return
 *     0    - OK\n
 *     <0   - error code
 */
int bdmf_trace_session_add(bdmf_session_handle session, uint32_t flags);


/* Delete trace session.
 * The function deletes output channel created by bdmf_trace_session_add()
 * \param[in]   session     Trace output session
 * \return:\n
 *     0    - OK\n
 *     <0   - error code
 */
int bdmf_trace_session_delete(bdmf_session_handle session);


/* Get the current trace level
 * \param[in]   drv     Object type or NULL for global level.
 * \return:\n
 *  >=0     - trace level\n
 *  <0      - error code
 */
bdmf_trace_level_t bdmf_trace_level(bdmf_type_handle drv);


/* Set trace level
 * \param[in]   drv         Object type handle or NULL for global level.
 *                          Global level is applied as a default to all existing and future
 *                          object types.
 * \param[in]   level       New trace level
 * \return: old trace level
 */
bdmf_trace_level_t bdmf_trace_level_set(bdmf_type_handle drv, bdmf_trace_level_t level);


/* Helper functions */


/** Print error trace conditional on object type's trace level
 * \param[in]   drv         Object type
 * \param[in]   fmt         printf-like format
 * \param[in]   args        0 or more parameters
 */
#define BDMF_TRACE_ERR_DRV(drv, fmt, args...)                       \
    do {                                                            \
        if (drv)                                                    \
        {                                                           \
            if (drv->trace_level >= bdmf_trace_level_error)         \
                bdmf_trace("ERR: %s#%d: %s: " fmt, __FUNCTION__, __LINE__, drv->name, ## args);\
        }                                                           \
        else                                                        \
            BDMF_TRACE_ERR(fmt, ## args);                           \
    } while(0)


/** Print object error trace conditional on object type's trace level
 * \param[in]   mo          Object
 * \param[in]   fmt         printf-like format
 * \param[in]   args        0 or more parameters
 */
#define BDMF_TRACE_ERR_OBJ(obj, fmt, args...)                       \
    do {                                                            \
        if (obj)                                                    \
        {                                                           \
            if (obj->drv->trace_level >= bdmf_trace_level_error)         \
                bdmf_trace("ERR: %s#%d: %s: " fmt, __FUNCTION__, __LINE__, obj->name, ## args);\
        }                                                           \
        else                                                        \
            BDMF_TRACE_ERR(fmt, ## args);                           \
    } while(0)


/** Print info trace conditional on object type's trace level
 * \param[in]   drv         Object type
 * \param[in]   fmt         printf-like format
 * \param[in]   args        0 or more parameters
 */
#define BDMF_TRACE_INFO_DRV(drv, fmt, args...)             \
    do {                                                            \
        if (drv && drv->trace_level >= bdmf_trace_level_info)       \
            bdmf_trace("INF: %s#%d: %s: " fmt, __FUNCTION__, __LINE__, drv->name, ## args);\
    } while(0)


/** Print info trace conditional on object type's trace level
 * \param[in]   obj         Object
 * \param[in]   fmt         printf-like format
 * \param[in]   args        0 or more parameters
 */
#define BDMF_TRACE_INFO_OBJ(obj, fmt, args...)             \
    do {                                                            \
        if (obj && obj->drv->trace_level >= bdmf_trace_level_info)       \
            bdmf_trace("INF: %s#%d: %s: " fmt, __FUNCTION__, __LINE__, obj->name, ## args);\
    } while(0)


/** Print info or error trace conditional on object type's trace level and return
 * \param[in]   rc          return code. 0=info trace, !=0-error trace
 * \param[in]   drv         Object type
 * \param[in]   fmt         printf-like format
 * \param[in]   args        0 or more parameters
 */
#define BDMF_TRACE_RET_DRV(rc, drv, fmt, args...)          \
    do {                                                            \
        if (rc)\
            BDMF_TRACE_ERR_DRV(drv, "status:%s. " fmt, bdmf_strerror(rc), ## args);\
        else \
            BDMF_TRACE_INFO_DRV(drv, "success. " fmt, ## args);\
        return (rc);\
    } while(0)


/** Print info or error trace conditional on object type's trace level and return
 * \param[in]   rc          return code. 0=info trace, !=0-error trace
 * \param[in]   obj         Object
 * \param[in]   fmt         printf-like format
 * \param[in]   args        0 or more parameters
 */
#define BDMF_TRACE_RET_OBJ(rc, obj, fmt, args...)          \
    do {                                                            \
        if (rc)\
            BDMF_TRACE_ERR_OBJ(obj, "status:%s. " fmt, bdmf_strerror(rc), ## args);\
        else \
            BDMF_TRACE_INFO_OBJ(obj, "success. " fmt, ## args);\
        return (rc);\
    } while(0)


#ifdef BDMF_DEBUG


/** Print debug trace conditional on object type's trace level
 * \param[in]   drv         Object type or NULL for global level.
 * \param[in]   fmt         printf-like format
 * \param[in]   args        0 or more parameters
 */
#define BDMF_TRACE_DBG_DRV(drv, fmt, args...)              \
    do {                                                            \
        if (drv && drv->trace_level >= bdmf_trace_level_debug)      \
            bdmf_trace("DBG: %s#%d: %s: " fmt, __FUNCTION__, __LINE__, drv->name, ## args);\
    } while(0)


/** Print debug trace conditional on object type's trace level
 * \param[in]   obj         Object or NULL for global level.
 * \param[in]   fmt         printf-like format
 * \param[in]   args        0 or more parameters
 */
#define BDMF_TRACE_DBG_OBJ(obj, fmt, args...)              \
    do {                                                            \
        if (obj && obj->drv->trace_level >= bdmf_trace_level_debug)      \
            bdmf_trace("DBG: %s#%d: %s: " fmt, __FUNCTION__, __LINE__, obj->name, ## args);\
    } while(0)

#else /* #ifdef BDMF_DEBUG */

#define BDMF_TRACE_DBG_DRV(drv, fmt, args...)
#define BDMF_TRACE_DBG_OBJ(drv, fmt, args...)

#endif /* #ifdef BDMF_DEBUG */

#define NOBDMF_TRACE_DBG(fmt, args...)
#define NOBDMF_TRACE_DBG_DRV(drv, fmt, args...)

/** @} end of bdmf_dev_trace group */


/* Register all aggregate types declared types using DECLARE_BDMF_AGGREGATE_TYPE */
int bdmf_register_aggregate_types(const unsigned long *section_start, const unsigned long *section_end);

/* Register all plugins that declared types using DECLARE_BDMF_TYPE */
int bdmf_register_plugins(const unsigned long *section_start, const unsigned long *section_end);

/* Unregister all aggregate types declared types using DECLARE_BDMF_AGGREGATE_TYPE */
int bdmf_unregister_aggregate_types(const unsigned long *section_start, const unsigned long *section_end);

/* Unregister all plugins that declared types using DECLARE_BDMF_TYPE */
int bdmf_unregister_plugins(const unsigned long *section_start, const unsigned long *section_end);

/** @} end of bdmf_dev group */

#endif /* _BDMF_DEV_H_ */
