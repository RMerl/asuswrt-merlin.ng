#include "includes.h"

#include <time.h>
#include <sys/wait.h>

#include "os.h"
#include "common.h"

#ifdef mapd_TRACE

#include "mapd_debug.h"
#include "trace.h"
#include "list.h"

static struct dl_list alloc_list = DL_LIST_HEAD_INIT(alloc_list);

#define ALLOC_MAGIC 0xa84ef1b2
#define FREED_MAGIC 0x67fd487a

struct os_alloc_trace {
	unsigned int magic;
	struct dl_list list;
	size_t len;
	mapd_TRACE_INFO
} __attribute__((aligned(16)));

#endif /* mapd_TRACE */
int os_get_reltime(struct os_reltime *t)
{
#ifndef __MACH__
#if defined(CLOCK_BOOTTIME)
	static clockid_t clock_id = CLOCK_BOOTTIME;
#elif defined(CLOCK_MONOTONIC)
	static clockid_t clock_id = CLOCK_MONOTONIC;
#else
	static clockid_t clock_id = CLOCK_REALTIME;
#endif
	struct timespec ts;
	int res;

	if (TEST_FAIL())
		return -1;

	while (1) {
		res = clock_gettime(clock_id, &ts);
		if (res == 0) {
			t->sec = ts.tv_sec;
			t->usec = ts.tv_nsec / 1000;
			return 0;
		}
		switch (clock_id) {
#ifdef CLOCK_BOOTTIME
		case CLOCK_BOOTTIME:
			clock_id = CLOCK_MONOTONIC;
			break;
#endif
#ifdef CLOCK_MONOTONIC
		case CLOCK_MONOTONIC:
			clock_id = CLOCK_REALTIME;
			break;
#endif
		case CLOCK_REALTIME:
			return -1;
		}
	}
#else /* __MACH__ */
	uint64_t abstime, nano;
	static mach_timebase_info_data_t info = { 0, 0 };

	if (!info.denom) {
		if (mach_timebase_info(&info) != KERN_SUCCESS)
			return -1;
	}

	abstime = mach_absolute_time();
	nano = (abstime * info.numer) / info.denom;

	t->sec = nano / NSEC_PER_SEC;
	t->usec = (nano - (((uint64_t) t->sec) * NSEC_PER_SEC)) / NSEC_PER_USEC;

	return 0;
#endif /* __MACH__ */
}

char * os_rel2abs_path(const char *rel_path)
{
	char *buf = NULL, *cwd, *ret;
	size_t len = 128, cwd_len, rel_len, ret_len;
	int last_errno;

	if (!rel_path)
		return NULL;

	if (rel_path[0] == '/')
		return os_strdup(rel_path);

	for (;;) {
		buf = os_malloc(len);
		if (buf == NULL)
			return NULL;
		cwd = getcwd(buf, len);
		if (cwd == NULL) {
			last_errno = errno;
			os_free(buf);
			if (last_errno != ERANGE)
				return NULL;
			len *= 2;
			if (len > 2000)
				return NULL;
		} else {
			buf[len - 1] = '\0';
			break;
		}
	}

	cwd_len = os_strlen(cwd);
	rel_len = os_strlen(rel_path);
	ret_len = cwd_len + 1 + rel_len + 1;
	ret = os_malloc(ret_len);
	if (ret) {
		os_memcpy(ret, cwd, cwd_len);
		ret[cwd_len] = '/';
		os_memcpy(ret + cwd_len + 1, rel_path, rel_len);
		ret[ret_len - 1] = '\0';
	}
	os_free(buf);
	return ret;
}

int os_fdatasync(FILE *stream)
{
	if (!fflush(stream)) {
#ifdef __linux__
		return fdatasync(fileno(stream));
#else /* !__linux__ */
#ifdef F_FULLFSYNC
		/* OS X does not implement fdatasync(). */
		return fcntl(fileno(stream), F_FULLFSYNC);
#else /* F_FULLFSYNC */
		return fsync(fileno(stream));
#endif /* F_FULLFSYNC */
#endif /* __linux__ */
	}

	return -1;
}

