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

#ifndef _BL_DB_ENGINE_H_
#define _BL_DB_ENGINE_H_

#include <bdmf_system.h>

/**  \defgroup bdmf_dbe Data Base Engine Module
Hierarchical data base is built from 3 types of objects:
- set: consists of control info and dynamic array of other sets OR of records of the same kind.
    - Mixing subsets and records in the same set is not supported.
    - Sets are objects that perform operations like access, locking, adding or removing elements,
      etc., via methods that can differ for every set.
    - Set elements are addressed using a single key.
    - Most sets are internally organized as arrays. However, other organizations (e.g., lists, hash tables)
      are also possible because each set can have different set of methods for element access.
- record: is a container for storing information.
    - Record consists of control info and a structure containing fields.
    - Record is the smallest DB element that has a handle and can be individually locked.
    - Record size is fixed at time when the set containing records is created.
- field: is a convenience element.
    - DB API includes field access macros for convenience and traceability.
      Apart from that, record layout is transparent to the DB engine.
    - DB user has an option of accessing record fields directly (as C structure fields), without using DB API
 @{
*/

/** Data base backend type
 */
typedef enum
{
    BDMF_DB_BACKEND_ARRAY,    /**< Array-based backend */
    BDMF_DB_BACKEND_HASH,     /**< Hash-based backend */
    BDMF_DB_BACKEND_OTHER     /**< User-defined backend */
} bdmf_db_backend_type_t;


/** Data locking policy
 */
typedef enum
{
    BDMF_DB_LOCK_NONE,                /**< No record-level locking. Can be used for records containing independent fields */
    BDMF_DB_LOCK_NB_READ_SHADOW_WRITE,/**< Non-blocking read, write using shadow area (default) */
    BDMF_DB_LOCK_SEM_READ_SEM_WRITE,  /**< Strong locking. Both read and write locks use semaphores */
    BDMF_DB_LOCK_OTHER                /**< User-defined locking policy */
} bdmf_db_lock_policy_t;


/** Data base key
 * Valid values >= 0
 */
typedef int bdmf_db_key_t;


/** Any key
 */
#define BDMF_DB_KEY_ANY       (-1)

/** Invalid key
 */
#define BDMF_DB_KEY_INVAL     (-2)

/** No more records
 */
#define BDMF_DB_KEY_NO_MORE   (-3)


 /** Data Base Set control block handle
 */
typedef struct bdmf_db_set *bdmf_dbset_handle_t;


/** Data base set or record handle
 */
typedef struct bdmf_db_entry *bdmf_db_handle_t;


/** Data base operations for notifications.
 */
typedef enum
{
    BDMF_DB_OPER_ADD,         /**< Entry has been added */
    BDMF_DB_OPER_DELETE,      /**< Entry has been deleted */
    BDMF_DB_OPER_UPDATE       /**< Entry has been modified */
} bdmf_db_oper_t;


/** Data base update notification callback.
 */
typedef void (*bdmf_db_notify_cb_t)(bdmf_dbset_handle_t set, bdmf_db_key_t key, bdmf_db_oper_t oper, void *new_data);


/** Format callback. Used by bdmf_dbrecord_read_formatted to convert record data to human-readible format */
typedef int (*bdmf_db_format_cb_t)(const void *data, char *buffer, int buffer_size);


/** Set-of-Sets init structure.
 */
typedef struct bdmf_db_sos_init
{
    const char *name;                   /**< Set name */
    bdmf_db_backend_type_t backend_type;  /**< Backend type */
    uint32_t max_entries;               /**< Max number of entries. 0=unlimited (not supported for array backend) */
} bdmf_db_sos_init_t;


/** Set-of-Records init structure.
 */
