// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <linux/types.h>

#include <asm/getopt.h>
#include <asm/sections.h>
#include <asm/state.h>
#include <os.h>
#include <rtc_def.h>

/* Operating System Interface */

struct os_mem_hdr {
	size_t length;		/* number of bytes in the block */
};

ssize_t os_read(int fd, void *buf, size_t count)
{
	return read(fd, buf, count);
}

ssize_t os_write(int fd, const void *buf, size_t count)
{
	return write(fd, buf, count);
}

off_t os_lseek(int fd, off_t offset, int whence)
{
	if (whence == OS_SEEK_SET)
		whence = SEEK_SET;
	else if (whence == OS_SEEK_CUR)
		whence = SEEK_CUR;
	else if (whence == OS_SEEK_END)
		whence = SEEK_END;
	else
		os_exit(1);
	return lseek(fd, offset, whence);
}

int os_open(const char *pathname, int os_flags)
{
	int flags;

	switch (os_flags & OS_O_MASK) {
	case OS_O_RDONLY:
	default:
		flags = O_RDONLY;
		break;

	case OS_O_WRONLY:
		flags = O_WRONLY;
		break;

	case OS_O_RDWR:
		flags = O_RDWR;
		break;
	}

	if (os_flags & OS_O_CREAT)
		flags |= O_CREAT;
	if (os_flags & OS_O_TRUNC)
		flags |= O_TRUNC;

	return open(pathname, flags, 0777);
}

int os_close(int fd)
{
	return close(fd);
}

int os_unlink(const char *pathname)
{
	return unlink(pathname);
}

void os_exit(int exit_code)
{
	exit(exit_code);
}

int os_write_file(const char *fname, const void *buf, int size)
{
	int fd;

	fd = os_open(fname, OS_O_WRONLY | OS_O_CREAT | OS_O_TRUNC);
	if (fd < 0) {
		printf("Cannot open file '%s'\n", fname);
		return -EIO;
	}
	if (os_write(fd, buf, size) != size) {
		printf("Cannot write to file '%s'\n", fname);
		os_close(fd);
		return -EIO;
	}
	os_close(fd);

	return 0;
}

int os_read_file(const char *fname, void **bufp, int *sizep)
{
	off_t size;
	int ret = -EIO;
	int fd;

	fd = os_open(fname, OS_O_RDONLY);
	if (fd < 0) {
		printf("Cannot open file '%s'\n", fname);
		goto err;
	}
	size = os_lseek(fd, 0, OS_SEEK_END);
	if (size < 0) {
		printf("Cannot seek to end of file '%s'\n", fname);
		goto err;
	}
	if (os_lseek(fd, 0, OS_SEEK_SET) < 0) {
		printf("Cannot seek to start of file '%s'\n", fname);
		goto err;
	}
	*bufp = os_malloc(size);
	if (!*bufp) {
		printf("Not enough memory to read file '%s'\n", fname);
		ret = -ENOMEM;
		goto err;
	}
	if (os_read(fd, *bufp, size) != size) {
		printf("Cannot read from file '%s'\n", fname);
		goto err;
	}
	os_close(fd);
	*sizep = size;

	return 0;
err:
	os_close(fd);
	return ret;
}

/* Restore tty state when we exit */
static struct termios orig_term;
static bool term_setup;
static bool term_nonblock;

void os_fd_restore(void)
{
	if (term_setup) {
		int flags;

		tcsetattr(0, TCSANOW, &orig_term);
		if (term_nonblock) {
			flags = fcntl(0, F_GETFL, 0);
			fcntl(0, F_SETFL, flags & ~O_NONBLOCK);
		}
		term_setup = false;
	}
}

/* Put tty into raw mode so <tab> and <ctrl+c> work */
void os_tty_raw(int fd, bool allow_sigs)
{
	struct termios term;
	int flags;

	if (term_setup)
		return;

	/* If not a tty, don't complain */
	if (tcgetattr(fd, &orig_term))
		return;

	term = orig_term;
	term.c_iflag = IGNBRK | IGNPAR;
	term.c_oflag = OPOST | ONLCR;
	term.c_cflag = CS8 | CREAD | CLOCAL;
	term.c_lflag = allow_sigs ? ISIG : 0;
	if (tcsetattr(fd, TCSANOW, &term))
		return;

	flags = fcntl(fd, F_GETFL, 0);
	if (!(flags & O_NONBLOCK)) {
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK))
			return;
		term_nonblock = true;
	}

	term_setup = true;
	atexit(os_fd_restore);
}

