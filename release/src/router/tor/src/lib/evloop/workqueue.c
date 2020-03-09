
/* copyright (c) 2013-2015, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file workqueue.c
 *
 * \brief Implements worker threads, queues of work for them, and mechanisms
 * for them to send answers back to the main thread.
 *
 * The main structure here is a threadpool_t : it manages a set of worker
 * threads, a queue of pending work, and a reply queue.  Every piece of work
 * is a workqueue_entry_t, containing data to process and a function to
 * process it with.
 *
 * The main thread informs the worker threads of pending work by using a
 * condition variable.  The workers inform the main process of completed work
 * by using an alert_sockets_t object, as implemented in net/alertsock.c.
 *
 * The main thread can also queue an "update" that will be handled by all the
 * workers.  This is useful for updating state that all the workers share.
 *
 * In Tor today, there is currently only one thread pool, used in cpuworker.c.
 */

#include "orconfig.h"
#include "lib/evloop/compat_libevent.h"
#include "lib/evloop/workqueue.h"

#include "lib/crypt_ops/crypto_rand.h"
#include "lib/intmath/weakrng.h"
#include "lib/log/ratelim.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/net/alertsock.h"
#include "lib/net/socket.h"
#include "lib/thread/threads.h"

#include "ext/tor_queue.h"
#include <event2/event.h>
#include <string.h>

#define WORKQUEUE_PRIORITY_FIRST WQ_PRI_HIGH
#define WORKQUEUE_PRIORITY_LAST WQ_PRI_LOW
#define WORKQUEUE_N_PRIORITIES (((int) WORKQUEUE_PRIORITY_LAST)+1)

TOR_TAILQ_HEAD(work_tailq_t, workqueue_entry_s);
typedef struct work_tailq_t work_tailq_t;

struct threadpool_s {
  /** An array of pointers to workerthread_t: one for each running worker
   * thread. */
  struct workerthread_s **threads;

  /** Condition variable that we wait on when we have no work, and which
   * gets signaled when our queue becomes nonempty. */
  tor_cond_t condition;
  /** Queues of pending work that we have to do. The queue with priority
   * <b>p</b> is work[p]. */
  work_tailq_t work[WORKQUEUE_N_PRIORITIES];

  /** The current 'update generation' of the threadpool.  Any thread that is
   * at an earlier generation needs to run the update function. */
  unsigned generation;

  /** Function that should be run for updates on each thread. */
  workqueue_reply_t (*update_fn)(void *, void *);
  /** Function to free update arguments if they can't be run. */
  void (*free_update_arg_fn)(void *);
  /** Array of n_threads update arguments. */
  void **update_args;
  /** Event to notice when another thread has sent a reply. */
  struct event *reply_event;
  void (*reply_cb)(threadpool_t *);

  /** Number of elements in threads. */
  int n_threads;
  /** Mutex to protect all the above fields. */
  tor_mutex_t lock;

  /** A reply queue to use when constructing new threads. */
  replyqueue_t *reply_queue;

  /** Functions used to allocate and free thread state. */
  void *(*new_thread_state_fn)(void*);
  void (*free_thread_state_fn)(void*);
  void *new_thread_state_arg;
};

/** Used to put a workqueue_priority_t value into a bitfield. */
#define workqueue_priority_bitfield_t ENUM_BF(workqueue_priority_t)
/** Number of bits needed to hold all legal values of workqueue_priority_t */
#define WORKQUEUE_PRIORITY_BITS 2

struct workqueue_entry_s {
  /** The next workqueue_entry_t that's pending on the same thread or
   * reply queue. */
  TOR_TAILQ_ENTRY(workqueue_entry_s) next_work;
  /** The threadpool to which this workqueue_entry_t was assigned. This field
   * is set when the workqueue_entry_t is created, and won't be cleared until
   * after it's handled in the main thread. */
  struct threadpool_s *on_pool;
  /** True iff this entry is waiting for a worker to start processing it. */
  uint8_t pending;
  /** Priority of this entry. */
  workqueue_priority_bitfield_t priority : WORKQUEUE_PRIORITY_BITS;
  /** Function to run in the worker thread. */
  workqueue_reply_t (*fn)(void *state, void *arg);
  /** Function to run while processing the reply queue. */
  void (*reply_fn)(void *arg);
  /** Argument for the above functions. */
  void *arg;
};

