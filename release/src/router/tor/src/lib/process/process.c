/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file process.c
 * \brief Module for working with other processes.
 **/

#define PROCESS_PRIVATE
#include "lib/buf/buffers.h"
#include "lib/net/buffers_net.h"
#include "lib/container/smartlist.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/process/process.h"
#include "lib/process/process_unix.h"
#include "lib/process/process_win32.h"
#include "lib/process/env.h"

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

/** A list of all <b>process_t</b> instances currently allocated. */
static smartlist_t *processes;

/**
 * Boolean.  If true, then Tor may call execve or CreateProcess via
 * tor_spawn_background.
 **/
static int may_spawn_background_process = 1;

/** Structure to represent a child process. */
struct process_t {
  /** Process status. */
  process_status_t status;

  /** Which protocol is the process using? */
  process_protocol_t protocol;

  /** Which function to call when we have data ready from stdout? */
  process_read_callback_t stdout_read_callback;

  /** Which function to call when we have data ready from stderr? */
  process_read_callback_t stderr_read_callback;

  /** Which function call when our process terminated? */
  process_exit_callback_t exit_callback;

  /** Our exit code when the process have terminated. */
  process_exit_code_t exit_code;

  /** Name of the command we want to execute (for example: /bin/ls). */
  char *command;

  /** The arguments used for the new process. The format here is one argument
   * per element of the smartlist_t. On Windows these arguments are combined
   * together using the <b>tor_join_win_cmdline</b> function. On Unix the
   * process name (argv[0]) and the trailing NULL is added automatically before
   * the process is executed. */
  smartlist_t *arguments;

  /** The environment used for the new process. */
  smartlist_t *environment;

  /** Buffer to store data from stdout when it is read. */
  buf_t *stdout_buffer;

  /** Buffer to store data from stderr when it is read. */
  buf_t *stderr_buffer;

  /** Buffer to store data to stdin before it is written. */
  buf_t *stdin_buffer;

  /** Do we need to store some custom data with the process? */
  void *data;

#ifndef _WIN32
  /** Our Unix process handle. */
  process_unix_t *unix_process;
#else
  /** Our Win32 process handle. */
  process_win32_t *win32_process;
#endif /* !defined(_WIN32) */
};

/** Convert a given process status in <b>status</b> to its string
 * representation. */
const char *
process_status_to_string(process_status_t status)
{
  switch (status) {
  case PROCESS_STATUS_NOT_RUNNING:
    return "not running";
  case PROCESS_STATUS_RUNNING:
    return "running";
  case PROCESS_STATUS_ERROR:
    return "error";
  }

  /* LCOV_EXCL_START */
  tor_assert_unreached();
  return NULL;
  /* LCOV_EXCL_STOP */
}

/** Convert a given process protocol in <b>protocol</b> to its string
 * representation. */
const char *
process_protocol_to_string(process_protocol_t protocol)
{
  switch (protocol) {
  case PROCESS_PROTOCOL_LINE:
    return "Line";
  case PROCESS_PROTOCOL_RAW:
    return "Raw";
  }

  /* LCOV_EXCL_START */
  tor_assert_unreached();
  return NULL;
  /* LCOV_EXCL_STOP */
}

/**
 * Turn off may_spawn_background_process, so that all future calls to
 * tor_spawn_background are guaranteed to fail.
 **/
void
tor_disable_spawning_background_processes(void)
{
  may_spawn_background_process = 0;
}

/** Initialize the Process subsystem.  This function initializes the Process
 * subsystem's global state. For cleaning up, <b>process_free_all()</b> should
 * be called. */
void
process_init(void)
{
  processes = smartlist_new();

#ifdef _WIN32
  process_win32_init();
#endif
}

/** Free up all resources that is handled by the Process subsystem. Note that
 * this call does not terminate already running processes. */
void
process_free_all(void)
{
#ifdef _WIN32
  process_win32_deinit();
#endif

  SMARTLIST_FOREACH(processes, process_t *, x, process_free(x));
  smartlist_free(processes);
}

/** Get a list of all processes.  This function returns a smartlist of
 * <b>process_t</b> containing all the currently allocated processes. */
const smartlist_t *
process_get_all_processes(void)
{
  return processes;
}

