/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file process_unix.c
 * \brief Module for working with Unix processes.
 **/

#define PROCESS_UNIX_PRIVATE
#include "lib/intmath/cmp.h"
#include "lib/buf/buffers.h"
#include "lib/net/buffers_net.h"
#include "lib/container/smartlist.h"
#include "lib/evloop/compat_libevent.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/process/process.h"
#include "lib/process/process_unix.h"
#include "lib/process/waitpid.h"
#include "lib/process/env.h"

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if defined(HAVE_SYS_PRCTL_H) && defined(__linux__)
#include <sys/prctl.h>
#endif

#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifndef _WIN32

/** Maximum number of file descriptors, if we cannot get it via sysconf() */
#define DEFAULT_MAX_FD 256

/** Internal state for Unix handles. */
struct process_unix_handle_t {
  /** Unix File Descriptor. */
  int fd;

  /** Have we reached end of file? */
  bool reached_eof;

  /** Event structure for libevent. */
  struct event *event;

  /** Are we writing? */
  bool is_writing;
};

/** Internal state for our Unix process. */
struct process_unix_t {
  /** Standard in handle. */
  process_unix_handle_t stdin_handle;

  /** Standard out handle. */
  process_unix_handle_t stdout_handle;

  /** Standard error handle. */
  process_unix_handle_t stderr_handle;

  /** The process identifier of our process. */
  pid_t pid;

  /** Waitpid Callback structure. */
  waitpid_callback_t *waitpid;
};

/** Returns a newly allocated <b>process_unix_t</b>. */
process_unix_t *
process_unix_new(void)
{
  process_unix_t *unix_process;
  unix_process = tor_malloc_zero(sizeof(process_unix_t));

  unix_process->stdin_handle.fd = -1;
  unix_process->stderr_handle.fd = -1;
  unix_process->stdout_handle.fd = -1;

  return unix_process;
}

/** Deallocates the given <b>unix_process</b>. */
void
process_unix_free_(process_unix_t *unix_process)
{
  if (! unix_process)
    return;

  /* Clean up our waitpid callback. */
  clear_waitpid_callback(unix_process->waitpid);

  /* FIXME(ahf): Refactor waitpid code? */
  unix_process->waitpid = NULL;

  /* Close all our file descriptors. */
  process_unix_close_file_descriptors(unix_process);

  tor_event_free(unix_process->stdout_handle.event);
  tor_event_free(unix_process->stderr_handle.event);
  tor_event_free(unix_process->stdin_handle.event);

  tor_free(unix_process);
}

/** Executes the given process as a child process of Tor.  This function is
 * responsible for setting up the child process and run it. This includes
 * setting up pipes for interprocess communication, initialize the waitpid
 * callbacks, and finally run fork() followed by execve(). Returns
 * <b>PROCESS_STATUS_RUNNING</b> upon success. */
process_status_t
process_unix_exec(process_t *process)
{
  static int max_fd = -1;

  process_unix_t *unix_process;
  pid_t pid;
  int stdin_pipe[2];
  int stdout_pipe[2];
  int stderr_pipe[2];
  int retval, fd;

  unix_process = process_get_unix_process(process);

  /* Create standard in pipe. */
  retval = pipe(stdin_pipe);

  if (-1 == retval) {
    log_warn(LD_PROCESS,
             "Unable to create pipe for stdin "
             "communication with process: %s",
             strerror(errno));

    return PROCESS_STATUS_ERROR;
  }

  /* Create standard out pipe. */
  retval = pipe(stdout_pipe);

  if (-1 == retval) {
    log_warn(LD_PROCESS,
             "Unable to create pipe for stdout "
             "communication with process: %s",
             strerror(errno));

    /** Cleanup standard in pipe. */
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);

    return PROCESS_STATUS_ERROR;
  }

  /* Create standard error pipe. */
  retval = pipe(stderr_pipe);

  if (-1 == retval) {
    log_warn(LD_PROCESS,
             "Unable to create pipe for stderr "
             "communication with process: %s",
             strerror(errno));

    /** Cleanup standard in pipe. */
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);

    /** Cleanup standard out pipe. */
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);

    return PROCESS_STATUS_ERROR;
  }

#ifdef _SC_OPEN_MAX
  if (-1 == max_fd) {
    max_fd = (int)sysconf(_SC_OPEN_MAX);

    if (max_fd == -1) {
      max_fd = DEFAULT_MAX_FD;
      log_warn(LD_PROCESS,
               "Cannot find maximum file descriptor, assuming: %d", max_fd);
    }
  }
#else /* !defined(_SC_OPEN_MAX) */
  max_fd = DEFAULT_MAX_FD;