size_t os_strlcpy(char *dest, const char *src, size_t siz)
{
	const char *s = src;
	size_t left = siz;

	if (left) {
		/* Copy string up to the maximum size of the dest buffer */
		while (--left != 0) {
			if ((*dest++ = *s++) == '\0')
				break;
		}
	}

	if (left == 0) {
		/* Not enough room for the string; force NUL-termination */
		if (siz != 0)
			*dest = '\0';
		while (*s++)
			; /* determine total src string length */
	}

	return s - src - 1;
}


int os_memcmp_const(const void *a, const void *b, size_t len)
{
	const u8 *aa = a;
	const u8 *bb = b;
	size_t i;
	u8 res;

	for (res = 0, i = 0; i < len; i++)
		res |= aa[i] ^ bb[i];

	return res;
}


void * os_memdup(const void *src, size_t len)
{
	void *r = os_malloc(len);

	if (r)
		os_memcpy(r, src, len);
	return r;
}


#ifdef mapd_TRACE

#if defined(mapd_TRACE_BFD) && defined(CONFIG_TESTING_OPTIONS)
char mapd_trace_fail_func[256] = { 0 };
unsigned int mapd_trace_fail_after;

static int testing_fail_alloc(void)
{
	const char *func[mapd_TRACE_LEN];
	size_t i, res, len;
	char *pos, *next;
	int match;

	if (!mapd_trace_fail_after)
		return 0;

	res = mapd_trace_calling_func(func, mapd_TRACE_LEN);
	i = 0;
	if (i < res && os_strcmp(func[i], __func__) == 0)
		i++;
	if (i < res && os_strcmp(func[i], "os_malloc") == 0)
		i++;
	if (i < res && os_strcmp(func[i], "os_zalloc") == 0)
		i++;
	if (i < res && os_strcmp(func[i], "os_calloc") == 0)
		i++;
	if (i < res && os_strcmp(func[i], "os_realloc") == 0)
		i++;
	if (i < res && os_strcmp(func[i], "os_realloc_array") == 0)
		i++;
	if (i < res && os_strcmp(func[i], "os_strdup") == 0)
		i++;
	if (i < res && os_strcmp(func[i], "os_memdup") == 0)
		i++;

	pos = mapd_trace_fail_func;

	match = 0;
	while (i < res) {
		int allow_skip = 1;
		int maybe = 0;

		if (*pos == '=') {
			allow_skip = 0;
			pos++;
		} else if (*pos == '?') {
			maybe = 1;
			pos++;
		}
		next = os_strchr(pos, ';');
		if (next)
			len = next - pos;
		else
			len = os_strlen(pos);
		if (os_memcmp(pos, func[i], len) != 0) {
			if (maybe && next) {
				pos = next + 1;
				continue;
			}
			if (allow_skip) {
				i++;
				continue;
			}
			return 0;
		}
		if (!next) {
			match = 1;
			break;
		}
		pos = next + 1;
		i++;
	}
	if (!match)
		return 0;

	mapd_trace_fail_after--;
	if (mapd_trace_fail_after == 0) {
		mapd_printf(MSG_INFO, "TESTING: fail allocation at %s",
			   mapd_trace_fail_func);
		for (i = 0; i < res; i++)
			mapd_printf(MSG_INFO, "backtrace[%d] = %s",
				   (int) i, func[i]);
		return 1;
	}

	return 0;
}


char mapd_trace_test_fail_func[256] = { 0 };
unsigned int mapd_trace_test_fail_after;