/** Allocate and initialize a new process.  This function returns a newly
 * allocated and initialized process data, which can be used to configure and
 * later run a subprocess of Tor. Use the various <b>process_set_*()</b>
 * methods to configure it and run the process using <b>process_exec()</b>. Use
 * <b>command</b> to specify the path to the command to run. You can either
 * specify an absolute path to the command or relative where Tor will use the
 * underlying operating system's functionality for finding the command to run.
 * */
process_t *
process_new(const char *command)
{
  tor_assert(command);

  process_t *process;
  process = tor_malloc_zero(sizeof(process_t));

  /* Set our command. */
  process->command = tor_strdup(command);

  /* By default we are not running. */
  process->status = PROCESS_STATUS_NOT_RUNNING;

  /* Prepare process environment. */
  process->arguments = smartlist_new();
  process->environment = smartlist_new();

  /* Prepare the buffers. */
  process->stdout_buffer = buf_new();
  process->stderr_buffer = buf_new();
  process->stdin_buffer = buf_new();

#ifndef _WIN32
  /* Prepare our Unix process handle. */
  process->unix_process = process_unix_new();
#else
  /* Prepare our Win32 process handle. */
  process->win32_process = process_win32_new();
#endif /* !defined(_WIN32) */

  smartlist_add(processes, process);

  return process;
}

/** Deallocate the given process in <b>process</b>. */
void
process_free_(process_t *process)
{
  if (! process)
    return;

  /* Cleanup parameters. */
  tor_free(process->command);

  /* Cleanup arguments and environment. */
  SMARTLIST_FOREACH(process->arguments, char *, x, tor_free(x));
  smartlist_free(process->arguments);

  SMARTLIST_FOREACH(process->environment, char *, x, tor_free(x));
  smartlist_free(process->environment);

  /* Cleanup the buffers. */
  buf_free(process->stdout_buffer);
  buf_free(process->stderr_buffer);
  buf_free(process->stdin_buffer);

#ifndef _WIN32
  /* Cleanup our Unix process handle. */
  process_unix_free(process->unix_process);
#else
  /* Cleanup our Win32 process handle. */
  process_win32_free(process->win32_process);
#endif /* !defined(_WIN32) */

  smartlist_remove(processes, process);

  tor_free(process);
}

/** Execute the given process.  This function executes the given process as a
 * subprocess of Tor. Returns <b>PROCESS_STATUS_RUNNING</b> upon success. */
process_status_t
process_exec(process_t *process)
{
  tor_assert(process);

  if (BUG(may_spawn_background_process == 0))
    return PROCESS_STATUS_ERROR;

  process_status_t status = PROCESS_STATUS_NOT_RUNNING;

  log_info(LD_PROCESS, "Starting new process: %s", process->command);

#ifndef _WIN32
  status = process_unix_exec(process);
#else
  status = process_win32_exec(process);
#endif

  /* Update our state. */
  process_set_status(process, status);

  if (status != PROCESS_STATUS_RUNNING) {
    log_warn(LD_PROCESS, "Failed to start process: %s",
             process_get_command(process));
  }

  return status;
}

/** Terminate the given process. Returns true on success,
 * otherwise false. */
bool
process_terminate(process_t *process)
{
  tor_assert(process);

  /* Terminating a non-running process isn't going to work. */
  if (process_get_status(process) != PROCESS_STATUS_RUNNING)
    return false;

  log_debug(LD_PROCESS, "Terminating process");

#ifndef _WIN32
  return process_unix_terminate(process);
#else
  return process_win32_terminate(process);
#endif
}

/** Returns the unique process identifier for the given <b>process</b>. */
process_pid_t
process_get_pid(process_t *process)
{
  tor_assert(process);

#ifndef _WIN32
  return process_unix_get_pid(process);
#else
  return process_win32_get_pid(process);
#endif
}

/** Set the callback function for output from the child process's standard out
 * handle.  This function sets the callback function which is called every time
 * the child process have written output to its standard out file handle.
 *
 * Use <b>process_set_protocol(process, PROCESS_PROTOCOL_LINE)</b> if you want
 * the callback to only contain complete "\n" or "\r\n" terminated lines. */
void
process_set_stdout_read_callback(process_t *process,
                                 process_read_callback_t callback)
{
  tor_assert(process);
  process->stdout_read_callback = callback;
}

