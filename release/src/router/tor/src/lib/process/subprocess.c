/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file subprocess.c
 * \brief Launch and monitor other processes.
 **/

#define SUBPROCESS_PRIVATE
#include "lib/process/subprocess.h"

#include "lib/container/smartlist.h"
#include "lib/err/torerr.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/log/win32err.h"
#include "lib/malloc/malloc.h"
#include "lib/process/env.h"
#include "lib/process/waitpid.h"
#include "lib/string/compat_ctype.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <errno.h>
#include <string.h>

/** Format a single argument for being put on a Windows command line.
 * Returns a newly allocated string */
static char *
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
char *
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

#ifndef _WIN32
/** Format <b>child_state</b> and <b>saved_errno</b> as a hex string placed in
 * <b>hex_errno</b>.  Called between fork and _exit, so must be signal-handler
 * safe.
 *
 * <b>hex_errno</b> must have at least HEX_ERRNO_SIZE+1 bytes available.
 *
 * The format of <b>hex_errno</b> is: "CHILD_STATE/ERRNO\n", left-padded
 * with spaces. CHILD_STATE indicates where
 * in the process of starting the child process did the failure occur (see
 * CHILD_STATE_* macros for definition), and SAVED_ERRNO is the value of
 * errno when the failure occurred.
 *
 * On success return the number of characters added to hex_errno, not counting
 * the terminating NUL; return -1 on error.
 */
STATIC int
format_helper_exit_status(unsigned char child_state, int saved_errno,
                          char *hex_errno)
{
  unsigned int unsigned_errno;
  int written, left;
  char *cur;
  size_t i;
  int res = -1;

  /* Fill hex_errno with spaces, and a trailing newline (memset may
     not be signal handler safe, so we can't use it) */
  for (i = 0; i < (HEX_ERRNO_SIZE - 1); i++)
    hex_errno[i] = ' ';
  hex_errno[HEX_ERRNO_SIZE - 1] = '\n';

  /* Convert errno to be unsigned for hex conversion */
  if (saved_errno < 0) {
    // Avoid overflow on the cast to unsigned int when result is INT_MIN
    // by adding 1 to the signed int negative value,
    // then, after it has been negated and cast to unsigned,
    // adding the original 1 back (the double-addition is intentional).
    // Otherwise, the cast to signed could cause a temporary int
    // to equal INT_MAX + 1, which is undefined.
    unsigned_errno = ((unsigned int) -(saved_errno + 1)) + 1;
  } else {
    unsigned_errno = (unsigned int) saved_errno;
  }

  /*
   * Count how many chars of space we have left, and keep a pointer into the
   * current point in the buffer.
   */
  left = HEX_ERRNO_SIZE+1;
  cur = hex_errno;

  /* Emit child_state */
  written = format_hex_number_sigsafe(child_state, cur, left);

  if (written <= 0)
    goto err;

  /* Adjust left and cur */
  left -= written;
  cur += written;
  if (left <= 0)
    goto err;

  /* Now the '/' */
  *cur = '/';

  /* Adjust left and cur */
  ++cur;
  --left;
  if (left <= 0)
    goto err;

  /* Need minus? */
  if (saved_errno < 0) {
    *cur = '-';
    ++cur;
    --left;
    if (left <= 0)
      goto err;
  }

  /* Emit unsigned_errno */
  written = format_hex_number_sigsafe(unsigned_errno, cur, left);

  if (written <= 0)
    goto err;

  /* Adjust left and cur */
  left -= written;
  cur += written;

  /* Check that we have enough space left for a newline and a NUL */
  if (left <= 1)
    goto err;

  /* Emit the newline and NUL */
  *cur++ = '\n';
  *cur++ = '\0';

  res = (int)(cur - hex_errno - 1);

  goto done;

 err:
  /*
   * In error exit, just write a '\0' in the first char so whatever called
   * this at least won't fall off the end.
   */
  *hex_errno = '\0';

 done:
  return res;
}
#endif /* !defined(_WIN32) */

/* Maximum number of file descriptors, if we cannot get it via sysconf() */
#define DEFAULT_MAX_FD 256

/** Terminate the process of <b>process_handle</b>, if that process has not
 * already exited.
 *
 * Return 0 if we succeeded in terminating the process (or if the process
 * already exited), and -1 if we tried to kill the process but failed.
 *
 * Based on code originally borrowed from Python's os.kill. */
