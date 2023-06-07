/*
 * ss.c		"sockstat", socket statistics
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/sysmacros.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fnmatch.h>
#include <getopt.h>
#include <stdbool.h>
#include <limits.h>
#include <stdarg.h>

#include "ss_util.h"
#include "utils.h"
#include "rt_names.h"
#include "ll_map.h"
#include "libnetlink.h"
#include "namespace.h"
#include "version.h"
#include "rt_names.h"
#include "cg_map.h"

#include <linux/tcp.h>
#include <linux/unix_diag.h>
#include <linux/netdevice.h>	/* for MAX_ADDR_LEN */
#include <linux/filter.h>
#include <linux/xdp_diag.h>
#include <linux/packet_diag.h>
#include <linux/netlink_diag.h>
#include <linux/sctp.h>
#include <linux/vm_sockets_diag.h>
#include <linux/net.h>
#include <linux/tipc.h>
#include <linux/tipc_netlink.h>
#include <linux/tipc_sockets_diag.h>
#include <linux/tls.h>
#include <linux/mptcp.h>

/* AF_VSOCK/PF_VSOCK is only provided since glibc 2.18 */
#ifndef PF_VSOCK
#define PF_VSOCK 40
#endif
#ifndef AF_VSOCK
#define AF_VSOCK PF_VSOCK
#endif

#ifndef IPPROTO_MPTCP
#define IPPROTO_MPTCP 262
#endif

#define BUF_CHUNK (1024 * 1024)	/* Buffer chunk allocation size */
#define BUF_CHUNKS_MAX 5	/* Maximum number of allocated buffer chunks */
#define LEN_ALIGN(x) (((x) + 1) & ~1)

#if HAVE_SELINUX
#include <selinux/selinux.h>
#else
/* Stubs for SELinux functions */
static int is_selinux_enabled(void)
{
	return -1;
}

static int getpidcon(pid_t pid, char **context)
{
	*context = NULL;
	return -1;
}

static int getfilecon(char *path, char **context)
{
	*context = NULL;
	return -1;
}

static int security_get_initial_context(char *name,  char **context)
{
	*context = NULL;
	return -1;
}
#endif

int preferred_family = AF_UNSPEC;
static int show_options;
int show_details;
static int show_users;
static int show_mem;
static int show_tcpinfo;
static int show_bpf;
static int show_proc_ctx;
static int show_sock_ctx;
static int show_header = 1;
static int follow_events;
static int sctp_ino;
static int show_tipcinfo;
static int show_tos;
static int show_cgroup;
static int show_inet_sockopt;
int oneline;

enum col_id {
	COL_NETID,
	COL_STATE,
	COL_RECVQ,
	COL_SENDQ,
	COL_ADDR,
	COL_SERV,
	COL_RADDR,
	COL_RSERV,
	COL_EXT,
	COL_PROC,
	COL_MAX
};

enum col_align {
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT
};

struct column {
	const enum col_align align;
	const char *header;
	const char *ldelim;
	int disabled;
	int width;	/* Calculated, including additional layout spacing */
	int max_len;	/* Measured maximum field length in this column */
};

static struct column columns[] = {
	{ ALIGN_LEFT,	"Netid",		"",	0, 0, 0 },
	{ ALIGN_LEFT,	"State",		" ",	0, 0, 0 },
	{ ALIGN_LEFT,	"Recv-Q",		" ",	0, 0, 0 },
	{ ALIGN_LEFT,	"Send-Q",		" ",	0, 0, 0 },
	{ ALIGN_RIGHT,	"Local Address:",	" ",	0, 0, 0 },
	{ ALIGN_LEFT,	"Port",			"",	0, 0, 0 },
	{ ALIGN_RIGHT,	"Peer Address:",	" ",	0, 0, 0 },
	{ ALIGN_LEFT,	"Port",			"",	0, 0, 0 },
	{ ALIGN_LEFT,	"Process",		"",	0, 0, 0 },
	{ ALIGN_LEFT,	"",			"",	0, 0, 0 },
};

static struct column *current_field = columns;

/* Output buffer: chained chunks of BUF_CHUNK bytes. Each field is written to
 * the buffer as a variable size token. A token consists of a 16 bits length
 * field, followed by a string which is not NULL-terminated.
 *
 * A new chunk is allocated and linked when the current chunk doesn't have
 * enough room to store the current token as a whole.
 */
struct buf_chunk {
	struct buf_chunk *next;	/* Next chained chunk */
	char *end;		/* Current end of content */
	char data[0];
};

struct buf_token {
	uint16_t len;		/* Data length, excluding length descriptor */
	char data[0];
};

static struct {
	struct buf_token *cur;	/* Position of current token in chunk */
	struct buf_chunk *head;	/* First chunk */
	struct buf_chunk *tail;	/* Current chunk */
	int chunks;		/* Number of allocated chunks */
} buffer;

static const char *TCP_PROTO = "tcp";
static const char *SCTP_PROTO = "sctp";
static const char *UDP_PROTO = "udp";
static const char *RAW_PROTO = "raw";
static const char *dg_proto;

enum {
	TCP_DB,
	MPTCP_DB,
	DCCP_DB,
	UDP_DB,
	RAW_DB,
	UNIX_DG_DB,
	UNIX_ST_DB,
	UNIX_SQ_DB,
	PACKET_DG_DB,
	PACKET_R_DB,
	NETLINK_DB,
	SCTP_DB,
	VSOCK_ST_DB,
	VSOCK_DG_DB,
	TIPC_DB,
	XDP_DB,
	MAX_DB
};

#define PACKET_DBM ((1<<PACKET_DG_DB)|(1<<PACKET_R_DB))
#define UNIX_DBM ((1<<UNIX_DG_DB)|(1<<UNIX_ST_DB)|(1<<UNIX_SQ_DB))
#define ALL_DB ((1<<MAX_DB)-1)
#define INET_L4_DBM ((1<<TCP_DB)|(1<<MPTCP_DB)|(1<<UDP_DB)|(1<<DCCP_DB)|(1<<SCTP_DB))
#define INET_DBM (INET_L4_DBM | (1<<RAW_DB))
#define VSOCK_DBM ((1<<VSOCK_ST_DB)|(1<<VSOCK_DG_DB))

enum {
	SS_UNKNOWN,
	SS_ESTABLISHED,
	SS_SYN_SENT,
	SS_SYN_RECV,
	SS_FIN_WAIT1,
	SS_FIN_WAIT2,
	SS_TIME_WAIT,
	SS_CLOSE,
	SS_CLOSE_WAIT,
	SS_LAST_ACK,
	SS_LISTEN,
	SS_CLOSING,
	SS_MAX
};

enum {
	SCTP_STATE_CLOSED		= 0,
	SCTP_STATE_COOKIE_WAIT		= 1,
	SCTP_STATE_COOKIE_ECHOED	= 2,
	SCTP_STATE_ESTABLISHED		= 3,
	SCTP_STATE_SHUTDOWN_PENDING	= 4,
	SCTP_STATE_SHUTDOWN_SENT	= 5,
	SCTP_STATE_SHUTDOWN_RECEIVED	= 6,
	SCTP_STATE_SHUTDOWN_ACK_SENT	= 7,
};

#define SS_ALL ((1 << SS_MAX) - 1)
#define SS_CONN (SS_ALL & ~((1<<SS_LISTEN)|(1<<SS_CLOSE)|(1<<SS_TIME_WAIT)|(1<<SS_SYN_RECV)))
#define TIPC_SS_CONN ((1<<SS_ESTABLISHED)|(1<<SS_LISTEN)|(1<<SS_CLOSE))

#include "ssfilter.h"

struct filter {
	int dbs;
	int states;
	uint64_t families;
	struct ssfilter *f;
	bool kill;
	struct rtnl_handle *rth_for_killing;
};

#define FAMILY_MASK(family) ((uint64_t)1 << (family))

static const struct filter default_dbs[MAX_DB] = {
	[TCP_DB] = {
		.states   = SS_CONN,
		.families = FAMILY_MASK(AF_INET) | FAMILY_MASK(AF_INET6),
	},
	[MPTCP_DB] = {
		.states   = SS_CONN,
		.families = FAMILY_MASK(AF_INET) | FAMILY_MASK(AF_INET6),
	},
	[DCCP_DB] = {
		.states   = SS_CONN,
		.families = FAMILY_MASK(AF_INET) | FAMILY_MASK(AF_INET6),
	},
	[UDP_DB] = {
		.states   = (1 << SS_ESTABLISHED),
		.families = FAMILY_MASK(AF_INET) | FAMILY_MASK(AF_INET6),
	},
	[RAW_DB] = {
		.states   = (1 << SS_ESTABLISHED),
		.families = FAMILY_MASK(AF_INET) | FAMILY_MASK(AF_INET6),
	},
	[UNIX_DG_DB] = {
		.states   = (1 << SS_CLOSE),
		.families = FAMILY_MASK(AF_UNIX),
	},
	[UNIX_ST_DB] = {
		.states   = SS_CONN,
		.families = FAMILY_MASK(AF_UNIX),
	},
	[UNIX_SQ_DB] = {
		.states   = SS_CONN,
		.families = FAMILY_MASK(AF_UNIX),
	},
	[PACKET_DG_DB] = {
		.states   = (1 << SS_CLOSE),
		.families = FAMILY_MASK(AF_PACKET),
	},
	[PACKET_R_DB] = {
		.states   = (1 << SS_CLOSE),
		.families = FAMILY_MASK(AF_PACKET),
	},
	[NETLINK_DB] = {
		.states   = (1 << SS_CLOSE),
		.families = FAMILY_MASK(AF_NETLINK),
	},
	[SCTP_DB] = {
		.states   = SS_CONN,
		.families = FAMILY_MASK(AF_INET) | FAMILY_MASK(AF_INET6),
	},
	[VSOCK_ST_DB] = {
		.states   = SS_CONN,
		.families = FAMILY_MASK(AF_VSOCK),
	},
	[VSOCK_DG_DB] = {
		.states   = SS_CONN,
		.families = FAMILY_MASK(AF_VSOCK),
	},
	[TIPC_DB] = {
		.states   = TIPC_SS_CONN,
		.families = FAMILY_MASK(AF_TIPC),
	},
	[XDP_DB] = {
		.states   = (1 << SS_CLOSE),
		.families = FAMILY_MASK(AF_XDP),
	},
};

static const struct filter default_afs[AF_MAX] = {
	[AF_INET] = {
		.dbs    = INET_DBM,
		.states = SS_CONN,
	},
	[AF_INET6] = {
		.dbs    = INET_DBM,
		.states = SS_CONN,
	},
	[AF_UNIX] = {
		.dbs    = UNIX_DBM,
		.states = SS_CONN,
	},
	[AF_PACKET] = {
		.dbs    = PACKET_DBM,
		.states = (1 << SS_CLOSE),
	},
	[AF_NETLINK] = {
		.dbs    = (1 << NETLINK_DB),
		.states = (1 << SS_CLOSE),
	},
	[AF_VSOCK] = {
		.dbs    = VSOCK_DBM,
		.states = SS_CONN,
	},
	[AF_TIPC] = {
		.dbs    = (1 << TIPC_DB),
		.states = TIPC_SS_CONN,
	},
	[AF_XDP] = {
		.dbs    = (1 << XDP_DB),
		.states = (1 << SS_CLOSE),
	},
};

static int do_default = 1;
static struct filter current_filter;

static void filter_db_set(struct filter *f, int db, bool enable)
{
	if (enable) {
		f->states   |= default_dbs[db].states;
		f->dbs	    |= 1 << db;
	} else {
		f->dbs &= ~(1 << db);
	}
	do_default   = 0;
}

static int filter_db_parse(struct filter *f, const char *s)
{
	const struct {
		const char *name;
		int dbs[MAX_DB + 1];
	} db_name_tbl[] = {
#define ENTRY(name, ...) { #name, { __VA_ARGS__, MAX_DB } }
		ENTRY(all, UDP_DB, DCCP_DB, TCP_DB, MPTCP_DB, RAW_DB,
			   UNIX_ST_DB, UNIX_DG_DB, UNIX_SQ_DB,
			   PACKET_R_DB, PACKET_DG_DB, NETLINK_DB,
			   SCTP_DB, VSOCK_ST_DB, VSOCK_DG_DB, XDP_DB),
		ENTRY(inet, UDP_DB, DCCP_DB, TCP_DB, MPTCP_DB, SCTP_DB, RAW_DB),
		ENTRY(udp, UDP_DB),
		ENTRY(dccp, DCCP_DB),
		ENTRY(tcp, TCP_DB),
		ENTRY(mptcp, MPTCP_DB),
		ENTRY(sctp, SCTP_DB),
		ENTRY(raw, RAW_DB),
		ENTRY(unix, UNIX_ST_DB, UNIX_DG_DB, UNIX_SQ_DB),
		ENTRY(unix_stream, UNIX_ST_DB),
		ENTRY(u_str, UNIX_ST_DB),	/* alias for unix_stream */
		ENTRY(unix_dgram, UNIX_DG_DB),
		ENTRY(u_dgr, UNIX_DG_DB),	/* alias for unix_dgram */
		ENTRY(unix_seqpacket, UNIX_SQ_DB),
		ENTRY(u_seq, UNIX_SQ_DB),	/* alias for unix_seqpacket */
		ENTRY(packet, PACKET_R_DB, PACKET_DG_DB),
		ENTRY(packet_raw, PACKET_R_DB),
		ENTRY(p_raw, PACKET_R_DB),	/* alias for packet_raw */
		ENTRY(packet_dgram, PACKET_DG_DB),
		ENTRY(p_dgr, PACKET_DG_DB),	/* alias for packet_dgram */
		ENTRY(netlink, NETLINK_DB),
		ENTRY(vsock, VSOCK_ST_DB, VSOCK_DG_DB),
		ENTRY(vsock_stream, VSOCK_ST_DB),
		ENTRY(v_str, VSOCK_ST_DB),	/* alias for vsock_stream */
		ENTRY(vsock_dgram, VSOCK_DG_DB),
		ENTRY(v_dgr, VSOCK_DG_DB),	/* alias for vsock_dgram */
		ENTRY(xdp, XDP_DB),
#undef ENTRY
	};
	bool enable = true;
	unsigned int i;
	const int *dbp;

	if (s[0] == '!') {
		enable = false;
		s++;
	}
	for (i = 0; i < ARRAY_SIZE(db_name_tbl); i++) {
		if (strcmp(s, db_name_tbl[i].name))
			continue;
		for (dbp = db_name_tbl[i].dbs; *dbp != MAX_DB; dbp++)
			filter_db_set(f, *dbp, enable);
		return 0;
	}
	return -1;
}

static void filter_af_set(struct filter *f, int af)
{
	f->states	   |= default_afs[af].states;
	f->families	   |= FAMILY_MASK(af);
	do_default	    = 0;
	preferred_family    = af;
}

static int filter_af_get(struct filter *f, int af)
{
	return !!(f->families & FAMILY_MASK(af));
}

static void filter_states_set(struct filter *f, int states)
{
	if (states)
		f->states = states;
}

static void filter_merge_defaults(struct filter *f)
{
	int db;
	int af;

	for (db = 0; db < MAX_DB; db++) {
		if (!(f->dbs & (1 << db)))
			continue;

		if (!(default_dbs[db].families & f->families))
			f->families |= default_dbs[db].families;
	}
	for (af = 0; af < AF_MAX; af++) {
		if (!(f->families & FAMILY_MASK(af)))
			continue;

		if (!(default_afs[af].dbs & f->dbs))
			f->dbs |= default_afs[af].dbs;
	}
}

static FILE *generic_proc_open(const char *env, const char *name)
{
	const char *p = getenv(env);
	char store[128];

	if (!p) {
		p = getenv("PROC_ROOT") ? : "/proc";
		snprintf(store, sizeof(store)-1, "%s/%s", p, name);
		p = store;
	}

	return fopen(p, "r");
}
#define net_tcp_open()		generic_proc_open("PROC_NET_TCP", "net/tcp")
#define net_tcp6_open()		generic_proc_open("PROC_NET_TCP6", "net/tcp6")
#define net_udp_open()		generic_proc_open("PROC_NET_UDP", "net/udp")
#define net_udp6_open()		generic_proc_open("PROC_NET_UDP6", "net/udp6")
#define net_raw_open()		generic_proc_open("PROC_NET_RAW", "net/raw")
#define net_raw6_open()		generic_proc_open("PROC_NET_RAW6", "net/raw6")
#define net_unix_open()		generic_proc_open("PROC_NET_UNIX", "net/unix")
#define net_packet_open()	generic_proc_open("PROC_NET_PACKET", \
							"net/packet")
#define net_netlink_open()	generic_proc_open("PROC_NET_NETLINK", \
							"net/netlink")
#define net_sockstat_open()	generic_proc_open("PROC_NET_SOCKSTAT", \
							"net/sockstat")
#define net_sockstat6_open()	generic_proc_open("PROC_NET_SOCKSTAT6", \
							"net/sockstat6")
#define net_snmp_open()		generic_proc_open("PROC_NET_SNMP", "net/snmp")
#define ephemeral_ports_open()	generic_proc_open("PROC_IP_LOCAL_PORT_RANGE", \
					"sys/net/ipv4/ip_local_port_range")

struct user_ent {
	struct user_ent	*next;
	unsigned int	ino;
	int		pid;
	int		fd;
	char		*process;
	char		*process_ctx;
	char		*socket_ctx;
};

#define USER_ENT_HASH_SIZE	256
static struct user_ent *user_ent_hash[USER_ENT_HASH_SIZE];

static int user_ent_hashfn(unsigned int ino)
{
	int val = (ino >> 24) ^ (ino >> 16) ^ (ino >> 8) ^ ino;

	return val & (USER_ENT_HASH_SIZE - 1);
}

static void user_ent_add(unsigned int ino, char *process,
					int pid, int fd,
					char *proc_ctx,
					char *sock_ctx)
{
	struct user_ent *p, **pp;

	p = malloc(sizeof(struct user_ent));
	if (!p) {
		fprintf(stderr, "ss: failed to malloc buffer\n");
		abort();
	}
	p->next = NULL;
	p->ino = ino;
	p->pid = pid;
	p->fd = fd;
	p->process = strdup(process);
	p->process_ctx = strdup(proc_ctx);
	p->socket_ctx = strdup(sock_ctx);

	pp = &user_ent_hash[user_ent_hashfn(ino)];
	p->next = *pp;
	*pp = p;
}

static void user_ent_destroy(void)
{
	struct user_ent *p, *p_next;
	int cnt = 0;

	while (cnt != USER_ENT_HASH_SIZE) {
		p = user_ent_hash[cnt];
		while (p) {
			free(p->process);
			free(p->process_ctx);
			free(p->socket_ctx);
			p_next = p->next;
			free(p);
			p = p_next;
		}
		cnt++;
	}
}

static void user_ent_hash_build(void)
{
	const char *root = getenv("PROC_ROOT") ? : "/proc/";
	struct dirent *d;
	char name[1024];
	int nameoff;
	DIR *dir;
	char *pid_context;
	char *sock_context;
	const char *no_ctx = "unavailable";
	static int user_ent_hash_build_init;

	/* If show_users & show_proc_ctx set only do this once */
	if (user_ent_hash_build_init != 0)
		return;

	user_ent_hash_build_init = 1;

	strlcpy(name, root, sizeof(name));

	if (strlen(name) == 0 || name[strlen(name)-1] != '/')
		strcat(name, "/");

	nameoff = strlen(name);

	dir = opendir(name);
	if (!dir)
		return;

	while ((d = readdir(dir)) != NULL) {
		struct dirent *d1;
		char process[16];
		char *p;
		int pid, pos;
		DIR *dir1;
		char crap;

		if (sscanf(d->d_name, "%d%c", &pid, &crap) != 1)
			continue;

		if (getpidcon(pid, &pid_context) != 0)
			pid_context = strdup(no_ctx);

		snprintf(name + nameoff, sizeof(name) - nameoff, "%d/fd/", pid);
		pos = strlen(name);
		if ((dir1 = opendir(name)) == NULL) {
			free(pid_context);
			continue;
		}

		process[0] = '\0';
		p = process;

		while ((d1 = readdir(dir1)) != NULL) {
			const char *pattern = "socket:[";
			unsigned int ino;
			char lnk[64];
			int fd;
			ssize_t link_len;
			char tmp[1024];

			if (sscanf(d1->d_name, "%d%c", &fd, &crap) != 1)
				continue;

			snprintf(name+pos, sizeof(name) - pos, "%d", fd);

			link_len = readlink(name, lnk, sizeof(lnk)-1);
			if (link_len == -1)
				continue;
			lnk[link_len] = '\0';

			if (strncmp(lnk, pattern, strlen(pattern)))
				continue;

			sscanf(lnk, "socket:[%u]", &ino);

			snprintf(tmp, sizeof(tmp), "%s/%d/fd/%s",
					root, pid, d1->d_name);

			if (getfilecon(tmp, &sock_context) <= 0)
				sock_context = strdup(no_ctx);

			if (*p == '\0') {
				FILE *fp;

				snprintf(tmp, sizeof(tmp), "%s/%d/stat",
					root, pid);
				if ((fp = fopen(tmp, "r")) != NULL) {
					if (fscanf(fp, "%*d (%[^)])", p) < 1)
						; /* ignore */
					fclose(fp);
				}
			}
			user_ent_add(ino, p, pid, fd,
					pid_context, sock_context);
			free(sock_context);
		}
		free(pid_context);
		closedir(dir1);
	}
	closedir(dir);
}

enum entry_types {
	USERS,
	PROC_CTX,
	PROC_SOCK_CTX
};

#define ENTRY_BUF_SIZE 512
static int find_entry(unsigned int ino, char **buf, int type)
{
	struct user_ent *p;
	int cnt = 0;
	char *ptr;
	char *new_buf;
	int len, new_buf_len;
	int buf_used = 0;
	int buf_len = 0;

	if (!ino)
		return 0;

	p = user_ent_hash[user_ent_hashfn(ino)];
	ptr = *buf = NULL;
	while (p) {
		if (p->ino != ino)
			goto next;

		while (1) {
			ptr = *buf + buf_used;
			switch (type) {
			case USERS:
				len = snprintf(ptr, buf_len - buf_used,
					"(\"%s\",pid=%d,fd=%d),",
					p->process, p->pid, p->fd);
				break;
			case PROC_CTX:
				len = snprintf(ptr, buf_len - buf_used,
					"(\"%s\",pid=%d,proc_ctx=%s,fd=%d),",
					p->process, p->pid,
					p->process_ctx, p->fd);
				break;
			case PROC_SOCK_CTX:
				len = snprintf(ptr, buf_len - buf_used,
					"(\"%s\",pid=%d,proc_ctx=%s,fd=%d,sock_ctx=%s),",
					p->process, p->pid,
					p->process_ctx, p->fd,
					p->socket_ctx);
				break;
			default:
				fprintf(stderr, "ss: invalid type: %d\n", type);
				abort();
			}

			if (len < 0 || len >= buf_len - buf_used) {
				new_buf_len = buf_len + ENTRY_BUF_SIZE;
				new_buf = realloc(*buf, new_buf_len);
				if (!new_buf) {
					fprintf(stderr, "ss: failed to malloc buffer\n");
					abort();
				}
				*buf = new_buf;
				buf_len = new_buf_len;
				continue;
			} else {
				buf_used += len;
				break;
			}
		}
		cnt++;
next:
		p = p->next;
	}
	if (buf_used) {
		ptr = *buf + buf_used;
		ptr[-1] = '\0';
	}
	return cnt;
}

