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


#include <bdmf_system.h>
#include <db_engine.h>


/** Data base entry
 * \ingroup bdmf_dbe
 */
typedef struct bdmf_db_entry
{
    void *data;       /* Set or record */
    uint8_t flags;
#define BDMF_DB_FLAG_VALID    0x01  /**< Entry is valid */
#define BDMF_DB_FLAG_RECORD   0x02  /**< Record */
#define BDMF_DB_FLAG_SOS      0x10  /**< Set of sets */
    uint8_t read_count;
    uint8_t write_pending;
    uint8_t ffu;
} bdmf_db_entry_t;

/** Data Base Record control block
 * \ingroup bdmf_dbe
 */
typedef struct bdmf_db_set bdmf_db_set_t;

/** Data Base Record control block
 * \ingroup bdmf_dbe
 */
typedef struct bdmf_db_record bdmf_db_record_t;

/** Set/Record change notification
 * \ingroup bdmf_dbe
 */
typedef struct bdmf_db_notify
{
    struct bdmf_db_notify *next;
    bdmf_db_notify_cb_t cb;
    long cb_priv;
} bdmf_db_notify_t;

/** Data Base Set control block
 * \ingroup bdmf_dbe
 */
struct bdmf_db_set
{
    bdmf_db_entry_t entry;        /**< Common fielsds for set and record */
    char *name;                 /**< Set name */
    bdmf_db_set_t *parent;        /**< Set parent */
    bdmf_db_key_t my_key;         /**< Key in the parent set */
    int max_entries;            /**< Max number of elements in the set. -1=inlimited */
    int num_entries;            /**< Current number of elements in the set */
    int entry_size;             /**< Set entry size. */
    int magic;                  /**< Magic number */
#define BDMF_DB_MAGIC_ACTIVE_SET         (('a'<<24) | ('s'<<16) | ('e'<<8) | 't')
#define BDMF_DB_MAGIC_FREE_SET           (('f'<<24) | ('s'<<16) | ('e'<<8) | 't')

    /** Get next element */
    bdmf_db_key_t (*entry_get_next)(const bdmf_db_set_t *_this, bdmf_db_key_t cur);

    /** Add new entry. returns 0 if ok */
    int (*entry_new)(bdmf_db_set_t *_this, bdmf_db_key_t key, void *data);

    /** Delete entry */
    int (*entry_delete)(bdmf_db_set_t *_this, bdmf_db_entry_t *entry);

    /*
     * Handle – key mapping
     */

    /** Convert entry handle to entry key */
    bdmf_db_key_t (*handle_to_key)(const bdmf_db_set_t *_this, const bdmf_db_entry_t *entry);

    /** Convert entry key to entry handle */
    bdmf_db_entry_t *(*key_to_handle)(const bdmf_db_set_t *_this, bdmf_db_key_t key);

    /*
     * Set/Record locking
     * entry is handle of the set or record to be locked/unlocked
     */

    /** Lock the whole set for reading */
    void (*lock_set_read)(bdmf_db_set_t *set);

    /** Unlock set locked for reading */
    void (*unlock_set_read)(bdmf_db_set_t *set);

    /** Lock the whole set for update */
    void (*lock_set_modify)(bdmf_db_set_t *set);

    /** Unlock set locked for update */
    void (*unlock_set_modify)(bdmf_db_set_t *set);

    /** Lock the set recursively for update (SoS only) */
    void (*lock_set_recursively_modify)(bdmf_db_set_t *set);

    /** Unlock recursively set locked for update (SoS only)  */
    void (*unlock_set_recursively_modify)(bdmf_db_set_t *set);

    /** Lock record for reading */
    void *(*lock_record_read)(bdmf_db_set_t *set, bdmf_db_key_t key);

    /** Release read lock */
    void (*unlock_record_read)(bdmf_db_set_t *set, bdmf_db_key_t key);

    /** Lock record for modification. */
    void *(*lock_record_write)(bdmf_db_set_t *set, bdmf_db_key_t key, int is_deletion);

    /** Release modify lock */
    void (*unlock_record_write)(bdmf_db_set_t *set, int is_deletion);

    /** Format function that converts record data to human-readable form */
    bdmf_db_format_cb_t format;

    /** Update notification mechanism.\n
     * Note that notification functions are called before the actual update of the data base, so that
     * there is an option to abort the update if needed.
     */
    bdmf_db_notify_t *notify_list_head;

    /** Shadow data record */
    void *shadow_data;

    /** holds the pointer to a write-locked entry */
    bdmf_db_entry_t * write_locked_entry ;

    /** mutex that prevents multiple simultaneous updates */
    bdmf_ta_mutex mutex_update;

    /** semaphore that holds update while there are unfinished
     *  readings */
    bdmf_mutex sem_wait_read_to_finish;

    /** fastlock */
    bdmf_fastlock fastlock;
};


/**< Data base record
 * \ingroup bdmf_dbe
 */
struct bdmf_db_record
{
    struct bdmf_db_entry e;       /**< Entry - common part for set and record */
};

/*
 * DB backend callbacks
 */

/*
 * Array-based backend
 */

/** Get next element */
static bdmf_db_key_t dbbe_array_entry_get_next(const bdmf_db_set_t *_this, bdmf_db_key_t key)
{
    assert(_this->entry.data);

    if (key < 0)
        key = 0;
    else
        ++key;

    while((unsigned)key<_this->max_entries)
    {
        bdmf_db_entry_t *entry = (bdmf_db_entry_t *)_this->entry.data + key;
        if ((entry->flags & BDMF_DB_FLAG_VALID))
            break;
        ++key;
    }

    if ((unsigned)key >= _this->max_entries)
        return BDMF_DB_KEY_NO_MORE;  /* no more */
    
    return key;
}

