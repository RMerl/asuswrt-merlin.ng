/*
 * Copyright (C) 2006-2017 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005 Jan Hutter
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

/*
 * Copyright (C) 2016 secunet Security Networks AG
 * Copyright (C) 2016 Thomas Egerer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @defgroup libcharon libcharon
 *
 * @defgroup attributes attributes
 * @ingroup libcharon
 *
 * @defgroup bus bus
 * @ingroup libcharon
 *
 * @defgroup listeners listeners
 * @ingroup bus
 *
 * @defgroup config config
 * @ingroup libcharon
 *
 * @defgroup control control
 * @ingroup libcharon
 *
 * @defgroup encoding encoding
 * @ingroup libcharon
 *
 * @defgroup payloads payloads
 * @ingroup encoding
 *
 * @defgroup kernel kernel
 * @ingroup libcharon
 *
 * @defgroup network network
 * @ingroup libcharon
 *
 * @defgroup cplugins plugins
 * @ingroup libcharon
 *
 * @defgroup cprocessing processing
 * @ingroup libcharon
 *
 * @defgroup cjobs jobs
 * @ingroup cprocessing
 *
 * @defgroup sa sa
 * @ingroup libcharon
 *
 * @defgroup ikev1 ikev1
 * @ingroup sa
 *
 * @defgroup ikev2 ikev2
 * @ingroup sa
 *
 * @defgroup authenticators_v1 authenticators
 * @ingroup ikev1
 *
 * @defgroup authenticators_v2 authenticators
 * @ingroup ikev2
 *
 * @defgroup eap eap
 * @ingroup sa
 *
 * @defgroup xauth xauth
 * @ingroup sa
 *
 * @defgroup tasks_v1 tasks
 * @ingroup ikev1
 *
 * @defgroup tasks_v2 tasks
 * @ingroup ikev2
 *
 * @addtogroup libcharon
 * @{
 *
 * IKEv2 keying daemon.
 *
 * All IKEv2 stuff is handled in charon. It uses a newer and more flexible
 * architecture than pluto. Charon uses a thread-pool (called processor),
 * which allows parallel execution SA-management. All threads originate
 * from the processor. Work is delegated to the processor by queueing jobs
 * to it.
   @verbatim

      +---------------------------------+       +----------------------------+
      |           controller            |       |          config            |
      +---------------------------------+       +----------------------------+
               |      |      |                           ^     ^    ^
               V      V      V                           |     |    |

       +----------+  +-----------+   +------+            +----------+    +----+
       | receiver |  |           |   |      |  +------+  | CHILD_SA |    | K  |
       +---+------+  | Scheduler |   | IKE- |  | IKE- |--+----------+    | e  |
           |         |           |   | SA   |--| SA   |  | CHILD_SA |    | r  |
    +------+---+     +-----------+   |      |  +------+  +----------+    | n  |
 <->|  socket  |           |         | Man- |                            | e  |
    +------+---+     +-----------+   | ager |  +------+  +----------+    | l  |
           |         |           |   |      |  | IKE- |--| CHILD_SA |    | -  |
       +---+------+  | Processor |---|      |--| SA   |  +----------+    | I  |
       |  sender  |  |           |   |      |  +------+                  | f  |
       +----------+  +-----------+   +------+                            +----+

               |      |      |                        |      |      |
               V      V      V                        V      V      V
      +---------------------------------+       +----------------------------+
      |               Bus               |       |         credentials        |
      +---------------------------------+       +----------------------------+

   @endverbatim
 * The scheduler is responsible to execute timed events. Jobs may be queued to
 * the scheduler to get executed at a defined time (e.g. rekeying). The
 * scheduler does not execute the jobs itself, it queues them to the processor.
 *
 * The IKE_SA manager managers all IKE_SA. It further handles the
 * synchronization:
 * Each IKE_SA must be checked out strictly and checked in again after use. The
 * manager guarantees that only one thread may check out a single IKE_SA. This
 * allows us to write the (complex) IKE_SAs routines non-threadsave.
 * The IKE_SA contain the state and the logic of each IKE_SA and handle the
 * messages.
 *
 * The CHILD_SA contains state about a IPsec security association and manages
 * them. An IKE_SA may have multiple CHILD_SAs. Communication to the kernel
 * takes place here through the kernel interface.
 *
 * The kernel interface installs IPsec security associations, policies, routes
 * and virtual addresses. It further provides methods to enumerate interfaces
 * and may notify the daemon about state changes at lower layers.
 *
 * The bus receives signals from the different threads and relays them to
 * interested listeners. Debugging signals, but also important state changes or
 * error messages are sent over the bus.
 * Its listeners are not only for logging, but also to track the state of an
 * IKE_SA.
 *
 * The controller, credential_manager, bus and backend_manager (config) are
 * places where a plugin ca register itself to provide information or observe
 * and control the daemon.
 */

#ifndef DAEMON_H_
#define DAEMON_H_

typedef struct daemon_t daemon_t;

#include <attributes/attribute_manager.h>
#include <kernel/kernel_interface.h>
#include <network/sender.h>
#include <network/receiver.h>
#include <network/socket_manager.h>
#include <control/controller.h>
#include <bus/bus.h>
#include <bus/listeners/custom_logger.h>
#include <sa/ike_sa_manager.h>
#include <sa/child_sa_manager.h>
#include <sa/trap_manager.h>
#include <sa/shunt_manager.h>
#include <sa/redirect_manager.h>
#include <config/backend_manager.h>
#include <sa/eap/eap_manager.h>
#include <sa/xauth/xauth_manager.h>

