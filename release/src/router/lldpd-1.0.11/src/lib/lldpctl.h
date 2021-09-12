/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef LLDPCTL_H
#define LLDPCTL_H

/**
 * @defgroup liblldpctl liblldpctl: library to interface with lldpd
 *
 * `liblldpctl` allows any program to convenienty query and modify the behaviour
 * of a running lldpd daemon.
 *
 * To use this library, use `pkg-config` to get the appropriate options:
 *   * `pkg-config --libs lldpctl` for `LIBS` or `LDFLAGS`
 *   * `pkg-config --cflags lldpctl` for `CFLAGS`
 *
 * @warning This library is tightly coupled with lldpd. The library to use
 *   should be the one shipped with lldpd. Clients of the library are then tied
 *   by the classic API/ABI rules and may be compiled separatly.
 *
 * There are two important structures in this library: @c lldpctl_conn_t which
 * represents a connection and @c lldpctl_atom_t which represents a piece of
 * information. Those types are opaque. No direct access to them should be done.
 *
 * The library is expected to be reentrant and therefore thread-safe. It is
 * however not expected that a connection to be used in several thread
 * simultaneously. This also applies to the different pieces of information
 * gathered through this connection. Several connection to lldpd can be used
 * simultaneously.
 *
 * The first step is to establish a connection. See @ref lldpctl_connection for
 * more information about this. The next step is to query the lldpd daemon. See
 * @ref lldpctl_atoms on how to do this.
 *
 * `liblldpctl` tries to handle errors in a coherent way. Any function returning
 * a pointer will return @c NULL on error and the last error can be retrieved
 * through @ref lldpctl_last_error() function. Most functions returning integers
 * will return a negative integer representing the error if something goes
 * wrong. The use of @ref lldpctl_last_error() allows one to check if this is a
 * real error if there is a doubt. See @ref lldpctl_errors_logs for more about
 * this.
 *
 * @{
 */


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

/**
 * @defgroup lldpctl_connection Managing connection to lldpd
 *
 * Connection with lldpd.
 *
 * This library does not handle IO. They are delegated to a set of functions to
 * allow a user to specify exactly how IO should be done. A user is expected to
 * provide two functions: the first one is called when the library requests
 * incoming data, the other one when it requests outgoing data. Moreover, the
 * user is also expected to call the appropriate functions when data comes back
 * (@ref lldpctl_recv()) or needs to be sent (@ref lldpctl_send()).
 *
 * Because the most common case is synchronous IO, `liblldpctl` will use classic
 * synchronous IO with the Unix socket if no IO functions are provided by the
 * user. For all other cases, the user must provide the appropriate functions.
 *
 * A connection should be allocated by using @ref lldpctl_new(). It needs to be
 * released with @ref lldpctl_release().
 *
 * @{
 */

/**
 * Get default transport name.
 *
 * Currently, this is the default location of the Unix socket.
 */
const char* lldpctl_get_default_transport(void);

/**
 * Structure referencing a connection with lldpd.
 *
 * This structure should be handled as opaque. It can be allocated
 * with @c lldpctl_new() and the associated resources will be freed
 * with @c lldpctl_release().
 */
typedef struct lldpctl_conn_t lldpctl_conn_t;

/**
 * Callback function invoked to send data to lldpd.
 *
 * @param conn      Handle to the connection to lldpd.
 * @param data      Bytes to be sent.
 * @param length    Length of provided data.
 * @param user_data Provided user data.
 * @return The number of bytes really sent or either @c LLDPCTL_ERR_WOULDBLOCK
 *         if no bytes can be sent without blocking or @c
 *         LLDPCTL_ERR_CALLBACK_FAILURE for other errors.
 */
typedef ssize_t (*lldpctl_send_callback)(lldpctl_conn_t *conn,
    const uint8_t *data, size_t length, void *user_data);

/**
 * Callback function invoked to receive data from lldpd.
 *
 * @param conn      Handle to the connection to lldpd.
 * @param data      Buffer for receiving data
 * @param length    Maximum bytes we can receive
 * @param user_data Provided user data.
 * @return The number of bytes really received or either @c
 *         LLDPCTL_ERR_WOULDBLOCK if no bytes can be received without blocking,
 *         @c LLDPCTL_ERR_CALLBACK_FAILURE for other errors or @c
 *         LLDPCTL_ERR_EOF if end of file was reached.
 */
typedef ssize_t (*lldpctl_recv_callback)(lldpctl_conn_t *conn,
    const uint8_t *data, size_t length, void *user_data);

/**
 * Function invoked when additional data is available from lldpd.
 *
 * This function should be invoked in case of asynchronous IO when new data is
 * available from lldpd (expected or unexpected).
 *
 * @param  conn      Handle to the connection to lldpd.
 * @param  data      Data received from lldpd.
 * @param  length    Length of data received.
 * @return The number of bytes available or a negative integer if an error has
 *         occurred. 0 is not an error. It usually means that a notification has
 *         been processed.
 */
ssize_t lldpctl_recv(lldpctl_conn_t *conn, const uint8_t *data, size_t length);

/**
 * Function invoked when there is an opportunity to send data to lldpd.
 *
 * This function should be invoked in case of asynchronous IO when new data can
 * be written to lldpd.
 *
 * @param  conn  Handle to the connection to lldpd.
 * @return The number of bytes processed or a negative integer if an error has
 *         occurred.
 */
ssize_t lldpctl_send(lldpctl_conn_t *conn);

/**
 * Function invoked to see if there's more data to be processed in the buffer.
 *
 * This function should be invoked to check for notifications in the data that
 * has already been read. Its used typically for asynchronous connections.
 *
 * @param  conn  Handle to the connection to lldpd.
 * @return 0 to indicate maybe more data is available for processing
 *         !0 to indicate no data or insufficient data for processing
 */
int lldpctl_process_conn_buffer(lldpctl_conn_t *conn);


/**
 * Allocate a new handler for connecting to lldpd.
 *
 * @param  send      Callback to be used when sending   new data is requested.
 * @param  recv      Callback to be used when receiving new data is requested.
 * @param  user_data Data to pass to callbacks.
 * @return An handler to be used to connect to lldpd or @c NULL in
 *         case of error. In the later case, the error is probable an
 *         out of memory condition.
 *
 * The allocated handler can be released with @c lldpctl_release(). If the
 * provided parameters are both @c NULL, default synchronous callbacks will be
 * used.
 */
