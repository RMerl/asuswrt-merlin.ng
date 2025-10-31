/* libcap-ng.c --
 * Copyright 2009-10, 2013, 2017, 2020-21 Red Hat Inc.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; see the file COPYING.LIB. If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
 * Boston, MA 02110-1335, USA.
 *
 * Authors:
 *      Steve Grubb <sgrubb@redhat.com>
 */

#include "config.h"
#include "cap-ng.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <endian.h>
#include <byteswap.h>
#ifdef HAVE_PTHREAD_H
#include <pthread.h>	// For pthread_atfork
#endif
#ifdef HAVE_SYSCALL_H
#include <sys/syscall.h>
#endif
#ifdef HAVE_LINUX_SECUREBITS_H
#include <linux/securebits.h>
#endif
#ifdef HAVE_LINUX_MAGIC_H
#include <sys/vfs.h>
#include <linux/magic.h>
#endif

# define hidden __attribute__ ((visibility ("hidden")))
unsigned int last_cap hidden = 0;
/*
 * Some milestones of when things became available:
 * 2.6.24 kernel	XATTR_NAME_CAPS
 * 2.6.25 kernel	PR_CAPBSET_DROP, CAPABILITY_VERSION_2
 * 2.6.26 kernel	PR_SET_SECUREBITS, SECURE_*_LOCKED, VERSION_3
 * 3.5    kernel	PR_SET_NO_NEW_PRIVS
 * 4.3    kernel	PR_CAP_AMBIENT
 * 4.14   kernel	VFS_CAP_REVISION_3
 */
#ifdef PR_CAPBSET_DROP
static int HAVE_PR_CAPBSET_DROP = 0;
#endif
#ifdef PR_SET_SECUREBITS
static int HAVE_PR_SET_SECUREBITS = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
static int HAVE_PR_SET_NO_NEW_PRIVS = 0;
#endif
#ifdef PR_CAP_AMBIENT
static int HAVE_PR_CAP_AMBIENT = 0;
#endif

/* External syscall prototypes */
extern int capset(cap_user_header_t header, cap_user_data_t data);
extern int capget(cap_user_header_t header, const cap_user_data_t data);

// Local functions
static void update_bounding_set(capng_act_t action, unsigned int capability,
	unsigned int idx);
static void update_ambient_set(capng_act_t action, unsigned int capability,
	unsigned int idx);

// Local defines
#define MASK(x) (1U << (x))
#ifdef PR_CAPBSET_DROP
#define UPPER_MASK ~((~0U)<<(last_cap-31))
#else
// For v1 systems UPPER_MASK will never be used
#define UPPER_MASK (unsigned)(~0U)
#endif

// Re-define cap_valid so its uniform between V1 and V3
#undef cap_valid
#define cap_valid(x) ((x) <= last_cap)

// If we don't have the xattr library, then we can't
// compile-in file system capabilities
#if !defined(HAVE_ATTR_XATTR_H) && !defined (HAVE_SYS_XATTR_H)
#undef VFS_CAP_U32
#endif

#ifdef VFS_CAP_U32
 #ifdef HAVE_SYS_XATTR_H
   #include <sys/xattr.h>
 #else
  #ifdef HAVE_ATTR_XATTR_H
   #include <attr/xattr.h>
  #endif
 #endif
 #if __BYTE_ORDER == __BIG_ENDIAN
  #define FIXUP(x) bswap_32(x)
 #else
  #define FIXUP(x) (x)
 #endif
#endif

#ifndef _LINUX_CAPABILITY_VERSION_1
#define _LINUX_CAPABILITY_VERSION_1 0x19980330
#endif
#ifndef _LINUX_CAPABILITY_VERSION_2
#define _LINUX_CAPABILITY_VERSION_2 0x20071026
#endif
#ifndef _LINUX_CAPABILITY_VERSION_3
#define _LINUX_CAPABILITY_VERSION_3 0x20080522
#endif

// This public API went private in the 2.6.36 kernel - hope it never changes
#ifndef XATTR_CAPS_SUFFIX
#define XATTR_CAPS_SUFFIX "capability"
#endif
#ifndef XATTR_SECURITY_PREFIX
#define XATTR_SECURITY_PREFIX "security."
#endif
#ifndef XATTR_NAME_CAPS
#define XATTR_NAME_CAPS XATTR_SECURITY_PREFIX XATTR_CAPS_SUFFIX
#endif


/* Child processes can't get caps back */
#ifndef SECURE_NOROOT
#define SECURE_NOROOT                   0
#endif
#ifndef SECURE_NOROOT_LOCKED
#define SECURE_NOROOT_LOCKED            1  /* make bit-0 immutable */
#endif
/* Setuid apps run by uid 0 don't get caps back */
#ifndef SECURE_NO_SETUID_FIXUP
#define SECURE_NO_SETUID_FIXUP          2
#endif
#ifndef SECURE_NO_SETUID_FIXUP_LOCKED
#define SECURE_NO_SETUID_FIXUP_LOCKED   3  /* make bit-2 immutable */
#endif

#ifndef VFS_CAP_U32
#define VFS_CAP_U32 2
#endif

#if (VFS_CAP_U32 != 2)
#error VFS_CAP_U32 does not match the library, you need a new version
#endif


// States: new, allocated, initted, updated, applied
typedef enum { CAPNG_NEW, CAPNG_ERROR, CAPNG_ALLOCATED, CAPNG_INIT,
	CAPNG_UPDATED, CAPNG_APPLIED } capng_states_t;

// Create an easy data struct out of the kernel definitions
typedef union {
	struct __user_cap_data_struct v1;
	struct __user_cap_data_struct v3[VFS_CAP_U32];
} cap_data_t;

// This struct keeps all state info
struct cap_ng
{
	int cap_ver;
	int vfs_cap_ver;
	struct __user_cap_header_struct hdr;
	cap_data_t data;
	capng_states_t state;
	__le32 rootid;
	__u32 bounds[VFS_CAP_U32];
	__u32 ambient[VFS_CAP_U32];
};

