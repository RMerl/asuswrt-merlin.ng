/* 
 * The PJLIB's timer heap is based (or more correctly, copied and modied)
 * from ACE library by Douglas C. Schmidt. ACE is an excellent OO framework
 * that implements many core patterns for concurrent communication software.
 * If you're looking for C++ alternative of PJLIB, then ACE is your best
 * solution.
 *
 * You may use this file according to ACE open source terms or PJLIB open
 * source terms. You can find the fine ACE library at:
 *  http://www.cs.wustl.edu/~schmidt/ACE.html
 *
 * ACE is Copyright (C)1993-2006 Douglas C. Schmidt <d.schmidt@vanderbilt.edu>
 *
 * GNU Public License:
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
#include <pj/timer.h>
#include <pj/pool.h>
#include <pj/os.h>
#include <pj/string.h>
#include <pj/assert.h>
#include <pj/errno.h>
#include <pj/lock.h>
#include <pj/log.h>
#include <pj/rand.h>
#include <pj/limits.h>

#define THIS_FILE       "timer.c"

#define HEAP_PARENT(X)  (X == 0 ? 0 : (((X) - 1) / 2))
#define HEAP_LEFT(X)    (((X)+(X))+1)


#define DEFAULT_MAX_TIMED_OUT_PER_POLL  (64)

/* Enable this to raise assertion in order to catch bug of timer entry
 * which has been deallocated without being cancelled. If disabled,
 * the timer heap will simply remove the destroyed entry (and print log)
 * and resume normally.
 * This setting only works if PJ_TIMER_USE_COPY is enabled.
 */
#define ASSERT_IF_ENTRY_DESTROYED (PJ_TIMER_USE_COPY? 0: 0)


enum
{
    F_DONT_CALL = 1,
    F_DONT_ASSERT = 2,
    F_SET_ID = 4
};

#if PJ_TIMER_USE_COPY

/* Duplicate/copy of the timer entry. */
typedef struct pj_timer_entry_dup
{
#if PJ_TIMER_USE_LINKED_LIST
    /**
    * Standard list members.
    */
    PJ_DECL_LIST_MEMBER(struct pj_timer_entry_dup);
#endif

    /**
     * The duplicate copy.
     */
    pj_timer_entry  dup;

    /**
     * Pointer of the original timer entry.
     */
    pj_timer_entry *entry;

    /** 
     * The future time when the timer expires, which the value is updated
     * by timer heap when the timer is scheduled.
     */
    pj_time_val     _timer_value;

    /**
     * Internal: the group lock used by this entry, set when
     * pj_timer_heap_schedule_w_lock() is used.
     */
    pj_grp_lock_t  *_grp_lock;

#if PJ_TIMER_DEBUG
    const char     *src_file;
    int             src_line;
#endif

} pj_timer_entry_dup;

#define GET_TIMER(ht, node) &ht->timer_dups[node->_timer_id]
#define GET_ENTRY(node) node->entry
#define GET_FIELD(node, _timer_id) node->dup._timer_id

#else

typedef pj_timer_entry pj_timer_entry_dup;

#define GET_TIMER(ht, node) node
#define GET_ENTRY(node) node
#define GET_FIELD(node, _timer_id) node->_timer_id

#endif

/**
 * The implementation of timer heap.
 */
struct pj_timer_heap_t
{
    /** Pool from which the timer heap resize will get the storage from */
    pj_pool_t *pool;

    /** Maximum size of the heap. */
    pj_size_t max_size;

    /** Current size of the heap. */
    pj_size_t cur_size;

    /** Max timed out entries to process per poll. */
    unsigned max_entries_per_poll;

    /** Lock object. */
    pj_lock_t *lock;

    /** Autodelete lock. */
    pj_bool_t auto_delete_lock;

    /**
     * Current contents of the Heap, which is organized as a "heap" of
     * pj_timer_entry *'s.  In this context, a heap is a "partially
     * ordered, almost complete" binary tree, which is stored in an
     * array.
     */
    pj_timer_entry_dup **heap;

#if PJ_TIMER_USE_LINKED_LIST
    /**
    * If timer heap uses linked list, then this will represent the head of
    * the list.
    */
    pj_timer_entry_dup head_list;
#endif