lldpctl_conn_t *lldpctl_new(lldpctl_send_callback send,
    lldpctl_recv_callback recv, void *user_data);

/**
 * Allocate a new handler for connecting to lldpd.
 *
 * @param  ctlname   the Unix-domain socket to connect to lldpd.
 * @param  send      Callback to be used when sending   new data is requested.
 * @param  recv      Callback to be used when receiving new data is requested.
 * @param  user_data Data to pass to callbacks.
 * @return An handler to be used to connect to lldpd or @c NULL in
 *         case of error. In the later case, the error is probable an
 *         out of memory condition.
 *
 * The allocated handler can be released with @c lldpctl_release(). If the
 * provided parameters are both @c NULL, default synchronous callbacks will be
 * used.
 */
lldpctl_conn_t *lldpctl_new_name(const char *ctlname, lldpctl_send_callback send,
    lldpctl_recv_callback recv, void *user_data);

/**
 * Release resources associated with a connection to lldpd.
 *
 * @param   conn Previously allocated handler to a connection to lldpd.
 * @return  0 on success or a negative integer
 *
 * @see lldpctl_new()
 */
int lldpctl_release(lldpctl_conn_t *conn);
/**@}*/

/**
 * @defgroup lldpctl_errors_logs Errors and logs handling
 *
 * Error codes and logs handling.
 *
 * When a function returns a pointer, it may return @c NULL to indicate an error
 * condition. In this case, it is possible to use @ref lldpctl_last_error() to
 * get the related error code which is one of the values in @ref lldpctl_error_t
 * enumeration. For display purpose @ref lldpctl_strerror() may be used to
 * translate this error code.
 *
 * When a function returns an integer, it may return a negative value. It
 * usually means this is an error but some functions may return a legitimate
 * negative value (for example @ref lldpctl_atom_get_int()). When there is a
 * doubt, @ref lldpctl_last_error() should be checked.
 *
 * An error is attached to a connection. If there is no connection, no error
 * handling is available. Most functions use a connection or an atom as first
 * argument and therefore are attached to a connection. To get the connection
 * related to an atom, use @ref lldpctl_atom_get_connection().
 *
 * Also have a look at @ref lldpctl_log_callback() function if you want a custom
 * log handling.
 *
 * @{
 */

/**
 * Setup log handlers.
 *
 * By default, liblldpctl will log to stderr. The following function will
 * register another callback for this purpose. Messages logged through this
 * callback may be cryptic. They are targeted for the developer. Message for end
 * users should rely on return codes.
 */
void lldpctl_log_callback(void (*cb)(int severity, const char *msg));

/**
 * Setup log level.
 *
 * By default, liblldpctl will only log warnings. The following function allows
 * to increase verbosity. This function has no effect if callbacks are
 * registered with the previous function.
 *
 * @param level    Level of verbosity (1 = warnings, 2 = info, 3 = debug).
 */
void lldpctl_log_level(int level);

/**
 * Possible error codes for functions that return negative integers on
 * this purpose or for @c lldpctl_last_error().
 */
typedef enum {
	/**
	 * No error has happened (yet).
	 */
	LLDPCTL_NO_ERROR = 0,
	/**
	 * A IO related operation would block if performed.
	 */
	LLDPCTL_ERR_WOULDBLOCK = -501,
	/**
	 * A IO related operation has reached a end of file condition.
	 */
	LLDPCTL_ERR_EOF = -502,
	/**
	 * The requested information does not exist. For example, when
	 * requesting an inexistant information from an atom.
	 */
	LLDPCTL_ERR_NOT_EXIST = -503,
	/**
	 * Cannot connect to the lldpd daemon. This error only happens with
	 * default synchronous handlers.
	 */
	LLDPCTL_ERR_CANNOT_CONNECT = -504,
	/**
	 * Atom is of incorrect type for the requested operation.
	 */
	LLDPCTL_ERR_INCORRECT_ATOM_TYPE = -505,
	/**
	 * An error occurred during serialization of message.
	 */
	LLDPCTL_ERR_SERIALIZATION =  -506,
	/**
	 * The requested operation cannot be performed because we have another
	 * operation already running.
	 */
	LLDPCTL_ERR_INVALID_STATE =  -507,
	/**
	 * The provided atom cannot be iterated.
	 */
	LLDPCTL_ERR_CANNOT_ITERATE =  -508,
	/**
	 * The provided value is invalid.
	 */
	LLDPCTL_ERR_BAD_VALUE =  -509,
	/**
	 * No new element can be created for this element.
	 */
	LLDPCTL_ERR_CANNOT_CREATE =  -510,
	/**
	 * The library is under unexpected conditions and cannot process
	 * any further data reliably.
	 */
	LLDPCTL_ERR_FATAL = -900,
	/**
	 * Out of memory condition. Things may get havoc here but we
	 * should be able to recover.
	 */
	LLDPCTL_ERR_NOMEM = -901,
	/**
	 * An error occurred in a user provided callback.
	 */
	LLDPCTL_ERR_CALLBACK_FAILURE = -902
} lldpctl_error_t;

/**
 * Describe a provided error code.
 *
 * @param error Error code to be described.
 * @return Statically allocated string describing the error.
 */
const char *lldpctl_strerror(lldpctl_error_t error);

/**
 * Get the last error associated to a connection to lldpd.
 *
 * @param  conn Previously allocated handler to a connection to lldpd.
 * @return 0 if no error is currently registered. A negative integer
 *         otherwise.
 *
 * For functions returning int, this function will return the same
 * error number. For functions returning something else, you can use
 * this function to get the appropriate error number.
 */
lldpctl_error_t lldpctl_last_error(lldpctl_conn_t *conn);

/**
 * Describe the last error associate to a connection.
 *
 * @param conn Previously allocated handler to a connection to lldpd.
 * @return Statically allocated string describing the error
 */
#define lldpctl_last_strerror(conn) lldpctl_strerror(lldpctl_last_error(conn))
/**@}*/