// Global variables with per thread uniqueness
static __thread struct cap_ng m =	{ 1, 1,
					{0, 0},
					{ {0, 0, 0} },
					CAPNG_NEW, CAPNG_UNSET_ROOTID,
					{0, 0},
					{0, 0} };

/*
 * Reset the state so that init gets called to erase everything
 */
static void deinit(void)
{
	m.state = CAPNG_NEW;
}

static inline int test_cap(unsigned int cap)
{
	// prctl returns 0 or 1 for valid caps, -1 otherwise
	return prctl(PR_CAPBSET_READ, cap) >= 0;
}

// The maximum cap value is determined by VFS_CAP_U32
#define MAX_CAP_VALUE (VFS_CAP_U32 * sizeof(__le32) * 8)

static void init_lib(void) __attribute__ ((constructor));
static void init_lib(void)
{
       // This is so dynamic libraries don't re-init
       static unsigned int run_once = 0;
       if (run_once)
               return;
       run_once = 1;

#ifdef HAVE_PTHREAD_H
	pthread_atfork(NULL, NULL, deinit);
#endif
	// Detect last cap
	if (last_cap == 0) {
		int fd;

		// Try to read last cap from procfs
		fd = open("/proc/sys/kernel/cap_last_cap", O_RDONLY);
		if (fd >= 0) {
#ifdef HAVE_LINUX_MAGIC_H
			struct statfs st;
			// Bail out if procfs is invalid or fstatfs fails
			if (fstatfs(fd, &st) || st.f_type != PROC_SUPER_MAGIC)
				goto fail;
#endif
			char buf[8];
			int num = read(fd, buf, sizeof(buf) - 1);
			if (num > 0) {
				buf[num] = 0;
				errno = 0;
				unsigned int val = strtoul(buf, NULL, 10);
				if (errno == 0)
					last_cap = val;
			}
fail:
			close(fd);
		}
		// Run a binary search over capabilities
		if (last_cap == 0) {
			// starting with last_cap=MAX_CAP_VALUE means we always know
			// that cap1 is invalid after the first iteration
			last_cap = MAX_CAP_VALUE;
			unsigned int cap0 = 0, cap1 = MAX_CAP_VALUE;

			while (cap0 < last_cap) {
				if (test_cap(last_cap))
					cap0 = last_cap;
				else
					cap1 = last_cap;

				last_cap = (cap0 + cap1) / 2U;
			}
		}
	}
	// Detect prctl options at runtime
#ifdef PR_CAPBSET_DROP
	errno = 0;
	prctl(PR_CAPBSET_READ, 0, 0, 0, 0);
	if (!errno)
		HAVE_PR_CAPBSET_DROP = 1;
#endif
#ifdef PR_SET_SECUREBITS
	errno = 0;
	prctl(PR_GET_SECUREBITS, 0, 0, 0, 0);
	if (!errno)
		HAVE_PR_SET_SECUREBITS = 1;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	errno = 0;
	prctl(PR_GET_NO_NEW_PRIVS, 0, 0, 0, 0);
	if (!errno)
		HAVE_PR_SET_NO_NEW_PRIVS = 1;
#endif
#ifdef PR_CAP_AMBIENT
	errno = 0;
	prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_IS_SET, 0, 0, 0);
	if (!errno)
		HAVE_PR_CAP_AMBIENT = 1;
#endif
}

static void init(void)
{
	// This is so static libs get initialized
	init_lib();

	if (m.state != CAPNG_NEW)
		return;

	memset(&m.hdr, 0, sizeof(m.hdr));
	(void)capget(&m.hdr, NULL); // Returns -EINVAL
	if (m.hdr.version == _LINUX_CAPABILITY_VERSION_3 ||
		m.hdr.version == _LINUX_CAPABILITY_VERSION_2) {
		m.cap_ver = 3;
	} else if (m.hdr.version == _LINUX_CAPABILITY_VERSION_1) {
		m.cap_ver = 1;
	} else {
		m.state = CAPNG_ERROR;
		return;
	}

#if VFS_CAP_REVISION == VFS_CAP_REVISION_1
	m.vfs_cap_ver = 1;
#else
	m.vfs_cap_ver = 2; // Intentionally set to 2 for both 2 & 3
#endif

	memset(&m.data, 0, sizeof(cap_data_t));
#ifdef HAVE_SYSCALL_H
	m.hdr.pid = (unsigned)syscall(__NR_gettid);
#else
	m.hdr.pid = (unsigned)getpid();
#endif
	m.rootid = CAPNG_UNSET_ROOTID;
	m.state = CAPNG_ALLOCATED;
}

void capng_clear(capng_select_t set)
{
	if (m.state == CAPNG_NEW)
		init();
	if (m.state == CAPNG_ERROR)
		return;

	if (set & CAPNG_SELECT_CAPS)
		memset(&m.data, 0, sizeof(cap_data_t));
#ifdef PR_CAPBSET_DROP
if (HAVE_PR_CAPBSET_DROP) {
	if (set & CAPNG_SELECT_BOUNDS)
		memset(m.bounds, 0, sizeof(m.bounds));
}
#endif
#ifdef PR_CAP_AMBIENT
if (HAVE_PR_CAP_AMBIENT) {
	if (set & CAPNG_SELECT_AMBIENT)
		memset(m.ambient, 0, sizeof(m.ambient));
}
#endif
	m.state = CAPNG_INIT;
}