struct replyqueue_s {
  /** Mutex to protect the answers field */
  tor_mutex_t lock;
  /** Doubly-linked list of answers that the reply queue needs to handle. */
  TOR_TAILQ_HEAD(, workqueue_entry_s) answers;

  /** Mechanism to wake up the main thread when it is receiving answers. */
  alert_sockets_t alert;
};

/** A worker thread represents a single thread in a thread pool. */
typedef struct workerthread_s {
  /** Which thread it this?  In range 0..in_pool->n_threads-1 */
  int index;
  /** The pool this thread is a part of. */
  struct threadpool_s *in_pool;
  /** User-supplied state field that we pass to the worker functions of each
   * work item. */
  void *state;
  /** Reply queue to which we pass our results. */
  replyqueue_t *reply_queue;
  /** The current update generation of this thread */
  unsigned generation;
  /** One over the probability of taking work from a lower-priority queue. */
  int32_t lower_priority_chance;
} workerthread_t;

static void queue_reply(replyqueue_t *queue, workqueue_entry_t *work);

/** Allocate and return a new workqueue_entry_t, set up to run the function
 * <b>fn</b> in the worker thread, and <b>reply_fn</b> in the main
 * thread. See threadpool_queue_work() for full documentation. */
static workqueue_entry_t *
workqueue_entry_new(workqueue_reply_t (*fn)(void*, void*),
                    void (*reply_fn)(void*),
                    void *arg)
{
  workqueue_entry_t *ent = tor_malloc_zero(sizeof(workqueue_entry_t));
  ent->fn = fn;
  ent->reply_fn = reply_fn;
  ent->arg = arg;
  ent->priority = WQ_PRI_HIGH;
  return ent;
}

#define workqueue_entry_free(ent) \
  FREE_AND_NULL(workqueue_entry_t, workqueue_entry_free_, (ent))

/**
 * Release all storage held in <b>ent</b>. Call only when <b>ent</b> is not on
 * any queue.
 */
static void
workqueue_entry_free_(workqueue_entry_t *ent)
{
  if (!ent)
    return;
  memset(ent, 0xf0, sizeof(*ent));
  tor_free(ent);
}

/**
 * Cancel a workqueue_entry_t that has been returned from
 * threadpool_queue_work.
 *
 * You must not call this function on any work whose reply function has been
 * executed in the main thread; that will cause undefined behavior (probably,
 * a crash).
 *
 * If the work is cancelled, this function return the argument passed to the
 * work function. It is the caller's responsibility to free this storage.
 *
 * This function will have no effect if the worker thread has already executed
 * or begun to execute the work item.  In that case, it will return NULL.
 */
void *
workqueue_entry_cancel(workqueue_entry_t *ent)
{
  int cancelled = 0;
  void *result = NULL;
  tor_mutex_acquire(&ent->on_pool->lock);
  workqueue_priority_t prio = ent->priority;
  if (ent->pending) {
    TOR_TAILQ_REMOVE(&ent->on_pool->work[prio], ent, next_work);
    cancelled = 1;
    result = ent->arg;
  }
  tor_mutex_release(&ent->on_pool->lock);

  if (cancelled) {
    workqueue_entry_free(ent);
  }
  return result;
}

/**DOCDOC

   must hold lock */
static int
worker_thread_has_work(workerthread_t *thread)
{
  unsigned i;
  for (i = WORKQUEUE_PRIORITY_FIRST; i <= WORKQUEUE_PRIORITY_LAST; ++i) {
    if (!TOR_TAILQ_EMPTY(&thread->in_pool->work[i]))
        return 1;
  }
  return thread->generation != thread->in_pool->generation;
}

