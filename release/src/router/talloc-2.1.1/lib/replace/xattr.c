/* 
   Unix SMB/CIFS implementation.
   replacement routines for xattr implementations
   Copyright (C) Jeremy Allison  1998-2005
   Copyright (C) Timur Bakeyev        2005
   Copyright (C) Bjoern Jacke    2006-2007
   Copyright (C) Herb Lewis           2003
   Copyright (C) Andrew Bartlett      2012

     ** NOTE! The following LGPL license applies to the replace
     ** library. This does NOT imply that all of Samba is released
     ** under the LGPL
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see <http://www.gnu.org/licenses/>.
*/

#define UID_WRAPPER_NOT_REPLACE
#include "replace.h"
#include "system/filesys.h"
#include "system/dir.h"

/******** Solaris EA helper function prototypes ********/
#ifdef HAVE_ATTROPEN
#define SOLARIS_ATTRMODE S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP
static int solaris_write_xattr(int attrfd, const char *value, size_t size);
static ssize_t solaris_read_xattr(int attrfd, void *value, size_t size);
static ssize_t solaris_list_xattr(int attrdirfd, char *list, size_t size);
static int solaris_unlinkat(int attrdirfd, const char *name);
static int solaris_attropen(const char *path, const char *attrpath, int oflag, mode_t mode);
static int solaris_openat(int fildes, const char *path, int oflag, mode_t mode);
#endif

/**************************************************************************
 Wrappers for extented attribute calls. Based on the Linux package with
 support for IRIX and (Net|Free)BSD also. Expand as other systems have them.
****************************************************************************/

ssize_t rep_getxattr (const char *path, const char *name, void *value, size_t size)
{
#if defined(HAVE_GETXATTR)
#ifndef XATTR_ADDITIONAL_OPTIONS
	return getxattr(path, name, value, size);
#else

/* So that we do not recursivly call this function */
#undef getxattr
	int options = 0;
	return getxattr(path, name, value, size, 0, options);
#endif
#elif defined(HAVE_GETEA)
	return getea(path, name, value, size);
#elif defined(HAVE_EXTATTR_GET_FILE)
	char *s;
	ssize_t retval;
	int attrnamespace = (strncmp(name, "system", 6) == 0) ? 
		EXTATTR_NAMESPACE_SYSTEM : EXTATTR_NAMESPACE_USER;
	const char *attrname = ((s=strchr(name, '.')) == NULL) ? name : s + 1;
	/*
	 * The BSD implementation has a nasty habit of silently truncating
	 * the returned value to the size of the buffer, so we have to check
	 * that the buffer is large enough to fit the returned value.
	 */
	if((retval=extattr_get_file(path, attrnamespace, attrname, NULL, 0)) >= 0) {
		if (size == 0) {
			return retval;
		} else if (retval > size) {
			errno = ERANGE;
			return -1;
		}
		if((retval=extattr_get_file(path, attrnamespace, attrname, value, size)) >= 0)
			return retval;
	}

	return -1;
#elif defined(HAVE_ATTR_GET)
	int retval, flags = 0;
	int valuelength = (int)size;
	char *attrname = strchr(name,'.') + 1;

	if (strncmp(name, "system", 6) == 0) flags |= ATTR_ROOT;

	retval = attr_get(path, attrname, (char *)value, &valuelength, flags);
	if (size == 0 && retval == -1 && errno == E2BIG) {
		return valuelength;
	}

	return retval ? retval : valuelength;
#elif defined(HAVE_ATTROPEN)
	ssize_t ret = -1;
	int attrfd = solaris_attropen(path, name, O_RDONLY, 0);
	if (attrfd >= 0) {
		ret = solaris_read_xattr(attrfd, value, size);
		close(attrfd);
	}
	return ret;
#else
	errno = ENOSYS;
	return -1;
#endif
}

