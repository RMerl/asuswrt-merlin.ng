/*
 * Copyright (C) 2005-2011 Martin Willi
 * Copyright (C) 2011 revosec AG
 * Copyright (C) 2008-2013 Tobias Brunner
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "processor.h"

#include <utils/debug.h>
#include <threading/thread.h>
#include <threading/condvar.h>
#include <threading/mutex.h>
#include <threading/thread_value.h>
#include <collections/linked_list.h>

typedef struct private_processor_t private_processor_t;

/**
 * Private data of processor_t class.
 */
struct private_processor_t {

	/**
	 * Public processor_t interface.
	 */
	processor_t public;

	/**
	 * Number of running threads
	 */
	u_int total_threads;

	/**
	 * Desired number of threads
	 */
	u_int desired_threads;

	/**
	 * Number of threads currently working, for each priority
	 */
	u_int working_threads[JOB_PRIO_MAX];

	/**
	 * All threads managed in the pool (including threads that have been
	 * canceled, this allows to join them later), as worker_thread_t
	 */
	linked_list_t *threads;

	/**
	 * A list of queued jobs for each priority
	 */
	linked_list_t *jobs[JOB_PRIO_MAX];

	/**
	 * Threads reserved for each priority
	 */
	int prio_threads[JOB_PRIO_MAX];

	/**
	 * access to job lists is locked through this mutex
	 */
	mutex_t *mutex;

	/**
	 * Condvar to wait for new jobs
	 */
	condvar_t *job_added;

	/**
	 * Condvar to wait for terminated threads
	 */
	condvar_t *thread_terminated;
};

/**
 * Worker thread
 */
typedef struct {

	/**
	 * Reference to the processor
	 */
	private_processor_t *processor;

	/**
	 * The actual thread
	 */
	thread_t *thread;

	/**
	 * Job currently being executed by this worker thread
	 */
	job_t *job;

	/**
	 * Priority of the current job
	 */
	job_priority_t priority;

} worker_thread_t;

static void process_jobs(worker_thread_t *worker);

/**
 * restart a terminated thread
 */
static void restart(worker_thread_t *worker)
{
	private_processor_t *this = worker->processor;
	job_t *job;

	DBG2(DBG_JOB, "terminated worker thread %.2u", thread_current_id());

	this->mutex->lock(this->mutex);
	/* cleanup worker thread  */
	this->working_threads[worker->priority]--;
	worker->job->status = JOB_STATUS_CANCELED;
	job = worker->job;
	/* unset the job before releasing the mutex, otherwise cancel() might
	 * interfere */
	worker->job = NULL;
	/* release mutex to avoid deadlocks if the same lock is required
	 * during queue_job() and in the destructor called here */
	this->mutex->unlock(this->mutex);
	job->destroy(job);
	this->mutex->lock(this->mutex);

	/* respawn thread if required */
	if (this->desired_threads >= this->total_threads)
	{
		worker_thread_t *new_worker;

		INIT(new_worker,
			.processor = this,
		);
		new_worker->thread = thread_create((thread_main_t)process_jobs,
										   new_worker);
		if (new_worker->thread)
		{
			this->threads->insert_last(this->threads, new_worker);
			this->mutex->unlock(this->mutex);
			return;
		}
		free(new_worker);
	}
	this->total_threads--;
	this->thread_terminated->signal(this->thread_terminated);
	this->mutex->unlock(this->mutex);
}

/**
 * Get number of idle threads, non-locking variant
 */
static u_int get_idle_threads_nolock(private_processor_t *this)
{
	u_int count, i;

	count = this->total_threads;
	for (i = 0; i < JOB_PRIO_MAX; i++)
	{
		count -= this->working_threads[i];
	}
	return count;
}

/**
 * Get a job from any job queue, starting with the highest priority.
 *
 * this->mutex is expected to be locked.
 */
