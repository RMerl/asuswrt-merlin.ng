/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file control.c
 * \brief Implementation for Tor's control-socket interface.
 *
 * A "controller" is an external program that monitors and controls a Tor
 * instance via a text-based protocol. It connects to Tor via a connection
 * to a local socket.
 *
 * The protocol is line-driven.  The controller sends commands terminated by a
 * CRLF.  Tor sends lines that are either <em>replies</em> to what the
 * controller has said, or <em>events</em> that Tor sends to the controller
 * asynchronously based on occurrences in the Tor network model.
 *
 * See the control-spec.txt file in the torspec.git repository for full
 * details on protocol.
 *
 * This module generally has two kinds of entry points: those based on having
 * received a command on a controller socket, which are handled in
 * connection_control_process_inbuf(), and dispatched to individual functions
 * with names like control_handle_COMMANDNAME(); and those based on events
 * that occur elsewhere in Tor, which are handled by functions with names like
 * control_event_EVENTTYPE().
 *
 * Controller events are not sent immediately; rather, they are inserted into
 * the queued_control_events array, and flushed later from
 * flush_queued_events_cb().  Doing this simplifies our callgraph greatly,
 * by limiting the number of places in Tor that can call back into the network
 * stack.
 **/

#define CONTROL_MODULE_PRIVATE
#define CONTROL_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "app/main/main.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/or/connection_or.h"
#include "core/proto/proto_control0.h"
#include "core/proto/proto_http.h"
#include "feature/control/control.h"
#include "feature/control/control_auth.h"
#include "feature/control/control_cmd.h"
#include "feature/control/control_events.h"
#include "feature/control/control_proto.h"
#include "feature/hs/hs_common.h"
#include "feature/hs/hs_service.h"
#include "lib/evloop/procmon.h"

#include "feature/control/control_connection_st.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

/**
 * Cast a `connection_t *` to a `control_connection_t *`.
 *
 * Exit with an assertion failure if the input is not a
 * `control_connection_t`.
 **/
control_connection_t *
TO_CONTROL_CONN(connection_t *c)
{
  tor_assert(c->magic == CONTROL_CONNECTION_MAGIC);
  return DOWNCAST(control_connection_t, c);
}

/**
 * Cast a `const connection_t *` to a `const control_connection_t *`.
 *
 * Exit with an assertion failure if the input is not a
 * `control_connection_t`.
 **/
const control_connection_t *
CONST_TO_CONTROL_CONN(const connection_t *c)
{
  return TO_CONTROL_CONN((connection_t*)c);
}

/** Create and add a new controller connection on <b>sock</b>.  If
 * <b>CC_LOCAL_FD_IS_OWNER</b> is set in <b>flags</b>, this Tor process should
 * exit when the connection closes.  If <b>CC_LOCAL_FD_IS_AUTHENTICATED</b>
 * is set, then the connection does not need to authenticate.
 */
int
control_connection_add_local_fd(tor_socket_t sock, unsigned flags)
{
  if (BUG(! SOCKET_OK(sock)))
    return -1;
  const int is_owner = !!(flags & CC_LOCAL_FD_IS_OWNER);
  const int is_authenticated = !!(flags & CC_LOCAL_FD_IS_AUTHENTICATED);
  control_connection_t *control_conn = control_connection_new(AF_UNSPEC);
  connection_t *conn = TO_CONN(control_conn);
  conn->s = sock;
  tor_addr_make_unspec(&conn->addr);
  conn->port = 1;
  conn->address = tor_strdup("<local socket>");

  /* We take ownership of this socket so that later, when we close it,
   * we don't freak out. */
  tor_take_socket_ownership(sock);

  if (set_socket_nonblocking(sock) < 0 ||
      connection_add(conn) < 0) {
    connection_free(conn);
    return -1;
  }

  control_conn->is_owning_control_connection = is_owner;

  if (connection_init_accepted_conn(conn, NULL) < 0) {
    connection_mark_for_close(conn);
    return -1;
  }

  if (is_authenticated) {
    conn->state = CONTROL_CONN_STATE_OPEN;
  }

  return 0;
}