/**
 * @defgroup lldpctl_atoms Extracting information: atoms
 *
 * Information retrieved from lldpd is represented as an atom.
 *
 * This is an opaque structure that can be passed along some functions to
 * transmit chassis, ports, VLAN and other information related to LLDP. Most
 * information are extracted using @c lldpctl_atom_get(), @c
 * lldpctl_atom_get_str(), @c lldpctl_atom_get_buffer() or @c
 * lldpctl_atom_get_int(), unless some IO with lldpd is needed to retrieve the
 * requested information. In this case, there exists an appropriate function to
 * convert the "deferred" atom into a normal one (like @c lldpctl_get_port()).
 *
 * For some information, setters are also available: @c lldpctl_atom_set(), @c
 * lldpctl_atom_set_str(), @c lldpctl_atom_set_buffer() or @c
 * lldpctl_atom_set_int(). Unlike getters, some of those may require IO to
 * achieve their goal.
 *
 * An atom is reference counted. The semantics are quite similar to Python and
 * you must be careful of the ownership of a reference. It is possible to own a
 * reference by calling @c lldpctl_atom_inc_ref(). Once the atom is not needed
 * any more, you can abandon ownership with @c lldpctl_atom_dec_ref(). Unless
 * documented otherwise, a function returning an atom will return a new
 * reference (the ownership is assigned to the caller, no need to call @c
 * lldpctl_atom_inc_ref()). Unless documented otherwise, when providing an atom
 * to a function, the atom is usually borrowed (no change in reference
 * counting). Currently, no function will steal ownership.
 *
 * It is quite important to use the reference counting functions
 * correctly. Segfaults or memory leaks may occur otherwise. Once the reference
 * count reaches 0, the atom is immediately freed. Reusing it will likely lead
 * to memory corruption.
 *
 * @{
 */

/**
 * Structure representing an element (chassis, port, VLAN, ...)
 *
 * @see lldpctl_atom_inc_ref(), lldpctl_atom_dec_ref().
 */
typedef struct lldpctl_atom_t lldpctl_atom_t;

/**
 * Structure representing a map from an integer to a character string.
 *
 * @see lldpctl_key_get_map().
 */
typedef const struct {
	int   value;
	char *string;
} lldpctl_map_t;

/**
 * Return the reference to connection with lldpd.
 *
 * @param atom The atom we want reference from.
 * @return The reference to the connection to lldpd.
 *
 * Each atom contains an internal reference to the corresponding connection to
 * lldpd. Use this function to get it.
 */
lldpctl_conn_t *lldpctl_atom_get_connection(lldpctl_atom_t *atom);

/**
 * Increment reference count for an atom.
 *
 * @param atom Atom we which to increase reference count.
 */
void lldpctl_atom_inc_ref(lldpctl_atom_t *atom);

/**
 * Decrement reference count for an atom.
 *
 * @param atom Atom we want to decrease reference count. Can be @c NULL. In this
 *             case, nothing happens.
 *
 * When the reference count becomes 0, the atom is freed.
 */
void lldpctl_atom_dec_ref(lldpctl_atom_t *atom);

/**
 * Possible events for a change (notification).
 *
 * @see lldpctl_watch_callback2
 */
typedef enum {
	lldpctl_c_deleted,	/**< The neighbor has been deleted */
	lldpctl_c_updated,	/**< The neighbor has been updated */
	lldpctl_c_added,	/**< This is a new neighbor */
} lldpctl_change_t;

/**
 * Callback function invoked when a change is detected.
 *
 * @param conn      Connection with lldpd. Should not be used.
 * @param type      Type of change detected.
 * @param interface Physical interface on which the change has happened.
 * @param neighbor  Changed neighbor.
 * @param data      Data provided when registering the callback.
 *
 * The provided interface and neighbor atoms are stolen by the callback: their
 * reference count are decremented when the callback ends. If you want to keep a
 * reference to it, be sure to increment the reference count in the callback.
 *
 * @warning The provided connection should not be used at all. Do not use @c
 * lldpctl_atom_set_*() functions on @c interface or @c neighbor either. If you
 * do, you will get a @c LLDPCTL_ERR_INVALID_STATE error.
 *
 * @see lldpctl_watch_callback
 */
typedef void (*lldpctl_change_callback)(lldpctl_conn_t *conn,
    lldpctl_change_t type,
    lldpctl_atom_t *interface,
    lldpctl_atom_t *neighbor,
    void *data);

/**
 * Callback function invoked when a change is detected.
 *
 * @param type      Type of change detected.
 * @param interface Physical interface on which the change has happened.
 * @param neighbor  Changed neighbor.
 * @param data      Data provided when registering the callback.
 *
 * The provided interface and neighbor atoms are stolen by the callback: their
 * reference count are decremented when the callback ends. If you want to keep a
 * reference to it, be sure to increment the reference count in the callback.
 *
 * @see lldpctl_watch_callback2
 */
typedef void (*lldpctl_change_callback2)(lldpctl_change_t type,
    lldpctl_atom_t *interface,
    lldpctl_atom_t *neighbor,
    void *data);

/**
 * Register a callback to be called on changes.
 *
 * @param conn Connection with lldpd.
 * @param cb   Replace the current callback with the provided one.
 * @param data Data that will be passed to the callback.
 * @return 0 in case of success or -1 in case of errors.
 *
 * This function will register the necessity to push neighbor changes to lldpd
 * and therefore will issue IO operations. The error code could then be @c
 * LLDPCTL_ERR_WOULDBLOCK.
 *
 * @warning Once a callback is registered, the connection shouldn't be used for
 * anything else than receiving notifications. If you do, you will get a @c
 * LLDPCTL_ERR_INVALID_STATE error.
 *
 * @deprecated This function is deprecated and lldpctl_watch_callback2 should be
 * used instead.
 */
int lldpctl_watch_callback(lldpctl_conn_t *conn,
    lldpctl_change_callback cb,
    void *data) __attribute__ ((deprecated));

/**
 * Register a callback to be called on changes.
 *
 * @param conn Connection with lldpd.
 * @param cb   Replace the current callback with the provided one.
 * @param data Data that will be passed to the callback.
 * @return 0 in case of success or -1 in case of errors.
 *
 * This function will register the necessity to push neighbor changes to lldpd
 * and therefore will issue IO operations. The error code could then be @c
 * LLDPCTL_ERR_WOULDBLOCK.
 *
 * @warning Once a callback is registered, the connection shouldn't be used for
 * anything else than receiving notifications. If you do, you will get a @c
 * LLDPCTL_ERR_INVALID_STATE error.
 */
int lldpctl_watch_callback2(lldpctl_conn_t *conn,
    lldpctl_change_callback2 cb,
    void *data);

/**
 * Wait for the next change.
 *
 * @param conn Connection with lldpd.
 * @return 0 on success or a negative integer in case of error.
 *
 * This function will return once a change has been detected. It is only useful
 * as a main loop when using the builtin blocking IO mechanism.
 */