#endif /* defined(_SC_OPEN_MAX) */

  pid = fork();

  if (0 == pid) {
    /* This code is running in the child process context. */

#if defined(HAVE_SYS_PRCTL_H) && defined(__linux__)
    /* Attempt to have the kernel issue a SIGTERM if the parent
     * goes away. Certain attributes of the binary being execve()ed
     * will clear this during the execve() call, but it's better
     * than nothing.
     */
    prctl(PR_SET_PDEATHSIG, SIGTERM);
#endif /* defined(HAVE_SYS_PRCTL_H) && defined(__linux__) */

    /* Link process stdout to the write end of the pipe. */
    retval = dup2(stdout_pipe[1], STDOUT_FILENO);
    if (-1 == retval)
      goto error;

    /* Link process stderr to the write end of the pipe. */
    retval = dup2(stderr_pipe[1], STDERR_FILENO);
    if (-1 == retval)
      goto error;

    /* Link process stdin to the read end of the pipe */
    retval = dup2(stdin_pipe[0], STDIN_FILENO);
    if (-1 == retval)
      goto error;

    /* Close our pipes now after they have been dup2()'ed. */
    close(stderr_pipe[0]);
    close(stderr_pipe[1]);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);

    /* Close all other fds, including the read end of the pipe.  XXX: We should
     * now be doing enough FD_CLOEXEC setting to make this needless.
     */
    for (fd = STDERR_FILENO + 1; fd < max_fd; fd++)
      close(fd);

    /* Create the argv value for our new process. */
    char **argv = process_get_argv(process);

    /* Create the env value for our new process. */
    process_environment_t *env = process_get_environment(process);

    /* Call the requested program. */
    execve(argv[0], argv, env->unixoid_environment_block);

    /* If we made it here it is because execve failed :-( */
    tor_free(argv);
    process_environment_free(env);

 error:
    fprintf(stderr, "Error from child process: %s", strerror(errno));
    _exit(1);
  }

  /* We are in the parent process. */
  if (-1 == pid) {
    log_warn(LD_PROCESS,
             "Failed to create child process: %s", strerror(errno));

    /** Cleanup standard in pipe. */
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);

    /** Cleanup standard out pipe. */
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);

    /** Cleanup standard error pipe. */
    close(stderr_pipe[0]);
    close(stderr_pipe[1]);

    return PROCESS_STATUS_ERROR;
  }

  /* Register our PID. */
  unix_process->pid = pid;

  /* Setup waitpid callbacks. */
  unix_process->waitpid = set_waitpid_callback(pid,
                                               process_unix_waitpid_callback,
                                               process);

  /* Handle standard out. */
  unix_process->stdout_handle.fd = stdout_pipe[0];
  retval = close(stdout_pipe[1]);

  if (-1 == retval) {
    log_warn(LD_PROCESS, "Failed to close write end of standard out pipe: %s",
             strerror(errno));
  }

  /* Handle standard error. */
  unix_process->stderr_handle.fd = stderr_pipe[0];
  retval = close(stderr_pipe[1]);

  if (-1 == retval) {
    log_warn(LD_PROCESS,
             "Failed to close write end of standard error pipe: %s",
             strerror(errno));
  }

  /* Handle standard in. */
  unix_process->stdin_handle.fd = stdin_pipe[1];
  retval = close(stdin_pipe[0]);

  if (-1 == retval) {
    log_warn(LD_PROCESS, "Failed to close read end of standard in pipe: %s",
             strerror(errno));
  }

  /* Setup our handles. */
  process_unix_setup_handle(process,
                            &unix_process->stdout_handle,
                            EV_READ|EV_PERSIST,
                            stdout_read_callback);

  process_unix_setup_handle(process,
                            &unix_process->stderr_handle,
                            EV_READ|EV_PERSIST,
                            stderr_read_callback);

  process_unix_setup_handle(process,
                            &unix_process->stdin_handle,
                            EV_WRITE|EV_PERSIST,
                            stdin_write_callback);

  /* Start reading from standard out and standard error. */
  process_unix_start_reading(&unix_process->stdout_handle);
  process_unix_start_reading(&unix_process->stderr_handle);

  return PROCESS_STATUS_RUNNING;
}

/** Terminate the given process. Returns true on success, otherwise false. */
bool
process_unix_terminate(process_t *process)
{
  tor_assert(process);

  process_unix_t *unix_process = process_get_unix_process(process);

  /* All running processes should have a waitpid. */
  if (BUG(unix_process->waitpid == NULL))
    return false;

  bool success = true;

  /* Send a SIGTERM to our child process. */
  int ret;

  ret = kill(unix_process->pid, SIGTERM);

  if (ret == -1) {
    log_warn(LD_PROCESS, "Unable to terminate process: %s",
             strerror(errno));
    success = false;
  }

  /* Close all our FD's. */
  if (! process_unix_close_file_descriptors(unix_process))
    success = false;

  return success;
}

