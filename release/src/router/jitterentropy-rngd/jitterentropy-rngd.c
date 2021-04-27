/*
 * Non-physical true random number generator based on timing jitter.
 *
 * Copyright Stephan Mueller <smueller@chronox.de>, 2014 - 2021
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * the GNU General Public License, in which case the provisions of the GPL are
 * required INSTEAD OF the above restrictions.  (This clause is
 * necessary due to a potential bad interaction between the GPL and
 * the restrictions contained in a BSD-style copyright.)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <asm/types.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/random.h>
#include <linux/version.h>
#include <signal.h>

#include "jitterentropy.h"

#define MAJVERSION 1 /* API / ABI incompatible changes, functional changes that
		      * require consumer to be updated (as long as this number
		      * is zero, the API is not considered stable and can
		      * change without a bump of the major version) */
#define MINVERSION 2 /* API compatible, ABI may change, functional
		      * enhancements only, consumer can be left unchanged if
		      * enhancements are not considered */
#define PATCHLEVEL 2 /* API / ABI compatible, no functional changes, no
		      * enhancements, bug fixes only */

static int Verbosity = 0;
static int force_sp80090b = 0;

struct kernel_rng {
	int fd;
	struct rand_data *ec;
	struct rand_pool_info *rpi;
	const char *dev;
};

static struct kernel_rng Random = {
	/*.fd = */ -1,
	/*.ec = */ NULL,
	/*.rpi = */ NULL,
	/*.dev = */ "/dev/random"
};

/*
 * handler for /dev/urandom not needed as used IOCTL alters input_pool
static struct kernel_rng Urandom = {
	.fd = 0,
	.ec = NULL,
	.rpi = NULL,
	.dev = "/dev/urandom"
};
*/

static int Pidfile_fd = -1;
/* "/var/run/jitterentropy-rngd.pid" */
static char *Pidfile = NULL;

static int Entropy_avail_fd = -1;

#define ENTROPYBYTES 32
#define OVERSAMPLINGFACTOR 2
#define ENTROPYTHRESH 1024
/*
 * After FORCE_RESEED_WAKEUPS, the installed alarm handler will unconditionally
 * trigger a reseed irrespective of the seed level. This ensures that new
 * seed is added after FORCE_RESEED_WAKEUPS * (alarm period defined in
 * install_alarm) == 120 * 5 == 600s.
 */
#define FORCE_RESEED_WAKEUPS	120
#define ENTROPYAVAIL "/proc/sys/kernel/random/entropy_avail"
#define LRNG_FILE "/proc/lrng_type"

static void install_alarm(void);
static void dealloc(void);
static void dealloc_rng(struct kernel_rng *rng);

static unsigned long kern_maj = ULONG_MAX, kern_minor, kern_patchlevel;

static void jentrng_versionstring(char *buf, size_t buflen)
{
	snprintf(buf, buflen, "jitterentropy-rngd %d.%d.%d",
		 MAJVERSION, MINVERSION, PATCHLEVEL);
}

/* Is the LRNG present instead of the legacy /dev/random? */
static int lrng_present(void)
{
	struct stat buf;
	static int lrng_present = -1;

	if (lrng_present < 0) {
		int ret = stat(LRNG_FILE, &buf);

		if (ret == -1 && errno == ENOENT)
			lrng_present = 0;
		else
			lrng_present = 1;
	}

	return lrng_present;
}

static int get_kernver(void)
{
	struct utsname kernel;
	char *saveptr = NULL;
	char *res = NULL;

	if (kern_maj != ULONG_MAX)
		return 0;

	if (uname(&kernel))
		return -errno;

	/* 5.11.2 */
	res = strtok_r(kernel.release, ".", &saveptr);
	if (!res) {
		printf("Could not parse kernel version");
		return -EFAULT;
	}
	kern_maj = strtoul(res, NULL, 10);

	res = strtok_r(NULL, ".", &saveptr);
	if (!res) {
		printf("Could not parse kernel version");
		return -EFAULT;
	}
	kern_minor = strtoul(res, NULL, 10);

	res = strtok_r(NULL, ".", &saveptr);
	if (!res) {
		printf("Could not parse kernel version");
		return -EFAULT;
	}
	kern_patchlevel = strtoul(res, NULL, 10);

	return 0;
}

