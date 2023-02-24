/*
 *		 Display and audit security attributes in an NTFS volume
 *
 * Copyright (c) 2007-2016 Jean-Pierre Andre
 *
 *	Options :
 *		-a auditing security data
 *		-b backing up NTFS ACLs
 *		-e set extra backed-up parameters (in conjunction with -s)
 *		-h displaying hexadecimal security descriptors within a file
 *		-r recursing in a directory
 *		-s setting backed-up NTFS ACLs
 *		-u getting a user mapping proposal
 *		-v verbose (very verbose if set twice)
 *	   also, if compile-time option is set
 *		-t run internal tests (with no access to storage)
 *
 *	On Linux (being root, with volume not mounted) :
 *		ntfssecaudit -h [file]
 *			display the security descriptors found in file
 *		ntfssecaudit -a[rv] volume
 *			audit the volume
 *		ntfssecaudit [-v] volume file
 *			display the security parameters of file
 *		ntfssecaudit -r[v] volume directory
 *			display the security parameters of files in directory
 *		ntfssecaudit -b[v] volume [directory]
 *			backup the security parameters of files in directory
 *		ntfssecaudit -s[ve] volume [backupfile]
 *			set the security parameters as indicated in backup
 *			with -e set extra parameters (Windows attrib)
 *		ntfssecaudit volume perms file
 *			set the security parameters of file to perms (mode or acl)
 *		ntfssecaudit -r[v] volume perms directory
 *			set the security parameters of files in directory to perms
 *          special cases, do not require being root :
 *		ntfssecaudit [-v] mounted-file
 *			display the security parameters of mounted file
 *		ntfssecaudit -u[v] mounted-file
 *			display a user mapping proposal
 *
 *
 *	On Windows (the volume being part of file name)
 *		ntfssecaudit -h [file]
 *			display the security descriptors found in file
 *		ntfssecaudit -a[rv] volume
 *			audit the volume
 *		ntfssecaudit [-v] file
 *			display the security parameters of file
 *		ntfssecaudit -r[v] directory
 *			display the security parameters of files in directory
 *		ntfssecaudit -b[v] directory
 *			backup the security parameters of files in directory
 *		ntfssecaudit -s[v] volume [backupfile]
 *			set the security parameters as indicated in backup
 *			with -e set extra parameters (Windows attrib)
 *		ntfssecaudit perms file
 *			set the security parameters of file to perms (mode or acl)
 *		ntfssecaudit -r[v] perms directory
 *			set the security parameters of files in directory to perms
 */

/*	History
 *
 *  Nov 2007
 *     - first version, by concatenating miscellaneous utilities
 *
 *  Jan 2008, version 1.0.1
 *     - fixed mode displaying
 *     - added a global severe errors count
 *
 *  Feb 2008, version 1.0.2
 *     - implemented conversions for big-endian machines
 *
 *  Mar 2008, version 1.0.3
 *     - avoided consistency checks on $Secure when there is no such file
 *
 *  Mar 2008, version 1.0.4
 *     - changed ordering of ACE's
 *     - changed representation for special flags
 *     - defaulted to stdin for option -h
 *     - added self tests (compile time option)
 *     - fixed errors specific to big-endian computers
 *
 *  Apr 2008, version 1.1.0
 *     - developped Posix ACLs to NTFS ACLs conversions
 *     - developped NTFS ACLs backup and restore
 *
 *  Apr 2008, version 1.1.1
 *     - fixed an error specific to big-endian computers
 *     - checked hash value and fixed error report in restore
 *     - improved display in showhex() and restore()
 *
 *  Apr 2008, version 1.1.2
 *     - improved and fixed Posix ACLs to NTFS ACLs conversions
 *
 *  Apr 2008, version 1.1.3
 *     - reenabled recursion for setting a new mode or ACL
 *     - processed Unicode file names and displayed them as UTF-8
 *     - allocated dynamically memory for file names when recursing
 *
 *  May 2008, version 1.1.4
 *     - all Unicode/UTF-8 strings checked and processed
 *
 *  Jul 2008, version 1.1.5
 *     - made Windows owner consistent with Linux owner when changing mode
 *     - allowed owner change on Windows when it does not match Linux owner
 *     - skipped currently unused code
 *
 *  Aug 2008, version 1.2.0
 *     - processed \.NTFS-3G\UserMapping on Windows
 *     - made use of user mappings through the security API or direct access
 *     - fixed a bug in restore
 *     - fixed UTF-8 conversions
 *
 *  Sep 2008, version 1.3.0
 *     - split the code to have part of it shared with ntfs-3g library
 *     - fixed testing for end of SDS block
 *     - added samples checking in selftest for easier debugging
 *
 *  Oct 2008, version 1.3.1
 *     - fixed outputting long long data when testing on a Palm organizer
 *
 *  Dec 2008, version 1.3.2
 *     - fixed restoring ACLs
 *     - added optional logging of ACL hashes to facilitate restore checks
 *     - fixed collecting SACLs
 *     - fixed setting special control flags
 *     - fixed clearing existing SACLs (Linux only) and DACLs
 *     - changed the sequencing of items when quering a security descriptor
 *     - avoided recursing on junctions and symlinks on Windows
 *
 *  Jan 2009, version 1.3.3
 *     - save/restore Windows attributes (code from Faisal)
 *
 *  Mar 2009, version 1.3.4
 *     - enabled displaying attributes of a mounted file over Linux
 *
 *  Apr 2009, version 1.3.5
 *     - fixed initialisation of stand-alone user mapping
 *     - fixed POSIXACL redefinition when included in the ntfs-3g package
 *     - fixed displaying of options
 *     - fixed a dependency on the shared library version used
 *
 *  May 2009, version 1.3.6
 *     - added new samples for self testing
 *
 *  Jun 2009, version 1.3.7
 *     - fixed displaying owner and group of a mounted file over Linux
 *
 *  Jul 2009, version 1.3.8
 *     - fixed again displaying owner and group of a mounted file over Linux
 *     - cleaned some code to avoid warnings
 *
 *  Nov 2009, version 1.3.9
 *     - allowed security descriptors up to 64K
 *
 *  Nov 2009, version 1.3.10
 *     - applied patches for MacOSX from Erik Larsson
 *
 *  Nov 2009, version 1.3.11
 *     - replace <attr/xattr.h> by <sys/xattr.h> (provided by glibc)
 *
 *  Dec 2009, version 1.3.12
 *     - worked around "const" possibly redefined in config.h
 *
 *  Dec 2009, version 1.3.13
 *     - fixed the return code of dorestore()
 *
 *  Dec 2009, version 1.3.14
 *     - adapted to opensolaris
 *
 *  Jan 2010, version 1.3.15
 *     - more adaptations to opensolaris
 *     - removed the fix for return code of dorestore()
 *
 *  Jan 2010, version 1.3.16
 *     - repeated the fix for return code of dorestore()
 *
 *  Mar 2010, version 1.3.17
 *     - adapted to new default user mapping
 *     - fixed #ifdef'd code for selftest
 *
 *  May 2010, version 1.3.18
 *     - redefined early error logging
 *
 *  Mar 2011, version 1.3.19
 *     - fixed interface to ntfs_initialize_file_security()
 *
 *  Apr 2011, version 1.3.20
 *     - fixed false memory leak detection
 *
 *  Jun 2011, version 1.3.21
 *     - cleaned a few unneeded variables
 *
 *  Nov 2011, version 1.3.22
 *     - added a distinctive prefix to owner and group SID
 *     - fixed a false memory leak detection
 *
 *  Jun 2012, version 1.3.23
 *     - added support for SACL (nickgarvey)
 *
 *  Jul 2012, version 1.3.24
 *     - added self-tests for authenticated users
 *     - added display of ace-inherited flag
 *     - made runnable on OpenIndiana
 *
 *  Aug 2012, version 1.4.0
 *     - added an option for user mapping proposal
 *
 *  Sep 2013, version 1.4.1
 *     - silenced an aliasing warning by gcc >= 4.8
 *
 *  May 2014, version 1.4.2
 *     - decoded GENERIC_ALL permissions
 *     - decoded more "well-known" and generic SIDs
 *     - showed Windows ownership in verbose situations
 *     - fixed apparent const violations
 *
 *  Dec 2014, version 1.4.3
 *     - fixed displaying "UserMapping" as a file name
 *
 *  Mar 2015, version 1.4.5
 *     - adapted to new NTFS ACLs when owner is same as group
 *
 *  May 2015, version 1.4.6
 *     - made to load shared library based on generic name
 *
 *  Mar 2016, Version 1.5.0
 *     - reorganized to rely on libntfs-3g even on Windows
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *		General parameters which may have to be adapted to needs
 */

#define AUDT_VERSION "1.5.0"

#define SELFTESTS 0
#define NOREVBOM 0 /* still unclear what this should be */

/*
 *			External declarations
 */

#include "config.h"

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_TIME_H
#include <time.h>
#endif /* HAVE_TIME_H */
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#else /* HAVE_SETXATTR */
#warning "The extended attribute package is not available"
#endif /* HAVE_SETXATTR */

#include "types.h"
#include "endians.h"
#include "support.h"
#include "layout.h"
#include "param.h"
#include "ntfstime.h"
#include "device_io.h"
#include "device.h"
#include "logging.h"
#include "runlist.h"
#include "mft.h"
#include "inode.h"
#include "attrib.h"
#include "bitmap.h"
#include "index.h"
#include "volume.h"
#include "unistr.h"
#include "mst.h"
#include "security.h"
#include "acls.h"
#include "realpath.h"
#include "utils.h"
#include "misc.h"

struct CALLBACK;

typedef int (*dircallback)(void *context, const ntfschar *ntfsname,
	const int length, const int type, const s64 pos,
	const MFT_REF mft_ref, const unsigned int dt_type);

#if POSIXACLS

static BOOL same_posix(struct POSIX_SECURITY *pxdesc1,
			struct POSIX_SECURITY *pxdesc2);

#endif /* POSIXACLS */

#ifndef HAVE_SYSLOG_H
void ntfs_log_early_error(const char *format, ...)
			__attribute__((format(printf, 1, 2)));
#endif /* HAVE_SYSLOG_H */

#define ACCOUNTSIZE 256  /* maximum size of an account name */
#define MAXFILENAME 4096
#define MAXATTRSZ 65536 /* Max sec attr size (16448 met for WinXP) */
#define MAXLINE 80 /* maximum processed size of a line */
#define BUFSZ 1024		/* buffer size to read mapping file */
#define LINESZ 120		/* maximum useful size of a mapping line */

typedef enum { RECSHOW, RECSET, RECSETPOSIX } RECURSE;
typedef enum { MAPNONE, MAPEXTERN, MAPLOCAL, MAPDUMMY } MAPTYPE;
typedef enum { CMD_AUDIT, CMD_BACKUP, CMD_HEX, CMD_HELP, CMD_SET,
			CMD_TEST, CMD_USERMAP, CMD_VERSION, CMD_NONE } CMDS;


#define MAXSECURID 262144
#define SECBLKSZ 8
#define MAPDIR ".NTFS-3G"
#define MAPFILE "UserMapping"

#ifdef HAVE_WINDOWS_H
#define DIRSEP "\\"
#else
#define DIRSEP "/"
#endif

	/* standard owner (and administrator) rights */

#define OWNER_RIGHTS (DELETE | READ_CONTROL | WRITE_DAC | WRITE_OWNER \
			| SYNCHRONIZE \
			| FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES \
			| FILE_READ_EA | FILE_WRITE_EA)

	/* standard world rights */

#define WORLD_RIGHTS (READ_CONTROL | FILE_READ_ATTRIBUTES | FILE_READ_EA \
			| SYNCHRONIZE)

	  /* inheritance flags for files and directories */

#define FILE_INHERITANCE NO_PROPAGATE_INHERIT_ACE
#define DIR_INHERITANCE (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE)

/*
 *		To identify NTFS ACL meaning Posix ACL granted to root
 *	we use rights always granted to anybody, so they have no impact
 *	either on Windows or on Linux.
 */

#define ROOT_OWNER_UNMARK SYNCHRONIZE	/* ACL granted to root as owner */
#define ROOT_GROUP_UNMARK FILE_READ_EA	/* ACL granted to root as group */

#define INSDS1 1
#define INSDS2 2
#define INSII 4
#define INSDH 8

struct SII {		/* this is an image of an $SII index entry */
	le16 offs;
	le16 size;
	le32 fill1;
	le16 indexsz;
	le16 indexksz;
	le16 flags;
	le16 fill2;
	le32 keysecurid;

	/* did not find official description for the following */
	le32 hash;
	le32 securid;
	le32 dataoffsl; /* documented as badly aligned */
	le32 dataoffsh;
	le32 datasize;
} ;

struct SDH {		/* this is an image of an $SDH index entry */
	le16 offs;
	le16 size;
	le32 fill1;
	le16 indexsz;
	le16 indexksz;
	le16 flags;
	le16 fill2;
	le32 keyhash;
	le32 keysecurid;

	/* did not find official description for the following */
	le32 hash;
	le32 securid;
	le32 dataoffsl;
	le32 dataoffsh;
	le32 datasize;
	le32 fill3;
	} ;

#ifdef HAVE_WINDOWS_H
/*
 *	Including <windows.h> leads to numerous conflicts with layout.h
 *	so define a few needed Windows calls unrelated to ntfs-3g
 */
BOOL WINAPI LookupAccountSidA(const char*, void*, char*, u32*,
		char*, u32*, s32*);
u32 WINAPI GetFileAttributesA(const char*);
#endif /* HAVE_WINDOWS_H */

#define INVALID_FILE_ATTRIBUTES (-1)/* error from ntfs_get_file_attributes() */

/*
 *		Structures for collecting directory contents
 */

struct LINK {
	struct LINK *next;
	char name[1];
} ;

struct CALLBACK {
	struct LINK *head;
	const char *dir;
} ;

static int callback(void *context, const ntfschar *ntfsname,
	const int length, const int type, const s64 pos,
	const MFT_REF mft_ref, const unsigned int dt_type);

struct SECURITY_DATA {
        u64 offset;
        char *attr;
        u32 hash;
        u32 length;
        unsigned int filecount:16;
        unsigned int mode:12;
        unsigned int flags:4;
} ;

/*
 *		  Global constants
 */

#define BANNER "ntfssecaudit " AUDT_VERSION " : NTFS security data auditing"

#ifdef SELFTESTS

/*
 *		Dummy mapping file (self tests only)
 */

#define DUMMYAUTH "S-1-5-21-3141592653-589793238-462843383-"
char dummymapping[] =	"500::" DUMMYAUTH "1000\n"
			"501::" DUMMYAUTH "1001\n"
			"502::" DUMMYAUTH "1002\n"
			"503::" DUMMYAUTH "1003\n"
			"516::" DUMMYAUTH "1016\n"
			":500:" DUMMYAUTH "513\r\n"
			":511:S-1-5-21-1607551490-981732888-1819828000-513\n"
			":516:" DUMMYAUTH "1012\r\n"
			"::"	DUMMYAUTH "10000\n";

/*
 *		SID for authenticated user (S-1-5-11)
 */

static const char authsidbytes[] = {
		1,		/* revision */
		1,		/* auth count */
		0, 0, 0, 0, 0, 5,	/* base */
		11, 0, 0, 0	/* 1st level */
};

static const SID *authsid = (const SID*)authsidbytes;

/*
 *		SID for local users (S-1-5-32-545)
 */

static const char localsidbytes[] = {
		1,		/* revision */
		2,		/* auth count */
		0, 0, 0, 0, 0, 5,	/* base */
		32, 0, 0, 0,	/* 1st level */
		33, 2, 0, 0	/* 2nd level */
};

static const SID *localsid = (const SID*)localsidbytes;

/*
 *		SID for system (S-1-5-18)
 */

static const char systemsidbytes[] = {
		1,		/* revision */
		1,		/* auth count */
		0, 0, 0, 0, 0, 5,	/* base */
		18, 0, 0, 0	/* 1st level */
	};

static const SID *systemsid = (const SID*)systemsidbytes;

#endif /* SELFTESTS */

/*
 *		  Global variables
 */

BOOL opt_e;	/* restore extra (currently windows attribs) */
BOOL opt_r;	/* recursively apply to subdirectories */
BOOL opt_u;	/* user mapping proposal */
int opt_v;  /* verbose or very verbose*/
CMDS cmd; /* command to process */
struct SECURITY_DATA *securdata[(MAXSECURID + (1 << SECBLKSZ) - 1)/(1 << SECBLKSZ)];
unsigned int errors; /* number of severe errors */
unsigned int warnings; /* number of non-severe errors */

struct SECURITY_CONTEXT context;
MAPTYPE mappingtype;
struct SECURITY_API *ntfs_context = (struct SECURITY_API*)NULL;

/*
 *		Open and close the security API (obsolete)
 */

static BOOL open_security_api(void)
{
	return (TRUE);
}

static BOOL close_security_api(void)
{
	return (0);
}

/*
 *		Open and close a volume
 *	Assumes a single volume is opened
 */

static BOOL open_volume(const char *volume, unsigned long flags)
{
	BOOL ok;

	ok = FALSE;
	if (!ntfs_context) {
		ntfs_context = ntfs_initialize_file_security(volume, flags);
		if (ntfs_context) {
			if (*(u32*)ntfs_context != MAGIC_API) {
				fprintf(stderr,"Versions of ntfs-3g and ntfssecaudit"
						" are not compatible\n");
			} else {
				fprintf(stderr,"\"%s\" opened %s\n",volume,
					(flags & NTFS_MNT_RDONLY
						 ? "read-only" : "read-write"));
				mappingtype = MAPEXTERN;
				ok = TRUE;
			}
		} else {
			fprintf(stderr,"Could not open \"%s\"\n",volume);
#ifdef HAVE_WINDOWS_H
			switch (errno) {
			case EACCES :
				fprintf(stderr,"You need Administrator rights to open \"%s\"\n",
							volume);
				break;
			case EBUSY :
				fprintf(stderr,"Looks like \"%s\" is mounted,\n",volume);
				fprintf(stderr,"close all applications using it\n");
				break;
			default :
				fprintf(stderr,"Close all applications using %s\n", volume);
				fprintf(stderr,"to make sure it is not mounted\n");
			}
#else
			fprintf(stderr,"Make sure \"%s\" is not mounted\n",volume);
#endif
		}
	} else
		fprintf(stderr,"A volume is already open\n");
	return (ok);
}

static BOOL close_volume(const char *volume)
{
	BOOL r;

	r = ntfs_leave_file_security(ntfs_context);
	if (r)
		fprintf(stderr,"\"%s\" closed\n",volume);
	else
		fprintf(stderr,"Could not close \"%s\"\n",volume);
	ntfs_context = (struct SECURITY_API*)NULL;
	return (r);
}

#ifdef HAVE_WINDOWS_H

/*
 *		Make a path suitable for feeding to libntfs-3g
 *
 *	Use '/' as a directory separator and remove /../ and /./
 */

static int cleanpath(char *path)
{
	int err;
	char *p;
	char *s, *d;

	err = 0;
	for (p=path; *p; p++)
		if (*p == '\\')
			*p = '/';
	do {
		s = (char*)NULL;
		p = strstr(path, "/./");
		if (p) {
			s = p + 3;
			d = p + 1;
		} else {
			p = strstr(path, "/../");
			if (p) {
				d = p;
				while ((d != path) && (*--d != '/'))
					d--;
				if ((d != p) && (*d == '/')) {
					s = p + 3;
				} else
					err = 1;
			}
		}
		if (s) {
			while (*s)
				*d++ = *s++;
			*d = 0;
		}
	} while (s && !err);
	return (err);
}

/*
 *		Build a path with Unix-type separators
 *
 *	The path from the ntfs root is required for libntfs-3g calls
 */

static char *unixname(const char *name)
{
	char *uname;

	uname = (char*)malloc(strlen(name) + 1);
	if (uname) {
		strcpy(uname, name);
		if (cleanpath(uname)) {
			fprintf(stderr,"Bad path %s\n",name);
			free(uname);
			uname = (char*)NULL;
		}
	}
	return (uname);
}

#endif /* HAVE_WINDOWS_H */

/*
 *		Extract small or big endian data from an array of bytes
 */

static unsigned int get2l(const char *attr, int p)
{
	int i;
	unsigned int v;

	v = 0;
	for (i=0; i<2; i++)
		v += (attr[p+i] & 255) << (8*i);
	return (v);
}

static unsigned long get4l(const char *attr, int p)
{
	int i;
	unsigned long v;

	v = 0;
	for (i=0; i<4; i++)
		v += ((long)(attr[p+i] & 255)) << (8*i);
	return (v);
}

static u64 get6h(const char *attr, int p)
{
	int i;
	u64 v;

	v = 0;
	for (i=0; i<6; i++)
		v = (v << 8) + (attr[p+i] & 255);
	return (v);
}

static u64 get8l(const char *attr, int p)
{
	int i;
	u64 v;

	v = 0;
	for (i=0; i<8; i++)
		v += ((long long)(attr[p+i] & 255)) << (8*i);
	return (v);
}

/*
 *		Set small or big endian data into an array of bytes
 */

static void set2l(char *p, unsigned int v)
{
	int i;

	for (i=0; i<2; i++)
		p[i] = ((v >> 8*i) & 255);
}

static void set4l(char *p, unsigned long v)
{
	int i;

	for (i=0; i<4; i++)
		p[i] = ((v >> 8*i) & 255);
}


/*
 *		hexadecimal dump of an array of bytes
 */

static void hexdump(const char *attr, int size, int level)
{
	int i,j;

	for (i=0; i<size; i+=16) {
		if (level)
			printf("%*c",level,' ');
		printf("%06x ",i);
		for (j=i; (j<(i+16)) && (j<size); j++)
			printf((j & 3 ? "%02x" : " %02x"),attr[j] & 255);
		printf("\n");
	}
}

static u32 hash(const le32 *buf, int size /* bytes */)
{
	u32 h;
	int i;

	h = 0;
	for (i=0; 4*i<size; i++)
		h = le32_to_cpu(buf[i]) + (h << 3) + ((h >> 29) & 7);
	return (h);
}

/*
 *		Evaluate the size of UTS-8 conversion of a UTF-16LE text
 *	trailing '\0' not accounted for
 *	Returns 0 for invalid input
 */

static unsigned int utf8size(const ntfschar *utf16, int length)
{
	int i;
	int count = 0;
	BOOL surrog;
	BOOL fail;

	surrog = FALSE;
	fail = FALSE;
	for (i = 0; i < length && utf16[i] && !fail; i++) {
		unsigned short c = le16_to_cpu(utf16[i]);
		if (surrog) {
			if ((c >= 0xdc00) && (c < 0xe000)) {
				surrog = FALSE;
				count += 4;
			} else 
				fail = TRUE;
		} else
			if (c < 0x80)
				count++;
			else if (c < 0x800)
				count += 2;
			else if (c < 0xd800)
				count += 3;
			else if (c < 0xdc00)
				surrog = TRUE;
#if NOREVBOM
			else if ((c >= 0xe000) && (c < 0xfffe))
#else
			else if (c >= 0xe000)
#endif
				count += 3;
			else 
				fail = TRUE;
	}
	if (surrog) 
		fail = TRUE;

	return (fail ? 0 : count);
}

/*
 *		Convert a UTF-16LE text to UTF-8
 *	Note : wcstombs() not used because on Linux it fails for characters
 *	not present in current locale
 *	Returns size or zero for invalid input
 */

static unsigned int makeutf8(char *utf8, const ntfschar *utf16, int length)
{
	int size;

	size = ntfs_ucstombs(utf16, length, &utf8, MAXFILENAME);
	return (size < 0 ? 0 : size);
}

/*
 *		Print a file name
 *	on Windows it prints UTF-16LE names as UTF-8
 */

static void printname(FILE *file, const char *name)
{
#ifdef HAVE_WINDOWS_H
	char *wname;
	char *p;

	wname = (char*)malloc(strlen(name) + 1);
	if (wname) {
		strcpy(wname, name);
		for (p=wname; *p; p++)
			if (*p == '/')
				*p = '\\';
		fprintf(file,"%s", wname);
		free(wname);
	}
#else /* HAVE_WINDOWS_H */
	fprintf(file,"%s",name);
#endif /* HAVE_WINDOWS_H */
}

/*
 *		Print the last error code
 */

static void printerror(FILE *file)
{
	if (errno)
		fprintf(file,"Error code %d : %s\n",errno,strerror(errno));
	switch (errno) {
	case EACCES :
		fprintf(file,"You probably need Administrator rights\n");
		break;
	case EBUSY :
		fprintf(file,"You probably try to write to a mounted device\n");
		break;
	default :
		break;
	}
}

#ifndef HAVE_SYSLOG_H

/*
 *		Redefine early error messages in stand-alone situations
 */

static void ntfs_log_early_error(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vfprintf(stderr,format,args);
	va_end(args);
}

#endif /* HAVE_SYSLOG_H */

/*
 *	Guess whether a security attribute is intended for a directory
 *	based on the presence of inheritable ACE
 *	(not 100% reliable)
 */

