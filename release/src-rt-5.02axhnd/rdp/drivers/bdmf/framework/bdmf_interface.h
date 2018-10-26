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
 * bdmf_interface.h
 *
 * Broadlight Device Management Framework - interface
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

#ifndef _BDMF_INTERFACE_H_
#define _BDMF_INTERFACE_H_

#define BDMF_MAX_MEM_SEGS 4

/** \defgroup bdmf Broadlight Device Management Framework
 * BDMF is a framework that Broadlight drivers use in order to access
 * generic functionality. The same framework is used by applications that need
 * access to functionality provided by the drivers. \n
 * BDMF has the following properties:\n
 * - it supports an arbitrary number of plug-in drivers that are attached to the framework at run time
 * - each plug-in driver registers one or more "Managed Object Type"s #bdmf_type
 * - Managed Object Type includes the following information
 *      - optional callback functions to be called at different stages in management object lifetime
 *      - a set of attributes - configuration parameters
 * - Following "Managed Object Type" registration, managed objects of the given type can be instantiated
 * and configured by setting their attributes
 * - Managed Objects can form parent-child hierarchies (although, there is no type inheritance)
 * - Managed Objects can be linked together forming Data Paths\n
 * \n
 * bdmf_interface.h header file contains types and functions that can be used by both
 * bdmf plug-in drivers and applications.
 * @{
 */
#if (defined(__KERNEL__) || defined(BDMF_SYSTEM_SIM) || defined(RDP_SIM))
#include <bdmf_system.h>
#endif
#include <bdmf_data_types.h>

#ifndef MAX
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif
#ifndef MIN
#define MIN(a, b) ((a) >= (b) ? (b) : (a))
#endif
#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))
#endif

#if (defined(__KERNEL__) || defined(BDMF_SYSTEM_SIM) || defined(RDP_SIM))
/** Managed object type handle */
typedef struct bdmf_type *bdmf_type_handle;

/** Managed object */
typedef struct bdmf_object *bdmf_object_handle;

/** Inter-object link */
typedef struct bdmf_link *bdmf_link_handle;

/** Handle that can represent different things */
typedef void *bdmf_handle;
#else /* userspace */
#if defined KERNEL_64
typedef uint64_t bdmf_type_handle;
typedef uint64_t bdmf_object_handle;
typedef uint64_t bdmf_link_handle;
typedef uint64_t bdmf_handle;
#else
typedef uint32_t bdmf_type_handle;
typedef uint32_t bdmf_object_handle;
typedef uint32_t bdmf_link_handle;
typedef uint32_t bdmf_handle;
#endif /* KERNEL_64 */
#endif /* __KERNEL__ */

/** Multi-attribute transaction handle link */
typedef bdmf_object_handle bdmf_mattr_handle;

/** Object's attribute handle */
typedef uint16_t bdmf_attr_id;

#define BDMF_MAX_INFO_NAME_LENGTH   32
#define BDMF_MAX_INFO_HELP_LENGTH   128

#define BDMF_TYPE_MAGIC     (('b'<<24) | ('d'<<16) | ('m'<<8) | 't')
#define BDMF_OBJECT_MAGIC   (('b'<<24) | ('d'<<16) | ('m'<<8) | 'o')
#define BDMF_MATTR_MAGIC    (('b'<<24) | ('d'<<16) | ('m'<<8) | 'a')
#define BDMF_LINK_MAGIC     (('b'<<24) | ('d'<<16) | ('m'<<8) | 'l')
#define BDMF_INVALID_MAGIC  (('~'<<24) | ('d'<<16) | ('m'<<8) | 'f')

/** \defgroup bdmf_type Managed object type control APIs
 *  \ingroup bdmf
 *
 * APIs in this group allow to locate and examine the registered Managed Object Types.
 * @{
 */

/** Object type information */
typedef struct bdmf_type_info
{
    char name[BDMF_MAX_INFO_NAME_LENGTH];       /**< Object type name */
    char help[BDMF_MAX_INFO_HELP_LENGTH];       /**< Object type description */
    int n_objects;                              /**< Number of managed objects */
    int n_attrs;                                /**< Number of attributes */
} bdmf_type_info_t;


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
bdmf_type_handle bdmf_type_get_next(bdmf_type_handle drv);