int lldpctl_watch(lldpctl_conn_t *conn);

/**
 * @defgroup liblldpctl_atom_get_special Retrieving atoms from lldpd
 *
 * Special access functions.
 *
 * Most information can be retrieved through @ref lldpctl_atom_get(), @ref
 * lldpctl_atom_get_int(), @ref lldpctl_atom_get_str() or @ref
 * lldpctl_atom_get_buffer() but some information can only be retrieved through
 * special functions because IO operation is needed (and also, for some of them,
 * because we don't have an atom yet).
 *
 * @{
 */

/**
 * Retrieve global configuration of lldpd daemon.
 *
 * @param conn Connection with lldpd.
 * @return The global configuration or @c NULL if an error happened.
 *
 * This function will make IO with the daemon to get the
 * configuration. Depending on the IO model, information may not be available
 * right now and the function should be called again later. If @c NULL is
 * returned, check the last error. If it is @c LLDPCTL_ERR_WOULDBLOCK, try again
 * later.
 */
lldpctl_atom_t *lldpctl_get_configuration(lldpctl_conn_t *conn);

/**
 * Retrieve the list of available interfaces.
 *
 * @param conn Previously allocated handler to a connection to lldpd.
 * @return The list of available ports or @c NULL if an error happened.
 *
 * This function will make IO with the daemon to get the list of
 * ports. Depending on the IO model, information may not be available right now
 * and the function should be called again later. If @c NULL is returned, check
 * what the last error is. If it is @c LLDPCTL_ERR_WOULDBLOCK, try again later
 * (when more data is available).
 *
 * The list of available ports can be iterated with @ref lldpctl_atom_foreach().
 */
lldpctl_atom_t *lldpctl_get_interfaces(lldpctl_conn_t *conn);

/**
 * Retrieve the information related to the local chassis.
 *
 * @param conn Previously allocated handler to a connection to lldpd.
 * @return Atom related to the local chassis which may be used in subsequent functions.
 *
 * This function may have to do IO to get the information related to the local
 * chassis. Depending on the IO mode, information may not be available right now
 * and the function should be called again later. If @c NULL is returned, check
 * what the last error is. If it is @c LLDPCTL_ERR_WOULDBLOCK, try again later
 * (when more data is available).
 */
lldpctl_atom_t *lldpctl_get_local_chassis(lldpctl_conn_t *conn);

/**
 * Retrieve the information related to a given interface.
 *
 * @param port The port we want to retrieve information from. This port is an
 *             atom retrieved from an interation on @c lldpctl_get_interfaces().
 * @return Atom related to this port which may be used in subsequent functions.
 *
 * This function may have to do IO to get the information related to the given
 * port. Depending on the IO mode, information may not be available right now
 * and the function should be called again later. If @c NULL is returned, check
 * what the last error is. If it is @c LLDPCTL_ERR_WOULDBLOCK, try again later
 * (when more data is available).
 */
lldpctl_atom_t *lldpctl_get_port(lldpctl_atom_t *port);

/**
 * Retrieve the default port information.
 *
 * This port contains default settings whenever a new port needs to be created.
 *
 * @param conn Previously allocated handler to a connection to lldpd.
 * @return Atom of the default port which may be used in subsequent functions.
 *
 * This function may have to do IO to get the information related to the given
 * port. Depending on the IO mode, information may not be available right now
 * and the function should be called again later. If @c NULL is returned, check
 * what the last error is. If it is @c LLDPCTL_ERR_WOULDBLOCK, try again later
 * (when more data is available).
 */
lldpctl_atom_t *lldpctl_get_default_port(lldpctl_conn_t *conn);

/**@}*/

/**
 * Piece of information that can be retrieved from/written to an atom.
 *
 * Each piece of information can potentially be retrieved as an atom (A), a
 * string (S), a buffer (B) or an integer (I). Additionaly, when an information
 * can be retrieved as an atom, it is usually iterable (L). When an atom can be
 * retrieved as a string and as an additional type, the string is expected to be
 * formatted. For example, the MAC address of a local port can be retrieved as a
 * buffer and a string. As a string, you'll get something like
 * "00:11:22:33:44:55". Also, all values that can be get as an integer or a
 * buffer can be get as a string too. There is no special formatting in this
 * case. "(BS)" means that the string get a special appropriate format.
 *
 * The name of a key is an indication on the type of atom that information can
 * be extracted from. For example, @c lldpctl_k_med_policy_type can be extracted
 * from an atom you got by iterating on @c lldpctl_k_port_med_policies. On the
 * other hand, @c lldpctl_k_port_descr and @c lldpctl_k_chassis can be retrieved
 * from an atom retrieved either by iterating @c lldpctl_k_port_neighbors or
 * with @c lldpctl_get_port().
 *
 * Some values may be written. They are marked with (W). Such a change may or
 * may not be transmitted immediatly. If they are not transmitted immediatly,
 * this means that the resulting atom should be written to another atom. For
 * example, when writting @c lldpctl_k_med_policy_tagged, you need to write the
 * resulting atom to @c lldpctl_k_port_med_policies. If the change is
 * transmitted immediatly, you need to check the error status of the connection
 * to know if it has been transmitted correctly. Notably, if you get @c
 * LLDPCTL_ERR_WOULDBLOCK, you need to try again later. Usually, changes are
 * transmitted immediatly. The exception are changes that need to be grouped to
 * be consistent, like a LLDP MED location. When a change is transmitted
 * immediatly, it is marked with (O). @c lldpctl_atom_set_str() may accept a @c
 * NULL value. This case is marked with (N) and usually reset the item to the
 * default value or no value.
 *
 * Some values may also be created. They are flagged with (C). This only applies
 * to elements that can be iterated (L) and written (W). The element created
 * still needs to be appended to the list by being written to it. The creation
 * is done with @c lldpctl_atom_create().
 *
 * An atom marked with (S) can be retrieved as a string only. It cannot be
 * written. An atom marked with (IS) can be retrieved as an integer and features
 * an appropriate representation as a string (usually, the name of a constant)
 * which is more meaningful than just the integer. An atom marked as (I) can be
 * retrieved as an integer and as a string. In the later case, this is just a
 * string representation of the integer. An atom marked with (AL) can be
 * retrieved as an atom only and can be iterated over. This is usually a list of
 * things. An atom marked (I,W) can be read as an integer or a string and can be
 * written as an integer. The change would not be commited until the atom is
 * written to the nearest atom supporting (A,WO) operation (eventually with an
 * indirection, i.e first write to a (A,W), then to a (A,WO)).
 */