/** Write all of the open control ports to ControlPortWriteToFile */
void
control_ports_write_to_file(void)
{
  smartlist_t *lines;
  char *joined = NULL;
  const or_options_t *options = get_options();

  if (!options->ControlPortWriteToFile)
    return;

  lines = smartlist_new();

  SMARTLIST_FOREACH_BEGIN(get_connection_array(), const connection_t *, conn) {
    if (conn->type != CONN_TYPE_CONTROL_LISTENER || conn->marked_for_close)
      continue;
#ifdef AF_UNIX
    if (conn->socket_family == AF_UNIX) {
      smartlist_add_asprintf(lines, "UNIX_PORT=%s\n", conn->address);
      continue;
    }
#endif /* defined(AF_UNIX) */
    smartlist_add_asprintf(lines, "PORT=%s:%d\n", conn->address, conn->port);
  } SMARTLIST_FOREACH_END(conn);

  joined = smartlist_join_strings(lines, "", 0, NULL);

  if (write_str_to_file(options->ControlPortWriteToFile, joined, 0) < 0) {
    log_warn(LD_CONTROL, "Writing %s failed: %s",
             options->ControlPortWriteToFile, strerror(errno));
  }
#ifndef _WIN32
  if (options->ControlPortFileGroupReadable) {
    if (chmod(options->ControlPortWriteToFile, 0640)) {
      log_warn(LD_FS,"Unable to make %s group-readable.",
               options->ControlPortWriteToFile);
    }
  }
#endif /* !defined(_WIN32) */
  tor_free(joined);
  SMARTLIST_FOREACH(lines, char *, cp, tor_free(cp));
  smartlist_free(lines);
}

const struct signal_name_t signal_table[] = {
  /* NOTE: this table is used for handling SIGNAL commands and generating
   * SIGNAL events.  Order is significant: if there are two entries for the
   * same numeric signal, the first one is the canonical name generated
   * for the events. */
  { SIGHUP, "RELOAD" },
  { SIGHUP, "HUP" },
  { SIGINT, "SHUTDOWN" },
  { SIGUSR1, "DUMP" },
  { SIGUSR1, "USR1" },
  { SIGUSR2, "DEBUG" },
  { SIGUSR2, "USR2" },
  { SIGTERM, "HALT" },
  { SIGTERM, "TERM" },
  { SIGTERM, "INT" },
  { SIGNEWNYM, "NEWNYM" },
  { SIGCLEARDNSCACHE, "CLEARDNSCACHE"},
  { SIGHEARTBEAT, "HEARTBEAT"},
  { SIGACTIVE, "ACTIVE" },
  { SIGDORMANT, "DORMANT" },
  { 0, NULL },
};

/** Called when <b>conn</b> has no more bytes left on its outbuf. */
int
connection_control_finished_flushing(control_connection_t *conn)
{
  tor_assert(conn);
  return 0;
}

/** Called when <b>conn</b> has gotten its socket closed. */
int
connection_control_reached_eof(control_connection_t *conn)
{
  tor_assert(conn);

  log_info(LD_CONTROL,"Control connection reached EOF. Closing.");
  connection_mark_for_close(TO_CONN(conn));
  return 0;
}

/** Shut down this Tor instance in the same way that SIGINT would, but
 * with a log message appropriate for the loss of an owning controller. */
static void
lost_owning_controller(const char *owner_type, const char *loss_manner)
{
  log_notice(LD_CONTROL, "Owning controller %s has %s -- exiting now.",
             owner_type, loss_manner);

  activate_signal(SIGTERM);
}

/** Called when <b>conn</b> is being freed. */
void
connection_control_closed(control_connection_t *conn)
{
  tor_assert(conn);

  conn->event_mask = 0;
  control_update_global_event_mask();

  /* Close all ephemeral Onion Services if any.
   * The list and it's contents are scrubbed/freed in connection_free_.
   */
  if (conn->ephemeral_onion_services) {
    SMARTLIST_FOREACH_BEGIN(conn->ephemeral_onion_services, char *, cp) {
      if (hs_address_is_valid(cp)) {
        hs_service_del_ephemeral(cp);
      } else {
        /* An invalid .onion in our list should NEVER happen */
        tor_fragile_assert();
      }
    } SMARTLIST_FOREACH_END(cp);
  }

  if (conn->is_owning_control_connection) {
    lost_owning_controller("connection", "closed");
  }
}