void *os_malloc(size_t length)
{
	int page_size = getpagesize();
	struct os_mem_hdr *hdr;

	/*
	 * Use an address that is hopefully available to us so that pointers
	 * to this memory are fairly obvious. If we end up with a different
	 * address, that's fine too.
	 */
	hdr = mmap((void *)0x10000000, length + page_size,
		   PROT_READ | PROT_WRITE | PROT_EXEC,
		   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (hdr == MAP_FAILED)
		return NULL;
	hdr->length = length;

	return (void *)hdr + page_size;
}

void os_free(void *ptr)
{
	int page_size = getpagesize();
	struct os_mem_hdr *hdr;

	if (ptr) {
		hdr = ptr - page_size;
		munmap(hdr, hdr->length + page_size);
	}
}

void *os_realloc(void *ptr, size_t length)
{
	int page_size = getpagesize();
	struct os_mem_hdr *hdr;
	void *buf = NULL;

	if (length) {
		buf = os_malloc(length);
		if (!buf)
			return buf;
		if (ptr) {
			hdr = ptr - page_size;
			if (length > hdr->length)
				length = hdr->length;
			memcpy(buf, ptr, length);
		}
	}
	if (ptr)
		os_free(ptr);

	return buf;
}

void os_usleep(unsigned long usec)
{
	usleep(usec);
}

uint64_t __attribute__((no_instrument_function)) os_get_nsec(void)
{
#if defined(CLOCK_MONOTONIC) && defined(_POSIX_MONOTONIC_CLOCK)
	struct timespec tp;
	if (EINVAL == clock_gettime(CLOCK_MONOTONIC, &tp)) {
		struct timeval tv;

		gettimeofday(&tv, NULL);
		tp.tv_sec = tv.tv_sec;
		tp.tv_nsec = tv.tv_usec * 1000;
	}
	return tp.tv_sec * 1000000000ULL + tp.tv_nsec;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000000ULL + tv.tv_usec * 1000;
#endif
}

static char *short_opts;
static struct option *long_opts;

int os_parse_args(struct sandbox_state *state, int argc, char *argv[])
{
	struct sandbox_cmdline_option **sb_opt = __u_boot_sandbox_option_start;
	size_t num_options = __u_boot_sandbox_option_count();
	size_t i;

	int hidden_short_opt;
	size_t si;

	int c;

	if (short_opts || long_opts)
		return 1;

	state->argc = argc;
	state->argv = argv;

	/* dynamically construct the arguments to the system getopt_long */
	short_opts = os_malloc(sizeof(*short_opts) * num_options * 2 + 1);
	long_opts = os_malloc(sizeof(*long_opts) * num_options);
	if (!short_opts || !long_opts)
		return 1;

	/*
	 * getopt_long requires "val" to be unique (since that is what the
	 * func returns), so generate unique values automatically for flags
	 * that don't have a short option.  pick 0x100 as that is above the
	 * single byte range (where ASCII/ISO-XXXX-X charsets live).
	 */
	hidden_short_opt = 0x100;
	si = 0;
	for (i = 0; i < num_options; ++i) {
		long_opts[i].name = sb_opt[i]->flag;
		long_opts[i].has_arg = sb_opt[i]->has_arg ?
			required_argument : no_argument;
		long_opts[i].flag = NULL;

		if (sb_opt[i]->flag_short) {
			short_opts[si++] = long_opts[i].val = sb_opt[i]->flag_short;
			if (long_opts[i].has_arg == required_argument)
				short_opts[si++] = ':';
		} else
			long_opts[i].val = sb_opt[i]->flag_short = hidden_short_opt++;
	}
	short_opts[si] = '\0';

	/* we need to handle output ourselves since u-boot provides printf */
	opterr = 0;

	/*
	 * walk all of the options the user gave us on the command line,
	 * figure out what u-boot option structure they belong to (via
	 * the unique short val key), and call the appropriate callback.
	 */
	while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
		for (i = 0; i < num_options; ++i) {
			if (sb_opt[i]->flag_short == c) {
				if (sb_opt[i]->callback(state, optarg)) {
					state->parse_err = sb_opt[i]->flag;
					return 0;
				}
				break;
			}
		}
		if (i == num_options) {
			/*
			 * store the faulting flag for later display.  we have to
			 * store the flag itself as the getopt parsing itself is
			 * tricky: need to handle the following flags (assume all
			 * of the below are unknown):
			 *   -a        optopt='a' optind=<next>
			 *   -abbbb    optopt='a' optind=<this>
			 *   -aaaaa    optopt='a' optind=<this>
			 *   --a       optopt=0   optind=<this>
			 * as you can see, it is impossible to determine the exact
			 * faulting flag without doing the parsing ourselves, so
			 * we just report the specific flag that failed.
			 */
			if (optopt) {
				static char parse_err[3] = { '-', 0, '\0', };
				parse_err[1] = optopt;
				state->parse_err = parse_err;
			} else
				state->parse_err = argv[optind - 1];
			break;
		}
	}

	return 0;
}