static unsigned long long cookie_sk_get(const uint32_t *cookie)
{
	return (((unsigned long long)cookie[1] << 31) << 1) | cookie[0];
}

static const char *sctp_sstate_name[] = {
	[SCTP_STATE_CLOSED] = "CLOSED",
	[SCTP_STATE_COOKIE_WAIT] = "COOKIE_WAIT",
	[SCTP_STATE_COOKIE_ECHOED] = "COOKIE_ECHOED",
	[SCTP_STATE_ESTABLISHED] = "ESTAB",
	[SCTP_STATE_SHUTDOWN_PENDING] = "SHUTDOWN_PENDING",
	[SCTP_STATE_SHUTDOWN_SENT] = "SHUTDOWN_SENT",
	[SCTP_STATE_SHUTDOWN_RECEIVED] = "SHUTDOWN_RECEIVED",
	[SCTP_STATE_SHUTDOWN_ACK_SENT] = "ACK_SENT",
};

static const char * const stype_nameg[] = {
	"UNKNOWN",
	[SOCK_STREAM] = "STREAM",
	[SOCK_DGRAM] = "DGRAM",
	[SOCK_RDM] = "RDM",
	[SOCK_SEQPACKET] = "SEQPACKET",
};

struct sockstat {
	struct sockstat	   *next;
	unsigned int	    type;
	uint16_t	    prot;
	uint16_t	    raw_prot;
	inet_prefix	    local;
	inet_prefix	    remote;
	int		    lport;
	int		    rport;
	int		    state;
	int		    rq, wq;
	unsigned int ino;
	unsigned int uid;
	int		    refcnt;
	unsigned int	    iface;
	unsigned long long  sk;
	char *name;
	char *peer_name;
	__u32		    mark;
	__u64		    cgroup_id;
};

struct dctcpstat {
	unsigned int	ce_state;
	unsigned int	alpha;
	unsigned int	ab_ecn;
	unsigned int	ab_tot;
	bool		enabled;
};

struct tcpstat {
	struct sockstat	    ss;
	unsigned int	    timer;
	unsigned int	    timeout;
	int		    probes;
	char		    cong_alg[16];
	double		    rto, ato, rtt, rttvar;
	int		    qack, ssthresh, backoff;
	double		    send_bps;
	int		    snd_wscale;
	int		    rcv_wscale;
	int		    mss;
	int		    rcv_mss;
	int		    advmss;
	unsigned int	    pmtu;
	unsigned int	    cwnd;
	unsigned int	    lastsnd;
	unsigned int	    lastrcv;
	unsigned int	    lastack;
	double		    pacing_rate;
	double		    pacing_rate_max;
	double		    delivery_rate;
	unsigned long long  bytes_acked;
	unsigned long long  bytes_received;
	unsigned int	    segs_out;
	unsigned int	    segs_in;
	unsigned int	    data_segs_out;
	unsigned int	    data_segs_in;
	unsigned int	    unacked;
	unsigned int	    retrans;
	unsigned int	    retrans_total;
	unsigned int	    lost;
	unsigned int	    sacked;
	unsigned int	    fackets;
	unsigned int	    reordering;
	unsigned int	    not_sent;
	unsigned int	    delivered;
	unsigned int	    delivered_ce;
	unsigned int	    dsack_dups;
	unsigned int	    reord_seen;
	double		    rcv_rtt;
	double		    min_rtt;
	int		    rcv_space;
	unsigned int        rcv_ssthresh;
	unsigned long long  busy_time;
	unsigned long long  rwnd_limited;
	unsigned long long  sndbuf_limited;
	unsigned long long  bytes_sent;
	unsigned long long  bytes_retrans;
	bool		    has_ts_opt;
	bool		    has_sack_opt;
	bool		    has_ecn_opt;
	bool		    has_ecnseen_opt;
	bool		    has_fastopen_opt;
	bool		    has_wscale_opt;
	bool		    app_limited;
	struct dctcpstat    *dctcp;
	struct tcp_bbr_info *bbr_info;
};

/* SCTP assocs share the same inode number with their parent endpoint. So if we
 * have seen the inode number before, it must be an assoc instead of the next
 * endpoint. */
static bool is_sctp_assoc(struct sockstat *s, const char *sock_name)
{
	if (strcmp(sock_name, "sctp"))
		return false;
	if (!sctp_ino || sctp_ino != s->ino)
		return false;
	return true;
}

static const char *unix_netid_name(int type)
{
	switch (type) {
	case SOCK_STREAM:
		return "u_str";
	case SOCK_SEQPACKET:
		return "u_seq";
	case SOCK_DGRAM:
	default:
		return "u_dgr";
	}
}

static const char *proto_name(int protocol)
{
	switch (protocol) {
	case 0:
		return "raw";
	case IPPROTO_UDP:
		return "udp";
	case IPPROTO_TCP:
		return "tcp";
	case IPPROTO_MPTCP:
		return "mptcp";
	case IPPROTO_SCTP:
		return "sctp";
	case IPPROTO_DCCP:
		return "dccp";
	case IPPROTO_ICMPV6:
		return "icmp6";
	}

	return "???";
}

static const char *vsock_netid_name(int type)
{
	switch (type) {
	case SOCK_STREAM:
		return "v_str";
	case SOCK_DGRAM:
		return "v_dgr";
	default:
		return "???";
	}
}

static const char *tipc_netid_name(int type)
{
	switch (type) {
	case SOCK_STREAM:
		return "ti_st";
	case SOCK_DGRAM:
		return "ti_dg";
	case SOCK_RDM:
		return "ti_rd";
	case SOCK_SEQPACKET:
		return "ti_sq";
	default:
		return "???";
	}
}

/* Allocate and initialize a new buffer chunk */
static struct buf_chunk *buf_chunk_new(void)
{
	struct buf_chunk *new = malloc(BUF_CHUNK);

	if (!new)
		abort();

	new->next = NULL;

	/* This is also the last block */
	buffer.tail = new;

	/* Next token will be stored at the beginning of chunk data area, and
	 * its initial length is zero.
	 */
	buffer.cur = (struct buf_token *)new->data;
	buffer.cur->len = 0;

	new->end = buffer.cur->data;

	buffer.chunks++;

	return new;
}

/* Return available tail room in given chunk */
static int buf_chunk_avail(struct buf_chunk *chunk)
{
	return BUF_CHUNK - offsetof(struct buf_chunk, data) -
	       (chunk->end - chunk->data);
}

/* Update end pointer and token length, link new chunk if we hit the end of the
 * current one. Return -EAGAIN if we got a new chunk, caller has to print again.
 */
static int buf_update(int len)
{
	struct buf_chunk *chunk = buffer.tail;
	struct buf_token *t = buffer.cur;

	/* Claim success if new content fits in the current chunk, and anyway
	 * if this is the first token in the chunk: in the latter case,
	 * allocating a new chunk won't help, so we'll just cut the output.
	 */
	if ((len < buf_chunk_avail(chunk) && len != -1 /* glibc < 2.0.6 */) ||
	    t == (struct buf_token *)chunk->data) {
		len = min(len, buf_chunk_avail(chunk));

		/* Total field length can't exceed 2^16 bytes, cut as needed */
		len = min(len, USHRT_MAX - t->len);

		chunk->end += len;
		t->len += len;
		return 0;
	}

	/* Content truncated, time to allocate more */
	chunk->next = buf_chunk_new();

	/* Copy current token over to new chunk, including length descriptor */
	memcpy(chunk->next->data, t, sizeof(t->len) + t->len);
	chunk->next->end += t->len;

	/* Discard partially written field in old chunk */
	chunk->end -= t->len + sizeof(t->len);

	return -EAGAIN;
}

/* Append content to buffer as part of the current field */
__attribute__((format(printf, 1, 2)))
static void out(const char *fmt, ...)
{
	struct column *f = current_field;
	va_list args;
	char *pos;
	int len;

	if (f->disabled)
		return;

	if (!buffer.head)
		buffer.head = buf_chunk_new();

again:	/* Append to buffer: if we have a new chunk, print again */

	pos = buffer.cur->data + buffer.cur->len;
	va_start(args, fmt);

	/* Limit to tail room. If we hit the limit, buf_update() will tell us */
	len = vsnprintf(pos, buf_chunk_avail(buffer.tail), fmt, args);
	va_end(args);

	if (buf_update(len))
		goto again;
}

static int print_left_spacing(struct column *f, int stored, int printed)
{
	int s;

	if (!f->width || f->align == ALIGN_LEFT)
		return 0;

	s = f->width - stored - printed;
	if (f->align == ALIGN_CENTER)
		/* If count of total spacing is odd, shift right by one */
		s = (s + 1) / 2;

	if (s > 0)
		return printf("%*c", s, ' ');

	return 0;
}

static void print_right_spacing(struct column *f, int printed)
{
	int s;

	if (!f->width || f->align == ALIGN_RIGHT)
		return;

	s = f->width - printed;
	if (f->align == ALIGN_CENTER)
		s /= 2;

	if (s > 0)
		printf("%*c", s, ' ');
}

/* Done with field: update buffer pointer, start new token after current one */
static void field_flush(struct column *f)
{
	struct buf_chunk *chunk;
	unsigned int pad;

	if (f->disabled)
		return;

	chunk = buffer.tail;
	pad = buffer.cur->len % 2;

	if (buffer.cur->len > f->max_len)
		f->max_len = buffer.cur->len;

	/* We need a new chunk if we can't store the next length descriptor.
	 * Mind the gap between end of previous token and next aligned position
	 * for length descriptor.
	 */
	if (buf_chunk_avail(chunk) - pad < sizeof(buffer.cur->len)) {
		chunk->end += pad;
		chunk->next = buf_chunk_new();
		return;
	}

	buffer.cur = (struct buf_token *)(buffer.cur->data +
					  LEN_ALIGN(buffer.cur->len));
	buffer.cur->len = 0;
	buffer.tail->end = buffer.cur->data;
}

static int field_is_last(struct column *f)
{
	return f - columns == COL_MAX - 1;
}

/* Get the next available token in the buffer starting from the current token */
static struct buf_token *buf_token_next(struct buf_token *cur)
{
	struct buf_chunk *chunk = buffer.tail;

	/* If we reached the end of chunk contents, get token from next chunk */
	if (cur->data + LEN_ALIGN(cur->len) == chunk->end) {
		buffer.tail = chunk = chunk->next;
		return chunk ? (struct buf_token *)chunk->data : NULL;
	}

	return (struct buf_token *)(cur->data + LEN_ALIGN(cur->len));
}

/* Free up all allocated buffer chunks */
static void buf_free_all(void)
{
	struct buf_chunk *tmp;

	for (buffer.tail = buffer.head; buffer.tail; ) {
		tmp = buffer.tail;
		buffer.tail = buffer.tail->next;
		free(tmp);
	}
	buffer.head = NULL;
	buffer.chunks = 0;
}

/* Get current screen width, returns -1 if TIOCGWINSZ fails */
static int render_screen_width(void)
{
	int width = -1;

	if (isatty(STDOUT_FILENO)) {
		struct winsize w;

		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
			if (w.ws_col > 0)
				width = w.ws_col;
		}
	}

	return width;
}

/* Calculate column width from contents length. If columns don't fit on one
 * line, break them into the least possible amount of lines and keep them
 * aligned across lines. Available screen space is equally spread between fields
 * as additional spacing.
 */
static void render_calc_width(void)
{
	int screen_width, first, len = 0, linecols = 0;
	struct column *c, *eol = columns - 1;
	bool compact_output = false;

	screen_width = render_screen_width();
	if (screen_width == -1) {
		screen_width = INT_MAX;
		compact_output = true;
	}

	/* First pass: set width for each column to measured content length */
	for (first = 1, c = columns; c - columns < COL_MAX; c++) {
		if (c->disabled)
			continue;

		if (!first && c->max_len)
			c->width = c->max_len + strlen(c->ldelim);
		else
			c->width = c->max_len;

		/* But don't exceed screen size. If we exceed the screen size
		 * for even a single field, it will just start on a line of its
		 * own and then naturally wrap.
		 */
		c->width = min(c->width, screen_width);

		if (c->width)
			first = 0;
	}

	if (compact_output) {
		/* Compact output, skip extending columns. */
		return;
	}

	/* Second pass: find out newlines and distribute available spacing */
	for (c = columns; c - columns < COL_MAX; c++) {
		int pad, spacing, rem, last;
		struct column *tmp;

		if (!c->width)
			continue;

		linecols++;
		len += c->width;

		for (last = 1, tmp = c + 1; tmp - columns < COL_MAX; tmp++) {
			if (tmp->width) {
				last = 0;
				break;
			}
		}

		if (!last && len < screen_width) {
			/* Columns fit on screen so far, nothing to do yet */
			continue;
		}

		if (len == screen_width) {
			/* Exact fit, just start with new line */
			goto newline;
		}

		if (len > screen_width) {
			/* Screen width exceeded: go back one column */
			len -= c->width;
			c--;
			linecols--;
		}

		/* Distribute remaining space to columns on this line */
		pad = screen_width - len;
		spacing = pad / linecols;
		rem = pad % linecols;
		for (tmp = c; tmp > eol; tmp--) {
			if (!tmp->width)
				continue;

			tmp->width += spacing;
			if (rem) {
				tmp->width++;
				rem--;
			}
		}

newline:
		/* Line break: reset line counters, mark end-of-line */
		eol = c;
		len = 0;
		linecols = 0;
	}
}

/* Render buffered output with spacing and delimiters, then free up buffers */
static void render(void)
{
	struct buf_token *token;
	int printed, line_started = 0;
	struct column *f;

	if (!buffer.head)
		return;

	token = (struct buf_token *)buffer.head->data;

	/* Ensure end alignment of last token, it wasn't necessarily flushed */
	buffer.tail->end += buffer.cur->len % 2;

	render_calc_width();

	/* Rewind and replay */
	buffer.tail = buffer.head;

	f = columns;
	while (!f->width)
		f++;

	while (token) {
		/* Print left delimiter only if we already started a line */
		if (line_started++)
			printed = printf("%s", f->ldelim);
		else
			printed = 0;

		/* Print field content from token data with spacing */
		printed += print_left_spacing(f, token->len, printed);
		printed += fwrite(token->data, 1, token->len, stdout);
		print_right_spacing(f, printed);

		/* Go to next non-empty field, deal with end-of-line */
		do {
			if (field_is_last(f)) {
				printf("\n");
				f = columns;
				line_started = 0;
			} else {
				f++;
			}
		} while (f->disabled);

		token = buf_token_next(token);
	}
	/* Deal with final end-of-line when the last non-empty field printed
	 * is not the last field.
	 */
	if (line_started)
		printf("\n");

	buf_free_all();
	current_field = columns;
}

/* Move to next field, and render buffer if we reached the maximum number of
 * chunks, at the last field in a line.
 */
static void field_next(void)
{
	if (field_is_last(current_field) && buffer.chunks >= BUF_CHUNKS_MAX) {
		render();
		return;
	}

	field_flush(current_field);
	if (field_is_last(current_field))
		current_field = columns;
	else
		current_field++;
}

/* Walk through fields and flush them until we reach the desired one */
static void field_set(enum col_id id)
{
	while (id != current_field - columns)
		field_next();
}

/* Print header for all non-empty columns */
static void print_header(void)
{
	while (!field_is_last(current_field)) {
		if (!current_field->disabled)
			out("%s", current_field->header);
		field_next();
	}
}

static void sock_state_print(struct sockstat *s)
{
	const char *sock_name;
	static const char * const sstate_name[] = {
		"UNKNOWN",
		[SS_ESTABLISHED] = "ESTAB",
		[SS_SYN_SENT] = "SYN-SENT",
		[SS_SYN_RECV] = "SYN-RECV",
		[SS_FIN_WAIT1] = "FIN-WAIT-1",
		[SS_FIN_WAIT2] = "FIN-WAIT-2",
		[SS_TIME_WAIT] = "TIME-WAIT",
		[SS_CLOSE] = "UNCONN",
		[SS_CLOSE_WAIT] = "CLOSE-WAIT",
		[SS_LAST_ACK] = "LAST-ACK",
		[SS_LISTEN] =	"LISTEN",
		[SS_CLOSING] = "CLOSING",
	};

	switch (s->local.family) {
	case AF_UNIX:
		sock_name = unix_netid_name(s->type);
		break;
	case AF_INET:
	case AF_INET6:
		sock_name = proto_name(s->type);
		break;
	case AF_PACKET:
		sock_name = s->type == SOCK_RAW ? "p_raw" : "p_dgr";
		break;
	case AF_NETLINK:
		sock_name = "nl";
		break;
	case AF_TIPC:
		sock_name = tipc_netid_name(s->type);
		break;
	case AF_VSOCK:
		sock_name = vsock_netid_name(s->type);
		break;
	case AF_XDP:
		sock_name = "xdp";
		break;
	default:
		sock_name = "unknown";
	}

	if (is_sctp_assoc(s, sock_name)) {
		field_set(COL_STATE);		/* Empty Netid field */
		out("`- %s", sctp_sstate_name[s->state]);
	} else {
		field_set(COL_NETID);
		out("%s", sock_name);
		field_set(COL_STATE);
		out("%s", sstate_name[s->state]);
	}

	field_set(COL_RECVQ);
	out("%-6d", s->rq);
	field_set(COL_SENDQ);
	out("%-6d", s->wq);
	field_set(COL_ADDR);
}

static void sock_details_print(struct sockstat *s)
{
	if (s->uid)
		out(" uid:%u", s->uid);

	out(" ino:%u", s->ino);
	out(" sk:%llx", s->sk);

	if (s->mark)
		out(" fwmark:0x%x", s->mark);

	if (s->cgroup_id)
		out(" cgroup:%s", cg_id_to_path(s->cgroup_id));
}

static void sock_addr_print(const char *addr, char *delim, const char *port,
		const char *ifname)
{
	if (ifname)
		out("%s" "%%" "%s%s", addr, ifname, delim);
	else
		out("%s%s", addr, delim);

	field_next();
	out("%s", port);
	field_next();
}

static const char *print_ms_timer(unsigned int timeout)
{
	static char buf[64];
	int secs, msecs, minutes;

	secs = timeout/1000;
	minutes = secs/60;
	secs = secs%60;
	msecs = timeout%1000;
	buf[0] = 0;
	if (minutes) {
		msecs = 0;
		snprintf(buf, sizeof(buf)-16, "%dmin", minutes);
		if (minutes > 9)
			secs = 0;
	}
	if (secs) {
		if (secs > 9)
			msecs = 0;
		sprintf(buf+strlen(buf), "%d%s", secs, msecs ? "." : "sec");
	}
	if (msecs)
		sprintf(buf+strlen(buf), "%03dms", msecs);
	return buf;
}

struct scache {
	struct scache *next;
	int port;
	char *name;
	const char *proto;
};

static struct scache *rlist;

static void init_service_resolver(void)
{
	char buf[128];
	FILE *fp = popen("/usr/sbin/rpcinfo -p 2>/dev/null", "r");

	if (!fp)
		return;

	if (!fgets(buf, sizeof(buf), fp)) {
		pclose(fp);
		return;
	}
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		unsigned int progn, port;
		char proto[128], prog[128] = "rpc.";
		struct scache *c;

		if (sscanf(buf, "%u %*d %s %u %s",
			   &progn, proto, &port, prog+4) != 4)
			continue;

		if (!(c = malloc(sizeof(*c))))
			continue;

		c->port = port;
		c->name = strdup(prog);
		if (strcmp(proto, TCP_PROTO) == 0)
			c->proto = TCP_PROTO;
		else if (strcmp(proto, UDP_PROTO) == 0)
			c->proto = UDP_PROTO;
		else if (strcmp(proto, SCTP_PROTO) == 0)
			c->proto = SCTP_PROTO;
		else
			c->proto = NULL;
		c->next = rlist;
		rlist = c;
	}
	pclose(fp);
}

/* Even do not try default linux ephemeral port ranges:
 * default /etc/services contains so much of useless crap
 * wouldbe "allocated" to this area that resolution
 * is really harmful. I shrug each time when seeing
 * "socks" or "cfinger" in dumps.
 */
static int is_ephemeral(int port)
{
	static int min = 0, max;

	if (!min) {
		FILE *f = ephemeral_ports_open();

		if (!f || fscanf(f, "%d %d", &min, &max) < 2) {
			min = 1024;
			max = 4999;
		}
		if (f)
			fclose(f);
	}
	return port >= min && port <= max;
}


static const char *__resolve_service(int port)
{
	struct scache *c;

	for (c = rlist; c; c = c->next) {
		if (c->port == port && c->proto == dg_proto)
			return c->name;
	}

	if (!is_ephemeral(port)) {
		static int notfirst;
		struct servent *se;

		if (!notfirst) {
			setservent(1);
			notfirst = 1;
		}
		se = getservbyport(htons(port), dg_proto);
		if (se)
			return se->s_name;
	}

	return NULL;
}

#define SCACHE_BUCKETS 1024
static struct scache *cache_htab[SCACHE_BUCKETS];

static const char *resolve_service(int port)
{
	static char buf[128];
	struct scache *c;
	const char *res;
	int hash;

	if (port == 0) {
		buf[0] = '*';
		buf[1] = 0;
		return buf;
	}

	if (numeric)
		goto do_numeric;

	if (dg_proto == RAW_PROTO)
		return inet_proto_n2a(port, buf, sizeof(buf));


	hash = (port^(((unsigned long)dg_proto)>>2)) % SCACHE_BUCKETS;

	for (c = cache_htab[hash]; c; c = c->next) {
		if (c->port == port && c->proto == dg_proto)
			goto do_cache;
	}

	c = malloc(sizeof(*c));
	if (!c)
		goto do_numeric;
	res = __resolve_service(port);
	c->port = port;
	c->name = res ? strdup(res) : NULL;
	c->proto = dg_proto;
	c->next = cache_htab[hash];
	cache_htab[hash] = c;

do_cache:
	if (c->name)
		return c->name;

do_numeric:
	sprintf(buf, "%u", port);
	return buf;
}

