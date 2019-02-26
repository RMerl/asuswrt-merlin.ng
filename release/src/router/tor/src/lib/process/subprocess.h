/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file subprocess.h
 * \brief Header for subprocess.c
 **/

#ifndef TOR_SUBPROCESS_H
#define TOR_SUBPROCESS_H

#include "lib/cc/torint.h"
#include "lib/testsupport/testsupport.h"
#include <stddef.h>
#ifdef _WIN32
#include <windows.h>
#endif

struct smartlist_t;

void tor_disable_spawning_background_processes(void);

typedef struct process_handle_t process_handle_t;
struct process_environment_t;
int tor_spawn_background(const char *const filename, const char **argv,
                         struct process_environment_t *env,
                         process_handle_t **process_handle_out);

#define SPAWN_ERROR_MESSAGE "ERR: Failed to spawn background process - code "

/** Status of an I/O stream. */
enum stream_status {
  IO_STREAM_OKAY,
  IO_STREAM_EAGAIN,
  IO_STREAM_TERM,
  IO_STREAM_CLOSED
};

const char *stream_status_to_string(enum stream_status stream_status);

enum stream_status get_string_from_pipe(int fd, char *buf, size_t count);

/* Values of process_handle_t.status. */
#define PROCESS_STATUS_NOTRUNNING 0
#define PROCESS_STATUS_RUNNING 1
#define PROCESS_STATUS_ERROR -1

#ifdef SUBPROCESS_PRIVATE
struct waitpid_callback_t;

/** Structure to represent the state of a process with which Tor is
 * communicating. The contents of this structure are private to util.c */
struct process_handle_t {
  /** One of the PROCESS_STATUS_* values */
  int status;
#ifdef _WIN32
  HANDLE stdin_pipe;
  HANDLE stdout_pipe;
  HANDLE stderr_pipe;
  PROCESS_INFORMATION pid;
#else /* !(defined(_WIN32)) */
  int stdin_pipe;
  int stdout_pipe;
  int stderr_pipe;
  pid_t pid;
  /** If the process has not given us a SIGCHLD yet, this has the
   * waitpid_callback_t that gets invoked once it has. Otherwise this
   * contains NULL. */
  struct waitpid_callback_t *waitpid_cb;
  /** The exit status reported by waitpid. */
  int waitpid_exit_status;
#endif /* defined(_WIN32) */
};
#endif /* defined(SUBPROCESS_PRIVATE) */

/* Return values of tor_get_exit_code() */
#define PROCESS_EXIT_RUNNING 1
#define PROCESS_EXIT_EXITED 0
#define PROCESS_EXIT_ERROR -1
int tor_get_exit_code(process_handle_t *process_handle,
                      int block, int *exit_code);
int tor_split_lines(struct smartlist_t *sl, char *buf, int len);
#ifdef _WIN32
ssize_t tor_read_all_handle(HANDLE h, char *buf, size_t count,
                            const process_handle_t *process);
#else
ssize_t tor_read_all_handle(int fd, char *buf, size_t count,
                            const process_handle_t *process,
                            int *eof);
#endif /* defined(_WIN32) */
ssize_t tor_read_all_from_process_stdout(
    const process_handle_t *process_handle, char *buf, size_t count);
ssize_t tor_read_all_from_process_stderr(
    const process_handle_t *process_handle, char *buf, size_t count);
char *tor_join_win_cmdline(const char *argv[]);

int tor_process_get_pid(process_handle_t *process_handle);
#ifdef _WIN32
HANDLE tor_process_get_stdout_pipe(process_handle_t *process_handle);
#else
int tor_process_get_stdout_pipe(process_handle_t *process_handle);
#endif

#ifdef _WIN32
MOCK_DECL(struct smartlist_t *, tor_get_lines_from_handle,(HANDLE *handle,
                           enum stream_status *stream_status));
#else
MOCK_DECL(struct smartlist_t *, tor_get_lines_from_handle,(int fd,
                                       enum stream_status *stream_status));
#endif /* defined(_WIN32) */

int tor_terminate_process(process_handle_t *process_handle);

MOCK_DECL(void, tor_process_handle_destroy,(process_handle_t *process_handle,
                                            int also_terminate_process));

#ifdef SUBPROCESS_PRIVATE
/* Prototypes for private functions only used by util.c (and unit tests) */

#ifndef _WIN32
STATIC int format_helper_exit_status(unsigned char child_state,
                              int saved_errno, char *hex_errno);

/* Space for hex values of child state, a slash, saved_errno (with
   leading minus) and newline (no null) */
#define HEX_ERRNO_SIZE (sizeof(char) * 2 + 1 + \
                        1 + sizeof(int) * 2 + 1)
#endif /* !defined(_WIN32) */

#endif /* defined(SUBPROCESS_PRIVATE) */

#endif