    /**
     * An array of "pointers" that allows each pj_timer_entry in the
     * <heap_> to be located in O(1) time.  Basically, <timer_id_[i]>
     * contains the slot in the <heap_> array where an pj_timer_entry
     * with timer id <i> resides.  Thus, the timer id passed back from
     * <schedule_entry> is really an slot into the <timer_ids> array.  The
     * <timer_ids_> array serves two purposes: negative values are
     * treated as "pointers" for the <freelist_>, whereas positive
     * values are treated as "pointers" into the <heap_> array.
     */
    pj_timer_id_t *timer_ids;

    /**
     * An array of timer entry copies.
     */
    pj_timer_entry_dup *timer_dups;

    /**
     * "Pointer" to the first element in the freelist contained within
     * the <timer_ids_> array, which is organized as a stack.
     */
    pj_timer_id_t timer_ids_freelist;

    /** Callback to be called when a timer expires. */
    pj_timer_heap_callback *callback;

};



PJ_INLINE(void) lock_timer_heap( pj_timer_heap_t *ht )
{
    if (ht->lock) {
        pj_lock_acquire(ht->lock);
    }
}

PJ_INLINE(void) unlock_timer_heap( pj_timer_heap_t *ht )
{
    if (ht->lock) {
        pj_lock_release(ht->lock);
    }
}


static void copy_node( pj_timer_heap_t *ht, pj_size_t slot, 
                       pj_timer_entry_dup *moved_node )
{
    PJ_CHECK_STACK();

    // Insert <moved_node> into its new location in the heap.
    ht->heap[slot] = moved_node;

    // Update the corresponding slot in the parallel <timer_ids_> array.
    ht->timer_ids[GET_FIELD(moved_node, _timer_id)] = (int)slot;
}

static pj_timer_id_t pop_freelist( pj_timer_heap_t *ht )
{
    // We need to truncate this to <int> for backwards compatibility.
    pj_timer_id_t new_id = ht->timer_ids_freelist;

    PJ_CHECK_STACK();

    // The freelist values in the <timer_ids_> are negative, so we need
    // to negate them to get the next freelist "pointer."
    ht->timer_ids_freelist =
        -ht->timer_ids[ht->timer_ids_freelist];

    return new_id;

}

static void push_freelist (pj_timer_heap_t *ht, pj_timer_id_t old_id)
{
    PJ_CHECK_STACK();

    // The freelist values in the <timer_ids_> are negative, so we need
    // to negate them to get the next freelist "pointer."
    ht->timer_ids[old_id] = -ht->timer_ids_freelist;
    ht->timer_ids_freelist = old_id;
}


static void reheap_down(pj_timer_heap_t *ht, pj_timer_entry_dup *moved_node,
                        size_t slot, size_t child)
{
    PJ_CHECK_STACK();

    // Restore the heap property after a deletion.

    while (child < ht->cur_size)
    {
        // Choose the smaller of the two children.
        if (child + 1 < ht->cur_size &&
            PJ_TIME_VAL_LT(ht->heap[child + 1]->_timer_value,
                           ht->heap[child]->_timer_value))
        {
            child++;
        }

        // Perform a <copy> if the child has a larger timeout value than
        // the <moved_node>.
        if (PJ_TIME_VAL_LT(ht->heap[child]->_timer_value,
                           moved_node->_timer_value))
        {
            copy_node( ht, slot, ht->heap[child]);
            slot = child;
            child = HEAP_LEFT(child);
        }
        else
            // We've found our location in the heap.
            break;
    }

    copy_node( ht, slot, moved_node);
}

static void reheap_up( pj_timer_heap_t *ht, pj_timer_entry_dup *moved_node,
                       size_t slot, size_t parent)
{
    // Restore the heap property after an insertion.

    while (slot > 0)
    {
        // If the parent node is greater than the <moved_node> we need
        // to copy it down.
        if (PJ_TIME_VAL_LT(moved_node->_timer_value,
                           ht->heap[parent]->_timer_value))
        {
            copy_node(ht, slot, ht->heap[parent]);
            slot = parent;
            parent = HEAP_PARENT(slot);
        }
        else
            break;
    }

    // Insert the new node into its proper resting place in the heap and
    // update the corresponding slot in the parallel <timer_ids> array.
    copy_node(ht, slot, moved_node);
}