static void inet_addr_print(const inet_prefix *a, int port,
			    unsigned int ifindex, bool v6only)
{
	char buf[1024];
	const char *ap = buf;
	const char *ifname = NULL;

	if (a->family == AF_INET) {
		ap = format_host(AF_INET, 4, a->data);
	} else {
		if (!v6only &&
		    !memcmp(a->data, &in6addr_any, sizeof(in6addr_any))) {
			buf[0] = '*';
			buf[1] = 0;
		} else {
			ap = format_host(a->family, 16, a->data);

			/* Numeric IPv6 addresses should be bracketed */
			if (strchr(ap, ':')) {
				snprintf(buf, sizeof(buf),
					 "[%s]", ap);
				ap = buf;
			}
		}
	}

	if (ifindex)
		ifname = ll_index_to_name(ifindex);

	sock_addr_print(ap, ":", resolve_service(port), ifname);
}

struct aafilter {
	inet_prefix	addr;
	int		port;
	unsigned int	iface;
	__u32		mark;
	__u32		mask;
	__u64		cgroup_id;
	struct aafilter *next;
};

static int inet2_addr_match(const inet_prefix *a, const inet_prefix *p,
			    int plen)
{
	if (!inet_addr_match(a, p, plen))
		return 0;

	/* Cursed "v4 mapped" addresses: v4 mapped socket matches
	 * pure IPv4 rule, but v4-mapped rule selects only v4-mapped
	 * sockets. Fair? */
	if (p->family == AF_INET && a->family == AF_INET6) {
		if (a->data[0] == 0 && a->data[1] == 0 &&
		    a->data[2] == htonl(0xffff)) {
			inet_prefix tmp = *a;

			tmp.data[0] = a->data[3];
			return inet_addr_match(&tmp, p, plen);
		}
	}
	return 1;
}

static int unix_match(const inet_prefix *a, const inet_prefix *p)
{
	char *addr, *pattern;

	memcpy(&addr, a->data, sizeof(addr));
	memcpy(&pattern, p->data, sizeof(pattern));
	if (pattern == NULL)
		return 1;
	if (addr == NULL)
		addr = "";
	return !fnmatch(pattern, addr, FNM_CASEFOLD);
}

static int run_ssfilter(struct ssfilter *f, struct sockstat *s)
{
	switch (f->type) {
		case SSF_S_AUTO:
	{
		if (s->local.family == AF_UNIX) {
			char *p;

			memcpy(&p, s->local.data, sizeof(p));
			return p == NULL || (p[0] == '@' && strlen(p) == 6 &&
					     strspn(p+1, "0123456789abcdef") == 5);
		}
		if (s->local.family == AF_PACKET)
			return s->lport == 0 && s->local.data[0] == 0;
		if (s->local.family == AF_NETLINK)
			return s->lport < 0;
		if (s->local.family == AF_VSOCK)
			return s->lport > 1023;

		return is_ephemeral(s->lport);
	}
		case SSF_DCOND:
	{
		struct aafilter *a = (void *)f->pred;

		if (a->addr.family == AF_UNIX)
			return unix_match(&s->remote, &a->addr);
		if (a->port != -1 && a->port != s->rport)
			return 0;
		if (a->addr.bitlen) {
			do {
				if (!inet2_addr_match(&s->remote, &a->addr, a->addr.bitlen))
					return 1;
			} while ((a = a->next) != NULL);
			return 0;
		}
		return 1;
	}
		case SSF_SCOND:
	{
		struct aafilter *a = (void *)f->pred;

		if (a->addr.family == AF_UNIX)
			return unix_match(&s->local, &a->addr);
		if (a->port != -1 && a->port != s->lport)
			return 0;
		if (a->addr.bitlen) {
			do {
				if (!inet2_addr_match(&s->local, &a->addr, a->addr.bitlen))
					return 1;
			} while ((a = a->next) != NULL);
			return 0;
		}
		return 1;
	}
		case SSF_D_GE:
	{
		struct aafilter *a = (void *)f->pred;

		return s->rport >= a->port;
	}
		case SSF_D_LE:
	{
		struct aafilter *a = (void *)f->pred;

		return s->rport <= a->port;
	}
		case SSF_S_GE:
	{
		struct aafilter *a = (void *)f->pred;

		return s->lport >= a->port;
	}
		case SSF_S_LE:
	{
		struct aafilter *a = (void *)f->pred;

		return s->lport <= a->port;
	}
		case SSF_DEVCOND:
	{
		struct aafilter *a = (void *)f->pred;

		return s->iface == a->iface;
	}
		case SSF_MARKMASK:
	{
		struct aafilter *a = (void *)f->pred;

		return (s->mark & a->mask) == a->mark;
	}
		case SSF_CGROUPCOND:
	{
		struct aafilter *a = (void *)f->pred;

		return s->cgroup_id == a->cgroup_id;
	}
		/* Yup. It is recursion. Sorry. */
		case SSF_AND:
		return run_ssfilter(f->pred, s) && run_ssfilter(f->post, s);
		case SSF_OR:
		return run_ssfilter(f->pred, s) || run_ssfilter(f->post, s);
		case SSF_NOT:
		return !run_ssfilter(f->pred, s);
		default:
		abort();
	}
}

/* Relocate external jumps by reloc. */
static void ssfilter_patch(char *a, int len, int reloc)
{
	while (len > 0) {
		struct inet_diag_bc_op *op = (struct inet_diag_bc_op *)a;

		if (op->no == len+4)
			op->no += reloc;
		len -= op->yes;
		a += op->yes;
	}
	if (len < 0)
		abort();
}

static int ssfilter_bytecompile(struct ssfilter *f, char **bytecode)
{
	switch (f->type) {
		case SSF_S_AUTO:
	{
		if (!(*bytecode = malloc(4))) abort();
		((struct inet_diag_bc_op *)*bytecode)[0] = (struct inet_diag_bc_op){ INET_DIAG_BC_AUTO, 4, 8 };
		return 4;
	}
		case SSF_DCOND:
		case SSF_SCOND:
	{
		struct aafilter *a = (void *)f->pred;
		struct aafilter *b;
		char *ptr;
		int  code = (f->type == SSF_DCOND ? INET_DIAG_BC_D_COND : INET_DIAG_BC_S_COND);
		int len = 0;

		for (b = a; b; b = b->next) {
			len += 4 + sizeof(struct inet_diag_hostcond);
			if (a->addr.family == AF_INET6)
				len += 16;
			else
				len += 4;
			if (b->next)
				len += 4;
		}
		if (!(ptr = malloc(len))) abort();
		*bytecode = ptr;
		for (b = a; b; b = b->next) {
			struct inet_diag_bc_op *op = (struct inet_diag_bc_op *)ptr;
			int alen = (a->addr.family == AF_INET6 ? 16 : 4);
			int oplen = alen + 4 + sizeof(struct inet_diag_hostcond);
			struct inet_diag_hostcond *cond = (struct inet_diag_hostcond *)(ptr+4);

			*op = (struct inet_diag_bc_op){ code, oplen, oplen+4 };
			cond->family = a->addr.family;
			cond->port = a->port;
			cond->prefix_len = a->addr.bitlen;
			memcpy(cond->addr, a->addr.data, alen);
			ptr += oplen;
			if (b->next) {
				op = (struct inet_diag_bc_op *)ptr;
				*op = (struct inet_diag_bc_op){ INET_DIAG_BC_JMP, 4, len - (ptr-*bytecode)};
				ptr += 4;
			}
		}
		return ptr - *bytecode;
	}
		case SSF_D_GE:
	{
		struct aafilter *x = (void *)f->pred;

		if (!(*bytecode = malloc(8))) abort();
		((struct inet_diag_bc_op *)*bytecode)[0] = (struct inet_diag_bc_op){ INET_DIAG_BC_D_GE, 8, 12 };
		((struct inet_diag_bc_op *)*bytecode)[1] = (struct inet_diag_bc_op){ 0, 0, x->port };
		return 8;
	}
		case SSF_D_LE:
	{
		struct aafilter *x = (void *)f->pred;

		if (!(*bytecode = malloc(8))) abort();
		((struct inet_diag_bc_op *)*bytecode)[0] = (struct inet_diag_bc_op){ INET_DIAG_BC_D_LE, 8, 12 };
		((struct inet_diag_bc_op *)*bytecode)[1] = (struct inet_diag_bc_op){ 0, 0, x->port };
		return 8;
	}
		case SSF_S_GE:
	{
		struct aafilter *x = (void *)f->pred;

		if (!(*bytecode = malloc(8))) abort();
		((struct inet_diag_bc_op *)*bytecode)[0] = (struct inet_diag_bc_op){ INET_DIAG_BC_S_GE, 8, 12 };
		((struct inet_diag_bc_op *)*bytecode)[1] = (struct inet_diag_bc_op){ 0, 0, x->port };
		return 8;
	}
		case SSF_S_LE:
	{
		struct aafilter *x = (void *)f->pred;

		if (!(*bytecode = malloc(8))) abort();
		((struct inet_diag_bc_op *)*bytecode)[0] = (struct inet_diag_bc_op){ INET_DIAG_BC_S_LE, 8, 12 };
		((struct inet_diag_bc_op *)*bytecode)[1] = (struct inet_diag_bc_op){ 0, 0, x->port };
		return 8;
	}

		case SSF_AND:
	{
		char *a1 = NULL, *a2 = NULL, *a;
		int l1, l2;

		l1 = ssfilter_bytecompile(f->pred, &a1);
		l2 = ssfilter_bytecompile(f->post, &a2);
		if (!l1 || !l2) {
			free(a1);
			free(a2);
			return 0;
		}
		if (!(a = malloc(l1+l2))) abort();
		memcpy(a, a1, l1);
		memcpy(a+l1, a2, l2);
		free(a1); free(a2);
		ssfilter_patch(a, l1, l2);
		*bytecode = a;
		return l1+l2;
	}
		case SSF_OR:
	{
		char *a1 = NULL, *a2 = NULL, *a;
		int l1, l2;

		l1 = ssfilter_bytecompile(f->pred, &a1);
		l2 = ssfilter_bytecompile(f->post, &a2);
		if (!l1 || !l2) {
			free(a1);
			free(a2);
			return 0;
		}
		if (!(a = malloc(l1+l2+4))) abort();
		memcpy(a, a1, l1);
		memcpy(a+l1+4, a2, l2);
		free(a1); free(a2);
		*(struct inet_diag_bc_op *)(a+l1) = (struct inet_diag_bc_op){ INET_DIAG_BC_JMP, 4, l2+4 };
		*bytecode = a;
		return l1+l2+4;
	}
		case SSF_NOT:
	{
		char *a1 = NULL, *a;
		int l1;

		l1 = ssfilter_bytecompile(f->pred, &a1);
		if (!l1) {
			free(a1);
			return 0;
		}
		if (!(a = malloc(l1+4))) abort();
		memcpy(a, a1, l1);
		free(a1);
		*(struct inet_diag_bc_op *)(a+l1) = (struct inet_diag_bc_op){ INET_DIAG_BC_JMP, 4, 8 };
		*bytecode = a;
		return l1+4;
	}
		case SSF_DEVCOND:
	{
		/* bytecompile for SSF_DEVCOND not supported yet */
		return 0;
	}
		case SSF_MARKMASK:
	{
		struct aafilter *a = (void *)f->pred;
		struct instr {
			struct inet_diag_bc_op op;
			struct inet_diag_markcond cond;
		};
		int inslen = sizeof(struct instr);

		if (!(*bytecode = malloc(inslen))) abort();
		((struct instr *)*bytecode)[0] = (struct instr) {
			{ INET_DIAG_BC_MARK_COND, inslen, inslen + 4 },
			{ a->mark, a->mask},
		};

		return inslen;
	}
		case SSF_CGROUPCOND:
	{
		struct aafilter *a = (void *)f->pred;
		struct instr {
			struct inet_diag_bc_op op;
			__u64 cgroup_id;
		} __attribute__((packed));
		int inslen = sizeof(struct instr);

		if (!(*bytecode = malloc(inslen))) abort();
		((struct instr *)*bytecode)[0] = (struct instr) {
			{ INET_DIAG_BC_CGROUP_COND, inslen, inslen + 4 },
			a->cgroup_id,
		};

		return inslen;
	}
		default:
		abort();
	}
}

static int remember_he(struct aafilter *a, struct hostent *he)
{
	char **ptr = he->h_addr_list;
	int cnt = 0;
	int len;

	if (he->h_addrtype == AF_INET)
		len = 4;
	else if (he->h_addrtype == AF_INET6)
		len = 16;
	else
		return 0;

	while (*ptr) {
		struct aafilter *b = a;

		if (a->addr.bitlen) {
			if ((b = malloc(sizeof(*b))) == NULL)
				return cnt;
			*b = *a;
			a->next = b;
		}
		memcpy(b->addr.data, *ptr, len);
		b->addr.bytelen = len;
		b->addr.bitlen = len*8;
		b->addr.family = he->h_addrtype;
		ptr++;
		cnt++;
	}
	return cnt;
}

static int get_dns_host(struct aafilter *a, const char *addr, int fam)
{
	static int notfirst;
	int cnt = 0;
	struct hostent *he;

	a->addr.bitlen = 0;
	if (!notfirst) {
		sethostent(1);
		notfirst = 1;
	}
	he = gethostbyname2(addr, fam == AF_UNSPEC ? AF_INET : fam);
	if (he)
		cnt = remember_he(a, he);
	if (fam == AF_UNSPEC) {
		he = gethostbyname2(addr, AF_INET6);
		if (he)
			cnt += remember_he(a, he);
	}
	return !cnt;
}

static int xll_initted;

static void xll_init(void)
{
	struct rtnl_handle rth;

	if (rtnl_open(&rth, 0) < 0)
		exit(1);

	ll_init_map(&rth);
	rtnl_close(&rth);
	xll_initted = 1;
}

static const char *xll_index_to_name(int index)
{
	if (!xll_initted)
		xll_init();
	return ll_index_to_name(index);
}

static int xll_name_to_index(const char *dev)
{
	if (!xll_initted)
		xll_init();
	return ll_name_to_index(dev);
}

void *parse_devcond(char *name)
{
	struct aafilter a = { .iface = 0 };
	struct aafilter *res;

	a.iface = xll_name_to_index(name);
	if (a.iface == 0) {
		char *end;
		unsigned long n;

		n = strtoul(name, &end, 0);
		if (!end || end == name || *end || n > UINT_MAX)
			return NULL;

		a.iface = n;
	}

	res = malloc(sizeof(*res));
	*res = a;

	return res;
}

static void vsock_set_inet_prefix(inet_prefix *a, __u32 cid)
{
	*a = (inet_prefix){
		.bytelen = sizeof(cid),
		.family = AF_VSOCK,
	};
	memcpy(a->data, &cid, sizeof(cid));
}

static char* find_port(char *addr, bool is_port)
{
	char *port = NULL;
	if (is_port)
		port = addr;
	else
		port = strchr(addr, ':');
	if (port && *port == ':')
		*port++ = '\0';
	return port;
}

void *parse_hostcond(char *addr, bool is_port)
{
	char *port = NULL;
	struct aafilter a = { .port = -1 };
	struct aafilter *res;
	int fam = preferred_family;
	struct filter *f = &current_filter;

	if (strncmp(addr, "unix:", 5) == 0) {
		fam = AF_UNIX;
		addr += 5;
	} else if (strncmp(addr, "link:", 5) == 0) {
		fam = AF_PACKET;
		addr += 5;
	} else if (strncmp(addr, "netlink:", 8) == 0) {
		fam = AF_NETLINK;
		addr += 8;
	} else if (strncmp(addr, "vsock:", 6) == 0) {
		fam = AF_VSOCK;
		addr += 6;
	} else if (strncmp(addr, "inet:", 5) == 0) {
		fam = AF_INET;
		addr += 5;
	} else if (strncmp(addr, "inet6:", 6) == 0) {
		fam = AF_INET6;
		addr += 6;
	}

	if (fam == AF_UNIX) {
		char *p;

		a.addr.family = AF_UNIX;
		p = strdup(addr);
		a.addr.bitlen = 8*strlen(p);
		memcpy(a.addr.data, &p, sizeof(p));
		goto out;
	}

	if (fam == AF_PACKET) {
		a.addr.family = AF_PACKET;
		a.addr.bitlen = 0;
		port = find_port(addr, is_port);
		if (port) {
			if (*port && strcmp(port, "*")) {
				if (get_integer(&a.port, port, 0)) {
					if ((a.port = xll_name_to_index(port)) <= 0)
						return NULL;
				}
			}
		}
		if (!is_port && addr[0] && strcmp(addr, "*")) {
			unsigned short tmp;

			a.addr.bitlen = 32;
			if (ll_proto_a2n(&tmp, addr))
				return NULL;
			a.addr.data[0] = ntohs(tmp);
		}
		goto out;
	}

	if (fam == AF_NETLINK) {
		a.addr.family = AF_NETLINK;
		a.addr.bitlen = 0;
		port = find_port(addr, is_port);
		if (port) {
			if (*port && strcmp(port, "*")) {
				if (get_integer(&a.port, port, 0)) {
					if (strcmp(port, "kernel") == 0)
						a.port = 0;
					else
						return NULL;
				}
			}
		}
		if (!is_port && addr[0] && strcmp(addr, "*")) {
			a.addr.bitlen = 32;
			if (nl_proto_a2n(&a.addr.data[0], addr) == -1)
				return NULL;
		}
		goto out;
	}

	if (fam == AF_VSOCK) {
		__u32 cid = ~(__u32)0;

		a.addr.family = AF_VSOCK;

		port = find_port(addr, is_port);

		if (port && strcmp(port, "*") &&
		    get_u32((__u32 *)&a.port, port, 0))
			return NULL;

		if (!is_port && addr[0] && strcmp(addr, "*")) {
			a.addr.bitlen = 32;
			if (get_u32(&cid, addr, 0))
				return NULL;
		}
		vsock_set_inet_prefix(&a.addr, cid);
		goto out;
	}

	/* URL-like literal [] */
	if (addr[0] == '[') {
		addr++;
		if ((port = strchr(addr, ']')) == NULL)
			return NULL;
		*port++ = 0;
	} else if (addr[0] == '*') {
		port = addr+1;
	} else {
		port = strrchr(strchr(addr, '/') ? : addr, ':');
	}

	if (is_port)
		port = addr;

	if (port && *port) {
		if (*port == ':')
			*port++ = 0;

		if (*port && *port != '*') {
			if (get_integer(&a.port, port, 0)) {
				struct servent *se1 = NULL;
				struct servent *se2 = NULL;

				if (current_filter.dbs&(1<<UDP_DB))
					se1 = getservbyname(port, UDP_PROTO);
				if (current_filter.dbs&(1<<TCP_DB))
					se2 = getservbyname(port, TCP_PROTO);
				if (se1 && se2 && se1->s_port != se2->s_port) {
					fprintf(stderr, "Error: ambiguous port \"%s\".\n", port);
					return NULL;
				}
				if (!se1)
					se1 = se2;
				if (se1) {
					a.port = ntohs(se1->s_port);
				} else {
					struct scache *s;

					for (s = rlist; s; s = s->next) {
						if ((s->proto == UDP_PROTO &&
						     (current_filter.dbs&(1<<UDP_DB))) ||
						    (s->proto == TCP_PROTO &&
						     (current_filter.dbs&(1<<TCP_DB)))) {
							if (s->name && strcmp(s->name, port) == 0) {
								if (a.port > 0 && a.port != s->port) {
									fprintf(stderr, "Error: ambiguous port \"%s\".\n", port);
									return NULL;
								}
								a.port = s->port;
							}
						}
					}
					if (a.port <= 0) {
						fprintf(stderr, "Error: \"%s\" does not look like a port.\n", port);
						return NULL;
					}
				}
			}
		}
	}
	if (!is_port && *addr && *addr != '*') {
		if (get_prefix_1(&a.addr, addr, fam)) {
			if (get_dns_host(&a, addr, fam)) {
				fprintf(stderr, "Error: an inet prefix is expected rather than \"%s\".\n", addr);
				return NULL;
			}
		}
	}

out:
	if (fam != AF_UNSPEC) {
		int states = f->states;
		f->families = 0;
		filter_af_set(f, fam);
		filter_states_set(f, states);
	}

	res = malloc(sizeof(*res));
	if (res)
		memcpy(res, &a, sizeof(a));
	return res;
}

void *parse_markmask(const char *markmask)
{
	struct aafilter a, *res;

	if (strchr(markmask, '/')) {
		if (sscanf(markmask, "%i/%i", &a.mark, &a.mask) != 2)
			return NULL;
	} else {
		a.mask = 0xffffffff;
		if (sscanf(markmask, "%i", &a.mark) != 1)
			return NULL;
	}

	res = malloc(sizeof(*res));
	if (res)
		memcpy(res, &a, sizeof(a));
	return res;
}

void *parse_cgroupcond(const char *path)
{
	struct aafilter *res;
	__u64 id;

	id = get_cgroup2_id(path);
	if (!id)
		return NULL;

	res = malloc(sizeof(*res));
	if (res)
		res->cgroup_id = id;

	return res;
}

static void proc_ctx_print(struct sockstat *s)
{
	char *buf;

	if (show_proc_ctx || show_sock_ctx) {
		if (find_entry(s->ino, &buf,
				(show_proc_ctx & show_sock_ctx) ?
				PROC_SOCK_CTX : PROC_CTX) > 0) {
			out(" users:(%s)", buf);
			free(buf);
		}
	} else if (show_users) {
		if (find_entry(s->ino, &buf, USERS) > 0) {
			out(" users:(%s)", buf);
			free(buf);
		}
	}
}

static void inet_stats_print(struct sockstat *s, bool v6only)
{
	sock_state_print(s);

	inet_addr_print(&s->local, s->lport, s->iface, v6only);
	inet_addr_print(&s->remote, s->rport, 0, v6only);

	proc_ctx_print(s);
}

static int proc_parse_inet_addr(char *loc, char *rem, int family, struct
		sockstat * s)
{
	s->local.family = s->remote.family = family;
	if (family == AF_INET) {
		sscanf(loc, "%x:%x", s->local.data, (unsigned *)&s->lport);
		sscanf(rem, "%x:%x", s->remote.data, (unsigned *)&s->rport);
		s->local.bytelen = s->remote.bytelen = 4;
		return 0;
	} else {
		sscanf(loc, "%08x%08x%08x%08x:%x",
		       s->local.data,
		       s->local.data + 1,
		       s->local.data + 2,
		       s->local.data + 3,
		       &s->lport);
		sscanf(rem, "%08x%08x%08x%08x:%x",
		       s->remote.data,
		       s->remote.data + 1,
		       s->remote.data + 2,
		       s->remote.data + 3,
		       &s->rport);
		s->local.bytelen = s->remote.bytelen = 16;
		return 0;
	}
	return -1;
}

