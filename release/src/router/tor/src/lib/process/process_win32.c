/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file process_win32.c
 * \brief Module for working with Windows processes.
 **/

#define PROCESS_WIN32_PRIVATE
#include "lib/intmath/cmp.h"
#include "lib/buf/buffers.h"
#include "lib/net/buffers_net.h"
#include "lib/container/smartlist.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/log/win32err.h"
#include "lib/process/process.h"
#include "lib/process/process_win32.h"
#include "lib/process/env.h"

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef _WIN32

/** The size of our intermediate buffers. */
#define BUFFER_SIZE (1024)

/** Timer that ticks once a second and calls the process_win32_timer_callback()
 * function. */
static periodic_timer_t *periodic_timer;

/** Structure to represent the state around the pipe HANDLE.
 *
 * This structure is used to store state about a given HANDLE, including
 * whether we have reached end of file, its intermediate buffers, and how much
 * data that is available in the intermediate buffer. */
struct process_win32_handle_t {
  /** Standard out pipe handle. */
  HANDLE pipe;

  /** True iff we have reached EOF from the pipe. */
  bool reached_eof;

  /** How much data is available in buffer. */
  size_t data_available;

  /** Intermediate buffer for ReadFileEx() and WriteFileEx(). */
  char buffer[BUFFER_SIZE];

  /** Overlapped structure for ReadFileEx() and WriteFileEx(). */
  OVERLAPPED overlapped;

  /** Are we waiting for another I/O operation to complete? */
  bool busy;
};

/** Structure to represent the Windows specific implementation details of this
 * Process backend.
 *
 * This structure is attached to <b>process_t</b> (see process.h) and is
 * reachable from <b>process_t</b> via the <b>process_get_win32_process()</b>
 * method. */
struct process_win32_t {
  /** Standard in state. */
  process_win32_handle_t stdin_handle;

  /** Standard out state. */
  process_win32_handle_t stdout_handle;

  /** Standard error state. */
  process_win32_handle_t stderr_handle;

  /** Process Information. */
  PROCESS_INFORMATION process_information;
};

/** Create a new <b>process_win32_t</b>.
 *
 * This function constructs a new <b>process_win32_t</b> and initializes the
 * default values. */
process_win32_t *
process_win32_new(void)
{
  process_win32_t *win32_process;
  win32_process = tor_malloc_zero(sizeof(process_win32_t));

  win32_process->stdin_handle.pipe = INVALID_HANDLE_VALUE;
  win32_process->stdout_handle.pipe = INVALID_HANDLE_VALUE;
  win32_process->stderr_handle.pipe = INVALID_HANDLE_VALUE;

  return win32_process;
}

/** Free a given <b>process_win32_t</b>.
 *
 * This function deinitializes and frees up the resources allocated for the
 * given <b>process_win32_t</b>. */
void
process_win32_free_(process_win32_t *win32_process)
{
  if (! win32_process)
    return;

  /* Cleanup our handles. */
  process_win32_cleanup_handle(&win32_process->stdin_handle);
  process_win32_cleanup_handle(&win32_process->stdout_handle);
  process_win32_cleanup_handle(&win32_process->stderr_handle);

  tor_free(win32_process);
}

/** Initialize the Windows backend of the Process subsystem. */
void
process_win32_init(void)
{
  /* We don't start the periodic timer here because it makes no sense to have
   * the timer running until we have some processes that benefits from the
   * timer timer ticks. */
}

/** Deinitialize the Windows backend of the Process subsystem. */
void
process_win32_deinit(void)
{
  /* Stop our timer, but only if it's running. */
  if (process_win32_timer_running())
    process_win32_timer_stop();
}

/** Execute the given process. This function is responsible for setting up
 * named pipes for I/O between the child process and the Tor process. Returns
 * <b>PROCESS_STATUS_RUNNING</b> upon success. */