static pj_timer_entry_dup * remove_node( pj_timer_heap_t *ht, size_t slot)
{
    pj_timer_entry_dup *removed_node = ht->heap[slot];

    // Return this timer id to the freelist.
    push_freelist( ht, GET_FIELD(removed_node, _timer_id) );

    // Decrement the size of the heap by one since we're removing the
    // "slot"th node.
    ht->cur_size--;

    // Set the ID
    if (GET_FIELD(removed_node, _timer_id) !=
        GET_ENTRY(removed_node)->_timer_id)
    {
#if PJ_TIMER_DEBUG
        PJ_LOG(3,(THIS_FILE, "Bug! Trying to remove entry %p from %s "
                             "line %d, which has been deallocated "
                             "without being cancelled",
                             GET_ENTRY(removed_node),
                             removed_node->src_file,
                             removed_node->src_line));
#else
        PJ_LOG(3,(THIS_FILE, "Bug! Trying to remove entry %p "
                             "which has been deallocated "
                             "without being cancelled",
                             GET_ENTRY(removed_node)));
#endif
#if ASSERT_IF_ENTRY_DESTROYED
        pj_assert(removed_node->dup._timer_id==removed_node->entry->_timer_id);
#endif
    }
    GET_ENTRY(removed_node)->_timer_id = -1;
    GET_FIELD(removed_node, _timer_id) = -1;

#if !PJ_TIMER_USE_LINKED_LIST
    // Only try to reheapify if we're not deleting the last entry.

    if (slot < ht->cur_size)
    {
        pj_size_t parent;
        pj_timer_entry_dup *moved_node = ht->heap[ht->cur_size];

        // Move the end node to the location being removed and update
        // the corresponding slot in the parallel <timer_ids> array.
        copy_node( ht, slot, moved_node);

        // If the <moved_node->time_value_> is great than or equal its
        // parent it needs be moved down the heap.
        parent = HEAP_PARENT (slot);

        if (PJ_TIME_VAL_GTE(moved_node->_timer_value,
                            ht->heap[parent]->_timer_value))
        {
            reheap_down( ht, moved_node, slot, HEAP_LEFT(slot));
        } else {
            reheap_up( ht, moved_node, slot, parent);
        }
    }
#else
    pj_list_erase(removed_node);
#endif

    return removed_node;
}

static pj_status_t grow_heap(pj_timer_heap_t *ht)
{
    // All the containers will double in size from max_size_
    size_t new_size = ht->max_size * 2;
#if PJ_TIMER_USE_COPY
    pj_timer_entry_dup *new_timer_dups = 0;
#endif
    pj_timer_id_t *new_timer_ids;
    pj_size_t i;
    pj_timer_entry_dup **new_heap = 0;

#if PJ_TIMER_USE_LINKED_LIST
    pj_timer_entry_dup *tmp_dup = NULL;
    pj_timer_entry_dup *new_dup;
#endif

    PJ_LOG(6,(THIS_FILE, "Growing heap size from %lu to %lu",
                         (unsigned long)ht->max_size,
                         (unsigned long)new_size));

    // First grow the heap itself.
    new_heap = (pj_timer_entry_dup**) 
               pj_pool_calloc(ht->pool, new_size, sizeof(pj_timer_entry_dup*));
    if (!new_heap)
        return PJ_ENOMEM;

#if PJ_TIMER_USE_COPY
    // Grow the array of timer copies.

    new_timer_dups = (pj_timer_entry_dup*) 
                     pj_pool_alloc(ht->pool,
                                   sizeof(pj_timer_entry_dup) * new_size);
    if (!new_timer_dups)
        return PJ_ENOMEM;

    memcpy(new_timer_dups, ht->timer_dups,
           ht->max_size * sizeof(pj_timer_entry_dup));
    for (i = 0; i < ht->cur_size; i++) {
        int idx = (int)(ht->heap[i] - ht->timer_dups);
        // Point to the address in the new array
        pj_assert(idx >= 0 && idx < (int)ht->max_size);
        new_heap[i] = &new_timer_dups[idx];
    }
    ht->timer_dups = new_timer_dups;
#else
    memcpy(new_heap, ht->heap, ht->max_size * sizeof(pj_timer_entry *));
#endif

#if PJ_TIMER_USE_LINKED_LIST
    tmp_dup = ht->head_list.next;
    pj_list_init(&ht->head_list);
    for (; tmp_dup != &ht->head_list; tmp_dup = tmp_dup->next)
    {
        int slot = ht->timer_ids[GET_FIELD(tmp_dup, _timer_id)];
        new_dup = new_heap[slot];
        pj_list_push_back(&ht->head_list, new_dup);
    }
#endif

    ht->heap = new_heap;

    // Grow the array of timer ids.

    new_timer_ids = 0;
    new_timer_ids = (pj_timer_id_t*)
                    pj_pool_alloc(ht->pool, new_size * sizeof(pj_timer_id_t));
    if (!new_timer_ids)
        return PJ_ENOMEM;

    memcpy( new_timer_ids, ht->timer_ids, ht->max_size * sizeof(pj_timer_id_t));

    //delete [] timer_ids_;
    ht->timer_ids = new_timer_ids;

    // And add the new elements to the end of the "freelist".
    for (i = ht->max_size; i < new_size; i++)
        ht->timer_ids[i] = -((pj_timer_id_t) (i + 1));

    ht->max_size = new_size;

    return PJ_SUCCESS;
}