static int proc_inet_split_line(char *line, char **loc, char **rem, char **data)
{
	char *p;

	if ((p = strchr(line, ':')) == NULL)
		return -1;

	*loc = p+2;
	if ((p = strchr(*loc, ':')) == NULL)
		return -1;

	p[5] = 0;
	*rem = p+6;
	if ((p = strchr(*rem, ':')) == NULL)
		return -1;

	p[5] = 0;
	*data = p+6;
	return 0;
}

/*
 * Display bandwidth in standard units
 * See: https://en.wikipedia.org/wiki/Data-rate_units
 * bw is in bits per second
 */
static char *sprint_bw(char *buf, double bw)
{
	if (numeric)
		sprintf(buf, "%.0f", bw);
	else if (bw >= 1e12)
		sprintf(buf, "%.3gT", bw / 1e12);
	else if (bw >= 1e9)
		sprintf(buf, "%.3gG", bw / 1e9);
	else if (bw >= 1e6)
		sprintf(buf, "%.3gM", bw / 1e6);
	else if (bw >= 1e3)
		sprintf(buf, "%.3gk", bw / 1e3);
	else
		sprintf(buf, "%g", bw);

	return buf;
}

static void sctp_stats_print(struct sctp_info *s)
{
	if (s->sctpi_tag)
		out(" tag:%x", s->sctpi_tag);
	if (s->sctpi_state)
		out(" state:%s", sctp_sstate_name[s->sctpi_state]);
	if (s->sctpi_rwnd)
		out(" rwnd:%d", s->sctpi_rwnd);
	if (s->sctpi_unackdata)
		out(" unackdata:%d", s->sctpi_unackdata);
	if (s->sctpi_penddata)
		out(" penddata:%d", s->sctpi_penddata);
	if (s->sctpi_instrms)
		out(" instrms:%d", s->sctpi_instrms);
	if (s->sctpi_outstrms)
		out(" outstrms:%d", s->sctpi_outstrms);
	if (s->sctpi_inqueue)
		out(" inqueue:%d", s->sctpi_inqueue);
	if (s->sctpi_outqueue)
		out(" outqueue:%d", s->sctpi_outqueue);
	if (s->sctpi_overall_error)
		out(" overerr:%d", s->sctpi_overall_error);
	if (s->sctpi_max_burst)
		out(" maxburst:%d", s->sctpi_max_burst);
	if (s->sctpi_maxseg)
		out(" maxseg:%d", s->sctpi_maxseg);
	if (s->sctpi_peer_rwnd)
		out(" prwnd:%d", s->sctpi_peer_rwnd);
	if (s->sctpi_peer_tag)
		out(" ptag:%x", s->sctpi_peer_tag);
	if (s->sctpi_peer_capable)
		out(" pcapable:%d", s->sctpi_peer_capable);
	if (s->sctpi_peer_sack)
		out(" psack:%d", s->sctpi_peer_sack);
	if (s->sctpi_s_autoclose)
		out(" autoclose:%d", s->sctpi_s_autoclose);
	if (s->sctpi_s_adaptation_ind)
		out(" adapind:%d", s->sctpi_s_adaptation_ind);
	if (s->sctpi_s_pd_point)
		out(" pdpoint:%d", s->sctpi_s_pd_point);
	if (s->sctpi_s_nodelay)
		out(" nodelay:%d", s->sctpi_s_nodelay);
	if (s->sctpi_s_disable_fragments)
		out(" nofrag:%d", s->sctpi_s_disable_fragments);
	if (s->sctpi_s_v4mapped)
		out(" v4mapped:%d", s->sctpi_s_v4mapped);
	if (s->sctpi_s_frag_interleave)
		out(" fraginl:%d", s->sctpi_s_frag_interleave);
}

static void tcp_stats_print(struct tcpstat *s)
{
	char b1[64];

	if (s->has_ts_opt)
		out(" ts");
	if (s->has_sack_opt)
		out(" sack");
	if (s->has_ecn_opt)
		out(" ecn");
	if (s->has_ecnseen_opt)
		out(" ecnseen");
	if (s->has_fastopen_opt)
		out(" fastopen");
	if (s->cong_alg[0])
		out(" %s", s->cong_alg);
	if (s->has_wscale_opt)
		out(" wscale:%d,%d", s->snd_wscale, s->rcv_wscale);
	if (s->rto)
		out(" rto:%g", s->rto);
	if (s->backoff)
		out(" backoff:%u", s->backoff);
	if (s->rtt)
		out(" rtt:%g/%g", s->rtt, s->rttvar);
	if (s->ato)
		out(" ato:%g", s->ato);

	if (s->qack)
		out(" qack:%d", s->qack);
	if (s->qack & 1)
		out(" bidir");

	if (s->mss)
		out(" mss:%d", s->mss);
	if (s->pmtu)
		out(" pmtu:%u", s->pmtu);
	if (s->rcv_mss)
		out(" rcvmss:%d", s->rcv_mss);
	if (s->advmss)
		out(" advmss:%d", s->advmss);
	if (s->cwnd)
		out(" cwnd:%u", s->cwnd);
	if (s->ssthresh)
		out(" ssthresh:%d", s->ssthresh);

	if (s->bytes_sent)
		out(" bytes_sent:%llu", s->bytes_sent);
	if (s->bytes_retrans)
		out(" bytes_retrans:%llu", s->bytes_retrans);
	if (s->bytes_acked)
		out(" bytes_acked:%llu", s->bytes_acked);
	if (s->bytes_received)
		out(" bytes_received:%llu", s->bytes_received);
	if (s->segs_out)
		out(" segs_out:%u", s->segs_out);
	if (s->segs_in)
		out(" segs_in:%u", s->segs_in);
	if (s->data_segs_out)
		out(" data_segs_out:%u", s->data_segs_out);
	if (s->data_segs_in)
		out(" data_segs_in:%u", s->data_segs_in);

	if (s->dctcp && s->dctcp->enabled) {
		struct dctcpstat *dctcp = s->dctcp;

		out(" dctcp:(ce_state:%u,alpha:%u,ab_ecn:%u,ab_tot:%u)",
			     dctcp->ce_state, dctcp->alpha, dctcp->ab_ecn,
			     dctcp->ab_tot);
	} else if (s->dctcp) {
		out(" dctcp:fallback_mode");
	}

	if (s->bbr_info) {
		__u64 bw;

		bw = s->bbr_info->bbr_bw_hi;
		bw <<= 32;
		bw |= s->bbr_info->bbr_bw_lo;

		out(" bbr:(bw:%sbps,mrtt:%g",
		    sprint_bw(b1, bw * 8.0),
		    (double)s->bbr_info->bbr_min_rtt / 1000.0);
		if (s->bbr_info->bbr_pacing_gain)
			out(",pacing_gain:%g",
			    (double)s->bbr_info->bbr_pacing_gain / 256.0);
		if (s->bbr_info->bbr_cwnd_gain)
			out(",cwnd_gain:%g",
			    (double)s->bbr_info->bbr_cwnd_gain / 256.0);
		out(")");
	}

	if (s->send_bps)
		out(" send %sbps", sprint_bw(b1, s->send_bps));
	if (s->lastsnd)
		out(" lastsnd:%u", s->lastsnd);
	if (s->lastrcv)
		out(" lastrcv:%u", s->lastrcv);
	if (s->lastack)
		out(" lastack:%u", s->lastack);

	if (s->pacing_rate) {
		out(" pacing_rate %sbps", sprint_bw(b1, s->pacing_rate));
		if (s->pacing_rate_max)
			out("/%sbps", sprint_bw(b1, s->pacing_rate_max));
	}

	if (s->delivery_rate)
		out(" delivery_rate %sbps", sprint_bw(b1, s->delivery_rate));
	if (s->delivered)
		out(" delivered:%u", s->delivered);
	if (s->delivered_ce)
		out(" delivered_ce:%u", s->delivered_ce);
	if (s->app_limited)
		out(" app_limited");

	if (s->busy_time) {
		out(" busy:%llums", s->busy_time / 1000);
		if (s->rwnd_limited)
			out(" rwnd_limited:%llums(%.1f%%)",
			    s->rwnd_limited / 1000,
			    100.0 * s->rwnd_limited / s->busy_time);
		if (s->sndbuf_limited)
			out(" sndbuf_limited:%llums(%.1f%%)",
			    s->sndbuf_limited / 1000,
			    100.0 * s->sndbuf_limited / s->busy_time);
	}

	if (s->unacked)
		out(" unacked:%u", s->unacked);
	if (s->retrans || s->retrans_total)
		out(" retrans:%u/%u", s->retrans, s->retrans_total);
	if (s->lost)
		out(" lost:%u", s->lost);
	if (s->sacked && s->ss.state != SS_LISTEN)
		out(" sacked:%u", s->sacked);
	if (s->dsack_dups)
		out(" dsack_dups:%u", s->dsack_dups);
	if (s->fackets)
		out(" fackets:%u", s->fackets);
	if (s->reordering != 3)
		out(" reordering:%d", s->reordering);
	if (s->reord_seen)
		out(" reord_seen:%d", s->reord_seen);
	if (s->rcv_rtt)
		out(" rcv_rtt:%g", s->rcv_rtt);
	if (s->rcv_space)
		out(" rcv_space:%d", s->rcv_space);
	if (s->rcv_ssthresh)
		out(" rcv_ssthresh:%u", s->rcv_ssthresh);
	if (s->not_sent)
		out(" notsent:%u", s->not_sent);
	if (s->min_rtt)
		out(" minrtt:%g", s->min_rtt);
}

static void tcp_timer_print(struct tcpstat *s)
{
	static const char * const tmr_name[] = {
		"off",
		"on",
		"keepalive",
		"timewait",
		"persist",
		"unknown"
	};

	if (s->timer) {
		if (s->timer > 4)
			s->timer = 5;
		out(" timer:(%s,%s,%d)",
			     tmr_name[s->timer],
			     print_ms_timer(s->timeout),
			     s->retrans);
	}
}

static void sctp_timer_print(struct tcpstat *s)
{
	if (s->timer)
		out(" timer:(T3_RTX,%s,%d)",
		    print_ms_timer(s->timeout), s->retrans);
}

static int tcp_show_line(char *line, const struct filter *f, int family)
{
	int rto = 0, ato = 0;
	struct tcpstat s = {};
	char *loc, *rem, *data;
	char opt[256];
	int n;
	int hz = get_user_hz();

	if (proc_inet_split_line(line, &loc, &rem, &data))
		return -1;

	int state = (data[1] >= 'A') ? (data[1] - 'A' + 10) : (data[1] - '0');

	if (!(f->states & (1 << state)))
		return 0;

	proc_parse_inet_addr(loc, rem, family, &s.ss);

	if (f->f && run_ssfilter(f->f, &s.ss) == 0)
		return 0;

	opt[0] = 0;
	n = sscanf(data, "%x %x:%x %x:%x %x %d %d %u %d %llx %d %d %d %u %d %[^\n]\n",
		   &s.ss.state, &s.ss.wq, &s.ss.rq,
		   &s.timer, &s.timeout, &s.retrans, &s.ss.uid, &s.probes,
		   &s.ss.ino, &s.ss.refcnt, &s.ss.sk, &rto, &ato, &s.qack, &s.cwnd,
		   &s.ssthresh, opt);

	if (n < 17)
		opt[0] = 0;

	if (n < 12) {
		rto = 0;
		s.cwnd = 2;
		s.ssthresh = -1;
		ato = s.qack = 0;
	}

	s.retrans   = s.timer != 1 ? s.probes : s.retrans;
	s.timeout   = (s.timeout * 1000 + hz - 1) / hz;
	s.ato	    = (double)ato / hz;
	s.qack	   /= 2;
	s.rto	    = (double)rto;
	s.ssthresh  = s.ssthresh == -1 ? 0 : s.ssthresh;
	s.rto	    = s.rto != 3 * hz  ? s.rto / hz : 0;
	s.ss.type   = IPPROTO_TCP;

	inet_stats_print(&s.ss, false);

	if (show_options)
		tcp_timer_print(&s);

	if (show_details) {
		sock_details_print(&s.ss);
		if (opt[0])
			out(" opt:\"%s\"", opt);
	}

	if (show_tcpinfo)
		tcp_stats_print(&s);

	return 0;
}

static int generic_record_read(FILE *fp,
			       int (*worker)(char*, const struct filter *, int),
			       const struct filter *f, int fam)
{
	char line[256];

	/* skip header */
	if (fgets(line, sizeof(line), fp) == NULL)
		goto outerr;

	while (fgets(line, sizeof(line), fp) != NULL) {
		int n = strlen(line);

		if (n == 0 || line[n-1] != '\n') {
			errno = -EINVAL;
			return -1;
		}
		line[n-1] = 0;

		if (worker(line, f, fam) < 0)
			return 0;
	}
outerr:

	return ferror(fp) ? -1 : 0;
}

static void print_skmeminfo(struct rtattr *tb[], int attrtype)
{
	const __u32 *skmeminfo;

	if (!tb[attrtype]) {
		if (attrtype == INET_DIAG_SKMEMINFO) {
			if (!tb[INET_DIAG_MEMINFO])
				return;

			const struct inet_diag_meminfo *minfo =
				RTA_DATA(tb[INET_DIAG_MEMINFO]);

			out(" mem:(r%u,w%u,f%u,t%u)",
				   minfo->idiag_rmem,
				   minfo->idiag_wmem,
				   minfo->idiag_fmem,
				   minfo->idiag_tmem);
		}
		return;
	}

	skmeminfo = RTA_DATA(tb[attrtype]);

	out(" skmem:(r%u,rb%u,t%u,tb%u,f%u,w%u,o%u",
		     skmeminfo[SK_MEMINFO_RMEM_ALLOC],
		     skmeminfo[SK_MEMINFO_RCVBUF],
		     skmeminfo[SK_MEMINFO_WMEM_ALLOC],
		     skmeminfo[SK_MEMINFO_SNDBUF],
		     skmeminfo[SK_MEMINFO_FWD_ALLOC],
		     skmeminfo[SK_MEMINFO_WMEM_QUEUED],
		     skmeminfo[SK_MEMINFO_OPTMEM]);

	if (RTA_PAYLOAD(tb[attrtype]) >=
		(SK_MEMINFO_BACKLOG + 1) * sizeof(__u32))
		out(",bl%u", skmeminfo[SK_MEMINFO_BACKLOG]);

	if (RTA_PAYLOAD(tb[attrtype]) >=
		(SK_MEMINFO_DROPS + 1) * sizeof(__u32))
		out(",d%u", skmeminfo[SK_MEMINFO_DROPS]);

	out(")");
}

static void print_md5sig(struct tcp_diag_md5sig *sig)
{
	out("%s/%d=",
	    format_host(sig->tcpm_family,
			sig->tcpm_family == AF_INET6 ? 16 : 4,
			&sig->tcpm_addr),
	    sig->tcpm_prefixlen);
	print_escape_buf(sig->tcpm_key, sig->tcpm_keylen, " ,");
}

static void tcp_tls_version(struct rtattr *attr)
{
	u_int16_t val;

	if (!attr)
		return;
	val = rta_getattr_u16(attr);

	switch (val) {
	case TLS_1_2_VERSION:
		out(" version: 1.2");
		break;
	case TLS_1_3_VERSION:
		out(" version: 1.3");
		break;
	default:
		out(" version: unknown(%hu)", val);
		break;
	}
}

static void tcp_tls_cipher(struct rtattr *attr)
{
	u_int16_t val;

	if (!attr)
		return;
	val = rta_getattr_u16(attr);

	switch (val) {
	case TLS_CIPHER_AES_GCM_128:
		out(" cipher: aes-gcm-128");
		break;
	case TLS_CIPHER_AES_GCM_256:
		out(" cipher: aes-gcm-256");
		break;
	}
}

static void tcp_tls_conf(const char *name, struct rtattr *attr)
{
	u_int16_t val;

	if (!attr)
		return;
	val = rta_getattr_u16(attr);

	switch (val) {
	case TLS_CONF_BASE:
		out(" %s: none", name);
		break;
	case TLS_CONF_SW:
		out(" %s: sw", name);
		break;
	case TLS_CONF_HW:
		out(" %s: hw", name);
		break;
	case TLS_CONF_HW_RECORD:
		out(" %s: hw-record", name);
		break;
	default:
		out(" %s: unknown(%hu)", name, val);
		break;
	}
}

static void mptcp_subflow_info(struct rtattr *tb[])
{
	u_int32_t flags = 0;

	if (tb[MPTCP_SUBFLOW_ATTR_FLAGS]) {
		char caps[32 + 1] = { 0 }, *cap = &caps[0];

		flags = rta_getattr_u32(tb[MPTCP_SUBFLOW_ATTR_FLAGS]);

		if (flags & MPTCP_SUBFLOW_FLAG_MCAP_REM)
			*cap++ = 'M';
		if (flags & MPTCP_SUBFLOW_FLAG_MCAP_LOC)
			*cap++ = 'm';
		if (flags & MPTCP_SUBFLOW_FLAG_JOIN_REM)
			*cap++ = 'J';
		if (flags & MPTCP_SUBFLOW_FLAG_JOIN_LOC)
			*cap++ = 'j';
		if (flags & MPTCP_SUBFLOW_FLAG_BKUP_REM)
			*cap++ = 'B';
		if (flags & MPTCP_SUBFLOW_FLAG_BKUP_LOC)
			*cap++ = 'b';
		if (flags & MPTCP_SUBFLOW_FLAG_FULLY_ESTABLISHED)
			*cap++ = 'e';
		if (flags & MPTCP_SUBFLOW_FLAG_CONNECTED)
			*cap++ = 'c';
		if (flags & MPTCP_SUBFLOW_FLAG_MAPVALID)
			*cap++ = 'v';
		if (flags)
			out(" flags:%s", caps);
	}
	if (tb[MPTCP_SUBFLOW_ATTR_TOKEN_REM] &&
	    tb[MPTCP_SUBFLOW_ATTR_TOKEN_LOC] &&
	    tb[MPTCP_SUBFLOW_ATTR_ID_REM] &&
	    tb[MPTCP_SUBFLOW_ATTR_ID_LOC])
		out(" token:%04x(id:%hhu)/%04x(id:%hhu)",
		    rta_getattr_u32(tb[MPTCP_SUBFLOW_ATTR_TOKEN_REM]),
		    rta_getattr_u8(tb[MPTCP_SUBFLOW_ATTR_ID_REM]),
		    rta_getattr_u32(tb[MPTCP_SUBFLOW_ATTR_TOKEN_LOC]),
		    rta_getattr_u8(tb[MPTCP_SUBFLOW_ATTR_ID_LOC]));
	if (tb[MPTCP_SUBFLOW_ATTR_MAP_SEQ])
		out(" seq:%llx",
		    rta_getattr_u64(tb[MPTCP_SUBFLOW_ATTR_MAP_SEQ]));
	if (tb[MPTCP_SUBFLOW_ATTR_MAP_SFSEQ])
		out(" sfseq:%x",
		    rta_getattr_u32(tb[MPTCP_SUBFLOW_ATTR_MAP_SFSEQ]));
	if (tb[MPTCP_SUBFLOW_ATTR_SSN_OFFSET])
		out(" ssnoff:%x",
		    rta_getattr_u32(tb[MPTCP_SUBFLOW_ATTR_SSN_OFFSET]));
	if (tb[MPTCP_SUBFLOW_ATTR_MAP_DATALEN])
		out(" maplen:%x",
		    rta_getattr_u32(tb[MPTCP_SUBFLOW_ATTR_MAP_DATALEN]));
}

#define TCPI_HAS_OPT(info, opt) !!(info->tcpi_options & (opt))

static void tcp_show_info(const struct nlmsghdr *nlh, struct inet_diag_msg *r,
		struct rtattr *tb[])
{
	double rtt = 0;
	struct tcpstat s = {};

	s.ss.state = r->idiag_state;

	print_skmeminfo(tb, INET_DIAG_SKMEMINFO);