int
tor_terminate_process(process_handle_t *process_handle)
{
#ifdef _WIN32
  if (tor_get_exit_code(process_handle, 0, NULL) == PROCESS_EXIT_RUNNING) {
    HANDLE handle = process_handle->pid.hProcess;

    if (!TerminateProcess(handle, 0))
      return -1;
    else
      return 0;
  }
#else /* !(defined(_WIN32)) */
  if (process_handle->waitpid_cb) {
    /* We haven't got a waitpid yet, so we can just kill off the process. */
    return kill(process_handle->pid, SIGTERM);
  }
#endif /* defined(_WIN32) */

  return 0; /* We didn't need to kill the process, so report success */
}

/** Return the Process ID of <b>process_handle</b>. */
int
tor_process_get_pid(process_handle_t *process_handle)
{
#ifdef _WIN32
  return (int) process_handle->pid.dwProcessId;
#else
  return (int) process_handle->pid;
#endif
}

#ifdef _WIN32
HANDLE
tor_process_get_stdout_pipe(process_handle_t *process_handle)
{
  return process_handle->stdout_pipe;
}
#else /* !(defined(_WIN32)) */
/* DOCDOC tor_process_get_stdout_pipe */
int
tor_process_get_stdout_pipe(process_handle_t *process_handle)
{
  return process_handle->stdout_pipe;
}
#endif /* defined(_WIN32) */

/* DOCDOC process_handle_new */
static process_handle_t *
process_handle_new(void)
{
  process_handle_t *out = tor_malloc_zero(sizeof(process_handle_t));

#ifdef _WIN32
  out->stdin_pipe = INVALID_HANDLE_VALUE;
  out->stdout_pipe = INVALID_HANDLE_VALUE;
  out->stderr_pipe = INVALID_HANDLE_VALUE;
#else
  out->stdin_pipe = -1;
  out->stdout_pipe = -1;
  out->stderr_pipe = -1;
#endif /* defined(_WIN32) */

  return out;
}

#ifndef _WIN32
/** Invoked when a process that we've launched via tor_spawn_background() has
 * been found to have terminated.
 */
static void
process_handle_waitpid_cb(int status, void *arg)
{
  process_handle_t *process_handle = arg;

  process_handle->waitpid_exit_status = status;
  clear_waitpid_callback(process_handle->waitpid_cb);
  if (process_handle->status == PROCESS_STATUS_RUNNING)
    process_handle->status = PROCESS_STATUS_NOTRUNNING;
  process_handle->waitpid_cb = 0;
}
#endif /* !defined(_WIN32) */

/**
 * @name child-process states
 *
 * Each of these values represents a possible state that a child process can
 * be in.  They're used to determine what to say when telling the parent how
 * far along we were before failure.
 *
 * @{
 */
#define CHILD_STATE_INIT 0
#define CHILD_STATE_PIPE 1
#define CHILD_STATE_MAXFD 2
#define CHILD_STATE_FORK 3
#define CHILD_STATE_DUPOUT 4
#define CHILD_STATE_DUPERR 5
#define CHILD_STATE_DUPIN 6
#define CHILD_STATE_CLOSEFD 7
#define CHILD_STATE_EXEC 8
#define CHILD_STATE_FAILEXEC 9
/** @} */
/**
 * Boolean.  If true, then Tor may call execve or CreateProcess via
 * tor_spawn_background.
 **/
static int may_spawn_background_process = 1;
/**
 * Turn off may_spawn_background_process, so that all future calls to
 * tor_spawn_background are guaranteed to fail.
 **/
void
tor_disable_spawning_background_processes(void)
{
  may_spawn_background_process = 0;
}
/** Start a program in the background. If <b>filename</b> contains a '/', then
 * it will be treated as an absolute or relative path.  Otherwise, on
 * non-Windows systems, the system path will be searched for <b>filename</b>.
 * On Windows, only the current directory will be searched. Here, to search the
 * system path (as well as the application directory, current working
 * directory, and system directories), set filename to NULL.
 *
 * The strings in <b>argv</b> will be passed as the command line arguments of
 * the child program (following convention, argv[0] should normally be the
 * filename of the executable, and this must be the case if <b>filename</b> is
 * NULL). The last element of argv must be NULL. A handle to the child process
 * will be returned in process_handle (which must be non-NULL). Read
 * process_handle.status to find out if the process was successfully launched.
 * For convenience, process_handle.status is returned by this function.
 *
 * Some parts of this code are based on the POSIX subprocess module from
 * Python, and example code from
 * http://msdn.microsoft.com/en-us/library/ms682499%28v=vs.85%29.aspx.
 */
