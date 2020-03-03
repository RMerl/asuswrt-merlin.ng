#define _GNU_SOURCE
#define __EXPORTED_HEADERS__

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <linux/falloc.h>
#include <linux/fcntl.h>
#include <linux/memfd.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#define MFD_DEF_SIZE 8192
#define STACK_SIZE 65535

static int sys_memfd_create(const char *name,
			    unsigned int flags)
{
	return syscall(__NR_memfd_create, name, flags);
}

static int mfd_assert_new(const char *name, loff_t sz, unsigned int flags)
{
	int r, fd;

	fd = sys_memfd_create(name, flags);
	if (fd < 0) {
		printf("memfd_create(\"%s\", %u) failed: %m\n",
		       name, flags);
		abort();
	}

	r = ftruncate(fd, sz);
	if (r < 0) {
		printf("ftruncate(%llu) failed: %m\n", (unsigned long long)sz);
		abort();
	}

	return fd;
}

static void mfd_fail_new(const char *name, unsigned int flags)
{
	int r;

	r = sys_memfd_create(name, flags);
	if (r >= 0) {
		printf("memfd_create(\"%s\", %u) succeeded, but failure expected\n",
		       name, flags);
		close(r);
		abort();
	}
}

static unsigned int mfd_assert_get_seals(int fd)
{
	int r;

	r = fcntl(fd, F_GET_SEALS);
	if (r < 0) {
		printf("GET_SEALS(%d) failed: %m\n", fd);
		abort();
	}

	return (unsigned int)r;
}

static void mfd_assert_has_seals(int fd, unsigned int seals)
{
	unsigned int s;

	s = mfd_assert_get_seals(fd);
	if (s != seals) {
		printf("%u != %u = GET_SEALS(%d)\n", seals, s, fd);
		abort();
	}
}

static void mfd_assert_add_seals(int fd, unsigned int seals)
{
	int r;
	unsigned int s;

	s = mfd_assert_get_seals(fd);
	r = fcntl(fd, F_ADD_SEALS, seals);
	if (r < 0) {
		printf("ADD_SEALS(%d, %u -> %u) failed: %m\n", fd, s, seals);
		abort();
	}
}

static void mfd_fail_add_seals(int fd, unsigned int seals)
{
	int r;
	unsigned int s;

	r = fcntl(fd, F_GET_SEALS);
	if (r < 0)
		s = 0;
	else
		s = (unsigned int)r;

	r = fcntl(fd, F_ADD_SEALS, seals);
	if (r >= 0) {
		printf("ADD_SEALS(%d, %u -> %u) didn't fail as expected\n",
				fd, s, seals);
		abort();
	}
}

static void mfd_assert_size(int fd, size_t size)
{
	struct stat st;
	int r;

	r = fstat(fd, &st);
	if (r < 0) {
		printf("fstat(%d) failed: %m\n", fd);
		abort();
	} else if (st.st_size != size) {
		printf("wrong file size %lld, but expected %lld\n",
		       (long long)st.st_size, (long long)size);
		abort();
	}
}

static int mfd_assert_dup(int fd)
{
	int r;

	r = dup(fd);
	if (r < 0) {
		printf("dup(%d) failed: %m\n", fd);
		abort();
	}

	return r;
}

static void *mfd_assert_mmap_shared(int fd)
{
	void *p;

	p = mmap(NULL,
		 MFD_DEF_SIZE,
		 PROT_READ | PROT_WRITE,
		 MAP_SHARED,
		 fd,
		 0);
	if (p == MAP_FAILED) {
		printf("mmap() failed: %m\n");
		abort();
	}

	return p;
}

static void *mfd_assert_mmap_private(int fd)
{
	void *p;

	p = mmap(NULL,
		 MFD_DEF_SIZE,
		 PROT_READ,
		 MAP_PRIVATE,
		 fd,
		 0);
	if (p == MAP_FAILED) {
		printf("mmap() failed: %m\n");
		abort();
	}

	return p;
}