process_status_t
process_win32_exec(process_t *process)
{
  tor_assert(process);

  process_win32_t *win32_process = process_get_win32_process(process);

  HANDLE stdout_pipe_read = NULL;
  HANDLE stdout_pipe_write = NULL;
  HANDLE stderr_pipe_read = NULL;
  HANDLE stderr_pipe_write = NULL;
  HANDLE stdin_pipe_read = NULL;
  HANDLE stdin_pipe_write = NULL;
  BOOL ret = FALSE;

  /* Setup our security attributes. */
  SECURITY_ATTRIBUTES security_attributes;
  security_attributes.nLength = sizeof(security_attributes);
  security_attributes.bInheritHandle = TRUE;
  /* FIXME: should we set explicit security attributes?
   * (See Ticket #2046, comment 5) */
  security_attributes.lpSecurityDescriptor = NULL;

  /* Create our standard out pipe. */
  if (! process_win32_create_pipe(&stdout_pipe_read,
                                  &stdout_pipe_write,
                                  &security_attributes,
                                  PROCESS_WIN32_PIPE_TYPE_READER)) {
    return PROCESS_STATUS_ERROR;
  }

  /* Create our standard error pipe. */
  if (! process_win32_create_pipe(&stderr_pipe_read,
                                  &stderr_pipe_write,
                                  &security_attributes,
                                  PROCESS_WIN32_PIPE_TYPE_READER)) {
    return PROCESS_STATUS_ERROR;
  }

  /* Create out standard in pipe. */
  if (! process_win32_create_pipe(&stdin_pipe_read,
                                  &stdin_pipe_write,
                                  &security_attributes,
                                  PROCESS_WIN32_PIPE_TYPE_WRITER)) {
    return PROCESS_STATUS_ERROR;
  }

  /* Configure startup info for our child process. */
  STARTUPINFOA startup_info;

  memset(&startup_info, 0, sizeof(startup_info));
  startup_info.cb = sizeof(startup_info);
  startup_info.hStdError = stderr_pipe_write;
  startup_info.hStdOutput = stdout_pipe_write;
  startup_info.hStdInput = stdin_pipe_read;
  startup_info.dwFlags |= STARTF_USESTDHANDLES;

  /* Create the env value for our new process. */
  process_environment_t *env = process_get_environment(process);

  /* Create the argv value for our new process. */
  char **argv = process_get_argv(process);

  /* Windows expects argv to be a whitespace delimited string, so join argv up
   */
  char *joined_argv = tor_join_win_cmdline((const char **)argv);

  /* Create the child process */
  ret = CreateProcessA(NULL,
                       joined_argv,
                       NULL,
                       NULL,
                       TRUE,
                       CREATE_NO_WINDOW,
                       env->windows_environment_block[0] == '\0' ?
                         NULL : env->windows_environment_block,
                       NULL,
                       &startup_info,
                       &win32_process->process_information);

  tor_free(argv);
  tor_free(joined_argv);
  process_environment_free(env);

  if (! ret) {
    log_warn(LD_PROCESS, "CreateProcessA() failed: %s",
      format_win32_error(GetLastError()));

    /* Cleanup our handles. */
    CloseHandle(stdout_pipe_read);
    CloseHandle(stdout_pipe_write);
    CloseHandle(stderr_pipe_read);
    CloseHandle(stderr_pipe_write);
    CloseHandle(stdin_pipe_read);
    CloseHandle(stdin_pipe_write);

    /* In the Unix backend, we do not get an error in the Tor process when a
     * child process fails to spawn its target executable since we need to
     * first do the fork() call in the Tor process and then the child process
     * is responsible for doing the call to execve().
     *
     * This means that the user of the process_exec() API must check for
     * whether it returns PROCESS_STATUS_ERROR, which will rarely happen on
     * Unix, but will happen for error cases on Windows where it does not
     * happen on Unix. For example: when the target executable does not exist
     * on the file system.
     *
     * To have somewhat feature compatibility between the Unix and the Windows
     * backend, we here notify the process_t owner that the process have exited
     * (even though it never managed to run) to ensure that the exit callback
     * is executed.
     */
    process_notify_event_exit(process, 0);

    return PROCESS_STATUS_ERROR;
  }

  /* TODO: Should we close hProcess and hThread in
   * process_handle->process_information? */
  win32_process->stdout_handle.pipe = stdout_pipe_read;
  win32_process->stderr_handle.pipe = stderr_pipe_read;
  win32_process->stdin_handle.pipe = stdin_pipe_write;

  /* Close our ends of the pipes that is now owned by the child process. */
  CloseHandle(stdout_pipe_write);
  CloseHandle(stderr_pipe_write);
  CloseHandle(stdin_pipe_read);

  /* Used by the callback functions from ReadFileEx() and WriteFileEx() such
   * that we can figure out which process_t that was responsible for the event.
   *
   * Warning, here be dragons:
   *
   *   MSDN says that the hEvent member of the overlapped structure is unused
   *   for ReadFileEx() and WriteFileEx, which allows us to store a pointer to
   *   our process state there.
   */
  win32_process->stdout_handle.overlapped.hEvent = (HANDLE)process;
  win32_process->stderr_handle.overlapped.hEvent = (HANDLE)process;
  win32_process->stdin_handle.overlapped.hEvent = (HANDLE)process;

  /* Start our timer if it is not already running. */
  if (! process_win32_timer_running())
    process_win32_timer_start();

  /* We use Windows Extended I/O functions, so our completion callbacks are
   * called automatically for us when there is data to read. Because of this
   * we start the read of standard out and error right away. */
  process_notify_event_stdout(process);
  process_notify_event_stderr(process);

  return PROCESS_STATUS_RUNNING;
}