/** Get managed object type handle given its name
 * 
 * The handle must be released using bdmf_put() function.
 * 
 * \param[in]   name    Managed object type name. The name can be followed by "@subsystem" to identify
 *                      type located at specific subsystem
 * \param[out]  pdrv    type handle or NULL if object type with the requested name is not found.\n
 *                      It is guaranteed that *drv contains valid handle if bdmf_type_find_get function returns 0.
 * \return
 *     0          - OK\n
 *    <0          - error\n
 */
int bdmf_type_find_get(const char *name, bdmf_type_handle *pdrv);


/** Lock managed object type handle
 *
 * The handle must be released using bdmf_put() function.
 *
 * \param[in]   drv     Managed object type handle
 */
void bdmf_type_get(bdmf_type_handle drv);


/** Release managed object type handle.
 * The function "unlocks" the handle by decrementing usecount in the
 * underlying structure. Following this call the handle may become invalid.
 * 
 * \param[in]   drv     Managed object type handle
 * \return
 *     void
 */
void bdmf_type_put(bdmf_type_handle drv);


/** Get managed object type info
 * 
 * \param[in]   drv     Managed object type handle
 * \param[out]  info    Managed object type info
 * \return
 *     0          - OK\n
 *    <0          - error\n
 */
int bdmf_type_info(bdmf_type_handle drv, bdmf_type_info_t *info);


/** Get number of attributes supported by managed object type
 *
 * \param[in]   drv     Managed object type handle
 * \return number of attributes
 */
int bdmf_type_num_attrs(bdmf_type_handle drv);

/** @} end of bdmf_type sub-group */

/** Attribute set
 * \ingroup bdmf_mattr
 */
typedef struct bdmf_mattr bdmf_mattr_t;

/** \defgroup bdmf_object Managed object control APIs
 *  \ingroup bdmf
 *
 * APIs in this group allow to create, destroy, locate and examine Managed Objects.
 * @{
 */

/** Create a new managed object using parameters in attribute descriptor string
 *
 * \param[in]   drv     Managed object type handle
 * \param[in]   owner   Object owner. If NULL, framework tries to identify owner using attributes
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
             bdmf_object_handle *pmo);

/** Create a new managed object using parameters in attribute set
 *
 * \param[in]   drv     Managed object type handle
 * \param[in]   owner   Object owner. If NULL, framework tries to identify owner using attributes
 * \param[in]   mattr   attribute set in the same format as in bdmf_mattr_set().\n
 *                      mattr is freed automatically.
 * \param[out]  pmo     New managed object handle
 * \return
 *     0      - OK \n
 *    <0      - error
 */
int bdmf_new_and_set(bdmf_type_handle drv,
             bdmf_object_handle owner, bdmf_mattr_handle mattr,
             bdmf_object_handle *pmo);


/** Destroy managed object
 *
 * \param[out]  mo      Managed object handle
 * \return
 *     0      - OK \n
 *    <0      - error
 */
int bdmf_destroy(bdmf_object_handle mo);


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
                  bdmf_object_handle *pmo);


/** Find managed object
 *
 * When no longer needed, the managed object handle must be released by bdmf_put() function.
 *
 * \param[in]   drv     Managed object type handle
 * \param[in]   owner   Object's owner (parent). If not set, the framework tries to identify
 *                      the parent using attributes in the discr string.
 * \param[in]   mattr   Set of attributes with values in the same format as in bdmf_mattr_set()\n
 *                      attributes in the set must be sufficient to uniquely identify the object.
 * \param[out]  pmo     Managed object handle
 * \return
 *     0      - OK \n
 *    <0      - error
 */
int bdmf_find_get_by_set(bdmf_type_handle drv,
                  bdmf_object_handle owner, const bdmf_mattr_t *mattr,
                  bdmf_object_handle *pmo);


/** Lock managed object, by increasing its use count.
 *
 * An object cannot be destroyed unless its use count is zero.
 * When no longer needed, the managed object handle must be released by bdmf_put() function.
 *
 * \param[in]   mo      Managed object handle
 * \return
 *     void
 */
void bdmf_get(bdmf_object_handle mo);