ssize_t rep_fgetxattr (int filedes, const char *name, void *value, size_t size)
{
#if defined(HAVE_FGETXATTR)
#ifndef XATTR_ADDITIONAL_OPTIONS
	return fgetxattr(filedes, name, value, size);
#else

/* So that we do not recursivly call this function */
#undef fgetxattr
	int options = 0;
	return fgetxattr(filedes, name, value, size, 0, options);
#endif
#elif defined(HAVE_FGETEA)
	return fgetea(filedes, name, value, size);
#elif defined(HAVE_EXTATTR_GET_FD)
	char *s;
	ssize_t retval;
	int attrnamespace = (strncmp(name, "system", 6) == 0) ? 
		EXTATTR_NAMESPACE_SYSTEM : EXTATTR_NAMESPACE_USER;
	const char *attrname = ((s=strchr(name, '.')) == NULL) ? name : s + 1;

	if((retval=extattr_get_fd(filedes, attrnamespace, attrname, NULL, 0)) >= 0) {
		if (size == 0) {
			return retval;
		} else if (retval > size) {
			errno = ERANGE;
			return -1;
		}
		if((retval=extattr_get_fd(filedes, attrnamespace, attrname, value, size)) >= 0)
			return retval;
	}

	return -1;
#elif defined(HAVE_ATTR_GETF)
	int retval, flags = 0;
	int valuelength = (int)size;
	char *attrname = strchr(name,'.') + 1;

	if (strncmp(name, "system", 6) == 0) flags |= ATTR_ROOT;

	retval = attr_getf(filedes, attrname, (char *)value, &valuelength, flags);
	if (size == 0 && retval == -1 && errno == E2BIG) {
		return valuelength;
	}
	return retval ? retval : valuelength;
#elif defined(HAVE_ATTROPEN)
	ssize_t ret = -1;
	int attrfd = solaris_openat(filedes, name, O_RDONLY|O_XATTR, 0);
	if (attrfd >= 0) {
		ret = solaris_read_xattr(attrfd, value, size);
		close(attrfd);
	}
	return ret;
#else
	errno = ENOSYS;
	return -1;
#endif
}

#if defined(HAVE_EXTATTR_LIST_FILE)

#define EXTATTR_PREFIX(s)	(s), (sizeof((s))-1)

static struct {
        int space;
	const char *name;
	size_t len;
} 
extattr[] = {
	{ EXTATTR_NAMESPACE_SYSTEM, EXTATTR_PREFIX("system.") },
        { EXTATTR_NAMESPACE_USER, EXTATTR_PREFIX("user.") },
};

typedef union {
	const char *path;
	int filedes;
} extattr_arg;

static ssize_t bsd_attr_list (int type, extattr_arg arg, char *list, size_t size)
{
	ssize_t list_size, total_size = 0;
	int i, t, len;
	char *buf;
	/* Iterate through extattr(2) namespaces */
	for(t = 0; t < ARRAY_SIZE(extattr); t++) {
		if (t != EXTATTR_NAMESPACE_USER && geteuid() != 0) {
			/* ignore all but user namespace when we are not root, see bug 10247 */
			continue;
		}
		switch(type) {
#if defined(HAVE_EXTATTR_LIST_FILE)
			case 0:
				list_size = extattr_list_file(arg.path, extattr[t].space, list, size);
				break;
#endif
#if defined(HAVE_EXTATTR_LIST_LINK)
			case 1:
				list_size = extattr_list_link(arg.path, extattr[t].space, list, size);
				break;
#endif
#if defined(HAVE_EXTATTR_LIST_FD)
			case 2:
				list_size = extattr_list_fd(arg.filedes, extattr[t].space, list, size);
				break;
#endif
			default:
				errno = ENOSYS;
				return -1;
		}
		/* Some error happend. Errno should be set by the previous call */
		if(list_size < 0)
			return -1;
		/* No attributes */
		if(list_size == 0)
			continue;
		/* XXX: Call with an empty buffer may be used to calculate
		   necessary buffer size. Unfortunately, we can't say, how
		   many attributes were returned, so here is the potential
		   problem with the emulation.
		*/
		if(list == NULL) {
			/* Take the worse case of one char attribute names - 
			   two bytes per name plus one more for sanity.
			*/
			total_size += list_size + (list_size/2 + 1)*extattr[t].len;
			continue;
		}
		/* Count necessary offset to fit namespace prefixes */
		len = 0;
		for(i = 0; i < list_size; i += list[i] + 1)
			len += extattr[t].len;

		total_size += list_size + len;
		/* Buffer is too small to fit the results */
		if(total_size > size) {
			errno = ERANGE;
			return -1;
		}
		/* Shift results back, so we can prepend prefixes */
		buf = (char *)memmove(list + len, list, list_size);

		for(i = 0; i < list_size; i += len + 1) {
			len = buf[i];
			strncpy(list, extattr[t].name, extattr[t].len + 1);
			list += extattr[t].len;
			strncpy(list, buf + i + 1, len);
			list[len] = '\0';
			list += len + 1;
		}
		size -= total_size;
	}
	return total_size;
}

