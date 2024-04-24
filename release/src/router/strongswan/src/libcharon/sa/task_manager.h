/*
 * Copyright (C) 2013-2023 Tobias Brunner
 * Copyright (C) 2006 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup task_manager task_manager
 * @{ @ingroup sa
 */

#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

typedef struct task_manager_t task_manager_t;
typedef enum task_queue_t task_queue_t;
typedef struct retransmission_t retransmission_t;

#include <limits.h>

#include <library.h>
#include <encoding/message.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * First retransmit timeout in seconds.
 */
#define RETRANSMIT_TIMEOUT 4.0

/**
 * Base which is raised to the power of the retransmission try.
 */
#define RETRANSMIT_BASE 1.8

/**
 * Number of retransmits done before giving up.
 */
#define RETRANSMIT_TRIES 5

/**
 * Maximum jitter in percent.
 */
#define RETRANSMIT_JITTER_MAX 20

/**
 * Interval for mobike routability checks in ms.
 */
#define ROUTABILITY_CHECK_INTERVAL 2500

/**
 * Number of routability checks before giving up
 */
#define ROUTABILITY_CHECK_TRIES 10

/**
 * Retransmission settings.
 *
 * See retransmission_timeout() for details on the calculation.
 */
struct retransmission_t {

	/**
	 * Timeout in seconds.
	 */
	double timeout;

	/**
	 * Base that's raised to the power of the retransmission try to calculate
	 * the timeout.
	 */
	double base;

	/**
	 * Limit for the calculated timeout (in ms).
	 */
	uint32_t limit;

	/**
	 * Maximum jitter to apply to calculated timeout (in percent). A random
	 * amount of that value will be subtracted from the calculated timeout.
	 */
	u_int jitter;

	/**
	 * Number of tries.
	 */
	u_int tries;

	/**
	 * Maximum number of tries possible with current retransmission settings
	 * before overflowing the range of uint32_t, which we use for the timeout.
	 * Note that UINT32_MAX milliseconds equal nearly 50 days, so using a high
	 * number of retransmits doesn't make much sense without `limit` anyway.
	 */
	u_int max_tries;
};

/**
 * Type of task queues the task manager uses to handle tasks
 */
enum task_queue_t {
	/** tasks currently active, initiated by us */
	TASK_QUEUE_ACTIVE,
	/** passive tasks initiated by the remote peer */
	TASK_QUEUE_PASSIVE,
	/** tasks queued for initiated, but not yet activated */
	TASK_QUEUE_QUEUED,
};

/**
 * The task manager, juggles task and handles message exchanges.
 *
 * On incoming requests, the task manager creates new tasks on demand and
 * juggles the request through all available tasks. Each task inspects the
 * request and adds payloads as necessary to the response.
 * On outgoing requests, the task manager delivers the request through the tasks
 * to build it, the response gets processed by each task to complete.
 * The task manager has an internal Queue to store task which should get
 * completed.
 * For the initial IKE_SA setup, several tasks are queued: One for the
 * unauthenticated IKE_SA setup, one for authentication, one for CHILD_SA setup
 * and maybe one for virtual IP assignment.
 * The task manager is also responsible for retransmission (see
 * retransmission_timeout() for details).
 */
struct task_manager_t {

	/**
	 * Process an incoming message.
	 *
	 * @param message		message to add payloads to
	 * @return
	 *						- DESTROY_ME if IKE_SA must be closed
	 *						- SUCCESS otherwise
	 */
	status_t (*process_message) (task_manager_t *this, message_t *message);

	/**
	 * Initiate an exchange with the currently queued tasks.
	 */
	status_t (*initiate) (task_manager_t *this);

	/**
	 * Queue a task in the manager.
	 *
	 * @param task			task to queue
	 */
	void (*queue_task)(task_manager_t *this, task_t *task);

	/**
	 * Queue a task in the manager, but delay its initiation for at least the
	 * given number of seconds.
	 *
	 * @param task			task to queue
	 * @param delay			minimum delay in s before initiating the task
	 */
	void (*queue_task_delayed)(task_manager_t *this, task_t *task,
							   uint32_t delay);

	/**
	 * Queue IKE_SA establishing tasks.
	 */
	void (*queue_ike)(task_manager_t *this);

	/**
	 * Queue IKE_SA rekey tasks.
	 */
	void (*queue_ike_rekey)(task_manager_t *this);

	/**
	 * Queue IKE_SA reauth tasks.
	 */
	void (*queue_ike_reauth)(task_manager_t *this);

	/**
	 * Queue MOBIKE task
	 *
	 * @param roam			TRUE to switch to new address
	 * @param address		TRUE to include address list update
	 */
	void (*queue_mobike)(task_manager_t *this, bool roam, bool address);

	/**
	 * Queue IKE_SA delete tasks.
	 */
	void (*queue_ike_delete)(task_manager_t *this);

	/**
	 * Queue CHILD_SA establishing tasks.
	 *
	 * @param cfg			CHILD_SA config to establish
	 * @param args			optional arguments for the initiation
	 */
	void (*queue_child)(task_manager_t *this, child_cfg_t *cfg,
						child_init_args_t *args);

	/**
	 * Queue CHILD_SA rekeying tasks.
	 *
	 * @param protocol		CHILD_SA protocol, AH|ESP
	 * @param spi			CHILD_SA SPI to rekey
	 */
	void (*queue_child_rekey)(task_manager_t *this, protocol_id_t protocol,
							  uint32_t spi);

	/**
	 * Queue CHILD_SA delete tasks.
	 *
	 * @param protocol		CHILD_SA protocol, AH|ESP
	 * @param spi			CHILD_SA SPI to rekey
	 * @param expired		TRUE if SA already expired
	 */
	void (*queue_child_delete)(task_manager_t *this, protocol_id_t protocol,
							   uint32_t spi, bool expired);