static pj_status_t insert_node(pj_timer_heap_t *ht,
                               pj_timer_entry *new_node,
                               const pj_time_val *future_time)
{
    pj_timer_entry_dup *timer_copy;

#if PJ_TIMER_USE_LINKED_LIST
    pj_timer_entry_dup *tmp_node = NULL;
#endif

    if (ht->cur_size + 2 >= ht->max_size) {
        pj_status_t status = grow_heap(ht);
        if (status != PJ_SUCCESS)
            return status;
    }

    timer_copy = GET_TIMER(ht, new_node);
#if PJ_TIMER_USE_COPY
    // Create a duplicate of the timer entry.
    pj_bzero(timer_copy, sizeof(*timer_copy));
    pj_memcpy(&timer_copy->dup, new_node, sizeof(*new_node));
    timer_copy->entry = new_node;
#endif

#if PJ_TIMER_USE_LINKED_LIST
    pj_list_init(timer_copy);
#endif

    timer_copy->_timer_value = *future_time;

#if !PJ_TIMER_USE_LINKED_LIST
    reheap_up(ht, timer_copy, ht->cur_size, HEAP_PARENT(ht->cur_size));
#else
    if (ht->cur_size == 0) {
        pj_list_push_back(&ht->head_list, timer_copy);
    } else if (PJ_TIME_VAL_GTE(*future_time,
                               ht->head_list.prev->_timer_value))
    {
        /* Insert the max value to the end of the list. */
        pj_list_insert_before(&ht->head_list, timer_copy);
    } else {
        tmp_node = ht->head_list.next;
        while (tmp_node->next != &ht->head_list &&
               PJ_TIME_VAL_GT(*future_time, tmp_node->_timer_value))
        {
            tmp_node = tmp_node->next;
        }
        if (PJ_TIME_VAL_LT(*future_time, tmp_node->_timer_value)) {
            pj_list_insert_before(tmp_node, timer_copy);
        } else {
            pj_list_insert_after(tmp_node, timer_copy);
        }
    }
    copy_node(ht, new_node->_timer_id-1, timer_copy);
#endif
    ht->cur_size++;

    return PJ_SUCCESS;
}


static pj_status_t schedule_entry( pj_timer_heap_t *ht,
                                   pj_timer_entry *entry, 
                                   const pj_time_val *future_time )
{
    if (ht->cur_size < ht->max_size)
    {
        // Obtain the next unique sequence number.
        // Set the entry
        entry->_timer_id = pop_freelist(ht);

        return insert_node( ht, entry, future_time );
    }
    else
        return -1;
}