/** Terminate the given process. Returns true on success, otherwise false. */
bool
process_win32_terminate(process_t *process)
{
  tor_assert(process);

  process_win32_t *win32_process = process_get_win32_process(process);

  /* Terminate our process. */
  BOOL ret;

  ret = TerminateProcess(win32_process->process_information.hProcess, 0);

  if (! ret) {
    log_warn(LD_PROCESS, "TerminateProcess() failed: %s",
             format_win32_error(GetLastError()));
    return false;
  }

  /* Cleanup our handles. */
  process_win32_cleanup_handle(&win32_process->stdin_handle);
  process_win32_cleanup_handle(&win32_process->stdout_handle);
  process_win32_cleanup_handle(&win32_process->stderr_handle);

  return true;
}

/** Returns the unique process identifier for the given <b>process</b>. */
process_pid_t
process_win32_get_pid(process_t *process)
{
  tor_assert(process);

  process_win32_t *win32_process = process_get_win32_process(process);
  return (process_pid_t)win32_process->process_information.dwProcessId;
}

/** Schedule an async write of the data found in <b>buffer</b> for the given
 * process.  This function runs an async write operation of the content of
 * buffer, if we are not already waiting for a pending I/O request. Returns the
 * number of bytes that Windows will hopefully write for us in the background.
 * */
int
process_win32_write(struct process_t *process, buf_t *buffer)
{
  tor_assert(process);
  tor_assert(buffer);

  process_win32_t *win32_process = process_get_win32_process(process);
  BOOL ret = FALSE;
  DWORD error_code = 0;
  const size_t buffer_size = buf_datalen(buffer);

  /* Windows is still writing our buffer. */
  if (win32_process->stdin_handle.busy)
    return 0;

  /* Nothing for us to do right now. */
  if (buffer_size == 0)
    return 0;

  /* We have reached end of file already? */
  if (BUG(win32_process->stdin_handle.reached_eof))
    return 0;

  /* Figure out how much data we should read. */
  const size_t write_size = MIN(buffer_size,
                                sizeof(win32_process->stdin_handle.buffer));

  /* Read data from the process_t buffer into our intermediate buffer. */
  buf_get_bytes(buffer, win32_process->stdin_handle.buffer, write_size);

  /* Because of the slightly weird API for WriteFileEx() we must set this to 0
   * before we call WriteFileEx() because WriteFileEx() does not reset the last
   * error itself when it's successful. See comment below after the call to
   * GetLastError(). */
  SetLastError(0);

  /* Schedule our write. */
  ret = WriteFileEx(win32_process->stdin_handle.pipe,
                    win32_process->stdin_handle.buffer,
                    write_size,
                    &win32_process->stdin_handle.overlapped,
                    process_win32_stdin_write_done);

  if (! ret) {
    error_code = GetLastError();

    /* No need to log at warning level for these two. */
    if (error_code == ERROR_HANDLE_EOF || error_code == ERROR_BROKEN_PIPE) {
      log_debug(LD_PROCESS, "WriteFileEx() returned EOF from pipe: %s",
                format_win32_error(error_code));
    } else {
      log_warn(LD_PROCESS, "WriteFileEx() failed: %s",
               format_win32_error(error_code));
    }

    win32_process->stdin_handle.reached_eof = true;
    return 0;
  }

  /* Here be dragons: According to MSDN's documentation for WriteFileEx() we
   * should check GetLastError() after a call to WriteFileEx() even though the
   * `ret` return value was successful. If everything is good, GetLastError()
   * returns `ERROR_SUCCESS` and nothing happens.
   *
   * XXX(ahf): I have not managed to trigger this code while stress-testing
   * this code. */
  error_code = GetLastError();

  if (error_code != ERROR_SUCCESS) {
    /* LCOV_EXCL_START */
    log_warn(LD_PROCESS, "WriteFileEx() failed after returning success: %s",
             format_win32_error(error_code));
    win32_process->stdin_handle.reached_eof = true;
    return 0;
    /* LCOV_EXCL_STOP */
  }

  /* This cast should be safe since our buffer can maximum be BUFFER_SIZE
   * large. */
  return (int)write_size;
}