void capng_fill(capng_select_t set)
{
	if (m.state == CAPNG_NEW)
		init();
	if (m.state == CAPNG_ERROR)
		return;

	if (set & CAPNG_SELECT_CAPS) {
		if (m.cap_ver == 1) {
			m.data.v1.effective = 0x7FFFFFFFU;
			m.data.v1.permitted = 0x7FFFFFFFU;
			m.data.v1.inheritable = 0;
		} else {
			m.data.v3[0].effective = 0xFFFFFFFFU;
			m.data.v3[0].permitted = 0xFFFFFFFFU;
			m.data.v3[0].inheritable = 0;
			m.data.v3[1].effective = 0xFFFFFFFFU;
			m.data.v3[1].permitted = 0xFFFFFFFFU;
			m.data.v3[1].inheritable = 0;
		}
	}
#ifdef PR_CAPBSET_DROP
if (HAVE_PR_CAPBSET_DROP) {
	if (set & CAPNG_SELECT_BOUNDS) {
		unsigned i;
		for (i=0; i<sizeof(m.bounds)/sizeof(__u32); i++)
			m.bounds[i] = 0xFFFFFFFFU;
	}
}
#endif
#ifdef PR_CAP_AMBIENT
if (HAVE_PR_CAP_AMBIENT) {
	if (set & CAPNG_SELECT_AMBIENT) {
		unsigned i;
		for (i=0; i<sizeof(m.ambient)/sizeof(__u32); i++)
			m.ambient[i] = 0xFFFFFFFFU;
	}
}
#endif
	m.state = CAPNG_INIT;
}

void capng_setpid(int pid)
{
	if (m.state == CAPNG_NEW)
		init();
	if (m.state == CAPNG_ERROR)
		return;

	m.hdr.pid = pid;
}

int capng_get_rootid(void)
{
#ifdef VFS_CAP_REVISION_3
	return m.rootid;
#else
	return CAPNG_UNSET_ROOTID;
#endif
}

int capng_set_rootid(int rootid)
{
#ifdef VFS_CAP_REVISION_3
	if (m.state == CAPNG_NEW)
		init();
	if (m.state == CAPNG_ERROR)
		return -1;

	if (rootid < 0)
		return -1;

	m.rootid = rootid;
	m.vfs_cap_ver = 3;

	return 0;
#else
	return -1;
#endif
}

#ifdef PR_CAPBSET_DROP
static int get_bounding_set(void)
{
	char buf[64];
	FILE *f;
	int rc;

	snprintf(buf, sizeof(buf), "/proc/%d/status", m.hdr.pid ? m.hdr.pid :
#ifdef HAVE_SYSCALL_H
		(int)syscall(__NR_gettid));
#else
		(int)getpid();
#endif
	f = fopen(buf, "re");
	if (f) {
		__fsetlocking(f, FSETLOCKING_BYCALLER);
		while (fgets(buf, sizeof(buf), f)) {
			if (strncmp(buf, "CapB", 4))
				continue;
			sscanf(buf, "CapBnd:  %08x%08x",
			       &m.bounds[1], &m.bounds[0]);
			fclose(f);
			return 0;
		}
		// Didn't find bounding set, fall through and try prctl way
		fclose(f);
	}
	// Might be in a container with no procfs - do it the hard way
	memset(m.bounds, 0, sizeof(m.bounds));
	unsigned int i = 0;
	do {
		rc = prctl(PR_CAPBSET_READ, i, 0, 0, 0);
		if (rc < 0)
			return -1;

		// Just add set bits
		if (rc)
			update_bounding_set(CAPNG_ADD, i%32, i>>5);
		i++;
	} while (cap_valid(i));

	return 0;
}
#endif

#ifdef PR_CAP_AMBIENT
static int get_ambient_set(void)
{
	char buf[64];
	FILE *f;
	int rc;

	snprintf(buf, sizeof(buf), "/proc/%d/status", m.hdr.pid ? m.hdr.pid :
#ifdef HAVE_SYSCALL_H
		(int)syscall(__NR_gettid));
#else
		(int)getpid();
#endif
	f = fopen(buf, "re");
	if (f) {
		__fsetlocking(f, FSETLOCKING_BYCALLER);
		while (fgets(buf, sizeof(buf), f)) {
			if (strncmp(buf, "CapA", 4))
				continue;
			sscanf(buf, "CapAmb:  %08x%08x",
			       &m.ambient[1], &m.ambient[0]);
			fclose(f);
			return 0;
		}
		fclose(f);
		// Didn't find ambient set, fall through and try prctl way
	}
	// Might be in a container with no procfs - do it the hard way
	memset(m.ambient, 0, sizeof(m.ambient));
	unsigned int i = 0;
	do {
		rc = prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_IS_SET, i, 0, 0);
		if (rc < 0)
			return -1;

		// Just add set bits
		if (rc)
			update_ambient_set(CAPNG_ADD, i%32, i>>5);
		i++;
	} while (cap_valid(i));

	return 0;
}
#endif

/*
 * Returns 0 on success and -1 on failure
 */
int capng_get_caps_process(void)
{
	int rc;

	if (m.state == CAPNG_NEW)
		init();
	if (m.state == CAPNG_ERROR)
		return -1;

	rc = capget((cap_user_header_t)&m.hdr, (cap_user_data_t)&m.data);
	if (rc == 0) {
		m.state = CAPNG_INIT;
#ifdef PR_CAPBSET_DROP
if (HAVE_PR_CAPBSET_DROP) {
		rc = get_bounding_set();
		if (rc < 0)
			m.state = CAPNG_ERROR;
}
#endif
#ifdef PR_CAP_AMBIENT
if (HAVE_PR_CAP_AMBIENT) {
		rc = get_ambient_set();
		if (rc < 0)
			m.state = CAPNG_ERROR;
}
#endif
	}

	return rc;
}