/* return true if kernel is greater or equal to given values, otherwise false */
static int kernver_ge(unsigned int maj, unsigned int minor,
		      unsigned int patchlevel)
{
	if (get_kernver())
		return 0;

	if (maj < kern_maj)
		return 1;
	if (maj == kern_maj) {
		if (minor < kern_minor)
			return 1;
		if (minor == kern_minor) {
			if (patchlevel <= kern_patchlevel)
				return 1;
		}
	}
	return 0;
}

static void usage(void)
{
	unsigned int ver = jent_version();
	char version[30];

	memset(version, 0, 30);
	jentrng_versionstring(version, sizeof(version));

	fprintf(stderr, "\njitterentropy rngd feeding entropy to input_pool of Linux RNG\n");
	fprintf(stderr, "Version %s\n\n", version);
	fprintf(stderr, "Reported numeric version number of jent library %u\n\n", ver);
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "\t-h --help\tThis help information\n");
	fprintf(stderr, "\t   --version\tPrint version\n");
	fprintf(stderr, "\t-v --verbose\tVerbose logging, multiple options increase verbosity\n");
	fprintf(stderr, "\t\t\tVerbose logging implies running in foreground\n");
	fprintf(stderr, "\t-p --pid\tWrite daemon PID to file\n");
	fprintf(stderr, "\t-s --sp800-90b\tForce SP800-90B compliance\n");
	fprintf(stderr, "LRNG presence %sdetected\n",
		lrng_present() ? "" : "not ");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	int c = 0;
	char version[30];

	while (1) {
		int opt_index = 0;
		static struct option opts[] = {
			{"verbose", 0, 0, 0},
			{"pid", 1, 0, 0},
			{"help", 0, 0, 0},
			{"version", 0, 0, 0},
			{"sp800-90b", 0, 0, 0},
			{0, 0, 0, 0}
		};
		c = getopt_long(argc, argv, "svp:h", opts, &opt_index);
		if (-1 == c)
			break;
		switch (c) {
		case 0:
			switch (opt_index) {
			case 0:
				Verbosity++;
				break;
			case 1:
				Pidfile = optarg;
				break;
			case 2:
				usage();
				break;
			case 3:
				jentrng_versionstring(version, sizeof(version));
				fprintf(stderr, "Version %s\n", version);
				fprintf(stderr, "Version Jitterentropy Core %u\n", jent_version());
				exit(0);
				break;
			case 4:
				force_sp80090b = 1;
				break;
			default:
				usage();
			}
			break;
		case 'v':
			Verbosity++;
			break;
		case 'p':
			Pidfile = optarg;
			break;
		case 'h':
			usage();
			break;
		case 's':
			force_sp80090b = 1;
			break;
		default:
			usage();
		}
	}
}

#define LOG_DEBUG	3
#define LOG_VERBOSE	2
#define LOG_WARN	1
#define LOG_ERR		0
static void dolog(int severity, const char *fmt, ...)
{
	va_list args;
	char msg[1024];
	char sev[10];

	if (severity <= Verbosity) {
		va_start(args, fmt);
		vsnprintf(msg, sizeof(msg), fmt, args);
		va_end(args);

		switch (severity) {
		case LOG_DEBUG:
			snprintf(sev, sizeof(sev), "Debug");
			break;
		case LOG_VERBOSE:
			snprintf(sev, sizeof(sev), "Verbose");
			break;
		case LOG_WARN:
			snprintf(sev, sizeof(sev), "Warning");
			break;
		case LOG_ERR:
			snprintf(sev, sizeof(sev), "Error");
			break;
		default:
			snprintf(sev, sizeof(sev), "Unknown");
		}
		printf("jitterentropy-rngd - %s: %s\n", sev, msg);
	}

	if (LOG_ERR == severity) {
		dealloc();
		exit(1);
	}
}

static inline void memset_secure(void *s, int c, size_t n)
{
	memset(s, c, n);
	__asm__ __volatile__("" : : "r" (s) : "memory");
}

/*******************************************************************
 * entropy handler functions
 *******************************************************************/