	if (tb[INET_DIAG_INFO]) {
		struct tcp_info *info;
		int len = RTA_PAYLOAD(tb[INET_DIAG_INFO]);

		/* workaround for older kernels with less fields */
		if (len < sizeof(*info)) {
			info = alloca(sizeof(*info));
			memcpy(info, RTA_DATA(tb[INET_DIAG_INFO]), len);
			memset((char *)info + len, 0, sizeof(*info) - len);
		} else
			info = RTA_DATA(tb[INET_DIAG_INFO]);

		if (show_options) {
			s.has_ts_opt	   = TCPI_HAS_OPT(info, TCPI_OPT_TIMESTAMPS);
			s.has_sack_opt	   = TCPI_HAS_OPT(info, TCPI_OPT_SACK);
			s.has_ecn_opt	   = TCPI_HAS_OPT(info, TCPI_OPT_ECN);
			s.has_ecnseen_opt  = TCPI_HAS_OPT(info, TCPI_OPT_ECN_SEEN);
			s.has_fastopen_opt = TCPI_HAS_OPT(info, TCPI_OPT_SYN_DATA);
		}

		if (tb[INET_DIAG_CONG])
			strncpy(s.cong_alg,
				rta_getattr_str(tb[INET_DIAG_CONG]),
				sizeof(s.cong_alg) - 1);

		if (TCPI_HAS_OPT(info, TCPI_OPT_WSCALE)) {
			s.has_wscale_opt  = true;
			s.snd_wscale	  = info->tcpi_snd_wscale;
			s.rcv_wscale	  = info->tcpi_rcv_wscale;
		}

		if (info->tcpi_rto && info->tcpi_rto != 3000000)
			s.rto = (double)info->tcpi_rto / 1000;

		s.backoff	 = info->tcpi_backoff;
		s.rtt		 = (double)info->tcpi_rtt / 1000;
		s.rttvar	 = (double)info->tcpi_rttvar / 1000;
		s.ato		 = (double)info->tcpi_ato / 1000;
		s.mss		 = info->tcpi_snd_mss;
		s.rcv_mss	 = info->tcpi_rcv_mss;
		s.advmss	 = info->tcpi_advmss;
		s.rcv_space	 = info->tcpi_rcv_space;
		s.rcv_rtt	 = (double)info->tcpi_rcv_rtt / 1000;
		s.lastsnd	 = info->tcpi_last_data_sent;
		s.lastrcv	 = info->tcpi_last_data_recv;
		s.lastack	 = info->tcpi_last_ack_recv;
		s.unacked	 = info->tcpi_unacked;
		s.retrans	 = info->tcpi_retrans;
		s.retrans_total  = info->tcpi_total_retrans;
		s.lost		 = info->tcpi_lost;
		s.sacked	 = info->tcpi_sacked;
		s.fackets	 = info->tcpi_fackets;
		s.reordering	 = info->tcpi_reordering;
		s.rcv_ssthresh   = info->tcpi_rcv_ssthresh;
		s.cwnd		 = info->tcpi_snd_cwnd;
		s.pmtu		 = info->tcpi_pmtu;

		if (info->tcpi_snd_ssthresh < 0xFFFF)
			s.ssthresh = info->tcpi_snd_ssthresh;

		rtt = (double) info->tcpi_rtt;
		if (tb[INET_DIAG_VEGASINFO]) {
			const struct tcpvegas_info *vinfo
				= RTA_DATA(tb[INET_DIAG_VEGASINFO]);

			if (vinfo->tcpv_enabled &&
					vinfo->tcpv_rtt && vinfo->tcpv_rtt != 0x7fffffff)
				rtt =  vinfo->tcpv_rtt;
		}

		if (tb[INET_DIAG_DCTCPINFO]) {
			struct dctcpstat *dctcp = malloc(sizeof(struct
						dctcpstat));

			const struct tcp_dctcp_info *dinfo
				= RTA_DATA(tb[INET_DIAG_DCTCPINFO]);

			dctcp->enabled	= !!dinfo->dctcp_enabled;
			dctcp->ce_state = dinfo->dctcp_ce_state;
			dctcp->alpha	= dinfo->dctcp_alpha;
			dctcp->ab_ecn	= dinfo->dctcp_ab_ecn;
			dctcp->ab_tot	= dinfo->dctcp_ab_tot;
			s.dctcp		= dctcp;
		}

		if (tb[INET_DIAG_BBRINFO]) {
			const void *bbr_info = RTA_DATA(tb[INET_DIAG_BBRINFO]);
			int len = min(RTA_PAYLOAD(tb[INET_DIAG_BBRINFO]),
				      sizeof(*s.bbr_info));

			s.bbr_info = calloc(1, sizeof(*s.bbr_info));
			if (s.bbr_info && bbr_info)
				memcpy(s.bbr_info, bbr_info, len);
		}

		if (rtt > 0 && info->tcpi_snd_mss && info->tcpi_snd_cwnd) {
			s.send_bps = (double) info->tcpi_snd_cwnd *
				(double)info->tcpi_snd_mss * 8000000. / rtt;
		}

		if (info->tcpi_pacing_rate &&
				info->tcpi_pacing_rate != ~0ULL) {
			s.pacing_rate = info->tcpi_pacing_rate * 8.;

			if (info->tcpi_max_pacing_rate &&
					info->tcpi_max_pacing_rate != ~0ULL)
				s.pacing_rate_max = info->tcpi_max_pacing_rate * 8.;
		}
		s.bytes_acked = info->tcpi_bytes_acked;
		s.bytes_received = info->tcpi_bytes_received;
		s.segs_out = info->tcpi_segs_out;
		s.segs_in = info->tcpi_segs_in;
		s.data_segs_out = info->tcpi_data_segs_out;
		s.data_segs_in = info->tcpi_data_segs_in;
		s.not_sent = info->tcpi_notsent_bytes;
		if (info->tcpi_min_rtt && info->tcpi_min_rtt != ~0U)
			s.min_rtt = (double) info->tcpi_min_rtt / 1000;
		s.delivery_rate = info->tcpi_delivery_rate * 8.;
		s.app_limited = info->tcpi_delivery_rate_app_limited;
		s.busy_time = info->tcpi_busy_time;
		s.rwnd_limited = info->tcpi_rwnd_limited;
		s.sndbuf_limited = info->tcpi_sndbuf_limited;
		s.delivered = info->tcpi_delivered;
		s.delivered_ce = info->tcpi_delivered_ce;
		s.dsack_dups = info->tcpi_dsack_dups;
		s.reord_seen = info->tcpi_reord_seen;
		s.bytes_sent = info->tcpi_bytes_sent;
		s.bytes_retrans = info->tcpi_bytes_retrans;
		tcp_stats_print(&s);
		free(s.dctcp);
		free(s.bbr_info);
	}
	if (tb[INET_DIAG_MD5SIG]) {
		struct tcp_diag_md5sig *sig = RTA_DATA(tb[INET_DIAG_MD5SIG]);
		int len = RTA_PAYLOAD(tb[INET_DIAG_MD5SIG]);

		out(" md5keys:");
		print_md5sig(sig++);
		for (len -= sizeof(*sig); len > 0; len -= sizeof(*sig)) {
			out(",");
			print_md5sig(sig++);
		}
	}
	if (tb[INET_DIAG_ULP_INFO]) {
		struct rtattr *ulpinfo[INET_ULP_INFO_MAX + 1] = { 0 };

		parse_rtattr_nested(ulpinfo, INET_ULP_INFO_MAX,
				    tb[INET_DIAG_ULP_INFO]);

		if (ulpinfo[INET_ULP_INFO_NAME])
			out(" tcp-ulp-%s",
			    rta_getattr_str(ulpinfo[INET_ULP_INFO_NAME]));

		if (ulpinfo[INET_ULP_INFO_TLS]) {
			struct rtattr *tlsinfo[TLS_INFO_MAX + 1] = { 0 };

			parse_rtattr_nested(tlsinfo, TLS_INFO_MAX,
					    ulpinfo[INET_ULP_INFO_TLS]);

			tcp_tls_version(tlsinfo[TLS_INFO_VERSION]);
			tcp_tls_cipher(tlsinfo[TLS_INFO_CIPHER]);
			tcp_tls_conf("rxconf", tlsinfo[TLS_INFO_RXCONF]);
			tcp_tls_conf("txconf", tlsinfo[TLS_INFO_TXCONF]);
		}
		if (ulpinfo[INET_ULP_INFO_MPTCP]) {
			struct rtattr *sfinfo[MPTCP_SUBFLOW_ATTR_MAX + 1] =
				{ 0 };

			parse_rtattr_nested(sfinfo, MPTCP_SUBFLOW_ATTR_MAX,
					    ulpinfo[INET_ULP_INFO_MPTCP]);
			mptcp_subflow_info(sfinfo);
		}
	}
}

static void mptcp_stats_print(struct mptcp_info *s)
{
	if (s->mptcpi_subflows)
		out(" subflows:%d", s->mptcpi_subflows);
	if (s->mptcpi_add_addr_signal)
		out(" add_addr_signal:%d", s->mptcpi_add_addr_signal);
	if (s->mptcpi_add_addr_accepted)
		out(" add_addr_accepted:%d", s->mptcpi_add_addr_accepted);
	if (s->mptcpi_subflows_max)
		out(" subflows_max:%d", s->mptcpi_subflows_max);
	if (s->mptcpi_add_addr_signal_max)
		out(" add_addr_signal_max:%d", s->mptcpi_add_addr_signal_max);
	if (s->mptcpi_add_addr_accepted_max)
		out(" add_addr_accepted_max:%d", s->mptcpi_add_addr_accepted_max);
	if (s->mptcpi_flags & MPTCP_INFO_FLAG_FALLBACK)
		out(" fallback");
	if (s->mptcpi_flags & MPTCP_INFO_FLAG_REMOTE_KEY_RECEIVED)
		out(" remote_key");
	if (s->mptcpi_token)
		out(" token:%x", s->mptcpi_token);
	if (s->mptcpi_write_seq)
		out(" write_seq:%llx", s->mptcpi_write_seq);
	if (s->mptcpi_snd_una)
		out(" snd_una:%llx", s->mptcpi_snd_una);
	if (s->mptcpi_rcv_nxt)
		out(" rcv_nxt:%llx", s->mptcpi_rcv_nxt);
}

static void mptcp_show_info(const struct nlmsghdr *nlh, struct inet_diag_msg *r,
			    struct rtattr *tb[])
{
	print_skmeminfo(tb, INET_DIAG_SKMEMINFO);

	if (tb[INET_DIAG_INFO]) {
		struct mptcp_info *info;
		int len = RTA_PAYLOAD(tb[INET_DIAG_INFO]);

		/* workaround for older kernels with less fields */
		if (len < sizeof(*info)) {
			info = alloca(sizeof(*info));
			memcpy(info, RTA_DATA(tb[INET_DIAG_INFO]), len);
			memset((char *)info + len, 0, sizeof(*info) - len);
		} else
			info = RTA_DATA(tb[INET_DIAG_INFO]);

		mptcp_stats_print(info);
	}
}

static const char *format_host_sa(struct sockaddr_storage *sa)
{
	union {
		struct sockaddr_in sin;
		struct sockaddr_in6 sin6;
	} *saddr = (void *)sa;

	switch (sa->ss_family) {
	case AF_INET:
		return format_host(AF_INET, 4, &saddr->sin.sin_addr);
	case AF_INET6:
		return format_host(AF_INET6, 16, &saddr->sin6.sin6_addr);
	default:
		return "";
	}
}

static void sctp_show_info(const struct nlmsghdr *nlh, struct inet_diag_msg *r,
		struct rtattr *tb[])
{
	struct sockaddr_storage *sa;
	int len;

	print_skmeminfo(tb, INET_DIAG_SKMEMINFO);

	if (tb[INET_DIAG_LOCALS]) {
		len = RTA_PAYLOAD(tb[INET_DIAG_LOCALS]);
		sa = RTA_DATA(tb[INET_DIAG_LOCALS]);

		out(" locals:%s", format_host_sa(sa));
		for (sa++, len -= sizeof(*sa); len > 0; sa++, len -= sizeof(*sa))
			out(",%s", format_host_sa(sa));

	}
	if (tb[INET_DIAG_PEERS]) {
		len = RTA_PAYLOAD(tb[INET_DIAG_PEERS]);
		sa = RTA_DATA(tb[INET_DIAG_PEERS]);

		out(" peers:%s", format_host_sa(sa));
		for (sa++, len -= sizeof(*sa); len > 0; sa++, len -= sizeof(*sa))
			out(",%s", format_host_sa(sa));
	}
	if (tb[INET_DIAG_INFO]) {
		struct sctp_info *info;
		len = RTA_PAYLOAD(tb[INET_DIAG_INFO]);

		/* workaround for older kernels with less fields */
		if (len < sizeof(*info)) {
			info = alloca(sizeof(*info));
			memcpy(info, RTA_DATA(tb[INET_DIAG_INFO]), len);
			memset((char *)info + len, 0, sizeof(*info) - len);
		} else
			info = RTA_DATA(tb[INET_DIAG_INFO]);

		sctp_stats_print(info);
	}
}

static void parse_diag_msg(struct nlmsghdr *nlh, struct sockstat *s)
{
	struct rtattr *tb[INET_DIAG_MAX+1];
	struct inet_diag_msg *r = NLMSG_DATA(nlh);

	parse_rtattr(tb, INET_DIAG_MAX, (struct rtattr *)(r+1),
		     nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

	s->state	= r->idiag_state;
	s->local.family	= s->remote.family = r->idiag_family;
	s->lport	= ntohs(r->id.idiag_sport);
	s->rport	= ntohs(r->id.idiag_dport);
	s->wq		= r->idiag_wqueue;
	s->rq		= r->idiag_rqueue;
	s->ino		= r->idiag_inode;
	s->uid		= r->idiag_uid;
	s->iface	= r->id.idiag_if;
	s->sk		= cookie_sk_get(&r->id.idiag_cookie[0]);

	s->mark = 0;
	if (tb[INET_DIAG_MARK])
		s->mark = rta_getattr_u32(tb[INET_DIAG_MARK]);
	s->cgroup_id = 0;
	if (tb[INET_DIAG_CGROUP_ID])
		s->cgroup_id = rta_getattr_u64(tb[INET_DIAG_CGROUP_ID]);
	if (tb[INET_DIAG_PROTOCOL])
		s->raw_prot = rta_getattr_u8(tb[INET_DIAG_PROTOCOL]);
	else
		s->raw_prot = 0;

	if (s->local.family == AF_INET)
		s->local.bytelen = s->remote.bytelen = 4;
	else
		s->local.bytelen = s->remote.bytelen = 16;

	memcpy(s->local.data, r->id.idiag_src, s->local.bytelen);
	memcpy(s->remote.data, r->id.idiag_dst, s->local.bytelen);
}

static int inet_show_sock(struct nlmsghdr *nlh,
			  struct sockstat *s)
{
	struct rtattr *tb[INET_DIAG_MAX+1];
	struct inet_diag_msg *r = NLMSG_DATA(nlh);
	unsigned char v6only = 0;

	parse_rtattr(tb, INET_DIAG_MAX, (struct rtattr *)(r+1),
		     nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

	if (tb[INET_DIAG_PROTOCOL])
		s->type = rta_getattr_u8(tb[INET_DIAG_PROTOCOL]);

	if (s->local.family == AF_INET6 && tb[INET_DIAG_SKV6ONLY])
		v6only = rta_getattr_u8(tb[INET_DIAG_SKV6ONLY]);

	inet_stats_print(s, v6only);

	if (show_options) {
		struct tcpstat t = {};

		t.timer = r->idiag_timer;
		t.timeout = r->idiag_expires;
		t.retrans = r->idiag_retrans;
		if (s->type == IPPROTO_SCTP)
			sctp_timer_print(&t);
		else
			tcp_timer_print(&t);
	}

	if (show_details) {
		sock_details_print(s);
		if (s->local.family == AF_INET6 && tb[INET_DIAG_SKV6ONLY])
			out(" v6only:%u", v6only);

		if (tb[INET_DIAG_SHUTDOWN]) {
			unsigned char mask;

			mask = rta_getattr_u8(tb[INET_DIAG_SHUTDOWN]);
			out(" %c-%c",
			    mask & 1 ? '-' : '<', mask & 2 ? '-' : '>');
		}
	}

	if (show_tos) {
		if (tb[INET_DIAG_TOS])
			out(" tos:%#x", rta_getattr_u8(tb[INET_DIAG_TOS]));
		if (tb[INET_DIAG_TCLASS])
			out(" tclass:%#x", rta_getattr_u8(tb[INET_DIAG_TCLASS]));
		if (tb[INET_DIAG_CLASS_ID])
			out(" class_id:%#x", rta_getattr_u32(tb[INET_DIAG_CLASS_ID]));
	}

	if (show_cgroup) {
		if (tb[INET_DIAG_CGROUP_ID])
			out(" cgroup:%s", cg_id_to_path(rta_getattr_u64(tb[INET_DIAG_CGROUP_ID])));
	}

	if (show_inet_sockopt) {
		if (tb[INET_DIAG_SOCKOPT] && RTA_PAYLOAD(tb[INET_DIAG_SOCKOPT]) >=
		    sizeof(struct inet_diag_sockopt)) {
			const struct inet_diag_sockopt *sockopt =
					RTA_DATA(tb[INET_DIAG_SOCKOPT]);
			if (!oneline)
				out("\n\tinet-sockopt: (");
			else
				out(" inet-sockopt: (");
			if (sockopt->recverr)
				out(" recverr");
			if (sockopt->is_icsk)
				out(" is_icsk");
			if (sockopt->freebind)
				out(" freebind");
			if (sockopt->hdrincl)
				out(" hdrincl");
			if (sockopt->mc_loop)
				out(" mc_loop");
			if (sockopt->transparent)
				out(" transparent");
			if (sockopt->mc_all)
				out(" mc_all");
			if (sockopt->nodefrag)
				out(" nodefrag");
			if (sockopt->bind_address_no_port)
				out(" bind_addr_no_port");
			if (sockopt->recverr_rfc4884)
				out(" recverr_rfc4884");
			if (sockopt->defer_connect)
				out(" defer_connect");
			out(")");
		}
	}

	if (show_mem || (show_tcpinfo && s->type != IPPROTO_UDP)) {
		if (!oneline)
			out("\n\t");
		if (s->type == IPPROTO_SCTP)
			sctp_show_info(nlh, r, tb);
		else if (s->type == IPPROTO_MPTCP)
			mptcp_show_info(nlh, r, tb);
		else
			tcp_show_info(nlh, r, tb);
	}
	sctp_ino = s->ino;

	return 0;
}

static int tcpdiag_send(int fd, int protocol, struct filter *f)
{
	struct sockaddr_nl nladdr = { .nl_family = AF_NETLINK };
	struct {
		struct nlmsghdr nlh;
		struct inet_diag_req r;
	} req = {
		.nlh.nlmsg_len = sizeof(req),
		.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST,
		.nlh.nlmsg_seq = MAGIC_SEQ,
		.r.idiag_family = AF_INET,
		.r.idiag_states = f->states,
	};
	char    *bc = NULL;
	int	bclen;
	struct msghdr msg;
	struct rtattr rta;
	struct iovec iov[3];
	int iovlen = 1;

	if (protocol == IPPROTO_UDP || protocol == IPPROTO_MPTCP)
		return -1;

	if (protocol == IPPROTO_TCP)
		req.nlh.nlmsg_type = TCPDIAG_GETSOCK;
	else
		req.nlh.nlmsg_type = DCCPDIAG_GETSOCK;
	if (show_mem) {
		req.r.idiag_ext |= (1<<(INET_DIAG_MEMINFO-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_SKMEMINFO-1));
	}

	if (show_tcpinfo) {
		req.r.idiag_ext |= (1<<(INET_DIAG_INFO-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_VEGASINFO-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_CONG-1));
	}

	if (show_tos) {
		req.r.idiag_ext |= (1<<(INET_DIAG_TOS-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_TCLASS-1));
	}

	iov[0] = (struct iovec){
		.iov_base = &req,
		.iov_len = sizeof(req)
	};
	if (f->f) {
		bclen = ssfilter_bytecompile(f->f, &bc);
		if (bclen) {
			rta.rta_type = INET_DIAG_REQ_BYTECODE;
			rta.rta_len = RTA_LENGTH(bclen);
			iov[1] = (struct iovec){ &rta, sizeof(rta) };
			iov[2] = (struct iovec){ bc, bclen };
			req.nlh.nlmsg_len += RTA_LENGTH(bclen);
			iovlen = 3;
		}
	}

	msg = (struct msghdr) {
		.msg_name = (void *)&nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = iov,
		.msg_iovlen = iovlen,
	};

	if (sendmsg(fd, &msg, 0) < 0) {
		close(fd);
		return -1;
	}

	return 0;
}

static int sockdiag_send(int family, int fd, int protocol, struct filter *f)
{
	struct sockaddr_nl nladdr = { .nl_family = AF_NETLINK };
	DIAG_REQUEST(req, struct inet_diag_req_v2 r);
	char    *bc = NULL;
	int	bclen;
	__u32	proto;
	struct msghdr msg;
	struct rtattr rta_bc;
	struct rtattr rta_proto;
	struct iovec iov[5];
	int iovlen = 1;

	if (family == PF_UNSPEC)
		return tcpdiag_send(fd, protocol, f);

	memset(&req.r, 0, sizeof(req.r));
	req.r.sdiag_family = family;
	req.r.sdiag_protocol = protocol;
	req.r.idiag_states = f->states;
	if (show_mem) {
		req.r.idiag_ext |= (1<<(INET_DIAG_MEMINFO-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_SKMEMINFO-1));
	}

	if (show_tcpinfo) {
		req.r.idiag_ext |= (1<<(INET_DIAG_INFO-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_VEGASINFO-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_CONG-1));
	}

	if (show_tos) {
		req.r.idiag_ext |= (1<<(INET_DIAG_TOS-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_TCLASS-1));
	}

	iov[0] = (struct iovec){
		.iov_base = &req,
		.iov_len = sizeof(req)
	};
	if (f->f) {
		bclen = ssfilter_bytecompile(f->f, &bc);
		if (bclen) {
			rta_bc.rta_type = INET_DIAG_REQ_BYTECODE;
			rta_bc.rta_len = RTA_LENGTH(bclen);
			iov[1] = (struct iovec){ &rta_bc, sizeof(rta_bc) };
			iov[2] = (struct iovec){ bc, bclen };
			req.nlh.nlmsg_len += RTA_LENGTH(bclen);
			iovlen = 3;
		}
	}

	/* put extended protocol attribute, if required */
	if (protocol > 255) {
		rta_proto.rta_type = INET_DIAG_REQ_PROTOCOL;
		rta_proto.rta_len = RTA_LENGTH(sizeof(proto));
		proto = protocol;
		iov[iovlen] = (struct iovec){ &rta_proto, sizeof(rta_proto) };
		iov[iovlen + 1] = (struct iovec){ &proto, sizeof(proto) };
		req.nlh.nlmsg_len += RTA_LENGTH(sizeof(proto));
		iovlen += 2;
	}

	msg = (struct msghdr) {
		.msg_name = (void *)&nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = iov,
		.msg_iovlen = iovlen,
	};

	if (sendmsg(fd, &msg, 0) < 0) {
		close(fd);
		return -1;
	}

	return 0;
}

struct inet_diag_arg {
	struct filter *f;
	int protocol;
	struct rtnl_handle *rth;
};

static int kill_inet_sock(struct nlmsghdr *h, void *arg, struct sockstat *s)
{
	struct inet_diag_msg *d = NLMSG_DATA(h);
	struct inet_diag_arg *diag_arg = arg;
	struct rtnl_handle *rth = diag_arg->rth;

	DIAG_REQUEST(req, struct inet_diag_req_v2 r);

	req.nlh.nlmsg_type = SOCK_DESTROY;
	req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	req.nlh.nlmsg_seq = ++rth->seq;
	req.r.sdiag_family = d->idiag_family;
	req.r.sdiag_protocol = diag_arg->protocol;
	req.r.id = d->id;

	if (diag_arg->protocol == IPPROTO_RAW) {
		struct inet_diag_req_raw *raw = (void *)&req.r;

		BUILD_BUG_ON(sizeof(req.r) != sizeof(*raw));
		raw->sdiag_raw_protocol = s->raw_prot;
	}

	return rtnl_talk(rth, &req.nlh, NULL);
}

static int show_one_inet_sock(struct nlmsghdr *h, void *arg)
{
	int err;
	struct inet_diag_arg *diag_arg = arg;
	struct inet_diag_msg *r = NLMSG_DATA(h);
	struct sockstat s = {};

	if (!(diag_arg->f->families & FAMILY_MASK(r->idiag_family)))
		return 0;

	parse_diag_msg(h, &s);
	s.type = diag_arg->protocol;

	if (diag_arg->f->f && run_ssfilter(diag_arg->f->f, &s) == 0)
		return 0;

	if (diag_arg->f->kill && kill_inet_sock(h, arg, &s) != 0) {
		if (errno == EOPNOTSUPP || errno == ENOENT) {
			/* Socket can't be closed, or is already closed. */
			return 0;
		} else {
			perror("SOCK_DESTROY answers");
			return -1;
		}
	}

	err = inet_show_sock(h, &s);
	if (err < 0)
		return err;

	return 0;
}

static int inet_show_netlink(struct filter *f, FILE *dump_fp, int protocol)
{
	int err = 0;
	struct rtnl_handle rth, rth2;
	int family = PF_INET;
	struct inet_diag_arg arg = { .f = f, .protocol = protocol };

	if (rtnl_open_byproto(&rth, 0, NETLINK_SOCK_DIAG))
		return -1;

	if (f->kill) {
		if (rtnl_open_byproto(&rth2, 0, NETLINK_SOCK_DIAG)) {
			rtnl_close(&rth);
			return -1;
		}
		arg.rth = &rth2;
	}

	rth.dump = MAGIC_SEQ;
	rth.dump_fp = dump_fp;
	if (preferred_family == PF_INET6)
		family = PF_INET6;

	/* extended protocol will use INET_DIAG_REQ_PROTOCOL,
	 * not supported by older kernels. On such kernel
	 * rtnl_dump will bail with rtnl_dump_error().
	 * Suppress the error to avoid confusing the user
	 */
	if (protocol > 255)
		rth.flags |= RTNL_HANDLE_F_SUPPRESS_NLERR;

again:
	if ((err = sockdiag_send(family, rth.fd, protocol, f)))
		goto Exit;

	if ((err = rtnl_dump_filter(&rth, show_one_inet_sock, &arg))) {
		if (family != PF_UNSPEC) {
			family = PF_UNSPEC;
			goto again;
		}
		goto Exit;
	}
	if (family == PF_INET && preferred_family != PF_INET) {
		family = PF_INET6;
		goto again;
	}

Exit:
	rtnl_close(&rth);
	if (arg.rth)
		rtnl_close(arg.rth);
	return err;
}

static int tcp_show_netlink_file(struct filter *f)
{
	FILE	*fp;
	char	buf[16384];
	int	err = -1;

	if ((fp = fopen(getenv("TCPDIAG_FILE"), "r")) == NULL) {
		perror("fopen($TCPDIAG_FILE)");
		return err;
	}

	while (1) {
		int err2;
		size_t status, nitems;
		struct nlmsghdr *h = (struct nlmsghdr *)buf;
		struct sockstat s = {};

		status = fread(buf, 1, sizeof(*h), fp);
		if (status != sizeof(*h)) {
			if (ferror(fp))
				perror("Reading header from $TCPDIAG_FILE");
			if (feof(fp))
				fprintf(stderr, "Unexpected EOF reading $TCPDIAG_FILE");
			break;
		}

		nitems = NLMSG_ALIGN(h->nlmsg_len - sizeof(*h));
		status = fread(h+1, 1, nitems, fp);

		if (status != nitems) {
			if (ferror(fp))
				perror("Reading $TCPDIAG_FILE");
			if (feof(fp))
				fprintf(stderr, "Unexpected EOF reading $TCPDIAG_FILE");
			break;
		}

		/* The only legal exit point */
		if (h->nlmsg_type == NLMSG_DONE) {
			err = 0;
			break;
		}

		if (h->nlmsg_type == NLMSG_ERROR) {
			struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(h);

			if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr))) {
				fprintf(stderr, "ERROR truncated\n");
			} else {
				errno = -err->error;
				perror("TCPDIAG answered");
			}
			break;
		}

		parse_diag_msg(h, &s);
		s.type = IPPROTO_TCP;

		if (f && f->f && run_ssfilter(f->f, &s) == 0)
			continue;

		err2 = inet_show_sock(h, &s);
		if (err2 < 0) {
			err = err2;
			break;
		}
	}

	fclose(fp);
	return err;
}