/** Find managed object given its type name and attributes.
 *
 * The function combines bdmf_type_find_get() and bdmf_find_get() .
 * When no longer needed, the managed object handle must be released by bdmf_put() function.
 *
 * \param[in]   discr   Object discriminator: \n
 *                      type name followed by optional "/attribute_string",
 *                      whereas attr_string is in the same format as in bdmf_new_and_configure().
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
int bdmf_find_get_by_name(const char *discr, bdmf_object_handle *pmo);


/** Release managed object handle locked by one of bdmf_get(),
 * bdmf_find_get(), bdmf_find_get_by_name(), bdmf_get_next()
 * 
 * Following ddmf_put call the object handle can become invalid.
 *
 * \param[in]   mo      Managed object handle
 */
void bdmf_put(bdmf_object_handle mo);


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
                                 bdmf_object_handle mo, const char *filter);


/** Get object name
 * \param[in]   mo      Current managed object or NULL
 * \return object name
 */
const char *bdmf_object_name(bdmf_object_handle mo);


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
                                       const char *type, bdmf_object_handle mo);

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
int bdmf_is_linked(bdmf_object_handle ds, bdmf_object_handle us, bdmf_link_handle *plink);


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
int bdmf_link(bdmf_object_handle ds,
              bdmf_object_handle us, const char *attrs);


/** Unlink managed objects from each other
 *
 * \param[in]   ds      Downstream object handle
 * \param[in]   us      Upstream object handle
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_unlink(bdmf_object_handle ds, bdmf_object_handle us);


/** Upstream link iterator
 *
 * \param[in]   mo      Managed object
 * \param[in]   prev    Previous upstream link handle. NULL=get first
 * \return   next us link handle or NULL
 */
bdmf_link_handle bdmf_get_next_us_link(bdmf_object_handle mo,
                                          bdmf_link_handle prev);


/** Downstream link iterator
 *
 * \param[in]   mo      Managed object
 * \param[in]   prev    Previous downstream link handle. NULL=get first
 * \return   next ds link handle or NULL
 */
bdmf_link_handle bdmf_get_next_ds_link(bdmf_object_handle mo,
                                        bdmf_link_handle prev);



/** Map DS link to object
 *
 * \param[in]   ds_link
 * \return object handle
 */
bdmf_object_handle bdmf_ds_link_to_object(bdmf_link_handle ds_link);


/** Map US link to object
 *
 * \param[in]   us_link
 * \return object handle
 */
bdmf_object_handle bdmf_us_link_to_object(bdmf_link_handle us_link);

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
              const char *what, char *buffer, uint32_t size);



/** Get object owner.
 *
 * \param[in]   mo      Managed object
 * \param[out]  owner   Object owner
 */
void bdmf_get_owner(const bdmf_object_handle mo, bdmf_object_handle *owner);

/** @} end of bdmf_object sub-group */


/** \defgroup bdmf_attr Managed object attribute control APIs
 *  \ingroup bdmf
 *
 * APIs in this group allow to examine and modify Managed Objects' attributes.\n
 * \n
 * Attributes are that main mean for system configuration. Managed Objects are configured
 * by setting their attributes.\n
 * Broadlight Device Management Framework supports built-in attribute types specified in
 * #bdmf_attr_type_t type. The list of built-in types can be extended by plug-in device drivers
 * using #bdmf_attr_aggregate and #bdmf_attr_custom types.\n
 * #bdmf_attr_aggregate attribute is in fact a "structure" consisting of one or multiple attributes.
 * Aggregates can include "fields" of any types, including aggregate #bdmf_attr_aggregate.
 * Aggregate attribute initialization rules are specified by the driver at aggregate type registration time.
 * By default aggregate buffer is initialized with 0 before field assignment starts. Aggregate structure can be either initialized
 * as a whole using bdmf_attr_set_as_buf() function, or field by field using bdmf_new_and_configure(), bdmf_configure()
 * or bdmf_attr_set_as_string().
 * Driver specifies at aggregate type registration time which fileds are mandatory and which are optional. All mandatory
 * fields must be specified in the same descriptor when setting aggregate attribute value field-by-field.\n
 * #bdmf_attr_custom attributes are defined at sole discretion of plug-in drivers. Driver provides callback
 * functions for converting custom attributes to/from the external format.
 *
 * @{
 */