/*
 * Handle – key mapping
 */

/** Convert entry handle to entry key */
static inline bdmf_db_key_t dbbe_array_handle_to_key(const bdmf_db_set_t *_this, const bdmf_db_entry_t *entry)
{
    bdmf_db_key_t key;
    
    assert(_this);
    assert(entry);
    assert(_this->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert(_this->entry.data);

    key = entry - (bdmf_db_entry_t *)_this->entry.data;
    if ((unsigned)key >= _this->max_entries)
        return BDMF_DB_KEY_INVAL;

    return key;
}


/** Convert entry key to entry handle */
static inline bdmf_db_entry_t *dbbe_array_key_to_handle(const bdmf_db_set_t *_this, bdmf_db_key_t key)
{
    assert(_this);
    assert(_this->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert(_this->entry.data);

    if ((unsigned long)key >= _this->max_entries)
        return NULL;

    return (bdmf_db_entry_t *)_this->entry.data + key;
}


/** sem-based set read-lock */
static void ddbe_set_lock_read_sem(bdmf_db_set_t *set)
{
    bdmf_ta_mutex_lock(&set->mutex_update);
}


/** sem-based set read-unlock */
static void ddbe_set_unlock_read_sem(bdmf_db_set_t *set)
{
    bdmf_ta_mutex_unlock(&set->mutex_update);
}


/** sem-based set modify-lock */
static void ddbe_set_lock_modify_sem(bdmf_db_set_t *set)
{
    bdmf_ta_mutex_lock(&set->mutex_update);
}


/** sem-based set modify-unlock */
static void ddbe_set_unlock_modify_sem(bdmf_db_set_t *set)
{
    bdmf_ta_mutex_unlock(&set->mutex_update);
}


/** helper function which recursively locks all subsets of SoS
 *  for modify */
static void ddbe_recursive_subsets_lock_modify(bdmf_db_set_t *sos)
{
    int key;
    int left_entries = sos->num_entries;
    bdmf_db_entry_t *entry;
    bdmf_db_set_t *subset;

    assert((sos->entry.flags & BDMF_DB_FLAG_SOS));

    for (key = 0; key < sos->max_entries; ++key)
    {
        entry = sos->key_to_handle(sos, key);

        if ((entry->flags & BDMF_DB_FLAG_VALID))
        {
            subset = (bdmf_db_set_t *)entry->data;
            subset->lock_set_recursively_modify(subset);

            --left_entries;
            /* if we handled all subsets we can break the "for" */
            if (left_entries==0)
            {
                break;
            }
        }
    }
}


/** helper function which recursively unlocks all subsets of SoS
 *  for modify */
static void ddbe_recursive_subsets_unlock_modify(bdmf_db_set_t *sos)
{
    int key;
    int left_entries = sos->num_entries;
    bdmf_db_entry_t *entry;
    bdmf_db_set_t *subset;

    assert((sos->entry.flags & BDMF_DB_FLAG_SOS));

    for (key = 0; key < sos->max_entries; ++key)
    {
        entry = sos->key_to_handle(sos, key);

        if ((entry->flags & BDMF_DB_FLAG_VALID))
        {
            subset = (bdmf_db_set_t *)entry->data;
            subset->unlock_set_recursively_modify(subset);

            --left_entries;
            /* if we handled all subsets we can break the "for" */
            if (left_entries==0)
            {
                break;
            }
        }
    }
}


/** sem-based set modify-lock recursively */
static void ddbe_set_lock_recursively_modify_sem(bdmf_db_set_t *set)
{
    bdmf_ta_mutex_lock(&set->mutex_update);
    if (set->entry.flags & BDMF_DB_FLAG_SOS)
        ddbe_recursive_subsets_lock_modify(set);
}


/** sem-based set modify-unlock recursively */
static void ddbe_set_unlock_recursively_modify_sem(bdmf_db_set_t *set)
{
    if (set->entry.flags & BDMF_DB_FLAG_SOS)
        ddbe_recursive_subsets_unlock_modify(set);
    bdmf_ta_mutex_unlock(&set->mutex_update);
}


/** NB-read-sem-write policy: set read lock */
static void ddbe_set_lock_read__nb_read_sem_write(bdmf_db_set_t *set)
{
    bdmf_fastlock_lock(&set->fastlock);
    ++set->entry.read_count;
    bdmf_fastlock_unlock(&set->fastlock);
}


/** NB-read-sem-write policy: set read unlock */
static void ddbe_set_unlock_read__nb_read_sem_write(bdmf_db_set_t *set)
{
    bdmf_fastlock_lock(&set->fastlock);
    if (!(--set->entry.read_count) &&
        set->entry.write_pending)
    {
        set->entry.write_pending = 0;
        bdmf_mutex_unlock(&set->sem_wait_read_to_finish);
    }
    bdmf_fastlock_unlock(&set->fastlock);
}


/** NB-read-sem-write policy: set modify lock */
static void ddbe_set_lock_modify__nb_read_sem_write(bdmf_db_set_t *set)
{
    bdmf_ta_mutex_lock(&set->mutex_update);
    while(1)
    {
        bdmf_fastlock_lock(&set->fastlock);
        if (!set->entry.read_count)
            break;
        /* Wait until read is completed */
        set->entry.write_pending = 1;
        bdmf_fastlock_unlock(&set->fastlock);
        bdmf_mutex_lock(&set->sem_wait_read_to_finish);
    }
    /* At this point fastlock is taken and there are no pending reads */
    return;
}


/** NB-read-sem-write policy: set modify unlock */
static void ddbe_set_unlock_modify__nb_read_sem_write(bdmf_db_set_t *set)
{
    bdmf_fastlock_unlock(&set->fastlock);
    bdmf_ta_mutex_unlock(&set->mutex_update);
}


/** sem-read/sem-write policy: lock entry */
static void *ddbe_sem_read_sem_write_lock(bdmf_db_set_t *set, bdmf_db_key_t key, int is_deletion)
{
    bdmf_db_entry_t *entry;

    bdmf_ta_mutex_lock(&set->mutex_update);

    /* there is nothing to return in deletion case */
    if (is_deletion)
        return NULL;

    entry = set->key_to_handle(set, key);
    if (!entry || !(entry->flags & BDMF_DB_FLAG_VALID))
    {
        bdmf_ta_mutex_unlock(&set->mutex_update);
        return NULL;
    }
    return entry->data;
}

/** sem-read/sem-write policy: unlock entry */
static void ddbe_sem_read_sem_write_unlock(bdmf_db_set_t *set)
{
    bdmf_ta_mutex_unlock(&set->mutex_update);
}

/** sem-read/sem-write policy: lock entry for reading */
static void *ddbe_sem_read_sem_write_lock_read(bdmf_db_set_t *set, bdmf_db_key_t key)
{
    return ddbe_sem_read_sem_write_lock(set, key, 0) ;
}

/** sem-read/sem-write policy: unlock entry for reading */
static void ddbe_sem_read_sem_write_unlock_read(bdmf_db_set_t *set, bdmf_db_key_t key)
{
    ddbe_sem_read_sem_write_unlock(set);
}

/** sem-read/sem-write policy: lock entry for writing */
static void *ddbe_sem_read_sem_write_lock_write(bdmf_db_set_t *set, bdmf_db_key_t key, int is_deletion)
{
    return ddbe_sem_read_sem_write_lock(set, key, is_deletion) ;
}

/** sem-read/sem-write policy: unlock entry for writing */
static void ddbe_sem_read_sem_write_unlock_write(bdmf_db_set_t *set, int is_deletion)
{
    ddbe_sem_read_sem_write_unlock(set);
}

/** non-blocking-read/shadow write policy: Lock entry for reading */
static void *ddbe_nb_read_shadow_write_lock_read(bdmf_db_set_t *set, bdmf_db_key_t key)
{
    bdmf_db_entry_t *entry;
    bdmf_fastlock_lock(&set->fastlock);
    entry = set->key_to_handle(set, key);
    if (!entry || !(entry->flags & BDMF_DB_FLAG_VALID))
    {
        bdmf_fastlock_unlock(&set->fastlock);
        return NULL;
    }
    ++entry->read_count;
    bdmf_fastlock_unlock(&set->fastlock);
    return entry->data;
}


/** non-blocking-read/shadow write policy: Unlock entry for reading */
static void ddbe_nb_read_shadow_write_unlock_read(bdmf_db_set_t *set, bdmf_db_key_t key)
{
    bdmf_db_entry_t *entry=set->key_to_handle(set, key);
    assert(entry);
    bdmf_fastlock_lock(&set->fastlock);
    /* If write is pending - finish it and release write lock */
    assert(entry->read_count);
    if (!(--entry->read_count) && set->entry.write_pending)
    {
        /* Write was pending. Release write task */
        set->entry.write_pending = 0;
        bdmf_mutex_unlock(&set->sem_wait_read_to_finish);
    }
    bdmf_fastlock_unlock(&set->fastlock);
}

/** non-blocking-read/shadow write policy: Lock entry for
 *  writing/deletion.
 *  returned value of NULL means error only in case that
 *  is_deletion is 0 */
static void *ddbe_nb_read_shadow_write_lock_write(bdmf_db_set_t *set, bdmf_db_key_t key, int is_deletion)
{
    bdmf_db_entry_t *entry;
    bdmf_ta_mutex_lock(&set->mutex_update);

    /* there is nothing to return in deletion case */
    if (is_deletion)
        return NULL;

    /* this check is needed since mutex_update is task-aware.
       it is not allowed for a task to lock for writing a 2nd record before unlocking the first one. 
    */
    if ( set->write_locked_entry )
    {
        bdmf_ta_mutex_unlock(&set->mutex_update);
        return NULL;
    }

    entry = set->key_to_handle(set, key);
    if (!entry || !(entry->flags & BDMF_DB_FLAG_VALID))
    {
        bdmf_ta_mutex_unlock(&set->mutex_update);
        return NULL;
    }
    /* Copy data to shadow entry */
    memcpy(set->shadow_data, entry->data, set->entry_size);
    set->write_locked_entry = entry;
    return set->shadow_data;
}

/** non-blocking-read/shadow write policy: Unlock entry for
 *  writing/deletion */
static void ddbe_nb_read_shadow_write_unlock_write(bdmf_db_set_t *set, int is_deletion)
{
    bdmf_db_entry_t *entry = set->write_locked_entry;
    void *old_data;

    assert(entry);
    while(1)
    {
        bdmf_fastlock_lock(&set->fastlock);
        /* Wait until neither record nor set are locked for reading */
        if (!entry->read_count && !set->entry.read_count)
            break;
        /* Read lock is active. wait */
        set->entry.write_pending = 1;
        bdmf_fastlock_unlock(&set->fastlock);
        bdmf_mutex_lock(&set->sem_wait_read_to_finish);
    }

    /* At this point there is no read lock and fastlock is taken */
    if (is_deletion)
    {
        /* delete the entry */
        set->entry_delete(set, entry);
    }
    else
    {
        /* Exchange record data with shadow and release all locks. */
        old_data = entry->data;
        entry->data = set->shadow_data;
        set->shadow_data = old_data;
        set->write_locked_entry = NULL;
    }
    bdmf_fastlock_unlock(&set->fastlock);
    bdmf_ta_mutex_unlock(&set->mutex_update);
}

/** none policy: set read-lock */
static inline void ddbe_set_lock_read_dummy(bdmf_db_set_t *set)
{
}

/** none policy: set read-unlock */
static inline void ddbe_set_unlock_read_dummy(bdmf_db_set_t *set)
{
}

/** none policy: set modify-lock */
static inline void ddbe_set_lock_modify_dummy(bdmf_db_set_t *set)
{
}

/** none policy: set modify-unlock */
static inline void ddbe_set_unlock_modify_dummy(bdmf_db_set_t *set)
{
}

/** none policy: set modify-lock recursively  */
static void ddbe_set_lock_recursively_modify_dummy(bdmf_db_set_t *set)
{
}

/** none policy: set modify-unlock recursively  */
static void ddbe_set_unlock_recursively_modify_dummy(bdmf_db_set_t *set)
{
}

/** none policy: record lock */
static inline void *ddbe_dummy_lock(bdmf_db_set_t *set, bdmf_db_key_t key, int is_deletion)
{
    bdmf_db_entry_t *entry;

    /* there is nothing to return in deletion case */
    if (is_deletion)
        return NULL;

    entry = set->key_to_handle(set, key);
    if (!entry || !(entry->flags & BDMF_DB_FLAG_VALID))
        return NULL;
    return entry->data;
}

/** none policy: record unlock */
static inline void ddbe_dummy_unlock(bdmf_db_set_t *set)
{
}

/** none policy: record lock for reading */
static inline void *ddbe_dummy_lock_read(bdmf_db_set_t *set, bdmf_db_key_t key)
{
    return ddbe_dummy_lock(set, key, 0);
}

/** none policy: record unlock for reading */
static inline void ddbe_dummy_unlock_read(bdmf_db_set_t *set, bdmf_db_key_t key)
{
    ddbe_dummy_unlock(set);
}

/** none policy: record lock for writing */
static inline void *ddbe_dummy_lock_write(bdmf_db_set_t *set, bdmf_db_key_t key, int is_deletion)
{
    return ddbe_dummy_lock(set, key, is_deletion);
}

/** none policy: record unlock for writing */
static inline void ddbe_dummy_unlock_write(bdmf_db_set_t *set, int is_deletion)
{
    ddbe_dummy_unlock(set);
}



/** Add new sub-set. returns 0 if ok
 * data contains new set handle
 */
static int ddbe_set_new(bdmf_db_set_t *_this, bdmf_db_key_t key, void *data)
{
    bdmf_db_entry_t *entry = _this->key_to_handle(_this, key);
    bdmf_db_set_t *new_set = (bdmf_db_set_t *)data;
    if (!entry)
        return BDMF_ERR_NOENT;
    if ((entry->flags & BDMF_DB_FLAG_VALID))
        return BDMF_ERR_ALREADY;
    ++_this->num_entries;
    entry->data = data;
    entry->flags |= BDMF_DB_FLAG_VALID;
    new_set->my_key = key;
    new_set->parent = _this;
    return 0;
}

/** Add new record. returns 0 if ok
 * data contains record data pointer
 */
static int ddbe_record_new(bdmf_db_set_t *_this, bdmf_db_key_t key, void *data)
{
    bdmf_db_record_t *record = (bdmf_db_record_t *)_this->key_to_handle(_this, key);
    if (!record || !record->e.data)
        return BDMF_ERR_PARM;
    if ((record->e.flags & BDMF_DB_FLAG_VALID))
        return BDMF_ERR_ALREADY;
    ++_this->num_entries;
    memcpy(record->e.data, data, _this->entry_size);
    record->e.flags |= BDMF_DB_FLAG_VALID;
    return 0;
}

/** Delete entry */
static int ddbe_entry_delete(bdmf_db_set_t *_this, bdmf_db_entry_t *entry)
{
    if (!entry)
        return BDMF_ERR_PARM;
    if (!(entry->flags & BDMF_DB_FLAG_VALID))
        return BDMF_ERR_ALREADY;
    entry->flags &= ~BDMF_DB_FLAG_VALID;
    --_this->num_entries;
    return 0;
}


/*
 * External APIs
 */


/** Initialize data base engine
 *
 * \return
 *      0   - OK\n
 *      <0  - error code
 * \ingroup bdmf_dbe
 */
int bdmf_db_module_init(void)
{
    return 0;
}


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
int bdmf_db_make_set_of_sets(const bdmf_db_sos_init_t *init, bdmf_dbset_handle_t *new_set)
{
    bdmf_db_set_t *sos;
    bdmf_db_entry_t *entries;

    /* Parameter check */
    if (!init || !init->name || !new_set)
        return BDMF_ERR_PARM;
    if ((init->backend_type == BDMF_DB_BACKEND_ARRAY) && !init->max_entries)
        return BDMF_ERR_PARM;

    /* Allocate set control block and set records */
    sos = bdmf_calloc(sizeof(bdmf_db_set_t) + strlen(init->name) + 1);
    if (!sos)
        return BDMF_ERR_NOMEM;
    sos->name = (char *)(sos + 1);
    strcpy(sos->name, init->name);
    sos->entry_size = sizeof(bdmf_db_set_t);
    sos->max_entries = init->max_entries;
    sos->my_key = BDMF_DB_KEY_INVAL;
    sos->entry.flags = BDMF_DB_FLAG_SOS;
    sos->magic = BDMF_DB_MAGIC_ACTIVE_SET;

    /* Set backend callbacks */
    switch(init->backend_type)
    {
        case BDMF_DB_BACKEND_ARRAY:
            entries = bdmf_calloc(sizeof(bdmf_db_set_t)*init->max_entries);
            if (!entries)
            {
                bdmf_free(sos);
                return BDMF_ERR_NOMEM;
            }
            sos->entry.data = entries;
            sos->entry_get_next = dbbe_array_entry_get_next;
            sos->handle_to_key = dbbe_array_handle_to_key;
            sos->key_to_handle = dbbe_array_key_to_handle;
            sos->entry_new = ddbe_set_new;
            sos->entry_delete = ddbe_entry_delete;
            break;
            
        default:
            printf("Only array-based DB backend is supported\n");
            bdmf_free(sos);
            return BDMF_ERR_INVALID_OP;
    }

    /* Set locking callbacks. SoS locking policy is always SEMAPHORE */
    /* in SoS, locking for read is same as for write (and is done recursively). */
    sos->lock_set_read = ddbe_set_lock_recursively_modify_sem;
    sos->unlock_set_read = ddbe_set_unlock_recursively_modify_sem;
    sos->lock_set_modify = ddbe_set_lock_modify_sem;
    sos->unlock_set_modify = ddbe_set_unlock_modify_sem;
    sos->lock_set_recursively_modify = ddbe_set_lock_recursively_modify_sem ;
    sos->unlock_set_recursively_modify = ddbe_set_unlock_recursively_modify_sem ;

    /* create mutex_update */
    bdmf_ta_mutex_init(&sos->mutex_update);
    bdmf_fastlock_init(&sos->fastlock);

    *new_set = sos;
    
    return 0;
}




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
int bdmf_db_make_set_of_records(const bdmf_db_sor_init_t *init, int alloc_records, bdmf_dbset_handle_t *new_set)
{
    bdmf_db_set_t *sor;
    bdmf_db_entry_t *entries = NULL;
    void *data = NULL;
    int i;

    /* Parameter check */
    if (!init || !init->name)
        return BDMF_ERR_PARM;
    if ((init->backend_type == BDMF_DB_BACKEND_ARRAY) && !init->max_entries)
        return BDMF_ERR_PARM;
    if (!init->record_size)
        return BDMF_ERR_PARM;

    /* Allocate set control block and set records */
    sor = bdmf_calloc(sizeof(bdmf_db_set_t) + strlen(init->name) + 1);
    if (!sor)
        return BDMF_ERR_NOMEM;
    sor->name = (char *)(sor + 1);
    strcpy(sor->name, init->name);
    sor->entry_size = init->record_size;
    sor->max_entries = init->max_entries;
    sor->my_key = BDMF_DB_KEY_INVAL;
    sor->magic = BDMF_DB_MAGIC_ACTIVE_SET;
    sor->format = init->format;

    /* Set backend callbacks */
    switch(init->backend_type)
    {
        case BDMF_DB_BACKEND_ARRAY:
            entries = bdmf_calloc(sizeof(bdmf_db_entry_t)*init->max_entries);
            if (!entries)
            {
                bdmf_free(sor);
                return BDMF_ERR_NOMEM;
            }
            sor->entry.data = entries;
            sor->entry_get_next = dbbe_array_entry_get_next;
            sor->handle_to_key = dbbe_array_handle_to_key;
            sor->key_to_handle = dbbe_array_key_to_handle;
            sor->entry_new = ddbe_record_new;
            sor->entry_delete = ddbe_entry_delete;

            /* Preallocate data */
            if (alloc_records)
            {
                int size = init->max_entries * init->record_size;
                if (init->lock_policy == BDMF_DB_LOCK_NB_READ_SHADOW_WRITE)
                    size += init->record_size; /* room for shadow entry */
                /* Allocate data + 1 extra for shadow area */
                data = bdmf_calloc(size);
                if (!data)
                {
                    bdmf_free(entries);
                    bdmf_free(sor);
                    return BDMF_ERR_NOMEM;
                }
                for(i=0; i<init->max_entries; i++)
                {
                    bdmf_db_entry_t *entry = (bdmf_db_entry_t *)sor->entry.data + i;
                    entry->data = data + i * init->record_size;
                }
                if (init->lock_policy == BDMF_DB_LOCK_NB_READ_SHADOW_WRITE)
                    sor->shadow_data = data + i * init->record_size;
            }

            /* Initialize records */
            for(i=0; i<init->max_entries; i++)
            {
                bdmf_db_entry_t *entry = (bdmf_db_entry_t *)sor->entry.data + i;
                entry->flags = BDMF_DB_FLAG_RECORD;
            }
            break;

        default:
            printf("Only array-based DB backend is supported\n");
            bdmf_free(sor);
            return BDMF_ERR_INVALID_OP;
    }

    /* Set locking callbacks based on locking policy */
    switch(init->lock_policy)
    {
        case BDMF_DB_LOCK_SEM_READ_SEM_WRITE:
            sor->lock_record_write = ddbe_sem_read_sem_write_lock_write;
            sor->lock_record_read = ddbe_sem_read_sem_write_lock_read;
            sor->unlock_record_write = ddbe_sem_read_sem_write_unlock_write;
            sor->unlock_record_read = ddbe_sem_read_sem_write_unlock_read;
            sor->lock_set_read = ddbe_set_lock_read_sem;
            sor->unlock_set_read = ddbe_set_unlock_read_sem;
            sor->lock_set_modify = ddbe_set_lock_modify_sem;
            sor->unlock_set_modify = ddbe_set_unlock_modify_sem;
            sor->lock_set_recursively_modify = ddbe_set_lock_recursively_modify_sem ;
            sor->unlock_set_recursively_modify = ddbe_set_unlock_recursively_modify_sem ;
            break;

        case BDMF_DB_LOCK_NONE:
        case BDMF_DB_LOCK_OTHER:
            sor->lock_record_write = ddbe_dummy_lock_write;
            sor->lock_record_read = ddbe_dummy_lock_read;
            sor->unlock_record_write = ddbe_dummy_unlock_write;
            sor->unlock_record_read = ddbe_dummy_unlock_read;
            sor->lock_set_read = ddbe_set_lock_read_dummy;
            sor->unlock_set_read = ddbe_set_unlock_read_dummy;
            sor->lock_set_modify = ddbe_set_lock_modify_dummy;
            sor->unlock_set_modify = ddbe_set_unlock_modify_dummy;
            sor->lock_set_recursively_modify = ddbe_set_lock_recursively_modify_dummy ;
            sor->unlock_set_recursively_modify = ddbe_set_unlock_recursively_modify_dummy ;
            break;

        case BDMF_DB_LOCK_NB_READ_SHADOW_WRITE:
            sor->lock_record_write = ddbe_nb_read_shadow_write_lock_write;
            sor->lock_record_read = ddbe_nb_read_shadow_write_lock_read;
            sor->unlock_record_write = ddbe_nb_read_shadow_write_unlock_write;
            sor->unlock_record_read = ddbe_nb_read_shadow_write_unlock_read;
            sor->lock_set_read = ddbe_set_lock_read__nb_read_sem_write;
            sor->unlock_set_read = ddbe_set_unlock_read__nb_read_sem_write;
            sor->lock_set_modify = ddbe_set_lock_modify__nb_read_sem_write;
            sor->unlock_set_modify = ddbe_set_unlock_modify__nb_read_sem_write;
            sor->lock_set_recursively_modify = ddbe_set_lock_recursively_modify_sem ;
            sor->unlock_set_recursively_modify = ddbe_set_unlock_recursively_modify_sem ;
            break;

        default:
            printf("Lock policy %d is not supported\n", init->lock_policy);
            if (data)
                bdmf_free(data);
            if (entries)
                bdmf_free(entries);
            bdmf_free(sor);
            return BDMF_ERR_INVALID_OP;
    }

    /* create mutex_update */
    bdmf_ta_mutex_init(&sor->mutex_update);
    /* create sem_wait_read_to_finish. it is initialized to be taken */
    bdmf_mutex_init(&sor->sem_wait_read_to_finish);
    bdmf_mutex_lock(&sor->sem_wait_read_to_finish);

    bdmf_fastlock_init(&sor->fastlock);

    *new_set = sor;

    return 0;
}


/** Lock data set for reading. When set is locked - it can't be
 *  modified.
 *
 * \param[in]   set             data base set to be locked
 *
 * \ingroup bdmf_dbe
 */
void bdmf_dbset_lock_read(bdmf_dbset_handle_t set)
{
    assert(set);
    assert(set->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert(set->lock_set_read);
    set->lock_set_read(set);
}


/** Release data set lock
 *
 * Unlock set locked by \ref bdmf_dbset_lock_read
 *
 * \param[in]   set             data base set to be unlocked
 *
 * \ingroup bdmf_dbe
 */
void bdmf_dbset_unlock_read(bdmf_dbset_handle_t set)
{
    assert(set);
    assert(set->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert(set->unlock_set_read);
    set->unlock_set_read(set);
}


/** Lock data set for modify. If the set is SoS, the locking
 *  will be recursive.
 *
 * \param[in]   set             data base set to be locked
 *
 * \ingroup bdmf_dbe
 */
void bdmf_dbset_lock_modify(bdmf_dbset_handle_t set)
{
    assert(set);
    assert(set->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert(set->lock_set_recursively_modify);
    set->lock_set_recursively_modify(set);
}


/** Release data set lock
 *
 * Unlock set locked by \ref bdmf_dbset_lock_modify
 *
 * \param[in]   set             data base set to be unlocked
 *
 * \ingroup bdmf_dbe
 */
void bdmf_dbset_unlock_modify(bdmf_dbset_handle_t set)
{
    assert(set);
    assert(set->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert(set->unlock_set_recursively_modify);
    set->unlock_set_recursively_modify(set);
}


/** Add set to the parent set with specific key.
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
 * \ingroup bdmf_dbe
 */
int bdmf_dbset_add(bdmf_dbset_handle_t sos, bdmf_db_key_t key, bdmf_dbset_handle_t new_set)
{
    int rc;
    assert(sos);
    assert(sos->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert((sos->entry.flags & BDMF_DB_FLAG_SOS));
    assert(new_set);
    assert(new_set->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert(new_set->my_key == BDMF_DB_KEY_INVAL);
    sos->lock_set_modify(sos);
    rc = sos->entry_new(sos, key, new_set);
    sos->unlock_set_modify(sos);
    return rc;
}


/** Get set handle given its key.
 *
 * \param[in]   sos             parent set of sets
 * \param[in]   key             set key.
 * \return
 *      !=0 - set handle
 *      NULL- doesn't exist
 * \ingroup bdmf_dbe
 */
bdmf_dbset_handle_t bdmf_dbset_handle(const bdmf_dbset_handle_t sos, bdmf_db_key_t key)
{
    bdmf_db_entry_t *entry;
    assert(sos);
    assert(sos->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert((sos->entry.flags & BDMF_DB_FLAG_SOS));
    entry = sos->key_to_handle(sos, key);
    if (!entry || !(entry->flags & BDMF_DB_FLAG_VALID))
        return NULL;
    return (bdmf_dbset_handle_t)entry->data;
}


/** Get set key given its handle.
 *
 * \param[in]   set             set handle
 * \param[in]   key             set key.
 * \return
 *      !=BDMF_DB_KEY_INVAL - set key
 *      BDMF_DB_KEY_INVAL - error
 * \ingroup bdmf_dbe
 */
bdmf_db_key_t bdmf_dbset_key(const bdmf_dbset_handle_t set)
{
    assert(set);
    assert(set->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    return set->my_key;
}


/** Get set name
 *
 * \param[in]   set             set handle
 * \return set name
 * \ingroup bdmf_dbe
 */
const char *bdmf_dbset_name(const bdmf_dbset_handle_t set)
{
    assert(set);
    assert(set->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    return set->name;
}


/** Get number of records in the set.
 *
 * \param[in]   set             set handle
 * \return number of active records in the set
 * \ingroup bdmf_dbe
 */
int bdmf_dbset_num_records(const bdmf_dbset_handle_t set)
{
    assert(set);
    assert(set->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    return set->num_entries;
}


/** Get entry size
 *
 * \param[in]   set             set handle
 * \return set entry size
 * \ingroup bdmf_dbe
 */
int bdmf_dbset_entry_size(const bdmf_dbset_handle_t set)
{
    assert(set);
    assert(set->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    return set->entry_size;
}


/** Add record to the parent set with specific key.
 *
 * The function creates a new record and adds it to the parent set with specific key.
 * The function automatically acquires modify lock and releases it
 * in the end of operation.
 * \param[in]   sor             parent set of records
 * \param[in]   key             key to add new set at
 * \param[in]   data            record data. Data size is defined at parent SOR registration time.
 * \param[out]  p_record        new record handle
 * \return
 *      =0  - OK\n
 *      <0  - error code
 * \ingroup bdmf_dbe
 */
int bdmf_dbrecord_add(bdmf_dbset_handle_t sor, bdmf_db_key_t key, const void *data)
{
    int rc;

    assert(sor);
    assert(data);
    assert(sor->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert((sor->entry.flags & BDMF_DB_FLAG_SOS)==0);

    sor->lock_set_modify(sor);
    rc=sor->entry_new(sor, key, (void *)data);
    sor->unlock_set_modify(sor);
    return rc;
}


/** Delete record from the parent SoR given the record key.
 *
 * The function automatically acquires modify lock and releases it
 * in the end of operation.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key.
 * \ingroup bdmf_dbe
 */
void bdmf_dbrecord_delete(bdmf_dbset_handle_t sor, bdmf_db_key_t key)
{
    sor->lock_record_write(sor, key, 1);
    sor->unlock_record_write(sor, 1);
}


/** Get record data pointer without locking.
 *
 * The function returns pointer to data structure stored in data base record.\n
 * Attention! The caller is required to aquire read or write lock - as appropriate
 * before calling this function and release the lock when processing is finished.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 * \return
 *      data pointer. NULL if there is no record matching the key.
 * \ingroup bdmf_dbe
 */
void *bdmf_dbrecord_getraw_nolock(bdmf_dbset_handle_t sor, bdmf_db_key_t key)
{
    bdmf_db_entry_t *entry;
    assert(sor);
    assert(sor->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert((sor->entry.flags & BDMF_DB_FLAG_SOS)==0);
    entry = sor->key_to_handle(sor, key);
    if (!entry || !(entry->flags & BDMF_DB_FLAG_VALID))
        return NULL;
    return entry->data;
}


/** Lock record for reading and return record data pointer.
 *
 * The function aquires read-lock and returns pointer to data structure stored in data base record.\n
 * read-lock must be released separately when the pointer is no longer in use.
 * Note that the default record-read lock is non-blocking and counting.
 * That means that multiple processes cam read-lock the same record without blocking.
 * The function is low-level. It is recommended to use macro \ref bdmf_dbrecord_get_read instead.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 * \return
 *      data pointer. NULL if there is no record matching the key.
 * \ingroup bdmf_dbe
 */
const void *bdmf_dbrecord_getraw_read(bdmf_dbset_handle_t sor, bdmf_db_key_t key)
{
    assert(sor);
    assert(sor->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert((sor->entry.flags & BDMF_DB_FLAG_SOS)==0);
    return sor->lock_record_read(sor, key);
}


/** Unlock record locked for reading.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 *
 * \ingroup bdmf_dbe
 */
void bdmf_dbrecord_unlock_read(bdmf_dbset_handle_t sor, bdmf_db_key_t key)
{
    assert(sor);
    assert(sor->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert((sor->entry.flags & BDMF_DB_FLAG_SOS)==0);
    sor->unlock_record_read(sor, key);
}


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
 * \ingroup bdmf_dbe
 */
int bdmf_dbrecord_read(bdmf_dbset_handle_t sor, bdmf_db_key_t key, int offset, int size, void *data)
{
    const void *d = bdmf_dbrecord_getraw_read(sor, key);
    if (!d)
        return BDMF_ERR_PARM;
    if ((unsigned)offset + (unsigned)size > sor->entry_size)
    {
        bdmf_dbrecord_unlock_read(sor, key);
        return BDMF_ERR_PARM;
    }
    memcpy(data, (char *)d+(unsigned)offset, (unsigned)size);
    bdmf_dbrecord_unlock_read(sor, key);
    return 0;
}


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
 * \ingroup bdmf_dbe
 */
void *bdmf_dbrecord_getraw_write(bdmf_dbset_handle_t sor, bdmf_db_key_t key)
{
    assert(sor);
    assert(sor->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert((sor->entry.flags & BDMF_DB_FLAG_SOS)==0);
    return sor->lock_record_write(sor, key, 0);
}


/** Unlock record locked for writing.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 *
 * \ingroup bdmf_dbe
 */
void bdmf_dbrecord_unlock_write(bdmf_dbset_handle_t sor, bdmf_db_key_t key)
{
    assert(sor);
    assert(sor->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert((sor->entry.flags & BDMF_DB_FLAG_SOS)==0);
    sor->unlock_record_write(sor, 0);
}


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
 * \ingroup bdmf_dbe
 */
int bdmf_dbrecord_write(bdmf_dbset_handle_t sor, bdmf_db_key_t key, int offset, int size, const void *data)
{
    void *d=bdmf_dbrecord_getraw_write(sor, key);
    if (!d)
        return BDMF_ERR_PARM;
    if ((unsigned)offset + (unsigned)size > sor->entry_size)
    {
        bdmf_dbrecord_unlock_write(sor, key);
        return BDMF_ERR_PARM;
    }
    memcpy((char *)d+(unsigned)offset, data, (unsigned)size);
    bdmf_dbrecord_unlock_write(sor, key);
    return 0;
}


/** Register notification function to get informed
 * when data base set is modified.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   cb              callback function pointer
 * \param[in]   cb_priv         private data that should be passed to the callback
 * \return
 *      =0  - OK\n
 *      <0  - error code
 * \ingroup bdmf_dbe
 */
int bdmf_dbset_notify_register(bdmf_dbset_handle_t sor, bdmf_db_notify_cb_t cb, long cb_priv)
{
    bdmf_db_notify_t *nf_new, *nf, *nf_prev = NULL;
    
    assert(sor);
    assert(sor->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    assert(cb);

    nf_new = bdmf_calloc(sizeof(bdmf_db_notify_t));
    if (!nf_new)
        return BDMF_ERR_NOMEM;
    nf_new->cb = cb;
    nf_new->cb_priv = cb_priv;

    /* Add to set's notification list */
    nf = sor->notify_list_head;
    while(nf)
    {
        nf_prev = nf;
        nf = nf->next;
    }
    if (nf_prev)
        nf_prev->next = nf_new;
    else
        sor->notify_list_head = nf_new;
    
    return 0;
}




/** Data base iterator
 *
 * \param[in]   set             data base set
 * \param[in]   prev            last entry. BDMF_DB_KEY_ANY=start from the beginning
 * \return  data base entry key following prev or BDMF_DB_KEY_NO_MORE if end is reached.\n
 *          BDMF_DB_KEY_INVAL is reqturned if prev key is invalid
 * \ingroup bdmf_dbe
 */
bdmf_db_key_t bdmf_dbset_iterate(bdmf_dbset_handle_t set, bdmf_db_key_t prev)
{
    assert(set);
    assert(set->magic == BDMF_DB_MAGIC_ACTIVE_SET);
    return set->entry_get_next(set, prev);
}


/* Print database structure */
static void _bdmf_db_print_structure(const bdmf_dbset_handle_t set, int level)
{
    int i;

    if (!set)
        return;
    
    /* Indentation */
    for(i=0; i<level; i++)
        printf("\t");

    if ((set->entry.flags & BDMF_DB_FLAG_SOS))
    {
        bdmf_db_key_t key = bdmf_dbset_iterate(set, BDMF_DB_KEY_ANY);
        printf("%-16s SoS max_entries=%d entries=%d\n", set->name, set->max_entries, set->num_entries);
        while(key >= 0)
        {
            _bdmf_db_print_structure(bdmf_dbset_handle(set, key), level+1);
            key = bdmf_dbset_iterate(set, key);
        }
    }
    else
    {
        printf("%-16s SoR max_entries=%d entries=%d record_size=%d total_size=%d\n",
               set->name, set->max_entries, set->num_entries, set->entry_size,
               set->entry_size*set->max_entries);
    }
}


/** Print database structure.
 *
 * \param[in]   set             root set
 * \ingroup bdmf_dbe
 */
void bdmf_dbset_print_structure(const bdmf_dbset_handle_t set)
{
    _bdmf_db_print_structure(set, 0);
}


/** Format record for printing.
 *
 * The function converts record data to human-readible format.
 *
 * \param[in]   sor             parent set of records
 * \param[in]   key             record key
 * \param[out]  buffer          output buffer
 * \param[in]   size            buffer size
 * \return
 *      >=0-amount of data placed in the buffer\n
 *      <0-error code
 */
int bdmf_dbrecord_read_formatted(bdmf_dbset_handle_t sor, bdmf_db_key_t key, char *buffer, int size)
{
    const void *data;
    int len;
    if (!buffer || !size)
        return BDMF_ERR_PARM;
    if (!sor->format)
        return BDMF_ERR_INVALID_OP;
    *buffer=0;
    data = bdmf_dbrecord_getraw_read(sor, key);
    if (!data)
        return 0;
    len = sor->format(data, buffer, size);
    bdmf_dbrecord_unlock_read(sor, key);
    return len;
}