static BOOL guess_dir(const char *attr)
{
	int off;
	int isdir;
	int cnt;
	int i;
	int x;

	isdir = 0;
	off = get4l(attr,16);
	if (off) {
		cnt = get2l(attr,off+4);
		x = 8;
		for (i=0; i<cnt; i++) {
			if (attr[off + x + 1] & 3)
				isdir = 1;
			x += get2l(attr,off + x + 2);
		}
	}
	return (isdir);
}

/*
 *		   Display a SID
 *   See http://msdn2.microsoft.com/en-us/library/aa379649.aspx
 */

static void showsid(const char *attr, int off, const char *prefix, int level)
{
	int cnt;
	int i;
	BOOL known;
	u64 auth;
	unsigned long first;
	unsigned long second;
	unsigned long last;
	char marker;

	if (cmd == CMD_BACKUP)
		marker = '#';
	else
		marker = ' ';
	cnt = attr[off+1] & 255;
	auth = get6h(attr,off+2);
	known = FALSE;
	/* SID names taken from https://support.microsoft.com/en-us/kb/243330 */
	if ((attr[off] == 1) /* revision */
	     && cnt
	     && (auth < 100)) {
		first = get4l(attr,off+8);
		switch (cnt) {
		case 0 : /* no level (error) */
			break;
		case 1 : /* single level */
			switch (auth) {
			case 0 :
				if (first == 0) {
					known = TRUE;
					printf("%*cNull SID\n",-level,marker);
				}
				break;
			case 1 :
				if (first == 0) {
					known = TRUE;
					printf("%*cWorld SID\n",-level,marker);
				}
				break;
			case 3 :
				switch (first) {
				case 0 :
					known = TRUE;
					printf("%*cCreator owner SID\n",-level,marker);
					break;
				case 1 :
					known = TRUE;
					printf("%*cCreator group SID\n",-level,marker);
					break;
				}
				break;
			case 5 :
				switch (first) {
				case 1 :
					known = TRUE;
					printf("%*cDialup SID\n",-level,marker);
					break;
				case 2 :
					known = TRUE;
					printf("%*cNetwork SID\n",-level,marker);
					break;
				case 3 :
					known = TRUE;
					printf("%*cBatch SID\n",-level,marker);
					break;
				case 4 :
					known = TRUE;
					printf("%*cInteractive SID\n",-level,marker);
					break;
				case 6 :
					known = TRUE;
					printf("%*cService SID\n",-level,marker);
					break;
				case 7 :
					known = TRUE;
					printf("%*cAnonymous SID\n",-level,marker);
					break;
				case 11 :
					known = TRUE;
					printf("%*cAuthenticated Users SID\n",-level,marker);
					break;
				case 13 :
					known = TRUE;
					printf("%*cTerminal Server Users SID\n",-level,marker);
					break;
				case 14 :
					known = TRUE;
					printf("%*cRemote Interactive Logon SID\n",-level,marker);
					break;
				case 18 :
					known = TRUE;
					printf("%*cLocal System SID\n",-level,marker);
					break;
				}
				break;
			}
			break;
		case 2 : /* double level */
			second = get4l(attr,off+12);
			switch (auth) {
			case 5 :
				if (first == 32) {
					known = TRUE;
					switch (second) {
					case 544 :
						printf("%*cAdministrators SID\n",-level,marker);
						break;
					case 545 :
						printf("%*cUsers SID\n",-level,marker);
						break;
					case 546 :
						printf("%*cGuests SID\n",-level,marker);
						break;
					default :
						printf("%*cSome domain SID\n",-level,marker);
						break;
					}
				}
				break;
			}
			break;
		default : /* three levels or more */
			second = get4l(attr,off+12);
			last = get4l(attr,off+4+4*cnt);
			switch (auth) {
			case 5 :
				if (first == 21) {
					known = TRUE;
					switch (last) {
					case 500 :
						printf("%*cAdministrator SID\n",-level,marker);
						break;
					case 501 :
						printf("%*cGuest SID\n",-level,marker);
						break;
					case 512 :
						printf("%*cDomain Admins SID\n",-level,marker);
						break;
					case 513 :
						printf("%*cDomain Users SID\n",-level,marker);
						break;
					case 514 :
						printf("%*cDomain Guests SID\n",-level,marker);
						break;
					default :
						printf("%*cLocal user-%lu SID\n",-level,marker,last);
						break;
					}
				}
				break;
			}
		}
	}
	if (!known)
		printf("%*cUnknown SID\n",-level,marker);
	printf("%*c%shex S-%x-",-level,marker,prefix,attr[off] & 255);
	printf("%llx",(long long)auth);
	for (i=0; i<cnt; i++)
		printf("-%lx",get4l(attr,off+8+4*i));
	printf("\n");
	printf("%*c%sdec S-%u-",-level,marker,prefix,attr[off] & 255);
	printf("%llu",(long long)auth);
	for (i=0; i<cnt; i++)
		printf("-%lu",get4l(attr,off+8+4*i));
	printf("\n");
}

static void showusid(const char *attr, int level)
{
	int off;
	char marker;

	if (cmd == CMD_BACKUP)
		marker = '#';
	else
		marker = ' ';
	if (level)
		printf("%*c",-level,marker);
	printf("Owner SID\n");
	off = get4l(attr,4);
	showsid(attr,off,"O:",level+4);
}

static void showgsid(const char *attr, int level)
{
	int off;
	char marker;

	if (cmd == CMD_BACKUP)
		marker = '#';
	else
		marker = ' ';
	if (level)
		printf("%*c",-level,marker);
	printf("Group SID\n");
	off = get4l(attr,8);
	showsid(attr,off,"G:",level+4);
}

static void showownership(const char *attr)
{
#ifdef HAVE_WINDOWS_H
	char account[ACCOUNTSIZE];
	BIGSID sidcopy;
	s32 use;
	u32 accountsz;
	u32 domainsz;
#endif /* HAVE_WINDOWS_H */
	enum { SHOWOWN, SHOWGRP, SHOWINT, SHOWDONE } shown;
	const char *sid;
	const char *prefix;
	u64 auth;
	int cnt;
	int off;
	int i;

	for (shown=SHOWOWN; shown<SHOWDONE; ) {
		switch (shown) {
		case SHOWOWN :
			off = get4l(attr,4);
			sid = &attr[off];
			prefix = "Windows owner";
			shown = SHOWGRP;
			break;
		case SHOWGRP :
			off = get4l(attr,8);
			sid = &attr[off];
			prefix = "Windows group";
			shown = SHOWINT;
			break;
#if OWNERFROMACL
		case SHOWINT :
			off = get4l(attr,4);
			prefix = "Interpreted owner";
			sid = (const char*)ntfs_acl_owner((const char*)attr);
			if (ntfs_same_sid((const SID*)sid,
						(const SID*)&attr[off]))
				sid = (const char*)NULL;
			shown = SHOWDONE;
			break;
#endif /* OWNERFROMACL */
		default :
			sid = (const char*)NULL;
			prefix = (const char*)NULL;
			shown = SHOWDONE;
			break;
		}
		if (sid) {
			cnt = sid[1] & 255;
			auth = get6h(sid,2);
			if (cmd == CMD_BACKUP)
				printf("# %s S-%d-",prefix,sid[0] & 255);
			else
				printf("%s S-%d-",prefix,sid[0] & 255);
			printf("%llu",(long long)auth);
			for (i=0; i<cnt; i++)
				printf("-%lu",get4l(sid,8+4*i));
#ifdef HAVE_WINDOWS_H
			memcpy(sidcopy,sid,ntfs_sid_size((const SID*)sid));
			accountsz = ACCOUNTSIZE;
			domainsz = ACCOUNTSIZE;
			if (LookupAccountSidA((const char*)NULL, sidcopy,
					account, &accountsz,
					(char*)NULL, &domainsz, &use))
				printf(" (%s)", account);
#endif /* HAVE_WINDOWS_H */
			printf("\n");
		}
	}
}

static void showheader(const char *attr, int level)
{
	int flags;
	char marker;

	if (cmd == CMD_BACKUP)
		marker = '#';
	else
		marker = ' ';
	if (level)
		printf("%*c",-level,marker);
	printf("Global header\n");
	printf("%*crevision %d\n",-level-4,marker,attr[0]);
	flags = get2l(attr,2);
	printf("%*cflags    0x%x\n",-level-4,marker,flags);
	if (flags & 1)
		printf("%*c    owner is defaulted\n",-level-4,marker);
	if (flags & 2)
		printf("%*c    group is defaulted\n",-level-4,marker);
	if (flags & 4)
		printf("%*c    DACL present\n",-level-4,marker);
	if (flags & 8)
		printf("%*c    DACL is defaulted\n",-level-4,marker);
	if (flags & 0x10)
		printf("%*c    SACL present\n",-level-4,marker);
	if (flags & 0x20)
		printf("%*c    SACL is defaulted\n",-level-4,marker);
	if (flags & 0x100)
		printf("%*c    DACL inheritance is requested\n",-level-4,marker);
	if (flags & 0x200)
		printf("%*c    SACL inheritance is requested\n",-level-4,marker);
	if (flags & 0x400)
		printf("%*c    DACL was inherited automatically\n",-level-4,marker);
	if (flags & 0x800)
		printf("%*c    SACL was inherited automatically\n",-level-4,marker);
	if (flags & 0x1000)
		printf("%*c    DACL cannot be modified by inheritable ACEs\n",-level-4,marker);
	if (flags & 0x2000)
		printf("%*c    SACL cannot be modified by inheritable ACEs\n",-level-4,marker);
	if (flags & 0x8000)
		printf("%*c    self relative descriptor\n",-level-4,marker);
	if (flags & 0x43eb)
		printf("%*c    unknown flags 0x%x present\n",-level-4,marker,
				flags & 0x43eb);
	printf("%*cOff USID 0x%x\n",-level-4,marker,(int)get4l(attr,4));
	printf("%*cOff GSID 0x%x\n",-level-4,marker,(int)get4l(attr,8));
	printf("%*cOff SACL 0x%x\n",-level-4,marker,(int)get4l(attr,12));
	printf("%*cOff DACL 0x%x\n",-level-4,marker,(int)get4l(attr,16));
}

static void showace(const char *attr, int off, int isdir, int level)
{
	int flags;
	u32 rights;
	char marker;

	if (cmd == CMD_BACKUP)
		marker = '#';
	else
		marker = ' ';
	printf("%*ctype     %d\n",-level,marker,attr[off]);
	switch (attr[off]) {
	case 0 :
		printf("%*cAccess allowed\n",-level-4,marker);
		break;
	case 1 :
		printf("%*cAccess denied\n",-level-4,marker);
		break;
	case 2 :
		printf("%*cSystem audit\n",-level-4,marker);
		break;
	default :
		printf("%*cunknown\n",-level-4,marker);
		break;
	}
	flags = attr[off+1] & 255;
	printf("%*cflags    0x%x\n",-level,marker,flags);
	if (flags & 1)
		printf("%*cObject inherits ACE\n",-level-4,marker);
	if (flags & 2)
		printf("%*cContainer inherits ACE\n",-level-4,marker);
	if (flags & 4)
		printf("%*cDon\'t propagate inherits ACE\n",-level-4,marker);
	if (flags & 8)
		printf("%*cInherit only ACE\n",-level-4,marker);
	if (flags & 0x10)
		printf("%*cACE was inherited\n",-level-4,marker);
	if (flags & 0x40)
		printf("%*cAudit on success\n",-level-4,marker);
	if (flags & 0x80)
		printf("%*cAudit on failure\n",-level-4,marker);

	printf("%*cSize     0x%x\n",-level,marker,get2l(attr,off+2));

	rights = get4l(attr,off+4);
	printf("%*cAcc rgts 0x%lx\n",-level,marker,(long)rights);
	printf("%*cObj specific acc rgts 0x%lx\n",-level-4,marker,(long)rights & 65535);
	if (isdir) /* a directory */ {
		if (rights & 0x01)
			printf("%*cList directory\n",-level-8,marker);
		if (rights & 0x02)
			printf("%*cAdd file\n",-level-8,marker);
		if (rights & 0x04)
			printf("%*cAdd subdirectory\n",-level-8,marker);
		if (rights & 0x08)
			printf("%*cRead EA\n",-level-8,marker);
		if (rights & 0x10)
			printf("%*cWrite EA\n",-level-8,marker);
		if (rights & 0x20)
			printf("%*cTraverse\n",-level-8,marker);
		if (rights & 0x40)
			printf("%*cDelete child\n",-level-8,marker);
		if (rights & 0x80)
			printf("%*cRead attributes\n",-level-8,marker);
		if (rights & 0x100)
			printf("%*cWrite attributes\n",-level-8,marker);
	}
	else {
	     /* see FILE_READ_DATA etc in winnt.h */
		if (rights & 0x01)
			printf("%*cRead data\n",-level-8,marker);
		if (rights & 0x02)
			printf("%*cWrite data\n",-level-8,marker);
		if (rights & 0x04)
			printf("%*cAppend data\n",-level-8,marker);
		if (rights & 0x08)
			printf("%*cRead EA\n",-level-8,marker);
		if (rights & 0x10)
			printf("%*cWrite EA\n",-level-8,marker);
		if (rights & 0x20)
			printf("%*cExecute\n",-level-8,marker);
		if (rights & 0x80)
			printf("%*cRead attributes\n",-level-8,marker);
		if (rights & 0x100)
			printf("%*cWrite attributes\n",-level-8,marker);
	}
	printf("%*cstandard acc rgts 0x%lx\n",-level-4,marker,(long)(rights >> 16) & 127);
	if (rights & 0x10000)
		printf("%*cDelete\n",-level-8,marker);
	if (rights & 0x20000)
		printf("%*cRead control\n",-level-8,marker);
	if (rights & 0x40000)
		printf("%*cWrite DAC\n",-level-8,marker);
	if (rights & 0x80000)
		printf("%*cWrite owner\n",-level-8,marker);
	if (rights & 0x100000)
		printf("%*cSynchronize\n",-level-8,marker);
	if (rights & 0x800000)
		printf("%*cCan access security ACL\n",-level-4,marker);
	if (rights & 0x10000000)
		printf("%*cGeneric all\n",-level-4,marker);
	if (rights & 0x20000000)
		printf("%*cGeneric execute\n",-level-4,marker);
	if (rights & 0x40000000)
		printf("%*cGeneric write\n",-level-4,marker);
	if (rights & 0x80000000)
		printf("%*cGeneric read\n",-level-4,marker);

	printf("%*cSID at 0x%x\n",-level,marker,off+8);
	showsid(attr,off+8,"",level+4);
	printf("%*cSummary :",-level,marker);
	if (attr[off] == 0)
		printf(" grant");
	if (attr[off] == 1)
		printf(" deny");
	if (rights & le32_to_cpu(FILE_GREAD | FILE_GWRITE | FILE_GEXEC)) {
		printf(" ");
		if (rights & le32_to_cpu(FILE_GREAD))
			printf("r");
		if (rights & le32_to_cpu(FILE_GWRITE))
			printf("w");
		if (rights & le32_to_cpu(FILE_GEXEC))
			printf("x");
	} else
		printf(" none");
	if (flags & 11)
		printf(" inherited");
	if (!(flags & 8)) {
		int sz;

		printf(" applied");
		sz = attr[off+9]*4 + 8;
		if (!memcmp(&attr[off+8],&attr[get4l(attr,4)],sz))
			printf(" to owner");
		if (!memcmp(&attr[off+8],&attr[get4l(attr,8)],sz))
			printf(" to group");
	}
	printf("\n");

}

static void showacl(const char *attr, int off, int isdir, int level)
{
	int i;
	int cnt;
	int size;
	int x;
	char marker;

	if (cmd == CMD_BACKUP)
		marker = '#';
	else
		marker = ' ';
	size = get2l(attr,off+2);
	printf("%*crevision %d\n",-level,marker,attr[off]);
	printf("%*cACL size %d\n",-level,marker,size);
	cnt = get2l(attr,off+4);
	printf("%*cACE cnt  %d\n",-level,marker,cnt);
	x = 8;
	for (i=0; (i<cnt) && (x < size); i++) {
		printf("%*cACE %d at 0x%x\n",-level,marker,i + 1,off+x);
		showace(attr,off + x,isdir,level+4);
		x += get2l(attr,off + x + 2);
	}
}

static void showdacl(const char *attr, int isdir, int level)
{
	int off;
	char marker;

	if (cmd == CMD_BACKUP)
		marker = '#';
	else
		marker = ' ';
	off = get4l(attr,16);
	if (off) {
		if (level)
			printf("%*c",-level,marker);
		printf("DACL\n");
		showacl(attr,off,isdir,level+4);
	} else {
		if (level)
			printf("%*c",-level,marker);
		printf("No DACL\n");
	}
}

static void showsacl(const char *attr, int isdir, int level)
{
	int off;
	char marker;

	if (cmd == CMD_BACKUP)
		marker = '#';
	else
		marker = ' ';
	off = get4l(attr,12);
	if (off) {
		if (level)
			printf("%*c",-level,marker);
		printf("SACL\n");
		showacl(attr,off,isdir,level+4);
	}
	else {
		if (level)
			printf("%*c",-level,marker);
		printf("No SACL\n");
	}
}

static void showall(const char *attr, int level)
{
	BOOL isdir;

	isdir = guess_dir(attr);
	showheader(attr,level);
	showusid(attr,level);
	showgsid(attr,level);
	showdacl(attr,isdir,level);
	showsacl(attr,isdir,level);
}

#if POSIXACLS
/*
 *		Display a Posix descriptor
 */

static void showposix(const struct POSIX_SECURITY *pxdesc)
{
	char txperm[4];
	const char *txtag;
	const char *txtype;
	const struct POSIX_ACL *acl;
	const struct POSIX_ACE *pxace;
	int acccnt;
	int defcnt;
	int firstdef;
	int perms;
	u16 tag;
	s32 id;
	int k,l;

	if (pxdesc) {
		acccnt = pxdesc->acccnt;
		defcnt = pxdesc->defcnt;
		firstdef = pxdesc->firstdef;
		acl = &pxdesc->acl;
		printf("Posix descriptor :\n");
		printf("    acccnt %d\n",acccnt);
		printf("    defcnt %d\n",defcnt);
		printf("    firstdef %d\n",firstdef);
		printf("    mode : 0%03o\n",(int)pxdesc->mode);
		printf("    tagsset : 0x%02x\n",(int)pxdesc->tagsset);
		printf("Posix ACL :\n");
		printf("    version %d\n",(int)acl->version);
		printf("    flags 0x%02x\n",(int)acl->flags);
		for (k=0; k<(acccnt + defcnt); k++) {
			if (k < acccnt)
				l = k;
			else
				l = firstdef + k - acccnt;
			pxace = &acl->ace[l];
			tag = pxace->tag;
			perms = pxace->perms;
			if (tag == POSIX_ACL_SPECIAL) {
				txperm[0] = (perms & S_ISVTX ? 's' : '-');
				txperm[1] = (perms & S_ISUID ? 'u' : '-');
				txperm[2] = (perms & S_ISGID ? 'g' : '-');
			} else {
				txperm[0] = (perms & 4 ? 'r' : '-');
				txperm[1] = (perms & 2 ? 'w' : '-');
				txperm[2] = (perms & 1 ? 'x' : '-');
			}
			txperm[3] = 0;
			if (k >= acccnt)
				txtype = "default";
			else
				txtype = "access ";
			switch (tag) {
			case POSIX_ACL_USER :
				txtag = "USER ";
				break;
			case POSIX_ACL_USER_OBJ :
				txtag = "USR-O";
				break;
			case POSIX_ACL_GROUP :
				txtag = "GROUP";
				break;
			case POSIX_ACL_GROUP_OBJ :
				txtag = "GRP-O";
				break;
			case POSIX_ACL_MASK :
				txtag = "MASK ";
				break;
			case POSIX_ACL_OTHER :
				txtag = "OTHER";
				break;
			case POSIX_ACL_SPECIAL :
				txtag = "SPECL";
				break;
			default :
				txtag = "UNKWN";
				break;
			}
			id = pxace->id;
			printf("ace %d : %s %s %4ld perms 0%03o %s\n",
				l,txtype,txtag,(long)id,
				perms,txperm);
		}
	} else
		printf("** NULL ACL\n");
}

#endif /* POSIXACLS */

/*
 *		Relay to get uid as defined during mounting
 */

static uid_t relay_find_user(const struct MAPPING *mapping __attribute__((unused)),
			const SID *usid)
{
	int uid;

	uid = ntfs_get_user(ntfs_context, usid);
	return (uid < 0 ? 0 : uid);
}

/*
 *		Relay to get gid as defined during mounting
 */

static gid_t relay_find_group(const struct MAPPING *mapping __attribute__((unused)),
			const SID *gsid)
{
	int gid;

	gid = ntfs_get_group(ntfs_context, gsid);
	return (gid < 0 ? 0 : gid);
}

#if POSIXACLS

static struct POSIX_SECURITY *linux_permissions_posix(const char *attr, BOOL isdir)
{
	const SECURITY_DESCRIPTOR_RELATIVE *phead;
#if OWNERFROMACL
	const SID *osid;
#endif /* OWNERFROMACL */
	const SID *usid;
	const SID *gsid;
	struct POSIX_SECURITY *posix_desc;

	phead = (const SECURITY_DESCRIPTOR_RELATIVE*)attr;
	gsid = (const SID*)&attr[le32_to_cpu(phead->group)];
#if OWNERFROMACL
	osid = (const SID*)&attr[le32_to_cpu(phead->owner)];
	usid = ntfs_acl_owner((const char*)attr);
#ifdef SELFTESTS
	if ((cmd != CMD_TEST) && !ntfs_same_sid(usid,osid))
		printf("== Linux owner is different from Windows owner\n");
#else /* SELFTESTS */
	if (!ntfs_same_sid(usid,osid))
		printf("== Linux owner is different from Windows owner\n");
#endif /* SELFTESTS */
#else /* OWNERFROMACL */
	usid = (const SID*)&attr[le32_to_cpu(phead->owner)];
#endif /* OWNERFROMACL */
	if (mappingtype == MAPEXTERN)
		posix_desc = ntfs_build_permissions_posix(
				ntfs_context->security.mapping,
				(const char*)attr, usid, gsid, isdir);
	else
		posix_desc = ntfs_build_permissions_posix(context.mapping,
				(const char*)attr, usid, gsid, isdir);
	if (!posix_desc) {
		printf("** Failed to build a Posix descriptor\n");
		errors++;
	}
	return (posix_desc);
}

#endif /* POSIXACLS */

static int linux_permissions(const char *attr, BOOL isdir)
{
	const SECURITY_DESCRIPTOR_RELATIVE *phead;
#if OWNERFROMACL
	const SID *osid;
#endif /* OWNERFROMACL */
	const SID *usid;
	const SID *gsid;
	int perm;

	phead = (const SECURITY_DESCRIPTOR_RELATIVE*)attr;
	gsid = (const SID*)&attr[le32_to_cpu(phead->group)];
#if OWNERFROMACL
	osid = (const SID*)&attr[le32_to_cpu(phead->owner)];
	usid = ntfs_acl_owner((const char*)attr);
#ifdef SELFTESTS
	if ((cmd != CMD_TEST) && !ntfs_same_sid(usid,osid))
		printf("== Linux owner is different from Windows owner\n");
#else /* SELFTESTS */
	if (!ntfs_same_sid(usid,osid))
		printf("== Linux owner is different from Windows owner\n");
#endif /* SELFTESTS */
#else /* OWNERFROMACL */
	usid = (const SID*)&attr[le32_to_cpu(phead->owner)];
#endif /* OWNERFROMACL */
	perm = ntfs_build_permissions((const char*)attr, usid, gsid, isdir);
	if (perm < 0) {
		printf("** Failed to build permissions\n");
		errors++;
	}
	return (perm);
}

static uid_t linux_owner(const char *attr)
{
	const SID *usid;
	uid_t uid;

#if OWNERFROMACL
	usid = ntfs_acl_owner((const char*)attr);
#else /* OWNERFROMACL */
	const SECURITY_DESCRIPTOR_RELATIVE *phead;

	phead = (const SECURITY_DESCRIPTOR_RELATIVE*)attr;
	usid = (const SID*)&attr[le32_to_cpu(phead->owner)];
#endif /* OWNERFROMACL */
#ifdef HAVE_WINDOWS_H
	uid = ntfs_find_user(context.mapping[MAPUSERS],usid);
#else /* defined(HAVE_WINDOWS_H) */
	if (mappingtype == MAPEXTERN)
		uid = relay_find_user(context.mapping[MAPUSERS],usid);
	else
		uid = ntfs_find_user(context.mapping[MAPUSERS],usid);
#endif /* defined(HAVE_WINDOWS_H) */
	return (uid);
}