/** Attribute types.
    Note that the list can be extended by registered aggregate and custom types.
*/
typedef enum {
    bdmf_attr_number,       /**< Numeric attribute */
    bdmf_attr_string,       /**< 0-terminated string */
    bdmf_attr_buffer,       /**< Buffer with binary data */
    bdmf_attr_pointer,      /**< A pointer */
    bdmf_attr_object,       /**< Object reference */
    bdmf_attr_ether_addr,   /**< 6-byte Ethernet h/w address */
    bdmf_attr_ip_addr,      /**< 4-byte IPv4 address or 16-byte IPv6 address */
    bdmf_attr_ipv4_addr,    /**< 4-byte IPv4 address */
    bdmf_attr_ipv6_addr,    /**< 16-byte IPv6 address */
    bdmf_attr_boolean,      /**< boolean. default(first) value = true (1) */
    bdmf_attr_enum,         /**< enumeration with list of values in static table */
    bdmf_attr_dyn_enum,     /**< dynamic enumeration with list of values generated by callback */
    bdmf_attr_enum_mask,    /**< Bitmask containg multiple enum values */
    bdmf_attr_aggregate,    /**< aggregate type: "structure" consisting of multiple attributes */
    bdmf_attr_custom,       /**< user-defined type */

    bdmf_attr_last_type
} bdmf_attr_type_t;

/** Attribute information */
typedef struct bdmf_attr_info
{
    char name[BDMF_MAX_INFO_NAME_LENGTH];       /**< Attribute name */
    char help[BDMF_MAX_INFO_HELP_LENGTH];       /**< Attribute description */
    bdmf_attr_type_t type;                      /**< Attribute type */
    uint16_t size;                              /**< Attribute size */
    uint16_t array_size;                        /**< Attribute array dimension */
}  bdmf_attr_info_t;

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
int bdmf_attr_by_name(bdmf_type_handle drv,
                      const char *name, bdmf_attr_id *p_attr);


/** Get attribute info
 * \param[in]   drv     Managed object type handle
 * \param[in]   attr    Attribute handle
 * \param[out]  info    Attribute info
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_attr_info(bdmf_type_handle drv,
                   bdmf_attr_id attr, bdmf_attr_info_t *info);


/** Get attribute array element value as number.
 * 
 * The function can be used for numeric, enum and ipv4 attributes.
 * 
 * \param[in]   mo_or_mattr     Managed object or mattr handle
 * \param[in]   attr            Attribute handle
 * \param[in]   index           Array element index
 * \param[out]  pval            Attribute value
 * \return
 *     0 - OK\n
 *     error code otherwise
 */
int bdmf_attrelem_get_as_num(bdmf_object_handle mo_or_mattr,
                             bdmf_attr_id attr, bdmf_index index, bdmf_number *pval);


/** Set attribute array element value as number.
 *
 * The function can be used for numeric, enum and ipv4 attributes.
 * 
 * \param[in]   mo_or_mattr     Managed object or mattr handle
 * \param[in]   attr            Attribute handle
 * \param[in]   index           Array element index
 * \param[in]   val             Attribute value
 * \return
 *     0 - OK\n
 *     error code otherwise
 */
int bdmf_attrelem_set_as_num(bdmf_object_handle mo_or_mattr,
                             bdmf_attr_id attr, bdmf_index index, bdmf_number val);


/** Get attribute array element value in string (external) format.
 * 
 * \param[in]   mo_or_mattr     Managed object or mattr handle
 * \param[in]   attr            Attribute handle
 * \param[in]   index           Array element index
 * \param[out]  buffer          Buffer for output string
 * \param[in]   size            Buffer size
 * \return
 *     0 - OK\n
 *     error code otherwise
 */
int bdmf_attrelem_get_as_string(bdmf_object_handle mo_or_mattr,
                                bdmf_attr_id attr, bdmf_index index,
                                char *buffer, uint32_t size);


/** Set attribute array element using value in string (external) format.
 * 
 * \param[in]   mo_or_mattr     Managed object or mattr handle
 * \param[in]   attr            Attribute handle
 * \param[in]   index           Array element index
 * \param[in]   val             Value in external format - 0-terminated string
 * \return
 *     0 - OK\n
 *     error code otherwise
 */