/** Return true iff <b>cmd</b> is allowable (or at least forgivable) at this
 * stage of the protocol. */
static int
is_valid_initial_command(control_connection_t *conn, const char *cmd)
{
  if (conn->base_.state == CONTROL_CONN_STATE_OPEN)
    return 1;
  if (!strcasecmp(cmd, "PROTOCOLINFO"))
    return (!conn->have_sent_protocolinfo &&
            conn->safecookie_client_hash == NULL);
  if (!strcasecmp(cmd, "AUTHCHALLENGE"))
    return (conn->safecookie_client_hash == NULL);
  if (!strcasecmp(cmd, "AUTHENTICATE") ||
      !strcasecmp(cmd, "QUIT"))
    return 1;
  return 0;
}

/** Do not accept any control command of more than 1MB in length.  Anything
 * that needs to be anywhere near this long probably means that one of our
 * interfaces is broken. */
#define MAX_COMMAND_LINE_LENGTH (1024*1024)

/** Wrapper around peek_buf_has_control0 command: presents the same
 * interface as that underlying functions, but takes a connection_t instead of
 * a buf_t.
 */
static int
peek_connection_has_control0_command(connection_t *conn)
{
  return peek_buf_has_control0_command(conn->inbuf);
}

static int
peek_connection_has_http_command(connection_t *conn)
{
  return peek_buf_has_http_command(conn->inbuf);
}

/**
 * Helper: take a nul-terminated command of given length, and find where the
 * command starts and the arguments begin.  Separate them, allocate a new
 * string in <b>current_cmd_out</b> for the command, and return a pointer
 * to the arguments.
 **/
STATIC char *
control_split_incoming_command(char *incoming_cmd,
                               size_t *data_len,
                               char **current_cmd_out)
{
  const bool is_multiline = *data_len && incoming_cmd[0] == '+';
  size_t cmd_len = 0;
  while (cmd_len < *data_len
         && !TOR_ISSPACE(incoming_cmd[cmd_len]))
    ++cmd_len;

  *current_cmd_out = tor_memdup_nulterm(incoming_cmd, cmd_len);
  char *args = incoming_cmd+cmd_len;
  tor_assert(*data_len>=cmd_len);
  *data_len -= cmd_len;
  if (is_multiline) {
    // Only match horizontal space: any line after the first is data,
    // not arguments.
    while ((*args == '\t' || *args == ' ') && *data_len) {
      ++args;
      --*data_len;
    }
  } else {
    while (TOR_ISSPACE(*args) && *data_len) {
      ++args;
      --*data_len;
    }
  }

  return args;
}

static const char CONTROLPORT_IS_NOT_AN_HTTP_PROXY_MSG[] =
  "HTTP/1.0 501 Tor ControlPort is not an HTTP proxy"
  "\r\nContent-Type: text/html; charset=iso-8859-1\r\n\r\n"
  "<html>\n"
  "<head>\n"
  "<title>Tor's ControlPort is not an HTTP proxy</title>\n"
  "</head>\n"
  "<body>\n"
  "<h1>Tor's ControlPort is not an HTTP proxy</h1>\n"
  "<p>\n"
  "It appears you have configured your web browser to use Tor's control port"
  " as an HTTP proxy.\n"
  "This is not correct: Tor's default SOCKS proxy port is 9050.\n"
  "Please configure your client accordingly.\n"
  "</p>\n"
  "<p>\n"
  "See <a href=\"https://www.torproject.org/documentation.html\">"
  "https://www.torproject.org/documentation.html</a> for more "
  "information.\n"
  "<!-- Plus this comment, to make the body response more than 512 bytes, so "
  "     IE will be willing to display it. Comment comment comment comment "
  "     comment comment comment comment comment comment comment comment.-->\n"
  "</p>\n"
  "</body>\n"
  "</html>\n";

/** Return an error on a control connection that tried to use the v0 protocol.
 */
static void
control_send_v0_reject(control_connection_t *conn)
{
  size_t body_len;
  char buf[128];
  set_uint16(buf+2, htons(0x0000)); /* type == error */
  set_uint16(buf+4, htons(0x0001)); /* code == internal error */
  strlcpy(buf+6, "The v0 control protocol is not supported by Tor 0.1.2.17 "
          "and later; upgrade your controller.",
          sizeof(buf)-6);
  body_len = 2+strlen(buf+6)+2; /* code, msg, nul. */
  set_uint16(buf+0, htons(body_len));
  connection_buf_add(buf, 4+body_len, TO_CONN(conn));

  connection_mark_and_flush(TO_CONN(conn));
}

