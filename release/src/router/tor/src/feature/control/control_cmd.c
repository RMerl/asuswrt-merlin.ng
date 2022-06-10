/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file control_cmd.c
 * \brief Implement various commands for Tor's control-socket interface.
 **/

#define CONTROL_MODULE_PRIVATE
#define CONTROL_CMD_PRIVATE
#define CONTROL_EVENTS_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "lib/confmgt/confmgt.h"
#include "app/main/main.h"
#include "core/mainloop/connection.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/or/connection_edge.h"
#include "core/or/circuitstats.h"
#include "core/or/extendinfo.h"
#include "feature/client/addressmap.h"
#include "feature/client/dnsserv.h"
#include "feature/client/entrynodes.h"
#include "feature/control/control.h"
#include "feature/control/control_auth.h"
#include "feature/control/control_cmd.h"
#include "feature/control/control_hs.h"
#include "feature/control/control_events.h"
#include "feature/control/control_getinfo.h"
#include "feature/control/control_proto.h"
#include "feature/hs/hs_control.h"
#include "feature/hs/hs_service.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerinfo.h"
#include "feature/nodelist/routerlist.h"
#include "feature/rend/rendcommon.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/encoding/confline.h"
#include "lib/encoding/kvline.h"

#include "core/or/cpath_build_state_st.h"
#include "core/or/entry_connection_st.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/socks_request_st.h"
#include "feature/control/control_cmd_args_st.h"
#include "feature/control/control_connection_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"

#include "app/config/statefile.h"

static int control_setconf_helper(control_connection_t *conn,
                                  const control_cmd_args_t *args,
                                  int use_defaults);

/** Yield true iff <b>s</b> is the state of a control_connection_t that has
 * finished authentication and is accepting commands. */
#define STATE_IS_OPEN(s) ((s) == CONTROL_CONN_STATE_OPEN)

/**
 * Release all storage held in <b>args</b>
 **/
void
control_cmd_args_free_(control_cmd_args_t *args)
{
  if (! args)
    return;

  if (args->args) {
    SMARTLIST_FOREACH(args->args, char *, c, tor_free(c));
    smartlist_free(args->args);
  }
  config_free_lines(args->kwargs);
  tor_free(args->cmddata);

  tor_free(args);
}

/** Erase all memory held in <b>args</b>. */
void
control_cmd_args_wipe(control_cmd_args_t *args)
{
  if (!args)
    return;

  if (args->args) {
    SMARTLIST_FOREACH(args->args, char *, c, memwipe(c, 0, strlen(c)));
  }
  for (config_line_t *line = args->kwargs; line; line = line->next) {
    memwipe(line->key, 0, strlen(line->key));
    memwipe(line->value, 0, strlen(line->value));
  }
  if (args->cmddata)
    memwipe(args->cmddata, 0, args->cmddata_len);
}

/**
 * Return true iff any element of the NULL-terminated <b>array</b> matches
 * <b>kwd</b>. Case-insensitive.
 **/
static bool
string_array_contains_keyword(const char **array, const char *kwd)
{
  for (unsigned i = 0; array[i]; ++i) {
    if (! strcasecmp(array[i], kwd))
      return true;
  }
  return false;
}

/** Helper for argument parsing: check whether the keyword arguments just
 * parsed in <b>result</b> were well-formed according to <b>syntax</b>.
 *
 * On success, return 0.  On failure, return -1 and set *<b>error_out</b>
 * to a newly allocated error string.
 **/
static int
kvline_check_keyword_args(const control_cmd_args_t *result,
                          const control_cmd_syntax_t *syntax,
                          char **error_out)
{
  if (result->kwargs == NULL) {
    tor_asprintf(error_out, "Cannot parse keyword argument(s)");
    return -1;
  }

  if (! syntax->allowed_keywords) {
    /* All keywords are permitted. */
    return 0;
  }

  /* Check for unpermitted arguments */
  const config_line_t *line;
  for (line = result->kwargs; line; line = line->next) {
    if (! string_array_contains_keyword(syntax->allowed_keywords,
                                        line->key)) {
      tor_asprintf(error_out, "Unrecognized keyword argument %s",
                   escaped(line->key));
      return -1;
    }
  }

  return 0;
}

/**
 * Helper: parse the arguments to a command according to <b>syntax</b>.  On
 * success, set *<b>error_out</b> to NULL and return a newly allocated
 * control_cmd_args_t.  On failure, set *<b>error_out</b> to newly allocated
 * error string, and return NULL.
 **/
STATIC control_cmd_args_t *
control_cmd_parse_args(const char *command,
                       const control_cmd_syntax_t *syntax,
                       size_t body_len,
                       const char *body,
                       char **error_out)
{
  *error_out = NULL;
  control_cmd_args_t *result = tor_malloc_zero(sizeof(control_cmd_args_t));
  const char *cmdline;
  char *cmdline_alloc = NULL;
  tor_assert(syntax->max_args < INT_MAX || syntax->max_args == UINT_MAX);

  result->command = command;

  if (syntax->store_raw_body) {
    tor_assert(body[body_len] == 0);
    result->raw_body = body;
  }

  const char *eol = memchr(body, '\n', body_len);
  if (syntax->want_cmddata) {
    if (! eol || (eol+1) == body+body_len) {
      *error_out = tor_strdup("Empty body");
      goto err;
    }
    cmdline_alloc = tor_memdup_nulterm(body, eol-body);
    cmdline = cmdline_alloc;
    ++eol;
    result->cmddata_len = read_escaped_data(eol, (body+body_len)-eol,
                                           &result->cmddata);
  } else {
    if (eol && (eol+1) != body+body_len) {
      *error_out = tor_strdup("Unexpected body");
      goto err;
    }
    cmdline = body;
  }

  result->args = smartlist_new();
  smartlist_split_string(result->args, cmdline, " ",
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK,
                         (int)(syntax->max_args+1));
  size_t n_args = smartlist_len(result->args);
  if (n_args < syntax->min_args) {
    tor_asprintf(error_out, "Need at least %u argument(s)",
                 syntax->min_args);
    goto err;
  } else if (n_args > syntax->max_args && ! syntax->accept_keywords) {
    tor_asprintf(error_out, "Cannot accept more than %u argument(s)",
                 syntax->max_args);
    goto err;
  }

  if (n_args > syntax->max_args) {
    /* We have extra arguments after the positional arguments, and we didn't
       treat them as an error, so they must count as keyword arguments: Either
       K=V pairs, or flags, or both. */
    tor_assert(n_args == syntax->max_args + 1);
    tor_assert(syntax->accept_keywords);
    char *remainder = smartlist_pop_last(result->args);
    result->kwargs = kvline_parse(remainder, syntax->kvline_flags);
    tor_free(remainder);
    if (kvline_check_keyword_args(result, syntax, error_out) < 0) {
      goto err;
    }
  }

  tor_assert_nonfatal(*error_out == NULL);
  goto done;
 err:
  tor_assert_nonfatal(*error_out != NULL);
  control_cmd_args_free(result);
 done:
  tor_free(cmdline_alloc);
  return result;
}

/**
 * Return true iff <b>lines</b> contains <b>flags</b> as a no-value
 * (keyword-only) entry.
 **/
static bool
config_lines_contain_flag(const config_line_t *lines, const char *flag)
{
  const config_line_t *line = config_line_find_case(lines, flag);
  return line && !strcmp(line->value, "");
}

static const control_cmd_syntax_t setconf_syntax = {
  .max_args=0,
  .accept_keywords=true,
  .kvline_flags=KV_OMIT_VALS|KV_QUOTED,
};

/** Called when we receive a SETCONF message: parse the body and try
 * to update our configuration.  Reply with a DONE or ERROR message.
 * Modifies the contents of body.*/
static int
handle_control_setconf(control_connection_t *conn,
                       const control_cmd_args_t *args)
{
  return control_setconf_helper(conn, args, 0);
}

static const control_cmd_syntax_t resetconf_syntax = {
  .max_args=0,
  .accept_keywords=true,
  .kvline_flags=KV_OMIT_VALS|KV_QUOTED,
};

/** Called when we receive a RESETCONF message: parse the body and try
 * to update our configuration.  Reply with a DONE or ERROR message.
 * Modifies the contents of body. */
static int
handle_control_resetconf(control_connection_t *conn,
                         const control_cmd_args_t *args)
{
  return control_setconf_helper(conn, args, 1);
}

static const control_cmd_syntax_t getconf_syntax = {
  .max_args=UINT_MAX
};

/** Called when we receive a GETCONF message.  Parse the request, and
 * reply with a CONFVALUE or an ERROR message */
static int
handle_control_getconf(control_connection_t *conn,
                       const control_cmd_args_t *args)
{
  const smartlist_t *questions = args->args;
  smartlist_t *answers = smartlist_new();
  smartlist_t *unrecognized = smartlist_new();
  const or_options_t *options = get_options();

  SMARTLIST_FOREACH_BEGIN(questions, const char *, q) {
    if (!option_is_recognized(q)) {
      control_reply_add_printf(unrecognized, 552,
                               "Unrecognized configuration key \"%s\"", q);
    } else {
      config_line_t *answer = option_get_assignment(options,q);
      if (!answer) {
        const char *name = option_get_canonical_name(q);
        control_reply_add_one_kv(answers, 250, KV_OMIT_VALS, name, "");
      }

      while (answer) {
        config_line_t *next;
        control_reply_add_one_kv(answers, 250, KV_RAW, answer->key,
                                 answer->value);
        next = answer->next;
        tor_free(answer->key);
        tor_free(answer->value);
        tor_free(answer);
        answer = next;
      }
    }
  } SMARTLIST_FOREACH_END(q);

  if (smartlist_len(unrecognized)) {
    control_write_reply_lines(conn, unrecognized);
  } else if (smartlist_len(answers)) {
    control_write_reply_lines(conn, answers);
  } else {
    send_control_done(conn);
  }

  control_reply_free(answers);
  control_reply_free(unrecognized);
  return 0;
}