/** This function is called from the Process subsystem whenever the Windows
 * backend says it has data ready. This function also ensures that we are
 * starting a new background read from the standard output of the child process
 * and asks Windows to call process_win32_stdout_read_done() when that
 * operation is finished. Returns the number of bytes moved into <b>buffer</b>.
 * */
int
process_win32_read_stdout(struct process_t *process, buf_t *buffer)
{
  tor_assert(process);
  tor_assert(buffer);

  process_win32_t *win32_process = process_get_win32_process(process);

  return process_win32_read_from_handle(&win32_process->stdout_handle,
                                        buffer,
                                        process_win32_stdout_read_done);
}

/** This function is called from the Process subsystem whenever the Windows
 * backend says it has data ready. This function also ensures that we are
 * starting a new background read from the standard error of the child process
 * and asks Windows to call process_win32_stderr_read_done() when that
 * operation is finished. Returns the number of bytes moved into <b>buffer</b>.
 * */
int
process_win32_read_stderr(struct process_t *process, buf_t *buffer)
{
  tor_assert(process);
  tor_assert(buffer);

  process_win32_t *win32_process = process_get_win32_process(process);

  return process_win32_read_from_handle(&win32_process->stderr_handle,
                                        buffer,
                                        process_win32_stderr_read_done);
}

/** This function is responsible for moving the Tor process into what Microsoft
 * calls an "alertable" state. Once the process is in an alertable state the
 * Windows kernel will notify us when our background I/O requests have finished
 * and the callbacks will be executed. */
void
process_win32_trigger_completion_callbacks(void)
{
  DWORD ret;

  /* The call to SleepEx(dwMilliseconds, dwAlertable) makes the process sleep
   * for dwMilliseconds and if dwAlertable is set to TRUE it will also cause
   * the process to enter alertable state, where the Windows kernel will notify
   * us about completed I/O requests from ReadFileEx() and WriteFileEX(), which
   * will cause our completion callbacks to be executed.
   *
   * This function returns 0 if the time interval expired or WAIT_IO_COMPLETION
   * if one or more I/O callbacks were executed. */
  ret = SleepEx(0, TRUE);

  /* Warn us if the function returned something we did not anticipate. */
  if (ret != 0 && ret != WAIT_IO_COMPLETION) {
    log_warn(LD_PROCESS, "SleepEx() returned %lu", ret);
  }
}

/** Start the periodic timer which is responsible for checking whether
 * processes are still alive and to make sure that the Tor process is
 * periodically being moved into an alertable state. */
void
process_win32_timer_start(void)
{
  /* Make sure we never start our timer if it's already running. */
  if (BUG(process_win32_timer_running()))
    return;

  /* Wake up once a second. */
  static const struct timeval interval = {1, 0};

  log_info(LD_PROCESS, "Starting Windows Process I/O timer");
  periodic_timer = periodic_timer_new(tor_libevent_get_base(),
                                      &interval,
                                      process_win32_timer_callback,
                                      NULL);
}

/** Stops the periodic timer. */
void
process_win32_timer_stop(void)
{
  if (BUG(periodic_timer == NULL))
    return;

  log_info(LD_PROCESS, "Stopping Windows Process I/O timer");
  periodic_timer_free(periodic_timer);
}

/** Returns true iff the periodic timer is running. */
bool
process_win32_timer_running(void)
{
  return periodic_timer != NULL;
}

/** This function is called whenever the periodic_timer ticks. The function is
 * responsible for moving the Tor process into an alertable state once a second
 * and checking for whether our child processes have terminated since the last
 * tick. */
STATIC void
process_win32_timer_callback(periodic_timer_t *timer, void *data)
{
  tor_assert(timer == periodic_timer);
  tor_assert(data == NULL);

  /* Move the process into an alertable state. */
  process_win32_trigger_completion_callbacks();

  /* Check if our processes are still alive. */

  /* Since the call to process_win32_timer_test_process() might call
   * process_notify_event_exit() which again might call process_free() which
   * updates the list of processes returned by process_get_all_processes() it
   * is important here that we make sure to not touch the list of processes if
   * the call to process_win32_timer_test_process() returns true. */
  bool done;

  do {
    const smartlist_t *processes = process_get_all_processes();
    done = true;

    SMARTLIST_FOREACH_BEGIN(processes, process_t *, process) {
      /* If process_win32_timer_test_process() returns true, it means that
       * smartlist_remove() might have been called on the list returned by
       * process_get_all_processes(). We start the loop over again until we
       * have a successful run over the entire list where the list was not
       * modified. */
      if (process_win32_timer_test_process(process)) {
        done = false;
        break;
      }
    } SMARTLIST_FOREACH_END(process);
  } while (! done);
}