static int mfd_assert_open(int fd, int flags, mode_t mode)
{
	char buf[512];
	int r;

	sprintf(buf, "/proc/self/fd/%d", fd);
	r = open(buf, flags, mode);
	if (r < 0) {
		printf("open(%s) failed: %m\n", buf);
		abort();
	}

	return r;
}

static void mfd_fail_open(int fd, int flags, mode_t mode)
{
	char buf[512];
	int r;

	sprintf(buf, "/proc/self/fd/%d", fd);
	r = open(buf, flags, mode);
	if (r >= 0) {
		printf("open(%s) didn't fail as expected\n", buf);
		abort();
	}
}

static void mfd_assert_read(int fd)
{
	char buf[16];
	void *p;
	ssize_t l;

	l = read(fd, buf, sizeof(buf));
	if (l != sizeof(buf)) {
		printf("read() failed: %m\n");
		abort();
	}

	/* verify PROT_READ *is* allowed */
	p = mmap(NULL,
		 MFD_DEF_SIZE,
		 PROT_READ,
		 MAP_PRIVATE,
		 fd,
		 0);
	if (p == MAP_FAILED) {
		printf("mmap() failed: %m\n");
		abort();
	}
	munmap(p, MFD_DEF_SIZE);

	/* verify MAP_PRIVATE is *always* allowed (even writable) */
	p = mmap(NULL,
		 MFD_DEF_SIZE,
		 PROT_READ | PROT_WRITE,
		 MAP_PRIVATE,
		 fd,
		 0);
	if (p == MAP_FAILED) {
		printf("mmap() failed: %m\n");
		abort();
	}
	munmap(p, MFD_DEF_SIZE);
}

static void mfd_assert_write(int fd)
{
	ssize_t l;
	void *p;
	int r;

	/* verify write() succeeds */
	l = write(fd, "\0\0\0\0", 4);
	if (l != 4) {
		printf("write() failed: %m\n");
		abort();
	}

	/* verify PROT_READ | PROT_WRITE is allowed */
	p = mmap(NULL,
		 MFD_DEF_SIZE,
		 PROT_READ | PROT_WRITE,
		 MAP_SHARED,
		 fd,
		 0);
	if (p == MAP_FAILED) {
		printf("mmap() failed: %m\n");
		abort();
	}
	*(char *)p = 0;
	munmap(p, MFD_DEF_SIZE);

	/* verify PROT_WRITE is allowed */
	p = mmap(NULL,
		 MFD_DEF_SIZE,
		 PROT_WRITE,
		 MAP_SHARED,
		 fd,
		 0);
	if (p == MAP_FAILED) {
		printf("mmap() failed: %m\n");
		abort();
	}
	*(char *)p = 0;
	munmap(p, MFD_DEF_SIZE);

	/* verify PROT_READ with MAP_SHARED is allowed and a following
	 * mprotect(PROT_WRITE) allows writing */
	p = mmap(NULL,
		 MFD_DEF_SIZE,
		 PROT_READ,
		 MAP_SHARED,
		 fd,
		 0);
	if (p == MAP_FAILED) {
		printf("mmap() failed: %m\n");
		abort();
	}

	r = mprotect(p, MFD_DEF_SIZE, PROT_READ | PROT_WRITE);
	if (r < 0) {
		printf("mprotect() failed: %m\n");
		abort();
	}

	*(char *)p = 0;
	munmap(p, MFD_DEF_SIZE);

	/* verify PUNCH_HOLE works */
	r = fallocate(fd,
		      FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
		      0,
		      MFD_DEF_SIZE);
	if (r < 0) {
		printf("fallocate(PUNCH_HOLE) failed: %m\n");
		abort();
	}
}