static int tcp_show(struct filter *f)
{
	FILE *fp = NULL;
	char *buf = NULL;
	int bufsize = 1024*1024;

	if (!filter_af_get(f, AF_INET) && !filter_af_get(f, AF_INET6))
		return 0;

	dg_proto = TCP_PROTO;

	if (getenv("TCPDIAG_FILE"))
		return tcp_show_netlink_file(f);

	if (!getenv("PROC_NET_TCP") && !getenv("PROC_ROOT")
	    && inet_show_netlink(f, NULL, IPPROTO_TCP) == 0)
		return 0;

	/* Sigh... We have to parse /proc/net/tcp... */
	while (bufsize >= 64*1024) {
		if ((buf = malloc(bufsize)) != NULL)
			break;
		bufsize /= 2;
	}
	if (buf == NULL) {
		errno = ENOMEM;
		return -1;
	}

	if (f->families & FAMILY_MASK(AF_INET)) {
		if ((fp = net_tcp_open()) == NULL)
			goto outerr;

		setbuffer(fp, buf, bufsize);
		if (generic_record_read(fp, tcp_show_line, f, AF_INET))
			goto outerr;
		fclose(fp);
	}

	if ((f->families & FAMILY_MASK(AF_INET6)) &&
	    (fp = net_tcp6_open()) != NULL) {
		setbuffer(fp, buf, bufsize);
		if (generic_record_read(fp, tcp_show_line, f, AF_INET6))
			goto outerr;
		fclose(fp);
	}

	free(buf);
	return 0;

outerr:
	do {
		int saved_errno = errno;

		free(buf);
		if (fp)
			fclose(fp);
		errno = saved_errno;
		return -1;
	} while (0);
}

static int mptcp_show(struct filter *f)
{
	if (!filter_af_get(f, AF_INET) && !filter_af_get(f, AF_INET6))
		return 0;

	if (!getenv("PROC_NET_MPTCP") && !getenv("PROC_ROOT")
	    && inet_show_netlink(f, NULL, IPPROTO_MPTCP) == 0)
		return 0;

	return 0;
}

static int dccp_show(struct filter *f)
{
	if (!filter_af_get(f, AF_INET) && !filter_af_get(f, AF_INET6))
		return 0;

	if (!getenv("PROC_NET_DCCP") && !getenv("PROC_ROOT")
	    && inet_show_netlink(f, NULL, IPPROTO_DCCP) == 0)
		return 0;

	return 0;
}

static int sctp_show(struct filter *f)
{
	if (!filter_af_get(f, AF_INET) && !filter_af_get(f, AF_INET6))
		return 0;

	if (!getenv("PROC_NET_SCTP") && !getenv("PROC_ROOT")
	    && inet_show_netlink(f, NULL, IPPROTO_SCTP) == 0)
		return 0;

	return 0;
}

static int dgram_show_line(char *line, const struct filter *f, int family)
{
	struct sockstat s = {};
	char *loc, *rem, *data;
	char opt[256];
	int n;

	if (proc_inet_split_line(line, &loc, &rem, &data))
		return -1;

	int state = (data[1] >= 'A') ? (data[1] - 'A' + 10) : (data[1] - '0');

	if (!(f->states & (1 << state)))
		return 0;

	proc_parse_inet_addr(loc, rem, family, &s);

	if (f->f && run_ssfilter(f->f, &s) == 0)
		return 0;

	opt[0] = 0;
	n = sscanf(data, "%x %x:%x %*x:%*x %*x %d %*d %u %d %llx %[^\n]\n",
	       &s.state, &s.wq, &s.rq,
	       &s.uid, &s.ino,
	       &s.refcnt, &s.sk, opt);

	if (n < 9)
		opt[0] = 0;

	s.type = dg_proto == UDP_PROTO ? IPPROTO_UDP : 0;
	inet_stats_print(&s, false);

	if (show_details && opt[0])
		out(" opt:\"%s\"", opt);

	return 0;
}

static int udp_show(struct filter *f)
{
	FILE *fp = NULL;

	if (!filter_af_get(f, AF_INET) && !filter_af_get(f, AF_INET6))
		return 0;

	dg_proto = UDP_PROTO;

	if (!getenv("PROC_NET_UDP") && !getenv("PROC_ROOT")
	    && inet_show_netlink(f, NULL, IPPROTO_UDP) == 0)
		return 0;

	if (f->families&FAMILY_MASK(AF_INET)) {
		if ((fp = net_udp_open()) == NULL)
			goto outerr;
		if (generic_record_read(fp, dgram_show_line, f, AF_INET))
			goto outerr;
		fclose(fp);
	}

	if ((f->families&FAMILY_MASK(AF_INET6)) &&
	    (fp = net_udp6_open()) != NULL) {
		if (generic_record_read(fp, dgram_show_line, f, AF_INET6))
			goto outerr;
		fclose(fp);
	}
	return 0;

outerr:
	do {
		int saved_errno = errno;

		if (fp)
			fclose(fp);
		errno = saved_errno;
		return -1;
	} while (0);
}

static int raw_show(struct filter *f)
{
	FILE *fp = NULL;

	if (!filter_af_get(f, AF_INET) && !filter_af_get(f, AF_INET6))
		return 0;

	dg_proto = RAW_PROTO;

	if (!getenv("PROC_NET_RAW") && !getenv("PROC_ROOT") &&
	    inet_show_netlink(f, NULL, IPPROTO_RAW) == 0)
		return 0;

	if (f->families&FAMILY_MASK(AF_INET)) {
		if ((fp = net_raw_open()) == NULL)
			goto outerr;
		if (generic_record_read(fp, dgram_show_line, f, AF_INET))
			goto outerr;
		fclose(fp);
	}

	if ((f->families&FAMILY_MASK(AF_INET6)) &&
	    (fp = net_raw6_open()) != NULL) {
		if (generic_record_read(fp, dgram_show_line, f, AF_INET6))
			goto outerr;
		fclose(fp);
	}
	return 0;

outerr:
	do {
		int saved_errno = errno;

		if (fp)
			fclose(fp);
		errno = saved_errno;
		return -1;
	} while (0);
}

#define MAX_UNIX_REMEMBER (1024*1024/sizeof(struct sockstat))

static void unix_list_drop_first(struct sockstat **list)
{
	struct sockstat *s = *list;

	(*list) = (*list)->next;
	free(s->name);
	free(s);
}

static bool unix_type_skip(struct sockstat *s, struct filter *f)
{
	if (s->type == SOCK_STREAM && !(f->dbs&(1<<UNIX_ST_DB)))
		return true;
	if (s->type == SOCK_DGRAM && !(f->dbs&(1<<UNIX_DG_DB)))
		return true;
	if (s->type == SOCK_SEQPACKET && !(f->dbs&(1<<UNIX_SQ_DB)))
		return true;
	return false;
}

static void unix_stats_print(struct sockstat *s, struct filter *f)
{
	char port_name[30] = {};

	sock_state_print(s);

	sock_addr_print(s->name ?: "*", " ",
			int_to_str(s->lport, port_name), NULL);
	sock_addr_print(s->peer_name ?: "*", " ",
			int_to_str(s->rport, port_name), NULL);

	proc_ctx_print(s);
}

static int unix_show_sock(struct nlmsghdr *nlh, void *arg)
{
	struct filter *f = (struct filter *)arg;
	struct unix_diag_msg *r = NLMSG_DATA(nlh);
	struct rtattr *tb[UNIX_DIAG_MAX+1];
	char name[128];
	struct sockstat stat = { .name = "*", .peer_name = "*" };

	parse_rtattr(tb, UNIX_DIAG_MAX, (struct rtattr *)(r+1),
		     nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

	stat.type  = r->udiag_type;
	stat.state = r->udiag_state;
	stat.ino   = stat.lport = r->udiag_ino;
	stat.local.family = stat.remote.family = AF_UNIX;

	if (unix_type_skip(&stat, f))
		return 0;

	if (tb[UNIX_DIAG_RQLEN]) {
		struct unix_diag_rqlen *rql = RTA_DATA(tb[UNIX_DIAG_RQLEN]);

		stat.rq = rql->udiag_rqueue;
		stat.wq = rql->udiag_wqueue;
	}
	if (tb[UNIX_DIAG_NAME]) {
		int len = RTA_PAYLOAD(tb[UNIX_DIAG_NAME]);

		memcpy(name, RTA_DATA(tb[UNIX_DIAG_NAME]), len);
		name[len] = '\0';
		if (name[0] == '\0') {
			int i;
			for (i = 0; i < len; i++)
				if (name[i] == '\0')
					name[i] = '@';
		}
		stat.name = &name[0];
		memcpy(stat.local.data, &stat.name, sizeof(stat.name));
	}
	if (tb[UNIX_DIAG_PEER])
		stat.rport = rta_getattr_u32(tb[UNIX_DIAG_PEER]);

	if (f->f && run_ssfilter(f->f, &stat) == 0)
		return 0;

	unix_stats_print(&stat, f);

	if (show_mem)
		print_skmeminfo(tb, UNIX_DIAG_MEMINFO);
	if (show_details) {
		if (tb[UNIX_DIAG_SHUTDOWN]) {
			unsigned char mask;

			mask = rta_getattr_u8(tb[UNIX_DIAG_SHUTDOWN]);
			out(" %c-%c",
			    mask & 1 ? '-' : '<', mask & 2 ? '-' : '>');
		}
		if (tb[UNIX_DIAG_VFS]) {
			struct unix_diag_vfs *uv = RTA_DATA(tb[UNIX_DIAG_VFS]);

			out(" ino:%u dev:%u/%u", uv->udiag_vfs_ino, major(uv->udiag_vfs_dev),
						 minor(uv->udiag_vfs_dev));
		}
		if (tb[UNIX_DIAG_ICONS]) {
			int len = RTA_PAYLOAD(tb[UNIX_DIAG_ICONS]);
			__u32 *peers = RTA_DATA(tb[UNIX_DIAG_ICONS]);
			int i;

			out(" peers:");
			for (i = 0; i < len / sizeof(__u32); i++)
				out(" %u", peers[i]);
		}
	}

	return 0;
}

static int handle_netlink_request(struct filter *f, struct nlmsghdr *req,
		size_t size, rtnl_filter_t show_one_sock)
{
	int ret = -1;
	struct rtnl_handle rth;

	if (rtnl_open_byproto(&rth, 0, NETLINK_SOCK_DIAG))
		return -1;

	rth.dump = MAGIC_SEQ;

	if (rtnl_send(&rth, req, size) < 0)
		goto Exit;

	if (rtnl_dump_filter(&rth, show_one_sock, f))
		goto Exit;

	ret = 0;
Exit:
	rtnl_close(&rth);
	return ret;
}

static int unix_show_netlink(struct filter *f)
{
	DIAG_REQUEST(req, struct unix_diag_req r);

	req.r.sdiag_family = AF_UNIX;
	req.r.udiag_states = f->states;
	req.r.udiag_show = UDIAG_SHOW_NAME | UDIAG_SHOW_PEER | UDIAG_SHOW_RQLEN;
	if (show_mem)
		req.r.udiag_show |= UDIAG_SHOW_MEMINFO;
	if (show_details)
		req.r.udiag_show |= UDIAG_SHOW_VFS | UDIAG_SHOW_ICONS;

	return handle_netlink_request(f, &req.nlh, sizeof(req), unix_show_sock);
}

static int unix_show(struct filter *f)
{
	FILE *fp;
	char buf[256];
	char name[128];
	int  newformat = 0;
	int  cnt;
	struct sockstat *list = NULL;
	const int unix_state_map[] = { SS_CLOSE, SS_SYN_SENT,
				       SS_ESTABLISHED, SS_CLOSING };

	if (!filter_af_get(f, AF_UNIX))
		return 0;

	if (!getenv("PROC_NET_UNIX") && !getenv("PROC_ROOT")
	    && unix_show_netlink(f) == 0)
		return 0;

	if ((fp = net_unix_open()) == NULL)
		return -1;
	if (!fgets(buf, sizeof(buf), fp)) {
		fclose(fp);
		return -1;
	}

	if (memcmp(buf, "Peer", 4) == 0)
		newformat = 1;
	cnt = 0;

	while (fgets(buf, sizeof(buf), fp)) {
		struct sockstat *u, **insp;
		int flags;

		if (!(u = calloc(1, sizeof(*u))))
			break;

		if (sscanf(buf, "%x: %x %x %x %x %x %d %s",
			   &u->rport, &u->rq, &u->wq, &flags, &u->type,
			   &u->state, &u->ino, name) < 8)
			name[0] = 0;

		u->lport = u->ino;
		u->local.family = u->remote.family = AF_UNIX;

		if (flags & (1 << 16)) {
			u->state = SS_LISTEN;
		} else if (u->state > 0 &&
			   u->state <= ARRAY_SIZE(unix_state_map)) {
			u->state = unix_state_map[u->state-1];
			if (u->type == SOCK_DGRAM && u->state == SS_CLOSE && u->rport)
				u->state = SS_ESTABLISHED;
		}
		if (unix_type_skip(u, f) ||
		    !(f->states & (1 << u->state))) {
			free(u);
			continue;
		}

		if (!newformat) {
			u->rport = 0;
			u->rq = 0;
			u->wq = 0;
		}

		if (name[0]) {
			u->name = strdup(name);
			if (!u->name) {
				free(u);
				break;
			}
		}

		if (u->rport) {
			struct sockstat *p;

			for (p = list; p; p = p->next) {
				if (u->rport == p->lport)
					break;
			}
			if (!p)
				u->peer_name = "?";
			else
				u->peer_name = p->name ? : "*";
		}

		if (f->f) {
			struct sockstat st = {
				.local.family = AF_UNIX,
				.remote.family = AF_UNIX,
			};

			memcpy(st.local.data, &u->name, sizeof(u->name));
			/* when parsing the old format rport is set to 0 and
			 * therefore peer_name remains NULL
			 */
			if (u->peer_name && strcmp(u->peer_name, "*"))
				memcpy(st.remote.data, &u->peer_name,
				       sizeof(u->peer_name));
			if (run_ssfilter(f->f, &st) == 0) {
				free(u->name);
				free(u);
				continue;
			}
		}

		insp = &list;
		while (*insp) {
			if (u->type < (*insp)->type ||
			    (u->type == (*insp)->type &&
			     u->ino < (*insp)->ino))
				break;
			insp = &(*insp)->next;
		}
		u->next = *insp;
		*insp = u;

		if (++cnt > MAX_UNIX_REMEMBER) {
			while (list) {
				unix_stats_print(list, f);
				unix_list_drop_first(&list);
			}
			cnt = 0;
		}
	}
	fclose(fp);
	while (list) {
		unix_stats_print(list, f);
		unix_list_drop_first(&list);
	}

	return 0;
}

static int packet_stats_print(struct sockstat *s, const struct filter *f)
{
	const char *addr, *port;
	char ll_name[16];

	s->local.family = s->remote.family = AF_PACKET;

	if (f->f) {
		s->local.data[0] = s->prot;
		if (run_ssfilter(f->f, s) == 0)
			return 1;
	}

	sock_state_print(s);

	if (s->prot == 3)
		addr = "*";
	else
		addr = ll_proto_n2a(htons(s->prot), ll_name, sizeof(ll_name));

	if (s->iface == 0)
		port = "*";
	else
		port = xll_index_to_name(s->iface);

	sock_addr_print(addr, ":", port, NULL);
	sock_addr_print("", "*", "", NULL);

	proc_ctx_print(s);

	if (show_details)
		sock_details_print(s);

	return 0;
}

static void packet_show_ring(struct packet_diag_ring *ring)
{
	out("blk_size:%d", ring->pdr_block_size);
	out(",blk_nr:%d", ring->pdr_block_nr);
	out(",frm_size:%d", ring->pdr_frame_size);
	out(",frm_nr:%d", ring->pdr_frame_nr);
	out(",tmo:%d", ring->pdr_retire_tmo);
	out(",features:0x%x", ring->pdr_features);
}

static int packet_show_sock(struct nlmsghdr *nlh, void *arg)
{
	const struct filter *f = arg;
	struct packet_diag_msg *r = NLMSG_DATA(nlh);
	struct packet_diag_info *pinfo = NULL;
	struct packet_diag_ring *ring_rx = NULL, *ring_tx = NULL;
	struct rtattr *tb[PACKET_DIAG_MAX+1];
	struct sockstat stat = {};
	uint32_t fanout = 0;
	bool has_fanout = false;

	parse_rtattr(tb, PACKET_DIAG_MAX, (struct rtattr *)(r+1),
		     nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

	/* use /proc/net/packet if all info are not available */
	if (!tb[PACKET_DIAG_MEMINFO])
		return -1;

	stat.type   = r->pdiag_type;
	stat.prot   = r->pdiag_num;
	stat.ino    = r->pdiag_ino;
	stat.state  = SS_CLOSE;
	stat.sk	    = cookie_sk_get(&r->pdiag_cookie[0]);

	if (tb[PACKET_DIAG_MEMINFO]) {
		__u32 *skmeminfo = RTA_DATA(tb[PACKET_DIAG_MEMINFO]);

		stat.rq = skmeminfo[SK_MEMINFO_RMEM_ALLOC];
	}

	if (tb[PACKET_DIAG_INFO]) {
		pinfo = RTA_DATA(tb[PACKET_DIAG_INFO]);
		stat.lport = stat.iface = pinfo->pdi_index;
	}

	if (tb[PACKET_DIAG_UID])
		stat.uid = rta_getattr_u32(tb[PACKET_DIAG_UID]);

	if (tb[PACKET_DIAG_RX_RING])
		ring_rx = RTA_DATA(tb[PACKET_DIAG_RX_RING]);

	if (tb[PACKET_DIAG_TX_RING])
		ring_tx = RTA_DATA(tb[PACKET_DIAG_TX_RING]);

	if (tb[PACKET_DIAG_FANOUT]) {
		has_fanout = true;
		fanout = rta_getattr_u32(tb[PACKET_DIAG_FANOUT]);
	}

	if (packet_stats_print(&stat, f))
		return 0;

	if (show_details) {
		if (pinfo) {
			if (oneline)
				out(" ver:%d", pinfo->pdi_version);
			else
				out("\n\tver:%d", pinfo->pdi_version);
			out(" cpy_thresh:%d", pinfo->pdi_copy_thresh);
			out(" flags( ");
			if (pinfo->pdi_flags & PDI_RUNNING)
				out("running");
			if (pinfo->pdi_flags & PDI_AUXDATA)
				out(" auxdata");
			if (pinfo->pdi_flags & PDI_ORIGDEV)
				out(" origdev");
			if (pinfo->pdi_flags & PDI_VNETHDR)
				out(" vnethdr");
			if (pinfo->pdi_flags & PDI_LOSS)
				out(" loss");
			if (!pinfo->pdi_flags)
				out("0");
			out(" )");
		}
		if (ring_rx) {
			if (oneline)
				out(" ring_rx(");
			else
				out("\n\tring_rx(");
			packet_show_ring(ring_rx);
			out(")");
		}
		if (ring_tx) {
			if (oneline)
				out(" ring_tx(");
			else
				out("\n\tring_tx(");
			packet_show_ring(ring_tx);
			out(")");
		}
		if (has_fanout) {
			uint16_t type = (fanout >> 16) & 0xffff;

			if (oneline)
				out(" fanout(");
			else
				out("\n\tfanout(");
			out("id:%d,", fanout & 0xffff);
			out("type:");

			if (type == 0)
				out("hash");
			else if (type == 1)
				out("lb");
			else if (type == 2)
				out("cpu");
			else if (type == 3)
				out("roll");
			else if (type == 4)
				out("random");
			else if (type == 5)
				out("qm");
			else
				out("0x%x", type);

			out(")");
		}
	}

	if (show_bpf && tb[PACKET_DIAG_FILTER]) {
		struct sock_filter *fil =
		       RTA_DATA(tb[PACKET_DIAG_FILTER]);
		int num = RTA_PAYLOAD(tb[PACKET_DIAG_FILTER]) /
			  sizeof(struct sock_filter);

		if (oneline)
			out(" bpf filter (%d): ", num);
		else
			out("\n\tbpf filter (%d): ", num);
		while (num) {
			out(" 0x%02x %u %u %u,",
			    fil->code, fil->jt, fil->jf, fil->k);
			num--;
			fil++;
		}
	}

	if (show_mem)
		print_skmeminfo(tb, PACKET_DIAG_MEMINFO);
	return 0;
}

static int packet_show_netlink(struct filter *f)
{
	DIAG_REQUEST(req, struct packet_diag_req r);

	req.r.sdiag_family = AF_PACKET;
	req.r.pdiag_show = PACKET_SHOW_INFO | PACKET_SHOW_MEMINFO |
		PACKET_SHOW_FILTER | PACKET_SHOW_RING_CFG | PACKET_SHOW_FANOUT;

	return handle_netlink_request(f, &req.nlh, sizeof(req), packet_show_sock);
}

static int packet_show_line(char *buf, const struct filter *f, int fam)
{
	unsigned long long sk;
	struct sockstat stat = {};
	int type, prot, iface, state, rq, uid, ino;

	sscanf(buf, "%llx %*d %d %x %d %d %u %u %u",
			&sk,
			&type, &prot, &iface, &state,
			&rq, &uid, &ino);

	if (stat.type == SOCK_RAW && !(f->dbs&(1<<PACKET_R_DB)))
		return 0;
	if (stat.type == SOCK_DGRAM && !(f->dbs&(1<<PACKET_DG_DB)))
		return 0;

	stat.type  = type;
	stat.prot  = prot;
	stat.lport = stat.iface = iface;
	stat.state = state;
	stat.rq    = rq;
	stat.uid   = uid;
	stat.ino   = ino;
	stat.state = SS_CLOSE;

	if (packet_stats_print(&stat, f))
		return 0;

	return 0;
}

static int packet_show(struct filter *f)
{
	FILE *fp;
	int rc = 0;

	if (!filter_af_get(f, AF_PACKET) || !(f->states & (1 << SS_CLOSE)))
		return 0;

	if (!getenv("PROC_NET_PACKET") && !getenv("PROC_ROOT") &&
			packet_show_netlink(f) == 0)
		return 0;

	if ((fp = net_packet_open()) == NULL)
		return -1;
	if (generic_record_read(fp, packet_show_line, f, AF_PACKET))
		rc = -1;

	fclose(fp);
	return rc;
}

static int xdp_stats_print(struct sockstat *s, const struct filter *f)
{
	const char *addr, *port;
	char q_str[16];

	s->local.family = s->remote.family = AF_XDP;

	if (f->f) {
		if (run_ssfilter(f->f, s) == 0)
			return 1;
	}

	sock_state_print(s);

	if (s->iface) {
		addr = xll_index_to_name(s->iface);
		snprintf(q_str, sizeof(q_str), "q%d", s->lport);
		port = q_str;
		sock_addr_print(addr, ":", port, NULL);
	} else {
		sock_addr_print("", "*", "", NULL);
	}

	sock_addr_print("", "*", "", NULL);

	proc_ctx_print(s);

	if (show_details)
		sock_details_print(s);

	return 0;
}

static void xdp_show_ring(const char *name, struct xdp_diag_ring *ring)
{
	if (oneline)
		out(" %s(", name);
	else
		out("\n\t%s(", name);
	out("entries:%u", ring->entries);
	out(")");
}

static void xdp_show_umem(struct xdp_diag_umem *umem, struct xdp_diag_ring *fr,
			  struct xdp_diag_ring *cr)
{
	if (oneline)
		out(" tumem(");
	else
		out("\n\tumem(");
	out("id:%u", umem->id);
	out(",size:%llu", umem->size);
	out(",num_pages:%u", umem->num_pages);
	out(",chunk_size:%u", umem->chunk_size);
	out(",headroom:%u", umem->headroom);
	out(",ifindex:%u", umem->ifindex);
	out(",qid:%u", umem->queue_id);
	out(",zc:%u", umem->flags & XDP_DU_F_ZEROCOPY);
	out(",refs:%u", umem->refs);
	out(")");

	if (fr)
		xdp_show_ring("fr", fr);
	if (cr)
		xdp_show_ring("cr", cr);
}

static void xdp_show_stats(struct xdp_diag_stats *stats)
{
	if (oneline)
		out(" stats(");
	else
		out("\n\tstats(");
	out("rx dropped:%llu", stats->n_rx_dropped);
	out(",rx invalid:%llu", stats->n_rx_invalid);
	out(",rx queue full:%llu", stats->n_rx_full);
	out(",rx fill ring empty:%llu", stats->n_fill_ring_empty);
	out(",tx invalid:%llu", stats->n_tx_invalid);
	out(",tx ring empty:%llu", stats->n_tx_ring_empty);
	out(")");
}

static int xdp_show_sock(struct nlmsghdr *nlh, void *arg)
{
	struct xdp_diag_ring *rx = NULL, *tx = NULL, *fr = NULL, *cr = NULL;
	struct xdp_diag_msg *msg = NLMSG_DATA(nlh);
	struct rtattr *tb[XDP_DIAG_MAX + 1];
	struct xdp_diag_info *info = NULL;
	struct xdp_diag_umem *umem = NULL;
	struct xdp_diag_stats *stats = NULL;
	const struct filter *f = arg;
	struct sockstat stat = {};

	parse_rtattr(tb, XDP_DIAG_MAX, (struct rtattr *)(msg + 1),
		     nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*msg)));

	stat.type = msg->xdiag_type;
	stat.ino = msg->xdiag_ino;
	stat.state = SS_CLOSE;
	stat.sk = cookie_sk_get(&msg->xdiag_cookie[0]);

	if (tb[XDP_DIAG_INFO]) {
		info = RTA_DATA(tb[XDP_DIAG_INFO]);
		stat.iface = info->ifindex;
		stat.lport = info->queue_id;
	}

	if (tb[XDP_DIAG_UID])
		stat.uid = rta_getattr_u32(tb[XDP_DIAG_UID]);
	if (tb[XDP_DIAG_RX_RING])
		rx = RTA_DATA(tb[XDP_DIAG_RX_RING]);
	if (tb[XDP_DIAG_TX_RING])
		tx = RTA_DATA(tb[XDP_DIAG_TX_RING]);
	if (tb[XDP_DIAG_UMEM])
		umem = RTA_DATA(tb[XDP_DIAG_UMEM]);
	if (tb[XDP_DIAG_UMEM_FILL_RING])
		fr = RTA_DATA(tb[XDP_DIAG_UMEM_FILL_RING]);
	if (tb[XDP_DIAG_UMEM_COMPLETION_RING])
		cr = RTA_DATA(tb[XDP_DIAG_UMEM_COMPLETION_RING]);
	if (tb[XDP_DIAG_MEMINFO]) {
		__u32 *skmeminfo = RTA_DATA(tb[XDP_DIAG_MEMINFO]);

		stat.rq = skmeminfo[SK_MEMINFO_RMEM_ALLOC];
	}
	if (tb[XDP_DIAG_STATS])
		stats = RTA_DATA(tb[XDP_DIAG_STATS]);

	if (xdp_stats_print(&stat, f))
		return 0;

	if (show_details) {
		if (rx)
			xdp_show_ring("rx", rx);
		if (tx)
			xdp_show_ring("tx", tx);
		if (umem)
			xdp_show_umem(umem, fr, cr);
		if (stats)
			xdp_show_stats(stats);
	}

	if (show_mem)
		print_skmeminfo(tb, XDP_DIAG_MEMINFO); // really?


	return 0;
}