/** Returns the unique process identifier for the given <b>process</b>. */
process_pid_t
process_unix_get_pid(process_t *process)
{
  tor_assert(process);

  process_unix_t *unix_process = process_get_unix_process(process);
  return (process_pid_t)unix_process->pid;
}

/** Write the given <b>buffer</b> as input to the given <b>process</b>'s
 * standard input. Returns the number of bytes written. */
int
process_unix_write(process_t *process, buf_t *buffer)
{
  tor_assert(process);
  tor_assert(buffer);

  process_unix_t *unix_process = process_get_unix_process(process);

  size_t buffer_flush_len = buf_datalen(buffer);
  const size_t max_to_write = MIN(PROCESS_MAX_WRITE, buffer_flush_len);

  /* If we have data to write (when buffer_flush_len > 0) and we are not
   * currently getting file descriptor events from the kernel, we tell the
   * kernel to start notifying us about when we can write to our file
   * descriptor and return. */
  if (buffer_flush_len > 0 && ! unix_process->stdin_handle.is_writing) {
    process_unix_start_writing(&unix_process->stdin_handle);
    return 0;
  }

  /* We don't have any data to write, but the kernel is currently notifying us
   * about whether we are able to write or not. Tell the kernel to stop
   * notifying us until we have data to write. */
  if (buffer_flush_len == 0 && unix_process->stdin_handle.is_writing) {
    process_unix_stop_writing(&unix_process->stdin_handle);
    return 0;
  }

  /* We have data to write and the kernel have told us to write it. */
  return buf_flush_to_pipe(buffer,
                           process_get_unix_process(process)->stdin_handle.fd,
                           max_to_write);
}

/** Read data from the given process's standard output and put it into
 * <b>buffer</b>. Returns the number of bytes read. */
int
process_unix_read_stdout(process_t *process, buf_t *buffer)
{
  tor_assert(process);
  tor_assert(buffer);

  process_unix_t *unix_process = process_get_unix_process(process);

  return process_unix_read_handle(process,
                                  &unix_process->stdout_handle,
                                  buffer);
}

/** Read data from the given process's standard error and put it into
 * <b>buffer</b>. Returns the number of bytes read. */
int
process_unix_read_stderr(process_t *process, buf_t *buffer)
{
  tor_assert(process);
  tor_assert(buffer);

  process_unix_t *unix_process = process_get_unix_process(process);

  return process_unix_read_handle(process,
                                  &unix_process->stderr_handle,
                                  buffer);
}

/** This function is called whenever libevent thinks we have data that could be
 * read from the child process's standard output. We notify the Process
 * subsystem, which is then responsible for calling back to us for doing the
 * actual reading of the data. */
STATIC void
stdout_read_callback(evutil_socket_t fd, short event, void *data)
{
  (void)fd;
  (void)event;

  process_t *process = data;
  tor_assert(process);

  process_notify_event_stdout(process);
}

/** This function is called whenever libevent thinks we have data that could be
 * read from the child process's standard error. We notify the Process
 * subsystem, which is then responsible for calling back to us for doing the
 * actual reading of the data. */
STATIC void
stderr_read_callback(evutil_socket_t fd, short event, void *data)
{
  (void)fd;
  (void)event;

  process_t *process = data;
  tor_assert(process);

  process_notify_event_stderr(process);
}

/** This function is called whenever libevent thinks we have data that could be
 * written the child process's standard input. We notify the Process subsystem,
 * which is then responsible for calling back to us for doing the actual write
 * of the data. */
STATIC void
stdin_write_callback(evutil_socket_t fd, short event, void *data)
{
  (void)fd;
  (void)event;

  process_t *process = data;
  tor_assert(process);

  process_notify_event_stdin(process);
}

/** This function tells libevent that we are interested in receiving read
 * events from the given <b>handle</b>. */
STATIC void
process_unix_start_reading(process_unix_handle_t *handle)
{
  tor_assert(handle);

  if (event_add(handle->event, NULL))
    log_warn(LD_PROCESS,
             "Unable to add libevent event for handle.");
}

/** This function tells libevent that we are no longer interested in receiving
 * read events from the given <b>handle</b>. */
STATIC void
process_unix_stop_reading(process_unix_handle_t *handle)
{
  tor_assert(handle);

  if (handle->event == NULL)
    return;

  if (event_del(handle->event))
    log_warn(LD_PROCESS,
             "Unable to delete libevent event for handle.");
}

/** This function tells libevent that we are interested in receiving write
 * events from the given <b>handle</b>. */