void os_dirent_free(struct os_dirent_node *node)
{
	struct os_dirent_node *next;

	while (node) {
		next = node->next;
		os_free(node);
		node = next;
	}
}

int os_dirent_ls(const char *dirname, struct os_dirent_node **headp)
{
	struct dirent *entry;
	struct os_dirent_node *head, *node, *next;
	struct stat buf;
	DIR *dir;
	int ret;
	char *fname;
	char *old_fname;
	int len;
	int dirlen;

	*headp = NULL;
	dir = opendir(dirname);
	if (!dir)
		return -1;

	/* Create a buffer upfront, with typically sufficient size */
	dirlen = strlen(dirname) + 2;
	len = dirlen + 256;
	fname = os_malloc(len);
	if (!fname) {
		ret = -ENOMEM;
		goto done;
	}

	for (node = head = NULL;; node = next) {
		errno = 0;
		entry = readdir(dir);
		if (!entry) {
			ret = errno;
			break;
		}
		next = os_malloc(sizeof(*node) + strlen(entry->d_name) + 1);
		if (!next) {
			os_dirent_free(head);
			ret = -ENOMEM;
			goto done;
		}
		if (dirlen + strlen(entry->d_name) > len) {
			len = dirlen + strlen(entry->d_name);
			old_fname = fname;
			fname = os_realloc(fname, len);
			if (!fname) {
				os_free(old_fname);
				os_free(next);
				os_dirent_free(head);
				ret = -ENOMEM;
				goto done;
			}
		}
		next->next = NULL;
		strcpy(next->name, entry->d_name);
		switch (entry->d_type) {
		case DT_REG:
			next->type = OS_FILET_REG;
			break;
		case DT_DIR:
			next->type = OS_FILET_DIR;
			break;
		case DT_LNK:
			next->type = OS_FILET_LNK;
			break;
		default:
			next->type = OS_FILET_UNKNOWN;
		}
		next->size = 0;
		snprintf(fname, len, "%s/%s", dirname, next->name);
		if (!stat(fname, &buf))
			next->size = buf.st_size;
		if (node)
			node->next = next;
		else
			head = next;
	}
	*headp = head;

done:
	closedir(dir);
	os_free(fname);
	return ret;
}

const char *os_dirent_typename[OS_FILET_COUNT] = {
	"   ",
	"SYM",
	"DIR",
	"???",
};

const char *os_dirent_get_typename(enum os_dirent_t type)
{
	if (type >= OS_FILET_REG && type < OS_FILET_COUNT)
		return os_dirent_typename[type];

	return os_dirent_typename[OS_FILET_UNKNOWN];
}

int os_get_filesize(const char *fname, loff_t *size)
{
	struct stat buf;
	int ret;

	ret = stat(fname, &buf);
	if (ret)
		return ret;
	*size = buf.st_size;
	return 0;
}