#ifdef VFS_CAP_U32
#ifdef VFS_CAP_REVISION_3
static int load_data(const struct vfs_ns_cap_data *filedata, int size)
#else
static int load_data(const struct vfs_cap_data *filedata, int size)
#endif
{
	unsigned int magic;

	if (m.cap_ver == 1)
		return -1;	// Should never get here but just in case

	magic = FIXUP(filedata->magic_etc);
	switch (magic & VFS_CAP_REVISION_MASK)
	{
		case VFS_CAP_REVISION_1:
			m.vfs_cap_ver = 1;
			if (size != XATTR_CAPS_SZ_1)
				return -1;
			break;
		case VFS_CAP_REVISION_2:
			m.vfs_cap_ver = 2;
			if (size != XATTR_CAPS_SZ_2)
				return -1;
			break;
#ifdef VFS_CAP_REVISION_3
		case VFS_CAP_REVISION_3:
			m.vfs_cap_ver = 3;
			if (size != XATTR_CAPS_SZ_3)
				return -1;
			break;
#endif
		default:
			return -1;
	}

	// Now stuff the data structures
	m.data.v3[0].permitted = FIXUP(filedata->data[0].permitted);
	m.data.v3[1].permitted = FIXUP(filedata->data[1].permitted);
	m.data.v3[0].inheritable = FIXUP(filedata->data[0].inheritable);
	m.data.v3[1].inheritable = FIXUP(filedata->data[1].inheritable);
	if (magic & VFS_CAP_FLAGS_EFFECTIVE) {
		m.data.v3[0].effective =
			m.data.v3[0].permitted | m.data.v3[0].inheritable;
		m.data.v3[1].effective =
			m.data.v3[1].permitted | m.data.v3[1].inheritable;
	} else {
		m.data.v3[0].effective = 0;
		m.data.v3[1].effective = 0;
	}
#ifdef VFS_CAP_REVISION_3
	if (size == XATTR_CAPS_SZ_3) {
	    struct vfs_ns_cap_data *d = (struct vfs_ns_cap_data *)filedata;
	    m.rootid = FIXUP(d->rootid);
	}
#endif
	return 0;
}
#endif

int capng_get_caps_fd(int fd)
{
#ifndef VFS_CAP_U32
	return -1;
#else
	int rc;
#ifdef VFS_CAP_REVISION_3
	struct vfs_ns_cap_data filedata;
#else
	struct vfs_cap_data filedata;
#endif
	if (m.state == CAPNG_NEW)
		init();
	if (m.state == CAPNG_ERROR)
		return -1;

	rc = fgetxattr(fd, XATTR_NAME_CAPS, &filedata, sizeof(filedata));
	if (rc <= 0)
		return -1;

	rc = load_data(&filedata, rc);
	if (rc == 0)
		m.state = CAPNG_INIT;
	else
		m.state = CAPNG_ERROR; // If load data failed, malformed data

	return rc;
#endif
}

static void v1_update(capng_act_t action, unsigned int capability, __u32 *data)
{
	if (action == CAPNG_ADD)
		*data |= MASK(capability);
	else
		*data &= ~(MASK(capability));
}

static void update_effective(capng_act_t action, unsigned int capability,
	unsigned int idx)
{
	if (action == CAPNG_ADD)
		m.data.v3[idx].effective |= MASK(capability);
	else
		m.data.v3[idx].effective &= ~(MASK(capability));
}

static void update_permitted(capng_act_t action, unsigned int capability,
	unsigned int idx)
{
	if (action == CAPNG_ADD)
		m.data.v3[idx].permitted |= MASK(capability);
	else
		m.data.v3[idx].permitted &= ~(MASK(capability));
}

static void update_inheritable(capng_act_t action, unsigned int capability,
	unsigned int idx)
{
	if (action == CAPNG_ADD)
		m.data.v3[idx].inheritable |= MASK(capability);
	else
		m.data.v3[idx].inheritable &= ~(MASK(capability));
}

static void update_bounding_set(capng_act_t action, unsigned int capability,
	unsigned int idx)
{
#ifdef PR_CAPBSET_DROP
if (HAVE_PR_CAPBSET_DROP) {
	if (action == CAPNG_ADD)
		m.bounds[idx] |= MASK(capability);
	else
		m.bounds[idx] &= ~(MASK(capability));
}
#endif
}

static void update_ambient_set(capng_act_t action, unsigned int capability,
	unsigned int idx)
{
#ifdef PR_CAP_AMBIENT
if (HAVE_PR_CAP_AMBIENT) {
	if (action == CAPNG_ADD)
		m.ambient[idx] |= MASK(capability);
	else
		m.ambient[idx] &= ~(MASK(capability));
}
#endif
}

int capng_update(capng_act_t action, capng_type_t type, unsigned int capability)
{
	// Before updating, we expect that the data is initialized to something
	if (m.state < CAPNG_INIT)
		return -1;
	if (!cap_valid(capability)) {
		errno = EINVAL;
		return -1;
	}

	if (m.cap_ver == 1) {
		if (CAPNG_EFFECTIVE & type)
			v1_update(action, capability, &m.data.v1.effective);
		if (CAPNG_PERMITTED & type)
			v1_update(action, capability, &m.data.v1.permitted);
		if (CAPNG_INHERITABLE & type)
			v1_update(action, capability, &m.data.v1.inheritable);
	} else {
		unsigned int idx;

		if (capability > 31) {
			idx = capability>>5;
			capability %= 32;
		} else
			idx = 0;

		if (CAPNG_EFFECTIVE & type)
			update_effective(action, capability, idx);
		if (CAPNG_PERMITTED & type)
			update_permitted(action, capability, idx);
		if (CAPNG_INHERITABLE & type)
			update_inheritable(action, capability, idx);
		if (CAPNG_BOUNDING_SET & type)
			update_bounding_set(action, capability, idx);
		if (CAPNG_AMBIENT & type)
			update_ambient_set(action, capability, idx);
	}

	m.state = CAPNG_UPDATED;
	return 0;
}

