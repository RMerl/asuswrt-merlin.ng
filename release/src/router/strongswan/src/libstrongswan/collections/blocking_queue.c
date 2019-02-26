/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
 * HSR Hochschule fuer Technik Rapperswil
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

#include "blocking_queue.h"

#include <threading/mutex.h>
#include <threading/thread.h>
#include <threading/condvar.h>
#include <collections/linked_list.h>

typedef struct private_blocking_queue_t private_blocking_queue_t;

/**
 * Private data of a blocking_queue_t object.
 */
struct private_blocking_queue_t {

	/**
	 * Public part
	 */
	blocking_queue_t public;

	/**
	 * Linked list containing all items in the queue
	 */
	linked_list_t *list;

	/**
	 * Mutex used to synchronize access to the queue
	 */
	mutex_t *mutex;

	/**
	 * Condvar used to wait for items
	 */
	condvar_t *condvar;

};

METHOD(blocking_queue_t, enqueue, void,
	private_blocking_queue_t *this, void *item)
{
	this->mutex->lock(this->mutex);
	this->list->insert_first(this->list, item);
	this->condvar->signal(this->condvar);
	this->mutex->unlock(this->mutex);
}

METHOD(blocking_queue_t, dequeue, void*,
	private_blocking_queue_t *this)
{
	bool oldstate;
	void *item;


	this->mutex->lock(this->mutex);
	thread_cleanup_push((thread_cleanup_t)this->mutex->unlock, this->mutex);
	/* ensure that a canceled thread does not dequeue any items */
	thread_cancellation_point();
	while (this->list->remove_last(this->list, &item) != SUCCESS)
	{
		oldstate = thread_cancelability(TRUE);
		this->condvar->wait(this->condvar, this->mutex);
		thread_cancelability(oldstate);
	}
	thread_cleanup_pop(TRUE);
	return item;
}

METHOD(blocking_queue_t, destroy, void,
	private_blocking_queue_t *this)
{
	this->list->destroy(this->list);
	this->condvar->destroy(this->condvar);
	this->mutex->destroy(this->mutex);
	free(this);
}

METHOD(blocking_queue_t, destroy_offset, void,
	private_blocking_queue_t *this, size_t offset)
{
	this->list->invoke_offset(this->list, offset);
	destroy(this);
}

METHOD(blocking_queue_t, destroy_function, void,
	private_blocking_queue_t *this, void (*fn)(void*))
{
	this->list->invoke_function(this->list, (linked_list_invoke_t)fn);
	destroy(this);
}

/*
 * Described in header.
 */
blocking_queue_t *blocking_queue_create()
{
	private_blocking_queue_t *this;

	INIT(this,
		.public = {
			.enqueue = _enqueue,
			.dequeue = _dequeue,
			.destroy = _destroy,
			.destroy_offset = _destroy_offset,
			.destroy_function = _destroy_function,
		},
		.list = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.condvar = condvar_create(CONDVAR_TYPE_DEFAULT),
	);

	return &this->public;
}