static gid_t linux_group(const char *attr)
{
	const SECURITY_DESCRIPTOR_RELATIVE *phead;
	const SID *gsid;
	gid_t gid;

	phead = (const SECURITY_DESCRIPTOR_RELATIVE*)attr;
	gsid = (const SID*)&attr[le32_to_cpu(phead->group)];
#ifdef HAVE_WINDOWS_H
	gid = ntfs_find_group(context.mapping[MAPGROUPS],gsid);
#else /* defined(HAVE_WINDOWS_H) */
	if (mappingtype == MAPEXTERN)
		gid = relay_find_group(context.mapping[MAPGROUPS],gsid);
	else
		gid = ntfs_find_group(context.mapping[MAPGROUPS],gsid);
#endif /* defined(HAVE_WINDOWS_H) */
	return (gid);
}

static void newblock(s32 key)
{
	struct SECURITY_DATA *psecurdata;
	int i;

	if ((key > 0) && (key < MAXSECURID) && !securdata[key >> SECBLKSZ]) {
		securdata[key >> SECBLKSZ] =
			(struct SECURITY_DATA*)malloc((1 << SECBLKSZ)*sizeof(struct SECURITY_DATA));
		if (securdata[key >> SECBLKSZ])
			for (i=0; i<(1 << SECBLKSZ); i++) {
				psecurdata = &securdata[key >> SECBLKSZ][i];
				psecurdata->filecount = 0;
				psecurdata->mode = 0;
				psecurdata->flags = 0;
				psecurdata->attr = (char*)NULL;
			}
	}
}

static void freeblocks(void)
{
	int i,j;
	struct SECURITY_DATA *psecurdata;

	for (i=0; i<(MAXSECURID + (1 << SECBLKSZ) - 1)/(1 << SECBLKSZ); i++)
		if (securdata[i]) {
			for (j=0; j<(1 << SECBLKSZ); j++) {
				psecurdata = &securdata[i][j];
				if (psecurdata->attr)
					free(psecurdata->attr);
			}
			free(securdata[i]);
		}
}

/*
 *		Basic read from a user mapping file (Win32)
 */

static int basicread(void *fileid, char *buf, size_t size,
		off_t pos __attribute__((unused)))
{
	return (read(*(int*)fileid, buf, size));
}

#ifdef SELFTESTS

/*
 *		Read a dummy mapping file for tests
 */

static int dummyread(void *fileid  __attribute__((unused)),
		char *buf, size_t size, off_t pos)
{
	size_t sz;

	if (pos >= (off_t)(sizeof(dummymapping) - 1))
		sz = 0;
	else
		if ((size + pos) >= (sizeof(dummymapping) - 1))
			sz = sizeof(dummymapping) - 1 - pos;
		else
			sz = size;
	if (sz > 0)
		memcpy(buf,&dummymapping[pos],sz);
	return (sz);
}

#endif /* SELFTESTS */

/*
 *		Apply default single user mapping
 *	returns zero if successful
 */

static int do_default_mapping(struct MAPPING *mapping[],
			 const SID *usid)
{
	struct MAPPING *usermapping;
	struct MAPPING *groupmapping;
	SID *sid;
	int sidsz;
	int res;

	res = -1;
	sidsz = ntfs_sid_size(usid);
	sid = (SID*)ntfs_malloc(sidsz);
	if (sid) {
		memcpy(sid,usid,sidsz);
		usermapping = (struct MAPPING*)ntfs_malloc(sizeof(struct MAPPING));
		if (usermapping) {
			groupmapping = (struct MAPPING*)ntfs_malloc(sizeof(struct MAPPING));
			if (groupmapping) {
				usermapping->sid = sid;
				usermapping->xid = 0;
				usermapping->next = (struct MAPPING*)NULL;
				groupmapping->sid = sid;
				groupmapping->xid = 0;
				groupmapping->next = (struct MAPPING*)NULL;
				mapping[MAPUSERS] = usermapping;
				mapping[MAPGROUPS] = groupmapping;
				res = 0;
			}
		}
	}
	return (res);
}

/*
 *		Build the user mapping
 *	- according to a mapping file if defined (or default present),
 *	- or try default single user mapping if possible
 *
 *	The mapping is specific to a mounted device
 *	No locking done, mounting assumed non multithreaded
 *
 *	returns zero if mapping is successful
 *	(failure should not be interpreted as an error)
 */

static int local_build_mapping(struct MAPPING *mapping[], const char *usermap_path)
{
#ifdef HAVE_WINDOWS_H
	char mapfile[sizeof(MAPDIR) + sizeof(MAPFILE) + 6];
	char currpath[MAXFILENAME];
#else /* HAVE_WINDOWS_H */
	char *mapfile;
	char *p;
#endif /* HAVE_WINDOWS_H */
	int fd;
	struct MAPLIST *item;
	struct MAPLIST *firstitem = (struct MAPLIST*)NULL;
	struct MAPPING *usermapping;
	struct MAPPING *groupmapping;
	static struct {
		u8 revision;
		u8 levels;
		be16 highbase;
		be32 lowbase;
		le32 level1;
		le32 level2;
		le32 level3;
		le32 level4;
		le32 level5;
	} defmap = {
		1, 5, const_cpu_to_be16(0), const_cpu_to_be32(5),
		const_cpu_to_le32(21),
		const_cpu_to_le32(DEFSECAUTH1), const_cpu_to_le32(DEFSECAUTH2),
		const_cpu_to_le32(DEFSECAUTH3), const_cpu_to_le32(DEFSECBASE)
	} ;

	/* be sure not to map anything until done */
	mapping[MAPUSERS] = (struct MAPPING*)NULL;
	mapping[MAPGROUPS] = (struct MAPPING*)NULL;

	if (usermap_path) {
#ifdef HAVE_WINDOWS_H
/* TODO : check whether the device can store acls */
		strcpy(mapfile,"x:\\" MAPDIR "\\" MAPFILE);
		if (((const le16*)usermap_path)[1] == ':')
  			mapfile[0] = usermap_path[0];
		else {
			getcwd(currpath,MAXFILENAME);
			mapfile[0] = currpath[0];
		}
		fd = open(mapfile,O_RDONLY);
#else /* HAVE_WINDOWS_H */
		fd = 0;
		mapfile = (char*)malloc(MAXFILENAME);
		if (mapfile) {
			/* build a full path to locate the mapping file */
/*
			if ((usermap_path[0] != '/')
			   && getcwd(mapfile,MAXFILENAME)) {
				strcat(mapfile,"/");
				strcat(mapfile,usermap_path);
			} else
				strcpy(mapfile,usermap_path);
*/
			p = ntfs_realpath(usermap_path, mapfile);
			if (p)
				p = strrchr(mapfile,'/');
			if (p)
				do {
					strcpy(p,"/" MAPDIR "/" MAPFILE);
					fd = open(mapfile,O_RDONLY);
					if (fd <= 0) {
						*p = 0;
						p = strrchr(mapfile,'/');
						if (p == mapfile)
							p = (char*)NULL;
					}
				} while ((fd <= 0) && p);
			free(mapfile);
			if (!p) {
				printf("** Could not find the user mapping file\n");
				if (usermap_path[0] != '/')
					printf("   Retry with full path of file\n");
				errors++;
			}
		}
#endif /* HAVE_WINDOWS_H */
		if (fd > 0) {
			firstitem = ntfs_read_mapping(basicread, (void*)&fd);
			close(fd);
		}
	} else {
#ifdef SELFTESTS
		firstitem = ntfs_read_mapping(dummyread, (void*)NULL);
#endif /* SELFTESTS */
	}

	if (firstitem) {
		usermapping = ntfs_do_user_mapping(firstitem);
		groupmapping = ntfs_do_group_mapping(firstitem);
		if (usermapping && groupmapping) {
			mapping[MAPUSERS] = usermapping;
			mapping[MAPGROUPS] = groupmapping;
		} else
			ntfs_log_error("There were no valid user or no valid group\n");
		/* now we can free the memory copy of input text */
		/* and rely on internal representation */
		while (firstitem) {
			item = firstitem->next;
			free(firstitem);
			firstitem = item;
		}
	} else {
		do_default_mapping(mapping,(const SID*)&defmap);
	}
	if (mapping[MAPUSERS])
		mappingtype = MAPLOCAL;
	return (!mapping[MAPUSERS]);
}

/*
 *		Get an hexadecimal number (source with MSB first)
 */

static u32 getmsbhex(const char *text)
{
	u32 v;
	int b;
	BOOL ok;

	v = 0;
	ok = TRUE;
	do {
		b = *text++;
		if ((b >= '0') && (b <= '9'))
			v = (v << 4) + b - '0';
		else
			if ((b >= 'a') && (b <= 'f'))
				v = (v << 4) + b - 'a' + 10;
			else
				if ((b >= 'A') && (b <= 'F'))
					v = (v << 4) + b - 'A' + 10;
				else ok = FALSE;
	} while (ok);
	return (v);
}


/*
 *		Get an hexadecimal number (source with LSB first)
 *	An odd number of digits might yield a strange result
 */

static u32 getlsbhex(const char *text)
{
	u32 v;
	int b;
	BOOL ok;
	int pos;

	v = 0;
	ok = TRUE;
	pos = 0;
	do {
		b = *text++;
		if ((b >= '0') && (b <= '9'))
			v |= (u32)(b - '0') << (pos ^ 4);
		else
			if ((b >= 'a') && (b <= 'f'))
				v |= (u32)(b - 'a' + 10) << (pos ^ 4);
			else
				if ((b >= 'A') && (b <= 'F'))
					v |= (u32)(b - 'A' + 10) << (pos ^ 4);
				else ok = FALSE;
		pos += 4;
	} while (ok);
	return (v);
}


/*
 *		Check whether a line looks like an hex dump
 */

static BOOL ishexdump(const char *line, int first, int lth)
{
	BOOL ok;
	int i;
	int b;

	ok = (first >= 0) && (lth >= (first + 16));
	for (i=0; ((first+i)<lth) && ok; i++) {
		b = line[first + i];
		if ((i == 6)
		    || (i == 7)
		    || (i == 16)
		    || (i == 25)
		    || (i == 34)
		    || (i == 43))
			ok = (b == ' ') || (b == '\n');
		else
			ok = ((b >= '0') && (b <= '9'))
			    || ((b >= 'a') && (b <= 'f'))
			    || ((b >= 'A') && (b <= 'F'));
	}
	return (ok);
}


/*
 *		Display security descriptors from a file
 *	This is typically to convert a verbose output to a very verbose one
 */

static void showhex(FILE *fd)
{
	static char attr[MAXATTRSZ];
	char line[MAXLINE+1];
#if POSIXACLS
	struct POSIX_SECURITY *pxdesc;
#endif /* POSIXACLS */
	int lth;
	int first;
	unsigned int pos;
	u32 v;
	int c;
	int isdir;
	int mode;
	unsigned int off;
	int i;
	le32 *pattr;
	BOOL acceptable;
	BOOL isdump;
	BOOL done;

	pos = 0;
	off = 0;
	done = FALSE;
	do {
			/* input a (partial) line without displaying */
		lth = 0;
		first = -1;
		do {
			c = getc(fd);
			if ((c != ' ') && (first < 0))
				first = lth;
			if (c == EOF)
				done = TRUE;
			else
				if (c != '\r')
					line[lth++] = c;
		} while (!done && (c != '\n') && (lth < MAXLINE));
			/* check whether this looks like an hexadecimal dump */
		isdump = ishexdump(line, first, lth);
		if (isdump) off = getmsbhex(&line[first]);
			/* line is not an hexadecimal dump */
			/* display what we have in store if acceptable */
		acceptable = ((!isdump || !off)
				&& (pos >= 20))
				&& (pos > get4l(attr,4))
				&& (pos > get4l(attr,8))
				&& (pos > get4l(attr,12))
				&& (pos > get4l(attr,16))
				&& (pos >= ntfs_attr_size(attr));
		if (acceptable) {
			printf("	Computed hash : 0x%08lx\n",
				    (unsigned long)hash((le32*)attr,
				    ntfs_attr_size(attr)));
			isdir = guess_dir(attr);
			printf("    Estimated type : %s\n",
					(isdir ? "directory" : "file"));
			if (!ntfs_valid_descr((char*)attr,pos)) {
				printf("**  Bad descriptor,"
					" trying to display anyway\n");
				errors++;
			}
			showheader(attr,4);
			showusid(attr,4);
			showgsid(attr,4);
			showdacl(attr,isdir,4);
			showsacl(attr,isdir,4);
			showownership(attr);
			mode = linux_permissions(attr,isdir);
			printf("Interpreted Unix mode 0%03o\n",mode);
#if POSIXACLS
				/*
				 * Posix display not possible when user
				 * mapping is not available (option -h)
				 */
			if (mappingtype != MAPNONE) {
				pxdesc = linux_permissions_posix(attr,isdir);
				if (pxdesc) {
					showposix(pxdesc);
					free(pxdesc);
				}
			}
#endif /* POSIXACLS */
			pos = 0;
		}
		if (isdump && !off)
			pos = off;
			/* line looks like an hexadecimal dump */
			/* decode it into attribute */
		if (isdump && (off == pos)) {
			for (i=first+8; i<lth; i+=9) {
				pattr = (le32*)&attr[pos];
				v = getlsbhex(&line[i]);
				*pattr = cpu_to_le32(v);
				pos += 4;
			}
		}
			/* display (full) current line */
		if (lth) printf("! ");
		for (i=0; i<lth; i++) {
			c = line[i];
			putchar(c);
		}
		while (!done && (c != '\n')) {
			c = getc(fd);
			if (c == EOF)
				done = TRUE;
			else
				putchar(c);
		}
	} while (!done);
}

static BOOL applyattr(const char *fullname, const char *attr,
			BOOL withattr, int attrib, s32 key)
{
	struct SECURITY_DATA *psecurdata;
	const char *curattr;
	char *newattr;
	int selection;
	BOOL bad;
	BOOL badattrib;
	BOOL err;

	err = FALSE;
	psecurdata = (struct SECURITY_DATA*)NULL;
	curattr = (const char*)NULL;
	newattr = (char*)NULL;
	if ((key > 0) && (key < MAXSECURID)) {
		if (!securdata[key >> SECBLKSZ])
			newblock(key);
		if (securdata[key >> SECBLKSZ]) {
			psecurdata = &securdata[key >> SECBLKSZ]
					[key & ((1 << SECBLKSZ) - 1)];
		}
	}

			/* If we have a usable attrib value. Try applying */
	badattrib = FALSE;
	if (opt_e && (attrib != INVALID_FILE_ATTRIBUTES)) {
		badattrib = !ntfs_set_file_attributes(ntfs_context, fullname, attrib);
		if (badattrib) {
			printf("** Could not set Windows attrib of ");
			printname(stdout,fullname);
			printf(" to 0x%x\n", attrib);
			printerror(stdout);
			warnings++;
		}
	}

	if (withattr) {
		if (psecurdata) {
			newattr = (char*)malloc(ntfs_attr_size(attr));
			if (newattr) {
				memcpy(newattr,attr,ntfs_attr_size(attr));
				psecurdata->attr = newattr;
			}
		}
		curattr = attr;
	} else
		/*
		 * No explicit attr in backup, use attr defined
		 * previously for the same id
		 */
		if (psecurdata)
			curattr = psecurdata->attr;


	if (curattr) {
		selection = OWNER_SECURITY_INFORMATION
			| GROUP_SECURITY_INFORMATION
			| DACL_SECURITY_INFORMATION
			| SACL_SECURITY_INFORMATION;
		bad = !ntfs_set_file_security(ntfs_context,fullname,
			selection, (const char*)curattr);
		if (bad) {
			printf("** Could not set the ACL of ");
			printname(stdout,fullname);
			printf("\n");
			printerror(stdout);
			err = TRUE;
		} else
			if (opt_v) {
				if (opt_e && !badattrib)
					printf("ACL and attrib have been applied to ");
				else
					printf("ACL has been applied to ");
				printname(stdout,fullname);
				printf("\n");

			}
	} else {
		printf("** There was no valid ACL for ");
		printname(stdout,fullname);
		printf("\n");
		err = TRUE;
	}
	return (!err);
}

/*
 *		Restore security descriptors from a file
 */

static BOOL restore(FILE *fd)
{
	static char attr[MAXATTRSZ];
	char line[MAXFILENAME+25];
	char fullname[MAXFILENAME+25];
	SECURITY_DESCRIPTOR_RELATIVE *phead;
	int lth;
	int first;
	unsigned int pos;
	int c;
	int isdir;
	int mode;
	s32 key;
	BOOL isdump;
	unsigned int off;
	u32 v;
	u32 oldhash;
	int i;
	int count;
	int attrib;
	le32 *pattr;
	BOOL withattr;
	BOOL done;

	pos = 0;
	off = 0;
	done = FALSE;
	withattr = FALSE;
	oldhash = 0;
	key = 0;
	errors = 0;
	count = 0;
	fullname[0] = 0;
	attrib = INVALID_FILE_ATTRIBUTES;
	do {
			/* input a (partial) line without processing */
		lth = 0;
		first = -1;
		do {
			c = getc(fd);
			if ((c != ' ') && (first < 0))
				first = lth;
			if (c == EOF)
				done = TRUE;
			else
				if (c != '\r')
					line[lth++] = c;
		} while (!done && (c != '\n') && (lth < (MAXFILENAME + 24)));
			/* check whether this looks like an hexadecimal dump */
		isdump = ishexdump(line, first, lth);
		if (isdump) off = getmsbhex(&line[first]);
			/* line is not an hexadecimal dump */
			/* apply what we have in store, only if valid */
		if ((!isdump || !off) && pos && ntfs_valid_descr((char*)attr,pos)) {
			withattr = TRUE;
			if (opt_v >= 2) {
				printf("	Computed hash : 0x%08lx\n",
					    (unsigned long)hash((le32*)attr,
					    ntfs_attr_size(attr)));
				isdir = guess_dir(attr);
				printf("    Estimated type : %s\n",(isdir ? "directory" : "file"));
				showheader(attr,4);
				showusid(attr,4);
				showgsid(attr,4);
				showdacl(attr,isdir,4);
				showsacl(attr,isdir,4);
				mode = linux_permissions(attr,isdir);
				showownership(attr);
				printf("Interpreted Unix mode 0%03o\n",mode);
			}
			pos = 0;
		}
		if (isdump && !off)
			pos = off;
			/* line looks like an hexadecimal dump */
			/* decode it into attribute */
		if (isdump && (off == pos)) {
			for (i=first+8; i<lth; i+=9) {
				pattr = (le32*)&attr[pos];
				v = getlsbhex(&line[i]);
				*pattr = cpu_to_le32(v);
				pos += 4;
			}
		}
			/* display (full) current line unless dump or verbose */
		if (!isdump || opt_v) {
			if(lth) printf("! ");
			for (i=0; i<lth; i++) {
				c = line[i];
				putchar(c);
			}
		}
		while (!done && (c != '\n')) {
			c = getc(fd);
			if (c == EOF)
				done = TRUE;
			else
				if (!isdump || opt_v)
					putchar(c);
		}

		line[lth] = 0;
		while ((lth > 0)
		    && ((line[lth-1] == '\n') || (line[lth-1] == '\r')))
			line[--lth] = 0;
		if (!strncmp(line,"Computed hash : 0x",18))
			oldhash = getmsbhex(&line[18]);
		if (!strncmp(line,"Security key : 0x",17))
			key = getmsbhex(&line[17]);
		if (!strncmp(line,"Windows attrib : 0x",19))
			attrib = getmsbhex(&line[19]);
		if (done
		    || !strncmp(line,"File ",5)
		    || !strncmp(line,"Directory ",10)) {
			/*
			 *  New file or directory (or end of file) :
			 *  apply attribute just collected
			 *  or apply attribute defined from current key
			 */

			if (withattr
			    && oldhash
			    && (hash((const le32*)attr,ntfs_attr_size(attr)) != oldhash)) {
				printf("** ACL rejected, its hash is not as expected\n");
				errors++;
			} else
				if (fullname[0]) {
					phead = (SECURITY_DESCRIPTOR_RELATIVE*)attr;
					/* set the request for auto-inheritance */
					if (phead->control & SE_DACL_AUTO_INHERITED)
						phead->control |= SE_DACL_AUTO_INHERIT_REQ;
					if (!applyattr(fullname,attr,withattr,
							attrib,key))
						errors++;
					else
						count++;
				}
			/* save current file or directory name */
			withattr = FALSE;
			key = 0;
			oldhash = 0;
			attrib = INVALID_FILE_ATTRIBUTES;
			if (!done) {
				if (!strncmp(line,"File ",5))
					strcpy(fullname,&line[5]);
				else
					strcpy(fullname,&line[10]);
#ifdef HAVE_WINDOWS_H
				cleanpath(fullname);
#endif /* HAVE_WINDOWS_H */
			}
		}
	} while (!done);
	printf("%d ACLs have been applied\n",count);
	return (FALSE);
}

static BOOL dorestore(const char *volume, FILE *fd)
{
	BOOL err;

	err = FALSE;
	if (!getuid()) {
 		if (open_security_api()) {
			if (open_volume(volume,NTFS_MNT_NONE)) {
				if (restore(fd)) err = TRUE;
				close_volume(volume);
			} else {
				fprintf(stderr,"Could not open volume %s\n",volume);
				printerror(stderr);
				err = TRUE;
			}
			close_security_api();
		} else {
			fprintf(stderr,"Could not open security API\n");
			printerror(stderr);
			err = TRUE;
		}
	} else {
		fprintf(stderr,"Restore is only possible as root\n");
		err = TRUE;
	}
	return (err);
}

#if POSIXACLS

/*
 *		Merge Posix ACL rights into an u32 (self test only)
 *
 *	Result format : -----rwxrwxrwxrwxrwx---rwxrwxrwx
 *                           U1 U2 G1 G2  M     o  g  w
 *
 *	Only two users (U1, U2) and two groups (G1, G2) taken into account
 */
static u32 merge_rights(const struct POSIX_SECURITY *pxdesc, BOOL def)
{
	const struct POSIX_ACE *pxace;
	int i;
	int users;
	int groups;
	int first;
	int last;
	u32 rights;

	rights = 0;
	users = 0;
	groups = 0;
	if (def) {
		first = pxdesc->firstdef;
		last = pxdesc->firstdef + pxdesc->defcnt - 1;
	} else {
		first = 0;
		last = pxdesc->acccnt - 1;
	}
	pxace = pxdesc->acl.ace;
	for (i=first; i<=last; i++) {
		switch (pxace[i].tag) {
		case POSIX_ACL_USER_OBJ :
			rights |= (pxace[i].perms & 7) << 6;
			break;
		case POSIX_ACL_USER :
			if (users < 2)
				rights |= ((u32)pxace[i].perms & 7) << (24 - 3*users);
			users++;
			break;
		case POSIX_ACL_GROUP_OBJ :
			rights |= (pxace[i].perms & 7) << 3;
			break;
		case POSIX_ACL_GROUP :
			if (groups < 2)
				rights |= ((u32)pxace[i].perms & 7) << (18 - 3*groups);
			groups++;
			break;
		case POSIX_ACL_MASK :
			rights |= ((u32)pxace[i].perms & 7) << 12;
			break;
		case POSIX_ACL_OTHER :
			rights |= (pxace[i].perms & 7);
			break;
		default :
			break;
		}
	}
	return (rights);
}

static BOOL same_posix(struct POSIX_SECURITY *pxdesc1,
			struct POSIX_SECURITY *pxdesc2)
{
	BOOL same;
	int i;

	same = pxdesc1
		&& pxdesc2
		&& (pxdesc1->mode == pxdesc2->mode)
		&& (pxdesc1->acccnt == pxdesc2->acccnt)
		&& (pxdesc1->defcnt == pxdesc2->defcnt)
		&& (pxdesc1->firstdef == pxdesc2->firstdef)
		&& (pxdesc1->tagsset == pxdesc2->tagsset)
		&& (pxdesc1->acl.version == pxdesc2->acl.version)
		&& (pxdesc1->acl.flags == pxdesc2->acl.flags);
	i = 0;
	while (same && (i < pxdesc1->acccnt)) {
		same = (pxdesc1->acl.ace[i].tag == pxdesc2->acl.ace[i].tag)
		   && (pxdesc1->acl.ace[i].perms == pxdesc2->acl.ace[i].perms)
		   && (pxdesc1->acl.ace[i].id == pxdesc2->acl.ace[i].id);
		i++;
	}
	i = pxdesc1->firstdef;
	while (same && (i < pxdesc1->firstdef + pxdesc1->defcnt)) {
		same = (pxdesc1->acl.ace[i].tag == pxdesc2->acl.ace[i].tag)
		   && (pxdesc1->acl.ace[i].perms == pxdesc2->acl.ace[i].perms)
		   && (pxdesc1->acl.ace[i].id == pxdesc2->acl.ace[i].id);
		i++;
	}
	return (same);
}

#endif /* POSIXACLS */

#if POSIXACLS & SELFTESTS