int
tor_spawn_background(const char *const filename, const char **argv,
                     process_environment_t *env,
                     process_handle_t **process_handle_out)
{
  if (BUG(may_spawn_background_process == 0)) {
    /* We should never reach this point if we're forbidden to spawn
     * processes. Instead we should have caught the attempt earlier. */
    return PROCESS_STATUS_ERROR;
  }

#ifdef _WIN32
  HANDLE stdout_pipe_read = NULL;
  HANDLE stdout_pipe_write = NULL;
  HANDLE stderr_pipe_read = NULL;
  HANDLE stderr_pipe_write = NULL;
  HANDLE stdin_pipe_read = NULL;
  HANDLE stdin_pipe_write = NULL;
  process_handle_t *process_handle;
  int status;

  STARTUPINFOA siStartInfo;
  BOOL retval = FALSE;

  SECURITY_ATTRIBUTES saAttr;
  char *joined_argv;

  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  /* TODO: should we set explicit security attributes? (#2046, comment 5) */
  saAttr.lpSecurityDescriptor = NULL;

  /* Assume failure to start process */
  status = PROCESS_STATUS_ERROR;

  /* Set up pipe for stdout */
  if (!CreatePipe(&stdout_pipe_read, &stdout_pipe_write, &saAttr, 0)) {
    log_warn(LD_GENERAL,
      "Failed to create pipe for stdout communication with child process: %s",
      format_win32_error(GetLastError()));
    return status;
  }
  if (!SetHandleInformation(stdout_pipe_read, HANDLE_FLAG_INHERIT, 0)) {
    log_warn(LD_GENERAL,
      "Failed to configure pipe for stdout communication with child "
      "process: %s", format_win32_error(GetLastError()));
    return status;
  }

  /* Set up pipe for stderr */
  if (!CreatePipe(&stderr_pipe_read, &stderr_pipe_write, &saAttr, 0)) {
    log_warn(LD_GENERAL,
      "Failed to create pipe for stderr communication with child process: %s",
      format_win32_error(GetLastError()));
    return status;
  }
  if (!SetHandleInformation(stderr_pipe_read, HANDLE_FLAG_INHERIT, 0)) {
    log_warn(LD_GENERAL,
      "Failed to configure pipe for stderr communication with child "
      "process: %s", format_win32_error(GetLastError()));
    return status;
  }

  /* Set up pipe for stdin */
  if (!CreatePipe(&stdin_pipe_read, &stdin_pipe_write, &saAttr, 0)) {
    log_warn(LD_GENERAL,
      "Failed to create pipe for stdin communication with child process: %s",
      format_win32_error(GetLastError()));
    return status;
  }
  if (!SetHandleInformation(stdin_pipe_write, HANDLE_FLAG_INHERIT, 0)) {
    log_warn(LD_GENERAL,
      "Failed to configure pipe for stdin communication with child "
      "process: %s", format_win32_error(GetLastError()));
    return status;
  }

  /* Create the child process */

  /* Windows expects argv to be a whitespace delimited string, so join argv up
   */
  joined_argv = tor_join_win_cmdline(argv);

  process_handle = process_handle_new();
  process_handle->status = status;

  ZeroMemory(&(process_handle->pid), sizeof(PROCESS_INFORMATION));
  ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
  siStartInfo.cb = sizeof(STARTUPINFO);
  siStartInfo.hStdError = stderr_pipe_write;
  siStartInfo.hStdOutput = stdout_pipe_write;
  siStartInfo.hStdInput = stdin_pipe_read;
  siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

  /* Create the child process */

  retval = CreateProcessA(filename,      // module name
                 joined_argv,   // command line
  /* TODO: should we set explicit security attributes? (#2046, comment 5) */
                 NULL,          // process security attributes
                 NULL,          // primary thread security attributes
                 TRUE,          // handles are inherited
  /*(TODO: set CREATE_NEW CONSOLE/PROCESS_GROUP to make GetExitCodeProcess()
   * work?) */
                 CREATE_NO_WINDOW,             // creation flags
                 (env==NULL) ? NULL : env->windows_environment_block,
                 NULL,          // use parent's current directory
                 &siStartInfo,  // STARTUPINFO pointer
                 &(process_handle->pid));  // receives PROCESS_INFORMATION

  tor_free(joined_argv);

  if (!retval) {
    log_warn(LD_GENERAL,
      "Failed to create child process %s: %s", filename?filename:argv[0],
      format_win32_error(GetLastError()));
    tor_free(process_handle);
  } else  {
    /* TODO: Close hProcess and hThread in process_handle->pid? */
    process_handle->stdout_pipe = stdout_pipe_read;
    process_handle->stderr_pipe = stderr_pipe_read;
    process_handle->stdin_pipe = stdin_pipe_write;
    status = process_handle->status = PROCESS_STATUS_RUNNING;
  }

  /* TODO: Close pipes on exit */
  *process_handle_out = process_handle;
  return status;
#else /* !(defined(_WIN32)) */
  pid_t pid;
  int stdout_pipe[2];
  int stderr_pipe[2];
  int stdin_pipe[2];
  int fd, retval;
  process_handle_t *process_handle;
  int status;

  const char *error_message = SPAWN_ERROR_MESSAGE;
  size_t error_message_length;

  /* Represents where in the process of spawning the program is;
     this is used for printing out the error message */
  unsigned char child_state = CHILD_STATE_INIT;

  char hex_errno[HEX_ERRNO_SIZE + 2]; /* + 1 should be sufficient actually */

  static int max_fd = -1;

  status = PROCESS_STATUS_ERROR;

  /* We do the strlen here because strlen() is not signal handler safe,
     and we are not allowed to use unsafe functions between fork and exec */
  error_message_length = strlen(error_message);

  // child_state = CHILD_STATE_PIPE;

  /* Set up pipe for redirecting stdout, stderr, and stdin of child */
  retval = pipe(stdout_pipe);
  if (-1 == retval) {
    log_warn(LD_GENERAL,
      "Failed to set up pipe for stdout communication with child process: %s",
       strerror(errno));
    return status;
  }

  retval = pipe(stderr_pipe);
  if (-1 == retval) {
    log_warn(LD_GENERAL,
      "Failed to set up pipe for stderr communication with child process: %s",
      strerror(errno));

    close(stdout_pipe[0]);
    close(stdout_pipe[1]);

    return status;
  }

  retval = pipe(stdin_pipe);
  if (-1 == retval) {
    log_warn(LD_GENERAL,
      "Failed to set up pipe for stdin communication with child process: %s",
       strerror(errno));

    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[0]);
    close(stderr_pipe[1]);

    return status;
  }

  // child_state = CHILD_STATE_MAXFD;