static size_t write_random(struct kernel_rng *rng, char *buf, size_t len,
			   size_t entropy_bytes, int force_reseed)
{
	size_t written = 0;
	int ret;

	 /* value is in bits */
	rng->rpi->entropy_count = (entropy_bytes * 8);
	rng->rpi->buf_size = len;
	memcpy(rng->rpi->buf, buf, len);

	ret = ioctl(rng->fd, RNDADDENTROPY, rng->rpi);
	if (0 > ret)
		dolog(LOG_WARN, "Error injecting entropy: %s", strerror(errno));
	else {
		dolog(LOG_DEBUG, "Injected %u bytes with an entropy count of %u bytes of entropy",
		      len, entropy_bytes);
		written = len;
	}

	rng->rpi->entropy_count = 0;
	rng->rpi->buf_size = 0;
	memset(rng->rpi->buf, 0, len);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
	/*
	 * The LRNG does not require this IOCTL as the reseed is automatically
	 * triggered.
	 */
	if (force_reseed && kernver_ge(4, 17, 0) &&
	    !lrng_present() &&
	    ioctl(rng->fd, RNDRESEEDCRNG) < 0 && errno != EINVAL) {
		dolog(LOG_WARN,
		      "Error triggering a reseed of the kernel DRNG: %s\n",
		      strerror(errno));
	}
#endif

	return written;
}

/*
 * Inject the data 90B-compliant considering the minimum n_out of 80 bits
 * of the folded SHA-1 operation reading the input_pool.
 *
 * The following seeding strategy is applied to ensure SP800-90B compliance:
 *
 * - If the LRNG is present, 90B compliance is always given and no special
 * handling is needed.
 *
 * - If the default /dev/random implementation is provided and the kernel offers
 * the RNDRESEEDCRNG, use it after injecting 80 bits of entropy to feed
 * the entropy into the ChaCha20 DRNG. In this case, the caller should use
 * the getrandom(2) system call or /dev/urandom to get SP800-90B compliant
 * data.
 *
 * - Kernels without the RNDRESEEDCRNG will never offer SP800-90B compliant
 * data via /dev/urandom or getrandom(2). Those should always use /dev/random.
 * In this case, the Jitter-RNG will feed only 80 bit chunks into the kernel.
 * This means that after /dev/random consumed 80 bits, new data is requested
 * from the Jitter-RNG.
 */
#define SHA1_FOLD_OUTPUT_SIZE	10
static size_t write_random_90B(struct kernel_rng *rng, char *buf, size_t len,
			       size_t entropy_bytes, int force_reseed)
{
	size_t written = 0, ptr;

	if (!force_reseed)
		return write_random(rng, buf, len, entropy_bytes, force_reseed);

	for (ptr = 0; ptr < len; ptr += SHA1_FOLD_OUTPUT_SIZE) {
		size_t todo = len - ptr, ent;

		if (todo > SHA1_FOLD_OUTPUT_SIZE)
			todo = SHA1_FOLD_OUTPUT_SIZE;

		ent = todo;
		if (ent > entropy_bytes)
			ent = entropy_bytes;
		entropy_bytes -= ent;

		written += write_random(rng, buf + ptr, todo, ent,
					force_reseed);
	}

	return written;
}

static ssize_t read_jent(struct kernel_rng *rng, char *buf, size_t buflen)
{
	unsigned int i;
	ssize_t ret = jent_read_entropy(rng->ec, buf, buflen);

	if (ret >= 0)
		return ret;

	dolog(LOG_WARN, "Cannot read entropy");

	/* Only catch the FIPS test failures in the loop below */
	if (ret != -2 && ret != -3)
		return ret;

	for (i = 1; i <= 10; i++) {
		dolog(LOG_WARN,
		      "Re-allocation attempt %u to clear permanent Jitter RNG error",
		      i);

		jent_entropy_collector_free(rng->ec);
		rng->ec = jent_entropy_collector_alloc(1, 0);
		if (!rng->ec) {
			dolog(LOG_WARN,
			      "Allocation of entropy collector failed");
		} else {
			ret = jent_read_entropy(rng->ec, buf, buflen);
			if (ret >= 0)
				return ret;

			dolog(LOG_WARN, "Cannot read entropy");

			if (ret != -2 && ret != -3)
				return ret;
		}
	}

	dolog(LOG_ERR,
	      "Failed to allocate new Jitter RNG instance and obtain entropy");

	return -EFAULT;
}