int bdmf_attrelem_set_as_string(bdmf_object_handle mo_or_mattr,
                                bdmf_attr_id attr, bdmf_index index, const char *val);


/** Get attribute array element value as binary array.
 * 
 * \param[in]   mo_or_mattr     Managed object or mattr handle
 * \param[in]   attr            Attribute handle
 * \param[in]   index           Array element index
 * \param[out]  buffer          Buffer where attribute value is returned
 * \param[in]   size            Buffer size
 * \return
 *     >=0 - number of bytes copied
 *      <0 - error code
 */
int bdmf_attrelem_get_as_buf(bdmf_object_handle mo_or_mattr,
                             bdmf_attr_id attr, bdmf_index index,
                             void *buffer, uint32_t size);


/** Set attribute array element value from binary array.
 * 
 * \param[in]   mo_or_mattr     Managed object or mattr handle
 * \param[in]   attr            Attribute handle
 * \param[in]   index           Array element index
 * \param[in]   buffer          Buffer containing value
 * \param[in]   size            Buffer size
 * \return
 *     >=0 - number of bytes copied
 *      <0 - error code
 */
int bdmf_attrelem_set_as_buf(bdmf_object_handle mo_or_mattr,
                             bdmf_attr_id attr, bdmf_index index, const void *buffer,
                             uint32_t size);

/** Get scalar attribute value as number.
 *
 * The function can be used for numeric, enum and ipv4 attributes.
 * 
 * \param[in]   mo_or_mattr     Managed object or mattr handle
 * \param[in]   attr            Attribute handle
 * \param[out]  pval            Attribute value
 * \return
 *     0 - OK\n
 *     error code otherwise
 */
static inline int bdmf_attr_get_as_num(bdmf_object_handle mo_or_mattr,
                                       bdmf_attr_id attr, bdmf_number *pval)
{
    return bdmf_attrelem_get_as_num(mo_or_mattr, attr, -1, pval);
}

/** Set scalar attribute value as number.
 *
 * The function can be used for numeric, enum and ipv4 attributes.
 * 
 * \param[in]   mo_or_mattr     Managed object or mattr handle
 * \param[in]   attr            Attribute handle
 * \param[in]   val             Attribute value
 * \return
 *     0 - OK\n
 *     error code otherwise
 */
static inline int bdmf_attr_set_as_num(bdmf_object_handle mo_or_mattr,
                                       bdmf_attr_id attr, bdmf_number val)
{
    return bdmf_attrelem_set_as_num(mo_or_mattr, attr, -1, val);
}

/** Get scalar attribute value as string (external value).
 * 
 * \param[in]   mo_or_mattr     Managed object or mattr handle
 * \param[in]   attr            Attribute handle
 * \param[out]  buffer          Buffer for output string
 * \param[in]   size            Buffer size
 * \return
 *     0 - OK\n
 *     error code otherwise
 */
static inline int bdmf_attr_get_as_string(bdmf_object_handle mo_or_mattr,
                                          bdmf_attr_id attr, char *buffer, uint32_t size)
{
    return bdmf_attrelem_get_as_string(mo_or_mattr, attr, -1, buffer, size);
}

/** Set attribute value from string (external value).
 * 
 * \param[in]   mo_or_mattr     Managed object or mattr handle
 * \param[in]   attr            Attribute handle
 * \param[in]   val             Value in external format - 0-terminated string
 * \return
 *     0 - OK\n
 *     error code otherwise
 */
static inline int bdmf_attr_set_as_string(bdmf_object_handle mo_or_mattr,
                                          bdmf_attr_id attr, const char *val)
{
    return bdmf_attrelem_set_as_string(mo_or_mattr, attr, -1, val);
}

/** Get scalar attribute value as binary array
 * \param[in]   mo_or_mattr     Managed object or mattr handle
 * \param[in]   attr            Attribute handle
 * \param[out]  buffer          Buffer where attribute value is returned
 * \param[in]   size            Buffer size
 * \return
 *     >=0 - number of bytes copied
 *      <0 - error code
 */
static inline int bdmf_attr_get_as_buf(bdmf_object_handle mo_or_mattr,
                                       bdmf_attr_id attr, void *buffer, uint32_t size)
{
    return bdmf_attrelem_get_as_buf(mo_or_mattr, attr, -1, buffer, size);
}