static void tryposix(struct POSIX_SECURITY *pxdesc)
{
	le32 owner_sid[] = /* S-1-5-21-3141592653-589793238-462843383-1016 */
		{
		cpu_to_le32(0x501), cpu_to_le32(0x05000000), cpu_to_le32(21),
		cpu_to_le32(DEFSECAUTH1), cpu_to_le32(DEFSECAUTH2),
		cpu_to_le32(DEFSECAUTH3), cpu_to_le32(1016)
		} ;
	le32 group_sid[] = /* S-1-5-21-3141592653-589793238-462843383-513 */
		{
		cpu_to_le32(0x501), cpu_to_le32(0x05000000), cpu_to_le32(21),
		cpu_to_le32(DEFSECAUTH1), cpu_to_le32(DEFSECAUTH2),
		cpu_to_le32(DEFSECAUTH3), cpu_to_le32(513)
		} ;

	char *attr;
	BOOL isdir;
	mode_t mode;
	struct POSIX_SECURITY *newpxdesc;
	struct POSIX_SECURITY *oldpxdesc;
	static char *oldattr = (char*)NULL;

	isdir = FALSE;
	if (oldattr) {
		oldpxdesc = linux_permissions_posix(oldattr, isdir);
		newpxdesc = ntfs_merge_descr_posix(pxdesc, oldpxdesc);
		if (!newpxdesc)
			newpxdesc = pxdesc;
		free(oldpxdesc);
		if (opt_v) {
			printf("merged descriptors :\n");
			showposix(newpxdesc);
		}
	} else
		newpxdesc = pxdesc;
	attr = ntfs_build_descr_posix(context.mapping,newpxdesc,
			isdir,(SID*)owner_sid,(SID*)group_sid);
	if (attr && ntfs_valid_descr(attr, ntfs_attr_size(attr))) {
		if (opt_v)
			hexdump(attr,ntfs_attr_size(attr),8);
		if (opt_v >= 2) {
			showheader(attr,4);
			showusid(attr,4);
			showgsid(attr,4);
			showdacl(attr,isdir,4);
			showsacl(attr,isdir,4);
			mode = linux_permissions(attr,isdir);
			printf("Interpreted Unix mode 0%03o\n",mode);
			printf("Interpreted back Posix descriptor :\n");
			newpxdesc = linux_permissions_posix(attr,isdir);
			showposix(newpxdesc);
			free(newpxdesc);
		}
		if (oldattr) free(oldattr);
		oldattr = attr;
	}
}

#endif /* POSIXACLS & SELFTESTS */

#ifdef SELFTESTS

/*
 *		Build a dummy security descriptor
 *	returns descriptor in allocated memory, must free() after use
 */

static char *build_dummy_descr(BOOL isdir __attribute__((unused)),
			const SID *usid, const SID *gsid,
			int cnt,
			 /* seq of int allow, SID *sid, int flags, u32 mask */
			...)
{
	char *attr;
	int attrsz;
	SECURITY_DESCRIPTOR_RELATIVE *pnhead;
	ACL *pacl;
	ACCESS_ALLOWED_ACE *pace;
	va_list ap;
	const SID *sid;
	u32 umask;
	le32 mask;
	int flags;
	BOOL allow;
	int pos;
	int usidsz;
	int gsidsz;
	int sidsz;
	int aclsz;
	int i;

	if (usid)
		usidsz = ntfs_sid_size(usid);
	else
		usidsz = 0;
	if (gsid)
		gsidsz = ntfs_sid_size(gsid);
	else
		gsidsz = 0;


	/* allocate enough space for the new security attribute */
	attrsz = sizeof(SECURITY_DESCRIPTOR_RELATIVE)	/* header */
	    + usidsz + gsidsz	/* usid and gsid */
	    + sizeof(ACL)	/* acl header */
	    + cnt*40;

	attr = (char*)ntfs_malloc(attrsz);
	if (attr) {
		/* build the main header part */
		pnhead = (SECURITY_DESCRIPTOR_RELATIVE*) attr;
		pnhead->revision = SECURITY_DESCRIPTOR_REVISION;
		pnhead->alignment = 0;
			/*
			 * The flag SE_DACL_PROTECTED prevents the ACL
			 * to be changed in an inheritance after creation
			 */
		pnhead->control = SE_DACL_PRESENT | SE_DACL_PROTECTED
				    | SE_SELF_RELATIVE;
			/*
			 * Windows prefers ACL first, do the same to
			 * get the same hash value and avoid duplication
			 */
		/* build the ACL header */
		pos = sizeof(SECURITY_DESCRIPTOR_RELATIVE);
		pacl = (ACL*)&attr[pos];
		pacl->revision = ACL_REVISION;
		pacl->alignment1 = 0;
		pacl->size = cpu_to_le16(0); /* fixed later */
		pacl->ace_count = cpu_to_le16(cnt);
		pacl->alignment2 = cpu_to_le16(0);

		/* enter the ACEs */

		pos += sizeof(ACL);
		aclsz = sizeof(ACL);
		va_start(ap,cnt);
		for (i=0; i<cnt; i++) {
			pace = (ACCESS_ALLOWED_ACE*)&attr[pos];
			allow = va_arg(ap,int);
			sid = va_arg(ap,SID*);
			flags = va_arg(ap,int);
			umask = va_arg(ap,u32);
			mask = cpu_to_le32(umask);
			sidsz = ntfs_sid_size(sid);
			pace->type = (allow ? ACCESS_ALLOWED_ACE_TYPE : ACCESS_DENIED_ACE_TYPE);
			pace->flags = flags;
			pace->size = cpu_to_le16(sidsz + 8);
			pace->mask = mask;
			memcpy(&pace->sid,sid,sidsz);
			aclsz += sidsz + 8;
			pos += sidsz + 8;
		}
		va_end(ap);

		/* append usid and gsid if defined */
		/* positions of ACL, USID and GSID into header */
		pnhead->owner = cpu_to_le32(0);
		pnhead->group = cpu_to_le32(0);
		if (usid) {
			memcpy(&attr[pos], usid, usidsz);
			pnhead->owner = cpu_to_le32(pos);
		}
		if (gsid) {
			memcpy(&attr[pos + usidsz], gsid, gsidsz);
			pnhead->group = cpu_to_le32(pos + usidsz);
		}
		/* positions of DACL and SACL into header */
		pnhead->sacl = cpu_to_le32(0);
		if (cnt) {
			pacl->size = cpu_to_le16(aclsz);
			pnhead->dacl =
			    cpu_to_le32(sizeof(SECURITY_DESCRIPTOR_RELATIVE));
		} else
			pnhead->dacl = cpu_to_le32(0);
		if (!ntfs_valid_descr(attr,pos+usidsz+gsidsz)) {
			printf("** Bad sample descriptor\n");
			free(attr);
			attr = (char*)NULL;
			errors++;
		}
	} else
		errno = ENOMEM;
	return (attr);
}

/*
 *		Check a few samples with special conditions
 */

static void check_samples(void)
{
	char *descr = (char*)NULL;
	BOOL isdir = FALSE;
	mode_t perms;
	mode_t expect = 0;
	int cnt;
	u32 expectacc;
	u32 expectdef;
#if POSIXACLS
	u32 accrights;
	u32 defrights;
	mode_t mixmode;
	struct POSIX_SECURITY *pxdesc;
	struct POSIX_SECURITY *pxsample;
	const char *txsample;
#endif /* POSIXACLS */
	le32 owner1[] = /* S-1-5-21-1833069642-4243175381-1340018762-1003 */
		{
		cpu_to_le32(0x501), cpu_to_le32(0x05000000), cpu_to_le32(21),
		cpu_to_le32(1833069642), cpu_to_le32(4243175381U),
		cpu_to_le32(1340018762), cpu_to_le32(1003)
		} ;
	le32 group1[] = /* S-1-5-21-1833069642-4243175381-1340018762-513 */
		{
		cpu_to_le32(0x501), cpu_to_le32(0x05000000), cpu_to_le32(21),
		cpu_to_le32(1833069642), cpu_to_le32(4243175381U),
		cpu_to_le32(1340018762), cpu_to_le32(513)
		} ;
	le32 group2[] = /* S-1-5-21-1607551490-981732888-1819828000-513 */
		{
		cpu_to_le32(0x501), cpu_to_le32(0x05000000), cpu_to_le32(21),
		cpu_to_le32(1607551490), cpu_to_le32(981732888),
		cpu_to_le32(1819828000), cpu_to_le32(513)
		} ;
	le32 owner3[] = /* S-1-5-21-3141592653-589793238-462843383-1016 */
		{
		cpu_to_le32(0x501), cpu_to_le32(0x05000000), cpu_to_le32(21),
		cpu_to_le32(DEFSECAUTH1), cpu_to_le32(DEFSECAUTH2),
		cpu_to_le32(DEFSECAUTH3), cpu_to_le32(1016)
		} ;
	le32 group3[] = /* S-1-5-21-3141592653-589793238-462843383-513 */
		{
		cpu_to_le32(0x501), cpu_to_le32(0x05000000), cpu_to_le32(21),
		cpu_to_le32(DEFSECAUTH1), cpu_to_le32(DEFSECAUTH2),
		cpu_to_le32(DEFSECAUTH3), cpu_to_le32(513)
		} ;

#if POSIXACLS
	struct {
		struct POSIX_SECURITY head;
		struct POSIX_ACE ace[4];
	} sampletry1 =
	{
		{ 0645, 4, 0, 4, 0x35, 0,
			{ POSIX_VERSION, 0, 0 }
		},
		{
			{ 1, 6, -1 },
			{ 4, 5, -1 },
			{ 16, 4, -1 },
			{ 32, 5, -1 }
		}
	} ;

	struct {
		struct POSIX_SECURITY head;
		struct POSIX_ACE ace[6];
	} sampletry3 =
	{
		{ 0100, 6, 0, 6, 0x3f, 0,
			{ POSIX_VERSION, 0, 0 }
		},
		{
			{ 1, 1, -1 },
			{ 2, 3, 1000 },
			{ 4, 1, -1 },
			{ 8, 3, 1002 },
			{ 16, 0, -1 },
			{ 32, 0, -1 }
		}
	} ;

	struct {
		struct POSIX_SECURITY head;
		struct POSIX_ACE ace[8];
	} sampletry4 =
	{
		{ 0140, 8, 0, 8, 0x3f, 0,
			{ POSIX_VERSION, 0, 0 }
		},
		{
			{ 1, 1, -1 },
			{ 2, 3, 516 },
			{ 2, 6, 1000 },
			{ 4, 1, -1 },
			{ 8, 6, 500 },
			{ 8, 3, 1002 },
			{ 16, 4, -1 },
			{ 32, 0, -1 }
		}
	} ;

	struct {
		struct POSIX_SECURITY head;
		struct POSIX_ACE ace[6];
	} sampletry5 =
	{
		{ 0454, 6, 0, 6, 0x3f, 0,
			{ POSIX_VERSION, 0, 0 }
		},
		{
			{ 1, 4, -1 },
			{ 2, 5, 516 },
			{ 4, 4, -1 },
			{ 8, 6, 500 },
			{ 16, 5, -1 },
			{ 32, 4, -1 }
		}
	} ;

	struct {
		struct POSIX_SECURITY head;
		struct POSIX_ACE ace[8];
	} sampletry6 =
	{
		{ 0332, 8, 0, 8, 0x3f, 0,
			{ POSIX_VERSION, 0, 0 }
		},
		{
			{ 1, 3, -1 },
			{ 2, 1,  0 },
			{ 2, 2,  1000 },
			{ 4, 6, -1 },
			{ 8, 4,  0 },
			{ 8, 5,  1002 },
			{ 16, 3, -1 },
			{ 32, 2, -1 }
		}
	} ;

	struct {
		struct POSIX_SECURITY head;
		struct POSIX_ACE ace[4];
	} sampletry8 =
	{
		{ 0677, 4, 0, 4, 0x35, 0,
			{ POSIX_VERSION, 0, 0 }
		},
		{
			{ 1, 6, -1 },
			{ 4, 7, -1 },
			{ 16, 7, -1 },
			{ 32, 7, -1 }
		}
	} ;

#endif /* POSIXACLS */


#if POSIXACLS
	for (cnt=1; cnt<=8; cnt++) {
		switch (cnt) {
		case 1 :
			pxsample = &sampletry1.head;
			txsample = "sampletry1-a";
			isdir = FALSE;
			descr = ntfs_build_descr_posix(context.mapping,&sampletry1.head,
				isdir, (const SID*)owner3, (const SID*)group3);
			break;
		case 2 :
			pxsample = &sampletry1.head;
			txsample = "sampletry1-b";
			isdir = FALSE;
			descr = ntfs_build_descr_posix(context.mapping,&sampletry1.head,
				isdir, (const SID*)adminsid, (const SID*)group3);
			break;
		case 3 :
			isdir = FALSE;
			pxsample = &sampletry3.head;
			txsample = "sampletry3";
			descr = ntfs_build_descr_posix(context.mapping,pxsample,
				isdir, (const SID*)group3, (const SID*)group3);
			break;
		case 4 :
			isdir = FALSE;
			pxsample = &sampletry4.head;
			txsample = "sampletry4";
			descr = ntfs_build_descr_posix(context.mapping,pxsample,
				isdir, (const SID*)owner3, (const SID*)group3);
			break;
		case 5 :
			isdir = FALSE;
			pxsample = &sampletry5.head;
			txsample = "sampletry5";
			descr = ntfs_build_descr_posix(context.mapping,pxsample,
				isdir, (const SID*)owner3, (const SID*)group3);
			break;
		case 6 :
			isdir = FALSE;
			pxsample = &sampletry6.head;
			txsample = "sampletry6-a";
			descr = ntfs_build_descr_posix(context.mapping,pxsample,
				isdir, (const SID*)owner3, (const SID*)group3);
			break;
		case 7 :
			isdir = FALSE;
			pxsample = &sampletry6.head;
			txsample = "sampletry6-b";
			descr = ntfs_build_descr_posix(context.mapping,pxsample,
				isdir, (const SID*)adminsid, (const SID*)adminsid);
			break;
		case 8 :
			pxsample = &sampletry8.head;
			txsample = "sampletry8";
			isdir = FALSE;
			descr = ntfs_build_descr_posix(context.mapping,&sampletry8.head,
				isdir, (const SID*)owner3, (const SID*)group3);
			break;
		default :
			pxsample = (struct POSIX_SECURITY*)NULL;
			txsample = (const char*)NULL;
		}
				/* check we get original back */
		if (descr)
			pxdesc = linux_permissions_posix(descr, isdir);
		else
			pxdesc = (struct POSIX_SECURITY*)NULL;
		if (!descr || !pxdesc || !same_posix(pxsample,pxdesc)) {
			printf("** Error in %s (errno %d)\n",txsample,errno);
			showposix(pxsample);
			if (descr)
				showall(descr,0);
			if (pxdesc)
				showposix(pxdesc);
			errors++;
		}
		free(descr);
		free(pxdesc);
	}

#endif /* POSIXACLS */


		/*
		 *		Check a few samples built by Windows,
		 *	which cannot be generated by Linux
		 */

	for (cnt=1; cnt<=10; cnt++) {
		switch(cnt) {
		case 1 :  /* hp/tmp */
			isdir = TRUE;
			descr = build_dummy_descr(isdir,
				(const SID*)owner1, (const SID*)group1,
				1,
				(int)TRUE, worldsid, (int)0x3, (u32)0x1f01ff);
			expect = expectacc = 0777;
			expectdef = 0;
			break;
		case 2 :  /* swsetup */
			isdir = TRUE;
			descr = build_dummy_descr(isdir, adminsid, (const SID*)group2,
				2,
				(int)TRUE, worldsid, (int)0x0, (u32)0x1f01ff,
				(int)TRUE, worldsid, (int)0xb, (u32)0x1f01ff);
			expectacc = expect = 0777;
			expectdef = 0777;
			break;
		case 3 :  /* Dr Watson */
			isdir = TRUE;
			descr = build_dummy_descr(isdir, (const SID*)owner3, (const SID*)group3,
				0);
			expectacc = expect = 0700;
			expectdef = 0;
			break;
		case 4 :
			isdir = FALSE;
			descr = build_dummy_descr(isdir,
				(const SID*)owner3, (const SID*)group3,
				4,
				(int)TRUE, (const SID*)owner3, 0,
					le32_to_cpu(FILE_READ_DATA | OWNER_RIGHTS),
				(int)TRUE, (const SID*)group3, 0,
					le32_to_cpu(FILE_WRITE_DATA),
				(int)TRUE, (const SID*)group2, 0,
					le32_to_cpu(FILE_WRITE_DATA | FILE_READ_DATA),
				(int)TRUE, (const SID*)worldsid, 0,
					le32_to_cpu(FILE_EXECUTE));
			expect = 0731;
			expectacc = 07070731;
			expectdef = 0;
			break;
		case 5 :  /* Vista/JP */
			isdir = TRUE;
			descr = build_dummy_descr(isdir, systemsid, systemsid,
				6,
				(int)TRUE, owner1, (int)0x0, (u32)0x1f01ff,
				(int)TRUE, systemsid, (int)0x0, (u32)0x1f01ff,
				(int)TRUE, adminsid, (int)0x0, (u32)0x1f01ff,
				(int)TRUE, owner1, (int)0xb, (u32)0x10000000,
				(int)TRUE, systemsid, (int)0xb, (u32)0x10000000,
				(int)TRUE, adminsid, (int)0xb, (u32)0x10000000);
			expectacc = expect = 0700;
			expectdef = 0700;
			break;
		case 6 :  /* Vista/JP2 */
			isdir = TRUE;
			descr = build_dummy_descr(isdir, systemsid, systemsid,
				7,
				(int)TRUE, owner1,    (int)0x0, (u32)0x1f01ff,
				(int)TRUE, systemsid, (int)0x0, (u32)0x1f01ff,
				(int)TRUE, adminsid,  (int)0x0, (u32)0x1f01ff,
				(int)TRUE, owner1,    (int)0xb, (u32)0x1f01ff,
				(int)TRUE, systemsid, (int)0xb, (u32)0x1f01ff,
				(int)TRUE, adminsid,  (int)0xb, (u32)0x1f01ff,
				(int)TRUE, owner3,    (int)0x3, (u32)0x1200a9);
			expectacc = 0500070700;
			expectdef = expect = 0700;
			break;
		case 7 :  /* WinXP/JP */
			isdir = TRUE;
			descr = build_dummy_descr(isdir, adminsid, systemsid,
				6,
				(int)TRUE, owner1, (int)0x0, (u32)0x1f01ff,
				(int)TRUE, systemsid, (int)0x0, (u32)0x1f01ff,
				(int)TRUE, adminsid, (int)0x0, (u32)0x1f01ff,
				(int)TRUE, owner1, (int)0xb, (u32)0x10000000,
				(int)TRUE, systemsid, (int)0xb, (u32)0x10000000,
				(int)TRUE, adminsid, (int)0xb, (u32)0x10000000);
			expectacc = expect = 0700;
			expectdef = 0700;
			break;
		case 8 :  /* WinXP/JP2 */
			isdir = TRUE;
			descr = build_dummy_descr(isdir, adminsid, systemsid,
				6,
				(int)TRUE, owner1,    (int)0x0, (u32)0x1f01ff,
				(int)TRUE, systemsid, (int)0x0, (u32)0x1f01ff,
				(int)TRUE, adminsid,  (int)0x0, (u32)0x1f01ff,
				(int)TRUE, owner1,    (int)0xb, (u32)0x10000000,
				(int)TRUE, systemsid, (int)0xb, (u32)0x10000000,
				(int)TRUE, adminsid,  (int)0xb, (u32)0x10000000);
			expectacc = expect = 0700;
			expectdef = 0700;
			break;
		case 9 :  /* Win8/bin */
			isdir = TRUE;
			descr = build_dummy_descr(isdir,
				(const SID*)owner3, (const SID*)owner3,
				6,
				(int)TRUE, authsid,   (int)0x3,  (u32)0x1f01ff,
				(int)TRUE, adminsid,  (int)0x13, (u32)0x1f01ff,
				(int)TRUE, systemsid, (int)0x13, (u32)0x1f01ff,
				(int)TRUE, localsid,  (int)0x13, (u32)0x1200a9,
				(int)TRUE, authsid,   (int)0x10, (u32)0x1301bf,
				(int)TRUE, authsid,   (int)0x1b, (u32)0xe0010000);
			expectacc = expect = 0777;
			expectdef = 0777;
			break;
		case 10 :  /* Win8/bin/linem.exe */
			isdir = FALSE;
			descr = build_dummy_descr(isdir,
				(const SID*)owner3, (const SID*)owner3,
				4,
				(int)TRUE, authsid,   (int)0x10, (u32)0x1f01ff,
				(int)TRUE, adminsid,  (int)0x10, (u32)0x1f01ff,
				(int)TRUE, systemsid, (int)0x10, (u32)0x1ff,
				(int)TRUE, localsid,  (int)0x10, (u32)0x1200a9);
			expectacc = expect = 0777;
			expectdef = 0;
			break;
		default :
			expectacc = expectdef = 0;
			break;
		}
		if (descr) {
			perms = linux_permissions(descr, isdir);
			if (perms != expect) {
				printf("** Error in sample %d, perms 0%03o expected 0%03o\n",
					cnt,perms,expect);
				showall(descr,0);
				errors++;
			} else {
#if POSIXACLS
				pxdesc = linux_permissions_posix(descr, isdir);
				if (pxdesc) {
					accrights = merge_rights(pxdesc,FALSE);
					defrights = merge_rights(pxdesc,TRUE);
					if (!(pxdesc->tagsset & ~(POSIX_ACL_USER_OBJ | POSIX_ACL_GROUP_OBJ | POSIX_ACL_OTHER)))
						mixmode = expect;
					else
						mixmode = (expect & 07707) | ((accrights >> 9) & 070);
					if ((pxdesc->mode != mixmode)
					  || (accrights != expectacc)
					  || (defrights != expectdef)) {
						printf("** Error in sample %d : mode %03o expected 0%03o\n",
							cnt,pxdesc->mode,mixmode);
						printf("     Posix access rights 0%03lo expected 0%03lo\n",
							(long)accrights,(long)expectacc);
						printf("          default rights 0%03lo expected 0%03lo\n",
							(long)defrights,(long)expectdef);
						showall(descr,0);
						showposix(pxdesc);
					}
					free(pxdesc);
				}
#endif /* POSIXACLS */
			}
		free(descr);
		}
	}
}


/*
 *		Check whether any basic permission setting is interpreted
 *	back exactly as set
 */