#endif

#if defined(HAVE_ATTR_LIST) && (defined(HAVE_SYS_ATTRIBUTES_H) || defined(HAVE_ATTR_ATTRIBUTES_H))
static char attr_buffer[ATTR_MAX_VALUELEN];

static ssize_t irix_attr_list(const char *path, int filedes, char *list, size_t size, int flags)
{
	int retval = 0, index;
	attrlist_cursor_t *cursor = 0;
	int total_size = 0;
	attrlist_t * al = (attrlist_t *)attr_buffer;
	attrlist_ent_t *ae;
	size_t ent_size, left = size;
	char *bp = list;

	while (true) {
	    if (filedes)
		retval = attr_listf(filedes, attr_buffer, ATTR_MAX_VALUELEN, flags, cursor);
	    else
		retval = attr_list(path, attr_buffer, ATTR_MAX_VALUELEN, flags, cursor);
	    if (retval) break;
	    for (index = 0; index < al->al_count; index++) {
		ae = ATTR_ENTRY(attr_buffer, index);
		ent_size = strlen(ae->a_name) + sizeof("user.");
		if (left >= ent_size) {
		    strncpy(bp, "user.", sizeof("user."));
		    strncat(bp, ae->a_name, ent_size - sizeof("user."));
		    bp += ent_size;
		    left -= ent_size;
		} else if (size) {
		    errno = ERANGE;
		    retval = -1;
		    break;
		}
		total_size += ent_size;
	    }
	    if (al->al_more == 0) break;
	}
	if (retval == 0) {
	    flags |= ATTR_ROOT;
	    cursor = 0;
	    while (true) {
		if (filedes)
		    retval = attr_listf(filedes, attr_buffer, ATTR_MAX_VALUELEN, flags, cursor);
		else
		    retval = attr_list(path, attr_buffer, ATTR_MAX_VALUELEN, flags, cursor);
		if (retval) break;
		for (index = 0; index < al->al_count; index++) {
		    ae = ATTR_ENTRY(attr_buffer, index);
		    ent_size = strlen(ae->a_name) + sizeof("system.");
		    if (left >= ent_size) {
			strncpy(bp, "system.", sizeof("system."));
			strncat(bp, ae->a_name, ent_size - sizeof("system."));
			bp += ent_size;
			left -= ent_size;
		    } else if (size) {
			errno = ERANGE;
			retval = -1;
			break;
		    }
		    total_size += ent_size;
		}
		if (al->al_more == 0) break;
	    }
	}
	return (ssize_t)(retval ? retval : total_size);
}

#endif

ssize_t rep_listxattr (const char *path, char *list, size_t size)
{
#if defined(HAVE_LISTXATTR)
#ifndef XATTR_ADDITIONAL_OPTIONS
	return listxattr(path, list, size);
#else
/* So that we do not recursivly call this function */
#undef listxattr
	int options = 0;
	return listxattr(path, list, size, options);
#endif
#elif defined(HAVE_LISTEA)
	return listea(path, list, size);
#elif defined(HAVE_EXTATTR_LIST_FILE)
	extattr_arg arg;
	arg.path = path;
	return bsd_attr_list(0, arg, list, size);
#elif defined(HAVE_ATTR_LIST) && defined(HAVE_SYS_ATTRIBUTES_H)
	return irix_attr_list(path, 0, list, size, 0);
#elif defined(HAVE_ATTROPEN)
	ssize_t ret = -1;
	int attrdirfd = solaris_attropen(path, ".", O_RDONLY, 0);
	if (attrdirfd >= 0) {
		ret = solaris_list_xattr(attrdirfd, list, size);
		close(attrdirfd);
	}
	return ret;
#else
	errno = ENOSYS;
	return -1;
#endif
}