/** Set the callback function for output from the child process's standard
 * error handle.  This function sets the callback function which is called
 * every time the child process have written output to its standard error file
 * handle.
 *
 * Use <b>process_set_protocol(process, PROCESS_PROTOCOL_LINE)</b> if you want
 * the callback to only contain complete "\n" or "\r\n" terminated lines. */
void
process_set_stderr_read_callback(process_t *process,
                                 process_read_callback_t callback)
{
  tor_assert(process);
  process->stderr_read_callback = callback;
}

/** Set the callback function for process exit notification.  The
 * <b>callback</b> function will be called every time your child process have
 * terminated. */
void
process_set_exit_callback(process_t *process,
                          process_exit_callback_t callback)
{
  tor_assert(process);
  process->exit_callback = callback;
}

/** Get the current command of the given process. */
const char *
process_get_command(const process_t *process)
{
  tor_assert(process);
  return process->command;
}

void
process_set_protocol(process_t *process, process_protocol_t protocol)
{
  tor_assert(process);
  process->protocol = protocol;
}

/** Get the currently used protocol of the given process. */
process_protocol_t
process_get_protocol(const process_t *process)
{
  tor_assert(process);
  return process->protocol;
}

/** Set opaque pointer to data.  This function allows you to store a pointer to
 * your own data in the given process. Use <b>process_get_data()</b> in the
 * various callback functions to retrieve the data again.
 *
 * Note that the given process does NOT take ownership of the data and you are
 * responsible for freeing up any resources allocated by the given data.
 * */
void
process_set_data(process_t *process, void *data)
{
  tor_assert(process);
  process->data = data;
}

/** Get the opaque pointer to callback data from the given process.  This
 * function allows you get the data you stored with <b>process_set_data()</b>
 * in the different callback functions. */
void *
process_get_data(const process_t *process)
{
  tor_assert(process);
  return process->data;
}

/** Set the status of a given process. */
void
process_set_status(process_t *process, process_status_t status)
{
  tor_assert(process);
  process->status = status;
}

/** Get the status of the given process. */
process_status_t
process_get_status(const process_t *process)
{
  tor_assert(process);
  return process->status;
}

/** Append an argument to the list of arguments in the given process. */
void
process_append_argument(process_t *process, const char *argument)
{
  tor_assert(process);
  tor_assert(argument);

  smartlist_add(process->arguments, tor_strdup(argument));
}

/** Returns a list of arguments (excluding the command itself) from the
 * given process. */
const smartlist_t *
process_get_arguments(const process_t *process)
{
  tor_assert(process);
  return process->arguments;
}

/** Returns a newly allocated Unix style argument vector. Use <b>tor_free()</b>
 * to deallocate it after use. */
char **
process_get_argv(const process_t *process)
{
  tor_assert(process);

  /** Generate a Unix style process argument vector from our process's
   * arguments smartlist_t. */
  char **argv = NULL;

  char *filename = process->command;
  const smartlist_t *arguments = process->arguments;
  const size_t size = smartlist_len(arguments);

  /* Make space for the process filename as argv[0] and a trailing NULL. */
  argv = tor_malloc_zero(sizeof(char *) * (size + 2));

  /* Set our filename as first argument. */
  argv[0] = filename;

  /* Put in the rest of the values from arguments. */
  SMARTLIST_FOREACH_BEGIN(arguments, char *, arg_val) {
    tor_assert(arg_val != NULL);

    argv[arg_val_sl_idx + 1] = arg_val;
  } SMARTLIST_FOREACH_END(arg_val);

  return argv;
}

/** This function clears the internal environment and copies over every string
 * from <b>env</b> as the new environment. */
void
process_reset_environment(process_t *process, const smartlist_t *env)
{
  tor_assert(process);
  tor_assert(env);

  /* Cleanup old environment. */
  SMARTLIST_FOREACH(process->environment, char *, x, tor_free(x));
  smartlist_free(process->environment);
  process->environment = smartlist_new();

  SMARTLIST_FOREACH(env, char *, x,
                    smartlist_add(process->environment, tor_strdup(x)));
}

/** Set the given <b>key</b>/<b>value</b> pair as environment variable in the
 * given process. */
