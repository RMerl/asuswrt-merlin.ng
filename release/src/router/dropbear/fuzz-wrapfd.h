#ifndef FUZZ_WRAPFD_H
#define FUZZ_WRAPFD_H

#include "buffer.h"

enum wrapfd_mode {
    UNUSED = 0,
    PLAIN,
    INPROGRESS,
    RANDOMIN
};

void wrapfd_setup(void);
void wrapfd_setseed(uint32_t seed);
// doesn't take ownership of buf. buf is optional.
void wrapfd_add(int fd, buffer *buf, enum wrapfd_mode mode);

// called via #defines for read/write/select
int wrapfd_read(int fd, void *out, size_t count);
int wrapfd_write(int fd, const void* in, size_t count);
int wrapfd_select(int nfds, fd_set *readfds, fd_set *writefds, 
    fd_set *exceptfds, struct timeval *timeout);
int wrapfd_close(int fd);

#endif // FUZZ_WRAPFD_H