int capng_updatev(capng_act_t action, capng_type_t type,
                unsigned int capability, ...)
{
	int rc;
	unsigned int cap;
	va_list ap;

	rc = capng_update(action, type, capability);
	if (rc)
		return rc;
	va_start(ap, capability);
	cap = va_arg(ap, unsigned int);
	while (cap_valid(cap)) {
		rc = capng_update(action, type, cap);
		if (rc)
			break;
		cap = va_arg(ap, unsigned int);
	}
	va_end(ap);

	// See if planned exit or invalid
	if (cap == (unsigned)-1)
		rc = 0;
	else {
		rc = -1;
		errno = EINVAL;
	}

	return rc;
}

int capng_apply(capng_select_t set)
{
	int rc = 0;

	// Before updating, we expect that the data is initialized to something
	if (m.state < CAPNG_INIT)
		return -1;

	if (set & CAPNG_SELECT_BOUNDS) {
#ifdef PR_CAPBSET_DROP
if (HAVE_PR_CAPBSET_DROP) {
		struct cap_ng state;
		memcpy(&state, &m, sizeof(state)); /* save state */
		if (capng_get_caps_process())
			return -9;
		if (capng_have_capability(CAPNG_EFFECTIVE, CAP_SETPCAP)) {
			unsigned int i;
			memcpy(&m, &state, sizeof(m)); /* restore state */
			for (i=0; i <= last_cap; i++) {
				if (capng_have_capability(CAPNG_BOUNDING_SET,
								 i) == 0) {
				    if (prctl(PR_CAPBSET_DROP, i, 0, 0, 0) <0) {
					rc = -2;
					goto try_caps;
				    }
				}
			}
			m.state = CAPNG_APPLIED;
			if (get_bounding_set() < 0) {
				rc = -3;
				goto try_caps;
			}
		} else {
			memcpy(&m, &state, sizeof(m)); /* restore state */
			rc = -4;
			goto try_caps;
		}
}
#endif
	}

	// Try caps is here so that if someone had SELECT_BOTH and we blew up
	// doing the bounding set, we at least try to set any capabilities
	// before returning in case the caller also doesn't bother checking
	// the return code.
try_caps:
	if (set & CAPNG_SELECT_CAPS) {
		if (capset((cap_user_header_t)&m.hdr,
				(cap_user_data_t)&m.data) == 0)
			m.state = CAPNG_APPLIED;
		else
			rc = -5;
	}

	// Most programs do not and should not mess with ambient capabilities.
	// Instead of returning here if rc is set, we'll let it try to
	// do something with ambient capabilities in hopes that it's lowering
	// capabilities. Again, this is for people that don't check their
	// return codes.
	//
	// Do ambient last so that inheritable and permitted are set by the
	// time we get here.
	if (set & CAPNG_SELECT_AMBIENT) {
#ifdef PR_CAP_AMBIENT
if (HAVE_PR_CAP_AMBIENT) {
		if (capng_have_capabilities(CAPNG_SELECT_AMBIENT) ==
								CAPNG_NONE) {
			if (prctl(PR_CAP_AMBIENT,
				   PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0) < 0) {
				rc = -6;
				goto out;
			}
		} else {
			unsigned int i;

			// Clear them all
			if (prctl(PR_CAP_AMBIENT,
				   PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0) < 0) {
				rc = -7;
				goto out;
			}
			for (i=0; i <= last_cap; i++) {
				if (capng_have_capability(CAPNG_AMBIENT, i))
					if (prctl(PR_CAP_AMBIENT,
					    PR_CAP_AMBIENT_RAISE, i, 0, 0) < 0){
						rc = -8;
						goto out;
					}
			}
		}
		m.state = CAPNG_APPLIED;
}
#endif
	}
out:
	return rc;
}

#ifdef VFS_CAP_U32
#ifdef VFS_CAP_REVISION_3
static int save_data(struct vfs_ns_cap_data *filedata, int *size)
#else
static int save_data(struct vfs_cap_data *filedata, int *size)
#endif
{
	// Now stuff the data structures
	if (m.vfs_cap_ver == 1) {
		filedata->data[0].permitted = FIXUP(m.data.v1.permitted);
		filedata->data[0].inheritable = FIXUP(m.data.v1.inheritable);
		filedata->magic_etc = FIXUP(VFS_CAP_REVISION_1);
		*size = XATTR_CAPS_SZ_1;
	} else if (m.vfs_cap_ver == 2 || m.vfs_cap_ver == 3) {
		int eff;

		if (m.data.v3[0].effective || m.data.v3[1].effective)
			eff = VFS_CAP_FLAGS_EFFECTIVE;
		else
			eff = 0;
		filedata->data[0].permitted = FIXUP(m.data.v3[0].permitted);
		filedata->data[0].inheritable = FIXUP(m.data.v3[0].inheritable);
		filedata->data[1].permitted = FIXUP(m.data.v3[1].permitted);
		filedata->data[1].inheritable = FIXUP(m.data.v3[1].inheritable);
		filedata->magic_etc = FIXUP(VFS_CAP_REVISION_2 | eff);
		*size = XATTR_CAPS_SZ_2;
	}
#ifdef VFS_CAP_REVISION_3
	if (m.vfs_cap_ver == 3) {
		// Kernel doesn't support namespaces with non-0 rootid
		if (m.rootid!= 0)
			return -1;
		filedata->rootid = FIXUP(m.rootid);
		*size = XATTR_CAPS_SZ_3;
	}
#endif

	return 0;
}
#endif

int capng_apply_caps_fd(int fd)
{
#ifndef VFS_CAP_U32
	return -1;
#else
	int rc, size = 0;
#ifdef VFS_CAP_REVISION_3
	struct vfs_ns_cap_data filedata;
#else
	struct vfs_cap_data filedata;
#endif
	struct stat buf;

	// Before updating, we expect that the data is initialized to something
	if (m.state < CAPNG_INIT)
		return -1;

	if (fstat(fd, &buf) != 0)
		return -1;
	if (S_ISLNK(buf.st_mode) || !S_ISREG(buf.st_mode)) {
		errno = EINVAL;
		return -1;
	}
	if (capng_have_capabilities(CAPNG_SELECT_CAPS) == CAPNG_NONE)
		rc = fremovexattr(fd, XATTR_NAME_CAPS);
	else {
		if (save_data(&filedata, &size)) {
			m.state = CAPNG_ERROR;
			errno = EINVAL;
			return -2;
		}
		rc = fsetxattr(fd, XATTR_NAME_CAPS, &filedata, size, 0);
	}

	if (rc == 0)
		m.state = CAPNG_APPLIED;

	return rc;
#endif
}