static size_t gather_entropy(struct kernel_rng *rng, int init)
{
	sigset_t blocking_set, previous_set;
#define ENTBLOCKSIZE	(ENTROPYBYTES * OVERSAMPLINGFACTOR)
/*
 * Maximum numbers of blocks is determined by numbers of reseed IOCTLs: if
 * the reseed IOCTL is used, we call ceil(256 / 80) numbers of IOCTLs. As
 * each IOCTL may drain the entropy pool by 256 bits, we need to ensure that
 * after the numbers of IOCTLs, we finally inject more blocks than the numbers
 * of IOCTLs into the input_pool. Otherwise the entropy estimator will never
 * rise and we encounter an endless loop.
 */
#define ENTBLOCKS	(4 + 2 + 1)
	char buf[(ENTBLOCKSIZE * ENTBLOCKS)];
	size_t buflen = ENTBLOCKSIZE;
	size_t ret = 0;

	sigemptyset(&previous_set);
	sigemptyset(&blocking_set);
	sigaddset(&blocking_set, SIGALRM);

	sigprocmask(SIG_BLOCK, &blocking_set, &previous_set);

	if (lrng_present()) {
		/*
		 * The LRNG operates fully 90B compliant, no special handling
		 * is necessary.
		 */
		if (read_jent(rng, buf, buflen) < 0)
			return 0;

		/* LRNG seeds automatically */
		ret = write_random(rng, buf, buflen, ENTROPYBYTES, 0);
	} else if (kernver_ge(4, 17, 0)) {
		unsigned int numblocks = 1, i;

		if (force_sp80090b || init) {
			numblocks = ENTBLOCKS;
			buflen *= numblocks;
		}

		/*
		 * Generate twice the entropy data, once for the input_pool
		 * and once for ChaCha20.
		 */
		if (read_jent(rng, buf, buflen) < 0)
			return 0;

		dolog(LOG_DEBUG, "Inject entropy into %s",
		      force_sp80090b ? "ChaCha20 DRNG" : "input pool");
		ret = write_random_90B(rng, buf, ENTBLOCKSIZE, ENTROPYBYTES,
				       force_sp80090b || init);
		numblocks--;

		for (i = 0; i < numblocks; i++) {
			dolog(LOG_DEBUG, "Inject entropy into input_pool");
			ret += write_random_90B(rng, buf + ENTBLOCKSIZE * i,
						ENTBLOCKSIZE, ENTROPYBYTES, 0);
		}
	} else {
		if (force_sp80090b)
			buflen = SHA1_FOLD_OUTPUT_SIZE;

		if (read_jent(rng, buf, buflen) < 0)
			return 0;

		ret = write_random_90B(rng, buf, buflen,
				       buflen / OVERSAMPLINGFACTOR, 0);
	}

	if (buflen != ret) {
		dolog(LOG_WARN, "Injected %lu bytes into %s, expected %d",
		      ret, rng->dev, buflen);
		ret = 0;
	}
	memset_secure(buf, 0, buflen);

	sigprocmask(SIG_SETMASK, &previous_set, NULL);

	return ret;
}

static int read_entropy_avail(int fd)
{
	ssize_t data = 0;
	char buf[5];
	int entropy = 0;

	data = read(fd, buf, sizeof(buf));
	lseek(fd, 0, SEEK_SET);

	if (0 > data) {
		dolog(LOG_WARN, "Error reading data from entropy_avail: %s",
		      strerror(errno));
		return 0;
	}
	if (0 == data) {
		dolog(LOG_WARN, "Could not read data from entropy_avail");
		return 0;
	}

	entropy = atoi(buf);
	if (0 > entropy || 4096 < entropy) {
		dolog(LOG_WARN, "Entropy read from entropy_avail (%d) is outsize of range", entropy);
		return 0;
	}

	return entropy;
}

/*******************************************************************
 * Signal handling functions
 *******************************************************************/

/*
 * Wakeup and check entropy_avail -- this covers the drain of entropy
 * from the nonblocking_pool via get_random_bytes
 */