void
process_set_environment(process_t *process,
                        const char *key,
                        const char *value)
{
  tor_assert(process);
  tor_assert(key);
  tor_assert(value);

  smartlist_add_asprintf(process->environment, "%s=%s", key, value);
}

/** Returns a newly allocated <b>process_environment_t</b> containing the
 * environment variables for the given process. */
process_environment_t *
process_get_environment(const process_t *process)
{
  tor_assert(process);
  return process_environment_make(process->environment);
}

#ifndef _WIN32
/** Get the internal handle for the Unix backend. */
process_unix_t *
process_get_unix_process(const process_t *process)
{
  tor_assert(process);
  tor_assert(process->unix_process);
  return process->unix_process;
}
#else /* defined(_WIN32) */
/** Get the internal handle for Windows backend. */
process_win32_t *
process_get_win32_process(const process_t *process)
{
  tor_assert(process);
  tor_assert(process->win32_process);
  return process->win32_process;
}
#endif /* !defined(_WIN32) */

/** Write <b>size</b> bytes of <b>data</b> to the given process's standard
 * input. */
void
process_write(process_t *process,
              const uint8_t *data, size_t size)
{
  tor_assert(process);
  tor_assert(data);

  buf_add(process->stdin_buffer, (char *)data, size);
  process_write_stdin(process, process->stdin_buffer);
}

/** As tor_vsnprintf(), but write the data to the given process's standard
 * input. */
void
process_vprintf(process_t *process,
                const char *format, va_list args)
{
  tor_assert(process);
  tor_assert(format);

  int size;
  char *data;

  size = tor_vasprintf(&data, format, args);
  tor_assert(data != NULL);
  process_write(process, (uint8_t *)data, size);
  tor_free(data);
}

/** As tor_snprintf(), but write the data to the given process's standard
 * input. */
void
process_printf(process_t *process,
               const char *format, ...)
{
  tor_assert(process);
  tor_assert(format);

  va_list ap;
  va_start(ap, format);
  process_vprintf(process, format, ap);
  va_end(ap);
}

/** This function is called by the Process backend when a given process have
 * data that is ready to be read from the child process's standard output
 * handle. */
void
process_notify_event_stdout(process_t *process)
{
  tor_assert(process);

  int ret;
  ret = process_read_stdout(process, process->stdout_buffer);

  if (ret > 0)
    process_read_data(process,
                      process->stdout_buffer,
                      process->stdout_read_callback);
}

/** This function is called by the Process backend when a given process have
 * data that is ready to be read from the child process's standard error
 * handle. */
void
process_notify_event_stderr(process_t *process)
{
  tor_assert(process);

  int ret;
  ret = process_read_stderr(process, process->stderr_buffer);

  if (ret > 0)
    process_read_data(process,
                      process->stderr_buffer,
                      process->stderr_read_callback);
}

/** This function is called by the Process backend when a given process is
 * allowed to begin writing data to the standard input of the child process. */
void
process_notify_event_stdin(process_t *process)
{
  tor_assert(process);

  process_write_stdin(process, process->stdin_buffer);
}

/** This function is called by the Process backend when a given process have
 * terminated. The exit status code is passed in <b>exit_code</b>. We mark the
 * process as no longer running and calls the <b>exit_callback</b> with
 * information about the process termination. The given <b>process</b> is
 * free'd iff the exit_callback returns true. */
void
process_notify_event_exit(process_t *process, process_exit_code_t exit_code)
{
  tor_assert(process);

  log_debug(LD_PROCESS,
            "Process terminated with exit code: %"PRIu64, exit_code);

  /* Update our state. */
  process_set_status(process, PROCESS_STATUS_NOT_RUNNING);
  process->exit_code = exit_code;

  /* Call our exit callback, if it exists. */
  bool free_process_handle = false;

  /* The exit callback will tell us if we should process_free() our handle. */
  if (process->exit_callback)
    free_process_handle = process->exit_callback(process, exit_code);

  if (free_process_handle)
    process_free(process);
}

/** This function is called whenever the Process backend have notified us that
 * there is data to be read from its standard out handle. Returns the number of
 * bytes that have been put into the given buffer. */