static void basictest(int kind, BOOL isdir, const SID *owner, const SID *group)
{
	char *attr;
	mode_t perm;
	mode_t gotback;
	u32 count;
	u32 acecount;
	u32 globhash;
	const SECURITY_DESCRIPTOR_RELATIVE *phead;
	const ACL *pacl;
	enum { ERRNO,
		ERRMA, ERRPA, /* error converting mode or Posix ACL to NTFS */
		ERRAM, ERRAP, /* error converting NTFS to mode or Posix ACL */
	} err;
	u32 expectcnt[] = {
		27800, 31896,
		24064, 28160,
		24064, 28160,
		24064, 28160,
		24904, 29000
	} ;
	u32 expecthash[] = {
		0x8f80865b, 0x7bc7960,
		0x8fd9ecfe, 0xddd4db0,
		0xa8b07400, 0xa189c20,
		0xc5689a00, 0xb6c09000,
		0xb040e509, 0x4f4db7f7
	} ;
#if POSIXACLS
	struct POSIX_SECURITY *pxdesc;
	char *pxattr;
	u32 pxcount;
	u32 pxacecount;
	u32 pxglobhash;
#endif /* POSIXACLS */

	count = 0;
	acecount = 0;
	globhash = 0;
#if POSIXACLS
	pxcount = 0;
	pxacecount = 0;
	pxglobhash = 0;
#endif /* POSIXACLS */
	for (perm=0; (perm<=07777) && (errors < 10); perm++) {
		err = ERRNO;
		/* file owned by plain user and group */
		attr = ntfs_build_descr(perm,isdir,owner,(const SID*)group);
		if (attr && ntfs_valid_descr(attr, ntfs_attr_size(attr))) {
			phead = (const SECURITY_DESCRIPTOR_RELATIVE*)attr;
			pacl = (const ACL*)&attr[le32_to_cpu(phead->dacl)];
			acecount += le16_to_cpu(pacl->ace_count);
			globhash += hash((const le32*)attr,ntfs_attr_size(attr));
			count++;
#if POSIXACLS
			/*
			 * Build a NTFS ACL from a mode, and
			 * decode to a Posix ACL, expecting to
			 * get the original mode back.
			 */
			pxdesc = linux_permissions_posix(attr, isdir);
			if (!pxdesc || (pxdesc->mode != perm)) {
				err = ERRAP;
				if (pxdesc)
					gotback = pxdesc->mode;
				else
					gotback = 0;
			} else {
			/*
			 * Build a NTFS ACL from the Posix ACL, expecting to
			 * get exactly the same NTFS ACL, then decode to a
			 * mode, expecting to get the original mode back.
			 */
				pxattr = ntfs_build_descr_posix(context.mapping,
						pxdesc,isdir,owner,
						(const SID*)group);
				if (pxattr && !memcmp(pxattr,attr,
						 ntfs_attr_size(attr))) {
					phead = (const SECURITY_DESCRIPTOR_RELATIVE*)attr;
					pacl = (const ACL*)&attr[le32_to_cpu(phead->dacl)];
					pxacecount += le16_to_cpu(pacl->ace_count);
					pxglobhash += hash((const le32*)attr,ntfs_attr_size(attr));
					pxcount++;
					gotback = linux_permissions(pxattr, isdir);
					if (gotback != perm)
						err = ERRAM;
					else
						free(pxattr);
				} else
					err = ERRPA;
				free(attr);
			}
			free(pxdesc);
#else /* POSIXACLS */
			gotback = linux_permissions(attr, isdir);
			if (gotback != perm)
				err = ERRAM;
			else
				free(attr);
#endif /* POSIXACLS */
		} else
			err = ERRMA;

		switch (err) {
		case ERRMA :
			printf("** no or wrong permission settings "
				"for kind %d perm %03o\n",kind,perm);
			if (attr && opt_v)
				hexdump(attr,ntfs_attr_size(attr),8);
			if (attr && (opt_v >= 2)) {
				showheader(attr,4);
				showusid(attr,4);
				showgsid(attr,4);
				showdacl(attr,isdir,4);
				showsacl(attr,isdir,4);
			}
			errors++;
			break;
		case ERRPA :
			printf("** no or wrong permission settings from PX "
				"for kind %d perm %03o\n",kind,perm);
			errors++;
			break;
#if POSIXACLS
		case ERRAM :
			printf("** wrong permission settings, "
				"kind %d perm 0%03o, gotback %03o\n",
				kind, perm, gotback);
			if (opt_v)
				hexdump(pxattr,ntfs_attr_size(pxattr),8);
			if (opt_v >= 2) {
				showheader(pxattr,4);
				showusid(pxattr,4);
				showgsid(pxattr,4);
				showdacl(pxattr,isdir,4);
				showsacl(pxattr,isdir,4);
			}
			errors++;
			break;
		case ERRAP :
			/* continued */
#else /* POSIXACLS */
		case ERRAM :
		case ERRAP :
#endif /* POSIXACLS */
			printf("** wrong permission settings, "
				"kind %d perm 0%03o, gotback %03o\n",
				kind, perm, gotback);
			if (opt_v)
				hexdump(attr,ntfs_attr_size(attr),8);
			if (opt_v >= 2) {
				showheader(attr,4);
				showusid(attr,4);
				showgsid(attr,4);
				showdacl(attr,isdir,4);
				showsacl(attr,isdir,4);
			}
			errors++;
			free(attr);
			break;
		default :
			break;
		}
	}
	printf("%lu ACLs built from mode, %lu ACE built, mean count %lu.%02lu\n",
		(unsigned long)count,(unsigned long)acecount,
		(unsigned long)acecount/count,acecount*100L/count%100L);
	if (acecount != expectcnt[kind]) {
		printf("** Error : ACE count %lu instead of %lu\n",
			(unsigned long)acecount,
			(unsigned long)expectcnt[kind]);
		errors++;
	}
	if (globhash != expecthash[kind]) {
		printf("** Error : wrong global hash 0x%lx instead of 0x%lx\n",
			(unsigned long)globhash, (unsigned long)expecthash[kind]);
		errors++;
	}
#if POSIXACLS
	printf("%lu ACLs built from Posix ACLs, %lu ACE built, mean count %lu.%02lu\n",
		(unsigned long)pxcount,(unsigned long)pxacecount,
		(unsigned long)pxacecount/pxcount,pxacecount*100L/pxcount%100L);
	if (pxacecount != expectcnt[kind]) {
		printf("** Error : ACE count %lu instead of %lu\n",
			(unsigned long)pxacecount,
			(unsigned long)expectcnt[kind]);
		errors++;
	}
	if (pxglobhash != expecthash[kind]) {
		printf("** Error : wrong global hash 0x%lx instead of 0x%lx\n",
			(unsigned long)pxglobhash, (unsigned long)expecthash[kind]);
		errors++;
	}
#endif /* POSIXACLS */
}

#if POSIXACLS

/*
 *		Check whether Posix ACL settings are interpreted
 *	back exactly as set
 */

static void posixtest(int kind, BOOL isdir,
			const SID *owner, const SID *group)
{
	struct POSIX_SECURITY *pxdesc;
	struct {
		struct POSIX_SECURITY pxdesc;
		struct POSIX_ACE aces[10];
	} desc;
	int ownobj;
	int grpobj;
	int usr;
	int grp;
	int wrld;
	int mask;
	int mindes, maxdes;
	int minmsk, maxmsk;
	char *pxattr;
	u32 count;
	u32 acecount;
	u32 globhash;
	const SECURITY_DESCRIPTOR_RELATIVE *phead;
	const ACL *pacl;
	struct POSIX_SECURITY *gotback;
	enum { ERRNO,
		ERRMA, ERRPA, /* error converting mode or Posix ACL to NTFS */
		ERRAM, ERRAP, /* error converting NTFS to mode or Posix ACL */
	} ;
	u32 expectcnt[] = {
		252720, 273456,
		199584, 220320,
		199584, 220320,
		199584, 220320,
		203904, 224640,
		0, 0,
		0, 0,
		0, 0,
		196452, 217188,
		165888, 186624,
		165888, 186624,
		165888, 186624,
		168480, 189216,
		0, 0,
		0, 0,
		0, 0,
		16368, 18672,
		0, 0,
		13824, 0,
		0, 0,
		14640, 0
	} ;
	u32 expecthash[] = {
		0x1808a6cd, 0xd82f7c60,
		0x5ad29e85, 0x518c7620,
		0x188ce270, 0x7e44e590,
		0x48a64800, 0x5bdf0030,
		0x1c64aec6, 0x8b0168fa,
		0, 0,
		0, 0,
		0, 0,
		0x169fb80e, 0x382d9a59,
		0xf9c28164, 0x1855d352,
		0xf9685700, 0x44d16700,
		0x587ebe90, 0xf7c51480,
		0x2cb1b518, 0x52408df6,
		0, 0,
		0, 0,
		0, 0,
		0x905f2e38, 0xd40c22f0,
		0, 0,
		0xdd76da00, 0,
		0, 0,
		0x718e34a0, 0
	};

	count = 0;
	acecount = 0;
	globhash = 0;
				/* fill headers */
	pxdesc = &desc.pxdesc;
	pxdesc->mode = 0;
	pxdesc->defcnt = 0;
	if (kind & 32) {
		pxdesc->acccnt = 4;
		pxdesc->firstdef = 4;
		pxdesc->tagsset = 0x35;
	} else {
		pxdesc->acccnt = 6;;
		pxdesc->firstdef = 6;
		pxdesc->tagsset = 0x3f;
	}
	pxdesc->acl.version = POSIX_VERSION;
	pxdesc->acl.flags = 0;
	pxdesc->acl.filler = 0;
				/* prefill aces */
	pxdesc->acl.ace[0].tag = POSIX_ACL_USER_OBJ;
	pxdesc->acl.ace[0].id = -1;
	if (kind & 32) {
		pxdesc->acl.ace[1].tag = POSIX_ACL_GROUP_OBJ;
		pxdesc->acl.ace[1].id = -1;
		pxdesc->acl.ace[2].tag = POSIX_ACL_MASK;
		pxdesc->acl.ace[2].id = -1;
		pxdesc->acl.ace[3].tag = POSIX_ACL_OTHER;
		pxdesc->acl.ace[3].id = -1;
	} else {
		pxdesc->acl.ace[1].tag = POSIX_ACL_USER;
		pxdesc->acl.ace[1].id = (kind & 16 ? 0 : 1000);
		pxdesc->acl.ace[2].tag = POSIX_ACL_GROUP_OBJ;
		pxdesc->acl.ace[2].id = -1;
		pxdesc->acl.ace[3].tag = POSIX_ACL_GROUP;
		pxdesc->acl.ace[3].id = (kind & 16 ? 0 : 1002);
		pxdesc->acl.ace[4].tag = POSIX_ACL_MASK;
		pxdesc->acl.ace[4].id = -1;
		pxdesc->acl.ace[5].tag = POSIX_ACL_OTHER;
		pxdesc->acl.ace[5].id = -1;
	}

	mindes = 3;
	maxdes = (kind & 32 ? mindes : 6);
	minmsk = 0;
	maxmsk = 7;
	for (mask=minmsk; mask<=maxmsk; mask++)
	for (ownobj=1; ownobj<7; ownobj++)
	for (grpobj=1; grpobj<7; grpobj++)
	for (wrld=0; wrld<8; wrld++)
	for (usr=mindes; usr<=maxdes; usr++)
	if (usr != 4)
	for (grp=mindes; grp<=maxdes; grp++)
	if (grp != 4) {
		pxdesc->mode = (ownobj << 6) | (mask << 3) | wrld;

		pxdesc->acl.ace[0].perms = ownobj;
		if (kind & 32) {
			pxdesc->acl.ace[1].perms = grpobj;
			pxdesc->acl.ace[2].perms = mask;
			pxdesc->acl.ace[3].perms = wrld;
		} else {
			pxdesc->acl.ace[1].perms = usr;
			pxdesc->acl.ace[2].perms = grpobj;
			pxdesc->acl.ace[3].perms = grp;
			pxdesc->acl.ace[4].perms = mask;
			pxdesc->acl.ace[5].perms = wrld;
		}

		gotback = (struct POSIX_SECURITY*)NULL;
		pxattr = ntfs_build_descr_posix(context.mapping,
				pxdesc,isdir,owner,group);
		if (pxattr && ntfs_valid_descr(pxattr, ntfs_attr_size(pxattr))) {
			phead = (const SECURITY_DESCRIPTOR_RELATIVE*)pxattr;
			pacl = (const ACL*)&pxattr[le32_to_cpu(phead->dacl)];
			acecount += le16_to_cpu(pacl->ace_count);
			globhash += hash((const le32*)pxattr,ntfs_attr_size(pxattr));
			count++;
			gotback = linux_permissions_posix(pxattr, isdir);
			if (gotback) {
				if (ntfs_valid_posix(gotback)) {
					if (!same_posix(pxdesc,gotback)) {
						printf("Non matching got back Posix ACL\n");
						printf("input ACL\n");
						showposix(pxdesc);
						printf("NTFS owner\n");
						showusid(pxattr,4);
						printf("NTFS group\n");
						showgsid(pxattr,4);
						printf("NTFS DACL\n");
						showdacl(pxattr,isdir,4);
						printf("gotback ACL\n");
						showposix(gotback);
						errors++;
					}
				} else {
					printf("Got back an invalid Posix ACL\n");
					errors++;
				}
				free(gotback);
			} else {
				printf("Could not get Posix ACL back\n");
				errors++;
			}

		} else {
			printf("NTFS ACL incorrect or not build\n");
			printf("input ACL\n");
			showposix(pxdesc);
			printf("NTFS DACL\n");
			if (pxattr)
				showdacl(pxattr,isdir,4);
			else
				printf("   (none)\n");
			if (gotback) {
				printf("gotback ACL\n");
				showposix(gotback);
			} else
				printf("no gotback ACL\n");
			errors++;
		}
		if (pxattr)
			free(pxattr);
	}
	printf("%lu ACLs built from Posix ACLs, %lu ACE built, mean count %lu.%02lu\n",
		(unsigned long)count,(unsigned long)acecount,
		(unsigned long)acecount/count,acecount*100L/count%100L);
	if (acecount != expectcnt[kind]) {
		printf("** Error ! expected ACE count %lu\n",
			(unsigned long)expectcnt[kind]);
		errors++;
	}
	if (globhash != expecthash[kind]) {
		printf("** Error : wrong global hash 0x%lx instead of 0x%lx\n",
			(unsigned long)globhash, (unsigned long)expecthash[kind]);
		errors++;
	}
}

#endif /* POSIXACLS */

static void selftests(void)
{
	le32 owner_sid[] = /* S-1-5-21-3141592653-589793238-462843383-1016 */
		{
		cpu_to_le32(0x501), cpu_to_le32(0x05000000), cpu_to_le32(21),
		cpu_to_le32(DEFSECAUTH1), cpu_to_le32(DEFSECAUTH2),
		cpu_to_le32(DEFSECAUTH3), cpu_to_le32(1016)
		} ;
	le32 group_sid[] = /* S-1-5-21-3141592653-589793238-462843383-513 */
		{
		cpu_to_le32(0x501), cpu_to_le32(0x05000000), cpu_to_le32(21),
		cpu_to_le32(DEFSECAUTH1), cpu_to_le32(DEFSECAUTH2),
		cpu_to_le32(DEFSECAUTH3), cpu_to_le32(513)
		} ;
#if POSIXACLS
	unsigned char kindlist[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
			   16, 17, 18, 20, 22, 24, 19, 21, 23, 25,
			   32, 33, 36, 40 } ;
	unsigned int k;
#endif /* POSIXACLS */
	int kind;
	const SID *owner;
	const SID *group;
	BOOL isdir;

#if POSIXACLS
	local_build_mapping(context.mapping, (const char*)NULL);
#endif /* POSIXACLS */
			/* first check samples */
	mappingtype = MAPDUMMY;
	check_samples();
		/*
		 * kind is oring of :
		 *   1 : directory
		 *   2 : owner is root
		 *   4 : group is root
		 *   8 : group is owner
		 *  16 : root is designated user/group
		 *  32 : mask present with no designated user/group
		 */
	for (kind=0; (kind<10) && (errors<10); kind++) {
		isdir = kind & 1;
		if (kind & 8)
			owner = (const SID*)group_sid;
		else
			owner = (kind & 2 ? adminsid : (const SID*)owner_sid);
		group = (kind & 4 ? adminsid : (const SID*)group_sid);
		basictest(kind, isdir, owner, group);
	}
#if POSIXACLS
	for (k=0; (k<sizeof(kindlist)) && (errors<10); k++) {
		kind = kindlist[k];
		isdir = kind & 1;
		if (kind & 8)
			owner = (const SID*)group_sid;
		else
			owner = (kind & 2 ? adminsid : (const SID*)owner_sid);
		group = (kind & 4 ? adminsid : (const SID*)group_sid);
		posixtest(kind, isdir, owner, group);
	}
	ntfs_free_mapping(context.mapping);
#endif /* POSIXACLS */
	if (errors >= 10)
		printf("** too many errors, test aborted\n");
}
#endif /* SELFTESTS */

/*
 *		   Get the security descriptor of a file
 */

static unsigned int getfull(char *attr, const char *fullname)
{
	static char part[MAXATTRSZ];
	BIGSID ownsid;
	int xowner;
	int ownersz;
	u16 ownerfl;
	u32 attrsz;
	u32 partsz;
	BOOL overflow;

	attrsz = 0;
	partsz = 0;
	overflow = FALSE;
	if (ntfs_get_file_security(ntfs_context,fullname,
				OWNER_SECURITY_INFORMATION,
				(char*)part,MAXATTRSZ,&partsz)) {
		xowner = get4l(part,4);
		if (xowner) {
			ownerfl = get2l(part,2);
			ownersz = ntfs_sid_size((SID*)&part[xowner]);
			if (ownersz <= (int)sizeof(BIGSID))
				memcpy(ownsid,&part[xowner],ownersz);
			else
				overflow = TRUE;
		} else {
			ownerfl = 0;
			ownersz = 0;
		}
			/*
			 *  SACL : just feed in or clean
			 */
		if (!ntfs_get_file_security(ntfs_context,fullname,
				SACL_SECURITY_INFORMATION,
				(char*)attr,MAXATTRSZ,&attrsz)) {
			attrsz = 20;
			set4l(attr,0);
			attr[0] = SECURITY_DESCRIPTOR_REVISION;
			set4l(&attr[12],0);
			if (opt_v >= 2)
				printf("   No SACL\n");
		}
			/*
			 *  append DACL and merge its flags
			 */
		partsz = 0;
		set4l(&attr[16],0);
		if (ntfs_get_file_security(ntfs_context,fullname,
		    DACL_SECURITY_INFORMATION,
		    (char*)part,MAXATTRSZ,&partsz)) {
			if ((attrsz + partsz - 20) <= MAXATTRSZ) {
				memcpy(&attr[attrsz],&part[20],partsz-20);
				set4l(&attr[16],(partsz > 20 ? attrsz : 0));
				set2l(&attr[2],get2l(attr,2) | (get2l(part,2)
					& const_le16_to_cpu(SE_DACL_PROTECTED
						   | SE_DACL_AUTO_INHERITED
						   | SE_DACL_PRESENT)));
				attrsz += partsz - 20;
			} else
				overflow = TRUE;
		} else
			if (partsz > MAXATTRSZ)
				overflow = TRUE;
			else {
				if (cmd == CMD_BACKUP)
					printf("#   No discretionary access control list\n");
				else
					printf("   No discretionary access control list\n");
				warnings++;
			}

			/*
			 *  append owner and merge its flag
			 */
		if (xowner && !overflow) {
			memcpy(&attr[attrsz],ownsid,ownersz);
			set4l(&attr[4],attrsz);
			set2l(&attr[2],get2l(attr,2)
			   | (ownerfl & const_le16_to_cpu(SE_OWNER_DEFAULTED)));
			attrsz += ownersz;
		} else
			set4l(&attr[4],0);
			/*
			 * append group
			 */
		partsz = 0;
		set4l(&attr[8],0);
		if (ntfs_get_file_security(ntfs_context,fullname,
		    GROUP_SECURITY_INFORMATION,
		    (char*)part,MAXATTRSZ,&partsz)) {
			if ((attrsz + partsz - 20) <= MAXATTRSZ) {
				memcpy(&attr[attrsz],&part[20],partsz-20);
				set4l(&attr[8],(partsz > 20 ? attrsz : 0));
				set2l(&attr[2],get2l(attr,2) | (get2l(part,2)
					& const_le16_to_cpu(SE_GROUP_DEFAULTED)));
				attrsz += partsz - 20;
			} else
				overflow = TRUE;
		} else
			if (partsz > MAXATTRSZ)
				overflow = TRUE;
			else {
				printf("**   No group SID\n");
				warnings++;
			}
		if (overflow) {
			printf("** Descriptor was too long (> %d)\n",MAXATTRSZ);
			warnings++;
			attrsz = 0;
		} else
			if (!ntfs_valid_descr((char*)attr,attrsz)) {
				printf("** Descriptor for %s is not valid\n",fullname);
				errors++;
				attrsz = 0;
			}

	} else {
		printf("** Could not get owner of %s\n",fullname);
		warnings++;
		attrsz = 0;
	}
	return (attrsz);
}

/*
 *		Update a security descriptor
 */

static BOOL updatefull(const char *name, u32 flags, char *attr)
{
	BOOL err;

// Why was the error not seen before ?
	err = !ntfs_set_file_security(ntfs_context, name, flags, attr);
	if (err) {
		printf("** Could not change attributes of %s\n",name);
		printerror(stdout);
		errors++;
	}
	return (!err);
}


#if POSIXACLS

/*
 *		   Set all the parameters associated to a file
 */

static BOOL setfull_posix(const char *fullname, const struct POSIX_SECURITY *pxdesc,
			BOOL isdir)
{
	static char attr[MAXATTRSZ];
	struct POSIX_SECURITY *oldpxdesc;
	struct POSIX_SECURITY *newpxdesc;
	const SECURITY_DESCRIPTOR_RELATIVE *phead;
	char *newattr;
	int err;
	unsigned int attrsz;
	int newattrsz;
	const SID *usid;
	const SID *gsid;
#if OWNERFROMACL
	const SID *osid;
#endif /* OWNERFROMACL */

	printf("%s ",(isdir ? "Directory" : "File"));
	printname(stdout,fullname);
	if (pxdesc->acccnt)
		printf("\n");
	else
		printf(" mode 0%03o\n",pxdesc->mode);

	err = FALSE;
	attrsz = getfull(attr, fullname);
	if (attrsz) {
		oldpxdesc = linux_permissions_posix(attr, isdir);
		if (opt_v >= 2) {
			printf("Posix equivalent of old ACL :\n");
			showposix(oldpxdesc);
		}
		if (oldpxdesc) {
			if (!pxdesc->defcnt
			   && !(pxdesc->tagsset &
			     (POSIX_ACL_USER | POSIX_ACL_GROUP | POSIX_ACL_MASK))) {
				if (!ntfs_merge_mode_posix(oldpxdesc,pxdesc->mode))
					newpxdesc = oldpxdesc;
				else {
					newpxdesc = (struct POSIX_SECURITY*)NULL;
					free(oldpxdesc);
				}
			} else {
				newpxdesc = ntfs_merge_descr_posix(pxdesc, oldpxdesc);
				free(oldpxdesc);
			}
			if (opt_v) {
				printf("New Posix ACL :\n");
				showposix(newpxdesc);
			}
		} else
			newpxdesc = (struct POSIX_SECURITY*)NULL;
		if (newpxdesc) {
			phead = (const SECURITY_DESCRIPTOR_RELATIVE*)attr;
			gsid = (const SID*)&attr[le32_to_cpu(phead->group)];
#if OWNERFROMACL
			osid = (const SID*)&attr[le32_to_cpu(phead->owner)];
			usid = ntfs_acl_owner((const char*)attr);
			if (!ntfs_same_sid(usid,osid))
				printf("== Windows owner might change\n");
#else /* OWNERFROMACL */
			usid = (const SID*)&attr[le32_to_cpu(phead->owner)];
#endif /* OWNERFROMACL */
			if (mappingtype == MAPEXTERN)
				newattr = ntfs_build_descr_posix(
					ntfs_context->security.mapping,
					newpxdesc,isdir,usid,gsid);
			else
				newattr = ntfs_build_descr_posix(
					context.mapping,
					newpxdesc,isdir,usid,gsid);
			free(newpxdesc);
		} else
			newattr = (char*)NULL;
		if (newattr) {
			newattrsz = ntfs_attr_size(newattr);
			if (opt_v) {
				printf("New NTFS security descriptor\n");
				hexdump(newattr,newattrsz,4);
			}
			if (opt_v >= 2) {
				printf("Expected hash : 0x%08lx\n",
					(unsigned long)hash((le32*)newattr,ntfs_attr_size(newattr)));
				showheader(newattr,0);
				showusid(newattr,0);
				showgsid(newattr,0);
				showdacl(newattr,isdir,0);
				showsacl(newattr,isdir,0);
			}

			if (!updatefull(fullname,
				DACL_SECURITY_INFORMATION
				| GROUP_SECURITY_INFORMATION
				| OWNER_SECURITY_INFORMATION,
					newattr))
				err = TRUE;
/*
{
struct POSIX_SECURITY *interp;
printf("Reinterpreted new Posix :\n");
interp = linux_permissions_posix(newattr,isdir);
showposix(interp);
free(interp);
}
*/
			free(newattr);
		} else
			err = TRUE;
	} else
		err = TRUE;
	return (!err);
}

#else /* POSIXACLS */

static BOOL setfull(const char *fullname, int mode, BOOL isdir)
{
	static char attr[MAXATTRSZ];
	const SECURITY_DESCRIPTOR_RELATIVE *phead;
	char *newattr;
	int err;
	unsigned int attrsz;
	int newattrsz;
	const SID *usid;
	const SID *gsid;
#if OWNERFROMACL
	const SID *osid;
#endif /* OWNERFROMACL */

	printf("%s ",(isdir ? "Directory" : "File"));
	printname(stdout,fullname);
	printf(" mode 0%03o\n",mode);
	attrsz = getfull(attr, fullname);
	err = FALSE;
	if (attrsz) {
		phead = (const SECURITY_DESCRIPTOR_RELATIVE*)attr;
		gsid = (const SID*)&attr[le32_to_cpu(phead->group)];
#if OWNERFROMACL
		osid = (const SID*)&attr[le32_to_cpu(phead->owner)];
		usid = ntfs_acl_owner((const char*)attr);
		if (!ntfs_same_sid(usid,osid))
			printf("== Windows owner might change\n");
#else /* OWNERFROMACL */
		usid = (const SID*)&attr[le32_to_cpu(phead->owner)];
#endif /* OWNERFROMACL */
		newattr = ntfs_build_descr(mode,isdir,usid,gsid);
		if (newattr) {
			newattrsz = ntfs_attr_size(newattr);
			if (opt_v) {
				printf("Security descriptor\n");
				hexdump(newattr,newattrsz,4);
			}
			if (opt_v >= 2) {
				printf("Expected hash : 0x%08lx\n",
					(unsigned long)hash((le32*)newattr,ntfs_attr_size(newattr)));
				showheader(newattr,0);
				showusid(newattr,0);
				showgsid(newattr,0);
				showdacl(newattr,isdir,0);
				showsacl(newattr,isdir,0);
			}

			if (!updatefull(fullname,
				DACL_SECURITY_INFORMATION
				| GROUP_SECURITY_INFORMATION
				| OWNER_SECURITY_INFORMATION,
					newattr))
				err = TRUE;
			free(newattr);
		}

	} else
		err = TRUE;
	return (err);
}

#endif /* POSIXACLS */