static void sig_entropy_avail(int sig)
{
	int entropy = 0;
	size_t written = 0;
	static unsigned int force_reseed = FORCE_RESEED_WAKEUPS;

	(void)sig;

	dolog(LOG_VERBOSE, "Wakeup call for alarm on %s", ENTROPYAVAIL);

	if (--force_reseed == 0) {
		force_reseed = FORCE_RESEED_WAKEUPS;
		dolog(LOG_DEBUG, "Force reseed", entropy);
		written = gather_entropy(&Random, 0);
		dolog(LOG_VERBOSE, "%lu bytes written to /dev/random", written);
		goto out;
	}

	entropy = read_entropy_avail(Entropy_avail_fd);

	if (0 == entropy)
		goto out;
	if (ENTROPYTHRESH < entropy) {
		dolog(LOG_DEBUG, "Sufficient entropy %d available", entropy);
		goto out;
	}
	dolog(LOG_DEBUG, "Insufficient entropy %d available", entropy);
	written = gather_entropy(&Random, 0);
	dolog(LOG_VERBOSE, "%lu bytes written to /dev/random", written);
out:
	install_alarm();
	return;
}

/* terminate the daemon cleanly */
static void sig_term(int sig)
{
	(void)sig;
	dolog(LOG_DEBUG, "Shutting down cleanly\n");

	/* Prevent the kernel from interfering with the shutdown */
	signal(SIGALRM, SIG_IGN);

	/* If we got another termination signal, just get killed */
	signal(SIGHUP, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	dealloc();
	exit(0);
}

/*
 * Wakeup on insufficient entropy on /dev/random
 */
static void select_fd(void)
{
	fd_set fds;
	int ret = 0;
	size_t written = 0;

	while (1) {
		FD_ZERO(&fds);
		dolog(LOG_DEBUG, "Polling /dev/random");
		FD_SET(Random.fd, &fds);
		/* only /dev/random implements polling */
		ret = select((Random.fd + 1), NULL, &fds, NULL, NULL);

		if (-1 == ret && EINTR != errno)
			dolog(LOG_ERR, "Select returned with error %s", strerror(errno));
		if (0 <= ret) {
			dolog(LOG_VERBOSE, "Wakeup call for select on /dev/random");
			written = gather_entropy(&Random, 0);
			dolog(LOG_VERBOSE, "%lu bytes written to /dev/random", written);
		}
	}
}

static void install_alarm(void)
{
	if (lrng_present())
		return;
	dolog(LOG_DEBUG, "Install alarm signal handler");
	signal(SIGALRM, sig_entropy_avail);
	alarm(5);
}

static void install_term(void)
{
	dolog(LOG_DEBUG, "Install termination signal handler");
	signal(SIGHUP, sig_term);
	signal(SIGINT, sig_term);
	signal(SIGQUIT, sig_term);
	signal(SIGTERM, sig_term);
}

/*******************************************************************
 * allocation functions
 *******************************************************************/

static void dealloc_rng(struct kernel_rng *rng)
{
	if (NULL != rng->ec) {
		jent_entropy_collector_free(rng->ec);
		rng->ec = NULL;
	}
	if (NULL != rng->rpi) {
		memset(rng->rpi, 0,(sizeof(struct rand_pool_info) +
				    (ENTROPYBYTES * OVERSAMPLINGFACTOR *
				     sizeof(char))));
		free(rng->rpi);
		rng->rpi = NULL;
	}
	if (-1 != rng->fd) {
		close(rng->fd);
		rng->fd = -1;
	}
}

static void dealloc(void)
{
	dealloc_rng(&Random);
	if (-1 != Entropy_avail_fd) {
		close(Entropy_avail_fd);
		Entropy_avail_fd = -1;
	}

	if (-1 != Pidfile_fd) {
		close(Pidfile_fd);
		Pidfile_fd = -1;
		if (NULL != Pidfile)
			unlink(Pidfile);
	}
}

static int alloc_rng(struct kernel_rng *rng)
{
	rng->ec = jent_entropy_collector_alloc(1, 0);
	if (!rng->ec) {
		dolog(LOG_ERR, "Allocation of entropy collector failed");
		return -EAGAIN;
	}

	rng->rpi = malloc((sizeof(struct rand_pool_info) +
			  (ENTROPYBYTES * OVERSAMPLINGFACTOR * sizeof(char))));
	if (!rng->rpi) {
		dolog(LOG_ERR, "Cannot allocate memory for random bytes");
		dealloc_rng(rng);
		return -ENOMEM;
	}

	rng->fd = open(rng->dev, O_WRONLY);
	if (-1 == rng->fd) {
		int errsv = errno;

		dolog(LOG_ERR, "Open of %s failed: %s", rng->dev, strerror(errno));
		dealloc_rng(rng);
		return -errsv;
	}

	return 0;
}

static int alloc(void)
{
	int ret = 0;
	size_t written = 0;

	ret = jent_entropy_init();
	if (ret) {
		dolog(LOG_ERR, "The initialization of CPU Jitter RNG failed with error code %d\n", ret);
		return ret;
	}

	ret = alloc_rng(&Random);
	if (ret)
		return ret;

	Entropy_avail_fd = open(ENTROPYAVAIL, O_RDONLY);
	if (-1 == Entropy_avail_fd) {
		int errsv = errno;

		dolog(LOG_ERR, "Open of %s failed: %s", ENTROPYAVAIL, strerror(errno));
		dealloc();
		return -errsv;
	}

	written = gather_entropy(&Random, 1);
	dolog(LOG_VERBOSE, "%lu bytes written to /dev/random", written);

	return 0;
}

static void create_pid_file(const char *pid_file)
{
	char pid_str[12];	/* max. integer length + '\n' + null */

	/* Ensure only one copy */
	Pidfile_fd = open(pid_file, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
	if (Pidfile_fd == -1)
		dolog(LOG_ERR, "Cannot open pid file\n");

	if (lockf(Pidfile_fd, F_TLOCK, 0) == -1) {
		if (errno == EAGAIN || errno == EACCES) {
			dolog(LOG_ERR, "PID file already locked\n");
			exit(1);
		} else
			dolog(LOG_ERR, "Cannot lock pid file\n");
	}

	if (ftruncate(Pidfile_fd, 0) == -1) {
		dolog(LOG_ERR, "Cannot truncate pid file\n");
		exit(1);
	}

	/* write our pid to the pid file */
	snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
	if (write(Pidfile_fd, pid_str, strlen(pid_str)) !=
	    (ssize_t)strlen(pid_str)) {
		dolog(LOG_ERR, "Cannot write to pid file\n");
		exit(1);
	}
}

static void daemonize(void)
{
	pid_t pid;
	
	/* already a daemon */
	if (1 == getppid())
	       return;

	pid = fork();
	if (pid < 0)
		dolog(LOG_ERR, "Cannot fork to daemonize\n");

	/* the parent process exits -- nothing has been allocated, nothing
	 * needs to be freed */
	if (0 < pid)
            exit(0);

	/* we are the child now */

	/* new SID for the child process */
	if (setsid() < 0)
		dolog(LOG_ERR, "Cannot obtain new SID for child\n");

	/* Change the current working directory.  This prevents the current
	 * directory from being locked; hence not being able to remove it. */
	if ((chdir("/")) < 0)
		dolog(LOG_ERR, "Cannot change directory\n");
	
	if (Pidfile && strlen(Pidfile))
		create_pid_file(Pidfile);

	/* Redirect standard files to /dev/null */
#if defined(__GNUC__) && __GNUC__ >= 5
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#endif
	freopen( "/dev/null", "r", stdin);
	freopen( "/dev/null", "w", stdout);
	freopen( "/dev/null", "w", stderr);
#if defined(__GNUC__) && __GNUC__ >= 5
#pragma GCC diagnostic pop
#endif
}


int main(int argc, char *argv[])
{
	int ret;

	parse_opts(argc, argv);

	if (geteuid())
		dolog(LOG_ERR, "Program must start as root!");

	ret = alloc();
	if (ret)
		return -ret;

	if (0 == Verbosity)
		daemonize();
	install_term();
	install_alarm();
	select_fd();
	/* NOTREACHED */
	dealloc();
	return 0;
}