/** Set scalar attribute value from binary array
 * \param[in]   mo_or_mattr     Managed object or mattr handle
 * \param[in]   attr            Attribute handle
 * \param[in]   buffer          Buffer containing value
 * \param[in]   size            Buffer size
 * \return
 *     >=0 - number of bytes copied
 *      <0 - error code
 */
static inline int bdmf_attr_set_as_buf(bdmf_object_handle mo_or_mattr,
                                       bdmf_attr_id attr, const void *buffer, uint32_t size)
{
    return bdmf_attrelem_set_as_buf(mo_or_mattr, attr, -1, buffer, size);
}


/** Add attribute array element value as number.
 *
 * The function can be used for dynamic arrays of numeric, enum and ipv4 attributes.
 *
 * \param[in]   mo              Managed object handle
 * \param[in]   attr            Attribute handle
 * \param[in, out]  index       Array element index (handle). On input may contain a hint
 * \param[in]   val             Attribute value
 * \return
 *     0 - OK\n
 *     error code otherwise
 */
int bdmf_attrelem_add_as_num(bdmf_object_handle mo, bdmf_attr_id attr, bdmf_index *index, bdmf_number val);


/** Add attribute array element value as string.
 *
 * \param[in]       mo          Managed object handle
 * \param[in]       attr        Attribute handle
 * \param[in, out]  index       Array element index (handle). On input may contain a hint
 * \param[in]       val         Value in external format - 0-terminated string
 * \return
 *     0 - OK\n
 *     error code otherwise
 */
int bdmf_attrelem_add_as_string(bdmf_object_handle mo, bdmf_attr_id attr, bdmf_index *index, const char *val);


/** Add attribute array element value as buffer.
 *
 * \param[in]       mo          Managed object handle
 * \param[in]       attr        Attribute handle
 * \param[in]       buffer      Buffer containing value
 * \param[in, out]  index       Array element index (handle). On input may contain a hint
 * \param[in]       size        Buffer size
 * \return
 *     >=0 - number of bytes copied
 *     error code otherwise
 */
int bdmf_attrelem_add_as_buf(bdmf_object_handle mo, bdmf_attr_id attr, bdmf_index *index, const void *buffer,
    uint32_t size);


/** Delete attribute array element
 *
 * The function can be used for dynamic arrays.
 *
 * \param[in]       mo          Managed object handle
 * \param[in]       attr        Attribute handle
 * \param[in]       index       Array element index (handle)
 * \return
 *     =0 - OK
 *     error code otherwise
 */
int bdmf_attrelem_delete(bdmf_object_handle mo, bdmf_attr_id attr, bdmf_index index);


/** Get next attribute array index
 *
 * \param[in]       mo          Managed object handle
 * \param[in]       attr        Attribute handle
 * \param[in, out]  index       Array element index (handle). Seed with BDMF_INDEX_UNASSIGNED for get-first
 * \return
 *     =0 - OK
 *     =BDMF_ERR_NOENT - no more entries
 */
int bdmf_attrelem_get_next(bdmf_object_handle mo, bdmf_attr_id attr, bdmf_index *index);


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
int bdmf_attrelem_find(bdmf_object_handle mo, bdmf_attr_id attr, bdmf_index *index, void *buffer,
    uint32_t size);


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
int bdmf_attrelem_find_by_string(bdmf_object_handle mo, bdmf_attr_id attr, bdmf_index *index, const char *val);


/** Set a number of attributes in a single call.
 * 
 * The attributes and values are passed as a comma-delimted
 * ASCII string of name=value pairs.\n
 * For enum attrubutes "=value" can be omitted. In this case
 * the 1st enum value is assumed. For example, if "bool_attr" is
 * boolean attribute, specifying "bool_attr" in the attribute string without value
 * means settings its value "=true", because "true" is the 1st value of "boolean"
 * enumeration.\n
 * For aggregate attributes the value must be surrounded by "{}" brackets and
 * has the following format:\n
 * aggr_attr1={field_name1=value1,field_name2=value2}\n
 * Nested "{} brackets are allowed to accommodate the case of aggregate attribute's
 * field itself being an aggregate.\n
 * 
 * Example of attribute string:\n
 * attr1=25,attr2=string1,attr3=enum_value3,attr4="string 4",enum_attr5,aggr_attr1={f1=v1,f2=v2}\n
 * 
 * \param[in]   mo      Managed object handle
 * \param[in]   set     Comma delimited list of name=value pairs
 * \return
 *     0 - OK\n
 *     error code otherwise
 */