static int xdp_show(struct filter *f)
{
	DIAG_REQUEST(req, struct xdp_diag_req r);

	if (!filter_af_get(f, AF_XDP) || !(f->states & (1 << SS_CLOSE)))
		return 0;

	req.r.sdiag_family = AF_XDP;
	req.r.xdiag_show = XDP_SHOW_INFO | XDP_SHOW_RING_CFG | XDP_SHOW_UMEM |
			   XDP_SHOW_MEMINFO | XDP_SHOW_STATS;

	return handle_netlink_request(f, &req.nlh, sizeof(req), xdp_show_sock);
}

static int netlink_show_one(struct filter *f,
				int prot, int pid, unsigned int groups,
				int state, int dst_pid, unsigned int dst_group,
				int rq, int wq,
				unsigned long long sk, unsigned long long cb)
{
	struct sockstat st = {
		.state		= SS_CLOSE,
		.rq		= rq,
		.wq		= wq,
		.local.family	= AF_NETLINK,
		.remote.family	= AF_NETLINK,
	};

	SPRINT_BUF(prot_buf) = {};
	const char *prot_name;
	char procname[64] = {};

	if (f->f) {
		st.rport = -1;
		st.lport = pid;
		st.local.data[0] = prot;
		if (run_ssfilter(f->f, &st) == 0)
			return 1;
	}

	sock_state_print(&st);

	prot_name = nl_proto_n2a(prot, prot_buf, sizeof(prot_buf));

	if (pid == -1) {
		procname[0] = '*';
	} else if (!numeric) {
		int done = 0;

		if (!pid) {
			done = 1;
			strncpy(procname, "kernel", 7);
		} else if (pid > 0) {
			FILE *fp;

			snprintf(procname, sizeof(procname), "%s/%d/stat",
				getenv("PROC_ROOT") ? : "/proc", pid);
			if ((fp = fopen(procname, "r")) != NULL) {
				if (fscanf(fp, "%*d (%[^)])", procname) == 1) {
					snprintf(procname+strlen(procname),
						sizeof(procname)-strlen(procname),
						"/%d", pid);
					done = 1;
				}
				fclose(fp);
			}
		}
		if (!done)
			int_to_str(pid, procname);
	} else {
		int_to_str(pid, procname);
	}

	sock_addr_print(prot_name, ":", procname, NULL);

	if (state == NETLINK_CONNECTED) {
		char dst_group_buf[30];
		char dst_pid_buf[30];

		sock_addr_print(int_to_str(dst_group, dst_group_buf), ":",
				int_to_str(dst_pid, dst_pid_buf), NULL);
	} else {
		sock_addr_print("", "*", "", NULL);
	}

	char *pid_context = NULL;

	if (show_proc_ctx) {
		/* The pid value will either be:
		 *   0 if destination kernel - show kernel initial context.
		 *   A valid process pid - use getpidcon.
		 *   A unique value allocated by the kernel or netlink user
		 *   to the process - show context as "not available".
		 */
		if (!pid)
			security_get_initial_context("kernel", &pid_context);
		else if (pid > 0)
			getpidcon(pid, &pid_context);

		out(" proc_ctx=%s", pid_context ? : "unavailable");
		free(pid_context);
	}

	if (show_details) {
		out(" sk=%llx cb=%llx groups=0x%08x", sk, cb, groups);
	}

	return 0;
}

static int netlink_show_sock(struct nlmsghdr *nlh, void *arg)
{
	struct filter *f = (struct filter *)arg;
	struct netlink_diag_msg *r = NLMSG_DATA(nlh);
	struct rtattr *tb[NETLINK_DIAG_MAX+1];
	int rq = 0, wq = 0;
	unsigned long groups = 0;

	parse_rtattr(tb, NETLINK_DIAG_MAX, (struct rtattr *)(r+1),
		     nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

	if (tb[NETLINK_DIAG_GROUPS] && RTA_PAYLOAD(tb[NETLINK_DIAG_GROUPS]))
		groups = *(unsigned long *) RTA_DATA(tb[NETLINK_DIAG_GROUPS]);

	if (tb[NETLINK_DIAG_MEMINFO]) {
		const __u32 *skmeminfo;

		skmeminfo = RTA_DATA(tb[NETLINK_DIAG_MEMINFO]);

		rq = skmeminfo[SK_MEMINFO_RMEM_ALLOC];
		wq = skmeminfo[SK_MEMINFO_WMEM_ALLOC];
	}

	if (netlink_show_one(f, r->ndiag_protocol, r->ndiag_portid, groups,
			 r->ndiag_state, r->ndiag_dst_portid, r->ndiag_dst_group,
			 rq, wq, 0, 0)) {
		return 0;
	}

	if (show_mem) {
		out("\t");
		print_skmeminfo(tb, NETLINK_DIAG_MEMINFO);
	}

	return 0;
}

static int netlink_show_netlink(struct filter *f)
{
	DIAG_REQUEST(req, struct netlink_diag_req r);

	req.r.sdiag_family = AF_NETLINK;
	req.r.sdiag_protocol = NDIAG_PROTO_ALL;
	req.r.ndiag_show = NDIAG_SHOW_GROUPS | NDIAG_SHOW_MEMINFO;

	return handle_netlink_request(f, &req.nlh, sizeof(req), netlink_show_sock);
}

static int netlink_show(struct filter *f)
{
	FILE *fp;
	char buf[256];
	int prot, pid;
	unsigned int groups;
	int rq, wq, rc;
	unsigned long long sk, cb;

	if (!filter_af_get(f, AF_NETLINK) || !(f->states & (1 << SS_CLOSE)))
		return 0;

	if (!getenv("PROC_NET_NETLINK") && !getenv("PROC_ROOT") &&
		netlink_show_netlink(f) == 0)
		return 0;

	if ((fp = net_netlink_open()) == NULL)
		return -1;
	if (!fgets(buf, sizeof(buf), fp)) {
		fclose(fp);
		return -1;
	}

	while (fgets(buf, sizeof(buf), fp)) {
		sscanf(buf, "%llx %d %d %x %d %d %llx %d",
		       &sk,
		       &prot, &pid, &groups, &rq, &wq, &cb, &rc);

		netlink_show_one(f, prot, pid, groups, 0, 0, 0, rq, wq, sk, cb);
	}

	fclose(fp);
	return 0;
}

static bool vsock_type_skip(struct sockstat *s, struct filter *f)
{
	if (s->type == SOCK_STREAM && !(f->dbs & (1 << VSOCK_ST_DB)))
		return true;
	if (s->type == SOCK_DGRAM && !(f->dbs & (1 << VSOCK_DG_DB)))
		return true;
	return false;
}

static void vsock_addr_print(inet_prefix *a, __u32 port)
{
	char cid_str[sizeof("4294967295")];
	char port_str[sizeof("4294967295")];
	__u32 cid;

	memcpy(&cid, a->data, sizeof(cid));

	if (cid == ~(__u32)0)
		snprintf(cid_str, sizeof(cid_str), "*");
	else
		snprintf(cid_str, sizeof(cid_str), "%u", cid);

	if (port == ~(__u32)0)
		snprintf(port_str, sizeof(port_str), "*");
	else
		snprintf(port_str, sizeof(port_str), "%u", port);

	sock_addr_print(cid_str, ":", port_str, NULL);
}

static void vsock_stats_print(struct sockstat *s, struct filter *f)
{
	sock_state_print(s);

	vsock_addr_print(&s->local, s->lport);
	vsock_addr_print(&s->remote, s->rport);

	proc_ctx_print(s);
}

static int vsock_show_sock(struct nlmsghdr *nlh, void *arg)
{
	struct filter *f = (struct filter *)arg;
	struct vsock_diag_msg *r = NLMSG_DATA(nlh);
	struct sockstat stat = {
		.type = r->vdiag_type,
		.lport = r->vdiag_src_port,
		.rport = r->vdiag_dst_port,
		.state = r->vdiag_state,
		.ino = r->vdiag_ino,
	};

	vsock_set_inet_prefix(&stat.local, r->vdiag_src_cid);
	vsock_set_inet_prefix(&stat.remote, r->vdiag_dst_cid);

	if (vsock_type_skip(&stat, f))
		return 0;

	if (f->f && run_ssfilter(f->f, &stat) == 0)
		return 0;

	vsock_stats_print(&stat, f);

	return 0;
}

static int vsock_show(struct filter *f)
{
	DIAG_REQUEST(req, struct vsock_diag_req r);

	if (!filter_af_get(f, AF_VSOCK))
		return 0;

	req.r.sdiag_family = AF_VSOCK;
	req.r.vdiag_states = f->states;

	return handle_netlink_request(f, &req.nlh, sizeof(req), vsock_show_sock);
}

static void tipc_sock_addr_print(struct rtattr *net_addr, struct rtattr *id)
{
	uint32_t node = rta_getattr_u32(net_addr);
	uint32_t identity = rta_getattr_u32(id);

	SPRINT_BUF(addr) = {};
	SPRINT_BUF(port) = {};

	sprintf(addr, "%u", node);
	sprintf(port, "%u", identity);
	sock_addr_print(addr, ":", port, NULL);

}

static int tipc_show_sock(struct nlmsghdr *nlh, void *arg)
{
	struct rtattr *stat[TIPC_NLA_SOCK_STAT_MAX + 1] = {};
	struct rtattr *attrs[TIPC_NLA_SOCK_MAX + 1] = {};
	struct rtattr *con[TIPC_NLA_CON_MAX + 1] = {};
	struct rtattr *info[TIPC_NLA_MAX + 1] = {};
	struct rtattr *msg_ref;
	struct sockstat ss = {};

	parse_rtattr(info, TIPC_NLA_MAX, NLMSG_DATA(nlh),
		     NLMSG_PAYLOAD(nlh, 0));

	if (!info[TIPC_NLA_SOCK])
		return 0;

	msg_ref = info[TIPC_NLA_SOCK];
	parse_rtattr(attrs, TIPC_NLA_SOCK_MAX, RTA_DATA(msg_ref),
		     RTA_PAYLOAD(msg_ref));

	msg_ref = attrs[TIPC_NLA_SOCK_STAT];
	parse_rtattr(stat, TIPC_NLA_SOCK_STAT_MAX,
		     RTA_DATA(msg_ref), RTA_PAYLOAD(msg_ref));


	ss.local.family = AF_TIPC;
	ss.type = rta_getattr_u32(attrs[TIPC_NLA_SOCK_TYPE]);
	ss.state = rta_getattr_u32(attrs[TIPC_NLA_SOCK_TIPC_STATE]);
	ss.uid = rta_getattr_u32(attrs[TIPC_NLA_SOCK_UID]);
	ss.ino = rta_getattr_u32(attrs[TIPC_NLA_SOCK_INO]);
	ss.rq = rta_getattr_u32(stat[TIPC_NLA_SOCK_STAT_RCVQ]);
	ss.wq = rta_getattr_u32(stat[TIPC_NLA_SOCK_STAT_SENDQ]);
	ss.sk = rta_getattr_u64(attrs[TIPC_NLA_SOCK_COOKIE]);

	sock_state_print (&ss);

	tipc_sock_addr_print(attrs[TIPC_NLA_SOCK_ADDR],
			     attrs[TIPC_NLA_SOCK_REF]);

	msg_ref = attrs[TIPC_NLA_SOCK_CON];
	if (msg_ref) {
		parse_rtattr(con, TIPC_NLA_CON_MAX,
			     RTA_DATA(msg_ref), RTA_PAYLOAD(msg_ref));

		tipc_sock_addr_print(con[TIPC_NLA_CON_NODE],
				     con[TIPC_NLA_CON_SOCK]);
	} else
		sock_addr_print("", "-", "", NULL);

	if (show_details)
		sock_details_print(&ss);

	proc_ctx_print(&ss);

	if (show_tipcinfo) {
		if (oneline)
			out(" type:%s", stype_nameg[ss.type]);
		else
			out("\n type:%s", stype_nameg[ss.type]);
		out(" cong:%s ",
		       stat[TIPC_NLA_SOCK_STAT_LINK_CONG] ? "link" :
		       stat[TIPC_NLA_SOCK_STAT_CONN_CONG] ? "conn" : "none");
		out(" drop:%d ",
		       rta_getattr_u32(stat[TIPC_NLA_SOCK_STAT_DROP]));

		if (attrs[TIPC_NLA_SOCK_HAS_PUBL])
			out(" publ");

		if (con[TIPC_NLA_CON_FLAG])
			out(" via {%u,%u} ",
			       rta_getattr_u32(con[TIPC_NLA_CON_TYPE]),
			       rta_getattr_u32(con[TIPC_NLA_CON_INST]));
	}

	return 0;
}

static int tipc_show(struct filter *f)
{
	DIAG_REQUEST(req, struct tipc_sock_diag_req r);

	memset(&req.r, 0, sizeof(req.r));
	req.r.sdiag_family = AF_TIPC;
	req.r.tidiag_states = f->states;

	return handle_netlink_request(f, &req.nlh, sizeof(req), tipc_show_sock);
}

struct sock_diag_msg {
	__u8 sdiag_family;
};

static int generic_show_sock(struct nlmsghdr *nlh, void *arg)
{
	struct sock_diag_msg *r = NLMSG_DATA(nlh);
	struct inet_diag_arg inet_arg = { .f = arg, .protocol = IPPROTO_MAX };
	int ret;

	switch (r->sdiag_family) {
	case AF_INET:
	case AF_INET6:
		inet_arg.rth = inet_arg.f->rth_for_killing;
		ret = show_one_inet_sock(nlh, &inet_arg);
		break;
	case AF_UNIX:
		ret = unix_show_sock(nlh, arg);
		break;
	case AF_PACKET:
		ret = packet_show_sock(nlh, arg);
		break;
	case AF_NETLINK:
		ret = netlink_show_sock(nlh, arg);
		break;
	case AF_VSOCK:
		ret = vsock_show_sock(nlh, arg);
		break;
	case AF_XDP:
		ret = xdp_show_sock(nlh, arg);
		break;
	default:
		ret = -1;
	}

	render();

	return ret;
}

static int handle_follow_request(struct filter *f)
{
	int ret = 0;
	int groups = 0;
	struct rtnl_handle rth, rth2;

	if (f->families & FAMILY_MASK(AF_INET) && f->dbs & (1 << TCP_DB))
		groups |= 1 << (SKNLGRP_INET_TCP_DESTROY - 1);
	if (f->families & FAMILY_MASK(AF_INET) && f->dbs & (1 << UDP_DB))
		groups |= 1 << (SKNLGRP_INET_UDP_DESTROY - 1);
	if (f->families & FAMILY_MASK(AF_INET6) && f->dbs & (1 << TCP_DB))
		groups |= 1 << (SKNLGRP_INET6_TCP_DESTROY - 1);
	if (f->families & FAMILY_MASK(AF_INET6) && f->dbs & (1 << UDP_DB))
		groups |= 1 << (SKNLGRP_INET6_UDP_DESTROY - 1);

	if (groups == 0)
		return -1;

	if (rtnl_open_byproto(&rth, groups, NETLINK_SOCK_DIAG))
		return -1;

	rth.dump = 0;
	rth.local.nl_pid = 0;

	if (f->kill) {
		if (rtnl_open_byproto(&rth2, groups, NETLINK_SOCK_DIAG)) {
			rtnl_close(&rth);
			return -1;
		}
		f->rth_for_killing = &rth2;
	}

	if (rtnl_dump_filter(&rth, generic_show_sock, f))
		ret = -1;

	rtnl_close(&rth);
	if (f->rth_for_killing)
		rtnl_close(f->rth_for_killing);
	return ret;
}

static int get_snmp_int(char *proto, char *key, int *result)
{
	char buf[1024];
	FILE *fp;
	int protolen = strlen(proto);
	int keylen = strlen(key);

	*result = 0;

	if ((fp = net_snmp_open()) == NULL)
		return -1;

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		char *p = buf;
		int  pos = 0;

		if (memcmp(buf, proto, protolen))
			continue;
		while ((p = strchr(p, ' ')) != NULL) {
			pos++;
			p++;
			if (memcmp(p, key, keylen) == 0 &&
			    (p[keylen] == ' ' || p[keylen] == '\n'))
				break;
		}
		if (fgets(buf, sizeof(buf), fp) == NULL)
			break;
		if (memcmp(buf, proto, protolen))
			break;
		p = buf;
		while ((p = strchr(p, ' ')) != NULL) {
			p++;
			if (--pos == 0) {
				sscanf(p, "%d", result);
				fclose(fp);
				return 0;
			}
		}
	}

	fclose(fp);
	errno = ESRCH;
	return -1;
}