static void mfd_fail_write(int fd)
{
	ssize_t l;
	void *p;
	int r;

	/* verify write() fails */
	l = write(fd, "data", 4);
	if (l != -EPERM) {
		printf("expected EPERM on write(), but got %d: %m\n", (int)l);
		abort();
	}

	/* verify PROT_READ | PROT_WRITE is not allowed */
	p = mmap(NULL,
		 MFD_DEF_SIZE,
		 PROT_READ | PROT_WRITE,
		 MAP_SHARED,
		 fd,
		 0);
	if (p != MAP_FAILED) {
		printf("mmap() didn't fail as expected\n");
		abort();
	}

	/* verify PROT_WRITE is not allowed */
	p = mmap(NULL,
		 MFD_DEF_SIZE,
		 PROT_WRITE,
		 MAP_SHARED,
		 fd,
		 0);
	if (p != MAP_FAILED) {
		printf("mmap() didn't fail as expected\n");
		abort();
	}

	/* Verify PROT_READ with MAP_SHARED with a following mprotect is not
	 * allowed. Note that for r/w the kernel already prevents the mmap. */
	p = mmap(NULL,
		 MFD_DEF_SIZE,
		 PROT_READ,
		 MAP_SHARED,
		 fd,
		 0);
	if (p != MAP_FAILED) {
		r = mprotect(p, MFD_DEF_SIZE, PROT_READ | PROT_WRITE);
		if (r >= 0) {
			printf("mmap()+mprotect() didn't fail as expected\n");
			abort();
		}
	}

	/* verify PUNCH_HOLE fails */
	r = fallocate(fd,
		      FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
		      0,
		      MFD_DEF_SIZE);
	if (r >= 0) {
		printf("fallocate(PUNCH_HOLE) didn't fail as expected\n");
		abort();
	}
}

static void mfd_assert_shrink(int fd)
{
	int r, fd2;

	r = ftruncate(fd, MFD_DEF_SIZE / 2);
	if (r < 0) {
		printf("ftruncate(SHRINK) failed: %m\n");
		abort();
	}

	mfd_assert_size(fd, MFD_DEF_SIZE / 2);

	fd2 = mfd_assert_open(fd,
			      O_RDWR | O_CREAT | O_TRUNC,
			      S_IRUSR | S_IWUSR);
	close(fd2);

	mfd_assert_size(fd, 0);
}

static void mfd_fail_shrink(int fd)
{
	int r;

	r = ftruncate(fd, MFD_DEF_SIZE / 2);
	if (r >= 0) {
		printf("ftruncate(SHRINK) didn't fail as expected\n");
		abort();
	}

	mfd_fail_open(fd,
		      O_RDWR | O_CREAT | O_TRUNC,
		      S_IRUSR | S_IWUSR);
}

static void mfd_assert_grow(int fd)
{
	int r;

	r = ftruncate(fd, MFD_DEF_SIZE * 2);
	if (r < 0) {
		printf("ftruncate(GROW) failed: %m\n");
		abort();
	}

	mfd_assert_size(fd, MFD_DEF_SIZE * 2);

	r = fallocate(fd,
		      0,
		      0,
		      MFD_DEF_SIZE * 4);
	if (r < 0) {
		printf("fallocate(ALLOC) failed: %m\n");
		abort();
	}

	mfd_assert_size(fd, MFD_DEF_SIZE * 4);
}

static void mfd_fail_grow(int fd)
{
	int r;

	r = ftruncate(fd, MFD_DEF_SIZE * 2);
	if (r >= 0) {
		printf("ftruncate(GROW) didn't fail as expected\n");
		abort();
	}

	r = fallocate(fd,
		      0,
		      0,
		      MFD_DEF_SIZE * 4);
	if (r >= 0) {
		printf("fallocate(ALLOC) didn't fail as expected\n");
		abort();
	}
}