typedef enum {
	lldpctl_k_config_tx_interval, /**< `(I,WO)` Transmit interval. When set to -1, it is meant to transmit now. */
	lldpctl_k_config_receiveonly, /**< `(I)` Receive only mode */
	lldpctl_k_config_mgmt_pattern, /**< `(S,WON)` Pattern to choose the management address */
	lldpctl_k_config_iface_pattern, /**< `(S,WON)` Pattern of enabled interfaces */
	lldpctl_k_config_cid_pattern,	/**< `(S)` Interface pattern to choose the chassis ID */
	lldpctl_k_config_description,	/**< `(S,WON)` Chassis description overridden */
	lldpctl_k_config_platform,	/**< `(S,WON)` Platform description overridden (CDP) */
	lldpctl_k_config_hostname,	/**< `(S,WON)` System name overridden */
	lldpctl_k_config_advertise_version, /**< `(I)` Advertise version */
	lldpctl_k_config_lldpmed_noinventory, /**< `(I)` Disable LLDP-MED inventory */
	lldpctl_k_config_paused,	      /**< `(I,WO)` lldpd is paused */
	lldpctl_k_config_fast_start_enabled, /**< `(I,WO)` Is fast start enabled */
	lldpctl_k_config_fast_start_interval, /**< `(I,WO)` Start fast transmit interval */
	lldpctl_k_config_ifdescr_update, /**< `(I,WO)` Enable or disable setting interface description */
	lldpctl_k_config_iface_promisc,  /**< `(I,WO)` Enable or disable promiscuous mode on interfaces */
	lldpctl_k_config_chassis_cap_advertise, /**< `(I,WO)` Enable or disable chassis capabilities advertisement */
	lldpctl_k_config_chassis_mgmt_advertise, /**< `(I,WO)` Enable or disable management addresses advertisement */
	lldpctl_k_config_cid_string,    /**< `(S,WON)` User defined string for the chassis ID */
	lldpctl_k_config_perm_iface_pattern, /**< `(S,WON)` Pattern of permanent interfaces */
	lldpctl_k_config_tx_interval_ms, /**< `(I,WO)` Transmit interval in milliseconds. Set to -1 to transmit now. */

	lldpctl_k_interface_name = 1000, /**< `(S)` The interface name. */

	lldpctl_k_port_name = 1100,	/**< `(S)` The port name. Only works for a local port. */
	lldpctl_k_port_index,	/**< `(I)` The port index. Only works for a local port. */
	/**
	 * `(AL)` The list of known neighbors for this port.
	 *
	 * A neighbor is in fact a remote port.
	 */
	lldpctl_k_port_neighbors = 1200,
	lldpctl_k_port_protocol,   /**< `(IS)` The protocol that was used to retrieve this information. */
	lldpctl_k_port_age,	   /**< `(I)`  Age of information, seconds from epoch. */
	lldpctl_k_port_id_subtype, /**< `(IS)` The subtype ID of this port.  */
	lldpctl_k_port_id,	   /**< `(BS,WO)` The ID of this port. */
	lldpctl_k_port_descr,	   /**< `(S,WO)` The description of this port. */
	lldpctl_k_port_hidden,	   /**< `(I)` Is this port hidden (or should it be displayed?)? */
	lldpctl_k_port_status,	   /**< `(IS,WO)` Operational status of this (local) port */
	lldpctl_k_port_chassis,	   /**< `(A)` Chassis associated to the port */
	lldpctl_k_port_ttl,        /**< `(I)` TTL for port, 0 if info is attached to chassis */
	lldpctl_k_port_vlan_tx,    /**< `(I,W)` VLAN tag for TX on port, -1 VLAN disabled */

	lldpctl_k_port_dot3_mfs = 1300,	   /**< `(I)` MFS */
	lldpctl_k_port_dot3_aggregid,   /**< `(I)` Port aggregation ID */
	lldpctl_k_port_dot3_autoneg_support, /**< `(I)` Autonegotiation support. */
	lldpctl_k_port_dot3_autoneg_enabled, /**< `(I)` Autonegotiation enabled. */
	lldpctl_k_port_dot3_autoneg_advertised, /**< `(I)` Advertised protocols. See `LLDP_DOT3_LINK_AUTONEG_*` */
	lldpctl_k_port_dot3_mautype, /**< `(IS)` Current MAU type. See `LLDP_DOT3_MAU_*` */

	lldpctl_k_port_dot3_power = 1400, /**< `(A,WO)` Dot3 power related stuff. */
	lldpctl_k_dot3_power_devicetype, /**< `(IS,W)` Device type. See `LLDP_DOT3_POWER_PSE/PD` */
	lldpctl_k_dot3_power_supported, /**< `(I,W)` Is MDI power supported. */
	lldpctl_k_dot3_power_enabled, /**< `(I,W)` Is MDI power enabled. */
	lldpctl_k_dot3_power_paircontrol, /**< `(I,W)` Pair-control enabled? */
	lldpctl_k_dot3_power_pairs, /**< `(IS,W)` See `LLDP_DOT3_POWERPAIRS_*` */
	lldpctl_k_dot3_power_class, /**< `(IS,W)` Power class. */
	lldpctl_k_dot3_power_type, /**< `(I,W)` 802.3AT power type */
	lldpctl_k_dot3_power_source, /**< `(IS,W)` 802.3AT power source */
	lldpctl_k_dot3_power_priority, /**< `(IS,W)` 802.3AT power priority */
	lldpctl_k_dot3_power_allocated, /**< `(I,W)` 802.3AT power allocated */
	lldpctl_k_dot3_power_requested, /**< `(I,W)` 802.3AT power requested */

	/* 802.3bt additions */
	lldpctl_k_dot3_power_pd_4pid, /**< `(IS,W)` 802.3BT both modes supported? */
	lldpctl_k_dot3_power_requested_a, /**< `(I,W)` 802.3BT power value requested for A */
	lldpctl_k_dot3_power_requested_b, /**< `(I,W)` 802.3BT power value requested for B */
	lldpctl_k_dot3_power_allocated_a, /**< `(I,W)` 802.3BT power value allocated for A */
	lldpctl_k_dot3_power_allocated_b, /**< `(I,W)` 802.3BT power value allocated for B */
	lldpctl_k_dot3_power_pse_status, /**< `(IS,W)` 802.3BT PSE powering status */
	lldpctl_k_dot3_power_pd_status, /**< `(IS,W)` 802.3BT PD powering status */
	lldpctl_k_dot3_power_pse_pairs_ext, /**< `(IS,W)` 802.3BT PSE power pairs */
	lldpctl_k_dot3_power_class_a, /**< `(IS,W)` 802.3BT power class for A */
	lldpctl_k_dot3_power_class_b, /**< `(IS,W)` 802.3BT power class for B */
	lldpctl_k_dot3_power_class_ext, /**< `(IS,W)` 802.3BT power class */
	lldpctl_k_dot3_power_type_ext, /**< `(IS,W)` 802.3BT power type */
	lldpctl_k_dot3_power_pd_load, /**< `(IS,W)` 802.3BT dualsig isolated? */
	lldpctl_k_dot3_power_pse_max, /**< `(I,W)` 802.3BT maximum available power */

	lldpctl_k_port_vlan_pvid = 1500, /**< `(I)` Primary VLAN ID */
	lldpctl_k_port_vlans, /**< `(AL)` List of VLAN */
	lldpctl_k_vlan_id, /**< `(I)` VLAN ID */
	lldpctl_k_vlan_name, /**< `(S)` VLAN name */

	lldpctl_k_port_ppvids = 1600, /**< `(AL)` List of PPVIDs */
	lldpctl_k_ppvid_status, /**< `(I)` Status of PPVID (see `LLDP_PPVID_CAP_*`) */
	lldpctl_k_ppvid_id, /**< `(I)` ID of PPVID */

	lldpctl_k_port_pis = 1700, /**< `(AL)` List of PIDs */
	lldpctl_k_pi_id,    /**< `(B)` PID value */

	lldpctl_k_chassis_index = 1800,   /**< `(I)` The chassis index. */
	lldpctl_k_chassis_id_subtype, /**< `(IS)` The subtype ID of this chassis. */
	lldpctl_k_chassis_id,	      /**< `(BS)` The ID of this chassis. */
	lldpctl_k_chassis_name,	      /**< `(S)` The name of this chassis. */
	lldpctl_k_chassis_descr,      /**< `(S)` The description of this chassis. */
	lldpctl_k_chassis_cap_available, /**< `(I)` Available capabalities (see `LLDP_CAP_*`) */
	lldpctl_k_chassis_cap_enabled,	 /**< `(I)` Enabled capabilities (see `LLDP_CAP_*`) */
	lldpctl_k_chassis_mgmt,		 /**< `(AL)` List of management addresses */
	lldpctl_k_chassis_ttl,		 /**< Deprecated */

	lldpctl_k_chassis_med_type = 1900, /**< `(IS)` Chassis MED type. See `LLDP_MED_CLASS_*` */
	lldpctl_k_chassis_med_cap,  /**< `(I)` Available MED capabilitied. See `LLDP_MED_CAP_*` */
	lldpctl_k_chassis_med_inventory_hw, /**< `(S)` LLDP MED inventory "Hardware Revision" */
	lldpctl_k_chassis_med_inventory_sw, /**< `(S)` LLDP MED inventory "Software Revision" */
	lldpctl_k_chassis_med_inventory_fw, /**< `(S)` LLDP MED inventory "Firmware Revision" */
	lldpctl_k_chassis_med_inventory_sn, /**< `(S)` LLDP MED inventory "Serial Number" */
	lldpctl_k_chassis_med_inventory_manuf, /**< `(S)` LLDP MED inventory "Manufacturer" */
	lldpctl_k_chassis_med_inventory_model, /**< `(S)` LLDP MED inventory "Model" */
	lldpctl_k_chassis_med_inventory_asset, /**< `(S)` LLDP MED inventory "Asset ID" */

	lldpctl_k_port_med_policies = 2000, /**< `(AL,WO)` MED policies attached to a port. */
	lldpctl_k_med_policy_type, /**< `(IS,W)` MED policy app type. See `LLDP_MED_APPTYPE_*`. 0 if a policy is not defined. */
	lldpctl_k_med_policy_unknown, /**< `(I,W)` Is MED policy defined? */
	lldpctl_k_med_policy_tagged, /**< `(I,W)` MED policy tagging */
	lldpctl_k_med_policy_vid,    /**< `(I,W)` MED policy VID */
	lldpctl_k_med_policy_priority, /**< `(I,W)` MED policy priority */
	lldpctl_k_med_policy_dscp,     /**< `(I,W)` MED policy DSCP */

	lldpctl_k_port_med_locations = 2100, /**< `(AL,WO)` MED locations attached to a port. */
	lldpctl_k_med_location_format, /**< `(IS,W)` MED location format. See
					* `LLDP_MED_LOCFORMAT_*`. 0 if this
					* location is not defined. When written,
					* the following fields will be zeroed
					* out. */
	lldpctl_k_med_location_geoid, /**< `(IS,W)` MED geoid. See `LLDP_MED_LOCATION_GEOID_*`. Only if format is COORD. */
	lldpctl_k_med_location_latitude,  /**< `(S,W)` MED latitude. Only if format is COORD. */
	lldpctl_k_med_location_longitude, /**< `(S,W)` MED longitude. Only if format is COORD. */
	lldpctl_k_med_location_altitude,  /**< `(S,W)` MED altitude. Only if format is COORD. */
	lldpctl_k_med_location_altitude_unit, /**< `(S,W)` MED altitude unit. See `LLDP_MED_LOCATION_ALTITUDE_UNIT_*`.
					       * Only if format is COORD. */

	lldpctl_k_med_location_country = 2200, /**< `(S,W)` MED country. Only if format is CIVIC. */
	lldpctl_k_med_location_elin, /**< `(S,W)` MED ELIN. Only if format is ELIN. */

	lldpctl_k_med_location_ca_elements = 2300, /**< `(AL,WC)` MED civic address elements. Only if format is CIVIC */
	lldpctl_k_med_civicaddress_type, /**< `(IS,W)` MED civic address type. */
	lldpctl_k_med_civicaddress_value, /**< `(S,W)` MED civic address value. */

	lldpctl_k_port_med_power = 2400, /**< `(A,WO)` LLDP-MED power related stuff. */
	lldpctl_k_med_power_type, /**< `(IS,W)` LLDP MED power device type. See `LLDP_MED_POW_TYPE_*` */
	lldpctl_k_med_power_source, /**< `(IS,W)` LLDP MED power source. See `LLDP_MED_POW_SOURCE_*` */
	lldpctl_k_med_power_priority, /**< `(IS,W)` LLDP MED power priority. See `LLDP_MED_POW_PRIO_*` */
	lldpctl_k_med_power_val, /**< `(I,W)` LLDP MED power value */

	lldpctl_k_mgmt_ip = 3000,	/**< `(S)` IP address */
	lldpctl_k_mgmt_iface_index = 30001,	/**< `(I)` Interface index */

	lldpctl_k_tx_cnt = 4000,	/**< `(I)` tx cnt. Only works for a local port. */
	lldpctl_k_rx_cnt,	/**< `(I)` rx cnt. Only works for a local port. */
	lldpctl_k_rx_discarded_cnt,	/**< `(I)` discarded cnt. Only works for a local port. */
	lldpctl_k_rx_unrecognized_cnt,	/**< `(I)` unrecognized cnt. Only works for a local port. */
	lldpctl_k_ageout_cnt,	/**< `(I)` ageout cnt. Only works for a local port. */
	lldpctl_k_insert_cnt,	/**< `(I)` insert cnt. Only works for a local port. */
	lldpctl_k_delete_cnt,	/**< `(I)` delete cnt. Only works for a local port. */
	lldpctl_k_config_tx_hold, /**< `(I,WO)` Transmit hold interval. */
	lldpctl_k_config_bond_slave_src_mac_type, /**< `(I,WO)` bond slave src mac type. */
	lldpctl_k_config_lldp_portid_type, /**< `(I,WO)` LLDP PortID TLV Subtype */
	lldpctl_k_config_lldp_agent_type, /**< `(I,WO)` LLDP agent type */
	lldpctl_k_config_max_neighbors, /**< `(I,WO)`Maximum number of neighbors per port. */

	lldpctl_k_custom_tlvs = 5000,		/**< `(AL)` custom TLVs */
	lldpctl_k_custom_tlvs_clear,		/** `(I,WO)` clear list of custom TLVs */
	lldpctl_k_custom_tlv,			/** `(AL,WO)` custom TLV **/
	lldpctl_k_custom_tlv_oui,		/**< `(I,WO)` custom TLV Organizationally Unique Identifier. Default is 0 (3 bytes) */
	lldpctl_k_custom_tlv_oui_subtype,	/**< `(I,WO)` custom TLV subtype. Default is 0 (1 byte) */
	lldpctl_k_custom_tlv_oui_info_string,	/**< `(I,WO)` custom TLV Organizationally Unique Identifier Information String (up to 507 bytes) */
	lldpctl_k_custom_tlv_op,		/**< `(I,WO)` custom TLV operation */

} lldpctl_key_t;