/* Get stats from sockstat */

struct ssummary {
	int socks;
	int tcp_mem;
	int tcp_total;
	int tcp_orphans;
	int tcp_tws;
	int tcp4_hashed;
	int udp4;
	int raw4;
	int frag4;
	int frag4_mem;
	int tcp6_hashed;
	int udp6;
	int raw6;
	int frag6;
	int frag6_mem;
};

static void get_sockstat_line(char *line, struct ssummary *s)
{
	char id[256], rem[256];

	if (sscanf(line, "%[^ ] %[^\n]\n", id, rem) != 2)
		return;

	if (strcmp(id, "sockets:") == 0)
		sscanf(rem, "%*s%d", &s->socks);
	else if (strcmp(id, "UDP:") == 0)
		sscanf(rem, "%*s%d", &s->udp4);
	else if (strcmp(id, "UDP6:") == 0)
		sscanf(rem, "%*s%d", &s->udp6);
	else if (strcmp(id, "RAW:") == 0)
		sscanf(rem, "%*s%d", &s->raw4);
	else if (strcmp(id, "RAW6:") == 0)
		sscanf(rem, "%*s%d", &s->raw6);
	else if (strcmp(id, "TCP6:") == 0)
		sscanf(rem, "%*s%d", &s->tcp6_hashed);
	else if (strcmp(id, "FRAG:") == 0)
		sscanf(rem, "%*s%d%*s%d", &s->frag4, &s->frag4_mem);
	else if (strcmp(id, "FRAG6:") == 0)
		sscanf(rem, "%*s%d%*s%d", &s->frag6, &s->frag6_mem);
	else if (strcmp(id, "TCP:") == 0)
		sscanf(rem, "%*s%d%*s%d%*s%d%*s%d%*s%d",
		       &s->tcp4_hashed,
		       &s->tcp_orphans, &s->tcp_tws, &s->tcp_total, &s->tcp_mem);
}

static int get_sockstat(struct ssummary *s)
{
	char buf[256];
	FILE *fp;

	memset(s, 0, sizeof(*s));

	if ((fp = net_sockstat_open()) == NULL)
		return -1;
	while (fgets(buf, sizeof(buf), fp) != NULL)
		get_sockstat_line(buf, s);
	fclose(fp);

	if ((fp = net_sockstat6_open()) == NULL)
		return 0;
	while (fgets(buf, sizeof(buf), fp) != NULL)
		get_sockstat_line(buf, s);
	fclose(fp);

	return 0;
}

static int print_summary(void)
{
	struct ssummary s;
	int tcp_estab;

	if (get_sockstat(&s) < 0)
		perror("ss: get_sockstat");
	if (get_snmp_int("Tcp:", "CurrEstab", &tcp_estab) < 0)
		perror("ss: get_snmpstat");

	printf("Total: %d\n", s.socks);

	printf("TCP:   %d (estab %d, closed %d, orphaned %d, timewait %d)\n",
	       s.tcp_total + s.tcp_tws, tcp_estab,
	       s.tcp_total - (s.tcp4_hashed + s.tcp6_hashed - s.tcp_tws),
	       s.tcp_orphans, s.tcp_tws);

	printf("\n");
	printf("Transport Total     IP        IPv6\n");
	printf("RAW	  %-9d %-9d %-9d\n", s.raw4+s.raw6, s.raw4, s.raw6);
	printf("UDP	  %-9d %-9d %-9d\n", s.udp4+s.udp6, s.udp4, s.udp6);
	printf("TCP	  %-9d %-9d %-9d\n", s.tcp4_hashed+s.tcp6_hashed, s.tcp4_hashed, s.tcp6_hashed);
	printf("INET	  %-9d %-9d %-9d\n",
	       s.raw4+s.udp4+s.tcp4_hashed+
	       s.raw6+s.udp6+s.tcp6_hashed,
	       s.raw4+s.udp4+s.tcp4_hashed,
	       s.raw6+s.udp6+s.tcp6_hashed);
	printf("FRAG	  %-9d %-9d %-9d\n", s.frag4+s.frag6, s.frag4, s.frag6);

	printf("\n");

	return 0;
}

static void _usage(FILE *dest)
{
	fprintf(dest,
"Usage: ss [ OPTIONS ]\n"
"       ss [ OPTIONS ] [ FILTER ]\n"
"   -h, --help          this message\n"
"   -V, --version       output version information\n"
"   -n, --numeric       don't resolve service names\n"
"   -r, --resolve       resolve host names\n"
"   -a, --all           display all sockets\n"
"   -l, --listening     display listening sockets\n"
"   -o, --options       show timer information\n"
"   -e, --extended      show detailed socket information\n"
"   -m, --memory        show socket memory usage\n"
"   -p, --processes     show process using socket\n"
"   -i, --info          show internal TCP information\n"
"       --tipcinfo      show internal tipc socket information\n"
"   -s, --summary       show socket usage summary\n"
"       --tos           show tos and priority information\n"
"       --cgroup        show cgroup information\n"
"   -b, --bpf           show bpf filter socket information\n"
"   -E, --events        continually display sockets as they are destroyed\n"
"   -Z, --context       display process SELinux security contexts\n"
"   -z, --contexts      display process and socket SELinux security contexts\n"
"   -N, --net           switch to the specified network namespace name\n"
"\n"
"   -4, --ipv4          display only IP version 4 sockets\n"
"   -6, --ipv6          display only IP version 6 sockets\n"
"   -0, --packet        display PACKET sockets\n"
"   -t, --tcp           display only TCP sockets\n"
"   -M, --mptcp         display only MPTCP sockets\n"
"   -S, --sctp          display only SCTP sockets\n"
"   -u, --udp           display only UDP sockets\n"
"   -d, --dccp          display only DCCP sockets\n"
"   -w, --raw           display only RAW sockets\n"
"   -x, --unix          display only Unix domain sockets\n"
"       --tipc          display only TIPC sockets\n"
"       --vsock         display only vsock sockets\n"
"   -f, --family=FAMILY display sockets of type FAMILY\n"
"       FAMILY := {inet|inet6|link|unix|netlink|vsock|tipc|xdp|help}\n"
"\n"
"   -K, --kill          forcibly close sockets, display what was closed\n"
"   -H, --no-header     Suppress header line\n"
"   -O, --oneline       socket's data printed on a single line\n"
"       --inet-sockopt  show various inet socket options\n"
"\n"
"   -A, --query=QUERY, --socket=QUERY\n"
"       QUERY := {all|inet|tcp|mptcp|udp|raw|unix|unix_dgram|unix_stream|unix_seqpacket|packet|netlink|vsock_stream|vsock_dgram|tipc}[,QUERY]\n"
"\n"
"   -D, --diag=FILE     Dump raw information about TCP sockets to FILE\n"
"   -F, --filter=FILE   read filter information from FILE\n"
"       FILTER := [ state STATE-FILTER ] [ EXPRESSION ]\n"
"       STATE-FILTER := {all|connected|synchronized|bucket|big|TCP-STATES}\n"
"         TCP-STATES := {established|syn-sent|syn-recv|fin-wait-{1,2}|time-wait|closed|close-wait|last-ack|listening|closing}\n"
"          connected := {established|syn-sent|syn-recv|fin-wait-{1,2}|time-wait|close-wait|last-ack|closing}\n"
"       synchronized := {established|syn-recv|fin-wait-{1,2}|time-wait|close-wait|last-ack|closing}\n"
"             bucket := {syn-recv|time-wait}\n"
"                big := {established|syn-sent|fin-wait-{1,2}|closed|close-wait|last-ack|listening|closing}\n"
		);
}

static void help(void) __attribute__((noreturn));
static void help(void)
{
	_usage(stdout);
	exit(0);
}

static void usage(void) __attribute__((noreturn));
static void usage(void)
{
	_usage(stderr);
	exit(-1);
}


static int scan_state(const char *state)
{
	static const char * const sstate_namel[] = {
		"UNKNOWN",
		[SS_ESTABLISHED] = "established",
		[SS_SYN_SENT] = "syn-sent",
		[SS_SYN_RECV] = "syn-recv",
		[SS_FIN_WAIT1] = "fin-wait-1",
		[SS_FIN_WAIT2] = "fin-wait-2",
		[SS_TIME_WAIT] = "time-wait",
		[SS_CLOSE] = "unconnected",
		[SS_CLOSE_WAIT] = "close-wait",
		[SS_LAST_ACK] = "last-ack",
		[SS_LISTEN] =	"listening",
		[SS_CLOSING] = "closing",
	};
	int i;

	if (strcasecmp(state, "close") == 0 ||
	    strcasecmp(state, "closed") == 0)
		return (1<<SS_CLOSE);
	if (strcasecmp(state, "syn-rcv") == 0)
		return (1<<SS_SYN_RECV);
	if (strcasecmp(state, "established") == 0)
		return (1<<SS_ESTABLISHED);
	if (strcasecmp(state, "all") == 0)
		return SS_ALL;
	if (strcasecmp(state, "connected") == 0)
		return SS_ALL & ~((1<<SS_CLOSE)|(1<<SS_LISTEN));
	if (strcasecmp(state, "synchronized") == 0)
		return SS_ALL & ~((1<<SS_CLOSE)|(1<<SS_LISTEN)|(1<<SS_SYN_SENT));
	if (strcasecmp(state, "bucket") == 0)
		return (1<<SS_SYN_RECV)|(1<<SS_TIME_WAIT);
	if (strcasecmp(state, "big") == 0)
		return SS_ALL & ~((1<<SS_SYN_RECV)|(1<<SS_TIME_WAIT));
	for (i = 0; i < SS_MAX; i++) {
		if (strcasecmp(state, sstate_namel[i]) == 0)
			return (1<<i);
	}

	fprintf(stderr, "ss: wrong state name: %s\n", state);
	exit(-1);
}

/* Values 'v' and 'V' are already used so a non-character is used */
#define OPT_VSOCK 256

/* Values of 't' are already used so a non-character is used */
#define OPT_TIPCSOCK 257
#define OPT_TIPCINFO 258

#define OPT_TOS 259

/* Values of 'x' are already used so a non-character is used */
#define OPT_XDPSOCK 260

#define OPT_CGROUP 261

#define OPT_INET_SOCKOPT 262

static const struct option long_opts[] = {
	{ "numeric", 0, 0, 'n' },
	{ "resolve", 0, 0, 'r' },
	{ "options", 0, 0, 'o' },
	{ "extended", 0, 0, 'e' },
	{ "memory", 0, 0, 'm' },
	{ "info", 0, 0, 'i' },
	{ "processes", 0, 0, 'p' },
	{ "bpf", 0, 0, 'b' },
	{ "events", 0, 0, 'E' },
	{ "dccp", 0, 0, 'd' },
	{ "tcp", 0, 0, 't' },
	{ "sctp", 0, 0, 'S' },
	{ "udp", 0, 0, 'u' },
	{ "raw", 0, 0, 'w' },
	{ "unix", 0, 0, 'x' },
	{ "tipc", 0, 0, OPT_TIPCSOCK},
	{ "vsock", 0, 0, OPT_VSOCK },
	{ "all", 0, 0, 'a' },
	{ "listening", 0, 0, 'l' },
	{ "ipv4", 0, 0, '4' },
	{ "ipv6", 0, 0, '6' },
	{ "packet", 0, 0, '0' },
	{ "family", 1, 0, 'f' },
	{ "socket", 1, 0, 'A' },
	{ "query", 1, 0, 'A' },
	{ "summary", 0, 0, 's' },
	{ "diag", 1, 0, 'D' },
	{ "filter", 1, 0, 'F' },
	{ "version", 0, 0, 'V' },
	{ "help", 0, 0, 'h' },
	{ "context", 0, 0, 'Z' },
	{ "contexts", 0, 0, 'z' },
	{ "net", 1, 0, 'N' },
	{ "tipcinfo", 0, 0, OPT_TIPCINFO},
	{ "tos", 0, 0, OPT_TOS },
	{ "cgroup", 0, 0, OPT_CGROUP },
	{ "kill", 0, 0, 'K' },
	{ "no-header", 0, 0, 'H' },
	{ "xdp", 0, 0, OPT_XDPSOCK},
	{ "mptcp", 0, 0, 'M' },
	{ "oneline", 0, 0, 'O' },
	{ "inet-sockopt", 0, 0, OPT_INET_SOCKOPT },
	{ 0 }

};

int main(int argc, char *argv[])
{
	int saw_states = 0;
	int saw_query = 0;
	int do_summary = 0;
	const char *dump_tcpdiag = NULL;
	FILE *filter_fp = NULL;
	int ch;
	int state_filter = 0;

	while ((ch = getopt_long(argc, argv,
				 "dhaletuwxnro460spbEf:mMiA:D:F:vVzZN:KHSO",
				 long_opts, NULL)) != EOF) {
		switch (ch) {
		case 'n':
			numeric = 1;
			break;
		case 'r':
			resolve_hosts = 1;
			break;
		case 'o':
			show_options = 1;
			break;
		case 'e':
			show_options = 1;
			show_details++;
			break;
		case 'm':
			show_mem = 1;
			break;
		case 'i':
			show_tcpinfo = 1;
			break;
		case 'p':
			show_users++;
			user_ent_hash_build();
			break;
		case 'b':
			show_options = 1;
			show_bpf++;
			break;
		case 'E':
			follow_events = 1;
			break;
		case 'd':
			filter_db_set(&current_filter, DCCP_DB, true);
			break;
		case 't':
			filter_db_set(&current_filter, TCP_DB, true);
			break;
		case 'S':
			filter_db_set(&current_filter, SCTP_DB, true);
			break;
		case 'u':
			filter_db_set(&current_filter, UDP_DB, true);
			break;
		case 'w':
			filter_db_set(&current_filter, RAW_DB, true);
			break;
		case 'x':
			filter_af_set(&current_filter, AF_UNIX);
			break;
		case OPT_VSOCK:
			filter_af_set(&current_filter, AF_VSOCK);
			break;
		case OPT_TIPCSOCK:
			filter_af_set(&current_filter, AF_TIPC);
			break;
		case 'a':
			state_filter = SS_ALL;
			break;
		case 'l':
			state_filter = (1 << SS_LISTEN) | (1 << SS_CLOSE);
			break;
		case '4':
			filter_af_set(&current_filter, AF_INET);
			break;
		case '6':
			filter_af_set(&current_filter, AF_INET6);
			break;
		case '0':
			filter_af_set(&current_filter, AF_PACKET);
			break;
		case OPT_XDPSOCK:
			filter_af_set(&current_filter, AF_XDP);
			break;
		case 'M':
			filter_db_set(&current_filter, MPTCP_DB, true);
			break;
		case 'f':
			if (strcmp(optarg, "inet") == 0)
				filter_af_set(&current_filter, AF_INET);
			else if (strcmp(optarg, "inet6") == 0)
				filter_af_set(&current_filter, AF_INET6);
			else if (strcmp(optarg, "link") == 0)
				filter_af_set(&current_filter, AF_PACKET);
			else if (strcmp(optarg, "unix") == 0)
				filter_af_set(&current_filter, AF_UNIX);
			else if (strcmp(optarg, "netlink") == 0)
				filter_af_set(&current_filter, AF_NETLINK);
			else if (strcmp(optarg, "tipc") == 0)
				filter_af_set(&current_filter, AF_TIPC);
			else if (strcmp(optarg, "vsock") == 0)
				filter_af_set(&current_filter, AF_VSOCK);
			else if (strcmp(optarg, "xdp") == 0)
				filter_af_set(&current_filter, AF_XDP);
			else if (strcmp(optarg, "help") == 0)
				help();
			else {
				fprintf(stderr, "ss: \"%s\" is invalid family\n",
						optarg);
				usage();
			}
			break;
		case 'A':
		{
			char *p, *p1;

			if (!saw_query) {
				current_filter.dbs = 0;
				state_filter = state_filter ?
					       state_filter : SS_CONN;
				saw_query = 1;
				do_default = 0;
			}
			p = p1 = optarg;
			do {
				if ((p1 = strchr(p, ',')) != NULL)
					*p1 = 0;
				if (filter_db_parse(&current_filter, p)) {
					fprintf(stderr, "ss: \"%s\" is illegal socket table id\n", p);
					usage();
				}
				p = p1 + 1;
			} while (p1);
			break;
		}
		case 's':
			do_summary = 1;
			break;
		case 'D':
			dump_tcpdiag = optarg;
			break;
		case 'F':
			if (filter_fp) {
				fprintf(stderr, "More than one filter file\n");
				exit(-1);
			}
			if (optarg[0] == '-')
				filter_fp = stdin;
			else
				filter_fp = fopen(optarg, "r");
			if (!filter_fp) {
				perror("fopen filter file");
				exit(-1);
			}
			break;
		case 'v':
		case 'V':
			printf("ss utility, iproute2-%s\n", version);
			exit(0);
		case 'z':
			show_sock_ctx++;
			/* fall through */
		case 'Z':
			if (is_selinux_enabled() <= 0) {
				fprintf(stderr, "ss: SELinux is not enabled.\n");
				exit(1);
			}
			show_proc_ctx++;
			user_ent_hash_build();
			break;
		case 'N':
			if (netns_switch(optarg))
				exit(1);
			break;
		case OPT_TIPCINFO:
			show_tipcinfo = 1;
			break;
		case OPT_TOS:
			show_tos = 1;
			break;
		case OPT_CGROUP:
			show_cgroup = 1;
			break;
		case 'K':
			current_filter.kill = 1;
			break;
		case 'H':
			show_header = 0;
			break;
		case 'O':
			oneline = 1;
			break;
		case OPT_INET_SOCKOPT:
			show_inet_sockopt = 1;
			break;
		case 'h':
			help();
		case '?':
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (do_summary) {
		print_summary();
		if (do_default && argc == 0)
			exit(0);
	}

	while (argc > 0) {
		if (strcmp(*argv, "state") == 0) {
			NEXT_ARG();
			if (!saw_states)
				state_filter = 0;
			state_filter |= scan_state(*argv);
			saw_states = 1;
		} else if (strcmp(*argv, "exclude") == 0 ||
			   strcmp(*argv, "excl") == 0) {
			NEXT_ARG();
			if (!saw_states)
				state_filter = SS_ALL;
			state_filter &= ~scan_state(*argv);
			saw_states = 1;
		} else {
			break;
		}
		argc--; argv++;
	}

	if (do_default) {
		state_filter = state_filter ? state_filter : SS_CONN;
		filter_db_parse(&current_filter, "all");
	}

	filter_states_set(&current_filter, state_filter);
	filter_merge_defaults(&current_filter);

	if (!numeric && resolve_hosts &&
	    (current_filter.dbs & (UNIX_DBM|INET_L4_DBM)))
		init_service_resolver();

	if (current_filter.dbs == 0) {
		fprintf(stderr, "ss: no socket tables to show with such filter.\n");
		exit(0);
	}
	if (current_filter.families == 0) {
		fprintf(stderr, "ss: no families to show with such filter.\n");
		exit(0);
	}
	if (current_filter.states == 0) {
		fprintf(stderr, "ss: no socket states to show with such filter.\n");
		exit(0);
	}

	if (dump_tcpdiag) {
		FILE *dump_fp = stdout;

		if (!(current_filter.dbs & (1<<TCP_DB))) {
			fprintf(stderr, "ss: tcpdiag dump requested and no tcp in filter.\n");
			exit(0);
		}
		if (dump_tcpdiag[0] != '-') {
			dump_fp = fopen(dump_tcpdiag, "w");
			if (!dump_tcpdiag) {
				perror("fopen dump file");
				exit(-1);
			}
		}
		inet_show_netlink(&current_filter, dump_fp, IPPROTO_TCP);
		fflush(dump_fp);
		exit(0);
	}

	if (ssfilter_parse(&current_filter.f, argc, argv, filter_fp))
		usage();

	if (!(current_filter.dbs & (current_filter.dbs - 1)))
		columns[COL_NETID].disabled = 1;

	if (!(current_filter.states & (current_filter.states - 1)))
		columns[COL_STATE].disabled = 1;

	if (show_header)
		print_header();

	fflush(stdout);

	if (follow_events)
		exit(handle_follow_request(&current_filter));

	if (current_filter.dbs & (1<<NETLINK_DB))
		netlink_show(&current_filter);
	if (current_filter.dbs & PACKET_DBM)
		packet_show(&current_filter);
	if (current_filter.dbs & UNIX_DBM)
		unix_show(&current_filter);
	if (current_filter.dbs & (1<<RAW_DB))
		raw_show(&current_filter);
	if (current_filter.dbs & (1<<UDP_DB))
		udp_show(&current_filter);
	if (current_filter.dbs & (1<<TCP_DB))
		tcp_show(&current_filter);
	if (current_filter.dbs & (1<<DCCP_DB))
		dccp_show(&current_filter);
	if (current_filter.dbs & (1<<SCTP_DB))
		sctp_show(&current_filter);
	if (current_filter.dbs & VSOCK_DBM)
		vsock_show(&current_filter);
	if (current_filter.dbs & (1<<TIPC_DB))
		tipc_show(&current_filter);
	if (current_filter.dbs & (1<<XDP_DB))
		xdp_show(&current_filter);
	if (current_filter.dbs & (1<<MPTCP_DB))
		mptcp_show(&current_filter);

	if (show_users || show_proc_ctx || show_sock_ctx)
		user_ent_destroy();

	render();

	return 0;
}