/** Extract the next workqueue_entry_t from the the thread's pool, removing
 * it from the relevant queues and marking it as non-pending.
 *
 * The caller must hold the lock. */
static workqueue_entry_t *
worker_thread_extract_next_work(workerthread_t *thread)
{
  threadpool_t *pool = thread->in_pool;
  work_tailq_t *queue = NULL, *this_queue;
  unsigned i;
  for (i = WORKQUEUE_PRIORITY_FIRST; i <= WORKQUEUE_PRIORITY_LAST; ++i) {
    this_queue = &pool->work[i];
    if (!TOR_TAILQ_EMPTY(this_queue)) {
      queue = this_queue;
      if (! crypto_fast_rng_one_in_n(get_thread_fast_rng(),
                                     thread->lower_priority_chance)) {
        /* Usually we'll just break now, so that we can get out of the loop
         * and use the queue where we found work. But with a small
         * probability, we'll keep looking for lower priority work, so that
         * we don't ignore our low-priority queues entirely. */
        break;
      }
    }
  }

  if (queue == NULL)
    return NULL;

  workqueue_entry_t *work = TOR_TAILQ_FIRST(queue);
  TOR_TAILQ_REMOVE(queue, work, next_work);
  work->pending = 0;
  return work;
}

/**
 * Main function for the worker thread.
 */
static void
worker_thread_main(void *thread_)
{
  workerthread_t *thread = thread_;
  threadpool_t *pool = thread->in_pool;
  workqueue_entry_t *work;
  workqueue_reply_t result;

  tor_mutex_acquire(&pool->lock);
  while (1) {
    /* lock must be held at this point. */
    while (worker_thread_has_work(thread)) {
      /* lock must be held at this point. */
      if (thread->in_pool->generation != thread->generation) {
        void *arg = thread->in_pool->update_args[thread->index];
        thread->in_pool->update_args[thread->index] = NULL;
        workqueue_reply_t (*update_fn)(void*,void*) =
            thread->in_pool->update_fn;
        thread->generation = thread->in_pool->generation;
        tor_mutex_release(&pool->lock);

        workqueue_reply_t r = update_fn(thread->state, arg);

        if (r != WQ_RPL_REPLY) {
          return;
        }

        tor_mutex_acquire(&pool->lock);
        continue;
      }
      work = worker_thread_extract_next_work(thread);
      if (BUG(work == NULL))
        break;
      tor_mutex_release(&pool->lock);

      /* We run the work function without holding the thread lock. This
       * is the main thread's first opportunity to give us more work. */
      result = work->fn(thread->state, work->arg);

      /* Queue the reply for the main thread. */
      queue_reply(thread->reply_queue, work);

      /* We may need to exit the thread. */
      if (result != WQ_RPL_REPLY) {
        return;
      }
      tor_mutex_acquire(&pool->lock);
    }
    /* At this point the lock is held, and there is no work in this thread's
     * queue. */

    /* TODO: support an idle-function */

    /* Okay. Now, wait till somebody has work for us. */
    if (tor_cond_wait(&pool->condition, &pool->lock, NULL) < 0) {
      log_warn(LD_GENERAL, "Fail tor_cond_wait.");
    }
  }
}

/** Put a reply on the reply queue.  The reply must not currently be on
 * any thread's work queue. */
static void
queue_reply(replyqueue_t *queue, workqueue_entry_t *work)
{
  int was_empty;
  tor_mutex_acquire(&queue->lock);
  was_empty = TOR_TAILQ_EMPTY(&queue->answers);
  TOR_TAILQ_INSERT_TAIL(&queue->answers, work, next_work);
  tor_mutex_release(&queue->lock);

  if (was_empty) {
    if (queue->alert.alert_fn(queue->alert.write_fd) < 0) {
      /* XXXX complain! */
    }
  }
}