// Change uids keeping/removing only certain capabilities
// flag to drop supp groups
int capng_change_id(int uid, int gid, capng_flags_t flag)
{
	int rc, ret, need_setgid, need_setuid;

	// Before updating, we expect that the data is initialized to something
	if (m.state < CAPNG_INIT)
		return -1;

	// Check the current capabilities
#ifdef PR_CAPBSET_DROP
if (HAVE_PR_CAPBSET_DROP) {
	// If newer kernel, we need setpcap to change the bounding set
	if (capng_have_capability(CAPNG_EFFECTIVE, CAP_SETPCAP) == 0 &&
					flag & CAPNG_CLEAR_BOUNDING)
		capng_update(CAPNG_ADD,
				CAPNG_EFFECTIVE|CAPNG_PERMITTED, CAP_SETPCAP);
}
#endif
	if (gid == -1 || capng_have_capability(CAPNG_EFFECTIVE, CAP_SETGID))
		need_setgid = 0;
	else {
		need_setgid = 1;
		capng_update(CAPNG_ADD, CAPNG_EFFECTIVE|CAPNG_PERMITTED,
				CAP_SETGID);
	}
	if (uid == -1 || capng_have_capability(CAPNG_EFFECTIVE, CAP_SETUID))
		need_setuid = 0;
	else {
		need_setuid = 1;
		capng_update(CAPNG_ADD, CAPNG_EFFECTIVE|CAPNG_PERMITTED,
				CAP_SETUID);
	}

	// Tell system we want to keep caps across uid change
	if (prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0))
		return -2;

	// Change to the temp capabilities
	rc = capng_apply(CAPNG_SELECT_CAPS);
	if (rc < 0) {
		ret = -3;
		goto err_out;
	}

	// If we are clearing ambient, only clear since its applied at the end
	if (flag & CAPNG_CLEAR_AMBIENT)
		capng_clear(CAPNG_SELECT_AMBIENT);

	// Clear bounding set if needed while we have CAP_SETPCAP
	if (flag & CAPNG_CLEAR_BOUNDING) {
		capng_clear(CAPNG_SELECT_BOUNDS);
		rc = capng_apply(CAPNG_SELECT_BOUNDS);
		if (rc) {
			ret = -8;
			goto err_out;
		}
	}

	// Change gid
	if (gid != -1) {
		rc = setresgid(gid, gid, gid);
		if (rc) {
			ret = -4;
			goto err_out;
		}
	}

	// See if we need to init supplemental groups
	if ((flag & CAPNG_INIT_SUPP_GRP) && uid != -1) {
		struct passwd *pw = getpwuid(uid);
		if (pw == NULL) {
			ret = -10;
			goto err_out;
		}
		if (gid != -1) {
			if (initgroups(pw->pw_name, gid)) {
				ret = -5;
				goto err_out;
			}
		} else if (initgroups(pw->pw_name, pw->pw_gid)) {
			ret = -5;
			goto err_out;
		}
	}

	// See if we need to unload supplemental groups
	if ((flag & CAPNG_DROP_SUPP_GRP) && gid != -1) {
		if (setgroups(0, NULL)) {
			ret = -5;
			goto err_out;
		}
	}

	// Change uid
	if (uid != -1) {
		rc = setresuid(uid, uid, uid);
		if (rc) {
			ret = -6;
			goto err_out;
		}
	}

	// Tell it we are done keeping capabilities
	rc = prctl(PR_SET_KEEPCAPS, 0, 0, 0, 0);
	if (rc)
		return -7;

	// Now throw away CAP_SETPCAP so no more changes
	if (need_setgid)
		capng_update(CAPNG_DROP, CAPNG_EFFECTIVE|CAPNG_PERMITTED,
				CAP_SETGID);
	if (need_setuid)
		capng_update(CAPNG_DROP, CAPNG_EFFECTIVE|CAPNG_PERMITTED,
				CAP_SETUID);

	// Now drop setpcap & apply
	capng_update(CAPNG_DROP, CAPNG_EFFECTIVE|CAPNG_PERMITTED,
				CAP_SETPCAP);
	rc = capng_apply(CAPNG_SELECT_CAPS|CAPNG_SELECT_AMBIENT);
	if (rc < 0)
		return -9;

	// Done
	m.state = CAPNG_UPDATED;
	return 0;

err_out:
	prctl(PR_SET_KEEPCAPS, 0, 0, 0, 0);
	return ret;
}

int capng_lock(void)
{
	// If either fail, return -1 since something is not right
#ifdef PR_SET_SECUREBITS
if (HAVE_PR_SET_SECUREBITS) {
	int rc = prctl(PR_SET_SECUREBITS,
			1 << SECURE_NOROOT |
			1 << SECURE_NOROOT_LOCKED |
			1 << SECURE_NO_SETUID_FIXUP |
			1 << SECURE_NO_SETUID_FIXUP_LOCKED, 0, 0, 0);
#ifdef PR_SET_NO_NEW_PRIVS
if (HAVE_PR_SET_NO_NEW_PRIVS) {
	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
		return -1;
}
#endif
	if (rc)
		return -1;
}
#endif

	return 0;
}