static void mfd_assert_grow_write(int fd)
{
	static char buf[MFD_DEF_SIZE * 8];
	ssize_t l;

	l = pwrite(fd, buf, sizeof(buf), 0);
	if (l != sizeof(buf)) {
		printf("pwrite() failed: %m\n");
		abort();
	}

	mfd_assert_size(fd, MFD_DEF_SIZE * 8);
}

static void mfd_fail_grow_write(int fd)
{
	static char buf[MFD_DEF_SIZE * 8];
	ssize_t l;

	l = pwrite(fd, buf, sizeof(buf), 0);
	if (l == sizeof(buf)) {
		printf("pwrite() didn't fail as expected\n");
		abort();
	}
}

static int idle_thread_fn(void *arg)
{
	sigset_t set;
	int sig;

	/* dummy waiter; SIGTERM terminates us anyway */
	sigemptyset(&set);
	sigaddset(&set, SIGTERM);
	sigwait(&set, &sig);

	return 0;
}

static pid_t spawn_idle_thread(unsigned int flags)
{
	uint8_t *stack;
	pid_t pid;

	stack = malloc(STACK_SIZE);
	if (!stack) {
		printf("malloc(STACK_SIZE) failed: %m\n");
		abort();
	}

	pid = clone(idle_thread_fn,
		    stack + STACK_SIZE,
		    SIGCHLD | flags,
		    NULL);
	if (pid < 0) {
		printf("clone() failed: %m\n");
		abort();
	}

	return pid;
}

static void join_idle_thread(pid_t pid)
{
	kill(pid, SIGTERM);
	waitpid(pid, NULL, 0);
}

/*
 * Test memfd_create() syscall
 * Verify syscall-argument validation, including name checks, flag validation
 * and more.
 */
static void test_create(void)
{
	char buf[2048];
	int fd;

	/* test NULL name */
	mfd_fail_new(NULL, 0);

	/* test over-long name (not zero-terminated) */
	memset(buf, 0xff, sizeof(buf));
	mfd_fail_new(buf, 0);

	/* test over-long zero-terminated name */
	memset(buf, 0xff, sizeof(buf));
	buf[sizeof(buf) - 1] = 0;
	mfd_fail_new(buf, 0);

	/* verify "" is a valid name */
	fd = mfd_assert_new("", 0, 0);
	close(fd);

	/* verify invalid O_* open flags */
	mfd_fail_new("", 0x0100);
	mfd_fail_new("", ~MFD_CLOEXEC);
	mfd_fail_new("", ~MFD_ALLOW_SEALING);
	mfd_fail_new("", ~0);
	mfd_fail_new("", 0x80000000U);

	/* verify MFD_CLOEXEC is allowed */
	fd = mfd_assert_new("", 0, MFD_CLOEXEC);
	close(fd);

	/* verify MFD_ALLOW_SEALING is allowed */
	fd = mfd_assert_new("", 0, MFD_ALLOW_SEALING);
	close(fd);

	/* verify MFD_ALLOW_SEALING | MFD_CLOEXEC is allowed */
	fd = mfd_assert_new("", 0, MFD_ALLOW_SEALING | MFD_CLOEXEC);
	close(fd);
}

/*
 * Test basic sealing
 * A very basic sealing test to see whether setting/retrieving seals works.
 */