/** Allocate and start a new worker thread to use state object <b>state</b>,
 * and send responses to <b>replyqueue</b>. */
static workerthread_t *
workerthread_new(int32_t lower_priority_chance,
                 void *state, threadpool_t *pool, replyqueue_t *replyqueue)
{
  workerthread_t *thr = tor_malloc_zero(sizeof(workerthread_t));
  thr->state = state;
  thr->reply_queue = replyqueue;
  thr->in_pool = pool;
  thr->lower_priority_chance = lower_priority_chance;

  if (spawn_func(worker_thread_main, thr) < 0) {
    //LCOV_EXCL_START
    tor_assert_nonfatal_unreached();
    log_err(LD_GENERAL, "Can't launch worker thread.");
    tor_free(thr);
    return NULL;
    //LCOV_EXCL_STOP
  }

  return thr;
}

/**
 * Queue an item of work for a thread in a thread pool.  The function
 * <b>fn</b> will be run in a worker thread, and will receive as arguments the
 * thread's state object, and the provided object <b>arg</b>. It must return
 * one of WQ_RPL_REPLY, WQ_RPL_ERROR, or WQ_RPL_SHUTDOWN.
 *
 * Regardless of its return value, the function <b>reply_fn</b> will later be
 * run in the main thread when it invokes replyqueue_process(), and will
 * receive as its argument the same <b>arg</b> object.  It's the reply
 * function's responsibility to free the work object.
 *
 * On success, return a workqueue_entry_t object that can be passed to
 * workqueue_entry_cancel(). On failure, return NULL.  (Failure is not
 * currently possible, but callers should check anyway.)
 *
 * Items are executed in a loose priority order -- each thread will usually
 * take from the queued work with the highest prioirity, but will occasionally
 * visit lower-priority queues to keep them from starving completely.
 *
 * Note that because of priorities and thread behavior, work items may not
 * be executed strictly in order.
 */
workqueue_entry_t *
threadpool_queue_work_priority(threadpool_t *pool,
                               workqueue_priority_t prio,
                               workqueue_reply_t (*fn)(void *, void *),
                               void (*reply_fn)(void *),
                               void *arg)
{
  tor_assert(((int)prio) >= WORKQUEUE_PRIORITY_FIRST &&
             ((int)prio) <= WORKQUEUE_PRIORITY_LAST);

  workqueue_entry_t *ent = workqueue_entry_new(fn, reply_fn, arg);
  ent->on_pool = pool;
  ent->pending = 1;
  ent->priority = prio;

  tor_mutex_acquire(&pool->lock);

  TOR_TAILQ_INSERT_TAIL(&pool->work[prio], ent, next_work);

  tor_cond_signal_one(&pool->condition);

  tor_mutex_release(&pool->lock);

  return ent;
}

/** As threadpool_queue_work_priority(), but assumes WQ_PRI_HIGH */
workqueue_entry_t *
threadpool_queue_work(threadpool_t *pool,
                      workqueue_reply_t (*fn)(void *, void *),
                      void (*reply_fn)(void *),
                      void *arg)
{
  return threadpool_queue_work_priority(pool, WQ_PRI_HIGH, fn, reply_fn, arg);
}

/**
 * Queue a copy of a work item for every thread in a pool.  This can be used,
 * for example, to tell the threads to update some parameter in their states.
 *
 * Arguments are as for <b>threadpool_queue_work</b>, except that the
 * <b>arg</b> value is passed to <b>dup_fn</b> once per each thread to
 * make a copy of it.
 *
 * UPDATE FUNCTIONS MUST BE IDEMPOTENT.  We do not guarantee that every update
 * will be run.  If a new update is scheduled before the old update finishes
 * running, then the new will replace the old in any threads that haven't run
 * it yet.
 *
 * Return 0 on success, -1 on failure.
 */