ssize_t rep_flistxattr (int filedes, char *list, size_t size)
{
#if defined(HAVE_FLISTXATTR)
#ifndef XATTR_ADDITIONAL_OPTIONS
	return flistxattr(filedes, list, size);
#else
/* So that we do not recursivly call this function */
#undef flistxattr
	int options = 0;
	return flistxattr(filedes, list, size, options);
#endif
#elif defined(HAVE_FLISTEA)
	return flistea(filedes, list, size);
#elif defined(HAVE_EXTATTR_LIST_FD)
	extattr_arg arg;
	arg.filedes = filedes;
	return bsd_attr_list(2, arg, list, size);
#elif defined(HAVE_ATTR_LISTF)
	return irix_attr_list(NULL, filedes, list, size, 0);
#elif defined(HAVE_ATTROPEN)
	ssize_t ret = -1;
	int attrdirfd = solaris_openat(filedes, ".", O_RDONLY|O_XATTR, 0);
	if (attrdirfd >= 0) {
		ret = solaris_list_xattr(attrdirfd, list, size);
		close(attrdirfd);
	}
	return ret;
#else
	errno = ENOSYS;
	return -1;
#endif
}

int rep_removexattr (const char *path, const char *name)
{
#if defined(HAVE_REMOVEXATTR)
#ifndef XATTR_ADDITIONAL_OPTIONS
	return removexattr(path, name);
#else
/* So that we do not recursivly call this function */
#undef removexattr
	int options = 0;
	return removexattr(path, name, options);
#endif
#elif defined(HAVE_REMOVEEA)
	return removeea(path, name);
#elif defined(HAVE_EXTATTR_DELETE_FILE)
	char *s;
	int attrnamespace = (strncmp(name, "system", 6) == 0) ? 
		EXTATTR_NAMESPACE_SYSTEM : EXTATTR_NAMESPACE_USER;
	const char *attrname = ((s=strchr(name, '.')) == NULL) ? name : s + 1;

	return extattr_delete_file(path, attrnamespace, attrname);
#elif defined(HAVE_ATTR_REMOVE)
	int flags = 0;
	char *attrname = strchr(name,'.') + 1;

	if (strncmp(name, "system", 6) == 0) flags |= ATTR_ROOT;

	return attr_remove(path, attrname, flags);
#elif defined(HAVE_ATTROPEN)
	int ret = -1;
	int attrdirfd = solaris_attropen(path, ".", O_RDONLY, 0);
	if (attrdirfd >= 0) {
		ret = solaris_unlinkat(attrdirfd, name);
		close(attrdirfd);
	}
	return ret;
#else
	errno = ENOSYS;
	return -1;
#endif
}

int rep_fremovexattr (int filedes, const char *name)
{
#if defined(HAVE_FREMOVEXATTR)
#ifndef XATTR_ADDITIONAL_OPTIONS
	return fremovexattr(filedes, name);
#else
/* So that we do not recursivly call this function */
#undef fremovexattr
	int options = 0;
	return fremovexattr(filedes, name, options);
#endif
#elif defined(HAVE_FREMOVEEA)
	return fremoveea(filedes, name);
#elif defined(HAVE_EXTATTR_DELETE_FD)
	char *s;
	int attrnamespace = (strncmp(name, "system", 6) == 0) ? 
		EXTATTR_NAMESPACE_SYSTEM : EXTATTR_NAMESPACE_USER;
	const char *attrname = ((s=strchr(name, '.')) == NULL) ? name : s + 1;

	return extattr_delete_fd(filedes, attrnamespace, attrname);
#elif defined(HAVE_ATTR_REMOVEF)
	int flags = 0;
	char *attrname = strchr(name,'.') + 1;

	if (strncmp(name, "system", 6) == 0) flags |= ATTR_ROOT;

	return attr_removef(filedes, attrname, flags);
#elif defined(HAVE_ATTROPEN)
	int ret = -1;
	int attrdirfd = solaris_openat(filedes, ".", O_RDONLY|O_XATTR, 0);
	if (attrdirfd >= 0) {
		ret = solaris_unlinkat(attrdirfd, name);
		close(attrdirfd);
	}
	return ret;
#else
	errno = ENOSYS;
	return -1;
#endif
}