/** Test whether a given process is still alive. Notify the Process subsystem
 * if our process have died. Returns true iff the given process have
 * terminated. */
STATIC bool
process_win32_timer_test_process(process_t *process)
{
  tor_assert(process);

  /* No need to look at processes that don't claim they are running. */
  if (process_get_status(process) != PROCESS_STATUS_RUNNING)
    return false;

  process_win32_t *win32_process = process_get_win32_process(process);
  BOOL ret = FALSE;
  DWORD exit_code = 0;

  /* Sometimes the Windows kernel won't give us the EOF/Broken Pipe error
   * message until some time after the process have actually terminated. We
   * make sure that our ReadFileEx() calls for the process have *all* returned
   * and both standard out and error have been marked as EOF before we try to
   * see if the process terminated.
   *
   * This ensures that we *never* call the exit callback of the `process_t`,
   * which potentially ends up calling `process_free()` on our `process_t`,
   * before all data have been received from the process.
   *
   * We do NOT have a check here for whether standard in reached EOF since
   * standard in's WriteFileEx() function is only called on-demand when we have
   * something to write and is thus usually not awaiting to finish any
   * operations. If we WriteFileEx() to a file that has terminated we'll simply
   * get an error from ReadFileEx() or its completion routine and move on with
   * life.  */
  if (! win32_process->stdout_handle.reached_eof)
    return false;

  if (! win32_process->stderr_handle.reached_eof)
    return false;

  /* We start by testing whether our process is still running. */
  ret = GetExitCodeProcess(win32_process->process_information.hProcess,
                           &exit_code);

  if (! ret) {
    log_warn(LD_PROCESS, "GetExitCodeProcess() failed: %s",
             format_win32_error(GetLastError()));
    return false;
  }

  /* Notify our process_t that our process have terminated. Since our
   * exit_callback might decide to process_free() our process handle it is very
   * important that we do not touch the process_t after the call to
   * process_notify_event_exit(). */
  if (exit_code != STILL_ACTIVE) {
    process_notify_event_exit(process, exit_code);
    return true;
  }

  return false;
}

/** Create a new overlapped named pipe. This function creates a new connected,
 * named, pipe in <b>*read_pipe</b> and <b>*write_pipe</b> if the function is
 * successful. Returns true on success, false on failure. */
STATIC bool
process_win32_create_pipe(HANDLE *read_pipe,
                          HANDLE *write_pipe,
                          SECURITY_ATTRIBUTES *attributes,
                          process_win32_pipe_type_t pipe_type)
{
  tor_assert(read_pipe);
  tor_assert(write_pipe);
  tor_assert(attributes);

  BOOL ret = FALSE;

  /* Buffer size. */
  const size_t size = 4096;

  /* Our additional read/write modes that depends on which pipe type we are
   * creating. */
  DWORD read_mode = 0;
  DWORD write_mode = 0;

  /* Generate the unique pipe name. */
  char pipe_name[MAX_PATH];
  static DWORD process_id = 0;
  static DWORD counter = 0;

  if (process_id == 0)
    process_id = GetCurrentProcessId();

  tor_snprintf(pipe_name, sizeof(pipe_name),
               "\\\\.\\Pipe\\Tor-Process-Pipe-%lu-%lu",
               process_id, counter++);

  /* Only one of our handles can be overlapped. */
  switch (pipe_type) {
  case PROCESS_WIN32_PIPE_TYPE_READER:
    read_mode = FILE_FLAG_OVERLAPPED;
    break;
  case PROCESS_WIN32_PIPE_TYPE_WRITER:
    write_mode = FILE_FLAG_OVERLAPPED;
    break;
  default:
    /* LCOV_EXCL_START */
    tor_assert_nonfatal_unreached_once();
    /* LCOV_EXCL_STOP */
  }

  /* Setup our read and write handles. */
  HANDLE read_handle;
  HANDLE write_handle;

  /* Create our named pipe. */
  read_handle = CreateNamedPipeA(pipe_name,
                                 (PIPE_ACCESS_INBOUND|read_mode),
                                 (PIPE_TYPE_BYTE|PIPE_WAIT),
                                 1,
                                 size,
                                 size,
                                 1000,
                                 attributes);

  if (read_handle == INVALID_HANDLE_VALUE) {
    log_warn(LD_PROCESS, "CreateNamedPipeA() failed: %s",
             format_win32_error(GetLastError()));
    return false;
  }

  /* Create our file in the pipe namespace. */
  write_handle = CreateFileA(pipe_name,
                             GENERIC_WRITE,
                             0,
                             attributes,
                             OPEN_EXISTING,
                             (FILE_ATTRIBUTE_NORMAL|write_mode),
                             NULL);

  if (write_handle == INVALID_HANDLE_VALUE) {
    log_warn(LD_PROCESS, "CreateFileA() failed: %s",
             format_win32_error(GetLastError()));

    CloseHandle(read_handle);

    return false;
  }

  /* Set the inherit flag for our pipe. */
  switch (pipe_type) {
  case PROCESS_WIN32_PIPE_TYPE_READER:
    ret = SetHandleInformation(read_handle, HANDLE_FLAG_INHERIT, 0);
    break;
  case PROCESS_WIN32_PIPE_TYPE_WRITER:
    ret = SetHandleInformation(write_handle, HANDLE_FLAG_INHERIT, 0);
    break;
  default:
    /* LCOV_EXCL_START */
    tor_assert_nonfatal_unreached_once();
    /* LCOV_EXCL_STOP */
  }

  if (! ret) {
    log_warn(LD_PROCESS, "SetHandleInformation() failed: %s",
             format_win32_error(GetLastError()));

    CloseHandle(read_handle);
    CloseHandle(write_handle);

    return false;
  }

  /* Everything is good. */
  *read_pipe = read_handle;
  *write_pipe = write_handle;

  return true;
}

