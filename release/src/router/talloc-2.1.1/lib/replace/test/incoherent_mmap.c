/* In OpenBSD, if you write to a file, another process doesn't see it
 * in its mmap.  Returns with exit status 0 if that is the case, 1 if
 * it's coherent, and other if there's a problem. */
#include <err.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DATA "coherent.mmap"

int main(int argc, char *argv[])
{
	int tochild[2], toparent[2];
	int fd;
	volatile unsigned char *map;
	unsigned char *page;
        const char *fname = argv[1];
	char c = 0;

	if (pipe(tochild) != 0 || pipe(toparent) != 0)
		err(2, "Creating pipe");

	if (!fname)
		fname = DATA;

	fd = open(fname, O_RDWR|O_CREAT|O_TRUNC, 0600);
	if (fd < 0)
		err(2, "opening %s", fname);
	unlink(fname);

	switch (fork()) {
	case -1:
		err(2, "Fork");
	case 0:
		close(tochild[1]);
		close(toparent[0]);

		/* Wait for parent to create file. */
		if (read(tochild[0], &c, 1) != 1)
			err(2, "reading from parent");

		/* Alter first byte. */
		pwrite(fd, &c, 1, 0);

		if (write(toparent[1], &c, 1) != 1)
			err(2, "writing to parent");
		exit(0);

	default:
		close(tochild[0]);
		close(toparent[1]);

		/* Create a file and mmap it. */
		page = malloc(getpagesize());
		memset(page, 0x42, getpagesize());
		if (write(fd, page, getpagesize()) != getpagesize())
			err(2, "writing first page");
		map = mmap(NULL, getpagesize(), PROT_READ|PROT_WRITE,
			   MAP_SHARED, fd, 0);
		if (map == MAP_FAILED)
			err(2, "mapping file");

		if (*map != 0x42)
			errx(2, "first byte isn't 0x42!");

		/* Tell child to alter file. */
		if (write(tochild[1], &c, 1) != 1)
			err(2, "writing to child");

		if (read(toparent[0], &c, 1) != 1)
			err(2, "reading from child");

		if (*map)
			errx(0, "mmap incoherent: first byte isn't 0.");

		exit(1);
	}
}