static BOOL proposal(const char *name, const char *attr)
{
	char fullname[MAXFILENAME];
	int uoff, goff;
	int i;
	u64 uauth, gauth;
	int ucnt, gcnt;
	int uid, gid;
	BOOL err;
#ifdef HAVE_WINDOWS_H
	char driveletter;
#else /* HAVE_WINDOWS_H */
	struct stat st;
	char *p,*q;
#endif /* HAVE_WINDOWS_H */

	err = FALSE;
#ifdef HAVE_WINDOWS_H
	uid = gid = 0;
#else /* HAVE_WINDOWS_H */
	uid = getuid();
	gid = getgid();
#endif /* HAVE_WINDOWS_H */
	uoff = get4l(attr,4);
	uauth = get6h(attr,uoff+2);
	ucnt = attr[uoff+1] & 255;
	goff = get4l(attr,8);
	gauth = get6h(attr,goff+2);
	gcnt = attr[goff+1] & 255;

	if ((ucnt == 5) && (gcnt == 5)
	    && (uauth == 5) && (gauth == 5)
	    && (get4l(attr,uoff+8) == 21) && (get4l(attr,goff+8) == 21)) {
		printf("# User mapping proposal :\n");
		printf("# -------------------- cut here -------------------\n");
		if (uid)
			printf("%d::",uid);
		else
			printf("user::");
		printf("S-%d-%llu",attr[uoff] & 255,(long long)uauth);
		for (i=0; i<ucnt; i++)
			printf("-%lu",get4l(attr,uoff+8+4*i));
		printf("\n");
		if (gid)
			printf(":%d:",gid);
		else
			printf(":group:");
		printf("S-%d-%llu",attr[goff] & 255,(long long)gauth);
		for (i=0; i<gcnt; i++)
			printf("-%lu",get4l(attr,goff+8+4*i));
		printf("\n");
			/* generic rule, based on group */
		printf("::S-%d-%llu",attr[goff] & 255,(long long)gauth);
		for (i=0; i<gcnt-1; i++)
			printf("-%lu",get4l(attr,goff+8+4*i));
		printf("-10000\n");
		printf("# -------------------- cut here -------------------\n");
		if (!uid || !gid) {
			printf("# Please replace \"user\" and \"group\" above by the uid\n");
			printf("# and gid of the Linux owner and group of ");
			printname(stdout,name);
			printf(", then\n");
			printf("# insert the modified lines into .NTFS-3G/UserMapping, with .NTFS-3G\n");
		} else
			printf("# Insert the above lines into .NTFS-3G/UserMapping, with .NTFS-3G\n");
#ifdef HAVE_WINDOWS_H
		printf("# being a directory of the root of the NTFS file system.\n");

		/* Get the drive letter to the file system */
		driveletter = 0;
		if ((((name[0] >= 'a') && (name[0] <= 'z'))
			|| ((name[0] >= 'A') && (name[0] <= 'Z')))
		    && (name[1] == ':'))
			driveletter = name[0];
		else {
			if (getcwd(fullname, MAXFILENAME)
					&& (fullname[1] == ':'))
				driveletter = fullname[0];
		}
		if (driveletter) {
			printf("# Example : %c:\\.NTFS-3G\\UserMapping\n",
				driveletter);
		}
#else /* HAVE_WINDOWS_H */
		printf("# being a hidden subdirectory of the root of the NTFS file system.\n");

		/* Get the path to the root of the file system */
/*
		if (name[0] != '/') {
			p = getcwd(fullname,MAXFILENAME);
			if (p) {
				strcat(fullname,"/");
				strcat(fullname,name);
			}
		} else {
			strcpy(fullname,name);
			p = fullname;
		}
*/
		p = ntfs_realpath(name, fullname);
		if (p) {
			/* go down the path to inode 5 */
			do {
				lstat(fullname,&st);
				q = strrchr(p,'/');
				if (q && (st.st_ino != 5))
					*q = 0;
			} while (strchr(p,'/') && (st.st_ino != 5));
		}
		if (p && (st.st_ino == 5)) {
			printf("# Example : ");
			printname(stdout,p);
			printf("/.NTFS-3G/UserMapping\n");
		}
#endif /* HAVE_WINDOWS_H */
	} else {
		printf("** Not possible : ");
		printname(stdout,name);
		printf(" was not created by a Windows user\n");
		err = TRUE;
	}
	return (err);
}

/*
 *		   Display all the parameters associated to a file
 */

static void showfull(const char *fullname, BOOL isdir)
{
	static char attr[MAXATTRSZ];
	static char part[MAXATTRSZ];
#if POSIXACLS
	struct POSIX_SECURITY *pxdesc;
#endif /* POSIXACLS */
	struct SECURITY_DATA *psecurdata;
	char *newattr;
	int securindex;
	int mode;
	int level;
	int attrib;
	u32 attrsz;
	u32 partsz;
	uid_t uid;
	gid_t gid;

	if (opt_v || (cmd == CMD_BACKUP)) {
		printf("%s ",(isdir ? "Directory" : "File"));
		printname(stdout, fullname);
		printf("\n");
	}

       /* get individual parameters, as when trying to get them */
       /* all, and one (typically SACL) is missing, we get none */
       /* and concatenate them, to be able to compute the checksum */

	partsz = 0;
	securindex = ntfs_get_file_security(ntfs_context,fullname,
				OWNER_SECURITY_INFORMATION,
				(char*)part,MAXATTRSZ,&partsz);

	attrib = ntfs_get_file_attributes(ntfs_context, fullname);
	if (attrib == INVALID_FILE_ATTRIBUTES) {
		printf("** Could not get file attrib\n");
		errors++;
	}
	if ((securindex < 0)
	    || (securindex >= MAXSECURID)
	    || ((securindex > 0)
		&& ((!opt_r && (cmd != CMD_BACKUP))
		   || !securdata[securindex >> SECBLKSZ]
		   || !securdata[securindex >> SECBLKSZ][securindex & ((1 << SECBLKSZ) - 1)].filecount)))
		{
		if (opt_v || (cmd == CMD_BACKUP)) {
			if ((securindex < -1) || (securindex >= MAXSECURID))
				printf("Security key : 0x%x out of range\n",securindex);
			else
				if (securindex == -1)
					printf("Security key : none\n");
				else
					printf("Security key : 0x%x\n",securindex);
		} else {
			printf("%s ",(isdir ? "Directory" : "File"));
			printname(stdout, fullname);
			if ((securindex < -1) || (securindex >= MAXSECURID))
				printf(" : key 0x%x out of range\n",securindex);
			else
				if (securindex == -1)
					printf(" : no key\n");
				else
					printf(" : key 0x%x\n",securindex);
		}

		attrsz = getfull(attr, fullname);
		if (attrsz) {
			psecurdata = (struct SECURITY_DATA*)NULL;
			if ((securindex < MAXSECURID) && (securindex > 0)) {
				if (!securdata[securindex >> SECBLKSZ])
					newblock(securindex);
				if (securdata[securindex >> SECBLKSZ])
					psecurdata = &securdata[securindex >> SECBLKSZ]
					   [securindex & ((1 << SECBLKSZ) - 1)];
			}
			if (((cmd == CMD_AUDIT) || (cmd == CMD_BACKUP))
			    && opt_v && psecurdata) {
				newattr = (char*)malloc(attrsz);
				printf("# %s ",(isdir ? "Directory" : "File"));
				printname(stdout, fullname);
				printf(" hash 0x%lx\n",
					(unsigned long)hash((le32*)attr,attrsz));
				if (newattr) {
					memcpy(newattr,attr,attrsz);
					psecurdata->attr = newattr;
				}
			}
			if ((opt_v || (cmd == CMD_BACKUP))
				&& ((securindex >= MAXSECURID)
				   || (securindex <= 0)
				   || !psecurdata
				   || (!psecurdata->filecount
					&& !psecurdata->flags))) {
				hexdump(attr,attrsz,8);
				printf("Computed hash : 0x%08lx\n",
					(unsigned long)hash((le32*)attr,attrsz));
			}
			if (ntfs_valid_descr((char*)attr,attrsz)) {
#if POSIXACLS
				pxdesc = linux_permissions_posix(attr,isdir);
				if (pxdesc)
					mode = pxdesc->mode;
				else
					mode = 0;
#else /* POSIXACLS */
				mode = linux_permissions(attr,isdir);
#endif /* POSIXACLS */
				attrib = ntfs_get_file_attributes(ntfs_context,fullname);
				if (opt_v >= 2) {
					level = (cmd == CMD_BACKUP ? 4 : 0);
					showheader(attr,level);
					showusid(attr,level);
					showgsid(attr,level);
					showdacl(attr,isdir,level);
					showsacl(attr,isdir,level);
				}
				if (attrib != INVALID_FILE_ATTRIBUTES)
					printf("Windows attrib : 0x%x\n",attrib);
				uid = linux_owner(attr);
				gid = linux_group(attr);
				if (cmd == CMD_BACKUP) {
				        showownership(attr);
					printf("# Interpreted Unix owner %d, group %d, mode 0%03o\n",
						(int)uid,(int)gid,mode);
				} else {
				        showownership(attr);
					printf("Interpreted Unix owner %d, group %d, mode 0%03o\n",
						(int)uid,(int)gid,mode);
				}
#if POSIXACLS
				if (pxdesc) {
					if ((cmd != CMD_BACKUP)
					    && (pxdesc->defcnt
					       || (pxdesc->tagsset
						   & (POSIX_ACL_USER
							| POSIX_ACL_GROUP
							| POSIX_ACL_MASK))))
						showposix(pxdesc);
					free(pxdesc);
				}
#endif /* POSIXACLS */
				if ((opt_r || (cmd == CMD_BACKUP))
				    && (securindex < MAXSECURID)
				    && (securindex > 0) && psecurdata) {
					psecurdata->filecount++;
					psecurdata->mode = mode;
				}
			} else {
				printf("** Descriptor fails sanity check\n");
				errors++;
			}
		}
	} else
		if (securindex > 0) {
			if (securdata[securindex >> SECBLKSZ]) {
				psecurdata = &securdata[securindex >> SECBLKSZ]
					[securindex & ((1 << SECBLKSZ) - 1)];
				psecurdata->filecount++;
				if ((cmd == CMD_BACKUP) || opt_r) {
					if ((cmd != CMD_BACKUP) && !opt_v) {
						printf("%s ",(isdir ? "Directory" : "File"));
						printname(stdout,fullname);
						printf("\n");
					}
					printf("Security key : 0x%x mode %03o (already displayed)\n",
						securindex,psecurdata->mode);
					if (attrib != INVALID_FILE_ATTRIBUTES)
						printf("Windows attrib : 0x%x\n",attrib);
				} else {
					printf("%s ",(isdir ? "Directory" : "File"));
					printname(stdout,fullname);
					printf(" : key 0x%x\n",securindex);
				}
				if (((cmd == CMD_AUDIT) || (cmd == CMD_BACKUP))
				    && opt_v
				    && psecurdata
				    && psecurdata->attr) {
					printf("# %s ",(isdir ? "Directory" : "File"));
					printname(stdout,fullname);
					printf(" hash 0x%lx\n",
						(unsigned long)hash((le32*)psecurdata->attr,
							ntfs_attr_size(psecurdata->attr)));
				}
			}
		} else {
			if (!opt_v && (cmd != CMD_BACKUP)) {
				printf("%s ",(isdir ? "Directory" : "File"));
				printname(stdout, fullname);
			}
			printf("   (Failed)\n");
			printf("** Could not get security data of ");
			printname(stdout, fullname);
			printf(", partsz %d\n", partsz);
			printerror(stdout);
			errors++;
		}
}

static BOOL recurseshow(const char *path)
{
	struct CALLBACK dircontext;
	struct LINK *current;
	BOOL isdir;
	BOOL err;

	err = FALSE;
	dircontext.head = (struct LINK*)NULL;
	dircontext.dir = path;
	isdir = ntfs_read_directory(ntfs_context, path,
			callback, &dircontext);
	if (isdir) {
		showfull(path,TRUE);
		if (opt_v) {
			if (cmd == CMD_BACKUP)
				printf("#\n#\n");
			else
				printf("\n\n");
		}
		while (dircontext.head) {
			current = dircontext.head;
			if (recurseshow(current->name)) err = TRUE;
			dircontext.head = dircontext.head->next;
			free(current);
		}
	} else
		if (errno == ENOTDIR) {
			showfull(path,FALSE);
			if (opt_v) {
				if (cmd == CMD_BACKUP)
					printf("#\n#\n");
				else
					printf("\n\n");
			}
		} else {
			printf("** Could not access %s\n",path);
			printerror(stdout);
			errors++;
			err = TRUE;
		}
	return (!err);
}


static BOOL singleshow(const char *path)
{
	BOOL isdir;
	BOOL err;

	err = FALSE;
	isdir = ntfs_read_directory(ntfs_context, path,
			callback, (struct CALLBACK*)NULL);
	if (isdir || (errno == ENOTDIR))
		showfull(path,isdir);
	else {
		printf("** Could not access %s\n",path);
		printerror(stdout);
		errors++;
		err = TRUE;
	}
	return (err);
}

#ifndef HAVE_WINDOWS_H

#ifdef HAVE_SETXATTR

static ssize_t ntfs_getxattr(const char *path, const char *name, void *value, size_t size)
{
#if defined(__APPLE__) || defined(__DARWIN__)
    return getxattr(path, name, value, size, 0, 0);
#else /* defined(__APPLE__) || defined(__DARWIN__) */
    return getxattr(path, name, value, size);
#endif /* defined(__APPLE__) || defined(__DARWIN__) */
}

/*
 *		   Display all the parameters associated to a mounted file
 *
 *	(Unix only)
 */

static BOOL showmounted(const char *fullname)
{

	static char attr[MAXATTRSZ];
	struct stat st;
#if POSIXACLS
	struct POSIX_SECURITY *pxdesc;
#endif /* POSIXACLS */
	BOOL mapped;
	int attrsz;
	int mode;
	uid_t uid;
	gid_t gid;
	u32 attrib;
	int level;
	BOOL isdir;
	BOOL err;

	err = FALSE;
	if (!stat(fullname,&st)) {
		isdir = S_ISDIR(st.st_mode);
		printf("%s ",(isdir ? "Directory" : "File"));
		printname(stdout,fullname);
		printf("\n");

		attrsz = ntfs_getxattr(fullname,"system.ntfs_acl",attr,MAXATTRSZ);
		if (attrsz > 0) {
			if (opt_v) {
				hexdump(attr,attrsz,8);
				printf("Computed hash : 0x%08lx\n",
					(unsigned long)hash((le32*)attr,attrsz));
			}
			if (ntfs_getxattr(fullname,"system.ntfs_attrib",&attrib,4) != 4) {
				printf("** Could not get file attrib\n");
				errors++;
			} else
				printf("Windows attrib : 0x%x\n",(int)attrib);
			if (ntfs_valid_descr(attr,attrsz)) {
				mapped = !local_build_mapping(context.mapping,fullname);
#if POSIXACLS
				if (mapped) {
					pxdesc = linux_permissions_posix(attr,isdir);
					if (pxdesc)
						mode = pxdesc->mode;
					else
						mode = 0;
				} else {
					pxdesc = (struct POSIX_SECURITY*)NULL;
					mode = linux_permissions(attr,isdir);
					printf("No user mapping : "
						"cannot display the Posix ACL\n");
				}
#else /* POSIXACLS */
				mode = linux_permissions(attr,isdir);
#endif /* POSIXACLS */
				if (opt_v >= 2) {
					level = (cmd == CMD_BACKUP ? 4 : 0);
					showheader(attr,level);
					showusid(attr,level);
					showgsid(attr,level);
					showdacl(attr,isdir,level);
					showsacl(attr,isdir,level);
				}
			        showownership(attr);
				if (mapped) {
					uid = linux_owner(attr);
					gid = linux_group(attr);
					printf("Interpreted Unix owner %d, group %d, mode 0%03o\n",
						(int)uid,(int)gid,mode);
				} else {
					printf("Interpreted Unix mode 0%03o (owner and group are unmapped)\n",
						mode);
				}
#if POSIXACLS
				if (pxdesc) {
					if ((pxdesc->defcnt
						|| (pxdesc->tagsset
						    & (POSIX_ACL_USER
							| POSIX_ACL_GROUP
							| POSIX_ACL_MASK))))
						showposix(pxdesc);
					free(pxdesc);
				}
				if (mapped)
					ntfs_free_mapping(context.mapping);
#endif /* POSIXACLS */
			} else {
				printf("Descriptor fails sanity check\n");
				errors++;
			}
		} else {
			printf("** Could not get the NTFS ACL, check whether file is on NTFS\n");
			errors++;
		}
	} else {
		printf("%s not found\n",fullname);
		err = TRUE;
	}
	return (err);
}

static BOOL processmounted(const char *fullname)
{

	static char attr[MAXATTRSZ];
	struct stat st;
	int attrsz;
	BOOL err;

	err = FALSE;
	if (cmd != CMD_USERMAP)
		err = showmounted(fullname);
	else
	if (!stat(fullname,&st)) {
		attrsz = ntfs_getxattr(fullname,"system.ntfs_acl",attr,MAXATTRSZ);
		if (attrsz > 0) {
			if (opt_v) {
				hexdump(attr,attrsz,8);
				printf("Computed hash : 0x%08lx\n",
					(unsigned long)hash((le32*)attr,attrsz));
			}
			if (ntfs_valid_descr(attr,attrsz)) {
				err = proposal(fullname, attr);
			} else {
				printf("*** Descriptor fails sanity check\n");
				errors++;
			}
		} else {
			printf("** Could not get the NTFS ACL, check whether file is on NTFS\n");
			errors++;
		}
	} else {
		printf("%s not found\n",fullname);
		err = TRUE;
	}
	return (err);
}

#else /* HAVE_SETXATTR */

static BOOL processmounted(const char *fullname __attribute__((unused)))
{
	fprintf(stderr,"Not possible on this configuration,\n");
	fprintf(stderr,"you have to use an unmounted partition\n");
	return (TRUE);
}

#endif /* HAVE_SETXATTR */

#endif /* HAVE_WINDOWS_H */

#if POSIXACLS

static BOOL recurseset_posix(const char *path, const struct POSIX_SECURITY *pxdesc)
{
	struct CALLBACK dircontext;
	struct LINK *current;
	BOOL isdir;
	BOOL err;

	err = FALSE;
	dircontext.head = (struct LINK*)NULL;
	dircontext.dir = path;
	isdir = ntfs_read_directory(ntfs_context, path,
			callback, &dircontext);
	if (isdir) {
		err = !setfull_posix(path,pxdesc,TRUE);
		if (err) {
			printf("** Failed to update %s\n",path);
			printerror(stdout);
			errors++;
		} else {
			if (cmd == CMD_BACKUP)
				printf("#\n#\n");
			else
				printf("\n\n");
			while (dircontext.head) {
				current = dircontext.head;
				recurseset_posix(current->name,pxdesc);
				dircontext.head = dircontext.head->next;
				free(current);
			}
		}
	} else
		if (errno == ENOTDIR) {
			err = !setfull_posix(path,pxdesc,FALSE);
			if (err) {
				printf("** Failed to update %s\n",path);
				printerror(stdout);
				errors++;
			}
		} else {
			printf("** Could not access %s\n",path);
			printerror(stdout);
			errors++;
			err = TRUE;
		}
	return (!err);
}

#else /* POSIXACLS */

static BOOL recurseset(const char *path, int mode)
{
	struct CALLBACK dircontext;
	struct LINK *current;
	BOOL isdir;
	BOOL err;

	err = FALSE;
	dircontext.head = (struct LINK*)NULL;
	dircontext.dir = path;
	isdir = ntfs_read_directory(ntfs_context, path,
			callback, &dircontext);
	if (isdir) {
		setfull(path,mode,TRUE);
		if (cmd == CMD_BACKUP)
			printf("#\n#\n");
		else
			printf("\n\n");
		while (dircontext.head) {
			current = dircontext.head;
			recurseset(current->name,mode);
			dircontext.head = dircontext.head->next;
			free(current);
		}
	} else
		if (errno == ENOTDIR)
			setfull(path,mode,FALSE);
		else {
			printf("** Could not access %s\n",path);
			printerror(stdout);
			errors++;
			err = TRUE;
		}
	return (!err);
}

#endif /* POSIXACLS */

#if POSIXACLS

static BOOL singleset_posix(const char *path, const struct POSIX_SECURITY *pxdesc)
{
	BOOL isdir;
	BOOL err;

	err = FALSE;
	isdir = ntfs_read_directory(ntfs_context, path,
			callback, (struct CALLBACK*)NULL);
	if (isdir || (errno == ENOTDIR)) {
		err = !setfull_posix(path,pxdesc,isdir);
		if (err) {
			printf("** Failed to update %s\n",path);
			printerror(stdout);
			errors++;
		}
	} else {
		printf("** Could not access %s\n",path);
		printerror(stdout);
		errors++;
		err = TRUE;
	}
	return (!err);
}

#else /* POSIXACLS */

static BOOL singleset(const char *path, int mode)
{
	BOOL isdir;
	BOOL err;

	err = FALSE;
	isdir = ntfs_read_directory(ntfs_context, path,
			callback, (struct CALLBACK*)NULL);
	if (isdir || (errno == ENOTDIR))
		setfull(path,mode,isdir);
	else {
		printf("** Could not access %s\n",path);
		printerror(stdout);
		errors++;
		err = TRUE;
	}
	return (!err);
}

#endif /* POSIXACLS */

static int callback(void *ctx, const ntfschar *ntfsname,
	const int length, const int type,
	const s64 pos  __attribute__((unused)),
	const MFT_REF mft_ref __attribute__((unused)),
	const unsigned int dt_type __attribute__((unused)))
{
	struct LINK *linkage;
	struct CALLBACK *dircontext;
	char *name;
	int newlth;
	int size;

	dircontext = (struct CALLBACK*)ctx;
	size = utf8size(ntfsname,length);
	if (dircontext
	    && (type != 2)     /* 2 : dos name (8+3) */
	    && (size > 0)      /* chars convertible to utf8 */
	    && ((length > 2)
		|| (ntfsname[0] != const_cpu_to_le16('.'))
		|| ((length > 1)
		    && (ntfsname[1] != const_cpu_to_le16('.'))))) {
		linkage = (struct LINK*)malloc(sizeof(struct LINK)
				+ strlen(dircontext->dir)
				+ size + 2);
		if (linkage) {
		/* may find ".fuse_hidden*" files */
		/* recommendation is not to hide them, so that */
		/* the user has a clue to delete them */
			strcpy(linkage->name,dircontext->dir);
			if (linkage->name[strlen(linkage->name) - 1] != '/')
				strcat(linkage->name,"/");
			name = &linkage->name[strlen(linkage->name)];
			newlth = makeutf8(name,ntfsname,length);
			name[newlth] = 0;
			linkage->next = dircontext->head;
			dircontext->head = linkage;
		}
	}
	return (0);
}

/*
 *		 Backup security descriptors in a directory tree
 */

static BOOL backup(const char *volume, const char *root)
{
	BOOL err;
	int count;
	int i,j;
	time_t now;
	const char *txtime;

	now = time((time_t*)NULL);
	txtime = ctime(&now);
	if (!getuid() && open_security_api()) {
		if (open_volume(volume,NTFS_MNT_RDONLY)) {
			printf("#\n# Recursive ACL collection on %s#\n",txtime);
			err = recurseshow(root);
			count = 0;
			for (i=0; i<(MAXSECURID + (1 << SECBLKSZ) - 1)/(1 << SECBLKSZ); i++)
				if (securdata[i])
					for (j=0; j<(1 << SECBLKSZ); j++)
						if (securdata[i][j].filecount) {
							count++;
						}
			printf("# %d security keys\n",count);
			close_volume(volume);
		} else {
			fprintf(stderr,"Could not open volume %s\n",volume);
			printerror(stdout);
			err = TRUE;
		}
		close_security_api();
	} else {
		if (getuid())
			fprintf(stderr,"This is only possible as root\n");
		else
			fprintf(stderr,"Could not open security API\n");
		err = TRUE;
	}
	return (err);
}

/*
 *		 List security descriptors in a directory tree
 */

static BOOL listfiles(const char *volume, const char *root)
{
	BOOL err;
	int i,j;
	int count;

	if (!getuid() && open_security_api()) {
		if (open_volume(volume,NTFS_MNT_RDONLY)) {
			if (opt_r) {
				printf("\nRecursive file check\n");
				err = recurseshow(root);
				printf("Summary\n");
				count = 0;
				for (i=0; i<(MAXSECURID + (1 << SECBLKSZ) - 1)/(1 << SECBLKSZ); i++)
					if (securdata[i])
						for (j=0; j<(1 << SECBLKSZ); j++)
							if (securdata[i][j].filecount) {
								printf("Key 0x%x : %d files, mode 0%03o\n",
									i*(1 << SECBLKSZ)+j,securdata[i][j].filecount,
									securdata[i][j].mode);
								count++;
							}
				printf("%d security keys\n",count);
			} else
				err = singleshow(root);
			close_volume(volume);
		} else {
			err = TRUE;
		}
		close_security_api();
	} else {
		if (getuid())
			fprintf(stderr,"This is only possible as root\n");
		else
			fprintf(stderr,"Could not open security API\n");
		err = TRUE;
	}
	return (err);
}