/**
 * Get a map related to a key.
 *
 * Many keys expect to be written with a discrete number of values. Take for
 * example @c lldpctl_k_med_civicaddress_type, it can take any integer between 1
 * and 128. However, each integer can be named. It can be useful for an
 * application to get a translation between the integer that can be provided and
 * a more human-readable name. This function allows to retrieve the
 * corresponding map.
 *
 * @param key    The piece of information we want a map from.
 * @return       The map or @c NULL if no map is available.
 *
 * The returned map has its last element set to 0. It is also expected that the
 * string value can be used with a set operation. It will be translated to the
 * integer value.
 */
lldpctl_map_t *lldpctl_key_get_map(lldpctl_key_t key);

/**
 * Retrieve a bit of information as an atom.
 *
 * @param atom The atom we want to query.
 * @param key  The information we want from the atom.
 * @return The atom representing the requested information or @c NULL if the
 *         information is not available.
 *
 * Not every value of @c info will be available as an atom. See the
 * documentation of @c lldpctl_key_t for values accepting to be extracted as an
 * atom. Usually, this is only iterable values or values representing a complex
 * object.
 *
 * The provided atom is not a _borrowed_ reference. You need to decrement the
 * reference count when you don't need it anymore.
 *
 * As a convenience, this function will return @c NULL if the first parameter is
 * @c NULL and no error will be raised.
 */