static const control_cmd_syntax_t loadconf_syntax = {
  .want_cmddata = true
};

/** Called when we get a +LOADCONF message. */
static int
handle_control_loadconf(control_connection_t *conn,
                        const control_cmd_args_t *args)
{
  setopt_err_t retval;
  char *errstring = NULL;

  retval = options_init_from_string(NULL, args->cmddata,
                                    CMD_RUN_TOR, NULL, &errstring);

  if (retval != SETOPT_OK)
    log_warn(LD_CONTROL,
             "Controller gave us config file that didn't validate: %s",
             errstring);

#define SEND_ERRMSG(code, msg)                          \
  control_printf_endreply(conn, code, msg "%s%s",       \
                          errstring ? ": " : "",        \
                          errstring ? errstring : "")
  switch (retval) {
  case SETOPT_ERR_PARSE:
    SEND_ERRMSG(552, "Invalid config file");
    break;
  case SETOPT_ERR_TRANSITION:
    SEND_ERRMSG(553, "Transition not allowed");
    break;
  case SETOPT_ERR_SETTING:
    SEND_ERRMSG(553, "Unable to set option");
    break;
  case SETOPT_ERR_MISC:
  default:
    SEND_ERRMSG(550, "Unable to load config");
    break;
  case SETOPT_OK:
    send_control_done(conn);
    break;
  }
#undef SEND_ERRMSG
  tor_free(errstring);
  return 0;
}

static const control_cmd_syntax_t setevents_syntax = {
  .max_args = UINT_MAX
};

/** Called when we get a SETEVENTS message: update conn->event_mask,
 * and reply with DONE or ERROR. */
static int
handle_control_setevents(control_connection_t *conn,
                         const control_cmd_args_t *args)
{
  int event_code;
  event_mask_t event_mask = 0;
  const smartlist_t *events = args->args;

  SMARTLIST_FOREACH_BEGIN(events, const char *, ev)
    {
      if (!strcasecmp(ev, "EXTENDED") ||
          !strcasecmp(ev, "AUTHDIR_NEWDESCS")) {
        log_warn(LD_CONTROL, "The \"%s\" SETEVENTS argument is no longer "
                 "supported.", ev);
        continue;
      } else {
        int i;
        event_code = -1;

        for (i = 0; control_event_table[i].event_name != NULL; ++i) {
          if (!strcasecmp(ev, control_event_table[i].event_name)) {
            event_code = control_event_table[i].event_code;
            break;
          }
        }

        if (event_code == -1) {
          control_printf_endreply(conn, 552, "Unrecognized event \"%s\"", ev);
          return 0;
        }
      }
      event_mask |= (((event_mask_t)1) << event_code);
    }
  SMARTLIST_FOREACH_END(ev);

  conn->event_mask = event_mask;

  control_update_global_event_mask();
  send_control_done(conn);
  return 0;
}

static const control_cmd_syntax_t saveconf_syntax = {
  .max_args = 0,
  .accept_keywords = true,
  .kvline_flags=KV_OMIT_VALS,
};

/** Called when we get a SAVECONF command. Try to flush the current options to
 * disk, and report success or failure. */
static int
handle_control_saveconf(control_connection_t *conn,
                        const control_cmd_args_t *args)
{
  bool force = config_lines_contain_flag(args->kwargs, "FORCE");
  const or_options_t *options = get_options();
  if ((!force && options->IncludeUsed) || options_save_current() < 0) {
    control_write_endreply(conn, 551,
                           "Unable to write configuration to disk.");
  } else {
    send_control_done(conn);
  }
  return 0;
}

static const control_cmd_syntax_t signal_syntax = {
  .min_args = 1,
  .max_args = 1,
};

/** Called when we get a SIGNAL command. React to the provided signal, and
 * report success or failure. (If the signal results in a shutdown, success
 * may not be reported.) */
static int
handle_control_signal(control_connection_t *conn,
                      const control_cmd_args_t *args)
{
  int sig = -1;
  int i;

  tor_assert(smartlist_len(args->args) == 1);
  const char *s = smartlist_get(args->args, 0);

  for (i = 0; signal_table[i].signal_name != NULL; ++i) {
    if (!strcasecmp(s, signal_table[i].signal_name)) {
      sig = signal_table[i].sig;
      break;
    }
  }

  if (sig < 0)
    control_printf_endreply(conn, 552, "Unrecognized signal code \"%s\"", s);
  if (sig < 0)
    return 0;

  send_control_done(conn);
  /* Flush the "done" first if the signal might make us shut down. */
  if (sig == SIGTERM || sig == SIGINT)
    connection_flush(TO_CONN(conn));

  activate_signal(sig);

  return 0;
}

static const control_cmd_syntax_t takeownership_syntax = {
  .max_args = UINT_MAX, // This should probably become zero. XXXXX
};

/** Called when we get a TAKEOWNERSHIP command.  Mark this connection
 * as an owning connection, so that we will exit if the connection
 * closes. */
static int
handle_control_takeownership(control_connection_t *conn,
                             const control_cmd_args_t *args)
{
  (void)args;

  conn->is_owning_control_connection = 1;

  log_info(LD_CONTROL, "Control connection %d has taken ownership of this "
           "Tor instance.",
           (int)(conn->base_.s));

  send_control_done(conn);
  return 0;
}

static const control_cmd_syntax_t dropownership_syntax = {
  .max_args = UINT_MAX, // This should probably become zero. XXXXX
};

/** Called when we get a DROPOWNERSHIP command.  Mark this connection
 * as a non-owning connection, so that we will not exit if the connection
 * closes. */
static int
handle_control_dropownership(control_connection_t *conn,
                             const control_cmd_args_t *args)
{
  (void)args;

  conn->is_owning_control_connection = 0;

  log_info(LD_CONTROL, "Control connection %d has dropped ownership of this "
           "Tor instance.",
           (int)(conn->base_.s));

  send_control_done(conn);
  return 0;
}

/** Given a text circuit <b>id</b>, return the corresponding circuit. */
static origin_circuit_t *
get_circ(const char *id)
{
  uint32_t n_id;
  int ok;
  n_id = (uint32_t) tor_parse_ulong(id, 10, 0, UINT32_MAX, &ok, NULL);
  if (!ok)
    return NULL;
  return circuit_get_by_global_id(n_id);
}

/** Given a text stream <b>id</b>, return the corresponding AP connection. */
static entry_connection_t *
get_stream(const char *id)
{
  uint64_t n_id;
  int ok;
  connection_t *conn;
  n_id = tor_parse_uint64(id, 10, 0, UINT64_MAX, &ok, NULL);
  if (!ok)
    return NULL;
  conn = connection_get_by_global_id(n_id);
  if (!conn || conn->type != CONN_TYPE_AP || conn->marked_for_close)
    return NULL;
  return TO_ENTRY_CONN(conn);
}

/** Helper for setconf and resetconf. Acts like setconf, except
 * it passes <b>use_defaults</b> on to options_trial_assign().  Modifies the
 * contents of body.
 */
static int
control_setconf_helper(control_connection_t *conn,
                       const control_cmd_args_t *args,
                       int use_defaults)
{
  setopt_err_t opt_err;
  char *errstring = NULL;
  const unsigned flags =
    CAL_CLEAR_FIRST | (use_defaults ? CAL_USE_DEFAULTS : 0);

  // We need a copy here, since confmgt.c wants to canonicalize cases.
  config_line_t *lines = config_lines_dup(args->kwargs);

  opt_err = options_trial_assign(lines, flags, &errstring);
  {
#define SEND_ERRMSG(code, msg)                                  \
    control_printf_endreply(conn, code, msg ": %s", errstring);

    switch (opt_err) {
      case SETOPT_ERR_MISC:
        SEND_ERRMSG(552, "Unrecognized option");
        break;
      case SETOPT_ERR_PARSE:
        SEND_ERRMSG(513, "Unacceptable option value");
        break;
      case SETOPT_ERR_TRANSITION:
        SEND_ERRMSG(553, "Transition not allowed");
        break;
      case SETOPT_ERR_SETTING:
      default:
        SEND_ERRMSG(553, "Unable to set option");
        break;
      case SETOPT_OK:
        config_free_lines(lines);
        send_control_done(conn);
        return 0;
    }
#undef SEND_ERRMSG
    log_warn(LD_CONTROL,
             "Controller gave us config lines that didn't validate: %s",
             errstring);
    config_free_lines(lines);
    tor_free(errstring);
    return 0;
  }
}

/** Return true iff <b>addr</b> is unusable as a mapaddress target because of
 * containing funny characters. */
