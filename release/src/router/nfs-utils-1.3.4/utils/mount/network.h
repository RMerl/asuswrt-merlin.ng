/*
 * network.h -- Provide common network functions for NFS mount/umount
 *
 * Copyright (C) 2007 Oracle.  All rights reserved.
 * Copyright (C) 2007 Chuck Lever <chuck.lever@oracle.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 0211-1301 USA
 *
 */

#ifndef _NFS_UTILS_MOUNT_NETWORK_H
#define _NFS_UTILS_MOUNT_NETWORK_H

#include <rpc/pmap_prot.h>

#define MNT_SENDBUFSIZE (2048U)
#define MNT_RECVBUFSIZE (1024U)

typedef struct {
	char **hostname;
	struct sockaddr_in saddr;
	struct pmap pmap;
} clnt_addr_t;

/* RPC call timeout values */
static const struct timeval TIMEOUT = { 20, 0 };
static const struct timeval RETRY_TIMEOUT = { 3, 0 };

int probe_bothports(clnt_addr_t *, clnt_addr_t *);
int nfs_probe_bothports(const struct sockaddr *, const socklen_t,
			struct pmap *, const struct sockaddr *,
			const socklen_t, struct pmap *, int);
int nfs_gethostbyname(const char *, struct sockaddr_in *);
int nfs_lookup(const char *hostname, const sa_family_t family,
		struct sockaddr *sap, socklen_t *salen);
int nfs_string_to_sockaddr(const char *, struct sockaddr *, socklen_t *);
int nfs_present_sockaddr(const struct sockaddr *,
			 const socklen_t, char *, const size_t);
int nfs_callback_address(const struct sockaddr *, const socklen_t,
		struct sockaddr *, socklen_t *);
int clnt_ping(struct sockaddr_in *, const unsigned long,
		const unsigned long, const unsigned int,
		struct sockaddr_in *);

struct mount_options;

enum {
	V_DEFAULT = 0,
	V_GENERAL,
	V_SPECIFIC,
	V_PARSE_ERR,
};

struct nfs_version {
	unsigned long major;
	unsigned long minor;
	int v_mode;
};

int nfs_nfs_proto_family(struct mount_options *options, sa_family_t *family);
int nfs_mount_proto_family(struct mount_options *options, sa_family_t *family);
int nfs_nfs_version(struct mount_options *options, struct nfs_version *version);
int  nfs_nfs_protocol(struct mount_options *options, unsigned long *protocol);

int nfs_options2pmap(struct mount_options *,
		      struct pmap *, struct pmap *);

int start_statd(void);

unsigned long nfsvers_to_mnt(const unsigned long);

int nfs_call_umount(clnt_addr_t *, dirpath *);
int nfs_advise_umount(const struct sockaddr *, const socklen_t,
		      const struct pmap *, const dirpath *);
CLIENT *mnt_openclnt(clnt_addr_t *, int *);
void mnt_closeclnt(CLIENT *, int);

int nfs_umount_do_umnt(struct mount_options *options,
		       char **hostname, char **dirname);

#endif	/* _NFS_UTILS_MOUNT_NETWORK_H */