static void test_basic(void)
{
	int fd;

	fd = mfd_assert_new("kern_memfd_basic",
			    MFD_DEF_SIZE,
			    MFD_CLOEXEC | MFD_ALLOW_SEALING);

	/* add basic seals */
	mfd_assert_has_seals(fd, 0);
	mfd_assert_add_seals(fd, F_SEAL_SHRINK |
				 F_SEAL_WRITE);
	mfd_assert_has_seals(fd, F_SEAL_SHRINK |
				 F_SEAL_WRITE);

	/* add them again */
	mfd_assert_add_seals(fd, F_SEAL_SHRINK |
				 F_SEAL_WRITE);
	mfd_assert_has_seals(fd, F_SEAL_SHRINK |
				 F_SEAL_WRITE);

	/* add more seals and seal against sealing */
	mfd_assert_add_seals(fd, F_SEAL_GROW | F_SEAL_SEAL);
	mfd_assert_has_seals(fd, F_SEAL_SHRINK |
				 F_SEAL_GROW |
				 F_SEAL_WRITE |
				 F_SEAL_SEAL);

	/* verify that sealing no longer works */
	mfd_fail_add_seals(fd, F_SEAL_GROW);
	mfd_fail_add_seals(fd, 0);

	close(fd);

	/* verify sealing does not work without MFD_ALLOW_SEALING */
	fd = mfd_assert_new("kern_memfd_basic",
			    MFD_DEF_SIZE,
			    MFD_CLOEXEC);
	mfd_assert_has_seals(fd, F_SEAL_SEAL);
	mfd_fail_add_seals(fd, F_SEAL_SHRINK |
			       F_SEAL_GROW |
			       F_SEAL_WRITE);
	mfd_assert_has_seals(fd, F_SEAL_SEAL);
	close(fd);
}

/*
 * Test SEAL_WRITE
 * Test whether SEAL_WRITE actually prevents modifications.
 */
static void test_seal_write(void)
{
	int fd;

	fd = mfd_assert_new("kern_memfd_seal_write",
			    MFD_DEF_SIZE,
			    MFD_CLOEXEC | MFD_ALLOW_SEALING);
	mfd_assert_has_seals(fd, 0);
	mfd_assert_add_seals(fd, F_SEAL_WRITE);
	mfd_assert_has_seals(fd, F_SEAL_WRITE);

	mfd_assert_read(fd);
	mfd_fail_write(fd);
	mfd_assert_shrink(fd);
	mfd_assert_grow(fd);
	mfd_fail_grow_write(fd);

	close(fd);
}

/*
 * Test SEAL_SHRINK
 * Test whether SEAL_SHRINK actually prevents shrinking
 */
static void test_seal_shrink(void)
{
	int fd;

	fd = mfd_assert_new("kern_memfd_seal_shrink",
			    MFD_DEF_SIZE,
			    MFD_CLOEXEC | MFD_ALLOW_SEALING);
	mfd_assert_has_seals(fd, 0);
	mfd_assert_add_seals(fd, F_SEAL_SHRINK);
	mfd_assert_has_seals(fd, F_SEAL_SHRINK);

	mfd_assert_read(fd);
	mfd_assert_write(fd);
	mfd_fail_shrink(fd);
	mfd_assert_grow(fd);
	mfd_assert_grow_write(fd);

	close(fd);
}

/*
 * Test SEAL_GROW
 * Test whether SEAL_GROW actually prevents growing
 */
static void test_seal_grow(void)
{
	int fd;

	fd = mfd_assert_new("kern_memfd_seal_grow",
			    MFD_DEF_SIZE,
			    MFD_CLOEXEC | MFD_ALLOW_SEALING);
	mfd_assert_has_seals(fd, 0);
	mfd_assert_add_seals(fd, F_SEAL_GROW);
	mfd_assert_has_seals(fd, F_SEAL_GROW);

	mfd_assert_read(fd);
	mfd_assert_write(fd);
	mfd_assert_shrink(fd);
	mfd_fail_grow(fd);
	mfd_fail_grow_write(fd);

	close(fd);
}

/*
 * Test SEAL_SHRINK | SEAL_GROW
 * Test whether SEAL_SHRINK | SEAL_GROW actually prevents resizing
 */