/** Cleanup a given <b>handle</b>. */
STATIC void
process_win32_cleanup_handle(process_win32_handle_t *handle)
{
  tor_assert(handle);

#if 0
  BOOL ret;
  DWORD error_code;

  /* Cancel any pending I/O requests: This means that instead of getting
   * ERROR_BROKEN_PIPE we get ERROR_OPERATION_ABORTED, but it doesn't seem
   * like this is needed. */
  ret = CancelIo(handle->pipe);

  if (! ret) {
    error_code = GetLastError();

    /* There was no pending I/O requests for our handle. */
    if (error_code != ERROR_NOT_FOUND) {
      log_warn(LD_PROCESS, "CancelIo() failed: %s",
               format_win32_error(error_code));
    }
  }
#endif /* 0 */

  /* Close our handle. */
  if (handle->pipe != INVALID_HANDLE_VALUE) {
    CloseHandle(handle->pipe);
    handle->pipe = INVALID_HANDLE_VALUE;
    handle->reached_eof = true;
  }
}

/** This function is called when ReadFileEx() completes its background read
 * from the child process's standard output. We notify the Process subsystem if
 * there is data available for it to read from us. */
STATIC VOID WINAPI
process_win32_stdout_read_done(DWORD error_code,
                               DWORD byte_count,
                               LPOVERLAPPED overlapped)
{
  tor_assert(overlapped);
  tor_assert(overlapped->hEvent);

  /* Extract our process_t from the hEvent member of OVERLAPPED. */
  process_t *process = (process_t *)overlapped->hEvent;
  process_win32_t *win32_process = process_get_win32_process(process);

  if (process_win32_handle_read_completion(&win32_process->stdout_handle,
                                           error_code,
                                           byte_count)) {
    /* Schedule our next read. */
    process_notify_event_stdout(process);
  }
}

/** This function is called when ReadFileEx() completes its background read
 * from the child process's standard error. We notify the Process subsystem if
 * there is data available for it to read from us. */
STATIC VOID WINAPI
process_win32_stderr_read_done(DWORD error_code,
                               DWORD byte_count,
                               LPOVERLAPPED overlapped)
{
  tor_assert(overlapped);
  tor_assert(overlapped->hEvent);

  /* Extract our process_t from the hEvent member of OVERLAPPED. */
  process_t *process = (process_t *)overlapped->hEvent;
  process_win32_t *win32_process = process_get_win32_process(process);

  if (process_win32_handle_read_completion(&win32_process->stderr_handle,
                                           error_code,
                                           byte_count)) {
    /* Schedule our next read. */
    process_notify_event_stderr(process);
  }
}

/** This function is called when WriteFileEx() completes its background write
 * to the child process's standard input. We notify the Process subsystem that
 * it can write data to us again. */
