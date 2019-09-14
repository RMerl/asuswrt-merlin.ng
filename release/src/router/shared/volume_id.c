#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <errno.h>

/****************************************************/
/* Use busybox routines to get labels for fat & ext */
/* Probe for label the same way that mount does.    */
/****************************************************/
#include "autoconf.h"
#include "volume_id_internal.h"
#include <rtconfig.h>

#ifdef DEBUG_NOISY
extern void cprintf(const char *format, ...);
#define _dprintf		cprintf
#define csprintf		cprintf
#else
#define _dprintf(args...)	do { } while(0)
#define csprintf(args...)	do { } while(0)
#endif

#ifdef __GLIBC__
#undef errno
#define errno (*__errno_location ())
#endif

#pragma GCC visibility push(hidden)

#ifndef DMALLOC
void* FAST_FUNC xmalloc(size_t size)
{
	return malloc(size);
}

void* FAST_FUNC xrealloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}
#endif /* DMALLOC */

ssize_t FAST_FUNC safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

/*
 * Read all of the supplied buffer from a file.
 * This does multiple reads as necessary.
 * Returns the amount read, or -1 on an error.
 * A short read is returned on an end of file.
 */
ssize_t FAST_FUNC full_read(int fd, void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;

	total = 0;

	while (len) {
		cc = safe_read(fd, buf, len);

		if (cc < 0) {
			if (total) {
				/* we already have some! */
				/* user can do another read to know the error code */
				return total;
			}
			return cc; /* read() returns -1 on failure. */
		}
		if (cc == 0)
			break;
		buf = ((char *)buf) + cc;
		total += cc;
		len -= cc;
	}

	return total;
}

#pragma GCC visibility pop

typedef char *(*probefunc)(struct volume_id *id, char *dev);

static char *probe_mbr(struct volume_id *id, char *dev)
{
	unsigned char *buf;

	buf = (unsigned char *)dev + strlen(dev) - 1;
	if (buf > (unsigned char *)dev && *buf <= '9')
		return NULL;

	buf = volume_id_get_buffer(id, 0, 0x200);
	if (buf == NULL)
		return NULL;

	/* signature */
	if (buf[0x1fe] != 0x55 || buf[0x1ff] != 0xaa)
		return NULL;

	/* boot flags */
	if ((buf[0x1be] | buf[0x1ce] | buf[0x1de] | buf[0x1ee]) & 0x7f)
		return NULL;

	return "mbr";
}

static char *probe_vfat(struct volume_id *id, char *dev)
{
	return volume_id_probe_vfat(id) ? NULL : "vfat";
}

static char *probe_swap(struct volume_id *id, char *dev)
{
	return volume_id_probe_linux_swap(id) ? NULL : "swap";
}

static char *probe_ext(struct volume_id *id, char *dev)
{
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL		0x0004
#define EXT4_FEATURE_RO_COMPAT_HUGE_FILE	0x0008
#define EXT4_FEATURE_RO_COMPAT_DIR_NLINK	0x0020
#define EXT4_FEATURE_INCOMPAT_EXTENTS		0x0040
#define EXT4_FEATURE_INCOMPAT_64BIT		0x0080

	unsigned char *buf;

	if (volume_id_probe_ext(id) != 0)
		return NULL;

	buf = volume_id_get_buffer(id, 0x400, 0x200);
	if (buf == NULL)
		return NULL;

	if (*(uint32_t *)(buf + 0x64) & cpu_to_le32(EXT4_FEATURE_RO_COMPAT_HUGE_FILE | EXT4_FEATURE_RO_COMPAT_DIR_NLINK) ||
	    *(uint32_t *)(buf + 0x60) & cpu_to_le32(EXT4_FEATURE_INCOMPAT_EXTENTS | EXT4_FEATURE_INCOMPAT_64BIT))
		return "ext4";

	if (*(uint32_t *)(buf + 0x5c) & cpu_to_le32(EXT3_FEATURE_COMPAT_HAS_JOURNAL))
		return "ext3";

	return "ext2";
}