#ifdef _SC_OPEN_MAX
  if (-1 == max_fd) {
    max_fd = (int) sysconf(_SC_OPEN_MAX);
    if (max_fd == -1) {
      max_fd = DEFAULT_MAX_FD;
      log_warn(LD_GENERAL,
               "Cannot find maximum file descriptor, assuming %d", max_fd);
    }
  }
#else /* !(defined(_SC_OPEN_MAX)) */
  max_fd = DEFAULT_MAX_FD;
#endif /* defined(_SC_OPEN_MAX) */

  // child_state = CHILD_STATE_FORK;

  pid = fork();
  if (0 == pid) {
    /* In child */

#if defined(HAVE_SYS_PRCTL_H) && defined(__linux__)
    /* Attempt to have the kernel issue a SIGTERM if the parent
     * goes away. Certain attributes of the binary being execve()ed
     * will clear this during the execve() call, but it's better
     * than nothing.
     */
    prctl(PR_SET_PDEATHSIG, SIGTERM);
#endif /* defined(HAVE_SYS_PRCTL_H) && defined(__linux__) */

    child_state = CHILD_STATE_DUPOUT;

    /* Link child stdout to the write end of the pipe */
    retval = dup2(stdout_pipe[1], STDOUT_FILENO);
    if (-1 == retval)
        goto error;

    child_state = CHILD_STATE_DUPERR;

    /* Link child stderr to the write end of the pipe */
    retval = dup2(stderr_pipe[1], STDERR_FILENO);
    if (-1 == retval)
        goto error;

    child_state = CHILD_STATE_DUPIN;

    /* Link child stdin to the read end of the pipe */
    retval = dup2(stdin_pipe[0], STDIN_FILENO);
    if (-1 == retval)
      goto error;

    // child_state = CHILD_STATE_CLOSEFD;

    close(stderr_pipe[0]);
    close(stderr_pipe[1]);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);

    /* Close all other fds, including the read end of the pipe */
    /* XXX: We should now be doing enough FD_CLOEXEC setting to make
     * this needless. */
    for (fd = STDERR_FILENO + 1; fd < max_fd; fd++) {
      close(fd);
    }

    // child_state = CHILD_STATE_EXEC;

    /* Call the requested program. We need the cast because
       execvp doesn't define argv as const, even though it
       does not modify the arguments */
    if (env)
      execve(filename, (char *const *) argv, env->unixoid_environment_block);
    else {
      static char *new_env[] = { NULL };
      execve(filename, (char *const *) argv, new_env);
    }

    /* If we got here, the exec or open(/dev/null) failed */

    child_state = CHILD_STATE_FAILEXEC;

  error:
    {
      /* XXX: are we leaking fds from the pipe? */
      int n, err=0;
      ssize_t nbytes;

      n = format_helper_exit_status(child_state, errno, hex_errno);

      if (n >= 0) {
        /* Write the error message. GCC requires that we check the return
           value, but there is nothing we can do if it fails */
        /* TODO: Don't use STDOUT, use a pipe set up just for this purpose */
        nbytes = write(STDOUT_FILENO, error_message, error_message_length);
        err = (nbytes < 0);
        nbytes = write(STDOUT_FILENO, hex_errno, n);
        err += (nbytes < 0);
      }

      _exit(err?254:255); // exit ok: in child.
    }

    /* Never reached, but avoids compiler warning */
    return status; // LCOV_EXCL_LINE
  }

  /* In parent */

  if (-1 == pid) {
    log_warn(LD_GENERAL, "Failed to fork child process: %s", strerror(errno));
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[0]);
    close(stderr_pipe[1]);
    return status;
  }

  process_handle = process_handle_new();
  process_handle->status = status;
  process_handle->pid = pid;

  /* TODO: If the child process forked but failed to exec, waitpid it */

  /* Return read end of the pipes to caller, and close write end */
  process_handle->stdout_pipe = stdout_pipe[0];
  retval = close(stdout_pipe[1]);

  if (-1 == retval) {
    log_warn(LD_GENERAL,
            "Failed to close write end of stdout pipe in parent process: %s",
            strerror(errno));
  }

  process_handle->waitpid_cb = set_waitpid_callback(pid,
                                                    process_handle_waitpid_cb,
                                                    process_handle);

  process_handle->stderr_pipe = stderr_pipe[0];
  retval = close(stderr_pipe[1]);

  if (-1 == retval) {
    log_warn(LD_GENERAL,
            "Failed to close write end of stderr pipe in parent process: %s",
            strerror(errno));
  }

  /* Return write end of the stdin pipe to caller, and close the read end */
  process_handle->stdin_pipe = stdin_pipe[1];
  retval = close(stdin_pipe[0]);

  if (-1 == retval) {
    log_warn(LD_GENERAL,
            "Failed to close read end of stdin pipe in parent process: %s",
            strerror(errno));
  }

  status = process_handle->status = PROCESS_STATUS_RUNNING;
  /* Set stdin/stdout/stderr pipes to be non-blocking */
  if (fcntl(process_handle->stdout_pipe, F_SETFL, O_NONBLOCK) < 0 ||
      fcntl(process_handle->stderr_pipe, F_SETFL, O_NONBLOCK) < 0 ||
      fcntl(process_handle->stdin_pipe, F_SETFL, O_NONBLOCK) < 0) {
    log_warn(LD_GENERAL, "Failed to set stderror/stdout/stdin pipes "
             "nonblocking in parent process: %s", strerror(errno));
  }

  *process_handle_out = process_handle;
  return status;