static int cancel( pj_timer_heap_t *ht, 
                   pj_timer_entry *entry, 
                   unsigned flags)
{
    long timer_node_slot;

    PJ_CHECK_STACK();

    // Check to see if the timer_id is out of range.
    // Moved to cancel_timer() as it needs to validate _timer_id earlier
    /*
    if (entry->_timer_id < 1 || (pj_size_t)entry->_timer_id >= ht->max_size) {
        entry->_timer_id = -1;
        return 0;
    }
    */

    timer_node_slot = ht->timer_ids[entry->_timer_id];

    if (timer_node_slot < 0) { // Check to see if timer_id is still valid.
        entry->_timer_id = -1;
        return 0;
    }

    if (entry != GET_ENTRY(ht->heap[timer_node_slot])) {
        if ((flags & F_DONT_ASSERT) == 0)
            pj_assert(entry == GET_ENTRY(ht->heap[timer_node_slot]));
        entry->_timer_id = -1;
        return 0;
    } else {
        remove_node( ht, timer_node_slot);

        if ((flags & F_DONT_CALL) == 0) {
            // Call the close hook.
            (*ht->callback)(ht, entry);
        }
        return 1;
    }
}


/*
 * Calculate memory size required to create a timer heap.
 */
PJ_DEF(pj_size_t) pj_timer_heap_mem_size(pj_size_t count)
{
    return /* size of the timer heap itself: */
           sizeof(pj_timer_heap_t) + 
           /* size of each entry: */
           (count+2) * (sizeof(pj_timer_entry_dup*)+sizeof(pj_timer_id_t)+
           sizeof(pj_timer_entry_dup)) +
           /* lock, pool etc: */
           132;
}

/*
 * Create a new timer heap.
 */
PJ_DEF(pj_status_t) pj_timer_heap_create( pj_pool_t *pool,
                                          pj_size_t size,
                                          pj_timer_heap_t **p_heap)
{
    pj_timer_heap_t *ht;
    pj_size_t i;

    PJ_ASSERT_RETURN(pool && p_heap, PJ_EINVAL);

    *p_heap = NULL;

    /* Magic? */
    size += 2;

    /* Allocate timer heap data structure from the pool */
    ht = PJ_POOL_ZALLOC_T(pool, pj_timer_heap_t);
    if (!ht)
        return PJ_ENOMEM;

    /* Initialize timer heap sizes */
    ht->max_size = size;
    ht->cur_size = 0;
    ht->max_entries_per_poll = DEFAULT_MAX_TIMED_OUT_PER_POLL;
    ht->timer_ids_freelist = 1;
    ht->pool = pool;

    /* Lock. */
    ht->lock = NULL;
    ht->auto_delete_lock = 0;

    // Create the heap array.
    ht->heap = (pj_timer_entry_dup**)
               pj_pool_calloc(pool, size, sizeof(pj_timer_entry_dup*));
    if (!ht->heap)
        return PJ_ENOMEM;

#if PJ_TIMER_USE_COPY
    // Create the timer entry copies array.
    ht->timer_dups = (pj_timer_entry_dup*)
                     pj_pool_alloc(pool, sizeof(pj_timer_entry_dup) * size);
    if (!ht->timer_dups)
        return PJ_ENOMEM;
#endif

    // Create the parallel
    ht->timer_ids = (pj_timer_id_t *)
                    pj_pool_alloc( pool, sizeof(pj_timer_id_t) * size);
    if (!ht->timer_ids)
        return PJ_ENOMEM;

    // Initialize the "freelist," which uses negative values to
    // distinguish freelist elements from "pointers" into the <heap_>
    // array.
    for (i=0; i<size; ++i)
        ht->timer_ids[i] = -((pj_timer_id_t) (i + 1));

#if PJ_TIMER_USE_LINKED_LIST
    pj_list_init(&ht->head_list);
#endif

    *p_heap = ht;
    return PJ_SUCCESS;
}

PJ_DEF(void) pj_timer_heap_destroy( pj_timer_heap_t *ht )
{
    if (ht->lock && ht->auto_delete_lock) {
        pj_lock_destroy(ht->lock);
        ht->lock = NULL;
    }
}

PJ_DEF(void) pj_timer_heap_set_lock(  pj_timer_heap_t *ht,
                                      pj_lock_t *lock,
                                      pj_bool_t auto_del )
{
    if (ht->lock && ht->auto_delete_lock)
        pj_lock_destroy(ht->lock);

    ht->lock = lock;
    ht->auto_delete_lock = auto_del;
}


