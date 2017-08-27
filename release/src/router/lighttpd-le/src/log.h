#ifndef _LOG_H_
#define _LOG_H_
#include "first.h"

#include "server.h"

struct timespec; /* declaration */
int log_clock_gettime_realtime (struct timespec *ts);

ssize_t write_all(int fd, const void* buf, size_t count);

/* Close fd and _try_ to get a /dev/null for it instead.
 * Returns 0 on success and -1 on failure (fd gets closed in all cases)
 */
int openDevNull(int fd);

int open_logfile_or_pipe(server *srv, const char* logfile);

int log_error_open(server *srv);
int log_error_close(server *srv);
int log_error_write(server *srv, const char *filename, unsigned int line, const char *fmt, ...);
int log_error_write_multiline_buffer(server *srv, const char *filename, unsigned int line, buffer *multiline, const char *fmt, ...);
int log_error_cycle(server *srv);

#endif
