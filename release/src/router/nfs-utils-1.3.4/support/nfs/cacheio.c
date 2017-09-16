/*
 * support/nfs/cacheio.c
 * support IO on the cache channel files in 2.5 and beyond.
 * These use 'qwords' which are like words, but with a little quoting.
 *
 */


/*
 * Support routines for text-based upcalls.
 * Fields are separated by spaces.
 * Fields are either mangled to quote space tab newline slosh with slosh
 * or a hexified with a leading \x
 * Record is terminated with newline.
 *
 */

#include <nfslib.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

void qword_add(char **bpp, int *lp, char *str)
{
	char *bp = *bpp;
	int len = *lp;
	char c;

	if (len < 0) return;

	while ((c=*str++) && len)
		switch(c) {
		case ' ':
		case '\t':
		case '\n':
		case '\\':
			if (len >= 4) {
				*bp++ = '\\';
				*bp++ = '0' + ((c & 0300)>>6);
				*bp++ = '0' + ((c & 0070)>>3);
				*bp++ = '0' + ((c & 0007)>>0);
			}
			len -= 4;
			break;
		default:
			*bp++ = c;
			len--;
		}
	if (c || len <1) len = -1;
	else {
		*bp++ = ' ';
		len--;
	}
	*bpp = bp;
	*lp = len;
}

void qword_addhex(char **bpp, int *lp, char *buf, int blen)
{
	char *bp = *bpp;
	int len = *lp;

	if (len < 0) return;

	if (len > 2) {
		*bp++ = '\\';
		*bp++ = 'x';
		len -= 2;
		while (blen && len >= 2) {
			unsigned char c = *buf++;
			*bp++ = '0' + ((c&0xf0)>>4) + (c>=0xa0)*('a'-'9'-1);
			*bp++ = '0' + (c&0x0f) + ((c&0x0f)>=0x0a)*('a'-'9'-1);
			len -= 2;
			blen--;
		}
	}
	if (blen || len<1) len = -1;
	else {
		*bp++ = ' ';
		len--;
	}
	*bpp = bp;
	*lp = len;
}

void qword_addint(char **bpp, int *lp, int n)
{
	int len;

	len = snprintf(*bpp, *lp, "%d ", n);
	if (len > *lp)
		len = *lp;
	*bpp += len;
	*lp -= len;
}

void qword_adduint(char **bpp, int *lp, unsigned int n)
{
	int len;

	len = snprintf(*bpp, *lp, "%u ", n);
	if (len > *lp)
		len = *lp;
	*bpp += len;
	*lp -= len;
}

void qword_addeol(char **bpp, int *lp)
{
	if (*lp <= 0)
		return;
	**bpp = '\n';
	(*bpp)++;
	(*lp)--;
}

#define isodigit(c) (isdigit(c) && c <= '7')
int qword_get(char **bpp, char *dest, int bufsize)
{
	/* return bytes copied, or -1 on error */
	char *bp = *bpp;
	int len = 0;

	while (*bp == ' ') bp++;

	if (bp[0] == '\\' && bp[1] == 'x') {
		/* HEX STRING */
		bp += 2;
		while (isxdigit(bp[0]) && isxdigit(bp[1]) && len < bufsize) {
			int byte = isdigit(*bp) ? *bp-'0' : toupper(*bp)-'A'+10;
			bp++;
			byte <<= 4;
			byte |= isdigit(*bp) ? *bp-'0' : toupper(*bp)-'A'+10;
			*dest++ = byte;
			bp++;
			len++;
		}
	} else {
		/* text with \nnn octal quoting */
		while (*bp != ' ' && *bp != '\n' && *bp && len < bufsize-1) {
			if (*bp == '\\' &&
			    isodigit(bp[1]) && (bp[1] <= '3') &&
			    isodigit(bp[2]) &&
			    isodigit(bp[3])) {
				int byte = (*++bp -'0');
				bp++;
				byte = (byte << 3) | (*bp++ - '0');
				byte = (byte << 3) | (*bp++ - '0');
				*dest++ = byte;
				len++;
			} else {
				*dest++ = *bp++;
				len++;
			}
		}
	}

	if (*bp != ' ' && *bp != '\n' && *bp != '\0')
		return -1;
	while (*bp == ' ') bp++;
	*bpp = bp;
	*dest = '\0';
	return len;
}

int qword_get_int(char **bpp, int *anint)
{
	char buf[50];
	char *ep;
	int rv;
	int len = qword_get(bpp, buf, 50);
	if (len < 0) return -1;
	if (len ==0) return -1;
	rv = strtol(buf, &ep, 0);
	if (*ep) return -1;
	*anint = rv;
	return 0;
}

int qword_get_uint(char **bpp, unsigned int *anint)
{
	char buf[50];
	char *ep;
	unsigned int rv;
	int len = qword_get(bpp, buf, 50);
	if (len < 0) return -1;
	if (len ==0) return -1;
	rv = strtoul(buf, &ep, 0);
	if (*ep) return -1;
	*anint = rv;
	return 0;
}

/* Check if we should use the new caching interface
 * This succeeds iff the "nfsd" filesystem is mounted on
 * /proc/fs/nfs
 */
int
check_new_cache(void)
{
	return	(access("/proc/fs/nfs/filehandle", F_OK) == 0) ||
		(access("/proc/fs/nfsd/filehandle", F_OK) == 0);
}	


/* flush the kNFSd caches.
 * Set the flush time to the mtime of _PATH_ETAB or
 * if force, to now.
 * the caches to flush are:
 *  auth.unix.ip nfsd.export nfsd.fh
 */

void
cache_flush(int force)
{
	struct stat stb;
	int c;
	char stime[20];
	char path[200];
	time_t now;
	/* Note: the order of these caches is important.
	 * They need to be flushed in dependancy order. So
	 * a cache that references items in another cache,
	 * as nfsd.fh entries reference items in nfsd.export,
	 * must be flushed before the cache that it references.
	 */
	static char *cachelist[] = {
		"auth.unix.ip",
		"auth.unix.gid",
		"nfsd.fh",
		"nfsd.export",
		NULL
	};
	now = time(0);
	if (force ||
	    stat(_PATH_ETAB, &stb) != 0 ||
	    stb.st_mtime > now)
		stb.st_mtime = time(0);
	
	sprintf(stime, "%ld\n", stb.st_mtime);
	for (c=0; cachelist[c]; c++) {
		int fd;
		sprintf(path, "/proc/net/rpc/%s/flush", cachelist[c]);
		fd = open(path, O_RDWR);
		if (fd >= 0) {
			if (write(fd, stime, strlen(stime)) != (ssize_t)strlen(stime)) {
				xlog_warn("Writing to '%s' failed: errno %d (%s)",
				path, errno, strerror(errno));
			}
			close(fd);
		}
	}
}