PJ_DEF(unsigned) pj_timer_heap_set_max_timed_out_per_poll(pj_timer_heap_t *ht,
                                                          unsigned count )
{
    unsigned old_count = ht->max_entries_per_poll;
    ht->max_entries_per_poll = count;
    return old_count;
}

PJ_DEF(pj_timer_entry*) pj_timer_entry_init( pj_timer_entry *entry,
                                             int id,
                                             void *user_data,
                                             pj_timer_heap_callback *cb )
{
    pj_assert(entry && cb);

    entry->_timer_id = -1;
    entry->id = id;
    entry->user_data = user_data;
    entry->cb = cb;
#if !PJ_TIMER_USE_COPY
    entry->_grp_lock = NULL;
#endif

    return entry;
}

PJ_DEF(pj_bool_t) pj_timer_entry_running( pj_timer_entry *entry )
{
    return (entry->_timer_id >= 1);
}

#if PJ_TIMER_DEBUG
static pj_status_t schedule_w_grp_lock_dbg(pj_timer_heap_t *ht,
                                           pj_timer_entry *entry,
                                           const pj_time_val *delay,
                                           pj_bool_t set_id,
                                           int id_val,
                                           pj_grp_lock_t *grp_lock,
                                           const char *src_file,
                                           int src_line)
#else
static pj_status_t schedule_w_grp_lock(pj_timer_heap_t *ht,
                                       pj_timer_entry *entry,
                                       const pj_time_val *delay,
                                       pj_bool_t set_id,
                                       int id_val,
                                       pj_grp_lock_t *grp_lock)
#endif
{
    pj_status_t status;
    pj_time_val expires;

    PJ_ASSERT_RETURN(ht && entry && delay, PJ_EINVAL);
    PJ_ASSERT_RETURN(entry->cb != NULL, PJ_EINVAL);

    /* Prevent same entry from being scheduled more than once */
    //PJ_ASSERT_RETURN(entry->_timer_id < 1, PJ_EINVALIDOP);

    pj_gettickcount(&expires);
    PJ_TIME_VAL_ADD(expires, *delay);

    lock_timer_heap(ht);

    /* Prevent same entry from being scheduled more than once */
    if (pj_timer_entry_running(entry)) {
        unlock_timer_heap(ht);
        PJ_LOG(3,(THIS_FILE, "Warning! Rescheduling outstanding entry (%p)",
                  entry));
        return PJ_EINVALIDOP;
    }

    status = schedule_entry(ht, entry, &expires);
    if (status == PJ_SUCCESS) {
        pj_timer_entry_dup *timer_copy = GET_TIMER(ht, entry);

        if (set_id)
            GET_FIELD(timer_copy, id) = entry->id = id_val;
        timer_copy->_grp_lock = grp_lock;
        if (timer_copy->_grp_lock) {
            pj_grp_lock_add_ref(timer_copy->_grp_lock);
        }
#if PJ_TIMER_DEBUG
        timer_copy->src_file = src_file;
        timer_copy->src_line = src_line;
#endif
    }
    unlock_timer_heap(ht);

    return status;
}


#if PJ_TIMER_DEBUG
PJ_DEF(pj_status_t) pj_timer_heap_schedule_dbg( pj_timer_heap_t *ht,
                                                pj_timer_entry *entry,
                                                const pj_time_val *delay,
                                                const char *src_file,
                                                int src_line)
{
    return schedule_w_grp_lock_dbg(ht, entry, delay, PJ_FALSE, 1, NULL,
                                   src_file, src_line);
}

PJ_DEF(pj_status_t) pj_timer_heap_schedule_w_grp_lock_dbg(
                                                pj_timer_heap_t *ht,
                                                pj_timer_entry *entry,
                                                const pj_time_val *delay,
                                                int id_val,
                                                pj_grp_lock_t *grp_lock,
                                                const char *src_file,
                                                int src_line)
{
    return schedule_w_grp_lock_dbg(ht, entry, delay, PJ_TRUE, id_val,
                                   grp_lock, src_file, src_line);
}