int
threadpool_queue_update(threadpool_t *pool,
                         void *(*dup_fn)(void *),
                         workqueue_reply_t (*fn)(void *, void *),
                         void (*free_fn)(void *),
                         void *arg)
{
  int i, n_threads;
  void (*old_args_free_fn)(void *arg);
  void **old_args;
  void **new_args;

  tor_mutex_acquire(&pool->lock);
  n_threads = pool->n_threads;
  old_args = pool->update_args;
  old_args_free_fn = pool->free_update_arg_fn;

  new_args = tor_calloc(n_threads, sizeof(void*));
  for (i = 0; i < n_threads; ++i) {
    if (dup_fn)
      new_args[i] = dup_fn(arg);
    else
      new_args[i] = arg;
  }

  pool->update_args = new_args;
  pool->free_update_arg_fn = free_fn;
  pool->update_fn = fn;
  ++pool->generation;

  tor_cond_signal_all(&pool->condition);

  tor_mutex_release(&pool->lock);

  if (old_args) {
    for (i = 0; i < n_threads; ++i) {
      if (old_args[i] && old_args_free_fn)
        old_args_free_fn(old_args[i]);
    }
    tor_free(old_args);
  }

  return 0;
}

/** Don't have more than this many threads per pool. */
#define MAX_THREADS 1024

/** For half of our threads, choose lower priority queues with probability
 * 1/N for each of these values. Both are chosen somewhat arbitrarily.  If
 * CHANCE_PERMISSIVE is too low, then we have a risk of low-priority tasks
 * stalling forever.  If it's too high, we have a risk of low-priority tasks
 * grabbing half of the threads. */
#define CHANCE_PERMISSIVE 37
#define CHANCE_STRICT INT32_MAX

/** Launch threads until we have <b>n</b>. */
static int
threadpool_start_threads(threadpool_t *pool, int n)
{
  if (BUG(n < 0))
    return -1; // LCOV_EXCL_LINE
  if (n > MAX_THREADS)
    n = MAX_THREADS;

  tor_mutex_acquire(&pool->lock);

  if (pool->n_threads < n)
    pool->threads = tor_reallocarray(pool->threads,
                                     sizeof(workerthread_t*), n);

  while (pool->n_threads < n) {
    /* For half of our threads, we'll choose lower priorities permissively;
     * for the other half, we'll stick more strictly to higher priorities.
     * This keeps slow low-priority tasks from taking over completely. */
    int32_t chance = (pool->n_threads & 1) ? CHANCE_STRICT : CHANCE_PERMISSIVE;

    void *state = pool->new_thread_state_fn(pool->new_thread_state_arg);
    workerthread_t *thr = workerthread_new(chance,
                                           state, pool, pool->reply_queue);

    if (!thr) {
      //LCOV_EXCL_START
      tor_assert_nonfatal_unreached();
      pool->free_thread_state_fn(state);
      tor_mutex_release(&pool->lock);
      return -1;
      //LCOV_EXCL_STOP
    }
    thr->index = pool->n_threads;
    pool->threads[pool->n_threads++] = thr;
  }
  tor_mutex_release(&pool->lock);

  return 0;
}

/**
 * Construct a new thread pool with <b>n</b> worker threads, configured to
 * send their output to <b>replyqueue</b>.  The threads' states will be
 * constructed with the <b>new_thread_state_fn</b> call, receiving <b>arg</b>
 * as its argument.  When the threads close, they will call
 * <b>free_thread_state_fn</b> on their states.
 */
threadpool_t *
threadpool_new(int n_threads,
               replyqueue_t *replyqueue,
               void *(*new_thread_state_fn)(void*),
               void (*free_thread_state_fn)(void*),
               void *arg)
{
  threadpool_t *pool;
  pool = tor_malloc_zero(sizeof(threadpool_t));
  tor_mutex_init_nonrecursive(&pool->lock);
  tor_cond_init(&pool->condition);
  unsigned i;
  for (i = WORKQUEUE_PRIORITY_FIRST; i <= WORKQUEUE_PRIORITY_LAST; ++i) {
    TOR_TAILQ_INIT(&pool->work[i]);
  }

  pool->new_thread_state_fn = new_thread_state_fn;
  pool->new_thread_state_arg = arg;
  pool->free_thread_state_fn = free_thread_state_fn;
  pool->reply_queue = replyqueue;

  if (threadpool_start_threads(pool, n_threads) < 0) {
    //LCOV_EXCL_START
    tor_assert_nonfatal_unreached();
    tor_cond_uninit(&pool->condition);
    tor_mutex_uninit(&pool->lock);
    tor_free(pool);
    return NULL;
    //LCOV_EXCL_STOP
  }

  return pool;
}