// -1 - error, 0 - no caps, 1 partial caps, 2 full caps
capng_results_t capng_have_capabilities(capng_select_t set)
{
	int empty = 0, full = 0;

	// First, try to init with current set
	if (m.state < CAPNG_INIT) {
		if (capng_get_caps_process())
			return CAPNG_FAIL;
	}

	// If we still don't have anything, error out
	if (m.state < CAPNG_INIT)
		return CAPNG_FAIL;

	if (set & CAPNG_SELECT_CAPS) {
		if (m.cap_ver == 1) {
			if (m.data.v1.effective == 0)
				empty = 1;
			// after fill, 30 bits starts from upper to lower
			else if (m.data.v1.effective == 0x7FFFFFFFU)
				full = 1;
			// actual capabilities read from system
			else if (m.data.v1.effective == 0xFFFFFEFFU)
				full = 1;
			else
				return CAPNG_PARTIAL;
		} else {
			if (m.data.v3[0].effective == 0)
				empty = 1;
			else if (m.data.v3[0].effective == 0xFFFFFFFFU)
				full = 1;
			else
				return CAPNG_PARTIAL;
			if ((m.data.v3[1].effective & UPPER_MASK) == 0 && !full)
				empty = 1;
			else if ((m.data.v3[1].effective & UPPER_MASK) ==
						UPPER_MASK && !empty)
				full = 1;
			else
				return CAPNG_PARTIAL;
		}
	}
#ifdef PR_CAPBSET_DROP
if (HAVE_PR_CAPBSET_DROP) {
	if (set & CAPNG_SELECT_BOUNDS) {
		if (m.bounds[0] == 0)
			empty = 1;
		else if (m.bounds[0] == 0xFFFFFFFFU)
			full = 1;
		else
			return CAPNG_PARTIAL;
		if ((m.bounds[1] & UPPER_MASK) == 0)
			empty = 1;
		else if ((m.bounds[1] & UPPER_MASK) == UPPER_MASK)
			full = 1;
		else
			return CAPNG_PARTIAL;
	}
} else
	empty = 1;
#endif
#ifdef PR_CAP_AMBIENT
if (HAVE_PR_CAP_AMBIENT) {
	if (set & CAPNG_SELECT_AMBIENT) {
		if (m.ambient[0] == 0)
			empty = 1;
		else if (m.ambient[0] == 0xFFFFFFFFU)
			full = 1;
		else
			return CAPNG_PARTIAL;
		if ((m.ambient[1] & UPPER_MASK) == 0)
			empty = 1;
		else if ((m.ambient[1] & UPPER_MASK) == UPPER_MASK)
			full = 1;
		else
			return CAPNG_PARTIAL;
	}
} else
	empty = 1;
#endif
	if (empty == 1 && full == 0)
		return CAPNG_NONE;
	else if (empty == 0 && full == 1)
		return CAPNG_FULL;

	return CAPNG_PARTIAL;
}

// -1 - error, 0 - no caps, 1 partial caps, 2 full caps
capng_results_t capng_have_permitted_capabilities(void)
{
	int empty = 0, full = 0;

	// First, try to init with current set
	if (m.state < CAPNG_INIT) {
		if (capng_get_caps_process())
			return CAPNG_FAIL;
	}

	// If we still don't have anything, error out
	if (m.state < CAPNG_INIT)
		return CAPNG_FAIL;

	if (m.data.v3[0].permitted == 0)
		empty = 1;
	else if (m.data.v3[0].permitted == 0xFFFFFFFFU)
		full = 1;
	else
		return CAPNG_PARTIAL;

	if ((m.data.v3[1].permitted & UPPER_MASK) == 0 && !full)
		empty = 1;
	else if ((m.data.v3[1].permitted & UPPER_MASK) == UPPER_MASK && !empty)
		full = 1;
	else
		return CAPNG_PARTIAL;

	if (empty == 1 && full == 0)
		return CAPNG_NONE;
	else if (empty == 0 && full == 1)
		return CAPNG_FULL;

	return CAPNG_PARTIAL;
}

static int check_effective(unsigned int capability, unsigned int idx)
{
	return MASK(capability) & m.data.v3[idx].effective ? 1 : 0;
}

static int check_permitted(unsigned int capability, unsigned int idx)
{
	return MASK(capability) & m.data.v3[idx].permitted ? 1 : 0;
}

static int check_inheritable(unsigned int capability, unsigned int idx)
{
	return MASK(capability) & m.data.v3[idx].inheritable ? 1 : 0;
}

static int bounds_bit_check(unsigned int capability, unsigned int idx)
{
#ifdef PR_CAPBSET_DROP
if (HAVE_PR_CAPBSET_DROP) {
	return MASK(capability) & m.bounds[idx] ? 1 : 0;
}
#endif
	return 0;
}

static int ambient_bit_check(unsigned int capability, unsigned int idx)
{
#ifdef PR_CAP_AMBIENT
if (HAVE_PR_CAP_AMBIENT) {
	return MASK(capability) & m.ambient[idx] ? 1 : 0;
}
#endif
	return 0;
}

static int v1_check(unsigned int capability, __u32 data)
{
	return MASK(capability) & data ? 1 : 0;
}

int capng_have_capability(capng_type_t which, unsigned int capability)
{
	// First, try to init with current set
	if (m.state < CAPNG_INIT) {
		if (capng_get_caps_process())
			return 0;
	}

	// If we still don't have anything, error out
	if (m.state < CAPNG_INIT)
		return 0;
	if (m.cap_ver == 1 && capability > 31)
		return 0;
	if (!cap_valid(capability))
		return 0;

	if (m.cap_ver == 1) {
		if (which == CAPNG_EFFECTIVE)
			return v1_check(capability, m.data.v1.effective);
		else if (which == CAPNG_PERMITTED)
			return v1_check(capability, m.data.v1.permitted);
		else if (which == CAPNG_INHERITABLE)
			return v1_check(capability, m.data.v1.inheritable);
	} else {
		unsigned int idx;

		if (capability > 31) {
			idx = capability>>5;
			capability %= 32;
		} else
			idx = 0;

		if (which == CAPNG_EFFECTIVE)
			return check_effective(capability, idx);
		else if (which == CAPNG_PERMITTED)
			return check_permitted(capability, idx);
		else if (which == CAPNG_INHERITABLE)
			return check_inheritable(capability, idx);
		else if (which == CAPNG_BOUNDING_SET)
			return bounds_bit_check(capability, idx);
		else if (which == CAPNG_AMBIENT)
			return ambient_bit_check(capability, idx);
	}
	return 0;
}

