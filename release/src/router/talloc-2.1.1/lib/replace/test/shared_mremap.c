/* this tests whether we can use mremap */

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DATA "conftest.mmap"

#ifndef MAP_FILE
#define MAP_FILE 0
#endif

#ifndef MAP_FAILED
#define MAP_FAILED (int *)-1
#endif

main()
{
	int *buf;
	int fd;
	int err = 1;

	fd = open(DATA, O_RDWR|O_CREAT|O_TRUNC, 0666);
	if (fd == -1) {
		exit(1);
	}

	buf = (int *)mmap(NULL, 0x1000, PROT_READ | PROT_WRITE,
			  MAP_FILE | MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED) {
		goto done;
	}

	buf = mremap(buf, 0x1000, 0x2000, MREMAP_MAYMOVE);
	if (buf == MAP_FAILED) {
		goto done;
	}

	err = 0;
done:
	close(fd);
	unlink(DATA);
	exit(err);
}