void os_putc(int ch)
{
	putchar(ch);
}

void os_puts(const char *str)
{
	while (*str)
		os_putc(*str++);
}

int os_write_ram_buf(const char *fname)
{
	struct sandbox_state *state = state_get_current();
	int fd, ret;

	fd = open(fname, O_CREAT | O_WRONLY, 0777);
	if (fd < 0)
		return -ENOENT;
	ret = write(fd, state->ram_buf, state->ram_size);
	close(fd);
	if (ret != state->ram_size)
		return -EIO;

	return 0;
}

int os_read_ram_buf(const char *fname)
{
	struct sandbox_state *state = state_get_current();
	int fd, ret;
	loff_t size;

	ret = os_get_filesize(fname, &size);
	if (ret < 0)
		return ret;
	if (size != state->ram_size)
		return -ENOSPC;
	fd = open(fname, O_RDONLY);
	if (fd < 0)
		return -ENOENT;

	ret = read(fd, state->ram_buf, state->ram_size);
	close(fd);
	if (ret != state->ram_size)
		return -EIO;

	return 0;
}

static int make_exec(char *fname, const void *data, int size)
{
	int fd;

	strcpy(fname, "/tmp/u-boot.jump.XXXXXX");
	fd = mkstemp(fname);
	if (fd < 0)
		return -ENOENT;
	if (write(fd, data, size) < 0)
		return -EIO;
	close(fd);
	if (chmod(fname, 0777))
		return -ENOEXEC;

	return 0;
}

/**
 * add_args() - Allocate a new argv with the given args
 *
 * This is used to create a new argv array with all the old arguments and some
 * new ones that are passed in
 *
 * @argvp:  Returns newly allocated args list
 * @add_args: Arguments to add, each a string
 * @count: Number of arguments in @add_args
 * @return 0 if OK, -ENOMEM if out of memory
 */
static int add_args(char ***argvp, char *add_args[], int count)
{
	char **argv, **ap;
	int argc;

	for (argc = 0; (*argvp)[argc]; argc++)
		;

	argv = os_malloc((argc + count + 1) * sizeof(char *));
	if (!argv) {
		printf("Out of memory for %d argv\n", count);
		return -ENOMEM;
	}
	for (ap = *argvp, argc = 0; *ap; ap++) {
		char *arg = *ap;

		/* Drop args that we don't want to propagate */
		if (*arg == '-' && strlen(arg) == 2) {
			switch (arg[1]) {
			case 'j':
			case 'm':
				ap++;
				continue;
			}
		} else if (!strcmp(arg, "--rm_memory")) {
			ap++;
			continue;
		}
		argv[argc++] = arg;
	}

	memcpy(argv + argc, add_args, count * sizeof(char *));
	argv[argc + count] = NULL;

	*argvp = argv;
	return 0;
}

/**
 * os_jump_to_file() - Jump to a new program
 *
 * This saves the memory buffer, sets up arguments to the new process, then
 * execs it.
 *
 * @fname: Filename to exec
 * @return does not return on success, any return value is an error
 */
static int os_jump_to_file(const char *fname)
{
	struct sandbox_state *state = state_get_current();
	char mem_fname[30];
	int fd, err;
	char *extra_args[5];
	char **argv = state->argv;
	int argc;
#ifdef DEBUG
	int i;
#endif

	strcpy(mem_fname, "/tmp/u-boot.mem.XXXXXX");
	fd = mkstemp(mem_fname);
	if (fd < 0)
		return -ENOENT;
	close(fd);
	err = os_write_ram_buf(mem_fname);
	if (err)
		return err;

	os_fd_restore();

	extra_args[0] = "-j";
	extra_args[1] = (char *)fname;
	extra_args[2] = "-m";
	extra_args[3] = mem_fname;
	argc = 4;
	if (state->ram_buf_rm)
		extra_args[argc++] = "--rm_memory";
	err = add_args(&argv, extra_args, argc);
	if (err)
		return err;
	argv[0] = (char *)fname;

#ifdef DEBUG
	for (i = 0; argv[i]; i++)
		printf("%d %s\n", i, argv[i]);
#endif

	if (state_uninit())
		os_exit(2);

	err = execv(fname, argv);
	os_free(argv);
	if (err) {
		perror("Unable to run image");
		printf("Image filename '%s'\n", fname);
		return err;
	}

	return unlink(fname);
}