char *capng_print_caps_numeric(capng_print_t where, capng_select_t set)
{
	char *ptr = NULL;

	if (m.state < CAPNG_INIT)
		return ptr;

	if (where == CAPNG_PRINT_STDOUT) {
		if (set & CAPNG_SELECT_CAPS) {
			if (m.cap_ver == 1) {
				printf( "Effective:    %08X\n"
					"Permitted:    %08X\n"
					"Inheritable:  %08X\n",
					m.data.v1.effective,
					m.data.v1.permitted,
					m.data.v1.inheritable);
			} else {
				printf( "Effective:    %08X, %08X\n"
					"Permitted:    %08X, %08X\n"
					"Inheritable:  %08X, %08X\n",
					m.data.v3[1].effective & UPPER_MASK,
					m.data.v3[0].effective,
					m.data.v3[1].permitted & UPPER_MASK,
					m.data.v3[0].permitted,
					m.data.v3[1].inheritable & UPPER_MASK,
					m.data.v3[0].inheritable);
			}
		}
#ifdef PR_CAPBSET_DROP
if (HAVE_PR_CAPBSET_DROP) {
		if (set & CAPNG_SELECT_BOUNDS)
			printf("Bounding Set: %08X, %08X\n",
				m.bounds[1] & UPPER_MASK, m.bounds[0]);
}
#endif
#ifdef PR_CAP_AMBIENT
if (HAVE_PR_CAP_AMBIENT) {
		if (set & CAPNG_SELECT_AMBIENT)
			printf("Ambient :     %08X, %08X\n",
				m.ambient[1] & UPPER_MASK, m.ambient[0]);
}
#endif
	} else if (where == CAPNG_PRINT_BUFFER) {
		if (set & CAPNG_SELECT_CAPS) {
			// Make it big enough for bounding & ambient set, too
			ptr = malloc(160);
			if (m.cap_ver == 1) {
				snprintf(ptr, 160,
					"Effective:   %08X\n"
					"Permitted:   %08X\n"
					"Inheritable: %08X\n",
					m.data.v1.effective,
					m.data.v1.permitted,
					m.data.v1.inheritable);
			} else {
				snprintf(ptr, 160,
					"Effective:   %08X, %08X\n"
					"Permitted:   %08X, %08X\n"
					"Inheritable: %08X, %08X\n",
					m.data.v3[1].effective & UPPER_MASK,
					m.data.v3[0].effective,
					m.data.v3[1].permitted & UPPER_MASK,
					m.data.v3[0].permitted,
					m.data.v3[1].inheritable & UPPER_MASK,
					m.data.v3[0].inheritable);
			}
		}
		if (set & CAPNG_SELECT_BOUNDS) {
#ifdef PR_CAPBSET_DROP
if (HAVE_PR_CAPBSET_DROP) {
			char *s;
			// If ptr is NULL, we only room for bounding and ambient
			if (ptr == NULL ) {
				ptr = malloc(80);
				if (ptr == NULL)
					return ptr;
				*ptr = 0;
				s = ptr;
			} else
				s = ptr + strlen(ptr);
			snprintf(s, 40, "Bounding Set: %08X, %08X\n",
					m.bounds[1] & UPPER_MASK, m.bounds[0]);
}
#endif
		}
		if (set & CAPNG_SELECT_AMBIENT) {
#ifdef PR_CAP_AMBIENT
if (HAVE_PR_CAP_AMBIENT) {
			char *s;
			// If ptr is NULL, we only room for ambient
			if (ptr == NULL ) {
				ptr = malloc(40);
				if (ptr == NULL)
					return ptr;
				*ptr = 0;
				s = ptr;
			} else
				s = ptr + strlen(ptr);
			snprintf(s, 40, "Ambient Set: %08X, %08X\n",
					m.ambient[1] & UPPER_MASK,
					m.ambient[0]);
}
#endif
		}
	}

	return ptr;
}

char *capng_print_caps_text(capng_print_t where, capng_type_t which)
{
	unsigned int i, once = 0, cnt = 0;
	char *ptr = NULL;

	if (m.state < CAPNG_INIT)
		return ptr;

	for (i=0; i<=last_cap; i++) {
		if (capng_have_capability(which, i)) {
			const char *n = capng_capability_to_name(i);
			if (n == NULL)
				n = "unknown";
			if (where == CAPNG_PRINT_STDOUT) {
				if (once == 0) {
					printf("%s", n);
					once++;
				} else
					printf(", %s", n);
			} else if (where == CAPNG_PRINT_BUFFER) {
				int len;
				if (once == 0) {
					ptr = malloc(last_cap*20);
					if (ptr == NULL)
						return ptr;
					len = sprintf(ptr+cnt, "%s", n);
					once++;
				} else
					len = sprintf(ptr+cnt, ", %s", n);
				if (len > 0)
					cnt+=len;
			}
		}
	}
	if (once == 0) {
		if (where == CAPNG_PRINT_STDOUT)
			printf("none");
		else
			ptr = strdup("none");
	}
	return ptr;
}

void *capng_save_state(void)
{
	void *ptr = malloc(sizeof(m));
	if (ptr)
		memcpy(ptr, &m, sizeof(m));
	return ptr;
}

void capng_restore_state(void **state)
{
	if (state) {
		void *ptr = *state;
		if (ptr)
			memcpy(&m, ptr, sizeof(m));
		free(ptr);
		*state = NULL;
	}
}