static void test_seal_resize(void)
{
	int fd;

	fd = mfd_assert_new("kern_memfd_seal_resize",
			    MFD_DEF_SIZE,
			    MFD_CLOEXEC | MFD_ALLOW_SEALING);
	mfd_assert_has_seals(fd, 0);
	mfd_assert_add_seals(fd, F_SEAL_SHRINK | F_SEAL_GROW);
	mfd_assert_has_seals(fd, F_SEAL_SHRINK | F_SEAL_GROW);

	mfd_assert_read(fd);
	mfd_assert_write(fd);
	mfd_fail_shrink(fd);
	mfd_fail_grow(fd);
	mfd_fail_grow_write(fd);

	close(fd);
}

/*
 * Test sharing via dup()
 * Test that seals are shared between dupped FDs and they're all equal.
 */
static void test_share_dup(void)
{
	int fd, fd2;

	fd = mfd_assert_new("kern_memfd_share_dup",
			    MFD_DEF_SIZE,
			    MFD_CLOEXEC | MFD_ALLOW_SEALING);
	mfd_assert_has_seals(fd, 0);

	fd2 = mfd_assert_dup(fd);
	mfd_assert_has_seals(fd2, 0);

	mfd_assert_add_seals(fd, F_SEAL_WRITE);
	mfd_assert_has_seals(fd, F_SEAL_WRITE);
	mfd_assert_has_seals(fd2, F_SEAL_WRITE);

	mfd_assert_add_seals(fd2, F_SEAL_SHRINK);
	mfd_assert_has_seals(fd, F_SEAL_WRITE | F_SEAL_SHRINK);
	mfd_assert_has_seals(fd2, F_SEAL_WRITE | F_SEAL_SHRINK);

	mfd_assert_add_seals(fd, F_SEAL_SEAL);
	mfd_assert_has_seals(fd, F_SEAL_WRITE | F_SEAL_SHRINK | F_SEAL_SEAL);
	mfd_assert_has_seals(fd2, F_SEAL_WRITE | F_SEAL_SHRINK | F_SEAL_SEAL);

	mfd_fail_add_seals(fd, F_SEAL_GROW);
	mfd_fail_add_seals(fd2, F_SEAL_GROW);
	mfd_fail_add_seals(fd, F_SEAL_SEAL);
	mfd_fail_add_seals(fd2, F_SEAL_SEAL);

	close(fd2);

	mfd_fail_add_seals(fd, F_SEAL_GROW);
	close(fd);
}

/*
 * Test sealing with active mmap()s
 * Modifying seals is only allowed if no other mmap() refs exist.
 */
static void test_share_mmap(void)
{
	int fd;
	void *p;

	fd = mfd_assert_new("kern_memfd_share_mmap",
			    MFD_DEF_SIZE,
			    MFD_CLOEXEC | MFD_ALLOW_SEALING);
	mfd_assert_has_seals(fd, 0);

	/* shared/writable ref prevents sealing WRITE, but allows others */
	p = mfd_assert_mmap_shared(fd);
	mfd_fail_add_seals(fd, F_SEAL_WRITE);
	mfd_assert_has_seals(fd, 0);
	mfd_assert_add_seals(fd, F_SEAL_SHRINK);
	mfd_assert_has_seals(fd, F_SEAL_SHRINK);
	munmap(p, MFD_DEF_SIZE);

	/* readable ref allows sealing */
	p = mfd_assert_mmap_private(fd);
	mfd_assert_add_seals(fd, F_SEAL_WRITE);
	mfd_assert_has_seals(fd, F_SEAL_WRITE | F_SEAL_SHRINK);
	munmap(p, MFD_DEF_SIZE);

	close(fd);
}

/*
 * Test sealing with open(/proc/self/fd/%d)
 * Via /proc we can get access to a separate file-context for the same memfd.
 * This is *not* like dup(), but like a real separate open(). Make sure the
 * semantics are as expected and we correctly check for RDONLY / WRONLY / RDWR.
 */