STATIC VOID WINAPI
process_win32_stdin_write_done(DWORD error_code,
                               DWORD byte_count,
                               LPOVERLAPPED overlapped)
{
  tor_assert(overlapped);
  tor_assert(overlapped->hEvent);

  (void)byte_count;

  process_t *process = (process_t *)overlapped->hEvent;
  process_win32_t *win32_process = process_get_win32_process(process);

  /* Mark our handle as not having any outstanding I/O requests. */
  win32_process->stdin_handle.busy = false;

  /* Check if we have been asked to write to the handle that have been marked
   * as having reached EOF. */
  if (BUG(win32_process->stdin_handle.reached_eof))
    return;

  if (error_code == 0) {
    /** Our data have been successfully written. Clear our state and schedule
     * the next write. */
    win32_process->stdin_handle.data_available = 0;
    memset(win32_process->stdin_handle.buffer, 0,
           sizeof(win32_process->stdin_handle.buffer));

    /* Schedule the next write. */
    process_notify_event_stdin(process);
  } else if (error_code == ERROR_HANDLE_EOF ||
             error_code == ERROR_BROKEN_PIPE) {
    /* Our WriteFileEx() call was successful, but we reached the end of our
     * file.  We mark our handle as having reached EOF and returns. */
    tor_assert(byte_count == 0);

    win32_process->stdin_handle.reached_eof = true;
  } else {
    /* An error happened: We warn the user and mark our handle as having
     * reached EOF */
    log_warn(LD_PROCESS,
             "Error in I/O completion routine from WriteFileEx(): %s",
             format_win32_error(error_code));
    win32_process->stdin_handle.reached_eof = true;
  }
}

/** This function reads data from the given <b>handle</b>'s internal buffer and
 * moves it into the given <b>buffer</b>. Additionally, we start the next
 * ReadFileEx() background operation with the given <b>callback</b> as
 * completion callback. Returns the number of bytes written to the buffer. */
STATIC int
process_win32_read_from_handle(process_win32_handle_t *handle,
                               buf_t *buffer,
                               LPOVERLAPPED_COMPLETION_ROUTINE callback)
{
  tor_assert(handle);
  tor_assert(buffer);
  tor_assert(callback);

  BOOL ret = FALSE;
  int bytes_available = 0;
  DWORD error_code = 0;

  /* We already have a request to read data that isn't complete yet. */
  if (BUG(handle->busy))
    return 0;

  /* Check if we have been asked to read from a handle that have already told
   * us that we have reached the end of the file. */
  if (BUG(handle->reached_eof))
    return 0;

  /* This cast should be safe since our buffer can be at maximum up to
   * BUFFER_SIZE in size. */
  bytes_available = (int)handle->data_available;

  if (handle->data_available > 0) {
    /* Read data from our intermediate buffer into the process_t buffer. */
    buf_add(buffer, handle->buffer, handle->data_available);

    /* Reset our read state. */
    handle->data_available = 0;
    memset(handle->buffer, 0, sizeof(handle->buffer));
  }

  /* Because of the slightly weird API for ReadFileEx() we must set this to 0
   * before we call ReadFileEx() because ReadFileEx() does not reset the last
   * error itself when it's successful. See comment below after the call to
   * GetLastError(). */
  SetLastError(0);

  /* Ask the Windows kernel to read data from our pipe into our buffer and call
   * the callback function when it is done. */
  ret = ReadFileEx(handle->pipe,
                   handle->buffer,
                   sizeof(handle->buffer),
                   &handle->overlapped,
                   callback);

  if (! ret) {
    error_code = GetLastError();

    /* No need to log at warning level for these two. */
    if (error_code == ERROR_HANDLE_EOF || error_code == ERROR_BROKEN_PIPE) {
      log_debug(LD_PROCESS, "ReadFileEx() returned EOF from pipe: %s",
                format_win32_error(error_code));
    } else {
      log_warn(LD_PROCESS, "ReadFileEx() failed: %s",
               format_win32_error(error_code));
    }

    handle->reached_eof = true;
    return bytes_available;
  }

  /* Here be dragons: According to MSDN's documentation for ReadFileEx() we
   * should check GetLastError() after a call to ReadFileEx() even though the
   * `ret` return value was successful. If everything is good, GetLastError()
   * returns `ERROR_SUCCESS` and nothing happens.
   *
   * XXX(ahf): I have not managed to trigger this code while stress-testing
   * this code. */
  error_code = GetLastError();

  if (error_code != ERROR_SUCCESS) {
    /* LCOV_EXCL_START */
    log_warn(LD_PROCESS, "ReadFileEx() failed after returning success: %s",
             format_win32_error(error_code));
    handle->reached_eof = true;
    return bytes_available;
    /* LCOV_EXCL_STOP */
  }

  /* We mark our handle as having a pending I/O request. */
  handle->busy = true;

  return bytes_available;
}

/** This function checks the callback values from ReadFileEx() in
 * <b>error_code</b> and <b>byte_count</b> if we have read data. Returns true
 * iff our caller should request more data from ReadFileEx(). */
