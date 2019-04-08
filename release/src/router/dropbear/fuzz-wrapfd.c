#define FUZZ_SKIP_WRAP 1
#include "includes.h"
#include "fuzz-wrapfd.h"

#include "dbutil.h"

#include "fuzz.h"

#define IOWRAP_MAXFD (FD_SETSIZE-1)
static const int MAX_RANDOM_IN = 50000;
static const double CHANCE_CLOSE = 1.0 / 600;
static const double CHANCE_INTR = 1.0 / 900;
static const double CHANCE_READ1 = 0.96;
static const double CHANCE_READ2 = 0.5;
static const double CHANCE_WRITE1 = 0.96;
static const double CHANCE_WRITE2 = 0.5;

struct fdwrap {
	enum wrapfd_mode mode;
	buffer *buf;
	int closein;
	int closeout;
};

static struct fdwrap wrap_fds[IOWRAP_MAXFD+1];
/* for quick selection of in-use descriptors */
static int wrap_used[IOWRAP_MAXFD+1];
static unsigned int nused;
static unsigned short rand_state[3];

void wrapfd_setup(void) {
	TRACE(("wrapfd_setup"))
	nused = 0;
	memset(wrap_fds, 0x0, sizeof(wrap_fds));
	memset(wrap_used, 0x0, sizeof(wrap_used));

	memset(rand_state, 0x0, sizeof(rand_state));
	wrapfd_setseed(50);
}

void wrapfd_setseed(uint32_t seed) {
	memcpy(rand_state, &seed, sizeof(seed));
	nrand48(rand_state);
}

void wrapfd_add(int fd, buffer *buf, enum wrapfd_mode mode) {
	TRACE(("wrapfd_add %d buf %p mode %d", fd, buf, mode))
	assert(fd >= 0);
	assert(fd <= IOWRAP_MAXFD);
	assert(wrap_fds[fd].mode == UNUSED);
	assert(buf || mode == RANDOMIN);

	wrap_fds[fd].mode = mode;
	wrap_fds[fd].buf = buf;
	wrap_fds[fd].closein = 0;
	wrap_fds[fd].closeout = 0;
	wrap_used[nused] = fd;

	nused++;
}

void wrapfd_remove(int fd) {
	unsigned int i, j;
	TRACE(("wrapfd_remove %d", fd))
	assert(fd >= 0);
	assert(fd <= IOWRAP_MAXFD);
	assert(wrap_fds[fd].mode != UNUSED);
	wrap_fds[fd].mode = UNUSED;


	/* remove from used list */
	for (i = 0, j = 0; i < nused; i++) {
		if (wrap_used[i] != fd) {
			wrap_used[j] = wrap_used[i];
			j++;
		}
	}
	nused--;
}

int wrapfd_close(int fd) {
	if (fd >= 0 && fd <= IOWRAP_MAXFD && wrap_fds[fd].mode != UNUSED) {
		wrapfd_remove(fd);
		return 0;
	} else {
		return close(fd);
	}
}

int wrapfd_read(int fd, void *out, size_t count) {
	size_t maxread;
	buffer *buf;

	if (!fuzz.wrapfds) {
		return read(fd, out, count);
	}

	if (fd < 0 || fd > IOWRAP_MAXFD || wrap_fds[fd].mode == UNUSED) {
		/* XXX - assertion failure? */
		TRACE(("Bad read descriptor %d\n", fd))
		errno = EBADF;
		return -1;
	}

	assert(count != 0);

	if (wrap_fds[fd].closein || erand48(rand_state) < CHANCE_CLOSE) {
		wrap_fds[fd].closein = 1;
		errno = ECONNRESET;
		return -1;
	}

	if (erand48(rand_state) < CHANCE_INTR) {
		errno = EINTR;
		return -1;
	}

	buf = wrap_fds[fd].buf;
	if (buf) {
		maxread = MIN(buf->len - buf->pos, count);
		/* returns 0 if buf is EOF, as intended */
		if (maxread > 0) {
			maxread = nrand48(rand_state) % maxread + 1;
		}
		memcpy(out, buf_getptr(buf, maxread), maxread);
		buf_incrpos(buf, maxread);
		return maxread;
	}

	maxread = MIN(MAX_RANDOM_IN, count);
	maxread = nrand48(rand_state) % maxread + 1;
	memset(out, 0xef, maxread);
	return maxread;
}

int wrapfd_write(int fd, const void* in, size_t count) {
	unsigned const volatile char* volin = in;
	unsigned int i;

	if (!fuzz.wrapfds) {
		return write(fd, in, count);
	}

	if (fd < 0 || fd > IOWRAP_MAXFD || wrap_fds[fd].mode == UNUSED) {
		/* XXX - assertion failure? */
		TRACE(("Bad read descriptor %d\n", fd))
		errno = EBADF;
		return -1;
	}

	assert(count != 0);

	/* force read to exercise sanitisers */
	for (i = 0; i < count; i++) {
		(void)volin[i];
	}

	if (wrap_fds[fd].closeout || erand48(rand_state) < CHANCE_CLOSE) {
		wrap_fds[fd].closeout = 1;
		errno = ECONNRESET;
		return -1;
	}

	if (erand48(rand_state) < CHANCE_INTR) {
		errno = EINTR;
		return -1;
	}

	return nrand48(rand_state) % (count+1);
}

int wrapfd_select(int nfds, fd_set *readfds, fd_set *writefds, 
	fd_set *exceptfds, struct timeval *timeout) {
	int i, nset, sel;
	int ret = 0;
	int fdlist[IOWRAP_MAXFD+1];

	memset(fdlist, 0x0, sizeof(fdlist));

	if (!fuzz.wrapfds) {
		return select(nfds, readfds, writefds, exceptfds, timeout);
	}

	assert(nfds <= IOWRAP_MAXFD+1);

	if (erand48(rand_state) < CHANCE_INTR) {
		errno = EINTR;
		return -1;
	}

	/* read */
	if (readfds != NULL && erand48(rand_state) < CHANCE_READ1) {
		for (i = 0, nset = 0; i < nfds; i++) {
			if (FD_ISSET(i, readfds)) {
				assert(wrap_fds[i].mode != UNUSED);
				fdlist[nset] = i;
				nset++;
			}
		}
		DROPBEAR_FD_ZERO(readfds);

		if (nset > 0) {
			/* set one */
			sel = fdlist[nrand48(rand_state) % nset];
			FD_SET(sel, readfds);
			ret++;

			if (erand48(rand_state) < CHANCE_READ2) {
				sel = fdlist[nrand48(rand_state) % nset];
				if (!FD_ISSET(sel, readfds)) {
					FD_SET(sel, readfds);
					ret++;
				}
			}
		}
	}

	/* write */
	if (writefds != NULL && erand48(rand_state) < CHANCE_WRITE1) {
		for (i = 0, nset = 0; i < nfds; i++) {
			if (FD_ISSET(i, writefds)) {
				assert(wrap_fds[i].mode != UNUSED);
				fdlist[nset] = i;
				nset++;
			}
		}
		DROPBEAR_FD_ZERO(writefds);

		/* set one */
		if (nset > 0) {
			sel = fdlist[nrand48(rand_state) % nset];
			FD_SET(sel, writefds);
			ret++;

			if (erand48(rand_state) < CHANCE_WRITE2) {
				sel = fdlist[nrand48(rand_state) % nset];
				if (!FD_ISSET(sel, writefds)) {
					FD_SET(sel, writefds);
					ret++;
				}
			}
		}
	}
	return ret;
}