int testing_test_fail(void)
{
	const char *func[mapd_TRACE_LEN];
	size_t i, res, len;
	char *pos, *next;
	int match;

	if (!mapd_trace_test_fail_after)
		return 0;

	res = mapd_trace_calling_func(func, mapd_TRACE_LEN);
	i = 0;
	if (i < res && os_strcmp(func[i], __func__) == 0)
		i++;

	pos = mapd_trace_test_fail_func;

	match = 0;
	while (i < res) {
		int allow_skip = 1;
		int maybe = 0;

		if (*pos == '=') {
			allow_skip = 0;
			pos++;
		} else if (*pos == '?') {
			maybe = 1;
			pos++;
		}
		next = os_strchr(pos, ';');
		if (next)
			len = next - pos;
		else
			len = os_strlen(pos);
		if (os_memcmp(pos, func[i], len) != 0) {
			if (maybe && next) {
				pos = next + 1;
				continue;
			}
			if (allow_skip) {
				i++;
				continue;
			}
			return 0;
		}
		if (!next) {
			match = 1;
			break;
		}
		pos = next + 1;
		i++;
	}
	if (!match)
		return 0;

	mapd_trace_test_fail_after--;
	if (mapd_trace_test_fail_after == 0) {
		mapd_printf(MSG_INFO, "TESTING: fail at %s",
			   mapd_trace_test_fail_func);
		for (i = 0; i < res; i++)
			mapd_printf(MSG_INFO, "backtrace[%d] = %s",
				   (int) i, func[i]);
		return 1;
	}

	return 0;
}

#else

static inline int testing_fail_alloc(void)
{
	return 0;
}
#endif

void * os_malloc(size_t size)
{
	struct os_alloc_trace *a;

	if (testing_fail_alloc())
		return NULL;

	a = malloc(sizeof(*a) + size);
	if (a == NULL)
		return NULL;
	a->magic = ALLOC_MAGIC;
	dl_list_add(&alloc_list, &a->list);
	a->len = size;
	mapd_trace_record(a);
	return a + 1;
}


void * os_realloc(void *ptr, size_t size)
{
	struct os_alloc_trace *a;
	size_t copy_len;
	void *n;

	if (ptr == NULL)
		return os_malloc(size);

	a = (struct os_alloc_trace *) ptr - 1;
	if (a->magic != ALLOC_MAGIC) {
		mapd_printf(MSG_INFO, "REALLOC[%p]: invalid magic 0x%x%s",
			   a, a->magic,
			   a->magic == FREED_MAGIC ? " (already freed)" : "");
		abort();
	}
	n = os_malloc(size);
	if (n == NULL)
		return NULL;
	copy_len = a->len;
	if (copy_len > size)
		copy_len = size;
	os_memcpy(n, a + 1, copy_len);
	os_free(ptr);
	return n;
}


void os_free(void *ptr)
{
	struct os_alloc_trace *a;

	if (ptr == NULL)
		return;
	a = (struct os_alloc_trace *) ptr - 1;
	if (a->magic != ALLOC_MAGIC) {
		mapd_printf(MSG_INFO, "FREE[%p]: invalid magic 0x%x%s",
			   a, a->magic,
			   a->magic == FREED_MAGIC ? " (already freed)" : "");
		abort();
	}
	dl_list_del(&a->list);
	a->magic = FREED_MAGIC;

	mapd_trace_check_ref(ptr);
	free(a);
}


void * os_zalloc(size_t size)
{
	void *ptr = os_malloc(size);
	if (ptr)
		os_memset(ptr, 0, size);
	return ptr;
}


char * os_strdup(const char *s)
{
	size_t len;
	char *d;
	len = os_strlen(s);
	d = os_malloc(len + 1);
	if (d == NULL)
		return NULL;
	os_memcpy(d, s, len);
	d[len] = '\0';
	return d;
}

#endif /* mapd_TRACE */


int os_exec(const char *program, const char *arg, int wait_completion)
{
	pid_t pid;
	int pid_status;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return -1;
	}

	if (pid == 0) {
		/* run the external commapd in the child process */
		const int MAX_ARG = 30;
		char *_program, *_arg, *pos;
		char *argv[MAX_ARG + 1];
		int i;

		_program = os_strdup(program);
		_arg = os_strdup(arg);

		argv[0] = _program;

		i = 1;
		pos = _arg;
		while (i < MAX_ARG && pos && *pos) {
			while (*pos == ' ')
				pos++;
			if (*pos == '\0')
				break;
			argv[i++] = pos;
			pos = os_strchr(pos, ' ');
			if (pos)
				*pos++ = '\0';
		}
		argv[i] = NULL;

		execv(program, argv);
		perror("execv");
		os_free(_program);
		os_free(_arg);
		exit(0);
		return -1;
	}

	if (wait_completion) {
		/* wait for the child process to complete in the parent */
		waitpid(pid, &pid_status, 0);
	}

	return 0;
}