#else
PJ_DEF(pj_status_t) pj_timer_heap_schedule( pj_timer_heap_t *ht,
                                            pj_timer_entry *entry,
                                            const pj_time_val *delay)
{
    return schedule_w_grp_lock(ht, entry, delay, PJ_FALSE, 1, NULL);
}

PJ_DEF(pj_status_t) pj_timer_heap_schedule_w_grp_lock(pj_timer_heap_t *ht,
                                                      pj_timer_entry *entry,
                                                      const pj_time_val *delay,
                                                      int id_val,
                                                      pj_grp_lock_t *grp_lock)
{
    return schedule_w_grp_lock(ht, entry, delay, PJ_TRUE, id_val, grp_lock);
}
#endif

static int cancel_timer(pj_timer_heap_t *ht,
                        pj_timer_entry *entry,
                        unsigned flags,
                        int id_val)
{
    pj_timer_entry_dup *timer_copy;
    pj_grp_lock_t *grp_lock;
    int count;

    PJ_ASSERT_RETURN(ht && entry, PJ_EINVAL);

    lock_timer_heap(ht);

    // Check to see if the timer_id is out of range
    if (entry->_timer_id < 1 || (pj_size_t)entry->_timer_id >= ht->max_size) {
        unlock_timer_heap(ht);
        return 0;
    }

    timer_copy = GET_TIMER(ht, entry);
    grp_lock = timer_copy->_grp_lock;

    count = cancel(ht, entry, flags | F_DONT_CALL);
    if (count > 0) {
        /* Timer entry found & cancelled */
        if (flags & F_SET_ID) {
            entry->id = id_val;
        }
        if (grp_lock) {
            pj_grp_lock_dec_ref(grp_lock);
        }
    }
    unlock_timer_heap(ht);

    return count;
}

PJ_DEF(int) pj_timer_heap_cancel( pj_timer_heap_t *ht,
                                  pj_timer_entry *entry)
{
    return cancel_timer(ht, entry, 0, 0);
}

PJ_DEF(int) pj_timer_heap_cancel_if_active(pj_timer_heap_t *ht,
                                           pj_timer_entry *entry,
                                           int id_val)
{
    return cancel_timer(ht, entry, F_SET_ID | F_DONT_ASSERT, id_val);
}

PJ_DEF(unsigned) pj_timer_heap_poll( pj_timer_heap_t *ht, 
                                     pj_time_val *next_delay )
{
    pj_time_val now;
    pj_time_val min_time_node = {0,0};
    unsigned count;
    pj_timer_id_t slot = 0;

    PJ_ASSERT_RETURN(ht, 0);

    lock_timer_heap(ht);
    if (!ht->cur_size && next_delay) {
        next_delay->sec = next_delay->msec = PJ_MAXINT32;
        unlock_timer_heap(ht);
        return 0;
    }

    count = 0;
    pj_gettickcount(&now);

    if (ht->cur_size) {
#if PJ_TIMER_USE_LINKED_LIST
        slot = ht->timer_ids[GET_FIELD(ht->head_list.next, _timer_id)];
#endif
        min_time_node = ht->heap[slot]->_timer_value;
    }

    while ( ht->cur_size && 
            PJ_TIME_VAL_LTE(min_time_node, now) &&
            count < ht->max_entries_per_poll ) 
    {
        pj_timer_entry_dup *node = remove_node(ht, slot);
        pj_timer_entry *entry = GET_ENTRY(node);
        /* Avoid re-use of this timer until the callback is done. */
        ///Not necessary, even causes problem (see also #2176).
        ///pj_timer_id_t node_timer_id = pop_freelist(ht);
        pj_grp_lock_t *grp_lock;
        pj_bool_t valid = PJ_TRUE;

        ++count;

        grp_lock = node->_grp_lock;
        node->_grp_lock = NULL;
        if (GET_FIELD(node, cb) != entry->cb ||
            GET_FIELD(node, user_data) != entry->user_data)
        {
            valid = PJ_FALSE;
#if PJ_TIMER_DEBUG
            PJ_LOG(3,(THIS_FILE, "Bug! Polling entry %p from %s line %d has "
                                 "been deallocated without being cancelled",
                                 GET_ENTRY(node),
                                 node->src_file, node->src_line));
#else
            PJ_LOG(3,(THIS_FILE, "Bug! Polling entry %p has "
                                 "been deallocated without being cancelled",
                                 GET_ENTRY(node)));
#endif
#if ASSERT_IF_ENTRY_DESTROYED
            pj_assert(node->dup.cb == entry->cb);
            pj_assert(node->dup.user_data == entry->user_data);
#endif
        }

        unlock_timer_heap(ht);

        PJ_RACE_ME(5);

        if (valid && entry->cb)
            (*entry->cb)(ht, entry);

        if (valid && grp_lock)
            pj_grp_lock_dec_ref(grp_lock);

        lock_timer_heap(ht);
        /* Now, the timer is really free for re-use. */
        ///push_freelist(ht, node_timer_id);

        if (ht->cur_size) {
#if PJ_TIMER_USE_LINKED_LIST
            slot = ht->timer_ids[GET_FIELD(ht->head_list.next, _timer_id)];
#endif
            min_time_node = ht->heap[slot]->_timer_value;
            /* Update now */
            pj_gettickcount(&now);
        }
    }
    if (ht->cur_size && next_delay) {
        *next_delay = ht->heap[0]->_timer_value;
        if (count > 0)
            pj_gettickcount(&now);
        PJ_TIME_VAL_SUB(*next_delay, now);
        if (next_delay->sec < 0 || next_delay->msec < 0)
            next_delay->sec = next_delay->msec = 0;
    } else if (next_delay) {
        next_delay->sec = next_delay->msec = PJ_MAXINT32;
    }
    unlock_timer_heap(ht);

    return count;
}