static int
address_is_invalid_mapaddress_target(const char *addr)
{
  if (!strcmpstart(addr, "*."))
    return address_is_invalid_destination(addr+2, 1);
  else
    return address_is_invalid_destination(addr, 1);
}

static const control_cmd_syntax_t mapaddress_syntax = {
  // no positional arguments are expected
  .max_args=0,
  // an arbitrary number of K=V entries are supported.
  .accept_keywords=true,
};

/** Called when we get a MAPADDRESS command; try to bind all listed addresses,
 * and report success or failure. */
static int
handle_control_mapaddress(control_connection_t *conn,
                          const control_cmd_args_t *args)
{
  smartlist_t *reply;
  char *r;
  size_t sz;

  reply = smartlist_new();
  const config_line_t *line;
  for (line = args->kwargs; line; line = line->next) {
    const char *from = line->key;
    const char *to = line->value;
    {
      if (address_is_invalid_mapaddress_target(to)) {
        smartlist_add_asprintf(reply,
                     "512-syntax error: invalid address '%s'", to);
        log_warn(LD_CONTROL,
                 "Skipping invalid argument '%s' in MapAddress msg", to);
      } else if (!strcmp(from, ".") || !strcmp(from, "0.0.0.0") ||
                 !strcmp(from, "::")) {
        const char type =
          !strcmp(from,".") ? RESOLVED_TYPE_HOSTNAME :
          (!strcmp(from, "0.0.0.0") ? RESOLVED_TYPE_IPV4 : RESOLVED_TYPE_IPV6);
        const char *address = addressmap_register_virtual_address(
                                                     type, tor_strdup(to));
        if (!address) {
          smartlist_add_asprintf(reply,
                   "451-resource exhausted: skipping '%s=%s'", from,to);
          log_warn(LD_CONTROL,
                   "Unable to allocate address for '%s' in MapAddress msg",
                   safe_str_client(to));
        } else {
          smartlist_add_asprintf(reply, "250-%s=%s", address, to);
        }
      } else {
        const char *msg;
        if (addressmap_register_auto(from, to, 1,
                                     ADDRMAPSRC_CONTROLLER, &msg) < 0) {
          smartlist_add_asprintf(reply,
                                 "512-syntax error: invalid address mapping "
                                 " '%s=%s': %s", from, to, msg);
          log_warn(LD_CONTROL,
                   "Skipping invalid argument '%s=%s' in MapAddress msg: %s",
                   from, to, msg);
        } else {
          smartlist_add_asprintf(reply, "250-%s=%s", from, to);
        }
      }
    }
  }

  if (smartlist_len(reply)) {
    ((char*)smartlist_get(reply,smartlist_len(reply)-1))[3] = ' ';
    r = smartlist_join_strings(reply, "\r\n", 1, &sz);
    connection_buf_add(r, sz, TO_CONN(conn));
    tor_free(r);
  } else {
    control_write_endreply(conn, 512, "syntax error: "
                           "not enough arguments to mapaddress.");
  }

  SMARTLIST_FOREACH(reply, char *, cp, tor_free(cp));
  smartlist_free(reply);
  return 0;
}

/** Given a string, convert it to a circuit purpose. */
static uint8_t
circuit_purpose_from_string(const char *string)
{
  if (!strcasecmpstart(string, "purpose="))
    string += strlen("purpose=");

  if (!strcasecmp(string, "general"))
    return CIRCUIT_PURPOSE_C_GENERAL;
  else if (!strcasecmp(string, "controller"))
    return CIRCUIT_PURPOSE_CONTROLLER;
  else
    return CIRCUIT_PURPOSE_UNKNOWN;
}

static const control_cmd_syntax_t extendcircuit_syntax = {
  .min_args=1,
  .max_args=1, // see note in function
  .accept_keywords=true,
  .kvline_flags=KV_OMIT_VALS
};

/** Called when we get an EXTENDCIRCUIT message.  Try to extend the listed
 * circuit, and report success or failure. */
static int
handle_control_extendcircuit(control_connection_t *conn,
                             const control_cmd_args_t *args)
{
  smartlist_t *router_nicknames=smartlist_new(), *nodes=NULL;
  origin_circuit_t *circ = NULL;
  uint8_t intended_purpose = CIRCUIT_PURPOSE_C_GENERAL;
  const config_line_t *kwargs = args->kwargs;
  const char *circ_id = smartlist_get(args->args, 0);
  const char *path_str = NULL;
  char *path_str_alloc = NULL;

  /* The syntax for this command is unfortunate. The second argument is
     optional, and is a comma-separated list long-format fingerprints, which
     can (historically!) contain an equals sign.

     Here we check the second argument to see if it's a path, and if so we
     remove it from the kwargs list and put it in path_str.
  */
  if (kwargs) {
    const config_line_t *arg1 = kwargs;
    if (!strcmp(arg1->value, "")) {
      path_str = arg1->key;
      kwargs = kwargs->next;
    } else if (arg1->key[0] == '$') {
      tor_asprintf(&path_str_alloc, "%s=%s", arg1->key, arg1->value);
      path_str = path_str_alloc;
      kwargs = kwargs->next;
    }
  }

  const config_line_t *purpose_line = config_line_find_case(kwargs, "PURPOSE");
  bool zero_circ = !strcmp("0", circ_id);

  if (purpose_line) {
    intended_purpose = circuit_purpose_from_string(purpose_line->value);
    if (intended_purpose == CIRCUIT_PURPOSE_UNKNOWN) {
      control_printf_endreply(conn, 552, "Unknown purpose \"%s\"",
                              purpose_line->value);
      goto done;
    }
  }

  if (zero_circ) {
    if (!path_str) {
      // "EXTENDCIRCUIT 0" with no path.
      circ = circuit_launch(intended_purpose, CIRCLAUNCH_NEED_CAPACITY);
      if (!circ) {
        control_write_endreply(conn, 551, "Couldn't start circuit");
      } else {
        control_printf_endreply(conn, 250, "EXTENDED %lu",
                                (unsigned long)circ->global_identifier);
      }
      goto done;
    }
  }

  if (!zero_circ && !(circ = get_circ(circ_id))) {
    control_printf_endreply(conn, 552, "Unknown circuit \"%s\"", circ_id);
    goto done;
  }

  if (!path_str) {
    control_write_endreply(conn, 512, "syntax error: path required.");
    goto done;
  }

  smartlist_split_string(router_nicknames, path_str, ",", 0, 0);

  nodes = smartlist_new();
  bool first_node = zero_circ;
  SMARTLIST_FOREACH_BEGIN(router_nicknames, const char *, n) {
    const node_t *node = node_get_by_nickname(n, 0);
    if (!node) {
      control_printf_endreply(conn, 552, "No such router \"%s\"", n);
      goto done;
    }
    if (!node_has_preferred_descriptor(node, first_node)) {
      control_printf_endreply(conn, 552, "No descriptor for \"%s\"", n);
      goto done;
    }
    smartlist_add(nodes, (void*)node);
    first_node = false;
  } SMARTLIST_FOREACH_END(n);

  if (!smartlist_len(nodes)) {
    control_write_endreply(conn, 512, "No router names provided");
    goto done;
  }

  if (zero_circ) {
    /* start a new circuit */
    circ = origin_circuit_init(intended_purpose, 0);
    circ->first_hop_from_controller = 1;
  }

  circ->any_hop_from_controller = 1;

  /* now circ refers to something that is ready to be extended */
  first_node = zero_circ;
  SMARTLIST_FOREACH(nodes, const node_t *, node,
  {
    /* We treat every hop as an exit to try to negotiate congestion
     * control, because we have no idea which hop the controller wil
     * try to use for streams and when */
    extend_info_t *info = extend_info_from_node(node, first_node, true);
    if (!info) {
      tor_assert_nonfatal(first_node);
      log_warn(LD_CONTROL,
               "controller tried to connect to a node that lacks a suitable "
               "descriptor, or which doesn't have any "
               "addresses that are allowed by the firewall configuration; "
               "circuit marked for closing.");
      circuit_mark_for_close(TO_CIRCUIT(circ), -END_CIRC_REASON_CONNECTFAILED);
      control_write_endreply(conn, 551, "Couldn't start circuit");
      goto done;
    }
    circuit_append_new_exit(circ, info);
    if (circ->build_state->desired_path_len > 1) {
      circ->build_state->onehop_tunnel = 0;
    }
    extend_info_free(info);
    first_node = 0;
  });

  /* now that we've populated the cpath, start extending */
  if (zero_circ) {
    int err_reason = 0;
    if ((err_reason = circuit_handle_first_hop(circ)) < 0) {
      circuit_mark_for_close(TO_CIRCUIT(circ), -err_reason);
      control_write_endreply(conn, 551, "Couldn't start circuit");
      goto done;
    }
  } else {
    if (circ->base_.state == CIRCUIT_STATE_OPEN ||
        circ->base_.state == CIRCUIT_STATE_GUARD_WAIT) {
      int err_reason = 0;
      circuit_set_state(TO_CIRCUIT(circ), CIRCUIT_STATE_BUILDING);
      if ((err_reason = circuit_send_next_onion_skin(circ)) < 0) {
        log_info(LD_CONTROL,
                 "send_next_onion_skin failed; circuit marked for closing.");
        circuit_mark_for_close(TO_CIRCUIT(circ), -err_reason);
        control_write_endreply(conn, 551, "Couldn't send onion skin");
        goto done;
      }
    }
  }

  control_printf_endreply(conn, 250, "EXTENDED %lu",
                          (unsigned long)circ->global_identifier);
  if (zero_circ) /* send a 'launched' event, for completeness */
    circuit_event_status(circ, CIRC_EVENT_LAUNCHED, 0);
 done:
  SMARTLIST_FOREACH(router_nicknames, char *, n, tor_free(n));
  smartlist_free(router_nicknames);
  smartlist_free(nodes);
  tor_free(path_str_alloc);
  return 0;
}