typedef struct bdmf_db_sor_init
{
    const char *name;                   /**< Set name */
    bdmf_db_backend_type_t backend_type;  /**< Backend type */
    bdmf_db_lock_policy_t lock_policy;    /**< Set locking policy */
    uint32_t max_entries;               /**< Max number of entries. 0=unlimited (not supported for array backend) */
    uint32_t record_size;               /**< Record size > 0 */
    bdmf_db_format_cb_t format;           /**< callback that converts record data to human-readable form */
} bdmf_db_sor_init_t;


/** Initialize data base engine
 * 
 * \return
 *      0   - OK\n
 *      <0  - error code
 */
int bdmf_db_module_init(void);


/** Make set-of-sets control block.
 * 
 * Helper function that creates a set of sets with reasonable defaults for all callbacks and fields.
 * Once created, the set control block can be tuned before adding the new set to its parent set.
 * \param[in]   init            set parameters
 * \param[out]  new_set         set control block
 * \return
 *      0   - OK\n
 *      <0  - error code
 */ 
int bdmf_db_make_set_of_sets(const bdmf_db_sos_init_t *init, bdmf_dbset_handle_t *new_set);


/** Make set-of-sets control block macro.
 *
 * Calls \ref bdmf_db_make_set_of_sets.
 * Prints error message and jumps to error_label in case of failure.
 * For parameter description see \ref bdmf_db_make_set_of_sets
 */
#define BDMF_DB_MAKE_SOS(_name,_backend,_max_entries,_p_handle,_rc,_error_label) \
({\
    bdmf_db_sos_init_t _init = { .name=_name, .max_entries=_max_entries, .backend_type=_backend};\
    _rc = bdmf_db_make_set_of_sets(&_init, _p_handle);\
    if (_rc)\
    {\
        bdmf_print("%s: failed to create set %s. rc=%d\n", __FUNCTION__, _name, _rc);\
        goto _error_label;\
    }\
})


/** Make set-of-records control block.
 *
 * Helper function that creates a set of records with reasonable defaults for all callbacks and fields.
 * Once created, the set control block can be tuned before adding the new set to its parent set.
 * \param[in]   init            set parameters
 * \param[in]   alloc_records   true (1) - allocate memory for all records.
 * \param[out]  new_set         set control block
 * \return
 *      0   - OK\n
 *      <0  - error code
 */ 
int bdmf_db_make_set_of_records(const bdmf_db_sor_init_t *init, int alloc_records, bdmf_dbset_handle_t *new_set);


/** Make set-of-records control block macro.
 *
 * Calls \ref bdmf_db_make_set_of_records.
 * Prints error message and jumps to error_label in case of failure.
 * For parameter description see \ref bdmf_db_make_set_of_records
 */
#define BDMF_DB_MAKE_SOR(_name,_backend,_lock,_max_entries,_rec_size,_is_alloc,_format,_p_handle,_rc,_error_label) \
({\
    bdmf_db_sor_init_t _init = { .name=_name, .max_entries=_max_entries, .backend_type=_backend,\
                               .lock_policy=_lock, .record_size=_rec_size,.format=_format};\
    _rc = bdmf_db_make_set_of_records(&_init,_is_alloc,_p_handle);\
    if (_rc)\
    {\
        bdmf_print("%s: failed to create record set %s. rc=%d\n", __FUNCTION__, _name, _rc);\
        goto _error_label;\
    }\
})


/** Lock data set. When set is locked - it can't be modified.
 * 
 * \param[in]   set             data base set to be locked
 * 
 */
void bdmf_dbset_lock_read(bdmf_dbset_handle_t set);


/** Release data set lock
 *
 * Unlock set locked by \ref bdmf_dbset_lock_read
 *
 * \param[in]   set             data base set to be unlocked
 */
void bdmf_dbset_unlock_read(bdmf_dbset_handle_t set);


/** Lock data set for modify. If the set is SoS, the locking
 *  will be recursive.
 *
 * \param[in]   set             data base set to be locked
 *
 * \ingroup bdmf_dbe
 */
void bdmf_dbset_lock_modify(bdmf_dbset_handle_t set);


