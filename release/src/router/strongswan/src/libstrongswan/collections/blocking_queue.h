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

/**
 * @defgroup blocking_queue blocking_queue
 * @{ @ingroup collections
 */

#ifndef BLOCKING_QUEUE_H_
#define BLOCKING_QUEUE_H_

typedef struct blocking_queue_t blocking_queue_t;

#include <library.h>

/**
 * Class implementing a synchronized blocking queue based on linked_list_t
 */
struct blocking_queue_t {

	/**
	 * Inserts a new item at the tail of the queue
	 *
	 * @param item		item to insert in queue
	 */
	void (*enqueue)(blocking_queue_t *this, void *item);

	/**
	 * Removes the first item in the queue and returns its value.
	 * If the queue is empty, this call blocks until a new item is inserted.
	 *
	 * @note This is a thread cancellation point
	 *
	 * @return			removed item
	 */
	void *(*dequeue)(blocking_queue_t *this);

	/**
	 * Destroys a blocking_queue_t object.
	 *
	 * @note No thread must wait in dequeue() when this function is called
	 */
	void (*destroy)(blocking_queue_t *this);

	/**
	 * Destroys a queue and its objects using the given destructor.
	 *
	 * If a queue and the contained objects should be destroyed, use
	 * destroy_offset. The supplied offset specifies the destructor to
	 * call on each object. The offset may be calculated using the offsetof
	 * macro, e.g.: queue->destroy_offset(queue, offsetof(object_t, destroy));
	 *
	 * @note No thread must wait in dequeue() when this function is called
	 *
	 * @param offset	offset of the objects destructor
	 */
	void (*destroy_offset)(blocking_queue_t *this, size_t offset);

	/**
	 * Destroys a queue and its objects using a cleanup function.
	 *
	 * If a queue and its contents should get destroyed using a specific
	 * cleanup function, use destroy_function. This is useful when the
	 * list contains malloc()-ed blocks which should get freed,
	 * e.g.: queue->destroy_function(queue, free);
	 *
	 * @note No thread must wait in dequeue() when this function is called
	 *
	 * @param function	function to call on each object
	 */
	void (*destroy_function)(blocking_queue_t *this, void (*)(void*));

};

/**
 * Creates an empty queue object.
 *
 * @return		blocking_queue_t object.
 */
blocking_queue_t *blocking_queue_create();

#endif /** BLOCKING_QUEUE_H_ @}*/