static const control_cmd_syntax_t setcircuitpurpose_syntax = {
  .max_args=1,
  .accept_keywords=true,
};

/** Called when we get a SETCIRCUITPURPOSE message. If we can find the
 * circuit and it's a valid purpose, change it. */
static int
handle_control_setcircuitpurpose(control_connection_t *conn,
                                 const control_cmd_args_t *args)
{
  origin_circuit_t *circ = NULL;
  uint8_t new_purpose;
  const char *circ_id = smartlist_get(args->args,0);

  if (!(circ = get_circ(circ_id))) {
    control_printf_endreply(conn, 552, "Unknown circuit \"%s\"", circ_id);
    goto done;
  }

  {
    const config_line_t *purp = config_line_find_case(args->kwargs, "PURPOSE");
    if (!purp) {
      control_write_endreply(conn, 552, "No purpose given");
      goto done;
    }
    new_purpose = circuit_purpose_from_string(purp->value);
    if (new_purpose == CIRCUIT_PURPOSE_UNKNOWN) {
      control_printf_endreply(conn, 552, "Unknown purpose \"%s\"",
                              purp->value);
      goto done;
    }
  }

  circuit_change_purpose(TO_CIRCUIT(circ), new_purpose);
  send_control_done(conn);

 done:
  return 0;
}

static const char *attachstream_keywords[] = {
  "HOP", NULL
};
static const control_cmd_syntax_t attachstream_syntax = {
  .min_args=2, .max_args=2,
  .accept_keywords=true,
  .allowed_keywords=attachstream_keywords
};

/** Called when we get an ATTACHSTREAM message.  Try to attach the requested
 * stream, and report success or failure. */
static int
handle_control_attachstream(control_connection_t *conn,
                            const control_cmd_args_t *args)
{
  entry_connection_t *ap_conn = NULL;
  origin_circuit_t *circ = NULL;
  crypt_path_t *cpath=NULL;
  int hop=0, hop_line_ok=1;
  const char *stream_id = smartlist_get(args->args, 0);
  const char *circ_id = smartlist_get(args->args, 1);
  int zero_circ = !strcmp(circ_id, "0");
  const config_line_t *hoparg = config_line_find_case(args->kwargs, "HOP");

  if (!(ap_conn = get_stream(stream_id))) {
    control_printf_endreply(conn, 552, "Unknown stream \"%s\"", stream_id);
    return 0;
  } else if (!zero_circ && !(circ = get_circ(circ_id))) {
    control_printf_endreply(conn, 552, "Unknown circuit \"%s\"", circ_id);
    return 0;
  } else if (circ) {
    if (hoparg) {
      hop = (int) tor_parse_ulong(hoparg->value, 10, 0, INT_MAX,
                                  &hop_line_ok, NULL);
      if (!hop_line_ok) { /* broken hop line */
        control_printf_endreply(conn, 552, "Bad value hop=%s",
                                hoparg->value);
        return 0;
      }
    }
  }

  if (ENTRY_TO_CONN(ap_conn)->state != AP_CONN_STATE_CONTROLLER_WAIT &&
      ENTRY_TO_CONN(ap_conn)->state != AP_CONN_STATE_CONNECT_WAIT &&
      ENTRY_TO_CONN(ap_conn)->state != AP_CONN_STATE_RESOLVE_WAIT) {
    control_write_endreply(conn, 555,
                           "Connection is not managed by controller.");
    return 0;
  }

  /* Do we need to detach it first? */
  if (ENTRY_TO_CONN(ap_conn)->state != AP_CONN_STATE_CONTROLLER_WAIT) {
    edge_connection_t *edge_conn = ENTRY_TO_EDGE_CONN(ap_conn);
    circuit_t *tmpcirc = circuit_get_by_edge_conn(edge_conn);
    connection_edge_end(edge_conn, END_STREAM_REASON_TIMEOUT);
    /* Un-mark it as ending, since we're going to reuse it. */
    edge_conn->edge_has_sent_end = 0;
    edge_conn->end_reason = 0;
    if (tmpcirc)
      circuit_detach_stream(tmpcirc, edge_conn);
    connection_entry_set_controller_wait(ap_conn);
  }

  if (circ && (circ->base_.state != CIRCUIT_STATE_OPEN)) {
    control_write_endreply(conn, 551,
                           "Can't attach stream to non-open origin circuit");
    return 0;
  }
  /* Is this a single hop circuit? */
  if (circ && (circuit_get_cpath_len(circ)<2 || hop==1)) {
    control_write_endreply(conn, 551,
                           "Can't attach stream to this one-hop circuit.");
    return 0;
  }

  if (circ && hop>0) {
    /* find this hop in the circuit, and set cpath */
    cpath = circuit_get_cpath_hop(circ, hop);
    if (!cpath) {
      control_printf_endreply(conn, 551, "Circuit doesn't have %d hops.", hop);
      return 0;
    }
  }
  if (connection_ap_handshake_rewrite_and_attach(ap_conn, circ, cpath) < 0) {
    control_write_endreply(conn, 551, "Unable to attach stream");
    return 0;
  }
  send_control_done(conn);
  return 0;
}

static const char *postdescriptor_keywords[] = {
  "cache", "purpose", NULL,
};

static const control_cmd_syntax_t postdescriptor_syntax = {
  .max_args = 0,
  .accept_keywords = true,
  .allowed_keywords = postdescriptor_keywords,
  .want_cmddata = true,
};

/** Called when we get a POSTDESCRIPTOR message.  Try to learn the provided
 * descriptor, and report success or failure. */
static int
handle_control_postdescriptor(control_connection_t *conn,
                              const control_cmd_args_t *args)
{
  const char *msg=NULL;
  uint8_t purpose = ROUTER_PURPOSE_GENERAL;
  int cache = 0; /* eventually, we may switch this to 1 */
  const config_line_t *line;

  line = config_line_find_case(args->kwargs, "purpose");
  if (line) {
    purpose = router_purpose_from_string(line->value);
    if (purpose == ROUTER_PURPOSE_UNKNOWN) {
      control_printf_endreply(conn, 552, "Unknown purpose \"%s\"",
                              line->value);
      goto done;
    }
  }
  line = config_line_find_case(args->kwargs, "cache");
  if (line) {
    if (!strcasecmp(line->value, "no"))
      cache = 0;
    else if (!strcasecmp(line->value, "yes"))
      cache = 1;
    else {
      control_printf_endreply(conn, 552, "Unknown cache request \"%s\"",
                              line->value);
      goto done;
    }
  }

  switch (router_load_single_router(args->cmddata, purpose, cache, &msg)) {
  case -1:
    if (!msg) msg = "Could not parse descriptor";
    control_write_endreply(conn, 554, msg);
    break;
  case 0:
    if (!msg) msg = "Descriptor not added";
    control_write_endreply(conn, 251, msg);
    break;
  case 1:
    send_control_done(conn);
    break;
  }

 done:
  return 0;
}

static const control_cmd_syntax_t redirectstream_syntax = {
  .min_args = 2,
  .max_args = UINT_MAX, // XXX should be 3.
};

/** Called when we receive a REDIRECTSTREAM command.  Try to change the target
 * address of the named AP stream, and report success or failure. */
static int
handle_control_redirectstream(control_connection_t *conn,
                              const control_cmd_args_t *cmd_args)
{
  entry_connection_t *ap_conn = NULL;
  char *new_addr = NULL;
  uint16_t new_port = 0;
  const smartlist_t *args = cmd_args->args;

  if (!(ap_conn = get_stream(smartlist_get(args, 0)))
           || !ap_conn->socks_request) {
    control_printf_endreply(conn, 552, "Unknown stream \"%s\"",
                            (char*)smartlist_get(args, 0));
  } else {
    int ok = 1;
    if (smartlist_len(args) > 2) { /* they included a port too */
      new_port = (uint16_t) tor_parse_ulong(smartlist_get(args, 2),
                                            10, 1, 65535, &ok, NULL);
    }
    if (!ok) {
      control_printf_endreply(conn, 512, "Cannot parse port \"%s\"",
                              (char*)smartlist_get(args, 2));
    } else {
      new_addr = tor_strdup(smartlist_get(args, 1));
    }
  }

  if (!new_addr)
    return 0;

  strlcpy(ap_conn->socks_request->address, new_addr,
          sizeof(ap_conn->socks_request->address));
  if (new_port)
    ap_conn->socks_request->port = new_port;
  tor_free(new_addr);
  send_control_done(conn);
  return 0;
}

static const control_cmd_syntax_t closestream_syntax = {
  .min_args = 2,
  .max_args = UINT_MAX, /* XXXX This is the original behavior, but
                         * maybe we should change the spec. */
};

/** Called when we get a CLOSESTREAM command; try to close the named stream
 * and report success or failure. */