/** Release data set lock
 *
 * Unlock set locked by \ref bdmf_dbset_lock_modify
 *
 * \param[in]   set             data base set to be unlocked
 *
 * \ingroup bdmf_dbe
 */
void bdmf_dbset_unlock_modify(bdmf_dbset_handle_t set);


/** Add set to the parent set.
 *
 * The function adds set to the parent set creating data base hierarchy.
 * The function automatically acquires modify lock and releases it
 * in the end of operation.
 * \param[in]   sos             parent set of sets
 * \param[in]   key             key to add new set at
 * \param[in]   new_set         set control block
 * \return
 *      =0  - OK\n
 *      <0  - error code
 */
int bdmf_dbset_add(bdmf_dbset_handle_t sos, bdmf_db_key_t key, bdmf_dbset_handle_t new_set);


/** Add set to the parent set with specific key macro.
 *
 * Calls \ref bdmf_dbset_add.
 * Prints error message and jumps to error_label in case of failure.
 * For parameter description see \ref bdmf_dbset_add
 */
#define BDMF_DBSET_ADD(_parent,_key,_set,_rc,_error_label) \
({\
    _rc = bdmf_dbset_add(_parent,_key,_set);\
    if (_rc)\
    {\
        bdmf_print("%s: failed to add set %s to %s. rc=%d\n", __FUNCTION__, \
                    bdmf_dbset_name(_set), bdmf_dbset_name(_parent), _rc);\
        goto _error_label;\
    }\
})


/** Get set handle given its key.
 *
 * \param[in]   sos             parent set of sets
 * \param[in]   key             set key.
 * \return
 *      !=0 - set handle
 *      NULL- doesn't exist
 */
bdmf_dbset_handle_t bdmf_dbset_handle(const bdmf_dbset_handle_t sos, bdmf_db_key_t key);


/** Get set key given its handle.
 *
 * \param[in]   set             set handle
 * \return
 *      !=BDMF_DB_KEY_INVAL - set key\n
 *      BDMF_DB_KEY_INVAL - error
 */
bdmf_db_key_t bdmf_dbset_key(const bdmf_dbset_handle_t set);


/** Get set name
 * 
 * \param[in]   set             set handle
 * \return set name
 */
const char *bdmf_dbset_name(const bdmf_dbset_handle_t set);


/** Get number of records in the set.
 *
 * \param[in]   set             set handle
 * \return number of active records in the set
 */
int bdmf_dbset_num_records(const bdmf_dbset_handle_t set);


/** Get entry size
 *
 * \param[in]   set             set handle
 * \return set entry size
 */
int bdmf_dbset_entry_size(const bdmf_dbset_handle_t set);


/** Add record to the parent set.
 *
 * The function creates a new record and adds it to the parent set with specific key.
 * The function automatically acquires modify lock and releases it
 * in the end of operation.
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 * \param[in]   data            record data. Data size is defined at parent SOR registration time.
 * \return
 *      =0  - OK\n
 *      <0  - error code
 */
int bdmf_dbrecord_add(bdmf_dbset_handle_t sor, bdmf_db_key_t key, const void *data);


/** Delete record from the parent SoR given the record key.
 * 
 * The function automatically acquires modify lock and releases it
 * in the end of operation.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key.
 */
void bdmf_dbrecord_delete(bdmf_dbset_handle_t sor, bdmf_db_key_t key);


/** Get record data pointer without locking.
 *
 * The function returns pointer to data structure stored in data base record.\n
 * Attention! The caller is required to aquire read or write lock - as appropriate
 * before calling this function and release the lock when processing is finished.
 * The function is low-level. It is recommended to use \ref bdmf_dbrecord_get_nolock instead.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 * \return
 *      data pointer. NULL if there is no record matching the key.
 */
void *bdmf_dbrecord_getraw_nolock(bdmf_dbset_handle_t sor, bdmf_db_key_t key);


