%/*
% * Copyright (c) 2009, Sun Microsystems, Inc.
% * All rights reserved.
% *
% * Redistribution and use in source and binary forms, with or without
% * modification, are permitted provided that the following conditions are met:
% * - Redistributions of source code must retain the above copyright notice,
% *   this list of conditions and the following disclaimer.
% * - Redistributions in binary form must reproduce the above copyright notice,
% *   this list of conditions and the following disclaimer in the documentation
% *   and/or other materials provided with the distribution.
% * - Neither the name of Sun Microsystems, Inc. nor the names of its
% *   contributors may be used to endorse or promote products derived
% *   from this software without specific prior written permission.
% *
% * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
% * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
% * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
% * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
% * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
% * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
% * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
% * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
% * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
% * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
% * POSSIBILITY OF SUCH DAMAGE.
% */

%/*
% * Copyright (c) 1985, 1990 by Sun Microsystems, Inc.
% */
%
%/* from @(#)mount.x	1.3 91/03/11 TIRPC 1.0 */

/*
 * Protocol description for the mount program
 */

#ifdef RPC_HDR
%#ifndef _rpcsvc_mount_h
%#define _rpcsvc_mount_h
%#include <memory.h>
#endif

const MNTPATHLEN = 1024;	/* maximum bytes in a pathname argument */
const MNTNAMLEN = 255;		/* maximum bytes in a name argument */
const FHSIZE = 32;		/* size in bytes of a file handle */

/*
 * The fhandle is the file handle that the server passes to the client.
 * All file operations are done using the file handles to refer to a file
 * or a directory. The file handle can contain whatever information the
 * server needs to distinguish an individual file.
 */
typedef opaque fhandle[FHSIZE];	

/*
 * If a status of zero is returned, the call completed successfully, and 
 * a file handle for the directory follows. A non-zero status indicates
 * some sort of error. The status corresponds with UNIX error numbers.
 */
union fhstatus switch (unsigned fhs_status) {
case 0:
	fhandle fhs_fhandle;
default:
	void;
};

/*
 * The type dirpath is the pathname of a directory
 */
typedef string dirpath<MNTPATHLEN>;

/*
 * The type name is used for arbitrary names (hostnames, groupnames)
 */
typedef string name<MNTNAMLEN>;

/*
 * A list of who has what mounted
 */
typedef struct mountbody *mountlist;
struct mountbody {
	name ml_hostname;
	dirpath ml_directory;
	mountlist ml_next;
};

/*
 * A list of netgroups
 */
typedef struct groupnode *groups;
struct groupnode {
	name gr_name;
	groups gr_next;
};

/*
 * A list of what is exported and to whom
 */
typedef struct exportnode *exports;
struct exportnode {
	dirpath ex_dir;
	groups ex_groups;
	exports ex_next;
};

/*
 * POSIX pathconf information
 */
struct ppathcnf {
	int	pc_link_max;	/* max links allowed */
	short	pc_max_canon;	/* max line len for a tty */
	short	pc_max_input;	/* input a tty can eat all at once */
	short	pc_name_max;	/* max file name length (dir entry) */
	short	pc_path_max;	/* max path name length (/x/y/x/.. ) */
	short	pc_pipe_buf;	/* size of a pipe (bytes) */
	u_char	pc_vdisable;	/* safe char to turn off c_cc[i] */
	char	pc_xxx;		/* alignment padding; cc_t == char */
	short	pc_mask[2];	/* validity and boolean bits */
};

/*
 * NFSv3 file handle
 */
const FHSIZE3 =	64;		/* max size of NFSv3 file handle in bytes */
typedef opaque		fhandle3<FHSIZE3>;

/*
 * NFSv3 mount status
 */
enum mountstat3 {
	MNT_OK			= 0,	/* no error */
	MNT3ERR_PERM		= 1,	/* not owner */
	MNT3ERR_NOENT		= 2,	/* no such file or directory */
	MNT3ERR_IO		= 5,	/* I/O error */
	MNT3ERR_ACCES		= 13,	/* Permission denied */
	MNT3ERR_NOTDIR		= 20,	/* Not a directory */
	MNT3ERR_INVAL		= 22,	/* Invalid argument */
	MNT3ERR_NAMETOOLONG	= 63,	/* File name too long */
	MNT3ERR_NOTSUPP		= 10004,/* Operation not supported */
	MNT3ERR_SERVERFAULT	= 10006	/* A failure on the server */
};

/*
 * NFSv3 mount result
 */
struct mountres3_ok {
	fhandle3	fhandle;
	int		auth_flavors<>;
};

union mountres3 switch (mountstat3 fhs_status) {
case MNT_OK:
	mountres3_ok	mountinfo; /* File handle and supported flavors */
default:
	void;
};