#endif /* defined(_WIN32) */
}

/** Destroy all resources allocated by the process handle in
 *  <b>process_handle</b>.
 *  If <b>also_terminate_process</b> is true, also terminate the
 *  process of the process handle. */
MOCK_IMPL(void,
tor_process_handle_destroy,(process_handle_t *process_handle,
                            int also_terminate_process))
{
  if (!process_handle)
    return;

  if (also_terminate_process) {
    if (tor_terminate_process(process_handle) < 0) {
      const char *errstr =
#ifdef _WIN32
        format_win32_error(GetLastError());
#else
        strerror(errno);
#endif
      log_notice(LD_GENERAL, "Failed to terminate process with "
                 "PID '%d' ('%s').", tor_process_get_pid(process_handle),
                 errstr);
    } else {
      log_info(LD_GENERAL, "Terminated process with PID '%d'.",
               tor_process_get_pid(process_handle));
    }
  }

  process_handle->status = PROCESS_STATUS_NOTRUNNING;

#ifdef _WIN32
  if (process_handle->stdout_pipe)
    CloseHandle(process_handle->stdout_pipe);

  if (process_handle->stderr_pipe)
    CloseHandle(process_handle->stderr_pipe);

  if (process_handle->stdin_pipe)
    CloseHandle(process_handle->stdin_pipe);
#else /* !(defined(_WIN32)) */
  close(process_handle->stdout_pipe);
  close(process_handle->stderr_pipe);
  close(process_handle->stdin_pipe);

  clear_waitpid_callback(process_handle->waitpid_cb);
#endif /* defined(_WIN32) */

  memset(process_handle, 0x0f, sizeof(process_handle_t));
  tor_free(process_handle);
}