STATIC void
process_unix_start_writing(process_unix_handle_t *handle)
{
  tor_assert(handle);

  if (event_add(handle->event, NULL))
    log_warn(LD_PROCESS,
             "Unable to add libevent event for handle.");

  handle->is_writing = true;
}

/** This function tells libevent that we are no longer interested in receiving
 * write events from the given <b>handle</b>. */
STATIC void
process_unix_stop_writing(process_unix_handle_t *handle)
{
  tor_assert(handle);

  if (handle->event == NULL)
    return;

  if (event_del(handle->event))
    log_warn(LD_PROCESS,
             "Unable to delete libevent event for handle.");

  handle->is_writing = false;
}

/** This function is called when the waitpid system have detected that our
 * process have terminated. We disable the waitpid system and notify the
 * Process subsystem that we have terminated. */
STATIC void
process_unix_waitpid_callback(int status, void *data)
{
  tor_assert(data);

  process_t *process = data;
  process_unix_t *unix_process = process_get_unix_process(process);

  /* Remove our waitpid callback. */
  clear_waitpid_callback(unix_process->waitpid);
  unix_process->waitpid = NULL;

  /* Notify our process. */
  process_notify_event_exit(process, status);

  /* Make sure you don't modify the process after we have called
   * process_notify_event_exit() on it, to allow users to process_free() it in
   * the exit callback. */
}

/** This function sets the file descriptor in the <b>handle</b> as non-blocking
 * and configures the libevent event structure based on the given <b>flags</b>
 * to ensure that <b>callback</b> is called whenever we have events on the
 * given <b>handle</b>. */
STATIC void
process_unix_setup_handle(process_t *process,
                          process_unix_handle_t *handle,
                          short flags,
                          event_callback_fn callback)
{
  tor_assert(process);
  tor_assert(handle);
  tor_assert(callback);

  /* Put our file descriptor into non-blocking mode. */
  if (fcntl(handle->fd, F_SETFL, O_NONBLOCK) < 0) {
    log_warn(LD_PROCESS, "Unable mark Unix handle as non-blocking: %s",
             strerror(errno));
  }

  /* Setup libevent event. */
  handle->event = tor_event_new(tor_libevent_get_base(),
                                handle->fd,
                                flags,
                                callback,
                                process);
}

/** This function reads data from the given <b>handle</b> and puts it into
 * <b>buffer</b>. Returns the number of bytes read this way. */
STATIC int
process_unix_read_handle(process_t *process,
                         process_unix_handle_t *handle,
                         buf_t *buffer)
{
  tor_assert(process);
  tor_assert(handle);
  tor_assert(buffer);

  int ret = 0;
  int eof = 0;
  int error = 0;

  ret = buf_read_from_pipe(buffer,
                           handle->fd,
                           PROCESS_MAX_READ,
                           &eof,
                           &error);

  if (error)
    log_warn(LD_PROCESS,
             "Unable to read data: %s", strerror(error));

  if (eof) {
    handle->reached_eof = true;
    process_unix_stop_reading(handle);
  }

  return ret;
}

/** Close the standard in, out, and error handles of the given
 * <b>unix_process</b>. */
STATIC bool
process_unix_close_file_descriptors(process_unix_t *unix_process)
{
  tor_assert(unix_process);

  int ret;
  bool success = true;

  /* Stop reading and writing before we close() our
   * file descriptors. */
  if (! unix_process->stdout_handle.reached_eof)
    process_unix_stop_reading(&unix_process->stdout_handle);

  if (! unix_process->stderr_handle.reached_eof)
    process_unix_stop_reading(&unix_process->stderr_handle);

  if (unix_process->stdin_handle.is_writing)
    process_unix_stop_writing(&unix_process->stdin_handle);

  if (unix_process->stdin_handle.fd != -1) {
    ret = close(unix_process->stdin_handle.fd);
    if (ret == -1) {
      log_warn(LD_PROCESS, "Unable to close standard in");
      success = false;
    }

    unix_process->stdin_handle.fd = -1;
  }

  if (unix_process->stdout_handle.fd != -1) {
    ret = close(unix_process->stdout_handle.fd);
    if (ret == -1) {
      log_warn(LD_PROCESS, "Unable to close standard out");
      success = false;
    }

    unix_process->stdout_handle.fd = -1;
  }

  if (unix_process->stderr_handle.fd != -1) {
    ret = close(unix_process->stderr_handle.fd);
    if (ret == -1) {
      log_warn(LD_PROCESS, "Unable to close standard error");
      success = false;
    }

    unix_process->stderr_handle.fd = -1;
  }

  return success;
}

#endif /* !defined(_WIN32) */