/** Get record data pointer without locking.
 *
 * The function returns pointer to data structure stored in data base record.\n
 * Attention! The caller is required to aquire read or write lock - as appropriate
 * before calling this function and release the lock when processing is finished.
 *
 * \param[in]   _sor            parent set of records
 * \param[in]   _key            record key
 * \param[in]   _record_type    underlying data type.
 * \return
 *      data pointer casted to the underlying data type\n
 *      NULL if there is no record matching the key.
 */
#define bdmf_dbrecord_get_nolock(_sor, _key, _record_type)      \
    ({ \
        assert(sizeof(_record_type)==bdmf_dbset_entry_size(_sor)); \
        (_record_type *)bdmf_dbrecord_getraw_nolock(_sor, _key); \
     })


/** Lock record for reading and return record data pointer.
 *
 * The function aquires read-lock and returns pointer to data structure stored in data base record.\n
 * read-lock must be released separately when the pointer is no longer in use.
 * Note that the default record-read lock is non-blocking and counting.
 * That means that multiple processes can read-lock the same record without blocking.
 * The function is low-level. It is recommended to use macro \ref bdmf_dbrecord_get_read instead.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 * \return
 *      data pointer. NULL if there is no record matching the key.
 */
const void *bdmf_dbrecord_getraw_read(bdmf_dbset_handle_t sor, bdmf_db_key_t key);


/** Lock record for reading and return record data pointer.
 *
 * The macro returns pointer to data structure stored in data base record.\n
 * The read-lock must be released separately when the pointer is no longer in use.
 * Note that the default record-read lock is non-blocking and counting.
 * That means that multiple processes can read-lock the same record without blocking.
 *
 * \param[in]   _sor            parent set of records
 * \param[in]   _key            record key
 * \param[in]   _record_type    underlying data type.
 * \return
 *      data pointer casted to the underlying data type
 */
#define bdmf_dbrecord_get_read(_sor, _key, _record_type) \
    ({ \
        assert(sizeof(_record_type)==bdmf_dbset_entry_size(_sor));  \
        (const _record_type *)bdmf_dbrecord_getraw_read(_sor, _key);\
     })


/** Unlock record locked for reading.
 *
 * This function must be called after \ref bdmf_dbrecord_get_read or
 * \ref bdmf_dbrecord_getraw_read. Following bdmf_dbrecord_read_unlock
 * call pointer returned by \ref bdmf_dbrecord_get_read becomes invalid.
 * 
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 * 
 */
void bdmf_dbrecord_unlock_read(bdmf_dbset_handle_t sor, bdmf_db_key_t key);


/** Read record data into user area.
 *
 * The function aquires read-lock, reads data into user area and releases read-lock.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 * \param[in]   offset          offset in data record
 * \param[in]   size            data size. Note that offset+size must be <= record_size
 * \param[in]   data            data pointer.
 * \return
 *      =0-OK\n
 *      <0-error code
 */
int bdmf_dbrecord_read(bdmf_dbset_handle_t sor, bdmf_db_key_t key, int offset, int size, void *data);


/** Get record field.
 *
 * The macro returns record field value.
 *
 * \param[in]   _sor            parent set of records
 * \param[in]   _key            record key
 * \param[in]   _record_type    type of the underlying data structure.
 * \param[in]   _field_name     data structure field name
 * \param[out]  _p_field_value  pointer of variable wehre data structure field value should be returned
 * \return
 *      =0-OK\n
 *      <0-error code
 */
#define bdmf_dbrecord_read_field(_sor, _key, _record_type, _field_name, _p_field_value) \
    bdmf_dbrecord_read(_sor, _key, offsetof(_record_type, _field_name), \
                        sizeof(*(_p_field_value)), _p_field_value);


/** Lock record for writing and return record data pointer.
 *
 * The function aquires write-lock and returns pointer to data structure stored in data base record.\n
 * write-lock must be released separately when the pointer is no longer in use.
 * The function is low-level. It is recommended to use macro \ref bdmf_dbrecord_get_write instead.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 * \return
 *      data pointer. NULL if there is no record matching the key.
 */
