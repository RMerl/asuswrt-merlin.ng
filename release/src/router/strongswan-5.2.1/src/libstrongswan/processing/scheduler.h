/*
 * Copyright (C) 2009 Tobias Brunner
 * Copyright (C) 2005-2007 Martin Willi
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

/**
 * @defgroup scheduler scheduler
 * @{ @ingroup processing
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

typedef struct scheduler_t scheduler_t;

#include <library.h>
#include <processing/jobs/job.h>

/**
 * The scheduler queues timed events which are then passed to the processor.
 *
 * The scheduler is implemented as a heap. A heap is a special kind of tree-
 * based data structure that satisfies the following property: if B is a child
 * node of A, then key(A) >= (or <=) key(B). So either the element with the
 * greatest (max-heap) or the smallest (min-heap) key is the root of the heap.
 * We use a min-heap with the key being the absolute unix time at which an
 * event is scheduled. So the root is always the event that will fire next.
 *
 * An earlier implementation of the scheduler used a sorted linked list to store
 * the events. That had the advantage that removing the next event was extremely
 * fast, also, adding an event scheduled before or after all other events was
 * equally fast (all in O(1)). The problem was, though, that adding an event
 * in-between got slower, as the number of events grew larger (O(n)).
 * For each connection there could be several events: IKE-rekey, NAT-keepalive,
 * retransmissions, expire (half-open), and others. So a gateway that probably
 * has to handle thousands of concurrent connnections has to be able to queue a
 * large number of events as fast as possible. Locking makes this even worse, to
 * provide thread-safety, no events can be processed, while an event is queued,
 * so making the insertion fast is even more important.
 *
 * That's the advantage of the heap. Adding an element to the heap can be
 * achieved in O(log n) - on the other hand, removing the root node also
 * requires O(log n) operations. Consider 10000 queued events. Inserting a new
 * event in the list implementation required up to 10000 comparisons. In the
 * heap implementation, the worst case is about 13.3 comparisons. That's a
 * drastic improvement.
 *
 * The implementation itself uses a binary tree mapped to a one-based array to
 * store the elements. This reduces storage overhead and simplifies navigation:
 * the children of the node at position n are at position 2n and 2n+1 (likewise
 * the parent node of the node at position n is at position [n/2]). Thus,
 * navigating up and down the tree is reduced to simple index computations.
 *
 * Adding an element to the heap works as follows: The heap is always filled
 * from left to right, until a row is full, then the next row is filled. Mapped
 * to an array this gets as simple as putting the new element to the first free
 * position. In a one-based array that position equals the number of elements
 * currently stored in the heap. Then the heap property has to be restored, i.e.
 * the new element has to be "bubbled up" the tree until the parent node's key
 * is smaller or the element got the new root of the tree.
 *
 * Removing the next event from the heap works similarly. The event itself is
 * the root node and stored at position 1 of the array. After removing it, the
 * root has to be replaced and the heap property has to be restored. This is
 * done by moving the bottom element (last row, rightmost element) to the root
 * and then "seep it down" by swapping it with child nodes until none of the
 * children has a smaller key or it is again a leaf node.
 */
struct scheduler_t {

	/**
	 * Adds a event to the queue, using a relative time offset in s.
	 *
	 * @param job			job to schedule
	 * @param time			relative time to schedule job, in s
	 */
	void (*schedule_job) (scheduler_t *this, job_t *job, u_int32_t s);

	/**
	 * Adds a event to the queue, using a relative time offset in ms.
	 *
	 * @param job			job to schedule
	 * @param time			relative time to schedule job, in ms
	 */
	void (*schedule_job_ms) (scheduler_t *this, job_t *job, u_int32_t ms);

	/**
	 * Adds a event to the queue, using an absolut time.
	 *
	 * The passed timeval should be calculated based on the time_monotonic()
	 * function.
	 *
	 * @param job			job to schedule
	 * @param time			absolut time to schedule job
	 */
	void (*schedule_job_tv) (scheduler_t *this, job_t *job, timeval_t tv);

	/**
	 * Returns number of jobs scheduled.
	 *
	 * @return				number of scheduled jobs
	 */
	u_int (*get_job_load) (scheduler_t *this);

	/**
	 * Destroys a scheduler object.
	 */
	void (*destroy) (scheduler_t *this);
};

/**
 * Create a scheduler.
 *
 * @return		scheduler_t object
 */
scheduler_t *scheduler_create(void);

#endif /** SCHEDULER_H_ @}*/
