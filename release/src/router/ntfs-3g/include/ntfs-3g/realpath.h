/*
 * realpath.h - realpath() aware of device mapper
 */

#ifndef REALPATH_H
#define REALPATH_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_REALPATH
#define ntfs_realpath realpath
#else
extern char *ntfs_realpath(const char *path, char *resolved_path);
#endif

#ifdef linux
extern char *ntfs_realpath_canonicalize(const char *path, char *resolved_path);
#else
#define ntfs_realpath_canonicalize ntfs_realpath
#endif

#endif /* REALPATH_H */
