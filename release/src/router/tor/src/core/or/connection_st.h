/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef CONNECTION_ST_H
#define CONNECTION_ST_H

struct buf_t;

/* Values for connection_t.magic: used to make sure that downcasts (casts from
* connection_t to foo_connection_t) are safe. */
#define BASE_CONNECTION_MAGIC 0x7C3C304Eu
#define OR_CONNECTION_MAGIC 0x7D31FF03u
#define EDGE_CONNECTION_MAGIC 0xF0374013u
#define ENTRY_CONNECTION_MAGIC 0xbb4a5703
#define DIR_CONNECTION_MAGIC 0x9988ffeeu
#define CONTROL_CONNECTION_MAGIC 0x8abc765du
#define LISTENER_CONNECTION_MAGIC 0x1a1ac741u

/** Description of a connection to another host or process, and associated
 * data.
 *
 * A connection is named based on what it's connected to -- an "OR
 * connection" has a Tor node on the other end, an "exit
 * connection" has a website or other server on the other end, and an
 * "AP connection" has an application proxy (and thus a user) on the
 * other end.
 *
 * Every connection has a type and a state.  Connections never change
 * their type, but can go through many state changes in their lifetime.
 *
 * Every connection has two associated input and output buffers.
 * Listeners don't use them.  For non-listener connections, incoming
 * data is appended to conn->inbuf, and outgoing data is taken from
 * conn->outbuf.  Connections differ primarily in the functions called
 * to fill and drain these buffers.
 */
struct connection_t {
  uint32_t magic; /**< For memory debugging: must equal one of
                   * *_CONNECTION_MAGIC. */

  uint8_t state; /**< Current state of this connection. */
  unsigned int type:5; /**< What kind of connection is this? */
  unsigned int purpose:5; /**< Only used for DIR and EXIT types currently. */

  /* The next fields are all one-bit booleans. Some are only applicable to
   * connection subtypes, but we hold them here anyway, to save space.
   */
  unsigned int read_blocked_on_bw:1; /**< Boolean: should we start reading
                            * again once the bandwidth throttler allows it? */
  unsigned int write_blocked_on_bw:1; /**< Boolean: should we start writing
                             * again once the bandwidth throttler allows
                             * writes? */
  unsigned int hold_open_until_flushed:1; /**< Despite this connection's being
                                      * marked for close, do we flush it
                                      * before closing it? */
  unsigned int inbuf_reached_eof:1; /**< Boolean: did read() return 0 on this
                                     * conn? */
  /** Set to 1 when we're inside connection_flushed_some to keep us from
   * calling connection_handle_write() recursively. */
  unsigned int in_flushed_some:1;
  /** True if connection_handle_write is currently running on this connection.
   */
  unsigned int in_connection_handle_write:1;

  /* For linked connections:
   */
  unsigned int linked:1; /**< True if there is, or has been, a linked_conn. */
  /** True iff we'd like to be notified about read events from the
   * linked conn. */
  unsigned int reading_from_linked_conn:1;
  /** True iff we're willing to write to the linked conn. */
  unsigned int writing_to_linked_conn:1;
  /** True iff we're currently able to read on the linked conn, and our
   * read_event should be made active with libevent. */
  unsigned int active_on_link:1;
  /** True iff we've called connection_close_immediate() on this linked
   * connection. */
  unsigned int linked_conn_is_closed:1;

  /** CONNECT/SOCKS proxy client handshake state (for outgoing connections). */
  unsigned int proxy_state:4;

  /** Our socket; set to TOR_INVALID_SOCKET if this connection is closed,
   * or has no socket. */
  tor_socket_t s;
  int conn_array_index; /**< Index into the global connection array. */

  struct event *read_event; /**< Libevent event structure. */
  struct event *write_event; /**< Libevent event structure. */
  struct buf_t *inbuf; /**< Buffer holding data read over this connection. */
  struct buf_t *outbuf; /**< Buffer holding data to write over this
                         * connection. */
  size_t outbuf_flushlen; /**< How much data should we try to flush from the
                           * outbuf? */
  time_t timestamp_last_read_allowed; /**< When was the last time libevent said
                                       * we could read? */
  time_t timestamp_last_write_allowed; /**< When was the last time libevent
                                        * said we could write? */

  time_t timestamp_created; /**< When was this connection_t created? */

  int socket_family; /**< Address family of this connection's socket.  Usually
                      * AF_INET, but it can also be AF_UNIX, or AF_INET6 */
  tor_addr_t addr; /**< IP that socket "s" is directly connected to;
                    * may be the IP address for a proxy or pluggable transport,
                    * see "address" for the address of the final destination.
                    */
  uint16_t port; /**< If non-zero, port that socket "s" is directly connected
                  * to; may be the port for a proxy or pluggable transport,
                  * see "address" for the port at the final destination. */
  uint16_t marked_for_close; /**< Should we close this conn on the next
                              * iteration of the main loop? (If true, holds
                              * the line number where this connection was
                              * marked.) */
  const char *marked_for_close_file; /**< For debugging: in which file were
                                      * we marked for close? */
  char *address; /**< FQDN (or IP) and port of the final destination for this
                  * connection; this is always the remote address, it is
                  * passed to a proxy or pluggable transport if one in use.
                  * See "addr" and "port" for the address that socket "s" is
                  * directly connected to.
                  * strdup into this, because free_connection() frees it. */
  /** Another connection that's connected to this one in lieu of a socket. */
  struct connection_t *linked_conn;

  /** Unique identifier for this connection on this Tor instance. */
  uint64_t global_identifier;

  /** Bytes read since last call to control_event_conn_bandwidth_used().
   * Only used if we're configured to emit CONN_BW events. */
  uint32_t n_read_conn_bw;

  /** Bytes written since last call to control_event_conn_bandwidth_used().
   * Only used if we're configured to emit CONN_BW events. */
  uint32_t n_written_conn_bw;
};

/** True iff <b>x</b> is an edge connection. */
#define CONN_IS_EDGE(x) \
  ((x)->type == CONN_TYPE_EXIT || (x)->type == CONN_TYPE_AP)

/** True iff the purpose of <b>conn</b> means that it's a server-side
 * directory connection. */
#define DIR_CONN_IS_SERVER(conn) ((conn)->purpose == DIR_PURPOSE_SERVER)

#endif /* !defined(CONNECTION_ST_H) */