static int
handle_control_closestream(control_connection_t *conn,
                           const control_cmd_args_t *cmd_args)
{
  entry_connection_t *ap_conn=NULL;
  uint8_t reason=0;
  int ok;
  const smartlist_t *args = cmd_args->args;

  tor_assert(smartlist_len(args) >= 2);

  if (!(ap_conn = get_stream(smartlist_get(args, 0))))
    control_printf_endreply(conn, 552, "Unknown stream \"%s\"",
                            (char*)smartlist_get(args, 0));
  else {
    reason = (uint8_t) tor_parse_ulong(smartlist_get(args,1), 10, 0, 255,
                                       &ok, NULL);
    if (!ok) {
      control_printf_endreply(conn, 552, "Unrecognized reason \"%s\"",
                              (char*)smartlist_get(args, 1));
      ap_conn = NULL;
    }
  }
  if (!ap_conn)
    return 0;

  connection_mark_unattached_ap(ap_conn, reason);
  send_control_done(conn);
  return 0;
}

static const control_cmd_syntax_t closecircuit_syntax = {
  .min_args=1, .max_args=1,
  .accept_keywords=true,
  .kvline_flags=KV_OMIT_VALS,
  // XXXX we might want to exclude unrecognized flags, but for now we
  // XXXX just ignore them for backward compatibility.
};

/** Called when we get a CLOSECIRCUIT command; try to close the named circuit
 * and report success or failure. */
static int
handle_control_closecircuit(control_connection_t *conn,
                            const control_cmd_args_t *args)
{
  const char *circ_id = smartlist_get(args->args, 0);
  origin_circuit_t *circ = NULL;

  if (!(circ=get_circ(circ_id))) {
    control_printf_endreply(conn, 552, "Unknown circuit \"%s\"", circ_id);
    return 0;
  }

  bool safe =  config_lines_contain_flag(args->kwargs, "IfUnused");

  if (!safe || !circ->p_streams) {
    circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_REQUESTED);
  }

  send_control_done(conn);
  return 0;
}

static const control_cmd_syntax_t resolve_syntax = {
  .max_args=0,
  .accept_keywords=true,
  .kvline_flags=KV_OMIT_VALS,
};

/** Called when we get a RESOLVE command: start trying to resolve
 * the listed addresses. */
static int
handle_control_resolve(control_connection_t *conn,
                       const control_cmd_args_t *args)
{
  smartlist_t *failed;
  int is_reverse = 0;

  if (!(conn->event_mask & (((event_mask_t)1)<<EVENT_ADDRMAP))) {
    log_warn(LD_CONTROL, "Controller asked us to resolve an address, but "
             "isn't listening for ADDRMAP events.  It probably won't see "
             "the answer.");
  }

  {
    const config_line_t *modearg = config_line_find_case(args->kwargs, "mode");
    if (modearg && !strcasecmp(modearg->value, "reverse"))
      is_reverse = 1;
  }
  failed = smartlist_new();
  for (const config_line_t *line = args->kwargs; line; line = line->next) {
    if (!strlen(line->value)) {
      const char *addr = line->key;
      if (dnsserv_launch_request(addr, is_reverse, conn)<0)
        smartlist_add(failed, (char*)addr);
    } else {
      // XXXX arguably we should reject unrecognized keyword arguments,
      // XXXX but the old implementation didn't do that.
    }
  }

  send_control_done(conn);
  SMARTLIST_FOREACH(failed, const char *, arg, {
      control_event_address_mapped(arg, arg, time(NULL),
                                   "internal", 0, 0);
  });

  smartlist_free(failed);
  return 0;
}

static const control_cmd_syntax_t protocolinfo_syntax = {
  .max_args = UINT_MAX
};

/** Return a comma-separated list of authentication methods for
    handle_control_protocolinfo().  Caller must free this string. */
static char *
get_authmethods(const or_options_t *options)
{
  int cookies = options->CookieAuthentication;
  char *methods;
  int passwd = (options->HashedControlPassword != NULL ||
                options->HashedControlSessionPassword != NULL);
  smartlist_t *mlist = smartlist_new();

  if (cookies) {
    smartlist_add(mlist, (char*)"COOKIE");
    smartlist_add(mlist, (char*)"SAFECOOKIE");
  }
  if (passwd)
    smartlist_add(mlist, (char*)"HASHEDPASSWORD");
  if (!cookies && !passwd)
    smartlist_add(mlist, (char*)"NULL");
  methods = smartlist_join_strings(mlist, ",", 0, NULL);
  smartlist_free(mlist);

  return methods;
}

/** Return escaped cookie filename.  Caller must free this string.
    Return NULL if cookie authentication is disabled. */
static char *
get_esc_cfile(const or_options_t *options)
{
  char *cfile = NULL, *abs_cfile = NULL, *esc_cfile = NULL;

  if (!options->CookieAuthentication)
    return NULL;

  cfile = get_controller_cookie_file_name();
  abs_cfile = make_path_absolute(cfile);
  esc_cfile = esc_for_log(abs_cfile);
  tor_free(cfile);
  tor_free(abs_cfile);
  return esc_cfile;
}

/** Compose the auth methods line of a PROTOCOLINFO reply. */
static void
add_authmethods(smartlist_t *reply)
{
  const or_options_t *options = get_options();
  char *methods = get_authmethods(options);
  char *esc_cfile = get_esc_cfile(options);

  control_reply_add_str(reply, 250, "AUTH");
  control_reply_append_kv(reply, "METHODS", methods);
  if (esc_cfile)
    control_reply_append_kv(reply, "COOKIEFILE", esc_cfile);

  tor_free(methods);
  tor_free(esc_cfile);
}

/** Called when we get a PROTOCOLINFO command: send back a reply. */
static int
handle_control_protocolinfo(control_connection_t *conn,
                            const control_cmd_args_t *cmd_args)
{
  const char *bad_arg = NULL;
  const smartlist_t *args = cmd_args->args;
  smartlist_t *reply = NULL;

  conn->have_sent_protocolinfo = 1;

  SMARTLIST_FOREACH(args, const char *, arg, {
      int ok;
      tor_parse_long(arg, 10, 0, LONG_MAX, &ok, NULL);
      if (!ok) {
        bad_arg = arg;
        break;
      }
    });
  if (bad_arg) {
    control_printf_endreply(conn, 513, "No such version %s",
                            escaped(bad_arg));
    /* Don't tolerate bad arguments when not authenticated. */
    if (!STATE_IS_OPEN(TO_CONN(conn)->state))
      connection_mark_for_close(TO_CONN(conn));
    return 0;
  }
  reply = smartlist_new();
  control_reply_add_str(reply, 250, "PROTOCOLINFO 1");
  add_authmethods(reply);
  control_reply_add_str(reply, 250, "VERSION");
  control_reply_append_kv(reply, "Tor", escaped(VERSION));
  control_reply_add_done(reply);

  control_write_reply_lines(conn, reply);
  control_reply_free(reply);
  return 0;
}

static const control_cmd_syntax_t usefeature_syntax = {
  .max_args = UINT_MAX
};

/** Called when we get a USEFEATURE command: parse the feature list, and
 * set up the control_connection's options properly. */
static int
handle_control_usefeature(control_connection_t *conn,
                          const control_cmd_args_t *cmd_args)
{
  const smartlist_t *args = cmd_args->args;
  int bad = 0;
  SMARTLIST_FOREACH_BEGIN(args, const char *, arg) {
      if (!strcasecmp(arg, "VERBOSE_NAMES"))
        ;
      else if (!strcasecmp(arg, "EXTENDED_EVENTS"))
        ;
      else {
        control_printf_endreply(conn, 552, "Unrecognized feature \"%s\"",
                                arg);
        bad = 1;
        break;
      }
  } SMARTLIST_FOREACH_END(arg);

  if (!bad) {
    send_control_done(conn);
  }

  return 0;
}

static const control_cmd_syntax_t dropguards_syntax = {
  .max_args = 0,
};

/** Implementation for the DROPGUARDS command. */
static int
handle_control_dropguards(control_connection_t *conn,
                          const control_cmd_args_t *args)
{
  (void) args; /* We don't take arguments. */

  static int have_warned = 0;
  if (! have_warned) {
    log_warn(LD_CONTROL, "DROPGUARDS is dangerous; make sure you understand "
             "the risks before using it. It may be removed in a future "
             "version of Tor.");
    have_warned = 1;
  }

  remove_all_entry_guards();
  send_control_done(conn);

  return 0;
}

static const control_cmd_syntax_t droptimeouts_syntax = {
  .max_args = 0,
};

/** Implementation for the DROPTIMEOUTS command. */
static int
handle_control_droptimeouts(control_connection_t *conn,
                          const control_cmd_args_t *args)
{
  (void) args; /* We don't take arguments. */

  static int have_warned = 0;
  if (! have_warned) {
    log_warn(LD_CONTROL, "DROPTIMEOUTS is dangerous; make sure you understand "
             "the risks before using it. It may be removed in a future "
             "version of Tor.");
    have_warned = 1;
  }

  circuit_build_times_reset(get_circuit_build_times_mutable());
  send_control_done(conn);
  or_state_mark_dirty(get_or_state(), 0);
  cbt_control_event_buildtimeout_set(get_circuit_build_times(),
                                     BUILDTIMEOUT_SET_EVENT_RESET);

  return 0;
}