PJ_DEF(pj_size_t) pj_timer_heap_count( pj_timer_heap_t *ht )
{
    PJ_ASSERT_RETURN(ht, 0);

    return ht->cur_size;
}

PJ_DEF(pj_status_t) pj_timer_heap_earliest_time( pj_timer_heap_t * ht,
                                                 pj_time_val *timeval)
{
    pj_assert(ht->cur_size != 0);
    if (ht->cur_size == 0)
        return PJ_ENOTFOUND;

    lock_timer_heap(ht);
    *timeval = ht->heap[0]->_timer_value;
    unlock_timer_heap(ht);

    return PJ_SUCCESS;
}

#if PJ_TIMER_DEBUG
PJ_DEF(void) pj_timer_heap_dump(pj_timer_heap_t *ht)
{
    lock_timer_heap(ht);

    PJ_LOG(3,(THIS_FILE, "Dumping timer heap:"));
    PJ_LOG(3,(THIS_FILE, "  Cur size: %d entries, max: %d",
                         (int)ht->cur_size, (int)ht->max_size));

    if (ht->cur_size) {
#if PJ_TIMER_USE_LINKED_LIST
        pj_timer_entry_dup *tmp_dup;
#else
        unsigned i;
#endif
        pj_time_val now;

        PJ_LOG(3,(THIS_FILE, "  Entries: "));
        PJ_LOG(3,(THIS_FILE, "    _id\tId\tElapsed\tSource"));
        PJ_LOG(3,(THIS_FILE, "    ----------------------------------"));

        pj_gettickcount(&now);

#if !PJ_TIMER_USE_LINKED_LIST
        for (i=0; i<(unsigned)ht->cur_size; ++i)
        {
            pj_timer_entry_dup *e = ht->heap[i];
#else
        for (tmp_dup = ht->head_list.next; tmp_dup != &ht->head_list;
             tmp_dup = tmp_dup->next)
        {
            pj_timer_entry_dup *e = tmp_dup;
#endif

            pj_time_val delta;

            if (PJ_TIME_VAL_LTE(e->_timer_value, now))
                delta.sec = delta.msec = 0;
            else {
                delta = e->_timer_value;
                PJ_TIME_VAL_SUB(delta, now);
            }

            PJ_LOG(3,(THIS_FILE, "    %d\t%d\t%d.%03d\t%s:%d",
                      GET_FIELD(e, _timer_id), GET_FIELD(e, id),
                      (int)delta.sec, (int)delta.msec,
                      e->src_file, e->src_line));
        }
    }

    unlock_timer_heap(ht);
}
#endif