#ifdef ME
#include <sa/ikev2/connect_manager.h>
#include <sa/ikev2/mediation_manager.h>
#endif /* ME */

/**
 * Number of threads in the thread pool, if not specified in config.
 */
#define DEFAULT_THREADS 16

/**
 * Primary UDP port used by IKE.
 */
#define IKEV2_UDP_PORT 500

/**
 * UDP port defined for use in case a NAT is detected.
 */
#define IKEV2_NATT_PORT 4500

/**
 * UDP port on which the daemon will listen for incoming traffic (also used as
 * source port for outgoing traffic).
 */
#ifndef CHARON_UDP_PORT
#define CHARON_UDP_PORT IKEV2_UDP_PORT
#endif

/**
 * UDP port used by the daemon in case a NAT is detected.
 */
#ifndef CHARON_NATT_PORT
#define CHARON_NATT_PORT IKEV2_NATT_PORT
#endif

/**
 * Main class of daemon, contains some globals.
 */
struct daemon_t {

	/**
	 * Socket manager instance
	 */
	socket_manager_t *socket;

	/**
	 * Kernel interface to communicate with kernel
	 */
	kernel_interface_t *kernel;

	/**
	 * A ike_sa_manager_t instance.
	 */
	ike_sa_manager_t *ike_sa_manager;

	/**
	 * A child_sa_manager_t instance.
	 */
	child_sa_manager_t *child_sa_manager;

	/**
	 * Manager for triggering policies, called traps
	 */
	trap_manager_t *traps;

	/**
	 * Manager for shunt PASS|DROP policies
	 */
	shunt_manager_t *shunts;

	/**
	 * Manager for IKE redirect providers
	 */
	redirect_manager_t *redirect;

	/**
	 * Manager for the different configuration backends.
	 */
	backend_manager_t *backends;

	/**
	 * The Sender-Thread.
	 */
	sender_t *sender;

	/**
	 * The Receiver-Thread.
	 */
	receiver_t *receiver;

	/**
	 * Manager for IKE configuration attributes
	 */
	attribute_manager_t *attributes;

	/**
	 * The signaling bus.
	 */
	bus_t *bus;

	/**
	 * Controller to control the daemon
	 */
	controller_t *controller;

	/**
	 * EAP manager to maintain registered EAP methods
	 */
	eap_manager_t *eap;

	/**
	 * XAuth manager to maintain registered XAuth methods
	 */
	xauth_manager_t *xauth;

#ifdef ME
	/**
	 * Connect manager
	 */
	connect_manager_t *connect_manager;

	/**
	 * Mediation manager
	 */
	mediation_manager_t *mediation_manager;
#endif /* ME */

	/**
	 * Initialize the daemon.
	 *
	 * @param plugins	list of plugins to load
	 * @return			TRUE, if successful
	 */
	bool (*initialize)(daemon_t *this, char *plugins);

	/**
	 * Starts the daemon, i.e. spawns the threads of the thread pool.
	 */
	void (*start)(daemon_t *this);

	/**
	 * Load/Reload loggers defined in strongswan.conf
	 *
	 * If none are defined in strongswan.conf default loggers configured via
	 * set_default_loggers() are loaded.
	 */
	void (*load_loggers)(daemon_t *this);

	/**
	 * Configure default loggers if none are defined in strongswan.conf
	 *
	 * @param levels	debug levels used to create default loggers if none are
	 *					defined in strongswan.conf (NULL to disable)
	 * @param to_stderr	TRUE to log to stderr/stdout if no loggers are defined
	 * 					in strongswan.conf (logging to syslog is always enabled)
	 */
	void (*set_default_loggers)(daemon_t *this, level_t levels[DBG_MAX],
								bool to_stderr);

	/**
	 * Set the log level for the given log group for all loaded loggers.
	 *
	 * This change is not persistent and gets reset if loggers are reloaded
	 * via load_loggers().
	 *
	 * @param group		log group
	 * @param level		log level
	 */
	void (*set_level)(daemon_t *this, debug_t group, level_t level);
};

/**
 * The one and only instance of the daemon.
 *
 * Set between libcharon_init() and libcharon_deinit() calls.
 */
extern daemon_t *charon;

/**
 * Initialize libcharon and create the "charon" instance of daemon_t.
 *
 * This function initializes the bus, listeners can be registered before
 * calling initialize().
 *
 * libcharon_init() may be called multiple times in a single process, but each
 * caller must call libcharon_deinit() for each call to libcharon_init().
 *
 * @return		FALSE if integrity check failed
 */
bool libcharon_init();

/**
 * Deinitialize libcharon and destroy the "charon" instance of daemon_t.
 */
void libcharon_deinit();

/**
 * Register a custom logger constructor.
 *
 * To be called from __attribute__((constructor)) functions.
 *
 * @param name				name of the logger (also used for loglevel config)
 * @param constructor		constructor to create custom logger
 */
void register_custom_logger(char *name,
							custom_logger_constructor_t constructor);

#endif /** DAEMON_H_ @}*/