/** Return an error on a control connection that tried to use HTTP.
 */
static void
control_send_http_reject(control_connection_t *conn)
{
  connection_write_str_to_buf(CONTROLPORT_IS_NOT_AN_HTTP_PROXY_MSG, conn);
  log_notice(LD_CONTROL, "Received HTTP request on ControlPort");
  connection_mark_and_flush(TO_CONN(conn));
}

/** Check if a control connection has tried to use a known invalid protocol.
 * If it has, then:
 *  - send a reject response,
 *  - log a notice-level message, and
 *  - return false. */
static bool
control_protocol_is_valid(control_connection_t *conn)
{
  /* Detect v0 commands and send a "no more v0" message. */
  if (conn->base_.state == CONTROL_CONN_STATE_NEEDAUTH &&
      peek_connection_has_control0_command(TO_CONN(conn))) {
    control_send_v0_reject(conn);
    return 0;
  }

  /* If the user has the HTTP proxy port and the control port confused. */
  if (conn->base_.state == CONTROL_CONN_STATE_NEEDAUTH &&
      peek_connection_has_http_command(TO_CONN(conn))) {
    control_send_http_reject(conn);
    return 0;
  }

  return 1;
}

/** Called when data has arrived on a v1 control connection: Try to fetch
 * commands from conn->inbuf, and execute them.
 */
int
connection_control_process_inbuf(control_connection_t *conn)
{
  size_t data_len;
  uint32_t cmd_data_len;
  char *args;

  tor_assert(conn);
  tor_assert(conn->base_.state == CONTROL_CONN_STATE_OPEN ||
             conn->base_.state == CONTROL_CONN_STATE_NEEDAUTH);

  if (!conn->incoming_cmd) {
    conn->incoming_cmd = tor_malloc(1024);
    conn->incoming_cmd_len = 1024;
    conn->incoming_cmd_cur_len = 0;
  }

  if (!control_protocol_is_valid(conn)) {
    return 0;
  }

 again:
  while (1) {
    size_t last_idx;
    int r;
    /* First, fetch a line. */
    do {
      data_len = conn->incoming_cmd_len - conn->incoming_cmd_cur_len;
      r = connection_buf_get_line(TO_CONN(conn),
                              conn->incoming_cmd+conn->incoming_cmd_cur_len,
                              &data_len);
      if (r == 0)
        /* Line not all here yet. Wait. */
        return 0;
      else if (r == -1) {
        if (data_len + conn->incoming_cmd_cur_len > MAX_COMMAND_LINE_LENGTH) {
          control_write_endreply(conn, 500, "Line too long.");
          connection_stop_reading(TO_CONN(conn));
          connection_mark_and_flush(TO_CONN(conn));
        }
        while (conn->incoming_cmd_len < data_len+conn->incoming_cmd_cur_len)
          conn->incoming_cmd_len *= 2;
        conn->incoming_cmd = tor_realloc(conn->incoming_cmd,
                                         conn->incoming_cmd_len);
      }
    } while (r != 1);

    tor_assert(data_len);

    last_idx = conn->incoming_cmd_cur_len;
    conn->incoming_cmd_cur_len += (int)data_len;

    /* We have appended a line to incoming_cmd.  Is the command done? */
    if (last_idx == 0 && *conn->incoming_cmd != '+')
      /* One line command, didn't start with '+'. */
      break;
    /* XXXX this code duplication is kind of dumb. */
    if (last_idx+3 == conn->incoming_cmd_cur_len &&
        tor_memeq(conn->incoming_cmd + last_idx, ".\r\n", 3)) {
      /* Just appended ".\r\n"; we're done. Remove it. */
      conn->incoming_cmd[last_idx] = '\0';
      conn->incoming_cmd_cur_len -= 3;
      break;
    } else if (last_idx+2 == conn->incoming_cmd_cur_len &&
               tor_memeq(conn->incoming_cmd + last_idx, ".\n", 2)) {
      /* Just appended ".\n"; we're done. Remove it. */
      conn->incoming_cmd[last_idx] = '\0';
      conn->incoming_cmd_cur_len -= 2;
      break;
    }
    /* Otherwise, read another line. */
  }
  data_len = conn->incoming_cmd_cur_len;

  /* Okay, we now have a command sitting on conn->incoming_cmd. See if we
   * recognize it.
   */
  tor_free(conn->current_cmd);
  args = control_split_incoming_command(conn->incoming_cmd, &data_len,
                                        &conn->current_cmd);
  if (BUG(!conn->current_cmd))
    return -1;

  /* If the connection is already closing, ignore further commands */
  if (TO_CONN(conn)->marked_for_close) {
    return 0;
  }

  /* Otherwise, Quit is always valid. */
  if (!strcasecmp(conn->current_cmd, "QUIT")) {
    control_write_endreply(conn, 250, "closing connection");
    connection_mark_and_flush(TO_CONN(conn));
    return 0;
  }

  if (conn->base_.state == CONTROL_CONN_STATE_NEEDAUTH &&
      !is_valid_initial_command(conn, conn->current_cmd)) {
    control_write_endreply(conn, 514, "Authentication required.");
    connection_mark_for_close(TO_CONN(conn));
    return 0;
  }

  if (data_len >= UINT32_MAX) {
    control_write_endreply(conn, 500, "A 4GB command? Nice try.");
    connection_mark_for_close(TO_CONN(conn));
    return 0;
  }

  cmd_data_len = (uint32_t)data_len;
  if (handle_control_command(conn, cmd_data_len, args) < 0)
    return -1;

  conn->incoming_cmd_cur_len = 0;
  goto again;
}