static char *probe_ntfs(struct volume_id *id, char *dev)
{
	return volume_id_probe_ntfs(id) ? NULL : "ntfs";
}

#ifdef RTCONFIG_HFS
static char *probe_hfs(struct volume_id *id, char *dev)
{
	unsigned char *buf;
	unsigned sig = 0;

	if (volume_id_probe_hfs_hfsplus(id) != 0)
		return NULL;

	buf = volume_id_get_buffer(id, 0x400, 0x200);
	if (buf == NULL)
		return NULL;

	if (buf[0] == 'B' && buf[1] == 'D')
		sig += 124;

	if (buf[sig] == 'H') {
		if (buf[sig + 1] == '+') /* ordinary HFS+ */
			return "hfs+j";
		if (buf[sig + 1] == 'X') /* case-sensitive HFS+ */
			return "hfs+jx";
	}

	return sig ? "hfs" : NULL;
}
#endif

#ifdef RTCONFIG_USB_CDROM
static char *probe_udf(struct volume_id *id, char *dev)
{
	return volume_id_probe_udf(id) ? NULL : "udf";
}

static char *probe_iso(struct volume_id *id, char *dev)
{
	return volume_id_probe_iso9660(id) ? NULL : "iso9660";
}
#endif

/* Probe FS type and fill FS type, label and uuid, if required.
 * Return >0 if FS is detected or 0 if unknown.
 * Return -1 on error or no device found.
 */
int probe_fs(char *device, char **type, char *label, char *uuid)
{
	static const probefunc probe[] = {
		probe_mbr,
		probe_vfat,
	};
	static const probefunc probe_full[] = {
#ifdef RTCONFIG_USB_CDROM
		probe_udf,
		probe_iso,
#endif
		probe_swap,
		probe_ext,
		probe_ntfs,
#ifdef RTCONFIG_HFS
		probe_hfs,
#endif
	};
	struct volume_id id;
	char *fstype = NULL;
	int i;

	memset(&id, 0, sizeof(id));
	if (type) *type = NULL;
	if (label) *label = '\0';
	if (uuid) *uuid = '\0';

	if ((id.fd = open(device, O_RDONLY)) < 0) {
		_dprintf("%s: open %s failed: %s", __FUNCTION__, device, strerror(errno));
		return -1;
	}

	/* signature in the first block, only small buffer needed */
	for (i = 0; i < ARRAY_SIZE(probe); i++) {
		fstype = probe[i](&id, device);
		if (fstype || id.error)
			goto found;
	}

	/* fill buffer with maximum */
	volume_id_get_buffer(&id, 0, SB_BUFFER_SIZE);

	for (i = 0; i < ARRAY_SIZE(probe_full); i++) {
		fstype = probe_full[i](&id, device);
		if (fstype || id.error)
			goto found;
	}

found:
	volume_id_free_buffer(&id);
	close(id.fd);

	if (fstype == NULL && id.error) {
		_dprintf("%s: probe %s failed: %s", __FUNCTION__, device, strerror(errno));
		return -1;
	}

	if (type)
		*type = fstype;
	if (label && *id.label != 0)
		strcpy(label, id.label);
	if (uuid && *id.uuid != 0)
		strcpy(uuid, id.uuid);

	return fstype ? 1 : 0;
}

/* Detect FS type.
 * Return FS type or "unknown" if not known/detected.
 * Return NULL on error or no device found.
 */
char *detect_fs_type(char *device)
{
	char *type;

	if (probe_fs(device, &type, NULL, NULL) < 0)
		return NULL;

	return type ? : "unknown";
}

/* Put the label in *label and uuid in *uuid.
 * Return >0 if there is a label/uuid or 0 if both are absent.
 * Return -1 on error or no device found.
 */
int find_label_or_uuid(char *device, char *label, char *uuid)
{
	if (probe_fs(device, NULL, label, uuid) < 0)
		return -1;

	return (label && *label) || (uuid && *uuid);
}