int os_jump_to_image(const void *dest, int size)
{
	char fname[30];
	int err;

	err = make_exec(fname, dest, size);
	if (err)
		return err;

	return os_jump_to_file(fname);
}

int os_find_u_boot(char *fname, int maxlen)
{
	struct sandbox_state *state = state_get_current();
	const char *progname = state->argv[0];
	int len = strlen(progname);
	const char *suffix;
	char *p;
	int fd;

	if (len >= maxlen || len < 4)
		return -ENOSPC;

	strcpy(fname, progname);
	suffix = fname + len - 4;

	/* If we are TPL, boot to SPL */
	if (!strcmp(suffix, "-tpl")) {
		fname[len - 3] = 's';
		fd = os_open(fname, O_RDONLY);
		if (fd >= 0) {
			close(fd);
			return 0;
		}

		/* Look for 'u-boot-tpl' in the tpl/ directory */
		p = strstr(fname, "/tpl/");
		if (p) {
			p[1] = 's';
			fd = os_open(fname, O_RDONLY);
			if (fd >= 0) {
				close(fd);
				return 0;
			}
		}
		return -ENOENT;
	}

	/* Look for 'u-boot' in the same directory as 'u-boot-spl' */
	if (!strcmp(suffix, "-spl")) {
		fname[len - 4] = '\0';
		fd = os_open(fname, O_RDONLY);
		if (fd >= 0) {
			close(fd);
			return 0;
		}
	}

	/* Look for 'u-boot' in the parent directory of spl/ */
	p = strstr(fname, "spl/");
	if (p) {
		/* Remove the "spl" characters */
		memmove(p, p + 4, strlen(p + 4) + 1);
		fd = os_open(fname, O_RDONLY);
		if (fd >= 0) {
			close(fd);
			return 0;
		}
	}

	return -ENOENT;
}

int os_spl_to_uboot(const char *fname)
{
	return os_jump_to_file(fname);
}

void os_localtime(struct rtc_time *rt)
{
	time_t t = time(NULL);
	struct tm *tm;

	tm = localtime(&t);
	rt->tm_sec = tm->tm_sec;
	rt->tm_min = tm->tm_min;
	rt->tm_hour = tm->tm_hour;
	rt->tm_mday = tm->tm_mday;
	rt->tm_mon = tm->tm_mon + 1;
	rt->tm_year = tm->tm_year + 1900;
	rt->tm_wday = tm->tm_wday;
	rt->tm_yday = tm->tm_yday;
	rt->tm_isdst = tm->tm_isdst;
}

void os_abort(void)
{
	abort();
}

int os_mprotect_allow(void *start, size_t len)
{
	int page_size = getpagesize();

	/* Move start to the start of a page, len to the end */
	start = (void *)(((ulong)start) & ~(page_size - 1));
	len = (len + page_size * 2) & ~(page_size - 1);

	return mprotect(start, len, PROT_READ | PROT_WRITE);
}

void *os_find_text_base(void)
{
	char line[500];
	void *base = NULL;
	int len;
	int fd;

	/*
	 * This code assumes that the first line of /proc/self/maps holds
	 * information about the text, for example:
	 *
	 * 5622d9907000-5622d9a55000 r-xp 00000000 08:01 15067168   u-boot
	 *
	 * The first hex value is assumed to be the address.
	 *
	 * This is tested in Linux 4.15.
	 */
	fd = open("/proc/self/maps", O_RDONLY);
	if (fd == -1)
		return NULL;
	len = read(fd, line, sizeof(line));
	if (len > 0) {
		char *end = memchr(line, '-', len);

		if (end) {
			unsigned long long addr;

			*end = '\0';
			if (sscanf(line, "%llx", &addr) == 1)
				base = (void *)addr;
		}
	}
	close(fd);

	return base;
}