/** Get the exit code of a process specified by <b>process_handle</b> and store
 * it in <b>exit_code</b>, if set to a non-NULL value.  If <b>block</b> is set
 * to true, the call will block until the process has exited.  Otherwise if
 * the process is still running, the function will return
 * PROCESS_EXIT_RUNNING, and exit_code will be left unchanged. Returns
 * PROCESS_EXIT_EXITED if the process did exit. If there is a failure,
 * PROCESS_EXIT_ERROR will be returned and the contents of exit_code (if
 * non-NULL) will be undefined. N.B. Under *nix operating systems, this will
 * probably not work in Tor, because waitpid() is called in main.c to reap any
 * terminated child processes.*/
int
tor_get_exit_code(process_handle_t *process_handle,
                  int block, int *exit_code)
{
#ifdef _WIN32
  DWORD retval;
  BOOL success;

  if (block) {
    /* Wait for the process to exit */
    retval = WaitForSingleObject(process_handle->pid.hProcess, INFINITE);
    if (retval != WAIT_OBJECT_0) {
      log_warn(LD_GENERAL, "WaitForSingleObject() failed (%d): %s",
              (int)retval, format_win32_error(GetLastError()));
      return PROCESS_EXIT_ERROR;
    }
  } else {
    retval = WaitForSingleObject(process_handle->pid.hProcess, 0);
    if (WAIT_TIMEOUT == retval) {
      /* Process has not exited */
      return PROCESS_EXIT_RUNNING;
    } else if (retval != WAIT_OBJECT_0) {
      log_warn(LD_GENERAL, "WaitForSingleObject() failed (%d): %s",
               (int)retval, format_win32_error(GetLastError()));
      return PROCESS_EXIT_ERROR;
    }
  }

  if (exit_code != NULL) {
    success = GetExitCodeProcess(process_handle->pid.hProcess,
                                 (PDWORD)exit_code);
    if (!success) {
      log_warn(LD_GENERAL, "GetExitCodeProcess() failed: %s",
               format_win32_error(GetLastError()));
      return PROCESS_EXIT_ERROR;
    }
  }
#else /* !(defined(_WIN32)) */
  int stat_loc;
  int retval;

  if (process_handle->waitpid_cb) {
    /* We haven't processed a SIGCHLD yet. */
    retval = waitpid(process_handle->pid, &stat_loc, block?0:WNOHANG);
    if (retval == process_handle->pid) {
      clear_waitpid_callback(process_handle->waitpid_cb);
      process_handle->waitpid_cb = NULL;
      process_handle->waitpid_exit_status = stat_loc;
    }
  } else {
    /* We already got a SIGCHLD for this process, and handled it. */
    retval = process_handle->pid;
    stat_loc = process_handle->waitpid_exit_status;
  }

  if (!block && 0 == retval) {
    /* Process has not exited */
    return PROCESS_EXIT_RUNNING;
  } else if (retval != process_handle->pid) {
    log_warn(LD_GENERAL, "waitpid() failed for PID %d: %s",
             (int)process_handle->pid, strerror(errno));
    return PROCESS_EXIT_ERROR;
  }

  if (!WIFEXITED(stat_loc)) {
    log_warn(LD_GENERAL, "Process %d did not exit normally",
             (int)process_handle->pid);
    return PROCESS_EXIT_ERROR;
  }

  if (exit_code != NULL)
    *exit_code = WEXITSTATUS(stat_loc);
#endif /* defined(_WIN32) */

  return PROCESS_EXIT_EXITED;
}

#ifdef _WIN32
/** Read from a handle <b>h</b> into <b>buf</b>, up to <b>count</b> bytes.  If
 * <b>hProcess</b> is NULL, the function will return immediately if there is
 * nothing more to read. Otherwise <b>hProcess</b> should be set to the handle
 * to the process owning the <b>h</b>. In this case, the function will exit
 * only once the process has exited, or <b>count</b> bytes are read. Returns
 * the number of bytes read, or -1 on error. */