static const char *hsfetch_keywords[] = {
  "SERVER", NULL,
};
static const control_cmd_syntax_t hsfetch_syntax = {
  .min_args = 1, .max_args = 1,
  .accept_keywords = true,
  .allowed_keywords = hsfetch_keywords,
};

/** Implementation for the HSFETCH command. */
static int
handle_control_hsfetch(control_connection_t *conn,
                       const control_cmd_args_t *args)

{
  smartlist_t *hsdirs = NULL;
  ed25519_public_key_t v3_pk;
  uint32_t version;
  const char *hsaddress = NULL;

  /* Extract the first argument (either HSAddress or DescID). */
  const char *arg1 = smartlist_get(args->args, 0);
  if (hs_address_is_valid(arg1)) {
    hsaddress = arg1;
    version = HS_VERSION_THREE;
    hs_parse_address(hsaddress, &v3_pk, NULL, NULL);
  } else {
    control_printf_endreply(conn, 513, "Invalid argument \"%s\"", arg1);
    goto done;
  }

  for (const config_line_t *line = args->kwargs; line; line = line->next) {
    if (!strcasecmp(line->key, "SERVER")) {
      const char *server = line->value;

      const node_t *node = node_get_by_hex_id(server, 0);
      if (!node) {
        control_printf_endreply(conn, 552, "Server \"%s\" not found", server);
        goto done;
      }
      if (!hsdirs) {
        /* Stores routerstatus_t cmddata for each specified server. */
        hsdirs = smartlist_new();
      }
      /* Valid server, add it to our local list. */
      smartlist_add(hsdirs, node->rs);
    } else {
      tor_assert_nonfatal_unreached();
    }
  }

  /* We are about to trigger HSDir fetch so send the OK now because after
   * that 650 event(s) are possible so better to have the 250 OK before them
   * to avoid out of order replies. */
  send_control_done(conn);

  /* Trigger the fetch using the built rend query and possibly a list of HS
   * directory to use. This function ignores the client cache thus this will
   * always send a fetch command. */
  if (version == HS_VERSION_THREE) {
    hs_control_hsfetch_command(&v3_pk, hsdirs);
  }

 done:
  /* Contains data pointer that we don't own thus no cleanup. */
  smartlist_free(hsdirs);
  return 0;
}

static const char *hspost_keywords[] = {
  "SERVER", "HSADDRESS", NULL
};
static const control_cmd_syntax_t hspost_syntax = {
  .min_args = 0, .max_args = 0,
  .accept_keywords = true,
  .want_cmddata = true,
  .allowed_keywords = hspost_keywords
};

/** Implementation for the HSPOST command. */
static int
handle_control_hspost(control_connection_t *conn,
                      const control_cmd_args_t *args)
{
  smartlist_t *hs_dirs = NULL;
  const char *encoded_desc = args->cmddata;
  const char *onion_address = NULL;
  const config_line_t *line;

  for (line = args->kwargs; line; line = line->next) {
    if (!strcasecmpstart(line->key, "SERVER")) {
      const char *server = line->value;
      const node_t *node = node_get_by_hex_id(server, 0);

      if (!node || !node->rs) {
        control_printf_endreply(conn, 552, "Server \"%s\" not found",
                                server);
        goto done;
      }
      /* Valid server, add it to our local list. */
      if (!hs_dirs)
        hs_dirs = smartlist_new();
      smartlist_add(hs_dirs, node->rs);
    } else if (!strcasecmpstart(line->key, "HSADDRESS")) {
      const char *address = line->value;
      if (!hs_address_is_valid(address)) {
        control_write_endreply(conn, 512, "Malformed onion address");
        goto done;
      }
      onion_address = address;
    } else {
      tor_assert_nonfatal_unreached();
    }
  }

  /* Handle the v3 case. */
  if (onion_address) {
    if (hs_control_hspost_command(encoded_desc, onion_address, hs_dirs) < 0) {
      control_write_endreply(conn, 554, "Invalid descriptor");
    } else {
      send_control_done(conn);
    }
    goto done;
  }

 done:
  smartlist_free(hs_dirs); /* Contents belong to the rend service code. */
  return 0;
}

/* Helper function for ADD_ONION that adds an ephemeral service depending on
 * the given hs_version.
 *
 * The secret key in pk depends on the hs_version. The ownership of the key
 * used in pk is given to the HS subsystem so the caller must stop accessing
 * it after.
 *
 * The port_cfgs is a list of service port. Ownership transferred to service.
 * The max_streams refers to the MaxStreams= key.
 * The max_streams_close_circuit refers to the MaxStreamsCloseCircuit key.
 * The ownership of that list is transferred to the service.
 *
 * On success (RSAE_OKAY), the address_out points to a newly allocated string
 * containing the onion address without the .onion part. On error, address_out
 * is untouched. */
STATIC hs_service_add_ephemeral_status_t
add_onion_helper_add_service(int hs_version,
                             add_onion_secret_key_t *pk,
                             smartlist_t *port_cfgs, int max_streams,
                             int max_streams_close_circuit,
                             smartlist_t *auth_clients_v3, char **address_out)
{
  hs_service_add_ephemeral_status_t ret;

  tor_assert(pk);
  tor_assert(port_cfgs);
  tor_assert(address_out);

  switch (hs_version) {
  case HS_VERSION_THREE:
    ret = hs_service_add_ephemeral(pk->v3, port_cfgs, max_streams,
                                   max_streams_close_circuit,
                                   auth_clients_v3, address_out);
    break;
  default:
    tor_assert_unreached();
  }

  return ret;
}

/** The list of onion services that have been added via ADD_ONION that do not
 * belong to any particular control connection.
 */
static smartlist_t *detached_onion_services = NULL;

/**
 * Return a list of detached onion services, or NULL if none exist.
 **/
smartlist_t *
get_detached_onion_services(void)
{
  return detached_onion_services;
}

static const char *add_onion_keywords[] = {
   "Port", "Flags", "MaxStreams", "ClientAuth", "ClientAuthV3", NULL
};
static const control_cmd_syntax_t add_onion_syntax = {
  .min_args = 1, .max_args = 1,
  .accept_keywords = true,
  .allowed_keywords = add_onion_keywords
};

/** Called when we get a ADD_ONION command; parse the body, and set up
 * the new ephemeral Onion Service. */