lldpctl_atom_t *lldpctl_atom_get(lldpctl_atom_t *atom, lldpctl_key_t key);

/**
 * Set a bit of information with an atom.
 *
 * @param atom  The atom we want to write to.
 * @param key   The key information we want to write.
 * @param value The value of the information we want to write.
 * @return The updated atom with the appropriate information.
 *
 * This function will return @c NULL in case of error. If the last error is @c
 * LLDPCTL_ERR_WOULDBLOCK, the write should be retried later with the exact same
 * parameters. LLDPCTL_ERR_BAD_VALUE is raised when the provided atom is not
 * correct.
 */
lldpctl_atom_t *lldpctl_atom_set(lldpctl_atom_t *atom, lldpctl_key_t key,
    lldpctl_atom_t *value);

/**
 * Retrieve a bit of information as a null-terminated string.
 *
 * @param atom The atom we want to query.
 * @param key  The information we want from the atom.
 * @return The requested string or @c NULL if the information is not available.
 *
 * Not every value of @c info will be available as a string. See the
 * documentation of @c lldpctl_key_t for values accepting to be extracted as a
 * string. Usually, only piece of information stored as string are available in
 * this form but sometimes, you can get a nice formatted string instead of an
 * integer with this function.
 *
 * As a convenience, this function will return @c NULL if the first parameter is
 * @c NULL and no error will be raised.
 *
 * The provided string may live inside the atom providing it. If you need it
 * longer, duplicate it.
 */
const char *lldpctl_atom_get_str(lldpctl_atom_t *atom, lldpctl_key_t key);

/**
 * Set a bit of information using a null-terminated string.
 *
 * @param atom  The atom we want to write to.
 * @param key   The key information we want to write.
 * @param value The value of the information we want to write.
 * @return The updated atom with the appropriate information.
 *
 * This function will return @c NULL in case of error. If the last error is @c
 * LLDPCTL_ERR_WOULDBLOCK, the write should be retried later with the exact same
 * parameters. LLDPCTL_ERR_BAD_VALUE is raised when the provided atom is not
 * correct.
 */
lldpctl_atom_t *lldpctl_atom_set_str(lldpctl_atom_t *atom, lldpctl_key_t key,
    const char *value);