int rep_setxattr (const char *path, const char *name, const void *value, size_t size, int flags)
{
#if defined(HAVE_SETXATTR)
#ifndef XATTR_ADDITIONAL_OPTIONS
	return setxattr(path, name, value, size, flags);
#else
/* So that we do not recursivly call this function */
#undef setxattr
	int options = 0;
	return setxattr(path, name, value, size, 0, options);
#endif
#elif defined(HAVE_SETEA)
	return setea(path, name, value, size, flags);
#elif defined(HAVE_EXTATTR_SET_FILE)
	char *s;
	int retval = 0;
	int attrnamespace = (strncmp(name, "system", 6) == 0) ? 
		EXTATTR_NAMESPACE_SYSTEM : EXTATTR_NAMESPACE_USER;
	const char *attrname = ((s=strchr(name, '.')) == NULL) ? name : s + 1;
	if (flags) {
		/* Check attribute existence */
		retval = extattr_get_file(path, attrnamespace, attrname, NULL, 0);
		if (retval < 0) {
			/* REPLACE attribute, that doesn't exist */
			if (flags & XATTR_REPLACE && errno == ENOATTR) {
				errno = ENOATTR;
				return -1;
			}
			/* Ignore other errors */
		}
		else {
			/* CREATE attribute, that already exists */
			if (flags & XATTR_CREATE) {
				errno = EEXIST;
				return -1;
			}
		}
	}
	retval = extattr_set_file(path, attrnamespace, attrname, value, size);
	return (retval < 0) ? -1 : 0;
#elif defined(HAVE_ATTR_SET)
	int myflags = 0;
	char *attrname = strchr(name,'.') + 1;

	if (strncmp(name, "system", 6) == 0) myflags |= ATTR_ROOT;
	if (flags & XATTR_CREATE) myflags |= ATTR_CREATE;
	if (flags & XATTR_REPLACE) myflags |= ATTR_REPLACE;

	return attr_set(path, attrname, (const char *)value, size, myflags);
#elif defined(HAVE_ATTROPEN)
	int ret = -1;
	int myflags = O_RDWR;
	int attrfd;
	if (flags & XATTR_CREATE) myflags |= O_EXCL;
	if (!(flags & XATTR_REPLACE)) myflags |= O_CREAT;
	attrfd = solaris_attropen(path, name, myflags, (mode_t) SOLARIS_ATTRMODE);
	if (attrfd >= 0) {
		ret = solaris_write_xattr(attrfd, value, size);
		close(attrfd);
	}
	return ret;
#else
	errno = ENOSYS;
	return -1;
#endif
}

int rep_fsetxattr (int filedes, const char *name, const void *value, size_t size, int flags)
{
#if defined(HAVE_FSETXATTR)
#ifndef XATTR_ADDITIONAL_OPTIONS
	return fsetxattr(filedes, name, value, size, flags);
#else
/* So that we do not recursivly call this function */
#undef fsetxattr
	int options = 0;
	return fsetxattr(filedes, name, value, size, 0, options);
#endif
#elif defined(HAVE_FSETEA)
	return fsetea(filedes, name, value, size, flags);
#elif defined(HAVE_EXTATTR_SET_FD)
	char *s;
	int retval = 0;
	int attrnamespace = (strncmp(name, "system", 6) == 0) ? 
		EXTATTR_NAMESPACE_SYSTEM : EXTATTR_NAMESPACE_USER;
	const char *attrname = ((s=strchr(name, '.')) == NULL) ? name : s + 1;
	if (flags) {
		/* Check attribute existence */
		retval = extattr_get_fd(filedes, attrnamespace, attrname, NULL, 0);
		if (retval < 0) {
			/* REPLACE attribute, that doesn't exist */
			if (flags & XATTR_REPLACE && errno == ENOATTR) {
				errno = ENOATTR;
				return -1;
			}
			/* Ignore other errors */
		}
		else {
			/* CREATE attribute, that already exists */
			if (flags & XATTR_CREATE) {
				errno = EEXIST;
				return -1;
			}
		}
	}
	retval = extattr_set_fd(filedes, attrnamespace, attrname, value, size);
	return (retval < 0) ? -1 : 0;
#elif defined(HAVE_ATTR_SETF)
	int myflags = 0;
	char *attrname = strchr(name,'.') + 1;

	if (strncmp(name, "system", 6) == 0) myflags |= ATTR_ROOT;
	if (flags & XATTR_CREATE) myflags |= ATTR_CREATE;
	if (flags & XATTR_REPLACE) myflags |= ATTR_REPLACE;

	return attr_setf(filedes, attrname, (const char *)value, size, myflags);
#elif defined(HAVE_ATTROPEN)
	int ret = -1;
	int myflags = O_RDWR | O_XATTR;
	int attrfd;
	if (flags & XATTR_CREATE) myflags |= O_EXCL;
	if (!(flags & XATTR_REPLACE)) myflags |= O_CREAT;
	attrfd = solaris_openat(filedes, name, myflags, (mode_t) SOLARIS_ATTRMODE);
	if (attrfd >= 0) {
		ret = solaris_write_xattr(attrfd, value, size);
		close(attrfd);
	}
	return ret;
#else
	errno = ENOSYS;
	return -1;
#endif
}