static void test_share_open(void)
{
	int fd, fd2;

	fd = mfd_assert_new("kern_memfd_share_open",
			    MFD_DEF_SIZE,
			    MFD_CLOEXEC | MFD_ALLOW_SEALING);
	mfd_assert_has_seals(fd, 0);

	fd2 = mfd_assert_open(fd, O_RDWR, 0);
	mfd_assert_add_seals(fd, F_SEAL_WRITE);
	mfd_assert_has_seals(fd, F_SEAL_WRITE);
	mfd_assert_has_seals(fd2, F_SEAL_WRITE);

	mfd_assert_add_seals(fd2, F_SEAL_SHRINK);
	mfd_assert_has_seals(fd, F_SEAL_WRITE | F_SEAL_SHRINK);
	mfd_assert_has_seals(fd2, F_SEAL_WRITE | F_SEAL_SHRINK);

	close(fd);
	fd = mfd_assert_open(fd2, O_RDONLY, 0);

	mfd_fail_add_seals(fd, F_SEAL_SEAL);
	mfd_assert_has_seals(fd, F_SEAL_WRITE | F_SEAL_SHRINK);
	mfd_assert_has_seals(fd2, F_SEAL_WRITE | F_SEAL_SHRINK);

	close(fd2);
	fd2 = mfd_assert_open(fd, O_RDWR, 0);

	mfd_assert_add_seals(fd2, F_SEAL_SEAL);
	mfd_assert_has_seals(fd, F_SEAL_WRITE | F_SEAL_SHRINK | F_SEAL_SEAL);
	mfd_assert_has_seals(fd2, F_SEAL_WRITE | F_SEAL_SHRINK | F_SEAL_SEAL);

	close(fd2);
	close(fd);
}

/*
 * Test sharing via fork()
 * Test whether seal-modifications work as expected with forked childs.
 */
static void test_share_fork(void)
{
	int fd;
	pid_t pid;

	fd = mfd_assert_new("kern_memfd_share_fork",
			    MFD_DEF_SIZE,
			    MFD_CLOEXEC | MFD_ALLOW_SEALING);
	mfd_assert_has_seals(fd, 0);

	pid = spawn_idle_thread(0);
	mfd_assert_add_seals(fd, F_SEAL_SEAL);
	mfd_assert_has_seals(fd, F_SEAL_SEAL);

	mfd_fail_add_seals(fd, F_SEAL_WRITE);
	mfd_assert_has_seals(fd, F_SEAL_SEAL);

	join_idle_thread(pid);

	mfd_fail_add_seals(fd, F_SEAL_WRITE);
	mfd_assert_has_seals(fd, F_SEAL_SEAL);

	close(fd);
}

int main(int argc, char **argv)
{
	pid_t pid;

	printf("memfd: CREATE\n");
	test_create();
	printf("memfd: BASIC\n");
	test_basic();

	printf("memfd: SEAL-WRITE\n");
	test_seal_write();
	printf("memfd: SEAL-SHRINK\n");
	test_seal_shrink();
	printf("memfd: SEAL-GROW\n");
	test_seal_grow();
	printf("memfd: SEAL-RESIZE\n");
	test_seal_resize();

	printf("memfd: SHARE-DUP\n");
	test_share_dup();
	printf("memfd: SHARE-MMAP\n");
	test_share_mmap();
	printf("memfd: SHARE-OPEN\n");
	test_share_open();
	printf("memfd: SHARE-FORK\n");
	test_share_fork();

	/* Run test-suite in a multi-threaded environment with a shared
	 * file-table. */
	pid = spawn_idle_thread(CLONE_FILES | CLONE_FS | CLONE_VM);
	printf("memfd: SHARE-DUP (shared file-table)\n");
	test_share_dup();
	printf("memfd: SHARE-MMAP (shared file-table)\n");
	test_share_mmap();
	printf("memfd: SHARE-OPEN (shared file-table)\n");
	test_share_open();
	printf("memfd: SHARE-FORK (shared file-table)\n");
	test_share_fork();
	join_idle_thread(pid);

	printf("memfd: DONE\n");

	return 0;
}
