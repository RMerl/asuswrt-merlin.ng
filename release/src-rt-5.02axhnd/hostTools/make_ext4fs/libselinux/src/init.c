#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <dlfcn.h>

#ifdef DARWIN
#include <sys/param.h>
#include <sys/mount.h>
#else
#include <sys/vfs.h>
#endif

#include <stdint.h>
#include <limits.h>

#include "dso.h"
#include "policy.h"
#include "selinux_internal.h"

char *selinux_mnt = NULL;
int selinux_page_size = 0;

static void init_selinuxmnt(void)
{
	char buf[BUFSIZ], *p;
	FILE *fp=NULL;
	struct statfs sfbuf;
	int rc;
	char *bufp;
	int exists = 0;

	if (selinux_mnt)
		return;

	/* We check to see if the preferred mount point for selinux file
	 * system has a selinuxfs. */
	do {
		rc = statfs(SELINUXMNT, &sfbuf);
	} while (rc < 0 && errno == EINTR);
	if (rc == 0) {
		if ((uint32_t)sfbuf.f_type == (uint32_t)SELINUX_MAGIC) {
			selinux_mnt = strdup(SELINUXMNT);
			return;
		}
	} 

	/* Drop back to detecting it the long way. */
	fp = fopen("/proc/filesystems", "r");
	if (!fp)
		return;

	while ((bufp = fgets(buf, sizeof buf - 1, fp)) != NULL) {
		if (strstr(buf, "selinuxfs")) {
			exists = 1;
			break;
		}
	}

	if (!exists) 
		goto out;

	fclose(fp);

	/* At this point, the usual spot doesn't have an selinuxfs so
	 * we look around for it */
	fp = fopen("/proc/mounts", "r");
	if (!fp)
		goto out;

	while ((bufp = fgets(buf, sizeof buf - 1, fp)) != NULL) {
		char *tmp;
		p = strchr(buf, ' ');
		if (!p)
			goto out;
		p++;
		tmp = strchr(p, ' ');
		if (!tmp)
			goto out;
		if (!strncmp(tmp + 1, "selinuxfs ", 10)) {
			*tmp = '\0';
			break;
		}
	}

	/* If we found something, dup it */
	if (bufp)
		selinux_mnt = strdup(p);

      out:
	if (fp)
		fclose(fp);
	return;
}

void fini_selinuxmnt(void)
{
	free(selinux_mnt);
	selinux_mnt = NULL;
}

void set_selinuxmnt(char *mnt)
{
	selinux_mnt = strdup(mnt);
}

hidden_def(set_selinuxmnt)

static void init_lib(void) __attribute__ ((constructor));
static void init_lib(void)
{
	selinux_page_size = sysconf(_SC_PAGE_SIZE);
	init_selinuxmnt();
}

static void fini_lib(void) __attribute__ ((destructor));
static void fini_lib(void)
{
	fini_selinuxmnt();
}