/**
 * Retrieve a bit of information as a buffer.
 *
 * @param atom        The atom we want to query.
 * @param key         The information we want from the atom.
 * @param[out] length The size of the returned buffer.
 * @return The requested buffer or @c NULL if the information is not available.
 *
 * Not every value of @c info will be available as a buffer. See the
 * documentation of @c lldpctl_key_t for values accepting to be extracted as a
 * string. Usually, only piece of information stored as buffer are available in
 * this form.
 *
 * As a convenience, this function will return @c NULL if the first parameter is
 * @c NULL and no error will be raised. If this function returns @c NULL, the
 * third parameter is set to 0.
 *
 * The provided buffer may live inside the atom providing it. If you need it
 * longer, duplicate it.
 */
const uint8_t *lldpctl_atom_get_buffer(lldpctl_atom_t *atom, lldpctl_key_t key,
    size_t *length);

/**
 * Set a bit of information using a buffer
 *
 * @param atom   The atom we want to write to.
 * @param key    The key information we want to write.
 * @param value  The value of the information we want to write.
 * @param length The length of the provided buffer.
 * @return The updated atom with the appropriate information.
 *
 * This function will return @c NULL in case of error. If the last error is @c
 * LLDPCTL_ERR_WOULDBLOCK, the write should be retried later with the exact same
 * parameters. LLDPCTL_ERR_BAD_VALUE is raised when the provided atom is not
 * correct.
 */
lldpctl_atom_t *lldpctl_atom_set_buffer(lldpctl_atom_t *atom, lldpctl_key_t key,
    const uint8_t *value, size_t length);

/**
 * Retrieve a bit of information as an integer.
 *
 * @param atom The atom we want to query.
 * @param key  The information we want from the atom.
 * @return The requested integer or -1 if the information is not available
 *
 * Not every value of @c info will be available as an integer. See the
 * documentation of @c lldpctl_key_t for values accepting to be extracted as a
 * string. Usually, only piece of information stored as an integer are available
 * in this form.
 *
 * Only @c lldpctl_last_error() can tell if the returned value is an error or
 * not. However, most values extracted from lldpd cannot be negative.
 */
long int lldpctl_atom_get_int(lldpctl_atom_t *atom, lldpctl_key_t key);

/**
 * Set a bit of information using an integer
 *
 * @param atom   The atom we want to write to.
 * @param key    The key information we want to write.
 * @param value  The value of the information we want to write.
 * @return The updated atom with the appropriate information.
 *
 * This function will return @c NULL in case of error. If the last error is @c
 * LLDPCTL_ERR_WOULDBLOCK, the write should be retried later with the exact same
 * parameters. LLDPCTL_ERR_BAD_VALUE is raised when the provided atom is not
 * correct.
 */
lldpctl_atom_t *lldpctl_atom_set_int(lldpctl_atom_t *atom, lldpctl_key_t key,
    long int value);

/**
 * @defgroup liblldpctl_atom_iter Iterating over atoms
 *
 * Iterate over atoms (lists).
 *
 * @{
 */
/**
 * Iterator over an iterable atom (a list of ports, a list of VLAN, ...). When
 * an atom is a list, it can be iterated over to extract the appropriate values.
 *
 * @see lldpctl_atom_iter(), lldpctl_atom_iter_next(), lldpctl_atom_iter_value()
 */
typedef struct lldpctl_atom_iter_t lldpctl_atom_iter_t;

/**
 * Return an iterator over a given atom.
 *
 * If an atom is iterable (if it is a list, like a list of ports, a list of
 * VLAN, a list of neighbors), it is possible to iterate over it. First use this
 * function to get an iterator then use @c lldpctl_atom_iter_next() to get the
 * next item and @c lldpctl_atom_iter_value() to the actuel item.
 *
 * @param atom The atom we want to create an iterator from.
 * @return The iterator or @c NULL if an error happened or if the atom is empty
 *         (check with @c lldpctl_last_error()).
 *
 * As a convenience, if the provided atom is @c NULL, this function will return
 * @c NULL and no error will be raised.
 */
lldpctl_atom_iter_t *lldpctl_atom_iter(lldpctl_atom_t *atom);

/**
 * Return the next element of an iterator.
 *
 * @param atom The atom we are currently iterating.
 * @param iter The iterator we want the next element from.
 * @return An iterator starting on the next element or @c NULL if we have no
 *         more elements
 *
 * @see lldpctl_atom_iter(), lldpctl_atom_iter_value().
 *
 * As a convenience, if the provided atom is @c NULL, this function will return
 * @c NULL and no error will be raised.
 */
lldpctl_atom_iter_t *lldpctl_atom_iter_next(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter);

/**
 * Return the value of an iterator.
 *
 * @param atom The atom we are currently iterating.
 * @param iter The iterator we want the next element from.
 * @return The atom currently associated with the iterator.
 *
 * @see lldpctl_atom_iter(), lldpctl_atom_iter_next().
 */
lldpctl_atom_t *lldpctl_atom_iter_value(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter);

/**
 * Convenience macro to iter over every value of an iterable object.
 *
 * @param atom  The atom you want to iterate on.
 * @param value Atom name that will be used to contain each value.
 *
 * This macro behaves as a for loop. Moreover, at the end of each iteration, the
 * reference count of the provided value is decremented. If you need to use it
 * outside of the loop, you need to increment it.
 */
#define lldpctl_atom_foreach(atom, value)						\
	for (lldpctl_atom_iter_t *iter##_LINE_ = lldpctl_atom_iter(atom);		\
	     iter##_LINE_ && (value = lldpctl_atom_iter_value(atom, iter##_LINE_));	\
	     iter##_LINE_ = lldpctl_atom_iter_next(atom, iter##_LINE_),			\
		 lldpctl_atom_dec_ref(value))

/**
 * Create a new value for an iterable element.
 *
 * The value is meant to be appended using @c lldpctl_atom_set(). Currently,
 * there is no way to delete an element from a list. It is also not advisable to
 * use getters on a newly created object until it is fully initialized. If its
 * internal representation is using a buffer, it may not be initialized until
 * the first set.
 *
 * @param atom The atom we want to create a new element for.
 * @return The new element.
 */
lldpctl_atom_t *lldpctl_atom_create(lldpctl_atom_t *atom);
/**@}*/
/**@}*/

#ifdef __cplusplus
}
#endif

/**@}*/

#endif