int bdmf_configure(bdmf_object_handle mo, const char *set);


/** \defgroup bdmf_mattr Attribute groups
 *  \ingroup bdmf_attr
 *
 * Attribute groups enable setting or fetching multiple object attributes
 * in a single call bdmf_mattr_set(), bdmf_mattr_get()\n
 * Attribute values can be an arbitrary mix of string and type-specific internal formats.
 * @{
 */

/** mattr operation */
typedef enum
{
    bdmf_attr_op_any,       /**< Set/Get attribute */
    bdmf_attr_op_set,       /**< Set attribute */
    bdmf_attr_op_get,       /**< Get attribute */
} bdmf_attr_op_t;

/** Attribute value */
typedef struct
{
    bdmf_attr_type_t val_type;  /**< value type */
    union
    {
        bdmf_number num;    /**< number , boolean, enum */
        const char *s;      /**< Value in string format */
        bdmf_number *pnum;  /**< Pointer to value for "get_as_num" operation */
        /** Raw buffer format */
        struct
        {
            void *ptr;
            uint16_t len;
        } buf;
    } x;
} bdmf_attr_val_t;

/** mattr set entry */
typedef struct bdmf_mattr_entry
{
    bdmf_attr_id aid;       /**< Attribute id */
    bdmf_index index;       /**< Array index */
    bdmf_attr_val_t val;    /**< Value */
} bdmf_mattr_entry_t;

/** Attribute set.
 * All fields in this structure are internal and should not be touched
 * by the drivers;
 */
struct bdmf_mattr
{
    uint32_t magic;         /**< Magic number to distinguish from other entities */
    bdmf_attr_op_t oper;    /**< Mattr operation */
    bdmf_type_handle drv;   /**< Type handle */
    int max_entries;        /**< Max number of entries in entries[] array */
    int num_entries;        /**< Current number of entries in entries[] array */
    int dynamic;            /**< 1=allocated dynamically */
    bdmf_mattr_entry_t entries[0]; /**< Mattr entries */
};


static inline bdmf_mattr_t *bdmf_mattr_init(bdmf_mattr_t *mattr, bdmf_type_handle drv)
{
    mattr->drv = drv;
    mattr->magic=BDMF_MATTR_MAGIC;
    mattr->num_entries = 0;
    mattr->max_entries = bdmf_type_num_attrs(drv);
    mattr->oper = bdmf_attr_op_any;
    mattr->dynamic = 0;
    return mattr;
}

/** Declare mattr descriptor
 * \param[in]   name        Variable name to declare of type (bdmf_mattr_t *)
 * \param[in]   op          Operation
 * \param[in]   drv         Type for which the set is created (FFU)
 */