MOCK_IMPL(STATIC int, process_read_stdout, (process_t *process, buf_t *buffer))
{
  tor_assert(process);
  tor_assert(buffer);

#ifndef _WIN32
  return process_unix_read_stdout(process, buffer);
#else
  return process_win32_read_stdout(process, buffer);
#endif
}

/** This function is called whenever the Process backend have notified us that
 * there is data to be read from its standard error handle. Returns the number
 * of bytes that have been put into the given buffer. */
MOCK_IMPL(STATIC int, process_read_stderr, (process_t *process, buf_t *buffer))
{
  tor_assert(process);
  tor_assert(buffer);

#ifndef _WIN32
  return process_unix_read_stderr(process, buffer);
#else
  return process_win32_read_stderr(process, buffer);
#endif
}

/** This function calls the backend function for the given process whenever
 * there is data to be written to the backends' file handles. */
MOCK_IMPL(STATIC void, process_write_stdin,
          (process_t *process, buf_t *buffer))
{
  tor_assert(process);
  tor_assert(buffer);

#ifndef _WIN32
  process_unix_write(process, buffer);
#else
  process_win32_write(process, buffer);
#endif
}

/** This function calls the protocol handlers based on the value of
 * <b>process_get_protocol(process)</b>. Currently we call
 * <b>process_read_buffer()</b> for <b>PROCESS_PROTOCOL_RAW</b> and
 * <b>process_read_lines()</b> for <b>PROCESS_PROTOCOL_LINE</b>. */
STATIC void
process_read_data(process_t *process,
                  buf_t *buffer,
                  process_read_callback_t callback)
{
  tor_assert(process);
  tor_assert(buffer);

  switch (process_get_protocol(process)) {
  case PROCESS_PROTOCOL_RAW:
    process_read_buffer(process, buffer, callback);
    break;
  case PROCESS_PROTOCOL_LINE:
    process_read_lines(process, buffer, callback);
    break;
  default:
    /* LCOV_EXCL_START */
    tor_assert_unreached();
    return;
    /* LCOV_EXCL_STOP */
  }
}

/** This function takes the content of the given <b>buffer</b> and passes it to
 * the given <b>callback</b> function, but ensures that an additional zero byte
 * is added to the end of the data such that the given callback implementation
 * can threat the content as a ASCIIZ string. */
STATIC void
process_read_buffer(process_t *process,
                    buf_t *buffer,
                    process_read_callback_t callback)
{
  tor_assert(process);
  tor_assert(buffer);

  const size_t size = buf_datalen(buffer);

  /* We allocate an extra byte for the zero byte in the end. */
  char *data = tor_malloc_zero(size + 1);

  buf_get_bytes(buffer, data, size);
  log_debug(LD_PROCESS, "Read data from process");

  if (callback)
    callback(process, data, size);

  tor_free(data);
}

/** This function tries to extract complete lines from the given <b>buffer</b>
 * and calls the given <b>callback</b> function whenever it has a complete
 * line. Before calling <b>callback</b> we remove the trailing "\n" or "\r\n"
 * from the line. If we are unable to extract a complete line we leave the data
 * in the buffer for next call. */
STATIC void
process_read_lines(process_t *process,
                   buf_t *buffer,
                   process_read_callback_t callback)
{
  tor_assert(process);
  tor_assert(buffer);

  const size_t size = buf_datalen(buffer) + 1;
  size_t line_size = 0;
  char *data = tor_malloc_zero(size);
  int ret;

  while (true) {
    line_size = size;
    ret = buf_get_line(buffer, data, &line_size);

    /* A complete line should always be smaller than the size of our
     * buffer. */
    tor_assert(ret != -1);

    /* Remove \n from the end of the line. */
    if (line_size >= 1 && data[line_size - 1] == '\n') {
      data[line_size - 1] = '\0';
      --line_size;
    }

    /* Remove \r from the end of the line. */
    if (line_size >= 1 && data[line_size - 1] == '\r') {
      data[line_size - 1] = '\0';
      --line_size;
    }

    if (ret == 1) {
      log_debug(LD_PROCESS, "Read line from process: \"%s\"", data);

      if (callback)
        callback(process, data, line_size);

      /* We have read a whole line, let's see if there is more lines to read.
       * */
      continue;
    }

    /* No complete line for us to read. We are done for now. */
    tor_assert_nonfatal(ret == 0);
    break;
  }

  tor_free(data);
}
