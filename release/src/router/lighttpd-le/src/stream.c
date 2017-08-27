#include "first.h"

#include "stream.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>

#include "sys-mmap.h"

#ifndef O_BINARY
# define O_BINARY 0
#endif

/* don't want to block when open()ing a fifo */
#if defined(O_NONBLOCK)
# define FIFO_NONBLOCK O_NONBLOCK
#else
# define FIFO_NONBLOCK 0
#endif

int stream_open(stream *f, const buffer *fn) {

#if !defined(__WIN32)

	struct stat st;
	int fd;

	f->start = NULL;
	f->size = 0;
	f->mapped = 0;

	if (-1 == (fd = open(fn->ptr, O_RDONLY | O_BINARY | FIFO_NONBLOCK))) {
		return -1;
	}

	if (-1 == fstat(fd, &st)) {
		close(fd);
		return -1;
	}

	if (0 == st.st_size) {
		/* empty file doesn't need a mapping */
		close(fd);
		return 0;
	}

	f->start = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);

	if (MAP_FAILED == f->start) {
		f->start = malloc((size_t)st.st_size);
		if (NULL == f->start
		    || st.st_size != read(fd, f->start, (size_t)st.st_size)) {
			free(f->start);
			f->start = NULL;
			close(fd);
			return -1;
		}
	} else {
		f->mapped = 1;
	}

	close(fd);

	f->size = st.st_size;
	return 0;

#elif defined __WIN32

	HANDLE *fh, *mh;
	void *p;
	LARGE_INTEGER fsize;

	f->start = NULL;
	f->size = 0;

	fh = CreateFile(fn->ptr,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_READONLY,
			NULL);

	if (!fh) return -1;

	if (0 != GetFileSizeEx(fh, &fsize)) {
		CloseHandle(fh);
		return -1;
	}

	if (0 == fsize) {
		CloseHandle(fh);
		return 0;
	}

	mh = CreateFileMapping( fh,
			NULL,
			PAGE_READONLY,
			(sizeof(off_t) > 4) ? fsize >> 32 : 0,
			fsize & 0xffffffff,
			NULL);

	if (!mh) {
/*
		LPVOID lpMsgBuf;
		FormatMessage(
		        FORMAT_MESSAGE_ALLOCATE_BUFFER |
		        FORMAT_MESSAGE_FROM_SYSTEM,
		        NULL,
		        GetLastError(),
		        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		        (LPTSTR) &lpMsgBuf,
		        0, NULL );
*/
		CloseHandle(fh);
		return -1;
	}

	p = MapViewOfFile(mh,
			FILE_MAP_READ,
			0,
			0,
			0);
	CloseHandle(mh);
	CloseHandle(fh);

	f->start = p;
	f->size = (off_t)fsize;
	return 0;

#endif

}

int stream_close(stream *f) {
#ifdef HAVE_MMAP
	if (f->start) {
		if (f->mapped) {
			f->mapped = 0;
			munmap(f->start, f->size);
		} else {
			free(f->start);
		}
	}
#elif defined(__WIN32)
	if (f->start) UnmapViewOfFile(f->start);
#endif

	f->start = NULL;
	f->size = 0;

	return 0;
}