void *bdmf_dbrecord_getraw_write(bdmf_dbset_handle_t sor, bdmf_db_key_t key);


/** Lock record for writing and return record data pointer.
 *
 * The function aquires write-lock and returns pointer to data structure stored in data base record.\n
 * write-lock must be released separately when the pointer is no longer in use.
 *
 * \param[in]   _sor            parent set of records
 * \param[in]   _key            record key
 * \param[in]   _record_type    underlying data type.
 * \return
 *      data pointer casted to the underlying data type
 */
#define bdmf_dbrecord_get_write(_sor, _key, _record_type)      \
    ({ \
        assert(sizeof(_record_type)==bdmf_dbset_entry_size(_sor));   \
        (_record_type *)bdmf_dbrecord_getraw_write(_sor, _key);\
     })


/** Unlock record locked for writing.
 *
 * This function must be called after \ref bdmf_dbrecord_get_write or
 * \ref bdmf_dbrecord_getraw_write. Following bdmf_dbrecord_write_unlock
 * call pointer returned by \ref bdmf_dbrecord_get_write becomes invalid.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 *
 */
void bdmf_dbrecord_unlock_write(bdmf_dbset_handle_t sor, bdmf_db_key_t key);



/** Write record data.
 *
 * The function aquires modify-lock, replaces data stored in data base record
 * and releses the lock.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 * \param[in]   offset          offset in data record
 * \param[in]   size            data size. Note that offset+size must be <= record_size
 * \param[in]   data            data pointer.
 * \return
 *      =0-OK\n
 *      <0-error code
 */
int bdmf_dbrecord_write(bdmf_dbset_handle_t sor, bdmf_db_key_t key, int offset, int size, const void *data);


/** Write record field.
 *
 * The macro updates record field value.\n
 * The macro aquires and releases record-modify lock.
 *
 * \param[in]   _sor            parent set of records
 * \param[in]   _key            record key
 * \param[in]   _record_type    type of the underlying data structure.
 * \param[in]   _field_name     data structure field name
 * \param[in]   _field_value    field value
 * \return
 *      =0-OK\n
 *      <0-error code
 */
#define bdmf_db_record_write_field(_sor, _key, _record_type, _field_name, _field_value) \
    ({ \
        typeof(((_record_type *)0)->_field_name) _f = _field_value;\
        bdmf_dbrecord_write(_sor, _key, offsetof(_record_type, _field_name), sizeof(_f), &_f);\
    });


/** Register notification function to get informed
 * when data base set is modified.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   cb              callback function pointer
 * \param[in]   cb_priv         private data that should be passed to the callback
 * \return
 *      =0  - OK\n
 *      <0  - error code
 */
int bdmf_dbset_notify_register(bdmf_dbset_handle_t sor, bdmf_db_notify_cb_t cb, long cb_priv);


/** Data base iterator
 *
 * \param[in]   set             data base set
 * \param[in]   prev            last entry. BDMF_DB_KEY_ANY=start from the beginning
 * \return  data base entry key following prev or BDMF_DB_KEY_NO_MORE if end is reached.\n
 *          BDMF_DB_KEY_INVAL is reqturned if prev key is invalid
 */
bdmf_db_key_t bdmf_dbset_iterate(bdmf_dbset_handle_t set, bdmf_db_key_t prev);


/** Print database structure.
 *
 * \param[in]   set             root set
 */
void bdmf_dbset_print_structure(const bdmf_dbset_handle_t set);


/** Format record for printing.
 *
 * The function converts record data to human-readable format.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 * \param[out]  buffer          output buffer
 * \param[in]   size            buffer size
 * \return
 *      >=0-amount of data placed in the buffer\n
 *      <0-error code
 */
int bdmf_dbrecord_read_formatted(bdmf_dbset_handle_t sor, bdmf_db_key_t key, char *buffer, int size);


/** @} end of bdmf_dbe group */


#endif /* _BL_DB_ENGINE_H_ */