ssize_t
tor_read_all_handle(HANDLE h, char *buf, size_t count,
                    const process_handle_t *process)
{
  size_t numread = 0;
  BOOL retval;
  DWORD byte_count;
  BOOL process_exited = FALSE;

  if (count > SIZE_T_CEILING || count > SSIZE_MAX)
    return -1;

  while (numread < count) {
    /* Check if there is anything to read */
    retval = PeekNamedPipe(h, NULL, 0, NULL, &byte_count, NULL);
    if (!retval) {
      log_warn(LD_GENERAL,
        "Failed to peek from handle: %s",
        format_win32_error(GetLastError()));
      return -1;
    } else if (0 == byte_count) {
      /* Nothing available: process exited or it is busy */

      /* Exit if we don't know whether the process is running */
      if (NULL == process)
        break;

      /* The process exited and there's nothing left to read from it */
      if (process_exited)
        break;

      /* If process is not running, check for output one more time in case
         it wrote something after the peek was performed. Otherwise keep on
         waiting for output */
      tor_assert(process != NULL);
      byte_count = WaitForSingleObject(process->pid.hProcess, 0);
      if (WAIT_TIMEOUT != byte_count)
        process_exited = TRUE;

      continue;
    }

    /* There is data to read; read it */
    retval = ReadFile(h, buf+numread, count-numread, &byte_count, NULL);
    tor_assert(byte_count + numread <= count);
    if (!retval) {
      log_warn(LD_GENERAL, "Failed to read from handle: %s",
        format_win32_error(GetLastError()));
      return -1;
    } else if (0 == byte_count) {
      /* End of file */
      break;
    }
    numread += byte_count;
  }
  return (ssize_t)numread;
}
#else /* !(defined(_WIN32)) */
/** Read from a handle <b>fd</b> into <b>buf</b>, up to <b>count</b> bytes.  If
 * <b>process</b> is NULL, the function will return immediately if there is
 * nothing more to read. Otherwise data will be read until end of file, or
 * <b>count</b> bytes are read.  Returns the number of bytes read, or -1 on
 * error. Sets <b>eof</b> to true if <b>eof</b> is not NULL and the end of the
 * file has been reached. */
ssize_t
tor_read_all_handle(int fd, char *buf, size_t count,
                    const process_handle_t *process,
                    int *eof)
{
  size_t numread = 0;
  ssize_t result;

  if (eof)
    *eof = 0;

  if (count > SIZE_T_CEILING || count > SSIZE_MAX)
    return -1;

  while (numread < count) {
    result = read(fd, buf+numread, count-numread);

    if (result == 0) {
      log_debug(LD_GENERAL, "read() reached end of file");
      if (eof)
        *eof = 1;
      break;
    } else if (result < 0 && errno == EAGAIN) {
      if (process)
        continue;
      else
        break;
    } else if (result < 0) {
      log_warn(LD_GENERAL, "read() failed: %s", strerror(errno));
      return -1;
    }

    numread += result;
  }

  log_debug(LD_GENERAL, "read() read %d bytes from handle", (int)numread);
  return (ssize_t)numread;
}
#endif /* defined(_WIN32) */

/** Read from stdout of a process until the process exits. */
ssize_t
tor_read_all_from_process_stdout(const process_handle_t *process_handle,
                                 char *buf, size_t count)
{
#ifdef _WIN32
  return tor_read_all_handle(process_handle->stdout_pipe, buf, count,
                             process_handle);
#else
  return tor_read_all_handle(process_handle->stdout_pipe, buf, count,
                             process_handle, NULL);
#endif /* defined(_WIN32) */
}

/** Read from stdout of a process until the process exits. */
ssize_t
tor_read_all_from_process_stderr(const process_handle_t *process_handle,
                                 char *buf, size_t count)
{
#ifdef _WIN32
  return tor_read_all_handle(process_handle->stderr_pipe, buf, count,
                             process_handle);
#else
  return tor_read_all_handle(process_handle->stderr_pipe, buf, count,
                             process_handle, NULL);
#endif /* defined(_WIN32) */
}

/** Return a string corresponding to <b>stream_status</b>. */
const char *
stream_status_to_string(enum stream_status stream_status)
{
  switch (stream_status) {
    case IO_STREAM_OKAY:
      return "okay";
    case IO_STREAM_EAGAIN:
      return "temporarily unavailable";
    case IO_STREAM_TERM:
      return "terminated";
    case IO_STREAM_CLOSED:
      return "closed";
    default:
      tor_fragile_assert();
      return "unknown";
  }
}

/** Split buf into lines, and add to smartlist. The buffer <b>buf</b> will be
 * modified. The resulting smartlist will consist of pointers to buf, so there
 * is no need to free the contents of sl. <b>buf</b> must be a NUL-terminated
 * string. <b>len</b> should be set to the length of the buffer excluding the
 * NUL. Non-printable characters (including NUL) will be replaced with "." */
