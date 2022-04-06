#ifndef FUZZ_WRAPFD_H
#define FUZZ_WRAPFD_H

#include "includes.h"
#include "buffer.h"

enum wrapfd_mode {
    UNUSED = 0,
    COMMONBUF, // using the common buffer
    DUMMY, // reads return fixed output, of random length
};

// buf is a common buffer read by all wrapped FDs. doesn't take ownership of buf
void wrapfd_setup(buffer *buf);
void wrapfd_setseed(uint32_t seed);
int wrapfd_new_fuzzinput(void);
int wrapfd_new_dummy(void);

// called via #defines for read/write/select
int wrapfd_read(int fd, void *out, size_t count);
int wrapfd_write(int fd, const void* in, size_t count);
int wrapfd_select(int nfds, fd_set *readfds, fd_set *writefds, 
    fd_set *exceptfds, struct timeval *timeout);
int wrapfd_close(int fd);
int fuzz_kill(pid_t pid, int sig);

#endif // FUZZ_WRAPFD_H