/**************************************************************************
 helper functions for Solaris' EA support
****************************************************************************/
#ifdef HAVE_ATTROPEN
static ssize_t solaris_read_xattr(int attrfd, void *value, size_t size)
{
	struct stat sbuf;

	if (fstat(attrfd, &sbuf) == -1) {
		errno = ENOATTR;
		return -1;
	}

	/* This is to return the current size of the named extended attribute */
	if (size == 0) {
		return sbuf.st_size;
	}

	/* check size and read xattr */
	if (sbuf.st_size > size) {
		errno = ERANGE;
		return -1;
	}

	return read(attrfd, value, sbuf.st_size);
}

static ssize_t solaris_list_xattr(int attrdirfd, char *list, size_t size)
{
	ssize_t len = 0;
	DIR *dirp;
	struct dirent *de;
	int newfd = dup(attrdirfd);
	/* CAUTION: The originating file descriptor should not be
	            used again following the call to fdopendir().
	            For that reason we dup() the file descriptor
		    here to make things more clear. */
	dirp = fdopendir(newfd);

	while ((de = readdir(dirp))) {
		size_t listlen = strlen(de->d_name) + 1;
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
			/* we don't want "." and ".." here: */
			continue;
		}

		if (size == 0) {
			/* return the current size of the list of extended attribute names*/
			len += listlen;
		} else {
			/* check size and copy entrieѕ + nul into list. */
			if ((len + listlen) > size) {
				errno = ERANGE;
				len = -1;
				break;
			} else {
				strlcpy(list + len, de->d_name, listlen);
				len += listlen;
			}
		}
	}

	if (closedir(dirp) == -1) {
		return -1;
	}
	return len;
}

static int solaris_unlinkat(int attrdirfd, const char *name)
{
	if (unlinkat(attrdirfd, name, 0) == -1) {
		if (errno == ENOENT) {
			errno = ENOATTR;
		}
		return -1;
	}
	return 0;
}

static int solaris_attropen(const char *path, const char *attrpath, int oflag, mode_t mode)
{
	int filedes = attropen(path, attrpath, oflag, mode);
	if (filedes == -1) {
		if (errno == EINVAL) {
			errno = ENOTSUP;
		} else {
			errno = ENOATTR;
		}
	}
	return filedes;
}

static int solaris_openat(int fildes, const char *path, int oflag, mode_t mode)
{
	int filedes = openat(fildes, path, oflag, mode);
	if (filedes == -1) {
		if (errno == EINVAL) {
			errno = ENOTSUP;
		} else {
			errno = ENOATTR;
		}
	}
	return filedes;
}

static int solaris_write_xattr(int attrfd, const char *value, size_t size)
{
	if ((ftruncate(attrfd, 0) == 0) && (write(attrfd, value, size) == size)) {
		return 0;
	} else {
		return -1;
	}
}
#endif /*HAVE_ATTROPEN*/