static bool get_job(private_processor_t *this, worker_thread_t *worker)
{
	int i, reserved = 0, idle;

	idle = get_idle_threads_nolock(this);

	for (i = 0; i < JOB_PRIO_MAX; i++)
	{
		if (reserved && reserved >= idle)
		{
			DBG2(DBG_JOB, "delaying %N priority jobs: %d threads idle, "
				 "but %d reserved for higher priorities",
				 job_priority_names, i, idle, reserved);
			/* wait until a job of higher priority gets queued */
			return FALSE;
		}
		if (this->working_threads[i] < this->prio_threads[i])
		{
			reserved += this->prio_threads[i] - this->working_threads[i];
		}
		if (this->jobs[i]->remove_first(this->jobs[i],
										(void**)&worker->job) == SUCCESS)
		{
			worker->priority = i;
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Process a single job (provided in worker->job, worker->priority is also
 * expected to be set)
 *
 * this->mutex is expected to be locked.
 */
static void process_job(private_processor_t *this, worker_thread_t *worker)
{
	job_t *to_destroy = NULL;
	job_requeue_t requeue;

	this->working_threads[worker->priority]++;
	worker->job->status = JOB_STATUS_EXECUTING;
	this->mutex->unlock(this->mutex);
	/* canceled threads are restarted to get a constant pool */
	thread_cleanup_push((thread_cleanup_t)restart, worker);
	while (TRUE)
	{
		requeue = worker->job->execute(worker->job);
		if (requeue.type != JOB_REQUEUE_TYPE_DIRECT)
		{
			break;
		}
		else if (!worker->job->cancel)
		{	/* only allow cancelable jobs to requeue directly */
			requeue.type = JOB_REQUEUE_TYPE_FAIR;
			break;
		}
	}
	thread_cleanup_pop(FALSE);
	this->mutex->lock(this->mutex);
	this->working_threads[worker->priority]--;
	if (worker->job->status == JOB_STATUS_CANCELED)
	{	/* job was canceled via a custom cancel() method or did not
		 * use JOB_REQUEUE_TYPE_DIRECT */
		to_destroy = worker->job;
	}
	else
	{
		switch (requeue.type)
		{
			case JOB_REQUEUE_TYPE_NONE:
				worker->job->status = JOB_STATUS_DONE;
				to_destroy = worker->job;
				break;
			case JOB_REQUEUE_TYPE_FAIR:
				worker->job->status = JOB_STATUS_QUEUED;
				this->jobs[worker->priority]->insert_last(
									this->jobs[worker->priority], worker->job);
				this->job_added->signal(this->job_added);
				break;
			case JOB_REQUEUE_TYPE_SCHEDULE:
				/* scheduler_t does not hold its lock when queuing jobs
				 * so this should be safe without unlocking our mutex */
				switch (requeue.schedule)
				{
					case JOB_SCHEDULE:
						lib->scheduler->schedule_job(lib->scheduler,
												worker->job, requeue.time.rel);
						break;
					case JOB_SCHEDULE_MS:
						lib->scheduler->schedule_job_ms(lib->scheduler,
												worker->job, requeue.time.rel);
						break;
					case JOB_SCHEDULE_TV:
						lib->scheduler->schedule_job_tv(lib->scheduler,
												worker->job, requeue.time.abs);
						break;
				}
				break;
			default:
				break;
		}
	}
	/* unset the current job to avoid interference with cancel() when
	 * destroying the job below */
	worker->job = NULL;

	if (to_destroy)
	{	/* release mutex to avoid deadlocks if the same lock is required
		 * during queue_job() and in the destructor called here */
		this->mutex->unlock(this->mutex);
		to_destroy->destroy(to_destroy);
		this->mutex->lock(this->mutex);
	}
}

/**
 * Process queued jobs, called by the worker threads
 */
static void process_jobs(worker_thread_t *worker)
{
	private_processor_t *this = worker->processor;

	/* worker threads are not cancelable by default */
	thread_cancelability(FALSE);

	DBG2(DBG_JOB, "started worker thread %.2u", thread_current_id());

	this->mutex->lock(this->mutex);
	while (this->desired_threads >= this->total_threads)
	{
		if (get_job(this, worker))
		{
			process_job(this, worker);
		}
		else
		{
			this->job_added->wait(this->job_added, this->mutex);
		}
	}
	this->total_threads--;
	this->thread_terminated->signal(this->thread_terminated);
	this->mutex->unlock(this->mutex);
}

METHOD(processor_t, get_total_threads, u_int,
	private_processor_t *this)
{
	u_int count;

	this->mutex->lock(this->mutex);
	count = this->total_threads;
	this->mutex->unlock(this->mutex);
	return count;
}

METHOD(processor_t, get_idle_threads, u_int,
	private_processor_t *this)
{
	u_int count;

	this->mutex->lock(this->mutex);
	count = get_idle_threads_nolock(this);
	this->mutex->unlock(this->mutex);
	return count;
}

/**
 * Check priority bounds
 */
static job_priority_t sane_prio(job_priority_t prio)
{
	if ((int)prio < 0 || prio >= JOB_PRIO_MAX)
	{
		return JOB_PRIO_MAX - 1;
	}
	return prio;
}

METHOD(processor_t, get_working_threads, u_int,
	private_processor_t *this, job_priority_t prio)
{
	u_int count;

	this->mutex->lock(this->mutex);
	count = this->working_threads[sane_prio(prio)];
	this->mutex->unlock(this->mutex);
	return count;
}

METHOD(processor_t, get_job_load, u_int,
	private_processor_t *this, job_priority_t prio)
{
	u_int load;

	prio = sane_prio(prio);
	this->mutex->lock(this->mutex);
	load = this->jobs[prio]->get_count(this->jobs[prio]);
	this->mutex->unlock(this->mutex);
	return load;
}

METHOD(processor_t, queue_job, void,
	private_processor_t *this, job_t *job)
{
	job_priority_t prio;

	prio = sane_prio(job->get_priority(job));
	job->status = JOB_STATUS_QUEUED;

	this->mutex->lock(this->mutex);
	this->jobs[prio]->insert_last(this->jobs[prio], job);
	this->job_added->signal(this->job_added);
	this->mutex->unlock(this->mutex);
}

METHOD(processor_t, execute_job, void,
	private_processor_t *this, job_t *job)
{
	job_priority_t prio;
	bool queued = FALSE;

	this->mutex->lock(this->mutex);
	if (this->desired_threads && get_idle_threads_nolock(this))
	{
		prio = sane_prio(job->get_priority(job));
		job->status = JOB_STATUS_QUEUED;
		/* insert job in front to execute it immediately */
		this->jobs[prio]->insert_first(this->jobs[prio], job);
		queued = TRUE;
	}
	this->job_added->signal(this->job_added);
	this->mutex->unlock(this->mutex);

	if (!queued)
	{
		job->execute(job);
		job->destroy(job);
	}
}

METHOD(processor_t, set_threads, void,
	private_processor_t *this, u_int count)
{
	this->mutex->lock(this->mutex);
	if (count > this->total_threads)
	{	/* increase thread count */
		worker_thread_t *worker;
		int i;

		this->desired_threads = count;
		DBG1(DBG_JOB, "spawning %d worker threads", count - this->total_threads);
		for (i = this->total_threads; i < count; i++)
		{
			INIT(worker,
				.processor = this,
			);
			worker->thread = thread_create((thread_main_t)process_jobs, worker);
			if (worker->thread)
			{
				this->threads->insert_last(this->threads, worker);
				this->total_threads++;
			}
			else
			{
				free(worker);
			}
		}
	}
	else if (count < this->total_threads)
	{	/* decrease thread count */
		this->desired_threads = count;
	}
	this->job_added->broadcast(this->job_added);
	this->mutex->unlock(this->mutex);
}

METHOD(processor_t, cancel, void,
	private_processor_t *this)
{
	enumerator_t *enumerator;
	worker_thread_t *worker;
	job_t *job;
	int i;

	this->mutex->lock(this->mutex);
	this->desired_threads = 0;
	/* cancel potentially blocking jobs */
	enumerator = this->threads->create_enumerator(this->threads);
	while (enumerator->enumerate(enumerator, (void**)&worker))
	{
		if (worker->job && worker->job->cancel)
		{
			worker->job->status = JOB_STATUS_CANCELED;
			if (!worker->job->cancel(worker->job))
			{	/* job requests to be canceled explicitly, otherwise we assume
				 * the thread terminates itself and can be joined */
				worker->thread->cancel(worker->thread);
			}
		}
	}
	enumerator->destroy(enumerator);
	while (this->total_threads > 0)
	{
		this->job_added->broadcast(this->job_added);
		this->thread_terminated->wait(this->thread_terminated, this->mutex);
	}
	while (this->threads->remove_first(this->threads,
									  (void**)&worker) == SUCCESS)
	{
		worker->thread->join(worker->thread);
		free(worker);
	}
	for (i = 0; i < JOB_PRIO_MAX; i++)
	{
		while (this->jobs[i]->remove_first(this->jobs[i],
										   (void**)&job) == SUCCESS)
		{
			job->destroy(job);
		}
	}
	this->mutex->unlock(this->mutex);
}

METHOD(processor_t, destroy, void,
	private_processor_t *this)
{
	int i;

	cancel(this);
	this->thread_terminated->destroy(this->thread_terminated);
	this->job_added->destroy(this->job_added);
	this->mutex->destroy(this->mutex);
	for (i = 0; i < JOB_PRIO_MAX; i++)
	{
		this->jobs[i]->destroy(this->jobs[i]);
	}
	this->threads->destroy(this->threads);
	free(this);
}

/*
 * Described in header.
 */
processor_t *processor_create()
{
	private_processor_t *this;
	int i;

	INIT(this,
		.public = {
			.get_total_threads = _get_total_threads,
			.get_idle_threads = _get_idle_threads,
			.get_working_threads = _get_working_threads,
			.get_job_load = _get_job_load,
			.queue_job = _queue_job,
			.execute_job = _execute_job,
			.set_threads = _set_threads,
			.cancel = _cancel,
			.destroy = _destroy,
		},
		.threads = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.job_added = condvar_create(CONDVAR_TYPE_DEFAULT),
		.thread_terminated = condvar_create(CONDVAR_TYPE_DEFAULT),
	);
	for (i = 0; i < JOB_PRIO_MAX; i++)
	{
		this->jobs[i] = linked_list_create();
		this->prio_threads[i] = lib->settings->get_int(lib->settings,
						"%s.processor.priority_threads.%N", 0, lib->ns,
						job_priority_names, i);
	}

	return &this->public;
}