program MOUNTPROG {
	/*
	 * Version one of the mount protocol communicates with version two
	 * of the NFS protocol. The only connecting point is the fhandle 
	 * structure, which is the same for both protocols.
	 */
	version MOUNTVERS {
		/*
		 * Does no work. It is made available in all RPC services
		 * to allow server reponse testing and timing
		 */
		void
		MOUNTPROC_NULL(void) = 0;

		/*	
		 * If fhs_status is 0, then fhs_fhandle contains the
	 	 * file handle for the directory. This file handle may
		 * be used in the NFS protocol. This procedure also adds
		 * a new entry to the mount list for this client mounting
		 * the directory.
		 * Unix authentication required.
		 */
		fhstatus 
		MOUNTPROC_MNT(dirpath) = 1;

		/*
		 * Returns the list of remotely mounted filesystems. The 
		 * mountlist contains one entry for each hostname and 
		 * directory pair.
		 */
		mountlist
		MOUNTPROC_DUMP(void) = 2;

		/*
		 * Removes the mount list entry for the directory
		 * Unix authentication required.
		 */
		void
		MOUNTPROC_UMNT(dirpath) = 3;

		/*
		 * Removes all of the mount list entries for this client
		 * Unix authentication required.
		 */
		void
		MOUNTPROC_UMNTALL(void) = 4;

		/*
		 * Returns a list of all the exported filesystems, and which
		 * machines are allowed to import it.
		 */
		exports
		MOUNTPROC_EXPORT(void)  = 5;

		/*
		 * Identical to MOUNTPROC_EXPORT above
		 */
		exports
		MOUNTPROC_EXPORTALL(void) = 6;
	} = 1;

	/*
	 * Version two of the mount protocol communicates with version two
	 * of the NFS protocol.
	 * The only difference from version one is the addition of a POSIX
	 * pathconf call.
	 */
	version MOUNTVERS_POSIX {
		/*
		 * Does no work. It is made available in all RPC services
		 * to allow server reponse testing and timing
		 */
		void
		MOUNTPROC_NULL(void) = 0;

		/*	
		 * If fhs_status is 0, then fhs_fhandle contains the
	 	 * file handle for the directory. This file handle may
		 * be used in the NFS protocol. This procedure also adds
		 * a new entry to the mount list for this client mounting
		 * the directory.
		 * Unix authentication required.
		 */
		fhstatus 
		MOUNTPROC_MNT(dirpath) = 1;

		/*
		 * Returns the list of remotely mounted filesystems. The 
		 * mountlist contains one entry for each hostname and 
		 * directory pair.
		 */
		mountlist
		MOUNTPROC_DUMP(void) = 2;

		/*
		 * Removes the mount list entry for the directory
		 * Unix authentication required.
		 */
		void
		MOUNTPROC_UMNT(dirpath) = 3;

		/*
		 * Removes all of the mount list entries for this client
		 * Unix authentication required.
		 */
		void
		MOUNTPROC_UMNTALL(void) = 4;

		/*
		 * Returns a list of all the exported filesystems, and which
		 * machines are allowed to import it.
		 */
		exports
		MOUNTPROC_EXPORT(void)  = 5;

		/*
		 * Identical to MOUNTPROC_EXPORT above
		 */
		exports
		MOUNTPROC_EXPORTALL(void) = 6;

		/*
		 * POSIX pathconf info (Sun hack)
		 */
		ppathcnf
		MOUNTPROC_PATHCONF(dirpath) = 7;
	} = 2;

	/*
	 * Version 3 of the protocol is for NFSv3
	 */
	version MOUNTVERS_NFSV3 {
		/*
		 * Does no work. It is made available in all RPC services
		 * to allow server reponse testing and timing
		 */
		void
		MOUNTPROC3_NULL(void) = 0;

		/*	
		 * If fhs_status is 0, then fhs_fhandle contains the
	 	 * file handle for the directory. This file handle may
		 * be used in the NFS protocol. This procedure also adds
		 * a new entry to the mount list for this client mounting
		 * the directory.
		 * Unix authentication required.
		 */
		mountres3 
		MOUNTPROC3_MNT(dirpath) = 1;

		/*
		 * Returns the list of remotely mounted filesystems. The 
		 * mountlist contains one entry for each hostname and 
		 * directory pair.
		 */
		mountlist
		MOUNTPROC3_DUMP(void) = 2;

		/*
		 * Removes the mount list entry for the directory
		 * Unix authentication required.
		 */
		void
		MOUNTPROC3_UMNT(dirpath) = 3;

		/*
		 * Removes all of the mount list entries for this client
		 * Unix authentication required.
		 */
		void
		MOUNTPROC3_UMNTALL(void) = 4;

		/*
		 * Returns a list of all the exported filesystems, and which
		 * machines are allowed to import it.
		 */
		exports
		MOUNTPROC3_EXPORT(void)  = 5;
	} = 3;
} = 100005;

#ifdef RPC_HDR
%#endif /*!_rpcsvc_mount_h*/
#endif