#ifdef HAVE_WINDOWS_H

static BOOL mapproposal(const char *volume, const char *name)
{
	BOOL err;
	u32 attrsz;
	int securindex;
	char attr[256]; /* header (20) and a couple of SIDs (max 68 each) */

	err = FALSE;
	if (!getuid() && open_security_api()) {
		if (open_volume(volume,NTFS_MNT_RDONLY)) {

			attrsz = 0;
			securindex = ntfs_get_file_security(ntfs_context,name,
					OWNER_SECURITY_INFORMATION
					    | GROUP_SECURITY_INFORMATION,
					(char*)attr,MAXATTRSZ,&attrsz);
			if (securindex)
				err = proposal(name,attr);
			else {
				fprintf(stderr,"*** Could not get the ACL of ");
				printname(stderr, name);
				fprintf(stderr,"\n");
				printerror(stderr);
				errors++;
			}
			close_volume(volume);
		} else {
			fprintf(stderr,"Could not open volume %s\n",volume);
			printerror(stdout);
			err = TRUE;
		}
		close_security_api();
	} else {
		if (getuid())
			fprintf(stderr,"This is only possible as root\n");
		else
			fprintf(stderr,"Could not open security API\n");
		err = TRUE;
	}
	return (err);
}

#endif

/*
 *		Check whether a SDS entry is valid
 */

static BOOL valid_sds(const char *attr, unsigned int offset,
		unsigned int entrysz, unsigned int size, u32 prevkey,
		BOOL second)
{
	BOOL unsane;
	u32 comphash;
	u32 key;

	unsane = FALSE;
	if (!get4l(attr,0) && !get4l(attr,4)) {
		printf("Entry at 0x%lx was deleted\n",(long)offset);
	} else {
		if ((ntfs_attr_size(&attr[20]) + 20) > entrysz) {
			printf("** Entry is truncated (expected size %ld)\n",
				(long)ntfs_attr_size(&attr[20] + 20));
			unsane = TRUE;
			errors++;
		}
		if ((ntfs_attr_size(&attr[20]) + 20) < entrysz) {
			printf("** Extra data appended to entry (expected size %ld)\n",
				(long)ntfs_attr_size(&attr[20]) + 20);
			warnings++;
		}
		if (!unsane && !ntfs_valid_descr((const char*)&attr[20],size)) {
			printf("** General sanity check has failed\n");
			unsane = TRUE;
			errors++;
		}
		if (!unsane) {
			comphash = hash((const le32*)&attr[20],entrysz-20);
			if ((u32)get4l(attr,0) == comphash) {
				if (opt_v >= 2)
					printf("Hash	 0x%08lx (correct)\n",
						(unsigned long)comphash);
			} else {
				printf("** hash  0x%08lx (computed : 0x%08lx)\n",
					(unsigned long)get4l(attr,0),
					(unsigned long)comphash);
				unsane = TRUE;
				errors++;
			}
		}
		if (!unsane) {
			if ((second ? get8l(attr,8) + 0x40000 : get8l(attr,8)) == offset) {
				if (opt_v >= 2)
					printf("Offset	 0x%lx (correct)\n",(long)offset);
			} else {
				printf("** offset  0x%llx (expected : 0x%llx)\n",
					(long long)get8l(attr,8),
					(long long)(second ? get8l(attr,8) - 0x40000 : get8l(attr,8)));
//				unsane = TRUE;
				errors++;
			}
		}
		if (!unsane) {
			key = get4l(attr,4);
			if (opt_v >= 2)
				printf("Key	 0x%x\n",(int)key);
			if (key) {
				if (key <= prevkey) {
					printf("** Unordered key 0x%lx after 0x%lx\n",
						(long)key,(long)prevkey);
					unsane = TRUE;
					errors++;
				}
			}
		}
	}
	return (!unsane);
}

/*
 *		Check whether a SDS entry is consistent with other known data
 *	and store current data for subsequent checks
 */

static int consist_sds(const char *attr, unsigned int offset,
		unsigned int entrysz, BOOL second)
{
	int errcnt;
	u32 key;
	u32 comphash;
	struct SECURITY_DATA *psecurdata;

	errcnt = 0;
	key = get4l(attr,4);
	if ((key > 0) && (key < MAXSECURID)) {
		printf("Valid entry at 0x%lx for key 0x%lx\n",
			(long)offset,(long)key);
		if (!securdata[key >> SECBLKSZ])
			newblock(key);
		if (securdata[key >> SECBLKSZ]) {
			psecurdata = &securdata[key >> SECBLKSZ][key & ((1 << SECBLKSZ) - 1)];
			comphash = hash((const le32*)&attr[20],entrysz-20);
			if (psecurdata->flags & INSDS1) {
				if (psecurdata->hash != comphash) {
					printf("** Different hash values : $SDS-1 0x%08lx $SDS-2 0x%08lx\n",
						(unsigned long)psecurdata->hash,
						(unsigned long)comphash);
					errcnt++;
					errors++;
				}
				if (psecurdata->offset != get8l(attr,8)) {
					printf("** Different offsets : $SDS-1 0x%llx $SDS-2 0x%llx\n",
						(long long)psecurdata->offset,(long long)get8l(attr,8));
					errcnt++;
					errors++;
				}
				if (psecurdata->length != get4l(attr,16)) {
					printf("** Different lengths : $SDS-1 0x%lx $SDS-2 0x%lx\n",
						(long)psecurdata->length,(long)get4l(attr,16));
					errcnt++;
					errors++;
				}
			} else {
				if (second) {
					printf("** Entry was not present in $SDS-1\n");
					errcnt++;
					errors++;
				}
				psecurdata->hash = comphash;
				psecurdata->offset = get8l(attr,8);
				psecurdata->length = get4l(attr,16);
			}
			psecurdata->flags |= (second ? INSDS2 : INSDS1);
		}
	} else
		if (key || get4l(attr,0)) {
			printf("** Security_id 0x%x out of bounds\n",key);
			warnings++;
		}
	return (errcnt);
}


/*
 *		       Auditing of $SDS
 */

static int audit_sds(BOOL second)
{
	static char attr[MAXATTRSZ + 20];
	BOOL isdir;
	BOOL done;
	BOOL unsane;
	u32 prevkey;
	int errcnt;
	int size;
	unsigned int entrysz;
	unsigned int entryalsz;
	unsigned int offset;
	int count;
	int deleted;
	int mode;

	if (second)
		printf("\nAuditing $SDS-2\n");
	else
		printf("\nAuditing $SDS-1\n");
	errcnt = 0;
	offset = (second ? 0x40000 : 0);
	count = 0;
	deleted = 0;
	done = FALSE;
	prevkey = 0;

	  /* get size of first record */

	size = ntfs_read_sds(ntfs_context,(char*)attr,20,offset);
	if (size != 20) {
		if ((size < 0) && (errno == ENOTSUP))
			printf("** There is no $SDS-%d in this volume\n",
							(second ? 2 : 1));
		else {
			printf("** Could not open $SDS-%d, size %d\n",
							(second ? 2 : 1),size);
			errors++;
			errcnt++;
		}
	} else
		do {
			entrysz = get4l(attr,16);
			entryalsz = ((entrysz - 1) | 15) + 1;
			if (entryalsz <= (MAXATTRSZ + 20)) {
				/* read next header in anticipation, to get its size */
				size = ntfs_read_sds(ntfs_context,
					(char*)&attr[20],entryalsz,offset + 20);
				if (opt_v)
					printf("\nAt offset 0x%lx got %lu bytes\n",(long)offset,(long)size);
			} else {
				printf("** Security attribute is too long (%ld bytes) - stopping\n",
					(long)entryalsz);
				errcnt++;
			}
			if ((entryalsz > (MAXATTRSZ + 20)) || (size < (int)(entrysz - 20)))
				done = TRUE;
			else {
				if (opt_v) {
					printf("Entry size %d bytes\n",entrysz);
					hexdump(&attr[20],size,8);
				}

				unsane = !valid_sds(attr,offset,entrysz,
					size,prevkey,second);
				if (!unsane) {
					if (!get4l(attr,0) && !get4l(attr,4))
						deleted++;
					else
						count++;
					errcnt += consist_sds(attr,offset,
						entrysz, second);
					if (opt_v >= 2) {
						isdir = guess_dir(&attr[20]);
						printf("Assuming %s descriptor\n",(isdir ? "directory" : "file"));
						showheader(&attr[20],0);
						showusid(&attr[20],0);
						showgsid(&attr[20],0);
						showdacl(&attr[20],isdir,0);
						showsacl(&attr[20],isdir,0);
						showownership(&attr[20]);
						mode = linux_permissions(
						    &attr[20],isdir);
						printf("Interpreted Unix mode 0%03o\n",mode);
					}
					prevkey = get4l(attr,4);
				}
				if (!unsane) {
					memcpy(attr,&attr[entryalsz],20);
					offset += entryalsz;
					if (!get4l(attr,16)
					   || ((((offset - 1) | 0x3ffff) - offset + 1) < 20)) {
						if (second)
							offset = ((offset - 1) | 0x7ffff) + 0x40001;
						else
							offset = ((offset - 1) | 0x7ffff) + 1;
						if (opt_v)
							printf("Trying next SDS-%d block at offset 0x%lx\n",
								(second ? 2 : 1), (long)offset);
						size = ntfs_read_sds(ntfs_context,
							(char*)attr,20,offset);
						if (size != 20) {
							if (opt_v)
								printf("Assuming end of $SDS, got %d bytes\n",size);
							done = TRUE;
						}
					}
				} else {
					printf("** Sanity check failed - stopping there\n");
					errcnt++;
					errors++;
					done = TRUE;
				}
			}
		} while (!done);
	if (count || deleted || errcnt) {
		printf("%d valid and %d deleted entries in $SDS-%d\n",
				count,deleted,(second ? 2 : 1));
		printf("%d errors in $SDS-%c\n",errcnt,(second ? '2' : '1'));
	}
	return (errcnt);
}

/*
 *		Check whether a SII entry is sane
 */

static BOOL valid_sii(const char *entry, u32 prevkey)
{
	BOOL valid;
	u32 key;

	valid = TRUE;
	key = get4l(entry,16);
	if (key <= prevkey) {
		printf("** Unordered key 0x%lx after 0x%lx\n",
			(long)key,(long)prevkey);
		valid = FALSE;
		errors++;
	}
	prevkey = key;
	if (get2l(entry,0) != 20) {
		printf("** offset %d (instead of 20)\n",(int)get2l(entry,0));
		valid = FALSE;
		errors++;
	}
	if (get2l(entry,2) != 20) {
		printf("** size %d (instead of 20)\n",(int)get2l(entry,2));
		valid = FALSE;
		errors++;
	}
	if (get4l(entry,4) != 0) {
		printf("** fill1 %d (instead of 0)\n",(int)get4l(entry,4));
		valid = FALSE;
		errors++;
	}
	if (get2l(entry,12) & 1) {
		if (get2l(entry,8) != 48) {
			printf("** index size %d (instead of 48)\n",(int)get2l(entry,8));
			valid = FALSE;
			errors++;
		}
	} else
		if (get2l(entry,8) != 40) {
			printf("** index size %d (instead of 40)\n",(int)get2l(entry,8));
			valid = FALSE;
			errors++;
		}
	if (get2l(entry,10) != 4) {
		printf("** index key size %d (instead of 4)\n",(int)get2l(entry,10));
		valid = FALSE;
		errors++;
	}
	if ((get2l(entry,12) & ~3) != 0) {
		printf("** flags 0x%x (instead of < 4)\n",(int)get2l(entry,12));
		valid = FALSE;
		errors++;
	}
	if (get2l(entry,14) != 0) {
		printf("** fill2 %d (instead of 0)\n",(int)get2l(entry,14));
		valid = FALSE;
		errors++;
	}
	if (get4l(entry,24) != key) {
		printf("** key 0x%x (instead of 0x%x)\n",
						(int)get4l(entry,24),(int)key);
		valid = FALSE;
		errors++;
	}
	return (valid);
}

/*
 *		Check whether a SII entry is consistent with other known data
 */

static int consist_sii(const char *entry)
{
	int errcnt;
	u32 key;
	struct SECURITY_DATA *psecurdata;

	errcnt = 0;
	key = get4l(entry,16);
	if ((key > 0) && (key < MAXSECURID)) {
		printf("Valid entry for key 0x%lx\n",(long)key);
		if (!securdata[key >> SECBLKSZ])
			newblock(key);
		if (securdata[key >> SECBLKSZ]) {
			psecurdata = &securdata[key >> SECBLKSZ][key & ((1 << SECBLKSZ) - 1)];
			psecurdata->flags |= INSII;
			if (psecurdata->flags & (INSDS1 | INSDS2)) {
				if ((u32)get4l(entry,20) != psecurdata->hash) {
					printf("** hash 0x%x (instead of 0x%x)\n",
						(unsigned int)get4l(entry,20),
						(unsigned int)psecurdata->hash);
					errors++;
				}
				if (get8l(entry,28) != psecurdata->offset) {
					printf("** offset 0x%llx (instead of 0x%llx)\n",
						(long long)get8l(entry,28),
						(long long)psecurdata->offset);
					errors++;
				}
				if (get4l(entry,36) != psecurdata->length) {
					printf("** length 0x%lx (instead of %ld)\n",
						(long)get4l(entry,36),
						(long)psecurdata->length);
					errors++;
				}
			} else {
				printf("** Entry was not present in $SDS\n");
				errors++;
				psecurdata->hash = get4l(entry,20);
				psecurdata->offset = get8l(entry,28);
				psecurdata->length = get4l(entry,36);
				if (opt_v) {
					printf("   hash 0x%x\n",(unsigned int)psecurdata->hash);
					printf("   offset 0x%llx\n",(long long)psecurdata->offset);
					printf("   length %ld\n",(long)psecurdata->length);
				}
				errcnt++;
			}
		}
	} else {
		printf("** Security_id 0x%x out of bounds\n",key);
		warnings++;
	}
	return (errcnt);
}


/*
 *		       Auditing of $SII
 */

static int audit_sii(void)
{
	char *entry;
	int errcnt;
	u32 prevkey;
	BOOL valid;
	BOOL done;
	int count;

	printf("\nAuditing $SII\n");
	errcnt = 0;
	count = 0;
	entry = (char*)NULL;
	prevkey = 0;
	done = FALSE;
	do {
		entry = (char*)ntfs_read_sii(ntfs_context,(INDEX_ENTRY*)entry);
		if (entry) {
			valid = valid_sii(entry,prevkey);
			if (valid) {
				count++;
				errcnt += consist_sii(entry);
				prevkey = get4l(entry,16);
			} else
				errcnt++;
		} else
			if ((errno == ENOTSUP) && !prevkey)
				printf("** There is no $SII in this volume\n");
	} while (entry && !done);
	if (count || errcnt) {
		printf("%d valid entries in $SII\n",count);
		printf("%d errors in $SII\n",errcnt);
	}
	return (errcnt);
}

/*
 *		Check whether a SII entry is sane
 */

static BOOL valid_sdh(const char *entry, u32 prevkey, u32 prevhash)
{
	BOOL valid;
	u32 key;
	u32 currhash;

	valid = TRUE;
	currhash = get4l(entry,16);
	key = get4l(entry,20);
	if ((currhash < prevhash)
		|| ((currhash == prevhash) && (key <= prevkey))) {
		printf("** Unordered hash and key 0x%x 0x%x after 0x%x 0x%x\n",
			(unsigned int)currhash,(unsigned int)key,
			(unsigned int)prevhash,(unsigned int)prevkey);
		valid = FALSE;
		errors++;
	}
	if ((opt_v >= 2) && (currhash == prevhash))
		printf("Hash collision (not an error)\n");

	if (get2l(entry,0) != 24) {
		printf("** offset %d (instead of 24)\n",(int)get2l(entry,0));
		valid = FALSE;
		errors++;
	}
	if (get2l(entry,2) != 20) {
		printf("** size %d (instead of 20)\n",(int)get2l(entry,2));
		valid = FALSE;
		errors++;
	}
	if (get4l(entry,4) != 0) {
		printf("** fill1 %d (instead of 0)\n",(int)get4l(entry,4));
		valid = FALSE;
		errors++;
	}
	if (get2l(entry,12) & 1) {
		if (get2l(entry,8) != 56) {
			printf("** index size %d (instead of 56)\n",(int)get2l(entry,8));
			valid = FALSE;
			errors++;
		}
	} else
		if (get2l(entry,8) != 48) {
			printf("** index size %d (instead of 48)\n",(int)get2l(entry,8));
			valid = FALSE;
			errors++;
		}
	if (get2l(entry,10) != 8) {
		printf("** index key size %d (instead of 8)\n",(int)get2l(entry,10));
		valid = FALSE;
		errors++;
	}
	if ((get2l(entry,12) & ~3) != 0) {
		printf("** flags 0x%x (instead of < 4)\n",(int)get2l(entry,12));
		valid = FALSE;
		errors++;
	}
	if (get2l(entry,14) != 0) {
		printf("** fill2 %d (instead of 0)\n",(int)get2l(entry,14));
		valid = FALSE;
		errors++;
	}
	if ((u32)get4l(entry,24) != currhash) {
		printf("** hash 0x%x (instead of 0x%x)\n",
			(unsigned int)get4l(entry,24),(unsigned int)currhash);
		valid = FALSE;
		errors++;
	}
	if (get4l(entry,28) != key) {
		printf("** key 0x%x (instead of 0x%x)\n",
			(int)get4l(entry,28),(int)key);
		valid = FALSE;
		errors++;
	}
	if (get4l(entry,44)
		&& (get4l(entry,44) != 0x490049)) {
		printf("** fill3 0x%lx (instead of 0 or 0x490049)\n",
			(long)get4l(entry,44));
		valid = FALSE;
		errors++;
	}
	return (valid);
}

/*
 *		Check whether a SDH entry is consistent with other known data
 */

static int consist_sdh(const char *entry)
{
	int errcnt;
	u32 key;
	struct SECURITY_DATA *psecurdata;

	errcnt = 0;
	key = get4l(entry,20);
	if ((key > 0) && (key < MAXSECURID)) {
		printf("Valid entry for key 0x%lx\n",(long)key);
		if (!securdata[key >> SECBLKSZ])
			newblock(key);
		if (securdata[key >> SECBLKSZ]) {
			psecurdata = &securdata[key >> SECBLKSZ][key & ((1 << SECBLKSZ) - 1)];
			psecurdata->flags |= INSDH;
			if (psecurdata->flags & (INSDS1 | INSDS2 | INSII)) {
				if ((u32)get4l(entry,24) != psecurdata->hash) {
					printf("** hash 0x%x (instead of 0x%x)\n",
						(unsigned int)get4l(entry,24),
						(unsigned int)psecurdata->hash);
					errors++;
				}
				if (get8l(entry,32) != psecurdata->offset) {
					printf("** offset 0x%llx (instead of 0x%llx)\n",
						(long long)get8l(entry,32),
						(long long)psecurdata->offset);
					errors++;
				}
				if (get4l(entry,40) != psecurdata->length) {
					printf("** length %ld (instead of %ld)\n",
						(long)get4l(entry,40),
						(long)psecurdata->length);
					errors++;
				}
			} else {
				printf("** Entry was not present in $SDS nor in $SII\n");
				errors++;
				psecurdata->hash = get4l(entry,24);
				psecurdata->offset = get8l(entry,32);
				psecurdata->length = get4l(entry,40);
				if (opt_v) {
					printf("   offset 0x%llx\n",(long long)psecurdata->offset);
					printf("   length %ld\n",(long)psecurdata->length);
				}
				errcnt++;
			}
		}
	} else {
		printf("** Security_id 0x%x out of bounds\n",key);
		warnings++;
	}
	return (errcnt);
}

/*
 *		       Auditing of $SDH
 */

static int audit_sdh(void)
{
	char *entry;
	int errcnt;
	int count;
	u32 prevkey;
	u32 prevhash;
	BOOL valid;
	BOOL done;

	printf("\nAuditing $SDH\n");
	count = 0;
	errcnt = 0;
	prevkey = 0;
	prevhash = 0;
	entry = (char*)NULL;
	done = FALSE;
	do {
		entry = (char*)ntfs_read_sdh(ntfs_context,(INDEX_ENTRY*)entry);
		if (entry) {
			valid = valid_sdh(entry,prevkey,prevhash);
			if (valid) {
				count++;
				errcnt += consist_sdh(entry);
				prevhash = get4l(entry,16);
				prevkey = get4l(entry,20);
			} else
				errcnt++;
		} else
			if ((errno == ENOTSUP) && !prevkey)
				printf("** There is no $SDH in this volume\n");
	} while (entry && !done);
	if (count || errcnt) {
		printf("%d valid entries in $SDH\n",count);
		printf("%d errors in $SDH\n",errcnt);
	}
	return (errcnt);
}

/*
 *		Audit summary
 */

static void audit_summary(void)
{
	int count;
	int flags;
	int cnt;
	int found;
	int i,j;

	count = 0;
	found = 0;
	if (opt_r) printf("Summary of security key use :\n");
	for (i=0; i<(MAXSECURID + (1 << SECBLKSZ) - 1)/(1 << SECBLKSZ); i++)
		if (securdata[i])
			for (j=0; j<(1 << SECBLKSZ); j++) {
				flags = securdata[i][j].flags & (INSDS1 + INSDS2 + INSII + INSDH);
				if (flags) found++;
				if (flags
					&& (flags != (INSDS1 + INSDS2 + INSII + INSDH)))
					{
					if (!count && !opt_r)
						printf("\n** Keys not present in all files :\n");
					cnt = securdata[i][j].filecount;
					if (opt_r)
						printf("Key 0x%x used by %d %s, not in",
							i*(1 << SECBLKSZ)+j,cnt,
							(cnt > 1 ? "files" : "file"));
					else
						printf("Key 0x%x not in", i*(1 << SECBLKSZ)+j);
					if (!(flags & INSDS1))
						printf(" SDS-1");
					if (!(flags & INSDS2))
						printf(" SDS-2");
					if (!(flags & INSII))
						printf(" SII");
					if (!(flags & INSDH))
						printf(" SDH");
					printf("\n");
					count++;
				} else {
					cnt = securdata[i][j].filecount;
					if (opt_r && cnt)
						printf("Key 0x%x used by %d %s\n",
							i*(1 << SECBLKSZ)+j,cnt,
							(cnt > 1 ? "files" : "file"));
				}
			}
	if (found) {
		if (count)
			printf("%d keys not present in all lists\n",count);
		else
			printf("All keys are present in all lists\n");
	}
}

/*
 *		       Auditing
 */

static BOOL audit(const char *volume)
{
	BOOL err;

	err = FALSE;
	if (!getuid() && open_security_api()) {
		if (open_volume(volume,NTFS_MNT_RDONLY)) {
			if (audit_sds(FALSE)) err = TRUE;
			if (audit_sds(TRUE)) err = TRUE;
			if (audit_sii()) err = TRUE;
			if (audit_sdh()) err = TRUE;
			if (opt_r) recurseshow("/");

			audit_summary();
			close_volume(volume);
		}
		else {
			fprintf(stderr,"Could not open volume %s\n",volume);
			printerror(stdout);
			err = TRUE;
		}
		close_security_api();
	}
	else {
		if (getuid())
			fprintf(stderr,"This is only possible as root\n");
		else fprintf(stderr,"Could not open security API\n");
		err = TRUE;
	}
	return (err);
}

#if POSIXACLS

/*
 *		Encode a Posix ACL string
 *	[d:]{ugmo}:uid[:perms],...
 */

static struct POSIX_SECURITY *encode_posix_acl(const char *str)
{
	int acccnt;
	int defcnt;
	int i,k,l;
	int c;
	s32 id;
	u16 perms;
	u16 apermsset;
	u16 dpermsset;
	u16 tag;
	u16 tagsset;
	mode_t mode;
	BOOL defacl;
	BOOL dmask;
	BOOL amask;
	const char *p;
	struct POSIX_ACL *acl;
	struct POSIX_SECURITY *pxdesc;
	enum { PXBEGIN, PXTAG, PXTAG1, PXID, PXID1, PXID2,
		PXPERM, PXPERM1, PXPERM2, PXOCT, PXNEXT, PXEND, PXERR
	} state;