#define BDMF_MATTR(name, drv)               \
    struct { bdmf_mattr_t hdr; bdmf_mattr_entry_t entries[bdmf_type_num_attrs(drv)]; } __ ## name; \
    bdmf_mattr_handle name = (bdmf_object_handle)bdmf_mattr_init((bdmf_mattr_t *)&__ ## name, drv)


/** Allocate mattr descriptor.
 *
 * This function provides an alternative to BDMF_MATTR macro.
 * Unlike BDMF_MATTR which allocates mattr descriptor on the stack,
 * bdmf_mattr_alloc() uses dynamic memory allocation.
 * Descriptor allocated by bdmf_mattr_alloc() must be released
 * using bdmf_free()
 * \param[in]   drv     Object type mattr block will be used for
 * \return mattr block pointer or NULL if no memory
 */
bdmf_mattr_handle bdmf_mattr_alloc(bdmf_type_handle drv);

/** Set a number of object attributes in a single call.
 *
 * \param[in]   mo      Managed object handle
 * \param[in]   mattr   Attribute set.\n
 *                      The set is released automatically.\n
 * \return
 *     0      - OK \n
 *    <0      - error
 */
int bdmf_mattr_set(bdmf_object_handle mo, bdmf_mattr_handle mattr);

/** Get a number of object attributes in a single call.
 *
 * \param[in]       mo      Managed object handle
 * \param[in]       mattr   Attribute set to be fetched
 * \return
 *     0      - OK \n
 *    <0      - error
 */
int bdmf_mattr_get(bdmf_object_handle mo, bdmf_mattr_handle mattr);

/** Release mattr chain
 *
 * \param[in]       mattr   Mattr to be released
 */
void bdmf_mattr_free(bdmf_mattr_handle mattr);


/** @} end of bdmf_mattr sub-group */

/** @} end of bdmf_attr sub-group */


/** \defgroup bdmf_if_trace Tracing and logging
 * \ingroup bdmf
 * @{
 */

/** Trace levels
 */
typedef enum
{
    bdmf_trace_level_none,      /**< Tracing is disabled */
    bdmf_trace_level_error,     /**< Trace errors only */
    bdmf_trace_level_info,      /**< Trace errors and info, including configuration changes */
    bdmf_trace_level_debug,     /**< Trace everything */
} bdmf_trace_level_t;

#ifndef BDMF_NO_TRACE

/** Global trace level (\ref bdmf_trace_level_t) */
extern int bdmf_global_trace_level;

/* Add trace entry
 * \param[in]   fmt         printf-like format
 */
void bdmf_trace(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/** Print error trace conditional on global trace level
 * \param[in]   fmt         printf-like format
 * \param[in]   args        0 or more parameters
 */
#define BDMF_TRACE_ERR(fmt, args...)                                \
    do {                                                            \
        if (bdmf_global_trace_level >= bdmf_trace_level_error)      \
            bdmf_trace("ERR: %s#%d: " fmt, __FUNCTION__, __LINE__, ## args);\
    } while(0)


/** Print info trace conditional on global trace level
 * \param[in]   fmt         printf-like format
 * \param[in]   args        0 or more parameters
 */
#define BDMF_TRACE_INFO(fmt, args...)                      \
    do {                                                            \
        if (bdmf_global_trace_level >= bdmf_trace_level_info)       \
            bdmf_trace("INF: %s#%d: " fmt, __FUNCTION__, __LINE__, ## args);\
    } while(0)


/** Print info or error trace conditional on global trace level and return
 * \param[in]   rc          return code. 0=info trace, !=0-error trace
 * \param[in]   fmt         printf-like format
 * \param[in]   args        0 or more parameters
 */
#define BDMF_TRACE_RET(rc, fmt, args...)                   \
    do {                                                            \
        if (rc)\
            BDMF_TRACE_ERR("status:%s " fmt, bdmf_strerror(rc), ## args);\
        else \
            BDMF_TRACE_INFO("success " fmt, ## args);\
        return (rc);\
    } while(0)


#ifdef BDMF_DEBUG

/** Print debug trace conditional on global trace level
 * \param[in]   fmt         printf-like format
 * \param[in]   args        0 or more parameters
 */
#define BDMF_TRACE_DBG(fmt, args...)                       \
    do {                                                            \
        if (bdmf_global_trace_level >= bdmf_trace_level_debug)      \
            bdmf_trace("DBG: %s#%d: " fmt, __FUNCTION__, __LINE__, ## args);\
    } while(0)

#else /* #ifdef BDMF_DEBUG */

#define BDMF_TRACE_DBG(fmt, args...)

#endif /* #ifdef BDMF_DEBUG */

/** @} end of bdmf_if_trace doxygen group */

#else /* #ifdef BDMF_NO_TRACE  */
#define BDMF_TRACE_RET(rc, fmt, args...) do { return rc; } while (0)
#define BDMF_TRACE_INFO(fmt, args...)
#define BDMF_TRACE_ERR(fmt, args...)
#define BDMF_TRACE_DBG(fmt, args...)
#define bdmf_trace printf
#endif

/** \defgroup bdmf_lock Protection APIs
 *  \ingroup bdmf
 *
 * APIs in this group are used for access protection in multi-tasking environment.
 * @{
 */

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
int bdmf_lock(void);

/** Release global lock.
 * Release global lock acquired by bdmf_lock() call.
 */
void bdmf_unlock(void);

/** @} end of bdmf_lock group */

/** @} end of bdmf group */

#endif /* _BDMF_INTERFACE_H_ */