/** Cached liveness for network liveness events and GETINFO
 */

static int network_is_live = 0;

int
get_cached_network_liveness(void)
{
  return network_is_live;
}

void
set_cached_network_liveness(int liveness)
{
  network_is_live = liveness;
}

/** A copy of the process specifier of Tor's owning controller, or
 * NULL if this Tor instance is not currently owned by a process. */
static char *owning_controller_process_spec = NULL;

/** A process-termination monitor for Tor's owning controller, or NULL
 * if this Tor instance is not currently owned by a process. */
static tor_process_monitor_t *owning_controller_process_monitor = NULL;

/** Process-termination monitor callback for Tor's owning controller
 * process. */
static void
owning_controller_procmon_cb(void *unused)
{
  (void)unused;

  lost_owning_controller("process", "vanished");
}

/** Set <b>process_spec</b> as Tor's owning controller process.
 * Exit on failure. */
void
monitor_owning_controller_process(const char *process_spec)
{
  const char *msg;

  tor_assert((owning_controller_process_spec == NULL) ==
             (owning_controller_process_monitor == NULL));

  if (owning_controller_process_spec != NULL) {
    if ((process_spec != NULL) && !strcmp(process_spec,
                                          owning_controller_process_spec)) {
      /* Same process -- return now, instead of disposing of and
       * recreating the process-termination monitor. */
      return;
    }

    /* We are currently owned by a process, and we should no longer be
     * owned by it.  Free the process-termination monitor. */
    tor_process_monitor_free(owning_controller_process_monitor);
    owning_controller_process_monitor = NULL;

    tor_free(owning_controller_process_spec);
    owning_controller_process_spec = NULL;
  }

  tor_assert((owning_controller_process_spec == NULL) &&
             (owning_controller_process_monitor == NULL));

  if (process_spec == NULL)
    return;

  owning_controller_process_spec = tor_strdup(process_spec);
  owning_controller_process_monitor =
    tor_process_monitor_new(tor_libevent_get_base(),
                            owning_controller_process_spec,
                            LD_CONTROL,
                            owning_controller_procmon_cb, NULL,
                            &msg);

  if (owning_controller_process_monitor == NULL) {
    log_err(LD_BUG, "Couldn't create process-termination monitor for "
            "owning controller: %s.  Exiting.",
            msg);
    owning_controller_process_spec = NULL;
    tor_shutdown_event_loop_and_exit(1);
  }
}

/** Free any leftover allocated memory of the control.c subsystem. */
void
control_free_all(void)
{
  control_auth_free_all();
  control_events_free_all();
  control_cmd_free_all();
  control_event_bootstrap_reset();
}