/** Return the reply queue associated with a given thread pool. */
replyqueue_t *
threadpool_get_replyqueue(threadpool_t *tp)
{
  return tp->reply_queue;
}

/** Allocate a new reply queue.  Reply queues are used to pass results from
 * worker threads to the main thread.  Since the main thread is running an
 * IO-centric event loop, it needs to get woken up with means other than a
 * condition variable. */
replyqueue_t *
replyqueue_new(uint32_t alertsocks_flags)
{
  replyqueue_t *rq;

  rq = tor_malloc_zero(sizeof(replyqueue_t));
  if (alert_sockets_create(&rq->alert, alertsocks_flags) < 0) {
    //LCOV_EXCL_START
    tor_free(rq);
    return NULL;
    //LCOV_EXCL_STOP
  }

  tor_mutex_init(&rq->lock);
  TOR_TAILQ_INIT(&rq->answers);

  return rq;
}

/** Internal: Run from the libevent mainloop when there is work to handle in
 * the reply queue handler. */
static void
reply_event_cb(evutil_socket_t sock, short events, void *arg)
{
  threadpool_t *tp = arg;
  (void) sock;
  (void) events;
  replyqueue_process(tp->reply_queue);
  if (tp->reply_cb)
    tp->reply_cb(tp);
}

/** Register the threadpool <b>tp</b>'s reply queue with Tor's global
 * libevent mainloop. If <b>cb</b> is provided, it is run after
 * each time there is work to process from the reply queue. Return 0 on
 * success, -1 on failure.
 */
int
threadpool_register_reply_event(threadpool_t *tp,
                                void (*cb)(threadpool_t *tp))
{
  struct event_base *base = tor_libevent_get_base();

  if (tp->reply_event) {
    tor_event_free(tp->reply_event);
  }
  tp->reply_event = tor_event_new(base,
                                  tp->reply_queue->alert.read_fd,
                                  EV_READ|EV_PERSIST,
                                  reply_event_cb,
                                  tp);
  tor_assert(tp->reply_event);
  tp->reply_cb = cb;
  return event_add(tp->reply_event, NULL);
}

/**
 * Process all pending replies on a reply queue. The main thread should call
 * this function every time the socket returned by replyqueue_get_socket() is
 * readable.
 */
void
replyqueue_process(replyqueue_t *queue)
{
  int r = queue->alert.drain_fn(queue->alert.read_fd);
  if (r < 0) {
    //LCOV_EXCL_START
    static ratelim_t warn_limit = RATELIM_INIT(7200);
    log_fn_ratelim(&warn_limit, LOG_WARN, LD_GENERAL,
                 "Failure from drain_fd: %s",
                   tor_socket_strerror(-r));
    //LCOV_EXCL_STOP
  }

  tor_mutex_acquire(&queue->lock);
  while (!TOR_TAILQ_EMPTY(&queue->answers)) {
    /* lock must be held at this point.*/
    workqueue_entry_t *work = TOR_TAILQ_FIRST(&queue->answers);
    TOR_TAILQ_REMOVE(&queue->answers, work, next_work);
    tor_mutex_release(&queue->lock);
    work->on_pool = NULL;

    work->reply_fn(work->arg);
    workqueue_entry_free(work);

    tor_mutex_acquire(&queue->lock);
  }

  tor_mutex_release(&queue->lock);
}