STATIC bool
process_win32_handle_read_completion(process_win32_handle_t *handle,
                                     DWORD error_code,
                                     DWORD byte_count)
{
  tor_assert(handle);

  /* Mark our handle as not having any outstanding I/O requests. */
  handle->busy = false;

  if (error_code == 0) {
    /* Our ReadFileEx() call was successful and there is data for us. */

    /* This cast should be safe since byte_count should never be larger than
     * BUFFER_SIZE. */
    tor_assert(byte_count <= BUFFER_SIZE);
    handle->data_available = (size_t)byte_count;

    /* Tell our caller to schedule the next read. */
    return true;
  } else if (error_code == ERROR_HANDLE_EOF ||
             error_code == ERROR_BROKEN_PIPE) {
    /* Our ReadFileEx() finished, but we reached the end of our file.  We mark
     * our handle as having reached EOF and returns. */
    tor_assert(byte_count == 0);

    handle->reached_eof = true;
  } else {
    /* An error happened: We warn the user and mark our handle as having
     * reached EOF */
    log_warn(LD_PROCESS,
             "Error in I/O completion routine from ReadFileEx(): %s",
             format_win32_error(error_code));

    handle->reached_eof = true;
  }

  /* Our caller should NOT schedule the next read. */
  return false;
}

/** Format a single argument for being put on a Windows command line.
 * Returns a newly allocated string */
STATIC char *
format_win_cmdline_argument(const char *arg)
{
  char *formatted_arg;
  char need_quotes;
  const char *c;
  int i;
  int bs_counter = 0;
  /* Backslash we can point to when one is inserted into the string */
  const char backslash = '\\';

  /* Smartlist of *char */
  smartlist_t *arg_chars;
  arg_chars = smartlist_new();

  /* Quote string if it contains whitespace or is empty */
  need_quotes = (strchr(arg, ' ') || strchr(arg, '\t') || '\0' == arg[0]);

  /* Build up smartlist of *chars */
  for (c=arg; *c != '\0'; c++) {
    if ('"' == *c) {
      /* Double up backslashes preceding a quote */
      for (i=0; i<(bs_counter*2); i++)
        smartlist_add(arg_chars, (void*)&backslash);
      bs_counter = 0;
      /* Escape the quote */
      smartlist_add(arg_chars, (void*)&backslash);
      smartlist_add(arg_chars, (void*)c);
    } else if ('\\' == *c) {
      /* Count backslashes until we know whether to double up */
      bs_counter++;
    } else {
      /* Don't double up slashes preceding a non-quote */
      for (i=0; i<bs_counter; i++)
        smartlist_add(arg_chars, (void*)&backslash);
      bs_counter = 0;
      smartlist_add(arg_chars, (void*)c);
    }
  }
  /* Don't double up trailing backslashes */
  for (i=0; i<bs_counter; i++)
    smartlist_add(arg_chars, (void*)&backslash);

  /* Allocate space for argument, quotes (if needed), and terminator */
  const size_t formatted_arg_len = smartlist_len(arg_chars) +
    (need_quotes ? 2 : 0) + 1;
  formatted_arg = tor_malloc_zero(formatted_arg_len);

  /* Add leading quote */
  i=0;
  if (need_quotes)
    formatted_arg[i++] = '"';

  /* Add characters */
  SMARTLIST_FOREACH(arg_chars, char*, ch,
  {
    formatted_arg[i++] = *ch;
  });

  /* Add trailing quote */
  if (need_quotes)
    formatted_arg[i++] = '"';
  formatted_arg[i] = '\0';

  smartlist_free(arg_chars);
  return formatted_arg;
}

/** Format a command line for use on Windows, which takes the command as a
 * string rather than string array. Follows the rules from "Parsing C++
 * Command-Line Arguments" in MSDN. Algorithm based on list2cmdline in the
 * Python subprocess module. Returns a newly allocated string */
STATIC char *
tor_join_win_cmdline(const char *argv[])
{
  smartlist_t *argv_list;
  char *joined_argv;
  int i;

  /* Format each argument and put the result in a smartlist */
  argv_list = smartlist_new();
  for (i=0; argv[i] != NULL; i++) {
    smartlist_add(argv_list, (void *)format_win_cmdline_argument(argv[i]));
  }

  /* Join the arguments with whitespace */
  joined_argv = smartlist_join_strings(argv_list, " ", 0, NULL);

  /* Free the newly allocated arguments, and the smartlist */
  SMARTLIST_FOREACH(argv_list, char *, arg,
  {
    tor_free(arg);
  });
  smartlist_free(argv_list);

  return joined_argv;
}

#endif /* defined(_WIN32) */