int
tor_split_lines(smartlist_t *sl, char *buf, int len)
{
  /* Index in buf of the start of the current line */
  int start = 0;
  /* Index in buf of the current character being processed */
  int cur = 0;
  /* Are we currently in a line */
  char in_line = 0;

  /* Loop over string */
  while (cur < len) {
    /* Loop until end of line or end of string */
    for (; cur < len; cur++) {
      if (in_line) {
        if ('\r' == buf[cur] || '\n' == buf[cur]) {
          /* End of line */
          buf[cur] = '\0';
          /* Point cur to the next line */
          cur++;
          /* Line starts at start and ends with a nul */
          break;
        } else {
          if (!TOR_ISPRINT(buf[cur]))
            buf[cur] = '.';
        }
      } else {
        if ('\r' == buf[cur] || '\n' == buf[cur]) {
          /* Skip leading vertical space */
          ;
        } else {
          in_line = 1;
          start = cur;
          if (!TOR_ISPRINT(buf[cur]))
            buf[cur] = '.';
        }
      }
    }
    /* We are at the end of the line or end of string. If in_line is true there
     * is a line which starts at buf+start and ends at a NUL. cur points to
     * the character after the NUL. */
    if (in_line)
      smartlist_add(sl, (void *)(buf+start));
    in_line = 0;
  }
  return smartlist_len(sl);
}

#ifdef _WIN32

/** Return a smartlist containing lines outputted from
 *  <b>handle</b>. Return NULL on error, and set
 *  <b>stream_status_out</b> appropriately. */
MOCK_IMPL(smartlist_t *,
tor_get_lines_from_handle, (HANDLE *handle,
                            enum stream_status *stream_status_out))
{
  int pos;
  char stdout_buf[600] = {0};
  smartlist_t *lines = NULL;

  tor_assert(stream_status_out);

  *stream_status_out = IO_STREAM_TERM;

  pos = tor_read_all_handle(handle, stdout_buf, sizeof(stdout_buf) - 1, NULL);
  if (pos < 0) {
    *stream_status_out = IO_STREAM_TERM;
    return NULL;
  }
  if (pos == 0) {
    *stream_status_out = IO_STREAM_EAGAIN;
    return NULL;
  }

  /* End with a null even if there isn't a \r\n at the end */
  /* TODO: What if this is a partial line? */
  stdout_buf[pos] = '\0';

  /* Split up the buffer */
  lines = smartlist_new();
  tor_split_lines(lines, stdout_buf, pos);

  /* Currently 'lines' is populated with strings residing on the
     stack. Replace them with their exact copies on the heap: */
  SMARTLIST_FOREACH(lines, char *, line,
                    SMARTLIST_REPLACE_CURRENT(lines, line, tor_strdup(line)));

  *stream_status_out = IO_STREAM_OKAY;

  return lines;
}

#else /* !(defined(_WIN32)) */

/** Return a smartlist containing lines outputted from
 *  <b>fd</b>. Return NULL on error, and set
 *  <b>stream_status_out</b> appropriately. */
MOCK_IMPL(smartlist_t *,
tor_get_lines_from_handle, (int fd, enum stream_status *stream_status_out))
{
  enum stream_status stream_status;
  char stdout_buf[400];
  smartlist_t *lines = NULL;

  while (1) {
    memset(stdout_buf, 0, sizeof(stdout_buf));

    stream_status = get_string_from_pipe(fd,
                                         stdout_buf, sizeof(stdout_buf) - 1);
    if (stream_status != IO_STREAM_OKAY)
      goto done;

    if (!lines) lines = smartlist_new();
    smartlist_split_string(lines, stdout_buf, "\n", 0, 0);
  }

 done:
  *stream_status_out = stream_status;
  return lines;
}

#endif /* defined(_WIN32) */

/** Reads from <b>fd</b> and stores input in <b>buf_out</b> making
 *  sure it's below <b>count</b> bytes.
 *  If the string has a trailing newline, we strip it off.
 *
 * This function is specifically created to handle input from managed
 * proxies, according to the pluggable transports spec. Make sure it
 * fits your needs before using it.
 *
 * Returns:
 * IO_STREAM_CLOSED: If the stream is closed.
 * IO_STREAM_EAGAIN: If there is nothing to read and we should check back
 *  later.
 * IO_STREAM_TERM: If something is wrong with the stream.
 * IO_STREAM_OKAY: If everything went okay and we got a string
 *  in <b>buf_out</b>. */
enum stream_status
get_string_from_pipe(int fd, char *buf_out, size_t count)
{
  ssize_t ret;

  tor_assert(count <= INT_MAX);

  ret = read(fd, buf_out, count);

  if (ret == 0)
    return IO_STREAM_CLOSED;
  else if (ret < 0 && errno == EAGAIN)
    return IO_STREAM_EAGAIN;
  else if (ret < 0)
    return IO_STREAM_TERM;

  if (buf_out[ret - 1] == '\n') {
    /* Remove the trailing newline */
    buf_out[ret - 1] = '\0';
  } else
    buf_out[ret] = '\0';

  return IO_STREAM_OKAY;
}