	/**
	 * Queue liveness checking tasks.
	 */
	void (*queue_dpd)(task_manager_t *this);

	/**
	 * Retransmit a request if it hasn't been acknowledged yet.
	 *
	 * A return value of INVALID_STATE means that the message was already
	 * acknowledged and has not to be retransmitted. A return value of SUCCESS
	 * means retransmission was required and the message has been resent.
	 *
	 * @param message_id	ID of the message to retransmit
	 * @return
	 *						- INVALID_STATE if retransmission not required
	 *						- SUCCESS if retransmission sent
	 */
	status_t (*retransmit) (task_manager_t *this, uint32_t message_id);

	/**
	 * Migrate all queued tasks from other to this.
	 *
	 * To rekey or reestablish an IKE_SA completely, all queued or active
	 * tasks should get migrated to the new IKE_SA.
	 *
	 * @param other			manager which gives away its tasks
	 */
	void (*adopt_tasks) (task_manager_t *this, task_manager_t *other);

	/**
	 * Increment a message ID counter, in- or outbound.
	 *
	 * If a message is processed outside of the manager, this call increments
	 * the message ID counters of the task manager.
	 *
	 * @param initiate		TRUE to increment the initiating ID
	 */
	void (*incr_mid)(task_manager_t *this, bool initiate);

	/**
	 * Get the current message ID counter, in- or outbound.
	 *
	 * @param initiate		TRUE to get the initiating ID
	 * @return				current message ID
	 */
	uint32_t (*get_mid)(task_manager_t *this, bool initiate);

	/**
	 * Reset message ID counters of the task manager.
	 *
	 * The IKEv2 protocol requires to restart exchanges with message IDs
	 * reset to zero (INVALID_KE_PAYLOAD, COOKIES, ...). The reset() method
	 * resets the message IDs and resets all active tasks using the migrate()
	 * method.
	 * Use a value of UINT_MAX to keep the current message ID.
	 * For IKEv1, the arguments do not set the message ID, but the DPD sequence
	 * number counters.
	 *
	 * @param initiate		message ID / DPD seq to initiate exchanges (send)
	 * @param respond		message ID / DPD seq to respond to exchanges (expect)
	 */
	void (*reset)(task_manager_t *this, uint32_t initiate, uint32_t respond);

	/**
	 * Check if we are currently waiting for a reply.
	 *
	 * @return				TRUE if we are waiting, FALSE otherwise
	 */
	bool (*busy) (task_manager_t *this);

	/**
	 * Create an enumerator over tasks in a specific queue.
	 *
	 * @param queue			queue to create an enumerator over
	 * @return				enumerator over task_t
	 */
	enumerator_t* (*create_task_enumerator)(task_manager_t *this,
											task_queue_t queue);

	/**
	 * Remove the task the given enumerator points to.
	 *
	 * @note This should be used with caution, in particular, for tasks in the
	 * active and passive queues.
	 *
	 * @param enumerator	enumerator created with the method above
	 */
	void (*remove_task)(task_manager_t *this, enumerator_t *enumerator);

	/**
	 * Flush all tasks, regardless of the queue.
	 */
	void (*flush)(task_manager_t *this);

	/**
	 * Flush a queue, canceling all tasks.
	 *
	 * @param queue			queue to flush
	 */
	void (*flush_queue)(task_manager_t *this, task_queue_t queue);

	/**
	 * Destroy the task_manager_t.
	 */
	void (*destroy) (task_manager_t *this);
};

/**
 * Parse the default retransmission settings.
 *
 * @param settings			retransmission settings that are filled in
 */
void retransmission_parse_default(retransmission_t *settings);

/**
 * Calculate the retransmission timeout in ms based on the given settings and
 * try.
 *
 * An exponential backoff algorithm is used. The timeout for each retransmit
 * is calculated as follows: min(timeout * (base ** try), limit)
 * From the result a random value (of at most jitter percent) is optionally
 * subtracted.
 *
 * Using an initial timeout of 4s, a base of 1.8, and 5 tries gives us:
 * @verbatim
                   | relative | absolute
   ---------------------------------------------------------
   4s * (1.8 ** 0) =    4s         4s
   4s * (1.8 ** 1) =    7s        11s
   4s * (1.8 ** 2) =   13s        24s
   4s * (1.8 ** 3) =   23s        47s
   4s * (1.8 ** 4) =   42s        89s
   4s * (1.8 ** 5) =   76s       165s

   @endverbatim
 * So the peer is considered dead after 2min 45s when no reply is received.
 *
 * @param settings			retransmission settings
 * @param try				zero-based try to send a packet
 * @param randomize			whether to apply jitter
 * @return					timeout for next retransmit in ms
 */
uint32_t retransmission_timeout(retransmission_t *settings, u_int try,
								bool randomize);

/**
 * Calculate total timeout in s for the given retransmission settings (ignoring
 * jitter).
 *
 * This is affected by modifications of base, timeout, limit and tries. The
 * resulting value can then be used e.g. in kernel plugins to set the system's
 * acquire timeout properly.
 *
 * @param settings			retransmission settings
 * @return					calculated total retransmission timeout in seconds
 */
u_int retransmission_timeout_total(retransmission_t *settings);

/**
 * Create a task manager instance for the correct IKE version.
 *
 * @param ike_sa			IKE_SA to create a task manager for
 * @return					task manager implementation for IKE version
 */
task_manager_t *task_manager_create(ike_sa_t *ike_sa);

#endif /** TASK_MANAGER_H_ @}*/