				/* raw evaluation of ACE count */
	p = str;
	amask = FALSE;
	dmask = FALSE;
	if (*p == 'd') {
		acccnt = 0;
		defcnt = 1;
	} else {
		if ((*p >= '0') && (*p <= '7'))
			acccnt = 0;
		else
			acccnt = 1;
		defcnt = 0;
	}
	while (*p)
		if (*p++ == ',') {
			if (*p == 'd') {
				defcnt++;
				if (p[1] && (p[2] == 'm'))
					dmask = TRUE;
			} else {
				acccnt++;
				if (*p == 'm')
					amask = TRUE;
			}
		}
		/* account for an implicit mask if none defined */
	if (acccnt && !amask)
		acccnt++;
	if (defcnt && !dmask)
		defcnt++;
	pxdesc = (struct POSIX_SECURITY*)malloc(sizeof(struct POSIX_SECURITY)
				+ (acccnt + defcnt)*sizeof(struct POSIX_ACE));
	if (pxdesc) {
		pxdesc->acccnt = acccnt;
		pxdesc->firstdef = acccnt;
		pxdesc->defcnt = defcnt;
		acl = &pxdesc->acl;
		p = str;
		state = PXBEGIN;
		id = 0;
		defacl = FALSE;
		mode = 0;
		apermsset = 0;
		dpermsset = 0;
		tag = 0;
		perms = 0;
		k = l = 0;
		c = *p++;
		while ((state != PXEND) && (state != PXERR)) {
			switch (state) {
			case PXBEGIN :
				if (c == 'd') {
					defacl = TRUE;
					state = PXTAG1;
					break;
				} else
					if ((c >= '0') && (c <= '7')) {
						mode = c - '0';
						state = PXOCT;
						break;
					}
				defacl = FALSE;
				/* fall through */
			case PXTAG :
				switch (c) {
				case 'u' :
					tag = POSIX_ACL_USER;
					state = PXID;
					break;
				case 'g' :
					tag = POSIX_ACL_GROUP;
					state = PXID;
					break;
				case 'o' :
					tag = POSIX_ACL_OTHER;
					state = PXID;
					break;
				case 'm' :
					tag = POSIX_ACL_MASK;
					state = PXID;
					break;
				default :
					state = PXERR;
					break;
				}
				break;
			case PXTAG1 :
				if (c == ':')
					state = PXTAG;
				else
					state = PXERR;
				break;
			case PXID :
				if (c == ':') {
					if ((tag == POSIX_ACL_OTHER)
					   || (tag == POSIX_ACL_MASK))
						state = PXPERM;
					else
						state = PXID1;
				} else
					state = PXERR;
				break;
			case PXID1 :
				if ((c >= '0') && (c <= '9')) {
					id = c - '0';
					state = PXID2;
				} else
					if (c == ':') {
						id = -1;
						if (tag == POSIX_ACL_USER)
							tag = POSIX_ACL_USER_OBJ;
						if (tag == POSIX_ACL_GROUP)
							tag = POSIX_ACL_GROUP_OBJ;
						state = PXPERM1;
					} else
						state = PXERR;
				break;
			case PXID2 :
				if ((c >= '0') && (c <= '9'))
					id = 10*id + c - '0';
				else
					if (c == ':')
						state = PXPERM1;
					else
						state = PXERR;
				break;
			case PXPERM :
				if (c == ':') {
					id = -1;
					state = PXPERM1;
				} else
					state = PXERR;
				break;
			case PXPERM1 :
				if ((c >= '0') && (c <= '7')) {
					perms = c - '0';
					state = PXNEXT;
					break;
				}
				state = PXPERM2;
				perms = 0;
				/* fall through */
			case PXPERM2 :
				switch (c) {
				case 'r' :
					perms |= POSIX_PERM_R;
					break;
				case 'w' :
					perms |= POSIX_PERM_W;
					break;
				case 'x' :
					perms |= POSIX_PERM_X;
					break;
				case ',' :
				case '\0' :
					if (defacl) {
						i = acccnt + l++;
						dpermsset |= perms;
					} else {
						i = k++;
						apermsset |= perms;
					}
					acl->ace[i].tag = tag;
					acl->ace[i].perms = perms;
					acl->ace[i].id = id;
					if (c == '\0')
						state = PXEND;
					else
						state = PXBEGIN;
					break;
				}
				break;
			case PXNEXT :
				if (!c || (c == ',')) {
					if (defacl) {
						i = acccnt + l++;
						dpermsset |= perms;
					} else {
						i = k++;
						apermsset |= perms;
					}
					acl->ace[i].tag = tag;
					acl->ace[i].perms = perms;
					acl->ace[i].id = id;
					if (c == '\0')
						state = PXEND;
					else
						state = PXBEGIN;
				} else
					state = PXERR;
				break;
			case PXOCT :
				if ((c >= '0') && (c <= '7'))
					mode = (mode << 3) + c - '0';
				else
					if (c == '\0')
						state = PXEND;
					else
						state = PXBEGIN;
				break;
			default :
				break;
			}
			c = *p++;
		}
			/* insert default mask if none defined */
		if (acccnt && !amask) {
			i = k++;
			acl->ace[i].tag = POSIX_ACL_MASK;
			acl->ace[i].perms = apermsset;
			acl->ace[i].id = -1;
		}
		if (defcnt && !dmask) {
			i = acccnt + l++;
			acl->ace[i].tag = POSIX_ACL_MASK;
			acl->ace[i].perms = dpermsset;
			acl->ace[i].id = -1;
		}
			/* compute the mode and tagsset */
		tagsset = 0;
		for (i=0; i<acccnt; i++) {
			tagsset |= acl->ace[i].tag;
			switch (acl->ace[i].tag) {
			case POSIX_ACL_USER_OBJ :
				mode |= acl->ace[i].perms << 6;
				break;
			case POSIX_ACL_GROUP_OBJ :
					/* unless mask seen first */
				if (!(tagsset & POSIX_ACL_MASK))
					mode |= acl->ace[i].perms << 3;
				break;
			case POSIX_ACL_OTHER :
				mode |= acl->ace[i].perms;
				break;
			case POSIX_ACL_MASK :
					/* overrides group */
				mode = (mode & 07707)
						| (acl->ace[i].perms << 3);
				break;
			default :
				break;
			}
		}
		pxdesc->mode = mode;
		pxdesc->tagsset = tagsset;
		pxdesc->acl.version = POSIX_VERSION;
		pxdesc->acl.flags = 0;
		pxdesc->acl.filler = 0;
		if (state != PXERR)
			ntfs_sort_posix(pxdesc);
showposix(pxdesc);
		if ((state == PXERR)
		   || (k != acccnt)
		   || (l != defcnt)
                   || !ntfs_valid_posix(pxdesc)) {
			if (~pxdesc->tagsset
			    & (POSIX_ACL_USER_OBJ | POSIX_ACL_GROUP_OBJ | POSIX_ACL_OTHER))
				fprintf(stderr,"User, group or other permissions missing\n");
			else
				fprintf(stderr,"Bad ACL description\n");
			free(pxdesc);
			pxdesc = (struct POSIX_SECURITY*)NULL;
		} else
			if (opt_v >= 2) {
				printf("Interpreted input description :\n");
				showposix(pxdesc);
			}
	} else
		errno = ENOMEM;
	return (pxdesc);
}

#endif /* POSIXACLS */

static BOOL setperms(const char *volume, const char *perms, const char *base)
{
	const char *p;
	BOOL cmderr;
	int i;
#if POSIXACLS
	struct POSIX_SECURITY *pxdesc;
#else /* POSIXACLS */
	int mode;
#endif /* POSIXACLS */

	cmderr = FALSE;
	p = perms;
#if POSIXACLS
	pxdesc = encode_posix_acl(p);
	if (pxdesc) {
		if (!getuid() && open_security_api()) {
			if (open_volume(volume,NTFS_MNT_NONE)) {
				if (opt_r) {
					for (i=0; i<(MAXSECURID + (1 << SECBLKSZ) - 1)/(1 << SECBLKSZ); i++)
						securdata[i] = (struct SECURITY_DATA*)NULL;
					recurseset_posix(base,pxdesc);
				} else
					singleset_posix(base,pxdesc);
				close_volume(volume);
			} else {
				fprintf(stderr,"Could not open volume %s\n",volume);
				printerror(stderr);
				cmderr = TRUE;
			}
			close_security_api();
		} else {
			if (getuid())
				fprintf(stderr,"This is only possible as root\n");
			else
				fprintf(stderr,"Could not open security API\n");
			cmderr = TRUE;
		}
		free(pxdesc);
	} else
		cmderr = TRUE;
#else /* POSIXACLS */
	mode = 0;
	while ((*p >= '0') && (*p <= '7'))
		mode = (mode << 3) + (*p++) - '0';
	if (*p) {
		fprintf(stderr,"New mode should be given in octal\n");
		cmderr = TRUE;
	} else {
		if (!getuid() && open_security_api()) {
			if (open_volume(volume,NTFS_MNT_NONE)) {
				if (opt_r) {
					for (i=0; i<(MAXSECURID + (1 << SECBLKSZ) - 1)/(1 << SECBLKSZ); i++)
						securdata[i] = (struct SECURITY_DATA*)NULL;
					recurseset(base,mode);
				} else
					singleset(base,mode);
				close_volume(volume);
			} else {
				fprintf(stderr,"Could not open volume %s\n",volume);
				printerror(stderr);
				cmderr = TRUE;
			}
			close_security_api();
		} else {
			if (getuid())
				fprintf(stderr,"This is only possible as root\n");
			else
				fprintf(stderr,"Could not open security API\n");
			cmderr = TRUE;
		}
	}
#endif /* POSIXACLS */
	return (cmderr);
}

static void usage(void)
{
#ifdef HAVE_WINDOWS_H
	fprintf(stderr,"Usage:\n");
#ifdef SELFTESTS
	fprintf(stderr,"   ntfssecaudit -t\n");
	fprintf(stderr,"	run self-tests\n");
#endif /* SELFTESTS */
	fprintf(stderr,"   ntfssecaudit -h [file]\n");
	fprintf(stderr,"	display security descriptors within file\n");
	fprintf(stderr,"   ntfssecaudit -a[rv] volume\n");
	fprintf(stderr,"	audit the volume\n");
	fprintf(stderr,"   ntfssecaudit [-v] file\n");
	fprintf(stderr,"	display the security parameters of file\n");
	fprintf(stderr,"   ntfssecaudit -r[v] directory\n");
	fprintf(stderr,"	display the security parameters of files in directory\n");
	fprintf(stderr,"   ntfssecaudit -b[v] directory\n");
	fprintf(stderr,"        backup the security parameters of files in directory\n");
	fprintf(stderr,"   ntfssecaudit -s[ev] volume [backupfile]\n");
	fprintf(stderr,"        set the security parameters as indicated in backup file\n");
	fprintf(stderr,"        with -e also set extra parameters (Windows attrib)\n");
	fprintf(stderr,"   ntfssecaudit perms file\n");
	fprintf(stderr,"	set the security parameters of file to perms\n");
	fprintf(stderr,"   ntfssecaudit -r[v] perms directory\n");
	fprintf(stderr,"	set the security parameters of files in directory to perms\n");
	fprintf(stderr,"   ntfssecaudit -u file\n");
	fprintf(stderr,"	get a user mapping proposal applicable to file\n");
#if POSIXACLS
	fprintf(stderr,"   Notes: perms can be an octal mode or a Posix ACL description\n");
#else /* POSIXACLS */
	fprintf(stderr,"   Notes: perms is an octal mode\n");
#endif /* POSIXACLS */
	fprintf(stderr,"          volume is a drive letter and colon (eg. D:)\n");
	fprintf(stderr,"          -v is for verbose, -vv for very verbose\n");
#else /* HAVE_WINDOWS_H */
	fprintf(stderr,"Usage:\n");
#ifdef SELFTESTS
	fprintf(stderr,"   ntfssecaudit -t\n");
	fprintf(stderr,"	run self-tests\n");
#endif /* SELFTESTS */
	fprintf(stderr,"   ntfssecaudit -h [file]\n");
	fprintf(stderr,"	display security descriptors within file\n");
	fprintf(stderr,"   ntfssecaudit -a[rv] volume\n");
	fprintf(stderr,"	audit the volume\n");
	fprintf(stderr,"   ntfssecaudit [-v] volume file\n");
	fprintf(stderr,"	display the security parameters of file\n");
	fprintf(stderr,"   ntfssecaudit -r[v] volume directory\n");
	fprintf(stderr,"	display the security parameters of files in directory\n");
	fprintf(stderr,"   ntfssecaudit -b[v] volume directory\n");
	fprintf(stderr,"        backup the security parameters of files in directory\n");
	fprintf(stderr,"   ntfssecaudit -s[ev] volume [backupfile]\n");
	fprintf(stderr,"        set the security parameters as indicated in backup file\n");
	fprintf(stderr,"        with -e also set extra parameters (Windows attrib)\n");
	fprintf(stderr,"   ntfssecaudit volume perms file\n");
	fprintf(stderr,"	set the security parameters of file to perms\n");
	fprintf(stderr,"   ntfssecaudit -r[v] volume perms directory\n");
	fprintf(stderr,"	set the security parameters of files in directory to perms\n");
#ifdef HAVE_SETXATTR
	fprintf(stderr," special cases, do not require being root :\n");
	fprintf(stderr,"   ntfssecaudit -u mounted-file\n");
	fprintf(stderr,"	get a user mapping proposal applicable to mounted file\n");
	fprintf(stderr,"   ntfssecaudit [-v] mounted-file\n");
	fprintf(stderr,"	display the security parameters of a mounted file\n");
#endif /* HAVE_SETXATTR */
#if POSIXACLS
	fprintf(stderr,"   Notes: perms can be an octal mode or a Posix ACL description\n");
#else /* POSIXACLS */
	fprintf(stderr,"   Notes: perms is an octal mode\n");
#endif /* POSIXACLS */
#if defined(__sun) && defined (__SVR4)
	fprintf(stderr,"          volume is a partition designator (eg. /dev/dsk/c5t0d0p1)\n");
#else /* defined(__sun) && defined (__SVR4) */
	fprintf(stderr,"          volume is a partition designator (eg. /dev/sdb2)\n");
#endif /* defined(__sun) && defined (__SVR4) */
	fprintf(stderr,"          -v is for verbose, -vv for very verbose\n");
#endif /* HAVE_WINDOWS_H */
}

static void version(void)
{
	static const char *EXEC_NAME = "ntfssecaudit";

// confusing (see banner)
	printf("\n%s v%s (libntfs-3g) - Audit security data on a NTFS "
			"Volume.\n\n", EXEC_NAME, VERSION);
	printf("    Copyright (c) 2007-2016 Jean-Pierre Andre\n");
	printf("\n%s\n%s%s\n", ntfs_gpl, ntfs_bugs, ntfs_home);
}

#ifdef HAVE_WINDOWS_H

/*
 *		Split a Windows file designator into volume and fullpath
 */

static BOOL splitarg(char **split, const char *arg)
{
	char curdir[MAXFILENAME];
	BOOL err;
	BOOL withvol;
	BOOL withfullpath;
	int lthd;
	char *volume;
	char *filename;

	err = TRUE;
	withvol = arg[0] && (arg[1] == ':');
	if (withvol)
		withfullpath = (arg[2] == '/') || (arg[2] == '\\');
	else
		withfullpath = (arg[0] == '/') || (arg[0] == '\\');
	lthd = 0;
	if (!withvol || !withfullpath) {
		if (getcwd(curdir, sizeof(curdir)))
			lthd = strlen(curdir);
	}
	if (withvol && !withfullpath && arg[2]
	    && ((arg[0] ^ curdir[0]) & 0x3f)) {
		fprintf(stderr,"%c: is not the current drive,\n",arg[0]);
		fprintf(stderr,"please use the full path\n");
	} else {
		if (withvol && !withfullpath && !arg[2]
		    && ((arg[0] ^ curdir[0]) & 0x3f)) {
			curdir[2] = '\\';
			curdir[3] = 0;
			lthd = 3;
		}
		volume = (char*)malloc(4);
		if (volume) {
			if (withvol)
				volume[0] = arg[0];
			else
				volume[0] = curdir[0];
			volume[1] = ':';
			volume[2] = 0;
			filename = (char*)malloc(strlen(arg) + lthd + 2);
			if (filename) {
				if (withfullpath) {
					if (withvol)
						strcpy(filename, &arg[2]);
					else
						strcpy(filename, arg);
				} else {
					strcpy(filename, &curdir[2]);
					if (curdir[lthd - 1] != '\\')
						strcat(filename, "\\");
					if (withvol)
						strcat(filename, &arg[2]);
					else
						strcat(filename, arg);
				}
				if (!cleanpath(filename)) {
					split[0] = volume;
					split[1] = filename;
					err = FALSE;
				} else {
					fprintf(stderr,"Bad path %s\n", arg);
				}
			} else
				free(volume);
		}
	}
	return (err);
}

#endif /* HAVE_WINDOWS_H */

/*
 *		Parse the command-line options
 */

static int parse_options(int argc, char *argv[])
{
	static const char *sopt = "-abehHrstuvV";
	static const struct option lopt[] = {
		{ "audit",	 no_argument,		NULL, 'a' },
		{ "backup",	 no_argument,		NULL, 'b' },
		{ "extra",	 no_argument,		NULL, 'e' },
		{ "help",	 no_argument,		NULL, 'H' },
		{ "hexdecode",	 no_argument,		NULL, 'h' },
		{ "recurse",	 no_argument,		NULL, 'r' },
		{ "set",	 no_argument,		NULL, 's' },
		{ "test",	 no_argument,		NULL, 't' },
		{ "user-mapping",no_argument,		NULL, 'u' },
		{ "verbose",	 no_argument,		NULL, 'v' },
		{ "version",	 no_argument,		NULL, 'V' },
		{ NULL,		 0,			NULL,  0  }
	};

	int c = -1;
	int err = 0;
	int ver = 0;
	int help = 0;
	int xarg = 0;
	CMDS prevcmd;

	opterr = 0; /* We'll handle the errors, thank you. */

	opt_e = FALSE;
	opt_r = FALSE;
	opt_v = 0;
	cmd = CMD_NONE;
	prevcmd = CMD_NONE;

	while ((c = getopt_long(argc, argv, sopt, lopt, NULL)) != -1) {
		switch (c) {
		case 1:
			if (!xarg)
				xarg = optind - 1;
			break;
		case 'a':
			prevcmd = cmd;
			cmd = CMD_AUDIT;
			break;
		case 'b':
			prevcmd = cmd;
			cmd = CMD_BACKUP;
			break;
		case 'e':
			opt_e = TRUE;
			break;
		case 'h':
			prevcmd = cmd;
			cmd = CMD_HEX;
			break;
		case 'H':
			help++;
			break;
		case 'r':
			opt_r = TRUE;
			break;
		case 's':
			prevcmd = cmd;
			cmd = CMD_SET;
			break;
#ifdef SELFTESTS
		case 't':
			prevcmd = cmd;
			cmd = CMD_TEST;
			break;
#endif
		case 'u':
			prevcmd = cmd;
			cmd = CMD_USERMAP;
			break;
		case 'v':
			opt_v++;
			break;
		case 'V':
			ver++;
			break;
		default:
			if ((c < 'a') || (c > 'z'))
				fprintf(stderr,"Unhandled option case: %d.\n", c);
			else
				fprintf(stderr,"Invalid option -%c\n",c);
			err++;
			break;
		}
		if ((cmd != CMD_NONE)
		    && (prevcmd != CMD_NONE)
		    && (prevcmd != cmd)) {
			fprintf(stderr,"Incompatible commands\n");
			err++;
		}
	}

	if (!xarg)
		xarg = argc;

	if (help || err)
		cmd = CMD_HELP;
	else
		if (ver)
			cmd = CMD_VERSION;

	return (err ? 0 : xarg);
}

int main(int argc, char *argv[])
{
	char *split[2];
	char *uname;
	FILE *fd;
	int xarg;
	BOOL cmderr;
	BOOL fail;
	int i;

	printf("%s\n",BANNER);
	cmderr = FALSE;
	fail = FALSE;
	errors = 0;
	warnings = 0;
	split[0] = split[1] = (char*)NULL;
	uname = (char*)NULL;
	xarg = parse_options(argc,argv);
	if (xarg) {
		for (i=0; i<(MAXSECURID + (1 << SECBLKSZ) - 1)/(1 << SECBLKSZ); i++)
			securdata[i] = (struct SECURITY_DATA*)NULL;
#if POSIXACLS
		context.mapping[MAPUSERS] = (struct MAPPING*)NULL;
		context.mapping[MAPGROUPS] = (struct MAPPING*)NULL;
#endif /* POSIXACLS */
		mappingtype = MAPNONE;

		switch (cmd) {
		case CMD_AUDIT :
			if (xarg == (argc - 1))
				fail = audit(argv[xarg]);
			else
				cmderr = TRUE;
			break;
		case CMD_BACKUP :
			switch (argc - xarg) {
			case 1 :
#ifdef HAVE_WINDOWS_H
				if (!splitarg(split, argv[xarg]))
					fail = backup(split[0], split[1]);
				else
					cmderr = TRUE;
#else
				fail = backup(argv[xarg],"/");
#endif
				break;
			case 2 :
				cmderr = backup(argv[xarg],argv[xarg+1]);
				break;
			default :
				cmderr = TRUE;
				break;
			}
			break;
		case CMD_HEX :
			switch (argc - xarg) {
			case 0 :
				showhex(stdin);
				break;
			case 1 :
				fd = fopen(argv[xarg],"rb");
				if (fd) {
					showhex(fd);
					fclose(fd);
				} else {
					fprintf(stderr,"Could not open %s\n",
							argv[xarg]);
					cmderr = TRUE;
				}
				break;
			default :
				cmderr = TRUE;
				break;
			}
			break;
		case CMD_NONE :
			switch (argc - xarg) {
			case 1 :
#ifdef HAVE_WINDOWS_H
				if (!splitarg(split, argv[xarg]))
					fail = listfiles(split[0], split[1]);
				else
					cmderr = TRUE;
#else
				if (opt_r)
					cmderr = listfiles(argv[xarg],"/");
				else
					cmderr = processmounted(argv[xarg]);
#endif
				break;
			case 2 :
#ifdef HAVE_WINDOWS_H
				if (!splitarg(split, argv[xarg + 1]))
					fail = setperms(split[0], 
							argv[xarg], split[1]);
				else
					cmderr = TRUE;
#else /* HAVE_WINDOWS_H */
				fail = listfiles(argv[xarg],argv[xarg+1]);
#endif /* HAVE_WINDOWS_H */
				break;
			case 3 :
#ifdef HAVE_WINDOWS_H
				uname = unixname(argv[xarg+2]);
				if (uname)
					fail = setperms(argv[xarg],
							argv[xarg+1], uname);
				else
					cmderr = TRUE;
#else /* HAVE_WINDOWS_H */
				cmderr = setperms(argv[xarg], argv[xarg+1],
							argv[xarg+2]);
#endif
				break;
			default :
				cmderr = TRUE;
				break;
			}
			break;
		case CMD_SET :
			switch (argc - xarg) {
			case 1 :
				cmderr = dorestore(argv[xarg],stdin);
				break;
			case 2 :
				fd = fopen(argv[xarg+1],"rb");
				if (fd) {
					if (dorestore(argv[xarg],fd))
						cmderr = TRUE;
					fclose(fd);
				} else {
					fprintf(stderr,"Could not open %s\n",
								argv[xarg]);
					cmderr = TRUE;
				}
				break;
			default :
				cmderr = TRUE;
			}
			break;
#ifdef SELFTESTS
		case CMD_TEST :
			if (xarg != argc)
				cmderr = TRUE;
			else
				selftests();
			break;
#endif
		case CMD_USERMAP :
			switch (argc - xarg) {
			case 1 :
#ifdef HAVE_WINDOWS_H
				if (!splitarg(split, argv[xarg]))
					fail = mapproposal(split[0],
								split[1]);
				else
					cmderr = TRUE;
#else /* HAVE_WINDOWS_H */
				processmounted(argv[xarg]);
#endif /* HAVE_WINDOWS_H */
				break;
			case 2 :
#ifdef HAVE_WINDOWS_H
				uname = unixname(argv[xarg+1]);
				if (uname)
					cmderr = mapproposal(argv[xarg],
								uname);
				else
					cmderr = TRUE;
#else /* HAVE_WINDOWS_H */
				cmderr = TRUE;
#endif /* HAVE_WINDOWS_H */
				break;
			default :
				cmderr = TRUE;
			}
			break;
		case CMD_HELP :
		default :
			usage();
			break;
		case CMD_VERSION :
			version();
			break;
		}

		if (warnings)
			printf("** %u %s signalled\n",warnings,
				(warnings > 1 ? "warnings were"
						: "warning was"));
		if (errors)
			printf("** %u %s found\n",errors,
				(errors > 1 ? "errors were" : "error was"));
		else
			if (fail)
				printf("Command failed\n");
			else
				if (cmderr)
					usage();
				else
					printf("No errors were found\n");
		if (!isatty(1)) {
			fflush(stdout);
			if (warnings)
				fprintf(stderr,"** %u %s signalled\n",warnings,
					(warnings > 1 ? "warnings were"
							: "warning was"));
			if (errors)
				fprintf(stderr,"** %u %s found\n",errors,
					(errors > 1 ? "errors were"
							: "error was"));
			else
				fprintf(stderr,"No errors were found\n");
		}
		if (split[0])
			free(split[0]);
		if (split[1])
			free(split[1]);
		if (uname)
			free(uname);
		freeblocks();
	} else
		usage();
	if (cmderr || errors)
		exit(1);
	return (0);
}
