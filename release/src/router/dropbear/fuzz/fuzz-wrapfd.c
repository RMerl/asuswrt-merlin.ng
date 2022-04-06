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
	int closein;
	int closeout;
};

static struct fdwrap wrap_fds[IOWRAP_MAXFD+1] = {{UNUSED, 0, 0}};
static int wrapfd_maxfd = -1;
static unsigned short rand_state[3];
static buffer *input_buf;
static int devnull_fd = -1;

static void wrapfd_remove(int fd);

void wrapfd_setup(buffer *buf) {
	TRACE(("wrapfd_setup"))

	// clean old ones
	int i;
	for (i = 0; i <= wrapfd_maxfd; i++) {
		if (wrap_fds[i].mode != UNUSED) {
			wrapfd_remove(i);
		}
	}
	wrapfd_maxfd = -1;

	memset(rand_state, 0x0, sizeof(rand_state));
	wrapfd_setseed(50);
	input_buf = buf;
}

void wrapfd_setseed(uint32_t seed) {
	memcpy(rand_state, &seed, sizeof(seed));
	nrand48(rand_state);
}

int wrapfd_new_fuzzinput() {
	if (devnull_fd == -1) {
		devnull_fd = open("/dev/null", O_RDONLY);
		assert(devnull_fd != -1);
	}

	int fd = dup(devnull_fd);
	assert(fd != -1);
	assert(wrap_fds[fd].mode == UNUSED);
	wrap_fds[fd].mode = COMMONBUF;
	wrap_fds[fd].closein = 0;
	wrap_fds[fd].closeout = 0;
	wrapfd_maxfd = MAX(fd, wrapfd_maxfd);

	return fd;
}

int wrapfd_new_dummy() {
	if (devnull_fd == -1) {
		devnull_fd = open("/dev/null", O_RDONLY);
		assert(devnull_fd != -1);
	}

	int fd = dup(devnull_fd);
	if (fd == -1) {
		return -1;
	}
	if (fd > IOWRAP_MAXFD) {
		close(fd);
		errno = EMFILE;
		return -1;
	}
	assert(wrap_fds[fd].mode == UNUSED);
	wrap_fds[fd].mode = DUMMY;
	wrap_fds[fd].closein = 0;
	wrap_fds[fd].closeout = 0;
	wrapfd_maxfd = MAX(fd, wrapfd_maxfd);

	return fd;
}


static void wrapfd_remove(int fd) {
	TRACE(("wrapfd_remove %d", fd))
	assert(fd >= 0);
	assert(fd <= IOWRAP_MAXFD);
	assert(wrap_fds[fd].mode != UNUSED);
	wrap_fds[fd].mode = UNUSED;
	close(fd);
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

	if (input_buf && wrap_fds[fd].mode == COMMONBUF) {
		maxread = MIN(input_buf->len - input_buf->pos, count);
		/* returns 0 if buf is EOF, as intended */
		if (maxread > 0) {
			maxread = nrand48(rand_state) % maxread + 1;
		}
		memcpy(out, buf_getptr(input_buf, maxread), maxread);
		buf_incrpos(input_buf, maxread);
		return maxread;
	}

	// return fixed output, of random length
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

int fuzz_kill(pid_t pid, int sig) {
	if (fuzz.fuzzing) {
		TRACE(("fuzz_kill ignoring pid %d signal %d", (pid), sig))
		if (sig >= 0) {
			return 0;
		} else {
			errno = EINVAL;
			return -1;
		}
	}
	return kill(pid, sig);
}