static int
handle_control_add_onion(control_connection_t *conn,
                         const control_cmd_args_t *args)
{
  /* Parse all of the arguments that do not involve handling cryptographic
   * material first, since there's no reason to touch that at all if any of
   * the other arguments are malformed.
   */
  rend_auth_type_t auth_type = REND_NO_AUTH;
  smartlist_t *port_cfgs = smartlist_new();
  smartlist_t *auth_clients_v3 = NULL;
  smartlist_t *auth_clients_v3_str = NULL;
  int discard_pk = 0;
  int detach = 0;
  int max_streams = 0;
  int max_streams_close_circuit = 0;
  int non_anonymous = 0;
  const config_line_t *arg;

  for (arg = args->kwargs; arg; arg = arg->next) {
    if (!strcasecmp(arg->key, "Port")) {
      /* "Port=VIRTPORT[,TARGET]". */
      hs_port_config_t *cfg = hs_parse_port_config(arg->value, ",", NULL);
      if (!cfg) {
        control_write_endreply(conn, 512, "Invalid VIRTPORT/TARGET");
        goto out;
      }
      smartlist_add(port_cfgs, cfg);
    } else if (!strcasecmp(arg->key, "MaxStreams")) {
      /* "MaxStreams=[0..65535]". */
      int ok = 0;
      max_streams = (int)tor_parse_long(arg->value, 10, 0, 65535, &ok, NULL);
      if (!ok) {
        control_write_endreply(conn, 512, "Invalid MaxStreams");
        goto out;
      }
    } else if (!strcasecmp(arg->key, "Flags")) {
      /* "Flags=Flag[,Flag]", where Flag can be:
       *   * 'DiscardPK' - If tor generates the keypair, do not include it in
       *                   the response.
       *   * 'Detach' - Do not tie this onion service to any particular control
       *                connection.
       *   * 'MaxStreamsCloseCircuit' - Close the circuit if MaxStreams is
       *                                exceeded.
       *   * 'BasicAuth' - Client authorization using the 'basic' method.
       *   * 'NonAnonymous' - Add a non-anonymous Single Onion Service. If this
       *                      flag is present, tor must be in non-anonymous
       *                      hidden service mode. If this flag is absent,
       *                      tor must be in anonymous hidden service mode.
       */
      static const char *discard_flag = "DiscardPK";
      static const char *detach_flag = "Detach";
      static const char *max_s_close_flag = "MaxStreamsCloseCircuit";
      static const char *v3auth_flag = "V3Auth";
      static const char *non_anonymous_flag = "NonAnonymous";

      smartlist_t *flags = smartlist_new();
      int bad = 0;

      smartlist_split_string(flags, arg->value, ",", SPLIT_IGNORE_BLANK, 0);
      if (smartlist_len(flags) < 1) {
        control_write_endreply(conn, 512, "Invalid 'Flags' argument");
        bad = 1;
      }
      SMARTLIST_FOREACH_BEGIN(flags, const char *, flag)
      {
        if (!strcasecmp(flag, discard_flag)) {
          discard_pk = 1;
        } else if (!strcasecmp(flag, detach_flag)) {
          detach = 1;
        } else if (!strcasecmp(flag, max_s_close_flag)) {
          max_streams_close_circuit = 1;
        } else if (!strcasecmp(flag, v3auth_flag)) {
          auth_type = REND_V3_AUTH;
        } else if (!strcasecmp(flag, non_anonymous_flag)) {
          non_anonymous = 1;
        } else {
          control_printf_endreply(conn, 512, "Invalid 'Flags' argument: %s",
                                  escaped(flag));
          bad = 1;
          break;
        }
      } SMARTLIST_FOREACH_END(flag);
      SMARTLIST_FOREACH(flags, char *, cp, tor_free(cp));
      smartlist_free(flags);
      if (bad)
        goto out;
    } else if (!strcasecmp(arg->key, "ClientAuthV3")) {
      hs_service_authorized_client_t *client_v3 =
                             parse_authorized_client_key(arg->value, LOG_INFO);
      if (!client_v3) {
        control_write_endreply(conn, 512, "Cannot decode v3 client auth key");
        goto out;
      }

      if (auth_clients_v3 == NULL) {
        auth_clients_v3 = smartlist_new();
        auth_clients_v3_str = smartlist_new();
      }

      smartlist_add(auth_clients_v3, client_v3);
      smartlist_add(auth_clients_v3_str, tor_strdup(arg->value));
    } else {
      tor_assert_nonfatal_unreached();
      goto out;
    }
  }
  if (smartlist_len(port_cfgs) == 0) {
    control_write_endreply(conn, 512, "Missing 'Port' argument");
    goto out;
  } else if (auth_type == REND_NO_AUTH && auth_clients_v3 != NULL) {
    control_write_endreply(conn, 512, "No auth type specified");
    goto out;
  } else if (auth_type != REND_NO_AUTH && auth_clients_v3 == NULL) {
    control_write_endreply(conn, 512, "No auth clients specified");
    goto out;
  } else if (non_anonymous != hs_service_non_anonymous_mode_enabled(
                                                            get_options())) {
    /* If we failed, and the non-anonymous flag is set, Tor must be in
     * anonymous hidden service mode.
     * The error message changes based on the current Tor config:
     * 512 Tor is in anonymous hidden service mode
     * 512 Tor is in non-anonymous hidden service mode
     * (I've deliberately written them out in full here to aid searchability.)
     */
    control_printf_endreply(conn, 512,
                            "Tor is in %sanonymous hidden service " "mode",
                            non_anonymous ? "" : "non-");
    goto out;
  }

  /* Parse the "keytype:keyblob" argument. */
  int hs_version = 0;
  add_onion_secret_key_t pk = { NULL };
  const char *key_new_alg = NULL;
  char *key_new_blob = NULL;

  const char *onionkey = smartlist_get(args->args, 0);
  if (add_onion_helper_keyarg(onionkey, discard_pk,
                              &key_new_alg, &key_new_blob, &pk, &hs_version,
                              conn) < 0) {
    goto out;
  }

  /* Create the HS, using private key pk and port config port_cfg.
   * hs_service_add_ephemeral() will take ownership of pk and port_cfg,
   * regardless of success/failure. */
  char *service_id = NULL;
  int ret = add_onion_helper_add_service(hs_version, &pk, port_cfgs,
                                         max_streams,
                                         max_streams_close_circuit,
                                         auth_clients_v3, &service_id);
  port_cfgs = NULL; /* port_cfgs is now owned by the hs_service code. */
  auth_clients_v3 = NULL; /* so is auth_clients_v3 */
  switch (ret) {
  case RSAE_OKAY:
  {
    if (detach) {
      if (!detached_onion_services)
        detached_onion_services = smartlist_new();
      smartlist_add(detached_onion_services, service_id);
    } else {
      if (!conn->ephemeral_onion_services)
        conn->ephemeral_onion_services = smartlist_new();
      smartlist_add(conn->ephemeral_onion_services, service_id);
    }

    tor_assert(service_id);
    control_printf_midreply(conn, 250, "ServiceID=%s", service_id);
    if (key_new_alg) {
      tor_assert(key_new_blob);
      control_printf_midreply(conn, 250, "PrivateKey=%s:%s",
                              key_new_alg, key_new_blob);
    }
    if (auth_clients_v3_str) {
      SMARTLIST_FOREACH(auth_clients_v3_str, char *, client_str, {
        control_printf_midreply(conn, 250, "ClientAuthV3=%s", client_str);
      });
    }

    send_control_done(conn);
    break;
  }
  case RSAE_BADPRIVKEY:
    control_write_endreply(conn, 551, "Failed to generate onion address");
    break;
  case RSAE_ADDREXISTS:
    control_write_endreply(conn, 550, "Onion address collision");
    break;
  case RSAE_BADVIRTPORT:
    control_write_endreply(conn, 512, "Invalid VIRTPORT/TARGET");
    break;
  case RSAE_BADAUTH:
    control_write_endreply(conn, 512, "Invalid client authorization");
    break;
  case RSAE_INTERNAL: FALLTHROUGH;
  default:
    control_write_endreply(conn, 551, "Failed to add Onion Service");
  }
  if (key_new_blob) {
    memwipe(key_new_blob, 0, strlen(key_new_blob));
    tor_free(key_new_blob);
  }

 out:
  if (port_cfgs) {
    SMARTLIST_FOREACH(port_cfgs, hs_port_config_t*, p,
                      hs_port_config_free(p));
    smartlist_free(port_cfgs);
  }
  if (auth_clients_v3) {
    SMARTLIST_FOREACH(auth_clients_v3, hs_service_authorized_client_t *, ac,
                      service_authorized_client_free(ac));
    smartlist_free(auth_clients_v3);
  }
  if (auth_clients_v3_str) {
    SMARTLIST_FOREACH(auth_clients_v3_str, char *, client_str,
                      tor_free(client_str));
    smartlist_free(auth_clients_v3_str);
  }

  return 0;
}

/** Helper function to handle parsing the KeyType:KeyBlob argument to the
 * ADD_ONION command. Return a new crypto_pk_t and if a new key was generated
 * and the private key not discarded, the algorithm and serialized private key,
 * or NULL and an optional control protocol error message on failure.  The
 * caller is responsible for freeing the returned key_new_blob.
 *
 * Note: The error messages returned are deliberately vague to avoid echoing
 * key material.
 *
 * Note: conn is only used for writing control replies. For testing
 * purposes, it can be NULL if control_write_reply() is appropriately
 * mocked.
 */
STATIC int
add_onion_helper_keyarg(const char *arg, int discard_pk,
                        const char **key_new_alg_out, char **key_new_blob_out,
                        add_onion_secret_key_t *decoded_key, int *hs_version,
                        control_connection_t *conn)
{
  smartlist_t *key_args = smartlist_new();
  const char *key_new_alg = NULL;
  char *key_new_blob = NULL;
  int ret = -1;

  smartlist_split_string(key_args, arg, ":", SPLIT_IGNORE_BLANK, 0);
  if (smartlist_len(key_args) != 2) {
    control_write_endreply(conn, 512, "Invalid key type/blob");
    goto err;
  }

  /* The format is "KeyType:KeyBlob". */
  static const char *key_type_new = "NEW";
  static const char *key_type_best = "BEST";
  static const char *key_type_ed25519_v3 = "ED25519-V3";

  const char *key_type = smartlist_get(key_args, 0);
  const char *key_blob = smartlist_get(key_args, 1);

  if (!strcasecmp(key_type_ed25519_v3, key_type)) {
    /* parsing of private ed25519 key */
    /* "ED25519-V3:<Base64 Blob>" - Loading a pre-existing ed25519 key. */
    ed25519_secret_key_t *sk = tor_malloc_zero(sizeof(*sk));
    if (base64_decode((char *) sk->seckey, sizeof(sk->seckey), key_blob,
                      strlen(key_blob)) != sizeof(sk->seckey)) {
      tor_free(sk);
      control_write_endreply(conn, 512, "Failed to decode ED25519-V3 key");
      goto err;
    }
    decoded_key->v3 = sk;
    *hs_version = HS_VERSION_THREE;
  } else if (!strcasecmp(key_type_new, key_type)) {
    /* "NEW:<Algorithm>" - Generating a new key, blob as algorithm. */
    if (!strcasecmp(key_type_ed25519_v3, key_blob) ||
        !strcasecmp(key_type_best, key_blob)) {
      /* "ED25519-V3", ed25519 key, also currently "BEST" by default. */
      ed25519_secret_key_t *sk = tor_malloc_zero(sizeof(*sk));
      if (ed25519_secret_key_generate(sk, 1) < 0) {
        tor_free(sk);
        control_printf_endreply(conn, 551, "Failed to generate %s key",
                                key_type_ed25519_v3);
        goto err;
      }
      if (!discard_pk) {
        ssize_t len = base64_encode_size(sizeof(sk->seckey), 0) + 1;
        key_new_blob = tor_malloc_zero(len);
        if (base64_encode(key_new_blob, len, (const char *) sk->seckey,
                          sizeof(sk->seckey), 0) != (len - 1)) {
          tor_free(sk);
          tor_free(key_new_blob);
          control_printf_endreply(conn, 551, "Failed to encode %s key",
                                  key_type_ed25519_v3);
          goto err;
        }
        key_new_alg = key_type_ed25519_v3;
      }
      decoded_key->v3 = sk;
      *hs_version = HS_VERSION_THREE;
    } else {
      control_write_endreply(conn, 513, "Invalid key type");
      goto err;
    }
  } else {
    control_write_endreply(conn, 513, "Invalid key type");
    goto err;
  }

  /* Succeeded in loading or generating a private key. */
  ret = 0;

 err:
  SMARTLIST_FOREACH(key_args, char *, cp, {
    memwipe(cp, 0, strlen(cp));
    tor_free(cp);
  });
  smartlist_free(key_args);

  *key_new_alg_out = key_new_alg;
  *key_new_blob_out = key_new_blob;

  return ret;
}

