/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Masami Komiya <mkomiya@sonare.it> 2004
 */

#ifndef __NFS_H__
#define __NFS_H__

#define SUNRPC_PORT     111

#define PROG_PORTMAP    100000
#define PROG_NFS        100003
#define PROG_MOUNT      100005

#define MSG_CALL        0
#define MSG_REPLY       1

#define PORTMAP_GETPORT 3

#define MOUNT_ADDENTRY  1
#define MOUNT_UMOUNTALL 4

#define NFS_LOOKUP      4
#define NFS_READLINK    5
#define NFS_READ        6

#define NFS3PROC_LOOKUP 3

#define NFS_FHSIZE      32
#define NFS3_FHSIZE     64

#define NFSERR_PERM     1
#define NFSERR_NOENT    2
#define NFSERR_ACCES    13
#define NFSERR_ISDIR    21
#define NFSERR_INVAL    22

/*
 * Block size used for NFS read accesses.  A RPC reply packet (including  all
 * headers) must fit within a single Ethernet frame to avoid fragmentation.
 * However, if CONFIG_IP_DEFRAG is set, a bigger value could be used.  In any
 * case, most NFS servers are optimized for a power of 2.
 */
#define NFS_READ_SIZE	1024	/* biggest power of two that fits Ether frame */
#define NFS_MAX_ATTRS	26

/* Values for Accept State flag on RPC answers (See: rfc1831) */
enum rpc_accept_stat {
	NFS_RPC_SUCCESS = 0,	/* RPC executed successfully */
	NFS_RPC_PROG_UNAVAIL = 1,	/* remote hasn't exported program */
	NFS_RPC_PROG_MISMATCH = 2,	/* remote can't support version # */
	NFS_RPC_PROC_UNAVAIL = 3,	/* program can't support procedure */
	NFS_RPC_GARBAGE_ARGS = 4,	/* procedure can't decode params */
	NFS_RPC_SYSTEM_ERR = 5	/* errors like memory allocation failure */
};

struct rpc_t {
	union {
		uint8_t data[NFS_READ_SIZE + (6 + NFS_MAX_ATTRS) *
			sizeof(uint32_t)];
		struct {
			uint32_t id;
			uint32_t type;
			uint32_t rpcvers;
			uint32_t prog;
			uint32_t vers;
			uint32_t proc;
			uint32_t data[1];
		} call;
		struct {
			uint32_t id;
			uint32_t type;
			uint32_t rstatus;
			uint32_t verifier;
			uint32_t v2;
			uint32_t astatus;
			uint32_t data[NFS_READ_SIZE / sizeof(uint32_t) +
				NFS_MAX_ATTRS];
		} reply;
	} u;
} __attribute__((packed));
void nfs_start(void);	/* Begin NFS */


/**********************************************************************/

#endif /* __NFS_H__ */