static const control_cmd_syntax_t del_onion_syntax = {
  .min_args = 1, .max_args = 1,
};

/** Called when we get a DEL_ONION command; parse the body, and remove
 * the existing ephemeral Onion Service. */
static int
handle_control_del_onion(control_connection_t *conn,
                         const control_cmd_args_t *cmd_args)
{
  int hs_version = 0;
  smartlist_t *args = cmd_args->args;
  tor_assert(smartlist_len(args) == 1);

  const char *service_id = smartlist_get(args, 0);
  if (hs_address_is_valid(service_id)) {
    hs_version = HS_VERSION_THREE;
  } else {
    control_write_endreply(conn, 512, "Malformed Onion Service id");
    goto out;
  }

  /* Determine if the onion service belongs to this particular control
   * connection, or if it is in the global list of detached services.  If it
   * is in neither, either the service ID is invalid in some way, or it
   * explicitly belongs to a different control connection, and an error
   * should be returned.
   */
  smartlist_t *services[2] = {
    conn->ephemeral_onion_services,
    detached_onion_services
  };
  smartlist_t *onion_services = NULL;
  int idx = -1;
  for (size_t i = 0; i < ARRAY_LENGTH(services); i++) {
    idx = smartlist_string_pos(services[i], service_id);
    if (idx != -1) {
      onion_services = services[i];
      break;
    }
  }
  if (onion_services == NULL) {
    control_write_endreply(conn, 552, "Unknown Onion Service id");
  } else {
    int ret = -1;
    switch (hs_version) {
    case HS_VERSION_THREE:
      ret = hs_service_del_ephemeral(service_id);
      break;
    default:
      /* The ret value will be -1 thus hitting the warning below. This should
       * never happen because of the check at the start of the function. */
      break;
    }
    if (ret < 0) {
      /* This should *NEVER* fail, since the service is on either the
       * per-control connection list, or the global one.
       */
      log_warn(LD_BUG, "Failed to remove Onion Service %s.",
               escaped(service_id));
      tor_fragile_assert();
    }

    /* Remove/scrub the service_id from the appropriate list. */
    char *cp = smartlist_get(onion_services, idx);
    smartlist_del(onion_services, idx);
    memwipe(cp, 0, strlen(cp));
    tor_free(cp);

    send_control_done(conn);
  }

 out:
  return 0;
}

static const control_cmd_syntax_t obsolete_syntax = {
  .max_args = UINT_MAX
};

/**
 * Called when we get an obsolete command: tell the controller that it is
 * obsolete.
 */
static int
handle_control_obsolete(control_connection_t *conn,
                        const control_cmd_args_t *args)
{
  (void)args;
  char *command = tor_strdup(conn->current_cmd);
  tor_strupper(command);
  control_printf_endreply(conn, 511, "%s is obsolete.", command);
  tor_free(command);
  return 0;
}

/**
 * Function pointer to a handler function for a controller command.
 **/
typedef int (*handler_fn_t) (control_connection_t *conn,
                             const control_cmd_args_t *args);

/**
 * Definition for a controller command.
 */
typedef struct control_cmd_def_t {
  /**
   * The name of the command. If the command is multiline, the name must
   * begin with "+".  This is not case-sensitive. */
  const char *name;
  /**
   * A function to execute the command.
   */
  handler_fn_t handler;
  /**
   * Zero or more CMD_FL_* flags, or'd together.
   */
  unsigned flags;
  /**
   * For parsed command: a syntax description.
   */
  const control_cmd_syntax_t *syntax;
} control_cmd_def_t;

/**
 * Indicates that the command's arguments are sensitive, and should be
 * memwiped after use.
 */
#define CMD_FL_WIPE (1u<<0)

#ifndef COCCI
/** Macro: declare a command with a one-line argument, a given set of flags,
 * and a syntax definition.
 **/
#define ONE_LINE(name, flags)                                   \
  {                                                             \
    (#name),                                                    \
      handle_control_ ##name,                                   \
      flags,                                                    \
      &name##_syntax,                                           \
   }

/**
 * Macro: declare a command with a multi-line argument and a given set of
 * flags.
 **/
#define MULTLINE(name, flags)                                   \
  { ("+"#name),                                                 \
      handle_control_ ##name,                                   \
      flags,                                                    \
      &name##_syntax                                            \
  }

/**
 * Macro: declare an obsolete command. (Obsolete commands give a different
 * error than non-existent ones.)
 **/
#define OBSOLETE(name)                          \
  { #name,                                      \
      handle_control_obsolete,                  \
      0,                                        \
      &obsolete_syntax,                         \
  }
#endif /* !defined(COCCI) */

/**
 * An array defining all the recognized controller commands.
 **/
static const control_cmd_def_t CONTROL_COMMANDS[] =
{
  ONE_LINE(setconf, 0),
  ONE_LINE(resetconf, 0),
  ONE_LINE(getconf, 0),
  MULTLINE(loadconf, 0),
  ONE_LINE(setevents, 0),
  ONE_LINE(authenticate, CMD_FL_WIPE),
  ONE_LINE(saveconf, 0),
  ONE_LINE(signal, 0),
  ONE_LINE(takeownership, 0),
  ONE_LINE(dropownership, 0),
  ONE_LINE(mapaddress, 0),
  ONE_LINE(getinfo, 0),
  ONE_LINE(extendcircuit, 0),
  ONE_LINE(setcircuitpurpose, 0),
  OBSOLETE(setrouterpurpose),
  ONE_LINE(attachstream, 0),
  MULTLINE(postdescriptor, 0),
  ONE_LINE(redirectstream, 0),
  ONE_LINE(closestream, 0),
  ONE_LINE(closecircuit, 0),
  ONE_LINE(usefeature, 0),
  ONE_LINE(resolve, 0),
  ONE_LINE(protocolinfo, 0),
  ONE_LINE(authchallenge, CMD_FL_WIPE),
  ONE_LINE(dropguards, 0),
  ONE_LINE(droptimeouts, 0),
  ONE_LINE(hsfetch, 0),
  MULTLINE(hspost, 0),
  ONE_LINE(add_onion, CMD_FL_WIPE),
  ONE_LINE(del_onion, CMD_FL_WIPE),
  ONE_LINE(onion_client_auth_add, CMD_FL_WIPE),
  ONE_LINE(onion_client_auth_remove, 0),
  ONE_LINE(onion_client_auth_view, 0),
};

/**
 * The number of entries in CONTROL_COMMANDS.
 **/
static const size_t N_CONTROL_COMMANDS = ARRAY_LENGTH(CONTROL_COMMANDS);

/**
 * Run a single control command, as defined by a control_cmd_def_t,
 * with a given set of arguments.
 */
static int
handle_single_control_command(const control_cmd_def_t *def,
                              control_connection_t *conn,
                              uint32_t cmd_data_len,
                              char *args)
{
  int rv = 0;

  control_cmd_args_t *parsed_args;
  char *err=NULL;
  tor_assert(def->syntax);
  parsed_args = control_cmd_parse_args(conn->current_cmd,
                                       def->syntax,
                                       cmd_data_len, args,
                                       &err);
  if (!parsed_args) {
    control_printf_endreply(conn, 512, "Bad arguments to %s: %s",
                            conn->current_cmd, err?err:"");
    tor_free(err);
  } else {
    if (BUG(err))
      tor_free(err);
    if (def->handler(conn, parsed_args))
      rv = 0;

    if (def->flags & CMD_FL_WIPE)
      control_cmd_args_wipe(parsed_args);

    control_cmd_args_free(parsed_args);
  }

  if (def->flags & CMD_FL_WIPE)
    memwipe(args, 0, cmd_data_len);

  return rv;
}

/**
 * Run a given controller command, as selected by the current_cmd field of
 * <b>conn</b>.
 */
int
handle_control_command(control_connection_t *conn,
                       uint32_t cmd_data_len,
                       char *args)
{
  tor_assert(conn);
  tor_assert(args);
  tor_assert(args[cmd_data_len] == '\0');

  for (unsigned i = 0; i < N_CONTROL_COMMANDS; ++i) {
    const control_cmd_def_t *def = &CONTROL_COMMANDS[i];
    if (!strcasecmp(conn->current_cmd, def->name)) {
      return handle_single_control_command(def, conn, cmd_data_len, args);
    }
  }

  control_printf_endreply(conn, 510, "Unrecognized command \"%s\"",
                          conn->current_cmd);

  return 0;
}

void
control_cmd_free_all(void)
{
  if (detached_onion_services) { /* Free the detached onion services */
    SMARTLIST_FOREACH(detached_onion_services, char *, cp, tor_free(cp));
    smartlist_free(detached_onion_services);
  }
}
